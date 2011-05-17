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
        throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Parameter out of range, value is too small.")) , uno::Reference< uno::XInterface >() );
    }
    if( nRange > nMax )
    {
        throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Parameter out of range, value is too high.")) , uno::Reference< uno::XInterface >() );
    }
}

// Attributes
double SAL_CALL
ScVbaPictureFormat::getBrightness() throw (uno::RuntimeException)
{
    sal_Int16 nLuminance = 0;
    m_xPropertySet->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AdjustLuminance")) ) >>= nLuminance;
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
    m_xPropertySet->setPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AdjustLuminance")), uno::makeAny( nLuminance ) );
}

double SAL_CALL
ScVbaPictureFormat::getContrast() throw (uno::RuntimeException)
{
    sal_Int16 nContrast = 0;
    m_xPropertySet->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AdjustContrast")) ) >>= nContrast;
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
    m_xPropertySet->setPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AdjustContrast")), uno::makeAny( nContrast ) );
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
    //VBA, minz@cn.ibm.com.
    if( nContrast < 0 )
    {
        nContrast = 0.0;
    }
    if( nContrast > 1 )
    {
        nContrast = 1.0;
    }
    setContrast( nContrast );
}


rtl::OUString&
ScVbaPictureFormat::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaPictureFormat") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaPictureFormat::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.msform.PictureFormat" ) );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
