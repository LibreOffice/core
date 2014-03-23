/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
       aCharNo      (this, SW_RES( FT_CHAR_COUNT )),
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
    if ( dynamic_cast< SwView* >(pVSh) )
        pSh = ((SwView*)pVSh)->GetWrtShellPtr();
    else if ( dynamic_cast< SwPagePreView* >(pVSh) )
        pSh = ((SwPagePreView*)pVSh)->GetViewShell();

    ASSERT( pSh, "Shell not found" );

    SwWait aWait( *pSh->GetDoc()->GetDocShell(), true );
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

