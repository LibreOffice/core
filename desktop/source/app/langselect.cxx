/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "sal/config.h"

#include "boost/shared_ptr.hpp"
#include "com/sun/star/configuration/theDefaultProvider.hpp"
#include "com/sun/star/container/XNameAccess.hpp"
#include "com/sun/star/lang/XLocalizable.hpp"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/Sequence.hxx"
#include "comphelper/configuration.hxx"
#include "comphelper/processfactory.hxx"
#include "i18nlangtag/lang.h"
#include "i18nlangtag/languagetag.hxx"
#include "i18nlangtag/mslangid.hxx"
#include "officecfg/Office/Linguistic.hxx"
#include "officecfg/Setup.hxx"
#include "officecfg/System.hxx"
#include "rtl/ustring.hxx"
#include "sal/log.hxx"
#include "sal/types.h"
#include "svl/languageoptions.hxx"

#include "app.hxx"

#include "cmdlineargs.hxx"
#include "langselect.hxx"

namespace desktop { namespace langselect {

namespace {

OUString foundLocale;

OUString getInstalledLocale(
    css::uno::Sequence<OUString> const & installed, OUString const & locale)
{
    if (locale.isEmpty())
        return OUString();  

    for (sal_Int32 i = 0; i != installed.getLength(); ++i) {
        if (installed[i] == locale) {
            return installed[i];
        }
    }
    ::std::vector<OUString> fallbacks( LanguageTag( locale).getFallbackStrings( false));
    for (size_t f=0; f < fallbacks.size(); ++f) {
        const OUString& rf = fallbacks[f];
        for (sal_Int32 i = 0; i != installed.getLength(); ++i) {
            if (installed[i] == rf) {
                return installed[i];
            }
        }
    }
    return OUString();
}

void setMsLangIdFallback(OUString const & locale) {
    
    
    if (!locale.isEmpty()) {
        LanguageType type = LanguageTag::convertToLanguageTypeWithFallback(locale);
        switch (SvtLanguageOptions::GetScriptTypeOfLanguage(type)) {
        case SCRIPTTYPE_ASIAN:
            MsLangId::setConfiguredAsianFallback(type);
            break;
        case SCRIPTTYPE_COMPLEX:
            MsLangId::setConfiguredComplexFallback(type);
            break;
        default:
            MsLangId::setConfiguredWesternFallback(type);
            break;
        }
    }
}

}

OUString getEmergencyLocale() {
    if (!foundLocale.isEmpty()) {
        return foundLocale;
    }
    try {
        css::uno::Sequence<OUString> inst(
            officecfg::Setup::Office::InstalledLocales::get()->
            getElementNames());
        OUString locale(
            getInstalledLocale(
                inst,
                officecfg::Office::Linguistic::General::UILocale::get()));
        if (!locale.isEmpty()) {
            return locale;
        }
        locale = getInstalledLocale(
            inst, officecfg::System::L10N::UILocale::get());
        if (!locale.isEmpty()) {
            return locale;
        }
        locale = getInstalledLocale(inst, "en-US");
        if (!locale.isEmpty()) {
            return locale;
        }
        if (inst.hasElements()) {
            return inst[0];
        }
    } catch (css::uno::Exception & e) {
        SAL_WARN("desktop.app", "ignoring Exception \"" << e.Message << "\"");
    }
    return OUString();
}

bool prepareLocale() {
    
    
    setMsLangIdFallback(officecfg::System::L10N::SystemLocale::get());
    
    setMsLangIdFallback(officecfg::System::L10N::Locale::get());
    css::uno::Sequence<OUString> inst(
        officecfg::Setup::Office::InstalledLocales::get()->getElementNames());
    OUString locale(officecfg::Office::Linguistic::General::UILocale::get());
    if (!locale.isEmpty()) {
        locale = getInstalledLocale(inst, locale);
        if (locale.isEmpty()) {
            
            try {
                boost::shared_ptr<comphelper::ConfigurationChanges> batch(
                    comphelper::ConfigurationChanges::create());
                officecfg::Office::Linguistic::General::UILocale::set(
                    "", batch);
                batch->commit();
            } catch (css::uno::Exception & e) {
                SAL_WARN(
                    "desktop.app",
                    "ignoring Exception \"" << e.Message << "\"");
            }
        }
    }
    bool cmdLanguage = false;
    if (locale.isEmpty()) {
        locale = getInstalledLocale(
            inst, Desktop::GetCommandLineArgs().GetLanguage());
        if (!locale.isEmpty()) {
            cmdLanguage = true;
        }
    }
    if (locale.isEmpty()) {
        locale = getInstalledLocale(
            inst, officecfg::System::L10N::UILocale::get());
    }
    if (locale.isEmpty()) {
        locale = getInstalledLocale(inst, "en-US");
    }
    if (locale.isEmpty() && inst.hasElements()) {
        locale = inst[0];
    }
    if (locale.isEmpty()) {
        return false;
    }
    LanguageTag tag(locale);
    
    
    
    css::uno::Reference<css::lang::XLocalizable>(
        com::sun::star::configuration::theDefaultProvider::get(
            comphelper::getProcessComponentContext()),
        css::uno::UNO_QUERY_THROW)->setLocale(tag.getLocale(false));
    if (!cmdLanguage) {
        try {
            boost::shared_ptr<comphelper::ConfigurationChanges> batch(
                comphelper::ConfigurationChanges::create());
            officecfg::Setup::L10N::ooLocale::set(locale, batch);
            batch->commit();
        } catch (css::uno::Exception & e) {
            SAL_WARN(
                "desktop.app", "ignoring Exception \"" << e.Message << "\"");
        }
    }
    MsLangId::setConfiguredSystemUILanguage(tag.getLanguageType(false));
    OUString setupSysLoc(officecfg::Setup::L10N::ooSetupSystemLocale::get());
    LanguageTag::setConfiguredSystemLanguage(
        setupSysLoc.isEmpty()
        ? MsLangId::getSystemLanguage()
        : LanguageTag(setupSysLoc).getLanguageType(false));
    
    
    setMsLangIdFallback(locale);
    foundLocale = locale;
    return true;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
