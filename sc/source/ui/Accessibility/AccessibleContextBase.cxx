/*************************************************************************
 *
 *  $RCSfile: AccessibleContextBase.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: sab $ $Date: 2002-02-25 11:46:49 $
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

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::accessibility;

//=====  internal  ============================================================

ScAccessibleContextBase::ScAccessibleContextBase(
        const uno::Reference<XAccessible>& rxParent,
        const sal_Int16 aRole)
    :
    maRole(aRole),
    mxParent(rxParent),
    mpEventListeners(NULL),
    mpFocusListeners(NULL)
{
    if (rxParent.is())
    {
        uno::Reference< XAccessibleEventBroadcaster > xBroadcaster (rxParent->getAccessibleContext(), uno::UNO_QUERY);
        if (xBroadcaster.is())
            xBroadcaster->addEventListener(this);
    }
}


ScAccessibleContextBase::~ScAccessibleContextBase(void)
{
    if (mpEventListeners || mpFocusListeners)
    {
        lang::EventObject aEvent;
        aEvent.Source = static_cast<cppu::OWeakObject*>(this);
        if (mpEventListeners)
        {
            mpEventListeners->disposeAndClear(aEvent);
            delete mpEventListeners;
        }
        if (mpFocusListeners)
        {
            mpFocusListeners->disposeAndClear(aEvent);
            delete mpFocusListeners;
        }
    }
    if (mxParent.is())
    {
        uno::Reference< XAccessibleEventBroadcaster > xBroadcaster (mxParent->getAccessibleContext(), uno::UNO_QUERY);
        if (xBroadcaster.is())
            xBroadcaster->removeEventListener(this);
    }
}

void ScAccessibleContextBase::SetDefunc()
{
    CommitDefunc();

    if (mxParent.is())
    {
        uno::Reference< XAccessibleEventBroadcaster > xBroadcaster (mxParent->getAccessibleContext(), uno::UNO_QUERY);
        if (xBroadcaster.is())
            xBroadcaster->removeEventListener(this);
        mxParent = NULL;
    }
}

//=====  SfxListener  =====================================================

void ScAccessibleContextBase::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
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
    ScUnoGuard();
    Rectangle aBounds(GetBoundingBox());
    return !((rPoint.X < aBounds.getX()) || (rPoint.X > (aBounds.getX() + aBounds.getWidth())) ||
            (rPoint.Y < aBounds.getY()) || (rPoint.Y > (aBounds.getY() + aBounds.getHeight())));
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
    ScUnoGuard();
    Rectangle aCoreBounds(GetBoundingBox());
    awt::Rectangle aBounds;
    aBounds.X = aCoreBounds.getX();
    aBounds.Y = aCoreBounds.getY();
    aBounds.Width = aCoreBounds.getWidth();
    aBounds.Height = aCoreBounds.getHeight();
    return aBounds;
}

awt::Point SAL_CALL ScAccessibleContextBase::getLocation(  )
        throw (uno::RuntimeException)
{
    ScUnoGuard();
    awt::Point aLocation;
    Rectangle aRect(GetBoundingBox());
    aLocation.X = aRect.getX();
    aLocation.Y = aRect.getY();
    return aLocation;
}

awt::Point SAL_CALL ScAccessibleContextBase::getLocationOnScreen(  )
        throw (uno::RuntimeException)
{
    ScUnoGuard();
    awt::Point aPoint;
    Rectangle aRect(GetBoundingBoxOnScreen());
    aPoint.X = aRect.getX();
    aPoint.Y = aRect.getY();
    return aPoint;
}

awt::Size SAL_CALL ScAccessibleContextBase::getSize(  )
        throw (uno::RuntimeException)
{
    ScUnoGuard();
    awt::Size aSize;
    Rectangle aRect(GetBoundingBox());
    aSize.Width = aRect.getWidth();
    aSize.Height = aRect.getHeight();
    return aSize;
}

sal_Bool SAL_CALL ScAccessibleContextBase::isShowing(  )
        throw (uno::RuntimeException)
{
    ScUnoGuard();
    sal_Bool bShowing(sal_False);
    uno::Reference<XAccessibleComponent> xParentComponent (mxParent->getAccessibleContext(), uno::UNO_QUERY);
    if (xParentComponent.is())
    {
        awt::Rectangle aParentBounds(xParentComponent->getBounds());
        awt::Rectangle aBounds(getBounds());
        Rectangle aCoreParentBounds(aParentBounds.X, aParentBounds.Y,
            aParentBounds.X + aParentBounds.Width, aParentBounds.Y + aParentBounds.Height);
        Rectangle aCoreBounds(aBounds.X, aBounds.Y,
            aBounds.X + aBounds.Width, aBounds.Y + aBounds.Height);
        bShowing = aCoreBounds.IsOver(aCoreParentBounds);
    }
    return bShowing;
}

sal_Bool SAL_CALL ScAccessibleContextBase::isVisible(  )
        throw (uno::RuntimeException)
{
    return sal_True;
}

sal_Bool SAL_CALL ScAccessibleContextBase::isFocusTraversable(  )
        throw (uno::RuntimeException)
{
    return sal_False;
}

void SAL_CALL ScAccessibleContextBase::addFocusListener(
    const uno::Reference< awt::XFocusListener >& xListener )
        throw (uno::RuntimeException)
{
    ScUnoGuard();
    if (xListener.is())
    {
        if (!mpFocusListeners)
            mpFocusListeners = new cppu::OInterfaceContainerHelper(maListenerMutex);
        mpFocusListeners->addInterface(xListener);
    }
}

void SAL_CALL ScAccessibleContextBase::removeFocusListener(
    const uno::Reference< awt::XFocusListener >& xListener )
        throw (uno::RuntimeException)
{
    ScUnoGuard();
    if (xListener.is() && mpFocusListeners)
        mpFocusListeners->removeInterface(xListener);
}

void SAL_CALL ScAccessibleContextBase::grabFocus(  )
        throw (uno::RuntimeException)
{
    DBG_ERROR("not implemented");
}

uno::Any SAL_CALL ScAccessibleContextBase::getAccessibleKeyBinding(  )
        throw (uno::RuntimeException)
{
    // here is no implementation, because here are no KeyBindings for every object
    return uno::Any();
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
    ScUnoGuard();
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
            for (sal_Int32 i=0; i<nChildCount; i++)
            {
                uno::Reference<XAccessible> xChild (xParentContext->getAccessibleChild (i));
                if (xChild.is())
                {
                    uno::Reference<XAccessibleContext> xChildContext = xChild->getAccessibleContext();
                    if (xChildContext == (XAccessibleContext*)this)
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
    ScUnoGuard();
    if (!msDescription.getLength())
    {
        OUString sDescription(createAccessibleDescription());
        DBG_ASSERT(sDescription.getLength(), "We should give always a descripition.");

        AccessibleEventObject aEvent;
        aEvent.EventId = AccessibleEventId::ACCESSIBLE_DESCRIPTION_EVENT;
        aEvent.OldValue <<= msDescription;
        aEvent.NewValue <<= sDescription;

        msDescription = sDescription;

        CommitChange(aEvent);
    }
    return msDescription;
}

OUString SAL_CALL
       ScAccessibleContextBase::getAccessibleName(void)
    throw (uno::RuntimeException)
{
    ScUnoGuard();
    if (!msName.getLength())
    {
        OUString sName(createAccessibleName());
        DBG_ASSERT(sName.getLength(), "We should give always a name.");

        AccessibleEventObject aEvent;
        aEvent.EventId = AccessibleEventId::ACCESSIBLE_NAME_EVENT;
        aEvent.OldValue <<= msName;
        aEvent.NewValue <<= sName;

        msName = sName;

        CommitChange(aEvent);
    }
    return msName;
}

uno::Reference<XAccessibleRelationSet> SAL_CALL
       ScAccessibleContextBase::getAccessibleRelationSet(void)
    throw (uno::RuntimeException)
{
    return uno::Reference<XAccessibleRelationSet>();
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
    ScUnoGuard();
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
    ScUnoGuard();
    if (xListener.is())
    {
        if (!mpEventListeners)
            mpEventListeners = new cppu::OInterfaceContainerHelper(maListenerMutex);
        mpEventListeners->addInterface(xListener);
    }
}

void SAL_CALL
       ScAccessibleContextBase::removeEventListener(
        const uno::Reference<XAccessibleEventListener>& xListener)
    throw (uno::RuntimeException)
{
    ScUnoGuard();
    if (xListener.is() && mpEventListeners)
        mpEventListeners->removeInterface(xListener);
}

    //=====  XAccessibleEventListener  ========================================

void SAL_CALL ScAccessibleContextBase::disposing(
    const lang::EventObject& rSource )
        throw (uno::RuntimeException)
{
    if (rSource.Source == mxParent)
        SetDefunc();
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
    for (int i=0; i<aSupportedServices.getLength(); i++)
        if (sServiceName == aSupportedServices[i])
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

uno::Sequence< ::com::sun::star::uno::Type>
    ScAccessibleContextBase::getTypes(void)
    throw (uno::RuntimeException)
{
    const ::com::sun::star::uno::Type aTypeList[] = {
        ::getCppuType((const uno::Reference<
            XAccessible>*)0),
        ::getCppuType((const uno::Reference<
            XAccessibleComponent>*)0),
        ::getCppuType((const uno::Reference<
            XAccessibleContext>*)0),
        ::getCppuType((const uno::Reference<
            XAccessibleEventBroadcaster>*)0),
        ::getCppuType((const uno::Reference<
            XAccessibleEventListener>*)0),
        ::getCppuType((const uno::Reference<
            lang::XServiceInfo>*)0),
        ::getCppuType((const uno::Reference<
            lang::XTypeProvider>*)0)
        };
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type>
        aTypeSequence (aTypeList, 6);
    return aTypeSequence;
}

uno::Sequence<sal_Int8> SAL_CALL
    ScAccessibleContextBase::getImplementationId(void)
    throw (uno::RuntimeException)
{
    static uno::Sequence<sal_Int8> aId;
    if (aId.getLength() == 0)
    {
        ScUnoGuard();
        aId.realloc (16);
        rtl_createUuid ((sal_uInt8 *)aId.getArray(), 0, sal_True);
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

void ScAccessibleContextBase::CommitChange(const AccessibleEventObject& rEvent)
{
    if (mpEventListeners)
    {
        //  Call all listeners.
        uno::Sequence< uno::Reference< uno::XInterface > > aListeners = mpEventListeners->getElements();
        sal_uInt32 nLength(aListeners.getLength());
        if (nLength)
        {
            const uno::Reference< uno::XInterface >* pInterfaces = aListeners.getConstArray();
            if (pInterfaces)
            {
                for (sal_uInt32 i = 0; i < nLength; i++)
                {
                    uno::Reference<XAccessibleEventListener> xListener(pInterfaces[i], uno::UNO_QUERY);
                    if (xListener.is())
                        xListener->notifyEvent(rEvent);
                }
            }
        }
    }
}

void ScAccessibleContextBase::CommitDefunc()
{
    utl::AccessibleStateSetHelper* pStateSet = new utl::AccessibleStateSetHelper();
    pStateSet->AddState(AccessibleStateType::DEFUNC);
    uno::Reference<XAccessibleStateSet> xStateSet (pStateSet);

    AccessibleEventObject aEvent;
    aEvent.EventId = AccessibleEventId::ACCESSIBLE_STATE_EVENT;
    aEvent.NewValue <<= xStateSet;

    CommitChange(aEvent);
}

void ScAccessibleContextBase::CommitFocusGained(const awt::FocusEvent& rFocusEvent)
{
    if (mpFocusListeners)
    {
        //  Call all listeners.
        uno::Sequence< uno::Reference< uno::XInterface > > aListeners = mpFocusListeners->getElements();
        sal_uInt32 nLength(aListeners.getLength());
        if (nLength)
        {
            const uno::Reference< uno::XInterface >* pInterfaces = aListeners.getConstArray();
            if (pInterfaces)
            {
                for (sal_uInt32 i = 0; i < nLength; i++)
                {
                    uno::Reference<awt::XFocusListener> xListener(pInterfaces[i], uno::UNO_QUERY);
                    if (xListener.is())
                        xListener->focusGained(rFocusEvent);
                }
            }
        }
    }
}

void ScAccessibleContextBase::CommitFocusLost(const awt::FocusEvent& rFocusEvent)
{
    if (mpFocusListeners)
    {
        //  Call all listeners.
        uno::Sequence< uno::Reference< uno::XInterface > > aListeners = mpFocusListeners->getElements();
        sal_uInt32 nLength(aListeners.getLength());
        if (nLength)
        {
            const uno::Reference< uno::XInterface >* pInterfaces = aListeners.getConstArray();
            if (pInterfaces)
            {
                for (sal_uInt32 i = 0; i < nLength; i++)
                {
                    uno::Reference<awt::XFocusListener> xListener(pInterfaces[i], uno::UNO_QUERY);
                    if (xListener.is())
                        xListener->focusLost(rFocusEvent);
                }
            }
        }
    }
}

Rectangle ScAccessibleContextBase::GetBoundingBoxOnScreen(void)
        throw (uno::RuntimeException)
{
    DBG_ERROR("not implemented");
    return Rectangle();
}

Rectangle ScAccessibleContextBase::GetBoundingBox(void)
        throw (uno::RuntimeException)
{
    DBG_ERROR("not implemented");
    return Rectangle();
}

