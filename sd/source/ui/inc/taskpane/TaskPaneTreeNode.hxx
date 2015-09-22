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

#ifndef INCLUDED_SD_SOURCE_UI_INC_TASKPANE_TASKPANETREENODE_HXX
#define INCLUDED_SD_SOURCE_UI_INC_TASKPANE_TASKPANETREENODE_HXX

#include "ILayoutableWindow.hxx"
#include <memory>
#include <vector>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <tools/link.hxx>

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
    tool panel. In the task pane, there are multiple hierarchies of such nodes,
    with every panel having an own tree. The pane node is the root of the tree, below
    that there are SubToolPanels and Window/Control objects. At the
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
    virtual ~TreeNode();

    void SetParentNode (TreeNode* pNewParent);
    TreeNode* GetParentNode();

    /** Return the Window pointer of a tree node.
    */
    virtual vcl::Window* GetWindow();

    /** Return a const pointer to the window of a tree node.
    */
    virtual const vcl::Window* GetConstWindow() const;

    /** Return the joined minimum width of all children, i.e. the largest of
        the minimum widths.
    */
    virtual sal_Int32 GetMinimumWidth();

    /** The default implementation always returns <FALSE/>
    */
    virtual bool IsResizable();

    /** Call this method whenever the size of one of the children of the
        called node has to be changed, e.g. when the layout menu shows more
        or less items than before.  As a typical result the node will layout
        and resize its children according to their size requirements.

        Please remember that the size of the children can be changed in the
        first place because scroll bars can give a node the space it needs.

        The default implementation passes this call to its parent.
    */
    virtual void RequestResize();

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
    virtual bool IsExpanded() const;

    /** Return whether the node can be expanded or collapsed.  The default
        implementation always returns <TRUE/> when there is window and
        <FALSE/> otherwise.  If <FALSE/> is returned
        then Expand() may be called but it will not change the expansion
        state.
    */
    virtual bool IsExpandable() const;

    /** The default implementation calls GetWindow()->Show().
    */
    virtual void Show (bool bVisibilityState);

    /** The default implementation returns GetWindow()->IsVisible().
    */
    virtual bool IsShowing() const;

    ControlContainer& GetControlContainer();

    /** Give each node access to a shell manage.  This usually is the shell
        manager of the ToolPanelViewShell.

        At least the root node has to overwrite this method since the
        default implementation simply returns the shell manager of its
        parent.
    */
    virtual TaskPaneShellManager* GetShellManager();

    /** You will rarely need to override this method.  To supply your own
        accessible object you should override CreateAccessible() instead.
    */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible> GetAccessibleObject();

    /** Override this method in order to supply a class specific accessible
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

    /** Add a listener that will be informatted in the future about state
        changes of the tree node.  This includes adding and removing
        children as well as focus, visibility, and expansion state.
        Multiple calls are ignored.  Each listener is added only once.
    */
    void AddStateChangeListener (const Link<>& rListener);

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
    ::std::unique_ptr<ControlContainer> mpControlContainer;

private:
    TreeNode* mpParent;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
