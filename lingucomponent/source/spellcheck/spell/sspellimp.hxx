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

#ifndef INCLUDED_LINGUCOMPONENT_SOURCE_SPELLCHECK_SPELL_SSPELLIMP_HXX
#define INCLUDED_LINGUCOMPONENT_SOURCE_SPELLCHECK_SPELL_SSPELLIMP_HXX

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceDisplayName.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/linguistic2/XSpellChecker.hpp>
#include <com/sun/star/linguistic2/XLinguServiceEventBroadcaster.hpp>

#include <linguistic/misc.hxx>
#include <linguistic/lngprophelp.hxx>

#include <lingutil.hxx>
#include <memory>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;

class Hunspell;

class SpellChecker :
    public cppu::WeakImplHelper
    <
        XSpellChecker,
        XLinguServiceEventBroadcaster,
        XInitialization,
        XComponent,
        XServiceInfo,
        XServiceDisplayName
    >
{
    struct DictItem
    {
        OUString                  m_aDName;
        Locale                    m_aDLoc;
        std::unique_ptr<Hunspell> m_pDict;
        rtl_TextEncoding          m_aDEnc;

        DictItem(OUString i_DName, Locale i_DLoc, rtl_TextEncoding i_DEnc);
    };

    std::vector<DictItem> m_DictItems;

    Sequence< Locale >                 m_aSuppLocales;

    ::comphelper::OInterfaceContainerHelper2       m_aEvtListeners;
    std::unique_ptr<linguistic::PropertyHelper_Spelling> m_pPropHelper;
    bool                                    m_bDisposing;

    SpellChecker(const SpellChecker &) = delete;
    SpellChecker & operator = (const SpellChecker &) = delete;

    linguistic::PropertyHelper_Spelling&  GetPropHelper_Impl();
    linguistic::PropertyHelper_Spelling&  GetPropHelper()
    {
        return m_pPropHelper ? *m_pPropHelper : GetPropHelper_Impl();
    }

    sal_Int16   GetSpellFailure( const OUString &rWord, const Locale &rLocale );
    Reference< XSpellAlternatives > GetProposals( const OUString &rWord, const Locale &rLocale );

public:
    SpellChecker();
    virtual ~SpellChecker() override;

    // XSupportedLocales (for XSpellChecker)
    virtual Sequence< Locale > SAL_CALL getLocales() override;
    virtual sal_Bool SAL_CALL hasLocale( const Locale& rLocale ) override;

    // XSpellChecker
    virtual sal_Bool SAL_CALL isValid( const OUString& rWord, const Locale& rLocale, const css::uno::Sequence< css::beans::PropertyValue >& rProperties ) override;
    virtual Reference< XSpellAlternatives > SAL_CALL spell( const OUString& rWord, const Locale& rLocale, const css::uno::Sequence< css::beans::PropertyValue >& rProperties ) override;

    // XLinguServiceEventBroadcaster
    virtual sal_Bool SAL_CALL addLinguServiceEventListener( const Reference< XLinguServiceEventListener >& rxLstnr ) override;
    virtual sal_Bool SAL_CALL removeLinguServiceEventListener( const Reference< XLinguServiceEventListener >& rxLstnr ) override;

    // XServiceDisplayName
    virtual OUString SAL_CALL getServiceDisplayName( const Locale& rLocale ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& rArguments ) override;

    // XComponent
    virtual void SAL_CALL dispose() override;
    virtual void SAL_CALL addEventListener( const Reference< XEventListener >& rxListener ) override;
    virtual void SAL_CALL removeEventListener( const Reference< XEventListener >& rxListener ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName ) override;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    static inline OUString  getImplementationName_Static() throw();
    static Sequence< OUString > getSupportedServiceNames_Static() throw();
};

inline OUString SpellChecker::getImplementationName_Static() throw()
{
    return OUString( "org.openoffice.lingu.MySpellSpellChecker" );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
