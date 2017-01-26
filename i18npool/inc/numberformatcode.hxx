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

#ifndef INCLUDED_I18NPOOL_INC_NUMBERFORMATCODE_HXX
#define INCLUDED_I18NPOOL_INC_NUMBERFORMATCODE_HXX

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase.hxx>

#include <com/sun/star/i18n/XNumberFormatCode.hpp>
#include <com/sun/star/i18n/XLocaleData4.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <deque>
#include <utility>

class NumberFormatCodeMapper : public cppu::WeakImplHelper
<
    css::i18n::XNumberFormatCode,
    css::lang::XServiceInfo
>
{
public:
    NumberFormatCodeMapper( const css::uno::Reference <
                    css::uno::XComponentContext >& rxContext );
    virtual ~NumberFormatCodeMapper() override;

    virtual css::i18n::NumberFormatCode SAL_CALL getDefault( sal_Int16 nFormatType, sal_Int16 nFormatUsage, const css::lang::Locale& rLocale ) override;
    virtual css::i18n::NumberFormatCode SAL_CALL getFormatCode( sal_Int16 nFormatIndex, const css::lang::Locale& rLocale ) override;
    virtual css::uno::Sequence< css::i18n::NumberFormatCode > SAL_CALL getAllFormatCode( sal_Int16 nFormatUsage, const css::lang::Locale& rLocale ) override;
    virtual css::uno::Sequence< css::i18n::NumberFormatCode > SAL_CALL getAllFormatCodes( const css::lang::Locale& rLocale ) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

private:
    osl::Mutex maMutex;
    css::uno::Reference < css::i18n::XLocaleData4 > m_xLocaleData;
    typedef std::pair< css::lang::Locale, css::uno::Sequence< css::i18n::FormatElement > > FormatElementCacheItem;
    std::deque < FormatElementCacheItem > m_aFormatElementCache;

    const css::uno::Sequence< css::i18n::FormatElement >& getFormats( const css::lang::Locale& rLocale );
    static OUString mapElementTypeShortToString(sal_Int16 formatType);
    static sal_Int16 mapElementTypeStringToShort(const OUString& formatType);
    static OUString mapElementUsageShortToString(sal_Int16 formatUsage);
    static sal_Int16 mapElementUsageStringToShort(const OUString& formatUsage);
};


#endif // INCLUDED_I18NPOOL_INC_NUMBERFORMATCODE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
