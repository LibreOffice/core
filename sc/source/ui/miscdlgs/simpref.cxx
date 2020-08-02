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

#include <reffact.hxx>
#include <document.hxx>
#include <simpref.hxx>

ScSimpleRefDlg::ScSimpleRefDlg(SfxBindings* pB, SfxChildWindow* pCW, weld::Window* pParent)
    : ScAnyRefDlgController(pB, pCW, pParent, "modules/scalc/ui/simplerefdialog.ui", "SimpleRefDialog")
    , bAutoReOpen(true)
    , bCloseOnButtonUp(false)
    , bSingleCell(false)
    , bMultiSelection(false)
    , m_xFtAssign(m_xBuilder->weld_label("area"))
    , m_xEdAssign(new formula::RefEdit(m_xBuilder->weld_entry("assign")))
    , m_xRbAssign(new formula::RefButton(m_xBuilder->weld_button("assignref")))
    , m_xBtnOk(m_xBuilder->weld_button("ok"))
    , m_xBtnCancel(m_xBuilder->weld_button("cancel"))
{
    m_xEdAssign->SetReferences(this, m_xFtAssign.get());
    m_xRbAssign->SetReferences(this, m_xEdAssign.get());

    // in order to keep the Strings with the FixedTexts in the resource:
    Init();
    SetDispatcherLock( true ); // activate modal mode
}

ScSimpleRefDlg::~ScSimpleRefDlg()
{
    SetDispatcherLock( false ); // deactivate modal mode
}

void ScSimpleRefDlg::FillInfo(SfxChildWinInfo& rWinInfo) const
{
    ScAnyRefDlgController::FillInfo(rWinInfo);
    rWinInfo.bVisible = bAutoReOpen;
}

void ScSimpleRefDlg::SetRefString(const OUString &rStr)
{
    m_xEdAssign->SetText(rStr);
}

void ScSimpleRefDlg::Init()
{
    m_xBtnOk->connect_clicked( LINK( this, ScSimpleRefDlg, OkBtnHdl ) );
    m_xBtnCancel->connect_clicked( LINK( this, ScSimpleRefDlg, CancelBtnHdl ) );
    bCloseFlag=false;
}

// Set the reference to a cell range selected with the mouse. This is then
// shown as the new selection in the reference field.
void ScSimpleRefDlg::SetReference( const ScRange& rRef, ScDocument& rDocP )
{
    if (!m_xEdAssign->GetWidget()->get_sensitive())
        return;

    if ( rRef.aStart != rRef.aEnd )
        RefInputStart(m_xEdAssign.get());

    theCurArea = rRef;
    OUString aRefStr;
    if ( bSingleCell )
    {
        ScAddress aAdr = rRef.aStart;
        aRefStr = aAdr.Format(ScRefFlags::ADDR_ABS_3D, &rDocP, rDocP.GetAddressConvention());
    }
    else
        aRefStr = theCurArea.Format(rDocP, ScRefFlags::RANGE_ABS_3D, rDocP.GetAddressConvention());

    if ( bMultiSelection )
    {
        OUString aVal = m_xEdAssign->GetText();
        Selection aSel = m_xEdAssign->GetSelection();
        aSel.Justify();
        aVal = aVal.replaceAt( aSel.Min(), aSel.Len(), aRefStr );
        Selection aNewSel( aSel.Min(), aSel.Min()+aRefStr.getLength() );
        m_xEdAssign->SetRefString( aVal );
        m_xEdAssign->SetSelection( aNewSel );
    }
    else
        m_xEdAssign->SetRefString( aRefStr );

    aChangeHdl.Call( aRefStr );
}

void ScSimpleRefDlg::Close()
{
    CancelBtnHdl(*m_xBtnCancel);
}

void ScSimpleRefDlg::SetActive()
{
    m_xEdAssign->GrabFocus();

    //  no NameModifyHdl.  Otherwise ranges could not be altered
    //  (after marking the reference, the old field content would be shown)
    //  (also, the selected DB name has also not been altered)

    RefInputDone();
}

bool ScSimpleRefDlg::IsRefInputMode() const
{
    return true;
}

void ScSimpleRefDlg::SetCloseHdl( const Link<const OUString*,void>& rLink )
{
    aCloseHdl=rLink;
}

void ScSimpleRefDlg::SetUnoLinks( const Link<const OUString&,void>& rDone,
                                  const Link<const OUString&,void>& rAbort,
                                  const Link<const OUString&,void>& rChange )
{
    aDoneHdl    = rDone;
    aAbortedHdl = rAbort;
    aChangeHdl  = rChange;
}

void ScSimpleRefDlg::SetFlags( bool bSetCloseOnButtonUp, bool bSetSingleCell, bool bSetMultiSelection )
{
    bCloseOnButtonUp = bSetCloseOnButtonUp;
    bSingleCell = bSetSingleCell;
    bMultiSelection = bSetMultiSelection;
}

void ScSimpleRefDlg::StartRefInput()
{
    if ( bMultiSelection )
    {
        // initially select the whole string, so it gets replaced by default
        m_xEdAssign->SelectAll();
    }

    m_xRbAssign->DoRef();
    bCloseFlag = true;
}

void ScSimpleRefDlg::RefInputDone( bool bForced)
{
    ScAnyRefDlgController::RefInputDone(bForced);
    if ( (bForced || bCloseOnButtonUp) && bCloseFlag )
        OkBtnHdl(*m_xBtnOk);
}

// Handler:

IMPL_LINK_NOARG(ScSimpleRefDlg, OkBtnHdl, weld::Button&, void)
{
    if (IsClosing())
        return;
    bAutoReOpen = false;
    OUString aResult=m_xEdAssign->GetText();
    aCloseHdl.Call(&aResult);
    Link<const OUString&,void> aUnoLink = aDoneHdl;     // stack var because this is deleted in DoClose
    DoClose( ScSimpleRefDlgWrapper::GetChildWindowId() );
    aUnoLink.Call( aResult );
}

IMPL_LINK_NOARG(ScSimpleRefDlg, CancelBtnHdl, weld::Button&, void)
{
    if (IsClosing())
        return;
    bAutoReOpen = false;
    OUString aResult=m_xEdAssign->GetText();
    aCloseHdl.Call(nullptr);
    Link<const OUString&,void> aUnoLink = aAbortedHdl;  // stack var because this is deleted in DoClose
    DoClose( ScSimpleRefDlgWrapper::GetChildWindowId() );
    aUnoLink.Call( aResult );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
