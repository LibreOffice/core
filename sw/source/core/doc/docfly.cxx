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
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentState.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <ndindex.hxx>
#include <docary.hxx>
#include <drawdoc.hxx>
#include <fmtcntnt.hxx>
#include <fmtanchr.hxx>
#include <txtflcnt.hxx>
#include <fmtflcnt.hxx>
#include <txtfrm.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <flyfrms.hxx>
#include <textboxhelper.hxx>
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
#include <calbck.hxx>
#include <boost/scoped_ptr.hpp>

//UUUU
#include <svx/xbtmpit.hxx>
#include <svx/xflftrit.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xflhtit.hxx>

using namespace ::com::sun::star;

size_t SwDoc::GetFlyCount( FlyCntType eType, bool bIgnoreTextBoxes ) const
{
    const SwFrameFormats& rFormats = *GetSpzFrameFormats();
    const size_t nSize = rFormats.size();
    size_t nCount = 0;
    const SwNodeIndex* pIdx;

    std::set<const SwFrameFormat*> aTextBoxes;
    if (bIgnoreTextBoxes)
        aTextBoxes = SwTextBoxHelper::findTextBoxes(this);

    for ( size_t i = 0; i < nSize; ++i)
    {
        const SwFrameFormat* pFlyFormat = rFormats[ i ];

        if (bIgnoreTextBoxes && aTextBoxes.find(pFlyFormat) != aTextBoxes.end())
            continue;

        if( RES_FLYFRMFMT == pFlyFormat->Which()
            && 0 != ( pIdx = pFlyFormat->GetContent().GetContentIdx() )
            && pIdx->GetNodes().IsDocNodes()
            )
        {
            const SwNode* pNd = GetNodes()[ pIdx->GetIndex() + 1 ];

            switch( eType )
            {
            case FLYCNTTYPE_FRM:
                if(!pNd->IsNoTextNode())
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

/// @attention If you change this, also update SwXFrameEnumeration in unocoll.
SwFrameFormat* SwDoc::GetFlyNum( size_t nIdx, FlyCntType eType, bool bIgnoreTextBoxes )
{
    SwFrameFormats& rFormats = *GetSpzFrameFormats();
    SwFrameFormat* pRetFormat = 0;
    const size_t nSize = rFormats.size();
    const SwNodeIndex* pIdx;
    size_t nCount = 0;

    std::set<const SwFrameFormat*> aTextBoxes;
    if (bIgnoreTextBoxes)
        aTextBoxes = SwTextBoxHelper::findTextBoxes(this);

    for( size_t i = 0; !pRetFormat && i < nSize; ++i )
    {
        SwFrameFormat* pFlyFormat = rFormats[ i ];

        if (bIgnoreTextBoxes && aTextBoxes.find(pFlyFormat) != aTextBoxes.end())
            continue;

        if( RES_FLYFRMFMT == pFlyFormat->Which()
            && 0 != ( pIdx = pFlyFormat->GetContent().GetContentIdx() )
            && pIdx->GetNodes().IsDocNodes()
            )
        {
            const SwNode* pNd = GetNodes()[ pIdx->GetIndex() + 1 ];
            switch( eType )
            {
            case FLYCNTTYPE_FRM:
                if( !pNd->IsNoTextNode() && nIdx == nCount++)
                    pRetFormat = pFlyFormat;
                break;
            case FLYCNTTYPE_GRF:
                if(pNd->IsGrfNode() && nIdx == nCount++ )
                    pRetFormat = pFlyFormat;
                break;
            case FLYCNTTYPE_OLE:
                if(pNd->IsOLENode() && nIdx == nCount++)
                    pRetFormat = pFlyFormat;
                break;
            default:
                if(nIdx == nCount++)
                    pRetFormat = pFlyFormat;
            }
        }
    }
    return pRetFormat;
}

static Point lcl_FindAnchorLayPos( SwDoc& rDoc, const SwFormatAnchor& rAnch,
                            const SwFrameFormat* pFlyFormat )
{
    Point aRet;
    if( rDoc.getIDocumentLayoutAccess().GetCurrentViewShell() )
        switch( rAnch.GetAnchorId() )
        {
        case FLY_AS_CHAR:
            if( pFlyFormat && rAnch.GetContentAnchor() )
            {
                const SwFrm* pOld = static_cast<const SwFlyFrameFormat*>(pFlyFormat)->GetFrm( &aRet, false );
                if( pOld )
                    aRet = pOld->Frm().Pos();
            }
            break;

        case FLY_AT_PARA:
        case FLY_AT_CHAR: // LAYER_IMPL
            if( rAnch.GetContentAnchor() )
            {
                const SwPosition *pPos = rAnch.GetContentAnchor();
                const SwContentNode* pNd = pPos->nNode.GetNode().GetContentNode();
                const SwFrm* pOld = pNd ? pNd->getLayoutFrm( rDoc.getIDocumentLayoutAccess().GetCurrentLayout(), &aRet, 0, false ) : 0;
                if( pOld )
                    aRet = pOld->Frm().Pos();
            }
            break;

        case FLY_AT_FLY: // LAYER_IMPL
            if( rAnch.GetContentAnchor() )
            {
                const SwFlyFrameFormat* pFormat = static_cast<SwFlyFrameFormat*>(rAnch.GetContentAnchor()->
                                                nNode.GetNode().GetFlyFormat());
                const SwFrm* pOld = pFormat ? pFormat->GetFrm( &aRet, false ) : 0;
                if( pOld )
                    aRet = pOld->Frm().Pos();
            }
            break;

        case FLY_AT_PAGE:
            {
                sal_uInt16 nPgNum = rAnch.GetPageNum();
                const SwPageFrm *pPage = static_cast<SwPageFrm*>(rDoc.getIDocumentLayoutAccess().GetCurrentLayout()->Lower());
                for( sal_uInt16 i = 1; (i <= nPgNum) && pPage; ++i,
                                    pPage =static_cast<const SwPageFrm*>(pPage->GetNext()) )
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

sal_Int8 SwDoc::SetFlyFrmAnchor( SwFrameFormat& rFormat, SfxItemSet& rSet, bool bNewFrms )
{
    // Changing anchors is almost always allowed.
    // Exception: Paragraph and character bound frames must not become
    // page bound, if they are located in the header or footer.
    const SwFormatAnchor &rOldAnch = rFormat.GetAnchor();
    const RndStdIds nOld = rOldAnch.GetAnchorId();

    SwFormatAnchor aNewAnch( static_cast<const SwFormatAnchor&>(rSet.Get( RES_ANCHOR )) );
    RndStdIds nNew = aNewAnch.GetAnchorId();

    // Is the new anchor valid?
    if( !aNewAnch.GetContentAnchor() && (FLY_AT_FLY == nNew ||
        (FLY_AT_PARA == nNew) || (FLY_AS_CHAR == nNew) ||
        (FLY_AT_CHAR == nNew) ))
    {
        return IGNOREANCHOR;
    }

    if( nOld == nNew )
        return DONTMAKEFRMS;

    Point aOldAnchorPos( ::lcl_FindAnchorLayPos( *this, rOldAnch, &rFormat ));
    Point aNewAnchorPos( ::lcl_FindAnchorLayPos( *this, aNewAnch, 0 ));

    // Destroy the old Frames.
    // The Views are hidden implicitly, so hiding them another time would be
    // kind of a show!
    rFormat.DelFrms();

    if ( FLY_AS_CHAR == nOld )
    {
        // We need to handle InContents in a special way:
        // The TextAttribut needs to be destroyed which, unfortunately, also
        // destroys the format. To avoid that, we disconnect the format from
        // the attribute.
        const SwPosition *pPos = rOldAnch.GetContentAnchor();
        SwTextNode *pTextNode = pPos->nNode.GetNode().GetTextNode();
        OSL_ENSURE( pTextNode->HasHints(), "Missing FlyInCnt-Hint." );
        const sal_Int32 nIdx = pPos->nContent.GetIndex();
        SwTextAttr * const  pHint =
            pTextNode->GetTextAttrForCharAt( nIdx, RES_TXTATR_FLYCNT );
        OSL_ENSURE( pHint && pHint->Which() == RES_TXTATR_FLYCNT,
                    "Missing FlyInCnt-Hint." );
        OSL_ENSURE( pHint && pHint->GetFlyCnt().GetFrameFormat() == &rFormat,
                    "Wrong TextFlyCnt-Hint." );
        if (pHint)
            const_cast<SwFormatFlyCnt&>(pHint->GetFlyCnt()).SetFlyFormat();

        // They are disconnected. We now have to destroy the attribute.
        pTextNode->DeleteAttributes( RES_TXTATR_FLYCNT, nIdx, nIdx );
    }

    // We can finally set the attribute. It needs to be the first one!
    // Undo depends on it!
    rFormat.SetFormatAttr( aNewAnch );

    // Correct the position
    const SfxPoolItem* pItem;
    switch( nNew )
    {
    case FLY_AS_CHAR:
            // If no position attributes are received, we have to make sure
            // that no forbidden automatic alignment is left.
        {
            const SwPosition *pPos = aNewAnch.GetContentAnchor();
            SwTextNode *pNd = pPos->nNode.GetNode().GetTextNode();
            OSL_ENSURE( pNd, "Crsr does not point to TextNode." );

            SwFormatFlyCnt aFormat( static_cast<SwFlyFrameFormat*>(&rFormat) );
            pNd->InsertItem( aFormat, pPos->nContent.GetIndex(), 0 );
        }

        if( SfxItemState::SET != rSet.GetItemState( RES_VERT_ORIENT, false, &pItem ))
        {
            SwFormatVertOrient aOldV( rFormat.GetVertOrient() );
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
            if( SfxItemState::SET != rSet.GetItemState( RES_HORI_ORIENT, false, &pItem ))
                pItem = 0;

            SwFormatHoriOrient aOldH( rFormat.GetHoriOrient() );

            if( text::HoriOrientation::NONE == aOldH.GetHoriOrient() && ( !pItem ||
                aOldH.GetPos() == static_cast<const SwFormatHoriOrient*>(pItem)->GetPos() ))
            {
                SwTwips nPos = (FLY_AS_CHAR == nOld) ? 0 : aOldH.GetPos();
                nPos += aOldAnchorPos.getX() - aNewAnchorPos.getX();

                if( pItem )
                {
                    SwFormatHoriOrient* pH = const_cast<SwFormatHoriOrient*>(static_cast<const SwFormatHoriOrient*>(pItem));
                    aOldH.SetHoriOrient( pH->GetHoriOrient() );
                    aOldH.SetRelationOrient( pH->GetRelationOrient() );
                }
                aOldH.SetPos( nPos );
                rSet.Put( aOldH );
            }

            if( SfxItemState::SET != rSet.GetItemState( RES_VERT_ORIENT, false, &pItem ))
                pItem = 0;
            SwFormatVertOrient aOldV( rFormat.GetVertOrient() );

            // #i28922# - correction: compare <aOldV.GetVertOrient() with
            // <text::VertOrientation::NONE>
            if( text::VertOrientation::NONE == aOldV.GetVertOrient() && (!pItem ||
                aOldV.GetPos() == static_cast<const SwFormatVertOrient*>(pItem)->GetPos() ) )
            {
                SwTwips nPos = (FLY_AS_CHAR == nOld) ? 0 : aOldV.GetPos();
                nPos += aOldAnchorPos.getY() - aNewAnchorPos.getY();
                if( pItem )
                {
                    SwFormatVertOrient* pV = const_cast<SwFormatVertOrient*>(static_cast<const SwFormatVertOrient*>(pItem));
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
        rFormat.MakeFrms();

    return MAKEFRMS;
}

static bool
lcl_SetFlyFrmAttr(SwDoc & rDoc,
        sal_Int8 (SwDoc::*pSetFlyFrmAnchor)(SwFrameFormat &, SfxItemSet &, bool),
        SwFrameFormat & rFlyFormat, SfxItemSet & rSet)
{
    // #i32968# Inserting columns in the frame causes MakeFrameFormat to put two
    // objects of type SwUndoFrameFormat on the undo stack. We don't want them.
    ::sw::UndoGuard const undoGuard(rDoc.GetIDocumentUndoRedo());

    // Is the anchor attribute included?
    // If so, we pass it to a special method, which returns true
    // if the Fly needs to be created anew, because we e.g change the FlyType.
    sal_Int8 const nMakeFrms =
        (SfxItemState::SET == rSet.GetItemState( RES_ANCHOR, false ))
             ?  (rDoc.*pSetFlyFrmAnchor)( rFlyFormat, rSet, false )
             :  DONTMAKEFRMS;

    const SfxPoolItem* pItem;
    SfxItemIter aIter( rSet );
    SfxItemSet aTmpSet( rDoc.GetAttrPool(), aFrameFormatSetRange );
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
            if( !IsInvalidItem( aIter.GetCurItem() ) && ( SfxItemState::SET !=
                rFlyFormat.GetAttrSet().GetItemState( nWhich, true, &pItem ) ||
                *pItem != *aIter.GetCurItem() ))
                aTmpSet.Put( *aIter.GetCurItem() );
            break;
        }

        if( aIter.IsAtEnd() )
            break;

    } while( 0 != ( nWhich = aIter.NextItem()->Which() ) );

    if( aTmpSet.Count() )
        rFlyFormat.SetFormatAttr( aTmpSet );

    if( MAKEFRMS == nMakeFrms )
        rFlyFormat.MakeFrms();

    return aTmpSet.Count() || MAKEFRMS == nMakeFrms;
}

void SwDoc::CheckForUniqueItemForLineFillNameOrIndex(SfxItemSet& rSet)
{
    SwDrawModel* pDrawModel = getIDocumentDrawModelAccess().GetOrCreateDrawModel();
    SfxItemIter aIter(rSet);

    for(const SfxPoolItem* pItem = aIter.FirstItem(); pItem; pItem = aIter.NextItem())
    {
        if (IsInvalidItem(pItem))
            continue;
        const SfxPoolItem* pResult = NULL;

        switch(pItem->Which())
        {
            case XATTR_FILLBITMAP:
            {
                pResult = static_cast< const XFillBitmapItem* >(pItem)->checkForUniqueItem(pDrawModel);
                break;
            }
            case XATTR_LINEDASH:
            {
                pResult = static_cast< const XLineDashItem* >(pItem)->checkForUniqueItem(pDrawModel);
                break;
            }
            case XATTR_LINESTART:
            {
                pResult = static_cast< const XLineStartItem* >(pItem)->checkForUniqueItem(pDrawModel);
                break;
            }
            case XATTR_LINEEND:
            {
                pResult = static_cast< const XLineEndItem* >(pItem)->checkForUniqueItem(pDrawModel);
                break;
            }
            case XATTR_FILLGRADIENT:
            {
                pResult = static_cast< const XFillGradientItem* >(pItem)->checkForUniqueItem(pDrawModel);
                break;
            }
            case XATTR_FILLFLOATTRANSPARENCE:
            {
                pResult = static_cast< const XFillFloatTransparenceItem* >(pItem)->checkForUniqueItem(pDrawModel);
                break;
            }
            case XATTR_FILLHATCH:
            {
                pResult = static_cast< const XFillHatchItem* >(pItem)->checkForUniqueItem(pDrawModel);
                break;
            }
        }

        if(pResult)
        {
            rSet.Put(*pResult);
            delete pResult;
        }
    }
}

bool SwDoc::SetFlyFrmAttr( SwFrameFormat& rFlyFormat, SfxItemSet& rSet )
{
    if( !rSet.Count() )
        return false;

    boost::scoped_ptr<SwUndoFormatAttrHelper> pSaveUndo;

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().ClearRedo(); // AppendUndo far below, so leave it
        pSaveUndo.reset( new SwUndoFormatAttrHelper( rFlyFormat ) );
    }

    bool const bRet = lcl_SetFlyFrmAttr(*this, &SwDoc::SetFlyFrmAnchor, rFlyFormat, rSet);

    if ( pSaveUndo.get() )
    {
        if ( pSaveUndo->GetUndo() )
        {
            GetIDocumentUndoRedo().AppendUndo( pSaveUndo->ReleaseUndo() );
        }
    }

    getIDocumentState().SetModified();

    SwTextBoxHelper::syncFlyFrmAttr(rFlyFormat, rSet);

    return bRet;
}

// #i73249#
void SwDoc::SetFlyFrmTitle( SwFlyFrameFormat& rFlyFrameFormat,
                            const OUString& sNewTitle )
{
    if ( rFlyFrameFormat.GetObjTitle() == sNewTitle )
    {
        return;
    }

    ::sw::DrawUndoGuard const drawUndoGuard(GetIDocumentUndoRedo());

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().AppendUndo( new SwUndoFlyStrAttr( rFlyFrameFormat,
                                          UNDO_FLYFRMFMT_TITLE,
                                          rFlyFrameFormat.GetObjTitle(),
                                          sNewTitle ) );
    }

    rFlyFrameFormat.SetObjTitle( sNewTitle, true );

    getIDocumentState().SetModified();
}

void SwDoc::SetFlyFrmDescription( SwFlyFrameFormat& rFlyFrameFormat,
                                  const OUString& sNewDescription )
{
    if ( rFlyFrameFormat.GetObjDescription() == sNewDescription )
    {
        return;
    }

    ::sw::DrawUndoGuard const drawUndoGuard(GetIDocumentUndoRedo());

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().AppendUndo( new SwUndoFlyStrAttr( rFlyFrameFormat,
                                          UNDO_FLYFRMFMT_DESCRIPTION,
                                          rFlyFrameFormat.GetObjDescription(),
                                          sNewDescription ) );
    }

    rFlyFrameFormat.SetObjDescription( sNewDescription, true );

    getIDocumentState().SetModified();
}

bool SwDoc::SetFrameFormatToFly( SwFrameFormat& rFormat, SwFrameFormat& rNewFormat,
                            SfxItemSet* pSet, bool bKeepOrient )
{
    bool bChgAnchor = false, bFrmSz = false;

    const SwFormatFrmSize aFrmSz( rFormat.GetFrmSize() );
    const SwFormatVertOrient aVert( rFormat.GetVertOrient() );
    const SwFormatHoriOrient aHori( rFormat.GetHoriOrient() );

    SwUndoSetFlyFormat* pUndo = 0;
    bool const bUndo = GetIDocumentUndoRedo().DoesUndo();
    if (bUndo)
    {
        pUndo = new SwUndoSetFlyFormat( rFormat, rNewFormat );
        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }

    // #i32968# Inserting columns in the section causes MakeFrameFormat to put
    // 2 objects of type SwUndoFrameFormat on the undo stack. We don't want them.
    ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());

    // Set the column first, or we'll have trouble with
    //Set/Reset/Synch. and so on
    const SfxPoolItem* pItem;
    if( SfxItemState::SET != rNewFormat.GetAttrSet().GetItemState( RES_COL ))
        rFormat.ResetFormatAttr( RES_COL );

    if( rFormat.DerivedFrom() != &rNewFormat )
    {
        rFormat.SetDerivedFrom( &rNewFormat );

        // 1. If not automatic = ignore; else = dispose
        // 2. Dispose of it!
        if( SfxItemState::SET == rNewFormat.GetAttrSet().GetItemState( RES_FRM_SIZE, false ))
        {
            rFormat.ResetFormatAttr( RES_FRM_SIZE );
            bFrmSz = true;
        }

        const SfxItemSet* pAsk = pSet;
        if( !pAsk ) pAsk = &rNewFormat.GetAttrSet();
        if( SfxItemState::SET == pAsk->GetItemState( RES_ANCHOR, false, &pItem )
            && static_cast<const SwFormatAnchor*>(pItem)->GetAnchorId() !=
                rFormat.GetAnchor().GetAnchorId() )
        {
            if( pSet )
                bChgAnchor = MAKEFRMS == SetFlyFrmAnchor( rFormat, *pSet, false );
            else
            {
                // Needs to have the FlyFormat range, because we set attributes in it,
                // in SetFlyFrmAnchor.
                SfxItemSet aFlySet( *rNewFormat.GetAttrSet().GetPool(),
                                    rNewFormat.GetAttrSet().GetRanges() );
                aFlySet.Put( *pItem );
                bChgAnchor = MAKEFRMS == SetFlyFrmAnchor( rFormat, aFlySet, false);
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
        rFormat.ResetFormatAttr(RES_VERT_ORIENT);
        rFormat.ResetFormatAttr(RES_HORI_ORIENT);
    }

    rFormat.ResetFormatAttr( RES_PRINT, RES_SURROUND );
    rFormat.ResetFormatAttr( RES_LR_SPACE, RES_UL_SPACE );
    rFormat.ResetFormatAttr( RES_BACKGROUND, RES_COL );
    rFormat.ResetFormatAttr( RES_URL, RES_EDIT_IN_READONLY );

    if( !bFrmSz )
        rFormat.SetFormatAttr( aFrmSz );

    if( bChgAnchor )
        rFormat.MakeFrms();

    if( pUndo )
        pUndo->DeRegisterFromFormat( rFormat );

    getIDocumentState().SetModified();

    return bChgAnchor;
}

void SwDoc::GetGrfNms( const SwFlyFrameFormat& rFormat, OUString* pGrfName,
                       OUString* pFltName )
{
    SwNodeIndex aIdx( *rFormat.GetContent().GetContentIdx(), 1 );
    const SwGrfNode* pGrfNd = aIdx.GetNode().GetGrfNode();
    if( pGrfNd && pGrfNd->IsLinkedFile() )
        pGrfNd->GetFileFilterNms( pGrfName, pFltName );
}

bool SwDoc::ChgAnchor( const SdrMarkList& _rMrkList,
                           RndStdIds _eAnchorType,
                           const bool _bSameOnly,
                           const bool _bPosCorr )
{
    OSL_ENSURE( getIDocumentLayoutAccess().GetCurrentLayout(), "No layout!" );

    if ( !_rMrkList.GetMarkCount() ||
         _rMrkList.GetMark( 0 )->GetMarkedSdrObj()->GetUpGroup() )
    {
        return false;
    }

    GetIDocumentUndoRedo().StartUndo( UNDO_INSATTR, NULL );

    bool bUnmark = false;
    for ( size_t i = 0; i < _rMrkList.GetMarkCount(); ++i )
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
            boost::scoped_ptr<const SwPosition> xOldAsCharAnchorPos;
            const RndStdIds eOldAnchorType = pContact->GetAnchorId();
            if ( !_bSameOnly && eOldAnchorType == FLY_AS_CHAR )
            {
                xOldAsCharAnchorPos.reset(new SwPosition(pContact->GetContentAnchor()));
            }

            if ( _bSameOnly )
                _eAnchorType = eOldAnchorType;

            SwFormatAnchor aNewAnch( _eAnchorType );
            SwAnchoredObject *pAnchoredObj = pContact->GetAnchoredObj(pObj);
            Rectangle aObjRect(pAnchoredObj->GetObjRect().SVRect());
            const Point aPt( aObjRect.TopLeft() );

            switch ( _eAnchorType )
            {
            case FLY_AT_PARA:
            case FLY_AT_CHAR:
                {
                    const Point aNewPoint = ( pOldAnchorFrm->IsVertical() ||
                                              pOldAnchorFrm->IsRightToLeft() )
                                            ? aObjRect.TopRight()
                                            : aPt;

                    // allow drawing objects in header/footer
                    pNewAnchorFrm = ::FindAnchor( pOldAnchorFrm, aNewPoint, false );
                    if ( pNewAnchorFrm->IsTextFrm() && static_cast<const SwTextFrm*>(pNewAnchorFrm)->IsFollow() )
                    {
                        pNewAnchorFrm = static_cast<const SwTextFrm*>(pNewAnchorFrm)->FindMaster();
                    }
                    if ( pNewAnchorFrm->IsProtected() )
                    {
                        pNewAnchorFrm = 0;
                    }
                    else
                    {
                        SwPosition aPos( *static_cast<const SwContentFrm*>(pNewAnchorFrm)->GetNode() );
                        aNewAnch.SetType( _eAnchorType );
                        aNewAnch.SetAnchor( &aPos );
                    }
                }
                break;

            case FLY_AT_FLY: // LAYER_IMPL
                {
                    // Search the closest SwFlyFrm starting from the upper left corner.
                    SwFrm *pTextFrm;
                    {
                        SwCrsrMoveState aState( MV_SETONLYTEXT );
                        SwPosition aPos( GetNodes() );
                        Point aPoint( aPt );
                        aPoint.setX(aPoint.getX() - 1);
                        getIDocumentLayoutAccess().GetCurrentLayout()->GetCrsrOfst( &aPos, aPoint, &aState );
                        // consider that drawing objects can be in
                        // header/footer. Thus, <GetFrm()> by left-top-corner
                        pTextFrm = aPos.nNode.GetNode().
                                        GetContentNode()->getLayoutFrm( getIDocumentLayoutAccess().GetCurrentLayout(), &aPt, 0, false );
                    }
                    const SwFrm *pTmp = ::FindAnchor( pTextFrm, aPt );
                    pNewAnchorFrm = pTmp->FindFlyFrm();
                    if( pNewAnchorFrm && !pNewAnchorFrm->IsProtected() )
                    {
                        const SwFrameFormat *pTmpFormat = static_cast<const SwFlyFrm*>(pNewAnchorFrm)->GetFormat();
                        const SwFormatContent& rContent = pTmpFormat->GetContent();
                        SwPosition aPos( *rContent.GetContentIdx() );
                        aNewAnch.SetAnchor( &aPos );
                        break;
                    }

                    aNewAnch.SetType( FLY_AT_PAGE );
                    // no break
                }
            case FLY_AT_PAGE:
                {
                    pNewAnchorFrm = getIDocumentLayoutAccess().GetCurrentLayout()->Lower();
                    while ( pNewAnchorFrm && !pNewAnchorFrm->Frm().IsInside( aPt ) )
                        pNewAnchorFrm = pNewAnchorFrm->GetNext();
                    if ( !pNewAnchorFrm )
                        continue;

                    aNewAnch.SetPageNum( static_cast<const SwPageFrm*>(pNewAnchorFrm)->GetPhyPageNum());
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
                    const_cast<SwTextFrm*>(static_cast<const SwTextFrm*>(pOldAnchorFrm))->Prepare();
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
                    aPoint.setX(aPoint.getX() - 1);    // Do not load in the DrawObj!
                    aNewAnch.SetType( FLY_AS_CHAR );
                    SwPosition aPos( *static_cast<const SwContentFrm*>(pNewAnchorFrm)->GetNode() );
                    if ( pNewAnchorFrm->Frm().IsInside( aPoint ) )
                    {
                    // We need to find a TextNode, because only there we can anchor a
                    // content-bound DrawObject.
                        SwCrsrMoveState aState( MV_SETONLYTEXT );
                        getIDocumentLayoutAccess().GetCurrentLayout()->GetCrsrOfst( &aPos, aPoint, &aState );
                    }
                    else
                    {
                        SwContentNode &rCNd = (SwContentNode&)
                            *static_cast<const SwContentFrm*>(pNewAnchorFrm)->GetNode();
                        if ( pNewAnchorFrm->Frm().Bottom() < aPt.Y() )
                            rCNd.MakeStartIndex( &aPos.nContent );
                        else
                            rCNd.MakeEndIndex( &aPos.nContent );
                    }
                    aNewAnch.SetAnchor( &aPos );
                    SetAttr( aNewAnch, *pContact->GetFormat() );
                    // #i26791# - adjust vertical positioning to 'center to
                    // baseline'
                    SetAttr( SwFormatVertOrient( 0, text::VertOrientation::CENTER, text::RelOrientation::FRAME ), *pContact->GetFormat() );
                    SwTextNode *pNd = aPos.nNode.GetNode().GetTextNode();
                    OSL_ENSURE( pNd, "Cursor not positioned at TextNode." );

                    SwFormatFlyCnt aFormat( pContact->GetFormat() );
                    pNd->InsertItem( aFormat, aPos.nContent.GetIndex(), 0 );
                }
                break;
            default:
                OSL_ENSURE( false, "unexpected AnchorId." );
            }

            if ( (FLY_AS_CHAR != _eAnchorType) &&
                 pNewAnchorFrm &&
                 ( !_bSameOnly || pNewAnchorFrm != pOldAnchorFrm ) )
            {
                // #i26791# - Direct object positioning no longer needed. Apply
                // of attributes (method call <SetAttr(..)>) takes care of the
                // invalidation of the object position.
                SetAttr( aNewAnch, *pContact->GetFormat() );
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

            // we have changed the anchoring attributes, and those are used to
            // order the object in its sorted list, so update its position
            pAnchoredObj->UpdateObjInSortedList();

            // #i54336#
            if (xOldAsCharAnchorPos)
            {
                if ( pNewAnchorFrm)
                {
                    // We need to handle InContents in a special way:
                    // The TextAttribut needs to be destroyed which, unfortunately, also
                    // destroys the format. To avoid that, we disconnect the format from
                    // the attribute.
                    const sal_Int32 nIndx( xOldAsCharAnchorPos->nContent.GetIndex() );
                    SwTextNode* pTextNode( xOldAsCharAnchorPos->nNode.GetNode().GetTextNode() );
                    assert(pTextNode && "<SwDoc::ChgAnchor(..)> - missing previous anchor text node for as-character anchored object");
                    SwTextAttr * const pHint =
                        pTextNode->GetTextAttrForCharAt( nIndx, RES_TXTATR_FLYCNT );
                    assert(pHint && "Missing FlyInCnt-Hint.");
                    const_cast<SwFormatFlyCnt&>(pHint->GetFlyCnt()).SetFlyFormat();

                    // They are disconnected. We now have to destroy the attribute.
                    pTextNode->DeleteAttributes( RES_TXTATR_FLYCNT, nIndx, nIndx );
                }
            }
        }
    }

    GetIDocumentUndoRedo().EndUndo( UNDO_END, NULL );
    getIDocumentState().SetModified();

    return bUnmark;
}

SwChainRet SwDoc::Chainable( const SwFrameFormat &rSource, const SwFrameFormat &rDest )
{
    // The Source must not yet have a Follow.
    const SwFormatChain &rOldChain = rSource.GetChain();
    if ( rOldChain.GetNext() )
        return SwChainRet::SOURCE_CHAINED;

    // Target must not be equal to Source and we also must not have a closed chain.
    const SwFrameFormat *pFormat = &rDest;
    do {
        if( pFormat == &rSource )
            return SwChainRet::SELF;
        pFormat = pFormat->GetChain().GetNext();
    } while ( pFormat );

    // There must not be a chaining from outside to inside or the other way around.
    if( rDest.IsLowerOf( rSource ) || rSource .IsLowerOf( rDest ) )
        return SwChainRet::SELF;

    // The Target must not yet have a Master.
    const SwFormatChain &rChain = rDest.GetChain();
    if( rChain.GetPrev() )
        return SwChainRet::IS_IN_CHAIN;

    // Target must be empty.
    const SwNodeIndex* pCntIdx = rDest.GetContent().GetContentIdx();
    if( !pCntIdx )
        return SwChainRet::NOT_FOUND;

    SwNodeIndex aNxtIdx( *pCntIdx, 1 );
    const SwTextNode* pTextNd = aNxtIdx.GetNode().GetTextNode();
    if( !pTextNd )
        return SwChainRet::NOT_FOUND;

    const sal_uLong nFlySttNd = pCntIdx->GetIndex();
    if( 2 != ( pCntIdx->GetNode().EndOfSectionIndex() - nFlySttNd ) ||
        pTextNd->GetText().getLength() )
    {
        return SwChainRet::NOT_EMPTY;
    }

    for( auto pSpzFrmFm : *GetSpzFrameFormats() )
    {
        const SwFormatAnchor& rAnchor = pSpzFrmFm->GetAnchor();
        sal_uLong nTstSttNd;
        // #i20622# - to-frame anchored objects are allowed.
        if ( ((rAnchor.GetAnchorId() == FLY_AT_PARA) ||
              (rAnchor.GetAnchorId() == FLY_AT_CHAR)) &&
             0 != rAnchor.GetContentAnchor() &&
             nFlySttNd <= ( nTstSttNd =
                         rAnchor.GetContentAnchor()->nNode.GetIndex() ) &&
             nTstSttNd < nFlySttNd + 2 )
        {
            return SwChainRet::NOT_EMPTY;
        }
    }

    // We also need to consider the right area.
    // Both Flys need to be located in the same area (Body, Header/Footer, Fly).
    // If the Source is not the selected frame, it's enough to find a suitable
    // one. e.g. if it's requested by the API.

    // both in the same fly, header, footer or on the page?
    const SwFormatAnchor &rSrcAnchor = rSource.GetAnchor(),
                      &rDstAnchor = rDest.GetAnchor();
    sal_uLong nEndOfExtras = GetNodes().GetEndOfExtras().GetIndex();
    bool bAllowed = false;
    if ( FLY_AT_PAGE == rSrcAnchor.GetAnchorId() )
    {
        if ( (FLY_AT_PAGE == rDstAnchor.GetAnchorId()) ||
            ( rDstAnchor.GetContentAnchor() &&
              rDstAnchor.GetContentAnchor()->nNode.GetIndex() > nEndOfExtras ))
            bAllowed = true;
    }
    else if( rSrcAnchor.GetContentAnchor() && rDstAnchor.GetContentAnchor() )
    {
        const SwNodeIndex &rSrcIdx = rSrcAnchor.GetContentAnchor()->nNode,
                            &rDstIdx = rDstAnchor.GetContentAnchor()->nNode;
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
            bAllowed = true;
    }

    return bAllowed ? SwChainRet::OK : SwChainRet::WRONG_AREA;
}

SwChainRet SwDoc::Chain( SwFrameFormat &rSource, const SwFrameFormat &rDest )
{
    SwChainRet nErr = Chainable( rSource, rDest );
    if ( nErr == SwChainRet::OK )
    {
        GetIDocumentUndoRedo().StartUndo( UNDO_CHAINE, NULL );

        SwFlyFrameFormat& rDestFormat = const_cast<SwFlyFrameFormat&>(static_cast<const SwFlyFrameFormat&>(rDest));

        // Attach Follow to the Master.
        SwFormatChain aChain = rDestFormat.GetChain();
        aChain.SetPrev( &static_cast<SwFlyFrameFormat&>(rSource) );
        SetAttr( aChain, rDestFormat );

        SfxItemSet aSet( GetAttrPool(), RES_FRM_SIZE, RES_FRM_SIZE,
                                        RES_CHAIN,  RES_CHAIN, 0 );

        // Attach Follow to the Master.
        aChain.SetPrev( &static_cast<SwFlyFrameFormat&>(rSource) );
        SetAttr( aChain, rDestFormat );

        // Attach Master to the Follow.
        // Make sure that the Master has a fixed height.
        aChain = rSource.GetChain();
        aChain.SetNext( &rDestFormat );
        aSet.Put( aChain );

        SwFormatFrmSize aSize( rSource.GetFrmSize() );
        if ( aSize.GetHeightSizeType() != ATT_FIX_SIZE )
        {
            SwFlyFrm *pFly = SwIterator<SwFlyFrm,SwFormat>( rSource ).First();
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

void SwDoc::Unchain( SwFrameFormat &rFormat )
{
    SwFormatChain aChain( rFormat.GetChain() );
    if ( aChain.GetNext() )
    {
        GetIDocumentUndoRedo().StartUndo( UNDO_UNCHAIN, NULL );
        SwFrameFormat *pFollow = aChain.GetNext();
        aChain.SetNext( 0 );
        SetAttr( aChain, rFormat );
        aChain = pFollow->GetChain();
        aChain.SetPrev( 0 );
        SetAttr( aChain, *pFollow );
        GetIDocumentUndoRedo().EndUndo( UNDO_UNCHAIN, NULL );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
