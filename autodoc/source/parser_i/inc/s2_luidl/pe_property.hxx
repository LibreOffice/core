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

#ifndef ADC_UIDL_PE_PROPERTY_HXX
#define ADC_UIDL_PE_PROPERTY_HXX



// USED SERVICES
    // BASE CLASSES

#include <s2_luidl/parsenv2.hxx>
#include <s2_luidl/pestate.hxx>
    // COMPONENTS
#include <ary/idl/i_property.hxx>
    // PARAMETERS
#include <ary/idl/i_gate.hxx>


namespace csi
{
namespace uidl
{


class PE_Variable;

class PE_Property : public UnoIDL_PE,
                    public ParseEnvState
{
  public:
    typedef ary::idl::Ce_id                     Ce_id;
    typedef ary::idl::Type_id                   Type_id;
    typedef ary::idl::Property::Stereotypes     Stereotypes;


                        PE_Property(
                            const Ce_id &       i_rCurOwner );

    virtual void        EstablishContacts(
                            UnoIDL_PE *         io_pParentPE,
                            ary::Repository &
                                                io_rRepository,
                            TokenProcessing_Result &
                                                o_rResult );
    virtual             ~PE_Property();

    virtual void        ProcessToken(
                            const Token &       i_rToken );

    virtual void        Process_Stereotype(
                            const TokStereotype &
                                                i_rToken );
    virtual void        Process_MetaType(
                            const TokMetaType & i_rToken );
    virtual void        Process_Punctuation(
                            const TokPunctuation &
                                                i_rToken );
    virtual void        Process_Default();

    void                PresetOptional()        { bIsOptional = true; }
    void                PresetStereotypes(
                            Stereotypes::E_Flags
                                                i_eFlag )
                                                { aStereotypes.Set_Flag(i_eFlag); }
  private:
    enum E_State
    {
        e_none,
        e_start,
        expect_variable,
        in_variable
    };

    virtual void        InitData();
    virtual void        ReceiveData();
    virtual void        TransferData();
    virtual UnoIDL_PE & MyPE();

    // DATA
    E_State             eState;
    const Ce_id *       pCurOwner;

    Dyn<PE_Variable>    pPE_Variable;

        // object-data
    Type_id             nCurParsedType;
    String              sCurParsedName;
    bool                bIsOptional;
    Stereotypes         aStereotypes;
};


}   // namespace uidl
}   // namespace csi


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
