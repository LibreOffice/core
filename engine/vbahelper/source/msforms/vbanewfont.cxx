/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

OUString VbaNewFont::getName()
{
    cpo::uno::Any aAny = mxProps->getPropertyValue( u"FontName"_ustr );
    return aAny.get< OUString >();
}

void VbaNewFont::setName( const OUString& rName )
{
    mxProps->setPropertyValue( u"FontName"_ustr , cpo::uno::Any( rName ) );
}

double VbaNewFont::getSize()
{
    cpo::uno::Any aAny = mxProps->getPropertyValue( u"FontHeight"_ustr );
    return aAny.get< float >();
}

void VbaNewFont::setSize( double fSize )
{
    mxProps->setPropertyValue( u"FontHeight"_ustr , cpo::uno::Any( static_cast< float >( fSize ) ) );
}

sal_Int16 VbaNewFont::getCharset()
{
    cpo::uno::Any aAny = mxProps->getPropertyValue( u"FontCharset"_ustr );
    return rtl_getBestWindowsCharsetFromTextEncoding( static_cast< rtl_TextEncoding >( aAny.get< sal_Int16 >() ) );
}

void VbaNewFont::setCharset( sal_Int16 nCharset )
{
    rtl_TextEncoding eFontEnc = RTL_TEXTENCODING_DONTKNOW;
    if( (0 <= nCharset) && (nCharset <= SAL_MAX_UINT8) )
        eFontEnc = rtl_getTextEncodingFromWindowsCharset( static_cast< sal_uInt8 >( nCharset ) );
    if( eFontEnc == RTL_TEXTENCODING_DONTKNOW )
        throw cpo::uno::RuntimeException(u"an unknown or missing encoding"_ustr);
    mxProps->setPropertyValue( u"FontCharset"_ustr , cpo::uno::Any( static_cast< sal_Int16 >( eFontEnc ) ) );
}

sal_Int16 VbaNewFont::getWeight()
{
    return getBold() ? 700 : 400;
}

void VbaNewFont::setWeight( sal_Int16 nWeight )
{
    setBold( nWeight >= 700 );
}

bool VbaNewFont::getBold()
{
    cpo::uno::Any aAny = mxProps->getPropertyValue( u"FontWeight"_ustr );
    return aAny.get< float >() > awt::FontWeight::NORMAL;
}

void VbaNewFont::setBold( bool bBold )
{
    mxProps->setPropertyValue( u"FontWeight"_ustr , cpo::uno::Any( bBold ? awt::FontWeight::BOLD : awt::FontWeight::NORMAL ) );
}

bool VbaNewFont::getItalic()
{
    cpo::uno::Any aAny = mxProps->getPropertyValue( u"FontSlant"_ustr );
    return aAny.get< awt::FontSlant >() != awt::FontSlant_NONE;
}

void VbaNewFont::setItalic( bool bItalic )
{
    mxProps->setPropertyValue( u"FontSlant"_ustr , cpo::uno::Any( bItalic ? awt::FontSlant_ITALIC : awt::FontSlant_NONE ) );
}

bool VbaNewFont::getUnderline()
{
    cpo::uno::Any aAny = mxProps->getPropertyValue(u"FontUnderline"_ustr );
    return aAny.get< sal_Int16 >() != awt::FontUnderline::NONE;
}

void VbaNewFont::setUnderline( bool bUnderline )
{
    mxProps->setPropertyValue(u"FontUnderline"_ustr , cpo::uno::Any( bUnderline ? awt::FontUnderline::SINGLE : awt::FontUnderline::NONE ) );
}

bool VbaNewFont::getStrikethrough()
{
    cpo::uno::Any aAny = mxProps->getPropertyValue( u"FontStrikeout"_ustr );
    return aAny.get< sal_Int16 >() != awt::FontStrikeout::NONE;
}

void VbaNewFont::setStrikethrough( bool bStrikethrough )
{
    mxProps->setPropertyValue( u"FontStrikeout"_ustr ,cpo::uno::Any( bStrikethrough ? awt::FontStrikeout::SINGLE : awt::FontStrikeout::NONE ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
