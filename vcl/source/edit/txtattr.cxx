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

void TextAttribFontColor::SetFont( vcl::Font& rFont ) const
{
    rFont.SetColor( maColor );
}

std::unique_ptr<TextAttrib> TextAttribFontColor::Clone() const
{
    return std::unique_ptr<TextAttrib>(new TextAttribFontColor( *this ));
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

void TextAttribFontWeight::SetFont( vcl::Font& rFont ) const
{
    rFont.SetWeight( meWeight );
}

std::unique_ptr<TextAttrib> TextAttribFontWeight::Clone() const
{
    return std::unique_ptr<TextAttrib>(new TextAttribFontWeight( *this ));
}

bool TextAttribFontWeight::operator==( const TextAttrib& rAttr ) const
{
    return ( ( TextAttrib::operator==(rAttr ) ) &&
                ( meWeight == static_cast<const TextAttribFontWeight&>(rAttr).meWeight ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
