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

#if defined UNX

#include <cstddef>

#include <osl/diagnose.h>
#include <rtl/strbuf.hxx>
#include <rtl/string.h>
#include <rtl/string.hxx>
#include <sal/types.h>
#include <tools/appendunixshellword.hxx>

namespace tools {

void appendUnixShellWord(
    OStringBuffer * accumulator, OString const & text)
{
    OSL_ASSERT(accumulator != nullptr);
    if (text.isEmpty()) {
        accumulator->append("''");
    } else {
        bool quoted = false;
        for (sal_Int32 i = 0; i < text.getLength(); ++i) {
            char c = text[i];
            if (c == '\'') {
                if (quoted) {
                    accumulator->append('\'');
                    quoted = false;
                }
                accumulator->append("\\'");
            } else {
                if (!quoted) {
                    accumulator->append('\'');
                    quoted = true;
                }
                accumulator->append(c);
            }
        }
        if (quoted) {
            accumulator->append('\'');
        }
    }
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
