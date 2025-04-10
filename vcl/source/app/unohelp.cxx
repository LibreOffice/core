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

#include <vcl/svapp.hxx>
#include <vcl/unohelp.hxx>

#include <osl/diagnose.h>

#include <comphelper/processfactory.hxx>

#include <com/sun/star/i18n/BreakIterator.hpp>
#include <com/sun/star/i18n/CharacterClassification.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontWidth.hpp>
#include <com/sun/star/awt/XExtendedToolkit.hpp>

using namespace ::com::sun::star;

uno::Reference < i18n::XBreakIterator > vcl::unohelper::CreateBreakIterator()
{
    const uno::Reference< uno::XComponentContext >& xContext = comphelper::getProcessComponentContext();
    return i18n::BreakIterator::create(xContext);
}

uno::Reference < i18n::XCharacterClassification > vcl::unohelper::CreateCharacterClassification()
{
    return i18n::CharacterClassification::create( comphelper::getProcessComponentContext() );
}

float vcl::unohelper::ConvertFontWidth( FontWidth eWidth )
{
    if( eWidth == WIDTH_DONTKNOW )
        return css::awt::FontWidth::DONTKNOW;
    else if( eWidth == WIDTH_ULTRA_CONDENSED )
        return css::awt::FontWidth::ULTRACONDENSED;
    else if( eWidth == WIDTH_EXTRA_CONDENSED )
        return css::awt::FontWidth::EXTRACONDENSED;
    else if( eWidth == WIDTH_CONDENSED )
        return css::awt::FontWidth::CONDENSED;
    else if( eWidth == WIDTH_SEMI_CONDENSED )
        return css::awt::FontWidth::SEMICONDENSED;
    else if( eWidth == WIDTH_NORMAL )
        return css::awt::FontWidth::NORMAL;
    else if( eWidth == WIDTH_SEMI_EXPANDED )
        return css::awt::FontWidth::SEMIEXPANDED;
    else if( eWidth == WIDTH_EXPANDED )
        return css::awt::FontWidth::EXPANDED;
    else if( eWidth == WIDTH_EXTRA_EXPANDED )
        return css::awt::FontWidth::EXTRAEXPANDED;
    else if( eWidth == WIDTH_ULTRA_EXPANDED )
        return css::awt::FontWidth::ULTRAEXPANDED;

    OSL_FAIL( "Unknown FontWidth" );
    return css::awt::FontWidth::DONTKNOW;
}

FontWidth vcl::unohelper::ConvertFontWidth( float f )
{
    if( f <= css::awt::FontWidth::DONTKNOW )
        return WIDTH_DONTKNOW;
    else if( f <= css::awt::FontWidth::ULTRACONDENSED )
        return WIDTH_ULTRA_CONDENSED;
    else if( f <= css::awt::FontWidth::EXTRACONDENSED )
        return WIDTH_EXTRA_CONDENSED;
    else if( f <= css::awt::FontWidth::CONDENSED )
        return WIDTH_CONDENSED;
    else if( f <= css::awt::FontWidth::SEMICONDENSED )
        return WIDTH_SEMI_CONDENSED;
    else if( f <= css::awt::FontWidth::NORMAL )
        return WIDTH_NORMAL;
    else if( f <= css::awt::FontWidth::SEMIEXPANDED )
        return WIDTH_SEMI_EXPANDED;
    else if( f <= css::awt::FontWidth::EXPANDED )
        return WIDTH_EXPANDED;
    else if( f <= css::awt::FontWidth::EXTRAEXPANDED )
        return WIDTH_EXTRA_EXPANDED;
    else if( f <= css::awt::FontWidth::ULTRAEXPANDED )
        return WIDTH_ULTRA_EXPANDED;

    OSL_FAIL( "Unknown FontWidth" );
    return WIDTH_DONTKNOW;
}

float vcl::unohelper::ConvertFontWeight( FontWeight eWeight )
{
    if( eWeight == WEIGHT_DONTKNOW )
        return css::awt::FontWeight::DONTKNOW;
    else if( eWeight == WEIGHT_THIN )
        return css::awt::FontWeight::THIN;
    else if( eWeight == WEIGHT_ULTRALIGHT )
        return css::awt::FontWeight::ULTRALIGHT;
    else if( eWeight == WEIGHT_LIGHT )
        return css::awt::FontWeight::LIGHT;
    else if( eWeight == WEIGHT_SEMILIGHT )
        return css::awt::FontWeight::SEMILIGHT;
    else if( ( eWeight == WEIGHT_NORMAL ) || ( eWeight == WEIGHT_MEDIUM ) )
        return css::awt::FontWeight::NORMAL;
    else if( eWeight == WEIGHT_SEMIBOLD )
        return css::awt::FontWeight::SEMIBOLD;
    else if( eWeight == WEIGHT_BOLD )
        return css::awt::FontWeight::BOLD;
    else if( eWeight == WEIGHT_ULTRABOLD )
        return css::awt::FontWeight::ULTRABOLD;
    else if( eWeight == WEIGHT_BLACK )
        return css::awt::FontWeight::BLACK;

    OSL_FAIL( "Unknown FontWeight" );
    return css::awt::FontWeight::DONTKNOW;
}

FontWeight vcl::unohelper::ConvertFontWeight( float f )
{
    if( f <= css::awt::FontWeight::DONTKNOW )
        return WEIGHT_DONTKNOW;
    else if( f <= css::awt::FontWeight::THIN )
        return WEIGHT_THIN;
    else if( f <= css::awt::FontWeight::ULTRALIGHT )
        return WEIGHT_ULTRALIGHT;
    else if( f <= css::awt::FontWeight::LIGHT )
        return WEIGHT_LIGHT;
    else if( f <= css::awt::FontWeight::SEMILIGHT )
        return WEIGHT_SEMILIGHT;
    else if( f <= css::awt::FontWeight::NORMAL )
        return WEIGHT_NORMAL;
    else if( f <= css::awt::FontWeight::SEMIBOLD )
        return WEIGHT_SEMIBOLD;
    else if( f <= css::awt::FontWeight::BOLD )
        return WEIGHT_BOLD;
    else if( f <= css::awt::FontWeight::ULTRABOLD )
        return WEIGHT_ULTRABOLD;
    else if( f <= css::awt::FontWeight::BLACK )
        return WEIGHT_BLACK;

    OSL_FAIL( "Unknown FontWeight" );
    return WEIGHT_DONTKNOW;
}

css::awt::FontSlant vcl::unohelper::ConvertFontSlant(FontItalic eItalic)
{
    css::awt::FontSlant eRet(css::awt::FontSlant_DONTKNOW);
    switch (eItalic)
    {
        case ITALIC_NONE:
            eRet = css::awt::FontSlant_NONE;
            break;
        case ITALIC_OBLIQUE:
            eRet = css::awt::FontSlant_OBLIQUE;
            break;
        case ITALIC_NORMAL:
            eRet = css::awt::FontSlant_ITALIC;
            break;
        case ITALIC_DONTKNOW:
            eRet = css::awt::FontSlant_DONTKNOW;
            break;
        case FontItalic_FORCE_EQUAL_SIZE:
            eRet = css::awt::FontSlant::FontSlant_MAKE_FIXED_SIZE;
            break;
    }
    return eRet;
}

FontItalic vcl::unohelper::ConvertFontSlant(css::awt::FontSlant eSlant)
{
    FontItalic eRet = ITALIC_DONTKNOW;
    switch (eSlant)
    {
        case css::awt::FontSlant_NONE:
            eRet = ITALIC_NONE;
            break;
        case css::awt::FontSlant_OBLIQUE:
            eRet = ITALIC_OBLIQUE;
            break;
        case css::awt::FontSlant_ITALIC:
            eRet = ITALIC_NORMAL;
            break;
        case css::awt::FontSlant_DONTKNOW:
            eRet = ITALIC_DONTKNOW;
            break;
        case css::awt::FontSlant_REVERSE_OBLIQUE:
            //there is no vcl reverse oblique
            eRet = ITALIC_OBLIQUE;
            break;
        case css::awt::FontSlant_REVERSE_ITALIC:
            //there is no vcl reverse normal
            eRet = ITALIC_NORMAL;
            break;
        case css::awt::FontSlant::FontSlant_MAKE_FIXED_SIZE:
            eRet = FontItalic_FORCE_EQUAL_SIZE;
            break;
    }
    return eRet;
}

Size vcl::unohelper::ConvertToVCLSize(const css::awt::Size& rAWTSize)
{
    return Size(rAWTSize.Width, rAWTSize.Height);
}

css::awt::Size vcl::unohelper::ConvertToAWTSize(const Size& rVCLSize)
{
    return css::awt::Size(rVCLSize.Width(), rVCLSize.Height());
}

Point vcl::unohelper::ConvertToVCLPoint(const css::awt::Point& rAWTPoint)
{
    return Point(rAWTPoint.X, rAWTPoint.Y);
}

css::awt::Point vcl::unohelper::ConvertToAWTPoint(const PointTemplateBase& rVCLPoint)
{
    return css::awt::Point(rVCLPoint.X(), rVCLPoint.Y());
}

tools::Rectangle vcl::unohelper::ConvertToVCLRect(const css::awt::Rectangle& rAWTRect)
{
    return ::tools::Rectangle(Point(rAWTRect.X, rAWTRect.Y),
                              Size(rAWTRect.Width, rAWTRect.Height));
}

css::awt::Rectangle vcl::unohelper::ConvertToAWTRect(const RectangleTemplateBase& rVCLRect)
{
    return css::awt::Rectangle(rVCLRect.Left(), rVCLRect.Top(), rVCLRect.GetWidth(),
                               rVCLRect.GetHeight());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
