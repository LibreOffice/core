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

#include <precomp.h>
#include "hfi_siservice.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/idl/i_ce.hxx>
#include <ary/idl/ik_function.hxx>
#include <ary/idl/ik_siservice.hxx>
#include <ary/doc/d_oldidldocu.hxx>
#include <toolkit/hf_docentry.hxx>
#include <toolkit/hf_linachain.hxx>
#include <toolkit/hf_navi_sub.hxx>
#include <toolkit/hf_title.hxx>
#include "hfi_doc.hxx"
#include "hfi_method.hxx"
#include "hfi_navibar.hxx"
#include "hfi_typetext.hxx"
#include "hi_env.hxx"
#include "hi_linkhelper.hxx"



namespace
{

const String
    C_sImplementedInterface("Supported Interface");

const String
    C_sList_Constructors("Constructors' Summary");
const String
    C_sList_Constructors_Label("ConstructorsSummary");
const String
    C_sDetails_Constructors("Constructors' Details");
const String
    C_sDetails_Constructors_Label("ConstructorsDetails");


enum E_SubListIndices
{
    sli_ConstructorsSummary = 0,
    sli_ConstructorsDetails = 1
};

} //anonymous namespace


HF_IdlSglIfcService::HF_IdlSglIfcService( Environment &         io_rEnv,
                                          Xml::Element &        o_rOut )
    :   HtmlFactory_Idl(io_rEnv, &o_rOut)
{
}

HF_IdlSglIfcService::~HF_IdlSglIfcService()
{
}

typedef ::ary::idl::ifc_sglifcservice::attr SglIfcServiceAttr;

void
HF_IdlSglIfcService::Produce_byData( const client & i_ce ) const
{
    Dyn<HF_NaviSubRow>
        pNaviSubRow( &make_Navibar(i_ce) );

    HF_TitleTable
        aTitle(CurOut());
    HF_LinkedNameChain
        aNameChain(aTitle.Add_Row());

    aNameChain.Produce_CompleteChain(Env().CurPosition(), nameChainLinker);
    produce_Title(aTitle, C_sCePrefix_Service, i_ce);

    HF_DocEntryList
        aTopList( aTitle.Add_Row() );
    aTopList.Produce_Term(C_sImplementedInterface);

    HF_IdlTypeText
        aImplementedInterface( Env(), aTopList.Produce_Definition(), true, &i_ce);
    aImplementedInterface.Produce_byData( SglIfcServiceAttr::BaseInterface(i_ce) );

    CurOut() << new Html::HorizontalLine;

    write_Docu(aTitle.Add_Row(), i_ce);
    CurOut() << new Html::HorizontalLine();

    dyn_ce_list
        dpConstructors;
    SglIfcServiceAttr::Get_Constructors(dpConstructors, i_ce);
    if ( (*dpConstructors).operator bool() )
    {
        produce_Members( *dpConstructors,
                         C_sList_Constructors,
                         C_sList_Constructors_Label,
                         C_sDetails_Constructors,
                         C_sDetails_Constructors_Label );
        pNaviSubRow->SwitchOn(sli_ConstructorsSummary);
        pNaviSubRow->SwitchOn(sli_ConstructorsDetails);
    }

    pNaviSubRow->Produce_Row();
    CurOut() << new Xml::XmlCode("<br>&nbsp;");
}

DYN HF_NaviSubRow &
HF_IdlSglIfcService::make_Navibar( const client & i_ce ) const
{
    HF_IdlNavigationBar
        aNaviBar(Env(), CurOut());
    aNaviBar.Produce_CeMainRow(i_ce, true);

    DYN HF_NaviSubRow &
        ret = aNaviBar.Add_SubRow();
    ret.AddItem(C_sList_Constructors, C_sList_Constructors_Label, false);
    ret.AddItem(C_sDetails_Constructors, C_sDetails_Constructors_Label, false);

    CurOut() << new Html::HorizontalLine();
    return ret;
}

typedef ary::idl::ifc_function::attr    funcAttr;

void
HF_IdlSglIfcService::produce_MemberDetails( HF_SubTitleTable &  o_table,
                                            const client &      i_ce ) const
{
    HF_IdlMethod
        aConstructor( Env(),
                      o_table.Add_Row()
                            >> *new Html::TableCell
                                << new Html::ClassAttr(C_sCellStyle_MDetail) );

    ary::Dyn_StdConstIterator<ary::idl::Parameter>
        pParameters;
    funcAttr::Get_Parameters(pParameters, i_ce);

    ary::Dyn_StdConstIterator<ary::idl::Type_id>
        pExceptions;
    funcAttr::Get_Exceptions(pExceptions, i_ce);

    aConstructor.Produce_byData( i_ce.LocalName(),
                                 funcAttr::ReturnType(i_ce),
                                 *pParameters,
                                 *pExceptions,
                                 funcAttr::IsOneway(i_ce),
                                 funcAttr::HasEllipse(i_ce),
                                 i_ce );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
