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
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <drawdoc.hxx>
#include <hintids.hxx>
#include <fmtflcnt.hxx>
#include <txtftn.hxx>
#include <fmtornt.hxx>
#include <fmtanchr.hxx>
#include <fmtfsize.hxx>
#include <frmfmt.hxx>
#include <fmtcntnt.hxx>
#include <ftnidx.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentDrawModelAccess.hxx>
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
#include <dcontact.hxx>
#include <ftninfo.hxx>
#include <redline.hxx>
#include <section.hxx>
#include <charfmt.hxx>
#include <calbck.hxx>

SwUndoFormatAttrHelper::SwUndoFormatAttrHelper( SwFormat& rFormat, bool bSvDrwPt )
    : SwClient( &rFormat )
    , m_bSaveDrawPt( bSvDrwPt )
{
}

void SwUndoFormatAttrHelper::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    if( pOld ) {
        if ( pOld->Which() == RES_OBJECTDYING ) {
            CheckRegistration( pOld, pNew );
        } else if ( pNew ) {
            if( POOLATTR_END >= pOld->Which() ) {
                if ( GetUndo() ) {
                    m_pUndo->PutAttr( *pOld );
                } else {
                    m_pUndo.reset( new SwUndoFormatAttr( *pOld,
                                                      *static_cast<SwFormat*>(GetRegisteredInNonConst()), m_bSaveDrawPt ) );
                }
            } else if ( RES_ATTRSET_CHG == pOld->Which() ) {
                if ( GetUndo() ) {
                    SfxItemIter aIter(
                        *(static_cast<const SwAttrSetChg*>(pOld))->GetChgSet() );
                    const SfxPoolItem* pItem = aIter.GetCurItem();
                    while ( pItem ) {
                        m_pUndo->PutAttr( *pItem );
                        if( aIter.IsAtEnd() )
                            break;
                        pItem = aIter.NextItem();
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
    : SwUndo( UNDO_INSFMTATTR )
    , m_pFormat( &rChgFormat )
    // #i56253#
    , m_pOldSet( new SfxItemSet( rOldSet ) )
    , m_nNodeIndex( 0 )
    , m_nFormatWhich( rChgFormat.Which() )
    , m_bSaveDrawPt( bSaveDrawPt )
{
    Init();
}

SwUndoFormatAttr::SwUndoFormatAttr( const SfxPoolItem& rItem, SwFormat& rChgFormat,
                              bool bSaveDrawPt )
    : SwUndo( UNDO_INSFMTATTR )
    , m_pFormat( &rChgFormat )
    , m_pOldSet( m_pFormat->GetAttrSet().Clone( false ) )
    , m_nNodeIndex( 0 )
    , m_nFormatWhich( rChgFormat.Which() )
    , m_bSaveDrawPt( bSaveDrawPt )
{
    m_pOldSet->Put( rItem );
    Init();
}

void SwUndoFormatAttr::Init()
{
    // treat change of anchor specially
    if ( SfxItemState::SET == m_pOldSet->GetItemState( RES_ANCHOR, false )) {
        SaveFlyAnchor( m_bSaveDrawPt );
    } else if ( RES_FRMFMT == m_nFormatWhich ) {
        SwDoc* pDoc = m_pFormat->GetDoc();
        if ( pDoc->GetTableFrameFormats()->Contains( m_pFormat )) {
            // Table Format: save table position, table formats are volatile!
            SwTable * pTable = SwIterator<SwTable,SwFormat>( *m_pFormat ).First();
            if ( pTable ) {
                m_nNodeIndex = pTable->GetTabSortBoxes()[ 0 ]->GetSttNd()
                               ->FindTableNode()->GetIndex();
            }
        } else if ( pDoc->GetSections().Contains( m_pFormat )) {
            m_nNodeIndex = m_pFormat->GetContent().GetContentIdx()->GetIndex();
        } else if ( dynamic_cast< SwTableBoxFormat* >( m_pFormat ) !=  nullptr ) {
            SwTableBox * pTableBox = SwIterator<SwTableBox,SwFormat>( *m_pFormat ).First();
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

    if ( !m_pOldSet.get() || !m_pFormat || !IsFormatInDoc( &rContext.GetDoc() ))
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
            SaveFlyAnchor();
        } else {
            // Anchor attribute not restored due to invalid anchor position.
            // Thus, delete anchor attribute.
            m_pOldSet->ClearItem( RES_ANCHOR );
        }
    }

    if ( !bAnchorAttrRestored ) {
        SwUndoFormatAttrHelper aTmp( *m_pFormat, m_bSaveDrawPt );
        m_pFormat->SetFormatAttr( *m_pOldSet );
        if ( aTmp.GetUndo() ) {
            // transfer ownership of helper object's old set
            m_pOldSet = std::move(aTmp.GetUndo()->m_pOldSet);
        } else {
            m_pOldSet->ClearItem();
        }

        if ( RES_FLYFRMFMT == m_nFormatWhich || RES_DRAWFRMFMT == m_nFormatWhich ) {
            rContext.SetSelections(static_cast<SwFrameFormat*>(m_pFormat), nullptr);
        }
    }
}

bool SwUndoFormatAttr::IsFormatInDoc( SwDoc* pDoc )
{
    // search for the Format in the Document; if it does not exist any more,
    // the attribute is not restored!
    bool bFound = false;
    switch ( m_nFormatWhich )
    {
        case RES_TXTFMTCOLL:
        case RES_CONDTXTFMTCOLL:
            bFound = pDoc->GetTextFormatColls()->Contains( m_pFormat );
            break;

        case RES_GRFFMTCOLL:
            bFound = pDoc->GetGrfFormatColls()->Contains(
                    static_cast<const SwGrfFormatColl*>(m_pFormat) );
            break;

        case RES_CHRFMT:
            bFound = pDoc->GetCharFormats()->Contains( m_pFormat );
            break;

        case RES_FRMFMT:
            if ( m_nNodeIndex && (m_nNodeIndex < pDoc->GetNodes().Count()) )
            {
                SwNode* pNd = pDoc->GetNodes()[ m_nNodeIndex ];
                if ( pNd->IsTableNode() )
                {
                    m_pFormat =
                        static_cast<SwTableNode*>(pNd)->GetTable().GetFrameFormat();
                    bFound = true;
                    break;
                }
                else if ( pNd->IsSectionNode() )
                {
                    m_pFormat =
                        static_cast<SwSectionNode*>(pNd)->GetSection().GetFormat();
                    bFound = true;
                    break;
                }
                else if ( pNd->IsStartNode() && (SwTableBoxStartNode ==
                    static_cast< SwStartNode* >(pNd)->GetStartNodeType()) )
                {
                    SwTableNode* pTableNode = pNd->FindTableNode();
                    if ( pTableNode )
                    {
                        SwTableBox* pBox =
                            pTableNode->GetTable().GetTableBox( m_nNodeIndex );
                        if ( pBox )
                        {
                            m_pFormat = pBox->GetFrameFormat();
                            bFound = true;
                            break;
                        }
                    }
                }
            }
            SAL_FALLTHROUGH;
        case RES_DRAWFRMFMT:
        case RES_FLYFRMFMT:
            bFound = pDoc->GetSpzFrameFormats()->Contains( m_pFormat );
            if ( !bFound )
            {
                bFound = pDoc->GetFrameFormats()->Contains( m_pFormat );
            }
            break;
    }

    if ( !bFound )
    {
        // Format does not exist; reset
        m_pFormat = nullptr;
    }

    return nullptr != m_pFormat;
}

// Check if it is still in Doc
SwFormat* SwUndoFormatAttr::GetFormat( SwDoc& rDoc )
{
    return m_pFormat && IsFormatInDoc( &rDoc ) ? m_pFormat : nullptr;
}

void SwUndoFormatAttr::RedoImpl(::sw::UndoRedoContext & rContext)
{
    // #i35443# - Because the undo stores the attributes for
    // redo, the same code as for <Undo(..)> can be applied for <Redo(..)>
    UndoImpl(rContext);
}

void SwUndoFormatAttr::RepeatImpl(::sw::RepeatContext & rContext)
{
    if ( !m_pOldSet.get() )
        return;

    SwDoc & rDoc(rContext.GetDoc());

    switch ( m_nFormatWhich ) {
    case RES_GRFFMTCOLL: {
        SwNoTextNode *const pNd =
            rContext.GetRepeatPaM().GetNode().GetNoTextNode();
        if( pNd ) {
            rDoc.SetAttr( m_pFormat->GetAttrSet(), *pNd->GetFormatColl() );
        }
    }
    break;

    case RES_TXTFMTCOLL:
    case RES_CONDTXTFMTCOLL:
    {
        SwTextNode *const pNd =
            rContext.GetRepeatPaM().GetNode().GetTextNode();
        if( pNd ) {
            rDoc.SetAttr( m_pFormat->GetAttrSet(), *pNd->GetFormatColl() );
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
                m_pFormat->GetAttrSet().GetItemState( RES_CNTNT )) {
                SfxItemSet aTmpSet( m_pFormat->GetAttrSet() );
                aTmpSet.ClearItem( RES_CNTNT );
                if( aTmpSet.Count() ) {
                    rDoc.SetAttr( aTmpSet, *pFly );
                }
            } else {
                rDoc.SetAttr( m_pFormat->GetAttrSet(), *pFly );
            }
        }
        break;
    }
    }
}

SwRewriter SwUndoFormatAttr::GetRewriter() const
{
    SwRewriter aRewriter;

    if (m_pFormat) {
        aRewriter.AddRule(UndoArg1, m_pFormat->GetName());
    }

    return aRewriter;
}

void SwUndoFormatAttr::PutAttr( const SfxPoolItem& rItem )
{
    m_pOldSet->Put( rItem );
    if ( RES_ANCHOR == rItem.Which() ) {
        SaveFlyAnchor( m_bSaveDrawPt );
    }
}

void SwUndoFormatAttr::SaveFlyAnchor( bool bSvDrwPt )
{
    // Format is valid, otherwise you would not reach this point here
    if( bSvDrwPt ) {
        if ( RES_DRAWFRMFMT == m_pFormat->Which() ) {
            Point aPt( static_cast<SwFrameFormat*>(m_pFormat)->FindSdrObject()
                       ->GetRelativePos() );
            // store old value as attribute, to keep SwUndoFormatAttr small
            m_pOldSet->Put( SwFormatFrameSize( ATT_VAR_SIZE, aPt.X(), aPt.Y() ) );
        }
    }

    const SwFormatAnchor& rAnchor =
        static_cast<const SwFormatAnchor&>( m_pOldSet->Get( RES_ANCHOR, false ) );
    if( !rAnchor.GetContentAnchor() )
        return;

    sal_Int32 nContent = 0;
    switch( rAnchor.GetAnchorId() ) {
    case FLY_AS_CHAR:
    case FLY_AT_CHAR:
        nContent = rAnchor.GetContentAnchor()->nContent.GetIndex();
        SAL_FALLTHROUGH;
    case FLY_AT_PARA:
    case FLY_AT_FLY:
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
    SwFrameFormat* pFrameFormat = static_cast<SwFrameFormat*>(m_pFormat);
    const SwFormatAnchor& rAnchor =
        static_cast<const SwFormatAnchor&>( m_pOldSet->Get( RES_ANCHOR, false ) );

    SwFormatAnchor aNewAnchor( rAnchor.GetAnchorId() );
    if (FLY_AT_PAGE != rAnchor.GetAnchorId()) {
        SwNode* pNd = pDoc->GetNodes()[ m_nNodeIndex  ];

        if (  (FLY_AT_FLY == rAnchor.GetAnchorId())
              ? ( !pNd->IsStartNode() || (SwFlyStartNode !=
                                          static_cast<SwStartNode*>(pNd)->GetStartNodeType()) )
              : !pNd->IsTextNode() ) {
            // #i35443# - invalid position.
            // Thus, anchor attribute not restored
            return false;
        }

        SwPosition aPos( *pNd );
        if ((FLY_AS_CHAR == rAnchor.GetAnchorId()) ||
            (FLY_AT_CHAR == rAnchor.GetAnchorId())) {
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
            const SwFormatFrameSize& rOldSize = static_cast<const SwFormatFrameSize&>(
                                               m_pOldSet->Get( RES_FRM_SIZE ) );
            aDrawSavePt.X() = rOldSize.GetWidth();
            aDrawSavePt.Y() = rOldSize.GetHeight();
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
    if (FLY_AS_CHAR == rOldAnch.GetAnchorId()) {
        // With InContents it's tricky: the text attribute needs to be deleted.
        // Unfortunately, this not only destroys the Frames but also the format.
        // To prevent that, first detach the connection between attribute and
        // format.
        const SwPosition *pPos = rOldAnch.GetContentAnchor();
        SwTextNode *pTextNode = static_cast<SwTextNode*>(&pPos->nNode.GetNode());
        OSL_ENSURE( pTextNode->HasHints(), "Missing FlyInCnt-Hint." );
        const sal_Int32 nIdx = pPos->nContent.GetIndex();
        SwTextAttr * const pHint =
            pTextNode->GetTextAttrForCharAt( nIdx, RES_TXTATR_FLYCNT );
        OSL_ENSURE( pHint && pHint->Which() == RES_TXTATR_FLYCNT,
                    "Missing FlyInCnt-Hint." );
        OSL_ENSURE( pHint && pHint->GetFlyCnt().GetFrameFormat() == pFrameFormat,
                    "Wrong TextFlyCnt-Hint." );
        const_cast<SwFormatFlyCnt&>(pHint->GetFlyCnt()).SetFlyFormat();

        // Connection is now detached, therefore the attribute can be deleted
        pTextNode->DeleteAttributes( RES_TXTATR_FLYCNT, nIdx, nIdx );
    }

    {
        m_pOldSet->Put( aNewAnchor );
        SwUndoFormatAttrHelper aTmp( *m_pFormat, m_bSaveDrawPt );
        m_pFormat->SetFormatAttr( *m_pOldSet );
        if ( aTmp.GetUndo() ) {
            m_nNodeIndex = aTmp.GetUndo()->m_nNodeIndex;
            // transfer ownership of helper object's old set
            m_pOldSet = std::move(aTmp.GetUndo()->m_pOldSet);
        } else {
            m_pOldSet->ClearItem();
        }
    }

    SwDrawContact *pCont = nullptr;
    if ( RES_DRAWFRMFMT == pFrameFormat->Which() ) {
        pCont = static_cast<SwDrawContact*>(pFrameFormat->FindContactObj());
        // The Draw model also prepared an Undo object for its right positioning
        // which unfortunately is relative. Therefore block here a position
        // change of the Contact object by setting the anchor.
        SdrObject* pObj = pCont->GetMaster();

        if( pCont->GetAnchorFrame() && !pObj->IsInserted() ) {
            OSL_ENSURE( pDoc->getIDocumentDrawModelAccess().GetDrawModel(),
                        "RestoreFlyAnchor without DrawModel" );
            pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage( 0 )->InsertObject( pObj );
        }
        pObj->SetRelativePos( aDrawSavePt );

        // cache the old value again
        m_pOldSet->Put(
            SwFormatFrameSize( ATT_VAR_SIZE, aDrawOldPt.X(), aDrawOldPt.Y() ) );
    }

    if (FLY_AS_CHAR == aNewAnchor.GetAnchorId()) {
        const SwPosition* pPos = aNewAnchor.GetContentAnchor();
        SwTextNode* pTextNd = pPos->nNode.GetNode().GetTextNode();
        OSL_ENSURE( pTextNd, "no Text Node at position." );
        SwFormatFlyCnt aFormat( pFrameFormat );
        pTextNd->InsertItem( aFormat, pPos->nContent.GetIndex(), 0 );
    }

    if (RES_DRAWFRMFMT != pFrameFormat->Which())
        pFrameFormat->MakeFrames();
    else
    {
        SdrObject* pSdrObj = pFrameFormat->FindSdrObject();
        pCont->GetAnchoredObj(pSdrObj)->MakeObjPos();
    }

    rContext.SetSelections(pFrameFormat, nullptr);

    // #i35443# - anchor attribute restored.
    return true;
}

SwUndoFormatResetAttr::SwUndoFormatResetAttr( SwFormat& rChangedFormat,
                                        const sal_uInt16 nWhichId )
    : SwUndo( UNDO_RESETATTR )
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
    if ( m_pOldItem.get() ) {
        m_pChangedFormat->SetFormatAttr( *m_pOldItem );
    }
}

void SwUndoFormatResetAttr::RedoImpl(::sw::UndoRedoContext &)
{
    if ( m_pOldItem.get() ) {
        m_pChangedFormat->ResetFormatAttr( m_nWhichId );
    }
}

SwUndoResetAttr::SwUndoResetAttr( const SwPaM& rRange, sal_uInt16 nFormatId )
    : SwUndo( UNDO_RESETATTR ), SwUndRng( rRange )
    , m_pHistory( new SwHistory )
    , m_nFormatId( nFormatId )
{
}

SwUndoResetAttr::SwUndoResetAttr( const SwPosition& rPos, sal_uInt16 nFormatId )
    : SwUndo( UNDO_RESETATTR )
    , m_pHistory( new SwHistory )
    , m_nFormatId( nFormatId )
{
    nSttNode = nEndNode = rPos.nNode.GetIndex();
    nSttContent = nEndContent = rPos.nContent.GetIndex();
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
        (nSttNode == nEndNode) && (nSttContent == nEndContent)) {
        SwTextNode* pTNd = rDoc.GetNodes()[ nSttNode ]->GetTextNode();
        if( pTNd ) {
            SwIndex aIdx( pTNd, nSttContent );
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
        SwNodeIndex aIdx( rDoc.GetNodes(), nSttNode );
        SwPosition aPos( aIdx, SwIndex( aIdx.GetNode().GetContentNode(),
                                        nSttContent ));

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
    m_Ids.clear();
    m_Ids.insert( rAttrs.begin(), rAttrs.end() );
}

SwUndoAttr::SwUndoAttr( const SwPaM& rRange, const SfxPoolItem& rAttr,
                        const SetAttrMode nFlags )
    : SwUndo( UNDO_INSATTR ), SwUndRng( rRange )
    , m_AttrSet( rRange.GetDoc()->GetAttrPool(), rAttr.Which(), rAttr.Which() )
    , m_pHistory( new SwHistory )
    , m_nNodeIndex( ULONG_MAX )
    , m_nInsertFlags( nFlags )
{
    m_AttrSet.Put( rAttr );
}

SwUndoAttr::SwUndoAttr( const SwPaM& rRange, const SfxItemSet& rSet,
                        const SetAttrMode nFlags )
    : SwUndo( UNDO_INSATTR ), SwUndRng( rRange )
    , m_AttrSet( rSet )
    , m_pHistory( new SwHistory )
    , m_nNodeIndex( ULONG_MAX )
    , m_nInsertFlags( nFlags )
{
}

SwUndoAttr::~SwUndoAttr()
{
}

void SwUndoAttr::SaveRedlineData( const SwPaM& rPam, bool bIsContent )
{
    SwDoc* pDoc = rPam.GetDoc();
    if ( pDoc->getIDocumentRedlineAccess().IsRedlineOn() ) {
        m_pRedlineData.reset( new SwRedlineData( bIsContent
                              ? nsRedlineType_t::REDLINE_INSERT
                              : nsRedlineType_t::REDLINE_FORMAT,
                              pDoc->getIDocumentRedlineAccess().GetRedlineAuthor() ) );
    }

    m_pRedlineSaveData.reset( new SwRedlineSaveDatas );
    if ( !FillSaveDataForFormat( rPam, *m_pRedlineSaveData )) {
        m_pRedlineSaveData.reset(nullptr);
    }

    SetRedlineMode( pDoc->getIDocumentRedlineAccess().GetRedlineMode() );
    if ( bIsContent ) {
        m_nNodeIndex = rPam.GetPoint()->nNode.GetIndex();
    }
}

void SwUndoAttr::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc *const pDoc = & rContext.GetDoc();

    RemoveIdx( *pDoc );

    if( IDocumentRedlineAccess::IsRedlineOn( GetRedlineMode() ) ) {
        SwPaM aPam(pDoc->GetNodes().GetEndOfContent());
        if ( ULONG_MAX != m_nNodeIndex ) {
            aPam.DeleteMark();
            aPam.GetPoint()->nNode = m_nNodeIndex;
            aPam.GetPoint()->nContent.Assign( aPam.GetContentNode(), nSttContent );
            aPam.SetMark();
            ++aPam.GetPoint()->nContent;
            pDoc->getIDocumentRedlineAccess().DeleteRedline(aPam, false, USHRT_MAX);
        } else {
            // remove all format redlines, will be recreated if needed
            SetPaM(aPam);
            pDoc->getIDocumentRedlineAccess().DeleteRedline(aPam, false, nsRedlineType_t::REDLINE_FORMAT);
            if ( m_pRedlineSaveData.get() ) {
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

    if ( m_pRedlineData.get() &&
         IDocumentRedlineAccess::IsRedlineOn( GetRedlineMode() ) ) {
        RedlineMode_t eOld = rDoc.getIDocumentRedlineAccess().GetRedlineMode();
        rDoc.getIDocumentRedlineAccess().SetRedlineMode_intern(static_cast<RedlineMode_t>(
                    eOld & ~nsRedlineMode_t::REDLINE_IGNORE));
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

        rDoc.getIDocumentRedlineAccess().SetRedlineMode_intern( eOld );
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

        case HSTRY_RESETATTRSET: {
            SwHistoryResetAttrSet * pHistoryHint
                = static_cast<SwHistoryResetAttrSet*>(pHstHint);
            nContent = pHistoryHint->GetContent();
            if ( COMPLETE_STRING != nContent ) {
                const std::vector<sal_uInt16>& rArr = pHistoryHint->GetArr();
                for ( size_t i = rArr.size(); i; ) {
                    if ( RES_TXTATR_FTN == rArr[ --i ] ) {
                        nNode = pHistoryHint->GetNode();
                        break;
                    }
                }
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

SwUndoDefaultAttr::SwUndoDefaultAttr( const SfxItemSet& rSet )
    : SwUndo( UNDO_SETDEFTATTR )
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
    if ( m_pOldSet.get() ) {
        SwUndoFormatAttrHelper aTmp(
            *rDoc.GetDfltTextFormatColl() );
        rDoc.SetDefault( *m_pOldSet );
        m_pOldSet.reset( nullptr );
        if ( aTmp.GetUndo() ) {
            // transfer ownership of helper object's old set
            m_pOldSet = std::move(aTmp.GetUndo()->m_pOldSet);
        }
    }
    if ( m_pTabStop.get() ) {
        SvxTabStopItem* pOld = static_cast<SvxTabStopItem*>(
                                   rDoc.GetDefault( RES_PARATR_TABSTOP ).Clone() );
        rDoc.SetDefault( *m_pTabStop );
        m_pTabStop.reset( pOld );
    }
}

void SwUndoDefaultAttr::RedoImpl(::sw::UndoRedoContext & rContext)
{
    UndoImpl(rContext);
}

SwUndoMoveLeftMargin::SwUndoMoveLeftMargin(
    const SwPaM& rPam, bool bFlag, bool bMod )
    : SwUndo( bFlag ? UNDO_INC_LEFTMARGIN : UNDO_DEC_LEFTMARGIN )
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
                         GetId() == UNDO_INC_LEFTMARGIN, m_bModulus );
}

void SwUndoMoveLeftMargin::RepeatImpl(::sw::RepeatContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    rDoc.MoveLeftMargin(rContext.GetRepeatPaM(), GetId() == UNDO_INC_LEFTMARGIN,
                        m_bModulus );
}

SwUndoChangeFootNote::SwUndoChangeFootNote(
    const SwPaM& rRange, const OUString& rText,
    sal_uInt16 nNum, bool bIsEndNote )
    : SwUndo( UNDO_CHGFTN ), SwUndRng( rRange )
    , m_pHistory( new SwHistory() )
    , m_Text( rText )
    , m_nNumber( nNum )
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
    rDoc.SetCurFootnote(rPaM, m_Text, m_nNumber, m_bEndNote);
    SetPaM(rPaM);
}

void SwUndoChangeFootNote::RepeatImpl(::sw::RepeatContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    rDoc.SetCurFootnote( rContext.GetRepeatPaM(), m_Text, m_nNumber, m_bEndNote );
}

SwUndoFootNoteInfo::SwUndoFootNoteInfo( const SwFootnoteInfo &rInfo )
    : SwUndo( UNDO_FTNINFO )
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

SwUndoEndNoteInfo::SwUndoEndNoteInfo( const SwEndNoteInfo &rInfo )
    : SwUndo( UNDO_FTNINFO )
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
    : SwUndo( UNDO_DONTEXPAND )
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
