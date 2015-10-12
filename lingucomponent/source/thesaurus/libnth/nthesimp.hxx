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

#ifndef INCLUDED_LINGUCOMPONENT_SOURCE_THESAURUS_LIBNTH_NTHESIMP_HXX
#define INCLUDED_LINGUCOMPONENT_SOURCE_THESAURUS_LIBNTH_NTHESIMP_HXX

#include <cppuhelper/implbase.hxx>
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

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;

namespace com { namespace sun { namespace star { namespace beans {
        class XPropertySet;
}}}}

class Thesaurus :
    public cppu::WeakImplHelper
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

    Thesaurus(const Thesaurus &) = delete;
    Thesaurus & operator = (const Thesaurus &) = delete;

    linguistic::PropertyHelper_Thesaurus&   GetPropHelper_Impl();
    linguistic::PropertyHelper_Thesaurus&   GetPropHelper()
    {
        return pPropHelper ? *pPropHelper : GetPropHelper_Impl();
    }

public:
    Thesaurus();
    virtual ~Thesaurus();

    // XSupportedLocales (for XThesaurus)
    virtual Sequence< Locale > SAL_CALL getLocales() throw(RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasLocale( const Locale& rLocale ) throw(RuntimeException, std::exception) SAL_OVERRIDE;

    // XThesaurus
    virtual Sequence< Reference < ::com::sun::star::linguistic2::XMeaning > > SAL_CALL queryMeanings( const OUString& rTerm, const Locale& rLocale, const PropertyValues& rProperties ) throw(IllegalArgumentException, RuntimeException, std::exception) SAL_OVERRIDE;

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

    static inline OUString
        getImplementationName_Static() throw();
        static Sequence< OUString >
        getSupportedServiceNames_Static() throw();

private:
    static OUString SAL_CALL makeLowerCase(const OUString&, CharClass *);
    static OUString SAL_CALL makeUpperCase(const OUString&, CharClass *);
    static OUString SAL_CALL makeInitCap(const OUString&, CharClass *);
};

inline OUString Thesaurus::getImplementationName_Static() throw()
{
    return OUString( "org.openoffice.lingu.new.Thesaurus" );
}

void * SAL_CALL Thesaurus_getFactory(
    char const * pImplName, css::lang::XMultiServiceFactory * pServiceManager,
    void *);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
