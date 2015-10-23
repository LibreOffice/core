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

#ifndef INCLUDED_MYSQLC_SOURCE_MYSQLC_GENERAL_HXX
#define INCLUDED_MYSQLC_SOURCE_MYSQLC_GENERAL_HXX

#include <config_lgpl.h>

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>

#include <cppconn/exception.h>

namespace mysqlc_sdbc_driver
{
    rtl::OUString getStringFromAny(const css::uno::Any& _rAny);

    void throwFeatureNotImplementedException(
            const sal_Char* _pAsciiFeatureName,
            const css::uno::Reference< css::uno::XInterface >& _rxContext,
            const css::uno::Any* _pNextException = NULL
        )
        throw (css::sdbc::SQLException);

    void throwInvalidArgumentException(
            const sal_Char* _pAsciiFeatureName,
            const css::uno::Reference< css::uno::XInterface >& _rxContext,
            const css::uno::Any* _pNextException = NULL
        )
        throw (css::sdbc::SQLException);

    void translateAndThrow(const ::sql::SQLException& _error, const css::uno::Reference< css::uno::XInterface >& _context, const rtl_TextEncoding encoding);

    int mysqlToOOOType(int mysqlType) throw ();


    rtl::OUString convert(const ::std::string& _string, const rtl_TextEncoding encoding);

    ::std::string convert(const rtl::OUString& _string, const rtl_TextEncoding encoding);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
