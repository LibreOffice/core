/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <cassert>
#include <functional>
#include <memory>
#include <mutex>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <compbase2.hxx>
#include <rtl/ustring.hxx>
#include <boost/container/small_vector.hpp>

namespace com::sun::star::lang {
    class XSingleComponentFactory;
}
namespace cppu { struct ContextEntry_Init; }
namespace com :: sun :: star :: lang { class XSingleServiceFactory; }
namespace com :: sun :: star :: uno { class XComponentContext; }

class RegistryKey;

namespace cppuhelper {

extern "C" {

typedef css::uno::XInterface * ImplementationConstructorFn(
    css::uno::XComponentContext *, css::uno::Sequence<css::uno::Any> const &);

}

typedef std::function<css::uno::XInterface * (css::uno::XComponentContext *, css::uno::Sequence<css::uno::Any> const&)> WrapperConstructorFn;

typedef WeakComponentImplHelper2<
    css::lang::XServiceInfo, css::lang::XMultiServiceFactory,
    css::lang::XMultiComponentFactory, css::container::XSet,
    css::container::XContentEnumerationAccess, css::beans::XPropertySet,
    css::beans::XPropertySetInfo, css::lang::XEventListener,
    css::lang::XInitialization>
ServiceManagerBase;

class ServiceManager : public ServiceManagerBase
{
public:
    struct Data {
        Data() = default;
        Data(const Data&) = delete;
        const Data& operator=(const Data&) = delete;

        struct Implementation {
            Implementation(
                OUString theName, OUString theLoader,
                OUString theUri, OUString theEnvironment,
                OUString theConstructorName,
                OUString thePrefix,
                bool theIsSingleInstance,
                css::uno::Reference< css::uno::XComponentContext > theAlienContext,
                OUString theRdbFile):
                name(std::move(theName)), loader(std::move(theLoader)), uri(std::move(theUri)), environment(std::move(theEnvironment)),
                constructorName(std::move(theConstructorName)), prefix(std::move(thePrefix)),
                isSingleInstance(theIsSingleInstance),
                alienContext(std::move(theAlienContext)), rdbFile(std::move(theRdbFile)),
                constructorFn(nullptr), status(STATUS_NEW), dispose(true)
            {}

            Implementation(
                OUString theName,
                css::uno::Reference< css::lang::XSingleComponentFactory >
                    const & theFactory1,
                css::uno::Reference< css::lang::XSingleServiceFactory > const &
                    theFactory2,
                css::uno::Reference< css::lang::XComponent > theComponent):
                name(std::move(theName)), isSingleInstance(false), constructorFn(nullptr),
                factory1(theFactory1), factory2(theFactory2),
                component(std::move(theComponent)), status(STATUS_LOADED), dispose(true)
            { assert(theFactory1.is() || theFactory2.is()); }

            Implementation(const Implementation&) = delete;
            const Implementation& operator=(const Implementation&) = delete;

            css::uno::Reference<css::uno::XInterface> createInstance(
                css::uno::Reference<css::uno::XComponentContext> const &
                    context,
                bool singletonRequest);

            css::uno::Reference<css::uno::XInterface>
            createInstanceWithArguments(
                css::uno::Reference<css::uno::XComponentContext> const &
                    context,
                bool singletonRequest,
                css::uno::Sequence<css::uno::Any> const & arguments);

            bool shallDispose() const { return isSingleInstance || !singletons.empty(); }

            enum Status { STATUS_NEW, STATUS_WRAPPER, STATUS_LOADED };

            // Logically, exactly one of constructorFn, factory1, factory2 should
            // be set.  However, there are two exceptions:  For one, when
            // constructorFn is set, ServiceManager::createContentEnumeration will
            // store the necessary ImplementationWrapper in factory1 (so that
            // multiple calls to createContentEnumeration will return the same
            // wrapper).  For another, when factory1 should be set but status is
            // STATUS_NEW, factory1 is not yet set (and when status is
            // STATUS_WRAPPER, factory1 is merely set to an
            // ImplementationWrapper---also due to a
            // ServiceManager::createContentEnumeration call---and will be
            // loaded later).
            OUString name;
            OUString loader;
            OUString uri;
            OUString environment;
            OUString constructorName;
            OUString prefix;
            bool isSingleInstance;
            css::uno::Reference< css::uno::XComponentContext > alienContext;
            OUString rdbFile;
            std::vector< OUString > services;
            std::vector< OUString > singletons;
            WrapperConstructorFn constructorFn;
            css::uno::Reference< css::lang::XSingleComponentFactory > factory1;
            css::uno::Reference< css::lang::XSingleServiceFactory > factory2;
            css::uno::Reference< css::lang::XComponent > component;
            Status status;

            std::mutex mutex;
            css::uno::Reference<css::uno::XInterface> singleInstance;
            css::uno::Reference< css::lang::XComponent > disposeInstance;
            bool dispose;

        private:
            css::uno::Reference<css::uno::XInterface> doCreateInstance(
                css::uno::Reference<css::uno::XComponentContext> const & context);

            css::uno::Reference<css::uno::XInterface> doCreateInstanceWithArguments(
                css::uno::Reference<css::uno::XComponentContext> const & context,
                css::uno::Sequence<css::uno::Any> const & arguments);

            void updateDisposeInstance(
                bool singletonRequest,
                css::uno::Reference<css::uno::XInterface> const & instance);
        };

        typedef std::unordered_map< OUString, std::shared_ptr< Implementation > >
            NamedImplementations;

        typedef
            std::unordered_map<
                css::uno::Reference< css::lang::XServiceInfo >,
                std::shared_ptr< Implementation > >
            DynamicImplementations;

        typedef
            std::unordered_map<
                OUString,
                boost::container::small_vector< std::shared_ptr< Implementation >, 2 > >
            ImplementationMap;

        NamedImplementations namedImplementations;
        DynamicImplementations dynamicImplementations;
        ImplementationMap services;
        ImplementationMap singletons;
    };

    ServiceManager() {}

    ServiceManager(const ServiceManager&) = delete;
    const ServiceManager& operator=(const ServiceManager&) = delete;

    using ServiceManagerBase::acquire;
    using ServiceManagerBase::release;

    void init(std::u16string_view rdbUris);

    void setContext(
        css::uno::Reference< css::uno::XComponentContext > const & context)
    {
        assert(context.is());
        assert(!context_.is());
        context_ = context;
    }

    void addSingletonContextEntries(
        std::vector< cppu::ContextEntry_Init > * entries);

    css::uno::Reference< css::uno::XComponentContext > const & getContext()
        const
    {
        assert(context_.is());
        return context_;
    }

    void loadImplementation(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        std::shared_ptr< Data::Implementation > const & implementation);

private:
    virtual ~ServiceManager() override;

    virtual void disposing(std::unique_lock<std::mutex>&) override;

    virtual OUString SAL_CALL getImplementationName() override;

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override;

    virtual css::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames() override;

    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance(
        OUString const & aServiceSpecifier) override;

    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL
    createInstanceWithArguments(
        OUString const & ServiceSpecifier,
        css::uno::Sequence< css::uno::Any > const & Arguments) override;

    virtual css::uno::Sequence< OUString > SAL_CALL
    getAvailableServiceNames() override;

    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL
    createInstanceWithContext(
        OUString const & aServiceSpecifier,
        css::uno::Reference< css::uno::XComponentContext > const & Context) override;

    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL
    createInstanceWithArgumentsAndContext(
        OUString const & ServiceSpecifier,
        css::uno::Sequence< css::uno::Any > const & Arguments,
        css::uno::Reference< css::uno::XComponentContext > const & Context) override;

    virtual css::uno::Type SAL_CALL getElementType() override;

    virtual sal_Bool SAL_CALL hasElements() override;

    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
    createEnumeration() override;

    virtual sal_Bool SAL_CALL has(css::uno::Any const & aElement) override;

    virtual void SAL_CALL insert(css::uno::Any const & aElement) override;

    virtual void SAL_CALL remove(css::uno::Any const & aElement) override;

    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
    createContentEnumeration(OUString const & aServiceName) override;

    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
    getPropertySetInfo() override;

    virtual void SAL_CALL setPropertyValue(
        OUString const & aPropertyName, css::uno::Any const & aValue) override;

    virtual css::uno::Any SAL_CALL getPropertyValue(
        OUString const & PropertyName) override;

    virtual void SAL_CALL addPropertyChangeListener(
        OUString const & aPropertyName,
        css::uno::Reference< css::beans::XPropertyChangeListener > const &
            xListener) override;

    virtual void SAL_CALL removePropertyChangeListener(
        OUString const & aPropertyName,
        css::uno::Reference< css::beans::XPropertyChangeListener > const &
            aListener) override;

    virtual void SAL_CALL addVetoableChangeListener(
        OUString const & PropertyName,
        css::uno::Reference< css::beans::XVetoableChangeListener > const &
            aListener) override;

    virtual void SAL_CALL removeVetoableChangeListener(
        OUString const & PropertyName,
        css::uno::Reference< css::beans::XVetoableChangeListener > const &
            aListener) override;

    virtual css::uno::Sequence< css::beans::Property > SAL_CALL getProperties() override;

    virtual css::beans::Property SAL_CALL getPropertyByName(
        OUString const & aName) override;

    virtual sal_Bool SAL_CALL hasPropertyByName(OUString const & Name) override;

    virtual void SAL_CALL disposing(css::lang::EventObject const & Source) override;

    virtual void SAL_CALL initialize(
        css::uno::Sequence<css::uno::Any> const & aArguments)
        override;

    void removeEventListenerFromComponent(
        css::uno::Reference< css::lang::XComponent > const & component);

    void readRdbDirectory(std::u16string_view uri, bool optional);

    void readRdbFile(OUString const & uri, bool optional);

    bool readLegacyRdbFile(OUString const & uri);

    OUString readLegacyRdbString(
        std::u16string_view uri, RegistryKey & key,
        OUString const & path);

    void readLegacyRdbStrings(
        std::u16string_view uri, RegistryKey & key,
        OUString const & path, std::vector< OUString > * strings);

    void insertRdbFiles(
        std::vector< OUString > const & uris,
        css::uno::Reference< css::uno::XComponentContext > const &
            alientContext);

    void insertLegacyFactory(
        css::uno::Reference< css::lang::XServiceInfo > const & factoryInfo);

    bool insertExtraData(Data const & extra);

    void removeRdbFiles(std::vector< OUString > const & uris);

    bool removeLegacyFactory(
        css::uno::Reference< css::lang::XServiceInfo > const & factoryInfo,
        bool removeListener);

    void removeImplementation(const OUString & name);

    std::shared_ptr< Data::Implementation > findServiceImplementation(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        OUString const & specifier);

    void preloadImplementations();

    css::uno::Reference< css::uno::XComponentContext > context_;
    Data data_;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
