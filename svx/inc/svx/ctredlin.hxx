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

#ifndef _SVX_CTREDLIN_HXX
#define _SVX_CTREDLIN_HXX

#include <vcl/morebtn.hxx>
#include <vcl/combobox.hxx>
#include <svtools/headbar.hxx>
#include <svtools/svtabbx.hxx>
#include "svtools/svlbitm.hxx"
#include <vcl/lstbox.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>

#ifndef _SVX_SIMPTABL
#include <svx/simptabl.hxx>
#endif
#include <vcl/tabctrl.hxx>
#include <tools/datetime.hxx>
#include <svtools/txtcmp.hxx>
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

class SvxRedlinEntry : public SvLBoxEntry
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
                    SvLBoxColorString( SvLBoxEntry*,sal_uInt16 nFlags,const XubString& rStr,
                                    const Color& rCol);
                    SvLBoxColorString();
                    ~SvLBoxColorString();

    void            Paint( const Point&, SvTreeListBox& rDev, sal_uInt16 nFlags,SvLBoxEntry* );
    SvLBoxItem*     Create() const;
};

class SVX_DLLPUBLIC SvxRedlinTable : public SvxSimpleTable
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

    virtual StringCompare   ColCompare(SvLBoxEntry*,SvLBoxEntry*);
    virtual void            InitEntry(SvLBoxEntry*,const XubString&,const Image&,const Image&,SvLBoxButtonKind);



public:

    SvxRedlinTable(SvxSimpleTableContainer& rParent, WinBits nBits = WB_BORDER);
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

    SvLBoxEntry*    InsertEntry(const String& ,RedlinData *pUserData,
                                SvLBoxEntry* pParent=NULL,sal_uIntPtr nPos=LIST_APPEND);

    SvLBoxEntry*    InsertEntry(const String& ,RedlinData *pUserData,const Color&,
                                SvLBoxEntry* pParent=NULL,sal_uIntPtr nPos=LIST_APPEND);


    virtual SvLBoxEntry* CreateEntry() const;

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
    CheckBox        aCbDate;
    ListBox         aLbDate;
    DateField       aDfDate;
    TimeField       aTfDate;
    ImageButton     aIbClock;
    FixedText       aFtDate2;
    DateField       aDfDate2;
    TimeField       aTfDate2;
    ImageButton     aIbClock2;
    CheckBox        aCbAuthor;
    ListBox         aLbAuthor;
    CheckBox        aCbRange;
    Edit            aEdRange;
    PushButton      aBtnRange;
    ListBox         aLbAction;
    CheckBox        aCbComment;
    Edit            aEdComment;
    String          aActionStr;
    String          aRangeStr;
    String          aStrMyName;
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

    String          GetMyName() const;
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

    SvxSimpleTableContainer m_aViewDataContainer;
    SvxRedlinTable  aViewData;
    PushButton      PbAccept;
    PushButton      PbReject;
    PushButton      PbAcceptAll;
    PushButton      PbRejectAll;
    PushButton      PbUndo;
    String          aTitle1;
    String          aTitle2;
    String          aTitle3;
    String          aTitle4;
    String          aTitle5;
    String          aStrMyName;
    long            nDistance;
    Size            aMinSize;

    DECL_LINK( PbClickHdl, PushButton* );


protected:

    void            Resize();

public:
                    SvxTPView( Window * pParent);

    String          GetMyName() const;

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

    Size            GetMinSizePixel();

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
};

//==================================================================
//  Redlining - Control (Accept- Changes)
//==================================================================

class SVX_DLLPUBLIC SvxAcceptChgCtr : public Control
{
private:

    Link            aMinSizeLink;
    TabControl      aTCAccept;
    SvxTPFilter*    pTPFilter;
    SvxTPView*      pTPView;
    Size            aMinSize;

protected:

    virtual void    Resize();

public:
                    SvxAcceptChgCtr( Window* pParent, const ResId& rResId );

                    ~SvxAcceptChgCtr();

    Size            GetMinSizePixel() const;

    void            ShowFilterPage();
    void            ShowViewPage();

    SvxTPFilter*    GetFilterPage();
    SvxTPView*      GetViewPage();
    SvxRedlinTable* GetViewTable();

    void            SetMinSizeHdl( const Link& rLink ) { aMinSizeLink= rLink; }
    const Link&     GetMinSizeHdl() const { return aMinSizeLink; }
};


#endif // _SVX_CTREDLIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
