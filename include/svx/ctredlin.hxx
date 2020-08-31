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
#include <vcl/weld.hxx>
#include <memory>

namespace utl {
    class SearchParam;
    class TextSearch;
}

namespace comphelper::string { class NaturalStringSorter; }

class Point;
class SvViewDataEntry;
class SvtCalendarBox;

enum class SvxRedlinDateMode
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
    Any = USHRT_MAX // special value to indicate any redline type in some method calls
};

/// Struct for sorting data.
class SAL_WARN_UNUSED SVX_DLLPUBLIC RedlinData
{
public:
                    RedlinData();
    virtual         ~RedlinData();
    DateTime        aDateTime;
    void*           pData;
    RedlineType     eType;
    bool            bDisabled;
};

class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxRedlinTable
{
private:
    std::unique_ptr<comphelper::string::NaturalStringSorter> xSorter;
    std::unique_ptr<weld::TreeView> xWriterTreeView;
    std::unique_ptr<weld::TreeView> xCalcTreeView;
    weld::TreeView* pTreeView;

    sal_uInt16      nDatePos;
    bool            bAuthor;
    bool            bDate;
    bool            bComment;
    bool            bSorted;
    SvxRedlinDateMode nDaTiMode;
    DateTime        aDaTiFirst;
    DateTime        aDaTiLast;
    DateTime        aDaTiFilterFirst;
    DateTime        aDaTiFilterLast;
    OUString        aAuthor;
    std::unique_ptr<utl::TextSearch> pCommentSearcher;

    int ColCompare(const weld::TreeIter& rLeft, const weld::TreeIter& rRight);

public:
    SvxRedlinTable(std::unique_ptr<weld::TreeView> xWriterControl,
                   std::unique_ptr<weld::TreeView> xCalcControl);

    void set_size_request(int nWidth, int nHeight);

    weld::TreeView& GetWidget() { return *pTreeView; }
    bool IsSorted() const { return bSorted; }

    ~SvxRedlinTable();

    // For FilterPage only {
    void            SetFilterDate(bool bFlag);
    void            SetDateTimeMode(SvxRedlinDateMode nMode);
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

    bool            IsValidEntry(const OUString &rAuthor, const DateTime &rDateTime, const OUString &rComment);
    bool            IsValidEntry(const OUString &rAuthor, const DateTime &rDateTime);
    bool            IsValidComment(const OUString &rComment);

    DECL_LINK(HeaderBarClick, int, void);
};

class SVX_DLLPUBLIC SvxTPage
{
protected:
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Container> m_xContainer;
public:
    SvxTPage(weld::Container* pParent, const OUString& rUIXMLDescription, const OString& rID);
    virtual ~SvxTPage();
    virtual void ActivatePage();
    void Show() { m_xContainer->show(); }
};

/// Tabpage with the filter text entries etc.
class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxTPFilter final : public SvxTPage
{
    Link<SvxTPFilter*,void>  aReadyLink;
    Link<SvxTPFilter*,void>  aRefLink;

    bool                   bModified;

    SvxRedlinTable* m_pRedlinTable;
    std::unique_ptr<weld::CheckButton> m_xCbDate;
    std::unique_ptr<weld::ComboBox> m_xLbDate;
    std::unique_ptr<SvtCalendarBox> m_xDfDate;
    std::unique_ptr<weld::TimeSpinButton> m_xTfDate;
    std::unique_ptr<weld::Button> m_xIbClock;
    std::unique_ptr<weld::Label> m_xFtDate2;
    std::unique_ptr<SvtCalendarBox> m_xDfDate2;
    std::unique_ptr<weld::TimeSpinButton> m_xTfDate2;
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

    DECL_LINK( SelDateHdl, weld::ComboBox&, void );
    DECL_LINK( RowEnableHdl, weld::Button&, void );
    DECL_LINK( TimeHdl, weld::Button&, void );
    DECL_LINK( ModifyHdl, weld::Entry&, void );
    DECL_LINK( ModifyListBoxHdl, weld::ComboBox&, void );
    DECL_LINK( ModifyDate, SvtCalendarBox&, void );
    DECL_LINK( ModifyTime, weld::TimeSpinButton&, void );
    DECL_LINK( RefHandle, weld::Button&, void );

    void            EnableDateLine1(bool bFlag);
    void            EnableDateLine2(bool bFlag);

public:
    SvxTPFilter(weld::Container* pParent);
    virtual ~SvxTPFilter() override;

    void            DeactivatePage();
    void            SetRedlinTable(SvxRedlinTable*);

    Date            GetFirstDate() const;
    void            SetFirstDate(const Date &aDate);
    tools::Time     GetFirstTime() const;
    void            SetFirstTime(const tools::Time &aTime);

    Date            GetLastDate() const;
    void            SetLastDate(const Date &aDate);
    tools::Time     GetLastTime() const;
    void            SetLastTime(const tools::Time &aTime);

    void            SetDateMode(sal_uInt16 nMode);
    SvxRedlinDateMode GetDateMode() const;

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

    void            SetReadyHdl( const Link<SvxTPFilter*,void>& rLink ) { aReadyLink= rLink; }


    // Methods for Calc {
    void            SetRefHdl( const Link<SvxTPFilter*,void>& rLink ) { aRefLink = rLink; }

    void            Enable( bool bEnable = true );
    // } Methods for Calc
};

/// Tabpage with the redlining entries.
class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxTPView final : public SvxTPage
{
private:

    Link<SvxTPView*,void>          AcceptClickLk;
    Link<SvxTPView*,void>          AcceptAllClickLk;
    Link<SvxTPView*,void>          RejectClickLk;
    Link<SvxTPView*,void>          RejectAllClickLk;
    Link<SvxTPView*,void>          UndoClickLk;

    bool bEnableAccept;
    bool bEnableAcceptAll;
    bool bEnableReject;
    bool bEnableRejectAll;
    bool bEnableUndo;

    bool bEnableClearFormat;
    bool bEnableClearFormatAll;

    weld::Window* m_pDialog;
    std::unique_ptr<weld::Button> m_xAccept;
    std::unique_ptr<weld::Button> m_xReject;
    std::unique_ptr<weld::Button> m_xAcceptAll;
    std::unique_ptr<weld::Button> m_xRejectAll;
    std::unique_ptr<weld::Button> m_xUndo;
    std::unique_ptr<SvxRedlinTable> m_xViewData;

    DECL_LINK( PbClickHdl, weld::Button&, void );

    void            EnableClearFormatButton(weld::Button&, bool bFlag);
public:
    SvxTPView(weld::Container* pParent, weld::Window* pDialog, weld::Builder* pTopLevel);
    virtual ~SvxTPView() override;

    SvxRedlinTable* GetTableControl() { return m_xViewData.get(); }

    void            EnableAccept(bool bFlag);
    void            EnableAcceptAll(bool bFlag);
    void            EnableReject(bool bFlag);
    void            EnableRejectAll(bool bFlag);
    void            EnableClearFormat(bool bFlag);
    void            EnableClearFormatAll(bool bFlag);
    void            EnableUndo(bool bFlag=true);
    void            DisableUndo()       {EnableUndo(false);}
    void            ShowUndo();

    void            SetAcceptClickHdl( const Link<SvxTPView*,void>& rLink ) { AcceptClickLk = rLink; }

    void            SetAcceptAllClickHdl( const Link<SvxTPView*,void>& rLink ) { AcceptAllClickLk = rLink; }

    void            SetRejectClickHdl( const Link<SvxTPView*,void>& rLink ) { RejectClickLk = rLink; }

    void            SetRejectAllClickHdl( const Link<SvxTPView*,void>& rLink ) { RejectAllClickLk = rLink; }

    void            SetUndoClickHdl( const Link<SvxTPView*,void>& rLink ) { UndoClickLk = rLink; }

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

    DECL_DLLPRIVATE_LINK(ActivatePageHdl, const OString&, void);
    DECL_DLLPRIVATE_LINK(DeactivatePageHdl, const OString&, bool);

public:
    SvxAcceptChgCtr(weld::Container* pParent, weld::Window* pDialog, weld::Builder* pTopLevel);
    ~SvxAcceptChgCtr();

    void            ShowFilterPage();

    SvxTPFilter*    GetFilterPage() { return m_xTPFilter.get(); }
    SvxTPView*      GetViewPage() { return m_xTPView.get(); }

    void set_help_id(const OString& rId) { m_xTabCtrl->set_help_id(rId); }
};

#endif // INCLUDED_SVX_CTREDLIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
