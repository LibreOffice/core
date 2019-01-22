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

#ifndef INCLUDED_SD_SOURCE_UI_INC_VIEWSHELLIMPLEMENTATION_HXX
#define INCLUDED_SD_SOURCE_UI_INC_VIEWSHELLIMPLEMENTATION_HXX

#include "ViewShell.hxx"
#include "ViewShellManager.hxx"
#include "ToolBarManager.hxx"
#include <o3tl/deleter.hxx>
#include <memory>

class SvxIMapDlg;

namespace sd {

/** This class contains (will contain) the implementation of methods that
    have not be accessible from the outside.
*/
class ViewShell::Implementation
{
public:
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
        pressed.  With docked tool bars this can lead to a size change of
        the view.  This would change the relative mouse coordinates and thus
        interpret every mouse click as move command.
    */
    class ToolBarManagerLock
    {
    public:
        /** Create a new instance.  This allows the mpSelf member to be set
            automatically.
        */
        static std::shared_ptr<ToolBarManagerLock> Create (
            const std::shared_ptr<ToolBarManager>& rpManager);
        /** Release the lock.  When the UI is captured
            (Application::IsUICaptured() returns <TRUE/>) then the lock is
            released later asynchronously.
            @param bForce
                When this flag is <TRUE/> then the lock is released even
                when IsUICaptured() returns <TRUE/>.
        */
        void Release (bool bForce = false);
        DECL_LINK(TimeoutCallback, Timer *, void);
    private:
        ::std::unique_ptr<ToolBarManager::UpdateLock, o3tl::default_delete<ToolBarManager::UpdateLock>> mpLock;
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
        std::shared_ptr<ToolBarManagerLock> mpSelf;
        ToolBarManagerLock (const std::shared_ptr<sd::ToolBarManager>& rpManager);
        ~ToolBarManagerLock();

        class Deleter;
        friend class Deleter;
    };
    // The member is not a unique_ptr because it takes over its own life time
    // control.
    std::weak_ptr<ToolBarManagerLock> mpUpdateLockForMouse;

    Implementation (ViewShell& rViewShell);
    ~Implementation() COVERITY_NOEXCEPT_FALSE;

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
    void AssignLayout ( SfxRequest const & rRequest, PageKind ePageKind );

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

        So what we do here in essence is to return one of the
        ViewShellFactoryIds that can be used to select the factory that
        creates the ViewShellBase subclass with the initial pane
        configuration that has in the center pane a view shell of the same
        type as mrViewShell.
    */
    SfxInterfaceId GetViewId();

    /** Return a pointer to the image map dialog that is displayed in some
        child window.
        @return
            Returns <NULL/> when the image map dialog is not available.
    */
    static SvxIMapDlg* GetImageMapDialog();

private:
    ViewShell& mrViewShell;
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
