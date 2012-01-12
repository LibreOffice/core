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
#include "pe_ignor.hxx"


// NOT FULLY DECLARED SERVICES


namespace cpp {



PE_Ignore::PE_Ignore( Cpp_PE * i_pParent )
    :   Cpp_PE(i_pParent),
        nBracketCounter(0),
        bBlockOpened(false)
{
    Setup_StatusFunctions();
}


PE_Ignore::~PE_Ignore()
{
}

void
PE_Ignore::Call_Handler( const cpp::Token & i_rTok )
{
    if ( NOT bBlockOpened )
    {
        switch (i_rTok.TypeId())
        {
            case Tid_SwBracket_Left:    SetTokenResult(done, stay);
                                        nBracketCounter++;
                                        bBlockOpened = true;
                                        break;
            case Tid_Semicolon:         SetTokenResult(done, pop_success);
                                        break;
            default:
                                        SetTokenResult(done, stay);
        }   // end switch
    }
    else if ( nBracketCounter > 0 )
    {
        SetTokenResult(done, stay);

        switch (i_rTok.TypeId())
        {
            case Tid_SwBracket_Left:    nBracketCounter++;
                                        break;
            case Tid_SwBracket_Right:   nBracketCounter--;
                                        break;
        }   // end switch
    }
    else if ( i_rTok.TypeId() == Tid_Semicolon )
    {
        SetTokenResult(done, pop_success);
    }
    else
    {
        SetTokenResult(not_done, pop_success);
    }
}

void
PE_Ignore::Setup_StatusFunctions()
{
    // Does nothing.
}

void
PE_Ignore::InitData()
{
    nBracketCounter = 0;
    bBlockOpened = false;
}

void
PE_Ignore::TransferData()
{
    // Does nothing.
}


}   // namespace cpp





