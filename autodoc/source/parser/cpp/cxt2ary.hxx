/*************************************************************************
 *
 *  $RCSfile: cxt2ary.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:29 $
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

namespace cpp
{


/** @descr
    This class provides information about the context of an
    CodeEntity, which is going to be stored in the repository.
    The information is used mainly by class ary::cpp::RwGate.

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
                            ary::cpp::RwGate &  io_rAryGate );
    virtual             ~ContextForAry();

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
    virtual ary::cpp::ProjectGroup &
                        inq_CurProjectGroup() const;
    virtual ary::cpp::FileGroup &
                        inq_CurFileGroup() const;

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
                            E_TokenDone         i_eDone,
                            E_EnvStackAction    i_eWhat2DoWithEnvStack,
                            ParseEnvironment *  i_pParseEnv2Push );
    virtual void        do_OpenNamespace(
                            ary::cpp::Namespace &
                                                io_rOpenedNamespace );
    virtual void        do_OpenExternC(
                            bool                i_bOnlyForOneDeclaration );
    virtual void        do_OpenClass(
                            ary::cpp::Class &   io_rOpenedClass );
    virtual void        do_OpenEnum(
                            ary::cpp::Enum &    io_rOpenedEnum );
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
    virtual void        do_Event_Class_FinishedBase(
                            const udmstri &     i_sBaseName );

    virtual void        do_Event_Store_Typedef(
                            ary::cpp::Typedef & io_rTypedef );
    virtual void        do_Event_Store_EnumValue(
                            ary::cpp::EnumValue &
                                                io_rEnumValue );
    virtual void        do_Event_Store_CppDefinition(
                            ary::cpp::CppDefinition &
                                                io_rDefinition );
    virtual void        do_Event_EnterFunction_ParameterList();
    virtual void        do_Event_Function_FinishedParameter(
                            const udmstri &     i_sParameterName );
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
                            DYN ary::Documentation &
                                                let_drInfo );
    virtual void        do_StartWaitingFor_Recovery();
    virtual ary::cpp::RwGate &
                        inq_AryGate() const;
    virtual const ary::cpp::InputContext &
                        inq_Context() const;
    virtual udmstri     inq_CurFileName() const;
    virtual uintt       inq_LineCount() const;
    virtual bool        inq_IsWaitingFor_Recovery() const;
    virtual bool        inq_IsExternC() const;

    // Interface FileScope_EventHandler
    virtual void        do_SetCurProject(
                            ary::cpp::ProjectGroup &
                                                io_rCurProject );
    virtual void        do_SetCurFile(
                            ary::cpp::FileGroup &
                                                io_rCurFile,
                            const udmstri &     i_sFileName );
    virtual void        do_Event_IncrLineCount();
    virtual void        do_Event_SwBracketOpen();
    virtual void        do_Event_SwBracketClose();
    virtual void        do_Event_Semicolon();
    virtual ary::cpp::ProjectGroup &
                        inq_CurProject() const;

    // Local types
    struct S_FileScopeInfo;
    struct S_OwnerStack;
    struct S_DocuDistributor;
    struct S_RecoveryGuard;

    // DATA
    ary::cpp::RwGate *  pGate;
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

