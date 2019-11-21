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

#include <sal/config.h>

#include <utility>

#include <UndoAttribute.hxx>
#include <svl/itemiter.hxx>
#include <editeng/tstpitem.hxx>
#include <svx/svdobj.hxx>
#include <hintids.hxx>
#include <fmtflcnt.hxx>
#include <txtftn.hxx>
#include <fmtanchr.hxx>
#include <fmtfsize.hxx>
#include <frmfmt.hxx>
#include <fmtcntnt.hxx>
#include <ftnidx.hxx>
#include <doc.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IShellCursorSupplier.hxx>
#include <docary.hxx>
#include <swundo.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <swtable.hxx>
#include <swtblfmt.hxx>
#include <UndoCore.hxx>
#include <hints.hxx>
#include <rolbck.hxx>
#include <ndnotxt.hxx>
#include <ftninfo.hxx>
#include <redline.hxx>
#include <section.hxx>
#include <charfmt.hxx>
#include <calbck.hxx>
#include <frameformats.hxx>

SwUndoFormatAttrHelper::SwUndoFormatAttrHelper( SwFormat& rFormat, bool bSvDrwPt )
    : SwClient( &rFormat )
    , m_bSaveDrawPt( bSvDrwPt )
{
}

void SwUndoFormatAttrHelper::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    if( pOld ) {
        if ( pOld->Which() == RES_OBJECTDYING ) {
            CheckRegistration( pOld );
        } else if ( pNew ) {
            const SwDoc& rDoc = *static_cast<SwFormat*>(GetRegisteredInNonConst())->GetDoc();
            if( POOLATTR_END >= pOld->Which() ) {
                if ( GetUndo() ) {
                    m_pUndo->PutAttr( *pOld, rDoc );
                } else {
                    m_pUndo.reset( new SwUndoFormatAttr( *pOld,
                                                      *static_cast<SwFormat*>(GetRegisteredInNonConst()), m_bSaveDrawPt ) );
                }
            } else if ( RES_ATTRSET_CHG == pOld->Which() ) {
                if ( GetUndo() ) {
                    SfxItemIter aIter(
                        *static_cast<const SwAttrSetChg*>(pOld)->GetChgSet() );
                    for (auto pItem = aIter.GetCurItem(); pItem; pItem = aIter.NextItem())
                    {
                        m_pUndo->PutAttr( *pItem, rDoc );
                    }
                } else {
                    m_pUndo.reset( new SwUndoFormatAttr(
                                       *static_cast<const SwAttrSetChg*>(pOld)->GetChgSet(),
                                       *static_cast<SwFormat*>(GetRegisteredInNonConst()), m_bSaveDrawPt ) );
                }
            }
        }
    }
}

SwUndoFormatAttr::SwUndoFormatAttr( const SfxItemSet& rOldSet,
                              SwFormat& rChgFormat,
                              bool bSaveDrawPt )
    : SwUndo( SwUndoId::INSFMTATTR, rChgFormat.GetDoc() )
    , m_sFormatName ( rChgFormat.GetName() )
    // #i56253#
    , m_pOldSet( new SfxItemSet( rOldSet ) )
    , m_nNodeIndex( 0 )
    , m_nFormatWhich( rChgFormat.Which() )
    , m_bSaveDrawPt( bSaveDrawPt )
{
    assert(m_sFormatName.getLength());
    SAL_WARN_IF(m_sFormatName.isEmpty(), "sw.core", "Format is missing name. Undo/redo could work incorrectly");

    Init( rChgFormat );
}

SwUndoFormatAttr::SwUndoFormatAttr( const SfxPoolItem& rItem, SwFormat& rChgFormat,
                              bool bSaveDrawPt )
    : SwUndo( SwUndoId::INSFMTATTR, rChgFormat.GetDoc() )
    , m_sFormatName(rChgFormat.GetName())
    , m_pOldSet( rChgFormat.GetAttrSet().Clone( false ) )
    , m_nNodeIndex( 0 )
    , m_nFormatWhich( rChgFormat.Which() )
    , m_bSaveDrawPt( bSaveDrawPt )
{
    assert(m_sFormatName.getLength());
    SAL_WARN_IF(m_sFormatName.isEmpty(), "sw.core", "Format is missing name. Undo/redo could work incorrectly");

    m_pOldSet->Put( rItem );
    Init( rChgFormat );
}

void SwUndoFormatAttr::Init( const SwFormat & rFormat )
{
    // tdf#126017 never save SwNodeIndex, it will go stale
    m_pOldSet->ClearItem(RES_CNTNT);
    // treat change of anchor specially
    if ( SfxItemState::SET == m_pOldSet->GetItemState( RES_ANCHOR, false )) {
        SaveFlyAnchor( &rFormat, m_bSaveDrawPt );
    } else if ( RES_FRMFMT == m_nFormatWhich ) {
        const SwDoc* pDoc = rFormat.GetDoc();
        if (pDoc->GetTableFrameFormats()->ContainsFormat(dynamic_cast<const SwFrameFormat&>(rFormat)))
        {
            // Table Format: save table position, table formats are volatile!
            SwTable * pTable = SwIterator<SwTable,SwFormat>( rFormat ).First();
            if ( pTable ) {
                m_nNodeIndex = pTable->GetTabSortBoxes()[ 0 ]->GetSttNd()
                               ->FindTableNode()->GetIndex();
            }
        } else if (pDoc->GetSections().ContainsFormat(&rFormat)) {
            m_nNodeIndex = rFormat.GetContent().GetContentIdx()->GetIndex();
        } else if ( dynamic_cast< const SwTableBoxFormat* >( &rFormat ) !=  nullptr ) {
            SwTableBox * pTableBox = SwIterator<SwTableBox,SwFormat>( rFormat ).First();
            if ( pTableBox ) {
                m_nNodeIndex = pTableBox->GetSttIdx();
            }
        }
    }
}

SwUndoFormatAttr::~SwUndoFormatAttr()
{
}

void SwUndoFormatAttr::UndoImpl(::sw::UndoRedoContext & rContext)
{
    // OD 2004-10-26 #i35443#
    // Important note: <Undo(..)> also called by <ReDo(..)>

    if (!m_pOldSet)
        return;

    SwFormat * pFormat = GetFormat(rContext.GetDoc());
    if (!pFormat)
        return;

    // #i35443# - If anchor attribute has been successful
    // restored, all other attributes are also restored.
    // Thus, keep track of its restoration
    bool bAnchorAttrRestored( false );
    if ( SfxItemState::SET == m_pOldSet->GetItemState( RES_ANCHOR, false )) {
        bAnchorAttrRestored = RestoreFlyAnchor(rContext);
        if ( bAnchorAttrRestored ) {
            // Anchor attribute successful restored.
            // Thus, keep anchor position for redo
            SaveFlyAnchor(pFormat);
        } else {
            // Anchor attribute not restored due to invalid anchor position.
            // Thus, delete anchor attribute.
            m_pOldSet->ClearItem( RES_ANCHOR );
        }
    }

    if ( !bAnchorAttrRestored ) {
        SwUndoFormatAttrHelper aTmp( *pFormat, m_bSaveDrawPt );
        pFormat->SetFormatAttr( *m_pOldSet );
        if ( aTmp.GetUndo() ) {
            // transfer ownership of helper object's old set
            m_pOldSet = std::move(aTmp.GetUndo()->m_pOldSet);
        } else {
            m_pOldSet->ClearItem();
        }

        if ( RES_FLYFRMFMT == m_nFormatWhich || RES_DRAWFRMFMT == m_nFormatWhich ) {
            rContext.SetSelections(static_cast<SwFrameFormat*>(pFormat), nullptr);
        }
    }
}

// Check if it is still in Doc
SwFormat* SwUndoFormatAttr::GetFormat( const SwDoc& rDoc )
{
    switch (m_nFormatWhich)
    {
    case RES_TXTFMTCOLL:
    case RES_CONDTXTFMTCOLL:
        return rDoc.FindTextFormatCollByName(m_sFormatName);

    case RES_GRFFMTCOLL:
        return SwDoc::FindFormatByName(*rDoc.GetGrfFormatColls(), m_sFormatName);

    case RES_CHRFMT:
        return rDoc.FindCharFormatByName(m_sFormatName);

    case RES_FRMFMT:
        if (m_nNodeIndex && (m_nNodeIndex < rDoc.GetNodes().Count()))
        {
            SwNode* pNd = rDoc.GetNodes()[m_nNodeIndex];
            if (pNd->IsTableNode())
            {
                return static_cast<SwTableNode*>(pNd)->GetTable().GetFrameFormat();
            }
            else if (pNd->IsSectionNode())
            {
                return static_cast<SwSectionNode*>(pNd)->GetSection().GetFormat();
            }
            else if (pNd->IsStartNode() && (SwTableBoxStartNode ==
                static_cast<SwStartNode*>(pNd)->GetStartNodeType()))
            {
                SwTableNode* pTableNode = pNd->FindTableNode();
                if (pTableNode)
                {
                    SwTableBox* pBox = pTableNode->GetTable().GetTableBox(m_nNodeIndex);
                    if (pBox)
                    {
                        return pBox->GetFrameFormat();
                    }
                }
            }
        }
        [[fallthrough]];
    case RES_DRAWFRMFMT:
    case RES_FLYFRMFMT:
        {
            SwFormat * pFormat = SwDoc::FindFormatByName(*rDoc.GetSpzFrameFormats(), m_sFormatName);
            if (pFormat)
                return pFormat;
            pFormat = SwDoc::FindFormatByName(*rDoc.GetFrameFormats(), m_sFormatName);
            if (pFormat)
                return pFormat;
        }
        break;
    }

    return nullptr;
}

void SwUndoFormatAttr::RedoImpl(::sw::UndoRedoContext & rContext)
{
    // #i35443# - Because the undo stores the attributes for
    // redo, the same code as for <Undo(..)> can be applied for <Redo(..)>
    UndoImpl(rContext);
}

void SwUndoFormatAttr::RepeatImpl(::sw::RepeatContext & rContext)
{
    if (!m_pOldSet)
        return;

    SwDoc & rDoc(rContext.GetDoc());

    SwFormat * pFormat = GetFormat(rDoc);
    if (!pFormat)
        return;

    switch ( m_nFormatWhich ) {
    case RES_GRFFMTCOLL: {
        SwNoTextNode *const pNd =
            rContext.GetRepeatPaM().GetNode().GetNoTextNode();
        if( pNd ) {
            rDoc.SetAttr( pFormat->GetAttrSet(), *pNd->GetFormatColl() );
        }
    }
    break;

    case RES_TXTFMTCOLL:
    case RES_CONDTXTFMTCOLL:
    {
        SwTextNode *const pNd =
            rContext.GetRepeatPaM().GetNode().GetTextNode();
        if( pNd ) {
            rDoc.SetAttr( pFormat->GetAttrSet(), *pNd->GetFormatColl() );
        }
    }
    break;

    case RES_FLYFRMFMT: {
        // Check if the cursor is in a flying frame
        // Steps: search in all FlyFrameFormats for the FlyContent attribute
        // and validate if the cursor is in the respective section
        SwFrameFormat *const pFly =
            rContext.GetRepeatPaM().GetNode().GetFlyFormat();
        if( pFly ) {
            // Bug 43672: do not set all attributes!
            if (SfxItemState::SET ==
                pFormat->GetAttrSet().GetItemState( RES_CNTNT )) {
                SfxItemSet aTmpSet( pFormat->GetAttrSet() );
                aTmpSet.ClearItem( RES_CNTNT );
                if( aTmpSet.Count() ) {
                    rDoc.SetAttr( aTmpSet, *pFly );
                }
            } else {
                rDoc.SetAttr( pFormat->GetAttrSet(), *pFly );
            }
        }
        break;
    }
    }
}

SwRewriter SwUndoFormatAttr::GetRewriter() const
{
    SwRewriter aRewriter;

    aRewriter.AddRule(UndoArg1, m_sFormatName);

    return aRewriter;
}

void SwUndoFormatAttr::PutAttr( const SfxPoolItem& rItem, const SwDoc& rDoc )
{
    if (RES_CNTNT == rItem.Which())
    {
        return; // tdf#126017 never save SwNodeIndex, it will go stale
    }
    m_pOldSet->Put( rItem );
    if ( RES_ANCHOR == rItem.Which() )
    {
        SwFormat * pFormat = GetFormat( rDoc );
        SaveFlyAnchor( pFormat, m_bSaveDrawPt );
    }
}

void SwUndoFormatAttr::SaveFlyAnchor( const SwFormat * pFormat, bool bSvDrwPt )
{
    // Format is valid, otherwise you would not reach this point here
    if( bSvDrwPt ) {
        if ( RES_DRAWFRMFMT == pFormat->Which() ) {
            Point aPt( static_cast<const SwFrameFormat*>(pFormat)->FindSdrObject()
                       ->GetRelativePos() );
            // store old value as attribute, to keep SwUndoFormatAttr small
            m_pOldSet->Put( SwFormatFrameSize( SwFrameSize::Variable, aPt.X(), aPt.Y() ) );
        }
    }

    const SwFormatAnchor& rAnchor =
        m_pOldSet->Get( RES_ANCHOR, false );
    if( !rAnchor.GetContentAnchor() )
        return;

    sal_Int32 nContent = 0;
    switch( rAnchor.GetAnchorId() ) {
    case RndStdIds::FLY_AS_CHAR:
    case RndStdIds::FLY_AT_CHAR:
        nContent = rAnchor.GetContentAnchor()->nContent.GetIndex();
        [[fallthrough]];
    case RndStdIds::FLY_AT_PARA:
    case RndStdIds::FLY_AT_FLY:
        m_nNodeIndex = rAnchor.GetContentAnchor()->nNode.GetIndex();
        break;
    default:
        return;
    }

    SwFormatAnchor aAnchor( rAnchor.GetAnchorId(), nContent );
    m_pOldSet->Put( aAnchor );
}

// #i35443# - Add return value, type <bool>.
// Return value indicates, if anchor attribute is restored.
// Note: If anchor attribute is restored, all other existing attributes
//       are also restored.
bool SwUndoFormatAttr::RestoreFlyAnchor(::sw::UndoRedoContext & rContext)
{
    SwDoc *const pDoc = & rContext.GetDoc();
    SwFrameFormat* pFrameFormat = static_cast<SwFrameFormat*>( GetFormat( *pDoc ) );
    const SwFormatAnchor& rAnchor =
        m_pOldSet->Get( RES_ANCHOR, false );

    SwFormatAnchor aNewAnchor( rAnchor.GetAnchorId() );
    if (RndStdIds::FLY_AT_PAGE != rAnchor.GetAnchorId()) {
        SwNode* pNd = pDoc->GetNodes()[ m_nNodeIndex  ];

        if (  (RndStdIds::FLY_AT_FLY == rAnchor.GetAnchorId())
              ? ( !pNd->IsStartNode() || (SwFlyStartNode !=
                                          static_cast<SwStartNode*>(pNd)->GetStartNodeType()) )
              : !pNd->IsTextNode() ) {
            // #i35443# - invalid position.
            // Thus, anchor attribute not restored
            return false;
        }

        SwPosition aPos( *pNd );
        if ((RndStdIds::FLY_AS_CHAR == rAnchor.GetAnchorId()) ||
            (RndStdIds::FLY_AT_CHAR == rAnchor.GetAnchorId())) {
            aPos.nContent.Assign( static_cast<SwTextNode*>(pNd), rAnchor.GetPageNum() );
            if ( aPos.nContent.GetIndex() > pNd->GetTextNode()->GetText().getLength()) {
                // #i35443# - invalid position.
                // Thus, anchor attribute not restored
                return false;
            }
        }
        aNewAnchor.SetAnchor( &aPos );
    } else
        aNewAnchor.SetPageNum( rAnchor.GetPageNum() );

    Point aDrawSavePt, aDrawOldPt;
    if( pDoc->getIDocumentLayoutAccess().GetCurrentViewShell() ) {
        if( RES_DRAWFRMFMT == pFrameFormat->Which() ) {
            // get the old cached value
            const SwFormatFrameSize& rOldSize = m_pOldSet->Get( RES_FRM_SIZE );
            aDrawSavePt.setX( rOldSize.GetWidth() );
            aDrawSavePt.setY( rOldSize.GetHeight() );
            m_pOldSet->ClearItem( RES_FRM_SIZE );

            // write the current value into cache
            aDrawOldPt = pFrameFormat->FindSdrObject()->GetRelativePos();
        } else {
            pFrameFormat->DelFrames();         // delete Frames
        }
    }

    const SwFormatAnchor &rOldAnch = pFrameFormat->GetAnchor();
    // #i54336#
    // Consider case, that as-character anchored object has moved its anchor position.
    if (RndStdIds::FLY_AS_CHAR == rOldAnch.GetAnchorId()) {
        // With InContents it's tricky: the text attribute needs to be deleted.
        // Unfortunately, this not only destroys the Frames but also the format.
        // To prevent that, first detach the connection between attribute and
        // format.
        const SwPosition *pPos = rOldAnch.GetContentAnchor();
        SwTextNode *pTextNode = static_cast<SwTextNode*>(&pPos->nNode.GetNode());
        SAL_WARN_IF( !pTextNode->HasHints(), "sw.core", "Missing FlyInCnt-Hint." );
        const sal_Int32 nIdx = pPos->nContent.GetIndex();
        SwTextAttr * const pHint =
            pTextNode->GetTextAttrForCharAt( nIdx, RES_TXTATR_FLYCNT );
        assert(pHint && "Missing Hint.");
        SAL_WARN_IF( pHint->Which() != RES_TXTATR_FLYCNT, "sw.core",
                    "Missing FlyInCnt-Hint." );
        SAL_WARN_IF( pHint->GetFlyCnt().GetFrameFormat() != pFrameFormat, "sw.core",
                    "Wrong TextFlyCnt-Hint." );
        const_cast<SwFormatFlyCnt&>(pHint->GetFlyCnt()).SetFlyFormat();

        // Connection is now detached, therefore the attribute can be deleted
        pTextNode->DeleteAttributes( RES_TXTATR_FLYCNT, nIdx, nIdx );
    }

    {
        m_pOldSet->Put( aNewAnchor );
        SwUndoFormatAttrHelper aTmp( *pFrameFormat, m_bSaveDrawPt );
        pFrameFormat->SetFormatAttr( *m_pOldSet );
        if ( aTmp.GetUndo() ) {
            m_nNodeIndex = aTmp.GetUndo()->m_nNodeIndex;
            // transfer ownership of helper object's old set
            m_pOldSet = std::move(aTmp.GetUndo()->m_pOldSet);
        } else {
            m_pOldSet->ClearItem();
        }
    }

    if ( RES_DRAWFRMFMT == pFrameFormat->Which() )
    {
        // The Draw model also prepared an Undo object for its right positioning
        // which unfortunately is relative. Therefore block here a position
        // change of the Contact object by setting the anchor.
        pFrameFormat->CallSwClientNotify(sw::RestoreFlyAnchorHint(aDrawSavePt));
        // cache the old value again
        m_pOldSet->Put(SwFormatFrameSize(SwFrameSize::Variable, aDrawOldPt.X(), aDrawOldPt.Y()));
    }

    if (RndStdIds::FLY_AS_CHAR == aNewAnchor.GetAnchorId()) {
        const SwPosition* pPos = aNewAnchor.GetContentAnchor();
        SwTextNode* pTextNd = pPos->nNode.GetNode().GetTextNode();
        SAL_WARN_IF( !pTextNd, "sw.core", "no Text Node at position." );
        SwFormatFlyCnt aFormat( pFrameFormat );
        pTextNd->InsertItem( aFormat, pPos->nContent.GetIndex(), 0 );
    }

    if (RES_DRAWFRMFMT != pFrameFormat->Which())
        pFrameFormat->MakeFrames();
    else
    {
        pFrameFormat->CallSwClientNotify(sw::DrawFrameFormatHint(sw::DrawFrameFormatHintId::POST_RESTORE_FLY_ANCHOR));
    }

    rContext.SetSelections(pFrameFormat, nullptr);

    // #i35443# - anchor attribute restored.
    return true;
}

SwUndoFormatResetAttr::SwUndoFormatResetAttr( SwFormat& rChangedFormat,
                                        const sal_uInt16 nWhichId )
    : SwUndo( SwUndoId::RESETATTR, rChangedFormat.GetDoc() )
    , m_pChangedFormat( &rChangedFormat )
    , m_nWhichId( nWhichId )
{
    const SfxPoolItem* pItem = nullptr;
    if (rChangedFormat.GetItemState(nWhichId, false, &pItem ) == SfxItemState::SET && pItem) {
        m_pOldItem.reset( pItem->Clone() );
    }
}

SwUndoFormatResetAttr::~SwUndoFormatResetAttr()
{
}

void SwUndoFormatResetAttr::UndoImpl(::sw::UndoRedoContext &)
{
    if (m_pOldItem)
    {
        m_pChangedFormat->SetFormatAttr( *m_pOldItem );
    }
}

void SwUndoFormatResetAttr::RedoImpl(::sw::UndoRedoContext &)
{
    if (m_pOldItem)
    {
        m_pChangedFormat->ResetFormatAttr( m_nWhichId );
    }
}

SwUndoResetAttr::SwUndoResetAttr( const SwPaM& rRange, sal_uInt16 nFormatId )
    : SwUndo( SwUndoId::RESETATTR, rRange.GetDoc() ), SwUndRng( rRange )
    , m_pHistory( new SwHistory )
    , m_nFormatId( nFormatId )
{
}

SwUndoResetAttr::SwUndoResetAttr( const SwPosition& rPos, sal_uInt16 nFormatId )
    : SwUndo( SwUndoId::RESETATTR, rPos.GetDoc() )
    , m_pHistory( new SwHistory )
    , m_nFormatId( nFormatId )
{
    m_nSttNode = m_nEndNode = rPos.nNode.GetIndex();
    m_nSttContent = m_nEndContent = rPos.nContent.GetIndex();
}

SwUndoResetAttr::~SwUndoResetAttr()
{
}

void SwUndoResetAttr::UndoImpl(::sw::UndoRedoContext & rContext)
{
    // reset old values
    SwDoc & rDoc = rContext.GetDoc();
    m_pHistory->TmpRollback( &rDoc, 0 );
    m_pHistory->SetTmpEnd( m_pHistory->Count() );

    if ((RES_CONDTXTFMTCOLL == m_nFormatId) &&
        (m_nSttNode == m_nEndNode) && (m_nSttContent == m_nEndContent)) {
        SwTextNode* pTNd = rDoc.GetNodes()[ m_nSttNode ]->GetTextNode();
        if( pTNd ) {
            SwIndex aIdx( pTNd, m_nSttContent );
            pTNd->DontExpandFormat( aIdx, false );
        }
    }

    AddUndoRedoPaM(rContext);
}

void SwUndoResetAttr::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    SwPaM & rPam = AddUndoRedoPaM(rContext);

    switch ( m_nFormatId ) {
    case RES_CHRFMT:
        rDoc.RstTextAttrs(rPam);
        break;
    case RES_TXTFMTCOLL:
        rDoc.ResetAttrs(rPam, false, m_Ids );
        break;
    case RES_CONDTXTFMTCOLL:
        rDoc.ResetAttrs(rPam, true, m_Ids );

        break;
    case RES_TXTATR_TOXMARK:
        // special treatment for TOXMarks
    {
        SwTOXMarks aArr;
        SwNodeIndex aIdx( rDoc.GetNodes(), m_nSttNode );
        SwPosition aPos( aIdx, SwIndex( aIdx.GetNode().GetContentNode(),
                                        m_nSttContent ));

        sal_uInt16 nCnt = SwDoc::GetCurTOXMark( aPos, aArr );
        if( nCnt ) {
            if( 1 < nCnt ) {
                // search for the right one
                SwHistoryHint* pHHint = (GetHistory())[ 0 ];
                if( pHHint && HSTRY_SETTOXMARKHNT == pHHint->Which() ) {
                    while( nCnt ) {
                        if ( static_cast<SwHistorySetTOXMark*>(pHHint)
                             ->IsEqual( *aArr[ --nCnt ] ) ) {
                            ++nCnt;
                            break;
                        }
                    }
                } else
                    nCnt = 0;
            }
            // found one, thus delete it
            if( nCnt-- ) {
                rDoc.DeleteTOXMark( aArr[ nCnt ] );
            }
        }
    }
    break;
    }
}

void SwUndoResetAttr::RepeatImpl(::sw::RepeatContext & rContext)
{
    if (m_nFormatId < RES_FMT_BEGIN) {
        return;
    }

    switch ( m_nFormatId ) {
    case RES_CHRFMT:
        rContext.GetDoc().RstTextAttrs(rContext.GetRepeatPaM());
        break;
    case RES_TXTFMTCOLL:
        rContext.GetDoc().ResetAttrs(rContext.GetRepeatPaM(), false, m_Ids);
        break;
    case RES_CONDTXTFMTCOLL:
        rContext.GetDoc().ResetAttrs(rContext.GetRepeatPaM(), true, m_Ids);
        break;
    }
}

void SwUndoResetAttr::SetAttrs( const std::set<sal_uInt16> &rAttrs )
{
    m_Ids = rAttrs;
}

SwUndoAttr::SwUndoAttr( const SwPaM& rRange, const SfxPoolItem& rAttr,
                        const SetAttrMode nFlags )
    : SwUndo( SwUndoId::INSATTR, rRange.GetDoc() ), SwUndRng( rRange )
    , m_AttrSet( rRange.GetDoc()->GetAttrPool(), {{rAttr.Which(), rAttr.Which()}} )
    , m_pHistory( new SwHistory )
    , m_nNodeIndex( ULONG_MAX )
    , m_nInsertFlags( nFlags )
{
    m_AttrSet.Put( rAttr );

    // Save character style as a style name, not as a reference
    const SfxPoolItem* pItem = m_AttrSet.GetItem(RES_TXTATR_CHARFMT);
    if (pItem)
    {
        uno::Any aValue;
        pItem->QueryValue(aValue, RES_TXTATR_CHARFMT);
        aValue >>= m_aChrFormatName;
    }
}

SwUndoAttr::SwUndoAttr( const SwPaM& rRange, const SfxItemSet& rSet,
                        const SetAttrMode nFlags )
    : SwUndo( SwUndoId::INSATTR, rRange.GetDoc() ), SwUndRng( rRange )
    , m_AttrSet( rSet )
    , m_pHistory( new SwHistory )
    , m_nNodeIndex( ULONG_MAX )
    , m_nInsertFlags( nFlags )
{
    // Save character style as a style name, not as a reference
    const SfxPoolItem* pItem = m_AttrSet.GetItem(RES_TXTATR_CHARFMT);
    if (pItem)
    {
        uno::Any aValue;
        pItem->QueryValue(aValue, RES_TXTATR_CHARFMT);
        aValue >>= m_aChrFormatName;
    }
}

SwUndoAttr::~SwUndoAttr()
{
}

void SwUndoAttr::SaveRedlineData( const SwPaM& rPam, bool bIsContent )
{
    SwDoc* pDoc = rPam.GetDoc();
    if ( pDoc->getIDocumentRedlineAccess().IsRedlineOn() ) {
        m_pRedlineData.reset( new SwRedlineData( bIsContent
                              ? RedlineType::Insert
                              : RedlineType::Format,
                              pDoc->getIDocumentRedlineAccess().GetRedlineAuthor() ) );
    }

    m_pRedlineSaveData.reset( new SwRedlineSaveDatas );
    if ( !FillSaveDataForFormat( rPam, *m_pRedlineSaveData ))
        m_pRedlineSaveData.reset();

    SetRedlineFlags( pDoc->getIDocumentRedlineAccess().GetRedlineFlags() );
    if ( bIsContent ) {
        m_nNodeIndex = rPam.GetPoint()->nNode.GetIndex();
    }
}

void SwUndoAttr::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc *const pDoc = & rContext.GetDoc();

    RemoveIdx( *pDoc );

    if( IDocumentRedlineAccess::IsRedlineOn( GetRedlineFlags() ) ) {
        SwPaM aPam(pDoc->GetNodes().GetEndOfContent());
        if ( ULONG_MAX != m_nNodeIndex ) {
            aPam.DeleteMark();
            aPam.GetPoint()->nNode = m_nNodeIndex;
            aPam.GetPoint()->nContent.Assign( aPam.GetContentNode(), m_nSttContent );
            aPam.SetMark();
            ++aPam.GetPoint()->nContent;
            pDoc->getIDocumentRedlineAccess().DeleteRedline(aPam, false, RedlineType::Any);
        } else {
            // remove all format redlines, will be recreated if needed
            SetPaM(aPam);
            pDoc->getIDocumentRedlineAccess().DeleteRedline(aPam, false, RedlineType::Format);
            if (m_pRedlineSaveData)
            {
                SetSaveData( *pDoc, *m_pRedlineSaveData );
            }
        }
    }

    const bool bToLast =  (1 == m_AttrSet.Count())
                          && (RES_TXTATR_FIELD <= *m_AttrSet.GetRanges())
                          && (*m_AttrSet.GetRanges() <= RES_TXTATR_ANNOTATION);

    // restore old values
    m_pHistory->TmpRollback( pDoc, 0, !bToLast );
    m_pHistory->SetTmpEnd( m_pHistory->Count() );

    // set cursor onto Undo area
    AddUndoRedoPaM(rContext);
}

void SwUndoAttr::RepeatImpl(::sw::RepeatContext & rContext)
{
    // RefMarks are not repeat capable
    if ( SfxItemState::SET != m_AttrSet.GetItemState( RES_TXTATR_REFMARK, false ) ) {
        rContext.GetDoc().getIDocumentContentOperations().InsertItemSet( rContext.GetRepeatPaM(),
                m_AttrSet, m_nInsertFlags );
    } else if ( 1 < m_AttrSet.Count() ) {
        SfxItemSet aTmpSet( m_AttrSet );
        aTmpSet.ClearItem( RES_TXTATR_REFMARK );
        rContext.GetDoc().getIDocumentContentOperations().InsertItemSet( rContext.GetRepeatPaM(),
                aTmpSet, m_nInsertFlags );
    }
}

void SwUndoAttr::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    SwPaM & rPam = AddUndoRedoPaM(rContext);

    // Restore pointer to char format from name
    if (!m_aChrFormatName.isEmpty())
    {
        SwCharFormat* pCharFormat = rDoc.FindCharFormatByName(m_aChrFormatName);
        if (pCharFormat)
        {
            SwFormatCharFormat aFormat(pCharFormat);
            m_AttrSet.Put(aFormat);
        }
    }

    if ( m_pRedlineData &&
         IDocumentRedlineAccess::IsRedlineOn( GetRedlineFlags() ) ) {
        RedlineFlags eOld = rDoc.getIDocumentRedlineAccess().GetRedlineFlags();
        rDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld & ~RedlineFlags::Ignore );
        rDoc.getIDocumentContentOperations().InsertItemSet( rPam, m_AttrSet, m_nInsertFlags );

        if ( ULONG_MAX != m_nNodeIndex ) {
            rPam.SetMark();
            if ( rPam.Move( fnMoveBackward ) ) {
                rDoc.getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline( *m_pRedlineData, rPam ),
                        true);
            }
            rPam.DeleteMark();
        } else {
            rDoc.getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline( *m_pRedlineData, rPam ), true);
        }

        rDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
    } else {
        rDoc.getIDocumentContentOperations().InsertItemSet( rPam, m_AttrSet, m_nInsertFlags );
    }
}

void SwUndoAttr::RemoveIdx( SwDoc& rDoc )
{
    if ( SfxItemState::SET != m_AttrSet.GetItemState( RES_TXTATR_FTN, false ))
        return ;

    SwNodes& rNds = rDoc.GetNodes();
    for ( sal_uInt16 n = 0; n < m_pHistory->Count(); ++n ) {
        sal_Int32 nContent = 0;
        sal_uLong nNode = 0;
        SwHistoryHint* pHstHint = (*m_pHistory)[ n ];
        switch ( pHstHint->Which() ) {
        case HSTRY_RESETTXTHNT: {
            SwHistoryResetText * pHistoryHint
                = static_cast<SwHistoryResetText*>(pHstHint);
            if ( RES_TXTATR_FTN == pHistoryHint->GetWhich() ) {
                nNode = pHistoryHint->GetNode();
                nContent = pHistoryHint->GetContent();
            }
        }
        break;

        default:
            break;
        }

        if( nNode ) {
            SwTextNode* pTextNd = rNds[ nNode ]->GetTextNode();
            if( pTextNd ) {
                SwTextAttr *const pTextHt =
                    pTextNd->GetTextAttrForCharAt(nContent, RES_TXTATR_FTN);
                if( pTextHt ) {
                    // ok, so get values
                    SwTextFootnote* pFootnote = static_cast<SwTextFootnote*>(pTextHt);
                    RemoveIdxFromSection( rDoc, pFootnote->GetStartNode()->GetIndex() );
                    return ;
                }
            }
        }
    }
}

SwUndoDefaultAttr::SwUndoDefaultAttr( const SfxItemSet& rSet, const SwDoc* pDoc )
    : SwUndo( SwUndoId::SETDEFTATTR, pDoc )
{
    const SfxPoolItem* pItem;
    if( SfxItemState::SET == rSet.GetItemState( RES_PARATR_TABSTOP, false, &pItem ) ) {
        // store separately, because it may change!
        m_pTabStop.reset( static_cast<SvxTabStopItem*>(pItem->Clone()) );
        if ( 1 != rSet.Count() ) { // are there more attributes?
            m_pOldSet.reset( new SfxItemSet( rSet ) );
        }
    } else {
        m_pOldSet.reset( new SfxItemSet( rSet ) );
    }
}

SwUndoDefaultAttr::~SwUndoDefaultAttr()
{
}

void SwUndoDefaultAttr::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    if (m_pOldSet)
    {
        SwUndoFormatAttrHelper aTmp(
            *rDoc.GetDfltTextFormatColl() );
        rDoc.SetDefault( *m_pOldSet );
        m_pOldSet.reset();
        if ( aTmp.GetUndo() ) {
            // transfer ownership of helper object's old set
            m_pOldSet = std::move(aTmp.GetUndo()->m_pOldSet);
        }
    }
    if (m_pTabStop)
    {
        std::unique_ptr<SvxTabStopItem> pOld(rDoc.GetDefault(RES_PARATR_TABSTOP).Clone());
        rDoc.SetDefault( *m_pTabStop );
        m_pTabStop = std::move( pOld );
    }
}

void SwUndoDefaultAttr::RedoImpl(::sw::UndoRedoContext & rContext)
{
    UndoImpl(rContext);
}

SwUndoMoveLeftMargin::SwUndoMoveLeftMargin(
    const SwPaM& rPam, bool bFlag, bool bMod )
    : SwUndo( bFlag ? SwUndoId::INC_LEFTMARGIN : SwUndoId::DEC_LEFTMARGIN, rPam.GetDoc() )
    , SwUndRng( rPam )
    , m_pHistory( new SwHistory )
    , m_bModulus( bMod )
{
}

SwUndoMoveLeftMargin::~SwUndoMoveLeftMargin()
{
}

void SwUndoMoveLeftMargin::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();

    // restore old values
    m_pHistory->TmpRollback( & rDoc, 0 );
    m_pHistory->SetTmpEnd( m_pHistory->Count() );

    AddUndoRedoPaM(rContext);
}

void SwUndoMoveLeftMargin::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    SwPaM & rPam = AddUndoRedoPaM(rContext);

    rDoc.MoveLeftMargin( rPam,
                         GetId() == SwUndoId::INC_LEFTMARGIN, m_bModulus );
}

void SwUndoMoveLeftMargin::RepeatImpl(::sw::RepeatContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    rDoc.MoveLeftMargin(rContext.GetRepeatPaM(), GetId() == SwUndoId::INC_LEFTMARGIN,
                        m_bModulus );
}

SwUndoChangeFootNote::SwUndoChangeFootNote(
    const SwPaM& rRange, const OUString& rText,
        bool const bIsEndNote)
    : SwUndo( SwUndoId::CHGFTN, rRange.GetDoc() ), SwUndRng( rRange )
    , m_pHistory( new SwHistory() )
    , m_Text( rText )
    , m_bEndNote( bIsEndNote )
{
}

SwUndoChangeFootNote::~SwUndoChangeFootNote()
{
}

void SwUndoChangeFootNote::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();

    m_pHistory->TmpRollback( &rDoc, 0 );
    m_pHistory->SetTmpEnd( m_pHistory->Count() );

    rDoc.GetFootnoteIdxs().UpdateAllFootnote();

    AddUndoRedoPaM(rContext);
}

void SwUndoChangeFootNote::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc( rContext.GetDoc() );
    SwPaM & rPaM = AddUndoRedoPaM(rContext);
    rDoc.SetCurFootnote(rPaM, m_Text, m_bEndNote);
    SetPaM(rPaM);
}

void SwUndoChangeFootNote::RepeatImpl(::sw::RepeatContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    rDoc.SetCurFootnote(rContext.GetRepeatPaM(), m_Text, m_bEndNote);
}

SwUndoFootNoteInfo::SwUndoFootNoteInfo( const SwFootnoteInfo &rInfo, const SwDoc* pDoc )
    : SwUndo( SwUndoId::FTNINFO, pDoc )
    , m_pFootNoteInfo( new SwFootnoteInfo( rInfo ) )
{
}

SwUndoFootNoteInfo::~SwUndoFootNoteInfo()
{
}

void SwUndoFootNoteInfo::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    SwFootnoteInfo *pInf = new SwFootnoteInfo( rDoc.GetFootnoteInfo() );
    rDoc.SetFootnoteInfo( *m_pFootNoteInfo );
    m_pFootNoteInfo.reset( pInf );
}

void SwUndoFootNoteInfo::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    SwFootnoteInfo *pInf = new SwFootnoteInfo( rDoc.GetFootnoteInfo() );
    rDoc.SetFootnoteInfo( *m_pFootNoteInfo );
    m_pFootNoteInfo.reset( pInf );
}

SwUndoEndNoteInfo::SwUndoEndNoteInfo( const SwEndNoteInfo &rInfo, const SwDoc* pDoc )
    : SwUndo( SwUndoId::FTNINFO, pDoc )
    , m_pEndNoteInfo( new SwEndNoteInfo( rInfo ) )
{
}

SwUndoEndNoteInfo::~SwUndoEndNoteInfo()
{
}

void SwUndoEndNoteInfo::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    SwEndNoteInfo *pInf = new SwEndNoteInfo( rDoc.GetEndNoteInfo() );
    rDoc.SetEndNoteInfo( *m_pEndNoteInfo );
    m_pEndNoteInfo.reset( pInf );
}

void SwUndoEndNoteInfo::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    SwEndNoteInfo *pInf = new SwEndNoteInfo( rDoc.GetEndNoteInfo() );
    rDoc.SetEndNoteInfo( *m_pEndNoteInfo );
    m_pEndNoteInfo.reset( pInf );
}

SwUndoDontExpandFormat::SwUndoDontExpandFormat( const SwPosition& rPos )
    : SwUndo( SwUndoId::DONTEXPAND, rPos.GetDoc() )
    , m_nNodeIndex( rPos.nNode.GetIndex() )
    , m_nContentIndex( rPos.nContent.GetIndex() )
{
}

void SwUndoDontExpandFormat::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwPaM *const pPam(& rContext.GetCursorSupplier().CreateNewShellCursor());
    SwDoc *const pDoc = & rContext.GetDoc();

    SwPosition& rPos = *pPam->GetPoint();
    rPos.nNode = m_nNodeIndex;
    rPos.nContent.Assign( rPos.nNode.GetNode().GetContentNode(), m_nContentIndex);
    pDoc->DontExpandFormat( rPos, false );
}

void SwUndoDontExpandFormat::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwPaM *const pPam(& rContext.GetCursorSupplier().CreateNewShellCursor());
    SwDoc *const pDoc = & rContext.GetDoc();

    SwPosition& rPos = *pPam->GetPoint();
    rPos.nNode = m_nNodeIndex;
    rPos.nContent.Assign( rPos.nNode.GetNode().GetContentNode(), m_nContentIndex);
    pDoc->DontExpandFormat( rPos );
}

void SwUndoDontExpandFormat::RepeatImpl(::sw::RepeatContext & rContext)
{
    SwPaM & rPam = rContext.GetRepeatPaM();
    SwDoc & rDoc = rContext.GetDoc();
    rDoc.DontExpandFormat( *rPam.GetPoint() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
