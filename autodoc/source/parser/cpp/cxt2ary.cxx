/*************************************************************************
 *
 *  $RCSfile: cxt2ary.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-15 18:45:56 $
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
#include "cxt2ary.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/re.hxx>
#include <ary/docu.hxx>
#include <ary/cpp/c_class.hxx>
#include <ary/cpp/c_define.hxx>
#include <ary/cpp/c_enum.hxx>
#include <ary/cpp/c_enuval.hxx>
#include <ary/cpp/c_funct.hxx>
#include <ary/cpp/c_macro.hxx>
#include <ary/cpp/c_rwgate.hxx>
#include <ary/cpp/c_tydef.hxx>
#include <ary/cpp/c_vari.hxx>
#include <ary/info/codeinfo.hxx>
#include <ary/info/docstore.hxx>
#include "icprivow.hxx"

// Implementationheader, only to be used in this file!
#include "sfscope.hxx"
#include "sownstck.hxx"
#include "sdocdist.hxx"
#include "srecover.hxx"





namespace cpp
{

using ary::cpp::E_Protection;

ContextForAry::ContextForAry( ary::cpp::RwGate &  io_rAryGate )
    :   pGate(&io_rAryGate),
        // aTokenResult,
        pFileScopeInfo( new S_FileScopeInfo ),
        pOwnerStack( new S_OwnerStack ),
        pDocuDistributor( new S_DocuDistributor ),
        pRecoveryGuard( new S_RecoveryGuard )
{
    OpenNamespace( pGate->GlobalNamespace() );
}

ContextForAry::~ContextForAry()
{
}

ary::cpp::ProjectGroup &
ContextForAry::inq_CurProjectGroup() const
{
    csv_assert(pFileScopeInfo->pCurProject != 0);

    return *pFileScopeInfo->pCurProject;
}

ary::cpp::FileGroup &
ContextForAry::inq_CurFileGroup() const
{
    csv_assert(pFileScopeInfo->pCurFile != 0);

    return *pFileScopeInfo->pCurFile;
}

ary::cpp::Namespace &
ContextForAry::inq_CurNamespace() const
{
    return pOwnerStack->CurNamespace();
}

ary::cpp::Class *
ContextForAry::inq_CurClass() const
{
    return pOwnerStack->CurClass();
}

ary::cpp::Enum *
ContextForAry::inq_CurEnum() const
{
    return pOwnerStack->CurEnum();
}


ary::cpp::InputContext::Owner &
ContextForAry::inq_CurOwner() const
{
    return pOwnerStack->CurOwner();
}

E_Protection
ContextForAry::inq_CurProtection() const
{
    return pOwnerStack->CurProtection();
}

void
ContextForAry::do_SetTokenResult( E_TokenDone           i_eDone,
                                  E_EnvStackAction      i_eWhat2DoWithEnvStack,
                                  ParseEnvironment *    i_pParseEnv2Push )
{
    aTokenResult.eDone = i_eDone;
    aTokenResult.eStackAction = i_eWhat2DoWithEnvStack;
    aTokenResult.pEnv2Push = i_pParseEnv2Push;
}

void
ContextForAry::do_OpenNamespace( ary::cpp::Namespace & io_rOpenedNamespace )
{
    pOwnerStack->OpenNamespace( io_rOpenedNamespace );
}

void
ContextForAry::do_OpenExternC( bool i_bOnlyForOneDeclaration )
{
    pOwnerStack->OpenExternC();
    // KORR
    // use i_bOnlyForOneDeclaration
}

void
ContextForAry::do_OpenClass( ary::cpp::Class & io_rOpenedClass )
{
    pOwnerStack->OpenClass(io_rOpenedClass);
    pDocuDistributor->SetCurrentlyStoredRe(io_rOpenedClass);
}

void
ContextForAry::do_OpenEnum( ary::cpp::Enum & io_rOpenedEnum )
{
    pOwnerStack->OpenEnum(io_rOpenedEnum);
    pDocuDistributor->SetCurrentlyStoredRe(io_rOpenedEnum);
}

void
ContextForAry::do_CloseBlock()
{
    pOwnerStack->CloseBlock();
}

void
ContextForAry::do_CloseClass()
{
    pOwnerStack->CloseClass();
}

void
ContextForAry::do_CloseEnum()
{
    pOwnerStack->CloseEnum();
}

void
ContextForAry::do_SetCurProtection( ary::cpp::E_Protection i_eProtection )
{
    pOwnerStack->SetCurProtection(i_eProtection);
}

void
ContextForAry::do_OpenTemplate( const StringVector & i_rParameters )
{
    pFileScopeInfo->pCurTemplateParameters = new StringVector(i_rParameters);
}

DYN StringVector *
ContextForAry::do_Get_CurTemplateParameters()
{
    return pFileScopeInfo->pCurTemplateParameters.Release();
}

void
ContextForAry::do_Event_Class_FinishedBase( const udmstri & i_sBaseName )
{
    // KORR_FUTURE
}

void
ContextForAry::do_Event_Store_Typedef( ary::cpp::Typedef & io_rTypedef )
{
    pDocuDistributor->SetCurrentlyStoredRe(io_rTypedef);
}

void
ContextForAry::do_Event_Store_EnumValue( ary::cpp::EnumValue & io_rEnumValue )
{
    pDocuDistributor->SetCurrentlyStoredRe(io_rEnumValue);
}

void
ContextForAry::do_Event_Store_CppDefinition( ary::cpp::CppDefinition & io_rDefinition )
{
    pDocuDistributor->SetCurrentlyStoredRe(io_rDefinition);
}

void
ContextForAry::do_Event_EnterFunction_ParameterList()
{
    // KORR_FUTURE
    // Inform pDocuDistributor about possibility of parameters' inline documentation.
}

void
ContextForAry::do_Event_Function_FinishedParameter( const udmstri & i_sParameterName )
{
    // KORR_FUTURE
}

void
ContextForAry::do_Event_LeaveFunction_ParameterList()
{
    // KORR_FUTURE
}

void
ContextForAry::do_Event_EnterFunction_Implementation()
{
    // KORR_FUTURE
}

void
ContextForAry::do_Event_LeaveFunction_Implementation()
{
    // KORR_FUTURE
}

void
ContextForAry::do_Event_Store_Function( ary::cpp::Function & io_rFunction )
{
    pDocuDistributor->SetCurrentlyStoredRe(io_rFunction);
}


void
ContextForAry::do_Event_Store_Variable( ary::cpp::Variable & io_rVariable )
{
    pDocuDistributor->SetCurrentlyStoredRe(io_rVariable);
}

void
ContextForAry::do_TakeDocu( DYN ary::Documentation & let_drInfo )
{
    let_drInfo.Store2(*pDocuDistributor);
}

void
ContextForAry::do_StartWaitingFor_Recovery()
{
    pRecoveryGuard->StartWaitingFor_Recovery();
}

ary::cpp::RwGate &
ContextForAry::inq_AryGate() const
{
     return * const_cast< ary::cpp::RwGate* >(pGate);
}

const ary::cpp::InputContext &
ContextForAry::inq_Context() const
{
    return *this;
}

udmstri
ContextForAry::inq_CurFileName() const
{
    return pFileScopeInfo->sCurFileName;
}

uintt
ContextForAry::inq_LineCount() const
{
    return pFileScopeInfo->nLineCount;
}

bool
ContextForAry::inq_IsWaitingFor_Recovery() const
{
    return pRecoveryGuard->IsWithinRecovery();
}

bool
ContextForAry::inq_IsExternC() const
{
     return pOwnerStack->IsExternC();
}

void
ContextForAry::do_SetCurProject( ary::cpp::ProjectGroup & io_rCurProject )
{
    pFileScopeInfo->pCurProject = &io_rCurProject;
}

void
ContextForAry::do_SetCurFile( ary::cpp::FileGroup & io_rCurFile,
                              const udmstri &       i_sFileName )
{
    pFileScopeInfo->pCurFile = &io_rCurFile;
    pFileScopeInfo->sCurFileName = i_sFileName;
    pFileScopeInfo->nLineCount = 0;
    pFileScopeInfo->pCurTemplateParameters = 0;

    pOwnerStack->Reset();
    pDocuDistributor->Reset();
    pRecoveryGuard->Reset();
}

void
ContextForAry::do_Event_IncrLineCount()
{
    ++pFileScopeInfo->nLineCount;
    pDocuDistributor->Event_LineBreak();
}

void
ContextForAry::do_Event_SwBracketOpen()
{
    pRecoveryGuard->Hdl_SwBracketOpen();
}

void
ContextForAry::do_Event_SwBracketClose()
{
    pRecoveryGuard->Hdl_SwBracketClose();
}

void
ContextForAry::do_Event_Semicolon()
{
    pRecoveryGuard->Hdl_Semicolon();
}

ary::cpp::ProjectGroup &
ContextForAry::inq_CurProject() const
{
    csv_assert( pFileScopeInfo->pCurProject != 0 );

    return *pFileScopeInfo->pCurProject;
}


}   // namespace cpp


