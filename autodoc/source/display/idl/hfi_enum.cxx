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
