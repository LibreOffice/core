/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
