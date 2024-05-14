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
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/awt/GradientStyle.hpp>
#include <ooo/vba/office/MsoGradientStyle.hpp>
#include "vbafillformat.hxx"
#include "vbacolorformat.hxx"

using namespace ooo::vba;
using namespace com::sun::star;

ScVbaFillFormat::ScVbaFillFormat( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< drawing::XShape >& xShape ) : ScVbaFillFormat_BASE( xParent, xContext ), m_xShape( xShape )
{
    m_xPropertySet.set( xShape, uno::UNO_QUERY_THROW );
    m_nFillStyle = drawing::FillStyle_SOLID;
    m_nForeColor = 0;
    m_nGradientAngle = 0;
}

void
ScVbaFillFormat::setFillStyle( drawing::FillStyle nFillStyle )
{
    m_nFillStyle = nFillStyle;
    if( m_nFillStyle == drawing::FillStyle_GRADIENT )
    {
        m_xPropertySet->setPropertyValue( u"FillStyle"_ustr , uno::Any( drawing::FillStyle_GRADIENT ) );
        awt::Gradient aGradient;
        // AXIAL
        // RADIAL
        // ELLIPTICAL
        // SQUARE
        // RECT
        aGradient.Style = awt::GradientStyle_LINEAR;
        aGradient.StartColor = ForeColor()->getRGB();
        aGradient.EndColor = BackColor()->getRGB();
        aGradient.Angle = m_nGradientAngle;
        aGradient.Border = 0;
        aGradient.XOffset = 0;
        aGradient.YOffset = 0;
        aGradient.StartIntensity = 100;
        aGradient.EndIntensity = 100;
        aGradient.StepCount = 1;
        m_xPropertySet->setPropertyValue( u"FillGradient"_ustr , uno::Any( aGradient ) );
    }
    else if( m_nFillStyle == drawing::FillStyle_SOLID )
    {
        m_xPropertySet->setPropertyValue( u"FillStyle"_ustr , uno::Any(drawing::FillStyle_SOLID) );
    }
}

void
ScVbaFillFormat::setForeColorAndInternalStyle( sal_Int32 nForeColor )
{
    m_nForeColor = nForeColor;
    setFillStyle( m_nFillStyle );
}

// Attributes
sal_Bool SAL_CALL
ScVbaFillFormat::getVisible()
{
    drawing::FillStyle nFillStyle;
    m_xPropertySet->getPropertyValue( u"FillStyle"_ustr ) >>= nFillStyle;
    if( nFillStyle == drawing::FillStyle_NONE )
        return false;
    return true;
}

void SAL_CALL
ScVbaFillFormat::setVisible( sal_Bool _visible )
{
    drawing::FillStyle aFillStyle;
    m_xPropertySet->getPropertyValue( u"FillStyle"_ustr ) >>= aFillStyle;
    if( !_visible )
    {
        m_xPropertySet->setPropertyValue( u"FillStyle"_ustr , uno::Any( drawing::FillStyle_NONE ) );
    }
    else
    {
        if( aFillStyle == drawing::FillStyle_NONE )
        {
            setFillStyle( m_nFillStyle );
        }
    }
}

double SAL_CALL
ScVbaFillFormat::getTransparency()
{
    sal_Int16 nTransparence = 0;
    double dTransparence = 0;
    m_xPropertySet->getPropertyValue( u"FillTransparence"_ustr ) >>= nTransparence;
    dTransparence = static_cast<double>( nTransparence );
    dTransparence /= 100;
    return dTransparence;
}

void SAL_CALL
ScVbaFillFormat::setTransparency( double _transparency )
{
    sal_Int16 nTransparence = static_cast< sal_Int16 >( _transparency * 100 );
    m_xPropertySet->setPropertyValue( u"FillTransparence"_ustr , uno::Any( nTransparence ) );
}


// Methods
void SAL_CALL
ScVbaFillFormat::Solid()
{
    setFillStyle( drawing::FillStyle_SOLID );
}

void SAL_CALL
ScVbaFillFormat::TwoColorGradient( sal_Int32 style, sal_Int32 /*variant*/ )
{
    if( style == office::MsoGradientStyle::msoGradientHorizontal )
    {
        m_nGradientAngle = 0;
        setFillStyle( drawing::FillStyle_GRADIENT );
    }
    else if( style == office::MsoGradientStyle::msoGradientVertical )
    {
        m_nGradientAngle = 900;
        setFillStyle( drawing::FillStyle_GRADIENT );
    }
    else if( style == office::MsoGradientStyle::msoGradientDiagonalDown )
    {
        m_nGradientAngle = 450;
        setFillStyle( drawing::FillStyle_GRADIENT );
    }
    else if( style == office::MsoGradientStyle::msoGradientDiagonalUp )
    {
        m_nGradientAngle = 900 + 450;
        setFillStyle( drawing::FillStyle_GRADIENT );
    }
}

uno::Reference< msforms::XColorFormat > SAL_CALL
ScVbaFillFormat::BackColor()
{
    if( !m_xColorFormat.is() )
        m_xColorFormat.set( new ScVbaColorFormat( getParent(), mxContext, this, m_xShape, ColorFormatType::FILLFORMAT_BACKCOLOR ) );
    return m_xColorFormat;
}

uno::Reference< msforms::XColorFormat > SAL_CALL
ScVbaFillFormat::ForeColor()
{
    if( !m_xColorFormat.is() )
        m_xColorFormat.set( new ScVbaColorFormat( getParent(), mxContext, this, m_xShape, ColorFormatType::FILLFORMAT_FORECOLOR ) );
    return m_xColorFormat;
}

OUString
ScVbaFillFormat::getServiceImplName()
{
    return u"ScVbaFillFormat"_ustr;
}

uno::Sequence< OUString >
ScVbaFillFormat::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.msforms.FillFormat"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
