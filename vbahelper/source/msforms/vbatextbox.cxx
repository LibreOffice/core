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

#include "vbatextbox.hxx"
#include "vbanewfont.hxx"
#include <com/sun/star/text/XTextRange.hpp>
#include <ooo/vba/msforms/fmBorderStyle.hpp>
#include <ooo/vba/msforms/fmSpecialEffect.hpp>

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
    // booleans are converted to uppercase strings
    OUString sVal = extractStringFromAny( _value, true );
    setText( sVal );
}

//getString() will cause some imfo lose.
OUString SAL_CALL
ScVbaTextBox::getText() throw (css::uno::RuntimeException)
{
    uno::Any aValue;
    aValue = m_xProps->getPropertyValue( "Text" );
    OUString sString;
    aValue >>= sString;
    return sString;
}

void SAL_CALL
ScVbaTextBox::setText( const OUString& _text ) throw (css::uno::RuntimeException)
{
    OUString sOldText = getText();

    if ( !mbDialog )
    {
        uno::Reference< text::XTextRange > xTextRange( m_xProps, uno::UNO_QUERY_THROW );
        xTextRange->setString( _text );
    }
    else
        m_xProps->setPropertyValue( "Text" , uno::makeAny( _text ) );

    if ( _text != sOldText )
    {
        fireChangeEvent();
    }
}

sal_Int32 SAL_CALL
ScVbaTextBox::getMaxLength() throw (css::uno::RuntimeException)
{
    uno::Any aValue;
    aValue = m_xProps->getPropertyValue( "MaxTextLen" );
    sal_Int32 nMaxLength = 0;
    aValue >>= nMaxLength;
    return nMaxLength;
}

void SAL_CALL
ScVbaTextBox::setMaxLength( sal_Int32 _maxlength ) throw (css::uno::RuntimeException)
{
    sal_Int16 _maxlength16 = static_cast<sal_Int16> (_maxlength);
    uno::Any aValue( _maxlength16 );
    m_xProps->setPropertyValue( "MaxTextLen" , aValue);
}

sal_Bool SAL_CALL
ScVbaTextBox::getMultiline() throw (css::uno::RuntimeException)
{
    uno::Any aValue;
    aValue = m_xProps->getPropertyValue( "MultiLine" );
    sal_Bool bRet = false;
    aValue >>= bRet;
    return bRet;
}

void SAL_CALL
ScVbaTextBox::setMultiline( sal_Bool _multiline ) throw (css::uno::RuntimeException)
{
    uno::Any aValue( _multiline );
    m_xProps->setPropertyValue( "MultiLine" , aValue);
}

sal_Int32 SAL_CALL ScVbaTextBox::getSpecialEffect() throw (uno::RuntimeException)
{
    return msforms::fmSpecialEffect::fmSpecialEffectSunken;
}

void SAL_CALL ScVbaTextBox::setSpecialEffect( sal_Int32 /*nSpecialEffect*/ ) throw (uno::RuntimeException)
{
    // #STUB
}

sal_Int32 SAL_CALL ScVbaTextBox::getBorderStyle() throw (uno::RuntimeException)
{
    return msforms::fmBorderStyle::fmBorderStyleNone;
}

void SAL_CALL ScVbaTextBox::setBorderStyle( sal_Int32 /*nBorderStyle*/ ) throw (uno::RuntimeException)
{
    // #STUB
}

sal_Int32 SAL_CALL ScVbaTextBox::getTextLength() throw (uno::RuntimeException)
{
    return getText().getLength();
}

uno::Reference< msforms::XNewFont > SAL_CALL ScVbaTextBox::getFont() throw (uno::RuntimeException)
{
    return new VbaNewFont( this, mxContext, m_xProps );
}

sal_Int32 SAL_CALL ScVbaTextBox::getBackColor() throw (uno::RuntimeException)
{
    return ScVbaControl::getBackColor();
}

void SAL_CALL ScVbaTextBox::setBackColor( sal_Int32 nBackColor ) throw (uno::RuntimeException)
{
    ScVbaControl::setBackColor( nBackColor );
}

sal_Bool SAL_CALL ScVbaTextBox::getAutoSize() throw (uno::RuntimeException)
{
    return ScVbaControl::getAutoSize();
}

void SAL_CALL ScVbaTextBox::setAutoSize( sal_Bool bAutoSize ) throw (uno::RuntimeException)
{
    ScVbaControl::setAutoSize( bAutoSize );
}

sal_Bool SAL_CALL ScVbaTextBox::getLocked() throw (uno::RuntimeException)
{
    return ScVbaControl::getLocked();
}

void SAL_CALL ScVbaTextBox::setLocked( sal_Bool bLocked ) throw (uno::RuntimeException)
{
    ScVbaControl::setLocked( bLocked );
}

OUString
ScVbaTextBox::getServiceImplName()
{
    return OUString( "ScVbaTextBox" );
}

uno::Sequence< OUString >
ScVbaTextBox::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.msforms.TextBox";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
