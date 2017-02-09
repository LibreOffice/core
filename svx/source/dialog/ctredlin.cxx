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

#include <o3tl/make_unique.hxx>
#include <vcl/dialog.hxx>
#include <vcl/layout.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <unotools/textsearch.hxx>
#include <svtools/svlbitm.hxx>
#include "svtools/viewdataentry.hxx"
#include <unotools/charclass.hxx>

#include <editeng/unolingu.hxx>
#include <svx/dialmgr.hxx>
#include "svx/dialogs.hrc"
#include <svx/ctredlin.hxx>
#include "helpid.hrc"

#define WRITER_DATE     2
#define CALC_DATE       3

RedlinData::RedlinData() : aDateTime(DateTime::EMPTY)
{
    bDisabled=false;
    pData=nullptr;
}

RedlinData::~RedlinData()
{
}

SvxRedlinEntry::SvxRedlinEntry()
    : SvTreeListEntry()
{
}

SvxRedlinEntry::~SvxRedlinEntry()
{
    RedlinData* pRedDat=static_cast<RedlinData*>(GetUserData());
    delete pRedDat;
}

SvLBoxColorString::SvLBoxColorString(const OUString& rStr, const Color& rCol)
    : SvLBoxString(rStr)
{
    aPrivColor=rCol;
    SetText( rStr );
}

SvLBoxColorString::SvLBoxColorString()
: SvLBoxString()
{
}

SvLBoxColorString::~SvLBoxColorString()
{
}

SvLBoxItem* SvLBoxColorString::Create() const
{
    return new SvLBoxColorString;
}

void SvLBoxColorString::Paint(const Point& rPos, SvTreeListBox& rDev, vcl::RenderContext& rRenderContext,
                              const SvViewDataEntry* pView, const SvTreeListEntry& rEntry)
{
    Color aColor = rRenderContext.GetTextColor();
    if (!pView->IsSelected())
    {
        rRenderContext.SetTextColor(aPrivColor);
    }
    SvLBoxString::Paint(rPos, rDev, rRenderContext, pView, rEntry);
    rRenderContext.SetTextColor(aColor);
}

SvxRedlinTable::SvxRedlinTable(SvSimpleTableContainer& rParent, WinBits nBits)
    : SvSimpleTable(rParent,nBits)
    , bIsCalc(false)
    , nDatePos(WRITER_DATE)
    , bAuthor(false)
    , bDate(false)
    , bComment(false)
    , nDaTiMode(SvxRedlinDateMode::BEFORE)
    , aDaTiFirst( DateTime::EMPTY )
    , aDaTiLast( DateTime::EMPTY )
    , aDaTiFilterFirst( DateTime::EMPTY )
    , aDaTiFilterLast( DateTime::EMPTY )
    , pCommentSearcher(nullptr)
{
    SetNodeDefaultImages();
}

SvxRedlinTable::~SvxRedlinTable()
{
    disposeOnce();
}

void SvxRedlinTable::dispose()
{
    delete pCommentSearcher;
    pCommentSearcher = nullptr;
    SvSimpleTable::dispose();
}

sal_Int32 SvxRedlinTable::ColCompare(SvTreeListEntry* pLeft,SvTreeListEntry* pRight)
{
    sal_Int32 nCompare = 0;

    if(aColCompareLink.IsSet())
    {
        SvSortData aRedlinCompare;
        aRedlinCompare.pLeft=pLeft;
        aRedlinCompare.pRight=pRight;
        nCompare = aColCompareLink.Call(&aRedlinCompare);
    }
    else
    {
        if(nDatePos==GetSortedCol())
        {
            RedlinData *pLeftData=static_cast<RedlinData *>(pLeft->GetUserData());
            RedlinData *pRightData=static_cast<RedlinData *>(pRight->GetUserData());

            if(pLeftData!=nullptr && pRightData!=nullptr)
            {
                if(pLeftData->aDateTime < pRightData->aDateTime)
                {
                    nCompare = -1;
                }
                else if(pLeftData->aDateTime > pRightData->aDateTime)
                {
                    nCompare = 1;
                }
            }
            else
                nCompare = SvSimpleTable::ColCompare(pLeft,pRight);
        }
        else
        {
            nCompare = SvSimpleTable::ColCompare(pLeft,pRight);
        }

    }

    return nCompare;
}
void SvxRedlinTable::SetCalcView()
{
    bIsCalc=true;
    nDatePos=CALC_DATE;
}

void SvxRedlinTable::UpdateFilterTest()
{
    Date aDateMax( Date::SYSTEM );
    aDateMax.AddYears(100);
    Date aDateMin(1,1,1989);
    tools::Time aTMin(0);
    tools::Time aTMax(23,59,59);

    DateTime aDTMin(aDateMin);
    DateTime aDTMax(aDateMax);

    switch(nDaTiMode)
    {
        case SvxRedlinDateMode::BEFORE:
                                aDaTiFilterFirst=aDTMin;
                                aDaTiFilterLast=aDaTiFirst;
                                break;
        case SvxRedlinDateMode::SAVE:
        case SvxRedlinDateMode::SINCE:
                                aDaTiFilterFirst=aDaTiFirst;
                                aDaTiFilterLast=aDTMax;
                                break;
        case SvxRedlinDateMode::EQUAL:
                                aDaTiFilterFirst=aDaTiFirst;
                                aDaTiFilterLast=aDaTiFirst;
                                aDaTiFilterFirst.SetTime(aTMin.GetTime());
                                aDaTiFilterLast.SetTime(aTMax.GetTime());
                                break;
        case SvxRedlinDateMode::NOTEQUAL:
                                aDaTiFilterFirst=aDaTiFirst;
                                aDaTiFilterLast=aDaTiFirst;
                                aDaTiFilterFirst.SetTime(aTMin.GetTime());
                                aDaTiFilterLast.SetTime(aTMax.GetTime());
                                break;
        case SvxRedlinDateMode::BETWEEN:
                                aDaTiFilterFirst=aDaTiFirst;
                                aDaTiFilterLast=aDaTiLast;
                                break;
        case SvxRedlinDateMode::NONE:
                                break;
    }
}

void SvxRedlinTable::SetFilterDate(bool bFlag)
{
    bDate=bFlag;
}

void SvxRedlinTable::SetDateTimeMode(SvxRedlinDateMode nMode)
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

void SvxRedlinTable::SetFirstTime(const tools::Time& aTime)
{
    aDaTiFirst.SetTime(aTime.GetTime());
}

void SvxRedlinTable::SetLastTime(const tools::Time& aTime)
{
    aDaTiLast.SetTime(aTime.GetTime());
}

void SvxRedlinTable::SetFilterAuthor(bool bFlag)
{
    bAuthor=bFlag;
}

void SvxRedlinTable::SetAuthor(const OUString &aString)
{
    aAuthor=aString;
}

void SvxRedlinTable::SetFilterComment(bool bFlag)
{
    bComment=bFlag;
}

void SvxRedlinTable::SetCommentParams( const utl::SearchParam* pSearchPara )
{
    if(pSearchPara!=nullptr)
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
    return nDaTiMode!=SvxRedlinDateMode::NOTEQUAL ? bRes : !bRes;
}

bool SvxRedlinTable::IsValidComment(const OUString &rCommentStr)
{
    if (!bComment)
        return true;

    sal_Int32 nStartPos = 0;
    sal_Int32 nEndPos = rCommentStr.getLength();
    return pCommentSearcher->SearchForward( rCommentStr, &nStartPos, &nEndPos);
}

SvTreeListEntry* SvxRedlinTable::InsertEntry(const OUString& rStr,
        RedlinData *pUserData, SvTreeListEntry* pParent, sal_uIntPtr nPos)
{
    const Color aColor = (pUserData && pUserData->bDisabled) ? Color(COL_GRAY) : GetTextColor();

    return InsertEntry(rStr, pUserData, aColor, pParent, nPos);
}

SvTreeListEntry* SvxRedlinTable::InsertEntry(const OUString& rStr,
        RedlinData *pUserData, const Color& rColor, SvTreeListEntry* pParent, sal_uIntPtr nPos)
{
    maEntryColor = rColor;
    maEntryImage = Image();

    sal_Int32 nIndex = 0;
    const OUString aFirstStr(rStr.getToken(0, '\t', nIndex));
    maEntryString = nIndex > 0 ? rStr.copy(nIndex) : OUString();

    return SvSimpleTable::InsertEntry(aFirstStr, pParent, false, nPos, pUserData);
}

SvTreeListEntry* SvxRedlinTable::InsertEntry(const Image &rRedlineType, const OUString& rStr,
        RedlinData *pUserData, SvTreeListEntry* pParent, sal_uIntPtr nPos)
{
    maEntryColor = (pUserData && pUserData->bDisabled) ? Color(COL_GRAY) : GetTextColor();
    maEntryImage = rRedlineType;
    maEntryString = rStr;

    return SvSimpleTable::InsertEntry(OUString(), pParent, false, nPos, pUserData);
}

SvTreeListEntry* SvxRedlinTable::CreateEntry() const
{
    return new SvxRedlinEntry;
}

void SvxRedlinTable::InitEntry(SvTreeListEntry* pEntry, const OUString& rStr,
        const Image& rColl, const Image& rExp, SvLBoxButtonKind eButtonKind)
{
    if (nTreeFlags & SvTreeFlags::CHKBTN)
    {
        pEntry->AddItem(o3tl::make_unique<SvLBoxButton>(
                eButtonKind, pCheckButtonData));
    }

    pEntry->AddItem(o3tl::make_unique<SvLBoxContextBmp>(
                rColl, rExp, true));

    // the type of the change
    assert((rStr.isEmpty() && !!maEntryImage) || (!rStr.isEmpty() && !maEntryImage));

    if (rStr.isEmpty())
        pEntry->AddItem(o3tl::make_unique<SvLBoxContextBmp>(
                        maEntryImage, maEntryImage, true));
    else
        pEntry->AddItem(o3tl::make_unique<SvLBoxColorString>(
                    rStr, maEntryColor));

    // the change tracking entries
    sal_Int32 nIndex = 0;
    const sal_uInt16 nCount = TabCount() - 1;
    for (sal_uInt16 nToken = 0; nToken < nCount; nToken++)
    {
        const OUString aToken = GetToken(maEntryString, nIndex);
        pEntry->AddItem(o3tl::make_unique<SvLBoxColorString>(
                    aToken, maEntryColor));
    }
}

SvxTPView::SvxTPView(vcl::Window *pParent, VclBuilderContainer *pTopLevel)
    : TabPage(pParent, "RedlineViewPage", "svx/ui/redlineviewpage.ui")
    , bEnableAccept(true)
    , bEnableAcceptAll(true)
    , bEnableReject(true)
    , bEnableRejectAll(true)
    , bEnableUndo(true)
{
    pTopLevel->get(m_pAccept, "accept");
    pTopLevel->get(m_pReject, "reject");
    pTopLevel->get(m_pAcceptAll, "acceptall");
    pTopLevel->get(m_pRejectAll, "rejectall");
    pTopLevel->get(m_pUndo, "undo");

    SvSimpleTableContainer* pTable = get<SvSimpleTableContainer>("changes");
    Size aControlSize(80, 65);
    aControlSize = LogicToPixel(aControlSize, MapUnit::MapAppFont);
    pTable->set_width_request(aControlSize.Width());
    pTable->set_height_request(aControlSize.Height());
    m_pViewData = VclPtr<SvxRedlinTable>::Create(*pTable, 0);

    Link<Button*,void> aLink=LINK( this, SvxTPView, PbClickHdl);

    m_pAccept->SetClickHdl(aLink);
    m_pAcceptAll->SetClickHdl(aLink);
    m_pReject->SetClickHdl(aLink);
    m_pRejectAll->SetClickHdl(aLink);
    m_pUndo->SetClickHdl(aLink);
}

void SvxTPView::ActivatePage()
{
    m_pAccept->Enable(bEnableAccept);
    m_pReject->Enable(bEnableReject);
    m_pAcceptAll->Enable(bEnableAcceptAll);
    m_pRejectAll->Enable(bEnableRejectAll);
    m_pUndo->Enable(bEnableUndo);
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
    disposeOnce();
}

void SvxTPView::dispose()
{
    m_pViewData.disposeAndClear();
    m_pAccept.clear();
    m_pReject.clear();
    m_pAcceptAll.clear();
    m_pRejectAll.clear();
    m_pUndo.clear();
    TabPage::dispose();
}

void SvxTPView::InsertWriterHeader()
{
    const long pTabs[] = { 5, 10, 20, 70, 120, 170 };
    m_pViewData->SetTabs(pTabs);

    OUString aStrTab('\t');
    OUString aString = get<FixedText>("action")->GetText()
                     + aStrTab
                     + get<FixedText>("author")->GetText()
                     + aStrTab
                     + get<FixedText>("date")->GetText()
                     + aStrTab
                     + get<FixedText>("comment")->GetText();
    m_pViewData->ClearHeader();
    m_pViewData->InsertHeaderEntry(aString);
}

void SvxTPView::InsertCalcHeader()
{
    const long pTabs[] = { 5, 10, 65, 120, 170, 220 };
    m_pViewData->SetTabs(pTabs);

    OUString aStrTab('\t');
    OUString aString = get<FixedText>("action")->GetText()
                     + aStrTab
                     + get<FixedText>("position")->GetText()
                     + aStrTab
                     + get<FixedText>("author")->GetText()
                     + aStrTab
                     + get<FixedText>("date")->GetText()
                     + aStrTab
                     + get<FixedText>("comment")->GetText();
    m_pViewData->ClearHeader();
    m_pViewData->InsertHeaderEntry(aString);
}

void SvxTPView::EnableAccept(bool bFlag)
{
    bEnableAccept = bFlag;
    m_pAccept->Enable(bFlag);
}

void SvxTPView::EnableAcceptAll(bool bFlag)
{
    bEnableAcceptAll = bFlag;
    m_pAcceptAll->Enable(bFlag);
}

void SvxTPView::EnableReject(bool bFlag)
{
    bEnableReject = bFlag;
    m_pReject->Enable(bFlag);
}

void SvxTPView::EnableRejectAll(bool bFlag)
{
    bEnableRejectAll = bFlag;
    m_pRejectAll->Enable(bFlag);
}

void SvxTPView::ShowUndo()
{
    m_pUndo->Show();
}

void SvxTPView::EnableUndo(bool bFlag)
{
    bEnableUndo = bFlag;
    m_pUndo->Enable(bFlag);
}


IMPL_LINK( SvxTPView, PbClickHdl, Button*, pButton, void )
{
    PushButton* pPushB = static_cast<PushButton*>(pButton);
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
}

SvxTPFilter::SvxTPFilter( vcl::Window * pParent)
    : TabPage(pParent, "RedlineFilterPage", "svx/ui/redlinefilterpage.ui")
    , pRedlinTable(nullptr)
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

    m_pDfDate->SetShowDateCentury( true );
    m_pDfDate2->SetShowDateCentury( true );

    m_pLbDate->SelectEntryPos(0);
    m_pLbDate->SetSelectHdl( LINK( this, SvxTPFilter, SelDateHdl ) );
    m_pIbClock->SetClickHdl( LINK( this, SvxTPFilter, TimeHdl) );
    m_pIbClock2->SetClickHdl( LINK( this, SvxTPFilter,TimeHdl) );
    m_pBtnRange->SetClickHdl( LINK( this, SvxTPFilter, RefHandle));

    Link<Button*,void> aLink=LINK( this, SvxTPFilter, RowEnableHdl) ;
    m_pCbDate->SetClickHdl(aLink);
    m_pCbAuthor->SetClickHdl(aLink);
    m_pCbRange->SetClickHdl(aLink);
    m_pCbAction->SetClickHdl(aLink);
    m_pCbComment->SetClickHdl(aLink);

    Link<Edit&,void> a2Link=LINK( this, SvxTPFilter, ModifyDate);
    m_pDfDate->SetModifyHdl(a2Link);
    m_pTfDate->SetModifyHdl(a2Link);
    m_pDfDate2->SetModifyHdl(a2Link);
    m_pTfDate2->SetModifyHdl(a2Link);

    Link<Edit&,void> a3Link=LINK( this, SvxTPFilter, ModifyHdl);
    m_pEdRange->SetModifyHdl(a3Link);
    m_pEdComment->SetModifyHdl(a3Link);
    m_pLbAction->SetSelectHdl(LINK( this, SvxTPFilter, ModifyListBoxHdl));
    m_pLbAuthor->SetSelectHdl(LINK( this, SvxTPFilter, ModifyListBoxHdl));

    RowEnableHdl(m_pCbDate);
    RowEnableHdl(m_pCbAuthor);
    RowEnableHdl(m_pCbRange);
    RowEnableHdl(m_pCbAction);
    RowEnableHdl(m_pCbComment);

    Date aDate( Date::SYSTEM );
    tools::Time aTime( tools::Time::SYSTEM );
    m_pDfDate->SetDate(aDate);
    m_pTfDate->SetTime(aTime);
    m_pDfDate2->SetDate(aDate);
    m_pTfDate2->SetTime(aTime);
    HideRange();
    ShowAction();
    bModified=false;
}

SvxTPFilter::~SvxTPFilter()
{
    disposeOnce();
}

void SvxTPFilter::dispose()
{
    pRedlinTable.clear();
    m_pCbDate.clear();
    m_pLbDate.clear();
    m_pDfDate.clear();
    m_pTfDate.clear();
    m_pIbClock.clear();
    m_pFtDate2.clear();
    m_pDfDate2.clear();
    m_pTfDate2.clear();
    m_pIbClock2.clear();
    m_pCbAuthor.clear();
    m_pLbAuthor.clear();
    m_pCbRange.clear();
    m_pEdRange.clear();
    m_pBtnRange.clear();
    m_pCbAction.clear();
    m_pLbAction.clear();
    m_pCbComment.clear();
    m_pEdComment.clear();
    TabPage::dispose();
}

void SvxTPFilter::SetRedlinTable(SvxRedlinTable* pTable)
{
    pRedlinTable=pTable;
}

void SvxTPFilter::EnableDateLine1(bool bFlag)
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
void SvxTPFilter::EnableDateLine2(bool bFlag)
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

tools::Time SvxTPFilter::GetFirstTime() const
{
    return m_pTfDate->GetTime();
}

void SvxTPFilter::SetFirstTime(const tools::Time &aTime)
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

tools::Time SvxTPFilter::GetLastTime() const
{
    return m_pTfDate2->GetTime();
}

void SvxTPFilter::SetLastTime(const tools::Time &aTime)
{
    m_pTfDate2->SetTime(aTime);
}

void SvxTPFilter::SetDateMode(sal_uInt16 nMode)
{
    m_pLbDate->SelectEntryPos(nMode);
    SelDateHdl(*m_pLbDate);
}

SvxRedlinDateMode SvxTPFilter::GetDateMode()
{
    return static_cast<SvxRedlinDateMode>(m_pLbDate->GetSelectEntryPos());
}
void SvxTPFilter::ClearAuthors()
{
    m_pLbAuthor->Clear();
}

void SvxTPFilter::InsertAuthor( const OUString& rString)
{
    m_pLbAuthor->InsertEntry(rString);
}

OUString SvxTPFilter::GetSelectedAuthor() const
{
    return m_pLbAuthor->GetSelectEntry();
}

void SvxTPFilter::SelectedAuthorPos(sal_Int32 nPos)
{
    m_pLbAuthor->SelectEntryPos(nPos);
}

sal_Int32 SvxTPFilter::SelectAuthor(const OUString& aString)
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

void SvxTPFilter::HideRange(bool bHide)
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

bool SvxTPFilter::IsDate()
{
    return m_pCbDate->IsChecked();
}

bool SvxTPFilter::IsAuthor()
{
    return m_pCbAuthor->IsChecked();
}

bool SvxTPFilter::IsRange()
{
    return m_pCbRange->IsChecked();
}

bool SvxTPFilter::IsAction()
{
    return m_pCbAction->IsChecked();
}

bool SvxTPFilter::IsComment()
{
    return m_pCbComment->IsChecked();
}

void SvxTPFilter::CheckDate(bool bFlag)
{
    m_pCbDate->Check(bFlag);
    RowEnableHdl(m_pCbDate);
    bModified=false;
}

void SvxTPFilter::CheckAuthor(bool bFlag)
{
    m_pCbAuthor->Check(bFlag);
    RowEnableHdl(m_pCbAuthor);
    bModified=false;
}

void SvxTPFilter::CheckRange(bool bFlag)
{
    m_pCbRange->Check(bFlag);
    RowEnableHdl(m_pCbRange);
    bModified=false;
}

void SvxTPFilter::CheckAction(bool bFlag)
{
    m_pCbAction->Check(bFlag);
    RowEnableHdl(m_pCbAction);
    bModified=false;
}

void SvxTPFilter::CheckComment(bool bFlag)
{
    m_pCbComment->Check(bFlag);
    RowEnableHdl(m_pCbComment);
    bModified=false;
}

void SvxTPFilter::ShowAction(bool bShow)
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


IMPL_LINK_NOARG( SvxTPFilter, SelDateHdl, ListBox&, void )
{
    SvxRedlinDateMode nKind = static_cast<SvxRedlinDateMode>(m_pLbDate->GetSelectEntryPos());
    switch(nKind)
    {
        case SvxRedlinDateMode::BEFORE:
                EnableDateLine1(true);
                EnableDateLine2(false);
                break;
        case SvxRedlinDateMode::SINCE:
                EnableDateLine1(true);
                EnableDateLine2(false);
                break;
        case SvxRedlinDateMode::EQUAL:
                EnableDateLine1(true);
                m_pTfDate->Disable();
                m_pTfDate->SetText(OUString());
                EnableDateLine2(false);
                break;
        case SvxRedlinDateMode::NOTEQUAL:
                EnableDateLine1(true);
                m_pTfDate->Disable();
                m_pTfDate->SetText(OUString());
                EnableDateLine2(false);
                break;
        case SvxRedlinDateMode::BETWEEN:
                EnableDateLine1(true);
                EnableDateLine2(true);
                break;
        case SvxRedlinDateMode::SAVE:
                EnableDateLine1(false);
                EnableDateLine2(false);
                break;
        case SvxRedlinDateMode::NONE:
                break;
    }
    bModified=true;
}

IMPL_LINK( SvxTPFilter, RowEnableHdl, Button*, pButton, void )
{
    CheckBox* pCB = static_cast<CheckBox*>(pButton);
    if (pCB == m_pCbDate)
    {
        m_pLbDate->Enable(m_pCbDate->IsChecked());
        m_pLbDate->Invalidate();
        EnableDateLine1(false);
        EnableDateLine2(false);
        if(m_pCbDate->IsChecked()) SelDateHdl(*m_pLbDate);
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

    if(pCB!=nullptr)
        bModified=true;
}

IMPL_LINK( SvxTPFilter, TimeHdl, Button*, pButton, void )
{
    ImageButton* pIB = static_cast<ImageButton*>(pButton);
    Date aDate( Date::SYSTEM );
    tools::Time aTime( tools::Time::SYSTEM );
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
    bModified=true;
}
IMPL_LINK_NOARG( SvxTPFilter, ModifyHdl, Edit&, void)
{
    bModified=true;
}
IMPL_LINK_NOARG( SvxTPFilter, ModifyListBoxHdl, ListBox&, void)
{
    bModified=true;
}
void SvxTPFilter::DeactivatePage()
{
    if(bModified)
    {
        if(pRedlinTable!=nullptr)
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
                    utl::SearchParam::SearchType::Regexp,false );

            pRedlinTable->SetCommentParams(&aSearchParam);

            pRedlinTable->UpdateFilterTest();
        }

        aReadyLink.Call(this);
    }
    bModified=false;
    TabPage::DeactivatePage();
}

void SvxTPFilter::Enable( bool bEnable)
{
    TabPage::Enable(bEnable);
    if(m_pCbDate->IsEnabled())
    {
        RowEnableHdl(m_pCbDate);
        RowEnableHdl(m_pCbAuthor);
        RowEnableHdl(m_pCbRange);
        RowEnableHdl(m_pCbComment);
    }
}
void SvxTPFilter::Disable()
{
    Enable( false );
}

IMPL_LINK( SvxTPFilter, ModifyDate, Edit&, rTF, void)
{
    Date aDate( Date::SYSTEM );
    tools::Time aTime(0);
    if (m_pDfDate==&rTF)
    {
        if(m_pDfDate->GetText().isEmpty())
           m_pDfDate->SetDate(aDate);

        if(pRedlinTable!=nullptr)
            pRedlinTable->SetFirstDate(m_pDfDate->GetDate());
    }
    else if (m_pDfDate2==&rTF)
    {
        if(m_pDfDate2->GetText().isEmpty())
           m_pDfDate2->SetDate(aDate);

        if(pRedlinTable!=nullptr)
            pRedlinTable->SetLastDate(m_pDfDate2->GetDate());
    }
    else if (m_pTfDate==&rTF)
    {
        if(m_pTfDate->GetText().isEmpty())
           m_pTfDate->SetTime(aTime);

        if(pRedlinTable!=nullptr)
            pRedlinTable->SetFirstTime(m_pTfDate->GetTime());
    }
    else if (m_pTfDate2==&rTF)
    {
        if(m_pTfDate2->GetText().isEmpty())
           m_pTfDate2->SetTime(aTime);

        if(pRedlinTable!=nullptr)
            pRedlinTable->SetLastTime(m_pTfDate2->GetTime());

    }
    ModifyHdl(*m_pDfDate);
}

IMPL_LINK( SvxTPFilter, RefHandle, Button*, pRef, void )
{
    if(pRef!=nullptr)
    {
        aRefLink.Call(this);
    }
}

SvxAcceptChgCtr::SvxAcceptChgCtr(vcl::Window* pParent, VclBuilderContainer* pTopLevel)
    : TabControl(pParent, WB_TABSTOP | WB_DIALOGCONTROL)
{
    m_pUIBuilder.reset(new VclBuilder(this, getUIRootDir(), "svx/ui/redlinecontrol.ui", "RedlineControl"));

    pTPFilter = VclPtr<SvxTPFilter>::Create(this);
    pTPView = VclPtr<SvxTPView>::Create(this, pTopLevel);

    m_nViewPageId = GetPageId("view");
    m_nFilterPageId = GetPageId("filter");
    SetTabPage(m_nViewPageId, pTPView);
    SetTabPage(m_nFilterPageId, pTPFilter);

    pTPFilter->SetRedlinTable(pTPView->GetTableControl());

    SetCurPageId(m_nViewPageId);

    Show();
}

SvxAcceptChgCtr::~SvxAcceptChgCtr()
{
    disposeOnce();
}

void SvxAcceptChgCtr::dispose()
{
    disposeBuilder();
    pTPView.disposeAndClear();
    pTPFilter.disposeAndClear();
    TabControl::dispose();
}

void SvxAcceptChgCtr::ShowFilterPage()
{
    SetCurPageId(m_nFilterPageId);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
