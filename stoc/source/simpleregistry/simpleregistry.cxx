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

#include <sal/config.h>

#include <cstdlib>
#include <vector>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/registry/InvalidRegistryException.hpp>
#include <com/sun/star/registry/InvalidValueException.hpp>
#include <com/sun/star/registry/MergeConflictException.hpp>
#include <com/sun/star/registry/RegistryKeyType.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include <osl/mutex.hxx>
#include <registry/registry.hxx>
#include <registry/regtype.h>
#include <rtl/ref.hxx>
#include <rtl/string.h>
#include <rtl/string.hxx>
#include <rtl/textcvt.h>
#include <rtl/textenc.h>
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>
#include <sal/types.h>

namespace {

class SimpleRegistry:
    public cppu::WeakImplHelper<
        css::registry::XSimpleRegistry, css::lang::XServiceInfo >
{
public:
    SimpleRegistry() {}

    virtual ~SimpleRegistry() {}

    osl::Mutex mutex_;

private:
    virtual OUString SAL_CALL getURL() throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL open(
        OUString const & rURL, sal_Bool bReadOnly, sal_Bool bCreate)
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL isValid() throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL close() throw (
        css::registry::InvalidRegistryException, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL destroy() throw(
        css::registry::InvalidRegistryException, css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference< css::registry::XRegistryKey > SAL_CALL
    getRootKey() throw(
        css::registry::InvalidRegistryException, css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL isReadOnly() throw(
        css::registry::InvalidRegistryException, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL mergeKey(
        OUString const & aKeyName, OUString const & aUrl)
        throw (
            css::registry::InvalidRegistryException,
            css::registry::MergeConflictException, css::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override
    { return OUString("com.sun.star.comp.stoc.SimpleRegistry"); }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) override
    { return cppu::supportsService(this, ServiceName); }

    virtual css::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames() throw (css::uno::RuntimeException, std::exception) override
    {
        css::uno::Sequence< OUString > names(1);
        names[0] = "com.sun.star.registry.SimpleRegistry";
        return names;
    }

    Registry registry_;
};

class Key: public cppu::WeakImplHelper< css::registry::XRegistryKey > {
public:
    Key(
        rtl::Reference< SimpleRegistry > const & registry,
        RegistryKey const & key):
        registry_(registry), key_(key) {}

private:
    virtual OUString SAL_CALL getKeyName()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL isReadOnly() throw (
        css::registry::InvalidRegistryException, css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL isValid() throw(css::uno::RuntimeException, std::exception) override;

    virtual css::registry::RegistryKeyType SAL_CALL getKeyType(
        OUString const & rKeyName)
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException, std::exception) override;

    virtual css::registry::RegistryValueType SAL_CALL getValueType() throw(
        css::registry::InvalidRegistryException, css::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL getLongValue() throw (
        css::registry::InvalidRegistryException,
        css::registry::InvalidValueException, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setLongValue(sal_Int32 value) throw (
        css::registry::InvalidRegistryException, css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< sal_Int32 > SAL_CALL getLongListValue() throw(
        css::registry::InvalidRegistryException,
        css::registry::InvalidValueException, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setLongListValue(
        css::uno::Sequence< sal_Int32 > const & seqValue)
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL getAsciiValue() throw (
        css::registry::InvalidRegistryException,
        css::registry::InvalidValueException, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setAsciiValue(OUString const & value) throw (
        css::registry::InvalidRegistryException, css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getAsciiListValue()
        throw (
            css::registry::InvalidRegistryException,
            css::registry::InvalidValueException, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setAsciiListValue(
        css::uno::Sequence< OUString > const & seqValue)
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL getStringValue() throw(
        css::registry::InvalidRegistryException,
        css::registry::InvalidValueException, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setStringValue(OUString const & value) throw (
        css::registry::InvalidRegistryException, css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getStringListValue()
        throw (
            css::registry::InvalidRegistryException,
            css::registry::InvalidValueException, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setStringListValue(
        css::uno::Sequence< OUString > const & seqValue)
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getBinaryValue() throw (
        css::registry::InvalidRegistryException,
        css::registry::InvalidValueException, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setBinaryValue(
        css::uno::Sequence< sal_Int8 > const & value)
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference< css::registry::XRegistryKey > SAL_CALL openKey(
        OUString const & aKeyName)
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference< css::registry::XRegistryKey > SAL_CALL
    createKey(OUString const & aKeyName) throw (
        css::registry::InvalidRegistryException, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL closeKey() throw (
        css::registry::InvalidRegistryException, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL deleteKey(OUString const & rKeyName) throw (
        css::registry::InvalidRegistryException, css::uno::RuntimeException, std::exception) override;

    virtual
    css::uno::Sequence< css::uno::Reference< css::registry::XRegistryKey > >
    SAL_CALL openKeys() throw (
        css::registry::InvalidRegistryException, css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getKeyNames() throw (
        css::registry::InvalidRegistryException, css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL createLink(
        OUString const & aLinkName, OUString const & aLinkTarget)
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL deleteLink(OUString const & rLinkName) throw (
        css::registry::InvalidRegistryException, css::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL getLinkTarget(OUString const & rLinkName)
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL getResolvedName(OUString const & aKeyName)
        throw (
            css::registry::InvalidRegistryException,
            css::uno::RuntimeException, std::exception) override;

    rtl::Reference< SimpleRegistry > registry_;
    RegistryKey key_;
};

OUString Key::getKeyName() throw (css::uno::RuntimeException, std::exception) {
    osl::MutexGuard guard(registry_->mutex_);
    return key_.getName();
}

sal_Bool Key::isReadOnly()
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard guard(registry_->mutex_);
    return key_.isReadOnly();
}

sal_Bool Key::isValid() throw (css::uno::RuntimeException, std::exception) {
    osl::MutexGuard guard(registry_->mutex_);
    return key_.isValid();
}

css::registry::RegistryKeyType Key::getKeyType(OUString const & )
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException, std::exception)
{
    return css::registry::RegistryKeyType_KEY;
}

css::registry::RegistryValueType Key::getValueType()
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard guard(registry_->mutex_);
    RegValueType type;
    sal_uInt32 size;
    RegError err = key_.getValueInfo(OUString(), &type, &size);
    switch (err) {
    case RegError::NO_ERROR:
        break;
    case RegError::INVALID_VALUE:
        type = RegValueType::NOT_DEFINED;
        break;
    default:
        throw css::registry::InvalidRegistryException(
            (("com.sun.star.registry.SimpleRegistry key getValueType:"
                      " underlying RegistryKey::getValueInfo() = ") +
             OUString::number(static_cast<int>(err))),
            static_cast< OWeakObject * >(this));
    }
    switch (type) {
    default:
        std::abort(); // this cannot happen
        // pseudo-fall-through to avoid warnings on MSC
    case RegValueType::NOT_DEFINED:
        return css::registry::RegistryValueType_NOT_DEFINED;
    case RegValueType::LONG:
        return css::registry::RegistryValueType_LONG;
    case RegValueType::STRING:
        return css::registry::RegistryValueType_ASCII;
    case RegValueType::UNICODE:
        return css::registry::RegistryValueType_STRING;
    case RegValueType::BINARY:
        return css::registry::RegistryValueType_BINARY;
    case RegValueType::LONGLIST:
        return css::registry::RegistryValueType_LONGLIST;
    case RegValueType::STRINGLIST:
        return css::registry::RegistryValueType_ASCIILIST;
    case RegValueType::UNICODELIST:
        return css::registry::RegistryValueType_STRINGLIST;
    }
}

sal_Int32 Key::getLongValue() throw (
    css::registry::InvalidRegistryException,
    css::registry::InvalidValueException, css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard guard(registry_->mutex_);
    sal_Int32 value;
    RegError err = key_.getValue(OUString(), &value);
    switch (err) {
    case RegError::NO_ERROR:
        break;
    case RegError::INVALID_VALUE:
        throw css::registry::InvalidValueException(
            OUString("com.sun.star.registry.SimpleRegistry key getLongValue:"
                     " underlying RegistryKey::getValue() = RegError::INVALID_VALUE"),
            static_cast< OWeakObject * >(this));
    default:
        throw css::registry::InvalidRegistryException(
            (("com.sun.star.registry.SimpleRegistry key getLongValue:"
                      " underlying RegistryKey::getValue() = ") +
             OUString::number(static_cast<int>(err))),
            static_cast< OWeakObject * >(this));
    }
    return value;
}

void Key::setLongValue(sal_Int32 value)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard guard(registry_->mutex_);
    RegError err = key_.setValue(
        OUString(), RegValueType::LONG, &value, sizeof (sal_Int32));
    if (err != RegError::NO_ERROR) {
        throw css::registry::InvalidRegistryException(
            (("com.sun.star.registry.SimpleRegistry key setLongValue:"
                      " underlying RegistryKey::setValue() = ") +
             OUString::number(static_cast<int>(err))),
            static_cast< OWeakObject * >(this));
    }
}

css::uno::Sequence< sal_Int32 > Key::getLongListValue() throw (
    css::registry::InvalidRegistryException,
    css::registry::InvalidValueException, css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard guard(registry_->mutex_);
    RegistryValueList< sal_Int32 > list;
    RegError err = key_.getLongListValue(OUString(), list);
    switch (err) {
    case RegError::NO_ERROR:
        break;
    case RegError::VALUE_NOT_EXISTS:
        return css::uno::Sequence< sal_Int32 >();
    case RegError::INVALID_VALUE:
        throw css::registry::InvalidValueException(
            OUString("com.sun.star.registry.SimpleRegistry key getLongListValue:"
                     " underlying RegistryKey::getLongListValue() ="
                     " RegError::INVALID_VALUE"),
            static_cast< OWeakObject * >(this));
    default:
        throw css::registry::InvalidRegistryException(
            (("com.sun.star.registry.SimpleRegistry key getLongListValue:"
                      " underlying RegistryKey::getLongListValue() = ") +
             OUString::number(static_cast<int>(err))),
            static_cast< OWeakObject * >(this));
    }
    sal_uInt32 n = list.getLength();
    if (n > SAL_MAX_INT32) {
        throw css::registry::InvalidValueException(
            OUString("com.sun.star.registry.SimpleRegistry key getLongListValue:"
                     " underlying RegistryKey::getLongListValue() too large"),
            static_cast< OWeakObject * >(this));
    }
    css::uno::Sequence< sal_Int32 > value(static_cast< sal_Int32 >(n));
    for (sal_uInt32 i = 0; i < n; ++i) {
        value[static_cast< sal_Int32 >(i)] = list.getElement(i);
    }
    return value;
}

void Key::setLongListValue(css::uno::Sequence< sal_Int32 > const & seqValue)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard guard(registry_->mutex_);
    std::vector< sal_Int32 > list;
    for (sal_Int32 i = 0; i < seqValue.getLength(); ++i) {
        list.push_back(seqValue[i]);
    }
    RegError err = key_.setLongListValue(
        OUString(), list.data(), static_cast< sal_uInt32 >(list.size()));
    if (err != RegError::NO_ERROR) {
        throw css::registry::InvalidRegistryException(
            (("com.sun.star.registry.SimpleRegistry key setLongListValue:"
                      " underlying RegistryKey::setLongListValue() = ") +
             OUString::number(static_cast<int>(err))),
            static_cast< OWeakObject * >(this));
    }
}

OUString Key::getAsciiValue() throw (
    css::registry::InvalidRegistryException,
    css::registry::InvalidValueException, css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard guard(registry_->mutex_);
    RegValueType type;
    sal_uInt32 size;
    RegError err = key_.getValueInfo(OUString(), &type, &size);
    if (err != RegError::NO_ERROR) {
        throw css::registry::InvalidRegistryException(
            (("com.sun.star.registry.SimpleRegistry key getAsciiValue:"
                      " underlying RegistryKey::getValueInfo() = ") +
             OUString::number(static_cast<int>(err))),
            static_cast< OWeakObject * >(this));
    }
    if (type != RegValueType::STRING) {
        throw css::registry::InvalidValueException(
            (("com.sun.star.registry.SimpleRegistry key getAsciiValue:"
                      " underlying RegistryKey type = ") +
             OUString::number(static_cast<int>(type))),
            static_cast< OWeakObject * >(this));
    }
    // size contains terminating null (error in underlying registry.cxx):
    if (size == 0) {
        throw css::registry::InvalidValueException(
            OUString("com.sun.star.registry.SimpleRegistry key getAsciiValue:"
                     " underlying RegistryKey size 0 cannot happen due to"
                     " design error"),
            static_cast< OWeakObject * >(this));
    }
    if (size > SAL_MAX_INT32) {
        throw css::registry::InvalidValueException(
            OUString("com.sun.star.registry.SimpleRegistry key getAsciiValue:"
                     " underlying RegistryKey size too large"),
            static_cast< OWeakObject * >(this));
    }
    std::vector< char > list(size);
    err = key_.getValue(OUString(), &list[0]);
    if (err != RegError::NO_ERROR) {
        throw css::registry::InvalidRegistryException(
            (("com.sun.star.registry.SimpleRegistry key getAsciiValue:"
                      " underlying RegistryKey::getValue() = ") +
             OUString::number(static_cast<int>(err))),
            static_cast< OWeakObject * >(this));
    }
    if (list[size - 1] != '\0') {
        throw css::registry::InvalidValueException(
            OUString("com.sun.star.registry.SimpleRegistry key getAsciiValue:"
                     " underlying RegistryKey value must be null-terminated due"
                     " to design error"),
            static_cast< OWeakObject * >(this));
    }
    OUString value;
    if (!rtl_convertStringToUString(
            &value.pData, &list[0],
            static_cast< sal_Int32 >(size - 1), RTL_TEXTENCODING_UTF8,
            (RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR |
             RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR |
             RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR)))
    {
        throw css::registry::InvalidValueException(
            OUString("com.sun.star.registry.SimpleRegistry key getAsciiValue:"
                     " underlying RegistryKey not UTF-8"),
            static_cast< OWeakObject * >(this));
    }
    return value;
}

void Key::setAsciiValue(OUString const & value)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard guard(registry_->mutex_);
    OString utf8;
    if (!value.convertToString(
            &utf8, RTL_TEXTENCODING_UTF8,
            (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR |
             RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR)))
    {
        throw css::uno::RuntimeException(
            OUString("com.sun.star.registry.SimpleRegistry key setAsciiValue:"
                     " value not UTF-16"),
            static_cast< OWeakObject * >(this));
    }
    RegError err = key_.setValue(
        OUString(), RegValueType::STRING,
        const_cast< char * >(utf8.getStr()), utf8.getLength() + 1);
        // +1 for terminating null (error in underlying registry.cxx)
    if (err != RegError::NO_ERROR) {
        throw css::registry::InvalidRegistryException(
            (("com.sun.star.registry.SimpleRegistry key setAsciiValue:"
                      " underlying RegistryKey::setValue() = ") +
             OUString::number(static_cast<int>(err))),
            static_cast< OWeakObject * >(this));
    }
}

css::uno::Sequence< OUString > Key::getAsciiListValue() throw (
    css::registry::InvalidRegistryException,
    css::registry::InvalidValueException, css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard guard(registry_->mutex_);
    RegistryValueList< char * > list;
    RegError err = key_.getStringListValue(OUString(), list);
    switch (err) {
    case RegError::NO_ERROR:
        break;
    case RegError::VALUE_NOT_EXISTS:
        return css::uno::Sequence< OUString >();
    case RegError::INVALID_VALUE:
        throw css::registry::InvalidValueException(
            OUString("com.sun.star.registry.SimpleRegistry key"
                     " getAsciiListValue: underlying"
                     " RegistryKey::getStringListValue() = RegError::INVALID_VALUE"),
            static_cast< OWeakObject * >(this));
    default:
        throw css::registry::InvalidRegistryException(
            (("com.sun.star.registry.SimpleRegistry key"
                      " getAsciiListValue: underlying"
                      " RegistryKey::getStringListValue() = ") +
             OUString::number(static_cast<int>(err))),
            static_cast< OWeakObject * >(this));
    }
    sal_uInt32 n = list.getLength();
    if (n > SAL_MAX_INT32) {
        throw css::registry::InvalidValueException(
            OUString("com.sun.star.registry.SimpleRegistry key"
                     " getAsciiListValue: underlying"
                     " RegistryKey::getStringListValue() too large"),
            static_cast< OWeakObject * >(this));
    }
    css::uno::Sequence< OUString > value(static_cast< sal_Int32 >(n));
    for (sal_uInt32 i = 0; i < n; ++i) {
        char * el = list.getElement(i);
        sal_Int32 size = rtl_str_getLength(el);
        if (!rtl_convertStringToUString(
                &value[static_cast< sal_Int32 >(i)].pData, el, size,
                RTL_TEXTENCODING_UTF8,
                (RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR |
                 RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR |
                 RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR)))
        {
            throw css::registry::InvalidValueException(
                OUString("com.sun.star.registry.SimpleRegistry key"
                         " getAsciiListValue: underlying RegistryKey not"
                         " UTF-8"),
                static_cast< OWeakObject * >(this));
        }
    }
    return value;
}

void Key::setAsciiListValue(
    css::uno::Sequence< OUString > const & seqValue)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard guard(registry_->mutex_);
    std::vector< OString > list;
    for (sal_Int32 i = 0; i < seqValue.getLength(); ++i) {
        OString utf8;
        if (!seqValue[i].convertToString(
                &utf8, RTL_TEXTENCODING_UTF8,
                (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR |
                 RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR)))
        {
            throw css::uno::RuntimeException(
                OUString("com.sun.star.registry.SimpleRegistry key"
                         " setAsciiListValue: value not UTF-16"),
                static_cast< OWeakObject * >(this));
        }
        list.push_back(utf8);
    }
    std::vector< char * > list2;
    for (std::vector< OString >::iterator i(list.begin()); i != list.end();
         ++i)
    {
        list2.push_back(const_cast< char * >(i->getStr()));
    }
    RegError err = key_.setStringListValue(
        OUString(), list2.data(), static_cast< sal_uInt32 >(list2.size()));
    if (err != RegError::NO_ERROR) {
        throw css::registry::InvalidRegistryException(
            (("com.sun.star.registry.SimpleRegistry key"
                      " setAsciiListValue: underlying"
                      " RegistryKey::setStringListValue() = ") +
             OUString::number(static_cast<int>(err))),
            static_cast< OWeakObject * >(this));
    }
}

OUString Key::getStringValue() throw (
    css::registry::InvalidRegistryException,
    css::registry::InvalidValueException, css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard guard(registry_->mutex_);
    RegValueType type;
    sal_uInt32 size;
    RegError err = key_.getValueInfo(OUString(), &type, &size);
    if (err != RegError::NO_ERROR) {
        throw css::registry::InvalidRegistryException(
            (("com.sun.star.registry.SimpleRegistry key getStringValue:"
                      " underlying RegistryKey::getValueInfo() = ") +
             OUString::number(static_cast<int>(err))),
            static_cast< OWeakObject * >(this));
    }
    if (type != RegValueType::UNICODE) {
        throw css::registry::InvalidValueException(
            (("com.sun.star.registry.SimpleRegistry key getStringValue:"
                      " underlying RegistryKey type = ") +
             OUString::number(static_cast<int>(type))),
            static_cast< OWeakObject * >(this));
    }
    // size contains terminating null and is *2 (error in underlying
    // registry.cxx):
    if (size == 0 || (size & 1) == 1) {
        throw css::registry::InvalidValueException(
            OUString("com.sun.star.registry.SimpleRegistry key getStringValue:"
                     " underlying RegistryKey size 0 or odd cannot happen due to"
                     " design error"),
            static_cast< OWeakObject * >(this));
    }
    if (size > SAL_MAX_INT32) {
        throw css::registry::InvalidValueException(
            OUString("com.sun.star.registry.SimpleRegistry key getStringValue:"
                     " underlying RegistryKey size too large"),
            static_cast< OWeakObject * >(this));
    }
    std::vector< sal_Unicode > list(size);
    err = key_.getValue(OUString(), &list[0]);
    if (err != RegError::NO_ERROR) {
        throw css::registry::InvalidRegistryException(
            (("com.sun.star.registry.SimpleRegistry key getStringValue:"
                      " underlying RegistryKey::getValue() = ") +
             OUString::number(static_cast<int>(err))),
            static_cast< OWeakObject * >(this));
    }
    if (list[size/2 - 1] != 0) {
        throw css::registry::InvalidValueException(
            OUString("com.sun.star.registry.SimpleRegistry key getStringValue:"
                     " underlying RegistryKey value must be null-terminated due"
                     " to design error"),
            static_cast< OWeakObject * >(this));
    }
    return OUString(&list[0], static_cast< sal_Int32 >(size/2 - 1));
}

void Key::setStringValue(OUString const & value)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard guard(registry_->mutex_);
    RegError err = key_.setValue(
        OUString(), RegValueType::UNICODE,
        const_cast< sal_Unicode * >(value.getStr()),
        (value.getLength() + 1) * sizeof (sal_Unicode));
        // +1 for terminating null (error in underlying registry.cxx)
    if (err != RegError::NO_ERROR) {
        throw css::registry::InvalidRegistryException(
            (("com.sun.star.registry.SimpleRegistry key setStringValue:"
                      " underlying RegistryKey::setValue() = ") +
             OUString::number(static_cast<int>(err))),
            static_cast< OWeakObject * >(this));
    }
}

css::uno::Sequence< OUString > Key::getStringListValue() throw (
    css::registry::InvalidRegistryException,
    css::registry::InvalidValueException, css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard guard(registry_->mutex_);
    RegistryValueList< sal_Unicode * > list;
    RegError err = key_.getUnicodeListValue(OUString(), list);
    switch (err) {
    case RegError::NO_ERROR:
        break;
    case RegError::VALUE_NOT_EXISTS:
        return css::uno::Sequence< OUString >();
    case RegError::INVALID_VALUE:
        throw css::registry::InvalidValueException(
            OUString("com.sun.star.registry.SimpleRegistry key"
                     " getStringListValue: underlying"
                     " RegistryKey::getUnicodeListValue() = RegError::INVALID_VALUE"),
            static_cast< OWeakObject * >(this));
    default:
        throw css::registry::InvalidRegistryException(
            (("com.sun.star.registry.SimpleRegistry key"
                      " getStringListValue: underlying"
                      " RegistryKey::getUnicodeListValue() = ") +
             OUString::number(static_cast<int>(err))),
            static_cast< OWeakObject * >(this));
    }
    sal_uInt32 n = list.getLength();
    if (n > SAL_MAX_INT32) {
        throw css::registry::InvalidValueException(
            OUString("com.sun.star.registry.SimpleRegistry key"
                     " getStringListValue: underlying"
                     " RegistryKey::getUnicodeListValue() too large"),
            static_cast< OWeakObject * >(this));
    }
    css::uno::Sequence< OUString > value(static_cast< sal_Int32 >(n));
    for (sal_uInt32 i = 0; i < n; ++i) {
        value[static_cast< sal_Int32 >(i)] = list.getElement(i);
    }
    return value;
}

void Key::setStringListValue(
    css::uno::Sequence< OUString > const & seqValue)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard guard(registry_->mutex_);
    std::vector< sal_Unicode * > list;
    for (sal_Int32 i = 0; i < seqValue.getLength(); ++i) {
        list.push_back(const_cast< sal_Unicode * >(seqValue[i].getStr()));
    }
    RegError err = key_.setUnicodeListValue(
        OUString(), list.data(), static_cast< sal_uInt32 >(list.size()));
    if (err != RegError::NO_ERROR) {
        throw css::registry::InvalidRegistryException(
            (("com.sun.star.registry.SimpleRegistry key"
                      " setStringListValue: underlying"
                      " RegistryKey::setUnicodeListValue() = ") +
             OUString::number(static_cast<int>(err))),
            static_cast< OWeakObject * >(this));
    }
}

css::uno::Sequence< sal_Int8 > Key::getBinaryValue()
    throw (
        css::registry::InvalidRegistryException,
        css::registry::InvalidValueException, css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard guard(registry_->mutex_);
    RegValueType type;
    sal_uInt32 size;
    RegError err = key_.getValueInfo(OUString(), &type, &size);
    if (err != RegError::NO_ERROR) {
        throw css::registry::InvalidRegistryException(
            (("com.sun.star.registry.SimpleRegistry key getBinaryValue:"
                      " underlying RegistryKey::getValueInfo() = ") +
             OUString::number(static_cast<int>(err))),
            static_cast< OWeakObject * >(this));
    }
    if (type != RegValueType::BINARY) {
        throw css::registry::InvalidValueException(
            (("com.sun.star.registry.SimpleRegistry key getBinaryValue:"
                      " underlying RegistryKey type = ") +
             OUString::number(static_cast<int>(type))),
            static_cast< OWeakObject * >(this));
    }
    if (size > SAL_MAX_INT32) {
        throw css::registry::InvalidValueException(
            OUString("com.sun.star.registry.SimpleRegistry key getBinaryValue:"
                     " underlying RegistryKey size too large"),
            static_cast< OWeakObject * >(this));
    }
    css::uno::Sequence< sal_Int8 > value(static_cast< sal_Int32 >(size));
    err = key_.getValue(OUString(), value.getArray());
    if (err != RegError::NO_ERROR) {
        throw css::registry::InvalidRegistryException(
            (("com.sun.star.registry.SimpleRegistry key getBinaryValue:"
                      " underlying RegistryKey::getValue() = ") +
             OUString::number(static_cast<int>(err))),
            static_cast< OWeakObject * >(this));
    }
    return value;
}

void Key::setBinaryValue(css::uno::Sequence< sal_Int8 > const & value)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard guard(registry_->mutex_);
    RegError err = key_.setValue(
        OUString(), RegValueType::BINARY,
        const_cast< sal_Int8 * >(value.getConstArray()),
        static_cast< sal_uInt32 >(value.getLength()));
    if (err != RegError::NO_ERROR) {
        throw css::registry::InvalidRegistryException(
            (("com.sun.star.registry.SimpleRegistry key setBinaryValue:"
                      " underlying RegistryKey::setValue() = ") +
             OUString::number(static_cast<int>(err))),
            static_cast< OWeakObject * >(this));
    }
}

css::uno::Reference< css::registry::XRegistryKey > Key::openKey(
    OUString const & aKeyName)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard guard(registry_->mutex_);
    RegistryKey key;
    RegError err = key_.openKey(aKeyName, key);
    switch (err) {
    case RegError::NO_ERROR:
        return new Key(registry_, key);
    case RegError::KEY_NOT_EXISTS:
        return css::uno::Reference< css::registry::XRegistryKey >();
    default:
        throw css::registry::InvalidRegistryException(
            (("com.sun.star.registry.SimpleRegistry key openKey:"
                      " underlying RegistryKey::openKey() = ") +
             OUString::number(static_cast<int>(err))),
            static_cast< OWeakObject * >(this));
    }
}

css::uno::Reference< css::registry::XRegistryKey > Key::createKey(
    OUString const & aKeyName)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard guard(registry_->mutex_);
    RegistryKey key;
    RegError err = key_.createKey(aKeyName, key);
    switch (err) {
    case RegError::NO_ERROR:
        return new Key(registry_, key);
    case RegError::INVALID_KEYNAME:
        return css::uno::Reference< css::registry::XRegistryKey >();
    default:
        throw css::registry::InvalidRegistryException(
            (("com.sun.star.registry.SimpleRegistry key createKey:"
                      " underlying RegistryKey::createKey() = ") +
             OUString::number(static_cast<int>(err))),
            static_cast< OWeakObject * >(this));
    }
}

void Key::closeKey()
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard guard(registry_->mutex_);
    RegError err = key_.closeKey();
    if (err != RegError::NO_ERROR) {
        throw css::registry::InvalidRegistryException(
            (("com.sun.star.registry.SimpleRegistry key closeKey:"
                      " underlying RegistryKey::closeKey() = ") +
             OUString::number(static_cast<int>(err))),
            static_cast< OWeakObject * >(this));
    }
}

void Key::deleteKey(OUString const & rKeyName)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard guard(registry_->mutex_);
    RegError err = key_.deleteKey(rKeyName);
    if (err != RegError::NO_ERROR) {
        throw css::registry::InvalidRegistryException(
            (("com.sun.star.registry.SimpleRegistry key deleteKey:"
                      " underlying RegistryKey::deleteKey() = ") +
             OUString::number(static_cast<int>(err))),
            static_cast< OWeakObject * >(this));
    }
}

css::uno::Sequence< css::uno::Reference< css::registry::XRegistryKey > >
Key::openKeys()
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard guard(registry_->mutex_);
    RegistryKeyArray list;
    RegError err = key_.openSubKeys(OUString(), list);
    if (err != RegError::NO_ERROR) {
        throw css::registry::InvalidRegistryException(
            (("com.sun.star.registry.SimpleRegistry key openKeys:"
                      " underlying RegistryKey::openSubKeys() = ") +
             OUString::number(static_cast<int>(err))),
            static_cast< OWeakObject * >(this));
    }
    sal_uInt32 n = list.getLength();
    if (n > SAL_MAX_INT32) {
        throw css::registry::InvalidRegistryException(
            OUString("com.sun.star.registry.SimpleRegistry key getKeyNames:"
                     " underlying RegistryKey::getKeyNames() too large"),
            static_cast< OWeakObject * >(this));
    }
    css::uno::Sequence< css::uno::Reference< css::registry::XRegistryKey > >
        keys(static_cast< sal_Int32 >(n));
    for (sal_uInt32 i = 0; i < n; ++i) {
        keys[static_cast< sal_Int32 >(i)] = new Key(
            registry_, list.getElement(i));
    }
    return keys;
}

css::uno::Sequence< OUString > Key::getKeyNames()
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard guard(registry_->mutex_);
    RegistryKeyNames list;
    RegError err = key_.getKeyNames(OUString(), list);
    if (err != RegError::NO_ERROR) {
        throw css::registry::InvalidRegistryException(
            (("com.sun.star.registry.SimpleRegistry key getKeyNames:"
                      " underlying RegistryKey::getKeyNames() = ") +
             OUString::number(static_cast<int>(err))),
            static_cast< OWeakObject * >(this));
    }
    sal_uInt32 n = list.getLength();
    if (n > SAL_MAX_INT32) {
        throw css::registry::InvalidRegistryException(
            OUString("com.sun.star.registry.SimpleRegistry key getKeyNames:"
                     " underlying RegistryKey::getKeyNames() too large"),
            static_cast< OWeakObject * >(this));
    }
    css::uno::Sequence< OUString > names(static_cast< sal_Int32 >(n));
    for (sal_uInt32 i = 0; i < n; ++i) {
        names[static_cast< sal_Int32 >(i)] = list.getElement(i);
    }
    return names;
}

sal_Bool Key::createLink(
    OUString const & aLinkName, OUString const & aLinkTarget)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard guard(registry_->mutex_);
    RegError err = key_.createLink(aLinkName, aLinkTarget);
    switch (err) {
    case RegError::NO_ERROR:
        return true;
    case RegError::INVALID_KEY:
    case RegError::DETECT_RECURSION:
        throw css::registry::InvalidRegistryException(
            (("com.sun.star.registry.SimpleRegistry key createLink:"
                      " underlying RegistryKey::createLink() = ") +
             OUString::number(static_cast<int>(err))),
            static_cast< OWeakObject * >(this));
    default:
        return false;
    }
}

void Key::deleteLink(OUString const & rLinkName)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard guard(registry_->mutex_);
    RegError err = key_.deleteLink(rLinkName);
    if (err != RegError::NO_ERROR) {
        throw css::registry::InvalidRegistryException(
            (("com.sun.star.registry.SimpleRegistry key deleteLink:"
                      " underlying RegistryKey::deleteLink() = ") +
             OUString::number(static_cast<int>(err))),
            static_cast< OWeakObject * >(this));
    }
}

OUString Key::getLinkTarget(OUString const & rLinkName)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard guard(registry_->mutex_);
    OUString target;
    RegError err = key_.getLinkTarget(rLinkName, target);
    if (err != RegError::NO_ERROR) {
        throw css::registry::InvalidRegistryException(
            (("com.sun.star.registry.SimpleRegistry key getLinkTarget:"
                      " underlying RegistryKey::getLinkTarget() = ") +
             OUString::number(static_cast<int>(err))),
            static_cast< OWeakObject * >(this));
    }
    return target;
}

OUString Key::getResolvedName(OUString const & aKeyName)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard guard(registry_->mutex_);
    OUString resolved;
    RegError err = key_.getResolvedKeyName(aKeyName, true, resolved);
    if (err != RegError::NO_ERROR) {
        throw css::registry::InvalidRegistryException(
            (("com.sun.star.registry.SimpleRegistry key getResolvedName:"
                      " underlying RegistryKey::getResolvedName() = ") +
             OUString::number(static_cast<int>(err))),
            static_cast< OWeakObject * >(this));
    }
    return resolved;
}

OUString SimpleRegistry::getURL() throw (css::uno::RuntimeException, std::exception) {
    osl::MutexGuard guard(mutex_);
    return registry_.getName();
}

void SimpleRegistry::open(
    OUString const & rURL, sal_Bool bReadOnly, sal_Bool bCreate)
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard guard(mutex_);
    RegError err = (rURL.isEmpty() && bCreate)
        ? RegError::REGISTRY_NOT_EXISTS
        : registry_.open(rURL, bReadOnly ? RegAccessMode::READONLY : RegAccessMode::READWRITE);
    if (err == RegError::REGISTRY_NOT_EXISTS && bCreate) {
        err = registry_.create(rURL);
    }
    if (err != RegError::NO_ERROR) {
        throw css::registry::InvalidRegistryException(
            ("com.sun.star.registry.SimpleRegistry.open(" +
             rURL +
             "): underlying Registry::open/create() = " +
             OUString::number(static_cast<int>(err))),
            static_cast< OWeakObject * >(this));
    }
}

sal_Bool SimpleRegistry::isValid() throw (css::uno::RuntimeException, std::exception) {
    osl::MutexGuard guard(mutex_);
    return registry_.isValid();
}

void SimpleRegistry::close()
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard guard(mutex_);
    RegError err = registry_.close();
    if (err != RegError::NO_ERROR) {
        throw css::registry::InvalidRegistryException(
            (("com.sun.star.registry.SimpleRegistry.close:"
                      " underlying Registry::close() = ") +
             OUString::number(static_cast<int>(err))),
            static_cast< OWeakObject * >(this));
    }
}

void SimpleRegistry::destroy()
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard guard(mutex_);
    RegError err = registry_.destroy(OUString());
    if (err != RegError::NO_ERROR) {
        throw css::registry::InvalidRegistryException(
            (("com.sun.star.registry.SimpleRegistry.destroy:"
                      " underlying Registry::destroy() = ") +
             OUString::number(static_cast<int>(err))),
            static_cast< OWeakObject * >(this));
    }
}

css::uno::Reference< css::registry::XRegistryKey > SimpleRegistry::getRootKey()
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard guard(mutex_);
    RegistryKey root;
    RegError err = registry_.openRootKey(root);
    if (err != RegError::NO_ERROR) {
        throw css::registry::InvalidRegistryException(
            (("com.sun.star.registry.SimpleRegistry.getRootKey:"
                      " underlying Registry::getRootKey() = ") +
             OUString::number(static_cast<int>(err))),
            static_cast< OWeakObject * >(this));
    }
    return new Key(this, root);
}

sal_Bool SimpleRegistry::isReadOnly()
    throw (css::registry::InvalidRegistryException, css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard guard(mutex_);
    return registry_.isReadOnly();
}

void SimpleRegistry::mergeKey(
    OUString const & aKeyName, OUString const & aUrl)
    throw (
        css::registry::InvalidRegistryException,
        css::registry::MergeConflictException, css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard guard(mutex_);
    RegistryKey root;
    RegError err = registry_.openRootKey(root);
    if (err == RegError::NO_ERROR) {
        err = registry_.mergeKey(root, aKeyName, aUrl);
    }
    switch (err) {
    case RegError::NO_ERROR:
    case RegError::MERGE_CONFLICT:
        break;
    case RegError::MERGE_ERROR:
        throw css::registry::MergeConflictException(
            OUString("com.sun.star.registry.SimpleRegistry.mergeKey:"
                     " underlying Registry::mergeKey() = RegError::MERGE_ERROR"),
            static_cast< cppu::OWeakObject * >(this));
    default:
        throw css::registry::InvalidRegistryException(
            (("com.sun.star.registry.SimpleRegistry.mergeKey:"
                      " underlying Registry::getRootKey/mergeKey() = ") +
             OUString::number(static_cast<int>(err))),
            static_cast< OWeakObject * >(this));
    }
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_stoc_SimpleRegistry_get_implementation(
    SAL_UNUSED_PARAMETER css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SimpleRegistry);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
