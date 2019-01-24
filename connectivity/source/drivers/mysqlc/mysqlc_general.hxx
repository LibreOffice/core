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

#include <osl/diagnose.h>
#include <mysql.h>

#if defined __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
#endif

#if defined __GNUC__
#pragma GCC diagnostic pop
#endif

namespace mysqlc_sdbc_driver
{
template <typename T>
void resetSqlVar(void** target, T* pValue, enum_field_types type, sal_Int32 nSize = 0)
{
    if (*target)
    {
        free(*target);
        *target = nullptr;
    }
    constexpr auto nUnitSize = sizeof(T);
    switch (type)
    {
        case MYSQL_TYPE_INT24:
        case MYSQL_TYPE_YEAR:
        case MYSQL_TYPE_NEWDATE:
        case MYSQL_TYPE_BIT:
        case MYSQL_TYPE_GEOMETRY:
        case MYSQL_TYPE_LONG:
        case MYSQL_TYPE_SHORT:
        case MYSQL_TYPE_TINY:
        case MYSQL_TYPE_LONGLONG:
        case MYSQL_TYPE_FLOAT:
        case MYSQL_TYPE_DOUBLE:
        case MYSQL_TYPE_TIME:
        case MYSQL_TYPE_DATE:
        case MYSQL_TYPE_DATETIME:
        case MYSQL_TYPE_TIMESTAMP:
            *target = malloc(nUnitSize);
            memcpy(*target, pValue, nUnitSize);
            break;
        case MYSQL_TYPE_STRING:
        case MYSQL_TYPE_BLOB:
        case MYSQL_TYPE_DECIMAL:
        case MYSQL_TYPE_VARCHAR:
        case MYSQL_TYPE_NEWDECIMAL:
        case MYSQL_TYPE_ENUM:
        case MYSQL_TYPE_SET:
        case MYSQL_TYPE_VAR_STRING:
        case MYSQL_TYPE_TINY_BLOB:
        case MYSQL_TYPE_MEDIUM_BLOB:
        case MYSQL_TYPE_LONG_BLOB:
            *target = malloc(nUnitSize * nSize);
            memcpy(*target, pValue, nUnitSize * nSize);
            break;
        case MYSQL_TYPE_NULL:
            // nothing I guess
            break;
        default:
            OSL_FAIL("resetSqlVar: unknown enum_field_type");
    }
}

void allocateSqlVar(void** mem, enum_field_types eType, unsigned nSize = 0);

void throwFeatureNotImplementedException(
    const sal_Char* _pAsciiFeatureName,
    const css::uno::Reference<css::uno::XInterface>& _rxContext);

void throwInvalidArgumentException(const sal_Char* _pAsciiFeatureName,
                                   const css::uno::Reference<css::uno::XInterface>& _rxContext);

void throwSQLExceptionWithMsg(const char* msg, unsigned int errorNum,
                              const css::uno::Reference<css::uno::XInterface>& _context,
                              const rtl_TextEncoding encoding);

sal_Int32 mysqlToOOOType(int eType, int charsetnr) noexcept;

OUString mysqlTypeToStr(unsigned mysql_type, unsigned mysql_flags);

sal_Int32 mysqlStrToOOOType(const OUString& sType);

OUString convert(const ::std::string& _string, const rtl_TextEncoding encoding);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
