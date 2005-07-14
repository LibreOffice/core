/*************************************************************************
 *
 *  $RCSfile: TaskPaneTreeNode.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2005-07-14 10:14:31 $
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

#ifndef SD_TASKPANE_TREE_NODE_HXX
#define SD_TASKPANE_TREE_NODE_HXX

#include "ILayoutableWindow.hxx"
#include <memory>
#include <vector>

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <com/sun/star/accessibility/XAccessible.hpp>
#endif
#include <tools/link.hxx>

namespace sd {
class ObjectBarManager;
};

namespace sd { namespace toolpanel {

class ControlContainer;
class TaskPaneShellManager;

enum TreeNodeStateChangeEventId {
    EID_CHILD_ADDED,
    EID_ALL_CHILDREN_REMOVED,
    EID_EXPANSION_STATE_CHANGED,
    EID_FOCUSED_STATE_CHANGED,
    EID_SHOWING_STATE_CHANGED
};


/** Base class for all members of the object hierarchy that makes up the
    tool panel.  There are usually at least three levels.  At the top level
    is the ToolPanel with one instance: the root of the tree.  At the
    middle level there are SubToolPanels and Window/Control objects.  At the
    lowest level there are only Window or Control objects.

    This class provides the means of communication between objects on
    different levels.
*/
class TreeNode
    : public ILayoutableWindow,
      public ILayouter
{
public:
    TreeNode (TreeNode* pParent);
    virtual ~TreeNode (void);

    /** Returns <TRUE/> if the node has no children, i.e. is a leaf of a
        tree.  In this case mpControlContainer is NULL.
    */
    bool IsLeaf (void);

    /** Returns true if the node has no parent, i.e. is the root of a tree.
    */
    bool IsRoot (void);

    void SetParentNode (TreeNode* pNewParent);
    TreeNode* GetParentNode (void);

    /** Return the Window pointer of a tree node.
    */
    virtual ::Window* GetWindow (void);

    /** Return a const pointer to the window of a tree node.
    */
    virtual const ::Window* GetConstWindow (void) const;

    /** Return the joined minimum width of all children, i.e. the largest of
        the minimum widths.
    */
    virtual sal_Int32 GetMinimumWidth (void);

    /** Give each node access to the object bar manager of the tool panel.

        At least the root node has to overwrite this method since the
        default implementation simply returns the object bar manager of the
        parent.
    */
    virtual ObjectBarManager* GetObjectBarManager (void);

    /** The default implementaion always returns <FALSE/>
    */
    virtual bool IsResizable (void);

    /** Call this method whenever the size of one of the children of the
        called node has to be changed, e.g. when the layout menu shows more
        or less items than before.  As a typical result the node will layout
        and resize its children according to their size requirements.

        Please remember that the size of the children can be changed in the
        first place because scroll bars can give a node the space it needs.

        The default implementation passes this call to its parent.
    */
    virtual void RequestResize (void);

    /** The default implementation shows the window (when it exists) when
        bExpansionState is <TRUE/>.  It hides the window otherwise.
        @return
            Returns <TRUE/> when the expansion state changes.  When an
            expansion state is requested that is already in place then
            <FALSE/> is returned.
    */
    virtual bool Expand (bool bExpansionState);

    /** The default implementation returns whether the window is showing.
        When there is no window then it returns <FALSE/>.
    */
    virtual bool IsExpanded (void) const;

    /** Return whether the node can be expanded or collapsed.  The default
        implementation always returns <TRUE/> when there is window and
        <FALSE/> otherwise.  If <FALSE/> is returned
        then Expand() may be called but it will not change the expansion
        state.
    */
    virtual bool IsExpandable (void) const;

    /** The default implementation calls GetWindow()->Show().
    */
    virtual void Show (bool bVisibilityState);

    /** The default implementation returns GetWindow()->IsVisible().
    */
    virtual bool IsShowing (void) const;

    ControlContainer& GetControlContainer (void);

    /** Give each node access to a shell manage.  This usually is the shell
        manager of the TaskPaneViewShell.

        At least the root node has to overwrite this method since the
        default implementation simply returns the shell manager of its
        parent.
    */
    virtual TaskPaneShellManager* GetShellManager (void);

    /** You will rarely need to overload this method.  To supply your own
        accessible object you should overload CreateAccessible() instead.
    */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible> GetAccessibleObject (void);

    /** Overload this method in order to supply a class specific accessible
        object.
        The default implementation will return a new instance of
        AccessibleTreeNode.
        @param rxParent
            The accessible parent of the accessible object to create.  It is
            not necessaryly the accessible object of the parent window of
            GetWindow().

    */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible> CreateAccessibleObject (
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible>&rxParent);

    /** Add a listener that will be informated in the future about state
        changes of the tree node.  This includes adding and removing
        children as well as focus, visibility, and expansion state.
        Multiple calls are ignored.  Each listener is added only once.
    */
    void AddStateChangeListener (const Link& rListener);

    /** Remove the listener form the list of state change listeners.
        @param rListener
            It is OK to specify a listener that is not currently
            registered.  Only when the listener is registered it is
            removed.  Otherwise the call is ignored.
    */
    void RemoveStateChangeListener (const Link& rListener);

    /** Call the state change listeners and pass a state change event with
        the specified event id.  The source field is set to this.
        @param pChild
            This optional parameter makes sense only with the
            EID_CHILD_ADDED event.
    */
    void FireStateChangeEvent (
        TreeNodeStateChangeEventId eEventId,
        TreeNode* pChild = NULL) const;

protected:
    ::std::auto_ptr<ControlContainer> mpControlContainer;

private:
    TreeNode* mpParent;
    typedef ::std::vector<Link> StateChangeListenerContainer;
    StateChangeListenerContainer maStateChangeListeners;
};




/** Objects of this class are sent to listeners to notify them about state
    changes of a tree node.
*/
class TreeNodeStateChangeEvent
{
public:

    TreeNodeStateChangeEvent (
        const TreeNode& rNode,
        TreeNodeStateChangeEventId eEventId,
        TreeNode* pChild = NULL);

    const TreeNode& mrSource;
    TreeNodeStateChangeEventId meEventId;
    TreeNode* mpChild;
};

} } // end of namespace ::sd::toolpanel

#endif
