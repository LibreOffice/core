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

#ifndef INCLUDED_SD_SOURCE_UI_INC_TOOLBARMANAGER_HXX
#define INCLUDED_SD_SOURCE_UI_INC_TOOLBARMANAGER_HXX

#include "ViewShell.hxx"
#include "ShellFactory.hxx"
#include <rtl/ustring.hxx>
#include <com/sun/star/frame/XFrame.hpp>

#include <sal/types.h>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

class SdrView;

namespace sd { namespace tools {
class EventMultiplexer;
} }

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
    : public ::boost::enable_shared_from_this<ToolBarManager>
{
public:
    /** Use this method instead of the constructor to create new objects of
        this class.
    */
    static ::boost::shared_ptr<ToolBarManager> Create (
        ViewShellBase& rBase,
        const ::boost::shared_ptr<tools::EventMultiplexer>& rpMultiplexer,
        const ::boost::shared_ptr<ViewShellManager>& rpViewShellManager);

    ~ToolBarManager();

    /** Call this method prior to the destructor to prevent the
        ToolBarManager from accessing the ViewShellManager or the
        XLayoutManager when those are possibly not well and alive anymore
        (like during the destruction of the ViewShellBase.)
    */
    void Shutdown();

    /** When the view in the center pane changes then this method sets up
        the initial set of tool bars for the new view.
        The ToolBarManager listenes for view switching itself and then calls
        MainViewShellChanged().  Calling this method from the outside should
        not be necessary.
        @param nShellType
            The type of the new main view shell.
    */
    void MainViewShellChanged (ViewShell::ShellType nShellType);
    void MainViewShellChanged (const ViewShell& rMainViewShell);

    /** Call this method when the selection has changed to update the more
        temporary tool bars (those in the TBG_FUNCTION group.)
    */
    void SelectionHasChanged (
        const ViewShell& rViewShell,
        const SdrView& rView);

    /** The set of tool bars that are handled by this manager class.
    */
    const static OUString msToolBar;                  // RID_DRAW_TOOLBOX, 23011
    const static OUString msOptionsToolBar;           // RID_DRAW_OPTIONS_TOOLBOX, 23020
    const static OUString msCommonTaskToolBar;        // RID_DRAW_COMMONTASK_TOOLBOX, 23021
    const static OUString msViewerToolBar;            // RID_DRAW_VIEWER_TOOLBOX, 23023
    const static OUString msSlideSorterToolBar;       // RID_SLIDE_TOOLBOX, 23012
    const static OUString msSlideSorterObjectBar;     // RID_SLIDE_OBJ_TOOLBOX, 23014
    const static OUString msOutlineToolBar;           // RID_OUTLINE_TOOLBOX, 23017
    const static OUString msMasterViewToolBar;        // SID_MASTERPAGE, 27053
    const static OUString msDrawingObjectToolBar;     // RID_DRAW_OBJ_TOOLBOX, 23013
    const static OUString msGluePointsToolBar;        // RID_GLUEPOINTS_TOOLBOX, 23019
    const static OUString msTextObjectBar;            // RID_DRAW_TEXT_TOOLBOX, 23016
    const static OUString msBezierObjectBar;          // RID_BEZIER_TOOLBOX, 23015
    const static OUString msGraphicObjectBar;         // RID_DRAW_GRAF_TOOLBOX, 23030
    const static OUString msMediaObjectBar;           // RID_DRAW_MEDIA_TOOLBOX, 23031
    const static OUString msTableObjectBar;           // RID_DRAW_TABLE_TOOLBOX

    /** The set of tool bar groups.
    */
    enum ToolBarGroup {
        TBG__FIRST,

        TBG_PERMANENT = TBG__FIRST,
        TBG_FUNCTION,
        TBG_COMMON_TASK,
        TBG_MASTER_MODE,

        TBG__LAST = TBG_MASTER_MODE
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
        UpdateLock(const ::boost::shared_ptr<ToolBarManager>& rpManager)
            : mpManager(rpManager) { mpManager->LockUpdate(); }
        ~UpdateLock() { mpManager->UnlockUpdate(); }
    private:
        ::boost::shared_ptr<ToolBarManager> mpManager;
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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
