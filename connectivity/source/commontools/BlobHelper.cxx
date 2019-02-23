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
#include <connectivity/BlobHelper.hxx>
#include <comphelper/seqstream.hxx>
#include <connectivity/dbexception.hxx>
#include <com/sun/star/sdbc/SQLException.hpp>

using namespace connectivity;
using namespace dbtools;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;

BlobHelper::BlobHelper(const css::uno::Sequence< sal_Int8 >& _val) : m_aValue(_val)
{
}

::sal_Int64 SAL_CALL BlobHelper::length(  )
{
    return m_aValue.getLength();
}

css::uno::Sequence< ::sal_Int8 > SAL_CALL BlobHelper::getBytes( ::sal_Int64 pos, ::sal_Int32 _length )
{
    if ( sal_Int32(pos + _length) > m_aValue.getLength() )
        throw css::sdbc::SQLException();
    return css::uno::Sequence< ::sal_Int8 >(m_aValue.getConstArray() + sal_Int32(pos),_length);
}

css::uno::Reference< css::io::XInputStream > SAL_CALL BlobHelper::getBinaryStream(  )
{
    return new ::comphelper::SequenceInputStream(m_aValue);
}


// The "return" after a call to throwFeatureNotImplementedSQLException()
// (which always throws) will be detected as unreachable when doing
// global inlining.

SAL_WNOUNREACHABLE_CODE_PUSH

::sal_Int64 SAL_CALL BlobHelper::position( const css::uno::Sequence< ::sal_Int8 >& /*pattern*/, ::sal_Int64 /*start*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XBlob::position", *this );
    return 0;
}

::sal_Int64 SAL_CALL BlobHelper::positionOfBlob( const css::uno::Reference< css::sdbc::XBlob >& /*pattern*/, ::sal_Int64 /*start*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XBlob::positionOfBlob", *this );
    return 0;
}

SAL_WNOUNREACHABLE_CODE_POP

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
