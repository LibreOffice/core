/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbatextbox.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 11:03:50 $
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
#include <com/sun/star/text/XTextRange.hpp>

#include "vbatextbox.hxx"
#include <vector>

using namespace com::sun::star;
using namespace org::openoffice;



ScVbaTextBox::ScVbaTextBox( const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< css::drawing::XControlShape >& xControlShape ) : TextBoxImpl_BASE( xContext, xControlShape )
{
}

// Attributes
//getString() will cause some imfo lose.
rtl::OUString SAL_CALL
ScVbaTextBox::getText() throw (css::uno::RuntimeException)
{
    uno::Any aValue;
    aValue = m_xProps->getPropertyValue
            (rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Text" ) ) );
    rtl::OUString sString;
    aValue >>= sString;
    return sString;
}

void SAL_CALL
ScVbaTextBox::setText( const rtl::OUString& _text ) throw (css::uno::RuntimeException)
{
    uno::Reference< text::XTextRange > xTextRange( m_xProps, uno::UNO_QUERY_THROW );
    xTextRange->setString( _text );
}

sal_Int32 SAL_CALL
ScVbaTextBox::getMaxLength() throw (css::uno::RuntimeException)
{
    uno::Any aValue;
    aValue = m_xProps->getPropertyValue
            (rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MaxTextLen" ) ) );
    sal_Int32 nMaxLength = 0;
    aValue >>= nMaxLength;
    return nMaxLength;
}

void SAL_CALL
ScVbaTextBox::setMaxLength( sal_Int32 _maxlength ) throw (css::uno::RuntimeException)
{
    uno::Any aValue( _maxlength );
    m_xProps->setPropertyValue
            (rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MaxTextLen" ) ), aValue);
}

sal_Bool SAL_CALL
ScVbaTextBox::getMultiline() throw (css::uno::RuntimeException)
{
    uno::Any aValue;
    aValue = m_xProps->getPropertyValue
            (rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MultiLine" ) ) );
    sal_Bool bRet = false;
    aValue >>= bRet;
    return bRet;
}

void SAL_CALL
ScVbaTextBox::setMultiline( sal_Bool _multiline ) throw (css::uno::RuntimeException)
{
    uno::Any aValue( _multiline );
    m_xProps->setPropertyValue
            (rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MultiLine" ) ), aValue);
}
