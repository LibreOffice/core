/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    return ( !type.isEmpty() && type[0] == '[' );
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
