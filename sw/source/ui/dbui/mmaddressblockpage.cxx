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

#include <mmaddressblockpage.hxx>
#include <mailmergewizard.hxx>
#include <swtypes.hxx>
#include <addresslistdialog.hxx>
#include <vcl/layout.hxx>
#include <vcl/txtattr.hxx>
#include <vcl/xtextedt.hxx>
#include <mmconfigitem.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdb/XColumn.hpp>
#include <comphelper/string.hxx>
#include "svtools/treelistentry.hxx"

#include <vector>
#include <boost/scoped_ptr.hpp>
#include <mmaddressblockpage.hrc>
#include <globals.hrc>
#include <dbui.hrc>
#include <helpid.h>

using namespace svt;
using namespace ::com::sun::star;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;

static void lcl_Move(Control* pCtrl, long nYOffset)
{
    Point aPos(pCtrl->GetPosPixel());
    aPos.Y() += nYOffset;
    pCtrl->SetPosPixel(aPos);
}

SwMailMergeAddressBlockPage::SwMailMergeAddressBlockPage( SwMailMergeWizard* _pParent) :
    svt::OWizardPage(_pParent, SW_RES(DLG_MM_ADDRESSBLOCK_PAGE)),
    m_aHeaderFI(        this, SW_RES(  FI_HEADER           ) ),
    m_aFirstFI(         this, SW_RES( FI_FIRST ) ),
    m_aAddressListFI(   this, SW_RES( FI_ADDRESSLIST ) ),
    m_aAddressListPB(   this, SW_RES( PB_ADDRESSLIST ) ),
    m_aCurrentAddressFI( this, SW_RES( FI_CURRENTADDRESS ) ),
    m_aFirstFL(         this, SW_RES( FL_FIRST ) ),
    m_aSecondFI(        this, SW_RES( FI_SECOND )),
    m_aSettingsFI(      this, SW_RES( FI_SECOND    ) ),
    m_aAddressCB(       this, SW_RES( CB_ADDRESS   ) ),
    m_aSettingsWIN(     this, SW_RES( WIN_SETTINGS   ) ),
    m_aSettingsPB(      this, SW_RES( PB_SETTINGS    ) ),
    m_aHideEmptyParagraphsCB( this, SW_RES( CB_HIDE_EMPTY_PARA ) ),
    m_aSecondFL(        this, SW_RES( FL_SECOND )),
    m_aThirdFI(         this, SW_RES( FI_THIRD ) ),
    m_aMatchFieldsFI(   this, SW_RES( FI_MATCH_FIELDS ) ),
    m_aAssignPB(        this, SW_RES( PB_ASSIGN      ) ),
    m_aThirdFL(         this, SW_RES( FL_THIRD ) ),
    m_aFourthFI(        this, SW_RES( FI_FOURTH ) ),
    m_aPreviewFI(       this, SW_RES( FI_PREVIEW     ) ),
    m_aPreviewWIN(      this, SW_RES( WIN_PREVIEW    ) ),
    m_aDocumentIndexFI( this, SW_RES( FI_DOCINDEX    ) ),
    m_aPrevSetIB(       this, SW_RES( IB_PREVSET     ) ),
    m_aNextSetIB(       this, SW_RES( IB_NEXTSET     ) ),
    m_sDocument(        SW_RES(       STR_DOCUMENT  ) ),
    m_sChangeAddress(   SW_RES(      STR_CHANGEADDRESS )),
    m_pWizard(_pParent)
{
    FreeResource();
    m_sCurrentAddress = m_aCurrentAddressFI.GetText();
    m_aAddressListPB.SetClickHdl(LINK(this, SwMailMergeAddressBlockPage, AddressListHdl_Impl));
    m_aSettingsPB.SetClickHdl(LINK(this, SwMailMergeAddressBlockPage, SettingsHdl_Impl));
    m_aAssignPB.SetClickHdl(LINK(this, SwMailMergeAddressBlockPage, AssignHdl_Impl ));
    m_aAddressCB.SetClickHdl(LINK(this, SwMailMergeAddressBlockPage, AddressBlockHdl_Impl));
    m_aSettingsWIN.SetSelectHdl(LINK(this, SwMailMergeAddressBlockPage, AddressBlockSelectHdl_Impl));
    m_aHideEmptyParagraphsCB.SetClickHdl(LINK(this, SwMailMergeAddressBlockPage, HideParagraphsHdl_Impl));

    Link aLink = LINK(this, SwMailMergeAddressBlockPage, InsertDataHdl_Impl);
    m_aPrevSetIB.SetClickHdl(aLink);
    m_aNextSetIB.SetClickHdl(aLink);
}

SwMailMergeAddressBlockPage::~SwMailMergeAddressBlockPage()
{
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
    m_aSettingsFI.Show( bIsLetter );
    m_aAddressCB.Show( bIsLetter );
    m_aSettingsWIN.Show( bIsLetter );
    m_aSettingsPB.Show( bIsLetter );
    m_aPreviewFI.Show( bIsLetter );
    m_aPreviewWIN.Show( bIsLetter );
    m_aAssignPB.Show( bIsLetter );
    m_aDocumentIndexFI.Show( bIsLetter );
    m_aPrevSetIB.Show( bIsLetter );
    m_aNextSetIB.Show( bIsLetter );
    m_aHideEmptyParagraphsCB.Show( bIsLetter );
    m_aSecondFL.Show( bIsLetter );
    m_aSecondFI.Show( bIsLetter );
    m_aSettingsFI.Show( bIsLetter );
    m_aMatchFieldsFI.Show( bIsLetter );
    m_aThirdFI.Show( bIsLetter );
    m_aThirdFL.Show( bIsLetter );
    m_aFourthFI.Show( bIsLetter );

    if(bIsLetter)
    {
        m_aHideEmptyParagraphsCB.Check( rConfigItem.IsHideEmptyParagraphs() );
        m_aDocumentIndexFI.SetText(m_sDocument.replaceFirst("%1", OUString::number(1)));

        m_aSettingsWIN.Clear();
        const uno::Sequence< OUString> aBlocks =
                    m_pWizard->GetConfigItem().GetAddressBlocks();
        for(sal_Int32 nAddress = 0; nAddress < aBlocks.getLength(); ++nAddress)
            m_aSettingsWIN.AddAddress(aBlocks[nAddress]);
        m_aSettingsWIN.SelectAddress((sal_uInt16)rConfigItem.GetCurrentAddressBlockIndex());
        m_aAddressCB.Check(rConfigItem.IsAddressBlock());
        AddressBlockHdl_Impl(&m_aAddressCB);
        m_aSettingsWIN.SetLayout(1, 2);
        InsertDataHdl_Impl(0);
    }
}

sal_Bool    SwMailMergeAddressBlockPage::commitPage( ::svt::WizardTypes::CommitPageReason _eReason )
{
    if ( ::svt::WizardTypes::eTravelForward == _eReason && !m_pWizard->GetConfigItem().GetResultSet().is() )
        return sal_False;
    return sal_True;
}

IMPL_LINK_NOARG(SwMailMergeAddressBlockPage, AddressListHdl_Impl)
{
    try
    {
        boost::scoped_ptr<SwAddressListDialog> xAddrDialog(new SwAddressListDialog(this));
        if(RET_OK == xAddrDialog->Execute())
        {
            SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
            rConfigItem.SetCurrentConnection(
                            xAddrDialog->GetSource(),
                            xAddrDialog->GetConnection(),
                            xAddrDialog->GetColumnsSupplier(),
                            xAddrDialog->GetDBData());
            OUString sFilter = xAddrDialog->GetFilter();
            rConfigItem.SetFilter( sFilter );
            InsertDataHdl_Impl(0);
            GetWizard()->UpdateRoadmap();
            GetWizard()->enableButtons(WZB_NEXT, GetWizard()->isStateEnabled(MM_GREETINGSPAGE));
        }
    }
    catch (const uno::Exception& e)
    {
        OSL_FAIL(OUStringToOString(e.Message, osl_getThreadTextEncoding()).getStr());
        MessageDialog(this, e.Message).Execute();
    }
    return 0;
}

IMPL_LINK(SwMailMergeAddressBlockPage, SettingsHdl_Impl, PushButton*, pButton)
{
    SwSelectAddressBlockDialog* pDlg =
                new SwSelectAddressBlockDialog(pButton, m_pWizard->GetConfigItem());
    SwMailMergeConfigItem& rConfig = m_pWizard->GetConfigItem();
    pDlg->SetAddressBlocks(rConfig.GetAddressBlocks(), m_aSettingsWIN.GetSelectedAddress());
    pDlg->SetSettings(rConfig.IsIncludeCountry(), rConfig.GetExcludeCountry());
    if(RET_OK == pDlg->Execute())
    {
        //the dialog provides the selected address at the first position!
        const uno::Sequence< OUString> aBlocks =
                    pDlg->GetAddressBlocks();
        rConfig.SetAddressBlocks(aBlocks);
        m_aSettingsWIN.Clear();
        for(sal_Int32 nAddress = 0; nAddress < aBlocks.getLength(); ++nAddress)
            m_aSettingsWIN.AddAddress(aBlocks[nAddress]);
        m_aSettingsWIN.SelectAddress(0);
        m_aSettingsWIN.Invalidate();    // #i40408
        rConfig.SetCountrySettings(pDlg->IsIncludeCountry(), pDlg->GetCountry());
        InsertDataHdl_Impl(0);
    }
    delete pDlg;
    GetWizard()->UpdateRoadmap();
    GetWizard()->enableButtons(WZB_NEXT, GetWizard()->isStateEnabled(MM_GREETINGSPAGE));
    return 0;
}

IMPL_LINK(SwMailMergeAddressBlockPage, AssignHdl_Impl, PushButton*, pButton)
{
    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
    sal_uInt16 nSel = m_aSettingsWIN.GetSelectedAddress();
    const uno::Sequence< OUString> aBlocks = rConfigItem.GetAddressBlocks();
    SwAssignFieldsDialog* pDlg =
            new SwAssignFieldsDialog(pButton, m_pWizard->GetConfigItem(), aBlocks[nSel], true);
    if(RET_OK == pDlg->Execute())
    {
        //preview update
        InsertDataHdl_Impl(0);
        GetWizard()->UpdateRoadmap();
        GetWizard()->enableButtons(WZB_NEXT, GetWizard()->isStateEnabled(MM_GREETINGSPAGE));
    }
    delete pDlg;
    return 0;
}

void SwMailMergeAddressBlockPage::EnableAddressBlock(sal_Bool bAll, sal_Bool bSelective)
{
    m_aSettingsFI.Enable(bAll);
    m_aAddressCB.Enable(bAll);
    bSelective &= bAll;
    m_aHideEmptyParagraphsCB.Enable(bSelective);
    m_aSettingsWIN.Enable(bSelective);
    m_aSettingsPB.Enable(bSelective);
    m_aPreviewFI.Enable(bSelective);
    m_aPreviewWIN.Enable(bSelective);
    m_aThirdFI.Enable(bSelective);
    m_aMatchFieldsFI.Enable(bSelective);
    m_aAssignPB.Enable(bSelective);
    m_aDocumentIndexFI.Enable(bSelective);
    m_aPrevSetIB.Enable(bSelective);
    m_aNextSetIB.Enable(bSelective);
}

IMPL_LINK(SwMailMergeAddressBlockPage, AddressBlockHdl_Impl, CheckBox*, pBox)
{
    EnableAddressBlock(pBox->IsEnabled(), pBox->IsChecked());
    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
    rConfigItem.SetAddressBlock(m_aAddressCB.IsChecked());
    m_pWizard->UpdateRoadmap();
    return 0;
}

IMPL_LINK_NOARG(SwMailMergeAddressBlockPage, AddressBlockSelectHdl_Impl)
{
    sal_uInt16 nSel = m_aSettingsWIN.GetSelectedAddress();
    const uno::Sequence< OUString> aBlocks =
                m_pWizard->GetConfigItem().GetAddressBlocks();
    OUString sPreview = SwAddressPreview::FillData(aBlocks[nSel], m_pWizard->GetConfigItem());
    m_aPreviewWIN.SetAddress(sPreview);
    m_pWizard->GetConfigItem().SetCurrentAddressBlockIndex( nSel );
    GetWizard()->UpdateRoadmap();
    GetWizard()->enableButtons(WZB_NEXT, GetWizard()->isStateEnabled(MM_GREETINGSPAGE));
    return 0;
}

IMPL_LINK(SwMailMergeAddressBlockPage, HideParagraphsHdl_Impl, CheckBox*, pBox)
{
    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
    rConfigItem.SetHideEmptyParagraphs( pBox->IsChecked() );
    return 0;
}

IMPL_LINK(SwMailMergeAddressBlockPage, InsertDataHdl_Impl, ImageButton*, pButton)
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
        bool bNext = pButton == &m_aNextSetIB;
        sal_Int32 nPos = rConfig.GetResultSetPosition();
        rConfig.MoveResultSet( bNext ? ++nPos : --nPos);
    }
    m_pWizard->LeaveWait();
    sal_Int32 nPos = rConfig.GetResultSetPosition();
    sal_Bool bEnable = sal_True;
    if(nPos < 1)
    {
        bEnable = sal_False;
        nPos = 1;
    }
    else
    {
        //if output type is letter
        if(m_aSettingsWIN.IsVisible())
        {
            //Fill data into preview
            sal_uInt16 nSel = m_aSettingsWIN.GetSelectedAddress();
            const uno::Sequence< OUString> aBlocks =
                        m_pWizard->GetConfigItem().GetAddressBlocks();
            OUString sPreview = SwAddressPreview::FillData(aBlocks[nSel], rConfig);
            m_aPreviewWIN.SetAddress(sPreview);
        }
    }
    m_aPrevSetIB.Enable(bEnable);
    m_aDocumentIndexFI.SetText(m_sDocument.replaceFirst("%1", OUString::number(nPos)));

    GetWizard()->enableButtons(WZB_NEXT, GetWizard()->isStateEnabled(MM_GREETINGSPAGE));
    sal_Bool bHasResultSet = rConfig.GetResultSet().is();
    m_aCurrentAddressFI.Show(bHasResultSet);
    if(bHasResultSet)
    {
        m_aCurrentAddressFI.SetText(m_sCurrentAddress.replaceFirst("%1", rConfig.GetCurrentDBData().sDataSource));
        m_aAddressListPB.SetText(m_sChangeAddress);
    }
    EnableAddressBlock(bHasResultSet, m_aAddressCB.IsChecked());
    return 0;
}

SwSelectAddressBlockDialog::SwSelectAddressBlockDialog(
                Window* pParent, SwMailMergeConfigItem& rConfig)
    : SfxModalDialog(pParent, "SelectBlockDialog",
        "modules/swriter/ui/selectblockdialog.ui")
    , m_rConfig(rConfig)
{
    get(m_pPreview, "preview");
    Size aSize(m_pPreview->LogicToPixel(Size(192, 100), MapMode(MAP_APPFONT)));
    m_pPreview->set_width_request(aSize.Width());
    m_pPreview->set_height_request(aSize.Height());
    get(m_pNewPB, "new");
    get(m_pCustomizePB, "edit");
    get(m_pDeletePB, "delete");
    get(m_pNeverRB, "never");
    get(m_pAlwaysRB, "always");
    get(m_pDependentRB, "dependent");
    get(m_pCountryED, "country");

    Link aCustomizeHdl = LINK(this, SwSelectAddressBlockDialog, NewCustomizeHdl_Impl);
    m_pNewPB->SetClickHdl(aCustomizeHdl);
    m_pCustomizePB->SetClickHdl(aCustomizeHdl);

    m_pDeletePB->SetClickHdl(LINK(this, SwSelectAddressBlockDialog, DeleteHdl_Impl));

    Link aLk = LINK(this, SwSelectAddressBlockDialog, IncludeHdl_Impl);
    m_pNeverRB->SetClickHdl(aLk);
    m_pAlwaysRB->SetClickHdl(aLk);
    m_pDependentRB->SetClickHdl(aLk);
    m_pPreview->SetLayout(2, 2);
    m_pPreview->EnableScrollBar();
}

SwSelectAddressBlockDialog::~SwSelectAddressBlockDialog()
{
}

void SwSelectAddressBlockDialog::SetAddressBlocks(const uno::Sequence< OUString>& rBlocks,
        sal_uInt16 nSelectedAddress)
{
    m_aAddressBlocks = rBlocks;
    for(sal_Int32 nAddress = 0; nAddress < m_aAddressBlocks.getLength(); ++nAddress)
        m_pPreview->AddAddress(m_aAddressBlocks[nAddress]);
    m_pPreview->SelectAddress(nSelectedAddress);
}

// return the address blocks and put the selected one to the first position
const uno::Sequence< OUString >&    SwSelectAddressBlockDialog::GetAddressBlocks()
{
    //put the selected block to the first position
    sal_uInt16 nSelect = m_pPreview->GetSelectedAddress();
    if(nSelect)
    {
        uno::Sequence< OUString >aTemp = m_aAddressBlocks;
        OUString* pTemp = aTemp.getArray();
        pTemp[0] = m_aAddressBlocks[nSelect];
        sal_uInt32 nIndex = 0;
        const sal_uInt32 nNumBlocks = m_aAddressBlocks.getLength();
        for(sal_uInt32 nAddress = 1; nAddress < nNumBlocks; ++nAddress)
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
        sal_Bool bIsCountry, OUString rCountry)
{
    RadioButton *pActive = m_pNeverRB;
    if(bIsCountry)
    {
        pActive = !rCountry.isEmpty() ? m_pDependentRB : m_pAlwaysRB;
        m_pCountryED->SetText(rCountry);
    }
    pActive->Check();
    IncludeHdl_Impl(pActive);
    m_pDeletePB->Enable(m_aAddressBlocks.getLength() > 1);
}

OUString     SwSelectAddressBlockDialog::GetCountry() const
{
    OUString sRet;
    if(m_pDependentRB->IsChecked())
        sRet = m_pCountryED->GetText();
    return sRet;
}

IMPL_LINK(SwSelectAddressBlockDialog, DeleteHdl_Impl, PushButton*, pButton)
{
    if(m_aAddressBlocks.getLength())
    {
        sal_uInt16 nSelected = m_pPreview->GetSelectedAddress();
        OUString* pAddressBlocks = m_aAddressBlocks.getArray();
        sal_Int32 nSource = 0;
        for(sal_Int32 nTarget = 0; nTarget < m_aAddressBlocks.getLength() - 1; nTarget++)
        {
            if(nSource == nSelected)
                ++nSource;
            pAddressBlocks[nTarget] = pAddressBlocks[nSource++];
        }
        m_aAddressBlocks.realloc(m_aAddressBlocks.getLength() - 1);
        if(m_aAddressBlocks.getLength() <= 1)
            pButton->Enable(sal_False);
        m_pPreview->RemoveSelectedAddress();
    }
    return 0;
}

IMPL_LINK(SwSelectAddressBlockDialog, NewCustomizeHdl_Impl, PushButton*, pButton)
{
    bool bCustomize = pButton == m_pCustomizePB;
    SwCustomizeAddressBlockDialog::DialogType nType = bCustomize ?
        SwCustomizeAddressBlockDialog::ADDRESSBLOCK_EDIT :
        SwCustomizeAddressBlockDialog::ADDRESSBLOCK_NEW;
    SwCustomizeAddressBlockDialog *pDlg =
        new SwCustomizeAddressBlockDialog(pButton,m_rConfig,nType);
    if(bCustomize)
    {
        pDlg->SetAddress(m_aAddressBlocks[m_pPreview->GetSelectedAddress()]);
    }
    if(RET_OK == pDlg->Execute())
    {
        if(bCustomize)
        {
            OUString sNew = pDlg->GetAddress();
            m_pPreview->ReplaceSelectedAddress(sNew);
            m_aAddressBlocks[m_pPreview->GetSelectedAddress()] = sNew;
        }
        else
        {
            OUString sNew = pDlg->GetAddress();
            m_pPreview->AddAddress(sNew);
            m_aAddressBlocks.realloc(m_aAddressBlocks.getLength() + 1);
            sal_uInt16 nSelect = (sal_uInt16)m_aAddressBlocks.getLength() - 1;
            m_aAddressBlocks[nSelect] = sNew;
            m_pPreview->SelectAddress(nSelect);
        }
        m_pDeletePB->Enable( m_aAddressBlocks.getLength() > 1);
    }
    delete pDlg;
    return 0;
}

IMPL_LINK(SwSelectAddressBlockDialog, IncludeHdl_Impl, RadioButton*, pButton)
{
    m_pCountryED->Enable(m_pDependentRB == pButton);
    return 0;
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeSwRestrictedComboBox(Window *pParent, VclBuilder::stringmap &rMap)
{
    WinBits nBits = WB_LEFT|WB_VCENTER|WB_3DLOOK;

    bool bDropdown = VclBuilder::extractDropdown(rMap);

    if (bDropdown)
        nBits |= WB_DROPDOWN;

    SwRestrictedComboBox* pComboBox = new SwRestrictedComboBox(pParent, nBits);
    pComboBox->EnableAutoSize(true);
    return pComboBox;
}

void SwRestrictedComboBox::KeyInput(const KeyEvent& rEvt)
{
    bool bCallParent = true;
    if(rEvt.GetCharCode())
    {
        OUString sKey(rEvt.GetCharCode());
        if( -1 != sForbiddenChars.indexOf(sKey))
            bCallParent = false;
    }
    if(bCallParent)
        ComboBox::KeyInput(rEvt);
}

void SwRestrictedComboBox::Modify()
{
    Selection aSel = GetSelection();
    OUString sTemp = GetText();
    for(sal_uInt16 i = 0; i < sForbiddenChars.getLength(); i++)
    {
        sTemp = comphelper::string::remove(sTemp, sForbiddenChars[i]);
    }
    sal_Int32 nDiff = GetText().getLength() - sTemp.getLength();
    if(nDiff)
    {
        aSel.setMin(aSel.getMin() - nDiff);
        aSel.setMax(aSel.getMin());
        SetText(sTemp);
        SetSelection(aSel);
    }
    if(GetModifyHdl().IsSet())
        GetModifyHdl().Call(this);
}

#define USER_DATA_SALUTATION        -1
#define USER_DATA_PUNCTUATION       -2
#define USER_DATA_TEXT              -3
#define USER_DATA_NONE              -4

SwCustomizeAddressBlockDialog::SwCustomizeAddressBlockDialog(
        Window* pParent, SwMailMergeConfigItem& rConfig, DialogType eType)
    : SfxModalDialog(pParent, "AddressBlockDialog",
        "modules/swriter/ui/addressblockdialog.ui")
    , m_rConfigItem(rConfig)
    , m_eType(eType)
{
    get(m_pOK, "ok");
    get(m_pPreviewWIN, "addrpreview");
    get(m_pFieldCB, "custom");
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
        SvTreeListEntry* pEntry = m_pAddressElementsLB->InsertEntry(OUString(SW_RES(ST_SALUTATION )));
        pEntry->SetUserData((void*)(sal_Int32)USER_DATA_SALUTATION );
        pEntry = m_pAddressElementsLB->InsertEntry(OUString(SW_RES(ST_PUNCTUATION)));
        pEntry->SetUserData((void*)(sal_Int32)USER_DATA_PUNCTUATION );
        pEntry = m_pAddressElementsLB->InsertEntry(OUString(SW_RES(ST_TEXT       )));
        pEntry->SetUserData((void*)(sal_Int32)USER_DATA_TEXT       );
        ResStringArray aSalutArr(SW_RES(RA_SALUTATION));
        sal_uInt16 i;
        for(i = 0; i < aSalutArr.Count(); ++i)
            m_aSalutations.push_back(aSalutArr.GetString(i));
        ResStringArray aPunctArr(SW_RES(RA_PUNCTUATION));
        for(i = 0; i < aPunctArr.Count(); ++i)
            m_aPunctuations.push_back(aPunctArr.GetString(i));
        m_pDragED->SetText(OUString("            "));
        SetText( OUString( SW_RES( eType == GREETING_MALE ? ST_TITLE_MALE : ST_TITLE_FEMALE)));
        m_pAddressElementsFT->SetText(OUString(SW_RES(ST_SALUTATIONELEMENTS)));
        m_pInsertFieldIB->SetQuickHelpText(OUString(SW_RES(ST_INSERTSALUTATIONFIELD)));
        m_pRemoveFieldIB->SetQuickHelpText(OUString(SW_RES(ST_REMOVESALUTATIONFIELD)));
        m_pDragFT->SetText(OUString(SW_RES(ST_DRAGSALUTATION)));
    }
    else
    {
        if(eType == ADDRESSBLOCK_EDIT)
            SetText(OUString(SW_RES(ST_TITLE_EDIT)));
        m_pDragED->SetText(OUString("\n\n\n\n\n"));
    }

    const ResStringArray& rHeaders = m_rConfigItem.GetDefaultAddressHeaders();
    for(sal_uInt16 i = 0; i < rHeaders.Count(); ++i)
    {
        const OUString rHeader = rHeaders.GetString( i );
        SvTreeListEntry* pEntry = m_pAddressElementsLB->InsertEntry(rHeader);
        pEntry->SetUserData((void*)(sal_IntPtr)i);
    }
    m_pOK->SetClickHdl(LINK(this, SwCustomizeAddressBlockDialog, OKHdl_Impl));
    m_pAddressElementsLB->SetSelectHdl(LINK(this, SwCustomizeAddressBlockDialog, ListBoxSelectHdl_Impl ));
    m_pDragED->SetModifyHdl(LINK(this, SwCustomizeAddressBlockDialog, EditModifyHdl_Impl));
    m_pDragED->SetSelectionChangedHdl( LINK( this, SwCustomizeAddressBlockDialog, SelectionChangedHdl_Impl));
    Link aFieldsLink = LINK(this, SwCustomizeAddressBlockDialog, FieldChangeHdl_Impl);
    m_pFieldCB->SetModifyHdl(aFieldsLink);
    m_pFieldCB->SetSelectHdl(aFieldsLink);
    Link aImgButtonHdl = LINK(this, SwCustomizeAddressBlockDialog, ImageButtonHdl_Impl);
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
}

IMPL_LINK_NOARG(SwCustomizeAddressBlockDialog, OKHdl_Impl)
{
    EndDialog(RET_OK);
    return 0;
}

IMPL_LINK(SwCustomizeAddressBlockDialog, ListBoxSelectHdl_Impl, DDListBox*, pBox)
{
    sal_Int32 nUserData = (sal_Int32)(sal_IntPtr)pBox->FirstSelected()->GetUserData();
    // Check if the selected entry is already in the address and then forbid inserting
    m_pInsertFieldIB->Enable(nUserData >= 0 || !HasItem_Impl(nUserData));
    return 0;
}

IMPL_LINK_NOARG(SwCustomizeAddressBlockDialog, EditModifyHdl_Impl)
{
    OUString sAddress = SwAddressPreview::FillData(
            GetAddress(),
            m_rConfigItem);
    m_pPreviewWIN->SetAddress(sAddress);
    UpdateImageButtons_Impl();
    return 0;
}

IMPL_LINK(SwCustomizeAddressBlockDialog, ImageButtonHdl_Impl, ImageButton*, pButton)
{
    if (m_pInsertFieldIB == pButton)
    {
        SvTreeListEntry* pEntry = m_pAddressElementsLB->GetCurEntry();
        if(pEntry)
        {
            OUString sEntry = "<" + m_pAddressElementsLB->GetEntryText(pEntry) + ">";
            m_pDragED->InsertNewEntry(sEntry);
        }
    }
    else if (m_pRemoveFieldIB == pButton)
    {
        m_pDragED->RemoveCurrentEntry();
    }
    else
    {
        sal_uInt16 nMove = MOVE_ITEM_DOWN;
        if (m_pUpIB == pButton)
            nMove = MOVE_ITEM_UP;
        else if (m_pLeftIB == pButton)
            nMove = MOVE_ITEM_LEFT;
        else if (m_pRightIB == pButton)
            nMove = MOVE_ITEM_RIGHT;
        m_pDragED->MoveCurrentItem(nMove);
    }
    UpdateImageButtons_Impl();
    return 0;
}

sal_Int32 SwCustomizeAddressBlockDialog::GetSelectedItem_Impl()
{
    sal_Int32 nRet = USER_DATA_NONE;
    OUString sSelected = m_pDragED->GetCurrentItem();
    if(!sSelected.isEmpty())
        for(sal_uLong i = 0; i < m_pAddressElementsLB->GetEntryCount();  ++i)
        {
            SvTreeListEntry* pEntry = m_pAddressElementsLB->GetEntry(i);
            OUString sEntry = m_pAddressElementsLB->GetEntryText(pEntry);
            if( sEntry == sSelected.copy( 1, sSelected.getLength() - 2 ) )
            {
                nRet = (sal_Int32)(sal_IntPtr)pEntry->GetUserData();
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
        if((sal_Int32)(sal_IntPtr)pEntry->GetUserData() == nUserData)
        {
            sEntry = m_pAddressElementsLB->GetEntryText(pEntry);
            break;
        }
    }
    //put it into '<>'
    sEntry = "<" + sEntry + ">";
    //search for this entry in the content
    OUString sText = m_pDragED->GetText();
    bool bRet = sText.indexOf(sEntry) != -1;
    return bRet;
}

IMPL_LINK(SwCustomizeAddressBlockDialog, SelectionChangedHdl_Impl, AddressMultiLineEdit*, pEdit)
{
    // called in case the selection of the edit field changes.
    // determine selection - if it's one of the editable fields then
    // enable the related ComboBox and fill it
    static bool bOnEntry = false;
    if(bOnEntry)
        return 0;

    bOnEntry = true;
    sal_Int32 nSelected = GetSelectedItem_Impl();
    if(USER_DATA_NONE != nSelected)
        pEdit->SelectCurrentItem();

    if(m_pFieldCB->IsVisible() && (USER_DATA_NONE != nSelected) && (nSelected < 0))
    {
        //search in ListBox if it's one of the first entries
        OUString sSelect;
        ::std::vector<OUString>* pVector = 0;
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
            ::std::vector<OUString>::iterator  aIterator;
            for( aIterator = pVector->begin(); aIterator != pVector->end(); ++aIterator)
                m_pFieldCB->InsertEntry(*aIterator);
        }
        m_pFieldCB->SetText(sSelect);
        m_pFieldCB->Enable(sal_True);
        m_pFieldFT->Enable(sal_True);
    }
    else
    {
        m_pFieldCB->Enable(sal_False);
        m_pFieldFT->Enable(sal_False);
    }

    UpdateImageButtons_Impl();
    bOnEntry = false;
    return 0;
}

IMPL_LINK_NOARG(SwCustomizeAddressBlockDialog, FieldChangeHdl_Impl)
{
    //changing the field content changes the related members, too
    sal_Int32 nSelected = GetSelectedItem_Impl();
    OUString sContent = m_pFieldCB->GetText();
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
    return 0;
}

void SwCustomizeAddressBlockDialog::UpdateImageButtons_Impl()
{
    sal_uInt16 nMove = m_pDragED->IsCurrentItemMoveable();
    m_pUpIB->Enable(nMove & MOVE_ITEM_UP );
    m_pLeftIB->Enable(nMove & MOVE_ITEM_LEFT );
    m_pRightIB->Enable(nMove & MOVE_ITEM_RIGHT );
    m_pDownIB->Enable(nMove & MOVE_ITEM_DOWN);
    m_pRemoveFieldIB->Enable(m_pDragED->HasCurrentItem() ? sal_True : sal_False);
    SvTreeListEntry* pEntry = m_pAddressElementsLB->GetCurEntry();
    m_pInsertFieldIB->Enable( pEntry &&
            (0 < (sal_Int32)(sal_IntPtr)pEntry->GetUserData() || !m_pFieldCB->GetText().isEmpty()));
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
            OUString sEntry = "<" + m_pAddressElementsLB->GetEntryText(pEntry) + ">";
            sal_Int32 nUserData = (sal_Int32)(sal_IntPtr)pEntry->GetUserData();
            switch(nUserData)
            {
                case USER_DATA_SALUTATION : sAddress = sAddress.replaceFirst(sEntry, m_sCurrentSalutation); break;
                case USER_DATA_PUNCTUATION: sAddress = sAddress.replaceFirst(sEntry, m_sCurrentPunctuation); break;
                case USER_DATA_TEXT       : sAddress = sAddress.replaceFirst(sEntry, m_sCurrentText); break;
            }
        }
    }
    return sAddress;
}

class SwAssignFieldsControl : public Control
{
    friend class SwAssignFieldsDialog;
    ScrollBar                   m_aVScroll;
    HeaderBar                   m_aHeaderHB;
    Window                      m_aWindow;

    ::std::vector<FixedInfo*>   m_aFieldNames;
    ::std::vector<ListBox*>     m_aMatches;
    ::std::vector<FixedInfo*>   m_aPreviews;

    SwMailMergeConfigItem&      m_rConfigItem;

    Link                        m_aModifyHdl;

    long                        m_nLBStartTopPos;
    long                        m_nYOffset;
    long                        m_nFirstYPos;

    DECL_LINK(ScrollHdl_Impl, ScrollBar*);
    DECL_LINK(MatchHdl_Impl, ListBox*);
    DECL_LINK(GotFocusHdl_Impl, ListBox*);

    virtual long        PreNotify( NotifyEvent& rNEvt );
    virtual void        Command( const CommandEvent& rCEvt );

    void                MakeVisible( sal_Int32 nIndex );
public:
    SwAssignFieldsControl(Window* pParent, const ResId& rResId,
                                SwMailMergeConfigItem& rConfigItem);
    ~SwAssignFieldsControl();

    void        SetModifyHdl(const Link& rModifyHdl)
                {
                    m_aModifyHdl = rModifyHdl;
                    m_aModifyHdl.Call(this);
                }
};

SwAssignFieldsControl::SwAssignFieldsControl(
        Window* pParent, const ResId& rResId, SwMailMergeConfigItem& rConfigItem) :
    Control(pParent, rResId),
    m_aVScroll(this,  ResId(SCR_1, *rResId.GetResMgr()     )),
    m_aHeaderHB(this, WB_BUTTONSTYLE | WB_BOTTOMBORDER),
    m_aWindow(this, ResId(WIN_DATA, *rResId.GetResMgr())),
    m_rConfigItem(rConfigItem),
    m_nLBStartTopPos(0),
    m_nYOffset(0),
    m_nFirstYPos(0)
{
    SetStyle(GetStyle()|WB_TABSTOP|WB_DIALOGCONTROL);
    SetHelpId(HID_MM_ASSIGN_FIELDS);
    long nHBHeight = m_aHeaderHB.CalcWindowSizePixel().Height();
    Size aOutputSize(GetOutputSize());
    m_aHeaderHB.SetSizePixel(
        Size(aOutputSize.Width(), nHBHeight));
    m_aHeaderHB.Show();
    m_aWindow.SetPosPixel(Point( 0, nHBHeight) );
    m_aWindow.SetSizePixel(Size(aOutputSize.Width() - m_aVScroll.GetSizePixel().Width(), aOutputSize.Height() - nHBHeight));
    m_aWindow.Show();

    //get the name of the default headers
    const ResStringArray& rHeaders = rConfigItem.GetDefaultAddressHeaders();
    //get the actual data
    uno::Reference< XColumnsSupplier > xColsSupp( rConfigItem.GetResultSet(), uno::UNO_QUERY);
    //get the name of the actual columns
    uno::Reference <XNameAccess> xColAccess = xColsSupp.is() ? xColsSupp->getColumns() : 0;
    uno::Sequence< OUString > aFields;
    if(xColAccess.is())
        aFields = xColAccess->getElementNames();
    const OUString* pFields = aFields.getConstArray();

    //get the current assignment list
    //each position in this sequence matches the position in the header array rHeaders
    //if no assignment is available an empty sequence will be returned
    uno::Sequence< OUString> aAssignments = rConfigItem.GetColumnAssignment( rConfigItem.GetCurrentDBData() );
    Link aMatchHdl = LINK(this, SwAssignFieldsControl, MatchHdl_Impl);
    Link aFocusHdl = LINK(this, SwAssignFieldsControl, GotFocusHdl_Impl);

    static const char* aHIDs[] =
    {
         HID_MM_HEADER_0,
         HID_MM_HEADER_1,
         HID_MM_HEADER_2,
         HID_MM_HEADER_3,
         HID_MM_HEADER_4,
         HID_MM_HEADER_5,
         HID_MM_HEADER_6,
         HID_MM_HEADER_7,
         HID_MM_HEADER_8,
         HID_MM_HEADER_9,
         HID_MM_HEADER_10,
         HID_MM_HEADER_11,
         HID_MM_HEADER_12,
         HID_MM_HEADER_13
    };

    //fill the controls
    for(sal_uInt16 i = 0; i < rHeaders.Count(); ++i)
    {
        const OUString rHeader = rHeaders.GetString( i );
        FixedInfo* pNewText = new FixedInfo(&m_aWindow, ResId( FT_FIELDS, *rResId.GetResMgr()));
        pNewText->SetText("<" + rHeader + ">");
        ListBox* pNewLB = new ListBox(&m_aWindow, ResId(LB_FIELDS, *rResId.GetResMgr()));
        pNewLB->SetHelpId( aHIDs[i] );
        pNewLB->SelectEntryPos(0);
        for(sal_Int32 nField = 0; nField < aFields.getLength(); ++nField)
            pNewLB->InsertEntry(pFields[nField]);
        FixedInfo* pNewPreview = new FixedInfo(&m_aWindow, ResId( FT_PREVIEW, *rResId.GetResMgr() ));
        //select the ListBox
        //if there is an assignment
        if(aAssignments.getLength() > i && !aAssignments[i].isEmpty())
            pNewLB->SelectEntry(aAssignments[i]);
        else //otherwise the current column name may match one of the db columns
            pNewLB->SelectEntry(rHeader);
        //then the preview can be filled accordingly
        if(xColAccess.is() && pNewLB->GetSelectEntryPos() > 0 &&
                xColAccess->hasByName(pNewLB->GetSelectEntry()))
        {
            uno::Any aCol = xColAccess->getByName(pNewLB->GetSelectEntry());
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
            m_nYOffset += pNewLB->GetSizePixel().Height();
        }

        long nMove = m_nYOffset * i;
        lcl_Move(pNewText, nMove);
        lcl_Move(pNewLB, nMove);
        lcl_Move(pNewPreview, nMove);
        //set the select handler
        pNewLB->SetSelectHdl(aMatchHdl);
        pNewLB->SetGetFocusHdl(aFocusHdl);

        m_aFieldNames.push_back(pNewText);
        m_aMatches.push_back(pNewLB);
        m_aPreviews.push_back(pNewPreview);
    }
    m_aVScroll.SetRange(Range(0, rHeaders.Count()));
    m_aVScroll.SetPageSize((aOutputSize.Height() - nHBHeight - m_nLBStartTopPos)/ m_nYOffset);
    m_aVScroll.EnableDrag();
    m_aVScroll.SetVisibleSize(m_aVScroll.GetPageSize());
    m_aVScroll.SetScrollHdl(LINK(this, SwAssignFieldsControl, ScrollHdl_Impl));

    FreeResource();
    m_aVScroll.SetPosPixel(Point(aOutputSize.Width() - m_aVScroll.GetSizePixel().Width(), nHBHeight));
    m_aVScroll.SetSizePixel(Size(m_aVScroll.GetSizePixel().Width(), aOutputSize.Height() - nHBHeight));

}

SwAssignFieldsControl::~SwAssignFieldsControl()
{
    ::std::vector<FixedInfo*>::iterator aFIIter;
    for(aFIIter = m_aFieldNames.begin(); aFIIter != m_aFieldNames.end(); ++aFIIter)
        delete *aFIIter;
    ::std::vector<ListBox*>::iterator aLBIter;
    for(aLBIter = m_aMatches.begin(); aLBIter != m_aMatches.end(); ++aLBIter)
        delete *aLBIter;
    for(aFIIter = m_aPreviews.begin(); aFIIter != m_aPreviews.end(); ++aFIIter)
        delete *aFIIter;
}

void SwAssignFieldsControl::Command( const CommandEvent& rCEvt )
{
    switch ( rCEvt.GetCommand() )
    {
        case COMMAND_WHEEL:
        case COMMAND_STARTAUTOSCROLL:
        case COMMAND_AUTOSCROLL:
        {
            const CommandWheelData* pWheelData = rCEvt.GetWheelData();
            if(pWheelData && !pWheelData->IsHorz() && COMMAND_WHEEL_ZOOM != pWheelData->GetMode())
            {
                HandleScrollCommand( rCEvt, 0, &m_aVScroll );
            }
        }
        break;
        default:
            Control::Command(rCEvt);
    }
}

long SwAssignFieldsControl::PreNotify( NotifyEvent& rNEvt )
{
    if(rNEvt.GetType() == EVENT_COMMAND)
    {
        const CommandEvent* pCEvt = rNEvt.GetCommandEvent();
        sal_uInt16 nCmd = pCEvt->GetCommand();
        if( COMMAND_WHEEL == nCmd )
        {
            Command(*pCEvt);
            return 1;
        }
    }
    return Control::PreNotify(rNEvt);
}

void SwAssignFieldsControl::MakeVisible( sal_Int32 nIndex )
{
    long nThumb = m_aVScroll.GetThumbPos();
    long nPage = m_aVScroll.GetPageSize();
    if(nThumb > nIndex)
        m_aVScroll.SetThumbPos( nIndex );
    else if( (nThumb + nPage) < nIndex)
        m_aVScroll.SetThumbPos( nIndex - nPage );
    else
        return;
    ScrollHdl_Impl( &m_aVScroll );
}

IMPL_LINK(SwAssignFieldsControl, ScrollHdl_Impl, ScrollBar*, pScroll)
{
    long nThumb = pScroll->GetThumbPos();
    // the scrollbar moves on a per line basis
    // the height of a line is stored in m_nYOffset
    // nThumb determines which line has to be set at the top (m_nYOffset)
    // The first line has to be -(nThumb * m_nYOffset) in the negative
    long nMove = m_nFirstYPos - (*m_aMatches.begin())->GetPosPixel().Y() - (nThumb * m_nYOffset);

    SetUpdateMode(sal_False);
    long nIndex;
    ::std::vector<FixedInfo*>::iterator aFIIter;
    for(nIndex = 0, aFIIter = m_aFieldNames.begin(); aFIIter != m_aFieldNames.end(); ++aFIIter, ++nIndex)
        lcl_Move(*aFIIter, nMove);
    ::std::vector<ListBox*>::iterator aLBIter;
    for(nIndex = 0, aLBIter = m_aMatches.begin(); aLBIter != m_aMatches.end(); ++aLBIter, ++nIndex)
        lcl_Move(*aLBIter, nMove);
    for(nIndex = 0, aFIIter = m_aPreviews.begin(); aFIIter != m_aPreviews.end(); ++aFIIter, ++nIndex)
        lcl_Move(*aFIIter, nMove);
    SetUpdateMode(sal_True);

    return 0;
}

IMPL_LINK(SwAssignFieldsControl, MatchHdl_Impl, ListBox*, pBox)
{
    const OUString sColumn = pBox->GetSelectEntry();
    uno::Reference< XColumnsSupplier > xColsSupp( m_rConfigItem.GetResultSet(), uno::UNO_QUERY);
    uno::Reference <XNameAccess> xColAccess = xColsSupp.is() ? xColsSupp->getColumns() : 0;
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
    ::std::vector<ListBox*>::iterator aLBIter;
    sal_Int32 nIndex = 0;
    for(aLBIter = m_aMatches.begin(); aLBIter != m_aMatches.end(); ++aLBIter, ++nIndex)
    {
        if(*aLBIter == pBox)
        {
            m_aPreviews[nIndex]->SetText(sPreview);
            break;
        }
    }
    m_aModifyHdl.Call(0);
    return 0;
}

IMPL_LINK(SwAssignFieldsControl, GotFocusHdl_Impl, ListBox*, pBox)
{
    if(0 != (GETFOCUS_TAB & pBox->GetGetFocusFlags()))
    {
        sal_Int32 nIndex = 0;
        ::std::vector<ListBox*>::iterator aLBIter;
        for(aLBIter = m_aMatches.begin(); aLBIter != m_aMatches.end(); ++aLBIter, ++nIndex)
        {
            if(*aLBIter == pBox)
            {
                MakeVisible(nIndex);
                break;
            }
        }
    }
    return 0;
}

SwAssignFieldsDialog::SwAssignFieldsDialog(
        Window* pParent, SwMailMergeConfigItem& rConfigItem,
        const OUString& rPreview,
        bool bIsAddressBlock) :
    SfxModalDialog(pParent, SW_RES(DLG_MM_ASSIGNFIELDS)),
    m_aMatchingFI( this, SW_RES(     FI_MATCHING)),
    m_pFieldsControl( new SwAssignFieldsControl(this, SW_RES(  CT_FIELDS  ), rConfigItem)),
    m_aPreviewFI( this, SW_RES(      FI_PREVIEW )),
    m_aPreviewWIN( this, SW_RES(     WIN_PREVIEW )),
    m_aSeparatorFL( this, SW_RES(    FL_SEPARATOR)),
    m_aOK( this, SW_RES(             PB_OK       )),
    m_aCancel( this, SW_RES(         PB_CANCEL   )),
    m_aHelp( this, SW_RES(           PB_HELP     )),
    m_sNone(SW_RESSTR(SW_STR_NONE)),
    m_rPreviewString(rPreview),
    m_rConfigItem(rConfigItem)
{
    //resize the HeaderBar
    OUString sAddressElement( SW_RESSTR(ST_ADDRESSELEMENT) );
    const OUString sMatchesTo( SW_RESSTR(ST_MATCHESTO) );
    const OUString sPreview( SW_RESSTR(ST_PREVIEW) );
    if(!bIsAddressBlock)
    {
        m_aPreviewFI.SetText(SW_RESSTR(ST_SALUTATIONPREVIEW));
        m_aMatchingFI.SetText(SW_RESSTR(ST_SALUTATIONMATCHING));
        sAddressElement = SW_RESSTR(ST_SALUTATIONELEMENT);
    }
    FreeResource();
    Size aOutputSize(m_pFieldsControl->m_aHeaderHB.GetSizePixel());
    sal_Int32 nFirstWidth;
    sal_Int32 nSecondWidth = nFirstWidth = aOutputSize.Width() / 3;
    const WinBits nHeadBits = HIB_VCENTER | HIB_FIXED| HIB_FIXEDPOS;
    m_pFieldsControl->m_aHeaderHB.InsertItem( 1, sAddressElement, nFirstWidth, nHeadBits|HIB_LEFT);
    m_pFieldsControl->m_aHeaderHB.InsertItem( 2, sMatchesTo,      nSecondWidth, nHeadBits|HIB_LEFT);
    m_pFieldsControl->m_aHeaderHB.InsertItem( 3, sPreview,
            aOutputSize.Width() - nFirstWidth - nSecondWidth, nHeadBits|HIB_LEFT);

    m_pFieldsControl->SetModifyHdl(LINK(this, SwAssignFieldsDialog, AssignmentModifyHdl_Impl ));

    m_aMatchingFI.SetText(m_aMatchingFI.GetText().replaceAll("%1", sMatchesTo));

    m_aOK.SetClickHdl(LINK(this, SwAssignFieldsDialog, OkHdl_Impl));
}

SwAssignFieldsDialog::~SwAssignFieldsDialog()
{
    delete m_pFieldsControl;
}

uno::Sequence< OUString > SwAssignFieldsDialog::CreateAssignments()
{
    uno::Sequence< OUString > aAssignments(
            m_rConfigItem.GetDefaultAddressHeaders().Count());
    OUString* pAssignments = aAssignments.getArray();
    ::std::vector<ListBox*>::iterator aLBIter;
    sal_Int32 nIndex = 0;
    for(aLBIter = m_pFieldsControl->m_aMatches.begin();
                aLBIter != m_pFieldsControl->m_aMatches.end();
                    ++aLBIter, ++nIndex)
    {
        const OUString sSelect = (*aLBIter)->GetSelectEntry();
        pAssignments[nIndex] = (m_sNone != sSelect) ? sSelect : OUString();
    }
    return aAssignments;
}

IMPL_LINK_NOARG(SwAssignFieldsDialog, OkHdl_Impl)
{
    m_rConfigItem.SetColumnAssignment(
                            m_rConfigItem.GetCurrentDBData(),
                            CreateAssignments() );
    EndDialog(RET_OK);
    return 0;
}

IMPL_LINK_NOARG(SwAssignFieldsDialog, AssignmentModifyHdl_Impl)
{
    uno::Sequence< OUString > aAssignments = CreateAssignments();
    const OUString sPreview = SwAddressPreview::FillData(
            m_rPreviewString, m_rConfigItem, &aAssignments);
    m_aPreviewWIN.SetAddress(sPreview);
    return 0;
}

DDListBox::DDListBox(Window* pParent, WinBits nStyle)
    : SvTreeListBox(pParent, nStyle)
    , m_pParentDialog(NULL)
{
    SetStyle( GetStyle() | /*WB_HASBUTTONS|WB_HASBUTTONSATROOT|*/
                            WB_CLIPCHILDREN );
    SetSelectionMode( SINGLE_SELECTION );
    SetDragDropMode(   SV_DRAGDROP_CTRL_COPY );
    EnableAsyncDrag(sal_True);
    // expand selection to the complete width of the ListBox
    SetHighlightRange();
    Show();

}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeDDListBox(Window *pParent, VclBuilder::stringmap &rMap)
{
    WinBits nWinStyle = WB_TABSTOP;
    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
        nWinStyle |= WB_BORDER;
    return new DDListBox(pParent, nWinStyle);
}

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

        TransferDataContainer* pContainer = new TransferDataContainer;
        uno::Reference<
             datatransfer::XTransferable > xRef( pContainer );

        sal_Int32 nUserData = (sal_Int32)(sal_IntPtr)pEntry->GetUserData();
        //special entries can only be once in the address / greeting
        if(nUserData >= 0 || !m_pParentDialog->HasItem_Impl(nUserData))
        {
            OUString sEntry = "<" + GetEntryText(pEntry) + ">";
            pContainer->CopyString( sEntry );
            pContainer->StartDrag( this, DND_ACTION_COPY, GetDragFinishedHdl() );
        }
    }
}

AddressMultiLineEdit::AddressMultiLineEdit(Window* pParent, WinBits nBits)
    : VclMultiLineEdit(pParent, nBits)
    , m_pParentDialog(NULL)
{
    GetTextView()->SupportProtectAttribute(sal_True);
    StartListening(*GetTextEngine());
    EnableFocusSelectionHide(sal_False);
}

Size AddressMultiLineEdit::GetOptimalSize() const
{
    return LogicToPixel(Size(160, 60), MapMode(MAP_APPFONT));
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeAddressMultiLineEdit(Window *pParent, VclBuilder::stringmap &rMap)
{
    WinBits nWinStyle = WB_LEFT|WB_TABSTOP;
    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
        nWinStyle |= WB_BORDER;
    return new AddressMultiLineEdit(pParent, nWinStyle);
}

void AddressMultiLineEdit::SetAddressDialog(SwCustomizeAddressBlockDialog *pParent)
{
    m_pParentDialog = pParent;
}

AddressMultiLineEdit::~AddressMultiLineEdit()
{
    EndListening(*GetTextEngine());
}

void    AddressMultiLineEdit::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint )
{
    if(rHint.ISA(TextHint) &&
            static_cast<const TextHint&>(rHint).GetId() == TEXT_HINT_VIEWSELECTIONCHANGED &&
            m_aSelectionLink.IsSet())
    {
        m_aSelectionLink.Call(this);
    }
}

long  AddressMultiLineEdit::PreNotify( NotifyEvent& rNEvt )
{
    long nHandled = 0;
    if( EVENT_KEYINPUT == rNEvt.GetType()  &&
        rNEvt.GetKeyEvent()->GetCharCode())
    {
        nHandled = 1;
    }
    else if(EVENT_MOUSEBUTTONDOWN == rNEvt.GetType()) {
        const MouseEvent *pMEvt = rNEvt.GetMouseEvent();
        if(pMEvt->GetClicks() >= 2)
            nHandled = 1;
    }
    if(!nHandled)
        nHandled = VclMultiLineEdit::PreNotify( rNEvt );
    return nHandled;

}

void AddressMultiLineEdit::SetText( const OUString& rStr )
{
    VclMultiLineEdit::SetText(rStr);
    //set attributes to all address tokens

    ExtTextEngine* pTextEngine = GetTextEngine();
    TextAttribProtect aProtectAttr;
    sal_uLong  nParaCount = pTextEngine->GetParagraphCount();
    for(sal_uLong nPara = 0; nPara < nParaCount; ++nPara)
    {
        xub_StrLen nIndex = 0;
        OUString sPara = pTextEngine->GetText( nPara );
        if(!sPara.isEmpty() && !sPara.endsWith(" "))
        {
            TextPaM aPaM(nPara, sPara.getLength());
            pTextEngine->ReplaceText(TextSelection( aPaM ), OUString(' '));
        }
        while(true)
        {
            sal_Int32 nStart = sPara.indexOf( '<', nIndex );
            sal_Int32 nEnd = nStart == -1 ? -1 : sPara.indexOf( '>', nStart );
            nIndex = nEnd;
            if(nStart != -1 && nEnd != -1)
                pTextEngine->SetAttrib( aProtectAttr, nPara, nStart, nEnd + 1, sal_False );
            else
                break;
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
            pTextEngine->ReplaceText( TextSelection( aPaM ), OUString("\n \n "));
        }
    }
}


// Insert the new entry in front of the entry at the beginning of the selection
void AddressMultiLineEdit::InsertNewEntry( const OUString& rStr )
{
    // insert new entry after current selected one.
    ExtTextView* pTextView = GetTextView();
    const TextSelection& rSelection = pTextView->GetSelection();
    sal_uLong nPara = rSelection.GetStart().GetPara();
    sal_uInt16 nIndex = rSelection.GetEnd().GetIndex();
    ExtTextEngine *pTextEngine = GetTextEngine();
    const TextCharAttrib *pAttrib;
    if(0 != (pAttrib = pTextEngine->FindCharAttrib( rSelection.GetStart(), TEXTATTR_PROTECTED )))
        nIndex = pAttrib->GetEnd();
    InsertNewEntryAtPosition( rStr, nPara, nIndex );

    // select the new entry
    pAttrib = pTextEngine->FindCharAttrib(TextPaM(nPara, nIndex),TEXTATTR_PROTECTED);
    TextSelection aEntrySel(TextPaM(nPara, nIndex), TextPaM(nPara, pAttrib->GetEnd()));
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
    ExtTextView* pTextView = GetTextView();
    pTextView->SetSelection(aEntrySel);
    m_aSelectionLink.Call(this);
}

void AddressMultiLineEdit::RemoveCurrentEntry()
{
    ExtTextEngine* pTextEngine = GetTextEngine();
    ExtTextView* pTextView = GetTextView();
    const TextSelection& rSelection = pTextView->GetSelection();
    const TextCharAttrib* pBeginAttrib = pTextEngine->FindCharAttrib( rSelection.GetStart(), TEXTATTR_PROTECTED );
    if(pBeginAttrib &&
            (pBeginAttrib->GetStart() <= rSelection.GetStart().GetIndex()
                            && pBeginAttrib->GetEnd() >= rSelection.GetEnd().GetIndex()))
    {
        sal_uLong nPara = rSelection.GetStart().GetPara();
        TextSelection aEntrySel(TextPaM( nPara, pBeginAttrib->GetStart()), TextPaM(nPara, pBeginAttrib->GetEnd()));
        pTextEngine->ReplaceText(aEntrySel, OUString());
        //restore the attributes
        SetText( GetAddress() );
        Modify();
    }
}

void AddressMultiLineEdit::MoveCurrentItem(sal_uInt16 nMove)
{
    ExtTextEngine* pTextEngine = GetTextEngine();
    ExtTextView* pTextView = GetTextView();
    const TextSelection& rSelection = pTextView->GetSelection();
    const TextCharAttrib* pBeginAttrib = pTextEngine->FindCharAttrib( rSelection.GetStart(), TEXTATTR_PROTECTED );
    if(pBeginAttrib &&
            (pBeginAttrib->GetStart() <= rSelection.GetStart().GetIndex()
                            && pBeginAttrib->GetEnd() >= rSelection.GetEnd().GetIndex()))
    {
        //current item has been found
        sal_uLong nPara = rSelection.GetStart().GetPara();
        sal_uInt16 nIndex = pBeginAttrib->GetStart();
        TextSelection aEntrySel(TextPaM( nPara, pBeginAttrib->GetStart()), TextPaM(nPara, pBeginAttrib->GetEnd()));
        OUString sCurrentItem = pTextEngine->GetText(aEntrySel);
        pTextEngine->RemoveAttrib( nPara, *pBeginAttrib );
        pTextEngine->ReplaceText(aEntrySel, OUString());
        switch(nMove)
        {
            case MOVE_ITEM_LEFT :
                if(nIndex)
                {
                    //go left to find a predecessor or simple text
                    --nIndex;
                    OUString sPara = pTextEngine->GetText( nPara );
                    sal_Int32 nSearchIndex = sPara.lastIndexOf( '>', nIndex+1 );
                    if( nSearchIndex != -1 && nSearchIndex == nIndex )
                    {
                        nSearchIndex = sPara.lastIndexOf( '<', nIndex );
                        if( nSearchIndex != -1 )
                            nIndex = nSearchIndex;
                    }
                }
            break;
            case MOVE_ITEM_RIGHT:
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
            case MOVE_ITEM_UP   :
                --nPara;
                nIndex = 0;
            break;
            case MOVE_ITEM_DOWN :
                ++nPara;
                nIndex = 0;
            break;
        }
        //add a new paragraph if there is none yet
        if(nPara >= pTextEngine->GetParagraphCount())
        {

            TextPaM aTemp(nPara - 1, pTextEngine->GetTextLen( nPara - 1 ));
            pTextEngine->ReplaceText( aTemp, OUString('\n'));
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
}

sal_uInt16  AddressMultiLineEdit::IsCurrentItemMoveable()
{
    sal_uInt16 nRet = 0;
    ExtTextEngine* pTextEngine = GetTextEngine();
    ExtTextView* pTextView = GetTextView();
    const TextSelection& rSelection = pTextView->GetSelection();
    const TextCharAttrib* pBeginAttrib = pTextEngine->FindCharAttrib( rSelection.GetStart(), TEXTATTR_PROTECTED );
    if(pBeginAttrib &&
            (pBeginAttrib->GetStart() <= rSelection.GetStart().GetIndex()
                            && pBeginAttrib->GetEnd() >= rSelection.GetEnd().GetIndex()))
    {
        if(pBeginAttrib->GetStart())
            nRet |= MOVE_ITEM_LEFT;
        //if there is an entry it can always be move to the right and down
        nRet |= MOVE_ITEM_RIGHT|MOVE_ITEM_DOWN;
        if(rSelection.GetStart().GetPara() > 0)
            nRet |= MOVE_ITEM_UP;
    }
    return nRet;
}

bool AddressMultiLineEdit::HasCurrentItem()
{
    ExtTextEngine* pTextEngine = GetTextEngine();
    ExtTextView* pTextView = GetTextView();
    const TextSelection& rSelection = pTextView->GetSelection();
    const TextCharAttrib* pBeginAttrib = pTextEngine->FindCharAttrib( rSelection.GetStart(), TEXTATTR_PROTECTED );
    return (pBeginAttrib &&
            (pBeginAttrib->GetStart() <= rSelection.GetStart().GetIndex()
                            && pBeginAttrib->GetEnd() >= rSelection.GetEnd().GetIndex()));
}

OUString AddressMultiLineEdit::GetCurrentItem()
{
    OUString sRet;
    ExtTextEngine* pTextEngine = GetTextEngine();
    ExtTextView* pTextView = GetTextView();
    const TextSelection& rSelection = pTextView->GetSelection();
    const TextCharAttrib* pBeginAttrib = pTextEngine->FindCharAttrib( rSelection.GetStart(), TEXTATTR_PROTECTED );
    if(pBeginAttrib &&
            (pBeginAttrib->GetStart() <= rSelection.GetStart().GetIndex()
                            && pBeginAttrib->GetEnd() >= rSelection.GetEnd().GetIndex()))
    {
        sal_uLong nPara = rSelection.GetStart().GetPara();
        TextSelection aEntrySel(TextPaM( nPara, pBeginAttrib->GetStart()), TextPaM(nPara, pBeginAttrib->GetEnd()));
        sRet = pTextEngine->GetText( aEntrySel );
    }
    return sRet;
}

void AddressMultiLineEdit::SelectCurrentItem()
{
    ExtTextEngine* pTextEngine = GetTextEngine();
    ExtTextView* pTextView = GetTextView();
    const TextSelection& rSelection = pTextView->GetSelection();
    const TextCharAttrib* pBeginAttrib = pTextEngine->FindCharAttrib( rSelection.GetStart(), TEXTATTR_PROTECTED );
    if(pBeginAttrib &&
            (pBeginAttrib->GetStart() <= rSelection.GetStart().GetIndex()
                            && pBeginAttrib->GetEnd() >= rSelection.GetEnd().GetIndex()))
    {
        sal_uLong nPara = rSelection.GetStart().GetPara();
        TextSelection aEntrySel(TextPaM( nPara, pBeginAttrib->GetStart()), TextPaM(nPara, pBeginAttrib->GetEnd()));
        pTextView->SetSelection(aEntrySel);
        Invalidate();
    }
}

OUString AddressMultiLineEdit::GetAddress()
{
    OUString sRet;
    ExtTextEngine* pTextEngine = GetTextEngine();
    sal_uLong  nParaCount = pTextEngine->GetParagraphCount();
    for(sal_uLong nPara = nParaCount; nPara; --nPara)
    {
        OUString sPara = comphelper::string::stripEnd(pTextEngine->GetText(nPara - 1), ' ');
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
