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

#ifndef SC_CONFLICTSDLG_HXX
#define SC_CONFLICTSDLG_HXX

#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <svx/ctredlin.hxx>

#include "chgtrack.hxx"
#include "docsh.hxx"

class ScViewData;


//=============================================================================

enum ScConflictAction
{
    SC_CONFLICT_ACTION_NONE,
    SC_CONFLICT_ACTION_KEEP_MINE,
    SC_CONFLICT_ACTION_KEEP_OTHER
};

typedef ::std::vector< sal_uLong > ScChangeActionList;


//=============================================================================
// struct ScConflictsListEntry
//=============================================================================

struct ScConflictsListEntry
{
    ScConflictAction    meConflictAction;
    ScChangeActionList  maSharedActions;
    ScChangeActionList  maOwnActions;

    bool                HasSharedAction( sal_uLong nSharedAction ) const;
    bool                HasOwnAction( sal_uLong nOwnAction ) const;
};


//=============================================================================

typedef ::std::vector< ScConflictsListEntry > ScConflictsList;


//=============================================================================
// class ScConflictsListHelper
//=============================================================================

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


//=============================================================================
// class ScConflictsFinder
//=============================================================================

class ScConflictsFinder
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
    virtual                 ~ScConflictsFinder();

    bool                    Find();
};


//=============================================================================
// class ScConflictsResolver
//=============================================================================

class ScConflictsResolver
{
private:
    ScChangeTrack*      mpTrack;
    ScConflictsList&    mrConflictsList;

public:
                        ScConflictsResolver( ScChangeTrack* pTrack, ScConflictsList& rConflictsList );
    virtual             ~ScConflictsResolver();

    void                HandleAction( ScChangeAction* pAction, bool bIsSharedAction,
                            bool bHandleContentAction, bool bHandleNonContentAction );
};


//=============================================================================
// class ScConflictsDlg
//=============================================================================

class ScConflictsDlg : public ModalDialog
{
private:
    FixedText           maFtConflicts;
    SvxSimpleTableContainer m_aLbConflictsContainer;
    SvxRedlinTable      maLbConflicts;
    PushButton          maBtnKeepMine;
    PushButton          maBtnKeepOther;
    FixedLine           maFlConflicts;
    PushButton          maBtnKeepAllMine;
    PushButton          maBtnKeepAllOthers;
    CancelButton        maBtnCancel;
    HelpButton          maBtnHelp;

    String              maStrTitleConflict;
    String              maStrTitleAuthor;
    String              maStrTitleDate;
    String              maStrUnknownUser;

    ScViewData*         mpViewData;
    ScDocument*         mpOwnDoc;
    ScChangeTrack*      mpOwnTrack;
    ScDocument*         mpSharedDoc;
    ScChangeTrack*      mpSharedTrack;
    ScConflictsList&    mrConflictsList;
    Size                maDialogSize;

    Timer               maSelectionTimer;
    bool                mbInSelectHdl;
    bool                mbInDeselectHdl;

    String              GetConflictString( const ScConflictsListEntry& rConflictEntry );
    String              GetActionString( const ScChangeAction* pAction, ScDocument* pDoc );
    void                HandleListBoxSelection( bool bSelectHandle );

    void                SetConflictAction( SvTreeListEntry* pRootEntry, ScConflictAction eConflictAction );
    void                KeepHandler( bool bMine );
    void                KeepAllHandler( bool bMine );

    DECL_LINK( SelectHandle, void* );
    DECL_LINK( DeselectHandle, void* );
    DECL_LINK( UpdateSelectionHdl, void* );
    DECL_LINK( KeepMineHandle, void* );
    DECL_LINK( KeepOtherHandle, void* );
    DECL_LINK( KeepAllMineHandle, void* );
    DECL_LINK( KeepAllOthersHandle, void* );

public:
                        ScConflictsDlg( Window* pParent, ScViewData* pViewData, ScDocument* pSharedDoc, ScConflictsList& rConflictsList );
                        ~ScConflictsDlg();

    virtual void        Resize();
    void                UpdateView();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
