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
#include <adoc/tk_docw.hxx>


// NOT FULLY DEFINED SERVICES
#include <adoc/tokintpr.hxx>



namespace adoc {


    static const char C_sSpace[300] =
        "                                         "
        "                                         "
        "                                         "
        "                                         "
        "                                         "
        "                                        ";


//***********************   Tok_DocWord     ******************//

void
Tok_DocWord::Trigger( TokenInterpreter & io_rInterpreter ) const
{
    io_rInterpreter.Hdl_DocWord(*this);
}

const char *
Tok_DocWord::Text() const
{
    return sText;
}

//***********************   Tok_Whitespace     ******************//


void
Tok_Whitespace::Trigger( TokenInterpreter & io_rInterpreter ) const
{
    io_rInterpreter.Hdl_Whitespace(*this);
}

const char *
Tok_Whitespace::Text() const
{
    return C_sSpace + 299 - nSize;
}



//***********************   Tok_LineStart     ******************//


void
Tok_LineStart::Trigger( TokenInterpreter &  io_rInterpreter ) const
{
    io_rInterpreter.Hdl_LineStart(*this);
}

const char *
Tok_LineStart::Text() const
{
    return C_sSpace + 299 - nSize;
}


//***********************   Tok_Eol     ******************//

void
Tok_Eol::Trigger( TokenInterpreter & io_rInterpreter ) const
{
    io_rInterpreter.Hdl_Eol(*this);
}

const char *
Tok_Eol::Text() const
{
    return "\n";
}



//***********************   Tok_EoDocu     ******************//

void
Tok_EoDocu::Trigger( TokenInterpreter & io_rInterpreter ) const
{
    io_rInterpreter.Hdl_EoDocu(*this);
}

const char *
Tok_EoDocu::Text() const
{
    return "*/";
}

}   // namespace adoc


