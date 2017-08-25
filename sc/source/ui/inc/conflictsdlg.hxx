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

#ifndef INCLUDED_SC_SOURCE_UI_INC_CONFLICTSDLG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_CONFLICTSDLG_HXX

#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/idle.hxx>
#include <svx/ctredlin.hxx>

#include "chgtrack.hxx"
#include "docsh.hxx"

class ScViewData;

enum ScConflictAction
{
    SC_CONFLICT_ACTION_NONE,
    SC_CONFLICT_ACTION_KEEP_MINE,
    SC_CONFLICT_ACTION_KEEP_OTHER
};

typedef ::std::vector< sal_uLong > ScChangeActionList;

// struct ScConflictsListEntry

struct ScConflictsListEntry
{
    ScConflictAction    meConflictAction;
    ScChangeActionList  maSharedActions;
    ScChangeActionList  maOwnActions;

    bool                HasSharedAction( sal_uLong nSharedAction ) const;
    bool                HasOwnAction( sal_uLong nOwnAction ) const;
};

typedef ::std::vector< ScConflictsListEntry > ScConflictsList;

// class ScConflictsListHelper

class ScConflictsListHelper
{
private:
    static void                     Transform_Impl( ScChangeActionList& rActionList, ScChangeActionMergeMap* pMergeMap );

public:
    static bool                     HasOwnAction( ScConflictsList& rConflictsList, sal_uLong nOwnAction );

    static ScConflictsListEntry*    GetSharedActionEntry( ScConflictsList& rConflictsList, sal_uLong nSharedAction );
    static ScConflictsListEntry*    GetOwnActionEntry( ScConflictsList& rConflictsList, sal_uLong nOwnAction );

    static void                     TransformConflictsList( ScConflictsList& rConflictsList,
                                        ScChangeActionMergeMap* pSharedMap, ScChangeActionMergeMap* pOwnMap );
};

// class ScConflictsFinder

class ScConflictsFinder final
{
private:
    ScChangeTrack*          mpTrack;
    sal_uLong                   mnStartShared;
    sal_uLong                   mnEndShared;
    sal_uLong                   mnStartOwn;
    sal_uLong                   mnEndOwn;
    ScConflictsList&        mrConflictsList;

    static bool             DoActionsIntersect( const ScChangeAction* pAction1, const ScChangeAction* pAction2 );
    ScConflictsListEntry*   GetIntersectingEntry( const ScChangeAction* pAction ) const;
    ScConflictsListEntry*   GetEntry( sal_uLong nSharedAction, const ScChangeActionList& rOwnActions );

public:
                            ScConflictsFinder( ScChangeTrack* pTrack, sal_uLong nStartShared, sal_uLong nEndShared,
                                sal_uLong nStartOwn, sal_uLong nEndOwn, ScConflictsList& rConflictsList );
                            ~ScConflictsFinder();

    bool                    Find();
};

// class ScConflictsResolver

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

// class ScConflictsDlg

class ScConflictsDlg : public ModalDialog
{
private:
    VclPtr<SvSimpleTableContainer> m_pLbConflictsContainer;
    VclPtr<SvxRedlinTable>      m_pLbConflicts;
    VclPtr<PushButton>          m_pBtnKeepMine;
    VclPtr<PushButton>          m_pBtnKeepOther;
    VclPtr<PushButton>          m_pBtnKeepAllMine;
    VclPtr<PushButton>          m_pBtnKeepAllOthers;

    OUString            maStrTitleConflict;
    OUString            maStrTitleAuthor;
    OUString            maStrTitleDate;
    OUString            maStrUnknownUser;

    ScViewData*         mpViewData;
    ScDocument*         mpOwnDoc;
    ScChangeTrack*      mpOwnTrack;
    ScDocument*         mpSharedDoc;
    ScChangeTrack*      mpSharedTrack;
    ScConflictsList&    mrConflictsList;
    Size                maDialogSize;

    Idle                maSelectionIdle;
    bool                mbInSelectHdl;
    bool                mbInDeselectHdl;

    OUString            GetConflictString( const ScConflictsListEntry& rConflictEntry );
    OUString            GetActionString( const ScChangeAction* pAction, ScDocument* pDoc );
    void                HandleListBoxSelection( bool bSelectHandle );

    static void         SetConflictAction( const SvTreeListEntry* pRootEntry, ScConflictAction eConflictAction );
    void                KeepHandler( bool bMine );
    void                KeepAllHandler( bool bMine );

    DECL_LINK( SelectHandle, SvTreeListBox*, void );
    DECL_LINK( DeselectHandle, SvTreeListBox*, void );
    DECL_LINK( UpdateSelectionHdl, Timer*, void );
    DECL_LINK( KeepMineHandle, Button*, void );
    DECL_LINK( KeepOtherHandle, Button*, void );
    DECL_LINK( KeepAllMineHandle, Button*, void );
    DECL_LINK( KeepAllOthersHandle, Button*, void );

public:
                        ScConflictsDlg( vcl::Window* pParent, ScViewData* pViewData, ScDocument* pSharedDoc, ScConflictsList& rConflictsList );
                        virtual ~ScConflictsDlg() override;
    virtual void        dispose() override;

    void                UpdateView();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
