/*************************************************************************
 *
 *  $RCSfile: pe_property.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:41:31 $
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
#include <s2_luidl/pe_property.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/idl/i_gate.hxx>
#include <ary/idl/i_property.hxx>
#include <ary/idl/i_service.hxx>
#include <ary/idl/ip_ce.hxx>
#include <ary_i/codeinf2.hxx>
#include <s2_luidl/pe_vari2.hxx>
#include <s2_luidl/tk_keyw.hxx>
#include <s2_luidl/tk_ident.hxx>
#include <s2_luidl/tk_punct.hxx>



namespace csi
{
namespace uidl
{



PE_Property::PE_Property( const Ce_id & i_rCurOwner )
    :   eState(e_none),
        pCurOwner(&i_rCurOwner),
        pPE_Variable(0),
        nCurParsedType(0),
        sCurParsedName(),
        bIsOptional(false),
        aStereotypes()
{
    pPE_Variable    = new PE_Variable(nCurParsedType, sCurParsedName);
}

void
PE_Property::EstablishContacts( UnoIDL_PE *                io_pParentPE,
                                ary::n22::Repository &      io_rRepository,
                                TokenProcessing_Result &    o_rResult )
{
    UnoIDL_PE::EstablishContacts(io_pParentPE,io_rRepository,o_rResult);
    pPE_Variable->EstablishContacts(this,io_rRepository,o_rResult);
}

PE_Property::~PE_Property()
{
}

void
PE_Property::ProcessToken( const Token & i_rToken )
{
    i_rToken.Trigger(*this);
}

void
PE_Property::Process_Stereotype( const TokStereotype & i_rToken )
{
    switch (i_rToken.Id())
    {
        case TokStereotype::ste_optional:
            bIsOptional = true;
            break;
        case TokStereotype::ste_readonly:
            aStereotypes.Set_Flag(Stereotypes::readonly);
            break;
        case TokStereotype::ste_bound:
            aStereotypes.Set_Flag(Stereotypes::bound);
            break;
        case TokStereotype::ste_constrained:
            aStereotypes.Set_Flag(Stereotypes::constrained);
            break;
        case TokStereotype::ste_maybeambiguous:
            aStereotypes.Set_Flag(Stereotypes::maybeambiguous);
            break;
        case TokStereotype::ste_maybedefault:
            aStereotypes.Set_Flag(Stereotypes::maybedefault);
            break;
        case TokStereotype::ste_maybevoid:
            aStereotypes.Set_Flag(Stereotypes::maybevoid);
            break;
        case TokStereotype::ste_removable:
            aStereotypes.Set_Flag(Stereotypes::removable);
            break;
        case TokStereotype::ste_transient:
            aStereotypes.Set_Flag(Stereotypes::transient);
            break;

        default:
            SetResult(not_done, pop_failure);
            eState = e_none;
            return;
    }

    SetResult(done, stay);
}

void
PE_Property::Process_MetaType( const TokMetaType & i_rToken )
{
    if (eState == e_start)
    {
        if ( i_rToken.Id() == TokMetaType::mt_property )
        {
            SetResult(done, stay);
            eState = expect_variable;
            return;
        }
    }   // endif (eState == e_start)

    SetResult(not_done, pop_failure);
    eState = e_none;
}

void
PE_Property::Process_Punctuation( const TokPunctuation & i_rToken )
{
    switch (eState)
    {
        case e_start:
            SetResult(done, stay);
            break;
        case expect_variable:
            if (i_rToken.Id() == TokPunctuation::Semicolon)
            {
                SetResult(done, pop_success);
                eState = e_none;
            }
            else if (i_rToken.Id() == TokPunctuation::Comma)
                SetResult(done, stay);
            else
                SetResult(not_done, pop_failure);
            break;
        default:
            csv_assert(false);
    }
}

void
PE_Property::Process_Default()
{
    if (eState == expect_variable)
    {
        SetResult(not_done, push_sure, pPE_Variable.Ptr());
        eState = in_variable;
    }
    else
        SetResult(not_done, pop_failure);
}

void
PE_Property::InitData()
{
    eState = e_start;

    nCurParsedType = 0;
    sCurParsedName = "";

    // bIsOptional and
    // aStereotypes
    //   may be preset by the PE_Service-(or PE_Interface-)parent
    //   with PresetOptional() or
    //   PresetStereotype()
    //   - therefore it must not be set here!
}

void
PE_Property::TransferData()
{
    if (bIsOptional)
    {
        SetOptional();
        bIsOptional = false;
    }

    ary::idl::CodeEntity *
        pCe = 0;
    csv_assert(pCurOwner->IsValid());

    pCe = &Gate().Ces().Store_Property( *pCurOwner,
                                        sCurParsedName,
                                        nCurParsedType,
                                        aStereotypes );

    csv_assert(pCe != 0);
    PassDocuAt(*pCe);

    nCurParsedType = 0;
    sCurParsedName.clear();
    aStereotypes = Stereotypes();

    eState = e_none;
}

void
PE_Property::ReceiveData()
{
    eState = expect_variable;
}


UnoIDL_PE &
PE_Property::MyPE()
{
    return *this;
}


}   // namespace uidl
}   // namespace csi

