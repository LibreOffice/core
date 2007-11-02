/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cr_metho.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 12:56:37 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
//          case '\t':  aFile.write( "\\t", 2);
//                      break;
            default:    aFile.write( pTrans, 1);
        }
    }   /* end for */

    aFile.write( C_sFuncEnd, (int) strlen(C_sFuncEnd) );


    aFile.close();
}


