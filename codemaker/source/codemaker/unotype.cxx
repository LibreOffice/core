/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_codemaker.hxx"

#include "codemaker/unotype.hxx"

#include "osl/diagnose.h"
#include "rtl/string.hxx"
#include "sal/types.h"

#include <vector>

codemaker::UnoType::Sort codemaker::UnoType::getSort(rtl::OString const & type)
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

bool codemaker::UnoType::isSequenceType(rtl::OString const & type) {
    return type.getLength() > 0 && type[0] == '[';
}

rtl::OString codemaker::UnoType::decompose(
    rtl::OString const & type, sal_Int32 * rank,
    std::vector< rtl::OString > * arguments)
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
