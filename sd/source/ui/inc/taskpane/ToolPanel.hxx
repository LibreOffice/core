/*************************************************************************
 *
 *  $RCSfile: ToolPanel.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-07-14 10:15:13 $
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
#ifndef SD_TOOL_PANEL_HXX
#define SD_TOOL_PANEL_HXX

#include "taskpane/TaskPaneTreeNode.hxx"
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
        @param nHelpId
            The help id is set at the title bar not the actual control.
    */
    sal_uInt32 AddControl (
        ::std::auto_ptr<ControlFactory> pControlFactory,
        const String& rTitle,
        ULONG nHelpId);

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
