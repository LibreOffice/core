/*************************************************************************
 *
 *  $RCSfile: fontbuff.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:11 $
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

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------------

#include "scitems.hxx"
#include <svx/cntritem.hxx>
#include <svx/crsditem.hxx>
#include <svx/eeitem.hxx>
#include <svx/postitem.hxx>
#include <svx/shdditem.hxx>
#include <svx/escpitem.hxx>
#include <svx/udlnitem.hxx>
#include <svx/wghtitem.hxx>
#include <sfx2/printer.hxx>
#include <vcl/system.hxx>

#include "attrib.hxx"
#include "document.hxx"
#include "global.hxx"
#include "docpool.hxx"
#include "patattr.hxx"

#include "flttools.hxx"
#include "fontbuff.hxx"
#include "lotfntbf.hxx"


const UINT16 ColorBuffer::nAnzDef = 56;


static const ColBuffEntry   pDefArrayBiff5[] = {
    { 0x00, 0x00, 0x00 },
    { 0xff, 0xff, 0xff },
    { 0xff, 0x00, 0x00 },
    { 0x00, 0xff, 0x00 },
    { 0x00, 0x00, 0xff },
    { 0xff, 0xff, 0x00 },
    { 0xff, 0x00, 0xff },
    { 0x00, 0xff, 0xff },
    { 0x80, 0x00, 0x00 },
    { 0x00, 0x80, 0x00 },
    { 0x00, 0x00, 0x80 },
    { 0x80, 0x80, 0x00 },
    { 0x80, 0x00, 0x80 },
    { 0x00, 0x80, 0x80 },
    { 0xc0, 0xc0, 0xc0 },
    { 0x80, 0x80, 0x80 },
    { 0x80, 0x80, 0xff },
    { 0x80, 0x20, 0x60 },
    { 0xff, 0xff, 0xc0 },
    { 0xa0, 0xe0, 0xe0 },
    { 0x60, 0x00, 0x80 },
    { 0xff, 0x80, 0x80 },
    { 0x00, 0x80, 0xc0 },
    { 0xc0, 0xc0, 0xff },
    { 0x00, 0x00, 0x80 },
    { 0xff, 0x00, 0xff },
    { 0xff, 0xff, 0x00 },
    { 0x00, 0xff, 0xff },
    { 0x80, 0x00, 0x80 },
    { 0x80, 0x00, 0x00 },
    { 0x00, 0x80, 0x80 },
    { 0x00, 0x00, 0xff },
    { 0x00, 0xcf, 0xff },
    { 0x69, 0xff, 0xff },
    { 0xe0, 0xff, 0xe0 },
    { 0xff, 0xff, 0x80 },
    { 0xa6, 0xca, 0xf0 },
    { 0xdd, 0x9c, 0xb3 },
    { 0xb3, 0x8f, 0xee },
    { 0xe3, 0xe3, 0xe3 },
    { 0x2a, 0x6f, 0xf9 },
    { 0x3f, 0xb8, 0xcd },
    { 0x48, 0x84, 0x36 },
    { 0x95, 0x8c, 0x41 },
    { 0x8e, 0x5e, 0x42 },
    { 0xa0, 0x62, 0x7a },
    { 0x62, 0x4f, 0xac },
    { 0x96, 0x96, 0x96 },
    { 0x1d, 0x2f, 0xbe },
    { 0x28, 0x66, 0x76 },
    { 0x00, 0x45, 0x00 },
    { 0x45, 0x3e, 0x01 },
    { 0x6a, 0x28, 0x13 },
    { 0x85, 0x39, 0x6a },
    { 0x4a, 0x32, 0x85 },
    { 0x42, 0x42, 0x42 }
    };


static const ColBuffEntry   pDefArrayBiff8[] = {
    { 0x00, 0x00, 0x00 },
    { 0xFF, 0xFF, 0xFF },
    { 0xFF, 0x00, 0x00 },
    { 0x00, 0xFF, 0x00 },
    { 0x00, 0x00, 0xFF },
    { 0xFF, 0xFF, 0x00 },
    { 0xFF, 0x00, 0xFF },
    { 0x00, 0xFF, 0xFF },
    { 0x80, 0x00, 0x00 },
    { 0x00, 0x80, 0x00 },
    { 0x00, 0x00, 0x80 },
    { 0x80, 0x80, 0x00 },
    { 0x80, 0x00, 0x80 },
    { 0x00, 0x80, 0x80 },
    { 0xC0, 0xC0, 0xC0 },
    { 0x80, 0x80, 0x80 },
    { 0x99, 0x99, 0xFF },
    { 0x99, 0x33, 0x66 },
    { 0xFF, 0xFF, 0xCC },
    { 0xCC, 0xFF, 0xFF },
    { 0x66, 0x00, 0x66 },
    { 0xFF, 0x80, 0x80 },
    { 0x00, 0x66, 0xCC },
    { 0xCC, 0xCC, 0xFF },
    { 0x00, 0x00, 0x80 },
    { 0xFF, 0x00, 0xFF },
    { 0xFF, 0xFF, 0x00 },
    { 0x00, 0xFF, 0xFF },
    { 0x80, 0x00, 0x80 },
    { 0x80, 0x00, 0x00 },
    { 0x00, 0x80, 0x80 },
    { 0x00, 0x00, 0xFF },
    { 0x00, 0xCC, 0xFF },
    { 0xCC, 0xFF, 0xFF },
    { 0xCC, 0xFF, 0xCC },
    { 0xFF, 0xFF, 0x99 },
    { 0x99, 0xCC, 0xFF },
    { 0xFF, 0x99, 0xCC },
    { 0xCC, 0x99, 0xFF },
    { 0xFF, 0xCC, 0x99 },
    { 0x33, 0x66, 0xFF },
    { 0x33, 0xCC, 0xCC },
    { 0x99, 0xCC, 0x00 },
    { 0xFF, 0xCC, 0x00 },
    { 0xFF, 0x99, 0x00 },
    { 0xFF, 0x66, 0x00 },
    { 0x66, 0x66, 0x99 },
    { 0x96, 0x96, 0x96 },
    { 0x00, 0x33, 0x66 },
    { 0x33, 0x99, 0x66 },
    { 0x00, 0x33, 0x00 },
    { 0x33, 0x33, 0x00 },
    { 0x99, 0x33, 0x00 },
    { 0x99, 0x33, 0x66 },
    { 0x33, 0x33, 0x99 },
    { 0x33, 0x33, 0x33 }
    };


#ifndef VCL
const UINT16    ColorBuffer::nColCorrect = 257;
#endif

const UINT16    ColorBuffer::nIndCorrect = 8;

const UINT16    LotusFontBuffer::nSize = 8;



ColorBuffer::ColorBuffer( RootData* pRD  ) : ExcRoot( pRD )
{
    pDefArray = NULL;

    UINT16      nSize = 64 - nIndCorrect;

    pArray = new SvxColorItem *[ nSize ];
    for( UINT16 nC = 0 ; nC < nSize ; nC++ )
        pArray[ nC ] = NULL;

    nCount = 0;
    nMax = nSize;
    bAuto = FALSE;

    pDefault = new SvxColorItem( Color( COL_BLACK ) );
}


ColorBuffer::~ColorBuffer()
{
    Reset();

    delete[] pArray;

    delete pDefault;
}


void ColorBuffer::Reset()
{
    for( UINT16 nC = 0 ; nC < nCount ; nC++ )
    {
        if( pArray[ nC ] )
        {
            delete pArray[ nC ];
            pArray[ nC ] = NULL;
        }
    }
    nCount = 0;
}


BOOL ColorBuffer::NewColor( UINT16 nR, UINT16 nG, UINT16 nB )
{
    if( nCount < nMax )
    {
#ifdef VCL
        pArray[ nCount ] = new SvxColorItem( Color( ( UINT8 ) nR, ( UINT8 ) nG, ( UINT8 ) nB) );
#else
        pArray[ nCount ] = new SvxColorItem( Color(
            nR * nColCorrect,
            nG * nColCorrect,
            nB * nColCorrect ) );
#endif
        nCount++;
        return TRUE;
    }

    return FALSE;
}


const SvxColorItem* ColorBuffer::GetColor( UINT16 nIndex, const BOOL bOptGetDefault )
{
    if( pExcRoot->eHauptDateiTyp != Biff5 && pExcRoot->eHauptDateiTyp != Biff8 && nIndex >= 24 )
    {
        // Auto in < Excel 5
        bAuto = TRUE;
        return bOptGetDefault? pDefault : NULL;
    }

    if( nIndex >= nIndCorrect )
        nIndex -= nIndCorrect;

    if( nIndex < nMax )
    {
        bAuto = FALSE;
        if( pArray[ nIndex ] )
            return pArray[ nIndex ];    // schon Generiert
        if( nIndex >= nCount )
        {                           // ausserhalb der Definierten
            if( nIndex < nAnzDef )
            {                           // ... aber innerhalb der Defaults
                pArray[ nIndex ] = new SvxColorItem( Color(
                    pDefArray[ nIndex ].nR,                 // R
                    pDefArray[ nIndex ].nG,                 // G
                    pDefArray[ nIndex ].nB ) );             // B
            }
            else
                // ... und ausserhalb der Defaults
                return bOptGetDefault? pDefault : NULL;
        }
        return pArray[ nIndex ];
    }
    else
    {                               // ueber Array-Kapazitaet bzw. Auto
        bAuto = TRUE;
        return bOptGetDefault? pDefault : NULL;
    }
}


void ColorBuffer::SetDefaults( void )
{
    if( pExcRoot->eHauptDateiTyp == Biff8 )
        pDefArray = pDefArrayBiff8;
    else
        pDefArray = pDefArrayBiff5;
}




const UINT16 FontBuffer::nDefaultMasterHeight = 200;


FontBuffer::FontBuffer( RootData* pRD ) : ExcRoot( pRD )
{
    aDefaultFont.bItalic = aDefaultFont.bStrikeout = aDefaultFont.bOutline = aDefaultFont.bShadow = FALSE;
    aDefaultFont.eUnderline = ExcUndNone;
    aDefaultFont.nWeight = 400; // NORMAL
    aDefaultFont.nColor = 0xFFFF;
}


FontBuffer::~FontBuffer()
{
    register UINT32         nCnt = List::Count();
    register ScExcFont*     pDel = ( ScExcFont* ) List::First();

    // NULL pointers may exist => not while(pDel)
    while( nCnt-- )
    {
        delete pDel;
        pDel = ( ScExcFont* ) List::Next();
    }
}


void FontBuffer::NewFont( UINT16 nHeight, BYTE nAttr0, UINT16 nIndexCol, const String& rName )
{
    // fuer Biff2-4

    BYTE    nUnderline;
    UINT16  nBoldness;

    if( nAttr0 & 0x04 )
        // underline
        nUnderline = ( BYTE ) ExcUndSingle;
    else
        nUnderline = ( BYTE ) ExcUndNone;

    if( nAttr0 & 0x01 )
        // bold
        nBoldness = 700;        // BOLD
    else
        nBoldness = 400;        // NORMAL

    NewFont( nHeight, nAttr0, nUnderline, nIndexCol, nBoldness, 0x00, 0x01, rName );
        // -> nFamily = DONTKNOW, nCharSet = DONTKNOW
}


void FontBuffer::NewFont( UINT16 nHeight, BYTE nAttr0, BYTE nUnderline,
    UINT16 nIndexCol, UINT16 nBoldness, BYTE nFamily, BYTE nCharSet, const String& rName )
{
    NewFont( nHeight, nAttr0, 0x0000, nUnderline, nIndexCol, nBoldness, nFamily, nCharSet, rName );
}


void FontBuffer::NewFont( UINT16 nHeight, BYTE nAttr0, UINT16 nScript, BYTE nUnderline,
    UINT16 nIndexCol, UINT16 nBoldness, BYTE nFamily, BYTE nCharSet, const String& rName )
{
    // fuer Biff5
    if( Count() == 4 )
        // 4 darf nicht vorkommen
        Insert( NULL, LIST_APPEND );

    // Umwandlung Windows-Font in SV-Font
    CharSet     eCharSet;
    switch( nCharSet )
    {
        case 0:                         // ANSI_CHARSET
            eCharSet = RTL_TEXTENCODING_MS_1252;
            break;
        case 255:                       // OEM_CHARSET
            eCharSet = RTL_TEXTENCODING_IBM_850;
            break;
        case 2:                         // SYMBOL_CHARSET
            eCharSet = RTL_TEXTENCODING_SYMBOL;
            break;
        default:
            eCharSet = GetSystemCharSet();
    }

    FontFamily  eFamily;
    // !ACHTUNG!: anders als in Windows-Doku scheint der Font im unteren
    // Nible des Bytes zu stehen -> Pitch unbekannt!
    switch( nFamily & 0x0F )    // ...eben nicht!
    {
        case 0x01:                      // FF_ROMAN
            eFamily = FAMILY_ROMAN;
            break;
        case 0x02:                      // FF_SWISS
            eFamily = FAMILY_SWISS;
            break;
        case 0x03:                      // FF_MODERN
            eFamily = FAMILY_MODERN;
            break;
        case 0x04:                      // FF_SCRIPT
            eFamily = FAMILY_SCRIPT;
            break;
        case 0x05:                      // FF_DECORATIVE
            eFamily = FAMILY_DECORATIVE;
            break;
        default:
            if( *pExcRoot->pCharset == RTL_TEXTENCODING_APPLE_ROMAN &&
                    ( rName.EqualsAscii( "Geneva" ) || rName.EqualsAscii( "Chicago" ) ) )
                eFamily = FAMILY_SWISS;
            else
                eFamily = FAMILY_DONTKNOW;
    }

    SvxFontItem*    pSvxFont = new SvxFontItem( eFamily, rName, EMPTY_STRING, PITCH_DONTKNOW, eCharSet );

    DBG_ASSERT( nHeight <  32767, "+FontList::NewFont(): Height >= 32767 - Pech..." );

    ScExcFont*      pScExcFont = new ScExcFont( *pSvxFont, *( new SvxFontHeightItem( ( ULONG ) nHeight ) ) );

    pScExcFont->nColor = nIndexCol;
    pScExcFont->nScript = ( UINT8 ) nScript;
    pScExcFont->eUnderline = ( ExcUnderlineType ) nUnderline;
    pScExcFont->nWeight = nBoldness;
    pScExcFont->bItalic = nAttr0 & 0x02;    // italic
    pScExcFont->bStrikeout = nAttr0 &0x08;  // strikeout
    pScExcFont->bOutline = nAttr0 &0x10;    // outline
    pScExcFont->bShadow = nAttr0 &0x20;     // shadow

    Insert( pScExcFont, LIST_APPEND );

    if( Count() == 1 )
    {
        double              f;
        switch( nHeight )
        {
            case 240:   f = 1.21860;        break;      //  12
            case 320:   f = 1.61040;        break;      //  16
            default:
            {
                SfxPrinter*     p = pExcRoot->pDoc->GetPrinter();

                if( p )
                {
                    const UINT16    nArial10Width = 111;

                    Font        aF( rName, Size( 0, nHeight ) );

                    aF.SetFamily( eFamily );
                    aF.SetCharSet( eCharSet );

                    p->SetFont( aF );

                    f = ( double ) p->GetTextWidth( _STRINGCONST( "0" ) ) / ( double ) nArial10Width;
                }
                else
                    f = ( double ) nHeight / ( double ) nDefaultMasterHeight;
            }
        }
        pExcRoot->fColScale *= f;
    }
}


void FontBuffer::Fill( const UINT16 nIndex, SfxItemSet& rItemSet, const BOOL bOwn )
{
    ScExcFont*      pFont = ( ScExcFont* ) GetObject( nIndex );

    if( !pFont )
        pFont = &aDefaultFont;

    SvxWeightItem   aWeightItem( GetWeight( pFont->nWeight ) );

    if( pFont->bItalic )
    {// italic
        SvxPostureItem aAttr( ITALIC_NORMAL );
        if( bOwn )
            rItemSet.Put( aAttr );
        else
            rItemSet.Put( aAttr, EE_CHAR_ITALIC );
    }


    if( !bOwn && pFont->nScript )
    {
        SvxEscapement   eEsc;

        switch( pFont->nScript )
        {
            case 0x01:  eEsc = SVX_ESCAPEMENT_SUPERSCRIPT;  break;
            case 0x02:  eEsc = SVX_ESCAPEMENT_SUBSCRIPT;    break;
            case 0x00:
            default:
                        eEsc = SVX_ESCAPEMENT_OFF;
        }

        rItemSet.Put( SvxEscapementItem( eEsc, EE_CHAR_ESCAPEMENT ) );
    }


    UINT16          nUnderline;

    switch( pFont->eUnderline )
    {
        case ExcUndSingle:
        case ExcUndSingleAcc:   nUnderline = UNDERLINE_SINGLE; break;
        case ExcUndDouble:
        case ExcUndDoubleAcc:   nUnderline = UNDERLINE_DOUBLE; break;
        default:                nUnderline = UNDERLINE_NONE;
    }

    SvxUnderlineItem    aUndItem( ( FontUnderline ) nUnderline );


    if( pFont->bStrikeout )
    {// strikeout
        SvxCrossedOutItem aAttr( STRIKEOUT_SINGLE );
        if( bOwn )
            rItemSet.Put( aAttr );
        else
            rItemSet.Put( aAttr, EE_CHAR_STRIKEOUT );
    }

    if( pFont->bOutline )
    {// outline
        SvxContourItem  aAttr( TRUE );
        if( bOwn )
            rItemSet.Put( aAttr );
        else
            rItemSet.Put( aAttr, EE_CHAR_OUTLINE );
    }

    if( pFont->bShadow )
    {// shadow
        SvxShadowedItem aAttr( TRUE );
        if( bOwn )
            rItemSet.Put( aAttr );
        else
            rItemSet.Put( aAttr, EE_CHAR_SHADOW );
    }

    if( bOwn )
    {
        rItemSet.Put( pFont->GetFontItem() );
        rItemSet.Put( pFont->GetHeightItem() );
        rItemSet.Put( *pExcRoot->pColor->GetColor( pFont->nColor ) );
        rItemSet.Put( aWeightItem );
        rItemSet.Put( aUndItem );
    }
    else
    {
        SvxFontItem     aFontItem = pFont->GetFontItem();

        if( aFontItem.GetCharSet() == *pExcRoot->pCharset )
            aFontItem.GetCharSet() = GetSystemCharSet();

        rItemSet.Put( aFontItem, EE_CHAR_FONTINFO );

        SvxFontHeightItem&  rHeightItem = pFont->GetHeightItem();
        long nOldHeight = rHeightItem.GetHeight();
        long nNewHeight = ( long ) ( HMM_PER_TWIPS * nOldHeight );

        rHeightItem.SetHeightValue( nNewHeight );

        rItemSet.Put( rHeightItem, EE_CHAR_FONTHEIGHT );

        rHeightItem.SetHeightValue( nOldHeight );

        rItemSet.Put( *pExcRoot->pColor->GetColor( pFont->nColor ), EE_CHAR_COLOR );
        rItemSet.Put( aWeightItem, EE_CHAR_WEIGHT );
        rItemSet.Put( aUndItem, EE_CHAR_UNDERLINE );
    }
}


void FontBuffer::Reset( void )
{
    register ScExcFont* pDel = ( ScExcFont* ) List::First();

    while( pDel )
    {
        if( pDel )
            delete pDel;
        pDel = ( ScExcFont* ) List::Next();
    }
    Clear();
}


BOOL FontBuffer::HasSuperOrSubscript( const UINT16 n ) const
{
    const ScExcFont*    p = ( const ScExcFont* ) GetObject( n );

    if( p )
        return p->nScript != 0x00;
    else
        return FALSE;
}


SvxWeightItem FontBuffer::GetWeight( const UINT16 nWeight )
{
    SvxWeightItem   aWeightItem;

    if( !nWeight )
        aWeightItem = WEIGHT_DONTKNOW;
    else if( nWeight < 150 )
        aWeightItem = WEIGHT_THIN;
    else if( nWeight < 250 )
        aWeightItem = WEIGHT_ULTRALIGHT;
    else if( nWeight < 325 )
        aWeightItem = WEIGHT_LIGHT;
    else if( nWeight < 375 )
        aWeightItem = WEIGHT_SEMILIGHT;
    else if( nWeight < 450 )
        aWeightItem = WEIGHT_NORMAL;
    else if( nWeight < 550 )
        aWeightItem = WEIGHT_MEDIUM;
    else if( nWeight < 650 )
        aWeightItem = WEIGHT_SEMIBOLD;
    else if( nWeight < 750 )
        aWeightItem = WEIGHT_BOLD;
    else if( nWeight < 850 )
        aWeightItem = WEIGHT_ULTRABOLD;
    else
        aWeightItem = WEIGHT_BLACK;

    return aWeightItem;
}




void LotusFontBuffer::Fill( const UINT8 nIndex, SfxItemSet& rItemSet )
{
    UINT8   nIntIndex = nIndex & 0x07;

    ENTRY*  pAkt = pData + nIntIndex;

    if( pAkt->pFont )
        rItemSet.Put( *pAkt->pFont );

    if( pAkt->pHeight )
        rItemSet.Put( *pAkt->pHeight );

    if( pAkt->pColor )
        rItemSet.Put( *pAkt->pColor );

    if( nIndex & 0x08 )
    {
        SvxWeightItem aWeightItem( WEIGHT_BOLD );
        rItemSet.Put( aWeightItem );
    }

    if( nIndex & 0x10 )
    {
        SvxPostureItem aAttr( ITALIC_NORMAL );
        rItemSet.Put( aAttr );
    }

    FontUnderline eUnderline;
    switch( nIndex & 0x60 ) // Bit 5+6
    {
        case 0x60:
        case 0x20:  eUnderline = UNDERLINE_SINGLE;      break;
        case 0x40:  eUnderline = UNDERLINE_DOUBLE;      break;
        default:    eUnderline = UNDERLINE_NONE;
    }
    if( eUnderline != UNDERLINE_NONE )
    {
        SvxUnderlineItem aUndItem( eUnderline );
        rItemSet.Put( aUndItem );
    }
}


void LotusFontBuffer::SetName( const UINT16 nIndex, const String& rName )
{
    DBG_ASSERT( nIndex < nSize, "*LotusFontBuffer::SetName(): Array zu klein!" );
    if( nIndex < nSize )
    {
        register ENTRY* pEntry = pData + nIndex;
        pEntry->TmpName( rName );

        if( pEntry->nType >= 0 )
            MakeFont( pEntry );
    }
}


void LotusFontBuffer::SetHeight( const UINT16 nIndex, const UINT16 nHeight )
{
    DBG_ASSERT( nIndex < nSize, "*LotusFontBuffer::SetHeight(): Array zu klein!" );
    if( nIndex < nSize )
        pData[ nIndex ].Height( *( new SvxFontHeightItem( ( ULONG ) nHeight * 20 ) ) );
}


void LotusFontBuffer::SetType( const UINT16 nIndex, const UINT16 nType )
{
    DBG_ASSERT( nIndex < nSize, "*LotusFontBuffer::SetType(): Array zu klein!" );
    if( nIndex < nSize )
    {
        register ENTRY* pEntry = pData + nIndex;
        pEntry->Type( nType );

        if( pEntry->pTmpName )
            MakeFont( pEntry );
    }
}


void LotusFontBuffer::MakeFont( ENTRY* pEntry )
{
    FontFamily      eFamily = FAMILY_DONTKNOW;
    FontPitch       ePitch = PITCH_DONTKNOW;
    CharSet         eCharSet = RTL_TEXTENCODING_DONTKNOW;

    switch( pEntry->nType )
    {
        case 0x00:                      // Helvetica
            eFamily = FAMILY_SWISS;
            ePitch  = PITCH_VARIABLE;
            break;
        case 0x01:                      // Times Roman
            eFamily = FAMILY_ROMAN;
            ePitch  = PITCH_VARIABLE;
            break;
        case 0x02:                      // Courier
            ePitch  = PITCH_FIXED;
            break;
        case 0x03:                      // Symbol
            eCharSet = RTL_TEXTENCODING_SYMBOL;
            break;
    }

    pEntry->pFont = new SvxFontItem( eFamily, *pEntry->pTmpName, EMPTY_STRING, ePitch, eCharSet );

    delete pEntry->pTmpName;
    pEntry->pTmpName = NULL;
}



