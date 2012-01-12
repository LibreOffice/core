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
#include <x_parse2.hxx>

// NOT FULLY DECLARED SERVICES

    enum E_Type
    {
        x_Any                       = 0,
        x_InvalidChar,
        x_UnexpectedEOF
    };
void
X_AutodocParser::GetInfo( std::ostream &        o_rOutputMedium ) const
{
    switch (eType)
    {
        case x_Any:
            o_rOutputMedium << "Unspecified parsing exception ." << Endl();
            break;
        case x_InvalidChar:
            o_rOutputMedium << "Unknown character during parsing." << Endl();
            break;
        case x_UnexpectedToken:
            o_rOutputMedium << "Unexpected token " << sName << " found." << Endl();
            break;
        case x_UnexpectedEOF:
            o_rOutputMedium << "Unexpected end of file found." << Endl();
            break;
        default:
            o_rOutputMedium << "Unknown exception during parsing." << Endl();
    }
}


