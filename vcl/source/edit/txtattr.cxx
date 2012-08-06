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


#include <vcl/txtattr.hxx>
#include <vcl/font.hxx>




TextAttrib::~TextAttrib()
{
}

int TextAttrib::operator==( const TextAttrib& rAttr ) const
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

void TextAttribFontColor::SetFont( Font& rFont ) const
{
    rFont.SetColor( maColor );
}

TextAttrib* TextAttribFontColor::Clone() const
{
    return new TextAttribFontColor( *this );
}

int TextAttribFontColor::operator==( const TextAttrib& rAttr ) const
{
    return ( ( TextAttrib::operator==(rAttr ) ) &&
                ( maColor == ((const TextAttribFontColor&)rAttr).maColor ) );
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

void TextAttribFontWeight::SetFont( Font& rFont ) const
{
    rFont.SetWeight( meWeight );
}

TextAttrib* TextAttribFontWeight::Clone() const
{
    return new TextAttribFontWeight( *this );
}

int TextAttribFontWeight::operator==( const TextAttrib& rAttr ) const
{
    return ( ( TextAttrib::operator==(rAttr ) ) &&
                ( meWeight == ((const TextAttribFontWeight&)rAttr).meWeight ) );
}


TextAttribHyperLink::TextAttribHyperLink( const TextAttribHyperLink& rAttr )
    : TextAttrib( rAttr ), maURL( rAttr.maURL ), maDescription( rAttr.maDescription )
{
    maColor = rAttr.maColor;
}

TextAttribHyperLink::~TextAttribHyperLink()
{
}

void TextAttribHyperLink::SetFont( Font& rFont ) const
{
    rFont.SetColor( maColor );
    rFont.SetUnderline( UNDERLINE_SINGLE );
}

TextAttrib* TextAttribHyperLink::Clone() const
{
    return new TextAttribHyperLink( *this );
}

int TextAttribHyperLink::operator==( const TextAttrib& rAttr ) const
{
    return ( ( TextAttrib::operator==(rAttr ) ) &&
                ( maURL == ((const TextAttribHyperLink&)rAttr).maURL ) &&
                ( maDescription == ((const TextAttribHyperLink&)rAttr).maDescription ) &&
                ( maColor == ((const TextAttribHyperLink&)rAttr).maColor ) );
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

void TextAttribProtect::SetFont( Font& ) const
{
}

TextAttrib*     TextAttribProtect::Clone() const
{
    return new TextAttribProtect();
}

int TextAttribProtect::operator==( const TextAttrib& rAttr ) const
{
    return ( TextAttrib::operator==(rAttr ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
