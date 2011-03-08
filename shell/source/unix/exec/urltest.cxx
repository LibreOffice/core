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
        rtl::OString aURL( url );
        rtl::OStringBuffer aBuffer;
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
        printf( "Number of tests failing: %d\n", errors);
        ret = -3;
    }
    else
        printf( "All tests passed OK.\n" );


    return ret;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
