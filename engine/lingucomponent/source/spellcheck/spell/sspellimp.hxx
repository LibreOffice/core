/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#ifndef INCLUDED_LINGUCOMPONENT_SOURCE_SPELLCHECK_SPELL_SSPELLIMP_HXX
#define INCLUDED_LINGUCOMPONENT_SOURCE_SPELLCHECK_SPELL_SSPELLIMP_HXX

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

#include <memory>

#include <hunspell.hxx>

class SpellChecker :
    public cppu::WeakImplHelper
    <
        css::linguistic2::XSpellChecker,
        css::linguistic2::XLinguServiceEventBroadcaster,
        css::lang::XInitialization,
        css::lang::XComponent,
        css::lang::XServiceInfo,
        css::lang::XServiceDisplayName
    >
{

    // The word list of one dictionary: the path of its .aff/.dic file pair without the
    // extension, the Hunspell instance loaded from those files, and the text encoding that
    // instance expects. m_pDict stays null until Load has run.
    struct DictInstance
    {
        OUString m_aDName;
        std::unique_ptr<Hunspell> m_pDict;
        rtl_TextEncoding m_aDEnc = RTL_TEXTENCODING_DONTKNOW;

        explicit DictInstance(OUString const& i_rDName)
            : m_aDName(i_rDName)
        {}

        void Load();
    };

    // One entry per locale. A dictionary that names several locales gets one DictItem for each
    // of them, and those items point to the same DictInstance.
    struct DictItem
    {
        css::lang::Locale m_aDLoc;
        std::shared_ptr<DictInstance> m_pInstance;

        DictItem(css::lang::Locale const& i_rDLocale, std::shared_ptr<DictInstance> const& i_pInstance)
            : m_aDLoc(i_rDLocale)
            , m_pInstance(i_pInstance)
        {}
    };

    std::vector<DictItem> m_DictItems;

    cpo::uno::Sequence< css::lang::Locale >                 m_aSuppLocales;

    ::comphelper::OInterfaceContainerHelper3<css::lang::XEventListener> m_aEvtListeners;
    std::unique_ptr<linguistic::PropertyHelper_Spelling> m_pPropHelper;
    bool                                    m_bDisposing;

    SpellChecker(const SpellChecker &) = delete;
    SpellChecker & operator = (const SpellChecker &) = delete;

    linguistic::PropertyHelper_Spelling&  GetPropHelper_Impl();
    linguistic::PropertyHelper_Spelling&  GetPropHelper()
    {
        return m_pPropHelper ? *m_pPropHelper : GetPropHelper_Impl();
    }

    sal_Int16   GetSpellFailure( const OUString &rWord, const css::lang::Locale &rLocale, int& rInfo );
    cpo::uno::Reference< css::linguistic2::XSpellAlternatives > GetProposals( const OUString &rWord, const css::lang::Locale &rLocale );

public:
    SpellChecker();
    virtual ~SpellChecker() override;

    // XSupportedLocales (for XSpellChecker)
    virtual cpo::uno::Sequence< css::lang::Locale > getLocales() override;
    virtual bool hasLocale( const css::lang::Locale& rLocale ) override;

    // XSpellChecker
    virtual bool isValid( const OUString& rWord, const css::lang::Locale& rLocale, const cpo::uno::Sequence< css::beans::PropertyValue >& rProperties ) override;
    virtual cpo::uno::Reference< css::linguistic2::XSpellAlternatives > spell( const OUString& rWord, const css::lang::Locale& rLocale, const cpo::uno::Sequence< css::beans::PropertyValue >& rProperties ) override;

    // XLinguServiceEventBroadcaster
    virtual bool addLinguServiceEventListener( const cpo::uno::Reference< css::linguistic2::XLinguServiceEventListener >& rxLstnr ) override;
    virtual bool removeLinguServiceEventListener( const cpo::uno::Reference< css::linguistic2::XLinguServiceEventListener >& rxLstnr ) override;

    // XServiceDisplayName
    virtual OUString getServiceDisplayName( const css::lang::Locale& rLocale ) override;

    // XInitialization
    virtual void initialize( const cpo::uno::Sequence< cpo::uno::Any >& rArguments ) override;

    // XComponent
    virtual void dispose() override;
    virtual void addEventListener( const cpo::uno::Reference< css::lang::XEventListener >& rxListener ) override;
    virtual void removeEventListener( const cpo::uno::Reference< css::lang::XEventListener >& rxListener ) override;

    // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& rServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
