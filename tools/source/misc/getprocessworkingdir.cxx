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

#include <cstddef>

#include "osl/diagnose.h"
#include "osl/file.hxx"
#include "osl/process.h"
#include "rtl/bootstrap.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "tools/getprocessworkingdir.hxx"

namespace tools {

bool getProcessWorkingDir(rtl::OUString * url) {
    OSL_ASSERT(url != NULL);
    rtl::OUString s(RTL_CONSTASCII_USTRINGPARAM("$OOO_CWD"));
    rtl::Bootstrap::expandMacros(s);
    if (s.getLength() == 0) {
        if (osl_getProcessWorkingDir(&url->pData) == osl_Process_E_None) {
            return true;
        }
    } else if (s[0] == '1') {
        *url = s.copy(1);
        return true;
    } else if (s[0] == '2' &&
               (osl::FileBase::getFileURLFromSystemPath(s.copy(1), *url) ==
                osl::FileBase::E_None))
    {
        return true;
    }
    *url = rtl::OUString();
    return false;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
