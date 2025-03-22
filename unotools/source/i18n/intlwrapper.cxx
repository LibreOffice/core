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
#include <unotools/collatorwrapper.hxx>
#include <unotools/localedatawrapper.hxx>
#include <com/sun/star/i18n/CollatorOptions.hpp>
#include <comphelper/processfactory.hxx>
#include <utility>

IntlWrapper::IntlWrapper( LanguageTag aLanguageTag )
        :
        maLanguageTag(std::move( aLanguageTag )),
        m_xContext( comphelper::getProcessComponentContext() )
{
}

IntlWrapper::~IntlWrapper()
{
}

void IntlWrapper::ImplNewLocaleData() const
{
    const_cast<IntlWrapper*>(this)->pLocaleData = LocaleDataWrapper::get( maLanguageTag );
}

void IntlWrapper::ImplNewCollator( bool bCaseSensitive ) const
{
    if ( bCaseSensitive )
    {
        const_cast<IntlWrapper*>(this)->moCaseCollator.emplace(m_xContext);
        const_cast<IntlWrapper*>(this)->moCaseCollator->loadDefaultCollator( maLanguageTag.getLocale(), 0 );
    }
    else
    {
        const_cast<IntlWrapper*>(this)->moCollator.emplace(m_xContext);
        const_cast<IntlWrapper*>(this)->moCollator->loadDefaultCollator( maLanguageTag.getLocale(),
                css::i18n::CollatorOptions::CollatorOptions_IGNORE_CASE );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
