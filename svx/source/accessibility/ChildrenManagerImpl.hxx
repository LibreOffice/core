/*************************************************************************
 *
 *  $RCSfile: ChildrenManagerImpl.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: af $ $Date: 2002-04-11 12:59:54 $
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

#ifndef _SVX_ACCESSIBILITY_CHILDREN_MANAGER_IMPL_HXX

#ifndef _SVX_ACCESSIBILITY_IACCESSIBLE_VIEW_FORWARDER_LISTENER_HXX
#include "IAccessibleViewForwarderListener.hxx"
#endif
#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_SHAPE_TREE_INFO_HXX
#include "AccessibleShapeTreeInfo.hxx"
#endif
#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_CONTEXT_BASE_HXX
#include "AccessibleContextBase.hxx"
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#include <vos/mutex.hxx>
#include <vector>
#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTLISTENER_HPP_
#include <com/sun/star/document/XEventListener.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBLE_XACCESSIBLE_HPP_
#include <drafts/com/sun/star/accessibility/XAccessible.hpp>
#endif
#include <vector>


using namespace ::rtl;
using namespace ::com::sun::star;

namespace accessibility {

class AccessibleShape;

class ChildDescriptor; // See below for declaration.
class CMShapeIterator;


/** This class contains the actual implementation of the children manager.

    <p>It maintains a set of shapes that is made up of single shapes stored
    in <member>maShapes</member> and lists of shapes
    <member>maShapeLists</member>.  These lists can be manipulated with the
    <member>AddShapeList</member>, <member>RemoveShape</member>,
    <member>RemoveShapeList</member> methods.  The actual set of children
    accessible through the <member>GetChildrenCount</member> and
    <member>GetChild</member> methods is the subset of shapes that lie
    completely or partially inside the visible area obtained from the view
    forwarder in the shape tree info.</p>

    @see ChildrenManager
*/
class ChildrenManagerImpl
    :   public cppu::WeakImplHelper1< ::com::sun::star::document::XEventListener>,
        public IAccessibleViewForwarderListener
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
    ChildrenManagerImpl (const ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::accessibility::XAccessible>& rxParent,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::XShapes>& rxShapeList,
        AccessibleShapeTreeInfo& rShapeTreeInfo,
        AccessibleContextBase& rContext);

    /** If there still are managed children these are marked as DEFUNC and
        released.
    */
    ~ChildrenManagerImpl (void) SAL_THROW (());

    /** Do that part of the initialization that you can not or should not do
        in the constructor like registering at broadcasters.
    */
    void Init (void);

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
            ::drafts::com::sun::star::accessibility::XAccessible>
        GetChild (long nIndex)
        throw (::com::sun::star::uno::RuntimeException);

    /** Return the requested accessible child.
        @param aChildDescriptor
            This object contains references to the original shape and its
            associated accessible object.
        @param bSendEventOnCreation
            If true and the requested child has to be created then send an
            event.  Otherwise, the default, be quiet.
        @return
            Returns a reference to the requested accessible child.  This
            reference is empty if it has not been possible to create the
            accessible object of the corresponding shape.
    */
    ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::accessibility::XAccessible>
        GetChild (ChildDescriptor& aChildDescriptor,
            bool bSendEventOnCreation = false)
        throw (::com::sun::star::uno::RuntimeException);

    /** Return the requested accessible child given a shape.  This method
        searches the list of descriptors for the one that holds the
        association of the given shape to the requested accessible object
        and returns that.  If no such descriptor is found that is
        interpreted so that the specified shape is not visible at the moment.
        @param xShape
            The shape for which to return the associated accessible object.
        @return
            Returns a reference to the requested accessible child.  The
            reference is empty if there is no shape descriptor that
            associates the shape with an accessible object.
    */
    ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::accessibility::XAccessible>
        GetChild (const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::XShape>& xShape)
        throw (::com::sun::star::uno::RuntimeException);

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

    void SetShapeList (const ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShapes>& xShapeList);

    void AddAccessibleShape (AccessibleShape* pShape);

    void ClearAccessibleShapeList (void);

    /** Take a new event shape tree info.  Call this method to inform the
        children manager of a change of the info bundle.
        @param rShapeTreeInfo
            The new info that replaces the current one.
    */
    void SetInfo (AccessibleShapeTreeInfo& rShapeTreeInfo);

    //=====  lang::XEventListener  ============================================

    virtual void SAL_CALL
        disposing (const ::com::sun::star::lang::EventObject& rEventObject)
        throw (::com::sun::star::uno::RuntimeException);


    //=====  document::XEventListener  ========================================

    virtual void SAL_CALL
        notifyEvent (const ::com::sun::star::document::EventObject& rEventObject)
        throw (::com::sun::star::uno::RuntimeException);

    //=====  IAccessibleViewForwarderListener  ================================
    virtual void ViewForwarderChanged (ChangeType aChangeType,
        const IAccessibleViewForwarder* pViewForwarder);


protected:
    friend class CMShapeIterator;

    /// Mutex guarding objects of this class.
    mutable ::vos::OMutex maMutex;

    /** This list holds the descriptors of all currently visible shapes and
        associated accessible object.

        <p>With the descriptors it maintains a mapping of shapes to
        accessible objects.  It acts as a cache in that accessible objects
        are only created on demand and released with every update (where the
        latter may be optimized by the update methods).<p>

        <p>The list is realized as a vector because it remains unchanged
        between updates (i.e. complete rebuilds of the list) and allows a
        fast (constant time) access to its elements for given indices.</p>
    */
    typedef ::std::vector<ChildDescriptor> ChildDescriptorListType;
    ChildDescriptorListType maVisibleChildren;

    /** The original list of UNO shapes.  The visible shapes are inserted
        into the list of visible children
        <member>maVisibleChildren</member>.
    */
    ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShapes> mxShapeList;

    /** This list of additional accessible shapes that can or shall not be
        created by the shape factory.  All visible accessible shapes in this
        list are inserted into the list of visible shapes
        <member>maVisibleChildren</member>.
    */
    std::vector<AccessibleShape*> maAccessibleShapes;

    /** Rectangle that describes the visible area in which a shape has to lie
        at least partly, to be accessible through this class.
    */
    Rectangle maVisibleArea;

    /** The parent of the shapes.  It is used for creating accessible
        objects for given shapes.
    */
    ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::accessibility::XAccessible> mxParent;

    /** Bundel of information passed down the shape tree.
    */
    AccessibleShapeTreeInfo& mrShapeTreeInfo;

    /** Reference to an accessible context object that is used to inform its
        listeners of new and remved children.
    */
    AccessibleContextBase& mrContext;

    /** Experimental: Get the index of the specified accessible object with
        respect to the list of children maintained by this object.

        @return
            Return the index of the given child or -1 to indicate that the
            child is unknown.
    */
    long GetChildIndex (const ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::accessibility::XAccessible>& xChild) const
        throw (::com::sun::star::uno::RuntimeException);

private:
    // Don't use the copy constructor or the assignment operator.  They are
    // not implemented (and are not intended to be).
    ChildrenManagerImpl (const ChildrenManagerImpl&);
    ChildrenManagerImpl& operator= (const ChildrenManagerImpl&);

    /** Three helper functions for the <member>Update</member> method.
    */

    /** Create a list of visible shapes from the list of UNO shapes
        <member>maShapeList</member> and the list of accessible objects.
        @param raDescriptorList
            For every visible shape from the two sources mentioned above one
            descriptor is added to this list.
    */
    void CreateListOfVisibleShapes (ChildDescriptorListType& raDescriptorList);

    /** From the internal list of (former) visible shapes remove those that
        are not member of the given list.  Send appropriate events for every
        such shape.
        @param raDescriptorList
            The new list of visible children againt which the internal one
            is compared.
    */
    void RemoveNonVisibleChildren (ChildDescriptorListType& raChildList);

    /** Merge the information that is already known about the visible shapes
        from the current list into the new list.
    */
    void MergeAccessibilityInformation (ChildDescriptorListType& raChildList);

    /** If the visible area has changed then send events that signal a
        change of their bounding boxes for all shapes that are members of
        both the current and the new list of visible shapes.
    */
    void SendVisibleAreaEvents (ChildDescriptorListType& raChildList);

    /** If children have to be created immediately and not on demand the
        create the missing accessible objects now.
    */
    void CreateAccessibilityObjects (ChildDescriptorListType& raNewChildList);
};




/** Pair for each visible shape, that contains a reference to the shape a
    reference to the corresponding accessible object.  The accessible object
    is only created on demand and is initially empty.
*/
class ChildDescriptor
{
public:
    /** Reference to a (partially) visible shape.
    */
    ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShape> mxShape;

    /** The corresponding accessible object.  This reference is initially
        empty and only replaced by a reference to a new object when that is
        requested from the outside.
    */
    ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::accessibility::XAccessible> mxAccessibleShape;

    AccessibleShape* mpAccessibleShape;

    /** Create a new descriptor for the specified shape with empty reference
        to accessible object.
    */
    explicit ChildDescriptor (const ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShape>& xShape);

    explicit ChildDescriptor (AccessibleShape* pShape);

    bool operator == (const ChildDescriptor& aDescriptor)
    {
        return mxShape == aDescriptor.mxShape;
    }
    bool operator < (const ChildDescriptor& aDescriptor)
    {
        return mxShape < aDescriptor.mxShape;
    }
};



} // end of namespace accessibility

#endif
