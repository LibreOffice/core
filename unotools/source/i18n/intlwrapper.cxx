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

#include <unotools/intlwrapper.hxx>
#include <com/sun/star/i18n/CollatorOptions.hpp>
#include <i18nlangtag/mslangid.hxx>
#include <comphelper/processfactory.hxx>

IntlWrapper::IntlWrapper(
            const css::uno::Reference< css::uno::XComponentContext > & rxContext,
            const LanguageTag& rLanguageTag )
        :
        maLanguageTag( rLanguageTag ),
        m_xContext( rxContext ),
        pLocaleData( nullptr ),
        pCollator( nullptr ),
        pCaseCollator( nullptr )
{
}

IntlWrapper::IntlWrapper(
            const LanguageTag& rLanguageTag )
        :
        maLanguageTag( rLanguageTag ),
        m_xContext( comphelper::getProcessComponentContext() ),
        pLocaleData( nullptr ),
        pCollator( nullptr ),
        pCaseCollator( nullptr )
{
}

IntlWrapper::~IntlWrapper()
{
    delete pLocaleData;
    delete pCollator;
    delete pCaseCollator;
}

void IntlWrapper::ImplNewLocaleData() const
{
    const_cast<IntlWrapper*>(this)->pLocaleData = new LocaleDataWrapper( m_xContext, maLanguageTag );
}

void IntlWrapper::ImplNewCollator( bool bCaseSensitive ) const
{
    CollatorWrapper* p = new CollatorWrapper( m_xContext );
    if ( bCaseSensitive )
    {
        p->loadDefaultCollator( maLanguageTag.getLocale(), 0 );
        const_cast<IntlWrapper*>(this)->pCaseCollator = p;
    }
    else
    {
        p->loadDefaultCollator( maLanguageTag.getLocale(),
                css::i18n::CollatorOptions::CollatorOptions_IGNORE_CASE );
        const_cast<IntlWrapper*>(this)->pCollator = p;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
