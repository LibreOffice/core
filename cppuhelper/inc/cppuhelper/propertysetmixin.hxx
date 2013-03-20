/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_CPPUHELPER_PROPERTYSETMIXIN_HXX
#define INCLUDED_CPPUHELPER_PROPERTYSETMIXIN_HXX

#include "sal/config.h"
#include "com/sun/star/beans/PropertyVetoException.hpp"
#include "com/sun/star/beans/UnknownPropertyException.hpp"
#include "com/sun/star/beans/XFastPropertySet.hpp"
#include "com/sun/star/beans/XPropertyAccess.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/lang/IllegalArgumentException.hpp"
#include "com/sun/star/lang/WrappedTargetException.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "sal/types.h"
#include "cppuhelperdllapi.h"

namespace com { namespace sun { namespace star {
    namespace beans {
        class XPropertyChangeListener;
        class XPropertySetInfo;
        class XVetoableChangeListener;
        struct PropertyValue;
    }
    namespace uno {
        class Any;
        class Type;
        class XComponentContext;
    }
} } }
namespace rtl { class OUString; }

namespace cppu {

template< typename T > class PropertySetMixin;

// Suppress warnings about virtual functions but non-virtual destructor:
#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4265)
#endif

/**
   @short A helper base class for <code>cppu::PropertySetMixin</code>.

   See the documentation of <code>cppu::PropertySetMixin</code> for
   further details.

   That <code>cppu::PropertySetMixin</code> is derived from this
   base class should be considered an implementation detail.  The functionality
   of <code>cppu::PropertySetMixin</code> that is inherited from this base
   class and is visible to subclasses of
   <code>cppu::PropertySetMixin</code> should be treated by such
   subclasses as being provided by <code>cppu::PropertySetMixin</code>
   directly (e.g., in such subclasses, use
   &ldquo;<code>PropertySetMixin::Implements</code>&rdquo; instead of
   &ldquo;<code>PropertySetMixinImpl::Implements</code>&rdquo;).

   @since UDK 3.2.1
*/
#if defined __GNUC__ && ((__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#endif
class CPPUHELPER_DLLPUBLIC PropertySetMixinImpl:
    public com::sun::star::beans::XPropertySet,
    public com::sun::star::beans::XFastPropertySet,
    public com::sun::star::beans::XPropertyAccess
{
protected:
    /**
       @short Flags used by subclasses of
       <code>cppu::PropertySetMixin</code> to specify what UNO interface
       types shall be supported.
    */
    enum Implements {
        /**
           @short Flag specifying that the UNO interface type
           <code>com::sun::star::beans::XPropertySet</code> shall be supported.
        */
        IMPLEMENTS_PROPERTY_SET = 1,

        /**
           @short Flag specifying that the UNO interface type
           <code>com::sun::star::beans::XFastPropertySet</code> shall be
           supported.
        */
        IMPLEMENTS_FAST_PROPERTY_SET = 2,

        /**
           @short Flag specifying that the UNO interface type
           <code>com::sun::star::beans::XPropertyAccess</code> shall be
           supported.
        */
        IMPLEMENTS_PROPERTY_ACCESS = 4
    };

    /**
       @short A class used by subclasses of
       <code>cppu::PropertySetMixin</code> when implementing UNO interface
       type attribute setter functions.

       This class is not thread safe; that is, the constructor,
       <code>notify</code>, and the destructor must be called from the same
       thread.

       See <code>cppu::PropertySetMixinImpl::prepareSet</code> for
       further details.
    */
    class CPPUHELPER_DLLPUBLIC BoundListeners {
    public:
        /**
           @short The constructor.

           May throw <code>std::bad_alloc</code>.
        */
        BoundListeners();

        /**
           @short The destructor.

           Does not throw.
        */
        ~BoundListeners();

        /**
           @short Notifies any
           <code>com::sun::star::beans::XPropertyChangeListener</code>s.

           May throw <code>com::sun::star::uno::RuntimeException</code>
           and <code>std::bad_alloc</code>.

           See <code>cppu::PropertySetMixinImpl::prepareSet</code>
           for further details.
         */
        void notify() const;

    private:
        BoundListeners(BoundListeners &); // not defined
        void operator =(BoundListeners); // not defined

        class Impl;
        Impl * m_impl;

        friend class PropertySetMixinImpl;
    };

    /**
       @short A function used by subclasses of
       <code>cppu::PropertySetMixin</code> when implementing UNO interface
       type attribute setter functions.

       First, this function checks whether this instance has already been
       disposed (see <code>cppu::PropertySetMixinImpl::dispose</code>),
       and throws a <code>com::sun::star::lang::DisposedException</code> if
       applicable.  For a constrained attribute (whose setter can explicitly
       raise <code>com::sun::star::beans::PropertyVetoException</code>), this
       function notifies any
       <code>com::sun::star::beans::XVetoableChangeListener</code>s.  For a
       bound attribute, this function modifies the passed-in
       <code>boundListeners</code> so that it can afterwards be used to notify
       any <code>com::sun::star::beans::XPropertyChangeListener</code>s.  This
       function should be called before storing the new attribute value, and
       <code>boundListeners->notify()</code> should be called exactly once after
       storing the new attribute value (in case the attribute is bound;
       otherwise, calling <code>boundListeners->notify()</code> is ignored).
       Furthermore, <code>boundListeners->notify()</code> and this function have
       to be called from the same thread.

       May throw
       <code>com::sun::star::beans::PropertyVetoException</code>,
       <code>com::sun::star::uno::RuntimeException</code> (and
       <code>com::sun::star::lang::DisposedException</code> in particular), and
       <code>std::bad_alloc</code>.

       @param propertyName  the name of the property (which is the same as the
       name of the attribute that is going to be set)

       @param oldValue the property value corresponding to the old attribute
       value.  This is only used as
       <code>com::sun::star::beans::PropertyChangeEvent::OldValue</code>, which
       is rather useless, anyway (see &ldquo;Using the Observer Pattern&rdquo;
       in <a href="http://tools.openoffice.org/CodingGuidelines.sxw">
       OpenOffice.org Coding Guidelines</a>).  If the attribute
       that is going to be set is neither bound nor constrained, or if
       <code>com::sun::star::beans::PropertyChangeEvent::OldValue</code> should
       not be set, a <code>VOID</code> <code>Any</code> can be used instead.

       @param newValue the property value corresponding to the new
       attribute value.  This is only used as
       <code>com::sun::star::beans::PropertyChangeEvent::NewValue</code>, which
       is rather useless, anyway (see &ldquo;Using the Observer Pattern&rdquo;
       in <a href="http://tools.openoffice.org/CodingGuidelines.sxw">
       OpenOffice.org Coding Guidelines</a>), <em>unless</em> the
       attribute that is going to be set is constrained.  If the attribute
       that is going to be set is neither bound nor constrained, or if it is
       only bound but
       <code>com::sun::star::beans::PropertyChangeEvent::NewValue</code> should
       not be set, a <code>VOID</code> <code>Any</code> can be used instead.

       @param boundListeners  a pointer to a fresh
       <code>cppu::PropertySetMixinImpl::BoundListeners</code> instance
       (which has not been passed to this function before, and on which
       <code>notify</code> has not yet been called); may only be null if the
       attribute that is going to be set is not bound
    */
    void prepareSet(
        rtl::OUString const & propertyName,
        com::sun::star::uno::Any const & oldValue,
        com::sun::star::uno::Any const & newValue,
        BoundListeners * boundListeners);

    /**
       @short Mark this instance as being disposed.

       See <code>com::sun::star::lang::XComponent</code> for the general
       concept of disposing UNO objects.  On the first call to this function,
       all registered listeners
       (<code>com::sun::star::beans::XPropertyChangeListener</code>s and
       <code>com::sun::star::beans::XVetoableChangeListener</code>s) are
       notified of the disposing source.  Any subsequent calls to this function
       are ignored.

       May throw <code>com::sun::star::uno::RuntimeException</code> and
       <code>std::bad_alloc</code>.
     */
    void dispose();

    /**
       @short A function used by subclasses of
       <code>cppu::PropertySetMixin</code> when implementing
       <code>com::sun::star::uno::XInterface::queryInterface</code>.

       This function checks for support of any of the UNO interface types
       specified in the call of the <code>cppu::PropertySetMixin</code>
       constructor.  It does not check for any other UNO interface types (not
       even for <code>com::sun::star::uno::XInterface</code>), and should not
       be used directly as the implementation of
       <code>com::sun::star::uno::XInterface::queryInterface</code> of this UNO
       object.
    */
    virtual com::sun::star::uno::Any SAL_CALL queryInterface(
        com::sun::star::uno::Type const & type)
        throw (com::sun::star::uno::RuntimeException);

    // @see com::sun::star::beans::XPropertySet::getPropertySetInfo
    virtual
    com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo >
    SAL_CALL getPropertySetInfo() throw (com::sun::star::uno::RuntimeException);

    // @see com::sun::star::beans::XPropertySet::setPropertyValue
    virtual void SAL_CALL setPropertyValue(
        rtl::OUString const & propertyName,
        com::sun::star::uno::Any const & value)
        throw (
            com::sun::star::beans::UnknownPropertyException,
            com::sun::star::beans::PropertyVetoException,
            com::sun::star::lang::IllegalArgumentException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException);

    // @see com::sun::star::beans::XPropertySet::getPropertyValue
    virtual com::sun::star::uno::Any SAL_CALL getPropertyValue(
        rtl::OUString const & propertyName)
        throw (
            com::sun::star::beans::UnknownPropertyException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException);

    /**
       @short Adds a
       <code>com::sun::star::beans::XPropertyChangeListener</code>.

       If a listener is added more than once, it will receive all
       relevant notifications multiple times.

       @see com::sun::star::beans::XPropertySet::addPropertyChangeListener
    */
    virtual void SAL_CALL addPropertyChangeListener(
        rtl::OUString const & propertyName,
        com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertyChangeListener > const & listener)
        throw (
            com::sun::star::beans::UnknownPropertyException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException);

    // @see com::sun::star::beans::XPropertySet::removePropertyChangeListener
    virtual void SAL_CALL removePropertyChangeListener(
        rtl::OUString const & propertyName,
        com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertyChangeListener > const & listener)
        throw (
            com::sun::star::beans::UnknownPropertyException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException);

    /**
       @short Adds a
       <code>com::sun::star::beans::XVetoableChangeListener</code>.

       If a listener is added more than once, it will receive all
       relevant notifications multiple times.

       @see com::sun::star::beans::XPropertySet::addVetoableChangeListener
    */
    virtual void SAL_CALL addVetoableChangeListener(
        rtl::OUString const & propertyName,
        com::sun::star::uno::Reference<
        com::sun::star::beans::XVetoableChangeListener > const & listener)
        throw (
            com::sun::star::beans::UnknownPropertyException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException);

    // @see com::sun::star::beans::XPropertySet::removeVetoableChangeListener
    virtual void SAL_CALL removeVetoableChangeListener(
        rtl::OUString const & propertyName,
        com::sun::star::uno::Reference<
        com::sun::star::beans::XVetoableChangeListener > const & listener)
        throw (
            com::sun::star::beans::UnknownPropertyException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException);

    // @see com::sun::star::beans::XFastPropertySet::setFastPropertyValue
    virtual void SAL_CALL setFastPropertyValue(
        sal_Int32 handle, com::sun::star::uno::Any const & value)
        throw (
            com::sun::star::beans::UnknownPropertyException,
            com::sun::star::beans::PropertyVetoException,
            com::sun::star::lang::IllegalArgumentException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException);

    // @see com::sun::star::beans::XFastPropertySet::getFastPropertyValue
    virtual com::sun::star::uno::Any SAL_CALL getFastPropertyValue(
        sal_Int32 handle)
        throw (
            com::sun::star::beans::UnknownPropertyException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException);

    // @see com::sun::star::beans::XPropertyAccess::getPropertyValues
    virtual
    com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >
    SAL_CALL getPropertyValues() throw (com::sun::star::uno::RuntimeException);

    // @see com::sun::star::beans::XPropertyAccess::setPropertyValues
    virtual void SAL_CALL setPropertyValues(
        com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >
        const & props)
        throw (
            com::sun::star::beans::UnknownPropertyException,
            com::sun::star::beans::PropertyVetoException,
            com::sun::star::lang::IllegalArgumentException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException);

private:
    PropertySetMixinImpl(PropertySetMixinImpl &); // not defined
    void operator =(PropertySetMixinImpl &); // not defined

    PropertySetMixinImpl(
        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >
        const & context,
        Implements implements,
        com::sun::star::uno::Sequence< rtl::OUString > const & absentOptional,
        com::sun::star::uno::Type const & type);

    class Impl;
    Impl * m_impl;

    friend class Impl;
    template< typename T > friend class PropertySetMixin;

    ~PropertySetMixinImpl();

    void checkUnknown(rtl::OUString const & propertyName);
};
#if defined __GNUC__ && ((__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4)
#pragma GCC diagnostic pop
#endif

/**
   @short A helper mixin to implement certain UNO interfaces related to property
   set handling on top of the attributes of a given UNO interface type.

   The UNO interface type is specified by the type parameter
   <code>T</code> (which must correspond to a UNO interface type).

   No specializations of this class template should be added by client
   code.

   @since UDK 3.2.1
*/
template< typename T > class PropertySetMixin: public PropertySetMixinImpl {
protected:
    /**
       @short The constructor.

       May throw <code>com::sun::star::uno::RuntimeException</code> and
       <code>std::bad_alloc</code>.

       @param context  the component context used by this class template; must
       not be null, and must supply the service
       <code>com.sun.star.reflection.CoreReflection</code> and the singleton
       <code>com.sun.star.reflection.theTypeDescriptionManager</code>

       @param implements  a combination of zero or more flags specifying what
       UNO interface types shall be supported

       @param absentOptional  a list of optional properties that are not
       present, and should thus not be visible via
       <code>com::sun::star::beans::XPropertySet::getPropertySetInfo</code>,
       <code>com::sun::star::beans::XPropertySet::addPropertyChangeListener<!--
       --></code>, <code>com::sun::star::beans::XPropertySet::<!--
       -->removePropertyChangeListener</code>,
       <code>com::sun::star::beans::XPropertySet::addVetoableChangeListener<!--
       --></code>, and <code>com::sun::star::beans::XPropertySet::<!--
       -->removeVetoableChangeListener</code>.  For consistency reasons, the
       given <code>absentOptional</code> should only contain the names of
       attributes that represent optional properties that are not present (that
       is, the attribute getters and setters always throw a
       <code>com::sun::star::beans::UnknownPropertyException</code>), and should
       contain each such name only once.  If an optional property is not present
       (that is, the corresponding attribute getter and setter always throw a
       <code>com::sun::star::beans::UnknownPropertyException</code>) but is not
       contained in the given <code>absentOptional</code>, then it will be
       visible via
       <code>com::sun::star::beans::XPropertySet::getPropertySetInfo</code> as a
       <code>com::sun::star::beans::Property</code> with a set
       <code>com::sun::star::beans::PropertyAttribute::OPTIONAL</code>.  If the
       given <code>implements</code> specifies that
       <code>com::sun::star::beans::XPropertySet</code> is not supported, then
       the given <code>absentOptional</code> is effectively ignored and can be
       empty.
    */
    PropertySetMixin(
        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >
        const & context,
        Implements implements,
        com::sun::star::uno::Sequence< rtl::OUString > const & absentOptional):
        PropertySetMixinImpl(
            context, implements, absentOptional, T::static_type())
    {}

    /**
       @short The destructor.

       Does not throw.
    */
    ~PropertySetMixin() {}

private:
    PropertySetMixin(PropertySetMixin &); // not defined
    void operator =(PropertySetMixin &); // not defined
};

#if defined _MSC_VER
#pragma warning(pop)
#endif

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
