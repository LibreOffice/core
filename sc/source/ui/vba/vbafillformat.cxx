/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbafillformat.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 10:51:04 $
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
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/awt/GradientStyle.hpp>
#include <org/openoffice/office/MsoGradientStyle.hpp>
#include "vbafillformat.hxx"
#include "vbacolorformat.hxx"

using namespace org::openoffice;
using namespace com::sun::star;

ScVbaFillFormat::ScVbaFillFormat( const uno::Reference< vba::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< drawing::XShape > xShape ) : ScVbaFillFormat_BASE( xParent, xContext ), m_xShape( xShape )
{
    m_xPropertySet.set( xShape, uno::UNO_QUERY_THROW );
    m_nFillStyle = drawing::FillStyle_SOLID;
    m_nForeColor = 0;
    m_nBackColor = 0;
    m_nGradientAngle = 0;
}

void
ScVbaFillFormat::setFillStyle( drawing::FillStyle nFillStyle ) throw (uno::RuntimeException)
{
    m_nFillStyle = nFillStyle;
    if( m_nFillStyle == drawing::FillStyle_GRADIENT )
    {
        m_xPropertySet->setPropertyValue( rtl::OUString::createFromAscii("FillStyle"), uno::makeAny( drawing::FillStyle_GRADIENT ) );
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
        m_xPropertySet->setPropertyValue( rtl::OUString::createFromAscii("FillGradient"), uno::makeAny( aGradient ) );
    }
    else if( m_nFillStyle == drawing::FillStyle_SOLID )
    {
        m_xPropertySet->setPropertyValue( rtl::OUString::createFromAscii("FillStyle"), uno::makeAny(drawing::FillStyle_SOLID) );
    }
}

void
ScVbaFillFormat::setForeColorAndInternalStyle( sal_Int32 nForeColor ) throw (css::uno::RuntimeException)
{
    m_nForeColor = nForeColor;
    setFillStyle( m_nFillStyle );
}

// Attributes
sal_Bool SAL_CALL
ScVbaFillFormat::getVisible() throw (uno::RuntimeException)
{
    drawing::FillStyle nFillStyle;
    m_xPropertySet->getPropertyValue( rtl::OUString::createFromAscii("FillStyle") ) >>= nFillStyle;
    if( nFillStyle == drawing::FillStyle_NONE )
        return sal_False;
    return sal_True;
}

void SAL_CALL
ScVbaFillFormat::setVisible( sal_Bool _visible ) throw (uno::RuntimeException)
{
    drawing::FillStyle aFillStyle;
    m_xPropertySet->getPropertyValue( rtl::OUString::createFromAscii("FillStyle") ) >>= aFillStyle;
    if( !_visible )
    {
        m_xPropertySet->setPropertyValue( rtl::OUString::createFromAscii("FillStyle"), uno::makeAny( drawing::FillStyle_NONE ) );
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
ScVbaFillFormat::getTransparency() throw (uno::RuntimeException)
{
    sal_Int16 nTransparence = 0;
    double dTransparence = 0;
    m_xPropertySet->getPropertyValue( rtl::OUString::createFromAscii( "FillTransparence" ) ) >>= nTransparence;
    dTransparence = static_cast<double>( nTransparence );
    dTransparence /= 100;
    return dTransparence;
}

void SAL_CALL
ScVbaFillFormat::setTransparency( double _transparency ) throw (uno::RuntimeException)
{
    sal_Int16 nTransparence = static_cast< sal_Int16 >( _transparency * 100 );
    m_xPropertySet->setPropertyValue( rtl::OUString::createFromAscii( "FillTransparence" ), uno::makeAny( nTransparence ) );
}


// Methods
void SAL_CALL
ScVbaFillFormat::Solid() throw (uno::RuntimeException)
{
    setFillStyle( drawing::FillStyle_SOLID );
}

void SAL_CALL
ScVbaFillFormat::TwoColorGradient( sal_Int32 style, sal_Int32 /*variant*/ ) throw (uno::RuntimeException)
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
ScVbaFillFormat::BackColor() throw (uno::RuntimeException)
{
    if( !m_xColorFormat.is() )
        m_xColorFormat.set( new ScVbaColorFormat( getParent(), mxContext, this, m_xShape, ColorFormatType::FILLFORMAT_BACKCOLOR ) );
    return m_xColorFormat;
}

uno::Reference< msforms::XColorFormat > SAL_CALL
ScVbaFillFormat::ForeColor() throw (uno::RuntimeException)
{
    if( !m_xColorFormat.is() )
        m_xColorFormat.set( new ScVbaColorFormat( getParent(), mxContext, this, m_xShape, ColorFormatType::FILLFORMAT_FORECOLOR ) );
    return m_xColorFormat;
}


rtl::OUString&
ScVbaFillFormat::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaFillFormat") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaFillFormat::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("org.openoffice.msforms.FillFormat" ) );
    }
    return aServiceNames;
}

