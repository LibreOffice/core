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

#include <stdio.h>
#include <ctype.h>
#include <swwait.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <swmodule.hxx>
#include <globals.hrc>
#include <docsh.hxx>
#include <pview.hxx>
#include <doc.hxx>
#include <docstdlg.hxx>
#include <modcfg.hxx>

// for statistics fields
#include <fldmgr.hxx>
#include <fldbas.hxx>

#include <docstdlg.hrc>


SfxTabPage *  SwDocStatPage::Create(Window *pParent, const SfxItemSet &rSet)
{
    return new SwDocStatPage(pParent, rSet);
}

SwDocStatPage::SwDocStatPage(Window *pParent, const SfxItemSet &rSet) :

    SfxTabPage  (pParent, SW_RES(TP_DOC_STAT), rSet),
    aPageLbl    (this, SW_RES( FT_PAGE       )),
    aPageNo     (this, SW_RES( FT_PAGE_COUNT )),
    aTableLbl   (this, SW_RES( FT_TABLE      )),
    aTableNo    (this, SW_RES( FT_TABLE_COUNT)),
    aGrfLbl     (this, SW_RES( FT_GRF        )),
    aGrfNo      (this, SW_RES( FT_GRF_COUNT  )),
    aOLELbl     (this, SW_RES( FT_OLE        )),
    aOLENo      (this, SW_RES( FT_OLE_COUNT  )),
    aParaLbl    (this, SW_RES( FT_PARA       )),
    aParaNo     (this, SW_RES( FT_PARA_COUNT )),
    aWordLbl    (this, SW_RES( FT_WORD       )),
    aWordNo     (this, SW_RES( FT_WORD_COUNT )),
    aCharLbl    (this, SW_RES( FT_CHAR       )),
    aCharNo     (this, SW_RES( FT_CHAR_COUNT )),
    aCharExclSpacesLbl (this, SW_RES( FT_CHAR_EXCL_SPACES )),
    aCharExclSpacesNo (this, SW_RES( FT_CHAR_COUNT_EXCL_SPACES )),
    aLineLbl    (this, SW_RES( FT_LINE       )),
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
    Description:    fill ItemSet when changed
 --------------------------------------------------------------------*/


sal_Bool  SwDocStatPage::FillItemSet(SfxItemSet & /*rSet*/)
{
    return sal_False;
}

void  SwDocStatPage::Reset(const SfxItemSet &/*rSet*/)
{
}
/*------------------------------------------------------------------------
 Description:   update / set data
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
    aCharExclSpacesNo.SetText(String::CreateFromInt32( rStat.nCharExcludingSpaces ));
}

/*------------------------------------------------------------------------
 Description:   update statistics
------------------------------------------------------------------------*/


void SwDocStatPage::Update()
{
    SfxViewShell *pVSh = SfxViewShell::Current();
    ViewShell *pSh = 0;
    if ( pVSh->ISA(SwView) )
        pSh = ((SwView*)pVSh)->GetWrtShellPtr();
    else if ( pVSh->ISA(SwPagePreView) )
        pSh = ((SwPagePreView*)pVSh)->GetViewShell();

    OSL_ENSURE( pSh, "Shell not found" );

    SwWait aWait( *pSh->GetDoc()->GetDocShell(), sal_True );
    pSh->StartAction();
    aDocStat = pSh->GetDoc()->GetUpdatedDocStat();
    pSh->EndAction();

    SetData(aDocStat);
}

IMPL_LINK_NOARG(SwDocStatPage, UpdateHdl)
{
    Update();
    SwDocShell* pDocShell = (SwDocShell*) SfxObjectShell::Current();
    SwFEShell* pFEShell = pDocShell->GetFEShell();
    if(pFEShell)
        aLineNo.SetText( String::CreateFromInt32( pFEShell->GetLineCount(sal_False)));
    //pButton->Disable();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
