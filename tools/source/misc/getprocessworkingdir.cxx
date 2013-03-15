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

#include <cstddef>

#include "osl/diagnose.h"
#include "osl/file.hxx"
#include "osl/process.h"
#include "rtl/bootstrap.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "tools/getprocessworkingdir.hxx"

namespace tools {

bool getProcessWorkingDir(rtl::OUString &rUrl)
{
    rUrl = rtl::OUString();
    rtl::OUString s("$OOO_CWD");
    rtl::Bootstrap::expandMacros(s);
    if (s.isEmpty())
    {
        if (osl_getProcessWorkingDir(&rUrl.pData) == osl_Process_E_None)
            return true;
    }
    else if (s[0] == '1')
    {
        rUrl = s.copy(1);
        return true;
    }
    else if (s[0] == '2' &&
               (osl::FileBase::getFileURLFromSystemPath(s.copy(1), rUrl) ==
                osl::FileBase::E_None))
    {
        return true;
    }
    return false;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
