/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/util/XFlushable.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <comphelper/processfactory.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <o3tl/environment.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <vcl/svapp.hxx>

#include "isheadless.hxx"
#include "setupvcl.hxx"

namespace {

struct Hook { // LINK only works as a member of a class...
    DECL_STATIC_LINK(Hook, deinitHook, LinkParamNone *, void);
};

// HACK so that defaultBootstrap_InitialComponentContext (in
// unobootstrapprotector) is called before InitVCL (below), but component
// context is disposed (redundantly again in unobootstrapprotector) from within
// DeInitVCL (cf. Desktop::DeInit, desktop/source/app/app.cxx):
IMPL_STATIC_LINK_NOARG(Hook, deinitHook, LinkParamNone *, void) {
    css::uno::Reference<css::uno::XComponentContext> context;
    try {
        context = comphelper::getProcessComponentContext();
    } catch (css::uno::RuntimeException &) {}

    if (!context)
        return;

    css::uno::Reference<css::lang::XMultiServiceFactory> config;
    try {
        config = css::configuration::theDefaultProvider::get(context);
    } catch (css::uno::DeploymentException &) {}
    if (config) {
        utl::ConfigManager::storeConfigItems();
        css::uno::Reference<css::util::XFlushable>(
            config, css::uno::UNO_QUERY_THROW)->flush();
    }

    // the desktop has to be terminate() before it can be dispose()
    css::uno::Reference<css::frame::XDesktop> xDesktop;
    try {
        xDesktop = css::frame::Desktop::create(comphelper::getProcessComponentContext());
    } catch (css::uno::DeploymentException &) {}
    if (xDesktop)
        try {
            xDesktop->terminate();
        } catch (css::uno::DeploymentException &) {}

    css::uno::Reference<css::lang::XComponent>(
        context, css::uno::UNO_QUERY_THROW)->dispose();

    comphelper::setProcessServiceFactory(nullptr);
}

}

void test::setUpVcl(bool const forceHeadless) {
    // Force locale (and resource files loaded):
    OUString locale = o3tl::getEnvironment(u"LO_TEST_LOCALE"_ustr);
    if (locale.isEmpty())
        locale = "en-US";

    SvtSysLocaleOptions localOptions;
    localOptions.SetLocaleConfigString(locale);
    localOptions.SetUILocaleConfigString(locale);
    LanguageTag tag(locale);
    MsLangId::setConfiguredSystemUILanguage(tag.getLanguageType(false));
    LanguageTag::setConfiguredSystemLanguage(tag.getLanguageType(false));
    InitVCL();
    if (forceHeadless || isHeadless()) {
        Application::EnableHeadlessMode(false);
    }
    Application::setDeInitHook(LINK(nullptr, Hook, deinitHook));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
