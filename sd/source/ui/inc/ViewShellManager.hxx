/*************************************************************************
 *
 *  $RCSfile: ViewShellManager.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:05:02 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
class VclWindowEvent;

namespace sd {

class ViewShell;
class ViewShellBase;
class ViewShellCache;

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
    ~ViewShellManager (void);

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

    /** Move the specified view shell to the top most position on the stack
        of view shells in relation to the other view shells.  After this the
        only shells that are higher on the stack are its object bars.

        <p>Call this method after a focus change to bring a view mode view
        shell and ist associated tool bar shells to the top of the
        stack.</p>

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

    /** Prevent updates of the shell stack.  While the sub shell manager is
        locked it will update its internal data structures but not alter the
        shell stack.  Use this method when there are several modifications
        to the shell stack to prevent multiple rebuilds of the shell stack
        and resulting broadcasts.
    */
    void LockUpdate (void);

    /** Allow updates of the shell stack.  This method has to be called the
        same number of times as LockUpdate() to really allow a rebuild of
        the shell stack.
        */
    void UnlockUpdate (void);

    ViewShellBase& GetViewShellBase (void) const;

    /** Deactivate the view shell manager so that it pretty much ignores all
        subsequent calls.  Use this when the owner of the view shell manager
        is about being destroyed but the view shell manager itself can not
        yet be deleted.
    */
    void ShutDown (void);

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

    /** Use this class to safly lock updates of the view shell stack.
    */
    class UpdateLocker
    {
    public:
        UpdateLocker (ViewShellManager& rManager);
        ~UpdateLocker (void);
    private:
        ViewShellManager&mrManager;
    };

private:
    ViewShellBase& mrBase;

    ::std::auto_ptr<ViewShellFactory> mpDefaultFactory;
    class SpecializedFactoryList;
    ::std::auto_ptr<SpecializedFactoryList> mpSpecializedFactories;
    class ActiveShellList;
    ::std::auto_ptr<ActiveShellList> mpActiveViewShells;
    ::std::auto_ptr<ViewShellCache> mpCache;

    int mnUpdateLockCount;

    /** When this flag is set then the main view shell is always kept at the
        top of the shell stack.
    */
    bool mbKeepMainViewShellOnTop;

    /** This flag is <TRUE/> until ShutDown() is called.  When it its value
        is <FALSE/> then the internal structures may be updated by
        subsequent method calls but no outside calls take place anymore.
    */
    bool mbIsValid;

    void GatherActiveShells (::std::vector<SfxShell*>& aShellsList);

    void TakeShellsFromStack (void);

    /** This method rebuilds the stack of shells that are stacked upon the
        view shell base.
    */
    void PushShellsOnStack (void);

    DECL_LINK(WindowEventHandler, VclWindowEvent*);
};





} // end of namespace sd

#endif

