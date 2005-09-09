/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ViewShellManager.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:20:26 $
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

class SfxShell;

namespace sd {

class ViewShell;
class ViewShellBase;

/** The sub-shell manager has the responsibility to maintain the sub shells
    stacked on a ViewShellBase object.  They form a two level hierarchy.  On
    the first level there are the view shells which are managed directly by
    this class.  On the second level there are the object bars wrapped into
    an SfxShell.  They are managed by ObjectBarManager objects, one for each
    view shell.

    <p>View shell management consists of maintaining the correct order of
    all stacked shells.  Those that belong to the view shell that currently
    has the focus have to be top-most to be considered first by the slot call
    dispatcher.  Object bars have to be on top of their view shells so that
    the associated functions are called first.</p>

    <p>This class uses pointers to the ::sd::ViewShell class instead of its
    ::SfxShell base class because it needs access to the object bar manager
    of the ViewShell class so that that manager can be instructed to
    activate or deactivate its object bars.</p>
*/
class ViewShellManager
{
public:
    typedef ShellFactory<ViewShell> ViewShellFactory;

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

    /** Register the default factory that is called to create a new instance
        of a shell for a given id when there is no factory that has been
        registered specially for that id.
        @param pFactory
            The factory object that is called to create a new shell instance.
    */
    void RegisterDefaultFactory (::std::auto_ptr<ViewShellFactory> pFactory);

    /** Register a factory that is called to create a shell for the
        specified id.  This takes precedence over the default factory.
    */
    void RegisterFactory (
        ShellId nId,
        ::std::auto_ptr<ViewShellFactory> pFactory);

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

    /** Deactivate the specified shell, i.e. take it and all of its
        object bars from the shell stack.
        @param pShell
            The shell to deactivate.
    */
    void DeactivateViewShell (const ViewShell* pShell);

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
    void MoveToTop (const ViewShell* pShell);

    /** Return the first, i.e. top most, view shell that has been activated
        under the given id.
        @param nId
            The id of the shell for which to return a pointer.
        @return
            When the specified shell is currently not active then NULL is
            returned.
    */
    ViewShell* GetShell (ShellId nId);

    /** Return the id of the given shell.
    */
    ShellId GetShellId (const ViewShell* pShell);

    /** Return the ViewShellBase for which objects of this class manage the
        stacked view shells.
    */
    ViewShellBase& GetViewShellBase (void) const;

    /** Use one of the registered factories to create the requested view
        shell.
        @return
            When a view shell of the requested type can not be created,
            e.g. because no factory for it has been registered, NULL is
            returned.
            The returned view shell can be destroyed by the caller.
    */
    ViewShell* CreateViewShell (
        ShellId nShellId,
        ::Window* pParentWindow,
        FrameView* pFrameView);

    class Implementation;

    /** Use this class to safely lock updates of the view shell stack.
    */
    class UpdateLock
    {
    public:
        explicit UpdateLock (ViewShellManager& rManager);
        explicit UpdateLock (Implementation& rManagerImplementation);
        ~UpdateLock (void);
    private:
        ViewShellManager::Implementation& mrManagerImplementation;
    };
    friend class UpdateLock;

private:
    ::std::auto_ptr<ViewShellManager::Implementation> mpImpl;
    bool mbValid;
};



} // end of namespace sd

#endif

