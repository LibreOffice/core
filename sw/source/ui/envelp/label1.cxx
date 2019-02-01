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

#include <memory>
#include <vcl/waitobj.hxx>
#include <rtl/ustring.hxx>
#include <sfx2/sfxsids.hrc>
#include <vcl/treelistentry.hxx>
#include <tools/lineend.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <swtypes.hxx>
#include <wrtsh.hxx>
#include <initui.hxx>
#include <labimp.hxx>
#include "labfmt.hxx"
#include "labprt.hxx"
#include <unotools.hxx>
#include <dbmgr.hxx>
#include <uitool.hxx>
#include <cmdid.h>
#include <helpids.h>
#include <globals.hrc>
#include <strings.hrc>
#include <envimg.hxx>

void SwLabRec::SetFromItem( const SwLabItem& rItem )
{
    m_nHDist  = rItem.m_lHDist;
    m_nVDist  = rItem.m_lVDist;
    m_nWidth  = rItem.m_lWidth;
    m_nHeight = rItem.m_lHeight;
    m_nLeft   = rItem.m_lLeft;
    m_nUpper  = rItem.m_lUpper;
    m_nCols   = rItem.m_nCols;
    m_nRows   = rItem.m_nRows;
    m_nPWidth  = rItem.m_lPWidth;
    m_nPHeight = rItem.m_lPHeight;
    m_bCont   = rItem.m_bCont;
}

void SwLabRec::FillItem( SwLabItem& rItem ) const
{
    rItem.m_lHDist  = m_nHDist;
    rItem.m_lVDist  = m_nVDist;
    rItem.m_lWidth  = m_nWidth;
    rItem.m_lHeight = m_nHeight;
    rItem.m_lLeft   = m_nLeft;
    rItem.m_lUpper  = m_nUpper;
    rItem.m_nCols   = m_nCols;
    rItem.m_lPWidth  = m_nPWidth;
    rItem.m_lPHeight = m_nPHeight;
    rItem.m_nRows   = m_nRows;
}

void SwLabDlg::ReplaceGroup_( const OUString &rMake )
{
    // Remove old entries
    m_pRecs->erase(m_pRecs->begin() + 1, m_pRecs->end());
    aLabelsCfg.FillLabels(rMake, *m_pRecs);
    aLstGroup = rMake;
}

void SwLabDlg::PageCreated(const OString &rId, SfxTabPage &rPage)
{
    if (rId == "labels")
    {
        if (m_bLabel)
        {
            static_cast<SwLabPage*>(&rPage)->SetDBManager(pDBManager);
            static_cast<SwLabPage*>(&rPage)->InitDatabaseBox();
        }
        else
            static_cast<SwLabPage*>(&rPage)->SetToBusinessCard();
    }
    else if (rId == "options")
        pPrtPage = static_cast<SwLabPrtPage*>(&rPage);
}

SwLabDlg::SwLabDlg(weld::Window* pParent, const SfxItemSet& rSet,
                                SwDBManager* pDBManager_, bool bLabel)
    : SfxTabDialogController(pParent, "modules/swriter/ui/labeldialog.ui", "LabelDialog", &rSet)
    , pDBManager(pDBManager_)
    , pPrtPage(nullptr)
    , aTypeIds(50, 10)
    , m_pRecs(new SwLabRecs)
    , m_bLabel(bLabel)
{
    weld::WaitObject aWait(pParent);

    // Read user label from writer.cfg
    SwLabItem aItem(static_cast<const SwLabItem&>(rSet.Get( FN_LABEL )));
    std::unique_ptr<SwLabRec> pRec(new SwLabRec);
    pRec->m_aMake = pRec->m_aType = SwResId(STR_CUSTOM_LABEL);
    pRec->SetFromItem( aItem );

    bool bDouble = false;

    for (std::unique_ptr<SwLabRec> & i : *m_pRecs)
    {
        if (pRec->m_aMake == i->m_aMake &&
            pRec->m_aType == i->m_aType)
        {
            bDouble = true;
            break;
        }
    }

    if (!bDouble)
        m_pRecs->insert( m_pRecs->begin(), std::move(pRec));

    size_t nLstGroup = 0;
    const std::vector<OUString>& rMan = aLabelsCfg.GetManufacturers();
    for(size_t nMan = 0; nMan < rMan.size(); ++nMan)
    {
        aMakes.push_back(rMan[nMan]);
        if ( rMan[nMan] == aItem.m_aLstMake )
            nLstGroup = nMan;
    }

    if ( !aMakes.empty() )
        ReplaceGroup_( aMakes[nLstGroup] );

    if (m_xExampleSet)
        m_xExampleSet->Put(aItem);

    AddTabPage("format", SwLabFormatPage::Create, nullptr);
    AddTabPage("options", SwLabPrtPage::Create, nullptr);
    m_sBusinessCardDlg = SwResId(STR_BUSINESS_CARDS);

    if (m_bLabel)
    {
        RemoveTabPage("business");
        RemoveTabPage("private");
        RemoveTabPage("medium");
        AddTabPage("labels", SwLabPage::Create, nullptr);
    }
    else
    {
        RemoveTabPage("labels");
        AddTabPage("medium", SwLabPage::Create, nullptr);
        AddTabPage("business", SwBusinessDataPage::Create, nullptr );
        AddTabPage("private", SwPrivateDataPage::Create, nullptr);
        m_xDialog->set_title(m_sBusinessCardDlg);
    }

    pParent->set_busy_cursor(false);
}

SwLabDlg::~SwLabDlg()
{
    m_pRecs.reset();
}

void SwLabDlg::GetLabItem(SwLabItem &rItem)
{
    const SwLabItem& rActItem = static_cast<const SwLabItem&>(GetExampleSet()->Get(FN_LABEL));
    const SwLabItem& rOldItem = static_cast<const SwLabItem&>(GetInputSetImpl()->Get(FN_LABEL));

    if (rActItem != rOldItem)
    {
        // Was already "put" with (hopefully) correct content
        rItem = rActItem;
    }
    else
    {
        rItem = rOldItem;

        // In rItem there are only settings defined by users.
        // Therefore get the real settings directly from Record
        SwLabRec* pRec = GetRecord(rItem.m_aType, rItem.m_bCont);
        pRec->FillItem( rItem );
    }
}

SwLabRec* SwLabDlg::GetRecord(const OUString &rRecName, bool bCont)
{
    SwLabRec* pRec = nullptr;
    bool bFound = false;
    const OUString sCustom(SwResId(STR_CUSTOM_LABEL));

    const size_t nCount = Recs().size();
    for (size_t i = 0; i < nCount; ++i)
    {
        pRec = Recs()[i].get();
        if (pRec->m_aType != sCustom &&
            rRecName == pRec->m_aType && bCont == pRec->m_bCont)
        {
            bFound = true;
            break;
        }
    }
    if (!bFound)    // User defined
        pRec = Recs()[0].get();

    return pRec;
}

Printer *SwLabDlg::GetPrt()
{
    if (pPrtPage)
        return pPrtPage->GetPrt();
    else
        return nullptr;
}

SwLabPage::SwLabPage(TabPageParent pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "modules/swriter/ui/cardmediumpage.ui", "CardMediumPage", &rSet)
    , pDBManager(nullptr)
    , aItem(static_cast<const SwLabItem&>(rSet.Get(FN_LABEL)))
    , m_xAddressFrame(m_xBuilder->weld_widget("addressframe"))
    , m_xAddrBox(m_xBuilder->weld_check_button("address"))
    , m_xWritingEdit(m_xBuilder->weld_text_view("textview"))
    , m_xDatabaseLB(m_xBuilder->weld_combo_box("database"))
    , m_xTableLB(m_xBuilder->weld_combo_box("table"))
    , m_xInsertBT(m_xBuilder->weld_button("insert"))
    , m_xDBFieldLB(m_xBuilder->weld_combo_box("field"))
    , m_xContButton(m_xBuilder->weld_radio_button("continuous"))
    , m_xSheetButton(m_xBuilder->weld_radio_button("sheet"))
    , m_xMakeBox(m_xBuilder->weld_combo_box("brand"))
    , m_xTypeBox(m_xBuilder->weld_combo_box("type"))
    , m_xHiddenSortTypeBox(m_xBuilder->weld_combo_box("hiddentype"))
    , m_xFormatInfo(m_xBuilder->weld_label("formatinfo"))
{
    WaitObject aWait(pParent.pParent);

    m_xWritingEdit->set_size_request(m_xWritingEdit->get_approximate_digit_width() * 30,
                                     m_xWritingEdit->get_height_rows(10));
    m_xHiddenSortTypeBox->make_sorted();

    long nListBoxWidth = m_xWritingEdit->get_approximate_digit_width() * 25;
    m_xTableLB->set_size_request(nListBoxWidth, -1);
    m_xDatabaseLB->set_size_request(nListBoxWidth, -1);
    m_xDBFieldLB->set_size_request(nListBoxWidth, -1);

    SetExchangeSupport();

    // Install handlers
    m_xAddrBox->connect_toggled(LINK(this, SwLabPage, AddrHdl));
    m_xDatabaseLB->connect_changed(LINK(this, SwLabPage, DatabaseHdl));
    m_xTableLB->connect_changed(LINK(this, SwLabPage, DatabaseHdl));
    m_xDBFieldLB->connect_changed(LINK(this, SwLabPage, DatabaseHdl));
    m_xInsertBT->connect_clicked(LINK(this, SwLabPage, FieldHdl));
    // Disable insert button first,
    // it'll be enabled if m_xDatabaseLB, m_pTableLB and m_pInsertBT are filled
    m_xInsertBT->set_sensitive(false);
    m_xContButton->connect_toggled(LINK(this, SwLabPage, PageHdl));
    m_xSheetButton->connect_toggled(LINK(this, SwLabPage, PageHdl));
    auto nMaxWidth = m_xMakeBox->get_approximate_digit_width() * 32;
    m_xMakeBox->set_size_request(nMaxWidth, -1);
    m_xTypeBox->set_size_request(nMaxWidth, -1);
    m_xMakeBox->connect_changed(LINK(this, SwLabPage, MakeHdl));
    m_xTypeBox->connect_changed(LINK(this, SwLabPage, TypeHdl));

    InitDatabaseBox();
}

SwLabPage::~SwLabPage()
{
}

void SwLabPage::SetToBusinessCard()
{
    m_xContainer->set_help_id(HID_BUSINESS_FMT_PAGE);
    m_xContButton->set_help_id(HID_BUSINESS_FMT_PAGE_CONT);
    m_xSheetButton->set_help_id(HID_BUSINESS_FMT_PAGE_SHEET);
    m_xMakeBox->set_help_id(HID_BUSINESS_FMT_PAGE_BRAND);
    m_xTypeBox->set_help_id(HID_BUSINESS_FMT_PAGE_TYPE);
    m_xAddressFrame->hide();
};

IMPL_LINK_NOARG(SwLabPage, AddrHdl, weld::ToggleButton&, void)
{
    OUString aWriting;

    if (m_xAddrBox->get_active())
        aWriting = convertLineEnd(MakeSender(), GetSystemLineEnd());

    m_xWritingEdit->set_text(aWriting);
    m_xWritingEdit->grab_focus();
}

IMPL_LINK( SwLabPage, DatabaseHdl, weld::ComboBox&, rListBox, void )
{
    sActDBName = m_xDatabaseLB->get_active_text();

    weld::WaitObject aObj(GetParentSwLabDlg()->getDialog());

    if (&rListBox == m_xDatabaseLB.get())
        GetDBManager()->GetTableNames(*m_xTableLB, sActDBName);

    if (&rListBox == m_xDatabaseLB.get() || &rListBox == m_xTableLB.get())
        GetDBManager()->GetColumnNames(*m_xDBFieldLB, sActDBName, m_xTableLB->get_active_text());

    if (!m_xDatabaseLB->get_active_text().isEmpty() && !m_xTableLB->get_active_text().isEmpty()
            && !m_xDBFieldLB->get_active_text().isEmpty())
        m_xInsertBT->set_sensitive(true);
    else
        m_xInsertBT->set_sensitive(false);
}

IMPL_LINK_NOARG(SwLabPage, FieldHdl, weld::Button&, void)
{
    OUString aStr("<" + m_xDatabaseLB->get_active_text() + "." +
                  m_xTableLB->get_active_text() + "." +
                  m_xTableLB->get_active_id() + "." +
                  m_xDBFieldLB->get_active_text() + ">");
    m_xWritingEdit->replace_selection(aStr);
    int nStartPos, nEndPos;
    m_xWritingEdit->get_selection_bounds(nStartPos, nEndPos);
    m_xWritingEdit->grab_focus();
    m_xWritingEdit->select_region(nStartPos, nEndPos);
}

IMPL_LINK_NOARG(SwLabPage, PageHdl, weld::ToggleButton&, void)
{
    MakeHdl(*m_xMakeBox);
}

IMPL_LINK_NOARG(SwLabPage, MakeHdl, weld::ComboBox&, void)
{
    weld::WaitObject aWait(GetParentSwLabDlg()->getDialog());

    m_xTypeBox->clear();
    m_xHiddenSortTypeBox->clear();
    GetParentSwLabDlg()->TypeIds().clear();

    const OUString aMake = m_xMakeBox->get_active_text();
    GetParentSwLabDlg()->ReplaceGroup( aMake );
    aItem.m_aLstMake = aMake;

    const bool   bCont    = m_xContButton->get_active();
    const size_t nCount   = GetParentSwLabDlg()->Recs().size();
    size_t nLstType = 0;

    const OUString sCustom(SwResId(STR_CUSTOM_LABEL));
    //insert the entries into the sorted list box
    for ( size_t i = 0; i < nCount; ++i )
    {
        const OUString aType(GetParentSwLabDlg()->Recs()[i]->m_aType);
        bool bInsert = false;
        if (GetParentSwLabDlg()->Recs()[i]->m_aType == sCustom)
        {
            bInsert = true;
            m_xTypeBox->append_text(aType );
        }
        else if (GetParentSwLabDlg()->Recs()[i]->m_bCont == bCont)
        {
            if (m_xHiddenSortTypeBox->find_text(aType) == -1)
            {
                bInsert = true;
                m_xHiddenSortTypeBox->append_text( aType );
            }
        }
        if(bInsert)
        {
            GetParentSwLabDlg()->TypeIds().push_back(i);
            if ( !nLstType && aType == aItem.m_aLstType )
                nLstType = GetParentSwLabDlg()->TypeIds().size();
        }
    }
    for (int nEntry = 0; nEntry < m_xHiddenSortTypeBox->get_count(); ++nEntry)
    {
        m_xTypeBox->append_text(m_xHiddenSortTypeBox->get_text(nEntry));
    }
    if (nLstType)
        m_xTypeBox->set_active_text(aItem.m_aLstType);
    else
        m_xTypeBox->set_active(0);
    TypeHdl(*m_xTypeBox);
}

IMPL_LINK_NOARG(SwLabPage, TypeHdl, weld::ComboBox&, void)
{
    DisplayFormat();
    aItem.m_aType = m_xTypeBox->get_active_text();
}

void SwLabPage::DisplayFormat()
{
    ScopedVclPtrInstance< MetricField > aField(this, WinBits(0));
    FieldUnit aMetric = ::GetDfltMetric(false);
    SetMetric(*aField, aMetric);
    aField->SetDecimalDigits(2);
    aField->SetMin         (0);
    aField->SetMax         (LONG_MAX);

    SwLabRec* pRec = GetSelectedEntryPos();
    aItem.m_aLstType = pRec->m_aType;
    SETFLDVAL(*aField, pRec->m_nWidth);
    aField->Reformat();
    const OUString aWString = aField->GetText();

    SETFLDVAL(*aField, pRec->m_nHeight);
    aField->Reformat();

    OUString aText = pRec->m_aType + ": " + aWString +
           " x " + aField->GetText() +
           " (" + OUString::number( pRec->m_nCols ) +
           " x " + OUString::number( pRec->m_nRows ) + ")";
    m_xFormatInfo->set_label(aText);
}

SwLabRec* SwLabPage::GetSelectedEntryPos()
{
    OUString sSelEntry(m_xTypeBox->get_active_text());

    return GetParentSwLabDlg()->GetRecord(sSelEntry, m_xContButton->get_active());
}

void SwLabPage::InitDatabaseBox()
{
    if( GetDBManager() )
    {
        m_xDatabaseLB->clear();
        css::uno::Sequence<OUString> aDataNames = SwDBManager::GetExistingDatabaseNames();
        const OUString* pDataNames = aDataNames.getConstArray();
        for (long i = 0; i < aDataNames.getLength(); i++)
            m_xDatabaseLB->append_text(pDataNames[i]);
        sal_Int32 nIdx{ 0 };
        OUString sDBName = sActDBName.getToken( 0, DB_DELIM, nIdx );
        OUString sTableName = sActDBName.getToken( 0, DB_DELIM, nIdx );
        m_xDatabaseLB->set_active_text(sDBName);
        if( !sDBName.isEmpty() && GetDBManager()->GetTableNames(*m_xTableLB, sDBName))
        {
            m_xTableLB->set_active_text(sTableName);
            GetDBManager()->GetColumnNames(*m_xDBFieldLB, sActDBName, sTableName);
        }
        else
            m_xDBFieldLB->clear();
    }
}

VclPtr<SfxTabPage> SwLabPage::Create(TabPageParent pParent, const SfxItemSet* rSet)
{
    return VclPtr<SwLabPage>::Create(pParent, *rSet);
}

void SwLabPage::ActivatePage(const SfxItemSet& rSet)
{
    Reset( &rSet );
}

DeactivateRC SwLabPage::DeactivatePage(SfxItemSet* _pSet)
{
    if (_pSet)
        FillItemSet(_pSet);

    return DeactivateRC::LeavePage;
}

void SwLabPage::FillItem(SwLabItem& rItem)
{
    rItem.m_bAddr    = m_xAddrBox->get_active();
    rItem.m_aWriting = m_xWritingEdit->get_text();
    rItem.m_bCont    = m_xContButton->get_active();
    rItem.m_aMake    = m_xMakeBox->get_active_text();
    rItem.m_aType    = m_xTypeBox->get_active_text();
    rItem.m_sDBName  = sActDBName;

    SwLabRec* pRec = GetSelectedEntryPos();
    pRec->FillItem( rItem );

    rItem.m_aLstMake = m_xMakeBox->get_active_text();
    rItem.m_aLstType = m_xTypeBox->get_active_text();
}

bool SwLabPage::FillItemSet(SfxItemSet* rSet)
{
    FillItem( aItem );
    rSet->Put( aItem );

    return true;
}

void SwLabPage::Reset(const SfxItemSet* rSet)
{
    m_xMakeBox->clear();

    size_t nLstGroup = 0;

    const size_t nCount = GetParentSwLabDlg()->Makes().size();
    for(size_t i = 0; i < nCount; ++i)
    {
        OUString& rStr = GetParentSwLabDlg()->Makes()[i];
        m_xMakeBox->append_text(rStr);

        if ( rStr == aItem.m_aLstMake)
            nLstGroup = i;
    }

    m_xMakeBox->set_active( nLstGroup );
    MakeHdl(*m_xMakeBox);

    aItem = static_cast<const SwLabItem&>( rSet->Get(FN_LABEL));
    OUString sDBName  = aItem.m_sDBName;

    OUString aWriting(convertLineEnd(aItem.m_aWriting, GetSystemLineEnd()));

    m_xAddrBox->set_active( aItem.m_bAddr );
    m_xWritingEdit->set_text( aWriting );

    for(const auto& rMake : GetParentSwLabDlg()->Makes())
    {
        if (m_xMakeBox->find_text(rMake) == -1)
            m_xMakeBox->append_text(rMake);
    }

    m_xMakeBox->set_active_text(aItem.m_aMake);
    //save the current type
    OUString sType(aItem.m_aType);
    MakeHdl(*m_xMakeBox);
    aItem.m_aType = sType;
    //#102806# a newly added make may not be in the type ListBox already
    if (m_xTypeBox->find_text(aItem.m_aType) == -1 && !aItem.m_aMake.isEmpty())
        GetParentSwLabDlg()->UpdateGroup( aItem.m_aMake );
    if (m_xTypeBox->find_text(aItem.m_aType) != -1)
    {
        m_xTypeBox->set_active_text(aItem.m_aType);
        TypeHdl(*m_xTypeBox);
    }
    if (m_xDatabaseLB->find_text(sDBName) != -1)
    {
        m_xDatabaseLB->set_active_text(sDBName);
        DatabaseHdl(*m_xDatabaseLB);
    }

    if (aItem.m_bCont)
        m_xContButton->set_active(true);
    else
        m_xSheetButton->set_active(true);
}

SwPrivateDataPage::SwPrivateDataPage(TabPageParent pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "modules/swriter/ui/privateuserpage.ui", "PrivateUserPage", &rSet)
    , m_xFirstNameED(m_xBuilder->weld_entry("firstname"))
    , m_xNameED(m_xBuilder->weld_entry("lastname"))
    , m_xShortCutED(m_xBuilder->weld_entry("shortname"))
    , m_xFirstName2ED(m_xBuilder->weld_entry("firstname2"))
    , m_xName2ED(m_xBuilder->weld_entry("lastname2"))
    , m_xShortCut2ED(m_xBuilder->weld_entry("shortname2"))
    , m_xStreetED(m_xBuilder->weld_entry("street"))
    , m_xZipED(m_xBuilder->weld_entry("izip"))
    , m_xCityED(m_xBuilder->weld_entry("icity"))
    , m_xCountryED(m_xBuilder->weld_entry("country"))
    , m_xStateED(m_xBuilder->weld_entry("state"))
    , m_xTitleED(m_xBuilder->weld_entry("title"))
    , m_xProfessionED(m_xBuilder->weld_entry("job"))
    , m_xPhoneED(m_xBuilder->weld_entry("phone"))
    , m_xMobilePhoneED(m_xBuilder->weld_entry("mobile"))
    , m_xFaxED(m_xBuilder->weld_entry("fax"))
    , m_xHomePageED(m_xBuilder->weld_entry("url"))
    , m_xMailED(m_xBuilder->weld_entry("email"))
{
    SetExchangeSupport();
}

SwPrivateDataPage::~SwPrivateDataPage()
{
}

VclPtr<SfxTabPage> SwPrivateDataPage::Create(TabPageParent pParent, const SfxItemSet* rSet)
{
    return VclPtr<SwPrivateDataPage>::Create(pParent, *rSet);
}

void SwPrivateDataPage::ActivatePage(const SfxItemSet& rSet)
{
    Reset(&rSet);
}

DeactivateRC SwPrivateDataPage::DeactivatePage(SfxItemSet* _pSet)
{
    if (_pSet)
        FillItemSet(_pSet);
    return DeactivateRC::LeavePage;
}

bool SwPrivateDataPage::FillItemSet(SfxItemSet* rSet)
{
    const SfxItemSet* pExampleSet = GetDialogExampleSet();
    assert(pExampleSet);
    SwLabItem aItem = static_cast<const SwLabItem&>(pExampleSet->Get(FN_LABEL));

    aItem.m_aPrivFirstName = m_xFirstNameED->get_text();
    aItem.m_aPrivName      = m_xNameED->get_text();
    aItem.m_aPrivShortCut  = m_xShortCutED->get_text();
    aItem.m_aPrivFirstName2 = m_xFirstName2ED->get_text();
    aItem.m_aPrivName2     = m_xName2ED->get_text();
    aItem.m_aPrivShortCut2 = m_xShortCut2ED->get_text();
    aItem.m_aPrivStreet    = m_xStreetED->get_text();
    aItem.m_aPrivZip       = m_xZipED->get_text();
    aItem.m_aPrivCity      = m_xCityED->get_text();
    aItem.m_aPrivCountry   = m_xCountryED->get_text();
    aItem.m_aPrivState     = m_xStateED->get_text();
    aItem.m_aPrivTitle     = m_xTitleED->get_text();
    aItem.m_aPrivProfession= m_xProfessionED->get_text();
    aItem.m_aPrivPhone     = m_xPhoneED->get_text();
    aItem.m_aPrivMobile    = m_xMobilePhoneED->get_text();
    aItem.m_aPrivFax       = m_xFaxED->get_text();
    aItem.m_aPrivWWW       = m_xHomePageED->get_text();
    aItem.m_aPrivMail      = m_xMailED->get_text();

    rSet->Put(aItem);
    return true;
}

void SwPrivateDataPage::Reset(const SfxItemSet* rSet)
{
    const SwLabItem& aItem = static_cast<const SwLabItem&>( rSet->Get(FN_LABEL) );
    m_xFirstNameED->set_text(aItem.m_aPrivFirstName);
    m_xNameED->set_text(aItem.m_aPrivName);
    m_xShortCutED->set_text(aItem.m_aPrivShortCut);
    m_xFirstName2ED->set_text(aItem.m_aPrivFirstName2);
    m_xName2ED->set_text(aItem.m_aPrivName2);
    m_xShortCut2ED->set_text(aItem.m_aPrivShortCut2);
    m_xStreetED->set_text(aItem.m_aPrivStreet);
    m_xZipED->set_text(aItem.m_aPrivZip);
    m_xCityED->set_text(aItem.m_aPrivCity);
    m_xCountryED->set_text(aItem.m_aPrivCountry);
    m_xStateED->set_text(aItem.m_aPrivState);
    m_xTitleED->set_text(aItem.m_aPrivTitle);
    m_xProfessionED->set_text(aItem.m_aPrivProfession);
    m_xPhoneED->set_text(aItem.m_aPrivPhone);
    m_xMobilePhoneED->set_text(aItem.m_aPrivMobile);
    m_xFaxED->set_text(aItem.m_aPrivFax);
    m_xHomePageED->set_text(aItem.m_aPrivWWW);
    m_xMailED->set_text(aItem.m_aPrivMail);
}

SwBusinessDataPage::SwBusinessDataPage(TabPageParent pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "modules/swriter/ui/businessdatapage.ui", "BusinessDataPage", &rSet)
    , m_xCompanyED(m_xBuilder->weld_entry("company"))
    , m_xCompanyExtED(m_xBuilder->weld_entry("company2"))
    , m_xSloganED(m_xBuilder->weld_entry("slogan"))
    , m_xStreetED(m_xBuilder->weld_entry("street"))
    , m_xZipED(m_xBuilder->weld_entry("izip"))
    , m_xCityED(m_xBuilder->weld_entry("icity"))
    , m_xCountryED(m_xBuilder->weld_entry("country"))
    , m_xStateED(m_xBuilder->weld_entry("state"))
    , m_xPositionED(m_xBuilder->weld_entry("position"))
    , m_xPhoneED(m_xBuilder->weld_entry("phone"))
    , m_xMobilePhoneED(m_xBuilder->weld_entry("mobile"))
    , m_xFaxED(m_xBuilder->weld_entry("fax"))
    , m_xHomePageED(m_xBuilder->weld_entry("url"))
    , m_xMailED(m_xBuilder->weld_entry("email"))
{
    SetExchangeSupport();
}

SwBusinessDataPage::~SwBusinessDataPage()
{
}

VclPtr<SfxTabPage> SwBusinessDataPage::Create(TabPageParent pParent, const SfxItemSet* rSet)
{
    return VclPtr<SwBusinessDataPage>::Create(pParent, *rSet);
}

void SwBusinessDataPage::ActivatePage(const SfxItemSet& rSet)
{
    Reset(&rSet);
}

DeactivateRC SwBusinessDataPage::DeactivatePage(SfxItemSet* _pSet)
{
    if (_pSet)
        FillItemSet(_pSet);
    return DeactivateRC::LeavePage;
}

bool SwBusinessDataPage::FillItemSet(SfxItemSet* rSet)
{
    const SfxItemSet* pExampleSet = GetDialogExampleSet();
    assert(pExampleSet);
    SwLabItem aItem = static_cast<const SwLabItem&>(pExampleSet->Get(FN_LABEL));

    aItem.m_aCompCompany   = m_xCompanyED->get_text();
    aItem.m_aCompCompanyExt= m_xCompanyExtED->get_text();
    aItem.m_aCompSlogan    = m_xSloganED->get_text();
    aItem.m_aCompStreet    = m_xStreetED->get_text();
    aItem.m_aCompZip       = m_xZipED->get_text();
    aItem.m_aCompCity      = m_xCityED->get_text();
    aItem.m_aCompCountry   = m_xCountryED->get_text();
    aItem.m_aCompState     = m_xStateED->get_text();
    aItem.m_aCompPosition  = m_xPositionED->get_text();
    aItem.m_aCompPhone     = m_xPhoneED->get_text();
    aItem.m_aCompMobile    = m_xMobilePhoneED->get_text();
    aItem.m_aCompFax       = m_xFaxED->get_text();
    aItem.m_aCompWWW       = m_xHomePageED->get_text();
    aItem.m_aCompMail      = m_xMailED->get_text();

    rSet->Put(aItem);
    return true;
}

void SwBusinessDataPage::Reset(const SfxItemSet* rSet)
{
    const SwLabItem& aItem = static_cast<const SwLabItem&>( rSet->Get(FN_LABEL) );
    m_xCompanyED->set_text(aItem.m_aCompCompany);
    m_xCompanyExtED->set_text(aItem.m_aCompCompanyExt);
    m_xSloganED->set_text(aItem.m_aCompSlogan);
    m_xStreetED->set_text(aItem.m_aCompStreet);
    m_xZipED->set_text(aItem.m_aCompZip);
    m_xCityED->set_text(aItem.m_aCompCity);
    m_xCountryED->set_text(aItem.m_aCompCountry);
    m_xStateED->set_text(aItem.m_aCompState);
    m_xPositionED->set_text(aItem.m_aCompPosition);
    m_xPhoneED->set_text(aItem.m_aCompPhone);
    m_xMobilePhoneED->set_text(aItem.m_aCompMobile);
    m_xFaxED->set_text(aItem.m_aCompFax);
    m_xHomePageED->set_text(aItem.m_aCompWWW);
    m_xMailED->set_text(aItem.m_aCompMail);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
