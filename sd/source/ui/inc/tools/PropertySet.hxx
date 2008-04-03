/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PropertySet.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:05:26 $
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
