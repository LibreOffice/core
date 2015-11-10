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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "osl/thread.h"
#include "osl/file.h"
#include "rtl/ustring.hxx"
#include "rtl/ustrbuf.h"

static bool hasOption(char const * szOption, int argc, char** argv);


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

    rtl_uString* pPath = nullptr;
    rtl_string2UString( &pPath, argv[1], strlen(argv[1]),
                        osl_getThreadTextEncoding(),OSTRING_TO_OUSTRING_CVTFLAGS );

    rtl_uString* pUrl = nullptr;
    if (osl_getFileURLFromSystemPath(pPath, &pUrl) != osl_File_E_None)
        return -1;
//escape the special characters

    sal_Unicode cEscapeChar = 0x5c;
    rtl_uString* pBuffer = nullptr;
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
    rtl_String* pBootVar = nullptr;
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



static bool hasOption(char const * szOption, int argc, char** argv)
{
    bool retVal = false;
    for(int i= 1; i < argc; i++)
    {
        if( ! strcmp(argv[i], szOption))
        {
            retVal = true;
            break;
        }
    }
    return retVal;
}





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
