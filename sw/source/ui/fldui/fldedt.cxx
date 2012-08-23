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

#include <vcl/msgbox.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/request.hxx>
#include <sfx2/app.hxx>
#include <svx/optgenrl.hxx>
#include <docufld.hxx>
#include <view.hxx>
#include <dbfld.hxx>
#include <wrtsh.hxx>
#include <flddb.hxx>
#include <flddinf.hxx>
#include <fldvar.hxx>
#include <flddok.hxx>
#include <fldfunc.hxx>
#include <fldref.hxx>
#include <fldedt.hxx>
#include <crsskip.hxx>


#include <cmdid.h>
#include <helpid.h>
#include <globals.hrc>
#include <fldui.hrc>
#include "swabstdlg.hxx"
#include "dialog.hrc"

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>

namespace swui
{
    SwAbstractDialogFactory * GetFactory();
}

SwFldEditDlg::SwFldEditDlg(SwView& rVw) :
    SfxSingleTabDialog(&rVw.GetViewFrame()->GetWindow(), 0, 0),
    pSh         (rVw.GetWrtShellPtr()),
    aPrevBT     (this, SW_RES(BTN_FLDEDT_PREV)),
    aNextBT     (this, SW_RES(BTN_FLDEDT_NEXT)),
    aAddressBT  (this, SW_RES(PB_FLDEDT_ADDRESS))
{
    SwFldMgr aMgr(pSh);

    SwField *pCurFld = aMgr.GetCurFld();
    if(!pCurFld)
        return;

    pSh->SetCareWin(this);

    /* #108536# Only create selection if there is none
        already. Normalize PaM instead of swapping. */
    if ( ! pSh->HasSelection() )
        pSh->Right(CRSR_SKIP_CHARS, sal_True, 1, sal_False );

    pSh->NormalizePam();

    sal_uInt16 nGroup = aMgr.GetGroup(sal_False, pCurFld->GetTypeId(), pCurFld->GetSubType());

    CreatePage(nGroup);

    GetOKButton()->SetClickHdl(LINK(this, SwFldEditDlg, OKHdl));

    // position buttons ourselves because otherwise when font sizes are
    // varying, they are in the woods, and because PB uses fixed pixel sizes
    // for its buttons and dialog width in SingleTabDlg.
    aPrevBT.SetPosPixel(Point(GetOKButton()->GetPosPixel().X(), aPrevBT.GetPosPixel().Y()));
    sal_uInt16 nWidth = static_cast< sal_uInt16 >(GetOKButton()->GetOutputSize().Width() / 2 - 3);
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
    Description: initialise controls
 --------------------------------------------------------------------*/
void SwFldEditDlg::Init()
{
    SwFldPage* pTabPage = (SwFldPage*)GetTabPage();

    if( pTabPage )
    {
        SwFldMgr& rMgr = pTabPage->GetFldMgr();

        SwField *pCurFld = rMgr.GetCurFld();

        if(!pCurFld)
            return;

        // Traveling only when more than one field
        pSh->StartAction();
        pSh->CreateCrsr();

        sal_Bool bMove = rMgr.GoNext();
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

SfxTabPage* SwFldEditDlg::CreatePage(sal_uInt16 nGroup)
{
    // create TabPage
    SfxTabPage* pTabPage = 0;
    const char* pHelpId = 0;

    switch (nGroup)
    {
        case GRP_DOC:
            pTabPage = SwFldDokPage::Create(this, *(SfxItemSet*)0);
            pHelpId = HID_EDIT_FLD_DOK;
            break;
        case GRP_FKT:
            pTabPage = SwFldFuncPage::Create(this, *(SfxItemSet*)0);
            pHelpId = HID_EDIT_FLD_FUNC;
            break;
        case GRP_REF:
            pTabPage = SwFldRefPage::Create(this, *(SfxItemSet*)0);
            pHelpId = HID_EDIT_FLD_REF;
            break;
        case GRP_REG:
            {
                SfxObjectShell* pDocSh = SfxObjectShell::Current();
                SfxItemSet* pSet = new SfxItemSet( pDocSh->GetPool(), SID_DOCINFO, SID_DOCINFO );
                using namespace ::com::sun::star;
                uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
                    pDocSh->GetModel(), uno::UNO_QUERY_THROW);
                uno::Reference<document::XDocumentProperties> xDocProps
                    = xDPS->getDocumentProperties();
                uno::Reference< beans::XPropertySet > xUDProps(
                    xDocProps->getUserDefinedProperties(),
                    uno::UNO_QUERY_THROW);
                pSet->Put( SfxUnoAnyItem( SID_DOCINFO, uno::makeAny(xUDProps) ) );
                pTabPage = SwFldDokInfPage::Create(this, *pSet);
                pHelpId = HID_EDIT_FLD_DOKINF;
                break;
            }
        case GRP_DB:
            pTabPage = SwFldDBPage::Create(this, *(SfxItemSet*)0);
            static_cast<SwFldDBPage*>(pTabPage)->SetWrtShell(*pSh);
            pHelpId = HID_EDIT_FLD_DB;
            break;
        case GRP_VAR:
            pTabPage = SwFldVarPage::Create(this, *(SfxItemSet*)0);
            pHelpId = HID_EDIT_FLD_VAR;
            break;

    }

    pTabPage->SetHelpId(pHelpId);
    static_cast<SwFldPage*>(pTabPage)->SetWrtShell(pSh);

    SetTabPage(pTabPage);

    String sTitle(GetText());
    sTitle.Insert(rtl::OUString(": "), 0);
    sTitle.Insert(SW_RESSTR(STR_FLD_EDIT_DLG), 0);
    SetText(sTitle);

    return pTabPage;
}

SwFldEditDlg::~SwFldEditDlg()
{
    pSh->SetCareWin(NULL);
    pSh->EnterStdMode();
}

void SwFldEditDlg::EnableInsert(sal_Bool bEnable)
{
    if( bEnable && pSh->IsReadOnlyAvailable() && pSh->HasReadonlySel() )
        bEnable = sal_False;
    GetOKButton()->Enable( bEnable );
}

void SwFldEditDlg::InsertHdl()
{
    GetOKButton()->Click();
}

/*--------------------------------------------------------------------
     Description: kick off changing of the field
 --------------------------------------------------------------------*/
IMPL_LINK_NOARG(SwFldEditDlg, OKHdl)
{
    if (GetOKButton()->IsEnabled())
    {
        SfxTabPage* pTabPage = GetTabPage();
        if (pTabPage)
        {
            pTabPage->FillItemSet(*(SfxItemSet*)0);

        }
        EndDialog( RET_OK );
    }

    return 0;
}

short SwFldEditDlg::Execute()
{
    // without TabPage no dialog
    return GetTabPage() ? Dialog::Execute() : RET_CANCEL;
}

/*--------------------------------------------------------------------
    Description: Traveling between fields of the same type
 --------------------------------------------------------------------*/
IMPL_LINK( SwFldEditDlg, NextPrevHdl, Button *, pButton )
{
    sal_Bool bNext = pButton == &aNextBT;

    pSh->EnterStdMode();

    SwFieldType *pOldTyp = 0;
    SwFldPage* pTabPage = (SwFldPage*)GetTabPage();

    //#112462# FillItemSet may delete the current field
    //that's why it has to be called before accessing the current field
    if( GetOKButton()->IsEnabled() )
        pTabPage->FillItemSet(*(SfxItemSet*)0);

    SwFldMgr& rMgr = pTabPage->GetFldMgr();
    SwField *pCurFld = rMgr.GetCurFld();
    if (pCurFld->GetTypeId() == TYP_DBFLD)
        pOldTyp = (SwDBFieldType*)pCurFld->GetTyp();

    rMgr.GoNextPrev( bNext, pOldTyp );
    pCurFld = rMgr.GetCurFld();

    /* #108536# Only create selection if there is none
        already. Normalize PaM instead of swapping. */
    if ( ! pSh->HasSelection() )
        pSh->Right(CRSR_SKIP_CHARS, sal_True, 1, sal_False );

    pSh->NormalizePam();

    sal_uInt16 nGroup = rMgr.GetGroup(sal_False, pCurFld->GetTypeId(), pCurFld->GetSubType());

    if (nGroup != pTabPage->GetGroup())
        pTabPage = (SwFldPage*)CreatePage(nGroup);

    pTabPage->EditNewField();

    Init();

    return 0;
}

IMPL_LINK_NOARG(SwFldEditDlg, AddressHdl)
{
    SwFldPage* pTabPage = (SwFldPage*)GetTabPage();
    SwFldMgr& rMgr = pTabPage->GetFldMgr();
    SwField *pCurFld = rMgr.GetCurFld();

    SfxItemSet aSet( pSh->GetAttrPool(),
                        SID_FIELD_GRABFOCUS, SID_FIELD_GRABFOCUS,
                        0L );

    sal_uInt16 nEditPos = UNKNOWN_EDIT;

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
    SwAbstractDialogFactory* pFact = swui::GetFactory();
    OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

    SfxAbstractDialog* pDlg = pFact->CreateSfxDialog( this, aSet,
        pSh->GetView().GetViewFrame()->GetFrame().GetFrameInterface(),
        RC_DLG_ADDR );
    OSL_ENSURE(pDlg, "Dialogdiet fail!");
    if(RET_OK == pDlg->Execute())
    {
        pSh->UpdateFlds( *pCurFld );
    }
    delete pDlg;
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
