/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TEST_SOURCE_ISHEADLESS_HXX
#define INCLUDED_TEST_SOURCE_ISHEADLESS_HXX

#include <sal/config.h>

#include <rtl/process.h>
#include <rtl/ustring.hxx>
#include <sal/types.h>

namespace test {

inline bool isHeadless() {
    sal_uInt32 n = rtl_getAppCommandArgCount();
    for (sal_uInt32 i = 0; i != n; ++i) {
        OUString arg;
        rtl_getAppCommandArg(i, &arg.pData);
        if (arg == "--headless") {
            return true;
        }
    }
    return false;
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
