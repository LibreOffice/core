/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: conflictsdlg.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 12:21:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

class ScViewData;


//=============================================================================

enum ScConflictAction
{
    SC_CONFLICT_ACTION_NONE,
    SC_CONFLICT_ACTION_KEEP_MINE,
    SC_CONFLICT_ACTION_KEEP_OTHER
};

typedef ::std::vector< ULONG > ScChangeActionList;


//=============================================================================
// struct ScConflictsListEntry
//=============================================================================

struct ScConflictsListEntry
{
    ScConflictAction    meConflictAction;
    ScChangeActionList  maSharedActions;
    ScChangeActionList  maOwnActions;

    bool                HasSharedAction( ULONG nSharedAction ) const;
    bool                HasOwnAction( ULONG nOwnAction ) const;
};


//=============================================================================

typedef ::std::vector< ScConflictsListEntry > ScConflictsList;


//=============================================================================
// class ScConflictsListHelper
//=============================================================================

class ScConflictsListHelper
{
public:
    static bool                    HasSharedAction( ScConflictsList& rConflictsList, ULONG nSharedAction );
    static bool                    HasOwnAction( ScConflictsList& rConflictsList, ULONG nOwnAction );

    static ScConflictsListEntry*   GetSharedActionEntry( ScConflictsList& rConflictsList, ULONG nSharedAction );
    static ScConflictsListEntry*   GetOwnActionEntry( ScConflictsList& rConflictsList, ULONG nOwnAction );
};


//=============================================================================
// class ScConflictsFinder
//=============================================================================

class ScConflictsFinder
{
private:
    ScChangeTrack*          mpSharedTrack;
    ULONG                   mnStartShared;
    ULONG                   mnEndShared;
    ScChangeTrack*          mpOwnTrack;
    ULONG                   mnStartOwn;
    ULONG                   mnEndOwn;
    ScConflictsList&        mrConflictsList;

    static bool             DoActionsIntersect( const ScChangeAction* pAction1, const ScChangeAction* pAction2 );
    ScConflictsListEntry*   GetIntersectingEntry( const ScChangeAction* pAction ) const;
    ScConflictsListEntry*   GetEntry( ULONG nSharedAction, const ScChangeActionList& rOwnActions );
    void                    RemovePrevContentEntries();

public:
                            ScConflictsFinder( ScChangeTrack* pSharedTrack, ULONG nStartShared, ULONG nEndShared,
                                ScChangeTrack* pOwnTrack, ULONG nStartOwn, ULONG nEndOwn,
                                ScConflictsList& rConflictsList );
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

    void                HandleAction( ScChangeAction* pAction, ULONG nOffset,
                            bool bIsSharedAction, bool bHandleContentAction,
                            bool bHandleNonContentAction );
};


//=============================================================================
// class ScConflictsListBox
//=============================================================================

class ScConflictsListBox: public SvxRedlinTable
{
private:

public:
                        ScConflictsListBox( Window* pParent, WinBits nBits = WB_BORDER );
                        ScConflictsListBox( Window* pParent, const ResId& rResId );
                        ~ScConflictsListBox();

    ULONG               GetRootEntryPos( const SvLBoxEntry* pRootEntry ) const;
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
