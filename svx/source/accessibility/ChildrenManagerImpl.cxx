/*************************************************************************
 *
 *  $RCSfile: ChildrenManagerImpl.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: af $ $Date: 2002-06-03 15:18:44 $
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
#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_SHAPE_INFO_HXX
#include "AccessibleShapeInfo.hxx"
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBLE_ACCESSIBLESTATETYPE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONSUPPLIER_HPP_
#include <com/sun/star/view/XSelectionSupplier.hpp>
#endif

#include <rtl/ustring.hxx>

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::accessibility;
using ::com::sun::star::uno::Reference;

namespace accessibility {

//=====  AccessibleChildrenManager  ===========================================

ChildrenManagerImpl::ChildrenManagerImpl (
    const uno::Reference<XAccessible>& rxParent,
    const uno::Reference<drawing::XShapes>& rxShapeList,
    const AccessibleShapeTreeInfo& rShapeTreeInfo,
    AccessibleContextBase& rContext)
    : mxShapeList (rxShapeList),
      mxParent (rxParent),
      maShapeTreeInfo (rShapeTreeInfo),
      mrContext (rContext)
{
    OSL_TRACE ("creating new children manager with %d children", rxShapeList->getCount());
}




ChildrenManagerImpl::~ChildrenManagerImpl (void)
{
    // Remove from broadcasters.
    Reference<view::XSelectionSupplier> xSelectionSupplier (
        maShapeTreeInfo.GetController(), uno::UNO_QUERY);
    if (xSelectionSupplier.is())
        xSelectionSupplier->removeSelectionChangeListener (
            static_cast<view::XSelectionChangeListener*>(this));

    if (maShapeTreeInfo.GetModelBroadcaster().is())
        maShapeTreeInfo.GetModelBroadcaster()->removeEventListener (
            static_cast<document::XEventListener*>(this));

    ClearAccessibleShapeList ();
    OSL_TRACE ("~ChildrenManagerImpl");
}




void ChildrenManagerImpl::Init (void)
{
    // Register as view::XSelectionChangeListener.
    Reference<view::XSelectionSupplier> xSelectionSupplier (
        maShapeTreeInfo.GetController(), uno::UNO_QUERY);
    if (xSelectionSupplier.is())
        xSelectionSupplier->addSelectionChangeListener (
            static_cast<view::XSelectionChangeListener*>(this));

    // Register at model as document::XEventListener.
    if (maShapeTreeInfo.GetModelBroadcaster().is())
        maShapeTreeInfo.GetModelBroadcaster()->addEventListener (
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
            ::rtl::OUString::createFromAscii(
                "no accessible child with index ") + nIndex,
            mxParent);

    return GetChild (maVisibleChildren[nIndex]);
}




/** Return the requested accessible child object.  Create it if it is not
    yet in the cache.
*/
uno::Reference<XAccessible>
    ChildrenManagerImpl::GetChild (ChildDescriptor& rChildDescriptor)
    throw (::com::sun::star::uno::RuntimeException)
{
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
                    AccessibleShapeInfo (
                        rChildDescriptor.mxShape,
                        mxParent,
                        this),
                    maShapeTreeInfo);
            //            rChildDescriptor.mpAccessibleShape = pShape;
            rChildDescriptor.mxAccessibleShape = uno::Reference<XAccessible> (
                static_cast<uno::XWeak*>(pShape),
                uno::UNO_QUERY);
        }
    }

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
    if (maShapeTreeInfo.GetViewForwarder() == NULL)
        return;
    Rectangle aVisibleArea = maShapeTreeInfo.GetViewForwarder()->GetVisibleArea();
    {
    OSL_TRACE ("ChildrenManagerImpl::update called with VisibleArea = %d %d %d %d",
        aVisibleArea.getX(), aVisibleArea.getY(),
        aVisibleArea.getWidth(), aVisibleArea.getHeight());
        OSL_TRACE ("   shape list contains %d children",
            mxShapeList.is()?mxShapeList->getCount():0);
    }

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

    // 5. If children have to be created immediately and not on demand then
    // create the missing accessible objects now.
    if ( ! bCreateNewObjectsOnDemand)
        CreateAccessibilityObjects (aNewChildList);

    // 6. Replace the current list of visible shapes with the new one.  Do
    // the same with the visible area.
    ::vos::OGuard aGuard (maMutex);
    maVisibleChildren = aNewChildList;
    maVisibleArea = aVisibleArea;
}




void ChildrenManagerImpl::CreateListOfVisibleShapes (
    ChildDescriptorListType& raDescriptorList)
{
    ::vos::OGuard aGuard (maMutex);

    OSL_ASSERT (maShapeTreeInfo.GetViewForwarder() != NULL);

    Rectangle aVisibleArea = maShapeTreeInfo.GetViewForwarder()->GetVisibleArea();

    // Add the visible shapes for wich the accessible objects already exist.
    AccessibleShapeList::iterator I;
    for (I=maAccessibleShapes.begin(); I!=maAccessibleShapes.end(); ++I)
    {
        if (I->is())
        {
            uno::Reference<XAccessibleComponent> xComponent (
                (*I)->getAccessibleContext(), uno::UNO_QUERY);
            if (xComponent.is())
            {
                // The bounding box of the object already is clipped to the
                // visible area.  The object is therefore visible if the
                // bounding box has non-zero extensions.
                awt::Rectangle aPixelBBox (xComponent->getBounds());
                if ((aPixelBBox.Width > 0) && (aPixelBBox.Height > 0))
                    raDescriptorList.push_back (ChildDescriptor (*I));
                else
                    OSL_TRACE ("accessible shape %x not visible", *I);
            }
        }
    }

    // Add the visible shapes for which only the XShapes exist.
    uno::Reference<container::XIndexAccess> xShapeAccess (mxShapeList, uno::UNO_QUERY);
    if (xShapeAccess.is())
    {
        sal_Int32 nShapeCount = xShapeAccess->getCount();
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
                OSL_TRACE ("shape %x not visible", xShape);
        }
    }
}




void ChildrenManagerImpl::RemoveNonVisibleChildren (
    ChildDescriptorListType& raNewChildList)
{
    // Iterate over list of formerly visible children and remove those that
    // are not visible anymore, i.e. member of the new list of visible
    // children.
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

                // Dispose and remove the object.
                if (I->mxShape.is())
                {
                    Reference<lang::XComponent> xComponent (I->mxAccessibleShape, uno::UNO_QUERY);
                    xComponent->dispose ();
                    I->mxAccessibleShape = NULL;
                }
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
                //                I->mpAccessibleShape = aOldChildDescriptor->mpAccessibleShape;
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
        AccessibleShape* pShape = I->GetAccessibleShape ();
        if (pShape != NULL)
            pShape->ViewForwarderChanged (
                IAccessibleViewForwarderListener::VISIBLE_AREA,
                maShapeTreeInfo.GetViewForwarder());
    }
}




void ChildrenManagerImpl::CreateAccessibilityObjects (
    ChildDescriptorListType& raNewChildList)
{
    OSL_TRACE ("sending events for new shape");
    ChildDescriptorListType::iterator I;
    for (I=raNewChildList.begin(); I!=raNewChildList.end(); I++)
    {
        // Create the associated accessible object when the flag says so and
        // it does not yet exist.
        if ( ! I->mxAccessibleShape.is())
            GetChild (*I);
        if (I->mxAccessibleShape.is() && I->mbCreateEventPending)
        {
            uno::Any aNewShape;
            aNewShape <<= I->mxAccessibleShape;
            mrContext.CommitChange (
                AccessibleEventId::ACCESSIBLE_CHILD_EVENT,
                aNewShape,
                uno::Any());
        }
    }
    OSL_TRACE ("done sending events for new shape");
}




void ChildrenManagerImpl::SetShapeList (const ::com::sun::star::uno::Reference<
    ::com::sun::star::drawing::XShapes>& xShapeList)
{
    mxShapeList = xShapeList;
}




void ChildrenManagerImpl::AddAccessibleShape (std::auto_ptr<AccessibleShape> pShape)
{
    if (pShape.get() != NULL)
        maAccessibleShapes.push_back (pShape.release());
}




void ChildrenManagerImpl::ClearAccessibleShapeList (void)
{
    // Clear the list of visible accessible objects.  Objects not created on
    // demand for XShapes are disposed later.
    ChildDescriptorListType::iterator I;
    for (I=maVisibleChildren.begin(); I!=maVisibleChildren.end(); I++)
        if (I->mxAccessibleShape.is() && ! I->mxShape.is())
        {
            uno::Any aShape;
            aShape <<= I->mxAccessibleShape;
            mrContext.CommitChange (
                AccessibleEventId::ACCESSIBLE_CHILD_EVENT,
                uno::Any(),
                aShape);

            // Dispose the object if there is a cooresponding UNO shape.
            // Otherwise it will be disposed below.
            if (I->mxShape.is())
            {
                Reference<lang::XComponent> xComponent (
                    I->mxAccessibleShape, uno::UNO_QUERY);
                if (xComponent.is())
                    xComponent->dispose ();
            }
            // Reset the reference to the accessible object in any case.  If
            // it has not been disposed above it will be soon.
            I->mxAccessibleShape = NULL;
        }
    maVisibleChildren.clear ();


    // Dispose all objects in the accessible shape list.
    AccessibleShapeList::iterator J;
    for (J=maAccessibleShapes.begin(); J!=maAccessibleShapes.end(); J++)
        if (J->is())
        {
            mrContext.CommitChange (
                AccessibleEventId::ACCESSIBLE_CHILD_EVENT,
                uno::Any(),
                uno::makeAny (*J));

            // Dispose the object.
            Reference<lang::XComponent> xComponent (*J, uno::UNO_QUERY);
            if (xComponent.is())
                xComponent->dispose ();
        }
    maAccessibleShapes.clear ();
}




void ChildrenManagerImpl::SetInfo (const AccessibleShapeTreeInfo& rShapeTreeInfo)
{
    uno::Reference<document::XEventBroadcaster> xCurrentBroadcaster;
    {
        ::vos::OGuard aGuard (maMutex);
        xCurrentBroadcaster = maShapeTreeInfo.GetModelBroadcaster();
        maShapeTreeInfo = rShapeTreeInfo;
    }

    if (maShapeTreeInfo.GetModelBroadcaster() != xCurrentBroadcaster)
    {
        // Register at new broadcaster.
        if (maShapeTreeInfo.GetModelBroadcaster().is())
            maShapeTreeInfo.GetModelBroadcaster()->addEventListener (
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
    if (rEventObject.Source == maShapeTreeInfo.GetModelBroadcaster())
    {
        maShapeTreeInfo.SetModelBroadcaster (NULL);
        // The disposing of a model should be handled elsewhere.  But to be
        // on the safe side we remove all of our children.
        ClearAccessibleShapeList ();
        SetShapeList (NULL);
    }

    else if (rEventObject.Source
        == Reference<view::XSelectionSupplier> (
            maShapeTreeInfo.GetController(), uno::UNO_QUERY))
    {
        maShapeTreeInfo.SetController (NULL);
    }
}




//=====  document::XEventListener  ============================================

/** Listen for new and removed shapes.
*/
void SAL_CALL
    ChildrenManagerImpl::notifyEvent (
        const document::EventObject& rEventObject)
    throw (uno::RuntimeException)
{
    static const OUString sShapeInserted (
        RTL_CONSTASCII_USTRINGPARAM("ShapeInserted"));
    static const OUString sShapeRemoved (
        RTL_CONSTASCII_USTRINGPARAM("ShapeRemoved"));

#ifdef DBG_UTIL
    OSL_TRACE ("ChildrenManagerImpl::notifyEvent %s",
        ::rtl::OUStringToOString(rEventObject.EventName,RTL_TEXTENCODING_ASCII_US).getStr());
#endif

    if (rEventObject.EventName.equals (sShapeInserted)
        || rEventObject.EventName.equals (sShapeRemoved))
    {
        // A child may have been added or removed.  Call Update() to
        // make the change visible.
        Update (false);
        UpdateSelection ();
    }
}




//=====  view::XSelectionChangeListener  ======================================

void  SAL_CALL
    ChildrenManagerImpl::selectionChanged (const lang::EventObject& rEvent)
        throw (uno::RuntimeException)
{
    OSL_TRACE ("selection changed");
    UpdateSelection ();
}




// This method is experimental.  Use with care.
long int ChildrenManagerImpl::GetChildIndex (const ::com::sun::star::uno::Reference<
    ::drafts::com::sun::star::accessibility::XAccessible>& xChild) const
    throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard (maMutex);
    for (unsigned long i=0; i<maVisibleChildren.size(); i++)
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
        for (unsigned long i=0; i<maVisibleChildren.size(); i++)
        {
            AccessibleShape* pShape = maVisibleChildren[i].GetAccessibleShape();
            if (pShape != NULL)
                pShape->ViewForwarderChanged (aChangeType, pViewForwarder);
        }
    }
}




//=====  IAccessibleParent  ===================================================

sal_Bool ChildrenManagerImpl::ReplaceChild (
    AccessibleShape* pCurrentChild,
    AccessibleShape* pReplacement)
    throw (uno::RuntimeException)
{
    sal_Bool bResult = sal_False;

    // Iterate over the visible children.  If one of them has an already
    // created accessible object that matches pCurrentChild then replace
    // it.  Otherwise the child to replace is either not in the list or has
    // not ye been created (and is therefore not in the list, too) and a
    // replacement is not necessary.
    ChildDescriptorListType::iterator I;
    for (I=maVisibleChildren.begin(); I!=maVisibleChildren.end(); I++)
    {
        if (I->GetAccessibleShape() == pCurrentChild)
        {
            // Dispose the current child and send an event about its deletion.
            pCurrentChild->dispose();
            mrContext.CommitChange (
                AccessibleEventId::ACCESSIBLE_CHILD_EVENT,
                uno::Any(),
                uno::makeAny (I->mxAccessibleShape));

            // Replace with replacement and send an event about existance
            // of the new child.
            I->mxAccessibleShape = pReplacement;
            mrContext.CommitChange (
                AccessibleEventId::ACCESSIBLE_CHILD_EVENT,
                uno::makeAny (I->mxAccessibleShape),
                uno::Any());
            bResult = sal_True;
            break;
        }
    }

    // When not found among the visible children we have to search the list
    // of accessible shapes.  This is not yet implemented.

    return bResult;
}




/** Update the <const>SELECTED</const> and the <const>FOCUSED</const> state
    of all visible children.  Maybe this should be changed to all children.

    Iterate over all descriptors of visible accessible shapes and look them
    up in the selection.
*/
void ChildrenManagerImpl::UpdateSelection (void)
{
    OSL_TRACE ("ChildrenManagerImpl::UpdateSelection");
    Reference<view::XSelectionSupplier> xSelectionSupplier (maShapeTreeInfo.GetController(), uno::UNO_QUERY);
    if ( ! xSelectionSupplier.is())
        return;

    // Try to cast the selection both to a multi selection and to a single
    // selection.
    Reference<container::XIndexAccess> xSelectedShapeAccess (
        xSelectionSupplier->getSelection(), uno::UNO_QUERY);
    Reference<drawing::XShape> xSelectedShape (
        xSelectionSupplier->getSelection(), uno::UNO_QUERY);

    // Remember the current and new focused shape.
    AccessibleShape* pCurrentlyFocusedShape = NULL;
    AccessibleShape* pNewFocusedShape = NULL;

    ChildDescriptorListType::iterator I;
    OSL_TRACE ("  there are %d visible children", maVisibleChildren.size());
    for (I=maVisibleChildren.begin(); I!=maVisibleChildren.end(); I++)
    {
        OSL_TRACE ("  looking for visible shape");
        AccessibleShape* pAccessibleShape = I->GetAccessibleShape();
        if (I->mxAccessibleShape.is() && I->mxShape.is() && pAccessibleShape!=NULL)
        {
            bool bShapeIsSelected = false;
            OSL_TRACE ("  shape exists");

            // Look up the shape in the selection.
            if (xSelectedShape.is())
            {
                OSL_TRACE ("  comparing to single selected shape.");
                if  (I->mxShape == xSelectedShape)
                {
                    bShapeIsSelected = true;
                    pNewFocusedShape = pAccessibleShape;
                }
            }
            else if (xSelectedShapeAccess.is())
            {
                OSL_TRACE ("  comparing to multiple selected shapes.");
                for (sal_Int32 i=0,nCount=xSelectedShapeAccess->getCount(); i<nCount&&!bShapeIsSelected; i++)
                    if (xSelectedShapeAccess->getByIndex(i) == I->mxShape)
                    {
                        bShapeIsSelected = true;
                        // In a multi-selection no shape has the focus.
                        if (nCount == 1)
                            pNewFocusedShape = pAccessibleShape;
                    }
            }

            OSL_TRACE ("  new selection state is %d", bShapeIsSelected?1:0);
            // Set or reset the SELECTED state.
            if (bShapeIsSelected)
                pAccessibleShape->SetState (AccessibleStateType::SELECTED);
            else
                pAccessibleShape->ResetState (AccessibleStateType::SELECTED);

            // Does the shape have the current selection?
            if (pAccessibleShape->GetState (AccessibleStateType::FOCUSED))
                pCurrentlyFocusedShape = pAccessibleShape;
        }
    }

    // Now reset and then set the FOCUSED state.
    if (pCurrentlyFocusedShape != pNewFocusedShape)
    {
        if (pCurrentlyFocusedShape != NULL)
            pCurrentlyFocusedShape->ResetState (AccessibleStateType::FOCUSED);
        if (pNewFocusedShape != NULL)
            pNewFocusedShape->SetState (AccessibleStateType::FOCUSED);
    }
}



//=====  AccessibleChildDescriptor  ===========================================

ChildDescriptor::ChildDescriptor (const Reference<drawing::XShape>& xShape)
    : mxShape (xShape),
      mxAccessibleShape (NULL),
      mbCreateEventPending (true)
{
    // Empty.
}




ChildDescriptor::ChildDescriptor (const Reference<XAccessible>& rxAccessibleShape)
    : mxShape (NULL),
      mxAccessibleShape (rxAccessibleShape),
      mbCreateEventPending (true)
{
    // Empty.
}




AccessibleShape* ChildDescriptor::GetAccessibleShape (void) const
{
    return static_cast<AccessibleShape*> (mxAccessibleShape.get());
}




/** Compare two child descriptors.  Take into account that a child
    descriptor may be based on a UNO shape or, already, on an accessible
    shape.
 */
bool ChildDescriptor::operator == (const ChildDescriptor& aDescriptor)
{
    if (mxShape == aDescriptor.mxShape)
        if (mxShape.is())
            return true;
        else if (mxAccessibleShape == aDescriptor.mxAccessibleShape)
            return true;

    return false;
}




bool ChildDescriptor::operator < (const ChildDescriptor& aDescriptor)
{
    return mxShape < aDescriptor.mxShape;
}


} // end of namespace accessibility
