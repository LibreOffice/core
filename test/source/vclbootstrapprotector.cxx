/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <boost/noncopyable.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/util/XFlushable.hpp>
#include <comphelper/processfactory.hxx>
#include <cppunit/Protector.h>
#include <cppunittester/protectorfactory.hxx>
#include <i18nlangtag/lang.h>
#include <i18nlangtag/languagetag.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <sal/types.h>
#include <tools/resmgr.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <vcl/svapp.hxx>

#include <isheadless.hxx>

namespace {

class Protector: public CppUnit::Protector, private boost::noncopyable {
public:
    Protector() {
        // Force locale (and resource files loaded) to en-US:
        ResMgr::SetDefaultLocale(LanguageTag("en-US"));
        SvtSysLocaleOptions localOptions;
        localOptions.SetLocaleConfigString("en-US");
        localOptions.SetUILocaleConfigString("en-US");
        MsLangId::setConfiguredSystemUILanguage(LANGUAGE_ENGLISH_US);
        LanguageTag::setConfiguredSystemLanguage(LANGUAGE_ENGLISH_US);
        InitVCL();
        if (test::isHeadless()) {
            Application::EnableHeadlessMode(true);
        }
        Application::setDeInitHook(LINK(this, Protector, deinitHook));
    }

private:
    virtual ~Protector() {
        DeInitVCL();
    }

    virtual bool protect(
        CppUnit::Functor const & functor, CppUnit::ProtectorContext const &)
        override
    { return functor(); }

    DECL_STATIC_LINK_TYPED(Protector, deinitHook, LinkParamNone*, void);
};

// HACK so that defaultBootstrap_InitialComponentContext (in
// unobootstrapprotector) is called before InitVCL (above), but component
// context is disposed (redundantly again in unobootstrapprotector) from within
// DeInitVCL (cf. Desktop::DeInit, desktop/source/app/app.cxx):
IMPL_STATIC_LINK_NOARG_TYPED(Protector, deinitHook, LinkParamNone*, void)
{
    css::uno::Reference<css::uno::XComponentContext> context;
    try {
        context = comphelper::getProcessComponentContext();
    } catch (css::uno::RuntimeException &) {}
    if (context.is()) {
        css::uno::Reference<css::lang::XMultiServiceFactory> config;
        try {
            config = css::configuration::theDefaultProvider::get(context);
        } catch (css::uno::DeploymentException &) {}
        if (config.is()) {
            utl::ConfigManager::storeConfigItems();
            css::uno::Reference<css::util::XFlushable>(
                config, css::uno::UNO_QUERY_THROW)->flush();
        }
        css::uno::Reference<css::lang::XComponent>(
            context, css::uno::UNO_QUERY_THROW)->dispose();
        comphelper::setProcessServiceFactory(0);
    }
}

}

extern "C" SAL_DLLPUBLIC_EXPORT CppUnit::Protector * SAL_CALL
vclbootstrapprotector() {
    return new Protector;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
