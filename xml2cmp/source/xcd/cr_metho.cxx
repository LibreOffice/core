/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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


