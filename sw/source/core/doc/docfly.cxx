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
#include <svx/svdmark.hxx>
#include <osl/diagnose.h>
#include <fmtfsize.hxx>
#include <fmtornt.hxx>
#include <dcontact.hxx>
#include <ndgrf.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentState.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <ndindex.hxx>
#include <drawdoc.hxx>
#include <fmtcntnt.hxx>
#include <fmtanchr.hxx>
#include <fmtflcnt.hxx>
#include <txtfrm.hxx>
#include <notxtfrm.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <flyfrm.hxx>
#include <textboxhelper.hxx>
#include <txatbase.hxx>
#include <frmfmt.hxx>
#include <ndtxt.hxx>
#include <pam.hxx>
#include <swundo.hxx>
#include <crstate.hxx>
#include <UndoCore.hxx>
#include <UndoAttribute.hxx>
#include <fmtcnct.hxx>
#include <dflyobj.hxx>
#include <undoflystrattr.hxx>
#include <calbck.hxx>
#include <frameformats.hxx>
#include <memory>
#include <svx/xbtmpit.hxx>
#include <svx/xflftrit.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xflhtit.hxx>
#include <formatflysplit.hxx>

using namespace ::com::sun::star;

size_t SwDoc::GetFlyCount( FlyCntType eType, bool bIgnoreTextBoxes ) const
{
    size_t nCount = 0;
    const SwNodeIndex* pIdx;

    for(sw::SpzFrameFormat* pFlyFormat: *GetSpzFrameFormats())
    {
        if (bIgnoreTextBoxes && SwTextBoxHelper::isTextBox(pFlyFormat, RES_FLYFRMFMT))
            continue;

        if( RES_FLYFRMFMT != pFlyFormat->Which() )
            continue;
        pIdx = pFlyFormat->GetContent().GetContentIdx();
        if( pIdx && pIdx->GetNodes().IsDocNodes() )
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
    SwFrameFormat* pRetFormat = nullptr;
    const SwNodeIndex* pIdx;
    size_t nCount = 0;

    for(sw::SpzFrameFormat* pFlyFormat: *GetSpzFrameFormats())
    {
        if (bIgnoreTextBoxes && SwTextBoxHelper::isTextBox(pFlyFormat, RES_FLYFRMFMT))
            continue;

        if( RES_FLYFRMFMT != pFlyFormat->Which() )
            continue;
        pIdx = pFlyFormat->GetContent().GetContentIdx();
        if( pIdx && pIdx->GetNodes().IsDocNodes() )
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

SwFrameFormat* SwDoc::GetFlyFrameFormatByName( const OUString& rFrameFormatName )
{
    auto pFrameFormats = GetSpzFrameFormats();
    auto it = pFrameFormats->findByTypeAndName( RES_FLYFRMFMT, rFrameFormatName );
    auto endIt = pFrameFormats->typeAndNameEnd();
    for ( ; it != endIt; ++it)
    {
        sw::SpzFrameFormat* pFlyFormat = *it;
        const SwNodeIndex* pIdx = pFlyFormat->GetContent().GetContentIdx();
        if( !pIdx || !pIdx->GetNodes().IsDocNodes() )
            continue;

        const SwNode* pNd = GetNodes()[ pIdx->GetIndex() + 1 ];
        if( !pNd->IsNoTextNode())
            return pFlyFormat;
    }
    return nullptr;
}

std::vector<SwFrameFormat const*> SwDoc::GetFlyFrameFormats(
    FlyCntType const eType, bool const bIgnoreTextBoxes)
{
    std::vector<SwFrameFormat const*> ret;
    ret.reserve(GetSpzFrameFormats()->size());

    for(sw::SpzFrameFormat* pFlyFormat: *GetSpzFrameFormats())
    {
        if (bIgnoreTextBoxes && SwTextBoxHelper::isTextBox(pFlyFormat, RES_FLYFRMFMT))
        {
            continue;
        }

        if (RES_FLYFRMFMT != pFlyFormat->Which())
        {
            continue;
        }

        SwNodeIndex const*const pIdx(pFlyFormat->GetContent().GetContentIdx());
        if (pIdx && pIdx->GetNodes().IsDocNodes())
        {
            SwNode const*const pNd = GetNodes()[ pIdx->GetIndex() + 1 ];
            switch (eType)
            {
            case FLYCNTTYPE_FRM:
                if (!pNd->IsNoTextNode())
                    ret.push_back(pFlyFormat);
                break;
            case FLYCNTTYPE_GRF:
                if (pNd->IsGrfNode())
                    ret.push_back(pFlyFormat);
                break;
            case FLYCNTTYPE_OLE:
                if (pNd->IsOLENode())
                    ret.push_back(pFlyFormat);
                break;
            default:
                ret.push_back(pFlyFormat);
            }
        }
    }

    return ret;
}

static Point lcl_FindAnchorLayPos( SwDoc& rDoc, const SwFormatAnchor& rAnch,
                            const SwFrameFormat* pFlyFormat )
{
    Point aRet;
    if( rDoc.getIDocumentLayoutAccess().GetCurrentViewShell() )
        switch( rAnch.GetAnchorId() )
        {
        case RndStdIds::FLY_AS_CHAR:
            if( pFlyFormat && rAnch.GetAnchorNode() )
            {
                const SwFrame* pOld = static_cast<const SwFlyFrameFormat*>(pFlyFormat)->GetFrame( &aRet );
                if( pOld )
                    aRet = pOld->getFrameArea().Pos();
            }
            break;

        case RndStdIds::FLY_AT_PARA:
        case RndStdIds::FLY_AT_CHAR: // LAYER_IMPL
            if( rAnch.GetAnchorNode() )
            {
                const SwContentNode* pNd = rAnch.GetAnchorNode()->GetContentNode();
                std::pair<Point, bool> const tmp(aRet, false);
                const SwFrame* pOld = pNd ? pNd->getLayoutFrame(rDoc.getIDocumentLayoutAccess().GetCurrentLayout(), nullptr, &tmp) : nullptr;
                if( pOld )
                    aRet = pOld->getFrameArea().Pos();
            }
            break;

        case RndStdIds::FLY_AT_FLY: // LAYER_IMPL
            if( rAnch.GetAnchorNode() )
            {
                const SwFlyFrameFormat* pFormat = static_cast<SwFlyFrameFormat*>(rAnch.GetAnchorNode()->
                                                GetFlyFormat());
                const SwFrame* pOld = pFormat ? pFormat->GetFrame( &aRet ) : nullptr;
                if( pOld )
                    aRet = pOld->getFrameArea().Pos();
            }
            break;

        case RndStdIds::FLY_AT_PAGE:
            {
                sal_uInt16 nPgNum = rAnch.GetPageNum();
                const SwPageFrame *pPage = static_cast<SwPageFrame*>(rDoc.getIDocumentLayoutAccess().GetCurrentLayout()->Lower());
                for( sal_uInt16 i = 1; (i <= nPgNum) && pPage; ++i,
                                    pPage =static_cast<const SwPageFrame*>(pPage->GetNext()) )
                    if( i == nPgNum )
                    {
                        aRet = pPage->getFrameArea().Pos();
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

sal_Int8 SwDoc::SetFlyFrameAnchor( SwFrameFormat& rFormat, SfxItemSet& rSet, bool bNewFrames )
{
    // Changing anchors is almost always allowed.
    // Exception: Paragraph and character bound frames must not become
    // page bound, if they are located in the header or footer.
    const SwFormatAnchor &rOldAnch = rFormat.GetAnchor();
    const RndStdIds nOld = rOldAnch.GetAnchorId();

    SwFormatAnchor aNewAnch( rSet.Get( RES_ANCHOR ) );
    RndStdIds nNew = aNewAnch.GetAnchorId();

    // Is the new anchor valid?
    if( !aNewAnch.GetAnchorNode() && (RndStdIds::FLY_AT_FLY == nNew ||
        (RndStdIds::FLY_AT_PARA == nNew) || (RndStdIds::FLY_AS_CHAR == nNew) ||
        (RndStdIds::FLY_AT_CHAR == nNew) ))
    {
        return IGNOREANCHOR;
    }

    if( nOld == nNew )
        return DONTMAKEFRMS;

    Point aOldAnchorPos( ::lcl_FindAnchorLayPos( *this, rOldAnch, &rFormat ));
    Point aNewAnchorPos( ::lcl_FindAnchorLayPos( *this, aNewAnch, nullptr ));

    // Destroy the old Frames.
    // The Views are hidden implicitly, so hiding them another time would be
    // kind of a show!
    rFormat.DelFrames();

    if ( RndStdIds::FLY_AS_CHAR == nOld )
    {
        // We need to handle InContents in a special way:
        // The TextAttribute needs to be destroyed which, unfortunately, also
        // destroys the format. To avoid that, we disconnect the format from
        // the attribute.
        SwNode *pAnchorNode = rOldAnch.GetAnchorNode();
        SwTextNode *pTextNode = pAnchorNode->GetTextNode();
        OSL_ENSURE( pTextNode->HasHints(), "Missing FlyInCnt-Hint." );
        const sal_Int32 nIdx = rOldAnch.GetAnchorContentOffset();
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
    switch( nNew )
    {
    case RndStdIds::FLY_AS_CHAR:
            // If no position attributes are received, we have to make sure
            // that no forbidden automatic alignment is left.
        {
            SwNode *pAnchorNode = aNewAnch.GetAnchorNode();
            SwTextNode *pNd = pAnchorNode->GetTextNode();
            OSL_ENSURE( pNd, "Cursor does not point to TextNode." );

            SwFormatFlyCnt aFormat( static_cast<SwFlyFrameFormat*>(&rFormat) );
            pNd->InsertItem( aFormat, aNewAnch.GetAnchorContentOffset(), 0 );
        }

        if( SfxItemState::SET != rSet.GetItemState( RES_VERT_ORIENT, false ))
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

    case RndStdIds::FLY_AT_PARA:
    case RndStdIds::FLY_AT_CHAR:   // LAYER_IMPL
    case RndStdIds::FLY_AT_FLY:    // LAYER_IMPL
    case RndStdIds::FLY_AT_PAGE:
        {
            // If only the anchor type has changed (char -> para -> page) and the absolute position
            // is unchanged even though there is a new relative orientation
            // (likely because the old orientation was not valid for the new anchor type),
            // then adjust the position to account for the moved anchor position.
            const SwFormatHoriOrient* pHoriOrientItem = rSet.GetItemIfSet( RES_HORI_ORIENT, false );

            SwFormatHoriOrient aOldH( rFormat.GetHoriOrient() );
            bool bPutOldH(false);

            if (text::HoriOrientation::NONE == aOldH.GetHoriOrient() && pHoriOrientItem
                && text::HoriOrientation::NONE == pHoriOrientItem->GetHoriOrient()
                && aOldH.GetPos() == pHoriOrientItem->GetPos())
            {
                SwTwips nPos = (RndStdIds::FLY_AS_CHAR == nOld) ? 0 : aOldH.GetPos();
                nPos += aOldAnchorPos.getX() - aNewAnchorPos.getX();

                assert(aOldH.GetRelationOrient() != pHoriOrientItem->GetRelationOrient());
                aOldH.SetRelationOrient(pHoriOrientItem->GetRelationOrient());

                aOldH.SetPos( nPos );
                bPutOldH = true;
            }
            if (nNew == RndStdIds::FLY_AT_PAGE)
            {
                sal_Int16 nRelOrient(pHoriOrientItem
                    ? pHoriOrientItem->GetRelationOrient()
                    : aOldH.GetRelationOrient());
                if (sw::GetAtPageRelOrientation(nRelOrient, false))
                {
                    SAL_INFO("sw.ui", "fixing horizontal RelOrientation for at-page anchor");
                    aOldH.SetRelationOrient(nRelOrient);
                    bPutOldH = true;
                }
            }
            if (bPutOldH)
            {
                rSet.Put( aOldH );
            }

            const SwFormatVertOrient* pVertOrientItem = rSet.GetItemIfSet( RES_VERT_ORIENT, false );
            SwFormatVertOrient aOldV( rFormat.GetVertOrient() );

            if (text::VertOrientation::NONE == aOldV.GetVertOrient() && pVertOrientItem
                && text::VertOrientation::NONE == pVertOrientItem->GetVertOrient()
                && aOldV.GetPos() == pVertOrientItem->GetPos())
            {
                SwTwips nPos = (RndStdIds::FLY_AS_CHAR == nOld) ? 0 : aOldV.GetPos();
                nPos += aOldAnchorPos.getY() - aNewAnchorPos.getY();

                assert(aOldV.GetRelationOrient() != pVertOrientItem->GetRelationOrient());
                aOldV.SetRelationOrient(pVertOrientItem->GetRelationOrient());

                aOldV.SetPos( nPos );
                rSet.Put( aOldV );
            }
        }
        break;
    default:
        break;
    }

    if( bNewFrames )
        rFormat.MakeFrames();

    return MAKEFRMS;
}

static bool
lcl_SetFlyFrameAttr(SwDoc & rDoc,
        sal_Int8 (SwDoc::*pSetFlyFrameAnchor)(SwFrameFormat &, SfxItemSet &, bool),
        SwFrameFormat & rFlyFormat, SfxItemSet & rSet)
{
    // #i32968# Inserting columns in the frame causes MakeFrameFormat to put two
    // objects of type SwUndoFrameFormat on the undo stack. We don't want them.
    ::sw::UndoGuard const undoGuard(rDoc.GetIDocumentUndoRedo());

    // Is the anchor attribute included?
    // If so, we pass it to a special method, which returns true
    // if the Fly needs to be created anew, because we e.g change the FlyType.
    sal_Int8 const nMakeFrames =
        (SfxItemState::SET == rSet.GetItemState( RES_ANCHOR, false ))
             ?  (rDoc.*pSetFlyFrameAnchor)( rFlyFormat, rSet, false )
             :  DONTMAKEFRMS;

    // ITEM: SfxItemIter and removing SfxPoolItems:
    std::vector<sal_uInt16> aDeleteWhichIDs;
    SfxItemSet aTmpSet(rDoc.GetAttrPool(), aFrameFormatSetRange);

    for (SfxItemIter aIter(rSet); !aIter.IsAtEnd() && 0 != aIter.GetCurWhich(); aIter.NextItem())
    {
        switch(aIter.GetCurWhich())
        {
        case RES_FILL_ORDER:
        case RES_BREAK:
        case RES_PAGEDESC:
        case RES_CNTNT:
        case RES_FOOTER:
            OSL_FAIL( "Unknown Fly attribute." );
            [[fallthrough]];
        case RES_CHAIN:
            aDeleteWhichIDs.push_back(aIter.GetCurWhich());
            break;
        case RES_ANCHOR:
            if( DONTMAKEFRMS != nMakeFrames )
                break;
            [[fallthrough]];
        default:
            {
                const SfxPoolItem* pGet(nullptr);
                if (!IsInvalidItem(aIter.GetCurItem()) && (SfxItemState::SET !=
                    rFlyFormat.GetAttrSet().GetItemState(aIter.GetCurWhich(), true, &pGet ) ||
                    *pGet != *aIter.GetCurItem()))
                    aTmpSet.Put(*aIter.GetCurItem());
                break;
            }
        }
    }

    for (auto nDelWhich : aDeleteWhichIDs)
        rSet.ClearItem(nDelWhich);

    if( aTmpSet.Count() )
        rFlyFormat.SetFormatAttr( aTmpSet );

    if( MAKEFRMS == nMakeFrames )
        rFlyFormat.MakeFrames();

    return aTmpSet.Count() || MAKEFRMS == nMakeFrames;
}

void SwDoc::CheckForUniqueItemForLineFillNameOrIndex(SfxItemSet& rSet)
{
    SwDrawModel* pDrawModel = getIDocumentDrawModelAccess().GetOrCreateDrawModel();
    SfxItemIter aIter(rSet);

    for (const SfxPoolItem* pItem = aIter.GetCurItem(); pItem; pItem = aIter.NextItem())
    {
        if (IsInvalidItem(pItem))
            continue;
        std::unique_ptr<SfxPoolItem> pResult;

        switch(pItem->Which())
        {
            case XATTR_FILLBITMAP:
            {
                pResult = pItem->StaticWhichCast(XATTR_FILLBITMAP).checkForUniqueItem(pDrawModel);
                break;
            }
            case XATTR_LINEDASH:
            {
                pResult = pItem->StaticWhichCast(XATTR_LINEDASH).checkForUniqueItem(pDrawModel);
                break;
            }
            case XATTR_LINESTART:
            {
                pResult = pItem->StaticWhichCast(XATTR_LINESTART).checkForUniqueItem(pDrawModel);
                break;
            }
            case XATTR_LINEEND:
            {
                pResult = pItem->StaticWhichCast(XATTR_LINEEND).checkForUniqueItem(pDrawModel);
                break;
            }
            case XATTR_FILLGRADIENT:
            {
                pResult = pItem->StaticWhichCast(XATTR_FILLGRADIENT).checkForUniqueItem(pDrawModel);
                break;
            }
            case XATTR_FILLFLOATTRANSPARENCE:
            {
                pResult = pItem->StaticWhichCast(XATTR_FILLFLOATTRANSPARENCE).checkForUniqueItem(pDrawModel);
                break;
            }
            case XATTR_FILLHATCH:
            {
                pResult = pItem->StaticWhichCast(XATTR_FILLHATCH).checkForUniqueItem(pDrawModel);
                break;
            }
        }

        if(pResult)
        {
            rSet.Put(std::move(pResult));
        }
    }
}

bool SwDoc::SetFlyFrameAttr( SwFrameFormat& rFlyFormat, SfxItemSet& rSet )
{
    if( !rSet.Count() )
        return false;

    SwDocModifyAndUndoGuard guard(rFlyFormat);

    bool const bRet = lcl_SetFlyFrameAttr(*this, &SwDoc::SetFlyFrameAnchor, rFlyFormat, rSet);

    //SwTextBoxHelper::syncFlyFrameAttr(rFlyFormat, rSet);

    return bRet;
}

// #i73249#
void SwDoc::SetFlyFrameTitle( SwFlyFrameFormat& rFlyFrameFormat,
                            const OUString& sNewTitle )
{
    if ( rFlyFrameFormat.GetObjTitle() == sNewTitle )
    {
        return;
    }

    ::sw::DrawUndoGuard const drawUndoGuard(GetIDocumentUndoRedo());

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().AppendUndo( std::make_unique<SwUndoFlyStrAttr>( rFlyFrameFormat,
                                          SwUndoId::FLYFRMFMT_TITLE,
                                          rFlyFrameFormat.GetObjTitle(),
                                          sNewTitle ) );
    }

    rFlyFrameFormat.SetObjTitle( sNewTitle, true );

    getIDocumentState().SetModified();
}

void SwDoc::SetFlyFrameDescription( SwFlyFrameFormat& rFlyFrameFormat,
                                  const OUString& sNewDescription )
{
    if ( rFlyFrameFormat.GetObjDescription() == sNewDescription )
    {
        return;
    }

    ::sw::DrawUndoGuard const drawUndoGuard(GetIDocumentUndoRedo());

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().AppendUndo( std::make_unique<SwUndoFlyStrAttr>( rFlyFrameFormat,
                                          SwUndoId::FLYFRMFMT_DESCRIPTION,
                                          rFlyFrameFormat.GetObjDescription(),
                                          sNewDescription ) );
    }

    rFlyFrameFormat.SetObjDescription( sNewDescription, true );

    getIDocumentState().SetModified();
}

void SwDoc::SetFlyFrameDecorative(SwFlyFrameFormat& rFlyFrameFormat,
                                  bool const isDecorative)
{
    if (rFlyFrameFormat.GetAttrSet().Get(RES_DECORATIVE).GetValue() == isDecorative)
    {
        return;
    }

    ::sw::DrawUndoGuard const drawUndoGuard(GetIDocumentUndoRedo());

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().AppendUndo(
            std::make_unique<SwUndoFlyDecorative>(rFlyFrameFormat, isDecorative));
    }

    rFlyFrameFormat.SetObjDecorative(isDecorative);

    getIDocumentState().SetModified();
}


bool SwDoc::SetFrameFormatToFly( SwFrameFormat& rFormat, SwFrameFormat& rNewFormat,
                            SfxItemSet* pSet, bool bKeepOrient )
{
    bool bChgAnchor = false, bFrameSz = false;

    const SwFormatFrameSize aFrameSz( rFormat.GetFrameSize() );

    SwUndoSetFlyFormat* pUndo = nullptr;
    bool const bUndo = GetIDocumentUndoRedo().DoesUndo();
    if (bUndo)
    {
        pUndo = new SwUndoSetFlyFormat( rFormat, rNewFormat );
        GetIDocumentUndoRedo().AppendUndo(std::unique_ptr<SwUndo>(pUndo));
    }

    // #i32968# Inserting columns in the section causes MakeFrameFormat to put
    // 2 objects of type SwUndoFrameFormat on the undo stack. We don't want them.
    ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());

    // Set the column first, or we'll have trouble with
    //Set/Reset/Synch. and so on
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
            bFrameSz = true;
        }

        const SfxItemSet* pAsk = pSet;
        if( !pAsk ) pAsk = &rNewFormat.GetAttrSet();
        const SwFormatAnchor* pFormatAnchor = pAsk->GetItemIfSet( RES_ANCHOR, false );
        if( pFormatAnchor
            && pFormatAnchor->GetAnchorId() !=
                rFormat.GetAnchor().GetAnchorId() )
        {
            if( pSet )
                bChgAnchor = MAKEFRMS == SetFlyFrameAnchor( rFormat, *pSet, false );
            else
            {
                // Needs to have the FlyFormat range, because we set attributes in it,
                // in SetFlyFrameAnchor.
                SfxItemSet aFlySet( *rNewFormat.GetAttrSet().GetPool(),
                                    rNewFormat.GetAttrSet().GetRanges() );
                aFlySet.Put( *pFormatAnchor );
                bChgAnchor = MAKEFRMS == SetFlyFrameAnchor( rFormat, aFlySet, false);
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
    rFormat.ResetFormatAttr( RES_EDIT_IN_READONLY );

    if( !bFrameSz )
        rFormat.SetFormatAttr( aFrameSz );

    if( bChgAnchor )
        rFormat.MakeFrames();

    if( pUndo )
        pUndo->EndListeningAll();

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
         _rMrkList.GetMark( 0 )->GetMarkedSdrObj()->getParentSdrObjectFromSdrObject() )
    {
        return false;
    }

    GetIDocumentUndoRedo().StartUndo( SwUndoId::INSATTR, nullptr );

    bool bUnmark = false;
    for ( size_t i = 0; i < _rMrkList.GetMarkCount(); ++i )
    {
        SdrObject* pObj = _rMrkList.GetMark( i )->GetMarkedSdrObj();
        if ( dynamic_cast<const SwVirtFlyDrawObj*>( pObj) ==  nullptr )
        {
            SwDrawContact* pContact = static_cast<SwDrawContact*>(GetUserCall(pObj));

            // consider, that drawing object has
            // no user call. E.g.: a 'virtual' drawing object is disconnected by
            // the anchor type change of the 'master' drawing object.
            // Continue with next selected object and assert, if this isn't excepted.
            if ( !pContact )
            {
#if OSL_DEBUG_LEVEL > 0
                auto pSwDrawVirtObj = dynamic_cast<SwDrawVirtObj*>( pObj);
                bool bNoUserCallExcepted = pSwDrawVirtObj && !pSwDrawVirtObj->IsConnected();
                OSL_ENSURE( bNoUserCallExcepted, "SwDoc::ChgAnchor(..) - no contact at selected drawing object" );
#endif
                continue;
            }

            // #i26791#
            const SwFrame* pOldAnchorFrame = pContact->GetAnchorFrame( pObj );
            const SwFrame* pNewAnchorFrame = pOldAnchorFrame;

            // #i54336#
            // Instead of only keeping the index position for an as-character
            // anchored object the complete <SwPosition> is kept, because the
            // anchor index position could be moved, if the object again is
            // anchored as character.
            std::optional<SwPosition> oOldAsCharAnchorPos;
            const RndStdIds eOldAnchorType = pContact->GetAnchorId();
            if ( !_bSameOnly && eOldAnchorType == RndStdIds::FLY_AS_CHAR )
            {
                oOldAsCharAnchorPos.emplace(*pContact->GetAnchorFormat().GetContentAnchor());
            }

            if ( _bSameOnly )
                _eAnchorType = eOldAnchorType;

            SwFormatAnchor aNewAnch( _eAnchorType );
            SwAnchoredObject *pAnchoredObj = pContact->GetAnchoredObj(pObj);
            tools::Rectangle aObjRect(pAnchoredObj->GetObjRect().SVRect());
            const Point aPt( aObjRect.TopLeft() );

            switch ( _eAnchorType )
            {
            case RndStdIds::FLY_AT_PARA:
            case RndStdIds::FLY_AT_CHAR:
                {
                    const Point aNewPoint = ( pOldAnchorFrame->IsVertical() ||
                                              pOldAnchorFrame->IsRightToLeft() )
                                            ? aObjRect.TopRight()
                                            : aPt;

                    // allow drawing objects in header/footer
                    pNewAnchorFrame = ::FindAnchor( pOldAnchorFrame, aNewPoint );
                    if ( pNewAnchorFrame->IsTextFrame() && static_cast<const SwTextFrame*>(pNewAnchorFrame)->IsFollow() )
                    {
                        pNewAnchorFrame = static_cast<const SwTextFrame*>(pNewAnchorFrame)->FindMaster();
                    }
                    if ( pNewAnchorFrame->IsProtected() )
                    {
                        pNewAnchorFrame = nullptr;
                    }
                    else
                    {
                        SwPosition aPos( pNewAnchorFrame->IsTextFrame()
                            ? *static_cast<SwTextFrame const*>(pNewAnchorFrame)->GetTextNodeForParaProps()
                            : *static_cast<SwNoTextFrame const*>(pNewAnchorFrame)->GetNode() );

                        aNewAnch.SetType( _eAnchorType );
                        aNewAnch.SetAnchor( &aPos );
                    }
                }
                break;

            case RndStdIds::FLY_AT_FLY: // LAYER_IMPL
                {
                    // Search the closest SwFlyFrame starting from the upper left corner.
                    SwFrame *pTextFrame;
                    {
                        SwCursorMoveState aState( CursorMoveState::SetOnlyText );
                        SwPosition aPos( GetNodes() );
                        Point aPoint( aPt );
                        aPoint.setX(aPoint.getX() - 1);
                        getIDocumentLayoutAccess().GetCurrentLayout()->GetModelPositionForViewPoint( &aPos, aPoint, &aState );
                        // consider that drawing objects can be in
                        // header/footer. Thus, <GetFrame()> by left-top-corner
                        std::pair<Point, bool> const tmp(aPt, false);
                        pTextFrame = aPos.GetNode().
                            GetContentNode()->getLayoutFrame(
                                getIDocumentLayoutAccess().GetCurrentLayout(),
                                nullptr, &tmp);
                    }
                    const SwFrame *pTmp = ::FindAnchor( pTextFrame, aPt );
                    pNewAnchorFrame = pTmp->FindFlyFrame();
                    if( pNewAnchorFrame && !pNewAnchorFrame->IsProtected() )
                    {
                        const SwFrameFormat *pTmpFormat = static_cast<const SwFlyFrame*>(pNewAnchorFrame)->GetFormat();
                        const SwFormatContent& rContent = pTmpFormat->GetContent();
                        SwPosition aPos( *rContent.GetContentIdx() );
                        aNewAnch.SetAnchor( &aPos );
                        break;
                    }

                    aNewAnch.SetType( RndStdIds::FLY_AT_PAGE );
                    [[fallthrough]];
                }
            case RndStdIds::FLY_AT_PAGE:
                {
                    pNewAnchorFrame = getIDocumentLayoutAccess().GetCurrentLayout()->Lower();
                    while ( pNewAnchorFrame && !pNewAnchorFrame->getFrameArea().Contains( aPt ) )
                        pNewAnchorFrame = pNewAnchorFrame->GetNext();
                    if ( !pNewAnchorFrame )
                        continue;

                    aNewAnch.SetPageNum( static_cast<const SwPageFrame*>(pNewAnchorFrame)->GetPhyPageNum());
                }
                break;
            case RndStdIds::FLY_AS_CHAR:
                if( _bSameOnly )    // Change of position/size
                {
                    if( !pOldAnchorFrame )
                    {
                        pContact->ConnectToLayout();
                        pOldAnchorFrame = pContact->GetAnchorFrame();
                    }
                    const_cast<SwTextFrame*>(static_cast<const SwTextFrame*>(pOldAnchorFrame))->Prepare();
                }
                else            // Change of anchors
                {
                    // allow drawing objects in header/footer
                    pNewAnchorFrame = ::FindAnchor( pOldAnchorFrame, aPt );
                    if( pNewAnchorFrame->IsProtected() )
                    {
                        pNewAnchorFrame = nullptr;
                        break;
                    }

                    bUnmark = ( 0 != i );
                    Point aPoint( aPt );
                    aPoint.setX(aPoint.getX() - 1);    // Do not load in the DrawObj!
                    aNewAnch.SetType( RndStdIds::FLY_AS_CHAR );
                    assert(pNewAnchorFrame->IsTextFrame()); // because AS_CHAR
                    SwTextFrame const*const pFrame(
                            static_cast<SwTextFrame const*>(pNewAnchorFrame));
                    SwPosition aPos( *pFrame->GetTextNodeForParaProps() );
                    if ( pNewAnchorFrame->getFrameArea().Contains( aPoint ) )
                    {
                    // We need to find a TextNode, because only there we can anchor a
                    // content-bound DrawObject.
                        SwCursorMoveState aState( CursorMoveState::SetOnlyText );
                        getIDocumentLayoutAccess().GetCurrentLayout()->GetModelPositionForViewPoint( &aPos, aPoint, &aState );
                    }
                    else
                    {
                        if ( pNewAnchorFrame->getFrameArea().Bottom() < aPt.Y() )
                        {
                            aPos = pFrame->MapViewToModelPos(TextFrameIndex(0));
                        }
                        else
                        {
                            aPos = pFrame->MapViewToModelPos(
                                TextFrameIndex(pFrame->GetText().getLength()));
                        }
                    }
                    aNewAnch.SetAnchor( &aPos );
                    SetAttr( aNewAnch, *pContact->GetFormat() );
                    // #i26791# - adjust vertical positioning to 'center to
                    // baseline'
                    SetAttr( SwFormatVertOrient( 0, text::VertOrientation::CENTER, text::RelOrientation::FRAME ), *pContact->GetFormat() );
                    SwTextNode *pNd = aPos.GetNode().GetTextNode();
                    OSL_ENSURE( pNd, "Cursor not positioned at TextNode." );

                    SwFormatFlyCnt aFormat( pContact->GetFormat() );
                    pNd->InsertItem( aFormat, aPos.GetContentIndex(), 0 );

                    // Has a textbox attached to the format? Sync it as well!
                    if (pContact->GetFormat() && pContact->GetFormat()->GetOtherTextBoxFormats())
                    {
                        SwTextBoxHelper::synchronizeGroupTextBoxProperty(
                            SwTextBoxHelper::changeAnchor, pContact->GetFormat(), pObj);
                    }
                }
                break;
            default:
                OSL_ENSURE( false, "unexpected AnchorId." );
            }

            if ( (RndStdIds::FLY_AS_CHAR != _eAnchorType) &&
                 pNewAnchorFrame &&
                 ( !_bSameOnly || pNewAnchorFrame != pOldAnchorFrame ) )
            {
                // #i26791# - Direct object positioning no longer needed. Apply
                // of attributes (method call <SetAttr(..)>) takes care of the
                // invalidation of the object position.
                if ( _bPosCorr )
                {
                    // #i33313# - consider not connected 'virtual' drawing
                    // objects
                    auto pSwDrawVirtObj = dynamic_cast<SwDrawVirtObj*>( pObj);
                    if ( pSwDrawVirtObj && !pSwDrawVirtObj->IsConnected() )
                    {
                        SwRect aNewObjRect( aObjRect );
                        static_cast<SwAnchoredDrawObject*>(pContact->GetAnchoredObj( nullptr ))
                                        ->AdjustPositioningAttr( pNewAnchorFrame,
                                                                 &aNewObjRect );
                    }
                    else
                    {
                        static_cast<SwAnchoredDrawObject*>(pContact->GetAnchoredObj( pObj ))
                                    ->AdjustPositioningAttr( pNewAnchorFrame );
                    }
                }
                if (aNewAnch.GetAnchorId() == RndStdIds::FLY_AT_PAGE)
                {
                    SwFormatHoriOrient item(pContact->GetFormat()->GetHoriOrient());
                    sal_Int16 nRelOrient(item.GetRelationOrient());
                    if (sw::GetAtPageRelOrientation(nRelOrient, false))
                    {
                        SAL_INFO("sw.ui", "fixing horizontal RelOrientation for at-page anchor");
                        item.SetRelationOrient(nRelOrient);
                        SetAttr(item, *pContact->GetFormat());
                    }
                }
                // tdf#136385 set the anchor last - otherwise it messes up the
                // position in SwDrawContact::Changed_() callback
                SetAttr(aNewAnch, *pContact->GetFormat());
            }

            // we have changed the anchoring attributes, and those are used to
            // order the object in its sorted list, so update its position
            pAnchoredObj->UpdateObjInSortedList();

            // #i54336#
            if (oOldAsCharAnchorPos)
            {
                if ( pNewAnchorFrame)
                {
                    // We need to handle InContents in a special way:
                    // The TextAttribute needs to be destroyed which, unfortunately, also
                    // destroys the format. To avoid that, we disconnect the format from
                    // the attribute.
                    const sal_Int32 nIndx( oOldAsCharAnchorPos->GetContentIndex() );
                    SwTextNode* pTextNode( oOldAsCharAnchorPos->GetNode().GetTextNode() );
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

    GetIDocumentUndoRedo().EndUndo( SwUndoId::END, nullptr );
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

    // Split flys are incompatible with chaining.
    const SwFormatFlySplit& rOldSplit = rSource.GetFlySplit();
    if (rOldSplit.GetValue())
    {
        return SwChainRet::SOURCE_CHAINED;
    }
    const SwFormatFlySplit& rNewSplit = rDest.GetFlySplit();
    if (rNewSplit.GetValue())
    {
        return SwChainRet::IS_IN_CHAIN;
    }

    // Target must be empty.
    const SwNodeIndex* pCntIdx = rDest.GetContent().GetContentIdx();
    if( !pCntIdx )
        return SwChainRet::NOT_FOUND;

    SwNodeIndex aNxtIdx( *pCntIdx, 1 );
    const SwTextNode* pTextNd = aNxtIdx.GetNode().GetTextNode();
    if( !pTextNd )
        return SwChainRet::NOT_FOUND;

    const SwNodeOffset nFlySttNd = pCntIdx->GetIndex();
    if( SwNodeOffset(2) != ( pCntIdx->GetNode().EndOfSectionIndex() - nFlySttNd ) ||
        pTextNd->GetText().getLength() )
    {
        return SwChainRet::NOT_EMPTY;
    }

    for(sw::SpzFrameFormat* pSpzFrameFm: *GetSpzFrameFormats())
    {
        const SwFormatAnchor& rAnchor = pSpzFrameFm->GetAnchor();
        // #i20622# - to-frame anchored objects are allowed.
        if ( (rAnchor.GetAnchorId() != RndStdIds::FLY_AT_PARA) &&
             (rAnchor.GetAnchorId() != RndStdIds::FLY_AT_CHAR) )
            continue;
        if ( nullptr == rAnchor.GetAnchorNode() )
            continue;
        SwNodeOffset nTstSttNd = rAnchor.GetAnchorNode()->GetIndex();
        if( nFlySttNd <= nTstSttNd && nTstSttNd < nFlySttNd + SwNodeOffset(2) )
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
    SwNodeOffset nEndOfExtras = GetNodes().GetEndOfExtras().GetIndex();
    bool bAllowed = false;
    if ( RndStdIds::FLY_AT_PAGE == rSrcAnchor.GetAnchorId() )
    {
        if ( (RndStdIds::FLY_AT_PAGE == rDstAnchor.GetAnchorId()) ||
            ( rDstAnchor.GetAnchorNode() &&
              rDstAnchor.GetAnchorNode()->GetIndex() > nEndOfExtras ))
            bAllowed = true;
    }
    else if( rSrcAnchor.GetAnchorNode() && rDstAnchor.GetAnchorNode() )
    {
        const SwNode &rSrcNd = *rSrcAnchor.GetAnchorNode(),
                     &rDstNd = *rDstAnchor.GetAnchorNode();
        const SwStartNode* pSttNd = nullptr;
        if( rSrcNd == rDstNd ||
            ( !pSttNd &&
                nullptr != ( pSttNd = rSrcNd.FindFlyStartNode() ) &&
                pSttNd == rDstNd.FindFlyStartNode() ) ||
            ( !pSttNd &&
                nullptr != ( pSttNd = rSrcNd.FindFooterStartNode() ) &&
                pSttNd == rDstNd.FindFooterStartNode() ) ||
            ( !pSttNd &&
                nullptr != ( pSttNd = rSrcNd.FindHeaderStartNode() ) &&
                pSttNd == rDstNd.FindHeaderStartNode() ) ||
            ( !pSttNd && rDstNd.GetIndex() > nEndOfExtras &&
                            rSrcNd.GetIndex() > nEndOfExtras ))
            bAllowed = true;
    }

    return bAllowed ? SwChainRet::OK : SwChainRet::WRONG_AREA;
}

SwChainRet SwDoc::Chain( SwFrameFormat &rSource, const SwFrameFormat &rDest )
{
    SwChainRet nErr = Chainable( rSource, rDest );
    if ( nErr == SwChainRet::OK )
    {
        GetIDocumentUndoRedo().StartUndo( SwUndoId::CHAINE, nullptr );

        SwFlyFrameFormat& rDestFormat = const_cast<SwFlyFrameFormat&>(static_cast<const SwFlyFrameFormat&>(rDest));

        // Attach Follow to the Master.
        SwFormatChain aChain = rDestFormat.GetChain();
        aChain.SetPrev( &static_cast<SwFlyFrameFormat&>(rSource) );
        SetAttr( aChain, rDestFormat );

        SfxItemSetFixed<RES_FRM_SIZE, RES_FRM_SIZE,
                        RES_CHAIN,  RES_CHAIN> aSet( GetAttrPool() );

        // Attach Follow to the Master.
        aChain.SetPrev( &static_cast<SwFlyFrameFormat&>(rSource) );
        SetAttr( aChain, rDestFormat );

        // Attach Master to the Follow.
        // Make sure that the Master has a fixed height.
        aChain = rSource.GetChain();
        aChain.SetNext( &rDestFormat );
        aSet.Put( aChain );

        SwFormatFrameSize aSize( rSource.GetFrameSize() );
        if ( aSize.GetHeightSizeType() != SwFrameSize::Fixed )
        {
            SwFlyFrame *pFly = SwIterator<SwFlyFrame,SwFormat>( rSource ).First();
            if ( pFly )
                aSize.SetHeight( pFly->getFrameArea().Height() );
            aSize.SetHeightSizeType( SwFrameSize::Fixed );
            aSet.Put( aSize );
        }
        SetAttr( aSet, rSource );

        GetIDocumentUndoRedo().EndUndo( SwUndoId::CHAINE, nullptr );
    }
    return nErr;
}

void SwDoc::Unchain( SwFrameFormat &rFormat )
{
    SwFormatChain aChain( rFormat.GetChain() );
    if ( aChain.GetNext() )
    {
        GetIDocumentUndoRedo().StartUndo( SwUndoId::UNCHAIN, nullptr );
        SwFrameFormat *pFollow = aChain.GetNext();
        aChain.SetNext( nullptr );
        SetAttr( aChain, rFormat );
        aChain = pFollow->GetChain();
        aChain.SetPrev( nullptr );
        SetAttr( aChain, *pFollow );
        GetIDocumentUndoRedo().EndUndo( SwUndoId::UNCHAIN, nullptr );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
