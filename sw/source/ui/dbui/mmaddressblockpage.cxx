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
#include <vcl/fixed.hxx>
#include <vcl/weld.hxx>
#include <vcl/txtattr.hxx>
#include <vcl/xtextedt.hxx>
#include <vcl/textview.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/treelistentry.hxx>
#include <mmconfigitem.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdb/XColumn.hpp>
#include <comphelper/string.hxx>
#include <sal/log.hxx>

#include <vector>
#include <globals.hrc>
#include <strings.hrc>
#include <dbui.hrc>
#include <mmaddressblockpage.hrc>
#include <helpids.h>

using namespace svt;
using namespace ::com::sun::star;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;

static void lcl_Move(Control* pCtrl, long nYOffset)
{
    Point aPos(pCtrl->GetPosPixel());
    aPos.AdjustY(nYOffset );
    pCtrl->SetPosPixel(aPos);
}

SwMailMergeAddressBlockPage::SwMailMergeAddressBlockPage( SwMailMergeWizard* _pParent) :
    svt::OWizardPage(_pParent, "MMAddressBlockPage",
        "modules/swriter/ui/mmaddressblockpage.ui")
    , m_pWizard(_pParent)
{
    get(m_pAddressListPB, "addresslist");
    get(m_pCurrentAddressFI, "currentaddress");
    get(m_pStep2, "step2");
    get(m_pStep3, "step3");
    get(m_pStep4, "step4");
    get(m_pSettingsFI, "settingsft");
    get(m_pAddressCB, "address");
    get(m_pSettingsWIN, "settingspreview");
    Size aSize(LogicToPixel(Size(164 , 45), MapMode(MapUnit::MapAppFont)));
    m_pSettingsWIN->set_width_request(aSize.Width());
    m_pSettingsWIN->set_height_request(aSize.Height());
    get(m_pSettingsPB, "settings");
    get(m_pHideEmptyParagraphsCB, "hideempty");
    get(m_pAssignPB, "assign");
    get(m_pPreviewWIN, "addresspreview");
    aSize = LogicToPixel(Size(176, 46), MapMode(MapUnit::MapAppFont));
    m_pPreviewWIN->set_width_request(aSize.Width());
    m_pPreviewWIN->set_height_request(aSize.Height());
    get(m_pDocumentIndexFI, "documentindex");
    get(m_pPrevSetIB, "prev");
    get(m_pNextSetIB, "next");
    m_sDocument = m_pDocumentIndexFI->GetText();
    m_sChangeAddress = get<Button>("differentlist")->GetText();

    m_sCurrentAddress = m_pCurrentAddressFI->GetText();
    m_pAddressListPB->SetClickHdl(LINK(this, SwMailMergeAddressBlockPage, AddressListHdl_Impl));
    m_pSettingsPB->SetClickHdl(LINK(this, SwMailMergeAddressBlockPage, SettingsHdl_Impl));
    m_pAssignPB->SetClickHdl(LINK(this, SwMailMergeAddressBlockPage, AssignHdl_Impl ));
    m_pAddressCB->SetClickHdl(LINK(this, SwMailMergeAddressBlockPage, AddressBlockHdl_Impl));
    m_pSettingsWIN->SetSelectHdl(LINK(this, SwMailMergeAddressBlockPage, AddressBlockSelectHdl_Impl));
    m_pHideEmptyParagraphsCB->SetClickHdl(LINK(this, SwMailMergeAddressBlockPage, HideParagraphsHdl_Impl));

    Link<Button*,void> aLink = LINK(this, SwMailMergeAddressBlockPage, InsertDataHdl_Impl);
    m_pPrevSetIB->SetClickHdl(aLink);
    m_pNextSetIB->SetClickHdl(aLink);
}

SwMailMergeAddressBlockPage::~SwMailMergeAddressBlockPage()
{
    disposeOnce();
}

void SwMailMergeAddressBlockPage::dispose()
{
    m_pAddressListPB.clear();
    m_pCurrentAddressFI.clear();
    m_pStep2.clear();
    m_pStep3.clear();
    m_pStep4.clear();
    m_pSettingsFI.clear();
    m_pAddressCB.clear();
    m_pSettingsWIN.clear();
    m_pSettingsPB.clear();
    m_pHideEmptyParagraphsCB.clear();
    m_pAssignPB.clear();
    m_pPreviewWIN.clear();
    m_pDocumentIndexFI.clear();
    m_pPrevSetIB.clear();
    m_pNextSetIB.clear();
    m_pWizard.clear();
    svt::OWizardPage::dispose();
}

bool SwMailMergeAddressBlockPage::canAdvance() const
{
    return m_pWizard->GetConfigItem().GetResultSet().is();
}

void SwMailMergeAddressBlockPage::ActivatePage()
{
    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
    bool bIsLetter = rConfigItem.IsOutputToLetter();

    //no address block is created for e-Mail
    m_pStep2->Show(bIsLetter);
    m_pStep3->Show(bIsLetter);
    m_pStep4->Show(bIsLetter);

    if (bIsLetter)
    {
        m_pHideEmptyParagraphsCB->Check( rConfigItem.IsHideEmptyParagraphs() );
        m_pDocumentIndexFI->SetText(m_sDocument.replaceFirst("%1", "1"));

        m_pSettingsWIN->Clear();
        const uno::Sequence< OUString> aBlocks =
                    m_pWizard->GetConfigItem().GetAddressBlocks();
        for(sal_Int32 nAddress = 0; nAddress < aBlocks.getLength(); ++nAddress)
            m_pSettingsWIN->AddAddress(aBlocks[nAddress]);
        m_pSettingsWIN->SelectAddress(static_cast<sal_uInt16>(rConfigItem.GetCurrentAddressBlockIndex()));
        m_pAddressCB->Check(rConfigItem.IsAddressBlock());
        AddressBlockHdl_Impl(m_pAddressCB);
        m_pSettingsWIN->SetLayout(1, 2);
        InsertDataHdl_Impl(nullptr);
    }
}

bool SwMailMergeAddressBlockPage::commitPage( ::svt::WizardTypes::CommitPageReason _eReason )
{
    return ::svt::WizardTypes::eTravelForward != _eReason || m_pWizard->GetConfigItem().GetResultSet().is();
}

IMPL_LINK_NOARG(SwMailMergeAddressBlockPage, AddressListHdl_Impl, Button*, void)
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
            InsertDataHdl_Impl(nullptr);
            GetWizard()->UpdateRoadmap();
            GetWizard()->enableButtons(WizardButtonFlags::NEXT, GetWizard()->isStateEnabled(MM_GREETINGSPAGE));
        }
    }
    catch (const uno::Exception& e)
    {
        SAL_WARN("sw", e);
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                  VclMessageType::Warning, VclButtonsType::Ok, e.Message));
        xBox->run();
    }
}

IMPL_LINK_NOARG(SwMailMergeAddressBlockPage, SettingsHdl_Impl, Button*, void)
{
    SwSelectAddressBlockDialog aDlg(GetFrameWeld(), m_pWizard->GetConfigItem());
    SwMailMergeConfigItem& rConfig = m_pWizard->GetConfigItem();
    aDlg.SetAddressBlocks(rConfig.GetAddressBlocks(), m_pSettingsWIN->GetSelectedAddress());
    aDlg.SetSettings(rConfig.IsIncludeCountry(), rConfig.GetExcludeCountry());
    if (aDlg.run() == RET_OK)
    {
        //the dialog provides the selected address at the first position!
        const uno::Sequence< OUString> aBlocks = aDlg.GetAddressBlocks();
        rConfig.SetAddressBlocks(aBlocks);
        m_pSettingsWIN->Clear();
        for(sal_Int32 nAddress = 0; nAddress < aBlocks.getLength(); ++nAddress)
            m_pSettingsWIN->AddAddress(aBlocks[nAddress]);
        m_pSettingsWIN->SelectAddress(0);
        m_pSettingsWIN->Invalidate();    // #i40408
        rConfig.SetCountrySettings(aDlg.IsIncludeCountry(), aDlg.GetCountry());
        InsertDataHdl_Impl(nullptr);
    }
    GetWizard()->UpdateRoadmap();
    GetWizard()->enableButtons(WizardButtonFlags::NEXT, GetWizard()->isStateEnabled(MM_GREETINGSPAGE));
}

IMPL_LINK(SwMailMergeAddressBlockPage, AssignHdl_Impl, Button*, pButton, void)
{
    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
    const sal_uInt16 nSel = m_pSettingsWIN->GetSelectedAddress();
    const uno::Sequence< OUString> aBlocks = rConfigItem.GetAddressBlocks();
    ScopedVclPtr<SwAssignFieldsDialog> pDlg(
        VclPtr<SwAssignFieldsDialog>::Create(
            pButton, m_pWizard->GetConfigItem(), aBlocks[nSel], true));
    if(RET_OK == pDlg->Execute())
    {
        //preview update
        InsertDataHdl_Impl(nullptr);
        GetWizard()->UpdateRoadmap();
        GetWizard()->enableButtons(WizardButtonFlags::NEXT, GetWizard()->isStateEnabled(MM_GREETINGSPAGE));
    }
}

void SwMailMergeAddressBlockPage::EnableAddressBlock(bool bAll, bool bSelective)
{
    m_pSettingsFI->Enable(bAll);
    m_pAddressCB->Enable(bAll);
    bSelective &= bAll;
    m_pHideEmptyParagraphsCB->Enable(bSelective);
    m_pSettingsWIN->Enable(bSelective);
    m_pSettingsPB->Enable(bSelective);
    m_pStep3->Enable(bSelective);
    m_pStep4->Enable(bSelective);
}

IMPL_LINK(SwMailMergeAddressBlockPage, AddressBlockHdl_Impl, Button*, pBox, void)
{
    EnableAddressBlock(pBox->IsEnabled(), static_cast<CheckBox*>(pBox)->IsChecked());
    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
    rConfigItem.SetAddressBlock(m_pAddressCB->IsChecked());
    m_pWizard->UpdateRoadmap();
    GetWizard()->enableButtons(WizardButtonFlags::NEXT, GetWizard()->isStateEnabled(MM_GREETINGSPAGE));
}

IMPL_LINK_NOARG(SwMailMergeAddressBlockPage, AddressBlockSelectHdl_Impl, LinkParamNone*, void)
{
    const sal_uInt16 nSel = m_pSettingsWIN->GetSelectedAddress();
    const uno::Sequence< OUString> aBlocks =
                m_pWizard->GetConfigItem().GetAddressBlocks();
    m_pPreviewWIN->SetAddress(SwAddressPreview::FillData(aBlocks[nSel],
                                                         m_pWizard->GetConfigItem()));
    m_pWizard->GetConfigItem().SetCurrentAddressBlockIndex( nSel );
    GetWizard()->UpdateRoadmap();
    GetWizard()->enableButtons(WizardButtonFlags::NEXT, GetWizard()->isStateEnabled(MM_GREETINGSPAGE));
}

IMPL_LINK(SwMailMergeAddressBlockPage, HideParagraphsHdl_Impl, Button*, pBox, void)
{
    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
    rConfigItem.SetHideEmptyParagraphs( static_cast<CheckBox*>(pBox)->IsChecked() );
}

IMPL_LINK(SwMailMergeAddressBlockPage, InsertDataHdl_Impl, Button*, pButton, void)
{
    //if no pButton is given, the first set has to be pre-set
    SwMailMergeConfigItem& rConfig = m_pWizard->GetConfigItem();
    m_pWizard->EnterWait();
    if(!pButton)
    {
        rConfig.GetResultSet();
    }
    else
    {
        bool bNext = pButton == m_pNextSetIB;
        sal_Int32 nPos = rConfig.GetResultSetPosition();
        rConfig.MoveResultSet( bNext ? ++nPos : --nPos);
    }
    m_pWizard->LeaveWait();
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
        if(m_pSettingsWIN->IsVisible())
        {
            //Fill data into preview
            const sal_uInt16 nSel = m_pSettingsWIN->GetSelectedAddress();
            const uno::Sequence< OUString> aBlocks =
                        m_pWizard->GetConfigItem().GetAddressBlocks();
            m_pPreviewWIN->SetAddress(SwAddressPreview::FillData(aBlocks[nSel], rConfig));
        }
    }
    m_pPrevSetIB->Enable(bEnable);
    m_pDocumentIndexFI->SetText(m_sDocument.replaceFirst("%1", OUString::number(nPos)));

    GetWizard()->enableButtons(WizardButtonFlags::NEXT, GetWizard()->isStateEnabled(MM_GREETINGSPAGE));
    bool bHasResultSet = rConfig.GetResultSet().is();
    m_pCurrentAddressFI->Show(bHasResultSet);
    if(bHasResultSet)
    {
        m_pCurrentAddressFI->SetText(m_sCurrentAddress.replaceFirst("%1", rConfig.GetCurrentDBData().sDataSource));
        m_pAddressListPB->SetText(m_sChangeAddress);
    }
    EnableAddressBlock(bHasResultSet, m_pAddressCB->IsChecked());
}

SwSelectAddressBlockDialog::SwSelectAddressBlockDialog(weld::Window* pParent, SwMailMergeConfigItem& rConfig)
    : SfxDialogController(pParent, "modules/swriter/ui/selectblockdialog.ui", "SelectBlockDialog")
    , m_rConfig(rConfig)
    , m_xPreview(new AddressPreview(m_xBuilder->weld_scrolled_window("previewwin")))
    , m_xNewPB(m_xBuilder->weld_button("new"))
    , m_xCustomizePB(m_xBuilder->weld_button("edit"))
    , m_xDeletePB(m_xBuilder->weld_button("delete"))
    , m_xNeverRB(m_xBuilder->weld_radio_button("never"))
    , m_xAlwaysRB(m_xBuilder->weld_radio_button("always"))
    , m_xDependentRB(m_xBuilder->weld_radio_button("dependent"))
    , m_xCountryED(m_xBuilder->weld_entry("country"))
    , m_xPreviewWin(new weld::CustomWeld(*m_xBuilder, "preview", *m_xPreview))
{
    m_xPreviewWin->set_size_request(m_xCountryED->get_approximate_digit_width() * 45,
                                    m_xCountryED->get_text_height() * 12);

    Link<weld::Button&,void> aCustomizeHdl = LINK(this, SwSelectAddressBlockDialog, NewCustomizeHdl_Impl);
    m_xNewPB->connect_clicked(aCustomizeHdl);
    m_xCustomizePB->connect_clicked(aCustomizeHdl);

    m_xDeletePB->connect_clicked(LINK(this, SwSelectAddressBlockDialog, DeleteHdl_Impl));

    Link<weld::ToggleButton&,void> aLk = LINK(this, SwSelectAddressBlockDialog, IncludeHdl_Impl);
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
    for (sal_Int32 nAddress = 0; nAddress < m_aAddressBlocks.getLength(); ++nAddress)
        m_xPreview->AddAddress(m_aAddressBlocks[nAddress]);
    m_xPreview->SelectAddress(nSelectedAddress);
}

// return the address blocks and put the selected one to the first position
const uno::Sequence< OUString >&    SwSelectAddressBlockDialog::GetAddressBlocks()
{
    //put the selected block to the first position
    const sal_Int32 nSelect = static_cast<sal_Int32>(m_xPreview->GetSelectedAddress());
    if(nSelect)
    {
        uno::Sequence< OUString >aTemp = m_aAddressBlocks;
        OUString* pTemp = aTemp.getArray();
        pTemp[0] = m_aAddressBlocks[nSelect];
        sal_Int32 nIndex = 0;
        const sal_Int32 nNumBlocks = m_aAddressBlocks.getLength();
        for(sal_Int32 nAddress = 1; nAddress < nNumBlocks; ++nAddress)
        {
            if(nIndex == nSelect)
                ++nIndex;
            pTemp[nAddress] = m_aAddressBlocks[nIndex];
            nIndex++;
        }
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
    if(m_aAddressBlocks.getLength())
    {
        const sal_Int32 nSelected = static_cast<sal_Int32>(m_xPreview->GetSelectedAddress());
        OUString* pAddressBlocks = m_aAddressBlocks.getArray();
        sal_Int32 nSource = 0;
        for(sal_Int32 nTarget = 0; nTarget < m_aAddressBlocks.getLength() - 1; nTarget++)
        {
            if(nSource == nSelected)
                ++nSource;
            pAddressBlocks[nTarget] = pAddressBlocks[nSource++];
        }
        m_aAddressBlocks.realloc(m_aAddressBlocks.getLength() - 1);
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
    ScopedVclPtr<SwCustomizeAddressBlockDialog> pDlg(
        VclPtr<SwCustomizeAddressBlockDialog>::Create(
            nullptr /*TODO*/,m_rConfig,nType));
    if(bCustomize)
    {
        pDlg->SetAddress(m_aAddressBlocks[m_xPreview->GetSelectedAddress()]);
    }
    if(RET_OK == pDlg->Execute())
    {
        const OUString sNew = pDlg->GetAddress();
        if(bCustomize)
        {
            m_xPreview->ReplaceSelectedAddress(sNew);
            m_aAddressBlocks[m_xPreview->GetSelectedAddress()] = sNew;
        }
        else
        {
            m_xPreview->AddAddress(sNew);
            m_aAddressBlocks.realloc(m_aAddressBlocks.getLength() + 1);
            const sal_Int32 nSelect = m_aAddressBlocks.getLength() - 1;
            m_aAddressBlocks[nSelect] = sNew;
            m_xPreview->SelectAddress(static_cast<sal_uInt16>(nSelect));
        }
        m_xDeletePB->set_sensitive(m_aAddressBlocks.getLength() > 1);
    }
}

IMPL_LINK_NOARG(SwSelectAddressBlockDialog, IncludeHdl_Impl, weld::ToggleButton&,  void)
{
    m_xCountryED->set_sensitive(m_xDependentRB->get_active());
}

#define USER_DATA_SALUTATION        -1
#define USER_DATA_PUNCTUATION       -2
#define USER_DATA_TEXT              -3
#define USER_DATA_NONE              -4

SwCustomizeAddressBlockDialog::SwCustomizeAddressBlockDialog(
        vcl::Window* pParent, SwMailMergeConfigItem& rConfig, DialogType eType)
    : SfxModalDialog(pParent, "AddressBlockDialog",
        "modules/swriter/ui/addressblockdialog.ui")
    , m_aTextFilter("<>")
    , m_rConfigItem(rConfig)
    , m_eType(eType)
{
    get(m_pOK, "ok");
    get(m_pPreviewWIN, "addrpreview");
    get(m_pFieldCB, "custom");
    m_pFieldCB->SetTextFilter(&m_aTextFilter);
    get(m_pFieldFT, "customft");
    get(m_pDownIB, "down");
    get(m_pRightIB, "right");
    get(m_pLeftIB, "left");
    get(m_pUpIB, "up");
    get(m_pDragED, "addressdest");
    m_pDragED->SetAddressDialog(this);
    get(m_pDragFT, "addressdestft");
    get(m_pRemoveFieldIB, "fromaddr");
    get(m_pInsertFieldIB, "toaddr");
    get(m_pAddressElementsLB, "addresses");
    m_pAddressElementsLB->set_height_request(16 * m_pAddressElementsLB->GetTextHeight());
    m_pAddressElementsLB->SetAddressDialog(this);
    get(m_pAddressElementsFT, "addressesft");

    if( eType >= GREETING_FEMALE )
    {
        m_pFieldFT->Show();
        m_pFieldCB->Show();
        SvTreeListEntry* pEntry = m_pAddressElementsLB->InsertEntry(SwResId(ST_SALUTATION));
        pEntry->SetUserData(reinterpret_cast<void*>(sal_Int32(USER_DATA_SALUTATION)) );
        pEntry = m_pAddressElementsLB->InsertEntry(SwResId(ST_PUNCTUATION));
        pEntry->SetUserData(reinterpret_cast<void*>(sal_Int32(USER_DATA_PUNCTUATION)) );
        pEntry = m_pAddressElementsLB->InsertEntry(SwResId(ST_TEXT));
        pEntry->SetUserData(reinterpret_cast<void*>(sal_Int32(USER_DATA_TEXT))       );
        for (size_t i = 0; i < SAL_N_ELEMENTS(RA_SALUTATION); ++i)
            m_aSalutations.push_back(SwResId(RA_SALUTATION[i]));
        for (size_t i = 0; i < SAL_N_ELEMENTS(RA_PUNCTUATION); ++i)
            m_aPunctuations.push_back(SwResId(RA_PUNCTUATION[i]));
        m_pDragED->SetText("            ");
        SetText(SwResId(eType == GREETING_MALE ? ST_TITLE_MALE : ST_TITLE_FEMALE));
        m_pAddressElementsFT->SetText(SwResId(ST_SALUTATIONELEMENTS));
        m_pInsertFieldIB->SetQuickHelpText(SwResId(ST_INSERTSALUTATIONFIELD));
        m_pRemoveFieldIB->SetQuickHelpText(SwResId(ST_REMOVESALUTATIONFIELD));
        m_pDragFT->SetText(SwResId(ST_DRAGSALUTATION));
    }
    else
    {
        if(eType == ADDRESSBLOCK_EDIT)
            SetText(SwResId(ST_TITLE_EDIT));
        m_pDragED->SetText("\n\n\n\n\n");
        /* Set custom HIDs for swriter/01/mm_newaddblo.xhp */
        m_pAddressElementsLB->SetHelpId( HID_MM_ADDBLOCK_ELEMENTS );
        m_pInsertFieldIB->SetHelpId( HID_MM_ADDBLOCK_INSERT );
        m_pRemoveFieldIB->SetHelpId( HID_MM_ADDBLOCK_REMOVE );
        m_pDragED->SetHelpId( HID_MM_ADDBLOCK_DRAG );
        m_pPreviewWIN->SetHelpId( HID_MM_ADDBLOCK_PREVIEW );
        m_pRightIB->SetHelpId( HID_MM_ADDBLOCK_MOVEBUTTONS );
        m_pLeftIB->SetHelpId( HID_MM_ADDBLOCK_MOVEBUTTONS );
        m_pDownIB->SetHelpId( HID_MM_ADDBLOCK_MOVEBUTTONS );
        m_pUpIB->SetHelpId( HID_MM_ADDBLOCK_MOVEBUTTONS );
    }

    const std::vector<std::pair<OUString, int>>& rHeaders = m_rConfigItem.GetDefaultAddressHeaders();
    for (size_t i = 0; i < rHeaders.size(); ++i)
    {
        SvTreeListEntry* pEntry = m_pAddressElementsLB->InsertEntry(rHeaders[i].first);
        pEntry->SetUserData(reinterpret_cast<void*>(static_cast<sal_IntPtr>(i)));
    }
    m_pOK->SetClickHdl(LINK(this, SwCustomizeAddressBlockDialog, OKHdl_Impl));
    m_pAddressElementsLB->SetSelectHdl(LINK(this, SwCustomizeAddressBlockDialog, ListBoxSelectHdl_Impl ));
    m_pDragED->SetModifyHdl(LINK(this, SwCustomizeAddressBlockDialog, EditModifyHdl_Impl));
    m_pDragED->SetSelectionChangedHdl( LINK( this, SwCustomizeAddressBlockDialog, SelectionChangedHdl_Impl));
    Link<Edit&,void> aFieldsLink = LINK(this, SwCustomizeAddressBlockDialog, FieldChangeHdl_Impl);
    m_pFieldCB->SetModifyHdl(aFieldsLink);
    m_pFieldCB->SetSelectHdl(LINK(this, SwCustomizeAddressBlockDialog, FieldChangeComboBoxHdl_Impl));
    Link<Button*,void> aImgButtonHdl = LINK(this, SwCustomizeAddressBlockDialog, ImageButtonHdl_Impl);
    m_pInsertFieldIB->SetClickHdl(aImgButtonHdl);
    m_pRemoveFieldIB->SetClickHdl(aImgButtonHdl);
    m_pUpIB->SetClickHdl(aImgButtonHdl);
    m_pLeftIB->SetClickHdl(aImgButtonHdl);
    m_pRightIB->SetClickHdl(aImgButtonHdl);
    m_pDownIB->SetClickHdl(aImgButtonHdl);
    UpdateImageButtons_Impl();
}

SwCustomizeAddressBlockDialog::~SwCustomizeAddressBlockDialog()
{
    disposeOnce();
}

void SwCustomizeAddressBlockDialog::dispose()
{
    m_pAddressElementsFT.clear();
    m_pAddressElementsLB.clear();
    m_pInsertFieldIB.clear();
    m_pRemoveFieldIB.clear();
    m_pDragFT.clear();
    m_pDragED.clear();
    m_pUpIB.clear();
    m_pLeftIB.clear();
    m_pRightIB.clear();
    m_pDownIB.clear();
    m_pFieldFT.clear();
    m_pFieldCB.clear();
    m_pPreviewWIN.clear();
    m_pOK.clear();
    SfxModalDialog::dispose();
}

IMPL_LINK_NOARG(SwCustomizeAddressBlockDialog, OKHdl_Impl, Button*, void)
{
    EndDialog(RET_OK);
}

IMPL_LINK(SwCustomizeAddressBlockDialog, ListBoxSelectHdl_Impl, SvTreeListBox*, pBox, void)
{
    sal_Int32 nUserData = static_cast<sal_Int32>(reinterpret_cast<sal_IntPtr>(pBox->FirstSelected()->GetUserData()));
    // Check if the selected entry is already in the address and then forbid inserting
    m_pInsertFieldIB->Enable(nUserData >= 0 || !HasItem_Impl(nUserData));
}

IMPL_LINK_NOARG(SwCustomizeAddressBlockDialog, EditModifyHdl_Impl, Edit&, void)
{
    m_pPreviewWIN->SetAddress(SwAddressPreview::FillData(GetAddress(), m_rConfigItem));
    UpdateImageButtons_Impl();
}

IMPL_LINK(SwCustomizeAddressBlockDialog, ImageButtonHdl_Impl, Button*, pButton, void)
{
    if (m_pInsertFieldIB == pButton)
    {
        SvTreeListEntry* pEntry = m_pAddressElementsLB->GetCurEntry();
        if(pEntry)
        {
            m_pDragED->InsertNewEntry("<" + m_pAddressElementsLB->GetEntryText(pEntry) + ">");
        }
    }
    else if (m_pRemoveFieldIB == pButton)
    {
        m_pDragED->RemoveCurrentEntry();
    }
    else
    {
        MoveItemFlags nMove = MoveItemFlags::Down;
        if (m_pUpIB == pButton)
            nMove = MoveItemFlags::Up;
        else if (m_pLeftIB == pButton)
            nMove = MoveItemFlags::Left;
        else if (m_pRightIB == pButton)
            nMove = MoveItemFlags::Right;
        m_pDragED->MoveCurrentItem(nMove);
    }
    UpdateImageButtons_Impl();
}

sal_Int32 SwCustomizeAddressBlockDialog::GetSelectedItem_Impl()
{
    sal_Int32 nRet = USER_DATA_NONE;
    const OUString sSelected = m_pDragED->GetCurrentItem();
    if(!sSelected.isEmpty())
        for(sal_uLong i = 0; i < m_pAddressElementsLB->GetEntryCount();  ++i)
        {
            SvTreeListEntry* pEntry = m_pAddressElementsLB->GetEntry(i);
            const OUString sEntry = m_pAddressElementsLB->GetEntryText(pEntry);
            if( sEntry == sSelected.copy( 1, sSelected.getLength() - 2 ) )
            {
                nRet = static_cast<sal_Int32>(reinterpret_cast<sal_IntPtr>(pEntry->GetUserData()));
                break;
            }
        }
    return nRet;
}

bool   SwCustomizeAddressBlockDialog::HasItem_Impl(sal_Int32 nUserData)
{
    //get the entry from the ListBox
    OUString sEntry;
    for(sal_uLong i = 0; i < m_pAddressElementsLB->GetEntryCount();  ++i)
    {
        SvTreeListEntry* pEntry = m_pAddressElementsLB->GetEntry(i);
        if(static_cast<sal_Int32>(reinterpret_cast<sal_IntPtr>(pEntry->GetUserData())) == nUserData)
        {
            sEntry = m_pAddressElementsLB->GetEntryText(pEntry);
            break;
        }
    }
    //search for this entry in the content
    return m_pDragED->GetText().indexOf("<" + sEntry + ">") >= 0;
}

IMPL_LINK(SwCustomizeAddressBlockDialog, SelectionChangedHdl_Impl, AddressMultiLineEdit&, rEdit, void)
{
    // called in case the selection of the edit field changes.
    // determine selection - if it's one of the editable fields then
    // enable the related ComboBox and fill it
    static bool bOnEntry = false;
    if(bOnEntry)
        return;

    bOnEntry = true;
    sal_Int32 nSelected = GetSelectedItem_Impl();
    if(USER_DATA_NONE != nSelected)
        rEdit.SelectCurrentItem();

    if(m_pFieldCB->IsVisible() && (USER_DATA_NONE != nSelected) && (nSelected < 0))
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
        m_pFieldCB->Clear();
        if(pVector) {
            for (const auto& rItem : *pVector)
                m_pFieldCB->InsertEntry(rItem);
        }
        m_pFieldCB->SetText(sSelect);
        m_pFieldCB->Enable();
        m_pFieldFT->Enable();
    }
    else
    {
        m_pFieldCB->Enable(false);
        m_pFieldFT->Enable(false);
    }

    UpdateImageButtons_Impl();
    bOnEntry = false;
}

IMPL_LINK_NOARG(SwCustomizeAddressBlockDialog, FieldChangeComboBoxHdl_Impl, ComboBox&, void)
{
    FieldChangeHdl_Impl(*m_pFieldCB);
}
IMPL_LINK_NOARG(SwCustomizeAddressBlockDialog, FieldChangeHdl_Impl, Edit&, void)
{
    //changing the field content changes the related members, too
    sal_Int32 nSelected = GetSelectedItem_Impl();
    const OUString sContent = m_pFieldCB->GetText();
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
    m_pPreviewWIN->SetAddress(GetAddress());
    m_pDragED->Modify();
}

void SwCustomizeAddressBlockDialog::UpdateImageButtons_Impl()
{
    MoveItemFlags nMove = m_pDragED->IsCurrentItemMoveable();
    m_pUpIB->Enable( bool(nMove & MoveItemFlags::Up) );
    m_pLeftIB->Enable( bool(nMove & MoveItemFlags::Left) );
    m_pRightIB->Enable( bool(nMove & MoveItemFlags::Right) );
    m_pDownIB->Enable( bool(nMove & MoveItemFlags::Down) );
    m_pRemoveFieldIB->Enable(m_pDragED->HasCurrentItem());
    SvTreeListEntry* pEntry = m_pAddressElementsLB->GetCurEntry();
    m_pInsertFieldIB->Enable( pEntry &&
            (0 < static_cast<sal_Int32>(reinterpret_cast<sal_IntPtr>(pEntry->GetUserData())) || !m_pFieldCB->GetText().isEmpty()));
}

void SwCustomizeAddressBlockDialog::SetAddress(const OUString& rAddress)
{
    m_pDragED->SetText( rAddress );
    UpdateImageButtons_Impl();
    m_pDragED->Modify();
}

OUString SwCustomizeAddressBlockDialog::GetAddress()
{
    OUString sAddress(m_pDragED->GetAddress());
    //remove placeholders by the actual content
    if(m_pFieldFT->IsVisible())
    {
        for(sal_uLong i = 0; i < m_pAddressElementsLB->GetEntryCount();  ++i)
        {
            SvTreeListEntry* pEntry = m_pAddressElementsLB->GetEntry(i);
            const OUString sEntry = "<" + m_pAddressElementsLB->GetEntryText(pEntry) + ">";
            sal_Int32 nUserData = static_cast<sal_Int32>(reinterpret_cast<sal_IntPtr>(pEntry->GetUserData()));
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

class SwAssignFieldsControl : public Control
{
    friend class SwAssignFieldsDialog;
    VclPtr<ScrollBar>           m_aVScroll;
    VclPtr<HeaderBar>           m_aHeaderHB;
    VclPtr<Window>              m_aWindow;

    std::vector<VclPtr<FixedText> >     m_aFieldNames;
    std::vector<VclPtr<ListBox> >       m_aMatches;
    std::vector<VclPtr<FixedText> >     m_aPreviews;

    SwMailMergeConfigItem*              m_rConfigItem;

    Link<LinkParamNone*,void>   m_aModifyHdl;

    long                        m_nLBStartTopPos;
    long                        m_nYOffset;
    long                        m_nFirstYPos;

    DECL_LINK(ScrollHdl_Impl, ScrollBar*, void);
    DECL_LINK(MatchHdl_Impl, ListBox&, void);
    DECL_LINK(GotFocusHdl_Impl, Control&, void);

    virtual bool        PreNotify( NotifyEvent& rNEvt ) override;
    virtual void        Command( const CommandEvent& rCEvt ) override;

    void                MakeVisible( sal_Int32 nIndex );
public:
    SwAssignFieldsControl(vcl::Window* pParent, WinBits nBits);
    virtual ~SwAssignFieldsControl() override;
    virtual void dispose() override;

    void        Init(SwMailMergeConfigItem& rConfigItem);
    void        SetModifyHdl(const Link<LinkParamNone*,void>& rModifyHdl)
                {
                    m_aModifyHdl = rModifyHdl;
                    m_aModifyHdl.Call(nullptr);
                }
    virtual void Resize() override;
    virtual Size GetOptimalSize() const override;
};

VCL_BUILDER_FACTORY_ARGS(SwAssignFieldsControl, WB_BORDER)

SwAssignFieldsControl::SwAssignFieldsControl(vcl::Window* pParent, WinBits nBits) :
    Control(pParent, nBits | WB_DIALOGCONTROL | WB_TABSTOP),
    m_aVScroll(VclPtr<ScrollBar>::Create(this)),
    m_aHeaderHB(VclPtr<HeaderBar>::Create(this, WB_BUTTONSTYLE | WB_BOTTOMBORDER)),
    m_aWindow(VclPtr<vcl::Window>::Create(this, WB_BORDER | WB_DIALOGCONTROL)),
    m_rConfigItem(nullptr),
    m_nLBStartTopPos(0),
    m_nYOffset(0),
    m_nFirstYPos(0)
{
    long nHBHeight = m_aHeaderHB->CalcWindowSizePixel().Height();
    Size aOutputSize(GetOutputSize());
    m_aVScroll->Show();
    m_aHeaderHB->SetSizePixel(
        Size(aOutputSize.Width(), nHBHeight));
    m_aHeaderHB->Show();
    m_aWindow->SetPosPixel(Point( 0, nHBHeight) );
    m_aWindow->SetSizePixel(Size(aOutputSize.Width() - m_aVScroll->GetSizePixel().Width(), aOutputSize.Height() - nHBHeight));
    m_aWindow->Show();
}

Size SwAssignFieldsControl::GetOptimalSize() const
{
    return LogicToPixel(Size(248, 120), MapMode(MapUnit::MapAppFont));
}

void SwAssignFieldsControl::Init(SwMailMergeConfigItem& rConfigItem)
{
    m_rConfigItem = &rConfigItem;
    Size aOutputSize(GetOutputSize());
    long nHBHeight = m_aHeaderHB->CalcWindowSizePixel().Height();

    //get the name of the default headers
    const std::vector<std::pair<OUString, int>>& rHeaders = rConfigItem.GetDefaultAddressHeaders();
    //get the actual data
    uno::Reference< XColumnsSupplier > xColsSupp( rConfigItem.GetResultSet(), uno::UNO_QUERY);
    //get the name of the actual columns
    uno::Reference <XNameAccess> xColAccess = xColsSupp.is() ? xColsSupp->getColumns() : nullptr;
    uno::Sequence< OUString > aFields;
    if(xColAccess.is())
        aFields = xColAccess->getElementNames();
    const OUString* pFields = aFields.getConstArray();

    //get the current assignment list
    //each position in this sequence matches the position in the header array rHeaders
    //if no assignment is available an empty sequence will be returned
    uno::Sequence< OUString> aAssignments = rConfigItem.GetColumnAssignment( rConfigItem.GetCurrentDBData() );
    Link<ListBox&,void> aMatchHdl = LINK(this, SwAssignFieldsControl, MatchHdl_Impl);
    Link<Control&,void> aFocusHdl = LINK(this, SwAssignFieldsControl, GotFocusHdl_Impl);

    //fill the controls
    long nControlWidth = aOutputSize.Width() / 3;
    long nControlHeight = -1;
    for (size_t i = 0; i < rHeaders.size(); ++i)
    {
        const OUString rHeader = rHeaders[i].first;
        VclPtr<FixedText> pNewText = VclPtr<FixedText>::Create(m_aWindow.get(), WB_VCENTER);
        pNewText->SetText("<" + rHeader + ">");
        VclPtr<ListBox> pNewLB = VclPtr<ListBox>::Create(m_aWindow.get(), WB_DROPDOWN | WB_VCENTER | WB_TABSTOP);
        pNewText->set_mnemonic_widget(pNewLB);
        pNewLB->InsertEntry(SwResId(SW_STR_NONE));
        pNewLB->SelectEntryPos(0);
        pNewLB->SetDropDownLineCount(5);

        if (nControlHeight == -1) //first time
        {
            nControlHeight = std::max(pNewLB->get_preferred_size().Height(),
                                      pNewText->get_preferred_size().Height());
        }

        for(sal_Int32 nField = 0; nField < aFields.getLength(); ++nField)
            pNewLB->InsertEntry(pFields[nField]);
        VclPtr<FixedText> pNewPreview = VclPtr<FixedText>::Create(m_aWindow.get(), WB_VCENTER);
        pNewText->SetSizePixel(Size(nControlWidth - 6, nControlHeight));
        pNewLB->SetSizePixel(Size(nControlWidth - 6, nControlHeight));
        pNewPreview->SetSizePixel(Size(aOutputSize.Width() - 2 * nControlWidth, nControlHeight));
        //select the ListBox
        //if there is an assignment
        if(static_cast<sal_uInt32>(aAssignments.getLength()) > i && !aAssignments[i].isEmpty())
            pNewLB->SelectEntry(aAssignments[i]);
        else //otherwise the current column name may match one of the db columns
            pNewLB->SelectEntry(rHeader);
        //then the preview can be filled accordingly
        if(xColAccess.is() && pNewLB->GetSelectedEntryPos() > 0 &&
                xColAccess->hasByName(pNewLB->GetSelectedEntry()))
        {
            uno::Any aCol = xColAccess->getByName(pNewLB->GetSelectedEntry());
            uno::Reference< XColumn > xColumn;
            aCol >>= xColumn;
            if(xColumn.is())
            {
                try
                {
                    pNewPreview->SetText(xColumn->getString());
                }
                catch (const SQLException&)
                {
                }
            }
        }
        if(!i)
        {
            //determine the vertical offset, use the bottom position of the ListBox
            m_nFirstYPos = m_nYOffset = pNewLB->GetPosPixel().Y();
            m_nLBStartTopPos = m_nYOffset;
            m_nYOffset += pNewLB->GetSizePixel().Height() + 6;
        }

        long nMove = m_nYOffset * i;
        pNewLB->SetSelectHdl(aMatchHdl);
        pNewLB->SetGetFocusHdl(aFocusHdl);

        m_aFieldNames.push_back(pNewText);
        m_aMatches.push_back(pNewLB);
        m_aPreviews.push_back(pNewPreview);
        pNewText->Show();
        pNewText->SetPosPixel(Point(6, nMove));
        pNewLB->Show();
        pNewLB->SetPosPixel(Point(nControlWidth, nMove));
        pNewPreview->Show();
        pNewPreview->SetPosPixel(Point(2 * nControlWidth + 6, nMove));
    }
    m_aVScroll->SetRange(Range(0, rHeaders.size()));
    m_aVScroll->SetPageSize((aOutputSize.Height() - nHBHeight - m_nLBStartTopPos)/ m_nYOffset);
    m_aVScroll->EnableDrag();
    m_aVScroll->SetVisibleSize(m_aVScroll->GetPageSize());
    m_aVScroll->SetScrollHdl(LINK(this, SwAssignFieldsControl, ScrollHdl_Impl));

    m_aVScroll->SetPosPixel(Point(aOutputSize.Width() - m_aVScroll->GetSizePixel().Width(), nHBHeight));
    m_aVScroll->SetSizePixel(Size(m_aVScroll->GetSizePixel().Width(), aOutputSize.Height() - nHBHeight));
}

SwAssignFieldsControl::~SwAssignFieldsControl()
{
    disposeOnce();
}

void SwAssignFieldsControl::dispose()
{
    for(auto& rFIItem : m_aFieldNames)
        rFIItem.disposeAndClear();
    for(auto& rLBItem : m_aMatches)
        rLBItem.disposeAndClear();
    for(auto& rFIItem : m_aPreviews)
        rFIItem.disposeAndClear();

    m_aFieldNames.clear();
    m_aMatches.clear();
    m_aPreviews.clear();

    m_aVScroll.disposeAndClear();
    m_aHeaderHB.disposeAndClear();
    m_aWindow.disposeAndClear();
    Control::dispose();
}

void SwAssignFieldsControl::Resize()
{
    Window::Resize();

    Size aOutputSize = GetOutputSize();
    long nHBHeight = m_aHeaderHB->CalcWindowSizePixel().Height();

    m_aWindow->SetSizePixel(Size(aOutputSize.Width() - m_aVScroll->GetSizePixel().Width(), aOutputSize.Height() - nHBHeight));

    m_aVScroll->SetPosPixel(Point(aOutputSize.Width() - m_aVScroll->GetSizePixel().Width(), nHBHeight));
    m_aVScroll->SetSizePixel(Size(m_aVScroll->GetSizePixel().Width(), aOutputSize.Height() - nHBHeight));
    if(m_nYOffset)
        m_aVScroll->SetPageSize((aOutputSize.Height() - nHBHeight - m_nLBStartTopPos)/ m_nYOffset);
    m_aVScroll->SetVisibleSize(m_aVScroll->GetPageSize());
    m_aVScroll->DoScroll(0);

    sal_Int32 nColWidth = aOutputSize.Width() / 3;
    m_aHeaderHB->SetSizePixel(Size(aOutputSize.Width(), nHBHeight));
    m_aHeaderHB->SetItemSize(1, nColWidth);
    m_aHeaderHB->SetItemSize(2, nColWidth);
    m_aHeaderHB->SetItemSize(3, nColWidth);

    if (m_aFieldNames.empty() || m_aMatches.empty())
        return;

    long nControlHeight = std::max(m_aFieldNames[0]->get_preferred_size().Height(),
                                   m_aMatches[0]->get_preferred_size().Height());

    for(auto& rFIItem : m_aFieldNames)
        rFIItem->SetSizePixel(Size(nColWidth - 6, nControlHeight));
    for(auto& rLBItem : m_aMatches)
    {
        long nPosY = rLBItem->GetPosPixel().Y();
        rLBItem->SetPosSizePixel(Point(nColWidth, nPosY), Size(nColWidth - 6, nControlHeight));
    }
    for(auto& rFIItem : m_aPreviews)
    {
        long nPosY = rFIItem->GetPosPixel().Y();
        rFIItem->SetPosSizePixel(Point(2 * nColWidth + 6, nPosY), Size(nColWidth, nControlHeight));
    }
}

void SwAssignFieldsControl::Command( const CommandEvent& rCEvt )
{
    switch ( rCEvt.GetCommand() )
    {
        case CommandEventId::Wheel:
        case CommandEventId::StartAutoScroll:
        case CommandEventId::AutoScroll:
        {
            const CommandWheelData* pWheelData = rCEvt.GetWheelData();
            if(pWheelData && !pWheelData->IsHorz() && CommandWheelMode::ZOOM != pWheelData->GetMode())
            {
                HandleScrollCommand( rCEvt, nullptr, m_aVScroll.get() );
            }
        }
        break;
        default:
            Control::Command(rCEvt);
    }
}

bool SwAssignFieldsControl::PreNotify( NotifyEvent& rNEvt )
{
    if(rNEvt.GetType() == MouseNotifyEvent::COMMAND)
    {
        const CommandEvent* pCEvt = rNEvt.GetCommandEvent();
        if( pCEvt->GetCommand() == CommandEventId::Wheel )
        {
            Command(*pCEvt);
            return true;
        }
    }
    return Control::PreNotify(rNEvt);
}

void SwAssignFieldsControl::MakeVisible( sal_Int32 nIndex )
{
    long nThumb = m_aVScroll->GetThumbPos();
    long nPage = m_aVScroll->GetPageSize();
    if(nThumb > nIndex)
        m_aVScroll->SetThumbPos( nIndex );
    else if( (nThumb + nPage) < nIndex)
        m_aVScroll->SetThumbPos( nIndex - nPage );
    else
        return;
    ScrollHdl_Impl( m_aVScroll.get() );
}

IMPL_LINK(SwAssignFieldsControl, ScrollHdl_Impl, ScrollBar*, pScroll, void)
{
    long nThumb = pScroll->GetThumbPos();
    // the scrollbar moves on a per line basis
    // the height of a line is stored in m_nYOffset
    // nThumb determines which line has to be set at the top (m_nYOffset)
    // The first line has to be -(nThumb * m_nYOffset) in the negative
    long nMove = m_nFirstYPos - (*m_aMatches.begin())->GetPosPixel().Y() - (nThumb * m_nYOffset);

    SetUpdateMode(false);
    for(auto& rFIItem : m_aFieldNames)
        lcl_Move(rFIItem, nMove);
    for(auto& rLBItem : m_aMatches)
        lcl_Move(rLBItem, nMove);
    for(auto& rFIItem : m_aPreviews)
        lcl_Move(rFIItem, nMove);
    SetUpdateMode(true);
}

IMPL_LINK(SwAssignFieldsControl, MatchHdl_Impl, ListBox&, rBox, void)
{
    const OUString sColumn = rBox.GetSelectedEntry();
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
    auto aLBIter = std::find(m_aMatches.begin(), m_aMatches.end(), &rBox);
    if(aLBIter != m_aMatches.end())
    {
        auto nIndex = static_cast<sal_Int32>(std::distance(m_aMatches.begin(), aLBIter));
        m_aPreviews[nIndex]->SetText(sPreview);
    }
    m_aModifyHdl.Call(nullptr);
}

IMPL_LINK(SwAssignFieldsControl, GotFocusHdl_Impl, Control&, rControl, void)
{
    ListBox* pBox = static_cast<ListBox*>(&rControl);
    if(GetFocusFlags::Tab & pBox->GetGetFocusFlags())
    {
        auto aLBIter = std::find(m_aMatches.begin(), m_aMatches.end(), pBox);
        if(aLBIter != m_aMatches.end())
        {
            auto nIndex = static_cast<sal_Int32>(std::distance(m_aMatches.begin(), aLBIter));
            MakeVisible(nIndex);
        }
    }
}

SwAssignFieldsDialog::SwAssignFieldsDialog(
        vcl::Window* pParent, SwMailMergeConfigItem& rConfigItem,
        const OUString& rPreview,
        bool bIsAddressBlock) :
    SfxModalDialog(pParent, "AssignFieldsDialog", "modules/swriter/ui/assignfieldsdialog.ui"),
    m_sNone(SwResId(SW_STR_NONE)),
    m_rPreviewString(rPreview),
    m_rConfigItem(rConfigItem)
{
    get(m_pMatchingFI, "MATCHING_LABEL");
    get(m_pPreviewFI, "PREVIEW_LABEL");
    get(m_pOK, "ok");
    get(m_pPreviewWIN, "PREVIEW");
    Size aSize(LogicToPixel(Size(248, 45), MapMode(MapUnit::MapAppFont)));
    m_pPreviewWIN->set_width_request(aSize.Width());
    m_pPreviewWIN->set_height_request(aSize.Height());
    get(m_pFieldsControl, "FIELDS");
    m_pFieldsControl->Init(rConfigItem);
    //resize the HeaderBar
    OUString sAddressElement( SwResId(ST_ADDRESSELEMENT) );
    const OUString sMatchesTo( SwResId(ST_MATCHESTO) );
    const OUString sPreview( SwResId(ST_PREVIEW) );
    if(!bIsAddressBlock)
    {
        m_pPreviewFI->SetText(SwResId(ST_SALUTATIONPREVIEW));
        m_pMatchingFI->SetText(SwResId(ST_SALUTATIONMATCHING));
        sAddressElement = SwResId(ST_SALUTATIONELEMENT);
    }

    Size aOutputSize(m_pFieldsControl->m_aHeaderHB->GetSizePixel());
    sal_Int32 nFirstWidth;
    sal_Int32 nSecondWidth = nFirstWidth = aOutputSize.Width() / 3;
    const HeaderBarItemBits nHeadBits = HeaderBarItemBits::FIXED| HeaderBarItemBits::FIXEDPOS;
    m_pFieldsControl->m_aHeaderHB->InsertItem( 1, sAddressElement, nFirstWidth, nHeadBits|HeaderBarItemBits::LEFT);
    m_pFieldsControl->m_aHeaderHB->InsertItem( 2, sMatchesTo,      nSecondWidth, nHeadBits|HeaderBarItemBits::LEFT);
    m_pFieldsControl->m_aHeaderHB->InsertItem( 3, sPreview,
            aOutputSize.Width() - nFirstWidth - nSecondWidth, nHeadBits|HeaderBarItemBits::LEFT);

    m_pFieldsControl->SetModifyHdl(LINK(this, SwAssignFieldsDialog, AssignmentModifyHdl_Impl ));

    m_pMatchingFI->SetText(m_pMatchingFI->GetText().replaceAll("%1", sMatchesTo));

    m_pOK->SetClickHdl(LINK(this, SwAssignFieldsDialog, OkHdl_Impl));
}

SwAssignFieldsDialog::~SwAssignFieldsDialog()
{
    disposeOnce();
}

void SwAssignFieldsDialog::dispose()
{
    m_pMatchingFI.clear();
    m_pFieldsControl.clear();
    m_pPreviewFI.clear();
    m_pPreviewWIN.clear();
    m_pOK.clear();
    SfxModalDialog::dispose();
}

uno::Sequence< OUString > SwAssignFieldsDialog::CreateAssignments()
{
    uno::Sequence< OUString > aAssignments(
            m_rConfigItem.GetDefaultAddressHeaders().size());
    OUString* pAssignments = aAssignments.getArray();
    sal_Int32 nIndex = 0;
    for(const auto& rLBItem : m_pFieldsControl->m_aMatches)
    {
        const OUString sSelect = rLBItem->GetSelectedEntry();
        pAssignments[nIndex] = (m_sNone != sSelect) ? sSelect : OUString();
        ++nIndex;
    }
    return aAssignments;
}

IMPL_LINK_NOARG(SwAssignFieldsDialog, OkHdl_Impl, Button*, void)
{
    m_rConfigItem.SetColumnAssignment(
                            m_rConfigItem.GetCurrentDBData(),
                            CreateAssignments() );
    EndDialog(RET_OK);
}

IMPL_LINK_NOARG(SwAssignFieldsDialog, AssignmentModifyHdl_Impl, LinkParamNone*, void)
{
    uno::Sequence< OUString > aAssignments = CreateAssignments();
    const OUString sPreview = SwAddressPreview::FillData(
            m_rPreviewString, m_rConfigItem, &aAssignments);
    m_pPreviewWIN->SetAddress(sPreview);
}

DDListBox::DDListBox(vcl::Window* pParent, WinBits nStyle)
    : SvTreeListBox(pParent, nStyle)
    , m_pParentDialog(nullptr)
{
    SetStyle( GetStyle() | /*WB_HASBUTTONS|WB_HASBUTTONSATROOT|*/
                            WB_CLIPCHILDREN );
    SetSelectionMode( SelectionMode::Single );
    SetDragDropMode( DragDropMode::CTRL_COPY );
    EnableAsyncDrag(true);
    // expand selection to the complete width of the ListBox
    SetHighlightRange();
    Show();

}

DDListBox::~DDListBox()
{
    disposeOnce();
}

void DDListBox::dispose()
{
    m_pParentDialog.clear();
    SvTreeListBox::dispose();
}

VCL_BUILDER_FACTORY_CONSTRUCTOR(DDListBox, WB_TABSTOP)

void DDListBox::SetAddressDialog(SwCustomizeAddressBlockDialog *pParent)
{
    m_pParentDialog = pParent;
}

void  DDListBox::StartDrag( sal_Int8 /*nAction*/, const Point& /*rPosPixel*/ )
{
    SvTreeListEntry* pEntry = GetCurEntry();
    if(pEntry)
    {
        ReleaseMouse();

        rtl::Reference<TransferDataContainer> pContainer = new TransferDataContainer;

        sal_Int32 nUserData = static_cast<sal_Int32>(reinterpret_cast<sal_IntPtr>(pEntry->GetUserData()));
        //special entries can only be once in the address / greeting
        if(nUserData >= 0 || !m_pParentDialog->HasItem_Impl(nUserData))
        {
            pContainer->CopyString( "<" + GetEntryText(pEntry) + ">" );
            pContainer->StartDrag( this, DND_ACTION_COPY, GetDragFinishedHdl() );
        }
    }
}

AddressMultiLineEdit::AddressMultiLineEdit(vcl::Window* pParent, WinBits nBits)
    : VclMultiLineEdit(pParent, nBits)
    , m_pParentDialog(nullptr)
{
    GetTextView()->SupportProtectAttribute(true);
    StartListening(*GetTextEngine());
    EnableFocusSelectionHide(false);
}

AddressMultiLineEdit::~AddressMultiLineEdit()
{
    disposeOnce();
}

void AddressMultiLineEdit::dispose()
{
    EndListening(*GetTextEngine());
    m_pParentDialog.clear();
    VclMultiLineEdit::dispose();
}


Size AddressMultiLineEdit::GetOptimalSize() const
{
    return LogicToPixel(Size(160, 60), MapMode(MapUnit::MapAppFont));
}


VCL_BUILDER_FACTORY_CONSTRUCTOR(AddressMultiLineEdit, WB_LEFT|WB_TABSTOP)

void AddressMultiLineEdit::SetAddressDialog(SwCustomizeAddressBlockDialog *pParent)
{
    m_pParentDialog = pParent;
}

void AddressMultiLineEdit::Notify(SfxBroadcaster& /*rBC*/, const SfxHint& rHint)
{
    if (m_aSelectionLink.IsSet() && dynamic_cast<const TextHint*>(&rHint))
    {
        const TextHint& rTextHint = static_cast<const TextHint&>(rHint);
        if (rTextHint.GetId() == SfxHintId::TextViewSelectionChanged ||
            rTextHint.GetId() == SfxHintId::TextViewCaretChanged)
        {
            m_aSelectionLink.Call(*this);
        }
    }
}

bool AddressMultiLineEdit::PreNotify( NotifyEvent& rNEvt )
{
    bool bHandled = false;
    if( MouseNotifyEvent::KEYINPUT == rNEvt.GetType()  &&
        rNEvt.GetKeyEvent()->GetCharCode())
    {
        bHandled = true;
    }
    else if(MouseNotifyEvent::MOUSEBUTTONDOWN == rNEvt.GetType()) {
        const MouseEvent *pMEvt = rNEvt.GetMouseEvent();
        if(pMEvt->GetClicks() >= 2)
            bHandled = true;
    }
    if(!bHandled)
        bHandled = VclMultiLineEdit::PreNotify( rNEvt );
    return bHandled;

}

void AddressMultiLineEdit::SetText( const OUString& rStr )
{
    VclMultiLineEdit::SetText(rStr);
    //set attributes to all address tokens

    ExtTextEngine* pTextEngine = GetTextEngine();
    TextAttribProtect aProtectAttr;
    const sal_uInt32 nParaCount = pTextEngine->GetParagraphCount();
    for(sal_uInt32 nPara = 0; nPara < nParaCount; ++nPara)
    {
        sal_Int32 nIndex = 0;
        const OUString sPara = pTextEngine->GetText( nPara );
        if(!sPara.isEmpty() && !sPara.endsWith(" "))
        {
            TextPaM aPaM(nPara, sPara.getLength());
            pTextEngine->ReplaceText(TextSelection( aPaM ), " ");
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
            pTextEngine->SetAttrib( aProtectAttr, nPara, nStart, nEnd + 1, false );
        }

    }
    // add two empty paragraphs at the end
    if(m_pParentDialog->m_eType == SwCustomizeAddressBlockDialog::ADDRESSBLOCK_NEW ||
            m_pParentDialog->m_eType == SwCustomizeAddressBlockDialog::ADDRESSBLOCK_EDIT)
    {
        sal_Int32 nLastLen = pTextEngine->GetText(nParaCount - 1).getLength();
        if(nLastLen)
        {
            TextPaM aPaM(nParaCount ? nParaCount - 1 : 0, nLastLen);
            pTextEngine->ReplaceText(TextSelection( aPaM ), "\n \n ");
        }
    }
}

// Insert the new entry in front of the entry at the beginning of the selection
void AddressMultiLineEdit::InsertNewEntry( const OUString& rStr )
{
    // insert new entry after current selected one.
    TextView* pTextView = GetTextView();
    const TextSelection& rSelection = pTextView->GetSelection();
    const sal_uInt32 nPara = rSelection.GetStart().GetPara();
    sal_Int32 nIndex = rSelection.GetEnd().GetIndex();
    ExtTextEngine *pTextEngine = GetTextEngine();
    const TextCharAttrib *pAttrib;
    if(nullptr != (pAttrib = pTextEngine->FindCharAttrib( rSelection.GetStart(), TEXTATTR_PROTECTED )))
        nIndex = pAttrib->GetEnd();
    InsertNewEntryAtPosition( rStr, nPara, nIndex );

    // select the new entry
    pAttrib = pTextEngine->FindCharAttrib(TextPaM(nPara, nIndex),TEXTATTR_PROTECTED);
    const sal_Int32 nEnd = pAttrib ? pAttrib->GetEnd() : nIndex;
    TextSelection aEntrySel(TextPaM(nPara, nIndex), TextPaM(nPara, nEnd));
    pTextView->SetSelection(aEntrySel);
    Invalidate();
    Modify();
}

void AddressMultiLineEdit::InsertNewEntryAtPosition( const OUString& rStr, sal_uLong nPara, sal_uInt16 nIndex )
{
    ExtTextEngine* pTextEngine = GetTextEngine();
    TextPaM aInsertPos( nPara, nIndex );

    pTextEngine->ReplaceText( aInsertPos, rStr );

    //restore the attributes
    SetText( GetAddress() );
    //select the newly inserted/moved element
    TextSelection aEntrySel(aInsertPos);
    TextView* pTextView = GetTextView();
    pTextView->SetSelection(aEntrySel);
    m_aSelectionLink.Call(*this);
}

void AddressMultiLineEdit::RemoveCurrentEntry()
{
    ExtTextEngine* pTextEngine = GetTextEngine();
    TextView* pTextView = GetTextView();
    const TextSelection& rSelection = pTextView->GetSelection();
    const TextCharAttrib* pBeginAttrib = pTextEngine->FindCharAttrib( rSelection.GetStart(), TEXTATTR_PROTECTED );
    if(pBeginAttrib &&
            (pBeginAttrib->GetStart() <= rSelection.GetStart().GetIndex()
                            && pBeginAttrib->GetEnd() >= rSelection.GetEnd().GetIndex()))
    {
        const sal_uInt32 nPara = rSelection.GetStart().GetPara();
        TextSelection aEntrySel(TextPaM( nPara, pBeginAttrib->GetStart()), TextPaM(nPara, pBeginAttrib->GetEnd()));
        pTextEngine->ReplaceText(aEntrySel, OUString());
        //restore the attributes
        SetText( GetAddress() );
        Modify();
    }
}

void AddressMultiLineEdit::MoveCurrentItem(MoveItemFlags nMove)
{
    ExtTextEngine* pTextEngine = GetTextEngine();
    TextView* pTextView = GetTextView();
    const TextSelection& rSelection = pTextView->GetSelection();
    const TextCharAttrib* pBeginAttrib = pTextEngine->FindCharAttrib( rSelection.GetStart(), TEXTATTR_PROTECTED );
    if(!pBeginAttrib ||
       !(pBeginAttrib->GetStart() <= rSelection.GetStart().GetIndex() &&
         pBeginAttrib->GetEnd() >= rSelection.GetEnd().GetIndex()))
        return;

    //current item has been found
    sal_uInt32 nPara = rSelection.GetStart().GetPara();
    sal_Int32 nIndex = pBeginAttrib->GetStart();
    TextSelection aEntrySel(TextPaM( nPara, pBeginAttrib->GetStart()), TextPaM(nPara, pBeginAttrib->GetEnd()));
    const OUString sCurrentItem = pTextEngine->GetText(aEntrySel);
    pTextEngine->RemoveAttrib( nPara, *pBeginAttrib );
    pTextEngine->ReplaceText(aEntrySel, OUString());
    switch(nMove)
    {
        case MoveItemFlags::Left :
            if(nIndex)
            {
                //go left to find a predecessor or simple text
                --nIndex;
                const OUString sPara = pTextEngine->GetText( nPara );
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
            const TextCharAttrib* pEndAttrib = pTextEngine->FindCharAttrib( rSelection.GetStart(), TEXTATTR_PROTECTED );
            if(pEndAttrib && pEndAttrib->GetEnd() >= nIndex)
            {
                nIndex = pEndAttrib->GetEnd();
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
    if(nPara >= pTextEngine->GetParagraphCount())
    {

        TextPaM aTemp(nPara - 1, pTextEngine->GetTextLen( nPara - 1 ));
        pTextEngine->ReplaceText(aTemp, "\n");
    }
    InsertNewEntryAtPosition( sCurrentItem, nPara, nIndex );

    // select the new entry [#i40817]
    const TextCharAttrib *pAttrib = pTextEngine->FindCharAttrib(TextPaM(nPara, nIndex),TEXTATTR_PROTECTED);
    if (pAttrib)
        aEntrySel = TextSelection(TextPaM(nPara, nIndex), TextPaM(nPara, pAttrib->GetEnd()));
    pTextView->SetSelection(aEntrySel);
    Invalidate();
    Modify();

}

MoveItemFlags  AddressMultiLineEdit::IsCurrentItemMoveable()
{
    MoveItemFlags nRet = MoveItemFlags::NONE;
    ExtTextEngine* pTextEngine = GetTextEngine();
    TextView* pTextView = GetTextView();
    const TextSelection& rSelection = pTextView->GetSelection();
    const TextCharAttrib* pBeginAttrib = pTextEngine->FindCharAttrib( rSelection.GetStart(), TEXTATTR_PROTECTED );
    if(pBeginAttrib &&
            (pBeginAttrib->GetStart() <= rSelection.GetStart().GetIndex()
                            && pBeginAttrib->GetEnd() >= rSelection.GetEnd().GetIndex()))
    {
        if(pBeginAttrib->GetStart())
            nRet |= MoveItemFlags::Left;
        //if there is an entry it can always be move to the right and down
        nRet |= MoveItemFlags::Right | MoveItemFlags::Down;
        if(rSelection.GetStart().GetPara() > 0)
            nRet |= MoveItemFlags::Up;
    }
    return nRet;
}

bool AddressMultiLineEdit::HasCurrentItem()
{
    ExtTextEngine* pTextEngine = GetTextEngine();
    TextView* pTextView = GetTextView();
    const TextSelection& rSelection = pTextView->GetSelection();
    const TextCharAttrib* pBeginAttrib = pTextEngine->FindCharAttrib( rSelection.GetStart(), TEXTATTR_PROTECTED );
    return (pBeginAttrib &&
            (pBeginAttrib->GetStart() <= rSelection.GetStart().GetIndex()
                            && pBeginAttrib->GetEnd() >= rSelection.GetEnd().GetIndex()));
}

OUString AddressMultiLineEdit::GetCurrentItem()
{
    ExtTextEngine* pTextEngine = GetTextEngine();
    TextView* pTextView = GetTextView();
    const TextSelection& rSelection = pTextView->GetSelection();
    const TextCharAttrib* pBeginAttrib = pTextEngine->FindCharAttrib( rSelection.GetStart(), TEXTATTR_PROTECTED );
    if(pBeginAttrib &&
            (pBeginAttrib->GetStart() <= rSelection.GetStart().GetIndex()
                            && pBeginAttrib->GetEnd() >= rSelection.GetEnd().GetIndex()))
    {
        const sal_uInt32 nPara = rSelection.GetStart().GetPara();
        TextSelection aEntrySel(TextPaM( nPara, pBeginAttrib->GetStart()), TextPaM(nPara, pBeginAttrib->GetEnd()));
        return pTextEngine->GetText( aEntrySel );
    }
    return OUString();
}

void AddressMultiLineEdit::SelectCurrentItem()
{
    ExtTextEngine* pTextEngine = GetTextEngine();
    TextView* pTextView = GetTextView();
    const TextSelection& rSelection = pTextView->GetSelection();
    const TextCharAttrib* pBeginAttrib = pTextEngine->FindCharAttrib( rSelection.GetStart(), TEXTATTR_PROTECTED );
    if(pBeginAttrib &&
            (pBeginAttrib->GetStart() <= rSelection.GetStart().GetIndex()
                            && pBeginAttrib->GetEnd() >= rSelection.GetEnd().GetIndex()))
    {
        const sal_uInt32 nPara = rSelection.GetStart().GetPara();
        TextSelection aEntrySel(TextPaM( nPara, pBeginAttrib->GetStart()), TextPaM(nPara, pBeginAttrib->GetEnd()));
        pTextView->SetSelection(aEntrySel);
        Invalidate();
    }
}

OUString AddressMultiLineEdit::GetAddress()
{
    OUString sRet;
    ExtTextEngine* pTextEngine = GetTextEngine();
    const sal_uInt32 nParaCount = pTextEngine->GetParagraphCount();
    for(sal_uInt32 nPara = nParaCount; nPara; --nPara)
    {
        const OUString sPara = comphelper::string::stripEnd(pTextEngine->GetText(nPara - 1), ' ');
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
