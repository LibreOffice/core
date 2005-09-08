/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: store.inl,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:40:01 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#define _STORE_STORE_INL_ "$Revision: 1.3 $"

/*========================================================================
 *
 * OStoreStream implementation.
 *
 *======================================================================*/
inline OStoreStream::OStoreStream (void) SAL_THROW(())
    : m_hImpl (0)
{
}

inline OStoreStream::~OStoreStream (void) SAL_THROW(())
{
    if (m_hImpl)
        store_releaseHandle (m_hImpl);
}

inline OStoreStream::OStoreStream (
    const OStoreStream& rOther) SAL_THROW(())
    : m_hImpl (rOther.m_hImpl)
{
    if (m_hImpl)
        store_acquireHandle (m_hImpl);
}

inline OStoreStream& OStoreStream::operator= (
    const OStoreStream& rOther) SAL_THROW(())
{
    if (m_hImpl)
        store_releaseHandle (m_hImpl);
    m_hImpl = rOther.m_hImpl;
    if (m_hImpl)
        store_acquireHandle (m_hImpl);
    return *this;
}

inline OStoreStream::OStoreStream (
    storeStreamHandle Handle) SAL_THROW(())
    : m_hImpl (Handle)
{
    if (m_hImpl)
        store_acquireHandle (m_hImpl);
}

inline OStoreStream::operator storeStreamHandle (void) const SAL_THROW(())
{
    return m_hImpl;
}

inline sal_Bool OStoreStream::isValid (void) const SAL_THROW(())
{
    return (!!m_hImpl);
}

inline storeError OStoreStream::create (
    storeFileHandle      hFile,
    const rtl::OUString &rPath,
    const rtl::OUString &rName,
    storeAccessMode      eMode) SAL_THROW(())
{
    if (m_hImpl)
    {
        store_releaseHandle (m_hImpl);
        m_hImpl = 0;
    }
    return store_openStream (
        hFile, rPath.pData, rName.pData, eMode, &m_hImpl);
}

inline void OStoreStream::close (void) SAL_THROW(())
{
    if (m_hImpl)
    {
        store_closeStream (m_hImpl);
        m_hImpl = 0;
    }
}

inline storeError OStoreStream::readAt (
    sal_uInt32  nOffset,
    void       *pBuffer,
    sal_uInt32  nBytes,
    sal_uInt32 &rnDone) SAL_THROW(())
{
    if (!m_hImpl)
        return store_E_InvalidHandle;

    return store_readStream (
        m_hImpl, nOffset, pBuffer, nBytes, &rnDone);
}

inline storeError OStoreStream::writeAt (
    sal_uInt32  nOffset,
    const void *pBuffer,
    sal_uInt32  nBytes,
    sal_uInt32 &rnDone) SAL_THROW(())
{
    if (!m_hImpl)
        return store_E_InvalidHandle;

    return store_writeStream (
        m_hImpl, nOffset, pBuffer, nBytes, &rnDone);
}

inline storeError OStoreStream::flush (void) const SAL_THROW(())
{
    if (!m_hImpl)
        return store_E_InvalidHandle;

    return store_flushStream (m_hImpl);
}

inline storeError OStoreStream::getSize (
    sal_uInt32 &rnSize) const SAL_THROW(())
{
    if (!m_hImpl)
        return store_E_InvalidHandle;

    return store_getStreamSize (m_hImpl, &rnSize);
}

inline storeError OStoreStream::setSize (
    sal_uInt32 nSize) SAL_THROW(())
{
    if (!m_hImpl)
        return store_E_InvalidHandle;

    return store_setStreamSize (m_hImpl, nSize);
}

/*========================================================================
 *
 * OStoreDirectory implementation.
 *
 *======================================================================*/
inline OStoreDirectory::OStoreDirectory (void) SAL_THROW(())
    : m_hImpl (0)
{
}

inline OStoreDirectory::~OStoreDirectory (void) SAL_THROW(())
{
    if (m_hImpl)
        store_releaseHandle (m_hImpl);
}

inline OStoreDirectory::OStoreDirectory (
    const OStoreDirectory& rOther) SAL_THROW(())
    : m_hImpl (rOther.m_hImpl)
{
    if (m_hImpl)
        store_acquireHandle (m_hImpl);
}

inline OStoreDirectory& OStoreDirectory::operator= (
    const OStoreDirectory& rOther) SAL_THROW(())
{
    if (m_hImpl)
        store_releaseHandle (m_hImpl);
    m_hImpl = rOther.m_hImpl;
    if (m_hImpl)
        store_acquireHandle (m_hImpl);
    return *this;
}

inline OStoreDirectory::OStoreDirectory (
    storeDirectoryHandle Handle) SAL_THROW(())
    : m_hImpl (Handle)
{
    if (m_hImpl)
        store_acquireHandle (m_hImpl);
}

inline OStoreDirectory::operator storeDirectoryHandle(void) const SAL_THROW(())
{
    return m_hImpl;
}

inline sal_Bool OStoreDirectory::isValid (void) const SAL_THROW(())
{
    return (!!m_hImpl);
}

inline storeError OStoreDirectory::create (
    storeFileHandle      hFile,
    const rtl::OUString &rPath,
    const rtl::OUString &rName,
    storeAccessMode      eMode) SAL_THROW(())
{
    if (m_hImpl)
    {
        store_releaseHandle (m_hImpl);
        m_hImpl = 0;
    }
    return store_openDirectory (
        hFile, rPath.pData, rName.pData, eMode, &m_hImpl);
}

inline void OStoreDirectory::close (void) SAL_THROW(())
{
    if (m_hImpl)
    {
        store_closeDirectory (m_hImpl);
        m_hImpl = 0;
    }
}

inline storeError OStoreDirectory::first (iterator& it) SAL_THROW(())
{
    if (!m_hImpl)
        return store_E_InvalidHandle;

    return store_findFirst (m_hImpl, &it);
}

inline storeError OStoreDirectory::next (iterator& it) SAL_THROW(())
{
    if (!m_hImpl)
        return store_E_InvalidHandle;

    return store_findNext (m_hImpl, &it);
}

inline storeError OStoreDirectory::travel (traveller& rTraveller) const
{
    storeError eErrCode = store_E_InvalidHandle;
    if (m_hImpl)
    {
        iterator it;
        eErrCode = store_findFirst (m_hImpl, &it);
        while ((eErrCode == store_E_None) && rTraveller.visit(it))
            eErrCode = store_findNext (m_hImpl, &it);
    }
    return eErrCode;
}

/*========================================================================
 *
 * OStoreFile implementation.
 *
 *======================================================================*/
inline OStoreFile::OStoreFile (void) SAL_THROW(())
    : m_hImpl (0)
{
}

inline OStoreFile::~OStoreFile (void) SAL_THROW(())
{
    if (m_hImpl)
        store_releaseHandle (m_hImpl);
}

inline OStoreFile::OStoreFile (
    const OStoreFile& rOther) SAL_THROW(())
    : m_hImpl (rOther.m_hImpl)
{
    if (m_hImpl)
        store_acquireHandle (m_hImpl);
}

inline OStoreFile& OStoreFile::operator= (
    const OStoreFile& rOther) SAL_THROW(())
{
    if (m_hImpl)
        store_releaseHandle (m_hImpl);
    m_hImpl = rOther.m_hImpl;
    if (m_hImpl)
        store_acquireHandle (m_hImpl);
    return *this;
}

inline OStoreFile::OStoreFile (
    storeFileHandle Handle) SAL_THROW(())
    : m_hImpl (Handle)
{
    if (m_hImpl)
        store_acquireHandle (m_hImpl);
}

inline OStoreFile::operator storeFileHandle (void) const SAL_THROW(())
{
    return m_hImpl;
}

inline sal_Bool OStoreFile::isValid (void) const SAL_THROW(())
{
    return (!!m_hImpl);
}

inline storeError OStoreFile::create (
    const rtl::OUString &rFilename,
    storeAccessMode      eAccessMode,
    sal_uInt16           nPageSize) SAL_THROW(())
{
    if (m_hImpl)
    {
        store_releaseHandle (m_hImpl);
        m_hImpl = 0;
    }
    return store_openFile (rFilename.pData, eAccessMode, nPageSize, &m_hImpl);
}

inline storeError OStoreFile::createInMemory (
    sal_uInt16 nPageSize) SAL_THROW(())
{
    if (m_hImpl)
    {
        store_releaseHandle (m_hImpl);
        m_hImpl = 0;
    }
    return store_createMemoryFile (nPageSize, &m_hImpl);
}

inline void OStoreFile::close (void) SAL_THROW(())
{
    if (m_hImpl)
    {
        store_closeFile (m_hImpl);
        m_hImpl = 0;
    }
}

inline storeError OStoreFile::flush (void) const SAL_THROW(())
{
    if (!m_hImpl)
        return store_E_InvalidHandle;

    return store_flushFile (m_hImpl);
}

inline storeError OStoreFile::getRefererCount (
    sal_uInt32 &rnRefCount) const SAL_THROW(())
{
    if (!m_hImpl)
        return store_E_InvalidHandle;

    return store_getFileRefererCount (m_hImpl, &rnRefCount);
}

inline storeError OStoreFile::getSize (
    sal_uInt32 &rnSize) const SAL_THROW(())
{
    if (!m_hImpl)
        return store_E_InvalidHandle;

    return store_getFileSize (m_hImpl, &rnSize);
}

inline storeError OStoreFile::attrib (
    const rtl::OUString &rPath,
    const rtl::OUString &rName,
    sal_uInt32           nMask1,
    sal_uInt32           nMask2,
    sal_uInt32          &rnAttrib) SAL_THROW(())
{
    if (!m_hImpl)
        return store_E_InvalidHandle;

    return store_attrib (
        m_hImpl, rPath.pData, rName.pData, nMask1, nMask2, &rnAttrib);
}

inline storeError OStoreFile::attrib (
    const rtl::OUString &rPath,
    const rtl::OUString &rName,
    sal_uInt32           nMask1,
    sal_uInt32           nMask2) SAL_THROW(())
{
    if (!m_hImpl)
        return store_E_InvalidHandle;

    return store_attrib (
        m_hImpl, rPath.pData, rName.pData, nMask1, nMask2, NULL);
}

inline storeError OStoreFile::link (
    const rtl::OUString &rSrcPath, const rtl::OUString &rSrcName,
    const rtl::OUString &rDstPath, const rtl::OUString &rDstName) SAL_THROW(())
{
    if (!m_hImpl)
        return store_E_InvalidHandle;

    return store_link (
        m_hImpl,
        rSrcPath.pData, rSrcName.pData,
        rDstPath.pData, rDstName.pData);
}

inline storeError OStoreFile::symlink (
    const rtl::OUString &rSrcPath, const rtl::OUString &rSrcName,
    const rtl::OUString &rDstPath, const rtl::OUString &rDstName) SAL_THROW(())
{
    if (!m_hImpl)
        return store_E_InvalidHandle;

    return store_symlink (
        m_hImpl,
        rSrcPath.pData, rSrcName.pData,
        rDstPath.pData, rDstName.pData);
}

inline storeError OStoreFile::rename (
    const rtl::OUString &rSrcPath, const rtl::OUString &rSrcName,
    const rtl::OUString &rDstPath, const rtl::OUString &rDstName) SAL_THROW(())
{
    if (!m_hImpl)
        return store_E_InvalidHandle;

    return store_rename (
        m_hImpl,
        rSrcPath.pData, rSrcName.pData,
        rDstPath.pData, rDstName.pData);
}

inline storeError OStoreFile::remove (
    const rtl::OUString &rPath, const rtl::OUString &rName) SAL_THROW(())
{
    if (!m_hImpl)
        return store_E_InvalidHandle;

    return store_remove (m_hImpl, rPath.pData, rName.pData);
}

