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


#include <stdio.h>
#include <ctype.h>
#include <swwait.hxx>
#include <wrtsh.hxx>
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#include <swmodule.hxx>
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _PVIEW_HXX
#include <pview.hxx>
#endif
#include <doc.hxx>
#include <docstdlg.hxx>
#ifndef _MODCFG_HXX
#include <modcfg.hxx>
#endif

// fuer Statistikfelder
#include <fldmgr.hxx>
#include <fldbas.hxx>

#ifndef _DOCSTDLG_HRC
#include <docstdlg.hrc>
#endif


/*--------------------------------------------------------------------
    Beschreibung: Create
 --------------------------------------------------------------------*/


SfxTabPage *  SwDocStatPage::Create(Window *pParent, const SfxItemSet &rSet)
{
    return new SwDocStatPage(pParent, rSet);
}

/*--------------------------------------------------------------------
    Beschreibung:   Ctor
 --------------------------------------------------------------------*/


SwDocStatPage::SwDocStatPage(Window *pParent, const SfxItemSet &rSet) :

    SfxTabPage  (pParent, SW_RES(TP_DOC_STAT), rSet),

    aTableLbl   (this, SW_RES( FT_TABLE      )),
    aGrfLbl     (this, SW_RES( FT_GRF        )),
    aOLELbl     (this, SW_RES( FT_OLE        )),
    aPageLbl    (this, SW_RES( FT_PAGE       )),
    aParaLbl    (this, SW_RES( FT_PARA       )),
    aWordLbl    (this, SW_RES( FT_WORD       )),
    aCharLbl    (this, SW_RES( FT_CHAR       )),
    aLineLbl    (this, SW_RES( FT_LINE       )),

    aTableNo    (this, SW_RES( FT_TABLE_COUNT)),
    aGrfNo      (this, SW_RES( FT_GRF_COUNT  )),
    aOLENo      (this, SW_RES( FT_OLE_COUNT  )),
    aPageNo     (this, SW_RES( FT_PAGE_COUNT )),
    aParaNo     (this, SW_RES( FT_PARA_COUNT )),
    aWordNo     (this, SW_RES( FT_WORD_COUNT )),
    aCharNo     (this, SW_RES( FT_CHAR_COUNT )),
    aLineNo     (this, SW_RES( FT_LINE_COUNT )),
    aUpdatePB   (this, SW_RES( PB_PDATE      ))
{
    Update();
    FreeResource();
    aUpdatePB.SetClickHdl(LINK(this, SwDocStatPage, UpdateHdl));
    //#111684# is the current view a page preview no SwFEShell can be found -> hide the update button
    SwDocShell* pDocShell = (SwDocShell*) SfxObjectShell::Current();
    SwFEShell* pFEShell = pDocShell->GetFEShell();
    if(!pFEShell)
    {
        aUpdatePB.Show(sal_False);
        aLineLbl.Show(sal_False);
        aLineNo .Show(sal_False);
    }

}


 SwDocStatPage::~SwDocStatPage()
{
}

/*--------------------------------------------------------------------
    Beschreibung:   ItemSet fuellen bei Aenderung
 --------------------------------------------------------------------*/


sal_Bool  SwDocStatPage::FillItemSet(SfxItemSet & /*rSet*/)
{
    return sal_False;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void  SwDocStatPage::Reset(const SfxItemSet &/*rSet*/)
{
}
/*------------------------------------------------------------------------
 Beschreibung:  Aktualisieren / Setzen der Daten
------------------------------------------------------------------------*/


void SwDocStatPage::SetData(const SwDocStat &rStat)
{
    aTableNo.SetText(String::CreateFromInt32( rStat.nTbl ));
    aGrfNo.SetText(String::CreateFromInt32( rStat.nGrf ));
    aOLENo.SetText(String::CreateFromInt32( rStat.nOLE ));
    aPageNo.SetText(String::CreateFromInt32( rStat.nPage ));
    aParaNo.SetText(String::CreateFromInt32( rStat.nPara ));
    aWordNo.SetText(String::CreateFromInt32( rStat.nWord ));
    aCharNo.SetText(String::CreateFromInt32( rStat.nChar ));
}

/*------------------------------------------------------------------------
 Beschreibung:  Aktualisieren der Statistik
------------------------------------------------------------------------*/


void SwDocStatPage::Update()
{
    SfxViewShell *pVSh = SfxViewShell::Current();
    ViewShell *pSh = 0;
    if ( pVSh->ISA(SwView) )
        pSh = ((SwView*)pVSh)->GetWrtShellPtr();
    else if ( pVSh->ISA(SwPagePreView) )
        pSh = ((SwPagePreView*)pVSh)->GetViewShell();

    ASSERT( pSh, "Shell not found" );

    SwWait aWait( *pSh->GetDoc()->GetDocShell(), sal_True );
    pSh->StartAction();
    aDocStat = pSh->GetDoc()->GetDocStat();
    pSh->GetDoc()->UpdateDocStat( aDocStat );
    pSh->EndAction();

    SetData(aDocStat);
}

/*-----------------19.06.97 16.37-------------------
    Zeilennummer aktualisieren
--------------------------------------------------*/
IMPL_LINK( SwDocStatPage, UpdateHdl, PushButton*, EMPTYARG)
{
    Update();
    SwDocShell* pDocShell = (SwDocShell*) SfxObjectShell::Current();
    SwFEShell* pFEShell = pDocShell->GetFEShell();
    if(pFEShell)
        aLineNo.SetText( String::CreateFromInt32( pFEShell->GetLineCount(sal_False)));
    //pButton->Disable();
    return 0;
}

