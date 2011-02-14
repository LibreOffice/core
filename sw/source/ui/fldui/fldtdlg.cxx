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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif



#ifndef _CMDID_H
#include <cmdid.h>
#endif
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
    Beschreibung:   Der Traeger des Dialoges
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
    GetOKButton().SetHelpText(aEmptyStr);   // Damit generierter Hilfetext verwendet wird

    GetCancelButton().SetText(String(SW_RES(STR_FLD_CLOSE)));
    GetCancelButton().SetHelpId(HID_FIELD_CLOSE);
    GetCancelButton().SetHelpText(aEmptyStr);   // Damit generierter Hilfetext verwendet wird

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

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SwFldDlg::~SwFldDlg()
{
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

sal_Bool SwFldDlg::Close()
{
    m_pBindings->GetDispatcher()->
        Execute(m_bDataBaseMode ? FN_INSERT_FIELD_DATA_ONLY : FN_INSERT_FIELD,
        SFX_CALLMODE_ASYNCHRON|SFX_CALLMODE_RECORD);
    return sal_True;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwFldDlg::Initialize(SfxChildWinInfo *pInfo)
{
    Point aPos;
    Size aSize;

    if ( pInfo->aSize.Width() != 0 && pInfo->aSize.Height() != 0 )
    {
        aPos = pInfo->aPos;
        if ( GetStyle() & WB_SIZEABLE )
            SetSizePixel( pInfo->aSize );

        // Initiale Gr"o\se aus pInfo merken
        aSize = GetSizePixel();

        // Soll das FloatingWindow eingezoomt werden ?
        if ( pInfo->nFlags & SFX_CHILDWIN_ZOOMIN )
            RollUp();
    }
    else
    {
        // Initiale Gr"o\se aus Resource oder ctor merken
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

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

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
     Beschreibung: Einfuegen von neuen Feldern anstossen
 --------------------------------------------------------------------*/

IMPL_LINK( SwFldDlg, OKHdl, Button *, EMPTYARG )
{
    if (GetOKButton().IsEnabled())
    {
        SfxTabPage* pPage = GetTabPage(GetCurPageId());
        pPage->FillItemSet(*(SfxItemSet*)0);

        GetOKButton().GrabFocus();  // Wegen InputField-Dlg
    }

    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung: Nach Dok-Wechsel Dialog neu initialisieren
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
    Beschreibung: Nach Dok-Wechsel TabPage neu initialisieren
 --------------------------------------------------------------------*/

void SwFldDlg::ReInitTabPage( sal_uInt16 nPageId, sal_Bool bOnlyActivate )
{
    SwFldPage* pPage = (SwFldPage* )GetTabPage(nPageId);

    if ( pPage )
        pPage->EditNewField( bOnlyActivate );   // TabPage neu initialisieren
}

/*--------------------------------------------------------------------
    Beschreibung: Nach Aktivierung einige TabPages neu initialisieren
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

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwFldDlg::EnableInsert(sal_Bool bEnable)
{
    if( bEnable )
    {
        SwView* pView = ::GetActiveView();
        DBG_ASSERT(pView, "no view found");
        if( !pView ||
                (pView->GetWrtShell().IsReadOnlyAvailable() &&
                    pView->GetWrtShell().HasReadonlySel()) )
            bEnable = sal_False;
    }
    GetOKButton().Enable(bEnable);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwFldDlg::InsertHdl()
{
    GetOKButton().Click();
}
/* -----------------27.11.2002 15:24-----------------
 *
 * --------------------------------------------------*/
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
/*-- 07.10.2003 14:01:44---------------------------------------------------

  -----------------------------------------------------------------------*/
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


