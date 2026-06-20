/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <comphelper/interfacecontainer3.hxx>
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

#include <unotools/charclass.hxx>

#include <lingutil.hxx>
#include <linguistic/misc.hxx>
#include <linguistic/lngprophelp.hxx>

#include <osl/file.hxx>
#include <mythes.hxx>
#include <memory>
#include <vector>

namespace com::sun::star::beans { class XPropertySet; }

class Thesaurus :
    public cppu::WeakImplHelper
    <
        css::linguistic2::XThesaurus,
        css::lang::XInitialization,
        css::lang::XComponent,
        css::lang::XServiceInfo,
        css::lang::XServiceDisplayName
    >
{
    css::uno::Sequence< css::lang::Locale > aSuppLocales;

    ::comphelper::OInterfaceContainerHelper3<css::lang::XEventListener> aEvtListeners;
    linguistic::PropertyHelper_Thesaurus*       pPropHelper;
    bool                                    bDisposing;
    struct ThesInfo
    {
        std::unique_ptr<CharClass> aCharSetInfo;
        std::unique_ptr<MyThes> aThes;
        rtl_TextEncoding aEncoding;
        css::lang::Locale aLocale;
        OUString aName;
    };
    std::vector<ThesInfo>                   mvThesInfo;

    // cache for the Thesaurus dialog
    css::uno::Sequence < css::uno::Reference < css::linguistic2::XMeaning > > prevMeanings;
    OUString  prevTerm;
    LanguageType prevLocale;

    Thesaurus(const Thesaurus &) = delete;
    Thesaurus & operator = (const Thesaurus &) = delete;

    linguistic::PropertyHelper_Thesaurus&   GetPropHelper_Impl();
    linguistic::PropertyHelper_Thesaurus&   GetPropHelper()
    {
        return pPropHelper ? *pPropHelper : GetPropHelper_Impl();
    }

public:
    Thesaurus();
    virtual ~Thesaurus() override;

    // XSupportedLocales (for XThesaurus)
    virtual css::uno::Sequence< css::lang::Locale > SAL_CALL getLocales() override;
    virtual bool SAL_CALL hasLocale( const css::lang::Locale& rLocale ) override;

    // XThesaurus
    virtual css::uno::Sequence< css::uno::Reference < css::linguistic2::XMeaning > > SAL_CALL queryMeanings( const OUString& rTerm, const css::lang::Locale& rLocale, const css::uno::Sequence< css::beans::PropertyValue >& rProperties ) override;

    // XServiceDisplayName
    virtual OUString SAL_CALL getServiceDisplayName( const css::lang::Locale& rLocale ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& rArguments ) override;

    // XComponent
    virtual void SAL_CALL dispose() override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& rxListener ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& rxListener ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual bool SAL_CALL supportsService( const OUString& rServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

private:
    static OUString makeLowerCase(const OUString&, CharClass const *);
    static OUString makeUpperCase(const OUString&, CharClass const *);
    static OUString makeInitCap(const OUString&, CharClass const *);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
