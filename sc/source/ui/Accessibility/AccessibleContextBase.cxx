/*************************************************************************
 *
 *  $RCSfile: AccessibleContextBase.cxx,v $
 *
 *  $Revision: 1.27 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 18:05:42 $
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


#ifndef _SC_ACCESSIBLECONTEXTBASE_HXX
#include "AccessibleContextBase.hxx"
#endif
#ifndef SC_UNOGUARD_HXX
#include "unoguard.hxx"
#endif

#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleEventId.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLESTATETYPE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleStateType.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYCHANGEEVENT_HPP_
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#endif

#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _TOOLKIT_HELPER_CONVERT_HXX_
#include <toolkit/helper/convert.hxx>
#endif
#ifndef _SFXSMPLHINT_HXX
#include <svtools/smplhint.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _UTL_ACCESSIBLERELATIONSETHELPER_HXX_
#include <unotools/accessiblerelationsethelper.hxx>
#endif
#ifndef _VCL_UNOHELP_HXX
#include <vcl/unohelp.hxx>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif
#ifndef COMPHELPER_ACCESSIBLE_EVENT_NOTIFIER
#include <comphelper/accessibleeventnotifier.hxx>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::accessibility;

//=====  internal  ============================================================

DBG_NAME(ScAccessibleContextBase)

ScAccessibleContextBase::ScAccessibleContextBase(
                                                 const uno::Reference<XAccessible>& rxParent,
                                                 const sal_Int16 aRole)
                                                 :
    ScAccessibleContextBaseWeakImpl(m_aMutex),
    maRole(aRole),
    mxParent(rxParent),
    mnClientId(0)
{
    DBG_CTOR(ScAccessibleContextBase, NULL);
}


ScAccessibleContextBase::~ScAccessibleContextBase(void)
{
    DBG_DTOR(ScAccessibleContextBase, NULL);

    if (!IsDefunc() && !rBHelper.bInDispose)
    {
        // increment refcount to prevent double call off dtor
        osl_incrementInterlockedCount( &m_refCount );
        // call dispose to inform object wich have a weak reference to this object
        dispose();
    }
}

void ScAccessibleContextBase::Init()
{
    // hold reference to make sure that the destructor is not called
    uno::Reference< XAccessibleContext > xOwnContext(this);

    if (mxParent.is())
    {
        uno::Reference< XAccessibleEventBroadcaster > xBroadcaster (mxParent->getAccessibleContext(), uno::UNO_QUERY);
        if (xBroadcaster.is())
            xBroadcaster->addEventListener(this);
    }
    msName = createAccessibleName();
    msDescription = createAccessibleDescription();
}

void SAL_CALL ScAccessibleContextBase::disposing()
{
    ScUnoGuard aGuard;
//  CommitDefunc(); not necessary and should not be send, because it cost a lot of time

    // hold reference to make sure that the destructor is not called
    uno::Reference< XAccessibleContext > xOwnContext(this);

    if ( mnClientId )
    {
        sal_Int32 nTemClientId(mnClientId);
        mnClientId =  0;
        comphelper::AccessibleEventNotifier::revokeClientNotifyDisposing( nTemClientId, *this );
    }

    if (mxParent.is())
    {
        uno::Reference< XAccessibleEventBroadcaster > xBroadcaster (mxParent->getAccessibleContext(), uno::UNO_QUERY);
        if (xBroadcaster.is())
            xBroadcaster->removeEventListener(this);
        mxParent = NULL;
    }

    ScAccessibleContextBaseWeakImpl::disposing();
}

//=====  XInterface  =====================================================

uno::Any SAL_CALL ScAccessibleContextBase::queryInterface( uno::Type const & rType )
    throw (uno::RuntimeException)
{
    uno::Any aAny (ScAccessibleContextBaseWeakImpl::queryInterface(rType));
    return aAny.hasValue() ? aAny : ScAccessibleContextBaseImplEvent::queryInterface(rType);
}

void SAL_CALL ScAccessibleContextBase::acquire()
    throw ()
{
    ScAccessibleContextBaseWeakImpl::acquire();
}

void SAL_CALL ScAccessibleContextBase::release()
    throw ()
{
    ScAccessibleContextBaseWeakImpl::release();
}

//=====  SfxListener  =====================================================

void ScAccessibleContextBase::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if (rHint.ISA( SfxSimpleHint ) )
    {
        const SfxSimpleHint& rRef = (const SfxSimpleHint&)rHint;
        if (rRef.GetId() == SFX_HINT_DYING)
        {
            // it seems the Broadcaster is dying, since the view is dying
            dispose();
        }
    }
}

//=====  XAccessible  =========================================================

uno::Reference< XAccessibleContext> SAL_CALL
    ScAccessibleContextBase::getAccessibleContext(void)
    throw (uno::RuntimeException)
{
    return this;
}

//=====  XAccessibleComponent  ================================================

sal_Bool SAL_CALL ScAccessibleContextBase::contains(const awt::Point& rPoint )
        throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
    return Rectangle (Point(), GetBoundingBox().GetSize()).IsInside(VCLPoint(rPoint));
}

uno::Reference< XAccessible > SAL_CALL ScAccessibleContextBase::getAccessibleAt(
        const awt::Point& rPoint )
        throw (uno::RuntimeException)
{
    DBG_ERROR("not implemented");
    return uno::Reference<XAccessible>();
}

awt::Rectangle SAL_CALL ScAccessibleContextBase::getBounds(  )
        throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
    return AWTRectangle(GetBoundingBox());
}

awt::Point SAL_CALL ScAccessibleContextBase::getLocation(  )
        throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
    return AWTPoint(GetBoundingBox().TopLeft());
}

awt::Point SAL_CALL ScAccessibleContextBase::getLocationOnScreen(  )
        throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
    return AWTPoint(GetBoundingBoxOnScreen().TopLeft());
}

awt::Size SAL_CALL ScAccessibleContextBase::getSize(  )
        throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
    return AWTSize(GetBoundingBox().GetSize());
}

sal_Bool SAL_CALL ScAccessibleContextBase::isShowing(  )
        throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
    sal_Bool bShowing(sal_False);
    if (mxParent.is())
    {
        uno::Reference<XAccessibleComponent> xParentComponent (mxParent->getAccessibleContext(), uno::UNO_QUERY);
        if (xParentComponent.is())
        {
            Rectangle aParentBounds(VCLRectangle(xParentComponent->getBounds()));
            Rectangle aBounds(VCLRectangle(getBounds()));
            bShowing = aBounds.IsOver(aParentBounds);
        }
    }
    return bShowing;
}

sal_Bool SAL_CALL ScAccessibleContextBase::isVisible(  )
        throw (uno::RuntimeException)
{
    return sal_True;
}

void SAL_CALL ScAccessibleContextBase::grabFocus(  )
        throw (uno::RuntimeException)
{
    DBG_ERROR("not implemented");
}

sal_Int32 SAL_CALL ScAccessibleContextBase::getForeground(  )
        throw (uno::RuntimeException)
{
    return COL_BLACK;
}

sal_Int32 SAL_CALL ScAccessibleContextBase::getBackground(  )
        throw (uno::RuntimeException)
{
    return COL_WHITE;
}

//=====  XAccessibleContext  ==================================================

sal_Int32 SAL_CALL
       ScAccessibleContextBase::getAccessibleChildCount(void)
    throw (uno::RuntimeException)
{
    DBG_ERROR("should be implemented in the abrevated class");
    return 0;
}

uno::Reference<XAccessible> SAL_CALL
    ScAccessibleContextBase::getAccessibleChild(sal_Int32 nIndex)
        throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    DBG_ERROR("should be implemented in the abrevated class");
    return uno::Reference<XAccessible>();
}

uno::Reference<XAccessible> SAL_CALL
       ScAccessibleContextBase::getAccessibleParent(void)
    throw (uno::RuntimeException)
{
    return mxParent;
}

sal_Int32 SAL_CALL
       ScAccessibleContextBase::getAccessibleIndexInParent(void)
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
    //  Use a simple but slow solution for now.  Optimize later.
   //   Return -1 to indicate that this object's parent does not know about the
   //   object.
    sal_Int32 nIndex(-1);

    //  Iterate over all the parent's children and search for this object.
    if (mxParent.is())
    {
        uno::Reference<XAccessibleContext> xParentContext (
            mxParent->getAccessibleContext());
        if (xParentContext.is())
        {
            sal_Int32 nChildCount = xParentContext->getAccessibleChildCount();
            for (sal_Int32 i=0; i<nChildCount; ++i)
            {
                uno::Reference<XAccessible> xChild (xParentContext->getAccessibleChild (i));
                if (xChild.is())
                {
                    if (xChild.get() == this)
                        nIndex = i;
                }
            }
        }
   }

   return nIndex;
}

sal_Int16 SAL_CALL
    ScAccessibleContextBase::getAccessibleRole(void)
    throw (uno::RuntimeException)
{
    return maRole;
}

::rtl::OUString SAL_CALL
       ScAccessibleContextBase::getAccessibleDescription(void)
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
    if (!msDescription.getLength())
    {
        OUString sDescription(createAccessibleDescription());
//      DBG_ASSERT(sDescription.getLength(), "We should give always a descripition.");

        if (msDescription != sDescription)
        {
            AccessibleEventObject aEvent;
            aEvent.EventId = AccessibleEventId::ACCESSIBLE_DESCRIPTION_EVENT;
            aEvent.Source = uno::Reference< XAccessible >(this);
            aEvent.OldValue <<= msDescription;
            aEvent.NewValue <<= sDescription;

            msDescription = sDescription;

            CommitChange(aEvent);
        }
    }
    return msDescription;
}

OUString SAL_CALL
       ScAccessibleContextBase::getAccessibleName(void)
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
    if (!msName.getLength())
    {
        OUString sName(createAccessibleName());
        DBG_ASSERT(sName.getLength(), "We should give always a name.");

        if (msName != sName)
        {
            AccessibleEventObject aEvent;
            aEvent.EventId = AccessibleEventId::ACCESSIBLE_NAME_EVENT;
            aEvent.Source = uno::Reference< XAccessible >(this);
            aEvent.OldValue <<= msName;
            aEvent.NewValue <<= sName;

            msName = sName;

            CommitChange(aEvent);
        }
    }
    return msName;
}

uno::Reference<XAccessibleRelationSet> SAL_CALL
       ScAccessibleContextBase::getAccessibleRelationSet(void)
    throw (uno::RuntimeException)
{
    return new utl::AccessibleRelationSetHelper();
}

uno::Reference<XAccessibleStateSet> SAL_CALL
        ScAccessibleContextBase::getAccessibleStateSet(void)
    throw (uno::RuntimeException)
{
    return uno::Reference<XAccessibleStateSet>();
}

lang::Locale SAL_CALL
       ScAccessibleContextBase::getLocale(void)
    throw (IllegalAccessibleComponentStateException,
        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
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

    //=====  XAccessibleEventBroadcaster  =====================================

void SAL_CALL
       ScAccessibleContextBase::addEventListener(
           const uno::Reference<XAccessibleEventListener>& xListener)
    throw (uno::RuntimeException)
{
    if (xListener.is())
    {
        ScUnoGuard aGuard;
        IsObjectValid();
        if (!IsDefunc())
        {
            if (!mnClientId)
                mnClientId = comphelper::AccessibleEventNotifier::registerClient( );
            comphelper::AccessibleEventNotifier::addEventListener( mnClientId, xListener );
        }
    }
}

void SAL_CALL
       ScAccessibleContextBase::removeEventListener(
        const uno::Reference<XAccessibleEventListener>& xListener)
    throw (uno::RuntimeException)
{
    if (xListener.is())
    {
        ScUnoGuard aGuard;
        if (!IsDefunc() && mnClientId)
        {
            sal_Int32 nListenerCount = comphelper::AccessibleEventNotifier::removeEventListener( mnClientId, xListener );
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
}

    //=====  XAccessibleEventListener  ========================================

void SAL_CALL ScAccessibleContextBase::disposing(
    const lang::EventObject& rSource )
        throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (rSource.Source == mxParent)
        dispose();
}

void SAL_CALL ScAccessibleContextBase::notifyEvent(
        const AccessibleEventObject& aEvent )
        throw (uno::RuntimeException)
{
}

//=====  XServiceInfo  ========================================================

::rtl::OUString SAL_CALL
       ScAccessibleContextBase::getImplementationName(void)
    throw (uno::RuntimeException)
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM ("ScAccessibleContextBase"));
}

sal_Bool SAL_CALL
     ScAccessibleContextBase::supportsService(const OUString& sServiceName)
    throw (uno::RuntimeException)
{
    //  Iterate over all supported service names and return true if on of them
    //  matches the given name.
    uno::Sequence< ::rtl::OUString> aSupportedServices (
        getSupportedServiceNames ());
    sal_Int32 nLength(aSupportedServices.getLength());
    const OUString* pServiceNames = aSupportedServices.getConstArray();
    for (int i=0; i<nLength; ++i, ++pServiceNames)
        if (sServiceName == *pServiceNames)
            return sal_True;
    return sal_False;
}

uno::Sequence< ::rtl::OUString> SAL_CALL
       ScAccessibleContextBase::getSupportedServiceNames(void)
    throw (uno::RuntimeException)
{
    uno::Sequence<OUString> aServiceNames(2);
    OUString* pServiceNames = aServiceNames.getArray();
    if (pServiceNames)
    {
        pServiceNames[0] = OUString(RTL_CONSTASCII_USTRINGPARAM ("drafts.com.sun.star.accessibility.Accessible"));
        pServiceNames[1] = OUString(RTL_CONSTASCII_USTRINGPARAM ("drafts.com.sun.star.accessibility.AccessibleContext"));
    }

    return aServiceNames;
}

//=====  XTypeProvider  =======================================================

uno::Sequence< uno::Type > SAL_CALL ScAccessibleContextBase::getTypes()
        throw (uno::RuntimeException)
{
    return comphelper::concatSequences(ScAccessibleContextBaseWeakImpl::getTypes(), ScAccessibleContextBaseImplEvent::getTypes());
}

uno::Sequence<sal_Int8> SAL_CALL
    ScAccessibleContextBase::getImplementationId(void)
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
    static uno::Sequence<sal_Int8> aId;
    if (aId.getLength() == 0)
    {
        aId.realloc (16);
        rtl_createUuid (reinterpret_cast<sal_uInt8 *>(aId.getArray()), 0, sal_True);
    }
    return aId;
}

//=====  internal  ============================================================

::rtl::OUString SAL_CALL
    ScAccessibleContextBase::createAccessibleDescription(void)
    throw (uno::RuntimeException)
{
    DBG_ERROR("should be implemented in the abrevated class");
    return rtl::OUString();
}

::rtl::OUString SAL_CALL
    ScAccessibleContextBase::createAccessibleName(void)
    throw (uno::RuntimeException)
{
    DBG_ERROR("should be implemented in the abrevated class");
    return rtl::OUString();
}

void ScAccessibleContextBase::CommitChange(const AccessibleEventObject& rEvent) const
{
    if (mnClientId)
        comphelper::AccessibleEventNotifier::addEvent( mnClientId, rEvent );
}

void ScAccessibleContextBase::ChangeName()
{
    AccessibleEventObject aEvent;
    aEvent.EventId = AccessibleEventId::ACCESSIBLE_NAME_EVENT;
    aEvent.Source = uno::Reference< XAccessible >(const_cast<ScAccessibleContextBase*>(this));
    aEvent.OldValue <<= msName;

    msName = rtl::OUString(); // reset the name so it will be hold again
    getAccessibleName(); // create the new name

    aEvent.NewValue <<= msName;

    CommitChange(aEvent);
}

void ScAccessibleContextBase::CommitDefunc() const
{
    AccessibleEventObject aEvent;
    aEvent.EventId = AccessibleEventId::ACCESSIBLE_STATE_EVENT;
    aEvent.Source = uno::Reference< XAccessible >(const_cast<ScAccessibleContextBase*>(this));
    aEvent.NewValue <<= AccessibleStateType::DEFUNC;

    CommitChange(aEvent);
}

void ScAccessibleContextBase::CommitFocusGained() const
{
    AccessibleEventObject aEvent;
    aEvent.EventId = AccessibleEventId::ACCESSIBLE_STATE_EVENT;
    aEvent.Source = uno::Reference< XAccessible >(const_cast<ScAccessibleContextBase*>(this));
    aEvent.NewValue <<= AccessibleStateType::FOCUSED;

    CommitChange(aEvent);

    ::vcl::unohelper::NotifyAccessibleStateEventGlobally(aEvent);
}

void ScAccessibleContextBase::CommitFocusLost() const
{
    AccessibleEventObject aEvent;
    aEvent.EventId = AccessibleEventId::ACCESSIBLE_STATE_EVENT;
    aEvent.Source = uno::Reference< XAccessible >(const_cast<ScAccessibleContextBase*>(this));
    aEvent.OldValue <<= AccessibleStateType::FOCUSED;

    CommitChange(aEvent);

    vcl::unohelper::NotifyAccessibleStateEventGlobally(aEvent);
}

Rectangle ScAccessibleContextBase::GetBoundingBoxOnScreen(void) const
        throw (uno::RuntimeException)
{
    DBG_ERROR("not implemented");
    return Rectangle();
}

Rectangle ScAccessibleContextBase::GetBoundingBox(void) const
        throw (uno::RuntimeException)
{
    DBG_ERROR("not implemented");
    return Rectangle();
}

void ScAccessibleContextBase::IsObjectValid() const
        throw (lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
        throw lang::DisposedException();
}
