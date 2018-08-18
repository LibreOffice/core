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

#include "com/sun/star/beans/XFastPropertySet.hpp"
#include "com/sun/star/beans/XPropertyAccess.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/uno/Reference.h"
#include "com/sun/star/uno/Sequence.hxx"
#include "sal/types.h"
#include "cppuhelper/cppuhelperdllapi.h"

namespace com { namespace sun { namespace star {
    namespace beans {
        class XPropertyChangeListener;
        class XPropertySetInfo;
        class XVetoableChangeListener;
        struct PropertyValue;
    }
    namespace uno {
        class Type;
        class XComponentContext;
    }
} } }

namespace cppu {

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
#if defined __GNUC__ && !defined __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#endif
class CPPUHELPER_DLLPUBLIC PropertySetMixinImpl:
    public css::beans::XPropertySet,
    public css::beans::XFastPropertySet,
    public css::beans::XPropertyAccess
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
           <code>css::beans::XPropertySet</code> shall be supported.
        */
        IMPLEMENTS_PROPERTY_SET = 1,

        /**
           @short Flag specifying that the UNO interface type
           <code>css::beans::XFastPropertySet</code> shall be
           supported.
        */
        IMPLEMENTS_FAST_PROPERTY_SET = 2,

        /**
           @short Flag specifying that the UNO interface type
           <code>css::beans::XPropertyAccess</code> shall be
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
           <code>css::beans::XPropertyChangeListener</code>s.

           May throw <code>css::uno::RuntimeException</code>
           and <code>std::bad_alloc</code>.

           See <code>cppu::PropertySetMixinImpl::prepareSet</code>
           for further details.
         */
        void notify() const;

    private:
        BoundListeners( const BoundListeners&) SAL_DELETED_FUNCTION;
        void operator=( const BoundListeners&) SAL_DELETED_FUNCTION;

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
       and throws a <code>css::lang::DisposedException</code> if
       applicable.  For a constrained attribute (whose setter can explicitly
       raise <code>css::beans::PropertyVetoException</code>), this
       function notifies any
       <code>css::beans::XVetoableChangeListener</code>s.  For a
       bound attribute, this function modifies the passed-in
       <code>boundListeners</code> so that it can afterwards be used to notify
       any <code>css::beans::XPropertyChangeListener</code>s.  This
       function should be called before storing the new attribute value, and
       <code>boundListeners->notify()</code> should be called exactly once after
       storing the new attribute value (in case the attribute is bound;
       otherwise, calling <code>boundListeners->notify()</code> is ignored).
       Furthermore, <code>boundListeners->notify()</code> and this function have
       to be called from the same thread.

       May throw
       <code>css::beans::PropertyVetoException</code>,
       <code>css::uno::RuntimeException</code> (and
       <code>css::lang::DisposedException</code> in particular), and
       <code>std::bad_alloc</code>.

       @param propertyName  the name of the property (which is the same as the
       name of the attribute that is going to be set)

       @param oldValue the property value corresponding to the old attribute
       value.  This is only used as
       <code>css::beans::PropertyChangeEvent::OldValue</code>, which
       is rather useless, anyway (see &ldquo;Using the Observer Pattern&rdquo;
       in <a href="http://tools.openoffice.org/CodingGuidelines.sxw">
       OpenOffice.org Coding Guidelines</a>).  If the attribute
       that is going to be set is neither bound nor constrained, or if
       <code>css::beans::PropertyChangeEvent::OldValue</code> should
       not be set, a <code>VOID</code> <code>Any</code> can be used instead.

       @param newValue the property value corresponding to the new
       attribute value.  This is only used as
       <code>css::beans::PropertyChangeEvent::NewValue</code>, which
       is rather useless, anyway (see &ldquo;Using the Observer Pattern&rdquo;
       in <a href="http://tools.openoffice.org/CodingGuidelines.sxw">
       OpenOffice.org Coding Guidelines</a>), <em>unless</em> the
       attribute that is going to be set is constrained.  If the attribute
       that is going to be set is neither bound nor constrained, or if it is
       only bound but
       <code>css::beans::PropertyChangeEvent::NewValue</code> should
       not be set, a <code>VOID</code> <code>Any</code> can be used instead.

       @param boundListeners  a pointer to a fresh
       <code>cppu::PropertySetMixinImpl::BoundListeners</code> instance
       (which has not been passed to this function before, and on which
       <code>notify</code> has not yet been called); may only be null if the
       attribute that is going to be set is not bound
    */
    void prepareSet(
        rtl::OUString const & propertyName,
        css::uno::Any const & oldValue,
        css::uno::Any const & newValue,
        BoundListeners * boundListeners);

    /**
       @short Mark this instance as being disposed.

       See <code>css::lang::XComponent</code> for the general
       concept of disposing UNO objects.  On the first call to this function,
       all registered listeners
       (<code>css::beans::XPropertyChangeListener</code>s and
       <code>css::beans::XVetoableChangeListener</code>s) are
       notified of the disposing source.  Any subsequent calls to this function
       are ignored.

       May throw <code>css::uno::RuntimeException</code> and
       <code>std::bad_alloc</code>.
     */
    void dispose();

    /**
       @short A function used by subclasses of
       <code>cppu::PropertySetMixin</code> when implementing
       <code>css::uno::XInterface::queryInterface</code>.

       This function checks for support of any of the UNO interface types
       specified in the call of the <code>cppu::PropertySetMixin</code>
       constructor.  It does not check for any other UNO interface types (not
       even for <code>css::uno::XInterface</code>), and should not
       be used directly as the implementation of
       <code>css::uno::XInterface::queryInterface</code> of this UNO
       object.
    */
    virtual css::uno::Any SAL_CALL queryInterface(
        css::uno::Type const & type) SAL_OVERRIDE;

    // @see css::beans::XPropertySet::getPropertySetInfo
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() SAL_OVERRIDE;

    // @see css::beans::XPropertySet::setPropertyValue
    virtual void SAL_CALL setPropertyValue(
        rtl::OUString const & propertyName,
        css::uno::Any const & value) SAL_OVERRIDE;

    // @see css::beans::XPropertySet::getPropertyValue
    virtual css::uno::Any SAL_CALL getPropertyValue(
        rtl::OUString const & propertyName) SAL_OVERRIDE;

    /**
       @short Adds a
       <code>css::beans::XPropertyChangeListener</code>.

       If a listener is added more than once, it will receive all
       relevant notifications multiple times.

       @see css::beans::XPropertySet::addPropertyChangeListener
    */
    virtual void SAL_CALL addPropertyChangeListener(
        rtl::OUString const & propertyName,
        css::uno::Reference<
        css::beans::XPropertyChangeListener > const & listener) SAL_OVERRIDE;

    // @see css::beans::XPropertySet::removePropertyChangeListener
    virtual void SAL_CALL removePropertyChangeListener(
        rtl::OUString const & propertyName,
        css::uno::Reference<
        css::beans::XPropertyChangeListener > const & listener) SAL_OVERRIDE;

    /**
       @short Adds a
       <code>css::beans::XVetoableChangeListener</code>.

       If a listener is added more than once, it will receive all
       relevant notifications multiple times.

       @see css::beans::XPropertySet::addVetoableChangeListener
    */
    virtual void SAL_CALL addVetoableChangeListener(
        rtl::OUString const & propertyName,
        css::uno::Reference<
        css::beans::XVetoableChangeListener > const & listener) SAL_OVERRIDE;

    // @see css::beans::XPropertySet::removeVetoableChangeListener
    virtual void SAL_CALL removeVetoableChangeListener(
        rtl::OUString const & propertyName,
        css::uno::Reference<
        css::beans::XVetoableChangeListener > const & listener) SAL_OVERRIDE;

    // @see css::beans::XFastPropertySet::setFastPropertyValue
    virtual void SAL_CALL setFastPropertyValue(
        sal_Int32 handle, css::uno::Any const & value) SAL_OVERRIDE;

    // @see css::beans::XFastPropertySet::getFastPropertyValue
    virtual css::uno::Any SAL_CALL getFastPropertyValue(
        sal_Int32 handle) SAL_OVERRIDE;

    // @see css::beans::XPropertyAccess::getPropertyValues
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getPropertyValues() SAL_OVERRIDE;

    // @see css::beans::XPropertyAccess::setPropertyValues
    virtual void SAL_CALL setPropertyValues(
        css::uno::Sequence< css::beans::PropertyValue > const & props) SAL_OVERRIDE;

private:
    PropertySetMixinImpl( const PropertySetMixinImpl&) SAL_DELETED_FUNCTION;
    void operator=( const PropertySetMixinImpl&) SAL_DELETED_FUNCTION;

    PropertySetMixinImpl(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        Implements implements,
        css::uno::Sequence< rtl::OUString > const & absentOptional,
        css::uno::Type const & type);

    class Impl;
    Impl * m_impl;

    friend class Impl;
    template< typename T > friend class PropertySetMixin;

    ~PropertySetMixinImpl();

    void checkUnknown(rtl::OUString const & propertyName);
};

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

       May throw <code>css::uno::RuntimeException</code> and
       <code>std::bad_alloc</code>.

       @param context  the component context used by this class template; must
       not be null, and must supply the
       <code>com.sun.star.reflection.theCoreReflection</code> and
       <code>com.sun.star.reflection.theTypeDescriptionManager</code> singletons

       @param implements  a combination of zero or more flags specifying what
       UNO interface types shall be supported

       @param absentOptional  a list of optional properties that are not
       present, and should thus not be visible via
       <code>css::beans::XPropertySet::getPropertySetInfo</code>,
       <code>css::beans::XPropertySet::addPropertyChangeListener<!--
       --></code>, <code>css::beans::XPropertySet::<!--
       -->removePropertyChangeListener</code>,
       <code>css::beans::XPropertySet::addVetoableChangeListener<!--
       --></code>, and <code>css::beans::XPropertySet::<!--
       -->removeVetoableChangeListener</code>.  For consistency reasons, the
       given <code>absentOptional</code> should only contain the names of
       attributes that represent optional properties that are not present (that
       is, the attribute getters and setters always throw a
       <code>css::beans::UnknownPropertyException</code>), and should
       contain each such name only once.  If an optional property is not present
       (that is, the corresponding attribute getter and setter always throw a
       <code>css::beans::UnknownPropertyException</code>) but is not
       contained in the given <code>absentOptional</code>, then it will be
       visible via
       <code>css::beans::XPropertySet::getPropertySetInfo</code> as a
       <code>css::beans::Property</code> with a set
       <code>css::beans::PropertyAttribute::OPTIONAL</code>.  If the
       given <code>implements</code> specifies that
       <code>css::beans::XPropertySet</code> is not supported, then
       the given <code>absentOptional</code> is effectively ignored and can be
       empty.
    */
    PropertySetMixin(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        Implements implements,
        css::uno::Sequence< rtl::OUString > const & absentOptional):
        PropertySetMixinImpl(
            context, implements, absentOptional, T::static_type())
    {}

    /**
       @short The destructor.

       Does not throw.
    */
    ~PropertySetMixin() {}

private:
    PropertySetMixin( const PropertySetMixin&) SAL_DELETED_FUNCTION;
    void operator=( const PropertySetMixin&) SAL_DELETED_FUNCTION;
};
#if defined __GNUC__ && !defined __clang__
#pragma GCC diagnostic pop
#endif

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
