/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
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
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("XMLSECURITY_TRACE")), value);
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
