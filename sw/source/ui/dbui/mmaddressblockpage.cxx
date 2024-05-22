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

#include "mmaddressblockpage.hxx"
#include <mailmergewizard.hxx>
#include <swtypes.hxx>
#include "addresslistdialog.hxx"
#include <editeng/eeitem.hxx>
#include <o3tl/safeint.hxx>
#include <svl/grabbagitem.hxx>
#include <svl/itemset.hxx>
#include <utility>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <vcl/transfer.hxx>
#include <mmconfigitem.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdb/XColumn.hpp>
#include <comphelper/sequence.hxx>
#include <comphelper/string.hxx>
#include <comphelper/diagnose_ex.hxx>

#include <vector>
#include <strings.hrc>
#include <mmaddressblockpage.hrc>
#include <helpids.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;

SwMailMergeAddressBlockPage::SwMailMergeAddressBlockPage(weld::Container* pPage, SwMailMergeWizard* pWizard)
    : vcl::OWizardPage(pPage, pWizard, u"modules/swriter/ui/mmaddressblockpage.ui"_ustr, u"MMAddressBlockPage"_ustr)
    , m_pWizard(pWizard)
    , m_xAddressListPB(m_xBuilder->weld_button(u"addresslist"_ustr))
    , m_xCurrentAddressFI(m_xBuilder->weld_label(u"currentaddress"_ustr))
    , m_xStep2(m_xBuilder->weld_container(u"step2"_ustr))
    , m_xStep3(m_xBuilder->weld_container(u"step3"_ustr))
    , m_xStep4(m_xBuilder->weld_container(u"step4"_ustr))
    , m_xSettingsFI(m_xBuilder->weld_label(u"settingsft"_ustr))
    , m_xAddressCB(m_xBuilder->weld_check_button(u"address"_ustr))
    , m_xSettingsPB(m_xBuilder->weld_button(u"settings"_ustr))
    , m_xHideEmptyParagraphsCB(m_xBuilder->weld_check_button(u"hideempty"_ustr))
    , m_xAssignPB(m_xBuilder->weld_button(u"assign"_ustr))
    , m_xDocumentIndexFI(m_xBuilder->weld_label(u"documentindex"_ustr))
    , m_xPrevSetIB(m_xBuilder->weld_button(u"prev"_ustr))
    , m_xNextSetIB(m_xBuilder->weld_button(u"next"_ustr))
    , m_xDifferentlist(m_xBuilder->weld_button(u"differentlist"_ustr))
    , m_xSettings(new SwAddressPreview(m_xBuilder->weld_scrolled_window(u"settingspreviewwin"_ustr, true)))
    , m_xPreview(new SwAddressPreview(m_xBuilder->weld_scrolled_window(u"addresspreviewwin"_ustr, true)))
    , m_xSettingsWIN(new weld::CustomWeld(*m_xBuilder, u"settingspreview"_ustr, *m_xSettings))
    , m_xPreviewWIN(new weld::CustomWeld(*m_xBuilder, u"addresspreview"_ustr, *m_xPreview))
{
    m_xSettingsWIN->set_size_request(m_xDifferentlist->get_approximate_digit_width() * 40,
                                     m_xDifferentlist->get_text_height() * 6);
    m_xPreviewWIN->set_size_request(m_xDifferentlist->get_approximate_digit_width() * 44,
                                    m_xDifferentlist->get_text_height() * 6);
    m_sChangeAddress = m_xDifferentlist->get_label();
    m_sDocument = m_xDocumentIndexFI->get_label();

    m_sCurrentAddress = m_xCurrentAddressFI->get_label();
    m_xAddressListPB->connect_clicked(LINK(this, SwMailMergeAddressBlockPage, AddressListHdl_Impl));
    m_xSettingsPB->connect_clicked(LINK(this, SwMailMergeAddressBlockPage, SettingsHdl_Impl));
    m_xAssignPB->connect_clicked(LINK(this, SwMailMergeAddressBlockPage, AssignHdl_Impl ));
    m_xAddressCB->connect_toggled(LINK(this, SwMailMergeAddressBlockPage, AddressBlockHdl_Impl));
    m_xSettings->SetSelectHdl(LINK(this, SwMailMergeAddressBlockPage, AddressBlockSelectHdl_Impl));
    m_xHideEmptyParagraphsCB->connect_toggled(LINK(this, SwMailMergeAddressBlockPage, HideParagraphsHdl_Impl));

    Link<weld::Button&,void> aLink = LINK(this, SwMailMergeAddressBlockPage, InsertDataHdl_Impl);
    m_xPrevSetIB->connect_clicked(aLink);
    m_xNextSetIB->connect_clicked(aLink);

    // lock in preferred size including current address line
    Size aSize1(m_xContainer->get_preferred_size());

    OUString sOrigLabel = m_xAddressListPB->get_label();
    m_xAddressListPB->set_label(m_sChangeAddress);
    Size aSize2(m_xContainer->get_preferred_size());
    m_xAddressListPB->set_label(sOrigLabel);

    m_xCurrentAddressFI->hide();

    m_xContainer->set_size_request(std::max(aSize1.Width(), aSize2.Width()),
                                   std::max(aSize1.Height(), aSize2.Height()));
}

SwMailMergeAddressBlockPage::~SwMailMergeAddressBlockPage()
{
    m_xPreviewWIN.reset();
    m_xSettingsWIN.reset();
    m_xPreview.reset();
    m_xSettings.reset();
}

bool SwMailMergeAddressBlockPage::canAdvance() const
{
    return m_pWizard->GetConfigItem().GetResultSet().is();
}

void SwMailMergeAddressBlockPage::Activate()
{
    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
    bool bIsLetter = rConfigItem.IsOutputToLetter();

    //no address block is created for e-Mail
    m_xStep2->set_visible(bIsLetter);
    m_xStep3->set_visible(bIsLetter);
    m_xStep4->set_visible(bIsLetter);

    if (!bIsLetter)
        return;

    m_xHideEmptyParagraphsCB->set_active( rConfigItem.IsHideEmptyParagraphs() );
    m_xDocumentIndexFI->set_label(m_sDocument.replaceFirst("%1", "1"));

    m_xSettings->Clear();
    const uno::Sequence< OUString> aBlocks =
                m_pWizard->GetConfigItem().GetAddressBlocks();
    for(const auto& rAddress : aBlocks)
        m_xSettings->AddAddress(rAddress);
    m_xSettings->SelectAddress(o3tl::narrowing<sal_uInt16>(rConfigItem.GetCurrentAddressBlockIndex()));
    m_xAddressCB->set_active(rConfigItem.IsAddressBlock());
    AddressBlockHdl_Impl(*m_xAddressCB);
    m_xSettings->SetLayout(1, 2);
    InsertDataHdl(nullptr);
}

bool SwMailMergeAddressBlockPage::commitPage( ::vcl::WizardTypes::CommitPageReason _eReason )
{
    return ::vcl::WizardTypes::eTravelForward != _eReason || m_pWizard->GetConfigItem().GetResultSet().is();
}

IMPL_LINK_NOARG(SwMailMergeAddressBlockPage, AddressListHdl_Impl, weld::Button&, void)
{
    try
    {
        SwAddressListDialog aAddrDialog(this);
        if (RET_OK == aAddrDialog.run())
        {
            SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
            rConfigItem.SetCurrentConnection(
                            aAddrDialog.GetSource(),
                            aAddrDialog.GetConnection(),
                            aAddrDialog.GetColumnsSupplier(),
                            aAddrDialog.GetDBData());
            OUString sFilter = aAddrDialog.GetFilter();
            rConfigItem.SetFilter( sFilter );
            InsertDataHdl(nullptr);
            GetWizard()->UpdateRoadmap();
            GetWizard()->enableButtons(WizardButtonFlags::NEXT, GetWizard()->isStateEnabled(MM_GREETINGSPAGE));
        }
    }
    catch (const uno::Exception& e)
    {
        TOOLS_WARN_EXCEPTION("sw", "");
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_pWizard->getDialog(),
                                                  VclMessageType::Warning, VclButtonsType::Ok, e.Message));
        xBox->run();
    }
}

IMPL_LINK_NOARG(SwMailMergeAddressBlockPage, SettingsHdl_Impl, weld::Button&, void)
{
    SwSelectAddressBlockDialog aDlg(m_pWizard->getDialog(), m_pWizard->GetConfigItem());
    SwMailMergeConfigItem& rConfig = m_pWizard->GetConfigItem();
    aDlg.SetAddressBlocks(rConfig.GetAddressBlocks(), m_xSettings->GetSelectedAddress());
    aDlg.SetSettings(rConfig.IsIncludeCountry(), rConfig.GetExcludeCountry());
    if (aDlg.run() == RET_OK)
    {
        //the dialog provides the selected address at the first position!
        const uno::Sequence< OUString> aBlocks = aDlg.GetAddressBlocks();
        rConfig.SetAddressBlocks(aBlocks);
        m_xSettings->Clear();
        for(const auto& rAddress : aBlocks)
            m_xSettings->AddAddress(rAddress);
        m_xSettings->SelectAddress(0);
        m_xSettings->Invalidate();    // #i40408
        rConfig.SetCountrySettings(aDlg.IsIncludeCountry(), aDlg.GetCountry());
        InsertDataHdl(nullptr);
    }
    GetWizard()->UpdateRoadmap();
    GetWizard()->enableButtons(WizardButtonFlags::NEXT, GetWizard()->isStateEnabled(MM_GREETINGSPAGE));
}

IMPL_LINK_NOARG(SwMailMergeAddressBlockPage, AssignHdl_Impl, weld::Button&, void)
{
    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
    const sal_uInt16 nSel = m_xSettings->GetSelectedAddress();
    const uno::Sequence< OUString> aBlocks = rConfigItem.GetAddressBlocks();
    SwAssignFieldsDialog aDlg(m_pWizard->getDialog(), m_pWizard->GetConfigItem(), aBlocks[nSel], true);
    if(RET_OK == aDlg.run())
    {
        //preview update
        InsertDataHdl(nullptr);
        GetWizard()->UpdateRoadmap();
        GetWizard()->enableButtons(WizardButtonFlags::NEXT, GetWizard()->isStateEnabled(MM_GREETINGSPAGE));
    }
}

void SwMailMergeAddressBlockPage::EnableAddressBlock(bool bAll, bool bSelective)
{
    m_xSettingsFI->set_sensitive(bAll);
    m_xAddressCB->set_sensitive(bAll);
    bSelective &= bAll;
    m_xHideEmptyParagraphsCB->set_sensitive(bSelective);
    m_xSettingsWIN->set_sensitive(bSelective);
    m_xSettingsPB->set_sensitive(bSelective);
    m_xStep3->set_sensitive(bSelective);
    m_xStep4->set_sensitive(bSelective);
}

IMPL_LINK(SwMailMergeAddressBlockPage, AddressBlockHdl_Impl, weld::Toggleable&, rBox, void)
{
    EnableAddressBlock(rBox.get_sensitive(), rBox.get_active());
    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
    rConfigItem.SetAddressBlock(m_xAddressCB->get_active());
    m_pWizard->UpdateRoadmap();
    GetWizard()->enableButtons(WizardButtonFlags::NEXT, GetWizard()->isStateEnabled(MM_GREETINGSPAGE));
}

IMPL_LINK_NOARG(SwMailMergeAddressBlockPage, AddressBlockSelectHdl_Impl, LinkParamNone*, void)
{
    const sal_uInt16 nSel = m_xSettings->GetSelectedAddress();
    const uno::Sequence< OUString> aBlocks =
                m_pWizard->GetConfigItem().GetAddressBlocks();
    m_xPreview->SetAddress(SwAddressPreview::FillData(aBlocks[nSel],
                                                         m_pWizard->GetConfigItem()));
    m_pWizard->GetConfigItem().SetCurrentAddressBlockIndex( nSel );
    GetWizard()->UpdateRoadmap();
    GetWizard()->enableButtons(WizardButtonFlags::NEXT, GetWizard()->isStateEnabled(MM_GREETINGSPAGE));
}

IMPL_LINK(SwMailMergeAddressBlockPage, HideParagraphsHdl_Impl, weld::Toggleable&, rBox, void)
{
    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
    rConfigItem.SetHideEmptyParagraphs(rBox.get_active());
}

void SwMailMergeAddressBlockPage::InsertDataHdl(const weld::Button* pButton)
{
    //if no pButton is given, the first set has to be pre-set
    SwMailMergeConfigItem& rConfig = m_pWizard->GetConfigItem();
    std::unique_ptr<weld::WaitObject> xWaitObj(new weld::WaitObject(m_pWizard->getDialog()));
    if(!pButton)
    {
        rConfig.GetResultSet();
    }
    else
    {
        bool bNext = pButton == m_xNextSetIB.get();
        sal_Int32 nPos = rConfig.GetResultSetPosition();
        rConfig.MoveResultSet( bNext ? ++nPos : --nPos);
    }
    xWaitObj.reset();
    sal_Int32 nPos = rConfig.GetResultSetPosition();
    bool bEnable = true;
    if(nPos < 1)
    {
        bEnable = false;
        nPos = 1;
    }
    else
    {
        //if output type is letter
        if (m_xSettings->IsVisible())
        {
            //Fill data into preview
            const sal_uInt16 nSel = m_xSettings->GetSelectedAddress();
            const uno::Sequence< OUString> aBlocks =
                        m_pWizard->GetConfigItem().GetAddressBlocks();
            m_xPreview->SetAddress(SwAddressPreview::FillData(aBlocks[nSel], rConfig));
        }
    }
    m_xPrevSetIB->set_sensitive(bEnable);
    m_xDocumentIndexFI->set_label(m_sDocument.replaceFirst("%1", OUString::number(nPos)));

    GetWizard()->enableButtons(WizardButtonFlags::NEXT, GetWizard()->isStateEnabled(MM_GREETINGSPAGE));
    bool bHasResultSet = rConfig.GetResultSet().is();
    m_xCurrentAddressFI->set_visible(bHasResultSet);
    if(bHasResultSet)
    {
        m_xCurrentAddressFI->set_label(m_sCurrentAddress.replaceFirst("%1", rConfig.GetCurrentDBData().sDataSource));
        m_xAddressListPB->set_label(m_sChangeAddress);
    }
    EnableAddressBlock(bHasResultSet, m_xAddressCB->get_active());
}

IMPL_LINK(SwMailMergeAddressBlockPage, InsertDataHdl_Impl, weld::Button&, rButton, void)
{
    InsertDataHdl(&rButton);
}

SwSelectAddressBlockDialog::SwSelectAddressBlockDialog(weld::Window* pParent, SwMailMergeConfigItem& rConfig)
    : SfxDialogController(pParent, u"modules/swriter/ui/selectblockdialog.ui"_ustr, u"SelectBlockDialog"_ustr)
    , m_rConfig(rConfig)
    , m_xPreview(new SwAddressPreview(m_xBuilder->weld_scrolled_window(u"previewwin"_ustr, true)))
    , m_xNewPB(m_xBuilder->weld_button(u"new"_ustr))
    , m_xCustomizePB(m_xBuilder->weld_button(u"edit"_ustr))
    , m_xDeletePB(m_xBuilder->weld_button(u"delete"_ustr))
    , m_xNeverRB(m_xBuilder->weld_radio_button(u"never"_ustr))
    , m_xAlwaysRB(m_xBuilder->weld_radio_button(u"always"_ustr))
    , m_xDependentRB(m_xBuilder->weld_radio_button(u"dependent"_ustr))
    , m_xCountryED(m_xBuilder->weld_entry(u"country"_ustr))
    , m_xPreviewWin(new weld::CustomWeld(*m_xBuilder, u"preview"_ustr, *m_xPreview))
{
    m_xPreviewWin->set_size_request(m_xCountryED->get_approximate_digit_width() * 45,
                                    m_xCountryED->get_text_height() * 12);

    Link<weld::Button&,void> aCustomizeHdl = LINK(this, SwSelectAddressBlockDialog, NewCustomizeHdl_Impl);
    m_xNewPB->connect_clicked(aCustomizeHdl);
    m_xCustomizePB->connect_clicked(aCustomizeHdl);

    m_xDeletePB->connect_clicked(LINK(this, SwSelectAddressBlockDialog, DeleteHdl_Impl));

    Link<weld::Toggleable&,void> aLk = LINK(this, SwSelectAddressBlockDialog, IncludeHdl_Impl);
    m_xNeverRB->connect_toggled(aLk);
    m_xAlwaysRB->connect_toggled(aLk);
    m_xDependentRB->connect_toggled(aLk);
    m_xPreview->SetLayout(2, 2);
    m_xPreview->EnableScrollBar();
}

SwSelectAddressBlockDialog::~SwSelectAddressBlockDialog()
{
}

void SwSelectAddressBlockDialog::SetAddressBlocks(const uno::Sequence< OUString>& rBlocks,
        sal_uInt16 nSelectedAddress)
{
    m_aAddressBlocks = rBlocks;
    for (const auto& rAddressBlock : m_aAddressBlocks)
        m_xPreview->AddAddress(rAddressBlock);
    m_xPreview->SelectAddress(nSelectedAddress);
}

// return the address blocks and put the selected one to the first position
const uno::Sequence< OUString >&    SwSelectAddressBlockDialog::GetAddressBlocks()
{
    //put the selected block to the first position
    const sal_Int32 nSelect = static_cast<sal_Int32>(m_xPreview->GetSelectedAddress());
    if(nSelect)
    {
        uno::Sequence< OUString >aTemp(m_aAddressBlocks.getLength());
        auto it = aTemp.getArray();
        *it = m_aAddressBlocks[nSelect];
        it = std::copy_n(std::cbegin(m_aAddressBlocks), nSelect - 1, std::next(it));
        std::copy(std::next(std::cbegin(m_aAddressBlocks), nSelect + 1), std::cend(m_aAddressBlocks), it);
        m_aAddressBlocks = aTemp;
    }
    return m_aAddressBlocks;
}

void SwSelectAddressBlockDialog::SetSettings(
        bool bIsCountry, const OUString& rCountry)
{
    weld::RadioButton *pActive = m_xNeverRB.get();
    if(bIsCountry)
    {
        pActive = !rCountry.isEmpty() ? m_xDependentRB.get() : m_xAlwaysRB.get();
        m_xCountryED->set_text(rCountry);
    }
    pActive->set_active(true);
    IncludeHdl_Impl(*pActive);
    m_xDeletePB->set_sensitive(m_aAddressBlocks.getLength() > 1);
}

OUString SwSelectAddressBlockDialog::GetCountry() const
{
    if (m_xDependentRB->get_active())
        return m_xCountryED->get_text();
    return OUString();
}

IMPL_LINK(SwSelectAddressBlockDialog, DeleteHdl_Impl, weld::Button&, rButton, void)
{
    if (m_aAddressBlocks.getLength())
    {
        const sal_Int32 nSelected = static_cast<sal_Int32>(m_xPreview->GetSelectedAddress());
        comphelper::removeElementAt(m_aAddressBlocks, nSelected);
        if (m_aAddressBlocks.getLength() <= 1)
            rButton.set_sensitive(false);
        m_xPreview->RemoveSelectedAddress();
    }
}

IMPL_LINK(SwSelectAddressBlockDialog, NewCustomizeHdl_Impl, weld::Button&, rButton, void)
{
    bool bCustomize = &rButton == m_xCustomizePB.get();
    SwCustomizeAddressBlockDialog::DialogType nType = bCustomize ?
        SwCustomizeAddressBlockDialog::ADDRESSBLOCK_EDIT :
        SwCustomizeAddressBlockDialog::ADDRESSBLOCK_NEW;
    std::unique_ptr<SwCustomizeAddressBlockDialog> xDlg(new SwCustomizeAddressBlockDialog(&rButton,
            m_rConfig, nType));
    if(bCustomize)
    {
        xDlg->SetAddress(m_aAddressBlocks[m_xPreview->GetSelectedAddress()]);
    }
    if (RET_OK != xDlg->run())
        return;

    const OUString sNew = xDlg->GetAddress();
    if(bCustomize)
    {
        m_xPreview->ReplaceSelectedAddress(sNew);
        m_aAddressBlocks.getArray()[m_xPreview->GetSelectedAddress()] = sNew;
    }
    else
    {
        m_xPreview->AddAddress(sNew);
        m_aAddressBlocks.realloc(m_aAddressBlocks.getLength() + 1);
        const sal_Int32 nSelect = m_aAddressBlocks.getLength() - 1;
        m_aAddressBlocks.getArray()[nSelect] = sNew;
        m_xPreview->SelectAddress(o3tl::narrowing<sal_uInt16>(nSelect));
    }
    m_xDeletePB->set_sensitive(m_aAddressBlocks.getLength() > 1);
}

IMPL_LINK_NOARG(SwSelectAddressBlockDialog, IncludeHdl_Impl, weld::Toggleable&,  void)
{
    m_xCountryED->set_sensitive(m_xDependentRB->get_active());
}

#define USER_DATA_SALUTATION        -1
#define USER_DATA_PUNCTUATION       -2
#define USER_DATA_TEXT              -3
#define USER_DATA_NONE              -4

IMPL_LINK(SwCustomizeAddressBlockDialog, TextFilterHdl, OUString&, rTest, bool)
{
    rTest = m_aTextFilter.filter(rTest);
    return true;
}

SwCustomizeAddressBlockDialog::SwCustomizeAddressBlockDialog(
        weld::Widget* pParent, SwMailMergeConfigItem& rConfig, DialogType eType)
    : SfxDialogController(pParent, u"modules/swriter/ui/addressblockdialog.ui"_ustr,
                          u"AddressBlockDialog"_ustr)
    , m_aTextFilter(u"<>"_ustr)
    , m_rConfigItem(rConfig)
    , m_eType(eType)
    , m_aSelectionChangedIdle("SwCustomizeAddressBlockDialog m_aSelectionChangedIdle")
    , m_xAddressElementsFT(m_xBuilder->weld_label(u"addressesft"_ustr))
    , m_xAddressElementsLB(m_xBuilder->weld_tree_view(u"addresses"_ustr))
    , m_xInsertFieldIB(m_xBuilder->weld_button(u"toaddr"_ustr))
    , m_xRemoveFieldIB(m_xBuilder->weld_button(u"fromaddr"_ustr))
    , m_xDragFT(m_xBuilder->weld_label(u"addressdestft"_ustr))
    , m_xUpIB(m_xBuilder->weld_button(u"up"_ustr))
    , m_xLeftIB(m_xBuilder->weld_button(u"left"_ustr))
    , m_xRightIB(m_xBuilder->weld_button(u"right"_ustr))
    , m_xDownIB(m_xBuilder->weld_button(u"down"_ustr))
    , m_xFieldFT(m_xBuilder->weld_label(u"customft"_ustr))
    , m_xFieldCB(m_xBuilder->weld_combo_box(u"custom"_ustr))
    , m_xOK(m_xBuilder->weld_button(u"ok"_ustr))
    , m_xPreview(new SwAddressPreview(m_xBuilder->weld_scrolled_window(u"previewwin"_ustr, true)))
    , m_xPreviewWIN(new weld::CustomWeld(*m_xBuilder, u"addrpreview"_ustr, *m_xPreview))
    , m_xDragED(new AddressMultiLineEdit(this))
    , m_xDragWIN(new weld::CustomWeld(*m_xBuilder, u"addressdest"_ustr, *m_xDragED))
{
    m_aSelectionChangedIdle.SetInvokeHandler( LINK( this, SwCustomizeAddressBlockDialog, SelectionChangedIdleHdl ) );

    Size aSize(m_xDragED->GetDrawingArea()->get_size_request());
    m_xPreview->set_size_request(aSize.Width(), aSize.Height());

    m_xFieldCB->connect_entry_insert_text(LINK(this, SwCustomizeAddressBlockDialog, TextFilterHdl));
    m_xAddressElementsLB->set_size_request(-1, m_xAddressElementsLB->get_height_rows(16));

    if( eType >= GREETING_FEMALE )
    {
        m_xFieldFT->show();
        m_xFieldCB->show();
        m_xAddressElementsLB->append(OUString::number(USER_DATA_SALUTATION), SwResId(ST_SALUTATION));
        m_xAddressElementsLB->append(OUString::number(USER_DATA_PUNCTUATION), SwResId(ST_PUNCTUATION));
        m_xAddressElementsLB->append(OUString::number(USER_DATA_TEXT), SwResId(ST_TEXT));
        for (auto const& aID : RA_SALUTATION)
            m_aSalutations.push_back(SwResId(aID));
        for (auto const& aID : RA_PUNCTUATION)
            m_aPunctuations.push_back(SwResId(aID));
        m_xDragED->SetText(u"            "_ustr);
        m_xDialog->set_title(SwResId(eType == GREETING_MALE ? ST_TITLE_MALE : ST_TITLE_FEMALE));
        m_xAddressElementsFT->set_label(SwResId(ST_SALUTATIONELEMENTS));
        m_xInsertFieldIB->set_tooltip_text(SwResId(ST_INSERTSALUTATIONFIELD));
        m_xRemoveFieldIB->set_tooltip_text(SwResId(ST_REMOVESALUTATIONFIELD));
        m_xDragFT->set_label(SwResId(ST_DRAGSALUTATION));
    }
    else
    {
        if (eType == ADDRESSBLOCK_EDIT)
            m_xDialog->set_title(SwResId(ST_TITLE_EDIT));
        m_xDragED->SetText(u"\n\n\n\n\n"_ustr);
        /* Set custom HIDs for swriter/01/mm_newaddblo.xhp */
        m_xAddressElementsLB->set_help_id( HID_MM_ADDBLOCK_ELEMENTS );
        m_xInsertFieldIB->set_help_id( HID_MM_ADDBLOCK_INSERT );
        m_xRemoveFieldIB->set_help_id( HID_MM_ADDBLOCK_REMOVE );
        m_xDragWIN->set_help_id( HID_MM_ADDBLOCK_DRAG );
        m_xPreviewWIN->set_help_id( HID_MM_ADDBLOCK_PREVIEW );
        m_xRightIB->set_help_id( HID_MM_ADDBLOCK_MOVEBUTTONS );
        m_xLeftIB->set_help_id( HID_MM_ADDBLOCK_MOVEBUTTONS );
        m_xDownIB->set_help_id( HID_MM_ADDBLOCK_MOVEBUTTONS );
        m_xUpIB->set_help_id( HID_MM_ADDBLOCK_MOVEBUTTONS );
    }

    const std::vector<std::pair<OUString, int>>& rHeaders = m_rConfigItem.GetDefaultAddressHeaders();
    for (size_t i = 0; i < rHeaders.size(); ++i)
        m_xAddressElementsLB->append(OUString::number(i), rHeaders[i].first);
    m_xOK->connect_clicked(LINK(this, SwCustomizeAddressBlockDialog, OKHdl_Impl));
    m_xAddressElementsLB->connect_changed(LINK(this, SwCustomizeAddressBlockDialog, ListBoxSelectHdl_Impl));
    if (m_xAddressElementsLB->n_children())
        m_xAddressElementsLB->select(0);
    m_xDragED->SetModifyHdl(LINK(this, SwCustomizeAddressBlockDialog, EditModifyHdl_Impl));
    m_xDragED->SetSelectionChangedHdl( LINK( this, SwCustomizeAddressBlockDialog, SelectionChangedHdl_Impl));
    m_xFieldCB->connect_changed(LINK(this, SwCustomizeAddressBlockDialog, FieldChangeComboBoxHdl_Impl));
    Link<weld::Button&,void> aImgButtonHdl = LINK(this, SwCustomizeAddressBlockDialog, ImageButtonHdl_Impl);
    m_xInsertFieldIB->connect_clicked(aImgButtonHdl);
    m_xRemoveFieldIB->connect_clicked(aImgButtonHdl);
    m_xUpIB->connect_clicked(aImgButtonHdl);
    m_xLeftIB->connect_clicked(aImgButtonHdl);
    m_xRightIB->connect_clicked(aImgButtonHdl);
    m_xDownIB->connect_clicked(aImgButtonHdl);
    UpdateImageButtons_Impl();
}

bool SwCustomizeAddressBlockDialog::SetCursorLogicPosition(const Point& rPosition)
{
    return m_xDragED->SetCursorLogicPosition(rPosition);
}

void SwCustomizeAddressBlockDialog::UpdateFields()
{
    m_xDragED->UpdateFields();
}

SwCustomizeAddressBlockDialog::~SwCustomizeAddressBlockDialog()
{
    m_xDragED->EndDropTarget();
}

IMPL_LINK_NOARG(SwCustomizeAddressBlockDialog, OKHdl_Impl, weld::Button&, void)
{
    m_xDialog->response(RET_OK);
}

IMPL_LINK(SwCustomizeAddressBlockDialog, ListBoxSelectHdl_Impl, weld::TreeView&, rBox, void)
{
    sal_Int32 nUserData = rBox.get_selected_id().toInt32();
    // Check if the selected entry is already in the address and then forbid inserting
    m_xInsertFieldIB->set_sensitive(nUserData >= 0 || !HasItem(nUserData));
}

IMPL_LINK_NOARG(SwCustomizeAddressBlockDialog, EditModifyHdl_Impl, AddressMultiLineEdit&, void)
{
    m_xPreview->SetAddress(SwAddressPreview::FillData(GetAddress(), m_rConfigItem));
    UpdateImageButtons_Impl();
}

IMPL_LINK(SwCustomizeAddressBlockDialog, ImageButtonHdl_Impl, weld::Button&, rButton, void)
{
    if (m_xInsertFieldIB.get() == &rButton)
    {
        int nEntry = m_xAddressElementsLB->get_selected_index();
        if (nEntry != -1)
        {
            m_xDragED->InsertNewEntry("<" + m_xAddressElementsLB->get_text(nEntry) + ">");
        }
    }
    else if (m_xRemoveFieldIB.get() == &rButton)
    {
        m_xDragED->RemoveCurrentEntry();
    }
    else
    {
        MoveItemFlags nMove = MoveItemFlags::Down;
        if (m_xUpIB.get() == &rButton)
            nMove = MoveItemFlags::Up;
        else if (m_xLeftIB.get() == &rButton)
            nMove = MoveItemFlags::Left;
        else if (m_xRightIB.get() == &rButton)
            nMove = MoveItemFlags::Right;
        m_xDragED->MoveCurrentItem(nMove);
    }
    UpdateImageButtons_Impl();
}

sal_Int32 SwCustomizeAddressBlockDialog::GetSelectedItem_Impl() const
{
    sal_Int32 nRet = USER_DATA_NONE;
    const OUString sSelected = m_xDragED->GetCurrentItem();
    if(!sSelected.isEmpty())
    {
        for (int i = 0, nEntryCount = m_xAddressElementsLB->n_children(); i < nEntryCount; ++i)
        {
            const OUString sEntry = m_xAddressElementsLB->get_text(i);
            if( sEntry == sSelected.subView( 1, sSelected.getLength() - 2 ) )
            {
                nRet = m_xAddressElementsLB->get_id(i).toInt32();
                break;
            }
        }
    }
    return nRet;
}

bool SwCustomizeAddressBlockDialog::HasItem(sal_Int32 nUserData)
{
    //get the entry from the ListBox
    OUString sEntry;
    for (int i = 0, nEntryCount = m_xAddressElementsLB->n_children(); i < nEntryCount; ++i)
    {
        if (m_xAddressElementsLB->get_id(i).toInt32() == nUserData)
        {
            sEntry = m_xAddressElementsLB->get_text(i);
            break;
        }
    }
    //search for this entry in the content
    return m_xDragED->GetText().indexOf(Concat2View("<" + sEntry + ">")) >= 0;
}

IMPL_LINK_NOARG(SwCustomizeAddressBlockDialog, SelectionChangedIdleHdl, Timer*, void)
{
    // called in case the selection of the edit field changes.
    // determine selection - if it's one of the editable fields then
    // enable the related ComboBox and fill it

    // don't trigger outself again
    m_xDragED->SetSelectionChangedHdl(Link<bool, void>());

    sal_Int32 nSelected = GetSelectedItem_Impl();
    if (USER_DATA_NONE != nSelected)
        m_xDragED->SelectCurrentItem();

    if(m_xFieldCB->get_visible() && (USER_DATA_NONE != nSelected) && (nSelected < 0))
    {
        //search in ListBox if it's one of the first entries
        OUString sSelect;
        std::vector<OUString>* pVector = nullptr;
        switch(nSelected) {
            case USER_DATA_SALUTATION:
                sSelect =  m_sCurrentSalutation;
                pVector = &m_aSalutations;
                break;
            case USER_DATA_PUNCTUATION:
                sSelect =  m_sCurrentPunctuation;
                pVector = &m_aPunctuations;
                break;
            case USER_DATA_TEXT:
                sSelect =  m_sCurrentText;
                break;
        }
        m_xFieldCB->clear();
        if(pVector) {
            for (const auto& rItem : *pVector)
                m_xFieldCB->append_text(rItem);
        }
        m_xFieldCB->set_entry_text(sSelect);
        m_xFieldCB->set_sensitive(true);
        m_xFieldFT->set_sensitive(true);
    }
    else
    {
        m_xFieldCB->set_sensitive(false);
        m_xFieldFT->set_sensitive(false);
    }

    UpdateImageButtons_Impl();
    m_xDragED->SetSelectionChangedHdl( LINK( this, SwCustomizeAddressBlockDialog, SelectionChangedHdl_Impl));
}

IMPL_LINK(SwCustomizeAddressBlockDialog, SelectionChangedHdl_Impl, bool, bIdle, void)
{
    if (bIdle)
        m_aSelectionChangedIdle.Start();
    else
    {
        m_aSelectionChangedIdle.Stop();
        SelectionChangedIdleHdl(nullptr);
    }
}

IMPL_LINK_NOARG(SwCustomizeAddressBlockDialog, FieldChangeComboBoxHdl_Impl, weld::ComboBox&, void)
{
    //changing the field content changes the related members, too
    sal_Int32 nSelected = GetSelectedItem_Impl();
    const OUString sContent = m_xFieldCB->get_active_text();
    switch(nSelected) {
        case USER_DATA_SALUTATION:
            m_sCurrentSalutation = sContent;
            break;
        case USER_DATA_PUNCTUATION:
            m_sCurrentPunctuation = sContent;
            break;
        case USER_DATA_TEXT:
            m_sCurrentText = sContent;
            break;
    }
    UpdateImageButtons_Impl();
    m_xPreview->SetAddress(GetAddress());
    EditModifyHdl_Impl(*m_xDragED);
}

void SwCustomizeAddressBlockDialog::UpdateImageButtons_Impl()
{
    MoveItemFlags nMove = m_xDragED->IsCurrentItemMoveable();
    m_xUpIB->set_sensitive( bool(nMove & MoveItemFlags::Up) );
    m_xLeftIB->set_sensitive( bool(nMove & MoveItemFlags::Left) );
    m_xRightIB->set_sensitive( bool(nMove & MoveItemFlags::Right) );
    m_xDownIB->set_sensitive( bool(nMove & MoveItemFlags::Down) );
    m_xRemoveFieldIB->set_sensitive(m_xDragED->HasCurrentItem());
    int nEntry = m_xAddressElementsLB->get_selected_index();
    m_xInsertFieldIB->set_sensitive( nEntry != -1 &&
            (m_xAddressElementsLB->get_id(nEntry).toInt32() >= 0 || !m_xFieldCB->get_active_text().isEmpty()));
}

void SwCustomizeAddressBlockDialog::SetAddress(const OUString& rAddress)
{
    m_xDragED->SetText(rAddress);
    UpdateImageButtons_Impl();
    EditModifyHdl_Impl(*m_xDragED);
}

OUString SwCustomizeAddressBlockDialog::GetAddress() const
{
    OUString sAddress(m_xDragED->GetAddress());
    //remove placeholders by the actual content
    if (m_xFieldFT->get_visible())
    {
        for (int i = 0, nEntryCount = m_xAddressElementsLB->n_children(); i < nEntryCount; ++i)
        {
            const OUString sEntry = "<" + m_xAddressElementsLB->get_text(i) + ">";
            sal_Int32 nUserData = m_xAddressElementsLB->get_id(i).toInt32();
            switch(nUserData)
            {
                case USER_DATA_SALUTATION:
                    sAddress = sAddress.replaceFirst(sEntry, m_sCurrentSalutation);
                    break;
                case USER_DATA_PUNCTUATION:
                    sAddress = sAddress.replaceFirst(sEntry, m_sCurrentPunctuation);
                    break;
                case USER_DATA_TEXT:
                    sAddress = sAddress.replaceFirst(sEntry, m_sCurrentText);
                    break;
            }
        }
    }
    return sAddress;
}

namespace {

struct SwAssignFragment
{
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Label> m_xLabel;
    std::unique_ptr<weld::ComboBox> m_xComboBox;
    std::unique_ptr<weld::Label> m_xPreview;

    SwAssignFragment(weld::Container* pGrid, int nLine)
        : m_xBuilder(Application::CreateBuilder(pGrid, u"modules/swriter/ui/assignfragment.ui"_ustr))
        , m_xLabel(m_xBuilder->weld_label(u"label"_ustr))
        , m_xComboBox(m_xBuilder->weld_combo_box(u"combobox"_ustr))
        , m_xPreview(m_xBuilder->weld_label(u"preview"_ustr))
    {
        m_xLabel->set_grid_left_attach(0);
        m_xLabel->set_grid_top_attach(nLine);

        m_xComboBox->set_grid_left_attach(1);
        m_xComboBox->set_grid_top_attach(nLine);

        m_xPreview->set_grid_left_attach(2);
        m_xPreview->set_grid_top_attach(nLine);
    }
};

}

class SwAssignFieldsControl
{
    friend class SwAssignFieldsDialog;
    std::unique_ptr<weld::ScrolledWindow> m_xVScroll;
    std::unique_ptr<weld::Container> m_xGrid;

    std::vector<SwAssignFragment> m_aFields;

    SwMailMergeConfigItem* m_rConfigItem;

    Link<LinkParamNone*,void>   m_aModifyHdl;

    DECL_LINK(MatchHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(GotFocusHdl_Impl, weld::Widget&, void);

    void MakeVisible(const tools::Rectangle & rRect);
public:
    SwAssignFieldsControl(std::unique_ptr<weld::ScrolledWindow> xWindow,
                          std::unique_ptr<weld::Container> xGrid);

    void Init(SwAssignFieldsDialog* pDialog, SwMailMergeConfigItem& rConfigItem);
    void SetModifyHdl(const Link<LinkParamNone*,void>& rModifyHdl)
    {
        m_aModifyHdl = rModifyHdl;
        m_aModifyHdl.Call(nullptr);
    }
};

SwAssignFieldsControl::SwAssignFieldsControl(std::unique_ptr<weld::ScrolledWindow> xWindow,
                                             std::unique_ptr<weld::Container> xGrid)
    : m_xVScroll(std::move(xWindow))
    , m_xGrid(std::move(xGrid))
    , m_rConfigItem(nullptr)
{
}

void SwAssignFieldsControl::Init(SwAssignFieldsDialog* pDialog, SwMailMergeConfigItem& rConfigItem)
{
    m_rConfigItem = &rConfigItem;

    //get the name of the default headers
    const std::vector<std::pair<OUString, int>>& rHeaders = rConfigItem.GetDefaultAddressHeaders();
    //get the actual data
    uno::Reference< XColumnsSupplier > xColsSupp( rConfigItem.GetResultSet(), uno::UNO_QUERY);
    //get the name of the actual columns
    uno::Reference <XNameAccess> xColAccess = xColsSupp.is() ? xColsSupp->getColumns() : nullptr;
    uno::Sequence< OUString > aFields;
    if(xColAccess.is())
        aFields = xColAccess->getElementNames();

    //get the current assignment list
    //each position in this sequence matches the position in the header array rHeaders
    //if no assignment is available an empty sequence will be returned
    uno::Sequence< OUString> aAssignments = rConfigItem.GetColumnAssignment( rConfigItem.GetCurrentDBData() );
    Link<weld::ComboBox&,void> aMatchHdl = LINK(this, SwAssignFieldsControl, MatchHdl_Impl);
    Link<weld::Widget&,void> aFocusHdl = LINK(this, SwAssignFieldsControl, GotFocusHdl_Impl);

    int nLabelWidth(0), nComboBoxWidth(0), nPreviewWidth(0);

    //fill the controls
    for (size_t i = 0; i < rHeaders.size(); ++i)
    {
        m_aFields.emplace_back(m_xGrid.get(), i);

        const OUString rHeader = rHeaders[i].first;
        weld::ComboBox& rNewLB = *m_aFields.back().m_xComboBox;
        rNewLB.append_text(SwResId(SW_STR_NONE));
        rNewLB.set_active(0);

        for (const OUString& rField : aFields)
            rNewLB.append_text(rField);
        //select the ListBox
        //if there is an assignment
        if(o3tl::make_unsigned(aAssignments.getLength()) > i && !aAssignments[i].isEmpty())
            rNewLB.set_active_text(aAssignments[i]);
        else //otherwise the current column name may match one of the db columns
            rNewLB.set_active_text(rHeader);

        weld::Label& rNewText = *m_aFields.back().m_xLabel;
        rNewText.set_label("<" + rHeader + ">");

        weld::Label& rNewPreview = *m_aFields.back().m_xPreview;
        //then the preview can be filled accordingly
        if (xColAccess.is() && rNewLB.get_active() > 0 &&
            xColAccess->hasByName(rNewLB.get_active_text()))
        {
            uno::Any aCol = xColAccess->getByName(rNewLB.get_active_text());
            uno::Reference< XColumn > xColumn;
            aCol >>= xColumn;
            if(xColumn.is())
            {
                try
                {
                    rNewPreview.set_label(xColumn->getString());
                }
                catch (const SQLException&)
                {
                }
            }
        }

        if (i == 0)
        {
            auto nLineHeight = m_xGrid->get_preferred_size().Height();
            m_xVScroll->set_size_request(m_xVScroll->get_approximate_digit_width() * 65,
                                         nLineHeight * 6);
            nComboBoxWidth = rNewLB.get_preferred_size().Width();
        }

        nLabelWidth = std::max<int>(nLabelWidth, rNewText.get_preferred_size().Width());
        nPreviewWidth = std::max<int>(nPreviewWidth, rNewPreview.get_preferred_size().Width());

        rNewLB.connect_changed(aMatchHdl);
        rNewLB.connect_focus_in(aFocusHdl);
        rNewText.show();
        rNewLB.show();
        rNewPreview.show();
    }
    pDialog->ConnectSizeGroups(nLabelWidth, nComboBoxWidth, nPreviewWidth);
}

void SwAssignFieldsControl::MakeVisible(const tools::Rectangle & rRect)
{
    //determine range of visible positions
    auto nMinVisiblePos = m_xVScroll->vadjustment_get_value();
    auto nMaxVisiblePos = nMinVisiblePos + m_xVScroll->vadjustment_get_page_size();
    if (rRect.Top() < nMinVisiblePos || rRect.Bottom() > nMaxVisiblePos)
        m_xVScroll->vadjustment_set_value(rRect.Top());
}

IMPL_LINK(SwAssignFieldsControl, MatchHdl_Impl, weld::ComboBox&, rBox, void)
{
    const OUString sColumn = rBox.get_active_text();
    uno::Reference< XColumnsSupplier > xColsSupp( m_rConfigItem->GetResultSet(), uno::UNO_QUERY);
    uno::Reference <XNameAccess> xColAccess = xColsSupp.is() ? xColsSupp->getColumns() : nullptr;
    OUString sPreview;
    if(xColAccess.is() && xColAccess->hasByName(sColumn))
    {
        uno::Any aCol = xColAccess->getByName(sColumn);
        uno::Reference< XColumn > xColumn;
        aCol >>= xColumn;
        if(xColumn.is())
        {
            try
            {
                sPreview = xColumn->getString();
            }
            catch (const sdbc::SQLException&)
            {
            }
        }
    }
    auto aLBIter = std::find_if(m_aFields.begin(), m_aFields.end(), [&rBox](const SwAssignFragment& rFragment){
                                                                     return &rBox == rFragment.m_xComboBox.get(); });
    if (aLBIter != m_aFields.end())
    {
        auto nIndex = static_cast<sal_Int32>(std::distance(m_aFields.begin(), aLBIter));
        m_aFields[nIndex].m_xPreview->set_label(sPreview);
    }
    m_aModifyHdl.Call(nullptr);
}

IMPL_LINK(SwAssignFieldsControl, GotFocusHdl_Impl, weld::Widget&, rBox, void)
{
    int x, y, width, height;
    rBox.get_extents_relative_to(*m_xGrid, x, y, width, height);
    // the container has a border of 3 in the .ui
    tools::Rectangle aRect(Point(x - 3, y - 3), Size(width + 6, height + 6));
    MakeVisible(aRect);
}

SwAssignFieldsDialog::SwAssignFieldsDialog(
        weld::Window* pParent, SwMailMergeConfigItem& rConfigItem,
        OUString aPreview,
        bool bIsAddressBlock)
    : SfxDialogController(pParent, u"modules/swriter/ui/assignfieldsdialog.ui"_ustr, u"AssignFieldsDialog"_ustr)
    , m_sNone(SwResId(SW_STR_NONE))
    , m_rPreviewString(std::move(aPreview))
    , m_rConfigItem(rConfigItem)
    , m_xPreview(new SwAddressPreview(m_xBuilder->weld_scrolled_window(u"previewwin"_ustr, true)))
    , m_xMatchingFI(m_xBuilder->weld_label(u"MATCHING_LABEL"_ustr))
    , m_xAddressTitle(m_xBuilder->weld_label(u"addresselem"_ustr))
    , m_xMatchTitle(m_xBuilder->weld_label(u"matchelem"_ustr))
    , m_xPreviewTitle(m_xBuilder->weld_label(u"previewelem"_ustr))
    , m_xPreviewFI(m_xBuilder->weld_label(u"PREVIEW_LABEL"_ustr))
    , m_xOK(m_xBuilder->weld_button(u"ok"_ustr))
    , m_xPreviewWin(new weld::CustomWeld(*m_xBuilder, u"PREVIEW"_ustr, *m_xPreview))
    , m_xFieldsControl(new SwAssignFieldsControl(m_xBuilder->weld_scrolled_window(u"scroll"_ustr),
                                                 m_xBuilder->weld_container(u"FIELDS"_ustr)))
{
    m_xPreviewWin->set_size_request(m_xMatchingFI->get_approximate_digit_width() * 45,
                                    m_xMatchingFI->get_text_height() * 5);
    m_xFieldsControl->Init(this, rConfigItem);

    const OUString sMatchesTo( SwResId(ST_MATCHESTO) );
    if (!bIsAddressBlock)
    {
        m_xPreviewFI->set_label(SwResId(ST_SALUTATIONPREVIEW));
        m_xMatchingFI->set_label(SwResId(ST_SALUTATIONMATCHING));
        m_xAddressTitle->set_label(SwResId(ST_SALUTATIONELEMENT));
    }

    m_xFieldsControl->SetModifyHdl(LINK(this, SwAssignFieldsDialog, AssignmentModifyHdl_Impl ));
    m_xMatchingFI->set_label(m_xMatchingFI->get_label().replaceAll("%1", sMatchesTo));
    m_xOK->connect_clicked(LINK(this, SwAssignFieldsDialog, OkHdl_Impl));
}

SwAssignFieldsDialog::~SwAssignFieldsDialog()
{
}

uno::Sequence< OUString > SwAssignFieldsDialog::CreateAssignments()
{
    uno::Sequence< OUString > aAssignments(
            m_rConfigItem.GetDefaultAddressHeaders().size());
    OUString* pAssignments = aAssignments.getArray();
    sal_Int32 nIndex = 0;
    for (const auto& rLBItem : m_xFieldsControl->m_aFields)
    {
        const OUString sSelect = rLBItem.m_xComboBox->get_active_text();
        pAssignments[nIndex] = (m_sNone != sSelect) ? sSelect : OUString();
        ++nIndex;
    }
    return aAssignments;
}

IMPL_LINK_NOARG(SwAssignFieldsDialog, OkHdl_Impl, weld::Button&, void)
{
    m_rConfigItem.SetColumnAssignment(
                            m_rConfigItem.GetCurrentDBData(),
                            CreateAssignments() );
    m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(SwAssignFieldsDialog, AssignmentModifyHdl_Impl, LinkParamNone*, void)
{
    uno::Sequence< OUString > aAssignments = CreateAssignments();
    const OUString sPreview = SwAddressPreview::FillData(
            m_rPreviewString, m_rConfigItem, &aAssignments);
    m_xPreview->SetAddress(sPreview);
}

void SwAssignFieldsDialog::ConnectSizeGroups(int nLabelWidth, int nComboBoxWidth, int nPreviewWidth)
{
    m_xAddressTitle->set_size_request(nLabelWidth, -1);
    m_xMatchTitle->set_size_request(nComboBoxWidth, -1);
    m_xPreviewTitle->set_size_request(nPreviewWidth, -1);
}

namespace
{
    const EECharAttrib* FindCharAttrib(int nStartPosition, std::vector<EECharAttrib>& rAttribList)
    {
        for (auto it = rAttribList.rbegin(); it != rAttribList.rend(); ++it)
        {
            const auto& rTextAtr = *it;
            if (rTextAtr.pAttr->Which() != EE_CHAR_GRABBAG)
                continue;
            if (rTextAtr.nStart <= nStartPosition && rTextAtr.nEnd >= nStartPosition)
            {
                return &rTextAtr;
            }
        }

        return nullptr;
    }
}

AddressMultiLineEdit::AddressMultiLineEdit(SwCustomizeAddressBlockDialog *pParent)
    : m_pParentDialog(pParent)
{
}

void AddressMultiLineEdit::EndDropTarget()
{
    if (m_xDropTarget.is())
    {
        m_xEditEngine->RemoveView(m_xEditView.get());
        auto xRealDropTarget = GetDrawingArea()->get_drop_target();
        uno::Reference<css::datatransfer::dnd::XDropTargetListener> xListener(m_xDropTarget, uno::UNO_QUERY);
        xRealDropTarget->removeDropTargetListener(xListener);
        m_xDropTarget.clear();
    }
}

AddressMultiLineEdit::~AddressMultiLineEdit()
{
    assert(!m_xDropTarget.is());
}

void AddressMultiLineEdit::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    Size aSize(pDrawingArea->get_ref_device().LogicToPixel(Size(160, 60), MapMode(MapUnit::MapAppFont)));
    pDrawingArea->set_size_request(aSize.Width(), aSize.Height());
    WeldEditView::SetDrawingArea(pDrawingArea);
}

bool AddressMultiLineEdit::KeyInput(const KeyEvent& rKEvt)
{
    if (rKEvt.GetKeyCode().GetCode() == KEY_ESCAPE)
        return false; // we want default esc behaviour
    if (rKEvt.GetCharCode())
        return true; // handled
    return WeldEditView::KeyInput(rKEvt);
}

bool AddressMultiLineEdit::Command(const CommandEvent& rCEvt)
{
    if (rCEvt.GetCommand() == CommandEventId::StartExtTextInput ||
        rCEvt.GetCommand() == CommandEventId::EndExtTextInput ||
        rCEvt.GetCommand() == CommandEventId::ExtTextInput)
    {
        return true;
    }
    return WeldEditView::Command(rCEvt);
}

bool AddressMultiLineEdit::MouseButtonDown(const MouseEvent& rMEvt)
{
    if (rMEvt.GetClicks() >= 2)
        return true; // handled
    return WeldEditView::MouseButtonDown(rMEvt);
}

OUString AddressMultiLineEdit::GetText() const
{
    return m_xEditEngine->GetText();
}

void AddressMultiLineEdit::SetText( const OUString& rStr )
{
    m_xEditEngine->SetText(rStr);
    //set attributes to all address tokens

    sal_Int32 nSequence(0);
    SfxGrabBagItem aProtectAttr(EE_CHAR_GRABBAG);
    const sal_uInt32 nParaCount = m_xEditEngine->GetParagraphCount();
    for(sal_uInt32 nPara = 0; nPara < nParaCount; ++nPara)
    {
        sal_Int32 nIndex = 0;
        const OUString sPara = m_xEditEngine->GetText( nPara );
        if (!sPara.isEmpty() && !sPara.endsWith(" "))
        {
            ESelection aPaM(nPara, sPara.getLength(), nPara, sPara.getLength());
            m_xEditEngine->QuickInsertText(u" "_ustr, aPaM);
        }
        for(;;)
        {
            const sal_Int32 nStart = sPara.indexOf( '<', nIndex );
            if (nStart < 0)
                break;
            const sal_Int32 nEnd = sPara.indexOf( '>', nStart );
            if (nEnd < 0)
                break;
            nIndex = nEnd;
            SfxItemSet aSet(m_xEditEngine->GetEmptyItemSet());
            // make each one different, so they are not collapsed together
            // as one attribute
            aProtectAttr.GetGrabBag()[u"Index"_ustr] <<= nSequence++;
            aSet.Put(aProtectAttr);
            m_xEditEngine->QuickSetAttribs(aSet, ESelection(nPara, nStart, nPara, nEnd + 1));
        }

    }
    // add two empty paragraphs at the end
    if(m_pParentDialog->m_eType == SwCustomizeAddressBlockDialog::ADDRESSBLOCK_NEW ||
            m_pParentDialog->m_eType == SwCustomizeAddressBlockDialog::ADDRESSBLOCK_EDIT)
    {
        sal_Int32 nLastLen = m_xEditEngine->GetText(nParaCount - 1).getLength();
        if(nLastLen)
        {
            int nPara = nParaCount ? nParaCount - 1 : 0;
            ESelection aPaM(nPara, nLastLen, nPara, nLastLen);
            m_xEditEngine->QuickInsertText(u"\n \n "_ustr, aPaM);
        }
    }

    m_xEditView->SetSelection(ESelection(0, 0, 0, 0));
}

// Insert the new entry in front of the entry at the beginning of the selection
void AddressMultiLineEdit::InsertNewEntry( const OUString& rStr )
{
    // insert new entry after current selected one.
    ESelection aSelection = m_xEditView->GetSelection();
    const sal_uInt32 nPara = aSelection.nStartPara;

    std::vector<EECharAttrib> aAttribList;
    m_xEditEngine->GetCharAttribs(nPara, aAttribList);

    sal_Int32 nIndex = aSelection.nEndPara;
    const EECharAttrib* pAttrib = FindCharAttrib(aSelection.nStartPos, aAttribList);
    if(nullptr != pAttrib)
        nIndex = pAttrib->nEnd;
    InsertNewEntryAtPosition( rStr, nPara, nIndex );

    // select the new entry
    m_xEditEngine->GetCharAttribs(nPara, aAttribList);
    pAttrib = FindCharAttrib(nIndex, aAttribList);
    const sal_Int32 nEnd = pAttrib ? pAttrib->nEnd : nIndex;
    ESelection aEntrySel(nPara, nIndex, nPara, nEnd);
    m_xEditView->SetSelection(aEntrySel);
    Invalidate();
    m_aModifyLink.Call(*this);
}

void AddressMultiLineEdit::InsertNewEntryAtPosition( const OUString& rStr, sal_uLong nPara, sal_uInt16 nIndex )
{
    ESelection aInsertPos(nPara, nIndex, nPara, nIndex);
    m_xEditEngine->QuickInsertText(rStr, aInsertPos);

    //restore the attributes
    SetText( GetAddress() );

    //select the newly inserted/moved element
    m_xEditView->SetSelection(aInsertPos);
    m_aSelectionLink.Call(false);
}

void AddressMultiLineEdit::RemoveCurrentEntry()
{
    ESelection aSelection = m_xEditView->GetSelection();

    std::vector<EECharAttrib> aAttribList;
    m_xEditEngine->GetCharAttribs(aSelection.nStartPara, aAttribList);

    const EECharAttrib* pBeginAttrib = FindCharAttrib(aSelection.nStartPos, aAttribList);
    if(pBeginAttrib &&
            (pBeginAttrib->nStart <= aSelection.nStartPos
                            && pBeginAttrib->nEnd >= aSelection.nEndPos))
    {
        const sal_uInt32 nPara = aSelection.nStartPara;
        ESelection aEntrySel(nPara, pBeginAttrib->nStart, nPara, pBeginAttrib->nEnd);
        m_xEditEngine->QuickInsertText(OUString(), aEntrySel);
        //restore the attributes
        SetText( GetAddress() );
        m_aModifyLink.Call(*this);
    }
}

void AddressMultiLineEdit::MoveCurrentItem(MoveItemFlags nMove)
{
    ESelection aSelection = m_xEditView->GetSelection();

    std::vector<EECharAttrib> aAttribList;
    m_xEditEngine->GetCharAttribs(aSelection.nStartPara, aAttribList);

    const EECharAttrib* pBeginAttrib = FindCharAttrib(aSelection.nStartPos, aAttribList);
    if(!pBeginAttrib ||
         pBeginAttrib->nStart > aSelection.nStartPos ||
         pBeginAttrib->nEnd < aSelection.nEndPos)
        return;

    //current item has been found
    sal_Int32 nPara = aSelection.nStartPara;
    sal_Int32 nIndex = pBeginAttrib->nStart;
    ESelection aEntrySel(nPara, pBeginAttrib->nStart, nPara, pBeginAttrib->nEnd);
    const OUString sCurrentItem = m_xEditEngine->GetText(aEntrySel);
    m_xEditEngine->RemoveAttribs(aEntrySel, false, EE_CHAR_GRABBAG);
    m_xEditEngine->QuickInsertText(OUString(), aEntrySel);
    m_xEditEngine->GetCharAttribs(nPara, aAttribList);
    switch (nMove)
    {
        case MoveItemFlags::Left :
            if(nIndex)
            {
                //go left to find a predecessor or simple text
                --nIndex;
                const OUString sPara = m_xEditEngine->GetText( nPara );
                sal_Int32 nSearchIndex = sPara.lastIndexOf( '>', nIndex+1 );
                if( nSearchIndex != -1 && nSearchIndex == nIndex )
                {
                    nSearchIndex = sPara.lastIndexOf( '<', nIndex );
                    if( nSearchIndex != -1 )
                        nIndex = nSearchIndex;
                }
            }
        break;
        case MoveItemFlags::Right:
        {
            //go right to find a successor or simple text
            ++nIndex;
            const EECharAttrib* pEndAttrib = FindCharAttrib(aSelection.nStartPos, aAttribList);
            if(pEndAttrib && pEndAttrib->nEnd >= nIndex)
            {
                nIndex = pEndAttrib->nEnd;
            }
        }
        break;
        case MoveItemFlags::Up   :
            --nPara;
            nIndex = 0;
        break;
        case MoveItemFlags::Down :
            ++nPara;
            nIndex = 0;
        break;
        default: break;
    }
    //add a new paragraph if there is none yet
    if (nPara >= m_xEditEngine->GetParagraphCount())
    {
        auto nInsPara = nPara - 1;
        auto nInsPos = m_xEditEngine->GetTextLen( nPara - 1 );
        ESelection aTemp(nInsPara, nInsPos, nInsPara, nInsPos);
        m_xEditEngine->QuickInsertText(u"\n"_ustr, aTemp);
    }
    InsertNewEntryAtPosition( sCurrentItem, nPara, nIndex );

    // select the new entry [#i40817]
    m_xEditEngine->GetCharAttribs(nPara, aAttribList);
    const EECharAttrib* pAttrib = FindCharAttrib(nIndex, aAttribList);
    if (pAttrib)
        aEntrySel = ESelection(nPara, nIndex, nPara, pAttrib->nEnd);
    m_xEditView->SetSelection(aEntrySel);
    Invalidate();
    m_aModifyLink.Call(*this);
}

MoveItemFlags AddressMultiLineEdit::IsCurrentItemMoveable() const
{
    MoveItemFlags nRet = MoveItemFlags::NONE;
    ESelection aSelection = m_xEditView->GetSelection();

    std::vector<EECharAttrib> aAttribList;
    m_xEditEngine->GetCharAttribs(aSelection.nStartPara, aAttribList);

    const EECharAttrib* pBeginAttrib = FindCharAttrib(aSelection.nStartPos, aAttribList);
    if (pBeginAttrib &&
            (pBeginAttrib->nStart <= aSelection.nStartPos
                            && pBeginAttrib->nEnd >= aSelection.nEndPos))
    {
        if (pBeginAttrib->nStart)
            nRet |= MoveItemFlags::Left;
        //if there is an entry it can always be move to the right and down
        nRet |= MoveItemFlags::Right | MoveItemFlags::Down;
        if (aSelection.nStartPara > 0)
            nRet |= MoveItemFlags::Up;
    }
    return nRet;
}

bool AddressMultiLineEdit::HasCurrentItem() const
{
    ESelection aSelection = m_xEditView->GetSelection();

    std::vector<EECharAttrib> aAttribList;
    m_xEditEngine->GetCharAttribs(aSelection.nStartPara, aAttribList);

    const EECharAttrib* pBeginAttrib = FindCharAttrib(aSelection.nStartPos, aAttribList);
    return (pBeginAttrib &&
            (pBeginAttrib->nStart <= aSelection.nStartPos
                            && pBeginAttrib->nEnd >= aSelection.nEndPos));
}

OUString AddressMultiLineEdit::GetCurrentItem() const
{
    ESelection aSelection = m_xEditView->GetSelection();

    std::vector<EECharAttrib> aAttribList;
    m_xEditEngine->GetCharAttribs(aSelection.nStartPara, aAttribList);

    const EECharAttrib* pBeginAttrib = FindCharAttrib(aSelection.nStartPos, aAttribList);
    if (pBeginAttrib &&
            (pBeginAttrib->nStart <= aSelection.nStartPos
                            && pBeginAttrib->nEnd >= aSelection.nEndPos))
    {
        const sal_uInt32 nPara = aSelection.nStartPara;
        ESelection aEntrySel(nPara, pBeginAttrib->nStart, nPara, pBeginAttrib->nEnd);
        return m_xEditEngine->GetText( aEntrySel );
    }
    return OUString();
}

void AddressMultiLineEdit::SelectCurrentItem()
{
    ESelection aSelection = m_xEditView->GetSelection();

    std::vector<EECharAttrib> aAttribList;
    m_xEditEngine->GetCharAttribs(aSelection.nStartPara, aAttribList);

    const EECharAttrib* pBeginAttrib = FindCharAttrib(aSelection.nStartPos, aAttribList);
    if (pBeginAttrib &&
            (pBeginAttrib->nStart <= aSelection.nStartPos
                            && pBeginAttrib->nEnd >= aSelection.nEndPos))
    {
        const sal_uInt32 nPara = aSelection.nStartPara;
        ESelection aEntrySel(nPara, pBeginAttrib->nStart, nPara, pBeginAttrib->nEnd);
        m_xEditView->SetSelection(aEntrySel);
        Invalidate();
    }
}

OUString AddressMultiLineEdit::GetAddress() const
{
    OUString sRet;
    const sal_uInt32 nParaCount = m_xEditEngine->GetParagraphCount();
    for(sal_uInt32 nPara = nParaCount; nPara; --nPara)
    {
        const OUString sPara = comphelper::string::stripEnd(m_xEditEngine->GetText(nPara - 1), ' ');
        //don't add empty trailing paragraphs
        if(!sRet.isEmpty() || !sPara.isEmpty())
        {
            sRet = sPara + sRet;
            //insert the para break
            if(nPara > 1)
                sRet = "\n" + sRet;
        }
    }
    return sRet;
}

void AddressMultiLineEdit::UpdateFields()
{
    ESelection aSelection = m_xEditView->GetSelection();

    //restore the attributes
    SetText( GetAddress() );

    //reselect the element
    m_xEditView->SetSelection(aSelection);
    m_aSelectionLink.Call(false);
}

void AddressMultiLineEdit::EditViewSelectionChange()
{
    WeldEditView::EditViewSelectionChange();
    m_aSelectionLink.Call(true);
}

namespace
{
    // sit between the tree as drag source and the editview as drop target and translate
    // the tree dnd data to the simple string the editview wants
    class DropTargetListener : public cppu::WeakImplHelper< css::datatransfer::dnd::XDropTargetListener,
                                                            css::datatransfer::dnd::XDropTarget >
    {
    private:
        css::uno::Reference<css::datatransfer::dnd::XDropTarget> m_xRealDropTarget;
        std::vector<css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>> m_aListeners;
        SwCustomizeAddressBlockDialog* m_pParentDialog;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& ) override
        {
            m_xRealDropTarget.clear();
            m_aListeners.clear();
        }

        // XDropTargetListener
        virtual void SAL_CALL drop( const css::datatransfer::dnd::DropTargetDropEvent& dtde ) override
        {
            SolarMutexGuard aGuard;

            auto aReplacement(dtde);

            Point aMousePos(dtde.LocationX, dtde.LocationY);
            bool bAllowed = m_pParentDialog->SetCursorLogicPosition(aMousePos);
            if (bAllowed)
            {
                if (weld::TreeView* pTree = m_pParentDialog->get_drag_source())
                {
                    int nEntry = pTree->get_selected_index();
                    if (nEntry != -1)
                    {
                        sal_Int32 nUserData = pTree->get_id(nEntry).toInt32();
                        //special entries can only be once in the address / greeting
                        if (nUserData >= 0 || !m_pParentDialog->HasItem(nUserData))
                        {
                            rtl::Reference<TransferDataContainer> xContainer = new TransferDataContainer;
                            xContainer->CopyString( "<" + pTree->get_text(nEntry) + ">" );

                            // replace what the treeview is offering with what ImpEditView::drop wants
                            aReplacement.Transferable = xContainer.get();
                        }
                    }
                }
            }

            std::vector<css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>> aListeners(m_aListeners);
            for (auto const& listener : aListeners)
                listener->drop(aReplacement);

            if (bAllowed)
                m_pParentDialog->UpdateFields();
        }

        virtual void SAL_CALL dragEnter( const css::datatransfer::dnd::DropTargetDragEnterEvent& dtdee ) override
        {
            auto aReplacement(dtdee);
            // replace what the treeview is offering with what ImpEditView::dragEnter wants
            aReplacement.SupportedDataFlavors.realloc(1);
            SotExchange::GetFormatDataFlavor(SotClipboardFormatId::STRING, aReplacement.SupportedDataFlavors.getArray()[0]);

            std::vector<css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>> aListeners(m_aListeners);
            for (auto const& listener : aListeners)
                listener->dragEnter(aReplacement);
        }

        virtual void SAL_CALL dragExit( const css::datatransfer::dnd::DropTargetEvent& dte ) override
        {
            std::vector<css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>> aListeners(m_aListeners);
            for (auto const& listener : aListeners)
                listener->dragExit( dte );
        }

        virtual void SAL_CALL dragOver( const css::datatransfer::dnd::DropTargetDragEvent& dtde ) override
        {
            std::vector<css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>> aListeners(m_aListeners);
            for (auto const& listener : aListeners)
                listener->dragOver( dtde );
        }

        virtual void SAL_CALL dropActionChanged( const css::datatransfer::dnd::DropTargetDragEvent& dtde ) override
        {
            std::vector<css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>> aListeners(m_aListeners);
            for (auto const& listener : aListeners)
                listener->dropActionChanged( dtde );
        }

        // XDropTarget
        virtual void SAL_CALL addDropTargetListener(const css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>& xListener) override
        {
            m_aListeners.push_back(xListener);
        }

        virtual void SAL_CALL removeDropTargetListener(const css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>& xListener) override
        {
            std::erase(m_aListeners, xListener);
        }

        virtual sal_Bool SAL_CALL isActive() override
        {
            return m_xRealDropTarget->isActive();
        }

        virtual void SAL_CALL setActive(sal_Bool active) override
        {
            m_xRealDropTarget->setActive(active);
        }

        virtual sal_Int8 SAL_CALL getDefaultActions() override
        {
            return m_xRealDropTarget->getDefaultActions();
        }

        virtual void SAL_CALL setDefaultActions(sal_Int8 actions) override
        {
            m_xRealDropTarget->setDefaultActions(actions);
        }

    public:
        DropTargetListener(css::uno::Reference<css::datatransfer::dnd::XDropTarget> xRealDropTarget,
                           SwCustomizeAddressBlockDialog* pParentDialog)
            : m_xRealDropTarget(std::move(xRealDropTarget))
            , m_pParentDialog(pParentDialog)
        {
        }
    };
}

css::uno::Reference<css::datatransfer::dnd::XDropTarget> AddressMultiLineEdit::GetDropTarget()
{
    if (!m_xDropTarget.is())
    {
        auto xRealDropTarget = GetDrawingArea()->get_drop_target();
        rtl::Reference<DropTargetListener> pProxy = new DropTargetListener(xRealDropTarget, m_pParentDialog);
        xRealDropTarget->addDropTargetListener(pProxy);
        m_xDropTarget = pProxy;
    }
    return m_xDropTarget;
}

bool AddressMultiLineEdit::SetCursorLogicPosition(const Point& rPosition)
{
    Point aMousePos = EditViewOutputDevice().PixelToLogic(rPosition);
    m_xEditView->SetCursorLogicPosition(aMousePos, false, true);

    ESelection aSelection = m_xEditView->GetSelection();
    std::vector<EECharAttrib> aAttribList;
    m_xEditEngine->GetCharAttribs(aSelection.nStartPara, aAttribList);
    return FindCharAttrib(aSelection.nStartPos, aAttribList) == nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
