/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "sal/config.h"

#include <cassert>

#include "boost/noncopyable.hpp"
#include "com/sun/star/beans/NamedValue.hpp"
#include "com/sun/star/beans/Property.hpp"
#include "com/sun/star/beans/XProperty.hpp"
#include "com/sun/star/container/NoSuchElementException.hpp"
#include "com/sun/star/container/XHierarchicalNameAccess.hpp"
#include "com/sun/star/container/XNamed.hpp"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/registry/InvalidRegistryException.hpp"
#include "com/sun/star/registry/InvalidValueException.hpp"
#include "com/sun/star/registry/MergeConflictException.hpp"
#include "com/sun/star/registry/RegistryKeyType.hpp"
#include "com/sun/star/registry/RegistryValueType.hpp"
#include "com/sun/star/registry/XRegistryKey.hpp"
#include "com/sun/star/registry/XSimpleRegistry.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/DeploymentException.hpp"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/Type.hxx"
#include "com/sun/star/uno/TypeClass.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/util/XFlushable.hpp"
#include "cppu/unotype.hxx"
#include "cppuhelper/implbase1.hxx"
#include "cppuhelper/implbase3.hxx"
#include "cppuhelper/weak.hxx"
#include "osl/mutex.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include "configurationregistry.hxx"

namespace com { namespace sun { namespace star { namespace util {
    class XFlushListener;
} } } }

namespace configmgr { namespace configuration_registry {

namespace {

class Service:
    public cppu::WeakImplHelper3<
        css::lang::XServiceInfo, css::registry::XSimpleRegistry,
        css::util::XFlushable >,
    private boost::noncopyable
{
public:
    Service(css::uno::Reference< css::uno::XComponentContext > const & context);

private:
    virtual ~Service() {}

    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException)
    { return configuration_registry::getImplementationName(); }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException)
    { return ServiceName == getSupportedServiceNames()[0]; } //TODO

    virtual css::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames() throw (css::uno::RuntimeException)
    { return configuration_registry::getSupportedServiceNames(); }

    virtual OUString SAL_CALL getURL() throw (css::uno::RuntimeException);

    virtual void SAL_CALL open(
        OUString const & rURL, sal_Bool bReadOnly, sal_Bool)
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isValid() throw (css::uno::RuntimeException);

    virtual void SAL_CALL close()
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException);

    virtual void SAL_CALL destroy()
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException);

    virtual css::uno::Reference< css::registry::XRegistryKey > SAL_CALL
    getRootKey()
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isReadOnly() throw (css::uno::RuntimeException);

    virtual void SAL_CALL mergeKey(OUString const &, OUString const &)
        throw (
            css::registry::InvalidRegistryException,
            css::registry::MergeConflictException, css::uno::RuntimeException);

    virtual void SAL_CALL flush() throw (css::uno::RuntimeException);

    virtual void SAL_CALL addFlushListener(
        css::uno::Reference< css::util::XFlushListener > const &)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL removeFlushListener(
        css::uno::Reference< css::util::XFlushListener > const &)
        throw (css::uno::RuntimeException);

    void checkValid();

    void checkValid_RuntimeException();

    void doClose();

    css::uno::Reference< css::lang::XMultiServiceFactory > provider_;
    osl::Mutex mutex_;
    css::uno::Reference< css::uno::XInterface > access_;
    OUString url_;
    bool readOnly_;

    friend class RegistryKey;
};

class RegistryKey:
    public cppu::WeakImplHelper1< css::registry::XRegistryKey >,
    private boost::noncopyable
{
public:
    RegistryKey(Service & service, css::uno::Any const & value):
        service_(service), value_(value) {}

private:
    virtual ~RegistryKey() {}

    virtual OUString SAL_CALL getKeyName()
        throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isReadOnly()
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isValid() throw (css::uno::RuntimeException);

    virtual css::registry::RegistryKeyType SAL_CALL getKeyType(
        OUString const &)
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException);

    virtual css::registry::RegistryValueType SAL_CALL getValueType()
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getLongValue()
        throw (
            css::registry::InvalidRegistryException,
            css::registry::InvalidValueException, css::uno::RuntimeException);

    virtual void SAL_CALL setLongValue(sal_Int32)
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException);

    virtual css::uno::Sequence< sal_Int32 > SAL_CALL getLongListValue()
        throw (
            css::registry::InvalidRegistryException,
            css::registry::InvalidValueException, css::uno::RuntimeException);

    virtual void SAL_CALL setLongListValue(
        css::uno::Sequence< sal_Int32 > const &)
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException);

    virtual OUString SAL_CALL getAsciiValue()
        throw (
            css::registry::InvalidRegistryException,
            css::registry::InvalidValueException, css::uno::RuntimeException);

    virtual void SAL_CALL setAsciiValue(OUString const &)
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException);

    virtual css::uno::Sequence< OUString > SAL_CALL getAsciiListValue()
        throw (
            css::registry::InvalidRegistryException,
            css::registry::InvalidValueException, css::uno::RuntimeException);

    virtual void SAL_CALL setAsciiListValue(
        css::uno::Sequence< OUString > const &)
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException);

    virtual OUString SAL_CALL getStringValue()
        throw (
            css::registry::InvalidRegistryException,
            css::registry::InvalidValueException, css::uno::RuntimeException);

    virtual void SAL_CALL setStringValue(OUString const &)
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException);

    virtual css::uno::Sequence< OUString > SAL_CALL getStringListValue()
        throw (
            css::registry::InvalidRegistryException,
            css::registry::InvalidValueException, css::uno::RuntimeException);

    virtual void SAL_CALL setStringListValue(
        css::uno::Sequence< OUString > const &)
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException);

    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getBinaryValue()
        throw (
            css::registry::InvalidRegistryException,
            css::registry::InvalidValueException, css::uno::RuntimeException);

    virtual void SAL_CALL setBinaryValue(css::uno::Sequence< sal_Int8 > const &)
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException);

    virtual css::uno::Reference< css::registry::XRegistryKey > SAL_CALL openKey(
        OUString const & aKeyName)
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException);

    virtual css::uno::Reference< css::registry::XRegistryKey > SAL_CALL
    createKey(OUString const &)
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException);

    virtual void SAL_CALL closeKey()
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException);

    virtual void SAL_CALL deleteKey(OUString const &)
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException);

    virtual
    css::uno::Sequence< css::uno::Reference< css::registry::XRegistryKey > >
    SAL_CALL openKeys()
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException);

    virtual css::uno::Sequence< OUString > SAL_CALL getKeyNames()
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL createLink(
        OUString const &, OUString const &)
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException);

    virtual void SAL_CALL deleteLink(OUString const &)
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException);

    virtual OUString SAL_CALL getLinkTarget(OUString const &)
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException);

    virtual OUString SAL_CALL getResolvedName(
        OUString const & aKeyName)
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException);

    Service & service_;
    css::uno::Any value_;
};

Service::Service(
    css::uno::Reference< css::uno::XComponentContext > const & context)
{
    assert(context.is());
    try {
        provider_ = css::uno::Reference< css::lang::XMultiServiceFactory >(
            context->getServiceManager()->createInstanceWithContext(
                "com.sun.star.configuration.DefaultProvider", context),
            css::uno::UNO_QUERY_THROW);
    } catch (css::uno::RuntimeException &) {
        throw;
    } catch (css::uno::Exception & e) {
        throw css::uno::DeploymentException(
            ("component context fails to supply service"
             " com.sun.star.configuration.DefaultProvider of type"
             " com.sun.star.lang.XMultiServiceFactory: " + e.Message),
            context);
    }
}

OUString Service::getURL() throw (css::uno::RuntimeException) {
    osl::MutexGuard g(mutex_);
    checkValid_RuntimeException();
    return url_;
}

void Service::open(OUString const & rURL, sal_Bool bReadOnly, sal_Bool)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    //TODO: bCreate
    osl::MutexGuard g(mutex_);
    if (access_.is()) {
        doClose();
    }
    css::uno::Sequence< css::uno::Any > args(1);
    args[0] <<= css::beans::NamedValue("nodepath", css::uno::makeAny(rURL));
    try {
        access_ = provider_->createInstanceWithArguments(
            (bReadOnly
             ? OUString("com.sun.star.configuration.ConfigurationAccess")
             : OUString(
                 "com.sun.star.configuration.ConfigurationUpdateAccess")),
            args);
    } catch (css::uno::RuntimeException &) {
        throw;
    } catch (css::uno::Exception & e) {
        throw css::uno::RuntimeException(
            ("com.sun.star.configuration.ConfigurationRegistry: open failed: " +
             e.Message),
            static_cast< cppu::OWeakObject * >(this));
    }
    url_ = rURL;
    readOnly_ = bReadOnly;
}

sal_Bool Service::isValid() throw (css::uno::RuntimeException) {
    osl::MutexGuard g(mutex_);
    return access_.is();
}

void Service::close()
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    osl::MutexGuard g(mutex_);
    checkValid();
    doClose();
}

void Service::destroy()
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    throw css::uno::RuntimeException(
        "com.sun.star.configuration.ConfigurationRegistry: not implemented",
        static_cast< cppu::OWeakObject * >(this));
}

css::uno::Reference< css::registry::XRegistryKey > Service::getRootKey()
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    osl::MutexGuard g(mutex_);
    checkValid();
    return new RegistryKey(*this, css::uno::makeAny(access_));
}

sal_Bool Service::isReadOnly() throw (css::uno::RuntimeException) {
    osl::MutexGuard g(mutex_);
    checkValid_RuntimeException();
    return readOnly_;
}

void Service::mergeKey(OUString const &, OUString const &)
    throw (
        css::registry::InvalidRegistryException,
        css::registry::MergeConflictException, css::uno::RuntimeException)
{
    throw css::uno::RuntimeException(
        "com.sun.star.configuration.ConfigurationRegistry: not implemented",
        static_cast< cppu::OWeakObject * >(this));
}

void Service::flush() throw (css::uno::RuntimeException)
{
    throw css::uno::RuntimeException(
        "com.sun.star.configuration.ConfigurationRegistry: not implemented",
        static_cast< cppu::OWeakObject * >(this));
}

void Service::addFlushListener(
    css::uno::Reference< css::util::XFlushListener > const &)
    throw (css::uno::RuntimeException)
{
    throw css::uno::RuntimeException(
        "com.sun.star.configuration.ConfigurationRegistry: not implemented",
        static_cast< cppu::OWeakObject * >(this));
}

void Service::removeFlushListener(
    css::uno::Reference< css::util::XFlushListener > const &)
    throw (css::uno::RuntimeException)
{
    throw css::uno::RuntimeException(
        "com.sun.star.configuration.ConfigurationRegistry: not implemented",
        static_cast< cppu::OWeakObject * >(this));
}

void Service::checkValid() {
    if (!access_.is()) {
        throw css::registry::InvalidRegistryException(
            "com.sun.star.configuration.ConfigurationRegistry: not valid",
            static_cast< cppu::OWeakObject * >(this));
    }
}

void Service::checkValid_RuntimeException() {
    if (!access_.is()) {
        throw css::uno::RuntimeException(
            "com.sun.star.configuration.ConfigurationRegistry: not valid",
            static_cast< cppu::OWeakObject * >(this));
    }
}

void Service::doClose() {
    access_.clear();
}

OUString RegistryKey::getKeyName() throw (css::uno::RuntimeException) {
    osl::MutexGuard g(service_.mutex_);
    service_.checkValid_RuntimeException();
    css::uno::Reference< css::container::XNamed > named;
    if (value_ >>= named) {
        return named->getName();
    }
    throw css::uno::RuntimeException(
        "com.sun.star.configuration.ConfigurationRegistry: not implemented",
        static_cast< cppu::OWeakObject * >(this));
}

sal_Bool RegistryKey::isReadOnly()
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    osl::MutexGuard g(service_.mutex_);
    service_.checkValid_RuntimeException();
    return service_.readOnly_; //TODO: read-only sub-nodes in update access?
}

sal_Bool RegistryKey::isValid() throw (css::uno::RuntimeException) {
    return service_.isValid();
}

css::registry::RegistryKeyType RegistryKey::getKeyType(OUString const &)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    osl::MutexGuard g(service_.mutex_);
    service_.checkValid();
    return css::registry::RegistryKeyType_KEY;
}

css::registry::RegistryValueType RegistryKey::getValueType()
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    osl::MutexGuard g(service_.mutex_);
    service_.checkValid();
    css::uno::Type t(value_.getValueType());
    switch (t.getTypeClass()) {
    case css::uno::TypeClass_LONG:
        return css::registry::RegistryValueType_LONG;
    case css::uno::TypeClass_STRING:
        return css::registry::RegistryValueType_STRING;
    case css::uno::TypeClass_SEQUENCE:
        if (t == cppu::UnoType< css::uno::Sequence< sal_Int8 > >::get()) {
            return css::registry::RegistryValueType_BINARY;
        } else if (t == cppu::UnoType< css::uno::Sequence< sal_Int32 > >::get())
        {
            return css::registry::RegistryValueType_LONGLIST;
        } else if (t ==
                   cppu::UnoType< css::uno::Sequence< OUString > >::get())
        {
            return css::registry::RegistryValueType_STRINGLIST;
        }
        // fall through
    default:
        return css::registry::RegistryValueType_NOT_DEFINED;
    }
}

sal_Int32 RegistryKey::getLongValue()
    throw (
        css::registry::InvalidRegistryException,
        css::registry::InvalidValueException, css::uno::RuntimeException)
{
    osl::MutexGuard g(service_.mutex_);
    service_.checkValid();
    sal_Int32 v = 0;
    if (value_ >>= v) {
        return v;
    }
    throw css::registry::InvalidValueException(
        "com.sun.star.configuration.ConfigurationRegistry",
        static_cast< cppu::OWeakObject * >(this));
}

void RegistryKey::setLongValue(sal_Int32)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    throw css::uno::RuntimeException(
        "com.sun.star.configuration.ConfigurationRegistry: not implemented",
        static_cast< cppu::OWeakObject * >(this));
}

css::uno::Sequence< sal_Int32 > RegistryKey::getLongListValue()
    throw (
        css::registry::InvalidRegistryException,
        css::registry::InvalidValueException, css::uno::RuntimeException)
{
    osl::MutexGuard g(service_.mutex_);
    service_.checkValid();
    css::uno::Sequence< sal_Int32 > v;
    if (value_ >>= v) {
        return v;
    }
    throw css::registry::InvalidValueException(
        "com.sun.star.configuration.ConfigurationRegistry",
        static_cast< cppu::OWeakObject * >(this));
}

void RegistryKey::setLongListValue(css::uno::Sequence< sal_Int32 > const &)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    throw css::uno::RuntimeException(
        "com.sun.star.configuration.ConfigurationRegistry: not implemented",
        static_cast< cppu::OWeakObject * >(this));
}

OUString RegistryKey::getAsciiValue()
    throw (
        css::registry::InvalidRegistryException,
        css::registry::InvalidValueException, css::uno::RuntimeException)
{
    osl::MutexGuard g(service_.mutex_);
    service_.checkValid();
    OUString v;
    if (value_ >>= v) {
        return v;
    }
    throw css::registry::InvalidValueException(
        "com.sun.star.configuration.ConfigurationRegistry",
        static_cast< cppu::OWeakObject * >(this));
}

void RegistryKey::setAsciiValue(OUString const &)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    throw css::uno::RuntimeException(
        "com.sun.star.configuration.ConfigurationRegistry: not implemented",
        static_cast< cppu::OWeakObject * >(this));
}

css::uno::Sequence< OUString > RegistryKey::getAsciiListValue()
    throw (
        css::registry::InvalidRegistryException,
        css::registry::InvalidValueException, css::uno::RuntimeException)
{
    osl::MutexGuard g(service_.mutex_);
    service_.checkValid();
    css::uno::Sequence< OUString > v;
    if (value_ >>= v) {
        return v;
    }
    throw css::registry::InvalidValueException(
        "com.sun.star.configuration.ConfigurationRegistry",
        static_cast< cppu::OWeakObject * >(this));
}

void RegistryKey::setAsciiListValue(css::uno::Sequence< OUString > const &)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    throw css::uno::RuntimeException(
        "com.sun.star.configuration.ConfigurationRegistry: not implemented",
        static_cast< cppu::OWeakObject * >(this));
}

OUString RegistryKey::getStringValue()
    throw (
        css::registry::InvalidRegistryException,
        css::registry::InvalidValueException, css::uno::RuntimeException)
{
    osl::MutexGuard g(service_.mutex_);
    service_.checkValid();
    OUString v;
    if (value_ >>= v) {
        return v;
    }
    throw css::registry::InvalidValueException(
        "com.sun.star.configuration.ConfigurationRegistry",
        static_cast< cppu::OWeakObject * >(this));
}

void RegistryKey::setStringValue(OUString const &)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    throw css::uno::RuntimeException(
        "com.sun.star.configuration.ConfigurationRegistry: not implemented",
        static_cast< cppu::OWeakObject * >(this));
}

css::uno::Sequence< OUString > RegistryKey::getStringListValue()
    throw (
        css::registry::InvalidRegistryException,
        css::registry::InvalidValueException, css::uno::RuntimeException)
{
    osl::MutexGuard g(service_.mutex_);
    service_.checkValid();
    css::uno::Sequence< OUString > v;
    if (value_ >>= v) {
        return v;
    }
    throw css::registry::InvalidValueException(
        "com.sun.star.configuration.ConfigurationRegistry",
        static_cast< cppu::OWeakObject * >(this));
}

void RegistryKey::setStringListValue(
    css::uno::Sequence< OUString > const &)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    throw css::uno::RuntimeException(
        "com.sun.star.configuration.ConfigurationRegistry: not implemented",
        static_cast< cppu::OWeakObject * >(this));
}

css::uno::Sequence< sal_Int8 > RegistryKey::getBinaryValue()
    throw (
        css::registry::InvalidRegistryException,
        css::registry::InvalidValueException, css::uno::RuntimeException)
{
    osl::MutexGuard g(service_.mutex_);
    service_.checkValid();
    css::uno::Sequence< sal_Int8 > v;
    if (value_ >>= v) {
        return v;
    }
    throw css::registry::InvalidValueException(
        "com.sun.star.configuration.ConfigurationRegistry",
        static_cast< cppu::OWeakObject * >(this));
}

void RegistryKey::setBinaryValue(css::uno::Sequence< sal_Int8 > const &)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    throw css::uno::RuntimeException(
        "com.sun.star.configuration.ConfigurationRegistry: not implemented",
        static_cast< cppu::OWeakObject * >(this));
}

css::uno::Reference< css::registry::XRegistryKey > RegistryKey::openKey(
    OUString const & aKeyName)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    osl::MutexGuard g(service_.mutex_);
    service_.checkValid_RuntimeException();
    css::uno::Reference< css::container::XHierarchicalNameAccess > access;
    if (value_ >>= access) {
        try {
            return new RegistryKey(
                service_, access->getByHierarchicalName(aKeyName));
        } catch (css::container::NoSuchElementException &) {}
    }
    return css::uno::Reference< css::registry::XRegistryKey >();
}

css::uno::Reference< css::registry::XRegistryKey > RegistryKey::createKey(
    OUString const &)
    throw (
        css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    throw css::uno::RuntimeException(
        "com.sun.star.configuration.ConfigurationRegistry: not implemented",
        static_cast< cppu::OWeakObject * >(this));
}

void RegistryKey::closeKey()
    throw (
        css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    osl::MutexGuard g(service_.mutex_);
    service_.checkValid_RuntimeException();
}

void RegistryKey::deleteKey(OUString const &)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    throw css::uno::RuntimeException(
        "com.sun.star.configuration.ConfigurationRegistry: not implemented",
        static_cast< cppu::OWeakObject * >(this));
}

css::uno::Sequence< css::uno::Reference< css::registry::XRegistryKey > >
RegistryKey::openKeys()
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    throw css::uno::RuntimeException(
        "com.sun.star.configuration.ConfigurationRegistry: not implemented",
        static_cast< cppu::OWeakObject * >(this));
}

css::uno::Sequence< OUString > RegistryKey::getKeyNames()
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    throw css::uno::RuntimeException(
        "com.sun.star.configuration.ConfigurationRegistry: not implemented",
        static_cast< cppu::OWeakObject * >(this));
}

sal_Bool RegistryKey::createLink(OUString const &, OUString const &)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    osl::MutexGuard g(service_.mutex_);
    service_.checkValid_RuntimeException();
    return false;
}

void RegistryKey::deleteLink(OUString const &)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    osl::MutexGuard g(service_.mutex_);
    service_.checkValid_RuntimeException();
}

OUString RegistryKey::getLinkTarget(OUString const &)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    osl::MutexGuard g(service_.mutex_);
    service_.checkValid_RuntimeException();
    return OUString();
}

OUString RegistryKey::getResolvedName(OUString const & aKeyName)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException)
{
    osl::MutexGuard g(service_.mutex_);
    service_.checkValid_RuntimeException();
    return aKeyName;
}

}

css::uno::Reference< css::uno::XInterface > create(
    css::uno::Reference< css::uno::XComponentContext > const & context)
{
    return static_cast< cppu::OWeakObject * >(new Service(context));
}

OUString getImplementationName() {
    return OUString("com.sun.star.comp.configuration.ConfigurationRegistry");
}

css::uno::Sequence< OUString > getSupportedServiceNames() {
    OUString name("com.sun.star.configuration.ConfigurationRegistry");
    return css::uno::Sequence< OUString >(&name, 1);
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
