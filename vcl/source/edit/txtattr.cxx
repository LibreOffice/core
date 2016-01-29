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

#include <vcl/txtattr.hxx>
#include <vcl/font.hxx>

TextAttrib::~TextAttrib()
{
}

bool TextAttrib::operator==( const TextAttrib& rAttr ) const
{
    return mnWhich == rAttr.mnWhich;
}

TextAttribFontColor::TextAttribFontColor( const Color& rColor )
    : TextAttrib( TEXTATTR_FONTCOLOR ), maColor( rColor )
{
}

TextAttribFontColor::TextAttribFontColor( const TextAttribFontColor& rAttr )
    : TextAttrib( rAttr ), maColor( rAttr.maColor )
{
}

TextAttribFontColor::~TextAttribFontColor()
{
}

void TextAttribFontColor::SetFont( vcl::Font& rFont ) const
{
    rFont.SetColor( maColor );
}

TextAttrib* TextAttribFontColor::Clone() const
{
    return new TextAttribFontColor( *this );
}

bool TextAttribFontColor::operator==( const TextAttrib& rAttr ) const
{
    return ( ( TextAttrib::operator==(rAttr ) ) &&
                ( maColor == static_cast<const TextAttribFontColor&>(rAttr).maColor ) );
}

TextAttribFontWeight::TextAttribFontWeight( FontWeight eWeight )
    : TextAttrib( TEXTATTR_FONTWEIGHT ), meWeight( eWeight )
{
}

TextAttribFontWeight::TextAttribFontWeight( const TextAttribFontWeight& rAttr )
    : TextAttrib( rAttr ), meWeight( rAttr.meWeight )
{
}

TextAttribFontWeight::~TextAttribFontWeight()
{
}

void TextAttribFontWeight::SetFont( vcl::Font& rFont ) const
{
    rFont.SetWeight( meWeight );
}

TextAttrib* TextAttribFontWeight::Clone() const
{
    return new TextAttribFontWeight( *this );
}

bool TextAttribFontWeight::operator==( const TextAttrib& rAttr ) const
{
    return ( ( TextAttrib::operator==(rAttr ) ) &&
                ( meWeight == static_cast<const TextAttribFontWeight&>(rAttr).meWeight ) );
}

TextAttribHyperLink::TextAttribHyperLink( const TextAttribHyperLink& rAttr )
    : TextAttrib( rAttr ), maURL( rAttr.maURL ), maDescription( rAttr.maDescription )
{
    maColor = rAttr.maColor;
}

TextAttribHyperLink::~TextAttribHyperLink()
{
}

void TextAttribHyperLink::SetFont( vcl::Font& rFont ) const
{
    rFont.SetColor( maColor );
    rFont.SetUnderline( LINESTYLE_SINGLE );
}

TextAttrib* TextAttribHyperLink::Clone() const
{
    return new TextAttribHyperLink( *this );
}

bool TextAttribHyperLink::operator==( const TextAttrib& rAttr ) const
{
    return ( ( TextAttrib::operator==(rAttr ) ) &&
                ( maURL == static_cast<const TextAttribHyperLink&>(rAttr).maURL ) &&
                ( maDescription == static_cast<const TextAttribHyperLink&>(rAttr).maDescription ) &&
                ( maColor == static_cast<const TextAttribHyperLink&>(rAttr).maColor ) );
}

TextAttribProtect::TextAttribProtect() :
    TextAttrib( TEXTATTR_PROTECTED )
{
}

TextAttribProtect::TextAttribProtect( const TextAttribProtect&) :
    TextAttrib( TEXTATTR_PROTECTED )
{
}

TextAttribProtect::~TextAttribProtect()
{
}

void TextAttribProtect::SetFont( vcl::Font& ) const
{
}

TextAttrib*     TextAttribProtect::Clone() const
{
    return new TextAttribProtect();
}

bool TextAttribProtect::operator==( const TextAttrib& rAttr ) const
{
    return ( TextAttrib::operator==(rAttr ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
