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

#include <rtl/tencinfo.h>
#include "vbanewfont.hxx"
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

using namespace ::com::sun::star;
using namespace ::ooo::vba;


VbaNewFont::VbaNewFont(
        const uno::Reference< beans::XPropertySet >& rxModelProps ) :
    mxProps( rxModelProps, uno::UNO_SET_THROW )
{
}

// XNewFont attributes

OUString SAL_CALL VbaNewFont::getName()
{
    uno::Any aAny = mxProps->getPropertyValue( u"FontName"_ustr );
    return aAny.get< OUString >();
}

void SAL_CALL VbaNewFont::setName( const OUString& rName )
{
    mxProps->setPropertyValue( u"FontName"_ustr , uno::Any( rName ) );
}

double SAL_CALL VbaNewFont::getSize()
{
    uno::Any aAny = mxProps->getPropertyValue( u"FontHeight"_ustr );
    return aAny.get< float >();
}

void SAL_CALL VbaNewFont::setSize( double fSize )
{
    mxProps->setPropertyValue( u"FontHeight"_ustr , uno::Any( static_cast< float >( fSize ) ) );
}

sal_Int16 SAL_CALL VbaNewFont::getCharset()
{
    uno::Any aAny = mxProps->getPropertyValue( u"FontCharset"_ustr );
    return rtl_getBestWindowsCharsetFromTextEncoding( static_cast< rtl_TextEncoding >( aAny.get< sal_Int16 >() ) );
}

void SAL_CALL VbaNewFont::setCharset( sal_Int16 nCharset )
{
    rtl_TextEncoding eFontEnc = RTL_TEXTENCODING_DONTKNOW;
    if( (0 <= nCharset) && (nCharset <= SAL_MAX_UINT8) )
        eFontEnc = rtl_getTextEncodingFromWindowsCharset( static_cast< sal_uInt8 >( nCharset ) );
    if( eFontEnc == RTL_TEXTENCODING_DONTKNOW )
        throw uno::RuntimeException(u"an unknown or missing encoding"_ustr);
    mxProps->setPropertyValue( u"FontCharset"_ustr , uno::Any( static_cast< sal_Int16 >( eFontEnc ) ) );
}

sal_Int16 SAL_CALL VbaNewFont::getWeight()
{
    return getBold() ? 700 : 400;
}

void SAL_CALL VbaNewFont::setWeight( sal_Int16 nWeight )
{
    setBold( nWeight >= 700 );
}

sal_Bool SAL_CALL VbaNewFont::getBold()
{
    uno::Any aAny = mxProps->getPropertyValue( u"FontWeight"_ustr );
    return aAny.get< float >() > awt::FontWeight::NORMAL;
}

void SAL_CALL VbaNewFont::setBold( sal_Bool bBold )
{
    mxProps->setPropertyValue( u"FontWeight"_ustr , uno::Any( bBold ? awt::FontWeight::BOLD : awt::FontWeight::NORMAL ) );
}

sal_Bool SAL_CALL VbaNewFont::getItalic()
{
    uno::Any aAny = mxProps->getPropertyValue( u"FontSlant"_ustr );
    return aAny.get< awt::FontSlant >() != awt::FontSlant_NONE;
}

void SAL_CALL VbaNewFont::setItalic( sal_Bool bItalic )
{
    mxProps->setPropertyValue( u"FontSlant"_ustr , uno::Any( bItalic ? awt::FontSlant_ITALIC : awt::FontSlant_NONE ) );
}

sal_Bool SAL_CALL VbaNewFont::getUnderline()
{
    uno::Any aAny = mxProps->getPropertyValue(u"FontUnderline"_ustr );
    return aAny.get< sal_Int16 >() != awt::FontUnderline::NONE;
}

void SAL_CALL VbaNewFont::setUnderline( sal_Bool bUnderline )
{
    mxProps->setPropertyValue(u"FontUnderline"_ustr , uno::Any( bUnderline ? awt::FontUnderline::SINGLE : awt::FontUnderline::NONE ) );
}

sal_Bool SAL_CALL VbaNewFont::getStrikethrough()
{
    uno::Any aAny = mxProps->getPropertyValue( u"FontStrikeout"_ustr );
    return aAny.get< sal_Int16 >() != awt::FontStrikeout::NONE;
}

void SAL_CALL VbaNewFont::setStrikethrough( sal_Bool bStrikethrough )
{
    mxProps->setPropertyValue( u"FontStrikeout"_ustr ,uno::Any( bStrikethrough ? awt::FontStrikeout::SINGLE : awt::FontStrikeout::NONE ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
