/*************************************************************************
 *
 *  $RCSfile: pe_expr.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: np $ $Date: 2002-05-14 09:02:19 $
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

#include <precomp.h>
#include "pe_expr.hxx"


// NOT FULLY DECLARED SERVICES


namespace cpp {



PE_Expression::PE_Expression( Cpp_PE * i_pParent )
    :   Cpp_PE(i_pParent),
        pStati( new PeStatusArray<PE_Expression> ),
        aResult_Text(100),
        nBracketCounter(0)
{
    Setup_StatusFunctions();
}


PE_Expression::~PE_Expression()
{
}

void
PE_Expression::Call_Handler( const cpp::Token & i_rTok )
{
    pStati->Cur().Call_Handler(i_rTok.TypeId(), i_rTok.Text());

#if 0
    switch (i_rTok.TypeId())
    {
        case Tid_SwBracket_Left:    SetTokenResult(done, stay);
                                    nBracketCounter++;
                                    bBlockOpened = true;
                                    break;
        case Tid_SwBracket_Right:   SetTokenResult(done, stay);
                                    nBracketCounter--;
                                    break;
        case Tid_Semicolon:         if (nBracketCounter == 0)
                                        SetTokenResult(done, pop_success);
                                    else
                                        SetTokenResult(done, stay);
                                    break;
        default:
                                    if  ( bBlockOpened AND nBracketCounter == 0 )
                                    {
                                        SetTokenResult(not_done, pop_success);
                                    }
                                    else
                                    {
                                        SetTokenResult(done, stay);
                                    }
    }   // end switch
#endif // 0
}

void
PE_Expression::Setup_StatusFunctions()
{
    typedef CallFunction<PE_Expression>::F_Tok  F_Tok;

    static F_Tok stateF_std[] =             { &PE_Expression::On_std_SwBracket_Left,
                                              &PE_Expression::On_std_SwBracket_Right,
                                              &PE_Expression::On_std_ArrayBracket_Left,
                                              &PE_Expression::On_std_ArrayBracket_Right,
                                              &PE_Expression::On_std_Bracket_Left,
                                              &PE_Expression::On_std_Bracket_Right,
                                              &PE_Expression::On_std_Semicolon,
                                              &PE_Expression::On_std_Comma };
    static INT16 stateT_std[] =             { Tid_SwBracket_Left,
                                              Tid_SwBracket_Right,
                                              Tid_ArrayBracket_Left,
                                              Tid_ArrayBracket_Right,
                                              Tid_Bracket_Left,
                                              Tid_Bracket_Right,
                                              Tid_Semicolon,
                                              Tid_Comma };

    SEMPARSE_CREATE_STATUS(PE_Expression, std, On_std_Default);
}

void
PE_Expression::InitData()
{
    pStati->SetCur(std);
    aResult_Text.seekp(0);
    nBracketCounter = 0;
}

void
PE_Expression::TransferData()
{
    pStati->SetCur(size_of_states);
    if ( aResult_Text.tellp() > 0)
        aResult_Text.pop_back(1);
}

void
PE_Expression::On_std_Default( const char * i_sText)
{
    SetTokenResult(done, stay);
    aResult_Text << i_sText << " ";
}

void
PE_Expression::On_std_SwBracket_Left( const char *)
{
    SetTokenResult(done, stay);
    nBracketCounter++;
}

void
PE_Expression::On_std_SwBracket_Right( const char *)
{
    nBracketCounter--;
    if ( nBracketCounter >= 0 )
        SetTokenResult(done, stay);
    else
        SetTokenResult(not_done, pop_success);
}

void
PE_Expression::On_std_ArrayBracket_Left( const char *)
{
    SetTokenResult(done, stay);
    nBracketCounter++;
}

void
PE_Expression::On_std_ArrayBracket_Right( const char *)
{
    nBracketCounter--;
    if ( nBracketCounter >= 0 )
        SetTokenResult(done, stay);
    else
        SetTokenResult(not_done, pop_success);
}

void
PE_Expression::On_std_Bracket_Left( const char *)
{
    SetTokenResult(done, stay);
    nBracketCounter++;
}

void
PE_Expression::On_std_Bracket_Right( const char *)
{
    nBracketCounter--;
    if ( nBracketCounter >= 0 )
        SetTokenResult(done, stay);
    else
        SetTokenResult(not_done, pop_success);
}

void
PE_Expression::On_std_Semicolon( const char *)
{
    SetTokenResult(not_done, pop_success);
}

void
PE_Expression::On_std_Comma( const char *)
{
    SetTokenResult(not_done, pop_success);
}


}   // namespace cpp






