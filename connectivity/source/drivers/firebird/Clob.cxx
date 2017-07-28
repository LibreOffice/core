/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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
    m_aBlob(new connectivity::firebird::Blob(pDatabaseHandle, pTransactionHandle, aBlobID))
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

    // read the entire blob
    // TODO FIXME better solution?
    uno::Sequence < sal_Int8 > aEntireBlob = m_aBlob->getBytes( 1, m_aBlob->length());
    OUString sEntireClob (  reinterpret_cast< sal_Char *>( aEntireBlob.getArray() ),
                            aEntireBlob.getLength(),
                            RTL_TEXTENCODING_UTF8 );
    return sEntireClob.getLength();
}

OUString SAL_CALL Clob::getSubString(sal_Int64 nPosition,
                                               sal_Int32 nLength)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(Clob_BASE::rBHelper.bDisposed);

    // read the entire blob
    // TODO FIXME better solution?
    // TODO FIXME Assume indexing of nPosition starts at position 1.
    uno::Sequence < sal_Int8 > aEntireBlob = m_aBlob->getBytes( 1, m_aBlob->length());
    OUString sEntireClob (  reinterpret_cast< sal_Char *>( aEntireBlob.getArray() ),
                            aEntireBlob.getLength(),
                            RTL_TEXTENCODING_UTF8 );

    if( nPosition + nLength - 1 > sEntireClob.getLength() )
        throw lang::IllegalArgumentException("nPosition out of range", *this, 0);

    return sEntireClob.copy(nPosition - 1 , nLength);
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
