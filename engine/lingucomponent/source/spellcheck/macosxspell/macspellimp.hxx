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

#ifndef INCLUDED_LINGUCOMPONENT_SOURCE_SPELLCHECK_MACOSXSPELL_MACSPELLIMP_HXX
#define INCLUDED_LINGUCOMPONENT_SOURCE_SPELLCHECK_MACOSXSPELL_MACSPELLIMP_HXX

#include <comphelper/interfacecontainer3.hxx>
#include <cppuhelper/implbase.hxx>

#include <premac.h>
#ifdef MACOSX
#import <Cocoa/Cocoa.h>
#else
#include <UIKit/UIKit.h>
#endif
#include <postmac.h>
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

class MacSpellChecker :
    public cppu::WeakImplHelper
    <
        css::linguistic2::XSpellChecker,
        css::linguistic2::XLinguServiceEventBroadcaster,
        css::lang::XInitialization,
        css::lang::XComponent,
        css::lang::XServiceInfo,
        css::lang::XServiceDisplayName
    >
{
    cpo::uno::Sequence< css::lang::Locale > aSuppLocales;
    rtl_TextEncoding *      aDEncs;
    css::lang::Locale *     aDLocs;
    OUString *              aDNames;
    sal_Int32               numdict;
#ifdef MACOSX
    int                     macTag;   // unique tag for this doc
#else
    UITextChecker *         pChecker;
#endif
    ::comphelper::OInterfaceContainerHelper3<css::lang::XEventListener> aEvtListeners;
    rtl::Reference< linguistic::PropertyHelper_Spell >    xPropHelper;
    bool                                    bDisposing;

    MacSpellChecker(const MacSpellChecker &) = delete;
    MacSpellChecker & operator = (const MacSpellChecker &) = delete;

    linguistic::PropertyHelper_Spell &  GetPropHelper_Impl();
    linguistic::PropertyHelper_Spell &  GetPropHelper()
    {
        return xPropHelper.is() ? *xPropHelper : GetPropHelper_Impl();
    }

    sal_Int16   GetSpellFailure( const OUString &rWord, const css::lang::Locale &rLocale );
    cpo::uno::Reference< css::linguistic2::XSpellAlternatives > GetProposals( const OUString &rWord, const css::lang::Locale &rLocale );

public:
    MacSpellChecker();
    virtual ~MacSpellChecker() override;

    // XSupportedLocales (for XSpellChecker)
    virtual cpo::uno::Sequence< css::lang::Locale > getLocales() override;
    virtual bool hasLocale( const css::lang::Locale& rLocale ) override;

    // XSpellChecker
    virtual bool isValid( const OUString& rWord, const css::lang::Locale& rLocale, const cpo::uno::Sequence<css::beans::PropertyValue>& rProperties ) override;
    virtual cpo::uno::Reference< css::linguistic2::XSpellAlternatives > spell( const OUString& rWord, const css::lang::Locale& rLocale, const cpo::uno::Sequence<css::beans::PropertyValue>& rProperties ) override;

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
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
