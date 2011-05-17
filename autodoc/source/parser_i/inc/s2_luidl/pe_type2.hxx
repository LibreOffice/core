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

#ifndef ADC_PE_TYPE2_HXX
#define ADC_PE_TYPE2_HXX



// USED SERVICES
    // BASE CLASSES
#include<s2_luidl/parsenv2.hxx>
#include<s2_luidl/pestate.hxx>
    // COMPONENTS
#include<ary/qualiname.hxx>
    // PARAMETERS


namespace csi
{
namespace uidl
{


class PE_Type : public UnoIDL_PE,
                public ParseEnvState
{
  public:
                         PE_Type(
                            ary::idl::Type_id & o_rResult );
    virtual             ~PE_Type();

    virtual void        ProcessToken(
                            const Token &       i_rToken );

    virtual void        Process_Identifier(
                            const TokIdentifier &
                                                i_rToken );
    virtual void        Process_NameSeparator();
    virtual void        Process_Punctuation(
                            const TokPunctuation &
                                                i_rToken );
    virtual void        Process_BuiltInType(
                            const TokBuiltInType &
                                                i_rToken );
    virtual void        Process_TypeModifier(
                            const TokTypeModifier &
                                                i_rToken );
    virtual void        Process_Default();

  private:
    enum E_State
    {
        e_none = 0,
        expect_type,
        expect_quname_part,
        expect_quname_separator,
        in_template_type
    };

    void                Finish();
    PE_Type &           MyTemplateType();

    virtual void        InitData();
    virtual void        TransferData();
    virtual UnoIDL_PE & MyPE();

    // DATA
    ary::idl::Type_id * pResult;

    uintt               nIsSequenceCounter;
    uintt               nSequenceDownCounter;
    bool                bIsUnsigned;
    ary::QualifiedName  sFullType;

    E_State             eState;
    String              sLastPart;

    Dyn<PE_Type>        pPE_TemplateType;   /// @attention Recursion, only initiate, if needed!
    ary::idl::Type_id   nTemplateType;
    std::vector<ary::idl::Type_id>
                        aTemplateParameters;
};



// IMPLEMENTATION


}   // namespace uidl
}   // namespace csi

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
