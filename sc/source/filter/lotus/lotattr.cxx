/*************************************************************************
 *
 *  $RCSfile: lotattr.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:14 $
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
#include <svx/algitem.hxx>
#include <svx/boxitem.hxx>
#include <svx/brshitem.hxx>

#include "document.hxx"
#include "patattr.hxx"
#include "docpool.hxx"
#include "attrib.hxx"

#include "lotattr.hxx"
#include "lotfntbf.hxx"
#include "root.hxx"



void LotAttrCache::LotusToScBorderLine( UINT8 nLine, SvxBorderLine& aBL )
{
    static const UINT16 pPara[ 4 ][ 3 ] =
    {
        { 0,0,0 },
        { DEF_LINE_WIDTH_1, 0, 0 },
        { DEF_LINE_WIDTH_2, 0, 0 },
        { DEF_LINE_WIDTH_1, DEF_LINE_WIDTH_1, DEF_LINE_WIDTH_1 }
    };

    nLine &= 0x03;

    if( nLine )
    {
        aBL.SetOutWidth( pPara[ nLine ][ 0 ] );
        aBL.SetInWidth( pPara[ nLine ][ 1 ] );
        aBL.SetDistance( pPara[ nLine ][ 2 ] );
    }
}


const SvxColorItem& LotAttrCache::GetColorItem( const UINT8 nLotIndex ) const
{
    DBG_ASSERT( nLotIndex > 0 && nLotIndex < 7,
        "-LotAttrCache::GetColorItem(): so nicht!" );

    return *ppColorItems[ nLotIndex - 1 ];
}


const Color& LotAttrCache::GetColor( const UINT8 nLotIndex ) const
{
    // Farbe <-> Index passt fuer Background, nicht aber fuer Fonts (0 <-> 7)!
    DBG_ASSERT( nLotIndex < 8, "*LotAttrCache::GetColor(): Index > 7!" );
    return pColTab[ nLotIndex ];
}


LotAttrCache::LotAttrCache( void )
{
    pDocPool = pLotusRoot->pDoc->GetPool();

    pColTab = new Color [ 8 ];
    pColTab[ 0 ] = Color( COL_WHITE );
    pColTab[ 1 ] = Color( COL_LIGHTBLUE );
    pColTab[ 2 ] = Color( COL_LIGHTGREEN );
    pColTab[ 3 ] = Color( COL_LIGHTCYAN );
    pColTab[ 4 ] = Color( COL_LIGHTRED );
    pColTab[ 5 ] = Color( COL_LIGHTMAGENTA );
    pColTab[ 6 ] = Color( COL_YELLOW );
    pColTab[ 7 ] = Color( COL_BLACK );

    ppColorItems[ 0 ] = new SvxColorItem( GetColor( 1 ) );      // 1
    ppColorItems[ 1 ] = new SvxColorItem( GetColor( 2 ) );
    ppColorItems[ 2 ] = new SvxColorItem( GetColor( 3 ) );
    ppColorItems[ 3 ] = new SvxColorItem( GetColor( 4 ) );
    ppColorItems[ 4 ] = new SvxColorItem( GetColor( 5 ) );
    ppColorItems[ 5 ] = new SvxColorItem( GetColor( 6 ) );      // 6

    pBlack = new SvxColorItem( Color( COL_BLACK ) );
    pWhite = new SvxColorItem( Color( COL_WHITE ) );
}


LotAttrCache::~LotAttrCache()
{
    ENTRY*  pAkt = ( ENTRY* ) List::First();

    while( pAkt )
    {
        delete pAkt;
        pAkt = ( ENTRY* ) List::Next();
    }

    for( UINT16 nCnt = 0 ; nCnt < 6 ; nCnt++ )
        delete ppColorItems[ nCnt ];

    delete pBlack;
    delete pWhite;

    delete[] pColTab;
}


const ScPatternAttr& LotAttrCache::GetPattAttr( const LotAttrWK3& rAttr )
{
    UINT32  nRefHash;
    ENTRY*  pAkt = ( ENTRY* ) List::First();

    MakeHash( rAttr, nRefHash );

    while( pAkt )
    {
        if( *pAkt == nRefHash )
            return *pAkt->pPattAttr;

        pAkt = ( ENTRY* ) List::Next();
    }

    // neues PatternAttribute erzeugen
    ScPatternAttr*  pNewPatt = new ScPatternAttr( pDocPool );
    SfxItemSet&     rItemSet = pNewPatt->GetItemSet();
    pAkt = new ENTRY( pNewPatt );

    pAkt->nHash0 = nRefHash;

    pLotusRoot->pFontBuff->Fill( rAttr.nFont, rItemSet );

    UINT8 nLine = rAttr.nLineStyle;
    if( nLine )
    {
        SvxBoxItem      aBox( ATTR_BORDER );
        SvxBorderLine   aTop, aLeft, aBottom, aRight;

        LotusToScBorderLine( nLine, aLeft );
        nLine >>= 2;
        LotusToScBorderLine( nLine, aRight );
        nLine >>= 2;
        LotusToScBorderLine( nLine, aTop );
        nLine >>= 2;
        LotusToScBorderLine( nLine, aBottom );

        aBox.SetLine( &aTop, BOX_LINE_TOP );
        aBox.SetLine( &aLeft, BOX_LINE_LEFT );
        aBox.SetLine( &aBottom, BOX_LINE_BOTTOM );
        aBox.SetLine( &aRight, BOX_LINE_RIGHT );

        rItemSet.Put( aBox );
    }

    UINT8               nFontCol = rAttr.nFontCol & 0x07;
    if( nFontCol )
    {
        // nFontCol > 0
        if( nFontCol < 7 )
            rItemSet.Put( GetColorItem( nFontCol ) );
        else
            rItemSet.Put( *pWhite );
    }

    UINT8 nBack = rAttr.nBack & 0x1F;
    if( nBack )
        rItemSet.Put( SvxBrushItem( GetColor( nBack & 0x07 ) ) );

    if( rAttr.nBack & 0x80 )
    {
        SvxHorJustifyItem   aHorJustify;
        aHorJustify.SetValue( SVX_HOR_JUSTIFY_CENTER );
        rItemSet.Put( aHorJustify );
    }

    List::Insert( pAkt, LIST_APPEND );

    return *pNewPatt;
    }


LotAttrCol::~LotAttrCol()
{
    Clear();
}


void LotAttrCol::SetAttr( const UINT16 nRow, const ScPatternAttr& rAttr )
{
    DBG_ASSERT( nRow <= MAXROW, "*LotAttrCol::SetAttr(): ... und rums?!" );

    ENTRY*      pAkt = ( ENTRY* ) List::Last();

    if( pAkt )
    {
        if( ( pAkt->nLastRow == nRow - 1 ) && ( &rAttr == pAkt->pPattAttr ) )
            pAkt->nLastRow = nRow;
        else
        {
            pAkt = new ENTRY;

            pAkt->pPattAttr = &rAttr;
            pAkt->nFirstRow = pAkt->nLastRow = nRow;
            List::Insert( pAkt, LIST_APPEND );
        }
    }
    else
    {   // erster Eintrag
        pAkt = new ENTRY;
        pAkt->pPattAttr = &rAttr;
        pAkt->nFirstRow = pAkt->nLastRow = nRow;
        List::Insert( pAkt, LIST_APPEND );
    }
}


void LotAttrCol::Apply( const UINT16 nColNum, const UINT16 nTabNum, const BOOL bClear )
{
    ScDocument*     pDoc = pLotusRoot->pDoc;
    ENTRY*          pAkt = ( ENTRY* ) List::First();

    while( pAkt )
    {
        pDoc->ApplyPatternAreaTab( nColNum, pAkt->nFirstRow, nColNum, pAkt->nLastRow,
            nTabNum, *pAkt->pPattAttr );

        pAkt = ( ENTRY* ) List::Next();
    }
}


void LotAttrCol::Clear( void )
{
    ENTRY*          pAkt = ( ENTRY* ) List::First();

    while( pAkt )
    {
        delete pAkt;
        pAkt = ( ENTRY* ) List::Next();
    }
}


LotAttrTable::LotAttrTable( void )
{
}


LotAttrTable::~LotAttrTable()
{
}


void LotAttrTable::SetAttr( const UINT8 nColFirst, const UINT8 nColLast, const UINT16 nRow,
                            const LotAttrWK3& rAttr )
{
    const ScPatternAttr&    rPattAttr = aAttrCache.GetPattAttr( rAttr );
    UINT16                  nColCnt;

    for( nColCnt = nColFirst ; nColCnt <= nColLast ; nColCnt++ )
        pCols[ nColCnt ].SetAttr( nRow, rPattAttr );
}


void LotAttrTable::Apply( const UINT16 nTabNum )
{
    UINT16                  nColCnt;
    for( nColCnt = 0 ; nColCnt <= MAXCOL ; nColCnt++ )
        pCols[ nColCnt ].Apply( nColCnt, nTabNum );     // macht auch gleich ein Clear() am Ende
}




