/*************************************************************************
 *
 *  $RCSfile: pe_defs.cxx,v $
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
#include "pe_defs.hxx"


// NOT FULLY DECLARED SERVICES
#include <cosv/template/tpltools.hxx>
#include <ary/cpp/c_rwgate.hxx>
#include <ary/cpp/c_define.hxx>
#include <ary/cpp/c_macro.hxx>
#include <ary/cpp/crwg_def.hxx>
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
        ary::cpp::Define &
            rNew = Env().AryGate().Defines().Store_Define(
                            Env().Context(), sName, aDefinition );
        Env().Event_Store_CppDefinition(rNew);
    }
    else
    {
        ary::cpp::Macro &
            rNew = Env().AryGate().Defines().Store_Macro(
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

    aDefinition.push_back( udmstri(i_sText) );
}

void
PE_Defines::On_expectMacroParameters_MacroParameter( const char * i_sText )
{
    SetTokenResult(done, stay);
    aParameters.push_back( udmstri(i_sText) );
}

void
PE_Defines::On_expectMacroParameters_PreProDefinition( const char * i_sText )
{
    SetTokenResult(done, pop_success);

    aDefinition.push_back( udmstri(i_sText) );
}


}   // namespace cpp

