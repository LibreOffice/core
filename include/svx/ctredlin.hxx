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

#ifndef _SVX_CTREDLIN_HXX
#define _SVX_CTREDLIN_HXX

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
#include "svx/svxdllapi.h"

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
    sal_Bool            bDisabled;
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
                    ~SvLBoxColorString();

    virtual void Paint(
        const Point& rPos, SvTreeListBox& rOutDev, const SvViewDataEntry* pView, const SvTreeListEntry* pEntry);

    SvLBoxItem*     Create() const;
};

class SVX_DLLPUBLIC SvxRedlinTable : public SvSimpleTable
{
    using SvTabListBox::InsertEntry;

private:

    sal_Bool            bIsCalc;
    sal_uInt16          nDatePos;
    sal_Bool            bAuthor;
    sal_Bool            bDate;
    sal_Bool            bComment;
    sal_uInt16          nDaTiMode;
    DateTime        aDaTiFirst;
    DateTime        aDaTiLast;
    DateTime        aDaTiFilterFirst;
    DateTime        aDaTiFilterLast;
    String          aAuthor;
    Color           aEntryColor;
    String          aCurEntry;
    utl::TextSearch* pCommentSearcher;
    Link            aColCompareLink;

protected:

    virtual StringCompare   ColCompare(SvTreeListEntry*,SvTreeListEntry*);
    virtual void            InitEntry(SvTreeListEntry*, const OUString&, const Image&, const Image&, SvLBoxButtonKind);



public:

    SvxRedlinTable(SvSimpleTableContainer& rParent, WinBits nBits = WB_BORDER);
    ~SvxRedlinTable();

    // For FilterPage only {
    void            SetFilterDate(sal_Bool bFlag=sal_True);
    void            SetDateTimeMode(sal_uInt16 nMode);
    void            SetFirstDate(const Date&);
    void            SetLastDate(const Date&);
    void            SetFirstTime(const Time&);
    void            SetLastTime(const Time&);
    void            SetFilterAuthor(sal_Bool bFlag=sal_True);
    void            SetAuthor(const String &);
    void            SetFilterComment(sal_Bool bFlag=sal_True);
    void            SetCommentParams( const utl::SearchParam* pSearchPara );

    void            UpdateFilterTest();
    // } For FilterPage only

    void            SetCalcView(sal_Bool bFlag=sal_True);

    // no NULL-pointer checking {
    sal_Bool            IsValidEntry(const String* pAuthor,const DateTime *pDateTime,const String* pComment);
    sal_Bool            IsValidEntry(const String* pAuthor,const DateTime *pDateTime);
    sal_Bool            IsValidComment(const String* pComment);
    // }

    SvTreeListEntry*    InsertEntry(const OUString& ,RedlinData *pUserData,
                                SvTreeListEntry* pParent=NULL,sal_uIntPtr nPos=LIST_APPEND);

    SvTreeListEntry*    InsertEntry(const OUString& ,RedlinData *pUserData,const Color&,
                                SvTreeListEntry* pParent=NULL,sal_uIntPtr nPos=LIST_APPEND);


    virtual SvTreeListEntry* CreateEntry() const;

    void            SetColCompareHdl(const Link& rLink ) { aColCompareLink = rLink; }
    const Link&     GetColCompareHdl() const { return aColCompareLink; }


};

//==================================================================
//  Filter- Tabpage
//==================================================================
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
    sal_Bool            bModified;

    DECL_LINK( SelDateHdl, ListBox* );
    DECL_LINK( RowEnableHdl, CheckBox* );
    DECL_LINK( TimeHdl, ImageButton* );
    DECL_LINK( ModifyHdl, void* );
    DECL_LINK( ModifyDate, void* );
    DECL_LINK( RefHandle, PushButton* );


protected:

    void            ShowDateFields(sal_uInt16 nKind);
    void            EnableDateLine1(sal_Bool bFlag);
    void            EnableDateLine2(sal_Bool bFlag);

public:
                    SvxTPFilter( Window * pParent);

    virtual void    DeactivatePage();
    void            SetRedlinTable(SvxRedlinTable*);

    Date            GetFirstDate() const;
    void            SetFirstDate(const Date &aDate);
    Time            GetFirstTime() const;
    void            SetFirstTime(const Time &aTime);

    Date            GetLastDate() const;
    void            SetLastDate(const Date &aDate);
    Time            GetLastTime() const;
    void            SetLastTime(const Time &aTime);

    void            SetDateMode(sal_uInt16 nMode);
    sal_uInt16          GetDateMode();

    void            ClearAuthors();
    void            InsertAuthor( const String& rString, sal_uInt16 nPos = LISTBOX_APPEND );
    String          GetSelectedAuthor()const;
    void            SelectedAuthorPos(sal_uInt16 nPos);
    sal_uInt16          SelectAuthor(const String& aString);
    void            SetComment(const String &rComment);
    String          GetComment()const;


    // Methods for Calc {
    void            SetRange(const String& rString);
    String          GetRange() const;
    void            HideRange(sal_Bool bHide=sal_True);
    void            SetFocusToRange();
    // } Methods for Calc

    void            DisableRef(sal_Bool bFlag);

    sal_Bool            IsDate();
    sal_Bool            IsAuthor();
    sal_Bool            IsRange();
    sal_Bool            IsAction();
    sal_Bool            IsComment();

    void            ShowAction(sal_Bool bShow=sal_True);

    void            CheckDate(sal_Bool bFlag=sal_True);
    void            CheckAuthor(sal_Bool bFlag=sal_True);
    void            CheckRange(sal_Bool bFlag=sal_True);
    void            CheckAction(sal_Bool bFlag=sal_True);
    void            CheckComment(sal_Bool bFlag=sal_True);

    ListBox*        GetLbAction();

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


//==================================================================
//  View- Tabpage
//==================================================================

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

    DECL_LINK( PbClickHdl, PushButton* );

public:
    SvxTPView(Window * pParent);
    ~SvxTPView();

    void            InsertWriterHeader();
    void            InsertCalcHeader();
    SvxRedlinTable* GetTableControl();

    void            EnableAccept(sal_Bool nFlag=sal_True);
    void            EnableAcceptAll(sal_Bool nFlag=sal_True);
    void            EnableReject(sal_Bool nFlag=sal_True);
    void            EnableRejectAll(sal_Bool nFlag=sal_True);
    void            EnableUndo(sal_Bool nFlag=sal_True);

    void            DisableAccept()     {EnableAccept(sal_False);}
    void            DisableAcceptAll()  {EnableAcceptAll(sal_False);}
    void            DisableReject()     {EnableReject(sal_False);}
    void            DisableRejectAll()  {EnableRejectAll(sal_False);}
    void            DisableUndo()       {EnableUndo(sal_False);}

    void            ShowUndo(sal_Bool nFlag=sal_True);
    void            HideUndo()          {ShowUndo(sal_False);}
    sal_Bool            IsUndoVisible();

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

    virtual void    ActivatePage();
    virtual void    DeactivatePage();
};

//==================================================================
//  Redlining - Control (Accept- Changes)
//==================================================================

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
                    SvxAcceptChgCtr(Window* pParent);

                    ~SvxAcceptChgCtr();

    void            ShowFilterPage();
    void            ShowViewPage();

    SvxTPFilter*    GetFilterPage();
    SvxTPView*      GetViewPage();
    SvxRedlinTable* GetViewTable();
};


#endif // _SVX_CTREDLIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
