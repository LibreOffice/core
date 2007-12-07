/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbalineformat.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 10:54:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include <org/openoffice/office/MsoArrowheadStyle.hpp>
#include <org/openoffice/office/MsoArrowheadLength.hpp>
#include <org/openoffice/office/MsoArrowheadWidth.hpp>
#include <org/openoffice/office/MsoLineDashStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include "vbalineformat.hxx"
#include "vbacolorformat.hxx"

using namespace org::openoffice;
using namespace com::sun::star;

ScVbaLineFormat::ScVbaLineFormat( const uno::Reference< oo::vba::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< drawing::XShape > xShape ) : ScVbaLineFormat_BASE( xParent, xContext ), m_xShape( xShape )
{
    m_xPropertySet.set( xShape, uno::UNO_QUERY_THROW );
    m_nLineDashStyle = office::MsoLineDashStyle::msoLineSolid;
    m_nLineWeight = 1;
}

sal_Int32
ScVbaLineFormat::calculateArrowheadSize()
{
    return 0;
}

sal_Int32
ScVbaLineFormat::convertLineStartEndNameToArrowheadStyle( rtl::OUString sLineName )
{
    sal_Int32 nLineType = office::MsoArrowheadStyle::msoArrowheadNone;
    if (sLineName.equals(rtl::OUString::createFromAscii("Small Arrow")) ||
        sLineName.equals(rtl::OUString::createFromAscii("Arrow")) ||
        sLineName.equals(rtl::OUString::createFromAscii("msArrowEnd")) ||
        sLineName.equals(rtl::OUString::createFromAscii("Double Arrow")))
    {
        // msoArrowheadTriangle
        nLineType = office::MsoArrowheadStyle::msoArrowheadTriangle;
    }
    else if (sLineName.equals(rtl::OUString::createFromAscii("Square 45")) ||
             sLineName.equals(rtl::OUString::createFromAscii("Square")) ||
             sLineName.equals(rtl::OUString::createFromAscii("msArrowDiamondEnd")))
    {
        // msoArrowheadDiamond
        nLineType = office::MsoArrowheadStyle::msoArrowheadDiamond;
    }
    else if (sLineName.equals(rtl::OUString::createFromAscii("Circle")) ||
             sLineName.equals(rtl::OUString::createFromAscii("msArrowOvalEnd")) ||
             sLineName.equals(rtl::OUString::createFromAscii("Dimension Lines")) )
    {
        // msoArrowheadOval
        nLineType = office::MsoArrowheadStyle::msoArrowheadOval;
    }
    else if (sLineName.equals(rtl::OUString::createFromAscii("Arrow concave")) ||
             sLineName.equals(rtl::OUString::createFromAscii("msArrowStealthEnd")))
    {
        // msoArrowheadStealth
        nLineType = office::MsoArrowheadStyle::msoArrowheadStealth;
    }
    else if (sLineName.equals(rtl::OUString::createFromAscii("Rounded short Arrow")) ||
             sLineName.equals(rtl::OUString::createFromAscii("Rounded large Arrow")) ||
             sLineName.equals(rtl::OUString::createFromAscii("Symmetric Arrow")) ||
             sLineName.equals(rtl::OUString::createFromAscii("msArrowOpenEnd")) ||
             sLineName.equals(rtl::OUString::createFromAscii("Line Arrow")))
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
            return rtl::OUString(rtl::OUString::createFromAscii( "" ) );
        case office::MsoArrowheadStyle::msoArrowheadStealth:
            return rtl::OUString::createFromAscii( "Arrow concave" );
        case office::MsoArrowheadStyle::msoArrowheadOpen:
            return rtl::OUString::createFromAscii("Line Arrow" );
        case office::MsoArrowheadStyle::msoArrowheadOval:
            return  rtl::OUString::createFromAscii("Circle" );
        case office::MsoArrowheadStyle::msoArrowheadDiamond:
            return rtl::OUString::createFromAscii( "Square 45" );
        case office::MsoArrowheadStyle::msoArrowheadTriangle:
            return rtl::OUString::createFromAscii( "Arrow" );
        default:
            throw uno::RuntimeException( rtl::OUString::createFromAscii("Invalid Arrow Style!"), uno::Reference< uno::XInterface >() );
    }
}

// Attributes
sal_Int32 SAL_CALL
ScVbaLineFormat::getBeginArrowheadStyle() throw (uno::RuntimeException)
{
    sal_Int32 nLineType = office::MsoArrowheadStyle::msoArrowheadNone;
    rtl::OUString sLineName;
    m_xPropertySet->getPropertyValue( rtl::OUString::createFromAscii( "LineStartName" ) ) >>= sLineName;
    if( ( sLineName.getLength() > 7 ) && ( sLineName.indexOf( rtl::OUString::createFromAscii( "msArray" ) ) ) != -1 )
    {
        sal_Int32 nIndex = sLineName.indexOf( rtl::OUString::createFromAscii(" ") );
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
    m_xPropertySet->setPropertyValue( rtl::OUString::createFromAscii( "LineStartName" ), uno::makeAny( sArrayName ) );
}

sal_Int32 SAL_CALL
ScVbaLineFormat::getBeginArrowheadLength() throw (uno::RuntimeException)
{
    throw uno::RuntimeException( rtl::OUString::createFromAscii("Property 'EndArrowheadWidth' is not supported."), uno::Reference< uno::XInterface >() );
}

void SAL_CALL
ScVbaLineFormat::setBeginArrowheadLength( sal_Int32 /*_beginarrowheadlength*/ ) throw (uno::RuntimeException)
{
    throw uno::RuntimeException( rtl::OUString::createFromAscii("Property 'EndArrowheadWidth' is not supported."), uno::Reference< uno::XInterface >() );
}

sal_Int32 SAL_CALL
ScVbaLineFormat::getBeginArrowheadWidth() throw (uno::RuntimeException)
{
    throw uno::RuntimeException( rtl::OUString::createFromAscii("Property 'EndArrowheadWidth' is not supported."), uno::Reference< uno::XInterface >() );
}

void SAL_CALL
ScVbaLineFormat::setBeginArrowheadWidth( sal_Int32 /*_beginarrowheadwidth*/ ) throw (uno::RuntimeException)
{
    throw uno::RuntimeException( rtl::OUString::createFromAscii("Property 'EndArrowheadWidth' is not supported."), uno::Reference< uno::XInterface >() );
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
    throw uno::RuntimeException( rtl::OUString::createFromAscii("Property 'EndArrowheadWidth' is not supported."), uno::Reference< uno::XInterface >() );
}

void SAL_CALL
ScVbaLineFormat::setEndArrowheadLength( sal_Int32 /*_endarrowheadlength*/ ) throw (uno::RuntimeException)
{
    throw uno::RuntimeException( rtl::OUString::createFromAscii("Property 'EndArrowheadWidth' is not supported."), uno::Reference< uno::XInterface >() );
}

sal_Int32 SAL_CALL
ScVbaLineFormat::getEndArrowheadWidth() throw (uno::RuntimeException)
{
    throw uno::RuntimeException( rtl::OUString::createFromAscii("Property 'EndArrowheadWidth' is not supported."), uno::Reference< uno::XInterface >() );
}

void SAL_CALL
ScVbaLineFormat::setEndArrowheadWidth( sal_Int32 /*_endarrowheadwidth*/ ) throw (uno::RuntimeException)
{
    throw uno::RuntimeException( rtl::OUString::createFromAscii("Property 'EndArrowheadWidth' is not supported."), uno::Reference< uno::XInterface >() );
}

double SAL_CALL
ScVbaLineFormat::getWeight() throw (uno::RuntimeException)
{
    sal_Int32 nLineWidth=0;
    m_xPropertySet->getPropertyValue( rtl::OUString::createFromAscii( "LineWidth") ) >>= nLineWidth;
    double dLineWidth = Millimeter::getInPoints( nLineWidth );
    return dLineWidth;
}

void SAL_CALL
ScVbaLineFormat::setWeight( double _weight ) throw (uno::RuntimeException)
{
    if( _weight < 0 )
        throw uno::RuntimeException( rtl::OUString::createFromAscii("Parameter: Must be positv."), uno::Reference< uno::XInterface >() );
    if( _weight == 0 )
        _weight = 0.5;
    m_nLineWeight = _weight;
    Millimeter aMillimeter;
    aMillimeter.setInPoints( _weight );

    sal_Int32 nLineWidth = static_cast<sal_Int32>( aMillimeter.getInHundredthsOfOneMillimeter() );
    m_xPropertySet->setPropertyValue( rtl::OUString::createFromAscii( "LineWidth" ), uno::makeAny( nLineWidth ) );
    setDashStyle( m_nLineDashStyle );
}

sal_Bool SAL_CALL
ScVbaLineFormat::getVisible() throw (uno::RuntimeException)
{
    drawing::LineStyle aLineStyle;
    m_xPropertySet->getPropertyValue( rtl::OUString::createFromAscii( "LineStyle" ) ) >>= aLineStyle;
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
    m_xPropertySet->getPropertyValue( rtl::OUString::createFromAscii( "LineStyle" ) ) >>= aLineStyle;
    if( !_visible )
    {
        aLineStyle = drawing::LineStyle_NONE;
        m_xPropertySet->setPropertyValue( rtl::OUString::createFromAscii( "LineStyle" ), uno::makeAny( aLineStyle ) );
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
    m_xPropertySet->getPropertyValue( rtl::OUString::createFromAscii( "LineTransparence" ) ) >>= nTransparency;
    double fTransparency = static_cast<double>( nTransparency );
    return fTransparency / 100;
}

void SAL_CALL
ScVbaLineFormat::setTransparency( double _transparency ) throw (uno::RuntimeException)
{
    sal_Int16 nTransparency = static_cast<sal_Int16>( _transparency * 100 );
    m_xPropertySet->setPropertyValue( rtl::OUString::createFromAscii( "LineTransparence" ), uno::makeAny( nTransparency ) );
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
    m_xPropertySet->getPropertyValue( rtl::OUString::createFromAscii( "LineStyle" ) ) >>= eLineStyle;
    if( eLineStyle == drawing::LineStyle_SOLID )
        m_nLineDashStyle = office::MsoLineDashStyle::msoLineSolid;
    else
    {
        drawing::LineDash aLineDash;
        m_xPropertySet->getPropertyValue( rtl::OUString::createFromAscii( "LineDash" ) ) >>= aLineDash;
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
        m_xPropertySet->setPropertyValue( rtl::OUString::createFromAscii( "LineStyle" ), uno::makeAny( drawing::LineStyle_SOLID  ));
    }
    else
    {
        m_xPropertySet->setPropertyValue( rtl::OUString::createFromAscii( "LineStyle" ), uno::makeAny( drawing::LineStyle_DASH ) );
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
            throw uno::RuntimeException( rtl::OUString::createFromAscii("this MsoLineDashStyle is not supported."), uno::Reference< uno::XInterface >() );
            break;
        }
        m_xPropertySet->setPropertyValue( rtl::OUString::createFromAscii( "LineDash" ), uno::makeAny( pLineDash ) );
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
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("org.openoffice.msform.LineFormat" ) );
    }
    return aServiceNames;
}


