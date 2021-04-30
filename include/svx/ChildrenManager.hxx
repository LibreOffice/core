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

#ifndef INCLUDED_SVX_CHILDRENMANAGER_HXX
#define INCLUDED_SVX_CHILDRENMANAGER_HXX

#include <sal/config.h>

#include <com/sun/star/uno/Reference.hxx>
#include <rtl/ref.hxx>
#include <svx/svxdllapi.h>
#include <tools/long.hxx>

namespace com::sun::star
{
namespace accessibility
{
class XAccessible;
}
namespace drawing
{
class XShape;
}
namespace drawing
{
class XShapes;
}
}

namespace accessibility
{
class AccessibleContextBase;
class AccessibleShapeTreeInfo;
class ChildrenManagerImpl;

/** The AccessibleChildrenManager class acts as a cache of the
    accessible objects of the currently visible shapes of a draw page and as
    a factory to create the corresponding accessible object for a given
    shape.

    <p>There are two sources of shapes.  The first is a list of UNO shapes.
    It is passes to the constructor and can be re-set with a call to
    <member>SetShapeList</member>.  Accessibility objects which represent
    these shapes are constructed usually on demand.  The second source is a
    set of single AccessibleShape objects added by calls to
    <member>AddAccessibleShape</member>.  These are not modified by this
    class.  They are only copied into the list of visible shapes depending
    on their visibility.  The later list can be cleared by calling
    <member>ClearAccessibleShapeList</member>.  The actual set of children
    accessible through the <member>GetChildrenCount</member> and
    <member>GetChild</member> methods are the accessible objects that lie
    completely or partially inside the visible area obtained from the view
    forwarder in the shape tree info.</p>

    <p>A children manager registers itself at the broadcaster passed to its
    constructor and transforms/forwards the received events to the listeners
    of the also given context base.  The transformation process includes
    interpreting a document::XEventListener, the change from
    XShape references to the associated XAccessible reference, and the
    creation of an XAccessibleEventObject.  The event object is then passed
    to the <method>fireEvent</method> method of the context and from there
    forwarded to all listeners.</p>

    <p>Note that some of the data given to the constructor is passed to the
    accessible shape objects created by that child manager which in turn
    pass these data to their child managers.  The data is thus propagated
    down a tree of child managers/accessible shape objects from a top level
    class that is usually a document/model.  This is the way to give all
    accessible objects access to data normally known only to the top level
    owner of the shapes.</p>
*/
class SVX_DLLPUBLIC ChildrenManager final
{
public:
    /** Create a children manager, which manages the children of the given
        parent.  The parent is used for creating accessible objects.  The
        list of shapes for which to create those objects is not derived from
        the parent and has to be provided separately by calling one of the
        update methods.
        @param rxParent
            The parent of the accessible objects which will be created
            on demand at some point of time in the future.
        @param rxShapeList
            List of UNO shapes to manage.
        @param rShapeTreeInfo
            Bundle of information passed down the shape tree.
        @param rContext
            An accessible context object that is called for firing events
            for new and deleted children, i.e. that holds a list of
            listeners to be informed.
    */
    ChildrenManager(const css::uno::Reference<css::accessibility::XAccessible>& rxParent,
                    const css::uno::Reference<css::drawing::XShapes>& rxShapeList,
                    const AccessibleShapeTreeInfo& rShapeTreeInfo, AccessibleContextBase& rContext);

    /** If there still are managed children these are marked as DEFUNC and
        released.
    */
    ~ChildrenManager();

    /** Return the number of currently visible accessible children.
        @return
            If there are no children a 0 is returned.
    */
    tools::Long GetChildCount() const noexcept;

    /** Return the requested accessible child or throw and
        IndexOutOfBoundsException if the given index is invalid.
        @param nIndex
            Index of the requested child.  Call getChildCount for obtaining
            the number of children.
        @return
            In case of a valid index this method returns a reference to the
            requested accessible child.  This reference is empty if it has
            not been possible to create the accessible object of the
            corresponding shape.
        @throws
            Throws an IndexOutOfBoundsException if the index is not valid.
    */
    css::uno::Reference<css::accessibility::XAccessible> GetChild(tools::Long nIndex);
    /// @throws css::lang::IndexOutOfBoundsException
    /// @throws css::uno::RuntimeException
    css::uno::Reference<css::drawing::XShape> GetChildShape(tools::Long nIndex);

    /** Update the child manager.  Take care of a modified set of children
        and modified visible area.  This method can optimize the update
        process with respect separate updates of a modified children list
        and visible area.
        @param bCreateNewObjectsOnDemand
            If </true> then accessible objects associated with the visible
            shapes are created only when asked for.  No event is sent on
            creation.  If </false> then the accessible objects are created
            before this method returns and events are sent to inform the
            listeners of the new object.
    */
    void Update(bool bCreateNewObjectsOnDemand = true);

    /** Replace the list of UNO shapes by the specified list.
        @param xShapeList
            The new list of shapes.
    */
    void SetShapeList(const css::uno::Reference<css::drawing::XShapes>& xShapeList);

    /** Add an accessible shape.  The difference to the UNO shapes in the
        list passed to the constructor the specified object does not have to
        be created by the shape factory.  This gives the caller full control
        over object creation.

        @param shape must be non-null
    */
    void AddAccessibleShape(css::uno::Reference<css::accessibility::XAccessible> const& shape);

    /** Clear the list of accessible shapes which have been added by
        previous calls to <member>AddAccessibleShape</member>.
    */
    void ClearAccessibleShapeList();

    /** Take a new event shape tree info.  Call this method to inform the
        children manager of a change of the info bundle.
        @param rShapeTreeInfo
            The new info that replaces the current one.
    */
    void SetInfo(AccessibleShapeTreeInfo const& rShapeTreeInfo);

    /** Update the SELECTED and FOCUSED states of all visible children
        according to the given selection.  This includes setting
        <em>and</em> resetting the states.
    */
    void UpdateSelection();

    /** Return whether one of the shapes managed by this object has
        currently the focus.
        @return
            Returns <true/> when there is a shape that has the focus and
            <false/> when there is no such shape.
    */
    bool HasFocus() const;

    /** When there is a shape that currently has the focus,
        i.e. <member>HasFocus()</member> returns <true/> then remove the
        focus from that shape.  Otherwise nothing changes.
    */
    void RemoveFocus();

    void ViewForwarderChanged();

private:
    rtl::Reference<ChildrenManagerImpl> mpImpl;

    ChildrenManager(const ChildrenManager&) = delete;
    ChildrenManager& operator=(const ChildrenManager&) = delete;
};

} // end of namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
