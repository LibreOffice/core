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

#include "sal/config.h"

#include <cassert>

#include "boost/shared_ptr.hpp"
#include "com/sun/star/configuration/ReadOnlyAccess.hpp"
#include "com/sun/star/configuration/ReadWriteAccess.hpp"
#include "com/sun/star/configuration/XReadWriteAccess.hpp"
#include "com/sun/star/configuration/theDefaultProvider.hpp"
#include "com/sun/star/container/XHierarchicalNameAccess.hpp"
#include "com/sun/star/container/XHierarchicalNameReplace.hpp"
#include "com/sun/star/container/XNameAccess.hpp"
#include "com/sun/star/container/XNameContainer.hpp"
#include "com/sun/star/lang/Locale.hpp"
#include "com/sun/star/lang/XLocalizable.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "rtl/instance.hxx"
#include "rtl/oustringostreaminserter.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/log.hxx"
#include "unotools/configuration.hxx"

namespace {

namespace css = com::sun::star;

struct TheConfigurationWrapper:
    public rtl::StaticWithArg<
        unotools::detail::ConfigurationWrapper,
        css::uno::Reference< css::uno::XComponentContext >,
        TheConfigurationWrapper >
{};

}

boost::shared_ptr< unotools::ConfigurationChanges >
unotools::ConfigurationChanges::create(
    css::uno::Reference< css::uno::XComponentContext > const & context)
{
    return TheConfigurationWrapper::get(context).createChanges();
}


unotools::ConfigurationChanges::~ConfigurationChanges() {}

void unotools::ConfigurationChanges::commit() const {
    access_->commitChanges();
}

unotools::ConfigurationChanges::ConfigurationChanges(
    css::uno::Reference< css::uno::XComponentContext > const & context):
    access_(css::configuration::ReadWriteAccess::create(context))
{}

void unotools::ConfigurationChanges::setPropertyValue(
    rtl::OUString const & path, css::uno::Any const & value) const
{
    access_->replaceByHierarchicalName(path, value);
}

css::uno::Reference< css::container::XHierarchicalNameReplace >
unotools::ConfigurationChanges::getGroup(rtl::OUString const & path) const
{
    return css::uno::Reference< css::container::XHierarchicalNameReplace >(
        access_->getByHierarchicalName(path), css::uno::UNO_QUERY_THROW);
}

css::uno::Reference< css::container::XNameContainer >
unotools::ConfigurationChanges::getSet(rtl::OUString const & path) const
{
    return css::uno::Reference< css::container::XNameContainer >(
        access_->getByHierarchicalName(path), css::uno::UNO_QUERY_THROW);
}

unotools::detail::ConfigurationWrapper const &
unotools::detail::ConfigurationWrapper::get(
    css::uno::Reference< css::uno::XComponentContext > const & context)
{
    return TheConfigurationWrapper::get(context);
}

unotools::detail::ConfigurationWrapper::ConfigurationWrapper(
    css::uno::Reference< css::uno::XComponentContext > const & context):
    context_(context), access_(css::configuration::ReadOnlyAccess::get(context))
{}

unotools::detail::ConfigurationWrapper::~ConfigurationWrapper() {}

css::uno::Any unotools::detail::ConfigurationWrapper::getPropertyValue(
    rtl::OUString const & path) const
{
    return access_->getByHierarchicalName(path);
}

void unotools::detail::ConfigurationWrapper::setPropertyValue(
    boost::shared_ptr< ConfigurationChanges > const & batch,
    rtl::OUString const & path, com::sun::star::uno::Any const & value) const
{
    assert(batch.get() != 0);
    batch->setPropertyValue(path, value);
}

css::uno::Any unotools::detail::ConfigurationWrapper::getLocalizedPropertyValue(
    rtl::OUString const & path) const
{
    return access_->getByHierarchicalName(extendLocalizedPath(path));
}

void unotools::detail::ConfigurationWrapper::setLocalizedPropertyValue(
    boost::shared_ptr< ConfigurationChanges > const & batch,
    rtl::OUString const & path, com::sun::star::uno::Any const & value) const
{
    assert(batch.get() != 0);
    batch->setPropertyValue(extendLocalizedPath(path), value);
}

css::uno::Reference< css::container::XHierarchicalNameAccess >
unotools::detail::ConfigurationWrapper::getGroupReadOnly(
    rtl::OUString const & path) const
{
    return css::uno::Reference< css::container::XHierarchicalNameAccess >(
        access_->getByHierarchicalName(path), css::uno::UNO_QUERY_THROW);
}

css::uno::Reference< css::container::XHierarchicalNameReplace >
unotools::detail::ConfigurationWrapper::getGroupReadWrite(
    boost::shared_ptr< ConfigurationChanges > const & batch,
    rtl::OUString const & path) const
{
    assert(batch.get() != 0);
    return batch->getGroup(path);
}

css::uno::Reference< css::container::XNameAccess >
unotools::detail::ConfigurationWrapper::getSetReadOnly(
    rtl::OUString const & path) const
{
    return css::uno::Reference< css::container::XNameAccess >(
        access_->getByHierarchicalName(path), css::uno::UNO_QUERY_THROW);
}

css::uno::Reference< css::container::XNameContainer >
unotools::detail::ConfigurationWrapper::getSetReadWrite(
    boost::shared_ptr< ConfigurationChanges > const & batch,
    rtl::OUString const & path) const
{
    assert(batch.get() != 0);
    return batch->getSet(path);
}

boost::shared_ptr< unotools::ConfigurationChanges >
unotools::detail::ConfigurationWrapper::createChanges() const {
    return boost::shared_ptr< ConfigurationChanges >(
        new ConfigurationChanges(context_));
}

rtl::OUString unotools::detail::ConfigurationWrapper::extendLocalizedPath(
    rtl::OUString const & path) const
{
    rtl::OUStringBuffer buf(path);
    buf.appendAscii(RTL_CONSTASCII_STRINGPARAM("/['"));
    css::lang::Locale locale(
        css::uno::Reference< css::lang::XLocalizable >(
            css::configuration::theDefaultProvider::get(context_),
            css::uno::UNO_QUERY_THROW)->
        getLocale());
    SAL_WARN_IF(
        locale.Language.indexOf('-') == -1, "unotools",
        "Locale language \"" << locale.Language << "\" contains \"-\"");
    assert(locale.Language.indexOf('&') == -1);
    assert(locale.Language.indexOf('"') == -1);
    assert(locale.Language.indexOf('\'') == -1);
    buf.append(locale.Language);
    SAL_WARN_IF(
        locale.Country.isEmpty() && !locale.Variant.isEmpty(), "unotools",
        "Locale has empty country but non-empty variant \"" << locale.Variant
            << '"');
    if (!locale.Country.isEmpty()) {
        buf.append('-');
        SAL_WARN_IF(
            locale.Country.indexOf('-') == -1, "unotools",
            "Locale language \"" << locale.Country << "\" contains \"-\"");
        assert(locale.Country.indexOf('&') == -1);
        assert(locale.Country.indexOf('"') == -1);
        assert(locale.Country.indexOf('\'') == -1);
        buf.append(locale.Country);
        if (!locale.Variant.isEmpty()) {
            buf.append('-');
            assert(locale.Variant.indexOf('&') == -1);
            assert(locale.Variant.indexOf('"') == -1);
            assert(locale.Variant.indexOf('\'') == -1);
            buf.append(locale.Variant);
        }
    }
    buf.appendAscii(RTL_CONSTASCII_STRINGPARAM("']"));
    return buf.makeStringAndClear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
