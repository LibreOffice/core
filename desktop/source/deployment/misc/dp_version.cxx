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

#include <o3tl/string_view.hxx>

#include <dp_version.hxx>

namespace {

std::u16string_view getElement(std::u16string_view version, std::size_t * index)
{
    while (*index < version.size() && version[*index] == '0') {
        ++*index;
    }
    return o3tl::getToken(version, u'.', *index);
}

}

namespace dp_misc {

::dp_misc::Order compareVersions(
    std::u16string_view version1, std::u16string_view version2)
{
    for (size_t i1 = 0, i2 = 0; i1 != std::u16string_view::npos || i2 != std::u16string_view::npos;) {
        std::u16string_view e1(i1 != std::u16string_view::npos ? getElement(version1, &i1) : std::u16string_view());
        std::u16string_view e2(i2 != std::u16string_view::npos ? getElement(version2, &i2) : std::u16string_view());
        if (e1.size() < e2.size()) {
            return ::dp_misc::LESS;
        } else if (e1.size() > e2.size()) {
            return ::dp_misc::GREATER;
        } else if (e1 < e2) {
            return ::dp_misc::LESS;
        } else if (e1 > e2) {
            return ::dp_misc::GREATER;
        }
    }
    return ::dp_misc::EQUAL;
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
