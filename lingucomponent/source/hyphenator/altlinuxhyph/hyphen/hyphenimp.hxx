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


#ifndef _LINGU2_HYPHENIMP_HXX_
#define _LINGU2_HYPHENIMP_HXX_

#include <uno/lbnames.h>            // CPPU_CURRENT_LANGUAGE_BINDING_NAME macro, which specify the environment type
#include <cppuhelper/implbase1.hxx> // helper for implementations
#include <cppuhelper/implbase6.hxx> // helper for implementations
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

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;


///////////////////////////////////////////////////////////////////////////


struct HDInfo {
  HyphenDict *     aPtr;
  OUString         aName;
  Locale           aLoc;
  rtl_TextEncoding eEnc;
  CharClass *      apCC;
};



class Hyphenator :
    public cppu::WeakImplHelper6
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

    // disallow copy-constructor and assignment-operator for now
    Hyphenator(const Hyphenator &);
    Hyphenator & operator = (const Hyphenator &);

    linguistic::PropertyHelper_Hyphenation& GetPropHelper_Impl();
    linguistic::PropertyHelper_Hyphenation& GetPropHelper()
    {
        return pPropHelper ? *pPropHelper : GetPropHelper_Impl();
    }

public:
    Hyphenator();

    virtual ~Hyphenator();

    // XSupportedLocales (for XHyphenator)
    virtual Sequence< Locale > SAL_CALL getLocales() throw(RuntimeException);
    virtual sal_Bool SAL_CALL hasLocale( const Locale& rLocale ) throw(RuntimeException);

    // XHyphenator
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XHyphenatedWord > SAL_CALL hyphenate( const ::rtl::OUString& aWord, const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nMaxLeading, const ::com::sun::star::beans::PropertyValues& aProperties ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XHyphenatedWord > SAL_CALL queryAlternativeSpelling( const ::rtl::OUString& aWord, const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nIndex, const ::com::sun::star::beans::PropertyValues& aProperties ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XPossibleHyphens > SAL_CALL createPossibleHyphens( const ::rtl::OUString& aWord, const ::com::sun::star::lang::Locale& aLocale, const ::com::sun::star::beans::PropertyValues& aProperties ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    // XLinguServiceEventBroadcaster
    virtual sal_Bool SAL_CALL addLinguServiceEventListener( const Reference< XLinguServiceEventListener >& rxLstnr ) throw(RuntimeException);
    virtual sal_Bool SAL_CALL removeLinguServiceEventListener( const Reference< XLinguServiceEventListener >& rxLstnr ) throw(RuntimeException);

    // XServiceDisplayName
    virtual OUString SAL_CALL getServiceDisplayName( const Locale& rLocale ) throw(RuntimeException);

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& rArguments ) throw(Exception, RuntimeException);

    // XComponent
    virtual void SAL_CALL dispose() throw(RuntimeException);
    virtual void SAL_CALL addEventListener( const Reference< XEventListener >& rxListener ) throw(RuntimeException);
    virtual void SAL_CALL removeEventListener( const Reference< XEventListener >& rxListener ) throw(RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName ) throw(RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(RuntimeException);


    static inline OUString  getImplementationName_Static() throw();
    static Sequence< OUString > getSupportedServiceNames_Static() throw();


private:
        sal_uInt16 SAL_CALL capitalType(const OUString&, CharClass *);
        OUString SAL_CALL makeLowerCase(const OUString&, CharClass *);
        OUString SAL_CALL makeUpperCase(const OUString&, CharClass *);
        OUString SAL_CALL makeInitCap(const OUString&, CharClass *);
};

inline OUString Hyphenator::getImplementationName_Static() throw()
{
    return A2OU( "org.openoffice.lingu.LibHnjHyphenator" );
}


///////////////////////////////////////////////////////////////////////////

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
