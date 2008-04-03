/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PropertySet.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:53:13 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "precompiled_sd.hxx"

#include "tools/PropertySet.hxx"
#include <boost/bind.hpp>
#include <algorithm>
#include <functional>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;

namespace sd { namespace tools {

PropertySet::PropertySet (void)
    : PropertySetInterfaceBase(m_aMutex),
      mpChangeListeners(new ChangeListenerContainer())
{
}




PropertySet::~PropertySet (void)
{
}




void SAL_CALL PropertySet::disposing (void)
{
}




beans::UnknownPropertyException PropertySet::CreateUnknownPropertyException (
    const rtl::OUString& rsPropertyName)
{
    return beans::UnknownPropertyException(
        OUString::createFromAscii("property ")
            + rsPropertyName
                + OUString::createFromAscii(" is not known"),
        static_cast<XWeak*>(this));
}




//----- XPropertySet ----------------------------------------------------------

Reference<beans::XPropertySetInfo> SAL_CALL PropertySet::getPropertySetInfo (void)
    throw(RuntimeException)
{
    return NULL;
}




void SAL_CALL PropertySet::setPropertyValue (
    const rtl::OUString& rsPropertyName,
    const css::uno::Any& rsPropertyValue)
    throw(css::beans::UnknownPropertyException,
        css::beans::PropertyVetoException,
        css::lang::IllegalArgumentException,
        css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    ThrowIfDisposed();

    Any aOldValue (SetPropertyValue(rsPropertyName,rsPropertyValue));
    if (aOldValue != rsPropertyValue)
    {
        // Inform listeners that are registered specifically for the
        // property and those registered for any property.
        beans::PropertyChangeEvent aEvent(
            static_cast<XWeak*>(this),
            rsPropertyName,
            sal_False,
            -1,
            aOldValue,
            rsPropertyValue);
        CallListeners(rsPropertyName, aEvent);
        CallListeners(OUString(), aEvent);
    }
}




Any SAL_CALL PropertySet::getPropertyValue (const OUString& rsPropertyName)
        throw(css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException)
{
    ThrowIfDisposed();

    return GetPropertyValue(rsPropertyName);
}




void SAL_CALL PropertySet::addPropertyChangeListener (
    const rtl::OUString& rsPropertyName,
    const css::uno::Reference<css::beans::XPropertyChangeListener>& rxListener)
    throw(css::beans::UnknownPropertyException,
        css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    if ( ! rxListener.is())
        throw lang::IllegalArgumentException();

    if (rBHelper.bDisposed || rBHelper.bInDispose)
        return;

    mpChangeListeners->insert(
        ChangeListenerContainer::value_type(
            rsPropertyName,
            rxListener));
}




void SAL_CALL PropertySet::removePropertyChangeListener (
    const rtl::OUString& rsPropertyName,
    const css::uno::Reference<css::beans::XPropertyChangeListener>& rxListener)
    throw(beans::UnknownPropertyException,
        css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    ::std::pair<ChangeListenerContainer::iterator,ChangeListenerContainer::iterator>
        aRange (mpChangeListeners->equal_range(rsPropertyName));

    ChangeListenerContainer::iterator iListener (
        ::std::find_if(
            aRange.first,
            aRange.second,
            std::compose1(
                std::bind1st(std::equal_to<Reference<beans::XPropertyChangeListener> >(),
                    rxListener),
                std::select2nd<ChangeListenerContainer::value_type>())));
    if (iListener != mpChangeListeners->end())
    {
        mpChangeListeners->erase(iListener);
    }
    else
    {
        throw lang::IllegalArgumentException();
    }
}




void SAL_CALL PropertySet::addVetoableChangeListener (
    const rtl::OUString& rsPropertyName,
    const css::uno::Reference<css::beans::XVetoableChangeListener>& rxListener)
    throw(css::beans::UnknownPropertyException,
        css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    // Constraint properties are not supported and thus no vetoable
    // listeners.
    (void)rsPropertyName;
    (void)rxListener;
}




void SAL_CALL PropertySet::removeVetoableChangeListener (
    const rtl::OUString& rsPropertyName,
    const css::uno::Reference<css::beans::XVetoableChangeListener>& rxListener)
    throw(css::beans::UnknownPropertyException,
        css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    // Constraint properties are not supported and thus no vetoable
    // listeners.
    (void)rsPropertyName;
    (void)rxListener;
}




//-----------------------------------------------------------------------------

void PropertySet::CallListeners (
    const rtl::OUString& rsPropertyName,
    const beans::PropertyChangeEvent& rEvent)
{
    ::std::pair<ChangeListenerContainer::iterator,ChangeListenerContainer::iterator>
        aRange (mpChangeListeners->equal_range(rsPropertyName));
    ChangeListenerContainer::const_iterator iListener;
    for (iListener=aRange.first; iListener!=aRange.second; ++iListener)
    {
        if (iListener->second.is())
            iListener->second->propertyChange(rEvent);
    }
}




void PropertySet::ThrowIfDisposed (void)
    throw (::com::sun::star::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                "PropertySet object has already been disposed")),
            static_cast<uno::XWeak*>(this));
    }
}

} } // end of namespace ::sd::tools
