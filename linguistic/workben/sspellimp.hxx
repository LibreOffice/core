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

#ifndef INCLUDED_LINGUISTIC_WORKBEN_SSPELLIMP_HXX
#define INCLUDED_LINGUISTIC_WORKBEN_SSPELLIMP_HXX

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

#include "linguistic/misc.hxx"
#include "sprophelp.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;


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
    Sequence< Locale >                      aSuppLocales;
    ::comphelper::OInterfaceContainerHelper2       aEvtListeners;
    Reference< XPropertyChangeListener >    xPropHelper;
    PropertyHelper_Spell *                  pPropHelper;
    sal_Bool                                    bDisposing;

    // disallow copy-constructor and assignment-operator for now
    SpellChecker(const SpellChecker &);
    SpellChecker & operator = (const SpellChecker &);

    PropertyHelper_Spell &  GetPropHelper_Impl();
    PropertyHelper_Spell &  GetPropHelper()
    {
        return pPropHelper ? *pPropHelper : GetPropHelper_Impl();
    }

    sal_Int16   GetSpellFailure( const OUString &rWord, const Locale &rLocale );
    Reference< XSpellAlternatives >
            GetProposals( const OUString &rWord, const Locale &rLocale );

public:
    SpellChecker();
    virtual ~SpellChecker();

    // XSupportedLocales (for XSpellChecker)
    virtual Sequence< Locale > SAL_CALL
        getLocales()
            throw(RuntimeException);
    virtual sal_Bool SAL_CALL
        hasLocale( const Locale& rLocale )
            throw(RuntimeException);

    // XSpellChecker
    virtual sal_Bool SAL_CALL
        isValid( const OUString& rWord, const Locale& rLocale,
                const PropertyValues& rProperties )
            throw(IllegalArgumentException,
                  RuntimeException);
    virtual Reference< XSpellAlternatives > SAL_CALL
        spell( const OUString& rWord, const Locale& rLocale,
                const PropertyValues& rProperties )
            throw(IllegalArgumentException,
                  RuntimeException);

    // XLinguServiceEventBroadcaster
    virtual sal_Bool SAL_CALL
        addLinguServiceEventListener(
            const Reference< XLinguServiceEventListener >& rxLstnr )
            throw(RuntimeException);
    virtual sal_Bool SAL_CALL
        removeLinguServiceEventListener(
            const Reference< XLinguServiceEventListener >& rxLstnr )
            throw(RuntimeException);

    // XServiceDisplayName
    virtual OUString SAL_CALL
        getServiceDisplayName( const Locale& rLocale )
            throw(RuntimeException);

    // XInitialization
    virtual void SAL_CALL
        initialize( const Sequence< Any >& rArguments )
            throw(Exception, RuntimeException);

    // XComponent
    virtual void SAL_CALL
        dispose()
            throw(RuntimeException);
    virtual void SAL_CALL
        addEventListener( const Reference< XEventListener >& rxListener )
            throw(RuntimeException);
    virtual void SAL_CALL
        removeEventListener( const Reference< XEventListener >& rxListener )
            throw(RuntimeException);

    // Service specific part

    // XServiceInfo
    virtual OUString SAL_CALL
        getImplementationName()
            throw(RuntimeException);
    virtual sal_Bool SAL_CALL
        supportsService( const OUString& rServiceName )
            throw(RuntimeException);
    virtual Sequence< OUString > SAL_CALL
        getSupportedServiceNames()
            throw(RuntimeException);


    static inline OUString
        getImplementationName_Static() throw();
    static Sequence< OUString >
        getSupportedServiceNames_Static() throw();
};

inline OUString SpellChecker::getImplementationName_Static() throw()
{
    return OUString( "com.sun.star.lingu.examples.SpellChecker" );
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
