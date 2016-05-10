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

#include "lockbyte.hxx"

#include "boost/noncopyable.hpp"
#include "sal/types.h"
#include "osl/diagnose.h"
#include "osl/file.h"
#include "osl/process.h"
#include "rtl/alloc.h"
#include "rtl/ustring.hxx"
#include "sal/log.hxx"

#include "object.hxx"
#include "storbase.hxx"

#include <string.h>

using namespace store;

/*========================================================================
 *
 * ILockBytes (non-virtual interface) implementation.
 *
 *======================================================================*/

storeError ILockBytes::initialize (rtl::Reference< PageData::Allocator > & rxAllocator, sal_uInt16 nPageSize)
{
    OSL_PRECOND((STORE_MINIMUM_PAGESIZE <= nPageSize) && (nPageSize <= STORE_MAXIMUM_PAGESIZE), "invalid PageSize");
    return initialize_Impl (rxAllocator, nPageSize);
}

storeError ILockBytes::readPageAt (PageHolder & rPage, sal_uInt32 nOffset)
{
    OSL_PRECOND(!(nOffset == STORE_PAGE_NULL), "store::ILockBytes::readPageAt(): invalid Offset");
    if (nOffset == STORE_PAGE_NULL)
        return store_E_CantSeek;

    return readPageAt_Impl (rPage, nOffset);
}

storeError ILockBytes::writePageAt (PageHolder const & rPage, sal_uInt32 nOffset)
{
    // [SECURITY:ValInput]
    PageData const * pagedata = rPage.get();
    OSL_PRECOND(!(pagedata == nullptr), "store::ILockBytes::writePageAt(): invalid Page");
    if (pagedata == nullptr)
        return store_E_InvalidParameter;

    sal_uInt32 const offset = pagedata->location();
    OSL_PRECOND(!(nOffset != offset), "store::ILockBytes::writePageAt(): inconsistent Offset");
    if (nOffset != offset)
        return store_E_InvalidParameter;

    OSL_PRECOND(!(nOffset == STORE_PAGE_NULL), "store::ILockBytes::writePageAt(): invalid Offset");
    if (nOffset == STORE_PAGE_NULL)
        return store_E_CantSeek;

    return writePageAt_Impl (rPage, nOffset);
}

storeError ILockBytes::readAt (sal_uInt32 nOffset, void * pBuffer, sal_uInt32 nBytes)
{
    // [SECURITY:ValInput]
    sal_uInt8 * dst_lo = static_cast<sal_uInt8*>(pBuffer);
    if (!(dst_lo != nullptr))
        return store_E_InvalidParameter;

    sal_uInt8 * dst_hi = dst_lo + nBytes;
    if (!(dst_lo < dst_hi))
        return (dst_lo > dst_hi) ? store_E_InvalidParameter : store_E_None;

    OSL_PRECOND(!(nOffset == STORE_PAGE_NULL), "store::ILockBytes::readAt(): invalid Offset");
    if (nOffset == STORE_PAGE_NULL)
        return store_E_CantSeek;

    sal_uInt64 const src_size = static_cast<sal_uInt64>(nOffset) + nBytes;
    if (src_size > SAL_MAX_UINT32)
        return store_E_CantSeek;

    return readAt_Impl (nOffset, dst_lo, nBytes);
}

storeError ILockBytes::writeAt (sal_uInt32 nOffset, void const * pBuffer, sal_uInt32 nBytes)
{
    // [SECURITY:ValInput]
    sal_uInt8 const * src_lo = static_cast<sal_uInt8 const*>(pBuffer);
    if (!(src_lo != nullptr))
        return store_E_InvalidParameter;

    sal_uInt8 const * src_hi = src_lo + nBytes;
    if (!(src_lo < src_hi))
        return (src_lo > src_hi) ? store_E_InvalidParameter : store_E_None;

    OSL_PRECOND(!(nOffset == STORE_PAGE_NULL), "store::ILockBytes::writeAt(): invalid Offset");
    if (nOffset == STORE_PAGE_NULL)
        return store_E_CantSeek;

    sal_uInt64 const dst_size = static_cast<sal_uInt64>(nOffset) + nBytes;
    if (dst_size > SAL_MAX_UINT32)
        return store_E_CantSeek;

    return writeAt_Impl (nOffset, src_lo, nBytes);
}

storeError ILockBytes::getSize (sal_uInt32 & rnSize)
{
    rnSize = 0;
    return getSize_Impl (rnSize);
}

storeError ILockBytes::setSize (sal_uInt32 nSize)
{
    return setSize_Impl (nSize);
}

storeError ILockBytes::flush()
{
    return flush_Impl();
}

/*========================================================================
 *
 * FileLockBytes implementation.
 *
 *======================================================================*/
namespace store
{

struct FileHandle
{
    oslFileHandle m_handle;

    FileHandle() : m_handle(nullptr) {}

    bool operator != (FileHandle const & rhs)
    {
        return (m_handle != rhs.m_handle);
    }

    static storeError errorFromNative (oslFileError eErrno)
    {
        switch (eErrno)
        {
        case osl_File_E_None:
            return store_E_None;

        case osl_File_E_NOENT:
            return store_E_NotExists;

        case osl_File_E_ACCES:
        case osl_File_E_PERM:
            return store_E_AccessViolation;

        case osl_File_E_AGAIN:
        case osl_File_E_DEADLK:
            return store_E_LockingViolation;

        case osl_File_E_BADF:
            return store_E_InvalidHandle;

        case osl_File_E_INVAL:
            return store_E_InvalidParameter;

        case osl_File_E_NOMEM:
            return store_E_OutOfMemory;

        case osl_File_E_NOSPC:
            return store_E_OutOfSpace;

        case osl_File_E_OVERFLOW:
            return store_E_CantSeek;

        default:
            return store_E_Unknown;
        }
    }

    static sal_uInt32 modeToNative (storeAccessMode eAccessMode)
    {
        sal_uInt32 nFlags = 0;
        switch (eAccessMode)
        {
        case store_AccessCreate:
        case store_AccessReadCreate:
            nFlags |= osl_File_OpenFlag_Create;
            SAL_FALLTHROUGH;
        case store_AccessReadWrite:
            nFlags |= osl_File_OpenFlag_Write;
            SAL_FALLTHROUGH;
        case store_AccessReadOnly:
            nFlags |= osl_File_OpenFlag_Read;
            break;
        default:
            OSL_PRECOND(false, "store::FileHandle: unknown storeAccessMode");
        }
        return nFlags;
    }

    storeError initialize (rtl_uString * pFilename, storeAccessMode eAccessMode)
    {
        // Verify arguments.
        sal_uInt32 nFlags = modeToNative (eAccessMode);
        if (!pFilename || !nFlags)
            return store_E_InvalidParameter;

        // Convert into FileUrl.
        OUString aFileUrl;
        if (osl_getFileURLFromSystemPath (pFilename, &(aFileUrl.pData)) != osl_File_E_None)
        {
            // Not system path. Assume file url.
            rtl_uString_assign (&(aFileUrl.pData), pFilename);
        }
        if (!aFileUrl.startsWith("file://"))
        {
            // Not file url. Assume relative path.
            OUString aCwdUrl;
            (void) osl_getProcessWorkingDir (&(aCwdUrl.pData));

            // Absolute file url.
            (void) osl_getAbsoluteFileURL (aCwdUrl.pData, aFileUrl.pData, &(aFileUrl.pData));
        }

        // Acquire handle.
        oslFileError result = osl_openFile (aFileUrl.pData, &m_handle, nFlags);
        if (result == osl_File_E_EXIST)
        {
            // Already existing (O_CREAT | O_EXCL).
            result = osl_openFile (aFileUrl.pData, &m_handle, osl_File_OpenFlag_Read | osl_File_OpenFlag_Write);
            if ((result == osl_File_E_None) && (eAccessMode == store_AccessCreate))
            {
                // Truncate existing file.
                result = osl_setFileSize (m_handle, 0);
            }
        }
        if (result != osl_File_E_None)
            return errorFromNative(result);
        return store_E_None;
    }

    /** @see FileLockBytes destructor
     */
    static void closeFile (oslFileHandle hFile)
    {
        (void) osl_closeFile (hFile);
    }

    /** @see ResourceHolder<T>::destructor_type
     */
    struct CloseFile
    {
        void operator()(FileHandle & rFile) const
        {
            // Release handle.
            closeFile (rFile.m_handle);
            rFile.m_handle = nullptr;
        }
    };
    typedef CloseFile destructor_type;
};

class FileLockBytes :
    public store::OStoreObject,
    public store::ILockBytes,
    private boost::noncopyable
{
    /** Representation.
     */
    oslFileHandle                         m_hFile;
    sal_uInt32                            m_nSize;
    rtl::Reference< PageData::Allocator > m_xAllocator;

    storeError initSize_Impl (sal_uInt32 & rnSize);

    /** ILockBytes implementation.
     */
    virtual storeError initialize_Impl (rtl::Reference< PageData::Allocator > & rxAllocator, sal_uInt16 nPageSize) override;

    virtual storeError readPageAt_Impl (PageHolder & rPage, sal_uInt32 nOffset) override;
    virtual storeError writePageAt_Impl (PageHolder const & rPage, sal_uInt32 nOffset) override;

    virtual storeError readAt_Impl (sal_uInt32 nOffset, void * pBuffer, sal_uInt32 nBytes) override;
    virtual storeError writeAt_Impl (sal_uInt32 nOffset, void const * pBuffer, sal_uInt32 nBytes) override;

    virtual storeError getSize_Impl (sal_uInt32 & rnSize) override;
    virtual storeError setSize_Impl (sal_uInt32 nSize) override;

    virtual storeError flush_Impl() override;

public:
    /** Construction.
     */
    explicit FileLockBytes (FileHandle & rFile);

protected:
    /** Destruction.
     */
    virtual ~FileLockBytes();
};

} // namespace store

FileLockBytes::FileLockBytes (FileHandle & rFile)
    : m_hFile (rFile.m_handle), m_nSize (SAL_MAX_UINT32), m_xAllocator()
{
}

FileLockBytes::~FileLockBytes()
{
    FileHandle::closeFile (m_hFile);
}

storeError FileLockBytes::initSize_Impl (sal_uInt32 & rnSize)
{
    /* osl_getFileSize() uses slow 'fstat(h, &size)',
     * instead of fast 'size = lseek(h, 0, SEEK_END)'.
     * so, init size here, and track changes.
     */
    sal_uInt64 uSize = 0;
    oslFileError result = osl_getFileSize (m_hFile, &uSize);
    if (result != osl_File_E_None)
        return FileHandle::errorFromNative(result);
    if (uSize > SAL_MAX_UINT32)
        return store_E_CantSeek;

    rnSize = sal::static_int_cast<sal_uInt32>(uSize);
    return store_E_None;
}

storeError FileLockBytes::initialize_Impl (rtl::Reference< PageData::Allocator > & rxAllocator, sal_uInt16 nPageSize)
{
    storeError result = initSize_Impl (m_nSize);
    if (result != store_E_None)
        return result;

    result = PageData::Allocator::createInstance (rxAllocator, nPageSize);
    if (result != store_E_None)
        return result;

    // @see readPageAt_Impl().
    m_xAllocator = rxAllocator;
    return store_E_None;
}

storeError FileLockBytes::readPageAt_Impl (PageHolder & rPage, sal_uInt32 nOffset)
{
    if (m_xAllocator.is())
    {
        PageHolder page (m_xAllocator->construct<PageData>(), m_xAllocator);
        page.swap (rPage);
    }

    if (!m_xAllocator.is())
        return store_E_InvalidAccess;
    if (!rPage.get())
        return store_E_OutOfMemory;

    PageData * pagedata = rPage.get();
    return readAt_Impl (nOffset, pagedata, pagedata->size());
}

storeError FileLockBytes::writePageAt_Impl (PageHolder const & rPage, sal_uInt32 nOffset)
{
    PageData const * pagedata = rPage.get();
    OSL_PRECOND(pagedata != nullptr, "contract violation");
    return writeAt_Impl (nOffset, pagedata, pagedata->size());
}

storeError FileLockBytes::readAt_Impl (sal_uInt32 nOffset, void * pBuffer, sal_uInt32 nBytes)
{
    sal_uInt64 nDone = 0;
    oslFileError result = osl_readFileAt (m_hFile, nOffset, pBuffer, nBytes, &nDone);
    if (result != osl_File_E_None)
        return FileHandle::errorFromNative(result);
    if (nDone != nBytes)
        return (nDone != 0) ? store_E_CantRead : store_E_NotExists;
    return store_E_None;
}

storeError FileLockBytes::writeAt_Impl (sal_uInt32 nOffset, void const * pBuffer, sal_uInt32 nBytes)
{
    sal_uInt64 nDone = 0;
    oslFileError result = osl_writeFileAt (m_hFile, nOffset, pBuffer, nBytes, &nDone);
    if (result != osl_File_E_None)
        return FileHandle::errorFromNative(result);
    if (nDone != nBytes)
        return store_E_CantWrite;

    sal_uInt64 const uSize = nOffset + nBytes;
    OSL_PRECOND(uSize < SAL_MAX_UINT32, "store::ILockBytes::writeAt() contract violation");
    if (uSize > m_nSize)
        m_nSize = sal::static_int_cast<sal_uInt32>(uSize);
    return store_E_None;
}

storeError FileLockBytes::getSize_Impl (sal_uInt32 & rnSize)
{
    rnSize = m_nSize;
    return store_E_None;
}

storeError FileLockBytes::setSize_Impl (sal_uInt32 nSize)
{
    oslFileError result = osl_setFileSize (m_hFile, nSize);
    if (result != osl_File_E_None)
        return FileHandle::errorFromNative(result);

    m_nSize = nSize;
    return store_E_None;
}

storeError FileLockBytes::flush_Impl()
{
    oslFileError result = osl_syncFile (m_hFile);
    if (result != osl_File_E_None)
        return FileHandle::errorFromNative(result);
    return store_E_None;
}

/*========================================================================
 *
 * MappedLockBytes implementation.
 *
 *======================================================================*/
namespace store
{

struct FileMapping
{
    sal_uInt8 * m_pAddr;
    sal_uInt32  m_nSize;
    oslFileHandle m_hFile;

    FileMapping() : m_pAddr(nullptr), m_nSize(0), m_hFile(nullptr) {}

    bool operator != (FileMapping const & rhs) const
    {
        return ((m_pAddr != rhs.m_pAddr) || (m_nSize != rhs.m_nSize));
    }

    oslFileError initialize (oslFileHandle hFile)
    {
        // Determine mapping size.
        sal_uInt64   uSize  = 0;
        oslFileError result = osl_getFileSize (hFile, &uSize);
        if (result != osl_File_E_None)
            return result;

        // [SECURITY:IntOver]
        if (uSize > SAL_MAX_UINT32)
            return osl_File_E_OVERFLOW;
        m_nSize = sal::static_int_cast<sal_uInt32>(uSize);

        m_hFile = hFile;

        // Acquire mapping.
        return osl_mapFile (hFile, reinterpret_cast<void**>(&m_pAddr), m_nSize, 0, osl_File_MapFlag_RandomAccess);
    }

    /** @see MappedLockBytes::destructor.
     */
    static void unmapFile (oslFileHandle hFile, sal_uInt8 * pAddr, sal_uInt32 nSize)
    {
        (void) osl_unmapMappedFile (hFile, pAddr, nSize);
        (void) osl_closeFile (hFile);
    }

    /** @see ResourceHolder<T>::destructor_type
     */
    struct UnmapFile
    {
        void operator ()(FileMapping & rMapping) const
        {
            // Release mapping.
            unmapFile (rMapping.m_hFile, rMapping.m_pAddr, rMapping.m_nSize);
            rMapping.m_pAddr = nullptr, rMapping.m_nSize = 0;
        }
    };
    typedef UnmapFile destructor_type;
};

class MappedLockBytes :
    public store::OStoreObject,
    public store::PageData::Allocator,
    public store::ILockBytes,
    private boost::noncopyable
{
    /** Representation.
     */
    sal_uInt8 * m_pData;
    sal_uInt32  m_nSize;
    sal_uInt16  m_nPageSize;
    oslFileHandle m_hFile;

    /** PageData::Allocator implementation.
     */
    virtual void allocate_Impl (void ** ppPage, sal_uInt16 * pnSize) override;
    virtual void deallocate_Impl (void * pPage) override;

    /** ILockBytes implementation.
     */
    virtual storeError initialize_Impl (rtl::Reference< PageData::Allocator > & rxAllocator, sal_uInt16 nPageSize) override;

    virtual storeError readPageAt_Impl (PageHolder & rPage, sal_uInt32 nOffset) override;
    virtual storeError writePageAt_Impl (PageHolder const & rPage, sal_uInt32 nOffset) override;

    virtual storeError readAt_Impl  (sal_uInt32 nOffset, void * pBuffer, sal_uInt32 nBytes) override;
    virtual storeError writeAt_Impl (sal_uInt32 nOffset, const void * pBuffer, sal_uInt32 nBytes) override;

    virtual storeError getSize_Impl (sal_uInt32 & rnSize) override;
    virtual storeError setSize_Impl (sal_uInt32 nSize) override;

    virtual storeError flush_Impl() override;

public:
    /** Construction.
     */
    explicit MappedLockBytes (FileMapping & rMapping);

protected:
    /* Destruction.
     */
    virtual ~MappedLockBytes();
};

} // namespace store

MappedLockBytes::MappedLockBytes (FileMapping & rMapping)
    : m_pData (rMapping.m_pAddr), m_nSize (rMapping.m_nSize), m_nPageSize(0), m_hFile (rMapping.m_hFile)
{
}

MappedLockBytes::~MappedLockBytes()
{
    FileMapping::unmapFile (m_hFile, m_pData, m_nSize);
}

void MappedLockBytes::allocate_Impl (void ** ppPage, sal_uInt16 * pnSize)
{
    OSL_PRECOND((ppPage != nullptr) && (pnSize != nullptr), "contract violation");
    if ((ppPage != nullptr) && (pnSize != nullptr))
        *ppPage = nullptr, *pnSize = m_nPageSize;
}

void MappedLockBytes::deallocate_Impl (void * pPage)
{
    OSL_PRECOND((m_pData <= pPage) && (pPage < m_pData + m_nSize), "contract violation");
    (void)pPage; // UNUSED
}

storeError MappedLockBytes::initialize_Impl (rtl::Reference< PageData::Allocator > & rxAllocator, sal_uInt16 nPageSize)
{
    rxAllocator = this;
    m_nPageSize = nPageSize;
    return store_E_None;
}

storeError MappedLockBytes::readPageAt_Impl (PageHolder & rPage, sal_uInt32 nOffset)
{
    sal_uInt8 * src_lo = m_pData + nOffset;
    if ((m_pData > src_lo) || (src_lo >= m_pData + m_nSize))
        return store_E_NotExists;

    sal_uInt8 * src_hi = src_lo + m_nPageSize;
    if ((m_pData > src_hi) || (src_hi > m_pData + m_nSize))
        return store_E_CantRead;

    PageHolder page (reinterpret_cast< PageData* >(src_lo), static_cast< PageData::Allocator* >(this));
    page.swap (rPage);

    return store_E_None;
}

storeError MappedLockBytes::writePageAt_Impl (PageHolder const & /*rPage*/, sal_uInt32 /*nOffset*/)
{
    return store_E_AccessViolation;
}

storeError MappedLockBytes::readAt_Impl (sal_uInt32 nOffset, void * pBuffer, sal_uInt32 nBytes)
{
    sal_uInt8 const * src_lo = m_pData + nOffset;
    if ((m_pData > src_lo) || (src_lo >= m_pData + m_nSize))
        return store_E_NotExists;

    sal_uInt8 const * src_hi = src_lo + nBytes;
    if ((m_pData > src_hi) || (src_hi > m_pData + m_nSize))
        return store_E_CantRead;

    memcpy (pBuffer, src_lo, (src_hi - src_lo));
    return store_E_None;
}

storeError MappedLockBytes::writeAt_Impl (sal_uInt32 /*nOffset*/, void const * /*pBuffer*/, sal_uInt32 /*nBytes*/)
{
    return store_E_AccessViolation;
}

storeError MappedLockBytes::getSize_Impl (sal_uInt32 & rnSize)
{
    rnSize = m_nSize;
    return store_E_None;
}

storeError MappedLockBytes::setSize_Impl (sal_uInt32 /*nSize*/)
{
    return store_E_AccessViolation;
}

storeError MappedLockBytes::flush_Impl()
{
    return store_E_None;
}

/*========================================================================
 *
 * MemoryLockBytes implementation.
 *
 *======================================================================*/
namespace store
{

class MemoryLockBytes :
    public store::OStoreObject,
    public store::ILockBytes,
    private boost::noncopyable
{
    /** Representation.
     */
    sal_uInt8 * m_pData;
    sal_uInt32  m_nSize;
    rtl::Reference< PageData::Allocator > m_xAllocator;

    /** ILockBytes implementation.
     */
    virtual storeError initialize_Impl (rtl::Reference< PageData::Allocator > & rxAllocator, sal_uInt16 nPageSize) override;

    virtual storeError readPageAt_Impl (PageHolder & rPage, sal_uInt32 nOffset) override;
    virtual storeError writePageAt_Impl (PageHolder const & rPage, sal_uInt32 nOffset) override;

    virtual storeError readAt_Impl  (sal_uInt32 nOffset, void * pBuffer, sal_uInt32 nBytes) override;
    virtual storeError writeAt_Impl (sal_uInt32 nOffset, const void * pBuffer, sal_uInt32 nBytes) override;

    virtual storeError getSize_Impl (sal_uInt32 & rnSize) override;
    virtual storeError setSize_Impl (sal_uInt32 nSize) override;

    virtual storeError flush_Impl() override;

public:
    /** Construction.
     */
    MemoryLockBytes();

protected:
    /** Destruction.
     */
    virtual ~MemoryLockBytes();
};

} // namespace store

MemoryLockBytes::MemoryLockBytes()
    : m_pData (nullptr), m_nSize (0), m_xAllocator()
{}

MemoryLockBytes::~MemoryLockBytes()
{
    rtl_freeMemory (m_pData);
}

storeError MemoryLockBytes::initialize_Impl (rtl::Reference< PageData::Allocator > & rxAllocator, sal_uInt16 nPageSize)
{
    storeError result = PageData::Allocator::createInstance (rxAllocator, nPageSize);
    if (result == store_E_None)
    {
        // @see readPageAt_Impl().
        m_xAllocator = rxAllocator;
    }
    return result;
}

storeError MemoryLockBytes::readPageAt_Impl (PageHolder & rPage, sal_uInt32 nOffset)
{
    if (m_xAllocator.is())
    {
        PageHolder page (m_xAllocator->construct<PageData>(), m_xAllocator);
        page.swap (rPage);
    }

    if (!m_xAllocator.is())
        return store_E_InvalidAccess;
    if (!rPage.get())
        return store_E_OutOfMemory;

    PageData * pagedata = rPage.get();
    return readAt_Impl (nOffset, pagedata, pagedata->size());
}

storeError MemoryLockBytes::writePageAt_Impl (PageHolder const & rPage, sal_uInt32 nOffset)
{
    PageData const * pagedata = rPage.get();
    OSL_PRECOND(!(pagedata == nullptr), "contract violation");
    return writeAt_Impl (nOffset, pagedata, pagedata->size());
}

storeError MemoryLockBytes::readAt_Impl (sal_uInt32 nOffset, void * pBuffer, sal_uInt32 nBytes)
{
    sal_uInt8 const * src_lo = m_pData + nOffset;
    if ((m_pData > src_lo) || (src_lo >= m_pData + m_nSize))
        return store_E_NotExists;

    sal_uInt8 const * src_hi = src_lo + nBytes;
    if ((m_pData > src_hi) || (src_hi > m_pData + m_nSize))
        return store_E_CantRead;

    memcpy (pBuffer, src_lo, (src_hi - src_lo));
    return store_E_None;
}

storeError MemoryLockBytes::writeAt_Impl (sal_uInt32 nOffset, const void * pBuffer, sal_uInt32 nBytes)
{
    sal_uInt64 const dst_size = nOffset + nBytes;
    OSL_PRECOND(dst_size < SAL_MAX_UINT32, "store::ILockBytes::writeAt() contract violation");
    if (dst_size > m_nSize)
    {
        storeError eErrCode = setSize_Impl (sal::static_int_cast<sal_uInt32>(dst_size));
        if (eErrCode != store_E_None)
            return eErrCode;
    }
    SAL_WARN_IF(dst_size > m_nSize, "store", "store::MemoryLockBytes::setSize_Impl() contract violation");

    sal_uInt8 * dst_lo = m_pData + nOffset;
    if (dst_lo >= m_pData + m_nSize)
        return store_E_CantSeek;

    sal_uInt8 * dst_hi = dst_lo + nBytes;
    if (dst_hi > m_pData + m_nSize)
        return store_E_CantWrite;

    memcpy (dst_lo, pBuffer, (dst_hi - dst_lo));
    return store_E_None;
}

storeError MemoryLockBytes::getSize_Impl (sal_uInt32 & rnSize)
{
    rnSize = m_nSize;
    return store_E_None;
}

storeError MemoryLockBytes::setSize_Impl (sal_uInt32 nSize)
{
    if (nSize != m_nSize)
    {
        sal_uInt8 * pData = static_cast<sal_uInt8*>(rtl_reallocateMemory (m_pData, nSize));
        if (pData != nullptr)
        {
            if (nSize > m_nSize)
                memset (pData + m_nSize, 0, sal::static_int_cast<size_t>(nSize - m_nSize));
        }
        else
        {
            if (nSize != 0)
                return store_E_OutOfMemory;
        }
        m_pData = pData, m_nSize = nSize;
    }
    return store_E_None;
}

storeError MemoryLockBytes::flush_Impl()
{
    return store_E_None;
}

/*========================================================================
 *
 * ILockBytes factory implementations.
 *
 *======================================================================*/
namespace store
{

template< class T > struct ResourceHolder
{
    typedef typename T::destructor_type destructor_type;

    T m_value;

    explicit ResourceHolder (T const & value = T()) : m_value (value) {}
    ~ResourceHolder() { reset(); }

    T & get() { return m_value; }
    T const & get() const { return m_value; }

    void set (T const & value) { m_value = value; }
    void reset (T const & value = T())
    {
        T tmp (m_value);
        if (tmp != value)
            destructor_type()(tmp);
        set (value);
    }
    T release()
    {
        T tmp (m_value);
        set (T());
        return tmp;
    }

    ResourceHolder & operator= (ResourceHolder & rhs)
    {
        reset (rhs.release());
        return *this;
    }
};

storeError
FileLockBytes_createInstance (
    rtl::Reference< ILockBytes > & rxLockBytes,
    rtl_uString *                  pFilename,
    storeAccessMode                eAccessMode
)
{
    // Acquire file handle.
    ResourceHolder<FileHandle> xFile;
    storeError result = xFile.get().initialize (pFilename, eAccessMode);
    if (result != store_E_None)
        return result;

    if (eAccessMode == store_AccessReadOnly)
    {
        ResourceHolder<FileMapping> xMapping;
        if (xMapping.get().initialize (xFile.get().m_handle) == osl_File_E_None)
        {
            rxLockBytes = new MappedLockBytes (xMapping.get());
            if (!rxLockBytes.is())
                return store_E_OutOfMemory;
            (void) xFile.release();
            (void) xMapping.release();
        }
    }
    if (!rxLockBytes.is())
    {
        rxLockBytes = new FileLockBytes (xFile.get());
        if (!rxLockBytes.is())
            return store_E_OutOfMemory;
        (void) xFile.release();
    }

    return store_E_None;
}

storeError
MemoryLockBytes_createInstance (
    rtl::Reference< ILockBytes > & rxLockBytes
)
{
    rxLockBytes = new MemoryLockBytes();
    if (!rxLockBytes.is())
        return store_E_OutOfMemory;

    return store_E_None;
}

} // namespace store

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
