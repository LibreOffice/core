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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>

// -----------------------------------------------------------------------------
// taken from setup2
std::string getTempPath()
{
    std::string sTempDir;

    char* pTmp = getenv( "TEMP" );
    if (pTmp == NULL) pTmp = getenv("temp");
    if (pTmp == NULL) pTmp = getenv("TMP");
    if (pTmp == NULL) pTmp = getenv("tmp");

    if( pTmp && strlen(pTmp) >= 2 )
    {
        sTempDir = std::string( pTmp );
    }
    else
    {
#if (defined UNX) || (defined OS2)
        int nLen;
        pTmp = P_tmpdir;
        nLen = strlen(pTmp);
        if (pTmp[ nLen - 1] == '/')
        {
            char cBuf[256];
            char* pBuf = cBuf;
            strncpy( pBuf, pTmp, nLen - 1 );
            pBuf[nLen - 1] = '\0';
            sTempDir = std::string( pBuf );
        }
        else
        {
            sTempDir = std::string( pTmp );
        }
#else
        fprintf(stderr, "error: No temp dir found.\n");
#endif
    }
    return sTempDir;
}
