/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pe_namsp.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:55:58 $
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

#include <precomp.h>
#include <pe_namsp.hxx>


// NOT FULLY DECLARED SERVICES
#include <all_toks.hxx>
#include <ary/cpp/c_namesp.hxx>
#include <ary/cpp/c_gate.hxx>
#include <ary/cpp/cp_ce.hxx>
#include <semantic/callf.hxx>
#include "x_parse.hxx"




namespace cpp
{

PE_Namespace::PE_Namespace( Cpp_PE * i_pParent )
    :   Cpp_PE(i_pParent),
        pStati( new PeStatusArray<PE_Namespace> ),
        // sLocalname
        bPush(false)
{
    Setup_StatusFunctions();
}

PE_Namespace::~PE_Namespace()
{
}

void
PE_Namespace::Setup_StatusFunctions()
{
    typedef CallFunction<PE_Namespace>::F_Tok   F_Tok;
    static F_Tok stateF_start[] =           { &PE_Namespace::On_start_Identifier,
                                              &PE_Namespace::On_start_SwBracket_Left };
    static INT16 stateT_start[] =           { Tid_Identifier,
                                              Tid_SwBracket_Left };
    static F_Tok stateF_gotName[] =         { &PE_Namespace::On_gotName_SwBracket_Left,
                                              &PE_Namespace::On_gotName_Assign };
    static INT16 stateT_gotName[] =         { Tid_SwBracket_Left,
                                              Tid_Assign };
    static F_Tok stateF_expectSemicolon[] = { &PE_Namespace::On_expectSemicolon_Semicolon };
    static INT16 stateT_expectSemicolon[] = { Tid_Semicolon };

    SEMPARSE_CREATE_STATUS(PE_Namespace, start, Hdl_SyntaxError);
    SEMPARSE_CREATE_STATUS(PE_Namespace, gotName, Hdl_SyntaxError);
    SEMPARSE_CREATE_STATUS(PE_Namespace, expectSemicolon, Hdl_SyntaxError);
}

void
PE_Namespace::Call_Handler( const cpp::Token &  i_rTok )
{
    pStati->Cur().Call_Handler(i_rTok.TypeId(), i_rTok.Text());
}

void
PE_Namespace::InitData()
{
    pStati->SetCur(start);
    sLocalName = "";
    bPush = false;
}

void
PE_Namespace::TransferData()
{
    if (bPush)
    {
        ary::cpp::Namespace &
            rNew = Env().AryGate().Ces().CheckIn_Namespace(
                                             Env().Context(),
                                             sLocalName );
        Env().OpenNamespace(rNew);
    }
}

void
PE_Namespace::Hdl_SyntaxError( const char * i_sText)
{
    throw X_Parser( X_Parser::x_UnexpectedToken,
                    i_sText != 0 ? i_sText : "",
                    Env().CurFileName(),
                    Env().LineCount() );
}

void
PE_Namespace::On_start_Identifier(const char * i_sText)
{
    SetTokenResult(done, stay);
    pStati->SetCur(gotName);

    sLocalName = i_sText;
}

void
PE_Namespace::On_start_SwBracket_Left(const char * )
{
    SetTokenResult(done, pop_success);
    pStati->SetCur(size_of_states);

    sLocalName = "";    // Anonymous namespace, a name is created in
                        //   Gate().CheckIn_Namespace() .

    bPush = true;
}

void
PE_Namespace::On_gotName_SwBracket_Left(const char * )
{
    SetTokenResult(done, pop_success);
    pStati->SetCur(size_of_states);

    bPush = true;
}

void
PE_Namespace::On_gotName_Assign(const char * )
{
    // KORR_FUTURE
    Hdl_SyntaxError(0);
}

void
PE_Namespace::On_expectSemicolon_Semicolon(const char * )
{
    SetTokenResult(done,pop_success);
    pStati->SetCur(size_of_states);
}

}   // namespace cpp




