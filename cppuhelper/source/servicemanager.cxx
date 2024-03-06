/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <algorithm>
#include <cassert>
#include <iostream>
#include <mutex>
#include <string_view>
#include <utility>
#include <vector>

#include <config_fuzzers.h>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/container/ElementExistException.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/loader/XImplementationLoader.hpp>
#include <com/sun/star/registry/InvalidRegistryException.hpp>
#include <com/sun/star/uno/DeploymentException.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/sequence.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/component_context.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/factory.hxx>
#include <o3tl/safeint.hxx>
#include <osl/file.hxx>
#include <osl/module.hxx>
#include <rtl/ref.hxx>
#include <rtl/uri.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <uno/environment.hxx>
#include <uno/mapping.hxx>
#include <o3tl/string_view.hxx>

#include "loadsharedlibcomponentfactory.hxx"

#include <registry/registry.hxx>
#include <xmlreader/xmlreader.hxx>

#include "paths.hxx"
#include "servicemanager.hxx"

namespace {

void insertImplementationMap(
    cppuhelper::ServiceManager::Data::ImplementationMap * destination,
    cppuhelper::ServiceManager::Data::ImplementationMap const & source)
{
    assert(destination != nullptr);
    for (const auto& [rName, rImpls] : source)
    {
        std::vector<
            std::shared_ptr<
                cppuhelper::ServiceManager::Data::Implementation > > & impls
            = (*destination)[rName];
        impls.insert(impls.end(), rImpls.begin(), rImpls.end());
    }
}

void removeFromImplementationMap(
    cppuhelper::ServiceManager::Data::ImplementationMap * map,
    std::vector< OUString > const & elements,
    std::shared_ptr< cppuhelper::ServiceManager::Data::Implementation >
        const & implementation)
{
    // The underlying data structures make this function somewhat inefficient,
    // but the assumption is that it is rarely called:
    assert(map != nullptr);
    for (const auto& rElement : elements)
    {
        cppuhelper::ServiceManager::Data::ImplementationMap::iterator j(
            map->find(rElement));
        assert(j != map->end());
        std::vector<
            std::shared_ptr<
                cppuhelper::ServiceManager::Data::Implementation > >::iterator
            k(std::find(j->second.begin(), j->second.end(), implementation));
        assert(k != j->second.end());
        j->second.erase(k);
        if (j->second.empty()) {
            map->erase(j);
        }
    }
}

// For simplicity, this code keeps throwing
// css::registry::InvalidRegistryException for invalid XML rdbs (even though
// that does not fit the exception's name):
class Parser {
public:
    Parser(
        OUString const & uri,
        css::uno::Reference< css::uno::XComponentContext > alienContext,
        cppuhelper::ServiceManager::Data * data);

    Parser(const Parser&) = delete;
    const Parser& operator=(const Parser&) = delete;

private:
    void handleComponent();

    void handleImplementation();

    void handleService();

    void handleSingleton();

    OUString getNameAttribute();

    xmlreader::XmlReader reader_;
    css::uno::Reference< css::uno::XComponentContext > alienContext_;
    cppuhelper::ServiceManager::Data * data_;
    OUString attrLoader_;
    OUString attrUri_;
    OUString attrEnvironment_;
    OUString attrPrefix_;
    std::shared_ptr< cppuhelper::ServiceManager::Data::Implementation >
        implementation_;
};

Parser::Parser(
    OUString const & uri,
    css::uno::Reference< css::uno::XComponentContext > alienContext,
    cppuhelper::ServiceManager::Data * data):
    reader_(uri), alienContext_(std::move(alienContext)), data_(data)
{
    assert(data != nullptr);
    int ucNsId = reader_.registerNamespaceIri(
        xmlreader::Span(
            RTL_CONSTASCII_STRINGPARAM(
                "http://openoffice.org/2010/uno-components")));
    enum State {
        STATE_BEGIN, STATE_END, STATE_COMPONENTS, STATE_COMPONENT_INITIAL,
        STATE_COMPONENT, STATE_IMPLEMENTATION, STATE_SERVICE, STATE_SINGLETON };
    for (State state = STATE_BEGIN;;) {
        xmlreader::Span name;
        int nsId;
        xmlreader::XmlReader::Result res = reader_.nextItem(
            xmlreader::XmlReader::Text::NONE, &name, &nsId);
        switch (state) {
        case STATE_BEGIN:
            if (res == xmlreader::XmlReader::Result::Begin && nsId == ucNsId
                && name.equals(RTL_CONSTASCII_STRINGPARAM("components")))
            {
                state = STATE_COMPONENTS;
                break;
            }
            throw css::registry::InvalidRegistryException(
                reader_.getUrl() + ": unexpected item in outer level");
        case STATE_END:
            if (res == xmlreader::XmlReader::Result::Done) {
                return;
            }
            throw css::registry::InvalidRegistryException(
                reader_.getUrl() + ": unexpected item in outer level");
        case STATE_COMPONENTS:
            if (res == xmlreader::XmlReader::Result::End) {
                state = STATE_END;
                break;
            }
            if (res == xmlreader::XmlReader::Result::Begin && nsId == ucNsId
                && name.equals(RTL_CONSTASCII_STRINGPARAM("component")))
            {
                handleComponent();
                state = STATE_COMPONENT_INITIAL;
                break;
            }
            throw css::registry::InvalidRegistryException(
                reader_.getUrl() + ": unexpected item in <components>");
        case STATE_COMPONENT:
            if (res == xmlreader::XmlReader::Result::End) {
                state = STATE_COMPONENTS;
                break;
            }
            [[fallthrough]];
        case STATE_COMPONENT_INITIAL:
            if (res == xmlreader::XmlReader::Result::Begin && nsId == ucNsId
                && name.equals(RTL_CONSTASCII_STRINGPARAM("implementation")))
            {
                handleImplementation();
                state = STATE_IMPLEMENTATION;
                break;
            }
            throw css::registry::InvalidRegistryException(
                reader_.getUrl() + ": unexpected item in <component>");
        case STATE_IMPLEMENTATION:
            if (res == xmlreader::XmlReader::Result::End) {
                state = STATE_COMPONENT;
                break;
            }
            if (res == xmlreader::XmlReader::Result::Begin && nsId == ucNsId
                && name.equals(RTL_CONSTASCII_STRINGPARAM("service")))
            {
                handleService();
                state = STATE_SERVICE;
                break;
            }
            if (res == xmlreader::XmlReader::Result::Begin && nsId == ucNsId
                && name.equals(RTL_CONSTASCII_STRINGPARAM("singleton")))
            {
                handleSingleton();
                state = STATE_SINGLETON;
                break;
            }
            throw css::registry::InvalidRegistryException(
                reader_.getUrl() + ": unexpected item in <implementation>");
        case STATE_SERVICE:
            if (res == xmlreader::XmlReader::Result::End) {
                state = STATE_IMPLEMENTATION;
                break;
            }
            throw css::registry::InvalidRegistryException(
                reader_.getUrl() + ": unexpected item in <service>");
        case STATE_SINGLETON:
            if (res == xmlreader::XmlReader::Result::End) {
                state = STATE_IMPLEMENTATION;
                break;
            }
            throw css::registry::InvalidRegistryException(
                reader_.getUrl() + ": unexpected item in <service>");
        }
    }
}

void Parser::handleComponent() {
    attrLoader_ = OUString();
    attrUri_ = OUString();
    attrEnvironment_ = OUString();
    attrPrefix_ = OUString();
    xmlreader::Span name;
    int nsId;
    while (reader_.nextAttribute(&nsId, &name)) {
        if (nsId == xmlreader::XmlReader::NAMESPACE_NONE
            && name.equals(RTL_CONSTASCII_STRINGPARAM("loader")))
        {
            if (!attrLoader_.isEmpty()) {
                throw css::registry::InvalidRegistryException(
                    reader_.getUrl()
                     + ": <component> has multiple \"loader\" attributes");
            }
            attrLoader_ = reader_.getAttributeValue(false).convertFromUtf8();
            if (attrLoader_.isEmpty()) {
                throw css::registry::InvalidRegistryException(
                    reader_.getUrl()
                     + ": <component> has empty \"loader\" attribute");
            }
        } else if (nsId == xmlreader::XmlReader::NAMESPACE_NONE
                   && name.equals(RTL_CONSTASCII_STRINGPARAM("uri")))
        {
            if (!attrUri_.isEmpty()) {
                throw css::registry::InvalidRegistryException(
                    reader_.getUrl()
                     + ": <component> has multiple \"uri\" attributes");
            }
            attrUri_ = reader_.getAttributeValue(false).convertFromUtf8();
            if (attrUri_.isEmpty()) {
                throw css::registry::InvalidRegistryException(
                    reader_.getUrl()
                     + ": <component> has empty \"uri\" attribute");
            }
        } else if (nsId == xmlreader::XmlReader::NAMESPACE_NONE
                   && name.equals(RTL_CONSTASCII_STRINGPARAM("environment")))
        {
            if (!attrEnvironment_.isEmpty()) {
                throw css::registry::InvalidRegistryException(
                    reader_.getUrl() +
                     ": <component> has multiple \"environment\" attributes");
            }
            attrEnvironment_ = reader_.getAttributeValue(false)
                .convertFromUtf8();
            if (attrEnvironment_.isEmpty()) {
                throw css::registry::InvalidRegistryException(
                    reader_.getUrl() +
                     ": <component> has empty \"environment\" attribute");
            }
        } else if (nsId == xmlreader::XmlReader::NAMESPACE_NONE
                   && name.equals(RTL_CONSTASCII_STRINGPARAM("prefix")))
        {
            if (!attrPrefix_.isEmpty()) {
                throw css::registry::InvalidRegistryException(
                    reader_.getUrl() +
                     ": <component> has multiple \"prefix\" attributes");
            }
            attrPrefix_ = reader_.getAttributeValue(false).convertFromUtf8();
            if (attrPrefix_.isEmpty()) {
                throw css::registry::InvalidRegistryException(
                    reader_.getUrl() +
                     ": <component> has empty \"prefix\" attribute");
            }
        } else {
            throw css::registry::InvalidRegistryException(
                reader_.getUrl() + ": unexpected attribute \""
                 + name.convertFromUtf8() + "\" in <component>");
        }
    }
    if (attrLoader_.isEmpty()) {
        throw css::registry::InvalidRegistryException(
            reader_.getUrl() + ": <component> is missing \"loader\" attribute");
    }
    if (attrUri_.isEmpty()) {
        throw css::registry::InvalidRegistryException(
            reader_.getUrl() + ": <component> is missing \"uri\" attribute");
    }
#ifndef DISABLE_DYNLOADING
    try {
        attrUri_ = rtl::Uri::convertRelToAbs(reader_.getUrl(), attrUri_);
    } catch (const rtl::MalformedUriException & e) {
        throw css::registry::InvalidRegistryException(
            reader_.getUrl() + ": bad \"uri\" attribute: " + e.getMessage());
    }
#endif
}

void Parser::handleImplementation() {
    OUString attrName;
    OUString attrConstructor;
    bool attrSingleInstance = false;
    xmlreader::Span name;
    int nsId;
    while (reader_.nextAttribute(&nsId, &name)) {
        if (nsId == xmlreader::XmlReader::NAMESPACE_NONE
            && name.equals(RTL_CONSTASCII_STRINGPARAM("name")))
        {
            if (!attrName.isEmpty()) {
                throw css::registry::InvalidRegistryException(
                    reader_.getUrl()
                     + ": <implementation> has multiple \"name\" attributes");
            }
            attrName = reader_.getAttributeValue(false).convertFromUtf8();
            if (attrName.isEmpty()) {
                throw css::registry::InvalidRegistryException(
                    reader_.getUrl()
                    + ": <implementation> has empty \"name\" attribute");
            }
        } else if (nsId == xmlreader::XmlReader::NAMESPACE_NONE
                   && name.equals(RTL_CONSTASCII_STRINGPARAM("constructor")))
        {
            if (!attrConstructor.isEmpty()) {
                throw css::registry::InvalidRegistryException(
                    reader_.getUrl()
                     + ": <implementation> has multiple \"constructor\""
                        " attributes");
            }
            attrConstructor = reader_.getAttributeValue(false)
                .convertFromUtf8();
            if (attrConstructor.isEmpty()) {
                throw css::registry::InvalidRegistryException(
                    reader_.getUrl()
                     + ": element has empty \"constructor\" attribute");
            }
            if (attrEnvironment_.isEmpty()) {
                throw css::registry::InvalidRegistryException(
                    reader_.getUrl()
                     + ": <implementation> has \"constructor\" attribute but"
                        " <component> has no \"environment\" attribute");
            }
        } else if (nsId == xmlreader::XmlReader::NAMESPACE_NONE
                   && name.equals(RTL_CONSTASCII_STRINGPARAM("single-instance")))
        {
            if (attrSingleInstance) {
                throw css::registry::InvalidRegistryException(
                    reader_.getUrl()
                    + ": <implementation> has multiple \"single-instance\" attributes");
            }
            if (!reader_.getAttributeValue(false).equals(RTL_CONSTASCII_STRINGPARAM("true"))) {
                throw css::registry::InvalidRegistryException(
                    reader_.getUrl() + ": <implementation> has bad \"single-instance\" attribute");
            }
            attrSingleInstance = true;
        } else {
            throw css::registry::InvalidRegistryException(
                reader_.getUrl() + ": unexpected element attribute \""
                 + name.convertFromUtf8() + "\" in <implementation>");
        }
    }
    if (attrName.isEmpty()) {
        throw css::registry::InvalidRegistryException(
            reader_.getUrl()
             + ": <implementation> is missing \"name\" attribute");
    }
    implementation_ =
        std::make_shared<cppuhelper::ServiceManager::Data::Implementation>(
            attrName, attrLoader_, attrUri_, attrEnvironment_, attrConstructor,
            attrPrefix_, attrSingleInstance, alienContext_, reader_.getUrl());
    if (!data_->namedImplementations.emplace(attrName, implementation_).
        second)
    {
        throw css::registry::InvalidRegistryException(
            reader_.getUrl() + ": duplicate <implementation name=\"" + attrName
             + "\">");
    }
}

void Parser::handleService() {
    OUString name(getNameAttribute());
    implementation_->services.push_back(name);
    data_->services[name].push_back(implementation_);
}

void Parser::handleSingleton() {
    OUString name(getNameAttribute());
    implementation_->singletons.push_back(name);
    data_->singletons[name].push_back(implementation_);
}

OUString Parser::getNameAttribute() {
    OUString attrName;
    xmlreader::Span name;
    int nsId;
    while (reader_.nextAttribute(&nsId, &name)) {
        if (nsId != xmlreader::XmlReader::NAMESPACE_NONE
            || !name.equals(RTL_CONSTASCII_STRINGPARAM("name")))
        {
            throw css::registry::InvalidRegistryException(
                reader_.getUrl() + ": expected element attribute \"name\"");
        }
        if (!attrName.isEmpty()) {
            throw css::registry::InvalidRegistryException(
                reader_.getUrl()
                 + ": element has multiple \"name\" attributes");
        }
        attrName = reader_.getAttributeValue(false).convertFromUtf8();
        if (attrName.isEmpty()) {
            throw css::registry::InvalidRegistryException(
                reader_.getUrl() + ": element has empty \"name\" attribute");
        }
    }
    if (attrName.isEmpty()) {
        throw css::registry::InvalidRegistryException(
            reader_.getUrl() + ": element is missing \"name\" attribute");
    }
    return attrName;
}

class ContentEnumeration:
    public cppu::WeakImplHelper< css::container::XEnumeration >
{
public:
    explicit ContentEnumeration(std::vector< css::uno::Any >&& factories):
        factories_(std::move(factories)), iterator_(factories_.begin()) {}

    ContentEnumeration(const ContentEnumeration&) = delete;
    const ContentEnumeration& operator=(const ContentEnumeration&) = delete;

private:
    virtual ~ContentEnumeration() override {}

    virtual sal_Bool SAL_CALL hasMoreElements() override;

    virtual css::uno::Any SAL_CALL nextElement() override;

    std::mutex mutex_;
    std::vector< css::uno::Any > factories_;
    std::vector< css::uno::Any >::const_iterator iterator_;
};

sal_Bool ContentEnumeration::hasMoreElements()
{
    std::scoped_lock g(mutex_);
    return iterator_ != factories_.end();
}

css::uno::Any ContentEnumeration::nextElement()
{
    std::scoped_lock g(mutex_);
    if (iterator_ == factories_.end()) {
        throw css::container::NoSuchElementException(
            "Bootstrap service manager service enumerator has no more elements",
            static_cast< cppu::OWeakObject * >(this));
    }
    return *iterator_++;
}

css::beans::Property getDefaultContextProperty() {
    return css::beans::Property(
        "DefaultContext", -1,
        cppu::UnoType< css::uno::XComponentContext >::get(),
        css::beans::PropertyAttribute::READONLY);
}

class SingletonFactory:
    public cppu::WeakImplHelper<css::lang::XSingleComponentFactory>
{
public:
    SingletonFactory(
        rtl::Reference< cppuhelper::ServiceManager > const & manager,
        std::shared_ptr<
            cppuhelper::ServiceManager::Data::Implementation > const &
            implementation):
        manager_(manager), implementation_(implementation)
    { assert(manager.is()); assert(implementation); }

    SingletonFactory(const SingletonFactory&) = delete;
    const SingletonFactory& operator=(const SingletonFactory&) = delete;

private:
    virtual ~SingletonFactory() override {}

    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL
    createInstanceWithContext(
        css::uno::Reference< css::uno::XComponentContext > const & Context) override;

    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL
    createInstanceWithArgumentsAndContext(
        css::uno::Sequence< css::uno::Any > const & Arguments,
        css::uno::Reference< css::uno::XComponentContext > const & Context) override;

    rtl::Reference< cppuhelper::ServiceManager > manager_;
    std::shared_ptr< cppuhelper::ServiceManager::Data::Implementation >
        implementation_;
};

css::uno::Reference< css::uno::XInterface >
SingletonFactory::createInstanceWithContext(
    css::uno::Reference< css::uno::XComponentContext > const & Context)
{
    manager_->loadImplementation(Context, implementation_);
    return implementation_->createInstance(Context, true);
}

css::uno::Reference< css::uno::XInterface >
SingletonFactory::createInstanceWithArgumentsAndContext(
    css::uno::Sequence< css::uno::Any > const & Arguments,
    css::uno::Reference< css::uno::XComponentContext > const & Context)
{
    manager_->loadImplementation(Context, implementation_);
    return implementation_->createInstanceWithArguments(
        Context, true, Arguments);
}

class ImplementationWrapper:
    public cppu::WeakImplHelper<
        css::lang::XSingleComponentFactory, css::lang::XSingleServiceFactory,
        css::lang::XServiceInfo >
{
public:
    ImplementationWrapper(
        rtl::Reference< cppuhelper::ServiceManager > const & manager,
        std::shared_ptr<
            cppuhelper::ServiceManager::Data::Implementation > const &
            implementation):
        manager_(manager), implementation_(implementation)
    { assert(manager.is()); assert(implementation); }

    ImplementationWrapper(const ImplementationWrapper&) = delete;
    const ImplementationWrapper& operator=(const ImplementationWrapper&) = delete;

private:
    virtual ~ImplementationWrapper() override {}

    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL
    createInstanceWithContext(
        css::uno::Reference< css::uno::XComponentContext > const & Context) override;

    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL
    createInstanceWithArgumentsAndContext(
        css::uno::Sequence< css::uno::Any > const & Arguments,
        css::uno::Reference< css::uno::XComponentContext > const & Context) override;

    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL
    createInstance() override;

    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL
    createInstanceWithArguments(
        css::uno::Sequence< css::uno::Any > const & Arguments) override;

    virtual OUString SAL_CALL getImplementationName() override;

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override;

    virtual css::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames() override;

    rtl::Reference< cppuhelper::ServiceManager > manager_;
    std::weak_ptr< cppuhelper::ServiceManager::Data::Implementation >
        implementation_;
};

css::uno::Reference< css::uno::XInterface >
ImplementationWrapper::createInstanceWithContext(
    css::uno::Reference< css::uno::XComponentContext > const & Context)
{
    std::shared_ptr< cppuhelper::ServiceManager::Data::Implementation > impl = implementation_.lock();
    assert(impl);
    manager_->loadImplementation(Context, impl);
    return impl->createInstance(Context, false);
}

css::uno::Reference< css::uno::XInterface >
ImplementationWrapper::createInstanceWithArgumentsAndContext(
    css::uno::Sequence< css::uno::Any > const & Arguments,
    css::uno::Reference< css::uno::XComponentContext > const & Context)
{
    std::shared_ptr< cppuhelper::ServiceManager::Data::Implementation > impl = implementation_.lock();
    assert(impl);
    manager_->loadImplementation(Context, impl);
    return impl->createInstanceWithArguments(
        Context, false, Arguments);
}

css::uno::Reference< css::uno::XInterface >
ImplementationWrapper::createInstance()
{
    return createInstanceWithContext(manager_->getContext());
}

css::uno::Reference< css::uno::XInterface >
ImplementationWrapper::createInstanceWithArguments(
    css::uno::Sequence< css::uno::Any > const & Arguments)
{
    return createInstanceWithArgumentsAndContext(
        Arguments, manager_->getContext());
}

OUString ImplementationWrapper::getImplementationName()
{
    std::shared_ptr< cppuhelper::ServiceManager::Data::Implementation > impl = implementation_.lock();
    assert(impl);
    return impl->name;
}

sal_Bool ImplementationWrapper::supportsService(OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence< OUString >
ImplementationWrapper::getSupportedServiceNames()
{
    std::shared_ptr< cppuhelper::ServiceManager::Data::Implementation > impl = implementation_.lock();
    assert(impl);
    if (impl->services.size()
        > o3tl::make_unsigned(SAL_MAX_INT32))
    {
        throw css::uno::RuntimeException(
            ("Implementation " + impl->name
             + " supports too many services"),
            static_cast< cppu::OWeakObject * >(this));
    }
    return comphelper::containerToSequence(impl->services);
}

}

css::uno::Reference<css::uno::XInterface>
cppuhelper::ServiceManager::Data::Implementation::createInstance(
    css::uno::Reference<css::uno::XComponentContext> const & context,
    bool singletonRequest)
{
    css::uno::Reference<css::uno::XInterface> inst;
    if (isSingleInstance) {
        std::unique_lock g(mutex);
        if (!singleInstance.is()) {
            singleInstance = doCreateInstance(context);
        }
        inst = singleInstance;
    } else {
        inst = doCreateInstance(context);
    }
    updateDisposeInstance(singletonRequest, inst);
    return inst;
}

css::uno::Reference<css::uno::XInterface>
cppuhelper::ServiceManager::Data::Implementation::createInstanceWithArguments(
    css::uno::Reference<css::uno::XComponentContext> const & context,
    bool singletonRequest, css::uno::Sequence<css::uno::Any> const & arguments)
{
    css::uno::Reference<css::uno::XInterface> inst;
    if (isSingleInstance) {
        std::unique_lock g(mutex);
        if (!singleInstance.is()) {
            singleInstance = doCreateInstanceWithArguments(context, arguments);
        }
        inst = singleInstance;
    } else {
        inst = doCreateInstanceWithArguments(context, arguments);
    }
    updateDisposeInstance(singletonRequest, inst);
    return inst;
}

css::uno::Reference<css::uno::XInterface>
cppuhelper::ServiceManager::Data::Implementation::doCreateInstance(
    css::uno::Reference<css::uno::XComponentContext> const & context)
{
    if (constructorFn) {
        return css::uno::Reference<css::uno::XInterface>(
            constructorFn(context.get(), css::uno::Sequence<css::uno::Any>()),
            SAL_NO_ACQUIRE);
    } else if (factory1.is()) {
        return factory1->createInstanceWithContext(context);
    } else {
        assert(factory2.is());
        return factory2->createInstance();
    }
}

css::uno::Reference<css::uno::XInterface>
cppuhelper::ServiceManager::Data::Implementation::doCreateInstanceWithArguments(
    css::uno::Reference<css::uno::XComponentContext> const & context,
    css::uno::Sequence<css::uno::Any> const & arguments)
{
    if (constructorFn) {
        css::uno::Reference<css::uno::XInterface> inst(
            constructorFn(context.get(), arguments), SAL_NO_ACQUIRE);
        //HACK: The constructor will either observe arguments and return inst
        // that does not implement XInitialization (or null), or ignore
        // arguments and return inst that implements XInitialization; this
        // should be removed again once XInitialization-based implementations
        // have become rare:
        css::uno::Reference<css::lang::XInitialization> init(
            inst, css::uno::UNO_QUERY);
        if (init.is()) {
            init->initialize(arguments);
        }
        return inst;
    } else if (factory1.is()) {
        return factory1->createInstanceWithArgumentsAndContext(
            arguments, context);
    } else {
        assert(factory2.is());
        return factory2->createInstanceWithArguments(arguments);
    }
}

void cppuhelper::ServiceManager::Data::Implementation::updateDisposeInstance(
    bool singletonRequest,
    css::uno::Reference<css::uno::XInterface> const & instance)
{
    // This is an optimization, to only call dispose once (from the component
    // context) on a singleton that is obtained both via the component context
    // and via the service manager; however, there is a harmless race here that
    // may cause two calls to dispose nevertheless (also, this calls dispose on
    // at most one of the instances obtained via the service manager, in case
    // the implementation hands out different instances):
    if (singletonRequest) {
        std::unique_lock g(mutex);
        disposeInstance.clear();
        dispose = false;
    } else if (shallDispose()) {
        css::uno::Reference<css::lang::XComponent> comp(
            instance, css::uno::UNO_QUERY);
        if (comp.is()) {
            std::unique_lock g(mutex);
            if (dispose) {
                disposeInstance = comp;
            }
        }
    }
}

void cppuhelper::ServiceManager::addSingletonContextEntries(
    std::vector< cppu::ContextEntry_Init > * entries)
{
    assert(entries != nullptr);
    for (const auto& [rName, rImpls] : data_.singletons)
    {
        assert(!rImpls.empty());
        assert(rImpls[0]);
        SAL_INFO_IF(
            rImpls.size() > 1, "cppuhelper",
            "Arbitrarily choosing " << rImpls[0]->name
                << " among multiple implementations for " << rName);
        entries->push_back(
            cppu::ContextEntry_Init(
                "/singletons/" + rName,
                css::uno::Any(
                    css::uno::Reference<css::lang::XSingleComponentFactory>(
                        new SingletonFactory(this, rImpls[0]))),
                true));
    }
}

void cppuhelper::ServiceManager::loadImplementation(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        std::shared_ptr< Data::Implementation > const & implementation)
{
    assert(implementation);
    {
        std::unique_lock g(m_aMutex);
        if (implementation->status == Data::Implementation::STATUS_LOADED) {
            return;
        }
    }
    OUString uri;
    try {
        uri = cppu::bootstrap_expandUri(implementation->uri);
    } catch (css::lang::IllegalArgumentException & e) {
        throw css::uno::DeploymentException(
            "Cannot expand URI" + implementation->uri + ": " + e.Message,
            static_cast< cppu::OWeakObject * >(this));
    }
    cppuhelper::WrapperConstructorFn ctor;
    css::uno::Reference< css::uno::XInterface > f0;
    // Special handling of SharedLibrary loader, with support for environment,
    // constructor, and prefix arguments:
    if (!implementation->alienContext.is()
        && implementation->loader == "com.sun.star.loader.SharedLibrary")
    {
        cppuhelper::detail::loadSharedLibComponentFactory(
            uri, implementation->environment,
            implementation->prefix, implementation->name,
            implementation->constructorName, this, &ctor, &f0);
        if (ctor) {
            assert(!implementation->environment.isEmpty());
        }
    } else {
        SAL_WARN_IF(
            !implementation->environment.isEmpty(), "cppuhelper",
            "Loader " << implementation->loader
                << " and non-empty environment "
                << implementation->environment);
        SAL_WARN_IF(
            !implementation->prefix.isEmpty(), "cppuhelper",
            "Loader " << implementation->loader
                << " and non-empty constructor "
                << implementation->constructorName);
        SAL_WARN_IF(
            !implementation->prefix.isEmpty(), "cppuhelper",
            "Loader " << implementation->loader
                << " and non-empty prefix " << implementation->prefix);
        css::uno::Reference< css::uno::XComponentContext > ctxt;
        css::uno::Reference< css::lang::XMultiComponentFactory > smgr;
        if (implementation->alienContext.is()) {
            ctxt = implementation->alienContext;
            smgr.set(ctxt->getServiceManager(), css::uno::UNO_SET_THROW);
        } else {
            assert(context.is());
            ctxt = context;
            smgr = this;
        }
        css::uno::Reference< css::loader::XImplementationLoader > loader(
            smgr->createInstanceWithContext(implementation->loader, ctxt),
            css::uno::UNO_QUERY_THROW);
        f0 = loader->activate(
            implementation->name, OUString(), uri,
            css::uno::Reference< css::registry::XRegistryKey >());
    }
    css::uno::Reference<css::lang::XSingleComponentFactory> f1;
    css::uno::Reference<css::lang::XSingleServiceFactory> f2;
    if (!ctor) {
        f1.set(f0, css::uno::UNO_QUERY);
        if (!f1.is()) {
            f2.set(f0, css::uno::UNO_QUERY);
            if (!f2.is()) {
                throw css::uno::DeploymentException(
                    ("Implementation " + implementation->name
                     + " does not provide a constructor or factory"),
                    static_cast< cppu::OWeakObject * >(this));
            }
        }
    }
    //TODO: There is a race here, as the relevant service factory can be removed
    // while the mutex is unlocked and loading can thus fail, as the entity from
    // which to load can disappear once the service factory is removed.
    std::unique_lock g(m_aMutex);
    if (!(m_bDisposed
          || implementation->status == Data::Implementation::STATUS_LOADED))
    {
        implementation->status = Data::Implementation::STATUS_LOADED;
        implementation->constructorFn = ctor;
        implementation->factory1 = f1;
        implementation->factory2 = f2;
    }
}

void cppuhelper::ServiceManager::disposing(std::unique_lock<std::mutex>& rGuard) {
    std::vector< css::uno::Reference<css::lang::XComponent> > sngls;
    std::vector< css::uno::Reference< css::lang::XComponent > > comps;
    Data clear;
    {
        for (const auto& rEntry : data_.namedImplementations)
        {
            assert(rEntry.second);
            if (rEntry.second->shallDispose()) {
                std::unique_lock g2(rEntry.second->mutex);
                if (rEntry.second->disposeInstance.is()) {
                    sngls.push_back(rEntry.second->disposeInstance);
                }
            }
        }
        for (const auto& rEntry : data_.dynamicImplementations)
        {
            assert(rEntry.second);
            if (rEntry.second->shallDispose()) {
                std::unique_lock g2(rEntry.second->mutex);
                if (rEntry.second->disposeInstance.is()) {
                    sngls.push_back(rEntry.second->disposeInstance);
                }
            }
            if (rEntry.second->component.is()) {
                comps.push_back(rEntry.second->component);
            }
        }
        data_.namedImplementations.swap(clear.namedImplementations);
        data_.dynamicImplementations.swap(clear.dynamicImplementations);
        data_.services.swap(clear.services);
        data_.singletons.swap(clear.singletons);
    }
    rGuard.unlock();
    for (const auto& rxSngl : sngls)
    {
        try {
            rxSngl->dispose();
        } catch (css::uno::RuntimeException & e) {
            SAL_WARN("cppuhelper", "Ignoring " << e << " while disposing singleton");
        }
    }
    for (const auto& rxComp : comps)
    {
        removeEventListenerFromComponent(rxComp);
    }
    rGuard.lock();
}

void cppuhelper::ServiceManager::initialize(
    css::uno::Sequence<css::uno::Any> const & aArguments)
{
    OUString arg;
    if (aArguments.getLength() != 1 || !(aArguments[0] >>= arg)
        || arg != "preload")
    {
        throw css::lang::IllegalArgumentException(
            "invalid ServiceManager::initialize argument",
            css::uno::Reference<css::uno::XInterface>(), 0);
    }
    preloadImplementations();
}

OUString cppuhelper::ServiceManager::getImplementationName()
{
    return
        "com.sun.star.comp.cppuhelper.bootstrap.ServiceManager";
}

sal_Bool cppuhelper::ServiceManager::supportsService(
    OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence< OUString >
cppuhelper::ServiceManager::getSupportedServiceNames()
{
    return { "com.sun.star.lang.MultiServiceFactory", "com.sun.star.lang.ServiceManager" };
}

css::uno::Reference< css::uno::XInterface >
cppuhelper::ServiceManager::createInstance(
    OUString const & aServiceSpecifier)
{
    assert(context_.is());
    return createInstanceWithContext(aServiceSpecifier, context_);
}

css::uno::Reference< css::uno::XInterface >
cppuhelper::ServiceManager::createInstanceWithArguments(
    OUString const & ServiceSpecifier,
    css::uno::Sequence< css::uno::Any > const & Arguments)
{
    assert(context_.is());
    return createInstanceWithArgumentsAndContext(
        ServiceSpecifier, Arguments, context_);
}

css::uno::Sequence< OUString >
cppuhelper::ServiceManager::getAvailableServiceNames()
{
    std::unique_lock g(m_aMutex);
    if (m_bDisposed) {
        return css::uno::Sequence< OUString >();
    }
    if (data_.services.size() > o3tl::make_unsigned(SAL_MAX_INT32)) {
        throw css::uno::RuntimeException(
            "getAvailableServiceNames: too many services",
            static_cast< cppu::OWeakObject * >(this));
    }
    return comphelper::mapKeysToSequence(data_.services);
}

css::uno::Reference< css::uno::XInterface >
cppuhelper::ServiceManager::createInstanceWithContext(
    OUString const & aServiceSpecifier,
    css::uno::Reference< css::uno::XComponentContext > const & Context)
{
    std::shared_ptr< Data::Implementation > impl(
        findServiceImplementation(Context, aServiceSpecifier));
    return impl == nullptr ? css::uno::Reference<css::uno::XInterface>()
                           : impl->createInstance(Context, false);
}

css::uno::Reference< css::uno::XInterface >
cppuhelper::ServiceManager::createInstanceWithArgumentsAndContext(
    OUString const & ServiceSpecifier,
    css::uno::Sequence< css::uno::Any > const & Arguments,
    css::uno::Reference< css::uno::XComponentContext > const & Context)
{
    std::shared_ptr< Data::Implementation > impl(
        findServiceImplementation(Context, ServiceSpecifier));
    return impl == nullptr ? css::uno::Reference<css::uno::XInterface>()
                           : impl->createInstanceWithArguments(Context, false, Arguments);
}

css::uno::Type cppuhelper::ServiceManager::getElementType()
{
    return css::uno::Type();
}

sal_Bool cppuhelper::ServiceManager::hasElements()
{
    std::unique_lock g(m_aMutex);
    return
        !(data_.namedImplementations.empty()
          && data_.dynamicImplementations.empty());
}

css::uno::Reference< css::container::XEnumeration >
cppuhelper::ServiceManager::createEnumeration()
{
    throw css::uno::RuntimeException(
        "ServiceManager createEnumeration: method not supported",
        static_cast< cppu::OWeakObject * >(this));
}

sal_Bool cppuhelper::ServiceManager::has(css::uno::Any const &)
{
    throw css::uno::RuntimeException(
        "ServiceManager has: method not supported",
        static_cast< cppu::OWeakObject * >(this));
}

void cppuhelper::ServiceManager::insert(css::uno::Any const & aElement)
{
    css::uno::Sequence< css::beans::NamedValue > args;
    if (aElement >>= args) {
        std::vector< OUString > uris;
        css::uno::Reference< css::uno::XComponentContext > alienContext;
        for (const auto & arg : args) {
            if (arg.Name == "uri") {
                OUString uri;
                if (!(arg.Value >>= uri)) {
                    throw css::lang::IllegalArgumentException(
                        "Bad uri argument",
                        static_cast< cppu::OWeakObject * >(this), 0);
                }
                uris.push_back(uri);
            } else if (arg.Name == "component-context") {
                if (alienContext.is()) {
                    throw css::lang::IllegalArgumentException(
                        "Multiple component-context arguments",
                        static_cast< cppu::OWeakObject * >(this), 0);
                }
                if (!(arg.Value >>= alienContext) || !alienContext.is()) {
                    throw css::lang::IllegalArgumentException(
                        "Bad component-context argument",
                        static_cast< cppu::OWeakObject * >(this), 0);
                }
            } else {
                throw css::lang::IllegalArgumentException(
                    "Bad argument " + arg.Name,
                    static_cast< cppu::OWeakObject * >(this), 0);
            }
        }
        insertRdbFiles(uris, alienContext);
        return;
    }
    css::uno::Reference< css::lang::XServiceInfo > info;
    if ((aElement >>= info) && info.is()) {
        insertLegacyFactory(info);
        return;
    }

    throw css::lang::IllegalArgumentException(
        "Bad insert element", static_cast< cppu::OWeakObject * >(this), 0);
}

void cppuhelper::ServiceManager::remove(css::uno::Any const & aElement)
{
    css::uno::Sequence< css::beans::NamedValue > args;
    if (aElement >>= args) {
        std::vector< OUString > uris;
        for (const auto & i : args) {
            if (i.Name != "uri") {
                throw css::lang::IllegalArgumentException(
                    "Bad argument " + i.Name,
                    static_cast< cppu::OWeakObject * >(this), 0);
            }
            OUString uri;
            if (!(i.Value >>= uri)) {
                throw css::lang::IllegalArgumentException(
                    "Bad uri argument",
                    static_cast< cppu::OWeakObject * >(this), 0);
            }
            uris.push_back(uri);
        }
        removeRdbFiles(uris);
        return;
    }
    css::uno::Reference< css::lang::XServiceInfo > info;
    if ((aElement >>= info) && info.is()) {
        if (!removeLegacyFactory(info, true)) {
            throw css::container::NoSuchElementException(
                "Remove non-inserted factory object",
                static_cast< cppu::OWeakObject * >(this));
        }
        return;
    }
    OUString impl;
    if (aElement >>= impl) {
        // For live-removal of extensions:
        removeImplementation(impl);
        return;
    }
    throw css::lang::IllegalArgumentException(
        "Bad remove element", static_cast< cppu::OWeakObject * >(this), 0);
}

css::uno::Reference< css::container::XEnumeration >
cppuhelper::ServiceManager::createContentEnumeration(
    OUString const & aServiceName)
{
    std::vector< std::shared_ptr< Data::Implementation > > impls;
    {
        std::unique_lock g(m_aMutex);
        Data::ImplementationMap::const_iterator i(
            data_.services.find(aServiceName));
        if (i != data_.services.end()) {
            impls = i->second;
        }
    }
    std::vector< css::uno::Any > factories;
    for (const auto& rxImpl : impls)
    {
        Data::Implementation * impl = rxImpl.get();
        assert(impl != nullptr);
        {
            std::unique_lock g(m_aMutex);
            if (m_bDisposed) {
                factories.clear();
                break;
            }
            if (impl->status == Data::Implementation::STATUS_NEW) {
                // Postpone actual implementation instantiation as long as
                // possible (so that e.g. opening LO's "Tools - Macros" menu
                // does not try to instantiate a JVM, which can lead to a
                // synchronous error dialog when no JVM is specified, and
                // showing the dialog while hovering over a menu can cause
                // trouble):
                impl->factory1 = new ImplementationWrapper(this, rxImpl);
                impl->status = Data::Implementation::STATUS_WRAPPER;
            }
            if (impl->constructorFn != nullptr && !impl->factory1.is()) {
                impl->factory1 = new ImplementationWrapper(this, rxImpl);
            }
        }
        if (impl->factory1.is()) {
            factories.push_back(css::uno::Any(impl->factory1));
        } else {
            assert(impl->factory2.is());
            factories.push_back(css::uno::Any(impl->factory2));
        }
    }
    return new ContentEnumeration(std::move(factories));
}

css::uno::Reference< css::beans::XPropertySetInfo >
cppuhelper::ServiceManager::getPropertySetInfo()
{
    return this;
}

void cppuhelper::ServiceManager::setPropertyValue(
    OUString const & aPropertyName, css::uno::Any const &)
{
    if (aPropertyName == "DefaultContext") {
        throw css::beans::PropertyVetoException(
            aPropertyName, static_cast< cppu::OWeakObject * >(this));
    } else {
        throw css::beans::UnknownPropertyException(
            aPropertyName, static_cast< cppu::OWeakObject * >(this));
    }
}

css::uno::Any cppuhelper::ServiceManager::getPropertyValue(
    OUString const & PropertyName)
{
    if (PropertyName != "DefaultContext") {
        throw css::beans::UnknownPropertyException(
            PropertyName, static_cast< cppu::OWeakObject * >(this));
    }
    assert(context_.is());
    return css::uno::Any(context_);
}

void cppuhelper::ServiceManager::addPropertyChangeListener(
    OUString const & aPropertyName,
    css::uno::Reference< css::beans::XPropertyChangeListener > const &
        xListener)
{
    if (!aPropertyName.isEmpty() && aPropertyName != "DefaultContext") {
        throw css::beans::UnknownPropertyException(
            aPropertyName, static_cast< cppu::OWeakObject * >(this));
    }
    // DefaultContext does not change, so just treat it as an event listener:
    return addEventListener(xListener);
}

void cppuhelper::ServiceManager::removePropertyChangeListener(
    OUString const & aPropertyName,
    css::uno::Reference< css::beans::XPropertyChangeListener > const &
        aListener)
{
    if (!aPropertyName.isEmpty() && aPropertyName != "DefaultContext") {
        throw css::beans::UnknownPropertyException(
            aPropertyName, static_cast< cppu::OWeakObject * >(this));
    }
    // DefaultContext does not change, so just treat it as an event listener:
    return removeEventListener(aListener);
}

void cppuhelper::ServiceManager::addVetoableChangeListener(
    OUString const & PropertyName,
    css::uno::Reference< css::beans::XVetoableChangeListener > const &
        aListener)
{
    if (!PropertyName.isEmpty() && PropertyName != "DefaultContext") {
        throw css::beans::UnknownPropertyException(
            PropertyName, static_cast< cppu::OWeakObject * >(this));
    }
    // DefaultContext does not change, so just treat it as an event listener:
    return addEventListener(aListener);
}

void cppuhelper::ServiceManager::removeVetoableChangeListener(
    OUString const & PropertyName,
    css::uno::Reference< css::beans::XVetoableChangeListener > const &
        aListener)
{
    if (!PropertyName.isEmpty() && PropertyName != "DefaultContext") {
        throw css::beans::UnknownPropertyException(
            PropertyName, static_cast< cppu::OWeakObject * >(this));
    }
    // DefaultContext does not change, so just treat it as an event listener:
    return removeEventListener(aListener);
}

css::uno::Sequence< css::beans::Property >
cppuhelper::ServiceManager::getProperties() {
    return { getDefaultContextProperty() };
}

css::beans::Property cppuhelper::ServiceManager::getPropertyByName(
    OUString const & aName)
{
    if (aName != "DefaultContext") {
        throw css::beans::UnknownPropertyException(
            aName, static_cast< cppu::OWeakObject * >(this));
    }
    return getDefaultContextProperty();
}

sal_Bool cppuhelper::ServiceManager::hasPropertyByName(
    OUString const & Name)
{
    return Name == "DefaultContext";
}

cppuhelper::ServiceManager::~ServiceManager() {}

void cppuhelper::ServiceManager::disposing(
    css::lang::EventObject const & Source)
{
    removeLegacyFactory(
        css::uno::Reference< css::lang::XServiceInfo >(
            Source.Source, css::uno::UNO_QUERY_THROW),
        false);
}

void cppuhelper::ServiceManager::removeEventListenerFromComponent(
    css::uno::Reference< css::lang::XComponent > const & component)
{
    assert(component.is());
    try {
        component->removeEventListener(this);
    } catch (css::uno::RuntimeException & e) {
        SAL_INFO(
            "cppuhelper",
            "Ignored removeEventListener RuntimeException " + e.Message);
    }
}

void cppuhelper::ServiceManager::init(std::u16string_view rdbUris) {
    for (sal_Int32 i = 0; i != -1;) {
        std::u16string_view uri(o3tl::getToken(rdbUris, 0, ' ', i));
        if (uri.empty()) {
            continue;
        }
        bool optional;
        bool directory;
        cppu::decodeRdbUri(&uri, &optional, &directory);
        if (directory) {
            readRdbDirectory(uri, optional);
        } else {
            readRdbFile(OUString(uri), optional);
        }
    }
}

void cppuhelper::ServiceManager::readRdbDirectory(
    std::u16string_view uri, bool optional)
{
    osl::Directory dir = OUString(uri);
    switch (dir.open()) {
    case osl::FileBase::E_None:
        break;
    case osl::FileBase::E_NOENT:
        if (optional) {
            SAL_INFO("cppuhelper", "Ignored optional " << OUString(uri));
            return;
        }
        [[fallthrough]];
    default:
        throw css::uno::DeploymentException(
            OUString::Concat("Cannot open directory ") + uri,
            static_cast< cppu::OWeakObject * >(this));
    }
    for (;;) {
        OUString url;
        if (!cppu::nextDirectoryItem(dir, &url)) {
            break;
        }
        readRdbFile(url, false);
    }
}

void cppuhelper::ServiceManager::readRdbFile(
    OUString const & uri, bool optional)
{
    try {
        Parser(
            uri, css::uno::Reference< css::uno::XComponentContext >(), &data_);
    } catch (css::container::NoSuchElementException &) {
        if (!optional) {
            throw css::uno::DeploymentException(
                uri + ": no such file",
                static_cast< cppu::OWeakObject * >(this));
        }
        SAL_INFO("cppuhelper", "Ignored optional " << uri);
    }
#if !ENABLE_FUZZERS
    catch (css::registry::InvalidRegistryException & e) {
        if (!readLegacyRdbFile(uri)) {
            throw css::uno::DeploymentException(
                "InvalidRegistryException: " + e.Message,
                static_cast< cppu::OWeakObject * >(this));
        }
    } catch (css::uno::RuntimeException &) {
        if (!readLegacyRdbFile(uri)) {
            throw;
        }
    }
#endif
}

#if !ENABLE_FUZZERS
bool cppuhelper::ServiceManager::readLegacyRdbFile(OUString const & uri) {
    Registry reg;
    switch (reg.open(uri, RegAccessMode::READONLY)) {
    case RegError::NO_ERROR:
        break;
    case RegError::REGISTRY_NOT_EXISTS:
    case RegError::INVALID_REGISTRY:
        {
            // Ignore empty rdb files (which are at least seen by subordinate
            // uno processes during extension registration; Registry::open can
            // fail on them if mmap(2) returns EINVAL for a zero length):
            osl::DirectoryItem item;
            if (osl::DirectoryItem::get(uri, item) == osl::FileBase::E_None) {
                osl::FileStatus status(osl_FileStatus_Mask_FileSize);
                if (item.getFileStatus(status) == osl::FileBase::E_None
                    && status.getFileSize() == 0)
                {
                    return true;
                }
            }
        }
        [[fallthrough]];
    default:
        return false;
    }
    RegistryKey rootKey;
    if (reg.openRootKey(rootKey) != RegError::NO_ERROR) {
        throw css::uno::DeploymentException(
            "Failure reading legacy rdb file " + uri,
            static_cast< cppu::OWeakObject * >(this));
    }
    RegistryKeyArray impls;
    switch (rootKey.openSubKeys("IMPLEMENTATIONS", impls)) {
    case RegError::NO_ERROR:
        break;
    case RegError::KEY_NOT_EXISTS:
        return true;
    default:
        throw css::uno::DeploymentException(
            "Failure reading legacy rdb file " + uri,
            static_cast< cppu::OWeakObject * >(this));
    }
    for (sal_uInt32 i = 0; i != impls.getLength(); ++i) {
        RegistryKey implKey(impls.getElement(i));
        assert(implKey.getName().match("/IMPLEMENTATIONS/"));
        OUString name(
            implKey.getName().copy(RTL_CONSTASCII_LENGTH("/IMPLEMENTATIONS/")));
        std::shared_ptr< Data::Implementation > impl =
            std::make_shared<Data::Implementation>(
                name, readLegacyRdbString(uri, implKey, "UNO/ACTIVATOR"),
                readLegacyRdbString(uri, implKey, "UNO/LOCATION"), "", "", "", false,
                css::uno::Reference< css::uno::XComponentContext >(), uri);
        if (!data_.namedImplementations.emplace(name, impl).second)
        {
            throw css::registry::InvalidRegistryException(
                uri + ": duplicate <implementation name=\"" + name + "\">");
        }
        readLegacyRdbStrings(
            uri, implKey, "UNO/SERVICES", &impl->services);
        for (const auto& rService : impl->services)
        {
            data_.services[rService].push_back(impl);
        }
        readLegacyRdbStrings(
            uri, implKey, "UNO/SINGLETONS", &impl->singletons);
        for (const auto& rSingleton : impl->singletons)
        {
            data_.singletons[rSingleton].push_back(impl);
        }
    }
    return true;
}

OUString cppuhelper::ServiceManager::readLegacyRdbString(
    std::u16string_view uri, RegistryKey & key, OUString const & path)
{
    RegistryKey subkey;
    RegValueType t;
    sal_uInt32 s(0);
    if (key.openKey(path, subkey) != RegError::NO_ERROR
        || subkey.getValueInfo(OUString(), &t, &s) != RegError::NO_ERROR
        || t != RegValueType::STRING
        || s == 0 || s > o3tl::make_unsigned(SAL_MAX_INT32))
    {
        throw css::uno::DeploymentException(
            OUString::Concat("Failure reading legacy rdb file ") + uri,
            static_cast< cppu::OWeakObject * >(this));
    }
    OUString val;
    std::vector< char > v(s); // assuming sal_uInt32 fits into vector::size_type
    if (subkey.getValue(OUString(), v.data()) != RegError::NO_ERROR
        || v.back() != '\0'
        || !rtl_convertStringToUString(
            &val.pData, v.data(), static_cast< sal_Int32 >(s - 1),
            RTL_TEXTENCODING_UTF8,
            (RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR
             | RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR
             | RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR)))
    {
        throw css::uno::DeploymentException(
            OUString::Concat("Failure reading legacy rdb file ") + uri,
            static_cast< cppu::OWeakObject * >(this));
    }
    return val;
}

void cppuhelper::ServiceManager::readLegacyRdbStrings(
    std::u16string_view uri, RegistryKey & key, OUString const & path,
    std::vector< OUString > * strings)
{
    assert(strings != nullptr);
    RegistryKey subkey;
    switch (key.openKey(path, subkey)) {
    case RegError::NO_ERROR:
        break;
    case RegError::KEY_NOT_EXISTS:
        return;
    default:
        throw css::uno::DeploymentException(
            OUString::Concat("Failure reading legacy rdb file ") + uri,
            static_cast< cppu::OWeakObject * >(this));
    }
    OUString prefix(subkey.getName() + "/");
    RegistryKeyNames names;
    if (subkey.getKeyNames(OUString(), names) != RegError::NO_ERROR) {
        throw css::uno::DeploymentException(
            OUString::Concat("Failure reading legacy rdb file ") + uri,
            static_cast< cppu::OWeakObject * >(this));
    }
    for (sal_uInt32 i = 0; i != names.getLength(); ++i) {
        assert(names.getElement(i).match(prefix));
        strings->push_back(names.getElement(i).copy(prefix.getLength()));
    }
}
#endif

void cppuhelper::ServiceManager::insertRdbFiles(
    std::vector< OUString > const & uris,
    css::uno::Reference< css::uno::XComponentContext > const & alienContext)
{
    Data extra;
    for (const auto& rUri : uris)
    {
        try {
            Parser(rUri, alienContext, &extra);
        } catch (css::container::NoSuchElementException &) {
            throw css::lang::IllegalArgumentException(
                rUri + ": no such file", static_cast< cppu::OWeakObject * >(this),
                0);
        } catch (css::registry::InvalidRegistryException & e) {
            throw css::lang::IllegalArgumentException(
                "InvalidRegistryException: " + e.Message,
                static_cast< cppu::OWeakObject * >(this), 0);
        }
    }
    insertExtraData(extra);
}

void cppuhelper::ServiceManager::insertLegacyFactory(
    css::uno::Reference< css::lang::XServiceInfo > const & factoryInfo)
{
    assert(factoryInfo.is());
    OUString name(factoryInfo->getImplementationName());
    css::uno::Reference< css::lang::XSingleComponentFactory > f1(
        factoryInfo, css::uno::UNO_QUERY);
    css::uno::Reference< css::lang::XSingleServiceFactory > f2;
    if (!f1.is()) {
        f2.set(factoryInfo, css::uno::UNO_QUERY);
        if (!f2.is()) {
            throw css::lang::IllegalArgumentException(
                ("Bad XServiceInfo argument implements neither"
                 " XSingleComponentFactory nor XSingleServiceFactory"),
                static_cast< cppu::OWeakObject * >(this), 0);
        }
    }
    css::uno::Reference< css::lang::XComponent > comp(
        factoryInfo, css::uno::UNO_QUERY);
    std::shared_ptr< Data::Implementation > impl =
        std::make_shared<Data::Implementation>(name, f1, f2, comp);
    Data extra;
    if (!name.isEmpty()) {
        extra.namedImplementations.emplace(name, impl);
    }
    extra.dynamicImplementations.emplace(factoryInfo, impl);
    const css::uno::Sequence< OUString > services(
        factoryInfo->getSupportedServiceNames());
    for (const auto & i : services) {
        impl->services.push_back(i);
        extra.services[i].push_back(impl);
    }
    if (insertExtraData(extra) && comp.is()) {
        comp->addEventListener(this);
    }
}

bool cppuhelper::ServiceManager::insertExtraData(Data const & extra) {
    {
        std::unique_lock g(m_aMutex);
        if (m_bDisposed) {
            return false;
        }
        auto i = std::find_if(extra.namedImplementations.begin(), extra.namedImplementations.end(),
            [this](const Data::NamedImplementations::value_type& rEntry) {
                return data_.namedImplementations.find(rEntry.first) != data_.namedImplementations.end(); });
        if (i != extra.namedImplementations.end())
        {
            throw css::lang::IllegalArgumentException(
                "Insert duplicate implementation name " + i->first,
                static_cast< cppu::OWeakObject * >(this), 0);
        }
        bool bDuplicate = std::any_of(extra.dynamicImplementations.begin(), extra.dynamicImplementations.end(),
            [this](const Data::DynamicImplementations::value_type& rEntry) {
                return data_.dynamicImplementations.find(rEntry.first) != data_.dynamicImplementations.end(); });
        if (bDuplicate)
        {
            throw css::lang::IllegalArgumentException(
                "Insert duplicate factory object",
                static_cast< cppu::OWeakObject * >(this), 0);
        }
        //TODO: The below leaves data_ in an inconsistent state upon exceptions:
        data_.namedImplementations.insert(
            extra.namedImplementations.begin(),
            extra.namedImplementations.end());
        data_.dynamicImplementations.insert(
            extra.dynamicImplementations.begin(),
            extra.dynamicImplementations.end());
        insertImplementationMap(&data_.services, extra.services);
        insertImplementationMap(&data_.singletons, extra.singletons);
    }
    //TODO: Updating the component context singleton data should be part of the
    // atomic service manager update:
    if (extra.singletons.empty())
        return true;

    assert(context_.is());
    css::uno::Reference< css::container::XNameContainer > cont(
        context_, css::uno::UNO_QUERY_THROW);
    for (const auto& [rName, rImpls] : extra.singletons)
    {
        OUString name("/singletons/" + rName);
        //TODO: Update should be atomic:
        try {
            cont->removeByName(name + "/arguments");
        } catch (const css::container::NoSuchElementException &) {}
        assert(!rImpls.empty());
        assert(rImpls[0]);
        SAL_INFO_IF(
            rImpls.size() > 1, "cppuhelper",
            "Arbitrarily choosing " << rImpls[0]->name
                << " among multiple implementations for singleton "
                << rName);
        try {
            cont->insertByName(
                name + "/service", css::uno::Any(rImpls[0]->name));
        } catch (css::container::ElementExistException &) {
            cont->replaceByName(
                name + "/service", css::uno::Any(rImpls[0]->name));
        }
        try {
            cont->insertByName(name, css::uno::Any());
        } catch (css::container::ElementExistException &) {
            SAL_INFO("cppuhelper", "Overwriting singleton " << rName);
            cont->replaceByName(name, css::uno::Any());
        }
    }
    return true;
}

void cppuhelper::ServiceManager::removeRdbFiles(
    std::vector< OUString > const & uris)
{
    // The underlying data structures make this function somewhat inefficient,
    // but the assumption is that it is rarely called (and that if it is called,
    // it is called with a uris vector of size one):
    std::vector< std::shared_ptr< Data::Implementation > > clear;
    {
        std::unique_lock g(m_aMutex);
        for (const auto& rUri : uris)
        {
            for (Data::NamedImplementations::iterator j(
                     data_.namedImplementations.begin());
                 j != data_.namedImplementations.end();)
            {
                assert(j->second);
                if (j->second->rdbFile == rUri) {
                    clear.push_back(j->second);
                    //TODO: The below leaves data_ in an inconsistent state upon
                    // exceptions:
                    removeFromImplementationMap(
                        &data_.services, j->second->services, j->second);
                    removeFromImplementationMap(
                        &data_.singletons, j->second->singletons,
                        j->second);
                    j = data_.namedImplementations.erase(j);
                } else {
                    ++j;
                }
            }
        }
    }
    //TODO: Update the component context singleton data
}

bool cppuhelper::ServiceManager::removeLegacyFactory(
    css::uno::Reference< css::lang::XServiceInfo > const & factoryInfo,
    bool removeListener)
{
    assert(factoryInfo.is());
    std::shared_ptr< Data::Implementation > clear;
    css::uno::Reference< css::lang::XComponent > comp;
    {
        std::unique_lock g(m_aMutex);
        Data::DynamicImplementations::iterator i(
            data_.dynamicImplementations.find(factoryInfo));
        if (i == data_.dynamicImplementations.end()) {
            return m_bDisposed;
        }
        assert(i->second);
        clear = i->second;
        if (removeListener) {
            comp = i->second->component;
        }
        //TODO: The below leaves data_ in an inconsistent state upon exceptions:
        removeFromImplementationMap(
            &data_.services, i->second->services, i->second);
        removeFromImplementationMap(
            &data_.singletons, i->second->singletons, i->second);
        if (!i->second->name.isEmpty()) {
            data_.namedImplementations.erase(i->second->name);
        }
        data_.dynamicImplementations.erase(i);
    }
    if (comp.is()) {
        removeEventListenerFromComponent(comp);
    }
    return true;
}

void cppuhelper::ServiceManager::removeImplementation(const OUString & name) {
    // The underlying data structures make this function somewhat inefficient,
    // but the assumption is that it is rarely called:
    std::shared_ptr< Data::Implementation > clear;
    {
        std::unique_lock g(m_aMutex);
        if (m_bDisposed) {
            return;
        }
        Data::NamedImplementations::iterator i(
            data_.namedImplementations.find(name));
        if (i == data_.namedImplementations.end()) {
            throw css::container::NoSuchElementException(
                "Remove non-inserted implementation " + name,
                static_cast< cppu::OWeakObject * >(this));
        }
        assert(i->second);
        clear = i->second;
        //TODO: The below leaves data_ in an inconsistent state upon exceptions:
        removeFromImplementationMap(
            &data_.services, i->second->services, i->second);
        removeFromImplementationMap(
            &data_.singletons, i->second->singletons, i->second);
        auto j = std::find_if(data_.dynamicImplementations.begin(), data_.dynamicImplementations.end(),
            [&i](const Data::DynamicImplementations::value_type& rEntry) { return rEntry.second == i->second; });
        if (j != data_.dynamicImplementations.end())
            data_.dynamicImplementations.erase(j);
        data_.namedImplementations.erase(i);
    }
}

std::shared_ptr< cppuhelper::ServiceManager::Data::Implementation >
cppuhelper::ServiceManager::findServiceImplementation(
    css::uno::Reference< css::uno::XComponentContext > const & context,
    OUString const & specifier)
{
    std::shared_ptr< Data::Implementation > impl;
    bool loaded;
    {
        std::unique_lock g(m_aMutex);
        Data::ImplementationMap::const_iterator i(
            data_.services.find(specifier));
        if (i == data_.services.end()) {
            Data::NamedImplementations::const_iterator j(
                data_.namedImplementations.find(specifier));
            if (j == data_.namedImplementations.end()) {
                SAL_INFO("cppuhelper", "No implementation for " << specifier);
                return std::shared_ptr< Data::Implementation >();
            }
            impl = j->second;
        } else {
            assert(!i->second.empty());
            SAL_INFO_IF(
                i->second.size() > 1, "cppuhelper",
                "Arbitrarily choosing " << i->second[0]->name
                    << " among multiple implementations for " << i->first);
            impl = i->second[0];
        }
        assert(impl);
        loaded = impl->status == Data::Implementation::STATUS_LOADED;
    }
    if (!loaded) {
        loadImplementation(context, impl);
    }
    return impl;
}

/// Make a simpler unique name for preload / progress reporting.
#ifndef DISABLE_DYNLOADING
static OUString simplifyModule(std::u16string_view uri)
{
    sal_Int32 nIdx;
    OUStringBuffer edit(uri);
    if ((nIdx = edit.lastIndexOf('/')) > 0)
        edit.remove(0,nIdx+1);
    if ((nIdx = edit.lastIndexOf(':')) > 0)
        edit.remove(0,nIdx+1);
    if ((nIdx = edit.lastIndexOf("lo.so")) > 0)
        edit.truncate(nIdx);
    if ((nIdx = edit.lastIndexOf(".3")) > 0)
        edit.truncate(nIdx);
    if ((nIdx = edit.lastIndexOf("gcc3.so")) > 0)
        edit.truncate(nIdx);
    if ((nIdx = edit.lastIndexOf(".so")) > 0)
        edit.truncate(nIdx);
    if ((nIdx = edit.lastIndexOf("_uno")) > 0)
        edit.truncate(nIdx);
    if ((nIdx = edit.lastIndexOf(".jar")) > 0)
        edit.truncate(nIdx);
    if (edit.indexOf("lib") == 0)
        edit.remove(0,3);
    return edit.makeStringAndClear();
}
#endif

/// Used only by LibreOfficeKit when used by Online to pre-initialize
void cppuhelper::ServiceManager::preloadImplementations() {
#ifdef DISABLE_DYNLOADING
    abort();
#else
    OUString aUri;
    std::unique_lock g(m_aMutex);
    css::uno::Environment aSourceEnv(css::uno::Environment::getCurrent());

    std::cerr << "preload:";
    std::vector<OUString> aReported;
    std::vector<OUString> aDisabled;
    OUStringBuffer aDisabledMsg;
    OUStringBuffer aMissingMsg;

    /// Allow external callers & testers to disable certain components
    const char *pDisable = getenv("UNODISABLELIBRARY");
    if (pDisable)
    {
        OUString aDisable(pDisable, strlen(pDisable), RTL_TEXTENCODING_UTF8);
        for (sal_Int32 i = 0; i >= 0; )
        {
            OUString tok( aDisable.getToken(0, ' ', i) );
            tok = tok.trim();
            if (!tok.isEmpty())
                aDisabled.push_back(tok);
        }
    }

    // loop all implementations
    for (const auto& rEntry : data_.namedImplementations)
    {
        if (rEntry.second->loader != "com.sun.star.loader.SharedLibrary" ||
            rEntry.second->status == Data::Implementation::STATUS_LOADED)
            continue;

        OUString simplified;
        try
        {
            const OUString &aLibrary = rEntry.second->uri;

            if (aLibrary.isEmpty())
                continue;

            simplified = simplifyModule(aLibrary);

            bool bDisabled =
                std::find(aDisabled.begin(), aDisabled.end(), simplified) != aDisabled.end();

            if (std::find(aReported.begin(), aReported.end(), aLibrary) == aReported.end())
            {
                if (bDisabled)
                {
                    aDisabledMsg.append(simplified + " ");
                }
                else
                {
                    std::cerr << " " << simplified;
                    std::cerr.flush();
                }
                aReported.push_back(aLibrary);
            }

            if (bDisabled)
                continue;

            // expand absolute URI implementation component library
            aUri = cppu::bootstrap_expandUri(aLibrary);
        }
        catch (css::lang::IllegalArgumentException& aError)
        {
            throw css::uno::DeploymentException(
                "Cannot expand URI" + rEntry.second->uri + ": " + aError.Message,
                static_cast< cppu::OWeakObject * >(this));
        }

        // load component library
        osl::Module aModule(aUri, SAL_LOADMODULE_NOW | SAL_LOADMODULE_GLOBAL);

        if (!aModule.is())
        {
            aMissingMsg.append(simplified + " ");
        }

        if (aModule.is() &&
            !rEntry.second->environment.isEmpty())
        {
            oslGenericFunction fpFactory;
            css::uno::Environment aTargetEnv;
            css::uno::Reference<css::uno::XInterface> xFactory;

            if(rEntry.second->constructorName.isEmpty())
            {
                OUString aSymFactory;
                // expand full name component factory symbol
                if (rEntry.second->prefix == "direct")
                    aSymFactory = rEntry.second->name.replace('.', '_') + "_" COMPONENT_GETFACTORY;
                else if (!rEntry.second->prefix.isEmpty())
                    aSymFactory = rEntry.second->prefix + "_" COMPONENT_GETFACTORY;
                else
                    aSymFactory = COMPONENT_GETFACTORY;

                // get function symbol component factory
                fpFactory = aModule.getFunctionSymbol(aSymFactory);
                if (fpFactory == nullptr)
                {
                    throw css::loader::CannotActivateFactoryException(
                        ("no factory symbol \"" + aSymFactory + "\" in component library :" + aUri),
                        css::uno::Reference<css::uno::XInterface>());
                }

                aTargetEnv = cppuhelper::detail::getEnvironment(rEntry.second->environment, rEntry.second->name);
                component_getFactoryFunc fpComponentFactory = reinterpret_cast<component_getFactoryFunc>(fpFactory);

                if (aSourceEnv.get() == aTargetEnv.get())
                {
                    // invoke function component factory
                    OString aImpl(OUStringToOString(rEntry.second->name, RTL_TEXTENCODING_ASCII_US));
                    xFactory.set(css::uno::Reference<css::uno::XInterface>(static_cast<css::uno::XInterface *>(
                        (*fpComponentFactory)(aImpl.getStr(), this, nullptr)), SAL_NO_ACQUIRE));
                }
            }
            else
            {
                // get function symbol component factory
                aTargetEnv = cppuhelper::detail::getEnvironment(rEntry.second->environment, rEntry.second->name);
                fpFactory = (aSourceEnv.get() == aTargetEnv.get()) ?
                    aModule.getFunctionSymbol(rEntry.second->constructorName) : nullptr;
            }

            css::uno::Reference<css::lang::XSingleComponentFactory> xSCFactory;
            css::uno::Reference<css::lang::XSingleServiceFactory> xSSFactory;

            // query interface XSingleComponentFactory or XSingleServiceFactory
            if (xFactory.is())
            {
                xSCFactory.set(xFactory, css::uno::UNO_QUERY);
                if (!xSCFactory.is())
                {
                    xSSFactory.set(xFactory, css::uno::UNO_QUERY);
                    if (!xSSFactory.is())
                        throw css::uno::DeploymentException(
                            ("Implementation " + rEntry.second->name
                             + " does not provide a constructor or factory"),
                            static_cast< cppu::OWeakObject * >(this));
                }
            }

            if (!rEntry.second->constructorName.isEmpty() && fpFactory)
                rEntry.second->constructorFn = WrapperConstructorFn(reinterpret_cast<ImplementationConstructorFn *>(fpFactory));

            rEntry.second->factory1 = xSCFactory;
            rEntry.second->factory2 = xSSFactory;
            rEntry.second->status = Data::Implementation::STATUS_LOADED;

        }

        // Some libraries use other (non-UNO) libraries requiring preinit
        oslGenericFunction fpPreload = aModule.getFunctionSymbol( "lok_preload_hook" );
        if (fpPreload)
        {
            static std::vector<oslGenericFunction> aPreloaded;
            if (std::find(aPreloaded.begin(), aPreloaded.end(), fpPreload) == aPreloaded.end())
            {
                aPreloaded.push_back(fpPreload);
                // unlock because we may be instantiating some services here
                g.unlock();
                fpPreload();
                g.lock();
            }
        }

        // leak aModule
        aModule.release();
    }
    std::cerr << std::endl;

    if (aMissingMsg.getLength() > 0)
    {
        OUString aMsg = aMissingMsg.makeStringAndClear();
        std::cerr << "Absent (often optional): " << aMsg << "\n";
    }
    if (aDisabledMsg.getLength() > 0)
    {
        OUString aMsg = aDisabledMsg.makeStringAndClear();
        std::cerr << "Disabled: " << aMsg << "\n";
    }
    std::cerr.flush();

    // Various rather important uno mappings.
    static struct {
        const char *mpFrom;
        const char *mpTo;
        const char *mpPurpose;
    } const aMappingLoad[] = {
        { "gcc3", "uno",  "" },
        { "uno",  "gcc3", "" },
    };

    static std::vector<css::uno::Mapping> maMaps;
    for (auto &it : aMappingLoad)
    {
        maMaps.push_back(css::uno::Mapping(
                             OUString::createFromAscii(it.mpFrom),
                             OUString::createFromAscii(it.mpTo),
                             OUString::createFromAscii(it.mpPurpose)));
    }
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
