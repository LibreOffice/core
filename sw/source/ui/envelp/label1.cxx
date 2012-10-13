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

#include <vcl/waitobj.hxx>
#include <rtl/ustring.hxx>
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
    aLabelsCfg.FillLabels(rtl::OUString(rMake), *pRecs);
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

    sal_Bool bDouble = sal_False;

    for (sal_uInt16 nRecPos = 0; nRecPos < pRecs->size(); nRecPos++)
    {
        if (pRec->aMake == (*pRecs)[nRecPos]->aMake &&
            pRec->aType == (*pRecs)[nRecPos]->aType)
        {
            bDouble = sal_True;
            break;
        }
    }

    if (!bDouble)
        pRecs->insert( pRecs->begin(), pRec );

    sal_uInt16 nLstGroup = 0;
    const ::com::sun::star::uno::Sequence<rtl::OUString>& rMan = aLabelsCfg.GetManufacturers();
    const rtl::OUString* pMan = rMan.getConstArray();
    for(sal_Int32 nMan = 0; nMan < rMan.getLength(); nMan++)
    {
        aMakes.push_back(pMan[nMan]);
        if ( pMan[nMan] == aItem.aLstMake )
            nLstGroup = (sal_uInt16) nMan;
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
    sal_Bool bFound = sal_False;
    String sCustom(SW_RES(STR_CUSTOM));

    const sal_uInt16 nCount = Recs().size();
    for (sal_uInt16 i = 0; i < nCount; i++)
    {
        pRec = Recs()[i];
        if (pRec->aType != sCustom &&
            rRecName == pRec->aType && bCont == pRec->bCont)
        {
            bFound = sal_True;
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

SwLabPage::SwLabPage(Window* pParent, const SfxItemSet& rSet) :
    SfxTabPage(pParent, SW_RES(TP_LAB_LAB), rSet),
    pNewDBMgr(NULL),
    aItem          ((const SwLabItem&) rSet.Get(FN_LABEL)),

    aWritingFL     (this, SW_RES(FL_WRITING)),
    aWritingText   (this, SW_RES(TXT_WRITING)),
    aAddrBox       (this, SW_RES(BOX_ADDR   )),
    aWritingEdit   (this, SW_RES(EDT_WRITING)),
    aDatabaseFT    (this, SW_RES(FT_DATABASE)),
    aDatabaseLB    (this, SW_RES(LB_DATABASE)),
    aTableFT       (this, SW_RES(FT_TABLE   )),
    aTableLB       (this, SW_RES(LB_TABLE   )),
    aInsertBT      (this, SW_RES(BTN_INSERT )),
    aDBFieldFT     (this, SW_RES(FT_DBFIELD )),
    aDBFieldLB     (this, SW_RES(LB_DBFIELD )),
    aFormatFL      (this, SW_RES(FL_FORMAT )),
    aContButton    (this, SW_RES(BTN_CONT   )),
    aSheetButton   (this, SW_RES(BTN_SHEET  )),
    aMakeText      (this, SW_RES(TXT_MAKE   )),
    aMakeBox       (this, SW_RES(BOX_MAKE   )),
    aTypeText      (this, SW_RES(TXT_TYPE   )),
    aTypeBox       (this, SW_RES(BOX_TYPE   )),
    aHiddenSortTypeBox(this, WB_SORT|WB_HIDE),
    aFormatInfo    (this, SW_RES(INF_FORMAT ))
   {
    WaitObject aWait( pParent );

    FreeResource();
    SetExchangeSupport();


    // Install handlers
    aAddrBox       .SetClickHdl (LINK(this, SwLabPage, AddrHdl         ));
    aDatabaseLB    .SetSelectHdl(LINK(this, SwLabPage, DatabaseHdl     ));
    aTableLB       .SetSelectHdl(LINK(this, SwLabPage, DatabaseHdl     ));
    aInsertBT      .SetClickHdl (LINK(this, SwLabPage, FieldHdl        ));
    aContButton    .SetClickHdl (LINK(this, SwLabPage, PageHdl         ));
    aSheetButton   .SetClickHdl (LINK(this, SwLabPage, PageHdl         ));
    aMakeBox       .SetSelectHdl(LINK(this, SwLabPage, MakeHdl         ));
    aTypeBox       .SetSelectHdl(LINK(this, SwLabPage, TypeHdl         ));

    InitDatabaseBox();

    size_t nLstGroup = 0;

    const sal_uInt16 nCount = (sal_uInt16)GetParentSwLabDlg()->Makes().size();
    for(size_t i = 0; i < nCount; ++i)
    {
        rtl::OUString& rStr = GetParentSwLabDlg()->Makes()[i];
        aMakeBox.InsertEntry( rStr );

        if ( rStr == aItem.aLstMake)
            nLstGroup = i;
    }

    aMakeBox.SelectEntryPos( nLstGroup );
    aMakeBox.GetSelectHdl().Call( &aMakeBox );
}

SwLabPage::~SwLabPage()
{
}

static void lcl_ChgYPos(Window& rWin, long nDiff)
{
    Point aTempPos(rWin.GetPosPixel());
    aTempPos.Y() += nDiff;
    rWin.SetPosPixel(aTempPos);
}

void SwLabPage::SetToBusinessCard()
{
    SetHelpId(HID_BUSINESS_FMT_PAGE);
    aContButton.SetHelpId(HID_BUSINESS_FMT_PAGE_CONT);
    aSheetButton.SetHelpId(HID_BUSINESS_FMT_PAGE_SHEET);
    aMakeBox.SetHelpId(HID_BUSINESS_FMT_PAGE_BRAND);
    aTypeBox.SetHelpId(HID_BUSINESS_FMT_PAGE_TYPE);
    m_bLabel = sal_False;
    aWritingText.Hide();
    aAddrBox.Hide();
    aWritingEdit.Hide();
    aDatabaseFT.Hide();
    aDatabaseLB.Hide();
    aTableFT.Hide();
    aTableLB.Hide();
    aInsertBT.Hide();
    aDBFieldFT.Hide();
    aDBFieldLB.Hide();
    aWritingFL.Hide();

    //resize the form
    Point aFLPos(aWritingFL.GetPosPixel());
    long nDiffPos = aFormatFL.GetPosPixel().Y() - aFLPos.Y();
    Size aFLSz(aFormatFL.GetSizePixel());
    aFormatFL.SetPosSizePixel(aFLPos, aFLSz);

    // move all controls up
    lcl_ChgYPos(aContButton, -nDiffPos);
    lcl_ChgYPos(aSheetButton, -nDiffPos);
    lcl_ChgYPos(aMakeText, -nDiffPos);

    lcl_ChgYPos(aTypeText, -nDiffPos);
    lcl_ChgYPos(aFormatInfo, -nDiffPos);

    Size aTmpSz(3, 3);
    aTmpSz = LogicToPixel(aTmpSz, MAP_APPFONT);

    lcl_ChgYPos(aMakeBox, - nDiffPos);
    Point aLBPos(aMakeBox.GetPosPixel());
    aLBPos.Y() += aMakeBox.GetSizePixel().Height() + aTmpSz.Height();
    aTypeBox.SetPosPixel(aLBPos);
};

IMPL_LINK_NOARG(SwLabPage, AddrHdl)
{
    String aWriting;

    if ( aAddrBox.IsChecked() )
        aWriting = convertLineEnd(MakeSender(), GetSystemLineEnd());

    aWritingEdit.SetText( aWriting );
    aWritingEdit.GrabFocus();
    return 0;
}

IMPL_LINK( SwLabPage, DatabaseHdl, ListBox *, pListBox )
{
    sActDBName = aDatabaseLB.GetSelectEntry();

    WaitObject aObj( GetParentSwLabDlg() );

    if (pListBox == &aDatabaseLB)
        GetNewDBMgr()->GetTableNames(&aTableLB, sActDBName);

    GetNewDBMgr()->GetColumnNames(&aDBFieldLB, sActDBName, aTableLB.GetSelectEntry());
    return 0;
}

IMPL_LINK_NOARG(SwLabPage, FieldHdl)
{
    rtl::OUStringBuffer aStr;
    aStr.append('<');
    aStr.append(aDatabaseLB.GetSelectEntry());
    aStr.append('.');
    aStr.append(aTableLB.GetSelectEntry());
    aStr.append('.');
    aStr.append(aTableLB.GetEntryData(aTableLB.GetSelectEntryPos()) == 0 ? '0' : '1');
    aStr.append('.');
    aStr.append(aDBFieldLB.GetSelectEntry());
    aStr.append('>');
    aWritingEdit.ReplaceSelected(aStr.makeStringAndClear());
    Selection aSel = aWritingEdit.GetSelection();
    aWritingEdit.GrabFocus();
    aWritingEdit.SetSelection(aSel);
    return 0;
}

IMPL_LINK_NOARG_INLINE_START(SwLabPage, PageHdl)
{
    aMakeBox.GetSelectHdl().Call( &aMakeBox );
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwLabPage, PageHdl)

IMPL_LINK_NOARG(SwLabPage, MakeHdl)
{
    WaitObject aWait( GetParentSwLabDlg() );

    aTypeBox.Clear();
    aHiddenSortTypeBox.Clear();
    GetParentSwLabDlg()->TypeIds().clear();

    const String aMake = aMakeBox.GetSelectEntry();
    GetParentSwLabDlg()->ReplaceGroup( aMake );
    aItem.aLstMake = aMake;

    const sal_Bool   bCont    = aContButton.IsChecked();
    const sal_uInt16 nCount   = GetParentSwLabDlg()->Recs().size();
          sal_uInt16 nLstType = 0;

    const String sCustom(SW_RES(STR_CUSTOM));
    //insert the entries into the sorted list box
    for ( sal_uInt16 i = 0; i < nCount; ++i )
    {
        const String aType ( GetParentSwLabDlg()->Recs()[i]->aType );
        sal_Bool bInsert = sal_False;
        if ( GetParentSwLabDlg()->Recs()[i]->aType == sCustom )
        {
            bInsert = sal_True;
            aTypeBox.InsertEntry(aType );
        }
        else if ( GetParentSwLabDlg()->Recs()[i]->bCont == bCont )
        {
            if ( aHiddenSortTypeBox.GetEntryPos(aType) == LISTBOX_ENTRY_NOTFOUND )
            {
                bInsert = sal_True;
                aHiddenSortTypeBox.InsertEntry( aType );
            }
        }
        if(bInsert)
        {
            GetParentSwLabDlg()->TypeIds().push_back(i);
            if ( !nLstType && aType == String(aItem.aLstType) )
                nLstType = GetParentSwLabDlg()->TypeIds().size();
        }
    }
    for(sal_uInt16 nEntry = 0; nEntry < aHiddenSortTypeBox.GetEntryCount(); nEntry++)
    {
        aTypeBox.InsertEntry(aHiddenSortTypeBox.GetEntry(nEntry));
    }
    if (nLstType)
        aTypeBox.SelectEntry(aItem.aLstType);
    else
        aTypeBox.SelectEntryPos(0);
    aTypeBox.GetSelectHdl().Call( &aTypeBox );
    return 0;
}

IMPL_LINK_NOARG_INLINE_START(SwLabPage, TypeHdl)
{
    DisplayFormat();
    aItem.aType = aTypeBox.GetSelectEntry();
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
    aText += String::CreateFromInt32( pRec->nCols );
    aText.AppendAscii( RTL_CONSTASCII_STRINGPARAM(" x "));
    aText += String::CreateFromInt32( pRec->nRows );
    aText += ')';
    aFormatInfo.SetText(aText);
}

SwLabRec* SwLabPage::GetSelectedEntryPos()
{
    String sSelEntry(aTypeBox.GetSelectEntry());

    return GetParentSwLabDlg()->GetRecord(sSelEntry, aContButton.IsChecked());
}

void SwLabPage::InitDatabaseBox()
{
    if( GetNewDBMgr() )
    {
        aDatabaseLB.Clear();
        ::com::sun::star::uno::Sequence<rtl::OUString> aDataNames = SwNewDBMgr::GetExistingDatabaseNames();
        const rtl::OUString* pDataNames = aDataNames.getConstArray();
        for (long i = 0; i < aDataNames.getLength(); i++)
            aDatabaseLB.InsertEntry(pDataNames[i]);
        String sDBName = sActDBName.GetToken( 0, DB_DELIM );
        String sTableName = sActDBName.GetToken( 1, DB_DELIM );
        aDatabaseLB.SelectEntry(sDBName);
        if( sDBName.Len() && GetNewDBMgr()->GetTableNames(&aTableLB, sDBName))
        {
            aTableLB.SelectEntry(sTableName);
            GetNewDBMgr()->GetColumnNames(&aDBFieldLB, sActDBName, sTableName);
        }
        else
            aDBFieldLB.Clear();
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
    rItem.bAddr    = aAddrBox.IsChecked();
    rItem.aWriting = aWritingEdit.GetText();
    rItem.bCont    = aContButton.IsChecked();
    rItem.aMake    = aMakeBox.GetSelectEntry();
    rItem.aType    = aTypeBox.GetSelectEntry();
    rItem.sDBName  = sActDBName;

    SwLabRec* pRec = GetSelectedEntryPos();
    pRec->FillItem( rItem );

    rItem.aLstMake = aMakeBox.GetSelectEntry();
    rItem.aLstType = aTypeBox.GetSelectEntry();
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

    aAddrBox    .Check      ( aItem.bAddr );
    aWritingEdit.SetText    ( aWriting );

    for(std::vector<rtl::OUString>::const_iterator i = GetParentSwLabDlg()->Makes().begin(); i != GetParentSwLabDlg()->Makes().end(); ++i)
    {
        if(aMakeBox.GetEntryPos(String(*i)) == LISTBOX_ENTRY_NOTFOUND)
            aMakeBox.InsertEntry(*i);
    }

    aMakeBox    .SelectEntry( aItem.aMake );
    //save the current type
    String sType(aItem.aType);
    aMakeBox.GetSelectHdl().Call( &aMakeBox );
    aItem.aType = sType;
    //#102806# a newly added make may not be in the type ListBox already
    if (aTypeBox.GetEntryPos(String(aItem.aType)) == LISTBOX_ENTRY_NOTFOUND && !aItem.aMake.isEmpty())
        GetParentSwLabDlg()->UpdateGroup( aItem.aMake );
    if (aTypeBox.GetEntryPos(String(aItem.aType)) != LISTBOX_ENTRY_NOTFOUND)
    {
        aTypeBox.SelectEntry(aItem.aType);
        aTypeBox.GetSelectHdl().Call(&aTypeBox);
    }
    if (aDatabaseLB.GetEntryPos(sDBName) != LISTBOX_ENTRY_NOTFOUND)
    {
        aDatabaseLB.SelectEntry(sDBName);
        aDatabaseLB.GetSelectHdl().Call(&aDatabaseLB);
    }

    if (aItem.bCont)
        aContButton .Check();
    else
        aSheetButton.Check();
}

void SwVisitingCardPage::ClearUserData()
{
    SvLBoxEntry* pEntry = aAutoTextLB.First();
    while(pEntry)
    {
        delete (String*)pEntry->GetUserData();
        pEntry = aAutoTextLB.Next(pEntry);
    }
}

void SwVisitingCardPage::SetUserData( sal_uInt32 nCnt,
                const rtl::OUString* pNames, const rtl::OUString* pValues )
{
    for( sal_uInt32 i = 0; i < nCnt; ++i )
    {
        SvLBoxEntry* pEntry = aAutoTextLB.InsertEntry( pNames[ i ] );
        pEntry->SetUserData( new String( pValues[ i ] ));
    }
}

SwVisitingCardPage::SwVisitingCardPage(Window* pParent, const SfxItemSet& rSet) :
    SfxTabPage(pParent, SW_RES(TP_VISITING_CARDS), rSet),
    aContentFL(this,        SW_RES( FL_CONTENT           )),
    aAutoTextLB(this,       SW_RES( LB_AUTO_TEXT            )),
    aAutoTextGroupFT(this,  SW_RES( FT_AUTO_TEXT_GROUP  )),
    aAutoTextGroupLB(this,  SW_RES( LB_AUTO_TEXT_GROUP  )),
    aExampleWIN(this,       SW_RES( WIN_EXAMPLE         )),
    sVisCardGroup(SW_RES(ST_VISCARD_GROUP)),
    pExampleFrame(0)
{
    FreeResource();
    aAutoTextLB.SetStyle( aAutoTextLB.GetStyle() | WB_HSCROLL );
    aAutoTextLB.SetSpaceBetweenEntries(0);
    aAutoTextLB.SetSelectionMode( SINGLE_SELECTION );
    aAutoTextLB.SetHelpId(HID_BUSINESS_CARD_CONTENT);

    SetExchangeSupport();
    aAutoTextLB.SetSelectHdl(LINK(this, SwVisitingCardPage, AutoTextSelectHdl));
    aAutoTextGroupLB.SetSelectHdl(LINK(this, SwVisitingCardPage, AutoTextSelectHdl));

    aExampleWIN.Hide();

    aAutoTextLB.Show();
    aAutoTextGroupFT.Show();
    aAutoTextGroupLB.Show();
    InitFrameControl();
}

SwVisitingCardPage::~SwVisitingCardPage()
{
    for(sal_uInt16 i = 0; i < aAutoTextGroupLB.GetEntryCount(); i++)
        delete (String*)aAutoTextGroupLB.GetEntryData( i );
    _xAutoText = 0;

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
    String* pGroup = (String*)aAutoTextGroupLB.GetEntryData(
                                    aAutoTextGroupLB.GetSelectEntryPos());
    OSL_ENSURE(pGroup, "no group selected?");

    if(pGroup)
        aLabItem.sGlossaryGroup = *pGroup;

    SvLBoxEntry* pSelEntry = aAutoTextLB.FirstSelected();
    if(pSelEntry)
        aLabItem.sGlossaryBlockName = *(String*)pSelEntry->GetUserData();
    rSet.Put(aLabItem);
    return sal_True;
}

static void lcl_SelectBlock(SvTreeListBox& rAutoTextLB, const String& rBlockName)
{
    SvLBoxEntry* pEntry = rAutoTextLB.First();
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

static sal_Bool lcl_FindBlock(SvTreeListBox& rAutoTextLB, const String& rBlockName)
{
    SvLBoxEntry* pEntry = rAutoTextLB.First();
    while(pEntry)
    {
        if(*(String*)pEntry->GetUserData() == rBlockName)
        {
            rAutoTextLB.Select(pEntry);
            return sal_True;
        }
        pEntry = rAutoTextLB.Next(pEntry);
    }
    return sal_False;
}

void SwVisitingCardPage::Reset(const SfxItemSet& rSet)
{
    aLabItem = (const SwLabItem&) rSet.Get(FN_LABEL);

    sal_Bool bFound = sal_False;
    sal_uInt16 i;
    for(i = 0; i < aAutoTextGroupLB.GetEntryCount() && !bFound; i++)
        if( String(aLabItem.sGlossaryGroup) ==
            *(String*)aAutoTextGroupLB.GetEntryData( i ))
        {
            bFound = sal_True;
            break;
        }

    if(!bFound)
    {
        // initially search for a group starting with "crd" which is the name of the
        // business card AutoTexts
        for(i = 0; i < aAutoTextGroupLB.GetEntryCount() && !bFound; i++)
            if(0 == (*(String*)aAutoTextGroupLB.GetEntryData( i )).SearchAscii( "crd") )
            {
                bFound = sal_True;
                break;
            }
    }
    if(bFound)
    {
        if(aAutoTextGroupLB.GetSelectEntryPos() != i)
        {
            aAutoTextGroupLB.SelectEntryPos(i);
            AutoTextSelectHdl(&aAutoTextGroupLB);
        }
        if(lcl_FindBlock(aAutoTextLB, aLabItem.sGlossaryBlockName))
        {
            SvLBoxEntry* pSelEntry = aAutoTextLB.FirstSelected();
            if( pSelEntry &&
                *(String*)pSelEntry->GetUserData() != String(aLabItem.sGlossaryBlockName))
            {
                lcl_SelectBlock(aAutoTextLB, aLabItem.sGlossaryBlockName);
                AutoTextSelectHdl(&aAutoTextLB);
            }
        }
    }
}

SwPrivateDataPage::SwPrivateDataPage(Window* pParent, const SfxItemSet& rSet) :
    SfxTabPage(pParent, SW_RES(TP_PRIVATE_DATA), rSet),
    aDataFL             (this, SW_RES( FL_DATA       )),

    aNameFT             (this, SW_RES( FT_NAME       )),
    aFirstNameED        (this, SW_RES( ED_FIRSTNAME )),
    aNameED             (this, SW_RES( ED_NAME      )),
    aShortCutED         (this, SW_RES( ED_SHORTCUT  )),

    aName2FT            (this, SW_RES( FT_NAME_2     )),
    aFirstName2ED       (this, SW_RES( ED_FIRSTNAME_2)),
    aName2ED            (this, SW_RES( ED_NAME_2        )),
    aShortCut2ED        (this, SW_RES( ED_SHORTCUT_2    )),

    aStreetFT           (this, SW_RES( FT_STREET     )),
    aStreetED           (this, SW_RES( ED_STREET        )),
    aZipCityFT          (this, SW_RES( FT_ZIPCITY   )),
    aZipED              (this, SW_RES( ED_ZIP       )),
    aCityED             (this, SW_RES( ED_CITY      )),
    aCountryStateFT     (this, SW_RES( FT_COUNTRYSTATE )),
    aCountryED          (this, SW_RES( ED_COUNTRY   )),
    aStateED            (this, SW_RES( ED_STATE     )),
    aTitleProfessionFT  (this, SW_RES( FT_TITLEPROF )),
    aTitleED            (this, SW_RES( ED_TITLE     )),
    aProfessionED       (this, SW_RES( ED_PROFESSION )),
    aPhoneFT            (this, SW_RES( FT_PHONE_MOBILE  )),
    aPhoneED            (this, SW_RES( ED_PHONE     )),
    aMobilePhoneED      (this, SW_RES( ED_MOBILE        )),
    aFaxFT              (this, SW_RES( FT_FAX       )),
    aFaxED              (this, SW_RES( ED_FAX       )),
    aWWWMailFT          (this, SW_RES( FT_WWWMAIL   )),
    aHomePageED         (this, SW_RES( ED_WWW       )),
    aMailED             (this, SW_RES( ED_MAIL      ))
{
    FreeResource();
    SetExchangeSupport();
}

SwPrivateDataPage::~SwPrivateDataPage()
{
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
    aItem.aPrivFirstName = aFirstNameED .GetText();
    aItem.aPrivName      = aNameED      .GetText(  );
    aItem.aPrivShortCut  = aShortCutED  .GetText(  );
    aItem.aPrivFirstName2 = aFirstName2ED   .GetText();
    aItem.aPrivName2     = aName2ED     .GetText(  );
    aItem.aPrivShortCut2 = aShortCut2ED .GetText(  );
    aItem.aPrivStreet    = aStreetED    .GetText(  );
    aItem.aPrivZip       = aZipED       .GetText(  );
    aItem.aPrivCity      = aCityED      .GetText(  );
    aItem.aPrivCountry   = aCountryED   .GetText(  );
    aItem.aPrivState     = aStateED     .GetText(  );
    aItem.aPrivTitle     = aTitleED     .GetText(  );
    aItem.aPrivProfession= aProfessionED.GetText(   );
    aItem.aPrivPhone     = aPhoneED     .GetText(  );
    aItem.aPrivMobile    = aMobilePhoneED.GetText(  );
    aItem.aPrivFax       = aFaxED       .GetText(  );
    aItem.aPrivWWW       = aHomePageED  .GetText(  );
    aItem.aPrivMail      = aMailED      .GetText(  );

    rSet.Put(aItem);
    return sal_True;
}

void SwPrivateDataPage::Reset(const SfxItemSet& rSet)
{
    const SwLabItem& aItem = (const SwLabItem&) rSet.Get(FN_LABEL);
    aFirstNameED.SetText(aItem.aPrivFirstName);
    aNameED     .SetText(aItem.aPrivName);
    aShortCutED .SetText(aItem.aPrivShortCut);
    aFirstName2ED.SetText(aItem.aPrivFirstName2);
    aName2ED     .SetText(aItem.aPrivName2);
    aShortCut2ED .SetText(aItem.aPrivShortCut2);
    aStreetED   .SetText(aItem.aPrivStreet);
    aZipED      .SetText(aItem.aPrivZip);
    aCityED     .SetText(aItem.aPrivCity);
    aCountryED  .SetText(aItem.aPrivCountry);
    aStateED    .SetText(aItem.aPrivState);
    aTitleED    .SetText(aItem.aPrivTitle);
    aProfessionED.SetText(aItem.aPrivProfession);
    aPhoneED    .SetText(aItem.aPrivPhone);
    aMobilePhoneED.SetText(aItem.aPrivMobile);
    aFaxED      .SetText(aItem.aPrivFax);
    aHomePageED .SetText(aItem.aPrivWWW);
    aMailED     .SetText(aItem.aPrivMail);
}

SwBusinessDataPage::SwBusinessDataPage(Window* pParent, const SfxItemSet& rSet) :
    SfxTabPage(pParent, SW_RES(TP_BUSINESS_DATA), rSet),
    aDataFL             (this, SW_RES( FL_DATA       )),
    aCompanyFT          (this, SW_RES( FT_COMP      )),
    aCompanyED          (this, SW_RES( ED_COMP      )),
    aCompanyExtFT       (this, SW_RES( FT_COMP_EXT  )),
    aCompanyExtED       (this, SW_RES( ED_COMP_EXT  )),
    aSloganFT           (this, SW_RES( FT_SLOGAN        )),
    aSloganED           (this, SW_RES( ED_SLOGAN        )),
    aStreetFT           (this, SW_RES( FT_STREET        )),
    aStreetED           (this, SW_RES( ED_STREET        )),
    aZipCityFT          (this, SW_RES( FT_ZIPCITY   )),
    aZipED              (this, SW_RES( ED_ZIP       )),
    aCityED             (this, SW_RES( ED_CITY      )),
    aCountryStateFT     (this, SW_RES( FT_COUNTRYSTATE  )),
    aCountryED          (this, SW_RES( ED_COUNTRY   )),
    aStateED            (this, SW_RES( ED_STATE     )),
    aPositionFT         (this, SW_RES( FT_POSITION  )),
    aPositionED         (this, SW_RES( ED_POSITION  )),
    aPhoneFT            (this, SW_RES( FT_PHONE_MOBILE  )),
    aPhoneED            (this, SW_RES( ED_PHONE     )),
    aMobilePhoneED      (this, SW_RES( ED_MOBILE        )),
    aFaxFT              (this, SW_RES( FT_FAX       )),
    aFaxED              (this, SW_RES( ED_FAX       )),
    aWWWMailFT          (this, SW_RES( FT_WWWMAIL   )),
    aHomePageED         (this, SW_RES( ED_WWW       )),
    aMailED             (this, SW_RES( ED_MAIL      ))
{
    FreeResource();
    SetExchangeSupport();
}


SwBusinessDataPage::~SwBusinessDataPage()
{
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

    aItem.aCompCompany   = aCompanyED      .GetText();
    aItem.aCompCompanyExt= aCompanyExtED   .GetText();
    aItem.aCompSlogan    = aSloganED       .GetText();
    aItem.aCompStreet    = aStreetED       .GetText();
    aItem.aCompZip       = aZipED          .GetText();
    aItem.aCompCity      = aCityED         .GetText();
    aItem.aCompCountry   = aCountryED      .GetText();
    aItem.aCompState     = aStateED        .GetText();
    aItem.aCompPosition  = aPositionED     .GetText();
    aItem.aCompPhone     = aPhoneED        .GetText();
    aItem.aCompMobile    = aMobilePhoneED  .GetText();
    aItem.aCompFax       = aFaxED          .GetText();
    aItem.aCompWWW       = aHomePageED     .GetText();
    aItem.aCompMail      = aMailED         .GetText();

    rSet.Put(aItem);
    return sal_True;
}

void SwBusinessDataPage::Reset(const SfxItemSet& rSet)
{
    const SwLabItem& aItem = (const SwLabItem&) rSet.Get(FN_LABEL);
    aCompanyED      .SetText(aItem.aCompCompany);
    aCompanyExtED   .SetText(aItem.aCompCompanyExt);
    aSloganED       .SetText(aItem.aCompSlogan);
    aStreetED       .SetText(aItem.aCompStreet);
    aZipED          .SetText(aItem.aCompZip);
    aCityED         .SetText(aItem.aCompCity);
    aCountryED      .SetText(aItem.aCompCountry);
    aStateED        .SetText(aItem.aCompState);
    aPositionED     .SetText(aItem.aCompPosition);
    aPhoneED        .SetText(aItem.aCompPhone);
    aMobilePhoneED  .SetText(aItem.aCompMobile);
    aFaxED          .SetText(aItem.aCompFax);
    aHomePageED     .SetText(aItem.aCompWWW);
    aMailED         .SetText(aItem.aCompMail);
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
