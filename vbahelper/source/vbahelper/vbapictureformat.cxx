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
#include "vbapictureformat.hxx"

using namespace ooo::vba;
using namespace com::sun::star;

ScVbaPictureFormat::ScVbaPictureFormat( const css::uno::Reference< ov::XHelperInterface >& xParent,
    const css::uno::Reference< css::uno::XComponentContext >& xContext,
    uno::Reference< drawing::XShape > xShape )
    throw( lang::IllegalArgumentException ) : ScVbaPictureFormat_BASE( xParent, xContext ), m_xShape( xShape )
{
    m_xPropertySet.set( m_xShape, uno::UNO_QUERY_THROW );
}

void
ScVbaPictureFormat::checkParameterRangeInDouble( double nRange, double nMin, double nMax ) throw (css::uno::RuntimeException)
{
    if( nRange < nMin )
    {
        throw uno::RuntimeException( "Parameter out of range, value is too small." , uno::Reference< uno::XInterface >() );
    }
    if( nRange > nMax )
    {
        throw uno::RuntimeException( "Parameter out of range, value is too high." , uno::Reference< uno::XInterface >() );
    }
}

// Attributes
double SAL_CALL
ScVbaPictureFormat::getBrightness() throw (uno::RuntimeException)
{
    sal_Int16 nLuminance = 0;
    m_xPropertySet->getPropertyValue( "AdjustLuminance" ) >>= nLuminance;
    double fBrightness = static_cast< double >( nLuminance );
    fBrightness = ( fBrightness +100 ) / 200;
    return fBrightness;
}

void SAL_CALL
ScVbaPictureFormat::setBrightness( double _brightness ) throw (uno::RuntimeException)
{
    checkParameterRangeInDouble( _brightness, 0.0, 1.0 );
    double fLuminance = _brightness * 200 - 100;
    sal_Int16 nLuminance = static_cast< sal_Int16 >( fLuminance );
    m_xPropertySet->setPropertyValue( "AdjustLuminance" , uno::makeAny( nLuminance ) );
}

double SAL_CALL
ScVbaPictureFormat::getContrast() throw (uno::RuntimeException)
{
    sal_Int16 nContrast = 0;
    m_xPropertySet->getPropertyValue( "AdjustContrast" ) >>= nContrast;
    double fContrast = static_cast< double >( nContrast );
    fContrast = ( fContrast + 100 ) / 200;
    return fContrast;
}

void SAL_CALL
ScVbaPictureFormat::setContrast( double _contrast ) throw (uno::RuntimeException)
{
    checkParameterRangeInDouble( _contrast, 0.0, 1.0 );
    double fContrast = _contrast * 200 - 100;
    sal_Int16 nContrast = static_cast< sal_Int16 >( fContrast );
    m_xPropertySet->setPropertyValue( "AdjustContrast" , uno::makeAny( nContrast ) );
}


// Methods
void SAL_CALL
ScVbaPictureFormat::IncrementBrightness( double increment ) throw (uno::RuntimeException)
{
    double fBrightness = getBrightness();
    fBrightness += increment;
    if( fBrightness < 0 )
    {
        fBrightness = 0.0;
    }
    if( fBrightness > 1 )
    {
        fBrightness = 1;
    }
    setBrightness( fBrightness );
}

void SAL_CALL
ScVbaPictureFormat::IncrementContrast( double increment ) throw (uno::RuntimeException)
{
    double nContrast = getContrast();
    nContrast += increment;
    if( increment < 0 )
    {
        increment = 0.0;
    }
    if( increment > 1 )
    {
        increment = 1.0;
    }
    setContrast( nContrast );
}

OUString
ScVbaPictureFormat::getServiceImplName()
{
    return OUString("ScVbaPictureFormat");
}

uno::Sequence< OUString >
ScVbaPictureFormat::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.msform.PictureFormat";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
