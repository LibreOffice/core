/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
#include <comphelper/servicehelper.hxx>
#include <osl/mutex.hxx>

#include <utility>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using ::com::sun::star::uno::Reference;

namespace accessibility {




typedef ::cppu::PartialWeakComponentImplHelper4<
    ::com::sun::star::accessibility::XAccessible,
    ::com::sun::star::accessibility::XAccessibleContext,
    ::com::sun::star::accessibility::XAccessibleEventBroadcaster,
    ::com::sun::star::lang::XServiceInfo> BaseClass;

AccessibleContextBase::AccessibleContextBase (
        const uno::Reference<XAccessible>& rxParent,
        const sal_Int16 aRole)
    :   BaseClass (MutexOwner::maMutex),
        mxStateSet (NULL),
        mxRelationSet (NULL),
        mxParent(rxParent),
        msDescription(),
        meDescriptionOrigin(NotSet),
        msName(),
        meNameOrigin(NotSet),
        mnClientId(0),
        maRole(aRole)
{
    
    ::utl::AccessibleStateSetHelper* pStateSet  = new ::utl::AccessibleStateSetHelper ();
    mxStateSet = pStateSet;

    
    
    if (pStateSet != NULL)
    {
        pStateSet->AddState (AccessibleStateType::ENABLED);
        pStateSet->AddState (AccessibleStateType::SENSITIVE);
        pStateSet->AddState (AccessibleStateType::SHOWING);
        pStateSet->AddState (AccessibleStateType::VISIBLE);
        pStateSet->AddState (AccessibleStateType::FOCUSABLE);
        pStateSet->AddState (AccessibleStateType::SELECTABLE);
    }

    
    ::utl::AccessibleRelationSetHelper* pRelationSet = new ::utl::AccessibleRelationSetHelper ();
    mxRelationSet = pRelationSet;
}




AccessibleContextBase::~AccessibleContextBase(void)
{
}




sal_Bool AccessibleContextBase::SetState (sal_Int16 aState)
{
    ::osl::ClearableMutexGuard aGuard (maMutex);
    ::utl::AccessibleStateSetHelper* pStateSet =
        static_cast< ::utl::AccessibleStateSetHelper*>(mxStateSet.get());
    if ((pStateSet != NULL) && !pStateSet->contains(aState))
    {
        pStateSet->AddState (aState);
        
        
        aGuard.clear();

        
        if (aState != AccessibleStateType::DEFUNC)
        {
            uno::Any aNewValue;
            aNewValue <<= aState;
            CommitChange(
                AccessibleEventId::STATE_CHANGED,
                aNewValue,
                uno::Any());
        }
        return sal_True;
    }
    else
        return sal_False;
}




sal_Bool AccessibleContextBase::ResetState (sal_Int16 aState)
{
    ::osl::ClearableMutexGuard aGuard (maMutex);
    ::utl::AccessibleStateSetHelper* pStateSet =
        static_cast< ::utl::AccessibleStateSetHelper*>(mxStateSet.get());
    if ((pStateSet != NULL) && pStateSet->contains(aState))
    {
        pStateSet->RemoveState (aState);
        
        aGuard.clear();

        uno::Any aOldValue;
        aOldValue <<= aState;
        CommitChange(
            AccessibleEventId::STATE_CHANGED,
            uno::Any(),
            aOldValue);
        return sal_True;
    }
    else
        return sal_False;
}




sal_Bool AccessibleContextBase::GetState (sal_Int16 aState)
{
    ::osl::MutexGuard aGuard (maMutex);
    ::utl::AccessibleStateSetHelper* pStateSet =
        static_cast< ::utl::AccessibleStateSetHelper*>(mxStateSet.get());
    if (pStateSet != NULL)
        return pStateSet->contains(aState);
    else
        
        return sal_False;
}




void AccessibleContextBase::SetRelationSet (
    const uno::Reference<XAccessibleRelationSet>& rxNewRelationSet)
    throw (::com::sun::star::uno::RuntimeException)
{
    OSL_TRACE ("setting relation set");

    
    
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






uno::Reference< XAccessibleContext> SAL_CALL
    AccessibleContextBase::getAccessibleContext (void)
    throw (uno::RuntimeException)
{
    ThrowIfDisposed ();
    return this;
}






/** No children.
*/
sal_Int32 SAL_CALL
       AccessibleContextBase::getAccessibleChildCount (void)
    throw (uno::RuntimeException)
{
    ThrowIfDisposed ();
    return 0;
}




/** Forward the request to the shape.  Return the requested shape or throw
    an exception for a wrong index.
*/
uno::Reference<XAccessible> SAL_CALL
    AccessibleContextBase::getAccessibleChild (sal_Int32 nIndex)
    throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
    throw lang::IndexOutOfBoundsException (
        "no child with index " + OUString(nIndex),
        NULL);
}




uno::Reference<XAccessible> SAL_CALL
       AccessibleContextBase::getAccessibleParent (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
    return mxParent;
}




sal_Int32 SAL_CALL
       AccessibleContextBase::getAccessibleIndexInParent (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
    

    
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
                    if (xChildContext == (XAccessibleContext*)this)
                        return i;
                }
            }
        }
   }

   
   
   return -1;
}




sal_Int16 SAL_CALL
    AccessibleContextBase::getAccessibleRole (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
    return maRole;
}




OUString SAL_CALL
       AccessibleContextBase::getAccessibleDescription (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();

    return msDescription;
}




OUString SAL_CALL
       AccessibleContextBase::getAccessibleName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();

    if (meNameOrigin == NotSet)
    {
        
        
        msName = CreateAccessibleName();
        meNameOrigin = AutomaticallyCreated;
    }

    return msName;
}




/** Return a copy of the relation set.
*/
uno::Reference<XAccessibleRelationSet> SAL_CALL
       AccessibleContextBase::getAccessibleRelationSet (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();

    
    ::utl::AccessibleRelationSetHelper* pRelationSet =
        static_cast< ::utl::AccessibleRelationSetHelper*>(mxRelationSet.get());
    if (pRelationSet != NULL)
    {
        return uno::Reference<XAccessibleRelationSet> (
            new ::utl::AccessibleRelationSetHelper (*pRelationSet));
    }
    else
        return uno::Reference<XAccessibleRelationSet>(NULL);
}




/** Return a copy of the state set.
    Possible states are:
        ENABLED
        SHOWING
        VISIBLE
*/
uno::Reference<XAccessibleStateSet> SAL_CALL
    AccessibleContextBase::getAccessibleStateSet (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ::utl::AccessibleStateSetHelper* pStateSet = NULL;

    if (rBHelper.bDisposed)
    {
        
        
        pStateSet = new ::utl::AccessibleStateSetHelper ();
        if (pStateSet != NULL)
            pStateSet->AddState (AccessibleStateType::DEFUNC);
    }
    else
    {
        
        pStateSet = static_cast< ::utl::AccessibleStateSetHelper*>(mxStateSet.get());

        
#if 0
        if (aState == AccessibleStateType::FOCUSED
            && pStateSet != NULL
            && mpText != NULL)
        {
            if (mpText->GetFocusedState ())
                pStateSet->AddState (aState);
            else
                pStateSet->RemoveState (aState);
        }
#endif
        if (pStateSet != NULL)
            pStateSet = new ::utl::AccessibleStateSetHelper (*pStateSet);
    }

    return uno::Reference<XAccessibleStateSet>(pStateSet);
}




lang::Locale SAL_CALL
       AccessibleContextBase::getLocale (void)
    throw (IllegalAccessibleComponentStateException,
        ::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
    
    if (mxParent.is())
    {
        uno::Reference<XAccessibleContext> xParentContext (
            mxParent->getAccessibleContext());
        if (xParentContext.is())
            return xParentContext->getLocale ();
    }

    
    
    throw IllegalAccessibleComponentStateException ();
}






void SAL_CALL AccessibleContextBase::addAccessibleEventListener (
        const uno::Reference<XAccessibleEventListener >& rxListener)
    throw (uno::RuntimeException)
{
    if (rxListener.is())
    {
        if (rBHelper.bDisposed || rBHelper.bInDispose)
        {
            uno::Reference<uno::XInterface> x ((lang::XComponent *)this, uno::UNO_QUERY);
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
    throw (uno::RuntimeException)
{
    ThrowIfDisposed ();
    if (rxListener.is())
    {
        sal_Int32 nListenerCount = comphelper::AccessibleEventNotifier::removeEventListener( mnClientId, rxListener );
        if ( !nListenerCount )
        {
            
            
            
            
            comphelper::AccessibleEventNotifier::revokeClient( mnClientId );
            mnClientId = 0;
        }
    }
}






OUString SAL_CALL AccessibleContextBase::getImplementationName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
    return OUString("AccessibleContextBase");
}




sal_Bool SAL_CALL
     AccessibleContextBase::supportsService (const OUString& sServiceName)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
    
    
    uno::Sequence< OUString > aSupportedServices (
        getSupportedServiceNames ());
    for (int i=0; i<aSupportedServices.getLength(); i++)
        if (sServiceName == aSupportedServices[i])
            return sal_True;
    return sal_False;
}




uno::Sequence< OUString > SAL_CALL
       AccessibleContextBase::getSupportedServiceNames (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
    static const OUString sServiceNames[2] = {
            OUString("com.sun.star.accessibility.Accessible"),
            OUString("com.sun.star.accessibility.AccessibleContext")
    };
    return uno::Sequence<OUString> (sServiceNames, 2);
}






uno::Sequence< ::com::sun::star::uno::Type>
    AccessibleContextBase::getTypes (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();

    
    
    return BaseClass::getTypes();
}

namespace
{
    class theAccessibleContextBaseImplementationId : public rtl::Static< UnoTunnelIdInit, theAccessibleContextBaseImplementationId > {};
}

uno::Sequence<sal_Int8> SAL_CALL
    AccessibleContextBase::getImplementationId (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return theAccessibleContextBaseImplementationId::get().getSeq();
}



void SAL_CALL AccessibleContextBase::disposing (void)
{
    SetState (AccessibleStateType::DEFUNC);

    ::osl::MutexGuard aGuard (maMutex);

    
    if ( mnClientId )
    {
        comphelper::AccessibleEventNotifier::revokeClientNotifyDisposing( mnClientId, *this );
        mnClientId =  0;
    }
}




void AccessibleContextBase::SetAccessibleDescription (
    const OUString& rDescription,
    StringOrigin eDescriptionOrigin)
    throw (uno::RuntimeException)
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
    throw (uno::RuntimeException)
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




OUString AccessibleContextBase::CreateAccessibleDescription (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return OUString("Empty Description");
}




OUString AccessibleContextBase::CreateAccessibleName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return OUString("Empty Name");
}




void AccessibleContextBase::CommitChange (
    sal_Int16 nEventId,
    const uno::Any& rNewValue,
    const uno::Any& rOldValue)
{
    
    
    
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




void AccessibleContextBase::ThrowIfDisposed (void)
    throw (::com::sun::star::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        OSL_TRACE ("Calling disposed object. Throwing exception:");
        throw lang::DisposedException ("object has been already disposed",
            static_cast<uno::XWeak*>(this));
    }
}



sal_Bool AccessibleContextBase::IsDisposed (void)
{
    return (rBHelper.bDisposed || rBHelper.bInDispose);
}



void AccessibleContextBase::SetAccessibleRole( sal_Int16 _nRole )
{
    maRole = _nRole;
}


} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
