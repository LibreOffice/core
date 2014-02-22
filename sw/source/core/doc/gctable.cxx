/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <hintids.hxx>
#include <editeng/boxitem.hxx>
#include <tblrwcl.hxx>
#include <swtblfmt.hxx>
#include <algorithm>
#include <boost/foreach.hpp>

using namespace ::editeng;

inline const SvxBorderLine* GetLineTB( const SvxBoxItem* pBox, bool bTop )
{
    return bTop ? pBox->GetTop() : pBox->GetBottom();
}

bool _SwGCBorder_BoxBrd::CheckLeftBorderOfFormat( const SwFrmFmt& rFmt )
{
    const SvxBorderLine* pBrd;
    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET == rFmt.GetItemState( RES_BOX, sal_True, &pItem ) &&
        0 != ( pBrd = ((SvxBoxItem*)pItem)->GetLeft() ) )
    {
        if( *pBrdLn == *pBrd )
            bAnyBorderFnd = true;
        return true;
    }
    return false;
}

static bool lcl_GCBorder_ChkBoxBrd_B( const SwTableBox* pBox, _SwGCBorder_BoxBrd* pPara );

static bool lcl_GCBorder_ChkBoxBrd_L( const SwTableLine* pLine, _SwGCBorder_BoxBrd* pPara )
{
    const SwTableBox* pBox = pLine->GetTabBoxes().front();
    return lcl_GCBorder_ChkBoxBrd_B( pBox, pPara );
}

static bool lcl_GCBorder_ChkBoxBrd_B( const SwTableBox* pBox, _SwGCBorder_BoxBrd* pPara )
{
    bool bRet = true;
    if( !pBox->GetTabLines().empty() )
    {
        for( sal_uInt16 n = 0, nLines = pBox->GetTabLines().size();
                n < nLines && bRet; ++n )
        {
            const SwTableLine* pLine = pBox->GetTabLines()[ n ];
            bRet = lcl_GCBorder_ChkBoxBrd_L( pLine, pPara );
        }
    }
    else
    {
        bRet = pPara->CheckLeftBorderOfFormat( *pBox->GetFrmFmt() );
    }
    return bRet;
}

static void lcl_GCBorder_GetLastBox_B( const SwTableBox* pBox, SwTableBoxes* pPara );

static void lcl_GCBorder_GetLastBox_L( const SwTableLine* pLine, SwTableBoxes* pPara )
{
    const SwTableBoxes& rBoxes = pLine->GetTabBoxes();
    SwTableBox* pBox = rBoxes.back();
    lcl_GCBorder_GetLastBox_B( pBox, pPara );
}

static void lcl_GCBorder_GetLastBox_B( const SwTableBox* pBox, SwTableBoxes* pPara )
{
    const SwTableLines& rLines = pBox->GetTabLines();
    if( !rLines.empty() )
    {
        BOOST_FOREACH( const SwTableLine* pLine, rLines )
            lcl_GCBorder_GetLastBox_L( pLine, pPara );
    }
    else
        pPara->push_back( (SwTableBox*)pBox );
}


static sal_uInt16 lcl_FindEndPosOfBorder( const SwCollectTblLineBoxes& rCollTLB,
                        const SvxBorderLine& rBrdLn, sal_uInt16& rStt, bool bTop )
{
    sal_uInt16 nPos, nLastPos = 0;
    for( sal_uInt16 nEnd = rCollTLB.Count(); rStt < nEnd; ++rStt )
    {
        const SfxPoolItem* pItem;
        const SvxBorderLine* pBrd;
        const SwTableBox& rBox = rCollTLB.GetBox( rStt, &nPos );

        if( SFX_ITEM_SET != rBox.GetFrmFmt()->GetItemState(RES_BOX,sal_True, &pItem )
            || 0 == ( pBrd = GetLineTB( (SvxBoxItem*)pItem, bTop ))
            || !( *pBrd == rBrdLn ))
            break;
        nLastPos = nPos;
    }
    return nLastPos;
}

static inline const SvxBorderLine* lcl_GCBorder_GetBorder( const SwTableBox& rBox,
                                                bool bTop,
                                                const SfxPoolItem** ppItem )
{
    return SFX_ITEM_SET == rBox.GetFrmFmt()->GetItemState( RES_BOX, sal_True, ppItem )
            ? GetLineTB( (SvxBoxItem*)*ppItem, bTop )
            : 0;
}

static void lcl_GCBorder_DelBorder( const SwCollectTblLineBoxes& rCollTLB,
                                sal_uInt16& rStt, bool bTop,
                                const SvxBorderLine& rLine,
                                const SfxPoolItem* pItem,
                                sal_uInt16 nEndPos,
                                SwShareBoxFmts* pShareFmts )
{
    SwTableBox* pBox = (SwTableBox*)&rCollTLB.GetBox( rStt );
    sal_uInt16 nNextPos;
    const SvxBorderLine* pLn = &rLine;

    do {
        if( pLn && *pLn == rLine )
        {
            SvxBoxItem aBox( *(SvxBoxItem*)pItem );
            if( bTop )
                aBox.SetLine( 0, BOX_LINE_TOP );
            else
                aBox.SetLine( 0, BOX_LINE_BOTTOM );

            if( pShareFmts )
                pShareFmts->SetAttr( *pBox, aBox );
            else
                pBox->ClaimFrmFmt()->SetFmtAttr( aBox );
        }

        if( ++rStt >= rCollTLB.Count() )
            break;

        pBox = (SwTableBox*)&rCollTLB.GetBox( rStt, &nNextPos );
        if( nNextPos > nEndPos )
            break;

        pLn = lcl_GCBorder_GetBorder( *pBox, bTop, &pItem );

    } while( true );
}

static void lcl_GC_Box_Border( const SwTableBox* pBox, _SwGCLineBorder* pPara );

void sw_GC_Line_Border( const SwTableLine* pLine, _SwGCLineBorder* pGCPara )
{
    
    {
        _SwGCBorder_BoxBrd aBPara;
        const SvxBorderLine* pBrd;
        const SfxPoolItem* pItem;
        const SwTableBoxes& rBoxes = pLine->GetTabBoxes();
        for( sal_uInt16 n = 0, nBoxes = rBoxes.size() - 1; n < nBoxes; ++n )
        {
            SwTableBoxes aBoxes;
            {
                SwTableBox* pBox = rBoxes[ n ];
                if( pBox->GetSttNd() )
                    aBoxes.insert( aBoxes.begin(), pBox );
                else
                    lcl_GCBorder_GetLastBox_B( pBox, &aBoxes );
            }

            SwTableBox* pBox;
            for( sal_uInt16 i = aBoxes.size(); i; )
                if( SFX_ITEM_SET == (pBox = aBoxes[ --i ])->GetFrmFmt()->
                    GetItemState( RES_BOX, sal_True, &pItem ) &&
                    0 != ( pBrd = ((SvxBoxItem*)pItem)->GetRight() ) )
                {
                    aBPara.SetBorder( *pBrd );
                    const SwTableBox* pNextBox = rBoxes[n+1];
                    if( lcl_GCBorder_ChkBoxBrd_B( pNextBox, &aBPara ) &&
                        aBPara.IsAnyBorderFound() )
                    {
                        SvxBoxItem aBox( *(SvxBoxItem*)pItem );
                        aBox.SetLine( 0, BOX_LINE_RIGHT );
                        if( pGCPara->pShareFmts )
                            pGCPara->pShareFmts->SetAttr( *pBox, aBox );
                        else
                            pBox->ClaimFrmFmt()->SetFmtAttr( aBox );
                    }
                }

            aBoxes.clear();
        }
    }

    
    if( !pGCPara->IsLastLine() )
    {
        SwCollectTblLineBoxes aBottom( false );
        SwCollectTblLineBoxes aTop( true );

        sw_Line_CollectBox( pLine, &aBottom );

        const SwTableLine* pNextLine = (*pGCPara->pLines)[ pGCPara->nLinePos+1 ];
        sw_Line_CollectBox( pNextLine, &aTop );

        
        sal_uInt16 nBtmPos, nTopPos,
                nSttBtm = 0, nSttTop = 0,
                nEndBtm = aBottom.Count(), nEndTop = aTop.Count();

        const SwTableBox *pBtmBox = &aBottom.GetBox( nSttBtm++, &nBtmPos ),
                         *pTopBox = &aTop.GetBox( nSttTop++, &nTopPos );
        const SfxPoolItem *pBtmItem = 0, *pTopItem = 0;
        const SvxBorderLine *pBtmLine(0), *pTopLine(0);
        bool bGetTopItem = true, bGetBtmItem = true;

        do {
            if( bGetBtmItem )
                pBtmLine = lcl_GCBorder_GetBorder( *pBtmBox, false, &pBtmItem );
            if( bGetTopItem )
                pTopLine = lcl_GCBorder_GetBorder( *pTopBox, true, &pTopItem );

            if( pTopLine && pBtmLine && *pTopLine == *pBtmLine )
            {
                
                sal_uInt16 nSavSttBtm = nSttBtm, nSavSttTop = nSttTop;
                sal_uInt16 nBtmEndPos = ::lcl_FindEndPosOfBorder( aBottom,
                                                *pTopLine, nSttBtm, false );
                if( !nBtmEndPos ) nBtmEndPos = nBtmPos;
                sal_uInt16 nTopEndPos = ::lcl_FindEndPosOfBorder( aTop,
                                                *pTopLine, nSttTop, true );
                if( !nTopEndPos ) nTopEndPos = nTopPos;


                if( nTopEndPos <= nBtmEndPos )
                {
                    
                    nSttTop = nSavSttTop;
                    if( nTopPos <= nBtmEndPos )
                        lcl_GCBorder_DelBorder( aTop, --nSttTop, true,
                                            *pBtmLine, pTopItem, nBtmEndPos,
                                            pGCPara->pShareFmts );
                    else
                        nSttBtm = nSavSttBtm;
                }
                else
                {
                    
                    nSttBtm = nSavSttBtm;
                    if( nBtmPos <= nTopEndPos )
                        lcl_GCBorder_DelBorder( aBottom, --nSttBtm, false,
                                            *pTopLine, pBtmItem, nTopEndPos,
                                            pGCPara->pShareFmts );
                    else
                        nSttTop = nSavSttTop;
                }
                nTopPos = nBtmPos;
            }

            if( nTopPos == nBtmPos )
            {
                if( nSttBtm >= nEndBtm || nSttTop >= nEndTop )
                    break;

                pBtmBox = &aBottom.GetBox( nSttBtm++, &nBtmPos );
                pTopBox = &aTop.GetBox( nSttTop++, &nTopPos );
                bGetTopItem = bGetBtmItem = true;
            }
            else if( nTopPos < nBtmPos )
            {
                if( nSttTop >= nEndTop )
                    break;
                pTopBox = &aTop.GetBox( nSttTop++, &nTopPos );
                bGetTopItem = true;
                bGetBtmItem = false;
            }
            else
            {
                if( nSttBtm >= nEndBtm )
                    break;
                pBtmBox = &aBottom.GetBox( nSttBtm++, &nBtmPos );
                bGetTopItem = false;
                bGetBtmItem = true;
            }

        } while( true );
    }

    for( SwTableBoxes::const_iterator it = pLine->GetTabBoxes().begin();
             it != pLine->GetTabBoxes().end(); ++it)
        lcl_GC_Box_Border(*it, pGCPara );

    ++pGCPara->nLinePos;
}

static void lcl_GC_Box_Border( const SwTableBox* pBox, _SwGCLineBorder* pPara )
{
    if( !pBox->GetTabLines().empty() )
    {
        _SwGCLineBorder aPara( *pBox );
        aPara.pShareFmts = pPara->pShareFmts;
        BOOST_FOREACH( const SwTableLine* pLine, pBox->GetTabLines() )
            sw_GC_Line_Border( pLine, &aPara );
    }
}

struct _GCLinePara
{
    SwTableLines* pLns;
    SwShareBoxFmts* pShareFmts;

    _GCLinePara( SwTableLines& rLns, _GCLinePara* pPara = 0 )
        : pLns( &rLns ), pShareFmts( pPara ? pPara->pShareFmts : 0 )
    {}
};

static bool lcl_MergeGCLine(SwTableLine* pLine, _GCLinePara* pPara);

static bool lcl_MergeGCBox(SwTableBox* pTblBox, _GCLinePara* pPara)
{
    sal_uInt16 n, nLen = pTblBox->GetTabLines().size();
    if( nLen )
    {
        
        _GCLinePara aPara( pTblBox->GetTabLines(), pPara );
        for( n = 0; n < pTblBox->GetTabLines().size() &&
            lcl_MergeGCLine( pTblBox->GetTabLines()[n], &aPara );
            ++n )
            ;

        if( 1 == pTblBox->GetTabLines().size() )
        {
            
            SwTableLine* pInsLine = pTblBox->GetUpper();
            SwTableLine* pCpyLine = pTblBox->GetTabLines()[0];
            SwTableBoxes::iterator it = std::find( pInsLine->GetTabBoxes().begin(), pInsLine->GetTabBoxes().end(), pTblBox );
            for( n = 0; n < pCpyLine->GetTabBoxes().size(); ++n )
                pCpyLine->GetTabBoxes()[n]->SetUpper( pInsLine );

            
            it = pInsLine->GetTabBoxes().erase( it );
            
            pInsLine->GetTabBoxes().insert( it, pCpyLine->GetTabBoxes().begin(), pCpyLine->GetTabBoxes().end());
            pCpyLine->GetTabBoxes().clear();
            
            delete pTblBox;

            return false; 
        }
    }
    return true;
}

static bool lcl_MergeGCLine(SwTableLine* pLn, _GCLinePara* pGCPara)
{
    sal_uInt16 nLen = pLn->GetTabBoxes().size();
    if( nLen )
    {
        while( 1 == nLen )
        {
            
            SwTableBox* pBox = pLn->GetTabBoxes().front();
            if( pBox->GetTabLines().empty() )
                break;

            SwTableLine* pLine = pBox->GetTabLines()[0];

            
            
            
            nLen = pBox->GetTabLines().size();

            SwTableLines& rLns = *pGCPara->pLns;
            sal_uInt16 nInsPos = rLns.GetPos( pLn );
            OSL_ENSURE( USHRT_MAX != nInsPos, "Could not find Line!" );

            SwTableBox* pUpper = pLn->GetUpper();

            rLns.erase( rLns.begin() + nInsPos );      
            rLns.insert( rLns.begin() + nInsPos, pBox->GetTabLines().begin(), pBox->GetTabLines().end() );

            
            
            const SfxPoolItem* pItem;
            if( SFX_ITEM_SET == pLn->GetFrmFmt()->GetItemState(
                                    RES_BACKGROUND, sal_True, &pItem ))
            {
                SwTableLines& rBoxLns = pBox->GetTabLines();
                for( sal_uInt16 nLns = 0; nLns < nLen; ++nLns )
                    if( SFX_ITEM_SET != rBoxLns[ nLns ]->GetFrmFmt()->
                            GetItemState( RES_BACKGROUND, sal_True ))
                        pGCPara->pShareFmts->SetAttr( *rBoxLns[ nLns ], *pItem );
            }

            pBox->GetTabLines().erase( pBox->GetTabLines().begin(), pBox->GetTabLines().begin() + nLen );  

            delete pLn;

            
            while( nLen-- )
                rLns[ nInsPos++ ]->SetUpper( pUpper );

            pLn = pLine;                        
            nLen = pLn->GetTabBoxes().size();
        }

        
        for( nLen = 0; nLen < pLn->GetTabBoxes().size(); ++nLen )
            if( !lcl_MergeGCBox( pLn->GetTabBoxes()[nLen], pGCPara ))
                --nLen;
    }
    return true;
}


void SwTable::GCLines()
{
    
    _GCLinePara aPara( GetTabLines() );
    SwShareBoxFmts aShareFmts;
    aPara.pShareFmts = &aShareFmts;
    for( sal_uInt16 n = 0; n < GetTabLines().size() &&
            lcl_MergeGCLine( GetTabLines()[n], &aPara ); ++n )
        ;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
