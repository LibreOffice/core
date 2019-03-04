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

#ifndef INCLUDED_LINGUCOMPONENT_SOURCE_HYPHENATOR_HYPHEN_HYPHENIMP_HXX
#define INCLUDED_LINGUCOMPONENT_SOURCE_HYPHENATOR_HYPHEN_HYPHENIMP_HXX

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceDisplayName.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/linguistic2/XHyphenator.hpp>
#include <com/sun/star/linguistic2/XLinguServiceEventBroadcaster.hpp>

#include <unotools/charclass.hxx>

#include <linguistic/misc.hxx>
#include <linguistic/lngprophelp.hxx>

#include <lingutil.hxx>
#include <stdio.h>

#include <hyphen.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;

struct HDInfo {
  HyphenDict *     aPtr;
  OUString         aName;
  Locale           aLoc;
  rtl_TextEncoding eEnc;
  std::unique_ptr<CharClass> apCC;
};

class Hyphenator :
    public cppu::WeakImplHelper
    <
        XHyphenator,
        XLinguServiceEventBroadcaster,
        XInitialization,
        XComponent,
        XServiceInfo,
        XServiceDisplayName
    >
{
    Sequence< Locale >                      aSuppLocales;
    std::vector< HDInfo >                   mvDicts;

    ::comphelper::OInterfaceContainerHelper2       aEvtListeners;
    std::unique_ptr<linguistic::PropertyHelper_Hyphenation> pPropHelper;
    bool                                    bDisposing;

    Hyphenator(const Hyphenator &) = delete;
    Hyphenator & operator = (const Hyphenator &) = delete;

    linguistic::PropertyHelper_Hyphenation& GetPropHelper_Impl();
    linguistic::PropertyHelper_Hyphenation& GetPropHelper()
    {
        return pPropHelper ? *pPropHelper : GetPropHelper_Impl();
    }

public:
    Hyphenator();

    virtual ~Hyphenator() override;

    // XSupportedLocales (for XHyphenator)
    virtual Sequence< Locale > SAL_CALL getLocales() override;
    virtual sal_Bool SAL_CALL hasLocale( const Locale& rLocale ) override;

    // XHyphenator
    virtual css::uno::Reference< css::linguistic2::XHyphenatedWord > SAL_CALL hyphenate( const OUString& aWord, const css::lang::Locale& aLocale, sal_Int16 nMaxLeading, const css::uno::Sequence< css::beans::PropertyValue >& aProperties ) override;
    virtual css::uno::Reference< css::linguistic2::XHyphenatedWord > SAL_CALL queryAlternativeSpelling( const OUString& aWord, const css::lang::Locale& aLocale, sal_Int16 nIndex, const css::uno::Sequence< css::beans::PropertyValue >& aProperties ) override;
    virtual css::uno::Reference< css::linguistic2::XPossibleHyphens > SAL_CALL createPossibleHyphens( const OUString& aWord, const css::lang::Locale& aLocale, const css::uno::Sequence< css::beans::PropertyValue >& aProperties ) override;

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

private:
        static OUString makeLowerCase(const OUString&, CharClass const *);
        static OUString makeUpperCase(const OUString&, CharClass const *);
        static OUString makeInitCap(const OUString&, CharClass const *);
};

inline OUString Hyphenator::getImplementationName_Static() throw()
{
    return OUString( "org.openoffice.lingu.LibHnjHyphenator" );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
