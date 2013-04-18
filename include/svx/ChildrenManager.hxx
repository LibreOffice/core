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

#ifndef _SVX_ACCESSIBILITY_CHILDREN_MANAGER_HXX
#define _SVX_ACCESSIBILITY_CHILDREN_MANAGER_HXX

#include <svx/IAccessibleViewForwarderListener.hxx>
#include <svx/AccessibleShapeTreeInfo.hxx>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <memory>
#include "svx/svxdllapi.h"

namespace accessibility {

class AccessibleContextBase;
class AccessibleShape;
class ChildrenManagerImpl;

/** The <type>AccessibleChildrenManager</type> class acts as a cache of the
    accessible objects of the currently visible shapes of a draw page and as
    a factory to create the corresponding accessible object for a given
    shape.

    <p>There are two sources of shapes.  The first is a list of UNO shapes.
    It is passes to the constructor and can be re-set with a call to
    <member>SetShapeList</member>.  Accessibility objects which represent
    these shapes are constructed usually on demand.  The second source is a
    set of single <type>AccessibleShape</type> objects added by calls to
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
    interpreting a <type>document::XEventListener</type>, the change from
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
class SVX_DLLPUBLIC ChildrenManager
    :   public IAccessibleViewForwarderListener
{
public:
    /** Create a children manager, which manages the children of the given
        parent.  The parent is used for creating accessible objects.  The
        list of shapes for which to create those objects is not derived from
        the parent and has to be provided seperately by calling one of the
        update methods.
        @param rxParent
            The parent of the accessible objects which will be created
            on demand at some point of time in the future.
        @param rxShapeList
            List of UNO shapes to manage.
        @param rShapeTreeInfo
            Bundel of information passed down the shape tree.
        @param rContext
            An accessible context object that is called for fireing events
            for new and deleted children, i.e. that holds a list of
            listeners to be informed.
    */
    ChildrenManager (const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible>& rxParent,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::XShapes>& rxShapeList,
        const AccessibleShapeTreeInfo& rShapeTreeInfo,
        AccessibleContextBase& rContext);

    /** If there still are managed children these are marked as DEFUNC and
        released.
    */
    virtual ~ChildrenManager (void);

    /** Return the number of currently visible accessible children.
        @return
            If there are no children a 0 is returned.
    */
    long GetChildCount (void) const throw ();

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
        @raises
            Throws an IndexOutOfBoundsException if the index is not valid.
    */
    ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible>
        GetChild (long nIndex)
        throw (::com::sun::star::uno::RuntimeException,
               ::com::sun::star::lang::IndexOutOfBoundsException);

    /** Update the child manager.  Take care of a modified set of children
        and modified visible area.  This method can optimize the update
        process with respect seperate updates of a modified children list
        and visible area.
        @param bCreateNewObjectsOnDemand
            If </true> then accessible objects associated with the visible
            shapes are created only when asked for.  No event is sent on
            creation.  If </false> then the accessible objects are created
            before this method returns and events are sent to inform the
            listeners of the new object.
    */
    void Update (bool bCreateNewObjectsOnDemand = true);

    /** Replace the list of UNO shapes by the specified list.
        @param xShapeList
            The new list of shapes.
    */
    void SetShapeList (const ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShapes>& xShapeList);

    /** Add an accessible shape.  The difference to the UNO shapes in the
        list passed to the constructor the specified object does not have to
        be created by the shape factory.  This gives the caller full control
        over object creation.

        @param pShape
            This class <em>does</em> take ownership of the argument.
    */
    void AddAccessibleShape (std::auto_ptr<AccessibleShape> pShape);

    /** Clear the list of accessible shapes which have been added by
        previous calls to <member>AddAccessibleShape</member>.
    */
    void ClearAccessibleShapeList (void);

    /** Take a new event shape tree info.  Call this method to inform the
        children manager of a change of the info bundle.
        @param rShapeTreeInfo
            The new info that replaces the current one.
    */
    void SetInfo (AccessibleShapeTreeInfo& rShapeTreeInfo);

    /** Update the SELECTED and FOCUSED states of all visible children
        according to the given selection.  This includes setting
        <em>and</em> resetting the states.
    */
    void UpdateSelection (void);

    /** Return whether one of the shapes managed by this object has
        currently the focus.
        @return
            Returns <true/> when there is a shape that has the focus and
            <false/> when there is no such shape.
    */
    bool HasFocus (void);

    /** When there is a shape that currently has the focus,
        i.e. <member>HasFocus()</member> returns <true/> then remove the
        focus from that shape.  Otherwise nothing changes.
    */
    void RemoveFocus (void);

    //=====  IAccessibleViewForwarderListener  ================================
    virtual void ViewForwarderChanged (ChangeType aChangeType,
        const IAccessibleViewForwarder* pViewForwarder);

protected:
    ChildrenManagerImpl* mpImpl;

private:
    // Don't use the copy constructor or the assignment operator.  They are
    // not implemented (and are not intended to be).
    SVX_DLLPRIVATE ChildrenManager (const ChildrenManager&);
    SVX_DLLPRIVATE ChildrenManager& operator= (const ChildrenManager&);
};

} // end of namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
