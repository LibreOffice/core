/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbaradiobutton.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 11:00:34 $
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
#include "vbaradiobutton.hxx"
#include <vector>

using namespace com::sun::star;
using namespace org::openoffice;


const static rtl::OUString LABEL( RTL_CONSTASCII_USTRINGPARAM("Label") );
const static rtl::OUString STATE( RTL_CONSTASCII_USTRINGPARAM("State") );
ScVbaRadioButton::ScVbaRadioButton( const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< css::drawing::XControlShape >& xControlShape ) : RadioButtonImpl_BASE( xContext, xControlShape )
{
}

// Attributes
rtl::OUString SAL_CALL
ScVbaRadioButton::getCaption() throw (css::uno::RuntimeException)
{
    rtl::OUString Label;
    m_xProps->getPropertyValue( LABEL ) >>= Label;
    return Label;
}

void SAL_CALL
ScVbaRadioButton::setCaption( const rtl::OUString& _caption ) throw (::com::sun::star::uno::RuntimeException)
{
    m_xProps->setPropertyValue( LABEL, uno::makeAny( _caption ) );
}

sal_Bool SAL_CALL
ScVbaRadioButton::getValue() throw (css::uno::RuntimeException)
{
    sal_Bool bValue = sal_False;
    sal_Int16 nValue = -1;
    m_xProps->getPropertyValue( STATE ) >>= nValue;
    if( nValue != 0 )
        bValue = sal_True;
    return bValue;
}

void SAL_CALL
ScVbaRadioButton::setValue( sal_Bool _value ) throw (css::uno::RuntimeException)
{
    sal_Int16 nValue = 0;
    if( _value )
        nValue = 1;
    m_xProps->setPropertyValue( STATE, uno::makeAny( nValue ) );
}
