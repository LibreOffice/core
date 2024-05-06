/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include "Clob.hxx"
#include "Blob.hxx"

#include <connectivity/CommonTools.hxx>
#include <connectivity/dbexception.hxx>

using namespace ::connectivity::firebird;

using namespace ::osl;

using namespace ::com::sun::star;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;

Clob::Clob(isc_db_handle* pDatabaseHandle,
           isc_tr_handle* pTransactionHandle,
           ISC_QUAD const & aBlobID):
    Clob_BASE(m_aMutex),
    m_aBlob(new connectivity::firebird::Blob(pDatabaseHandle, pTransactionHandle, aBlobID)),
    m_nCharCount(-1)
{
}

void SAL_CALL Clob::disposing()
{
    m_aBlob->dispose();
    m_aBlob.clear();
    Clob_BASE::disposing();
}

sal_Int64 SAL_CALL Clob::length()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(Clob_BASE::rBHelper.bDisposed);

    if( m_nCharCount >= 0 )
        return m_nCharCount;
    m_nCharCount = 0;

    // Read each segment, and calculate it's size by interpreting it as a
    // character stream. Assume that no characters are split by the segments.
    bool bLastSegmRead = false;
    std::vector<char> aSegmentBytes;
    do
    {
        bLastSegmRead = m_aBlob->readOneSegment( aSegmentBytes );
        OUString sSegment(aSegmentBytes.data(), aSegmentBytes.size(), RTL_TEXTENCODING_UTF8);

        if( !bLastSegmRead)
            m_nCharCount += sSegment.getLength();
    }while( !bLastSegmRead );

    m_aBlob->closeInput(); // reset position
    return m_nCharCount;
}

OUString SAL_CALL Clob::getSubString(sal_Int64 nPosition,
                                               sal_Int32 nLength)
{
    if (nPosition < 1) // XClob is indexed from 1
        throw lang::IllegalArgumentException(u"nPosition < 1"_ustr, *this, 0);
    --nPosition; // make 0-based

    if (nLength < 0)
        throw lang::IllegalArgumentException(u"nLength < 0"_ustr, *this, 0);

    MutexGuard aGuard(m_aMutex);
    checkDisposed(Clob_BASE::rBHelper.bDisposed);
    // TODO do not reset position if it is not necessary
    m_aBlob->closeInput(); // reset position

    OUStringBuffer sSegmentBuffer;
    std::vector<char> aSegmentBytes;

    for (;;)
    {
        bool bLastRead = m_aBlob->readOneSegment( aSegmentBytes );
        // TODO: handle possible case of split UTF-8 character
        OUString sSegment(aSegmentBytes.data(), aSegmentBytes.size(), RTL_TEXTENCODING_UTF8);

        // skip irrelevant parts
        if (sSegment.getLength() < nPosition)
        {
            if (bLastRead)
                throw lang::IllegalArgumentException(u"nPosition out of range"_ustr, *this, 0);
            nPosition -= sSegment.getLength();
            continue;
        }

        // Getting here for the first time, nPosition may be > 0, meaning copy start offset.
        // This also handles sSegment.getLength() == nPosition case, including nLength == 0.
        const sal_Int32 nCharsToCopy = std::min<sal_Int32>(sSegment.getLength() - nPosition,
                                                           nLength - sSegmentBuffer.getLength());
        sSegmentBuffer.append(sSegment.subView(nPosition, nCharsToCopy));
        if (sSegmentBuffer.getLength() == nLength)
            return sSegmentBuffer.makeStringAndClear();

        assert(sSegmentBuffer.getLength() < nLength);

        if (bLastRead)
            throw lang::IllegalArgumentException(u"out of range"_ustr, *this, 0);

        nPosition = 0; // No offset after first append
    }
}

uno::Reference< XInputStream > SAL_CALL  Clob::getCharacterStream()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(Clob_BASE::rBHelper.bDisposed);

    return m_aBlob->getBinaryStream();
}

sal_Int64 SAL_CALL Clob::position(const OUString& /*rPattern*/,
                                   sal_Int32 /*nStart*/)
{
    ::dbtools::throwFeatureNotImplementedSQLException(u"Clob::position"_ustr, *this);
}

sal_Int64 SAL_CALL Clob::positionOfClob(const Reference <XClob >& /*rPattern*/,
                                         sal_Int64 /*aStart*/)
{
    ::dbtools::throwFeatureNotImplementedSQLException(u"Clob::positionOfClob"_ustr, *this);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
