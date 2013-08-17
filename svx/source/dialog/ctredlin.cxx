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

#include <vcl/dialog.hxx>
#include <vcl/layout.hxx>
#include <vcl/svapp.hxx>
#include <tools/shl.hxx>
#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <unotools/textsearch.hxx>
#include <svtools/svlbitm.hxx>
#include "svtools/viewdataentry.hxx"
#include <unotools/charclass.hxx>

#include <editeng/unolingu.hxx>
#include <svx/dialmgr.hxx>
#include "svx/dialogs.hrc"
#include "ctredlin.hrc"
#include <svx/ctredlin.hxx>
#include "helpid.hrc"

//============================================================================
//  Local Defines and Function
//----------------------------------------------------------------------------

static long nStaticTabs[]=
{
    5,10,65,120,170,220
};

#define WRITER_DATE     2
#define CALC_DATE       3

RedlinData::RedlinData()
    :
        aDateTime( DateTime::EMPTY )
{
    bDisabled=sal_False;
    pData=NULL;
}
RedlinData::~RedlinData()
{
}

//============================================================================
//  class SvxRedlinEntry (Entries for list)
//----------------------------------------------------------------------------

SvxRedlinEntry::SvxRedlinEntry()
    : SvTreeListEntry()
{
}

SvxRedlinEntry::~SvxRedlinEntry()
{
    RedlinData* pRedDat=(RedlinData*) GetUserData();
    delete pRedDat;
}

// Functions in the inserts Items of SvxRedlinTable

DBG_NAME(SvLBoxColorString);

SvLBoxColorString::SvLBoxColorString(SvTreeListEntry*pEntry, sal_uInt16 nFlags, const OUString& rStr,
    const Color& rCol)
    : SvLBoxString(pEntry, nFlags, rStr)
{
    DBG_CTOR(SvLBoxColorString,0);
    aPrivColor=rCol;
    SetText( rStr );
}

SvLBoxColorString::SvLBoxColorString()
: SvLBoxString()
{
    DBG_CTOR(SvLBoxColorString,0);
}

SvLBoxColorString::~SvLBoxColorString()
{
    DBG_DTOR(SvLBoxColorString,0);
}

SvLBoxItem* SvLBoxColorString::Create() const
{
    DBG_CHKTHIS(SvLBoxColorString,0);
    return new SvLBoxColorString;
}

void SvLBoxColorString::Paint(
    const Point& rPos, SvTreeListBox& rDev,
    const SvViewDataEntry* pView, const SvTreeListEntry* pEntry)
/* [Description]

   Paint function of the SvLBoxColorString class. The relevant text with the
   selected color is drawn in the output device.
*/

{
    Color aColor=rDev.GetTextColor();
    Color a2Color=aColor;
    if (!pView->IsSelected())
    {
        rDev.SetTextColor(aPrivColor);
    }
    SvLBoxString::Paint(rPos, rDev, pView, pEntry);
    rDev.SetTextColor(a2Color);
}

//============================================================================
//  class SvxRedlinTable
//----------------------------------------------------------------------------

SvxRedlinTable::SvxRedlinTable(SvSimpleTableContainer& rParent,WinBits nBits)
    : SvSimpleTable(rParent,nBits),
    aDaTiFirst( DateTime::EMPTY ),
    aDaTiLast( DateTime::EMPTY ),
    aDaTiFilterFirst( DateTime::EMPTY ),
    aDaTiFilterLast( DateTime::EMPTY )
{
    bAuthor=sal_False;
    bDate=sal_False;
    bIsCalc=sal_False;
    bComment=sal_False;
    nDatePos=WRITER_DATE;
    pCommentSearcher=NULL;
    SetNodeDefaultImages();
}

SvxRedlinTable::~SvxRedlinTable()
{
    delete pCommentSearcher;
}

StringCompare SvxRedlinTable::ColCompare(SvTreeListEntry* pLeft,SvTreeListEntry* pRight)
{
    StringCompare eCompare=COMPARE_EQUAL;

    if(aColCompareLink.IsSet())
    {
        SvSortData aRedlinCompare;
        aRedlinCompare.pLeft=pLeft;
        aRedlinCompare.pRight=pRight;
        eCompare=(StringCompare) aColCompareLink.Call(&aRedlinCompare);
    }
    else
    {
        if(nDatePos==GetSortedCol())
        {
            RedlinData *pLeftData=(RedlinData *)(pLeft->GetUserData());
            RedlinData *pRightData=(RedlinData *)(pRight->GetUserData());

            if(pLeftData!=NULL && pRightData!=NULL)
            {
                if(pLeftData->aDateTime < pRightData->aDateTime)
                {
                    eCompare=COMPARE_LESS;
                }
                else if(pLeftData->aDateTime > pRightData->aDateTime)
                {
                    eCompare=COMPARE_GREATER;
                }
            }
            else
                eCompare=SvSimpleTable::ColCompare(pLeft,pRight);
        }
        else
        {
            eCompare=SvSimpleTable::ColCompare(pLeft,pRight);
        }

    }

    return eCompare;
}
void SvxRedlinTable::SetCalcView(sal_Bool bFlag)
{
    bIsCalc=bFlag;
    if(bFlag)
    {
        nDatePos=CALC_DATE;
    }
    else
    {
        nDatePos=WRITER_DATE;
    }
}


void SvxRedlinTable::UpdateFilterTest()
{
    Date aDateMax( Date::SYSTEM );
    sal_uInt16 nYEAR=aDateMax.GetYear()+100;
    aDateMax.SetYear(nYEAR);
    Date aDateMin(1,1,1989);
    Time aTMin(0);
    Time aTMax(23,59,59);


    DateTime aDTMin(aDateMin);
    DateTime aDTMax(aDateMax);

    switch(nDaTiMode)
    {
        case FLT_DATE_BEFORE:
                                aDaTiFilterFirst=aDTMin;
                                aDaTiFilterLast=aDaTiFirst;
                                break;
        case FLT_DATE_SAVE:
        case FLT_DATE_SINCE:
                                aDaTiFilterFirst=aDaTiFirst;
                                aDaTiFilterLast=aDTMax;
                                break;
        case FLT_DATE_EQUAL:
                                aDaTiFilterFirst=aDaTiFirst;
                                aDaTiFilterLast=aDaTiFirst;
                                aDaTiFilterFirst.SetTime(aTMin.GetTime());
                                aDaTiFilterLast.SetTime(aTMax.GetTime());
                                break;
        case FLT_DATE_NOTEQUAL:
                                aDaTiFilterFirst=aDaTiFirst;
                                aDaTiFilterLast=aDaTiFirst;
                                aDaTiFilterFirst.SetTime(aTMin.GetTime());
                                aDaTiFilterLast.SetTime(aTMax.GetTime());
                                break;
        case FLT_DATE_BETWEEN:
                                aDaTiFilterFirst=aDaTiFirst;
                                aDaTiFilterLast=aDaTiLast;
                                break;
    }
}


void SvxRedlinTable::SetFilterDate(sal_Bool bFlag)
{
    bDate=bFlag;
}

void SvxRedlinTable::SetDateTimeMode(sal_uInt16 nMode)
{
    nDaTiMode=nMode;
}

void SvxRedlinTable::SetFirstDate(const Date& aDate)
{
    aDaTiFirst.SetDate(aDate.GetDate());
}

void SvxRedlinTable::SetLastDate(const Date& aDate)
{
    aDaTiLast.SetDate(aDate.GetDate());
}

void SvxRedlinTable::SetFirstTime(const Time& aTime)
{
    aDaTiFirst.SetTime(aTime.GetTime());
}

void SvxRedlinTable::SetLastTime(const Time& aTime)
{
    aDaTiLast.SetTime(aTime.GetTime());
}

void SvxRedlinTable::SetFilterAuthor(sal_Bool bFlag)
{
    bAuthor=bFlag;
}

void SvxRedlinTable::SetAuthor(const OUString &aString)
{
    aAuthor=aString;
}

void SvxRedlinTable::SetFilterComment(sal_Bool bFlag)
{
    bComment=bFlag;
}

void SvxRedlinTable::SetCommentParams( const utl::SearchParam* pSearchPara )
{
    if(pSearchPara!=NULL)
    {
        delete pCommentSearcher;

        pCommentSearcher=new utl::TextSearch(*pSearchPara, LANGUAGE_SYSTEM );
    }
}

bool SvxRedlinTable::IsValidEntry(const OUString &rAuthorStr,
                                  const DateTime &rDateTime,
                                  const OUString &rCommentStr)
{
    return IsValidEntry(rAuthorStr, rDateTime) && IsValidComment(rCommentStr);
}

bool SvxRedlinTable::IsValidEntry(const OUString &rAuthorStr, const DateTime &rDateTime)
{
    if (bAuthor && aAuthor!=rAuthorStr)
        return false;

    if (!bDate)
        return true;

    const bool bRes = rDateTime.IsBetween(aDaTiFilterFirst, aDaTiFilterLast);
    return nDaTiMode!=FLT_DATE_NOTEQUAL ? bRes : !bRes;
}

bool SvxRedlinTable::IsValidComment(const OUString &rCommentStr)
{
    if (!bComment)
        return true;

    sal_Int32 nStartPos = 0;
    sal_Int32 nEndPos = rCommentStr.getLength();
    return pCommentSearcher->SearchForward( rCommentStr, &nStartPos, &nEndPos);
}

SvTreeListEntry* SvxRedlinTable::InsertEntry(const OUString& rStr,RedlinData *pUserData,
                                SvTreeListEntry* pParent,sal_uIntPtr nPos)
{
    const Color aColor = (pUserData && pUserData->bDisabled) ? Color(COL_GRAY) : GetTextColor();
    return InsertEntry(rStr, pUserData, aColor, pParent, nPos);
}

SvTreeListEntry* SvxRedlinTable::InsertEntry(const OUString& rStr,RedlinData *pUserData,const Color& aColor,
                                SvTreeListEntry* pParent,sal_uIntPtr nPos)
{
    aEntryColor=aColor;

    sal_Int32 nIndex = 0;
    const OUString aFirstStr( rStr.getToken(0, '\t', nIndex ) );
    aCurEntry = nIndex>0 ? rStr.copy(nIndex) : OUString();

    return SvSimpleTable::InsertEntry( aFirstStr, pParent, sal_False, nPos, pUserData );
}

SvTreeListEntry* SvxRedlinTable::CreateEntry() const
{
    return new SvxRedlinEntry;
}

void SvxRedlinTable::InitEntry(SvTreeListEntry* pEntry, const OUString& rStr,
    const Image& rColl, const Image& rExp, SvLBoxButtonKind eButtonKind)
{
    SvLBoxButton* pButton;
    SvLBoxString* pString;
    SvLBoxContextBmp* pContextBmp;

    if( nTreeFlags & TREEFLAG_CHKBTN )
    {
        pButton= new SvLBoxButton( pEntry,eButtonKind,0,pCheckButtonData );
        pEntry->AddItem( pButton );
    }

    pContextBmp= new SvLBoxContextBmp(pEntry,0, rColl,rExp, true);
    pEntry->AddItem( pContextBmp );

    pString = new SvLBoxColorString( pEntry, 0, rStr ,aEntryColor);
    pEntry->AddItem( pString );

    sal_Int32 nIndex = 0;
    // TODO: verify if TabCount() is always >0 here!
    const sal_uInt16 nCount = TabCount()-1;
    for( sal_uInt16 nToken = 0; nToken < nCount; nToken++ )
    {
        const OUString aToken = GetToken(aCurEntry, nIndex);
        SvLBoxColorString* pStr = new SvLBoxColorString( pEntry, 0, aToken, aEntryColor);
        pEntry->AddItem( pStr );
    }
}




//============================================================================
//  class SvxTPView
//----------------------------------------------------------------------------

SvxTPView::SvxTPView(Window * pParent)
    : TabPage(pParent, "RedlineViewPage", "svx/ui/redlineviewpage.ui")
{
    Dialog *pTopLevel = GetParentDialog();
    pTopLevel->get(m_pAccept, "accept");
    pTopLevel->get(m_pReject, "reject");
    pTopLevel->get(m_pAcceptAll, "acceptall");
    pTopLevel->get(m_pRejectAll, "rejectall");
    pTopLevel->get(m_pUndo, "undo");

    SvSimpleTableContainer* pTable = get<SvSimpleTableContainer>("changes");
    Size aControlSize(221, 65);
    aControlSize = LogicToPixel(aControlSize, MAP_APPFONT);
    pTable->set_width_request(aControlSize.Width());
    pTable->set_height_request(aControlSize.Height());
    m_pViewData = new SvxRedlinTable(*pTable, 0);

    Link aLink=LINK( this, SvxTPView, PbClickHdl);

    m_pAccept->SetClickHdl(aLink);
    m_pAcceptAll->SetClickHdl(aLink);
    m_pReject->SetClickHdl(aLink);
    m_pRejectAll->SetClickHdl(aLink);
    m_pUndo->SetClickHdl(aLink);

    m_pViewData->SetTabs(nStaticTabs);
}

void SvxTPView::ActivatePage()
{
    m_pAccept->Enable();
    m_pReject->Enable();
    m_pAcceptAll->Enable();
    m_pRejectAll->Enable();
    m_pUndo->Enable();
    TabPage::ActivatePage();
}

void SvxTPView::DeactivatePage()
{
    TabPage::DeactivatePage();
    m_pAccept->Disable();
    m_pReject->Disable();
    m_pAcceptAll->Disable();
    m_pRejectAll->Disable();
    m_pUndo->Disable();
}

SvxTPView::~SvxTPView()
{
    delete m_pViewData;
}

void SvxTPView::InsertWriterHeader()
{
    OUString aStrTab('\t');
    OUString aString(get<FixedText>("action")->GetText());
    aString += aStrTab;
    aString += get<FixedText>("author")->GetText();
    aString += aStrTab;
    aString += get<FixedText>("date")->GetText();
    aString += aStrTab;
    aString += get<FixedText>("comment")->GetText();
    m_pViewData->ClearHeader();
    m_pViewData->InsertHeaderEntry(aString);
}

void SvxTPView::InsertCalcHeader()
{
    OUString aStrTab('\t');
    OUString aString(get<FixedText>("action")->GetText());
    aString += aStrTab;
    aString += get<FixedText>("position")->GetText();
    aString += aStrTab;
    aString += get<FixedText>("author")->GetText();
    aString += aStrTab;
    aString += get<FixedText>("date")->GetText();
    aString += aStrTab;
    aString += get<FixedText>("comment")->GetText();
    m_pViewData->ClearHeader();
    m_pViewData->InsertHeaderEntry(aString);
}

void SvxTPView::EnableAccept(sal_Bool nFlag)
{
    m_pAccept->Enable(nFlag);
}

void SvxTPView::EnableAcceptAll(sal_Bool nFlag)
{
    m_pAcceptAll->Enable(nFlag);
}

void SvxTPView::EnableReject(sal_Bool nFlag)
{
    m_pReject->Enable(nFlag);
}

void SvxTPView::EnableRejectAll(sal_Bool nFlag)
{
    m_pRejectAll->Enable(nFlag);
}

void SvxTPView::ShowUndo(sal_Bool nFlag)
{
    m_pUndo->Show(nFlag);
}

void SvxTPView::EnableUndo(sal_Bool nFlag)
{
    m_pUndo->Enable(nFlag);
}

SvxRedlinTable* SvxTPView::GetTableControl()
{
    return m_pViewData;
}

IMPL_LINK( SvxTPView, PbClickHdl, PushButton*, pPushB )
{
    if (pPushB == m_pAccept)
    {
        AcceptClickLk.Call(this);
    }
    else if (pPushB == m_pAcceptAll)
    {
        AcceptAllClickLk.Call(this);
    }
    else if (pPushB == m_pReject)
    {
        RejectClickLk.Call(this);
    }
    else if (pPushB == m_pRejectAll)
    {
        RejectAllClickLk.Call(this);
    }
    else if (pPushB == m_pUndo)
    {
        UndoClickLk.Call(this);
    }

    return 0;
}




//============================================================================
//  class SvxTPFilter
//----------------------------------------------------------------------------

SvxTPFilter::SvxTPFilter( Window * pParent)
    : TabPage(pParent, "RedlineFilterPage", "svx/ui/redlinefilterpage.ui")
    , pRedlinTable(NULL)
    , bModified(false)
{
    get(m_pCbDate, "date");
    get(m_pLbDate, "datecond");
    get(m_pDfDate, "startdate");
    get(m_pTfDate, "starttime");
    get(m_pIbClock, "startclock");
    get(m_pFtDate2, "and");
    get(m_pDfDate2, "enddate");
    get(m_pTfDate2, "endtime");
    get(m_pIbClock2, "endclock");
    get(m_pCbAuthor, "author");
    get(m_pLbAuthor, "authorlist");
    get(m_pCbRange, "range");
    get(m_pEdRange, "rangeedit");
    get(m_pBtnRange, "dotdotdot");
    get(m_pCbAction, "action");
    get(m_pLbAction, "actionlist");
    get(m_pCbComment, "comment");
    get(m_pEdComment, "commentedit");

    m_pLbAuthor->SetAccessibleName(m_pCbAuthor->GetText());
    m_pEdComment->SetAccessibleName(m_pCbComment->GetText());

    m_pDfDate->SetShowDateCentury( sal_True );
    m_pDfDate2->SetShowDateCentury( sal_True );

    m_pLbDate->SelectEntryPos(0);
    m_pLbDate->SetSelectHdl( LINK( this, SvxTPFilter, SelDateHdl ) );
    m_pIbClock->SetClickHdl( LINK( this, SvxTPFilter, TimeHdl) );
    m_pIbClock2->SetClickHdl( LINK( this, SvxTPFilter,TimeHdl) );
    m_pBtnRange->SetClickHdl( LINK( this, SvxTPFilter, RefHandle));

    Link aLink=LINK( this, SvxTPFilter, RowEnableHdl) ;
    m_pCbDate->SetClickHdl(aLink);
    m_pCbAuthor->SetClickHdl(aLink);
    m_pCbRange->SetClickHdl(aLink);
    m_pCbAction->SetClickHdl(aLink);
    m_pCbComment->SetClickHdl(aLink);

    Link a2Link=LINK( this, SvxTPFilter, ModifyDate);
    m_pDfDate->SetModifyHdl(a2Link);
    m_pTfDate->SetModifyHdl(a2Link);
    m_pDfDate2->SetModifyHdl(a2Link);
    m_pTfDate2->SetModifyHdl(a2Link);

    Link a3Link=LINK( this, SvxTPFilter, ModifyHdl);
    m_pEdRange->SetModifyHdl(a3Link);
    m_pEdComment->SetModifyHdl(a3Link);
    m_pLbAction->SetSelectHdl(a3Link);
    m_pLbAuthor->SetSelectHdl(a3Link);

    RowEnableHdl(m_pCbDate);
    RowEnableHdl(m_pCbAuthor);
    RowEnableHdl(m_pCbRange);
    RowEnableHdl(m_pCbAction);
    RowEnableHdl(m_pCbComment);

    Date aDate( Date::SYSTEM );
    Time aTime( Time::SYSTEM );
    m_pDfDate->SetDate(aDate);
    m_pTfDate->SetTime(aTime);
    m_pDfDate2->SetDate(aDate);
    m_pTfDate2->SetTime(aTime);
    HideRange();
    ShowAction();
    bModified=sal_False;
}

void SvxTPFilter::SetRedlinTable(SvxRedlinTable* pTable)
{
    pRedlinTable=pTable;
}

void SvxTPFilter::ShowDateFields(sal_uInt16 nKind)
{
    switch(nKind)
    {
        case FLT_DATE_BEFORE:
                EnableDateLine1(sal_True);
                EnableDateLine2(sal_False);
                break;
        case FLT_DATE_SINCE:
                EnableDateLine1(sal_True);
                EnableDateLine2(sal_False);
                break;
        case FLT_DATE_EQUAL:
                EnableDateLine1(sal_True);
                m_pTfDate->Disable();
                m_pTfDate->SetText(OUString());
                EnableDateLine2(sal_False);
                break;
        case FLT_DATE_NOTEQUAL:
                EnableDateLine1(sal_True);
                m_pTfDate->Disable();
                m_pTfDate->SetText(OUString());
                EnableDateLine2(sal_False);
                break;
        case FLT_DATE_BETWEEN:
                EnableDateLine1(sal_True);
                EnableDateLine2(sal_True);
                break;
        case FLT_DATE_SAVE:
                EnableDateLine1(sal_False);
                EnableDateLine2(sal_False);
                break;
    }
}

void SvxTPFilter::EnableDateLine1(sal_Bool bFlag)
{
    if(bFlag && m_pCbDate->IsChecked())
    {
        m_pDfDate->Enable();
        m_pTfDate->Enable();
        m_pIbClock->Enable();
    }
    else
    {
        m_pDfDate->Disable();
        m_pTfDate->Disable();
        m_pIbClock->Disable();
    }
}
void SvxTPFilter::EnableDateLine2(sal_Bool bFlag)
{
    if(bFlag && m_pCbDate->IsChecked())
    {
        m_pFtDate2->Enable();
        m_pDfDate2->Enable();
        m_pTfDate2->Enable();
        m_pIbClock2->Enable();
    }
    else
    {
        m_pFtDate2->Disable();
        m_pDfDate2->Disable();
        m_pDfDate2->SetText(OUString());
        m_pTfDate2->Disable();
        m_pTfDate2->SetText(OUString());
        m_pIbClock2->Disable();
    }
}

Date SvxTPFilter::GetFirstDate() const
{
    return m_pDfDate->GetDate();
}

void SvxTPFilter::SetFirstDate(const Date &aDate)
{
    m_pDfDate->SetDate(aDate);
}

Time SvxTPFilter::GetFirstTime() const
{
    return m_pTfDate->GetTime();
}

void SvxTPFilter::SetFirstTime(const Time &aTime)
{
    m_pTfDate->SetTime(aTime);
}


Date SvxTPFilter::GetLastDate() const
{
    return m_pDfDate2->GetDate();
}

void SvxTPFilter::SetLastDate(const Date &aDate)
{
    m_pDfDate2->SetDate(aDate);
}

Time SvxTPFilter::GetLastTime() const
{
    return m_pTfDate2->GetTime();
}

void SvxTPFilter::SetLastTime(const Time &aTime)
{
    m_pTfDate2->SetTime(aTime);
}

void SvxTPFilter::SetDateMode(sal_uInt16 nMode)
{
    m_pLbDate->SelectEntryPos(nMode);
    SelDateHdl(m_pLbDate);
}

sal_uInt16 SvxTPFilter::GetDateMode()
{
    return (sal_uInt16) m_pLbDate->GetSelectEntryPos();
}
void SvxTPFilter::ClearAuthors()
{
    m_pLbAuthor->Clear();
}

void SvxTPFilter::InsertAuthor( const OUString& rString, sal_uInt16 nPos)
{
    m_pLbAuthor->InsertEntry(rString,nPos);
}

OUString SvxTPFilter::GetSelectedAuthor() const
{
    return m_pLbAuthor->GetSelectEntry();
}

void SvxTPFilter::SelectedAuthorPos(sal_uInt16 nPos)
{
    m_pLbAuthor->SelectEntryPos(nPos);
}

sal_uInt16 SvxTPFilter::SelectAuthor(const OUString& aString)
{
    m_pLbAuthor->SelectEntry(aString);
    return m_pLbAuthor->GetSelectEntryPos();
}

void SvxTPFilter::SetRange(const OUString& rString)
{
    m_pEdRange->SetText(rString);
}

OUString SvxTPFilter::GetRange() const
{
    return m_pEdRange->GetText();
}

void SvxTPFilter::SetFocusToRange()
{
    m_pEdRange->GrabFocus();
}

void SvxTPFilter::HideRange(sal_Bool bHide)
{
    if (bHide)
    {
        m_pCbRange->Hide();
        m_pEdRange->Hide();
        m_pBtnRange->Hide();
    }
    else
    {
        ShowAction(false);
        m_pCbRange->Show();
        m_pEdRange->Show();
        m_pBtnRange->Show();
    }
}

void SvxTPFilter::SetComment(const OUString &rComment)
{
    m_pEdComment->SetText(rComment);
}

OUString SvxTPFilter::GetComment()const
{
    return m_pEdComment->GetText();
}

sal_Bool SvxTPFilter::IsDate()
{
    return m_pCbDate->IsChecked();
}

sal_Bool SvxTPFilter::IsAuthor()
{
    return m_pCbAuthor->IsChecked();
}

sal_Bool SvxTPFilter::IsRange()
{
    return m_pCbRange->IsChecked();
}

sal_Bool SvxTPFilter::IsAction()
{
    return m_pCbAction->IsChecked();
}

sal_Bool SvxTPFilter::IsComment()
{
    return m_pCbComment->IsChecked();
}

void SvxTPFilter::CheckDate(sal_Bool bFlag)
{
    m_pCbDate->Check(bFlag);
    RowEnableHdl(m_pCbDate);
    bModified=sal_False;
}

void SvxTPFilter::CheckAuthor(sal_Bool bFlag)
{
    m_pCbAuthor->Check(bFlag);
    RowEnableHdl(m_pCbAuthor);
    bModified=sal_False;
}

void SvxTPFilter::CheckRange(sal_Bool bFlag)
{
    m_pCbRange->Check(bFlag);
    RowEnableHdl(m_pCbRange);
    bModified=sal_False;
}

void SvxTPFilter::CheckAction(sal_Bool bFlag)
{
    m_pCbAction->Check(bFlag);
    RowEnableHdl(m_pCbAction);
    bModified=sal_False;
}

void SvxTPFilter::CheckComment(sal_Bool bFlag)
{
    m_pCbComment->Check(bFlag);
    RowEnableHdl(m_pCbComment);
    bModified=sal_False;
}

void SvxTPFilter::ShowAction(sal_Bool bShow)
{
    if(!bShow)
    {
        m_pCbAction->Hide();
        m_pLbAction->Hide();
    }
    else
    {
        HideRange();
        m_pCbAction->Show();
        m_pLbAction->Show();
    }
}

ListBox* SvxTPFilter::GetLbAction()
{
    return m_pLbAction;
}

IMPL_LINK( SvxTPFilter, SelDateHdl, ListBox*, pLb )
{
    ShowDateFields((sal_uInt16)m_pLbDate->GetSelectEntryPos());
    ModifyHdl(pLb);
    return 0;
}

IMPL_LINK( SvxTPFilter, RowEnableHdl, CheckBox*, pCB )
{
    if (pCB == m_pCbDate)
    {
        m_pLbDate->Enable(m_pCbDate->IsChecked());
        m_pLbDate->Invalidate();
        EnableDateLine1(sal_False);
        EnableDateLine2(sal_False);
        if(m_pCbDate->IsChecked()) SelDateHdl(m_pLbDate);
    }
    else if (pCB == m_pCbAuthor)
    {
        m_pLbAuthor->Enable(m_pCbAuthor->IsChecked());
        m_pLbAuthor->Invalidate();
    }
    else if (pCB == m_pCbRange)
    {
        m_pEdRange->Enable(m_pCbRange->IsChecked());
        m_pBtnRange->Enable(m_pCbRange->IsChecked());
    }
    else if (pCB == m_pCbAction)
    {
        m_pLbAction->Enable(m_pCbAction->IsChecked());
        m_pLbAction->Invalidate();
    }
    else if (pCB == m_pCbComment)
    {
        m_pEdComment->Enable(m_pCbComment->IsChecked());
        m_pEdComment->Invalidate();
    }

    ModifyHdl(pCB);
    return 0;
}

IMPL_LINK( SvxTPFilter, TimeHdl, ImageButton*,pIB )
{
    Date aDate( Date::SYSTEM );
    Time aTime( Time::SYSTEM );
    if (pIB == m_pIbClock)
    {
        m_pDfDate->SetDate(aDate);
        m_pTfDate->SetTime(aTime);
    }
    else if (pIB == m_pIbClock2)
    {
        m_pDfDate2->SetDate(aDate);
        m_pTfDate2->SetTime(aTime);
    }
    ModifyHdl(m_pDfDate);
    return 0;
}

IMPL_LINK( SvxTPFilter, ModifyHdl, void*, pCtr)
{
    if(pCtr!=NULL)
    {
        if (pCtr == m_pCbDate || pCtr == m_pLbDate ||
            pCtr == m_pDfDate || pCtr == m_pTfDate ||
            pCtr == m_pIbClock || pCtr == m_pFtDate2 ||
            pCtr == m_pDfDate2 || pCtr == m_pTfDate2 ||
            pCtr == m_pIbClock2)
        {
            aModifyDateLink.Call(this);
        }
        else if (pCtr == m_pCbAuthor || pCtr == m_pLbAuthor)
        {
            aModifyAuthorLink.Call(this);
        }
        else if (pCtr == m_pCbRange || pCtr == m_pLbAction || pCtr == m_pEdRange ||
                pCtr == m_pBtnRange)
        {
            aModifyRefLink.Call(this);
        }
        else if (pCtr == m_pCbComment || pCtr == m_pEdComment)
        {
            aModifyComLink.Call(this);
        }

        bModified=sal_True;
        aModifyLink.Call(this);
    }
    return 0;
}

void SvxTPFilter::DeactivatePage()
{
    if(bModified)
    {
        if(pRedlinTable!=NULL)
        {
            pRedlinTable->SetFilterDate(IsDate());
            pRedlinTable->SetDateTimeMode(GetDateMode());
            pRedlinTable->SetFirstDate(m_pDfDate->GetDate());
            pRedlinTable->SetLastDate(m_pDfDate2->GetDate());
            pRedlinTable->SetFirstTime(m_pTfDate->GetTime());
            pRedlinTable->SetLastTime(m_pTfDate2->GetTime());
            pRedlinTable->SetFilterAuthor(IsAuthor());
            pRedlinTable->SetAuthor(GetSelectedAuthor());

            pRedlinTable->SetFilterComment(IsComment());

            utl::SearchParam aSearchParam( m_pEdComment->GetText(),
                    utl::SearchParam::SRCH_REGEXP,sal_False,sal_False,sal_False );

            pRedlinTable->SetCommentParams(&aSearchParam);

            pRedlinTable->UpdateFilterTest();
        }

        aReadyLink.Call(this);
    }
    bModified=sal_False;
    TabPage::DeactivatePage();
}

void SvxTPFilter::Enable( bool bEnable, bool bChild)
{
    TabPage::Enable(bEnable,bChild);
    if(m_pCbDate->IsEnabled())
    {
        RowEnableHdl(m_pCbDate);
        RowEnableHdl(m_pCbAuthor);
        RowEnableHdl(m_pCbRange);
        RowEnableHdl(m_pCbComment);
    }
}
void SvxTPFilter::Disable( bool bChild)
{
    Enable( false, bChild );
}

IMPL_LINK( SvxTPFilter, ModifyDate, void*,pTF)
{

    Date aDate( Date::SYSTEM );
    Time aTime(0);
    if (m_pDfDate==pTF)
    {
        if(m_pDfDate->GetText().isEmpty())
           m_pDfDate->SetDate(aDate);

        if(pRedlinTable!=NULL)
            pRedlinTable->SetFirstDate(m_pDfDate->GetDate());
    }
    else if (m_pDfDate2==pTF)
    {
        if(m_pDfDate2->GetText().isEmpty())
           m_pDfDate2->SetDate(aDate);

        if(pRedlinTable!=NULL)
            pRedlinTable->SetLastDate(m_pDfDate2->GetDate());
    }
    else if (m_pTfDate==pTF)
    {
        if(m_pTfDate->GetText().isEmpty())
           m_pTfDate->SetTime(aTime);

        if(pRedlinTable!=NULL)
            pRedlinTable->SetFirstTime(m_pTfDate->GetTime());
    }
    else if (m_pTfDate2==pTF)
    {
        if(m_pTfDate2->GetText().isEmpty())
           m_pTfDate2->SetTime(aTime);

        if(pRedlinTable!=NULL)
            pRedlinTable->SetLastTime(m_pTfDate2->GetTime());

    }
    ModifyHdl(m_pDfDate);
    return 0;
}

IMPL_LINK( SvxTPFilter, RefHandle, PushButton*, pRef )
{
    if(pRef!=NULL)
    {
        aRefLink.Call(this);
    }
    return 0;
}

//============================================================================
//  class SvxAcceptChgCtr
//----------------------------------------------------------------------------

SvxAcceptChgCtr::SvxAcceptChgCtr(Window* pParent)
    : TabControl(pParent, WB_TABSTOP | WB_DIALOGCONTROL)
{
    m_pUIBuilder = new VclBuilder(this, getUIRootDir(), "svx/ui/redlinecontrol.ui", "RedlineControl");

    pTPFilter = new SvxTPFilter(this);
    pTPView = new SvxTPView(this);

    m_nViewPageId = GetPageId("view");
    m_nFilterPageId = GetPageId("filter");
    SetTabPage(m_nViewPageId, pTPView);
    SetTabPage(m_nFilterPageId, pTPFilter);

    pTPFilter->SetRedlinTable(GetViewTable());

    ShowViewPage();

    Show();
}

SvxAcceptChgCtr::~SvxAcceptChgCtr()
{
    delete pTPView;
    delete pTPFilter;
}

void SvxAcceptChgCtr::ShowFilterPage()
{
    SetCurPageId(m_nFilterPageId);
}

void SvxAcceptChgCtr::ShowViewPage()
{
    SetCurPageId(m_nViewPageId);
}

SvxTPFilter* SvxAcceptChgCtr::GetFilterPage()
{
    return pTPFilter;
}

SvxTPView* SvxAcceptChgCtr::GetViewPage()
{
    return pTPView;
}

SvxRedlinTable* SvxAcceptChgCtr::GetViewTable()
{
    return pTPView ? pTPView->GetTableControl() : NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
