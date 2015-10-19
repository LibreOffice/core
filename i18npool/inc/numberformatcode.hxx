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

class NumberFormatCodeMapper : public cppu::WeakImplHelper
<
    css::i18n::XNumberFormatCode,
    css::lang::XServiceInfo
>
{
public:
    NumberFormatCodeMapper( const css::uno::Reference <
                    css::uno::XComponentContext >& rxContext );
    virtual ~NumberFormatCodeMapper();

    virtual css::i18n::NumberFormatCode SAL_CALL getDefault( sal_Int16 nFormatType, sal_Int16 nFormatUsage, const css::lang::Locale& rLocale ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::i18n::NumberFormatCode SAL_CALL getFormatCode( sal_Int16 nFormatIndex, const css::lang::Locale& rLocale ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::i18n::NumberFormatCode > SAL_CALL getAllFormatCode( sal_Int16 nFormatUsage, const css::lang::Locale& rLocale ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::i18n::NumberFormatCode > SAL_CALL getAllFormatCodes( const css::lang::Locale& rLocale ) throw(css::uno::RuntimeException, std::exception) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName)
                throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                throw( css::uno::RuntimeException, std::exception ) override;

private:
    css::lang::Locale aLocale;
    css::uno::Reference < css::uno::XComponentContext > mxContext;
    css::uno::Sequence< css::i18n::FormatElement > aFormatSeq;
    css::uno::Reference < css::i18n::XLocaleData4 > mxLocaleData;
    bool bFormatsValid;

    void setupLocale( const css::lang::Locale& rLocale );
    void getFormats( const css::lang::Locale& rLocale );
    static OUString mapElementTypeShortToString(sal_Int16 formatType);
    static sal_Int16 mapElementTypeStringToShort(const OUString& formatType);
    static OUString mapElementUsageShortToString(sal_Int16 formatUsage);
    static sal_Int16 mapElementUsageStringToShort(const OUString& formatUsage);
    void createLocaleDataObject();
};


#endif // INCLUDED_I18NPOOL_INC_NUMBERFORMATCODE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
