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


/*--------------------------------------------------------------------
    Description:    carrier of the dialog
 --------------------------------------------------------------------*/


SwFldDlg::SwFldDlg(SfxBindings* pB, SwChildWinWrapper* pCW, Window *pParent)
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
    m_bHtmlMode = (::GetHtmlMode((SwDocShell*)SfxObjectShell::Current()) & HTMLMODE_ON) != 0;

    GetCancelButton().SetClickHdl(LINK(this, SwFldDlg, CancelHdl));

    GetOKButton().SetClickHdl(LINK(this, SwFldDlg, OKHdl));

    m_nDokId = AddTabPage("document", SwFldDokPage::Create, 0);
    m_nVarId = AddTabPage("variables", SwFldVarPage::Create, 0);
    m_nDokInf = AddTabPage("docinfo", SwFldDokInfPage::Create, 0);

    if (!m_bHtmlMode)
    {
        m_nRefId = AddTabPage("ref", SwFldRefPage::Create, 0);
        m_nFuncId = AddTabPage("functions", SwFldFuncPage::Create, 0);

        utl::OConfigurationTreeRoot aCfgRoot
            = utl::OConfigurationTreeRoot::createWithComponentContext(
                ::comphelper::getProcessComponentContext(),
                OUString(
                        "/org.openoffice.Office.DataAccess/Policies/Features/Writer" ),
                -1,
                utl::OConfigurationTreeRoot::CM_READONLY);

        sal_Bool bDatabaseFields = sal_True;
        aCfgRoot.getNodeValue(
            OUString("DatabaseFields")) >>= bDatabaseFields;

        if (bDatabaseFields)
            m_nDbId = AddTabPage("database", SwFldDBPage::Create, 0);
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

SwFldDlg::~SwFldDlg()
{
}

sal_Bool SwFldDlg::Close()
{
    m_pBindings->GetDispatcher()->
        Execute(m_bDataBaseMode ? FN_INSERT_FIELD_DATA_ONLY : FN_INSERT_FIELD,
        SFX_CALLMODE_ASYNCHRON|SFX_CALLMODE_RECORD);
    return true;
}

void SwFldDlg::Initialize(SfxChildWinInfo *pInfo)
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
        if ( pInfo->nFlags & SFX_CHILDWIN_ZOOMIN )
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

SfxItemSet* SwFldDlg::CreateInputItemSet( sal_uInt16 nID  )
{
    if ( nID == m_nDokInf )
    {
        SwDocShell* pDocSh = (SwDocShell*)SfxObjectShell::Current();
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

/*--------------------------------------------------------------------
     Description: kick off inserting of new fields
 --------------------------------------------------------------------*/

IMPL_LINK_NOARG(SwFldDlg, OKHdl)
{
    if (GetOKButton().IsEnabled())
    {
        SfxTabPage* pPage = GetTabPage(GetCurPageId());
        pPage->FillItemSet(*(SfxItemSet*)0);

        GetOKButton().GrabFocus();  // because of InputField-Dlg
    }

    return 0;
}

IMPL_LINK_NOARG(SwFldDlg, CancelHdl)
{
    Close();
    return 0;
}


/*--------------------------------------------------------------------
    Description: newly initialise dialog after Doc-Switch
 --------------------------------------------------------------------*/

void SwFldDlg::ReInitDlg()
{
    SwDocShell* pDocSh = (SwDocShell*)SfxObjectShell::Current();
    sal_Bool bNewMode = (::GetHtmlMode(pDocSh) & HTMLMODE_ON) != 0;

    if (bNewMode != m_bHtmlMode)
    {
        SfxViewFrame::Current()->GetDispatcher()->
            Execute(FN_INSERT_FIELD, SFX_CALLMODE_ASYNCHRON|SFX_CALLMODE_RECORD);
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

/*--------------------------------------------------------------------
    Description: newly initialise TabPage after Doc-Switch
 --------------------------------------------------------------------*/

void SwFldDlg::ReInitTabPage( sal_uInt16 nPageId, sal_Bool bOnlyActivate )
{
    SwFldPage* pPage = (SwFldPage* )GetTabPage(nPageId);

    if ( pPage )
        pPage->EditNewField( bOnlyActivate );   // newly initialise TabPage
}

/*--------------------------------------------------------------------
    Description: newly initialise after activation of a few TabPages
 --------------------------------------------------------------------*/

void SwFldDlg::Activate()
{
    SwView* pView = ::GetActiveView();
    if( pView )
    {
        bool bHtmlMode = (::GetHtmlMode((SwDocShell*)SfxObjectShell::Current()) & HTMLMODE_ON) != 0;
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

void SwFldDlg::EnableInsert(sal_Bool bEnable)
{
    if( bEnable )
    {
        SwView* pView = ::GetActiveView();
        OSL_ENSURE(pView, "no view found");
        if( !pView ||
                (pView->GetWrtShell().IsReadOnlyAvailable() &&
                    pView->GetWrtShell().HasReadonlySel()) )
            bEnable = sal_False;
    }
    GetOKButton().Enable(bEnable);
}

void SwFldDlg::InsertHdl()
{
    GetOKButton().Click();
}

void SwFldDlg::ActivateDatabasePage()
{
    m_bDataBaseMode = sal_True;
    ShowPage(m_nDbId);
    SfxTabPage* pDBPage = GetTabPage(m_nDbId);
    if( pDBPage )
    {
        ((SwFldDBPage*)pDBPage)->ActivateMailMergeAddress();
    }
    //remove all other pages
    RemoveTabPage("document");
    RemoveTabPage("variables");
    RemoveTabPage("docinfo");
    RemoveTabPage("ref");
    RemoveTabPage("functions");
}

void SwFldDlg::ShowReferencePage()
{
    ShowPage(m_nRefId);
}

void SwFldDlg::PageCreated(sal_uInt16 nId, SfxTabPage& rPage)
{
    if (nId == m_nDbId)
    {
        SfxDispatcher* pDispatch = m_pBindings->GetDispatcher();
        SfxViewFrame* pViewFrame = pDispatch ? pDispatch->GetFrame() : 0;
        if(pViewFrame)
        {
            const TypeId aSwViewTypeId = TYPE(SwView);
            SfxViewShell* pViewShell = SfxViewShell::GetFirst( &aSwViewTypeId );
            while(pViewShell && pViewShell->GetViewFrame() != pViewFrame)
            {
                pViewShell = SfxViewShell::GetNext( *pViewShell, &aSwViewTypeId );
            }
            if(pViewShell)
                static_cast<SwFldDBPage&>(rPage).SetWrtShell(static_cast<SwView*>(pViewShell)->GetWrtShell());
        }
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
