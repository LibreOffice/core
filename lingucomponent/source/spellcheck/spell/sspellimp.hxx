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
#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>
#include <com/sun/star/linguistic2/XLinguServiceEventBroadcaster.hpp>

#include <linguistic/misc.hxx>
#include <linguistic/lngprophelp.hxx>

#include <lingutil.hxx>

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
    Sequence< Locale >                 aSuppLocales;
    Hunspell **                        aDicts;
    rtl_TextEncoding *                 aDEncs;
    Locale *                           aDLocs;
    OUString *                         aDNames;
    sal_Int32                          numdict;

    ::cppu::OInterfaceContainerHelper       aEvtListeners;
    linguistic::PropertyHelper_Spelling*    pPropHelper;
    bool                                    bDisposing;

    SpellChecker(const SpellChecker &) = delete;
    SpellChecker & operator = (const SpellChecker &) = delete;

    linguistic::PropertyHelper_Spelling&  GetPropHelper_Impl();
    linguistic::PropertyHelper_Spelling&  GetPropHelper()
    {
        return pPropHelper ? *pPropHelper : GetPropHelper_Impl();
    }

    sal_Int16   GetSpellFailure( const OUString &rWord, const Locale &rLocale );
    Reference< XSpellAlternatives > GetProposals( const OUString &rWord, const Locale &rLocale );

public:
    SpellChecker();
    virtual ~SpellChecker();

    // XSupportedLocales (for XSpellChecker)
    virtual Sequence< Locale > SAL_CALL getLocales() throw(RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasLocale( const Locale& rLocale ) throw(RuntimeException, std::exception) SAL_OVERRIDE;

    // XSpellChecker
    virtual sal_Bool SAL_CALL isValid( const OUString& rWord, const Locale& rLocale, const PropertyValues& rProperties ) throw(IllegalArgumentException, RuntimeException, std::exception) SAL_OVERRIDE;
    virtual Reference< XSpellAlternatives > SAL_CALL spell( const OUString& rWord, const Locale& rLocale, const PropertyValues& rProperties ) throw(IllegalArgumentException, RuntimeException, std::exception) SAL_OVERRIDE;

    // XLinguServiceEventBroadcaster
    virtual sal_Bool SAL_CALL addLinguServiceEventListener( const Reference< XLinguServiceEventListener >& rxLstnr ) throw(RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL removeLinguServiceEventListener( const Reference< XLinguServiceEventListener >& rxLstnr ) throw(RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceDisplayName
    virtual OUString SAL_CALL getServiceDisplayName( const Locale& rLocale ) throw(RuntimeException, std::exception) SAL_OVERRIDE;

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& rArguments ) throw(Exception, RuntimeException, std::exception) SAL_OVERRIDE;

    // XComponent
    virtual void SAL_CALL dispose() throw(RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addEventListener( const Reference< XEventListener >& rxListener ) throw(RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeEventListener( const Reference< XEventListener >& rxListener ) throw(RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName ) throw(RuntimeException, std::exception) SAL_OVERRIDE;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(RuntimeException, std::exception) SAL_OVERRIDE;

    static inline OUString  getImplementationName_Static() throw();
    static Sequence< OUString > getSupportedServiceNames_Static() throw();
};

inline OUString SpellChecker::getImplementationName_Static() throw()
{
    return OUString( "org.openoffice.lingu.MySpellSpellChecker" );
}

void * SAL_CALL SpellChecker_getFactory(
    char const * pImplName, css::lang::XMultiServiceFactory * pServiceManager,
    void *);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
