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

#include <cmdid.h>
#include <swtypes.hxx>
#include <unotools/confignode.hxx>
#include <comphelper/processfactory.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/htmlmode.hxx>
#include <viewopt.hxx>
#include <docsh.hxx>
#include <fldwrap.hxx>
#include <flddb.hxx>
#include <flddinf.hxx>
#include <fldvar.hxx>
#include <flddok.hxx>
#include <fldfunc.hxx>
#include <fldref.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <fldtdlg.hxx>
#include <swmodule.hxx>

#include <helpid.h>
#include <fldui.hrc>
#include <globals.hrc>

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>

// carrier of the dialog
SwFieldDlg::SwFieldDlg(SfxBindings* pB, SwChildWinWrapper* pCW, vcl::Window *pParent)
    : SfxTabDialog(pParent, "FieldDialog", "modules/swriter/ui/fielddialog.ui")
    , m_pChildWin(pCW)
    , m_pBindings(pB)
    , m_bDataBaseMode(false)
    , m_nDokId(0)
    , m_nVarId(0)
    , m_nDokInf(0)
    , m_nRefId(0)
    , m_nFuncId(0)
    , m_nDbId(0)
{
    SetStyle(GetStyle()|WB_STDMODELESS);
    m_bHtmlMode = (::GetHtmlMode(static_cast<SwDocShell*>(SfxObjectShell::Current())) & HTMLMODE_ON) != 0;

    GetCancelButton().SetClickHdl(LINK(this, SwFieldDlg, CancelHdl));

    GetOKButton().SetClickHdl(LINK(this, SwFieldDlg, OKHdl));

    m_nDokId = AddTabPage("document", SwFieldDokPage::Create, 0);
    m_nVarId = AddTabPage("variables", SwFieldVarPage::Create, 0);
    m_nDokInf = AddTabPage("docinfo", SwFieldDokInfPage::Create, 0);

    if (!m_bHtmlMode)
    {
        m_nRefId = AddTabPage("ref", SwFieldRefPage::Create, 0);
        m_nFuncId = AddTabPage("functions", SwFieldFuncPage::Create, 0);

        utl::OConfigurationTreeRoot aCfgRoot
            = utl::OConfigurationTreeRoot::createWithComponentContext(
                ::comphelper::getProcessComponentContext(),
                "/org.openoffice.Office.DataAccess/Policies/Features/Writer",
                -1,
                utl::OConfigurationTreeRoot::CM_READONLY);

        bool bDatabaseFields = true;
        aCfgRoot.getNodeValue(
            OUString("DatabaseFields")) >>= bDatabaseFields;

        if (bDatabaseFields)
            m_nDbId = AddTabPage("database", SwFieldDBPage::Create, 0);
        else
            RemoveTabPage("database");
    }
    else
    {
        RemoveTabPage("ref");
        RemoveTabPage("functions");
        RemoveTabPage("database");
    }
}

SwFieldDlg::~SwFieldDlg()
{
}

bool SwFieldDlg::Close()
{
    m_pBindings->GetDispatcher()->
        Execute(m_bDataBaseMode ? FN_INSERT_FIELD_DATA_ONLY : FN_INSERT_FIELD,
        SfxCallMode::ASYNCHRON|SfxCallMode::RECORD);
    return true;
}

void SwFieldDlg::Initialize(SfxChildWinInfo *pInfo)
{
    Point aPos;
    Size aSize;

    if ( pInfo->aSize.Width() != 0 && pInfo->aSize.Height() != 0 )
    {
        aPos = pInfo->aPos;
        if ( GetStyle() & WB_SIZEABLE )
            SetSizePixel( pInfo->aSize );

        // remember initial size from pInfo
        aSize = GetSizePixel();

        // should the FloatingWindow get zoomed?
        if ( pInfo->nFlags & SfxChildWindowFlags::ZOOMIN )
            RollUp();
    }
    else
    {
        // remember initial size from resource or ctor
        aSize = GetSizePixel();

        Size aParentSize = GetParent()->GetOutputSizePixel();
        aPos.X() += ( aParentSize.Width() - aSize.Width() ) / 2;
        aPos.Y() += ( aParentSize.Height() - aSize.Height() ) / 2;
    }

    Point aPoint;
    Rectangle aRect = GetDesktopRectPixel();
    aPoint.X() = aRect.Right() - aSize.Width();
    aPoint.Y() = aRect.Bottom() - aSize.Height();

    aPoint = OutputToScreenPixel( aPoint );

    if ( aPos.X() > aPoint.X() )
        aPos.X() = aPoint.X() ;
    if ( aPos.Y() > aPoint.Y() )
        aPos.Y() = aPoint.Y();

    if ( aPos.X() < 0 ) aPos.X() = 0;
    if ( aPos.Y() < 0 ) aPos.Y() = 0;

    SetPosPixel( aPos );
}

SfxItemSet* SwFieldDlg::CreateInputItemSet( sal_uInt16 nID  )
{
    if ( nID == m_nDokInf )
    {
        SwDocShell* pDocSh = static_cast<SwDocShell*>(SfxObjectShell::Current());
        SfxItemSet* pISet = new SfxItemSet( pDocSh->GetPool(), SID_DOCINFO, SID_DOCINFO );
        using namespace ::com::sun::star;
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
            pDocSh->GetModel(), uno::UNO_QUERY_THROW);
        uno::Reference<document::XDocumentProperties> xDocProps
            = xDPS->getDocumentProperties();
        uno::Reference< beans::XPropertySet > xUDProps(
            xDocProps->getUserDefinedProperties(),
            uno::UNO_QUERY_THROW);
        pISet->Put( SfxUnoAnyItem( SID_DOCINFO, uno::makeAny(xUDProps) ) );
        return pISet;
    }
    else
        return 0;
}

// kick off inserting of new fields
IMPL_LINK_NOARG_TYPED(SwFieldDlg, OKHdl, Button*, void)
{
    if (GetOKButton().IsEnabled())
    {
        SfxTabPage* pPage = GetTabPage(GetCurPageId());
        pPage->FillItemSet(0);

        GetOKButton().GrabFocus();  // because of InputField-Dlg
    }
}

IMPL_LINK_NOARG_TYPED(SwFieldDlg, CancelHdl, Button*, void)
{
    Close();
}

// newly initialise dialog after Doc-Switch
void SwFieldDlg::ReInitDlg()
{
    SwDocShell* pDocSh = static_cast<SwDocShell*>(SfxObjectShell::Current());
    bool bNewMode = (::GetHtmlMode(pDocSh) & HTMLMODE_ON) != 0;

    if (bNewMode != m_bHtmlMode)
    {
        SfxViewFrame::Current()->GetDispatcher()->
            Execute(FN_INSERT_FIELD, SfxCallMode::ASYNCHRON|SfxCallMode::RECORD);
        Close();
    }

    SwView* pActiveView = ::GetActiveView();
    if(!pActiveView)
        return;
    const SwWrtShell& rSh = pActiveView->GetWrtShell();
    GetOKButton().Enable( !rSh.IsReadOnlyAvailable() ||
                          !rSh.HasReadonlySel() );

    ReInitTabPage(m_nDokId);
    ReInitTabPage(m_nVarId);
    ReInitTabPage(m_nDokInf);

    if (!m_bHtmlMode)
    {
        ReInitTabPage(m_nRefId);
        ReInitTabPage(m_nFuncId);
        ReInitTabPage(m_nDbId);
    }

    m_pChildWin->SetOldDocShell(pDocSh);
}

// newly initialise TabPage after Doc-Switch
void SwFieldDlg::ReInitTabPage( sal_uInt16 nPageId, bool bOnlyActivate )
{
    SwFieldPage* pPage = static_cast<SwFieldPage* >(GetTabPage(nPageId));
    if ( pPage )
        pPage->EditNewField( bOnlyActivate );   // newly initialise TabPage
}

// newly initialise after activation of a few TabPages
void SwFieldDlg::Activate()
{
    SwView* pView = ::GetActiveView();
    if( pView )
    {
        bool bHtmlMode = (::GetHtmlMode(static_cast<SwDocShell*>(SfxObjectShell::Current())) & HTMLMODE_ON) != 0;
        const SwWrtShell& rSh = pView->GetWrtShell();
        GetOKButton().Enable( !rSh.IsReadOnlyAvailable() ||
                              !rSh.HasReadonlySel() );

        ReInitTabPage(m_nVarId, true);

        if( !bHtmlMode )
        {
            ReInitTabPage(m_nRefId, true);
            ReInitTabPage(m_nFuncId, true);
        }
    }
}

void SwFieldDlg::EnableInsert(bool bEnable)
{
    if( bEnable )
    {
        SwView* pView = ::GetActiveView();
        OSL_ENSURE(pView, "no view found");
        if( !pView ||
                (pView->GetWrtShell().IsReadOnlyAvailable() &&
                    pView->GetWrtShell().HasReadonlySel()) )
            bEnable = false;
    }
    GetOKButton().Enable(bEnable);
}

void SwFieldDlg::InsertHdl()
{
    GetOKButton().Click();
}

void SwFieldDlg::ActivateDatabasePage()
{
    m_bDataBaseMode = true;
    ShowPage(m_nDbId);
    SfxTabPage* pDBPage = GetTabPage(m_nDbId);
    if( pDBPage )
    {
        static_cast<SwFieldDBPage*>(pDBPage)->ActivateMailMergeAddress();
    }
    //remove all other pages
    RemoveTabPage("document");
    RemoveTabPage("variables");
    RemoveTabPage("docinfo");
    RemoveTabPage("ref");
    RemoveTabPage("functions");
}

void SwFieldDlg::ShowReferencePage()
{
    ShowPage(m_nRefId);
}

void SwFieldDlg::PageCreated(sal_uInt16 nId, SfxTabPage& rPage)
{
    if (nId == m_nDbId)
    {
        SfxDispatcher* pDispatch = m_pBindings->GetDispatcher();
        SfxViewFrame* pViewFrame = pDispatch ? pDispatch->GetFrame() : 0;
        if(pViewFrame)
        {
            SfxViewShell* pViewShell = SfxViewShell::GetFirst( true, checkSfxViewShell<SwView> );
            while(pViewShell && pViewShell->GetViewFrame() != pViewFrame)
            {
                pViewShell = SfxViewShell::GetNext( *pViewShell, true, checkSfxViewShell<SwView> );
            }
            if(pViewShell)
                static_cast<SwFieldDBPage&>(rPage).SetWrtShell(static_cast<SwView*>(pViewShell)->GetWrtShell());
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
