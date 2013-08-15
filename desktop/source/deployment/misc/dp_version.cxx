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


#include "sal/config.h"

#include "com/sun/star/deployment/XPackage.hpp"
#include "rtl/ustring.hxx"

#include "dp_version.hxx"

namespace {

OUString getElement(OUString const & version, ::sal_Int32 * index)
{
    while (*index < version.getLength() && version[*index] == '0') {
        ++*index;
    }
    return version.getToken(0, '.', *index);
}

}

namespace dp_misc {

::dp_misc::Order compareVersions(
    OUString const & version1, OUString const & version2)
{
    for (::sal_Int32 i1 = 0, i2 = 0; i1 >= 0 || i2 >= 0;) {
        OUString e1(i1 >= 0 ? getElement(version1, &i1) : OUString());
        OUString e2(i2 >= 0 ? getElement(version2, &i2) : OUString());
        if (e1.getLength() < e2.getLength()) {
            return ::dp_misc::LESS;
        } else if (e1.getLength() > e2.getLength()) {
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
