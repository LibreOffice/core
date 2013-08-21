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


#include "ChildrenManagerImpl.hxx"
#include <svx/ShapeTypeHandler.hxx>
#include <svx/AccessibleShapeInfo.hxx>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <comphelper/uno3.hxx>

#include <rtl/ustring.hxx>
#include <tools/debug.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using ::com::sun::star::uno::Reference;


namespace accessibility {

namespace
{
void adjustIndexInParentOfShapes(ChildDescriptorListType& _rList)
{
    ChildDescriptorListType::iterator aEnd = _rList.end();
    sal_Int32 i=0;
    for ( ChildDescriptorListType::iterator aIter = _rList.begin(); aIter != aEnd; ++aIter,++i)
        aIter->setIndexAtAccessibleShape(i);
}
}

//=====  AccessibleChildrenManager  ===========================================

ChildrenManagerImpl::ChildrenManagerImpl (
    const uno::Reference<XAccessible>& rxParent,
    const uno::Reference<drawing::XShapes>& rxShapeList,
    const AccessibleShapeTreeInfo& rShapeTreeInfo,
    AccessibleContextBase& rContext)
    : ::cppu::WeakComponentImplHelper2<
          ::com::sun::star::document::XEventListener,
          ::com::sun::star::view::XSelectionChangeListener>(maMutex),
      mxShapeList (rxShapeList),
      mxParent (rxParent),
      maShapeTreeInfo (rShapeTreeInfo),
      mrContext (rContext),
      mnNewNameIndex(1),
      mpFocusedShape(NULL)
{
}




ChildrenManagerImpl::~ChildrenManagerImpl (void)
{
    DBG_ASSERT (rBHelper.bDisposed || rBHelper.bInDispose,
        "~AccessibleDrawDocumentView: object has not been disposed");
}




void ChildrenManagerImpl::Init (void)
{
    // Register as view::XSelectionChangeListener.
    Reference<frame::XController> xController(maShapeTreeInfo.GetController());
    Reference<view::XSelectionSupplier> xSelectionSupplier (
        xController, uno::UNO_QUERY);
    if (xSelectionSupplier.is())
    {
        xController->addEventListener(
            static_cast<document::XEventListener*>(this));

        xSelectionSupplier->addSelectionChangeListener (
            static_cast<view::XSelectionChangeListener*>(this));
    }

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
    throw (::com::sun::star::uno::RuntimeException,
           ::com::sun::star::lang::IndexOutOfBoundsException)
{
    // Check whether the given index is valid.
    if (nIndex < 0 || (unsigned long)nIndex >= maVisibleChildren.size())
        throw lang::IndexOutOfBoundsException (
            "no accessible child with index " + OUString::number(nIndex),
            mxParent);

    return GetChild (maVisibleChildren[nIndex],nIndex);
}




/** Return the requested accessible child object.  Create it if it is not
    yet in the cache.
*/
uno::Reference<XAccessible>
    ChildrenManagerImpl::GetChild (ChildDescriptor& rChildDescriptor,sal_Int32 _nIndex)
    throw (::com::sun::star::uno::RuntimeException)
{
    if ( ! rChildDescriptor.mxAccessibleShape.is())
    {
        ::osl::MutexGuard aGuard (maMutex);
        // Make sure that the requested accessible object has not been
        // created while locking the global mutex.
        if ( ! rChildDescriptor.mxAccessibleShape.is())
        {
            AccessibleShapeInfo aShapeInfo(
                        rChildDescriptor.mxShape,
                        mxParent,
                        this,
                        mnNewNameIndex++);
            // Create accessible object that corresponds to the descriptor's
            // shape.
            AccessibleShape* pShape =
                ShapeTypeHandler::Instance().CreateAccessibleObject (
                    aShapeInfo,
                    maShapeTreeInfo);
            rChildDescriptor.mxAccessibleShape = uno::Reference<XAccessible> (
                static_cast<uno::XWeak*>(pShape),
                uno::UNO_QUERY);
            // Now that there is a reference to the new accessible shape we
            // can safely call its Init() method.
            if ( pShape != NULL )
            {
                pShape->Init();
                pShape->setIndexInParent(_nIndex);
            }
        }
    }

    return rChildDescriptor.mxAccessibleShape;
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

    // 1. Create a local list of visible shapes.
    ChildDescriptorListType aChildList;
    CreateListOfVisibleShapes (aChildList);

    // 2. Merge the information that is already known about the visible
    // shapes from the current list into the new list.
    MergeAccessibilityInformation (aChildList);

    // 3. Replace the current list of visible shapes with the new one.  Do
    // the same with the visible area.
    {
        ::osl::MutexGuard aGuard (maMutex);
        adjustIndexInParentOfShapes(aChildList);

        // Use swap to copy the contents of the new list in constant time.
        maVisibleChildren.swap (aChildList);

        // aChildList now contains all the old children, while maVisibleChildren
        // contains all the current children

        // 4. Find all shapes in the old list that are not in the current list,
        // send appropriate events and remove the accessible shape.
        //
        // Do this *after* we have set our new list of children, because
        // removing a child may cause
        //
        // ChildDescriptor::disposeAccessibleObject -->
        // AccessibleContextBase::CommitChange -->
        // AtkListener::notifyEvent ->
        // AtkListener::handleChildRemoved ->
        // AtkListener::updateChildList
        // AccessibleDrawDocumentView::getAccessibleChildCount ->
        // ChildrenManagerImpl::GetChildCount ->
        // maVisibleChildren.size()
        //
        // to be fired, and so the operations will take place on
        // the list we are trying to replace
        //
        RemoveNonVisibleChildren (maVisibleChildren, aChildList);

        aChildList.clear();

        maVisibleArea = aVisibleArea;
    }

    // 5. If the visible area has changed then send events that signal a
    // change of their bounding boxes for all shapes that are members of
    // both the current and the new list of visible shapes.
    if (maVisibleArea != aVisibleArea)
        SendVisibleAreaEvents (maVisibleChildren);

    // 6. If children have to be created immediately and not on demand then
    // create the missing accessible objects now.
    if ( ! bCreateNewObjectsOnDemand)
        CreateAccessibilityObjects (maVisibleChildren);
}




void ChildrenManagerImpl::CreateListOfVisibleShapes (
    ChildDescriptorListType& raDescriptorList)
{
    ::osl::MutexGuard aGuard (maMutex);

    OSL_ASSERT (maShapeTreeInfo.GetViewForwarder() != NULL);

    Rectangle aVisibleArea = maShapeTreeInfo.GetViewForwarder()->GetVisibleArea();

    // Add the visible shapes for which the accessible objects already exist.
    AccessibleShapeList::iterator I,aEnd = maAccessibleShapes.end();
    for (I=maAccessibleShapes.begin(); I != aEnd; ++I)
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
            }
        }
    }

    // Add the visible shapes for which only the XShapes exist.
    uno::Reference<container::XIndexAccess> xShapeAccess (mxShapeList, uno::UNO_QUERY);
    if (xShapeAccess.is())
    {
        sal_Int32 nShapeCount = xShapeAccess->getCount();
        raDescriptorList.reserve( nShapeCount );
        awt::Point aPos;
        awt::Size aSize;
        Rectangle aBoundingBox;
        uno::Reference<drawing::XShape> xShape;
        for (sal_Int32 i=0; i<nShapeCount; ++i)
        {
            xShapeAccess->getByIndex(i) >>= xShape;
            aPos = xShape->getPosition();
            aSize = xShape->getSize();

            aBoundingBox.Left() = aPos.X;
            aBoundingBox.Top() = aPos.Y;
            aBoundingBox.Right() = aPos.X + aSize.Width;
            aBoundingBox.Bottom() = aPos.Y + aSize.Height;

            // Insert shape if it is visible, i.e. its bounding box overlaps
            // the visible area.
            if ( aBoundingBox.IsOver (aVisibleArea) )
                raDescriptorList.push_back (ChildDescriptor (xShape));
        }
    }
}




void ChildrenManagerImpl::RemoveNonVisibleChildren (
    const ChildDescriptorListType& rNewChildList,
    ChildDescriptorListType& rOldChildList)
{
    // Iterate over list of formerly visible children and remove those that
    // are not visible anymore, i.e. member of the new list of visible
    // children.
    ChildDescriptorListType::iterator I, aEnd = rOldChildList.end();
    for (I=rOldChildList.begin(); I != aEnd; ++I)
    {
        if (::std::find(rNewChildList.begin(), rNewChildList.end(), *I) == rNewChildList.end())
        {
            // The child is disposed when there is a UNO shape from which
            // the accessible shape can be created when the shape becomes
            // visible again.  When there is no such UNO shape then simply
            // reset the descriptor but keep the accessibility object.
            if (I->mxShape.is())
            {
                UnregisterAsDisposeListener (I->mxShape);
                I->disposeAccessibleObject (mrContext);
            }
            else
            {
                AccessibleShape* pAccessibleShape = I->GetAccessibleShape();
                pAccessibleShape->ResetState (AccessibleStateType::VISIBLE);
                I->mxAccessibleShape = NULL;
            }
        }
    }
}




void ChildrenManagerImpl::MergeAccessibilityInformation (
    ChildDescriptorListType& raNewChildList)
{
    ChildDescriptorListType::iterator aOldChildDescriptor;
    ChildDescriptorListType::iterator I, aEnd = raNewChildList.end();
    for (I=raNewChildList.begin(); I != aEnd; ++I)
    {
        aOldChildDescriptor = ::std::find (maVisibleChildren.begin(), maVisibleChildren.end(), *I);

        // Copy accessible shape if that exists in the old descriptor.
        bool bRegistrationIsNecessary = true;
        if (aOldChildDescriptor != maVisibleChildren.end())
            if (aOldChildDescriptor->mxAccessibleShape.is())
            {
                I->mxAccessibleShape = aOldChildDescriptor->mxAccessibleShape;
                I->mbCreateEventPending = false;
                bRegistrationIsNecessary = false;
            }
        if (bRegistrationIsNecessary)
            RegisterAsDisposeListener (I->mxShape);
    }
}




void ChildrenManagerImpl::SendVisibleAreaEvents (
    ChildDescriptorListType& raNewChildList)
{
    ChildDescriptorListType::iterator I,aEnd = raNewChildList.end();
    for (I=raNewChildList.begin(); I != aEnd; ++I)
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
    ChildDescriptorListType::iterator I, aEnd = raNewChildList.end();
    sal_Int32 nPos = 0;
    for ( I = raNewChildList.begin(); I != aEnd; ++I,++nPos)
    {
        // Create the associated accessible object when the flag says so and
        // it does not yet exist.
        if ( ! I->mxAccessibleShape.is() )
            GetChild (*I,nPos);
        if (I->mxAccessibleShape.is() && I->mbCreateEventPending)
        {
            I->mbCreateEventPending = false;
            mrContext.CommitChange (
                AccessibleEventId::CHILD,
                uno::makeAny(I->mxAccessibleShape),
                uno::Any());
        }
    }
}




void ChildrenManagerImpl::AddShape (const Reference<drawing::XShape>& rxShape)
{
    if (rxShape.is())
    {
        ::osl::ClearableMutexGuard aGuard (maMutex);

        // Test visibility of the shape.
        Rectangle aVisibleArea = maShapeTreeInfo.GetViewForwarder()->GetVisibleArea();
        awt::Point aPos = rxShape->getPosition();
        awt::Size aSize = rxShape->getSize();

        Rectangle aBoundingBox (
            aPos.X,
            aPos.Y,
            aPos.X + aSize.Width,
            aPos.Y + aSize.Height);

        // Add the shape only when it belongs to the list of shapes stored
        // in mxShapeList (which is either a page or a group shape).
        Reference<container::XChild> xChild (rxShape, uno::UNO_QUERY);
        if (xChild.is())
        {
            Reference<drawing::XShapes> xParent (xChild->getParent(), uno::UNO_QUERY);
            if (xParent == mxShapeList)
                if (aBoundingBox.IsOver (aVisibleArea))
                {
                    // Add shape to list of visible shapes.
                    maVisibleChildren.push_back (ChildDescriptor (rxShape));

                    // Create accessibility object.
                    ChildDescriptor& rDescriptor = maVisibleChildren.back();
                    GetChild (rDescriptor, maVisibleChildren.size()-1);

                    // Inform listeners about new child.
                    uno::Any aNewShape;
                    aNewShape <<= rDescriptor.mxAccessibleShape;
                    aGuard.clear();
                    mrContext.CommitChange (
                        AccessibleEventId::CHILD,
                        aNewShape,
                        uno::Any());
                    RegisterAsDisposeListener (rDescriptor.mxShape);
                }
        }
    }
}




void ChildrenManagerImpl::RemoveShape (const Reference<drawing::XShape>& rxShape)
{
    if (rxShape.is())
    {
        ::osl::ClearableMutexGuard aGuard (maMutex);

        // Search shape in list of visible children.
        ChildDescriptorListType::iterator I (
            ::std::find (maVisibleChildren.begin(), maVisibleChildren.end(),
                ChildDescriptor (rxShape)));
        if (I != maVisibleChildren.end())
        {
            // Remove descriptor from that list.
            Reference<XAccessible> xAccessibleShape (I->mxAccessibleShape);

            UnregisterAsDisposeListener (I->mxShape);
            // Dispose the accessible object.
            I->disposeAccessibleObject (mrContext);

            // Now we can safely remove the child descriptor and thus
            // invalidate the iterator.
            maVisibleChildren.erase (I);

            adjustIndexInParentOfShapes(maVisibleChildren);
        }
    }
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
    // Copy the list of (visible) shapes to local lists and clear the
    // originals.
    ChildDescriptorListType aLocalVisibleChildren;
    aLocalVisibleChildren.swap(maVisibleChildren);
    AccessibleShapeList aLocalAccessibleShapes;
    aLocalAccessibleShapes.swap(maAccessibleShapes);

    // Tell the listeners that all children are gone.
    mrContext.CommitChange (
        AccessibleEventId::INVALIDATE_ALL_CHILDREN,
        uno::Any(),
        uno::Any());

    // There are no accessible shapes left so the index assigned to new
    // accessible shapes can be reset.
    mnNewNameIndex = 1;

    // Now the objects in the local lists can be safely disposed without
    // having problems with callers that want to update their child lists.

    // Clear the list of visible accessible objects.  Objects not created on
    // demand for XShapes are treated below.
    ChildDescriptorListType::iterator I,aEnd = aLocalVisibleChildren.end();
    for (I=aLocalVisibleChildren.begin(); I != aEnd; ++I)
        if ( I->mxAccessibleShape.is() && I->mxShape.is() )
        {
            ::comphelper::disposeComponent(I->mxAccessibleShape);
            I->mxAccessibleShape = NULL;
        }

    // Dispose all objects in the accessible shape list.
    AccessibleShapeList::iterator J,aEnd2 = aLocalAccessibleShapes.end();
    for (J=aLocalAccessibleShapes.begin(); J != aEnd2; ++J)
        if (J->is())
        {
            // Dispose the object.
            ::comphelper::disposeComponent(*J);
            *J = NULL;
        }
}




/** If the broadcasters change at which this object is registered then
    unregister at old and register at new broadcasters.
*/
void ChildrenManagerImpl::SetInfo (const AccessibleShapeTreeInfo& rShapeTreeInfo)
{
    // Remember the current broadcasters and exchange the shape tree info.
    Reference<document::XEventBroadcaster> xCurrentBroadcaster;
    Reference<frame::XController> xCurrentController;
    Reference<view::XSelectionSupplier> xCurrentSelectionSupplier;
    {
        ::osl::MutexGuard aGuard (maMutex);
        xCurrentBroadcaster = maShapeTreeInfo.GetModelBroadcaster();
        xCurrentController = maShapeTreeInfo.GetController();
        xCurrentSelectionSupplier = Reference<view::XSelectionSupplier> (
            xCurrentController, uno::UNO_QUERY);
        maShapeTreeInfo = rShapeTreeInfo;
    }

    // Move registration to new model.
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

    // Move registration to new selection supplier.
    Reference<frame::XController> xNewController(maShapeTreeInfo.GetController());
    Reference<view::XSelectionSupplier> xNewSelectionSupplier (
        xNewController, uno::UNO_QUERY);
    if (xNewSelectionSupplier != xCurrentSelectionSupplier)
    {
        // Register at new broadcaster.
        if (xNewSelectionSupplier.is())
        {
            xNewController->addEventListener(
                static_cast<document::XEventListener*>(this));

            xNewSelectionSupplier->addSelectionChangeListener (
                static_cast<view::XSelectionChangeListener*>(this));
        }

        // Unregister at old broadcaster.
        if (xCurrentSelectionSupplier.is())
        {
            xCurrentSelectionSupplier->removeSelectionChangeListener (
                static_cast<view::XSelectionChangeListener*>(this));

            xCurrentController->removeEventListener(
                static_cast<document::XEventListener*>(this));
        }
    }
}




//=====  lang::XEventListener  ================================================

void SAL_CALL
    ChildrenManagerImpl::disposing (const lang::EventObject& rEventObject)
    throw (uno::RuntimeException)
{
    if (rEventObject.Source == maShapeTreeInfo.GetModelBroadcaster()
            || rEventObject.Source == maShapeTreeInfo.GetController())
    {
        impl_dispose();
    }

    // Handle disposing UNO shapes.
    else
    {
        Reference<drawing::XShape> xShape (rEventObject.Source, uno::UNO_QUERY);

        // Find the descriptor for the given shape.
        ChildDescriptorListType::iterator I (
            ::std::find (maVisibleChildren.begin(), maVisibleChildren.end(),
                ChildDescriptor (xShape)));
        if (I != maVisibleChildren.end())
        {
            // Clear the descriptor.
            I->disposeAccessibleObject (mrContext);
            I->mxShape = NULL;
        }
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
    static const OUString sShapeInserted ("ShapeInserted");
    static const OUString sShapeRemoved ("ShapeRemoved");


    if (rEventObject.EventName == sShapeInserted)
        AddShape (Reference<drawing::XShape>(rEventObject.Source, uno::UNO_QUERY));
    else if (rEventObject.EventName == sShapeRemoved)
        RemoveShape (Reference<drawing::XShape>(rEventObject.Source, uno::UNO_QUERY));
    // else ignore unknown event.
}




//=====  view::XSelectionChangeListener  ======================================

void  SAL_CALL
    ChildrenManagerImpl::selectionChanged (const lang::EventObject& /*rEvent*/)
        throw (uno::RuntimeException)
{
    UpdateSelection ();
}




void ChildrenManagerImpl::impl_dispose (void)
{
    Reference<frame::XController> xController(maShapeTreeInfo.GetController());
    // Remove from broadcasters.
    try
    {
        Reference<view::XSelectionSupplier> xSelectionSupplier (
            xController, uno::UNO_QUERY);
        if (xSelectionSupplier.is())
        {
            xSelectionSupplier->removeSelectionChangeListener (
                static_cast<view::XSelectionChangeListener*>(this));
        }
    }
    catch( uno::RuntimeException&)
    {}

    try
    {
        if (xController.is())
            xController->removeEventListener(
                static_cast<document::XEventListener*>(this));
    }
    catch( uno::RuntimeException&)
    {}

    maShapeTreeInfo.SetController (NULL);

    try
    {
        // Remove from broadcaster.
        if (maShapeTreeInfo.GetModelBroadcaster().is())
            maShapeTreeInfo.GetModelBroadcaster()->removeEventListener (
                static_cast<document::XEventListener*>(this));
        maShapeTreeInfo.SetModelBroadcaster (NULL);
    }
    catch( uno::RuntimeException& )
    {}

    ClearAccessibleShapeList ();
    SetShapeList (NULL);
}



void SAL_CALL ChildrenManagerImpl::disposing (void)
{
    impl_dispose();
}

//=====  IAccessibleViewForwarderListener  ====================================

void ChildrenManagerImpl::ViewForwarderChanged (ChangeType aChangeType,
        const IAccessibleViewForwarder* pViewForwarder)
{
    if (aChangeType == IAccessibleViewForwarderListener::VISIBLE_AREA)
        Update (false);
    else
    {
        ::osl::MutexGuard aGuard (maMutex);
        ChildDescriptorListType::iterator I, aEnd = maVisibleChildren.end();
        for (I=maVisibleChildren.begin(); I != aEnd; ++I)
        {
            AccessibleShape* pShape = I->GetAccessibleShape();
            if (pShape != NULL)
                pShape->ViewForwarderChanged (aChangeType, pViewForwarder);
        }
    }
}




//=====  IAccessibleParent  ===================================================

sal_Bool ChildrenManagerImpl::ReplaceChild (
    AccessibleShape* pCurrentChild,
    const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& _rxShape,
    const long _nIndex,
    const AccessibleShapeTreeInfo& _rShapeTreeInfo)
    throw (uno::RuntimeException)
{
    AccessibleShapeInfo aShapeInfo( _rxShape, pCurrentChild->getAccessibleParent(), this, _nIndex );
    // create the new child
    AccessibleShape* pNewChild = ShapeTypeHandler::Instance().CreateAccessibleObject (
        aShapeInfo,
        _rShapeTreeInfo
    );
    Reference< XAccessible > xNewChild( pNewChild );    // keep this alive (do this before calling Init!)
    if ( pNewChild )
        pNewChild->Init();

    sal_Bool bResult = sal_False;

    // Iterate over the visible children.  If one of them has an already
    // created accessible object that matches pCurrentChild then replace
    // it.  Otherwise the child to replace is either not in the list or has
    // not ye been created (and is therefore not in the list, too) and a
    // replacement is not necessary.
    ChildDescriptorListType::iterator I,aEnd = maVisibleChildren.end();
    for (I=maVisibleChildren.begin(); I != aEnd; ++I)
    {
        if (I->GetAccessibleShape() == pCurrentChild)
        {
            // Dispose the current child and send an event about its deletion.
            pCurrentChild->dispose();
            mrContext.CommitChange (
                AccessibleEventId::CHILD,
                uno::Any(),
                uno::makeAny (I->mxAccessibleShape));

            // Replace with replacement and send an event about existance
            // of the new child.
            I->mxAccessibleShape = pNewChild;
            mrContext.CommitChange (
                AccessibleEventId::CHILD,
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

    If there is no valid controller then all shapes are deselected and
    unfocused.  If the controller's frame is not active then all shapes are
    unfocused.
*/
void ChildrenManagerImpl::UpdateSelection (void)
{
    Reference<frame::XController> xController(maShapeTreeInfo.GetController());
    Reference<view::XSelectionSupplier> xSelectionSupplier (
        xController, uno::UNO_QUERY);

    // Try to cast the selection both to a multi selection and to a single
    // selection.
    Reference<container::XIndexAccess> xSelectedShapeAccess;
    Reference<drawing::XShape> xSelectedShape;
    if (xSelectionSupplier.is())
    {
        xSelectedShapeAccess = Reference<container::XIndexAccess> (
            xSelectionSupplier->getSelection(), uno::UNO_QUERY);
        xSelectedShape = Reference<drawing::XShape> (
            xSelectionSupplier->getSelection(), uno::UNO_QUERY);
    }

    // Remember the current and new focused shape.
    AccessibleShape* pCurrentlyFocusedShape = NULL;
    AccessibleShape* pNewFocusedShape = NULL;

    ChildDescriptorListType::iterator I, aEnd = maVisibleChildren.end();
    for (I=maVisibleChildren.begin(); I != aEnd; ++I)
    {
        AccessibleShape* pAccessibleShape = I->GetAccessibleShape();
        if (I->mxAccessibleShape.is() && I->mxShape.is() && pAccessibleShape!=NULL)
        {
            bool bShapeIsSelected = false;

            // Look up the shape in the (single or multi-) selection.
            if (xSelectedShape.is())
            {
                if  (I->mxShape == xSelectedShape)
                {
                    bShapeIsSelected = true;
                    pNewFocusedShape = pAccessibleShape;
                }
            }
            else if (xSelectedShapeAccess.is())
            {
                sal_Int32 nCount=xSelectedShapeAccess->getCount();
                for (sal_Int32 i=0; i<nCount&&!bShapeIsSelected; i++)
                    if (xSelectedShapeAccess->getByIndex(i) == I->mxShape)
                    {
                        bShapeIsSelected = true;
                        // In a multi-selection no shape has the focus.
                        if (nCount == 1)
                            pNewFocusedShape = pAccessibleShape;
                    }
            }

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

    // Check if the frame we are in is currently active.  If not then make
    // sure to not send a FOCUSED state change.
    if (xController.is())
    {
        Reference<frame::XFrame> xFrame (xController->getFrame());
        if (xFrame.is())
            if ( ! xFrame->isActive())
                pNewFocusedShape = NULL;
    }

    // Move focus from current to newly focused shape.
    if (pCurrentlyFocusedShape != pNewFocusedShape)
    {
        if (pCurrentlyFocusedShape != NULL)
            pCurrentlyFocusedShape->ResetState (AccessibleStateType::FOCUSED);
        if (pNewFocusedShape != NULL)
            pNewFocusedShape->SetState (AccessibleStateType::FOCUSED);
    }

    // Remember whether there is a shape that now has the focus.
    mpFocusedShape = pNewFocusedShape;
}




bool ChildrenManagerImpl::HasFocus (void)
{
    return mpFocusedShape != NULL;
}




void ChildrenManagerImpl::RemoveFocus (void)
{
    if (mpFocusedShape != NULL)
    {
        mpFocusedShape->ResetState (AccessibleStateType::FOCUSED);
        mpFocusedShape = NULL;
    }
}



void ChildrenManagerImpl::RegisterAsDisposeListener (
    const Reference<drawing::XShape>& xShape)
{
    Reference<lang::XComponent> xComponent (xShape, uno::UNO_QUERY);
    if (xComponent.is())
        xComponent->addEventListener (
            static_cast<document::XEventListener*>(this));
}




void ChildrenManagerImpl::UnregisterAsDisposeListener (
    const Reference<drawing::XShape>& xShape)
{
    Reference<lang::XComponent> xComponent (xShape, uno::UNO_QUERY);
    if (xComponent.is())
        xComponent->removeEventListener (
            static_cast<document::XEventListener*>(this));
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
    // Make sure that the accessible object has the <const>VISIBLE</const>
    // state set.
    AccessibleShape* pAccessibleShape = GetAccessibleShape();
    pAccessibleShape->SetState (AccessibleStateType::VISIBLE);
}




ChildDescriptor::~ChildDescriptor (void)
{
}




AccessibleShape* ChildDescriptor::GetAccessibleShape (void) const
{
    return static_cast<AccessibleShape*> (mxAccessibleShape.get());
}
// -----------------------------------------------------------------------------
void ChildDescriptor::setIndexAtAccessibleShape(sal_Int32 _nIndex)
{
    AccessibleShape* pShape = GetAccessibleShape();
    if ( pShape )
        pShape->setIndexInParent(_nIndex);
}
// -----------------------------------------------------------------------------




void ChildDescriptor::disposeAccessibleObject (AccessibleContextBase& rParent)
{
    if (mxAccessibleShape.is())
    {
        // Send event that the shape has been removed.
        uno::Any aOldValue;
        aOldValue <<= mxAccessibleShape;
        rParent.CommitChange (
            AccessibleEventId::CHILD,
            uno::Any(),
            aOldValue);

        // Dispose and remove the object.
        Reference<lang::XComponent> xComponent (mxAccessibleShape, uno::UNO_QUERY);
        if (xComponent.is())
            xComponent->dispose ();

        mxAccessibleShape = NULL;
    }
}


} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
