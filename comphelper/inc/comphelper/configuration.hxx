/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2011 Red Hat, Inc., Stephan Bergmann <sbergman@redhat.com>
 * (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef INCLUDED_COMPHELPER_CONFIGURATION_HXX
#define INCLUDED_COMPHELPER_CONFIGURATION_HXX

#include "sal/config.h"

#include "boost/noncopyable.hpp"
#include "boost/optional.hpp"
#include "boost/shared_ptr.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "comphelper/comphelperdllapi.h"
#include "comphelper/processfactory.hxx"
#include "sal/types.h"

namespace com { namespace sun { namespace star {
    namespace configuration { class XReadWriteAccess; }
    namespace container {
        class XHierarchicalNameAccess;
        class XHierarchicalNameReplace;
        class XNameAccess;
        class XNameContainer;
    }
    namespace uno { class XComponentContext; }
} } }

namespace comphelper {

namespace detail { class ConfigurationWrapper; }

/// A batch of configuration changes that is committed as a whole.
///
/// Client code needs to call commit explicitly; otherwise the changes are lost
/// when the instance is destroyed.
///
/// This is the only class from this header file that client code should use
/// directly.
class COMPHELPER_DLLPUBLIC ConfigurationChanges: private boost::noncopyable {
public:
    static boost::shared_ptr< ConfigurationChanges > create(
        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >
            const & context = comphelper::getProcessComponentContext());

    ~ConfigurationChanges();

    void commit() const;

private:
    SAL_DLLPRIVATE ConfigurationChanges(
        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >
            const & context);

    SAL_DLLPRIVATE void setPropertyValue(
        OUString const & path, com::sun::star::uno::Any const & value)
        const;

    SAL_DLLPRIVATE com::sun::star::uno::Reference<
        com::sun::star::container::XHierarchicalNameReplace >
    getGroup(OUString const & path) const;

    SAL_DLLPRIVATE
    com::sun::star::uno::Reference< com::sun::star::container::XNameContainer >
    getSet(OUString const & path) const;

    com::sun::star::uno::Reference<
        com::sun::star::configuration::XReadWriteAccess > access_;

    friend class detail::ConfigurationWrapper;
};

namespace detail {

/// @internal
class COMPHELPER_DLLPUBLIC ConfigurationWrapper: private boost::noncopyable {
public:
    static ConfigurationWrapper const & get(
        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >
            const & context);

    SAL_DLLPRIVATE explicit ConfigurationWrapper(
        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >
            const & context);

    SAL_DLLPRIVATE ~ConfigurationWrapper();

    com::sun::star::uno::Any getPropertyValue(OUString const & path) const;

    void setPropertyValue(
        boost::shared_ptr< ConfigurationChanges > const & batch,
        OUString const & path, com::sun::star::uno::Any const & value)
        const;

    com::sun::star::uno::Any getLocalizedPropertyValue(
        OUString const & path) const;

    void setLocalizedPropertyValue(
        boost::shared_ptr< ConfigurationChanges > const & batch,
        OUString const & path, com::sun::star::uno::Any const & value)
        const;

    com::sun::star::uno::Reference<
        com::sun::star::container::XHierarchicalNameAccess >
    getGroupReadOnly(OUString const & path) const;

    com::sun::star::uno::Reference<
        com::sun::star::container::XHierarchicalNameReplace >
    getGroupReadWrite(
        boost::shared_ptr< ConfigurationChanges > const & batch,
        OUString const & path) const;

    com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >
    getSetReadOnly(OUString const & path) const;

    com::sun::star::uno::Reference< com::sun::star::container::XNameContainer >
    getSetReadWrite(
        boost::shared_ptr< ConfigurationChanges > const & batch,
        OUString const & path) const;

    boost::shared_ptr< ConfigurationChanges > createChanges() const;

private:
    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >
        context_;

    com::sun::star::uno::Reference<
        com::sun::star::container::XHierarchicalNameAccess > access_;
};

/// @internal
template< typename T > struct Convert: private boost::noncopyable {
    static com::sun::star::uno::Any toAny(T const & value)
    { return com::sun::star::uno::makeAny(value); }

    static T fromAny(com::sun::star::uno::Any const & value)
    { return value.get< T >(); }

private:
    Convert(); // not defined
    ~Convert(); // not defined
};

/// @internal
template< typename T > struct Convert< boost::optional< T > >:
    private boost::noncopyable
{
    static com::sun::star::uno::Any toAny(boost::optional< T > const & value) {
        return value
            ? com::sun::star::uno::makeAny(value.get())
            : com::sun::star::uno::Any();
    }

    static boost::optional< T > fromAny(com::sun::star::uno::Any const & value)
    {
        return value.hasValue()
            ? boost::optional< T >(value.get< T >()) : boost::optional< T >();
    }

private:
    Convert(); // not defined
    ~Convert(); // not defined
};

}

/// A type-safe wrapper around a (non-localized) configuration property.
///
/// Automatically generated headers for the various configuration properties
/// derive from this template and make available its member functions to access
/// each given configuration property.
template< typename T, typename U > struct ConfigurationProperty:
    private boost::noncopyable
{
    /// Get the value of the given (non-localized) configuration property.
    ///
    /// For nillable properties, U is of type boost::optional<U'>.
    static U get(
        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >
            const & context = comphelper::getProcessComponentContext())
    {
        // Folding this into one statement causes a bogus error at least with
        // Red Hat GCC 4.6.2-1:
        com::sun::star::uno::Any a(
            detail::ConfigurationWrapper::get(context).getPropertyValue(
                T::path()));
        return detail::Convert< U >::fromAny(a);
    }

    /// Set the value of the given (non-localized) configuration property, via a
    /// given changes batch.
    ///
    /// For nillable properties, U is of type boost::optional<U'>.
    static void set(
        U const & value,
        boost::shared_ptr< ConfigurationChanges > const & batch,
        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >
            const & context = comphelper::getProcessComponentContext())
    {
        detail::ConfigurationWrapper::get(context).setPropertyValue(
            batch, T::path(), detail::Convert< U >::toAny(value));
    }

private:
    ConfigurationProperty(); // not defined
    ~ConfigurationProperty(); // not defined
};

/// A type-safe wrapper around a localized configuration property.
///
/// Automatically generated headers for the various localized configuration
/// properties derive from this template and make available its member functions
/// to access each given localized configuration property.
template< typename T, typename U > struct ConfigurationLocalizedProperty:
    private boost::noncopyable
{
    /// Get the value of the given localized configuration property, for the
    /// locale currently set at the
    /// com.sun.star.configuration.theDefaultProvider.
    ///
    /// For nillable properties, U is of type boost::optional<U'>.
    static U get(
        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >
            const & context = comphelper::getProcessComponentContext())
    {
        // Folding this into one statement causes a bogus error at least with
        // Red Hat GCC 4.6.2-1:
        com::sun::star::uno::Any a(
            detail::ConfigurationWrapper::get(context).
            getLocalizedPropertyValue(T::path()));
        return detail::Convert< U >::fromAny(a);
    }

    /// Set the value of the given localized configuration property, for the
    /// locale currently set at the
    /// com.sun.star.configuration.theDefaultProvider, via a given changes
    /// batch.
    ///
    /// For nillable properties, U is of type boost::optional<U'>.
    static void set(
        U const & value,
        boost::shared_ptr< ConfigurationChanges > const & batch,
        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >
            const & context = comphelper::getProcessComponentContext())
    {
        detail::ConfigurationWrapper::get(context).setLocalizedPropertyValue(
            batch, T::path(), detail::Convert< U >::toAny(value));
    }

private:
    ConfigurationLocalizedProperty(); // not defined
    ~ConfigurationLocalizedProperty(); // not defined
};

/// A type-safe wrapper around a configuration group.
///
/// Automatically generated headers for the various configuration groups derive
/// from this template and make available its member functions to access each
/// given configuration group.
template< typename T > struct ConfigurationGroup: private boost::noncopyable {
    /// Get read-only access to the given configuration group.
    static com::sun::star::uno::Reference<
        com::sun::star::container::XHierarchicalNameAccess >
    get(com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >
            const & context = comphelper::getProcessComponentContext())
    {
        return detail::ConfigurationWrapper::get(context).getGroupReadOnly(
            T::path());
    }

    /// Get read/write access to the given configuration group, storing any
    /// modifications via the given changes batch.
    static com::sun::star::uno::Reference<
        com::sun::star::container::XHierarchicalNameReplace >
    get(boost::shared_ptr< ConfigurationChanges > const & batch,
        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >
            const & context = comphelper::getProcessComponentContext())
    {
        return detail::ConfigurationWrapper::get(context).getGroupReadWrite(
            batch, T::path());
    }

private:
    ConfigurationGroup(); // not defined
    ~ConfigurationGroup(); // not defined
};

/// A type-safe wrapper around a configuration set.
///
/// Automatically generated headers for the various configuration sets derive
/// from this template and make available its member functions to access each
/// given configuration set.
template< typename T > struct ConfigurationSet: private boost::noncopyable {
    /// Get read-only access to the given configuration set.
    static
    com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >
    get(com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >
            const & context = comphelper::getProcessComponentContext())
    {
        return detail::ConfigurationWrapper::get(context).getSetReadOnly(
            T::path());
    }

    /// Get read/write access to the given configuration set, storing any
    /// modifications via the given changes batch.
    static
    com::sun::star::uno::Reference< com::sun::star::container::XNameContainer >
    get(boost::shared_ptr< ConfigurationChanges > const & batch,
        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >
            const & context = comphelper::getProcessComponentContext())
    {
        return detail::ConfigurationWrapper::get(context).getSetReadWrite(
            batch, T::path());
    }

private:
    ConfigurationSet(); // not defined
    ~ConfigurationSet(); // not defined
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
