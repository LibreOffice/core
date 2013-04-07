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


#include "codemaker/unotype.hxx"

#include "osl/diagnose.h"
#include "rtl/string.hxx"
#include "sal/types.h"

#include <vector>

codemaker::UnoType::Sort codemaker::UnoType::getSort(OString const & type)
{
    return type == "void" ? SORT_VOID
        : type == "boolean" ? SORT_BOOLEAN
        : type == "byte" ? SORT_BYTE
        : type == "short" ? SORT_SHORT
        : type == "unsigned short" ? SORT_UNSIGNED_SHORT
        : type == "long" ? SORT_LONG
        : type == "unsigned long" ? SORT_UNSIGNED_LONG
        : type == "hyper" ? SORT_HYPER
        : type == "unsigned hyper" ? SORT_UNSIGNED_HYPER
        : type == "float" ? SORT_FLOAT
        : type == "double" ? SORT_DOUBLE
        : type == "char" ? SORT_CHAR
        : type == "string" ? SORT_STRING
        : type == "type" ? SORT_TYPE
        : type == "any" ? SORT_ANY
        : SORT_COMPLEX;
}

bool codemaker::UnoType::isSequenceType(OString const & type) {
    return !type.isEmpty() && type[0] == '[';
}

OString codemaker::UnoType::decompose(
    OString const & type, sal_Int32 * rank,
    std::vector< OString > * arguments)
{
    sal_Int32 len = type.getLength();
    sal_Int32 i = 0;
    while (len - i > 1 && type[i + 1] == ']') {
        i += 2;
    }
    if (rank != 0) {
        *rank = i / 2;
    }
    sal_Int32 j = arguments == 0 ? -1 : type.indexOf('<', i);
    if (j < 0) {
        return type.copy(i);
    }
    sal_Int32 k = j;
    do {
        ++k; // skip '<' or ','
        sal_Int32 l = k;
        for (sal_Int32 level = 0; l != len; ++l) {
            char c = type[l];
            if (c == ',') {
                if (level == 0) {
                    break;
                }
            } else if (c == '<') {
                ++level;
            } else if (c == '>') {
                if (level == 0) {
                    break;
                }
                --level;
            }
        }
        arguments->push_back(type.copy(k, l - k));
        k = l;
    } while (k != len && type[k] != '>');
    OSL_ASSERT(k == len - 1 && type[k] == '>');
    return type.copy(i, j - i);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
