/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: font.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 17:54:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SV_FONT_HXX
#define _SV_FONT_HXX

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _TOOLS_GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _TOOLS_STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef INCLUDED_I18NPOOL_LANG_H
#include <i18npool/lang.h>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif
#ifndef _VCL_VCLENUM_HXX
#include <vcl/vclenum.hxx>
#endif
#ifndef _VCL_FNTSTYLE_HXX
#include <vcl/fntstyle.hxx>
#endif

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
