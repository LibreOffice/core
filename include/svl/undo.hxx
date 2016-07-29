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
#ifndef INCLUDED_SVL_UNDO_HXX
#define INCLUDED_SVL_UNDO_HXX

#include <svl/svldllapi.h>
#include <rtl/ustring.hxx>

#include <limits>
#include <memory>

struct MarkedUndoAction;

class SVL_DLLPUBLIC SfxRepeatTarget
{
public:
    virtual             ~SfxRepeatTarget() = 0;
};


class SVL_DLLPUBLIC SfxUndoContext
{
public:
    virtual             ~SfxUndoContext() = 0;
};


class SVL_DLLPUBLIC SfxUndoAction
{
public:
                            SfxUndoAction();
    virtual                 ~SfxUndoAction();

    virtual void            Undo();
    virtual void            UndoWithContext( SfxUndoContext& i_context );
    virtual void            Redo();
    virtual void            RedoWithContext( SfxUndoContext& i_context );
    virtual void            Repeat(SfxRepeatTarget&);
    virtual bool            CanRepeat(SfxRepeatTarget&) const;

    virtual bool            Merge( SfxUndoAction *pNextAction );

    virtual OUString    GetComment() const;
    virtual OUString    GetRepeatComment(SfxRepeatTarget&) const;
    virtual sal_uInt16  GetId() const;
    /// ID of the view shell that created this undo action.
    virtual sal_Int32 GetViewShellId() const;
    virtual void dumpAsXml(struct _xmlTextWriter* pWriter) const;

private:
    SfxUndoAction( const SfxUndoAction& ) = delete;
    SfxUndoAction& operator=( const SfxUndoAction& ) = delete;
};


/// is a mark on the Undo stack
typedef sal_Int32 UndoStackMark;
#define MARK_INVALID    ::std::numeric_limits< UndoStackMark >::max()

class SVL_DLLPUBLIC SfxUndoActions
{
    struct Impl;
    std::unique_ptr<Impl> mpImpl;

public:
    SfxUndoActions();
    SfxUndoActions( const SfxUndoActions& r );
    ~SfxUndoActions();

    bool empty() const;
    size_t size() const;

    const MarkedUndoAction& operator[]( size_t i ) const;
    MarkedUndoAction& operator[]( size_t i );

    const SfxUndoAction* GetUndoAction( size_t i ) const;
    SfxUndoAction* GetUndoAction( size_t i );

    void Remove( size_t i_pos );
    void Remove( size_t i_pos, size_t i_count );
    void Insert( SfxUndoAction* i_action, size_t i_pos );
};

/** do not make use of these implementation details, unless you
    really really have to! */
struct SVL_DLLPUBLIC SfxUndoArray
{
    SfxUndoActions          aUndoActions;
    size_t                  nMaxUndoActions;
    size_t                  nCurUndoAction;
    SfxUndoArray            *pFatherUndoArray;
                            SfxUndoArray(size_t nMax=0):
                                nMaxUndoActions(nMax), nCurUndoAction(0),
                                pFatherUndoArray(nullptr) {}
    virtual ~SfxUndoArray();
};


/** do not make use of these implementation details, unless you
    really really have to! */
class SVL_DLLPUBLIC SfxListUndoAction : public SfxUndoAction, public SfxUndoArray

/*  [Explanation]

    UndoAction to composite multiple Undos in one UndoAction.
    These actions are used by SfxUndomanager. With < SfxUndoManager::EnterListAction >
    you can go one composite level down and with < SfxUndoManager::LeaveListAction > up again.
    Redo and Undo work element wise on SfxListUndoActions.
*/
{
    struct Impl;
    std::unique_ptr<Impl> mpImpl;

public:

    SfxListUndoAction(
        const OUString &rComment, const OUString& rRepeatComment, sal_uInt16 nId, SfxUndoArray *pFather );
    virtual ~SfxListUndoAction();

    virtual void            Undo() override;
    virtual void            UndoWithContext( SfxUndoContext& i_context ) override;
    virtual void            Redo() override;
    virtual void            RedoWithContext( SfxUndoContext& i_context ) override;
    virtual void            Repeat(SfxRepeatTarget&) override;
    virtual bool            CanRepeat(SfxRepeatTarget&) const override;

    virtual bool            Merge( SfxUndoAction *pNextAction ) override;

    virtual OUString        GetComment() const override;
    virtual OUString        GetRepeatComment(SfxRepeatTarget&) const override;
    virtual sal_uInt16      GetId() const override;

    void SetComment(const OUString& rComment);
    void dumpAsXml(struct _xmlTextWriter* pWriter) const override;
};


/**  is a callback interface for notifications about state changes of an SfxUndoManager
*/
class SAL_NO_VTABLE SfxUndoListener
{
public:
    virtual void actionUndone( const OUString& i_actionComment ) = 0;
    virtual void actionRedone( const OUString& i_actionComment ) = 0;
    virtual void undoActionAdded( const OUString& i_actionComment ) = 0;
    virtual void cleared() = 0;
    virtual void clearedRedo() = 0;
    virtual void resetAll() = 0;
    virtual void listActionEntered( const OUString& i_comment ) = 0;
    virtual void listActionLeft( const OUString& i_comment ) = 0;
    virtual void listActionCancelled() = 0;
    virtual void undoManagerDying() = 0;

protected:
    ~SfxUndoListener() {}
};


namespace svl
{
    class SAL_NO_VTABLE SAL_DLLPUBLIC_RTTI IUndoManager
    {
    public:
        static bool const CurrentLevel = true;
        static bool const TopLevel = false;

        virtual                 ~IUndoManager() { };

        virtual void            SetMaxUndoActionCount( size_t nMaxUndoActionCount ) = 0;

        virtual void            AddUndoAction( SfxUndoAction *pAction, bool bTryMerg=false ) = 0;

        virtual size_t          GetUndoActionCount( bool const i_currentLevel = CurrentLevel ) const = 0;
        virtual sal_uInt16      GetUndoActionId() const = 0;
        virtual OUString        GetUndoActionComment( size_t nNo=0, bool const i_currentLevel = CurrentLevel ) const = 0;
        virtual SfxUndoAction*  GetUndoAction( size_t nNo=0 ) const = 0;

        virtual size_t          GetRedoActionCount( bool const i_currentLevel = CurrentLevel ) const = 0;
        virtual OUString        GetRedoActionComment( size_t nNo=0, bool const i_currentLevel = CurrentLevel ) const = 0;
        virtual SfxUndoAction*  GetRedoAction() const = 0;

        virtual bool            Undo() = 0;
        virtual bool            Redo() = 0;

        /** clears both the Redo and the Undo stack.

            Will assert and bail out when called while within a list action (<member>IsInListAction</member>).
        */
        virtual void            Clear() = 0;

        /** clears the Redo stack.

            Will assert and bail out when called while within a list action (<member>IsInListAction</member>).
        */
        virtual void            ClearRedo() = 0;

        /** leaves any possible open list action (<member>IsInListAction</member>), and clears both the Undo and the
            Redo stack.

            Effectively, calling this method is equivalent to <code>while ( IsInListAction() ) LeaveListAction();</code>,
            followed by <code>Clear()</code>. The only difference to this calling sequence is that Reset is an
            atomic operation, also resulting in only one notification.
        */
        virtual void            Reset() = 0;

        /** determines whether an Undo or Redo is currently running
        */
        virtual bool            IsDoing() const = 0;

        virtual size_t          GetRepeatActionCount() const = 0;
        virtual OUString        GetRepeatActionComment( SfxRepeatTarget &rTarget) const = 0;
        virtual bool            Repeat( SfxRepeatTarget &rTarget ) = 0;
        virtual bool            CanRepeat( SfxRepeatTarget &rTarget ) const = 0;

        virtual void            EnterListAction(const OUString &rComment, const OUString& rRepeatComment, sal_uInt16 nId=0) = 0;

        /** leaves the list action entered with EnterListAction
            @return the number of the sub actions in the list which has just been left. Note that in case no such
                actions exist, the list action does not contribute to the Undo stack, but is silently removed.
        */
        virtual size_t          LeaveListAction() = 0;

        /** leaves the list action entered with EnterListAction, and forcefully merges the previous
            action on the stack into the newly created list action.

            Say you have an Undo action A on the stack, then call EnterListAction, followed by one or more calls to
            AddUndoAction, followed by a call to LeaveAndMergeListAction. In opposite to LeaveListAction, your Undo
            stack will now still contain one undo action: the newly created list action, whose first child is the
            original A, whose other children are those you added via AddUndoAction, and whose comment is the same as
            the comment of A.

            Effectively, this means that all actions added between EnterListAction and LeaveAndMergeListAction are
            hidden from the user.

            @return the number of the sub actions in the list which has just been left. Note that in case no such
                actions exist, the list action does not contribute to the Undo stack, but is silently removed.
        */
        virtual size_t          LeaveAndMergeListAction() = 0;

        /// determines whether we're within a ListAction context, i.e. a LeaveListAction/LeaveAndMergeListAction call is pending
        virtual bool            IsInListAction() const = 0;

        /// determines how many nested list actions are currently open
        virtual size_t          GetListActionDepth() const = 0;

        /** clears the redo stack and removes the top undo action */
        virtual void            RemoveLastUndoAction() = 0;

        /** enables (true) or disables (false) recording of undo actions

            If undo actions are added while undo is disabled, they are deleted.
            Disabling undo does not clear the current undo buffer!

            Multiple calls to <code>EnableUndo</code> are not cumulative. That is, calling <code>EnableUndo( false )</code>
            twice, and then calling <code>EnableUndo( true )</code> means that Undo is enable afterwards.
        */
        virtual void            EnableUndo( bool bEnable ) = 0;

        // returns true if undo is currently enabled
        // This returns false if undo was disabled using EnableUndo( false ) and
        // also during the runtime of the Undo() and Redo() methods.
        virtual bool            IsUndoEnabled() const = 0;

        /// adds a new listener to be notified about changes in the UndoManager's state
        virtual void            AddUndoListener( SfxUndoListener& i_listener ) = 0;
        virtual void            RemoveUndoListener( SfxUndoListener& i_listener ) = 0;
   };
}


namespace svl { namespace undo { namespace impl
{
    class UndoManagerGuard;
    class LockGuard;
} } }

struct SfxUndoManager_Data;
class SVL_DLLPUBLIC SfxUndoManager : public ::svl::IUndoManager
{
    std::unique_ptr< SfxUndoManager_Data >
                            m_xData;
public:
                            SfxUndoManager( size_t nMaxUndoActionCount = 20 );
    virtual                 ~SfxUndoManager();

    // IUndoManager overridables
    virtual void            SetMaxUndoActionCount( size_t nMaxUndoActionCount ) override;
    virtual void            AddUndoAction( SfxUndoAction *pAction, bool bTryMerg=false ) override;
    virtual size_t          GetUndoActionCount( bool const i_currentLevel = CurrentLevel ) const override;
    virtual sal_uInt16      GetUndoActionId() const override;
    virtual OUString        GetUndoActionComment( size_t nNo=0, bool const i_currentLevel = CurrentLevel ) const override;
    virtual SfxUndoAction*  GetUndoAction( size_t nNo=0 ) const override;
    virtual size_t          GetRedoActionCount( bool const i_currentLevel = CurrentLevel ) const override;
    virtual OUString        GetRedoActionComment( size_t nNo=0, bool const i_currentLevel = CurrentLevel ) const override;
    virtual SfxUndoAction*  GetRedoAction() const override;
    virtual bool            Undo() override;
    virtual bool            Redo() override;
    virtual void            Clear() override;
    virtual void            ClearRedo() override;
    virtual void            Reset() override;
    virtual bool            IsDoing() const override;
    virtual size_t          GetRepeatActionCount() const override;
    virtual OUString        GetRepeatActionComment( SfxRepeatTarget &rTarget) const override;
    virtual bool            Repeat( SfxRepeatTarget &rTarget ) override;
    virtual bool            CanRepeat( SfxRepeatTarget &rTarget ) const override;
    virtual void            EnterListAction(const OUString &rComment, const OUString& rRepeatComment, sal_uInt16 nId=0) override;
    virtual size_t          LeaveListAction() override;
    virtual size_t          LeaveAndMergeListAction() override;
    virtual bool            IsInListAction() const override;
    virtual size_t          GetListActionDepth() const override;
    virtual void            RemoveLastUndoAction() override;
    virtual void            EnableUndo( bool bEnable ) override;
    virtual bool            IsUndoEnabled() const override;
    virtual void            AddUndoListener( SfxUndoListener& i_listener ) override;
    virtual void            RemoveUndoListener( SfxUndoListener& i_listener ) override;

    /** marks the current top-level element of the Undo stack, and returns a unique ID for it
    */
    UndoStackMark   MarkTopUndoAction();

    /** removes a mark given by its ID.
        After the call, the mark ID is invalid.
    */
    void            RemoveMark( UndoStackMark const i_mark );

    /** determines whether the top action on the Undo stack has a given mark
    */
    bool            HasTopUndoActionMark( UndoStackMark const i_mark );

    /** removes the oldest Undo actions from the stack
    */
    void            RemoveOldestUndoAction();

    void dumpAsXml(struct _xmlTextWriter* pWriter) const;

protected:
    bool    UndoWithContext( SfxUndoContext& i_context );
    bool    RedoWithContext( SfxUndoContext& i_context );

    void    ImplClearRedo_NoLock( bool const i_currentLevel );

    /** clears all undo actions on the current level, plus all undo actions on superordinate levels,
        as soon as those levels are reached.

        If no list action is active currently, i.e. we're on the top level already, this method is equivalent to
        ->Clear.

        Otherwise, the Undo actions on the current level are removed. Upon leaving the current list action, all
        undo actions on the then-current level are removed, too. This is continued until the top level is reached.
    */
    void    ClearAllLevels();

private:
    size_t  ImplLeaveListAction( const bool i_merge, ::svl::undo::impl::UndoManagerGuard& i_guard );
    bool    ImplAddUndoAction_NoNotify( SfxUndoAction* pAction, bool bTryMerge, bool bClearRedo, ::svl::undo::impl::UndoManagerGuard& i_guard );
    void    ImplClearRedo( ::svl::undo::impl::UndoManagerGuard& i_guard, bool const i_currentLevel );
    void    ImplClearUndo( ::svl::undo::impl::UndoManagerGuard& i_guard );
    void    ImplClearCurrentLevel_NoNotify( ::svl::undo::impl::UndoManagerGuard& i_guard );
    size_t  ImplGetRedoActionCount_Lock( bool const i_currentLevel = CurrentLevel ) const;
    bool    ImplIsUndoEnabled_Lock() const;
    bool    ImplIsInListAction_Lock() const;
    void    ImplEnableUndo_Lock( bool const i_enable );

    bool ImplUndo( SfxUndoContext* i_contextOrNull );
    bool ImplRedo( SfxUndoContext* i_contextOrNull );

    friend class ::svl::undo::impl::LockGuard;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
