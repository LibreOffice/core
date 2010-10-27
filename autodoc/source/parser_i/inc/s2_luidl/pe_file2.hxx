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
