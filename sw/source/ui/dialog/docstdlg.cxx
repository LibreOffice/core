/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: docstdlg.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 11:36:55 $
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


#include <stdio.h>
#include <ctype.h>

#ifndef _SWWAIT_HXX
#include <swwait.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _PVIEW_HXX
#include <pview.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCSTDLG_HXX
#include <docstdlg.hxx>
#endif
#ifndef _MODCFG_HXX
#include <modcfg.hxx>
#endif

// fuer Statistikfelder
#ifndef _FLDMGR_HXX
#include <fldmgr.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif

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
        aUpdatePB.Show(FALSE);
        aLineLbl.Show(FALSE);
        aLineNo .Show(FALSE);
    }

}


 SwDocStatPage::~SwDocStatPage()
{
}

/*--------------------------------------------------------------------
    Beschreibung:   ItemSet fuellen bei Aenderung
 --------------------------------------------------------------------*/


BOOL  SwDocStatPage::FillItemSet(SfxItemSet & /*rSet*/)
{
    return FALSE;
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

    SwWait aWait( *pSh->GetDoc()->GetDocShell(), TRUE );
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
        aLineNo.SetText( String::CreateFromInt32( pFEShell->GetLineCount(FALSE)));
    //pButton->Disable();
    return 0;
}

