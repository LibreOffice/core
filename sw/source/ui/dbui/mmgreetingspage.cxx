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
#include <dbui.hrc>
#include <com/sun/star/sdb/XColumn.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <comphelper/diagnose_ex.hxx>
#include <swmodule.hxx>
#include <view.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

static void lcl_FillGreetingsBox(weld::ComboBox& rBox,
                        SwMailMergeConfigItem const & rConfig,
                        SwMailMergeConfigItem::Gender eType)
{
    const Sequence< OUString> rEntries = rConfig.GetGreetings(eType);
    for(const auto& rEntry : rEntries)
        rBox.append_text(rEntry);
    rBox.set_active(rConfig.GetCurrentGreeting(eType));
}

static void lcl_StoreGreetingsBox(const weld::ComboBox& rBox,
                        SwMailMergeConfigItem& rConfig,
                        SwMailMergeConfigItem::Gender eType)
{
    Sequence< OUString> aEntries(rBox.get_count());
    OUString* pEntries = aEntries.getArray();
    for(sal_Int32 nEntry = 0; nEntry < rBox.get_count(); ++nEntry)
        pEntries[nEntry] = rBox.get_text(nEntry);
    rConfig.SetGreetings(eType, aEntries);
    rConfig.SetCurrentGreeting(eType, rBox.get_active());
}

IMPL_LINK_NOARG(SwGreetingsHandler, IndividualHdl_Impl, weld::Toggleable&, void)
{
    bool bIndividual = m_xPersonalizedCB->get_sensitive() && m_xPersonalizedCB->get_active();
    m_xFemaleFT->set_sensitive(bIndividual);
    m_xFemaleLB->set_sensitive(bIndividual);
    m_xFemalePB->set_sensitive(bIndividual);
    m_xMaleFT->set_sensitive(bIndividual);
    m_xMaleLB->set_sensitive(bIndividual);
    m_xMalePB->set_sensitive(bIndividual);
    m_xFemaleFI->set_sensitive(bIndividual);
    m_xFemaleColumnFT->set_sensitive(bIndividual);
    m_xFemaleColumnLB->set_sensitive(bIndividual);
    m_xFemaleFieldFT->set_sensitive(bIndividual);
    m_xFemaleFieldCB->set_sensitive(bIndividual);

    if( m_bIsTabPage )
    {
        m_rConfigItem.SetIndividualGreeting(bIndividual, false);
        m_pWizard->UpdateRoadmap();
        m_pWizard->enableButtons(WizardButtonFlags::NEXT, m_pWizard->isStateEnabled(MM_LAYOUTPAGE));
    }
    UpdatePreview();
}

IMPL_LINK(SwGreetingsHandler, GreetingHdl_Impl, weld::Button&, rButton, void)
{
    std::unique_ptr<SwCustomizeAddressBlockDialog> xDlg(new SwCustomizeAddressBlockDialog(&rButton, m_rConfigItem,
                        &rButton == m_xMalePB.get() ?
                        SwCustomizeAddressBlockDialog::GREETING_MALE :
                        SwCustomizeAddressBlockDialog::GREETING_FEMALE ));
    if (RET_OK == xDlg->run())
    {
        weld::ComboBox* pToInsert = &rButton == m_xMalePB.get() ? m_xMaleLB.get() : m_xFemaleLB.get();
        pToInsert->append_text(xDlg->GetAddress());
        pToInsert->set_active(pToInsert->get_count() - 1);
        if(m_bIsTabPage)
        {
            m_pWizard->UpdateRoadmap();
            m_pWizard->enableButtons(WizardButtonFlags::NEXT, m_pWizard->isStateEnabled(MM_LAYOUTPAGE));
        }
        UpdatePreview();
    }
}

void SwGreetingsHandler::UpdatePreview()
{
    //the base class does nothing
}

IMPL_LINK_NOARG(SwMailMergeGreetingsPage, AssignHdl_Impl, weld::Button&, void)
{
    const OUString sPreview(m_xFemaleLB->get_active_text() + "\n" + m_xMaleLB->get_active_text());
    SwAssignFieldsDialog aDlg(m_pWizard->getDialog(), m_rConfigItem, sPreview, false);
    if (RET_OK == aDlg.run())
    {
        UpdatePreview();
        m_pWizard->UpdateRoadmap();
        m_pWizard->enableButtons(WizardButtonFlags::NEXT, m_pWizard->isStateEnabled(MM_LAYOUTPAGE));
    }
}

IMPL_LINK_NOARG(SwMailMergeGreetingsPage, GreetingSelectListBoxHdl_Impl, weld::ComboBox&, void)
{
    UpdatePreview();
}

IMPL_LINK_NOARG(SwMailMergeGreetingsPage, GreetingSelectComboBoxHdl_Impl, weld::ComboBox&, void)
{
    UpdatePreview();
}

void SwMailMergeGreetingsPage::UpdatePreview()
{
    //find out which type of greeting should be selected:
    bool bFemale = false;
    bool bNoValue = !m_xFemaleColumnLB->get_sensitive();
    if( !bNoValue )
    {
        const OUString sFemaleValue = m_xFemaleFieldCB->get_active_text();
        const OUString sFemaleColumn = m_xFemaleColumnLB->get_active_text();
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
                    TOOLS_WARN_EXCEPTION( "sw", "");
                }
            }
        }
    }

    OUString sPreview = bFemale ? m_xFemaleLB->get_active_text() :
        bNoValue ? m_xNeutralCB->get_active_text() : m_xMaleLB->get_active_text();

    sPreview = SwAddressPreview::FillData(sPreview, m_rConfigItem);
    m_xPreview->SetAddress(sPreview);
}

void SwGreetingsHandler::Contains(bool bContainsGreeting)
{
    m_xPersonalizedCB->set_sensitive(bContainsGreeting);
    bool bEnablePersonal = bContainsGreeting && m_xPersonalizedCB->get_active();
    m_xFemaleFT->set_sensitive(bEnablePersonal);
    m_xFemaleLB->set_sensitive(bEnablePersonal);
    m_xFemalePB->set_sensitive(bEnablePersonal);
    m_xMaleFT->set_sensitive(bEnablePersonal);
    m_xMaleLB->set_sensitive(bEnablePersonal);
    m_xMalePB->set_sensitive(bEnablePersonal);
    m_xFemaleFI->set_sensitive(bEnablePersonal);
    m_xFemaleColumnFT->set_sensitive(bEnablePersonal);
    m_xFemaleColumnLB->set_sensitive(bEnablePersonal);
    m_xFemaleFieldFT->set_sensitive(bEnablePersonal);
    m_xFemaleFieldCB->set_sensitive(bEnablePersonal);
    m_xNeutralFT->set_sensitive(bContainsGreeting);
    m_xNeutralCB->set_sensitive(bContainsGreeting);
}

SwMailMergeGreetingsPage::SwMailMergeGreetingsPage(weld::Container* pPage, SwMailMergeWizard* pWizard)
    : vcl::OWizardPage(pPage, pWizard, "modules/swriter/ui/mmsalutationpage.ui", "MMSalutationPage")
    , SwGreetingsHandler(pWizard->GetConfigItem(), *m_xBuilder)
    , m_xPreview(new SwAddressPreview(m_xBuilder->weld_scrolled_window("previewwin", true)))
    , m_xPreviewFI(m_xBuilder->weld_label("previewft"))
    , m_xAssignPB(m_xBuilder->weld_button("assign"))
    , m_xDocumentIndexFI(m_xBuilder->weld_label("documentindex"))
    , m_xPrevSetIB(m_xBuilder->weld_button("prev"))
    , m_xNextSetIB(m_xBuilder->weld_button("next"))
    , m_xPreviewWIN(new weld::CustomWeld(*m_xBuilder, "preview", *m_xPreview))
{
    m_pWizard = pWizard;

    Size aSize(m_xPreview->GetDrawingArea()->get_ref_device().LogicToPixel(Size(186, 21), MapMode(MapUnit::MapAppFont)));
    m_xPreviewWIN->set_size_request(aSize.Width(), aSize.Height());
    m_sDocument = m_xDocumentIndexFI->get_label();

    m_bIsTabPage = true;

    m_xGreetingLineCB->connect_toggled(LINK(this, SwMailMergeGreetingsPage, ContainsHdl_Impl));
    Link<weld::Toggleable&,void> aIndividualLink = LINK(this, SwGreetingsHandler, IndividualHdl_Impl);
    m_xPersonalizedCB->connect_toggled(aIndividualLink);
    Link<weld::Button&,void> aGreetingLink = LINK(this, SwGreetingsHandler, GreetingHdl_Impl);
    m_xFemalePB->connect_clicked(aGreetingLink);
    m_xMalePB->connect_clicked(aGreetingLink);
    m_xAssignPB->connect_clicked(LINK(this, SwMailMergeGreetingsPage, AssignHdl_Impl));
    Link<weld::ComboBox&,void> aLBoxLink2 = LINK(this, SwMailMergeGreetingsPage, GreetingSelectListBoxHdl_Impl);
    m_xFemaleLB->connect_changed(aLBoxLink2);
    m_xMaleLB->connect_changed(aLBoxLink2);
    m_xFemaleColumnLB->connect_changed(aLBoxLink2);
    m_xFemaleFieldCB->connect_changed(LINK(this, SwMailMergeGreetingsPage, GreetingSelectComboBoxHdl_Impl));
    m_xNeutralCB->connect_changed(LINK(this, SwMailMergeGreetingsPage, GreetingSelectComboBoxHdl_Impl));

    Link<weld::Button&,void> aDataLink = LINK(this, SwMailMergeGreetingsPage, InsertDataHdl_Impl);
    m_xPrevSetIB->connect_clicked(aDataLink);
    m_xNextSetIB->connect_clicked(aDataLink);

    m_xGreetingLineCB->set_active(m_rConfigItem.IsGreetingLine(false));
    m_xPersonalizedCB->set_active(m_rConfigItem.IsIndividualGreeting(false));
    ContainsHdl_Impl(*m_xGreetingLineCB);
    aIndividualLink.Call(*m_xPersonalizedCB);

    lcl_FillGreetingsBox(*m_xFemaleLB, m_rConfigItem, SwMailMergeConfigItem::FEMALE);
    lcl_FillGreetingsBox(*m_xMaleLB, m_rConfigItem, SwMailMergeConfigItem::MALE);
    lcl_FillGreetingsBox(*m_xNeutralCB, m_rConfigItem, SwMailMergeConfigItem::NEUTRAL);

    m_xDocumentIndexFI->set_label(m_sDocument.replaceFirst("%1", "1"));
}

SwMailMergeGreetingsPage::~SwMailMergeGreetingsPage()
{
    m_xPreviewWIN.reset();
    m_xPreview.reset();
}

void SwMailMergeGreetingsPage::Activate()
{
    //try to find the gender setting
    m_xFemaleColumnLB->clear();
    Reference< sdbcx::XColumnsSupplier > xColsSupp = m_rConfigItem.GetColumnsSupplier();
    if(xColsSupp.is())
    {
        Reference < container::XNameAccess> xColAccess = xColsSupp->getColumns();
        const Sequence< OUString > aColumns = xColAccess->getElementNames();
        for(const auto& rColumn : aColumns)
            m_xFemaleColumnLB->append_text(rColumn);
    }

    m_xFemaleColumnLB->set_active_text(m_rConfigItem.GetAssignedColumn(MM_PART_GENDER));
    m_xFemaleColumnLB->save_value();

    m_xFemaleFieldCB->set_entry_text(m_rConfigItem.GetFemaleGenderValue());
    m_xFemaleFieldCB->save_value();

    UpdatePreview();
    m_pWizard->enableButtons(WizardButtonFlags::NEXT, m_pWizard->isStateEnabled(MM_LAYOUTPAGE));
}

bool SwMailMergeGreetingsPage::commitPage( ::vcl::WizardTypes::CommitPageReason )
{
    if (m_xFemaleColumnLB->get_value_changed_from_saved())
    {
        const SwDBData& rDBData = m_rConfigItem.GetCurrentDBData();
        Sequence< OUString> aAssignment = m_rConfigItem.GetColumnAssignment( rDBData );
        if(aAssignment.getLength() <= MM_PART_GENDER)
            aAssignment.realloc(MM_PART_GENDER + 1);
        aAssignment.getArray()[MM_PART_GENDER] = m_xFemaleColumnLB->get_active_text();
        m_rConfigItem.SetColumnAssignment( rDBData, aAssignment );
    }
    if (m_xFemaleFieldCB->get_value_changed_from_saved())
        m_rConfigItem.SetFemaleGenderValue(m_xFemaleFieldCB->get_active_text());

    lcl_StoreGreetingsBox(*m_xFemaleLB, m_rConfigItem, SwMailMergeConfigItem::FEMALE);
    lcl_StoreGreetingsBox(*m_xMaleLB, m_rConfigItem, SwMailMergeConfigItem::MALE);

    sal_Int32 nCurrentTextPos = m_xNeutralCB->find_text(m_xNeutralCB->get_active_text());
    if (nCurrentTextPos == -1)
    {
        m_xNeutralCB->append_text(m_xNeutralCB->get_active_text());
        m_xNeutralCB->set_active(m_xNeutralCB->get_count() - 1);
    }
    lcl_StoreGreetingsBox(*m_xNeutralCB, m_rConfigItem, SwMailMergeConfigItem::NEUTRAL);
    m_rConfigItem.SetGreetingLine(m_xGreetingLineCB->get_active(), false);
    m_rConfigItem.SetIndividualGreeting(m_xPersonalizedCB->get_active(), false);
    return true;
}

IMPL_LINK(SwMailMergeGreetingsPage, ContainsHdl_Impl, weld::Toggleable&, rBox, void)
{
    bool bContainsGreeting = rBox.get_active();
    SwGreetingsHandler::Contains(bContainsGreeting);
    m_xPreviewFI->set_sensitive(bContainsGreeting);
    m_xPreviewWIN->set_sensitive(bContainsGreeting);
    m_xAssignPB->set_sensitive(bContainsGreeting);
    m_xDocumentIndexFI->set_sensitive(bContainsGreeting);
    m_xPrevSetIB->set_sensitive(bContainsGreeting);
    m_xNextSetIB->set_sensitive(bContainsGreeting);
    m_rConfigItem.SetGreetingLine(m_xGreetingLineCB->get_active(), false);
    m_pWizard->UpdateRoadmap();
    m_pWizard->enableButtons(WizardButtonFlags::NEXT, m_pWizard->isStateEnabled(MM_LAYOUTPAGE));
}

IMPL_LINK(SwMailMergeGreetingsPage, InsertDataHdl_Impl, weld::Button&, rButton, void)
{
    bool bNext = &rButton == m_xNextSetIB.get();
    sal_Int32 nPos = m_rConfigItem.GetResultSetPosition();
    m_rConfigItem.MoveResultSet( bNext ? ++nPos : --nPos);
    nPos = m_rConfigItem.GetResultSetPosition();
    bool bEnable = true;
    if(nPos < 1)
    {
        bEnable = false;
        nPos = 1;
    }
    else
        UpdatePreview();
    m_xPrevSetIB->set_sensitive(bEnable);
    m_xNextSetIB->set_sensitive(bEnable);
    m_xDocumentIndexFI->set_sensitive(bEnable);
    m_xDocumentIndexFI->set_label(m_sDocument.replaceFirst("%1", OUString::number(nPos)));
}

SwMailBodyDialog::SwMailBodyDialog(weld::Window* pParent)
    : SfxDialogController(pParent, "modules/swriter/ui/mmmailbody.ui", "MailBodyDialog")
    , SwGreetingsHandler(*GetActiveView()->GetMailMergeConfigItem(), *m_xBuilder)
    , m_xBodyMLE(m_xBuilder->weld_text_view("bodymle"))
    , m_xOK(m_xBuilder->weld_button("ok"))
{
    m_bIsTabPage = false;
    m_xBodyMLE->set_size_request(m_xBodyMLE->get_approximate_digit_width() * 45,
                                 m_xBodyMLE->get_height_rows(6));
    m_xGreetingLineCB->connect_toggled(LINK(this, SwMailBodyDialog, ContainsHdl_Impl));
    Link<weld::Toggleable&,void> aIndividualLink = LINK(this, SwGreetingsHandler, IndividualHdl_Impl);
    m_xPersonalizedCB->connect_toggled(aIndividualLink);
    Link<weld::Button&,void> aGreetingLink = LINK(this, SwGreetingsHandler, GreetingHdl_Impl);
    m_xFemalePB->connect_clicked(aGreetingLink);
    m_xMalePB->connect_clicked(aGreetingLink);
    m_xOK->connect_clicked(LINK(this, SwMailBodyDialog, OKHdl));

    m_xGreetingLineCB->set_active(m_rConfigItem.IsGreetingLine(true));
    m_xPersonalizedCB->set_active(m_rConfigItem.IsIndividualGreeting(true));
    ContainsHdl_Impl(*m_xGreetingLineCB);
    aIndividualLink.Call(*m_xPersonalizedCB);

    lcl_FillGreetingsBox(*m_xFemaleLB, m_rConfigItem, SwMailMergeConfigItem::FEMALE);
    lcl_FillGreetingsBox(*m_xMaleLB, m_rConfigItem, SwMailMergeConfigItem::MALE);
    lcl_FillGreetingsBox(*m_xNeutralCB, m_rConfigItem, SwMailMergeConfigItem::NEUTRAL);

    //try to find the gender setting
    m_xFemaleColumnLB->clear();
    Reference< sdbcx::XColumnsSupplier > xColsSupp = m_rConfigItem.GetColumnsSupplier();
    if(xColsSupp.is())
    {
        Reference < container::XNameAccess> xColAccess = xColsSupp->getColumns();
        const Sequence< OUString > aColumns = xColAccess->getElementNames();
        for(const auto& rColumn : aColumns)
            m_xFemaleColumnLB->append_text(rColumn);
    }

    m_xFemaleColumnLB->set_active_text(m_rConfigItem.GetAssignedColumn(MM_PART_GENDER));
    m_xFemaleColumnLB->save_value();

    m_xFemaleFieldCB->set_entry_text(m_rConfigItem.GetFemaleGenderValue());
    m_xFemaleFieldCB->save_value();
}

SwMailBodyDialog::~SwMailBodyDialog()
{
}

IMPL_LINK(SwMailBodyDialog, ContainsHdl_Impl, weld::Toggleable&, rBox, void)
{
    SwGreetingsHandler::Contains(rBox.get_active());
    m_rConfigItem.SetGreetingLine(rBox.get_active(), true);
}

IMPL_LINK_NOARG(SwMailBodyDialog, OKHdl, weld::Button&, void)
{
    m_rConfigItem.SetGreetingLine(
                m_xGreetingLineCB->get_active(), false);
    m_rConfigItem.SetIndividualGreeting(
                m_xPersonalizedCB->get_active(), false);

    if (m_xFemaleColumnLB->get_value_changed_from_saved())
    {
        const SwDBData& rDBData = m_rConfigItem.GetCurrentDBData();
        Sequence< OUString> aAssignment = m_rConfigItem.GetColumnAssignment( rDBData );
        sal_Int32 nPos = m_xFemaleColumnLB->get_active();
        if(aAssignment.getLength() < MM_PART_GENDER)
            aAssignment.realloc(MM_PART_GENDER);
        if( nPos > 0 )
            aAssignment.getArray()[MM_PART_GENDER] = m_xFemaleColumnLB->get_active_text();
        else
            aAssignment.getArray()[MM_PART_GENDER].clear();
        m_rConfigItem.SetColumnAssignment( rDBData, aAssignment );
    }
    if (m_xFemaleFieldCB->get_value_changed_from_saved())
        m_rConfigItem.SetFemaleGenderValue(m_xFemaleFieldCB->get_active_text());

    m_xDialog->response(RET_OK);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
