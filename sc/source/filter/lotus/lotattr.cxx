/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include "lotattr.hxx"

#include <boost/bind.hpp>

#include <editeng/boxitem.hxx>
#include <editeng/brshitem.hxx>
#include <editeng/justifyitem.hxx>
#include <svx/algitem.hxx>

#include "attrib.hxx"
#include "docpool.hxx"
#include "document.hxx"
#include "lotfntbf.hxx"
#include "patattr.hxx"
#include "root.hxx"
#include "scitems.hxx"


using namespace ::com::sun::star;


LotAttrCache::ENTRY::ENTRY (ScPatternAttr* p)
    : pPattAttr(p)
{
}

LotAttrCache::ENTRY::~ENTRY ()
{
    delete pPattAttr;
}

LotAttrCache::LotAttrCache (LOTUS_ROOT* pLotRoot):
    mpLotusRoot(pLotRoot)
{
    pDocPool = mpLotusRoot->pDoc->GetPool();

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
    for( sal_uInt16 nCnt = 0 ; nCnt < 6 ; nCnt++ )
        delete ppColorItems[ nCnt ];

    delete pBlack;
    delete pWhite;

    delete[] pColTab;
}


const ScPatternAttr& LotAttrCache::GetPattAttr( const LotAttrWK3& rAttr )
{
    sal_uInt32  nRefHash;
    MakeHash( rAttr, nRefHash );

    boost::ptr_vector<ENTRY>::const_iterator iter = std::find_if(aEntries.begin(),aEntries.end(),
                                                                 boost::bind(&ENTRY::nHash0,_1) == nRefHash);

    if (iter != aEntries.end())
        return *(iter->pPattAttr);

    // neues PatternAttribute erzeugen
    ScPatternAttr*  pNewPatt = new ScPatternAttr(pDocPool);

    SfxItemSet&     rItemSet = pNewPatt->GetItemSet();
    ENTRY *pAkt = new ENTRY( pNewPatt );

    pAkt->nHash0 = nRefHash;

    mpLotusRoot->pFontBuff->Fill( rAttr.nFont, rItemSet );

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

    aEntries.push_back(pAkt);

    return *pNewPatt;
}


void LotAttrCache::LotusToScBorderLine( sal_uInt8 nLine, ::editeng::SvxBorderLine& aBL )
{
    nLine &= 0x03;

    switch ( nLine )
    {
        default:
        case 0: aBL.SetBorderLineStyle(table::BorderLineStyle::NONE); break;
        case 1: aBL.SetWidth( DEF_LINE_WIDTH_1 ); break;
        case 2: aBL.SetWidth( DEF_LINE_WIDTH_2 ); break;
        case 3:
        {
            aBL.SetBorderLineStyle(table::BorderLineStyle::DOUBLE);
            aBL.SetWidth( DEF_LINE_WIDTH_1 );
        }
        break;
    }
}

const SvxColorItem& LotAttrCache::GetColorItem( const sal_uInt8 nLotIndex ) const
{
    OSL_ENSURE( nLotIndex > 0 && nLotIndex < 7,
		"-LotAttrCache::GetColorItem(): caller hast to check index!" );

    return *ppColorItems[ nLotIndex - 1 ];
}

const Color& LotAttrCache::GetColor( const sal_uInt8 nLotIndex ) const
{
    // Farbe <-> Index passt fuer Background, nicht aber fuer Fonts (0 <-> 7)!
	OSL_ENSURE( nLotIndex < 8, "*LotAttrCache::GetColor(): Index > 7, caller hast to check index!" );

    return pColTab[ nLotIndex ];
}

void LotAttrCol::SetAttr( const SCROW nRow, const ScPatternAttr& rAttr )
{
    // Actually with the current implementation of MAXROWCOUNT>=64k and nRow
    // being read as sal_uInt16 there's no chance that nRow would be invalid..
    OSL_ENSURE( ValidRow(nRow), "*LotAttrCol::SetAttr(): ... und rums?!" );

    boost::ptr_vector<ENTRY>::reverse_iterator iterLast = aEntries.rbegin();

    if(iterLast != aEntries.rend())
    {
        if( ( iterLast->nLastRow == nRow - 1 ) && ( &rAttr == iterLast->pPattAttr ) )
            iterLast->nLastRow = nRow;
        else
        {
            ENTRY *pAkt = new ENTRY;

            pAkt->pPattAttr = &rAttr;
            pAkt->nFirstRow = pAkt->nLastRow = nRow;

            aEntries.push_back(pAkt);
        }
    }
    else
    {   // erster Eintrag
        ENTRY *pAkt = new ENTRY;
        pAkt->pPattAttr = &rAttr;
        pAkt->nFirstRow = pAkt->nLastRow = nRow;

        aEntries.push_back(pAkt);
    }
}


void LotAttrCol::Apply( const SCCOL nColNum, const SCTAB nTabNum )
{
    ScDocument*     pDoc = pLotusRoot->pDoc;

    boost::ptr_vector<ENTRY>::iterator iter;
    for (iter = aEntries.begin(); iter != aEntries.end(); ++iter)
    {
        pDoc->ApplyPatternAreaTab(nColNum,iter->nFirstRow,nColNum,iter->nLastRow,
                                  nTabNum, *(iter->pPattAttr));
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


void LotAttrTable::Apply( const SCTAB nTabNum )
{
    SCCOL nColCnt;
    for( nColCnt = 0 ; nColCnt <= MAXCOL ; nColCnt++ )
        pCols[ nColCnt ].Apply( nColCnt, nTabNum );     // macht auch gleich ein Clear() am Ende
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
