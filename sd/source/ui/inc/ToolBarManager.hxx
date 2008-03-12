/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ToolBarManager.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:42:56 $
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

#ifndef SD_TOOL_BAR_MANAGER_HXX
#define SD_TOOL_BAR_MANAGER_HXX

#include "ViewShell.hxx"
#include "ShellFactory.hxx"
#include <rtl/ustring.hxx>

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

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
    from those assoicated with the main view.

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
        tools::EventMultiplexer& rMultiplexer,
        ViewShellManager& rViewShellManager);

    ~ToolBarManager (void);

    /** Call this method prior to the destructor to prevent the
        ToolBarManager from accessing the ViewShellManager or the
        XLayoutManager when those are possibly not well and alive anymore
        (like during the destruction of the ViewShellBase.)
    */
    void Shutdown (void);

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
    const static ::rtl::OUString msToolBar;                  // RID_DRAW_TOOLBOX, 23011
                                                             // RID_GRAPHIC_TOOLBOX, 23025
    const static ::rtl::OUString msOptionsToolBar;           // RID_DRAW_OPTIONS_TOOLBOX, 23020
                                                             // RID_GRAPHIC_OPTIONS_TOOLBOX, 23026
    const static ::rtl::OUString msCommonTaskToolBar;        // RID_DRAW_COMMONTASK_TOOLBOX, 23021
    const static ::rtl::OUString msViewerToolBar;            // RID_DRAW_VIEWER_TOOLBOX, 23023
                                                             // RID_GRAPHIC_VIEWER_TOOLBOX, 23024
    const static ::rtl::OUString msSlideSorterToolBar;       // RID_SLIDE_TOOLBOX, 23012
    const static ::rtl::OUString msSlideSorterObjectBar;     // RID_SLIDE_OBJ_TOOLBOX, 23014
    const static ::rtl::OUString msOutlineToolBar;           // RID_OUTLINE_TOOLBOX, 23017
    const static ::rtl::OUString msMasterViewToolBar;        // SID_MASTERPAGE, 27053
    const static ::rtl::OUString msDrawingObjectToolBar;     // RID_DRAW_OBJ_TOOLBOX, 23013
    const static ::rtl::OUString msGluePointsToolBar;        // RID_GLUEPOINTS_TOOLBOX, 23019
    const static ::rtl::OUString msTextObjectBar;            // RID_DRAW_TEXT_TOOLBOX, 23016
                                                             // RID_GRAPHIC_TEXT_TOOLBOX, 23028
    const static ::rtl::OUString msBezierObjectBar;          // RID_BEZIER_TOOLBOX, 23015
    const static ::rtl::OUString msGraphicObjectBar;         // RID_DRAW_GRAF_TOOLBOX, 23030
    const static ::rtl::OUString msMediaObjectBar;           // RID_DRAW_MEDIA_TOOLBOX, 23031
    const static ::rtl::OUString msTableObjectBar;           // RID_DRAW_TABLE_TOOLBOX

    /** The set of tool bar groups.
    */
    enum ToolBarGroup {
        TBG__FIRST,

        TBG_PERMANENT = TBG__FIRST,
        TBG_FUNCTION,
        TBG_MASTER_MODE,

        TBG__LAST = TBG_MASTER_MODE
    };

    /** Only after calls with bValid=<TRUE/> may the tool bar manager use
        the frame::XLayoutManager to change the visible tool bars.  Call
        this method when the controller is attached to or detachted from the
        frame.  When called with <FALSE/> then ResetAllToolBars() is
        executed.
    */
    void SetValid (bool bValid);

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
    void ResetAllToolBars (void);

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
        const ::rtl::OUString& rsToolBarName);

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
        const ::rtl::OUString& rsToolBarName);

    void RemoveToolBarShell (
        ToolBarGroup eGroup,
        ShellId nToolBarId);

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
        const ::rtl::OUString& rsToolBarName);

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

    void PreUpdate (void);

    /** Request an update of the active tool bars.  The update is made
        asynchronously.
    */
    void RequestUpdate (void);

    /** This is a hint for the ToolBarManager to improve the performance
        when it updates its tool bars when its own lock is released.  Taking
        control of the release of the update lock of the ViewShellManager
        avoids some shell stack modifications and tool bar updates.
    */
    void LockViewShellManager (void);

    /** Use this class to prevent the visible tool bars from being updated
        (and thus causing repaints and GUI rearrangements) when several tool
        bar operations are made in a row.
    */
    class UpdateLock { public:
        UpdateLock(const ::boost::shared_ptr<ToolBarManager>& rpManager)
            : mpManager(rpManager) { mpManager->LockUpdate(); }
        ~UpdateLock(void) { mpManager->UnlockUpdate(); }
    private:
        ::boost::shared_ptr<ToolBarManager> mpManager;
    };
    friend class UpdateLock;

    /** Return whether updates of tool bars are locked.
    */
    bool IsUpdateLocked (void) const;

    void ToolBarsDestroyed(void);

private:
    class Implementation;
    ::boost::scoped_ptr<Implementation> mpImpl;

    /** The ViewShellBase is used to get the XLayoutManager and to determine
        the plug in mode.
    */
    ToolBarManager (void);

    void LockUpdate (void);
    void UnlockUpdate (void);
};

} // end of namespace sd

#endif
