/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _LINGU2_SPELLIMP_HXX_
#define _LINGU2_SPELLIMP_HXX_

#include <uno/lbnames.h>            // CPPU_CURRENT_LANGUAGE_BINDING_NAME macro, which specify the environment type
#include <cppuhelper/implbase1.hxx> // helper for implementations
#include <cppuhelper/implbase6.hxx> // helper for implementations
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceDisplayName.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/linguistic2/XSpellChecker.hpp>
#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>
#include <com/sun/star/linguistic2/XLinguServiceEventBroadcaster.hpp>
#include <tools/table.hxx>

#include "linguistic/misc.hxx"
#include "sprophelp.hxx"

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;



class SpellChecker :
    public cppu::WeakImplHelper6
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
    ::cppu::OInterfaceContainerHelper       aEvtListeners;
    Reference< XPropertyChangeListener >    xPropHelper;
    PropertyHelper_Spell *                  pPropHelper;
    BOOL                                    bDisposing;

    // disallow copy-constructor and assignment-operator for now
    SpellChecker(const SpellChecker &);
    SpellChecker & operator = (const SpellChecker &);

    PropertyHelper_Spell &  GetPropHelper_Impl();
    PropertyHelper_Spell &  GetPropHelper()
    {
        return pPropHelper ? *pPropHelper : GetPropHelper_Impl();
    }

    INT16   GetSpellFailure( const OUString &rWord, const Locale &rLocale );
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
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.lingu.examples.SpellChecker" ));
}



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
