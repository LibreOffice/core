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


OString codemaker::UnoType::decompose(
    OString const & type, sal_Int32 * rank,
    std::vector< OString > * arguments)
{
    sal_Int32 len = type.getLength();
    sal_Int32 i = 0;
    while (len - i > 1 && type[i + 1] == ']') {
        i += 2;
    }
    if (rank != nullptr) {
        *rank = i / 2;
    }
    sal_Int32 j = arguments == nullptr ? -1 : type.indexOf('<', i);
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
