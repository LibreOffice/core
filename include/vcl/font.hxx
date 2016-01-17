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

#ifndef INCLUDED_VCL_FONT_HXX
#define INCLUDED_VCL_FONT_HXX

#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <vcl/dllapi.h>
#include <i18nlangtag/languagetag.hxx>
#include <tools/color.hxx>
#include <tools/gen.hxx>
#include <vcl/vclenum.hxx>
#include <vcl/fntstyle.hxx>

class SvStream;
#define FontAlign TextAlign

class Impl_Font;
class ImplFontAttributes;
namespace vcl { class Font; }
// need to first declare these outside the vcl namespace, or the friend declarations won't work right
VCL_DLLPUBLIC SvStream&  ReadFont( SvStream& rIStm, vcl::Font& );
VCL_DLLPUBLIC SvStream&  WriteFont( SvStream& rOStm, const vcl::Font& );

namespace vcl {

class VCL_DLLPUBLIC Font
{
private:
    Impl_Font*          mpImplFont;
    void                MakeUnique();

public:
                        Font();
                        Font( const Font& );
                        Font( const OUString& rFamilyName, const Size& );
                        Font( const OUString& rFamilyName, const OUString& rStyleName, const Size& );
                        Font( FontFamily eFamily, const Size& );
                        ~Font();

    // setting the color on the font is obsolete, the only remaining
    // valid use is for keeping backward compatibility with old MetaFiles
    void                SetColor( const Color& );
    const Color&        GetColor() const;
    void                SetFillColor( const Color& );
    const Color&        GetFillColor() const;
    void                SetTransparent( bool bTransparent );
    bool            IsTransparent() const;
    void                SetAlign( FontAlign );
    FontAlign           GetAlign() const;

    void                SetName( const OUString& rFamilyName );
    const OUString&     GetFamilyName() const;
    void                SetStyleName( const OUString& rStyleName );
    const OUString&     GetStyleName() const;
    void                SetSize( const Size& );
    const Size&         GetSize() const;
    void                SetHeight( long nHeight );
    long                GetHeight() const;
    void                SetWidth( long nWidth );
    long                GetWidth() const;

    void                SetFamily( FontFamily );
    FontFamily          GetFamily() const;
    void                SetCharSet( rtl_TextEncoding );
    rtl_TextEncoding    GetCharSet() const;
    // Prefer LanguageTag over LanguageType
    void                SetLanguageTag( const LanguageTag & );
    const LanguageTag&  GetLanguageTag() const;
    void                SetCJKContextLanguageTag( const LanguageTag& );
    const LanguageTag&  GetCJKContextLanguageTag() const;
    void                SetLanguage( LanguageType );
    LanguageType        GetLanguage() const;
    void                SetCJKContextLanguage( LanguageType );
    LanguageType        GetCJKContextLanguage() const;
    void                SetPitch( FontPitch ePitch );
    FontPitch           GetPitch() const;

    void                SetOrientation( short nLineOrientation );
    short               GetOrientation() const;
    void                SetVertical( bool bVertical );
    bool                IsVertical() const;
    void                SetKerning( FontKerning nKerning );
    FontKerning         GetKerning() const;
    bool            IsKerning() const;

    void                SetWeight( FontWeight );
    FontWeight          GetWeight() const;
    void                SetWidthType( FontWidth );
    FontWidth           GetWidthType() const;
    void                SetItalic( FontItalic );
    FontItalic          GetItalic() const;
    void                SetOutline( bool bOutline );
    bool                IsOutline() const;
    void                SetShadow( bool bShadow );
    bool                IsShadow() const;
    void                SetRelief( FontRelief );
    FontRelief          GetRelief() const;
    void                SetUnderline( FontUnderline );
    FontUnderline       GetUnderline() const;
    void                SetOverline( FontUnderline );
    FontUnderline       GetOverline() const;
    void                SetStrikeout( FontStrikeout );
    FontStrikeout       GetStrikeout() const;
    void                SetEmphasisMark( FontEmphasisMark );
    FontEmphasisMark    GetEmphasisMark() const;
    void                SetWordLineMode( bool bWordLine );
    bool                IsWordLineMode() const;

    void                Merge( const Font& rFont );
    void                GetFontAttributes( ImplFontAttributes& rAttrs ) const;

    Font&               operator=( const Font& );
    bool                operator==( const Font& ) const;
    bool                operator!=( const Font& rFont ) const
                            { return !(Font::operator==( rFont )); }
    bool                IsSameInstance( const Font& ) const;

    friend VCL_DLLPUBLIC SvStream&  ::ReadFont( SvStream& rIStm, vcl::Font& );
    friend VCL_DLLPUBLIC SvStream&  ::WriteFont( SvStream& rOStm, const vcl::Font& );

    static Font identifyFont( const void* pBuffer, sal_uInt32 nLen );
};

}

#endif  // _VCL_FONT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
