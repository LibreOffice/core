/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <comphelper/interfacecontainer3.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceDisplayName.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/linguistic2/XSpellChecker.hpp>
#include <com/sun/star/linguistic2/XLinguServiceEventBroadcaster.hpp>

#include <linguistic/lngprophelp.hxx>
#include <systools/win32/comtools.hxx>

#include <prewin.h>
#include <spellcheck.h>
#include <postwin.h>

#include <map>
#include <memory>
#include <optional>
#include <vector>

// Spell checker backed by the Windows Spell Checking API. It claims only the locales that
// Windows supports and no installed hunspell dictionary covers, so for any one locale either
// this service or the hunspell one answers, never both.
class WinSpellChecker
    : public cppu::WeakImplHelper<css::linguistic2::XSpellChecker,
                                  css::linguistic2::XLinguServiceEventBroadcaster,
                                  css::lang::XInitialization, css::lang::XComponent,
                                  css::lang::XServiceInfo, css::lang::XServiceDisplayName>
{
    // One claimed locale and the language tag Windows knows it by.
    struct LocaleTag
    {
        css::lang::Locale m_aLocale;
        OUString m_aTag;
    };

    // Keeps the COM apartment of the main thread, the one every call comes in on, alive for
    // as long as the factory and the checkers created from it are held.
    std::optional<sal::systools::CoInitializeGuard> m_oCoInit;
    sal::systools::COMReference<ISpellCheckerFactory> m_xFactory;
    // The checker of each tag, created on first use. An entry holding nothing records that
    // Windows failed to create a checker for that tag, so it is not asked again for every word.
    std::map<OUString, sal::systools::COMReference<ISpellChecker>> m_aCheckers;

    bool m_bLocalesRead;
    std::vector<LocaleTag> m_aLocaleTags;
    cpo::uno::Sequence<css::lang::Locale> m_aSuppLocales;

    ::comphelper::OInterfaceContainerHelper3<css::lang::XEventListener> m_aEvtListeners;
    std::unique_ptr<linguistic::PropertyHelper_Spelling> m_pPropHelper;
    bool m_bDisposing;

    WinSpellChecker(const WinSpellChecker&) = delete;
    WinSpellChecker& operator=(const WinSpellChecker&) = delete;

    linguistic::PropertyHelper_Spelling& GetPropHelper_Impl();
    linguistic::PropertyHelper_Spelling& GetPropHelper()
    {
        return m_pPropHelper ? *m_pPropHelper : GetPropHelper_Impl();
    }

    void ReadLocales();
    const OUString* FindTag(const css::lang::Locale& rLocale) const;
    ISpellChecker* GetChecker(const css::lang::Locale& rLocale);

    sal_Int16 GetSpellFailure(const OUString& rWord, const css::lang::Locale& rLocale);
    sal_Int16 CheckWord(const OUString& rWord, const css::lang::Locale& rLocale,
                        const cpo::uno::Sequence<css::beans::PropertyValue>& rProperties);
    cpo::uno::Reference<css::linguistic2::XSpellAlternatives>
    GetProposals(const OUString& rWord, const css::lang::Locale& rLocale, sal_Int16 nFailure);

public:
    WinSpellChecker();
    ~WinSpellChecker() override;

    // XSupportedLocales (for XSpellChecker)
    cpo::uno::Sequence<css::lang::Locale> getLocales() override;
    bool hasLocale(const css::lang::Locale& rLocale) override;

    // XSpellChecker
    bool isValid(const OUString& rWord, const css::lang::Locale& rLocale,
                 const cpo::uno::Sequence<css::beans::PropertyValue>& rProperties) override;
    cpo::uno::Reference<css::linguistic2::XSpellAlternatives>
    spell(const OUString& rWord, const css::lang::Locale& rLocale,
          const cpo::uno::Sequence<css::beans::PropertyValue>& rProperties) override;

    // XLinguServiceEventBroadcaster
    bool addLinguServiceEventListener(
        const cpo::uno::Reference<css::linguistic2::XLinguServiceEventListener>& rxLstnr) override;
    bool removeLinguServiceEventListener(
        const cpo::uno::Reference<css::linguistic2::XLinguServiceEventListener>& rxLstnr) override;

    // XServiceDisplayName
    OUString getServiceDisplayName(const css::lang::Locale& rLocale) override;

    // XInitialization
    void initialize(const cpo::uno::Sequence<cpo::uno::Any>& rArguments) override;

    // XComponent
    void dispose() override;
    void
    addEventListener(const cpo::uno::Reference<css::lang::XEventListener>& rxListener) override;
    void
    removeEventListener(const cpo::uno::Reference<css::lang::XEventListener>& rxListener) override;

    // XServiceInfo
    OUString getImplementationName() override;
    bool supportsService(const OUString& rServiceName) override;
    cpo::uno::Sequence<OUString> getSupportedServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
