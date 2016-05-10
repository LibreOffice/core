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

#define DEF_FLY_WIDTH    2268   // Default width for FlyFrames (2268 == 4cm)

static bool lcl_IsItemSet(const SwContentNode & rNode, sal_uInt16 which)
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
        const OUString sName("Name");
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
    if ( dynamic_cast<const SwFlyDrawObj*>( pObj) ==  nullptr &&
         dynamic_cast<const SwVirtFlyDrawObj*>( pObj) ==  nullptr &&
         typeid(SdrObject) != typeid(pObj) )
    {
        if ( getIDocumentDrawModelAccess().IsVisibleLayerId( nLayerIdForClone ) )
        {
            nLayerIdForClone = getIDocumentDrawModelAccess().GetInvisibleLayerIdByVisibleOne( nLayerIdForClone );
        }
    }
    pObj->SetLayer( nLayerIdForClone );

    return pObj;
}

SwFlyFrameFormat* SwDoc::_MakeFlySection( const SwPosition& rAnchPos,
                                    const SwContentNode& rNode,
                                    RndStdIds eRequestId,
                                    const SfxItemSet* pFlySet,
                                    SwFrameFormat* pFrameFormat )
{
    if( !pFrameFormat )
        pFrameFormat = getIDocumentStylePoolAccess().GetFrameFormatFromPool( RES_POOLFRM_FRAME );

    OUString sName;
    if( !mbInReading )
        switch( rNode.GetNodeType() )
        {
        case ND_GRFNODE:        sName = GetUniqueGrfName();     break;
        case ND_OLENODE:        sName = GetUniqueOLEName();     break;
        default:                sName = GetUniqueFrameName();   break;
        }
    SwFlyFrameFormat* pFormat = MakeFlyFrameFormat( sName, pFrameFormat );

    // Create content and connect to the format.
    // Create ContentNode and put it into the autotext selection.
    SwNodeRange aRange( GetNodes().GetEndOfAutotext(), -1,
                        GetNodes().GetEndOfAutotext() );
    GetNodes().SectionDown( &aRange, SwFlyStartNode );

    pFormat->SetFormatAttr( SwFormatContent( rNode.StartOfSectionNode() ));

    const SwFormatAnchor* pAnchor = nullptr;
    if( pFlySet )
    {
        pFlySet->GetItemState( RES_ANCHOR, false,
                                reinterpret_cast<const SfxPoolItem**>(&pAnchor) );
        if( SfxItemState::SET == pFlySet->GetItemState( RES_CNTNT, false ))
        {
            SfxItemSet aTmpSet( *pFlySet );
            aTmpSet.ClearItem( RES_CNTNT );
            pFormat->SetFormatAttr( aTmpSet );
        }
        else
            pFormat->SetFormatAttr( *pFlySet );
    }

    // Anchor not yet set?
    RndStdIds eAnchorId;
    // #i107811# Assure that at-page anchored fly frames have a page num or a
    // content anchor set.
    if ( !pAnchor ||
         ( FLY_AT_PAGE != pAnchor->GetAnchorId() &&
           !pAnchor->GetContentAnchor() ) ||
         ( FLY_AT_PAGE == pAnchor->GetAnchorId() &&
           !pAnchor->GetContentAnchor() &&
           pAnchor->GetPageNum() == 0 ) )
    {
        // set it again, needed for Undo
        SwFormatAnchor aAnch( pFormat->GetAnchor() );
        if (pAnchor && (FLY_AT_FLY == pAnchor->GetAnchorId()))
        {
            SwPosition aPos( *rAnchPos.nNode.GetNode().FindFlyStartNode() );
            aAnch.SetAnchor( &aPos );
            eAnchorId = FLY_AT_FLY;
        }
        else
        {
            if( eRequestId != aAnch.GetAnchorId() &&
                SfxItemState::SET != pFormat->GetItemState( RES_ANCHOR ) )
            {
                aAnch.SetType( eRequestId );
            }

            eAnchorId = aAnch.GetAnchorId();
            if ( FLY_AT_PAGE != eAnchorId || !pAnchor || aAnch.GetPageNum() == 0)
            {
                aAnch.SetAnchor( &rAnchPos );
            }
        }
        pFormat->SetFormatAttr( aAnch );
    }
    else
        eAnchorId = pFormat->GetAnchor().GetAnchorId();

    if ( FLY_AS_CHAR == eAnchorId )
    {
        const sal_Int32 nStt = rAnchPos.nContent.GetIndex();
        SwTextNode * pTextNode = rAnchPos.nNode.GetNode().GetTextNode();

        OSL_ENSURE(pTextNode!= nullptr, "There should be a SwTextNode!");

        if (pTextNode != nullptr)
        {
            SwFormatFlyCnt aFormat( pFormat );
            // may fail if there's no space left or header/ftr
            if (!pTextNode->InsertItem(aFormat, nStt, nStt))
            {   // pFormat is dead now
                return nullptr;
            }
        }
    }

    if( SfxItemState::SET != pFormat->GetAttrSet().GetItemState( RES_FRM_SIZE ))
    {
        SwFormatFrameSize aFormatSize( ATT_VAR_SIZE, 0, DEF_FLY_WIDTH );
        const SwNoTextNode* pNoTextNode = rNode.GetNoTextNode();
        if( pNoTextNode )
        {
            // Set size
             Size aSize( pNoTextNode->GetTwipSize() );
            if( MINFLY > aSize.Width() )
                aSize.Width() = DEF_FLY_WIDTH;
            aFormatSize.SetWidth( aSize.Width() );
            if( aSize.Height() )
            {
                aFormatSize.SetHeight( aSize.Height() );
                aFormatSize.SetHeightSizeType( ATT_FIX_SIZE );
            }
        }
        pFormat->SetFormatAttr( aFormatSize );
    }

    // Set up frames
    if( getIDocumentLayoutAccess().GetCurrentViewShell() )
        pFormat->MakeFrames();           // ???

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        sal_uLong nNodeIdx = rAnchPos.nNode.GetIndex();
        const sal_Int32 nCntIdx = rAnchPos.nContent.GetIndex();
        GetIDocumentUndoRedo().AppendUndo(
            new SwUndoInsLayFormat( pFormat, nNodeIdx, nCntIdx ));
    }

    getIDocumentState().SetModified();
    return pFormat;
}

SwFlyFrameFormat* SwDoc::MakeFlySection( RndStdIds eAnchorType,
                                    const SwPosition* pAnchorPos,
                                    const SfxItemSet* pFlySet,
                                    SwFrameFormat* pFrameFormat, bool bCalledFromShell )
{
    SwFlyFrameFormat* pFormat = nullptr;
    if ( !pAnchorPos && (FLY_AT_PAGE != eAnchorType) )
    {
        const SwFormatAnchor* pAnch;
        if( (pFlySet && SfxItemState::SET == pFlySet->GetItemState(
                RES_ANCHOR, false, reinterpret_cast<const SfxPoolItem**>(&pAnch) )) ||
            ( pFrameFormat && SfxItemState::SET == pFrameFormat->GetItemState(
                RES_ANCHOR, true, reinterpret_cast<const SfxPoolItem**>(&pAnch) )) )
        {
            if ( (FLY_AT_PAGE != pAnch->GetAnchorId()) )
            {
                pAnchorPos = pAnch->GetContentAnchor();
            }
        }
    }

    if (pAnchorPos)
    {
        if( !pFrameFormat )
            pFrameFormat = getIDocumentStylePoolAccess().GetFrameFormatFromPool( RES_POOLFRM_FRAME );

        sal_uInt16 nCollId = static_cast<sal_uInt16>(
            GetDocumentSettingManager().get(DocumentSettingId::HTML_MODE) ? RES_POOLCOLL_TEXT : RES_POOLCOLL_FRAME );

        /* If there is no adjust item in the paragraph style for the content node of the new fly section
           propagate an existing adjust item at the anchor to the new content node. */
        SwContentNode * pNewTextNd = GetNodes().MakeTextNode
            (SwNodeIndex( GetNodes().GetEndOfAutotext()),
             getIDocumentStylePoolAccess().GetTextCollFromPool( nCollId ));
        SwContentNode * pAnchorNode = pAnchorPos->nNode.GetNode().GetContentNode();
        assert(pAnchorNode); // pAnchorNode from cursor, must be valid

        const SfxPoolItem * pItem = nullptr;

        if (bCalledFromShell && !lcl_IsItemSet(*pNewTextNd, RES_PARATR_ADJUST) &&
            SfxItemState::SET == pAnchorNode->GetSwAttrSet().
            GetItemState(RES_PARATR_ADJUST, true, &pItem))
        {
            static_cast<SwContentNode *>(pNewTextNd)->SetAttr(*pItem);
        }

         pFormat = _MakeFlySection( *pAnchorPos, *pNewTextNd,
                                eAnchorType, pFlySet, pFrameFormat );
    }
    return pFormat;
}

SwFlyFrameFormat* SwDoc::MakeFlyAndMove( const SwPaM& rPam, const SfxItemSet& rSet,
                                    const SwSelBoxes* pSelBoxes,
                                    SwFrameFormat *pParent )
{
    const SwFormatAnchor& rAnch = static_cast<const SwFormatAnchor&>(rSet.Get( RES_ANCHOR ));

    GetIDocumentUndoRedo().StartUndo( UNDO_INSLAYFMT, nullptr );

    SwFlyFrameFormat* pFormat = MakeFlySection( rAnch.GetAnchorId(), rPam.GetPoint(),
                                        &rSet, pParent );

    // If content is selected, it becomes the new frame's content.
    // Namely, it is moved into the NodeArray's appropriate section.

    if( pFormat )
    {
        do {        // middle check loop
            const SwFormatContent &rContent = pFormat->GetContent();
            OSL_ENSURE( rContent.GetContentIdx(), "No content prepared." );
            SwNodeIndex aIndex( *(rContent.GetContentIdx()), 1 );
            SwContentNode *pNode = aIndex.GetNode().GetContentNode();

            // Attention: Do not create an index on the stack, or we
            // cannot delete ContentNode in the end!
            SwPosition aPos( aIndex );
            aPos.nContent.Assign( pNode, 0 );

            if( pSelBoxes && !pSelBoxes->empty() )
            {
                // Table selection
                // Copy parts of a table: create a table with the same width as the
                // original one and move (copy and delete) the selected boxes.
                // The size is corrected on a percentage basis.

                SwTableNode* pTableNd = const_cast<SwTableNode*>((*pSelBoxes)[0]->
                                                GetSttNd()->FindTableNode());
                if( !pTableNd )
                    break;

                SwTable& rTable = pTableNd->GetTable();

                // Did we select the whole table?
                if( pSelBoxes->size() == rTable.GetTabSortBoxes().size() )
                {
                    // move the whole table
                    SwNodeRange aRg( *pTableNd, 0, *pTableNd->EndOfSectionNode(), 1 );

                    // If we move the whole table and it is located within a
                    // FlyFrame, the we create a TextNode after it.
                    // So that this FlyFrame is preserved.
                    if( aRg.aEnd.GetNode().IsEndNode() )
                        GetNodes().MakeTextNode( aRg.aStart,
                                    GetDfltTextFormatColl() );

                    getIDocumentContentOperations().MoveNodeRange( aRg, aPos.nNode, SwMoveFlags::DEFAULT );
                }
                else
                {
                    rTable.MakeCopy( this, aPos, *pSelBoxes );
                    // Don't delete a part of a table with row span!!
                    // You could delete the content instead -> ToDo
                    //rTable.DeleteSel( this, *pSelBoxes, 0, 0, true, true );
                }

                // If the table is within the frame, then copy without the following TextNode
                aIndex = rContent.GetContentIdx()->GetNode().EndOfSectionIndex() - 1;
                OSL_ENSURE( aIndex.GetNode().GetTextNode(),
                        "a TextNode should be here" );
                aPos.nContent.Assign( nullptr, 0 );       // Deregister index!
                GetNodes().Delete( aIndex );

                // This is a hack: whilst FlyFrames/Headers/Footers are not undoable we delete all Undo objects
                if( GetIDocumentUndoRedo().DoesUndo() )
                {
                    GetIDocumentUndoRedo().DelAllUndoObj();
                }
            }
            else
            {
                // copy all Pams and then delete all
                bool bOldFlag = mbCopyIsMove;
                bool const bOldUndo = GetIDocumentUndoRedo().DoesUndo();
                bool const bOldRedlineMove(getIDocumentRedlineAccess().IsRedlineMove());
                mbCopyIsMove = true;
                GetIDocumentUndoRedo().DoUndo(false);
                getIDocumentRedlineAccess().SetRedlineMove(true);
                for(const SwPaM& rTmp : rPam.GetRingContainer())
                {
                    if( rTmp.HasMark() &&
                        *rTmp.GetPoint() != *rTmp.GetMark() )
                    {
                        // aPos is the newly created fly section, so definitely outside rPam, it's pointless to check that again.
                        getIDocumentContentOperations().CopyRange( *const_cast<SwPaM*>(&rTmp), aPos, /*bCopyAll=*/false, /*bCheckPos=*/false );
                    }
                }
                getIDocumentRedlineAccess().SetRedlineMove(bOldRedlineMove);
                mbCopyIsMove = bOldFlag;
                GetIDocumentUndoRedo().DoUndo(bOldUndo);

                for(const SwPaM& rTmp : rPam.GetRingContainer())
                {
                    if( rTmp.HasMark() &&
                        *rTmp.GetPoint() != *rTmp.GetMark() )
                    {
                        getIDocumentContentOperations().DeleteAndJoin( *const_cast<SwPaM*>(&rTmp) );
                    }
                }
            }
        } while( false );
    }

    getIDocumentState().SetModified();

    GetIDocumentUndoRedo().EndUndo( UNDO_INSLAYFMT, nullptr );

    return pFormat;
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

SwPosFlyFrames SwDoc::GetAllFlyFormats( const SwPaM* pCmpRange, bool bDrawAlso,
                           bool bAsCharAlso ) const
{
    SwPosFlyFrames aRetval;

    // collect all anchored somehow to paragraphs
    for( auto pFly : *GetSpzFrameFormats() )
    {
        bool bDrawFormat = bDrawAlso && RES_DRAWFRMFMT == pFly->Which();
        bool bFlyFormat = RES_FLYFRMFMT == pFly->Which();
        if( bFlyFormat || bDrawFormat )
        {
            const SwFormatAnchor& rAnchor = pFly->GetAnchor();
            SwPosition const*const pAPos = rAnchor.GetContentAnchor();
            if (pAPos &&
                ((FLY_AT_PARA == rAnchor.GetAnchorId()) ||
                 (FLY_AT_FLY  == rAnchor.GetAnchorId()) ||
                 (FLY_AT_CHAR == rAnchor.GetAnchorId()) ||
                 ((FLY_AS_CHAR == rAnchor.GetAnchorId()) && bAsCharAlso)))
            {
                if( pCmpRange &&
                    !lcl_TstFlyRange( pCmpRange, pAPos, rAnchor.GetAnchorId() ))
                        continue;       // not a valid FlyFrame
                aRetval.insert(SwPosFlyFramePtr(new SwPosFlyFrame(pAPos->nNode, pFly, aRetval.size())));
            }
        }
    }

    // If we don't have a layout we can't get page anchored FlyFrames.
    // Also, page anchored FlyFrames are only returned if no range is specified.
    if( !getIDocumentLayoutAccess().GetCurrentViewShell() || pCmpRange )
    {
        return aRetval;
    }

    const SwPageFrame *pPage = static_cast<const SwPageFrame*>(getIDocumentLayoutAccess().GetCurrentLayout()->GetLower());
    while( pPage )
    {
        if( pPage->GetSortedObjs() )
        {
            const SwSortedObjs &rObjs = *pPage->GetSortedObjs();
            for( size_t i = 0; i < rObjs.size(); ++i)
            {
                SwAnchoredObject* pAnchoredObj = rObjs[i];
                SwFrameFormat *pFly;
                if ( dynamic_cast<const SwFlyFrame*>( pAnchoredObj) !=  nullptr )
                    pFly = &(pAnchoredObj->GetFrameFormat());
                else if ( bDrawAlso )
                    pFly = &(pAnchoredObj->GetFrameFormat());
                else
                    continue;

                const SwFormatAnchor& rAnchor = pFly->GetAnchor();
                if ((FLY_AT_PARA != rAnchor.GetAnchorId()) &&
                    (FLY_AT_FLY  != rAnchor.GetAnchorId()) &&
                    (FLY_AT_CHAR != rAnchor.GetAnchorId()))
                {
                    const SwContentFrame * pContentFrame = pPage->FindFirstBodyContent();
                    if ( !pContentFrame )
                    {
                        // Oops! An empty page.
                        // In order not to lose the whole frame (RTF) we
                        // look for the last Content before the page.
                        const SwPageFrame *pPrv = static_cast<const SwPageFrame*>(pPage->GetPrev());
                        while ( !pContentFrame && pPrv )
                        {
                            pContentFrame = pPrv->FindFirstBodyContent();
                            pPrv = static_cast<const SwPageFrame*>(pPrv->GetPrev());
                        }
                    }
                    if ( pContentFrame )
                    {
                        SwNodeIndex aIdx( *pContentFrame->GetNode() );
                        aRetval.insert(SwPosFlyFramePtr(new SwPosFlyFrame(aIdx, pFly, aRetval.size())));
                    }
                }
            }
        }
        pPage = static_cast<const SwPageFrame*>(pPage->GetNext());
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
    const SfxPoolItem *pOldItem = nullptr;

    rOldSet.GetItemState( nWhich, false, &pOldItem);
    if (pOldItem != nullptr)
        rNewSet.Put( *pOldItem );
    else
    {
        pOldItem = rOldSet.GetItem( nWhich );
        if (pOldItem != nullptr)
        {
            const SfxPoolItem *pNewItem = rNewSet.GetItem( nWhich );
            if (pNewItem != nullptr)
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

static SwFlyFrameFormat *
lcl_InsertLabel(SwDoc & rDoc, SwTextFormatColls *const pTextFormatCollTable,
        SwUndoInsertLabel *const pUndo,
        SwLabelType const eType, OUString const& rText, OUString const& rSeparator,
            const OUString& rNumberingSeparator,
            const bool bBefore, const sal_uInt16 nId, const sal_uLong nNdIdx,
            const OUString& rCharacterStyle,
            const bool bCpyBrd )
{
    ::sw::UndoGuard const undoGuard(rDoc.GetIDocumentUndoRedo());

    bool bTable = false;    // To save some code.

    // Get the field first, because we retrieve the TextColl via the field's name
    OSL_ENSURE( nId == USHRT_MAX  || nId < rDoc.getIDocumentFieldsAccess().GetFieldTypes()->size(),
            "FieldType index out of bounds." );
    SwFieldType *pType = (nId != USHRT_MAX) ? (*rDoc.getIDocumentFieldsAccess().GetFieldTypes())[nId] : nullptr;
    OSL_ENSURE(!pType || pType->Which() == RES_SETEXPFLD, "wrong Id for Label");

    SwTextFormatColl * pColl = nullptr;
    if( pType )
    {
        for( auto i = pTextFormatCollTable->size(); i; )
        {
            if( (*pTextFormatCollTable)[ --i ]->GetName()==pType->GetName() )
            {
                pColl = (*pTextFormatCollTable)[i];
                break;
            }
        }
        OSL_ENSURE( pColl, "no text collection found" );
    }

    if( !pColl )
    {
        pColl = rDoc.getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_LABEL );
    }

    SwTextNode *pNew = nullptr;
    SwFlyFrameFormat* pNewFormat = nullptr;

    switch ( eType )
    {
        case LTYPE_TABLE:
            bTable = true;
            SAL_FALLTHROUGH;
        case LTYPE_FLY:
            // At the FlySection's Beginning/End insert the corresponding Node with its Field.
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
                pNew = rDoc.GetNodes().MakeTextNode( aIdx, pColl );
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
                SwFrameFormat *pOldFormat = rDoc.GetNodes()[nNdIdx]->GetFlyFormat();
                OSL_ENSURE( pOldFormat, "Couldn't find the Fly's Format." );
                // #i115719#
                // <title> and <description> attributes are lost when calling <DelFrames()>.
                // Thus, keep them and restore them after the calling <MakeFrames()>
                const bool bIsSwFlyFrameFormatInstance( dynamic_cast<SwFlyFrameFormat*>(pOldFormat) != nullptr );
                const OUString sTitle( bIsSwFlyFrameFormatInstance
                                     ? static_cast<SwFlyFrameFormat*>(pOldFormat)->GetObjTitle()
                                     : OUString() );
                const OUString sDescription( bIsSwFlyFrameFormatInstance
                                           ? static_cast<SwFlyFrameFormat*>(pOldFormat)->GetObjDescription()
                                           : OUString() );
                pOldFormat->DelFrames();

                pNewFormat = rDoc.MakeFlyFrameFormat( rDoc.GetUniqueFrameName(),
                                rDoc.getIDocumentStylePoolAccess().GetFrameFormatFromPool(RES_POOLFRM_FRAME) );

                /* #i6447#: Only the selected items are copied from the old
                   format. */
                SfxItemSet* pNewSet = pNewFormat->GetAttrSet().Clone();

                // Copy only the set attributes.
                // The others should apply from the Templates.
                lcl_CpyAttr( *pNewSet, pOldFormat->GetAttrSet(), RES_PRINT );
                lcl_CpyAttr( *pNewSet, pOldFormat->GetAttrSet(), RES_OPAQUE );
                lcl_CpyAttr( *pNewSet, pOldFormat->GetAttrSet(), RES_PROTECT );
                lcl_CpyAttr( *pNewSet, pOldFormat->GetAttrSet(), RES_SURROUND );
                lcl_CpyAttr( *pNewSet, pOldFormat->GetAttrSet(), RES_VERT_ORIENT );
                lcl_CpyAttr( *pNewSet, pOldFormat->GetAttrSet(), RES_HORI_ORIENT );
                lcl_CpyAttr( *pNewSet, pOldFormat->GetAttrSet(), RES_LR_SPACE );
                lcl_CpyAttr( *pNewSet, pOldFormat->GetAttrSet(), RES_UL_SPACE );
                lcl_CpyAttr( *pNewSet, pOldFormat->GetAttrSet(), RES_BACKGROUND );
                if( bCpyBrd )
                {
                    // If there's no BoxItem at graphic, but the new Format has one, then set the
                    // default item in the new Set. Because the graphic's size has never changed!
                    const SfxPoolItem *pItem;
                    if( SfxItemState::SET == pOldFormat->GetAttrSet().
                            GetItemState( RES_BOX, true, &pItem ))
                        pNewSet->Put( *pItem );
                    else if( SfxItemState::SET == pNewFormat->GetAttrSet().
                            GetItemState( RES_BOX ))
                        pNewSet->Put( *GetDfltAttr( RES_BOX ) );

                    if( SfxItemState::SET == pOldFormat->GetAttrSet().
                            GetItemState( RES_SHADOW, true, &pItem ))
                        pNewSet->Put( *pItem );
                    else if( SfxItemState::SET == pNewFormat->GetAttrSet().
                            GetItemState( RES_SHADOW ))
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
                pNewSet->Put( pOldFormat->GetAnchor() );

                // The new one should be changeable in its height.
                SwFormatFrameSize aFrameSize( pOldFormat->GetFrameSize() );
                aFrameSize.SetHeightSizeType( ATT_MIN_SIZE );
                pNewSet->Put( aFrameSize );

                SwStartNode* pSttNd = rDoc.GetNodes().MakeTextSection(
                            SwNodeIndex( rDoc.GetNodes().GetEndOfAutotext() ),
                            SwFlyStartNode, pColl );
                pNewSet->Put( SwFormatContent( pSttNd ));

                pNewFormat->SetFormatAttr( *pNewSet );

                // InContents need to be treated in a special way:
                // The TextAttribute needs to be destroyed.
                // Unfortunately, this also destroys the Format next to the Frames.
                // To avoid this, we disconnect the attribute from the Format.

                const SwFormatAnchor& rAnchor = pNewFormat->GetAnchor();
                if ( FLY_AS_CHAR == rAnchor.GetAnchorId() )
                {
                    const SwPosition *pPos = rAnchor.GetContentAnchor();
                    SwTextNode *pTextNode = pPos->nNode.GetNode().GetTextNode();
                    OSL_ENSURE( pTextNode->HasHints(), "Missing FlyInCnt-Hint." );
                    const sal_Int32 nIdx = pPos->nContent.GetIndex();
                    SwTextAttr * const pHint =
                        pTextNode->GetTextAttrForCharAt(nIdx, RES_TXTATR_FLYCNT);

                    OSL_ENSURE( pHint && pHint->Which() == RES_TXTATR_FLYCNT,
                                "Missing FlyInCnt-Hint." );
                    OSL_ENSURE( pHint && pHint->GetFlyCnt().GetFrameFormat() == pOldFormat,
                                "Wrong TextFlyCnt-Hint." );

                    const_cast<SwFormatFlyCnt&>(pHint->GetFlyCnt()).SetFlyFormat(
                            pNewFormat );
                }

                // The old one should not have a flow and it should be adjusted to above and
                // middle.
                // Also, the width should be 100% and it should also adjust the hight, if changed.
                pNewSet->ClearItem();

                pNewSet->Put( SwFormatSurround( SURROUND_NONE ) );
                pNewSet->Put( SvxOpaqueItem( RES_OPAQUE, true ) );

                sal_Int16 eVert = bBefore ? text::VertOrientation::BOTTOM : text::VertOrientation::TOP;
                pNewSet->Put( SwFormatVertOrient( 0, eVert ) );
                pNewSet->Put( SwFormatHoriOrient( 0, text::HoriOrientation::CENTER ) );

                aFrameSize = pOldFormat->GetFrameSize();

                SwOLENode* pOleNode = rDoc.GetNodes()[nNdIdx + 1]->GetOLENode();
                bool isMath = false;
                if(pOleNode)
                {
                    svt::EmbeddedObjectRef& xRef = pOleNode->GetOLEObj().GetObject();
                    if(xRef.is())
                    {
                        SvGlobalName aCLSID( xRef->getClassID() );
                        isMath = ( SotExchange::IsMath( aCLSID ) != 0 );
                    }
                }
                aFrameSize.SetWidthPercent(isMath ? 0 : 100);
                aFrameSize.SetHeightPercent(SwFormatFrameSize::SYNCED);
                pNewSet->Put( aFrameSize );

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
                SwFormatAnchor aAnch( FLY_AT_PARA );
                SwNodeIndex aAnchIdx( *pNewFormat->GetContent().GetContentIdx(), 1 );
                pNew = aAnchIdx.GetNode().GetTextNode();
                SwPosition aPos( aAnchIdx );
                aAnch.SetAnchor( &aPos );
                pNewSet->Put( aAnch );

                if( pUndo )
                    pUndo->SetFlys( *pOldFormat, *pNewSet, *pNewFormat );
                else
                    pOldFormat->SetFormatAttr( *pNewSet );

                delete pNewSet;

                // Have only the FlyFrames created.
                // We leave this to established methods (especially for InCntFlys).
                pNewFormat->MakeFrames();
                // #i115719#
                if ( bIsSwFlyFrameFormatInstance )
                {
                    static_cast<SwFlyFrameFormat*>(pOldFormat)->SetObjTitle( sTitle );
                    static_cast<SwFlyFrameFormat*>(pOldFormat)->SetObjDescription( sDescription );
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
        OUString aText;
        if( bOrderNumberingFirst )
        {
            aText = rNumberingSeparator;
        }
        if( pType)
        {
            aText += pType->GetName();
            if( !bOrderNumberingFirst )
                aText += " ";
        }
        sal_Int32 nIdx = aText.getLength();
        if( !rText.isEmpty() )
        {
            aText += rSeparator;
        }
        const sal_Int32 nSepIdx = aText.getLength();
        aText += rText;

        // Insert string
        SwIndex aIdx( pNew, 0 );
        pNew->InsertText( aText, aIdx );

        // Insert field
        if(pType)
        {
            SwSetExpField aField( static_cast<SwSetExpFieldType*>(pType), OUString(), SVX_NUM_ARABIC);
            if( bOrderNumberingFirst )
                nIdx = 0;
            SwFormatField aFormat( aField );
            pNew->InsertItem( aFormat, nIdx, nIdx );
            if(!rCharacterStyle.isEmpty())
            {
                SwCharFormat* pCharFormat = rDoc.FindCharFormatByName(rCharacterStyle);
                if( !pCharFormat )
                {
                    const sal_uInt16 nMyId = SwStyleNameMapper::GetPoolIdFromUIName(rCharacterStyle, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT);
                    pCharFormat = rDoc.getIDocumentStylePoolAccess().GetCharFormatFromPool( nMyId );
                }
                if (pCharFormat)
                {
                    SwFormatCharFormat aCharFormat( pCharFormat );
                    pNew->InsertItem( aCharFormat, 0,
                        nSepIdx + 1, SetAttrMode::DONTEXPAND );
                }
            }
        }

        if ( bTable )
        {
            if ( bBefore )
            {
                if ( !pNew->GetSwAttrSet().GetKeep().GetValue()  )
                    pNew->SetAttr( SvxFormatKeepItem( true, RES_KEEP ) );
            }
            else
            {
                SwTableNode *const pNd =
                    rDoc.GetNodes()[nNdIdx]->GetStartNode()->GetTableNode();
                SwTable &rTable = pNd->GetTable();
                if ( !rTable.GetFrameFormat()->GetKeep().GetValue() )
                    rTable.GetFrameFormat()->SetFormatAttr( SvxFormatKeepItem( true, RES_KEEP ) );
                if ( pUndo )
                    pUndo->SetUndoKeep();
            }
        }
        rDoc.getIDocumentState().SetModified();
    }

    return pNewFormat;
}

SwFlyFrameFormat *
SwDoc::InsertLabel(
        SwLabelType const eType, OUString const& rText, OUString const& rSeparator,
        OUString const& rNumberingSeparator,
        bool const bBefore, sal_uInt16 const nId, sal_uLong const nNdIdx,
        OUString const& rCharacterStyle,
        bool const bCpyBrd )
{
    SwUndoInsertLabel * pUndo(nullptr);
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        pUndo = new SwUndoInsertLabel(
                        eType, rText, rSeparator, rNumberingSeparator,
                        bBefore, nId, rCharacterStyle, bCpyBrd );
    }

    SwFlyFrameFormat *const pNewFormat = lcl_InsertLabel(*this, mpTextFormatCollTable, pUndo,
            eType, rText, rSeparator, rNumberingSeparator, bBefore,
            nId, nNdIdx, rCharacterStyle, bCpyBrd);

    if (pUndo)
    {
        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }
    else
    {
        GetIDocumentUndoRedo().DelAllUndoObj();
    }

    return pNewFormat;
}

static SwFlyFrameFormat *
lcl_InsertDrawLabel( SwDoc & rDoc, SwTextFormatColls *const pTextFormatCollTable,
        SwUndoInsertLabel *const pUndo, SwDrawFrameFormat *const pOldFormat,
        OUString const& rText,
                                     const OUString& rSeparator,
                                     const OUString& rNumberSeparator,
                                     const sal_uInt16 nId,
                                     const OUString& rCharacterStyle,
                                     SdrObject& rSdrObj )
{
    ::sw::UndoGuard const undoGuard(rDoc.GetIDocumentUndoRedo());
    ::sw::DrawUndoGuard const drawUndoGuard(rDoc.GetIDocumentUndoRedo());

    // Because we get by the TextColl's name, we need to create the field first.
    OSL_ENSURE( nId == USHRT_MAX  || nId < rDoc.getIDocumentFieldsAccess().GetFieldTypes()->size(),
            "FieldType index out of bounds" );
    SwFieldType *pType = nId != USHRT_MAX ? (*rDoc.getIDocumentFieldsAccess().GetFieldTypes())[nId] : nullptr;
    OSL_ENSURE( !pType || pType->Which() == RES_SETEXPFLD, "Wrong label id" );

    SwTextFormatColl *pColl = nullptr;
    if( pType )
    {
        for( auto i = pTextFormatCollTable->size(); i; )
        {
            if( (*pTextFormatCollTable)[ --i ]->GetName()==pType->GetName() )
            {
                pColl = (*pTextFormatCollTable)[i];
                break;
            }
        }
        OSL_ENSURE( pColl, "no text collection found" );
    }

    if( !pColl )
    {
        pColl = rDoc.getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_LABEL );
    }

    SwTextNode* pNew = nullptr;
    SwFlyFrameFormat* pNewFormat = nullptr;

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

    pOldFormat->DelFrames();

    // InContents need to be treated in a special way:
    // The TextAttribute needs to be destroyed.
    // Unfortunately, this also destroys the Format next to the Frames.
    // To avoid this, we disconnect the attribute from the Format.
    SfxItemSet* pNewSet = pOldFormat->GetAttrSet().Clone( false );

    // Protect the Frame's size and position
    if ( rSdrObj.IsMoveProtect() || rSdrObj.IsResizeProtect() )
    {
        SvxProtectItem aProtect(RES_PROTECT);
        aProtect.SetContentProtect( false );
        aProtect.SetPosProtect( rSdrObj.IsMoveProtect() );
        aProtect.SetSizeProtect( rSdrObj.IsResizeProtect() );
        pNewSet->Put( aProtect );
    }

    // Take over the text wrap
    lcl_CpyAttr( *pNewSet, pOldFormat->GetAttrSet(), RES_SURROUND );

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
    pNewSet->Put( pOldFormat->GetHoriOrient() );
    pNewSet->Put( pOldFormat->GetVertOrient() );

    pNewSet->Put( pOldFormat->GetAnchor() );

    // The new one should be variable in its height!
     Size aSz( rSdrObj.GetCurrentBoundRect().GetSize() );
    SwFormatFrameSize aFrameSize( ATT_MIN_SIZE, aSz.Width(), aSz.Height() );
    pNewSet->Put( aFrameSize );

    // Apply the margin to the new Frame.
    // Don't set a border, use the one from the Template.
    pNewSet->Put( pOldFormat->GetLRSpace() );
    pNewSet->Put( pOldFormat->GetULSpace() );

    SwStartNode* pSttNd =
        rDoc.GetNodes().MakeTextSection(
            SwNodeIndex( rDoc.GetNodes().GetEndOfAutotext() ),
                                    SwFlyStartNode, pColl );

    pNewFormat = rDoc.MakeFlyFrameFormat( rDoc.GetUniqueFrameName(),
                 rDoc.getIDocumentStylePoolAccess().GetFrameFormatFromPool( RES_POOLFRM_FRAME ) );

    // Set border and shadow to default if the template contains any.
    if( SfxItemState::SET == pNewFormat->GetAttrSet().GetItemState( RES_BOX ))
        pNewSet->Put( *GetDfltAttr( RES_BOX ) );

    if( SfxItemState::SET == pNewFormat->GetAttrSet().GetItemState(RES_SHADOW))
        pNewSet->Put( *GetDfltAttr( RES_SHADOW ) );

    pNewFormat->SetFormatAttr( SwFormatContent( pSttNd ));
    pNewFormat->SetFormatAttr( *pNewSet );

    const SwFormatAnchor& rAnchor = pNewFormat->GetAnchor();
    if ( FLY_AS_CHAR == rAnchor.GetAnchorId() )
    {
        const SwPosition *pPos = rAnchor.GetContentAnchor();
        SwTextNode *pTextNode = pPos->nNode.GetNode().GetTextNode();
        OSL_ENSURE( pTextNode->HasHints(), "Missing FlyInCnt-Hint." );
        const sal_Int32 nIdx = pPos->nContent.GetIndex();
        SwTextAttr * const pHint =
            pTextNode->GetTextAttrForCharAt( nIdx, RES_TXTATR_FLYCNT );

#if OSL_DEBUG_LEVEL > 0
        OSL_ENSURE( pHint && pHint->Which() == RES_TXTATR_FLYCNT,
                    "Missing FlyInCnt-Hint." );
        OSL_ENSURE( pHint && pHint->GetFlyCnt().
                    GetFrameFormat() == static_cast<SwFrameFormat*>(pOldFormat),
                    "Wrong TextFlyCnt-Hint." );
#endif
        const_cast<SwFormatFlyCnt&>(pHint->GetFlyCnt()).SetFlyFormat( pNewFormat );
    }

    // The old one should not have a flow
    // and it should be adjusted to above and middle.
    pNewSet->ClearItem();

    pNewSet->Put( SwFormatSurround( SURROUND_NONE ) );
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
    pNewSet->Put( SwFormatVertOrient( 0, text::VertOrientation::TOP, text::RelOrientation::FRAME ) );
    pNewSet->Put( SwFormatHoriOrient( 0, text::HoriOrientation::CENTER, text::RelOrientation::FRAME ) );

    // The old one is paragraph-bound to the new one's paragraph.
    SwFormatAnchor aAnch( FLY_AT_PARA );
    SwNodeIndex aAnchIdx( *pNewFormat->GetContent().GetContentIdx(), 1 );
    pNew = aAnchIdx.GetNode().GetTextNode();
    SwPosition aPos( aAnchIdx );
    aAnch.SetAnchor( &aPos );
    pNewSet->Put( aAnch );

    if( pUndo )
    {
        pUndo->SetFlys( *pOldFormat, *pNewSet, *pNewFormat );
        // #i26791# - position no longer needed
        pUndo->SetDrawObj( nLayerId );
    }
    else
        pOldFormat->SetFormatAttr( *pNewSet );

    delete pNewSet;

    // Have only the FlyFrames created.
    // We leave this to established methods (especially for InCntFlys).
    pNewFormat->MakeFrames();

    OSL_ENSURE( pNew, "No Label inserted" );

    if( pNew )
    {
        //#i61007# order of captions
        bool bOrderNumberingFirst = SW_MOD()->GetModuleConfig()->IsCaptionOrderNumberingFirst();

        // prepare string
        OUString aText;
        if( bOrderNumberingFirst )
        {
            aText = rNumberSeparator;
        }
        if ( pType )
        {
            aText += pType->GetName();
            if( !bOrderNumberingFirst )
                aText += " ";
        }
        sal_Int32 nIdx = aText.getLength();
        aText += rSeparator;
        const sal_Int32 nSepIdx = aText.getLength();
        aText += rText;

        // insert text
        SwIndex aIdx( pNew, 0 );
        pNew->InsertText( aText, aIdx );

        // insert field
        if ( pType )
        {
            SwSetExpField aField( static_cast<SwSetExpFieldType*>(pType), OUString(), SVX_NUM_ARABIC );
            if( bOrderNumberingFirst )
                nIdx = 0;
            SwFormatField aFormat( aField );
            pNew->InsertItem( aFormat, nIdx, nIdx );
            if ( !rCharacterStyle.isEmpty() )
            {
                SwCharFormat * pCharFormat = rDoc.FindCharFormatByName(rCharacterStyle);
                if ( !pCharFormat )
                {
                    const sal_uInt16 nMyId = SwStyleNameMapper::GetPoolIdFromUIName( rCharacterStyle, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT );
                    pCharFormat = rDoc.getIDocumentStylePoolAccess().GetCharFormatFromPool( nMyId );
                }
                if ( pCharFormat )
                {
                    SwFormatCharFormat aCharFormat( pCharFormat );
                    pNew->InsertItem( aCharFormat, 0, nSepIdx + 1,
                            SetAttrMode::DONTEXPAND );
                }
            }
        }
    }

    return pNewFormat;
}

SwFlyFrameFormat* SwDoc::InsertDrawLabel(
        OUString const& rText,
        OUString const& rSeparator,
        OUString const& rNumberSeparator,
        sal_uInt16 const nId,
        OUString const& rCharacterStyle,
        SdrObject& rSdrObj )
{
    SwDrawContact *const pContact =
        static_cast<SwDrawContact*>(GetUserCall( &rSdrObj ));
    OSL_ENSURE( RES_DRAWFRMFMT == pContact->GetFormat()->Which(),
            "InsertDrawLabel(): not a DrawFrameFormat" );
    if (!pContact)
        return nullptr;

    SwDrawFrameFormat* pOldFormat = static_cast<SwDrawFrameFormat *>(pContact->GetFormat());
    if (!pOldFormat)
        return nullptr;

    SwUndoInsertLabel * pUndo = nullptr;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().ClearRedo();
        pUndo = new SwUndoInsertLabel(
            LTYPE_DRAW, rText, rSeparator, rNumberSeparator, false,
            nId, rCharacterStyle, false );
    }

    SwFlyFrameFormat *const pNewFormat = lcl_InsertDrawLabel(
        *this, mpTextFormatCollTable, pUndo, pOldFormat,
        rText, rSeparator, rNumberSeparator, nId, rCharacterStyle, rSdrObj);

    if (pUndo)
    {
        GetIDocumentUndoRedo().AppendUndo( pUndo );
    }
    else
    {
        GetIDocumentUndoRedo().DelAllUndoObj();
    }

    return pNewFormat;
}

static OUString lcl_GetUniqueFlyName(const SwDoc* pDoc, sal_uInt16 nDefStrId, RES_FMT eType)
{
    if( pDoc->IsInMailMerge())
    {
        OUString newName = "MailMergeFly"
            + OStringToOUString( DateTimeToOString( DateTime( DateTime::SYSTEM )), RTL_TEXTENCODING_ASCII_US )
            + OUString::number( pDoc->GetSpzFrameFormats()->size() + 1 );
        return newName;
    }

    ResId aId( nDefStrId, *pSwResMgr );
    OUString aName( aId );
    sal_Int32 nNmLen = aName.getLength();

    const SwFrameFormats& rFormats = *pDoc->GetSpzFrameFormats();

    std::vector<sal_uInt8> aSetFlags(rFormats.size()/8 + 2);

    for( SwFrameFormats::size_type n = 0; n < rFormats.size(); ++n )
    {
        const SwFrameFormat* pFlyFormat = rFormats[ n ];
        if (eType != pFlyFormat->Which())
            continue;
        OUString sName;
        if (eType == RES_DRAWFRMFMT)
        {
            const SdrObject *pObj = pFlyFormat->FindSdrObject();
            if (pObj)
                sName = pObj->GetName();
        }
        else
        {
            sName = pFlyFormat->GetName();
        }
        if (sName.startsWith(aName))
        {
            // Only get and set the Flag
            const sal_Int32 nNum = sName.copy(nNmLen).toInt32()-1;
            if( nNum >= 0 && static_cast<SwFrameFormats::size_type>(nNum) < rFormats.size() )
                aSetFlags[ nNum / 8 ] |= (0x01 << ( nNum & 0x07 ));
        }
    }

    // All numbers are flagged accordingly, so determine the right one
    SwFrameFormats::size_type nNum = rFormats.size();
    for( std::vector<sal_uInt8>::size_type n=0; n<aSetFlags.size(); ++n )
    {
        sal_uInt8 nTmp = aSetFlags[ n ];
        if( 0xff != nTmp )
        {
            // so determine the number
            nNum = n * 8;
            while( nTmp & 1 )
            {
                ++nNum;
                nTmp >>= 1;
            }
            break;
        }
    }

    return aName + OUString::number( ++nNum );
}

OUString SwDoc::GetUniqueGrfName() const
{
    return lcl_GetUniqueFlyName(this, STR_GRAPHIC_DEFNAME, RES_FLYFRMFMT);
}

OUString SwDoc::GetUniqueOLEName() const
{
    return lcl_GetUniqueFlyName(this, STR_OBJECT_DEFNAME, RES_FLYFRMFMT);
}

OUString SwDoc::GetUniqueFrameName() const
{
    return lcl_GetUniqueFlyName(this, STR_FRAME_DEFNAME, RES_FLYFRMFMT);
}

OUString SwDoc::GetUniqueShapeName() const
{
    return lcl_GetUniqueFlyName(this, STR_SHAPE_DEFNAME, RES_DRAWFRMFMT);
}

const SwFlyFrameFormat* SwDoc::FindFlyByName( const OUString& rName, sal_Int8 nNdTyp ) const
{
    const SwFrameFormats& rFormats = *GetSpzFrameFormats();
    for( auto n = rFormats.size(); n; )
    {
        const SwFrameFormat* pFlyFormat = rFormats[ --n ];
        const SwNodeIndex* pIdx = nullptr;
        if( RES_FLYFRMFMT == pFlyFormat->Which() && pFlyFormat->GetName() == rName &&
            nullptr != ( pIdx = pFlyFormat->GetContent().GetContentIdx() ) &&
            pIdx->GetNode().GetNodes().IsDocNodes() )
        {
            if( nNdTyp )
            {
                // query for the right NodeType
                const SwNode* pNd = GetNodes()[ pIdx->GetIndex()+1 ];
                if( nNdTyp == ND_TEXTNODE
                        ? !pNd->IsNoTextNode()
                        : nNdTyp == pNd->GetNodeType() )
                    return static_cast<const SwFlyFrameFormat*>(pFlyFormat);
            }
            else
                return static_cast<const SwFlyFrameFormat*>(pFlyFormat);
        }
    }
    return nullptr;
}

void SwDoc::SetFlyName( SwFlyFrameFormat& rFormat, const OUString& rName )
{
    OUString sName( rName );
    if( sName.isEmpty() || FindFlyByName( sName ) )
    {
        sal_uInt16 nTyp = STR_FRAME_DEFNAME;
        const SwNodeIndex* pIdx = rFormat.GetContent().GetContentIdx();
        if( pIdx && pIdx->GetNode().GetNodes().IsDocNodes() )
        {
            switch( GetNodes()[ pIdx->GetIndex() + 1 ]->GetNodeType() )
            {
                case ND_GRFNODE:
                    nTyp = STR_GRAPHIC_DEFNAME;
                    break;
                case ND_OLENODE:
                    nTyp = STR_OBJECT_DEFNAME;
                    break;
            }
        }
        sName = lcl_GetUniqueFlyName(this, nTyp, RES_FLYFRMFMT);
    }
    rFormat.SetName( sName, true );
    getIDocumentState().SetModified();
}

void SwDoc::SetAllUniqueFlyNames()
{
    sal_Int32 n, nFlyNum = 0, nGrfNum = 0, nOLENum = 0;

    ResId nFrameId( STR_FRAME_DEFNAME, *pSwResMgr ),
          nGrfId( STR_GRAPHIC_DEFNAME, *pSwResMgr ),
          nOLEId( STR_OBJECT_DEFNAME, *pSwResMgr );
    const OUString sFlyNm( nFrameId );
    const OUString sGrfNm( nGrfId );
    const OUString sOLENm( nOLEId );

    if( 255 < ( n = GetSpzFrameFormats()->size() ))
        n = 255;
    SwFrameFormats aArr;
    aArr.reserve( n );
    SwFrameFormat* pFlyFormat;
    bool bContainsAtPageObjWithContentAnchor = false;

    for( n = GetSpzFrameFormats()->size(); n; )
    {
        if( RES_FLYFRMFMT == (pFlyFormat = (*GetSpzFrameFormats())[ --n ])->Which() )
        {
            const OUString aNm = pFlyFormat->GetName();
            if ( !aNm.isEmpty() )
            {
                sal_Int32 *pNum = nullptr;
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
                aArr.push_back( pFlyFormat );

        }
        if ( !bContainsAtPageObjWithContentAnchor )
        {
            const SwFormatAnchor& rAnchor = pFlyFormat->GetAnchor();
            if ( (FLY_AT_PAGE == rAnchor.GetAnchorId()) &&
                 rAnchor.GetContentAnchor() )
            {
                bContainsAtPageObjWithContentAnchor = true;
            }
        }
    }
    SetContainsAtPageObjWithContentAnchor( bContainsAtPageObjWithContentAnchor );

    for( n = aArr.size(); n; )
    {
        const SwNodeIndex* pIdx;

        if( nullptr != ( pIdx = ( pFlyFormat = aArr[ --n ])->GetContent().GetContentIdx() )
            && pIdx->GetNode().GetNodes().IsDocNodes() )
        {
            switch( GetNodes()[ pIdx->GetIndex() + 1 ]->GetNodeType() )
            {
            case ND_GRFNODE:
                pFlyFormat->SetName( sGrfNm + OUString::number( ++nGrfNum ));
                break;
            case ND_OLENODE:
                pFlyFormat->SetName( sOLENm + OUString::number( ++nOLENum ));
                break;
            default:
                pFlyFormat->SetName( sFlyNm + OUString::number( ++nFlyNum ));
                break;
            }
        }
    }
    aArr.clear();

    if( !GetFootnoteIdxs().empty() )
    {
        SwTextFootnote::SetUniqueSeqRefNo( *this );
        // #i52775# Chapter footnotes did not get updated correctly.
        // Calling UpdateAllFootnote() instead of UpdateFootnote() solves this problem,
        // but I do not dare to call UpdateAllFootnote() in all cases: Safety first.
        if ( FTNNUM_CHAPTER == GetFootnoteInfo().eNum )
        {
            GetFootnoteIdxs().UpdateAllFootnote();
        }
        else
        {
            SwNodeIndex aTmp( GetNodes() );
            GetFootnoteIdxs().UpdateFootnote( aTmp );
        }
    }
}

bool SwDoc::IsInHeaderFooter( const SwNodeIndex& rIdx ) const
{
    // That can also be a Fly in a Fly in the Header.
    // Is also used by sw3io, to determine if a Redline object is
    // in the Header or Footer.
    // Because Redlines are also attached to Start and EndNode,
    // the Index must not necessarily be from a ContentNode.
    SwNode* pNd = &rIdx.GetNode();
    const SwNode* pFlyNd = pNd->FindFlyStartNode();
    while( pFlyNd )
    {
        // get up by using the Anchor
#if OSL_DEBUG_LEVEL > 0
        std::list<const SwFrameFormat*> checkFormats;
        for( auto pFormat : *GetSpzFrameFormats() )
        {
            const SwNodeIndex* pIdx = pFormat->GetContent().GetContentIdx();
            if( pIdx && pFlyNd == &pIdx->GetNode() )
                checkFormats.push_back( pFormat );
        }
#endif
        std::vector<SwFrameFormat*> const*const pFlys(pFlyNd->GetAnchoredFlys());
        bool bFound(false);
        for (size_t i = 0; pFlys && i < pFlys->size(); ++i)
        {
            const SwFrameFormat *const pFormat = (*pFlys)[i];
            const SwNodeIndex* pIdx = pFormat->GetContent().GetContentIdx();
            if( pIdx && pFlyNd == &pIdx->GetNode() )
            {
#if OSL_DEBUG_LEVEL > 0
                std::list<const SwFrameFormat*>::iterator checkPos = std::find(
                        checkFormats.begin(), checkFormats.end(), pFormat );
                assert( checkPos != checkFormats.end());
                checkFormats.erase( checkPos );
#endif
                const SwFormatAnchor& rAnchor = pFormat->GetAnchor();
                if ((FLY_AT_PAGE == rAnchor.GetAnchorId()) ||
                    !rAnchor.GetContentAnchor() )
                {
                    return false;
                }

                pNd = &rAnchor.GetContentAnchor()->nNode.GetNode();
                pFlyNd = pNd->FindFlyStartNode();
                bFound = true;
                break;
            }
        }
        if (!bFound)
        {
            OSL_ENSURE(mbInReading, "Found a FlySection but not a Format!");
            return false;
        }
    }

    return nullptr != pNd->FindHeaderStartNode() ||
            nullptr != pNd->FindFooterStartNode();
}

short SwDoc::GetTextDirection( const SwPosition& rPos,
                               const Point* pPt ) const
{
    short nRet = -1;

    SwContentNode *pNd = rPos.nNode.GetNode().GetContentNode();

    // #i42921# - use new method <SwContentNode::GetTextDirection(..)>
    if ( pNd )
    {
        nRet = pNd->GetTextDirection( rPos, pPt );
    }
    if ( nRet == -1 )
    {
        const SvxFrameDirectionItem* pItem = nullptr;
        if( pNd )
        {
            // Are we in a FlyFrame? Then look at that for the correct attribute
            const SwFrameFormat* pFlyFormat = pNd->GetFlyFormat();
            while( pFlyFormat )
            {
                pItem = &pFlyFormat->GetFrameDir();
                if( FRMDIR_ENVIRONMENT == pItem->GetValue() )
                {
                    pItem = nullptr;
                    const SwFormatAnchor* pAnchor = &pFlyFormat->GetAnchor();
                    if ((FLY_AT_PAGE != pAnchor->GetAnchorId()) &&
                        pAnchor->GetContentAnchor())
                    {
                        pFlyFormat = pAnchor->GetContentAnchor()->nNode.
                                            GetNode().GetFlyFormat();
                    }
                    else
                        pFlyFormat = nullptr;
                }
                else
                    pFlyFormat = nullptr;
            }

            if( !pItem )
            {
                const SwPageDesc* pPgDsc = pNd->FindPageDesc( false );
                if( pPgDsc )
                    pItem = &pPgDsc->GetMaster().GetFrameDir();
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

std::set<SwRootFrame*> SwDoc::GetAllLayouts()
{
    std::set<SwRootFrame*> aAllLayouts;
    SwViewShell *pStart = getIDocumentLayoutAccess().GetCurrentViewShell();
    if(pStart)
    {
        for(SwViewShell& rShell : pStart->GetRingContainer())
        {
            if(rShell.GetLayout())
                aAllLayouts.insert(rShell.GetLayout());
        }
    }
    return aAllLayouts;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
