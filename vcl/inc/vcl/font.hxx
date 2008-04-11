/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: font.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _SV_FONT_HXX
#define _SV_FONT_HXX

#include <vcl/dllapi.h>
#include <tools/gen.hxx>
#include <tools/string.hxx>
#include <i18npool/lang.h>
#include <tools/color.hxx>
#include <vcl/vclenum.hxx>
#include <vcl/fntstyle.hxx>

class SvStream;
#define FontAlign TextAlign

class Impl_Font;
class ImplFontAttributes;

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
                        Font( const String& rFamilyName, const Size& );
                        Font( const String& rFamilyName, const String& rStyleName, const Size& );
                        Font( FontFamily eFamily, const Size& );
                        ~Font();

    void                SetColor( const Color& );
    const Color&        GetColor() const;
    void                SetFillColor( const Color& );
    const Color&        GetFillColor() const;
    void                SetTransparent( BOOL bTransparent );
    BOOL                IsTransparent() const;
    void                SetAlign( FontAlign );
    FontAlign           GetAlign() const;

    void                SetName( const String& rFamilyName );
    const String&       GetName() const;
    void                SetStyleName( const String& rStyleName );
    const String&       GetStyleName() const;
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
    void                SetLanguage( LanguageType );
    LanguageType        GetLanguage() const;
    void                SetCJKContextLanguage( LanguageType );
    LanguageType        GetCJKContextLanguage() const;
    void                SetPitch( FontPitch ePitch );
    FontPitch           GetPitch() const;

    void                SetOrientation( short nLineOrientation );
    short               GetOrientation() const;
    void                SetVertical( BOOL bVertical );
    BOOL                IsVertical() const;
    void                SetKerning( FontKerning nKerning );
    FontKerning         GetKerning() const;
    BOOL                IsKerning() const;

    void                SetWeight( FontWeight );
    FontWeight          GetWeight() const;
    void                SetWidthType( FontWidth );
    FontWidth           GetWidthType() const;
    void                SetItalic( FontItalic );
    FontItalic          GetItalic() const;
    void                SetOutline( BOOL bOutline );
    BOOL                IsOutline() const;
    void                SetShadow( BOOL bShadow );
    BOOL                IsShadow() const;
    void                SetRelief( FontRelief );
    FontRelief          GetRelief() const;
    void                SetUnderline( FontUnderline );
    FontUnderline       GetUnderline() const;
    void                SetStrikeout( FontStrikeout );
    FontStrikeout       GetStrikeout() const;
    void                SetEmphasisMark( FontEmphasisMark );
    FontEmphasisMark    GetEmphasisMark() const;
    void                SetWordLineMode( BOOL bWordLine );
    BOOL                IsWordLineMode() const;

    void                Merge( const Font& rFont );
    void                GetFontAttributes( ImplFontAttributes& rAttrs ) const;

    Font&               operator=( const Font& );
    BOOL                operator==( const Font& ) const;
    BOOL                operator!=( const Font& rFont ) const
                            { return !(Font::operator==( rFont )); }
    BOOL                IsSameInstance( const Font& ) const;

    friend VCL_DLLPUBLIC SvStream&  operator>>( SvStream& rIStm, Font& );
    friend VCL_DLLPUBLIC SvStream&  operator<<( SvStream& rOStm, const Font& );

    static Font identifyFont( const void* pBuffer, sal_uInt32 nLen );
};

#endif  // _VCL_FONT_HXX
