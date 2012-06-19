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

#ifndef ADC_UIDL_PE_VARI2_HXX
#define ADC_UIDL_PE_VARI2_HXX



// USED SERVICES
    // BASE CLASSES
#include <s2_luidl/parsenv2.hxx>
#include <s2_luidl/pestate.hxx>
    // COMPONENTS
    // PARAMETERS


namespace csi
{
namespace uidl
{


class PE_Type;


class PE_Variable : public UnoIDL_PE,
                    public ParseEnvState
{
  public:
                        PE_Variable(
                            ary::idl::Type_id & i_rResult_Type,
                            String &            i_rResult_Name );
    virtual void        EstablishContacts(
                            UnoIDL_PE *         io_pParentPE,
                            ary::Repository &
                                                io_rRepository,
                            TokenProcessing_Result &
                                                o_rResult );
    virtual             ~PE_Variable();

    virtual void        ProcessToken(
                            const Token &       i_rToken );

    virtual void        Process_Default();
    virtual void        Process_Identifier(
                            const TokIdentifier &
                                                i_rToken );
    virtual void        Process_Punctuation(
                            const TokPunctuation &
                                                i_rToken );
    virtual void        Process_BuiltInType(
                            const TokBuiltInType &
                                                i_rToken );
  private:
    enum E_State
    {
        e_none,
        expect_type,
        expect_name,
        expect_finish
    };

    virtual void        InitData();
    virtual void        ReceiveData();
    virtual void        TransferData();
    virtual UnoIDL_PE & MyPE();

    // DATA
    E_State             eState;
    ary::idl::Type_id * pResult_Type;
    String *            pResult_Name;

    Dyn<PE_Type>        pPE_Type;
};



}   // namespace uidl
}   // namespace csi


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
