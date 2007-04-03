/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ToolPanel.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-03 16:14:08 $
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
#ifndef SD_TOOL_PANEL_HXX
#define SD_TOOL_PANEL_HXX

#include "taskpane/TaskPaneTreeNode.hxx"
#include "taskpane/TitledControl.hxx"
#ifndef _SV_CTRL_HXX
#include <vcl/ctrl.hxx>
#endif

#include <vector>
#include <memory>

class Window;

namespace sd { namespace toolpanel {

class TaskPaneViewShell;
class ControlFactory;

/** This control is basically a layout manager for the vertical
    stacking of controls.  At any one time there is exactly one of
    them visible.  For the others only their title bar is shown.

    <p>Each control is independant of both other controls and the tool
    panel itself.  To be usable by the tool panel they only have to be
    derived from the vcl Window class.</p>
*/
class ToolPanel
    : public Control,
      public TreeNode
{
public:
    /** Create a new tool panel with the given window as its parent.
        This will usually be a child window.
    */
    ToolPanel (
        ::Window* pParentWindow,
        TaskPaneViewShell& rViewShell);

    virtual ~ToolPanel (void);

    /** Append the given control to the end of the list of controls that are
        managed by the tool panel.
        @param pControlFactory
            Factory that provides the control on demand, typically when it
            is expanded the first time.
        @param rTitle
            The string that is displayed in the title bar above the control.
        @param nHelpId
            The help id is set at the title bar not the actual control.
        @param rClickHandler
            The click handler typically expands a control when the user has
            clicked on its title.
    */
    sal_uInt32 AddControl (
        ::std::auto_ptr<ControlFactory> pControlFactory,
        const String& rTitle,
        ULONG nHelpId,
        const TitledControl::ClickHandler& rClickHandler);

    virtual void Resize (void);

    virtual Size GetPreferredSize (void);
    virtual sal_Int32 GetPreferredWidth (sal_Int32 nHeight);
    virtual sal_Int32 GetPreferredHeight (sal_Int32 nWidth);
    virtual bool IsResizable (void);
    virtual ::Window* GetWindow (void);

    virtual void RequestResize (void);

    virtual TaskPaneShellManager* GetShellManager (void);

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible> CreateAccessibleObject (
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible>& rxParent);

    using Window::GetWindow;

protected:
    /** Initiate a rearrangement of the controls.
    */
    void ListHasChanged (void);

private:
    TaskPaneViewShell& mrViewShell;

    /// Guard against recursive calls or Rearrange().
    bool mbRearrangeActive;

    /** Calculate position, size, and visibility of the controls.
        Call this method after the list of controls has changed.
    */
    void Rearrange (void);
};



} } // end of namespace ::sd::toolpanel

#endif
