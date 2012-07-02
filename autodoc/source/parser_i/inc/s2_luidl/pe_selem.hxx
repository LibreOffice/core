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

#ifndef LUIDL_PE_SELEM_HXX
#define LUIDL_PE_SELEM_HXX



// USED SERVICES
    // BASE CLASSES
#include <s2_luidl/parsenv2.hxx>
#include <s2_luidl/pestate.hxx>
    // COMPONENTS
    // PARAMETERS
#include <ary/idl/i_gate.hxx>


namespace csi
{
namespace uidl
{

class PE_Type;
class Struct;

class PE_StructElement : public UnoIDL_PE,
                         public ParseEnvState
{
  public:
    typedef ary::idl::Ce_id       RStructElement;
    typedef ary::idl::Ce_id       RStruct;

                        PE_StructElement(       /// Use for Struct-elements
                            RStructElement &    o_rResult,
                            const RStruct &     i_rCurStruct,
                            const String &      i_rCurStructTemplateParam );
                        PE_StructElement(       /// Use for Exception-elements
                            RStructElement &    o_rResult,
                            const RStruct &     i_rCurExc );
    virtual void        EstablishContacts(
                            UnoIDL_PE *         io_pParentPE,
                            ary::Repository &   io_rRepository,
                            TokenProcessing_Result &
                                                o_rResult );
                        ~PE_StructElement();

    virtual void        ProcessToken(
                            const Token &       i_rToken );

    virtual void        Process_Default();
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
        expect_type,
        expect_name,
        expect_finish
    };

    virtual void        InitData();
    virtual void        TransferData();
    virtual UnoIDL_PE & MyPE();

    ary::idl::Type_id   lhf_FindTemplateParamType() const;

    // DATA
    E_State             eState;
    RStructElement *    pResult;
    const RStruct *     pCurStruct;
    bool                bIsExceptionElement;

    Dyn<PE_Type>        pPE_Type;
    ary::idl::Type_id   nType;
    String              sName;
    const String *      pCurStructTemplateParam;
};



}   // namespace uidl
}   // namespace csi


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
