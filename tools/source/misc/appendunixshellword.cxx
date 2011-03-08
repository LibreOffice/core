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

#include "precompiled_tools.hxx"
#include "sal/config.h"

#if defined UNX

#include <cstddef>

#include "osl/diagnose.h"
#include "rtl/strbuf.hxx"
#include "rtl/string.h"
#include "rtl/string.hxx"
#include "sal/types.h"
#include "tools/appendunixshellword.hxx"

namespace tools {

void appendUnixShellWord(
    rtl::OStringBuffer * accumulator, rtl::OString const & text)
{
    OSL_ASSERT(accumulator != NULL);
    if (text.getLength() == 0) {
        accumulator->append(RTL_CONSTASCII_STRINGPARAM("''"));
    } else {
        bool quoted = false;
        for (sal_Int32 i = 0; i < text.getLength(); ++i) {
            char c = text[i];
            if (c == '\'') {
                if (quoted) {
                    accumulator->append('\'');
                    quoted = false;
                }
                accumulator->append(RTL_CONSTASCII_STRINGPARAM("\\'"));
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
