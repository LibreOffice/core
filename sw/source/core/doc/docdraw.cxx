/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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
#include <editeng/flditem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/colritem.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdogrp.hxx>
#include <editeng/measfld.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <fmtanchr.hxx>
#include <charatr.hxx>
#include <frmfmt.hxx>
#include <charfmt.hxx>
#include <viewimp.hxx>
#include <doc.hxx>
#include <docfunc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentState.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <poolfmt.hxx>
#include <drawdoc.hxx>
#include <UndoDraw.hxx>
#include <swundo.hxx>
#include <dcontact.hxx>
#include <dview.hxx>
#include <mvsave.hxx>
#include <flyfrm.hxx>
#include <dflyobj.hxx>
#include <txtfrm.hxx>
#include <editeng/frmdiritem.hxx>
#include <fmtornt.hxx>
#include <svx/svditer.hxx>

#include <vector>

using namespace ::com::sun::star;
using namespace ::com::sun::star::linguistic2;

/** local method to determine positioning and alignment attributes for a drawing
 *  object, which is newly connected to the layout.
 *
 * Used for a newly formed group object <SwDoc::GroupSelection(..)>
 * and the members of a destroyed group <SwDoc::UnGroupSelection(..)>
 */
static void lcl_AdjustPositioningAttr( SwDrawFrameFormat* _pFrameFormat,
                                const SdrObject& _rSdrObj )
{
    const SwContact* pContact = GetUserCall( &_rSdrObj );
    OSL_ENSURE( pContact, "<lcl_AdjustPositioningAttr(..)> - missing contact object." );

    // determine position of new group object relative to its anchor frame position
    SwTwips nHoriRelPos = 0;
    SwTwips nVertRelPos = 0;
    {
        const SwFrame* pAnchorFrame = pContact->GetAnchoredObj( &_rSdrObj )->GetAnchorFrame();
        OSL_ENSURE( !pAnchorFrame ||
                !pAnchorFrame->IsTextFrame() ||
                !static_cast<const SwTextFrame*>(pAnchorFrame)->IsFollow(),
                "<lcl_AdjustPositioningAttr(..)> - anchor frame is a follow." );
        bool bVert = false;
        bool bR2L = false;
        // #i45952# - use anchor position of anchor frame, if it exist.
        Point aAnchorPos;
        if ( pAnchorFrame )
        {
            // #i45952#
            aAnchorPos = pAnchorFrame->GetFrameAnchorPos( ::HasWrap( &_rSdrObj ) );
            bVert = pAnchorFrame->IsVertical();
            bR2L = pAnchorFrame->IsRightToLeft();
        }
        else
        {
            // #i45952#
            aAnchorPos = _rSdrObj.GetAnchorPos();
            // If no anchor frame exist - e.g. because no layout exists - the
            // default layout direction is taken.
            const SvxFrameDirectionItem& rDirItem =
                _pFrameFormat->GetAttrSet().GetPool()->GetDefaultItem( RES_FRAMEDIR );
            switch ( rDirItem.GetValue() )
            {
                case SvxFrameDirection::Vertical_LR_TB:
                {
                    // vertical from left-to-right
                    bVert = true;
                    bR2L = true;
                    OSL_FAIL( "<lcl_AdjustPositioningAttr(..)> - vertical from left-to-right not supported." );
                }
                break;
                case SvxFrameDirection::Vertical_RL_TB:
                {
                    // vertical from right-to-left
                    bVert = true;
                    bR2L = false;
                }
                break;
                case SvxFrameDirection::Horizontal_RL_TB:
                {
                    // horizontal from right-to-left
                    bVert = false;
                    bR2L = true;
                }
                break;
                case SvxFrameDirection::Horizontal_LR_TB:
                {
                    // horizontal from left-to-right
                    bVert = false;
                    bR2L = false;
                }
                break;
                case SvxFrameDirection::Environment:
                    SAL_WARN("sw.core", "lcl_AdjustPositioningAttr(..) SvxFrameDirection::Environment not supported");
                    break;
                default: break;
            }

        }
        // use geometry of drawing object
        const tools::Rectangle aObjRect = _rSdrObj.GetSnapRect();

        if ( bVert )
        {
            if ( bR2L ) {
                  //SvxFrameDirection::Vertical_LR_TB
                  nHoriRelPos = aObjRect.Left() - aAnchorPos.getX();
                  nVertRelPos = aObjRect.Top() - aAnchorPos.getY();
            } else {
                //SvxFrameDirection::Vertical_RL_TB
                nHoriRelPos = aObjRect.Top() - aAnchorPos.getY();
                nVertRelPos = aAnchorPos.getX() - aObjRect.Right();
            }
        }
        else if ( bR2L )
        {
            nHoriRelPos = aAnchorPos.getX() - aObjRect.Right();
            nVertRelPos = aObjRect.Top() - aAnchorPos.getY();
        }
        else
        {
            nHoriRelPos = aObjRect.Left() - aAnchorPos.getX();
            nVertRelPos = aObjRect.Top() - aAnchorPos.getY();
        }
    }

    _pFrameFormat->SetFormatAttr( SwFormatHoriOrient( nHoriRelPos, text::HoriOrientation::NONE, text::RelOrientation::FRAME ) );
    _pFrameFormat->SetFormatAttr( SwFormatVertOrient( nVertRelPos, text::VertOrientation::NONE, text::RelOrientation::FRAME ) );
    // #i44334#, #i44681# - positioning attributes already set
    _pFrameFormat->PosAttrSet();
    // #i34750# - keep current object rectangle for  drawing
    // objects. The object rectangle is used on events from the drawing layer
    // to adjust the positioning attributes - see <SwDrawContact::Changed_(..)>.
    {
        const SwAnchoredObject* pAnchoredObj = pContact->GetAnchoredObj( &_rSdrObj );
        if ( auto pAnchoredDrawObj = dynamic_cast<const SwAnchoredDrawObject*>( pAnchoredObj) )
        {
            const tools::Rectangle aObjRect = _rSdrObj.GetSnapRect();
            const_cast<SwAnchoredDrawObject*>(pAnchoredDrawObj)
                                        ->SetLastObjRect( aObjRect );
        }
    }
}

SwDrawContact* SwDoc::GroupSelection( SdrView& rDrawView )
{
    // replace marked 'virtual' drawing objects by the corresponding 'master'
    // drawing objects.
    SwDrawView::ReplaceMarkedDrawVirtObjs( rDrawView );

    const SdrMarkList &rMrkList = rDrawView.GetMarkedObjectList();
    SdrObject *pObj = rMrkList.GetMark( 0 )->GetMarkedSdrObj();
    bool bNoGroup = ( nullptr == pObj->getParentSdrObjectFromSdrObject() );
    SwDrawContact* pNewContact = nullptr;
    if( bNoGroup )
    {
        SwDrawFrameFormat *pFormat = nullptr;

        // Revoke anchor attribute.
        SwDrawContact *pMyContact = static_cast<SwDrawContact*>(GetUserCall(pObj));
        const SwFormatAnchor aAnch( pMyContact->GetFormat()->GetAnchor() );

        std::unique_ptr<SwUndoDrawGroup> pUndo;
        if (GetIDocumentUndoRedo().DoesUndo())
            pUndo.reset(new SwUndoDrawGroup( o3tl::narrowing<sal_uInt16>(rMrkList.GetMarkCount()), *this));

        // #i53320#
        bool bGroupMembersNotPositioned( false );
        {
            SwAnchoredDrawObject* pAnchoredDrawObj =
                static_cast<SwAnchoredDrawObject*>(pMyContact->GetAnchoredObj( pObj ));
            bGroupMembersNotPositioned = pAnchoredDrawObj->NotYetPositioned();
        }

        std::map<const SdrObject*, SwFrameFormat*> vSavedTextBoxes;
        // Destroy ContactObjects and formats.
        for( size_t i = 0; i < rMrkList.GetMarkCount(); ++i )
        {
            pObj = rMrkList.GetMark( i )->GetMarkedSdrObj();
            SwDrawContact *pContact = static_cast<SwDrawContact*>(GetUserCall(pObj));

            // #i53320#
#if OSL_DEBUG_LEVEL > 0
            SwAnchoredDrawObject* pAnchoredDrawObj =
                static_cast<SwAnchoredDrawObject*>(pContact->GetAnchoredObj( pObj ));
            OSL_ENSURE( bGroupMembersNotPositioned == pAnchoredDrawObj->NotYetPositioned(),
                    "<SwDoc::GroupSelection(..)> - group members have different positioning status!" );
#endif
            // Before the format will be killed, save its textbox for later use.
            if (auto pShapeFormat = pContact->GetFormat())
                if (auto pTextBoxNode = pShapeFormat->GetOtherTextBoxFormat())
                    for (const auto& rTextBoxElement : pTextBoxNode->GetAllTextBoxes())
                        vSavedTextBoxes.emplace(rTextBoxElement);

            pFormat = static_cast<SwDrawFrameFormat*>(pContact->GetFormat());
            // Deletes itself!
            pContact->Changed(*pObj, SdrUserCallType::Delete, pObj->GetLastBoundRect() );
            pObj->SetUserCall( nullptr );

            if( pUndo )
                pUndo->AddObj( i, pFormat, pObj );
            else
                DelFrameFormat( pFormat );

            // #i45952# - re-introduce position normalization of group member
            // objects, because its anchor position is cleared, when they are
            // grouped.
            Point aAnchorPos( pObj->GetAnchorPos() );
            pObj->NbcSetAnchorPos( Point( 0, 0 ) );
            pObj->NbcMove( Size( aAnchorPos.getX(), aAnchorPos.getY() ) );
        }

        pFormat = MakeDrawFrameFormat( GetUniqueDrawObjectName(),
                                GetDfltFrameFormat() );
        pFormat->SetFormatAttr( aAnch );
        // #i36010# - set layout direction of the position
        pFormat->SetPositionLayoutDir(
            text::PositionLayoutDir::PositionInLayoutDirOfAnchor );

        // Add the saved textboxes to the new format.
        auto pTextBoxNode = new SwTextBoxNode(pFormat);
        for (const auto& pTextBoxEntry : vSavedTextBoxes)
        {
            pTextBoxNode->AddTextBox(const_cast<SdrObject*>(pTextBoxEntry.first),
                                     pTextBoxEntry.second);
            pTextBoxEntry.second->SetOtherTextBoxFormat(pTextBoxNode);
        }
        pFormat->SetOtherTextBoxFormat(pTextBoxNode);
        vSavedTextBoxes.clear();

        rDrawView.GroupMarked();
        OSL_ENSURE( rMrkList.GetMarkCount() == 1, "GroupMarked more or none groups." );

        SdrObject* pNewGroupObj = rMrkList.GetMark( 0 )->GetMarkedSdrObj();
        pNewGroupObj->SetName(pFormat->GetName());
        pNewContact = new SwDrawContact( pFormat, pNewGroupObj );
        // #i35635#
        pNewContact->MoveObjToVisibleLayer( pNewGroupObj );
        pNewContact->ConnectToLayout();
        // #i53320# - No adjustment of the positioning and alignment
        // attributes, if group members aren't positioned yet.
        if ( !bGroupMembersNotPositioned )
        {
            // #i26791# - Adjust positioning and alignment attributes.
            lcl_AdjustPositioningAttr( pFormat, *pNewGroupObj );
        }

        if( pUndo )
        {
            pUndo->SetGroupFormat( pFormat );
            GetIDocumentUndoRedo().AppendUndo( std::move(pUndo) );
        }
    }
    else
    {
        if (GetIDocumentUndoRedo().DoesUndo())
        {
            GetIDocumentUndoRedo().ClearRedo();
        }

        rDrawView.GroupMarked();
        OSL_ENSURE( rMrkList.GetMarkCount() == 1, "GroupMarked more or none groups." );
    }

    return pNewContact;
}

static void lcl_CollectTextBoxesForSubGroupObj(SwFrameFormat* pTargetFormat, SwTextBoxNode* pTextBoxNode,
                                               SdrObject* pSourceObjs)
{
    if (auto pChildrenObjs = pSourceObjs->getChildrenOfSdrObject())
        for (size_t i = 0; i < pChildrenObjs->GetObjCount(); ++i)
            lcl_CollectTextBoxesForSubGroupObj(pTargetFormat, pTextBoxNode, pChildrenObjs->GetObj(i));
    else
    {
        if (auto pTextBox = pTextBoxNode->GetTextBox(pSourceObjs))
        {
            if (!pTargetFormat->GetOtherTextBoxFormat())
            {
                pTargetFormat->SetOtherTextBoxFormat(new SwTextBoxNode(pTargetFormat));
            }
            pTargetFormat->GetOtherTextBoxFormat()->AddTextBox(pSourceObjs, pTextBox);
            pTextBox->SetOtherTextBoxFormat(pTargetFormat->GetOtherTextBoxFormat());
        }
    }
}


void SwDoc::UnGroupSelection( SdrView& rDrawView )
{
    bool const bUndo = GetIDocumentUndoRedo().DoesUndo();
    if( bUndo )
    {
        GetIDocumentUndoRedo().ClearRedo();
    }

    // replace marked 'virtual' drawing objects by the corresponding 'master'
    // drawing objects.
    SwDrawView::ReplaceMarkedDrawVirtObjs( rDrawView );

    const SdrMarkList &rMrkList = rDrawView.GetMarkedObjectList();
    std::unique_ptr<std::vector< std::pair< SwDrawFrameFormat*, SdrObject* > >[]> pFormatsAndObjs;
    const size_t nMarkCount( rMrkList.GetMarkCount() );
    if ( nMarkCount )
    {
        pFormatsAndObjs.reset( new std::vector< std::pair< SwDrawFrameFormat*, SdrObject* > >[nMarkCount] );
        SdrObject *pMyObj = rMrkList.GetMark( 0 )->GetMarkedSdrObj();
        if( !pMyObj->getParentSdrObjectFromSdrObject() )
        {
            for ( size_t i = 0; i < nMarkCount; ++i )
            {
                SdrObject *pObj = rMrkList.GetMark( i )->GetMarkedSdrObj();
                if ( auto pObjGroup = dynamic_cast<SdrObjGroup*>(pObj) )
                {
                    SwDrawContact *pContact = static_cast<SwDrawContact*>(GetUserCall(pObj));

                    SwTextBoxNode* pTextBoxNode = nullptr;
                    if (auto pGroupFormat = pContact->GetFormat())
                        pTextBoxNode = pGroupFormat->GetOtherTextBoxFormat();

                    SwFormatAnchor aAnch( pContact->GetFormat()->GetAnchor() );
                    SdrObjList *pLst = pObjGroup->GetSubList();

                    SwUndoDrawUnGroup* pUndo = nullptr;
                    if( bUndo )
                    {
                        pUndo = new SwUndoDrawUnGroup( pObjGroup, *this );
                        GetIDocumentUndoRedo().AppendUndo(std::unique_ptr<SwUndo>(pUndo));
                    }

                    for ( size_t i2 = 0; i2 < pLst->GetObjCount(); ++i2 )
                    {
                        SdrObject* pSubObj = pLst->GetObj( i2 );
                        SwDrawFrameFormat *pFormat = MakeDrawFrameFormat( GetUniqueShapeName(),
                                                            GetDfltFrameFormat() );
                        pFormat->SetFormatAttr( aAnch );

                        if (pTextBoxNode)
                        {
                            if (!pObj->getChildrenOfSdrObject())
                            {
                                if (auto pTextBoxFormat = pTextBoxNode->GetTextBox(pSubObj))
                                {
                                    auto pNewTextBoxNode = new SwTextBoxNode(pFormat);
                                    pNewTextBoxNode->AddTextBox(pSubObj, pTextBoxFormat);
                                    pFormat->SetOtherTextBoxFormat(pNewTextBoxNode);
                                    pTextBoxFormat->SetOtherTextBoxFormat(pNewTextBoxNode);
                                }
                            }
                            else
                            {
                                lcl_CollectTextBoxesForSubGroupObj(pFormat, pTextBoxNode, pSubObj);
                            }
                        }
                        // #i36010# - set layout direction of the position
                        pFormat->SetPositionLayoutDir(
                            text::PositionLayoutDir::PositionInLayoutDirOfAnchor );
                        if (pSubObj->GetName().isEmpty())
                            pSubObj->SetName(pFormat->GetName());
                        pFormatsAndObjs[i].emplace_back( pFormat, pSubObj );

                        if( bUndo )
                            pUndo->AddObj( o3tl::narrowing<sal_uInt16>(i2), pFormat );
                    }
                }
            }
        }
    }
    rDrawView.UnGroupMarked();
    // creation of <SwDrawContact> instances for the former group members and
    // its connection to the Writer layout.
    for ( size_t i = 0; i < nMarkCount; ++i )
    {
        SwUndoDrawUnGroupConnectToLayout* pUndo = nullptr;
        if( bUndo )
        {
            pUndo = new SwUndoDrawUnGroupConnectToLayout(*this);
            GetIDocumentUndoRedo().AppendUndo(std::unique_ptr<SwUndo>(pUndo));
        }

        while ( !pFormatsAndObjs[i].empty() )
        {
            SwDrawFrameFormat* pFormat( pFormatsAndObjs[i].back().first );
            SdrObject* pObj( pFormatsAndObjs[i].back().second );
            pFormatsAndObjs[i].pop_back();

            SwDrawContact* pContact = new SwDrawContact( pFormat, pObj );
            pContact->MoveObjToVisibleLayer( pObj );
            pContact->ConnectToLayout();
            lcl_AdjustPositioningAttr( pFormat, *pObj );

            if ( bUndo )
            {
                pUndo->AddFormatAndObj( pFormat, pObj );
            }
        }
    }
}

bool SwDoc::DeleteSelection( SwDrawView& rDrawView )
{
    bool bCallBase = false;
    const SdrMarkList &rMrkList = rDrawView.GetMarkedObjectList();
    if( rMrkList.GetMarkCount() )
    {
        GetIDocumentUndoRedo().StartUndo(SwUndoId::EMPTY, nullptr);
        bool bDelMarked = true;

        if( 1 == rMrkList.GetMarkCount() )
        {
            SdrObject *pObj = rMrkList.GetMark( 0 )->GetMarkedSdrObj();
            if( auto pDrawObj = dynamic_cast<SwVirtFlyDrawObj*>( pObj) )
            {
                SwFlyFrameFormat* pFrameFormat = pDrawObj->GetFlyFrame()->GetFormat();
                if( pFrameFormat )
                {
                    getIDocumentLayoutAccess().DelLayoutFormat( pFrameFormat );
                    bDelMarked = false;
                }
            }
        }

        for( size_t i = 0; i < rMrkList.GetMarkCount(); ++i )
        {
            SdrObject *pObj = rMrkList.GetMark( i )->GetMarkedSdrObj();
            if( dynamic_cast<const SwVirtFlyDrawObj*>( pObj) ==  nullptr )
            {
                SwDrawContact *pC = static_cast<SwDrawContact*>(GetUserCall(pObj));
                SwDrawFrameFormat *pFrameFormat = static_cast<SwDrawFrameFormat*>(pC->GetFormat());
                if( pFrameFormat &&
                    RndStdIds::FLY_AS_CHAR == pFrameFormat->GetAnchor().GetAnchorId() )
                {
                    rDrawView.MarkObj( pObj, rDrawView.Imp().GetPageView(), true );
                    --i;
                    getIDocumentLayoutAccess().DelLayoutFormat( pFrameFormat );
                }
            }
        }

        if( rMrkList.GetMarkCount() && bDelMarked )
        {
            SdrObject *pObj = rMrkList.GetMark( 0 )->GetMarkedSdrObj();
            if( !pObj->getParentSdrObjectFromSdrObject() )
            {
                std::unique_ptr<SwUndoDrawDelete> pUndo;
                if (GetIDocumentUndoRedo().DoesUndo())
                    pUndo.reset(new SwUndoDrawDelete( o3tl::narrowing<sal_uInt16>(rMrkList.GetMarkCount()), *this ));

                // Destroy ContactObjects, save formats.
                for( size_t i = 0; i < rMrkList.GetMarkCount(); ++i )
                {
                    const SdrMark& rMark = *rMrkList.GetMark( i );
                    pObj = rMark.GetMarkedSdrObj();
                    SwDrawContact *pContact = static_cast<SwDrawContact*>(pObj->GetUserCall());
                    if( pContact ) // of course not for grouped objects
                    {
                        SwDrawFrameFormat *pFormat = static_cast<SwDrawFrameFormat*>(pContact->GetFormat());
                        // before delete of selection is performed, marked
                        // <SwDrawVirtObj>-objects have to be replaced by its
                        // reference objects.  Thus, assert, if a
                        // <SwDrawVirt>-object is found in the mark list.
                        if ( dynamic_cast<const SwDrawVirtObj*>( pObj) !=  nullptr )
                        {
                            OSL_FAIL( "<SwDrawVirtObj> is still marked for delete. application will crash!" );
                        }
                        // Deletes itself!
                        pContact->Changed(*pObj, SdrUserCallType::Delete, pObj->GetLastBoundRect() );
                        pObj->SetUserCall( nullptr );

                        if( pUndo )
                            pUndo->AddObj( pFormat, rMark );
                        else
                            DelFrameFormat( pFormat );
                    }
                }

                if( pUndo )
                {
                    GetIDocumentUndoRedo().AppendUndo( std::move(pUndo) );
                }
            }
            bCallBase = true;
        }
        getIDocumentState().SetModified();

        GetIDocumentUndoRedo().EndUndo(SwUndoId::EMPTY, nullptr);
    }

    return bCallBase;
}

ZSortFly::ZSortFly(const SwFrameFormat* pFrameFormat, const SwFormatAnchor* pFlyAn, sal_uInt32 nArrOrdNum)
    : m_pFormat(pFrameFormat)
    , m_pAnchor(pFlyAn)
    , m_nOrdNum(nArrOrdNum)
{
    SAL_WARN_IF(m_pFormat->Which() != RES_FLYFRMFMT && m_pFormat->Which() != RES_DRAWFRMFMT, "sw.core", "What kind of format is this?");
    m_pFormat->CallSwClientNotify(sw::GetZOrderHint(m_nOrdNum));
}

/// In the Outliner, set a link to the method for field display in edit objects.
void SwDoc::SetCalcFieldValueHdl(Outliner* pOutliner)
{
    pOutliner->SetCalcFieldValueHdl(LINK(this, SwDoc, CalcFieldValueHdl));
}

/// Recognise fields/URLs in the Outliner and set how they are displayed.
IMPL_LINK(SwDoc, CalcFieldValueHdl, EditFieldInfo*, pInfo, void)
{
    if (!pInfo)
        return;

    const SvxFieldItem& rField = pInfo->GetField();
    const SvxFieldData* pField = rField.GetField();

    if (auto pDateField = dynamic_cast<const SvxDateField*>( pField))
    {
        // Date field
        pInfo->SetRepresentation(
            pDateField->GetFormatted(
                    *GetNumberFormatter(), LANGUAGE_SYSTEM) );
    }
    else if (auto pURLField = dynamic_cast<const SvxURLField*>( pField))
    {
        // URL field
        switch ( pURLField->GetFormat() )
        {
            case SvxURLFormat::AppDefault: //!!! Can be set in App???
            case SvxURLFormat::Repr:
                pInfo->SetRepresentation(pURLField->GetRepresentation());
                break;

            case SvxURLFormat::Url:
                pInfo->SetRepresentation(pURLField->GetURL());
                break;
        }

        sal_uInt16 nChrFormat;

        if (IsVisitedURL(pURLField->GetURL()))
            nChrFormat = RES_POOLCHR_INET_VISIT;
        else
            nChrFormat = RES_POOLCHR_INET_NORMAL;

        SwFormat *pFormat = getIDocumentStylePoolAccess().GetCharFormatFromPool(nChrFormat);

        Color aColor(COL_LIGHTBLUE);
        if (pFormat)
            aColor = pFormat->GetColor().GetValue();

        pInfo->SetTextColor(aColor);
    }
    else if (dynamic_cast<const SdrMeasureField*>( pField))
    {
        // Clear measure field
        pInfo->SetFieldColor(std::optional<Color>());
    }
    else if ( auto pTimeField = dynamic_cast<const SvxExtTimeField*>( pField) )
    {
        // Time field
        pInfo->SetRepresentation(
            pTimeField->GetFormatted(*GetNumberFormatter(), LANGUAGE_SYSTEM) );
    }
    else
    {
        OSL_FAIL("unknown field command");
        pInfo->SetRepresentation( OUString( '?' ) );
    }
}

// #i62875#
namespace docfunc
{
    bool ExistsDrawObjs( SwDoc& p_rDoc )
    {
        bool bExistsDrawObjs( false );

        if ( p_rDoc.getIDocumentDrawModelAccess().GetDrawModel() &&
             p_rDoc.getIDocumentDrawModelAccess().GetDrawModel()->GetPage( 0 ) )
        {
            const SdrPage& rSdrPage( *(p_rDoc.getIDocumentDrawModelAccess().GetDrawModel()->GetPage( 0 )) );

            SdrObjListIter aIter( &rSdrPage, SdrIterMode::Flat );
            while( aIter.IsMore() )
            {
                SdrObject* pObj( aIter.Next() );
                if ( !dynamic_cast<SwVirtFlyDrawObj*>(pObj) &&
                     !dynamic_cast<SwFlyDrawObj*>(pObj) )
                {
                    bExistsDrawObjs = true;
                    break;
                }
            }
        }

        return bExistsDrawObjs;
    }

    bool AllDrawObjsOnPage( SwDoc& p_rDoc )
    {
        bool bAllDrawObjsOnPage( true );

        if ( p_rDoc.getIDocumentDrawModelAccess().GetDrawModel() &&
             p_rDoc.getIDocumentDrawModelAccess().GetDrawModel()->GetPage( 0 ) )
        {
            const SdrPage& rSdrPage( *(p_rDoc.getIDocumentDrawModelAccess().GetDrawModel()->GetPage( 0 )) );

            SdrObjListIter aIter( &rSdrPage, SdrIterMode::Flat );
            while( aIter.IsMore() )
            {
                SdrObject* pObj( aIter.Next() );
                if ( !dynamic_cast<SwVirtFlyDrawObj*>(pObj) &&
                     !dynamic_cast<SwFlyDrawObj*>(pObj) )
                {
                    SwDrawContact* pDrawContact =
                            dynamic_cast<SwDrawContact*>(::GetUserCall( pObj ));
                    if ( pDrawContact )
                    {
                        SwAnchoredDrawObject* pAnchoredDrawObj =
                            dynamic_cast<SwAnchoredDrawObject*>(pDrawContact->GetAnchoredObj( pObj ));

                        // error handling
                        {
                            if ( !pAnchoredDrawObj )
                            {
                                OSL_FAIL( "<docfunc::AllDrawObjsOnPage() - missing anchored draw object" );
                                bAllDrawObjsOnPage = false;
                                break;
                            }
                        }

                        if ( pAnchoredDrawObj->NotYetPositioned() )
                        {
                            // The drawing object isn't yet layouted.
                            // Thus, it isn't known, if all drawing objects are on page.
                            bAllDrawObjsOnPage = false;
                            break;
                        }
                        else if ( pAnchoredDrawObj->IsOutsidePage() )
                        {
                            bAllDrawObjsOnPage = false;
                            break;
                        }
                    }
                    else
                    {
                        // contact object of drawing object doesn't exists.
                        // Thus, the drawing object isn't yet positioned.
                        // Thus, it isn't known, if all drawing objects are on page.
                        bAllDrawObjsOnPage = false;
                        break;
                    }
                }
            }
        }

        return bAllDrawObjsOnPage;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
