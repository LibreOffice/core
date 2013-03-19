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

#include "diagnose.hxx"
#include <stdio.h>
#include <stdarg.h>
#include "rtl/instance.hxx"
#include "rtl/bootstrap.hxx"

namespace xmlsecurity {

struct UseDiagnose : public rtl::StaticWithInit< bool, UseDiagnose>
{
    bool operator () () const
    {
        ::rtl::OUString value;
        sal_Bool res = rtl::Bootstrap::get(
            ::rtl::OUString("XMLSECURITY_TRACE"), value);
        return res == sal_True ? true : false;
    }
};

/* the function will print the string when
   - build with debug
   - the bootstrap variable XMLSECURITY_TRACE is set.
 */
void xmlsec_trace(const char* pszFormat, ...)
{
    bool bDebug = false;

#if OSL_DEBUG_LEVEL > 1
    bDebug = true;
#endif
    if (bDebug || UseDiagnose::get())
    {
        va_list args;
        fprintf(stderr, "[xmlsecurity] ");
        va_start(args, pszFormat);
        vfprintf(stderr, pszFormat, args);
        va_end(args);

        fprintf(stderr,"\n");
        fflush(stderr);
    }
}



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
