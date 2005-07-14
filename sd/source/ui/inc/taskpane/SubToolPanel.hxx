/*************************************************************************
 *
 *  $RCSfile: SubToolPanel.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2005-07-14 10:14:17 $
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

#ifndef SD_TASKPANE_SUB_TOOL_PANEL_HXX
#define SD_TASKPANE_SUB_TOOL_PANEL_HXX

#include "taskpane/TaskPaneTreeNode.hxx"

#include <vcl/ctrl.hxx>
#include <vcl/scrbar.hxx>
#include <memory>
#include <vector>


class Window;

namespace sd { namespace toolpanel {

class ToolPanel;

/** The sub tool panel is in function similar to the tool panel.  It
    differes in two points.  First, it is a control that can be used
    as element in a tool panel and thus is actually a nested tool
    panel.

    <p>Secondly, it formats its elements differently.  The actual controls
    are placed one below the other with a title bar above each control.
    Clicking on the title bar expands or collapses the control.  When there
    is not enough space then scroll bars are shown.</p>

    <p>To avoid flickering when painting the sub tool panel the background
    is made transparent and painting it is done by this class.  While
    layouting its children it remembers the gaps between children and stores
    them in maStripeList.  In Paint() those gaps as well as the border
    arround all children are painted in the background color.</p>
*/
class SubToolPanel
    : public Control,
      public TreeNode
{
public:
    /** Create a new sub tool panel with the given window as its
        parent.  This will usually be a child window.
    */
    SubToolPanel (TreeNode* pParent);
    virtual ~SubToolPanel (void);

    /** Add a control to the sub panel.
        @param rTitle
            The title that will be shown in the two title bars that
            belong to the control.
        @param nHelpId
            The help id is set at the title bar not the actual control.
    */
    void AddControl (
        ::std::auto_ptr<TreeNode> pControl,
        const String& rTitle,
        ULONG nHelpId);
    void AddControl (::std::auto_ptr<TreeNode> pControl);

    virtual void Paint (const Rectangle& rRect);

    /** Initiate a rearrangement of the controls and title bars.
    */
    virtual void Resize (void);

    virtual void RequestResize (void);

    virtual Size GetPreferredSize (void);
    virtual sal_Int32 GetPreferredWidth (sal_Int32 nHeight);
    virtual sal_Int32 GetPreferredHeight (sal_Int32 nWidth);
    virtual bool IsResizable (void);
    virtual ::Window* GetWindow (void);
    virtual sal_Int32 GetMinimumWidth (void);

    virtual void ExpandControl (
        TreeNode* pControl,
        bool bExpansionState);

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible> CreateAccessibleObject (
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible>& rxParent);

protected:
    /** Initiate a rearrangement of the controls.
    */
    void ListHasChanged (void);

private:
    ::Window maWindowFiller;
    bool mbIsRearrangePending;
    bool mbIsLayoutPending;
    sal_uInt32 mnChildrenWidth;
    /// Border above top-most and below bottom-most control.
    const int mnVerticalBorder;
    /// Gap between two controls.
    const int mnVerticalGap;
    /// Border at the left and right of the controls.
    const int mnHorizontalBorder;
    /** List of horizontal stripes that is created from the gaps between
        children when they are layouted.  The stripes are painted in Paint()
        to fill the space arround the children.
    */
    typedef ::std::vector< ::std::pair<int,int> > StripeList;
    StripeList maStripeList;

    /** Calculate position, size, and visibility of the controls.
        Call this method after the list of controls, their expansion
        state, or the size of the sub panel has changed.
    */
    void Rearrange (void);

    /** Determine the minimal size that is necessary to show the controls
        one over the other.  It may be smaller than the available area.
    */
    Size GetRequiredSize (void);

    /** Place the child windows one above the other and return the size of
        the bounding box.
    */
    sal_Int32 LayoutChildren (void);

    DECL_LINK(WindowEventListener, VclSimpleEvent*);
};

} } // end of namespace ::sd::toolpanel

#endif
