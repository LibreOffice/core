/*************************************************************************
 *
 *  $RCSfile: pe_attri.cxx,v $
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
#include <s2_luidl/pe_attri.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary_i/codeinf2.hxx>
#include <s2_luidl/pe_vari2.hxx>
#include <s2_luidl/tk_keyw.hxx>
#include <s2_luidl/tk_ident.hxx>
#include <s2_luidl/tk_punct.hxx>
#include <csi/l_uidl/attrib.hxx>



namespace csi
{
namespace uidl
{



PE_Attribute::PE_Attribute( RAttribute &        o_rResult,
                            const RInterface &  i_rCurInterface )
    :   eState(e_none),
        pData(0),
        pResult(&o_rResult),
        pCurInterface(&i_rCurInterface),
        pPE_Variable(0),
        aCurParsedType(0),
        sCurParsedName(""),
        bIsOptional(false),
        bIsReadonly(false)
{
    pPE_Variable    = new PE_Variable(aCurParsedType, sCurParsedName);
}

void
PE_Attribute::EstablishContacts( UnoIDL_PE *                io_pParentPE,
                                 ary::Repository &          io_rRepository,
                                 TokenProcessing_Result &   o_rResult )
{
    UnoIDL_PE::EstablishContacts(io_pParentPE,io_rRepository,o_rResult);
    pPE_Variable->EstablishContacts(this,io_rRepository,o_rResult);
}

PE_Attribute::~PE_Attribute()
{
}

void
PE_Attribute::ProcessToken( const Token & i_rToken )
{
    i_rToken.Trigger(*this);
}

void
PE_Attribute::Process_Stereotype( const TokStereotype & i_rToken )
{
    if (i_rToken.Id() == TokStereotype::ste_readonly)
    {
        pData->Data().bReadonly = true;
        SetResult(done, stay);
        return;
    }
    else if (i_rToken.Id() == TokStereotype::ste_optional)
    {
        bIsOptional = true;
        SetResult(done, stay);
        return;
    }

    SetResult(not_done, pop_failure);
    eState = e_none;
}

void
PE_Attribute::Process_MetaType( const TokMetaType & i_rToken )
{
    if (eState == e_start)
    {
        if ( i_rToken.Id() == TokMetaType::mt_attribute OR
             i_rToken.Id() == TokMetaType::mt_property )
        {
            SetResult(done, stay);
            eState = expect_variable;
            return;
        }
    }

    SetResult(not_done, pop_failure);
    eState = e_none;
}

void
PE_Attribute::Process_Punctuation( const TokPunctuation & i_rToken )
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
PE_Attribute::Process_Default()
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
PE_Attribute::InitData()
{
    eState = e_start;

    pData = new Attribute;
    *pResult = 0;
    aCurParsedType = 0;
    sCurParsedName = "";

    // bIsOptional may be preset by the PE_Service-parent with PresetOptional()
    //   - therefore it must not be set here!
}

void
PE_Attribute::TransferData()
{
    if (bIsOptional)
    {
        SetOptional();
        bIsOptional = false;
    }
    if (bIsReadonly)
    {
        pData->Data().bReadonly = true;
        bIsReadonly = false;
    }

    *pResult = Gate().Store_Attribute( pCurInterface->Id(), *pData );
    PassDocuAt(pResult->Id());

    pData = 0;

    eState = e_none;
}

void
PE_Attribute::ReceiveData()
{
    pData->Data().pType = aCurParsedType;
    pData->Data().sName = sCurParsedName;
    eState = expect_variable;
}


UnoIDL_PE &
PE_Attribute::MyPE()
{
     return *this;
}


}   // namespace uidl
}   // namespace csi

