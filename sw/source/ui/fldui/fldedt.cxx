/*************************************************************************
 *
 *  $RCSfile: fldedt.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: os $ $Date: 2000-11-13 12:07:14 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _BASEDLGS_HXX //autogen
#include <sfx2/basedlgs.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SVX_ADRITEM_HXX //autogen
#include <svx/adritem.hxx>
#endif
#ifndef _SVX_OPTGENRL_HXX //autogen
#include <svx/optgenrl.hxx>
#endif

#ifndef _ADDRDLG_HXX
#include <addrdlg.hxx>
#endif
#ifndef _DOCUFLD_HXX
#include <docufld.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _DBFLD_HXX
#include <dbfld.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
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
#ifndef _FLDEDT_HXX
#include <fldedt.hxx>
#endif


#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _FLDUI_HRC
#include <fldui.hrc>
#endif


/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SwFldEditDlg::SwFldEditDlg(SwView& rVw) :
    SfxSingleTabDialog(&rVw.GetViewFrame()->GetWindow(), 0, 0),
    pSh         (rVw.GetWrtShellPtr()),
    aAddressBT  (this, SW_RES(PB_FLDEDT_ADDRESS)),
    aPrevBT     (this, SW_RES(BTN_FLDEDT_PREV)),
    aNextBT     (this, SW_RES(BTN_FLDEDT_NEXT))
{
    SwFldMgr aMgr(pSh);

    SwField *pCurFld = aMgr.GetCurFld();
    if(!pCurFld)
        return;

    pSh->SetCareWin(this);
    pSh->Right(TRUE);
    pSh->SwapPam();
    USHORT nGroup = aMgr.GetGroup(FALSE, pCurFld->GetTypeId(), pCurFld->GetSubType());

    CreatePage(nGroup);

    GetOKButton()->SetClickHdl(LINK(this, SwFldEditDlg, OKHdl));

    // Buttons selbst positionieren, da sie sonst bei unterschiedlichen
    // Fontgroessen im Wald stehen, und da PB im SingleTabDlg feste Pixelgroessen
    // fuer seine Buttons und die Dialogbreite verwendet.
    aPrevBT.SetPosPixel(Point(GetOKButton()->GetPosPixel().X(), aPrevBT.GetPosPixel().Y()));
    USHORT nWidth = GetOKButton()->GetOutputSize().Width() / 2 - 3;
    Size aNewSize(LogicToPixel(Size(nWidth, GetOKButton()->GetOutputSize().Height())));
    aPrevBT.SetSizePixel(aNewSize);

    aNextBT.SetSizePixel(aPrevBT.GetSizePixel());

    long nXPos = GetOKButton()->GetPosPixel().X() + GetOKButton()->GetSizePixel().Width()
                    - aNextBT.GetSizePixel().Width() - 1;
    aNextBT.SetPosPixel(Point(nXPos, aNextBT.GetPosPixel().Y()));

    aAddressBT.SetPosPixel(Point(GetOKButton()->GetPosPixel().X(), aAddressBT.GetPosPixel().Y()));
    aAddressBT.SetSizePixel(GetOKButton()->GetSizePixel());

    aPrevBT.SetClickHdl(LINK(this, SwFldEditDlg, NextPrevHdl));
    aNextBT.SetClickHdl(LINK(this, SwFldEditDlg, NextPrevHdl));

    aAddressBT.SetClickHdl(LINK(this, SwFldEditDlg, AddressHdl));
    aAddressBT.SetHelpId(HID_FLDEDT_ADDRESS);

    Init();
}

/*--------------------------------------------------------------------
    Beschreibung: Controlls initialisieren
 --------------------------------------------------------------------*/

void SwFldEditDlg::Init()
{
    SwFldPage* pPage = (SwFldPage*)GetTabPage();

    if( pPage )
    {
        SwFldMgr& rMgr = pPage->GetFldMgr();

        SwField *pCurFld = rMgr.GetCurFld();

        if(!pCurFld)
            return;

        // Traveling nur bei mehr als einem Feld
        pSh->StartAction();
        pSh->CreateCrsr();

        BOOL bMove = rMgr.GoNext();
        if( bMove )
            rMgr.GoPrev();
        aNextBT.Enable(bMove);

        if( 0 != ( bMove = rMgr.GoPrev() ) )
            rMgr.GoNext();
        aPrevBT.Enable( bMove );

        if (pCurFld->GetTypeId() == TYP_EXTUSERFLD)
            aAddressBT.Show();

        pSh->DestroyCrsr();
        pSh->EndAction();
    }

    GetOKButton()->Enable( !pSh->IsReadOnlyAvailable() ||
                           !pSh->HasReadonlySel() );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SfxTabPage* SwFldEditDlg::CreatePage(USHORT nGroup)
{
    // TabPage erzeugen
    SfxTabPage* pPage = 0;
    USHORT nHelpId = 0;

    switch (nGroup)
    {
        case GRP_DOC:
            pPage = SwFldDokPage::Create(this, *(SfxItemSet*)0);
            nHelpId = HID_EDIT_FLD_DOK;
            break;
        case GRP_FKT:
            pPage = SwFldFuncPage::Create(this, *(SfxItemSet*)0);
            nHelpId = HID_EDIT_FLD_FUNC;
            break;
        case GRP_REF:
            pPage = SwFldRefPage::Create(this, *(SfxItemSet*)0);
            nHelpId = HID_EDIT_FLD_REF;
            break;
        case GRP_REG:
            pPage = SwFldDokInfPage::Create(this, *(SfxItemSet*)0);
            nHelpId = HID_EDIT_FLD_DOKINF;
            break;
        case GRP_DB:
            pPage = SwFldDBPage::Create(this, *(SfxItemSet*)0);
            nHelpId = HID_EDIT_FLD_DB;
            break;
        case GRP_VAR:
            pPage = SwFldVarPage::Create(this, *(SfxItemSet*)0);
            nHelpId = HID_EDIT_FLD_VAR;
            break;

    }
    pPage->SetHelpId(nHelpId);
    SetTabPage(pPage);

    String sTitle(GetText());
    sTitle.Insert(String::CreateFromAscii(": "), 0);
    sTitle.Insert(SW_RESSTR(STR_FLD_EDIT_DLG), 0);
    SetText(sTitle);

    return pPage;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SwFldEditDlg::~SwFldEditDlg()
{
    pSh->SetCareWin(NULL);
    pSh->EnterStdMode();
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwFldEditDlg::EnableInsert(BOOL bEnable)
{
    if( bEnable && pSh->IsReadOnlyAvailable() && pSh->HasReadonlySel() )
        bEnable = FALSE;
    GetOKButton()->Enable( bEnable );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwFldEditDlg::InsertHdl()
{
    GetOKButton()->Click();
}

/*--------------------------------------------------------------------
     Beschreibung: Aendern des Feldes anstossen
 --------------------------------------------------------------------*/

IMPL_LINK( SwFldEditDlg, OKHdl, Button *, pBtn )
{
    if (GetOKButton()->IsEnabled())
    {
        SfxTabPage* pPage = GetTabPage();
        if (pPage)
        {
            pPage->FillItemSet(*(SfxItemSet*)0);

        }
        EndDialog( RET_OK );
    }

    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

short SwFldEditDlg::Execute()
{
    // Ohne TabPage kein Dialog
    return GetTabPage() ? Dialog::Execute() : RET_CANCEL;
}

/*--------------------------------------------------------------------
    Beschreibung: Traveling zwishen Feldern gleichen Typs
 --------------------------------------------------------------------*/

IMPL_LINK( SwFldEditDlg, NextPrevHdl, Button *, pButton )
{
    BOOL bNext = pButton == &aNextBT;

    pSh->EnterStdMode();

    SwFieldType *pOldTyp = 0;
    SwFldPage* pPage = (SwFldPage*)GetTabPage();
    SwFldMgr& rMgr = pPage->GetFldMgr();
    SwField *pCurFld = rMgr.GetCurFld();

    if (pCurFld->GetTypeId() == TYP_DBFLD)
        pOldTyp = (SwDBFieldType*)pCurFld->GetTyp();

    if( GetOKButton()->IsEnabled() )
        pPage->FillItemSet(*(SfxItemSet*)0);

    rMgr.GoNextPrev( bNext, pOldTyp );
    pCurFld = rMgr.GetCurFld();
    pSh->Right(TRUE);
    pSh->SwapPam();

    USHORT nGroup = rMgr.GetGroup(FALSE, pCurFld->GetTypeId(), pCurFld->GetSubType());

    if (nGroup != pPage->GetGroup())
        pPage = (SwFldPage*)CreatePage(nGroup);

    pPage->EditNewField();

    Init();

    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwFldEditDlg, AddressHdl, PushButton *, pButton )
{
    SwFldPage* pPage = (SwFldPage*)GetTabPage();
    SwFldMgr& rMgr = pPage->GetFldMgr();
    SwField *pCurFld = rMgr.GetCurFld();

    SfxItemSet aSet( pSh->GetAttrPool(),
                        SID_ATTR_ADDRESS, SID_ATTR_ADDRESS,
                        SID_FIELD_GRABFOCUS, SID_FIELD_GRABFOCUS,
                        0L );
    SvxAddressItem aAddress;
    aAddress.SetWhich(SID_ATTR_ADDRESS);
    aSet.Put( aAddress );
    USHORT nEditPos = UNKNOWN_EDIT;

    switch(pCurFld->GetSubType())
    {
        case EU_FIRSTNAME:  nEditPos = FIRSTNAME_EDIT;  break;
        case EU_NAME:       nEditPos = LASTNAME_EDIT;   break;
        case EU_SHORTCUT:   nEditPos = SHORTNAME_EDIT;  break;
        case EU_COMPANY:    nEditPos = COMPANY_EDIT;    break;
        case EU_STREET:     nEditPos = STREET_EDIT;     break;
        case EU_TITLE:      nEditPos = TITLE_EDIT;      break;
        case EU_POSITION:   nEditPos = POSITION_EDIT;   break;
        case EU_PHONE_PRIVATE:nEditPos = TELPRIV_EDIT;  break;
        case EU_PHONE_COMPANY:nEditPos = TELCOMPANY_EDIT;   break;
        case EU_FAX:        nEditPos = FAX_EDIT;        break;
        case EU_EMAIL:      nEditPos = EMAIL_EDIT;      break;
        case EU_COUNTRY:    nEditPos = COUNTRY_EDIT;    break;
        case EU_ZIP:        nEditPos = PLZ_EDIT;        break;
        case EU_CITY:       nEditPos = CITY_EDIT;       break;
        case EU_STATE:      nEditPos = STATE_EDIT;      break;

        default:            nEditPos = UNKNOWN_EDIT;    break;

    }
    aSet.Put(SfxUInt16Item(SID_FIELD_GRABFOCUS, nEditPos));
    SwAddrDlg aDlg( this, aSet );
    aDlg.Execute();

    const SfxItemSet* pSfxItemSet = aDlg.GetOutputItemSet();
    const SfxPoolItem* pItem;
    if( pSfxItemSet && SFX_ITEM_SET == pSfxItemSet->GetItemState(
            SID_ATTR_ADDRESS, FALSE, &pItem ) )
    {
        ((SvxAddressItem*)pItem)->Store();
        pSh->UpdateFlds( *pCurFld );
    }

    return 0;
}


