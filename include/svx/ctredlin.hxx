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

#ifndef INCLUDED_SVX_CTREDLIN_HXX
#define INCLUDED_SVX_CTREDLIN_HXX

#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <svx/svxdllapi.h>
#include <tools/date.hxx>
#include <tools/datetime.hxx>
#include <tools/link.hxx>
#include <tools/time.hxx>
#include <vcl/weld/Builder.hxx>
#include <vcl/weld/ComboBox.hxx>
#include <vcl/weld/Entry.hxx>
#include <vcl/weld/FormattedSpinButton.hxx>
#include <vcl/weld/Notebook.hxx>
#include <vcl/weld/TreeView.hxx>
#include <vcl/weld/weld.hxx>
#include <memory>

namespace utl {
    class SearchParam;
    class TextSearch;
}

namespace comphelper::string { class NaturalStringSorter; }

class SvtCalendarBox;

enum class SvxRedlineDateMode
{
    BEFORE, SINCE, EQUAL, NOTEQUAL, BETWEEN, SAVE, NONE
};

enum class RedlineType : sal_uInt16
{
    // Range of RedlineTypes is 0 to 127.
    Insert = 0x0,// Content has been inserted.
    Delete = 0x1,// Content has been deleted.
    Format = 0x2,// Attributes have been applied.
    Table = 0x3,// Table structure has been altered.
    FmtColl = 0x4,// Style has been altered (Autoformat!).
    ParagraphFormat = 0x5,// Paragraph attributes have been changed.
    TableRowInsert = 0x6,// Table row has been inserted.
    TableRowDelete = 0x7,// Table row has been deleted.
    TableCellInsert = 0x8,// Table cell has been inserted.
    TableCellDelete = 0x9,// Table cell has been deleted.
    None = USHRT_MAX - 1, // special value to indicate missing redlining in some return value
    Any = USHRT_MAX // special value to indicate any redline type in some method calls
};

SVX_DLLPUBLIC std::ostream& operator<<(std::ostream& rStream, const RedlineType& eType);

/// Struct for sorting data.
class SAL_WARN_UNUSED SVX_DLLPUBLIC RedlineData
{
public:
    RedlineData();
    virtual ~RedlineData();
    DateTime        aDateTime;
    void*           pData;
    RedlineType     eType;
    bool            bDisabled;
};

class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxRedlineTable
{
private:
    std::unique_ptr<comphelper::string::NaturalStringSorter> m_xSorter;
    std::unique_ptr<weld::TreeView> m_xWriterTreeView;
    std::unique_ptr<weld::TreeView> m_xCalcTreeView;
    weld::TreeView* m_pTreeView;

    weld::ComboBox* m_pSortByComboBox;

    sal_uInt16      m_nDatePos;
    bool            m_bAuthor;
    bool            m_bDate;
    bool            m_bComment;
    bool            m_bSorted;
    SvxRedlineDateMode m_nDaTiMode;
    DateTime        m_aDaTiFirst;
    DateTime        m_aDaTiLast;
    DateTime        m_aDaTiFilterFirst;
    DateTime        m_aDaTiFilterLast;
    OUString        m_aAuthor;
    std::unique_ptr<utl::TextSearch> m_pCommentSearcher;

    int ColCompare(const weld::TreeIter& rLeft, const weld::TreeIter& rRight);

public:
    SvxRedlineTable(std::unique_ptr<weld::TreeView> xWriterControl,
                    std::unique_ptr<weld::TreeView> xCalcControl, weld::ComboBox* pSortByControl);

    weld::TreeView& GetWidget() { return *m_pTreeView; }
    bool IsSorted() const { return m_bSorted; }

    ~SvxRedlineTable();

    // For FilterPage only {
    void            SetFilterDate(bool bFlag);
    void SetDateTimeMode(SvxRedlineDateMode nMode);
    void            SetFirstDate(const Date&);
    void            SetLastDate(const Date&);
    void            SetFirstTime(const tools::Time&);
    void            SetLastTime(const tools::Time&);
    void            SetFilterAuthor(bool bFlag);
    void            SetAuthor(const OUString &);
    void            SetFilterComment(bool bFlag);
    void            SetCommentParams( const utl::SearchParam* pSearchPara );

    void            UpdateFilterTest();
    // } For FilterPage only

    void            SetCalcView();
    void            SetWriterView();

    bool            IsValidEntry(std::u16string_view rAuthor, const DateTime &rDateTime, const OUString &rComment);
    bool            IsValidEntry(std::u16string_view rAuthor, const DateTime &rDateTime);
    bool            IsValidComment(const OUString &rComment);

    DECL_LINK(HeaderBarClick, int, void);
};

class SVX_DLLPUBLIC SvxTPage
{
protected:
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Container> m_xContainer;
public:
    SvxTPage(weld::Container* pParent, const OUString& rUIXMLDescription, const OUString& rID);
    virtual ~SvxTPage();
    virtual void ActivatePage();
    void Show() { m_xContainer->show(); }
};

namespace weld
{
    class TimeFormatter;
}

/// Tabpage with the filter text entries etc.
class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxTPFilter final : public SvxTPage
{
    Link<SvxTPFilter*,void>  m_aReadyLink;
    Link<SvxTPFilter*,void>  m_aRefLink;

    bool                   m_bModified;

    SvxRedlineTable* m_pRedlineTable;
    std::unique_ptr<weld::CheckButton> m_xCbDate;
    std::unique_ptr<weld::ComboBox> m_xLbDate;
    std::unique_ptr<SvtCalendarBox> m_xDfDate;
    std::unique_ptr<weld::FormattedSpinButton> m_xTfDate;
    std::unique_ptr<weld::TimeFormatter> m_xTfDateFormatter;
    std::unique_ptr<weld::Button> m_xIbClock;
    std::unique_ptr<weld::Label> m_xFtDate2;
    std::unique_ptr<SvtCalendarBox> m_xDfDate2;
    std::unique_ptr<weld::FormattedSpinButton> m_xTfDate2;
    std::unique_ptr<weld::TimeFormatter> m_xTfDate2Formatter;
    std::unique_ptr<weld::Button> m_xIbClock2;
    std::unique_ptr<weld::CheckButton> m_xCbAuthor;
    std::unique_ptr<weld::ComboBox> m_xLbAuthor;
    std::unique_ptr<weld::CheckButton> m_xCbRange;
    std::unique_ptr<weld::Entry> m_xEdRange;
    std::unique_ptr<weld::Button> m_xBtnRange;
    std::unique_ptr<weld::CheckButton> m_xCbAction;
    std::unique_ptr<weld::ComboBox> m_xLbAction;
    std::unique_ptr<weld::CheckButton> m_xCbComment;
    std::unique_ptr<weld::Entry> m_xEdComment;

    DECL_DLLPRIVATE_LINK( SelDateHdl, weld::ComboBox&, void );
    DECL_DLLPRIVATE_LINK( RowEnableHdl, weld::Toggleable&, void );
    DECL_DLLPRIVATE_LINK( TimeHdl, weld::Button&, void );
    DECL_DLLPRIVATE_LINK( ModifyHdl, weld::Entry&, void );
    DECL_DLLPRIVATE_LINK( ModifyListBoxHdl, weld::ComboBox&, void );
    DECL_DLLPRIVATE_LINK( ModifyDate, SvtCalendarBox&, void );
    DECL_DLLPRIVATE_LINK( ModifyTime, weld::FormattedSpinButton&, void );
    DECL_DLLPRIVATE_LINK( RefHandle, weld::Button&, void );

    void            EnableDateLine1(bool bFlag);
    void            EnableDateLine2(bool bFlag);

public:
    SvxTPFilter(weld::Container* pParent);
    virtual ~SvxTPFilter() override;

    void            DeactivatePage();
    void SetRedlineTable(SvxRedlineTable*);

    Date            GetFirstDate() const;
    void            SetFirstDate(const Date &aDate);
    tools::Time     GetFirstTime() const;
    void            SetFirstTime(const tools::Time &aTime);

    Date            GetLastDate() const;
    void            SetLastDate(const Date &aDate);
    tools::Time     GetLastTime() const;
    void            SetLastTime(const tools::Time &aTime);

    void            SetDateMode(sal_uInt16 nMode);
    SvxRedlineDateMode GetDateMode() const;

    void            ClearAuthors();
    void            InsertAuthor( const OUString& rString );
    OUString        GetSelectedAuthor()const;
    void            SelectedAuthorPos(sal_Int32  nPos);
    sal_Int32       SelectAuthor(const OUString& aString);
    void            SetComment(const OUString& rComment);
    OUString        GetComment()const;

    // Methods for Calc {
    void            SetRange(const OUString& rString);
    OUString        GetRange() const;
    void            HideRange(bool bHide=true);
    void            SetFocusToRange();
    // } Methods for Calc

    bool            IsDate() const;
    bool            IsAuthor() const;
    bool            IsRange() const;
    bool            IsAction() const;
    bool            IsComment() const;

    void            ShowAction(bool bShow=true);

    void            CheckDate(bool bFlag);
    void            CheckAuthor(bool bFlag);
    void            CheckRange(bool bFlag);
    void            CheckAction(bool bFlag);
    void            CheckComment(bool bFlag);

    weld::ComboBox* GetLbAction() { return m_xLbAction.get(); }

    void            SetReadyHdl( const Link<SvxTPFilter*,void>& rLink ) { m_aReadyLink= rLink; }


    // Methods for Calc {
    void            SetRefHdl( const Link<SvxTPFilter*,void>& rLink ) { m_aRefLink = rLink; }

    void            Enable( bool bEnable = true );
    // } Methods for Calc
};

/// Tabpage with the redlining entries.
class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxTPView final : public SvxTPage
{
private:
    Link<SvxTPView*,void>          m_AcceptClickLk;
    Link<SvxTPView*,void>          m_AcceptAllClickLk;
    Link<SvxTPView*,void>          m_RejectClickLk;
    Link<SvxTPView*,void>          m_RejectAllClickLk;
    Link<SvxTPView*,void>          m_UndoClickLk;

    Link<SvxTPView*,void> m_SortByComboBoxChangedLk;

    bool m_bEnableAccept;
    bool m_bEnableAcceptAll;
    bool m_bEnableReject;
    bool m_bEnableRejectAll;
    bool m_bEnableUndo;

    bool m_bEnableClearFormat;
    bool m_bEnableClearFormatAll;

    std::unique_ptr<weld::Button> m_xAccept;
    std::unique_ptr<weld::Button> m_xReject;
    std::unique_ptr<weld::Button> m_xAcceptAll;
    std::unique_ptr<weld::Button> m_xRejectAll;
    std::unique_ptr<weld::Button> m_xUndo;
    std::unique_ptr<weld::ComboBox> m_xSortByComboBox;
    std::unique_ptr<SvxRedlineTable> m_xViewData;

    DECL_DLLPRIVATE_LINK( PbClickHdl, weld::Button&, void );
    DECL_DLLPRIVATE_LINK(SortByComboBoxChangedHdl, weld::ComboBox&, void);

public:
    SvxTPView(weld::Container* pParent);
    virtual ~SvxTPView() override;

    SvxRedlineTable* GetTableControl() { return m_xViewData.get(); }

    weld::ComboBox* GetSortByComboBoxControl() { return m_xSortByComboBox.get(); }

    void            EnableAccept(bool bFlag);
    void            EnableAcceptAll(bool bFlag);
    void            EnableReject(bool bFlag);
    void            EnableRejectAll(bool bFlag);
    void            EnableClearFormat(bool bFlag);
    void            EnableClearFormatAll(bool bFlag);
    void            EnableUndo(bool bFlag=true);
    void            DisableUndo()       {EnableUndo(false);}
    void            ShowUndo();

    void            SetAcceptClickHdl( const Link<SvxTPView*,void>& rLink ) { m_AcceptClickLk = rLink; }

    void            SetAcceptAllClickHdl( const Link<SvxTPView*,void>& rLink ) { m_AcceptAllClickLk = rLink; }

    void            SetRejectClickHdl( const Link<SvxTPView*,void>& rLink ) { m_RejectClickLk = rLink; }

    void            SetRejectAllClickHdl( const Link<SvxTPView*,void>& rLink ) { m_RejectAllClickLk = rLink; }

    void            SetUndoClickHdl( const Link<SvxTPView*,void>& rLink ) { m_UndoClickLk = rLink; }

    void SetSortByComboBoxChangedHdl(const Link<SvxTPView*, void>& rLink)
    {
        m_SortByComboBoxChangedLk = rLink;
    }

    virtual void    ActivatePage() override;
    void            DeactivatePage();
};

//  Redlining - Control (Accept- Changes)
class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxAcceptChgCtr
{
private:
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Notebook> m_xTabCtrl;

    std::unique_ptr<SvxTPFilter> m_xTPFilter;
    std::unique_ptr<SvxTPView> m_xTPView;

    DECL_DLLPRIVATE_LINK(ActivatePageHdl, const OUString&, void);
    DECL_DLLPRIVATE_LINK(DeactivatePageHdl, const OUString&, bool);

public:
    SvxAcceptChgCtr(weld::Container* pParent);
    ~SvxAcceptChgCtr();

    void            ShowFilterPage();

    SvxTPFilter*    GetFilterPage() { return m_xTPFilter.get(); }
    SvxTPView& GetViewPage() { return *m_xTPView; }
};

#endif // INCLUDED_SVX_CTREDLIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
