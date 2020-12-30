/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

#include <store/storedllapi.h>
#include <store/types.h>
#include <rtl/ustring.h>
#include <sal/types.h>

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
STORE_DLLPUBLIC storeError store_acquireHandle (
    storeHandle Handle
) SAL_THROW_EXTERN_C();

/** Release a Handle.
    @param  Handle [in] the Handle.
    @return store_E_None          upon success,
            store_E_InvalidHandle otherwise.
 */
STORE_DLLPUBLIC storeError store_releaseHandle (
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
STORE_DLLPUBLIC storeError store_createMemoryFile (
    sal_uInt16       nPageSize,
    storeFileHandle *phFile
) SAL_THROW_EXTERN_C();

/** Open a file.
    @param  pFilename [in] the filename as URL or system path.
    @param  eAccessMode [in] the access mode.
            storeAccessMode::Create     truncate existing and create,
            store_AccessReadCreate create not existing,
            storeAccessMode::ReadWrite  write existing,
            storeAccessMode::ReadOnly   never modifies.
    @param  nPageSize [in] the creation page size,
            integer multiple of minimum page size.
    @param  phFile [out] the File Handle.
    @return store_E_None upon success
 */
STORE_DLLPUBLIC storeError store_openFile (
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
STORE_DLLPUBLIC storeError store_closeFile (
    storeFileHandle hFile
) SAL_THROW_EXTERN_C();

/** Flush a file.
    @param  hFile [in] the File Handle.
    @return store_E_None upon success
 */
STORE_DLLPUBLIC storeError store_flushFile (
    storeFileHandle hFile
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
STORE_DLLPUBLIC storeError store_openDirectory (
    storeFileHandle       hFile,
    rtl_uString const    *pPath,
    rtl_uString const    *pName,
    storeAccessMode       eAccessMode,
    storeDirectoryHandle *phDirectory
) SAL_THROW_EXTERN_C();

/** Find first directory entry.
    @param  hDirectory [in] the Directory Handle.
    @param  pFindData [out] the Find Data structure.
    @return store_E_None       upon success,
            store_E_NoMoreFile upon end of iteration.
 */
STORE_DLLPUBLIC storeError store_findFirst (
    storeDirectoryHandle  hDirectory,
    storeFindData        *pFindData
) SAL_THROW_EXTERN_C();

/** Find next directory entry.
    @param  hDirectory [in] the Directory Handle.
    @param  pFindData [out] the Find Data structure.
    @return store_E_None       upon success,
            store_E_NoMoreFile upon end of iteration.
 */
STORE_DLLPUBLIC storeError store_findNext (
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
    @param  eMode [in] the access mode.
    @param  phStrm [out] the Stream Handle.
    @return store_E_None upon success
 */
STORE_DLLPUBLIC storeError store_openStream (
    storeFileHandle    hFile,
    rtl_uString const *pPath,
    rtl_uString const *pName,
    storeAccessMode    eMode,
    storeStreamHandle *phStrm
) SAL_THROW_EXTERN_C();

/** Read from a stream.
    @param  hStrm [in] the Stream Handle.
    @param  nOffset [in] the offset of the first byte to read.
    @param  pBuffer [out] the buffer.
    @param  nBytes [in] the number of bytes to read.
    @param  pnDone [out] the number of bytes actually read.
    @return store_E_None upon success
 */
STORE_DLLPUBLIC storeError store_readStream (
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
STORE_DLLPUBLIC storeError store_writeStream (
    storeStreamHandle  hStrm,
    sal_uInt32         nOffset,
    const void        *pBuffer,
    sal_uInt32         nBytes,
    sal_uInt32        *pnDone
) SAL_THROW_EXTERN_C();

/** Remove a file entry.
    @param  hFile [in] the File Handle
    @param  pPath [in] the entry path
    @param  pName [in] the entry name
    @return store_E_None upon success
 */
STORE_DLLPUBLIC storeError store_remove (
    storeFileHandle hFile,
    rtl_uString const *pPath,
    rtl_uString const *pName
) SAL_THROW_EXTERN_C();

#ifdef __cplusplus
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
