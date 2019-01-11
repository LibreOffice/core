/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <string_view>

#include "Clob.hxx"
#include "Blob.hxx"
#include "Connection.hxx"
#include "Util.hxx"

#include <connectivity/dbexception.hxx>
#include <cppuhelper/exc_hlp.hxx>

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
    do
    {
        uno::Sequence < sal_Int8 > aSegmentBytes;
        bLastSegmRead = m_aBlob->readOneSegment( aSegmentBytes );
        OUString sSegment ( reinterpret_cast< sal_Char *>( aSegmentBytes.getArray() ),
                            aSegmentBytes.getLength(),
                            RTL_TEXTENCODING_UTF8 );

        if( !bLastSegmRead)
            m_nCharCount += sSegment.getLength();
    }while( !bLastSegmRead );

    m_aBlob->closeInput(); // reset position
    return m_nCharCount;
}

OUString SAL_CALL Clob::getSubString(sal_Int64 nPosition,
                                               sal_Int32 nLength)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(Clob_BASE::rBHelper.bDisposed);
    // TODO do not reset position if it is not necessary
    m_aBlob->closeInput(); // reset position

    OUStringBuffer sSegmentBuffer;
    sal_Int64 nActPos = 1;
    sal_Int32 nActLen = 0;

    // skip irrelevant parts
    while( nActPos < nPosition )
    {
        uno::Sequence < sal_Int8 > aSegmentBytes;
        bool bLastRead = m_aBlob->readOneSegment( aSegmentBytes );
        if( bLastRead )
            throw lang::IllegalArgumentException("nPosition out of range", *this, 0);

        OUString sSegment ( reinterpret_cast< sal_Char *>( aSegmentBytes.getArray() ),
                            aSegmentBytes.getLength(),
                            RTL_TEXTENCODING_UTF8 );
        sal_Int32 nStrLen = sSegment.getLength();
        nActPos += nStrLen;
        if( nActPos > nPosition )
        {
            sal_Int32 nCharsToCopy = static_cast<sal_Int32>(nActPos - nPosition);
            if( nCharsToCopy > nLength )
                nCharsToCopy = nLength;
            // append relevant part of first segment
            sSegmentBuffer.append( std::u16string_view(sSegment).substr(0, nCharsToCopy) );
            nActLen += sSegmentBuffer.getLength();
        }
    }

    // read nLength characters
    while( nActLen < nLength )
    {
        uno::Sequence < sal_Int8 > aSegmentBytes;
        bool bLastRead = m_aBlob->readOneSegment( aSegmentBytes );

        OUString sSegment ( reinterpret_cast< sal_Char *>( aSegmentBytes.getArray() ),
                            aSegmentBytes.getLength(),
                            RTL_TEXTENCODING_UTF8 );
        sal_Int32 nStrLen = sSegment.getLength();
        if( nActLen + nStrLen > nLength )
            sSegmentBuffer.append(std::u16string_view(sSegment).substr(0, nLength - nActLen));
        else
            sSegmentBuffer.append(sSegment);
        nActLen += nStrLen;

        if( bLastRead && nActLen < nLength )
            throw lang::IllegalArgumentException("out of range", *this, 0);
    }

    return sSegmentBuffer.makeStringAndClear();
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
    ::dbtools::throwFeatureNotImplementedSQLException("Clob::position", *this);
    return 0;
}

sal_Int64 SAL_CALL Clob::positionOfClob(const Reference <XClob >& /*rPattern*/,
                                         sal_Int64 /*aStart*/)
{
    ::dbtools::throwFeatureNotImplementedSQLException("Blob::positionOfBlob", *this);
    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
