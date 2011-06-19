/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"
#include "connectivity/BlobHelper.hxx"
#include <comphelper/seqstream.hxx>
#include "connectivity/dbexception.hxx"

using namespace connectivity;
using namespace dbtools;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;

BlobHelper::BlobHelper(const ::com::sun::star::uno::Sequence< sal_Int8 >& _val) : m_aValue(_val)
{
}
// -----------------------------------------------------------------------------
::sal_Int64 SAL_CALL BlobHelper::length(  ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    return m_aValue.getLength();
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::sal_Int8 > SAL_CALL BlobHelper::getBytes( ::sal_Int64 pos, ::sal_Int32 _length ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    if ( sal_Int32(pos + _length) > m_aValue.getLength() )
        throw ::com::sun::star::sdbc::SQLException();
    return ::com::sun::star::uno::Sequence< ::sal_Int8 >(m_aValue.getConstArray() + sal_Int32(pos),_length);
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL BlobHelper::getBinaryStream(  ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    return new ::comphelper::SequenceInputStream(m_aValue);
}
// -----------------------------------------------------------------------------
::sal_Int64 SAL_CALL BlobHelper::position( const ::com::sun::star::uno::Sequence< ::sal_Int8 >& /*pattern*/, ::sal_Int64 /*start*/ ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XBlob::position", *this );
    return 0;
}
// -----------------------------------------------------------------------------
::sal_Int64 SAL_CALL BlobHelper::positionOfBlob( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XBlob >& /*pattern*/, ::sal_Int64 /*start*/ ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XBlob::positionOfBlob", *this );
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
