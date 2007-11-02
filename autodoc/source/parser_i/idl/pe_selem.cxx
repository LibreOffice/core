/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pe_selem.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 17:08:18 $
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
#include <s2_luidl/pe_selem.hxx>

// NOT FULLY DECLARED SERVICES
#include <ary/idl/i_gate.hxx>
#include <ary/idl/i_struct.hxx>
#include <ary/idl/i_structelem.hxx>
#include <ary/idl/ip_ce.hxx>
#include <ary/doc/d_oldidldocu.hxx>
#include <s2_luidl/pe_type2.hxx>
#include <s2_luidl/tk_ident.hxx>
#include <s2_luidl/tk_punct.hxx>


namespace csi
{
namespace uidl
{

namespace
{
    const String  C_sNone;
}

PE_StructElement::PE_StructElement( RStructElement &    o_rResult,
                                    const RStruct &     i_rCurStruct,
                                    const String &      i_rCurStructTemplateParam )
    :   eState(e_none),
        pResult(&o_rResult),
        pCurStruct(&i_rCurStruct),
        bIsExceptionElement(false),
        pPE_Type(0),
        nType(0),
        sName(),
        pCurStructTemplateParam(&i_rCurStructTemplateParam)
{
    pPE_Type = new PE_Type(nType);
}

PE_StructElement::PE_StructElement( RStructElement &    o_rResult,
                                    const RStruct &     i_rCurExc )
    :   eState(e_none),
        pResult(&o_rResult),
        pCurStruct(&i_rCurExc),
        bIsExceptionElement(true),
        pPE_Type(0),
        nType(0),
        sName(),
        pCurStructTemplateParam(&C_sNone)
{
    pPE_Type = new PE_Type(nType);
}

void
PE_StructElement::EstablishContacts( UnoIDL_PE *                io_pParentPE,
                                     ary::Repository &      io_rRepository,
                                     TokenProcessing_Result &   o_rResult )
{
    UnoIDL_PE::EstablishContacts(io_pParentPE,io_rRepository,o_rResult);
    pPE_Type->EstablishContacts(this,io_rRepository,o_rResult);
}

PE_StructElement::~PE_StructElement()
{
}

void
PE_StructElement::ProcessToken( const Token & i_rToken )
{
    i_rToken.Trigger(*this);
}

void
PE_StructElement::Process_Default()
{
    if (eState == expect_type)
    {
        SetResult( not_done, push_sure, pPE_Type.Ptr() );
        eState = expect_name;
    }
    else
        csv_assert(false);
}

void
PE_StructElement::Process_Identifier( const TokIdentifier & i_rToken )
{
    csv_assert(*i_rToken.Text() != 0);

    if (eState == expect_type)
    {
        if ( *pCurStructTemplateParam == i_rToken.Text() )
        {
            nType = lhf_FindTemplateParamType();
               SetResult( done, stay );
            eState = expect_name;
        }
        else    // No template parameter type existing, or not matching:
        {
               SetResult( not_done, push_sure, pPE_Type.Ptr() );
            eState = expect_name;
        }
    }
    else if (eState == expect_name)
    {
        sName = i_rToken.Text();
        SetResult( done, stay );
        eState = expect_finish;
    }
    else
        csv_assert(false);
}

void
PE_StructElement::Process_Punctuation( const TokPunctuation &)
{
    csv_assert(eState == expect_finish);

    SetResult( done, pop_success );
}

void
PE_StructElement::InitData()
{
    eState = expect_type;

    nType = 0;
    sName = "";
}

void
PE_StructElement::TransferData()
{
    csv_assert(pResult != 0 AND pCurStruct != 0);

    ary::idl::StructElement *
        pCe = 0;
    if (bIsExceptionElement)
    {
        pCe = & Gate().Ces().Store_ExceptionMember(
                                            *pCurStruct,
                                            sName,
                                            nType );
    }
    else
    {
        pCe = & Gate().Ces().Store_StructMember(
                                            *pCurStruct,
                                            sName,
                                            nType );
    }
    *pResult = pCe->CeId();
    PassDocuAt(*pCe);

    eState = e_none;
}

UnoIDL_PE &
PE_StructElement::MyPE()
{
    return *this;
}

ary::idl::Type_id
PE_StructElement::lhf_FindTemplateParamType() const
{
    const ary::idl::CodeEntity &
        rCe = Gate().Ces().Find_Ce(*pCurStruct);
    const ary::idl::Struct &
        rStruct = static_cast< const ary::idl::Struct& >(rCe);
    return rStruct.TemplateParameterType();
}


}   // namespace uidl
}   // namespace csi
