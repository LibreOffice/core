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

ScSimpleRefDlg::ScSimpleRefDlg(SfxBindings* pB, SfxChildWindow* pCW, vcl::Window* pParent)
    : ScAnyRefDlg(pB, pCW, pParent, "SimpleRefDialog", "modules/scalc/ui/simplerefdialog.ui")
,

        bAutoReOpen     ( true ),
        bCloseOnButtonUp( false ),
        bSingleCell     ( false ),
        bMultiSelection ( false )
{
    get(m_pFtAssign, "area");
    get(m_pEdAssign, "assign");
    m_pEdAssign->SetReferences(this, m_pFtAssign);
    get(m_pRbAssign, "assignref");
    m_pRbAssign->SetReferences(this, m_pEdAssign);

    get(m_pBtnOk, "ok");
    get(m_pBtnCancel, "cancel");

    // in order to keep the Strings with the FixedTexts in the resource:
    Init();
    SetDispatcherLock( true ); // activate modal mode
}

ScSimpleRefDlg::~ScSimpleRefDlg()
{
    disposeOnce();
}

void ScSimpleRefDlg::dispose()
{
    SetDispatcherLock( false ); // deactivate modal mode
    m_pFtAssign.clear();
    m_pEdAssign.clear();
    m_pRbAssign.clear();
    m_pBtnOk.clear();
    m_pBtnCancel.clear();
    ScAnyRefDlg::dispose();
}

void ScSimpleRefDlg::FillInfo(SfxChildWinInfo& rWinInfo) const
{
    ScAnyRefDlg::FillInfo(rWinInfo);
    rWinInfo.bVisible=bAutoReOpen;
}

void ScSimpleRefDlg::SetRefString(const OUString &rStr)
{
    m_pEdAssign->SetText(rStr);
}

void ScSimpleRefDlg::Init()
{
    m_pBtnOk->SetClickHdl      ( LINK( this, ScSimpleRefDlg, OkBtnHdl ) );
    m_pBtnCancel->SetClickHdl  ( LINK( this, ScSimpleRefDlg, CancelBtnHdl ) );
    bCloseFlag=false;
}

// Set the reference to a cell range selected with the mouse. This is then
// shown as the new selection in the reference field.
void ScSimpleRefDlg::SetReference( const ScRange& rRef, ScDocument* pDocP )
{
    if ( m_pEdAssign->IsEnabled() )
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart(m_pEdAssign);

        theCurArea = rRef;
        OUString aRefStr;
        if ( bSingleCell )
        {
            ScAddress aAdr = rRef.aStart;
            aRefStr = aAdr.Format(ScRefFlags::ADDR_ABS_3D, pDocP, pDocP->GetAddressConvention());
        }
        else
            aRefStr = theCurArea.Format(ScRefFlags::RANGE_ABS_3D, pDocP, pDocP->GetAddressConvention());

        if ( bMultiSelection )
        {
            OUString aVal = m_pEdAssign->GetText();
            Selection aSel = m_pEdAssign->GetSelection();
            aSel.Justify();
            aVal = aVal.replaceAt( aSel.Min(), aSel.Len(), aRefStr );
            Selection aNewSel( aSel.Min(), aSel.Min()+aRefStr.getLength() );
            m_pEdAssign->SetRefString( aVal );
            m_pEdAssign->SetSelection( aNewSel );
        }
        else
            m_pEdAssign->SetRefString( aRefStr );

        aChangeHdl.Call( aRefStr );
    }
}

bool ScSimpleRefDlg::Close()
{
    CancelBtnHdl(m_pBtnCancel);
    return true;
}

void ScSimpleRefDlg::SetActive()
{
    m_pEdAssign->GrabFocus();

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
        m_pEdAssign->SetSelection( Selection( 0, m_pEdAssign->GetText().getLength() ) );
    }

    m_pRbAssign->DoRef();
    bCloseFlag = true;
}

void ScSimpleRefDlg::RefInputDone( bool bForced)
{
    ScAnyRefDlg::RefInputDone(bForced);
    if ( (bForced || bCloseOnButtonUp) && bCloseFlag )
        OkBtnHdl(m_pBtnOk);
}

// Handler:

IMPL_LINK_NOARG(ScSimpleRefDlg, OkBtnHdl, Button*, void)
{
    bAutoReOpen=false;
    OUString aResult=m_pEdAssign->GetText();
    aCloseHdl.Call(&aResult);
    Link<const OUString&,void> aUnoLink = aDoneHdl;     // stack var because this is deleted in DoClose
    DoClose( ScSimpleRefDlgWrapper::GetChildWindowId() );
    aUnoLink.Call( aResult );
}

IMPL_LINK_NOARG(ScSimpleRefDlg, CancelBtnHdl, Button*, void)
{
    bAutoReOpen=false;
    OUString aResult=m_pEdAssign->GetText();
    aCloseHdl.Call(nullptr);
    Link<const OUString&,void> aUnoLink = aAbortedHdl;  // stack var because this is deleted in DoClose
    DoClose( ScSimpleRefDlgWrapper::GetChildWindowId() );
    aUnoLink.Call( aResult );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
