/*************************************************************************
 *
 *  $RCSfile: AccessibleContextBase.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2002-02-08 17:35:56 $
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

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::accessibility;

namespace accessibility {

//=====  internal  ============================================================

AccessibleContextBase::AccessibleContextBase (
        const uno::Reference<XAccessible>& rxParent,
        const sal_Int16 aRole)
    :
    maRole(aRole),
    mxParent(rxParent)
{
}




AccessibleContextBase::~AccessibleContextBase(void)
{
}




//=====  XAccessible  =========================================================

uno::Reference< XAccessibleContext> SAL_CALL
    AccessibleContextBase::getAccessibleContext (void)
    throw (uno::RuntimeException)
{
    return this;
}




//=====  XAccessibleContext  ==================================================

/** No children.
*/
sal_Int32 SAL_CALL
       AccessibleContextBase::getAccessibleChildCount (void)
    throw (uno::RuntimeException)
{
    return 0;
}




/** Forward the request to the shape.  Return the requested shape or throw
    an exception for a wrong index.
*/
uno::Reference<XAccessible> SAL_CALL
    AccessibleContextBase::getAccessibleChild (long nIndex)
    throw (::com::sun::star::uno::RuntimeException)
{
    throw lang::IndexOutOfBoundsException (
        ::rtl::OUString::createFromAscii ("no child with index " + nIndex),
        NULL);
}




uno::Reference<XAccessible> SAL_CALL
       AccessibleContextBase::getAccessibleParent (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return mxParent;
}




sal_Int32 SAL_CALL
       AccessibleContextBase::getAccessibleIndexInParent (void)
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
    AccessibleContextBase::getAccessibleRole (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return maRole;
}




::rtl::OUString SAL_CALL
       AccessibleContextBase::getAccessibleDescription (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return msDescription;
}




OUString SAL_CALL
       AccessibleContextBase::getAccessibleName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return msName;
}




/** Return empty reference to indicate that the relation set is not
    supported.
*/
uno::Reference<XAccessibleRelationSet> SAL_CALL
       AccessibleContextBase::getAccessibleRelationSet (void)
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
    AccessibleContextBase::getAccessibleStateSet (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return uno::Reference<XAccessibleStateSet>();
}




lang::Locale SAL_CALL
       AccessibleContextBase::getLocale (void)
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
       AccessibleContextBase::addPropertyChangeListener (
           const uno::Reference<beans::XPropertyChangeListener>& xListener)
    throw (::com::sun::star::uno::RuntimeException)
{
    if (xListener.is())
    {
        //  Make sure that the given listener is not already a member of the
        //  listener list.
        sal_Bool bFound(sal_False);
           PropertyChangeListenerListType::iterator aItr = mxPropertyChangeListeners.begin();
        while (!bFound && (aItr != mxPropertyChangeListeners.end()))
            if (*aItr == xListener)
                bFound = sal_True;
            else
                aItr++;

        if (!bFound)
        {
            //  Append the new listener to the end of the listener list.
            ::vos::OGuard aGuard (maMutex);
            mxPropertyChangeListeners.push_back (xListener);
        }
    }
}




void SAL_CALL
       AccessibleContextBase::removePropertyChangeListener (
        const uno::Reference<beans::XPropertyChangeListener>& xListener)
    throw (::com::sun::star::uno::RuntimeException)
{
    if (xListener.is())
    {
        //  Find the listener to remove by iterating over the whole list.
        sal_Bool bFound(sal_False);
           PropertyChangeListenerListType::iterator aItr = mxPropertyChangeListeners.begin();
        while(!bFound && (aItr != mxPropertyChangeListeners.end()))
        {
            if (*aItr == xListener)
            {
                //  Remove the listener and leave the iteration loop.
                ::vos::OGuard aGuard (maMutex);
                mxPropertyChangeListeners.erase (aItr);
                bFound = sal_True;
            }
            else
                aItr++;
        }
    }
}




//=====  XServiceInfo  ========================================================

::rtl::OUString SAL_CALL
       AccessibleContextBase::getImplementationName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM ("AccessibleContextBase"));
}

sal_Bool SAL_CALL
     AccessibleContextBase::supportsService (const OUString& sServiceName)
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
       AccessibleContextBase::getSupportedServiceNames (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    const OUString sServiceName (RTL_CONSTASCII_USTRINGPARAM ("drafts.com.sun.star.accessibility.AccessibleContext"));
    return uno::Sequence<OUString> (&sServiceName, 1);
}

//=====  XTypeProvider  =======================================================

uno::Sequence< ::com::sun::star::uno::Type>
    AccessibleContextBase::getTypes (void)
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
    AccessibleContextBase::getImplementationId (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    static uno::Sequence<sal_Int8> aId;
    if (aId.getLength() == 0)
    {
        ::vos::OGuard aGuard (maMutex);
        aId.realloc (16);
        rtl_createUuid ((sal_uInt8 *)aId.getArray(), 0, sal_True);
    }
    return aId;
}

//=====  XServiceName  ========================================================

::rtl::OUString
    AccessibleContextBase::getServiceName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM ("drafts.com.sun.star.accessibility.AccessibleContext"));
}

//=====  internal  ============================================================

void
    AccessibleContextBase::setAccessibleDescription (const ::rtl::OUString& rDescription)
    throw (uno::RuntimeException)
{
    if (msDescription != rDescription)
    {
        uno::Any aOldValue, aNewValue;
        aOldValue <<= msDescription;
        aNewValue <<= rDescription;

        msDescription = rDescription;

        CommitChange(OUString::createFromAscii ("AccessibleDescription"), aNewValue, aOldValue);
    }
}

void
    AccessibleContextBase::setAccessibleName (const ::rtl::OUString& rName)
    throw (uno::RuntimeException)
{
    if (msName != rName)
    {
        uno::Any aOldValue, aNewValue;
        aOldValue <<= msName;
        aNewValue <<= rName;

        msName = rName;

        CommitChange(OUString::createFromAscii ("AccessibleName"), aNewValue, aOldValue);
    }
}




::rtl::OUString AccessibleContextBase::createAccessibleDescription (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii ("Empty Description");
}




::rtl::OUString AccessibleContextBase::createAccessibleName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii ("Empty Name");
}




void AccessibleContextBase::CommitChange(const rtl::OUString& rPropertyName,
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
       PropertyChangeListenerListType::iterator I;
       for (I=mxPropertyChangeListeners.begin();
           I!=mxPropertyChangeListeners.end(); I++)
    {
        (*I)->propertyChange (aEvent);
    }
}

} // end of namespace accessibility
