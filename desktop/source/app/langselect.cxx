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

#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XLocalizable.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <comphelper/configuration.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/logging.hxx>
#include <i18nlangtag/lang.h>
#include <i18nlangtag/languagetag.hxx>
#include <i18nlangtag/mslangid.hxx>
#include "officecfg/Office/Linguistic.hxx"
#include "officecfg/Setup.hxx"
#include "officecfg/System.hxx"
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <sal/types.h>
#include <svl/languageoptions.hxx>

#include "app.hxx"

#include "cmdlineargs.hxx"
#include "langselect.hxx"

namespace desktop { namespace langselect {

namespace {

OUString foundLocale;

void setMsLangIdFallback(OUString const & locale) {
    // #i32939# setting of default document language
    // See #i42730# for rules for determining source of settings
    if (!locale.isEmpty()) {
        LanguageType type = LanguageTag::convertToLanguageTypeWithFallback(locale);
        switch (SvtLanguageOptions::GetScriptTypeOfLanguage(type)) {
        case SvtScriptType::ASIAN:
            MsLangId::setConfiguredAsianFallback(type);
            break;
        case SvtScriptType::COMPLEX:
            MsLangId::setConfiguredComplexFallback(type);
            break;
        default:
            MsLangId::setConfiguredWesternFallback(type);
            break;
        }
    }
}

}

bool prepareLocale() {
    // #i42730# Get the windows 16Bit locale, it should be preferred over the UI
    // locale:
    setMsLangIdFallback(officecfg::System::L10N::SystemLocale::get());
    // #i32939# Use system locale to set document default locale:
    setMsLangIdFallback(officecfg::System::L10N::Locale::get());
    css::uno::Sequence<OUString> inst(
        officecfg::Setup::Office::InstalledLocales::get()->getElementNames());
    OUString locale(officecfg::Office::Linguistic::General::UILocale::get());
    if (!locale.isEmpty()) {
        locale = getInstalledLocaleForLanguage(inst, locale);
        if (locale.isEmpty()) {
            // Selected language is not/no longer installed:
            try {
                std::shared_ptr<comphelper::ConfigurationChanges> batch(
                    comphelper::ConfigurationChanges::create());
                officecfg::Office::Linguistic::General::UILocale::set(
                    "", batch);
                batch->commit();
            } catch (css::uno::Exception & e) {
                SAL_WARN("desktop.app", "ignoring " << e);
            }
        }
    }
    bool cmdLanguage = false;
    if (locale.isEmpty()) {
        locale = getInstalledLocaleForLanguage(
            inst, Desktop::GetCommandLineArgs().GetLanguage());
        if (!locale.isEmpty()) {
            cmdLanguage = true;
        }
    }
    if (locale.isEmpty()) {
        locale = getInstalledLocaleForSystemUILanguage(inst);
    }
    if (locale.isEmpty()) {
        return false;
    }
    LanguageTag tag(locale);
    // Prepare default config provider by localizing it to the selected
    // locale this will ensure localized configuration settings to be
    // selected according to the UI language:
    css::uno::Reference<css::lang::XLocalizable>(
        css::configuration::theDefaultProvider::get(
            comphelper::getProcessComponentContext()),
        css::uno::UNO_QUERY_THROW)->setLocale(tag.getLocale(false));
    if (!cmdLanguage) {
        try {
            std::shared_ptr<comphelper::ConfigurationChanges> batch(
                comphelper::ConfigurationChanges::create());
            officecfg::Setup::L10N::ooLocale::set(locale, batch);
            batch->commit();
        } catch (css::uno::Exception & e) {
            SAL_WARN("desktop.app", "ignoring " << e);
        }
    }
    MsLangId::setConfiguredSystemUILanguage(tag.getLanguageType(false));

    OUString setupSysLoc(officecfg::Setup::L10N::ooSetupSystemLocale::get());
    LanguageTag::setConfiguredSystemLanguage(
        setupSysLoc.isEmpty()
        ? MsLangId::getSystemLanguage()
        : LanguageTag(setupSysLoc).getLanguageType(false));
    // #i32939# setting of default document locale
    // #i32939# this should not be based on the UI language
    // So obtain the system locale now configured just above and pass it on,
    // resolved of course.
    LanguageTag docTag(LANGUAGE_SYSTEM);
    setMsLangIdFallback(docTag.getBcp47());

    foundLocale = locale;
    return true;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
