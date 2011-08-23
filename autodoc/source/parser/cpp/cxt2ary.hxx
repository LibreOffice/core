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

#ifndef ADC_CPP_CTX2ARY_HXX
#define ADC_CPP_CTX2ARY_HXX



// USED SERVICES
    // BASE CLASSES
#include <ary/cpp/inpcontx.hxx>
#include <doc_deal.hxx>
#include "pev.hxx"
#include "fevnthdl.hxx"
    // COMPONENTS
    // PARAMETERS

namespace ary
{
namespace loc
{
    class File;
}
}



namespace cpp
{


/** @descr
    This class provides information about the context of an
    CodeEntity, which is going to be stored in the repository.
    The information is used mainly by class ary::cpp::Gate.

    Also it provides information for the parser about actual
    state of several public variables.

    @todo
    Include events, which allow correct storing of inline
    documentation after enum values, parameters,
    base classes.
*/
class ContextForAry : public ary::cpp::InputContext,
                      public cpp::PeEnvironment,
                      public cpp::FileScope_EventHandler,
                      public DocuDealer
{
  public:
    // LIFECYCLE
                        ContextForAry(
                            ary::cpp::Gate &  io_rAryGate );
    virtual				~ContextForAry();

    // OPERATIONS
    void                ResetResult()           { aTokenResult.Reset(); }

    // INQUIRY
    const TokenProcessing_Result &
                        CurResult() const       { return aTokenResult; }
    // ACCESS
    TokenProcessing_Result &
                        CurResult()             { return aTokenResult; }

  private:
    // Interface ary::cpp::InputContext:
    virtual ary::loc::File &
                        inq_CurFile() const;
    virtual ary::cpp::Namespace &
                        inq_CurNamespace() const;
    virtual ary::cpp::Class *
                        inq_CurClass() const;
    virtual ary::cpp::Enum *
                        inq_CurEnum() const;

    virtual Owner &     inq_CurOwner() const;
    virtual ary::cpp::E_Protection
                        inq_CurProtection() const;

    // Interface PeEnvironment
    virtual void        do_SetTokenResult(
                            E_TokenDone			i_eDone,
                            E_EnvStackAction	i_eWhat2DoWithEnvStack,
                            ParseEnvironment *	i_pParseEnv2Push );
    virtual void        do_OpenNamespace(
                            ary::cpp::Namespace &
                                                io_rOpenedNamespace );
    virtual void        do_OpenExternC(
                            bool				i_bOnlyForOneDeclaration );
    virtual void        do_OpenClass(
                            ary::cpp::Class &	io_rOpenedClass );
    virtual void        do_OpenEnum(
                            ary::cpp::Enum &	io_rOpenedEnum );
    virtual void        do_CloseBlock();
    virtual void        do_CloseClass();
    virtual void        do_CloseEnum();
    virtual void        do_SetCurProtection(
                            ary::cpp::E_Protection
                                                i_eProtection );
    virtual void        do_OpenTemplate(
                            const StringVector &
                                                i_rParameters );
    virtual DYN StringVector *
                        do_Get_CurTemplateParameters();
    virtual void        do_Close_OpenTemplate();
    virtual void        do_Event_Class_FinishedBase(
                            const String  &     i_sBaseName );

    virtual void        do_Event_Store_Typedef(
                            ary::cpp::Typedef & io_rTypedef );
    virtual void        do_Event_Store_EnumValue(
                            ary::cpp::EnumValue &
                                                io_rEnumValue );
    virtual void        do_Event_Store_CppDefinition(
                            ary::cpp::DefineEntity &
                                                io_rDefinition );
    virtual void        do_Event_EnterFunction_ParameterList();
    virtual void        do_Event_Function_FinishedParameter(
                            const String  &     i_sParameterName );
    virtual void        do_Event_LeaveFunction_ParameterList();
    virtual void        do_Event_EnterFunction_Implementation();
    virtual void        do_Event_LeaveFunction_Implementation();
    virtual void        do_Event_Store_Function(
                            ary::cpp::Function &
                                                io_rFunction );
    virtual void        do_Event_Store_Variable(
                            ary::cpp::Variable &
                                                io_rVariable );
    virtual void        do_TakeDocu(
                            DYN ary::doc::OldCppDocu &
                                                let_drInfo );
    virtual void        do_StartWaitingFor_Recovery();
    virtual ary::cpp::Gate &
                        inq_AryGate() const;
    virtual const ary::cpp::InputContext &
                        inq_Context() const;
    virtual String		inq_CurFileName() const;
    virtual uintt       inq_LineCount() const;
    virtual bool        inq_IsWaitingFor_Recovery() const;
    virtual bool        inq_IsExternC() const;

    // Interface FileScope_EventHandler
    virtual void        do_SetCurFile(
                            ary::loc::File &    io_rCurFile );
    virtual void        do_Event_IncrLineCount();
    virtual void        do_Event_SwBracketOpen();
    virtual void        do_Event_SwBracketClose();
    virtual void        do_Event_Semicolon();

    // Local types
    struct S_FileScopeInfo;
    struct S_OwnerStack;
    struct S_DocuDistributor;
    struct S_RecoveryGuard;

    // DATA
    ary::cpp::Gate *    pGate;
    TokenProcessing_Result
                        aTokenResult;
    Dyn<S_FileScopeInfo>
                        pFileScopeInfo;
    Dyn<S_OwnerStack>   pOwnerStack;
    Dyn<S_DocuDistributor>
                        pDocuDistributor;
    Dyn<S_RecoveryGuard>
                        pRecoveryGuard;
};




}   // namespace cpp
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
