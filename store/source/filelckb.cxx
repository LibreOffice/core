/*************************************************************************
 *
 *  $RCSfile: filelckb.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: mhu $ $Date: 2001-03-15 15:34:05 $
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

#define _STORE_FILELCKB_CXX_ "$Revision: 1.4 $"

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _SAL_MACROS_H_
#include <sal/macros.h>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _OSL_FILE_H_
#include <osl/file.h>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif

#ifndef _STORE_OBJECT_HXX_
#include <store/object.hxx>
#endif
#ifndef _STORE_LOCKBYTE_HXX_
#include <store/lockbyte.hxx>
#endif
#ifndef _STORE_FILELCKB_HXX_
#include <store/filelckb.hxx>
#endif

#ifndef _STORE_TYPES_H_
#include <store/types.h>
#endif

using namespace store;

/*========================================================================
 *
 * OFileLockBytes internals.
 *
 *======================================================================*/
#ifdef DEBUG
#define inline static
#endif /* DEBUG */

/*
 * __store_memcpy.
 */
#include <string.h>
inline void __store_memcpy (void *dst, const void *src, sal_uInt32 n)
{
    ::memcpy (dst, src, n);
}

/*
 * __store_errcode_mapping.
 */
struct __store_errcode_mapping_st
{
    sal_uInt32 m_nErrno;
    storeError m_nCode;
};

static storeError __store_errnoToErrCode (sal_uInt32 nErrno);
#define ERROR_FROM_NATIVE(e) __store_errnoToErrCode((sal_uInt32)(e))

/*
 * __store_file (basic file I/O; platform specific; inline).
 */
#define store_File_OpenRead     0x01L
#define store_File_OpenWrite    0x02L
#define store_File_OpenNoBuffer 0x04L
#define store_File_OpenNoCreate 0x08L
#define store_File_OpenTruncate 0x10L

#ifdef __STORE_IO_NATIVE
#undef __STORE_IO_NATIVE
#endif

#ifdef SAL_OS2
#include <fileos2.cxx>
#endif /* SAL_OS2 */

#ifdef SAL_UNX
#include <fileunx.cxx>
#endif /* SAL_UNX */

#ifdef SAL_W32
#include <filew32.cxx>
#endif /* SAL_W32 */

#ifndef __STORE_IO_NATIVE
#include <filestd.cxx>
#endif  /* !_STORE_IO_NATIVE */

/*
 * __store_errnoToErrCode.
 */
static storeError __store_errnoToErrCode (sal_uInt32 nErrno)
{
    int i, n = sizeof(__store_errcode_map) / sizeof(__store_errcode_map[0]);
    for (i = 0; i < n; i++)
    {
        if (__store_errcode_map[i].m_nErrno == nErrno)
            return __store_errcode_map[i].m_nCode;
    }
    return store_E_Unknown;
}

#ifdef DEBUG
#ifdef inline
#undef inline
#endif
#define inline
#endif /* DEBUG */

/*========================================================================
 *
 * OMappingDescriptor_Impl.
 *
 *======================================================================*/
struct OMappingDescriptor_Impl
{
    typedef OMappingDescriptor_Impl self;

    /** Representation.
     */
    sal_uInt32  m_nOffset;
    sal_uInt32  m_nSize;
    sal_uInt8  *m_pData;

    /** Construction.
     */
    inline OMappingDescriptor_Impl (
        sal_uInt32 nOffset = 0xffffffff,
        sal_uInt32 nSize   = 0);

    /** Assignment.
     */
    inline self& operator= (const self& rDescr);

    /** Comparison.
     */
    inline sal_Bool operator== (const self& rDescr) const;
    inline sal_Bool operator<= (const self& rDescr) const;

    /** normalize.
     */
    inline void normalize (
        sal_uInt32 nAlignment,
        sal_uInt32 nSizeLimit);

    /** cleanup.
     */
    inline void cleanup (void);

    /** unmap.
     */
    inline void unmap (void);

    /** sync.
     */
    inline void sync (void);
};

/*
 * OMappingDescriptor_Impl.
 */
inline OMappingDescriptor_Impl::OMappingDescriptor_Impl (
    sal_uInt32 nOffset, sal_uInt32 nSize)
    : m_nOffset (nOffset),
      m_nSize   (nSize),
      m_pData   (NULL)
{
}

/*
 * operator=().
 */
inline OMappingDescriptor_Impl&
OMappingDescriptor_Impl::operator= (const self& rDescr)
{
    m_nOffset = rDescr.m_nOffset;
    m_nSize   = rDescr.m_nSize;
    m_pData   = rDescr.m_pData;

    return *this;
}

/*
 * operator==().
 */
inline sal_Bool
OMappingDescriptor_Impl::operator== (const self& rDescr) const
{
    return ((m_nOffset == rDescr.m_nOffset) &&
            (m_nSize   == rDescr.m_nSize  )    );
}

/*
 * operator<=().
 */
inline sal_Bool
OMappingDescriptor_Impl::operator<= (const self& rDescr) const
{
    return ((m_nOffset == rDescr.m_nOffset) &&
            (m_nSize   <= rDescr.m_nSize  )    );
}

/*
 * normalize.
 */
inline void OMappingDescriptor_Impl::normalize (
    sal_uInt32 nAlignment, sal_uInt32 nSizeLimit)
{
    sal_uInt32 nRemain = (m_nSize % nAlignment);
    if (nRemain)
        m_nSize += (nAlignment - nRemain);

    m_nOffset -= (m_nOffset % nAlignment);
    if ((m_nOffset + m_nSize) > nSizeLimit)
        m_nSize = nSizeLimit - m_nOffset;
}

/*
 * unmap.
 */
inline void OMappingDescriptor_Impl::unmap (void)
{
    if (m_pData)
    {
        __store_munmap (m_pData, m_nSize);
        m_pData = 0, m_nSize = 0;
    }
}

/*
 * sync.
 */
inline void OMappingDescriptor_Impl::sync (void)
{
    if (m_pData)
        __store_msync (m_pData, m_nSize);
}

/*
 * cleanup.
 */
inline void OMappingDescriptor_Impl::cleanup (void)
{
    if (m_pData)
    {
        __store_msync  (m_pData, m_nSize);
        __store_munmap (m_pData, m_nSize);
        m_pData = 0, m_nSize = 0;
    }
}

/*========================================================================
 *
 * OFileLockBytes_Impl interface.
 *
 *======================================================================*/
namespace store
{

class OFileLockBytes_Impl
{
    HSTORE     m_hFile;
#ifdef DEBUG
    sal_Char  *m_pszFilename;
#endif /* DEBUG */
    sal_Bool   m_bMemmap    : 1;
    sal_Bool   m_bWriteable : 1;

    HSTORE     m_hMap;              // OMappingDescriptor (?)
    sal_uInt32 m_nAlignment;        // mapping alignment
    sal_uInt32 m_nSize;             // mapping size

    OMappingDescriptor_Impl m_aDescrOne; // OMemoryDescriptor (?)
    OMappingDescriptor_Impl m_aDescrAny;

public:
    OFileLockBytes_Impl (void);
    ~OFileLockBytes_Impl (void);

    sal_Bool isValid (void) const { return (!!m_hFile); }

    storeError close (void);
    storeError create (
        const sal_Char *pszFilename,
        storeAccessMode eAccessMode);
    storeError create (
        rtl_uString    *pFilename,
        storeAccessMode eAccessMode);

    storeError memmap (OMappingDescriptor_Impl &rDescr);
    storeError size (void);

    storeError resize (sal_uInt32 nSize);

    storeError readAt (
        sal_uInt32  nOffset,
        void       *pBuffer,
        sal_uInt32  nBytes,
        sal_uInt32 &rnDone);
    storeError writeAt (
        sal_uInt32  nOffset,
        const void *pBuffer,
        sal_uInt32  nBytes,
        sal_uInt32 &rnDone);

    storeError sync (void);
    storeError stat (sal_uInt32 &rnSize);
};

} // namespace store

/*========================================================================
 *
 * OFileLockBytes_Impl (inline) implementation.
 *
 *======================================================================*/
/*
 * OFileLockBytes_Impl.
 */
inline OFileLockBytes_Impl::OFileLockBytes_Impl (void)
    : m_hFile       (0),
#ifdef DEBUG
      m_pszFilename (0),
#endif /* DEBUG */
      m_bWriteable  (sal_False),
      m_hMap        (0),
      m_nSize       (0)
{
    m_nAlignment = __store_malign();
    m_bMemmap = (!(m_nAlignment == (sal_uInt32)(-1)));
}

/*
 * ~OFileLockBytes_Impl.
 */
inline OFileLockBytes_Impl::~OFileLockBytes_Impl (void)
{
    if (m_hFile)
    {
        if (m_hMap)
        {
            m_aDescrOne.cleanup();
            m_aDescrAny.cleanup();

            __store_funmap (m_hMap);
            m_hMap = 0;
        }

        __store_fclose (m_hFile);
        m_hFile = 0;
    }
#ifdef DEBUG
    if (m_pszFilename)
    {
        ::free (m_pszFilename);
        m_pszFilename = 0;
    }
#endif /* DEBUG */
}

/*
 * close.
 */
inline storeError OFileLockBytes_Impl::close (void)
{
    if (m_hFile)
    {
        if (m_hMap)
        {
            m_aDescrOne.cleanup();
            m_aDescrAny.cleanup();

            __store_funmap (m_hMap);
            m_hMap = 0;
        }

        __store_fclose (m_hFile);
        m_hFile = 0;
    }
    return store_E_None;
}

/*
 * create(sal_Char*).
 */
inline storeError OFileLockBytes_Impl::create (
    const sal_Char *pszFilename, storeAccessMode eAccessMode)
{
    if (m_hFile)
    {
        if (m_hMap)
        {
            m_aDescrOne.cleanup();
            m_aDescrAny.cleanup();

            __store_funmap (m_hMap);
            m_hMap = 0;
        }

        __store_fclose (m_hFile);
        m_hFile = 0;
    }

    m_bWriteable = (!(eAccessMode == store_AccessReadOnly));

    sal_uInt32 nMode = store_File_OpenRead;
    if (m_bWriteable)
        nMode |= store_File_OpenWrite;
    else
        nMode |= store_File_OpenNoCreate;

    if (eAccessMode == store_AccessCreate)
        nMode |= store_File_OpenTruncate;
    if (eAccessMode == store_AccessReadWrite)
        nMode |= store_File_OpenNoCreate;

    if (m_bMemmap)
        nMode |= store_File_OpenNoBuffer;

    storeError eErrCode = __store_fopen (pszFilename, nMode, m_hFile);
#ifdef DEBUG
    if (eErrCode == store_E_None)
    {
        sal_uInt32 nLen = ::strlen (pszFilename);
        m_pszFilename = (sal_Char*)(::realloc (m_pszFilename, nLen + 1));
        ::memcpy (m_pszFilename, pszFilename, nLen + 1);
    }
#endif /* DEBUG */
    return eErrCode;
}

/*
 * create(rtl_uString*).
 */
inline storeError OFileLockBytes_Impl::create (
    rtl_uString *pFilename, storeAccessMode eAccessMode)
{
    oslFileError result;

    rtl::OUString aNormPath;
    result = osl_normalizePath (pFilename, &(aNormPath.pData));
    if (!(result == osl_File_E_None))
    {
        // Neither System, nor Normalized. May be FileUrl.
        result = osl_getNormalizedPathFromFileURL (
            pFilename, &(aNormPath.pData));
        if (!(result == osl_File_E_None))
        {
            // Invalid path.
            return store_E_InvalidParameter;
        }
    }

    rtl::OUString aSystemPath;
    result = osl_getSystemPathFromNormalizedPath (
        aNormPath.pData, &(aSystemPath.pData));
    if (!(result == osl_File_E_None))
    {
        // Invalid path.
        return store_E_InvalidParameter;
    }

    rtl::OString aFilename (
        aSystemPath.pData->buffer,
        aSystemPath.pData->length,
        osl_getThreadTextEncoding());

    return create (aFilename.pData->buffer, eAccessMode);
}

/*
 * memmap.
 */
inline storeError OFileLockBytes_Impl::memmap (OMappingDescriptor_Impl &rDescr)
{
    if (rDescr <= m_aDescrOne)
        rDescr = m_aDescrOne;
    if (rDescr <= m_aDescrAny)
        rDescr = m_aDescrAny;

    if (!rDescr.m_pData)
    {
        if (rDescr.m_nOffset == 0)
            m_aDescrOne.unmap();
        else
            m_aDescrAny.unmap();

        if (!m_hMap)
        {
            if (m_bWriteable)
                m_hMap = __store_fmap_rw (m_hFile);
            else
                m_hMap = __store_fmap_ro (m_hFile);
            if (!m_hMap)
                return ERROR_FROM_NATIVE(__store_errno());
        }

        if (m_bWriteable)
            rDescr.m_pData = __store_mmap_rw (
                m_hMap, rDescr.m_nOffset, rDescr.m_nSize);
        else
            rDescr.m_pData = __store_mmap_ro (
                m_hMap, rDescr.m_nOffset, rDescr.m_nSize);
        if (!rDescr.m_pData)
            return ERROR_FROM_NATIVE(__store_errno());

        if (rDescr.m_nOffset == 0)
            m_aDescrOne = rDescr;
        else
            m_aDescrAny = rDescr;
    }
    return store_E_None;
}

/*
 * size.
 */
inline storeError OFileLockBytes_Impl::size (void)
{
    if (!m_hMap)
        return __store_fsize (m_hFile, m_nSize);
    else
        return store_E_None;
}

/*
 * resize.
 */
inline storeError OFileLockBytes_Impl::resize (sal_uInt32 nSize)
{
    storeError eErrCode = size();
    if (eErrCode != store_E_None)
        return eErrCode;

    if (nSize != m_nSize)
    {
        if (m_hMap)
        {
#ifdef __STORE_FEATURE_WRITETHROUGH
            // Note: file creation slowed down by about 60 percent.
            m_aDescrOne.cleanup();
            m_aDescrAny.cleanup();
#else
            m_aDescrOne.unmap();
            m_aDescrAny.unmap();
#endif /* __STORE_FEATURE_WRITETHROUGH */

            __store_funmap (m_hMap);
            m_hMap = 0;
        }

        eErrCode = __store_ftrunc (m_hFile, nSize);
        if (eErrCode != store_E_None)
            return eErrCode;

        m_nSize = nSize;
    }
    return store_E_None;
}

/*
 * readAt.
 */
inline storeError OFileLockBytes_Impl::readAt (
    sal_uInt32  nOffset,
    void       *pBuffer,
    sal_uInt32  nBytes,
    sal_uInt32 &rnDone)
{
    storeError eErrCode = store_E_None;
    if (m_bMemmap)
    {
        // Memory mapped I/O.
        eErrCode = size();
        if (eErrCode != store_E_None)
            return eErrCode;

        if (!(nOffset < m_nSize))
            return store_E_None;

        nBytes = SAL_MIN(nOffset + nBytes, m_nSize) - nOffset;
        if (!(nBytes > 0))
            return store_E_None;

        OMappingDescriptor_Impl aDescr;
        if (m_bWriteable)
            aDescr = OMappingDescriptor_Impl (nOffset, nBytes);
        else
            aDescr = OMappingDescriptor_Impl (0, m_nSize);
        aDescr.normalize (m_nAlignment, m_nSize);

        eErrCode = memmap (aDescr);
        if (eErrCode != store_E_None)
            return eErrCode;

        aDescr.m_pData += (nOffset - aDescr.m_nOffset);
        __store_memcpy (pBuffer, aDescr.m_pData, nBytes);

        rnDone = nBytes;
    }
    else
    {
        // File I/O.
        eErrCode = __store_fseek (m_hFile, nOffset);
        if (eErrCode != store_E_None)
            return eErrCode;

        eErrCode = __store_fread (m_hFile, pBuffer, nBytes, rnDone);
    }
    return eErrCode;
}

/*
 * writeAt.
 */
inline storeError OFileLockBytes_Impl::writeAt (
    sal_uInt32  nOffset,
    const void *pBuffer,
    sal_uInt32  nBytes,
    sal_uInt32 &rnDone)
{
    storeError eErrCode = store_E_None;
    if (m_bMemmap)
    {
        // Memory mapped I/O. Determine current size.
        eErrCode = size();
        if (eErrCode != store_E_None)
            return eErrCode;

        // Check current size.
        if (m_nSize < (nOffset + nBytes))
        {
            // Extend.
            eErrCode = resize (nOffset + nBytes);
            if (eErrCode != store_E_None)
                return eErrCode;
        }

        OMappingDescriptor_Impl aDescr (nOffset, nBytes);
        aDescr.normalize (m_nAlignment, m_nSize);

        eErrCode = memmap (aDescr);
        if (eErrCode != store_E_None)
            return eErrCode;

        aDescr.m_pData += (nOffset - aDescr.m_nOffset);
        __store_memcpy (aDescr.m_pData, pBuffer, nBytes);

        rnDone = nBytes;
    }
    else
    {
        // File I/O.
        eErrCode = __store_fseek (m_hFile, nOffset);
        if (eErrCode != store_E_None)
            return eErrCode;

        eErrCode = __store_fwrite (m_hFile, pBuffer, nBytes, rnDone);
    }
    return eErrCode;
}

/*
 * sync.
 */
inline storeError OFileLockBytes_Impl::sync (void)
{
    if (m_bMemmap)
    {
        // Memory mapped I/O.
        m_aDescrOne.sync();
        m_aDescrAny.sync();
    }
    else
    {
        // File I/O.
        __store_fsync (m_hFile);
    }
    return store_E_None;
}

/*
 * stat.
 */
inline storeError OFileLockBytes_Impl::stat (sal_uInt32 &rnSize)
{
    storeError eErrCode = size();
    if (eErrCode == store_E_None)
        rnSize = m_nSize;
    return eErrCode;
}

/*========================================================================
 *
 * OFileLockBytes implementation.
 *
 *======================================================================*/
/*
 * OFileLockBytes.
 */
OFileLockBytes::OFileLockBytes (void)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard (m_aMutex);
    m_pImpl = new OFileLockBytes_Impl();
}

/*
 * ~OFileLockBytes.
 */
OFileLockBytes::~OFileLockBytes (void)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard (m_aMutex);
    delete m_pImpl;
}

/*
 * acquire.
 */
oslInterlockedCount SAL_CALL OFileLockBytes::acquire (void)
{
    return OStoreObject::acquire();
}

/*
 * release.
 */
oslInterlockedCount SAL_CALL OFileLockBytes::release (void)
{
    return OStoreObject::release();
}

/*
 * create.
 */
storeError OFileLockBytes::create (
    rtl_uString     *pFilename,
    storeAccessMode  eAccessMode)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard (m_aMutex);

    if (pFilename)
        return m_pImpl->create (pFilename, eAccessMode);
    else
        return store_E_InvalidParameter;
}

/*
 * readAt.
 */
storeError OFileLockBytes::readAt (
    sal_uInt32  nOffset,
    void       *pBuffer,
    sal_uInt32  nBytes,
    sal_uInt32 &rnDone)
{
    // Initialize [out] param.
    rnDone = 0;

    // Acquire exclusive access.
    osl::MutexGuard aGuard (m_aMutex);
    if (m_pImpl->isValid())
        return m_pImpl->readAt (nOffset, pBuffer, nBytes, rnDone);
    else
        return store_E_InvalidHandle;
}

/*
 * writeAt.
 */
storeError OFileLockBytes::writeAt (
    sal_uInt32  nOffset,
    const void *pBuffer,
    sal_uInt32  nBytes,
    sal_uInt32 &rnDone)
{
    // Initialize [out] param.
    rnDone = 0;

    // Acquire exclusive access.
    osl::MutexGuard aGuard (m_aMutex);
    if (m_pImpl->isValid())
        return m_pImpl->writeAt (nOffset, pBuffer, nBytes, rnDone);
    else
        return store_E_InvalidHandle;
}

/*
 * flush.
 */
storeError OFileLockBytes::flush (void)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard (m_aMutex);
    if (m_pImpl->isValid())
        return m_pImpl->sync();
    else
        return store_E_InvalidHandle;
}

/*
 * setSize.
 */
storeError OFileLockBytes::setSize (sal_uInt32 nSize)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard (m_aMutex);
    if (m_pImpl->isValid())
        return m_pImpl->resize (nSize);
    else
        return store_E_InvalidHandle;
}

/*
 * stat.
 */
storeError OFileLockBytes::stat (sal_uInt32 &rnSize)
{
    // Initialize [out] param.
    rnSize = 0;

    // Acquire exclusive access.
    osl::MutexGuard aGuard (m_aMutex);
    if (m_pImpl->isValid())
        return m_pImpl->stat (rnSize);
    else
        return store_E_InvalidHandle;
}

/*
 * lockRange.
 */
storeError OFileLockBytes::lockRange (
    sal_uInt32 nOffset, sal_uInt32 nBytes)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard (m_aMutex);
    if (m_pImpl->isValid())
        return store_E_None; // E_Unsupported
    else
        return store_E_InvalidHandle;
}

/*
 * unlockRange.
 */
storeError OFileLockBytes::unlockRange (
    sal_uInt32 nOffset, sal_uInt32 nBytes)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard (m_aMutex);
    if (m_pImpl->isValid())
        return store_E_None; // E_Unsupported
    else
        return store_E_InvalidHandle;
}

