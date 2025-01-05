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
#include <osl/diagnose.h>
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
#include <IDocumentState.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IShellCursorSupplier.hxx>
#include <docary.hxx>
#include <swcrsr.hxx>
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
#include <editsh.hxx>

SwUndoFormatAttrHelper::SwUndoFormatAttrHelper(SwFormat& rFormat, bool bSvDrwPt)
    : SwClient(&rFormat)
    , m_rFormat(rFormat)
    , m_bSaveDrawPt(bSvDrwPt)
{
}

void SwUndoFormatAttrHelper::SwClientNotify(const SwModify&, const SfxHint& rHint)
{
    if (rHint.GetId() == SfxHintId::SwAttrSetChange)
    {
        auto pChangeHint = static_cast<const sw::AttrSetChangeHint*>(&rHint);
        if(!pChangeHint->m_pOld)
            return;
        if(!pChangeHint->m_pNew)
            return;
        const SwDoc& rDoc = *m_rFormat.GetDoc();
        auto pOld = pChangeHint->m_pOld;
        auto& rChgSet = *pOld->GetChgSet();
        if(!GetUndo())
            m_pUndo.reset(new SwUndoFormatAttr(SfxItemSet(rChgSet), m_rFormat, m_bSaveDrawPt));
        else {
            SfxItemIter aIter(rChgSet);
            for(auto pItem = aIter.GetCurItem(); pItem; pItem = aIter.NextItem())
                m_pUndo->PutAttr(*pItem, rDoc);
        }
        return;
    }
    else if (rHint.GetId() == SfxHintId::SwObjectDying)
    {
        assert(false);
    }
    if (rHint.GetId() != SfxHintId::SwLegacyModify)
        return;
    auto pLegacy = static_cast<const sw::LegacyModifyHint*>(&rHint);
    if(!pLegacy->m_pOld)
        return;
    if(!pLegacy->m_pNew)
        return;
    const SwDoc& rDoc = *m_rFormat.GetDoc();
    auto pOld = pLegacy->m_pOld;
    if(POOLATTR_END >= pLegacy->m_pOld->Which())
    {
        if(!GetUndo())
            m_pUndo.reset(new SwUndoFormatAttr(*pOld, m_rFormat, m_bSaveDrawPt));
        else
            m_pUndo->PutAttr(*pOld, rDoc);
    }
}

SwDocModifyAndUndoGuard::SwDocModifyAndUndoGuard(SwFormat& format)
    : doc(format.GetName().isEmpty() ? nullptr : format.GetDoc())
    , helper(doc ? new SwUndoFormatAttrHelper(format) : nullptr)
{
}

SwDocModifyAndUndoGuard::~SwDocModifyAndUndoGuard()
{
    if (helper && helper->GetUndo())
    {
        // helper tracks changes, even when DoesUndo is false, to detect modified state
        if (doc->GetIDocumentUndoRedo().DoesUndo())
            doc->GetIDocumentUndoRedo().AppendUndo(helper->ReleaseUndo());

        doc->getIDocumentState().SetModified();
    }
}

SwUndoFormatAttr::SwUndoFormatAttr( SfxItemSet&& rOldSet,
                              SwFormat& rChgFormat,
                              bool bSaveDrawPt )
    : SwUndo( SwUndoId::INSFMTATTR, rChgFormat.GetDoc() )
    , m_sFormatName ( rChgFormat.GetName() )
    // #i56253#
    , m_oOldSet( std::move( rOldSet ) )
    , m_nAnchorContentOffset( 0 )
    , m_nNodeIndex( 0 )
    , m_nFormatWhich( rChgFormat.Which() )
    , m_bSaveDrawPt( bSaveDrawPt )
{
    assert(m_sFormatName.getLength());

    Init( rChgFormat );
}

SwUndoFormatAttr::SwUndoFormatAttr( const SfxPoolItem& rItem, SwFormat& rChgFormat,
                              bool bSaveDrawPt )
    : SwUndo( SwUndoId::INSFMTATTR, rChgFormat.GetDoc() )
    , m_sFormatName(rChgFormat.GetName())
    , m_oOldSet( rChgFormat.GetAttrSet().CloneAsValue( false ) )
    , m_nAnchorContentOffset( 0 )
    , m_nNodeIndex( 0 )
    , m_nFormatWhich( rChgFormat.Which() )
    , m_bSaveDrawPt( bSaveDrawPt )
{
    assert(m_sFormatName.getLength());

    m_oOldSet->Put( rItem );
    Init( rChgFormat );
}

void SwUndoFormatAttr::Init( const SwFormat & rFormat )
{
    // tdf#126017 never save SwNodeIndex, it will go stale
    m_oOldSet->ClearItem(RES_CNTNT);
    // treat change of anchor specially
    if ( SfxItemState::SET == m_oOldSet->GetItemState( RES_ANCHOR, false )) {
        SaveFlyAnchor( &rFormat, m_bSaveDrawPt );
    } else if ( RES_FRMFMT == m_nFormatWhich ) {
        const SwDoc* pDoc = rFormat.GetDoc();
        auto pTableFormat = dynamic_cast<const SwTableFormat*>(&rFormat);
        if (pTableFormat && pDoc->GetTableFrameFormats()->ContainsFormat(const_cast<SwTableFormat*>(pTableFormat)))
        {
            // Table Format: save table position, table formats are volatile!
            SwTable * pTable = SwIterator<SwTable,SwFormat>( rFormat ).First();
            if ( pTable ) {
                m_nNodeIndex = pTable->GetTabSortBoxes()[ 0 ]->GetSttNd()
                               ->FindTableNode()->GetIndex();
            }
        } else if (dynamic_cast<const SwSectionFormat*>(&rFormat)) {
            if (auto pContentIndex = rFormat.GetContent().GetContentIdx())
                m_nNodeIndex = pContentIndex->GetIndex();
        } else if(auto pBoxFormat = dynamic_cast<const SwTableBoxFormat*>(&rFormat))
        {
            auto pTableBox = pBoxFormat->GetTableBox();
            if(pTableBox)
                m_nNodeIndex = pTableBox->GetSttIdx();
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

    if (!m_oOldSet)
        return;

    SwFormat * pFormat = GetFormat(rContext.GetDoc());
    if (!pFormat)
        return;

    // #i35443# - If anchor attribute has been successful
    // restored, all other attributes are also restored.
    // Thus, keep track of its restoration
    bool bAnchorAttrRestored( false );
    if ( SfxItemState::SET == m_oOldSet->GetItemState( RES_ANCHOR, false )) {
        bAnchorAttrRestored = RestoreFlyAnchor(rContext);
        if ( bAnchorAttrRestored ) {
            // Anchor attribute successful restored.
            // Thus, keep anchor position for redo
            SaveFlyAnchor(pFormat);
        } else {
            // Anchor attribute not restored due to invalid anchor position.
            // Thus, delete anchor attribute.
            m_oOldSet->ClearItem( RES_ANCHOR );
        }
    }

    if ( bAnchorAttrRestored )        return;

    SwUndoFormatAttrHelper aTmp( *pFormat, m_bSaveDrawPt );
    pFormat->SetFormatAttr( *m_oOldSet );
    if ( aTmp.GetUndo() ) {
        // transfer ownership of helper object's old set
        if (aTmp.GetUndo()->m_oOldSet)
            m_oOldSet.emplace(std::move(*aTmp.GetUndo()->m_oOldSet));
        else
            m_oOldSet.reset();
    } else {
        m_oOldSet->ClearItem();
    }

    if ( RES_FLYFRMFMT == m_nFormatWhich || RES_DRAWFRMFMT == m_nFormatWhich ) {
        rContext.SetSelections(static_cast<SwFrameFormat*>(pFormat), nullptr);
    }

    SfxStyleFamily nFamily = SfxStyleFamily::None;
    if (RES_TXTFMTCOLL == m_nFormatWhich || RES_CONDTXTFMTCOLL == m_nFormatWhich)
        nFamily = SfxStyleFamily::Para;
    else if (RES_CHRFMT == m_nFormatWhich)
        nFamily = SfxStyleFamily::Char;

    if (m_oOldSet && m_oOldSet->Count() > 0 && nFamily != SfxStyleFamily::None)
        rContext.GetDoc().BroadcastStyleOperation(pFormat->GetName(), nFamily, SfxHintId::StyleSheetModified);
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
        return rDoc.GetGrfFormatColls()->FindFormatByName(m_sFormatName);

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
            auto it = rDoc.GetSpzFrameFormats()->findByTypeAndName( m_nFormatWhich, m_sFormatName );
            if( it != rDoc.GetSpzFrameFormats()->typeAndNameEnd() )
                return *it;
            SwFormat* pFormat = rDoc.GetFrameFormats()->FindFormatByName(m_sFormatName);
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
    if (!m_oOldSet)
        return;

    SwDoc & rDoc(rContext.GetDoc());

    SwFormat * pFormat = GetFormat(rDoc);
    if (!pFormat)
        return;

    switch ( m_nFormatWhich ) {
    case RES_GRFFMTCOLL: {
        SwNoTextNode *const pNd =
            rContext.GetRepeatPaM().GetPointNode().GetNoTextNode();
        if( pNd ) {
            rDoc.SetAttr( pFormat->GetAttrSet(), *pNd->GetFormatColl() );
        }
    }
    break;

    case RES_TXTFMTCOLL:
    case RES_CONDTXTFMTCOLL:
    {
        SwTextNode *const pNd =
            rContext.GetRepeatPaM().GetPointNode().GetTextNode();
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
            rContext.GetRepeatPaM().GetPointNode().GetFlyFormat();
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
    m_oOldSet->Put( rItem );
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
            m_oOldSet->Put( SwFormatFrameSize( SwFrameSize::Variable, aPt.X(), aPt.Y() ) );
        }
    }

    const SwFormatAnchor& rAnchor =
        m_oOldSet->Get( RES_ANCHOR, false );
    if( !rAnchor.GetAnchorNode() || rAnchor.GetAnchorId() == RndStdIds::FLY_AT_PAGE)
        return;

    switch( rAnchor.GetAnchorId() ) {
    case RndStdIds::FLY_AS_CHAR:
    case RndStdIds::FLY_AT_CHAR:
        m_nAnchorContentOffset = rAnchor.GetAnchorContentOffset();
        [[fallthrough]];
    case RndStdIds::FLY_AT_PARA:
    case RndStdIds::FLY_AT_FLY:
        m_nNodeIndex = rAnchor.GetAnchorNode()->GetIndex();
        break;
    default:
        assert(false);
    }

    SwFormatAnchor aAnchor( rAnchor.GetAnchorId(), 0 );
    m_oOldSet->Put( aAnchor );
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
        m_oOldSet->Get( RES_ANCHOR, false );

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
            aPos.SetContent( m_nAnchorContentOffset );
            if ( aPos.GetContentIndex() > pNd->GetTextNode()->GetText().getLength()) {
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
            const SwFormatFrameSize& rOldSize = m_oOldSet->Get( RES_FRM_SIZE );
            aDrawSavePt.setX( rOldSize.GetWidth() );
            aDrawSavePt.setY( rOldSize.GetHeight() );
            m_oOldSet->ClearItem( RES_FRM_SIZE );

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
        SwTextNode *pTextNode = static_cast<SwTextNode*>(rOldAnch.GetAnchorNode());
        OSL_ENSURE( pTextNode->HasHints(), "Missing FlyInCnt-Hint." );
        const sal_Int32 nIdx = rOldAnch.GetAnchorContentOffset();
        SwTextAttr * const pHint =
            pTextNode->GetTextAttrForCharAt( nIdx, RES_TXTATR_FLYCNT );
        assert(pHint && "Missing Hint.");
        OSL_ENSURE( pHint->Which() == RES_TXTATR_FLYCNT,
                    "Missing FlyInCnt-Hint." );
        OSL_ENSURE( pHint->GetFlyCnt().GetFrameFormat() == pFrameFormat,
                    "Wrong TextFlyCnt-Hint." );
        const_cast<SwFormatFlyCnt&>(pHint->GetFlyCnt()).SetFlyFormat();

        // Connection is now detached, therefore the attribute can be deleted
        pTextNode->DeleteAttributes( RES_TXTATR_FLYCNT, nIdx, nIdx );
    }

    {
        m_oOldSet->Put( aNewAnchor );
        SwUndoFormatAttrHelper aTmp( *pFrameFormat, m_bSaveDrawPt );
        pFrameFormat->SetFormatAttr( *m_oOldSet );
        if ( aTmp.GetUndo() ) {
            m_nNodeIndex = aTmp.GetUndo()->m_nNodeIndex;
            // transfer ownership of helper object's old set
            if (aTmp.GetUndo()->m_oOldSet)
                m_oOldSet.emplace(std::move(*aTmp.GetUndo()->m_oOldSet));
            else
                m_oOldSet.reset();
        } else {
            m_oOldSet->ClearItem();
        }
    }

    if ( RES_DRAWFRMFMT == pFrameFormat->Which() )
    {
        // The Draw model also prepared an Undo object for its right positioning
        // which unfortunately is relative. Therefore block here a position
        // change of the Contact object by setting the anchor.
        const SwFormatVertOrient& rVertOrient = pFrameFormat->GetVertOrient();
        const SwFormatHoriOrient& rHoriOrient = pFrameFormat->GetHoriOrient();
        Point aFormatPos(rHoriOrient.GetPos(), rVertOrient.GetPos());
        if (aDrawSavePt != aFormatPos)
        {
            // If the position would be the same, then skip the call: either it would do nothing or
            // it would just go wrong.
            pFrameFormat->CallSwClientNotify(sw::RestoreFlyAnchorHint(aDrawSavePt));
        }

        // cache the old value again
        m_oOldSet->Put(SwFormatFrameSize(SwFrameSize::Variable, aDrawOldPt.X(), aDrawOldPt.Y()));
    }

    if (RndStdIds::FLY_AS_CHAR == aNewAnchor.GetAnchorId()) {
        SwTextNode* pTextNd = aNewAnchor.GetAnchorNode()->GetTextNode();
        OSL_ENSURE( pTextNd, "no Text Node at position." );
        SwFormatFlyCnt aFormat( pFrameFormat );
        pTextNd->InsertItem( aFormat, aNewAnchor.GetAnchorContentOffset(), 0 );
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
                                              const std::vector<sal_uInt16>& rIds )
    : SwUndo( SwUndoId::RESETATTR, rChangedFormat.GetDoc() )
    , m_pChangedFormat( &rChangedFormat )
    , m_aSet(*rChangedFormat.GetAttrSet().GetPool())
{
    for (const auto& nWhichId : rIds)
    {
        const SfxPoolItem* pItem = nullptr;
        if (rChangedFormat.GetItemState(nWhichId, false, &pItem ) == SfxItemState::SET && pItem)
            m_aSet.Put(*pItem);
    }
}

SwUndoFormatResetAttr::~SwUndoFormatResetAttr()
{
}

void SwUndoFormatResetAttr::UndoImpl(::sw::UndoRedoContext &)
{
    m_pChangedFormat->SetFormatAttr(m_aSet);
    BroadcastStyleChange();
}

void SwUndoFormatResetAttr::RedoImpl(::sw::UndoRedoContext &)
{
    SfxItemIter aIter(m_aSet);
    for (auto pItem = aIter.GetCurItem(); pItem; pItem = aIter.NextItem())
        m_pChangedFormat->ResetFormatAttr(pItem->Which());
    BroadcastStyleChange();
}

void SwUndoFormatResetAttr::BroadcastStyleChange()
{
    auto nWhich = m_pChangedFormat->Which();
    SfxStyleFamily nFamily = SfxStyleFamily::None;

    if (RES_TXTFMTCOLL == nWhich || RES_CONDTXTFMTCOLL == nWhich)
        nFamily = SfxStyleFamily::Para;
    else if (RES_CHRFMT == nWhich)
        nFamily = SfxStyleFamily::Char;

    if (nFamily != SfxStyleFamily::None)
        m_pChangedFormat->GetDoc()->BroadcastStyleOperation(m_pChangedFormat->GetName(), nFamily, SfxHintId::StyleSheetModified);
}

SwUndoResetAttr::SwUndoResetAttr( const SwPaM& rRange, sal_uInt16 nFormatId )
    : SwUndo( SwUndoId::RESETATTR, &rRange.GetDoc() ), SwUndRng( rRange )
    , m_pHistory( new SwHistory )
    , m_nFormatId( nFormatId )
{
}

SwUndoResetAttr::SwUndoResetAttr( const SwPosition& rPos, sal_uInt16 nFormatId )
    : SwUndo( SwUndoId::RESETATTR, &rPos.GetDoc() )
    , m_pHistory( new SwHistory )
    , m_nFormatId( nFormatId )
{
    m_nSttNode = m_nEndNode = rPos.GetNodeIndex();
    m_nSttContent = m_nEndContent = rPos.GetContentIndex();
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
        if( pTNd )
            pTNd->DontExpandFormat( m_nSttContent, false );
    }
    else if (m_nFormatId == RES_TXTATR_REFMARK)
    {
        rDoc.GetEditShell()->SwViewShell::UpdateFields();
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
        SwPosition aPos( aIdx, aIdx.GetNode().GetContentNode(), m_nSttContent );

        int nCnt = SwDoc::GetCurTOXMark( aPos, aArr );
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
    case RES_TXTATR_REFMARK:
    {
        SwHistoryHint* pHistoryHint = GetHistory()[0];
        if (pHistoryHint && HSTRY_SETREFMARKHNT == pHistoryHint->Which())
        {
            rDoc.ForEachRefMark(
                [&pHistoryHint, &rDoc] (const SwFormatRefMark& rFormatRefMark) -> bool
                {
                    if (static_cast<SwHistorySetRefMark*>(pHistoryHint)->GetRefName() ==
                            rFormatRefMark.GetRefName())
                    {
                        rDoc.DeleteFormatRefMark(&rFormatRefMark);
                        rDoc.GetEditShell()->SwViewShell::UpdateFields();
                        return false;
                    }
                    return true;
                });
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

void SwUndoResetAttr::SetAttrs( o3tl::sorted_vector<sal_uInt16> && rAttrs )
{
    m_Ids = std::move(rAttrs);
}

SwUndoAttr::SwUndoAttr( const SwPaM& rRange, const SfxPoolItem& rAttr,
                        const SetAttrMode nFlags )
    : SwUndo( SwUndoId::INSATTR, &rRange.GetDoc() ), SwUndRng( rRange )
    , m_AttrSet( rRange.GetDoc().GetAttrPool(), rAttr.Which(), rAttr.Which() )
    , m_pHistory( new SwHistory )
    , m_nNodeIndex( NODE_OFFSET_MAX )
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

SwUndoAttr::SwUndoAttr( const SwPaM& rRange, SfxItemSet aSet,
                        const SetAttrMode nFlags )
    : SwUndo( SwUndoId::INSATTR, &rRange.GetDoc() ), SwUndRng( rRange )
    , m_AttrSet(std::move( aSet ))
    , m_pHistory( new SwHistory )
    , m_nNodeIndex( NODE_OFFSET_MAX )
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
    SwDoc& rDoc = rPam.GetDoc();
    if ( rDoc.getIDocumentRedlineAccess().IsRedlineOn() ) {
        m_pRedlineData.reset( new SwRedlineData( bIsContent
                              ? RedlineType::Insert
                              : RedlineType::Format,
                              rDoc.getIDocumentRedlineAccess().GetRedlineAuthor() ) );
    }

    m_pRedlineSaveData.reset( new SwRedlineSaveDatas );
    if ( !FillSaveDataForFormat( rPam, *m_pRedlineSaveData ))
        m_pRedlineSaveData.reset();

    SetRedlineFlags( rDoc.getIDocumentRedlineAccess().GetRedlineFlags() );
    if ( bIsContent ) {
        m_nNodeIndex = rPam.GetPoint()->GetNodeIndex();
    }
}

void SwUndoAttr::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc *const pDoc = & rContext.GetDoc();

    RemoveIdx( *pDoc );

    if( IDocumentRedlineAccess::IsRedlineOn( GetRedlineFlags() ) ) {
        SwPaM aPam(pDoc->GetNodes().GetEndOfContent());
        if ( NODE_OFFSET_MAX != m_nNodeIndex ) {
            aPam.DeleteMark();
            aPam.GetPoint()->Assign( m_nNodeIndex, m_nSttContent );
            aPam.SetMark();
            aPam.GetPoint()->AdjustContent(+1);
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
                          && (RES_TXTATR_FIELD <= m_AttrSet.GetRanges()[0].first)
                          && (m_AttrSet.GetRanges()[0].first <= RES_TXTATR_ANNOTATION);

    // restore old values
    m_pHistory->TmpRollback( pDoc, 0, !bToLast );
    m_pHistory->SetTmpEnd( m_pHistory->Count() );

    // set cursor onto Undo area
    if (!(m_nInsertFlags & SetAttrMode::NO_CURSOR_CHANGE))
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

void SwUndoAttr::redoAttribute(SwPaM& rPam, sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();

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

        if ( NODE_OFFSET_MAX != m_nNodeIndex ) {
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

void SwUndoAttr::RedoImpl(sw::UndoRedoContext & rContext)
{
    if (m_nInsertFlags & SetAttrMode::NO_CURSOR_CHANGE)
    {
        SwPaM aPam(rContext.GetDoc().GetNodes().GetEndOfContent());
        SetPaM(aPam, false);
        redoAttribute(aPam, rContext);
    }
    else
    {
        SwPaM& rPam = AddUndoRedoPaM(rContext);
        redoAttribute(rPam, rContext);
    }
}

void SwUndoAttr::RemoveIdx( SwDoc& rDoc )
{
    if ( SfxItemState::SET != m_AttrSet.GetItemState( RES_TXTATR_FTN, false ))
        return ;

    SwNodes& rNds = rDoc.GetNodes();
    for ( sal_uInt16 n = 0; n < m_pHistory->Count(); ++n ) {
        sal_Int32 nContent = 0;
        SwNodeOffset nNode(0);
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

SwUndoDefaultAttr::SwUndoDefaultAttr( const SfxItemSet& rSet, const SwDoc& rDoc )
    : SwUndo( SwUndoId::SETDEFTATTR, &rDoc )
{
    const SvxTabStopItem* pItem = rSet.GetItemIfSet( RES_PARATR_TABSTOP, false );
    if( pItem )
    {
        // store separately, because it may change!
        m_pTabStop.reset(pItem->Clone());
        if ( 1 != rSet.Count() ) { // are there more attributes?
            m_oOldSet.emplace( rSet );
        }
    } else {
        m_oOldSet.emplace( rSet );
    }
}

SwUndoDefaultAttr::~SwUndoDefaultAttr()
{
}

void SwUndoDefaultAttr::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    if (m_oOldSet)
    {
        SwUndoFormatAttrHelper aTmp(
            *rDoc.GetDfltTextFormatColl() );
        rDoc.SetDefault( *m_oOldSet );
        m_oOldSet.reset();
        if ( aTmp.GetUndo() ) {
            // transfer ownership of helper object's old set
            if (aTmp.GetUndo()->m_oOldSet)
                m_oOldSet.emplace(std::move(*aTmp.GetUndo()->m_oOldSet));
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
    : SwUndo( bFlag ? SwUndoId::INC_LEFTMARGIN : SwUndoId::DEC_LEFTMARGIN, &rPam.GetDoc() )
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
                         GetId() == SwUndoId::INC_LEFTMARGIN, m_bModulus,
                         rDoc.getIDocumentLayoutAccess().GetCurrentLayout() );
}

void SwUndoMoveLeftMargin::RepeatImpl(::sw::RepeatContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    rDoc.MoveLeftMargin(rContext.GetRepeatPaM(), GetId() == SwUndoId::INC_LEFTMARGIN,
                        m_bModulus, rDoc.getIDocumentLayoutAccess().GetCurrentLayout());
}

SwUndoChangeFootNote::SwUndoChangeFootNote(
    const SwPaM& rRange, OUString aText,
        bool const bIsEndNote)
    : SwUndo( SwUndoId::CHGFTN, &rRange.GetDoc() ), SwUndRng( rRange )
    , m_pHistory( new SwHistory() )
    , m_Text(std::move( aText ))
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

SwUndoFootNoteInfo::SwUndoFootNoteInfo( const SwFootnoteInfo &rInfo, const SwDoc& rDoc )
    : SwUndo( SwUndoId::FTNINFO, &rDoc )
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

SwUndoEndNoteInfo::SwUndoEndNoteInfo( const SwEndNoteInfo &rInfo, const SwDoc& rDoc )
    : SwUndo( SwUndoId::FTNINFO, &rDoc )
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
    : SwUndo( SwUndoId::DONTEXPAND, &rPos.GetDoc() )
    , m_nNodeIndex( rPos.GetNodeIndex() )
    , m_nContentIndex( rPos.GetContentIndex() )
{
}

void SwUndoDontExpandFormat::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwCursor *const pPam(& rContext.GetCursorSupplier().CreateNewShellCursor());
    SwDoc *const pDoc = & rContext.GetDoc();

    SwPosition& rPos = *pPam->GetPoint();
    rPos.Assign( m_nNodeIndex, m_nContentIndex );
    pDoc->DontExpandFormat( rPos, false );
}

void SwUndoDontExpandFormat::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwPaM *const pPam(& rContext.GetCursorSupplier().CreateNewShellCursor());
    SwDoc *const pDoc = & rContext.GetDoc();

    SwPosition& rPos = *pPam->GetPoint();
    rPos.Assign( m_nNodeIndex, m_nContentIndex );
    pDoc->DontExpandFormat( rPos );
}

void SwUndoDontExpandFormat::RepeatImpl(::sw::RepeatContext & rContext)
{
    SwPaM & rPam = rContext.GetRepeatPaM();
    SwDoc & rDoc = rContext.GetDoc();
    rDoc.DontExpandFormat( *rPam.GetPoint() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
