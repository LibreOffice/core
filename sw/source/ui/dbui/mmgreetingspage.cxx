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

#include <mmgreetingspage.hxx>
#include <mailmergewizard.hxx>
#include <mmconfigitem.hxx>
#include <mmaddressblockpage.hxx>
#include <swtypes.hxx>
#include <vcl/msgbox.hxx>
#include <mmgreetingspage.hrc>
#include <dbui.hrc>
#include <com/sun/star/sdb/XColumn.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <helpid.h>

#include <boost/scoped_ptr.hpp>

using namespace svt;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

static void lcl_FillGreetingsBox(ListBox& rBox,
                        SwMailMergeConfigItem& rConfig,
                        SwMailMergeConfigItem::Gender eType)
{
    const Sequence< OUString> rEntries = rConfig.GetGreetings(eType);
    for(sal_Int32 nEntry = 0; nEntry < rEntries.getLength(); ++nEntry)
        rBox.InsertEntry(rEntries[nEntry]);
    rBox.SelectEntryPos((sal_uInt16)rConfig.GetCurrentGreeting(eType));
}

static void lcl_FillGreetingsBox(ComboBox& rBox,
                        SwMailMergeConfigItem& rConfig,
                        SwMailMergeConfigItem::Gender eType)
{
    const Sequence< OUString> rEntries = rConfig.GetGreetings(eType);
    for(sal_Int32 nEntry = 0; nEntry < rEntries.getLength(); ++nEntry)
        rBox.InsertEntry(rEntries[nEntry]);
    rBox.SelectEntryPos((sal_uInt16)rConfig.GetCurrentGreeting(eType));
}

static void lcl_StoreGreetingsBox(ListBox& rBox,
                        SwMailMergeConfigItem& rConfig,
                        SwMailMergeConfigItem::Gender eType)
{
    Sequence< OUString> aEntries(rBox.GetEntryCount());
    OUString* pEntries = aEntries.getArray();
    for(sal_Int32 nEntry = 0; nEntry < rBox.GetEntryCount(); ++nEntry)
        pEntries[nEntry] = rBox.GetEntry(nEntry);
    rConfig.SetGreetings(eType, aEntries);
    rConfig.SetCurrentGreeting(eType, rBox.GetSelectEntryPos());
}

static void lcl_StoreGreetingsBox(ComboBox& rBox,
                        SwMailMergeConfigItem& rConfig,
                        SwMailMergeConfigItem::Gender eType)
{
    Sequence< OUString> aEntries(rBox.GetEntryCount());
    OUString* pEntries = aEntries.getArray();
    for(sal_Int32 nEntry = 0; nEntry < rBox.GetEntryCount(); ++nEntry)
        pEntries[nEntry] = rBox.GetEntry(nEntry);
    rConfig.SetGreetings(eType, aEntries);
    rConfig.SetCurrentGreeting(eType, rBox.GetSelectEntryPos());
}

IMPL_LINK_NOARG(SwGreetingsHandler, IndividualHdl_Impl)
{
    sal_Bool bIndividual = m_pPersonalizedCB->IsEnabled() && m_pPersonalizedCB->IsChecked();
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

    if( m_bIsTabPage )
    {
        m_pWizard->GetConfigItem().SetIndividualGreeting(bIndividual, sal_False);
        m_pWizard->UpdateRoadmap();
        m_pWizard->enableButtons(WZB_NEXT, m_pWizard->isStateEnabled(MM_PREPAREMERGEPAGE));
    }
    UpdatePreview();
    return 0;
}

IMPL_LINK(SwGreetingsHandler, GreetingHdl_Impl, PushButton*, pButton)
{
    boost::scoped_ptr<SwCustomizeAddressBlockDialog> pDlg(
            new SwCustomizeAddressBlockDialog(pButton, m_pWizard->GetConfigItem(),
                        pButton == m_pMalePB ?
                        SwCustomizeAddressBlockDialog::GREETING_MALE :
                        SwCustomizeAddressBlockDialog::GREETING_FEMALE ));
    if(RET_OK == pDlg->Execute())
    {
        ListBox* pToInsert = pButton == m_pMalePB ? m_pMaleLB : m_pFemaleLB;
        pToInsert->SelectEntryPos(pToInsert->InsertEntry(pDlg->GetAddress()));
        if(m_bIsTabPage)
        {
            m_pWizard->UpdateRoadmap();
            m_pWizard->enableButtons(WZB_NEXT, m_pWizard->isStateEnabled(MM_PREPAREMERGEPAGE));
        }
        UpdatePreview();
    }
    return 0;
}

void    SwGreetingsHandler::UpdatePreview()
{
    //the base class does nothing
}

IMPL_LINK(SwMailMergeGreetingsPage, AssignHdl_Impl, PushButton*, pButton)
{
    OUString sPreview = m_pFemaleLB->GetSelectEntry();
    sPreview += "\n";
    sPreview += m_pMaleLB->GetSelectEntry();
    boost::scoped_ptr<SwAssignFieldsDialog> pDlg(
            new SwAssignFieldsDialog(pButton, m_pWizard->GetConfigItem(), sPreview, false));
    if(RET_OK == pDlg->Execute())
    {
        UpdatePreview();
        m_pWizard->UpdateRoadmap();
        m_pWizard->enableButtons(WZB_NEXT, m_pWizard->isStateEnabled(MM_PREPAREMERGEPAGE));
    }
    return 0;
}

IMPL_LINK_NOARG(SwMailMergeGreetingsPage, GreetingSelectHdl_Impl)
{
    UpdatePreview();
    return 0;
}

void SwMailMergeGreetingsPage::UpdatePreview()
{
    //find out which type of greeting should be selected:
    bool bFemale = false;
    bool bNoValue = !m_pFemaleColumnLB->IsEnabled();
    if( !bNoValue )
    {
        OUString sFemaleValue = m_pFemaleFieldCB->GetText();
        OUString sFemaleColumn = m_pFemaleColumnLB->GetSelectEntry();
        Reference< sdbcx::XColumnsSupplier > xColsSupp( m_pWizard->GetConfigItem().GetResultSet(), UNO_QUERY);
        Reference < container::XNameAccess> xColAccess = xColsSupp.is() ? xColsSupp->getColumns() : 0;
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
                    OUString sFemaleColumnValue = xColumn->getString();
                    bFemale = sFemaleColumnValue == sFemaleValue;
                    //bNoValue = !sFemaleColumnValue.getLength();
                    if( !bNoValue )
                    {
                        //no last name value marks the greeting also als neutral
                        SwMailMergeConfigItem& rConfig = m_pWizard->GetConfigItem();
                        OUString sLastNameColumn = rConfig.GetAssignedColumn(MM_PART_LASTNAME);
                        if ( xColAccess->hasByName(sLastNameColumn) )
                        {
                            aCol = xColAccess->getByName(sLastNameColumn);
                            aCol >>= xColumn;
                            OUString sLastNameColumnValue = xColumn->getString();
                            bNoValue = sLastNameColumnValue.isEmpty();
                        }
                    }
                }
                catch (const sdbc::SQLException&)
                {
                    OSL_FAIL("SQLException caught");
                }
            }
        }
    }

    OUString sPreview = bFemale ? OUString(m_pFemaleLB->GetSelectEntry()) :
        bNoValue ? m_pNeutralCB->GetText() : OUString(m_pMaleLB->GetSelectEntry());

    sPreview = SwAddressPreview::FillData(sPreview, m_pWizard->GetConfigItem());
    m_pPreviewWIN->SetAddress(sPreview);
}

void    SwGreetingsHandler::Contains(sal_Bool bContainsGreeting)
{
    m_pPersonalizedCB->Enable(bContainsGreeting);
    sal_Bool bEnablePersonal = bContainsGreeting && m_pPersonalizedCB->IsChecked();
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
        "modules/swriter/ui/mmsalutationpage.ui")
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
    Size aSize(LogicToPixel(Size(186, 21), MapMode(MAP_APPFONT)));
    m_pPreviewWIN->set_width_request(aSize.Width());
    m_pPreviewWIN->set_height_request(aSize.Height());
    get(m_pAssignPB, "assign");
    get(m_pDocumentIndexFI, "documentindex");
    m_sDocument = m_pDocumentIndexFI->GetText();
    get(m_pPrevSetIB, "prev");
    get(m_pNextSetIB, "next");

    m_bIsTabPage = true;

    m_pGreetingLineCB->SetClickHdl(LINK(this, SwMailMergeGreetingsPage, ContainsHdl_Impl));
    Link aIndividualLink = LINK(this, SwGreetingsHandler, IndividualHdl_Impl);
    m_pPersonalizedCB->SetClickHdl(aIndividualLink);
    Link aGreetingLink = LINK(this, SwGreetingsHandler, GreetingHdl_Impl);
    m_pFemalePB->SetClickHdl(aGreetingLink);
    m_pMalePB->SetClickHdl(aGreetingLink);
    m_pAssignPB->SetClickHdl(LINK(this, SwMailMergeGreetingsPage, AssignHdl_Impl));
    Link aLBoxLink = LINK(this, SwMailMergeGreetingsPage, GreetingSelectHdl_Impl);
    m_pFemaleLB->SetSelectHdl(aLBoxLink);
    m_pMaleLB->SetSelectHdl(aLBoxLink);
    m_pFemaleColumnLB->SetSelectHdl(aLBoxLink);
    m_pFemaleFieldCB->SetSelectHdl(aLBoxLink);
    m_pFemaleFieldCB->SetModifyHdl(aLBoxLink);
    m_pNeutralCB->SetSelectHdl(aLBoxLink);
    m_pNeutralCB->SetModifyHdl(aLBoxLink);

    Link aDataLink = LINK(this, SwMailMergeGreetingsPage, InsertDataHdl_Impl);
    m_pPrevSetIB->SetClickHdl(aDataLink);
    m_pNextSetIB->SetClickHdl(aDataLink);

    SwMailMergeConfigItem& rConfig = m_pWizard->GetConfigItem();
    m_pGreetingLineCB->Check(rConfig.IsGreetingLine(sal_False));
    m_pPersonalizedCB->Check(rConfig.IsIndividualGreeting(sal_False));
    ContainsHdl_Impl(m_pGreetingLineCB);
    aIndividualLink.Call(0);

    lcl_FillGreetingsBox(*m_pFemaleLB, rConfig, SwMailMergeConfigItem::FEMALE);
    lcl_FillGreetingsBox(*m_pMaleLB, rConfig, SwMailMergeConfigItem::MALE);
    lcl_FillGreetingsBox(*m_pNeutralCB, rConfig, SwMailMergeConfigItem::NEUTRAL);

    m_pDocumentIndexFI->SetText(m_sDocument.replaceFirst("%1", OUString::number(1)));
}

SwMailMergeGreetingsPage::~SwMailMergeGreetingsPage()
{
}

void SwMailMergeGreetingsPage::ActivatePage()
{
    SwMailMergeConfigItem& rConfig = m_pWizard->GetConfigItem();

    //try to find the gender setting
    m_pFemaleColumnLB->Clear();
    Reference< sdbcx::XColumnsSupplier > xColsSupp = rConfig.GetColumnsSupplier();
    if(xColsSupp.is())
    {
        Reference < container::XNameAccess> xColAccess = xColsSupp->getColumns();
        Sequence< OUString > aColumns = xColAccess->getElementNames();
        for(sal_Int32 nName = 0; nName < aColumns.getLength(); ++nName)
            m_pFemaleColumnLB->InsertEntry(aColumns[nName]);
    }

    OUString sGenderColumn = rConfig.GetAssignedColumn(MM_PART_GENDER);
    m_pFemaleColumnLB->SelectEntry(sGenderColumn);
    m_pFemaleColumnLB->SaveValue();

    m_pFemaleFieldCB->SetText(rConfig.GetFemaleGenderValue());
    m_pFemaleFieldCB->SaveValue();

    UpdatePreview();
    m_pWizard->enableButtons(WZB_NEXT, m_pWizard->isStateEnabled(MM_PREPAREMERGEPAGE));
}

bool SwMailMergeGreetingsPage::commitPage( ::svt::WizardTypes::CommitPageReason )
{
    SwMailMergeConfigItem& rConfig = m_pWizard->GetConfigItem();

    if (m_pFemaleColumnLB->GetSelectEntryPos() != m_pFemaleColumnLB->GetSavedValue())
    {
        const SwDBData& rDBData = rConfig.GetCurrentDBData();
        Sequence< OUString> aAssignment = rConfig.GetColumnAssignment( rDBData );
        if(aAssignment.getLength() <= MM_PART_GENDER)
            aAssignment.realloc(MM_PART_GENDER + 1);
        aAssignment[MM_PART_GENDER] = m_pFemaleColumnLB->GetSelectEntry();
        rConfig.SetColumnAssignment( rDBData, aAssignment );
    }
    if (m_pFemaleFieldCB->GetText() != m_pFemaleFieldCB->GetSavedValue())
        rConfig.SetFemaleGenderValue(m_pFemaleFieldCB->GetText());

    lcl_StoreGreetingsBox(*m_pFemaleLB, rConfig, SwMailMergeConfigItem::FEMALE);
    lcl_StoreGreetingsBox(*m_pMaleLB, rConfig, SwMailMergeConfigItem::MALE);

    sal_Int32 nCurrentTextPos = m_pNeutralCB->GetEntryPos(m_pNeutralCB->GetText());
    if(COMBOBOX_ENTRY_NOTFOUND == nCurrentTextPos)
    {
        sal_Int32 nCount = m_pNeutralCB->GetEntryCount();
        m_pNeutralCB->InsertEntry(m_pNeutralCB->GetText(), nCount);
        m_pNeutralCB->SelectEntryPos(nCount);
    }
    lcl_StoreGreetingsBox(*m_pNeutralCB, rConfig, SwMailMergeConfigItem::NEUTRAL);
    rConfig.SetGreetingLine(m_pGreetingLineCB->IsChecked(), sal_False);
    rConfig.SetIndividualGreeting(m_pPersonalizedCB->IsChecked(), sal_False);
    return true;
}

IMPL_LINK(SwMailMergeGreetingsPage, ContainsHdl_Impl, CheckBox*, pBox)
{
    sal_Bool bContainsGreeting = pBox->IsChecked();
    SwGreetingsHandler::Contains(bContainsGreeting);
    m_pPreviewFI-> Enable(bContainsGreeting);
    m_pPreviewWIN->Enable(bContainsGreeting);
    m_pAssignPB->  Enable(bContainsGreeting);
    m_pDocumentIndexFI->  Enable(bContainsGreeting);
    m_pPrevSetIB->Enable(bContainsGreeting);
    m_pNextSetIB->Enable(bContainsGreeting);
    SwMailMergeConfigItem& rConfig = m_pWizard->GetConfigItem();
    rConfig.SetGreetingLine(m_pGreetingLineCB->IsChecked(), sal_False);
    m_pWizard->UpdateRoadmap();
    return 0;
}

IMPL_LINK(SwMailMergeGreetingsPage, InsertDataHdl_Impl, ImageButton*, pButton)
{
    //if no pButton is given, the first set has to be pre-set
    SwMailMergeConfigItem& rConfig = m_pWizard->GetConfigItem();
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
    sal_Int32 nPos = rConfig.GetResultSetPosition();
    sal_Bool bEnable = sal_True;
    if(nPos < 1)
    {
        bEnable = sal_False;
        nPos = 1;
    }
    else
        UpdatePreview();
    m_pPrevSetIB->Enable(bEnable);
    m_pNextSetIB->Enable(bEnable);
    m_pDocumentIndexFI->Enable(bEnable);
    m_pDocumentIndexFI->SetText(m_sDocument.replaceFirst("%1", OUString::number(nPos)));
    return 0;
}

SwMailBodyDialog::SwMailBodyDialog(Window* pParent, SwMailMergeWizard* _pWizard) :
    SfxModalDialog(pParent, SW_RES(DLG_MM_MAILBODY)),
    m_aGreetingLineCB(this, SW_RES(    CB_GREETINGLINE ) ),
    m_aPersonalizedCB(this, SW_RES(      CB_PERSONALIZED ) ),
    m_aFemaleFT(this, SW_RES(            FT_FEMALE   ) ),
    m_aFemaleLB(this, SW_RES(            LB_FEMALE   ) ),
    m_aFemalePB(this, SW_RES(            PB_FEMALE   ) ),
    m_aMaleFT(this, SW_RES(              FT_MALE     ) ),
    m_aMaleLB(this, SW_RES(              LB_MALE     ) ),
    m_aMalePB(this, SW_RES(              PB_MALE     ) ),
    m_aFemaleFI(this, SW_RES(            FI_FEMALE      ) ),
    m_aFemaleColumnFT(this, SW_RES(      FT_FEMALECOLUMN ) ),
    m_aFemaleColumnLB(this, SW_RES(      LB_FEMALECOLUMN ) ),
    m_aFemaleFieldFT(this, SW_RES(       FT_FEMALEFIELD  ) ),
    m_aFemaleFieldCB(this, SW_RES(       CB_FEMALEFIELD  ) ),
    m_aNeutralFT(this, SW_RES(           FT_NEUTRAL      ) ),
    m_aNeutralCB(this, SW_RES(         CB_NEUTRAL      ) ),
    m_aBodyFT(   this, SW_RES(         FT_BODY         ) ),
    m_aBodyMLE(   this, SW_RES(        MLE_BODY        ) ),
    m_aSeparatorFL(   this, SW_RES(    FL_SEPARATOR    ) ),
    m_aOK(   this, SW_RES(             PB_OK           ) ),
    m_aCancel(   this, SW_RES(         PB_CANCEL       ) ),
    m_aHelp(   this, SW_RES(           PB_HELP         ) )
{
    m_pWizard = _pWizard;
    m_pGreetingLineCB = &m_aGreetingLineCB;
    m_pPersonalizedCB = &m_aPersonalizedCB;
    m_pFemaleFT = &      m_aFemaleFT;
    m_pFemaleLB = &      m_aFemaleLB;
    m_pFemalePB = &      m_aFemalePB;
    m_pMaleFT = &        m_aMaleFT;
    m_pMaleLB = &        m_aMaleLB;
    m_pMalePB = &        m_aMalePB;
    m_pFemaleFI = &      m_aFemaleFI;
    m_pFemaleColumnFT = &m_aFemaleColumnFT;
    m_pFemaleColumnLB = &m_aFemaleColumnLB;
    m_pFemaleFieldFT = & m_aFemaleFieldFT;
    m_pFemaleFieldCB = & m_aFemaleFieldCB;
    m_pNeutralFT = &     m_aNeutralFT;
    m_pNeutralCB    = &m_aNeutralCB;
    m_bIsTabPage = false;

    m_pPersonalizedCB->SetHelpId(   HID_MM_BODY_CB_PERSONALIZED     );
    m_pFemaleLB->SetHelpId(         HID_MM_BODY_LB_FEMALE           );
    m_pFemalePB->SetHelpId(         HID_MM_BODY_PB_FEMALE           );
    m_pMaleLB->SetHelpId(           HID_MM_BODY_LB_MALE             );
    m_pMalePB->SetHelpId(           HID_MM_BODY_PB_MALE             );
    m_pFemaleColumnLB->SetHelpId(   HID_MM_BODY_LB_FEMALECOLUMN     );
    m_pFemaleFieldCB->SetHelpId(    HID_MM_BODY_CB_FEMALEFIELD      );
    m_pNeutralCB->SetHelpId(        HID_MM_BODY_CB_NEUTRAL          );

    FreeResource();
    m_aGreetingLineCB.SetClickHdl(LINK(this, SwMailBodyDialog, ContainsHdl_Impl));
    Link aIndividualLink = LINK(this, SwGreetingsHandler, IndividualHdl_Impl);
    m_aPersonalizedCB.SetClickHdl(aIndividualLink);
    Link aGreetingLink = LINK(this, SwGreetingsHandler, GreetingHdl_Impl);
    m_aFemalePB.SetClickHdl(aGreetingLink);
    m_aMalePB.SetClickHdl(aGreetingLink);
    m_aOK.SetClickHdl(LINK(this, SwMailBodyDialog, OKHdl));

    SwMailMergeConfigItem& rConfig = m_pWizard->GetConfigItem();
    m_aGreetingLineCB.Check(rConfig.IsGreetingLine(sal_True));
    m_aPersonalizedCB.Check(rConfig.IsIndividualGreeting(sal_True));
    ContainsHdl_Impl(&m_aGreetingLineCB);
    aIndividualLink.Call(0);

    lcl_FillGreetingsBox(m_aFemaleLB, rConfig, SwMailMergeConfigItem::FEMALE);
    lcl_FillGreetingsBox(m_aMaleLB, rConfig, SwMailMergeConfigItem::MALE);
    lcl_FillGreetingsBox(m_aNeutralCB, rConfig, SwMailMergeConfigItem::NEUTRAL);

    //try to find the gender setting
    m_aFemaleColumnLB.Clear();
    Reference< sdbcx::XColumnsSupplier > xColsSupp = rConfig.GetColumnsSupplier();
    if(xColsSupp.is())
    {
        Reference < container::XNameAccess> xColAccess = xColsSupp->getColumns();
        Sequence< OUString > aColumns = xColAccess->getElementNames();
        for(sal_Int32 nName = 0; nName < aColumns.getLength(); ++nName)
            m_aFemaleColumnLB.InsertEntry(aColumns[nName]);
    }

    OUString sGenderColumn = rConfig.GetAssignedColumn(MM_PART_GENDER);
    m_aFemaleColumnLB.SelectEntry(sGenderColumn);
    m_aFemaleColumnLB.SaveValue();

    m_aFemaleFieldCB.SetText(rConfig.GetFemaleGenderValue());
    m_aFemaleFieldCB.SaveValue();
}

SwMailBodyDialog::~SwMailBodyDialog()
{
}

IMPL_LINK(SwMailBodyDialog, ContainsHdl_Impl, CheckBox*, pBox)
{
    SwGreetingsHandler::Contains(pBox->IsChecked());
    m_pWizard->GetConfigItem().SetGreetingLine(pBox->IsChecked(), sal_True);
    return 0;
}

IMPL_LINK_NOARG(SwMailBodyDialog, OKHdl)
{
    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
    rConfigItem.SetGreetingLine(
                m_aGreetingLineCB.IsChecked(), sal_False);
    rConfigItem.SetIndividualGreeting(
                m_aPersonalizedCB.IsChecked(), sal_False);

    if(m_aFemaleColumnLB.GetSelectEntryPos() != m_aFemaleColumnLB.GetSavedValue())
    {
        const SwDBData& rDBData = rConfigItem.GetCurrentDBData();
        Sequence< OUString> aAssignment = rConfigItem.GetColumnAssignment( rDBData );
        sal_Int32 nPos = m_aFemaleColumnLB.GetSelectEntryPos();
        if(aAssignment.getLength() < MM_PART_GENDER)
            aAssignment.realloc(MM_PART_GENDER);
        if( nPos > 0 )
            aAssignment[MM_PART_GENDER] = m_aFemaleColumnLB.GetSelectEntry();
        else
            aAssignment[MM_PART_GENDER] = OUString();
        rConfigItem.SetColumnAssignment( rDBData, aAssignment );
    }
    if(m_aFemaleFieldCB.GetText() != m_aFemaleFieldCB.GetSavedValue())
        rConfigItem.SetFemaleGenderValue(m_aFemaleFieldCB.GetText());

    EndDialog(RET_OK);
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
