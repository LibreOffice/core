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
 * [ Copyright (C) 2012 Red Hat, Inc., Stephan Bergmann <sbergman@redhat.com>
 *   (initial developer) ]
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

#include <algorithm>
#include <cassert>
#include <cstring>
#include <map>
#include <vector>

#include "boost/noncopyable.hpp"
#include "boost/shared_ptr.hpp"
#include "com/sun/star/beans/NamedValue.hpp"
#include "com/sun/star/beans/PropertyAttribute.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/container/ElementExistException.hpp"
#include "com/sun/star/container/XContentEnumerationAccess.hpp"
#include "com/sun/star/container/XEnumeration.hpp"
#include "com/sun/star/container/XHierarchicalNameAccess.hpp"
#include "com/sun/star/container/XNameContainer.hpp"
#include "com/sun/star/container/XSet.hpp"
#include "com/sun/star/lang/XInitialization.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/lang/XSingleComponentFactory.hpp"
#include "com/sun/star/lang/XSingleServiceFactory.hpp"
#include "com/sun/star/loader/XImplementationLoader.hpp"
#include "com/sun/star/registry/InvalidRegistryException.hpp"
#include "com/sun/star/registry/XSimpleRegistry.hpp"
#include "com/sun/star/uno/DeploymentException.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "cppuhelper/bootstrap.hxx"
#include "cppuhelper/compbase8.hxx"
#include "cppuhelper/component_context.hxx"
#include "cppuhelper/implbase1.hxx"
#include "cppuhelper/implbase3.hxx"
#include "cppuhelper/shlib.hxx"
#include "osl/file.hxx"
#include "registry/registry.hxx"
#include "rtl/bootstrap.hxx"
#include "rtl/ref.hxx"
#include "rtl/uri.hxx"
#include "rtl/ustring.hxx"
#include "xmlreader/xmlreader.hxx"

#include "macro_expander.hxx"
#include "paths.hxx"
#include "servicefactory_detail.hxx"

namespace {

namespace css = com::sun::star;

bool nextDirectoryItem(osl::Directory & directory, rtl::OUString * url) {
    assert(url != 0);
    for (;;) {
        osl::DirectoryItem i;
        switch (directory.getNextItem(i, SAL_MAX_UINT32)) {
        case osl::FileBase::E_None:
            break;
        case osl::FileBase::E_NOENT:
            return false;
        default:
            throw css::uno::DeploymentException(
                "Cannot iterate directory",
                css::uno::Reference< css::uno::XInterface >());
        }
        osl::FileStatus stat(
            osl_FileStatus_Mask_Type | osl_FileStatus_Mask_FileName |
            osl_FileStatus_Mask_FileURL);
        if (i.getFileStatus(stat) != osl::FileBase::E_None) {
            throw css::uno::DeploymentException(
                "Cannot stat in directory",
                css::uno::Reference< css::uno::XInterface >());
        }
        if (stat.getFileType() != osl::FileStatus::Directory) { //TODO: symlinks
            // Ignore backup files:
            rtl::OUString name(stat.getFileName());
            if (!(name.match(".") || name.endsWith("~"))) {
                *url = stat.getFileURL();
                return true;
            }
        }
    }
}

void decodeRdbUri(rtl::OUString * uri, bool * optional, bool * directory) {
    assert(uri != 0 && optional != 0 && directory != 0);
    *optional = (*uri)[0] == '?';
    if (*optional) {
        *uri = uri->copy(1);
    }
    *directory = uri->getLength() >= 3 && (*uri)[0] == '<'
        && (*uri)[uri->getLength() - 2] == '>'
        && (*uri)[uri->getLength() - 1] == '*';
    if (*directory) {
        *uri = uri->copy(1, uri->getLength() - 3);
    }
}

struct ImplementationInfo: private boost::noncopyable {
    ImplementationInfo(
        rtl::OUString const & theName, rtl::OUString const & theLoader,
        rtl::OUString const & theUri, rtl::OUString const & thePrefix,
        css::uno::Reference< css::uno::XComponentContext > const &
            theAlienContext,
        rtl::OUString const & theRdbFile):
        name(theName), loader(theLoader), uri(theUri), prefix(thePrefix),
        alienContext(theAlienContext), rdbFile(theRdbFile)
    {}

    explicit ImplementationInfo(rtl::OUString const & theName): name(theName) {}

    rtl::OUString const name;
    rtl::OUString const loader;
    rtl::OUString const uri;
    rtl::OUString const prefix;
    css::uno::Reference< css::uno::XComponentContext > const alienContext;
    rtl::OUString const rdbFile;
    std::vector< rtl::OUString > services;
    std::vector< rtl::OUString > singletons;
};

struct Implementation: private boost::noncopyable {
    Implementation(
        rtl::OUString const & name, rtl::OUString const & loader,
        rtl::OUString const & uri, rtl::OUString const & prefix,
        css::uno::Reference< css::uno::XComponentContext > const & alienContext,
        rtl::OUString const & rdbFile):
        info(
            new ImplementationInfo(
                name, loader, uri, prefix, alienContext, rdbFile)),
        loaded(false)
    {}

    Implementation(
        rtl::OUString const & name,
        css::uno::Reference< css::lang::XSingleComponentFactory > const &
            theFactory1,
        css::uno::Reference< css::lang::XSingleServiceFactory > const &
            theFactory2,
        css::uno::Reference< css::lang::XComponent > const & theComponent):
        info(new ImplementationInfo(name)), factory1(theFactory1),
        factory2(theFactory2), component(theComponent), loaded(true)
    {}

    boost::shared_ptr< ImplementationInfo > info;
    css::uno::Reference< css::lang::XSingleComponentFactory > factory1;
    css::uno::Reference< css::lang::XSingleServiceFactory > factory2;
    css::uno::Reference< css::lang::XComponent > component;
    bool loaded;
};

typedef std::map< rtl::OUString, boost::shared_ptr< Implementation > >
NamedImplementations;

typedef
    std::map<
        css::uno::Reference< css::lang::XServiceInfo >,
        boost::shared_ptr< Implementation > >
    DynamicImplementations;

typedef
    std::map<
        rtl::OUString, std::vector< boost::shared_ptr< Implementation > > >
    ImplementationMap;

void insertImplementationMap(
    ImplementationMap * destination, ImplementationMap const & source)
{
    assert(destination != 0);
    for (ImplementationMap::const_iterator i(source.begin()); i != source.end();
         ++i)
    {
        std::vector< boost::shared_ptr< Implementation > > & impls
            = (*destination)[i->first];
        impls.insert(impls.end(), i->second.begin(), i->second.end());
    }
}

void removeFromImplementationMap(
    ImplementationMap * map, std::vector< rtl::OUString > const & elements,
    boost::shared_ptr< Implementation > const & implementation)
{
    // The underlying data structures make this function somewhat inefficient,
    // but the assumption is that it is rarely called:
    assert(map != 0);
    for (std::vector< rtl::OUString >::const_iterator i(elements.begin());
         i != elements.end(); ++i)
    {
        ImplementationMap::iterator j(map->find(*i));
        assert(j != map->end());
        std::vector< boost::shared_ptr< Implementation > >::iterator k(
            std::find(j->second.begin(), j->second.end(), implementation));
        assert(k != j->second.end());
        j->second.erase(k);
        if (j->second.empty()) {
            map->erase(j);
        }
    }
}

struct Data: private boost::noncopyable {
    NamedImplementations namedImplementations;
    DynamicImplementations dynamicImplementations;
    ImplementationMap services;
    ImplementationMap singletons;
};

// This is largely a copy from stoc/source/simpleregistry/textualservices.cxx
// (which it obsoletes); cppuhelper's published interface concept makes it
// difficult to make both places use a shared Parser implementation, so I
// created a copy for now (until the whole stoc/source/simpleregistry stuff can
// be removed in an incompatible LibreOffice version).  For simplicity, this
// code keeps throwing css::registry::InvalidRegistryException for invalid XML
// rdbs (even though that does not fit the exception's name):
class Parser: private boost::noncopyable {
public:
    Parser(
        rtl::OUString const & uri,
        css::uno::Reference< css::uno::XComponentContext > const & alienContext,
        Data * data);

private:
    void handleComponent();

    void handleImplementation();

    void handleService();

    void handleSingleton();

    rtl::OUString getNameAttribute();

    xmlreader::XmlReader reader_;
    css::uno::Reference< css::uno::XComponentContext > alienContext_;
    Data * data_;
    rtl::OUString attrLoader_;
    rtl::OUString attrUri_;
    rtl::OUString attrPrefix_;
    rtl::OUString attrImplementation_;
    boost::shared_ptr< Implementation > implementation_;
};

Parser::Parser(
    rtl::OUString const & uri,
    css::uno::Reference< css::uno::XComponentContext > const & alienContext,
    Data * data):
    reader_(uri), alienContext_(alienContext), data_(data)
{
    assert(data != 0);
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
            xmlreader::XmlReader::TEXT_NONE, &name, &nsId);
        switch (state) {
        case STATE_BEGIN:
            if (res == xmlreader::XmlReader::RESULT_BEGIN && nsId == ucNsId
                && name.equals(RTL_CONSTASCII_STRINGPARAM("components")))
            {
                state = STATE_COMPONENTS;
                break;
            }
            throw css::registry::InvalidRegistryException(
                reader_.getUrl() + ": unexpected item in outer level",
                css::uno::Reference< css::uno::XInterface >());
        case STATE_END:
            if (res == xmlreader::XmlReader::RESULT_DONE) {
                return;
            }
            throw css::registry::InvalidRegistryException(
                reader_.getUrl() + ": unexpected item in outer level",
                css::uno::Reference< css::uno::XInterface >());
        case STATE_COMPONENTS:
            if (res == xmlreader::XmlReader::RESULT_END) {
                state = STATE_END;
                break;
            }
            if (res == xmlreader::XmlReader::RESULT_BEGIN && nsId == ucNsId
                && name.equals(RTL_CONSTASCII_STRINGPARAM("component")))
            {
                handleComponent();
                state = STATE_COMPONENT_INITIAL;
                break;
            }
            throw css::registry::InvalidRegistryException(
                reader_.getUrl() + ": unexpected item in <components>",
                css::uno::Reference< css::uno::XInterface >());
        case STATE_COMPONENT:
            if (res == xmlreader::XmlReader::RESULT_END) {
                state = STATE_COMPONENTS;
                break;
            }
            // fall through
        case STATE_COMPONENT_INITIAL:
            if (res == xmlreader::XmlReader::RESULT_BEGIN && nsId == ucNsId
                && name.equals(RTL_CONSTASCII_STRINGPARAM("implementation")))
            {
                handleImplementation();
                state = STATE_IMPLEMENTATION;
                break;
            }
            throw css::registry::InvalidRegistryException(
                reader_.getUrl() + ": unexpected item in <component>",
                css::uno::Reference< css::uno::XInterface >());
        case STATE_IMPLEMENTATION:
            if (res == xmlreader::XmlReader::RESULT_END) {
                state = STATE_COMPONENT;
                break;
            }
            if (res == xmlreader::XmlReader::RESULT_BEGIN && nsId == ucNsId
                && name.equals(RTL_CONSTASCII_STRINGPARAM("service")))
            {
                handleService();
                state = STATE_SERVICE;
                break;
            }
            if (res == xmlreader::XmlReader::RESULT_BEGIN && nsId == ucNsId
                && name.equals(RTL_CONSTASCII_STRINGPARAM("singleton")))
            {
                handleSingleton();
                state = STATE_SINGLETON;
                break;
            }
            throw css::registry::InvalidRegistryException(
                reader_.getUrl() + ": unexpected item in <implementation>",
                css::uno::Reference< css::uno::XInterface >());
        case STATE_SERVICE:
            if (res == xmlreader::XmlReader::RESULT_END) {
                state = STATE_IMPLEMENTATION;
                break;
            }
            throw css::registry::InvalidRegistryException(
                reader_.getUrl() + ": unexpected item in <service>",
                css::uno::Reference< css::uno::XInterface >());
        case STATE_SINGLETON:
            if (res == xmlreader::XmlReader::RESULT_END) {
                state = STATE_IMPLEMENTATION;
                break;
            }
            throw css::registry::InvalidRegistryException(
                reader_.getUrl() + ": unexpected item in <service>",
                css::uno::Reference< css::uno::XInterface >());
        }
    }
}

void Parser::handleComponent() {
    attrLoader_ = rtl::OUString();
    attrUri_ = rtl::OUString();
    attrPrefix_ = rtl::OUString();
    xmlreader::Span name;
    int nsId;
    while (reader_.nextAttribute(&nsId, &name)) {
        if (nsId == xmlreader::XmlReader::NAMESPACE_NONE
            && name.equals(RTL_CONSTASCII_STRINGPARAM("loader")))
        {
            if (!attrLoader_.isEmpty()) {
                throw css::registry::InvalidRegistryException(
                    (reader_.getUrl()
                     + ": <component> has multiple \"loader\" attributes"),
                    css::uno::Reference< css::uno::XInterface >());
            }
            attrLoader_ = reader_.getAttributeValue(false).convertFromUtf8();
            if (attrLoader_.isEmpty()) {
                throw css::registry::InvalidRegistryException(
                    (reader_.getUrl()
                     + ": <component> has empty \"loader\" attribute"),
                    css::uno::Reference< css::uno::XInterface >());
            }
        } else if (nsId == xmlreader::XmlReader::NAMESPACE_NONE
                   && name.equals(RTL_CONSTASCII_STRINGPARAM("uri")))
        {
            if (!attrUri_.isEmpty()) {
                throw css::registry::InvalidRegistryException(
                    (reader_.getUrl()
                     + ": <component> has multiple \"uri\" attributes"),
                    css::uno::Reference< css::uno::XInterface >());
            }
            attrUri_ = reader_.getAttributeValue(false).convertFromUtf8();
            if (attrUri_.isEmpty()) {
                throw css::registry::InvalidRegistryException(
                    (reader_.getUrl()
                     + ": <component> has empty \"uri\" attribute"),
                    css::uno::Reference< css::uno::XInterface >());
            }
        } else if (nsId == xmlreader::XmlReader::NAMESPACE_NONE
                   && name.equals(RTL_CONSTASCII_STRINGPARAM("prefix")))
        {
            if (!attrPrefix_.isEmpty()) {
                throw css::registry::InvalidRegistryException(
                    (reader_.getUrl() +
                     ": <component> has multiple \"prefix\" attributes"),
                    css::uno::Reference< css::uno::XInterface >());
            }
            attrPrefix_ = reader_.getAttributeValue(false).convertFromUtf8();
            if (attrPrefix_.isEmpty()) {
                throw css::registry::InvalidRegistryException(
                    (reader_.getUrl() +
                     ": <component> has empty \"prefix\" attribute"),
                    css::uno::Reference< css::uno::XInterface >());
            }
        } else {
            throw css::registry::InvalidRegistryException(
                (reader_.getUrl() + ": unexpected attribute \""
                 + name.convertFromUtf8() + "\" in <component>"),
                css::uno::Reference< css::uno::XInterface >());
        }
    }
    if (attrLoader_.isEmpty()) {
        throw css::registry::InvalidRegistryException(
            reader_.getUrl() + ": <component> is missing \"loader\" attribute",
            css::uno::Reference< css::uno::XInterface >());
    }
    if (attrUri_.isEmpty()) {
        throw css::registry::InvalidRegistryException(
            reader_.getUrl() + ": <component> is missing \"uri\" attribute",
            css::uno::Reference< css::uno::XInterface >());
    }
#ifndef DISABLE_DYNLOADING
    try {
        attrUri_ = rtl::Uri::convertRelToAbs(reader_.getUrl(), attrUri_);
    } catch (const rtl::MalformedUriException & e) {
        throw css::registry::InvalidRegistryException(
            reader_.getUrl() + ": bad \"uri\" attribute: " + e.getMessage(),
            css::uno::Reference< css::uno::XInterface >());
    }
#endif
}

void Parser::handleImplementation() {
    attrImplementation_ = getNameAttribute();
    implementation_.reset(
        new Implementation(
            attrImplementation_, attrLoader_, attrUri_, attrPrefix_,
            alienContext_, reader_.getUrl()));
    if (!data_->namedImplementations.insert(
            NamedImplementations::value_type(
                attrImplementation_, implementation_)).
        second)
    {
        throw css::registry::InvalidRegistryException(
            (reader_.getUrl() + ": duplicate <implementation name=\""
             + attrImplementation_ + "\">"),
            css::uno::Reference< css::uno::XInterface >());
    }
}

void Parser::handleService() {
    rtl::OUString name(getNameAttribute());
    implementation_->info->services.push_back(name);
    data_->services[name].push_back(implementation_);
}

void Parser::handleSingleton() {
    rtl::OUString name(getNameAttribute());
    implementation_->info->singletons.push_back(name);
    data_->singletons[name].push_back(implementation_);
}

rtl::OUString Parser::getNameAttribute() {
    rtl::OUString attrName;
    xmlreader::Span name;
    int nsId;
    while (reader_.nextAttribute(&nsId, &name)) {
        if (nsId == xmlreader::XmlReader::NAMESPACE_NONE
            && name.equals(RTL_CONSTASCII_STRINGPARAM("name")))
        {
            if (!attrName.isEmpty()) {
                throw css::registry::InvalidRegistryException(
                    (reader_.getUrl()
                     + ": element has multiple \"name\" attributes"),
                    css::uno::Reference< css::uno::XInterface >());
            }
            attrName = reader_.getAttributeValue(false).convertFromUtf8();
            if (attrName.isEmpty()) {
                throw css::registry::InvalidRegistryException(
                    reader_.getUrl() + ": element has empty \"name\" attribute",
                    css::uno::Reference< css::uno::XInterface >());
            }
        } else {
            throw css::registry::InvalidRegistryException(
                reader_.getUrl() + ": expected element attribute \"name\"",
                css::uno::Reference< css::uno::XInterface >());
        }
    }
    if (attrName.isEmpty()) {
        throw css::registry::InvalidRegistryException(
            reader_.getUrl() + ": element is missing \"name\" attribute",
            css::uno::Reference< css::uno::XInterface >());
    }
    return attrName;
}

class ContentEnumeration:
    public cppu::WeakImplHelper1< css::container::XEnumeration >,
    private boost::noncopyable
{
public:
    explicit ContentEnumeration(std::vector< css::uno::Any > const & factories):
        factories_(factories), iterator_(factories_.begin()) {}

private:
    virtual ~ContentEnumeration() {}

    virtual sal_Bool SAL_CALL hasMoreElements()
        throw (css::uno::RuntimeException);

    virtual css::uno::Any SAL_CALL nextElement()
        throw (
            css::container::NoSuchElementException,
            css::lang::WrappedTargetException, css::uno::RuntimeException);

    osl::Mutex mutex_;
    std::vector< css::uno::Any > factories_;
    std::vector< css::uno::Any >::const_iterator iterator_;
};

sal_Bool ContentEnumeration::hasMoreElements()
    throw (css::uno::RuntimeException)
{
    osl::MutexGuard g(mutex_);
    return iterator_ != factories_.end();
}

css::uno::Any ContentEnumeration::nextElement()
    throw (
        css::container::NoSuchElementException,
        css::lang::WrappedTargetException, css::uno::RuntimeException)
{
    osl::MutexGuard g(mutex_);
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

typedef cppu::WeakComponentImplHelper8<
    css::lang::XServiceInfo, css::lang::XMultiServiceFactory,
    css::lang::XMultiComponentFactory, css::container::XSet,
    css::container::XContentEnumerationAccess, css::beans::XPropertySet,
    css::beans::XPropertySetInfo, css::lang::XEventListener >
ServiceManagerBase;

class ServiceManager:
    private osl::Mutex, public ServiceManagerBase, private boost::noncopyable
{
public:
    explicit ServiceManager(rtl::OUString const & rdbUris):
        ServiceManagerBase(*static_cast< osl::Mutex * >(this))
    { readRdbs(rdbUris); }

    using ServiceManagerBase::acquire;
    using ServiceManagerBase::release;

    void setContext(
        css::uno::Reference< css::uno::XComponentContext > const & context)
    {
        assert(context.is());
        assert(!context_.is());
        context_ = context;
    }

    css::uno::Reference< css::uno::XComponentContext > getContext() const {
        assert(context_.is());
        return context_;
    }

    Data const & getData() const { return data_; }

    void loadImplementation(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        boost::shared_ptr< ImplementationInfo > const & info,
        css::uno::Reference< css::lang::XSingleComponentFactory > * factory1,
        css::uno::Reference< css::lang::XSingleServiceFactory > * factory2);

    virtual rtl::OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService(rtl::OUString const & ServiceName)
        throw (css::uno::RuntimeException);

    virtual css::uno::Sequence< rtl::OUString > SAL_CALL
    getSupportedServiceNames() throw (css::uno::RuntimeException);

    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance(
        rtl::OUString const & aServiceSpecifier)
        throw (css::uno::Exception, css::uno::RuntimeException);

    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL
    createInstanceWithArguments(
        rtl::OUString const & ServiceSpecifier,
        css::uno::Sequence< css::uno::Any > const & Arguments)
        throw (css::uno::Exception, css::uno::RuntimeException);

    virtual css::uno::Sequence< rtl::OUString > SAL_CALL
    getAvailableServiceNames() throw (css::uno::RuntimeException);

    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL
    createInstanceWithContext(
        rtl::OUString const & aServiceSpecifier,
        css::uno::Reference< css::uno::XComponentContext > const & Context)
        throw (css::uno::Exception, css::uno::RuntimeException);

    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL
    createInstanceWithArgumentsAndContext(
        rtl::OUString const & ServiceSpecifier,
        css::uno::Sequence< css::uno::Any > const & Arguments,
        css::uno::Reference< css::uno::XComponentContext > const & Context)
        throw (css::uno::Exception, css::uno::RuntimeException);

    virtual css::uno::Type SAL_CALL getElementType()
        throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL hasElements() throw (css::uno::RuntimeException);

    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
    createEnumeration() throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL has(css::uno::Any const & aElement)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL insert(css::uno::Any const & aElement)
        throw (
            css::lang::IllegalArgumentException,
            css::container::ElementExistException, css::uno::RuntimeException);

    virtual void SAL_CALL remove(css::uno::Any const & aElement)
        throw (
            css::lang::IllegalArgumentException,
            css::container::NoSuchElementException, css::uno::RuntimeException);

    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
    createContentEnumeration(rtl::OUString const & aServiceName)
        throw (css::uno::RuntimeException);

    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
    getPropertySetInfo() throw (css::uno::RuntimeException);

    virtual void SAL_CALL setPropertyValue(
        rtl::OUString const & aPropertyName, css::uno::Any const & aValue)
        throw (
            css::beans::UnknownPropertyException,
            css::beans::PropertyVetoException,
            css::lang::IllegalArgumentException,
            css::lang::WrappedTargetException, css::uno::RuntimeException);

    virtual css::uno::Any SAL_CALL getPropertyValue(
        rtl::OUString const & PropertyName)
        throw (
            css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException, css::uno::RuntimeException);

    virtual void SAL_CALL addPropertyChangeListener(
        rtl::OUString const & aPropertyName,
        css::uno::Reference< css::beans::XPropertyChangeListener > const &
            xListener)
        throw (
            css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException, css::uno::RuntimeException);

    virtual void SAL_CALL removePropertyChangeListener(
        rtl::OUString const & aPropertyName,
        css::uno::Reference< css::beans::XPropertyChangeListener > const &
            aListener)
        throw (
            css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException, css::uno::RuntimeException);

    virtual void SAL_CALL addVetoableChangeListener(
        rtl::OUString const & PropertyName,
        css::uno::Reference< css::beans::XVetoableChangeListener > const &
            aListener)
        throw (
            css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException, css::uno::RuntimeException);

    virtual void SAL_CALL removeVetoableChangeListener(
        rtl::OUString const & PropertyName,
        css::uno::Reference< css::beans::XVetoableChangeListener > const &
            aListener)
        throw (
            css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException, css::uno::RuntimeException);

    virtual css::uno::Sequence< css::beans::Property > SAL_CALL getProperties()
        throw (css::uno::RuntimeException);

    virtual css::beans::Property SAL_CALL getPropertyByName(
        rtl::OUString const & aName)
        throw (
            css::beans::UnknownPropertyException, css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL hasPropertyByName(rtl::OUString const & Name)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL disposing(css::lang::EventObject const & Source)
        throw (css::uno::RuntimeException);

private:
    virtual ~ServiceManager() {}

    virtual void SAL_CALL disposing();

    // needs to be called with rBHelper.rMutex locked:
    bool isDisposed() { return rBHelper.bDisposed || rBHelper.bInDispose; }

    void removeEventListenerFromComponent(
        css::uno::Reference< css::lang::XComponent > const & component);

    void readRdbs(rtl::OUString const & uris);

    void readRdbDirectory(rtl::OUString const & uri, bool optional);

    void readRdbFile(rtl::OUString const & uri, bool optional);

    bool readLegacyRdbFile(rtl::OUString const & uri);

    rtl::OUString readLegacyRdbString(
        rtl::OUString const & uri, RegistryKey & key,
        rtl::OUString const & path);

    void readLegacyRdbStrings(
        rtl::OUString const & uri, RegistryKey & key,
        rtl::OUString const & path, std::vector< rtl::OUString > * strings);

    void insertRdbFiles(
        std::vector< rtl::OUString > const & uris,
        css::uno::Reference< css::uno::XComponentContext > const &
            alientContext);

    void insertLegacyFactory(
        css::uno::Reference< css::lang::XServiceInfo > const & factoryInfo);

    bool insertExtraData(Data const & extra);

    void removeRdbFiles(std::vector< rtl::OUString > const & uris);

    bool removeLegacyFactory(
        css::uno::Reference< css::lang::XServiceInfo > const & factoryInfo,
        bool removeListener);

    void removeImplementation(rtl::OUString name);

    boost::shared_ptr< Implementation > findServiceImplementation(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        rtl::OUString const & specifier);

    css::uno::Reference< css::uno::XComponentContext > context_;
    Data data_;
};

class FactoryWrapper:
    public cppu::WeakImplHelper3<
        css::lang::XSingleComponentFactory, css::lang::XSingleServiceFactory,
        css::lang::XServiceInfo >,
    private boost::noncopyable
{
public:
    FactoryWrapper(
        rtl::Reference< ServiceManager > const & manager,
        boost::shared_ptr< ImplementationInfo > const & info):
        manager_(manager), info_(info), loaded_(false)
    { assert(manager.is() && info.get() != 0); }

private:
    virtual ~FactoryWrapper() {}

    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL
    createInstanceWithContext(
        css::uno::Reference< css::uno::XComponentContext > const & Context)
        throw (css::uno::Exception, css::uno::RuntimeException);

    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL
    createInstanceWithArgumentsAndContext(
        css::uno::Sequence< css::uno::Any > const & Arguments,
        css::uno::Reference< css::uno::XComponentContext > const & Context)
        throw (css::uno::Exception, css::uno::RuntimeException);

    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL
    createInstance() throw (css::uno::Exception, css::uno::RuntimeException);

    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL
    createInstanceWithArguments(
        css::uno::Sequence< css::uno::Any > const & Arguments)
        throw (css::uno::Exception, css::uno::RuntimeException);

    virtual rtl::OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService(rtl::OUString const & ServiceName)
        throw (css::uno::RuntimeException);

    virtual css::uno::Sequence< rtl::OUString > SAL_CALL
    getSupportedServiceNames() throw (css::uno::RuntimeException);

    void loadImplementation(
        css::uno::Reference< css::uno::XComponentContext > const & context);

    rtl::Reference< ServiceManager > manager_;
    boost::shared_ptr< ImplementationInfo > info_;

    osl::Mutex mutex_;
    bool loaded_;
    css::uno::Reference< css::lang::XSingleComponentFactory > factory1_;
    css::uno::Reference< css::lang::XSingleServiceFactory > factory2_;
};

void ServiceManager::loadImplementation(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        boost::shared_ptr< ImplementationInfo > const & info,
        css::uno::Reference< css::lang::XSingleComponentFactory > * factory1,
        css::uno::Reference< css::lang::XSingleServiceFactory > * factory2)
{
    assert(
        info.get() != 0 && factory1 != 0 && !factory1->is() && factory2 != 0
        && !factory2->is());
    rtl::OUString uri;
    try {
        uri = cppu::bootstrap_expandUri(info->uri);
    } catch (css::lang::IllegalArgumentException & e) {
        throw css::uno::DeploymentException(
            "Cannot expand URI" + info->uri + ": " + e.Message,
            static_cast< cppu::OWeakObject * >(this));
    }
    css::uno::Reference< css::uno::XInterface > f0;
    // Shortcut loading via SharedLibrary loader, to pass in prefix argument
    // (which the loader's activate implementation would normally obtain through
    // the legacy xKey argument):
    if (!info->alienContext.is()
        && info->loader == "com.sun.star.loader.SharedLibrary")
    {
        rtl::OUString prefix(info->prefix);
        if (!prefix.isEmpty()) {
            prefix += "_";
        }
        f0 = cppu::loadSharedLibComponentFactory(
            uri, rtl::OUString(), info->name, this,
            css::uno::Reference< css::registry::XRegistryKey >(), prefix);
    } else {
        SAL_INFO_IF(
            !info->prefix.isEmpty(), "cppuhelper",
            "Loader " << info->loader << " and non-empty prefix "
                << info->prefix);
        css::uno::Reference< css::uno::XComponentContext > ctxt;
        css::uno::Reference< css::lang::XMultiComponentFactory > smgr;
        if (info->alienContext.is()) {
            ctxt = info->alienContext;
            smgr = css::uno::Reference< css::lang::XMultiComponentFactory >(
                ctxt->getServiceManager(), css::uno::UNO_SET_THROW);
        } else {
            assert(context.is());
            ctxt = context;
            smgr = this;
        }
        css::uno::Reference< css::loader::XImplementationLoader > loader(
            smgr->createInstanceWithContext(info->loader, ctxt),
            css::uno::UNO_QUERY_THROW);
        f0 = loader->activate(
            info->name, rtl::OUString(), uri,
            css::uno::Reference< css::registry::XRegistryKey >());
    }
    factory1->set(f0, css::uno::UNO_QUERY);
    if (!factory1->is()) {
        factory2->set(f0, css::uno::UNO_QUERY);
    }
}

rtl::OUString ServiceManager::getImplementationName()
    throw (css::uno::RuntimeException)
{
    return rtl::OUString(
        "com.sun.star.comp.cppuhelper.bootstrap.ServiceManager");
}

sal_Bool ServiceManager::supportsService(rtl::OUString const & ServiceName)
    throw (css::uno::RuntimeException)
{
    css::uno::Sequence< rtl::OUString > names(getSupportedServiceNames());
    for (sal_Int32 i = 0; i != names.getLength(); ++i) {
        if (ServiceName == names[i]) {
            return true;
        }
    }
    return false;
}

css::uno::Sequence< rtl::OUString > ServiceManager::getSupportedServiceNames()
    throw (css::uno::RuntimeException)
{
    css::uno::Sequence< rtl::OUString > names(2);
    names[0] = "com.sun.star.lang.MultiServiceFactory";
    names[1] = "com.sun.star.lang.ServiceManager";
    return names;
}

css::uno::Reference< css::uno::XInterface > ServiceManager::createInstance(
    rtl::OUString const & aServiceSpecifier)
    throw (css::uno::Exception, css::uno::RuntimeException)
{
    assert(context_.is());
    return createInstanceWithContext(aServiceSpecifier, context_);
}

css::uno::Reference< css::uno::XInterface >
ServiceManager::createInstanceWithArguments(
    rtl::OUString const & ServiceSpecifier,
    css::uno::Sequence< css::uno::Any > const & Arguments)
    throw (css::uno::Exception, css::uno::RuntimeException)
{
    assert(context_.is());
    return createInstanceWithArgumentsAndContext(
        ServiceSpecifier, Arguments, context_);
}

css::uno::Sequence< rtl::OUString > ServiceManager::getAvailableServiceNames()
    throw (css::uno::RuntimeException)
{
    osl::MutexGuard g(rBHelper.rMutex);
    if (isDisposed()) {
        return css::uno::Sequence< rtl::OUString >();
    }
    ImplementationMap::size_type n = data_.services.size();
    if (n > static_cast< sal_uInt32 >(SAL_MAX_INT32)) {
        throw css::uno::RuntimeException(
            "getAvailableServiceNames: too many services",
            static_cast< cppu::OWeakObject * >(this));
    }
    css::uno::Sequence< rtl::OUString > names(static_cast< sal_Int32 >(n));
    sal_Int32 i = 0;
    for (ImplementationMap::const_iterator j(data_.services.begin());
         j != data_.services.end(); ++j)
    {
        names[i++] = j->first;
    }
    assert(i == names.getLength());
    return names;
}

css::uno::Reference< css::uno::XInterface >
ServiceManager::createInstanceWithContext(
    rtl::OUString const & aServiceSpecifier,
    css::uno::Reference< css::uno::XComponentContext > const & Context)
    throw (css::uno::Exception, css::uno::RuntimeException)
{
    boost::shared_ptr< Implementation > impl(
        findServiceImplementation(Context, aServiceSpecifier));
    if (impl.get() == 0) {
        return css::uno::Reference< css::uno::XInterface >();
    }
    if (impl->factory1.is()) {
        return impl->factory1->createInstanceWithContext(Context);
    }
    if (impl->factory2.is()) {
        return impl->factory2->createInstance();
    }
    throw css::uno::DeploymentException(
        "Implementation " + impl->info->name + " does not provide a factory",
        static_cast< cppu::OWeakObject * >(this));
}

css::uno::Reference< css::uno::XInterface >
ServiceManager::createInstanceWithArgumentsAndContext(
    rtl::OUString const & ServiceSpecifier,
    css::uno::Sequence< css::uno::Any > const & Arguments,
    css::uno::Reference< css::uno::XComponentContext > const & Context)
    throw (css::uno::Exception, css::uno::RuntimeException)
{
    boost::shared_ptr< Implementation > impl(
        findServiceImplementation(Context, ServiceSpecifier));
    if (impl.get() == 0) {
        return css::uno::Reference< css::uno::XInterface >();
    }
    if (impl->factory1.is()) {
        return impl->factory1->createInstanceWithArgumentsAndContext(
            Arguments, Context);
    }
    if (impl->factory2.is()) {
        return impl->factory2->createInstanceWithArguments(Arguments);
    }
    throw css::uno::DeploymentException(
        "Implementation " + impl->info->name + " does not provide a factory",
        static_cast< cppu::OWeakObject * >(this));
}

css::uno::Type ServiceManager::getElementType()
    throw (css::uno::RuntimeException)
{
    return css::uno::Type();
}

sal_Bool ServiceManager::hasElements() throw (css::uno::RuntimeException) {
    osl::MutexGuard g(rBHelper.rMutex);
    return
        !(data_.namedImplementations.empty()
          && data_.dynamicImplementations.empty());
}

css::uno::Reference< css::container::XEnumeration >
ServiceManager::createEnumeration() throw (css::uno::RuntimeException) {
    throw css::uno::RuntimeException(
        "ServiceManager createEnumeration: method not supported",
        static_cast< cppu::OWeakObject * >(this));
}

sal_Bool ServiceManager::has(css::uno::Any const &)
    throw (css::uno::RuntimeException)
{
    throw css::uno::RuntimeException(
        "ServiceManager has: method not supported",
        static_cast< cppu::OWeakObject * >(this));
}

void ServiceManager::insert(css::uno::Any const & aElement)
    throw (
        css::lang::IllegalArgumentException,
        css::container::ElementExistException, css::uno::RuntimeException)
{
    css::uno::Sequence< css::beans::NamedValue > args;
    if (aElement >>= args) {
        std::vector< rtl::OUString > uris;
        css::uno::Reference< css::uno::XComponentContext > alienContext;
        for (sal_Int32 i = 0; i < args.getLength(); ++i) {
            if (args[i].Name == "uri") {
                rtl::OUString uri;
                if (!(args[i].Value >>= uri)) {
                    throw css::lang::IllegalArgumentException(
                        "Bad uri argument",
                        static_cast< cppu::OWeakObject * >(this), 0);
                }
                uris.push_back(uri);
            } else if (args[i].Name == "component-context") {
                if (alienContext.is()) {
                    throw css::lang::IllegalArgumentException(
                        "Multiple component-context arguments",
                        static_cast< cppu::OWeakObject * >(this), 0);
                }
                if (!(args[i].Value >>= alienContext) || !alienContext.is()) {
                    throw css::lang::IllegalArgumentException(
                        "Bad component-context argument",
                        static_cast< cppu::OWeakObject * >(this), 0);
                }
            } else {
                throw css::lang::IllegalArgumentException(
                    "Bad argument " + args[i].Name,
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
// At least revisions up to 1.7 of LanguageTool.oxt (incl. the bundled 1.4.0 in
// module languagetool) contain an (actively registered) factory that does not
// implement XServiceInfo (see <http://sourceforge.net/tracker/?
// func=detail&aid=3526635&group_id=110216&atid=655717> "SingletonFactory should
// implement XServiceInfo"); the old OServiceManager::insert
// (stoc/source/servicemanager/servicemanager.cxx) silently did not add such
// broken factories to its m_ImplementationNameMap, so ignore them here for
// backwards compatibility of live-insertion of extensions, too (can go again
// for incompatible LO 4):
#if SUPD < 400
    css::uno::Reference< css::lang::XSingleComponentFactory > legacy;
    if ((aElement >>= legacy) && legacy.is()) {
        SAL_WARN(
            "cppuhelper",
            "Ignored XSingleComponentFactory not implementing XServiceInfo");
        return;
    }
#endif
    throw css::lang::IllegalArgumentException(
        "Bad insert element", static_cast< cppu::OWeakObject * >(this), 0);
}

void ServiceManager::remove(css::uno::Any const & aElement)
    throw (
        css::lang::IllegalArgumentException,
        css::container::NoSuchElementException, css::uno::RuntimeException)
{
    css::uno::Sequence< css::beans::NamedValue > args;
    if (aElement >>= args) {
        std::vector< rtl::OUString > uris;
        for (sal_Int32 i = 0; i < args.getLength(); ++i) {
            if (args[i].Name == "uri") {
                rtl::OUString uri;
                if (!(args[i].Value >>= uri)) {
                    throw css::lang::IllegalArgumentException(
                        "Bad uri argument",
                        static_cast< cppu::OWeakObject * >(this), 0);
                }
                uris.push_back(uri);
            } else {
                throw css::lang::IllegalArgumentException(
                    "Bad argument " + args[i].Name,
                    static_cast< cppu::OWeakObject * >(this), 0);
            }
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
    rtl::OUString impl;
    if (aElement >>= impl) {
        // For live-removal of extensions:
        removeImplementation(impl);
        return;
    }
    throw css::lang::IllegalArgumentException(
        "Bad remove element", static_cast< cppu::OWeakObject * >(this), 0);
}

css::uno::Reference< css::container::XEnumeration >
ServiceManager::createContentEnumeration(rtl::OUString const & aServiceName)
    throw (css::uno::RuntimeException)
{
    std::vector< boost::shared_ptr< Implementation > > impls;
    {
        osl::MutexGuard g(rBHelper.rMutex);
        ImplementationMap::const_iterator i(data_.services.find(aServiceName));
        if (i != data_.services.end()) {
            impls = i->second;
        }
    }
    std::vector< css::uno::Any > factories;
    for (std::vector< boost::shared_ptr< Implementation > >::const_iterator i(
             impls.begin());
         i != impls.end(); ++i)
    {
        Implementation * impl = i->get();
        assert(impl != 0);
        {
            osl::MutexGuard g(rBHelper.rMutex);
            if (isDisposed()) {
                factories.clear();
                break;
            }
            if (!impl->loaded) {
                // Postpone actual factory instantiation as long as possible (so
                // that e.g. opening LO's "Tools - Macros" menu does not try to
                // instantiate a JVM, which can lead to a synchronous error
                // dialog when no JVM is specified, and showing the dialog while
                // hovering over a menu can cause trouble):
                impl->factory1 = new FactoryWrapper(this, impl->info);
                impl->loaded = true;
            }
        }
        if (impl->factory1.is()) {
            factories.push_back(css::uno::makeAny(impl->factory1));
        } else if (impl->factory2.is()) {
            factories.push_back(css::uno::makeAny(impl->factory2));
        } else {
            throw css::uno::DeploymentException(
                ("Implementation " + impl->info->name
                 + " does not provide a factory"),
                static_cast< cppu::OWeakObject * >(this));
        }
    }
    return new ContentEnumeration(factories);
}

css::uno::Reference< css::beans::XPropertySetInfo >
ServiceManager::getPropertySetInfo() throw (css::uno::RuntimeException) {
    return this;
}

void ServiceManager::setPropertyValue(
    rtl::OUString const & aPropertyName, css::uno::Any const &)
    throw (
        css::beans::UnknownPropertyException, css::beans::PropertyVetoException,
        css::lang::IllegalArgumentException, css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    if (aPropertyName == "DefaultContext") {
        throw css::beans::PropertyVetoException(
            aPropertyName, static_cast< cppu::OWeakObject * >(this));
    } else {
        throw css::beans::UnknownPropertyException(
            aPropertyName, static_cast< cppu::OWeakObject * >(this));
    }
}

css::uno::Any ServiceManager::getPropertyValue(
    rtl::OUString const & PropertyName)
    throw (
        css::beans::UnknownPropertyException, css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    if (PropertyName != "DefaultContext") {
        throw css::beans::UnknownPropertyException(
            PropertyName, static_cast< cppu::OWeakObject * >(this));
    }
    assert(context_.is());
    return css::uno::makeAny(context_);
}

void ServiceManager::addPropertyChangeListener(
    rtl::OUString const & aPropertyName,
    css::uno::Reference< css::beans::XPropertyChangeListener > const &
        xListener)
    throw (
        css::beans::UnknownPropertyException, css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    if (!aPropertyName.isEmpty() && aPropertyName != "DefaultContext") {
        throw css::beans::UnknownPropertyException(
            aPropertyName, static_cast< cppu::OWeakObject * >(this));
    }
    // DefaultContext does not change, so just treat it as an event listener:
    return addEventListener(xListener.get());
}

void ServiceManager::removePropertyChangeListener(
    rtl::OUString const & aPropertyName,
    css::uno::Reference< css::beans::XPropertyChangeListener > const &
        aListener)
    throw (
        css::beans::UnknownPropertyException, css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    if (!aPropertyName.isEmpty() && aPropertyName != "DefaultContext") {
        throw css::beans::UnknownPropertyException(
            aPropertyName, static_cast< cppu::OWeakObject * >(this));
    }
    // DefaultContext does not change, so just treat it as an event listener:
    return removeEventListener(aListener.get());
}

void ServiceManager::addVetoableChangeListener(
    rtl::OUString const & PropertyName,
    css::uno::Reference< css::beans::XVetoableChangeListener > const &
        aListener)
    throw (
        css::beans::UnknownPropertyException, css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    if (!PropertyName.isEmpty() && PropertyName != "DefaultContext") {
        throw css::beans::UnknownPropertyException(
            PropertyName, static_cast< cppu::OWeakObject * >(this));
    }
    // DefaultContext does not change, so just treat it as an event listener:
    return addEventListener(aListener.get());
}

void ServiceManager::removeVetoableChangeListener(
    rtl::OUString const & PropertyName,
    css::uno::Reference< css::beans::XVetoableChangeListener > const &
        aListener)
    throw (
        css::beans::UnknownPropertyException, css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    if (!PropertyName.isEmpty() && PropertyName != "DefaultContext") {
        throw css::beans::UnknownPropertyException(
            PropertyName, static_cast< cppu::OWeakObject * >(this));
    }
    // DefaultContext does not change, so just treat it as an event listener:
    return removeEventListener(aListener.get());
}

css::uno::Sequence< css::beans::Property > ServiceManager::getProperties()
    throw (css::uno::RuntimeException)
{
    css::uno::Sequence< css::beans::Property > props(1);
    props[0] = getDefaultContextProperty();
    return props;
}

css::beans::Property ServiceManager::getPropertyByName(
    rtl::OUString const & aName)
    throw (css::beans::UnknownPropertyException, css::uno::RuntimeException)
{
    if (aName != "DefaultContext") {
        throw css::beans::UnknownPropertyException(
            aName, static_cast< cppu::OWeakObject * >(this));
    }
    return getDefaultContextProperty();
}

sal_Bool ServiceManager::hasPropertyByName(rtl::OUString const & Name)
    throw (css::uno::RuntimeException)
{
    return Name == "DefaultContext";
}

void ServiceManager::disposing(css::lang::EventObject const & Source)
    throw (css::uno::RuntimeException)
{
    removeLegacyFactory(
        css::uno::Reference< css::lang::XServiceInfo >(
            Source.Source, css::uno::UNO_QUERY_THROW),
        false);
}

void ServiceManager::disposing() {
    std::vector< css::uno::Reference< css::lang::XComponent > > comps;
    Data clear;
    {
        osl::MutexGuard g(rBHelper.rMutex);
        for (DynamicImplementations::const_iterator i(
                 data_.dynamicImplementations.begin());
             i != data_.dynamicImplementations.end(); ++i)
        {
            assert(i->second.get() != 0);
            if (i->second->component.is()) {
                comps.push_back(i->second->component);
            }
        }
        data_.namedImplementations.swap(clear.namedImplementations);
        data_.dynamicImplementations.swap(clear.dynamicImplementations);
        data_.services.swap(clear.services);
        data_.singletons.swap(clear.singletons);
    }
    for (std::vector<
             css::uno::Reference< css::lang::XComponent > >::const_iterator i(
                 comps.begin());
         i != comps.end(); ++i)
    {
        removeEventListenerFromComponent(*i);
    }
}

void ServiceManager::removeEventListenerFromComponent(
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

void ServiceManager::readRdbs(rtl::OUString const & uris) {
    for (sal_Int32 i = 0; i != -1;) {
        rtl::OUString uri(uris.getToken(0, ' ', i));
        if (uri.isEmpty()) {
            continue;
        }
        bool optional;
        bool directory;
        decodeRdbUri(&uri, &optional, &directory);
        if (directory) {
            readRdbDirectory(uri, optional);
        } else {
            readRdbFile(uri, optional);
        }
    }
}

void ServiceManager::readRdbDirectory(rtl::OUString const & uri, bool optional)
{
    osl::Directory dir(uri);
    switch (dir.open()) {
    case osl::FileBase::E_None:
        break;
    case osl::FileBase::E_NOENT:
        if (optional) {
            SAL_INFO("cppuhelper", "Ignored optional " << uri);
            return;
        }
        // fall through
    default:
        throw css::uno::DeploymentException(
            "Cannot open directory " + uri,
            static_cast< cppu::OWeakObject * >(this));
    }
    for (;;) {
        rtl::OUString url;
        if (!nextDirectoryItem(dir, &url)) {
            break;
        }
        readRdbFile(url, false);
    }
}

void ServiceManager::readRdbFile(rtl::OUString const & uri, bool optional) {
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
    } catch (css::registry::InvalidRegistryException & e) {
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
}

bool ServiceManager::readLegacyRdbFile(rtl::OUString const & uri) {
    Registry reg;
    switch (reg.open(uri, REG_READONLY)) {
    case REG_NO_ERROR:
        break;
    case REG_REGISTRY_NOT_EXISTS:
    case REG_INVALID_REGISTRY:
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
        // fall through
    default:
        return false;
    }
    RegistryKey rootKey;
    if (reg.openRootKey(rootKey) != REG_NO_ERROR) {
        throw css::uno::DeploymentException(
            "Failure reading legacy rdb file " + uri,
            static_cast< cppu::OWeakObject * >(this));
    }
    RegistryKeyArray impls;
    switch (rootKey.openSubKeys("IMPLEMENTATIONS", impls)) {
    case REG_NO_ERROR:
        break;
    case REG_KEY_NOT_EXISTS:
        return true;
    default:
        throw css::uno::DeploymentException(
            "Failure reading legacy rdb file " + uri,
            static_cast< cppu::OWeakObject * >(this));
    }
    for (sal_uInt32 i = 0; i != impls.getLength(); ++i) {
        RegistryKey implKey(impls.getElement(i));
        assert(implKey.getName().match("/IMPLEMENTATIONS/"));
        rtl::OUString name(
            implKey.getName().copy(RTL_CONSTASCII_LENGTH("/IMPLEMENTATIONS/")));
        boost::shared_ptr< Implementation > impl(
            new Implementation(
                name, readLegacyRdbString(uri, implKey, "UNO/ACTIVATOR"),
                readLegacyRdbString(uri, implKey, "UNO/LOCATION"),
                rtl::OUString(),
                css::uno::Reference< css::uno::XComponentContext >(), uri));
        if (!data_.namedImplementations.insert(
                NamedImplementations::value_type(name, impl)).
            second)
        {
            throw css::registry::InvalidRegistryException(
                uri + ": duplicate <implementation name=\"" + name + "\">",
                css::uno::Reference< css::uno::XInterface >());
        }
        readLegacyRdbStrings(
            uri, implKey, "UNO/SERVICES", &impl->info->services);
        for (std::vector< rtl::OUString >::const_iterator j(
                 impl->info->services.begin());
             j != impl->info->services.end(); ++j)
        {
            data_.services[*j].push_back(impl);
        }
        readLegacyRdbStrings(
            uri, implKey, "UNO/SINGLETONS", &impl->info->singletons);
        for (std::vector< rtl::OUString >::const_iterator j(
                 impl->info->singletons.begin());
             j != impl->info->singletons.end(); ++j)
        {
            data_.singletons[*j].push_back(impl);
        }
    }
    return true;
}

rtl::OUString ServiceManager::readLegacyRdbString(
    rtl::OUString const & uri, RegistryKey & key, rtl::OUString const & path)
{
    RegistryKey subkey;
    RegValueType t;
    sal_uInt32 s(0);
    if (key.openKey(path, subkey) != REG_NO_ERROR
        || subkey.getValueInfo(rtl::OUString(), &t, &s) != REG_NO_ERROR
        || t != RG_VALUETYPE_STRING
        || s == 0 || s > static_cast< sal_uInt32 >(SAL_MAX_INT32))
    {
        throw css::uno::DeploymentException(
            "Failure reading legacy rdb file " + uri,
            static_cast< cppu::OWeakObject * >(this));
    }
    rtl::OUString val;
    std::vector< char > v(s); // assuming sal_uInt32 fits into vector::size_type
    if (subkey.getValue(rtl::OUString(), &v[0]) != REG_NO_ERROR
        || v.back() != '\0'
        || !rtl_convertStringToUString(
            &val.pData, &v[0], static_cast< sal_Int32 >(s - 1),
            RTL_TEXTENCODING_UTF8,
            (RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR
             | RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR
             | RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR)))
    {
        throw css::uno::DeploymentException(
            "Failure reading legacy rdb file " + uri,
            static_cast< cppu::OWeakObject * >(this));
    }
    return val;
}

void ServiceManager::readLegacyRdbStrings(
    rtl::OUString const & uri, RegistryKey & key, rtl::OUString const & path,
    std::vector< rtl::OUString > * strings)
{
    assert(strings != 0);
    RegistryKey subkey;
    switch (key.openKey(path, subkey)) {
    case REG_NO_ERROR:
        break;
    case REG_KEY_NOT_EXISTS:
        return;
    default:
        throw css::uno::DeploymentException(
            "Failure reading legacy rdb file " + uri,
            static_cast< cppu::OWeakObject * >(this));
    }
    rtl::OUString prefix(subkey.getName() + "/");
    RegistryKeyNames names;
    if (subkey.getKeyNames(rtl::OUString(), names) != REG_NO_ERROR) {
        throw css::uno::DeploymentException(
            "Failure reading legacy rdb file " + uri,
            static_cast< cppu::OWeakObject * >(this));
    }
    for (sal_uInt32 i = 0; i != names.getLength(); ++i) {
        assert(names.getElement(i).match(prefix));
        strings->push_back(names.getElement(i).copy(prefix.getLength()));
    }
}

void ServiceManager::insertRdbFiles(
    std::vector< rtl::OUString > const & uris,
    css::uno::Reference< css::uno::XComponentContext > const & alienContext)
{
    Data extra;
    for (std::vector< rtl::OUString >::const_iterator i(uris.begin());
         i != uris.end(); ++i)
    {
        try {
            Parser(*i, alienContext, &extra);
        } catch (css::container::NoSuchElementException &) {
            throw css::lang::IllegalArgumentException(
                *i + ": no such file", static_cast< cppu::OWeakObject * >(this),
                0);
        } catch (css::registry::InvalidRegistryException & e) {
            throw css::lang::IllegalArgumentException(
                "InvalidRegistryException: " + e.Message,
                static_cast< cppu::OWeakObject * >(this), 0);
        }
    }
    insertExtraData(extra);
}

void ServiceManager::insertLegacyFactory(
    css::uno::Reference< css::lang::XServiceInfo > const & factoryInfo)
{
    assert(factoryInfo.is());
    rtl::OUString name(factoryInfo->getImplementationName());
    css::uno::Reference< css::lang::XSingleComponentFactory > f1(
        factoryInfo, css::uno::UNO_QUERY);
    css::uno::Reference< css::lang::XSingleServiceFactory > f2;
    if (!f1.is()) {
        f2 = css::uno::Reference< css::lang::XSingleServiceFactory >(
            factoryInfo, css::uno::UNO_QUERY);
    }
    css::uno::Reference< css::lang::XComponent > comp(
        factoryInfo, css::uno::UNO_QUERY);
    boost::shared_ptr< Implementation > impl(
        new Implementation(name, f1, f2, comp));
    Data extra;
    if (!name.isEmpty()) {
        extra.namedImplementations.insert(
            NamedImplementations::value_type(name, impl));
    }
    extra.dynamicImplementations.insert(
        DynamicImplementations::value_type(factoryInfo, impl));
    css::uno::Sequence< rtl::OUString > services(
        factoryInfo->getSupportedServiceNames());
    for (sal_Int32 i = 0; i != services.getLength(); ++i) {
        impl->info->services.push_back(services[i]);
        extra.services[services[i]].push_back(impl);
    }
    if (insertExtraData(extra) && comp.is()) {
        comp->addEventListener(this);
    }
}

bool ServiceManager::insertExtraData(Data const & extra) {
    {
        osl::MutexGuard g(rBHelper.rMutex);
        if (isDisposed()) {
            return false;
        }
        for (NamedImplementations::const_iterator i(
                 extra.namedImplementations.begin());
             i != extra.namedImplementations.end(); ++i)
        {
            if (data_.namedImplementations.find(i->first)
                != data_.namedImplementations.end())
            {
                throw css::lang::IllegalArgumentException(
                    "Insert duplicate implementation name " + i->first,
                    static_cast< cppu::OWeakObject * >(this), 0);
            }
        }
        for (DynamicImplementations::const_iterator i(
                 extra.dynamicImplementations.begin());
             i != extra.dynamicImplementations.end(); ++i)
        {
            if (data_.dynamicImplementations.find(i->first)
                != data_.dynamicImplementations.end())
            {
                throw css::lang::IllegalArgumentException(
                    "Insert duplicate factory object",
                    static_cast< cppu::OWeakObject * >(this), 0);
            }
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
    if (!extra.singletons.empty()) {
        assert(context_.is());
        css::uno::Reference< css::container::XNameContainer > cont(
            context_, css::uno::UNO_QUERY_THROW);
        for (ImplementationMap::const_iterator i(extra.singletons.begin());
             i != extra.singletons.end(); ++i)
        {
            rtl::OUString name("/singletons/" + i->first);
            //TODO: Update should be atomic:
            try {
                cont->removeByName(name + "/arguments");
            } catch (const css::container::NoSuchElementException &) {}
            assert(!i->second.empty());
            assert(i->second[0].get() != 0);
            SAL_INFO_IF(
                i->second.size() > 1, "cppuhelper",
                "Arbitrarily chosing " << i->second[0]->info->name
                    << " among multiple implementations for singleton "
                    << i->first);
            try {
                cont->insertByName(
                    name + "/service", css::uno::Any(i->second[0]->info->name));
            } catch (css::container::ElementExistException &) {
                cont->replaceByName(
                    name + "/service", css::uno::Any(i->second[0]->info->name));
            }
            try {
                cont->insertByName(name, css::uno::Any());
            } catch (css::container::ElementExistException &) {
                SAL_INFO("cppuhelper", "Overwriting singleton " << i->first);
                cont->replaceByName(name, css::uno::Any());
            }
        }
    }
    return true;
}

void ServiceManager::removeRdbFiles(std::vector< rtl::OUString > const & uris) {
    // The underlying data structures make this function somewhat inefficient,
    // but the assumption is that it is rarely called (and that if it is called,
    // it is called with a uris vector of size one):
    std::vector< boost::shared_ptr< Implementation > > clear;
    {
        osl::MutexGuard g(rBHelper.rMutex);
        for (std::vector< rtl::OUString >::const_iterator i(uris.begin());
             i != uris.end(); ++i)
        {
            for (NamedImplementations::iterator j(
                     data_.namedImplementations.begin());
                 j != data_.namedImplementations.end();)
            {
                assert(j->second.get() != 0);
                if (j->second->info->rdbFile == *i) {
                    clear.push_back(j->second);
                    //TODO: The below leaves data_ in an inconsistent state upon
                    // exceptions:
                    removeFromImplementationMap(
                        &data_.services, j->second->info->services, j->second);
                    removeFromImplementationMap(
                        &data_.singletons, j->second->info->singletons,
                        j->second);
                    data_.namedImplementations.erase(j++);
                } else {
                    ++j;
                }
            }
        }
    }
    //TODO: Update the component context singleton data
}

bool ServiceManager::removeLegacyFactory(
    css::uno::Reference< css::lang::XServiceInfo > const & factoryInfo,
    bool removeListener)
{
    assert(factoryInfo.is());
    boost::shared_ptr< Implementation > clear;
    css::uno::Reference< css::lang::XComponent > comp;
    {
        osl::MutexGuard g(rBHelper.rMutex);
        DynamicImplementations::iterator i(
            data_.dynamicImplementations.find(factoryInfo));
        if (i == data_.dynamicImplementations.end()) {
            return isDisposed();
        }
        assert(i->second.get() != 0);
        clear = i->second;
        //TODO: The below leaves data_ in an inconsistent state upon exceptions:
        removeFromImplementationMap(
            &data_.services, i->second->info->services, i->second);
        removeFromImplementationMap(
            &data_.singletons, i->second->info->singletons, i->second);
        if (!i->second->info->name.isEmpty()) {
            data_.namedImplementations.erase(i->second->info->name);
        }
        data_.dynamicImplementations.erase(i);
        if (removeListener) {
            comp = i->second->component;
        }
    }
    if (comp.is()) {
        removeEventListenerFromComponent(comp);
    }
    return true;
}

void ServiceManager::removeImplementation(rtl::OUString name) {
    // The underlying data structures make this function somewhat inefficient,
    // but the assumption is that it is rarely called:
    boost::shared_ptr< Implementation > clear;
    {
        osl::MutexGuard g(rBHelper.rMutex);
        if (isDisposed()) {
            return;
        }
        NamedImplementations::iterator i(data_.namedImplementations.find(name));
        if (i == data_.namedImplementations.end()) {
            throw css::container::NoSuchElementException(
                "Remove non-inserted implementation " + name,
                static_cast< cppu::OWeakObject * >(this));
        }
        assert(i->second.get() != 0);
        clear = i->second;
        //TODO: The below leaves data_ in an inconsistent state upon exceptions:
        removeFromImplementationMap(
            &data_.services, i->second->info->services, i->second);
        removeFromImplementationMap(
            &data_.singletons, i->second->info->singletons, i->second);
        for (DynamicImplementations::iterator j(
                 data_.dynamicImplementations.begin());
             j != data_.dynamicImplementations.end(); ++j)
        {
            if (j->second == i->second) {
                data_.dynamicImplementations.erase(j);
                break;
            }
        }
        data_.namedImplementations.erase(i);
    }
}

boost::shared_ptr< Implementation > ServiceManager::findServiceImplementation(
    css::uno::Reference< css::uno::XComponentContext > const & context,
    rtl::OUString const & specifier)
{
    boost::shared_ptr< Implementation > impl;
    bool loaded;
    {
        osl::MutexGuard g(rBHelper.rMutex);
        ImplementationMap::const_iterator i(data_.services.find(specifier));
        if (i == data_.services.end()) {
            NamedImplementations::const_iterator j(
                data_.namedImplementations.find(specifier));
            if (j == data_.namedImplementations.end()) {
                SAL_INFO("cppuhelper", "No implementation for " << specifier);
                return boost::shared_ptr< Implementation >();
            }
            impl = j->second;
        } else {
            assert(!i->second.empty());
            SAL_INFO_IF(
                i->second.size() > 1, "cppuhelper",
                "Arbitrarily chosing " << i->second[0]->info->name
                    << " among multiple implementations for " << i->first);
            impl = i->second[0];
        }
        assert(impl.get() != 0);
        loaded = impl->loaded;
    }
    //TODO: There is a race here, as the relevant service factory can be removed
    // while the mutex is unlocked and loading can thus fail, as the entity from
    // which to load can disappear once the service factory is removed.
    if (!loaded) {
        css::uno::Reference< css::lang::XSingleComponentFactory > f1;
        css::uno::Reference< css::lang::XSingleServiceFactory > f2;
        loadImplementation(context, impl->info, &f1, &f2);
        osl::MutexGuard g(rBHelper.rMutex);
        if (!(isDisposed() || impl->loaded)) {
            impl->loaded = true;
            impl->factory1 = f1;
            impl->factory2 = f2;
        }
    }
    return impl;
}

css::uno::Reference< css::uno::XInterface >
FactoryWrapper::createInstanceWithContext(
    css::uno::Reference< css::uno::XComponentContext > const & Context)
    throw (css::uno::Exception, css::uno::RuntimeException)
{
    loadImplementation(Context);
    return factory1_.is()
        ? factory1_->createInstanceWithContext(Context)
        : factory2_->createInstance();
}

css::uno::Reference< css::uno::XInterface >
FactoryWrapper::createInstanceWithArgumentsAndContext(
    css::uno::Sequence< css::uno::Any > const & Arguments,
    css::uno::Reference< css::uno::XComponentContext > const & Context)
    throw (css::uno::Exception, css::uno::RuntimeException)
{
    loadImplementation(Context);
    return factory1_.is()
        ? factory1_->createInstanceWithArgumentsAndContext(Arguments, Context)
        : factory2_->createInstanceWithArguments(Arguments);
}

css::uno::Reference< css::uno::XInterface > FactoryWrapper::createInstance()
    throw (css::uno::Exception, css::uno::RuntimeException)
{
    loadImplementation(manager_->getContext());
    return factory1_.is()
        ? factory1_->createInstanceWithContext(manager_->getContext())
        : factory2_->createInstance();
}

css::uno::Reference< css::uno::XInterface >
FactoryWrapper::createInstanceWithArguments(
    css::uno::Sequence< css::uno::Any > const & Arguments)
    throw (css::uno::Exception, css::uno::RuntimeException)
{
    loadImplementation(manager_->getContext());
    return factory1_.is()
        ? factory1_->createInstanceWithArgumentsAndContext(
            Arguments, manager_->getContext())
        : factory2_->createInstanceWithArguments(Arguments);
}

rtl::OUString FactoryWrapper::getImplementationName()
    throw (css::uno::RuntimeException)
{
    return info_->name;
}

sal_Bool FactoryWrapper::supportsService(rtl::OUString const & ServiceName)
    throw (css::uno::RuntimeException)
{
    css::uno::Sequence< rtl::OUString > names(getSupportedServiceNames());
    for (sal_Int32 i = 0; i != names.getLength(); ++i) {
        if (ServiceName == names[i]) {
            return true;
        }
    }
    return false;
}

css::uno::Sequence< rtl::OUString > FactoryWrapper::getSupportedServiceNames()
    throw (css::uno::RuntimeException)
{
    if (info_->services.size() > static_cast< sal_uInt32 >(SAL_MAX_INT32)) {
        throw css::uno::RuntimeException(
            "Implementation " + info_->name + " supports too many services",
            static_cast< cppu::OWeakObject * >(this));
    }
    css::uno::Sequence< rtl::OUString > names(
        static_cast< sal_Int32 >(info_->services.size()));
    sal_Int32 i = 0;
    for (std::vector< rtl::OUString >::const_iterator j(
             info_->services.begin());
         j != info_->services.end(); ++j)
    {
        names[i++] = *j;
    }
    return names;
}

void FactoryWrapper::loadImplementation(
    css::uno::Reference< css::uno::XComponentContext > const & context)
{
    {
        osl::MutexGuard g(mutex_);
        if (loaded_) {
            return;
        }
    }
    css::uno::Reference< css::lang::XSingleComponentFactory > f1;
    css::uno::Reference< css::lang::XSingleServiceFactory > f2;
    //TODO: There is a race here, as the relevant service factory can already
    // have been removed and loading can thus fail, as the entity from which to
    // load can disappear once the service factory is removed:
    manager_->loadImplementation(context, info_, &f1, &f2);
    if (!(f1.is() || f2.is())) {
        throw css::uno::DeploymentException(
            "Implementation " + info_->name + " does not provide a factory",
            static_cast< cppu::OWeakObject * >(this));
    }
    osl::MutexGuard g(mutex_);
    if (!loaded_) {
        loaded_ = true;
        factory1_ = f1;
        factory2_ = f2;
    }
}

css::uno::Reference< css::uno::XComponentContext > bootstrapComponentContext(
    css::uno::Reference< css::registry::XSimpleRegistry > const & typeRegistry,
    rtl::OUString const & serviceUris, rtl::Bootstrap const & bootstrap)
{
    rtl::Reference< ServiceManager > smgr(new ServiceManager(serviceUris));
    cppu::ContextEntry_Init entry;
    std::vector< cppu::ContextEntry_Init > context_values;
    context_values.push_back(
        cppu::ContextEntry_Init(
            "/singletons/com.sun.star.lang.theServiceManager",
            css::uno::makeAny(
                css::uno::Reference< css::uno::XInterface >(
                    static_cast< cppu::OWeakObject * >(smgr.get()))),
            false));
    context_values.push_back( //TODO: from services.rdb?
        cppu::ContextEntry_Init(
            "/singletons/com.sun.star.reflection.theTypeDescriptionManager",
            css::uno::makeAny(
                rtl::OUString("com.sun.star.comp.stoc.TypeDescriptionManager")),
            true /*TODO: false?*/));
    context_values.push_back( //TODO: from services.rdb?
        cppu::ContextEntry_Init(
            "/singletons/com.sun.star.util.theMacroExpander",
            css::uno::makeAny(
                cppuhelper::detail::create_bootstrap_macro_expander_factory()),
            true));
    Data const & data = smgr->getData();
    for (ImplementationMap::const_iterator i(data.singletons.begin());
         i != data.singletons.end(); ++i)
    {
        assert(!i->second.empty());
        assert(i->second[0].get() != 0);
        SAL_INFO_IF(
            i->second.size() > 1, "cppuhelper",
            "Arbitrarily chosing " << i->second[0]->info->name
                << " among multiple implementations for " << i->first);
        context_values.push_back(
            cppu::ContextEntry_Init(
                "/singletons/" + i->first,
                css::uno::makeAny(i->second[0]->info->name), true));
    }
    cppu::add_access_control_entries(&context_values, bootstrap);
    assert(!context_values.empty());
    css::uno::Reference< css::uno::XComponentContext > context(
        createComponentContext(
            &context_values[0], context_values.size(),
            css::uno::Reference< css::uno::XComponentContext >()));
    smgr->setContext(context);
    css::uno::Reference< css::container::XHierarchicalNameAccess > tdmgr(
        context->getValueByName(
            "/singletons/com.sun.star.reflection.theTypeDescriptionManager"),
        css::uno::UNO_QUERY_THROW);
    if (typeRegistry.is()) {
        css::uno::Sequence< css::uno::Any > arg(1);
        arg[0] <<= typeRegistry;
        css::uno::Reference< css::container::XSet >(
            tdmgr, css::uno::UNO_QUERY_THROW)->
            insert(
                css::uno::makeAny(
                    smgr->createInstanceWithArgumentsAndContext(
                        ("com.sun.star.comp.stoc"
                         ".RegistryTypeDescriptionProvider"),
                        arg, context)));
    }
    cppu::installTypeDescriptionManager(tdmgr);
    return context;
}

rtl::OUString getBootstrapVariable(
    rtl::Bootstrap const & bootstrap, rtl::OUString const & name)
{
    rtl::OUString v;
    if (!bootstrap.getFrom(name, v)) {
        throw css::uno::DeploymentException(
            "Cannot obtain " + name + " from uno ini",
            css::uno::Reference< css::uno::XInterface >());
    }
    return v;
}

css::uno::Reference< css::registry::XSimpleRegistry > readTypeRdbFile(
    rtl::OUString const & uri, bool optional,
    css::uno::Reference< css::registry::XSimpleRegistry > const & lastRegistry,
    css::uno::Reference< css::lang::XSingleServiceFactory > const &
        simpleRegistryFactory,
    css::uno::Reference< css::lang::XSingleServiceFactory > const &
        nestedRegistryFactory)
{
    assert(simpleRegistryFactory.is() && nestedRegistryFactory.is());
    try {
        css::uno::Reference< css::registry::XSimpleRegistry > simple(
            simpleRegistryFactory->createInstance(), css::uno::UNO_QUERY_THROW);
        simple->open(uri, true, false);
        if (lastRegistry.is()) {
            css::uno::Reference< css::registry::XSimpleRegistry > nested(
                nestedRegistryFactory->createInstance(),
                css::uno::UNO_QUERY_THROW);
            css::uno::Sequence< css::uno::Any > args(2);
            args[0] <<= lastRegistry;
            args[1] <<= simple;
            css::uno::Reference< css::lang::XInitialization >(
                nested, css::uno::UNO_QUERY_THROW)->
                initialize(args);
            return nested;
        } else {
            return simple;
        }
    } catch (css::registry::InvalidRegistryException & e) {
        if (!optional) {
            throw css::uno::DeploymentException(
                "Invalid registry " + uri + ":" + e.Message,
                css::uno::Reference< css::uno::XInterface >());
        }
        SAL_INFO("cppuhelper", "Ignored optional " << uri);
        return lastRegistry;
    }
}

css::uno::Reference< css::registry::XSimpleRegistry > readTypeRdbDirectory(
    rtl::OUString const & uri, bool optional,
    css::uno::Reference< css::registry::XSimpleRegistry > const & lastRegistry,
    css::uno::Reference< css::lang::XSingleServiceFactory > const &
        simpleRegistryFactory,
    css::uno::Reference< css::lang::XSingleServiceFactory > const &
        nestedRegistryFactory)
{
    assert(simpleRegistryFactory.is() && nestedRegistryFactory.is());
    osl::Directory dir(uri);
    switch (dir.open()) {
    case osl::FileBase::E_None:
        break;
    case osl::FileBase::E_NOENT:
        if (optional) {
            SAL_INFO("cppuhelper", "Ignored optional " << uri);
            return lastRegistry;
        }
        // fall through
    default:
        throw css::uno::DeploymentException(
            "Cannot open directory " + uri,
            css::uno::Reference< css::uno::XInterface >());
    }
    css::uno::Reference< css::registry::XSimpleRegistry > last(lastRegistry);
    for (;;) {
        rtl::OUString fileUri;
        if (!nextDirectoryItem(dir, &fileUri)) {
            break;
        }
        last = readTypeRdbFile(
            fileUri, optional, last, simpleRegistryFactory,
            nestedRegistryFactory);
    }
    return last;
}

css::uno::Reference< css::registry::XSimpleRegistry > createTypeRegistry(
    rtl::OUString const & uris, rtl::OUString const & libraryDirectoryUri)
{
    css::uno::Reference< css::lang::XMultiComponentFactory > factory(
        cppu::bootstrapInitialSF(libraryDirectoryUri));
    css::uno::Reference< css::lang::XSingleServiceFactory > simpleRegs(
        cppu::loadSharedLibComponentFactory(
            "bootstrap.uno" SAL_DLLEXTENSION, libraryDirectoryUri,
            "com.sun.star.comp.stoc.SimpleRegistry",
            css::uno::Reference< css::lang::XMultiServiceFactory >(
                factory, css::uno::UNO_QUERY_THROW),
            css::uno::Reference< css::registry::XRegistryKey >()),
        css::uno::UNO_QUERY_THROW);
    css::uno::Reference< css::lang::XSingleServiceFactory > nestedRegs(
        cppu::loadSharedLibComponentFactory(
            "bootstrap.uno" SAL_DLLEXTENSION, libraryDirectoryUri,
            "com.sun.star.comp.stoc.NestedRegistry",
            css::uno::Reference< css::lang::XMultiServiceFactory >(
                factory, css::uno::UNO_QUERY_THROW),
            css::uno::Reference< css::registry::XRegistryKey >()),
        css::uno::UNO_QUERY_THROW);
    css::uno::Reference< css::registry::XSimpleRegistry > reg;
    for (sal_Int32 i = 0; i != -1;) {
        rtl::OUString uri(uris.getToken(0, ' ', i));
        if (uri.isEmpty()) {
            continue;
        }
        bool optional;
        bool directory;
        decodeRdbUri(&uri, &optional, &directory);
        reg = directory
            ? readTypeRdbDirectory(uri, optional, reg, simpleRegs, nestedRegs)
            : readTypeRdbFile(uri, optional, reg, simpleRegs, nestedRegs);
    }
    return reg;
}

}

css::uno::Reference< css::uno::XComponentContext >
cppu::defaultBootstrap_InitialComponentContext(rtl::OUString const & iniUri)
    SAL_THROW((css::uno::Exception))
{
    rtl::Bootstrap bs(iniUri);
    if (bs.getHandle() == 0) {
        throw css::uno::DeploymentException(
            "Cannot open uno ini " + iniUri,
            css::uno::Reference< css::uno::XInterface >());
    }
    return bootstrapComponentContext(
        createTypeRegistry(
            getBootstrapVariable(bs, "UNO_TYPES"),
            getBootstrapVariable(bs, "URE_INTERNAL_LIB_DIR")),
        getBootstrapVariable(bs, "UNO_SERVICES"), bs);
}

css::uno::Reference< css::uno::XComponentContext >
cppu::defaultBootstrap_InitialComponentContext()
    SAL_THROW((css::uno::Exception))
{
    return defaultBootstrap_InitialComponentContext(getUnoIniUri());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
