/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef ADC_UIDL_PE_FUNC2_HXX
#define ADC_UIDL_PE_FUNC2_HXX

#include "sal/config.h"

#include "sal/types.h"

// USED SERVICES
    // BASE CLASSES
#include <s2_luidl/parsenv2.hxx>
#include <s2_luidl/pestate.hxx>
    // COMPONENTS
#include <ary/idl/i_param.hxx>
    // PARAMETERS

namespace ary
{
     namespace idl
    {
         class Function;
    }
}


namespace csi
{
namespace uidl
{

class PE_Type;
class PE_Variable;

class PE_Function : public UnoIDL_PE,
                    public ParseEnvState
{
  public:
    typedef ary::idl::Ce_id     RParent;
    typedef ary::idl::Ce_id     RFunction;

    enum E_Constructor { constructor };

    /// Constructor for interfaces.
                        PE_Function(
                            const RParent &     i_rCurInterface );

    /// Constructor for single interface based services.
                        PE_Function(
                            const RParent &     i_rCurService,
                            E_Constructor       i_eCtorMarker );

    virtual void        EstablishContacts(
                            UnoIDL_PE *         io_pParentPE,
                            ary::Repository &   io_rRepository,
                            TokenProcessing_Result &
                                                o_rResult );
    virtual             ~PE_Function();

    virtual void        ProcessToken(
                            const Token &       i_rToken );

    virtual void        Process_Stereotype(
                            const TokStereotype &
                                                i_rToken );
    virtual void        Process_Identifier(
                            const TokIdentifier &
                                                i_rToken );
    virtual void        Process_Punctuation(
                            const TokPunctuation &
                                                i_rToken );
    virtual void        Process_BuiltInType(
                            const TokBuiltInType &
                                                i_rToken );
    virtual void        Process_ParameterHandling(
                            const TokParameterHandling &
                                                i_rToken );
    virtual void        Process_Raises();
    virtual void        Process_Default();

  private:
    enum E_State
    {
        e_none,
        e_start,
        in_return_type,
        expect_name,
        expect_params_list,
        expect_parameter,
        expect_parameter_variable,
        in_parameter_variable,
        expect_parameter_separator,
        params_finished,
        expect_exceptions_list,
        expect_exception,
        in_exception,
        expect_exception_separator,
        exceptions_finished
    };

    void                GoIntoReturnType();
    void                GoIntoParameterVariable();
    void                GoIntoException();
    void                OnDefault();

    virtual void        InitData();
    virtual void        ReceiveData();
    virtual void        TransferData();
    virtual UnoIDL_PE & MyPE();

    // DATA
    E_State             eState;

    String              sData_Name;
    ary::idl::Type_id   nData_ReturnType;
    bool                bData_Oneway;
    ary::idl::Function *
                        pCurFunction;

    const RParent *     pCurParent;

    Dyn<PE_Type>        pPE_Type;
    ary::idl::Type_id   nCurParsedType;     // ReturnType or Exception

    String              sName;

    Dyn<PE_Variable>    pPE_Variable;
    ary::idl::E_ParameterDirection
                        eCurParsedParam_Direction;
    ary::idl::Type_id   nCurParsedParam_Type;
    String              sCurParsedParam_Name;
    bool                bIsForConstructors;
};



}   // namespace uidl
}   // namespace csi


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
