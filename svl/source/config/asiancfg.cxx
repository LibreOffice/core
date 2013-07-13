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
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/container/ElementExistException.hpp"
#include "com/sun/star/container/NoSuchElementException.hpp"
#include "com/sun/star/container/XNameAccess.hpp"
#include "com/sun/star/container/XNameContainer.hpp"
#include "com/sun/star/lang/Locale.hpp"
#include "com/sun/star/lang/XSingleServiceFactory.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/Sequence.hxx"
#include "comphelper/configuration.hxx"
#include "comphelper/processfactory.hxx"
#include "officecfg/Office/Common.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/log.hxx"
#include "sal/types.h"
#include "i18nlangtag/languagetag.hxx"
#include "svl/asiancfg.hxx"

namespace {

OUString toString(css::lang::Locale const & locale) {
    SAL_WARN_IF( locale.Language.indexOf('-') != -1, "svl",
            "Locale language \"" << locale.Language << "\" contains \"-\"");
    SAL_WARN_IF( locale.Country.indexOf('-') != -1, "svl",
            "Locale country \"" << locale.Country << "\" contains \"-\"");
    return LanguageTag::convertToBcp47( locale, false);
}

}

struct SvxAsianConfig::Impl: private boost::noncopyable {
    Impl():
        context(comphelper::getProcessComponentContext()),
        batch(comphelper::ConfigurationChanges::create(context))
    {}

    css::uno::Reference< css::uno::XComponentContext > context;

    boost::shared_ptr< comphelper::ConfigurationChanges > batch;
};

SvxAsianConfig::SvxAsianConfig(): impl_(new Impl) {}

SvxAsianConfig::~SvxAsianConfig() {}

void SvxAsianConfig::Commit() {
    impl_->batch->commit();
}

bool SvxAsianConfig::IsKerningWesternTextOnly() const {
    return
        officecfg::Office::Common::AsianLayout::IsKerningWesternTextOnly::get(
            impl_->context);
}

void SvxAsianConfig::SetKerningWesternTextOnly(bool value) {
    officecfg::Office::Common::AsianLayout::IsKerningWesternTextOnly::set(
        value, impl_->batch, impl_->context);
}

sal_Int16 SvxAsianConfig::GetCharDistanceCompression() const {
    return
        officecfg::Office::Common::AsianLayout::CompressCharacterDistance::get(
            impl_->context);
}

void SvxAsianConfig::SetCharDistanceCompression(sal_Int16 value) {
    officecfg::Office::Common::AsianLayout::CompressCharacterDistance::set(
        value, impl_->batch, impl_->context);
}

css::uno::Sequence< css::lang::Locale > SvxAsianConfig::GetStartEndCharLocales()
    const
{
    css::uno::Sequence< OUString > ns(
        officecfg::Office::Common::AsianLayout::StartEndCharacters::get(
            impl_->context)->
        getElementNames());
    css::uno::Sequence< css::lang::Locale > ls(ns.getLength());
    for (sal_Int32 i = 0; i < ns.getLength(); ++i) {
        ls[i] = LanguageTag::convertToLocale( ns[i], false);
    }
    return ls;
}

bool SvxAsianConfig::GetStartEndChars(
    css::lang::Locale const & locale, OUString & startChars,
    OUString & endChars) const
{
    css::uno::Reference< css::container::XNameAccess > set(
        officecfg::Office::Common::AsianLayout::StartEndCharacters::get(
            impl_->context));
    css::uno::Any v;
    try {
        v = set->getByName(toString(locale));
    } catch (css::container::NoSuchElementException &) {
        return false;
    }
    css::uno::Reference< css::beans::XPropertySet > el(
        v.get< css::uno::Reference< css::beans::XPropertySet > >(),
        css::uno::UNO_SET_THROW);
    startChars = el->getPropertyValue("StartCharacters").get< OUString >();
    endChars = el->getPropertyValue("EndCharacters").get< OUString >();
    return true;
}

void SvxAsianConfig::SetStartEndChars(
    css::lang::Locale const & locale, OUString const * startChars,
    OUString const * endChars)
{
    assert((startChars == 0) == (endChars == 0));
    css::uno::Reference< css::container::XNameContainer > set(
        officecfg::Office::Common::AsianLayout::StartEndCharacters::get(
            impl_->batch, impl_->context));
    OUString name(toString(locale));
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
            el->setPropertyValue("StartCharacters", css::uno::makeAny(*startChars));
            el->setPropertyValue("EndCharacters", css::uno::makeAny(*endChars));
        } else {
            css::uno::Reference< css::beans::XPropertySet > el(
                (css::uno::Reference< css::lang::XSingleServiceFactory >(
                    set, css::uno::UNO_QUERY_THROW)->
                 createInstance()),
                css::uno::UNO_QUERY_THROW);
            el->setPropertyValue("StartCharacters", css::uno::makeAny(*startChars));
            el->setPropertyValue("EndCharacters", css::uno::makeAny(*endChars));
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
