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

#include "shellexec.hxx"

#include <osl/process.h>

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <strings.h>

// -----------------------------------------------------------------------

int main(int argc, const char *argv[])
{
    int ret = 0;

    if( argc != 2 )
    {
        fprintf(stderr, "Usage: urltest <urllist>\n");
        return -1;
    }

    FILE * fp = fopen( argv[1], "r" );
    if( NULL == fp )
    {
        perror( argv[1] );
        return -1;
    }

    // expect urltest.sh beside this binary
    char line[LINE_MAX];
    size_t len = strlen(argv[0]);
    strcpy( line, argv[0] );
    strcpy( line + len, ".sh " );
    len += 4;

    unsigned int errors = 0;

    // read url(s) to test from file
    char url[512];
    while( NULL != fgets(url, sizeof(url), fp))
    {
        // remove trailing line break
        strtok( url, "\r\n" );

        printf( "Passing URL: %s\n", url );

        // test the encoding functionality from shellexec.cxx
        OString aURL( url );
        OStringBuffer aBuffer;
        escapeForShell(aBuffer, aURL);

        // append encoded URL as (only) parameter to the script
        strcpy( line + len, aBuffer.getStr() );

        printf( "Command line: %s\n", line );

        FILE * pipe = popen( line, "r" );
        if( NULL != pipe )
        {
            char buffer[BUFSIZ];

            // initialize buffer with '\0'
            memset(buffer, '\0', BUFSIZ);

            // read the output of the script
            if(NULL == fgets( buffer, BUFSIZ, pipe))
            {
                perror("FAILED: output of script could not be read");
                printf( "\n");
                ++errors;
                continue;
            }

            // remove trailing line break again
            strtok( buffer, "\r\n" );

            int n = pclose(pipe);
            if( 0 != n )
            {
                printf("FAILED: fclose returned %d\n\n", n );
                ++errors;
                continue;
            }

            if( 0 == strcmp( url, buffer ) )
            {
                // strings are identical: good !
                printf( "OK\n\n");
            }
            else
            {
                // compare failed
                printf( "FAILED: returned string is %s\n\n", buffer);
                ++errors;
            }

        }
        else
        {
            perror( line );
            ret = -2;
            break;
        }
    }

    if( ferror( fp ) )
    {
        perror( argv[1] );
        ret = -1;
    }

    fclose( fp );

    if( errors )
    {
        printf( "Number of tests failing: %u\n", errors);
        ret = -3;
    }
    else
        printf( "All tests passed OK.\n" );


    return ret;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
