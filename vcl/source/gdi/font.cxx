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


#include "unotools/fontcfg.hxx"

#include "tools/stream.hxx"
#include "tools/vcompat.hxx"
#include "tools/debug.hxx"
#include <tools/gen.hxx>

#include "vcl/font.hxx"

#include "impfont.hxx"
#include "outfont.hxx"
#include "sft.hxx"

#include <sal/macros.h>

#include <algorithm>

using namespace vcl;

DBG_NAME( Font )

Impl_Font::Impl_Font() :
    maColor( COL_TRANSPARENT ),
    maFillColor( COL_TRANSPARENT )
{
    mnRefCount          = 1;
    meCharSet           = RTL_TEXTENCODING_DONTKNOW;
    meLanguage          = LANGUAGE_DONTKNOW;
    meCJKLanguage       = LANGUAGE_DONTKNOW;
    meFamily            = FAMILY_DONTKNOW;
    mePitch             = PITCH_DONTKNOW;
    meAlign             = ALIGN_TOP;
    meWeight            = WEIGHT_DONTKNOW;
    meWidthType         = WIDTH_DONTKNOW;
    meItalic            = ITALIC_NONE;
    meUnderline         = UNDERLINE_NONE;
    meOverline          = UNDERLINE_NONE;
    meStrikeout         = STRIKEOUT_NONE;
    meRelief            = RELIEF_NONE;
    meEmphasisMark      = EMPHASISMARK_NONE;
    mnOrientation       = 0;
    mnKerning           = 0;
    mbWordLine          = false;
    mbOutline           = false;
    mbShadow            = false;
    mbVertical          = false;
    mbTransparent       = true;
    mbConfigLookup      = false;
}

Impl_Font::Impl_Font( const Impl_Font& rImplFont )
:   maFamilyName( rImplFont.maFamilyName ),
    maStyleName( rImplFont.maStyleName ),
    maSize( rImplFont.maSize ),
    maColor( rImplFont.maColor ),
    maFillColor( rImplFont.maFillColor )
{
    mnRefCount          = 1;
    meCharSet           = rImplFont.meCharSet;
    meLanguage          = rImplFont.meLanguage;
    meCJKLanguage       = rImplFont.meCJKLanguage;
    meFamily            = rImplFont.meFamily;
    mePitch             = rImplFont.mePitch;
    meAlign             = rImplFont.meAlign;
    meWeight            = rImplFont.meWeight;
    meWidthType         = rImplFont.meWidthType;
    meItalic            = rImplFont.meItalic;
    meUnderline         = rImplFont.meUnderline;
    meOverline          = rImplFont.meOverline;
    meStrikeout         = rImplFont.meStrikeout;
    meRelief            = rImplFont.meRelief;
    meEmphasisMark      = rImplFont.meEmphasisMark;
    mnOrientation       = rImplFont.mnOrientation;
    mnKerning           = rImplFont.mnKerning;
    mbWordLine          = rImplFont.mbWordLine;
    mbOutline           = rImplFont.mbOutline;
    mbShadow            = rImplFont.mbShadow;
    mbVertical          = rImplFont.mbVertical;
    mbTransparent       = rImplFont.mbTransparent;
    mbConfigLookup      = rImplFont.mbConfigLookup;
}

bool Impl_Font::operator==( const Impl_Font& rOther ) const
{
    // equality tests split up for easier debugging
    if( (meWeight   != rOther.meWeight)
    ||  (meItalic   != rOther.meItalic)
    ||  (meFamily   != rOther.meFamily)
    ||  (mePitch    != rOther.mePitch) )
        return false;

    if( (meCharSet     != rOther.meCharSet)
    ||  (meLanguage    != rOther.meLanguage)
    ||  (meCJKLanguage != rOther.meCJKLanguage)
    ||  (meAlign       != rOther.meAlign) )
        return false;

    if( (maSize         != rOther.maSize)
    ||  (mnOrientation  != rOther.mnOrientation)
    ||  (mbVertical     != rOther.mbVertical) )
        return false;

    if( (maFamilyName   != rOther.maFamilyName)
    ||  (maStyleName    != rOther.maStyleName) )
        return false;

    if( (maColor        != rOther.maColor)
    ||  (maFillColor    != rOther.maFillColor) )
        return false;

    if( (meUnderline    != rOther.meUnderline)
    ||  (meOverline     != rOther.meOverline)
    ||  (meStrikeout    != rOther.meStrikeout)
    ||  (meRelief       != rOther.meRelief)
    ||  (meEmphasisMark != rOther.meEmphasisMark)
    ||  (mbWordLine     != rOther.mbWordLine)
    ||  (mbOutline      != rOther.mbOutline)
    ||  (mbShadow       != rOther.mbShadow)
    ||  (mnKerning      != rOther.mnKerning)
    ||  (mbTransparent  != rOther.mbTransparent) )
        return false;

    return true;
}

void Impl_Font::AskConfig()
{
    if( mbConfigLookup )
        return;

    mbConfigLookup = true;

    // prepare the FontSubst configuration lookup
    const utl::FontSubstConfiguration& rFontSubst = utl::FontSubstConfiguration::get();

    OUString      aShortName;
    OUString      aFamilyName;
    sal_uLong       nType = 0;
    FontWeight  eWeight = WEIGHT_DONTKNOW;
    FontWidth   eWidthType = WIDTH_DONTKNOW;
    OUString    aMapName = maFamilyName;
    GetEnglishSearchFontName( aMapName );
    utl::FontSubstConfiguration::getMapName( aMapName,
        aShortName, aFamilyName, eWeight, eWidthType, nType );

    // lookup the font name in the configuration
    const utl::FontNameAttr* pFontAttr = rFontSubst.getSubstInfo( aMapName );

    // if the direct lookup failed try again with an alias name
    if ( !pFontAttr && (aShortName != aMapName) )
        pFontAttr = rFontSubst.getSubstInfo( aShortName );

    if( pFontAttr )
    {
        // the font was found in the configuration
        if( meFamily == FAMILY_DONTKNOW )
        {
            if ( pFontAttr->Type & IMPL_FONT_ATTR_SERIF )
                meFamily = FAMILY_ROMAN;
            else if ( pFontAttr->Type & IMPL_FONT_ATTR_SANSSERIF )
                meFamily = FAMILY_SWISS;
            else if ( pFontAttr->Type & IMPL_FONT_ATTR_TYPEWRITER )
                meFamily = FAMILY_MODERN;
            else if ( pFontAttr->Type & IMPL_FONT_ATTR_ITALIC )
                meFamily = FAMILY_SCRIPT;
            else if ( pFontAttr->Type & IMPL_FONT_ATTR_DECORATIVE )
                meFamily = FAMILY_DECORATIVE;
        }

        if( mePitch == PITCH_DONTKNOW )
        {
            if ( pFontAttr->Type & IMPL_FONT_ATTR_FIXED )
                mePitch = PITCH_FIXED;
        }
    }

    // if some attributes are still unknown then use the FontSubst magic
    if( meFamily == FAMILY_DONTKNOW )
    {
        if( nType & IMPL_FONT_ATTR_SERIF )
            meFamily = FAMILY_ROMAN;
        else if( nType & IMPL_FONT_ATTR_SANSSERIF )
            meFamily = FAMILY_SWISS;
        else if( nType & IMPL_FONT_ATTR_TYPEWRITER )
            meFamily = FAMILY_MODERN;
        else if( nType & IMPL_FONT_ATTR_ITALIC )
            meFamily = FAMILY_SCRIPT;
        else if( nType & IMPL_FONT_ATTR_DECORATIVE )
            meFamily = FAMILY_DECORATIVE;
    }

    if( meWeight == WEIGHT_DONTKNOW )
        meWeight = eWeight;
    if( meWidthType == WIDTH_DONTKNOW )
        meWidthType = eWidthType;
}

void Font::MakeUnique()
{
    // create a copy if others still reference it
    if ( mpImplFont->mnRefCount != 1 )
    {
        if ( mpImplFont->mnRefCount )
            mpImplFont->mnRefCount--;
        mpImplFont = new Impl_Font( *mpImplFont );
    }
}

Font::Font()
{
    DBG_CTOR( Font, NULL );

    static Impl_Font aStaticImplFont;
    // RefCount is zero for static objects
    aStaticImplFont.mnRefCount = 0;
    mpImplFont = &aStaticImplFont;
}

Font::Font( const Font& rFont )
{
    DBG_CTOR( Font, NULL );
    DBG_CHKOBJ( &rFont, Font, NULL );
    bool bRefIncrementable = rFont.mpImplFont->mnRefCount < ::std::numeric_limits<FontRefCount>::max();
    DBG_ASSERT( bRefIncrementable, "Font: RefCount overflow" );

    mpImplFont = rFont.mpImplFont;
    // do not count static objects (where RefCount is zero)
    if ( mpImplFont->mnRefCount && bRefIncrementable )
        mpImplFont->mnRefCount++;
}

Font::Font( const OUString& rFamilyName, const Size& rSize )
{
    DBG_CTOR( Font, NULL );

    mpImplFont               = new Impl_Font;
    mpImplFont->maFamilyName = rFamilyName;
    mpImplFont->maSize       = rSize;
}

Font::Font( const OUString& rFamilyName, const OUString& rStyleName, const Size& rSize )
{
    DBG_CTOR( Font, NULL );

    mpImplFont              = new Impl_Font;
    mpImplFont->maFamilyName= rFamilyName;
    mpImplFont->maStyleName = rStyleName;
    mpImplFont->maSize      = rSize;
}

Font::Font( FontFamily eFamily, const Size& rSize )
{
    DBG_CTOR( Font, NULL );

    mpImplFont              = new Impl_Font;
    mpImplFont->meFamily    = eFamily;
    mpImplFont->maSize      = rSize;
}

Font::~Font()
{
    DBG_DTOR( Font, NULL );

    // decrement reference counter and delete if last reference
    // if the object is not static (Refcounter==0)
    if ( mpImplFont->mnRefCount )
    {
        if ( mpImplFont->mnRefCount == 1 )
            delete mpImplFont;
        else
            mpImplFont->mnRefCount--;
    }
}

void Font::SetColor( const Color& rColor )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->maColor != rColor )
    {
        MakeUnique();
        mpImplFont->maColor = rColor;
    }
}

void Font::SetFillColor( const Color& rColor )
{
    DBG_CHKTHIS( Font, NULL );

    MakeUnique();
    mpImplFont->maFillColor = rColor;
    if ( rColor.GetTransparency() )
        mpImplFont->mbTransparent = true;
}

void Font::SetTransparent( sal_Bool bTransparent )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->mbTransparent != bTransparent )
    {
        MakeUnique();
        mpImplFont->mbTransparent = bTransparent;
    }
}

void Font::SetAlign( FontAlign eAlign )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->meAlign != eAlign )
    {
        MakeUnique();
        mpImplFont->meAlign = eAlign;
    }
}

void Font::SetName( const OUString& rFamilyName )
{
    DBG_CHKTHIS( Font, NULL );

    MakeUnique();
    mpImplFont->maFamilyName = rFamilyName;
}

void Font::SetStyleName( const OUString& rStyleName )
{
    DBG_CHKTHIS( Font, NULL );

    MakeUnique();
    mpImplFont->maStyleName = rStyleName;
}

void Font::SetSize( const Size& rSize )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->maSize != rSize )
    {
        MakeUnique();
        mpImplFont->maSize = rSize;
    }
}

void Font::SetFamily( FontFamily eFamily )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->meFamily != eFamily )
    {
        MakeUnique();
        mpImplFont->meFamily = eFamily;
    }
}

void Font::SetCharSet( rtl_TextEncoding eCharSet )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->meCharSet != eCharSet )
    {
        MakeUnique();
        mpImplFont->meCharSet = eCharSet;
    }
}

void Font::SetLanguage( LanguageType eLanguage )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->meLanguage != eLanguage )
    {
        MakeUnique();
        mpImplFont->meLanguage = eLanguage;
    }
}

void Font::SetCJKContextLanguage( LanguageType eLanguage )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->meCJKLanguage != eLanguage )
    {
        MakeUnique();
        mpImplFont->meCJKLanguage = eLanguage;
    }
}

void Font::SetPitch( FontPitch ePitch )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->mePitch != ePitch )
    {
        MakeUnique();
        mpImplFont->mePitch = ePitch;
    }
}

void Font::SetOrientation( short nOrientation )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->mnOrientation != nOrientation )
    {
        MakeUnique();
        mpImplFont->mnOrientation = nOrientation;
    }
}

void Font::SetVertical( sal_Bool bVertical )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->mbVertical != bVertical )
    {
        MakeUnique();
        mpImplFont->mbVertical = bVertical;
    }
}

void Font::SetKerning( FontKerning nKerning )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->mnKerning != nKerning )
    {
        MakeUnique();
        mpImplFont->mnKerning = nKerning;
    }
}

sal_Bool Font::IsKerning() const
{
    return (mpImplFont->mnKerning & KERNING_FONTSPECIFIC) != 0;
}

void Font::SetWeight( FontWeight eWeight )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->meWeight != eWeight )
    {
        MakeUnique();
        mpImplFont->meWeight = eWeight;
    }
}

void Font::SetWidthType( FontWidth eWidth )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->meWidthType != eWidth )
    {
        MakeUnique();
        mpImplFont->meWidthType = eWidth;
    }
}

void Font::SetItalic( FontItalic eItalic )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->meItalic != eItalic )
    {
        MakeUnique();
        mpImplFont->meItalic = eItalic;
    }
}

void Font::SetOutline( sal_Bool bOutline )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->mbOutline != bOutline )
    {
        MakeUnique();
        mpImplFont->mbOutline = bOutline;
    }
}

void Font::SetShadow( sal_Bool bShadow )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->mbShadow != bShadow )
    {
        MakeUnique();
        mpImplFont->mbShadow = bShadow;
    }
}

void Font::SetUnderline( FontUnderline eUnderline )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->meUnderline != eUnderline )
    {
        MakeUnique();
        mpImplFont->meUnderline = eUnderline;
    }
}

void Font::SetOverline( FontUnderline eOverline )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->meOverline != eOverline )
    {
        MakeUnique();
        mpImplFont->meOverline = eOverline;
    }
}

void Font::SetStrikeout( FontStrikeout eStrikeout )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->meStrikeout != eStrikeout )
    {
        MakeUnique();
        mpImplFont->meStrikeout = eStrikeout;
    }
}

void Font::SetRelief( FontRelief eRelief )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->meRelief != eRelief )
    {
        MakeUnique();
        mpImplFont->meRelief = eRelief;
    }
}

void Font::SetEmphasisMark( FontEmphasisMark eEmphasisMark )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->meEmphasisMark != eEmphasisMark )
    {
        MakeUnique();
        mpImplFont->meEmphasisMark = eEmphasisMark;
    }
}

void Font::SetWordLineMode( sal_Bool bWordLine )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->mbWordLine != bWordLine )
    {
        MakeUnique();
        mpImplFont->mbWordLine = bWordLine;
    }
}

Font& Font::operator=( const Font& rFont )
{
    DBG_CHKTHIS( Font, NULL );
    DBG_CHKOBJ( &rFont, Font, NULL );
    bool bRefIncrementable = rFont.mpImplFont->mnRefCount < ::std::numeric_limits<FontRefCount>::max();
    DBG_ASSERT( bRefIncrementable, "Font: RefCount overflow" );

    // Increment RefCount first, so that we can reference ourselves
    // RefCount == 0 for static objects
    if ( rFont.mpImplFont->mnRefCount && bRefIncrementable )
        rFont.mpImplFont->mnRefCount++;

    // If it's not static ImplData and if it's the last reference, delete it
    // else decrement RefCount
    if ( mpImplFont->mnRefCount )
    {
        if ( mpImplFont->mnRefCount == 1 )
            delete mpImplFont;
        else
            mpImplFont->mnRefCount--;
    }

    mpImplFont = rFont.mpImplFont;

    return *this;
}

sal_Bool Font::operator==( const Font& rFont ) const
{
    DBG_CHKTHIS( Font, NULL );
    DBG_CHKOBJ( &rFont, Font, NULL );

    if( mpImplFont == rFont.mpImplFont )
        return sal_True;
    if( *mpImplFont == *rFont.mpImplFont )
        return sal_True;

    return sal_False;
}

void Font::Merge( const Font& rFont )
{
    if ( !rFont.GetName().isEmpty() )
    {
        SetName( rFont.GetName() );
        SetStyleName( rFont.GetStyleName() );
        SetCharSet( GetCharSet() );
        SetLanguage( rFont.GetLanguage() );
        SetCJKContextLanguage( rFont.GetCJKContextLanguage() );
        // don't use access methods here, might lead to AskConfig(), if DONTKNOW
        SetFamily( rFont.mpImplFont->meFamily );
        SetPitch( rFont.mpImplFont->mePitch );
    }

    // don't use access methods here, might lead to AskConfig(), if DONTKNOW
    if ( rFont.mpImplFont->meWeight != WEIGHT_DONTKNOW )
        SetWeight( rFont.GetWeight() );
    if ( rFont.mpImplFont->meItalic != ITALIC_DONTKNOW )
        SetItalic( rFont.GetItalic() );
    if ( rFont.mpImplFont->meWidthType != WIDTH_DONTKNOW )
        SetWidthType( rFont.GetWidthType() );


    if ( rFont.GetSize().Height() )
        SetSize( rFont.GetSize() );
    if ( rFont.GetUnderline() != UNDERLINE_DONTKNOW )
    {
        SetUnderline( rFont.GetUnderline() );
        SetWordLineMode( rFont.IsWordLineMode() );
    }
    if ( rFont.GetOverline() != UNDERLINE_DONTKNOW )
    {
        SetOverline( rFont.GetOverline() );
        SetWordLineMode( rFont.IsWordLineMode() );
    }
    if ( rFont.GetStrikeout() != STRIKEOUT_DONTKNOW )
    {
        SetStrikeout( rFont.GetStrikeout() );
        SetWordLineMode( rFont.IsWordLineMode() );
    }

    // Defaults?
    SetOrientation( rFont.GetOrientation() );
    SetVertical( rFont.IsVertical() );
    SetEmphasisMark( rFont.GetEmphasisMark() );
    SetKerning( rFont.IsKerning() );
    SetOutline( rFont.IsOutline() );
    SetShadow( rFont.IsShadow() );
    SetRelief( rFont.GetRelief() );
}

void Font::GetFontAttributes( ImplFontAttributes& rAttrs ) const
{
    rAttrs.SetFamilyName( mpImplFont->maFamilyName );
    rAttrs.SetStyleName( mpImplFont->maStyleName );
    rAttrs.SetFamilyType( mpImplFont->meFamily );
    rAttrs.SetPitch( mpImplFont->mePitch );
    rAttrs.SetItalic( mpImplFont->meItalic );
    rAttrs.SetWeight( mpImplFont->meWeight );
    rAttrs.SetWidthType( WIDTH_DONTKNOW );
    rAttrs.SetSymbolFlag( mpImplFont->meCharSet == RTL_TEXTENCODING_SYMBOL );
}

SvStream& operator>>( SvStream& rIStm, Impl_Font& rImpl_Font )
{
    VersionCompat   aCompat( rIStm, STREAM_READ );
    sal_uInt16          nTmp16;
    sal_Bool            bTmp;
    sal_uInt8           nTmp8;

    rImpl_Font.maFamilyName = rIStm.ReadUniOrByteString(rIStm.GetStreamCharSet());
    rImpl_Font.maStyleName = rIStm.ReadUniOrByteString(rIStm.GetStreamCharSet());
    rIStm >> rImpl_Font.maSize;

    rIStm >> nTmp16; rImpl_Font.meCharSet = (rtl_TextEncoding) nTmp16;
    rIStm >> nTmp16; rImpl_Font.meFamily = (FontFamily) nTmp16;
    rIStm >> nTmp16; rImpl_Font.mePitch = (FontPitch) nTmp16;
    rIStm >> nTmp16; rImpl_Font.meWeight = (FontWeight) nTmp16;
    rIStm >> nTmp16; rImpl_Font.meUnderline = (FontUnderline) nTmp16;
    rIStm >> nTmp16; rImpl_Font.meStrikeout = (FontStrikeout) nTmp16;
    rIStm >> nTmp16; rImpl_Font.meItalic = (FontItalic) nTmp16;
    rIStm >> nTmp16; rImpl_Font.meLanguage = (LanguageType) nTmp16;
    rIStm >> nTmp16; rImpl_Font.meWidthType = (FontWidth) nTmp16;

    rIStm >> rImpl_Font.mnOrientation;

    rIStm >> bTmp; rImpl_Font.mbWordLine = bTmp;
    rIStm >> bTmp; rImpl_Font.mbOutline = bTmp;
    rIStm >> bTmp; rImpl_Font.mbShadow = bTmp;
    rIStm >> nTmp8; rImpl_Font.mnKerning = nTmp8;

    if( aCompat.GetVersion() >= 2 )
    {
        rIStm >> nTmp8;     rImpl_Font.meRelief = (FontRelief)nTmp8;
        rIStm >> nTmp16;    rImpl_Font.meCJKLanguage = (LanguageType)nTmp16;
        rIStm >> bTmp;      rImpl_Font.mbVertical = bTmp;
        rIStm >> nTmp16;    rImpl_Font.meEmphasisMark = (FontEmphasisMark)nTmp16;
    }
    if( aCompat.GetVersion() >= 3 )
    {
        rIStm >> nTmp16; rImpl_Font.meOverline = (FontUnderline) nTmp16;
    }
    // Relief
    // CJKContextLanguage

    return rIStm;
}

SvStream& operator<<( SvStream& rOStm, const Impl_Font& rImpl_Font )
{
    VersionCompat aCompat( rOStm, STREAM_WRITE, 3 );
    rOStm.WriteUniOrByteString( rImpl_Font.maFamilyName, rOStm.GetStreamCharSet() );
    rOStm.WriteUniOrByteString( rImpl_Font.maStyleName, rOStm.GetStreamCharSet() );
    rOStm << rImpl_Font.maSize;

    rOStm << (sal_uInt16) GetStoreCharSet( rImpl_Font.meCharSet );
    rOStm << (sal_uInt16) rImpl_Font.meFamily;
    rOStm << (sal_uInt16) rImpl_Font.mePitch;
    rOStm << (sal_uInt16) rImpl_Font.meWeight;
    rOStm << (sal_uInt16) rImpl_Font.meUnderline;
    rOStm << (sal_uInt16) rImpl_Font.meStrikeout;
    rOStm << (sal_uInt16) rImpl_Font.meItalic;
    rOStm << (sal_uInt16) rImpl_Font.meLanguage;
    rOStm << (sal_uInt16) rImpl_Font.meWidthType;

    rOStm << rImpl_Font.mnOrientation;

    rOStm << (sal_Bool) rImpl_Font.mbWordLine;
    rOStm << (sal_Bool) rImpl_Font.mbOutline;
    rOStm << (sal_Bool) rImpl_Font.mbShadow;
    rOStm << (sal_uInt8) rImpl_Font.mnKerning;

    // new in version 2
    rOStm << (sal_uInt8)        rImpl_Font.meRelief;
    rOStm << (sal_uInt16)   rImpl_Font.meCJKLanguage;
    rOStm << (sal_Bool)     rImpl_Font.mbVertical;
    rOStm << (sal_uInt16)   rImpl_Font.meEmphasisMark;

    // new in version 3
    rOStm << (sal_uInt16) rImpl_Font.meOverline;

    return rOStm;
}

SvStream& operator>>( SvStream& rIStm, Font& rFont )
{
    rFont.MakeUnique();
    return( rIStm >> *rFont.mpImplFont );
}

SvStream& operator<<( SvStream& rOStm, const Font& rFont )
{
    return( rOStm << *rFont.mpImplFont );
}

namespace
{
    bool identifyTrueTypeFont( const void* i_pBuffer, sal_uInt32 i_nSize, Font& o_rResult )
    {
        bool bResult = false;
        TrueTypeFont* pTTF = NULL;
        if( OpenTTFontBuffer( const_cast<void*>(i_pBuffer), i_nSize, 0, &pTTF ) == SF_OK )
        {
            TTGlobalFontInfo aInfo;
            GetTTGlobalFontInfo( pTTF, &aInfo );
            // most importantly: the family name
            if( aInfo.ufamily )
                o_rResult.SetName( aInfo.ufamily );
            else if( aInfo.family )
                o_rResult.SetName( OStringToOUString( aInfo.family, RTL_TEXTENCODING_ASCII_US ) );
            // set weight
            if( aInfo.weight )
            {
                if( aInfo.weight < FW_EXTRALIGHT )
                    o_rResult.SetWeight( WEIGHT_THIN );
                else if( aInfo.weight < FW_LIGHT )
                    o_rResult.SetWeight( WEIGHT_ULTRALIGHT );
                else if( aInfo.weight < FW_NORMAL )
                    o_rResult.SetWeight( WEIGHT_LIGHT );
                else if( aInfo.weight < FW_MEDIUM )
                    o_rResult.SetWeight( WEIGHT_NORMAL );
                else if( aInfo.weight < FW_SEMIBOLD )
                    o_rResult.SetWeight( WEIGHT_MEDIUM );
                else if( aInfo.weight < FW_BOLD )
                    o_rResult.SetWeight( WEIGHT_SEMIBOLD );
                else if( aInfo.weight < FW_EXTRABOLD )
                    o_rResult.SetWeight( WEIGHT_BOLD );
                else if( aInfo.weight < FW_BLACK )
                    o_rResult.SetWeight( WEIGHT_ULTRABOLD );
                else
                    o_rResult.SetWeight( WEIGHT_BLACK );
            }
            else
                o_rResult.SetWeight( (aInfo.macStyle & 1) ? WEIGHT_BOLD : WEIGHT_NORMAL );
            // set width
            if( aInfo.width )
            {
                if( aInfo.width == FWIDTH_ULTRA_CONDENSED )
                    o_rResult.SetWidth( WIDTH_ULTRA_CONDENSED );
                else if( aInfo.width == FWIDTH_EXTRA_CONDENSED )
                    o_rResult.SetWidth( WIDTH_EXTRA_CONDENSED );
                else if( aInfo.width == FWIDTH_CONDENSED )
                    o_rResult.SetWidth( WIDTH_CONDENSED );
                else if( aInfo.width == FWIDTH_SEMI_CONDENSED )
                    o_rResult.SetWidth( WIDTH_SEMI_CONDENSED );
                else if( aInfo.width == FWIDTH_NORMAL )
                    o_rResult.SetWidth( WIDTH_NORMAL );
                else if( aInfo.width == FWIDTH_SEMI_EXPANDED )
                    o_rResult.SetWidth( WIDTH_SEMI_EXPANDED );
                else if( aInfo.width == FWIDTH_EXPANDED )
                    o_rResult.SetWidth( WIDTH_EXPANDED );
                else if( aInfo.width == FWIDTH_EXTRA_EXPANDED )
                    o_rResult.SetWidth( WIDTH_EXTRA_EXPANDED );
                else if( aInfo.width >= FWIDTH_ULTRA_EXPANDED )
                    o_rResult.SetWidth( WIDTH_ULTRA_EXPANDED );
            }
            // set italic
            o_rResult.SetItalic( (aInfo.italicAngle != 0) ? ITALIC_NORMAL : ITALIC_NONE );

            // set pitch
            o_rResult.SetPitch( (aInfo.pitch == 0) ? PITCH_VARIABLE : PITCH_FIXED );

            // set style name
            if( aInfo.usubfamily )
                o_rResult.SetStyleName( OUString( aInfo.usubfamily ) );
            else if( aInfo.subfamily )
                o_rResult.SetStyleName( OUString::createFromAscii( aInfo.subfamily ) );

            // cleanup
            CloseTTFont( pTTF );
            // success
            bResult = true;
        }
        return bResult;
    }

    struct WeightSearchEntry
    {
        const char* string;
        int         string_len;
        FontWeight  weight;

        bool operator<( const WeightSearchEntry& rRight ) const
        {
            return rtl_str_compareIgnoreAsciiCase_WithLength( string, string_len, rRight.string, rRight.string_len ) < 0;
        }
    }
    weight_table[] =
    {
        { "black", 5, WEIGHT_BLACK },
        { "bold", 4, WEIGHT_BOLD },
        { "book", 4, WEIGHT_LIGHT },
        { "demi", 4, WEIGHT_SEMIBOLD },
        { "heavy", 5, WEIGHT_BLACK },
        { "light", 5, WEIGHT_LIGHT },
        { "medium", 6, WEIGHT_MEDIUM },
        { "regular", 7, WEIGHT_NORMAL },
        { "super", 5, WEIGHT_ULTRABOLD },
        { "thin", 4, WEIGHT_THIN }
    };

    bool identifyType1Font( const char* i_pBuffer, sal_uInt32 i_nSize, Font& o_rResult )
    {
        bool bResult = false;
        // might be a type1, find eexec
        const char* pStream = i_pBuffer;
        const char* pExec = "eexec";
        const char* pExecPos = std::search( pStream, pStream+i_nSize, pExec, pExec+5 );
        if( pExecPos != pStream+i_nSize)
        {
            // find /FamilyName entry
            static const char* pFam = "/FamilyName";
            const char* pFamPos = std::search( pStream, pExecPos, pFam, pFam+11 );
            if( pFamPos != pExecPos )
            {
                // extract the string value behind /FamilyName
                const char* pOpen = pFamPos+11;
                while( pOpen < pExecPos && *pOpen != '(' )
                    pOpen++;
                const char* pClose = pOpen;
                while( pClose < pExecPos && *pClose != ')' )
                    pClose++;
                if( pClose - pOpen > 1 )
                {
                    o_rResult.SetName( OStringToOUString( OString( pOpen+1, pClose-pOpen-1 ), RTL_TEXTENCODING_ASCII_US ) );
                }
            }

            // parse /ItalicAngle
            static const char* pItalic = "/ItalicAngle";
            const char* pItalicPos = std::search( pStream, pExecPos, pItalic, pItalic+12 );
            if( pItalicPos != pExecPos )
            {
                sal_Int32 nItalic = rtl_str_toInt32( pItalicPos+12, 10 );
                o_rResult.SetItalic( (nItalic != 0) ? ITALIC_NORMAL : ITALIC_NONE );
            }

            // parse /Weight
            static const char* pWeight = "/Weight";
            const char* pWeightPos = std::search( pStream, pExecPos, pWeight, pWeight+7 );
            if( pWeightPos != pExecPos )
            {
                // extract the string value behind /Weight
                const char* pOpen = pWeightPos+7;
                while( pOpen < pExecPos && *pOpen != '(' )
                    pOpen++;
                const char* pClose = pOpen;
                while( pClose < pExecPos && *pClose != ')' )
                    pClose++;
                if( pClose - pOpen > 1 )
                {
                    WeightSearchEntry aEnt;
                    aEnt.string = pOpen+1;
                    aEnt.string_len = (pClose-pOpen)-1;
                    aEnt.weight = WEIGHT_NORMAL;
                    const int nEnt = SAL_N_ELEMENTS( weight_table );
                    WeightSearchEntry* pFound = std::lower_bound( weight_table, weight_table+nEnt, aEnt );
                    if( pFound != (weight_table+nEnt) )
                        o_rResult.SetWeight( pFound->weight );
                }
            }

            // parse isFixedPitch
            static const char* pFixed = "/isFixedPitch";
            const char* pFixedPos = std::search( pStream, pExecPos, pFixed, pFixed+13 );
            if( pFixedPos != pExecPos )
            {
                // skip whitespace
                while( pFixedPos < pExecPos-4 &&
                       ( *pFixedPos == ' '  ||
                         *pFixedPos == '\t' ||
                         *pFixedPos == '\r' ||
                         *pFixedPos == '\n' ) )
                {
                    pFixedPos++;
                }
                // find "true" value
                if( rtl_str_compareIgnoreAsciiCase_WithLength( pFixedPos, 4, "true", 4 ) == 0 )
                    o_rResult.SetPitch( PITCH_FIXED );
                else
                    o_rResult.SetPitch( PITCH_VARIABLE );
            }
        }
        return bResult;
    }
}

Font Font::identifyFont( const void* i_pBuffer, sal_uInt32 i_nSize )
{
    Font aResult;
    if( ! identifyTrueTypeFont( i_pBuffer, i_nSize, aResult ) )
    {
        const char* pStream = reinterpret_cast<const char*>(i_pBuffer);
        if( pStream && i_nSize > 100 &&
             *pStream == '%' && pStream[1] == '!' )
        {
            identifyType1Font( pStream, i_nSize, aResult );
        }
    }

    return aResult;
}

// The inlines from the font.hxx header are now instantiated for pImpl-ification
const Color& Font::GetColor() const { return mpImplFont->maColor; }

const Color& Font::GetFillColor() const { return mpImplFont->maFillColor; }

sal_Bool Font::IsTransparent() const { return mpImplFont->mbTransparent; }

FontAlign Font::GetAlign() const { return mpImplFont->meAlign; }

const OUString& Font::GetName() const { return mpImplFont->maFamilyName; }

const OUString& Font::GetStyleName() const { return mpImplFont->maStyleName; }

const Size& Font::GetSize() const { return mpImplFont->maSize; }

void Font::SetHeight( long nHeight ) { SetSize( Size( mpImplFont->maSize.Width(), nHeight ) ); }

long Font::GetHeight() const { return mpImplFont->maSize.Height(); }

void Font::SetWidth( long nWidth ) { SetSize( Size( nWidth, mpImplFont->maSize.Height() ) ); }

long Font::GetWidth() const { return mpImplFont->maSize.Width(); }

rtl_TextEncoding Font::GetCharSet() const { return mpImplFont->meCharSet; }

LanguageType Font::GetLanguage() const { return mpImplFont->meLanguage; }

LanguageType Font::GetCJKContextLanguage() const { return mpImplFont->meCJKLanguage; }

short Font::GetOrientation() const { return mpImplFont->mnOrientation; }

sal_Bool Font::IsVertical() const { return mpImplFont->mbVertical; }

FontKerning Font::GetKerning() const { return mpImplFont->mnKerning; }

FontPitch Font::GetPitch() const { return mpImplFont->GetPitch(); }

FontWeight Font::GetWeight() const { return mpImplFont->GetWeight(); }

FontWidth Font::GetWidthType() const { return mpImplFont->GetWidthType(); }

FontItalic Font::GetItalic() const { return mpImplFont->GetItalic(); }

FontFamily Font::GetFamily() const { return mpImplFont->GetFamily(); }

sal_Bool Font::IsOutline() const { return mpImplFont->mbOutline; }

sal_Bool Font::IsShadow() const { return mpImplFont->mbShadow; }

FontRelief Font::GetRelief() const { return mpImplFont->meRelief; }

FontUnderline Font::GetUnderline() const { return mpImplFont->meUnderline; }

FontUnderline Font::GetOverline()  const { return mpImplFont->meOverline; }

FontStrikeout Font::GetStrikeout() const { return mpImplFont->meStrikeout; }

FontEmphasisMark Font::GetEmphasisMark() const { return mpImplFont->meEmphasisMark; }

sal_Bool Font::IsWordLineMode() const { return mpImplFont->mbWordLine; }

sal_Bool Font::IsSameInstance( const Font& rFont ) const { return (mpImplFont == rFont.mpImplFont); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
