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

#ifndef INCLUDED_SD_SOURCE_UI_INC_TASKPANE_CONTROLCONTAINER_HXX
#define INCLUDED_SD_SOURCE_UI_INC_TASKPANE_CONTROLCONTAINER_HXX

#include <osl/mutex.hxx>

#include <vector>
#include <memory>

namespace sd { namespace toolpanel {

class TreeNode;

/** This container manages the children of a TreeNode.  It handles the
    expansion and visibility state of its child controls.  The container
    does not do the layouting or painting of the controls.  Instead it asks
    its owner to do that.

    The difference between expansion state and visibility is that when a
    control is collapsed at least a title bar is shown for it.  When it is
    not visible then even this title bar is not shown.  In that case the
    user can not expand the control.  A control has to be visible in order
    to be expanded or collapsed.

    Whenever you expand or collapse, show or hide a child control then use
    this container class.  Do not call the respective methods of the child
    directly.
*/
class ControlContainer
{
public:
    enum VisibilityState { VS_SHOW, VS_HIDE, VS_TOGGLE };
    enum ExpansionState { ES_EXPAND, ES_COLLAPSE, ES_TOGGLE };

    /** Create a new control container.
        @param pParent
            This node is asked to re-calculate the size of its children when
            a child of this container is expanded or collapsed.
    */
    ControlContainer (TreeNode* pNode);

    virtual ~ControlContainer (void);

    /** This is function makes sure that all children are deleted.  Call
        this function from the destructor of a sub class to have all child
        windows deleted before the destructor of another base class of that
        sub class is called.  When that other base class is some kind of a
        window it would otherwise complain that there are living children.
    */
    void DeleteChildren (void);

    /** Add the given control to the set of controls managed by the
        container.  This control is then expanded.
        @return
            Return the index under which the control has been inserted in
            the container.  It is the same index that is returned by
            GetControlIndex().
    */
    sal_uInt32 AddControl (::std::auto_ptr<TreeNode> pControl);

    /** Expand (default) or collapse the specified control.  When
        expanding a control in a single expansion environment then all
        other controls are collapsed.  The specified control is being
        made the active control as returned by GetActiveControl().
    */
    virtual void SetExpansionState (
        sal_uInt32 nIndex,
        ExpansionState aState);
    virtual void SetExpansionState (
        TreeNode* pControl,
        ExpansionState aState);
    virtual void SetVisibilityState (
        sal_uInt32 nIndex,
        VisibilityState aState);

    /** Return the index of the given control.
    */
    sal_uInt32 GetControlIndex (TreeNode* pControl) const;

    /** Return the number of controls in the container.
    */
    sal_uInt32 GetControlCount (void) const;

    /** Return the number of visible controls in the container.
    */
    sal_uInt32 GetVisibleControlCount (void) const;

    /** Return the control with the specified index regardless of whether
        that control is hidden or visible.
    */
    TreeNode* GetControl (sal_uInt32 nIndex) const;

    /** Return the index of the control previous to that that is specified
        by the given index.
        @param nIndex
            Index of the control for which to return the index of the
            previous control.  This index is guaranteed not to be returned.
        @param bIncludeHidden
            This flag tells the method whether to include the controls that
            are not visible in the search for the previous control.  When it
            is <FALSE/> the hidden controls are skipped.
        @param bCycle
            When this flag is <TRUE/> then the search for the previous
            control wraps around when reaching the first control.
        @return
            Returns the index to the previous control or (sal_uInt32)-1 when
            there is no previous control.  This would be the case when there
            is only one (visible) child.
    */
    sal_uInt32 GetPreviousIndex (
        sal_uInt32 nIndex,
        bool bIncludeHidden=false,
        bool bCycle=false) const;

    /** Return the index of the control next to that that is specified by
        the given index.
        @param nIndex
            Index of the control for which to return the index of the next
            control.  This index is guaranteed not to be returned.
        @param bIncludeHidden
            This flag tells the method whether to include the controls that
            are not visible in the search for the next control.  When it is
            <FALSE/> the hidden controls are skipped.
        @param bCycle
            When this flag is <TRUE/> then the search for the next control
            wraps around when reaching the last control.
        @return
            Returns the index to the next control or (sal_uInt32)-1 when
            there is no next control.  This would be the case when there is
            only one (visible) child.
    */
    sal_uInt32 GetNextIndex (
        sal_uInt32 nIndex,
        bool bIncludeHidden=false,
        bool bCycle=false) const;

    void SetMultiSelection (bool bFlag);

    /** This is method is called when the list of controls has changed,
        i.e. a new control has been added.  The default implementation is
        empty.  Overwrite this method in derived classes in order to react to
        such changes.
    */
    virtual void ListHasChanged (void);

private:
    osl::Mutex maMutex;

    /// List of controls managed by a container.
    typedef ::std::vector<TreeNode*> ControlList;
    ControlList maControlList;

    /** This parent is used for resize requests when children are expanded
        or collapsed.
    */
    TreeNode* mpNode;

    /** The index of the currently expanded control.  A value of
        (sal_uInt32)-1 indicates that no control is active.  This may be the
        case after adding controls to the container.
    */
    sal_uInt32 mnActiveControlIndex;

    bool mbMultiSelection;
};

} } // end of namespace ::sd::toolpanel

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
