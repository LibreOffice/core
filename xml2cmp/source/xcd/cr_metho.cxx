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

#include "cr_metho.hxx"

#include <string.h>
#include <fstream>
#include <iostream>



const char C_sFileHeader1[]
                      = "/* ";
const char C_sFileHeader2[]
                      = " */\r\n/* Implementation of component_getDescriptionFunc() */\r\n\r\n"
                        "#include <sal/types.h>\r\n\r\n";
const char C_sFuncBegin[]
                      = "#ifdef __cplusplus\r\n"
                        "extern \"C\" {\r\n"
                        "#endif\r\n\r\n"
                        "const sal_Char * SAL_CALL\r\ncomponent_getDescriptionFunc()\r\n"
                        "{\r\n"
                        "    return (const sal_Char*) \r\n"
                        "    \"";
const char C_sFuncEnd[]    =  "\";\r\n"
                        "}\r\n\r\n"
                        "#ifdef __cplusplus\r\n"
                        "} /* end of extern \"C\" */\r\n"
                        "#endif\r\n";


void
Create_AccessMethod( const char *           i_pOutputFileName,
                     const char *           i_sText )
{
    const char * pText = i_sText;
    const char * pTrans = 0;
    const char   sDescrLineChange[] = "\"\r\n    \"";
    int    sDescrLen = (int) strlen(sDescrLineChange);

    std::ofstream aFile(i_pOutputFileName, std::ios::out
#if defined(WNT) || defined(OS2)
                                               | std::ios::binary
#endif
    );


    if ( !aFile )
    {
        std::cerr << "Error: " << i_pOutputFileName << " could not be created." << std::endl;
        return;
    }

    aFile.write( C_sFileHeader1, (int) strlen(C_sFileHeader1) );
    aFile.write( i_pOutputFileName, (int) strlen(i_pOutputFileName) );
    aFile.write( C_sFileHeader2, (int) strlen(C_sFileHeader2) );
    aFile.write( C_sFuncBegin, (int) strlen(C_sFuncBegin) );

    for ( pTrans = pText; *pTrans != '\0'; pTrans++ )
    {
        switch (*pTrans)
        {
            case '"':   aFile.write( "\\\"", 2);
                        break;
            case '\n':  aFile.write( "\\n", 2);
                        aFile.write( sDescrLineChange, sDescrLen);
                        break;
            case '\r':  aFile.write( "\\r", 2);
                        break;
            default:    aFile.write( pTrans, 1);
        }
    }   /* end for */

    aFile.write( C_sFuncEnd, (int) strlen(C_sFuncEnd) );


    aFile.close();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
