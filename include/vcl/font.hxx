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

#ifndef _SV_FONT_HXX
#define _SV_FONT_HXX

#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <vcl/dllapi.h>
#include <i18nlangtag/languagetag.hxx>
#include <tools/color.hxx>
#include <vcl/vclenum.hxx>
#include <vcl/fntstyle.hxx>

class SvStream;
#define FontAlign TextAlign

class Impl_Font;
class ImplFontAttributes;
class Size;

// --------
// - Font -
// --------

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
    void                SetTransparent( sal_Bool bTransparent );
    sal_Bool            IsTransparent() const;
    void                SetAlign( FontAlign );
    FontAlign           GetAlign() const;

    void                SetName( const OUString& rFamilyName );
    const OUString&     GetName() const;
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
    void                SetVertical( sal_Bool bVertical );
    sal_Bool            IsVertical() const;
    void                SetKerning( FontKerning nKerning );
    FontKerning         GetKerning() const;
    sal_Bool            IsKerning() const;

    void                SetWeight( FontWeight );
    FontWeight          GetWeight() const;
    void                SetWidthType( FontWidth );
    FontWidth           GetWidthType() const;
    void                SetItalic( FontItalic );
    FontItalic          GetItalic() const;
    void                SetOutline( sal_Bool bOutline );
    sal_Bool                IsOutline() const;
    void                SetShadow( sal_Bool bShadow );
    sal_Bool                IsShadow() const;
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
    void                SetWordLineMode( sal_Bool bWordLine );
    sal_Bool                IsWordLineMode() const;

    void                Merge( const Font& rFont );
    void                GetFontAttributes( ImplFontAttributes& rAttrs ) const;

    Font&               operator=( const Font& );
    sal_Bool                operator==( const Font& ) const;
    sal_Bool                operator!=( const Font& rFont ) const
                            { return !(Font::operator==( rFont )); }
    sal_Bool                IsSameInstance( const Font& ) const;

    friend VCL_DLLPUBLIC SvStream&  operator>>( SvStream& rIStm, Font& );
    friend VCL_DLLPUBLIC SvStream&  operator<<( SvStream& rOStm, const Font& );

    static Font identifyFont( const void* pBuffer, sal_uInt32 nLen );
};

#endif  // _VCL_FONT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
