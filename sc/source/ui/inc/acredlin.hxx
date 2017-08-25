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
                    virtual ~ScRedlinData() override;
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
    VclPtr<PopupMenu>       m_xPopup;
    VclPtr<SvxAcceptChgCtr> m_pAcceptChgCtr;
    ScViewData*             pViewData;
    ScDocument*             pDoc;
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
    bool                    bIgnoreMsg:1;
    bool                    bNoSelection:1;
    bool                    bHasFilterEntry:1;
    bool                    bUseColor:1;

    void            Init();

    DECL_LINK( FilterHandle, SvxTPFilter*, void );
    DECL_LINK( RefHandle, SvxTPFilter*, void );
    DECL_LINK( RejectHandle, SvxTPView*, void );
    DECL_LINK( AcceptHandle, SvxTPView*, void );
    DECL_LINK( RejectAllHandle, SvxTPView*, void );
    DECL_LINK( AcceptAllHandle, SvxTPView*, void );
    DECL_LINK( ExpandingHandle, SvTreeListBox*, bool );
    DECL_LINK( SelectHandle, SvTreeListBox*, void );
    DECL_LINK( RefInfoHandle, const OUString*, void );

    DECL_LINK( UpdateSelectionHdl, Timer*, void );
    DECL_LINK( ChgTrackModHdl, ScChangeTrack&, void );
    DECL_LINK( CommandHdl, SvSimpleTable*, void );
    DECL_LINK( ReOpenTimerHdl, Timer*, void );
    DECL_LINK( ColCompareHdl, const SvSortData*, sal_Int32 );

protected:

    void            RejectFiltered();
    void            AcceptFiltered();

    bool            IsValidAction(const ScChangeAction* pScChangeAction);

    OUString* MakeTypeString(ScChangeActionType eType);

    SvTreeListEntry* AppendChangeAction(
        const ScChangeAction* pScChangeAction,
        SvTreeListEntry* pParent=nullptr,bool bDelMaster = false,
        bool bDisabled = false);

    SvTreeListEntry* AppendFilteredAction(
        const ScChangeAction* pScChangeAction,ScChangeActionState eState,
        SvTreeListEntry* pParent = nullptr,bool bDelMaster = false,
        bool bDisabled = false);

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

    void            AppendChanges(const ScChangeTrack* pChanges,sal_uLong nStartAction, sal_uLong nEndAction);

    void            RemoveEntrys(sal_uLong nStartAction,sal_uLong nEndAction);
    void            UpdateEntrys(const ScChangeTrack* pChgTrack, sal_uLong nStartAction,sal_uLong nEndAction);

    void            UpdateView();
    void            ClearView();

    bool            Expand(const ScChangeTrack* pChanges,const ScChangeAction* pScChangeAction,
                           SvTreeListEntry* pEntry, bool bFilter = false);

public:
                    ScAcceptChgDlg( SfxBindings* pB, SfxChildWindow* pCW, vcl::Window* pParent,
                               ScViewData*      ptrViewData);

                    virtual ~ScAcceptChgDlg() override;
    virtual void    dispose() override;

    void            ReInit(ScViewData* ptrViewData);

    void            Initialize (SfxChildWinInfo* pInfo);
    virtual void    FillInfo(SfxChildWinInfo&) const override;

};

#endif // INCLUDED_SC_SOURCE_UI_INC_ACREDLIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
