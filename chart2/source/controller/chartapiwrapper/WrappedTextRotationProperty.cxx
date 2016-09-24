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

#include "WrappedTextRotationProperty.hxx"
#include "macros.hxx"
#include <com/sun/star/beans/XPropertyState.hpp>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Any;

namespace chart
{

WrappedTextRotationProperty::WrappedTextRotationProperty( bool bDirectState )
    : ::chart::WrappedProperty( "TextRotation" , "TextRotation" )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
