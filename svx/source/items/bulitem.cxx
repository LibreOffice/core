/*************************************************************************
 *
 *  $RCSfile: bulitem.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:20 $
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

// include ---------------------------------------------------------------

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _SV_SYSTEM_HXX
#include <vcl/system.hxx>
#endif
#pragma hdrstop

#define _SVX_BULITEM_CXX

#include "bulitem.hxx"
#include "svxerr.hxx"

#define BULITEM_VERSION     ((USHORT)2)

// -----------------------------------------------------------------------

TYPEINIT1(SvxBulletItem,SfxPoolItem);

// -----------------------------------------------------------------------

void SvxBulletItem::StoreFont( SvStream& rStream, const Font& rFont )
{
    USHORT nTemp;

    rStream << rFont.GetColor();
    nTemp = (USHORT)rFont.GetFamily(); rStream << nTemp;
    nTemp = (USHORT)GetStoreCharSet( rFont.GetCharSet(),
                                       rStream.GetVersion() );
                                        rStream << nTemp;
    nTemp = (USHORT)rFont.GetPitch(); rStream << nTemp;
    nTemp = (USHORT)rFont.GetAlign(); rStream << nTemp;
    nTemp = (USHORT)rFont.GetWeight(); rStream << nTemp;
    nTemp = (USHORT)rFont.GetUnderline(); rStream << nTemp;
    nTemp = (USHORT)rFont.GetStrikeout(); rStream << nTemp;
    nTemp = (USHORT)rFont.GetItalic(); rStream << nTemp;

    // UNICODE: rStream << rFont.GetName();
    rStream.WriteByteString(rFont.GetName());

    rStream << rFont.IsOutline();
    rStream << rFont.IsShadow();
    rStream << rFont.IsTransparent();
}

// -----------------------------------------------------------------------

Font SvxBulletItem::CreateFont( SvStream& rStream, USHORT nVer )
{
    Font aFont;
    Color aColor;
    rStream >> aColor;    aFont.SetColor( aColor );
    USHORT nTemp;
    rStream >> nTemp; aFont.SetFamily((FontFamily)nTemp);
    rStream >> nTemp; aFont.SetCharSet((rtl_TextEncoding)nTemp);
    rStream >> nTemp; aFont.SetPitch((FontPitch)nTemp);
    rStream >> nTemp; aFont.SetAlign((FontAlign)nTemp);
    rStream >> nTemp; aFont.SetWeight((FontWeight)nTemp);
    rStream >> nTemp; aFont.SetUnderline((FontUnderline)nTemp);
    rStream >> nTemp; aFont.SetStrikeout((FontStrikeout)nTemp);
    rStream >> nTemp; aFont.SetItalic((FontItalic)nTemp);

    // UNICODE: rStream >> aName; aFont.SetName( aName );
    String aName;
    rStream.ReadByteString(aName);
    aFont.SetName( aName );

    if( nVer == 1 )
    {
        long nHeight, nWidth;
        rStream >> nHeight; rStream >> nWidth; Size aSize( nWidth, nHeight );
        aFont.SetSize( aSize );
    }

    BOOL bTemp;
    rStream >> bTemp; aFont.SetOutline( bTemp );
    rStream >> bTemp; aFont.SetShadow( bTemp );
    rStream >> bTemp; aFont.SetTransparent( bTemp );
    return aFont;
}


// -----------------------------------------------------------------------

SvxBulletItem::SvxBulletItem( USHORT nWhich ) : SfxPoolItem( nWhich )
{
    SetDefaultFont_Impl();
    SetDefaults_Impl();
    nValidMask = 0xFFFF;
}

// -----------------------------------------------------------------------

SvxBulletItem::SvxBulletItem( BYTE nNewStyle, const Font& rFont, USHORT nStart,
    USHORT nWhich ) : SfxPoolItem( nWhich )
{
    SetDefaults_Impl();
    nStyle = nNewStyle;
    aFont  = rFont;
    nValidMask = 0xFFFF;
}

// -----------------------------------------------------------------------

SvxBulletItem::SvxBulletItem( const Font& rFont, xub_Unicode cSymb,
    USHORT nWhich ) : SfxPoolItem( nWhich )
{
    SetDefaults_Impl();
    aFont   = rFont;
    cSymbol = cSymb;
    nStyle  = BS_BULLET;
    nValidMask = 0xFFFF;
}

// -----------------------------------------------------------------------

SvxBulletItem::SvxBulletItem( const Bitmap& rBmp, USHORT nWhich ) :
    SfxPoolItem( nWhich ),
    aGraphicObject( rBmp )
{
    SetDefaults_Impl();
    nStyle = BS_BMP;
    nValidMask = 0xFFFF;
}

// -----------------------------------------------------------------------

SvxBulletItem::SvxBulletItem( const GraphicObject& rGraphicObject, USHORT nWhich ) :
    SfxPoolItem( nWhich ),
    aGraphicObject( rGraphicObject )
{
    SetDefaults_Impl();
    nStyle = BS_BMP;
    nValidMask = 0xFFFF;
}

// -----------------------------------------------------------------------

SvxBulletItem::SvxBulletItem( SvStream& rStrm, USHORT nWhich )
 : SfxPoolItem( nWhich )
{
    rStrm >> nStyle;
    if( nStyle != BS_BMP )
        aFont = CreateFont( rStrm, BULITEM_VERSION );
    else
    {
        // Sicheres Laden mit Test auf leere Bitmap
        Bitmap          aBmp;
        const UINT32    nOldPos = rStrm.Tell();
        // #69345# Errorcode beim Bitmap lesen ignorieren,
        // siehe Kommentar #67581# in SvxBulletItem::Store()
        BOOL bOldError = rStrm.GetError() ? TRUE : FALSE;
        rStrm >> aBmp;
        if ( !bOldError && rStrm.GetError() )
        {
            rStrm.ResetError();
            // #71493# Keine Warnung: Das BulletItem interessiert seit 5.0 im Dateiformat nicht mehr.
            // rStrm.SetError(ERRCODE_CLASS_READ | ERRCODE_SVX_BULLETITEM_NOBULLET | ERRCODE_WARNING_MASK);
        }

        if(!aBmp)
        {
            // Korrektur bei leerer Bitmap
            rStrm.Seek( nOldPos );
            nStyle = BS_NONE;
        }
        else
            aGraphicObject.SetGraphic( aBmp );
    }
    rStrm >> nWidth;
    rStrm >> nStart;
    rStrm >> nJustify;

    char cTmpSymbol;
    rStrm >> cTmpSymbol;
    cSymbol = ByteString::ConvertToUnicode( cTmpSymbol, aFont.GetCharSet() );

    rStrm >> nScale;

    // UNICODE: rStrm >> aPrevText;
    rStrm.ReadByteString(aPrevText);

    // UNICODE: rStrm >> aFollowText;
    rStrm.ReadByteString(aFollowText);

    nValidMask = 0xFFFF;
}

// -----------------------------------------------------------------------

SvxBulletItem::SvxBulletItem( const SvxBulletItem& rItem)
    : SfxPoolItem( rItem )
{
    aFont           = rItem.aFont;
    aGraphicObject  = rItem.aGraphicObject;
    aPrevText       = rItem.aPrevText;
    aFollowText     = rItem.aFollowText;
    nStart          = rItem.nStart;
    nStyle          = rItem.nStyle;
    nWidth          = rItem.nWidth;
    nScale          = rItem.nScale;
    cSymbol         = rItem.cSymbol;
    nJustify        = rItem.nJustify;
    nValidMask      = rItem.nValidMask;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxBulletItem::Clone( SfxItemPool *pPool ) const
{
    return new SvxBulletItem( *this );
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxBulletItem::Create( SvStream& rStrm, USHORT nVersion ) const
{
    return new SvxBulletItem( rStrm, Which() );
}

// -----------------------------------------------------------------------

void SvxBulletItem::SetDefaultFont_Impl()
{
    aFont = System::GetStandardFont( STDFONT_ROMAN );
    aFont.SetAlign( ALIGN_BOTTOM);
    aFont.SetTransparent( TRUE );
}

// -----------------------------------------------------------------------

void SvxBulletItem::SetDefaults_Impl()
{
    nWidth      = 1200;  // 1.2cm
    nStart      = 1;
    nStyle      = BS_123;
    nJustify    = BJ_HLEFT | BJ_VCENTER;
    cSymbol     = sal_Unicode(' ');
    nScale      = 75;
}

// -----------------------------------------------------------------------

USHORT SvxBulletItem::GetVersion( USHORT nVersion ) const
{
    return BULITEM_VERSION;
}


void SvxBulletItem::CopyValidProperties( const SvxBulletItem& rCopyFrom )
{
    Font aFont = GetFont();
    Font aNewFont = rCopyFrom.GetFont();
    if ( rCopyFrom.IsValid( VALID_FONTNAME ) )
    {
        aFont.SetName( aNewFont.GetName() );
        aFont.SetFamily( aNewFont.GetFamily() );
        aFont.SetStyleName( aNewFont.GetStyleName() );
    }
    if ( rCopyFrom.IsValid( VALID_FONTCOLOR ) )
        aFont.SetColor( aNewFont.GetColor() );
    if ( rCopyFrom.IsValid( VALID_SYMBOL ) )
        SetSymbol( rCopyFrom.GetSymbol() );
    if ( rCopyFrom.IsValid( VALID_BITMAP ) )
        SetGraphicObject( rCopyFrom.GetGraphicObject() );
    if ( rCopyFrom.IsValid( VALID_SCALE ) )
        SetScale( rCopyFrom.GetScale() );
    if ( rCopyFrom.IsValid( VALID_START ) )
        SetStart( rCopyFrom.GetStart() );
    if ( rCopyFrom.IsValid( VALID_STYLE ) )
        SetStyle( rCopyFrom.GetStyle() );
    if ( rCopyFrom.IsValid( VALID_PREVTEXT ) )
        SetPrevText( rCopyFrom.GetPrevText() );
    if ( rCopyFrom.IsValid( VALID_FOLLOWTEXT ) )
        SetFollowText( rCopyFrom.GetFollowText() );

    SetFont( aFont );
}


// -----------------------------------------------------------------------

int SvxBulletItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT(rItem.ISA(SvxBulletItem),"operator==Types not matching");
    const SvxBulletItem& rBullet = (const SvxBulletItem&)rItem;
    // ValidMask mitvergleichen, da sonst kein Putten in ein AttrSet moeglich,
    // wenn sich das Item nur in der ValidMask von einem existierenden unterscheidet.
    if( nValidMask != rBullet.nValidMask    ||
        nStyle != rBullet.nStyle            ||
        nScale != rBullet.nScale            ||
        nJustify != rBullet.nJustify        ||
        nWidth != rBullet.nWidth            ||
        nStart != rBullet.nStart            ||
        cSymbol != rBullet.cSymbol          ||
        aPrevText != rBullet.aPrevText      ||
        aFollowText != rBullet.aFollowText )
            return 0;

    if( ( nStyle != BS_BMP ) && ( aFont != rBullet.aFont ) )
        return 0;
    if( ( nStyle == BS_BMP ) && ( aGraphicObject != rBullet.aGraphicObject ) )
        return 0;
    if( ( nStyle == BS_BMP ) && ( aGraphicObject.GetPrefSize() != rBullet.aGraphicObject.GetPrefSize() ) )
        return 0;

    return 1;
}

// -----------------------------------------------------------------------

SvStream& SvxBulletItem::Store( SvStream& rStrm, USHORT nItemVersion ) const
{
    // Korrektur bei leerer Bitmap
    if((nStyle == BS_BMP) && (aGraphicObject.GetType() == GRAPHIC_NONE ))
    {
        ((SvxBulletItem*)this)->nStyle = BS_NONE;
    }

    rStrm << nStyle;
    if( nStyle != BS_BMP )
        StoreFont( rStrm, aFont );
    else
    {
        ULONG nStart = rStrm.Tell();

        // Kleine Vorab-Schaetzung der Groesse...
        USHORT nFac = ( rStrm.GetCompressMode() != COMPRESSMODE_NONE ) ? 3 : 1;
        const Bitmap aBmp( aGraphicObject.GetGraphic().GetBitmap() );
        ULONG nBytes = aBmp.GetSizeBytes();
        if ( nBytes < (0xFF00*nFac) )
            rStrm << aBmp;

        ULONG nEnd = rStrm.Tell();
        // #67581# Item darf mit Overhead nicht mehr als 64K schreiben,
        // sonst platzt der SfxMultiRecord
        // Dann lieber auf die Bitmap verzichten, ist nur fuer Outliner
        // und auch nur fuer <= 5.0 wichtig.
        // Beim Einlesen merkt der Stream-Operator der Bitmap, dass dort keine steht.
        // Hiermit funktioniert jetzt der Fall das die grosse Bitmap aus einem anderen
        // Fileformat entstanden ist, welches keine 64K belegt, aber wenn eine
        // Bitmap > 64K verwendet wird, hat das SvxNumBulletItem beim Laden ein Problem,
        // stuerzt aber nicht ab.

        if ( (nEnd-nStart) > 0xFF00 )
            rStrm.Seek( nStart );
    }
    rStrm << nWidth;
    rStrm << nStart;
    rStrm << nJustify;
    rStrm << (char)ByteString::ConvertFromUnicode( cSymbol, aFont.GetCharSet() );
    rStrm << nScale;

    // UNICODE: rStrm << aPrevText;
    rStrm.WriteByteString(aPrevText);

    // UNICODE: rStrm << aFollowText;
    rStrm.WriteByteString(aFollowText);

    return rStrm;
}

//------------------------------------------------------------------------

XubString SvxBulletItem::GetFullText() const
{
    XubString aStr( aPrevText );
    aStr += cSymbol;
    aStr += aFollowText;
    return aStr;
}

//------------------------------------------------------------------------

SfxItemPresentation SvxBulletItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
    SfxItemPresentation eRet = SFX_ITEM_PRESENTATION_NONE;
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            eRet = SFX_ITEM_PRESENTATION_NONE;
            break;

        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = GetFullText();
            eRet = SFX_ITEM_PRESENTATION_COMPLETE;
            break;
    }
    return eRet;
}


