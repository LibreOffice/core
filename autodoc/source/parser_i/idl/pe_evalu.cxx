/*************************************************************************
 *
 *  $RCSfile: pe_evalu.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:34 $
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
#include <s2_luidl/pe_evalu.hxx>

// NOT FULLY DECLARED SERVICES
#include <ary_i/codeinf2.hxx>
#include <s2_luidl/tk_ident.hxx>
#include <s2_luidl/tk_punct.hxx>
#include <s2_luidl/tk_const.hxx>


namespace csi
{
namespace uidl
{


#ifdef DF
#undef DF
#endif
#define DF  PE_Value::On_Default

PE_Value::F_TOK
PE_Value::aDispatcher[PE_Value::e_STATES_MAX][PE_Value::tt_MAX] =
        {   { DF, DF, DF },  // e_none
            { On_expect_name_Identifier,
                  DF, DF },  // expect_name
            { DF, On_got_name_Punctuation,
                      On_got_name_Assignment }  // got_name
        };



inline void
PE_Value::CallHandler( const char *     i_sTokenText,
                       E_TokenType      i_eTokenType )
    { (this->*aDispatcher[eState][i_eTokenType])(i_sTokenText); }





PE_Value::PE_Value( udmstri &   o_rName,
                    udmstri &   o_rAssignment,
                    bool        i_bIsConst )
    :   eState(e_none),
        pName(&o_rName),
        pAssignment(&o_rAssignment),
        bIsConst(i_bIsConst)
{
}

void
PE_Value::EstablishContacts( UnoIDL_PE *                io_pParentPE,
                             ary::Repository &          io_rRepository,
                             TokenProcessing_Result &   o_rResult )
{
    UnoIDL_PE::EstablishContacts(io_pParentPE,io_rRepository,o_rResult);
}

PE_Value::~PE_Value()
{
}

void
PE_Value::ProcessToken( const Token & i_rToken )
{
    i_rToken.Trigger(*this);
}

void
PE_Value::Process_Identifier( const TokIdentifier & i_rToken )
{
    CallHandler(i_rToken.Text(), tt_identifier);
}

void
PE_Value::Process_Punctuation( const TokPunctuation & i_rToken )
{
    CallHandler(i_rToken.Text(), tt_punctuation);
}

void
PE_Value::Process_Assignment( const TokAssignment & i_rToken )
{
    CallHandler(i_rToken.Text(), tt_assignment);
}

void
PE_Value::On_expect_name_Identifier(const char * i_sText)
{
    *pName = i_sText;
    SetResult(done,stay);
    eState = got_name;
}

void
PE_Value::On_got_name_Punctuation(const char * i_sText)
{
    if ( i_sText[0] == ',' AND NOT IsConst()
         OR i_sText[0] == ';' AND IsConst() )
    {
        SetResult(done,pop_success);
        eState = e_none;
    }
    else if (i_sText[0] == '}' AND NOT IsConst())
    {
        SetResult(not_done,pop_success);
        eState = e_none;
    }
    else
        On_Default(i_sText);
}

void
PE_Value::On_got_name_Assignment(const char * i_sText)
{
    *pAssignment = i_sText;
    SetResult(done,pop_success);
    eState = e_none;
}

void
PE_Value::On_Default(const char * )
{
    SetResult(not_done,pop_failure);
}

void
PE_Value::InitData()
{
    eState = expect_name;

    *pName = "";
    *pAssignment = "";
}

void
PE_Value::TransferData()
{
    csv_assert(pName->length() > 0);
    eState = e_none;
}

UnoIDL_PE &
PE_Value::MyPE()
{
    return *this;
}

}   // namespace uidl
}   // namespace csi

