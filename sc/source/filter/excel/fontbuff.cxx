/*************************************************************************
 *
 *  $RCSfile: fontbuff.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: dr $ $Date: 2002-04-04 12:59:00 $
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

#include "fontbuff.hxx"
#include "lotfntbf.hxx"
#include "flttools.hxx"

#ifndef _SC_FILTERTOOLS_HXX
#include "FilterTools.hxx"
#endif

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
    pAutoColor = new SvxColorItem( Color( COL_AUTO ) );
}


ColorBuffer::~ColorBuffer()
{
    Reset();

    delete[] pArray;

    delete pDefault;
    delete pAutoColor;
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
        pArray[ nCount ] = new SvxColorItem( Color( ( UINT8 ) nR, ( UINT8 ) nG, ( UINT8 ) nB) );
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


const SvxColorItem* ColorBuffer::GetFontColor( sal_uInt16 nIndex )
{
    const SvxColorItem* pItem = GetColor( nIndex, sal_False );
    return pItem ? pItem : pAutoColor;
}


void ColorBuffer::SetDefaults( void )
{
    if( pExcRoot->eHauptDateiTyp == Biff8 )
        pDefArray = pDefArrayBiff8;
    else
        pDefArray = pDefArrayBiff5;
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



