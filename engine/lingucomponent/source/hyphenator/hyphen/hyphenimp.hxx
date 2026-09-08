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

#ifndef INCLUDED_LINGUCOMPONENT_SOURCE_HYPHENATOR_HYPHEN_HYPHENIMP_HXX
#define INCLUDED_LINGUCOMPONENT_SOURCE_HYPHENATOR_HYPHEN_HYPHENIMP_HXX

#include <comphelper/interfacecontainer3.hxx>
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

struct HDInfo {
  HyphenDict *     aPtr;
  OUString         aName;
  css::lang::Locale aLoc;
  rtl_TextEncoding eEnc;
  std::unique_ptr<CharClass> apCC;
};

class Hyphenator :
    public cppu::WeakImplHelper
    <
        css::linguistic2::XHyphenator,
        css::linguistic2::XLinguServiceEventBroadcaster,
        css::lang::XInitialization,
        css::lang::XComponent,
        css::lang::XServiceInfo,
        css::lang::XServiceDisplayName
    >
{
    cpo::uno::Sequence< css::lang::Locale > aSuppLocales;
    std::vector< HDInfo >                   mvDicts;

    ::comphelper::OInterfaceContainerHelper3<css::lang::XEventListener> aEvtListeners;
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
    virtual cpo::uno::Sequence< css::lang::Locale > getLocales() override;
    virtual bool hasLocale( const css::lang::Locale& rLocale ) override;

    // XHyphenator
    virtual cpo::uno::Reference< css::linguistic2::XHyphenatedWord > hyphenate( const OUString& aWord, const css::lang::Locale& aLocale, sal_Int16 nMaxLeading, const cpo::uno::Sequence< css::beans::PropertyValue >& aProperties ) override;
    virtual cpo::uno::Reference< css::linguistic2::XHyphenatedWord > queryAlternativeSpelling( const OUString& aWord, const css::lang::Locale& aLocale, sal_Int16 nIndex, const cpo::uno::Sequence< css::beans::PropertyValue >& aProperties ) override;
    virtual cpo::uno::Reference< css::linguistic2::XPossibleHyphens > createPossibleHyphens( const OUString& aWord, const css::lang::Locale& aLocale, const cpo::uno::Sequence< css::beans::PropertyValue >& aProperties ) override;

    // XLinguServiceEventBroadcaster
    virtual bool addLinguServiceEventListener( const cpo::uno::Reference< css::linguistic2::XLinguServiceEventListener >& rxLstnr ) override;
    virtual bool removeLinguServiceEventListener( const cpo::uno::Reference< css::linguistic2::XLinguServiceEventListener >& rxLstnr ) override;

    // XServiceDisplayName
    virtual OUString getServiceDisplayName( const css::lang::Locale& rLocale ) override;

    // XInitialization
    virtual void initialize( const cpo::uno::Sequence< cpo::uno::Any >& rArguments ) override;

    // XComponent
    virtual void dispose() override;
    virtual void addEventListener( const cpo::uno::Reference< css::lang::XEventListener >& rxListener ) override;
    virtual void removeEventListener( const cpo::uno::Reference< css::lang::XEventListener >& rxListener ) override;

    // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& rServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

private:
    void ensureLocales();
    const HDInfo* getMatchingDict(const css::lang::Locale& aLocale);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
