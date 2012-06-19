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
#include "hfi_constgroup.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/idl/i_ce.hxx>
#include <ary/idl/ik_constgroup.hxx>
#include <toolkit/hf_linachain.hxx>
#include <toolkit/hf_navi_sub.hxx>
#include <toolkit/hf_title.hxx>
#include "hfi_navibar.hxx"
#include "hfi_property.hxx"
#include "hi_linkhelper.hxx"


extern const String
    C_sCePrefix_Constants("constants group");


namespace
{

const String
    C_sList_Constants("Constants");
const String
    C_sList_Constants_Label("Constants");
const String
    C_sList_ConstantDetails("Constants' Details");
const String
    C_sList_ConstantDetails_Label("ConstantDetails");

enum E_SubListIndices
{
    sli_ConstantsSummary = 0,
    sli_ConstantDetails = 1
};


}   // anonymous namespace



HF_IdlConstGroup::HF_IdlConstGroup( Environment &   io_rEnv,
                                    Xml::Element &  o_rOut )
    :   HtmlFactory_Idl(io_rEnv, &o_rOut)
{
}

HF_IdlConstGroup::~HF_IdlConstGroup()
{
}

void
HF_IdlConstGroup::Produce_byData( const client & i_ce ) const
{
    Dyn<HF_NaviSubRow>
        pNaviSubRow( &make_Navibar(i_ce) );

    HF_TitleTable
        aTitle(CurOut());
    HF_LinkedNameChain
        aNameChain(aTitle.Add_Row());

    aNameChain.Produce_CompleteChain(Env().CurPosition(), nameChainLinker);
    produce_Title(aTitle, C_sCePrefix_Constants, i_ce);

    write_Docu(aTitle.Add_Row(), i_ce);
    CurOut() << new Html::HorizontalLine();

    dyn_ce_list
        dpConstants;
    ary::idl::ifc_constgroup::attr::Get_Constants(dpConstants, i_ce);

    if ( (*dpConstants).operator bool() )
    {
        produce_Members( *dpConstants,
                         C_sList_Constants,
                         C_sList_Constants_Label,
                         C_sList_ConstantDetails,
                         C_sList_ConstantDetails_Label );
        pNaviSubRow->SwitchOn(sli_ConstantsSummary);
        pNaviSubRow->SwitchOn(sli_ConstantDetails);
    }
    pNaviSubRow->Produce_Row();
}

HF_NaviSubRow &
HF_IdlConstGroup::make_Navibar( const client & i_ce ) const
{
    HF_IdlNavigationBar
        aNaviBar(Env(), CurOut());
    aNaviBar.Produce_CeMainRow(i_ce,true);  // true := avoid link to Use-page.

    DYN HF_NaviSubRow &
        ret = aNaviBar.Add_SubRow();
    ret.AddItem(C_sList_Constants, C_sList_Constants_Label, false);
    ret.AddItem(C_sList_ConstantDetails, C_sList_ConstantDetails_Label, false);

    CurOut() << new Html::HorizontalLine();
    return ret;
}

void
HF_IdlConstGroup::produce_MemberDetails( HF_SubTitleTable &  o_table,
                                         const client &      i_ce ) const
{
    HF_IdlConstant
        aElement( Env(), o_table );
    aElement.Produce_byData(i_ce);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
