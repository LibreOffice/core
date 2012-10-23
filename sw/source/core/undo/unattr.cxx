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

#include <UndoAttribute.hxx>
#include <svl/itemiter.hxx>
#include <editeng/tstpitem.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
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
#include <switerator.hxx>

SwUndoFmtAttrHelper::SwUndoFmtAttrHelper( SwFmt& rFmt, bool bSvDrwPt )
    : SwClient( &rFmt )
    , m_pUndo( 0 )
    , m_bSaveDrawPt( bSvDrwPt )
{
}

void SwUndoFmtAttrHelper::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    if( pOld )
    {
        if ( pOld->Which() == RES_OBJECTDYING )
        {
            CheckRegistration( pOld, pNew );
        }
        else if ( pNew )
        {
            if( POOLATTR_END >= pOld->Which() )
            {
                if ( GetUndo() )
                {
                    m_pUndo->PutAttr( *pOld );
                }
                else
                {
                    m_pUndo.reset( new SwUndoFmtAttr( *pOld,
                            *static_cast<SwFmt*>(GetRegisteredInNonConst()), m_bSaveDrawPt ) );
                }
            }
            else if ( RES_ATTRSET_CHG == pOld->Which() )
            {
                if ( GetUndo() )
                {
                    SfxItemIter aIter(
                            *(static_cast<const SwAttrSetChg*>(pOld))->GetChgSet() );
                    const SfxPoolItem* pItem = aIter.GetCurItem();
                    while ( pItem )
                    {
                        m_pUndo->PutAttr( *pItem );
                        if( aIter.IsAtEnd() )
                            break;
                        pItem = aIter.NextItem();
                    }
                }
                else
                {
                    m_pUndo.reset( new SwUndoFmtAttr(
                            *static_cast<const SwAttrSetChg*>(pOld)->GetChgSet(),
                            *static_cast<SwFmt*>(GetRegisteredInNonConst()), m_bSaveDrawPt ) );
                }
            }
        }
    }
}

SwUndoFmtAttr::SwUndoFmtAttr( const SfxItemSet& rOldSet,
                              SwFmt& rChgFmt,
                              bool bSaveDrawPt )
    : SwUndo( UNDO_INSFMTATTR )
    , m_pFmt( &rChgFmt )
      // #i56253#
    , m_pOldSet( new SfxItemSet( rOldSet ) )
    , m_nNodeIndex( 0 )
    , m_nFmtWhich( rChgFmt.Which() )
    , m_bSaveDrawPt( bSaveDrawPt )
{
    Init();
}

SwUndoFmtAttr::SwUndoFmtAttr( const SfxPoolItem& rItem, SwFmt& rChgFmt,
                              bool bSaveDrawPt )
    : SwUndo( UNDO_INSFMTATTR )
    , m_pFmt( &rChgFmt )
    , m_pOldSet( m_pFmt->GetAttrSet().Clone( sal_False ) )
    , m_nNodeIndex( 0 )
    , m_nFmtWhich( rChgFmt.Which() )
    , m_bSaveDrawPt( bSaveDrawPt )
{
    m_pOldSet->Put( rItem );
    Init();
}

void SwUndoFmtAttr::Init()
{
    // treat change of anchor specially
    if ( SFX_ITEM_SET == m_pOldSet->GetItemState( RES_ANCHOR, sal_False ))
    {
        SaveFlyAnchor( m_bSaveDrawPt );
    }
    else if ( RES_FRMFMT == m_nFmtWhich )
    {
        SwDoc* pDoc = m_pFmt->GetDoc();
        if ( pDoc->GetTblFrmFmts()->Contains(static_cast<const SwFrmFmt*>(m_pFmt)))
        {
            // Table Format: save table position, table formats are volatile!
            SwTable * pTbl = SwIterator<SwTable,SwFmt>::FirstElement( *m_pFmt );
            if ( pTbl )
            {
                m_nNodeIndex = pTbl->GetTabSortBoxes()[ 0 ]->GetSttNd()
                    ->FindTableNode()->GetIndex();
            }
        }
        else if ( pDoc->GetSections().Contains(static_cast<const SwSectionFmt*>(m_pFmt)))
        {
            m_nNodeIndex = m_pFmt->GetCntnt().GetCntntIdx()->GetIndex();
        }
        else if ( 0 != dynamic_cast< SwTableBoxFmt* >( m_pFmt ) )
        {
            SwTableBox * pTblBox = SwIterator<SwTableBox,SwFmt>::FirstElement( *m_pFmt );
            if ( pTblBox )
            {
                m_nNodeIndex = pTblBox->GetSttIdx();
            }
        }
    }
}

SwUndoFmtAttr::~SwUndoFmtAttr()
{
}

void SwUndoFmtAttr::UndoImpl(::sw::UndoRedoContext & rContext)
{
    // OD 2004-10-26 #i35443#
    // Important note: <Undo(..)> also called by <ReDo(..)>

    if ( !m_pOldSet.get() || !m_pFmt || !IsFmtInDoc( &rContext.GetDoc() ))
        return;

    // #i35443# - If anchor attribute has been successfull
    // restored, all other attributes are also restored.
    // Thus, keep track of its restoration
    bool bAnchorAttrRestored( false );
    if ( SFX_ITEM_SET == m_pOldSet->GetItemState( RES_ANCHOR, sal_False ))
    {
        bAnchorAttrRestored = RestoreFlyAnchor(rContext);
        if ( bAnchorAttrRestored )
        {
            // Anchor attribute successfull restored.
            // Thus, keep anchor position for redo
            SaveFlyAnchor();
        }
        else
        {
            // Anchor attribute not restored due to invalid anchor position.
            // Thus, delete anchor attribute.
            m_pOldSet->ClearItem( RES_ANCHOR );
        }
    }

    if ( !bAnchorAttrRestored )
    {
        SwUndoFmtAttrHelper aTmp( *m_pFmt, m_bSaveDrawPt );
        m_pFmt->SetFmtAttr( *m_pOldSet );
        if ( aTmp.GetUndo() )
        {
            // transfer ownership of helper object's old set
            m_pOldSet = aTmp.GetUndo()->m_pOldSet;
        }
        else
        {
            m_pOldSet->ClearItem();
        }

        if ( RES_FLYFRMFMT == m_nFmtWhich || RES_DRAWFRMFMT == m_nFmtWhich )
        {
            rContext.SetSelections(static_cast<SwFrmFmt*>(m_pFmt), 0);
        }
    }
}

bool SwUndoFmtAttr::IsFmtInDoc( SwDoc* pDoc )
{
    // search for the Format in the Document; if it does not exist any more,
    // the attribute is not restored!
    sal_uInt16 nPos = USHRT_MAX;
    switch ( m_nFmtWhich )
    {
        case RES_TXTFMTCOLL:
            nPos = pDoc->GetTxtFmtColls()->GetPos(
                    static_cast<const SwTxtFmtColl*>(m_pFmt) );
            break;

        case RES_GRFFMTCOLL:
            nPos = pDoc->GetGrfFmtColls()->GetPos(
                    static_cast<const SwGrfFmtColl*>(m_pFmt) );
            break;

        case RES_CHRFMT:
            nPos = pDoc->GetCharFmts()->GetPos(
                    static_cast<SwCharFmt*>(m_pFmt) );
            break;

        case RES_FRMFMT:
            if ( m_nNodeIndex && (m_nNodeIndex < pDoc->GetNodes().Count()) )
            {
                SwNode* pNd = pDoc->GetNodes()[ m_nNodeIndex ];
                if ( pNd->IsTableNode() )
                {
                    m_pFmt =
                        static_cast<SwTableNode*>(pNd)->GetTable().GetFrmFmt();
                    nPos = 0;
                    break;
                }
                else if ( pNd->IsSectionNode() )
                {
                    m_pFmt =
                        static_cast<SwSectionNode*>(pNd)->GetSection().GetFmt();
                    nPos = 0;
                    break;
                }
                else if ( pNd->IsStartNode() && (SwTableBoxStartNode ==
                    static_cast< SwStartNode* >(pNd)->GetStartNodeType()) )
                {
                    SwTableNode* pTblNode = pNd->FindTableNode();
                    if ( pTblNode )
                    {
                        SwTableBox* pBox =
                            pTblNode->GetTable().GetTblBox( m_nNodeIndex );
                        if ( pBox )
                        {
                            m_pFmt = pBox->GetFrmFmt();
                            nPos = 0;
                            break;
                        }
                    }
                }
            }
            // no break!
        case RES_DRAWFRMFMT:
        case RES_FLYFRMFMT:
            nPos = pDoc->GetSpzFrmFmts()->GetPos(
                    static_cast<const SwFrmFmt*>(m_pFmt) );
            if ( USHRT_MAX == nPos )
            {
                nPos = pDoc->GetFrmFmts()->GetPos(
                    static_cast<const SwFrmFmt*>(m_pFmt) );
            }
            break;
    }

    if ( USHRT_MAX == nPos )
    {
        // Format does not exist; reset
        m_pFmt = 0;
    }

    return 0 != m_pFmt;
}

// Check if it is still in Doc
SwFmt* SwUndoFmtAttr::GetFmt( SwDoc& rDoc )
{
    return m_pFmt && IsFmtInDoc( &rDoc ) ? m_pFmt : 0;
}

void SwUndoFmtAttr::RedoImpl(::sw::UndoRedoContext & rContext)
{
    // #i35443# - Because the undo stores the attributes for
    // redo, the same code as for <Undo(..)> can be applied for <Redo(..)>
    UndoImpl(rContext);
}

void SwUndoFmtAttr::RepeatImpl(::sw::RepeatContext & rContext)
{
    if ( !m_pOldSet.get() )
        return;

    SwDoc & rDoc(rContext.GetDoc());

    switch ( m_nFmtWhich )
    {
    case RES_GRFFMTCOLL:
        {
            SwNoTxtNode *const pNd =
                rContext.GetRepeatPaM().GetNode()->GetNoTxtNode();
            if( pNd )
            {
                rDoc.SetAttr( m_pFmt->GetAttrSet(), *pNd->GetFmtColl() );
            }
        }
        break;

    case RES_TXTFMTCOLL:
        {
            SwTxtNode *const pNd =
                rContext.GetRepeatPaM().GetNode()->GetTxtNode();
            if( pNd )
            {
                rDoc.SetAttr( m_pFmt->GetAttrSet(), *pNd->GetFmtColl() );
            }
        }
        break;

    case RES_FLYFRMFMT:
        {
            // Check if the cursor is in a flying frame
            // Steps: search in all FlyFrmFormats for the FlyCntnt attribute
            // and validate if the cursor is in the respective section
            SwFrmFmt *const pFly =
                rContext.GetRepeatPaM().GetNode()->GetFlyFmt();
            if( pFly )
            {
                // Bug 43672: do not set all attributes!
                if (SFX_ITEM_SET ==
                        m_pFmt->GetAttrSet().GetItemState( RES_CNTNT ))
                {
                    SfxItemSet aTmpSet( m_pFmt->GetAttrSet() );
                    aTmpSet.ClearItem( RES_CNTNT );
                    if( aTmpSet.Count() )
                    {
                        rDoc.SetAttr( aTmpSet, *pFly );
                    }
                }
                else
                {
                    rDoc.SetAttr( m_pFmt->GetAttrSet(), *pFly );
                }
            }
            break;
        }
    }
}

SwRewriter SwUndoFmtAttr::GetRewriter() const
{
    SwRewriter aRewriter;

    if (m_pFmt)
    {
        aRewriter.AddRule(UndoArg1, m_pFmt->GetName());
    }

    return aRewriter;
}

void SwUndoFmtAttr::PutAttr( const SfxPoolItem& rItem )
{
    m_pOldSet->Put( rItem );
    if ( RES_ANCHOR == rItem.Which() )
    {
        SaveFlyAnchor( m_bSaveDrawPt );
    }
}

void SwUndoFmtAttr::SaveFlyAnchor( bool bSvDrwPt )
{
    // Format is valid, otherwise you would not reach this point here
    if( bSvDrwPt )
    {
        if ( RES_DRAWFRMFMT == m_pFmt->Which() )
        {
            Point aPt( static_cast<SwFrmFmt*>(m_pFmt)->FindSdrObject()
                            ->GetRelativePos() );
            // store old value as attribute, to keep SwUndoFmtAttr small
            m_pOldSet->Put( SwFmtFrmSize( ATT_VAR_SIZE, aPt.X(), aPt.Y() ) );
        }
    }

    const SwFmtAnchor& rAnchor =
        static_cast<const SwFmtAnchor&>( m_pOldSet->Get( RES_ANCHOR, sal_False ) );
    if( !rAnchor.GetCntntAnchor() )
        return;

    xub_StrLen nCntnt = 0;
    switch( rAnchor.GetAnchorId() )
    {
    case FLY_AS_CHAR:
    case FLY_AT_CHAR:
        nCntnt = rAnchor.GetCntntAnchor()->nContent.GetIndex();
    case FLY_AT_PARA:
    case FLY_AT_FLY:
        m_nNodeIndex = rAnchor.GetCntntAnchor()->nNode.GetIndex();
        break;
    default:
        return;
    }

    SwFmtAnchor aAnchor( rAnchor.GetAnchorId(), nCntnt );
    m_pOldSet->Put( aAnchor );
}

// #i35443# - Add return value, type <bool>.
// Return value indicates, if anchor attribute is restored.
// Note: If anchor attribute is restored, all other existing attributes
//       are also restored.
bool SwUndoFmtAttr::RestoreFlyAnchor(::sw::UndoRedoContext & rContext)
{
    SwDoc *const pDoc = & rContext.GetDoc();
    SwFlyFrmFmt* pFrmFmt = static_cast<SwFlyFrmFmt*>(m_pFmt);
    const SwFmtAnchor& rAnchor =
        static_cast<const SwFmtAnchor&>( m_pOldSet->Get( RES_ANCHOR, sal_False ) );

    SwFmtAnchor aNewAnchor( rAnchor.GetAnchorId() );
    if (FLY_AT_PAGE != rAnchor.GetAnchorId())
    {
        SwNode* pNd = pDoc->GetNodes()[ m_nNodeIndex  ];

        if (  (FLY_AT_FLY == rAnchor.GetAnchorId())
            ? ( !pNd->IsStartNode() || (SwFlyStartNode !=
                    static_cast<SwStartNode*>(pNd)->GetStartNodeType()) )
            : !pNd->IsTxtNode() )
        {
            // #i35443# - invalid position.
            // Thus, anchor attribute not restored
            return false;
        }

        SwPosition aPos( *pNd );
        if ((FLY_AS_CHAR == rAnchor.GetAnchorId()) ||
            (FLY_AT_CHAR == rAnchor.GetAnchorId()))
        {
            aPos.nContent.Assign( (SwTxtNode*)pNd, rAnchor.GetPageNum() );
            if ( aPos.nContent.GetIndex() >
                    static_cast<SwTxtNode*>(pNd)->GetTxt().Len() )
            {
                // #i35443# - invalid position.
                // Thus, anchor attribute not restored
                return false;
            }
        }
        aNewAnchor.SetAnchor( &aPos );
    }
    else
        aNewAnchor.SetPageNum( rAnchor.GetPageNum() );

    Point aDrawSavePt, aDrawOldPt;
    if( pDoc->GetCurrentViewShell() )   //swmod 071108//swmod 071225
    {
        if( RES_DRAWFRMFMT == pFrmFmt->Which() )
        {
            // get the old cached value
            const SwFmtFrmSize& rOldSize = static_cast<const SwFmtFrmSize&>(
                    m_pOldSet->Get( RES_FRM_SIZE ) );
            aDrawSavePt.X() = rOldSize.GetWidth();
            aDrawSavePt.Y() = rOldSize.GetHeight();
            m_pOldSet->ClearItem( RES_FRM_SIZE );

            // write the current value into cache
            aDrawOldPt = pFrmFmt->FindSdrObject()->GetRelativePos();
        }
        else
        {
            pFrmFmt->DelFrms();         // delete Frms
        }
    }

    const SwFmtAnchor &rOldAnch = pFrmFmt->GetAnchor();
    // #i54336#
    // Consider case, that as-character anchored object has moved its anchor position.
    if (FLY_AS_CHAR == rOldAnch.GetAnchorId())
    {
        // With InCntnts it's tricky: the text attribute needs to be deleted.
        // Unfortunately, this not only destroys the Frms but also the format.
        // To prevent that, first detach the connection between attribute and
        // format.
        const SwPosition *pPos = rOldAnch.GetCntntAnchor();
        SwTxtNode *pTxtNode = (SwTxtNode*)&pPos->nNode.GetNode();
        OSL_ENSURE( pTxtNode->HasHints(), "Missing FlyInCnt-Hint." );
        const xub_StrLen nIdx = pPos->nContent.GetIndex();
        SwTxtAttr * const pHnt =
            pTxtNode->GetTxtAttrForCharAt( nIdx, RES_TXTATR_FLYCNT );
        OSL_ENSURE( pHnt && pHnt->Which() == RES_TXTATR_FLYCNT,
                    "Missing FlyInCnt-Hint." );
        OSL_ENSURE( pHnt && pHnt->GetFlyCnt().GetFrmFmt() == pFrmFmt,
                    "Wrong TxtFlyCnt-Hint." );
        const_cast<SwFmtFlyCnt&>(pHnt->GetFlyCnt()).SetFlyFmt();

        // Connection is now detached, therefore the attribute can be deleted
        pTxtNode->DeleteAttributes( RES_TXTATR_FLYCNT, nIdx, nIdx );
    }

    {
        m_pOldSet->Put( aNewAnchor );
        SwUndoFmtAttrHelper aTmp( *m_pFmt, m_bSaveDrawPt );
        m_pFmt->SetFmtAttr( *m_pOldSet );
        if ( aTmp.GetUndo() )
        {
            m_nNodeIndex = aTmp.GetUndo()->m_nNodeIndex;
            // transfer ownership of helper object's old set
            m_pOldSet = aTmp.GetUndo()->m_pOldSet;
        }
        else
        {
            m_pOldSet->ClearItem();
        }
    }

    if ( RES_DRAWFRMFMT == pFrmFmt->Which() )
    {
        SwDrawContact *pCont =
            static_cast<SwDrawContact*>(pFrmFmt->FindContactObj());
        // The Draw model also prepared an Undo object for its right positioning
        // which unfortunately is relative. Therefore block here a position
        // change of the Contact object by setting the anchor.
        SdrObject* pObj = pCont->GetMaster();

        if( pCont->GetAnchorFrm() && !pObj->IsInserted() )
        {
            OSL_ENSURE( pDoc->GetDrawModel(), "RestoreFlyAnchor without DrawModel" );
            pDoc->GetDrawModel()->GetPage( 0 )->InsertObject( pObj );
        }
        pObj->SetRelativePos( aDrawSavePt );

        // cache the old value again
        m_pOldSet->Put(
            SwFmtFrmSize( ATT_VAR_SIZE, aDrawOldPt.X(), aDrawOldPt.Y() ) );
    }

    if (FLY_AS_CHAR == aNewAnchor.GetAnchorId())
    {
        const SwPosition* pPos = aNewAnchor.GetCntntAnchor();
        SwTxtNode* pTxtNd = pPos->nNode.GetNode().GetTxtNode();
        OSL_ENSURE( pTxtNd, "no Text Node at position." );
        SwFmtFlyCnt aFmt( pFrmFmt );
        pTxtNd->InsertItem( aFmt, pPos->nContent.GetIndex(), 0 );
    }


    if( RES_DRAWFRMFMT != pFrmFmt->Which() )
        pFrmFmt->MakeFrms();

    rContext.SetSelections(pFrmFmt, 0);

    // #i35443# - anchor attribute restored.
    return true;
}

SwUndoFmtResetAttr::SwUndoFmtResetAttr( SwFmt& rChangedFormat,
                                        const sal_uInt16 nWhichId )
    : SwUndo( UNDO_RESETATTR )
    , m_pChangedFormat( &rChangedFormat )
    , m_nWhichId( nWhichId )
    , m_pOldItem( 0 )
{
    const SfxPoolItem* pItem = 0;
    if (rChangedFormat.GetItemState( nWhichId, sal_False, &pItem ) == SFX_ITEM_SET)
    {
        m_pOldItem.reset( pItem->Clone() );
    }
}

SwUndoFmtResetAttr::~SwUndoFmtResetAttr()
{
}

void SwUndoFmtResetAttr::UndoImpl(::sw::UndoRedoContext &)
{
    if ( m_pOldItem.get() )
    {
        m_pChangedFormat->SetFmtAttr( *m_pOldItem );
    }
}

void SwUndoFmtResetAttr::RedoImpl(::sw::UndoRedoContext &)
{
    if ( m_pOldItem.get() )
    {
        m_pChangedFormat->ResetFmtAttr( m_nWhichId );
    }
}

SwUndoResetAttr::SwUndoResetAttr( const SwPaM& rRange, sal_uInt16 nFmtId )
    : SwUndo( UNDO_RESETATTR ), SwUndRng( rRange )
    , m_pHistory( new SwHistory )
    , m_nFormatId( nFmtId )
{
}

SwUndoResetAttr::SwUndoResetAttr( const SwPosition& rPos, sal_uInt16 nFmtId )
    : SwUndo( UNDO_RESETATTR )
    , m_pHistory( new SwHistory )
    , m_nFormatId( nFmtId )
{
    nSttNode = nEndNode = rPos.nNode.GetIndex();
    nSttCntnt = nEndCntnt = rPos.nContent.GetIndex();
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
        (nSttNode == nEndNode) && (nSttCntnt == nEndCntnt))
    {
        SwTxtNode* pTNd = rDoc.GetNodes()[ nSttNode ]->GetTxtNode();
        if( pTNd )
        {
            SwIndex aIdx( pTNd, nSttCntnt );
            pTNd->DontExpandFmt( aIdx, sal_False );
        }
    }

    AddUndoRedoPaM(rContext);
}

void SwUndoResetAttr::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    SwPaM & rPam = AddUndoRedoPaM(rContext);

    switch ( m_nFormatId )
    {
    case RES_CHRFMT:
        rDoc.RstTxtAttrs(rPam);
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
            SwPosition aPos( aIdx, SwIndex( aIdx.GetNode().GetCntntNode(),
                                                                nSttCntnt ));

            sal_uInt16 nCnt = rDoc.GetCurTOXMark( aPos, aArr );
            if( nCnt )
            {
                if( 1 < nCnt )
                {
                    // search for the right one
                    SwHistoryHint* pHHint = (GetHistory())[ 0 ];
                    if( pHHint && HSTRY_SETTOXMARKHNT == pHHint->Which() )
                    {
                        while( nCnt )
                        {
                            if ( static_cast<SwHistorySetTOXMark*>(pHHint)
                                    ->IsEqual( *aArr[ --nCnt ] ) )
                            {
                                ++nCnt;
                                break;
                            }
                        }
                    }
                    else
                        nCnt = 0;
                }
                // found one, thus delete it
                if( nCnt-- )
                {
                    rDoc.DeleteTOXMark( aArr[ nCnt ] );
                }
            }
        }
        break;
    }
}

void SwUndoResetAttr::RepeatImpl(::sw::RepeatContext & rContext)
{
    if (m_nFormatId < RES_FMT_BEGIN)
    {
        return;
    }

    switch ( m_nFormatId )
    {
    case RES_CHRFMT:
        rContext.GetDoc().RstTxtAttrs(rContext.GetRepeatPaM());
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
    , m_pRedlineData( 0 )
    , m_pRedlineSaveData( 0 )
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
    , m_pRedlineData( 0 )
    , m_pRedlineSaveData( 0 )
    , m_nNodeIndex( ULONG_MAX )
    , m_nInsertFlags( nFlags )
{
}

SwUndoAttr::~SwUndoAttr()
{
}

void SwUndoAttr::SaveRedlineData( const SwPaM& rPam, sal_Bool bIsCntnt )
{
    SwDoc* pDoc = rPam.GetDoc();
    if ( pDoc->IsRedlineOn() )
    {
        m_pRedlineData.reset( new SwRedlineData( bIsCntnt
                                    ? nsRedlineType_t::REDLINE_INSERT
                                    : nsRedlineType_t::REDLINE_FORMAT,
                                pDoc->GetRedlineAuthor() ) );
    }

    m_pRedlineSaveData.reset( new SwRedlineSaveDatas );
    if ( !FillSaveDataForFmt( rPam, *m_pRedlineSaveData ))
    {
        m_pRedlineSaveData.reset(0);
    }

    SetRedlineMode( pDoc->GetRedlineMode() );
    if ( bIsCntnt )
    {
        m_nNodeIndex = rPam.GetPoint()->nNode.GetIndex();
    }
}

void SwUndoAttr::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc *const pDoc = & rContext.GetDoc();

    RemoveIdx( *pDoc );

    if( IDocumentRedlineAccess::IsRedlineOn( GetRedlineMode() ) )
    {
        SwPaM aPam(pDoc->GetNodes().GetEndOfContent());
        if ( ULONG_MAX != m_nNodeIndex )
        {
            aPam.DeleteMark();
            aPam.GetPoint()->nNode = m_nNodeIndex;
            aPam.GetPoint()->nContent.Assign( aPam.GetCntntNode(), nSttCntnt );
            aPam.SetMark();
            aPam.GetPoint()->nContent++;
            pDoc->DeleteRedline(aPam, false, USHRT_MAX);
        }
        else
        {
            // remove all format redlines, will be recreated if needed
            SetPaM(aPam);
            pDoc->DeleteRedline(aPam, false, nsRedlineType_t::REDLINE_FORMAT);
            if ( m_pRedlineSaveData.get() )
            {
                SetSaveData( *pDoc, *m_pRedlineSaveData );
            }
        }
    }

    const bool bToLast =  (1 == m_AttrSet.Count())
                       && (RES_TXTATR_FIELD <= *m_AttrSet.GetRanges())
                       && (*m_AttrSet.GetRanges() <= RES_TXTATR_FTN);

    // restore old values
    m_pHistory->TmpRollback( pDoc, 0, !bToLast );
    m_pHistory->SetTmpEnd( m_pHistory->Count() );

    // set cursor onto Undo area
    AddUndoRedoPaM(rContext);
}

void SwUndoAttr::RepeatImpl(::sw::RepeatContext & rContext)
{
    // RefMarks are not repeat capable
    if ( SFX_ITEM_SET != m_AttrSet.GetItemState( RES_TXTATR_REFMARK, sal_False ) )
    {
        rContext.GetDoc().InsertItemSet( rContext.GetRepeatPaM(),
                                           m_AttrSet, m_nInsertFlags );
    }
    else if ( 1 < m_AttrSet.Count() )
    {
        SfxItemSet aTmpSet( m_AttrSet );
        aTmpSet.ClearItem( RES_TXTATR_REFMARK );
        rContext.GetDoc().InsertItemSet( rContext.GetRepeatPaM(),
                                           aTmpSet, m_nInsertFlags );
    }
}

void SwUndoAttr::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    SwPaM & rPam = AddUndoRedoPaM(rContext);

    if ( m_pRedlineData.get() &&
         IDocumentRedlineAccess::IsRedlineOn( GetRedlineMode() ) )
    {
        RedlineMode_t eOld = rDoc.GetRedlineMode();
        rDoc.SetRedlineMode_intern(static_cast<RedlineMode_t>(
                    eOld & ~nsRedlineMode_t::REDLINE_IGNORE));
        rDoc.InsertItemSet( rPam, m_AttrSet, m_nInsertFlags );

        if ( ULONG_MAX != m_nNodeIndex )
        {
            rPam.SetMark();
            if ( rPam.Move( fnMoveBackward ) )
            {
                rDoc.AppendRedline( new SwRedline( *m_pRedlineData, rPam ),
                        true);
            }
            rPam.DeleteMark();
        }
        else
        {
            rDoc.AppendRedline( new SwRedline( *m_pRedlineData, rPam ), true);
        }

        rDoc.SetRedlineMode_intern( eOld );
    }
    else
    {
        rDoc.InsertItemSet( rPam, m_AttrSet, m_nInsertFlags );
    }
}

void SwUndoAttr::RemoveIdx( SwDoc& rDoc )
{
    if ( SFX_ITEM_SET != m_AttrSet.GetItemState( RES_TXTATR_FTN, sal_False ))
        return ;

    SwHistoryHint* pHstHnt;
    SwNodes& rNds = rDoc.GetNodes();
    for ( sal_uInt16 n = 0; n < m_pHistory->Count(); ++n )
    {
        xub_StrLen nCntnt = 0;
        sal_uLong nNode = 0;
        pHstHnt = (*m_pHistory)[ n ];
        switch ( pHstHnt->Which() )
        {
            case HSTRY_RESETTXTHNT:
                {
                    SwHistoryResetTxt * pHistoryHint
                        = static_cast<SwHistoryResetTxt*>(pHstHnt);
                    if ( RES_TXTATR_FTN == pHistoryHint->GetWhich() )
                    {
                        nNode = pHistoryHint->GetNode();
                        nCntnt = pHistoryHint->GetCntnt();
                    }
                }
                break;

            case HSTRY_RESETATTRSET:
                {
                    SwHistoryResetAttrSet * pHistoryHint
                        = static_cast<SwHistoryResetAttrSet*>(pHstHnt);
                    nCntnt = pHistoryHint->GetCntnt();
                    if ( STRING_MAXLEN != nCntnt )
                    {
                        const std::vector<sal_uInt16>& rArr = pHistoryHint->GetArr();
                        for ( sal_uInt16 i = rArr.size(); i; )
                        {
                            if ( RES_TXTATR_FTN == rArr[ --i ] )
                            {
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

        if( nNode )
        {
            SwTxtNode* pTxtNd = rNds[ nNode ]->GetTxtNode();
            if( pTxtNd )
            {
                SwTxtAttr *const pTxtHt =
                    pTxtNd->GetTxtAttrForCharAt(nCntnt, RES_TXTATR_FTN);
                if( pTxtHt )
                {
                    // ok, so get values
                    SwTxtFtn* pFtn = static_cast<SwTxtFtn*>(pTxtHt);
                    RemoveIdxFromSection( rDoc, pFtn->GetStartNode()->GetIndex() );
                    return ;
                }
            }
        }
    }
}

SwUndoDefaultAttr::SwUndoDefaultAttr( const SfxItemSet& rSet )
    : SwUndo( UNDO_SETDEFTATTR )
    , m_pOldSet( 0 )
    , m_pTabStop( 0 )
{
    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET == rSet.GetItemState( RES_PARATR_TABSTOP, sal_False, &pItem ) )
    {
        // store separately, because it may change!
        m_pTabStop.reset( static_cast<SvxTabStopItem*>(pItem->Clone()) );
        if ( 1 != rSet.Count() ) // are there more attributes?
        {
            m_pOldSet.reset( new SfxItemSet( rSet ) );
        }
    }
    else
    {
        m_pOldSet.reset( new SfxItemSet( rSet ) );
    }
}

SwUndoDefaultAttr::~SwUndoDefaultAttr()
{
}

void SwUndoDefaultAttr::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    if ( m_pOldSet.get() )
    {
        SwUndoFmtAttrHelper aTmp(
                *const_cast<SwTxtFmtColl*>(rDoc.GetDfltTxtFmtColl()) );
        rDoc.SetDefault( *m_pOldSet );
        m_pOldSet.reset( 0 );
        if ( aTmp.GetUndo() )
        {
            // transfer ownership of helper object's old set
            m_pOldSet = aTmp.GetUndo()->m_pOldSet;
        }
    }
    if ( m_pTabStop.get() )
    {
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
            const SwPaM& rPam, sal_Bool bFlag, sal_Bool bMod )
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
            const SwPaM& rRange, const String& rTxt,
            sal_uInt16 nNum, bool bIsEndNote )
    : SwUndo( UNDO_CHGFTN ), SwUndRng( rRange )
    , m_pHistory( new SwHistory() )
    , m_Text( rTxt )
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

    rDoc.GetFtnIdxs().UpdateAllFtn();

    AddUndoRedoPaM(rContext);
}

void SwUndoChangeFootNote::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc( rContext.GetDoc() );
    SwPaM & rPaM = AddUndoRedoPaM(rContext);
    rDoc.SetCurFtn(rPaM, m_Text, m_nNumber, m_bEndNote);
    SetPaM(rPaM);
}

void SwUndoChangeFootNote::RepeatImpl(::sw::RepeatContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    rDoc.SetCurFtn( rContext.GetRepeatPaM(), m_Text, m_nNumber, m_bEndNote );
}

SwUndoFootNoteInfo::SwUndoFootNoteInfo( const SwFtnInfo &rInfo )
    : SwUndo( UNDO_FTNINFO )
    , m_pFootNoteInfo( new SwFtnInfo( rInfo ) )
{
}

SwUndoFootNoteInfo::~SwUndoFootNoteInfo()
{
}

void SwUndoFootNoteInfo::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    SwFtnInfo *pInf = new SwFtnInfo( rDoc.GetFtnInfo() );
    rDoc.SetFtnInfo( *m_pFootNoteInfo );
    m_pFootNoteInfo.reset( pInf );
}

void SwUndoFootNoteInfo::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    SwFtnInfo *pInf = new SwFtnInfo( rDoc.GetFtnInfo() );
    rDoc.SetFtnInfo( *m_pFootNoteInfo );
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

SwUndoDontExpandFmt::SwUndoDontExpandFmt( const SwPosition& rPos )
    : SwUndo( UNDO_DONTEXPAND )
    , m_nNodeIndex( rPos.nNode.GetIndex() )
    , m_nContentIndex( rPos.nContent.GetIndex() )
{
}

void SwUndoDontExpandFmt::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwPaM *const pPam(& rContext.GetCursorSupplier().CreateNewShellCursor());
    SwDoc *const pDoc = & rContext.GetDoc();

    SwPosition& rPos = *pPam->GetPoint();
    rPos.nNode = m_nNodeIndex;
    rPos.nContent.Assign( rPos.nNode.GetNode().GetCntntNode(), m_nContentIndex);
    pDoc->DontExpandFmt( rPos, sal_False );
}

void SwUndoDontExpandFmt::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwPaM *const pPam(& rContext.GetCursorSupplier().CreateNewShellCursor());
    SwDoc *const pDoc = & rContext.GetDoc();

    SwPosition& rPos = *pPam->GetPoint();
    rPos.nNode = m_nNodeIndex;
    rPos.nContent.Assign( rPos.nNode.GetNode().GetCntntNode(), m_nContentIndex);
    pDoc->DontExpandFmt( rPos );
}

void SwUndoDontExpandFmt::RepeatImpl(::sw::RepeatContext & rContext)
{
    SwPaM & rPam = rContext.GetRepeatPaM();
    SwDoc & rDoc = rContext.GetDoc();
    rDoc.DontExpandFmt( *rPam.GetPoint() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
