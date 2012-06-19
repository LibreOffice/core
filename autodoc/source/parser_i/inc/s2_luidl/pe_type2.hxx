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
