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

using namespace ::com::sun::star;

bool SwWrtShell::MoveBookMark( BookMarkMove eFuncId, const ::sw::mark::IMark* const pMark)
{
    addCurrentPosition();
    (this->*m_fnKillSel)( nullptr, false );

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
    (this->*m_fnKillSel)( nullptr, false );

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
    std::shared_ptr<SwContentControl> pContentControl = rContentControl.GetContentControl();
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
                GetSfxViewShell()->libreOfficeKitViewCallback(LOK_CALLBACK_CONTENT_CONTROL,
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

    (this->*m_fnKillSel)(nullptr, false);

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

bool SwWrtShell::GotoFieldmark(::sw::mark::IFieldmark const * const pMark)
{
    (this->*m_fnKillSel)( nullptr, false );
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

void SwWrtShell::GotoMark( const OUString& rName )
{
    IDocumentMarkAccess::const_iterator_t ppMark = getIDocumentMarkAccess()->findMark( rName );
    if (ppMark == getIDocumentMarkAccess()->getAllMarksEnd())
        return;
    MoveBookMark( BOOKMARK_INDEX, *ppMark );
}

void SwWrtShell::GotoMark( const ::sw::mark::IMark* const pMark )
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
                if(xInfo->hasPropertyByName( "ButtonType" ))
                {
                    aTmp = xPropSet->getPropertyValue( "ButtonType" );
                    form::FormButtonType eTmpButtonType;
                    aTmp >>= eTmpButtonType;
                    if( form::FormButtonType_URL == eTmpButtonType)
                    {
                        // Label
                        aTmp = xPropSet->getPropertyValue( "Label" );
                        OUString uTmp;
                        if( (aTmp >>= uTmp) && !uTmp.isEmpty())
                        {
                            rDescr = uTmp;
                        }

                        // util::URL
                        aTmp = xPropSet->getPropertyValue( "TargetURL" );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
