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

#ifndef ADC_UIDL_PE_ENUM2_HXX
#define ADC_UIDL_PE_ENUM2_HXX

#include <s2_luidl/parsenv2.hxx>
#include <s2_luidl/pestate.hxx>


namespace csi
{
namespace uidl
{

class PE_Value;

class PE_Enum : public UnoIDL_PE,
                    public ParseEnvState
{
  public:
                        PE_Enum();
    virtual void        EstablishContacts(
                            UnoIDL_PE *         io_pParentPE,
                            ary::Repository &   io_rRepository,
                            TokenProcessing_Result &
                                                o_rResult );
                        ~PE_Enum();

    virtual void        ProcessToken(
                            const Token &       i_rToken );

    virtual void        Process_Identifier(
                            const TokIdentifier &
                                                i_rToken );
    virtual void        Process_Punctuation(
                            const TokPunctuation &
                                                i_rToken );

  private:
    enum E_State
    {
        e_none,
        expect_name,
        expect_curl_bracket_open,
        expect_value,
        expect_finish,
        e_STATES_MAX
    };
    enum E_TokenType
    {
        tt_identifier,
        tt_punctuation,
        tt_MAX
    };
    typedef void (PE_Enum::*F_TOK)(const char *);


    void                CallHandler(
                            const char *        i_sTokenText,
                            E_TokenType         i_eTokenType );

    void                On_expect_name_Identifier(const char * i_sText);
    void                On_expect_curl_bracket_open_Punctuation(const char * i_sText);
    void                On_expect_value_Punctuation(const char * i_sText);
    void                On_expect_value_Identifier(const char * i_sText);
    void                On_expect_finish_Punctuation(const char * i_sText);
    void                On_Default(const char * );

    void                EmptySingleValueData();
    void                CreateSingleValue();

    virtual void        InitData();
    virtual void        ReceiveData();
    virtual void        TransferData();
    virtual UnoIDL_PE & MyPE();

  // DATA
    static F_TOK        aDispatcher[e_STATES_MAX][tt_MAX];

    E_State             eState;

    String              sData_Name;
    ary::idl::Ce_id     nDataId;

    Dyn<PE_Value>       pPE_Value;
    String              sName;
    String              sAssignment;
};



}   // namespace uidl
}   // namespace csi


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
