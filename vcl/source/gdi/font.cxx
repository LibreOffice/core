/*************************************************************************
 *
 *  $RCSfile: font.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 09:26:52 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _VCOMPAT_HXX
#include <tools/vcompat.hxx>
#endif
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_FONT_HXX
#include <font.hxx>
#endif
#ifndef _SV_IMPFONT_HXX
#include <impfont.hxx>
#endif

// =======================================================================

DBG_NAME( Font );

// -----------------------------------------------------------------------

Impl_Font::Impl_Font() :
    maColor( COL_BLACK ),
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
}

// -----------------------------------------------------------------------

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
}

// -----------------------------------------------------------------------

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

// =======================================================================

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

// -----------------------------------------------------------------------

Font::Font()
{
    DBG_CTOR( Font, NULL );

    static Impl_Font aStaticImplFont;
    // RefCount is zero for static objects
    aStaticImplFont.mnRefCount = 0;
    mpImplFont = &aStaticImplFont;
}

// -----------------------------------------------------------------------

Font::Font( const Font& rFont )
{
    DBG_CTOR( Font, NULL );
    DBG_CHKOBJ( &rFont, Font, NULL );
    DBG_ASSERT( rFont.mpImplFont->mnRefCount < 0xFFFE, "Font: RefCount overflow" );

    mpImplFont = rFont.mpImplFont;
    // do not count static objects (where RefCount is zero)
    if ( mpImplFont->mnRefCount )
        mpImplFont->mnRefCount++;
}

// -----------------------------------------------------------------------

Font::Font( const String& rFamilyName, const Size& rSize )
{
    DBG_CTOR( Font, NULL );

    mpImplFont              = new Impl_Font;
    mpImplFont->maFamilyName= rFamilyName;
    mpImplFont->maSize      = rSize;
}

// -----------------------------------------------------------------------

Font::Font( const String& rFamilyName, const String& rStyleName, const Size& rSize )
{
    DBG_CTOR( Font, NULL );

    mpImplFont              = new Impl_Font;
    mpImplFont->maFamilyName= rFamilyName;
    mpImplFont->maStyleName = rStyleName;
    mpImplFont->maSize      = rSize;
}

// -----------------------------------------------------------------------

Font::Font( FontFamily eFamily, const Size& rSize )
{
    DBG_CTOR( Font, NULL );

    mpImplFont              = new Impl_Font;
    mpImplFont->meFamily    = eFamily;
    mpImplFont->maSize      = rSize;
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

void Font::SetColor( const Color& rColor )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->maColor != rColor )
    {
        MakeUnique();
        mpImplFont->maColor = rColor;
    }
}

// -----------------------------------------------------------------------

void Font::SetFillColor( const Color& rColor )
{
    DBG_CHKTHIS( Font, NULL );

    MakeUnique();
    mpImplFont->maFillColor = rColor;
    if ( rColor.GetTransparency() )
        mpImplFont->mbTransparent = true;
}

// -----------------------------------------------------------------------

void Font::SetTransparent( BOOL bTransparent )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->mbTransparent != bTransparent )
    {
        MakeUnique();
        mpImplFont->mbTransparent = bTransparent;
    }
}

// -----------------------------------------------------------------------

void Font::SetAlign( FontAlign eAlign )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->meAlign != eAlign )
    {
        MakeUnique();
        mpImplFont->meAlign = eAlign;
    }
}

// -----------------------------------------------------------------------

void Font::SetName( const String& rFamilyName )
{
    DBG_CHKTHIS( Font, NULL );

    MakeUnique();
    mpImplFont->maFamilyName = rFamilyName;
}

// -----------------------------------------------------------------------

void Font::SetStyleName( const String& rStyleName )
{
    DBG_CHKTHIS( Font, NULL );

    MakeUnique();
    mpImplFont->maStyleName = rStyleName;
}

// -----------------------------------------------------------------------

void Font::SetSize( const Size& rSize )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->maSize != rSize )
    {
        MakeUnique();
        mpImplFont->maSize = rSize;
    }
}

// -----------------------------------------------------------------------

void Font::SetFamily( FontFamily eFamily )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->meFamily != eFamily )
    {
        MakeUnique();
        mpImplFont->meFamily = eFamily;
    }
}

// -----------------------------------------------------------------------

void Font::SetCharSet( CharSet eCharSet )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->meCharSet != eCharSet )
    {
        MakeUnique();
        mpImplFont->meCharSet = eCharSet;
    }
}

// -----------------------------------------------------------------------

void Font::SetLanguage( LanguageType eLanguage )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->meLanguage != eLanguage )
    {
        MakeUnique();
        mpImplFont->meLanguage = eLanguage;
    }
}

// -----------------------------------------------------------------------

void Font::SetCJKContextLanguage( LanguageType eLanguage )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->meCJKLanguage != eLanguage )
    {
        MakeUnique();
        mpImplFont->meCJKLanguage = eLanguage;
    }
}

// -----------------------------------------------------------------------

void Font::SetPitch( FontPitch ePitch )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->mePitch != ePitch )
    {
        MakeUnique();
        mpImplFont->mePitch = ePitch;
    }
}

// -----------------------------------------------------------------------

void Font::SetOrientation( short nOrientation )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->mnOrientation != nOrientation )
    {
        MakeUnique();
        mpImplFont->mnOrientation = nOrientation;
    }
}

// -----------------------------------------------------------------------

void Font::SetVertical( BOOL bVertical )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->mbVertical != bVertical )
    {
        MakeUnique();
        mpImplFont->mbVertical = bVertical;
    }
}

// -----------------------------------------------------------------------

void Font::SetKerning( FontKerning nKerning )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->mnKerning != nKerning )
    {
        MakeUnique();
        mpImplFont->mnKerning = nKerning;
    }
}

// -----------------------------------------------------------------------

BOOL Font::IsKerning() const
{
    return (mpImplFont->mnKerning & KERNING_FONTSPECIFIC) != 0;
}

// -----------------------------------------------------------------------

void Font::SetWeight( FontWeight eWeight )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->meWeight != eWeight )
    {
        MakeUnique();
        mpImplFont->meWeight = eWeight;
    }
}

// -----------------------------------------------------------------------

void Font::SetWidthType( FontWidth eWidth )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->meWidthType != eWidth )
    {
        MakeUnique();
        mpImplFont->meWidthType = eWidth;
    }
}

// -----------------------------------------------------------------------

void Font::SetItalic( FontItalic eItalic )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->meItalic != eItalic )
    {
        MakeUnique();
        mpImplFont->meItalic = eItalic;
    }
}

// -----------------------------------------------------------------------

void Font::SetOutline( BOOL bOutline )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->mbOutline != bOutline )
    {
        MakeUnique();
        mpImplFont->mbOutline = bOutline;
    }
}

// -----------------------------------------------------------------------

void Font::SetShadow( BOOL bShadow )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->mbShadow != bShadow )
    {
        MakeUnique();
        mpImplFont->mbShadow = bShadow;
    }
}

// -----------------------------------------------------------------------

void Font::SetUnderline( FontUnderline eUnderline )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->meUnderline != eUnderline )
    {
        MakeUnique();
        mpImplFont->meUnderline = eUnderline;
    }
}

// -----------------------------------------------------------------------

void Font::SetStrikeout( FontStrikeout eStrikeout )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->meStrikeout != eStrikeout )
    {
        MakeUnique();
        mpImplFont->meStrikeout = eStrikeout;
    }
}

// -----------------------------------------------------------------------

void Font::SetRelief( FontRelief eRelief )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->meRelief != eRelief )
    {
        MakeUnique();
        mpImplFont->meRelief = eRelief;
    }
}

// -----------------------------------------------------------------------

void Font::SetEmphasisMark( FontEmphasisMark eEmphasisMark )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->meEmphasisMark != eEmphasisMark )
    {
        MakeUnique();
        mpImplFont->meEmphasisMark = eEmphasisMark;
    }
}

// -----------------------------------------------------------------------

void Font::SetWordLineMode( BOOL bWordLine )
{
    DBG_CHKTHIS( Font, NULL );

    if( mpImplFont->mbWordLine != bWordLine )
    {
        MakeUnique();
        mpImplFont->mbWordLine = bWordLine;
    }
}

// -----------------------------------------------------------------------

Font& Font::operator=( const Font& rFont )
{
    DBG_CHKTHIS( Font, NULL );
    DBG_CHKOBJ( &rFont, Font, NULL );
    DBG_ASSERT( rFont.mpImplFont->mnRefCount < 0xFFFE, "Font: RefCount overflow" );

    // Zuerst Referenzcounter erhoehen, damit man sich selbst zuweisen kann
    // RefCount == 0 fuer statische Objekte
    if ( rFont.mpImplFont->mnRefCount )
        rFont.mpImplFont->mnRefCount++;

    // Wenn es keine statischen ImplDaten sind, dann loeschen, wenn es
    // die letzte Referenz ist, sonst Referenzcounter decrementieren
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

// -----------------------------------------------------------------------

BOOL Font::operator==( const Font& rFont ) const
{
    DBG_CHKTHIS( Font, NULL );
    DBG_CHKOBJ( &rFont, Font, NULL );

    if( mpImplFont == rFont.mpImplFont )
        return TRUE;
    if( *mpImplFont == *rFont.mpImplFont )
        return TRUE;

    return FALSE;
}

// -----------------------------------------------------------------------

void Font::Merge( const Font& rFont )
{
    if ( rFont.GetName().Len() )
    {
        SetName( rFont.GetName() );
        SetStyleName( rFont.GetStyleName() );
        SetFamily( rFont.GetFamily() );
        SetCharSet( GetCharSet() );
        SetLanguage( rFont.GetLanguage() );
        SetCJKContextLanguage( rFont.GetCJKContextLanguage() );
        SetPitch( rFont.GetPitch() );
    }

    if ( rFont.GetSize().Height() )
        SetSize( rFont.GetSize() );
    if ( rFont.GetWeight() != WEIGHT_DONTKNOW )
        SetWeight( rFont.GetWeight() );
    if ( rFont.GetWidthType() != WIDTH_DONTKNOW )
        SetWidthType( rFont.GetWidthType() );
    if ( rFont.GetItalic() != ITALIC_DONTKNOW )
        SetItalic( rFont.GetItalic() );
    if ( rFont.GetUnderline() != UNDERLINE_DONTKNOW )
    {
        SetUnderline( rFont.GetUnderline() );
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

// -----------------------------------------------------------------------

SvStream& operator>>( SvStream& rIStm, Impl_Font& rImpl_Font )
{
    VersionCompat   aCompat( rIStm, STREAM_READ );
    UINT16          nTmp16;
    BOOL            bTmp;
    BYTE            nTmp8;

    rIStm.ReadByteString( rImpl_Font.maFamilyName, rIStm.GetStreamCharSet() );
    rIStm.ReadByteString( rImpl_Font.maStyleName, rIStm.GetStreamCharSet() );
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
    // Relief
    // CJKContextLanguage

    return rIStm;
}

// -----------------------------------------------------------------------

SvStream& operator<<( SvStream& rOStm, const Impl_Font& rImpl_Font )
{
    VersionCompat aCompat( rOStm, STREAM_WRITE, 2 );
    rOStm.WriteByteString( rImpl_Font.maFamilyName, rOStm.GetStreamCharSet() );
    rOStm.WriteByteString( rImpl_Font.maStyleName, rOStm.GetStreamCharSet() );
    rOStm << rImpl_Font.maSize;

    rOStm << (UINT16) GetStoreCharSet( rImpl_Font.meCharSet, rOStm.GetVersion() );
    rOStm << (UINT16) rImpl_Font.meFamily;
    rOStm << (UINT16) rImpl_Font.mePitch;
    rOStm << (UINT16) rImpl_Font.meWeight;
    rOStm << (UINT16) rImpl_Font.meUnderline;
    rOStm << (UINT16) rImpl_Font.meStrikeout;
    rOStm << (UINT16) rImpl_Font.meItalic;
    rOStm << (UINT16) rImpl_Font.meLanguage;
    rOStm << (UINT16) rImpl_Font.meWidthType;

    rOStm << rImpl_Font.mnOrientation;

    rOStm << (BOOL) rImpl_Font.mbWordLine;
    rOStm << (BOOL) rImpl_Font.mbOutline;
    rOStm << (BOOL) rImpl_Font.mbShadow;
    rOStm << (BYTE) rImpl_Font.mnKerning;

    // new in version 2
    rOStm << (BYTE)     rImpl_Font.meRelief;
    rOStm << (UINT16)   rImpl_Font.meCJKLanguage;
    rOStm << (BOOL)     rImpl_Font.mbVertical;
    rOStm << (UINT16)   rImpl_Font.meEmphasisMark;

    return rOStm;
}

// -----------------------------------------------------------------------

SvStream& operator>>( SvStream& rIStm, Font& rFont )
{
    rFont.MakeUnique();
    return( rIStm >> *rFont.mpImplFont );
}

// -----------------------------------------------------------------------

SvStream& operator<<( SvStream& rOStm, const Font& rFont )
{
    return( rOStm << *rFont.mpImplFont );
}

// -----------------------------------------------------------------------

// the inlines from the font.hxx header moved here during pImp-lification
// TODO: reformat
const Color& Font::GetColor() const { return mpImplFont->maColor; }
const Color& Font::GetFillColor() const { return mpImplFont->maFillColor; }
BOOL Font::IsTransparent() const { return mpImplFont->mbTransparent; }
FontAlign Font::GetAlign() const { return mpImplFont->meAlign; }
const String& Font::GetName() const { return mpImplFont->maFamilyName; }
const String& Font::GetStyleName() const { return mpImplFont->maStyleName; }
const Size& Font::GetSize() const { return mpImplFont->maSize; }
void Font::SetHeight( long nHeight ) { SetSize( Size( mpImplFont->maSize.Width(), nHeight ) ); }
long Font::GetHeight() const { return mpImplFont->maSize.Height(); }
void Font::SetWidth( long nWidth ) { SetSize( Size( nWidth, mpImplFont->maSize.Height() ) ); }
long Font::GetWidth() const { return mpImplFont->maSize.Width(); }
FontFamily Font::GetFamily() const { return mpImplFont->meFamily; }
rtl_TextEncoding Font::GetCharSet() const { return mpImplFont->meCharSet; }
LanguageType Font::GetLanguage() const { return mpImplFont->meLanguage; }
LanguageType Font::GetCJKContextLanguage() const { return mpImplFont->meCJKLanguage; }
FontPitch Font::GetPitch() const { return mpImplFont->mePitch; }
short Font::GetOrientation() const { return mpImplFont->mnOrientation; }
BOOL Font::IsVertical() const { return mpImplFont->mbVertical; }
FontKerning Font::GetKerning() const { return mpImplFont->mnKerning; }
FontWeight Font::GetWeight() const { return mpImplFont->meWeight; }
FontWidth Font::GetWidthType() const { return mpImplFont->meWidthType; }
FontItalic Font::GetItalic() const { return mpImplFont->meItalic; }
BOOL Font::IsOutline() const { return mpImplFont->mbOutline; }
BOOL Font::IsShadow() const { return mpImplFont->mbShadow; }
FontRelief Font::GetRelief() const { return mpImplFont->meRelief; }
FontUnderline Font::GetUnderline() const { return mpImplFont->meUnderline; }
FontStrikeout Font::GetStrikeout() const { return mpImplFont->meStrikeout; }
FontEmphasisMark Font::GetEmphasisMark() const { return mpImplFont->meEmphasisMark; }
BOOL Font::IsWordLineMode() const { return mpImplFont->mbWordLine; }
BOOL Font::IsSameInstance( const Font& rFont ) const { return (mpImplFont == rFont.mpImplFont); }
