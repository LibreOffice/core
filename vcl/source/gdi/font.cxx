/*************************************************************************
 *
 *  $RCSfile: font.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: th $ $Date: 2000-10-27 14:41:44 $
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

#define _SV_FONT_CXX

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
    meFamily            = FAMILY_DONTKNOW;
    mePitch             = PITCH_DONTKNOW;
    meAlign             = ALIGN_TOP;
    meWeight            = WEIGHT_DONTKNOW;
    meWidthType         = WIDTH_DONTKNOW;
    meUnderline         = UNDERLINE_NONE;
    meStrikeout         = STRIKEOUT_NONE;
    meEmphasisMark      = EMPHASISMARK_NONE;
    meItalic            = ITALIC_NONE;
    mbWordLine          = FALSE;
    mbOutline           = FALSE;
    mbShadow            = FALSE;
    mbKerning           = FALSE;
    mbTransparent       = TRUE;
    mnOrientation       = 0;
    mbVertical          = FALSE;
}

// -----------------------------------------------------------------------

Impl_Font::Impl_Font( const Impl_Font& rImplFont ) :
    maColor( rImplFont.maColor ),
    maFillColor( rImplFont.maFillColor ),
    maName( rImplFont.maName ),
    maStyleName( rImplFont.maStyleName ),
    maSize( rImplFont.maSize )
{
    mnRefCount          = 1;
    meCharSet           = rImplFont.meCharSet;
    meLanguage          = rImplFont.meLanguage;
    meFamily            = rImplFont.meFamily;
    mePitch             = rImplFont.mePitch;
    meAlign             = rImplFont.meAlign;
    meWeight            = rImplFont.meWeight;
    meWidthType         = rImplFont.meWidthType;
    meUnderline         = rImplFont.meUnderline;
    meStrikeout         = rImplFont.meStrikeout;
    meEmphasisMark      = rImplFont.meEmphasisMark;
    meItalic            = rImplFont.meItalic;
    mbWordLine          = rImplFont.mbWordLine;
    mbOutline           = rImplFont.mbOutline;
    mbShadow            = rImplFont.mbShadow;
    mbKerning           = rImplFont.mbKerning;
    mbTransparent       = rImplFont.mbTransparent;
    mnOrientation       = rImplFont.mnOrientation;
    mbVertical          = rImplFont.mbVertical;
}

// -----------------------------------------------------------------------

void Font::MakeUnique()
{
    // Falls noch andere Referenzen bestehen, dann kopieren
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

#ifdef WIN
    static Impl_Font _near aStaticImplFont;
#else
    static Impl_Font aStaticImplFont;
#endif
    // RefCount == 0 fuer statische Objekte
    aStaticImplFont.mnRefCount = 0;
    mpImplFont = &aStaticImplFont;
}

// -----------------------------------------------------------------------

Font::Font( const Font& rFont )
{
    DBG_CTOR( Font, NULL );
    DBG_CHKOBJ( &rFont, Font, NULL );
    DBG_ASSERT( rFont.mpImplFont->mnRefCount < 0xFFFE, "Font: RefCount overflow" );

    // shared Instance Daten uebernehmen und Referenzcounter erhoehen
    mpImplFont = rFont.mpImplFont;
    // RefCount == 0 fuer statische Objekte
    if ( mpImplFont->mnRefCount )
        mpImplFont->mnRefCount++;
}

// -----------------------------------------------------------------------

Font::Font( const XubString& rName, const Size& rSize )
{
    DBG_CTOR( Font, NULL );

    mpImplFont              = new Impl_Font;
    mpImplFont->maName      = rName;
    mpImplFont->maSize      = rSize;
}

// -----------------------------------------------------------------------

Font::Font( const XubString& rName, const XubString& rStyleName, const Size& rSize )
{
    DBG_CTOR( Font, NULL );

    mpImplFont              = new Impl_Font;
    mpImplFont->maName      = rName;
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

    // Wenn es keine statischen ImplDaten sind, dann loeschen, wenn es
    // die letzte Referenz ist, sonst Referenzcounter decrementieren
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

    MakeUnique();
    mpImplFont->maColor = rColor;
}

// -----------------------------------------------------------------------

void Font::SetFillColor( const Color& rColor )
{
    DBG_CHKTHIS( Font, NULL );

    MakeUnique();
    mpImplFont->maFillColor = rColor;
    if ( rColor.GetTransparency() )
        mpImplFont->mbTransparent = TRUE;
}

// -----------------------------------------------------------------------

void Font::SetTransparent( BOOL bTransparent )
{
    DBG_CHKTHIS( Font, NULL );

    MakeUnique();
    mpImplFont->mbTransparent = bTransparent;
}

// -----------------------------------------------------------------------

void Font::SetAlign( FontAlign eAlign )
{
    DBG_CHKTHIS( Font, NULL );

    MakeUnique();
    mpImplFont->meAlign = eAlign;
}

// -----------------------------------------------------------------------

void Font::SetName( const XubString& rName )
{
    DBG_CHKTHIS( Font, NULL );

    MakeUnique();
    mpImplFont->maName = rName;
}

// -----------------------------------------------------------------------

void Font::SetStyleName( const XubString& rStyleName )
{
    DBG_CHKTHIS( Font, NULL );

    MakeUnique();
    mpImplFont->maStyleName = rStyleName;
}

// -----------------------------------------------------------------------

void Font::SetSize( const Size& rSize )
{
    DBG_CHKTHIS( Font, NULL );

    MakeUnique();
    mpImplFont->maSize = rSize;
}

// -----------------------------------------------------------------------

void Font::SetFamily( FontFamily eFamily )
{
    DBG_CHKTHIS( Font, NULL );

    MakeUnique();
    mpImplFont->meFamily = eFamily;
}

// -----------------------------------------------------------------------

void Font::SetCharSet( CharSet eCharSet )
{
    DBG_CHKTHIS( Font, NULL );

    MakeUnique();
    mpImplFont->meCharSet = eCharSet;
}

// -----------------------------------------------------------------------

void Font::SetLanguage( LanguageType eLanguage )
{
    DBG_CHKTHIS( Font, NULL );

    MakeUnique();
    mpImplFont->meLanguage = eLanguage;
}

// -----------------------------------------------------------------------

void Font::SetPitch( FontPitch ePitch )
{
    DBG_CHKTHIS( Font, NULL );

    MakeUnique();
    mpImplFont->mePitch = ePitch;
}

// -----------------------------------------------------------------------

void Font::SetOrientation( short nOrientation )
{
    DBG_CHKTHIS( Font, NULL );

    MakeUnique();
    mpImplFont->mnOrientation = nOrientation;
}

// -----------------------------------------------------------------------

void Font::SetVertical( BOOL bVertical )
{
    DBG_CHKTHIS( Font, NULL );

    MakeUnique();
    mpImplFont->mbVertical = bVertical;
}

// -----------------------------------------------------------------------

void Font::SetKerning( BOOL bKerning )
{
    DBG_CHKTHIS( Font, NULL );

    MakeUnique();
    mpImplFont->mbKerning = bKerning;
}

// -----------------------------------------------------------------------

void Font::SetWeight( FontWeight eWeight )
{
    DBG_CHKTHIS( Font, NULL );

    MakeUnique();
    mpImplFont->meWeight = eWeight;
}

// -----------------------------------------------------------------------

void Font::SetWidthType( FontWidth eWidth )
{
    DBG_CHKTHIS( Font, NULL );

    MakeUnique();
    mpImplFont->meWidthType = eWidth;
}

// -----------------------------------------------------------------------

void Font::SetItalic( FontItalic eItalic )
{
    DBG_CHKTHIS( Font, NULL );

    MakeUnique();
    mpImplFont->meItalic = eItalic;
}

// -----------------------------------------------------------------------

void Font::SetOutline( BOOL bOutline )
{
    DBG_CHKTHIS( Font, NULL );

    MakeUnique();
    mpImplFont->mbOutline = bOutline;
}

// -----------------------------------------------------------------------

void Font::SetShadow( BOOL bShadow )
{
    DBG_CHKTHIS( Font, NULL );

    MakeUnique();
    mpImplFont->mbShadow = bShadow;
}

// -----------------------------------------------------------------------

void Font::SetUnderline( FontUnderline eUnderline )
{
    DBG_CHKTHIS( Font, NULL );

    MakeUnique();
    mpImplFont->meUnderline = eUnderline;
}

// -----------------------------------------------------------------------

void Font::SetStrikeout( FontStrikeout eStrikeout )
{
    DBG_CHKTHIS( Font, NULL );

    MakeUnique();
    mpImplFont->meStrikeout = eStrikeout;
}

// -----------------------------------------------------------------------

void Font::SetEmphasisMark( FontEmphasisMark eEmphasisMark )
{
    DBG_CHKTHIS( Font, NULL );

    MakeUnique();
    mpImplFont->meEmphasisMark = eEmphasisMark;
}

// -----------------------------------------------------------------------

void Font::SetWordLineMode( BOOL bWordLine )
{
    DBG_CHKTHIS( Font, NULL );

    MakeUnique();
    mpImplFont->mbWordLine = bWordLine;
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

    if ( mpImplFont == rFont.mpImplFont )
        return TRUE;

    if ( (mpImplFont->meWeight          == rFont.mpImplFont->meWeight           ) &&
         (mpImplFont->meItalic          == rFont.mpImplFont->meItalic           ) &&
         (mpImplFont->meFamily          == rFont.mpImplFont->meFamily           ) &&
         (mpImplFont->mePitch           == rFont.mpImplFont->mePitch            ) &&
         (mpImplFont->meCharSet         == rFont.mpImplFont->meCharSet          ) &&
         (mpImplFont->meAlign           == rFont.mpImplFont->meAlign            ) &&
         (mpImplFont->maName            == rFont.mpImplFont->maName             ) &&
         (mpImplFont->maStyleName       == rFont.mpImplFont->maStyleName        ) &&
         (mpImplFont->maColor           == rFont.mpImplFont->maColor            ) &&
         (mpImplFont->maFillColor       == rFont.mpImplFont->maFillColor        ) &&
         (mpImplFont->maSize            == rFont.mpImplFont->maSize             ) &&
         (mpImplFont->mnOrientation     == rFont.mpImplFont->mnOrientation      ) &&
         (mpImplFont->meUnderline       == rFont.mpImplFont->meUnderline        ) &&
         (mpImplFont->meStrikeout       == rFont.mpImplFont->meStrikeout        ) &&
         (mpImplFont->meEmphasisMark    == rFont.mpImplFont->meEmphasisMark     ) &&
         (mpImplFont->mbWordLine        == rFont.mpImplFont->mbWordLine         ) &&
         (mpImplFont->mbOutline         == rFont.mpImplFont->mbOutline          ) &&
         (mpImplFont->mbShadow          == rFont.mpImplFont->mbShadow           ) &&
         (mpImplFont->mbKerning         == rFont.mpImplFont->mbKerning          ) &&
         (mpImplFont->mbTransparent     == rFont.mpImplFont->mbTransparent      ) )
        return TRUE;
    else
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
}

// -----------------------------------------------------------------------

SvStream& operator>>( SvStream& rIStm, Impl_Font& rImpl_Font )
{
    VersionCompat   aCompat( rIStm, STREAM_READ );
    UINT16          nTmp16;
    BOOL            bTmp;

    rIStm.ReadByteString( rImpl_Font.maName, rIStm.GetStreamCharSet() );
    rIStm.ReadByteString( rImpl_Font.maStyleName, rIStm.GetStreamCharSet() );
    rIStm >> rImpl_Font.maSize;
//  rIStm >> rImpl_Font.maColor;                                    // removed since SUPD396
//  rIStm >> rImpl_Font.maFillColor;                                // removed since SUPD396

    rIStm >> nTmp16; rImpl_Font.meCharSet = (rtl_TextEncoding) nTmp16;
    rIStm >> nTmp16; rImpl_Font.meFamily = (FontFamily) nTmp16;
    rIStm >> nTmp16; rImpl_Font.mePitch = (FontPitch) nTmp16;
//  rIStm >> nTmp16; rImpl_Font.meAlign = (FontAlign) nTmp16;       // removed since SUPD396
    rIStm >> nTmp16; rImpl_Font.meWeight = (FontWeight) nTmp16;
    rIStm >> nTmp16; rImpl_Font.meUnderline = (FontUnderline) nTmp16;
    rIStm >> nTmp16; rImpl_Font.meStrikeout = (FontStrikeout) nTmp16;
    rIStm >> nTmp16; rImpl_Font.meItalic = (FontItalic) nTmp16;
    rIStm >> nTmp16; rImpl_Font.meLanguage = (LanguageType) nTmp16; // new since SUPD 396
    rIStm >> nTmp16; rImpl_Font.meWidthType = (FontWidth) nTmp16;   // new since SUPD 396

    rIStm >> rImpl_Font.mnOrientation;

    rIStm >> bTmp; rImpl_Font.mbWordLine = bTmp;
    rIStm >> bTmp; rImpl_Font.mbOutline = bTmp;
    rIStm >> bTmp; rImpl_Font.mbShadow = bTmp;
    rIStm >> bTmp; rImpl_Font.mbKerning = bTmp;
//  rIStm >> bTmp; rImpl_Font.mbTransparent = bTmp;                 // removed since SUPD396

    return rIStm;
}

// -----------------------------------------------------------------------

SvStream& operator<<( SvStream& rOStm, const Impl_Font& rImpl_Font )
{
    VersionCompat aCompat( rOStm, STREAM_WRITE, 1 );
    rOStm.WriteByteString( rImpl_Font.maName, rOStm.GetStreamCharSet() );
    rOStm.WriteByteString( rImpl_Font.maStyleName, rOStm.GetStreamCharSet() );
    rOStm << rImpl_Font.maSize;
//  rOStm << rImpl_Font.maColor;                // removed since SUPD396
//  rOStm << rImpl_Font.maFillColor;            // removed since SUPD396

    rOStm << (UINT16) GetStoreCharSet( rImpl_Font.meCharSet, rOStm.GetVersion() );
    rOStm << (UINT16) rImpl_Font.meFamily;
    rOStm << (UINT16) rImpl_Font.mePitch;
//  rOStm << (UINT16) rImpl_Font.meAlign;       // removed since SUPD396
    rOStm << (UINT16) rImpl_Font.meWeight;
    rOStm << (UINT16) rImpl_Font.meUnderline;
    rOStm << (UINT16) rImpl_Font.meStrikeout;
    rOStm << (UINT16) rImpl_Font.meItalic;
    rOStm << (UINT16) rImpl_Font.meLanguage;    // new since SUPD 396
    rOStm << (UINT16) rImpl_Font.meWidthType;   // new since SUPD 396

    rOStm << rImpl_Font.mnOrientation;

    rOStm << (BOOL) rImpl_Font.mbWordLine;
    rOStm << (BOOL) rImpl_Font.mbOutline;
    rOStm << (BOOL) rImpl_Font.mbShadow;
    rOStm << (BOOL) rImpl_Font.mbKerning;
//  rOStm << (BOOL) rImpl_Font.mbTransparent;   // removed since SUPD396

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
