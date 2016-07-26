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

#undef SC_DLLIMPLEMENTATION

#include "scitems.hxx"
#include <comphelper/string.hxx>
#include <svx/drawitem.hxx>
#include <svx/xtable.hxx>
#include <sfx2/objsh.hxx>
#include <unotools/useroptions.hxx>
#include <vcl/msgbox.hxx>
#include <unotools/localedatawrapper.hxx>

#include "global.hxx"
#include "globstr.hrc"
#include "tabvwsh.hxx"
#include "viewdata.hxx"
#include "document.hxx"
#include "scresid.hxx"
#include "scendlg.hxx"

ScNewScenarioDlg::ScNewScenarioDlg( vcl::Window* pParent, const OUString& rName, bool bEdit, bool bSheetProtected)

    : ModalDialog(pParent, "ScenarioDialog",
        "modules/scalc/ui/scenariodialog.ui")
    , aDefScenarioName(rName)
    , bIsEdit(bEdit)
{
    get(m_pEdName, "name");
    get(m_pEdComment, "comment");
    Size aSize(m_pEdComment->LogicToPixel(Size(183, 46), MAP_APPFONT));
    m_pEdComment->set_width_request(aSize.Width());
    m_pEdComment->set_height_request(aSize.Height());
    get(m_pCbShowFrame, "showframe");
    get(m_pLbColor, "bordercolor");
    get(m_pCbTwoWay, "copyback");
    get(m_pCbCopyAll, "copysheet");
    get(m_pCbProtect, "preventchanges");
    get(m_pBtnOk, "ok");

    if (bIsEdit)
        SetText(get<FixedText>("alttitle")->GetText());

    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    if ( pDocSh )
    {
        const SfxPoolItem* pItem = pDocSh->GetItem( SID_COLOR_TABLE );
        if ( pItem )
        {
            XColorListRef pColorList = static_cast<const SvxColorListItem*>(pItem)->GetColorList();
            if (pColorList.is())
            {
                m_pLbColor->SetUpdateMode( false );
                long nCount = pColorList->Count();
                for ( long n=0; n<nCount; n++ )
                {
                    XColorEntry* pEntry = pColorList->GetColor(n);
                    m_pLbColor->InsertEntry( pEntry->GetColor(), pEntry->GetName() );
                }
                m_pLbColor->SetUpdateMode( true );
            }
        }
    }

    SvtUserOptions aUserOpt;

    OUString sCreatedBy(get<FixedText>("createdft")->GetText());
    OUString sOn(get<FixedText>("onft")->GetText());

    OUString aComment(sCreatedBy + " " + aUserOpt.GetFirstName() + " " +aUserOpt.GetLastName()
              + ", " + sOn + " " + ScGlobal::GetpLocaleData()->getDate(Date(Date::SYSTEM))
              + ", " + ScGlobal::GetpLocaleData()->getTime(tools::Time(tools::Time::SYSTEM)));

    m_pEdComment->SetText(aComment);
    m_pEdName->SetText(rName);
    m_pBtnOk->SetClickHdl( LINK( this, ScNewScenarioDlg, OkHdl ) );
    m_pCbShowFrame->SetClickHdl( LINK( this, ScNewScenarioDlg, EnableHdl ) );

    m_pLbColor->SelectEntry( Color( COL_LIGHTGRAY ) );
    m_pCbShowFrame->Check();
    m_pCbTwoWay->Check();
    m_pCbCopyAll->Check(false);
    m_pCbProtect->Check();

    if (bIsEdit)
        m_pCbCopyAll->Enable(false);
    // If the Sheet is protected then we disable the Scenario Protect input
    // and default it to true above. Note we are in 'Add' mode here as: if
    // Sheet && scenario protection are true, then we cannot edit this dialog.
    if (bSheetProtected)
        m_pCbProtect->Enable(false);
}

ScNewScenarioDlg::~ScNewScenarioDlg()
{
    disposeOnce();
}

void ScNewScenarioDlg::dispose()
{
    m_pEdName.clear();
    m_pEdComment.clear();
    m_pCbShowFrame.clear();
    m_pLbColor.clear();
    m_pCbTwoWay.clear();
    m_pCbCopyAll.clear();
    m_pCbProtect.clear();
    m_pBtnOk.clear();
    ModalDialog::dispose();
}

void ScNewScenarioDlg::GetScenarioData( OUString& rName, OUString& rComment,
                                        Color& rColor, sal_uInt16& rFlags ) const
{
    rComment = m_pEdComment->GetText();
    rName    = m_pEdName->GetText();

    if (rName.isEmpty())
        rName = aDefScenarioName;

    rColor = m_pLbColor->GetSelectEntryColor();
    sal_uInt16 nBits = 0;
    if (m_pCbShowFrame->IsChecked())
        nBits |= SC_SCENARIO_SHOWFRAME;
    if (m_pCbTwoWay->IsChecked())
        nBits |= SC_SCENARIO_TWOWAY;
    if (m_pCbCopyAll->IsChecked())
        nBits |= SC_SCENARIO_COPYALL;
    if (m_pCbProtect->IsChecked())
        nBits |= SC_SCENARIO_PROTECT;
    rFlags = nBits;
}

void ScNewScenarioDlg::SetScenarioData( const OUString& rName, const OUString& rComment,
                                        const Color& rColor, sal_uInt16 nFlags )
{
    m_pEdComment->SetText(rComment);
    m_pEdName->SetText(rName);
    m_pLbColor->SelectEntry(rColor);

    m_pCbShowFrame->Check ( (nFlags & SC_SCENARIO_SHOWFRAME)  != 0 );
    EnableHdl(m_pCbShowFrame);
    m_pCbTwoWay->Check    ( (nFlags & SC_SCENARIO_TWOWAY)     != 0 );
    //  CopyAll nicht
    m_pCbProtect->Check    ( (nFlags & SC_SCENARIO_PROTECT)     != 0 );
}

IMPL_LINK_NOARG_TYPED(ScNewScenarioDlg, OkHdl, Button*, void)
{
    OUString      aName = comphelper::string::strip(m_pEdName->GetText(), ' ');
    ScDocument* pDoc    = static_cast<ScTabViewShell*>(SfxViewShell::Current())->GetViewData().GetDocument();

    m_pEdName->SetText( aName );

    if ( !ScDocument::ValidTabName( aName ) )
    {
        ScopedVclPtrInstance<InfoBox>(this, ScGlobal::GetRscString(STR_INVALIDTABNAME))->Execute();
        m_pEdName->GrabFocus();
    }
    else if ( !bIsEdit && !pDoc->ValidNewTabName( aName ) )
    {
        ScopedVclPtrInstance<InfoBox>(this, ScGlobal::GetRscString(STR_NEWTABNAMENOTUNIQUE))->Execute();
        m_pEdName->GrabFocus();
    }
    else
        EndDialog( RET_OK );

    //! beim Editieren testen, ob eine andere Tabelle den Namen hat!
}

IMPL_LINK_TYPED( ScNewScenarioDlg, EnableHdl, Button*, pBox, void )
{
    if (pBox == m_pCbShowFrame)
        m_pLbColor->Enable( m_pCbShowFrame->IsChecked() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
