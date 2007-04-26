/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fldwrap.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-26 09:10:26 $
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



#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
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
//CHINA001 #ifndef _FLDDB_HXX
//CHINA001 #include <flddb.hxx>
//CHINA001 #endif
//CHINA001 #ifndef _FLDDINF_HXX
//CHINA001 #include <flddinf.hxx>
//CHINA001 #endif
//CHINA001 #ifndef _FLDVAR_HXX
//CHINA001 #include <fldvar.hxx>
//CHINA001 #endif
//CHINA001 #ifndef _FLDDOK_HXX
//CHINA001 #include <flddok.hxx>
//CHINA001 #endif
//CHINA001 #ifndef _FLDFUNC_HXX
//CHINA001 #include <fldfunc.hxx>
//CHINA001 #endif
//CHINA001 #ifndef _FLDREF_HXX
//CHINA001 #include <fldref.hxx>
//CHINA001 #endif
#ifndef _WRTSH_HXX //autogen
#include <wrtsh.hxx>
#endif
#ifndef _SWVIEW_HXX //autogen
#include <view.hxx>
#endif
//CHINA001 #ifndef _FLDTDLG_HXX
//CHINA001 #include <fldtdlg.hxx>
//CHINA001 #endif
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
#include "swabstdlg.hxx" //CHINA001

SFX_IMPL_CHILDWINDOW(SwFldDlgWrapper, FN_INSERT_FIELD)

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SwChildWinWrapper::SwChildWinWrapper(Window *pParentWindow, USHORT nId) :
        SfxChildWindow(pParentWindow, nId),
        pDocSh(0)
{
    // Flackern der Buttons vermeiden:
    aUpdateTimer.SetTimeout(200);
    aUpdateTimer.SetTimeoutHdl(LINK(this, SwChildWinWrapper, UpdateHdl));
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

BOOL SwChildWinWrapper::ReInitDlg(SwDocShell *pDocSh)
{
    BOOL bRet = FALSE;

    if (pDocSh != GetOldDocShell())
    {
        aUpdateTimer.Stop();
        bRet = TRUE;            // Sofortiges Update
    }
    else
        aUpdateTimer.Start();

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

SwFldDlgWrapper::SwFldDlgWrapper( Window* pParent, USHORT nId,
                                    SfxBindings* pB,
                                    SfxChildWinInfo* pInfo )
    : SwChildWinWrapper( pParent, nId )
{
//CHINA001  SwFldDlg *pDlg = new SwFldDlg( pB, this, pParent );
//CHINA001  pWindow = pDlg;
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();//CHINA001
    DBG_ASSERT(pFact, "SwAbstractDialogFactory fail!");//CHINA001

    AbstractSwFldDlg* pDlg = pFact->CreateSwFldDlg(pB, this, pParent, DLG_FLD_INSERT );
    DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001
    pDlgInterface = pDlg;
    pWindow = pDlg->GetWindow();
    pDlg->Start();
    eChildAlignment = SFX_ALIGN_NOALIGNMENT;
}

/*--------------------------------------------------------------------
    Beschreibung: Nach Dok-Wechsel Dialog neu initialisieren
 --------------------------------------------------------------------*/

BOOL SwFldDlgWrapper::ReInitDlg(SwDocShell *pDocSh)
{
    BOOL bRet;

    if ((bRet = SwChildWinWrapper::ReInitDlg(pDocSh)) == TRUE)  // Sofort aktualisieren, Dok-Wechsel
    {
    //CHINA001 ((SwFldDlg*)GetWindow())->ReInitDlg();
        pDlgInterface->ReInitDlg(); //CHINA001
    }

    return bRet;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwFldDlgWrapper::ShowPage(USHORT nPage)
{
    //CHINA001 ((SwFldDlg*)GetWindow())->ShowPage(nPage ? nPage : TP_FLD_REF);
    pDlgInterface->ShowPage(nPage ? nPage : TP_FLD_REF); //CHINA001
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
    aInfo.bVisible = FALSE;
    return aInfo;
}
/* -----------------04.02.2003 14:17-----------------
 *
 * --------------------------------------------------*/
SwFldDataOnlyDlgWrapper::SwFldDataOnlyDlgWrapper( Window* pParent, USHORT nId,
                                    SfxBindings* pB,
                                    SfxChildWinInfo* pInfo )
    : SwChildWinWrapper( pParent, nId )
{
//CHINA001     SwFldDlg *pDlg = new SwFldDlg( pB, this, pParent );
//CHINA001  pWindow = pDlg;
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();//CHINA001
    DBG_ASSERT(pFact, "SwAbstractDialogFactory fail!");//CHINA001

    AbstractSwFldDlg* pDlg = pFact->CreateSwFldDlg(pB, this, pParent, DLG_FLD_INSERT );
    DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001
    pDlgInterface = pDlg; //CHINA001

    pWindow = pDlg->GetWindow();//CHINA001
    pDlg->ActivateDatabasePage();
    pDlg->Start();
    pDlg->Initialize( pInfo );
    eChildAlignment = SFX_ALIGN_NOALIGNMENT;
}
/* -----------------04.02.2003 14:17-----------------
 * re-init after doc activation
 * --------------------------------------------------*/
BOOL SwFldDataOnlyDlgWrapper::ReInitDlg(SwDocShell *pDocSh)
{
    BOOL bRet;
    if ((bRet = SwChildWinWrapper::ReInitDlg(pDocSh)) == TRUE)  // Sofort aktualisieren, Dok-Wechsel
    {
        //CHINA001 ((SwFldDlg*)GetWindow())->ReInitDlg();
        pDlgInterface->ReInitDlg(); //CHINA001
    }

    return bRet;
}
