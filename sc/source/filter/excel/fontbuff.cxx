/*************************************************************************
 *
 *  $RCSfile: fontbuff.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 18:04:33 $
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

#include "lotfntbf.hxx"
#include "flttools.hxx"

#ifndef SC_FTOOLS_HXX
#include "ftools.hxx"
#endif

const UINT16    LotusFontBuffer::nSize = 8;

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



