/*************************************************************************
 *
 *  $RCSfile: AccessibleContextBase.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: sab $ $Date: 2002-01-31 10:39:09 $
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

#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleRole.hpp>
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

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::accessibility;

//=====  internal  ============================================================

ScAccessibleContextBase::ScAccessibleContextBase (
        const uno::Reference<XAccessible>& rxParent,
        const sal_Int16 aRole)
    :
    maRole(aRole),
    mxParent(rxParent),
    mpPropertyChangeListeners(NULL)
{
}


ScAccessibleContextBase::~ScAccessibleContextBase(void)
{
    if (mpPropertyChangeListeners)
    {
        lang::EventObject aEvent;
        aEvent.Source = static_cast<cppu::OWeakObject*>(this);
        mpPropertyChangeListeners->disposeAndClear(aEvent);
        delete mpPropertyChangeListeners;
    }
}

//=====  XAccessible  =========================================================

uno::Reference< XAccessibleContext> SAL_CALL
    ScAccessibleContextBase::getAccessibleContext (void)
    throw (uno::RuntimeException)
{
    return this;
}

//=====  XAccessibleContext  ==================================================

sal_Int32 SAL_CALL
       ScAccessibleContextBase::getAccessibleChildCount (void)
    throw (uno::RuntimeException)
{
    DBG_ERROR("not implemented");
    return 0;
}




uno::Reference<XAccessible> SAL_CALL
    ScAccessibleContextBase::getAccessibleChild (long nIndex)
    throw (uno::RuntimeException/*,
        lang::IndexOutOfBoundsException*/)
{
    DBG_ERROR("not implemented");
    return uno::Reference<XAccessible>();
}




uno::Reference<XAccessible> SAL_CALL
       ScAccessibleContextBase::getAccessibleParent (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return mxParent;
}




sal_Int32 SAL_CALL
       ScAccessibleContextBase::getAccessibleIndexInParent (void)
    throw (::com::sun::star::uno::RuntimeException)
{
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
    ScAccessibleContextBase::getAccessibleRole (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return maRole;
}




::rtl::OUString SAL_CALL
       ScAccessibleContextBase::getAccessibleDescription (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    DBG_ERROR("not implemented");
    /*if (!msDescription.getLength())
    {
        OUString sDescription(createAccessibleDescription());
        DBG_ASSERT(sDescription.getLength(), "We should give always a descripition.");

        uno::Any aOldValue, aNewValue;
        aOldValue <<= msDescription;
        aNewValue <<= sDescription;

        msDescription = sDescription;

        CommitChange(OUString::createFromAscii ("AccessibleDescription"), aNewValue, aOldValue);
    }*/
    return msDescription;
}




OUString SAL_CALL
       ScAccessibleContextBase::getAccessibleName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    DBG_ERROR("not implemented");
    /*if (!msName.getLength())
    {
        OUString sName(createAccessibleName());
        DBG_ASSERT(sName.getLength(), "We should give always a name.");

        uno::Any aOldValue, aNewValue;
        aOldValue <<= msName;
        aNewValue <<= sName;

        msName = sName;

        CommitChange(OUString::createFromAscii ("AccessibleName"), aNewValue, aOldValue);
    }*/
    return msName;
}




/** Return empty reference to indicate that the relation set is not
    supported.
*/
uno::Reference<XAccessibleRelationSet> SAL_CALL
       ScAccessibleContextBase::getAccessibleRelationSet (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return uno::Reference<XAccessibleRelationSet>();
}




/** Create the set of states the object is in.  Possible states are:
        EDITABLE
        ENABLED
        SHOWING
        VISIBLE
    For now, return only an empty reference and wait for an implementation of
    the XAccessibleStateSet interface.
*/
uno::Reference<XAccessibleStateSet> SAL_CALL
        ScAccessibleContextBase::getAccessibleStateSet (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return uno::Reference<XAccessibleStateSet>();
}




lang::Locale SAL_CALL
       ScAccessibleContextBase::getLocale (void)
    throw (IllegalAccessibleComponentStateException,
        ::com::sun::star::uno::RuntimeException)
{
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




void SAL_CALL
       ScAccessibleContextBase::addPropertyChangeListener (
           const uno::Reference<beans::XPropertyChangeListener>& xListener)
    throw (::com::sun::star::uno::RuntimeException)
{
    if (xListener.is())
    {
        if (!mpPropertyChangeListeners)
            mpPropertyChangeListeners = new cppu::OInterfaceContainerHelper(maMutex);
        mpPropertyChangeListeners->addInterface(xListener);
    }
}




void SAL_CALL
       ScAccessibleContextBase::removePropertyChangeListener (
        const uno::Reference<beans::XPropertyChangeListener>& xListener)
    throw (::com::sun::star::uno::RuntimeException)
{
    if (xListener.is())
        mpPropertyChangeListeners->removeInterface(xListener);
}

//=====  XServiceInfo  ========================================================

::rtl::OUString SAL_CALL
       ScAccessibleContextBase::getImplementationName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM ("ScAccessibleContextBase"));
}

sal_Bool SAL_CALL
     ScAccessibleContextBase::supportsService (const OUString& sServiceName)
    throw (::com::sun::star::uno::RuntimeException)
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
       ScAccessibleContextBase::getSupportedServiceNames (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    const OUString sServiceName (RTL_CONSTASCII_USTRINGPARAM ("drafts.com.sun.star.accessibility.AccessibleContext"));
    return uno::Sequence<OUString> (&sServiceName, 1);
}

//=====  XTypeProvider  =======================================================

uno::Sequence< ::com::sun::star::uno::Type>
    ScAccessibleContextBase::getTypes (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    const ::com::sun::star::uno::Type aTypeList[] = {
        ::getCppuType((const uno::Reference<
            XAccessible>*)0),
        ::getCppuType((const uno::Reference<
            XAccessibleContext>*)0),
        ::getCppuType((const uno::Reference<
            lang::XServiceInfo>*)0),
        ::getCppuType((const uno::Reference<
            lang::XTypeProvider>*)0),
        ::getCppuType((const uno::Reference<
            lang::XServiceName>*)0)
        };
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type>
        aTypeSequence (aTypeList, 5);
    return aTypeSequence;
}

uno::Sequence<sal_Int8> SAL_CALL
    ScAccessibleContextBase::getImplementationId (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    static uno::Sequence<sal_Int8> aId;
    if (aId.getLength() == 0)
    {
        ::osl::MutexGuard aGuard (maMutex);
        aId.realloc (16);
        rtl_createUuid ((sal_uInt8 *)aId.getArray(), 0, sal_True);
    }
    return aId;
}

//=====  XServiceName  ========================================================

::rtl::OUString
    ScAccessibleContextBase::getServiceName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM("drafts.com.sun.star.accessibility.AccessibleContext"));
}

//=====  internal  ============================================================

void ScAccessibleContextBase::CommitChange(const rtl::OUString& rPropertyName,
                    const uno::Any& rNewValue,
                    const uno::Any& rOldValue)
{
    beans::PropertyChangeEvent  aEvent;
    aEvent.PropertyName = rPropertyName;
    aEvent.Further = sal_False;
    aEvent.PropertyHandle = -1;
    aEvent.OldValue = rOldValue;
    aEvent.NewValue = rNewValue;

    //  Call all listeners.
    uno::Sequence< uno::Reference< uno::XInterface > > aListeners = mpPropertyChangeListeners->getElements();
    sal_uInt32 nLength(aListeners.getLength());
    if (nLength)
    {
        const uno::Reference< uno::XInterface >* pInterfaces = aListeners.getConstArray();
        if (pInterfaces)
        {
            for (sal_uInt32 i = 0; i < nLength; i++)
            {
                uno::Reference<beans::XPropertyChangeListener> xListener(pInterfaces[i], uno::UNO_QUERY);
                if (xListener.is())
                    xListener->propertyChange(aEvent);
            }
        }
    }
}
