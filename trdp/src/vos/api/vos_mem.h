/**********************************************************************************************************************/
/**
 * @file            vos_mem.h
 *
 * @brief           Memory and queue functions for OS abstraction
 *
 * @details			This module provides three services: 1. A memory control supervison
 *					- Private memory management with optimised fragmentation handling
 *					- A message queue handler (system-wide on supported systems)
 *					- Access to shared memory (on supported systems only)
 *					Within the prototype TRDP stack, only the memory management unit is currently in use.
 *
 * @note            Project: TCNOpen TRDP prototype stack
 *
 * @author          Bernd Loehr, NewTec GmbH
 *					Peter Brander (Memory scheme)
 *
 * @remarks All rights reserved. Reproduction, modification, use or disclosure
 *          to third parties without express authority is forbidden,
 *          Copyright Bombardier Transportation GmbH, Germany, 2012.
 *
 *
 * $Id$
 *
 */

#ifndef VOS_MEM_H
#define VOS_MEM_H

/***********************************************************************************************************************
 * INCLUDES
 */

#include "vos_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************************************************************
 * DEFINES
 */

#ifndef __cdecl
#define __cdecl
#endif

#define VOS_MEM_NBLOCKSIZES         15  /*<< Number of different sizes of memory allocation blocks */
#define VOS_MEM_MAX_PREALLOCATE     10  /*<< Max blocks to pre-allocate */

/** We internally allocate memory always by these block sizes. The largest available block is 524288 Bytes, provided
    the overal size of the used memory allocation area is larger. */
#define VOS_MEM_BLOCKSIZES  {32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, \
                             16384, 32768, 65536, 131072, 262144, 524288}

/** Default pre-allocation of free memory blocks. To avoid problems with too many small blocks and no large one.
   Specify how many of each block size that should be pre-allocated (and freed!) to pre-segment the memory area. */
#define VOS_MEM_PREALLOCATE  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 4, 0, 0}

/***********************************************************************************************************************
 * TYPEDEFS
 */

/** Opaque queue define	*/
typedef struct VOS_QUEUE_T *VOS_QUEUE_T;
typedef struct VOS_SHRD_T *VOS_SHRD_T;

/** enumeration for memory block sizes */
typedef enum
{
    TRDP_MEM_BLK_32 = 0,
    TRDP_MEM_BLK_64,
    TRDP_MEM_BLK_128,
    TRDP_MEM_BLK_256,
    TRDP_MEM_BLK_512,
    TRDP_MEM_BLK_1024,
    TRDP_MEM_BLK_2048,
    TRDP_MEM_BLK_4096,
    TRDP_MEM_BLK_8192,
    TRDP_MEM_BLK_16384,
    TRDP_MEM_BLK_32768,
    TRDP_MEM_BLK_65536,
    TRDP_MEM_BLK_131072,
    TRDP_MEM_BLK_262144,
    TRDP_MEM_BLK_524288
} VOS_MEM_BLK_T;

/***********************************************************************************************************************
 * PROTOTYPES
 */

/**********************************************************************************************************************/
/*	Memory																										      */
/**********************************************************************************************************************/

/**********************************************************************************************************************/
/** Initialize the memory unit.
 *  Init a supplied block of memory and prepare it for use with vos_alloc and vos_dealloc. The used block sizes can
 *	be supplied and will be preallocated.
 *
 *  @param[in]      pMemoryArea		Pointer to memory area to use
 *  @param[in]      size			Size of provided memory area
 *  @param[in]      fragMem			Pointer to list of preallocate block sizes, used to fragment memory for large blocks
 *  @retval         VOS_NO_ERR		no error
 *  @retval         VOS_PARAM_ERR	parameter out of range/invalid
 *  @retval         VOS_MEM_ERR		no memory available
 */

EXT_DECL VOS_ERR_T vos_memInit (
    UINT8           *pMemoryArea,
    UINT32          size,
    const UINT32    fragMem[VOS_MEM_NBLOCKSIZES]);

/**********************************************************************************************************************/
/** Delete the memory area.
 *	This will eventually invalidate any previously allocated memory blocks! It should be called last before the
 *  application quits. No further access to the memory blocks is allowed after this call.
 *
 *  @param[in]      pMemoryArea		Pointer to memory area to use
 *  @retval         VOS_NO_ERR		no error
 *  @retval         VOS_INIT_ERR	module not initialised
 *  @retval         VOS_PARAM_ERR	parameter out of range/invalid
 */

EXT_DECL VOS_ERR_T vos_memDelete (
    UINT8 *pMemoryArea);

/**********************************************************************************************************************/
/** Allocate a block of memory (from memory area above).
 *
 *  @param[in]      size			Size of requested block
 *
 *  @retval         Pointer to memory area
 *  @retval         NULL if no memory available
 */

EXT_DECL UINT8 *vos_memAlloc (
    UINT32 size);

/**********************************************************************************************************************/
/** Deallocate a block of memory (from memory area above).
 *
 *  @param[in]      pMemBlock		Pointer to memory block to be freed
 *  @retval         VOS_NO_ERR		no error
 *  @retval         VOS_INIT_ERR	module not initialised
 *  @retval         VOS_PARAM_ERR	parameter out of range/invalid
 */

EXT_DECL VOS_ERR_T vos_memFree (
    void *pMemBlock);

/**********************************************************************************************************************/
/** Return used and available memory (of memory area above).
 *
 *  @param[out]     pAllocatedMemory	Pointer to allocated memory size
 *  @param[out]     pFreeMemory			Pointer to free memory size
 *  @param[out]     pFragMem			Pointer to list of used memoryblocks
 *  @retval         VOS_NO_ERR			no error
 *  @retval         VOS_INIT_ERR	    module not initialised
 *  @retval         VOS_PARAM_ERR		parameter out of range/invalid
 */

EXT_DECL VOS_ERR_T vos_memCount(
    UINT32  * pAllocatedMemory,
    UINT32  * pFreeMemory,
    UINT32  * pFragMem[VOS_MEM_NBLOCKSIZES]);


/**********************************************************************************************************************/
/*	Queues																										      */
/**********************************************************************************************************************/

/**********************************************************************************************************************/
/** Initialize a message queue.
 *  Returns a handle for further calls
 *
 *  @param[in]      pKey			Unique identifier (file name)
 *  @param[out]     pQueueID		Pointer to returned queue handle
 *  @param[in]      maxNoMsg		maximum number of messages
 *  @param[in]      maxLength		maximum size of one messages
 *  @retval         VOS_NO_ERR		no error
 *  @retval         VOS_INIT_ERR	module not initialised
 *  @retval         VOS_NOINIT_ERR	invalid handle
 *  @retval         VOS_PARAM_ERR	parameter out of range/invalid
 *  @retval         VOS_INIT_ERR	not supported
 *  @retval         VOS_QUEUE_ERR	error creating queue
 */

EXT_DECL VOS_ERR_T vos_queueCreate (
    const CHAR8 *pKey,
    VOS_QUEUE_T *pQueueId,
    UINT32      maxNoMsg,
    UINT32      maxLength);

/**********************************************************************************************************************/
/** Destroy a message queue.
 *  Free all resources used by this queue
 *
 *  @param[in]      queueID			Queue handle
 *  @retval         VOS_NO_ERR		no error
 *  @retval         VOS_INIT_ERR	module not initialised
 *  @retval         VOS_NOINIT_ERR	invalid handle
 *  @retval         VOS_PARAM_ERR	parameter out of range/invalid
 */

EXT_DECL VOS_ERR_T vos_queueDestroy (
    VOS_QUEUE_T queueID);

/**********************************************************************************************************************/
/** Send a message.
 *
 *
 *  @param[in]      queueID			Queue handle
 *  @param[in]      pMsg			Pointer to message to be sent
 *  @param[in]      size			Message size
 *  @retval         VOS_NO_ERR		no error
 *  @retval         VOS_INIT_ERR	module not initialised
 *  @retval         VOS_NOINIT_ERR	invalid handle
 *  @retval         VOS_PARAM_ERR	parameter out of range/invalid
 *  @retval         VOS_QUEUE_FULL	queue is full
 */

EXT_DECL VOS_ERR_T vos_queueSend (
    VOS_QUEUE_T queueID,
    const UINT8 *pMsg,
    UINT32      size);

/**********************************************************************************************************************/
/** Get a message.
 *
 *
 *  @param[in]      queueID			Queue handle
 *  @param[out]     pMsg			Pointer to message to be received
 *  @param[in,out]  pSize			Pointer to max. message size on entry, actual size on exit
 *  @param[in]      usTimeout		Maximum time to wait for a message in usec
 *  @retval         VOS_NO_ERR		no error
 *  @retval         VOS_INIT_ERR	module not initialised
 *  @retval         VOS_NOINIT_ERR	invalid handle
 *  @retval         VOS_PARAM_ERR	parameter out of range/invalid
 *  @retval         VOS_QUEUE_ERR	queue is empty
 */

EXT_DECL VOS_ERR_T vos_queueReceive (
    VOS_QUEUE_T queueID,
    UINT8       *pMsg,
    UINT32      *pSize,
    UINT32      usTimeout);

/**********************************************************************************************************************/
/*	Shared memory																										      */
/**********************************************************************************************************************/

/**********************************************************************************************************************/
/** Create a shared memory area or attach to existing one.
 *  The first call with the a specified key will create a shared memory area with the supplied size and will return
 *  a handle and a pointer to that area. If the area already exists, the area will be attached.
 *	This function is not available in each target implementation.
 *
 *  @param[in]      pKey			Unique identifier (file name)
 *  @param[out]     pHandle			Pointer to returned handle
 *  @param[out]     ppMemoryArea	Pointer to pointer to memory area
 *  @param[in,out]  pSize			Pointer to size of area to allocate, on return actual size after attach
 *  @retval         VOS_NO_ERR		no error
 *  @retval         VOS_INIT_ERR	module not initialised
 *  @retval         VOS_NOINIT_ERR	invalid handle
 *  @retval         VOS_PARAM_ERR	parameter out of range/invalid
 *  @retval         VOS_MEM_ERR		no memory available
 */

EXT_DECL VOS_ERR_T vos_sharedOpen (
    const CHAR8 *pKey,
    VOS_SHRD_T  *pHandle,
    UINT8       * *ppMemoryArea,
    UINT32      *pSize);


/**********************************************************************************************************************/
/** Close connection to the shared memory area.
 *  If the area was created by the calling process, the area will be closed (freed). If the area was attached,
 *  it will be detached.
 *	This function is not available in each target implementation.
 *
 *  @param[in]      handle			Returned handle
 *  @param[in]      pMemoryArea		Pointer to memory area
 *  @retval         VOS_NO_ERR		no error
 *  @retval         VOS_INIT_ERR	module not initialised
 *  @retval         VOS_NOINIT_ERR	invalid handle
 *  @retval         VOS_PARAM_ERR	parameter out of range/invalid
 */

EXT_DECL VOS_ERR_T vos_sharedClose (
    VOS_SHRD_T  handle,
    const UINT8 *pMemoryArea);



#ifdef __cplusplus
}
#endif

#endif /* VOS_MEM_H */
