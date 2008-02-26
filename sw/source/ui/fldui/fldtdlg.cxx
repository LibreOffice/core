/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fldtdlg.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 14:24:57 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif

#ifndef _UNOTOOLS_CONFIGNODE_HXX_
#include <unotools/confignode.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _BASEDLGS_HXX //autogen
#include <sfx2/basedlgs.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX //autogen
#include <svx/htmlmode.hxx>
#endif

#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _FLDWRAP_HXX
#include <fldwrap.hxx>
#endif
#ifndef _FLDDB_HXX
#include <flddb.hxx>
#endif
#ifndef _FLDDINF_HXX
#include <flddinf.hxx>
#endif
#ifndef _FLDVAR_HXX
#include <fldvar.hxx>
#endif
#ifndef _FLDDOK_HXX
#include <flddok.hxx>
#endif
#ifndef _FLDFUNC_HXX
#include <fldfunc.hxx>
#endif
#ifndef _FLDREF_HXX
#include <fldref.hxx>
#endif
#ifndef _WRTSH_HXX //autogen
#include <wrtsh.hxx>
#endif
#ifndef _SWVIEW_HXX //autogen
#include <view.hxx>
#endif
#ifndef _FLDTDLG_HXX
#include <fldtdlg.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif

#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _FLDUI_HRC
#include <fldui.hrc>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _FLDTDLG_HRC
#include <fldtdlg.hrc>
#endif

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>


/*--------------------------------------------------------------------
    Beschreibung:   Der Traeger des Dialoges
 --------------------------------------------------------------------*/


SwFldDlg::SwFldDlg(SfxBindings* pB, SwChildWinWrapper* pCW, Window *pParent)
    : SfxTabDialog( pParent, SW_RES( DLG_FLD_INSERT )),
    m_pChildWin(pCW),
    m_pBindings(pB),
    m_bDataBaseMode(FALSE)
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
                        "/org.openoffice.Office.DataAccess/ApplicationIntegration/InstalledFeatures/Writer" ) ),
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

BOOL SwFldDlg::Close()
{
    m_pBindings->GetDispatcher()->
        Execute(m_bDataBaseMode ? FN_INSERT_FIELD_DATA_ONLY : FN_INSERT_FIELD,
        SFX_CALLMODE_ASYNCHRON|SFX_CALLMODE_RECORD);
    return TRUE;
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

SfxItemSet* SwFldDlg::CreateInputItemSet( USHORT nID  )
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
        uno::Reference< beans::XPropertySetInfo > xSetInfo
            = xUDProps->getPropertySetInfo();
        const uno::Sequence< beans::Property > props
            = xSetInfo->getProperties();
        uno::Sequence< ::rtl::OUString > names(props.getLength());
        for (sal_Int32 i = 0; i < props.getLength(); ++i) {
            names[i] = props[i].Name;
        }
        pISet->Put( SfxUnoAnyItem( SID_DOCINFO, uno::makeAny(names) ) );
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
    BOOL bNewMode = (::GetHtmlMode(pDocSh) & HTMLMODE_ON) != 0;

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

void SwFldDlg::ReInitTabPage( USHORT nPageId, BOOL bOnlyActivate )
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
        BOOL bHtmlMode = (::GetHtmlMode((SwDocShell*)SfxObjectShell::Current()) & HTMLMODE_ON) != 0;
        const SwWrtShell& rSh = pView->GetWrtShell();
        GetOKButton().Enable( !rSh.IsReadOnlyAvailable() ||
                              !rSh.HasReadonlySel() );

        ReInitTabPage( TP_FLD_VAR, TRUE );

        if( !bHtmlMode )
        {
            ReInitTabPage( TP_FLD_REF, TRUE );
            ReInitTabPage( TP_FLD_FUNC, TRUE );
        }
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwFldDlg::EnableInsert(BOOL bEnable)
{
    if( bEnable )
    {
        SwView* pView = ::GetActiveView();
        DBG_ASSERT(pView, "no view found");
        if( !pView ||
                (pView->GetWrtShell().IsReadOnlyAvailable() &&
                    pView->GetWrtShell().HasReadonlySel()) )
            bEnable = FALSE;
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
    m_bDataBaseMode = TRUE;
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
void SwFldDlg::PageCreated(USHORT nId, SfxTabPage& rPage)
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


