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

#include <precomp.h>
#include <x_parse.hxx>

// NOT FULLY DECLARED SERVICES



X_Parser::X_Parser( E_Event             i_eEvent,
                    const char *        i_sObject,
                    const String &      i_sCausingFile_FullPath,
                    uintt               i_nCausingLineNr  )
    :   eEvent(i_eEvent),
        sObject(i_sObject),
        sCausingFile_FullPath(i_sCausingFile_FullPath),
        nCausingLineNr(i_nCausingLineNr)
{
}

X_Parser::~X_Parser()
{
}

X_Parser::E_Event
X_Parser::GetEvent() const
{
     return eEvent;
}

void
X_Parser::GetInfo( std::ostream & o_rOutputMedium ) const
{
    o_rOutputMedium << "Error in file "
                    << sCausingFile_FullPath
                    << " in line "
                    << nCausingLineNr
                    << ": ";


    switch (eEvent)
    {
        case x_InvalidChar:
            o_rOutputMedium << "Unknown character '"
                            << sObject
                            << "'";
            break;
        case x_UnexpectedToken:
            o_rOutputMedium << "Unexpected token \""
                            << sObject
                            << "\"";
            break;
        case x_UnexpectedEOF:
            o_rOutputMedium << "Unexpected end of file.";
            break;
        case x_UnspecifiedSyntaxError:
            o_rOutputMedium << "Unspecified syntax problem in file.";
            break;
        case x_Any:
        default:
            o_rOutputMedium << "Unspecified parsing exception.";
    }   // end switch
    o_rOutputMedium << Endl();
}


std::ostream &
operator<<( std::ostream &                  o_rOut,
            const autodoc::X_Parser_Ifc &   i_rException )
{
    i_rException.GetInfo(o_rOut);
     return o_rOut;
}
