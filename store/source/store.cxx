/*************************************************************************
 *
 *  $RCSfile: store.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mhu $ $Date: 2001-03-13 20:49:56 $
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
 *  Contributor(s): Matthias Huetsch <matthias.huetsch@sun.com>
 *
 *
 ************************************************************************/

#define _STORE_STORE_CXX_ "$Revision: 1.2 $"

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_MEMORY_H_
#include <rtl/memory.h>
#endif
#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#ifndef _STORE_STORE_H_
#include <store/store.h>
#endif
#ifndef _STORE_OBJECT_HXX_
#include <store/object.hxx>
#endif
#ifndef _STORE_FILELCKB_HXX_
#include <store/filelckb.hxx>
#endif
#ifndef _STORE_MEMLCKB_HXX_
#include <store/memlckb.hxx>
#endif

#ifndef _STORE_STORBASE_HXX_
#include <storbase.hxx>
#endif
#ifndef _STORE_STORPAGE_HXX_
#include <storpage.hxx>
#endif
#ifndef _STORE_STORLCKB_HXX_
#include <storlckb.hxx>
#endif

using rtl::Reference;
using rtl::OString;

using namespace store;

/*========================================================================
 *
 * storeHandle implementation.
 *
 *======================================================================*/
/*
 * store_acquireHandle.
 */
storeError SAL_CALL store_acquireHandle (
    storeHandle Handle
) SAL_THROW_EXTERN_C()
{
    IStoreHandle *pHandle = static_cast<IStoreHandle*>(Handle);
    if (!pHandle)
        return store_E_InvalidHandle;

    pHandle->acquire();
    return store_E_None;
}

/*
 * store_releaseHandle.
 */
storeError SAL_CALL store_releaseHandle (
    storeHandle Handle
) SAL_THROW_EXTERN_C()
{
    IStoreHandle *pHandle = static_cast<IStoreHandle*>(Handle);
    if (!pHandle)
        return store_E_InvalidHandle;

    pHandle->release();
    return store_E_None;
}

/*========================================================================
 *
 * storeFileHandle implementation.
 *
 *======================================================================*/
/*
 * store_createMemoryFile.
 */
storeError SAL_CALL store_createMemoryFile (
    sal_uInt16       nPageSize,
    storeFileHandle *phFile
) SAL_THROW_EXTERN_C()
{
    if (!phFile)
        return store_E_InvalidParameter;
    *phFile = NULL;

    Reference<OMemoryLockBytes> xLockBytes (new OMemoryLockBytes());
    if (!xLockBytes.is())
        return store_E_OutOfMemory;

    Reference<OStorePageManager> xManager (new OStorePageManager());
    if (!xManager.is())
        return store_E_OutOfMemory;

    storeError eErrCode = xManager->initialize (
        &*xLockBytes, store_AccessCreate, nPageSize);
    if (eErrCode != store_E_None)
        return eErrCode;

    xManager->acquire();

    *phFile = (storeFileHandle)&(*xManager);
    return store_E_None;
}

/*
 * store_openFile.
 */
storeError SAL_CALL store_openFile (
    rtl_uString     *pFilename,
    storeAccessMode  eAccessMode,
    sal_uInt16       nPageSize,
    storeFileHandle *phFile
) SAL_THROW_EXTERN_C()
{
    if (phFile)
        *phFile = NULL;

    if (!(pFilename && phFile))
        return store_E_InvalidParameter;

    Reference<OFileLockBytes> xLockBytes (new OFileLockBytes());
    if (!xLockBytes.is())
        return store_E_OutOfMemory;

    storeError eErrCode = xLockBytes->create (pFilename, eAccessMode);
    if (eErrCode != store_E_None)
        return eErrCode;

    Reference<OStorePageManager> xManager (new OStorePageManager());
    if (!xManager.is())
        return store_E_OutOfMemory;

    eErrCode = xManager->initialize (&*xLockBytes, eAccessMode, nPageSize);
    if (eErrCode != store_E_None)
        return eErrCode;

    xManager->acquire();

    *phFile = (storeFileHandle)&(*xManager);
    return store_E_None;
}

/*
 * store_closeFile.
 */
storeError SAL_CALL store_closeFile (
    storeFileHandle Handle
) SAL_THROW_EXTERN_C()
{
    OStorePageManager *pManager =
        OStoreHandle<OStorePageManager>::query (Handle);
    if (!pManager)
        return store_E_InvalidHandle;

    pManager->close();
    pManager->release();
    return store_E_None;
}

/*
 * store_flushFile.
 */
storeError SAL_CALL store_flushFile (
    storeFileHandle Handle
) SAL_THROW_EXTERN_C()
{
    OStoreHandle<OStorePageManager> xManager (
        OStoreHandle<OStorePageManager>::query (Handle));
    if (!xManager.is())
        return store_E_InvalidHandle;

    return xManager->flush();
}

/*
 * store_getFileRefererCount.
 */
storeError SAL_CALL store_getFileRefererCount (
    storeFileHandle  Handle,
    sal_uInt32      *pnRefCount
) SAL_THROW_EXTERN_C()
{
    OStoreHandle<OStorePageManager> xManager (
        OStoreHandle<OStorePageManager>::query (Handle));
    if (!xManager.is())
        return store_E_InvalidHandle;

    if (!pnRefCount)
        return store_E_InvalidParameter;

    *pnRefCount = xManager->getRefererCount();
    return store_E_None;
}

/*
 * store_getFileSize.
 */
storeError SAL_CALL store_getFileSize (
    storeFileHandle  Handle,
    sal_uInt32      *pnSize
) SAL_THROW_EXTERN_C()
{
    OStoreHandle<OStorePageManager> xManager (
        OStoreHandle<OStorePageManager>::query (Handle));
    if (!xManager.is())
        return store_E_InvalidHandle;

    if (!pnSize)
        return store_E_InvalidParameter;

    return xManager->size (*pnSize);
}

/*
 * store_rebuildFile.
 */
storeError SAL_CALL store_rebuildFile (
    rtl_uString *pSrcFilename,
    rtl_uString *pDstFilename
) SAL_THROW_EXTERN_C()
{
    storeError eErrCode = store_E_None;

    if (!(pSrcFilename && pDstFilename))
        return store_E_InvalidParameter;

    Reference<OStorePageManager> xManager (new OStorePageManager());
    if (!xManager.is())
        return store_E_OutOfMemory;

    Reference<OFileLockBytes> xSrcLB (new OFileLockBytes());
    if (!xSrcLB.is())
        return store_E_OutOfMemory;

    eErrCode = xSrcLB->create (pSrcFilename, store_AccessReadOnly);
    if (eErrCode != store_E_None)
        return eErrCode;

    Reference<OFileLockBytes> xDstLB (new OFileLockBytes());
    if (!xDstLB.is())
        return store_E_OutOfMemory;

    eErrCode = xDstLB->create (pDstFilename, store_AccessCreate);
    if (eErrCode != store_E_None)
        return eErrCode;

    return xManager->rebuild (&*xSrcLB, &*xDstLB);
}

/*========================================================================
 *
 * storeDirectoryHandle implementation.
 *
 *======================================================================*/
/*
 * store_openDirectory.
 */
storeError SAL_CALL store_openDirectory (
    storeFileHandle       hFile,
    rtl_uString          *pPath,
    rtl_uString          *pName,
    storeAccessMode       eAccessMode,
    storeDirectoryHandle *phDirectory
) SAL_THROW_EXTERN_C()
{
    storeError eErrCode = store_E_None;
    if (phDirectory)
        *phDirectory = NULL;

    OStoreHandle<OStorePageManager> xManager (
        OStoreHandle<OStorePageManager>::query (hFile));
    if (!xManager.is())
        return store_E_InvalidHandle;

    if (!(pPath && pName && phDirectory))
        return store_E_InvalidParameter;

    Reference<OStoreDirectory> xDirectory (new OStoreDirectory());
    if (!xDirectory.is())
        return store_E_OutOfMemory;

    eErrCode = xDirectory->create (&*xManager, pPath, pName, eAccessMode);
    if (eErrCode != store_E_None)
        return eErrCode;

    xDirectory->acquire();

    *phDirectory = (storeDirectoryHandle)&(*xDirectory);
    return store_E_None;
}

/*
 * store_closeDirectory.
 */
storeError SAL_CALL store_closeDirectory (
    storeDirectoryHandle Handle
) SAL_THROW_EXTERN_C()
{
    OStoreDirectory *pDirectory =
        OStoreHandle<OStoreDirectory>::query (Handle);
    if (!pDirectory)
        return store_E_InvalidHandle;

    pDirectory->release();
    return store_E_None;
}

/*
 * store_findFirst.
 */
storeError SAL_CALL store_findFirst (
    storeDirectoryHandle  Handle,
    storeFindData        *pFindData
) SAL_THROW_EXTERN_C()
{
    OStoreHandle<OStoreDirectory> xDirectory (
        OStoreHandle<OStoreDirectory>::query (Handle));
    if (!xDirectory.is())
        return store_E_InvalidHandle;

    if (!pFindData)
        return store_E_InvalidParameter;

    // Initialize FindData.
    rtl_zeroMemory (pFindData, sizeof (storeFindData));

    // Find first.
    pFindData->m_nReserved = (sal_uInt32)(~0);
    return xDirectory->iterate (*pFindData);
}

/*
 * store_findNext.
 */
storeError SAL_CALL store_findNext (
    storeDirectoryHandle  Handle,
    storeFindData        *pFindData
) SAL_THROW_EXTERN_C()
{
    OStoreHandle<OStoreDirectory> xDirectory (
        OStoreHandle<OStoreDirectory>::query (Handle));
    if (!xDirectory.is())
        return store_E_InvalidHandle;

    if (!pFindData)
        return store_E_InvalidParameter;

    // Check FindData.
    if (!pFindData->m_nReserved)
        return store_E_NoMoreFiles;

    // Find next.
    pFindData->m_nReserved -= 1;
    return xDirectory->iterate (*pFindData);
}

/*========================================================================
 *
 * storeStreamHandle implementation.
 *
 *======================================================================*/
/*
 * store_openStream
 */
storeError SAL_CALL store_openStream (
    storeFileHandle    hFile,
    rtl_uString       *pPath,
    rtl_uString       *pName,
    storeAccessMode    eAccessMode,
    storeStreamHandle *phStream
) SAL_THROW_EXTERN_C()
{
    storeError eErrCode = store_E_None;
    if (phStream)
        *phStream = NULL;

    OStoreHandle<OStorePageManager> xManager (
        OStoreHandle<OStorePageManager>::query (hFile));
    if (!xManager.is())
        return store_E_InvalidHandle;

    if (!(pPath && pName && phStream))
        return store_E_InvalidParameter;

    Reference<OStoreLockBytes> xLockBytes (new OStoreLockBytes());
    if (!xLockBytes.is())
        return store_E_OutOfMemory;

    eErrCode = xLockBytes->create (&*xManager, pPath, pName, eAccessMode);
    if (eErrCode != store_E_None)
        return eErrCode;

    xLockBytes->acquire();

    *phStream = (storeStreamHandle)&(*xLockBytes);
    return store_E_None;
}

/*
 * store_closeStream.
 */
storeError SAL_CALL store_closeStream (
    storeStreamHandle Handle
) SAL_THROW_EXTERN_C()
{
    OStoreLockBytes *pLockBytes =
        OStoreHandle<OStoreLockBytes>::query (Handle);
    if (!pLockBytes)
        return store_E_InvalidHandle;

    pLockBytes->release();
    return store_E_None;
}

/*
 * store_readStream.
 */
storeError SAL_CALL store_readStream (
    storeStreamHandle  Handle,
    sal_uInt32         nOffset,
    void              *pBuffer,
    sal_uInt32         nBytes,
    sal_uInt32        *pnDone
) SAL_THROW_EXTERN_C()
{
    OStoreHandle<OStoreLockBytes> xLockBytes (
        OStoreHandle<OStoreLockBytes>::query (Handle));
    if (!xLockBytes.is())
        return store_E_InvalidHandle;

    if (!(pBuffer && pnDone))
        return store_E_InvalidParameter;

    return xLockBytes->readAt (nOffset, pBuffer, nBytes, *pnDone);
}

/*
 * store_writeStream.
 */
storeError SAL_CALL store_writeStream (
    storeStreamHandle  Handle,
    sal_uInt32         nOffset,
    const void        *pBuffer,
    sal_uInt32         nBytes,
    sal_uInt32        *pnDone
) SAL_THROW_EXTERN_C()
{
    OStoreHandle<OStoreLockBytes> xLockBytes (
        OStoreHandle<OStoreLockBytes>::query (Handle));
    if (!xLockBytes.is())
        return store_E_InvalidHandle;

    if (!(pBuffer && pnDone))
        return store_E_InvalidParameter;

    return xLockBytes->writeAt (nOffset, pBuffer, nBytes, *pnDone);
}

/*
 * store_flushStream.
 */
storeError SAL_CALL store_flushStream (
    storeStreamHandle Handle
) SAL_THROW_EXTERN_C()
{
    OStoreHandle<OStoreLockBytes> xLockBytes (
        OStoreHandle<OStoreLockBytes>::query (Handle));
    if (!xLockBytes.is())
        return store_E_InvalidHandle;

    return xLockBytes->flush();
}

/*
 * store_getStreamSize.
 */
storeError SAL_CALL store_getStreamSize (
    storeStreamHandle  Handle,
    sal_uInt32        *pnSize
) SAL_THROW_EXTERN_C()
{
    OStoreHandle<OStoreLockBytes> xLockBytes (
        OStoreHandle<OStoreLockBytes>::query (Handle));
    if (!xLockBytes.is())
        return store_E_InvalidHandle;

    if (!pnSize)
        return store_E_InvalidParameter;

    return xLockBytes->stat (*pnSize);
}

/*
 * store_setStreamSize.
 */
storeError SAL_CALL store_setStreamSize (
    storeStreamHandle Handle,
    sal_uInt32        nSize
) SAL_THROW_EXTERN_C()
{
    OStoreHandle<OStoreLockBytes> xLockBytes (
        OStoreHandle<OStoreLockBytes>::query (Handle));
    if (!xLockBytes.is())
        return store_E_InvalidHandle;

    return xLockBytes->setSize (nSize);
}

/*========================================================================
 *
 * Common storeDirectoryHandle and storeStreamHandle operations.
 *
 *======================================================================*/
/*
 * store_attrib.
 */
storeError SAL_CALL store_attrib (
    storeFileHandle Handle,
    rtl_uString    *pPath,
    rtl_uString    *pName,
    sal_uInt32      nMask1,
    sal_uInt32      nMask2,
    sal_uInt32     *pnAttrib
) SAL_THROW_EXTERN_C()
{
    storeError eErrCode = store_E_None;
    if (pnAttrib)
        *pnAttrib = 0;

    OStoreHandle<OStorePageManager> xManager (
        OStoreHandle<OStorePageManager>::query (Handle));
    if (!xManager.is())
        return store_E_InvalidHandle;

    if (!(pPath && pName))
        return store_E_InvalidParameter;

    // Setup page key.
    OString aPath (pPath->buffer, pPath->length, RTL_TEXTENCODING_UTF8);
    OString aName (pName->buffer, pName->length, RTL_TEXTENCODING_UTF8);
    OStorePageKey aKey;

    eErrCode = OStorePageNameBlock::namei (aPath.pData, aName.pData, aKey);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Obtain or modify page attributes.
    sal_uInt32 nAttrib = 0;
    eErrCode = xManager->attrib (aKey, nMask1, nMask2, nAttrib);
    if (pnAttrib)
        *pnAttrib = nAttrib;
    return eErrCode;
}

/*
 * store_link.
 */
storeError SAL_CALL store_link (
    storeFileHandle Handle,
    rtl_uString *pSrcPath, rtl_uString *pSrcName,
    rtl_uString *pDstPath, rtl_uString *pDstName
) SAL_THROW_EXTERN_C()
{
    storeError eErrCode = store_E_None;

    OStoreHandle<OStorePageManager> xManager (
        OStoreHandle<OStorePageManager>::query (Handle));
    if (!xManager.is())
        return store_E_InvalidHandle;

    if (!(pSrcPath && pSrcName))
        return store_E_InvalidParameter;

    if (!(pDstPath && pDstName))
        return store_E_InvalidParameter;

    // Setup 'Source' page key.
    OString aSrcPath (
        pSrcPath->buffer, pSrcPath->length, RTL_TEXTENCODING_UTF8);
    OString aSrcName (
        pSrcName->buffer, pSrcName->length, RTL_TEXTENCODING_UTF8);
    OStorePageKey aSrcKey;

    eErrCode = OStorePageNameBlock::namei (
        aSrcPath.pData, aSrcName.pData, aSrcKey);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Setup 'Destination' page key.
    OString aDstPath (
        pDstPath->buffer, pDstPath->length, RTL_TEXTENCODING_UTF8);
    OString aDstName (
        pDstName->buffer, pDstName->length, RTL_TEXTENCODING_UTF8);
    OStorePageKey aDstKey;

    eErrCode = OStorePageNameBlock::namei (
        aDstPath.pData, aDstName.pData, aDstKey);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Link 'Source' hard to 'Destination'.
    return xManager->link (aSrcKey, aDstKey);
}

/*
 * store_symlink.
 */
storeError SAL_CALL store_symlink (
    storeFileHandle Handle,
    rtl_uString *pSrcPath, rtl_uString *pSrcName,
    rtl_uString *pDstPath, rtl_uString *pDstName
) SAL_THROW_EXTERN_C()
{
    storeError eErrCode = store_E_None;

    OStoreHandle<OStorePageManager> xManager (
        OStoreHandle<OStorePageManager>::query (Handle));
    if (!xManager.is())
        return store_E_InvalidHandle;

    if (!(pSrcPath && pSrcName))
        return store_E_InvalidParameter;

    if (!(pDstPath && pDstName))
        return store_E_InvalidParameter;

    // Setup 'Destination' page key.
    OString aDstPath (
        pDstPath->buffer, pDstPath->length, RTL_TEXTENCODING_UTF8);
    OString aDstName (
        pDstName->buffer, pDstName->length, RTL_TEXTENCODING_UTF8);
    OStorePageKey aDstKey;

    eErrCode = OStorePageNameBlock::namei (
        aDstPath.pData, aDstName.pData, aDstKey);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Insert 'Source' as symlink to 'Destination'.
    OString aSrcPath (
        pSrcPath->buffer, pSrcPath->length, RTL_TEXTENCODING_UTF8);
    OString aSrcName (
        pSrcName->buffer, pSrcName->length, RTL_TEXTENCODING_UTF8);

    return xManager->symlink (aSrcPath.pData, aSrcName.pData, aDstKey);
}

/*
 * store_rename.
 */
storeError SAL_CALL store_rename (
    storeFileHandle Handle,
    rtl_uString *pSrcPath, rtl_uString *pSrcName,
    rtl_uString *pDstPath, rtl_uString *pDstName
) SAL_THROW_EXTERN_C()
{
    storeError eErrCode = store_E_None;

    OStoreHandle<OStorePageManager> xManager (
        OStoreHandle<OStorePageManager>::query (Handle));
    if (!xManager.is())
        return store_E_InvalidHandle;

    if (!(pSrcPath && pSrcName))
        return store_E_InvalidParameter;

    if (!(pDstPath && pDstName))
        return store_E_InvalidParameter;

    // Setup 'Source' page key.
    OString aSrcPath (
        pSrcPath->buffer, pSrcPath->length, RTL_TEXTENCODING_UTF8);
    OString aSrcName (
        pSrcName->buffer, pSrcName->length, RTL_TEXTENCODING_UTF8);
    OStorePageKey aSrcKey;

    eErrCode = OStorePageNameBlock::namei (
        aSrcPath.pData, aSrcName.pData, aSrcKey);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Rename 'Source' into 'Destination'.
    OString aDstPath (
        pDstPath->buffer, pDstPath->length, RTL_TEXTENCODING_UTF8);
    OString aDstName (
        pDstName->buffer, pDstName->length, RTL_TEXTENCODING_UTF8);

    return xManager->rename (aSrcKey, aDstPath.pData, aDstName.pData);
}

/*
 * store_remove.
 */
storeError SAL_CALL store_remove (
    storeFileHandle Handle,
    rtl_uString    *pPath,
    rtl_uString    *pName
) SAL_THROW_EXTERN_C()
{
    storeError eErrCode = store_E_None;

    OStoreHandle<OStorePageManager> xManager (
        OStoreHandle<OStorePageManager>::query (Handle));
    if (!xManager.is())
        return store_E_InvalidHandle;

    if (!(pPath && pName))
        return store_E_InvalidParameter;

    // Setup page key.
    OString aPath (pPath->buffer, pPath->length, RTL_TEXTENCODING_UTF8);
    OString aName (pName->buffer, pName->length, RTL_TEXTENCODING_UTF8);
    OStorePageKey aKey;

    eErrCode = OStorePageNameBlock::namei (aPath.pData, aName.pData, aKey);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Remove.
    return xManager->remove (aKey);
}

