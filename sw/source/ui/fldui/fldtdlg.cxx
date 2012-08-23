/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <cmdid.h>
#include <swtypes.hxx>
#include <unotools/confignode.hxx>
#include <comphelper/processfactory.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/msgbox.hxx>
#include <svx/htmlmode.hxx>
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
#include <fldtdlg.hrc>

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>


/*--------------------------------------------------------------------
    Description:    carrier of the dialog
 --------------------------------------------------------------------*/


SwFldDlg::SwFldDlg(SfxBindings* pB, SwChildWinWrapper* pCW, Window *pParent)
    : SfxTabDialog( pParent, SW_RES( DLG_FLD_INSERT )),
    m_pChildWin(pCW),
    m_pBindings(pB),
    m_bDataBaseMode(sal_False)
{
    SetStyle(GetStyle()|WB_STDMODELESS);
    m_bHtmlMode = (::GetHtmlMode((SwDocShell*)SfxObjectShell::Current()) & HTMLMODE_ON) != 0;

    RemoveResetButton();

    GetOKButton().SetText(String(SW_RES(STR_FLD_INSERT)));
    GetOKButton().SetHelpId(HID_FIELD_INSERT);
    GetOKButton().SetHelpText(aEmptyStr);   // so that generated help text is used

    GetCancelButton().SetText(String(SW_RES(STR_FLD_CLOSE)));
    GetCancelButton().SetHelpId(HID_FIELD_CLOSE);
    GetCancelButton().SetHelpText(aEmptyStr);   // so that generated help text is used
    GetCancelButton().SetClickHdl(LINK(this, SwFldDlg, CancelHdl));

    FreeResource();

    GetOKButton().SetClickHdl(LINK(this, SwFldDlg, OKHdl));

    AddTabPage(TP_FLD_DOK, SwFldDokPage::Create, 0);
    AddTabPage(TP_FLD_VAR, SwFldVarPage::Create, 0);
    AddTabPage(TP_FLD_DOKINF, SwFldDokInfPage::Create, 0);

    if (!m_bHtmlMode)
    {
        AddTabPage(TP_FLD_REF, SwFldRefPage::Create, 0);
        AddTabPage(TP_FLD_FUNC, SwFldFuncPage::Create, 0);

        utl::OConfigurationTreeRoot aCfgRoot
            = utl::OConfigurationTreeRoot::createWithServiceFactory(
                ::comphelper::getProcessServiceFactory(),
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "/org.openoffice.Office.DataAccess/Policies/Features/Writer" ) ),
                -1,
                utl::OConfigurationTreeRoot::CM_READONLY);

        sal_Bool bDatabaseFields = sal_True;
        aCfgRoot.getNodeValue(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DatabaseFields"))) >>= bDatabaseFields;

        if (bDatabaseFields)
            AddTabPage(TP_FLD_DB, SwFldDBPage::Create, 0);
        else
            RemoveTabPage(TP_FLD_DB);
    }
    else
    {
        RemoveTabPage(TP_FLD_REF);
        RemoveTabPage(TP_FLD_FUNC);
        RemoveTabPage(TP_FLD_DB);
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
    return sal_True;
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
    if ( nID == TP_FLD_DOKINF )
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

    ReInitTabPage(TP_FLD_DOK);
    ReInitTabPage(TP_FLD_VAR);
    ReInitTabPage(TP_FLD_DOKINF);

    if (!m_bHtmlMode)
    {
        ReInitTabPage(TP_FLD_REF);
        ReInitTabPage(TP_FLD_FUNC);
        ReInitTabPage(TP_FLD_DB);
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
        sal_Bool bHtmlMode = (::GetHtmlMode((SwDocShell*)SfxObjectShell::Current()) & HTMLMODE_ON) != 0;
        const SwWrtShell& rSh = pView->GetWrtShell();
        GetOKButton().Enable( !rSh.IsReadOnlyAvailable() ||
                              !rSh.HasReadonlySel() );

        ReInitTabPage( TP_FLD_VAR, sal_True );

        if( !bHtmlMode )
        {
            ReInitTabPage( TP_FLD_REF, sal_True );
            ReInitTabPage( TP_FLD_FUNC, sal_True );
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
    ShowPage( TP_FLD_DB );
    SfxTabPage* pDBPage =  GetTabPage( TP_FLD_DB );
    if( pDBPage )
    {
        ((SwFldDBPage*)pDBPage)->ActivateMailMergeAddress();
    }
    //remove all other pages
    RemoveTabPage(TP_FLD_DOK);
    RemoveTabPage(TP_FLD_VAR);
    RemoveTabPage(TP_FLD_DOKINF);
    RemoveTabPage(TP_FLD_REF);
    RemoveTabPage(TP_FLD_FUNC);
}

void SwFldDlg::PageCreated(sal_uInt16 nId, SfxTabPage& rPage)
{
    if( TP_FLD_DB == nId)
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
