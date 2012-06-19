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

#ifndef LUIDL_PE_FILE2_HXX
#define LUIDL_PE_FILE2_HXX



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
class Module;
}   // namespace idl
}   // namespace ary


namespace csi
{
namespace uidl
{

class TokenDistributor;
class PE_Service;
class PE_Singleton;
class PE_Interface;
class PE_Struct;
class PE_Exception;
class PE_Constant;
class PE_Enum;
class PE_Typedef;


class PE_File : public UnoIDL_PE,
                public ParseEnvState
{
  public:
                        PE_File(
                            TokenDistributor &  i_rTokenAdmin,
                            const ParserInfo &  i_parseInfo );
    virtual void        EstablishContacts(
                            UnoIDL_PE *         io_pParentPE,
                            ary::Repository &   io_rRepository,
                            TokenProcessing_Result &
                                                o_rResult );
                        ~PE_File();

    virtual void        ProcessToken(
                            const Token &       i_rToken );

    virtual void        Process_Identifier(
                            const TokIdentifier &
                                                i_rToken );
    virtual void        Process_Punctuation(
                            const TokPunctuation &
                                                i_rToken );
    virtual void        Process_MetaType(
                            const TokMetaType & i_rToken );
    virtual void        Process_Stereotype(
                            const TokStereotype &
                                                i_rToken );
    virtual void        Process_Default();

  private:
    enum E_State
    {
        e_none,
        e_std,
        wait_for_module,
        wait_for_module_bracket,
        wait_for_module_semicolon,
        in_sub_pe,
        on_default
    };

    virtual void        InitData();
    virtual void        TransferData();
    virtual void        ReceiveData();
    virtual UnoIDL_PE & MyPE();
    virtual const ary::idl::Module &
                        CurNamespace() const;
    virtual const ParserInfo &
                        ParseInfo() const;
    // DATA
    TokenDistributor *  pTokenAdmin;
    Dyn<PE_Service>     pPE_Service;
    Dyn<PE_Singleton>   pPE_Singleton;
    Dyn<PE_Interface>   pPE_Interface;
    Dyn<PE_Struct>      pPE_Struct;
    Dyn<PE_Exception>   pPE_Exception;
    Dyn<PE_Constant>    pPE_Constant;
    Dyn<PE_Enum>        pPE_Enum;
    Dyn<PE_Typedef>     pPE_Typedef;

    const ary::idl::Module *
                        pCurNamespace;
    const ParserInfo *  pParseInfo;

    E_State             eState;
    uintt               nBracketCount_inDefMode;
};


}   // namespace uidl
}   // namespace csi

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
