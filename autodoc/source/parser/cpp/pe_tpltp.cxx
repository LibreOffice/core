/*************************************************************************
 *
 *  $RCSfile: pe_tpltp.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: np $ $Date: 2002-05-07 18:32:24 $
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
#include "pe_tpltp.hxx"


// NOT FULLY DECLARED SERVICES
#include <cosv/template/tpltools.hxx>



namespace cpp {



PE_TemplateTop::PE_TemplateTop( Cpp_PE * i_pParent )
    :   Cpp_PE(i_pParent),
        pStati( new PeStatusArray<PE_TemplateTop> ),
        // aResult_Parameters,
        bCurIsConstant(false)
{
        Setup_StatusFunctions();
}


PE_TemplateTop::~PE_TemplateTop()
{
}

void
PE_TemplateTop::Call_Handler( const cpp::Token & i_rTok )
{
    pStati->Cur().Call_Handler(i_rTok.TypeId(), i_rTok.Text());
}

void
PE_TemplateTop::Setup_StatusFunctions()
{
    typedef CallFunction<PE_TemplateTop>::F_Tok F_Tok;

    static F_Tok stateF_start[] =           { &PE_TemplateTop::On_start_Less };
    static INT16 stateT_start[] =           { Tid_Less };

                                            // KORR_FUTURE : add "typename".
    static F_Tok stateF_expect_qualifier[]= { &PE_TemplateTop::On_expect_qualifier_class };
    static INT16 stateT_expect_qualifier[]= { Tid_class };

    static F_Tok stateF_expect_name[] =     { &PE_TemplateTop::On_expect_name_Identifier };
    static INT16 stateT_expect_name[] =     { Tid_Identifier };

    static F_Tok stateF_expect_separator[]= { &PE_TemplateTop::On_expect_separator_Comma,
                                              &PE_TemplateTop::On_expect_separator_Greater };
    static INT16 stateT_expect_separator[]= { Tid_Comma,
                                              Tid_Greater };

    SEMPARSE_CREATE_STATUS(PE_TemplateTop, start,            Hdl_SyntaxError);
    SEMPARSE_CREATE_STATUS(PE_TemplateTop, expect_qualifier, On_expect_qualifier_Other);
    SEMPARSE_CREATE_STATUS(PE_TemplateTop, expect_name,      Hdl_SyntaxError);
    SEMPARSE_CREATE_STATUS(PE_TemplateTop, expect_separator, Hdl_SyntaxError);
}

void
PE_TemplateTop::InitData()
{
    pStati->SetCur(start);
    csv::erase_container(aResult_Parameters);
    bCurIsConstant = false;
}

void
PE_TemplateTop::TransferData()
{
    pStati->SetCur(size_of_states);
}

void
PE_TemplateTop::Hdl_SyntaxError(const char * i_sText)
{
    StdHandlingOfSyntaxError(i_sText);
}

void
PE_TemplateTop::On_start_Less( const char *)
{
    SetTokenResult(done, stay);
    pStati->SetCur(expect_qualifier);
}

void
PE_TemplateTop::On_expect_qualifier_class( const char *)
{
    SetTokenResult(done, stay);
    pStati->SetCur(expect_name);
}

void
PE_TemplateTop::On_expect_qualifier_Other( const char *)
{
    SetTokenResult(done, stay);
    pStati->SetCur(expect_name);

    bCurIsConstant = true;
}

void
PE_TemplateTop::On_expect_name_Identifier( const char * i_sText)
{
    SetTokenResult(done, stay);
    pStati->SetCur(expect_separator);

    if ( NOT bCurIsConstant )
    {
        udmstri sText( StreamLock(50)() << "typename " << i_sText << c_str );
        aResult_Parameters.push_back(sText);
    }
    else //
    {
        udmstri sText( StreamLock(50)() << "constant " << i_sText << c_str );
        aResult_Parameters.push_back(sText);
        bCurIsConstant = false;
    }  // endif
}

void
PE_TemplateTop::On_expect_separator_Comma( const char *)
{
    SetTokenResult(done, stay);
    pStati->SetCur(expect_qualifier);
}

void
PE_TemplateTop::On_expect_separator_Greater( const char *)
{
    SetTokenResult(done, pop_success);
}


}   // namespace cpp






