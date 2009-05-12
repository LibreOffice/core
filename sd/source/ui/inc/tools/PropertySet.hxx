/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PropertySet.hxx,v $
 *
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SD_TOOLS_PROPERTY_SET_HXX
#define SD_TOOLS_PROPERTY_SET_HXX

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase1.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <boost/scoped_ptr.hpp>
#include <map>

namespace css = ::com::sun::star;

namespace sd { namespace tools {

namespace {
    typedef ::cppu::WeakComponentImplHelper1 <
        css::beans::XPropertySet
    > PropertySetInterfaceBase;
}


/** A very simple implementation of the XPropertySet interface.  It does not
    support constrained properties and thus does not support vetoable
    listeners.  It does not support the optional property set info.

    In order to use it you have to derive from this class and implement the
    GetPropertyValue() and SetPropertyValue() methods.
*/
class PropertySet
    : protected ::cppu::BaseMutex,
      public PropertySetInterfaceBase
{
public:
    explicit PropertySet (void);
    virtual ~PropertySet (void);

    virtual void SAL_CALL disposing (void);


    /** Create an UnknownPropertyException, but do not yet throw it.  This
        method fills in the fields of the exception.
    */
    css::beans::UnknownPropertyException CreateUnknownPropertyException (
        const rtl::OUString& rsPropertyName);

    // XPropertySet

    virtual css::uno::Reference<css::beans::XPropertySetInfo>
        SAL_CALL getPropertySetInfo (void)
        throw(css::uno::RuntimeException);

    virtual void SAL_CALL setPropertyValue (
        const rtl::OUString& rsPropertyName,
        const css::uno::Any& rsPropertyValue)
        throw(css::beans::UnknownPropertyException,
            css::beans::PropertyVetoException,
            css::lang::IllegalArgumentException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException);

    virtual css::uno::Any SAL_CALL getPropertyValue (const rtl::OUString& rsPropertyName)
        throw(css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException);

    virtual void SAL_CALL addPropertyChangeListener (
        const rtl::OUString& rsPropertyName,
        const css::uno::Reference<css::beans::XPropertyChangeListener>& rxListener)
        throw(css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException);

    virtual void SAL_CALL removePropertyChangeListener (
        const rtl::OUString& rsPropertyName,
        const css::uno::Reference<css::beans::XPropertyChangeListener>& rxListener)
        throw(css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException);

    virtual void SAL_CALL addVetoableChangeListener (
        const rtl::OUString& rsPropertyName,
        const css::uno::Reference<css::beans::XVetoableChangeListener>& rxListener)
        throw(css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException);

    virtual void SAL_CALL removeVetoableChangeListener (
        const rtl::OUString& rsPropertyName,
        const css::uno::Reference<css::beans::XVetoableChangeListener>& rxListener)
        throw(css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException);

protected:
    /** Return the requested property value.
        @throw com::sun::star::beans::UnknownPropertyException when the
            property is not supported.
    */
    virtual css::uno::Any GetPropertyValue (const ::rtl::OUString& rsPropertyName) = 0;
    /** Set the given property value.
        @return the old value.
        @throw com::sun::star::beans::UnknownPropertyException when the
            property is not supported.
    */
    virtual css::uno::Any SetPropertyValue (
        const ::rtl::OUString& rsPropertyName,
        const css::uno::Any& rValue) = 0;

private:
    typedef ::std::multimap<rtl::OUString,
        css::uno::Reference<css::beans::XPropertyChangeListener> > ChangeListenerContainer;
    ::boost::scoped_ptr<ChangeListenerContainer> mpChangeListeners;

    /** Call all listeners that are registered for the given property name.
        Call this method with an empty property name to call listeners that
        are registered for all properties.
    */
    void CallListeners (
        const rtl::OUString& rsPropertyName,
        const css::beans::PropertyChangeEvent& rEvent);

    /** This method throws a DisposedException when the object has already been
        disposed.
    */
    void ThrowIfDisposed (void)
        throw (css::lang::DisposedException);
};

} } // end of namespace ::sd::tools

#endif
