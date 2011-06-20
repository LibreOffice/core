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
#include "precompiled_chart2.hxx"

#include "WrappedTextRotationProperty.hxx"
#include "macros.hxx"
#include <com/sun/star/beans/XPropertyState.hpp>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Any;
using ::rtl::OUString;


//.............................................................................
namespace chart
{

WrappedTextRotationProperty::WrappedTextRotationProperty( bool bDirectState )
    : ::chart::WrappedProperty( C2U( "TextRotation" ), C2U( "TextRotation" ) )
    , m_bDirectState( bDirectState )
{
}
WrappedTextRotationProperty::~WrappedTextRotationProperty()
{
}

beans::PropertyState WrappedTextRotationProperty::getPropertyState( const uno::Reference< beans::XPropertyState >& xInnerPropertyState ) const
                        throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    if( m_bDirectState )
        return beans::PropertyState_DIRECT_VALUE;
    return WrappedProperty::getPropertyState( xInnerPropertyState );
}

Any WrappedTextRotationProperty::convertInnerToOuterValue( const Any& rInnerValue ) const
{
    Any aRet;
    double fVal = 0;
    if( rInnerValue >>= fVal )
    {
        sal_Int32 n100thDegrees = static_cast< sal_Int32 >( fVal * 100.0 );
        aRet <<= n100thDegrees;
    }
    return aRet;
}
Any WrappedTextRotationProperty::convertOuterToInnerValue( const Any& rOuterValue ) const
{
    Any aRet;
    sal_Int32 nVal = 0;
    if( rOuterValue >>= nVal )
    {
        double fDoubleDegrees = ( static_cast< double >( nVal ) / 100.0 );
        aRet <<= fDoubleDegrees;
    }
    return aRet;
}

} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
