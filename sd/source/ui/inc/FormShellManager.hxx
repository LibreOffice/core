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

#ifndef SD_FORM_SHELL_MANAGER_HXX
#define SD_FORM_SHELL_MANAGER_HXX

#include <ViewShellManager.hxx>

#include <tools/link.hxx>
#include <svl/lstner.hxx>

class VclWindowEvent;
class FmFormShell;

namespace sd { namespace tools { class EventMultiplexerEvent; } }

namespace sd {

class PaneManagerEvent;
class ViewShell;
class ViewShellBase;

/** This simple class is responsible for putting the form shell above or
    below the main view shell on the shell stack maintained by the ObjectBarManager.

    The form shell is moved above the view shell when the form shell is
    activated, i.e. the FormControlActivated handler is called.

    It is moved below the view shell when the main window of the
    main view shell is focused.

    The form shell is created and destroyed by the ViewShellManager by using
    a factory object provided by the FormShellManager.
*/
class FormShellManager
    : public SfxListener
{
public:
    FormShellManager (ViewShellBase& rBase);
    virtual ~FormShellManager (void);

    /** Typically called by a ShellFactory.  It tells the
        FormShellManager which form shell to manage.
        @param pFormShell
            This may be <NULL/> to disconnect the ViewShellManager from the
            form shell.
    */
    void SetFormShell (FmFormShell* pFormShell);

    /** Return the form shell last set with SetFormShell().
        @return
            The result may be <NULL/> when the SetFormShell() method has not
            yet been called or was last called with <NULL/>.
    */
    FmFormShell* GetFormShell (void);

private:
    ViewShellBase& mrBase;

    /** Ownership of the form shell lies with the ViewShellManager.  This
        reference is kept so that the FormShellManager can detect when a new
        form shell is passed to SetFormShell().
    */
    FmFormShell* mpFormShell;

    /** Remember whether the form shell is currently above or below the main
        view shell.
    */
    bool mbFormShellAboveViewShell;

    /** The factory is remembered so that it removed from the
        ViewShellManager when the FormShellManager is destroyed.
    */
    ViewShellManager::SharedShellFactory mpSubShellFactory;

    bool mbIsMainViewChangePending;

    ::Window* mpMainViewShellWindow;

    /** Register at window of center pane and at the form shell that
        represents the form tool bar.  The former informs this manager about
        the deselection of the form shell.  The later informs about its
        selection.
    */
    void RegisterAtCenterPane (void);

    /** Unregister the listeners that were registered in
        RegisterAtCenterPane().
    */
    void UnregisterAtCenterPane (void);

    /** This call back is called by the application window (among others)
        when the window gets the focus.  In this case the form shell is
        moved to the bottom of the shell stack.
    */
    DECL_LINK(WindowEventHandler, VclWindowEvent*);

    /** This call back is called when view in the center pane is replaced.
        When this happens then we unregister at the window of the old and
        register at the window of the new shell.
    */
    DECL_LINK(ConfigurationUpdateHandler, ::sd::tools::EventMultiplexerEvent*);

    /** This call back is called by the form shell when it gets the focus.
        In this case the form shell is moved to the top of the shell stack.
    */
    DECL_LINK(FormControlActivated, FmFormShell*);

    /** This method is called by the form shell when that is destroyed.  It
        acts as a last resort against referencing a dead form shell.  With
        the factory working properly this method should not be necessary
        (and may be removed in the future.)
    */
    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint);
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
