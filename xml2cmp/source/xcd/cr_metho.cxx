/*************************************************************************
 *
 *  $RCSfile: cr_metho.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: np $ $Date: 2001-03-23 13:24:04 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "cr_metho.hxx"

#include <string.h>
#include <fstream>
#include <iostream>


using std::cerr;
using std::ofstream;
using std::ios;


char C_sFileHeader1[] = "/* ";
char C_sFileHeader2[] = " */\r\n/* Implementation of component_getDescriptionFunc() */\r\n\r\n"
                        "#include <sal/types.h>\r\n\r\n";
char C_sFuncBegin[]   = "#ifdef __cplusplus\r\n"
                        "extern \"C\" {\r\n"
                        "#endif\r\n\r\n"
                        "const sal_Char * SAL_CALL\r\ncomponent_getDescriptionFunc()\r\n"
                        "{\r\n"
                        "    return (sal_Char*) \r\n"
                        "    \"";
char C_sFuncEnd[]    =  "\";\r\n"
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
    char   sDescrLineChange[] = "\"\r\n    \"";
    int    sDescrLen = strlen(sDescrLineChange);

    ofstream aFile(i_pOutputFileName, ios::out
#ifdef WNT
                                               | ios::binary
#endif
    );


    if ( !aFile )
    {
        cerr << "Error: " << i_pOutputFileName << " could not be created." << std::endl;
        return;
    }

    aFile.write( C_sFileHeader1, strlen(C_sFileHeader1) );
    aFile.write( i_pOutputFileName, strlen(i_pOutputFileName) );
    aFile.write( C_sFileHeader2, strlen(C_sFileHeader2) );
    aFile.write( C_sFuncBegin, strlen(C_sFuncBegin) );

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

    aFile.write( C_sFuncEnd, strlen(C_sFuncEnd) );


    aFile.close();
}



