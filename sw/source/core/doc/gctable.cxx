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



#include <hintids.hxx>
#include <editeng/boxitem.hxx>
#include <tblrwcl.hxx>
#include <swtblfmt.hxx>

using namespace ::editeng;

inline const SvxBorderLine* GetLineTB( const SvxBoxItem* pBox, sal_Bool bTop )
{
    return bTop ? pBox->GetTop() : pBox->GetBottom();
}


sal_Bool _SwGCBorder_BoxBrd::CheckLeftBorderOfFormat( const SwFrmFmt& rFmt )
{
    const SvxBorderLine* pBrd;
    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET == rFmt.GetItemState( RES_BOX, sal_True, &pItem ) &&
        0 != ( pBrd = ((SvxBoxItem*)pItem)->GetLeft() ) )
    {
        if( *pBrdLn == *pBrd )
            bAnyBorderFnd = sal_True;
        return sal_True;
    }
    return sal_False;
}



sal_Bool lcl_GCBorder_ChkBoxBrd_L( const SwTableLine*& rpLine, void* pPara )
{
    const SwTableBox* pBox = rpLine->GetTabBoxes()[ 0 ];
    return lcl_GCBorder_ChkBoxBrd_B( pBox, pPara );
}

sal_Bool lcl_GCBorder_ChkBoxBrd_B( const SwTableBox*& rpBox, void* pPara )
{
    sal_Bool bRet = sal_True;
    if( rpBox->GetTabLines().Count() )
    {
        for( sal_uInt16 n = 0, nLines = rpBox->GetTabLines().Count();
                n < nLines && bRet; ++n )
        {
            const SwTableLine* pLine = rpBox->GetTabLines()[ n ];
            bRet = lcl_GCBorder_ChkBoxBrd_L( pLine, pPara );
        }
    }
    else
    {
        _SwGCBorder_BoxBrd* pBPara = (_SwGCBorder_BoxBrd*)pPara;
        bRet = pBPara->CheckLeftBorderOfFormat( *rpBox->GetFrmFmt() );
    }
    return bRet;
}

sal_Bool lcl_GCBorder_GetLastBox_L( const SwTableLine*& rpLine, void* pPara )
{
    const SwTableBoxes& rBoxes = rpLine->GetTabBoxes();
    const SwTableBox* pBox = rBoxes[ rBoxes.Count()-1 ];
    ::lcl_GCBorder_GetLastBox_B( pBox, pPara );
    return sal_True;
}

sal_Bool lcl_GCBorder_GetLastBox_B( const SwTableBox*& rpBox, void* pPara )
{
    SwTableLines& rLines = (SwTableLines&)rpBox->GetTabLines();
    if( rLines.Count() )
        rLines.ForEach( &lcl_GCBorder_GetLastBox_L, pPara );
    else
        ((SwTableBoxes*)pPara)->Insert( rpBox, ((SwTableBoxes*)pPara)->Count() );
    return sal_True;
}

// Find the "end" of the passed BorderLine. Returns the "Layout"Pos!
sal_uInt16 lcl_FindEndPosOfBorder( const SwCollectTblLineBoxes& rCollTLB,
                        const SvxBorderLine& rBrdLn, sal_uInt16& rStt, sal_Bool bTop )
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

inline const SvxBorderLine* lcl_GCBorder_GetBorder( const SwTableBox& rBox,
                                                sal_Bool bTop,
                                                const SfxPoolItem** ppItem )
{
    return SFX_ITEM_SET == rBox.GetFrmFmt()->GetItemState( RES_BOX, sal_True, ppItem )
            ? GetLineTB( (SvxBoxItem*)*ppItem, bTop )
            : 0;
}

void lcl_GCBorder_DelBorder( const SwCollectTblLineBoxes& rCollTLB,
                                sal_uInt16& rStt, sal_Bool bTop,
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

    } while( sal_True );
}


sal_Bool lcl_GC_Line_Border( const SwTableLine*& rpLine, void* pPara )
{
    _SwGCLineBorder* pGCPara = (_SwGCLineBorder*)pPara;

    // First the right edge with the left edge of the succeeding Box within this Line
    {
        _SwGCBorder_BoxBrd aBPara;
        const SvxBorderLine* pBrd;
        const SfxPoolItem* pItem;
        const SwTableBoxes& rBoxes = rpLine->GetTabBoxes();
        for( sal_uInt16 n = 0, nBoxes = rBoxes.Count() - 1; n < nBoxes; ++n )
        {
            SwTableBoxes aBoxes;
            {
                const SwTableBox* pBox = rBoxes[ n ];
                if( pBox->GetSttNd() )
                    aBoxes.Insert( pBox, 0 );
                else
                    lcl_GCBorder_GetLastBox_B( pBox, &aBoxes );
            }

            SwTableBox* pBox;
            for( sal_uInt16 i = aBoxes.Count(); i; )
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

            aBoxes.Remove( 0, aBoxes.Count() );
        }
    }

    // And now the own bottom edge with the succeeding top edge
    if( !pGCPara->IsLastLine() )
    {
        SwCollectTblLineBoxes aBottom( sal_False );
        SwCollectTblLineBoxes aTop( sal_True );

        ::lcl_Line_CollectBox( rpLine, &aBottom );

        const SwTableLine* pNextLine = (*pGCPara->pLines)[ pGCPara->nLinePos+1 ];
        ::lcl_Line_CollectBox( pNextLine, &aTop );

        // remove all "duplicated" Lines that are the same
        sal_uInt16 nBtmPos, nTopPos,
                nSttBtm = 0, nSttTop = 0,
                nEndBtm = aBottom.Count(), nEndTop = aTop.Count();

        const SwTableBox *pBtmBox = &aBottom.GetBox( nSttBtm++, &nBtmPos ),
                         *pTopBox = &aTop.GetBox( nSttTop++, &nTopPos );
        const SfxPoolItem *pBtmItem = 0, *pTopItem = 0;
        const SvxBorderLine *pBtmLine(0), *pTopLine(0);
        sal_Bool bGetTopItem = sal_True, bGetBtmItem = sal_True;

        do {
            if( bGetBtmItem )
                pBtmLine = lcl_GCBorder_GetBorder( *pBtmBox, sal_False, &pBtmItem );
            if( bGetTopItem )
                pTopLine = lcl_GCBorder_GetBorder( *pTopBox, sal_True, &pTopItem );

            if( pTopLine && pBtmLine && *pTopLine == *pBtmLine )
            {
                // We can remove one, but which one?
                sal_uInt16 nSavSttBtm = nSttBtm, nSavSttTop = nSttTop;
                sal_uInt16 nBtmEndPos = ::lcl_FindEndPosOfBorder( aBottom,
                                                *pTopLine, nSttBtm, sal_False );
                if( !nBtmEndPos ) nBtmEndPos = nBtmPos;
                sal_uInt16 nTopEndPos = ::lcl_FindEndPosOfBorder( aTop,
                                                *pTopLine, nSttTop, sal_True );
                if( !nTopEndPos ) nTopEndPos = nTopPos;


                if( nTopEndPos <= nBtmEndPos )
                {
                    // Delete the TopBorders until BottomEndPos
                    nSttTop = nSavSttTop;
                    if( nTopPos <= nBtmEndPos )
                        lcl_GCBorder_DelBorder( aTop, --nSttTop, sal_True,
                                            *pBtmLine, pTopItem, nBtmEndPos,
                                            pGCPara->pShareFmts );
                    else
                        nSttBtm = nSavSttBtm;
                }
                else
                {
                    // Else delete the BottomBorders until TopEndPos
                    nSttBtm = nSavSttBtm;
                    if( nBtmPos <= nTopEndPos )
                        lcl_GCBorder_DelBorder( aBottom, --nSttBtm, sal_False,
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
                bGetTopItem = bGetBtmItem = sal_True;
            }
            else if( nTopPos < nBtmPos )
            {
                if( nSttTop >= nEndTop )
                    break;
                pTopBox = &aTop.GetBox( nSttTop++, &nTopPos );
                bGetTopItem = sal_True;
                bGetBtmItem = sal_False;
            }
            else
            {
                if( nSttBtm >= nEndBtm )
                    break;
                pBtmBox = &aBottom.GetBox( nSttBtm++, &nBtmPos );
                bGetTopItem = sal_False;
                bGetBtmItem = sal_True;
            }

        } while( sal_True );
    }

    ((SwTableLine*)rpLine)->GetTabBoxes().ForEach( &lcl_GC_Box_Border, pPara );

    ++pGCPara->nLinePos;

    return sal_True;
}

sal_Bool lcl_GC_Box_Border( const SwTableBox*& rpBox, void* pPara )
{
    if( rpBox->GetTabLines().Count() )
    {
        _SwGCLineBorder aPara( *rpBox );
        aPara.pShareFmts = ((_SwGCLineBorder*)pPara)->pShareFmts;
        ((SwTableBox*)rpBox)->GetTabLines().ForEach( &lcl_GC_Line_Border, &aPara );
    }
    return sal_True;
}

struct _GCLinePara
{
    SwTableLines* pLns;
    SwShareBoxFmts* pShareFmts;

    _GCLinePara( SwTableLines& rLns, _GCLinePara* pPara = 0 )
        : pLns( &rLns ), pShareFmts( pPara ? pPara->pShareFmts : 0 )
    {}
};

sal_Bool lcl_MergeGCBox( const SwTableBox*& rpTblBox, void* pPara )
{
    SwTableBox*& rpBox = (SwTableBox*&)rpTblBox;
    sal_uInt16 n, nLen = rpBox->GetTabLines().Count();
    if( nLen )
    {
        // ATTENTION: The Line count can change!
        _GCLinePara aPara( rpBox->GetTabLines(), (_GCLinePara*)pPara );
        for( n = 0; n < rpBox->GetTabLines().Count() &&
            lcl_MergeGCLine( *(rpBox->GetTabLines().GetData() + n), &aPara );
            ++n )
            ;

        if( 1 == rpBox->GetTabLines().Count() )
        {
            // Box with a Line, then move all the Line's Boxes after this Box
            // into the parent Line and delete this Box
            SwTableLine* pInsLine = rpBox->GetUpper();
            SwTableLine* pCpyLine = rpBox->GetTabLines()[0];
            sal_uInt16 nInsPos = pInsLine->GetTabBoxes().C40_GETPOS( SwTableBox, rpBox );
            for( n = 0; n < pCpyLine->GetTabBoxes().Count(); ++n )
                pCpyLine->GetTabBoxes()[n]->SetUpper( pInsLine );

            pInsLine->GetTabBoxes().Insert( &pCpyLine->GetTabBoxes(), nInsPos+1 );
            pCpyLine->GetTabBoxes().Remove( 0, n );
            // Delete the old Box with the Line
            pInsLine->GetTabBoxes().DeleteAndDestroy( nInsPos );

            return sal_False; // set up anew
        }
    }
    return sal_True;
}

sal_Bool lcl_MergeGCLine( const SwTableLine*& rpLine, void* pPara )
{
    SwTableLine* pLn = (SwTableLine*)rpLine;
    sal_uInt16 nLen = pLn->GetTabBoxes().Count();
    if( nLen )
    {
        _GCLinePara* pGCPara = (_GCLinePara*)pPara;
        while( 1 == nLen )
        {
            // We have a Box with Lines
            SwTableBox* pBox = pLn->GetTabBoxes()[0];
            if( !pBox->GetTabLines().Count() )
                break;

            SwTableLine* pLine = pBox->GetTabLines()[0];

            // pLine turns into the current Line (that is rpLine), the rest is moved
            // into the LinesArray past the current one.
            // The LinesArray is in pPara!
            nLen = pBox->GetTabLines().Count();

            SwTableLines& rLns = *pGCPara->pLns;
            const SwTableLine* pTmp = pLn;
            sal_uInt16 nInsPos = rLns.GetPos( pTmp );
            OSL_ENSURE( USHRT_MAX != nInsPos, "Could not find Line!" );

            SwTableBox* pUpper = pLn->GetUpper();

            rLns.Remove( nInsPos, 1 );      // remove the Line from the array
            rLns.Insert( &pBox->GetTabLines(), nInsPos );

            // JP 31.03.99: Bug 60000
            // Pass the attributes of the to-be-deleted Lines to the "inserted" one
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

            pBox->GetTabLines().Remove( 0, nLen );  // Remove Lines from the array

            delete pLn;

            // Set the dependency anew
            while( nLen-- )
                rLns[ nInsPos++ ]->SetUpper( pUpper );

            pLn = pLine;                        // and set up anew
            nLen = pLn->GetTabBoxes().Count();
        }

        // ATTENTION: The number of boxes can change!
        for( nLen = 0; nLen < pLn->GetTabBoxes().Count(); ++nLen )
            if( !lcl_MergeGCBox( *(pLn->GetTabBoxes().GetData() + nLen ), pPara ))
                --nLen;
    }
    return sal_True;
}

// Clean structure a bit
void SwTable::GCLines()
{
    // ATTENTION: The Line count can change!
    _GCLinePara aPara( GetTabLines() );
    SwShareBoxFmts aShareFmts;
    aPara.pShareFmts = &aShareFmts;
    for( sal_uInt16 n = 0; n < GetTabLines().Count() &&
            lcl_MergeGCLine( *(GetTabLines().GetData() + n ), &aPara ); ++n )
        ;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
