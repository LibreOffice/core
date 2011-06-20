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

#ifndef SD_VIEW_SHELL_IMPLEMENTATION_HXX
#define SD_VIEW_SHELL_IMPLEMENTATION_HXX

#include "ViewShell.hxx"
#include "ViewShellManager.hxx"
#include "ToolBarManager.hxx"

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <memory>


class SvxIMapDlg;

namespace sd {

class DrawController;

/** This class contains (will contain) the implementation of methods that
    have not be accessible from the outside.
*/
class ViewShell::Implementation
{
public:
    bool mbIsShowingUIControls;
    bool mbIsMainViewShell;
    /// Set to true when the ViewShell::Init() method has been called.
    bool mbIsInitialized;
    /** Set to true while ViewShell::ArrangeGUIElements() is being
        executed.  It is used as guard against recursive execution.
    */
    bool mbArrangeActive;

    /** Remember a link to the sub shell factory, so that it can be
        unregistered at the ViewShellManager when a ViewShell is deleted.
    */
    ViewShellManager::SharedShellFactory mpSubShellFactory;

    /** This update lock for the ToolBarManager exists in order to avoid
        problems with tool bars being displayed while the mouse button is
        pressed.  Whith docked tool bars this can lead to a size change of
        the view.  This would change the relative mouse coordinates and thus
        interpret every mouse click as move command.
    */
    class ToolBarManagerLock
    {
    public:
        /** Create a new instance.  This allows the mpSelf member to be set
            automatically.
        */
        static ::boost::shared_ptr<ToolBarManagerLock> Create (
            const ::boost::shared_ptr<ToolBarManager>& rpManager);
        /** Release the lock.  When the UI is captured
            (Application::IsUICaptured() returns <TRUE/>) then the lock is
            released later asynchronously.
            @param bForce
                When this flag is <TRUE/> then the lock is released even
                when IsUICaptured() returns <TRUE/>.
        */
        void Release (bool bForce = false);
        DECL_LINK(TimeoutCallback,Timer*);
    private:
        ::std::auto_ptr<ToolBarManager::UpdateLock> mpLock;
        /** The timer is used both as a safe guard to unlock the update lock
            when Release() is not called explicitly.  It is also used to
            defer the release of the lock to a time when the UI is not
            captured.
        */
        Timer maTimer;
        /** The shared_ptr to this allows the ToolBarManagerLock to control
            its own lifetime.  This, of course, does work only when no one
            holds another shared_ptr longer than only temporary.
        */
        ::boost::shared_ptr<ToolBarManagerLock> mpSelf;
        ToolBarManagerLock (const ::boost::shared_ptr<sd::ToolBarManager>& rpManager);
        ~ToolBarManagerLock (void);

        class Deleter;
        friend class Deleter;
    };
    // The member is not an auto_ptr because it takes over its own life time
    // control.
    ::boost::weak_ptr<ToolBarManagerLock> mpUpdateLockForMouse;

    Implementation (ViewShell& rViewShell);
    ~Implementation (void);

    /** Process the SID_MODIFY slot.
    */
    void ProcessModifyPageSlot (
        SfxRequest& rRequest,
        SdPage* pCurrentPage,
        PageKind ePageKind);

    /** Assign the given layout to the given page.  This method is at the
        moment merely a front end for ProcessModifyPageSlot.
        @param pPage
            If a NULL pointer is given then this call is ignored.
    */
    void AssignLayout ( SfxRequest& rRequest, PageKind ePageKind );

    /** Determine the view id of the view shell.  This corresponds to the
        view id stored in the SfxViewFrame class.

        We can not use the view of that class because with the introduction
        of the multi pane GUI we do not switch the SfxViewShell anymore when
        switching the view in the center pane.  The view id of the
        SfxViewFrame is thus not modified and we can not set it from the
        outside.

        The view id is still needed for the SFX to determine on start up
        (e.g. after loading a document) which ViewShellBase sub class to
        use.  These sub classes--like OutlineViewShellBase--exist only to be
        used by the SFX as factories.  They only set the initial pane
        configuration, nothing more.

        So what we do here in essence is to return on of the
        ViewShellFactoryIds that can be used to select the factory that
        creates the ViewShellBase subclass with the initial pane
        configuration that has in the center pane a view shell of the same
        type as mrViewShell.
    */
    sal_uInt16 GetViewId (void);

    /** Return a pointer to the image map dialog that is displayed in some
        child window.
        @return
            Returns <NULL/> when the image map dialog is not available.
    */
    static SvxIMapDlg* GetImageMapDialog (void);

private:
    ViewShell& mrViewShell;
};


} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
