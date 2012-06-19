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
#include "hfi_enum.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/idl/i_ce.hxx>
#include <ary/idl/ik_enum.hxx>
#include <toolkit/hf_linachain.hxx>
#include <toolkit/hf_navi_sub.hxx>
#include <toolkit/hf_title.hxx>
#include "hfi_navibar.hxx"
#include "hfi_property.hxx"
#include "hi_linkhelper.hxx"


extern const String
    C_sCePrefix_Enum("enum");

namespace
{

const String
    C_sList_Values("Values");
const String
    C_sList_Values_Label("Values");
const String
    C_sList_ValueDetails("Values' Details");
const String
    C_sList_ValueDetails_Label("ValueDetails");

enum E_SubListIndices
{
    sli_ValuesSummary = 0,
    sli_ValueDetails = 1
};

}   // anonymous namespace

HF_IdlEnum::HF_IdlEnum( Environment &       io_rEnv,
                        Xml::Element &      o_rOut )
    :   HtmlFactory_Idl(io_rEnv, &o_rOut)
{
}

HF_IdlEnum::~HF_IdlEnum()
{
}

void
HF_IdlEnum::Produce_byData( const client & i_ce ) const
{
    Dyn<HF_NaviSubRow>
        pNaviSubRow( &make_Navibar(i_ce) );

    HF_TitleTable
        aTitle(CurOut());

    HF_LinkedNameChain
        aNameChain(aTitle.Add_Row());

    aNameChain.Produce_CompleteChain(Env().CurPosition(), nameChainLinker);
    produce_Title(aTitle, C_sCePrefix_Enum, i_ce);

    write_Docu(aTitle.Add_Row(), i_ce);
    CurOut() << new Html::HorizontalLine();

    dyn_ce_list
        dpValues;
    ary::idl::ifc_enum::attr::Get_Values(dpValues, i_ce);
    if ( (*dpValues).operator bool() )
    {
        produce_Members( *dpValues,
                         C_sList_Values,
                         C_sList_Values_Label,
                         C_sList_ValueDetails,
                         C_sList_ValueDetails_Label );
        pNaviSubRow->SwitchOn(sli_ValuesSummary);
        pNaviSubRow->SwitchOn(sli_ValueDetails);
    }
    pNaviSubRow->Produce_Row();
}

HF_NaviSubRow &
HF_IdlEnum::make_Navibar( const client & i_ce ) const
{
    HF_IdlNavigationBar
        aNaviBar(Env(), CurOut());
    aNaviBar.Produce_CeMainRow(i_ce);

    DYN HF_NaviSubRow &
        ret = aNaviBar.Add_SubRow();
    ret.AddItem(C_sList_Values, C_sList_Values_Label, false);
    ret.AddItem(C_sList_ValueDetails, C_sList_ValueDetails_Label, false);

    CurOut() << new Html::HorizontalLine();
    return ret;
}

void
HF_IdlEnum::produce_MemberDetails( HF_SubTitleTable &  o_table,
                                   const client &      i_ce) const
{
    HF_IdlEnumValue
        aElement( Env(), o_table );
    aElement.Produce_byData(i_ce);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
