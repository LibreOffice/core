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
#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>
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
  CharClass *      apCC;
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
    HDInfo * aDicts;
    sal_Int32 numdict;

    ::cppu::OInterfaceContainerHelper       aEvtListeners;
    Reference< XMultiServiceFactory >       rSMgr;
    linguistic::PropertyHelper_Hyphenation* pPropHelper;
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

    virtual ~Hyphenator();

    // XSupportedLocales (for XHyphenator)
    virtual Sequence< Locale > SAL_CALL getLocales() throw(RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasLocale( const Locale& rLocale ) throw(RuntimeException, std::exception) override;

    // XHyphenator
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XHyphenatedWord > SAL_CALL hyphenate( const OUString& aWord, const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nMaxLeading, const ::com::sun::star::beans::PropertyValues& aProperties ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XHyphenatedWord > SAL_CALL queryAlternativeSpelling( const OUString& aWord, const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nIndex, const ::com::sun::star::beans::PropertyValues& aProperties ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XPossibleHyphens > SAL_CALL createPossibleHyphens( const OUString& aWord, const ::com::sun::star::lang::Locale& aLocale, const ::com::sun::star::beans::PropertyValues& aProperties ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    // XLinguServiceEventBroadcaster
    virtual sal_Bool SAL_CALL addLinguServiceEventListener( const Reference< XLinguServiceEventListener >& rxLstnr ) throw(RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL removeLinguServiceEventListener( const Reference< XLinguServiceEventListener >& rxLstnr ) throw(RuntimeException, std::exception) override;

    // XServiceDisplayName
    virtual OUString SAL_CALL getServiceDisplayName( const Locale& rLocale ) throw(RuntimeException, std::exception) override;

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& rArguments ) throw(Exception, RuntimeException, std::exception) override;

    // XComponent
    virtual void SAL_CALL dispose() throw(RuntimeException, std::exception) override;
    virtual void SAL_CALL addEventListener( const Reference< XEventListener >& rxListener ) throw(RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEventListener( const Reference< XEventListener >& rxListener ) throw(RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName ) throw(RuntimeException, std::exception) override;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(RuntimeException, std::exception) override;

    static inline OUString  getImplementationName_Static() throw();
    static Sequence< OUString > getSupportedServiceNames_Static() throw();

private:
        static OUString SAL_CALL makeLowerCase(const OUString&, CharClass *);
        static OUString SAL_CALL makeUpperCase(const OUString&, CharClass *);
        static OUString SAL_CALL makeInitCap(const OUString&, CharClass *);
};

inline OUString Hyphenator::getImplementationName_Static() throw()
{
    return OUString( "org.openoffice.lingu.LibHnjHyphenator" );
}

void * SAL_CALL Hyphenator_getFactory(
    char const * pImplName, css::lang::XMultiServiceFactory * pServiceManager,
    void *);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
