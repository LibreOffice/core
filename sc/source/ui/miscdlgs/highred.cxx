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

//  class ScHighlightChgDlg

ScHighlightChgDlg::ScHighlightChgDlg( SfxBindings* pB, SfxChildWindow* pCW, vcl::Window* pParent,
                      ScViewData*       ptrViewData)
    : ScAnyRefDlg(pB, pCW, pParent, "ShowChangesDialog",
        "modules/scalc/ui/showchangesdialog.ui")
    , pViewData(ptrViewData)
    , pDoc(ptrViewData->GetDocument())
{
    m_pFilterCtr = VclPtr<SvxTPFilter>::Create(get<VclContainer>("box"));
    get(m_pHighlightBox, "showchanges");
    get(m_pCbAccept, "showaccepted");
    get(m_pCbReject, "showrejected");
    get(m_pEdAssign, "range");
    m_pEdAssign->SetReferences(this, m_pFilterCtr->get<vcl::Window>("range"));
    m_pEdAssign->SetSizePixel(m_pEdAssign->get_preferred_size());
    get(m_pRbAssign, "rangeref");
    m_pRbAssign->SetReferences(this, m_pEdAssign);
    get(m_pOkButton, "ok");

    m_pOkButton->SetClickHdl(LINK( this, ScHighlightChgDlg, OKBtnHdl));
    m_pHighlightBox->SetClickHdl(LINK( this, ScHighlightChgDlg, HighlightHandle ));
    m_pFilterCtr->SetRefHdl(LINK( this, ScHighlightChgDlg, RefHandle ));
    m_pFilterCtr->HideRange(false);
    m_pFilterCtr->Show();
    SetDispatcherLock( true );

    Init();
}

ScHighlightChgDlg::~ScHighlightChgDlg()
{
    disposeOnce();
}

void ScHighlightChgDlg::dispose()
{
    SetDispatcherLock( false );
    m_pFilterCtr.disposeAndClear();
    m_pHighlightBox.clear();
    m_pCbAccept.clear();
    m_pCbReject.clear();
    m_pOkButton.clear();
    m_pEdAssign.clear();
    m_pRbAssign.clear();
    ScAnyRefDlg::dispose();
}

void ScHighlightChgDlg::Init()
{
    OSL_ENSURE( pViewData && pDoc, "ViewData or Document not found!" );

    ScChangeTrack* pChanges=pDoc->GetChangeTrack();
    if(pChanges!=nullptr)
    {
        aChangeViewSet.SetTheAuthorToShow(pChanges->GetUser());
        m_pFilterCtr->ClearAuthors();
        const std::set<OUString>& rUserColl = pChanges->GetUserCollection();
        for (const auto& rItem : rUserColl)
            m_pFilterCtr->InsertAuthor(rItem);
    }

    ScChangeViewSettings* pViewSettings=pDoc->GetChangeViewSettings();

    if(pViewSettings!=nullptr)
        aChangeViewSet=*pViewSettings;
    m_pHighlightBox->Check(aChangeViewSet.ShowChanges());
    m_pFilterCtr->CheckDate(aChangeViewSet.HasDate());
    m_pFilterCtr->SetFirstDate(aChangeViewSet.GetTheFirstDateTime());
    m_pFilterCtr->SetFirstTime(aChangeViewSet.GetTheFirstDateTime());
    m_pFilterCtr->SetLastDate(aChangeViewSet.GetTheLastDateTime());
    m_pFilterCtr->SetLastTime(aChangeViewSet.GetTheLastDateTime());
    m_pFilterCtr->SetDateMode(static_cast<sal_uInt16>(aChangeViewSet.GetTheDateMode()));
    m_pFilterCtr->CheckAuthor(aChangeViewSet.HasAuthor());
    m_pFilterCtr->CheckComment(aChangeViewSet.HasComment());
    m_pFilterCtr->SetComment(aChangeViewSet.GetTheComment());

    m_pCbAccept->Check(aChangeViewSet.IsShowAccepted());
    m_pCbReject->Check(aChangeViewSet.IsShowRejected());

    OUString aString=aChangeViewSet.GetTheAuthorToShow();
    if(!aString.isEmpty())
    {
        m_pFilterCtr->SelectAuthor(aString);
    }
    else
    {
        m_pFilterCtr->SelectedAuthorPos(0);
    }

    m_pFilterCtr->CheckRange(aChangeViewSet.HasRange());

    if ( !aChangeViewSet.GetTheRangeList().empty() )
    {
        const ScRange & rRangeEntry = aChangeViewSet.GetTheRangeList().front();
        OUString aRefStr(rRangeEntry.Format(ScRefFlags::RANGE_ABS_3D, pDoc));
        m_pFilterCtr->SetRange(aRefStr);
    }
    m_pFilterCtr->Enable();
    HighlightHandle(m_pHighlightBox);
}

// Set the reference to a cell range selected with the mouse. This is then
// shown as the new selection in the reference field.

void ScHighlightChgDlg::SetReference( const ScRange& rRef, ScDocument* pDocP )
{
    if ( m_pEdAssign->IsVisible() )
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart(m_pEdAssign);
        OUString aRefStr(rRef.Format(ScRefFlags::RANGE_ABS_3D, pDocP, pDocP->GetAddressConvention()));
        m_pEdAssign->SetRefString( aRefStr );
        m_pFilterCtr->SetRange(aRefStr);
    }
}

bool ScHighlightChgDlg::Close()
{
    return DoClose( ScHighlightChgDlgWrapper::GetChildWindowId() );
}

void ScHighlightChgDlg::RefInputDone( bool bForced)
{
    ScAnyRefDlg::RefInputDone(bForced);
    if(bForced || !m_pRbAssign->IsVisible())
    {
        m_pFilterCtr->SetRange(m_pEdAssign->GetText());
        m_pFilterCtr->SetFocusToRange();
        m_pEdAssign->Hide();
        m_pRbAssign->Hide();
    }
}

void ScHighlightChgDlg::SetActive()
{
}

bool ScHighlightChgDlg::IsRefInputMode() const
{
    return m_pEdAssign->IsVisible();
}

IMPL_LINK( ScHighlightChgDlg, HighlightHandle, Button*, pCb, void )
{
    if(pCb!=nullptr)
    {
        if(m_pHighlightBox->IsChecked())
        {
            m_pFilterCtr->Enable();
            m_pCbAccept->Enable();
            m_pCbReject->Enable();
        }
        else
        {
            m_pFilterCtr->Disable();
            m_pCbAccept->Disable();
            m_pCbReject->Disable();
        }
    }
}

IMPL_LINK( ScHighlightChgDlg, RefHandle, SvxTPFilter*, pRef, void )
{
    if(pRef!=nullptr)
    {
        SetDispatcherLock( true );
        m_pEdAssign->Show();
        m_pRbAssign->Show();
        m_pEdAssign->SetText(m_pFilterCtr->GetRange());
        m_pEdAssign->GrabFocus();
        ScAnyRefDlg::RefInputStart(m_pEdAssign, m_pRbAssign);
    }
}

IMPL_LINK( ScHighlightChgDlg, OKBtnHdl, Button*, pOKBtn, void )
{
    if (pOKBtn == m_pOkButton)
    {
        aChangeViewSet.SetShowChanges(m_pHighlightBox->IsChecked());
        aChangeViewSet.SetHasDate(m_pFilterCtr->IsDate());
        SvxRedlinDateMode eMode = m_pFilterCtr->GetDateMode();
        aChangeViewSet.SetTheDateMode( eMode );
        Date aFirstDate( m_pFilterCtr->GetFirstDate() );
        tools::Time aFirstTime( m_pFilterCtr->GetFirstTime() );
        Date aLastDate( m_pFilterCtr->GetLastDate() );
        tools::Time aLastTime( m_pFilterCtr->GetLastTime() );
        aChangeViewSet.SetTheFirstDateTime( DateTime( aFirstDate, aFirstTime ) );
        aChangeViewSet.SetTheLastDateTime( DateTime( aLastDate, aLastTime ) );
        aChangeViewSet.SetHasAuthor(m_pFilterCtr->IsAuthor());
        aChangeViewSet.SetTheAuthorToShow(m_pFilterCtr->GetSelectedAuthor());
        aChangeViewSet.SetHasRange(m_pFilterCtr->IsRange());
        aChangeViewSet.SetShowAccepted(m_pCbAccept->IsChecked());
        aChangeViewSet.SetShowRejected(m_pCbReject->IsChecked());
        aChangeViewSet.SetHasComment(m_pFilterCtr->IsComment());
        aChangeViewSet.SetTheComment(m_pFilterCtr->GetComment());
        ScRangeList aLocalRangeList;
        aLocalRangeList.Parse(m_pFilterCtr->GetRange(), pDoc);
        aChangeViewSet.SetTheRangeList(aLocalRangeList);
        aChangeViewSet.AdjustDateMode( *pDoc );
        pDoc->SetChangeViewSettings(aChangeViewSet);
        pViewData->GetDocShell()->PostPaintGridAll();
        Close();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
