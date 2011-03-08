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
#include <com/sun/star/text/XTextRange.hpp>

#include "vbatextbox.hxx"
#include <vector>

using namespace com::sun::star;
using namespace ooo::vba;



ScVbaTextBox::ScVbaTextBox( const uno::Reference< ov::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< uno::XInterface >& xControl, const uno::Reference< frame::XModel >& xModel, AbstractGeometryAttributes* pGeomHelper, bool bDialog ) : TextBoxImpl_BASE( xParent, xContext, xControl, xModel, pGeomHelper ), mbDialog( bDialog )
{
}

// Attributes
uno::Any SAL_CALL
ScVbaTextBox::getValue() throw (css::uno::RuntimeException)
{
    return uno::makeAny( getText() );
}

void SAL_CALL
ScVbaTextBox::setValue( const uno::Any& _value ) throw (css::uno::RuntimeException)
{
    rtl::OUString sVal = getAnyAsString( _value );
    setText( sVal );
}

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
    rtl::OUString sOldText = getText();

    if ( !mbDialog )
    {
        uno::Reference< text::XTextRange > xTextRange( m_xProps, uno::UNO_QUERY_THROW );
        xTextRange->setString( _text );
    }
    else
        m_xProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Text") ), uno::makeAny( _text ) );

    if ( _text != sOldText )
    {
        fireChangeEvent();
    }
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
    sal_Int16 _maxlength16 = static_cast<sal_Int16> (_maxlength);
    uno::Any aValue( _maxlength16 );
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

rtl::OUString&
ScVbaTextBox::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaTextBox") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaTextBox::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.msforms.TextBox" ) );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
