/*************************************************************************
 *
 *  $RCSfile: ChildrenManagerImpl.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: af $ $Date: 2002-04-18 16:38:26 $
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

#include "ChildrenManagerImpl.hxx"

#ifndef _SVX_ACCESSIBILITY_SHAPE_TYPE_HANDLER_HXX
#include "ShapeTypeHandler.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::accessibility;

namespace accessibility {

struct ChildDescriptor; // See below for declaration.

static AccessibleShapeTreeInfo aEmptyShapeTreeInfo;

//=====  AccessibleChildrenManager  ===========================================

ChildrenManagerImpl::ChildrenManagerImpl (
    const uno::Reference<XAccessible>& rxParent,
    const uno::Reference<drawing::XShapes>& rxShapeList,
    const AccessibleShapeTreeInfo& rShapeTreeInfo,
    AccessibleContextBase& rContext)
    : mxShapeList (rxShapeList),
      mxParent (rxParent),
      mrShapeTreeInfo (rShapeTreeInfo),
      mrContext (rContext)
{
    OSL_TRACE ("creating new children manager with %d children", rxShapeList->getCount());
}




ChildrenManagerImpl::~ChildrenManagerImpl (void)
{
    ClearAccessibleShapeList ();
    OSL_TRACE ("~ChildrenManagerImpl");
}




void ChildrenManagerImpl::Init (void)
{
    // Register as document::XEventListener.
    if (mrShapeTreeInfo.GetControllerBroadcaster().is())
        mrShapeTreeInfo.GetControllerBroadcaster()->addEventListener (
            static_cast<document::XEventListener*>(this));
}




long ChildrenManagerImpl::GetChildCount (void) const throw ()
{
    return maVisibleChildren.size();
}




/** Return the requested accessible child object.  Create it if it is not
    yet in the cache.
*/
uno::Reference<XAccessible>
    ChildrenManagerImpl::GetChild (long nIndex)
    throw (::com::sun::star::uno::RuntimeException)
{
    // Check wether the given index is valid.
    if (nIndex < 0 || (unsigned long)nIndex >= maVisibleChildren.size())
        throw lang::IndexOutOfBoundsException (
            ::rtl::OUString::createFromAscii ("no accessible child with index ") + nIndex,
            mxParent);

    return GetChild (maVisibleChildren[nIndex]);
}




/** Return the requested accessible child object.  Create it if it is not
    yet in the cache.
*/
uno::Reference<XAccessible>
    ChildrenManagerImpl::GetChild (ChildDescriptor& rChildDescriptor,
        bool bSendEventOnCreation)
    throw (::com::sun::star::uno::RuntimeException)
{
    bool bChildHasBeenCreated = false;

    if ( ! rChildDescriptor.mxAccessibleShape.is())
    {
        ::vos::OGuard aGuard (maMutex);
        //        ::osl::Guard< ::osl::Mutex> aGuard (::osl::Mutex::getGlobalMutex());
        // Make sure that the requested accessible object has not been
        // created while locking the global mutex.
        if ( ! rChildDescriptor.mxAccessibleShape.is())
        {
            // Create accessible object that corresponds to the descriptor's
            // shape.
            AccessibleShape* pShape =
                ShapeTypeHandler::Instance().CreateAccessibleObject (
                    rChildDescriptor.mxShape,
                    mxParent,
                    mrShapeTreeInfo);
            rChildDescriptor.mpAccessibleShape = pShape;
            rChildDescriptor.mxAccessibleShape = uno::Reference<XAccessible> (
                static_cast<uno::XWeak*>(pShape),
                uno::UNO_QUERY);
            // Remember that the child has been created to inform the
            // listeners after unlocking the global mutex.
            bChildHasBeenCreated = rChildDescriptor.mxAccessibleShape.is();
        }
    }
    /*
    if (bChildHasBeenCreated && bSendEventOnCreation && mxParent.is())
    {
        OSL_TRACE ("sending events for new shape");
        // Create event and inform listeners of the object creation.
        uno::Any aNewShape;
        aNewShape <<= rChildDescriptor.mxAccessibleShape;
        mrContext.CommitChange (
            AccessibleEventId::ACCESSIBLE_CHILD_EVENT,
            aNewShape,
            uno::Any());
        OSL_TRACE ("done sending events for new shape");
    }
    */
    return rChildDescriptor.mxAccessibleShape;
}




uno::Reference<XAccessible>
    ChildrenManagerImpl::GetChild (const uno::Reference<drawing::XShape>& xShape)
    throw (uno::RuntimeException)
{
    ChildDescriptorListType::iterator I;
    for (I=maVisibleChildren.begin(); I!=maVisibleChildren.end(); I++)
    {
        if (I->mxShape == xShape)
            return I->mxAccessibleShape;
    }
    return uno::Reference<XAccessible> ();
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
void ChildrenManagerImpl::Update (bool bCreateNewObjectsOnDemand)
{
    Rectangle aVisibleArea = mrShapeTreeInfo.GetViewForwarder()->GetVisibleArea();
    OSL_TRACE ("ChildrenManagerImpl::update called with VisibleArea = %d %d %d %d",
        aVisibleArea.getX(), aVisibleArea.getY(),
        aVisibleArea.getWidth(), aVisibleArea.getHeight());

    // 1. Create a local list of visible shapes.
    ChildDescriptorListType aNewChildList;
    CreateListOfVisibleShapes (aNewChildList);

    // 2. Find all shapes in the current list that are not in the new list,
    // send appropriate events and remove the accessible shape.
    RemoveNonVisibleChildren (aNewChildList);

    // 3. Merge the information that is already known about the visible
    // shapes from the current list into the new list.
    MergeAccessibilityInformation (aNewChildList);

    // 4. If the visible area has changed then send events that signal a
    // change of their bounding boxes for all shapes that are members of
    // both the current and the new list of visible shapes.
    if (maVisibleArea != aVisibleArea)
        SendVisibleAreaEvents (aNewChildList);

    // 5. If children have to be created immediately and not on demand the
    // create the missing accessible objects now.
    if ( ! bCreateNewObjectsOnDemand)
        CreateAccessibilityObjects (aNewChildList);

    // 6. Replace the current list of visible shapes with the new one.  Do
    // the same with the visible area.
    ::vos::OGuard aGuard (maMutex);

    // Remember the new list of children.
    maVisibleChildren = aNewChildList;

    // Remember the new list and visible area for later use.
    maVisibleArea = aVisibleArea;
}




void ChildrenManagerImpl::CreateListOfVisibleShapes (
    ChildDescriptorListType& raDescriptorList)
{
    ::vos::OGuard aGuard (maMutex);

    // Add the visible shapes for wich the accessible objects already exist.
    vector<AccessibleShape*>::iterator I;
    for (I=maAccessibleShapes.begin(); I!=maAccessibleShapes.end(); ++I)
    {
        raDescriptorList.push_back (ChildDescriptor (*I));
    }

    uno::Reference<container::XIndexAccess> xShapeAccess (mxShapeList, uno::UNO_QUERY);
    if (xShapeAccess.is())
    {
        sal_Int32 nShapeCount = xShapeAccess->getCount();
        Rectangle aVisibleArea = mrShapeTreeInfo.GetViewForwarder()->GetVisibleArea();
        for (sal_Int32 i=0; i<nShapeCount; i++)
        {
            uno::Reference<drawing::XShape> xShape;
            xShapeAccess->getByIndex(i) >>= xShape;
            Rectangle aBoundingBox (
                xShape->getPosition().X,
                xShape->getPosition().Y,
                xShape->getPosition().X + xShape->getSize().Width,
                xShape->getPosition().Y + xShape->getSize().Height);
            // Insert shape if it is visible, i.e. its bounding box overlaps
            // the visible area.
            if (aBoundingBox.IsOver (aVisibleArea))
                raDescriptorList.push_back (ChildDescriptor (xShape));
            else
                OSL_TRACE ("shape %d outside visible area (%d,%d,%d,%d)",
                    i,
                    xShape->getPosition().X,
                    xShape->getPosition().Y,
                    xShape->getSize().Width,
                    xShape->getSize().Height);
        }
    }
}




void ChildrenManagerImpl::RemoveNonVisibleChildren (
    ChildDescriptorListType& raNewChildList)
{
    // Iterate over list of formerly visible children and remove those that
    // are not visible, i.e. member of the new list of visible children,
    // anymore.
    ChildDescriptorListType::iterator I;
    for (I=maVisibleChildren.begin(); I!=maVisibleChildren.end(); I++)
    {
        if (find (raNewChildList.begin(), raNewChildList.end(), *I) == raNewChildList.end())
        {
            OSL_TRACE ("child no longer visible");
            if (I->mxAccessibleShape.is())
            {
                // Send event that the shape has been removed.
                uno::Any aOldValue;
                aOldValue <<= I->mxAccessibleShape;
                mrContext.CommitChange (
                    AccessibleEventId::ACCESSIBLE_CHILD_EVENT,
                    uno::Any(),
                    aOldValue);

                // Remove the accessible object.
                I->mxAccessibleShape = NULL;
            }
        }
    }
}




void ChildrenManagerImpl::MergeAccessibilityInformation (
    ChildDescriptorListType& raNewChildList)
{
    ChildDescriptorListType::iterator aOldChildDescriptor;
    ChildDescriptorListType::iterator I;
    for (I=raNewChildList.begin(); I!=raNewChildList.end(); I++)
    {
        aOldChildDescriptor = find (maVisibleChildren.begin(), maVisibleChildren.end(), *I);

        // Copy accessible shape if that exists in the old descriptor.
        if (aOldChildDescriptor != maVisibleChildren.end())
            if (aOldChildDescriptor->mxAccessibleShape.is())
            {
                I->mxAccessibleShape = aOldChildDescriptor->mxAccessibleShape;
                I->mpAccessibleShape = aOldChildDescriptor->mpAccessibleShape;
                I->mbCreateEventPending = false;
            }
    }
}




void ChildrenManagerImpl::SendVisibleAreaEvents (
    ChildDescriptorListType& raNewChildList)
{
    ChildDescriptorListType::iterator I;
    for (I=raNewChildList.begin(); I!=raNewChildList.end(); I++)
    {
        // Tell shape of changed visible area.  To do this, fake a
        // change of the view forwarder.  (Actually we usually get here
        // as a result of a change of the view forwarder).
        if (I->mpAccessibleShape != NULL)
            I->mpAccessibleShape->ViewForwarderChanged (
                IAccessibleViewForwarderListener::VISIBLE_AREA,
                mrShapeTreeInfo.GetViewForwarder());
    }
}




void ChildrenManagerImpl::CreateAccessibilityObjects (
    ChildDescriptorListType& raNewChildList)
{
    ChildDescriptorListType::iterator I;
    for (I=raNewChildList.begin(); I!=raNewChildList.end(); I++)
    {
        // Create the associated accessible object when the flag says so and
        // it does not yet exist.
        if ( ! I->mxAccessibleShape.is())
            GetChild (*I);
        if (I->mxAccessibleShape.is() && I->mbCreateEventPending)
        {
            OSL_TRACE ("sending events for new shape");
            uno::Any aNewShape;
            aNewShape <<= I->mxAccessibleShape;
            mrContext.CommitChange (
                AccessibleEventId::ACCESSIBLE_CHILD_EVENT,
                aNewShape,
                uno::Any());
            OSL_TRACE ("done sending events for new shape");
        }
    }
}




void ChildrenManagerImpl::SetShapeList (const ::com::sun::star::uno::Reference<
    ::com::sun::star::drawing::XShapes>& xShapeList)
{
    mxShapeList = xShapeList;
}




void ChildrenManagerImpl::AddAccessibleShape (AccessibleShape* pShape)
{
    if (pShape != NULL)
        maAccessibleShapes.push_back (pShape);
}




void ChildrenManagerImpl::ClearAccessibleShapeList (void)
{
    maAccessibleShapes.clear ();

    // Send events that tell the listeners that the current children are
    // removed.
    ChildDescriptorListType::iterator I;
    for (I=maVisibleChildren.begin(); I!=maVisibleChildren.end(); I++)
        if (I->mxAccessibleShape.is())
        {
            uno::Any aShape;
            aShape <<= I->mxAccessibleShape;
            mrContext.CommitChange (
                AccessibleEventId::ACCESSIBLE_CHILD_EVENT,
                uno::Any(),
                aShape);
        }
    maVisibleChildren.clear ();
}




void ChildrenManagerImpl::SetInfo (AccessibleShapeTreeInfo& rShapeTreeInfo)
{
    uno::Reference<document::XEventBroadcaster> xCurrentBroadcaster;
    {
        ::vos::OGuard aGuard (maMutex);
        xCurrentBroadcaster = mrShapeTreeInfo.GetControllerBroadcaster();
        mrShapeTreeInfo = rShapeTreeInfo;
    }

    if (mrShapeTreeInfo.GetControllerBroadcaster() != xCurrentBroadcaster)
    {
        // Register at new broadcaster.
        if (mrShapeTreeInfo.GetControllerBroadcaster().is())
            mrShapeTreeInfo.GetControllerBroadcaster()->addEventListener (
                static_cast<document::XEventListener*>(this));

        // Unregister at old broadcaster.
        if (xCurrentBroadcaster.is())
            xCurrentBroadcaster->removeEventListener (
                static_cast<document::XEventListener*>(this));
    }
}




//=====  lang::XEventListener  ================================================

void SAL_CALL
    ChildrenManagerImpl::disposing (const lang::EventObject& rEventObject)
    throw (uno::RuntimeException)
{
    // Not yet interested in disposing events.
}




//=====  document::XEventListener  ============================================

void SAL_CALL
    ChildrenManagerImpl::notifyEvent (const document::EventObject& rEventObject)
    throw (uno::RuntimeException)
{
    OSL_TRACE ("ChildrenManagerImpl::notifyEvent");
    const OUString sShapeModified (RTL_CONSTASCII_USTRINGPARAM("ShapeModified"));
    if (rEventObject.EventName.equals (sShapeModified))
    {
        OSL_TRACE ("  Is ShapeNotified event");
        // Some property of a shape has been modified.  Find the associated
        // accessible object and send an event that indicates a change of the
        // visible data to all listeners.
        uno::Reference<drawing::XShape> xShape (rEventObject.Source, uno::UNO_QUERY);
        ::vos::OGuard aGuard (maMutex);
        long int i,
            nCount = maVisibleChildren.size();
        for (i=0; i<nCount; i++)
        {
            if (maVisibleChildren[i].mxShape == xShape)
            {
                OSL_TRACE ("   Found accessible object for shape.");
                ChildDescriptor& rChildDescriptor = maVisibleChildren[i];
                if (rChildDescriptor.mpAccessibleShape != NULL)
                    rChildDescriptor.mpAccessibleShape->CommitChange (
                        AccessibleEventId::ACCESSIBLE_VISIBLE_DATA_EVENT,
                        uno::Any(),
                        uno::Any());
                break;
            }
        }
    }
}




// This method is experimental.  Use with care.
long int ChildrenManagerImpl::GetChildIndex (const ::com::sun::star::uno::Reference<
    ::drafts::com::sun::star::accessibility::XAccessible>& xChild) const
    throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard (maMutex);
    for (long i=0; i<maVisibleChildren.size(); i++)
    {
        // Is this equality comparison valid?
        if (maVisibleChildren[i].mxAccessibleShape == xChild)
            return i;
    }

    return -1;
}




//=====  IAccessibleViewForwarderListener  ====================================
void ChildrenManagerImpl::ViewForwarderChanged (ChangeType aChangeType,
        const IAccessibleViewForwarder* pViewForwarder)
{
    if (aChangeType == IAccessibleViewForwarderListener::VISIBLE_AREA)
        Update (false);
    else
    {
        ::vos::OGuard aGuard (maMutex);
        for (long i=0; i<maVisibleChildren.size(); i++)
            if (maVisibleChildren[i].mpAccessibleShape != NULL)
                maVisibleChildren[i].mpAccessibleShape->ViewForwarderChanged (
                    aChangeType,
                    pViewForwarder);
    }
}




//=====  AccessibleChildDescriptor  ===========================================

ChildDescriptor::ChildDescriptor (const uno::Reference<
        drawing::XShape>& xShape)
    : mxShape (xShape),
      mpAccessibleShape (NULL),
      mbCreateEventPending (true)
{
    // Empty.
}

ChildDescriptor::ChildDescriptor (AccessibleShape* pShape)
    : mxShape (NULL),
      mpAccessibleShape (pShape),
      mxAccessibleShape (pShape),
      mbCreateEventPending (true)
{
    // Empty.
}





} // end of namespace accessibility
