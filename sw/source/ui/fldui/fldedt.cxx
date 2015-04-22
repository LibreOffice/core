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
#include <expfld.hxx>
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
#include <boost/scoped_ptr.hpp>
#include <swuiexp.hxx>

void SwFldEditDlg::EnsureSelection(SwField *pCurFld, SwFldMgr &rMgr)
{
    if (pSh->CrsrInsideInputFld())
    {
        // move cursor to start of Input Field
        SwInputField* pInputFld = dynamic_cast<SwInputField*>(pCurFld);
        if (pInputFld && pInputFld->GetFmtFld())
        {
            pSh->GotoField( *(pInputFld->GetFmtFld()) );
        }
    }

    /* Only create selection if there is none already.
       Normalize PaM instead of swapping. */
    if (!pSh->HasSelection())
    {
        SwShellCrsr* pCrsr = pSh->getShellCrsr(true);
        SwPosition aOrigPos(*pCrsr->GetPoint());

        //After this attempt it is possible that rMgr.GetCurFld() != pCurFld if
        //the field was in e.g. a zero height portion and so invisible in which
        //case it will be skipped over
        pSh->Right(CRSR_SKIP_CHARS, true, 1, false );
        //So (fdo#50640) if it didn't work then reposition back to the original
        //location where the field was
        SwField *pRealCurFld = rMgr.GetCurFld();
        bool bSelectionFailed = pCurFld != pRealCurFld;
        if (bSelectionFailed)
        {
            pCrsr->DeleteMark();
            *pCrsr->GetPoint() = aOrigPos;
        }
    }

    pSh->NormalizePam();

    assert(pCurFld == rMgr.GetCurFld());
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
    if (!pCurFld)
        return;

    SwViewShell::SetCareWin(this);

    EnsureSelection(pCurFld, aMgr);

    sal_uInt16 nGroup = aMgr.GetGroup(false, pCurFld->GetTypeId(), pCurFld->GetSubType());

    CreatePage(nGroup);

    GetOKButton()->SetClickHdl(LINK(this, SwFldEditDlg, OKHdl));

    m_pPrevBT->SetClickHdl(LINK(this, SwFldEditDlg, NextPrevHdl));
    m_pNextBT->SetClickHdl(LINK(this, SwFldEditDlg, NextPrevHdl));

    m_pAddressBT->SetClickHdl(LINK(this, SwFldEditDlg, AddressHdl));

    Init();
}

// initialise controls
void SwFldEditDlg::Init()
{
    VclPtr<SwFldPage> pTabPage = static_cast<SwFldPage*>(GetTabPage());

    if( pTabPage )
    {
        SwFldMgr& rMgr = pTabPage->GetFldMgr();

        SwField *pCurFld = rMgr.GetCurFld();

        if(!pCurFld)
            return;

        // Traveling only when more than one field
        pSh->StartAction();
        pSh->CreateCrsr();

        bool bMove = rMgr.GoNext();
        if( bMove )
            rMgr.GoPrev();
        m_pNextBT->Enable(bMove);

        if( ( bMove = rMgr.GoPrev() ) )
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

VclPtr<SfxTabPage> SwFldEditDlg::CreatePage(sal_uInt16 nGroup)
{
    // create TabPage
    VclPtr<SfxTabPage> pTabPage;

    switch (nGroup)
    {
        case GRP_DOC:
            pTabPage = SwFldDokPage::Create(get_content_area(), 0);
            break;
        case GRP_FKT:
            pTabPage = SwFldFuncPage::Create(get_content_area(), 0);
            break;
        case GRP_REF:
            pTabPage = SwFldRefPage::Create(get_content_area(), 0);
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
                pTabPage = SwFldDokInfPage::Create(get_content_area(), pSet);
                break;
            }
        case GRP_DB:
            pTabPage = SwFldDBPage::Create(get_content_area(), 0);
            static_cast<SwFldDBPage*>(pTabPage.get())->SetWrtShell(*pSh);
            break;
        case GRP_VAR:
            pTabPage = SwFldVarPage::Create(get_content_area(), 0);
            break;

    }

    assert(pTabPage);

    if (pTabPage)
    {
        static_cast<SwFldPage*>(pTabPage.get())->SetWrtShell(pSh);
        SetTabPage(pTabPage);
    }

    return pTabPage;
}

SwFldEditDlg::~SwFldEditDlg()
{
    disposeOnce();
}

void SwFldEditDlg::dispose()
{
    SwViewShell::SetCareWin(NULL);
    pSh->EnterStdMode();
    m_pPrevBT.clear();
    m_pNextBT.clear();
    m_pAddressBT.clear();
    SfxSingleTabDialog::dispose();
}

void SwFldEditDlg::EnableInsert(bool bEnable)
{
    if( bEnable && pSh->IsReadOnlyAvailable() && pSh->HasReadonlySel() )
        bEnable = false;
    GetOKButton()->Enable( bEnable );
}

void SwFldEditDlg::InsertHdl()
{
    GetOKButton()->Click();
}

// kick off changing of the field
IMPL_LINK_NOARG(SwFldEditDlg, OKHdl)
{
    if (GetOKButton()->IsEnabled())
    {
        VclPtr<SfxTabPage> pTabPage = GetTabPage();
        if (pTabPage)
            pTabPage->FillItemSet(0);
        EndDialog( RET_OK );
    }

    return 0;
}

short SwFldEditDlg::Execute()
{
    // without TabPage no dialog
    return GetTabPage() ? Dialog::Execute() : static_cast<short>(RET_CANCEL);
}

// Traveling between fields of the same type
IMPL_LINK( SwFldEditDlg, NextPrevHdl, Button *, pButton )
{
    bool bNext = pButton == m_pNextBT;

    pSh->EnterStdMode();

    SwFieldType *pOldTyp = 0;
    VclPtr<SwFldPage> pTabPage = static_cast<SwFldPage*>(GetTabPage());

    //#112462# FillItemSet may delete the current field
    //that's why it has to be called before accessing the current field
    if( GetOKButton()->IsEnabled() )
        pTabPage->FillItemSet(0);

    SwFldMgr& rMgr = pTabPage->GetFldMgr();
    SwField *pCurFld = rMgr.GetCurFld();
    if (pCurFld->GetTypeId() == TYP_DBFLD)
        pOldTyp = static_cast<SwDBFieldType*>(pCurFld->GetTyp());

    rMgr.GoNextPrev( bNext, pOldTyp );
    pCurFld = rMgr.GetCurFld();

    EnsureSelection(pCurFld, rMgr);

    sal_uInt16 nGroup = rMgr.GetGroup(false, pCurFld->GetTypeId(), pCurFld->GetSubType());

    if (nGroup != pTabPage->GetGroup())
        pTabPage = static_cast<SwFldPage*>(CreatePage(nGroup).get());

    pTabPage->EditNewField();

    Init();

    return 0;
}

IMPL_LINK_NOARG(SwFldEditDlg, AddressHdl)
{
    SwFldPage* pTabPage = static_cast<SwFldPage*>(GetTabPage());
    SwFldMgr& rMgr = pTabPage->GetFldMgr();
    SwField *pCurFld = rMgr.GetCurFld();

    SfxItemSet aSet( pSh->GetAttrPool(),
                        SID_FIELD_GRABFOCUS, SID_FIELD_GRABFOCUS,
                        0L );

    EditPosition nEditPos = EditPosition::UNKNOWN;

    switch(pCurFld->GetSubType())
    {
        case EU_FIRSTNAME:  nEditPos = EditPosition::FIRSTNAME;  break;
        case EU_NAME:       nEditPos = EditPosition::LASTNAME;   break;
        case EU_SHORTCUT:   nEditPos = EditPosition::SHORTNAME;  break;
        case EU_COMPANY:    nEditPos = EditPosition::COMPANY;    break;
        case EU_STREET:     nEditPos = EditPosition::STREET;     break;
        case EU_TITLE:      nEditPos = EditPosition::TITLE;      break;
        case EU_POSITION:   nEditPos = EditPosition::POSITION;   break;
        case EU_PHONE_PRIVATE:nEditPos = EditPosition::TELPRIV;  break;
        case EU_PHONE_COMPANY:nEditPos = EditPosition::TELCOMPANY;   break;
        case EU_FAX:        nEditPos = EditPosition::FAX;        break;
        case EU_EMAIL:      nEditPos = EditPosition::EMAIL;      break;
        case EU_COUNTRY:    nEditPos = EditPosition::COUNTRY;    break;
        case EU_ZIP:        nEditPos = EditPosition::PLZ;        break;
        case EU_CITY:       nEditPos = EditPosition::CITY;       break;
        case EU_STATE:      nEditPos = EditPosition::STATE;      break;

        default:            nEditPos = EditPosition::UNKNOWN;    break;

    }
    aSet.Put(SfxUInt16Item(SID_FIELD_GRABFOCUS, static_cast<sal_uInt16>(nEditPos)));
    SwAbstractDialogFactory* pFact = swui::GetFactory();
    OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

    boost::scoped_ptr<SfxAbstractDialog> pDlg(pFact->CreateSfxDialog( this, aSet,
        pSh->GetView().GetViewFrame()->GetFrame().GetFrameInterface(),
        RC_DLG_ADDR ));
    OSL_ENSURE(pDlg, "Dialog creation failed!");
    if(RET_OK == pDlg->Execute())
    {
        pSh->UpdateFlds( *pCurFld );
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
