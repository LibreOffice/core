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

#ifndef LUIDL_PE_SELEM_HXX
#define LUIDL_PE_SELEM_HXX



// USED SERVICES
    // BASE CLASSES
#include <s2_luidl/parsenv2.hxx>
#include <s2_luidl/pestate.hxx>
    // COMPONENTS
    // PARAMETERS
#include <ary/idl/i_gate.hxx>


namespace udm {
class Agent_Struct;
}   // namespace udm


namespace csi
{
namespace uidl
{

class PE_Type;
class StructElement;
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
