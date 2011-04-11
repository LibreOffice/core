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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "osl/thread.h"
#include "osl/file.h"
#include "rtl/ustring.hxx"
#include  "rtl/ustrbuf.h"

static sal_Bool hasOption(char const * szOption, int argc, char** argv);


#define HELP_TEXT    \
"SYNOPSIS \n\n" \
"\tsp2bv [-h] [-?] string \n\n" \
"DESCRIPTION\n\n" \
"\tsp2bv stands for \"system path to bootstrap variable\"." \
" First the system path is converted into a file URL. Then all " \
"characters which have a special meaning in bootstrap variables, " \
"such as \'$\' are escaped. The resulting string is written to " \
"stdout an can be assigned to a bootstrap variable.\n" \
"\n\n" \
"OPTIONS \n\n" \
"\tThe following options are supported: \n" \
"-?\n " \
"--help" \
"    Display help information.\n"




int main(int argc, char **argv)
{
    if( hasOption("--help",argc, argv) || hasOption("-h", argc, argv))
    {
        fprintf(stdout, HELP_TEXT);// default
        return 0;
    }

    if (argc != 2)
    {
        fprintf(stdout, HELP_TEXT);
        return -1;
    }

    rtl_uString* pPath = NULL;
    rtl_string2UString( &pPath, argv[1], strlen(argv[1]),
                        osl_getThreadTextEncoding(),OSTRING_TO_OUSTRING_CVTFLAGS );

    rtl_uString* pUrl = NULL;
    if (osl_getFileURLFromSystemPath(pPath, &pUrl) != osl_File_E_None)
        return -1;
//escape the special characters

    sal_Unicode cEscapeChar = 0x5c;
    rtl_uString* pBuffer = NULL;
    sal_Int32 nCapacity = 255;
    rtl_uString_new_WithLength( &pBuffer, nCapacity );

    const sal_Unicode* pCur = pUrl->buffer;
    for (int i = 0; i != pUrl->length; i++)
    {
        switch( *pCur)
        {
        case '$':
            rtl_uStringbuffer_insert( &pBuffer, &nCapacity, pBuffer->length, &cEscapeChar, 1);
            rtl_uStringbuffer_insert( &pBuffer, &nCapacity, pBuffer->length, pCur, 1);
            break;
        case '{':
        case '}':
        case '\\': fprintf(stderr, "sp2vb: file URL contains invalid characters!\n");
            return -1;
        default:
            rtl_uStringbuffer_insert( &pBuffer, &nCapacity, pBuffer->length, pCur, 1);
        }
        pCur ++;
    }
//convert back to byte string so that we can print it.
    rtl_String* pBootVar = NULL;
    rtl_uString2String( &pBootVar, pBuffer->buffer, pBuffer->length,
                        osl_getThreadTextEncoding(), OUSTRING_TO_OSTRING_CVTFLAGS);

    fprintf(stdout, "%s", pBootVar->buffer);
    fflush(stdout);

    rtl_uString_release(pBuffer);
    rtl_uString_release(pPath);
    rtl_uString_release(pUrl);
    rtl_string_release(pBootVar);
    return 0;
}



static sal_Bool hasOption(char const * szOption, int argc, char** argv)
{
    sal_Bool retVal= sal_False;
    for(sal_Int16 i= 1; i < argc; i++)
    {
        if( ! strcmp(argv[i], szOption))
        {
            retVal= sal_True;
            break;
        }
    }
    return retVal;
}





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
