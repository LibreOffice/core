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
#include <docsh.hxx>
#include <chgtrack.hxx>

#include <highred.hxx>


ScHighlightChgDlg::ScHighlightChgDlg(SfxBindings* pB, SfxChildWindow* pCW, weld::Window* pParent,
                                     ScViewData* ptrViewData)
    : ScAnyRefDlgController(pB, pCW, pParent, "modules/scalc/ui/showchangesdialog.ui", "ShowChangesDialog")
    , pViewData(ptrViewData)
    , rDoc(ptrViewData->GetDocument())
    , m_xHighlightBox(m_xBuilder->weld_check_button("showchanges"))
    , m_xCbAccept(m_xBuilder->weld_check_button("showaccepted"))
    , m_xCbReject(m_xBuilder->weld_check_button("showrejected"))
    , m_xOkButton(m_xBuilder->weld_button("ok"))
    , m_xEdAssign(new formula::RefEdit(m_xBuilder->weld_entry("range")))
    , m_xRbAssign(new formula::RefButton(m_xBuilder->weld_button("rangeref")))
    , m_xBox(m_xBuilder->weld_container("box"))
    , m_xFilterCtr(new SvxTPFilter(m_xBox.get()))
{
    m_xEdAssign->SetReferences(this, nullptr);
    m_xRbAssign->SetReferences(this, m_xEdAssign.get());

    m_xOkButton->connect_clicked(LINK( this, ScHighlightChgDlg, OKBtnHdl));
    m_xHighlightBox->connect_clicked(LINK( this, ScHighlightChgDlg, HighlightHandle ));
    m_xFilterCtr->SetRefHdl(LINK( this, ScHighlightChgDlg, RefHandle ));
    m_xFilterCtr->HideRange(false);
    m_xFilterCtr->Show();
    SetDispatcherLock( true );

    Init();
}

ScHighlightChgDlg::~ScHighlightChgDlg()
{
    SetDispatcherLock( false );
}

void ScHighlightChgDlg::Init()
{
    OSL_ENSURE( pViewData, "ViewData or Document not found!" );

    ScChangeTrack* pChanges = rDoc.GetChangeTrack();
    if(pChanges!=nullptr)
    {
        aChangeViewSet.SetTheAuthorToShow(pChanges->GetUser());
        m_xFilterCtr->ClearAuthors();
        const std::set<OUString>& rUserColl = pChanges->GetUserCollection();
        for (const auto& rItem : rUserColl)
            m_xFilterCtr->InsertAuthor(rItem);
    }

    ScChangeViewSettings* pViewSettings = rDoc.GetChangeViewSettings();

    if(pViewSettings!=nullptr)
        aChangeViewSet=*pViewSettings;
    m_xHighlightBox->set_active(aChangeViewSet.ShowChanges());
    m_xFilterCtr->CheckDate(aChangeViewSet.HasDate());

    DateTime aEmpty(DateTime::EMPTY);

    DateTime aDateTime(aChangeViewSet.GetTheFirstDateTime());
    if (aDateTime != aEmpty)
    {
        m_xFilterCtr->SetFirstDate(aDateTime);
        m_xFilterCtr->SetFirstTime(aDateTime);
    }
    aDateTime = aChangeViewSet.GetTheLastDateTime();
    if (aDateTime != aEmpty)
    {
        m_xFilterCtr->SetLastDate(aDateTime);
        m_xFilterCtr->SetLastTime(aDateTime);
    }

    m_xFilterCtr->SetDateMode(static_cast<sal_uInt16>(aChangeViewSet.GetTheDateMode()));
    m_xFilterCtr->CheckAuthor(aChangeViewSet.HasAuthor());
    m_xFilterCtr->CheckComment(aChangeViewSet.HasComment());
    m_xFilterCtr->SetComment(aChangeViewSet.GetTheComment());

    m_xCbAccept->set_active(aChangeViewSet.IsShowAccepted());
    m_xCbReject->set_active(aChangeViewSet.IsShowRejected());

    OUString aString=aChangeViewSet.GetTheAuthorToShow();
    if(!aString.isEmpty())
    {
        m_xFilterCtr->SelectAuthor(aString);
    }
    else
    {
        m_xFilterCtr->SelectedAuthorPos(0);
    }

    m_xFilterCtr->CheckRange(aChangeViewSet.HasRange());

    if ( !aChangeViewSet.GetTheRangeList().empty() )
    {
        const ScRange & rRangeEntry = aChangeViewSet.GetTheRangeList().front();
        OUString aRefStr(rRangeEntry.Format(rDoc, ScRefFlags::RANGE_ABS_3D));
        m_xFilterCtr->SetRange(aRefStr);
    }
    m_xFilterCtr->Enable(true);
    HighlightHandle(*m_xHighlightBox);
}

// Set the reference to a cell range selected with the mouse. This is then
// shown as the new selection in the reference field.
void ScHighlightChgDlg::SetReference( const ScRange& rRef, ScDocument& rDocP )
{
    if (m_xEdAssign->GetWidget()->get_visible())
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart(m_xEdAssign.get());
        OUString aRefStr(rRef.Format(rDocP, ScRefFlags::RANGE_ABS_3D, rDocP.GetAddressConvention()));
        m_xEdAssign->SetRefString( aRefStr );
        m_xFilterCtr->SetRange(aRefStr);
    }
}

void ScHighlightChgDlg::Close()
{
    DoClose( ScHighlightChgDlgWrapper::GetChildWindowId() );
}

void ScHighlightChgDlg::RefInputDone( bool bForced)
{
    ScAnyRefDlgController::RefInputDone(bForced);
    if (bForced || !m_xRbAssign->GetWidget()->get_visible())
    {
        m_xFilterCtr->SetRange(m_xEdAssign->GetText());
        m_xFilterCtr->SetFocusToRange();
        m_xEdAssign->GetWidget()->hide();
        m_xRbAssign->GetWidget()->hide();
    }
}

void ScHighlightChgDlg::SetActive()
{
}

bool ScHighlightChgDlg::IsRefInputMode() const
{
    return m_xEdAssign->GetWidget()->get_visible();
}

IMPL_LINK_NOARG(ScHighlightChgDlg, HighlightHandle, weld::Button&, void)
{
    if (m_xHighlightBox->get_active())
    {
        m_xFilterCtr->Enable(true);
        m_xCbAccept->set_sensitive(true);
        m_xCbReject->set_sensitive(true);
    }
    else
    {
        m_xFilterCtr->Enable(false);
        m_xCbAccept->set_sensitive(false);
        m_xCbReject->set_sensitive(false);
    }
}

IMPL_LINK( ScHighlightChgDlg, RefHandle, SvxTPFilter*, pRef, void )
{
    if(pRef!=nullptr)
    {
        SetDispatcherLock( true );
        m_xEdAssign->GetWidget()->show();
        m_xRbAssign->GetWidget()->show();
        m_xEdAssign->SetText(m_xFilterCtr->GetRange());
        m_xEdAssign->GrabFocus();
        ScAnyRefDlgController::RefInputStart(m_xEdAssign.get(), m_xRbAssign.get());
    }
}

IMPL_LINK_NOARG(ScHighlightChgDlg, OKBtnHdl, weld::Button&, void)
{
    aChangeViewSet.SetShowChanges(m_xHighlightBox->get_active());
    aChangeViewSet.SetHasDate(m_xFilterCtr->IsDate());
    SvxRedlinDateMode eMode = m_xFilterCtr->GetDateMode();
    aChangeViewSet.SetTheDateMode( eMode );
    Date aFirstDate( m_xFilterCtr->GetFirstDate() );
    tools::Time aFirstTime( m_xFilterCtr->GetFirstTime() );
    Date aLastDate( m_xFilterCtr->GetLastDate() );
    tools::Time aLastTime( m_xFilterCtr->GetLastTime() );
    aChangeViewSet.SetTheFirstDateTime( DateTime( aFirstDate, aFirstTime ) );
    aChangeViewSet.SetTheLastDateTime( DateTime( aLastDate, aLastTime ) );
    aChangeViewSet.SetHasAuthor(m_xFilterCtr->IsAuthor());
    aChangeViewSet.SetTheAuthorToShow(m_xFilterCtr->GetSelectedAuthor());
    aChangeViewSet.SetHasRange(m_xFilterCtr->IsRange());
    aChangeViewSet.SetShowAccepted(m_xCbAccept->get_active());
    aChangeViewSet.SetShowRejected(m_xCbReject->get_active());
    aChangeViewSet.SetHasComment(m_xFilterCtr->IsComment());
    aChangeViewSet.SetTheComment(m_xFilterCtr->GetComment());
    ScRangeList aLocalRangeList;
    aLocalRangeList.Parse(m_xFilterCtr->GetRange(), rDoc);
    aChangeViewSet.SetTheRangeList(aLocalRangeList);
    aChangeViewSet.AdjustDateMode( rDoc );
    rDoc.SetChangeViewSettings(aChangeViewSet);
    pViewData->GetDocShell()->PostPaintGridAll();
    response(RET_OK);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
