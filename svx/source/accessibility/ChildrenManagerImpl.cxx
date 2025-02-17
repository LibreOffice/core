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

#include <sal/config.h>

#include <cassert>

#include "ChildrenManagerImpl.hxx"
#include <svx/ShapeTypeHandler.hxx>
#include <svx/AccessibleControlShape.hxx>
#include <svx/AccessibleShape.hxx>
#include <svx/AccessibleShapeInfo.hxx>
#include <svx/IAccessibleViewForwarder.hxx>
#include <utility>
#include <vcl/svapp.hxx>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XShapeEventBroadcaster.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <comphelper/lok.hxx>
#include <o3tl/safeint.hxx>
#include <rtl/ustring.hxx>
#include <tools/debug.hxx>
#include <svx/SvxShapeTypes.hxx>
#include <vcl/window.hxx>
#include <shapecollection.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using ::com::sun::star::uno::Reference;

namespace accessibility {

namespace
{
void adjustIndexInParentOfShapes(ChildDescriptorListType& _rList)
{
    sal_Int32 i=0;
    for (auto& rItem : _rList)
    {
        rItem.setIndexAtAccessibleShape(i);
        ++i;
    }
}
}

// AccessibleChildrenManager
ChildrenManagerImpl::ChildrenManagerImpl (
    uno::Reference<XAccessible> xParent,
    uno::Reference<drawing::XShapes> xShapeList,
    const AccessibleShapeTreeInfo& rShapeTreeInfo,
    AccessibleContextBase& rContext)
    : mxShapeList (std::move(xShapeList)),
      mxParent (std::move(xParent)),
      maShapeTreeInfo (rShapeTreeInfo),
      mrContext (rContext),
      mpFocusedShape(nullptr)
{
}


ChildrenManagerImpl::~ChildrenManagerImpl()
{
    DBG_ASSERT (m_bDisposed, "~AccessibleDrawDocumentView: object has not been disposed");
}


void ChildrenManagerImpl::Init()
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


sal_Int64 ChildrenManagerImpl::GetChildCount() const noexcept
{
    return maVisibleChildren.size();
}


const css::uno::Reference<css::drawing::XShape>& ChildrenManagerImpl::GetChildShape(sal_Int64 nIndex)
{
    // Check whether the given index is valid.
    if (nIndex < 0 || o3tl::make_unsigned(nIndex) >= maVisibleChildren.size())
        throw lang::IndexOutOfBoundsException (
            "no accessible child with index " + OUString::number(nIndex),
            mxParent);
    return maVisibleChildren[nIndex].mxShape;
}

/** Return the requested accessible child object.  Create it if it is not
    yet in the cache.
*/
uno::Reference<XAccessible>
    ChildrenManagerImpl::GetChild (sal_Int64 nIndex)
{
    // Check whether the given index is valid.
    if (nIndex < 0 || o3tl::make_unsigned(nIndex) >= maVisibleChildren.size())
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
{
    if ( ! rChildDescriptor.mxAccessibleShape.is())
    {
        SolarMutexGuard g;
        // Make sure that the requested accessible object has not been
        // created while locking the global mutex.
        if ( ! rChildDescriptor.mxAccessibleShape.is())
        {
            AccessibleShapeInfo aShapeInfo(
                        rChildDescriptor.mxShape,
                        mxParent,
                        this);
            // Create accessible object that corresponds to the descriptor's
            // shape.
            rtl::Reference<AccessibleShape> pShape(
                ShapeTypeHandler::Instance().CreateAccessibleObject (
                    aShapeInfo,
                    maShapeTreeInfo));
            rChildDescriptor.mxAccessibleShape = pShape;
            if ( pShape.is() )
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
    if (maShapeTreeInfo.GetViewForwarder() == nullptr)
        return;
    tools::Rectangle aVisibleArea = maShapeTreeInfo.GetViewForwarder()->GetVisibleArea();

    // 1. Create a local list of visible shapes.
    ChildDescriptorListType aChildList;
    CreateListOfVisibleShapes (aChildList);

    // 2. Replace the current list of visible shapes with the new one.  Do
    // the same with the visible area.
    {
        SolarMutexGuard g;

        // Use swap to copy the contents of the new list in constant time.
        maVisibleChildren.swap (aChildList);

        // 3. Merge the information that is already known about the visible
        // shapes from the previous list into the new list and identify
        // old children that are now unused
        std::vector<ChildDescriptor*> aUnusedChildList = MergeAccessibilityInformation (aChildList);

        adjustIndexInParentOfShapes(maVisibleChildren);

        // aChildList now contains all the old children, while maVisibleChildren
        // contains all the current children

        // 4. Find all shapes in the old list that are not in the current list,
        // send appropriate events and remove the accessible shape.

        // Do this *after* we have set our new list of children, because
        // removing a child may cause

        // ChildDescriptor::disposeAccessibleObject -->
        // AccessibleContextBase::CommitChange -->
        // AtkListener::notifyEvent ->
        // AtkListener::handleChildRemoved ->
        // AtkListener::updateChildList
        // AccessibleDrawDocumentView::getAccessibleChildCount ->
        // ChildrenManagerImpl::GetChildCount ->
        // maVisibleChildren.size()

        // to be fired, and so the operations will take place on
        // the list we are trying to replace

        RemoveNonVisibleChildren (aUnusedChildList);

        aUnusedChildList.clear();
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
    if (bCreateNewObjectsOnDemand)
        return;

    //operate on a copy of the list and restore it afterwards to guard
    //against the pathological case where maVisibleChildren gets modified
    //by other calls to this object while CreateAccessibilityObjects
    //executes which can happen when java is disabled and the "enable-java"
    //dialog appears during the instantiation of the linguistic components
    //triggered by the creation of shapes belonging to the a11y objects
    //
    //i.e. launch start-center, launch impress with java disabled and
    //a java-using linguistic component installed
    maVisibleChildren.swap(aChildList);
    CreateAccessibilityObjects(aChildList);
    maVisibleChildren.swap(aChildList);
}

void ChildrenManagerImpl::CreateListOfVisibleShapes (
    ChildDescriptorListType& raDescriptorList)
{
    SolarMutexGuard g;

    OSL_ASSERT (maShapeTreeInfo.GetViewForwarder() != nullptr);

    tools::Rectangle aVisibleArea = maShapeTreeInfo.GetViewForwarder()->GetVisibleArea();

    // Add the visible shapes for which the accessible objects already exist.
    for (const auto& rpShape : maAccessibleShapes)
    {
        if (rpShape.is())
        {
            uno::Reference<XAccessibleComponent> xComponent (
                rpShape->getAccessibleContext(), uno::UNO_QUERY);
            if (xComponent.is())
            {
                // The bounding box of the object already is clipped to the
                // visible area.  The object is therefore visible if the
                // bounding box has non-zero extensions.
                awt::Rectangle aPixelBBox (xComponent->getBounds());
                if ((aPixelBBox.Width > 0) && (aPixelBBox.Height > 0))
                    raDescriptorList.emplace_back(rpShape);
            }
        }
    }

    // Add the visible shapes for which only the XShapes exist.
    if (!mxShapeList.is() || !mxShapeList->hasElements())
        return;

    sal_Int32 nShapeCount = mxShapeList->getCount();
    raDescriptorList.reserve( nShapeCount );
    awt::Point aPos;
    awt::Size aSize;
    tools::Rectangle aBoundingBox;
    uno::Reference<drawing::XShape> xShape;
    for (sal_Int32 i=0; i<nShapeCount; ++i)
    {
        mxShapeList->getByIndex(i) >>= xShape;
        aPos = xShape->getPosition();
        aSize = xShape->getSize();

        aBoundingBox.SetLeft( aPos.X );
        aBoundingBox.SetTop( aPos.Y );
        aBoundingBox.SetRight( aPos.X + aSize.Width );
        aBoundingBox.SetBottom( aPos.Y + aSize.Height );

        // Insert shape if it is visible, i.e. its bounding box overlaps
        // the visible area. In the LOK case we skip the overlap check
        // since we could remove a shape that is visible on the client.
        if ( aBoundingBox.Overlaps(aVisibleArea) || comphelper::LibreOfficeKit::isActive())
            raDescriptorList.emplace_back(xShape);
    }
}

namespace
{

bool childDescriptorLess(const ChildDescriptor& lhs, const ChildDescriptor& rhs)
{

    auto pLhsShape = lhs.mxShape.get();
    auto pRhsShape = rhs.mxShape.get();
    if (pLhsShape || pRhsShape)
        return pLhsShape < pRhsShape;
    return lhs.mxAccessibleShape.get() < rhs.mxAccessibleShape.get();
}

bool childDescriptorPtrLess(const ChildDescriptor* lhs, const ChildDescriptor* rhs)
{
    return childDescriptorLess(*lhs, *rhs);
}

}
    
void ChildrenManagerImpl::RemoveNonVisibleChildren (
    const std::vector<ChildDescriptor*>& rNonVisibleChildren)
{
    for (ChildDescriptor* pChild : rNonVisibleChildren)
    {
        // The child is disposed when there is a UNO shape from which
        // the accessible shape can be created when the shape becomes
        // visible again.  When there is no such UNO shape then simply
        // reset the descriptor but keep the accessibility object.
        if (pChild->mxShape.is())
        {
            UnregisterAsDisposeListener (pChild->mxShape);
            pChild->disposeAccessibleObject (mrContext);
        }
        else
        {
            AccessibleShape* pAccessibleShape = pChild->GetAccessibleShape();
            pAccessibleShape->ResetState (AccessibleStateType::VISIBLE);
            pChild->mxAccessibleShape = nullptr;
        }
    }
}

std::vector<ChildDescriptor*> ChildrenManagerImpl::MergeAccessibilityInformation (
    ChildDescriptorListType& raOldChildList)

{
    // create a working copy of the vector of current children with pointers to elements,
    // sort the old list and copy by mxShape, and then walk old/current lists in parallel,
    // which avoids an O(n^2) loop
    // (order of maVisibleChildren must remain unchanged to not randomly change a11y tree)
    std::vector<ChildDescriptor*> aSortedVisibleChildren(maVisibleChildren.size());
    std::transform(maVisibleChildren.begin(), maVisibleChildren.end(),
                   aSortedVisibleChildren.begin(), [](auto& e) {return &e;});
    std::sort(aSortedVisibleChildren.begin(), aSortedVisibleChildren.end(), childDescriptorPtrLess);

    // old list can be reordered without problems
    std::sort(raOldChildList.begin(), raOldChildList.end(), childDescriptorLess);

    ChildDescriptorListType::const_iterator aOldChildDescriptor = raOldChildList.begin();
    ChildDescriptorListType::const_iterator aEndOldChildren = raOldChildList.end();
    for (ChildDescriptor* pChild : aSortedVisibleChildren)
    {
        while (aOldChildDescriptor != aEndOldChildren && childDescriptorLess(*aOldChildDescriptor, *pChild))
        {
            aOldChildDescriptor++;
        }

        // Copy accessible shape if that exists in the old descriptor.
        if (aOldChildDescriptor != aEndOldChildren && *aOldChildDescriptor == *pChild &&
            aOldChildDescriptor->mxAccessibleShape.is())
        {
            pChild->mxAccessibleShape = aOldChildDescriptor->mxAccessibleShape;
            pChild->mbCreateEventPending = false;
        }
        else
            RegisterAsDisposeListener (pChild->mxShape);
    }

    // collect list of children that are in the old, but not the new vector
    std::vector<ChildDescriptor*> aObsoleteChildren;

    auto newIt = aSortedVisibleChildren.begin();
    auto newEnd = aSortedVisibleChildren.end();
    for (ChildDescriptor& rOldChild : raOldChildList)
    {
        while (newIt != newEnd && childDescriptorLess(**newIt, rOldChild))
            newIt++;
        if (newIt == newEnd || !(**newIt == rOldChild) )
            aObsoleteChildren.push_back(&rOldChild);
    }

    return aObsoleteChildren;
}

void ChildrenManagerImpl::SendVisibleAreaEvents (
    ChildDescriptorListType& raNewChildList)
{
    for (const auto& rChild : raNewChildList)
    {
        // Tell shape of changed visible area.  To do this, fake a
        // change of the view forwarder.  (Actually we usually get here
        // as a result of a change of the view forwarder).
        AccessibleShape* pShape = rChild.GetAccessibleShape ();
        if (pShape != nullptr)
            pShape->ViewForwarderChanged();
    }
}


void ChildrenManagerImpl::CreateAccessibilityObjects (
    ChildDescriptorListType& raNewChildList)
{
    sal_Int32 nPos = 0;
    for ( auto& rChild : raNewChildList)
    {
        // Create the associated accessible object when the flag says so and
        // it does not yet exist.
        if ( ! rChild.mxAccessibleShape.is() )
            GetChild (rChild, nPos);
        if (rChild.mxAccessibleShape.is() && rChild.mbCreateEventPending)
        {
            rChild.mbCreateEventPending = false;
            mrContext.CommitChange (
                AccessibleEventId::CHILD,
                uno::Any(uno::Reference<XAccessible>(rChild.mxAccessibleShape)),
                uno::Any(), -1);
        }
        ++nPos;
    }
}


void ChildrenManagerImpl::AddShape (const Reference<drawing::XShape>& rxShape)
{
    if (!rxShape.is())
        return;

    SolarMutexClearableGuard aGuard;

    // Test visibility of the shape.
    tools::Rectangle aVisibleArea = maShapeTreeInfo.GetViewForwarder()->GetVisibleArea();
    awt::Point aPos = rxShape->getPosition();
    awt::Size aSize = rxShape->getSize();

    tools::Rectangle aBoundingBox (
        aPos.X,
        aPos.Y,
        aPos.X + aSize.Width,
        aPos.Y + aSize.Height);

    // Add the shape only when it belongs to the list of shapes stored
    // in mxShapeList (which is either a page or a group shape).
    Reference<container::XChild> xChild (rxShape, uno::UNO_QUERY);
    if (!xChild.is())
        return;

    Reference<drawing::XShapes> xParent (xChild->getParent(), uno::UNO_QUERY);
    if (xParent != mxShapeList)
        return;

    if (!aBoundingBox.Overlaps(aVisibleArea) && !comphelper::LibreOfficeKit::isActive())
        return;

    // Add shape to list of visible shapes.
    maVisibleChildren.emplace_back(rxShape);

    // Create accessibility object.
    ChildDescriptor& rDescriptor = maVisibleChildren.back();
    GetChild (rDescriptor, maVisibleChildren.size()-1);

    // Inform listeners about new child.
    uno::Any aNewShape;
    aNewShape <<= uno::Reference<XAccessible>(rDescriptor.mxAccessibleShape);
    aGuard.clear();
    mrContext.CommitChange (
        AccessibleEventId::CHILD,
        aNewShape,
        uno::Any(),
        maVisibleChildren.size() - 1);
    RegisterAsDisposeListener(rxShape);
}


void ChildrenManagerImpl::RemoveShape (const Reference<drawing::XShape>& rxShape)
{
    if (!rxShape.is())
        return;

    SolarMutexGuard g;

    // Search shape in list of visible children.
    ChildDescriptorListType::iterator I (
        ::std::find (maVisibleChildren.begin(), maVisibleChildren.end(),
            ChildDescriptor (rxShape)));
    if (I == maVisibleChildren.end())
        return;

    // Remove descriptor from that list.
    rtl::Reference<AccessibleShape> xHoldAlive(I->mxAccessibleShape);

    UnregisterAsDisposeListener (I->mxShape);
    // Dispose the accessible object.
    I->disposeAccessibleObject (mrContext);

    // Now we can safely remove the child descriptor and thus
    // invalidate the iterator.
    maVisibleChildren.erase (I);

    adjustIndexInParentOfShapes(maVisibleChildren);
}


void ChildrenManagerImpl::SetShapeList (const css::uno::Reference<css::drawing::XShapes>& xShapeList)
{
    mxShapeList = xShapeList;
}


void ChildrenManagerImpl::AddAccessibleShape (rtl::Reference<AccessibleShape> const & shape)
{
    assert(shape.is());
    maAccessibleShapes.push_back (shape);
}


void ChildrenManagerImpl::ClearAccessibleShapeList()
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
        uno::Any(), -1);

    // Now the objects in the local lists can be safely disposed without
    // having problems with callers that want to update their child lists.

    // Clear the list of visible accessible objects.  Objects not created on
    // demand for XShapes are treated below.
    for (auto& rChild : aLocalVisibleChildren)
        if ( rChild.mxAccessibleShape.is() && rChild.mxShape.is() )
        {
            rChild.mxAccessibleShape->dispose();
            rChild.mxAccessibleShape = nullptr;
        }

    // Dispose all objects in the accessible shape list.
    for (auto& rpShape : aLocalAccessibleShapes)
        if (rpShape.is())
        {
            // Dispose the object.
            rpShape->dispose();
            rpShape = nullptr;
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
        SolarMutexGuard g;
        xCurrentBroadcaster = maShapeTreeInfo.GetModelBroadcaster();
        xCurrentController = maShapeTreeInfo.GetController();
        xCurrentSelectionSupplier.set( xCurrentController, uno::UNO_QUERY);
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
    if (xNewSelectionSupplier == xCurrentSelectionSupplier)
        return;

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

// lang::XEventListener
void SAL_CALL
    ChildrenManagerImpl::disposing (const lang::EventObject& rEventObject)
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
            I->mxShape = nullptr;
        }
    }
}

// document::XEventListener
/** Listen for new and removed shapes.
*/
void SAL_CALL
    ChildrenManagerImpl::notifyEvent (
        const document::EventObject& rEventObject)
{
    // tdf#158169 if we are already disposed, execute no actions, but inform the
    // caller that we are disposed
    if ( m_bDisposed )
        throw lang::DisposedException();

    if (rEventObject.EventName == "ShapeInserted")
        AddShape (Reference<drawing::XShape>(rEventObject.Source, uno::UNO_QUERY));
    else if (rEventObject.EventName == "ShapeRemoved")
        RemoveShape (Reference<drawing::XShape>(rEventObject.Source, uno::UNO_QUERY));
    // else ignore unknown event.
}

// view::XSelectionChangeListener
void  SAL_CALL
    ChildrenManagerImpl::selectionChanged (const lang::EventObject& /*rEvent*/)
{
    UpdateSelection ();
}


void ChildrenManagerImpl::impl_dispose()
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

    maShapeTreeInfo.SetController (nullptr);

    try
    {
        // Remove from broadcaster.
        if (maShapeTreeInfo.GetModelBroadcaster().is())
            maShapeTreeInfo.GetModelBroadcaster()->removeEventListener (
                static_cast<document::XEventListener*>(this));
        maShapeTreeInfo.SetModelBroadcaster (nullptr);
    }
    catch( uno::RuntimeException& )
    {}

    ClearAccessibleShapeList ();
    SetShapeList (nullptr);
}


void ChildrenManagerImpl::disposing(std::unique_lock<std::mutex>&)
{
    impl_dispose();
}

// IAccessibleViewForwarderListener
void ChildrenManagerImpl::ViewForwarderChanged()
{
    Update(false);
}

// IAccessibleParent
bool ChildrenManagerImpl::ReplaceChild (
    AccessibleShape* pCurrentChild,
    const css::uno::Reference< css::drawing::XShape >& _rxShape,
    const tools::Long /*_nIndex*/,
    const AccessibleShapeTreeInfo& _rShapeTreeInfo)
{
    // Iterate over the visible children.  If one of them has an already
    // created accessible object that matches pCurrentChild then replace
    // it.  Otherwise the child to replace is either not in the list or has
    // not ye been created (and is therefore not in the list, too) and a
    // replacement is not necessary.
    auto I = std::find_if(maVisibleChildren.begin(), maVisibleChildren.end(),
        [&pCurrentChild](const ChildDescriptor& rChild) { return rChild.GetAccessibleShape() == pCurrentChild; });

    if (I != maVisibleChildren.end())
    {
        // Dispose the current child and send an event about its deletion.
        pCurrentChild->dispose();
        mrContext.CommitChange (
            AccessibleEventId::CHILD,
            uno::Any(),
            uno::Any (uno::Reference<XAccessible>(I->mxAccessibleShape)), -1);

        // Replace with replacement and send an event about existence
        // of the new child.
        AccessibleShapeInfo aShapeInfo( _rxShape, pCurrentChild->getAccessibleParent(), this );
        // create the new child
        rtl::Reference<AccessibleShape> pNewChild(ShapeTypeHandler::Instance().CreateAccessibleObject (
            aShapeInfo,
            _rShapeTreeInfo
        ));
        if ( pNewChild.is() )
            pNewChild->Init();

        I->mxAccessibleShape = pNewChild.get();
        mrContext.CommitChange (
            AccessibleEventId::CHILD,
            uno::Any (uno::Reference<XAccessible>(I->mxAccessibleShape)),
            uno::Any(), -1);

        return true;
    }

    // When not found among the visible children we have to search the list
    // of accessible shapes.  This is not yet implemented.
    return false;
}

// Add the impl method for IAccessibleParent interface
AccessibleControlShape * ChildrenManagerImpl::GetAccControlShapeFromModel(css::beans::XPropertySet* pSet)
{
    sal_Int64 count = GetChildCount();
    for (sal_Int64 index=0;index<count;index++)
    {
        AccessibleShape* pAccShape = maVisibleChildren[index].GetAccessibleShape();
        if (pAccShape && ::accessibility::ShapeTypeHandler::Instance().GetTypeId(pAccShape->GetXShape()) == DRAWING_CONTROL)
        {
            auto* pCtlAccShape = static_cast<::accessibility::AccessibleControlShape*>(pAccShape);
            if (pCtlAccShape->GetControlModel() == pSet)
                return pCtlAccShape;
        }
    }
    return nullptr;
}
uno::Reference<XAccessible>
    ChildrenManagerImpl::GetAccessibleCaption (const uno::Reference<drawing::XShape>& xShape)
{
    auto I = std::find_if(maVisibleChildren.begin(), maVisibleChildren.end(),
        [&xShape](const ChildDescriptor& rChild) { return rChild.mxShape.get() == xShape.get(); });
    if (I != maVisibleChildren.end())
        return I->mxAccessibleShape;
    return uno::Reference<XAccessible> ();
}

/** Update the <const>SELECTED</const> and the <const>FOCUSED</const> state
    of all visible children.  Maybe this should be changed to all children.

    Iterate over all descriptors of visible accessible shapes and look them
    up in the selection.

    If there is no valid controller then all shapes are deselected and
    unfocused.  If the controller's frame is not active then all shapes are
    unfocused.
*/
void ChildrenManagerImpl::UpdateSelection()
{
    // Remember the current and new focused shape.
    AccessibleShape* pCurrentlyFocusedShape = nullptr;
    AccessibleShape* pNewFocusedShape = nullptr;
    typedef std::pair< AccessibleShape* , sal_Bool > PAIR_SHAPE;//sal_Bool Selected,UnSelected.
    typedef std::vector< PAIR_SHAPE > VEC_SHAPE;
    VEC_SHAPE vecSelect;
    int nAddSelect=0;
    bool bHasSelectedShape=false;
    if (!maVisibleChildren.empty())
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
            xSelectedShapeAccess.set( xSelectionSupplier->getSelection(), uno::UNO_QUERY);
            xSelectedShape.set( xSelectionSupplier->getSelection(), uno::UNO_QUERY);
        }

        // tdf#139220 to quickly find if a given drawing::XShape is selected
        std::vector<css::uno::Reference<css::drawing::XShape>> aSortedSelectedShapes;
        if (!xSelectedShape.is() && xSelectedShapeAccess.is())
        {
            sal_Int32 nCount = xSelectedShapeAccess->getCount();
            if (auto pSvxShape = dynamic_cast<SvxShapeCollection*>(xSelectedShapeAccess.get()))
            {
                aSortedSelectedShapes = pSvxShape->getAllShapes();
            }
            else
            {
                aSortedSelectedShapes.reserve(nCount);
                for (sal_Int32 i = 0; i < nCount; ++i)
                {
                    css::uno::Reference<css::drawing::XShape> xShape(xSelectedShapeAccess->getByIndex(i), uno::UNO_QUERY);
                    aSortedSelectedShapes.push_back(xShape);
                }
            }
            std::sort(aSortedSelectedShapes.begin(), aSortedSelectedShapes.end());
        }

        for (const auto& rChild : maVisibleChildren)
        {
            AccessibleShape* pAccessibleShape = rChild.GetAccessibleShape();
            if (rChild.mxAccessibleShape.is() && rChild.mxShape.is() && pAccessibleShape!=nullptr)
            {
                short nRole = pAccessibleShape->getAccessibleRole();
                bool bDrawShape = (
                    nRole == AccessibleRole::GRAPHIC ||
                    nRole == AccessibleRole::EMBEDDED_OBJECT ||
                    nRole == AccessibleRole::SHAPE ||
                    nRole == AccessibleRole::IMAGE_MAP ||
                    nRole == AccessibleRole::TABLE_CELL ||
                    nRole == AccessibleRole::TABLE );
                bool bShapeIsSelected = false;

                // Look up the shape in the (single or multi-) selection.
                if (xSelectedShape.is())
                {
                    if  (rChild.mxShape == xSelectedShape)
                    {
                        bShapeIsSelected = true;
                        pNewFocusedShape = pAccessibleShape;
                    }
                }
                else if (!aSortedSelectedShapes.empty())
                {
                    if (std::binary_search(aSortedSelectedShapes.begin(), aSortedSelectedShapes.end(), rChild.mxShape))
                    {
                        bShapeIsSelected = true;
                        // In a multi-selection no shape has the focus.
                        if (aSortedSelectedShapes.size() == 1)
                            pNewFocusedShape = pAccessibleShape;
                    }
                }

                // Set or reset the SELECTED state.
                if (bShapeIsSelected)
                {
                    if (pAccessibleShape->SetState (AccessibleStateType::SELECTED))
                    {
                        if (bDrawShape)
                        {
                            vecSelect.emplace_back(pAccessibleShape,true);
                            ++nAddSelect;
                        }
                    }
                    else
                    {//Selected not change,has selected shape before
                        bHasSelectedShape=true;
                    }
                }
                else
                    //pAccessibleShape->ResetState (AccessibleStateType::SELECTED);
                {
                    if(pAccessibleShape->ResetState (AccessibleStateType::SELECTED))
                    {
                        if(bDrawShape)
                        {
                            vecSelect.emplace_back(pAccessibleShape,false);
                        }
                    }
                }
                // Does the shape have the current selection?
                if (pAccessibleShape->GetState (AccessibleStateType::FOCUSED))
                    pCurrentlyFocusedShape = pAccessibleShape;
            }
        }
    }

    vcl::Window *pParentWindow = maShapeTreeInfo.GetWindow();
    bool bShapeActive= false;
    // For table cell, the table's parent must be checked to make sure it has focus.
    if (pParentWindow)
    {
        vcl::Window *pPWindow = pParentWindow->GetParent();
        if (pParentWindow->HasFocus() || (pPWindow && pPWindow->HasFocus()))
            bShapeActive =true;
    }
    // Move focus from current to newly focused shape.
    if (pCurrentlyFocusedShape != pNewFocusedShape)
    {
        if (pCurrentlyFocusedShape != nullptr)
            pCurrentlyFocusedShape->ResetState (AccessibleStateType::FOCUSED);
        if (pNewFocusedShape != nullptr && bShapeActive)
            pNewFocusedShape->SetState (AccessibleStateType::FOCUSED);
    }

    if (nAddSelect >= 10 )//fire selection  within
    {
        mrContext.CommitChange(AccessibleEventId::SELECTION_CHANGED_WITHIN,uno::Any(),uno::Any(), -1);
        nAddSelect =0 ;//not fire selection event
    }
    for (VEC_SHAPE::reverse_iterator vi = vecSelect.rbegin(), aEndVecSelect = vecSelect.rend(); vi != aEndVecSelect ;++vi)
    {
        PAIR_SHAPE &pairShape= *vi;
        Reference< XAccessible > xShape(pairShape.first);
        uno::Any anyShape;
        anyShape <<= xShape;

        if (pairShape.second)//Selection add
        {
            if (bHasSelectedShape)
            {
                if (  nAddSelect > 0 )
                {
                    mrContext.CommitChange(AccessibleEventId::SELECTION_CHANGED_ADD,anyShape,uno::Any(), -1);
                }
            }
            else
            {
                //if has not selected shape ,first selected shape is fire selection event;
                if (nAddSelect > 0 )
                {
                    mrContext.CommitChange(AccessibleEventId::SELECTION_CHANGED,anyShape,uno::Any(), -1);
                }
                if (nAddSelect > 1 )//check other selected shape fire selection add event
                {
                    bHasSelectedShape=true;
                }
            }
        }
        else //selection remove
        {
            mrContext.CommitChange(AccessibleEventId::SELECTION_CHANGED_REMOVE,anyShape,uno::Any(), -1);
        }
    }

    // We need to know when text content is no more edited but shape is still selected.
    // For instance when ESC is pressed.
    // The only difference is provided by nSelectedChildCount: on editing is equal to 1.
    // In the following a shape get selected, but it was already selected, anyway editing is no more active.
    if (comphelper::LibreOfficeKit::isActive() && pNewFocusedShape && nAddSelect == 0)
    {
        sal_Int64 nChildCount = pNewFocusedShape->getAccessibleChildCount();
        sal_Int64 nSelectedChildCount = pNewFocusedShape->getSelectedAccessibleChildCount();
        if (nChildCount > 0 && nSelectedChildCount == 0)
        {
            Reference< XAccessible > xShape(pNewFocusedShape);
            uno::Any anyShape;
            anyShape <<= xShape;
            mrContext.CommitChange(AccessibleEventId::SELECTION_CHANGED,anyShape,uno::Any(), -1);
        }
    }

    // Remember whether there is a shape that now has the focus.
    mpFocusedShape = pNewFocusedShape;
}


bool ChildrenManagerImpl::HasFocus() const
{
    return mpFocusedShape != nullptr;
}


void ChildrenManagerImpl::RemoveFocus()
{
    if (mpFocusedShape != nullptr)
    {
        mpFocusedShape->ResetState (AccessibleStateType::FOCUSED);
        mpFocusedShape = nullptr;
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

// AccessibleChildDescriptor
ChildDescriptor::ChildDescriptor (const Reference<drawing::XShape>& xShape)
    : mxShape (xShape),
      mbCreateEventPending (true)
{
    // Empty.
}


ChildDescriptor::ChildDescriptor (const rtl::Reference<AccessibleShape>& rxAccessibleShape)
    : mxAccessibleShape (rxAccessibleShape),
      mbCreateEventPending (true)
{
    // Make sure that the accessible object has the <const>VISIBLE</const>
    // state set.
    AccessibleShape* pAccessibleShape = GetAccessibleShape();
    pAccessibleShape->SetState (AccessibleStateType::VISIBLE);
}

void ChildDescriptor::setIndexAtAccessibleShape(sal_Int32 _nIndex)
{
    AccessibleShape* pShape = GetAccessibleShape();
    if ( pShape )
        pShape->setIndexInParent(_nIndex);
}


void ChildDescriptor::disposeAccessibleObject (AccessibleContextBase& rParent)
{
    if (!mxAccessibleShape.is())
        return;

    // Send event that the shape has been removed.
    uno::Any aOldValue;
    aOldValue <<= uno::Reference<XAccessible>(mxAccessibleShape);
    rParent.CommitChange (
        AccessibleEventId::CHILD,
        uno::Any(),
        aOldValue, -1);

    // Dispose and remove the object.
    if (mxAccessibleShape.is())
        mxAccessibleShape->dispose();

    mxAccessibleShape = nullptr;
}


} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
