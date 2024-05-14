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
#include <ooo/vba/office/MsoArrowheadStyle.hpp>
#include <ooo/vba/office/MsoLineDashStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include "vbalineformat.hxx"
#include "vbacolorformat.hxx"

using namespace ooo::vba;
using namespace com::sun::star;

ScVbaLineFormat::ScVbaLineFormat( const uno::Reference< ov::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< drawing::XShape >& xShape ) : ScVbaLineFormat_BASE( xParent, xContext ), m_xShape( xShape )
{
    m_xPropertySet.set( xShape, uno::UNO_QUERY_THROW );
    m_nLineDashStyle = office::MsoLineDashStyle::msoLineSolid;
    m_nLineWeight = 1;
}

sal_Int32
ScVbaLineFormat::convertLineStartEndNameToArrowheadStyle( std::u16string_view sLineName )
{
    sal_Int32 nLineType = office::MsoArrowheadStyle::msoArrowheadNone;
    if ( sLineName == u"Small Arrow" || sLineName == u"Arrow" || sLineName == u"msArrowEnd" || sLineName == u"Double Arrow")
    {
        // msoArrowheadTriangle
        nLineType = office::MsoArrowheadStyle::msoArrowheadTriangle;
    }
    else if ( sLineName == u"Square 45" || sLineName == u"Square" || sLineName == u"msArrowDiamondEnd")
    {
        // msoArrowheadDiamond
        nLineType = office::MsoArrowheadStyle::msoArrowheadDiamond;
    }
    else if (sLineName == u"Circle" || sLineName == u"msArrowOvalEnd" || sLineName == u"Dimension Lines" )
    {
        // msoArrowheadOval
        nLineType = office::MsoArrowheadStyle::msoArrowheadOval;
    }
    else if ( sLineName == u"Arrow concave" || sLineName == u"msArrowStealthEnd" )
    {
        // msoArrowheadStealth
        nLineType = office::MsoArrowheadStyle::msoArrowheadStealth;
    }
    else if ( sLineName == u"Rounded short Arrow" || sLineName == u"Rounded large Arrow" || sLineName == u"Symmetric Arrow"
           || sLineName == u"msArrowOpenEnd" || sLineName == u"Line Arrow" )
    {
        // msoArrowheadOpen
        nLineType = office::MsoArrowheadStyle::msoArrowheadOpen;
    }
    else
    {
        // msoArrowheadNone
        nLineType = office::MsoArrowheadStyle::msoArrowheadNone;
    }
    return nLineType;
}

OUString
ScVbaLineFormat::convertArrowheadStyleToLineStartEndName( sal_Int32 nArrowheadStyle )
{
    switch( nArrowheadStyle )
    {
        case office::MsoArrowheadStyle::msoArrowheadNone:
            return OUString();
        case office::MsoArrowheadStyle::msoArrowheadStealth:
            return u"Arrow concave"_ustr;
        case office::MsoArrowheadStyle::msoArrowheadOpen:
            return u"Line Arrow"_ustr;
        case office::MsoArrowheadStyle::msoArrowheadOval:
            return  u"Circle"_ustr;
        case office::MsoArrowheadStyle::msoArrowheadDiamond:
            return u"Square 45"_ustr;
        case office::MsoArrowheadStyle::msoArrowheadTriangle:
            return u"Arrow"_ustr;
        default:
            throw uno::RuntimeException( u"Invalid Arrow Style!"_ustr );
    }
}

// Attributes
sal_Int32 SAL_CALL
ScVbaLineFormat::getBeginArrowheadStyle()
{
    sal_Int32 nLineType = office::MsoArrowheadStyle::msoArrowheadNone;
    OUString sLineName;
    m_xPropertySet->getPropertyValue( u"LineStartName"_ustr ) >>= sLineName;
    if( ( sLineName.getLength() > 7 ) && ( sLineName.indexOf( "msArray" ) ) != -1 )
    {
        sal_Int32 nIndex = sLineName.indexOf( ' ' );
        OUString sName = sLineName.copy( 0, nIndex );
        //sal_Int32 nSize = o3tl::toInt32(sLineName.subView( nIndex + 1 ));
        nLineType = convertLineStartEndNameToArrowheadStyle( sName );
    }
    else
    {
        nLineType = convertLineStartEndNameToArrowheadStyle( sLineName );
    }
    return nLineType;
}

void SAL_CALL
ScVbaLineFormat::setBeginArrowheadStyle( sal_Int32 _beginarrowheadstyle )
{
    OUString sArrayName = convertArrowheadStyleToLineStartEndName( _beginarrowheadstyle );
    m_xPropertySet->setPropertyValue( u"LineStartName"_ustr , uno::Any( sArrayName ) );
}

sal_Int32 SAL_CALL
ScVbaLineFormat::getBeginArrowheadLength()
{
    // #STUB
    // force error
    throw uno::RuntimeException( u"Property 'EndArrowheadWidth' is not supported."_ustr );
}

void SAL_CALL
ScVbaLineFormat::setBeginArrowheadLength( sal_Int32 /*_beginarrowheadlength*/ )
{
    // #STUB
    // force error
    throw uno::RuntimeException( u"Property 'EndArrowheadWidth' is not supported."_ustr );
}

sal_Int32 SAL_CALL
ScVbaLineFormat::getBeginArrowheadWidth()
{
    // #STUB
    // force error
    throw uno::RuntimeException( u"Property 'EndArrowheadWidth' is not supported."_ustr );
}

void SAL_CALL
ScVbaLineFormat::setBeginArrowheadWidth( sal_Int32 /*_beginarrowheadwidth*/ )
{
    // #STUB
    // force error
    throw uno::RuntimeException( u"Property 'EndArrowheadWidth' is not supported."_ustr );
}

sal_Int32 SAL_CALL
ScVbaLineFormat::getEndArrowheadStylel()
{
    // #STUB
    return 0;
}

void SAL_CALL
ScVbaLineFormat::setEndArrowheadStylel( sal_Int32 /*_endarrowheadstylel*/ )
{
    // #STUB
}

sal_Int32 SAL_CALL
ScVbaLineFormat::getEndArrowheadLength()
{
    // #STUB
    // force error
    throw uno::RuntimeException( u"Property 'EndArrowheadWidth' is not supported."_ustr );
}

void SAL_CALL
ScVbaLineFormat::setEndArrowheadLength( sal_Int32 /*_endarrowheadlength*/ )
{
    // #STUB
    throw uno::RuntimeException( u"Property 'EndArrowheadWidth' is not supported."_ustr );
}

sal_Int32 SAL_CALL
ScVbaLineFormat::getEndArrowheadWidth()
{
    // #STUB
    throw uno::RuntimeException( u"Property 'EndArrowheadWidth' is not supported."_ustr );
}

void SAL_CALL
ScVbaLineFormat::setEndArrowheadWidth( sal_Int32 /*_endarrowheadwidth*/ )
{
    // #STUB
    throw uno::RuntimeException( u"Property 'EndArrowheadWidth' is not supported."_ustr );
}

double SAL_CALL
ScVbaLineFormat::getWeight()
{
    sal_Int32 nLineWidth=0;
    m_xPropertySet->getPropertyValue( u"LineWidth"_ustr ) >>= nLineWidth;
    double dLineWidth = Millimeter::getInPoints( nLineWidth );
    return dLineWidth;
}

void SAL_CALL
ScVbaLineFormat::setWeight( double _weight )
{
    if( _weight < 0 )
        throw uno::RuntimeException( u"Parameter: Must be positive."_ustr );
    if( _weight == 0 )
        _weight = 0.5;
    m_nLineWeight = _weight;
    Millimeter aMillimeter;
    aMillimeter.setInPoints( _weight );

    sal_Int32 nLineWidth = static_cast<sal_Int32>( aMillimeter.getInHundredthsOfOneMillimeter() );
    m_xPropertySet->setPropertyValue( u"LineWidth"_ustr , uno::Any( nLineWidth ) );
    setDashStyle( m_nLineDashStyle );
}

sal_Bool SAL_CALL
ScVbaLineFormat::getVisible()
{
    drawing::LineStyle aLineStyle;
    m_xPropertySet->getPropertyValue( u"LineStyle"_ustr ) >>= aLineStyle;
    if( aLineStyle == drawing::LineStyle_NONE )
    {
        return false;
    }
    return true;
}

void SAL_CALL
ScVbaLineFormat::setVisible( sal_Bool _visible )
{
    drawing::LineStyle aLineStyle;
    m_xPropertySet->getPropertyValue( u"LineStyle"_ustr ) >>= aLineStyle;
    if( !_visible )
    {
        aLineStyle = drawing::LineStyle_NONE;
        m_xPropertySet->setPropertyValue( u"LineStyle"_ustr , uno::Any( aLineStyle ) );
    }
    else
    {
        if( aLineStyle == drawing::LineStyle_NONE )
        {
            setDashStyle( m_nLineDashStyle );
        }
    }
}

double SAL_CALL
ScVbaLineFormat::getTransparency()
{
    sal_Int16 nTransparency = 0;
    m_xPropertySet->getPropertyValue( u"LineTransparence"_ustr ) >>= nTransparency;
    double fTransparency = static_cast<double>( nTransparency );
    return fTransparency / 100;
}

void SAL_CALL
ScVbaLineFormat::setTransparency( double _transparency )
{
    sal_Int16 nTransparency = static_cast<sal_Int16>( _transparency * 100 );
    m_xPropertySet->setPropertyValue( u"LineTransparence"_ustr , uno::Any( nTransparency ) );
}

sal_Int16 SAL_CALL
ScVbaLineFormat::getStyle()
{
    //OpenOffice.org only supports one LineStyle (other than the DashStyles)
    //Therefore we can only return the SingleLine
    return 1;
}

void SAL_CALL
ScVbaLineFormat::setStyle( sal_Int16 /*_style */)
{
    //OpenOffice.org only supports one LineStyle (other than the DashStyles)
    //Therefore we do not set the LineStyle, because it maybe is already set
    //to Dashed or Single Line. Setting the 'Visible' or 'DashStyle' properties
    //will be done with the according methods.

    // #STUB
}

sal_Int32 SAL_CALL
ScVbaLineFormat::getDashStyle()
{
    drawing::LineStyle eLineStyle;
    //LineStyle integer in Xray
    m_xPropertySet->getPropertyValue( u"LineStyle"_ustr ) >>= eLineStyle;
    if( eLineStyle == drawing::LineStyle_SOLID )
        m_nLineDashStyle = office::MsoLineDashStyle::msoLineSolid;
    else
    {
        drawing::LineDash aLineDash;
        m_xPropertySet->getPropertyValue( u"LineDash"_ustr ) >>= aLineDash;
        if( aLineDash.Dots == 0 )
        {
            //LineDash
            //LineLongDash
            m_nLineDashStyle = office::MsoLineDashStyle::msoLineDash;
            if( aLineDash.Distance > 0 && ( aLineDash.DashLen / aLineDash.Distance > 1 ) )
            {
                m_nLineDashStyle = office::MsoLineDashStyle::msoLineLongDash;
            }
        }
        else if( aLineDash.Dots == 1 )
        {
            // LineDashDot
            // LineLongDashDot
            // LineSquareDot
            // LineRoundDot ! not supported
            m_nLineDashStyle = office::MsoLineDashStyle::msoLineDashDot;
            if( aLineDash.Dashes == 0 )
            {
                m_nLineDashStyle = office::MsoLineDashStyle::msoLineSquareDot;
            }
            else
            {
                if( aLineDash.Distance > 0 && ( aLineDash.DashLen / aLineDash.Distance > 1 ) )
                {
                    m_nLineDashStyle = office::MsoLineDashStyle::msoLineLongDashDot;
                }
            }
        }
        else if( aLineDash.Dots == 2 )
        {
            // LineDashDotDot
            m_nLineDashStyle = office::MsoLineDashStyle::msoLineDashDotDot;
        }
    }

    return m_nLineDashStyle;
}

void SAL_CALL
ScVbaLineFormat::setDashStyle( sal_Int32 _dashstyle )
{
    m_nLineDashStyle = _dashstyle;
    if( _dashstyle == office::MsoLineDashStyle::msoLineSolid )
    {
        m_xPropertySet->setPropertyValue( u"LineStyle"_ustr , uno::Any( drawing::LineStyle_SOLID  ));
    }
    else
    {
        m_xPropertySet->setPropertyValue( u"LineStyle"_ustr , uno::Any( drawing::LineStyle_DASH ) );
        drawing::LineDash  aLineDash;
        Millimeter aMillimeter( m_nLineWeight );
        sal_Int32 nPixel = static_cast< sal_Int32 >( aMillimeter.getInHundredthsOfOneMillimeter() );
        switch( _dashstyle )
        {
        case office::MsoLineDashStyle::msoLineDashDot:
            aLineDash.Dots = 1;
            aLineDash.DotLen = nPixel;
            aLineDash.Dashes = 1;
            aLineDash.DashLen = 5 * nPixel;
            aLineDash.Distance = 4 * nPixel;
            break;
        case office::MsoLineDashStyle::msoLineLongDashDot:
            aLineDash.Dots = 1;
            aLineDash.DotLen = nPixel;
            aLineDash.Dashes = 1;
            aLineDash.DashLen = 10 * nPixel;
            aLineDash.Distance = 4 * nPixel;
            break;
        case office::MsoLineDashStyle::msoLineDash:
            aLineDash.Dots = 0;
            aLineDash.DotLen = 0;
            aLineDash.Dashes = 1;
            aLineDash.DashLen = 6 * nPixel;
            aLineDash.Distance = 4 * nPixel;
            break;
        case office::MsoLineDashStyle::msoLineDashDotDot:
            aLineDash.Dots = 2;
            aLineDash.DotLen = nPixel;
            aLineDash.Dashes = 1;
            aLineDash.DashLen = 10 * nPixel;
            aLineDash.Distance = 3 * nPixel;
            break;
        case office::MsoLineDashStyle::msoLineLongDash:
            aLineDash.Dots = 0;
            aLineDash.DotLen = 0;
            aLineDash.Dashes = 1;
            aLineDash.DashLen = 10 * nPixel;
            aLineDash.Distance = 4 * nPixel;
            break;
        case office::MsoLineDashStyle::msoLineSquareDot:
            aLineDash.Dots = 1;
            aLineDash.DotLen = nPixel;
            aLineDash.Dashes = 0;
            aLineDash.DashLen = 0;
            aLineDash.Distance = nPixel;
            break;
        case office::MsoLineDashStyle::msoLineRoundDot:
            aLineDash.Dots = 1;
            aLineDash.DotLen = nPixel;
            aLineDash.Dashes = 0;
            aLineDash.DashLen = 0;
            aLineDash.Distance = nPixel;
            break;
        default:
            throw uno::RuntimeException( u"this MsoLineDashStyle is not supported."_ustr );
        }
        m_xPropertySet->setPropertyValue( u"LineDash"_ustr , uno::Any( aLineDash ) );
    }
}

// Methods
uno::Reference< msforms::XColorFormat > SAL_CALL
ScVbaLineFormat::BackColor()
{
    return uno::Reference< msforms::XColorFormat >( new ScVbaColorFormat( getParent(), mxContext, this, m_xShape, ::ColorFormatType::LINEFORMAT_BACKCOLOR ) );
}

uno::Reference< msforms::XColorFormat > SAL_CALL
ScVbaLineFormat::ForeColor()
{
    return uno::Reference< msforms::XColorFormat >( new ScVbaColorFormat( getParent(), mxContext, this, m_xShape, ::ColorFormatType::LINEFORMAT_FORECOLOR ) );
}

OUString
ScVbaLineFormat::getServiceImplName()
{
    return u"ScVbaLineFormat"_ustr;
}

uno::Sequence< OUString >
ScVbaLineFormat::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.msform.LineFormat"_ustr
    };
    return aServiceNames;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
