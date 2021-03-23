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

#pragma once

#include "ShellFactory.hxx"
#include <rtl/ustring.hxx>

#include <sal/types.h>
#include <memory>

class SdrView;
namespace sd { class ViewShell; }
namespace sd::tools { class EventMultiplexer; }

namespace sd {

class ViewShellBase;
class ViewShellManager;

/** Manage the set of visible tool bars (and object bars).  Usually they
    belong to the current view in the center pane.

    Tool bars are managed in groups.  Each group can be set, reset, or
    modified independently of the others.  This allows for instance to
    replace the toolbars associated with the current function independently
    from those associated with the main view.

    The ToolBarManager has two high level methods which contain the
    knowledge about which tool bars to show in a specific context.
    When the view in the center pane changes then MainViewShellChanged()
    sets up the tool bars for the new view.  On changes of the selection the
    SelectionHasChanged() method shows the tool bars for the new context.

    The update of the actually visible tool bars to the set currently
    required by the main view shell and its functions is divided into two
    parts, PreUpdate() and PostUpdate().  This are to be called before
    respectively after the update of the view shell stack.  The reason for
    this is to save time by not updating tool bars that will not be visible
    in a short time on a view shell switch.
*/
class ToolBarManager
    : public std::enable_shared_from_this<ToolBarManager>
{
public:
    /** Use this method instead of the constructor to create new objects of
        this class.
    */
    static std::shared_ptr<ToolBarManager> Create (
        ViewShellBase& rBase,
        const std::shared_ptr<tools::EventMultiplexer>& rpMultiplexer,
        const std::shared_ptr<ViewShellManager>& rpViewShellManager);

    ~ToolBarManager();

    /** Call this method prior to the destructor to prevent the
        ToolBarManager from accessing the ViewShellManager or the
        XLayoutManager when those are possibly not well and alive anymore
        (like during the destruction of the ViewShellBase.)
    */
    void Shutdown();

    /** When the view in the center pane changes then this method sets up
        the initial set of tool bars for the new view.
        The ToolBarManager listens for view switching itself and then calls
        MainViewShellChanged().  Calling this method from the outside should
        not be necessary.
        @param nShellType
            The type of the new main view shell.
    */
    void MainViewShellChanged ();
    void MainViewShellChanged (const ViewShell& rMainViewShell);

    /** Call this method when the selection has changed to update the more
        temporary tool bars (those in the ToolBarGroup::Function group.)
    */
    void SelectionHasChanged (
        const ViewShell& rViewShell,
        const SdrView& rView);

    /** The set of tool bars that are handled by this manager class.
    */
    constexpr static OUStringLiteral msToolBar = u"toolbar"; // Draw_Toolbox_Sd, 23011
    constexpr static OUStringLiteral msOptionsToolBar = u"optionsbar";
        // Draw_Options_Toolbox, 23020
    constexpr static OUStringLiteral msCommonTaskToolBar = u"commontaskbar";
        // Draw_CommonTask_Toolbox, 23021
    constexpr static OUStringLiteral msViewerToolBar = u"viewerbar"; // Draw_Viewer_Toolbox, 23023
    constexpr static OUStringLiteral msSlideSorterToolBar = u"slideviewtoolbar";
        // Slide_Toolbox, 23012
    constexpr static OUStringLiteral msSlideSorterObjectBar = u"slideviewobjectbar";
        // Slide_Obj_Toolbox, 23014
    constexpr static OUStringLiteral msOutlineToolBar = u"outlinetoolbar"; // Outline_Toolbox, 23017
    constexpr static OUStringLiteral msMasterViewToolBar = u"masterviewtoolbar";
        // SID_MASTERPAGE, 27053
    constexpr static OUStringLiteral msDrawingObjectToolBar = u"drawingobjectbar";
        // Draw_Obj_Toolbox, 23013
    constexpr static OUStringLiteral msGluePointsToolBar = u"gluepointsobjectbar";
        // Gluepoints_Toolbox, 23019
    constexpr static OUStringLiteral msTextObjectBar = u"textobjectbar";
        // Draw_Text_Toolbox_Sd, 23016
    constexpr static OUStringLiteral msBezierObjectBar = u"bezierobjectbar";
        // Bezier_Toolbox_Sd, 23015
    constexpr static OUStringLiteral msGraphicObjectBar = u"graphicobjectbar";
        // Draw_Graf_Toolbox, 23030
    constexpr static OUStringLiteral msMediaObjectBar = u"mediaobjectbar";
        // Draw_Media_Toolbox, 23031
    constexpr static OUStringLiteral msTableObjectBar = u"tableobjectbar";
        // Draw_Table_Toolbox, 23018

    /** The set of tool bar groups.
    */
    enum class ToolBarGroup {
        Permanent,
        Function,
        CommonTask,
        MasterMode,
        LAST = MasterMode
    };

    /** Reset the set of visible object bars in the specified group.  Tool
        bars in other groups are not affected.
        @param rParentShell
            When this shell is not the main view then the method returns
            immediately.
        @param eGroup
            Only the tool bars in this group are rest.
    */
    void ResetToolBars (ToolBarGroup eGroup);

    /** Reset all tool bars, regardless of the group they belong to.
        @param rParentShell
            When this shell is not the main view then the method returns
            immediately.
    */
    void ResetAllToolBars();

    /** Add the tool bar with the given name to the specified group of tool
        bars.
        @param rParentShell
            When this shell is not the main view then the method returns
            immediately.
        @param eGroup
            The new tool bar is added to this group.
        @param rsToolBarName
            The base name of the tool bar.  A proper prefix (like
            private:resource/toolbar/) is added.  The name may be one of the
            ones defined above.  Other names are allowed as well.
    */
    void AddToolBar (
        ToolBarGroup eGroup,
        const OUString& rsToolBarName);

    /** Add the tool bar shell to the shell stack.  This method basically
        forwards the call to the ViewShellManager.
        For some tool bar shells additional tool bars are made visible.
        @param rParentShell
            When this shell is not the main view then the method returns
            immediately.
        @param eGroup
            The group is used for the actual tool bars.
        @param nToolBarId
            Id of the tool bar shell.
    */
    void AddToolBarShell (
        ToolBarGroup eGroup,
        ShellId nToolBarId);

    /** Remove the tool bar with the given name from the specified group.
        If the tool bar is not visible then nothing happens.
        If the tool bar is a member of another group then nothing happens
        either.
    */
    void RemoveToolBar (
        ToolBarGroup eGroup,
        const OUString& rsToolBarName);

    /** This is basically a shortcut for ResetToolBars(),AddToolBar().  The
        main difference is, that all sub shells of the specified parent
        shell are deactivated as well.
        @param rParentShell
            When this shell is not the main view then the method returns
            immediately.
        @param eGroup
            The new tool bar is added to this group.
        @param rsToolBarName
            The base name of the tool bar.  A proper prefix (like
            private:resource/toolbar/) is added.  The name may be one of the
            ones defined above.  Other names are allowed as well.
    */
    void SetToolBar (
        ToolBarGroup eGroup,
        const OUString& rsToolBarName);

    /** This is basically a shortcut for ResetToolBars(),AddToolBar().  The
        main difference is, that all sub shells of the specified parent
        shell are deactivated as well.
        @param rParentShell
            When this shell is not the main view then the method returns
            immediately.
        @param rParentShell
            When this shell is not the main view then the method returns
            immediately.
        @param eGroup
            The group is currently not used.
        @param nToolBarId
            Id of the tool bar shell.
    */
    void SetToolBarShell (
        ToolBarGroup eGroup,
        ShellId nToolBarId);

    void PreUpdate();

    /** Request an update of the active tool bars.  The update is made
        asynchronously.
    */
    void RequestUpdate();

    /** This is a hint for the ToolBarManager to improve the performance
        when it updates its tool bars when its own lock is released.  Taking
        control of the release of the update lock of the ViewShellManager
        avoids some shell stack modifications and tool bar updates.
    */
    void LockViewShellManager();

    /** Use this class to prevent the visible tool bars from being updated
        (and thus causing repaints and GUI rearrangements) when several tool
        bar operations are made in a row.
    */
    class UpdateLock { public:
        UpdateLock(const std::shared_ptr<ToolBarManager>& rpManager)
            : mpManager(rpManager) { mpManager->LockUpdate(); }
        ~UpdateLock() COVERITY_NOEXCEPT_FALSE { mpManager->UnlockUpdate(); }
    private:
        std::shared_ptr<ToolBarManager> mpManager;
    };
    friend class UpdateLock;

    void ToolBarsDestroyed();

private:
    class Implementation;
    std::unique_ptr<Implementation> mpImpl;

    /** The ViewShellBase is used to get the XLayoutManager and to determine
        the plug in mode.
    */
    ToolBarManager();

    void LockUpdate();
    void UnlockUpdate();
};

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
