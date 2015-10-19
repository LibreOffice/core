/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_COMPHELPER_CONFIGURATION_HXX
#define INCLUDED_COMPHELPER_CONFIGURATION_HXX

#include <sal/config.h>

#include <boost/optional.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <comphelper/comphelperdllapi.h>
#include <comphelper/processfactory.hxx>
#include <sal/types.h>
#include <memory>

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
class COMPHELPER_DLLPUBLIC ConfigurationChanges {
public:
    static std::shared_ptr<ConfigurationChanges> create(
        css::uno::Reference< css::uno::XComponentContext >
            const & context = comphelper::getProcessComponentContext());

    ~ConfigurationChanges();

    void commit() const;

private:
    ConfigurationChanges(const ConfigurationChanges&) = delete;
    ConfigurationChanges& operator=(const ConfigurationChanges&) = delete;

    SAL_DLLPRIVATE ConfigurationChanges(
        css::uno::Reference< css::uno::XComponentContext >
            const & context);

    SAL_DLLPRIVATE void setPropertyValue(
        OUString const & path, css::uno::Any const & value)
        const;

    SAL_DLLPRIVATE css::uno::Reference<
        css::container::XHierarchicalNameReplace >
    getGroup(OUString const & path) const;

    SAL_DLLPRIVATE
    css::uno::Reference< css::container::XNameContainer >
    getSet(OUString const & path) const;

    css::uno::Reference<
        css::configuration::XReadWriteAccess > access_;

    friend class detail::ConfigurationWrapper;
};

namespace detail {

/// @internal
class COMPHELPER_DLLPUBLIC ConfigurationWrapper {
public:
    static ConfigurationWrapper const & get(
        css::uno::Reference< css::uno::XComponentContext >
            const & context);

    SAL_DLLPRIVATE explicit ConfigurationWrapper(
        css::uno::Reference< css::uno::XComponentContext >
            const & context);

    SAL_DLLPRIVATE ~ConfigurationWrapper();

    bool isReadOnly(OUString const & path) const;

    css::uno::Any getPropertyValue(OUString const & path) const;

    static void setPropertyValue(
        std::shared_ptr< ConfigurationChanges > const & batch,
        OUString const & path, css::uno::Any const & value);

    css::uno::Any getLocalizedPropertyValue(
        OUString const & path) const;

    static void setLocalizedPropertyValue(
        std::shared_ptr< ConfigurationChanges > const & batch,
        OUString const & path, css::uno::Any const & value);

    css::uno::Reference<
        css::container::XHierarchicalNameAccess >
    getGroupReadOnly(OUString const & path) const;

    static css::uno::Reference<
        css::container::XHierarchicalNameReplace >
    getGroupReadWrite(
        std::shared_ptr< ConfigurationChanges > const & batch,
        OUString const & path);

    css::uno::Reference< css::container::XNameAccess >
    getSetReadOnly(OUString const & path) const;

    static css::uno::Reference< css::container::XNameContainer >
    getSetReadWrite(
        std::shared_ptr< ConfigurationChanges > const & batch,
        OUString const & path);

    std::shared_ptr< ConfigurationChanges > createChanges() const;

private:
    ConfigurationWrapper(const ConfigurationWrapper&) = delete;
    ConfigurationWrapper& operator=(const ConfigurationWrapper&) = delete;

    css::uno::Reference< css::uno::XComponentContext >          context_;

    css::uno::Reference< css::configuration::XReadWriteAccess > access_;
        // should really be an css.configuration.ReadOnlyAccess (with added
        // css.beans.XHierarchicalPropertySetInfo), but then
        // configmgr::Access::asProperty() would report all properties as
        // READONLY, so isReadOnly() would not work
};

/// @internal
template< typename T > struct Convert {
    static css::uno::Any toAny(T const & value)
    { return css::uno::makeAny(value); }

    static T fromAny(css::uno::Any const & value)
    { return value.get< T >(); }

private:
    Convert(const Convert&) = delete;
    Convert& operator=(const Convert&) = delete;

    Convert() = delete;
    ~Convert() = delete;
};

/// @internal
template< typename T > struct Convert< boost::optional< T > >
{
    static css::uno::Any toAny(boost::optional< T > const & value) {
        return value
            ? css::uno::makeAny(value.get())
            : css::uno::Any();
    }

    static boost::optional< T > fromAny(css::uno::Any const & value)
    {
        return value.hasValue()
            ? boost::optional< T >(value.get< T >()) : boost::optional< T >();
    }

private:
    Convert(const Convert&) = delete;
    Convert& operator=(const Convert&) = delete;

    Convert() = delete;
    ~Convert() = delete;
};

}

/// A type-safe wrapper around a (non-localized) configuration property.
///
/// Automatically generated headers for the various configuration properties
/// derive from this template and make available its member functions to access
/// each given configuration property.
template< typename T, typename U > struct ConfigurationProperty
{
    /// Get the read-only status of the given (non-localized) configuration
    /// property.
    static bool isReadOnly(
        css::uno::Reference<css::uno::XComponentContext> const & context
            = comphelper::getProcessComponentContext())
    {
        return detail::ConfigurationWrapper::get(context).isReadOnly(T::path());
    }

    /// Get the value of the given (non-localized) configuration property.
    ///
    /// For nillable properties, U is of type boost::optional<U'>.
    static U get(
        css::uno::Reference< css::uno::XComponentContext >
            const & context = comphelper::getProcessComponentContext())
    {
        // Folding this into one statement causes a bogus error at least with
        // Red Hat GCC 4.6.2-1:
        css::uno::Any a(
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
        std::shared_ptr< ConfigurationChanges > const & batch)
    {
        comphelper::detail::ConfigurationWrapper::setPropertyValue(
            batch, T::path(), detail::Convert< U >::toAny(value));
    }

private:
    ConfigurationProperty(const ConfigurationProperty&) = delete;
    ConfigurationProperty& operator=(const ConfigurationProperty&) = delete;

    ConfigurationProperty() = delete;
    ~ConfigurationProperty() = delete;
};

/// A type-safe wrapper around a localized configuration property.
///
/// Automatically generated headers for the various localized configuration
/// properties derive from this template and make available its member functions
/// to access each given localized configuration property.
template< typename T, typename U > struct ConfigurationLocalizedProperty
{
    /// Get the value of the given localized configuration property, for the
    /// locale currently set at the
    /// com.sun.star.configuration.theDefaultProvider.
    ///
    /// For nillable properties, U is of type boost::optional<U'>.
    static U get(
        css::uno::Reference< css::uno::XComponentContext >
            const & context = comphelper::getProcessComponentContext())
    {
        // Folding this into one statement causes a bogus error at least with
        // Red Hat GCC 4.6.2-1:
        css::uno::Any a(
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
        std::shared_ptr< ConfigurationChanges > const & batch)
    {
        comphelper::detail::ConfigurationWrapper::setLocalizedPropertyValue(
            batch, T::path(), detail::Convert< U >::toAny(value));
    }

private:
    ConfigurationLocalizedProperty(const ConfigurationLocalizedProperty&) = delete;
    ConfigurationLocalizedProperty& operator=(const ConfigurationLocalizedProperty&) = delete;

    ConfigurationLocalizedProperty() = delete;
    ~ConfigurationLocalizedProperty() = delete;
};

/// A type-safe wrapper around a configuration group.
///
/// Automatically generated headers for the various configuration groups derive
/// from this template and make available its member functions to access each
/// given configuration group.
template< typename T > struct ConfigurationGroup {
    /// Get read-only access to the given configuration group.
    static css::uno::Reference<
        css::container::XHierarchicalNameAccess >
    get(css::uno::Reference< css::uno::XComponentContext >
            const & context = comphelper::getProcessComponentContext())
    {
        return detail::ConfigurationWrapper::get(context).getGroupReadOnly(
            T::path());
    }

    /// Get read/write access to the given configuration group, storing any
    /// modifications via the given changes batch.
    static css::uno::Reference<
        css::container::XHierarchicalNameReplace >
    get(std::shared_ptr< ConfigurationChanges > const & batch)
    {
        return comphelper::detail::ConfigurationWrapper::getGroupReadWrite(
            batch, T::path());
    }

private:
    ConfigurationGroup(const ConfigurationGroup&) = delete;
    ConfigurationGroup& operator=(const ConfigurationGroup&) = delete;

    ConfigurationGroup() = delete;
    ~ConfigurationGroup() = delete;
};

/// A type-safe wrapper around a configuration set.
///
/// Automatically generated headers for the various configuration sets derive
/// from this template and make available its member functions to access each
/// given configuration set.
template< typename T > struct ConfigurationSet {
    /// Get read-only access to the given configuration set.
    static
    css::uno::Reference< css::container::XNameAccess >
    get(css::uno::Reference< css::uno::XComponentContext >
            const & context = comphelper::getProcessComponentContext())
    {
        return detail::ConfigurationWrapper::get(context).getSetReadOnly(
            T::path());
    }

    /// Get read/write access to the given configuration set, storing any
    /// modifications via the given changes batch.
    static
    css::uno::Reference< css::container::XNameContainer >
    get(std::shared_ptr< ConfigurationChanges > const & batch)
    {
        return comphelper::detail::ConfigurationWrapper::getSetReadWrite(
            batch, T::path());
    }

private:
    ConfigurationSet(const ConfigurationSet&) = delete;
    ConfigurationSet& operator=(const ConfigurationSet&) = delete;

    ConfigurationSet() = delete;
    ~ConfigurationSet() = delete;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
