/*************************************************************************
 *
 *  $RCSfile: pe_excp.cxx,v $
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
#include <s2_luidl/pe_excp.hxx>


// NOT FULLY DECLARED SERVICES
#include <ary_i/codeinf2.hxx>
#include <s2_luidl/tk_ident.hxx>
#include <s2_luidl/tk_punct.hxx>
#include <s2_luidl/tk_keyw.hxx>
#include <s2_luidl/pe_type2.hxx>
#include <s2_luidl/pe_selem.hxx>
#include <csi/l_uidl/except.hxx>
#include <csi/l_uidl/struelem.hxx>
#include <ary_i/uidl/gate.hxx>



namespace csi
{
namespace uidl
{


PE_Exception::PE_Exception()
    // :    aWork,
    //      pStati
{
    pStati = new S_Stati(*this);
}

void
PE_Exception::EstablishContacts( UnoIDL_PE *                io_pParentPE,
                              ary::Repository &         io_rRepository,
                              TokenProcessing_Result &  o_rResult )
{
    UnoIDL_PE::EstablishContacts(io_pParentPE,io_rRepository,o_rResult);
    Work().pPE_Element->EstablishContacts(this,io_rRepository,o_rResult);
    Work().pPE_Type->EstablishContacts(this,io_rRepository,o_rResult);
}

PE_Exception::~PE_Exception()
{
}

void
PE_Exception::ProcessToken( const Token & i_rToken )
{
    i_rToken.Trigger(*Stati().pCurStatus);
}


void
PE_Exception::InitData()
{
    Work().InitData();
    Stati().pCurStatus = &Stati().aWaitForName;
}

void
PE_Exception::TransferData()
{
    if (NOT Work().bIsPreDeclaration)
    {
        csv_assert(Work().pData != 0);
        csv_assert(Work().pCurStruct);
    }
    Stati().pCurStatus = &Stati().aNone;
}

void
PE_Exception::ReceiveData()
{
    Stati().pCurStatus->On_SubPE_Left();
}

PE_Exception::S_Work::S_Work()
    :   // pData
        bIsPreDeclaration(false),
        pCurStruct(0),
        pPE_Element(0),
        aCurParsed_ElementRef(0),
        pPE_Type(0)
        // aCurParsed_Base

{
    pPE_Element = new PE_StructElement(aCurParsed_ElementRef,pCurStruct);
    pPE_Type = new PE_Type(aCurParsed_Base);
}

void
PE_Exception::S_Work::InitData()
{
    pData = new Exception;
    bIsPreDeclaration = false;
    pCurStruct = 0;
}

void
PE_Exception::S_Work::Prepare_PE_QualifiedName()
{
    aCurParsed_ElementRef = 0;
}

void
PE_Exception::S_Work::Prepare_PE_Element()
{
    aCurParsed_Base = 0;
}

void
PE_Exception::S_Work::Data_Set_Name( const char * i_sName )
{
    pData->Data().sName = i_sName;
}

void
PE_Exception::S_Work::Data_Add_CurParsed_ElementRef()
{
    pData->Data().aElements.push_back(aCurParsed_ElementRef);
}


PE_Exception::S_Stati::S_Stati(PE_Exception & io_rStruct)
    :   aNone(io_rStruct),
        aWaitForName(io_rStruct),
        aGotName(io_rStruct),
        aWaitForBase(io_rStruct),
        aGotBase(io_rStruct),
        aWaitForElement(io_rStruct),
        aWaitForFinish(io_rStruct),
        pCurStatus(0)
{
    pCurStatus = &aNone;
}


//***********************       Stati       ***************************//


UnoIDL_PE &
PE_Exception::PE_StructState::MyPE()
{
    return rStruct;
}


void
PE_Exception::State_WaitForName::Process_Identifier( const TokIdentifier & i_rToken )
{
    Work().Data_Set_Name(i_rToken.Text());
    MoveState( Stati().aGotName );
    SetResult(done,stay);
}

void
PE_Exception::State_GotName::Process_Punctuation( const TokPunctuation & i_rToken )
{
    if ( i_rToken.Id() != TokPunctuation::Semicolon )
    {
        Work().pCurStruct =
                PE().Gate().Store_Exception(
                        PE().CurNamespace().Id(),
                        *Work().pData );
        PE().PassDocuAt(Work().pCurStruct.Id());


        switch (i_rToken.Id())
        {
            case TokPunctuation::Colon:
                MoveState( Stati().aWaitForBase );
                SetResult(done,push_sure,Work().pPE_Type.Ptr());
                Work().Prepare_PE_QualifiedName();
                break;
            case TokPunctuation::CurledBracketOpen:
                MoveState( Stati().aWaitForElement );
                SetResult(done,stay);
                break;
            default:
                SetResult(not_done,pop_failure);
        }   // end switch
    }
    else
    {
        Delete_dyn(Work().pData);
        SetResult(done,pop_success);
    }
}

void
PE_Exception::State_WaitForBase::On_SubPE_Left()
{
    Work().pData->Data().pBaseException = Work().aCurParsed_Base;
    MoveState(Stati().aGotBase);
}

void
PE_Exception::State_GotBase::Process_Punctuation( const TokPunctuation & i_rToken )
{
    if ( i_rToken.Id() == TokPunctuation::CurledBracketOpen )
    {
        MoveState( Stati().aWaitForElement );
        SetResult(done,stay);
    }
    else
    {
        SetResult(not_done,pop_failure);
    }
}

void
PE_Exception::State_WaitForElement::Process_Identifier( const TokIdentifier & )
{
    SetResult( not_done, push_sure, Work().pPE_Element.Ptr() );
    Work().Prepare_PE_Element();
}

void
PE_Exception::State_WaitForElement::Process_NameSeparator()
{
    SetResult( not_done, push_sure, Work().pPE_Element.Ptr());
    Work().Prepare_PE_Element();
}

void
PE_Exception::State_WaitForElement::Process_BuiltInType( const TokBuiltInType & )
{
    SetResult( not_done, push_sure, Work().pPE_Element.Ptr());
    Work().Prepare_PE_Element();
}

void
PE_Exception::State_WaitForElement::Process_TypeModifier(const TokTypeModifier & )
{
    SetResult( not_done, push_sure, Work().pPE_Element.Ptr());
    Work().Prepare_PE_Element();
}

void
PE_Exception::State_WaitForElement::Process_Punctuation( const TokPunctuation & i_rToken )
{
    if ( i_rToken.Id() == TokPunctuation::CurledBracketClose )
    {
        MoveState( Stati().aWaitForFinish );
        SetResult( done, stay );
    }
    else
    {
        SetResult( not_done, pop_failure );
    }
}

void
PE_Exception::State_WaitForElement::On_SubPE_Left()
{
    Work().Data_Add_CurParsed_ElementRef();
}

void
PE_Exception::State_WaitForFinish::Process_Punctuation( const TokPunctuation & i_rToken )
{
    if (i_rToken.Id() == TokPunctuation::Semicolon)
    {
        MoveState( Stati().aNone );
        SetResult( done, pop_success );
    }
    else
    {
        SetResult( not_done, pop_failure );
    }
}


}   // namespace uidl
}   // namespace csi

