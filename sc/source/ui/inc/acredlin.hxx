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

#ifndef INCLUDED_SC_SOURCE_UI_INC_ACREDLIN_HXX
#define INCLUDED_SC_SOURCE_UI_INC_ACREDLIN_HXX

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
#include <vcl/idle.hxx>

class ScViewData;
class ScDocument;

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

class ScAcceptChgDlg : public SfxModelessDialog
{
private:

    Idle                    aSelectionIdle;
    Idle                    aReOpenIdle;
    VclPtr<SvxAcceptChgCtr> m_pAcceptChgCtr;
    ScViewData*             pViewData;
    ScDocument*             pDoc;
    ScRangeName             aLocalRangeName;
    VclPtr<SvxTPFilter>     pTPFilter;
    VclPtr<SvxTPView>       pTPView;
    VclPtr<SvxRedlinTable>  pTheView; // #i48648 now SvHeaderTabListBox
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

    DECL_LINK_TYPED( FilterHandle, SvxTPFilter*, void );
    DECL_LINK_TYPED( RefHandle, SvxTPFilter*, void );
    DECL_LINK_TYPED( RejectHandle, SvxTPView*, void);
    DECL_LINK_TYPED( AcceptHandle, SvxTPView*, void);
    DECL_LINK_TYPED( RejectAllHandle, SvxTPView*, void);
    DECL_LINK_TYPED( AcceptAllHandle, SvxTPView*, void);
    DECL_LINK_TYPED( ExpandingHandle, SvTreeListBox*, bool);
    DECL_LINK_TYPED( SelectHandle, SvTreeListBox*, void);
    DECL_LINK( RefInfoHandle, OUString*);

    DECL_LINK_TYPED( UpdateSelectionHdl, Idle*, void );
    DECL_LINK_TYPED( ChgTrackModHdl, ScChangeTrack&, void);
    DECL_LINK_TYPED( CommandHdl, SvSimpleTable*, void);
    DECL_LINK_TYPED( ReOpenTimerHdl, Idle*, void );
    DECL_LINK_TYPED( ColCompareHdl, const SvSortData*, sal_Int32);

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
                    ScAcceptChgDlg( SfxBindings* pB, SfxChildWindow* pCW, vcl::Window* pParent,
                               ScViewData*      ptrViewData);

                    virtual ~ScAcceptChgDlg();
    virtual void    dispose() SAL_OVERRIDE;

    void            ReInit(ScViewData* ptrViewData);

    virtual bool    PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;

    void            Initialize (SfxChildWinInfo* pInfo);
    virtual void    FillInfo(SfxChildWinInfo&) const SAL_OVERRIDE;

};

#endif // INCLUDED_SC_SOURCE_UI_INC_ACREDLIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
