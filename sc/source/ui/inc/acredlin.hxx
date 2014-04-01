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

#ifndef SC_ACREDLIN_HXX
#define SC_ACREDLIN_HXX

#include <vcl/morebtn.hxx>
#include <vcl/combobox.hxx>
#include <vcl/group.hxx>
#include <svtools/headbar.hxx>
#include <svtools/simptabl.hxx>
#include <svtools/svtabbx.hxx>
#include "rangenam.hxx"
#include "anyrefdg.hxx"
#include <vcl/lstbox.hxx>
#include <svx/ctredlin.hxx>
#include "chgtrack.hxx"
#include "chgviset.hxx"
#include <vcl/timer.hxx>

class ScViewData;
class ScDocument;

#define FLT_DATE_BEFORE     0
#define FLT_DATE_SINCE      1
#define FLT_DATE_EQUAL      2
#define FLT_DATE_NOTEQUAL   3
#define FLT_DATE_BETWEEN    4
#define FLT_DATE_SAVE       5


class ScViewEntryPtr
{
private:
    OUString*       pAction;
    OUString*       pPos;
    OUString*       pAuthor;
    OUString*       pDate;
    OUString*       pComment;
    void*           pData;

public:

    OUString*       GetpAction()    {return pAction; }
    OUString*       GetpPos()       {return pPos;    }
    OUString*       GetpAuthor()    {return pAuthor; }
    OUString*       GetpDate()      {return pDate;   }
    OUString*       GetpComment()   {return pComment;}
    void*           GetpData()      {return pData;   }

    void        SetpAction (OUString* pString)    {pAction= pString;}
    void        SetpPos    (OUString* pString)    {pPos   = pString;}
    void        SetpAuthor (OUString* pString)    {pAuthor= pString;}
    void        SetpDate   (OUString* pString)    {pDate  = pString;}
    void        SetpComment(OUString* pString)    {pComment=pString;}
    void        SetpData   (void*   pdata)      {pData   =pdata;}
};

class ScRedlinData : public RedlinData
{
public:

                    ScRedlinData();
                    virtual ~ScRedlinData();
    SCTAB           nTable;
    SCCOL           nCol;
    SCROW           nRow;
    sal_uLong       nActionNo;
    sal_uLong       nInfo;
    bool            bIsRejectable;
    bool            bIsAcceptable;
};

typedef long LExpNum;

//@ Expand entrys are ambiguous and therefore removed
//DECLARE_TABLE( ScChgTrackExps, LExpNum)


class ScAcceptChgDlg : public SfxModelessDialog
{
private:

    Timer                   aSelectionTimer;
    Timer                   aReOpenTimer;
    SvxAcceptChgCtr*        m_pAcceptChgCtr;
    ScViewData*             pViewData;
    ScDocument*             pDoc;
    ScRangeName             aLocalRangeName;
    Selection               theCurSel;
    SvxTPFilter*            pTPFilter;
    SvxTPView*              pTPView;
    SvxRedlinTable*         pTheView; // #i48648 now SvHeaderTabListBox
    ScRangeList             aRangeList;
    ScChangeViewSettings    aChangeViewSet;
    OUString           aStrInsertCols;
    OUString           aStrInsertRows;
    OUString           aStrInsertTabs;
    OUString           aStrDeleteCols;
    OUString           aStrDeleteRows;
    OUString           aStrDeleteTabs;
    OUString           aStrMove;
    OUString           aStrContent;
    OUString           aStrReject;
    OUString           aStrAllAccepted;
    OUString           aStrAllRejected;
    OUString           aStrNoEntry;
    OUString           aStrContentWithChild;
    OUString           aStrChildContent;
    OUString           aStrChildOrgContent;
    OUString           aStrEmpty;
    OUString           aUnknown;
    sal_uLong                   nAcceptCount;
    sal_uLong                   nRejectCount;
    bool                    bAcceptEnableFlag:1;
    bool                    bRejectEnableFlag:1;
    bool                    bNeedsUpdate:1;
    bool                    bIgnoreMsg:1;
    bool                    bNoSelection:1;
    bool                    bHasFilterEntry:1;
    bool                    bUseColor:1;

    void            Init();
    void            InitFilter();

    DECL_LINK( FilterHandle, SvxTPFilter* );
    DECL_LINK( RefHandle, void* );
    DECL_LINK( FilterModified, void* );
    DECL_LINK( RejectHandle, SvxTPView*);
    DECL_LINK( AcceptHandle, SvxTPView*);
    DECL_LINK( RejectAllHandle, void*);
    DECL_LINK( AcceptAllHandle, void*);
    DECL_LINK( ExpandingHandle, SvxRedlinTable*);
    DECL_LINK( SelectHandle, void*);
    DECL_LINK( RefInfoHandle, OUString*);

    DECL_LINK( UpdateSelectionHdl, void*);
    DECL_LINK( ChgTrackModHdl, ScChangeTrack*);
    DECL_LINK( CommandHdl, void*);
    DECL_LINK( ReOpenTimerHdl, void*);
    DECL_LINK( ColCompareHdl, SvSortData*);



protected:

    void            RejectFiltered();
    void            AcceptFiltered();

    bool            IsValidAction(const ScChangeAction* pScChangeAction);

    OUString* MakeTypeString(ScChangeActionType eType);

    SvTreeListEntry* InsertChangeAction(
        const ScChangeAction* pScChangeAction,ScChangeActionState eState,
        SvTreeListEntry* pParent=NULL,bool bDelMaster = false,
        bool bDisabled = false,sal_uLong nPos = TREELIST_APPEND);

    SvTreeListEntry* InsertFilteredAction(
        const ScChangeAction* pScChangeAction,ScChangeActionState eState,
        SvTreeListEntry* pParent = NULL,bool bDelMaster = false,
        bool bDisabled = false, sal_uLong nPos = TREELIST_APPEND);

    SvTreeListEntry*    InsertChangeActionContent(const ScChangeActionContent* pScChangeAction,
                                              SvTreeListEntry* pParent,sal_uLong nSpecial);

    void            GetDependents( const ScChangeAction* pScChangeAction,
                                ScChangeActionMap& aActionMap,
                                SvTreeListEntry* pEntry);

    bool            InsertContentChildren( ScChangeActionMap* pActionMap, SvTreeListEntry* pParent );

    bool            InsertAcceptedORejected(SvTreeListEntry* pParent);

    bool            InsertDeletedChildren( const ScChangeAction* pChangeAction, ScChangeActionMap* pActionMap,
                                        SvTreeListEntry* pParent);

    bool            InsertChildren( ScChangeActionMap* pActionMap, SvTreeListEntry* pParent );

    void            AppendChanges(ScChangeTrack* pChanges,sal_uLong nStartAction, sal_uLong nEndAction,
                                    sal_uLong nPos=TREELIST_APPEND);

    void            RemoveEntrys(sal_uLong nStartAction,sal_uLong nEndAction);
    void            UpdateEntrys(ScChangeTrack* pChgTrack, sal_uLong nStartAction,sal_uLong nEndAction);

    void            UpdateView();
    void            ClearView();

    bool            Expand(ScChangeTrack* pChanges,const ScChangeAction* pScChangeAction,
                           SvTreeListEntry* pEntry, bool bFilter = false);

public:
                    ScAcceptChgDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                               ScViewData*      ptrViewData);

                    virtual ~ScAcceptChgDlg();

    void            ReInit(ScViewData* ptrViewData);

    virtual bool    PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;

    void            Initialize (SfxChildWinInfo* pInfo);
    virtual void    FillInfo(SfxChildWinInfo&) const SAL_OVERRIDE;

};


#endif // SC_NAMEDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
