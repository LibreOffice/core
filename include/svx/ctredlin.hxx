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

#include <svtools/headbar.hxx>
#include <svtools/simptabl.hxx>
#include <svtools/svlbitm.hxx>
#include <svtools/svtabbx.hxx>
#include <svtools/treelistentry.hxx>
#include <tools/datetime.hxx>
#include <vcl/button.hxx>
#include <vcl/combobox.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/tabctrl.hxx>
#include <unotools/textsearch.hxx>
#include <svx/svxdllapi.h>

enum class SvxRedlinDateMode
{
    BEFORE, SINCE, EQUAL, NOTEQUAL, BETWEEN, SAVE, NONE
};

/// Struct for sorting data.
class SVX_DLLPUBLIC SAL_WARN_UNUSED RedlinData
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
        virtual     ~SvxRedlinEntry();
};

/// Class for the representation of Strings depending on the font.
class SAL_WARN_UNUSED SvLBoxColorString : public SvLBoxString
{
private:

    Color           aPrivColor;

public:
                    SvLBoxColorString( SvTreeListEntry*, sal_uInt16 nFlags, const OUString& rStr,
                                    const Color& rCol);
                    SvLBoxColorString();
                    virtual ~SvLBoxColorString();

    /** Paint function of the SvLBoxColorString class.

        The relevant text with the selected color is drawn in the output device.
    */
    virtual void Paint(const Point& rPos, SvTreeListBox& rOutDev, vcl::RenderContext& rRenderContext,
                       const SvViewDataEntry* pView, const SvTreeListEntry* pEntry) SAL_OVERRIDE;

    SvLBoxItem*     Create() const SAL_OVERRIDE;
};

class SVX_DLLPUBLIC SAL_WARN_UNUSED SvxRedlinTable : public SvSimpleTable
{
    using SvTabListBox::InsertEntry;

private:

    bool            bIsCalc;
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
    utl::TextSearch* pCommentSearcher;
    Link<>          aColCompareLink;

protected:

    virtual sal_Int32       ColCompare(SvTreeListEntry*,SvTreeListEntry*) SAL_OVERRIDE;
    virtual void            InitEntry(SvTreeListEntry*, const OUString&, const Image&, const Image&, SvLBoxButtonKind) SAL_OVERRIDE;

public:

    SvxRedlinTable(SvSimpleTableContainer& rParent, WinBits nBits = WB_BORDER);
    virtual ~SvxRedlinTable();
    virtual void    dispose() SAL_OVERRIDE;

    // For FilterPage only {
    void            SetFilterDate(bool bFlag=true);
    void            SetDateTimeMode(SvxRedlinDateMode nMode);
    void            SetFirstDate(const Date&);
    void            SetLastDate(const Date&);
    void            SetFirstTime(const tools::Time&);
    void            SetLastTime(const tools::Time&);
    void            SetFilterAuthor(bool bFlag=true);
    void            SetAuthor(const OUString &);
    void            SetFilterComment(bool bFlag=true);
    void            SetCommentParams( const utl::SearchParam* pSearchPara );

    void            UpdateFilterTest();
    // } For FilterPage only

    void            SetCalcView(bool bFlag=true);

    bool            IsValidEntry(const OUString &rAuthor, const DateTime &rDateTime, const OUString &rComment);
    bool            IsValidEntry(const OUString &rAuthor, const DateTime &rDateTime);
    bool            IsValidComment(const OUString &rComment);

    /** Insert a redline entry.

        The rStr contains the entire redline entry; the columns are delimited by '\t'.
    */
    SvTreeListEntry* InsertEntry(const OUString &rStr, RedlinData *pUserData,
                                 SvTreeListEntry* pParent = NULL, sal_uIntPtr nPos = TREELIST_APPEND);

    /** Insert a redline entry.

        The rStr contains the entire redline entry; the columns are delimited by '\t'.
    */
    SvTreeListEntry* InsertEntry(const OUString &rStr, RedlinData *pUserData, const Color&,
                                 SvTreeListEntry* pParent = NULL, sal_uIntPtr nPos = TREELIST_APPEND);

    /** Insert a redline entry.

        rRedlineType contains the image for this redline entry (plus for insertion, minus for deletion etc.).
        rStr contains the rest of the redline entry; the columns are delimited by '\t'.
    */
    SvTreeListEntry* InsertEntry(const Image &rRedlineType, const OUString &rStr, RedlinData *pUserData,
                                 SvTreeListEntry* pParent = NULL, sal_uIntPtr nPos = TREELIST_APPEND);

    virtual SvTreeListEntry* CreateEntry() const SAL_OVERRIDE;

    void            SetColCompareHdl(const Link<>& rLink ) { aColCompareLink = rLink; }
    const Link<>&   GetColCompareHdl() const { return aColCompareLink; }
};

/// Tabpage with the filter text entries etc.
class SVX_DLLPUBLIC SAL_WARN_UNUSED SvxTPFilter: public TabPage
{
private:

    Link<>          aReadyLink;
    Link<>          aModifyLink;
    Link<>          aModifyDateLink;
    Link<>          aModifyAuthorLink;
    Link<>          aModifyRefLink;
    Link<>          aRefLink;
    Link<>          aModifyComLink;

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
    bool            bModified;

    DECL_LINK( SelDateHdl, ListBox* );
    DECL_LINK( RowEnableHdl, CheckBox* );
    DECL_LINK( TimeHdl, ImageButton* );
    DECL_LINK( ModifyHdl, void* );
    DECL_LINK( ModifyDate, void* );
    DECL_LINK( RefHandle, PushButton* );

protected:

    void            ShowDateFields(SvxRedlinDateMode nKind);
    void            EnableDateLine1(bool bFlag);
    void            EnableDateLine2(bool bFlag);

public:
                    SvxTPFilter( vcl::Window * pParent);
    virtual         ~SvxTPFilter();
    virtual void    dispose() SAL_OVERRIDE;

    virtual void    DeactivatePage() SAL_OVERRIDE;
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
    void            InsertAuthor( const OUString& rString, sal_Int32  nPos = LISTBOX_APPEND );
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

    void            DisableRef(bool bFlag);

    bool            IsDate();
    bool            IsAuthor();
    bool            IsRange();
    bool            IsAction();
    bool            IsComment();

    void            ShowAction(bool bShow=true);

    void            CheckDate(bool bFlag=true);
    void            CheckAuthor(bool bFlag=true);
    void            CheckRange(bool bFlag=true);
    void            CheckAction(bool bFlag=true);
    void            CheckComment(bool bFlag=true);

    ListBox*        GetLbAction() { return m_pLbAction;}

    void            SetReadyHdl( const Link<>& rLink ) { aReadyLink= rLink; }
    const Link<>&   GetReadyHdl() const { return aReadyLink; }

    void            SetModifyHdl( const Link<>& rLink ) { aModifyLink = rLink; }
    const Link<>&   GetModifyHdl() const { return aModifyLink; }

    void            SetModifyDateHdl( const Link<>& rLink ) { aModifyDateLink = rLink; }
    const Link<>&   GetModifyDateHdl() const { return aModifyDateLink; }

    void            SetModifyAuthorHdl( const Link<>& rLink ) { aModifyAuthorLink = rLink; }
    const Link<>&   GetModifyAuthorHdl() const { return aModifyAuthorLink; }

    void            SetModifyCommentHdl(const Link<>& rLink ) { aModifyComLink = rLink; }
    const Link<>&   GetModifyCommentHdl() const { return aModifyComLink; }


    // Methods for Calc {
    void            SetModifyRangeHdl( const Link<>& rLink ) { aModifyRefLink = rLink; }
    const Link<>&   GetModifyRangeHdl() const { return aModifyRefLink; }

    void            SetRefHdl( const Link<>& rLink ) { aRefLink = rLink; }
    const Link<>&   GetRefHdl() const { return aRefLink; }

    void            Enable( bool bEnable = true, bool bChild = true );
    void            Disable( bool bChild = true );
    // } Methods for Calc
};

/// Tabpage with the redlining entries.
class SVX_DLLPUBLIC SAL_WARN_UNUSED SvxTPView : public TabPage
{
private:

    Link<>          AcceptClickLk;
    Link<>          AcceptAllClickLk;
    Link<>          RejectClickLk;
    Link<>          RejectAllClickLk;
    Link<>          UndoClickLk;

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

    DECL_LINK( PbClickHdl, PushButton* );

public:
    SvxTPView(vcl::Window * pParent, VclBuilderContainer *pTopLevel);
    virtual ~SvxTPView();
    virtual void    dispose() SAL_OVERRIDE;

    void            InsertWriterHeader();
    void            InsertCalcHeader();
    SvxRedlinTable* GetTableControl() { return m_pViewData;}

    void            EnableAccept(bool nFlag=true);
    void            EnableAcceptAll(bool nFlag=true);
    void            EnableReject(bool nFlag=true);
    void            EnableRejectAll(bool nFlag=true);
    void            EnableUndo(bool nFlag=true);

    void            DisableAccept()     {EnableAccept(false);}
    void            DisableAcceptAll()  {EnableAcceptAll(false);}
    void            DisableReject()     {EnableReject(false);}
    void            DisableRejectAll()  {EnableRejectAll(false);}
    void            DisableUndo()       {EnableUndo(false);}

    void            ShowUndo(bool nFlag=true);
    void            HideUndo()          {ShowUndo(false);}
    bool            IsUndoVisible();

    void            SetAcceptClickHdl( const Link<>& rLink ) { AcceptClickLk = rLink; }
    const Link<>&   GetAcceptClickHdl() const { return AcceptClickLk; }

    void            SetAcceptAllClickHdl( const Link<>& rLink ) { AcceptAllClickLk = rLink; }
    const Link<>&   GetAcceptAllClickHdl() const { return AcceptAllClickLk; }

    void            SetRejectClickHdl( const Link<>& rLink ) { RejectClickLk = rLink; }
    const Link<>&   GetRejectClickHdl() const { return RejectClickLk; }

    void            SetRejectAllClickHdl( const Link<>& rLink ) { RejectAllClickLk = rLink; }
    const Link<>&   GetRejectAllClickHdl() const { return RejectAllClickLk; }

    void            SetUndoClickHdl( const Link<>& rLink ) { UndoClickLk = rLink; }
    const Link<>&   GetUndoAllClickHdl() const { return UndoClickLk; }

    virtual void    ActivatePage() SAL_OVERRIDE;
    virtual void    DeactivatePage() SAL_OVERRIDE;
};


//  Redlining - Control (Accept- Changes)
class SVX_DLLPUBLIC SAL_WARN_UNUSED SvxAcceptChgCtr
    : public TabControl
    , public VclBuilderContainer
{
private:

    VclPtr<SvxTPFilter>    pTPFilter;
    VclPtr<SvxTPView>      pTPView;

    sal_uInt16      m_nViewPageId;
    sal_uInt16      m_nFilterPageId;

public:
                    SvxAcceptChgCtr(vcl::Window* pParent, VclBuilderContainer* pTopLevel);

                    virtual ~SvxAcceptChgCtr();
    virtual void    dispose() SAL_OVERRIDE;

    void            ShowFilterPage();
    void            ShowViewPage();

    SvxTPFilter*    GetFilterPage() { return pTPFilter;}
    SvxTPView*      GetViewPage() { return pTPView;}
    SvxRedlinTable* GetViewTable();
};

#endif // INCLUDED_SVX_CTREDLIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
