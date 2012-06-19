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

#ifndef LUIDL_PE_TYDF2_HXX
#define LUIDL_PE_TYDF2_HXX



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


class PE_Typedef : public UnoIDL_PE,
                   public ParseEnvState
{
  public:
                        PE_Typedef();
    virtual void        EstablishContacts(
                            UnoIDL_PE *         io_pParentPE,
                            ary::Repository &   io_rRepository,
                            TokenProcessing_Result &
                                                o_rResult );
                        ~PE_Typedef();

    virtual void        ProcessToken(
                            const Token &       i_rToken );

    virtual void        Process_Identifier(
                            const TokIdentifier &
                                                i_rToken );
    virtual void        Process_Punctuation(
                            const TokPunctuation &
                                                i_rToken );
    virtual void        Process_Default();

  private:
    enum E_State
    {
        e_none = 0,
        expect_description,
        expect_name,
        got_name,
        e_STATES_MAX
    };
    enum E_TokenType    /// @ATTENTION  Do not change existing values (except of tt_MAX) !!! Else array-indices will break.
    {
        tt_any = 0,
        tt_identifier,
        tt_punctuation,
        tt_MAX
    };
    typedef void (PE_Typedef::*F_TOK)(const char *);


    void                CallHandler(
                            const char *        i_sTokenText,
                            E_TokenType         i_eTokenType );

    void                On_expect_description_Any(const char * i_sText);
    void                On_expect_name_Identifier(const char * i_sText);
    void                On_got_name_Punctuation(const char * i_sText);
    void                On_Default(const char * );

    virtual void        InitData();
    virtual void        ReceiveData();
    virtual void        TransferData();
    virtual UnoIDL_PE & MyPE();

    // DATA
    static F_TOK        aDispatcher[e_STATES_MAX][tt_MAX];

    E_State             eState;
    Dyn<PE_Type>        pPE_Type;
    ary::idl::Type_id   nType;
    String              sName;
};



}   // namespace uidl
}   // namespace csi



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
