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
    struct DictItem
    {
        OUString                  m_aDName;
        css::lang::Locale         m_aDLoc;
        std::unique_ptr<Hunspell> m_pDict;
        rtl_TextEncoding          m_aDEnc;

        DictItem(OUString i_DName, css::lang::Locale i_DLoc, rtl_TextEncoding i_DEnc);
    };

    std::vector<DictItem> m_DictItems;

    css::uno::Sequence< css::lang::Locale >                 m_aSuppLocales;

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
    css::uno::Reference< css::linguistic2::XSpellAlternatives > GetProposals( const OUString &rWord, const css::lang::Locale &rLocale );

public:
    SpellChecker();
    virtual ~SpellChecker() override;

    // XSupportedLocales (for XSpellChecker)
    virtual css::uno::Sequence< css::lang::Locale > SAL_CALL getLocales() override;
    virtual bool SAL_CALL hasLocale( const css::lang::Locale& rLocale ) override;

    // XSpellChecker
    virtual bool SAL_CALL isValid( const OUString& rWord, const css::lang::Locale& rLocale, const css::uno::Sequence< css::beans::PropertyValue >& rProperties ) override;
    virtual css::uno::Reference< css::linguistic2::XSpellAlternatives > SAL_CALL spell( const OUString& rWord, const css::lang::Locale& rLocale, const css::uno::Sequence< css::beans::PropertyValue >& rProperties ) override;

    // XLinguServiceEventBroadcaster
    virtual bool SAL_CALL addLinguServiceEventListener( const css::uno::Reference< css::linguistic2::XLinguServiceEventListener >& rxLstnr ) override;
    virtual bool SAL_CALL removeLinguServiceEventListener( const css::uno::Reference< css::linguistic2::XLinguServiceEventListener >& rxLstnr ) override;

    // XServiceDisplayName
    virtual OUString SAL_CALL getServiceDisplayName( const css::lang::Locale& rLocale ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< cpo::uno::Any >& rArguments ) override;

    // XComponent
    virtual void SAL_CALL dispose() override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& rxListener ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& rxListener ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual bool SAL_CALL supportsService( const OUString& rServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
