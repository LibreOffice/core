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

#include <cassert>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/ElementExistException.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <comphelper/configuration.hxx>
#include <comphelper/processfactory.hxx>
#include <officecfg/Office/Common.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <sal/types.h>
#include <i18nlangtag/languagetag.hxx>
#include <svl/asiancfg.hxx>

namespace {

OUString toString(css::lang::Locale const & locale) {
    SAL_WARN_IF( locale.Language.indexOf('-') != -1, "svl",
            "Locale language \"" << locale.Language << "\" contains \"-\"");
    SAL_WARN_IF( locale.Country.indexOf('-') != -1, "svl",
            "Locale country \"" << locale.Country << "\" contains \"-\"");
    return LanguageTag::convertToBcp47( locale, false);
}

}

struct SvxAsianConfig::Impl {
    Impl():
        batch(comphelper::ConfigurationChanges::create())
    {}

    Impl(const Impl&) = delete;
    Impl& operator=(const Impl&) = delete;

    std::shared_ptr< comphelper::ConfigurationChanges > batch;
};

SvxAsianConfig::SvxAsianConfig(): impl_(new Impl) {}

SvxAsianConfig::~SvxAsianConfig() {}

void SvxAsianConfig::Commit() {
    impl_->batch->commit();
}

// static
bool SvxAsianConfig::IsKerningWesternTextOnly() {
    return
        officecfg::Office::Common::AsianLayout::IsKerningWesternTextOnly::get();
}

void SvxAsianConfig::SetKerningWesternTextOnly(bool value) {
    officecfg::Office::Common::AsianLayout::IsKerningWesternTextOnly::set(
        value, impl_->batch);
}

// static
CharCompressType SvxAsianConfig::GetCharDistanceCompression() {
    return static_cast<CharCompressType>(officecfg::Office::Common::AsianLayout::CompressCharacterDistance::get());
}

void SvxAsianConfig::SetCharDistanceCompression(CharCompressType value) {
    officecfg::Office::Common::AsianLayout::CompressCharacterDistance::set(
        static_cast<sal_uInt16>(value), impl_->batch);
}

// static
css::uno::Sequence< css::lang::Locale > SvxAsianConfig::GetStartEndCharLocales()
{
    const css::uno::Sequence< OUString > ns(
        officecfg::Office::Common::AsianLayout::StartEndCharacters::get()->
        getElementNames());
    css::uno::Sequence< css::lang::Locale > ls(ns.getLength());
    std::transform(ns.begin(), ns.end(), ls.getArray(),
        [](const OUString& rName) -> css::lang::Locale {
            return LanguageTag::convertToLocale( rName, false); });
    return ls;
}

// static
bool SvxAsianConfig::GetStartEndChars(
    css::lang::Locale const & locale, OUString & startChars,
    OUString & endChars)
{
    css::uno::Reference< css::container::XNameAccess > set(
        officecfg::Office::Common::AsianLayout::StartEndCharacters::get());
    css::uno::Any v;
    try {
        v = set->getByName(toString(locale));
    } catch (css::container::NoSuchElementException &) {
        return false;
    }
    css::uno::Reference< css::beans::XPropertySet > el(
        v.get< css::uno::Reference< css::beans::XPropertySet > >(),
        css::uno::UNO_SET_THROW);
    startChars = el->getPropertyValue(u"StartCharacters"_ustr).get< OUString >();
    endChars = el->getPropertyValue(u"EndCharacters"_ustr).get< OUString >();
    return true;
}

void SvxAsianConfig::SetStartEndChars(
    css::lang::Locale const & locale, OUString const * startChars,
    OUString const * endChars)
{
    assert((startChars == nullptr) == (endChars == nullptr));
    css::uno::Reference< css::container::XNameContainer > set(
        officecfg::Office::Common::AsianLayout::StartEndCharacters::get(
            impl_->batch));
    OUString name(toString(locale));
    if (startChars == nullptr) {
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
            el->setPropertyValue(u"StartCharacters"_ustr, css::uno::Any(*startChars));
            el->setPropertyValue(u"EndCharacters"_ustr, css::uno::Any(*endChars));
        } else {
            css::uno::Reference< css::beans::XPropertySet > el(
                (css::uno::Reference< css::lang::XSingleServiceFactory >(
                    set, css::uno::UNO_QUERY_THROW)->
                 createInstance()),
                css::uno::UNO_QUERY_THROW);
            el->setPropertyValue(u"StartCharacters"_ustr, css::uno::Any(*startChars));
            el->setPropertyValue(u"EndCharacters"_ustr, css::uno::Any(*endChars));
            css::uno::Any v2(el);
            try {
                set->insertByName(name, v2);
            } catch (css::container::ElementExistException &) {
                SAL_INFO("svl", "Concurrent update race for \"" << name << '"');
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
