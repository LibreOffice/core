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

#ifndef INCLUDED_SD_SOURCE_UI_INC_FORMSHELLMANAGER_HXX
#define INCLUDED_SD_SOURCE_UI_INC_FORMSHELLMANAGER_HXX

#include <ViewShellManager.hxx>

#include <tools/link.hxx>
#include <svl/lstner.hxx>
#include <vcl/vclptr.hxx>

class VclWindowEvent;
class FmFormShell;

namespace sd { namespace tools { class EventMultiplexerEvent; } }

namespace sd {

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
    virtual ~FormShellManager();

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
    FmFormShell* GetFormShell() { return mpFormShell;}

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

    VclPtr<vcl::Window> mpMainViewShellWindow;

    /** Register at window of center pane and at the form shell that
        represents the form tool bar.  The former informs this manager about
        the deselection of the form shell.  The later informs about its
        selection.
    */
    void RegisterAtCenterPane();

    /** Unregister the listeners that were registered in
        RegisterAtCenterPane().
    */
    void UnregisterAtCenterPane();

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
    DECL_LINK_TYPED(FormControlActivated, LinkParamNone*, void);

    /** This method is called by the form shell when that is destroyed.  It
        acts as a last resort against referencing a dead form shell.  With
        the factory working properly this method should not be necessary
        (and may be removed in the future.)
    */
    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) SAL_OVERRIDE;
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
