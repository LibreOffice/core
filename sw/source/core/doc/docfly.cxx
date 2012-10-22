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
#include <svl/itemiter.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdmark.hxx>
#include <fmtfsize.hxx>
#include <fmtornt.hxx>
#include <fmtsrnd.hxx>
#include <dcontact.hxx>

#include <ndgrf.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <ndindex.hxx>
#include <docary.hxx>
#include <fmtcntnt.hxx>
#include <fmtanchr.hxx>
#include <txtflcnt.hxx>
#include <fmtflcnt.hxx>
#include <txtfrm.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <flyfrms.hxx>
#include <frmtool.hxx>
#include <frmfmt.hxx>
#include <ndtxt.hxx>
#include <pam.hxx>
#include <tblsel.hxx>
#include <swundo.hxx>
#include <swtable.hxx>
#include <crstate.hxx>
#include <UndoCore.hxx>
#include <UndoAttribute.hxx>
#include <fmtcnct.hxx>
#include <dflyobj.hxx>
#include <undoflystrattr.hxx>
#include <switerator.hxx>

extern sal_uInt16 GetHtmlMode( const SwDocShell* );

using namespace ::com::sun::star;

sal_uInt16 SwDoc::GetFlyCount( FlyCntType eType ) const
{
    const SwFrmFmts& rFmts = *GetSpzFrmFmts();
    sal_uInt16 nSize = rFmts.size();
    sal_uInt16 nCount = 0;
    const SwNodeIndex* pIdx;
    for ( sal_uInt16 i = 0; i < nSize; i++)
    {
        const SwFrmFmt* pFlyFmt = rFmts[ i ];
        if( RES_FLYFRMFMT == pFlyFmt->Which()
            && 0 != ( pIdx = pFlyFmt->GetCntnt().GetCntntIdx() )
            && pIdx->GetNodes().IsDocNodes()
            )
        {
            const SwNode* pNd = GetNodes()[ pIdx->GetIndex() + 1 ];

            switch( eType )
            {
            case FLYCNTTYPE_FRM:
                if(!pNd->IsNoTxtNode())
                    nCount++;
                break;

            case FLYCNTTYPE_GRF:
                if( pNd->IsGrfNode() )
                    nCount++;
                break;

            case FLYCNTTYPE_OLE:
                if(pNd->IsOLENode())
                    nCount++;
                break;

            default:
                nCount++;
            }
        }
    }
    return nCount;
}

// If you change this, also update SwXFrameEnumeration in unocoll.
SwFrmFmt* SwDoc::GetFlyNum( sal_uInt16 nIdx, FlyCntType eType )
{
    SwFrmFmts& rFmts = *GetSpzFrmFmts();
    SwFrmFmt* pRetFmt = 0;
    sal_uInt16 nSize = rFmts.size();
    const SwNodeIndex* pIdx;
    sal_uInt16 nCount = 0;
    for( sal_uInt16 i = 0; !pRetFmt && i < nSize; ++i )
    {
        SwFrmFmt* pFlyFmt = rFmts[ i ];
        if( RES_FLYFRMFMT == pFlyFmt->Which()
            && 0 != ( pIdx = pFlyFmt->GetCntnt().GetCntntIdx() )
            && pIdx->GetNodes().IsDocNodes()
            )
        {
            const SwNode* pNd = GetNodes()[ pIdx->GetIndex() + 1 ];
            switch( eType )
            {
            case FLYCNTTYPE_FRM:
                if( !pNd->IsNoTxtNode() && nIdx == nCount++)
                    pRetFmt = pFlyFmt;
                break;
            case FLYCNTTYPE_GRF:
                if(pNd->IsGrfNode() && nIdx == nCount++ )
                    pRetFmt = pFlyFmt;
                break;
            case FLYCNTTYPE_OLE:
                if(pNd->IsOLENode() && nIdx == nCount++)
                    pRetFmt = pFlyFmt;
                break;
            default:
                if(nIdx == nCount++)
                    pRetFmt = pFlyFmt;
            }
        }
    }
    return pRetFmt;
}

static Point lcl_FindAnchorLayPos( SwDoc& rDoc, const SwFmtAnchor& rAnch,
                            const SwFrmFmt* pFlyFmt )
{
    Point aRet;
    if( rDoc.GetCurrentViewShell() )    //swmod 071107//swmod 071225
        switch( rAnch.GetAnchorId() )
        {
        case FLY_AS_CHAR:
            if( pFlyFmt && rAnch.GetCntntAnchor() )
            {
                const SwFrm* pOld = ((SwFlyFrmFmt*)pFlyFmt)->GetFrm( &aRet, sal_False );
                if( pOld )
                    aRet = pOld->Frm().Pos();
            }
            break;

        case FLY_AT_PARA:
        case FLY_AT_CHAR: // LAYER_IMPL
            if( rAnch.GetCntntAnchor() )
            {
                const SwPosition *pPos = rAnch.GetCntntAnchor();
                const SwCntntNode* pNd = pPos->nNode.GetNode().GetCntntNode();
                const SwFrm* pOld = pNd ? pNd->getLayoutFrm( rDoc.GetCurrentLayout(), &aRet, 0, sal_False ) : 0;
                if( pOld )
                    aRet = pOld->Frm().Pos();
            }
            break;

        case FLY_AT_FLY: // LAYER_IMPL
            if( rAnch.GetCntntAnchor() )
            {
                const SwFlyFrmFmt* pFmt = (SwFlyFrmFmt*)rAnch.GetCntntAnchor()->
                                                nNode.GetNode().GetFlyFmt();
                const SwFrm* pOld = pFmt ? pFmt->GetFrm( &aRet, sal_False ) : 0;
                if( pOld )
                    aRet = pOld->Frm().Pos();
            }
            break;

        case FLY_AT_PAGE:
            {
                sal_uInt16 nPgNum = rAnch.GetPageNum();
                const SwPageFrm *pPage = (SwPageFrm*)rDoc.GetCurrentLayout()->Lower();
                for( sal_uInt16 i = 1; (i <= nPgNum) && pPage; ++i,
                                    pPage = (const SwPageFrm*)pPage->GetNext() )
                    if( i == nPgNum )
                    {
                        aRet = pPage->Frm().Pos();
                        break;
                    }
            }
            break;
        default:
            break;
        }
    return aRet;
}

#define MAKEFRMS 0
#define IGNOREANCHOR 1
#define DONTMAKEFRMS 2

sal_Int8 SwDoc::SetFlyFrmAnchor( SwFrmFmt& rFmt, SfxItemSet& rSet, bool bNewFrms )
{
    // Changing anchors is almost always allowed.
    // Exception: Paragraph and character bound frames must not become
    // page bound, if they are located in the header or footer.
    const SwFmtAnchor &rOldAnch = rFmt.GetAnchor();
    const RndStdIds nOld = rOldAnch.GetAnchorId();

    SwFmtAnchor aNewAnch( (SwFmtAnchor&)rSet.Get( RES_ANCHOR ) );
    RndStdIds nNew = aNewAnch.GetAnchorId();

    // Is the new anchor valid?
    if( !aNewAnch.GetCntntAnchor() && (FLY_AT_FLY == nNew ||
        (FLY_AT_PARA == nNew) || (FLY_AS_CHAR == nNew) ||
        (FLY_AT_CHAR == nNew) ))
    {
        return IGNOREANCHOR;
    }

    if( nOld == nNew )
        return DONTMAKEFRMS;


    Point aOldAnchorPos( ::lcl_FindAnchorLayPos( *this, rOldAnch, &rFmt ));
    Point aNewAnchorPos( ::lcl_FindAnchorLayPos( *this, aNewAnch, 0 ));

    // Destroy the old Frames.
    // The Views are hidden implicitly, so hiding them another time would be
    // kind of a show!
    rFmt.DelFrms();

    if ( FLY_AS_CHAR == nOld )
    {
        // We need to handle InCntnts in a special way:
        // The TxtAttribut needs to be destroyed which, unfortunately, also
        // destroys the format. To avoid that, we disconnect the format from
        // the attribute.
        const SwPosition *pPos = rOldAnch.GetCntntAnchor();
        SwTxtNode *pTxtNode = pPos->nNode.GetNode().GetTxtNode();
        OSL_ENSURE( pTxtNode->HasHints(), "Missing FlyInCnt-Hint." );
        const xub_StrLen nIdx = pPos->nContent.GetIndex();
        SwTxtAttr * const  pHnt =
            pTxtNode->GetTxtAttrForCharAt( nIdx, RES_TXTATR_FLYCNT );
        OSL_ENSURE( pHnt && pHnt->Which() == RES_TXTATR_FLYCNT,
                    "Missing FlyInCnt-Hint." );
        OSL_ENSURE( pHnt && pHnt->GetFlyCnt().GetFrmFmt() == &rFmt,
                    "Wrong TxtFlyCnt-Hint." );
        if (pHnt)
            const_cast<SwFmtFlyCnt&>(pHnt->GetFlyCnt()).SetFlyFmt();

        // They are disconnected. We now have to destroy the attribute.
        pTxtNode->DeleteAttributes( RES_TXTATR_FLYCNT, nIdx, nIdx );
    }

    // We can finally set the attribute. It needs to be the first one!
    // Undo depends on it!
    rFmt.SetFmtAttr( aNewAnch );

    // Correct the position
    const SfxPoolItem* pItem;
    switch( nNew )
    {
    case FLY_AS_CHAR:
            // If no position attributes are received, we have to make sure
            // that no forbidden automatic alignment is left.
        {
            const SwPosition *pPos = aNewAnch.GetCntntAnchor();
            SwTxtNode *pNd = pPos->nNode.GetNode().GetTxtNode();
            OSL_ENSURE( pNd, "Crsr does not point to TxtNode." );

            SwFmtFlyCnt aFmt( static_cast<SwFlyFrmFmt*>(&rFmt) );
            pNd->InsertItem( aFmt, pPos->nContent.GetIndex(), 0 );
        }

        if( SFX_ITEM_SET != rSet.GetItemState( RES_VERT_ORIENT, sal_False, &pItem ))
        {
            SwFmtVertOrient aOldV( rFmt.GetVertOrient() );
            bool bSet = true;
            switch( aOldV.GetVertOrient() )
            {
            case text::VertOrientation::LINE_TOP:     aOldV.SetVertOrient( text::VertOrientation::TOP );   break;
            case text::VertOrientation::LINE_CENTER:  aOldV.SetVertOrient( text::VertOrientation::CENTER); break;
            case text::VertOrientation::LINE_BOTTOM:  aOldV.SetVertOrient( text::VertOrientation::BOTTOM); break;
            case text::VertOrientation::NONE:         aOldV.SetVertOrient( text::VertOrientation::CENTER); break;
            default:
                bSet = false;
            }
            if( bSet )
                rSet.Put( aOldV );
        }
        break;

    case FLY_AT_PARA:
    case FLY_AT_CHAR:   // LAYER_IMPL
    case FLY_AT_FLY:    // LAYER_IMPL
    case FLY_AT_PAGE:
        {
            // If no position attributes are coming in, we correct the position in a way
            // such that the fly's document coordinates are preserved.
            // If only the alignment changes in the position attributes (text::RelOrientation::FRAME
            // vs. text::RelOrientation::PRTAREA), we also correct the position.
            if( SFX_ITEM_SET != rSet.GetItemState( RES_HORI_ORIENT, sal_False, &pItem ))
                pItem = 0;

            SwFmtHoriOrient aOldH( rFmt.GetHoriOrient() );

            if( text::HoriOrientation::NONE == aOldH.GetHoriOrient() && ( !pItem ||
                aOldH.GetPos() == ((SwFmtHoriOrient*)pItem)->GetPos() ))
            {
                SwTwips nPos = (FLY_AS_CHAR == nOld) ? 0 : aOldH.GetPos();
                nPos += aOldAnchorPos.X() - aNewAnchorPos.X();

                if( pItem )
                {
                    SwFmtHoriOrient* pH = (SwFmtHoriOrient*)pItem;
                    aOldH.SetHoriOrient( pH->GetHoriOrient() );
                    aOldH.SetRelationOrient( pH->GetRelationOrient() );
                }
                aOldH.SetPos( nPos );
                rSet.Put( aOldH );
            }

            if( SFX_ITEM_SET != rSet.GetItemState( RES_VERT_ORIENT, sal_False, &pItem ))
                pItem = 0;
            SwFmtVertOrient aOldV( rFmt.GetVertOrient() );

            // #i28922# - correction: compare <aOldV.GetVertOrient() with
            // <text::VertOrientation::NONE>
            if( text::VertOrientation::NONE == aOldV.GetVertOrient() && (!pItem ||
                aOldV.GetPos() == ((SwFmtVertOrient*)pItem)->GetPos() ) )
            {
                SwTwips nPos = (FLY_AS_CHAR == nOld) ? 0 : aOldV.GetPos();
                nPos += aOldAnchorPos.Y() - aNewAnchorPos.Y();
                if( pItem )
                {
                    SwFmtVertOrient* pV = (SwFmtVertOrient*)pItem;
                    aOldV.SetVertOrient( pV->GetVertOrient() );
                    aOldV.SetRelationOrient( pV->GetRelationOrient() );
                }
                aOldV.SetPos( nPos );
                rSet.Put( aOldV );
            }
        }
        break;
    default:
        break;
    }

    if( bNewFrms )
        rFmt.MakeFrms();

    return MAKEFRMS;
}

static bool
lcl_SetFlyFrmAttr(SwDoc & rDoc,
        sal_Int8 (SwDoc::*pSetFlyFrmAnchor)(SwFrmFmt &, SfxItemSet &, bool),
        SwFrmFmt & rFlyFmt, SfxItemSet & rSet)
{
    // #i32968# Inserting columns in the frame causes MakeFrmFmt to put two
    // objects of type SwUndoFrmFmt on the undo stack. We don't want them.
    ::sw::UndoGuard const undoGuard(rDoc.GetIDocumentUndoRedo());

    // Is the anchor attribute included?
    // If so, we pass it to a special method, which returns sal_True
    // if the Fly needs to be created anew, because we e.g change the FlyType.
    sal_Int8 const nMakeFrms =
        (SFX_ITEM_SET == rSet.GetItemState( RES_ANCHOR, sal_False ))
             ?  (rDoc.*pSetFlyFrmAnchor)( rFlyFmt, rSet, false )
             :  DONTMAKEFRMS;

    const SfxPoolItem* pItem;
    SfxItemIter aIter( rSet );
    SfxItemSet aTmpSet( rDoc.GetAttrPool(), aFrmFmtSetRange );
    sal_uInt16 nWhich = aIter.GetCurItem()->Which();
    do {
        switch( nWhich )
        {
        case RES_FILL_ORDER:
        case RES_BREAK:
        case RES_PAGEDESC:
        case RES_CNTNT:
        case RES_FOOTER:
            OSL_FAIL( "Unknown Fly attribute." );
            // no break;
        case RES_CHAIN:
            rSet.ClearItem( nWhich );
            break;
        case RES_ANCHOR:
            if( DONTMAKEFRMS != nMakeFrms )
                break;

        default:
            if( !IsInvalidItem( aIter.GetCurItem() ) && ( SFX_ITEM_SET !=
                rFlyFmt.GetAttrSet().GetItemState( nWhich, sal_True, &pItem ) ||
                *pItem != *aIter.GetCurItem() ))
                aTmpSet.Put( *aIter.GetCurItem() );
            break;
        }

        if( aIter.IsAtEnd() )
            break;

    } while( 0 != ( nWhich = aIter.NextItem()->Which() ) );

    if( aTmpSet.Count() )
        rFlyFmt.SetFmtAttr( aTmpSet );

    if( MAKEFRMS == nMakeFrms )
        rFlyFmt.MakeFrms();

    return aTmpSet.Count() || MAKEFRMS == nMakeFrms;
}

bool SwDoc::SetFlyFrmAttr( SwFrmFmt& rFlyFmt, SfxItemSet& rSet )
{
    if( !rSet.Count() )
        return false;

    ::std::auto_ptr<SwUndoFmtAttrHelper> pSaveUndo;

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().ClearRedo(); // AppendUndo far below, so leave it
        pSaveUndo.reset( new SwUndoFmtAttrHelper( rFlyFmt ) );
    }

    bool const bRet =
        lcl_SetFlyFrmAttr(*this, &SwDoc::SetFlyFrmAnchor, rFlyFmt, rSet);

    if ( pSaveUndo.get() )
    {
        if ( pSaveUndo->GetUndo() )
        {
            GetIDocumentUndoRedo().AppendUndo( pSaveUndo->ReleaseUndo() );
        }
    }

    SetModified();

    return bRet;
}

// #i73249#
void SwDoc::SetFlyFrmTitle( SwFlyFrmFmt& rFlyFrmFmt,
                            const String& sNewTitle )
{
    if ( rFlyFrmFmt.GetObjTitle() == sNewTitle )
    {
        return;
    }

    ::sw::DrawUndoGuard const drawUndoGuard(GetIDocumentUndoRedo());

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().AppendUndo( new SwUndoFlyStrAttr( rFlyFrmFmt,
                                          UNDO_FLYFRMFMT_TITLE,
                                          rFlyFrmFmt.GetObjTitle(),
                                          sNewTitle ) );
    }

    rFlyFrmFmt.SetObjTitle( sNewTitle, true );

    SetModified();
}

void SwDoc::SetFlyFrmDescription( SwFlyFrmFmt& rFlyFrmFmt,
                                  const String& sNewDescription )
{
    if ( rFlyFrmFmt.GetObjDescription() == sNewDescription )
    {
        return;
    }

    ::sw::DrawUndoGuard const drawUndoGuard(GetIDocumentUndoRedo());

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().AppendUndo( new SwUndoFlyStrAttr( rFlyFrmFmt,
                                          UNDO_FLYFRMFMT_DESCRIPTION,
                                          rFlyFrmFmt.GetObjDescription(),
                                          sNewDescription ) );
    }

    rFlyFrmFmt.SetObjDescription( sNewDescription, true );

    SetModified();
}

bool SwDoc::SetFrmFmtToFly( SwFrmFmt& rFmt, SwFrmFmt& rNewFmt,
                            SfxItemSet* pSet, bool bKeepOrient )
{
    bool bChgAnchor = false, bFrmSz = false;

    const SwFmtFrmSize aFrmSz( rFmt.GetFrmSize() );
    const SwFmtVertOrient aVert( rFmt.GetVertOrient() );
    const SwFmtHoriOrient aHori( rFmt.GetHoriOrient() );

    SwUndoSetFlyFmt* pUndo = 0;
    bool const bUndo = GetIDocumentUndoRedo().DoesUndo();
    if (bUndo)
    {
        pUndo = new SwUndoSetFlyFmt( rFmt, rNewFmt );
        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }

    // #i32968# Inserting columns in the section causes MakeFrmFmt to put
    // 2 objects of type SwUndoFrmFmt on the undo stack. We don't want them.
    ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());

    // Set the column first, or we'll have trouble with
    //Set/Reset/Synch. and so on
    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET != rNewFmt.GetAttrSet().GetItemState( RES_COL ))
        rFmt.ResetFmtAttr( RES_COL );

    if( rFmt.DerivedFrom() != &rNewFmt )
    {
        rFmt.SetDerivedFrom( &rNewFmt );

        // 1. If not automatic = ignore; else = dispose
        // 2. Dispose of it!
        if( SFX_ITEM_SET == rNewFmt.GetAttrSet().GetItemState( RES_FRM_SIZE, sal_False ))
        {
            rFmt.ResetFmtAttr( RES_FRM_SIZE );
            bFrmSz = true;
        }

        const SfxItemSet* pAsk = pSet;
        if( !pAsk ) pAsk = &rNewFmt.GetAttrSet();
        if( SFX_ITEM_SET == pAsk->GetItemState( RES_ANCHOR, sal_False, &pItem )
            && ((SwFmtAnchor*)pItem)->GetAnchorId() !=
                rFmt.GetAnchor().GetAnchorId() )
        {
            if( pSet )
                bChgAnchor = MAKEFRMS == SetFlyFrmAnchor( rFmt, *pSet, false );
            else
            {
                // Needs to have the FlyFmt range, because we set attributes in it,
                // in SetFlyFrmAnchor.
                SfxItemSet aFlySet( *rNewFmt.GetAttrSet().GetPool(),
                                    rNewFmt.GetAttrSet().GetRanges() );
                aFlySet.Put( *pItem );
                bChgAnchor = MAKEFRMS == SetFlyFrmAnchor( rFmt, aFlySet, false);
            }
        }
    }

    // Only reset vertical and horizontal orientation, if we have automatic alignment
    // set in the template. Otherwise use the old value.
    // If we update the frame template the Fly should NOT lose its orientation (which
    // is not being updated!).
    // text::HoriOrientation::NONE and text::VertOrientation::NONE are allowed now
    if (!bKeepOrient)
    {
        rFmt.ResetFmtAttr(RES_VERT_ORIENT);
        rFmt.ResetFmtAttr(RES_HORI_ORIENT);
    }

    rFmt.ResetFmtAttr( RES_PRINT, RES_SURROUND );
    rFmt.ResetFmtAttr( RES_LR_SPACE, RES_UL_SPACE );
    rFmt.ResetFmtAttr( RES_BACKGROUND, RES_COL );
    rFmt.ResetFmtAttr( RES_URL, RES_EDIT_IN_READONLY );

    if( !bFrmSz )
        rFmt.SetFmtAttr( aFrmSz );

    if( bChgAnchor )
        rFmt.MakeFrms();

    if( pUndo )
        pUndo->DeRegisterFromFormat( rFmt );

    SetModified();

    return bChgAnchor;
}

void SwDoc::GetGrfNms( const SwFlyFrmFmt& rFmt, String* pGrfName,
                        String* pFltName ) const
{
    SwNodeIndex aIdx( *rFmt.GetCntnt().GetCntntIdx(), 1 );
    const SwGrfNode* pGrfNd = aIdx.GetNode().GetGrfNode();
    if( pGrfNd && pGrfNd->IsLinkedFile() )
        pGrfNd->GetFileFilterNms( pGrfName, pFltName );
}

sal_Bool SwDoc::ChgAnchor( const SdrMarkList& _rMrkList,
                           RndStdIds _eAnchorType,
                           const sal_Bool _bSameOnly,
                           const sal_Bool _bPosCorr )
{
    OSL_ENSURE( GetCurrentLayout(), "No layout!" );

    if ( !_rMrkList.GetMarkCount() ||
         _rMrkList.GetMark( 0 )->GetMarkedSdrObj()->GetUpGroup() )
    {
        return false;
    }

    GetIDocumentUndoRedo().StartUndo( UNDO_INSATTR, NULL );

    sal_Bool bUnmark = sal_False;
    for ( sal_uInt16 i = 0; i < _rMrkList.GetMarkCount(); ++i )
    {
        SdrObject* pObj = _rMrkList.GetMark( i )->GetMarkedSdrObj();
        if ( !pObj->ISA(SwVirtFlyDrawObj) )
        {
            SwDrawContact* pContact = static_cast<SwDrawContact*>(GetUserCall(pObj));

            // consider, that drawing object has
            // no user call. E.g.: a 'virtual' drawing object is disconnected by
            // the anchor type change of the 'master' drawing object.
            // Continue with next selected object and assert, if this isn't excepted.
            if ( !pContact )
            {
#if OSL_DEBUG_LEVEL > 0
                bool bNoUserCallExcepted =
                        pObj->ISA(SwDrawVirtObj) &&
                        !static_cast<SwDrawVirtObj*>(pObj)->IsConnected();
                OSL_ENSURE( bNoUserCallExcepted, "SwDoc::ChgAnchor(..) - no contact at selected drawing object" );
#endif
                continue;
            }

            // #i26791#
            const SwFrm* pOldAnchorFrm = pContact->GetAnchorFrm( pObj );
            const SwFrm* pNewAnchorFrm = pOldAnchorFrm;

            // #i54336#
            // Instead of only keeping the index position for an as-character
            // anchored object the complete <SwPosition> is kept, because the
            // anchor index position could be moved, if the object again is
            // anchored as character.
            const SwPosition* pOldAsCharAnchorPos( 0L );
            const RndStdIds eOldAnchorType = pContact->GetAnchorId();
            if ( !_bSameOnly && eOldAnchorType == FLY_AS_CHAR )
            {
                pOldAsCharAnchorPos = new SwPosition( pContact->GetCntntAnchor() );
            }

            if ( _bSameOnly )
                _eAnchorType = eOldAnchorType;

            SwFmtAnchor aNewAnch( _eAnchorType );
            Rectangle aObjRect( pContact->GetAnchoredObj( pObj )->GetObjRect().SVRect() );
            const Point aPt( aObjRect.TopLeft() );

            switch ( _eAnchorType )
            {
            case FLY_AT_PARA:
            case FLY_AT_CHAR:
                {
                    const Point aNewPoint = pOldAnchorFrm &&
                                            ( pOldAnchorFrm->IsVertical() ||
                                              pOldAnchorFrm->IsRightToLeft() )
                                            ? aObjRect.TopRight()
                                            : aPt;

                    // allow drawing objects in header/footer
                    pNewAnchorFrm = ::FindAnchor( pOldAnchorFrm, aNewPoint, false );
                    if ( pNewAnchorFrm->IsTxtFrm() && ((SwTxtFrm*)pNewAnchorFrm)->IsFollow() )
                    {
                        pNewAnchorFrm = ((SwTxtFrm*)pNewAnchorFrm)->FindMaster();
                    }
                    if ( pNewAnchorFrm->IsProtected() )
                    {
                        pNewAnchorFrm = 0;
                    }
                    else
                    {
                        SwPosition aPos( *((SwCntntFrm*)pNewAnchorFrm)->GetNode() );
                        aNewAnch.SetType( _eAnchorType );
                        aNewAnch.SetAnchor( &aPos );
                    }
                }
                break;

            case FLY_AT_FLY: // LAYER_IMPL
                {
                    // Search the closest SwFlyFrm starting from the upper left corner.
                    SwFrm *pTxtFrm;
                    {
                        SwCrsrMoveState aState( MV_SETONLYTEXT );
                        SwPosition aPos( GetNodes() );
                        Point aPoint( aPt );
                        aPoint.X() -= 1;
                        GetCurrentLayout()->GetCrsrOfst( &aPos, aPoint, &aState );
                        // consider that drawing objects can be in
                        // header/footer. Thus, <GetFrm()> by left-top-corner
                        pTxtFrm = aPos.nNode.GetNode().
                                        GetCntntNode()->getLayoutFrm( GetCurrentLayout(), &aPt, 0, sal_False );
                    }
                    const SwFrm *pTmp = ::FindAnchor( pTxtFrm, aPt );
                    pNewAnchorFrm = pTmp->FindFlyFrm();
                    if( pNewAnchorFrm && !pNewAnchorFrm->IsProtected() )
                    {
                        const SwFrmFmt *pTmpFmt = ((SwFlyFrm*)pNewAnchorFrm)->GetFmt();
                        const SwFmtCntnt& rCntnt = pTmpFmt->GetCntnt();
                        SwPosition aPos( *rCntnt.GetCntntIdx() );
                        aNewAnch.SetAnchor( &aPos );
                        break;
                    }

                    aNewAnch.SetType( FLY_AT_PAGE );
                    // no break
                }
            case FLY_AT_PAGE:
                {
                    pNewAnchorFrm = GetCurrentLayout()->Lower();
                    while ( pNewAnchorFrm && !pNewAnchorFrm->Frm().IsInside( aPt ) )
                        pNewAnchorFrm = pNewAnchorFrm->GetNext();
                    if ( !pNewAnchorFrm )
                        continue;

                    aNewAnch.SetPageNum( ((SwPageFrm*)pNewAnchorFrm)->GetPhyPageNum());
                }
                break;
            case FLY_AS_CHAR:
                if( _bSameOnly )    // Change of position/size
                {
                    if( !pOldAnchorFrm )
                    {
                        pContact->ConnectToLayout();
                        pOldAnchorFrm = pContact->GetAnchorFrm();
                    }
                    ((SwTxtFrm*)pOldAnchorFrm)->Prepare();
                }
                else            // Change of anchors
                {
                    // allow drawing objects in header/footer
                    pNewAnchorFrm = ::FindAnchor( pOldAnchorFrm, aPt, false );
                    if( pNewAnchorFrm->IsProtected() )
                    {
                        pNewAnchorFrm = 0;
                        break;
                    }

                    bUnmark = ( 0 != i );
                    Point aPoint( aPt );
                    aPoint.X() -= 1;    // Do not load in the DrawObj!
                    aNewAnch.SetType( FLY_AS_CHAR );
                    SwPosition aPos( *((SwCntntFrm*)pNewAnchorFrm)->GetNode() );
                    if ( pNewAnchorFrm->Frm().IsInside( aPoint ) )
                    {
                    // We need to find a TextNode, because only there we can anchor a
                    // content-bound DrawObject.
                        SwCrsrMoveState aState( MV_SETONLYTEXT );
                        GetCurrentLayout()->GetCrsrOfst( &aPos, aPoint, &aState );  //swmod 080218
                    }
                    else
                    {
                        SwCntntNode &rCNd = (SwCntntNode&)
                            *((SwCntntFrm*)pNewAnchorFrm)->GetNode();
                        if ( pNewAnchorFrm->Frm().Bottom() < aPt.Y() )
                            rCNd.MakeStartIndex( &aPos.nContent );
                        else
                            rCNd.MakeEndIndex( &aPos.nContent );
                    }
                    aNewAnch.SetAnchor( &aPos );
                    SetAttr( aNewAnch, *pContact->GetFmt() );
                    // #i26791# - adjust vertical positioning to 'center to
                    // baseline'
                    SetAttr( SwFmtVertOrient( 0, text::VertOrientation::CENTER, text::RelOrientation::FRAME ), *pContact->GetFmt() );
                    SwTxtNode *pNd = aPos.nNode.GetNode().GetTxtNode();
                    OSL_ENSURE( pNd, "Cursor not positioned at TxtNode." );

                    SwFmtFlyCnt aFmt( pContact->GetFmt() );
                    pNd->InsertItem( aFmt, aPos.nContent.GetIndex(), 0 );
                }
                break;
            default:
                OSL_ENSURE( !this, "unexpected AnchorId." );
            }

            if ( (FLY_AS_CHAR != _eAnchorType) &&
                 pNewAnchorFrm &&
                 ( !_bSameOnly || pNewAnchorFrm != pOldAnchorFrm ) )
            {
                // #i26791# - Direct object positioning no longer needed. Apply
                // of attributes (method call <SetAttr(..)>) takes care of the
                // invalidation of the object position.
                SetAttr( aNewAnch, *pContact->GetFmt() );
                if ( _bPosCorr )
                {
                    // #i33313# - consider not connected 'virtual' drawing
                    // objects
                    if ( pObj->ISA(SwDrawVirtObj) &&
                         !static_cast<SwDrawVirtObj*>(pObj)->IsConnected() )
                    {
                        SwRect aNewObjRect( aObjRect );
                        static_cast<SwAnchoredDrawObject*>(pContact->GetAnchoredObj( 0L ))
                                        ->AdjustPositioningAttr( pNewAnchorFrm,
                                                                 &aNewObjRect );

                    }
                    else
                    {
                        static_cast<SwAnchoredDrawObject*>(pContact->GetAnchoredObj( pObj ))
                                    ->AdjustPositioningAttr( pNewAnchorFrm );
                    }
                }
            }

            // #i54336#
            if ( pNewAnchorFrm && pOldAsCharAnchorPos )
            {
                // We need to handle InCntnts in a special way:
                // The TxtAttribut needs to be destroyed which, unfortunately, also
                // destroys the format. To avoid that, we disconnect the format from
                // the attribute.
                const xub_StrLen nIndx( pOldAsCharAnchorPos->nContent.GetIndex() );
                SwTxtNode* pTxtNode( pOldAsCharAnchorPos->nNode.GetNode().GetTxtNode() );
                OSL_ENSURE( pTxtNode, "<SwDoc::ChgAnchor(..)> - missing previous anchor text node for as-character anchored object" );
                OSL_ENSURE( pTxtNode->HasHints(), "Missing FlyInCnt-Hint." );
                SwTxtAttr * const pHnt =
                    pTxtNode->GetTxtAttrForCharAt( nIndx, RES_TXTATR_FLYCNT );
                const_cast<SwFmtFlyCnt&>(pHnt->GetFlyCnt()).SetFlyFmt();

                // They are disconnected. We now have to destroy the attribute.
                pTxtNode->DeleteAttributes( RES_TXTATR_FLYCNT, nIndx, nIndx );
                delete pOldAsCharAnchorPos;
            }
        }
    }

    GetIDocumentUndoRedo().EndUndo( UNDO_END, NULL );
    SetModified();

    return bUnmark;
}

int SwDoc::Chainable( const SwFrmFmt &rSource, const SwFrmFmt &rDest )
{
    // The Source must not yet have a Follow.
    const SwFmtChain &rOldChain = rSource.GetChain();
    if ( rOldChain.GetNext() )
        return SW_CHAIN_SOURCE_CHAINED;

    // Target must not be equal to Source and we also must not have a closed chain.
    const SwFrmFmt *pFmt = &rDest;
    do {
        if( pFmt == &rSource )
            return SW_CHAIN_SELF;
        pFmt = pFmt->GetChain().GetNext();
    } while ( pFmt );

    // There must not be a chaining from outside to inside or the other way around.
    if( rDest.IsLowerOf( rSource ) || rSource .IsLowerOf( rDest ) )
        return SW_CHAIN_SELF;

    // The Target must not yet have a Master.
    const SwFmtChain &rChain = rDest.GetChain();
    if( rChain.GetPrev() )
        return SW_CHAIN_IS_IN_CHAIN;

    // Target must be empty.
    const SwNodeIndex* pCntIdx = rDest.GetCntnt().GetCntntIdx();
    if( !pCntIdx )
        return SW_CHAIN_NOT_FOUND;

    SwNodeIndex aNxtIdx( *pCntIdx, 1 );
    const SwTxtNode* pTxtNd = aNxtIdx.GetNode().GetTxtNode();
    if( !pTxtNd )
        return SW_CHAIN_NOT_FOUND;

    const sal_uLong nFlySttNd = pCntIdx->GetIndex();
    if( 2 != ( pCntIdx->GetNode().EndOfSectionIndex() - nFlySttNd ) ||
        pTxtNd->GetTxt().Len() )
        return SW_CHAIN_NOT_EMPTY;

    sal_uInt16 nArrLen = GetSpzFrmFmts()->size();
    for( sal_uInt16 n = 0; n < nArrLen; ++n )
    {
        const SwFmtAnchor& rAnchor = (*GetSpzFrmFmts())[ n ]->GetAnchor();
        sal_uLong nTstSttNd;
        // #i20622# - to-frame anchored objects are allowed.
        if ( ((rAnchor.GetAnchorId() == FLY_AT_PARA) ||
              (rAnchor.GetAnchorId() == FLY_AT_CHAR)) &&
             0 != rAnchor.GetCntntAnchor() &&
             nFlySttNd <= ( nTstSttNd =
                         rAnchor.GetCntntAnchor()->nNode.GetIndex() ) &&
             nTstSttNd < nFlySttNd + 2 )
        {
            return SW_CHAIN_NOT_EMPTY;
        }
    }

    // We also need to consider the right area.
    // Both Flys need to be located in the same area (Body, Header/Footer, Fly).
    // If the Source is not the selected frame, it's enough to find a suitable
    // one. e.g. if it's requested by the API.

    // both in the same fly, header, footer or on the page?
    const SwFmtAnchor &rSrcAnchor = rSource.GetAnchor(),
                      &rDstAnchor = rDest.GetAnchor();
    sal_uLong nEndOfExtras = GetNodes().GetEndOfExtras().GetIndex();
    sal_Bool bAllowed = sal_False;
    if ( FLY_AT_PAGE == rSrcAnchor.GetAnchorId() )
    {
        if ( (FLY_AT_PAGE == rDstAnchor.GetAnchorId()) ||
            ( rDstAnchor.GetCntntAnchor() &&
              rDstAnchor.GetCntntAnchor()->nNode.GetIndex() > nEndOfExtras ))
            bAllowed = sal_True;
    }
    else if( rSrcAnchor.GetCntntAnchor() && rDstAnchor.GetCntntAnchor() )
    {
        const SwNodeIndex &rSrcIdx = rSrcAnchor.GetCntntAnchor()->nNode,
                            &rDstIdx = rDstAnchor.GetCntntAnchor()->nNode;
        const SwStartNode* pSttNd = 0;
        if( rSrcIdx == rDstIdx ||
            ( !pSttNd &&
                0 != ( pSttNd = rSrcIdx.GetNode().FindFlyStartNode() ) &&
                pSttNd == rDstIdx.GetNode().FindFlyStartNode() ) ||
            ( !pSttNd &&
                0 != ( pSttNd = rSrcIdx.GetNode().FindFooterStartNode() ) &&
                pSttNd == rDstIdx.GetNode().FindFooterStartNode() ) ||
            ( !pSttNd &&
                0 != ( pSttNd = rSrcIdx.GetNode().FindHeaderStartNode() ) &&
                pSttNd == rDstIdx.GetNode().FindHeaderStartNode() ) ||
            ( !pSttNd && rDstIdx.GetIndex() > nEndOfExtras &&
                            rSrcIdx.GetIndex() > nEndOfExtras ))
            bAllowed = sal_True;
    }

    return bAllowed ? SW_CHAIN_OK : SW_CHAIN_WRONG_AREA;
}

int SwDoc::Chain( SwFrmFmt &rSource, const SwFrmFmt &rDest )
{
    int nErr = Chainable( rSource, rDest );
    if ( !nErr )
    {
        GetIDocumentUndoRedo().StartUndo( UNDO_CHAINE, NULL );

        SwFlyFrmFmt& rDestFmt = (SwFlyFrmFmt&)rDest;

        // Attach Follow to the Master.
        SwFmtChain aChain = rDestFmt.GetChain();
        aChain.SetPrev( &(SwFlyFrmFmt&)rSource );
        SetAttr( aChain, rDestFmt );

        SfxItemSet aSet( GetAttrPool(), RES_FRM_SIZE, RES_FRM_SIZE,
                                        RES_CHAIN,  RES_CHAIN, 0 );

        // Attach Follow to the Master.
        aChain.SetPrev( &(SwFlyFrmFmt&)rSource );
        SetAttr( aChain, rDestFmt );

        // Attach Master to the Follow.
        // Make sure that the Master has a fixed height.
        aChain = rSource.GetChain();
        aChain.SetNext( &rDestFmt );
        aSet.Put( aChain );

        SwFmtFrmSize aSize( rSource.GetFrmSize() );
        if ( aSize.GetHeightSizeType() != ATT_FIX_SIZE )
        {
            SwFlyFrm *pFly = SwIterator<SwFlyFrm,SwFmt>::FirstElement( rSource );
            if ( pFly )
                aSize.SetHeight( pFly->Frm().Height() );
            aSize.SetHeightSizeType( ATT_FIX_SIZE );
            aSet.Put( aSize );
        }
        SetAttr( aSet, rSource );

        GetIDocumentUndoRedo().EndUndo( UNDO_CHAINE, NULL );
    }
    return nErr;
}

void SwDoc::Unchain( SwFrmFmt &rFmt )
{
    SwFmtChain aChain( rFmt.GetChain() );
    if ( aChain.GetNext() )
    {
        GetIDocumentUndoRedo().StartUndo( UNDO_UNCHAIN, NULL );
        SwFrmFmt *pFollow = aChain.GetNext();
        aChain.SetNext( 0 );
        SetAttr( aChain, rFmt );
        aChain = pFollow->GetChain();
        aChain.SetPrev( 0 );
        SetAttr( aChain, *pFollow );
        GetIDocumentUndoRedo().EndUndo( UNDO_UNCHAIN, NULL );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
