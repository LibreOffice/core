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

#include <editeng/AccessibleContextBase.hxx>

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/accessibility/XAccessibleEventListener.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>

#include <unotools/accessiblestatesethelper.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <comphelper/accessibleeventnotifier.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/mutex.hxx>

#include <utility>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

namespace accessibility {

// internal

AccessibleContextBase::AccessibleContextBase (
        const uno::Reference<XAccessible>& rxParent,
        const sal_Int16 aRole)
    :   WeakComponentImplHelper(MutexOwner::maMutex),
        mxParent(rxParent),
        msDescription(),
        meDescriptionOrigin(NotSet),
        msName(),
        meNameOrigin(NotSet),
        mnClientId(0),
        maRole(aRole)
{
    // Create the state set.
    ::utl::AccessibleStateSetHelper* pStateSet  = new ::utl::AccessibleStateSetHelper ();
    mxStateSet = pStateSet;

    // Set some states.  Don't use the SetState method because no events
    // shall be broadcasted (that is not yet initialized anyway).
    pStateSet->AddState (AccessibleStateType::ENABLED);
    pStateSet->AddState (AccessibleStateType::SENSITIVE);
    pStateSet->AddState (AccessibleStateType::SHOWING);
    pStateSet->AddState (AccessibleStateType::VISIBLE);
    pStateSet->AddState (AccessibleStateType::FOCUSABLE);
    pStateSet->AddState (AccessibleStateType::SELECTABLE);

    // Create the relation set.
    ::utl::AccessibleRelationSetHelper* pRelationSet = new ::utl::AccessibleRelationSetHelper ();
    mxRelationSet = pRelationSet;
}

AccessibleContextBase::~AccessibleContextBase()
{
}

bool AccessibleContextBase::SetState (sal_Int16 aState)
{
    ::osl::ClearableMutexGuard aGuard (maMutex);
    ::utl::AccessibleStateSetHelper* pStateSet =
        static_cast< ::utl::AccessibleStateSetHelper*>(mxStateSet.get());
    if ((pStateSet != nullptr) && !pStateSet->contains(aState))
    {
        pStateSet->AddState (aState);
        // Clear the mutex guard so that it is not locked during calls to
        // listeners.
        aGuard.clear();

        // Send event for all states except the DEFUNC state.
        if (aState != AccessibleStateType::DEFUNC)
        {
            uno::Any aNewValue;
            aNewValue <<= aState;
            CommitChange(
                AccessibleEventId::STATE_CHANGED,
                aNewValue,
                uno::Any());
        }
        return true;
    }
    else
        return false;
}


bool AccessibleContextBase::ResetState (sal_Int16 aState)
{
    ::osl::ClearableMutexGuard aGuard (maMutex);
    ::utl::AccessibleStateSetHelper* pStateSet =
        static_cast< ::utl::AccessibleStateSetHelper*>(mxStateSet.get());
    if ((pStateSet != nullptr) && pStateSet->contains(aState))
    {
        pStateSet->RemoveState (aState);
        // Clear the mutex guard so that it is not locked during calls to listeners.
        aGuard.clear();

        uno::Any aOldValue;
        aOldValue <<= aState;
        CommitChange(
            AccessibleEventId::STATE_CHANGED,
            uno::Any(),
            aOldValue);
        return true;
    }
    else
        return false;
}


bool AccessibleContextBase::GetState (sal_Int16 aState)
{
    ::osl::MutexGuard aGuard (maMutex);
    ::utl::AccessibleStateSetHelper* pStateSet =
        static_cast< ::utl::AccessibleStateSetHelper*>(mxStateSet.get());
    if (pStateSet != nullptr)
        return pStateSet->contains(aState);
    else
        // If there is no state set then return false as a default value.
        return false;
}


void AccessibleContextBase::SetRelationSet (
    const uno::Reference<XAccessibleRelationSet>& rxNewRelationSet)
{
    // Try to emit some meaningful events indicating differing relations in
    // both sets.
    typedef std::pair<short int,short int> RD;
    const RD aRelationDescriptors[] = {
        RD(AccessibleRelationType::CONTROLLED_BY, AccessibleEventId::CONTROLLED_BY_RELATION_CHANGED),
        RD(AccessibleRelationType::CONTROLLER_FOR, AccessibleEventId::CONTROLLER_FOR_RELATION_CHANGED),
        RD(AccessibleRelationType::LABELED_BY, AccessibleEventId::LABELED_BY_RELATION_CHANGED),
        RD(AccessibleRelationType::LABEL_FOR, AccessibleEventId::LABEL_FOR_RELATION_CHANGED),
        RD(AccessibleRelationType::MEMBER_OF, AccessibleEventId::MEMBER_OF_RELATION_CHANGED),
        RD(AccessibleRelationType::INVALID, -1),
    };
    for (int i=0; aRelationDescriptors[i].first!=AccessibleRelationType::INVALID; i++)
        if (mxRelationSet->containsRelation(aRelationDescriptors[i].first)
        != rxNewRelationSet->containsRelation(aRelationDescriptors[i].first))
        CommitChange (aRelationDescriptors[i].second, uno::Any(), uno::Any());

    mxRelationSet = rxNewRelationSet;
}


// XAccessible

uno::Reference< XAccessibleContext> SAL_CALL
    AccessibleContextBase::getAccessibleContext()
{
    return this;
}


// XAccessibleContext

/** No children.
*/
sal_Int32 SAL_CALL
       AccessibleContextBase::getAccessibleChildCount()
{
    return 0;
}


/** Forward the request to the shape.  Return the requested shape or throw
    an exception for a wrong index.
*/
uno::Reference<XAccessible> SAL_CALL
    AccessibleContextBase::getAccessibleChild (sal_Int32 nIndex)
{
    ThrowIfDisposed ();
    throw lang::IndexOutOfBoundsException (
        "no child with index " + OUString::number(nIndex),
        nullptr);
}


uno::Reference<XAccessible> SAL_CALL
       AccessibleContextBase::getAccessibleParent()
{
    ThrowIfDisposed ();
    return mxParent;
}


sal_Int32 SAL_CALL
       AccessibleContextBase::getAccessibleIndexInParent()
{
    ThrowIfDisposed ();
    //  Use a simple but slow solution for now.  Optimize later.

    //  Iterate over all the parent's children and search for this object.
    if (mxParent.is())
    {
        uno::Reference<XAccessibleContext> xParentContext (
            mxParent->getAccessibleContext());
        if (xParentContext.is())
        {
            sal_Int32 nChildCount = xParentContext->getAccessibleChildCount();
            for (sal_Int32 i=0; i<nChildCount; i++)
            {
                uno::Reference<XAccessible> xChild (xParentContext->getAccessibleChild (i));
                if (xChild.is())
                {
                    uno::Reference<XAccessibleContext> xChildContext = xChild->getAccessibleContext();
                    if (xChildContext == static_cast<XAccessibleContext*>(this))
                        return i;
                }
            }
        }
    }

    //   Return -1 to indicate that this object's parent does not know about the
    //   object.
    return -1;
}


sal_Int16 SAL_CALL
    AccessibleContextBase::getAccessibleRole()
{
    ThrowIfDisposed ();
    return maRole;
}


OUString SAL_CALL
       AccessibleContextBase::getAccessibleDescription()
{
    ThrowIfDisposed ();

    return msDescription;
}


OUString SAL_CALL
       AccessibleContextBase::getAccessibleName()
{
    ThrowIfDisposed ();

    if (meNameOrigin == NotSet)
    {
        // Do not send an event because this is the first time it has been
        // requested.
        msName = CreateAccessibleName();
        meNameOrigin = AutomaticallyCreated;
    }

    return msName;
}


/** Return a copy of the relation set.
*/
uno::Reference<XAccessibleRelationSet> SAL_CALL
       AccessibleContextBase::getAccessibleRelationSet()
{
    ThrowIfDisposed ();

    // Create a copy of the relation set and return it.
    ::utl::AccessibleRelationSetHelper* pRelationSet =
        static_cast< ::utl::AccessibleRelationSetHelper*>(mxRelationSet.get());
    if (pRelationSet != nullptr)
    {
        return uno::Reference<XAccessibleRelationSet> (
            new ::utl::AccessibleRelationSetHelper (*pRelationSet));
    }
    else
        return uno::Reference<XAccessibleRelationSet>(nullptr);
}


/** Return a copy of the state set.
    Possible states are:
        ENABLED
        SHOWING
        VISIBLE
*/
uno::Reference<XAccessibleStateSet> SAL_CALL
    AccessibleContextBase::getAccessibleStateSet()
{
    ::utl::AccessibleStateSetHelper* pStateSet = nullptr;

    if (rBHelper.bDisposed)
    {
        // We are already disposed!
        // Create a new state set that has only set the DEFUNC state.
        pStateSet = new ::utl::AccessibleStateSetHelper ();
        pStateSet->AddState (AccessibleStateType::DEFUNC);
    }
    else
    {
        // Create a copy of the state set and return it.
        pStateSet = static_cast< ::utl::AccessibleStateSetHelper*>(mxStateSet.get());

        if (pStateSet != nullptr)
            pStateSet = new ::utl::AccessibleStateSetHelper (*pStateSet);
    }

    return uno::Reference<XAccessibleStateSet>(pStateSet);
}


lang::Locale SAL_CALL
       AccessibleContextBase::getLocale()
{
    ThrowIfDisposed ();
    // Delegate request to parent.
    if (mxParent.is())
    {
        uno::Reference<XAccessibleContext> xParentContext (
            mxParent->getAccessibleContext());
        if (xParentContext.is())
            return xParentContext->getLocale ();
    }

    //  No locale and no parent.  Therefore throw exception to indicate this
    //  cluelessness.
    throw IllegalAccessibleComponentStateException ();
}


// XAccessibleEventListener

void SAL_CALL AccessibleContextBase::addAccessibleEventListener (
        const uno::Reference<XAccessibleEventListener >& rxListener)
{
    if (rxListener.is())
    {
        if (rBHelper.bDisposed || rBHelper.bInDispose)
        {
            uno::Reference<uno::XInterface> x (static_cast<lang::XComponent *>(this), uno::UNO_QUERY);
            rxListener->disposing (lang::EventObject (x));
        }
        else
        {
            if (!mnClientId)
                mnClientId = comphelper::AccessibleEventNotifier::registerClient( );
            comphelper::AccessibleEventNotifier::addEventListener( mnClientId, rxListener );
        }
    }
}


void SAL_CALL AccessibleContextBase::removeAccessibleEventListener (
        const uno::Reference<XAccessibleEventListener >& rxListener )
{
    ThrowIfDisposed ();
    if (rxListener.is() && mnClientId)
    {
        sal_Int32 nListenerCount = comphelper::AccessibleEventNotifier::removeEventListener( mnClientId, rxListener );
        if ( !nListenerCount )
        {
            // no listeners anymore
            // -> revoke ourself. This may lead to the notifier thread dying (if we were the last client),
            // and at least to us not firing any events anymore, in case somebody calls
            // NotifyAccessibleEvent, again
            comphelper::AccessibleEventNotifier::revokeClient( mnClientId );
            mnClientId = 0;
        }
    }
}

// XServiceInfo
OUString SAL_CALL AccessibleContextBase::getImplementationName()
{
    return OUString("AccessibleContextBase");
}

sal_Bool SAL_CALL AccessibleContextBase::supportsService (const OUString& sServiceName)
{
    return cppu::supportsService(this, sServiceName);
}

uno::Sequence< OUString > SAL_CALL
       AccessibleContextBase::getSupportedServiceNames()
{
    return {
        "com.sun.star.accessibility.Accessible",
        "com.sun.star.accessibility.AccessibleContext"};
}


// XTypeProvider

uno::Sequence<sal_Int8> SAL_CALL
    AccessibleContextBase::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// internal

void SAL_CALL AccessibleContextBase::disposing()
{
    SetState (AccessibleStateType::DEFUNC);

    ::osl::MutexGuard aGuard (maMutex);

    // Send a disposing to all listeners.
    if ( mnClientId )
    {
        comphelper::AccessibleEventNotifier::revokeClientNotifyDisposing( mnClientId, *this );
        mnClientId =  0;
    }
}


void AccessibleContextBase::SetAccessibleDescription (
    const OUString& rDescription,
    StringOrigin eDescriptionOrigin)
{
    if (eDescriptionOrigin < meDescriptionOrigin
        || (eDescriptionOrigin == meDescriptionOrigin && msDescription != rDescription))
    {
        uno::Any aOldValue, aNewValue;
        aOldValue <<= msDescription;
        aNewValue <<= rDescription;

        msDescription = rDescription;
        meDescriptionOrigin = eDescriptionOrigin;

        CommitChange(
            AccessibleEventId::DESCRIPTION_CHANGED,
            aNewValue,
            aOldValue);
    }
}


void AccessibleContextBase::SetAccessibleName (
    const OUString& rName,
    StringOrigin eNameOrigin)
{
    if (eNameOrigin < meNameOrigin
        || (eNameOrigin == meNameOrigin && msName != rName))
    {
        uno::Any aOldValue, aNewValue;
        aOldValue <<= msName;
        aNewValue <<= rName;

        msName = rName;
        meNameOrigin = eNameOrigin;

        CommitChange(
            AccessibleEventId::NAME_CHANGED,
            aNewValue,
            aOldValue);
    }
}


OUString AccessibleContextBase::CreateAccessibleName()
{
    return OUString("Empty Name");
}


void AccessibleContextBase::CommitChange (
    sal_Int16 nEventId,
    const uno::Any& rNewValue,
    const uno::Any& rOldValue)
{
    // Do not call FireEvent and do not even create the event object when no
    // listener has been registered yet.  Creating the event object can
    // otherwise lead to a crash.  See issue 93419 for details.
    if (mnClientId != 0)
    {
        AccessibleEventObject aEvent (
            static_cast<XAccessibleContext*>(this),
            nEventId,
            rNewValue,
            rOldValue);

        FireEvent (aEvent);
    }
}


void AccessibleContextBase::FireEvent (const AccessibleEventObject& aEvent)
{
    if (mnClientId)
        comphelper::AccessibleEventNotifier::addEvent( mnClientId, aEvent );
}


void AccessibleContextBase::ThrowIfDisposed()
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException ("object has been already disposed",
            static_cast<uno::XWeak*>(this));
    }
}


bool AccessibleContextBase::IsDisposed()
{
    return (rBHelper.bDisposed || rBHelper.bInDispose);
}


void AccessibleContextBase::SetAccessibleRole( sal_Int16 _nRole )
{
    maRole = _nRole;
}


} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
