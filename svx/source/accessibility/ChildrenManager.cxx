/*************************************************************************
 *
 *  $RCSfile: ChildrenManager.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: af $ $Date: 2002-03-18 10:16:42 $
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

#ifndef _SVX_ACCESSIBILITY_CHILDREN_MANAGER_HXX
#include "ChildrenManager.hxx"
#endif
#ifndef _SVX_ACCESSIBILITY_SHAPE_TYPE_HANDLER_HXX
#include "ShapeTypeHandler.hxx"
#endif
#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_CONTEXT_BASE_HXX
#include "AccessibleContextBase.hxx"
#endif

#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBLE_ACCESSIBLEEVENTID_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleEventId.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XIndexAccess_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_INDEXOUTOFBOUNDSEXCEPTION_HPP_
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#endif


#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::accessibility;

namespace accessibility {


//=====  AccessibleChildrenManager  ===========================================

ChildrenManager::ChildrenManager (
    const uno::Reference<XAccessible>& rxParent,
    AccessibleShapeTreeInfo& rShapeTreeInfo,
    AccessibleContextBase& rContext)
    : mxParent (rxParent),
      mrShapeTreeInfo (rShapeTreeInfo),
      mrContext (rContext)
{
    /*    // Register as document::XEventListener.
    if (mrShapeTreeInfo.getControllerBroadcaster().is())
        mrShapeTreeInfo.getControllerBroadcaster()->addEventListener (
            static_cast<document::XEventListener*>(this));
    */
}




ChildrenManager::~ChildrenManager (void)
{
    // emtpy
    OSL_TRACE ("~ChildrenManager");
}




long ChildrenManager::GetChildCount (void) const throw ()
{
    return maChildDescriptorList.size();
}




/** Return the requested accessible child object.  Create it if it is not
    yet in the cache.
*/
::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::accessibility::XAccessible>
    ChildrenManager::GetChild (long nIndex)
    throw (::com::sun::star::uno::RuntimeException)
{
    // Check wether the given index is valid.
    if (nIndex < 0 || (unsigned long)nIndex >= maChildDescriptorList.size())
        throw lang::IndexOutOfBoundsException (
            ::rtl::OUString::createFromAscii ("no accessible child with index " + nIndex),
            mxParent);

    return GetChild (maChildDescriptorList[nIndex]);
}




/** Return the requested accessible child object.  Create it if it is not
    yet in the cache.
*/
::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::accessibility::XAccessible>
    ChildrenManager::GetChild (ChildDescriptor& aChildDescriptor)
    throw (::com::sun::star::uno::RuntimeException)
{
    bool bChildHasBeenCreated = false;

    if ( ! aChildDescriptor.mxAccessibleShape.is())
    {
        ::vos::OGuard aGuard (maMutex);
        //        ::osl::Guard< ::osl::Mutex> aGuard (::osl::Mutex::getGlobalMutex());
        // Make sure that the requested accessible object has not been
        // created while locking the global mutex.
        if ( ! aChildDescriptor.mxAccessibleShape.is())
        {
            // Create accessible object that corresponds to the descriptor's
            // shape.
            aChildDescriptor.mxAccessibleShape =
                ShapeTypeHandler::Instance().CreateAccessibleObject (
                    aChildDescriptor.mxShape,
                    mxParent,
                    mrShapeTreeInfo);
            // Remember that the child has been created to inform the
            // listeners after unlocking the global mutex.
            bChildHasBeenCreated = aChildDescriptor.mxAccessibleShape.is();
        }
    }

    if (bChildHasBeenCreated && mxParent.is())
    {
        OSL_TRACE ("sending events for new shape");
        // Create event and inform listeners of the object creation.
        uno::Any aNewValue,
            aOldValue;
        aOldValue <<= uno::Reference<XAccessible>();
        aNewValue <<= aChildDescriptor.mxAccessibleShape;
        AccessibleEventObject aEvent (
            mxParent,
            AccessibleEventId::ACCESSIBLE_CHILD_EVENT,
            aNewValue,
            aOldValue);
        mrContext.FireEvent (aEvent);
        OSL_TRACE ("done sending events for new shape");
    }

    return aChildDescriptor.mxAccessibleShape;
}




uno::Reference<XAccessible>
    ChildrenManager::GetChild (const uno::Reference<drawing::XShape>& xShape)
    throw (uno::RuntimeException)
{
    ChildDescriptorListType::iterator I;
    for (I=maChildDescriptorList.begin(); I!=maChildDescriptorList.end(); I++)
    {
        if (I->mxShape == xShape)
            return I->mxAccessibleShape;
    }
    return uno::Reference<XAccessible> ();
}




/** For now forward the update of the shape list to the general update
    method.
*/
void ChildrenManager::Update (const ::com::sun::star::uno::Reference<
    ::com::sun::star::drawing::XShapes>& xShapeList,
        bool bCreateNewObjectsOnDemand)
{
    Update (xShapeList, maVisibleArea, bCreateNewObjectsOnDemand);
}




/** For now forward the update of the visible area to the general update
    method.
*/
void ChildrenManager::Update (const Rectangle& aVisibleArea,
        bool bCreateNewObjectsOnDemand)
{
    Update (mxOriginalShapeList, aVisibleArea, bCreateNewObjectsOnDemand);
}




/** Find all shapes among the specified shapes that lie fully or partially
    inside the visible area.  Put those shapes into the cleared cache. The
    corresponding accessible objects will be created on demand.

    At the moment, first all accessible objects are removed from the cache
    and the appropriate listeners are informed of this.  Next, the list is
    created again.  This should be optimized in the future to not remove and
    create objects that will be in the list before and after the update
    method.
*/
void ChildrenManager::Update (const ::com::sun::star::uno::Reference<
    ::com::sun::star::drawing::XShapes>& xShapeList,
    const Rectangle& aVisibleArea,
        bool bCreateNewObjectsOnDemand)
{
    OSL_TRACE ("ChildrenManager::update called with VisibleArea = %d %d %d %d",
        aVisibleArea.getX(), aVisibleArea.getY(),
        aVisibleArea.getWidth(), aVisibleArea.getHeight());

    // 1. Create a local list of visible shapes.
    ChildDescriptorListType aNewChildList;
    uno::Reference<container::XIndexAccess> xIA (xShapeList, uno::UNO_QUERY);
    if (xIA.is())
    {
        long n = xIA->getCount();
        for (long i=0; i<n; i++)
        {
            uno::Reference<drawing::XShape> xShape (xIA->getByIndex(i), uno::UNO_QUERY);
            Rectangle aBoundingBox (
                xShape->getPosition().X,
                xShape->getPosition().Y,
                xShape->getSize().Width,
                xShape->getSize().Height);
            if (aBoundingBox.IsOver (aVisibleArea))
                aNewChildList.push_back (ChildDescriptor (xShape));
        }
    }

    // 2. Find all shapes in the current list that are not in the new list
    // and send appropriate events.
    ChildDescriptorListType::iterator I;
    for (I=maChildDescriptorList.begin(); I!=maChildDescriptorList.end(); I++)
    {
        if (find (aNewChildList.begin(), aNewChildList.end(), *I) == aNewChildList.end())
        {
            if (I->mxAccessibleShape.is())
            {
                // Send event that the shape has been removed.
                uno::Any aNewValue,
                    aOldValue;
                aOldValue <<= I->mxAccessibleShape;
                aNewValue <<= uno::Reference<XAccessible>();
                AccessibleEventObject aEvent (
                    mxParent,
                    AccessibleEventId::ACCESSIBLE_CHILD_EVENT,
                    aNewValue,
                    aOldValue);
                mrContext.FireEvent (aEvent);

                // Remove the accessible object.
                I->mxAccessibleShape = NULL;
            }
        }
    }

    mrShapeTreeInfo.CalcCSChangeTransformation ();

    // 3. Find all shapes in the new list that are not in the old list and
    // send appropriate events.
    // 4. If the visible area has changed then send events that denote a
    // change of their bounding boxes for all shapes that have been in the
    // old list as well,
    bool bVisibleAreaChanged = (maVisibleArea != aVisibleArea);
    ChildDescriptorListType::iterator aOldChildDescriptor;
    for (I=aNewChildList.begin(); I!=aNewChildList.end(); I++)
    {
        aOldChildDescriptor = find (maChildDescriptorList.begin(), maChildDescriptorList.end(), *I);

        // Copy accessible shape if that exists in the old descriptor.
        if (aOldChildDescriptor != maChildDescriptorList.end())
            if (aOldChildDescriptor->mxAccessibleShape.is())
                I->mxAccessibleShape = aOldChildDescriptor->mxAccessibleShape;

        // Create the associated accessible object when the flag says so and
        // it does not yet exist.
        if ( ! I->mxAccessibleShape.is())
        {
            if ( ! bCreateNewObjectsOnDemand)
                GetChild (*I);
        }
        else if (bVisibleAreaChanged)
        {
            // Send visual data change event.
            AccessibleEventObject aEvent (
                I->mxAccessibleShape,
                AccessibleEventId::ACCESSIBLE_VISIBLE_DATA_EVENT,
                uno::Any(),
                uno::Any());
            mrContext.FireEvent (aEvent);
        }
    }

    // 5. Copy the new data into this object's members.
    {
        ::vos::OGuard aGuard (maMutex);

        // Clear the list of children.
        maChildDescriptorList = aNewChildList;

        // Remember the new list and visible area for later use.
        mxOriginalShapeList = xShapeList;
        maVisibleArea = aVisibleArea;
    }
}



/*
void ChildrenManager::update_old (const ::com::sun::star::uno::Reference<
    ::com::sun::star::drawing::XShapes>& xShapeList,
    const Rectangle& aVisibleArea,
        bool bCreateNewObjectsOnDemand)
{
    // Iterate over all children and create child events denoting
    // that the children are about to be removed.
    ChildDescriptorListType::iterator I;
    for (I=maChildDescriptorList.begin(); I!=maChildDescriptorList.end(); I++)
    {
        // Create event and inform listeners of object removal.
        if (I->mxAccessibleShape.is())
        {
            uno::Any aNewValue,
                aOldValue;
            aOldValue <<= I->mxAccessibleShape;
            aNewValue <<= uno::Reference<XAccessible>();
            AccessibleEventObject aEvent (
                mxParent,
                AccessibleEventId::ACCESSIBLE_CHILD_EVENT,
                aNewValue,
                aOldValue);
            mrContext.fireEvent (aEvent);
        }
    }

    {
        ::vos::OGuard aGuard (maMutex);

        // Clear the list of children.
        maChildDescriptorList.clear ();

        // Remember the new list and visible area for later use.
        mxOriginalShapeList = xShapeList;
        maVisibleArea = aVisibleArea;
    }

    // Determine all fully and partially visible shapes and add those to the list
    // of managed children.
    uno::Reference<container::XIndexAccess> xIA (mxOriginalShapeList, uno::UNO_QUERY);
    if (xIA.is())
    {
        long n = xIA->getCount();
        for (long i=0; i<n; i++)
        {
            uno::Reference<drawing::XShape> xShape (xIA->getByIndex(i), uno::UNO_QUERY);
            Rectangle aBoundingBox (
                xShape->getPosition().X,
                xShape->getPosition().Y,
                xShape->getSize().Width,
                xShape->getSize().Height);
            if (aBoundingBox.IsOver (maVisibleArea))
            {
                {
                ::vos::OGuard aGuard (maMutex);

                // Add shape to list of visible children.
                maChildDescriptorList.push_back (ChildDescriptor (xShape));
                }

                // Create the associated accessible object when the flag says so.
                if ( ! bCreateNewObjectsOnDemand)
                    getChild (maChildDescriptorList.back());
            }
        }
    }
}
*/



void ChildrenManager::SetInfo (AccessibleShapeTreeInfo& rShapeTreeInfo)
{
    uno::Reference<document::XEventBroadcaster> xCurrentBroadcaster;
    {
        ::vos::OGuard aGuard (maMutex);
        xCurrentBroadcaster = mrShapeTreeInfo.GetControllerBroadcaster();
        mrShapeTreeInfo = rShapeTreeInfo;
    }

    // Register at new broadcaster.
    if (mrShapeTreeInfo.GetControllerBroadcaster().is())
        mrShapeTreeInfo.GetControllerBroadcaster()->addEventListener (
            static_cast<document::XEventListener*>(this));

    // Unregister at old broadcaster.
    if (xCurrentBroadcaster.is())
        xCurrentBroadcaster->removeEventListener (
            static_cast<document::XEventListener*>(this));

    /*    ChildDescriptorListType::iterator I;
    for (I=maChildDescriptorList.begin(); I!=maChildDescriptorList.end(); I++)
        if (I->mxAccessibleShape.is())
            I->mxAccessible->setInfo (rShapeTreeInfo);
    */
}




//=====  lang::XEventListener  ================================================

void SAL_CALL
    ChildrenManager::disposing (const lang::EventObject& rEventObject)
    throw (uno::RuntimeException)
{
    // Not yet interested in disposing events.
}




//=====  document::XEventListener  ============================================

void SAL_CALL
    ChildrenManager::notifyEvent (const document::EventObject& rEventObject)
    throw (uno::RuntimeException)
{
    OSL_TRACE ("notifyEvent");
    const OUString sShapeModified (RTL_CONSTASCII_USTRINGPARAM ("ShapeModified"));
    if (rEventObject.EventName.equals (sShapeModified))
    {
        OSL_TRACE ("  Is ShapeNotified event");
        // Some property of a shape has been modified.  Find the associated
        // accessible object and send an event that indicates a change of the
        // visible data to all listeners.
        uno::Reference<drawing::XShape> xShape (rEventObject.Source, uno::UNO_QUERY);
        uno::Any aOldValue;
        aOldValue <<= xShape; // Test
        uno::Reference<XAccessible> xAccessibleShape = GetChild (xShape);
        if (xAccessibleShape.is())
        {
            AccessibleEventObject aEvent (
                xAccessibleShape,
                AccessibleEventId::ACCESSIBLE_VISIBLE_DATA_EVENT,
                uno::Any(),
                aOldValue);
            mrContext.FireEvent (aEvent);
        }
        else
            OSL_TRACE ("   Could not find accessible object for shape.");
    }
}




// This method is experimental.  Use with care.
long int ChildrenManager::GetChildIndex (const ::com::sun::star::uno::Reference<
    ::drafts::com::sun::star::accessibility::XAccessible>& xChild) const
    throw (::com::sun::star::uno::RuntimeException)
{
    for (long i=0; i<maChildDescriptorList.size(); i++)
    {
        // Is this euqality comparison valid?
        if (maChildDescriptorList[i].mxAccessibleShape == xChild)
            return i;
    }

    return -1;
}




//=====  AccessibleChildDescriptor  ===========================================

ChildDescriptor::ChildDescriptor (const uno::Reference<
        drawing::XShape>& xShape)
    : mxShape (xShape)
{
    // Empty.
}


} // end of namespace accessibility
