/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ViewShellManager.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:00:13 $
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

#ifndef SD_VIEW_SHELL_MANAGER_HXX
#define SD_VIEW_SHELL_MANAGER_HXX

#include "ShellFactory.hxx"

#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif
#include <memory>
#include <vector>
#include <boost/shared_ptr.hpp>

class FmFormShell;
class SfxShell;
class SfxUndoManager;

namespace sd {

class ViewShell;
class ViewShellBase;

/** The ViewShellManager has the responsibility to manage the view shells
    and sub shells on the SFX shell stack.  They form a two level hierarchy
    (the underlying ViewShellBase, the only true SfxViewShell descendant,
    forms a third level.)  On the first level there are the view shells
    (what formely was called view shell, anyway; nowadays they are derived
    from SfxShell.) and shells for panes. On the second level there are sub
    shells (also derived from SfxShell) that usually are tool bars.

    <p>On the SFX shell stack the regular sub shells are placed above their
    view shells.  The FormShell is a special case.  With the SetFormShell()
    method it can be placed directly above or below one of the view
    shells.</p>

    <p>Shells managed by this class are created by factories or are given
    directly to Activate... methods.  For the sub shells there is one
    factory for every view shell.  Factories are added or removed via the
    (Add|Remove)SubShellFactory() methods.  The FormShell is managed with the
    factory of its view shell.</p>
*/
class ViewShellManager
{
public:
    typedef ::boost::shared_ptr<ShellFactory<SfxShell> > SharedShellFactory;

    ViewShellManager (ViewShellBase& rBase);

    /** Before the destructor is called the method Shutdown() has to have
        been called.
    */
    ~ViewShellManager (void);

    /** Tell a ViewShellManager object to prepare to be deleted, i.e. to
        destroy all of its resources and to ignore all following calls.
        Use this when the owner of the view shell manager is about being
        destroyed but the view shell manager itself can not yet be deleted.
    */
    void Shutdown (void);

    /** Set the factory for sub shells of the specified view shell.
    */
    void AddSubShellFactory (
        ViewShell* pViewShell,
        const SharedShellFactory& rpFactory);
    void RemoveSubShellFactory (
        ViewShell* pViewShell,
        const SharedShellFactory& rpFactory);

    /** Create a new (or possibly recycle an existing) instance of a view
        shell that is specified by the given id.  When called multiple times
        with the same id then multiple instances of the associated view
        shell are created.  Use the returned pointer to distinguish between
        these.
        @param nId
            The id of the shell to activate.
        @return
            A pointer to the activated shell is returned.  Use this pointer
            to identify the shell in other method calls.  When an activation
            is not possible then NULL is returned.
    */
    ViewShell* ActivateViewShell (
        ShellId nId,
        ::Window* pParentWindow,
        FrameView* pFrameView);

    /** Activate the given view shell.
    */
    void ActivateViewShell (ViewShell* pViewShell);

    /** Activate the given shell which is not a view shell.  For view shells
        use the ActivateViewShell() method.
    */
    void ActivateShell (SfxShell* pShell);

    /** Deactivate the specified shell, i.e. take it and all of its
        object bars from the shell stack.
        @param pShell
            The shell to deactivate.
    */
    void DeactivateViewShell (const ViewShell* pShell);

    /** Deactivate the specified shell.  The shell is not destroyed.
    */
    void DeactivateShell (const SfxShell* pShell);

    /** Associate the form shell with a view shell and their relative
        position.  This method does not change the shell stack, it just
        stores the given values for the next shell stack update.
        @param pParentShell
            The view shell of the form shell.
        @param pFormShell
            The form shell.
        @param bAbove
            When <TRUE/> then the form shell will be placed directly above
            pViewShell on the SFX shell stack.  Otherwise the form shell is
            placed directly below the view shell.
    */
    void SetFormShell (const ViewShell* pParentShell, FmFormShell* pFormShell, bool bAbove);

    /** Activate the specified shell as sub shell for the given view shell.
        The sub shell factory associated with the view shell is used to
        create the sub shell.
        @param rParentShell
            The new sub shell will be placed above this view shell.
        @param nId
            This id is used only with the factory registered for the parent
            view shell.
    */
    void ActivateSubShell (const ViewShell& rParentShell, ShellId nId);

    /** Deactivate the specified sub shell.
    */
    void DeactivateSubShell (const ViewShell& rParentShell, ShellId nId);

    /** Deactivate all sub shells of the given view shell.
    */
    void DeactivateAllSubShells (const ViewShell& rParentShell);

    /** Move the specified sub shells to the top position among the sub
        shells of the parent view shell.  The rest of the SFX shell stack
        does not change (but the all shells above the sub shells have to be
        taken once off the stack and are then moved back on again.)
    */
    void MoveSubShellToTop (const ViewShell& rParentShell, ShellId nId);

    /** Send all sub shells of the specified view shell an Invalidate()
        call.  This does not modify the shell stack.
    */
    void InvalidateAllSubShells (
        ViewShell* pViewShell);

    /** Call this method to when a 'secondary' shell is moved to or from the
        stack, e.g. an object bar.  As a result a pending
        TakeShellsFromStack() is executed and at the next UnlockUpdate() to
        lock level 0 the shells are asked about their secondary shells to
        push on the shell stack.
    */
    void InvalidateShellStack (const SfxShell* pShell);

    /** Move the specified view shell to the top most position on the stack
        of view shells in relation to the other view shells.  After this the
        only shells that are higher on the stack are its object bars.

        Call this method after a focus change to bring a view mode view
        shell and ist associated tool bar shells to the top of the
        stack.

        The mbKeepMainViewShellOnTop flag is not obeyed.

        @param nId
            The id of the shell to move to the top.
    */
    void MoveToTop (const ViewShell& rShell);

    /** Return the first, i.e. top most, view shell that has been activated
        under the given id.
        @param nId
            The id of the shell for which to return a pointer.
        @return
            When the specified shell is currently not active then NULL is
            returned.
    */
    SfxShell* GetShell (ShellId nId) const;

    /** Return the top-most shell on the SFX shell stack regardless of
        whether that is a view shell or a sub shell.
    */
    SfxShell* GetTopShell (void) const;

    /** Return the id of the given shell.
    */
    ShellId GetShellId (const SfxShell* pShell) const;

    /** Replace the references to one SfxUndoManager to that of another at all
        shells on the SFX shell stack.  Call this method when an undo
        manager is about to be destroyed.
        @param pManager
            The undo manager to be replaced.
        @param pReplacement
            The undo manager that replaces pManager.
    */
    void ReplaceUndoManager (SfxUndoManager* pManager, SfxUndoManager* pReplacement);

    /** Use this class to safely lock updates of the view shell stack.
    */
    class UpdateLock
    {
    public:
        UpdateLock (const ::boost::shared_ptr<ViewShellManager>& rpManager)
            : mpManager(rpManager) {mpManager->LockUpdate();}
        ~UpdateLock (void) {mpManager->UnlockUpdate();};
    private:
        ::boost::shared_ptr<ViewShellManager> mpManager;
    };
    friend class UpdateLock;

private:
    class Implementation;
    ::std::auto_ptr<ViewShellManager::Implementation> mpImpl;
    bool mbValid;

    void LockUpdate (void);
    void UnlockUpdate (void);
};



} // end of namespace sd

#endif

