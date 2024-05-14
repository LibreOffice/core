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
#include <utility>

#include "vbapictureformat.hxx"

using namespace ooo::vba;
using namespace com::sun::star;

ScVbaPictureFormat::ScVbaPictureFormat( const css::uno::Reference< ov::XHelperInterface >& xParent,
    const css::uno::Reference< css::uno::XComponentContext >& xContext,
    uno::Reference< drawing::XShape > xShape )
    : ScVbaPictureFormat_BASE( xParent, xContext ), m_xShape(std::move( xShape ))
{
    m_xPropertySet.set( m_xShape, uno::UNO_QUERY_THROW );
}

void
ScVbaPictureFormat::checkParameterRangeInDouble( double nRange, double nMin, double nMax )
{
    if( nRange < nMin )
    {
        throw uno::RuntimeException( u"Parameter out of range, value is too small."_ustr );
    }
    if( nRange > nMax )
    {
        throw uno::RuntimeException( u"Parameter out of range, value is too high."_ustr );
    }
}

// Attributes
double SAL_CALL
ScVbaPictureFormat::getBrightness()
{
    sal_Int16 nLuminance = 0;
    m_xPropertySet->getPropertyValue( u"AdjustLuminance"_ustr ) >>= nLuminance;
    double fBrightness = static_cast< double >( nLuminance );
    fBrightness = ( fBrightness +100 ) / 200;
    return fBrightness;
}

void SAL_CALL
ScVbaPictureFormat::setBrightness( double _brightness )
{
    checkParameterRangeInDouble( _brightness, 0.0, 1.0 );
    double fLuminance = _brightness * 200 - 100;
    sal_Int16 nLuminance = static_cast< sal_Int16 >( fLuminance );
    m_xPropertySet->setPropertyValue( u"AdjustLuminance"_ustr , uno::Any( nLuminance ) );
}

double SAL_CALL
ScVbaPictureFormat::getContrast()
{
    sal_Int16 nContrast = 0;
    m_xPropertySet->getPropertyValue( u"AdjustContrast"_ustr ) >>= nContrast;
    double fContrast = static_cast< double >( nContrast );
    fContrast = ( fContrast + 100 ) / 200;
    return fContrast;
}

void SAL_CALL
ScVbaPictureFormat::setContrast( double _contrast )
{
    checkParameterRangeInDouble( _contrast, 0.0, 1.0 );
    double fContrast = _contrast * 200 - 100;
    sal_Int16 nContrast = static_cast< sal_Int16 >( fContrast );
    m_xPropertySet->setPropertyValue( u"AdjustContrast"_ustr , uno::Any( nContrast ) );
}


// Methods
void SAL_CALL
ScVbaPictureFormat::IncrementBrightness( double increment )
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
ScVbaPictureFormat::IncrementContrast( double increment )
{
    double nContrast = getContrast();
    nContrast += increment;
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

OUString
ScVbaPictureFormat::getServiceImplName()
{
    return u"ScVbaPictureFormat"_ustr;
}

uno::Sequence< OUString >
ScVbaPictureFormat::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.msform.PictureFormat"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
