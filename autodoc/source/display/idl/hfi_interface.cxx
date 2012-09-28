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
#include "hfi_interface.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/idl/i_ce.hxx>
#include <ary/idl/i_gate.hxx>
#include <ary/idl/ik_function.hxx>
#include <ary/idl/ik_interface.hxx>
#include <ary/idl/ip_ce.hxx>
#include <ary/idl/ip_type.hxx>
#include <toolkit/hf_docentry.hxx>
#include <toolkit/hf_linachain.hxx>
#include <toolkit/hf_navi_sub.hxx>
#include <toolkit/hf_title.hxx>
#include "hfi_doc.hxx"
#include "hfi_hierarchy.hxx"
#include "hfi_method.hxx"
#include "hfi_navibar.hxx"
#include "hfi_property.hxx"
#include "hfi_tag.hxx"
#include "hfi_typetext.hxx"
#include "hi_linkhelper.hxx"


extern const String
    C_sCePrefix_Interface("interface");

namespace
{

const String
    C_sBaseInterface("Base Interfaces");
const String
    C_sList_BaseComments("Comments on Base Interfaces");
const String
    C_sList_Methods("Methods' Summary");
const String
    C_sList_Methods_Label("MethodsSummary");
const String
    C_sDetails_Methods("Methods' Details");
const String
    C_sDetails_Methods_Label("MethodsDetails");

const String
    C_sList_Attributes("Attributes' Summary");
const String
    C_sList_Attributes_Label("AttributesSummary");
const String
    C_sList_AttributesDetails("Attributes' Details");
const String
    C_sList_AttributesDetails_Label("AttributesDetails");



enum E_SubListIndices
{
    sli_MethodsSummay = 0,
    sli_AttributesSummary = 1,
    sli_MethodDetails = 2,
    sli_AttributesDetails = 3
};

} //anonymous namespace




HF_IdlInterface::HF_IdlInterface( Environment &         io_rEnv,
                                  Xml::Element &        o_rOut )
    :   HtmlFactory_Idl(io_rEnv, &o_rOut),
        eCurProducedMembers(mem_none)
{
}

HF_IdlInterface::~HF_IdlInterface()
{
}

void
HF_IdlInterface::Produce_byData( const client & i_ce ) const
{
    Dyn<HF_NaviSubRow>
        pNaviSubRow( &make_Navibar(i_ce) );

    HF_TitleTable
        aTitle(CurOut());

    HF_LinkedNameChain
        aNameChain(aTitle.Add_Row());
    aNameChain.Produce_CompleteChain(Env().CurPosition(), nameChainLinker);

    produce_Title(aTitle, C_sCePrefix_Interface, i_ce);

    produce_BaseHierarchy( aTitle.Add_Row(),
                           i_ce,
                           C_sBaseInterface );

    write_Docu(aTitle.Add_Row(), i_ce);
    CurOut() << new Html::HorizontalLine();

    dyn_ce_list dpFunctions;
    ary::idl::ifc_interface::attr::Get_Functions(dpFunctions, i_ce);
    if ( (*dpFunctions).operator bool() )
    {
        eCurProducedMembers = mem_Functions;

        produce_Members( *dpFunctions,
                         C_sList_Methods,
                         C_sList_Methods_Label,
                         C_sDetails_Methods,
                         C_sDetails_Methods_Label,
                         HtmlFactory_Idl::viewtype_summary );
        pNaviSubRow->SwitchOn(sli_MethodsSummay);
    }

    dyn_ce_list
        dpAttributes;
    ary::idl::ifc_interface::attr::Get_Attributes(dpAttributes, i_ce);
    if ( (*dpAttributes).operator bool() )
    {
        eCurProducedMembers = mem_Attributes;

        produce_Members( *dpAttributes,
                         C_sList_Attributes,
                         C_sList_Attributes_Label,
                         C_sList_AttributesDetails,
                         C_sList_AttributesDetails_Label,
                         HtmlFactory_Idl::viewtype_summary );
        pNaviSubRow->SwitchOn(sli_AttributesSummary);
    }

    ary::idl::ifc_interface::attr::Get_Functions(dpFunctions, i_ce);
    if ( (*dpFunctions).operator bool() )
    {
        eCurProducedMembers = mem_Functions;

        produce_Members( *dpFunctions,
                         C_sList_Methods,
                         C_sList_Methods_Label,
                         C_sDetails_Methods,
                         C_sDetails_Methods_Label,
                         HtmlFactory_Idl::viewtype_details );
        pNaviSubRow->SwitchOn(sli_MethodDetails);
    }

    ary::idl::ifc_interface::attr::Get_Attributes(dpAttributes, i_ce);
    if ( (*dpAttributes).operator bool() )
    {
        eCurProducedMembers = mem_Attributes;

        produce_Members( *dpAttributes,
                         C_sList_Attributes,
                         C_sList_Attributes_Label,
                         C_sList_AttributesDetails,
                         C_sList_AttributesDetails_Label,
                         HtmlFactory_Idl::viewtype_details );
        pNaviSubRow->SwitchOn(sli_AttributesDetails);
    }

    eCurProducedMembers = mem_none;

    pNaviSubRow->Produce_Row();
}

DYN HF_NaviSubRow &
HF_IdlInterface::make_Navibar( const client & i_ce ) const
{
    HF_IdlNavigationBar
        aNaviBar(Env(), CurOut());
    aNaviBar.Produce_CeMainRow(i_ce);

    DYN HF_NaviSubRow &
        ret = aNaviBar.Add_SubRow();
    ret.AddItem(C_sList_Methods, C_sList_Methods_Label, false);
    ret.AddItem(C_sList_Attributes, C_sList_Attributes_Label, false);
    ret.AddItem(C_sDetails_Methods, C_sDetails_Methods_Label, false);
    ret.AddItem(C_sList_AttributesDetails, C_sList_AttributesDetails_Label, false);

    CurOut() << new Html::HorizontalLine();
    return ret;
}

void
HF_IdlInterface::produce_MemberDetails( HF_SubTitleTable &  o_table,
                                        const client &      i_ce ) const
{
    switch (eCurProducedMembers)
    {
        case mem_Functions:
                    break;
        case mem_Attributes:
        {
                    HF_IdlAttribute
                        aAttribute( Env(), o_table);
                    aAttribute.Produce_byData( i_ce );
                    return;
        };
        default:    //Won't happen.
                    return;
    }   // end switch

    typedef ary::idl::ifc_function::attr    funcAttr;

    HF_IdlMethod
        aFunction(  Env(),
                    o_table.Add_Row()
                        >> *new Html::TableCell
                           << new Html::ClassAttr(C_sCellStyle_MDetail) );

    ary::Dyn_StdConstIterator<ary::idl::Parameter>
        pParameters;
    funcAttr::Get_Parameters(pParameters, i_ce);

    ary::Dyn_StdConstIterator<ary::idl::Type_id>
        pExceptions;
    funcAttr::Get_Exceptions(pExceptions, i_ce);

    aFunction.Produce_byData( i_ce.LocalName(),
                              funcAttr::ReturnType(i_ce),
                              *pParameters,
                              *pExceptions,
                              funcAttr::IsOneway(i_ce),
                              funcAttr::HasEllipse(i_ce),
                              i_ce );
}

void
HF_IdlInterface::produce_BaseHierarchy( Xml::Element &      o_screen,
                                        const client &      i_ce,
                                        const String &      i_sLabel ) const
{
    ary::Dyn_StdConstIterator<ary::idl::CommentedRelation>
        pHelp;
    ary::idl::ifc_interface::attr::Get_Bases(pHelp, i_ce);
    if (NOT (*pHelp).operator bool())
        return;

    // Check for XInterface as only base:
    ary::StdConstIterator<ary::idl::CommentedRelation> &
        itTest = *pHelp;
    ary::idl::Ce_id
        nCe = Env().Gate().Types().Search_CeRelatedTo((*itTest).Type());
    if (nCe.IsValid())
    {
        // KORR_FUTURE
        //   Rather check for id(!) of com::sun::star::uno::XInterface.
        if (Env().Gate().Ces().Find_Ce(nCe).LocalName() == "XInterface")
        {
            ++itTest;
            if (NOT itTest.operator bool())
                return;
        }
    }

    // Write hierarchy:

    HF_DocEntryList
        aDocList( o_screen );
    aDocList.Produce_Term(i_sLabel);
    Xml::Element &
        rBaseList = aDocList.Produce_Definition();

// NEW
    Write_BaseHierarchy(rBaseList, Env(), i_ce);

    // Write comments:
        // KORR_FUTURE: Make sure, no empty table is constructed when comments list is empty.
    HF_SubTitleTable
        aBaseTable( aDocList.Produce_Definition(),
                "",
                C_sList_BaseComments,
                2,
                HF_SubTitleTable::sublevel_3 );

    ary::Dyn_StdConstIterator<ary::idl::CommentedRelation>
        pBases;
    ary::idl::ifc_interface::attr::Get_Bases(pBases, i_ce);
    for ( ary::StdConstIterator<ary::idl::CommentedRelation> & it = *pBases;
          it.operator bool();
          ++it )
    {
        Xml::Element &
            rRow = aBaseTable.Add_Row();

        Xml::Element &
            rTerm = rRow
                        >> *new Html::TableCell
                            << new Html::ClassAttr(C_sCellStyle_SummaryLeft);
        HF_IdlTypeText
            aTypeDisplay( Env(), rTerm, 0);
        aTypeDisplay.Produce_byData((*it).Type());

        Xml::Element &
            rDocu = rRow
                        >> *new Html::TableCell
                            << new Html::ClassAttr(C_sCellStyle_SummaryRight);

        HF_DocEntryList
            aDocuList(rDocu);

        if ((*it).Info() != 0)
        {
//          aDocuList.Produce_Term("Comment on Base Reference");

            HF_IdlDocu
                aDocuDisplay(Env(), aDocuList);
            aDocuDisplay.Produce_fromReference(*(*it).Info(), i_ce);
        }
        else
        {
            const client *
                pCe = Env().Linker().Search_CeFromType((*it).Type());
            const ce_info *
                pShort = pCe != 0
                            ?   Get_IdlDocu(pCe->Docu())
                            :   (const ce_info *)(0);
            if ( pShort != 0 )
            {
                aDocuList.Produce_NormalTerm("(referenced interface's summary:)");

                Xml::Element &
                    rDef = aDocuList.Produce_Definition();
                HF_IdlDocuTextDisplay
                    aShortDisplay( Env(), &rDef, *pCe);
                pShort->Short().DisplayAt(aShortDisplay);
            }   // end if (pShort != 0)
        }   // endif ( (*i_commentedRef).Info() != 0 ) else
    }   // end for
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
