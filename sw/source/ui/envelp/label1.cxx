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
#include <svtools/treelistentry.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <swtypes.hxx>
#include <wrtsh.hxx>
#include <initui.hxx>
#include "labimp.hxx"
#include <labfmt.hxx>
#include <labprt.hxx>
#include <unotools.hxx>
#include <dbmgr.hxx>
#include "uitool.hxx"
#include <cmdid.h>
#include <helpid.h>
#include <globals.hrc>
#include <strings.hrc>
#include <envimg.hxx>

void SwLabRec::SetFromItem( const SwLabItem& rItem )
{
    lHDist  = rItem.m_lHDist;
    lVDist  = rItem.m_lVDist;
    lWidth  = rItem.m_lWidth;
    lHeight = rItem.m_lHeight;
    lLeft   = rItem.m_lLeft;
    lUpper  = rItem.m_lUpper;
    nCols   = rItem.m_nCols;
    nRows   = rItem.m_nRows;
    lPWidth  = rItem.m_lPWidth;
    lPHeight = rItem.m_lPHeight;
    bCont   = rItem.m_bCont;
}

void SwLabRec::FillItem( SwLabItem& rItem ) const
{
    rItem.m_lHDist  = lHDist;
    rItem.m_lVDist  = lVDist;
    rItem.m_lWidth  = lWidth;
    rItem.m_lHeight = lHeight;
    rItem.m_lLeft   = lLeft;
    rItem.m_lUpper  = lUpper;
    rItem.m_nCols   = nCols;
    rItem.m_lPWidth  = lPWidth;
    rItem.m_lPHeight = lPHeight;
    rItem.m_nRows   = nRows;
}

void SwLabDlg::ReplaceGroup_( const OUString &rMake )
{
    // Remove old entries
    m_pRecs->erase(m_pRecs->begin() + 1, m_pRecs->end());
    aLabelsCfg.FillLabels(rMake, *m_pRecs);
    aLstGroup = rMake;
}

void SwLabDlg::PageCreated(sal_uInt16 nId, SfxTabPage &rPage)
{
    if (nId == m_nLabelId)
    {
        if(m_bLabel)
        {
            static_cast<SwLabPage*>(&rPage)->SetDBManager(pDBManager);
            static_cast<SwLabPage*>(&rPage)->InitDatabaseBox();
        }
        else
            static_cast<SwLabPage*>(&rPage)->SetToBusinessCard();
    }
    else if (nId == m_nOptionsId)
        pPrtPage = static_cast<SwLabPrtPage*>(&rPage);
}

SwLabDlg::SwLabDlg(vcl::Window* pParent, const SfxItemSet& rSet,
                                SwDBManager* pDBManager_, bool bLabel)
    : SfxTabDialog(pParent, "LabelDialog",
        "modules/swriter/ui/labeldialog.ui", &rSet)
    , pDBManager(pDBManager_)
    , pPrtPage(nullptr)
    , aTypeIds(50, 10)
    , m_pRecs(new SwLabRecs)
    , m_bLabel(bLabel)
    , m_nOptionsId(0)
    , m_nLabelId(0)
    , m_nCardsId(0)
{
    WaitObject aWait( pParent );

    AddTabPage("format", SwLabFormatPage::Create, nullptr);
    m_nOptionsId = AddTabPage("options", SwLabPrtPage::Create, nullptr);
    m_nCardsId = AddTabPage("cards", SwVisitingCardPage::Create, nullptr);
    m_sBusinessCardDlg = GetPageText(m_nCardsId);

    if (m_bLabel)
    {
        RemoveTabPage("business");
        RemoveTabPage("private");
        RemoveTabPage("cards");
        RemoveTabPage("medium");
        m_nLabelId = AddTabPage("labels", SwLabPage::Create, nullptr);
    }
    else
    {
        RemoveTabPage("labels");
        RemoveTabPage("cards");
        m_nLabelId = AddTabPage("medium", SwLabPage::Create, nullptr);
        AddTabPage("business", SwBusinessDataPage::Create, nullptr );
        AddTabPage("private", SwPrivateDataPage::Create, nullptr);
        SetText(m_sBusinessCardDlg);
    }
    // Read user label from writer.cfg
    SwLabItem aItem(static_cast<const SwLabItem&>(rSet.Get( FN_LABEL )));
    std::unique_ptr<SwLabRec> pRec(new SwLabRec);
    pRec->aMake = pRec->aType = SwResId(STR_CUSTOM_LABEL);
    pRec->SetFromItem( aItem );

    bool bDouble = false;

    for (std::unique_ptr<SwLabRec> & i : *m_pRecs)
    {
        if (pRec->aMake == i->aMake &&
            pRec->aType == i->aType)
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

    if (m_pExampleSet)
        m_pExampleSet->Put(aItem);
}

SwLabDlg::~SwLabDlg()
{
    disposeOnce();
}

void SwLabDlg::dispose()
{
    delete m_pRecs;
    pPrtPage.clear();
    SfxTabDialog::dispose();
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
        if (pRec->aType != sCustom &&
            rRecName == pRec->aType && bCont == pRec->bCont)
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

SwLabPage::SwLabPage(vcl::Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "CardMediumPage",
        "modules/swriter/ui/cardmediumpage.ui", &rSet)
    , pDBManager(nullptr)
    , aItem(static_cast<const SwLabItem&>(rSet.Get(FN_LABEL)))
{
    WaitObject aWait( pParent );

    get(m_pAddressFrame, "addressframe");
    get(m_pAddrBox, "address");
    get(m_pWritingEdit, "textview");
    m_pWritingEdit->set_height_request(m_pWritingEdit->GetTextHeight() * 10);
    m_pWritingEdit->set_width_request(m_pWritingEdit->approximate_char_width() * 25);
    get(m_pDatabaseLB, "database");
    get(m_pTableLB, "table");
    get(m_pInsertBT, "insert");
    get(m_pDBFieldLB, "field");
    get(m_pContButton, "continuous");
    get(m_pSheetButton, "sheet");
    get(m_pMakeBox, "brand");
    get(m_pTypeBox, "type");
    get(m_pFormatInfo, "formatinfo");
    get(m_pHiddenSortTypeBox, "hiddentype");
    m_pHiddenSortTypeBox->SetStyle(m_pHiddenSortTypeBox->GetStyle() | WB_SORT);

    long nListBoxWidth = approximate_char_width() * 30;
    m_pTableLB->set_width_request(nListBoxWidth);
    m_pDatabaseLB->set_width_request(nListBoxWidth);
    m_pDBFieldLB->set_width_request(nListBoxWidth);

    SetExchangeSupport();

    // Install handlers
    m_pAddrBox->SetClickHdl (LINK(this, SwLabPage, AddrHdl         ));
    m_pDatabaseLB->SetSelectHdl(LINK(this, SwLabPage, DatabaseHdl     ));
    m_pTableLB->SetSelectHdl(LINK(this, SwLabPage, DatabaseHdl     ));
    m_pInsertBT->SetClickHdl (LINK(this, SwLabPage, FieldHdl        ));
    m_pContButton->SetClickHdl (LINK(this, SwLabPage, PageHdl         ));
    m_pSheetButton->SetClickHdl (LINK(this, SwLabPage, PageHdl         ));
    m_pMakeBox->SetSelectHdl(LINK(this, SwLabPage, MakeHdl         ));
    m_pTypeBox->SetSelectHdl(LINK(this, SwLabPage, TypeHdl         ));

    InitDatabaseBox();

    size_t nLstGroup = 0;

    const size_t nCount = GetParentSwLabDlg()->Makes().size();
    for(size_t i = 0; i < nCount; ++i)
    {
        OUString& rStr = GetParentSwLabDlg()->Makes()[i];
        m_pMakeBox->InsertEntry( rStr );

        if ( rStr == aItem.m_aLstMake)
            nLstGroup = i;
    }

    m_pMakeBox->SelectEntryPos( nLstGroup );
    m_pMakeBox->GetSelectHdl().Call(*m_pMakeBox);
}

SwLabPage::~SwLabPage()
{
    disposeOnce();
}

void SwLabPage::dispose()
{
    m_pAddressFrame.clear();
    m_pAddrBox.clear();
    m_pWritingEdit.clear();
    m_pDatabaseLB.clear();
    m_pTableLB.clear();
    m_pInsertBT.clear();
    m_pDBFieldLB.clear();
    m_pContButton.clear();
    m_pSheetButton.clear();
    m_pMakeBox.clear();
    m_pTypeBox.clear();
    m_pHiddenSortTypeBox.clear();
    m_pFormatInfo.clear();
    SfxTabPage::dispose();
}

void SwLabPage::SetToBusinessCard()
{
    SetHelpId(HID_BUSINESS_FMT_PAGE);
    m_pContButton->SetHelpId(HID_BUSINESS_FMT_PAGE_CONT);
    m_pSheetButton->SetHelpId(HID_BUSINESS_FMT_PAGE_SHEET);
    m_pMakeBox->SetHelpId(HID_BUSINESS_FMT_PAGE_BRAND);
    m_pTypeBox->SetHelpId(HID_BUSINESS_FMT_PAGE_TYPE);
    m_pAddressFrame->Hide();
};

IMPL_LINK_NOARG(SwLabPage, AddrHdl, Button*, void)
{
    OUString aWriting;

    if ( m_pAddrBox->IsChecked() )
        aWriting = convertLineEnd(MakeSender(), GetSystemLineEnd());

    m_pWritingEdit->SetText( aWriting );
    m_pWritingEdit->GrabFocus();
}

IMPL_LINK( SwLabPage, DatabaseHdl, ListBox&, rListBox, void )
{
    sActDBName = m_pDatabaseLB->GetSelectEntry();

    WaitObject aObj( GetParentSwLabDlg() );

    if (&rListBox == m_pDatabaseLB)
        GetDBManager()->GetTableNames(m_pTableLB, sActDBName);

    GetDBManager()->GetColumnNames(m_pDBFieldLB, sActDBName, m_pTableLB->GetSelectEntry());
}

IMPL_LINK_NOARG(SwLabPage, FieldHdl, Button*, void)
{
    OUString aStr("<" + m_pDatabaseLB->GetSelectEntry() + "." +
                  m_pTableLB->GetSelectEntry() + "." +
                  (m_pTableLB->GetSelectEntryData() == nullptr ? OUString("0") : OUString("1")) + "." +
                  m_pDBFieldLB->GetSelectEntry() + ">");
    m_pWritingEdit->ReplaceSelected(aStr);
    Selection aSel = m_pWritingEdit->GetSelection();
    m_pWritingEdit->GrabFocus();
    m_pWritingEdit->SetSelection(aSel);
}

IMPL_LINK_NOARG(SwLabPage, PageHdl, Button*, void)
{
    m_pMakeBox->GetSelectHdl().Call(*m_pMakeBox);
}

IMPL_LINK_NOARG(SwLabPage, MakeHdl, ListBox&, void)
{
    WaitObject aWait( GetParentSwLabDlg() );

    m_pTypeBox->Clear();
    m_pHiddenSortTypeBox->Clear();
    GetParentSwLabDlg()->TypeIds().clear();

    const OUString aMake = m_pMakeBox->GetSelectEntry();
    GetParentSwLabDlg()->ReplaceGroup( aMake );
    aItem.m_aLstMake = aMake;

    const bool   bCont    = m_pContButton->IsChecked();
    const size_t nCount   = GetParentSwLabDlg()->Recs().size();
    size_t nLstType = 0;

    const OUString sCustom(SwResId(STR_CUSTOM_LABEL));
    //insert the entries into the sorted list box
    for ( size_t i = 0; i < nCount; ++i )
    {
        const OUString aType(GetParentSwLabDlg()->Recs()[i]->aType);
        bool bInsert = false;
        if (GetParentSwLabDlg()->Recs()[i]->aType == sCustom)
        {
            bInsert = true;
            m_pTypeBox->InsertEntry(aType );
        }
        else if (GetParentSwLabDlg()->Recs()[i]->bCont == bCont)
        {
            if ( m_pHiddenSortTypeBox->GetEntryPos(aType) == LISTBOX_ENTRY_NOTFOUND )
            {
                bInsert = true;
                m_pHiddenSortTypeBox->InsertEntry( aType );
            }
        }
        if(bInsert)
        {
            GetParentSwLabDlg()->TypeIds().push_back(i);
            if ( !nLstType && aType == aItem.m_aLstType )
                nLstType = GetParentSwLabDlg()->TypeIds().size();
        }
    }
    for(sal_Int32 nEntry = 0; nEntry < m_pHiddenSortTypeBox->GetEntryCount(); ++nEntry)
    {
        m_pTypeBox->InsertEntry(m_pHiddenSortTypeBox->GetEntry(nEntry));
    }
    if (nLstType)
        m_pTypeBox->SelectEntry(aItem.m_aLstType);
    else
        m_pTypeBox->SelectEntryPos(0);
    m_pTypeBox->GetSelectHdl().Call(*m_pTypeBox);
}

IMPL_LINK_NOARG(SwLabPage, TypeHdl, ListBox&, void)
{
    DisplayFormat();
    aItem.m_aType = m_pTypeBox->GetSelectEntry();
}

void SwLabPage::DisplayFormat()
{
    ScopedVclPtrInstance< MetricField > aField(this, WinBits(0));
    FieldUnit aMetric = ::GetDfltMetric(false);
    SetMetric(*aField.get(), aMetric);
    aField->SetDecimalDigits(2);
    aField->SetMin         (0);
    aField->SetMax         (LONG_MAX);

    SwLabRec* pRec = GetSelectedEntryPos();
    aItem.m_aLstType = pRec->aType;
    SETFLDVAL(*aField.get(), pRec->lWidth);
    aField->Reformat();
    const OUString aWString = aField->GetText();

    SETFLDVAL(*aField.get(), pRec->lHeight);
    aField->Reformat();

    OUString aText = pRec->aType + ": " + aWString +
           " x " + aField->GetText() +
           " (" + OUString::number( pRec->nCols ) +
           " x " + OUString::number( pRec->nRows ) + ")";
    m_pFormatInfo->SetText(aText);
}

SwLabRec* SwLabPage::GetSelectedEntryPos()
{
    OUString sSelEntry(m_pTypeBox->GetSelectEntry());

    return GetParentSwLabDlg()->GetRecord(sSelEntry, m_pContButton->IsChecked());
}

void SwLabPage::InitDatabaseBox()
{
    if( GetDBManager() )
    {
        m_pDatabaseLB->Clear();
        css::uno::Sequence<OUString> aDataNames = SwDBManager::GetExistingDatabaseNames();
        const OUString* pDataNames = aDataNames.getConstArray();
        for (long i = 0; i < aDataNames.getLength(); i++)
            m_pDatabaseLB->InsertEntry(pDataNames[i]);
        OUString sDBName = sActDBName.getToken( 0, DB_DELIM );
        OUString sTableName = sActDBName.getToken( 1, DB_DELIM );
        m_pDatabaseLB->SelectEntry(sDBName);
        if( !sDBName.isEmpty() && GetDBManager()->GetTableNames(m_pTableLB, sDBName))
        {
            m_pTableLB->SelectEntry(sTableName);
            GetDBManager()->GetColumnNames(m_pDBFieldLB, sActDBName, sTableName);
        }
        else
            m_pDBFieldLB->Clear();
    }
}

VclPtr<SfxTabPage> SwLabPage::Create(vcl::Window* pParent, const SfxItemSet* rSet)
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
    rItem.m_bAddr    = m_pAddrBox->IsChecked();
    rItem.m_aWriting = m_pWritingEdit->GetText();
    rItem.m_bCont    = m_pContButton->IsChecked();
    rItem.m_aMake    = m_pMakeBox->GetSelectEntry();
    rItem.m_aType    = m_pTypeBox->GetSelectEntry();
    rItem.m_sDBName  = sActDBName;

    SwLabRec* pRec = GetSelectedEntryPos();
    pRec->FillItem( rItem );

    rItem.m_aLstMake = m_pMakeBox->GetSelectEntry();
    rItem.m_aLstType = m_pTypeBox->GetSelectEntry();
}

bool SwLabPage::FillItemSet(SfxItemSet* rSet)
{
    FillItem( aItem );
    rSet->Put( aItem );

    return true;
}

void SwLabPage::Reset(const SfxItemSet* rSet)
{
    aItem = static_cast<const SwLabItem&>( rSet->Get(FN_LABEL));
    OUString sDBName  = aItem.m_sDBName;

    OUString aWriting(convertLineEnd(aItem.m_aWriting, GetSystemLineEnd()));

    m_pAddrBox->Check( aItem.m_bAddr );
    m_pWritingEdit->SetText    ( aWriting );

    for(std::vector<OUString>::const_iterator i = GetParentSwLabDlg()->Makes().begin(); i != GetParentSwLabDlg()->Makes().end(); ++i)
    {
        if(m_pMakeBox->GetEntryPos(*i) == LISTBOX_ENTRY_NOTFOUND)
            m_pMakeBox->InsertEntry(*i);
    }

    m_pMakeBox->SelectEntry( aItem.m_aMake );
    //save the current type
    OUString sType(aItem.m_aType);
    m_pMakeBox->GetSelectHdl().Call(*m_pMakeBox);
    aItem.m_aType = sType;
    //#102806# a newly added make may not be in the type ListBox already
    if (m_pTypeBox->GetEntryPos(aItem.m_aType) == LISTBOX_ENTRY_NOTFOUND && !aItem.m_aMake.isEmpty())
        GetParentSwLabDlg()->UpdateGroup( aItem.m_aMake );
    if (m_pTypeBox->GetEntryPos(aItem.m_aType) != LISTBOX_ENTRY_NOTFOUND)
    {
        m_pTypeBox->SelectEntry(aItem.m_aType);
        m_pTypeBox->GetSelectHdl().Call(*m_pTypeBox);
    }
    if (m_pDatabaseLB->GetEntryPos(sDBName) != LISTBOX_ENTRY_NOTFOUND)
    {
        m_pDatabaseLB->SelectEntry(sDBName);
        m_pDatabaseLB->GetSelectHdl().Call(*m_pDatabaseLB);
    }

    if (aItem.m_bCont)
        m_pContButton->Check();
    else
        m_pSheetButton->Check();
}

void SwVisitingCardPage::ClearUserData()
{
    SvTreeListEntry* pEntry = m_pAutoTextLB->First();
    while(pEntry)
    {
        delete static_cast<OUString*>(pEntry->GetUserData());
        pEntry = m_pAutoTextLB->Next(pEntry);
    }
}

void SwVisitingCardPage::SetUserData( sal_uInt32 nCnt,
                const OUString* pNames, const OUString* pValues )
{
    for( sal_uInt32 i = 0; i < nCnt; ++i )
    {
        SvTreeListEntry* pEntry = m_pAutoTextLB->InsertEntry( pNames[ i ] );
        pEntry->SetUserData( new OUString( pValues[ i ] ));
    }
}

SwVisitingCardPage::SwVisitingCardPage(vcl::Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "CardFormatPage",
        "modules/swriter/ui/cardformatpage.ui", &rSet)
    , pExampleFrame(nullptr)
{
    get(m_pAutoTextLB, "treeview");
    m_pAutoTextLB->set_height_request(m_pAutoTextLB->GetTextHeight() * 16);
    get(m_pAutoTextGroupLB, "autotext");
    get(m_pExampleWIN, "preview");

    m_pAutoTextLB->SetStyle( m_pAutoTextLB->GetStyle() | WB_HSCROLL );
    m_pAutoTextLB->SetSpaceBetweenEntries(0);
    m_pAutoTextLB->SetSelectionMode( SelectionMode::Single );

    SetExchangeSupport();
    m_pAutoTextLB->SetSelectHdl(LINK(this, SwVisitingCardPage, AutoTextSelectTreeListBoxHdl));
    m_pAutoTextGroupLB->SetSelectHdl(LINK(this, SwVisitingCardPage, AutoTextSelectHdl));

    m_pExampleWIN->Hide();

    InitFrameControl();
}

SwVisitingCardPage::~SwVisitingCardPage()
{
    disposeOnce();
}

void SwVisitingCardPage::dispose()
{
    for(sal_Int32 i = 0; i < m_pAutoTextGroupLB->GetEntryCount(); ++i)
        delete static_cast<OUString*>(m_pAutoTextGroupLB->GetEntryData( i ));
    m_xAutoText = nullptr;

    ClearUserData();
    delete pExampleFrame;
    m_pAutoTextLB.clear();
    m_pAutoTextGroupLB.clear();
    m_pExampleWIN.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SwVisitingCardPage::Create(vcl::Window* pParent, const SfxItemSet* rSet)
{
    return VclPtr<SwVisitingCardPage>::Create(pParent, *rSet);
}

void SwVisitingCardPage::ActivatePage(const SfxItemSet& rSet)
{
    Reset( &rSet );
    UpdateFields();
}

DeactivateRC SwVisitingCardPage::DeactivatePage(SfxItemSet* _pSet)
{
    if (_pSet)
        FillItemSet(_pSet);
    return DeactivateRC::LeavePage;
}

bool SwVisitingCardPage::FillItemSet(SfxItemSet* rSet)
{
    const OUString* pGroup = static_cast<const OUString*>(m_pAutoTextGroupLB->GetSelectEntryData());
    OSL_ENSURE(pGroup, "no group selected?");

    if (pGroup)
        aLabItem.m_sGlossaryGroup = *pGroup;

    SvTreeListEntry* pSelEntry = m_pAutoTextLB->FirstSelected();
    if(pSelEntry)
        aLabItem.m_sGlossaryBlockName = *static_cast<OUString*>(pSelEntry->GetUserData());
    rSet->Put(aLabItem);
    return true;
}

static void lcl_SelectBlock(SvTreeListBox& rAutoTextLB, const OUString& rBlockName)
{
    SvTreeListEntry* pEntry = rAutoTextLB.First();
    while(pEntry)
    {
        if(*static_cast<OUString*>(pEntry->GetUserData()) == rBlockName)
        {
            rAutoTextLB.Select(pEntry);
            rAutoTextLB.MakeVisible(pEntry);
            break;
        }
        pEntry = rAutoTextLB.Next(pEntry);
    }
}

static bool lcl_FindBlock(SvTreeListBox& rAutoTextLB, const OUString& rBlockName)
{
    SvTreeListEntry* pEntry = rAutoTextLB.First();
    while(pEntry)
    {
        if(*static_cast<OUString*>(pEntry->GetUserData()) == rBlockName)
        {
            rAutoTextLB.Select(pEntry);
            return true;
        }
        pEntry = rAutoTextLB.Next(pEntry);
    }
    return false;
}

void SwVisitingCardPage::Reset(const SfxItemSet* rSet)
{
    aLabItem = static_cast<const SwLabItem&>( rSet->Get(FN_LABEL) );

    bool bFound = false;
    sal_Int32 i;
    for(i = 0; i < m_pAutoTextGroupLB->GetEntryCount(); i++)
        if( aLabItem.m_sGlossaryGroup == *static_cast<const OUString*>(m_pAutoTextGroupLB->GetEntryData( i )))
        {
            bFound = true;
            break;
        }

    if(!bFound)
    {
        // initially search for a group starting with "crd" which is the name of the
        // business card AutoTexts
        for(i = 0; i < m_pAutoTextGroupLB->GetEntryCount(); i++)
            if (static_cast<const OUString*>(m_pAutoTextGroupLB->GetEntryData(i))->startsWith("crd"))
            {
                bFound = true;
                break;
            }
    }
    if(bFound)
    {
        if(m_pAutoTextGroupLB->GetSelectEntryPos() != i)
        {
            m_pAutoTextGroupLB->SelectEntryPos(i);
            AutoTextSelectHdl(*m_pAutoTextGroupLB);
        }
        if(lcl_FindBlock(*m_pAutoTextLB, aLabItem.m_sGlossaryBlockName))
        {
            SvTreeListEntry* pSelEntry = m_pAutoTextLB->FirstSelected();
            if( pSelEntry &&
                *static_cast<OUString*>(pSelEntry->GetUserData()) != aLabItem.m_sGlossaryBlockName)
            {
                lcl_SelectBlock(*m_pAutoTextLB, aLabItem.m_sGlossaryBlockName);
                if(m_xAutoText.is() && pExampleFrame->IsInitialized())
                    pExampleFrame->ClearDocument();
            }
        }
    }
}

SwPrivateDataPage::SwPrivateDataPage(vcl::Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "PrivateUserPage",
        "modules/swriter/ui/privateuserpage.ui", &rSet)
{
    get(m_pFirstNameED, "firstname");
    get(m_pNameED, "lastname");
    get(m_pShortCutED, "shortname");
    get(m_pFirstName2ED, "firstname2");
    get(m_pName2ED, "lastname2");
    get(m_pShortCut2ED, "shortname2");
    get(m_pStreetED, "street");
    get(m_pZipED, "izip");
    get(m_pCityED, "icity");
    get(m_pCountryED, "country");
    get(m_pStateED, "state");
    get(m_pTitleED, "title");
    get(m_pProfessionED, "job");
    get(m_pPhoneED, "phone");
    get(m_pMobilePhoneED, "mobile");
    get(m_pFaxED, "fax");
    get(m_pHomePageED, "url");
    get(m_pMailED, "email");

    SetExchangeSupport();
}

SwPrivateDataPage::~SwPrivateDataPage()
{
    disposeOnce();
}

void SwPrivateDataPage::dispose()
{
    m_pFirstNameED.clear();
    m_pNameED.clear();
    m_pShortCutED.clear();
    m_pFirstName2ED.clear();
    m_pName2ED.clear();
    m_pShortCut2ED.clear();
    m_pStreetED.clear();
    m_pZipED.clear();
    m_pCityED.clear();
    m_pCountryED.clear();
    m_pStateED.clear();
    m_pTitleED.clear();
    m_pProfessionED.clear();
    m_pPhoneED.clear();
    m_pMobilePhoneED.clear();
    m_pFaxED.clear();
    m_pHomePageED.clear();
    m_pMailED.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SwPrivateDataPage::Create(vcl::Window* pParent, const SfxItemSet* rSet)
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

    SwLabItem aItem = static_cast<const SwLabItem&>( GetTabDialog()->GetExampleSet()->Get(FN_LABEL) );
    aItem.m_aPrivFirstName = m_pFirstNameED->GetText();
    aItem.m_aPrivName      = m_pNameED->GetText(  );
    aItem.m_aPrivShortCut  = m_pShortCutED->GetText(  );
    aItem.m_aPrivFirstName2 = m_pFirstName2ED->GetText();
    aItem.m_aPrivName2     = m_pName2ED->GetText(  );
    aItem.m_aPrivShortCut2 = m_pShortCut2ED->GetText(  );
    aItem.m_aPrivStreet    = m_pStreetED->GetText(  );
    aItem.m_aPrivZip       = m_pZipED->GetText(  );
    aItem.m_aPrivCity      = m_pCityED->GetText(  );
    aItem.m_aPrivCountry   = m_pCountryED->GetText(  );
    aItem.m_aPrivState     = m_pStateED->GetText(  );
    aItem.m_aPrivTitle     = m_pTitleED->GetText(  );
    aItem.m_aPrivProfession= m_pProfessionED->GetText(   );
    aItem.m_aPrivPhone     = m_pPhoneED->GetText(  );
    aItem.m_aPrivMobile    = m_pMobilePhoneED->GetText(  );
    aItem.m_aPrivFax       = m_pFaxED->GetText(  );
    aItem.m_aPrivWWW       = m_pHomePageED->GetText(  );
    aItem.m_aPrivMail      = m_pMailED->GetText(  );

    rSet->Put(aItem);
    return true;
}

void SwPrivateDataPage::Reset(const SfxItemSet* rSet)
{
    const SwLabItem& aItem = static_cast<const SwLabItem&>( rSet->Get(FN_LABEL) );
    m_pFirstNameED->SetText(aItem.m_aPrivFirstName);
    m_pNameED->SetText(aItem.m_aPrivName);
    m_pShortCutED->SetText(aItem.m_aPrivShortCut);
    m_pFirstName2ED->SetText(aItem.m_aPrivFirstName2);
    m_pName2ED->SetText(aItem.m_aPrivName2);
    m_pShortCut2ED->SetText(aItem.m_aPrivShortCut2);
    m_pStreetED->SetText(aItem.m_aPrivStreet);
    m_pZipED->SetText(aItem.m_aPrivZip);
    m_pCityED->SetText(aItem.m_aPrivCity);
    m_pCountryED->SetText(aItem.m_aPrivCountry);
    m_pStateED->SetText(aItem.m_aPrivState);
    m_pTitleED->SetText(aItem.m_aPrivTitle);
    m_pProfessionED->SetText(aItem.m_aPrivProfession);
    m_pPhoneED->SetText(aItem.m_aPrivPhone);
    m_pMobilePhoneED->SetText(aItem.m_aPrivMobile);
    m_pFaxED->SetText(aItem.m_aPrivFax);
    m_pHomePageED->SetText(aItem.m_aPrivWWW);
    m_pMailED->SetText(aItem.m_aPrivMail);
}

SwBusinessDataPage::SwBusinessDataPage(vcl::Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "BusinessDataPage",
        "modules/swriter/ui/businessdatapage.ui", &rSet)
{
    get(m_pCompanyED, "company");
    get(m_pCompanyExtED, "company2");
    get(m_pSloganED, "slogan");
    get(m_pStreetED, "street");
    get(m_pZipED, "izip");
    get(m_pCityED, "icity");
    get(m_pCountryED, "country");
    get(m_pStateED, "state");
    get(m_pPositionED, "position");
    get(m_pPhoneED, "phone");
    get(m_pMobilePhoneED, "mobile");
    get(m_pFaxED, "fax");
    get(m_pHomePageED, "url");
    get(m_pMailED, "email");
    SetExchangeSupport();
}

SwBusinessDataPage::~SwBusinessDataPage()
{
    disposeOnce();
}

void SwBusinessDataPage::dispose()
{
    m_pCompanyED.clear();
    m_pCompanyExtED.clear();
    m_pSloganED.clear();
    m_pStreetED.clear();
    m_pZipED.clear();
    m_pCityED.clear();
    m_pCountryED.clear();
    m_pStateED.clear();
    m_pPositionED.clear();
    m_pPhoneED.clear();
    m_pMobilePhoneED.clear();
    m_pFaxED.clear();
    m_pHomePageED.clear();
    m_pMailED.clear();
    SfxTabPage::dispose();
}


VclPtr<SfxTabPage> SwBusinessDataPage::Create(vcl::Window* pParent, const SfxItemSet* rSet)
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
    SwLabItem aItem = static_cast<const SwLabItem&>( GetTabDialog()->GetExampleSet()->Get(FN_LABEL) );

    aItem.m_aCompCompany   = m_pCompanyED->GetText();
    aItem.m_aCompCompanyExt= m_pCompanyExtED->GetText();
    aItem.m_aCompSlogan    = m_pSloganED->GetText();
    aItem.m_aCompStreet    = m_pStreetED->GetText();
    aItem.m_aCompZip       = m_pZipED->GetText();
    aItem.m_aCompCity      = m_pCityED->GetText();
    aItem.m_aCompCountry   = m_pCountryED->GetText();
    aItem.m_aCompState     = m_pStateED->GetText();
    aItem.m_aCompPosition  = m_pPositionED->GetText();
    aItem.m_aCompPhone     = m_pPhoneED->GetText();
    aItem.m_aCompMobile    = m_pMobilePhoneED->GetText();
    aItem.m_aCompFax       = m_pFaxED->GetText();
    aItem.m_aCompWWW       = m_pHomePageED->GetText();
    aItem.m_aCompMail      = m_pMailED->GetText();

    rSet->Put(aItem);
    return true;
}

void SwBusinessDataPage::Reset(const SfxItemSet* rSet)
{
    const SwLabItem& aItem = static_cast<const SwLabItem&>( rSet->Get(FN_LABEL) );
    m_pCompanyED->SetText(aItem.m_aCompCompany);
    m_pCompanyExtED->SetText(aItem.m_aCompCompanyExt);
    m_pSloganED->SetText(aItem.m_aCompSlogan);
    m_pStreetED->SetText(aItem.m_aCompStreet);
    m_pZipED->SetText(aItem.m_aCompZip);
    m_pCityED->SetText(aItem.m_aCompCity);
    m_pCountryED->SetText(aItem.m_aCompCountry);
    m_pStateED->SetText(aItem.m_aCompState);
    m_pPositionED->SetText(aItem.m_aCompPosition);
    m_pPhoneED->SetText(aItem.m_aCompPhone);
    m_pMobilePhoneED->SetText(aItem.m_aCompMobile);
    m_pFaxED->SetText(aItem.m_aCompFax);
    m_pHomePageED->SetText(aItem.m_aCompWWW);
    m_pMailED->SetText(aItem.m_aCompMail);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
