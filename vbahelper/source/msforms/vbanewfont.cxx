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

#include <vbanewfont.hxx>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>

using namespace ::com::sun::star;
using namespace ::ooo::vba;

// ============================================================================

VbaNewFont::VbaNewFont(
        const uno::Reference< XHelperInterface >& rxParent,
        const uno::Reference< uno::XComponentContext >& rxContext,
        const uno::Reference< beans::XPropertySet >& rxModelProps ) throw (uno::RuntimeException) :
    VbaNewFont_BASE( rxParent, rxContext ),
    mxProps( rxModelProps, uno::UNO_SET_THROW )
{
}

// XNewFont attributes

OUString SAL_CALL VbaNewFont::getName() throw (uno::RuntimeException)
{
    uno::Any aAny = mxProps->getPropertyValue( "FontName" );
    return aAny.get< OUString >();
}

void SAL_CALL VbaNewFont::setName( const OUString& rName ) throw (uno::RuntimeException)
{
    mxProps->setPropertyValue( "FontName" , uno::Any( rName ) );
}

double SAL_CALL VbaNewFont::getSize() throw (uno::RuntimeException)
{
    uno::Any aAny = mxProps->getPropertyValue( "FontHeight" );
    return aAny.get< float >();
}

void SAL_CALL VbaNewFont::setSize( double fSize ) throw (uno::RuntimeException)
{
    mxProps->setPropertyValue( "FontHeight" , uno::Any( static_cast< float >( fSize ) ) );
}

sal_Int16 SAL_CALL VbaNewFont::getCharset() throw (uno::RuntimeException)
{
    uno::Any aAny = mxProps->getPropertyValue( "FontCharset" );
    return rtl_getBestWindowsCharsetFromTextEncoding( static_cast< rtl_TextEncoding >( aAny.get< sal_Int16 >() ) );
}

void SAL_CALL VbaNewFont::setCharset( sal_Int16 nCharset ) throw (uno::RuntimeException)
{
    rtl_TextEncoding eFontEnc = RTL_TEXTENCODING_DONTKNOW;
    if( (0 <= nCharset) && (nCharset <= SAL_MAX_UINT8) )
        eFontEnc = rtl_getTextEncodingFromWindowsCharset( static_cast< sal_uInt8 >( nCharset ) );
    if( eFontEnc == RTL_TEXTENCODING_DONTKNOW )
        throw uno::RuntimeException();
    mxProps->setPropertyValue( "FontCharset" , uno::Any( static_cast< sal_Int16 >( eFontEnc ) ) );
}

sal_Int16 SAL_CALL VbaNewFont::getWeight() throw (uno::RuntimeException)
{
    return getBold() ? 700 : 400;
}

void SAL_CALL VbaNewFont::setWeight( sal_Int16 nWeight ) throw (uno::RuntimeException)
{
    setBold( nWeight >= 700 );
}

sal_Bool SAL_CALL VbaNewFont::getBold() throw (uno::RuntimeException)
{
    uno::Any aAny = mxProps->getPropertyValue( "FontWeight" );
    return aAny.get< float >() > awt::FontWeight::NORMAL;
}

void SAL_CALL VbaNewFont::setBold( sal_Bool bBold ) throw (uno::RuntimeException)
{
    mxProps->setPropertyValue( "FontWeight" , uno::Any( bBold ? awt::FontWeight::BOLD : awt::FontWeight::NORMAL ) );
}

sal_Bool SAL_CALL VbaNewFont::getItalic() throw (uno::RuntimeException)
{
    uno::Any aAny = mxProps->getPropertyValue( "FontSlant" );
    return aAny.get< awt::FontSlant >() != awt::FontSlant_NONE;
}

void SAL_CALL VbaNewFont::setItalic( sal_Bool bItalic ) throw (uno::RuntimeException)
{
    mxProps->setPropertyValue( "FontSlant" , uno::Any( bItalic ? awt::FontSlant_ITALIC : awt::FontSlant_NONE ) );
}

sal_Bool SAL_CALL VbaNewFont::getUnderline() throw (uno::RuntimeException)
{
    uno::Any aAny = mxProps->getPropertyValue("FontUnderline" );
    return aAny.get< sal_Int16 >() != awt::FontUnderline::NONE;
}

void SAL_CALL VbaNewFont::setUnderline( sal_Bool bUnderline ) throw (uno::RuntimeException)
{
    mxProps->setPropertyValue("FontUnderline" , uno::Any( bUnderline ? awt::FontUnderline::SINGLE : awt::FontUnderline::NONE ) );
}

sal_Bool SAL_CALL VbaNewFont::getStrikethrough() throw (uno::RuntimeException)
{
    uno::Any aAny = mxProps->getPropertyValue( "FontStrikeout" );
    return aAny.get< sal_Int16 >() != awt::FontStrikeout::NONE;
}

void SAL_CALL VbaNewFont::setStrikethrough( sal_Bool bStrikethrough ) throw (uno::RuntimeException)
{
    mxProps->setPropertyValue( "FontStrikeout" ,uno::Any( bStrikethrough ? awt::FontStrikeout::SINGLE : awt::FontStrikeout::NONE ) );
}

// XHelperInterface

VBAHELPER_IMPL_XHELPERINTERFACE( VbaNewFont, "ooo.vba.msforms.NewFont" )

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
