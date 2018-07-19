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

#include "mmgreetingspage.hxx"
#include <mailmergewizard.hxx>
#include <mmconfigitem.hxx>
#include "mmaddressblockpage.hxx"
#include <swtypes.hxx>
#include <dbui.hrc>
#include <com/sun/star/sdb/XColumn.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <swmodule.hxx>
#include <view.hxx>

using namespace svt;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

static void lcl_FillGreetingsBox(ListBox& rBox,
                        SwMailMergeConfigItem const & rConfig,
                        SwMailMergeConfigItem::Gender eType)
{
    const Sequence< OUString> rEntries = rConfig.GetGreetings(eType);
    for(sal_Int32 nEntry = 0; nEntry < rEntries.getLength(); ++nEntry)
        rBox.InsertEntry(rEntries[nEntry]);
    rBox.SelectEntryPos(rConfig.GetCurrentGreeting(eType));
}

static void lcl_FillGreetingsBox(ComboBox& rBox,
                        SwMailMergeConfigItem const & rConfig,
                        SwMailMergeConfigItem::Gender eType)
{
    const Sequence< OUString> rEntries = rConfig.GetGreetings(eType);
    for(sal_Int32 nEntry = 0; nEntry < rEntries.getLength(); ++nEntry)
        rBox.InsertEntry(rEntries[nEntry]);
    rBox.SelectEntryPos(rConfig.GetCurrentGreeting(eType));
}

static void lcl_StoreGreetingsBox(ListBox const & rBox,
                        SwMailMergeConfigItem& rConfig,
                        SwMailMergeConfigItem::Gender eType)
{
    Sequence< OUString> aEntries(rBox.GetEntryCount());
    OUString* pEntries = aEntries.getArray();
    for(sal_Int32 nEntry = 0; nEntry < rBox.GetEntryCount(); ++nEntry)
        pEntries[nEntry] = rBox.GetEntry(nEntry);
    rConfig.SetGreetings(eType, aEntries);
    rConfig.SetCurrentGreeting(eType, rBox.GetSelectedEntryPos());
}

static void lcl_StoreGreetingsBox(ComboBox const & rBox,
                        SwMailMergeConfigItem& rConfig,
                        SwMailMergeConfigItem::Gender eType)
{
    Sequence< OUString> aEntries(rBox.GetEntryCount());
    OUString* pEntries = aEntries.getArray();
    for(sal_Int32 nEntry = 0; nEntry < rBox.GetEntryCount(); ++nEntry)
        pEntries[nEntry] = rBox.GetEntry(nEntry);
    rConfig.SetGreetings(eType, aEntries);
    rConfig.SetCurrentGreeting(eType, rBox.GetSelectedEntryPos());
}

IMPL_LINK_NOARG(SwGreetingsHandler, IndividualHdl_Impl, Button*, void)
{
    bool bIndividual = m_pPersonalizedCB->IsEnabled() && m_pPersonalizedCB->IsChecked();
    m_pFemaleFT->Enable(bIndividual);
    m_pFemaleLB->Enable(bIndividual);
    m_pFemalePB->Enable(bIndividual);
    m_pMaleFT->Enable(bIndividual);
    m_pMaleLB->Enable(bIndividual);
    m_pMalePB->Enable(bIndividual);
    m_pFemaleFI->Enable(bIndividual);
    m_pFemaleColumnFT->Enable(bIndividual);
    m_pFemaleColumnLB->Enable(bIndividual);
    m_pFemaleFieldFT->Enable(bIndividual);
    m_pFemaleFieldCB->Enable(bIndividual);

    m_pNeutralFT->Enable(!bIndividual);
    m_pNeutralCB->Enable(!bIndividual);

    if( m_bIsTabPage )
    {
        m_rConfigItem.SetIndividualGreeting(bIndividual, false);
        m_pWizard->UpdateRoadmap();
        m_pWizard->enableButtons(WizardButtonFlags::NEXT, m_pWizard->isStateEnabled(MM_LAYOUTPAGE));
    }
    UpdatePreview();
}

IMPL_LINK(SwGreetingsHandler, GreetingHdl_Impl, Button*, pButton, void)
{
    ScopedVclPtr<SwCustomizeAddressBlockDialog> pDlg(
            VclPtr<SwCustomizeAddressBlockDialog>::Create(pButton, m_rConfigItem,
                        pButton == m_pMalePB ?
                        SwCustomizeAddressBlockDialog::GREETING_MALE :
                        SwCustomizeAddressBlockDialog::GREETING_FEMALE ));
    if(RET_OK == pDlg->Execute())
    {
        ListBox* pToInsert = pButton == m_pMalePB ? m_pMaleLB.get() : m_pFemaleLB.get();
        pToInsert->SelectEntryPos(pToInsert->InsertEntry(pDlg->GetAddress()));
        if(m_bIsTabPage)
        {
            m_pWizard->UpdateRoadmap();
            m_pWizard->enableButtons(WizardButtonFlags::NEXT, m_pWizard->isStateEnabled(MM_LAYOUTPAGE));
        }
        UpdatePreview();
    }
}

void    SwGreetingsHandler::UpdatePreview()
{
    //the base class does nothing
}

IMPL_LINK(SwMailMergeGreetingsPage, AssignHdl_Impl, Button*, pButton, void)
{
    const OUString sPreview(m_pFemaleLB->GetSelectedEntry() + "\n" + m_pMaleLB->GetSelectedEntry());
    ScopedVclPtr<SwAssignFieldsDialog> pDlg(
            VclPtr<SwAssignFieldsDialog>::Create(pButton, m_rConfigItem, sPreview, false));
    if(RET_OK == pDlg->Execute())
    {
        UpdatePreview();
        m_pWizard->UpdateRoadmap();
        m_pWizard->enableButtons(WizardButtonFlags::NEXT, m_pWizard->isStateEnabled(MM_LAYOUTPAGE));
    }
}

IMPL_LINK_NOARG(SwMailMergeGreetingsPage, GreetingSelectHdl_Impl, Edit&, void)
{
    UpdatePreview();
}
IMPL_LINK_NOARG(SwMailMergeGreetingsPage, GreetingSelectListBoxHdl_Impl, ListBox&, void)
{
    UpdatePreview();
}
IMPL_LINK_NOARG(SwMailMergeGreetingsPage, GreetingSelectComboBoxHdl_Impl, ComboBox&, void)
{
    UpdatePreview();
}

void SwMailMergeGreetingsPage::UpdatePreview()
{
    //find out which type of greeting should be selected:
    bool bFemale = false;
    bool bNoValue = !m_pFemaleColumnLB->IsEnabled();
    if( !bNoValue )
    {
        const OUString sFemaleValue = m_pFemaleFieldCB->GetText();
        const OUString sFemaleColumn = m_pFemaleColumnLB->GetSelectedEntry();
        Reference< sdbcx::XColumnsSupplier > xColsSupp( m_rConfigItem.GetResultSet(), UNO_QUERY);
        Reference < container::XNameAccess> xColAccess = xColsSupp.is() ? xColsSupp->getColumns() : nullptr;
        if(!sFemaleValue.isEmpty() && !sFemaleColumn.isEmpty() &&
                xColAccess.is() &&
                xColAccess->hasByName(sFemaleColumn))
        {
            //get the content and exchange it in the address string
            Any aCol = xColAccess->getByName(sFemaleColumn);
            Reference< sdb::XColumn > xColumn;
            aCol >>= xColumn;
            if(xColumn.is())
            {
                try
                {
                    bFemale = xColumn->getString() == sFemaleValue;

                    //no last name value marks the greeting also as neutral
                    const OUString sLastNameColumn =
                        m_rConfigItem.GetAssignedColumn(MM_PART_LASTNAME);
                    if ( xColAccess->hasByName(sLastNameColumn) )
                    {
                        aCol = xColAccess->getByName(sLastNameColumn);
                        aCol >>= xColumn;
                        bNoValue = xColumn->getString().isEmpty();
                    }
                }
                catch (const sdbc::SQLException&)
                {
                    OSL_FAIL("SQLException caught");
                }
            }
        }
    }

    OUString sPreview = bFemale ? m_pFemaleLB->GetSelectedEntry() :
        bNoValue ? m_pNeutralCB->GetText() : m_pMaleLB->GetSelectedEntry();

    sPreview = SwAddressPreview::FillData(sPreview, m_rConfigItem);
    m_pPreviewWIN->SetAddress(sPreview);
}

void    SwGreetingsHandler::Contains(bool bContainsGreeting)
{
    m_pPersonalizedCB->Enable(bContainsGreeting);
    bool bEnablePersonal = bContainsGreeting && m_pPersonalizedCB->IsChecked();
    m_pFemaleFT->Enable(bEnablePersonal);
    m_pFemaleLB->Enable(bEnablePersonal);
    m_pFemalePB->Enable(bEnablePersonal);
    m_pMaleFT->Enable(bEnablePersonal);
    m_pMaleLB->Enable(bEnablePersonal);
    m_pMalePB->Enable(bEnablePersonal);
    m_pFemaleFI->Enable(bEnablePersonal);
    m_pFemaleColumnFT->Enable(bEnablePersonal);
    m_pFemaleColumnLB->Enable(bEnablePersonal);
    m_pFemaleFieldFT->Enable(bEnablePersonal);
    m_pFemaleFieldCB->Enable(bEnablePersonal);

    m_pNeutralFT->Enable(bContainsGreeting);
    m_pNeutralCB->Enable(bContainsGreeting);
}

SwMailMergeGreetingsPage::SwMailMergeGreetingsPage(SwMailMergeWizard* _pParent)
    : svt::OWizardPage(_pParent, "MMSalutationPage",
        "modules/swriter/ui/mmsalutationpage.ui"),
    SwGreetingsHandler(_pParent->GetConfigItem())
{
    m_pWizard = _pParent;

    get(m_pGreetingLineCB, "greeting");
    get(m_pPersonalizedCB, "personalized");
    get(m_pFemaleFT, "femaleft");
    get(m_pFemaleLB, "female");
    get(m_pFemalePB, "newfemale");
    get(m_pMaleFT, "maleft");
    get(m_pMaleLB, "male");
    get(m_pMalePB, "newmale");
    get(m_pFemaleFI, "femalefi");
    get(m_pFemaleColumnFT, "femalecolft");
    get(m_pFemaleColumnLB, "femalecol");
    get(m_pFemaleFieldFT, "femalefieldft");
    get(m_pFemaleFieldCB, "femalefield");
    get(m_pNeutralFT, "generalft");
    get(m_pNeutralCB, "general");
    get(m_pPreviewFI, "previewft");
    get(m_pPreviewWIN, "preview");
    Size aSize(LogicToPixel(Size(186, 21), MapMode(MapUnit::MapAppFont)));
    m_pPreviewWIN->set_width_request(aSize.Width());
    m_pPreviewWIN->set_height_request(aSize.Height());
    get(m_pAssignPB, "assign");
    get(m_pDocumentIndexFI, "documentindex");
    m_sDocument = m_pDocumentIndexFI->GetText();
    get(m_pPrevSetIB, "prev");
    get(m_pNextSetIB, "next");

    m_bIsTabPage = true;

    m_pGreetingLineCB->SetClickHdl(LINK(this, SwMailMergeGreetingsPage, ContainsHdl_Impl));
    Link<Button*,void> aIndividualLink = LINK(this, SwGreetingsHandler, IndividualHdl_Impl);
    m_pPersonalizedCB->SetClickHdl(aIndividualLink);
    Link<Button*,void> aGreetingLink = LINK(this, SwGreetingsHandler, GreetingHdl_Impl);
    m_pFemalePB->SetClickHdl(aGreetingLink);
    m_pMalePB->SetClickHdl(aGreetingLink);
    m_pAssignPB->SetClickHdl(LINK(this, SwMailMergeGreetingsPage, AssignHdl_Impl));
    Link<Edit&,void> aLBoxLink = LINK(this, SwMailMergeGreetingsPage, GreetingSelectHdl_Impl);
    Link<ListBox&,void> aLBoxLink2 = LINK(this, SwMailMergeGreetingsPage, GreetingSelectListBoxHdl_Impl);
    m_pFemaleLB->SetSelectHdl(aLBoxLink2);
    m_pMaleLB->SetSelectHdl(aLBoxLink2);
    m_pFemaleColumnLB->SetSelectHdl(aLBoxLink2);
    m_pFemaleFieldCB->SetSelectHdl(LINK(this, SwMailMergeGreetingsPage, GreetingSelectComboBoxHdl_Impl));
    m_pFemaleFieldCB->SetModifyHdl(aLBoxLink);
    m_pNeutralCB->SetSelectHdl(LINK(this, SwMailMergeGreetingsPage, GreetingSelectComboBoxHdl_Impl));
    m_pNeutralCB->SetModifyHdl(aLBoxLink);

    Link<Button*,void> aDataLink = LINK(this, SwMailMergeGreetingsPage, InsertDataHdl_Impl);
    m_pPrevSetIB->SetClickHdl(aDataLink);
    m_pNextSetIB->SetClickHdl(aDataLink);

    m_pGreetingLineCB->Check(m_rConfigItem.IsGreetingLine(false));
    m_pPersonalizedCB->Check(m_rConfigItem.IsIndividualGreeting(false));
    ContainsHdl_Impl(m_pGreetingLineCB);
    aIndividualLink.Call(nullptr);

    lcl_FillGreetingsBox(*m_pFemaleLB, m_rConfigItem, SwMailMergeConfigItem::FEMALE);
    lcl_FillGreetingsBox(*m_pMaleLB, m_rConfigItem, SwMailMergeConfigItem::MALE);
    lcl_FillGreetingsBox(*m_pNeutralCB, m_rConfigItem, SwMailMergeConfigItem::NEUTRAL);

    m_pDocumentIndexFI->SetText(m_sDocument.replaceFirst("%1", "1"));
}

SwMailMergeGreetingsPage::~SwMailMergeGreetingsPage()
{
    disposeOnce();
}

void SwMailMergeGreetingsPage::dispose()
{
    m_pPreviewFI.clear();
    m_pPreviewWIN.clear();
    m_pAssignPB.clear();
    m_pDocumentIndexFI.clear();
    m_pPrevSetIB.clear();
    m_pNextSetIB.clear();
    svt::OWizardPage::dispose();
}

void SwMailMergeGreetingsPage::ActivatePage()
{
    //try to find the gender setting
    m_pFemaleColumnLB->Clear();
    Reference< sdbcx::XColumnsSupplier > xColsSupp = m_rConfigItem.GetColumnsSupplier();
    if(xColsSupp.is())
    {
        Reference < container::XNameAccess> xColAccess = xColsSupp->getColumns();
        Sequence< OUString > aColumns = xColAccess->getElementNames();
        for(sal_Int32 nName = 0; nName < aColumns.getLength(); ++nName)
            m_pFemaleColumnLB->InsertEntry(aColumns[nName]);
    }

    m_pFemaleColumnLB->SelectEntry(m_rConfigItem.GetAssignedColumn(MM_PART_GENDER));
    m_pFemaleColumnLB->SaveValue();

    m_pFemaleFieldCB->SetText(m_rConfigItem.GetFemaleGenderValue());
    m_pFemaleFieldCB->SaveValue();

    UpdatePreview();
    m_pWizard->enableButtons(WizardButtonFlags::NEXT, m_pWizard->isStateEnabled(MM_LAYOUTPAGE));
}

bool SwMailMergeGreetingsPage::commitPage( ::svt::WizardTypes::CommitPageReason )
{
    if (m_pFemaleColumnLB->IsValueChangedFromSaved())
    {
        const SwDBData& rDBData = m_rConfigItem.GetCurrentDBData();
        Sequence< OUString> aAssignment = m_rConfigItem.GetColumnAssignment( rDBData );
        if(aAssignment.getLength() <= MM_PART_GENDER)
            aAssignment.realloc(MM_PART_GENDER + 1);
        aAssignment[MM_PART_GENDER] = m_pFemaleColumnLB->GetSelectedEntry();
        m_rConfigItem.SetColumnAssignment( rDBData, aAssignment );
    }
    if (m_pFemaleFieldCB->IsValueChangedFromSaved())
        m_rConfigItem.SetFemaleGenderValue(m_pFemaleFieldCB->GetText());

    lcl_StoreGreetingsBox(*m_pFemaleLB, m_rConfigItem, SwMailMergeConfigItem::FEMALE);
    lcl_StoreGreetingsBox(*m_pMaleLB, m_rConfigItem, SwMailMergeConfigItem::MALE);

    sal_Int32 nCurrentTextPos = m_pNeutralCB->GetEntryPos(m_pNeutralCB->GetText());
    if(COMBOBOX_ENTRY_NOTFOUND == nCurrentTextPos)
    {
        sal_Int32 nCount = m_pNeutralCB->GetEntryCount();
        m_pNeutralCB->InsertEntry(m_pNeutralCB->GetText(), nCount);
        m_pNeutralCB->SelectEntryPos(nCount);
    }
    lcl_StoreGreetingsBox(*m_pNeutralCB, m_rConfigItem, SwMailMergeConfigItem::NEUTRAL);
    m_rConfigItem.SetGreetingLine(m_pGreetingLineCB->IsChecked(), false);
    m_rConfigItem.SetIndividualGreeting(m_pPersonalizedCB->IsChecked(), false);
    return true;
}

IMPL_LINK(SwMailMergeGreetingsPage, ContainsHdl_Impl, Button*, pBox, void)
{
    bool bContainsGreeting = static_cast<CheckBox*>(pBox)->IsChecked();
    SwGreetingsHandler::Contains(bContainsGreeting);
    m_pPreviewFI-> Enable(bContainsGreeting);
    m_pPreviewWIN->Enable(bContainsGreeting);
    m_pAssignPB->  Enable(bContainsGreeting);
    m_pDocumentIndexFI->  Enable(bContainsGreeting);
    m_pPrevSetIB->Enable(bContainsGreeting);
    m_pNextSetIB->Enable(bContainsGreeting);
    m_rConfigItem.SetGreetingLine(m_pGreetingLineCB->IsChecked(), false);
    m_pWizard->UpdateRoadmap();
    m_pWizard->enableButtons(WizardButtonFlags::NEXT, m_pWizard->isStateEnabled(MM_LAYOUTPAGE));
}

IMPL_LINK(SwMailMergeGreetingsPage, InsertDataHdl_Impl, Button*, pButton, void)
{
    //if no pButton is given, the first set has to be pre-set
    if(!pButton)
    {
        m_rConfigItem.GetResultSet();
    }
    else
    {
        bool bNext = pButton == m_pNextSetIB;
        sal_Int32 nPos = m_rConfigItem.GetResultSetPosition();
        m_rConfigItem.MoveResultSet( bNext ? ++nPos : --nPos);
    }
    sal_Int32 nPos = m_rConfigItem.GetResultSetPosition();
    bool bEnable = true;
    if(nPos < 1)
    {
        bEnable = false;
        nPos = 1;
    }
    else
        UpdatePreview();
    m_pPrevSetIB->Enable(bEnable);
    m_pNextSetIB->Enable(bEnable);
    m_pDocumentIndexFI->Enable(bEnable);
    m_pDocumentIndexFI->SetText(m_sDocument.replaceFirst("%1", OUString::number(nPos)));
}

SwMailBodyDialog::SwMailBodyDialog(vcl::Window* pParent) :
    SfxModalDialog(pParent, "MailBodyDialog", "modules/swriter/ui/mmmailbody.ui"),
    SwGreetingsHandler(*GetActiveView()->GetMailMergeConfigItem())
{
    get(m_pGreetingLineCB, "greeting");
    get(m_pPersonalizedCB, "personalized");
    get(m_pFemaleFT, "femaleft");
    get(m_pFemaleLB, "female");
    get(m_pFemalePB, "newfemale");
    get(m_pMaleFT, "maleft");
    get(m_pMaleLB, "male");
    get(m_pMalePB, "newmale");
    get(m_pFemaleFI, "femalefi");
    get(m_pFemaleColumnFT, "femalecolft");
    get(m_pFemaleColumnLB, "femalecol");
    get(m_pFemaleFieldFT, "femalefieldft");
    get(m_pFemaleFieldCB, "femalefield");
    get(m_pNeutralFT, "generalft");
    get(m_pNeutralCB, "general");
    get(m_pBodyFT, "bodyft");
    get(m_pBodyMLE, "bodymle");
    m_pBodyMLE->SetStyle(m_pBodyMLE->GetStyle() | WB_HSCROLL | WB_VSCROLL | WB_IGNORETAB);
    Size aSize(LogicToPixel(Size(180, 50), MapMode(MapUnit::MapAppFont)));
    m_pBodyMLE->set_width_request(aSize.Width());
    m_pBodyMLE->set_height_request(aSize.Height());
    get(m_pOK, "ok");
    m_bIsTabPage = false;

    m_pGreetingLineCB->SetClickHdl(LINK(this, SwMailBodyDialog, ContainsHdl_Impl));
    Link<Button*,void> aIndividualLink = LINK(this, SwGreetingsHandler, IndividualHdl_Impl);
    m_pPersonalizedCB->SetClickHdl(aIndividualLink);
    Link<Button*,void> aGreetingLink = LINK(this, SwGreetingsHandler, GreetingHdl_Impl);
    m_pFemalePB->SetClickHdl(aGreetingLink);
    m_pMalePB->SetClickHdl(aGreetingLink);
    m_pOK->SetClickHdl(LINK(this, SwMailBodyDialog, OKHdl));

    m_pGreetingLineCB->Check(m_rConfigItem.IsGreetingLine(true));
    m_pPersonalizedCB->Check(m_rConfigItem.IsIndividualGreeting(true));
    ContainsHdl_Impl(m_pGreetingLineCB);
    aIndividualLink.Call(nullptr);

    lcl_FillGreetingsBox(*m_pFemaleLB, m_rConfigItem, SwMailMergeConfigItem::FEMALE);
    lcl_FillGreetingsBox(*m_pMaleLB, m_rConfigItem, SwMailMergeConfigItem::MALE);
    lcl_FillGreetingsBox(*m_pNeutralCB, m_rConfigItem, SwMailMergeConfigItem::NEUTRAL);

    //try to find the gender setting
    m_pFemaleColumnLB->Clear();
    Reference< sdbcx::XColumnsSupplier > xColsSupp = m_rConfigItem.GetColumnsSupplier();
    if(xColsSupp.is())
    {
        Reference < container::XNameAccess> xColAccess = xColsSupp->getColumns();
        Sequence< OUString > aColumns = xColAccess->getElementNames();
        for(sal_Int32 nName = 0; nName < aColumns.getLength(); ++nName)
            m_pFemaleColumnLB->InsertEntry(aColumns[nName]);
    }

    m_pFemaleColumnLB->SelectEntry(m_rConfigItem.GetAssignedColumn(MM_PART_GENDER));
    m_pFemaleColumnLB->SaveValue();

    m_pFemaleFieldCB->SetText(m_rConfigItem.GetFemaleGenderValue());
    m_pFemaleFieldCB->SaveValue();
}

SwMailBodyDialog::~SwMailBodyDialog()
{
    disposeOnce();
}

void SwMailBodyDialog::dispose()
{
    m_pBodyFT.clear();
    m_pBodyMLE.clear();
    m_pOK.clear();
    SfxModalDialog::dispose();
}

IMPL_LINK(SwMailBodyDialog, ContainsHdl_Impl, Button*, pButton, void)
{
    CheckBox* pBox = static_cast<CheckBox*>(pButton);
    SwGreetingsHandler::Contains(pBox->IsChecked());
    m_rConfigItem.SetGreetingLine(pBox->IsChecked(), true);
}

IMPL_LINK_NOARG(SwMailBodyDialog, OKHdl, Button*, void)
{
    m_rConfigItem.SetGreetingLine(
                m_pGreetingLineCB->IsChecked(), false);
    m_rConfigItem.SetIndividualGreeting(
                m_pPersonalizedCB->IsChecked(), false);

    if(m_pFemaleColumnLB->IsValueChangedFromSaved())
    {
        const SwDBData& rDBData = m_rConfigItem.GetCurrentDBData();
        Sequence< OUString> aAssignment = m_rConfigItem.GetColumnAssignment( rDBData );
        sal_Int32 nPos = m_pFemaleColumnLB->GetSelectedEntryPos();
        if(aAssignment.getLength() < MM_PART_GENDER)
            aAssignment.realloc(MM_PART_GENDER);
        if( nPos > 0 )
            aAssignment[MM_PART_GENDER] = m_pFemaleColumnLB->GetSelectedEntry();
        else
            aAssignment[MM_PART_GENDER].clear();
        m_rConfigItem.SetColumnAssignment( rDBData, aAssignment );
    }
    if(m_pFemaleFieldCB->IsValueChangedFromSaved())
        m_rConfigItem.SetFemaleGenderValue(m_pFemaleFieldCB->GetText());

    EndDialog(RET_OK);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
