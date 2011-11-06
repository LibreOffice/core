/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



//------------------------------------------------------------------------

#include "scitems.hxx"
#include <svx/algitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brshitem.hxx>

#include "document.hxx"
#include "patattr.hxx"
#include "docpool.hxx"
#include "attrib.hxx"

#include "lotattr.hxx"
#include "lotfntbf.hxx"
#include "root.hxx"



void LotAttrCache::LotusToScBorderLine( sal_uInt8 nLine, SvxBorderLine& aBL )
{
    static const sal_uInt16 pPara[ 4 ][ 3 ] =
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


const SvxColorItem& LotAttrCache::GetColorItem( const sal_uInt8 nLotIndex ) const
{
    DBG_ASSERT( nLotIndex > 0 && nLotIndex < 7,
        "-LotAttrCache::GetColorItem(): so nicht!" );

    return *ppColorItems[ nLotIndex - 1 ];
}


const Color& LotAttrCache::GetColor( const sal_uInt8 nLotIndex ) const
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

    ppColorItems[ 0 ] = new SvxColorItem( GetColor( 1 ), ATTR_FONT_COLOR );     // 1
    ppColorItems[ 1 ] = new SvxColorItem( GetColor( 2 ), ATTR_FONT_COLOR );
    ppColorItems[ 2 ] = new SvxColorItem( GetColor( 3 ), ATTR_FONT_COLOR );
    ppColorItems[ 3 ] = new SvxColorItem( GetColor( 4 ), ATTR_FONT_COLOR );
    ppColorItems[ 4 ] = new SvxColorItem( GetColor( 5 ), ATTR_FONT_COLOR );
    ppColorItems[ 5 ] = new SvxColorItem( GetColor( 6 ), ATTR_FONT_COLOR );     // 6

    pBlack = new SvxColorItem( Color( COL_BLACK ), ATTR_FONT_COLOR );
    pWhite = new SvxColorItem( Color( COL_WHITE ), ATTR_FONT_COLOR );
}


LotAttrCache::~LotAttrCache()
{
    ENTRY*  pAkt = ( ENTRY* ) List::First();

    while( pAkt )
    {
        delete pAkt;
        pAkt = ( ENTRY* ) List::Next();
    }

    for( sal_uInt16 nCnt = 0 ; nCnt < 6 ; nCnt++ )
        delete ppColorItems[ nCnt ];

    delete pBlack;
    delete pWhite;

    delete[] pColTab;
}


const ScPatternAttr& LotAttrCache::GetPattAttr( const LotAttrWK3& rAttr )
{
    sal_uInt32  nRefHash;
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

    sal_uInt8 nLine = rAttr.nLineStyle;
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

    sal_uInt8               nFontCol = rAttr.nFontCol & 0x07;
    if( nFontCol )
    {
        // nFontCol > 0
        if( nFontCol < 7 )
            rItemSet.Put( GetColorItem( nFontCol ) );
        else
            rItemSet.Put( *pWhite );
    }

    sal_uInt8 nBack = rAttr.nBack & 0x1F;
    if( nBack )
        rItemSet.Put( SvxBrushItem( GetColor( nBack & 0x07 ), ATTR_BACKGROUND ) );

    if( rAttr.nBack & 0x80 )
    {
        SvxHorJustifyItem   aHorJustify(SVX_HOR_JUSTIFY_CENTER, ATTR_HOR_JUSTIFY );
        rItemSet.Put( aHorJustify );
    }

    List::Insert( pAkt, LIST_APPEND );

    return *pNewPatt;
    }


LotAttrCol::~LotAttrCol()
{
    Clear();
}


void LotAttrCol::SetAttr( const SCROW nRow, const ScPatternAttr& rAttr )
{
    DBG_ASSERT( ValidRow(nRow), "*LotAttrCol::SetAttr(): ... und rums?!" );

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


void LotAttrCol::Apply( const SCCOL nColNum, const SCTAB nTabNum, const sal_Bool /*bClear*/ )
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


void LotAttrTable::SetAttr( const SCCOL nColFirst, const SCCOL nColLast, const SCROW nRow,
                            const LotAttrWK3& rAttr )
{
    const ScPatternAttr&    rPattAttr = aAttrCache.GetPattAttr( rAttr );
    SCCOL                   nColCnt;

    for( nColCnt = nColFirst ; nColCnt <= nColLast ; nColCnt++ )
        pCols[ nColCnt ].SetAttr( nRow, rPattAttr );
}


void LotAttrTable::Apply( const SCTAB nTabNum )
{
    SCCOL                   nColCnt;
    for( nColCnt = 0 ; nColCnt <= MAXCOL ; nColCnt++ )
        pCols[ nColCnt ].Apply( nColCnt, nTabNum );     // macht auch gleich ein Clear() am Ende
}




