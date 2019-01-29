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
#include <svtools/simptabl.hxx>
#include <vcl/svlbitm.hxx>
#include <vcl/svtabbx.hxx>
#include <vcl/treelistbox.hxx>
#include <vcl/treelistentry.hxx>
#include <tools/color.hxx>
#include <tools/contnr.hxx>
#include <tools/date.hxx>
#include <tools/datetime.hxx>
#include <tools/link.hxx>
#include <tools/time.hxx>
#include <tools/wintypes.hxx>
#include <vcl/builder.hxx>
#include <vcl/image.hxx>
#include <vcl/outdev.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/tabctrl.hxx>
#include <memory>

namespace utl {
    class SearchParam;
    class TextSearch;
}

namespace vcl { class Window; }

class Button;
class CheckBox;
class DateField;
class Edit;
class FixedText;
class ListBox;
class Point;
class PushButton;
class SvViewDataEntry;
class TimeField;
struct SvSortData;

enum class SvxRedlinDateMode
{
    BEFORE, SINCE, EQUAL, NOTEQUAL, BETWEEN, SAVE, NONE
};

/// Struct for sorting data.
class SAL_WARN_UNUSED SVX_DLLPUBLIC RedlinData
{
public:
                    RedlinData();
    virtual         ~RedlinData();
    bool            bDisabled;
    DateTime        aDateTime;
    void*           pData;
};

/// Entries for list.
class SAL_WARN_UNUSED SvxRedlinEntry : public SvTreeListEntry
{
public:
                    SvxRedlinEntry();
        virtual     ~SvxRedlinEntry() override;
};

/// Class for the representation of Strings depending on the font.
class SAL_WARN_UNUSED SvLBoxColorString : public SvLBoxString
{
private:

    Color           aPrivColor;

public:
                    SvLBoxColorString( const OUString& rStr, const Color& rCol);
                    SvLBoxColorString();
                    virtual ~SvLBoxColorString() override;

    /** Paint function of the SvLBoxColorString class.

        The relevant text with the selected color is drawn in the output device.
    */
    virtual void Paint(const Point& rPos, SvTreeListBox& rOutDev, vcl::RenderContext& rRenderContext,
                       const SvViewDataEntry* pView, const SvTreeListEntry& rEntry) override;

    virtual std::unique_ptr<SvLBoxItem> Clone(SvLBoxItem const * pSource) const override;
};

class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxRedlinTable : public SvSimpleTable
{
    using SvTabListBox::InsertEntry;

private:

    sal_uInt16      nDatePos;
    bool            bAuthor;
    bool            bDate;
    bool            bComment;
    SvxRedlinDateMode nDaTiMode;
    DateTime        aDaTiFirst;
    DateTime        aDaTiLast;
    DateTime        aDaTiFilterFirst;
    DateTime        aDaTiFilterLast;
    OUString        aAuthor;
    Color           maEntryColor;
    Image           maEntryImage;
    OUString        maEntryString;
    std::unique_ptr<utl::TextSearch> pCommentSearcher;
    Link<const SvSortData*,sal_Int32>  aColCompareLink;

protected:

    virtual sal_Int32       ColCompare(SvTreeListEntry*,SvTreeListEntry*) override;
    virtual void            InitEntry(SvTreeListEntry*, const OUString&, const Image&, const Image&, SvLBoxButtonKind) override;

public:

    SvxRedlinTable(SvSimpleTableContainer& rParent, WinBits nBits = WB_BORDER);
    virtual ~SvxRedlinTable() override;
    virtual void    dispose() override;

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

    bool            IsValidEntry(const OUString &rAuthor, const DateTime &rDateTime, const OUString &rComment);
    bool            IsValidEntry(const OUString &rAuthor, const DateTime &rDateTime);
    bool            IsValidComment(const OUString &rComment);

    /** Insert a redline entry.

        The rStr contains the entire redline entry; the columns are delimited by '\t'.
    */
    SvTreeListEntry* InsertEntry(const OUString &rStr, std::unique_ptr<RedlinData> pUserData,
                                 SvTreeListEntry* pParent = nullptr, sal_uLong nPos = TREELIST_APPEND);

    /** Insert a redline entry.

        The rStr contains the entire redline entry; the columns are delimited by '\t'.
    */
    SvTreeListEntry* InsertEntry(const OUString &rStr, std::unique_ptr<RedlinData> pUserData, const Color&,
                                 SvTreeListEntry* pParent, sal_uLong nPos = TREELIST_APPEND);

    /** Insert a redline entry.

        rRedlineType contains the image for this redline entry (plus for insertion, minus for deletion etc.).
        rStr contains the rest of the redline entry; the columns are delimited by '\t'.
    */
    SvTreeListEntry* InsertEntry(const Image &rRedlineType, const OUString &rStr, std::unique_ptr<RedlinData> pUserData,
                                 SvTreeListEntry* pParent, sal_uLong nPos = TREELIST_APPEND);

    virtual SvTreeListEntry* CreateEntry() const override;

    void            SetColCompareHdl(const Link<const SvSortData*,sal_Int32>& rLink ) { aColCompareLink = rLink; }
};

/// Tabpage with the filter text entries etc.
class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxTPFilter final : public TabPage
{
    Link<SvxTPFilter*,void>  aReadyLink;
    Link<SvxTPFilter*,void>  aRefLink;

    VclPtr<SvxRedlinTable> pRedlinTable;
    VclPtr<CheckBox>       m_pCbDate;
    VclPtr<ListBox>        m_pLbDate;
    VclPtr<DateField>      m_pDfDate;
    VclPtr<TimeField>      m_pTfDate;
    VclPtr<PushButton>     m_pIbClock;
    VclPtr<FixedText>      m_pFtDate2;
    VclPtr<DateField>      m_pDfDate2;
    VclPtr<TimeField>      m_pTfDate2;
    VclPtr<PushButton>     m_pIbClock2;
    VclPtr<CheckBox>       m_pCbAuthor;
    VclPtr<ListBox>        m_pLbAuthor;
    VclPtr<CheckBox>       m_pCbRange;
    VclPtr<Edit>           m_pEdRange;
    VclPtr<PushButton>     m_pBtnRange;
    VclPtr<CheckBox>       m_pCbAction;
    VclPtr<ListBox>        m_pLbAction;
    VclPtr<CheckBox>       m_pCbComment;
    VclPtr<Edit>           m_pEdComment;
    bool                   bModified;

    DECL_LINK( SelDateHdl, ListBox&, void );
    DECL_LINK( RowEnableHdl, Button*, void );
    DECL_LINK( TimeHdl, Button*, void );
    DECL_LINK( ModifyHdl, Edit&, void );
    DECL_LINK( ModifyListBoxHdl, ListBox&, void );
    DECL_LINK( ModifyDate, Edit&, void );
    DECL_LINK( RefHandle, Button*, void );

    void            EnableDateLine1(bool bFlag);
    void            EnableDateLine2(bool bFlag);

public:
                    SvxTPFilter( vcl::Window * pParent);
    virtual         ~SvxTPFilter() override;
    virtual void    dispose() override;

    virtual void    DeactivatePage() override;
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
    SvxRedlinDateMode GetDateMode();

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

    bool            IsDate();
    bool            IsAuthor();
    bool            IsRange();
    bool            IsAction();
    bool            IsComment();

    void            ShowAction(bool bShow=true);

    void            CheckDate(bool bFlag);
    void            CheckAuthor(bool bFlag);
    void            CheckRange(bool bFlag);
    void            CheckAction(bool bFlag);
    void            CheckComment(bool bFlag);

    ListBox*        GetLbAction() { return m_pLbAction;}

    void            SetReadyHdl( const Link<SvxTPFilter*,void>& rLink ) { aReadyLink= rLink; }


    // Methods for Calc {
    void            SetRefHdl( const Link<SvxTPFilter*,void>& rLink ) { aRefLink = rLink; }

    void            Enable( bool bEnable = true );
    void            Disable();
    // } Methods for Calc
};

/// Tabpage with the redlining entries.
class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxTPView : public TabPage
{
private:

    Link<SvxTPView*,void>          AcceptClickLk;
    Link<SvxTPView*,void>          AcceptAllClickLk;
    Link<SvxTPView*,void>          RejectClickLk;
    Link<SvxTPView*,void>          RejectAllClickLk;
    Link<SvxTPView*,void>          UndoClickLk;

    VclPtr<SvxRedlinTable> m_pViewData;
    VclPtr<PushButton>     m_pAccept;
    VclPtr<PushButton>     m_pReject;
    VclPtr<PushButton>     m_pAcceptAll;
    VclPtr<PushButton>     m_pRejectAll;
    VclPtr<PushButton>     m_pUndo;

    bool bEnableAccept;
    bool bEnableAcceptAll;
    bool bEnableReject;
    bool bEnableRejectAll;
    bool bEnableUndo;

    DECL_LINK( PbClickHdl, Button*, void );

public:
    SvxTPView(vcl::Window * pParent, VclBuilderContainer *pTopLevel);
    virtual ~SvxTPView() override;
    virtual void    dispose() override;

    void            InsertWriterHeader();
    void            InsertCalcHeader();
    SvxRedlinTable* GetTableControl() { return m_pViewData;}

    void            EnableAccept(bool bFlag);
    void            EnableAcceptAll(bool bFlag);
    void            EnableReject(bool bFlag);
    void            EnableRejectAll(bool bFlag);
    static void     EnableClearFormatButton(VclPtr<PushButton>, bool bFlag);
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
    virtual void    DeactivatePage() override;
};


//  Redlining - Control (Accept- Changes)
class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxAcceptChgCtr
    : public TabControl
    , public VclBuilderContainer
{
private:

    VclPtr<SvxTPFilter>    pTPFilter;
    VclPtr<SvxTPView>      pTPView;

    sal_uInt16      m_nFilterPageId;

public:
                    SvxAcceptChgCtr(vcl::Window* pParent, VclBuilderContainer* pTopLevel);

                    virtual ~SvxAcceptChgCtr() override;
    virtual void    dispose() override;

    void            ShowFilterPage();

    SvxTPFilter*    GetFilterPage() { return pTPFilter;}
    SvxTPView*      GetViewPage() { return pTPView;}
};

#endif // INCLUDED_SVX_CTREDLIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
