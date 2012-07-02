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

#ifndef LUIDL_PE_EVALU_HXX
#define LUIDL_PE_EVALU_HXX



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

class PE_Value : public UnoIDL_PE,
                 public ParseEnvState
{
  public:
                        PE_Value(
                            String &            o_rName,
                            String &            o_rAssignment,
                            bool                i_bIsConst );
    virtual void        EstablishContacts(
                            UnoIDL_PE *         io_pParentPE,
                            ary::Repository &
                                                io_rRepository,
                            TokenProcessing_Result &
                                                o_rResult );
                        ~PE_Value();

    virtual void        ProcessToken(
                            const Token &       i_rToken );

    virtual void        Process_Identifier(
                            const TokIdentifier &
                                                i_rToken );
    virtual void        Process_Punctuation(
                            const TokPunctuation &
                                                i_rToken );
    virtual void        Process_Assignment(
                            const TokAssignment &
                                                i_rToken );
  private:
    enum E_State
    {
        e_none = 0,
        expect_name,
        got_name,
        e_STATES_MAX
    };
    enum E_TokenType    /// @ATTENTION  Do not change existing values (except of tt_MAX) !!! Else array-indices will break.
    {
        tt_identifier = 0,
        tt_punctuation = 1,
        tt_assignment = 2,
        tt_MAX
    };
    typedef void (PE_Value::*F_TOK)(const char *);


    void                CallHandler(
                            const char *        i_sTokenText,
                            E_TokenType         i_eTokenType );

    void                On_expect_name_Identifier(const char * i_sText);
    void                On_got_name_Punctuation(const char * i_sText);
    void                On_got_name_Assignment(const char * i_sText);
    void                On_Default(const char * );

    virtual void        InitData();
    virtual void        TransferData();
    virtual UnoIDL_PE & MyPE();

    bool                IsConst() const         { return bIsConst; }

    static F_TOK        aDispatcher[e_STATES_MAX][tt_MAX];

    E_State             eState;
    String  *           pName;
    String  *           pAssignment;
    bool                bIsConst;
};



}   // namespace uidl
}   // namespace csi


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
