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

#ifndef INCLUDED_SD_SOURCE_UI_INC_TASKPANE_TOOLPANELVIEWSHELL_HXX
#define INCLUDED_SD_SOURCE_UI_INC_TASKPANE_TOOLPANELVIEWSHELL_HXX

#include "ViewShell.hxx"
#include "glob.hxx"
#include "taskpane/PanelId.hxx"
#include "framework/FrameworkHelper.hxx"
#include <vcl/button.hxx>
#include <sfx2/shell.hxx>
#include <sfx2/viewfac.hxx>
#include <sfx2/dockwin.hxx>

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

class PopupMenu;

namespace com { namespace sun { namespace star { namespace ui {
    class XUIElement;
} } } }

namespace sd {
namespace toolpanel {
class TaskPaneShellManager;
class ToolPanelViewShell_Impl;
/** The tool panel is a view shell for some very specific reasons:
    - It fits better into the concept of panes being docking windows whose
    content, a view shell, can be exchanged on runtime.
    - A control in the tool panel that wants to show a context menu has to
    do that over the dispatcher of a shell.  These shells, usually
    implemented by the controls themselves, have to be managed by someone.
    If interpreted as object bars this can be handled by the
    ObjectBarManager of the ViewShell.
*/
class ToolPanelViewShell
    : public ViewShell
{
public:
    TYPEINFO_OVERRIDE();
    SFX_DECL_INTERFACE(SD_IF_SDTOOLPANELSHELL)

public:
    ToolPanelViewShell (
        SfxViewFrame* pFrame,
        ViewShellBase& rViewShellBase,
        vcl::Window* pParentWindow,
        FrameView* pFrameView);
    virtual ~ToolPanelViewShell();

    using sd::ViewShell::KeyInput;

    virtual SdPage* GetActualPage() SAL_OVERRIDE;
    virtual SdPage* getCurrentPage() const SAL_OVERRIDE;

    virtual void ArrangeGUIElements() SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible>
        CreateAccessibleDocumentView (::sd::Window* pWindow) SAL_OVERRIDE;

    virtual css::uno::Reference<css::drawing::XDrawSubController> CreateSubController() SAL_OVERRIDE;

    /** Relocate all toplevel controls to the given parent window.
    */
    virtual bool RelocateToParentWindow (vcl::Window* pParentWindow) SAL_OVERRIDE;

private:
    std::unique_ptr< ToolPanelViewShell_Impl >   mpImpl;

    ::boost::shared_ptr<TaskPaneShellManager> mpSubShellManager;

    /** The id of the menu in the menu bar/tool box of the parent docking
        window.
    */
    sal_uInt16 mnMenuId;
};

} } // end of namespace ::sd::toolpanel

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
