/*************************************************************************
 *
 *  $RCSfile: viewdlg2.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 15:50:47 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#pragma hdrstop

#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif


#ifndef _FLDMGR_HXX //autogen
#include <fldmgr.hxx>
#endif
#ifndef _EXPFLD_HXX //autogen
#include <expfld.hxx>
#endif

#ifndef _MODOPT_HXX //autogen
#include <modcfg.hxx>
#endif

#include "swmodule.hxx"
#include "view.hxx"
#include "wview.hxx"
#include "wrtsh.hxx"
#include "cmdid.h"
#include "cption.hxx"
#include "caption.hxx"
#include "insfnote.hxx"
#include "poolfmt.hxx"
#include "edtwin.hxx"
#ifndef _SWSTYLENAMEMAPPER_HXX
#include <SwStyleNameMapper.hxx>
#endif

#include "view.hrc"

extern String* pOldGrfCat;
extern String* pOldTabCat;
extern String* pOldFrmCat;
extern String* pOldDrwCat;

/* -----------------06.11.98 13:45-------------------
 *
 * --------------------------------------------------*/

void SwView::ExecDlgExt(SfxRequest &rReq)
{
    Window *pMDI = &GetViewFrame()->GetWindow();
    ModalDialog *pDialog = 0;
    const SfxItemSet* pOutSet = 0;

    switch ( rReq.GetSlot() )
    {
        case FN_INSERT_CAPTION:
        {
            pDialog = new SwCaptionDialog( pMDI, *this );
            break;
        }
        case  FN_EDIT_FOOTNOTE:
        {
            pDialog = new SwInsFootNoteDlg( pMDI, *pWrtShell, TRUE );
            pDialog->SetHelpId(FN_EDIT_FOOTNOTE);
            pDialog->SetText( SW_RESSTR(STR_EDIT_FOOTNOTE) );
            break;
        }
    }
    if ( pDialog )
    {
        pDialog->Execute();
        delete pDialog;
    }
}

/* -----------------06.11.98 14:53-------------------
 *
 * --------------------------------------------------*/

void SwView::AutoCaption(const USHORT nType, const SvGlobalName *pOleId)
{
    SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();

    BOOL bWeb = 0 != PTR_CAST(SwWebView, this);
    if (pModOpt->IsInsWithCaption(bWeb))
    {
        const InsCaptionOpt *pOpt = pModOpt->GetCapOption(bWeb, (SwCapObjType)nType, pOleId);
        if (pOpt && pOpt->UseCaption() == TRUE)
            InsertCaption(pOpt);
    }
}

/* -----------------06.11.98 12:58-------------------
 *
 * --------------------------------------------------*/

void SwView::InsertCaption(const InsCaptionOpt *pOpt)
{
    if (!pOpt)
        return;

    const String &rName = pOpt->GetCategory();

    // Existiert Pool-Vorlage gleichen Namens?
    SwWrtShell &rSh = GetWrtShell();
    USHORT nPoolId = SwStyleNameMapper::GetPoolIdFromUIName(rName, GET_POOLID_TXTCOLL);
    if( USHRT_MAX != nPoolId )
        rSh.GetTxtCollFromPool(nPoolId);
        // Pool-Vorlage existiert nicht: Existiert sie am Dokument?
    else if( !rSh.GetParaStyle(rName) )
    {
        // Sie existiert auch nicht am Dokument: erzeugen
        SwTxtFmtColl* pDerivedFrom = rSh.GetTxtCollFromPool(RES_POOLCOLL_LABEL);
        rSh.MakeTxtFmtColl(rName, pDerivedFrom);
    }

    USHORT eType = (SwWrtShell::SelectionType)rSh.GetSelectionType();
    if (eType & SwWrtShell::SEL_OLE)
        eType = SwWrtShell::SEL_GRF;

    // SwLabelType
    const USHORT eT = eType & SwWrtShell::SEL_TBL ? LTYPE_TABLE :
                      eType & SwWrtShell::SEL_FRM ? LTYPE_FLY :
                      eType == SwWrtShell::SEL_TXT ? LTYPE_FLY :
                      eType & SwWrtShell::SEL_DRW ? LTYPE_DRAW :
                                                    LTYPE_OBJECT;

    SwFldMgr aMgr(&rSh);
    SwSetExpFieldType* pFldType =
            (SwSetExpFieldType*)aMgr.GetFldType(RES_SETEXPFLD, rName);
    if (!pFldType)
    {
        // Neuen Feldtypen erzeugen
        SwSetExpFieldType aSwSetExpFieldType(rSh.GetDoc(), rName, GSE_SEQ);
        aMgr.InsertFldType(aSwSetExpFieldType);
        pFldType = (SwSetExpFieldType*)aMgr.GetFldType(RES_SETEXPFLD, rName);
    }

    if (!pOpt->IgnoreSeqOpts())
    {
        if (pFldType)
        {
            pFldType->SetDelimiter(pOpt->GetSeparator());
            pFldType->SetOutlineLvl(pOpt->GetLevel());
        }
    }

    USHORT       nID    = 0;
    SwFieldType* pType  = 0;
    const USHORT nCount = aMgr.GetFldTypeCount();
    for (USHORT i = 0; i < nCount; ++i)
    {
        pType = aMgr.GetFldType(USHRT_MAX, i);
        String aTmpName( pType->GetName() );
        if (aTmpName == rName && pType->Which() == RES_SETEXPFLD)
        {
            nID = i;
            break;
        }
    }
    rSh.StartAllAction();

    GetWrtShell().InsertLabel( (SwLabelType)eT,
                                pOpt->GetCaption(),
                                !pOpt->GetPos(),
                                nID,
                                pOpt->CopyAttributes() );
    // Nummernformat setzen
    ((SwSetExpFieldType*)pType)->SetSeqFormat(pOpt->GetNumType());

    rSh.UpdateExpFlds( TRUE );

    rSh.EndAllAction();

    if ( rSh.IsFrmSelected() )
    {
        GetEditWin().StopInsFrm();
        rSh.EnterSelFrmMode();
    }

    // Kategorie merken
    String** ppStr = 0;
    if (eType & SwWrtShell::SEL_GRF)
        ppStr = &pOldGrfCat;
    else if( eType & SwWrtShell::SEL_TBL)
        ppStr = &pOldTabCat;
    else if( eType & SwWrtShell::SEL_FRM)
        ppStr = &pOldFrmCat;
    else if( eType == SwWrtShell::SEL_TXT)
        ppStr = &pOldFrmCat;
    else if( eType & SwWrtShell::SEL_DRW)
        ppStr = &pOldDrwCat;

    if( ppStr )
    {
        if( !*ppStr )
            *ppStr = new String( rName );
        else
            **ppStr = rName;
    }
}


