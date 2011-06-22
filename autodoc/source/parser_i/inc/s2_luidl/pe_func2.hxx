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

#ifndef ADC_UIDL_PE_FUNC2_HXX
#define ADC_UIDL_PE_FUNC2_HXX

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
