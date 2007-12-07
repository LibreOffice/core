/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbapictureformat.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 10:58:26 $
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
#include "vbapictureformat.hxx"

using namespace org::openoffice;
using namespace com::sun::star;

ScVbaPictureFormat::ScVbaPictureFormat( const css::uno::Reference< oo::vba::XHelperInterface >& xParent,
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
        throw uno::RuntimeException( rtl::OUString::createFromAscii("Parameter out of range, value is too small.") , uno::Reference< uno::XInterface >() );
    }
    if( nRange > nMax )
    {
        throw uno::RuntimeException( rtl::OUString::createFromAscii("Parameter out of range, value is too high.") , uno::Reference< uno::XInterface >() );
    }
}

// Attributes
double SAL_CALL
ScVbaPictureFormat::getBrightness() throw (uno::RuntimeException)
{
    sal_Int16 nLuminance = 0;
    m_xPropertySet->getPropertyValue( rtl::OUString::createFromAscii("AdjustLuminance") ) >>= nLuminance;
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
    m_xPropertySet->setPropertyValue( rtl::OUString::createFromAscii("AdjustLuminance"), uno::makeAny( nLuminance ) );
}

double SAL_CALL
ScVbaPictureFormat::getContrast() throw (uno::RuntimeException)
{
    sal_Int16 nContrast = 0;
    m_xPropertySet->getPropertyValue( rtl::OUString::createFromAscii("AdjustContrast") ) >>= nContrast;
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
    m_xPropertySet->setPropertyValue( rtl::OUString::createFromAscii("AdjustContrast"), uno::makeAny( nContrast ) );
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
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("org.openoffice.msform.PictureFormat" ) );
    }
    return aServiceNames;
}
