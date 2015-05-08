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

#include <unotools/linguprops.hxx>
#include <unotools/lingucfg.hxx>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <hintids.hxx>
#include <com/sun/star/util/XCloseable.hpp>
#include <sfx2/progress.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/protitem.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/prntitem.hxx>
#include <svx/fmglob.hxx>
#include <svx/svdouno.hxx>
#include <svx/fmpage.hxx>
#include <editeng/frmdiritem.hxx>
#include <swmodule.hxx>
#include <modcfg.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <SwStyleNameMapper.hxx>
#include <drawdoc.hxx>
#include <fchrfmt.hxx>
#include <frmatr.hxx>
#include <txatbase.hxx>
#include <fmtfld.hxx>
#include <fmtornt.hxx>
#include <fmtcntnt.hxx>
#include <fmtanchr.hxx>
#include <fmtfsize.hxx>
#include <fmtsrnd.hxx>
#include <fmtflcnt.hxx>
#include <fmtcnct.hxx>
#include <frmfmt.hxx>
#include <txtflcnt.hxx>
#include <docfld.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <ndnotxt.hxx>
#include <ndole.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <DocumentSettingManager.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <DocumentContentOperationsManager.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentState.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <cntfrm.hxx>
#include <flyfrm.hxx>
#include <fesh.hxx>
#include <docsh.hxx>
#include <dflyobj.hxx>
#include <dcontact.hxx>
#include <swundo.hxx>
#include <flypos.hxx>
#include <UndoInsert.hxx>
#include <expfld.hxx>
#include <poolfmt.hxx>
#include <docary.hxx>
#include <swtable.hxx>
#include <tblsel.hxx>
#include <viewopt.hxx>
#include <fldupde.hxx>
#include <txtftn.hxx>
#include <ftnidx.hxx>
#include <ftninfo.hxx>
#include <pagedesc.hxx>
#include <PostItMgr.hxx>
#include <comcore.hrc>
#include <tools/datetimeutils.hxx>

#include <unoframe.hxx>

#include <sortedobjs.hxx>

#include <vector>

using namespace ::com::sun::star;

#define DEF_FLY_WIDTH    2268   // Default width for FlyFrms (2268 == 4cm)

static bool lcl_IsItemSet(const SwCntntNode & rNode, sal_uInt16 which)
{
    bool bResult = false;

    if (SfxItemState::SET == rNode.GetSwAttrSet().GetItemState(which))
        bResult = true;

    return bResult;
}

SdrObject* SwDoc::CloneSdrObj( const SdrObject& rObj, bool bMoveWithinDoc,
                                bool bInsInPage )
{
    // #i52858# - method name changed
    SdrPage *pPg = getIDocumentDrawModelAccess().GetOrCreateDrawModel()->GetPage( 0 );
    if( !pPg )
    {
        pPg = getIDocumentDrawModelAccess().GetDrawModel()->AllocPage( false );
        getIDocumentDrawModelAccess().GetDrawModel()->InsertPage( pPg );
    }

    SdrObject *pObj = rObj.Clone();
    if( bMoveWithinDoc && FmFormInventor == pObj->GetObjInventor() )
    {
        // We need to preserve the Name for Controls
        uno::Reference< awt::XControlModel >  xModel = static_cast<SdrUnoObj*>(pObj)->GetUnoControlModel();
        uno::Any aVal;
        uno::Reference< beans::XPropertySet >  xSet(xModel, uno::UNO_QUERY);
        OUString sName("Name");
        if( xSet.is() )
            aVal = xSet->getPropertyValue( sName );
        if( bInsInPage )
            pPg->InsertObject( pObj );
        if( xSet.is() )
            xSet->setPropertyValue( sName, aVal );
    }
    else if( bInsInPage )
        pPg->InsertObject( pObj );

    // For drawing objects: set layer of cloned object to invisible layer
    SdrLayerID nLayerIdForClone = rObj.GetLayer();
    if ( !pObj->ISA(SwFlyDrawObj) &&
         !pObj->ISA(SwVirtFlyDrawObj) &&
         !IS_TYPE(SdrObject,pObj) )
    {
        if ( getIDocumentDrawModelAccess().IsVisibleLayerId( nLayerIdForClone ) )
        {
            nLayerIdForClone = getIDocumentDrawModelAccess().GetInvisibleLayerIdByVisibleOne( nLayerIdForClone );
        }
    }
    pObj->SetLayer( nLayerIdForClone );

    return pObj;
}

SwFlyFrmFmt* SwDoc::_MakeFlySection( const SwPosition& rAnchPos,
                                    const SwCntntNode& rNode,
                                    RndStdIds eRequestId,
                                    const SfxItemSet* pFlySet,
                                    SwFrmFmt* pFrmFmt )
{
    if( !pFrmFmt )
        pFrmFmt = getIDocumentStylePoolAccess().GetFrmFmtFromPool( RES_POOLFRM_FRAME );

    OUString sName;
    if( !mbInReading )
        switch( rNode.GetNodeType() )
        {
        case ND_GRFNODE:        sName = GetUniqueGrfName();     break;
        case ND_OLENODE:        sName = GetUniqueOLEName();     break;
        default:                sName = GetUniqueFrameName();   break;
        }
    SwFlyFrmFmt* pFmt = MakeFlyFrmFmt( sName, pFrmFmt );

    // Create content and connect to the format.
    // Create CntntNode and put it into the autotext selection.
    SwNodeRange aRange( GetNodes().GetEndOfAutotext(), -1,
                        GetNodes().GetEndOfAutotext() );
    GetNodes().SectionDown( &aRange, SwFlyStartNode );

    pFmt->SetFmtAttr( SwFmtCntnt( rNode.StartOfSectionNode() ));

    const SwFmtAnchor* pAnchor = 0;
    if( pFlySet )
    {
        pFlySet->GetItemState( RES_ANCHOR, false,
                                (const SfxPoolItem**)&pAnchor );
        if( SfxItemState::SET == pFlySet->GetItemState( RES_CNTNT, false ))
        {
            SfxItemSet aTmpSet( *pFlySet );
            aTmpSet.ClearItem( RES_CNTNT );
            pFmt->SetFmtAttr( aTmpSet );
        }
        else
            pFmt->SetFmtAttr( *pFlySet );
    }

    // Anchor not yet set?
    RndStdIds eAnchorId = pAnchor ? pAnchor->GetAnchorId()
                                  : pFmt->GetAnchor().GetAnchorId();
    // #i107811# Assure that at-page anchored fly frames have a page num or a
    // content anchor set.
    if ( !pAnchor ||
         ( FLY_AT_PAGE != pAnchor->GetAnchorId() &&
           !pAnchor->GetCntntAnchor() ) ||
         ( FLY_AT_PAGE == pAnchor->GetAnchorId() &&
           !pAnchor->GetCntntAnchor() &&
           pAnchor->GetPageNum() == 0 ) )
    {
        // set it again, needed for Undo
        SwFmtAnchor aAnch( pFmt->GetAnchor() );
        if (pAnchor && (FLY_AT_FLY == pAnchor->GetAnchorId()))
        {
            SwPosition aPos( *rAnchPos.nNode.GetNode().FindFlyStartNode() );
            aAnch.SetAnchor( &aPos );
            eAnchorId = FLY_AT_FLY;
        }
        else
        {
            if( eRequestId != aAnch.GetAnchorId() &&
                SfxItemState::SET != pFmt->GetItemState( RES_ANCHOR, true ) )
            {
                aAnch.SetType( eRequestId );
            }

            eAnchorId = aAnch.GetAnchorId();
            if ( FLY_AT_PAGE != eAnchorId ||
                 ( FLY_AT_PAGE == eAnchorId &&
                   ( !pAnchor ||
                     aAnch.GetPageNum() == 0 ) ) )
            {
                aAnch.SetAnchor( &rAnchPos );
            }
        }
        pFmt->SetFmtAttr( aAnch );
    }
    else
        eAnchorId = pFmt->GetAnchor().GetAnchorId();

    if ( FLY_AS_CHAR == eAnchorId )
    {
        const sal_Int32 nStt = rAnchPos.nContent.GetIndex();
        SwTxtNode * pTxtNode = rAnchPos.nNode.GetNode().GetTxtNode();

        OSL_ENSURE(pTxtNode!= 0, "There should be a SwTxtNode!");

        if (pTxtNode != NULL)
        {
            SwFmtFlyCnt aFmt( pFmt );
            // may fail if there's no space left or header/ftr
            if (!pTxtNode->InsertItem(aFmt, nStt, nStt))
            {   // pFmt is dead now
                return 0;
            }
        }
    }

    if( SfxItemState::SET != pFmt->GetAttrSet().GetItemState( RES_FRM_SIZE ))
    {
        SwFmtFrmSize aFmtSize( ATT_VAR_SIZE, 0, DEF_FLY_WIDTH );
        const SwNoTxtNode* pNoTxtNode = rNode.GetNoTxtNode();
        if( pNoTxtNode )
        {
            // Set size
             Size aSize( pNoTxtNode->GetTwipSize() );
            if( MINFLY > aSize.Width() )
                aSize.Width() = DEF_FLY_WIDTH;
            aFmtSize.SetWidth( aSize.Width() );
            if( aSize.Height() )
            {
                aFmtSize.SetHeight( aSize.Height() );
                aFmtSize.SetHeightSizeType( ATT_FIX_SIZE );
            }
        }
        pFmt->SetFmtAttr( aFmtSize );
    }

    // Set up frames
    if( getIDocumentLayoutAccess().GetCurrentViewShell() )
        pFmt->MakeFrms();           // ???

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        sal_uLong nNodeIdx = rAnchPos.nNode.GetIndex();
        const sal_Int32 nCntIdx = rAnchPos.nContent.GetIndex();
        GetIDocumentUndoRedo().AppendUndo(
            new SwUndoInsLayFmt( pFmt, nNodeIdx, nCntIdx ));
    }

    getIDocumentState().SetModified();
    return pFmt;
}

SwFlyFrmFmt* SwDoc::MakeFlySection( RndStdIds eAnchorType,
                                    const SwPosition* pAnchorPos,
                                    const SfxItemSet* pFlySet,
                                    SwFrmFmt* pFrmFmt, bool bCalledFromShell )
{
    SwFlyFrmFmt* pFmt = 0;
    if ( !pAnchorPos && (FLY_AT_PAGE != eAnchorType) )
    {
        const SwFmtAnchor* pAnch;
        if( (pFlySet && SfxItemState::SET == pFlySet->GetItemState(
                RES_ANCHOR, false, (const SfxPoolItem**)&pAnch )) ||
            ( pFrmFmt && SfxItemState::SET == pFrmFmt->GetItemState(
                RES_ANCHOR, true, (const SfxPoolItem**)&pAnch )) )
        {
            if ( (FLY_AT_PAGE != pAnch->GetAnchorId()) )
            {
                pAnchorPos = pAnch->GetCntntAnchor();
            }
        }
    }

    if (pAnchorPos)
    {
        if( !pFrmFmt )
            pFrmFmt = getIDocumentStylePoolAccess().GetFrmFmtFromPool( RES_POOLFRM_FRAME );

        sal_uInt16 nCollId = static_cast<sal_uInt16>(
            GetDocumentSettingManager().get(IDocumentSettingAccess::HTML_MODE) ? RES_POOLCOLL_TEXT : RES_POOLCOLL_FRAME );

        /* If there is no adjust item in the paragraph style for the content node of the new fly section
           propagate an existing adjust item at the anchor to the new content node. */
        SwCntntNode * pNewTxtNd = GetNodes().MakeTxtNode
            (SwNodeIndex( GetNodes().GetEndOfAutotext()),
             getIDocumentStylePoolAccess().GetTxtCollFromPool( nCollId ));
        SwCntntNode * pAnchorNode = pAnchorPos->nNode.GetNode().GetCntntNode();
        assert(pAnchorNode); // pAnchorNode from cursor, must be valid

        const SfxPoolItem * pItem = NULL;

        if (bCalledFromShell && !lcl_IsItemSet(*pNewTxtNd, RES_PARATR_ADJUST) &&
            SfxItemState::SET == pAnchorNode->GetSwAttrSet().
            GetItemState(RES_PARATR_ADJUST, true, &pItem))
        {
            static_cast<SwCntntNode *>(pNewTxtNd)->SetAttr(*pItem);
        }

         pFmt = _MakeFlySection( *pAnchorPos, *pNewTxtNd,
                                eAnchorType, pFlySet, pFrmFmt );
    }
    return pFmt;
}

SwFlyFrmFmt* SwDoc::MakeFlyAndMove( const SwPaM& rPam, const SfxItemSet& rSet,
                                    const SwSelBoxes* pSelBoxes,
                                    SwFrmFmt *pParent )
{
    const SwFmtAnchor& rAnch = static_cast<const SwFmtAnchor&>(rSet.Get( RES_ANCHOR ));

    GetIDocumentUndoRedo().StartUndo( UNDO_INSLAYFMT, NULL );

    SwFlyFrmFmt* pFmt = MakeFlySection( rAnch.GetAnchorId(), rPam.GetPoint(),
                                        &rSet, pParent );

    // If content is selected, it becomes the new frame's content.
    // Namely, it is moved into the NodeArray's appropriate section.

    if( pFmt )
    {
        do {        // middle check loop
            const SwFmtCntnt &rCntnt = pFmt->GetCntnt();
            OSL_ENSURE( rCntnt.GetCntntIdx(), "No content prepared." );
            SwNodeIndex aIndex( *(rCntnt.GetCntntIdx()), 1 );
            SwCntntNode *pNode = aIndex.GetNode().GetCntntNode();

            // Attention: Do not create an index on the stack, or we
            // cannot delete CntntNode in the end!
            SwPosition aPos( aIndex );
            aPos.nContent.Assign( pNode, 0 );

            if( pSelBoxes && !pSelBoxes->empty() )
            {
                // Table selection
                // Copy parts of a table: create a table with the same width as the
                // original one and move (copy and delete) the selected boxes.
                // The size is corrected on a percentage basis.

                SwTableNode* pTblNd = (SwTableNode*)(*pSelBoxes)[0]->
                                                GetSttNd()->FindTableNode();
                if( !pTblNd )
                    break;

                SwTable& rTbl = pTblNd->GetTable();

                // Did we select the whole table?
                if( pSelBoxes->size() == rTbl.GetTabSortBoxes().size() )
                {
                    // move the whole table
                    SwNodeRange aRg( *pTblNd, 0, *pTblNd->EndOfSectionNode(), 1 );

                    // If we move the whole table and it is located within a
                    // FlyFrame, the we create a TextNode after it.
                    // So that this FlyFrame is preserved.
                    if( aRg.aEnd.GetNode().IsEndNode() )
                        GetNodes().MakeTxtNode( aRg.aStart,
                                    (SwTxtFmtColl*)GetDfltTxtFmtColl() );

                    getIDocumentContentOperations().MoveNodeRange( aRg, aPos.nNode, IDocumentContentOperations::DOC_MOVEDEFAULT );
                }
                else
                {
                    rTbl.MakeCopy( this, aPos, *pSelBoxes );
                    // Don't delete a part of a table with row span!!
                    // You could delete the content instead -> ToDo
                    //rTbl.DeleteSel( this, *pSelBoxes, 0, 0, true, true );
                }

                // If the table is within the frame, then copy without the following TextNode
                aIndex = rCntnt.GetCntntIdx()->GetNode().EndOfSectionIndex() - 1;
                OSL_ENSURE( aIndex.GetNode().GetTxtNode(),
                        "a TextNode should be here" );
                aPos.nContent.Assign( 0, 0 );       // Deregister index!
                GetNodes().Delete( aIndex, 1 );

                // This is a hack: whilst FlyFrames/Headers/Footers are not undoable we delete all Undo objects
                if( GetIDocumentUndoRedo().DoesUndo() )
                {
                    GetIDocumentUndoRedo().DelAllUndoObj();
                }
            }
            else
            {
                // copy all Pams and then delete all
                SwPaM* pTmp = (SwPaM*)&rPam;
                bool bOldFlag = mbCopyIsMove;
                bool const bOldUndo = GetIDocumentUndoRedo().DoesUndo();
                bool const bOldRedlineMove(getIDocumentRedlineAccess().IsRedlineMove());
                mbCopyIsMove = true;
                GetIDocumentUndoRedo().DoUndo(false);
                getIDocumentRedlineAccess().SetRedlineMove(true);
                do {
                    if( pTmp->HasMark() &&
                        *pTmp->GetPoint() != *pTmp->GetMark() )
                    {
                        getIDocumentContentOperations().CopyRange( *pTmp, aPos, false );
                    }
                    pTmp = static_cast<SwPaM*>(pTmp->GetNext());
                } while ( &rPam != pTmp );
                getIDocumentRedlineAccess().SetRedlineMove(bOldRedlineMove);
                mbCopyIsMove = bOldFlag;
                GetIDocumentUndoRedo().DoUndo(bOldUndo);

                pTmp = (SwPaM*)&rPam;
                do {
                    if( pTmp->HasMark() &&
                        *pTmp->GetPoint() != *pTmp->GetMark() )
                    {
                        getIDocumentContentOperations().DeleteAndJoin( *pTmp );
                    }
                    pTmp = static_cast<SwPaM*>(pTmp->GetNext());
                } while ( &rPam != pTmp );
            }
        } while( false );
    }

    getIDocumentState().SetModified();

    GetIDocumentUndoRedo().EndUndo( UNDO_INSLAYFMT, NULL );

    return pFmt;
}


/*
 * paragraph frames - o.k. if the PaM includes the paragraph from the beginning
 *                    to the beginning of the next paragraph at least
 * frames at character - o.k. if the PaM starts at least at the same position
 *                      as the frame
 */
static bool lcl_TstFlyRange( const SwPaM* pPam, const SwPosition* pFlyPos,
                        RndStdIds nAnchorId )
{
    bool bOk = false;
    const SwPaM* pTmp = pPam;
    do {
        const sal_uInt32 nFlyIndex = pFlyPos->nNode.GetIndex();
        const SwPosition* pPaMStart = pTmp->Start();
        const SwPosition* pPaMEnd = pTmp->End();
        const sal_uInt32 nPamStartIndex = pPaMStart->nNode.GetIndex();
        const sal_uInt32 nPamEndIndex = pPaMEnd->nNode.GetIndex();
        if (FLY_AT_PARA == nAnchorId)
            bOk = (nPamStartIndex < nFlyIndex && nPamEndIndex > nFlyIndex) ||
               (((nPamStartIndex == nFlyIndex) && (pPaMStart->nContent.GetIndex() == 0)) &&
               (nPamEndIndex > nFlyIndex));
        else
        {
            const sal_Int32 nFlyContentIndex = pFlyPos->nContent.GetIndex();
            const sal_Int32 nPamEndContentIndex = pPaMEnd->nContent.GetIndex();
            bOk = (nPamStartIndex < nFlyIndex &&
                (( nPamEndIndex > nFlyIndex )||
                 ((nPamEndIndex == nFlyIndex) &&
                  (nPamEndContentIndex > nFlyContentIndex))) )
                ||
                       (((nPamStartIndex == nFlyIndex) &&
                      (pPaMStart->nContent.GetIndex() <= nFlyContentIndex)) &&
                     ((nPamEndIndex > nFlyIndex) ||
                     (nPamEndContentIndex > nFlyContentIndex )));
        }

    } while( !bOk && pPam != ( pTmp = static_cast<const SwPaM*>(pTmp->GetNext()) ));
    return bOk;
}

SwPosFlyFrms SwDoc::GetAllFlyFmts( const SwPaM* pCmpRange, bool bDrawAlso,
                           bool bAsCharAlso ) const
{
    SwPosFlyFrms aRetval;
    SwFrmFmt *pFly;

    // collect all anchored somehow to paragraphs
    for( sal_uInt16 n = 0; n < GetSpzFrmFmts()->size(); ++n )
    {
        pFly = (*GetSpzFrmFmts())[ n ];
        bool bDrawFmt = bDrawAlso && RES_DRAWFRMFMT == pFly->Which();
        bool bFlyFmt = RES_FLYFRMFMT == pFly->Which();
        if( bFlyFmt || bDrawFmt )
        {
            const SwFmtAnchor& rAnchor = pFly->GetAnchor();
            SwPosition const*const pAPos = rAnchor.GetCntntAnchor();
            if (pAPos &&
                ((FLY_AT_PARA == rAnchor.GetAnchorId()) ||
                 (FLY_AT_FLY  == rAnchor.GetAnchorId()) ||
                 (FLY_AT_CHAR == rAnchor.GetAnchorId()) ||
                 ((FLY_AS_CHAR == rAnchor.GetAnchorId()) && bAsCharAlso)))
            {
                if( pCmpRange &&
                    !lcl_TstFlyRange( pCmpRange, pAPos, rAnchor.GetAnchorId() ))
                        continue;       // not a valid FlyFrame
                aRetval.insert(SwPosFlyFrmPtr(new SwPosFlyFrm(pAPos->nNode, pFly, aRetval.size())));
            }
        }
    }

    // If we don't have a layout we can't get page anchored FlyFrames.
    // Also, page anchored FlyFrames are only returned if no range is specified.
    if( !getIDocumentLayoutAccess().GetCurrentViewShell() || pCmpRange )
    {
        return aRetval;
    }

    const SwPageFrm *pPage = static_cast<const SwPageFrm*>(getIDocumentLayoutAccess().GetCurrentLayout()->GetLower());
    while( pPage )
    {
        if( pPage->GetSortedObjs() )
        {
            const SwSortedObjs &rObjs = *pPage->GetSortedObjs();
            for( size_t i = 0; i < rObjs.size(); ++i)
            {
                SwAnchoredObject* pAnchoredObj = rObjs[i];
                if ( pAnchoredObj->ISA(SwFlyFrm) )
                    pFly = &(pAnchoredObj->GetFrmFmt());
                else if ( bDrawAlso )
                    pFly = &(pAnchoredObj->GetFrmFmt());
                else
                    continue;

                const SwFmtAnchor& rAnchor = pFly->GetAnchor();
                if ((FLY_AT_PARA != rAnchor.GetAnchorId()) &&
                    (FLY_AT_FLY  != rAnchor.GetAnchorId()) &&
                    (FLY_AT_CHAR != rAnchor.GetAnchorId()))
                {
                    const SwCntntFrm * pCntntFrm = pPage->FindFirstBodyCntnt();
                    if ( !pCntntFrm )
                    {
                        // Oops! An empty page.
                        // In order not to lose the whole frame (RTF) we
                        // look for the last Cntnt before the page.
                        const SwPageFrm *pPrv = static_cast<const SwPageFrm*>(pPage->GetPrev());
                        while ( !pCntntFrm && pPrv )
                        {
                            pCntntFrm = pPrv->FindFirstBodyCntnt();
                            pPrv = static_cast<const SwPageFrm*>(pPrv->GetPrev());
                        }
                    }
                    if ( pCntntFrm )
                    {
                        SwNodeIndex aIdx( *pCntntFrm->GetNode() );
                        aRetval.insert(SwPosFlyFrmPtr(new SwPosFlyFrm(aIdx, pFly, aRetval.size())));
                    }
                }
            }
        }
        pPage = static_cast<const SwPageFrm*>(pPage->GetNext());
    }

    return aRetval;
}

/* #i6447# changed behaviour if lcl_CpyAttr:

   If the old item set contains the item to set (no inheritance) copy the item
   into the new set.

   If the old item set contains the item by inheritance and the new set
   contains the item, too:
   If the two items differ copy the item from the old set to the new set.

   Otherwise the new set will not be changed.
*/
static void lcl_CpyAttr( SfxItemSet &rNewSet, const SfxItemSet &rOldSet, sal_uInt16 nWhich )
{
    const SfxPoolItem *pOldItem = NULL, *pNewItem = NULL;

    rOldSet.GetItemState( nWhich, false, &pOldItem);
    if (pOldItem != NULL)
        rNewSet.Put( *pOldItem );
    else
    {
        pOldItem = rOldSet.GetItem( nWhich, true);
        if (pOldItem != NULL)
        {
            pNewItem = rNewSet.GetItem( nWhich, true);
            if (pNewItem != NULL)
            {
                if (*pOldItem != *pNewItem)
                    rNewSet.Put( *pOldItem );
            }
            else {
                OSL_FAIL("What am I doing here?");
            }
        }
        else {
            OSL_FAIL("What am I doing here?");
        }
    }

}

static SwFlyFrmFmt *
lcl_InsertLabel(SwDoc & rDoc, SwTxtFmtColls *const pTxtFmtCollTbl,
        SwUndoInsertLabel *const pUndo,
        SwLabelType const eType, OUString const& rTxt, OUString const& rSeparator,
            const OUString& rNumberingSeparator,
            const bool bBefore, const sal_uInt16 nId, const sal_uLong nNdIdx,
            const OUString& rCharacterStyle,
            const bool bCpyBrd )
{
    ::sw::UndoGuard const undoGuard(rDoc.GetIDocumentUndoRedo());

    bool bTable = false;    // To save some code.

    // Get the field first, because we retrieve the TxtColl via the field's name
    OSL_ENSURE( nId == USHRT_MAX  || nId < rDoc.getIDocumentFieldsAccess().GetFldTypes()->size(),
            "FldType index out of bounds." );
    SwFieldType *pType = (nId != USHRT_MAX) ? (*rDoc.getIDocumentFieldsAccess().GetFldTypes())[nId] : NULL;
    OSL_ENSURE(!pType || pType->Which() == RES_SETEXPFLD, "wrong Id for Label");

    SwTxtFmtColl * pColl = NULL;
    if( pType )
    {
        for( sal_uInt16 i = pTxtFmtCollTbl->size(); i; )
        {
            if( (*pTxtFmtCollTbl)[ --i ]->GetName()==pType->GetName() )
            {
                pColl = (*pTxtFmtCollTbl)[i];
                break;
            }
        }
        OSL_ENSURE( pColl, "no text collection found" );
    }

    if( !pColl )
    {
        pColl = rDoc.getIDocumentStylePoolAccess().GetTxtCollFromPool( RES_POOLCOLL_LABEL );
    }

    SwTxtNode *pNew = NULL;
    SwFlyFrmFmt* pNewFmt = NULL;

    switch ( eType )
    {
        case LTYPE_TABLE:
            bTable = true;
            // no break here
        case LTYPE_FLY:
            // At the FlySection's Beginning/End insert the corresponding Node with it's Field.
            // The Frame is created automatically.
            {
                SwStartNode *pSttNd = rDoc.GetNodes()[nNdIdx]->GetStartNode();
                OSL_ENSURE( pSttNd, "No StartNode in InsertLabel." );
                sal_uLong nNode;
                if( bBefore )
                {
                    nNode = pSttNd->GetIndex();
                    if( !bTable )
                        ++nNode;
                }
                else
                {
                    nNode = pSttNd->EndOfSectionIndex();
                    if( bTable )
                        ++nNode;
                }

                if( pUndo )
                    pUndo->SetNodePos( nNode );

                // Create Node for labeling paragraph.
                SwNodeIndex aIdx( rDoc.GetNodes(), nNode );
                pNew = rDoc.GetNodes().MakeTxtNode( aIdx, pColl );
            }
            break;

        case LTYPE_OBJECT:
            {
                // Destroy Frame,
                // insert new Frame,
                // insert the corresponding Node with Field into the new Frame,
                // insert the old Frame with the Object (Picture/OLE) paragraph-bound into the new Frame,
                // create Frames.

                // Get the FlyFrame's Format and decouple the Layout.
                SwFrmFmt *pOldFmt = rDoc.GetNodes()[nNdIdx]->GetFlyFmt();
                OSL_ENSURE( pOldFmt, "Couldn't find the Fly's Format." );
                // #i115719#
                // <title> and <description> attributes are lost when calling <DelFrms()>.
                // Thus, keep them and restore them after the calling <MakeFrms()>
                const bool bIsSwFlyFrmFmtInstance( dynamic_cast<SwFlyFrmFmt*>(pOldFmt) != 0 );
                const OUString sTitle( bIsSwFlyFrmFmtInstance
                                     ? static_cast<SwFlyFrmFmt*>(pOldFmt)->GetObjTitle()
                                     : OUString() );
                const OUString sDescription( bIsSwFlyFrmFmtInstance
                                           ? static_cast<SwFlyFrmFmt*>(pOldFmt)->GetObjDescription()
                                           : OUString() );
                pOldFmt->DelFrms();

                pNewFmt = rDoc.MakeFlyFrmFmt( rDoc.GetUniqueFrameName(),
                                rDoc.getIDocumentStylePoolAccess().GetFrmFmtFromPool(RES_POOLFRM_FRAME) );

                /* #i6447#: Only the selected items are copied from the old
                   format. */
                SfxItemSet* pNewSet = pNewFmt->GetAttrSet().Clone( true );

                // Copy only the set attributes.
                // The others should apply from the Templates.
                lcl_CpyAttr( *pNewSet, pOldFmt->GetAttrSet(), RES_PRINT );
                lcl_CpyAttr( *pNewSet, pOldFmt->GetAttrSet(), RES_OPAQUE );
                lcl_CpyAttr( *pNewSet, pOldFmt->GetAttrSet(), RES_PROTECT );
                lcl_CpyAttr( *pNewSet, pOldFmt->GetAttrSet(), RES_SURROUND );
                lcl_CpyAttr( *pNewSet, pOldFmt->GetAttrSet(), RES_VERT_ORIENT );
                lcl_CpyAttr( *pNewSet, pOldFmt->GetAttrSet(), RES_HORI_ORIENT );
                lcl_CpyAttr( *pNewSet, pOldFmt->GetAttrSet(), RES_LR_SPACE );
                lcl_CpyAttr( *pNewSet, pOldFmt->GetAttrSet(), RES_UL_SPACE );
                lcl_CpyAttr( *pNewSet, pOldFmt->GetAttrSet(), RES_BACKGROUND );
                if( bCpyBrd )
                {
                    // If there's no BoxItem at graphic, but the new Format has one, then set the
                    // default item in the new Set. Because the graphic's size has never changed!
                    const SfxPoolItem *pItem;
                    if( SfxItemState::SET == pOldFmt->GetAttrSet().
                            GetItemState( RES_BOX, true, &pItem ))
                        pNewSet->Put( *pItem );
                    else if( SfxItemState::SET == pNewFmt->GetAttrSet().
                            GetItemState( RES_BOX, true ))
                        pNewSet->Put( *GetDfltAttr( RES_BOX ) );

                    if( SfxItemState::SET == pOldFmt->GetAttrSet().
                            GetItemState( RES_SHADOW, true, &pItem ))
                        pNewSet->Put( *pItem );
                    else if( SfxItemState::SET == pNewFmt->GetAttrSet().
                            GetItemState( RES_SHADOW, true ))
                        pNewSet->Put( *GetDfltAttr( RES_SHADOW ) );
                }
                else
                {
                    // Hard-set the attributes, because they could come from the Template
                    // and then size calculations could not be correct anymore.
                    pNewSet->Put( SvxBoxItem(RES_BOX) );
                    pNewSet->Put( SvxShadowItem(RES_SHADOW) );
                }

                // Always transfer the anchor, which is a hard attribute anyways.
                pNewSet->Put( pOldFmt->GetAnchor() );

                // The new one should be changeable in its height.
                SwFmtFrmSize aFrmSize( pOldFmt->GetFrmSize() );
                aFrmSize.SetHeightSizeType( ATT_MIN_SIZE );
                pNewSet->Put( aFrmSize );

                SwStartNode* pSttNd = rDoc.GetNodes().MakeTextSection(
                            SwNodeIndex( rDoc.GetNodes().GetEndOfAutotext() ),
                            SwFlyStartNode, pColl );
                pNewSet->Put( SwFmtCntnt( pSttNd ));

                pNewFmt->SetFmtAttr( *pNewSet );

                // InCntnts need to be treated in a special way:
                // The TxtAttribute needs to be destroyed.
                // Unfortunately, this also destroys the Format next to the Frames.
                // To avoid this, we disconnect the attribute from the Format.

                const SwFmtAnchor& rAnchor = pNewFmt->GetAnchor();
                if ( FLY_AS_CHAR == rAnchor.GetAnchorId() )
                {
                    const SwPosition *pPos = rAnchor.GetCntntAnchor();
                    SwTxtNode *pTxtNode = pPos->nNode.GetNode().GetTxtNode();
                    OSL_ENSURE( pTxtNode->HasHints(), "Missing FlyInCnt-Hint." );
                    const sal_Int32 nIdx = pPos->nContent.GetIndex();
                    SwTxtAttr * const pHnt =
                        pTxtNode->GetTxtAttrForCharAt(nIdx, RES_TXTATR_FLYCNT);

                    OSL_ENSURE( pHnt && pHnt->Which() == RES_TXTATR_FLYCNT,
                                "Missing FlyInCnt-Hint." );
                    OSL_ENSURE( pHnt && pHnt->GetFlyCnt().GetFrmFmt() == pOldFmt,
                                "Wrong TxtFlyCnt-Hint." );

                    const_cast<SwFmtFlyCnt&>(pHnt->GetFlyCnt()).SetFlyFmt(
                            pNewFmt );
                }

                // The old one should not have a flow and it should be adjusted to above and
                // middle.
                // Also, the width should be 100% and it should also adjust the hight, if changed.
                pNewSet->ClearItem();

                pNewSet->Put( SwFmtSurround( SURROUND_NONE ) );
                pNewSet->Put( SvxOpaqueItem( RES_OPAQUE, true ) );

                sal_Int16 eVert = bBefore ? text::VertOrientation::BOTTOM : text::VertOrientation::TOP;
                pNewSet->Put( SwFmtVertOrient( 0, eVert ) );
                pNewSet->Put( SwFmtHoriOrient( 0, text::HoriOrientation::CENTER ) );

                aFrmSize = pOldFmt->GetFrmSize();
                aFrmSize.SetWidthPercent( 0 );
                aFrmSize.SetHeightPercent( 255 );
                pNewSet->Put( aFrmSize );

                // Hard-set the attributes, because they could come from the Template
                // and then size calculations could not be correct anymore.
                if( bCpyBrd )
                {
                    pNewSet->Put( SvxBoxItem(RES_BOX) );
                    pNewSet->Put( SvxShadowItem(RES_SHADOW) );
                }
                pNewSet->Put( SvxLRSpaceItem(RES_LR_SPACE) );
                pNewSet->Put( SvxULSpaceItem(RES_UL_SPACE) );

                // The old one is paragraph-bound to the paragraph in the new one.
                SwFmtAnchor aAnch( FLY_AT_PARA );
                SwNodeIndex aAnchIdx( *pNewFmt->GetCntnt().GetCntntIdx(), 1 );
                pNew = aAnchIdx.GetNode().GetTxtNode();
                SwPosition aPos( aAnchIdx );
                aAnch.SetAnchor( &aPos );
                pNewSet->Put( aAnch );

                if( pUndo )
                    pUndo->SetFlys( *pOldFmt, *pNewSet, *pNewFmt );
                else
                    pOldFmt->SetFmtAttr( *pNewSet );

                delete pNewSet;

                // Have only the FlyFrames created.
                // We leave this to established methods (especially for InCntFlys).
                pNewFmt->MakeFrms();
                // #i115719#
                if ( bIsSwFlyFrmFmtInstance )
                {
                    static_cast<SwFlyFrmFmt*>(pOldFmt)->SetObjTitle( sTitle );
                    static_cast<SwFlyFrmFmt*>(pOldFmt)->SetObjDescription( sDescription );
                }
            }
            break;

        default:
            OSL_ENSURE(false, "unknown LabelType?");
    }
    OSL_ENSURE( pNew, "No Label inserted" );
    if( pNew )
    {
        // #i61007# order of captions
        bool bOrderNumberingFirst = SW_MOD()->GetModuleConfig()->IsCaptionOrderNumberingFirst();
        // Work up OUString
        OUString aTxt;
        if( bOrderNumberingFirst )
        {
            aTxt = rNumberingSeparator;
        }
        if( pType)
        {
            aTxt += pType->GetName();
            if( !bOrderNumberingFirst )
                aTxt += " ";
        }
        sal_Int32 nIdx = aTxt.getLength();
        if( !rTxt.isEmpty() )
        {
            aTxt += rSeparator;
        }
        const sal_Int32 nSepIdx = aTxt.getLength();
        aTxt += rTxt;

        // Insert string
        SwIndex aIdx( pNew, 0 );
        pNew->InsertText( aTxt, aIdx );

        // Insert field
        if(pType)
        {
            SwSetExpField aFld( static_cast<SwSetExpFieldType*>(pType), OUString(), SVX_NUM_ARABIC);
            if( bOrderNumberingFirst )
                nIdx = 0;
            SwFmtFld aFmt( aFld );
            pNew->InsertItem( aFmt, nIdx, nIdx );
            if(!rCharacterStyle.isEmpty())
            {
                SwCharFmt* pCharFmt = rDoc.FindCharFmtByName(rCharacterStyle);
                if( !pCharFmt )
                {
                    const sal_uInt16 nMyId = SwStyleNameMapper::GetPoolIdFromUIName(rCharacterStyle, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT);
                    pCharFmt = rDoc.getIDocumentStylePoolAccess().GetCharFmtFromPool( nMyId );
                }
                if (pCharFmt)
                {
                    SwFmtCharFmt aCharFmt( pCharFmt );
                    pNew->InsertItem( aCharFmt, 0,
                        nSepIdx + 1, nsSetAttrMode::SETATTR_DONTEXPAND );
                }
            }
        }

        if ( bTable )
        {
            if ( bBefore )
            {
                if ( !pNew->GetSwAttrSet().GetKeep().GetValue()  )
                    pNew->SetAttr( SvxFmtKeepItem( true, RES_KEEP ) );
            }
            else
            {
                SwTableNode *const pNd =
                    rDoc.GetNodes()[nNdIdx]->GetStartNode()->GetTableNode();
                SwTable &rTbl = pNd->GetTable();
                if ( !rTbl.GetFrmFmt()->GetKeep().GetValue() )
                    rTbl.GetFrmFmt()->SetFmtAttr( SvxFmtKeepItem( true, RES_KEEP ) );
                if ( pUndo )
                    pUndo->SetUndoKeep();
            }
        }
        rDoc.getIDocumentState().SetModified();
    }

    return pNewFmt;
}

SwFlyFrmFmt *
SwDoc::InsertLabel(
        SwLabelType const eType, OUString const& rTxt, OUString const& rSeparator,
        OUString const& rNumberingSeparator,
        bool const bBefore, sal_uInt16 const nId, sal_uLong const nNdIdx,
        OUString const& rCharacterStyle,
        bool const bCpyBrd )
{
    SwUndoInsertLabel * pUndo(0);
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        pUndo = new SwUndoInsertLabel(
                        eType, rTxt, rSeparator, rNumberingSeparator,
                        bBefore, nId, rCharacterStyle, bCpyBrd );
    }

    SwFlyFrmFmt *const pNewFmt = lcl_InsertLabel(*this, mpTxtFmtCollTbl, pUndo,
            eType, rTxt, rSeparator, rNumberingSeparator, bBefore,
            nId, nNdIdx, rCharacterStyle, bCpyBrd);

    if (pUndo)
    {
        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }
    else
    {
        GetIDocumentUndoRedo().DelAllUndoObj();
    }

    return pNewFmt;
}

static SwFlyFrmFmt *
lcl_InsertDrawLabel( SwDoc & rDoc, SwTxtFmtColls *const pTxtFmtCollTbl,
        SwUndoInsertLabel *const pUndo, SwDrawFrmFmt *const pOldFmt,
        OUString const& rTxt,
                                     const OUString& rSeparator,
                                     const OUString& rNumberSeparator,
                                     const sal_uInt16 nId,
                                     const OUString& rCharacterStyle,
                                     SdrObject& rSdrObj )
{
    ::sw::UndoGuard const undoGuard(rDoc.GetIDocumentUndoRedo());
    ::sw::DrawUndoGuard const drawUndoGuard(rDoc.GetIDocumentUndoRedo());

    // Because we get by the TxtColl's name, we need to create the field first.
    OSL_ENSURE( nId == USHRT_MAX  || nId < rDoc.getIDocumentFieldsAccess().GetFldTypes()->size(),
            "FldType index out of bounds" );
    SwFieldType *pType = nId != USHRT_MAX ? (*rDoc.getIDocumentFieldsAccess().GetFldTypes())[nId] : 0;
    OSL_ENSURE( !pType || pType->Which() == RES_SETEXPFLD, "Wrong label id" );

    SwTxtFmtColl *pColl = NULL;
    if( pType )
    {
        for( sal_uInt16 i = pTxtFmtCollTbl->size(); i; )
        {
            if( (*pTxtFmtCollTbl)[ --i ]->GetName()==pType->GetName() )
            {
                pColl = (*pTxtFmtCollTbl)[i];
                break;
            }
        }
        OSL_ENSURE( pColl, "no text collection found" );
    }

    if( !pColl )
    {
        pColl = rDoc.getIDocumentStylePoolAccess().GetTxtCollFromPool( RES_POOLCOLL_LABEL );
    }

    SwTxtNode* pNew = NULL;
    SwFlyFrmFmt* pNewFmt = NULL;

    // Destroy Frame,
    // insert new Frame,
    // insert the corresponding Node with Field into the new Frame,
    // insert the old Frame with the Object (Picture/OLE) paragraph-bound into the new Frame,
    // create Frames.

    // Keep layer ID of drawing object before removing
    // its frames.
    // Note: The layer ID is passed to the undo and have to be the correct value.
    //       Removing the frames of the drawing object changes its layer.
    const SdrLayerID nLayerId = rSdrObj.GetLayer();

    pOldFmt->DelFrms();

    // InCntnts need to be treated in a special way:
    // The TxtAttribute needs to be destroyed.
    // Unfortunately, this also destroys the Format next to the Frames.
    // To avoid this, we disconnect the attribute from the Format.
    SfxItemSet* pNewSet = pOldFmt->GetAttrSet().Clone( false );

    // Protect the Frame's size and position
    if ( rSdrObj.IsMoveProtect() || rSdrObj.IsResizeProtect() )
    {
        SvxProtectItem aProtect(RES_PROTECT);
        aProtect.SetCntntProtect( false );
        aProtect.SetPosProtect( rSdrObj.IsMoveProtect() );
        aProtect.SetSizeProtect( rSdrObj.IsResizeProtect() );
        pNewSet->Put( aProtect );
    }

    // Take over the text wrap
    lcl_CpyAttr( *pNewSet, pOldFmt->GetAttrSet(), RES_SURROUND );

    // Send the frame to the back, if needed.
    // Consider the 'invisible' hell layer.
    if ( rDoc.getIDocumentDrawModelAccess().GetHellId() != nLayerId &&
         rDoc.getIDocumentDrawModelAccess().GetInvisibleHellId() != nLayerId )
    {
        SvxOpaqueItem aOpaque( RES_OPAQUE );
        aOpaque.SetValue( true );
        pNewSet->Put( aOpaque );
    }

    // Take over position
    // #i26791# - use directly drawing object's positioning attributes
    pNewSet->Put( pOldFmt->GetHoriOrient() );
    pNewSet->Put( pOldFmt->GetVertOrient() );

    pNewSet->Put( pOldFmt->GetAnchor() );

    // The new one should be variable in its height!
     Size aSz( rSdrObj.GetCurrentBoundRect().GetSize() );
    SwFmtFrmSize aFrmSize( ATT_MIN_SIZE, aSz.Width(), aSz.Height() );
    pNewSet->Put( aFrmSize );

    // Apply the margin to the new Frame.
    // Don't set a border, use the one from the Template.
    pNewSet->Put( pOldFmt->GetLRSpace() );
    pNewSet->Put( pOldFmt->GetULSpace() );

    SwStartNode* pSttNd =
        rDoc.GetNodes().MakeTextSection(
            SwNodeIndex( rDoc.GetNodes().GetEndOfAutotext() ),
                                    SwFlyStartNode, pColl );

    pNewFmt = rDoc.MakeFlyFrmFmt( rDoc.GetUniqueFrameName(),
                 rDoc.getIDocumentStylePoolAccess().GetFrmFmtFromPool( RES_POOLFRM_FRAME ) );

    // Set border and shadow to default if the template contains any.
    if( SfxItemState::SET == pNewFmt->GetAttrSet().GetItemState( RES_BOX, true ))
        pNewSet->Put( *GetDfltAttr( RES_BOX ) );

    if( SfxItemState::SET == pNewFmt->GetAttrSet().GetItemState(RES_SHADOW,true))
        pNewSet->Put( *GetDfltAttr( RES_SHADOW ) );

    pNewFmt->SetFmtAttr( SwFmtCntnt( pSttNd ));
    pNewFmt->SetFmtAttr( *pNewSet );

    const SwFmtAnchor& rAnchor = pNewFmt->GetAnchor();
    if ( FLY_AS_CHAR == rAnchor.GetAnchorId() )
    {
        const SwPosition *pPos = rAnchor.GetCntntAnchor();
        SwTxtNode *pTxtNode = pPos->nNode.GetNode().GetTxtNode();
        OSL_ENSURE( pTxtNode->HasHints(), "Missing FlyInCnt-Hint." );
        const sal_Int32 nIdx = pPos->nContent.GetIndex();
        SwTxtAttr * const pHnt =
            pTxtNode->GetTxtAttrForCharAt( nIdx, RES_TXTATR_FLYCNT );

#if OSL_DEBUG_LEVEL > 0
        OSL_ENSURE( pHnt && pHnt->Which() == RES_TXTATR_FLYCNT,
                    "Missing FlyInCnt-Hint." );
        OSL_ENSURE( pHnt && ((SwFmtFlyCnt&)pHnt->GetFlyCnt()).
                    GetFrmFmt() == (SwFrmFmt*)pOldFmt,
                    "Wrong TxtFlyCnt-Hint." );
#endif
        const_cast<SwFmtFlyCnt&>(pHnt->GetFlyCnt()).SetFlyFmt( pNewFmt );
    }

    // The old one should not have a flow
    // and it should be adjusted to above and middle.
    pNewSet->ClearItem();

    pNewSet->Put( SwFmtSurround( SURROUND_NONE ) );
    if (nLayerId == rDoc.getIDocumentDrawModelAccess().GetHellId())
    {
    // Consider drawing objects in the 'invisible' hell layer
        rSdrObj.SetLayer( rDoc.getIDocumentDrawModelAccess().GetHeavenId() );
    }
    else if (nLayerId == rDoc.getIDocumentDrawModelAccess().GetInvisibleHellId())
    {
        rSdrObj.SetLayer( rDoc.getIDocumentDrawModelAccess().GetInvisibleHeavenId() );
    }
    pNewSet->Put( SvxLRSpaceItem( RES_LR_SPACE ) );
    pNewSet->Put( SvxULSpaceItem( RES_UL_SPACE ) );

    // #i26791# - set position of the drawing object, which is labeled.
    pNewSet->Put( SwFmtVertOrient( 0, text::VertOrientation::TOP, text::RelOrientation::FRAME ) );
    pNewSet->Put( SwFmtHoriOrient( 0, text::HoriOrientation::CENTER, text::RelOrientation::FRAME ) );

    // The old one is paragraph-bound to the new one's paragraph.
    SwFmtAnchor aAnch( FLY_AT_PARA );
    SwNodeIndex aAnchIdx( *pNewFmt->GetCntnt().GetCntntIdx(), 1 );
    pNew = aAnchIdx.GetNode().GetTxtNode();
    SwPosition aPos( aAnchIdx );
    aAnch.SetAnchor( &aPos );
    pNewSet->Put( aAnch );

    if( pUndo )
    {
        pUndo->SetFlys( *pOldFmt, *pNewSet, *pNewFmt );
        // #i26791# - position no longer needed
        pUndo->SetDrawObj( nLayerId );
    }
    else
        pOldFmt->SetFmtAttr( *pNewSet );

    delete pNewSet;

    // Have only the FlyFrames created.
    // We leave this to established methods (especially for InCntFlys).
    pNewFmt->MakeFrms();

    OSL_ENSURE( pNew, "No Label inserted" );

    if( pNew )
    {
        //#i61007# order of captions
        bool bOrderNumberingFirst = SW_MOD()->GetModuleConfig()->IsCaptionOrderNumberingFirst();

        // prepare string
        OUString aTxt;
        if( bOrderNumberingFirst )
        {
            aTxt = rNumberSeparator;
        }
        if ( pType )
        {
            aTxt += pType->GetName();
            if( !bOrderNumberingFirst )
                aTxt += " ";
        }
        sal_Int32 nIdx = aTxt.getLength();
        aTxt += rSeparator;
        const sal_Int32 nSepIdx = aTxt.getLength();
        aTxt += rTxt;

        // insert text
        SwIndex aIdx( pNew, 0 );
        pNew->InsertText( aTxt, aIdx );

        // insert field
        if ( pType )
        {
            SwSetExpField aFld( static_cast<SwSetExpFieldType*>(pType), OUString(), SVX_NUM_ARABIC );
            if( bOrderNumberingFirst )
                nIdx = 0;
            SwFmtFld aFmt( aFld );
            pNew->InsertItem( aFmt, nIdx, nIdx );
            if ( !rCharacterStyle.isEmpty() )
            {
                SwCharFmt * pCharFmt = rDoc.FindCharFmtByName(rCharacterStyle);
                if ( !pCharFmt )
                {
                    const sal_uInt16 nMyId = SwStyleNameMapper::GetPoolIdFromUIName( rCharacterStyle, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT );
                    pCharFmt = rDoc.getIDocumentStylePoolAccess().GetCharFmtFromPool( nMyId );
                }
                if ( pCharFmt )
                {
                    SwFmtCharFmt aCharFmt( pCharFmt );
                    pNew->InsertItem( aCharFmt, 0, nSepIdx + 1,
                            nsSetAttrMode::SETATTR_DONTEXPAND );
                }
            }
        }
    }

    return pNewFmt;
}

SwFlyFrmFmt* SwDoc::InsertDrawLabel(
        OUString const& rTxt,
        OUString const& rSeparator,
        OUString const& rNumberSeparator,
        sal_uInt16 const nId,
        OUString const& rCharacterStyle,
        SdrObject& rSdrObj )
{
    SwDrawContact *const pContact =
        static_cast<SwDrawContact*>(GetUserCall( &rSdrObj ));
    OSL_ENSURE( RES_DRAWFRMFMT == pContact->GetFmt()->Which(),
            "InsertDrawLabel(): not a DrawFrmFmt" );
    if (!pContact)
        return 0;

    SwDrawFrmFmt* pOldFmt = static_cast<SwDrawFrmFmt *>(pContact->GetFmt());
    if (!pOldFmt)
        return 0;

    SwUndoInsertLabel * pUndo = 0;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().ClearRedo();
        pUndo = new SwUndoInsertLabel(
            LTYPE_DRAW, rTxt, rSeparator, rNumberSeparator, false,
            nId, rCharacterStyle, false );
    }

    SwFlyFrmFmt *const pNewFmt = lcl_InsertDrawLabel(
        *this, mpTxtFmtCollTbl, pUndo, pOldFmt,
        rTxt, rSeparator, rNumberSeparator, nId, rCharacterStyle, rSdrObj);

    if (pUndo)
    {
        GetIDocumentUndoRedo().AppendUndo( pUndo );
    }
    else
    {
        GetIDocumentUndoRedo().DelAllUndoObj();
    }

    return pNewFmt;
}

IMPL_STATIC_LINK( SwDoc, BackgroundDone, SvxBrushItem*, EMPTYARG )
{
    SwViewShell *pSh, *pStartSh;
    pSh = pStartSh = pThis->getIDocumentLayoutAccess().GetCurrentViewShell();
    if( pStartSh )
        do {
            if( pSh->GetWin() )
            {
                // Make sure to repaint with virtual device
                pSh->LockPaint();
                pSh->UnlockPaint( true );
            }
            pSh = static_cast<SwViewShell*>(pSh->GetNext());
        } while( pSh != pStartSh );
    return 0;
}

static OUString lcl_GetUniqueFlyName( const SwDoc* pDoc, sal_uInt16 nDefStrId )
{
    if( pDoc->IsInMailMerge())
    {
        OUString newName = "MailMergeFly"
            + OStringToOUString( DateTimeToOString( DateTime( DateTime::SYSTEM )), RTL_TEXTENCODING_ASCII_US )
            + OUString::number( pDoc->GetSpzFrmFmts()->size() + 1 );
        return newName;
    }

    ResId aId( nDefStrId, *pSwResMgr );
    OUString aName( aId );
    sal_Int32 nNmLen = aName.getLength();

    const SwFrmFmts& rFmts = *pDoc->GetSpzFrmFmts();

    sal_uInt16 nNum, nTmp, nFlagSize = ( rFmts.size() / 8 ) +2;
    sal_uInt8* pSetFlags = new sal_uInt8[ nFlagSize ];
    sal_uInt16 n;

    memset( pSetFlags, 0, nFlagSize );

    for( n = 0; n < rFmts.size(); ++n )
    {
        const SwFrmFmt* pFlyFmt = rFmts[ n ];
        if( RES_FLYFRMFMT == pFlyFmt->Which() &&
            pFlyFmt->GetName().startsWith( aName ) )
        {
            // Only get and set the Flag
            nNum = static_cast< sal_uInt16 >( pFlyFmt->GetName().copy( nNmLen ).toInt32() );
            if( nNum-- && nNum < rFmts.size() )
                pSetFlags[ nNum / 8 ] |= (0x01 << ( nNum & 0x07 ));
        }
    }

    // All numbers are flagged accordingly, so determine the right one
    nNum = rFmts.size();
    for( n = 0; n < nFlagSize; ++n )
        if( 0xff != ( nTmp = pSetFlags[ n ] ))
        {
            // so determine the number
            nNum = n * 8;
            while( nTmp & 1 )
                ++nNum, nTmp >>= 1;
            break;
        }

    delete [] pSetFlags;
    return aName += OUString::number( ++nNum );
}

OUString SwDoc::GetUniqueGrfName() const
{
    return lcl_GetUniqueFlyName( this, STR_GRAPHIC_DEFNAME );
}

OUString SwDoc::GetUniqueOLEName() const
{
    return lcl_GetUniqueFlyName( this, STR_OBJECT_DEFNAME );
}

OUString SwDoc::GetUniqueFrameName() const
{
    return lcl_GetUniqueFlyName( this, STR_FRAME_DEFNAME );
}

const SwFlyFrmFmt* SwDoc::FindFlyByName( const OUString& rName, sal_Int8 nNdTyp ) const
{
    const SwFrmFmts& rFmts = *GetSpzFrmFmts();
    for( sal_uInt16 n = rFmts.size(); n; )
    {
        const SwFrmFmt* pFlyFmt = rFmts[ --n ];
        const SwNodeIndex* pIdx = 0;
        if( RES_FLYFRMFMT == pFlyFmt->Which() && pFlyFmt->GetName() == rName &&
            0 != ( pIdx = pFlyFmt->GetCntnt().GetCntntIdx() ) &&
            pIdx->GetNode().GetNodes().IsDocNodes() )
        {
            if( nNdTyp )
            {
                // query for the right NodeType
                const SwNode* pNd = GetNodes()[ pIdx->GetIndex()+1 ];
                if( nNdTyp == ND_TEXTNODE
                        ? !pNd->IsNoTxtNode()
                        : nNdTyp == pNd->GetNodeType() )
                    return static_cast<const SwFlyFrmFmt*>(pFlyFmt);
            }
            else
                return static_cast<const SwFlyFrmFmt*>(pFlyFmt);
        }
    }
    return 0;
}

void SwDoc::SetFlyName( SwFlyFrmFmt& rFmt, const OUString& rName )
{
    OUString sName( rName );
    if( sName.isEmpty() || FindFlyByName( sName ) )
    {
        sal_uInt16 nTyp = STR_FRAME_DEFNAME;
        const SwNodeIndex* pIdx = rFmt.GetCntnt().GetCntntIdx();
        if( pIdx && pIdx->GetNode().GetNodes().IsDocNodes() )
            switch( GetNodes()[ pIdx->GetIndex() + 1 ]->GetNodeType() )
            {
            case ND_GRFNODE:    nTyp = STR_GRAPHIC_DEFNAME; break;
            case ND_OLENODE:    nTyp = STR_OBJECT_DEFNAME;  break;
            }
        sName = lcl_GetUniqueFlyName( this, nTyp );
    }
    rFmt.SetName( sName, true );
    getIDocumentState().SetModified();
}

void SwDoc::SetAllUniqueFlyNames()
{
    sal_Int32 n, nFlyNum = 0, nGrfNum = 0, nOLENum = 0;

    ResId nFrmId( STR_FRAME_DEFNAME, *pSwResMgr ),
          nGrfId( STR_GRAPHIC_DEFNAME, *pSwResMgr ),
          nOLEId( STR_OBJECT_DEFNAME, *pSwResMgr );
    OUString sFlyNm( nFrmId );
    OUString sGrfNm( nGrfId );
    OUString sOLENm( nOLEId );

    if( 255 < ( n = GetSpzFrmFmts()->size() ))
        n = 255;
    SwFrmFmts aArr;
    aArr.reserve( n );
    SwFrmFmt* pFlyFmt;
    bool bContainsAtPageObjWithContentAnchor = false;

    for( n = GetSpzFrmFmts()->size(); n; )
    {
        if( RES_FLYFRMFMT == (pFlyFmt = (*GetSpzFrmFmts())[ --n ])->Which() )
        {
            sal_Int32 *pNum = 0;
            const OUString aNm = pFlyFmt->GetName();
            if ( !aNm.isEmpty() )
            {
                sal_Int32 nLen = 0;
                if ( aNm.startsWith(sGrfNm) )
                {
                    nLen = sGrfNm.getLength();
                    pNum = &nGrfNum;
                }
                else if( aNm.startsWith(sFlyNm) )
                {
                    nLen = sFlyNm.getLength();
                    pNum = &nFlyNum;
                }
                else if( aNm.startsWith(sOLENm) )
                {
                    nLen = sOLENm.getLength();
                    pNum = &nOLENum;
                }

                if ( pNum )
                {
                    const sal_Int32 nNewLen = aNm.copy( nLen ).toInt32();
                    if (*pNum < nNewLen)
                        *pNum = nNewLen;
                }
            }
            else
                // we want to set that afterwards
                aArr.push_back( pFlyFmt );

        }
        if ( !bContainsAtPageObjWithContentAnchor )
        {
            const SwFmtAnchor& rAnchor = pFlyFmt->GetAnchor();
            if ( (FLY_AT_PAGE == rAnchor.GetAnchorId()) &&
                 rAnchor.GetCntntAnchor() )
            {
                bContainsAtPageObjWithContentAnchor = true;
            }
        }
    }
    SetContainsAtPageObjWithContentAnchor( bContainsAtPageObjWithContentAnchor );

    const SwNodeIndex* pIdx;

    for( n = aArr.size(); n; )
        if( 0 != ( pIdx = ( pFlyFmt = aArr[ --n ])->GetCntnt().GetCntntIdx() )
            && pIdx->GetNode().GetNodes().IsDocNodes() )
        {
            sal_uInt16 nNum;
            OUString sNm;
            switch( GetNodes()[ pIdx->GetIndex() + 1 ]->GetNodeType() )
            {
            case ND_GRFNODE:
                sNm = sGrfNm;
                nNum = ++nGrfNum;
                break;
            case ND_OLENODE:
                sNm = sOLENm;
                nNum = ++nOLENum;
                break;
            default:
                sNm = sFlyNm;
                nNum = ++nFlyNum;
                break;
            }
            pFlyFmt->SetName( sNm + OUString::number( nNum ));
        }
    aArr.clear();

    if( !GetFtnIdxs().empty() )
    {
        SwTxtFtn::SetUniqueSeqRefNo( *this );
        // #i52775# Chapter footnotes did not get updated correctly.
        // Calling UpdateAllFtn() instead of UpdateFtn() solves this problem,
        // but I do not dare to call UpdateAllFtn() in all cases: Safety first.
        if ( FTNNUM_CHAPTER == GetFtnInfo().eNum )
        {
            GetFtnIdxs().UpdateAllFtn();
        }
        else
        {
            SwNodeIndex aTmp( GetNodes() );
            GetFtnIdxs().UpdateFtn( aTmp );
        }
    }
}

bool SwDoc::IsInHeaderFooter( const SwNodeIndex& rIdx ) const
{
    // That can also be a Fly in a Fly in the Header.
    // Is also used by sw3io, to determine if a Redline object is
    // in the Header or Footer.
    // Because Redlines are also attached to Start and EndNoden,
    // the Index must not necessarily be from a ContentNode.
    SwNode* pNd = &rIdx.GetNode();
    const SwNode* pFlyNd = pNd->FindFlyStartNode();
    while( pFlyNd )
    {
        // get up by using the Anchor
#if OSL_DEBUG_LEVEL > 0
        std::list<const SwFrmFmt*> checkFmts;
        sal_uInt16 n;
        for( n = 0; n < GetSpzFrmFmts()->size(); ++n )
        {
            const SwFrmFmt* pFmt = (*GetSpzFrmFmts())[ n ];
            const SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
            if( pIdx && pFlyNd == &pIdx->GetNode() )
                checkFmts.push_back( pFmt );
        }
#endif
        SwFrmFmtAnchorMap::const_iterator_pair range = GetFrmFmtAnchorMap()->equal_range( SwNodeIndex( *pFlyNd ));
        SwFrmFmtAnchorMap::const_iterator it;
        for( it = range.first;
             it != range.second;
             ++it )
        {
            const SwFrmFmt* pFmt = it->second;
            const SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
            if( pIdx && pFlyNd == &pIdx->GetNode() )
            {
#if OSL_DEBUG_LEVEL > 0
                std::list<const SwFrmFmt*>::iterator checkPos = std::find( checkFmts.begin(), checkFmts.end(), pFmt );
                assert( checkPos != checkFmts.end());
                checkFmts.erase( checkPos );
#endif
                const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
                if ((FLY_AT_PAGE == rAnchor.GetAnchorId()) ||
                    !rAnchor.GetCntntAnchor() )
                {
                    return false;
                }

                pNd = &rAnchor.GetCntntAnchor()->nNode.GetNode();
                pFlyNd = pNd->FindFlyStartNode();
                break;
            }
        }
        if( it == range.second )
        {
            OSL_ENSURE( mbInReading, "Found a FlySection but not a Format!" );
            return false;
        }
    }

    return 0 != pNd->FindHeaderStartNode() ||
            0 != pNd->FindFooterStartNode();
}

short SwDoc::GetTextDirection( const SwPosition& rPos,
                               const Point* pPt ) const
{
    short nRet = -1;

    SwCntntNode *pNd = rPos.nNode.GetNode().GetCntntNode();

    // #i42921# - use new method <SwCntntNode::GetTextDirection(..)>
    if ( pNd )
    {
        nRet = pNd->GetTextDirection( rPos, pPt );
    }
    if ( nRet == -1 )
    {
        const SvxFrameDirectionItem* pItem = 0;
        if( pNd )
        {
            // Are we in a FlyFrame? Then look at that for the correct attribute
            const SwFrmFmt* pFlyFmt = pNd->GetFlyFmt();
            while( pFlyFmt )
            {
                pItem = &pFlyFmt->GetFrmDir();
                if( FRMDIR_ENVIRONMENT == pItem->GetValue() )
                {
                    pItem = 0;
                    const SwFmtAnchor* pAnchor = &pFlyFmt->GetAnchor();
                    if ((FLY_AT_PAGE != pAnchor->GetAnchorId()) &&
                        pAnchor->GetCntntAnchor())
                    {
                        pFlyFmt = pAnchor->GetCntntAnchor()->nNode.
                                            GetNode().GetFlyFmt();
                    }
                    else
                        pFlyFmt = 0;
                }
                else
                    pFlyFmt = 0;
            }

            if( !pItem )
            {
                const SwPageDesc* pPgDsc = pNd->FindPageDesc( false );
                if( pPgDsc )
                    pItem = &pPgDsc->GetMaster().GetFrmDir();
            }
        }
        if( !pItem )
            pItem = static_cast<const SvxFrameDirectionItem*>(&GetAttrPool().GetDefaultItem(
                                                            RES_FRAMEDIR ));
        nRet = pItem->GetValue();
    }
    return nRet;
}

bool SwDoc::IsInVerticalText( const SwPosition& rPos, const Point* pPt ) const
{
    const short nDir = GetTextDirection( rPos, pPt );
    return FRMDIR_VERT_TOP_RIGHT == nDir || FRMDIR_VERT_TOP_LEFT == nDir;
}

std::set<SwRootFrm*> SwDoc::GetAllLayouts()
{
    std::set<SwRootFrm*> aAllLayouts;
    SwViewShell *pStart = getIDocumentLayoutAccess().GetCurrentViewShell();
    SwViewShell *pTemp = pStart;
    if ( pTemp )
    {
        do
        {
            if (pTemp->GetLayout())
            {
                aAllLayouts.insert(pTemp->GetLayout());
                pTemp = static_cast<SwViewShell*>(pTemp->GetNext());
            }
        } while(pTemp!=pStart);
    }

    return aAllLayouts;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
