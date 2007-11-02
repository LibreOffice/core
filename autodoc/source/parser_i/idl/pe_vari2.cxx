/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pe_vari2.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 17:10:00 $
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
#include <s2_luidl/pe_vari2.hxx>


// NOT FULLY DECLARED SERVICES
#include <ary/idl/i_gate.hxx>
#include <ary/idl/i_property.hxx>
#include <ary/idl/ip_ce.hxx>
#include <ary/doc/d_oldidldocu.hxx>
#include <s2_luidl/pe_type2.hxx>
#include <s2_luidl/tk_keyw.hxx>
#include <s2_luidl/tk_ident.hxx>
#include <s2_luidl/tk_punct.hxx>


namespace csi
{
namespace uidl
{


PE_Variable::PE_Variable( ary::idl::Type_id &   i_rResult_Type,
                          String &              i_rResult_Name )
    :   eState(e_none),
        pResult_Type(&i_rResult_Type),
        pResult_Name(&i_rResult_Name),
        pPE_Type(0)
{
    pPE_Type = new PE_Type(i_rResult_Type);
}

void
PE_Variable::EstablishContacts( UnoIDL_PE *                 io_pParentPE,
                                ary::Repository &           io_rRepository,
                                TokenProcessing_Result &    o_rResult )
{
    UnoIDL_PE::EstablishContacts(io_pParentPE,io_rRepository,o_rResult);
    pPE_Type->EstablishContacts(this,io_rRepository,o_rResult);
}

PE_Variable::~PE_Variable()
{
}

void
PE_Variable::ProcessToken( const Token & i_rToken )
{
    i_rToken.Trigger(*this);
}


void
PE_Variable::Process_Default()
{
    if (eState == expect_type)
    {
        SetResult( not_done, push_sure, pPE_Type.Ptr() );
    }
    else
        csv_assert(false);
}

void
PE_Variable::Process_Identifier( const TokIdentifier & i_rToken )
{
    if (eState == expect_type)
    {
        SetResult( not_done, push_sure, pPE_Type.Ptr() );
    }
    else if (eState == expect_name)
    {
        *pResult_Name = i_rToken.Text();
        SetResult( done, stay );
        eState = expect_finish;
    }
    else
        csv_assert(false);
}

void
PE_Variable::Process_Punctuation( const TokPunctuation & )
{
    if (eState == expect_finish)
    {
        SetResult( not_done, pop_success );
        eState = e_none;
    }
    else if (eState == expect_name)
    {
        SetResult( not_done, pop_success );
        eState = e_none;
    }
    else
        csv_assert(false);
}

void
PE_Variable::Process_BuiltInType( const TokBuiltInType & i_rToken )
{
    if (eState == expect_type)
    {
        SetResult( not_done, push_sure, pPE_Type.Ptr() );
    }
    else if (eState == expect_name AND i_rToken.Id() == TokBuiltInType::bty_ellipse)
    {
        SetResult( not_done, pop_success );
        eState = e_none;
    }
    else
        csv_assert(false);
}

void
PE_Variable::InitData()
{
    eState = expect_type;

    *pResult_Type = 0;
    *pResult_Name = "";
}

void
PE_Variable::ReceiveData()
{
    eState = expect_name;
}

void
PE_Variable::TransferData()
{
    eState = e_none;
}

UnoIDL_PE &
PE_Variable::MyPE()
{
    return *this;
}

}   // namespace uidl
}   // namespace csi

