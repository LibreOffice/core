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

#include <memory>
#include <lotattr.hxx>

#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/justifyitem.hxx>
#include <sal/log.hxx>

#include <docpool.hxx>
#include <document.hxx>
#include <lotfntbf.hxx>
#include <patattr.hxx>
#include <root.hxx>
#include <scitems.hxx>

using namespace ::com::sun::star;

LotAttrCache::ENTRY::ENTRY (std::unique_ptr<ScPatternAttr> p)
    : pPattAttr(std::move(p))
    , nHash0(0)
{
}

LotAttrCache::ENTRY::~ENTRY ()
{
}

LotAttrCache::LotAttrCache (LOTUS_ROOT* pLotRoot)
    : mpLotusRoot(pLotRoot)
{
    pDocPool = mpLotusRoot->pDoc->GetPool();

    pColTab.reset( new Color [ 8 ] );
    pColTab[ 0 ] = COL_WHITE;
    pColTab[ 1 ] = COL_LIGHTBLUE;
    pColTab[ 2 ] = COL_LIGHTGREEN;
    pColTab[ 3 ] = COL_LIGHTCYAN;
    pColTab[ 4 ] = COL_LIGHTRED;
    pColTab[ 5 ] = COL_LIGHTMAGENTA;
    pColTab[ 6 ] = COL_YELLOW;
    pColTab[ 7 ] = COL_BLACK;

    ppColorItems[ 0 ].reset( new SvxColorItem( GetColor( 1 ), ATTR_FONT_COLOR ) );     // 1
    ppColorItems[ 1 ].reset( new SvxColorItem( GetColor( 2 ), ATTR_FONT_COLOR ) );
    ppColorItems[ 2 ].reset( new SvxColorItem( GetColor( 3 ), ATTR_FONT_COLOR ) );
    ppColorItems[ 3 ].reset( new SvxColorItem( GetColor( 4 ), ATTR_FONT_COLOR ) );
    ppColorItems[ 4 ].reset( new SvxColorItem( GetColor( 5 ), ATTR_FONT_COLOR ) );
    ppColorItems[ 5 ].reset( new SvxColorItem( GetColor( 6 ), ATTR_FONT_COLOR ) );     // 6

    pWhite.reset( new SvxColorItem( COL_WHITE, ATTR_FONT_COLOR ) );
}

LotAttrCache::~LotAttrCache()
{
}

const ScPatternAttr& LotAttrCache::GetPattAttr( const LotAttrWK3& rAttr )
{
    sal_uInt32  nRefHash;
    MakeHash( rAttr, nRefHash );

    std::vector< std::unique_ptr<ENTRY> >::const_iterator iter
        = std::find_if(aEntries.begin(),aEntries.end(),
                       [nRefHash] (const std::unique_ptr<ENTRY>& rEntry) { return rEntry->nHash0 == nRefHash; } );

    if (iter != aEntries.end())
        return *((*iter)->pPattAttr);

    // generate new Pattern Attribute
    ScPatternAttr*  pNewPatt = new ScPatternAttr(pDocPool);

    SfxItemSet&     rItemSet = pNewPatt->GetItemSet();
    ENTRY *pCurrent = new ENTRY( std::unique_ptr<ScPatternAttr>(pNewPatt) );

    pCurrent->nHash0 = nRefHash;

    mpLotusRoot->maFontBuff.Fill( rAttr.nFont, rItemSet );

    sal_uInt8 nLine = rAttr.nLineStyle;
    if( nLine )
    {
        SvxBoxItem      aBox( ATTR_BORDER );
        ::editeng::SvxBorderLine    aTop, aLeft, aBottom, aRight;

        LotusToScBorderLine( nLine, aLeft );
        nLine >>= 2;
        LotusToScBorderLine( nLine, aRight );
        nLine >>= 2;
        LotusToScBorderLine( nLine, aTop );
        nLine >>= 2;
        LotusToScBorderLine( nLine, aBottom );

        aBox.SetLine( &aTop, SvxBoxItemLine::TOP );
        aBox.SetLine( &aLeft, SvxBoxItemLine::LEFT );
        aBox.SetLine( &aBottom, SvxBoxItemLine::BOTTOM );
        aBox.SetLine( &aRight, SvxBoxItemLine::RIGHT );

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
        SvxHorJustifyItem   aHorJustify(SvxCellHorJustify::Center, ATTR_HOR_JUSTIFY );
        rItemSet.Put( aHorJustify );
    }

    aEntries.push_back(std::unique_ptr<ENTRY>(pCurrent));

    return *pNewPatt;
}

void LotAttrCache::LotusToScBorderLine( sal_uInt8 nLine, ::editeng::SvxBorderLine& aBL )
{
    nLine &= 0x03;

    switch ( nLine )
    {
        case 0: aBL.SetBorderLineStyle(SvxBorderLineStyle::NONE); break;
        case 1: aBL.SetWidth( DEF_LINE_WIDTH_1 ); break;
        case 2: aBL.SetWidth( DEF_LINE_WIDTH_2 ); break;
        case 3:
        {
            aBL.SetBorderLineStyle(SvxBorderLineStyle::DOUBLE_THIN);
            aBL.SetWidth( DEF_LINE_WIDTH_1 );
        }
        break;
    }
}

const SvxColorItem& LotAttrCache::GetColorItem( const sal_uInt8 nLotIndex ) const
{
    // *LotAttrCache::GetColorItem(): caller has to check index!
    assert( nLotIndex > 0 && nLotIndex < 7 );

    return *ppColorItems[ nLotIndex - 1 ];
}

const Color& LotAttrCache::GetColor( const sal_uInt8 nLotIndex ) const
{
    // color <-> index fits background, but not for fonts (0 <-> 7)!
    // *LotAttrCache::GetColor(): Index > 7, caller hast to check index!"
    assert( nLotIndex < 8 );

    return pColTab[ nLotIndex ];
}

void LotAttrCol::SetAttr( const SCROW nRow, const ScPatternAttr& rAttr )
{
    // Actually with the current implementation of MAXROWCOUNT>=64k and nRow
    // being read as sal_uInt16 there's no chance that nRow would be invalid..
    SAL_WARN_IF( !ValidRow(nRow), "sc.filter", "*LotAttrCol::SetAttr(): ... and failed?!" );

    std::vector<std::unique_ptr<ENTRY> >::reverse_iterator iterLast = aEntries.rbegin();

    if(iterLast != aEntries.rend())
    {
        if( ( (*iterLast)->nLastRow == nRow - 1 ) && ( &rAttr == (*iterLast)->pPattAttr ) )
            (*iterLast)->nLastRow = nRow;
        else
        {
            ENTRY *pCurrent = new ENTRY;

            pCurrent->pPattAttr = &rAttr;
            pCurrent->nFirstRow = pCurrent->nLastRow = nRow;

            aEntries.push_back(std::unique_ptr<ENTRY>(pCurrent));
        }
    }
    else
    {   // first entry
        ENTRY *pCurrent = new ENTRY;
        pCurrent->pPattAttr = &rAttr;
        pCurrent->nFirstRow = pCurrent->nLastRow = nRow;

        aEntries.push_back(std::unique_ptr<ENTRY>(pCurrent));
    }
}

void LotAttrCol::Apply(LOTUS_ROOT* pLotusRoot, const SCCOL nColNum, const SCTAB nTabNum)
{
    ScDocument*     pDoc = pLotusRoot->pDoc;

    for (const auto& rxEntry : aEntries)
    {
        pDoc->ApplyPatternAreaTab(nColNum, rxEntry->nFirstRow, nColNum, rxEntry->nLastRow,
                                  nTabNum, *(rxEntry->pPattAttr));
    }
}

LotAttrTable::LotAttrTable(LOTUS_ROOT* pLotRoot):
    aAttrCache(pLotRoot)
{
}

void LotAttrTable::SetAttr( const SCCOL nColFirst, const SCCOL nColLast, const SCROW nRow,
                            const LotAttrWK3& rAttr )
{
    // With the current implementation of MAXCOLCOUNT>=1024 and nColFirst and
    // nColLast being calculated as sal_uInt8+sal_uInt8 there's no chance that
    // they would be invalid.
    const ScPatternAttr &rPattAttr = aAttrCache.GetPattAttr( rAttr );
    SCCOL nColCnt;

    for( nColCnt = nColFirst ; nColCnt <= nColLast ; nColCnt++ )
        pCols[ nColCnt ].SetAttr( nRow, rPattAttr );
}

void LotAttrTable::Apply(LOTUS_ROOT* pLotusRoot, const SCTAB nTabNum)
{
    SCCOL nColCnt;
    for( nColCnt = 0 ; nColCnt <= MAXCOL ; nColCnt++ )
        pCols[ nColCnt ].Apply(pLotusRoot, nColCnt, nTabNum);     // does a Clear() at end
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
