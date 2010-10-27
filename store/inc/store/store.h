/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _STORE_STORE_H_
#define _STORE_STORE_H_ "$Revision: 1.6 $"

#include <store/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Handle opaque type.
 */
typedef void* storeHandle;


/** Acquire a Handle.
    @param  Handle [in] the Handle.
    @return store_E_None upon success
 */
storeError SAL_CALL store_acquireHandle (
    storeHandle Handle
) SAL_THROW_EXTERN_C();


/** Release a Handle.
    @param  Handle [in] the Handle.
    @return store_E_None          upon success,
            store_E_InvalidHandle otherwise.
 */
storeError SAL_CALL store_releaseHandle (
    storeHandle Handle
) SAL_THROW_EXTERN_C();



/** File Handle opaque type.
 */
typedef void* storeFileHandle;


/** Open a temporary file in memory.
    @param  nPageSize [in] the creation page size,
            integer multiple of minimum page size.
    @param  phFile [out] the File Handle.
    @return store_E_None upon success
 */
storeError SAL_CALL store_createMemoryFile (
    sal_uInt16       nPageSize,
    storeFileHandle *phFile
) SAL_THROW_EXTERN_C();


/** Open a file.
    @param  pFilename [in] the filename as URL or system path.
    @param  eAccessMode [in] the access mode.
            store_AccessCreate     truncate existing and create,
            store_AccessReadCreate create not existing,
            store_AccessReadWrite  write existing,
            store_AccessReadOnly   never modifies.
    @param  nPageSize [in] the creation page size,
            integer multiple of minimum page size.
    @param  phFile [out] the File Handle.
    @return store_E_None upon success
 */
storeError SAL_CALL store_openFile (
    rtl_uString     *pFilename,
    storeAccessMode  eAccessMode,
    sal_uInt16       nPageSize,
    storeFileHandle *phFile
) SAL_THROW_EXTERN_C();


/** Close a file.
    @param  hFile [in] the File Handle.
    @return store_E_None upon     success,
            store_E_InvalidHandle otherwise.
 */
storeError SAL_CALL store_closeFile (
    storeFileHandle hFile
) SAL_THROW_EXTERN_C();


/** Flush a file.
    @param  hFile [in] the File Handle.
    @return store_E_None upon success
 */
storeError SAL_CALL store_flushFile (
    storeFileHandle hFile
) SAL_THROW_EXTERN_C();


/** Get the number of referers to a file.
    @param  hFile [in] the File Handle.
    @param  pnRefCount [out] number of open directories and streams.
    @return store_E_None upon success
 */
storeError SAL_CALL store_getFileRefererCount (
    storeFileHandle  hFile,
    sal_uInt32      *pnRefCount
) SAL_THROW_EXTERN_C();


/** Get the size of a file.
    @param  hFile [in] the File Handle.
    @param  pnSize [out] the file size in bytes.
    @return store_E_None upon success
 */
storeError SAL_CALL store_getFileSize (
    storeFileHandle  hFile,
    sal_uInt32      *pnSize
) SAL_THROW_EXTERN_C();


/** Recover and Compact a file into another file.
    @see store_openFile()

    @param  pSrcFilename [in] opened with store_AccessReadOnly.
    @param  pDstFilename [in] created with store_AccessCreate.
    @return store_E_None upon success
 */
storeError SAL_CALL store_rebuildFile (
    rtl_uString *pSrcFilename,
    rtl_uString *pDstFilename
) SAL_THROW_EXTERN_C();



/** Directory Handle opaque type.
 */
typedef void* storeDirectoryHandle;


/** Open a directory.
    @see store_openFile()

    @param  hFile [in] the File Handle.
    @param  pPath [in] the directory path.
    @param  pName [in] the directory name.
    @param  eAccessMode [in] the access mode.
    @param  phDirectory [out] the Directory Handle.
    @return store_E_None upon success
 */
storeError SAL_CALL store_openDirectory (
    storeFileHandle       hFile,
    rtl_uString          *pPath,
    rtl_uString          *pName,
    storeAccessMode       eAccessMode,
    storeDirectoryHandle *phDirectory
) SAL_THROW_EXTERN_C();


/** Close a directory.
    @param  hDirectory [in] the Directory Handle.
    @return store_E_None          upon success,
            store_E_InvalidHandle otherwise.
 */
storeError SAL_CALL store_closeDirectory (
    storeDirectoryHandle hDirectory
) SAL_THROW_EXTERN_C();


/** Find first directory entry.
    @param  hDirectory [in] the Directory Handle.
    @param  pFindData [out] the Find Data structure.
    @return store_E_None       upon success,
            store_E_NoMoreFile upon end of iteration.
 */
storeError SAL_CALL store_findFirst (
    storeDirectoryHandle  hDirectory,
    storeFindData        *pFindData
) SAL_THROW_EXTERN_C();


/** Find next directory entry.
    @param  hDirectory [in] the Directory Handle.
    @param  pFindData [out] the Find Data structure.
    @return store_E_None       upon success,
            store_E_NoMoreFile upon end of iteration.
 */
storeError SAL_CALL store_findNext (
    storeDirectoryHandle  hDirectory,
    storeFindData        *pFindData
) SAL_THROW_EXTERN_C();



/** Stream Handle opaque type.
 */
typedef void* storeStreamHandle;


/** Open a stream.
    @see store_openFile()

    @param  hFile [in] the File Handle.
    @param  pPath [in] the stream path.
    @param  pName [in] the stream name.
    @param  eAccessMode [in] the access mode.
    @param  phStrm [out] the Stream Handle.
    @return store_E_None upon success
 */
storeError SAL_CALL store_openStream (
    storeFileHandle    hFile,
    rtl_uString       *pPath,
    rtl_uString       *pName,
    storeAccessMode    eMode,
    storeStreamHandle *phStrm
) SAL_THROW_EXTERN_C();


/** Close a stream.
    @param  hStrm [in] the Stream Handle.
    @return store_E_None          upon success,
            store_E_InvalidHandle otherwise.
 */
storeError SAL_CALL store_closeStream (
    storeStreamHandle hStrm
) SAL_THROW_EXTERN_C();


/** Read from a stream.
    @param  hStrm [in] the Stream Handle.
    @param  nOffset [in] the offset of the first byte to read.
    @param  pBuffer [out] the buffer.
    @param  nBytes [in] the number of bytes to read.
    @param  pnDone [out] the number of bytes actually read.
    @return store_E_None upon success
 */
storeError SAL_CALL store_readStream (
    storeStreamHandle  hStrm,
    sal_uInt32         nOffset,
    void              *pBuffer,
    sal_uInt32         nBytes,
    sal_uInt32        *pnDone
) SAL_THROW_EXTERN_C();


/** Write to a stream.
    @param  hStrm [in] the Stream Handle.
    @param  nOffset [in] the offset of the first byte to write.
    @param  pBuffer [in] the buffer.
    @param  nBytes [in] the number of bytes to write.
    @param  pnDone [out] the number of bytes actually written.
    @return store_E_None upon success
 */
storeError SAL_CALL store_writeStream (
    storeStreamHandle  hStrm,
    sal_uInt32         nOffset,
    const void        *pBuffer,
    sal_uInt32         nBytes,
    sal_uInt32        *pnDone
) SAL_THROW_EXTERN_C();


/** Flush a stream.
    @param  hStrm [in] the Stream Handle.
    @return store_E_None upon success
 */
storeError SAL_CALL store_flushStream (
    storeStreamHandle hStrm
) SAL_THROW_EXTERN_C();


/** Get the size of a stream.
    @param  hStrm [in] the Stream Handle.
    @param  pnSize [out] the stream size in bytes.
    @return store_E_None upon success
 */
storeError SAL_CALL store_getStreamSize (
    storeStreamHandle  hStrm,
    sal_uInt32        *pnSize
) SAL_THROW_EXTERN_C();


/** Set the size of a stream.
    @param  hStrm [in] the Stream Handle.
    @param  nSize [in] the new stream size in bytes.
    @return store_E_None upon success
 */
storeError SAL_CALL store_setStreamSize (
    storeStreamHandle hStrm,
    sal_uInt32        nSize
) SAL_THROW_EXTERN_C();



/** Set attributes of a file entry.
    @param  hFile [in] the File Handle.
    @param  pPath [in] the entry path.
    @param  pName [in] the entry name.
    @param  nMask1 [in] the attributes to be cleared.
    @param  nMask2 [in] the attributes to be set.
    @param  pnAttrib [out] the resulting attributes, may be NULL.
    @return store_E_None upon success
 */
storeError SAL_CALL store_attrib (
    storeFileHandle hFile,
    rtl_uString    *pPath,
    rtl_uString    *pName,
    sal_uInt32      nMask1,
    sal_uInt32      nMask2,
    sal_uInt32     *pnAttrib
) SAL_THROW_EXTERN_C();


/** Insert a file entry as 'hard link' to another file entry.
    @precond  Source must not exist, Destination must exist.
    @postcond Source has attribute STORE_ATTRIB_ISLINK.
    @see      store_attrib()

    @param  hFile [in] the File Handle
    @param  pSrcPath [in] the Source path
    @param  pSrcName [in] the Source name
    @param  pDstPath [in] the Destination path
    @param  pDstName [in] the Destination name
    @return store_E_None upon success
 */
storeError SAL_CALL store_link (
    storeFileHandle hFile,
    rtl_uString *pSrcPath, rtl_uString *pSrcName,
    rtl_uString *pDstPath, rtl_uString *pDstName
) SAL_THROW_EXTERN_C();


/** Insert a file entry as 'symbolic link' to another file entry.
    @precond  Source must not exist
    @postcond Source has attribute STORE_ATTRIB_ISLINK.
    @see      store_attrib()

    @param  hFile [in] the File Handle
    @param  pSrcPath [in] the Source path
    @param  pSrcName [in] the Source name
    @param  pDstPath [in] the Destination path
    @param  pDstName [in] the Destination name
    @return store_E_None upon success
 */
storeError SAL_CALL store_symlink (
    storeFileHandle hFile,
    rtl_uString *pSrcPath, rtl_uString *pSrcName,
    rtl_uString *pDstPath, rtl_uString *pDstName
) SAL_THROW_EXTERN_C();


/** Rename a file entry.
    @param  hFile [in] the File Handle
    @param  pSrcPath [in] the Source path
    @param  pSrcName [in] the Source name
    @param  pDstPath [in] the Destination path
    @param  pDstName [in] the Destination name
    @return store_E_None upon success
 */
storeError SAL_CALL store_rename (
    storeFileHandle hFile,
    rtl_uString *pSrcPath, rtl_uString *pSrcName,
    rtl_uString *pDstPath, rtl_uString *pDstName
) SAL_THROW_EXTERN_C();


/** Remove a file entry.
    @param  hFile [in] the File Handle
    @param  pPath [in] the entry path
    @param  pName [in] the entry name
    @return store_E_None upon success
 */
storeError SAL_CALL store_remove (
    storeFileHandle hFile,
    rtl_uString    *pPath,
    rtl_uString    *pName
) SAL_THROW_EXTERN_C();

/*========================================================================
 *
 * The End.
 *
 *======================================================================*/

#ifdef __cplusplus
}
#endif

#endif /* _STORE_STORE_H_ */




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
