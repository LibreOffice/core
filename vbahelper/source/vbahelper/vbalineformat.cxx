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
ScVbaLineFormat::calculateArrowheadSize() const
{
    return 0;
}

sal_Int32
ScVbaLineFormat::convertLineStartEndNameToArrowheadStyle( rtl::OUString sLineName )
{
    sal_Int32 nLineType = office::MsoArrowheadStyle::msoArrowheadNone;
    if (sLineName.equals(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Small Arrow"))) ||
        sLineName.equals(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Arrow"))) ||
        sLineName.equals(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("msArrowEnd"))) ||
        sLineName.equals(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Double Arrow"))))
    {
        // msoArrowheadTriangle
        nLineType = office::MsoArrowheadStyle::msoArrowheadTriangle;
    }
    else if (sLineName.equals(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Square 45"))) ||
             sLineName.equals(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Square"))) ||
             sLineName.equals(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("msArrowDiamondEnd"))))
    {
        // msoArrowheadDiamond
        nLineType = office::MsoArrowheadStyle::msoArrowheadDiamond;
    }
    else if (sLineName.equals(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Circle"))) ||
             sLineName.equals(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("msArrowOvalEnd"))) ||
             sLineName.equals(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Dimension Lines"))) )
    {
        // msoArrowheadOval
        nLineType = office::MsoArrowheadStyle::msoArrowheadOval;
    }
    else if (sLineName.equals(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Arrow concave"))) ||
             sLineName.equals(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("msArrowStealthEnd"))))
    {
        // msoArrowheadStealth
        nLineType = office::MsoArrowheadStyle::msoArrowheadStealth;
    }
    else if (sLineName.equals(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Rounded short Arrow"))) ||
             sLineName.equals(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Rounded large Arrow"))) ||
             sLineName.equals(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Symmetric Arrow"))) ||
             sLineName.equals(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("msArrowOpenEnd"))) ||
             sLineName.equals(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Line Arrow"))))
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

rtl::OUString
ScVbaLineFormat::convertArrowheadStyleToLineStartEndName( sal_Int32 nArrowheadStyle ) throw (uno::RuntimeException)
{
    switch( nArrowheadStyle )
    {
        case office::MsoArrowheadStyle::msoArrowheadNone:
            return rtl::OUString();
        case office::MsoArrowheadStyle::msoArrowheadStealth:
            return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Arrow concave"));
        case office::MsoArrowheadStyle::msoArrowheadOpen:
            return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Line Arrow"));
        case office::MsoArrowheadStyle::msoArrowheadOval:
            return  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Circle"));
        case office::MsoArrowheadStyle::msoArrowheadDiamond:
            return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Square 45"));
        case office::MsoArrowheadStyle::msoArrowheadTriangle:
            return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Arrow"));
        default:
            throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Invalid Arrow Style!")), uno::Reference< uno::XInterface >() );
    }
}

// Attributes
sal_Int32 SAL_CALL
ScVbaLineFormat::getBeginArrowheadStyle() throw (uno::RuntimeException)
{
    sal_Int32 nLineType = office::MsoArrowheadStyle::msoArrowheadNone;
    rtl::OUString sLineName;
    m_xPropertySet->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LineStartName")) ) >>= sLineName;
    if( ( sLineName.getLength() > 7 ) && ( sLineName.indexOf( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("msArray")) ) ) != -1 )
    {
        sal_Int32 nIndex = sLineName.indexOf( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" ")) );
        rtl::OUString sName = sLineName.copy( 0, nIndex );
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
    rtl::OUString sArrayName = convertArrowheadStyleToLineStartEndName( _beginarrowheadstyle );
    m_xPropertySet->setPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LineStartName")), uno::makeAny( sArrayName ) );
}

sal_Int32 SAL_CALL
ScVbaLineFormat::getBeginArrowheadLength() throw (uno::RuntimeException)
{
    throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Property 'EndArrowheadWidth' is not supported.")), uno::Reference< uno::XInterface >() );
}

void SAL_CALL
ScVbaLineFormat::setBeginArrowheadLength( sal_Int32 /*_beginarrowheadlength*/ ) throw (uno::RuntimeException)
{
    throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Property 'EndArrowheadWidth' is not supported.")), uno::Reference< uno::XInterface >() );
}

sal_Int32 SAL_CALL
ScVbaLineFormat::getBeginArrowheadWidth() throw (uno::RuntimeException)
{
    throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Property 'EndArrowheadWidth' is not supported.")), uno::Reference< uno::XInterface >() );
}

void SAL_CALL
ScVbaLineFormat::setBeginArrowheadWidth( sal_Int32 /*_beginarrowheadwidth*/ ) throw (uno::RuntimeException)
{
    throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Property 'EndArrowheadWidth' is not supported.")), uno::Reference< uno::XInterface >() );
}

sal_Int32 SAL_CALL
ScVbaLineFormat::getEndArrowheadStylel() throw (uno::RuntimeException)
{
    return 0;
}

void SAL_CALL
ScVbaLineFormat::setEndArrowheadStylel( sal_Int32 /*_endarrowheadstylel*/ ) throw (uno::RuntimeException)
{
}

sal_Int32 SAL_CALL
ScVbaLineFormat::getEndArrowheadLength() throw (uno::RuntimeException)
{
    throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Property 'EndArrowheadWidth' is not supported.")), uno::Reference< uno::XInterface >() );
}

void SAL_CALL
ScVbaLineFormat::setEndArrowheadLength( sal_Int32 /*_endarrowheadlength*/ ) throw (uno::RuntimeException)
{
    throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Property 'EndArrowheadWidth' is not supported.")), uno::Reference< uno::XInterface >() );
}

sal_Int32 SAL_CALL
ScVbaLineFormat::getEndArrowheadWidth() throw (uno::RuntimeException)
{
    throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Property 'EndArrowheadWidth' is not supported.")), uno::Reference< uno::XInterface >() );
}

void SAL_CALL
ScVbaLineFormat::setEndArrowheadWidth( sal_Int32 /*_endarrowheadwidth*/ ) throw (uno::RuntimeException)
{
    throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Property 'EndArrowheadWidth' is not supported.")), uno::Reference< uno::XInterface >() );
}

double SAL_CALL
ScVbaLineFormat::getWeight() throw (uno::RuntimeException)
{
    sal_Int32 nLineWidth=0;
    m_xPropertySet->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LineWidth")) ) >>= nLineWidth;
    double dLineWidth = Millimeter::getInPoints( nLineWidth );
    return dLineWidth;
}

void SAL_CALL
ScVbaLineFormat::setWeight( double _weight ) throw (uno::RuntimeException)
{
    if( _weight < 0 )
        throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Parameter: Must be positive.")), uno::Reference< uno::XInterface >() );
    if( _weight == 0 )
        _weight = 0.5;
    m_nLineWeight = _weight;
    Millimeter aMillimeter;
    aMillimeter.setInPoints( _weight );

    sal_Int32 nLineWidth = static_cast<sal_Int32>( aMillimeter.getInHundredthsOfOneMillimeter() );
    m_xPropertySet->setPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LineWidth")), uno::makeAny( nLineWidth ) );
    setDashStyle( m_nLineDashStyle );
}

sal_Bool SAL_CALL
ScVbaLineFormat::getVisible() throw (uno::RuntimeException)
{
    drawing::LineStyle aLineStyle;
    m_xPropertySet->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LineStyle")) ) >>= aLineStyle;
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
    m_xPropertySet->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LineStyle")) ) >>= aLineStyle;
    if( !_visible )
    {
        aLineStyle = drawing::LineStyle_NONE;
        m_xPropertySet->setPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LineStyle")), uno::makeAny( aLineStyle ) );
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
    m_xPropertySet->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LineTransparence")) ) >>= nTransparency;
    double fTransparency = static_cast<double>( nTransparency );
    return fTransparency / 100;
}

void SAL_CALL
ScVbaLineFormat::setTransparency( double _transparency ) throw (uno::RuntimeException)
{
    sal_Int16 nTransparency = static_cast<sal_Int16>( _transparency * 100 );
    m_xPropertySet->setPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LineTransparence")), uno::makeAny( nTransparency ) );
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
}

sal_Int32 SAL_CALL
ScVbaLineFormat::getDashStyle() throw (uno::RuntimeException)
{
    drawing::LineStyle eLineStyle;
    //LineStyle integer in Xray
    m_xPropertySet->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LineStyle")) ) >>= eLineStyle;
    if( eLineStyle == drawing::LineStyle_SOLID )
        m_nLineDashStyle = office::MsoLineDashStyle::msoLineSolid;
    else
    {
        drawing::LineDash aLineDash;
        m_xPropertySet->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LineDash")) ) >>= aLineDash;
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
        m_xPropertySet->setPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LineStyle")), uno::makeAny( drawing::LineStyle_SOLID  ));
    }
    else
    {
        m_xPropertySet->setPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LineStyle")), uno::makeAny( drawing::LineStyle_DASH ) );
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
            throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("this MsoLineDashStyle is not supported.")), uno::Reference< uno::XInterface >() );
        }
        m_xPropertySet->setPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LineDash")), uno::makeAny( pLineDash ) );
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


rtl::OUString&
ScVbaLineFormat::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaLineFormat") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaLineFormat::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.msform.LineFormat" ) );
    }
    return aServiceNames;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
