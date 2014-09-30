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
#include <vcl/combobox.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/tabctrl.hxx>
#include <unotools/textsearch.hxx>
#include <svx/svxdllapi.h>

#define FLT_DATE_BEFORE     0
#define FLT_DATE_SINCE      1
#define FLT_DATE_EQUAL      2
#define FLT_DATE_NOTEQUAL   3
#define FLT_DATE_BETWEEN    4
#define FLT_DATE_SAVE       5


//  Struct for sorting data

class SVX_DLLPUBLIC RedlinData
{
public:
                    RedlinData();
    virtual         ~RedlinData();
    bool            bDisabled;
    DateTime        aDateTime;
    void*           pData;
};

class SvxRedlinEntry : public SvTreeListEntry
{
public:
                    SvxRedlinEntry();
        virtual     ~SvxRedlinEntry();
};

// Class for the representation of Strings depending on the font
class SvLBoxColorString : public SvLBoxString
{
private:

    Color           aPrivColor;

public:
                    SvLBoxColorString( SvTreeListEntry*, sal_uInt16 nFlags, const OUString& rStr,
                                    const Color& rCol);
                    SvLBoxColorString();
                    virtual ~SvLBoxColorString();

    virtual void Paint(
        const Point& rPos, SvTreeListBox& rOutDev, const SvViewDataEntry* pView, const SvTreeListEntry* pEntry) SAL_OVERRIDE;

    SvLBoxItem*     Create() const SAL_OVERRIDE;
};

class SVX_DLLPUBLIC SvxRedlinTable : public SvSimpleTable
{
    using SvTabListBox::InsertEntry;

private:

    bool            bIsCalc;
    sal_uInt16      nDatePos;
    bool            bAuthor;
    bool            bDate;
    bool            bComment;
    sal_uInt16      nDaTiMode;
    DateTime        aDaTiFirst;
    DateTime        aDaTiLast;
    DateTime        aDaTiFilterFirst;
    DateTime        aDaTiFilterLast;
    OUString        aAuthor;
    Color           aEntryColor;
    OUString        aCurEntry;
    utl::TextSearch* pCommentSearcher;
    Link            aColCompareLink;

protected:

    virtual sal_Int32       ColCompare(SvTreeListEntry*,SvTreeListEntry*) SAL_OVERRIDE;
    virtual void            InitEntry(SvTreeListEntry*, const OUString&, const Image&, const Image&, SvLBoxButtonKind) SAL_OVERRIDE;



public:

    SvxRedlinTable(SvSimpleTableContainer& rParent, WinBits nBits = WB_BORDER);
    virtual ~SvxRedlinTable();

    // For FilterPage only {
    void            SetFilterDate(bool bFlag=true);
    void            SetDateTimeMode(sal_uInt16 nMode);
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

    SvTreeListEntry*    InsertEntry(const OUString& ,RedlinData *pUserData,
                                SvTreeListEntry* pParent=NULL,sal_uIntPtr nPos=TREELIST_APPEND);

    SvTreeListEntry*    InsertEntry(const OUString& ,RedlinData *pUserData,const Color&,
                                SvTreeListEntry* pParent=NULL,sal_uIntPtr nPos=TREELIST_APPEND);


    virtual SvTreeListEntry* CreateEntry() const SAL_OVERRIDE;

    void            SetColCompareHdl(const Link& rLink ) { aColCompareLink = rLink; }
    const Link&     GetColCompareHdl() const { return aColCompareLink; }


};


//  Filter- Tabpage

class SVX_DLLPUBLIC SvxTPFilter: public TabPage
{
private:

    Link            aReadyLink;
    Link            aModifyLink;
    Link            aModifyDateLink;
    Link            aModifyAuthorLink;
    Link            aModifyRefLink;
    Link            aRefLink;
    Link            aModifyComLink;

    SvxRedlinTable* pRedlinTable;
    CheckBox*       m_pCbDate;
    ListBox*        m_pLbDate;
    DateField*      m_pDfDate;
    TimeField*      m_pTfDate;
    PushButton*     m_pIbClock;
    FixedText*      m_pFtDate2;
    DateField*      m_pDfDate2;
    TimeField*      m_pTfDate2;
    PushButton*     m_pIbClock2;
    CheckBox*       m_pCbAuthor;
    ListBox*        m_pLbAuthor;
    CheckBox*       m_pCbRange;
    Edit*           m_pEdRange;
    PushButton*     m_pBtnRange;
    CheckBox*       m_pCbAction;
    ListBox*        m_pLbAction;
    CheckBox*       m_pCbComment;
    Edit*           m_pEdComment;
    bool            bModified;

    DECL_LINK( SelDateHdl, ListBox* );
    DECL_LINK( RowEnableHdl, CheckBox* );
    DECL_LINK( TimeHdl, ImageButton* );
    DECL_LINK( ModifyHdl, void* );
    DECL_LINK( ModifyDate, void* );
    DECL_LINK( RefHandle, PushButton* );


protected:

    void            ShowDateFields(sal_uInt16 nKind);
    void            EnableDateLine1(bool bFlag);
    void            EnableDateLine2(bool bFlag);

public:
                    SvxTPFilter( vcl::Window * pParent);

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
    sal_uInt16          GetDateMode();

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

    void            SetReadyHdl( const Link& rLink ) { aReadyLink= rLink; }
    const Link&     GetReadyHdl() const { return aReadyLink; }

    void            SetModifyHdl( const Link& rLink ) { aModifyLink = rLink; }
    const Link&     GetModifyHdl() const { return aModifyLink; }

    void            SetModifyDateHdl( const Link& rLink ) { aModifyDateLink = rLink; }
    const Link&     GetModifyDateHdl() const { return aModifyDateLink; }

    void            SetModifyAuthorHdl( const Link& rLink ) { aModifyAuthorLink = rLink; }
    const Link&     GetModifyAuthorHdl() const { return aModifyAuthorLink; }

    void            SetModifyCommentHdl(const Link& rLink ) { aModifyComLink = rLink; }
    const Link&     GetModifyCommentHdl() const { return aModifyComLink; }


    // Methods for Calc {
    void            SetModifyRangeHdl( const Link& rLink ) { aModifyRefLink = rLink; }
    const Link&     GetModifyRangeHdl() const { return aModifyRefLink; }

    void            SetRefHdl( const Link& rLink ) { aRefLink = rLink; }
    const Link&     GetRefHdl() const { return aRefLink; }

    void            Enable( bool bEnable = true, bool bChild = true );
    void            Disable( bool bChild = true );

    // } Methods for Calc
};



//  View- Tabpage


class SVX_DLLPUBLIC SvxTPView: public TabPage
{
private:

    Link            AcceptClickLk;
    Link            AcceptAllClickLk;
    Link            RejectClickLk;
    Link            RejectAllClickLk;
    Link            UndoClickLk;

    SvxRedlinTable* m_pViewData;
    PushButton*     m_pAccept;
    PushButton*     m_pReject;
    PushButton*     m_pAcceptAll;
    PushButton*     m_pRejectAll;
    PushButton*     m_pUndo;

    bool bEnableAccept;
    bool bEnableAcceptAll;
    bool bEnableReject;
    bool bEnableRejectAll;
    bool bEnableUndo;

    DECL_LINK( PbClickHdl, PushButton* );

public:
    SvxTPView(vcl::Window * pParent);
    virtual ~SvxTPView();

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

    void            SetAcceptClickHdl( const Link& rLink ) { AcceptClickLk = rLink; }
    const Link&     GetAcceptClickHdl() const { return AcceptClickLk; }

    void            SetAcceptAllClickHdl( const Link& rLink ) { AcceptAllClickLk = rLink; }
    const Link&     GetAcceptAllClickHdl() const { return AcceptAllClickLk; }

    void            SetRejectClickHdl( const Link& rLink ) { RejectClickLk = rLink; }
    const Link&     GetRejectClickHdl() const { return RejectClickLk; }

    void            SetRejectAllClickHdl( const Link& rLink ) { RejectAllClickLk = rLink; }
    const Link&     GetRejectAllClickHdl() const { return RejectAllClickLk; }

    void            SetUndoClickHdl( const Link& rLink ) { UndoClickLk = rLink; }
    const Link&     GetUndoAllClickHdl() const { return UndoClickLk; }

    virtual void    ActivatePage() SAL_OVERRIDE;
    virtual void    DeactivatePage() SAL_OVERRIDE;
};


//  Redlining - Control (Accept- Changes)


class SVX_DLLPUBLIC SvxAcceptChgCtr
    : public TabControl
    , public VclBuilderContainer
{
private:

    SvxTPFilter*    pTPFilter;
    SvxTPView*      pTPView;

    sal_uInt16      m_nViewPageId;
    sal_uInt16      m_nFilterPageId;

public:
                    SvxAcceptChgCtr(vcl::Window* pParent);

                    virtual ~SvxAcceptChgCtr();

    void            ShowFilterPage();
    void            ShowViewPage();

    SvxTPFilter*    GetFilterPage() { return pTPFilter;}
    SvxTPView*      GetViewPage() { return pTPView;}
    SvxRedlinTable* GetViewTable();
};


#endif // INCLUDED_SVX_CTREDLIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
