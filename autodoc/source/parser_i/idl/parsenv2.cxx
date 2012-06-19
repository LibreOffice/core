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

#include <precomp.h>
#include <s2_luidl/parsenv2.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/ary.hxx>
#include <ary/getncast.hxx>
#include <ary/qualiname.hxx>
#include <ary/doc/d_oldidldocu.hxx>
#include <ary/idl/i_gate.hxx>
#include <ary/idl/i_ce.hxx>
#include <ary/idl/i_enum.hxx>
#include <ary/idl/i_enumvalue.hxx>
#include <ary/idl/i_module.hxx>
#include <ary/idl/ip_ce.hxx>
#include <parser/parserinfo.hxx>
#include <adc_msg.hxx>
#include <s2_luidl/uidl_tok.hxx>
#include <x_parse2.hxx>




namespace csi
{
namespace uidl
{


UnoIDL_PE::~UnoIDL_PE()
{
}

void
UnoIDL_PE::EstablishContacts( UnoIDL_PE *               io_pParentPE,
                              ary::Repository &         io_rRepository,
                              TokenProcessing_Result &  o_rResult )
{
    pRepository = &io_rRepository;
    aMyNode.EstablishContacts(io_pParentPE, io_rRepository.Gate_Idl(), o_rResult);
}

//void
//UnoIDL_PE::EstablishContacts( UnoIDL_PE *             io_pParentPE,
//                            ary::idl::Gate &          io_rGate,
//                            TokenProcessing_Result &  o_rResult )
//{
//  aMyNode.EstablishContacts(io_pParentPE, io_rGate, o_rResult);
//}

void
UnoIDL_PE::Enter( E_EnvStackAction  i_eWayOfEntering )
{
    switch (i_eWayOfEntering)
    {
        case push_sure:
                InitData();
                break;
        case push_try:
                csv_assert(false);
                break;
        case pop_success:
                ReceiveData();
                break;
        case pop_failure:
                throw X_AutodocParser(X_AutodocParser::x_Any);
                // no break because of throw
        default:
            csv_assert(false);
    }   // end switch
}

void
UnoIDL_PE::Leave( E_EnvStackAction  i_eWayOfLeaving )
{
    switch (i_eWayOfLeaving)
    {
        case push_sure:
                break;
        case push_try:
                csv_assert(false);
                break;
        case pop_success:
                TransferData();
                break;
        case pop_failure:
                throw X_AutodocParser(X_AutodocParser::x_Any);
                // no break because of throw
        default:
            csv_assert(false);
    }   // end switch
}

void
UnoIDL_PE::SetDocu( DYN ary::doc::OldIdlDocu * let_dpDocu )
{
    pDocu = let_dpDocu;
}

void
UnoIDL_PE::SetPublished()
{
    if (NOT pDocu)
    {
        pDocu = new ary::doc::OldIdlDocu;
    }
    pDocu->SetPublished();
}

void
UnoIDL_PE::SetOptional()
{
    if (NOT pDocu)
    {
        pDocu = new ary::doc::OldIdlDocu;
    }
    pDocu->SetOptional();
}

void
UnoIDL_PE::PassDocuAt( ary::idl::CodeEntity & io_rCe )
{
    if (pDocu)
    {
        io_rCe.Set_Docu(*pDocu.Release());
    }
    else if // KORR_FUTURE
            // Re-enable doc-warning for Enum Values, as soon as there is a
            //   @option -no-doc-for-enumvalues.
            (     NOT ary::is_type<ary::idl::Module>(io_rCe)
              AND NOT ary::is_type<ary::idl::Enum>(io_rCe)  )
    {
        TheMessages().Out_MissingDoc(
                        io_rCe.LocalName(),
                        ParseInfo().CurFile(),
                        ParseInfo().CurLine() );
    }
}

void
UnoIDL_PE::InitData()
{
    // Needs not anything to do.
}

void
UnoIDL_PE::ReceiveData()
{
    // Needs not anything to do.
}

const ary::idl::Module &
UnoIDL_PE::CurNamespace() const
{
    if ( Parent() != 0 )
        return Parent()->CurNamespace();
    else
    {
        csv_assert(false);
        return *(const ary::idl::Module*)0;
    }
}

const ParserInfo &
UnoIDL_PE::ParseInfo() const
{
    if ( Parent() != 0 )
        return Parent()->ParseInfo();
    else
    {
        csv_assert(false);
        return *(const ParserInfo*)0;
    }
}

UnoIDL_PE::UnoIDL_PE()
    :   aMyNode(),
        pDocu(),
        pRepository(0)
{
}


}   // namespace uidl
}   // namespace csi

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
