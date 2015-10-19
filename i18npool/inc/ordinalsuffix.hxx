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

#ifndef INCLUDED_I18NPOOL_INC_ORDINALSUFFIX_HXX
#define INCLUDED_I18NPOOL_INC_ORDINALSUFFIX_HXX

#include <com/sun/star/i18n/XOrdinalSuffix.hpp>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/Locale.hpp>


namespace com { namespace sun { namespace star { namespace i18n {

class OrdinalSuffixService : public cppu::WeakImplHelper
<
    XOrdinalSuffix,
    css::lang::XServiceInfo
>
{
    public:
        OrdinalSuffixService();
        virtual ~OrdinalSuffixService();

        // XOrdinalSuffix
        virtual css::uno::Sequence< OUString > SAL_CALL getOrdinalSuffix( sal_Int32 nNumber, const css::lang::Locale &rLocale ) throw(css::uno::RuntimeException, std::exception) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw(css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw(css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence < OUString > SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;

};
} } } }

#endif // INCLUDED_I18NPOOL_INC_ORDINALSUFFIX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
