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

#pragma once
#if 1



// USED SERVICES
    // BASE CLASSES
#include <s2_luidl/parsenv2.hxx>
#include <s2_luidl/pestate.hxx>
    // COMPONENTS
    // PARAMETERS

namespace ary
{
namespace idl
{
    class Interface;
}
}

namespace csi
{
namespace uidl
{



class PE_Function;
class PE_Attribute;
class PE_Type;

class PE_Interface : public UnoIDL_PE,
                     public ParseEnvState
{
  public:
                        PE_Interface();
    virtual             ~PE_Interface();

    virtual void        EstablishContacts(
                            UnoIDL_PE *         io_pParentPE,
                            ary::Repository &   io_rRepository,
                            TokenProcessing_Result &
                                                o_rResult );
    virtual void        ProcessToken(
                            const Token &       i_rToken );

    virtual void        Process_MetaType(
                            const TokMetaType & i_rToken );
    virtual void        Process_Identifier(
                            const TokIdentifier &
                                                i_rToken );
    virtual void        Process_Punctuation(
                            const TokPunctuation &
                                                i_rToken );
    virtual void        Process_NameSeparator();
    virtual void        Process_BuiltInType(
                            const TokBuiltInType &
                                                i_rToken );
    virtual void        Process_TypeModifier(
                            const TokTypeModifier &
                                                i_rToken );
    virtual void        Process_Stereotype(
                            const TokStereotype &
                                                i_rToken );
    virtual void        Process_Default();

  private:
    enum E_State        /// @ATTENTION  Do not change existing values (except of e_STATES_MAX) !!! Else array-indices will break.
    {
        e_none = 0,
        need_uik,
        uik,
        need_ident,
        ident,
        need_interface,
        need_name,
        wait_for_base,
        in_base,            // in header, after ":"
        need_curlbr_open,
        e_std,
        in_function,
        in_attribute,
        need_finish,
        in_base_interface,  // in body, after "interface"
        e_STATES_MAX
    };
    enum E_TokenType    /// @ATTENTION  Do not change existing values (except of tt_MAX) !!! Else array-indices will break.
    {
        tt_metatype = 0,
        tt_identifier = 1,
        tt_punctuation = 2,
        tt_startoftype = 3,
        tt_stereotype = 4,
        tt_MAX
    };
    typedef void (PE_Interface::*F_TOK)(const char *);


    void                On_need_uik_MetaType(const char * i_sText);
    void                On_uik_Identifier(const char * i_sText);
    void                On_uik_Punctuation(const char * i_sText);
    void                On_need_ident_MetaType(const char * i_sText);
    void                On_ident_Identifier(const char * i_sText);
    void                On_ident_Punctuation(const char * i_sText);
    void                On_need_interface_MetaType(const char * i_sText);
    void                On_need_name_Identifer(const char * i_sText);
    void                On_wait_for_base_Punctuation(const char * i_sText);
    void                On_need_curlbr_open_Punctuation(const char * i_sText);
    void                On_std_Metatype(const char * i_sText);
    void                On_std_Punctuation(const char * i_sText);
    void                On_std_Stereotype(const char * i_sText);
    void                On_std_GotoFunction(const char * i_sText);
    void                On_std_GotoAttribute(const char * i_sText);
    void                On_std_GotoBaseInterface(const char * i_sText);
    void                On_need_finish_Punctuation(const char * i_sText);
    void                On_Default(const char * i_sText);

    void                CallHandler(
                            const char *        i_sTokenText,
                            E_TokenType         i_eTokenType );

    virtual void        InitData();
    virtual void        TransferData();
    virtual void        ReceiveData();
    virtual UnoIDL_PE & MyPE();

    void                store_Interface();

    // DATA
    static F_TOK        aDispatcher[e_STATES_MAX][tt_MAX];

    E_State             eState;
    String              sData_Name;
    bool                bIsPreDeclaration;
    ary::idl::Interface *
                        pCurInterface;
    ary::idl::Ce_id     nCurInterface;

    Dyn<PE_Function>    pPE_Function;
    Dyn<PE_Attribute>   pPE_Attribute;

    Dyn<PE_Type>        pPE_Type;
    ary::idl::Type_id   nCurParsed_Base;
    bool                bOptionalMember;
};



// IMPLEMENTATION


}   // namespace uidl
}   // namespace csi


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
