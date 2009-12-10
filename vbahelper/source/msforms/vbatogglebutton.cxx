/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
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
#include "vbatogglebutton.hxx"
#include <vector>

using namespace com::sun::star;
using namespace ooo::vba;


const static rtl::OUString LABEL( RTL_CONSTASCII_USTRINGPARAM("Label") );
const static rtl::OUString TOGGLE( RTL_CONSTASCII_USTRINGPARAM("Toggle") );
const static rtl::OUString STATE( RTL_CONSTASCII_USTRINGPARAM("State") );
ScVbaToggleButton::ScVbaToggleButton( const css::uno::Reference< ov::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< uno::XInterface >& xControl, const uno::Reference< frame::XModel >& xModel, ov::AbstractGeometryAttributes* pGeomHelper ) : ToggleButtonImpl_BASE( xParent, xContext, xControl, xModel, pGeomHelper )
{
    OSL_TRACE("ScVbaToggleButton(ctor)");
    m_xProps->setPropertyValue( TOGGLE, uno::makeAny( sal_True ) );
}

ScVbaToggleButton::~ScVbaToggleButton()
{
    OSL_TRACE("~ScVbaToggleButton(dtor)");
}

// Attributes
rtl::OUString SAL_CALL
ScVbaToggleButton::getCaption() throw (css::uno::RuntimeException)
{
    rtl::OUString Label;
    m_xProps->getPropertyValue( LABEL ) >>= Label;
    return Label;
}

void SAL_CALL
ScVbaToggleButton::setCaption( const rtl::OUString& _caption ) throw (::com::sun::star::uno::RuntimeException)
{
    m_xProps->setPropertyValue( LABEL, uno::makeAny( _caption ) );
}

uno::Any SAL_CALL
ScVbaToggleButton::getValue() throw (uno::RuntimeException)
{
    sal_Int16 nState = 0;
        m_xProps->getPropertyValue( STATE ) >>= nState;
     return uno::makeAny( nState ? sal_Int16( -1 ) : sal_Int16( 0 ) );
}

void SAL_CALL
ScVbaToggleButton::setValue( const uno::Any& _value ) throw (uno::RuntimeException)
{
    sal_Int16 nState = 0;
    _value >>= nState;
        OSL_TRACE( "nState - %d", nState );
    nState = ( nState == -1 ) ?  1 : 0;
        OSL_TRACE( "nState - %d", nState );
    m_xProps->setPropertyValue( STATE, uno::makeAny(  nState ) );
}

rtl::OUString&
ScVbaToggleButton::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaToggleButton") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaToggleButton::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.msforms.ToggleButton" ) );
    }
    return aServiceNames;
}

