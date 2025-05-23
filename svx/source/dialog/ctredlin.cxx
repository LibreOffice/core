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

#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <o3tl/safeint.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/weldutils.hxx>
#include <svtools/ctrlbox.hxx>
#include <unotools/textsearch.hxx>

#include <helpids.h>

#include <svx/ctredlin.hxx>
#include <o3tl/unreachable.hxx>

#define WRITER_DATE     2
#define CALC_DATE       3

RedlinData::RedlinData()
    : aDateTime(DateTime::EMPTY)
    , pData(nullptr)
    , eType(RedlineType::Any)
    , bDisabled(false)
{
}

RedlinData::~RedlinData()
{
}

SvxRedlinTable::SvxRedlinTable(std::unique_ptr<weld::TreeView> xWriterControl,
                               std::unique_ptr<weld::TreeView> xCalcControl,
                               weld::ComboBox* pSortByControl)
    : xSorter(new comphelper::string::NaturalStringSorter(::comphelper::getProcessComponentContext(),
        Application::GetSettings().GetUILanguageTag().getLocale()))
    , xWriterTreeView(std::move(xWriterControl))
    , xCalcTreeView(std::move(xCalcControl))
    , pTreeView(nullptr)
    , m_pSortByComboBox(pSortByControl)
    , nDatePos(WRITER_DATE)
    , bAuthor(false)
    , bDate(false)
    , bComment(false)
    , bSorted(false)
    , nDaTiMode(SvxRedlinDateMode::BEFORE)
    , aDaTiFirst( DateTime::EMPTY )
    , aDaTiLast( DateTime::EMPTY )
    , aDaTiFilterFirst( DateTime::EMPTY )
    , aDaTiFilterLast( DateTime::EMPTY )
{
    if (xWriterTreeView)
    {
        xWriterTreeView->set_size_request(-1, xWriterTreeView->get_height_rows(8));
        xWriterTreeView->connect_column_clicked(LINK(this, SvxRedlinTable, HeaderBarClick));
        xWriterTreeView->set_sort_func([this](const weld::TreeIter& rLeft, const weld::TreeIter& rRight){
            return ColCompare(rLeft, rRight);
        });
        pTreeView = xWriterTreeView.get();
    }
    if (xCalcTreeView)
    {
        xCalcTreeView->set_size_request(-1, xCalcTreeView->get_height_rows(8));
        xCalcTreeView->connect_column_clicked(LINK(this, SvxRedlinTable, HeaderBarClick));
        xCalcTreeView->set_sort_func([this](const weld::TreeIter& rLeft, const weld::TreeIter& rRight){
            return ColCompare(rLeft, rRight);
        });
        pTreeView = xCalcTreeView.get();
    }
}

SvxRedlinTable::~SvxRedlinTable()
{
}

IMPL_LINK(SvxRedlinTable, HeaderBarClick, int, nColumn, void)
{
    if (nColumn == -1)
    {
        pTreeView->make_unsorted();
        bSorted = false;
    }
    else if (!bSorted)
    {
        pTreeView->make_sorted();
        bSorted = true;
    }

    bool bSortAtoZ = pTreeView->get_sort_order();

    //set new arrow positions in headerbar
    if (nColumn != -1 && nColumn == pTreeView->get_sort_column())
    {
        bSortAtoZ = !bSortAtoZ;
        pTreeView->set_sort_order(bSortAtoZ);
    }
    else
    {
        int nOldSortColumn = pTreeView->get_sort_column();
        if (nOldSortColumn != -1)
            pTreeView->set_sort_indicator(TRISTATE_INDET, nOldSortColumn);
        pTreeView->set_sort_column(nColumn);
    }

    if (nColumn != -1)
    {
        //sort lists
        pTreeView->set_sort_indicator(bSortAtoZ ? TRISTATE_TRUE : TRISTATE_FALSE, nColumn);
        if (m_pSortByComboBox)
            m_pSortByComboBox->set_active(nColumn);
    }
}

int SvxRedlinTable::ColCompare(const weld::TreeIter& rLeft, const weld::TreeIter& rRight)
{
    sal_Int32 nCompare = 0;

    int nSortCol = pTreeView->get_sort_column();

    if (pTreeView == xWriterTreeView.get() && nSortCol == 0)
    {
        RedlinData *pLeftData = weld::fromId<RedlinData*>(pTreeView->get_id(rLeft));
        RedlinData *pRightData = weld::fromId<RedlinData*>(pTreeView->get_id(rRight));

        if (pLeftData && pRightData)
        {
            if (pLeftData->eType < pRightData->eType)
                nCompare = -1;
            else if (pLeftData->eType > pRightData->eType)
                nCompare = 1;
            return nCompare;
        }
    }

    if (nSortCol == nDatePos)
    {
        RedlinData *pLeftData = weld::fromId<RedlinData*>(pTreeView->get_id(rLeft));
        RedlinData *pRightData = weld::fromId<RedlinData*>(pTreeView->get_id(rRight));

        if (pLeftData && pRightData)
        {
            if (pLeftData->aDateTime < pRightData->aDateTime)
                nCompare = -1;
            else if (pLeftData->aDateTime > pRightData->aDateTime)
                nCompare = 1;
            return nCompare;
        }
    }

    return xSorter->compare(pTreeView->get_text(rLeft, nSortCol),
                            pTreeView->get_text(rRight, nSortCol));
}

void SvxRedlinTable::UpdateFilterTest()
{
    Date aDateMax( Date::SYSTEM );
    aDateMax.AddYears(100);
    Date aDateMin(1,1,1989);
    tools::Time aTMin(tools::Time::EMPTY);
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
        pCommentSearcher.reset(new utl::TextSearch(*pSearchPara, LANGUAGE_SYSTEM ));
    }
}

bool SvxRedlinTable::IsValidEntry(std::u16string_view rAuthorStr,
                                  const DateTime &rDateTime,
                                  const OUString &rCommentStr)
{
    return IsValidEntry(rAuthorStr, rDateTime) && IsValidComment(rCommentStr);
}

bool SvxRedlinTable::IsValidEntry(std::u16string_view rAuthorStr, const DateTime &rDateTime)
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

SvxTPage::~SvxTPage()
{
}

void SvxTPage::ActivatePage()
{
}

SvxTPView::SvxTPView(weld::Container* pParent)
    : SvxTPage(pParent, u"svx/ui/redlineviewpage.ui"_ustr, u"RedlineViewPage"_ustr)
    , bEnableAccept(true)
    , bEnableAcceptAll(true)
    , bEnableReject(true)
    , bEnableRejectAll(true)
    , bEnableUndo(true)
    , bEnableClearFormat(false)
    , bEnableClearFormatAll(false)
    , m_xAccept(m_xBuilder->weld_button(u"accept"_ustr))
    , m_xReject(m_xBuilder->weld_button(u"reject"_ustr))
    , m_xAcceptAll(m_xBuilder->weld_button(u"acceptall"_ustr))
    , m_xRejectAll(m_xBuilder->weld_button(u"rejectall"_ustr))
    , m_xUndo(m_xBuilder->weld_button(u"undo"_ustr))
    , m_xSortByComboBox(m_xBuilder->weld_combo_box(u"sortbycombobox"_ustr))
    , m_xViewData(new SvxRedlinTable(m_xBuilder->weld_tree_view(u"writerchanges"_ustr),
                                     m_xBuilder->weld_tree_view(u"calcchanges"_ustr),
                                     m_xSortByComboBox.get()))
{
    m_xSortByComboBox->connect_changed(LINK(this, SvxTPView, SortByComboBoxChangedHdl));

    Link<weld::Button&,void> aLink=LINK( this, SvxTPView, PbClickHdl);

    m_xAccept->connect_clicked(aLink);
    m_xAcceptAll->connect_clicked(aLink);
    m_xReject->connect_clicked(aLink);
    m_xRejectAll->connect_clicked(aLink);
    m_xUndo->connect_clicked(aLink);
}

void SvxTPView::ActivatePage()
{
    m_xAccept->set_sensitive(bEnableAccept);
    m_xReject->set_sensitive(bEnableReject);
    m_xAcceptAll->set_sensitive(bEnableAcceptAll);
    m_xRejectAll->set_sensitive(bEnableRejectAll);
    m_xUndo->set_sensitive(bEnableUndo);
}

void SvxTPView::DeactivatePage()
{
    m_xAccept->set_sensitive(false);
    m_xReject->set_sensitive(false);
    m_xAcceptAll->set_sensitive(false);
    m_xRejectAll->set_sensitive(false);
    m_xUndo->set_sensitive(false);
}

SvxTPView::~SvxTPView()
{
}

void SvxRedlinTable::SetWriterView()
{
    nDatePos = WRITER_DATE;
    if (xCalcTreeView)
        xCalcTreeView->hide();
    xWriterTreeView->show();
    if (m_pSortByComboBox)
        m_pSortByComboBox->weld_parent()->show();
    pTreeView = xWriterTreeView.get();

    auto nDigitWidth = pTreeView->get_approximate_digit_width();
    std::vector<int> aWidths
    {
        o3tl::narrowing<int>(nDigitWidth * 10),
        o3tl::narrowing<int>(nDigitWidth * 20),
        o3tl::narrowing<int>(nDigitWidth * 20)
    };
    pTreeView->set_column_fixed_widths(aWidths);
}

void SvxRedlinTable::SetCalcView()
{
    nDatePos = CALC_DATE;
    if (xWriterTreeView)
        xWriterTreeView->hide();
    if (m_pSortByComboBox)
        m_pSortByComboBox->weld_parent()->hide();
    xCalcTreeView->show();
    pTreeView = xCalcTreeView.get();

    auto nDigitWidth = pTreeView->get_approximate_digit_width();
    std::vector<int> aWidths
    {
        o3tl::narrowing<int>(nDigitWidth * 20),
        o3tl::narrowing<int>(nDigitWidth * 20),
        o3tl::narrowing<int>(nDigitWidth * 20),
        o3tl::narrowing<int>(nDigitWidth * 20)
    };
    pTreeView->set_column_fixed_widths(aWidths);
}

void SvxTPView::EnableAccept(bool bFlag)
{
    bEnableAccept = bFlag;
    m_xAccept->set_sensitive(bFlag);
}

void SvxTPView::EnableAcceptAll(bool bFlag)
{
    bEnableAcceptAll = bFlag;
    m_xAcceptAll->set_sensitive(bFlag);
}

void SvxTPView::EnableReject(bool bFlag)
{
    bEnableReject = bFlag;
    m_xReject->set_sensitive(bFlag);
}

void SvxTPView::EnableRejectAll(bool bFlag)
{
    bEnableRejectAll = bFlag;
    m_xRejectAll->set_sensitive(bFlag);
}

void SvxTPView::EnableClearFormat(bool bFlag)
{
    if (bEnableClearFormat == bFlag)
        return;
    bEnableClearFormat = bFlag;
}

void SvxTPView::EnableClearFormatAll(bool bFlag)
{
    if (bEnableClearFormatAll == bFlag)
        return;
    bEnableClearFormatAll = bFlag;
}

void SvxTPView::ShowUndo()
{
    m_xUndo->show();
}

void SvxTPView::EnableUndo(bool bFlag)
{
    bEnableUndo = bFlag;
    m_xUndo->set_sensitive(bFlag);
}

IMPL_LINK_NOARG(SvxTPView, SortByComboBoxChangedHdl, weld::ComboBox&, void)
{
    if (SortByComboBoxChangedLk.IsSet())
        SortByComboBoxChangedLk.Call(this);
}

IMPL_LINK( SvxTPView, PbClickHdl, weld::Button&, rPushB, void)
{
    if (&rPushB == m_xAccept.get())
    {
        AcceptClickLk.Call(this);
    }
    else if (&rPushB == m_xAcceptAll.get())
    {
        AcceptAllClickLk.Call(this);
    }
    else if (&rPushB == m_xReject.get())
    {
        RejectClickLk.Call(this);
    }
    else if (&rPushB == m_xRejectAll.get())
    {
        RejectAllClickLk.Call(this);
    }
    else if (&rPushB == m_xUndo.get())
    {
        UndoClickLk.Call(this);
    }
}

SvxTPage::SvxTPage(weld::Container* pParent, const OUString& rUIXMLDescription, const OUString& rID)
    : m_xBuilder(Application::CreateBuilder(pParent, rUIXMLDescription))
    , m_xContainer(m_xBuilder->weld_container(rID))
{
}

SvxTPFilter::SvxTPFilter(weld::Container* pParent)
    : SvxTPage(pParent, u"svx/ui/redlinefilterpage.ui"_ustr, u"RedlineFilterPage"_ustr)
    , bModified(false)
    , m_pRedlinTable(nullptr)
    , m_xCbDate(m_xBuilder->weld_check_button(u"date"_ustr))
    , m_xLbDate(m_xBuilder->weld_combo_box(u"datecond"_ustr))
    , m_xDfDate(new SvtCalendarBox(m_xBuilder->weld_menu_button(u"startdate"_ustr)))
    , m_xTfDate(m_xBuilder->weld_formatted_spin_button(u"starttime"_ustr))
    , m_xTfDateFormatter(new weld::TimeFormatter(*m_xTfDate))
    , m_xIbClock(m_xBuilder->weld_button(u"startclock"_ustr))
    , m_xFtDate2(m_xBuilder->weld_label(u"and"_ustr))
    , m_xDfDate2(new SvtCalendarBox(m_xBuilder->weld_menu_button(u"enddate"_ustr)))
    , m_xTfDate2(m_xBuilder->weld_formatted_spin_button(u"endtime"_ustr))
    , m_xTfDate2Formatter(new weld::TimeFormatter(*m_xTfDate2))
    , m_xIbClock2(m_xBuilder->weld_button(u"endclock"_ustr))
    , m_xCbAuthor(m_xBuilder->weld_check_button(u"author"_ustr))
    , m_xLbAuthor(m_xBuilder->weld_combo_box(u"authorlist"_ustr))
    , m_xCbRange(m_xBuilder->weld_check_button(u"range"_ustr))
    , m_xEdRange(m_xBuilder->weld_entry(u"rangeedit"_ustr))
    , m_xBtnRange(m_xBuilder->weld_button(u"dotdotdot"_ustr))
    , m_xCbAction(m_xBuilder->weld_check_button(u"action"_ustr))
    , m_xLbAction(m_xBuilder->weld_combo_box(u"actionlist"_ustr))
    , m_xCbComment(m_xBuilder->weld_check_button(u"comment"_ustr))
    , m_xEdComment(m_xBuilder->weld_entry(u"commentedit"_ustr))
{
    m_xTfDateFormatter->EnableEmptyField(false);
    m_xTfDate2Formatter->EnableEmptyField(false);

    m_xLbDate->set_active(0);
    m_xLbDate->connect_changed( LINK( this, SvxTPFilter, SelDateHdl ) );
    m_xIbClock->connect_clicked( LINK( this, SvxTPFilter, TimeHdl) );
    m_xIbClock2->connect_clicked( LINK( this, SvxTPFilter,TimeHdl) );
    m_xBtnRange->connect_clicked( LINK( this, SvxTPFilter, RefHandle));

    Link<weld::Toggleable&,void> aLink=LINK( this, SvxTPFilter, RowEnableHdl) ;
    m_xCbDate->connect_toggled(aLink);
    m_xCbAuthor->connect_toggled(aLink);
    m_xCbRange->connect_toggled(aLink);
    m_xCbAction->connect_toggled(aLink);
    m_xCbComment->connect_toggled(aLink);

    Link<SvtCalendarBox&,void> a2Link=LINK(this, SvxTPFilter, ModifyDate);
    m_xDfDate->connect_activated(a2Link);
    m_xDfDate2->connect_activated(a2Link);

    Link<weld::FormattedSpinButton&,void> a3Link=LINK(this, SvxTPFilter, ModifyTime);
    m_xTfDate->connect_value_changed(a3Link);
    m_xTfDate2->connect_value_changed(a3Link);

    Link<weld::Entry&,void> a4Link=LINK( this, SvxTPFilter, ModifyHdl);
    m_xEdRange->connect_changed(a4Link);
    m_xEdComment->connect_changed(a4Link);
    m_xLbAction->connect_changed(LINK( this, SvxTPFilter, ModifyListBoxHdl));
    m_xLbAuthor->connect_changed(LINK( this, SvxTPFilter, ModifyListBoxHdl));

    RowEnableHdl(*m_xCbDate);
    RowEnableHdl(*m_xCbAuthor);
    RowEnableHdl(*m_xCbRange);
    RowEnableHdl(*m_xCbAction);
    RowEnableHdl(*m_xCbComment);

    DateTime aDateTime(DateTime::SYSTEM);
    SetFirstDate(aDateTime);
    SetLastDate(aDateTime);
    SetFirstTime(aDateTime);
    SetLastTime(aDateTime);
    HideRange();
    ShowAction();
    bModified=false;
}

SvxTPFilter::~SvxTPFilter()
{
}

void SvxTPFilter::SetRedlinTable(SvxRedlinTable* pTable)
{
    m_pRedlinTable = pTable;
}

void SvxTPFilter::EnableDateLine1(bool bFlag)
{
    if(bFlag && m_xCbDate->get_active())
    {
        m_xDfDate->set_sensitive(true);
        m_xTfDate->set_sensitive(true);
        m_xIbClock->set_sensitive(true);
    }
    else
    {
        m_xDfDate->set_sensitive(false);
        m_xTfDate->set_sensitive(false);
        m_xIbClock->set_sensitive(false);
    }
}
void SvxTPFilter::EnableDateLine2(bool bFlag)
{
    if(bFlag && m_xCbDate->get_active())
    {
        m_xFtDate2->set_sensitive(true);
        m_xDfDate2->set_sensitive(true);
        m_xTfDate2->set_sensitive(true);
        m_xIbClock2->set_sensitive(true);
    }
    else
    {
        m_xFtDate2->set_sensitive(false);
        m_xDfDate2->set_sensitive(false);
        m_xDfDate2->set_label(OUString());
        m_xTfDate2->set_sensitive(false);
        m_xTfDate2->set_text(OUString());
        m_xIbClock2->set_sensitive(false);
    }
}

Date SvxTPFilter::GetFirstDate() const
{
    return m_xDfDate->get_date();
}

void SvxTPFilter::SetFirstDate(const Date &aDate)
{
    m_xDfDate->set_date(aDate);
}

tools::Time SvxTPFilter::GetFirstTime() const
{
    return m_xTfDateFormatter->GetTime();
}

void SvxTPFilter::SetFirstTime(const tools::Time &aTime)
{
    m_xTfDateFormatter->SetTime(aTime);
}

Date SvxTPFilter::GetLastDate() const
{
    return m_xDfDate2->get_date();
}

void SvxTPFilter::SetLastDate(const Date &aDate)
{
    m_xDfDate2->set_date(aDate);
}

tools::Time SvxTPFilter::GetLastTime() const
{
    return m_xTfDate2Formatter->GetTime();
}

void SvxTPFilter::SetLastTime(const tools::Time &aTime)
{
    m_xTfDate2Formatter->SetTime(aTime);
}

void SvxTPFilter::SetDateMode(sal_uInt16 nMode)
{
    m_xLbDate->set_active(nMode);
    SelDateHdl(*m_xLbDate);
}

SvxRedlinDateMode SvxTPFilter::GetDateMode() const
{
    return static_cast<SvxRedlinDateMode>(m_xLbDate->get_active());
}
void SvxTPFilter::ClearAuthors()
{
    m_xLbAuthor->clear();
}

void SvxTPFilter::InsertAuthor( const OUString& rString)
{
    m_xLbAuthor->append_text(rString);
}

OUString SvxTPFilter::GetSelectedAuthor() const
{
    return m_xLbAuthor->get_active_text();
}

void SvxTPFilter::SelectedAuthorPos(sal_Int32 nPos)
{
    m_xLbAuthor->set_active(nPos);
}

sal_Int32 SvxTPFilter::SelectAuthor(const OUString& aString)
{
    m_xLbAuthor->set_active_text(aString);
    return m_xLbAuthor->get_active();
}

void SvxTPFilter::SetRange(const OUString& rString)
{
    m_xEdRange->set_text(rString);
}

OUString SvxTPFilter::GetRange() const
{
    return m_xEdRange->get_text();
}

void SvxTPFilter::SetFocusToRange()
{
    m_xEdRange->grab_focus();
}

void SvxTPFilter::HideRange(bool bHide)
{
    if (bHide)
    {
        m_xCbRange->hide();
        m_xEdRange->hide();
        m_xBtnRange->hide();
    }
    else
    {
        ShowAction(false);
        m_xCbRange->show();
        m_xEdRange->show();
        m_xBtnRange->show();
    }
}

void SvxTPFilter::SetComment(const OUString &rComment)
{
    m_xEdComment->set_text(rComment);
}

OUString SvxTPFilter::GetComment()const
{
    return m_xEdComment->get_text();
}

bool SvxTPFilter::IsDate() const
{
    return m_xCbDate->get_active();
}

bool SvxTPFilter::IsAuthor() const
{
    return m_xCbAuthor->get_active();
}

bool SvxTPFilter::IsRange() const
{
    return m_xCbRange->get_active();
}

bool SvxTPFilter::IsAction() const
{
    return m_xCbAction->get_active();
}

bool SvxTPFilter::IsComment() const
{
    return m_xCbComment->get_active();
}

void SvxTPFilter::CheckDate(bool bFlag)
{
    m_xCbDate->set_active(bFlag);
    RowEnableHdl(*m_xCbDate);
    bModified=false;
}

void SvxTPFilter::CheckAuthor(bool bFlag)
{
    m_xCbAuthor->set_active(bFlag);
    RowEnableHdl(*m_xCbAuthor);
    bModified=false;
}

void SvxTPFilter::CheckRange(bool bFlag)
{
    m_xCbRange->set_active(bFlag);
    RowEnableHdl(*m_xCbRange);
    bModified=false;
}

void SvxTPFilter::CheckAction(bool bFlag)
{
    m_xCbAction->set_active(bFlag);
    RowEnableHdl(*m_xCbAction);
    bModified=false;
}

void SvxTPFilter::CheckComment(bool bFlag)
{
    m_xCbComment->set_active(bFlag);
    RowEnableHdl(*m_xCbComment);
    bModified=false;
}

void SvxTPFilter::ShowAction(bool bShow)
{
    if(!bShow)
    {
        m_xCbAction->hide();
        m_xLbAction->hide();
    }
    else
    {
        HideRange();
        m_xCbAction->show();
        m_xLbAction->show();
    }
}

IMPL_LINK_NOARG(SvxTPFilter, SelDateHdl, weld::ComboBox&, void)
{
    SvxRedlinDateMode nKind = static_cast<SvxRedlinDateMode>(m_xLbDate->get_active());
    switch(nKind)
    {
        case SvxRedlinDateMode::BEFORE:
        case SvxRedlinDateMode::SINCE:
            EnableDateLine1(true);
            EnableDateLine2(false);
            break;
        case SvxRedlinDateMode::EQUAL:
        case SvxRedlinDateMode::NOTEQUAL:
            EnableDateLine1(true);
            m_xTfDate->set_sensitive(false);
            m_xTfDate->set_text(OUString());
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
    bModified = true;
}

IMPL_LINK(SvxTPFilter, RowEnableHdl, weld::Toggleable&, rCB, void)
{
    if (&rCB == m_xCbDate.get())
    {
        m_xLbDate->set_sensitive(m_xCbDate->get_active());
        EnableDateLine1(false);
        EnableDateLine2(false);
        if(m_xCbDate->get_active()) SelDateHdl(*m_xLbDate);
    }
    else if (&rCB == m_xCbAuthor.get())
    {
        m_xLbAuthor->set_sensitive(m_xCbAuthor->get_active());
    }
    else if (&rCB == m_xCbRange.get())
    {
        m_xEdRange->set_sensitive(m_xCbRange->get_active());
        m_xBtnRange->set_sensitive(m_xCbRange->get_active());
    }
    else if (&rCB == m_xCbAction.get())
    {
        m_xLbAction->set_sensitive(m_xCbAction->get_active());
    }
    else if (&rCB == m_xCbComment.get())
    {
        m_xEdComment->set_sensitive(m_xCbComment->get_active());
    }
    bModified = true;
}

IMPL_LINK(SvxTPFilter, TimeHdl, weld::Button&, rIB, void)
{
    DateTime aDateTime( DateTime::SYSTEM );
    if (&rIB == m_xIbClock.get())
    {
        SetFirstDate(aDateTime);
        SetFirstTime(aDateTime);
    }
    else if (&rIB == m_xIbClock2.get())
    {
        SetLastDate(aDateTime);
        SetLastTime(aDateTime);
    }
    bModified=true;
}

IMPL_LINK_NOARG(SvxTPFilter, ModifyHdl, weld::Entry&, void)
{
    bModified=true;
}

IMPL_LINK_NOARG(SvxTPFilter, ModifyListBoxHdl, weld::ComboBox&, void)
{
    bModified=true;
}

void SvxTPFilter::DeactivatePage()
{
    if(bModified)
    {
        if (m_pRedlinTable)
        {
            m_pRedlinTable->SetFilterDate(IsDate());
            m_pRedlinTable->SetDateTimeMode(GetDateMode());
            m_pRedlinTable->SetFirstDate(GetFirstDate());
            m_pRedlinTable->SetLastDate(GetLastDate());
            m_pRedlinTable->SetFirstTime(GetFirstTime());
            m_pRedlinTable->SetLastTime(GetLastTime());
            m_pRedlinTable->SetFilterAuthor(IsAuthor());
            m_pRedlinTable->SetAuthor(GetSelectedAuthor());

            m_pRedlinTable->SetFilterComment(IsComment());

            utl::SearchParam aSearchParam( m_xEdComment->get_text(),
                    utl::SearchParam::SearchType::Regexp,false );

            m_pRedlinTable->SetCommentParams(&aSearchParam);

            m_pRedlinTable->UpdateFilterTest();
        }

        aReadyLink.Call(this);
    }
    bModified=false;
}

void SvxTPFilter::Enable(bool bEnable)
{
    m_xContainer->set_sensitive(bEnable);
    if (m_xCbDate->get_sensitive())
    {
        RowEnableHdl(*m_xCbDate);
        RowEnableHdl(*m_xCbAuthor);
        RowEnableHdl(*m_xCbRange);
        RowEnableHdl(*m_xCbComment);
    }
}

IMPL_LINK(SvxTPFilter, ModifyDate, SvtCalendarBox&, rTF, void)
{
    Date aDate( Date::SYSTEM );
    if (m_xDfDate.get() == &rTF)
    {
        if (m_xDfDate->get_label().isEmpty())
           m_xDfDate->set_date(aDate);

        if(m_pRedlinTable!=nullptr)
            m_pRedlinTable->SetFirstDate(m_xDfDate->get_date());
    }
    else if (m_xDfDate2.get() == &rTF)
    {
        if (m_xDfDate2->get_label().isEmpty())
           m_xDfDate2->set_date(aDate);

        if (m_pRedlinTable)
            m_pRedlinTable->SetLastDate(m_xDfDate2->get_date());
    }
    bModified=true;
}

IMPL_LINK(SvxTPFilter, ModifyTime, weld::FormattedSpinButton&, rTF, void)
{
    tools::Time aTime(tools::Time::EMPTY);
    if (m_xTfDate.get() == &rTF)
    {
        if (m_xTfDate->get_text().isEmpty())
            SetFirstTime(aTime);

        if (m_pRedlinTable!=nullptr)
            m_pRedlinTable->SetFirstTime(GetFirstTime());
    }
    else if (m_xTfDate2.get() == &rTF)
    {
        if (m_xTfDate2->get_text().isEmpty())
            SetLastTime(aTime);

        if (m_pRedlinTable!=nullptr)
            m_pRedlinTable->SetLastTime(GetLastTime());

    }
    bModified=true;
}

IMPL_LINK_NOARG(SvxTPFilter, RefHandle, weld::Button&, void)
{
    aRefLink.Call(this);
}

SvxAcceptChgCtr::SvxAcceptChgCtr(weld::Container* pParent)
    : m_xBuilder(Application::CreateBuilder(pParent, u"svx/ui/redlinecontrol.ui"_ustr))
    , m_xTabCtrl(m_xBuilder->weld_notebook(u"tabcontrol"_ustr))
{
    m_xTabCtrl->connect_enter_page(LINK(this, SvxAcceptChgCtr, ActivatePageHdl));
    m_xTabCtrl->connect_leave_page(LINK(this, SvxAcceptChgCtr, DeactivatePageHdl));

    m_xTPFilter.reset(new SvxTPFilter(m_xTabCtrl->get_page(u"filter"_ustr)));
    m_xTPView.reset(new SvxTPView(m_xTabCtrl->get_page(u"view"_ustr)));
    m_xTPFilter->SetRedlinTable(m_xTPView->GetTableControl());
    m_xTabCtrl->set_current_page(u"view"_ustr);
    m_xTabCtrl->set_help_id(HID_REDLINE_CTRL_VIEW);
    m_xTabCtrl->show();
}

SvxAcceptChgCtr::~SvxAcceptChgCtr()
{
    m_xTPFilter.reset();
    m_xTPView.reset();
}

void SvxAcceptChgCtr::ShowFilterPage()
{
    m_xTabCtrl->set_current_page(u"filter"_ustr);
}

IMPL_LINK(SvxAcceptChgCtr, ActivatePageHdl, const OUString&, rPage, void)
{
    if (rPage == "filter")
    {
        m_xTPFilter->ActivatePage();
        m_xTabCtrl->set_help_id(HID_REDLINE_CTRL_FILTER);
    }
    else if (rPage == "view")
    {
        m_xTPView->ActivatePage();
        m_xTabCtrl->set_help_id(HID_REDLINE_CTRL_VIEW);
    }
}

IMPL_LINK(SvxAcceptChgCtr, DeactivatePageHdl, const OUString&, rPage, bool)
{
    if (rPage == "filter")
        m_xTPFilter->DeactivatePage();
    else if (rPage == "view")
        m_xTPView->DeactivatePage();
    return true;
}

std::ostream& operator<<(std::ostream& rStream, const RedlineType& eType)
{
    switch (eType)
    {
        case RedlineType::Insert:
            return rStream << "RedlineType::Insert";
        case RedlineType::Delete:
            return rStream << "RedlineType::Delete";
        case RedlineType::Format:
            return rStream << "RedlineType::Format";
        case RedlineType::Table:
            return rStream << "RedlineType::Table";
        case RedlineType::FmtColl:
            return rStream << "RedlineType::FmtColl";
        case RedlineType::ParagraphFormat:
            return rStream << "RedlineType::ParagraphFormat";
        case RedlineType::TableRowInsert:
            return rStream << "RedlineType::TableRowInsert";
        case RedlineType::TableRowDelete:
            return rStream << "RedlineType::TableRowDelete";
        case RedlineType::TableCellInsert:
            return rStream << "RedlineType::TableCellInsert";
        case RedlineType::TableCellDelete:
            return rStream << "RedlineType::TableCellDelete";
        case RedlineType::None:
            return rStream << "RedlineType::None";
        case RedlineType::Any:
            return rStream << "RedlineType::Any";
        default:
            O3TL_UNREACHABLE;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
