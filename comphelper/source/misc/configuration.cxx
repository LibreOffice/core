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
#include <mutex>
#include <string_view>
#include <unordered_map>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/configuration/ReadOnlyAccess.hpp>
#include <com/sun/star/configuration/ReadWriteAccess.hpp>
#include <com/sun/star/configuration/XReadWriteAccess.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XHierarchicalNameReplace.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XLocalizable.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/util/XChangesListener.hpp>
#include <com/sun/star/util/XChangesNotifier.hpp>
#include <comphelper/solarmutex.hxx>
#include <comphelper/configuration.hxx>
#include <comphelper/configurationlistener.hxx>
#include <cppuhelper/implbase.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <i18nlangtag/languagetag.hxx>

namespace com::sun::star::uno { class XComponentContext; }

namespace {

OUString getDefaultLocale(
    css::uno::Reference< css::uno::XComponentContext > const & context)
{
    return LanguageTag(
        css::uno::Reference< css::lang::XLocalizable >(
            css::configuration::theDefaultProvider::get(context),
            css::uno::UNO_QUERY_THROW)->
        getLocale()).getBcp47(false);
}

OUString extendLocalizedPath(std::u16string_view path, OUString const & locale) {
    SAL_WARN_IF(
        locale.match("*"), "comphelper",
        "Locale \"" << locale << "\" starts with \"*\"");
    assert(locale.indexOf('&') == -1);
    assert(locale.indexOf('"') == -1);
    assert(locale.indexOf('\'') == -1);
    return OUString::Concat(path) + "/['*" + locale + "']";
}

}

std::shared_ptr< comphelper::ConfigurationChanges >
comphelper::ConfigurationChanges::create()
{
    return detail::ConfigurationWrapper::get().createChanges();
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
comphelper::detail::ConfigurationWrapper::get()
{
    static comphelper::detail::ConfigurationWrapper WRAPPER;
    return WRAPPER;
}

namespace
{
std::mutex gMutex;
std::unordered_map<OUString, css::uno::Any> gPropertyCache;
css::uno::Reference< css::util::XChangesNotifier > gNotifier;
css::uno::Reference< css::util::XChangesListener > gListener;

class ConfigurationChangesListener
    : public ::cppu::WeakImplHelper<css::util::XChangesListener>
{
public:
    ConfigurationChangesListener()
    {}
    // util::XChangesListener
    virtual void SAL_CALL changesOccurred( const css::util::ChangesEvent& ) override
    {
        std::scoped_lock aGuard(gMutex);
        gPropertyCache.clear();
    }
    virtual void SAL_CALL disposing(const css::lang::EventObject&) override
    {
        std::scoped_lock aGuard(gMutex);
        gPropertyCache.clear();
    }
};

} // namespace

comphelper::detail::ConfigurationWrapper::ConfigurationWrapper():
    context_(comphelper::getProcessComponentContext()),
    access_(css::configuration::ReadWriteAccess::create(context_, "*"))
{
    // Set up a configuration notifier to invalidate the cache as needed.
    try
    {
        css::uno::Reference< css::lang::XMultiServiceFactory > xConfigProvider(
            css::configuration::theDefaultProvider::get( context_ ) );

        // set root path
        css::uno::Sequence< css::uno::Any > params {
            css::uno::Any( css::beans::NamedValue{ "nodepath", css::uno::Any( OUString("/"))} ),
            css::uno::Any( css::beans::NamedValue{ "locale", css::uno::Any( OUString("*"))} ) };

        css::uno::Reference< css::uno::XInterface > xCfg
            = xConfigProvider->createInstanceWithArguments(u"com.sun.star.configuration.ConfigurationAccess",
                params);

        gNotifier = css::uno::Reference< css::util::XChangesNotifier >(xCfg, css::uno::UNO_QUERY);
        assert(gNotifier.is());
        gListener = css::uno::Reference< ConfigurationChangesListener >(new ConfigurationChangesListener());
        gNotifier->addChangesListener(gListener);
    }
    catch(const css::uno::Exception&)
    {
        assert(false);
    }
}

comphelper::detail::ConfigurationWrapper::~ConfigurationWrapper()
{
    gPropertyCache.clear();
    gNotifier.clear();
    gListener.clear();
}

bool comphelper::detail::ConfigurationWrapper::isReadOnly(OUString const & path)
    const
{
    return
        (access_->getPropertyByHierarchicalName(path).Attributes
         & css::beans::PropertyAttribute::READONLY)
        != 0;
}

css::uno::Any comphelper::detail::ConfigurationWrapper::getPropertyValue(OUString const& path) const
{
    std::scoped_lock aGuard(gMutex);
    // Cache the configuration access, since some of the keys are used in hot code.
    auto it = gPropertyCache.find(path);
    if( it != gPropertyCache.end())
        return it->second;

    sal_Int32 idx = path.lastIndexOf("/");
    assert(idx!=-1);
    OUString parentPath = path.copy(0, idx);
    OUString childName = path.copy(idx+1);

    css::uno::Reference<css::container::XNameAccess> access(
        access_->getByHierarchicalName(parentPath), css::uno::UNO_QUERY_THROW);
    css::uno::Any property = access->getByName(childName);
    gPropertyCache.emplace(path, property);
    return property;
}

void comphelper::detail::ConfigurationWrapper::setPropertyValue(
    std::shared_ptr< ConfigurationChanges > const & batch,
    OUString const & path, css::uno::Any const & value)
{
    assert(batch);
    batch->setPropertyValue(path, value);
}

css::uno::Any
comphelper::detail::ConfigurationWrapper::getLocalizedPropertyValue(
    std::u16string_view path) const
{
    return access_->getByHierarchicalName(
        extendLocalizedPath(path, getDefaultLocale(context_)));
}

void comphelper::detail::ConfigurationWrapper::setLocalizedPropertyValue(
    std::shared_ptr< ConfigurationChanges > const & batch,
    OUString const & path, css::uno::Any const & value)
{
    assert(batch);
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
    assert(batch);
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
    assert(batch);
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
    for (auto const& listener : maListeners)
    {
        mxConfig->removePropertyChangeListener( listener->maName, this );
        listener->dispose();
    }
    maListeners.clear();
    mbDisposed = true;
}

void SAL_CALL comphelper::ConfigurationListener::disposing(css::lang::EventObject const &)
{
    dispose();
}

void SAL_CALL comphelper::ConfigurationListener::propertyChange(
    css::beans::PropertyChangeEvent const &rEvt )
{
    // Code is commonly used inside the SolarMutexGuard
    // so to avoid concurrent writes to the property,
    // and allow fast, lock-less access, guard here.
    //
    // Note that we are abusing rtl::Reference here to do acquire/release because,
    // unlike osl::Guard, it is tolerant of null pointers, and on some code paths, the
    // SolarMutex does not exist.
    rtl::Reference<comphelper::SolarMutex> xMutexGuard( comphelper::SolarMutex::get() );

    assert( rEvt.Source == mxConfig );
    for (auto const& listener : maListeners)
    {
        if ( listener->maName == rEvt.PropertyName )
        {
            // ignore rEvt.NewValue - in theory it could be stale => not set.
            css::uno::Any aValue = mxConfig->getPropertyValue( listener->maName );
            listener->setProperty( aValue );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
