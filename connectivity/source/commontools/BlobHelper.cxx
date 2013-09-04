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

// The "return" after a call to throwFeatureNotImplementedException()
// (which always throws) will be detected as unreachable when doing
// global inlining.

SAL_WNOUNREACHABLE_CODE_PUSH

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

SAL_WNOUNREACHABLE_CODE_POP

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
