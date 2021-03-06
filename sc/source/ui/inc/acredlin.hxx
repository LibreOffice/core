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

#pragma once

#include <svx/ctredlin.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/bindings.hxx>
#include <chgtrack.hxx>
#include <chgviset.hxx>
#include <vcl/idle.hxx>

class ScViewData;
class ScDocument;

struct SfxChildWinInfo;

class ScRedlinData : public RedlinData
{
public:
                    ScRedlinData();
                    virtual ~ScRedlinData() override;
    sal_uLong       nActionNo;
    sal_uLong       nInfo;
    SCTAB           nTable;
    SCCOL           nCol;
    SCROW           nRow;
    bool            bIsRejectable;
    bool            bIsAcceptable;
};

class ScAcceptChgDlg final : public SfxModelessDialogController
{
    Idle                    aSelectionIdle;
    Idle                    aReOpenIdle;
    ScViewData*             pViewData;
    ScDocument*             pDoc;
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

    SvxTPFilter* pTPFilter;
    SvxTPView* pTPView;
    SvxRedlinTable* pTheView; // #i48648 now SvHeaderTabListBox

    std::unique_ptr<weld::Container> m_xContentArea;
    std::unique_ptr<weld::Menu> m_xPopup, m_xSortMenu;
    std::unique_ptr<SvxAcceptChgCtr> m_xAcceptChgCtr;

    void            Init();

    DECL_LINK( FilterHandle, SvxTPFilter*, void );
    DECL_LINK( RefHandle, SvxTPFilter*, void );
    DECL_LINK( RejectHandle, SvxTPView*, void );
    DECL_LINK( AcceptHandle, SvxTPView*, void );
    DECL_LINK( RejectAllHandle, SvxTPView*, void );
    DECL_LINK( AcceptAllHandle, SvxTPView*, void );
    DECL_LINK( ExpandingHandle, const weld::TreeIter&, bool );
    DECL_LINK( SelectHandle, weld::TreeView&, void );
    DECL_LINK( RefInfoHandle, const OUString*, void );

    DECL_LINK( UpdateSelectionHdl, Timer*, void );
    DECL_LINK( ChgTrackModHdl, ScChangeTrack&, void );
    DECL_LINK( CommandHdl, const CommandEvent&, bool );
    DECL_LINK( ReOpenTimerHdl, Timer*, void );

    int ColCompareHdl(const weld::TreeIter& rLeft, const weld::TreeIter& rRight) const;

    void            RejectFiltered();
    void            AcceptFiltered();

    bool            IsValidAction(const ScChangeAction* pScChangeAction);

    OUString* MakeTypeString(ScChangeActionType eType);

    std::unique_ptr<weld::TreeIter> AppendChangeAction(
        const ScChangeAction* pScChangeAction, bool bCreateOnDemand,
        const weld::TreeIter* pParent = nullptr, bool bDelMaster = false,
        bool bDisabled = false);

    std::unique_ptr<weld::TreeIter> AppendFilteredAction(
        const ScChangeAction* pScChangeAction,ScChangeActionState eState,
        bool bCreateOnDemand,
        const weld::TreeIter* pParent = nullptr, bool bDelMaster = false,
        bool bDisabled = false);

    std::unique_ptr<weld::TreeIter> InsertChangeActionContent(const ScChangeActionContent* pScChangeAction,
        const weld::TreeIter& rParent, sal_uLong nSpecial);

    void            GetDependents(const ScChangeAction* pScChangeAction,
                                 ScChangeActionMap& aActionMap,
                                 const weld::TreeIter& rEntry);

    bool            InsertContentChildren(ScChangeActionMap* pActionMap, const weld::TreeIter& rParent);

    bool            InsertAcceptedORejected(const weld::TreeIter& rParent);

    bool            InsertDeletedChildren(const ScChangeAction* pChangeAction, ScChangeActionMap* pActionMap,
                                          const weld::TreeIter& rParent);

    bool            InsertChildren(ScChangeActionMap* pActionMap, const weld::TreeIter& rParent);

    void            AppendChanges(const ScChangeTrack* pChanges,sal_uLong nStartAction, sal_uLong nEndAction);

    void            RemoveEntries(sal_uLong nStartAction,sal_uLong nEndAction);
    void            UpdateEntries(const ScChangeTrack* pChgTrack, sal_uLong nStartAction,sal_uLong nEndAction);

    void            UpdateView();
    void            ClearView();

    bool            Expand(const ScChangeTrack* pChanges,const ScChangeAction* pScChangeAction,
                           const weld::TreeIter& rEntry, bool bFilter = false);

public:
    ScAcceptChgDlg(SfxBindings* pB, SfxChildWindow* pCW, weld::Window* pParent,
                   ScViewData* ptrViewData);
    virtual ~ScAcceptChgDlg() override;

    void            ReInit(ScViewData* ptrViewData);

    void            Initialize (SfxChildWinInfo* pInfo);
    virtual void    FillInfo(SfxChildWinInfo&) const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
