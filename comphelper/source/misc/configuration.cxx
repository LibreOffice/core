/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cassert>
#include <memory>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/configuration/ReadOnlyAccess.hpp>
#include <com/sun/star/configuration/ReadWriteAccess.hpp>
#include <com/sun/star/configuration/XReadWriteAccess.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XHierarchicalNameReplace.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XLocalizable.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/solarmutex.hxx>
#include <comphelper/configuration.hxx>
#include <comphelper/configurationlistener.hxx>
#include <rtl/instance.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <i18nlangtag/languagetag.hxx>

namespace {

struct TheConfigurationWrapper:
    public rtl::StaticWithArg<
        comphelper::detail::ConfigurationWrapper,
        css::uno::Reference< css::uno::XComponentContext >,
        TheConfigurationWrapper >
{};

OUString getDefaultLocale(
    css::uno::Reference< css::uno::XComponentContext > const & context)
{
    return LanguageTag(
        css::uno::Reference< css::lang::XLocalizable >(
            css::configuration::theDefaultProvider::get(context),
            css::uno::UNO_QUERY_THROW)->
        getLocale()).getBcp47(false);
}

OUString extendLocalizedPath(OUString const & path, OUString const & locale) {
    OUStringBuffer buf(path);
    buf.append("/['*");
    SAL_WARN_IF(
        locale.match("*"), "comphelper",
        "Locale \"" << locale << "\" starts with \"*\"");
    assert(locale.indexOf('&') == -1);
    assert(locale.indexOf('"') == -1);
    assert(locale.indexOf('\'') == -1);
    buf.append(locale);
    buf.append("']");
    return buf.makeStringAndClear();
}

}

std::shared_ptr< comphelper::ConfigurationChanges >
comphelper::ConfigurationChanges::create(
    css::uno::Reference< css::uno::XComponentContext > const & context)
{
    return TheConfigurationWrapper::get(context).createChanges();
}

comphelper::ConfigurationChanges::~ConfigurationChanges() {}

void comphelper::ConfigurationChanges::commit() const {
    access_->commitChanges();
}

comphelper::ConfigurationChanges::ConfigurationChanges(
    css::uno::Reference< css::uno::XComponentContext > const & context):
    access_(
        css::configuration::ReadWriteAccess::create(
            context, getDefaultLocale(context)))
{}

void comphelper::ConfigurationChanges::setPropertyValue(
    OUString const & path, css::uno::Any const & value) const
{
    access_->replaceByHierarchicalName(path, value);
}

css::uno::Reference< css::container::XHierarchicalNameReplace >
comphelper::ConfigurationChanges::getGroup(OUString const & path) const
{
    return css::uno::Reference< css::container::XHierarchicalNameReplace >(
        access_->getByHierarchicalName(path), css::uno::UNO_QUERY_THROW);
}

css::uno::Reference< css::container::XNameContainer >
comphelper::ConfigurationChanges::getSet(OUString const & path) const
{
    return css::uno::Reference< css::container::XNameContainer >(
        access_->getByHierarchicalName(path), css::uno::UNO_QUERY_THROW);
}

comphelper::detail::ConfigurationWrapper const &
comphelper::detail::ConfigurationWrapper::get(
    css::uno::Reference< css::uno::XComponentContext > const & context)
{
    return TheConfigurationWrapper::get(context);
}

comphelper::detail::ConfigurationWrapper::ConfigurationWrapper(
    css::uno::Reference< css::uno::XComponentContext > const & context):
    context_(context),
    access_(css::configuration::ReadWriteAccess::create(context, "*"))
{}

comphelper::detail::ConfigurationWrapper::~ConfigurationWrapper() {}

bool comphelper::detail::ConfigurationWrapper::isReadOnly(OUString const & path)
    const
{
css::beans::Property SB(access_->getPropertyByHierarchicalName(path));
    return
        (access_->getPropertyByHierarchicalName(path).Attributes
         & css::beans::PropertyAttribute::READONLY)
        != 0;
}

css::uno::Any comphelper::detail::ConfigurationWrapper::getPropertyValue(
    OUString const & path) const
{
    return access_->getByHierarchicalName(path);
}

void comphelper::detail::ConfigurationWrapper::setPropertyValue(
    std::shared_ptr< ConfigurationChanges > const & batch,
    OUString const & path, css::uno::Any const & value)
{
    assert(batch.get() != nullptr);
    batch->setPropertyValue(path, value);
}

css::uno::Any
comphelper::detail::ConfigurationWrapper::getLocalizedPropertyValue(
    OUString const & path) const
{
    return access_->getByHierarchicalName(
        extendLocalizedPath(path, getDefaultLocale(context_)));
}

void comphelper::detail::ConfigurationWrapper::setLocalizedPropertyValue(
    std::shared_ptr< ConfigurationChanges > const & batch,
    OUString const & path, css::uno::Any const & value)
{
    assert(batch.get() != nullptr);
    batch->setPropertyValue(path, value);
}

css::uno::Reference< css::container::XHierarchicalNameAccess >
comphelper::detail::ConfigurationWrapper::getGroupReadOnly(
    OUString const & path) const
{
    return css::uno::Reference< css::container::XHierarchicalNameAccess >(
        (css::configuration::ReadOnlyAccess::create(
            context_, getDefaultLocale(context_))->
         getByHierarchicalName(path)),
        css::uno::UNO_QUERY_THROW);
}

css::uno::Reference< css::container::XHierarchicalNameReplace >
comphelper::detail::ConfigurationWrapper::getGroupReadWrite(
    std::shared_ptr< ConfigurationChanges > const & batch,
    OUString const & path)
{
    assert(batch.get() != nullptr);
    return batch->getGroup(path);
}

css::uno::Reference< css::container::XNameAccess >
comphelper::detail::ConfigurationWrapper::getSetReadOnly(
    OUString const & path) const
{
    return css::uno::Reference< css::container::XNameAccess >(
        (css::configuration::ReadOnlyAccess::create(
            context_, getDefaultLocale(context_))->
         getByHierarchicalName(path)),
        css::uno::UNO_QUERY_THROW);
}

css::uno::Reference< css::container::XNameContainer >
comphelper::detail::ConfigurationWrapper::getSetReadWrite(
    std::shared_ptr< ConfigurationChanges > const & batch,
    OUString const & path)
{
    assert(batch.get() != nullptr);
    return batch->getSet(path);
}

std::shared_ptr< comphelper::ConfigurationChanges >
comphelper::detail::ConfigurationWrapper::createChanges() const {
    return std::shared_ptr< ConfigurationChanges >(
        new ConfigurationChanges(context_));
}

void comphelper::ConfigurationListener::addListener(ConfigurationListenerPropertyBase *pListener)
{
    maListeners.push_back( pListener );
    mxConfig->addPropertyChangeListener( pListener->maName, this );
    pListener->setProperty( mxConfig->getPropertyValue( pListener->maName ) );
}

void comphelper::ConfigurationListener::removeListener(ConfigurationListenerPropertyBase *pListener)
{
    auto it = std::find( maListeners.begin(), maListeners.end(), pListener );
    if ( it != maListeners.end() )
    {
        maListeners.erase( it );
        mxConfig->removePropertyChangeListener( pListener->maName, this );
    }
}

void comphelper::ConfigurationListener::dispose()
{
    for (auto it = maListeners.begin(); it != maListeners.end(); ++it)
    {
        mxConfig->removePropertyChangeListener( (*it)->maName, this );
        (*it)->dispose();
    }
    maListeners.clear();
}

void SAL_CALL comphelper::ConfigurationListener::disposing(css::lang::EventObject const &)
    throw (css::uno::RuntimeException, std::exception)
{
    dispose();
}

void SAL_CALL comphelper::ConfigurationListener::propertyChange(
    css::beans::PropertyChangeEvent const &rEvt )
    throw (css::uno::RuntimeException, std::exception)
{
    // Code is commonly used inside the SolarMutexGuard
    // so to avoid concurrent writes to the property,
    // and allow fast, lock-less access, guard here.
    rtl::Reference< comphelper::SolarMutex > xMutexGuard(
        comphelper::SolarMutex::get() );

    assert( rEvt.Source == mxConfig );
    for ( auto it = maListeners.begin(); it != maListeners.end(); ++it )
    {
        if ( (*it)->maName == rEvt.PropertyName )
        {
            // ignore rEvt.NewValue - in theory it could be stale => not set.
            css::uno::Any aValue = mxConfig->getPropertyValue( (*it)->maName );
            (*it)->setProperty( aValue );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
