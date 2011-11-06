/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include <svtools/txtattr.hxx>
#include <vcl/font.hxx>




TextAttrib::~TextAttrib()
{
}

void TextAttrib::SetFont( Font& ) const
{
}

TextAttrib* TextAttrib::Clone() const
{
    return NULL;
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


TextAttribHyperLink::TextAttribHyperLink( const XubString& rURL )
    : TextAttrib( TEXTATTR_HYPERLINK ), maURL( rURL )
{
    maColor = COL_BLUE;
}

TextAttribHyperLink::TextAttribHyperLink( const XubString& rURL, const XubString& rDescription )
    : TextAttrib( TEXTATTR_HYPERLINK ), maURL( rURL ), maDescription( rDescription )
{
    maColor = COL_BLUE;
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

/*-- 24.06.2004 14:49:44---------------------------------------------------

  -----------------------------------------------------------------------*/
TextAttribProtect::TextAttribProtect() :
    TextAttrib( TEXTATTR_PROTECTED )
{
}
/*-- 24.06.2004 14:49:44---------------------------------------------------

  -----------------------------------------------------------------------*/
TextAttribProtect::TextAttribProtect( const TextAttribProtect&) :
    TextAttrib( TEXTATTR_PROTECTED )
{
}
/*-- 24.06.2004 14:49:44---------------------------------------------------

  -----------------------------------------------------------------------*/
TextAttribProtect::~TextAttribProtect()
{
}
/*-- 24.06.2004 14:49:44---------------------------------------------------

  -----------------------------------------------------------------------*/
void TextAttribProtect::SetFont( Font& ) const
{
}
/*-- 24.06.2004 14:49:44---------------------------------------------------

  -----------------------------------------------------------------------*/
TextAttrib*     TextAttribProtect::Clone() const
{
    return new TextAttribProtect();
}
/*-- 24.06.2004 14:49:45---------------------------------------------------

  -----------------------------------------------------------------------*/
int TextAttribProtect::operator==( const TextAttrib& rAttr ) const
{
    return ( TextAttrib::operator==(rAttr ) );
}
