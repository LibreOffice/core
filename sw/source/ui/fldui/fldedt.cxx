/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

SwFldEditDlg::SwFldEditDlg(SwView& rVw)
    : SfxSingleTabDialog(&rVw.GetViewFrame()->GetWindow(), 0,
        "EditFieldDialog", "modules/swriter/ui/editfielddialog.ui")
    , pSh(rVw.GetWrtShellPtr())
{
    get(m_pPrevBT, "prev");
    get(m_pNextBT, "next");
    get(m_pAddressBT, "edit");

    SwFldMgr aMgr(pSh);

    SwField *pCurFld = aMgr.GetCurFld();
    if(!pCurFld)
        return;

    pSh->SetCareWin(this);

    /* #108536# Only create selection if there is none
        already. Normalize PaM instead of swapping. */
    if ( ! pSh->HasSelection() )
        pSh->Right(CRSR_SKIP_CHARS, true, 1, false);

    pSh->NormalizePam();

    sal_uInt16 nGroup = aMgr.GetGroup(false, pCurFld->GetTypeId(), pCurFld->GetSubType());

    CreatePage(nGroup);

    GetOKButton()->SetClickHdl(LINK(this, SwFldEditDlg, OKHdl));

    m_pPrevBT->SetClickHdl(LINK(this, SwFldEditDlg, NextPrevHdl));
    m_pNextBT->SetClickHdl(LINK(this, SwFldEditDlg, NextPrevHdl));

    m_pAddressBT->SetClickHdl(LINK(this, SwFldEditDlg, AddressHdl));

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
        m_pNextBT->Enable(bMove);

        if( 0 != ( bMove = rMgr.GoPrev() ) )
            rMgr.GoNext();
        m_pPrevBT->Enable( bMove );

        if (pCurFld->GetTypeId() == TYP_EXTUSERFLD)
            m_pAddressBT->Show();

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

    switch (nGroup)
    {
        case GRP_DOC:
            pTabPage = SwFldDokPage::Create(get_content_area(), *(SfxItemSet*)0);
            break;
        case GRP_FKT:
            pTabPage = SwFldFuncPage::Create(get_content_area(), *(SfxItemSet*)0);
            break;
        case GRP_REF:
            pTabPage = SwFldRefPage::Create(get_content_area(), *(SfxItemSet*)0);
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
                pTabPage = SwFldDokInfPage::Create(get_content_area(), *pSet);
                break;
            }
        case GRP_DB:
            pTabPage = SwFldDBPage::Create(get_content_area(), *(SfxItemSet*)0);
            static_cast<SwFldDBPage*>(pTabPage)->SetWrtShell(*pSh);
            break;
        case GRP_VAR:
            pTabPage = SwFldVarPage::Create(get_content_area(), *(SfxItemSet*)0);
            break;

    }

    static_cast<SwFldPage*>(pTabPage)->SetWrtShell(pSh);

    setTabPage(pTabPage);

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
    return GetTabPage() ? Dialog::Execute() : static_cast<short>(RET_CANCEL);
}

/*--------------------------------------------------------------------
    Description: Traveling between fields of the same type
 --------------------------------------------------------------------*/
IMPL_LINK( SwFldEditDlg, NextPrevHdl, Button *, pButton )
{
    bool bNext = pButton == m_pNextBT;

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
