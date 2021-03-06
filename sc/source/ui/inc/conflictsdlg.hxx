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

#include <vcl/idle.hxx>
#include <svx/ctredlin.hxx>

#include "docsh.hxx"

class ScViewData;
class ScChangeTrack;
class ScChangeAction;

enum ScConflictAction
{
    SC_CONFLICT_ACTION_NONE,
    SC_CONFLICT_ACTION_KEEP_MINE,
    SC_CONFLICT_ACTION_KEEP_OTHER
};

// struct ScConflictsListEntry

struct ScConflictsListEntry
{
    ScConflictAction    meConflictAction;
    std::vector<sal_uLong>  maSharedActions;
    std::vector<sal_uLong>  maOwnActions;

    bool                HasSharedAction( sal_uLong nSharedAction ) const;
    bool                HasOwnAction( sal_uLong nOwnAction ) const;
};

typedef ::std::vector< ScConflictsListEntry > ScConflictsList;


class ScConflictsListHelper
{
private:
    static void                     Transform_Impl( std::vector<sal_uLong>& rActionList, ScChangeActionMergeMap* pMergeMap );

public:
    static bool                     HasOwnAction( ScConflictsList& rConflictsList, sal_uLong nOwnAction );

    static ScConflictsListEntry*    GetSharedActionEntry( ScConflictsList& rConflictsList, sal_uLong nSharedAction );
    static ScConflictsListEntry*    GetOwnActionEntry( ScConflictsList& rConflictsList, sal_uLong nOwnAction );

    static void                     TransformConflictsList( ScConflictsList& rConflictsList,
                                        ScChangeActionMergeMap* pSharedMap, ScChangeActionMergeMap* pOwnMap );
};


class ScConflictsFinder final
{
private:
    ScChangeTrack*          mpTrack;
    sal_uLong               mnStartShared;
    sal_uLong               mnEndShared;
    sal_uLong               mnStartOwn;
    sal_uLong               mnEndOwn;
    ScConflictsList&        mrConflictsList;

    static bool             DoActionsIntersect( const ScChangeAction* pAction1, const ScChangeAction* pAction2 );
    ScConflictsListEntry*   GetIntersectingEntry( const ScChangeAction* pAction ) const;
    ScConflictsListEntry&   GetEntry(sal_uLong nSharedAction, const std::vector<sal_uLong>& rOwnActions);

public:
                            ScConflictsFinder( ScChangeTrack* pTrack, sal_uLong nStartShared, sal_uLong nEndShared,
                                sal_uLong nStartOwn, sal_uLong nEndOwn, ScConflictsList& rConflictsList );
                            ~ScConflictsFinder();

    bool                    Find();
};


class ScConflictsResolver final
{
private:
    ScChangeTrack*      mpTrack;
    ScConflictsList&    mrConflictsList;

public:
                        ScConflictsResolver( ScChangeTrack* pTrack, ScConflictsList& rConflictsList );
                        ~ScConflictsResolver();

    void                HandleAction( ScChangeAction* pAction, bool bIsSharedAction,
                            bool bHandleContentAction, bool bHandleNonContentAction );
};


class ScConflictsDlg : public weld::GenericDialogController
{
private:
    OUString            maStrUnknownUser;

    ScViewData* const   mpViewData;
    ScDocument*         mpOwnDoc;
    ScChangeTrack*      mpOwnTrack;
    ScDocument* const   mpSharedDoc;
    ScChangeTrack*      mpSharedTrack;
    ScConflictsList&    mrConflictsList;

    Idle                maSelectionIdle;
    bool                mbInSelectHdl;

    std::unique_ptr<weld::Button> m_xBtnKeepMine;
    std::unique_ptr<weld::Button> m_xBtnKeepOther;
    std::unique_ptr<weld::Button> m_xBtnKeepAllMine;
    std::unique_ptr<weld::Button> m_xBtnKeepAllOthers;
    std::unique_ptr<SvxRedlinTable> m_xLbConflicts;

    OUString            GetConflictString( const ScConflictsListEntry& rConflictEntry );
    void                SetActionString(const ScChangeAction* pAction, ScDocument* pDoc, const weld::TreeIter& rEntry);
    void                HandleListBoxSelection();

    void                SetConflictAction(const weld::TreeIter& rRootEntry, ScConflictAction eConflictAction);
    void                KeepHandler( bool bMine );
    void                KeepAllHandler( bool bMine );

    DECL_LINK( SelectHandle, weld::TreeView&, void );
    DECL_LINK( UpdateSelectionHdl, Timer*, void );
    DECL_LINK( KeepMineHandle, weld::Button&, void );
    DECL_LINK( KeepOtherHandle, weld::Button&, void );
    DECL_LINK( KeepAllMineHandle, weld::Button&, void );
    DECL_LINK( KeepAllOthersHandle, weld::Button&, void );

public:
    ScConflictsDlg(weld::Window* pParent, ScViewData* pViewData, ScDocument* pSharedDoc, ScConflictsList& rConflictsList);
    virtual ~ScConflictsDlg() override;

    void                UpdateView();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
