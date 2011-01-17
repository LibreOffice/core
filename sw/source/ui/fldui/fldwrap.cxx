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



#ifndef _CMDID_H
#include <cmdid.h>
#endif
#include <swtypes.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/msgbox.hxx>
#include <svx/htmlmode.hxx>
#include <viewopt.hxx>
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#include <fldwrap.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <swmodule.hxx>

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
#include "swabstdlg.hxx"

SFX_IMPL_CHILDWINDOW(SwFldDlgWrapper, FN_INSERT_FIELD)

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SwChildWinWrapper::SwChildWinWrapper(Window *pParentWindow, sal_uInt16 nId) :
        SfxChildWindow(pParentWindow, nId),
        m_pDocSh(0)
{
    // Flackern der Buttons vermeiden:
    m_aUpdateTimer.SetTimeout(200);
    m_aUpdateTimer.SetTimeoutHdl(LINK(this, SwChildWinWrapper, UpdateHdl));
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwChildWinWrapper, UpdateHdl, void*, EMPTYARG )
{
    GetWindow()->Activate();    // Dialog aktualisieren

    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung: Nach Dok-Wechsel Dialog neu initialisieren
 --------------------------------------------------------------------*/

sal_Bool SwChildWinWrapper::ReInitDlg(SwDocShell *)
{
    sal_Bool bRet = sal_False;

    if (m_pDocSh != GetOldDocShell())
    {
        m_aUpdateTimer.Stop();
        bRet = sal_True;            // Sofortiges Update
    }
    else
        m_aUpdateTimer.Start();

    return bRet;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SfxChildWinInfo SwFldDlgWrapper::GetInfo() const
{
    SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();
    aInfo.aPos = GetWindow()->OutputToAbsoluteScreenPixel(aInfo.aPos);
    return aInfo;
}


/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SwFldDlgWrapper::SwFldDlgWrapper( Window* _pParent, sal_uInt16 nId,
                                    SfxBindings* pB,
                                    SfxChildWinInfo*  )
    : SwChildWinWrapper( _pParent, nId )
{
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "SwAbstractDialogFactory fail!");

    AbstractSwFldDlg* pDlg = pFact->CreateSwFldDlg(pB, this, _pParent, DLG_FLD_INSERT );
    DBG_ASSERT(pDlg, "Dialogdiet fail!");
    pDlgInterface = pDlg;
    pWindow = pDlg->GetWindow();
    pDlg->Start();
    eChildAlignment = SFX_ALIGN_NOALIGNMENT;
}

/*--------------------------------------------------------------------
    Beschreibung: Nach Dok-Wechsel Dialog neu initialisieren
 --------------------------------------------------------------------*/

sal_Bool SwFldDlgWrapper::ReInitDlg(SwDocShell *pDocSh)
{
    sal_Bool bRet;

    if ((bRet = SwChildWinWrapper::ReInitDlg(pDocSh)) == sal_True)  // Sofort aktualisieren, Dok-Wechsel
    {
        pDlgInterface->ReInitDlg();
    }

    return bRet;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwFldDlgWrapper::ShowPage(sal_uInt16 nPage)
{
    pDlgInterface->ShowPage(nPage ? nPage : TP_FLD_REF);
}

SFX_IMPL_CHILDWINDOW(SwFldDataOnlyDlgWrapper, FN_INSERT_FIELD_DATA_ONLY)

/* -----------------04.02.2003 14:17-----------------
 *
 * --------------------------------------------------*/
SfxChildWinInfo SwFldDataOnlyDlgWrapper::GetInfo() const
{
    SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();
// prevent instatiation of dialog other than by calling
// the mail merge dialog
    aInfo.bVisible = sal_False;
    return aInfo;
}
/* -----------------04.02.2003 14:17-----------------
 *
 * --------------------------------------------------*/
SwFldDataOnlyDlgWrapper::SwFldDataOnlyDlgWrapper( Window* _pParent, sal_uInt16 nId,
                                    SfxBindings* pB,
                                    SfxChildWinInfo* pInfo )
    : SwChildWinWrapper( _pParent, nId )
{
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "SwAbstractDialogFactory fail!");

    AbstractSwFldDlg* pDlg = pFact->CreateSwFldDlg(pB, this, _pParent, DLG_FLD_INSERT );
    DBG_ASSERT(pDlg, "Dialogdiet fail!");
    pDlgInterface = pDlg;

    pWindow = pDlg->GetWindow();
    pDlg->ActivateDatabasePage();
    pDlg->Start();
    pDlg->Initialize( pInfo );
    eChildAlignment = SFX_ALIGN_NOALIGNMENT;
}
/* -----------------04.02.2003 14:17-----------------
 * re-init after doc activation
 * --------------------------------------------------*/
sal_Bool SwFldDataOnlyDlgWrapper::ReInitDlg(SwDocShell *pDocSh)
{
    sal_Bool bRet;
    if ((bRet = SwChildWinWrapper::ReInitDlg(pDocSh)) == sal_True)  // Sofort aktualisieren, Dok-Wechsel
    {
        pDlgInterface->ReInitDlg();
    }

    return bRet;
}
