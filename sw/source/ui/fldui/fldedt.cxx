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

#include <config_features.h>

#include <sfx2/basedlgs.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/request.hxx>
#include <sfx2/app.hxx>
#include <svx/optgenrl.hxx>
#include <svx/svxids.hrc>
#include <docufld.hxx>
#include <expfld.hxx>
#include <view.hxx>
#include <dbfld.hxx>
#include <wrtsh.hxx>
#include "flddb.hxx"
#include "flddinf.hxx"
#include "fldvar.hxx"
#include "flddok.hxx"
#include "fldfunc.hxx"
#include "fldref.hxx"
#include <fldedt.hxx>

#include <cmdid.h>
#include <globals.hrc>
#include <swabstdlg.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <memory>
#include <swuiexp.hxx>

void SwFieldEditDlg::EnsureSelection(SwField *pCurField, SwFieldMgr &rMgr)
{
    if (pSh->CursorInsideInputField())
    {
        // move cursor to start of Input Field
        SwInputField* pInputField = dynamic_cast<SwInputField*>(pCurField);
        if (pInputField && pInputField->GetFormatField())
        {
            pSh->GotoField( *(pInputField->GetFormatField()) );
        }
        else
        {
            SwSetExpField *const pSetField(dynamic_cast<SwSetExpField*>(pCurField));
            if (pSetField)
            {
                assert(pSetField->GetFormatField());
                pSh->GotoField( *(pSetField->GetFormatField()) );
            }
            else
            {
                assert(!"what input field is this");
            }
        }
    }

    /* Only create selection if there is none already.
       Normalize PaM instead of swapping. */
    if (!pSh->HasSelection())
    {
        SwShellCursor* pCursor = pSh->getShellCursor(true);
        SwPosition aOrigPos(*pCursor->GetPoint());

        //After this attempt it is possible that rMgr.GetCurField() != pCurField if
        //the field was in e.g. a zero height portion and so invisible in which
        //case it will be skipped over
        pSh->Right(CRSR_SKIP_CHARS, true, 1, false );
        //So (fdo#50640) if it didn't work then reposition back to the original
        //location where the field was
        SwField *pRealCurField = rMgr.GetCurField();
        bool bSelectionFailed = pCurField != pRealCurField;
        if (bSelectionFailed)
        {
            pCursor->DeleteMark();
            *pCursor->GetPoint() = aOrigPos;
        }
    }

    pSh->NormalizePam();

    assert(pCurField == rMgr.GetCurField());
}

SwFieldEditDlg::SwFieldEditDlg(SwView const & rVw)
    : SfxSingleTabDialogController(rVw.GetViewFrame()->GetWindow().GetFrameWeld(), nullptr,
        "modules/swriter/ui/editfielddialog.ui", "EditFieldDialog")
    , pSh(rVw.GetWrtShellPtr())
    , m_xPrevBT(m_xBuilder->weld_button("prev"))
    , m_xNextBT(m_xBuilder->weld_button("next"))
    , m_xAddressBT(m_xBuilder->weld_button("edit"))
{
    SwFieldMgr aMgr(pSh);

    SwField *pCurField = aMgr.GetCurField();
    if (!pCurField)
        return;

    SwViewShell::SetCareDialog(m_xDialog);

    EnsureSelection(pCurField, aMgr);

    sal_uInt16 nGroup = SwFieldMgr::GetGroup(pCurField->GetTypeId(), pCurField->GetSubType());

    CreatePage(nGroup);

    GetOKButton().connect_clicked(LINK(this, SwFieldEditDlg, OKHdl));

    m_xPrevBT->connect_clicked(LINK(this, SwFieldEditDlg, NextPrevHdl));
    m_xNextBT->connect_clicked(LINK(this, SwFieldEditDlg, NextPrevHdl));

    m_xAddressBT->connect_clicked(LINK(this, SwFieldEditDlg, AddressHdl));

    Init();
}

// initialise controls
void SwFieldEditDlg::Init()
{
    VclPtr<SwFieldPage> pTabPage = static_cast<SwFieldPage*>(GetTabPage());

    if( pTabPage )
    {
        SwFieldMgr& rMgr = pTabPage->GetFieldMgr();

        SwField *pCurField = rMgr.GetCurField();

        if(!pCurField)
            return;

        // Traveling only when more than one field
        pSh->StartAction();
        pSh->ClearMark();
        pSh->CreateCursor();

        bool bMove = rMgr.GoNext();
        if( bMove )
            rMgr.GoPrev();
        m_xNextBT->set_sensitive(bMove);

        bMove = rMgr.GoPrev();
        if( bMove )
            rMgr.GoNext();
        m_xPrevBT->set_sensitive( bMove );

        if (pCurField->GetTypeId() == TYP_EXTUSERFLD)
            m_xAddressBT->set_sensitive(true);
        else
            m_xAddressBT->set_sensitive(false);

        pSh->DestroyCursor();
        pSh->EndAction();
    }

    GetOKButton().set_sensitive(!pSh->IsReadOnlyAvailable() ||
                                !pSh->HasReadonlySel());
}

VclPtr<SfxTabPage> SwFieldEditDlg::CreatePage(sal_uInt16 nGroup)
{
    TabPageParent pPageParent(get_content_area(), this);

    // create TabPage
    VclPtr<SfxTabPage> pTabPage;

    switch (nGroup)
    {
        case GRP_DOC:
            pTabPage = SwFieldDokPage::Create(pPageParent, nullptr);
            break;
        case GRP_FKT:
            pTabPage = SwFieldFuncPage::Create(pPageParent, nullptr);
            break;
        case GRP_REF:
            pTabPage = SwFieldRefPage::Create(pPageParent, nullptr);
            break;
        case GRP_REG:
            {
                SfxObjectShell* pDocSh = SfxObjectShell::Current();
                SfxItemSet* pSet = new SfxItemSet( pDocSh->GetPool(), svl::Items<SID_DOCINFO, SID_DOCINFO>{} );
                using namespace ::com::sun::star;
                uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
                    pDocSh->GetModel(), uno::UNO_QUERY_THROW);
                uno::Reference<document::XDocumentProperties> xDocProps
                    = xDPS->getDocumentProperties();
                uno::Reference< beans::XPropertySet > xUDProps(
                    xDocProps->getUserDefinedProperties(),
                    uno::UNO_QUERY_THROW);
                pSet->Put( SfxUnoAnyItem( SID_DOCINFO, uno::makeAny(xUDProps) ) );
                pTabPage = SwFieldDokInfPage::Create(pPageParent, pSet);
                break;
            }
#if HAVE_FEATURE_DBCONNECTIVITY
        case GRP_DB:
            pTabPage = SwFieldDBPage::Create(pPageParent, nullptr);
            static_cast<SwFieldDBPage*>(pTabPage.get())->SetWrtShell(*pSh);
            break;
#endif
        case GRP_VAR:
            pTabPage = SwFieldVarPage::Create(pPageParent, nullptr);
            break;

    }

    assert(pTabPage);

    if (pTabPage)
    {
        static_cast<SwFieldPage*>(pTabPage.get())->SetWrtShell(pSh);
        SetTabPage(pTabPage);
    }

    return pTabPage;
}

SwFieldEditDlg::~SwFieldEditDlg()
{
    SwViewShell::SetCareDialog(nullptr);
    pSh->EnterStdMode();
}

void SwFieldEditDlg::EnableInsert(bool bEnable)
{
    if( bEnable && pSh->IsReadOnlyAvailable() && pSh->HasReadonlySel() )
        bEnable = false;
    GetOKButton().set_sensitive(bEnable);
}

void SwFieldEditDlg::InsertHdl()
{
    GetOKButton().clicked();
}

// kick off changing of the field
IMPL_LINK_NOARG(SwFieldEditDlg, OKHdl, weld::Button&, void)
{
    if (GetOKButton().get_sensitive())
    {
        VclPtr<SfxTabPage> pTabPage = GetTabPage();
        if (pTabPage)
            pTabPage->FillItemSet(nullptr);
        m_xDialog->response(RET_OK);
    }
}

short SwFieldEditDlg::run()
{
    // without TabPage no dialog
    return GetTabPage() ? SfxSingleTabDialogController::run() : static_cast<short>(RET_CANCEL);
}

// Traveling between fields of the same type
IMPL_LINK(SwFieldEditDlg, NextPrevHdl, weld::Button&, rButton, void)
{
    bool bNext = &rButton == m_xNextBT.get();

    pSh->EnterStdMode();

    SwFieldType *pOldTyp = nullptr;
    VclPtr<SwFieldPage> pTabPage = static_cast<SwFieldPage*>(GetTabPage());

    //#112462# FillItemSet may delete the current field
    //that's why it has to be called before accessing the current field
    if (GetOKButton().get_sensitive())
        pTabPage->FillItemSet(nullptr);

    SwFieldMgr& rMgr = pTabPage->GetFieldMgr();
    SwField *pCurField = rMgr.GetCurField();
    if (pCurField->GetTypeId() == TYP_DBFLD)
        pOldTyp = static_cast<SwDBFieldType*>(pCurField->GetTyp());

    rMgr.GoNextPrev( bNext, pOldTyp );
    pCurField = rMgr.GetCurField();

    EnsureSelection(pCurField, rMgr);

    sal_uInt16 nGroup = SwFieldMgr::GetGroup(pCurField->GetTypeId(), pCurField->GetSubType());

    if (nGroup != pTabPage->GetGroup())
        pTabPage = static_cast<SwFieldPage*>(CreatePage(nGroup).get());

    pTabPage->EditNewField();

    Init();
}

IMPL_LINK_NOARG(SwFieldEditDlg, AddressHdl, weld::Button&, void)
{
    SwFieldPage* pTabPage = static_cast<SwFieldPage*>(GetTabPage());
    SwFieldMgr& rMgr = pTabPage->GetFieldMgr();
    SwField *pCurField = rMgr.GetCurField();

    SfxItemSet aSet( pSh->GetAttrPool(),
                        svl::Items<SID_FIELD_GRABFOCUS, SID_FIELD_GRABFOCUS>{} );

    EditPosition nEditPos = EditPosition::UNKNOWN;

    switch(pCurField->GetSubType())
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
    SwAbstractDialogFactory& rFact = swui::GetFactory();

    ScopedVclPtr<SfxAbstractDialog> pDlg(rFact.CreateSwAddressAbstractDlg(m_xDialog.get(), aSet));
    if (RET_OK == pDlg->Execute())
    {
        pSh->UpdateOneField(*pCurField);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
