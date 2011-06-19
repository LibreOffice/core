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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"


#include "rtl/ustring.hxx"
#include "osl/time.h"
#include <stdio.h>


/*
 *   mfe : maybe it would be wishful to include initialization
 *         of the global timer in dllmain or _init directly.
 *         But noneoftheless this (should) work too.
 */
namespace osl
{

class OGlobalTimer
{

public:

    OGlobalTimer() {
        getTime();
    }

    sal_uInt32 getTime()
    {
        return osl_getGlobalTimer();
    }


};

static OGlobalTimer aGlobalTimer;

}


extern "C"
{
void debug_ustring(rtl_uString* ustr)
{
    sal_Char* psz=0;
    rtl_String* str=0;

    if ( ustr != 0 )
    {
        rtl_uString2String( &str,
                            rtl_uString_getStr(ustr),
                            rtl_uString_getLength(ustr),
                            RTL_TEXTENCODING_UTF8,
                            OUSTRING_TO_OSTRING_CVTFLAGS );

        psz = rtl_string_getStr(str);
    }

    fprintf(stderr,"'%s'\n",psz);

    if ( str != 0 )
    {
        rtl_string_release(str);
    }

    return;
}

}

void debug_oustring(rtl::OUString& ustr)
{

    debug_ustring(ustr.pData);

    return;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
