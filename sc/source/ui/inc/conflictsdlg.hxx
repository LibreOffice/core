/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
//UNUSED2008-05  static bool                     HasSharedAction( ScConflictsList& rConflictsList, sal_uLong nSharedAction );
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
// class ScConflictsListBox
//=============================================================================

class ScConflictsListBox: public SvxRedlinTable
{
private:

public:
//UNUSED2008-05         ScConflictsListBox( Window* pParent, WinBits nBits = WB_BORDER );
                        ScConflictsListBox( Window* pParent, const ResId& rResId );
                        ~ScConflictsListBox();

//UNUSED2008-05  sal_uLong               GetRootEntryPos( const SvLBoxEntry* pRootEntry ) const;
};

//=============================================================================
// class ScConflictsDlg
//=============================================================================

class ScConflictsDlg : public ModalDialog
{
private:
    FixedText           maFtConflicts;
    ScConflictsListBox  maLbConflicts;
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

    void                SetConflictAction( SvLBoxEntry* pRootEntry, ScConflictAction eConflictAction );
    void                KeepHandler( bool bMine );
    void                KeepAllHandler( bool bMine );

    DECL_LINK( SelectHandle, SvxRedlinTable* );
    DECL_LINK( DeselectHandle, SvxRedlinTable* );
    DECL_LINK( UpdateSelectionHdl, Timer* );
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
