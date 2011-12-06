/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "sal/config.h"

#include <cassert>

#include "boost/noncopyable.hpp"
#include "com/sun/star/beans/NamedValue.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/container/ElementExistException.hpp"
#include "com/sun/star/container/NoSuchElementException.hpp"
#include "com/sun/star/container/XNameContainer.hpp"
#include "com/sun/star/lang/Locale.hpp"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/lang/XSingleServiceFactory.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/util/XChangesBatch.hpp"
#include "comphelper/processfactory.hxx"
#include "rtl/oustringostreaminserter.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/log.hxx"
#include "sal/types.h"
#include "svl/asiancfg.hxx"

namespace {

namespace css = com::sun::star;

css::uno::Reference< css::beans::XPropertySet >
obtainPropertySet() {
    css::uno::Sequence< css::uno::Any > args(1);
    args[0] <<= css::beans::NamedValue(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("nodepath")),
        css::uno::makeAny(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "/org.openoffice.Office.Common/AsianLayout"))));
    return css::uno::Reference< css::beans::XPropertySet >(
        (css::uno::Reference< css::lang::XMultiServiceFactory >(
            (css::uno::Reference< css::lang::XMultiServiceFactory >(
                comphelper::getProcessServiceFactory(),
                css::uno::UNO_SET_THROW)->
             createInstance(
                 rtl::OUString(
                     RTL_CONSTASCII_USTRINGPARAM(
                         "com.sun.star.configuration.ConfigurationProvider")))),
            css::uno::UNO_QUERY_THROW)->
         createInstanceWithArguments(
             rtl::OUString(
                 RTL_CONSTASCII_USTRINGPARAM(
                     "com.sun.star.configuration.ConfigurationUpdateAccess")),
             args)),
        css::uno::UNO_QUERY_THROW);
}

rtl::OUString toString(css::lang::Locale const & locale) {
    SAL_WARN_IF(
        locale.Language.indexOf('-') != -1, "svl",
        "Locale language \"" << locale.Language << "\" contains \"-\"");
    rtl::OUStringBuffer buf(locale.Language);
    SAL_WARN_IF(
        locale.Country.isEmpty() && !locale.Variant.isEmpty(), "svl",
        "Locale has empty country but non-empty variant \"" << locale.Variant
            << '"');
    if (!locale.Country.isEmpty()) {
        buf.append('-');
        SAL_WARN_IF(
            locale.Country.indexOf('-') != -1, "svl",
            "Locale country \"" << locale.Country << "\" contains \"-\"");
        buf.append(locale.Country);
        if (!locale.Variant.isEmpty()) {
            buf.append('-');
            buf.append(locale.Variant);
        }
    }
    return buf.makeStringAndClear();
}

}

class SvxAsianConfig::Impl: private boost::noncopyable {
public:
    Impl(): propertySet_(obtainPropertySet()) {}

    css::uno::Reference< css::beans::XPropertySet > getPropertySet() const
    { return propertySet_; }

    css::uno::Reference< css::container::XNameContainer >
    getStartEndCharacters() const;

private:
    css::uno::Reference< css::beans::XPropertySet > propertySet_;
};

css::uno::Reference< css::container::XNameContainer >
SvxAsianConfig::Impl::getStartEndCharacters() const {
    return
        css::uno::Reference< css::container::XNameContainer >(
            (propertySet_->getPropertyValue(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("StartEndCharacters"))).
             get< css::uno::Reference< css::container::XNameContainer > >()),
            css::uno::UNO_SET_THROW);
}

SvxAsianConfig::SvxAsianConfig(): impl_(new Impl) {}

SvxAsianConfig::~SvxAsianConfig() {}

void SvxAsianConfig::Commit() {
    css::uno::Reference< css::util::XChangesBatch >(
        impl_->getPropertySet(), css::uno::UNO_QUERY_THROW)->commitChanges();
}

bool SvxAsianConfig::IsKerningWesternTextOnly() const {
    return
        impl_->getPropertySet()->getPropertyValue(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("IsKerningWesternTextOnly"))).
        get< bool >();
}

void SvxAsianConfig::SetKerningWesternTextOnly(bool value) {
    impl_->getPropertySet()->setPropertyValue(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsKerningWesternTextOnly")),
        css::uno::makeAny(value));
}

sal_Int16 SvxAsianConfig::GetCharDistanceCompression() const {
    return
        impl_->getPropertySet()->getPropertyValue(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("CompressCharacterDistance"))).
        get< sal_Int16 >();
}

void SvxAsianConfig::SetCharDistanceCompression(sal_Int16 value) {
    assert(value >= 0 && value <= 2);
    impl_->getPropertySet()->setPropertyValue(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CompressCharacterDistance")),
        css::uno::makeAny(value));
}

css::uno::Sequence< css::lang::Locale > SvxAsianConfig::GetStartEndCharLocales()
    const
{
    css::uno::Sequence< rtl::OUString > ns(
        impl_->getStartEndCharacters()->getElementNames());
    css::uno::Sequence< css::lang::Locale > ls(ns.getLength());
    for (sal_Int32 i = 0; i < ns.getLength(); ++i) {
        sal_Int32 n = 0;
        ls[i].Language = ns[i].getToken(0, '-', n);
        ls[i].Country = ns[i].getToken(0, '-', n);
        ls[i].Variant = ns[i].getToken(0, '-', n);
    }
    return ls;
}

bool SvxAsianConfig::GetStartEndChars(
    css::lang::Locale const & locale, rtl::OUString & startChars,
    rtl::OUString & endChars) const
{
    css::uno::Reference< css::container::XNameContainer > set(
        impl_->getStartEndCharacters());
    css::uno::Any v;
    try {
        v = set->getByName(toString(locale));
    } catch (css::container::NoSuchElementException &) {
        return false;
    }
    css::uno::Reference< css::beans::XPropertySet > el(
        v.get< css::uno::Reference< css::beans::XPropertySet > >(),
        css::uno::UNO_SET_THROW);
    startChars =
        el->getPropertyValue(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("StartCharacters"))).
        get< rtl::OUString >();
    endChars =
        el->getPropertyValue(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("EndCharacters"))).
        get< rtl::OUString >();
    return true;
}

void SvxAsianConfig::SetStartEndChars(
    css::lang::Locale const & locale, rtl::OUString const * startChars,
    rtl::OUString const * endChars)
{
    assert((startChars == 0) == (endChars == 0));
    css::uno::Reference< css::container::XNameContainer > set(
        impl_->getStartEndCharacters());
    rtl::OUString name(toString(locale));
    if (startChars == 0) {
        try {
            set->removeByName(name);
        } catch (css::container::NoSuchElementException &) {}
    } else {
        bool found;
        css::uno::Any v;
        try {
            v = set->getByName(name);
            found = true;
        } catch (css::container::NoSuchElementException &) {
            found = false;
        }
        if (found) {
            css::uno::Reference< css::beans::XPropertySet > el(
                v.get< css::uno::Reference< css::beans::XPropertySet > >(),
                css::uno::UNO_SET_THROW);
            el->setPropertyValue(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("StartCharacters")),
                css::uno::makeAny(*startChars));
            el->setPropertyValue(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("EndCharacters")),
                css::uno::makeAny(*endChars));
        } else {
            css::uno::Reference< css::beans::XPropertySet > el(
                (css::uno::Reference< css::lang::XSingleServiceFactory >(
                    set, css::uno::UNO_QUERY_THROW)->
                 createInstance()),
                css::uno::UNO_QUERY_THROW);
            el->setPropertyValue(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("StartCharacters")),
                css::uno::makeAny(*startChars));
            el->setPropertyValue(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("EndCharacters")),
                css::uno::makeAny(*endChars));
            css::uno::Any v2(css::uno::makeAny(el));
            try {
                set->insertByName(name, v2);
            } catch (css::container::ElementExistException &) {
                SAL_INFO("svl", "Concurrent update race for \"" << name << '"');
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
