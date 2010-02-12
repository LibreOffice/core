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
#include "precompiled_testshl2.hxx"

#include <string>
#include <stdarg.h>
#include <stdio.h>

void t_print(const char* _pFormatStr, va_list &args)
{
    vprintf(_pFormatStr, args);
}

void t_print(const char* _pFormatStr, ...)
{
    va_list  args;
    va_start( args, _pFormatStr );
    t_print(_pFormatStr, args);
    va_end( args );
}


// ----------------------------------- Main -----------------------------------
#if (defined UNX) || (defined OS2)
int main( int /* argc */ , char* /* argv */ [] )
#else
    int _cdecl main( int /* argc */ , char* /* argv */ [] )
#endif
{
    std::string sStr("TestString.");
    printf("Test %s\n", sStr.c_str() );
    t_print("Test %s\n", sStr.c_str() );
    return 0;
}
