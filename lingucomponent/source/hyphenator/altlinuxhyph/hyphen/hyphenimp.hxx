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
