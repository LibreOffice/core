/*************************************************************************
 *
 *  $RCSfile: AccessibleContextBase.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 16:52:52 $
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


#include "AccessibleContextBase.hxx"

#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif


#ifndef _COM_SUN_STAR_BEANS_PROPERTYCHANGEEVENT_HPP_
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEEVENTLISTENER_HPP_
#include <com/sun/star/accessibility/XAccessibleEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLERELATIONTYPE_HPP_
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#endif

#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX_
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _UTL_ACCESSIBLERELATIONSETHELPER_HXX_
#include <unotools/accessiblerelationsethelper.hxx>
#endif
#ifndef COMPHELPER_ACCESSIBLE_EVENT_NOTIFIER
#include <comphelper/accessibleeventnotifier.hxx>
#endif

#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif

#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>

#include <utility>

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using ::com::sun::star::uno::Reference;

namespace accessibility {

//=====  internal  ============================================================

// Define a shortcut for the somewhot longish base class name.
typedef ::cppu::WeakComponentImplHelper4<
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
        maRole(aRole),
        mnClientId(0)
{
    // Create the state set.
    ::utl::AccessibleStateSetHelper* pStateSet  = new ::utl::AccessibleStateSetHelper ();
    mxStateSet = pStateSet;

    // Set some states.  Don't use the SetState method because no events
    // shall be broadcastet (that is not yet initialized anyway).
    if (pStateSet != NULL)
    {
        pStateSet->AddState (AccessibleStateType::ENABLED);
        pStateSet->AddState (AccessibleStateType::SHOWING);
        pStateSet->AddState (AccessibleStateType::VISIBLE);
        pStateSet->AddState (AccessibleStateType::FOCUSABLE);
        pStateSet->AddState (AccessibleStateType::SELECTABLE);
    }

    // Create the relation set.
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
        // Clear the mutex guard so that it is not locked during calls to listeners.
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
        // If there is no state set then return false as a default value.
        return sal_False;
}




void AccessibleContextBase::SetRelationSet (
    const uno::Reference<XAccessibleRelationSet>& rxNewRelationSet)
    throw (::com::sun::star::uno::RuntimeException)
{
    OSL_TRACE ("setting relation set");

    // Try to emit some meaningfull events indicating differing relations in
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




//=====  XAccessible  =========================================================

uno::Reference< XAccessibleContext> SAL_CALL
    AccessibleContextBase::getAccessibleContext (void)
    throw (uno::RuntimeException)
{
    ThrowIfDisposed ();
    return this;
}




//=====  XAccessibleContext  ==================================================

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
    AccessibleContextBase::getAccessibleChild (long nIndex)
    throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
    throw lang::IndexOutOfBoundsException (
        ::rtl::OUString::createFromAscii ("no child with index " + nIndex),
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
                    if (xChildContext == (XAccessibleContext*)this)
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
    AccessibleContextBase::getAccessibleRole (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
    return maRole;
}




::rtl::OUString SAL_CALL
       AccessibleContextBase::getAccessibleDescription (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
    if (msDescription.getLength() == 0)
        // Do not send an event because this is the first time it has been
        // requested.
        msDescription = CreateAccessibleDescription();
    return msDescription;
}




OUString SAL_CALL
       AccessibleContextBase::getAccessibleName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
    if (msName.getLength() == 0)
        // Do not send an event because this is the first time it has been
        // requested.
        msName = CreateAccessibleName();
    return msName;
}




/** Return a copy of the relation set.
*/
uno::Reference<XAccessibleRelationSet> SAL_CALL
       AccessibleContextBase::getAccessibleRelationSet (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();

    // Create a copy of the relation set and return it.
    ::utl::AccessibleRelationSetHelper* pRelationSet =
        static_cast< ::utl::AccessibleRelationSetHelper*>(mxRelationSet.get());
    if (pRelationSet != NULL)
    {
        return uno::Reference<XAccessibleRelationSet> (
            new ::utl::AccessibleRelationSetHelper (*pRelationSet));
    }
    else
        return uno::Reference<XAccessibleRelationSet>(NULL);
    return uno::Reference<XAccessibleRelationSet>();
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
        // We are already disposed!
        // Create a new state set that has only set the DEFUNC state.
        pStateSet = new ::utl::AccessibleStateSetHelper ();
        if (pStateSet != NULL)
            pStateSet->AddState (AccessibleStateType::DEFUNC);
    }
    else
    {
        // Create a copy of the state set and return it.
        pStateSet = static_cast< ::utl::AccessibleStateSetHelper*>(mxStateSet.get());

        // Merge current focused state from edit engine.
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




//=====  XAccessibleEventListener  ============================================

void SAL_CALL
    AccessibleContextBase::addEventListener (
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




void SAL_CALL
    AccessibleContextBase::removeEventListener (
        const uno::Reference<XAccessibleEventListener >& rxListener )
    throw (uno::RuntimeException)
{
    ThrowIfDisposed ();
    if (rxListener.is())
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




//=====  XServiceInfo  ========================================================

::rtl::OUString SAL_CALL
       AccessibleContextBase::getImplementationName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
    return OUString(RTL_CONSTASCII_USTRINGPARAM("AccessibleContextBase"));
}




sal_Bool SAL_CALL
     AccessibleContextBase::supportsService (const OUString& sServiceName)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
    //  Iterate over all supported service names and return true if on of them
    //  matches the given name.
    uno::Sequence< ::rtl::OUString> aSupportedServices (
        getSupportedServiceNames ());
    for (int i=0; i<aSupportedServices.getLength(); i++)
        if (sServiceName == aSupportedServices[i])
            return sal_True;
    return sal_False;
}




uno::Sequence< ::rtl::OUString> SAL_CALL
       AccessibleContextBase::getSupportedServiceNames (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
    static const OUString sServiceNames[2] = {
        OUString(RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.accessibility.Accessible")),
        OUString(RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.accessibility.AccessibleContext"))
    };
    return uno::Sequence<OUString> (sServiceNames, 2);
}




//=====  XTypeProvider  =======================================================

uno::Sequence< ::com::sun::star::uno::Type>
    AccessibleContextBase::getTypes (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();

    // This class supports no interfaces on its own.  Just return those
    // supported by the base class.
    return BaseClass::getTypes();
}




uno::Sequence<sal_Int8> SAL_CALL
    AccessibleContextBase::getImplementationId (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
    static uno::Sequence<sal_Int8> aId;
    if (aId.getLength() == 0)
    {
        ::osl::MutexGuard aGuard (maMutex);
        aId.realloc (16);
        rtl_createUuid ((sal_uInt8 *)aId.getArray(), 0, sal_True);
    }
    return aId;
}




//=====  internal  ============================================================

void SAL_CALL AccessibleContextBase::disposing (void)
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




void AccessibleContextBase::SetAccessibleDescription (const ::rtl::OUString& rDescription)
    throw (uno::RuntimeException)
{
    if (msDescription != rDescription)
    {
        uno::Any aOldValue, aNewValue;
        aOldValue <<= msDescription;
        aNewValue <<= rDescription;

        msDescription = rDescription;

        CommitChange(
            AccessibleEventId::DESCRIPTION_CHANGED,
            aNewValue,
            aOldValue);
    }
}




void AccessibleContextBase::SetAccessibleName (const ::rtl::OUString& rName)
    throw (uno::RuntimeException)
{
    if (msName != rName)
    {
        uno::Any aOldValue, aNewValue;
        aOldValue <<= msName;
        aNewValue <<= rName;

        msName = rName;

        CommitChange(
            AccessibleEventId::NAME_CHANGED,
            aNewValue,
            aOldValue);
    }
}




::rtl::OUString AccessibleContextBase::CreateAccessibleDescription (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii ("Empty Description");
}




::rtl::OUString AccessibleContextBase::CreateAccessibleName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii ("Empty Name");
}




void AccessibleContextBase::CommitChange (
    sal_Int16 nEventId,
    const uno::Any& rNewValue,
    const uno::Any& rOldValue)
{
    AccessibleEventObject aEvent (
        static_cast<XAccessibleContext*>(this),
        nEventId,
        rNewValue,
        rOldValue);

    FireEvent (aEvent);
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
        throw lang::DisposedException (
            OUString(RTL_CONSTASCII_USTRINGPARAM("object has been already disposed")),
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


} // end of namespace accessibility
