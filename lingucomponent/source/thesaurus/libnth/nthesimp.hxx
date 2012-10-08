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

#ifndef _LINGU2_THESIMP_HXX_
#define _LINGU2_THESIMP_HXX_

#include <uno/lbnames.h>            // CPPU_CURRENT_LANGUAGE_BINDING_NAME macro, which specify the environment type
#include <cppuhelper/implbase1.hxx> // helper for implementations
#include <cppuhelper/implbase5.hxx> // helper for implementations
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceDisplayName.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/linguistic2/XMeaning.hpp>
#include <com/sun/star/linguistic2/XThesaurus.hpp>

#include <com/sun/star/linguistic2/XLinguServiceManager.hpp>
#include <com/sun/star/linguistic2/XSpellChecker1.hpp>

#include <unotools/charclass.hxx>

#include <lingutil.hxx>
#include <linguistic/misc.hxx>
#include <linguistic/lngprophelp.hxx>

#include <osl/file.hxx>
#include "mythes.hxx"

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;

namespace com { namespace sun { namespace star { namespace beans {
        class XPropertySet;
}}}}



///////////////////////////////////////////////////////////////////////////


class Thesaurus :
    public cppu::WeakImplHelper5
    <
        XThesaurus,
        XInitialization,
        XComponent,
        XServiceInfo,
        XServiceDisplayName
    >
{
    Sequence< Locale >                      aSuppLocales;

    ::cppu::OInterfaceContainerHelper       aEvtListeners;
    linguistic::PropertyHelper_Thesaurus*       pPropHelper;
    bool                                    bDisposing;
    CharClass **                            aCharSetInfo;
    MyThes **                               aThes;
    rtl_TextEncoding *                      aTEncs;
    Locale *                                aTLocs;
    OUString *                              aTNames;
    sal_Int32                               numthes;

    // cache for the Thesaurus dialog
    Sequence < Reference < ::com::sun::star::linguistic2::XMeaning > > prevMeanings;
    OUString  prevTerm;
    sal_Int16 prevLocale;

    // disallow copy-constructor and assignment-operator for now
    Thesaurus(const Thesaurus &);
    Thesaurus & operator = (const Thesaurus &);

    linguistic::PropertyHelper_Thesaurus&   GetPropHelper_Impl();
    linguistic::PropertyHelper_Thesaurus&   GetPropHelper()
    {
        return pPropHelper ? *pPropHelper : GetPropHelper_Impl();
    }


public:
    Thesaurus();
    virtual ~Thesaurus();

    // XSupportedLocales (for XThesaurus)
    virtual Sequence< Locale > SAL_CALL getLocales() throw(RuntimeException);
    virtual sal_Bool SAL_CALL hasLocale( const Locale& rLocale ) throw(RuntimeException);

    // XThesaurus
    virtual Sequence< Reference < ::com::sun::star::linguistic2::XMeaning > > SAL_CALL queryMeanings( const OUString& rTerm, const Locale& rLocale, const PropertyValues& rProperties ) throw(IllegalArgumentException, RuntimeException);

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


    static inline OUString
        getImplementationName_Static() throw();
        static Sequence< OUString >
        getSupportedServiceNames_Static() throw();

private:
        sal_uInt16 SAL_CALL capitalType(const OUString&, CharClass *);
        OUString SAL_CALL makeLowerCase(const OUString&, CharClass *);
        OUString SAL_CALL makeUpperCase(const OUString&, CharClass *);
        OUString SAL_CALL makeInitCap(const OUString&, CharClass *);

/*  static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XLinguServiceManager > xLngSvcMgr;
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellChecker1 > xSpell;
*/
    static ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XLinguServiceManager > GetLngSvcMgr();

};

inline OUString Thesaurus::getImplementationName_Static() throw()
{
    return A2OU( "org.openoffice.lingu.new.Thesaurus" );
}


///////////////////////////////////////////////////////////////////////////

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
