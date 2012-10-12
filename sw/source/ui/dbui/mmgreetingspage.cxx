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

using namespace svt;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

static void lcl_FillGreetingsBox(ListBox& rBox,
                        SwMailMergeConfigItem& rConfig,
                        SwMailMergeConfigItem::Gender eType)
{
    const Sequence< ::rtl::OUString> rEntries = rConfig.GetGreetings(eType);
    for(sal_Int32 nEntry = 0; nEntry < rEntries.getLength(); ++nEntry)
        rBox.InsertEntry(rEntries[nEntry]);
    rBox.SelectEntryPos((sal_uInt16)rConfig.GetCurrentGreeting(eType));
}

static void lcl_FillGreetingsBox(ComboBox& rBox,
                        SwMailMergeConfigItem& rConfig,
                        SwMailMergeConfigItem::Gender eType)
{
    const Sequence< ::rtl::OUString> rEntries = rConfig.GetGreetings(eType);
    for(sal_Int32 nEntry = 0; nEntry < rEntries.getLength(); ++nEntry)
        rBox.InsertEntry(rEntries[nEntry]);
    rBox.SelectEntryPos((sal_uInt16)rConfig.GetCurrentGreeting(eType));
}

static void lcl_StoreGreetingsBox(ListBox& rBox,
                        SwMailMergeConfigItem& rConfig,
                        SwMailMergeConfigItem::Gender eType)
{
    Sequence< ::rtl::OUString> aEntries(rBox.GetEntryCount());
    ::rtl::OUString* pEntries = aEntries.getArray();
    for(sal_uInt16 nEntry = 0; nEntry < rBox.GetEntryCount(); ++nEntry)
        pEntries[nEntry] = rBox.GetEntry(nEntry);
    rConfig.SetGreetings(eType, aEntries);
    rConfig.SetCurrentGreeting(eType, rBox.GetSelectEntryPos());
}

static void lcl_StoreGreetingsBox(ComboBox& rBox,
                        SwMailMergeConfigItem& rConfig,
                        SwMailMergeConfigItem::Gender eType)
{
    Sequence< ::rtl::OUString> aEntries(rBox.GetEntryCount());
    ::rtl::OUString* pEntries = aEntries.getArray();
    for(sal_uInt16 nEntry = 0; nEntry < rBox.GetEntryCount(); ++nEntry)
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
    SwCustomizeAddressBlockDialog* pDlg =
            new SwCustomizeAddressBlockDialog(pButton, m_pWizard->GetConfigItem(),
                        pButton == m_pMalePB ?
                        SwCustomizeAddressBlockDialog::GREETING_MALE :
                        SwCustomizeAddressBlockDialog::GREETING_FEMALE );
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
    delete pDlg;
    return 0;
}

void    SwGreetingsHandler::UpdatePreview()
{
    //the base class does nothing
}

IMPL_LINK(SwMailMergeGreetingsPage, AssignHdl_Impl, PushButton*, pButton)
{
    String sPreview = m_aFemaleLB.GetSelectEntry();
    sPreview += '\n';
    sPreview += m_aMaleLB.GetSelectEntry();
    SwAssignFieldsDialog* pDlg =
            new SwAssignFieldsDialog(pButton, m_pWizard->GetConfigItem(), sPreview, false);
    if(RET_OK == pDlg->Execute())
    {
        UpdatePreview();
        m_pWizard->UpdateRoadmap();
        m_pWizard->enableButtons(WZB_NEXT, m_pWizard->isStateEnabled(MM_PREPAREMERGEPAGE));
    }
    delete pDlg;
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
        ::rtl::OUString sFemaleValue = m_aFemaleFieldCB.GetText();
        ::rtl::OUString sFemaleColumn = m_aFemaleColumnLB.GetSelectEntry();
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
                    ::rtl::OUString sFemaleColumnValue = xColumn->getString();
                    bFemale = sFemaleColumnValue == sFemaleValue;
                    //bNoValue = !sFemaleColumnValue.getLength();
                    if( !bNoValue )
                    {
                        //no last name value marks the greeting also als neutral
                        SwMailMergeConfigItem& rConfig = m_pWizard->GetConfigItem();
                        ::rtl::OUString sLastNameColumn = rConfig.GetAssignedColumn(MM_PART_LASTNAME);
                        if ( xColAccess->hasByName(sLastNameColumn) )
                        {
                            aCol = xColAccess->getByName(sLastNameColumn);
                            aCol >>= xColumn;
                            ::rtl::OUString sLastNameColumnValue = xColumn->getString();
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

    String sPreview = bFemale ? m_aFemaleLB.GetSelectEntry() :
        bNoValue ? m_aNeutralCB.GetText() : m_aMaleLB.GetSelectEntry();

    sPreview = SwAddressPreview::FillData(sPreview, m_pWizard->GetConfigItem());
    m_aPreviewWIN.SetAddress(sPreview);
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

SwMailMergeGreetingsPage::SwMailMergeGreetingsPage( SwMailMergeWizard* _pParent) :
    svt::OWizardPage(_pParent, SW_RES(DLG_MM_GREETINGS_PAGE)),
#ifdef MSC
#pragma warning (disable : 4355)
#endif
    m_aHeaderFI(this, SW_RES(          FI_HEADER     ) ),
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
    m_aPreviewFI(       this, SW_RES( FI_PREVIEW     ) ),
    m_aPreviewWIN(      this, SW_RES( WIN_PREVIEW    ) ),
    m_aAssignPB(        this, SW_RES( PB_ASSIGN      ) ),
    m_aDocumentIndexFI( this, SW_RES( FI_DOCINDEX    ) ),
    m_aPrevSetIB(       this, SW_RES( IB_PREVSET     ) ),
    m_aNextSetIB(       this, SW_RES( IB_NEXTSET     ) ),
    m_sDocument(        SW_RES(       STR_DOCUMENT  ) )
#ifdef MSC
#pragma warning (default : 4355)
#endif
{
    m_pWizard = _pParent;
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
    m_bIsTabPage = true;

    m_pPersonalizedCB->SetHelpId(   HID_MM_GREETINGS_CB_PERSONALIZED);
    m_pFemaleLB->SetHelpId(         HID_MM_GREETINGS_LB_FEMALE      );
    m_pFemalePB->SetHelpId(         HID_MM_GREETINGS_PB_FEMALE      );
    m_pMaleLB->SetHelpId(           HID_MM_GREETINGS_LB_MALE        );
    m_pMalePB->SetHelpId(           HID_MM_GREETINGS_PB_MALE        );
    m_pFemaleColumnLB->SetHelpId(   HID_MM_GREETINGS_LB_FEMALECOLUMN);
    m_pFemaleFieldCB->SetHelpId(    HID_MM_GREETINGS_CB_FEMALEFIELD );
    m_pNeutralCB->SetHelpId(        HID_MM_GREETINGS_CB_NEUTRAL     );

    FreeResource();
    m_aGreetingLineCB.SetClickHdl(LINK(this, SwMailMergeGreetingsPage, ContainsHdl_Impl));
    Link aIndividualLink = LINK(this, SwGreetingsHandler, IndividualHdl_Impl);
    m_aPersonalizedCB.SetClickHdl(aIndividualLink);
    Link aGreetingLink = LINK(this, SwGreetingsHandler, GreetingHdl_Impl);
    m_aFemalePB.SetClickHdl(aGreetingLink);
    m_aMalePB.SetClickHdl(aGreetingLink);
    m_aAssignPB.SetClickHdl(LINK(this, SwMailMergeGreetingsPage, AssignHdl_Impl));
    Link aLBoxLink = LINK(this, SwMailMergeGreetingsPage, GreetingSelectHdl_Impl);
    m_aFemaleLB.SetSelectHdl(aLBoxLink);
    m_aMaleLB.SetSelectHdl(aLBoxLink);
    m_aFemaleColumnLB.SetSelectHdl(aLBoxLink);
    m_aFemaleFieldCB.SetSelectHdl(aLBoxLink);
    m_aFemaleFieldCB.SetModifyHdl(aLBoxLink);
    m_aNeutralCB.SetSelectHdl(aLBoxLink);
    m_aNeutralCB.SetModifyHdl(aLBoxLink);

    Link aDataLink = LINK(this, SwMailMergeGreetingsPage, InsertDataHdl_Impl);
    m_aPrevSetIB.SetClickHdl(aDataLink);
    m_aNextSetIB.SetClickHdl(aDataLink);


    SwMailMergeConfigItem& rConfig = m_pWizard->GetConfigItem();
    m_aGreetingLineCB.Check(rConfig.IsGreetingLine(sal_False));
    m_aPersonalizedCB.Check(rConfig.IsIndividualGreeting(sal_False));
    ContainsHdl_Impl(&m_aGreetingLineCB);
    aIndividualLink.Call(0);

    lcl_FillGreetingsBox(m_aFemaleLB, rConfig, SwMailMergeConfigItem::FEMALE);
    lcl_FillGreetingsBox(m_aMaleLB, rConfig, SwMailMergeConfigItem::MALE);
    lcl_FillGreetingsBox(m_aNeutralCB, rConfig, SwMailMergeConfigItem::NEUTRAL);

    String sTemp(m_sDocument);
    sTemp.SearchAndReplaceAscii("%1", String::CreateFromInt32(1));
    m_aDocumentIndexFI.SetText(sTemp);
}

SwMailMergeGreetingsPage::~SwMailMergeGreetingsPage()
{
}

void SwMailMergeGreetingsPage::ActivatePage()
{
    SwMailMergeConfigItem& rConfig = m_pWizard->GetConfigItem();

    //try to find the gender setting
    m_aFemaleColumnLB.Clear();
    Reference< sdbcx::XColumnsSupplier > xColsSupp = rConfig.GetColumnsSupplier();
    if(xColsSupp.is())
    {
        Reference < container::XNameAccess> xColAccess = xColsSupp->getColumns();
        Sequence< ::rtl::OUString > aColumns = xColAccess->getElementNames();
        for(sal_Int32 nName = 0; nName < aColumns.getLength(); ++nName)
            m_aFemaleColumnLB.InsertEntry(aColumns[nName]);
    }

    ::rtl::OUString sGenderColumn = rConfig.GetAssignedColumn(MM_PART_GENDER);
    m_aFemaleColumnLB.SelectEntry(sGenderColumn);
    m_aFemaleColumnLB.SaveValue();

    m_aFemaleFieldCB.SetText(rConfig.GetFemaleGenderValue());
    m_aFemaleFieldCB.SaveValue();

    UpdatePreview();
    m_pWizard->enableButtons(WZB_NEXT, m_pWizard->isStateEnabled(MM_PREPAREMERGEPAGE));
}

sal_Bool    SwMailMergeGreetingsPage::commitPage( ::svt::WizardTypes::CommitPageReason )
{
    SwMailMergeConfigItem& rConfig = m_pWizard->GetConfigItem();

    if(m_aFemaleColumnLB.GetSelectEntryPos() != m_aFemaleColumnLB.GetSavedValue())
    {
        const SwDBData& rDBData = rConfig.GetCurrentDBData();
        Sequence< ::rtl::OUString> aAssignment = rConfig.GetColumnAssignment( rDBData );
        if(aAssignment.getLength() <= MM_PART_GENDER)
            aAssignment.realloc(MM_PART_GENDER + 1);
        aAssignment[MM_PART_GENDER] = m_aFemaleColumnLB.GetSelectEntry();
        rConfig.SetColumnAssignment( rDBData, aAssignment );
    }
    if(m_aFemaleFieldCB.GetText() != m_aFemaleFieldCB.GetSavedValue())
        rConfig.SetFemaleGenderValue(m_aFemaleFieldCB.GetText());

    lcl_StoreGreetingsBox(m_aFemaleLB, rConfig, SwMailMergeConfigItem::FEMALE);
    lcl_StoreGreetingsBox(m_aMaleLB, rConfig, SwMailMergeConfigItem::MALE);

    sal_uInt16 nCurrentTextPos = m_aNeutralCB.GetEntryPos( m_aNeutralCB.GetText() );
    if(LISTBOX_ENTRY_NOTFOUND == nCurrentTextPos)
    {
        sal_uInt16 nCount = m_aNeutralCB.GetEntryCount();
        m_aNeutralCB.InsertEntry( m_aNeutralCB.GetText(), nCount );
        m_aNeutralCB.SelectEntryPos(nCount);
    }
    lcl_StoreGreetingsBox(m_aNeutralCB, rConfig, SwMailMergeConfigItem::NEUTRAL);
    rConfig.SetGreetingLine(m_aGreetingLineCB.IsChecked(), sal_False);
    rConfig.SetIndividualGreeting(m_aPersonalizedCB.IsChecked(), sal_False);
    return sal_True;
}

IMPL_LINK(SwMailMergeGreetingsPage, ContainsHdl_Impl, CheckBox*, pBox)
{
    sal_Bool bContainsGreeting = pBox->IsChecked();
    SwGreetingsHandler::Contains(bContainsGreeting);
    m_aPreviewFI. Enable(bContainsGreeting);
    m_aPreviewWIN.Enable(bContainsGreeting);
    m_aAssignPB.  Enable(bContainsGreeting);
    m_aDocumentIndexFI.  Enable(bContainsGreeting);
    m_aPrevSetIB.Enable(bContainsGreeting);
    m_aNextSetIB.Enable(bContainsGreeting);
    SwMailMergeConfigItem& rConfig = m_pWizard->GetConfigItem();
    rConfig.SetGreetingLine(m_aGreetingLineCB.IsChecked(), sal_False);
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
        sal_Bool bNext = pButton == &m_aNextSetIB;
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
    m_aPrevSetIB.Enable(bEnable);
    m_aNextSetIB.Enable(bEnable);
    m_aDocumentIndexFI.Enable(bEnable);
    String sTemp(m_sDocument);
    sTemp.SearchAndReplaceAscii("%1", String::CreateFromInt32(nPos));
    m_aDocumentIndexFI.SetText(sTemp);
    return 0;
}

SwMailBodyDialog::SwMailBodyDialog(Window* pParent, SwMailMergeWizard* _pWizard) :
    SfxModalDialog(pParent, SW_RES(DLG_MM_MAILBODY)),
#ifdef MSC
#pragma warning (disable : 4355)
#endif
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
#ifdef MSC
#pragma warning (default : 4355)
#endif
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
        Sequence< ::rtl::OUString > aColumns = xColAccess->getElementNames();
        for(sal_Int32 nName = 0; nName < aColumns.getLength(); ++nName)
            m_aFemaleColumnLB.InsertEntry(aColumns[nName]);
    }

    ::rtl::OUString sGenderColumn = rConfig.GetAssignedColumn(MM_PART_GENDER);
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
        Sequence< ::rtl::OUString> aAssignment = rConfigItem.GetColumnAssignment( rDBData );
        sal_Int32 nPos = m_aFemaleColumnLB.GetSelectEntryPos();
        if(aAssignment.getLength() < MM_PART_GENDER)
            aAssignment.realloc(MM_PART_GENDER);
        if( nPos > 0 )
            aAssignment[MM_PART_GENDER] = m_aFemaleColumnLB.GetSelectEntry();
        else
            aAssignment[MM_PART_GENDER] = ::rtl::OUString();
        rConfigItem.SetColumnAssignment( rDBData, aAssignment );
    }
    if(m_aFemaleFieldCB.GetText() != m_aFemaleFieldCB.GetSavedValue())
        rConfigItem.SetFemaleGenderValue(m_aFemaleFieldCB.GetText());

    EndDialog(RET_OK);
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
