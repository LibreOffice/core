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
#include <ooo/vba/office/MsoArrowheadLength.hpp>
#include <ooo/vba/office/MsoArrowheadWidth.hpp>
#include <ooo/vba/office/MsoLineDashStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include "vbalineformat.hxx"
#include "vbacolorformat.hxx"

using namespace ooo::vba;
using namespace com::sun::star;

ScVbaLineFormat::ScVbaLineFormat( const uno::Reference< ov::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< drawing::XShape > xShape ) : ScVbaLineFormat_BASE( xParent, xContext ), m_xShape( xShape )
{
    m_xPropertySet.set( xShape, uno::UNO_QUERY_THROW );
    m_nLineDashStyle = office::MsoLineDashStyle::msoLineSolid;
    m_nLineWeight = 1;
}

sal_Int32
ScVbaLineFormat::convertLineStartEndNameToArrowheadStyle( OUString sLineName )
{
    sal_Int32 nLineType = office::MsoArrowheadStyle::msoArrowheadNone;
    if ( sLineName == "Small Arrow" || sLineName == "Arrow" || sLineName == "msArrowEnd" || sLineName == "Double Arrow")
    {
        // msoArrowheadTriangle
        nLineType = office::MsoArrowheadStyle::msoArrowheadTriangle;
    }
    else if ( sLineName == "Square 45" || sLineName == "Square" || sLineName == "msArrowDiamondEnd")
    {
        // msoArrowheadDiamond
        nLineType = office::MsoArrowheadStyle::msoArrowheadDiamond;
    }
    else if (sLineName == "Circle" || sLineName == "msArrowOvalEnd" || sLineName == "Dimension Lines" )
    {
        // msoArrowheadOval
        nLineType = office::MsoArrowheadStyle::msoArrowheadOval;
    }
    else if ( sLineName == "Arrow concave" || sLineName == "msArrowStealthEnd" )
    {
        // msoArrowheadStealth
        nLineType = office::MsoArrowheadStyle::msoArrowheadStealth;
    }
    else if ( sLineName == "Rounded short Arrow" || sLineName == "Rounded large Arrow" || sLineName == "Symmetric Arrow"
           || sLineName == "msArrowOpenEnd" || sLineName == "Line Arrow" )
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
ScVbaLineFormat::convertArrowheadStyleToLineStartEndName( sal_Int32 nArrowheadStyle ) throw (uno::RuntimeException)
{
    switch( nArrowheadStyle )
    {
        case office::MsoArrowheadStyle::msoArrowheadNone:
            return OUString();
        case office::MsoArrowheadStyle::msoArrowheadStealth:
            return OUString("Arrow concave");
        case office::MsoArrowheadStyle::msoArrowheadOpen:
            return OUString("Line Arrow");
        case office::MsoArrowheadStyle::msoArrowheadOval:
            return  OUString("Circle");
        case office::MsoArrowheadStyle::msoArrowheadDiamond:
            return OUString("Square 45");
        case office::MsoArrowheadStyle::msoArrowheadTriangle:
            return OUString("Arrow");
        default:
            throw uno::RuntimeException( "Invalid Arrow Style!" , uno::Reference< uno::XInterface >() );
    }
}

// Attributes
sal_Int32 SAL_CALL
ScVbaLineFormat::getBeginArrowheadStyle() throw (uno::RuntimeException)
{
    sal_Int32 nLineType = office::MsoArrowheadStyle::msoArrowheadNone;
    OUString sLineName;
    m_xPropertySet->getPropertyValue( "LineStartName" ) >>= sLineName;
    if( ( sLineName.getLength() > 7 ) && ( sLineName.indexOf( "msArray" ) ) != -1 )
    {
        sal_Int32 nIndex = sLineName.indexOf( ' ' );
        OUString sName = sLineName.copy( 0, nIndex );
        //sal_Int32 nSize = sLineName.copy( nIndex + 1 ).toInt32();
        nLineType = convertLineStartEndNameToArrowheadStyle( sName );
    }
    else
    {
        nLineType = convertLineStartEndNameToArrowheadStyle( sLineName );
    }
    return nLineType;
}

void SAL_CALL
ScVbaLineFormat::setBeginArrowheadStyle( sal_Int32 _beginarrowheadstyle ) throw (uno::RuntimeException)
{
    OUString sArrayName = convertArrowheadStyleToLineStartEndName( _beginarrowheadstyle );
    m_xPropertySet->setPropertyValue( "LineStartName" , uno::makeAny( sArrayName ) );
}

sal_Int32 SAL_CALL
ScVbaLineFormat::getBeginArrowheadLength() throw (uno::RuntimeException)
{
    // #STUB
    // force error
    throw uno::RuntimeException( "Property 'EndArrowheadWidth' is not supported." , uno::Reference< uno::XInterface >() );
}

void SAL_CALL
ScVbaLineFormat::setBeginArrowheadLength( sal_Int32 /*_beginarrowheadlength*/ ) throw (uno::RuntimeException)
{
    // #STUB
    // force error
    throw uno::RuntimeException( "Property 'EndArrowheadWidth' is not supported." , uno::Reference< uno::XInterface >() );
}

sal_Int32 SAL_CALL
ScVbaLineFormat::getBeginArrowheadWidth() throw (uno::RuntimeException)
{
    // #STUB
    // force error
    throw uno::RuntimeException( "Property 'EndArrowheadWidth' is not supported." , uno::Reference< uno::XInterface >() );
}

void SAL_CALL
ScVbaLineFormat::setBeginArrowheadWidth( sal_Int32 /*_beginarrowheadwidth*/ ) throw (uno::RuntimeException)
{
    // #STUB
    // force error
    throw uno::RuntimeException( "Property 'EndArrowheadWidth' is not supported." , uno::Reference< uno::XInterface >() );
}

sal_Int32 SAL_CALL
ScVbaLineFormat::getEndArrowheadStylel() throw (uno::RuntimeException)
{
    // #STUB
    return 0;
}

void SAL_CALL
ScVbaLineFormat::setEndArrowheadStylel( sal_Int32 /*_endarrowheadstylel*/ ) throw (uno::RuntimeException)
{
    // #STUB
}

sal_Int32 SAL_CALL
ScVbaLineFormat::getEndArrowheadLength() throw (uno::RuntimeException)
{
    // #STUB
    // force error
    throw uno::RuntimeException( "Property 'EndArrowheadWidth' is not supported." , uno::Reference< uno::XInterface >() );
}

void SAL_CALL
ScVbaLineFormat::setEndArrowheadLength( sal_Int32 /*_endarrowheadlength*/ ) throw (uno::RuntimeException)
{
    // #STUB
    throw uno::RuntimeException( "Property 'EndArrowheadWidth' is not supported." , uno::Reference< uno::XInterface >() );
}

sal_Int32 SAL_CALL
ScVbaLineFormat::getEndArrowheadWidth() throw (uno::RuntimeException)
{
    // #STUB
    throw uno::RuntimeException( "Property 'EndArrowheadWidth' is not supported." , uno::Reference< uno::XInterface >() );
}

void SAL_CALL
ScVbaLineFormat::setEndArrowheadWidth( sal_Int32 /*_endarrowheadwidth*/ ) throw (uno::RuntimeException)
{
    // #STUB
    throw uno::RuntimeException( "Property 'EndArrowheadWidth' is not supported." , uno::Reference< uno::XInterface >() );
}

double SAL_CALL
ScVbaLineFormat::getWeight() throw (uno::RuntimeException)
{
    sal_Int32 nLineWidth=0;
    m_xPropertySet->getPropertyValue( "LineWidth" ) >>= nLineWidth;
    double dLineWidth = Millimeter::getInPoints( nLineWidth );
    return dLineWidth;
}

void SAL_CALL
ScVbaLineFormat::setWeight( double _weight ) throw (uno::RuntimeException)
{
    if( _weight < 0 )
        throw uno::RuntimeException( "Parameter: Must be positive." , uno::Reference< uno::XInterface >() );
    if( _weight == 0 )
        _weight = 0.5;
    m_nLineWeight = _weight;
    Millimeter aMillimeter;
    aMillimeter.setInPoints( _weight );

    sal_Int32 nLineWidth = static_cast<sal_Int32>( aMillimeter.getInHundredthsOfOneMillimeter() );
    m_xPropertySet->setPropertyValue( "LineWidth" , uno::makeAny( nLineWidth ) );
    setDashStyle( m_nLineDashStyle );
}

sal_Bool SAL_CALL
ScVbaLineFormat::getVisible() throw (uno::RuntimeException)
{
    drawing::LineStyle aLineStyle;
    m_xPropertySet->getPropertyValue( "LineStyle" ) >>= aLineStyle;
    if( aLineStyle == drawing::LineStyle_NONE )
    {
        return sal_False;
    }
    return sal_True;
}

void SAL_CALL
ScVbaLineFormat::setVisible( sal_Bool _visible ) throw (uno::RuntimeException)
{
    drawing::LineStyle aLineStyle;
    m_xPropertySet->getPropertyValue( "LineStyle" ) >>= aLineStyle;
    if( !_visible )
    {
        aLineStyle = drawing::LineStyle_NONE;
        m_xPropertySet->setPropertyValue( "LineStyle" , uno::makeAny( aLineStyle ) );
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
ScVbaLineFormat::getTransparency() throw (uno::RuntimeException)
{
    sal_Int16 nTransparency = 0;
    m_xPropertySet->getPropertyValue( "LineTransparence" ) >>= nTransparency;
    double fTransparency = static_cast<double>( nTransparency );
    return fTransparency / 100;
}

void SAL_CALL
ScVbaLineFormat::setTransparency( double _transparency ) throw (uno::RuntimeException)
{
    sal_Int16 nTransparency = static_cast<sal_Int16>( _transparency * 100 );
    m_xPropertySet->setPropertyValue( "LineTransparence" , uno::makeAny( nTransparency ) );
}

sal_Int16 SAL_CALL
ScVbaLineFormat::getStyle() throw (uno::RuntimeException)
{
    //OpenOffice.org only supports one LineStyle (other than the DashStyles)
    //Therefore we can only return the SingleLine
    return 1;
}

void SAL_CALL
ScVbaLineFormat::setStyle( sal_Int16 /*_style */) throw (uno::RuntimeException)
{
    //OpenOffice.org only supports one LineStyle (other than the DashStyles)
    //Therefore we do not set the LineStyle, because it maybe is already set
    //to Dashed or Single Line. Setting the 'Visible' or 'DashStyle' properties
    //will be done with the according methods.

    // #STUB
}

sal_Int32 SAL_CALL
ScVbaLineFormat::getDashStyle() throw (uno::RuntimeException)
{
    drawing::LineStyle eLineStyle;
    //LineStyle integer in Xray
    m_xPropertySet->getPropertyValue( "LineStyle" ) >>= eLineStyle;
    if( eLineStyle == drawing::LineStyle_SOLID )
        m_nLineDashStyle = office::MsoLineDashStyle::msoLineSolid;
    else
    {
        drawing::LineDash aLineDash;
        m_xPropertySet->getPropertyValue( "LineDash" ) >>= aLineDash;
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
ScVbaLineFormat::setDashStyle( sal_Int32 _dashstyle ) throw (uno::RuntimeException)
{
    m_nLineDashStyle = _dashstyle;
    if( _dashstyle == office::MsoLineDashStyle::msoLineSolid )
    {
        m_xPropertySet->setPropertyValue( "LineStyle" , uno::makeAny( drawing::LineStyle_SOLID  ));
    }
    else
    {
        m_xPropertySet->setPropertyValue( "LineStyle" , uno::makeAny( drawing::LineStyle_DASH ) );
        drawing::LineDash  pLineDash;
        Millimeter aMillimeter( m_nLineWeight );
        sal_Int32 nPixel = static_cast< sal_Int32 >( aMillimeter.getInHundredthsOfOneMillimeter() );
        switch( _dashstyle )
        {
        case office::MsoLineDashStyle::msoLineDashDot:
            pLineDash.Dots = 1;
            pLineDash.DotLen = nPixel;
            pLineDash.Dashes = 1;
            pLineDash.DashLen = 5 * nPixel;
            pLineDash.Distance = 4 * nPixel;
            break;
        case office::MsoLineDashStyle::msoLineLongDashDot:
            pLineDash.Dots = 1;
            pLineDash.DotLen = nPixel;
            pLineDash.Dashes = 1;
            pLineDash.DashLen = 10 * nPixel;
            pLineDash.Distance = 4 * nPixel;
            break;
        case office::MsoLineDashStyle::msoLineDash:
            pLineDash.Dots = 0;
            pLineDash.DotLen = 0;
            pLineDash.Dashes = 1;
            pLineDash.DashLen = 6 * nPixel;
            pLineDash.Distance = 4 * nPixel;
            break;
        case office::MsoLineDashStyle::msoLineDashDotDot:
            pLineDash.Dots = 2;
            pLineDash.DotLen = nPixel;
            pLineDash.Dashes = 1;
            pLineDash.DashLen = 10 * nPixel;
            pLineDash.Distance = 3 * nPixel;
            break;
        case office::MsoLineDashStyle::msoLineLongDash:
            pLineDash.Dots = 0;
            pLineDash.DotLen = 0;
            pLineDash.Dashes = 1;
            pLineDash.DashLen = 10 * nPixel;
            pLineDash.Distance = 4 * nPixel;
            break;
        case office::MsoLineDashStyle::msoLineSquareDot:
            pLineDash.Dots = 1;
            pLineDash.DotLen = nPixel;
            pLineDash.Dashes = 0;
            pLineDash.DashLen = 0;
            pLineDash.Distance = nPixel;
            break;
        case office::MsoLineDashStyle::msoLineRoundDot:
            pLineDash.Dots = 1;
            pLineDash.DotLen = nPixel;
            pLineDash.Dashes = 0;
            pLineDash.DashLen = 0;
            pLineDash.Distance = nPixel;
            break;
        default:
            throw uno::RuntimeException( "this MsoLineDashStyle is not supported." , uno::Reference< uno::XInterface >() );
        }
        m_xPropertySet->setPropertyValue( "LineDash" , uno::makeAny( pLineDash ) );
    }
}

// Methods
uno::Reference< msforms::XColorFormat > SAL_CALL
ScVbaLineFormat::BackColor() throw (uno::RuntimeException)
{
    return uno::Reference< msforms::XColorFormat >( new ScVbaColorFormat( getParent(), mxContext, this, m_xShape, ::ColorFormatType::LINEFORMAT_BACKCOLOR ) );
}

uno::Reference< msforms::XColorFormat > SAL_CALL
ScVbaLineFormat::ForeColor() throw (uno::RuntimeException)
{
    return uno::Reference< msforms::XColorFormat >( new ScVbaColorFormat( getParent(), mxContext, this, m_xShape, ::ColorFormatType::LINEFORMAT_FORECOLOR ) );
}

OUString
ScVbaLineFormat::getServiceImplName()
{
    return OUString("ScVbaLineFormat");
}

uno::Sequence< OUString >
ScVbaLineFormat::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.msform.LineFormat";
    }
    return aServiceNames;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
