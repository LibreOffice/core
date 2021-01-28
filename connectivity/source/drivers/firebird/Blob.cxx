/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "Blob.hxx"
#include "Util.hxx"

#include <com/sun/star/io/BufferSizeExceededException.hpp>
#include <com/sun/star/io/NotConnectedException.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <connectivity/CommonTools.hxx>
#include <connectivity/dbexception.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <tools/diagnose_ex.h>

using namespace ::connectivity::firebird;

using namespace ::cppu;
using namespace ::osl;

using namespace ::com::sun::star;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;

Blob::Blob(isc_db_handle* pDatabaseHandle,
           isc_tr_handle* pTransactionHandle,
           ISC_QUAD const & aBlobID):
    Blob_BASE(m_aMutex),
    m_pDatabaseHandle(pDatabaseHandle),
    m_pTransactionHandle(pTransactionHandle),
    m_blobID(aBlobID),
#if SAL_TYPES_SIZEOFPOINTER == 8
    m_blobHandle(0),
#else
    m_blobHandle(nullptr),
#endif
    m_bBlobOpened(false),
    m_nBlobLength(0),
    m_nMaxSegmentSize(0),
    m_nBlobPosition(0)
{
}

void Blob::ensureBlobIsOpened()
{
    MutexGuard aGuard(m_aMutex);

    if (m_bBlobOpened)
        return;

    ISC_STATUS aErr;
    aErr = isc_open_blob2(m_statusVector,
                          m_pDatabaseHandle,
                          m_pTransactionHandle,
                          &m_blobHandle,
                          &m_blobID,
                          0,
                          nullptr);

    if (aErr)
        evaluateStatusVector(m_statusVector, u"isc_open_blob2", *this);

    m_bBlobOpened = true;
    m_nBlobPosition = 0;

    char aBlobItems[] = {
        isc_info_blob_total_length,
        isc_info_blob_max_segment
    };

    // Assuming a data (e.g. length of blob) is maximum 64 bit.
    // That means we need 8 bytes for data + 2 for length of data + 1 for item
    // identifier for each item.
    char aResultBuffer[11 + 11];

    aErr = isc_blob_info(m_statusVector,
                  &m_blobHandle,
                  sizeof(aBlobItems),
                  aBlobItems,
                  sizeof(aResultBuffer),
                  aResultBuffer);

    if (aErr)
        evaluateStatusVector(m_statusVector, u"isc_blob_info", *this);

    char* pIt = aResultBuffer;
    while( *pIt != isc_info_end ) // info is in clusters
    {
        char item = *pIt++;
        short aResultLength = static_cast<short>(isc_vax_integer(pIt, 2));

        pIt += 2;
        switch(item)
        {
            case isc_info_blob_total_length:
                m_nBlobLength = isc_vax_integer(pIt, aResultLength);
                break;
            case isc_info_blob_max_segment:
                m_nMaxSegmentSize = isc_vax_integer(pIt, aResultLength);
                break;
            default:
                assert(false);
                break;
        }
        pIt += aResultLength;
    }
}

sal_uInt16 Blob::getMaximumSegmentSize()
{
    ensureBlobIsOpened();

    return m_nMaxSegmentSize;
}

bool Blob::readOneSegment(uno::Sequence< sal_Int8 >& rDataOut)
{
    checkDisposed(Blob_BASE::rBHelper.bDisposed);
    ensureBlobIsOpened();

    sal_uInt16 nMaxSize = getMaximumSegmentSize();

    if(rDataOut.getLength() < nMaxSize)
        rDataOut.realloc(nMaxSize);

    sal_uInt16 nActualSize = 0;
    ISC_STATUS aRet = isc_get_segment(m_statusVector,
            &m_blobHandle,
            &nActualSize,
            nMaxSize,
            reinterpret_cast<char*>(rDataOut.getArray()) );

    if (aRet && aRet != isc_segstr_eof && IndicatesError(m_statusVector))
    {
        OUString sError(StatusVectorToString(m_statusVector, u"isc_get_segment"));
        throw IOException(sError, *this);
    }
    m_nBlobPosition += nActualSize;
    return aRet == isc_segstr_eof;  // last segment read
}


void Blob::closeBlob()
{
    MutexGuard aGuard(m_aMutex);

    if (!m_bBlobOpened)
        return;

    ISC_STATUS aErr;
    aErr = isc_close_blob(m_statusVector,
                          &m_blobHandle);
    if (aErr)
        evaluateStatusVector(m_statusVector, u"isc_close_blob", *this);

    m_bBlobOpened = false;
#if SAL_TYPES_SIZEOFPOINTER == 8
    m_blobHandle = 0;
#else
    m_blobHandle = nullptr;
#endif
}

void SAL_CALL Blob::disposing()
{
    try
    {
         closeBlob();
    }
    catch (const SQLException &)
    {
        // we cannot throw any exceptions here...
        TOOLS_WARN_EXCEPTION("connectivity.firebird", "isc_close_blob failed");
        assert(false);
    }
    Blob_BASE::disposing();
}

sal_Int64 SAL_CALL Blob::length()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(Blob_BASE::rBHelper.bDisposed);
    ensureBlobIsOpened();

    return m_nBlobLength;
}

uno::Sequence< sal_Int8 > SAL_CALL  Blob::getBytes(sal_Int64 nPosition,
                                                   sal_Int32 nBytes)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(Blob_BASE::rBHelper.bDisposed);
    ensureBlobIsOpened();

    if (nPosition > m_nBlobLength || nPosition < 1)
        throw lang::IllegalArgumentException("nPosition out of range", *this, 0);
    // We only have to read as many bytes as are available, i.e. nPosition+nBytes
    // can legally be greater than the total length, hence we don't bother to check.

    if (nPosition -1 < m_nBlobPosition)
    {
        // Resets to the beginning (we can't seek these blobs)
        closeBlob();
        ensureBlobIsOpened();
    }

    // nPosition is indexed from 1.
    skipBytes(nPosition - m_nBlobPosition -1 );

    // Don't bother preallocating: readBytes does the appropriate calculations
    // and reallocates for us.
    uno::Sequence< sal_Int8 > aBytes;
    readBytes(aBytes, nBytes);
    return aBytes;
}

uno::Reference< XInputStream > SAL_CALL  Blob::getBinaryStream()
{
    return this;
}

sal_Int64 SAL_CALL  Blob::position(const uno::Sequence< sal_Int8 >& /*rPattern*/,
                                   sal_Int64 /*nStart*/)
{
    ::dbtools::throwFeatureNotImplementedSQLException("Blob::position", *this);
    return 0;
}

sal_Int64 SAL_CALL  Blob::positionOfBlob(const uno::Reference< XBlob >& /*rPattern*/,
                                         sal_Int64 /*aStart*/)
{
    ::dbtools::throwFeatureNotImplementedSQLException("Blob::positionOfBlob", *this);
    return 0;
}

// ---- XInputStream ----------------------------------------------------------

sal_Int32 SAL_CALL Blob::readBytes(uno::Sequence< sal_Int8 >& rDataOut,
                                   sal_Int32 nBytes)
{
    MutexGuard aGuard(m_aMutex);

    try
    {
        checkDisposed(Blob_BASE::rBHelper.bDisposed);
        ensureBlobIsOpened();
    }
    catch (const NotConnectedException&)
    {
        throw;
    }
    catch (const BufferSizeExceededException&)
    {
        throw;
    }
    catch (const IOException&)
    {
        throw;
    }
    catch (const RuntimeException&)
    {
        throw;
    }
    catch (const Exception& e)
    {
        css::uno::Any a(cppu::getCaughtException());
        throw css::lang::WrappedTargetRuntimeException(
            "wrapped Exception " + e.Message,
            css::uno::Reference<css::uno::XInterface>(), a);
    }

    // Ensure we have enough space for the amount of data we can actually read.
    const sal_Int64 nBytesAvailable = m_nBlobLength - m_nBlobPosition;
    const sal_Int32 nBytesToRead = std::min<sal_Int64>(nBytes, nBytesAvailable);

    if (rDataOut.getLength() < nBytesToRead)
        rDataOut.realloc(nBytesToRead);

    sal_Int32 nTotalBytesRead = 0;
    ISC_STATUS aErr;
    while (nTotalBytesRead < nBytesToRead)
    {
        sal_uInt16 nBytesRead = 0;
        sal_uInt64 nDataRemaining = nBytesToRead - nTotalBytesRead;
        sal_uInt16 nReadSize = std::min<sal_uInt64>(nDataRemaining, SAL_MAX_UINT16);
        aErr = isc_get_segment(m_statusVector,
                               &m_blobHandle,
                               &nBytesRead,
                               nReadSize,
                               reinterpret_cast<char*>(rDataOut.getArray()) + nTotalBytesRead);
        if (aErr && IndicatesError(m_statusVector))
        {
            OUString sError(StatusVectorToString(m_statusVector, u"isc_get_segment"));
            throw IOException(sError, *this);
        }
        nTotalBytesRead += nBytesRead;
        m_nBlobPosition += nBytesRead;
    }

    return nTotalBytesRead;
}

sal_Int32 SAL_CALL Blob::readSomeBytes(uno::Sequence< sal_Int8 >& rDataOut,
                                sal_Int32 nMaximumBytes)
{
    // We don't have any way of verifying how many bytes are immediately available,
    // hence we just pass through direct to readBytes
    // (Spec: "reads the available number of bytes, at maximum nMaxBytesToRead.")
    return readBytes(rDataOut, nMaximumBytes);
}

void SAL_CALL Blob::skipBytes(sal_Int32 nBytesToSkip)
{
    // There is no way of directly skipping, hence we have to pretend to skip
    // by reading & discarding the data.
    uno::Sequence< sal_Int8 > aBytes;
    readBytes(aBytes, nBytesToSkip);
}

sal_Int32 SAL_CALL Blob::available()
{
    MutexGuard aGuard(m_aMutex);

    try
    {
        checkDisposed(Blob_BASE::rBHelper.bDisposed);
        ensureBlobIsOpened();
    }
    catch (const NotConnectedException&)
    {
        throw;
    }
    catch (const IOException&)
    {
        throw;
    }
    catch (const RuntimeException&)
    {
        throw;
    }
    catch (const Exception& e)
    {
        css::uno::Any a(cppu::getCaughtException());
        throw css::lang::WrappedTargetRuntimeException(
            "wrapped Exception " + e.Message,
            css::uno::Reference<css::uno::XInterface>(), a);
    }

    return m_nBlobLength - m_nBlobPosition;
}

void SAL_CALL Blob::closeInput()
{
    try
    {
        closeBlob();
    }
    catch (const NotConnectedException&)
    {
        throw;
    }
    catch (const IOException&)
    {
        throw;
    }
    catch (const RuntimeException&)
    {
        throw;
    }
    catch (const Exception& e)
    {
        css::uno::Any a(cppu::getCaughtException());
        throw css::lang::WrappedTargetRuntimeException(
            "wrapped Exception " + e.Message,
            css::uno::Reference<css::uno::XInterface>(), a);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
