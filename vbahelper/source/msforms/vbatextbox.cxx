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

ScVbaTextBox::ScVbaTextBox( const uno::Reference< ov::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< uno::XInterface >& xControl, const uno::Reference< frame::XModel >& xModel, std::unique_ptr<AbstractGeometryAttributes> pGeomHelper, bool bDialog )
    : TextBoxImpl_BASE( xParent, xContext, xControl, xModel, std::move(pGeomHelper) ), mbDialog( bDialog )
{
}

// Attributes
uno::Any SAL_CALL
ScVbaTextBox::getValue()
{
    return uno::Any( getText() );
}

void SAL_CALL
ScVbaTextBox::setValue( const uno::Any& _value )
{
    // booleans are converted to uppercase strings
    OUString sVal = extractStringFromAny( _value, true );
    setText( sVal );
}

//getString() will cause some info lose.
OUString SAL_CALL
ScVbaTextBox::getText()
{
    uno::Any aValue = m_xProps->getPropertyValue( u"Text"_ustr );
    OUString sString;
    aValue >>= sString;
    return sString;
}

void SAL_CALL
ScVbaTextBox::setText( const OUString& _text )
{
    OUString oldText( getText() );
    if ( !mbDialog )
    {
        uno::Reference< text::XTextRange > xTextRange( m_xProps, uno::UNO_QUERY_THROW );
        xTextRange->setString( _text );
    }
    else
        m_xProps->setPropertyValue( u"Text"_ustr , uno::Any( _text ) );
    if ( oldText != _text )
        fireChangeEvent();
}

sal_Int32 SAL_CALL
ScVbaTextBox::getMaxLength()
{
    uno::Any aValue = m_xProps->getPropertyValue( u"MaxTextLen"_ustr );
    sal_Int16 nMaxLength = 0;
    aValue >>= nMaxLength;
    return static_cast<sal_Int32>(nMaxLength);
}

void SAL_CALL
ScVbaTextBox::setMaxLength( sal_Int32 _maxlength )
{
    sal_Int16 nTmp( _maxlength );
    uno::Any aValue( nTmp );
    m_xProps->setPropertyValue( u"MaxTextLen"_ustr , aValue);
}

sal_Bool SAL_CALL
ScVbaTextBox::getMultiline()
{
    uno::Any aValue = m_xProps->getPropertyValue( u"MultiLine"_ustr );
    bool bRet = false;
    aValue >>= bRet;
    return bRet;
}

void SAL_CALL
ScVbaTextBox::setMultiline( sal_Bool _multiline )
{
    uno::Any aValue( _multiline );
    m_xProps->setPropertyValue( u"MultiLine"_ustr , aValue);
}

sal_Int32 SAL_CALL ScVbaTextBox::getSpecialEffect()
{
    return msforms::fmSpecialEffect::fmSpecialEffectSunken;
}

void SAL_CALL ScVbaTextBox::setSpecialEffect( sal_Int32 /*nSpecialEffect*/ )
{
    // #STUB
}

sal_Int32 SAL_CALL ScVbaTextBox::getBorderStyle()
{
    return msforms::fmBorderStyle::fmBorderStyleNone;
}

void SAL_CALL ScVbaTextBox::setBorderStyle( sal_Int32 /*nBorderStyle*/ )
{
    // #STUB
}

sal_Int32 SAL_CALL ScVbaTextBox::getTextLength()
{
    return getText().getLength();
}

uno::Reference< msforms::XNewFont > SAL_CALL ScVbaTextBox::getFont()
{
    return new VbaNewFont( m_xProps );
}

sal_Int32 SAL_CALL ScVbaTextBox::getBackColor()
{
    return ScVbaControl::getBackColor();
}

void SAL_CALL ScVbaTextBox::setBackColor( sal_Int32 nBackColor )
{
    ScVbaControl::setBackColor( nBackColor );
}

sal_Bool SAL_CALL ScVbaTextBox::getAutoSize()
{
    return ScVbaControl::getAutoSize();
}

void SAL_CALL ScVbaTextBox::setAutoSize( sal_Bool bAutoSize )
{
    ScVbaControl::setAutoSize( bAutoSize );
}

sal_Bool SAL_CALL ScVbaTextBox::getLocked()
{
    return ScVbaControl::getLocked();
}

void SAL_CALL ScVbaTextBox::setLocked( sal_Bool bLocked )
{
    ScVbaControl::setLocked( bLocked );
}

OUString
ScVbaTextBox::getServiceImplName()
{
    return u"ScVbaTextBox"_ustr;
}

uno::Sequence< OUString >
ScVbaTextBox::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.msforms.TextBox"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
