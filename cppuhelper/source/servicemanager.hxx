/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CPPUHELPER_SOURCE_SERVICEMANAGER_HXX
#define INCLUDED_CPPUHELPER_SOURCE_SERVICEMANAGER_HXX

#include <sal/config.h>

#include <cassert>
#include <map>
#include <memory>
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
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>

namespace com { namespace sun { namespace star { namespace lang {
    class XSingleComponentFactory;
} } } }
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

typedef cppu::WeakComponentImplHelper<
    css::lang::XServiceInfo, css::lang::XMultiServiceFactory,
    css::lang::XMultiComponentFactory, css::container::XSet,
    css::container::XContentEnumerationAccess, css::beans::XPropertySet,
    css::beans::XPropertySetInfo, css::lang::XEventListener,
    css::lang::XInitialization>
ServiceManagerBase;

class ServiceManager:
    private cppu::BaseMutex, public ServiceManagerBase
{
public:
    struct Data {
        Data() = default;
        Data(const Data&) = delete;
        const Data& operator=(const Data&) = delete;

        struct ImplementationInfo {
            ImplementationInfo(
                OUString const & theName, OUString const & theLoader,
                OUString const & theUri,
                OUString const & theEnvironment,
                OUString const & theConstructor,
                OUString const & thePrefix,
                css::uno::Reference< css::uno::XComponentContext > const &
                    theAlienContext,
                OUString const & theRdbFile):
                name(theName), loader(theLoader), uri(theUri),
                environment(theEnvironment), constructor(theConstructor),
                prefix(thePrefix), alienContext(theAlienContext),
                rdbFile(theRdbFile)
            {}

            explicit ImplementationInfo(OUString const & theName):
                name(theName) {}

            ImplementationInfo(const ImplementationInfo&) = delete;
            const ImplementationInfo& operator=(const ImplementationInfo&) = delete;

            OUString const name;
            OUString const loader;
            OUString const uri;
            OUString const environment;
            OUString const constructor;
            OUString const prefix;
            css::uno::Reference< css::uno::XComponentContext > const
                alienContext;
            OUString const rdbFile;
            std::vector< OUString > services;
            std::vector< OUString > singletons;
        };

        struct Implementation {
            Implementation(
                OUString const & name, OUString const & loader,
                OUString const & uri, OUString const & environment,
                OUString const & constructorName,
                OUString const & prefix,
                css::uno::Reference< css::uno::XComponentContext > const &
                    alienContext,
                OUString const & rdbFile):
                info(
                    new ImplementationInfo(
                        name, loader, uri, environment, constructorName, prefix,
                        alienContext, rdbFile)),
                constructor(nullptr), status(STATUS_NEW), dispose(true)
            {}

            Implementation(
                OUString const & name,
                css::uno::Reference< css::lang::XSingleComponentFactory >
                    const & theFactory1,
                css::uno::Reference< css::lang::XSingleServiceFactory > const &
                    theFactory2,
                css::uno::Reference< css::lang::XComponent > const &
                    theComponent):
                info(new ImplementationInfo(name)), constructor(nullptr),
                factory1(theFactory1), factory2(theFactory2),
                component(theComponent), status(STATUS_LOADED), dispose(true)
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

            enum Status { STATUS_NEW, STATUS_WRAPPER, STATUS_LOADED };

            // Logically, exactly one of constructor, factory1, factory2 should
            // be set.  However, there are two exceptions:  For one, when
            // constructor is set, ServiceManager::createContentEnumeration will
            // store the necessary ImplementationWrapper in factory1 (so that
            // multiple calls to createContentEnumeration will return the same
            // wrapper).  For another, when factory1 should be set but status is
            // STATUS_NEW, factory1 is not yet set (and when status is
            // STATUS_WRAPPER, factory1 is merely set to an
            // ImplementationWrapper---also due to a
            // ServiceManager::createContentEnumeration call---and will be
            // loaded later).
            std::shared_ptr< ImplementationInfo > info;
            WrapperConstructorFn constructor;
            css::uno::Reference< css::lang::XSingleComponentFactory > factory1;
            css::uno::Reference< css::lang::XSingleServiceFactory > factory2;
            css::uno::Reference< css::lang::XComponent > component;
            Status status;

            osl::Mutex mutex;
            css::uno::Reference< css::lang::XComponent > disposeSingleton;
            bool dispose;

        private:
            void updateDisposeSingleton(
                bool singletonRequest,
                css::uno::Reference<css::uno::XInterface> const & instance);
        };

        typedef std::map< OUString, std::shared_ptr< Implementation > >
            NamedImplementations;

        typedef
            std::map<
                css::uno::Reference< css::lang::XServiceInfo >,
                std::shared_ptr< Implementation > >
            DynamicImplementations;

        typedef
            std::map<
                OUString,
                std::vector< std::shared_ptr< Implementation > > >
            ImplementationMap;

        NamedImplementations namedImplementations;
        DynamicImplementations dynamicImplementations;
        ImplementationMap services;
        ImplementationMap singletons;
    };

    ServiceManager(): ServiceManagerBase(m_aMutex) {}

    ServiceManager(const ServiceManager&) = delete;
    const ServiceManager& operator=(const ServiceManager&) = delete;

    using ServiceManagerBase::acquire;
    using ServiceManagerBase::release;

    void init(OUString const & rdbUris);

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

    virtual void SAL_CALL disposing() override;

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

    // needs to be called with rBHelper.rMutex locked:
    bool isDisposed() const { return rBHelper.bDisposed || rBHelper.bInDispose; }

    void removeEventListenerFromComponent(
        css::uno::Reference< css::lang::XComponent > const & component);

    void readRdbDirectory(OUString const & uri, bool optional);

    void readRdbFile(OUString const & uri, bool optional);

    bool readLegacyRdbFile(OUString const & uri);

    OUString readLegacyRdbString(
        OUString const & uri, RegistryKey & key,
        OUString const & path);

    void readLegacyRdbStrings(
        OUString const & uri, RegistryKey & key,
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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
