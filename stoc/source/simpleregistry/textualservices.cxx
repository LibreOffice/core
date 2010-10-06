/*************************************************************************
*
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2000, 2010 Oracle and/or its affiliates.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* This file is part of OpenOffice.org.
*
* OpenOffice.org is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License version 3
* only, as published by the Free Software Foundation.
*
* OpenOffice.org is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License version 3 for more details
* (a copy is included in the LICENSE file that accompanied this code).
*
* You should have received a copy of the GNU Lesser General Public License
* version 3 along with OpenOffice.org.  If not, see
* <http://www.openoffice.org/license.html>
* for a copy of the LGPLv3 License.
*
************************************************************************/

#include "precompiled_stoc.hxx"
#include "sal/config.h"

#include <cstddef>
#include <cstdlib>
#include <map>
#include <vector>

#include "boost/noncopyable.hpp"
#include "com/sun/star/container/NoSuchElementException.hpp"
#include "com/sun/star/registry/InvalidRegistryException.hpp"
#include "com/sun/star/registry/XRegistryKey.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/XInterface.hpp"
#include "cppuhelper/implbase1.hxx"
#include "osl/diagnose.h"
#include "rtl/malformeduriexception.hxx"
#include "rtl/ref.hxx"
#include "rtl/string.h"
#include "rtl/uri.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "salhelper/simplereferenceobject.hxx"
#include "xmlreader/span.hxx"
#include "xmlreader/xmlreader.hxx"

#include "textualservices.hxx"

namespace stoc { namespace simpleregistry {

namespace {

namespace css = com::sun::star;

struct Implementation {
    rtl::OUString uri;
    rtl::OUString loader;
    std::vector< rtl::OUString > services;
    std::vector< rtl::OUString > singletons;
};

typedef std::map< rtl::OUString, Implementation > Implementations;

typedef std::map< rtl::OUString, std::vector< rtl::OUString > >
    ImplementationMap;

}

class Data: public salhelper::SimpleReferenceObject, private boost::noncopyable
{
public:
    Implementations implementations;
    ImplementationMap services;
    ImplementationMap singletons;
};

namespace {

class Parser: private boost::noncopyable {
public:
    Parser(rtl::OUString const & uri, rtl::Reference< Data > const & data);

private:
    void handleComponent();

    void handleImplementation();

    void handleService();

    void handleSingleton();

    rtl::OUString getNameAttribute();

    xmlreader::XmlReader reader_;
    rtl::Reference< Data > data_;
    rtl::OUString attrUri_;
    rtl::OUString attrLoader_;
    rtl::OUString attrImplementation_;
};

Parser::Parser(rtl::OUString const & uri, rtl::Reference< Data > const & data):
    reader_(uri), data_(data)
{
    OSL_ASSERT(data.is());
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
            if (res == xmlreader::XmlReader::RESULT_BEGIN && nsId == ucNsId &&
                name.equals(RTL_CONSTASCII_STRINGPARAM("components")))
            {
                state = STATE_COMPONENTS;
                break;
            }
            throw css::registry::InvalidRegistryException(
                (reader_.getUrl() +
                 rtl::OUString(
                     RTL_CONSTASCII_USTRINGPARAM(
                         ": unexpected item in outer level"))),
                css::uno::Reference< css::uno::XInterface >());
        case STATE_END:
            if (res == xmlreader::XmlReader::RESULT_DONE) {
                return;
            }
            throw css::registry::InvalidRegistryException(
                (reader_.getUrl() +
                 rtl::OUString(
                     RTL_CONSTASCII_USTRINGPARAM(
                         ": unexpected item in outer level"))),
                css::uno::Reference< css::uno::XInterface >());
        case STATE_COMPONENTS:
            if (res == xmlreader::XmlReader::RESULT_END) {
                state = STATE_END;
                break;
            }
            if (res == xmlreader::XmlReader::RESULT_BEGIN && nsId == ucNsId &&
                name.equals(RTL_CONSTASCII_STRINGPARAM("component")))
            {
                handleComponent();
                state = STATE_COMPONENT_INITIAL;
                break;
            }
            throw css::registry::InvalidRegistryException(
                (reader_.getUrl() +
                 rtl::OUString(
                     RTL_CONSTASCII_USTRINGPARAM(
                         ": unexpected item in <components>"))),
                css::uno::Reference< css::uno::XInterface >());
        case STATE_COMPONENT:
            if (res == xmlreader::XmlReader::RESULT_END) {
                state = STATE_COMPONENTS;
                break;
            }
            // fall through
        case STATE_COMPONENT_INITIAL:
            if (res == xmlreader::XmlReader::RESULT_BEGIN && nsId == ucNsId &&
                name.equals(RTL_CONSTASCII_STRINGPARAM("implementation")))
            {
                handleImplementation();
                state = STATE_IMPLEMENTATION;
                break;
            }
            throw css::registry::InvalidRegistryException(
                (reader_.getUrl() +
                 rtl::OUString(
                     RTL_CONSTASCII_USTRINGPARAM(
                         ": unexpected item in <component>"))),
                css::uno::Reference< css::uno::XInterface >());
        case STATE_IMPLEMENTATION:
            if (res == xmlreader::XmlReader::RESULT_END) {
                state = STATE_COMPONENT;
                break;
            }
            if (res == xmlreader::XmlReader::RESULT_BEGIN && nsId == ucNsId &&
                name.equals(RTL_CONSTASCII_STRINGPARAM("service")))
            {
                handleService();
                state = STATE_SERVICE;
                break;
            }
            if (res == xmlreader::XmlReader::RESULT_BEGIN && nsId == ucNsId &&
                name.equals(RTL_CONSTASCII_STRINGPARAM("singleton")))
            {
                handleSingleton();
                state = STATE_SINGLETON;
                break;
            }
            throw css::registry::InvalidRegistryException(
                (reader_.getUrl() +
                 rtl::OUString(
                     RTL_CONSTASCII_USTRINGPARAM(
                         ": unexpected item in <implementation>"))),
                css::uno::Reference< css::uno::XInterface >());
        case STATE_SERVICE:
            if (res == xmlreader::XmlReader::RESULT_END) {
                state = STATE_IMPLEMENTATION;
                break;
            }
            throw css::registry::InvalidRegistryException(
                (reader_.getUrl() +
                 rtl::OUString(
                     RTL_CONSTASCII_USTRINGPARAM(
                         ": unexpected item in <service>"))),
                css::uno::Reference< css::uno::XInterface >());
        case STATE_SINGLETON:
            if (res == xmlreader::XmlReader::RESULT_END) {
                state = STATE_IMPLEMENTATION;
                break;
            }
            throw css::registry::InvalidRegistryException(
                (reader_.getUrl() +
                 rtl::OUString(
                     RTL_CONSTASCII_USTRINGPARAM(
                         ": unexpected item in <service>"))),
                css::uno::Reference< css::uno::XInterface >());
        }
    }
}

void Parser::handleComponent() {
    attrUri_ = rtl::OUString();
    attrLoader_ = rtl::OUString();
    xmlreader::Span name;
    int nsId;
    while (reader_.nextAttribute(&nsId, &name)) {
        if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
            name.equals(RTL_CONSTASCII_STRINGPARAM("uri")))
        {
            if (attrUri_.getLength() != 0) {
                throw css::registry::InvalidRegistryException(
                    (reader_.getUrl() +
                     rtl::OUString(
                         RTL_CONSTASCII_USTRINGPARAM(
                             ": <component> has multiple \"uri\" attributes"))),
                    css::uno::Reference< css::uno::XInterface >());
            }
            attrUri_ = reader_.getAttributeValue(false).convertFromUtf8();
            if (attrUri_.getLength() == 0) {
                throw css::registry::InvalidRegistryException(
                    (reader_.getUrl() +
                     rtl::OUString(
                         RTL_CONSTASCII_USTRINGPARAM(
                             ": <component> has empty \"uri\" attribute"))),
                    css::uno::Reference< css::uno::XInterface >());
            }
        } else if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                   name.equals(RTL_CONSTASCII_STRINGPARAM("loader")))
        {
            if (attrLoader_.getLength() != 0) {
                throw css::registry::InvalidRegistryException(
                    (reader_.getUrl() +
                     rtl::OUString(
                         RTL_CONSTASCII_USTRINGPARAM(
                             ": <component> has multiple \"loader\""
                             " attributes"))),
                    css::uno::Reference< css::uno::XInterface >());
            }
            attrLoader_ = reader_.getAttributeValue(false).convertFromUtf8();
            if (attrLoader_.getLength() == 0) {
                throw css::registry::InvalidRegistryException(
                    (reader_.getUrl() +
                     rtl::OUString(
                         RTL_CONSTASCII_USTRINGPARAM(
                             ": <component> has empty \"loader\" attribute"))),
                    css::uno::Reference< css::uno::XInterface >());
            }
        } else {
            throw css::registry::InvalidRegistryException(
                (reader_.getUrl() +
                 rtl::OUString(
                     RTL_CONSTASCII_USTRINGPARAM(
                         ": expected <component> attribute \"uri\" or"
                         " \"loader\""))),
                css::uno::Reference< css::uno::XInterface >());
        }
    }
    if (attrUri_.getLength() == 0) {
        throw css::registry::InvalidRegistryException(
            (reader_.getUrl() +
             rtl::OUString(
                 RTL_CONSTASCII_USTRINGPARAM(
                     ": <component> is missing \"uri\" attribute"))),
            css::uno::Reference< css::uno::XInterface >());
    }
    if (attrLoader_.getLength() == 0) {
        throw css::registry::InvalidRegistryException(
            (reader_.getUrl() +
             rtl::OUString(
                 RTL_CONSTASCII_USTRINGPARAM(
                     ": <component> is missing \"loader\" attribute"))),
            css::uno::Reference< css::uno::XInterface >());
    }
    try {
        attrUri_ = rtl::Uri::convertRelToAbs(reader_.getUrl(), attrUri_);
    } catch (rtl::MalformedUriException & e) {
        throw css::registry::InvalidRegistryException(
            (reader_.getUrl() +
             rtl::OUString(
                 RTL_CONSTASCII_USTRINGPARAM(": bad \"uri\" attribute: ")) +
             e.getMessage()),
            css::uno::Reference< css::uno::XInterface >());
    }
}

void Parser::handleImplementation() {
    attrImplementation_ = getNameAttribute();
    if (data_->implementations.find(attrImplementation_) !=
        data_->implementations.end())
    {
        throw css::registry::InvalidRegistryException(
            (reader_.getUrl() +
             rtl::OUString(
                 RTL_CONSTASCII_USTRINGPARAM(
                     ": duplicate <implementation name=\"")) +
             attrImplementation_ +
             rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\">"))),
            css::uno::Reference< css::uno::XInterface >());
    }
    data_->implementations[attrImplementation_].uri = attrUri_;
    data_->implementations[attrImplementation_].loader = attrLoader_;
}

void Parser::handleService() {
    rtl::OUString name = getNameAttribute();
    data_->implementations[attrImplementation_].services.push_back(name);
    data_->services[name].push_back(attrImplementation_);
}

void Parser::handleSingleton() {
    rtl::OUString name = getNameAttribute();
    data_->implementations[attrImplementation_].singletons.push_back(name);
    data_->singletons[name].push_back(attrImplementation_);
}

rtl::OUString Parser::getNameAttribute() {
    rtl::OUString attrName;
    xmlreader::Span name;
    int nsId;
    while (reader_.nextAttribute(&nsId, &name)) {
        if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
            name.equals(RTL_CONSTASCII_STRINGPARAM("name")))
        {
            if (attrName.getLength() != 0) {
                throw css::registry::InvalidRegistryException(
                    (reader_.getUrl() +
                     rtl::OUString(
                         RTL_CONSTASCII_USTRINGPARAM(
                             ": element has multiple \"name\" attributes"))),
                    css::uno::Reference< css::uno::XInterface >());
            }
            attrName = reader_.getAttributeValue(false).convertFromUtf8();
            if (attrName.getLength() == 0) {
                throw css::registry::InvalidRegistryException(
                    (reader_.getUrl() +
                     rtl::OUString(
                         RTL_CONSTASCII_USTRINGPARAM(
                             ": element has empty \"name\" attribute"))),
                    css::uno::Reference< css::uno::XInterface >());
            }
        } else {
            throw css::registry::InvalidRegistryException(
                (reader_.getUrl() +
                 rtl::OUString(
                     RTL_CONSTASCII_USTRINGPARAM(
                         ": expected element attribute \"name\""))),
                css::uno::Reference< css::uno::XInterface >());
        }
    }
    if (attrName.getLength() == 0) {
        throw css::registry::InvalidRegistryException(
            (reader_.getUrl() +
             rtl::OUString(
                 RTL_CONSTASCII_USTRINGPARAM(
                     ": element is missing \"name\" attribute"))),
            css::uno::Reference< css::uno::XInterface >());
    }
    return attrName;
}

rtl::OUString pathToString(std::vector< rtl::OUString > const & path) {
    rtl::OUStringBuffer buf;
    for (std::vector< rtl::OUString >::const_iterator i(path.begin());
         i != path.end(); ++i)
    {
        buf.append(sal_Unicode('/'));
        buf.append(*i);
    }
    if (buf.getLength() == 0) {
        buf.append(sal_Unicode('/'));
    }
    return buf.makeStringAndClear();
}

class Key: public cppu::WeakImplHelper1< css::registry::XRegistryKey > {
public:
    Key(
        rtl::Reference< Data > const & data,
        std::vector< rtl::OUString > const & path):
        data_(data), path_(path) { OSL_ASSERT(data.is());
 }

private:
    /*
      /
        IMPLEMENTATIONS
          <implementation>
            UNO
              LOCATION utf-8
              ACTIVATOR utf-8
              SERVICES
                <service>
                ...
              SINGLETONS
                <singleton> utf-16
                ...
          ...
        SERVICES
          <service> utf-8-list
          ...
        SINGLETONS
          <singleton> utf-16
            REGISTERED_BY utf-8-list
          ...
    */
    enum State {
        STATE_ROOT, STATE_IMPLEMENTATIONS, STATE_IMPLEMENTATION, STATE_UNO,
        STATE_LOCATION, STATE_ACTIVATOR, STATE_IMPLEMENTATION_SERVICES,
        STATE_IMPLEMENTATION_SERVICE, STATE_IMPLEMENTATION_SINGLETONS,
        STATE_IMPLEMENTATION_SINGLETON, STATE_SERVICES, STATE_SERVICE,
        STATE_SINGLETONS, STATE_SINGLETON, STATE_REGISTEREDBY };

    virtual rtl::OUString SAL_CALL getKeyName()
        throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isReadOnly() throw (
        css::registry::InvalidRegistryException, css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isValid() throw(css::uno::RuntimeException);

    virtual css::registry::RegistryKeyType SAL_CALL getKeyType(
        rtl::OUString const & rKeyName)
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException);

    virtual css::registry::RegistryValueType SAL_CALL getValueType() throw(
        css::registry::InvalidRegistryException, css::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getLongValue() throw (
        css::registry::InvalidRegistryException,
        css::registry::InvalidValueException, css::uno::RuntimeException);

    virtual void SAL_CALL setLongValue(sal_Int32 value) throw (
        css::registry::InvalidRegistryException, css::uno::RuntimeException);

    virtual css::uno::Sequence< sal_Int32 > SAL_CALL getLongListValue() throw(
        css::registry::InvalidRegistryException,
        css::registry::InvalidValueException, css::uno::RuntimeException);

    virtual void SAL_CALL setLongListValue(
        com::sun::star::uno::Sequence< sal_Int32 > const & seqValue)
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException);

    virtual rtl::OUString SAL_CALL getAsciiValue() throw (
        css::registry::InvalidRegistryException,
        css::registry::InvalidValueException, css::uno::RuntimeException);

    virtual void SAL_CALL setAsciiValue(rtl::OUString const & value) throw (
        css::registry::InvalidRegistryException, css::uno::RuntimeException);

    virtual css::uno::Sequence< rtl::OUString > SAL_CALL getAsciiListValue()
        throw (
            css::registry::InvalidRegistryException,
            css::registry::InvalidValueException, css::uno::RuntimeException);

    virtual void SAL_CALL setAsciiListValue(
        css::uno::Sequence< rtl::OUString > const & seqValue)
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException);

    virtual rtl::OUString SAL_CALL getStringValue() throw(
        css::registry::InvalidRegistryException,
        css::registry::InvalidValueException, css::uno::RuntimeException);

    virtual void SAL_CALL setStringValue(rtl::OUString const & value) throw (
        css::registry::InvalidRegistryException, css::uno::RuntimeException);

    virtual css::uno::Sequence< rtl::OUString > SAL_CALL getStringListValue()
        throw (
            css::registry::InvalidRegistryException,
            css::registry::InvalidValueException, css::uno::RuntimeException);

    virtual void SAL_CALL setStringListValue(
        css::uno::Sequence< rtl::OUString > const & seqValue)
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException);

    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getBinaryValue() throw (
        css::registry::InvalidRegistryException,
        css::registry::InvalidValueException, css::uno::RuntimeException);

    virtual void SAL_CALL setBinaryValue(
        css::uno::Sequence< sal_Int8 > const & value)
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException);

    virtual css::uno::Reference< css::registry::XRegistryKey > SAL_CALL openKey(
        rtl::OUString const & aKeyName)
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException);

    virtual css::uno::Reference< css::registry::XRegistryKey > SAL_CALL
    createKey(rtl::OUString const & aKeyName) throw (
        css::registry::InvalidRegistryException, css::uno::RuntimeException);

    virtual void SAL_CALL closeKey() throw (
        css::registry::InvalidRegistryException, css::uno::RuntimeException);

    virtual void SAL_CALL deleteKey(rtl::OUString const & rKeyName) throw (
        css::registry::InvalidRegistryException, css::uno::RuntimeException);

    virtual
    css::uno::Sequence< css::uno::Reference< css::registry::XRegistryKey > >
    SAL_CALL openKeys() throw (
        css::registry::InvalidRegistryException, css::uno::RuntimeException);

    virtual css::uno::Sequence< rtl::OUString > SAL_CALL getKeyNames() throw (
        css::registry::InvalidRegistryException, css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL createLink(
        rtl::OUString const & aLinkName, rtl::OUString const & aLinkTarget)
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException);

    virtual void SAL_CALL deleteLink(rtl::OUString const & rLinkName) throw (
        css::registry::InvalidRegistryException, css::uno::RuntimeException);

    virtual rtl::OUString SAL_CALL getLinkTarget(
        rtl::OUString const & rLinkName)
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException);

    virtual rtl::OUString SAL_CALL getResolvedName(
        rtl::OUString const & aKeyName)
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException);

    bool find(
        rtl::OUString const & relative, std::vector< rtl::OUString > * path,
        State * state, css::registry::RegistryValueType * type) const;

    css::uno::Sequence< rtl::OUString > getChildren();

    rtl::Reference< Data > data_;
    std::vector< rtl::OUString > path_;
};

rtl::OUString Key::getKeyName() throw (css::uno::RuntimeException) {
    return pathToString(path_);
}

sal_Bool Key::isReadOnly()
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    return true;
}

sal_Bool Key::isValid() throw(css::uno::RuntimeException) {
    return true;
}

css::registry::RegistryKeyType Key::getKeyType(rtl::OUString const & rKeyName)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    if (!find(rtl::OUString(), 0, 0, 0)) {
        throw css::registry::InvalidRegistryException(
            (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("unknown key ")) +
             rKeyName),
            static_cast< cppu::OWeakObject * >(this));
    }
    return css::registry::RegistryKeyType_KEY;
}

css::registry::RegistryValueType Key::getValueType()
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    css::registry::RegistryValueType type =
        css::registry::RegistryValueType_NOT_DEFINED;
    OSL_VERIFY(find(rtl::OUString(), 0, 0, &type));
    return type;
}

sal_Int32 Key::getLongValue() throw (
    css::registry::InvalidRegistryException,
    css::registry::InvalidValueException, css::uno::RuntimeException)
{
    throw css::registry::InvalidValueException(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.registry.SimpleRegistry textual services key"
                " getLongValue not supported")),
        static_cast< OWeakObject * >(this));
}

void Key::setLongValue(sal_Int32)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    throw css::registry::InvalidRegistryException(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.registry.SimpleRegistry textual services key"
                " setLongValue not supported")),
        static_cast< OWeakObject * >(this));
}

css::uno::Sequence< sal_Int32 > Key::getLongListValue() throw (
    css::registry::InvalidRegistryException,
    css::registry::InvalidValueException, css::uno::RuntimeException)
{
    throw css::registry::InvalidValueException(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.registry.SimpleRegistry textual services key"
                " getLongListValue not supported")),
        static_cast< OWeakObject * >(this));
}

void Key::setLongListValue(css::uno::Sequence< sal_Int32 > const &)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    throw css::registry::InvalidRegistryException(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.registry.SimpleRegistry textual services key"
                " setLongListValue not supported")),
        static_cast< OWeakObject * >(this));
}

rtl::OUString Key::getAsciiValue() throw (
    css::registry::InvalidRegistryException,
    css::registry::InvalidValueException, css::uno::RuntimeException)
{
    State state = STATE_ROOT;
    OSL_VERIFY(find(rtl::OUString(), 0, &state, 0));
    switch (state) {
    case STATE_LOCATION:
        return data_->implementations[path_[1]].uri;
    case STATE_ACTIVATOR:
        return data_->implementations[path_[1]].loader;
    default:
        throw css::registry::InvalidValueException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.registry.SimpleRegistry textual services key"
                    " getAsciiValue: wrong type")),
            static_cast< OWeakObject * >(this));
    }
}

void Key::setAsciiValue(rtl::OUString const &)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    throw css::registry::InvalidRegistryException(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.registry.SimpleRegistry textual services key"
                " setAsciiValue not supported")),
        static_cast< OWeakObject * >(this));
}

css::uno::Sequence< rtl::OUString > Key::getAsciiListValue() throw (
    css::registry::InvalidRegistryException,
    css::registry::InvalidValueException, css::uno::RuntimeException)
{
    State state = STATE_ROOT;
    OSL_VERIFY(find(rtl::OUString(), 0, &state, 0));
    std::vector< rtl::OUString > const * list;
    switch (state) {
    case STATE_SERVICE:
        list = &data_->services[path_[1]];
        break;
    case STATE_REGISTEREDBY:
        list = &data_->singletons[path_[1]];
        break;
    default:
        throw css::registry::InvalidValueException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.registry.SimpleRegistry textual services key"
                    " getAsciiListValue: wrong type")),
            static_cast< OWeakObject * >(this));
    }
    if (list->size() > SAL_MAX_INT32) {
        throw css::registry::InvalidValueException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.registry.SimpleRegistry textual services key"
                    " getAsciiListValue: too large")),
            static_cast< OWeakObject * >(this));
    }
    css::uno::Sequence< rtl::OUString > seq(
        static_cast< sal_Int32 >(list->size()));
    sal_Int32 i = 0;
    for (std::vector< rtl::OUString >::const_iterator j(list->begin());
         j != list->end(); ++j)
    {
        seq[i++] = *j;
    }
    return seq;
}

void Key::setAsciiListValue(css::uno::Sequence< rtl::OUString > const &)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    throw css::registry::InvalidRegistryException(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.registry.SimpleRegistry textual services key"
                " setAsciiListValue not supported")),
        static_cast< OWeakObject * >(this));
}

rtl::OUString Key::getStringValue() throw (
    css::registry::InvalidRegistryException,
    css::registry::InvalidValueException, css::uno::RuntimeException)
{
    State state = STATE_ROOT;
    OSL_VERIFY(find(rtl::OUString(), 0, &state, 0));
    switch (state) {
    case STATE_IMPLEMENTATION_SINGLETON:
    case STATE_SINGLETON:
        throw css::registry::InvalidRegistryException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.registry.SimpleRegistry textual services key"
                    " getStringValue: does not associate singletons with"
                    " services")),
            static_cast< OWeakObject * >(this));
    default:
        break;
    }
    // default case extracted from switch to avoid erroneous compiler warnings
    // on Solaris:
    throw css::registry::InvalidValueException(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.registry.SimpleRegistry textual services key"
                " getStringValue: wrong type")),
        static_cast< OWeakObject * >(this));
}

void Key::setStringValue(rtl::OUString const &)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    throw css::registry::InvalidRegistryException(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.registry.SimpleRegistry textual services key"
                " setStringValue not supported")),
        static_cast< OWeakObject * >(this));
}

css::uno::Sequence< rtl::OUString > Key::getStringListValue() throw (
    css::registry::InvalidRegistryException,
    css::registry::InvalidValueException, css::uno::RuntimeException)
{
    throw css::registry::InvalidValueException(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.registry.SimpleRegistry textual services key"
                " getStringListValue not supported")),
        static_cast< OWeakObject * >(this));
}

void Key::setStringListValue(css::uno::Sequence< rtl::OUString > const &)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    throw css::registry::InvalidRegistryException(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.registry.SimpleRegistry textual services key"
                " setStringListValue not supported")),
        static_cast< OWeakObject * >(this));
}

css::uno::Sequence< sal_Int8 > Key::getBinaryValue()
    throw (
        css::registry::InvalidRegistryException,
        css::registry::InvalidValueException, css::uno::RuntimeException)
{
    throw css::registry::InvalidValueException(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.registry.SimpleRegistry textual services key"
                " getBinarValue not supported")),
        static_cast< OWeakObject * >(this));
}

void Key::setBinaryValue(css::uno::Sequence< sal_Int8 > const &)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    throw css::registry::InvalidRegistryException(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.registry.SimpleRegistry textual services key"
                " setBinaryValue not supported")),
        static_cast< OWeakObject * >(this));
}

css::uno::Reference< css::registry::XRegistryKey > Key::openKey(
    rtl::OUString const & aKeyName)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    std::vector< rtl::OUString > path;
    if (!find(aKeyName, &path, 0, 0)) {
        return css::uno::Reference< css::registry::XRegistryKey >();
    }
    return new Key(data_, path);
}

css::uno::Reference< css::registry::XRegistryKey > Key::createKey(
    rtl::OUString const &)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    throw css::registry::InvalidRegistryException(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.registry.SimpleRegistry textual services key"
                " createKey not supported")),
        static_cast< OWeakObject * >(this));
}

void Key::closeKey()
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{}

void Key::deleteKey(rtl::OUString const &)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    throw css::registry::InvalidRegistryException(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.registry.SimpleRegistry textual services key"
                " deleteKey not supported")),
        static_cast< OWeakObject * >(this));
}

css::uno::Sequence< css::uno::Reference< css::registry::XRegistryKey > >
Key::openKeys()
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    css::uno::Sequence< rtl::OUString > names(getChildren());
    css::uno::Sequence< css::uno::Reference< css::registry::XRegistryKey > >
        keys(names.getLength());
    for (sal_Int32 i = 0; i < keys.getLength(); ++i) {
        keys[i] = openKey(names[i]);
        OSL_ASSERT(keys[i].is());
    }
    return keys;
}

css::uno::Sequence< rtl::OUString > Key::getKeyNames()
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    css::uno::Sequence< rtl::OUString > names(getChildren());
    rtl::OUString prefix(pathToString(path_));
    prefix += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
    for (sal_Int32 i = 0; i < names.getLength(); ++i) {
        names[i] = prefix + names[i];
    }
    return names;
}

sal_Bool Key::createLink(rtl::OUString const &, rtl::OUString const &)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    throw css::registry::InvalidRegistryException(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.registry.SimpleRegistry textual services key"
                " createLink not supported")),
        static_cast< OWeakObject * >(this));
}

void Key::deleteLink(rtl::OUString const &)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    throw css::registry::InvalidRegistryException(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.registry.SimpleRegistry textual services key"
                " deleteLink not supported")),
        static_cast< OWeakObject * >(this));
}

rtl::OUString Key::getLinkTarget(rtl::OUString const &)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    throw css::registry::InvalidRegistryException(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.registry.SimpleRegistry textual services key"
                " getLinkTarget not supported")),
        static_cast< OWeakObject * >(this));
}

rtl::OUString Key::getResolvedName(rtl::OUString const & aKeyName)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    std::vector< rtl::OUString > path;
    find(aKeyName, &path, 0, 0);
    return pathToString(path);
}

bool Key::find(
    rtl::OUString const & relative, std::vector< rtl::OUString > * path,
    State * state, css::registry::RegistryValueType * type) const
{
    std::vector< rtl::OUString > p(path_);
    sal_Int32 i = 0;
    do {
        rtl::OUString seg(relative.getToken(0, '/', i));
        if (seg.getLength() != 0) {
            p.push_back(seg);
        }
    } while (i >= 0);
    if (path != 0) {
        *path = p;
    }
    std::size_t const MAX_TRANSITIONS = 4;
    struct StateInfo {
        css::registry::RegistryValueType type;
        std::size_t count;
        struct { char const * segment; State state; }
            transitions[MAX_TRANSITIONS];
    };
    static StateInfo const info[] = {
        // STATE_ROOT:
        { css::registry::RegistryValueType_NOT_DEFINED, 3,
          { { "IMPLEMENTATIONS", STATE_IMPLEMENTATIONS },
            { "SERVICES", STATE_SERVICES },
            { "SINGLETONS", STATE_SINGLETONS } } },
        // STATE_IMPLEMENTATIONS:
        { css::registry::RegistryValueType_NOT_DEFINED, 1,
          { { 0, STATE_IMPLEMENTATION } } },
        // STATE_IMPLEMENTATION:
        { css::registry::RegistryValueType_NOT_DEFINED, 1,
          { { "UNO", STATE_UNO } } },
        // STATE_UNO:
        { css::registry::RegistryValueType_NOT_DEFINED, 4,
          { { "LOCATION", STATE_LOCATION },
            { "ACTIVATOR", STATE_ACTIVATOR },
            { "SERVICES", STATE_IMPLEMENTATION_SERVICES },
            { "SINGLETONS", STATE_IMPLEMENTATION_SINGLETONS } } },
        // STATE_LOCATION:
        { css::registry::RegistryValueType_ASCII, 0, {} },
        // STATE_ACTIVATOR:
        { css::registry::RegistryValueType_ASCII, 0, {} },
        // STATE_IMPLEMENTATION_SERVICES:
        { css::registry::RegistryValueType_NOT_DEFINED, 1,
          { { 0, STATE_IMPLEMENTATION_SERVICE } } },
        // STATE_IMPLEMENTATION_SERVICE:
        { css::registry::RegistryValueType_NOT_DEFINED, 0, {} },
        // STATE_IMPLEMENTATION_SINGLETONS:
        { css::registry::RegistryValueType_NOT_DEFINED, 1,
          { { 0, STATE_IMPLEMENTATION_SINGLETON } } },
        // STATE_IMPLEMENTATION_SINGLETON:
        { css::registry::RegistryValueType_STRING, 0, {} },
        // STATE_SERVICES:
        { css::registry::RegistryValueType_NOT_DEFINED, 1,
          { { 0, STATE_SERVICE } } },
        // STATE_SERVICE:
        { css::registry::RegistryValueType_ASCIILIST, 0, {} },
        // STATE_SINGLETONS:
        { css::registry::RegistryValueType_NOT_DEFINED, 1,
          { { 0, STATE_SINGLETON } } },
        // STATE_SINGLETON:
        { css::registry::RegistryValueType_STRING, 1,
          { { "REGISTERED_BY", STATE_REGISTEREDBY } } },
        // STATE_REGISTEREDBY:
        { css::registry::RegistryValueType_ASCIILIST, 0, {} } };
    State s = STATE_ROOT;
    for (std::vector< rtl::OUString >::iterator j(p.begin()); j != p.end(); ++j)
    {
        bool found = false;
        for (std::size_t k = 0; k < info[s].count; ++k) {
            if (info[s].transitions[k].segment == 0) {
                switch (info[s].transitions[k].state) {
                case STATE_IMPLEMENTATION:
                    found = data_->implementations.find(*j) !=
                        data_->implementations.end();
                    break;
                case STATE_IMPLEMENTATION_SERVICE:
                case STATE_IMPLEMENTATION_SINGLETON:
                    found = true; //TODO
                    break;
                case STATE_SERVICE:
                    found = data_->services.find(*j) != data_->services.end();
                    break;
                case STATE_SINGLETON:
                    found = data_->singletons.find(*j) !=
                        data_->singletons.end();
                    break;
                default:
                    std::abort(); // this cannot happen
                }
            } else {
                found = j->equalsAscii(info[s].transitions[k].segment);
            }
            if (found) {
                s = info[s].transitions[k].state;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }
    if (state != 0) {
        *state = s;
    }
    if (type != 0) {
        *type = info[s].type;
    }
    return true;
}

css::uno::Sequence< rtl::OUString > Key::getChildren() {
    State state = STATE_ROOT;
    OSL_VERIFY(find(rtl::OUString(), 0, &state, 0));
    switch (state) {
    default:
        std::abort(); // this cannot happen
        // pseudo-fall-through to avoid warnings on MSC
    case STATE_ROOT:
        {
            css::uno::Sequence< rtl::OUString > seq(3);
            seq[0] = rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("IMPLEMENTATIONS"));
            seq[1] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SERVICES"));
            seq[2] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SINGLETONS"));
            return seq;
        }
    case STATE_IMPLEMENTATIONS:
        {
            if (data_->implementations.size() > SAL_MAX_INT32) {
                throw css::registry::InvalidValueException(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "com.sun.star.registry.SimpleRegistry textual"
                            " services key openKeys: too large")),
                    static_cast< OWeakObject * >(this));
            }
            css::uno::Sequence< rtl::OUString > seq(
                    static_cast< sal_Int32 >(data_->implementations.size()));
            sal_Int32 i = 0;
            for (Implementations::iterator j(data_->implementations.begin());
                 j != data_->implementations.end(); ++j)
            {
                seq[i++] = j->first;
            }
            return seq;
        }
    case STATE_UNO:
        {
            css::uno::Sequence< rtl::OUString > seq(4);
            seq[0] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LOCATION"));
            seq[1] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ACTIVATOR"));
            seq[2] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SERVICES"));
            seq[3] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SINGLETONS"));
            return seq;
        }
    case STATE_LOCATION:
    case STATE_ACTIVATOR:
    case STATE_IMPLEMENTATION_SERVICE:
    case STATE_IMPLEMENTATION_SINGLETON:
    case STATE_SERVICE:
    case STATE_REGISTEREDBY:
        return css::uno::Sequence< rtl::OUString >();
    case STATE_IMPLEMENTATION_SERVICES:
        {
            if (data_->implementations[path_[1]].services.size() >
                SAL_MAX_INT32)
            {
                throw css::registry::InvalidValueException(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "com.sun.star.registry.SimpleRegistry textual"
                            " services key openKeys: too large")),
                    static_cast< OWeakObject * >(this));
            }
            css::uno::Sequence< rtl::OUString > seq(
                static_cast< sal_Int32 >(
                    data_->implementations[path_[1]].services.size()));
            sal_Int32 i = 0;
            for (std::vector< rtl::OUString >::iterator j(
                     data_->implementations[path_[1]].services.begin());
                 j != data_->implementations[path_[1]].services.end(); ++j)
            {
                seq[i++] = *j;
            }
            return seq;
        }
    case STATE_IMPLEMENTATION_SINGLETONS:
        {
            if (data_->implementations[path_[1]].singletons.size() >
                SAL_MAX_INT32)
            {
                throw css::registry::InvalidValueException(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "com.sun.star.registry.SimpleRegistry textual"
                            " services key openKeys: too large")),
                    static_cast< OWeakObject * >(this));
            }
            css::uno::Sequence< rtl::OUString > seq(
                static_cast< sal_Int32 >(
                    data_->implementations[path_[1]].singletons.size()));
            sal_Int32 i = 0;
            for (std::vector< rtl::OUString >::iterator j(
                     data_->implementations[path_[1]].singletons.begin());
                 j != data_->implementations[path_[1]].singletons.end(); ++j)
            {
                seq[i++] = *j;
            }
            return seq;
        }
    case STATE_SERVICES:
        {
            if (data_->services.size() > SAL_MAX_INT32) {
                throw css::registry::InvalidValueException(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "com.sun.star.registry.SimpleRegistry textual"
                            " services key openKeys: too large")),
                    static_cast< OWeakObject * >(this));
            }
            css::uno::Sequence< rtl::OUString > seq(
                static_cast< sal_Int32 >(data_->services.size()));
            sal_Int32 i = 0;
            for (ImplementationMap::iterator j(data_->services.begin());
                 j != data_->services.end(); ++j)
            {
                seq[i++] = j->first;
            }
            return seq;
        }
    case STATE_SINGLETONS:
        {
            if (data_->singletons.size() > SAL_MAX_INT32) {
                throw css::registry::InvalidValueException(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "com.sun.star.registry.SimpleRegistry textual"
                            " services key openKeys: too large")),
                    static_cast< OWeakObject * >(this));
            }
            css::uno::Sequence< rtl::OUString > seq(
                static_cast< sal_Int32 >(data_->singletons.size()));
            sal_Int32 i = 0;
            for (ImplementationMap::iterator j(data_->singletons.begin());
                 j != data_->singletons.end(); ++j)
            {
                seq[i++] = j->first;
            }
            return seq;
        }
    case STATE_SINGLETON:
        {
            css::uno::Sequence< rtl::OUString > seq(1);
            seq[0] = rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("REGISTERED_BY"));
            return seq;
        }
    }
}

}

TextualServices::TextualServices(rtl::OUString const & uri):
    uri_(uri), data_(new Data)
{
    try {
        Parser(uri, data_);
    } catch (css::container::NoSuchElementException &) {
        throw css::registry::InvalidRegistryException(
            (uri +
             rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(": no such file"))),
            css::uno::Reference< css::uno::XInterface >());
    }
}

TextualServices::~TextualServices() {}

css::uno::Reference< css::registry::XRegistryKey > TextualServices::getRootKey()
{
    return new Key(data_, std::vector< rtl::OUString >());
}

} }
