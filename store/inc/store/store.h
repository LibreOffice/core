/*************************************************************************
 *
 *  $RCSfile: store.h,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:18:31 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _STORE_STORE_H_
#define _STORE_STORE_H_ "$Revision: 1.1.1.1 $"

#ifndef _STORE_TYPES_H_
#include <store/types.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** Opaque type storeHandle.
 */
typedef void* storeHandle;

/** store_acquireHandle.
 *  @param  Handle [in]
 *  @return store_E_None upon success
 */
storeError SAL_CALL store_acquireHandle (storeHandle Handle);

/** store_releaseHandle.
 *  @param  Handle [in]
 *  @return store_E_None          upon success,
 *          store_E_InvalidHandle otherwise.
 */
storeError SAL_CALL store_releaseHandle (storeHandle Handle);


/** Opaque type storeFileHandle.
 */
typedef void* storeFileHandle;

/** store_createMemoryFile.
 *  @param  nPageSize [in] the creation page size,
 *          integer multiple of minimum page size.
 *  @param  phFile [out]
 *  @return store_E_None upon success
 */
storeError SAL_CALL store_createMemoryFile (
    sal_uInt16       nPageSize,
    storeFileHandle *phFile);

/** store_openFile.
 *  @param  pFilename [in] the filename in host syntax.
 *  @param  eAccessMode [in] the access mode.
 *          store_AccessCreate     truncate existing and create,
 *          store_AccessReadCreate create not existing,
 *          store_AccessReadWrite  write existing,
 *          store_AccessReadOnly   never modifies.
 *  @param  nPageSize [in] the creation page size,
 *          integer multiple of minimum page size.
 *  @param  phFile [out]
 *  @return store_E_None upon success
 */
storeError SAL_CALL store_openFile (
    rtl_uString     *pFilename,
    storeAccessMode  eAccessMode,
    sal_uInt16       nPageSize,
    storeFileHandle *phFile);

/** store_closeFile.
 *  @param  hFile [in]
 *  @return store_E_None upon     success,
 *          store_E_InvalidHandle otherwise.
 */
storeError SAL_CALL store_closeFile (
    storeFileHandle hFile);

/** store_flushFile.
 *  @param  hFile [in]
 *  @return store_E_None upon success
 */
storeError SAL_CALL store_flushFile (
    storeFileHandle hFile);

/** store_getFileRefererCount.
 *  @param  hFile [in]
 *  @param  pnRefCount [out] number of open directories and streams.
 *  @return store_E_None upon success
 */
storeError SAL_CALL store_getFileRefererCount (
    storeFileHandle  hFile,
    sal_uInt32      *pnRefCount);

/** store_getFileSize.
 *  @param  hFile [in]
 *  @param  pnSize [out]
 *  @return store_E_None upon success
 */
storeError SAL_CALL store_getFileSize (
    storeFileHandle  hFile,
    sal_uInt32      *pnSize);

/** store_rebuildFile.
 *  @param  pSrcFilename [in] opened with store_AccessReadOnly.
 *  @param  pDstFilename [in] created with store_AccessCreate.
 *  @return store_E_None upon success
 */
storeError SAL_CALL store_rebuildFile (
    rtl_uString *pSrcFilename,
    rtl_uString *pDstFilename);


/** Opaque type storeDirectoryHandle.
 */
typedef void* storeDirectoryHandle;

/** store_openDirectory.
 *  @param  hFile [in]
 *  @param  pPath [in]
 *  @param  pName [in]
 *  @param  eAccessMode [in]
 *  @param  phDirectory [out]
 *  @return store_E_None upon success
 */
storeError SAL_CALL store_openDirectory (
    storeFileHandle       hFile,
    rtl_uString          *pPath,
    rtl_uString          *pName,
    storeAccessMode       eAccessMode,
    storeDirectoryHandle *phDirectory);

/** store_closeDirectory.
 *  @param  hDirectory [in]
 *  @return store_E_None          upon success,
 *          store_E_InvalidHandle otherwise.
 */
storeError SAL_CALL store_closeDirectory (
    storeDirectoryHandle hDirectory);

/** store_findFirst.
 *  @param  hDirectory [in]
 *  @param  pFindData [out]
 *  @return store_E_None       upon success,
 *          store_E_NoMoreFile upon end of iteration.
 */
storeError SAL_CALL store_findFirst (
    storeDirectoryHandle  hDirectory,
    storeFindData        *pFindData);

/** store_findNext.
 *  @param  hDirectory [in]
 *  @param  pFindData [out]
 *  @return store_E_None       upon success,
 *          store_E_NoMoreFile upon end of iteration.
 */
storeError SAL_CALL store_findNext (
    storeDirectoryHandle  hDirectory,
    storeFindData        *pFindData);


/** Opaque type storeStreamHandle.
 */
typedef void* storeStreamHandle;

/** store_openStream.
 *  @param  hFile [in]
 *  @param  pPath [in]
 *  @param  pName [in]
 *  @param  eAccessMode [in]
 *  @param  phStrm [out]
 *  @return store_E_None upon success
 */
storeError SAL_CALL store_openStream (
    storeFileHandle    hFile,
    rtl_uString       *pPath,
    rtl_uString       *pName,
    storeAccessMode    eMode,
    storeStreamHandle *phStrm);

/** store_closeStream.
 *  @param  hStrm [in]
 *  @return store_E_None          upon success,
 *          store_E_InvalidHandle otherwise.
 */
storeError SAL_CALL store_closeStream (
    storeStreamHandle hStrm);

/** store_readStream.
 *  @param  hStrm [in]
 *  @param  nOffset [in]
 *  @param  pBuffer [out]
 *  @param  nBytes [in]
 *  @param  pnDone [out]
 *  @return store_E_None upon success
 */
storeError SAL_CALL store_readStream (
    storeStreamHandle  hStrm,
    sal_uInt32         nOffset,
    void              *pBuffer,
    sal_uInt32         nBytes,
    sal_uInt32        *pnDone);

/** store_writeStream.
 *  @param  hStrm [in]
 *  @param  nOffset [in]
 *  @param  pBuffer [in]
 *  @param  nBytes [in]
 *  @param  pnDone [out]
 *  @return store_E_None upon success
 */
storeError SAL_CALL store_writeStream (
    storeStreamHandle  hStrm,
    sal_uInt32         nOffset,
    const void        *pBuffer,
    sal_uInt32         nBytes,
    sal_uInt32        *pnDone);

/** store_flushStream.
 *  @param  hStrm [in]
 *  @return store_E_None upon success
 */
storeError SAL_CALL store_flushStream (
    storeStreamHandle hStrm);

/** store_getStreamSize.
 *  @param  hStrm [in]
 *  @param  pnSize [out]
 *  @return store_E_None upon success
 */
storeError SAL_CALL store_getStreamSize (
    storeStreamHandle  hStrm,
    sal_uInt32        *pnSize);

/** store_setStreamSize.
 *  @param  hStrm [in]
 *  @param  nSize [in]
 *  @return store_E_None upon success
 */
storeError SAL_CALL store_setStreamSize (
    storeStreamHandle hStrm,
    sal_uInt32        nSize);

/** store_attrib.
 *  @param  hFile [in]
 *  @param  pPath [in]
 *  @param  pName [in]
 *  @param  nMask1 [in]
 *  @param  nMask2 [in]
 *  @param  pnAttrib [out] may be NULL.
 *  @return store_E_None upon success
 */
storeError SAL_CALL store_attrib (
    storeFileHandle hFile,
    rtl_uString    *pPath,
    rtl_uString    *pName,
    sal_uInt32      nMask1,
    sal_uInt32      nMask2,
    sal_uInt32     *pnAttrib);

/** store_link.
 *  @param  hFile [in]
 *  @param  pSrcPath [in]
 *  @param  pSrcName [in]
 *  @param  pDstPath [in]
 *  @param  pDstName [in]
 *  @return store_E_None upon success
 */
storeError SAL_CALL store_link (
    storeFileHandle hFile,
    rtl_uString *pSrcPath, rtl_uString *pSrcName,
    rtl_uString *pDstPath, rtl_uString *pDstName);

/** store_symlink.
 *  @param  hFile [in]
 *  @param  pSrcPath [in]
 *  @param  pSrcName [in]
 *  @param  pDstPath [in]
 *  @param  pDstName [in]
 *  @return store_E_None upon success
 */
storeError SAL_CALL store_symlink (
    storeFileHandle hFile,
    rtl_uString *pSrcPath, rtl_uString *pSrcName,
    rtl_uString *pDstPath, rtl_uString *pDstName);

/** store_rename.
 *  @param  hFile [in]
 *  @param  pSrcPath [in]
 *  @param  pSrcName [in]
 *  @param  pDstPath [in]
 *  @param  pDstName [in]
 *  @return store_E_None upon success
 */
storeError SAL_CALL store_rename (
    storeFileHandle hFile,
    rtl_uString *pSrcPath, rtl_uString *pSrcName,
    rtl_uString *pDstPath, rtl_uString *pDstName);

/** store_remove.
 *  @param  hFile [in]
 *  @param  pPath [in]
 *  @param  pName [in]
 *  @return store_E_None upon success
 */
storeError SAL_CALL store_remove (
    storeFileHandle hFile,
    rtl_uString    *pPath,
    rtl_uString    *pName);

/*========================================================================
 *
 * The End.
 *
 *======================================================================*/

#ifdef __cplusplus
}
#endif

#endif /* _STORE_STORE_H_ */




