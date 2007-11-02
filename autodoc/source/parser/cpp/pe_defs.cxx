/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pe_defs.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:53:22 $
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
#include "pe_defs.hxx"


// NOT FULLY DECLARED SERVICES
#include <cosv/tpl/tpltools.hxx>
#include <ary/cpp/c_gate.hxx>
#include <ary/cpp/c_define.hxx>
#include <ary/cpp/c_macro.hxx>
#include <ary/cpp/cp_def.hxx>
#include "all_toks.hxx"


namespace cpp
{


PE_Defines::PE_Defines( Cpp_PE * i_pParent )
    :   Cpp_PE(i_pParent),
        pStati( new PeStatusArray<PE_Defines> ),
        // sName,
        // aParameters,
        // sDefinition,
        bIsMacro(false)
{
    Setup_StatusFunctions();
}


PE_Defines::~PE_Defines()
{
}

void
PE_Defines::Call_Handler( const cpp::Token & i_rTok )
{
    pStati->Cur().Call_Handler(i_rTok.TypeId(), i_rTok.Text());
}

void
PE_Defines::Setup_StatusFunctions()
{
    typedef CallFunction<PE_Defines>::F_Tok F_Tok;
    static F_Tok stateF_expectName[] =      { &PE_Defines::On_expectName_DefineName,
                                              &PE_Defines::On_expectName_MacroName
                                            };
    static INT16 stateT_expectName[] =      { Tid_DefineName,
                                              Tid_MacroName
                                            };

    static F_Tok stateF_gotDefineName[] =   { &PE_Defines::On_gotDefineName_PreProDefinition };
    static INT16 stateT_gotDefineName[] =   { Tid_PreProDefinition };

    static F_Tok stateF_expectMacroParameters[] =
                                            { &PE_Defines::On_expectMacroParameters_MacroParameter,
                                              &PE_Defines::On_expectMacroParameters_PreProDefinition
                                            };
    static INT16 stateT_expectMacroParameters[] =
                                               { Tid_MacroParameter,
                                              Tid_PreProDefinition
                                            };

    SEMPARSE_CREATE_STATUS(PE_Defines, expectName, Hdl_SyntaxError);
    SEMPARSE_CREATE_STATUS(PE_Defines, gotDefineName, Hdl_SyntaxError);
    SEMPARSE_CREATE_STATUS(PE_Defines, expectMacroParameters, Hdl_SyntaxError);
}

void
PE_Defines::InitData()
{
    pStati->SetCur(expectName);

    sName.clear();
    csv::erase_container( aParameters );
    csv::erase_container( aDefinition );
    bIsMacro = false;
}

void
PE_Defines::TransferData()
{
    if (NOT bIsMacro)
    {
        if (aDefinition.empty() OR aDefinition.front().empty())
            return;

        ary::cpp::Define &
            rNew = Env().AryGate().Defs().Store_Define(
                            Env().Context(), sName, aDefinition );
        Env().Event_Store_CppDefinition(rNew);
    }
    else
    {
        ary::cpp::Macro &
            rNew = Env().AryGate().Defs().Store_Macro(
                            Env().Context(), sName, aParameters, aDefinition );
        Env().Event_Store_CppDefinition(rNew);
    }
    pStati->SetCur(size_of_states);
}

void
PE_Defines::Hdl_SyntaxError( const char * i_sText)
{
    StdHandlingOfSyntaxError(i_sText);
}

void
PE_Defines::On_expectName_DefineName( const char * i_sText )
{
    SetTokenResult(done, stay);
    pStati->SetCur(gotDefineName);

    sName = i_sText;
    bIsMacro = false;
}

void
PE_Defines::On_expectName_MacroName( const char * i_sText )
{
    SetTokenResult(done, stay);
    pStati->SetCur(expectMacroParameters);

    sName = i_sText;
    bIsMacro = true;
}

void
PE_Defines::On_gotDefineName_PreProDefinition( const char * i_sText )
{
    SetTokenResult(done, pop_success);

    aDefinition.push_back( String (i_sText) );
}

void
PE_Defines::On_expectMacroParameters_MacroParameter( const char * i_sText )
{
    SetTokenResult(done, stay);
    aParameters.push_back( String (i_sText) );
}

void
PE_Defines::On_expectMacroParameters_PreProDefinition( const char * i_sText )
{
    SetTokenResult(done, pop_success);

    aDefinition.push_back( String (i_sText) );
}


}   // namespace cpp

