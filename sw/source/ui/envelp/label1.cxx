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

#include <vcl/waitobj.hxx>
#include <rtl/ustring.hxx>
#include "svtools/treelistentry.hxx"
#include <com/sun/star/uno/Sequence.h>
#include <swtypes.hxx>
#include <wrtsh.hxx>
#include <initui.hxx>
#include <labimp.hxx>
#include <labfmt.hxx>
#include <labprt.hxx>
#include <unotools.hxx>
#include <dbmgr.hxx>
#include "uitool.hxx"
#include <cmdid.h>
#include <helpid.h>
#include <globals.hrc>
#include <label.hrc>

//impl in envimg.cxx
extern SW_DLLPUBLIC String MakeSender();


void SwLabRec::SetFromItem( const SwLabItem& rItem )
{
    lHDist  = rItem.lHDist;
    lVDist  = rItem.lVDist;
    lWidth  = rItem.lWidth;
    lHeight = rItem.lHeight;
    lLeft   = rItem.lLeft;
    lUpper  = rItem.lUpper;
    nCols   = rItem.nCols;
    nRows   = rItem.nRows;
    lPWidth  = rItem.lPWidth;
    lPHeight = rItem.lPHeight;
    bCont   = rItem.bCont;
}

void SwLabRec::FillItem( SwLabItem& rItem ) const
{
    rItem.lHDist  = lHDist;
    rItem.lVDist  = lVDist;
    rItem.lWidth  = lWidth;
    rItem.lHeight = lHeight;
    rItem.lLeft   = lLeft;
    rItem.lUpper  = lUpper;
    rItem.nCols   = nCols;
    rItem.lPWidth  = lPWidth;
    rItem.lPHeight = lPHeight;
    rItem.nRows   = nRows;
}

void SwLabDlg::_ReplaceGroup( const String &rMake )
{
    // Remove old entries
    pRecs->erase(pRecs->begin() + 1, pRecs->end());
    aLabelsCfg.FillLabels(OUString(rMake), *pRecs);
    aLstGroup = rMake;
}

void SwLabDlg::PageCreated(sal_uInt16 nId, SfxTabPage &rPage)
{
    if (nId == TP_LAB_LAB)
    {
        if(m_bLabel)
        {
            ((SwLabPage*)&rPage)->SetNewDBMgr(pNewDBMgr);
            ((SwLabPage*)&rPage)->InitDatabaseBox();
        }
        else
            ((SwLabPage*)&rPage)->SetToBusinessCard();
    }
    else if (nId == TP_LAB_PRT)
        pPrtPage = (SwLabPrtPage*)&rPage;
}

SwLabDlg::SwLabDlg(Window* pParent, const SfxItemSet& rSet,
                                SwNewDBMgr* pDBMgr, sal_Bool bLabel) :
    SfxTabDialog( pParent, SW_RES(DLG_LAB), &rSet, sal_False ),
    pNewDBMgr(pDBMgr),
    pPrtPage(0),
    aTypeIds( 50, 10 ),
    pRecs   ( new SwLabRecs() ),
    sBusinessCardDlg(SW_RES(ST_BUSINESSCARDDLG)),
    sFormat(SW_RES(ST_FIRSTPAGE_LAB)),
    sMedium(SW_RES(ST_FIRSTPAGE_BC)),
    m_bLabel(bLabel)
{
    WaitObject aWait( pParent );

    FreeResource();

    GetOKButton().SetText(String(SW_RES(STR_BTN_NEW_DOC)));
    GetOKButton().SetHelpId(HID_LABEL_INSERT);
    GetOKButton().SetHelpText(aEmptyStr);   // in order for generated help text to get used

    AddTabPage(TP_LAB_LAB, m_bLabel ? sFormat : sMedium ,SwLabPage   ::Create, 0, sal_False, 0);
    AddTabPage(TP_VISITING_CARDS, SwVisitingCardPage::Create, 0);
    AddTabPage(TP_LAB_FMT, SwLabFmtPage::Create, 0);
    AddTabPage(TP_LAB_PRT, SwLabPrtPage::Create, 0);
    AddTabPage(TP_BUSINESS_DATA, SwBusinessDataPage::Create, 0 );
    AddTabPage(TP_PRIVATE_DATA, SwPrivateDataPage::Create, 0);


    if(m_bLabel)
    {
        RemoveTabPage(TP_BUSINESS_DATA);
        RemoveTabPage(TP_PRIVATE_DATA);
        RemoveTabPage(TP_VISITING_CARDS);
    }
    else
    {
        SetText(sBusinessCardDlg);
    }
    // Read user label from writer.cfg
    SwLabItem aItem((const SwLabItem&)rSet.Get( FN_LABEL ));
    SwLabRec* pRec = new SwLabRec;
    const String aTmp( SW_RES( STR_CUSTOM ) );
    pRec->aMake   = pRec->aType = aTmp;
    pRec->SetFromItem( aItem );

    bool bDouble = false;

    for (sal_uInt16 nRecPos = 0; nRecPos < pRecs->size(); nRecPos++)
    {
        if (pRec->aMake == (*pRecs)[nRecPos]->aMake &&
            pRec->aType == (*pRecs)[nRecPos]->aType)
        {
            bDouble = true;
            break;
        }
    }

    if (!bDouble)
        pRecs->insert( pRecs->begin(), pRec );

    sal_uInt16 nLstGroup = 0;
    const std::vector<OUString>& rMan = aLabelsCfg.GetManufacturers();
    for(sal_uInt16 nMan = 0; nMan < rMan.size(); nMan++)
    {
        aMakes.push_back(rMan[nMan]);
        if ( rMan[nMan] == aItem.aLstMake )
            nLstGroup = nMan;
    }

    if ( !aMakes.empty() )
        _ReplaceGroup( aMakes[nLstGroup] );

    if (pExampleSet)
        pExampleSet->Put(aItem);
}

SwLabDlg::~SwLabDlg()
{
    delete pRecs;
}

void SwLabDlg::GetLabItem(SwLabItem &rItem)
{
    const SwLabItem& rActItem = (const SwLabItem&)GetExampleSet()->Get(FN_LABEL);
    const SwLabItem& rOldItem = (const SwLabItem&)GetInputSetImpl()->Get(FN_LABEL);

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
        SwLabRec* pRec = GetRecord(rItem.aType, rItem.bCont);
        pRec->FillItem( rItem );
    }
}

SwLabRec* SwLabDlg::GetRecord(const String &rRecName, sal_Bool bCont)
{
    SwLabRec* pRec = NULL;
    bool bFound = false;
    String sCustom(SW_RES(STR_CUSTOM));

    const sal_uInt16 nCount = Recs().size();
    for (sal_uInt16 i = 0; i < nCount; i++)
    {
        pRec = Recs()[i];
        if (pRec->aType != sCustom &&
            rRecName == pRec->aType && bCont == pRec->bCont)
        {
            bFound = true;
            break;
        }
    }
    if (!bFound)    // User defined
        pRec = Recs()[0];

    return(pRec);
}

Printer *SwLabDlg::GetPrt()
{
    if (pPrtPage)
        return (pPrtPage->GetPrt());
    else
        return (NULL);
}

SwLabPage::SwLabPage(Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "CardMediumPage",
        "modules/swriter/ui/cardmediumpage.ui", rSet)
    , pNewDBMgr(NULL)
    , aItem((const SwLabItem&)rSet.Get(FN_LABEL))
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

    const sal_uInt16 nCount = (sal_uInt16)GetParentSwLabDlg()->Makes().size();
    for(size_t i = 0; i < nCount; ++i)
    {
        OUString& rStr = GetParentSwLabDlg()->Makes()[i];
        m_pMakeBox->InsertEntry( rStr );

        if ( rStr == aItem.aLstMake)
            nLstGroup = i;
    }

    m_pMakeBox->SelectEntryPos( nLstGroup );
    m_pMakeBox->GetSelectHdl().Call(m_pMakeBox);
}

void SwLabPage::SetToBusinessCard()
{
    SetHelpId(HID_BUSINESS_FMT_PAGE);
    m_pContButton->SetHelpId(HID_BUSINESS_FMT_PAGE_CONT);
    m_pSheetButton->SetHelpId(HID_BUSINESS_FMT_PAGE_SHEET);
    m_pMakeBox->SetHelpId(HID_BUSINESS_FMT_PAGE_BRAND);
    m_pTypeBox->SetHelpId(HID_BUSINESS_FMT_PAGE_TYPE);
    m_bLabel = sal_False;
    m_pAddressFrame->Hide();
};

IMPL_LINK_NOARG(SwLabPage, AddrHdl)
{
    String aWriting;

    if ( m_pAddrBox->IsChecked() )
        aWriting = convertLineEnd(MakeSender(), GetSystemLineEnd());

    m_pWritingEdit->SetText( aWriting );
    m_pWritingEdit->GrabFocus();
    return 0;
}

IMPL_LINK( SwLabPage, DatabaseHdl, ListBox *, pListBox )
{
    sActDBName = m_pDatabaseLB->GetSelectEntry();

    WaitObject aObj( GetParentSwLabDlg() );

    if (pListBox == m_pDatabaseLB)
        GetNewDBMgr()->GetTableNames(m_pTableLB, sActDBName);

    GetNewDBMgr()->GetColumnNames(m_pDBFieldLB, sActDBName, m_pTableLB->GetSelectEntry());
    return 0;
}

IMPL_LINK_NOARG(SwLabPage, FieldHdl)
{
    OUString aStr("<" + OUString(m_pDatabaseLB->GetSelectEntry()) + "." +
                  OUString(m_pTableLB->GetSelectEntry()) + "." +
                  (m_pTableLB->GetEntryData(m_pTableLB->GetSelectEntryPos()) == 0 ? OUString("0") : OUString("1")) + "." +
                  m_pDBFieldLB->GetSelectEntry() + ">");
    m_pWritingEdit->ReplaceSelected(aStr);
    Selection aSel = m_pWritingEdit->GetSelection();
    m_pWritingEdit->GrabFocus();
    m_pWritingEdit->SetSelection(aSel);
    return 0;
}

IMPL_LINK_NOARG_INLINE_START(SwLabPage, PageHdl)
{
    m_pMakeBox->GetSelectHdl().Call(m_pMakeBox);
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwLabPage, PageHdl)

IMPL_LINK_NOARG(SwLabPage, MakeHdl)
{
    WaitObject aWait( GetParentSwLabDlg() );

    m_pTypeBox->Clear();
    m_pHiddenSortTypeBox->Clear();
    GetParentSwLabDlg()->TypeIds().clear();

    const String aMake = m_pMakeBox->GetSelectEntry();
    GetParentSwLabDlg()->ReplaceGroup( aMake );
    aItem.aLstMake = aMake;

    const sal_Bool   bCont    = m_pContButton->IsChecked();
    const sal_uInt16 nCount   = GetParentSwLabDlg()->Recs().size();
          sal_uInt16 nLstType = 0;

    const String sCustom(SW_RES(STR_CUSTOM));
    //insert the entries into the sorted list box
    for ( sal_uInt16 i = 0; i < nCount; ++i )
    {
        const String aType ( GetParentSwLabDlg()->Recs()[i]->aType );
        bool bInsert = false;
        if ( GetParentSwLabDlg()->Recs()[i]->aType == sCustom )
        {
            bInsert = true;
            m_pTypeBox->InsertEntry(aType );
        }
        else if ( GetParentSwLabDlg()->Recs()[i]->bCont == bCont )
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
            if ( !nLstType && aType == String(aItem.aLstType) )
                nLstType = GetParentSwLabDlg()->TypeIds().size();
        }
    }
    for(sal_uInt16 nEntry = 0; nEntry < m_pHiddenSortTypeBox->GetEntryCount(); nEntry++)
    {
        m_pTypeBox->InsertEntry(m_pHiddenSortTypeBox->GetEntry(nEntry));
    }
    if (nLstType)
        m_pTypeBox->SelectEntry(aItem.aLstType);
    else
        m_pTypeBox->SelectEntryPos(0);
    m_pTypeBox->GetSelectHdl().Call(m_pTypeBox);
    return 0;
}

IMPL_LINK_NOARG_INLINE_START(SwLabPage, TypeHdl)
{
    DisplayFormat();
    aItem.aType = m_pTypeBox->GetSelectEntry();
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwLabPage, TypeHdl)

void SwLabPage::DisplayFormat()
{
    MetricField aField(this, WinBits(0));
    FieldUnit aMetric = ::GetDfltMetric(sal_False);
    SetMetric(aField, aMetric);
    aField.SetDecimalDigits(2);
    aField.SetMin         (0);
    aField.SetMax         (LONG_MAX);

    SwLabRec* pRec = GetSelectedEntryPos();
    aItem.aLstType = pRec->aType;
    SETFLDVAL(aField, pRec->lWidth);
    aField.Reformat();
    const String aWString = aField.GetText();

    SETFLDVAL(aField, pRec->lHeight);
    aField.Reformat();

    String aText = pRec->aType;
    aText.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": "));
    aText += aWString;
    aText.AppendAscii( RTL_CONSTASCII_STRINGPARAM(" x "));
    aText += aField.GetText();
    aText.AppendAscii( RTL_CONSTASCII_STRINGPARAM(" ("));
    aText += OUString::number( pRec->nCols );
    aText.AppendAscii( RTL_CONSTASCII_STRINGPARAM(" x "));
    aText += OUString::number( pRec->nRows );
    aText += ')';
    m_pFormatInfo->SetText(aText);
}

SwLabRec* SwLabPage::GetSelectedEntryPos()
{
    String sSelEntry(m_pTypeBox->GetSelectEntry());

    return GetParentSwLabDlg()->GetRecord(sSelEntry, m_pContButton->IsChecked());
}

void SwLabPage::InitDatabaseBox()
{
    if( GetNewDBMgr() )
    {
        m_pDatabaseLB->Clear();
        ::com::sun::star::uno::Sequence<OUString> aDataNames = SwNewDBMgr::GetExistingDatabaseNames();
        const OUString* pDataNames = aDataNames.getConstArray();
        for (long i = 0; i < aDataNames.getLength(); i++)
            m_pDatabaseLB->InsertEntry(pDataNames[i]);
        String sDBName = sActDBName.GetToken( 0, DB_DELIM );
        String sTableName = sActDBName.GetToken( 1, DB_DELIM );
        m_pDatabaseLB->SelectEntry(sDBName);
        if( sDBName.Len() && GetNewDBMgr()->GetTableNames(m_pTableLB, sDBName))
        {
            m_pTableLB->SelectEntry(sTableName);
            GetNewDBMgr()->GetColumnNames(m_pDBFieldLB, sActDBName, sTableName);
        }
        else
            m_pDBFieldLB->Clear();
    }
}

SfxTabPage* SwLabPage::Create(Window* pParent, const SfxItemSet& rSet)
{
    return new SwLabPage(pParent, rSet);
}

void SwLabPage::ActivatePage(const SfxItemSet& rSet)
{
    Reset( rSet );
}

int SwLabPage::DeactivatePage(SfxItemSet* _pSet)
{
    if (_pSet)
        FillItemSet(*_pSet);

    return sal_True;
}

void SwLabPage::FillItem(SwLabItem& rItem)
{
    rItem.bAddr    = m_pAddrBox->IsChecked();
    rItem.aWriting = m_pWritingEdit->GetText();
    rItem.bCont    = m_pContButton->IsChecked();
    rItem.aMake    = m_pMakeBox->GetSelectEntry();
    rItem.aType    = m_pTypeBox->GetSelectEntry();
    rItem.sDBName  = sActDBName;

    SwLabRec* pRec = GetSelectedEntryPos();
    pRec->FillItem( rItem );

    rItem.aLstMake = m_pMakeBox->GetSelectEntry();
    rItem.aLstType = m_pTypeBox->GetSelectEntry();
}

sal_Bool SwLabPage::FillItemSet(SfxItemSet& rSet)
{
    FillItem( aItem );
    rSet.Put( aItem );

    return sal_True;
}

void SwLabPage::Reset(const SfxItemSet& rSet)
{
    aItem = (const SwLabItem&) rSet.Get(FN_LABEL);
    String sDBName  = aItem.sDBName;

    String aWriting(convertLineEnd(aItem.aWriting, GetSystemLineEnd()));

    m_pAddrBox->Check( aItem.bAddr );
    m_pWritingEdit->SetText    ( aWriting );

    for(std::vector<OUString>::const_iterator i = GetParentSwLabDlg()->Makes().begin(); i != GetParentSwLabDlg()->Makes().end(); ++i)
    {
        if(m_pMakeBox->GetEntryPos(String(*i)) == LISTBOX_ENTRY_NOTFOUND)
            m_pMakeBox->InsertEntry(*i);
    }

    m_pMakeBox->SelectEntry( aItem.aMake );
    //save the current type
    String sType(aItem.aType);
    m_pMakeBox->GetSelectHdl().Call(m_pMakeBox);
    aItem.aType = sType;
    //#102806# a newly added make may not be in the type ListBox already
    if (m_pTypeBox->GetEntryPos(String(aItem.aType)) == LISTBOX_ENTRY_NOTFOUND && !aItem.aMake.isEmpty())
        GetParentSwLabDlg()->UpdateGroup( aItem.aMake );
    if (m_pTypeBox->GetEntryPos(String(aItem.aType)) != LISTBOX_ENTRY_NOTFOUND)
    {
        m_pTypeBox->SelectEntry(aItem.aType);
        m_pTypeBox->GetSelectHdl().Call(m_pTypeBox);
    }
    if (m_pDatabaseLB->GetEntryPos(sDBName) != LISTBOX_ENTRY_NOTFOUND)
    {
        m_pDatabaseLB->SelectEntry(sDBName);
        m_pDatabaseLB->GetSelectHdl().Call(m_pDatabaseLB);
    }

    if (aItem.bCont)
        m_pContButton->Check();
    else
        m_pSheetButton->Check();
}

void SwVisitingCardPage::ClearUserData()
{
    SvTreeListEntry* pEntry = m_pAutoTextLB->First();
    while(pEntry)
    {
        delete (String*)pEntry->GetUserData();
        pEntry = m_pAutoTextLB->Next(pEntry);
    }
}

void SwVisitingCardPage::SetUserData( sal_uInt32 nCnt,
                const OUString* pNames, const OUString* pValues )
{
    for( sal_uInt32 i = 0; i < nCnt; ++i )
    {
        SvTreeListEntry* pEntry = m_pAutoTextLB->InsertEntry( pNames[ i ] );
        pEntry->SetUserData( new String( pValues[ i ] ));
    }
}

SwVisitingCardPage::SwVisitingCardPage(Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "CardFormatPage",
        "modules/swriter/ui/cardformatpage.ui", rSet)
    , pExampleFrame(0)
{
    get(m_pAutoTextLB, "treeview");
    m_pAutoTextLB->set_height_request(m_pAutoTextLB->GetTextHeight() * 16);
    get(m_pAutoTextGroupLB, "autotext");
    get(m_pExampleWIN, "preview");

    m_pAutoTextLB->SetStyle( m_pAutoTextLB->GetStyle() | WB_HSCROLL );
    m_pAutoTextLB->SetSpaceBetweenEntries(0);
    m_pAutoTextLB->SetSelectionMode( SINGLE_SELECTION );

    SetExchangeSupport();
    m_pAutoTextLB->SetSelectHdl(LINK(this, SwVisitingCardPage, AutoTextSelectHdl));
    m_pAutoTextGroupLB->SetSelectHdl(LINK(this, SwVisitingCardPage, AutoTextSelectHdl));

    m_pExampleWIN->Hide();

    InitFrameControl();
}

SwVisitingCardPage::~SwVisitingCardPage()
{
    for(sal_uInt16 i = 0; i < m_pAutoTextGroupLB->GetEntryCount(); i++)
        delete (String*)m_pAutoTextGroupLB->GetEntryData( i );
    m_xAutoText = 0;

    ClearUserData();
    delete pExampleFrame;
}

SfxTabPage* SwVisitingCardPage::Create(Window* pParent, const SfxItemSet& rSet)
{
    return new SwVisitingCardPage(pParent, rSet);
}

void SwVisitingCardPage::ActivatePage(const SfxItemSet& rSet)
{
    Reset( rSet );
    UpdateFields();
}

int  SwVisitingCardPage::DeactivatePage(SfxItemSet* _pSet)
{
    if (_pSet)
        FillItemSet(*_pSet);
    return LEAVE_PAGE;
}

sal_Bool SwVisitingCardPage::FillItemSet(SfxItemSet& rSet)
{
    String* pGroup = (String*)m_pAutoTextGroupLB->GetEntryData(
                                    m_pAutoTextGroupLB->GetSelectEntryPos());
    OSL_ENSURE(pGroup, "no group selected?");

    if(pGroup)
        aLabItem.sGlossaryGroup = *pGroup;

    SvTreeListEntry* pSelEntry = m_pAutoTextLB->FirstSelected();
    if(pSelEntry)
        aLabItem.sGlossaryBlockName = *(String*)pSelEntry->GetUserData();
    rSet.Put(aLabItem);
    return sal_True;
}

static void lcl_SelectBlock(SvTreeListBox& rAutoTextLB, const String& rBlockName)
{
    SvTreeListEntry* pEntry = rAutoTextLB.First();
    while(pEntry)
    {
        if(*(String*)pEntry->GetUserData() == rBlockName)
        {
            rAutoTextLB.Select(pEntry);
            rAutoTextLB.MakeVisible(pEntry);
            break;
        }
        pEntry = rAutoTextLB.Next(pEntry);
    }
}

static bool lcl_FindBlock(SvTreeListBox& rAutoTextLB, const String& rBlockName)
{
    SvTreeListEntry* pEntry = rAutoTextLB.First();
    while(pEntry)
    {
        if(*(String*)pEntry->GetUserData() == rBlockName)
        {
            rAutoTextLB.Select(pEntry);
            return true;
        }
        pEntry = rAutoTextLB.Next(pEntry);
    }
    return false;
}

void SwVisitingCardPage::Reset(const SfxItemSet& rSet)
{
    aLabItem = (const SwLabItem&) rSet.Get(FN_LABEL);

    bool bFound = false;
    sal_uInt16 i;
    for(i = 0; i < m_pAutoTextGroupLB->GetEntryCount() && !bFound; i++)
        if( String(aLabItem.sGlossaryGroup) ==
            *(String*)m_pAutoTextGroupLB->GetEntryData( i ))
        {
            bFound = true;
            break;
        }

    if(!bFound)
    {
        // initially search for a group starting with "crd" which is the name of the
        // business card AutoTexts
        for(i = 0; i < m_pAutoTextGroupLB->GetEntryCount() && !bFound; i++)
            if(0 == (*(String*)m_pAutoTextGroupLB->GetEntryData( i )).SearchAscii( "crd") )
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
            AutoTextSelectHdl(m_pAutoTextGroupLB);
        }
        if(lcl_FindBlock(*m_pAutoTextLB, aLabItem.sGlossaryBlockName))
        {
            SvTreeListEntry* pSelEntry = m_pAutoTextLB->FirstSelected();
            if( pSelEntry &&
                *(String*)pSelEntry->GetUserData() != String(aLabItem.sGlossaryBlockName))
            {
                lcl_SelectBlock(*m_pAutoTextLB, aLabItem.sGlossaryBlockName);
                AutoTextSelectHdl(m_pAutoTextLB);
            }
        }
    }
}

SwPrivateDataPage::SwPrivateDataPage(Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "PrivateUserPage",
        "modules/swriter/ui/privateuserpage.ui", rSet)
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

SfxTabPage* SwPrivateDataPage::Create(Window* pParent, const SfxItemSet& rSet)
{
    return new SwPrivateDataPage(pParent, rSet);
}

void SwPrivateDataPage::ActivatePage(const SfxItemSet& rSet)
{
    Reset(rSet);
}

int  SwPrivateDataPage::DeactivatePage(SfxItemSet* _pSet)
{
    if (_pSet)
        FillItemSet(*_pSet);
    return LEAVE_PAGE;
}

sal_Bool SwPrivateDataPage::FillItemSet(SfxItemSet& rSet)
{

    SwLabItem aItem = (const SwLabItem&) GetTabDialog()->GetExampleSet()->Get(FN_LABEL);
    aItem.aPrivFirstName = m_pFirstNameED->GetText();
    aItem.aPrivName      = m_pNameED->GetText(  );
    aItem.aPrivShortCut  = m_pShortCutED->GetText(  );
    aItem.aPrivFirstName2 = m_pFirstName2ED->GetText();
    aItem.aPrivName2     = m_pName2ED->GetText(  );
    aItem.aPrivShortCut2 = m_pShortCut2ED->GetText(  );
    aItem.aPrivStreet    = m_pStreetED->GetText(  );
    aItem.aPrivZip       = m_pZipED->GetText(  );
    aItem.aPrivCity      = m_pCityED->GetText(  );
    aItem.aPrivCountry   = m_pCountryED->GetText(  );
    aItem.aPrivState     = m_pStateED->GetText(  );
    aItem.aPrivTitle     = m_pTitleED->GetText(  );
    aItem.aPrivProfession= m_pProfessionED->GetText(   );
    aItem.aPrivPhone     = m_pPhoneED->GetText(  );
    aItem.aPrivMobile    = m_pMobilePhoneED->GetText(  );
    aItem.aPrivFax       = m_pFaxED->GetText(  );
    aItem.aPrivWWW       = m_pHomePageED->GetText(  );
    aItem.aPrivMail      = m_pMailED->GetText(  );

    rSet.Put(aItem);
    return sal_True;
}

void SwPrivateDataPage::Reset(const SfxItemSet& rSet)
{
    const SwLabItem& aItem = (const SwLabItem&) rSet.Get(FN_LABEL);
    m_pFirstNameED->SetText(aItem.aPrivFirstName);
    m_pNameED->SetText(aItem.aPrivName);
    m_pShortCutED->SetText(aItem.aPrivShortCut);
    m_pFirstName2ED->SetText(aItem.aPrivFirstName2);
    m_pName2ED->SetText(aItem.aPrivName2);
    m_pShortCut2ED->SetText(aItem.aPrivShortCut2);
    m_pStreetED->SetText(aItem.aPrivStreet);
    m_pZipED->SetText(aItem.aPrivZip);
    m_pCityED->SetText(aItem.aPrivCity);
    m_pCountryED->SetText(aItem.aPrivCountry);
    m_pStateED->SetText(aItem.aPrivState);
    m_pTitleED->SetText(aItem.aPrivTitle);
    m_pProfessionED->SetText(aItem.aPrivProfession);
    m_pPhoneED->SetText(aItem.aPrivPhone);
    m_pMobilePhoneED->SetText(aItem.aPrivMobile);
    m_pFaxED->SetText(aItem.aPrivFax);
    m_pHomePageED->SetText(aItem.aPrivWWW);
    m_pMailED->SetText(aItem.aPrivMail);
}

SwBusinessDataPage::SwBusinessDataPage(Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "BusinessDataPage",
        "modules/swriter/ui/businessdatapage.ui", rSet)
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

SfxTabPage* SwBusinessDataPage::Create(Window* pParent, const SfxItemSet& rSet)
{
    return new SwBusinessDataPage(pParent, rSet);
}

void SwBusinessDataPage::ActivatePage(const SfxItemSet& rSet)
{
    Reset(rSet);
}

int  SwBusinessDataPage::DeactivatePage(SfxItemSet* _pSet)
{
    if (_pSet)
        FillItemSet(*_pSet);
    return LEAVE_PAGE;
}

sal_Bool SwBusinessDataPage::FillItemSet(SfxItemSet& rSet)
{
    SwLabItem aItem = (const SwLabItem&) GetTabDialog()->GetExampleSet()->Get(FN_LABEL);

    aItem.aCompCompany   = m_pCompanyED->GetText();
    aItem.aCompCompanyExt= m_pCompanyExtED->GetText();
    aItem.aCompSlogan    = m_pSloganED->GetText();
    aItem.aCompStreet    = m_pStreetED->GetText();
    aItem.aCompZip       = m_pZipED->GetText();
    aItem.aCompCity      = m_pCityED->GetText();
    aItem.aCompCountry   = m_pCountryED->GetText();
    aItem.aCompState     = m_pStateED->GetText();
    aItem.aCompPosition  = m_pPositionED->GetText();
    aItem.aCompPhone     = m_pPhoneED->GetText();
    aItem.aCompMobile    = m_pMobilePhoneED->GetText();
    aItem.aCompFax       = m_pFaxED->GetText();
    aItem.aCompWWW       = m_pHomePageED->GetText();
    aItem.aCompMail      = m_pMailED->GetText();

    rSet.Put(aItem);
    return sal_True;
}

void SwBusinessDataPage::Reset(const SfxItemSet& rSet)
{
    const SwLabItem& aItem = (const SwLabItem&) rSet.Get(FN_LABEL);
    m_pCompanyED->SetText(aItem.aCompCompany);
    m_pCompanyExtED->SetText(aItem.aCompCompanyExt);
    m_pSloganED->SetText(aItem.aCompSlogan);
    m_pStreetED->SetText(aItem.aCompStreet);
    m_pZipED->SetText(aItem.aCompZip);
    m_pCityED->SetText(aItem.aCompCity);
    m_pCountryED->SetText(aItem.aCompCountry);
    m_pStateED->SetText(aItem.aCompState);
    m_pPositionED->SetText(aItem.aCompPosition);
    m_pPhoneED->SetText(aItem.aCompPhone);
    m_pMobilePhoneED->SetText(aItem.aCompMobile);
    m_pFaxED->SetText(aItem.aCompFax);
    m_pHomePageED->SetText(aItem.aCompWWW);
    m_pMailED->SetText(aItem.aCompMail);
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
