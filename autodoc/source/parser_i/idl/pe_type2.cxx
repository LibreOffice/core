/*************************************************************************
 *
 *  $RCSfile: pe_type2.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-18 14:11:41 $
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
#include <s2_luidl/pe_type2.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/idl/i_gate.hxx>
#include <ary/idl/i_type.hxx>
#include <ary/idl/ip_type.hxx>
#include <ary_i/codeinf2.hxx>
#include <s2_luidl/uidl_tok.hxx>
#include <s2_luidl/tk_ident.hxx>
#include <s2_luidl/tk_keyw.hxx>



namespace csi
{
namespace uidl
{


PE_Type::PE_Type( ary::idl::Type_id & o_rResult )
    :   pResult(&o_rResult),
        nIsSequenceCounter(0),
        bIsUnsigned(false),
        sFullType(),
        eState(e_none),
        sLastPart()
{
}

PE_Type::~PE_Type()
{
}

void
PE_Type::ProcessToken( const Token & i_rToken )
{
    i_rToken.Trigger(*this);
}

void
PE_Type::Process_Identifier( const TokIdentifier & i_rToken )
{
    if (eState == expect_type)
    {
        sLastPart = i_rToken.Text();
        eState = expect_quname_separator;
        SetResult(done, stay);
    }
    else if (eState == expect_quname_part)
    {
        sLastPart = i_rToken.Text();
        eState = expect_quname_separator;
        SetResult(done, stay);
    }
    else if (eState == expect_quname_separator)
    {
        Finish();
    }
}

void
PE_Type::Process_NameSeparator()
{
    if (eState == expect_type)
    {
        sFullType.Init(true);
        eState = expect_quname_part;
        SetResult(done, stay);
    }
    else if (eState == expect_quname_separator)
    {
        sFullType += sLastPart;
        eState = expect_quname_part;
        SetResult(done, stay);
    }
}

void
PE_Type::Process_Punctuation( const TokPunctuation & i_rToken )
{
    Finish();
}

void
PE_Type::Process_BuiltInType( const TokBuiltInType & i_rToken )
{
    if (eState == expect_type)
    {
        sLastPart = i_rToken.Text();
        eState = expect_quname_separator;
        SetResult(done, stay);
    }
    else if (eState == expect_quname_part)
    {
        sLastPart = i_rToken.Text();
        eState = expect_quname_separator;
        SetResult(done, stay);
    }
    else if (eState == expect_quname_separator)
    {
        sFullType.SetLocalName(sLastPart);
        SetResult(not_done, pop_success);
    }
}

void
PE_Type::Process_TypeModifier( const TokTypeModifier & i_rToken )
{
    if (eState == expect_type)
    {
        switch ( i_rToken.Id() )
        {
            case TokTypeModifier::tmod_unsigned:
                    bIsUnsigned = true;
                    break;
            case TokTypeModifier::tmod_sequence:
                    nIsSequenceCounter++;
                    break;
            default:
                csv_assert(false);
        }
        SetResult(done, stay);
    }
    else if (eState == expect_quname_separator)
    {
        sFullType.SetLocalName(sLastPart);
        SetResult(not_done, pop_success);
    }
}

void
PE_Type::Process_Default()
{
    Finish();
}

void
PE_Type::Finish()
{
    sFullType.SetLocalName(sLastPart);
    SetResult(not_done, pop_success);
}

void
PE_Type::InitData()
{
    eState = expect_type;

    nIsSequenceCounter = 0;
    bIsUnsigned = false;
    sFullType.Empty();
    sLastPart.clear();
}

void
PE_Type::TransferData()
{
    if (bIsUnsigned)
    {
        String sName( StreamLock(40)() << "unsigned " << sFullType.LocalName() << c_str );
        sFullType.SetLocalName(sName);
    }

    const ary::idl::Type &
        result = Gate().Types().CheckIn_Type( sFullType,
                                              nIsSequenceCounter,
                                              CurNamespace().CeId() );
    *pResult = result.TypeId();
    eState = e_none;
}

UnoIDL_PE &
PE_Type::MyPE()
{
     return *this;
}


}   // namespace uidl
}   // namespace csi



