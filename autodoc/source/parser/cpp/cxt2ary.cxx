/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <precomp.h>
#include "cxt2ary.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/entity.hxx>
#include <ary/cpp/c_class.hxx>
#include <ary/cpp/c_define.hxx>
#include <ary/cpp/c_enum.hxx>
#include <ary/cpp/c_enuval.hxx>
#include <ary/cpp/c_funct.hxx>
#include <ary/cpp/c_macro.hxx>
#include <ary/cpp/c_tydef.hxx>
#include <ary/cpp/c_vari.hxx>
#include <ary/cpp/c_gate.hxx>
#include <ary/cpp/cp_ce.hxx>
#include <ary/loc/loc_file.hxx>
#include <ary/doc/d_oldcppdocu.hxx>
#include <ary/info/docstore.hxx>
#include "icprivow.hxx"

// Implementationheaders, only to be used in this file!
#include "sfscope.hxx"
#include "sownstck.hxx"
#include "sdocdist.hxx"
#include "srecover.hxx"





namespace cpp
{

using ary::cpp::E_Protection;

ContextForAry::ContextForAry( ary::cpp::Gate &  io_rAryGate )
    :   pGate(&io_rAryGate),
        aTokenResult(),
        pFileScopeInfo( new S_FileScopeInfo ),
        pOwnerStack( new S_OwnerStack ),
        pDocuDistributor( new S_DocuDistributor ),
        pRecoveryGuard( new S_RecoveryGuard )
{
    OpenNamespace( pGate->Ces().GlobalNamespace() );
}

ContextForAry::~ContextForAry()
{
}

ary::loc::File &
ContextForAry::inq_CurFile() const
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
ContextForAry::do_SetTokenResult( E_TokenDone			i_eDone,
                                  E_EnvStackAction	    i_eWhat2DoWithEnvStack,
                                  ParseEnvironment *	i_pParseEnv2Push )
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
ContextForAry::do_OpenExternC( bool )
{
    pOwnerStack->OpenExternC();
    // KORR_FUTURE
    // use i_bOnlyForOneDeclaration
}

void
ContextForAry::do_OpenClass( ary::cpp::Class & io_rOpenedClass )
{
    pOwnerStack->OpenClass(io_rOpenedClass);
    pDocuDistributor->SetCurrentlyStoredRe(io_rOpenedClass);
}

void
ContextForAry::do_OpenEnum(	ary::cpp::Enum & io_rOpenedEnum )
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
ContextForAry::do_Close_OpenTemplate()
{
    if (pFileScopeInfo->pCurTemplateParameters)
        delete pFileScopeInfo->pCurTemplateParameters.Release();
}

void
ContextForAry::do_Event_Class_FinishedBase( const String & )
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
ContextForAry::do_Event_Store_CppDefinition( ary::cpp::DefineEntity & io_rDefinition )
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
ContextForAry::do_Event_Function_FinishedParameter( const String & )
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
ContextForAry::do_TakeDocu( DYN ary::doc::OldCppDocu & let_drInfo )
{
    let_drInfo.Store2(*pDocuDistributor);
}

void
ContextForAry::do_StartWaitingFor_Recovery()
{
    pRecoveryGuard->StartWaitingFor_Recovery();
}

ary::cpp::Gate &
ContextForAry::inq_AryGate() const
{
     return * const_cast< ary::cpp::Gate* >(pGate);
}

const ary::cpp::InputContext &
ContextForAry::inq_Context() const
{
    return *this;
}

String
ContextForAry::inq_CurFileName() const
{
    return pFileScopeInfo->pCurFile != 0
            ?   pFileScopeInfo->pCurFile->LocalName()
            :   String::Null_();
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
ContextForAry::do_SetCurFile( ary::loc::File &  io_rCurFile )
{
    pFileScopeInfo->pCurFile = &io_rCurFile;
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




}   // namespace cpp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
