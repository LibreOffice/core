/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "Blob.hxx"
#include "Connection.hxx"
#include "Util.hxx"

#include "connectivity/dbexception.hxx"

using namespace ::connectivity::firebird;

using namespace ::cppu;
using namespace ::osl;

using namespace ::com::sun::star;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;

Blob::Blob(isc_db_handle* pDatabaseHandle,
           isc_tr_handle* pTransactionHandle,
           ISC_QUAD& aBlobID):
    Blob_BASE(m_aMutex),
    m_pDatabaseHandle(pDatabaseHandle),
    m_pTransactionHandle(pTransactionHandle),
    m_blobID(aBlobID),
    m_blobHandle(0),
    m_bBlobOpened(false),
    m_blobData(0)
{
}

void Blob::ensureBlobIsOpened()
    throw(SQLException)
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
                          NULL);
    if (aErr)
        evaluateStatusVector(m_statusVector, "isc_open_blob2", *this);

}

void SAL_CALL Blob::disposing(void)
{
    MutexGuard aGuard(m_aMutex);

    if (m_bBlobOpened)
    {
        ISC_STATUS aErr;
        aErr = isc_close_blob(m_statusVector,
                              &m_blobHandle);
        if (aErr)
        {
            try
            {
                evaluateStatusVector(m_statusVector, "isc_close_blob", *this);
            }
            catch (SQLException e)
            {
                // we cannot throw any exceptions here anyway
                SAL_WARN("connectivity.firebird", "isc_close_blob failed\n" <<
                         e.Message);
            }
        }
    }

    Blob_BASE::disposing();
}

sal_Int64 SAL_CALL Blob::length()
    throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(Blob_BASE::rBHelper.bDisposed);
    ensureBlobIsOpened();

    char aBlobItems[] = {
        isc_info_blob_total_length
    };
    char aResultBuffer[20];

    isc_blob_info(m_statusVector,
                  &m_blobHandle,
                  sizeof(aBlobItems),
                  aBlobItems,
                  sizeof(aResultBuffer),
                  aResultBuffer);

    evaluateStatusVector(m_statusVector, "isc_blob_info", *this);
    if (*aResultBuffer == isc_info_blob_total_length)
    {
        short aResultLength = (short) isc_vax_integer(aResultBuffer, 2);
        return isc_vax_integer(aResultBuffer+2, aResultLength);
    }
    return 0;
}

uno::Sequence< sal_Int8 > SAL_CALL  Blob::getBytes(sal_Int64 aPosition, sal_Int32 aLength)
    throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(Blob_BASE::rBHelper.bDisposed);
    ensureBlobIsOpened();

    sal_Int64 aTotalLength = length();

    if (!(aPosition + aLength < aTotalLength))
    {
        throw SQLException("Byte array requested outwith valid range", *this, OUString(), 1, Any() );
    }

    if (aTotalLength != m_blobData.getLength())
    {
        m_blobData = uno::Sequence< sal_Int8 >(aTotalLength);
        char* pArray = (char*) m_blobData.getArray();
        sal_Int64 aBytesRead = 0;

        unsigned short aLengthRead; // The amount read in in a isc_get_segment call

        ISC_STATUS aErr;
        do
        {
            aErr = isc_get_segment(m_statusVector,
                                   &m_blobHandle,
                                   &aLengthRead,
                                   aTotalLength - aBytesRead,
                                   pArray + aBytesRead);
        }
        while (aErr == 0 || m_statusVector[1] == isc_segment);
        // Denotes either sucessful read, or only part of segment read successfully.
        if (aErr)
        {
            m_blobData = uno::Sequence< sal_Int8 >(0);
            evaluateStatusVector(m_statusVector, "isc_get_segment", *this);
        }
    }

    if (aLength<aTotalLength)
    {
        uno::Sequence< sal_Int8 > aRet(aLength);
        memcpy(aRet.getArray(), m_blobData.getArray() + aLength, aLength);
        return aRet;
    }
    else
    {
        return m_blobData; // TODO: subsequence
    }
}

uno::Reference< XInputStream > SAL_CALL  Blob::getBinaryStream()
    throw(SQLException, RuntimeException)
{
//     MutexGuard aGuard(m_aMutex);
//     checkDisposed(Blob_BASE::rBHelper.bDisposed);
//     ensureBlobIsOpened();

    ::dbtools::throwFeatureNotImplementedException("Blob::positionOfBlob", *this);
    return NULL;
}

sal_Int64 SAL_CALL  Blob::position(const uno::Sequence< sal_Int8 >& rPattern,
                                   sal_Int64 aStart)
    throw(SQLException, RuntimeException)
{
//     MutexGuard aGuard(m_aMutex);
//     checkDisposed(Blob_BASE::rBHelper.bDisposed);
//     ensureBlobIsOpened();

    (void) rPattern;
    (void) aStart;
    ::dbtools::throwFeatureNotImplementedException("Blob::positionOfBlob", *this);
    return 0;
}

sal_Int64 SAL_CALL  Blob::positionOfBlob(const uno::Reference< XBlob >& rPattern,
                                         sal_Int64 aStart)
    throw(SQLException, RuntimeException)
{
//     MutexGuard aGuard(m_aMutex);
//     checkDisposed(Blob_BASE::rBHelper.bDisposed);
//     ensureBlobIsOpened();

    (void) rPattern;
    (void) aStart;
    ::dbtools::throwFeatureNotImplementedException("Blob::positionOfBlob", *this);
    return 0;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */