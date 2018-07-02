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

#include <hintids.hxx>
#include <editeng/boxitem.hxx>
#include <tblrwcl.hxx>
#include <swtblfmt.hxx>
#include <algorithm>

using namespace ::editeng;

inline const SvxBorderLine* GetLineTB( const SvxBoxItem* pBox, bool bTop )
{
    return bTop ? pBox->GetTop() : pBox->GetBottom();
}

bool SwGCBorder_BoxBrd::CheckLeftBorderOfFormat( const SwFrameFormat& rFormat )
{
    const SvxBorderLine* pBrd;
    const SfxPoolItem* pItem;
    if( SfxItemState::SET == rFormat.GetItemState( RES_BOX, true, &pItem ) &&
        nullptr != ( pBrd = static_cast<const SvxBoxItem*>(pItem)->GetLeft() ) )
    {
        if( *pBrdLn == *pBrd )
            bAnyBorderFnd = true;
        return true;
    }
    return false;
}

static bool lcl_GCBorder_ChkBoxBrd_B( const SwTableBox* pBox, SwGCBorder_BoxBrd* pPara );

static bool lcl_GCBorder_ChkBoxBrd_L( const SwTableLine* pLine, SwGCBorder_BoxBrd* pPara )
{
    const SwTableBox* pBox = pLine->GetTabBoxes().front().get();
    return lcl_GCBorder_ChkBoxBrd_B( pBox, pPara );
}

static bool lcl_GCBorder_ChkBoxBrd_B( const SwTableBox* pBox, SwGCBorder_BoxBrd* pPara )
{
    if( !pBox->GetTabLines().empty() )
    {
        for( auto pLine : pBox->GetTabLines() )
        {
            if (!lcl_GCBorder_ChkBoxBrd_L( pLine, pPara ))
            {
                return false;
            }
        }
        return true;
    }

    return pPara->CheckLeftBorderOfFormat( *pBox->GetFrameFormat() );
}

static void lcl_GCBorder_GetLastBox_B( const SwTableBox* pBox, std::vector<SwTableBox*>* pPara );

static void lcl_GCBorder_GetLastBox_L( const SwTableLine* pLine, std::vector<SwTableBox*>* pPara )
{
    const SwTableBoxes& rBoxes = pLine->GetTabBoxes();
    SwTableBox* pBox = rBoxes.back().get();
    lcl_GCBorder_GetLastBox_B( pBox, pPara );
}

static void lcl_GCBorder_GetLastBox_B( const SwTableBox* pBox, std::vector<SwTableBox*>* pPara )
{
    const SwTableLines& rLines = pBox->GetTabLines();
    if( !rLines.empty() )
    {
        for( const SwTableLine* pLine : rLines )
            lcl_GCBorder_GetLastBox_L( pLine, pPara );
    }
    else
        pPara->push_back( const_cast<SwTableBox*>(pBox) );
}

// Find the "end" of the passed BorderLine. Returns the "Layout"Pos!
static sal_uInt16 lcl_FindEndPosOfBorder( const SwCollectTableLineBoxes& rCollTLB,
                        const SvxBorderLine& rBrdLn, size_t& rStt, bool bTop )
{
    sal_uInt16 nPos, nLastPos = 0;
    for( size_t nEnd = rCollTLB.Count(); rStt < nEnd; ++rStt )
    {
        const SfxPoolItem* pItem;
        const SvxBorderLine* pBrd;
        const SwTableBox& rBox = rCollTLB.GetBox( rStt, &nPos );

        if( SfxItemState::SET != rBox.GetFrameFormat()->GetItemState(RES_BOX,true, &pItem )
            || nullptr == ( pBrd = GetLineTB( static_cast<const SvxBoxItem*>(pItem), bTop ))
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
    return SfxItemState::SET == rBox.GetFrameFormat()->GetItemState( RES_BOX, true, ppItem )
            ? GetLineTB( static_cast<const SvxBoxItem*>(*ppItem), bTop )
            : nullptr;
}

static void lcl_GCBorder_DelBorder( const SwCollectTableLineBoxes& rCollTLB,
                                size_t& rStt, bool bTop,
                                const SvxBorderLine& rLine,
                                const SfxPoolItem* pItem,
                                sal_uInt16 nEndPos,
                                SwShareBoxFormats* pShareFormats )
{
    SwTableBox* pBox = const_cast<SwTableBox*>(&rCollTLB.GetBox( rStt ));
    sal_uInt16 nNextPos;
    const SvxBorderLine* pLn = &rLine;

    do {
        if( pLn && *pLn == rLine )
        {
            SvxBoxItem aBox( *static_cast<const SvxBoxItem*>(pItem) );
            if( bTop )
                aBox.SetLine( nullptr, SvxBoxItemLine::TOP );
            else
                aBox.SetLine( nullptr, SvxBoxItemLine::BOTTOM );

            if( pShareFormats )
                pShareFormats->SetAttr( *pBox, aBox );
            else
                pBox->ClaimFrameFormat()->SetFormatAttr( aBox );
        }

        if( ++rStt >= rCollTLB.Count() )
            break;

        pBox = const_cast<SwTableBox*>(&rCollTLB.GetBox( rStt, &nNextPos ));
        if( nNextPos > nEndPos )
            break;

        pLn = lcl_GCBorder_GetBorder( *pBox, bTop, &pItem );

    } while( true );
}

static void lcl_GC_Box_Border( const SwTableBox* pBox, SwGCLineBorder* pPara );

void sw_GC_Line_Border( const SwTableLine* pLine, SwGCLineBorder* pGCPara )
{
    // First the right edge with the left edge of the succeeding Box within this Line
    {
        SwGCBorder_BoxBrd aBPara;
        const SvxBorderLine* pBrd;
        const SfxPoolItem* pItem;
        const SwTableBoxes& rBoxes = pLine->GetTabBoxes();
        for( SwTableBoxes::size_type n = 0, nBoxes = rBoxes.size() - 1; n < nBoxes; ++n )
        {
            std::vector<SwTableBox*> aBoxes;
            {
                SwTableBox* pBox = rBoxes[ n ].get();
                if( pBox->GetSttNd() )
                    aBoxes.insert( aBoxes.begin(), pBox );
                else
                    lcl_GCBorder_GetLastBox_B( pBox, &aBoxes );
            }

            for( SwTableBoxes::size_type i = aBoxes.size(); i; )
            {
                SwTableBox* pBox;
                if( SfxItemState::SET == (pBox = aBoxes[ --i ])->GetFrameFormat()->
                    GetItemState( RES_BOX, true, &pItem ) &&
                    nullptr != ( pBrd = static_cast<const SvxBoxItem*>(pItem)->GetRight() ) )
                {
                    aBPara.SetBorder( *pBrd );
                    const SwTableBox* pNextBox = rBoxes[n+1].get();
                    if( lcl_GCBorder_ChkBoxBrd_B( pNextBox, &aBPara ) &&
                        aBPara.IsAnyBorderFound() )
                    {
                        SvxBoxItem aBox( *static_cast<const SvxBoxItem*>(pItem) );
                        aBox.SetLine( nullptr, SvxBoxItemLine::RIGHT );
                        if( pGCPara->pShareFormats )
                            pGCPara->pShareFormats->SetAttr( *pBox, aBox );
                        else
                            pBox->ClaimFrameFormat()->SetFormatAttr( aBox );
                    }
                }
            }

            aBoxes.clear();
        }
    }

    // And now the own bottom edge with the succeeding top edge
    if( !pGCPara->IsLastLine() )
    {
        SwCollectTableLineBoxes aBottom( false );
        SwCollectTableLineBoxes aTop( true );

        sw_Line_CollectBox( pLine, &aBottom );

        const SwTableLine* pNextLine = (*pGCPara->pLines)[ pGCPara->nLinePos+1 ];
        sw_Line_CollectBox( pNextLine, &aTop );

        // remove all "duplicated" Lines that are the same
        sal_uInt16 nBtmPos, nTopPos;

        size_t nSttBtm {0};
        size_t nSttTop {0};
        const size_t nEndBtm {aBottom.Count()};
        const size_t nEndTop {aTop.Count()};

        const SwTableBox *pBtmBox = &aBottom.GetBox( nSttBtm++, &nBtmPos );
        const SwTableBox *pTopBox = &aTop.GetBox( nSttTop++, &nTopPos );
        const SfxPoolItem *pBtmItem = nullptr, *pTopItem = nullptr;
        const SvxBorderLine *pBtmLine(nullptr), *pTopLine(nullptr);
        bool bGetTopItem = true, bGetBtmItem = true;

        do {
            if( bGetBtmItem )
                pBtmLine = lcl_GCBorder_GetBorder( *pBtmBox, false, &pBtmItem );
            if( bGetTopItem )
                pTopLine = lcl_GCBorder_GetBorder( *pTopBox, true, &pTopItem );

            if( pTopLine && pBtmLine && *pTopLine == *pBtmLine )
            {
                // We can remove one, but which one?
                const size_t nSavSttBtm {nSttBtm};
                const size_t nSavSttTop {nSttTop};
                sal_uInt16 nBtmEndPos = ::lcl_FindEndPosOfBorder( aBottom,
                                                *pTopLine, nSttBtm, false );
                if( !nBtmEndPos ) nBtmEndPos = nBtmPos;
                sal_uInt16 nTopEndPos = ::lcl_FindEndPosOfBorder( aTop,
                                                *pTopLine, nSttTop, true );
                if( !nTopEndPos ) nTopEndPos = nTopPos;

                if( nTopEndPos <= nBtmEndPos )
                {
                    // Delete the TopBorders until BottomEndPos
                    nSttTop = nSavSttTop;
                    if( nTopPos <= nBtmEndPos )
                        lcl_GCBorder_DelBorder( aTop, --nSttTop, true,
                                            *pBtmLine, pTopItem, nBtmEndPos,
                                            pGCPara->pShareFormats );
                    else
                        nSttBtm = nSavSttBtm;
                }
                else
                {
                    // Else delete the BottomBorders until TopEndPos
                    nSttBtm = nSavSttBtm;
                    if( nBtmPos <= nTopEndPos )
                        lcl_GCBorder_DelBorder( aBottom, --nSttBtm, false,
                                            *pTopLine, pBtmItem, nTopEndPos,
                                            pGCPara->pShareFormats );
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
        lcl_GC_Box_Border(it->get(), pGCPara );

    ++pGCPara->nLinePos;
}

static void lcl_GC_Box_Border( const SwTableBox* pBox, SwGCLineBorder* pPara )
{
    if( !pBox->GetTabLines().empty() )
    {
        SwGCLineBorder aPara( *pBox );
        aPara.pShareFormats = pPara->pShareFormats;
        for( const SwTableLine* pLine : pBox->GetTabLines() )
            sw_GC_Line_Border( pLine, &aPara );
    }
}

struct GCLinePara
{
    SwTableLines* pLns;
    SwShareBoxFormats* pShareFormats;

    GCLinePara( SwTableLines& rLns, GCLinePara* pPara = nullptr )
        : pLns( &rLns ), pShareFormats( pPara ? pPara->pShareFormats : nullptr )
    {}
};

static bool lcl_MergeGCLine(SwTableLine* pLine, GCLinePara* pPara);

static bool lcl_MergeGCBox(SwTableBox* pTableBox, GCLinePara* pPara)
{
    if( !pTableBox->GetTabLines().empty() )
    {
        // ATTENTION: The Line count can change!
        GCLinePara aPara( pTableBox->GetTabLines(), pPara );
        for( SwTableLines::size_type n = 0;
            n < pTableBox->GetTabLines().size() && lcl_MergeGCLine( pTableBox->GetTabLines()[n], &aPara );
            ++n )
            ;

        if( 1 == pTableBox->GetTabLines().size() )
        {
            // we have a box with a single line, so we just replace it by the line's boxes
            SwTableLine* pInsLine = pTableBox->GetUpper();
            SwTableLine* pCpyLine = pTableBox->GetTabLines()[0];
            auto it = std::find_if( pInsLine->GetTabBoxes().begin(), pInsLine->GetTabBoxes().end(),
                        [&] (std::unique_ptr<SwTableBox> const & itBox) { return itBox.get() == pTableBox; } );
            for( auto & pTabBox : pCpyLine->GetTabBoxes() )
                pTabBox->SetUpper( pInsLine );

            // remove the old box from its parent line and delete it
            it = pInsLine->GetTabBoxes().erase( it );
            // insert the nested line's boxes in its place
            pInsLine->GetTabBoxes().insert( it, std::make_move_iterator(pCpyLine->GetTabBoxes().begin()), std::make_move_iterator(pCpyLine->GetTabBoxes().end()));
            pCpyLine->GetTabBoxes().clear();

            return false; // set up anew
        }
    }
    return true;
}

static bool lcl_MergeGCLine(SwTableLine* pLn, GCLinePara* pGCPara)
{
    SwTableBoxes::size_type nBoxes = pLn->GetTabBoxes().size();
    if( nBoxes )
    {
        while( 1 == nBoxes )
        {
            // We have a Box with Lines
            SwTableBox* pBox = pLn->GetTabBoxes().front().get();
            if( pBox->GetTabLines().empty() )
                break;

            SwTableLine* pLine = pBox->GetTabLines()[0];

            // pLine turns into the current Line (that is rpLine), the rest is moved
            // into the LinesArray past the current one.
            // The LinesArray is in pPara!
            SwTableLines::size_type nLines = pBox->GetTabLines().size();

            SwTableLines& rLns = *pGCPara->pLns;
            sal_uInt16 nInsPos = rLns.GetPos( pLn );
            OSL_ENSURE( USHRT_MAX != nInsPos, "Could not find Line!" );

            SwTableBox* pUpper = pLn->GetUpper();

            rLns.erase( rLns.begin() + nInsPos );      // remove the Line from the array
            rLns.insert( rLns.begin() + nInsPos, pBox->GetTabLines().begin(), pBox->GetTabLines().end() );

            // JP 31.03.99: Bug 60000
            // Pass the attributes of the to-be-deleted Lines to the "inserted" one
            const SfxPoolItem* pItem;
            if( SfxItemState::SET == pLn->GetFrameFormat()->GetItemState(
                                    RES_BACKGROUND, true, &pItem ))
            {
                SwTableLines& rBoxLns = pBox->GetTabLines();
                for( auto pBoxLine : rBoxLns )
                    if( SfxItemState::SET != pBoxLine->GetFrameFormat()->
                            GetItemState( RES_BACKGROUND ))
                        pGCPara->pShareFormats->SetAttr( *pBoxLine, *pItem );
            }

            pBox->GetTabLines().erase( pBox->GetTabLines().begin(), pBox->GetTabLines().begin() + nLines );  // Remove Lines from the array

            delete pLn;

            // Set the dependency anew
            while( nLines-- )
                rLns[ nInsPos++ ]->SetUpper( pUpper );

            pLn = pLine;                        // and set up anew
            nBoxes = pLn->GetTabBoxes().size();
        }

        // ATTENTION: The number of boxes can change!
        for( SwTableBoxes::size_type nLen = 0; nLen < pLn->GetTabBoxes().size(); ++nLen )
            if( !lcl_MergeGCBox( pLn->GetTabBoxes()[nLen].get(), pGCPara ))
                --nLen;
    }
    return true;
}

// Clean structure a bit
void SwTable::GCLines()
{
    // ATTENTION: The Line count can change!
    GCLinePara aPara( GetTabLines() );
    SwShareBoxFormats aShareFormats;
    aPara.pShareFormats = &aShareFormats;
    for( SwTableLines::size_type n = 0; n < GetTabLines().size() &&
            lcl_MergeGCLine( GetTabLines()[n], &aPara ); ++n )
        ;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
