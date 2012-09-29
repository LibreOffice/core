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


#include "store/store.h"

#include <sal/types.h>
#include <rtl/string.hxx>
#include <rtl/ref.hxx>

#include "object.hxx"
#include "lockbyte.hxx"

#include "storbase.hxx"
#include "storpage.hxx"
#include "stordir.hxx"
#include "storlckb.hxx"

using rtl::Reference;
using rtl::OString;

namespace store
{
/** Template helper class as type safe Reference to store_handle_type.
 */
template<class store_handle_type>
class OStoreHandle : public rtl::Reference<store_handle_type>
{
public:
    OStoreHandle (store_handle_type * pHandle)
        : rtl::Reference<store_handle_type> (pHandle)
    {}

    static store_handle_type * SAL_CALL query (void * pHandle)
    {
        return store::query (
            static_cast<IStoreHandle*>(pHandle),
            static_cast<store_handle_type*>(0));
    }
};
}


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

    Reference<ILockBytes> xLockBytes;

    storeError eErrCode = MemoryLockBytes_createInstance(xLockBytes);
    if (eErrCode != store_E_None)
        return eErrCode;
    OSL_ASSERT(xLockBytes.is());

    Reference<OStorePageManager> xManager (new OStorePageManager());
    if (!xManager.is())
        return store_E_OutOfMemory;

    eErrCode = xManager->initialize (
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

    Reference<ILockBytes> xLockBytes;

    storeError eErrCode = FileLockBytes_createInstance (xLockBytes, pFilename, eAccessMode);
    if (eErrCode != store_E_None)
        return eErrCode;
    OSL_ASSERT(xLockBytes.is());

    Reference<OStorePageManager> xManager (new OStorePageManager());
    if (!xManager.is())
        return store_E_OutOfMemory;

    eErrCode = xManager->initialize (
        &*xLockBytes, eAccessMode, nPageSize);
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

    storeError eErrCode = pManager->close();
    pManager->release();
    return eErrCode;
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

    Reference<ILockBytes> xSrcLB;
    eErrCode = FileLockBytes_createInstance (xSrcLB, pSrcFilename, store_AccessReadOnly);
    if (eErrCode != store_E_None)
        return eErrCode;
    OSL_ASSERT(xSrcLB.is());

    Reference<ILockBytes> xDstLB;
    eErrCode = FileLockBytes_createInstance (xDstLB, pDstFilename, store_AccessCreate);
    if (eErrCode != store_E_None)
        return eErrCode;
    OSL_ASSERT(xDstLB.is());

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

    Reference<OStoreDirectory_Impl> xDirectory (new OStoreDirectory_Impl());
    if (!xDirectory.is())
        return store_E_OutOfMemory;

    OString aPath (pPath->buffer, pPath->length, RTL_TEXTENCODING_UTF8);
    OString aName (pName->buffer, pName->length, RTL_TEXTENCODING_UTF8);

    eErrCode = xDirectory->create (&*xManager, aPath.pData, aName.pData, eAccessMode);
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
    OStoreDirectory_Impl *pDirectory =
        OStoreHandle<OStoreDirectory_Impl>::query (Handle);
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
    OStoreHandle<OStoreDirectory_Impl> xDirectory (
        OStoreHandle<OStoreDirectory_Impl>::query (Handle));
    if (!xDirectory.is())
        return store_E_InvalidHandle;

    if (!pFindData)
        return store_E_InvalidParameter;

    // Initialize FindData.
    memset (pFindData, 0, sizeof (storeFindData));

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
    OStoreHandle<OStoreDirectory_Impl> xDirectory (
        OStoreHandle<OStoreDirectory_Impl>::query (Handle));
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

    OString aPath (pPath->buffer, pPath->length, RTL_TEXTENCODING_UTF8);
    OString aName (pName->buffer, pName->length, RTL_TEXTENCODING_UTF8);

    eErrCode = xLockBytes->create (&*xManager, aPath.pData, aName.pData, eAccessMode);
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

    eErrCode = OStorePageManager::namei (aPath.pData, aName.pData, aKey);
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

    eErrCode = OStorePageManager::namei (
        aSrcPath.pData, aSrcName.pData, aSrcKey);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Setup 'Destination' page key.
    OString aDstPath (
        pDstPath->buffer, pDstPath->length, RTL_TEXTENCODING_UTF8);
    OString aDstName (
        pDstName->buffer, pDstName->length, RTL_TEXTENCODING_UTF8);
    OStorePageKey aDstKey;

    eErrCode = OStorePageManager::namei (
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

    eErrCode = OStorePageManager::namei (
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

    eErrCode = OStorePageManager::namei (
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

    eErrCode = OStorePageManager::namei (aPath.pData, aName.pData, aKey);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Remove.
    return xManager->remove (aKey);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
