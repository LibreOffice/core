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

Impl_Font::Impl_Font() :
    maColor( COL_TRANSPARENT ),
    maFillColor( COL_TRANSPARENT ),
    maLanguageTag( LANGUAGE_DONTKNOW ),
    maCJKLanguageTag( LANGUAGE_DONTKNOW )
{
    mnRefCount          = 1;
    meCharSet           = RTL_TEXTENCODING_DONTKNOW;
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
    mnKerning           = FontKerning::NONE;
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
    maFillColor( rImplFont.maFillColor ),
    maLanguageTag( rImplFont.maLanguageTag ),
    maCJKLanguageTag( rImplFont.maCJKLanguageTag )
{
    mnRefCount          = 1;
    meCharSet           = rImplFont.meCharSet;
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

    if( (meCharSet        != rOther.meCharSet)
    ||  (maLanguageTag    != rOther.maLanguageTag)
    ||  (maCJKLanguageTag != rOther.maCJKLanguageTag)
    ||  (meAlign          != rOther.meAlign) )
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
    ImplFontAttrs nType = ImplFontAttrs::None;
    FontWeight  eWeight = WEIGHT_DONTKNOW;
    FontWidth   eWidthType = WIDTH_DONTKNOW;
    OUString    aMapName = GetEnglishSearchFontName( maFamilyName );

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
            if ( pFontAttr->Type & ImplFontAttrs::Serif )
                meFamily = FAMILY_ROMAN;
            else if ( pFontAttr->Type & ImplFontAttrs::SansSerif )
                meFamily = FAMILY_SWISS;
            else if ( pFontAttr->Type & ImplFontAttrs::Typewriter )
                meFamily = FAMILY_MODERN;
            else if ( pFontAttr->Type & ImplFontAttrs::Italic )
                meFamily = FAMILY_SCRIPT;
            else if ( pFontAttr->Type & ImplFontAttrs::Decorative )
                meFamily = FAMILY_DECORATIVE;
        }

        if( mePitch == PITCH_DONTKNOW )
        {
            if ( pFontAttr->Type & ImplFontAttrs::Fixed )
                mePitch = PITCH_FIXED;
        }
    }

    // if some attributes are still unknown then use the FontSubst magic
    if( meFamily == FAMILY_DONTKNOW )
    {
        if( nType & ImplFontAttrs::Serif )
            meFamily = FAMILY_ROMAN;
        else if( nType & ImplFontAttrs::SansSerif )
            meFamily = FAMILY_SWISS;
        else if( nType & ImplFontAttrs::Typewriter )
            meFamily = FAMILY_MODERN;
        else if( nType & ImplFontAttrs::Italic )
            meFamily = FAMILY_SCRIPT;
        else if( nType & ImplFontAttrs::Decorative )
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
    static Impl_Font aStaticImplFont;
    // RefCount is zero for static objects
    aStaticImplFont.mnRefCount = 0;
    mpImplFont = &aStaticImplFont;
}

Font::Font( const vcl::Font& rFont )
{
    bool bRefIncrementable = rFont.mpImplFont->mnRefCount < ::std::numeric_limits<sal_uInt32>::max();
    DBG_ASSERT( bRefIncrementable, "Font: RefCount overflow" );

    mpImplFont = rFont.mpImplFont;
    // do not count static objects (where RefCount is zero)
    if ( mpImplFont->mnRefCount && bRefIncrementable )
        mpImplFont->mnRefCount++;
}

Font::Font( const OUString& rFamilyName, const Size& rSize )
{
    mpImplFont               = new Impl_Font;
    mpImplFont->maFamilyName = rFamilyName;
    mpImplFont->maSize       = rSize;
}

Font::Font( const OUString& rFamilyName, const OUString& rStyleName, const Size& rSize )
{
    mpImplFont              = new Impl_Font;
    mpImplFont->maFamilyName= rFamilyName;
    mpImplFont->maStyleName = rStyleName;
    mpImplFont->maSize      = rSize;
}

Font::Font( FontFamily eFamily, const Size& rSize )
{
    mpImplFont              = new Impl_Font;
    mpImplFont->meFamily    = eFamily;
    mpImplFont->maSize      = rSize;
}

Font::~Font()
{
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
    if( mpImplFont->maColor != rColor )
    {
        MakeUnique();
        mpImplFont->maColor = rColor;
    }
}

void Font::SetFillColor( const Color& rColor )
{
    MakeUnique();
    mpImplFont->maFillColor = rColor;
    if ( rColor.GetTransparency() )
        mpImplFont->mbTransparent = true;
}

void Font::SetTransparent( bool bTransparent )
{
    if( mpImplFont->mbTransparent != bTransparent )
    {
        MakeUnique();
        mpImplFont->mbTransparent = bTransparent;
    }
}

void Font::SetAlign( FontAlign eAlign )
{
    if( mpImplFont->meAlign != eAlign )
    {
        MakeUnique();
        mpImplFont->meAlign = eAlign;
    }
}

void Font::SetName( const OUString& rFamilyName )
{
    MakeUnique();
    mpImplFont->maFamilyName = rFamilyName;
}

void Font::SetStyleName( const OUString& rStyleName )
{
    MakeUnique();
    mpImplFont->maStyleName = rStyleName;
}

void Font::SetSize( const Size& rSize )
{
    if( mpImplFont->maSize != rSize )
    {
        MakeUnique();
        mpImplFont->maSize = rSize;
    }
}

void Font::SetFamily( FontFamily eFamily )
{
    if( mpImplFont->meFamily != eFamily )
    {
        MakeUnique();
        mpImplFont->meFamily = eFamily;
    }
}

void Font::SetCharSet( rtl_TextEncoding eCharSet )
{
    if( mpImplFont->meCharSet != eCharSet )
    {
        MakeUnique();
        mpImplFont->meCharSet = eCharSet;
    }
}

void Font::SetLanguageTag( const LanguageTag& rLanguageTag )
{
    if( mpImplFont->maLanguageTag != rLanguageTag )
    {
        MakeUnique();
        mpImplFont->maLanguageTag = rLanguageTag;
    }
}

void Font::SetCJKContextLanguageTag( const LanguageTag& rLanguageTag )
{
    if( mpImplFont->maCJKLanguageTag != rLanguageTag )
    {
        MakeUnique();
        mpImplFont->maCJKLanguageTag = rLanguageTag;
    }
}

void Font::SetLanguage( LanguageType eLanguage )
{
    if( mpImplFont->maLanguageTag.getLanguageType( false) != eLanguage )
    {
        MakeUnique();
        mpImplFont->maLanguageTag.reset( eLanguage);
    }
}

void Font::SetCJKContextLanguage( LanguageType eLanguage )
{
    if( mpImplFont->maCJKLanguageTag.getLanguageType( false) != eLanguage )
    {
        MakeUnique();
        mpImplFont->maCJKLanguageTag.reset( eLanguage);
    }
}

void Font::SetPitch( FontPitch ePitch )
{
    if( mpImplFont->mePitch != ePitch )
    {
        MakeUnique();
        mpImplFont->mePitch = ePitch;
    }
}

void Font::SetOrientation( short nOrientation )
{
    if( mpImplFont->mnOrientation != nOrientation )
    {
        MakeUnique();
        mpImplFont->mnOrientation = nOrientation;
    }
}

void Font::SetVertical( bool bVertical )
{
    if( mpImplFont->mbVertical != bVertical )
    {
        MakeUnique();
        mpImplFont->mbVertical = bVertical;
    }
}

void Font::SetKerning( FontKerning nKerning )
{
    if( mpImplFont->mnKerning != nKerning )
    {
        MakeUnique();
        mpImplFont->mnKerning = nKerning;
    }
}

bool Font::IsKerning() const
{
    return bool(mpImplFont->mnKerning & FontKerning::FontSpecific);
}

void Font::SetWeight( FontWeight eWeight )
{
    if( mpImplFont->meWeight != eWeight )
    {
        MakeUnique();
        mpImplFont->meWeight = eWeight;
    }
}

void Font::SetWidthType( FontWidth eWidth )
{
    if( mpImplFont->meWidthType != eWidth )
    {
        MakeUnique();
        mpImplFont->meWidthType = eWidth;
    }
}

void Font::SetItalic( FontItalic eItalic )
{
    if( mpImplFont->meItalic != eItalic )
    {
        MakeUnique();
        mpImplFont->meItalic = eItalic;
    }
}

void Font::SetOutline( bool bOutline )
{
    if( mpImplFont->mbOutline != bOutline )
    {
        MakeUnique();
        mpImplFont->mbOutline = bOutline;
    }
}

void Font::SetShadow( bool bShadow )
{
    if( mpImplFont->mbShadow != bShadow )
    {
        MakeUnique();
        mpImplFont->mbShadow = bShadow;
    }
}

void Font::SetUnderline( FontUnderline eUnderline )
{
    if( mpImplFont->meUnderline != eUnderline )
    {
        MakeUnique();
        mpImplFont->meUnderline = eUnderline;
    }
}

void Font::SetOverline( FontUnderline eOverline )
{
    if( mpImplFont->meOverline != eOverline )
    {
        MakeUnique();
        mpImplFont->meOverline = eOverline;
    }
}

void Font::SetStrikeout( FontStrikeout eStrikeout )
{
    if( mpImplFont->meStrikeout != eStrikeout )
    {
        MakeUnique();
        mpImplFont->meStrikeout = eStrikeout;
    }
}

void Font::SetRelief( FontRelief eRelief )
{
    if( mpImplFont->meRelief != eRelief )
    {
        MakeUnique();
        mpImplFont->meRelief = eRelief;
    }
}

void Font::SetEmphasisMark( FontEmphasisMark eEmphasisMark )
{
    if( mpImplFont->meEmphasisMark != eEmphasisMark )
    {
        MakeUnique();
        mpImplFont->meEmphasisMark = eEmphasisMark;
    }
}

void Font::SetWordLineMode( bool bWordLine )
{
    if( mpImplFont->mbWordLine != bWordLine )
    {
        MakeUnique();
        mpImplFont->mbWordLine = bWordLine;
    }
}

Font& Font::operator=( const vcl::Font& rFont )
{
    bool bRefIncrementable = rFont.mpImplFont->mnRefCount < ::std::numeric_limits<sal_uInt32>::max();
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

bool Font::operator==( const vcl::Font& rFont ) const
{
    return mpImplFont == rFont.mpImplFont || *mpImplFont == *rFont.mpImplFont;
}

void Font::Merge( const vcl::Font& rFont )
{
    if ( !rFont.GetFamilyName().isEmpty() )
    {
        SetName( rFont.GetFamilyName() );
        SetStyleName( rFont.GetStyleName() );
        SetCharSet( GetCharSet() );
        SetLanguageTag( rFont.GetLanguageTag() );
        SetCJKContextLanguageTag( rFont.GetCJKContextLanguageTag() );
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
    SetKerning( rFont.IsKerning() ? FontKerning::FontSpecific : FontKerning::NONE );
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

SvStream& ReadImpl_Font( SvStream& rIStm, Impl_Font& rImpl_Font )
{
    VersionCompat   aCompat( rIStm, StreamMode::READ );
    sal_uInt16          nTmp16;
    bool            bTmp;
    sal_uInt8           nTmp8;

    rImpl_Font.maFamilyName = rIStm.ReadUniOrByteString(rIStm.GetStreamCharSet());
    rImpl_Font.maStyleName = rIStm.ReadUniOrByteString(rIStm.GetStreamCharSet());
    ReadPair( rIStm, rImpl_Font.maSize );

    rIStm.ReadUInt16( nTmp16 ); rImpl_Font.meCharSet = (rtl_TextEncoding) nTmp16;
    rIStm.ReadUInt16( nTmp16 ); rImpl_Font.meFamily = (FontFamily) nTmp16;
    rIStm.ReadUInt16( nTmp16 ); rImpl_Font.mePitch = (FontPitch) nTmp16;
    rIStm.ReadUInt16( nTmp16 ); rImpl_Font.meWeight = (FontWeight) nTmp16;
    rIStm.ReadUInt16( nTmp16 ); rImpl_Font.meUnderline = (FontUnderline) nTmp16;
    rIStm.ReadUInt16( nTmp16 ); rImpl_Font.meStrikeout = (FontStrikeout) nTmp16;
    rIStm.ReadUInt16( nTmp16 ); rImpl_Font.meItalic = (FontItalic) nTmp16;
    rIStm.ReadUInt16( nTmp16 ); rImpl_Font.maLanguageTag.reset( (LanguageType) nTmp16);
    rIStm.ReadUInt16( nTmp16 ); rImpl_Font.meWidthType = (FontWidth) nTmp16;

    rIStm.ReadInt16( rImpl_Font.mnOrientation );

    rIStm.ReadCharAsBool( bTmp ); rImpl_Font.mbWordLine = bTmp;
    rIStm.ReadCharAsBool( bTmp ); rImpl_Font.mbOutline = bTmp;
    rIStm.ReadCharAsBool( bTmp ); rImpl_Font.mbShadow = bTmp;
    rIStm.ReadUChar( nTmp8 ); rImpl_Font.mnKerning = static_cast<FontKerning>(nTmp8);

    if( aCompat.GetVersion() >= 2 )
    {
        rIStm.ReadUChar( nTmp8 );     rImpl_Font.meRelief = (FontRelief)nTmp8;
        rIStm.ReadUInt16( nTmp16 );    rImpl_Font.maCJKLanguageTag.reset( (LanguageType)nTmp16);
        rIStm.ReadCharAsBool( bTmp );      rImpl_Font.mbVertical = bTmp;
        rIStm.ReadUInt16( nTmp16 );    rImpl_Font.meEmphasisMark = (FontEmphasisMark)nTmp16;
    }
    if( aCompat.GetVersion() >= 3 )
    {
        rIStm.ReadUInt16( nTmp16 ); rImpl_Font.meOverline = (FontUnderline) nTmp16;
    }
    // Relief
    // CJKContextLanguage

    return rIStm;
}

SvStream& WriteImpl_Font( SvStream& rOStm, const Impl_Font& rImpl_Font )
{
    VersionCompat aCompat( rOStm, StreamMode::WRITE, 3 );
    rOStm.WriteUniOrByteString( rImpl_Font.maFamilyName, rOStm.GetStreamCharSet() );
    rOStm.WriteUniOrByteString( rImpl_Font.maStyleName, rOStm.GetStreamCharSet() );
    WritePair( rOStm, rImpl_Font.maSize );

    rOStm.WriteUInt16( GetStoreCharSet( rImpl_Font.meCharSet ) );
    rOStm.WriteUInt16( rImpl_Font.meFamily );
    rOStm.WriteUInt16( rImpl_Font.mePitch );
    rOStm.WriteUInt16( rImpl_Font.meWeight );
    rOStm.WriteUInt16( rImpl_Font.meUnderline );
    rOStm.WriteUInt16( rImpl_Font.meStrikeout );
    rOStm.WriteUInt16( rImpl_Font.meItalic );
    rOStm.WriteUInt16( rImpl_Font.maLanguageTag.getLanguageType( false) );
    rOStm.WriteUInt16( rImpl_Font.meWidthType );

    rOStm.WriteInt16( rImpl_Font.mnOrientation );

    rOStm.WriteBool( rImpl_Font.mbWordLine );
    rOStm.WriteBool( rImpl_Font.mbOutline );
    rOStm.WriteBool( rImpl_Font.mbShadow );
    rOStm.WriteUChar( static_cast<sal_uInt8>(rImpl_Font.mnKerning) );

    // new in version 2
    rOStm.WriteUChar( rImpl_Font.meRelief );
    rOStm.WriteUInt16( rImpl_Font.maCJKLanguageTag.getLanguageType( false) );
    rOStm.WriteBool( rImpl_Font.mbVertical );
    rOStm.WriteUInt16( rImpl_Font.meEmphasisMark );

    // new in version 3
    rOStm.WriteUInt16( rImpl_Font.meOverline );

    return rOStm;
}

SvStream& ReadFont( SvStream& rIStm, vcl::Font& rFont )
{
    rFont.MakeUnique();
    return ReadImpl_Font( rIStm, *rFont.mpImplFont );
}

SvStream& WriteFont( SvStream& rOStm, const vcl::Font& rFont )
{
    return WriteImpl_Font( rOStm, *rFont.mpImplFont );
}

namespace
{
    bool identifyTrueTypeFont( const void* i_pBuffer, sal_uInt32 i_nSize, Font& o_rResult )
    {
        bool bResult = false;
        TrueTypeFont* pTTF = nullptr;
        if( OpenTTFontBuffer( i_pBuffer, i_nSize, 0, &pTTF ) == SF_OK )
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
        const char* pStream = static_cast<const char*>(i_pBuffer);
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

bool Font::IsTransparent() const { return mpImplFont->mbTransparent; }

FontAlign Font::GetAlign() const { return mpImplFont->meAlign; }

const OUString& Font::GetFamilyName() const { return mpImplFont->maFamilyName; }

const OUString& Font::GetStyleName() const { return mpImplFont->maStyleName; }

const Size& Font::GetSize() const { return mpImplFont->maSize; }

void Font::SetHeight( long nHeight ) { SetSize( Size( mpImplFont->maSize.Width(), nHeight ) ); }

long Font::GetHeight() const { return mpImplFont->maSize.Height(); }

void Font::SetWidth( long nWidth ) { SetSize( Size( nWidth, mpImplFont->maSize.Height() ) ); }

long Font::GetWidth() const { return mpImplFont->maSize.Width(); }

rtl_TextEncoding Font::GetCharSet() const { return mpImplFont->meCharSet; }

const LanguageTag& Font::GetLanguageTag() const { return mpImplFont->maLanguageTag; }

const LanguageTag& Font::GetCJKContextLanguageTag() const { return mpImplFont->maCJKLanguageTag; }

LanguageType Font::GetLanguage() const { return mpImplFont->maLanguageTag.getLanguageType( false); }

LanguageType Font::GetCJKContextLanguage() const { return mpImplFont->maCJKLanguageTag.getLanguageType( false); }

short Font::GetOrientation() const { return mpImplFont->mnOrientation; }

bool Font::IsVertical() const { return mpImplFont->mbVertical; }

FontKerning Font::GetKerning() const { return mpImplFont->mnKerning; }

FontPitch Font::GetPitch() const { return mpImplFont->GetPitch(); }

FontWeight Font::GetWeight() const { return mpImplFont->GetWeight(); }

FontWidth Font::GetWidthType() const { return mpImplFont->GetWidthType(); }

FontItalic Font::GetItalic() const { return mpImplFont->GetItalic(); }

FontFamily Font::GetFamily() const { return mpImplFont->GetFamily(); }

bool Font::IsOutline() const { return mpImplFont->mbOutline; }

bool Font::IsShadow() const { return mpImplFont->mbShadow; }

FontRelief Font::GetRelief() const { return mpImplFont->meRelief; }

FontUnderline Font::GetUnderline() const { return mpImplFont->meUnderline; }

FontUnderline Font::GetOverline()  const { return mpImplFont->meOverline; }

FontStrikeout Font::GetStrikeout() const { return mpImplFont->meStrikeout; }

FontEmphasisMark Font::GetEmphasisMark() const { return mpImplFont->meEmphasisMark; }

bool Font::IsWordLineMode() const { return mpImplFont->mbWordLine; }

bool Font::IsSameInstance( const vcl::Font& rFont ) const { return (mpImplFont == rFont.mpImplFont); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
