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

#include <svx/svxids.hrc>
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/svdmark.hxx>
#include <svx/svdview.hxx>
#include <svx/svdouno.hxx>
#include <svx/srchdlg.hxx>
#include <com/sun/star/form/FormButtonType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <osl/diagnose.h>
#include <sfx2/dispatch.hxx>
#include <comphelper/lok.hxx>
#include <tools/json_writer.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

#include <swmodule.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <IMark.hxx>
#include <doc.hxx>
#include <formatcontentcontrol.hxx>
#include <IDocumentUndoRedo.hxx>
#include <SwRewriter.hxx>
#include <strings.hrc>
#include <textcontentcontrol.hxx>

#include <vector>
#include <set>
#include <IDocumentContentOperations.hxx>
#include <ndtxt.hxx>

using namespace ::com::sun::star;

bool SwWrtShell::MoveBookMark( BookMarkMove eFuncId, const ::sw::mark::MarkBase* const pMark)
{
    addCurrentPosition();
    (this->*m_fnKillSel)( nullptr, false, ScrollSizeMode::ScrollSizeDefault );

    bool bRet = true;
    switch(eFuncId)
    {
        case BOOKMARK_INDEX:bRet = SwCursorShell::GotoMark( pMark );break;
        case BOOKMARK_NEXT: bRet = SwCursorShell::GoNextBookmark();break;
        case BOOKMARK_PREV: bRet = SwCursorShell::GoPrevBookmark();break;
        default:;//prevent warning
    }

    if( bRet && IsSelFrameMode() )
    {
        UnSelectFrame();
        LeaveSelFrameMode();
    }
    if( IsSelection() )
    {
        m_fnKillSel = &SwWrtShell::ResetSelect;
        m_fnSetCursor = &SwWrtShell::SetCursorKillSel;
    }
    return bRet;
}

bool SwWrtShell::GotoField( const SwFormatField& rField )
{
    (this->*m_fnKillSel)( nullptr, false, ScrollSizeMode::ScrollSizeDefault );

    bool bRet = SwCursorShell::GotoFormatField( rField );
    if( bRet && IsSelFrameMode() )
    {
        UnSelectFrame();
        LeaveSelFrameMode();
    }

    if( IsSelection() )
    {
        m_fnKillSel = &SwWrtShell::ResetSelect;
        m_fnSetCursor = &SwWrtShell::SetCursorKillSel;
    }

    return bRet;
}

bool SwWrtShell::GotoContentControl(const SwFormatContentControl& rContentControl,
                                    bool bOnlyRefresh)
{
    const std::shared_ptr<SwContentControl>& pContentControl = rContentControl.GetContentControl();
    if (IsFrameSelected() && pContentControl && pContentControl->GetPicture())
    {
        // A frame is already selected, and its anchor is inside a picture content control.
        if (pContentControl->GetShowingPlaceHolder())
        {
            // Replace the placeholder image with a real one.
            GetView().StopShellTimer();
            if (comphelper::LibreOfficeKit::isActive())
            {
                tools::JsonWriter aJson;
                aJson.put("action", "change-picture");
                OString pJson(aJson.finishAndGetAsOString());
                if (SfxViewShell* pNotifySh = GetSfxViewShell())
                    pNotifySh->libreOfficeKitViewCallback(LOK_CALLBACK_CONTENT_CONTROL,
                                                              pJson);
            }
            else
            {
                GetView().GetViewFrame().GetDispatcher()->Execute(SID_CHANGE_PICTURE,
                                                                   SfxCallMode::SYNCHRON);
            }
            pContentControl->SetShowingPlaceHolder(false);
        }
        return true;
    }

    (this->*m_fnKillSel)(nullptr, false, ScrollSizeMode::ScrollSizeDefault);

    bool bRet = SwCursorShell::GotoFormatContentControl(rContentControl);

    if (bRet && pContentControl && pContentControl->GetCheckbox())
    {
        // Checkbox: GotoFormatContentControl() selected the old state.
        LockView(/*bViewLocked=*/true);
        OUString aOldState = GetCursorDescr();
        OUString aNewState;
        if (pContentControl->GetChecked())
            aNewState = bOnlyRefresh ? pContentControl->GetCheckedState()
                                     : pContentControl->GetUncheckedState();
        else
            aNewState = bOnlyRefresh ? pContentControl->GetUncheckedState()
                                     : pContentControl->GetCheckedState();

        SwRewriter aRewriter;
        aRewriter.AddRule(UndoArg1, aOldState);
        aRewriter.AddRule(UndoArg2, SwResId(STR_YIELDS));
        aRewriter.AddRule(UndoArg3, aNewState);
        GetIDocumentUndoRedo().StartUndo(SwUndoId::REPLACE, &aRewriter);

        // Toggle the state.
        pContentControl->SetReadWrite(true);
        DelLeft();
        if (!bOnlyRefresh)
            pContentControl->SetChecked(!pContentControl->GetChecked());
        Insert(aNewState);
        pContentControl->SetReadWrite(false);

        GetIDocumentUndoRedo().EndUndo(SwUndoId::REPLACE, &aRewriter);
        LockView(/*bViewLocked=*/false);
        ShowCursor();
    }
    else if (bRet && pContentControl && pContentControl->GetSelectedListItem())
    {
        // Dropdown: GotoFormatContentControl() selected the old content.
        size_t nSelectedListItem = *pContentControl->GetSelectedListItem();
        LockView(/*bViewLocked=*/true);
        OUString aOldState = GetCursorDescr();
        OUString aNewState = pContentControl->GetListItems()[nSelectedListItem].ToString();
        SwRewriter aRewriter;
        aRewriter.AddRule(UndoArg1, aOldState);
        aRewriter.AddRule(UndoArg2, SwResId(STR_YIELDS));
        aRewriter.AddRule(UndoArg3, SwResId(STR_START_QUOTE) + aNewState + SwResId(STR_END_QUOTE));
        GetIDocumentUndoRedo().StartUndo(SwUndoId::REPLACE, &aRewriter);

        // Update the content.
        pContentControl->SetReadWrite(true);
        DelLeft();
        pContentControl->SetSelectedListItem(std::nullopt);
        Insert(aNewState);
        pContentControl->SetReadWrite(false);

        GetIDocumentUndoRedo().EndUndo(SwUndoId::REPLACE, &aRewriter);
        LockView(/*bViewLocked=*/false);
        ShowCursor();
    }
    else if (bRet && pContentControl && pContentControl->GetSelectedDate())
    {
        // Date: GotoFormatContentControl() selected the old content.
        LockView(/*bViewLocked=*/true);
        OUString aOldState = GetCursorDescr();
        OUString aNewState = pContentControl->GetDateString();
        SwRewriter aRewriter;
        aRewriter.AddRule(UndoArg1, aOldState);
        aRewriter.AddRule(UndoArg2, SwResId(STR_YIELDS));
        aRewriter.AddRule(UndoArg3, SwResId(STR_START_QUOTE) + aNewState + SwResId(STR_END_QUOTE));
        GetIDocumentUndoRedo().StartUndo(SwUndoId::REPLACE, &aRewriter);

        // Write the doc model.
        pContentControl->SetCurrentDateValue(*pContentControl->GetSelectedDate());
        pContentControl->SetSelectedDate(std::nullopt);

        // Update the content.
        DelLeft();
        Insert(aNewState);

        GetIDocumentUndoRedo().EndUndo(SwUndoId::REPLACE, &aRewriter);
        LockView(/*bViewLocked=*/false);
        ShowCursor();
    }

    if (bRet && IsSelFrameMode())
    {
        UnSelectFrame();
        LeaveSelFrameMode();
    }

    if (IsSelection())
    {
        m_fnKillSel = &SwWrtShell::ResetSelect;
        m_fnSetCursor = &SwWrtShell::SetCursorKillSel;
    }

    return bRet;
}

bool SwWrtShell::GotoFieldmark(::sw::mark::Fieldmark const * const pMark)
{
    (this->*m_fnKillSel)( nullptr, false, ScrollSizeMode::ScrollSizeDefault );
    bool bRet = SwCursorShell::GotoFieldmark(pMark);
    if( bRet && IsSelFrameMode() )
    {
        UnSelectFrame();
        LeaveSelFrameMode();
    }
    if( IsSelection() )
    {
        m_fnKillSel = &SwWrtShell::ResetSelect;
        m_fnSetCursor = &SwWrtShell::SetCursorKillSel;
    }
    return bRet;
}

// Invalidate FontWork-Slots

void SwWrtShell::DrawSelChanged( )
{
    static sal_uInt16 const aInval[] =
    {
        SID_ATTR_FILL_STYLE, SID_ATTR_FILL_COLOR, SID_ATTR_LINE_STYLE,
        SID_ATTR_LINE_WIDTH, SID_ATTR_LINE_COLOR,
        /*AF: these may be needed for the sidebar.
        SID_SVX_AREA_TRANSPARENCY, SID_SVX_AREA_TRANSP_GRADIENT,
        SID_SVX_AREA_TRANS_TYPE,
        */
        0
    };

    GetView().GetViewFrame().GetBindings().Invalidate(aInval);

    bool bOldVal = g_bNoInterrupt;
    g_bNoInterrupt = true;    // Trick to run AttrChangedNotify by timer.
    GetView().AttrChangedNotify(nullptr);
    g_bNoInterrupt = bOldVal;
}

void SwWrtShell::GotoMark( const SwMarkName& rName )
{
    auto ppMark = getIDocumentMarkAccess()->findMark( rName );
    if (ppMark == getIDocumentMarkAccess()->getAllMarksEnd())
        return;
    MoveBookMark( BOOKMARK_INDEX, *ppMark );
}

void SwWrtShell::GotoMark( const ::sw::mark::MarkBase* const pMark )
{
    MoveBookMark( BOOKMARK_INDEX, pMark );
}

bool SwWrtShell::GoNextBookmark()
{
    if ( !getIDocumentMarkAccess()->getBookmarksCount() )
    {
        SvxSearchDialogWrapper::SetSearchLabel( SearchLabel::NavElementNotFound );
        return false;
    }
    LockView( true );
    bool bRet = MoveBookMark( BOOKMARK_NEXT );
    if ( !bRet )
    {
        MoveBookMark( BOOKMARK_INDEX, *getIDocumentMarkAccess()->getBookmarksBegin() );
        SvxSearchDialogWrapper::SetSearchLabel( SearchLabel::EndWrapped );
    }
    else
        SvxSearchDialogWrapper::SetSearchLabel( SearchLabel::Empty );
    LockView( false );
    ShowCursor();
    return true;
}

bool SwWrtShell::GoPrevBookmark()
{
    if ( !getIDocumentMarkAccess()->getBookmarksCount() )
    {
        SvxSearchDialogWrapper::SetSearchLabel( SearchLabel::NavElementNotFound );
        return false;
    }
    LockView( true );
    bool bRet = MoveBookMark( BOOKMARK_PREV );
    if ( !bRet )
    {
        MoveBookMark( BOOKMARK_INDEX, *( getIDocumentMarkAccess()->getBookmarksEnd() - 1 ) );
        SvxSearchDialogWrapper::SetSearchLabel( SearchLabel::StartWrapped );
    }
    else
        SvxSearchDialogWrapper::SetSearchLabel( SearchLabel::Empty );
    LockView( false );
    ShowCursor();
    return true;
}

void SwWrtShell::ExecMacro( const SvxMacro& rMacro, OUString* pRet, SbxArray* pArgs )
{
    // execute macro, if it is allowed.
    if ( IsMacroExecAllowed() )
    {
        GetDoc()->ExecMacro( rMacro, pRet, pArgs );
    }
}

sal_uInt16 SwWrtShell::CallEvent( SvMacroItemId nEvent, const SwCallMouseEvent& rCallEvent,
                                bool bChkPtr)
{
    return GetDoc()->CallEvent( nEvent, rCallEvent, bChkPtr );
}

    // If a util::URL-Button is selected, return its util::URL
    // otherwise an empty string.
bool SwWrtShell::GetURLFromButton( OUString& rURL, OUString& rDescr ) const
{
    bool bRet = false;
    const SdrView *pDView = GetDrawView();
    if( pDView )
    {
        // A fly is precisely achievable if it is selected.
        const SdrMarkList &rMarkList = pDView->GetMarkedObjectList();

        if (rMarkList.GetMark(0))
        {
            SdrUnoObj* pUnoCtrl = dynamic_cast<SdrUnoObj*>( rMarkList.GetMark(0)->GetMarkedSdrObj() );
            if (pUnoCtrl && SdrInventor::FmForm == pUnoCtrl->GetObjInventor())
            {
                const uno::Reference< awt::XControlModel >&  xControlModel = pUnoCtrl->GetUnoControlModel();

                OSL_ENSURE( xControlModel.is(), "UNO-Control without Model" );
                if( !xControlModel.is() )
                    return bRet;

                uno::Reference< beans::XPropertySet >  xPropSet(xControlModel, uno::UNO_QUERY);

                uno::Any aTmp;

                uno::Reference< beans::XPropertySetInfo >   xInfo = xPropSet->getPropertySetInfo();
                if(xInfo->hasPropertyByName( u"ButtonType"_ustr ))
                {
                    aTmp = xPropSet->getPropertyValue( u"ButtonType"_ustr );
                    form::FormButtonType eTmpButtonType;
                    aTmp >>= eTmpButtonType;
                    if( form::FormButtonType_URL == eTmpButtonType)
                    {
                        // Label
                        aTmp = xPropSet->getPropertyValue( u"Label"_ustr );
                        OUString uTmp;
                        if( (aTmp >>= uTmp) && !uTmp.isEmpty())
                        {
                            rDescr = uTmp;
                        }

                        // util::URL
                        aTmp = xPropSet->getPropertyValue( u"TargetURL"_ustr );
                        if( (aTmp >>= uTmp) && !uTmp.isEmpty())
                        {
                            rURL = uTmp;
                        }
                        bRet = true;
                    }
                }
            }
        }
    }

    return bRet;
}

SwPostItMgr* SwWrtShell::GetPostItMgr()
{
    return m_rView.GetPostItMgr();
}

void SwWrtShell::SortChapters(const SwOutlineNodes::size_type nOutlineNodePos)
{
    auto sort_chapters = [this](const SwNode* pParentNode, int nOutlineLevel)
    {
        const SwNode* pEndNode;

        std::vector<const SwTextNode*> vLevelOutlineNodes;
        auto GetLevelOutlineNodesAndEndNode = [&]()
        {
            vLevelOutlineNodes.clear();
            bool bParentFound = false;
            pEndNode = &GetNodes().GetEndOfContent();
            for (const SwNode* pNode : GetNodes().GetOutLineNds())
            {
                if (!pNode->IsTextNode())
                    continue;
                if (pParentNode && !bParentFound)
                {
                    bParentFound = pNode == pParentNode;
                    continue;
                }
                if (pNode->GetTextNode()->GetAttrOutlineLevel() < nOutlineLevel)
                {
                    pEndNode = pNode;
                    break;
                }
                if (pNode->GetTextNode()->GetAttrOutlineLevel() == nOutlineLevel)
                    vLevelOutlineNodes.emplace_back(pNode->GetTextNode());
            }
        };

        GetLevelOutlineNodesAndEndNode();

        std::vector<const SwTextNode*> vSortedLevelOutlineNodes = vLevelOutlineNodes;
        std::stable_sort(vSortedLevelOutlineNodes.begin(), vSortedLevelOutlineNodes.end(),
                         [](const SwTextNode* a, const SwTextNode* b)
                         {
                             const OUString& raText = a->GetText();
                             const OUString& rbText = b->GetText();
                             return raText < rbText;
                         });

        for (size_t i = 0, nSize = vLevelOutlineNodes.size(); i < nSize; i++)
        {
            // Find the position that the sorted node is at in the unsorted vector.
            // This is the position of the node in the unsorted vector that is used for the start of
            // the range of nodes to be moved in this iteration.
            size_t j = 0;
            for (; j < nSize; j++)
            {
                if (vSortedLevelOutlineNodes[i] == vLevelOutlineNodes[j])
                    break;
            }

            // The end node in the range is the next entry in the unsorted vector or the pEndNode set
            // by GetLevelOutlineNodesAndEndNode or the end of the document.
            const SwNode* pEndRangeNode;
            if (j + 1 < nSize)
                pEndRangeNode = vLevelOutlineNodes[j + 1];
            else
                pEndRangeNode = pEndNode;

            SwNodeRange aNodeRange(*vLevelOutlineNodes[j], SwNodeOffset(0), *pEndRangeNode,
                                   SwNodeOffset(0));

            // Move the range of nodes to before the node in the unsorted outline vector at the
            // current iteration index to match the position of the outline node in the sorted vector.
            getIDocumentContentOperations().MoveNodeRange(
                aNodeRange, *const_cast<SwTextNode*>(vLevelOutlineNodes[i]),
                SwMoveFlags::DEFAULT | SwMoveFlags::CREATEUNDOOBJ);

            GetLevelOutlineNodesAndEndNode();
        }
    };

    const SwOutlineNodes& rOutlineNodes = GetNodes().GetOutLineNds();

    if (rOutlineNodes.empty())
        return;

    StartAction();
    StartUndo(SwUndoId::SORT_CHAPTERS);

    // Create an ordered set of outline levels in the outline nodes for use to determine
    // the lowest level to use for first sort and to only iterate over higher levels used.
    std::set<int> aOutlineLevelSet;
    for (const SwNode* pNode : rOutlineNodes)
    {
        int nOutlineLevel = pNode->GetTextNode()->GetAttrOutlineLevel();
        aOutlineLevelSet.emplace(nOutlineLevel);
    }

    // No parent node for the lowest outline level nodes sort.
    if (nOutlineNodePos == SwOutlineNodes::npos)
    {
        sort_chapters(nullptr /*pParentNode*/,
                      aOutlineLevelSet.extract(aOutlineLevelSet.begin()).value());
        for (int nOutlineLevel : aOutlineLevelSet)
        {
            for (size_t i = 0, nSize = rOutlineNodes.size(); i + 1 < nSize; i++)
            {
                if (rOutlineNodes[i]->GetTextNode()->GetAttrOutlineLevel() < nOutlineLevel
                    && rOutlineNodes[i + 1]->GetTextNode()->GetAttrOutlineLevel() == nOutlineLevel)
                {
                    const SwNode* pParentNode = rOutlineNodes[i];
                    sort_chapters(pParentNode, nOutlineLevel);
                }
            }
        }
    }
    else
    {
        for (int nOutlineLevel : aOutlineLevelSet)
        {
            if (rOutlineNodes[nOutlineNodePos]->GetTextNode()->GetAttrOutlineLevel()
                < nOutlineLevel)
            {
                const SwNode* pParentNode = rOutlineNodes[nOutlineNodePos];
                sort_chapters(pParentNode, nOutlineLevel);
            }
        }
    }

    EndUndo(SwUndoId::SORT_CHAPTERS);
    EndAction();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
