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
#include <UndoSection.hxx>

#include <osl/diagnose.h>
#include <comphelper/scopeguard.hxx>
#include <sfx2/linkmgr.hxx>
#include <fmtcntnt.hxx>
#include <doc.hxx>
#include <IDocumentLinksAdministration.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <poolfmt.hxx>
#include <docary.hxx>
#include <swundo.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <UndoCore.hxx>
#include <section.hxx>
#include <rolbck.hxx>
#include <redline.hxx>
#include <doctxm.hxx>
#include <ftnidx.hxx>
#include <rootfrm.hxx>
#include <editsh.hxx>
/// OD 04.10.2002 #102894#
/// class Calc needed for calculation of the hidden condition of a section.
#include <calc.hxx>

static std::unique_ptr<SfxItemSet> lcl_GetAttrSet( const SwSection& rSect )
{
    // save attributes of the format (columns, color, ...)
    // Content and Protect items are not interesting since they are already
    // stored in Section, thus delete them.
    std::unique_ptr<SfxItemSet> pAttr;
    if( rSect.GetFormat() )
    {
        sal_uInt16 nCnt = 1;
        if( rSect.IsProtect() )
            ++nCnt;

        if( nCnt < rSect.GetFormat()->GetAttrSet().Count() )
        {
            pAttr.reset(new SfxItemSet( rSect.GetFormat()->GetAttrSet() ));
            pAttr->ClearItem( RES_PROTECT );
            pAttr->ClearItem( RES_CNTNT );
            if( !pAttr->Count() )
            {
                pAttr.reset();
            }
        }
    }
    return pAttr;
}

SwUndoInsSection::SwUndoInsSection(
        SwPaM const& rPam, SwSectionData const& rNewData,
        SfxItemSet const*const pSet,
        std::tuple<SwTOXBase const*, sw::RedlineMode, sw::FieldmarkMode> const*const pTOXBase)
    : SwUndo( SwUndoId::INSSECTION, &rPam.GetDoc() ), SwUndRng( rPam )
    , m_pSectionData(new SwSectionData(rNewData))
    , m_pTOXBase( pTOXBase
        ? std::make_unique<std::tuple<SwTOXBase *, sw::RedlineMode, sw::FieldmarkMode>>(
            new SwTOXBase(*std::get<0>(*pTOXBase)), std::get<1>(*pTOXBase), std::get<2>(*pTOXBase))
        : nullptr )
    , m_pAttrSet( (pSet && pSet->Count()) ? new SfxItemSet( *pSet ) : nullptr )
    , m_nSectionNodePos(0)
    , m_bSplitAtStart(false)
    , m_bSplitAtEnd(false)
    , m_bUpdateFootnote(false)
{
    SwDoc& rDoc = rPam.GetDoc();
    if( rDoc.getIDocumentRedlineAccess().IsRedlineOn() )
    {
        m_pRedlData.reset(new SwRedlineData( RedlineType::Insert,
                                        rDoc.getIDocumentRedlineAccess().GetRedlineAuthor() ));
        SetRedlineFlags( rDoc.getIDocumentRedlineAccess().GetRedlineFlags() );
    }
    m_pRedlineSaveData.reset( new SwRedlineSaveDatas );
    if( !FillSaveData( rPam, *m_pRedlineSaveData, false ))
            m_pRedlineSaveData.reset();

    if( rPam.HasMark() )
        return;

    const SwContentNode* pCNd = rPam.GetPoint()->nNode.GetNode().GetContentNode();
    if( pCNd && pCNd->HasSwAttrSet() && (
        !rPam.GetPoint()->nContent.GetIndex() ||
        rPam.GetPoint()->nContent.GetIndex() == pCNd->Len() ))
    {
        SfxItemSet aBrkSet( rDoc.GetAttrPool(), aBreakSetRange );
        aBrkSet.Put( *pCNd->GetpSwAttrSet() );
        if( aBrkSet.Count() )
        {
            m_pHistory.reset( new SwHistory );
            m_pHistory->CopyFormatAttr( aBrkSet, pCNd->GetIndex() );
        }
    }
}

SwUndoInsSection::~SwUndoInsSection()
{
}

void SwUndoInsSection::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();

    RemoveIdxFromSection( rDoc, m_nSectionNodePos );

    SwSectionNode *const pNd =
        rDoc.GetNodes()[ m_nSectionNodePos ]->GetSectionNode();
    OSL_ENSURE( pNd, "where is my SectionNode?" );

    if( IDocumentRedlineAccess::IsRedlineOn( GetRedlineFlags() ))
        rDoc.getIDocumentRedlineAccess().DeleteRedline( *pNd, true, RedlineType::Any );

    // no selection?
    SwNodeIndex aIdx( *pNd );
    if( ( !m_nEndNode && COMPLETE_STRING == m_nEndContent ) ||
        ( m_nSttNode == m_nEndNode && m_nSttContent == m_nEndContent ))
        // delete simply all nodes
        rDoc.GetNodes().Delete( aIdx, pNd->EndOfSectionIndex() -
                                        aIdx.GetIndex() );
    else
        // just delete format, rest happens automatically
        rDoc.DelSectionFormat( pNd->GetSection().GetFormat() );

    // do we need to consolidate?
    if (m_bSplitAtStart)
    {
        Join( rDoc, m_nSttNode );
    }

    if (m_bSplitAtEnd)
    {
        Join( rDoc, m_nEndNode );
    }

    if (m_pHistory)
    {
        m_pHistory->TmpRollback( &rDoc, 0, false );
    }

    if (m_bUpdateFootnote)
    {
        rDoc.GetFootnoteIdxs().UpdateFootnote( aIdx );
    }

    AddUndoRedoPaM(rContext);

    if (m_pRedlineSaveData)
        SetSaveData( rDoc, *m_pRedlineSaveData );
}

void SwUndoInsSection::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    SwPaM & rPam( AddUndoRedoPaM(rContext) );

    const SwTOXBaseSection* pUpdateTOX = nullptr;
    if (m_pTOXBase)
    {
        SwRootFrame const* pLayout(nullptr);
        SwRootFrame * pLayoutToReset(nullptr);
        sw::FieldmarkMode eFieldmarkMode{};
        comphelper::ScopeGuard g([&]() {
                if (pLayoutToReset)
                {
                    pLayoutToReset->SetHideRedlines(std::get<1>(*m_pTOXBase) == sw::RedlineMode::Shown);
                    pLayoutToReset->SetFieldmarkMode(eFieldmarkMode);
                }
            });
        o3tl::sorted_vector<SwRootFrame *> layouts(rDoc.GetAllLayouts());
        for (SwRootFrame const*const p : layouts)
        {
            if ((std::get<1>(*m_pTOXBase) == sw::RedlineMode::Hidden) == p->IsHideRedlines()
                && std::get<2>(*m_pTOXBase) == p->GetFieldmarkMode())
            {
                pLayout = p;
                break;
            }
        }
        if (!pLayout)
        {
            assert(!layouts.empty()); // must have one layout
            pLayoutToReset = *layouts.begin();
            eFieldmarkMode = pLayoutToReset->GetFieldmarkMode();
            pLayoutToReset->SetHideRedlines(std::get<1>(*m_pTOXBase) == sw::RedlineMode::Hidden);
            pLayoutToReset->SetFieldmarkMode(std::get<2>(*m_pTOXBase));
            pLayout = pLayoutToReset;
        }
        pUpdateTOX = rDoc.InsertTableOf( *rPam.GetPoint(),
            // don't expand: will be done by SwUndoUpdateIndex::RedoImpl()
            *std::get<0>(*m_pTOXBase), m_pAttrSet.get(), false, pLayout);
    }
    else
    {
        rDoc.InsertSwSection(rPam, *m_pSectionData, nullptr, m_pAttrSet.get());
    }

    if (m_pHistory)
    {
        m_pHistory->SetTmpEnd( m_pHistory->Count() );
    }

    SwSectionNode *const pSectNd =
        rDoc.GetNodes()[ m_nSectionNodePos ]->GetSectionNode();
    if (m_pRedlData &&
        IDocumentRedlineAccess::IsRedlineOn( GetRedlineFlags()))
    {
        RedlineFlags eOld = rDoc.getIDocumentRedlineAccess().GetRedlineFlags();
        rDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern(eOld & ~RedlineFlags::Ignore);

        SwPaM aPam( *pSectNd->EndOfSectionNode(), *pSectNd, 1 );
        rDoc.getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline( *m_pRedlData, aPam ), true);
        rDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
    }
    else if( !( RedlineFlags::Ignore & GetRedlineFlags() ) &&
            !rDoc.getIDocumentRedlineAccess().GetRedlineTable().empty() )
    {
        SwPaM aPam( *pSectNd->EndOfSectionNode(), *pSectNd, 1 );
        rDoc.getIDocumentRedlineAccess().SplitRedline( aPam );
    }

    if( pUpdateTOX )
    {
        // initiate formatting
        SwEditShell* pESh = rDoc.GetEditShell();
        if( pESh )
            pESh->CalcLayout();

        // insert page numbers
        const_cast<SwTOXBaseSection*>(pUpdateTOX)->UpdatePageNum();
    }
}

void SwUndoInsSection::RepeatImpl(::sw::RepeatContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    if (m_pTOXBase)
    {
        rDoc.InsertTableOf(*rContext.GetRepeatPaM().GetPoint(),
            *std::get<0>(*m_pTOXBase), m_pAttrSet.get(), true,
            rDoc.getIDocumentLayoutAccess().GetCurrentLayout()); // TODO add shell to RepeatContext?
    }
    else
    {
        rDoc.InsertSwSection(rContext.GetRepeatPaM(),
            *m_pSectionData, nullptr, m_pAttrSet.get());
    }
}

void SwUndoInsSection::Join( SwDoc& rDoc, sal_uLong nNode )
{
    SwNodeIndex aIdx( rDoc.GetNodes(), nNode );
    SwTextNode* pTextNd = aIdx.GetNode().GetTextNode();
    OSL_ENSURE( pTextNd, "Where is my TextNode?" );

    {
        RemoveIdxRel(
            nNode + 1,
            SwPosition( aIdx, SwIndex( pTextNd, pTextNd->GetText().getLength() ) ) );
    }
    pTextNd->JoinNext();

    if (m_pHistory)
    {
        SwIndex aCntIdx( pTextNd, 0 );
        pTextNd->RstTextAttr( aCntIdx, pTextNd->Len(), 0, nullptr, true );
    }
}

void
SwUndoInsSection::SaveSplitNode(SwTextNode *const pTextNd, bool const bAtStart)
{
    if( pTextNd->GetpSwpHints() )
    {
        if (!m_pHistory)
        {
            m_pHistory.reset( new SwHistory );
        }
        m_pHistory->CopyAttr( pTextNd->GetpSwpHints(), pTextNd->GetIndex(), 0,
                            pTextNd->GetText().getLength(), false );
    }

    if (bAtStart)
    {
        m_bSplitAtStart = true;
    }
    else
    {
        m_bSplitAtEnd = true;
    }
}

class SwUndoDelSection
    : public SwUndo
{
private:
    std::unique_ptr<SwSectionData> const m_pSectionData; /// section not TOX
    std::unique_ptr<SwTOXBase> const m_pTOXBase; /// set iff section is TOX
    std::unique_ptr<SfxItemSet> const m_pAttrSet;
    std::shared_ptr< ::sfx2::MetadatableUndo > const m_pMetadataUndo;
    sal_uLong const m_nStartNode;
    sal_uLong const m_nEndNode;

public:
    SwUndoDelSection(
        SwSectionFormat const&, SwSection const&, SwNodeIndex const*const);

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
};

std::unique_ptr<SwUndo> MakeUndoDelSection(SwSectionFormat const& rFormat)
{
    return std::make_unique<SwUndoDelSection>(rFormat, *rFormat.GetSection(),
                rFormat.GetContent().GetContentIdx());
}

SwUndoDelSection::SwUndoDelSection(
            SwSectionFormat const& rSectionFormat, SwSection const& rSection,
            SwNodeIndex const*const pIndex)
    : SwUndo( SwUndoId::DELSECTION, rSectionFormat.GetDoc() )
    , m_pSectionData( new SwSectionData(rSection) )
    , m_pTOXBase( dynamic_cast<const SwTOXBaseSection*>( &rSection) !=  nullptr
            ? new SwTOXBase(static_cast<SwTOXBaseSection const&>(rSection))
            : nullptr )
    , m_pAttrSet( ::lcl_GetAttrSet(rSection) )
    , m_pMetadataUndo( rSectionFormat.CreateUndo() )
    , m_nStartNode( pIndex->GetIndex() )
    , m_nEndNode( pIndex->GetNode().EndOfSectionIndex() )
{
}

void SwUndoDelSection::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();

    if (m_pTOXBase)
    {
        // sw_redlinehide: this should work as-is; there will be another undo for the update
        rDoc.InsertTableOf(m_nStartNode, m_nEndNode-2, *m_pTOXBase,
                m_pAttrSet.get());
    }
    else
    {
        SwNodeIndex aStt( rDoc.GetNodes(), m_nStartNode );
        SwNodeIndex aEnd( rDoc.GetNodes(), m_nEndNode-2 );
        SwSectionFormat* pFormat = rDoc.MakeSectionFormat();
        if (m_pAttrSet)
        {
            pFormat->SetFormatAttr( *m_pAttrSet );
        }

        /// OD 04.10.2002 #102894#
        /// remember inserted section node for further calculations
        SwSectionNode* pInsertedSectNd = rDoc.GetNodes().InsertTextSection(
                aStt, *pFormat, *m_pSectionData, nullptr, & aEnd);

        if( SfxItemState::SET == pFormat->GetItemState( RES_FTN_AT_TXTEND ) ||
            SfxItemState::SET == pFormat->GetItemState( RES_END_AT_TXTEND ))
        {
            rDoc.GetFootnoteIdxs().UpdateFootnote( aStt );
        }

        /// OD 04.10.2002 #102894#
        /// consider that section is hidden by condition.
        /// If section is hidden by condition,
        /// recalculate condition and update hidden condition flag.
        /// Recalculation is necessary, because fields, on which the hide
        /// condition depends, can be changed - fields changes aren't undoable.
        /// NOTE: setting hidden condition flag also creates/deletes corresponding
        ///     frames, if the hidden condition flag changes.
        SwSection& aInsertedSect = pInsertedSectNd->GetSection();
        if ( aInsertedSect.IsHidden() &&
             !aInsertedSect.GetCondition().isEmpty() )
        {
            SwCalc aCalc( rDoc );
            rDoc.getIDocumentFieldsAccess().FieldsToCalc(aCalc, pInsertedSectNd->GetIndex(), SAL_MAX_INT32);
            bool bRecalcCondHidden =
                    aCalc.Calculate( aInsertedSect.GetCondition() ).GetBool();
            aInsertedSect.SetCondHidden( bRecalcCondHidden );
        }

        pFormat->RestoreMetadata(m_pMetadataUndo);
    }
}

void SwUndoDelSection::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();

    SwSectionNode *const pNd =
        rDoc.GetNodes()[ m_nStartNode ]->GetSectionNode();
    OSL_ENSURE( pNd, "Where is my SectionNode?" );
    // just delete format, rest happens automatically
    rDoc.DelSectionFormat( pNd->GetSection().GetFormat() );
}

namespace {

class SwUndoUpdateSection
    : public SwUndo
{
private:
    std::unique_ptr<SwSectionData> m_pSectionData;
    std::unique_ptr<SfxItemSet> m_pAttrSet;
    sal_uLong const m_nStartNode;
    bool const m_bOnlyAttrChanged;

public:
    SwUndoUpdateSection(
        SwSection const&, SwNodeIndex const*const, bool const bOnlyAttr);

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
};

}

std::unique_ptr<SwUndo>
MakeUndoUpdateSection(SwSectionFormat const& rFormat, bool const bOnlyAttr)
{
    return std::make_unique<SwUndoUpdateSection>(*rFormat.GetSection(),
                rFormat.GetContent().GetContentIdx(), bOnlyAttr);
}

SwUndoUpdateSection::SwUndoUpdateSection(
        SwSection const& rSection, SwNodeIndex const*const pIndex,
        bool const bOnlyAttr)
    : SwUndo( SwUndoId::CHGSECTION, &pIndex->GetNode().GetDoc() )
    , m_pSectionData( new SwSectionData(rSection) )
    , m_pAttrSet( ::lcl_GetAttrSet(rSection) )
    , m_nStartNode( pIndex->GetIndex() )
    , m_bOnlyAttrChanged( bOnlyAttr )
{
}

void SwUndoUpdateSection::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    SwSectionNode *const pSectNd =
        rDoc.GetNodes()[ m_nStartNode ]->GetSectionNode();
    OSL_ENSURE( pSectNd, "Where is my SectionNode?" );

    SwSection& rNdSect = pSectNd->GetSection();
    SwFormat* pFormat = rNdSect.GetFormat();

    std::unique_ptr<SfxItemSet> pCur = ::lcl_GetAttrSet( rNdSect );
    if (m_pAttrSet)
    {
        // The Content and Protect items must persist
        const SfxPoolItem* pItem;
        m_pAttrSet->Put( pFormat->GetFormatAttr( RES_CNTNT ));
        if( SfxItemState::SET == pFormat->GetItemState( RES_PROTECT, true, &pItem ))
        {
            m_pAttrSet->Put( *pItem );
        }
        pFormat->DelDiffs( *m_pAttrSet );
        m_pAttrSet->ClearItem( RES_CNTNT );
        pFormat->SetFormatAttr( *m_pAttrSet );
    }
    else
    {
        // than the old ones need to be deleted
        pFormat->ResetFormatAttr( RES_FRMATR_BEGIN, RES_BREAK );
        pFormat->ResetFormatAttr( RES_HEADER, RES_OPAQUE );
        pFormat->ResetFormatAttr( RES_SURROUND, RES_FRMATR_END-1 );
    }
    m_pAttrSet = std::move(pCur);

    if (m_bOnlyAttrChanged)
        return;

    const bool bUpdate =
           (!rNdSect.IsLinkType() && m_pSectionData->IsLinkType())
        || (    !m_pSectionData->GetLinkFileName().isEmpty()
            &&  (m_pSectionData->GetLinkFileName() !=
                    rNdSect.GetLinkFileName()));

    // swap stored section data with live section data
    SwSectionData *const pOld( new SwSectionData(rNdSect) );
    rNdSect.SetSectionData(*m_pSectionData);
    m_pSectionData.reset(pOld);

    if( bUpdate )
        rNdSect.CreateLink( LinkCreateType::Update );
    else if( SectionType::Content == rNdSect.GetType() && rNdSect.IsConnected() )
    {
        rNdSect.Disconnect();
        rDoc.getIDocumentLinksAdministration().GetLinkManager().Remove( &rNdSect.GetBaseLink() );
    }
}

void SwUndoUpdateSection::RedoImpl(::sw::UndoRedoContext & rContext)
{
    UndoImpl(rContext);
}


SwUndoUpdateIndex::SwUndoUpdateIndex(SwTOXBaseSection & rTOX)
    : SwUndo(SwUndoId::INSSECTION, rTOX.GetFormat()->GetDoc())
    , m_pSaveSectionOriginal(new SwUndoSaveSection)
    , m_pSaveSectionUpdated(new SwUndoSaveSection)
    , m_nStartIndex(rTOX.GetFormat()->GetSectionNode()->GetIndex() + 1)
{
    SwDoc & rDoc(*rTOX.GetFormat()->GetDoc());
    assert(rDoc.GetNodes()[m_nStartIndex-1]->IsSectionNode());
    assert(rDoc.GetNodes()[rDoc.GetNodes()[m_nStartIndex]->EndOfSectionIndex()-1]->IsTextNode()); // -1 for extra empty node
    // note: title is optional
    assert(rDoc.GetNodes()[m_nStartIndex]->IsTextNode()
        || rDoc.GetNodes()[m_nStartIndex]->IsSectionNode());
    SwNodeIndex const first(rDoc.GetNodes(), m_nStartIndex);
    if (first.GetNode().IsSectionNode())
    {
        SwSectionFormat & rSectionFormat(*first.GetNode().GetSectionNode()->GetSection().GetFormat());
        // note: DelSectionFormat will create & append SwUndoDelSection!
        rDoc.DelSectionFormat(& rSectionFormat); // remove inner section nodes
    }
    assert(first.GetNode().IsTextNode()); // invariant: ToX section is *never* empty
    SwNodeIndex const last(rDoc.GetNodes(), rDoc.GetNodes()[m_nStartIndex]->EndOfSectionIndex() - 2); // skip empty node
    assert(last.GetNode().IsTextNode());
    m_pSaveSectionOriginal->SaveSection(SwNodeRange(first, last), false);
}

SwUndoUpdateIndex::~SwUndoUpdateIndex() = default;

void SwUndoUpdateIndex::TitleSectionInserted(SwSectionFormat & rFormat)
{
    SwNodeIndex const tmp(rFormat.GetDoc()->GetNodes(), m_nStartIndex); // title inserted before empty node
    assert(tmp.GetNode().IsSectionNode());
    assert(tmp.GetNode().GetSectionNode()->GetSection().GetFormat() == &rFormat);
    m_pTitleSectionUpdated.reset(static_cast<SwUndoDelSection*>(MakeUndoDelSection(rFormat).release()));
}

void SwUndoUpdateIndex::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc(rContext.GetDoc());
    if (m_pTitleSectionUpdated)
    {
        m_pTitleSectionUpdated->RedoImpl(rContext);
    }
    SwNodeIndex const first(rDoc.GetNodes(), m_nStartIndex);
    assert(first.GetNode().IsTextNode()); // invariant: ToX section is *never* empty
    SwNodeIndex const last(rDoc.GetNodes(), rDoc.GetNodes()[m_nStartIndex]->EndOfSectionIndex() - 1);
    assert(last.GetNode().IsTextNode());
    // dummy node so that SaveSection doesn't remove ToX section...
    SwTextNode *const pDeletionPrevention = rDoc.GetNodes().MakeTextNode(
        SwNodeIndex(*rDoc.GetNodes()[m_nStartIndex]->EndOfSectionNode()),
        rDoc.getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_TEXT));
    m_pSaveSectionUpdated->SaveSection(SwNodeRange(first, last), false);
    m_pSaveSectionOriginal->RestoreSection(&rDoc, first, true);
    // delete before restoring nested undo, so its node indexes match
    SwNodeIndex const del(*pDeletionPrevention);
    SwDoc::CorrAbs(del, del, SwPosition(SwNodeIndex(*rDoc.GetNodes()[m_nStartIndex]->EndOfSectionNode())), true);
    rDoc.GetNodes().Delete(del);
    // original title section will be restored by next Undo, see ctor!
}

void SwUndoUpdateIndex::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc(rContext.GetDoc());
    // original title section was deleted by previous Undo, see ctor!
    SwNodeIndex const first(rDoc.GetNodes(), m_nStartIndex);
    assert(first.GetNode().IsTextNode()); // invariant: ToX section is *never* empty
    SwNodeIndex const last(rDoc.GetNodes(), rDoc.GetNodes()[m_nStartIndex]->EndOfSectionIndex() - 1);
    assert(last.GetNode().IsTextNode());
    // dummy node so that SaveSection doesn't remove ToX section...
    SwTextNode *const pDeletionPrevention = rDoc.GetNodes().MakeTextNode(
        SwNodeIndex(*rDoc.GetNodes()[m_nStartIndex]->EndOfSectionNode()),
        rDoc.getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_TEXT));
    m_pSaveSectionOriginal->SaveSection(SwNodeRange(first, last), false);
    m_pSaveSectionUpdated->RestoreSection(&rDoc, first, true);
    // delete before restoring nested undo, so its node indexes match
    SwNodeIndex const del(*pDeletionPrevention);
    SwDoc::CorrAbs(del, del, SwPosition(SwNodeIndex(*rDoc.GetNodes()[m_nStartIndex]->EndOfSectionNode())), true);
    rDoc.GetNodes().Delete(del);
    if (m_pTitleSectionUpdated)
    {
        m_pTitleSectionUpdated->UndoImpl(rContext);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
