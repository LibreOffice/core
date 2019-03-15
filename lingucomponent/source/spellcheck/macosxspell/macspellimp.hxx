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

#ifndef INCLUDED_LINGUCOMPONENT_SOURCE_SPELLCHECK_MACOSXSPELL_MACSPELLIMP_HXX
#define INCLUDED_LINGUCOMPONENT_SOURCE_SPELLCHECK_MACOSXSPELL_MACSPELLIMP_HXX

#include <cppuhelper/implbase.hxx>

#ifdef MACOSX
#include <premac.h>
#include <Carbon/Carbon.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>
#endif
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceDisplayName.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/linguistic2/XSpellChecker.hpp>
#include <com/sun/star/linguistic2/XLinguServiceEventBroadcaster.hpp>

#include <linguistic/misc.hxx>
#include <linguistic/lngprophelp.hxx>

#include <lingutil.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;

class MacSpellChecker :
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
    Sequence< Locale >      aSuppLocales;
    rtl_TextEncoding *      aDEncs;
    Locale *                aDLocs;
    OUString *              aDNames;
    sal_Int32               numdict;
    int                     macTag;   // unique tag for this doc

    ::comphelper::OInterfaceContainerHelper2       aEvtListeners;
    Reference< XPropertyChangeListener >    xPropHelper;
    linguistic::PropertyHelper_Spell *      pPropHelper;
    bool                                    bDisposing;

    MacSpellChecker(const MacSpellChecker &) = delete;
    MacSpellChecker & operator = (const MacSpellChecker &) = delete;

    linguistic::PropertyHelper_Spell &  GetPropHelper_Impl();
    linguistic::PropertyHelper_Spell &  GetPropHelper()
    {
        return pPropHelper ? *pPropHelper : GetPropHelper_Impl();
    }

    sal_Int16   GetSpellFailure( const OUString &rWord, const Locale &rLocale );

public:
    MacSpellChecker();
    virtual ~MacSpellChecker() override;

    // XSupportedLocales (for XSpellChecker)
    virtual Sequence< Locale > SAL_CALL getLocales() override;
    virtual sal_Bool SAL_CALL hasLocale( const Locale& rLocale ) override;

    // XSpellChecker
    virtual sal_Bool SAL_CALL isValid( const OUString& rWord, const Locale& rLocale, const css::uno::Sequence<PropertyValue>& rProperties ) override;
    virtual Reference< XSpellAlternatives > SAL_CALL spell( const OUString& rWord, const Locale& rLocale, const css::uno::Sequence<PropertyValue>& rProperties ) override;

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

inline OUString MacSpellChecker::getImplementationName_Static() throw()
{
    return OUString( "org.openoffice.lingu.MacOSXSpellChecker" );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
