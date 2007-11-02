/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pe_tydf2.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 17:09:25 $
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
#include <s2_luidl/pe_tydf2.hxx>

// NOT FULLY DECLARED SERVICES
#include <ary/idl/i_gate.hxx>
#include <ary/idl/i_typedef.hxx>
#include <ary/idl/ip_ce.hxx>
#include <ary/doc/d_oldidldocu.hxx>
#include <s2_luidl/pe_type2.hxx>
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
#define DF  &PE_Typedef::On_Default

PE_Typedef::F_TOK
PE_Typedef::aDispatcher[PE_Typedef::e_STATES_MAX][PE_Typedef::tt_MAX] =
        {   { DF, DF, DF },  // e_none
            { &PE_Typedef::On_expect_description_Any,
                  &PE_Typedef::On_expect_description_Any,
                      DF },  // expect_description
            { DF, &PE_Typedef::On_expect_name_Identifier,
                      DF },  // expect_name
            { DF, DF, &PE_Typedef::On_got_name_Punctuation }  // got_name
        };



inline void
PE_Typedef::CallHandler( const char *       i_sTokenText,
                         E_TokenType        i_eTokenType )
    { (this->*aDispatcher[eState][i_eTokenType])(i_sTokenText); }





PE_Typedef::PE_Typedef()
    :   eState(e_none),
        pPE_Type(0),
        nType(0),
        sName()
{
    pPE_Type = new PE_Type(nType);
}

void
PE_Typedef::EstablishContacts( UnoIDL_PE *              io_pParentPE,
                               ary::Repository &    io_rRepository,
                               TokenProcessing_Result & o_rResult )
{
    UnoIDL_PE::EstablishContacts(io_pParentPE,io_rRepository,o_rResult);
    pPE_Type->EstablishContacts(this,io_rRepository,o_rResult);
}

PE_Typedef::~PE_Typedef()
{
}

void
PE_Typedef::ProcessToken( const Token & i_rToken )
{
    i_rToken.Trigger(*this);
}

void
PE_Typedef::Process_Identifier( const TokIdentifier & i_rToken )
{
    CallHandler(i_rToken.Text(), tt_identifier);
}

void
PE_Typedef::Process_Punctuation( const TokPunctuation & i_rToken )
{
    CallHandler(i_rToken.Text(), tt_punctuation);
}

void
PE_Typedef::Process_Default()
{
    CallHandler("", tt_any);
}

void
PE_Typedef::On_expect_description_Any(const char *)
{
    SetResult(not_done,push_sure, pPE_Type.Ptr());
}

void
PE_Typedef::On_expect_name_Identifier(const char * i_sText)
{
    sName = i_sText;
    SetResult(done,stay);
    eState = got_name;
}

void
PE_Typedef::On_got_name_Punctuation(const char * i_sText)
{
    if ( i_sText[0] == ';' )
    {
        SetResult(done,pop_success);
        eState = e_none;
    }
    else
        On_Default(i_sText);
}

void
PE_Typedef::On_Default(const char * )
{
    SetResult(not_done,pop_failure);
}

void
PE_Typedef::InitData()
{
    eState = expect_description;
    nType = 0;
    sName = "";
}

void
PE_Typedef::ReceiveData()
{
    eState = expect_name;
}

void
PE_Typedef::TransferData()
{
    ary::idl::Typedef &
        rCe = Gate().Ces().Store_Typedef(CurNamespace().CeId(), sName, nType);
    PassDocuAt(rCe);
    eState = e_none;
}

UnoIDL_PE &
PE_Typedef::MyPE()
{
    return *this;
}

}   // namespace uidl
}   // namespace csi

