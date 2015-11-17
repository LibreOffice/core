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

#include <sal/config.h>

#include <sal/log.hxx>
#include <unotools/numberformatcodewrapper.hxx>
#include <com/sun/star/i18n/NumberFormatMapper.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::uno;

NumberFormatCodeWrapper::NumberFormatCodeWrapper(
            const Reference< uno::XComponentContext > & rxContext,
            const lang::Locale& rLocale
            )
{
    setLocale( rLocale );
    xNFC = i18n::NumberFormatMapper::create( rxContext );
}

NumberFormatCodeWrapper::~NumberFormatCodeWrapper()
{
}

void NumberFormatCodeWrapper::setLocale( const css::lang::Locale& rLocale )
{
    aLocale = rLocale;
}

css::i18n::NumberFormatCode
NumberFormatCodeWrapper::getFormatCode( sal_Int16 formatIndex ) const
{
    try
    {
        return xNFC->getFormatCode( formatIndex, aLocale );
    }
    catch ( const Exception& )
    {
        SAL_WARN( "unotools.i18n", "getFormatCode: Exception caught!" );
    }
    return css::i18n::NumberFormatCode();
}

css::uno::Sequence< css::i18n::NumberFormatCode >
NumberFormatCodeWrapper::getAllFormatCode( sal_Int16 formatUsage ) const
{
    try
    {
        return xNFC->getAllFormatCode( formatUsage, aLocale );
    }
    catch ( const Exception& )
    {
        SAL_WARN( "unotools.i18n", "getAllFormatCode: Exception caught!" );
    }
    return css::uno::Sequence< css::i18n::NumberFormatCode > (0);
}

css::uno::Sequence< css::i18n::NumberFormatCode >
NumberFormatCodeWrapper::getAllFormatCodes() const
{
    try
    {
        return xNFC->getAllFormatCodes( aLocale );
    }
    catch ( const Exception& )
    {
        SAL_WARN( "unotools.i18n", "getAllFormatCodes: Exception caught!" );
    }
    return css::uno::Sequence< css::i18n::NumberFormatCode > (0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
