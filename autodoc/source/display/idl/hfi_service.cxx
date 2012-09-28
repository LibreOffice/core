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
#include "hfi_service.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/idl/i_ce.hxx>
#include <ary/idl/ik_property.hxx>
#include <ary/idl/ik_service.hxx>
#include <toolkit/hf_docentry.hxx>
#include <toolkit/hf_linachain.hxx>
#include <toolkit/hf_navi_sub.hxx>
#include <toolkit/hf_title.hxx>
#include "hfi_doc.hxx"
#include "hfi_navibar.hxx"
#include "hfi_property.hxx"
#include "hfi_tag.hxx"
#include "hfi_typetext.hxx"
#include "hi_linkhelper.hxx"




extern const String
    C_sCePrefix_Service("service");

namespace
{

const String
    C_sList_IncludedServices("Services' Summary");
const String
    C_sList_IncludedServices_Heading("Included Services - Summary");
const String
    C_sList_IncludedServices_Label("ServicesSummary");
const String
    C_sList_IncludedServicesDetails("Services' Details");
const String
    C_sList_IncludedServicesDetails_Heading("Included Services - Details");
const String
    C_sList_IncludedServicesDetails_Label("ServicesDetails");
const String
    C_sList_ExportedInterfaces("Interfaces' Summary");
const String
    C_sList_ExportedInterfaces_Heading("Exported Interfaces - Summary");
const String
    C_sList_ExportedInterfaces_Label("InterfacesSummary");
const String
    C_sList_ExportedInterfacesDetails("Interfaces' Details");
const String
    C_sList_ExportedInterfacesDetails_Heading("Exported Interfaces - Details");
const String
    C_sList_ExportedInterfacesDetails_Label("InterfacesDetails");
const String
    C_sList_Properties("Properties' Summary");
const String
    C_sList_Properties_Label("PropertiesSummary");
const String
    C_sList_PropertiesDetails("Properties' Details");
const String
    C_sList_PropertiesDetails_Label("PropertiesDetails");


enum E_SubListIndices
{
    sli_IncludedServicesSummary = 0,
    sli_InterfacesSummary = 1,
    sli_PropertiesSummary = 2,
    sli_IncludedServicesDetails = 3,
    sli_InterfacesDetails = 4,
    sli_PropertiesDetails = 5
};

} //anonymous namespace


HF_IdlService::HF_IdlService( Environment &         io_rEnv,
                              Xml::Element &        o_rOut )
    :   HtmlFactory_Idl(io_rEnv, &o_rOut)
{
}

HF_IdlService::~HF_IdlService()
{

}

typedef ::ary::idl::ifc_service::attr
    ServiceAttr;
typedef ::ary::Dyn_StdConstIterator< ::ary::idl::CommentedRelation >
    dyn_comref_list;

void
HF_IdlService::produce_CommentedRelations( const client &   i_ce,
                                           comref_list &    it_list,
                                           const String &   i_summaryTitle,
                                           const String &   i_summaryLabel,
                                           const String &   i_detailsTitle,
                                           const String &   i_detailsLabel,
                                           const E_DocType  i_docType ) const
{
    csv_assert( it_list );

    bool bSummaryOnly = ( i_docType == doctype_summaryOnly );
    HF_SubTitleTable aTable(
                CurOut(),
                bSummaryOnly ? i_summaryLabel : i_detailsLabel,
                bSummaryOnly ? i_summaryTitle : i_detailsTitle,
                2 );

    for ( ; it_list; ++it_list )
    {
        Xml::Element &
            rRow = aTable.Add_Row();

        if ( bSummaryOnly )
        {
            produce_Link(rRow, (*it_list).Type());
            produce_LinkSummary(i_ce, rRow, *it_list);
        }
        else
        {
            HF_IdlCommentedRelationElement
                aRelation( Env(), aTable, *it_list );
            aRelation.Produce_byData( i_ce );
        }
    }   // end for
}

void
HF_IdlService::Produce_byData( const client & i_ce ) const
{
    Dyn<HF_NaviSubRow>
        pNaviSubRow( &make_Navibar(i_ce) );

    HF_TitleTable
        aTitle(CurOut());
    HF_LinkedNameChain
        aNameChain(aTitle.Add_Row());

    aNameChain.Produce_CompleteChain(Env().CurPosition(), nameChainLinker);
    produce_Title(aTitle, C_sCePrefix_Service, i_ce);

    write_Docu(aTitle.Add_Row(), i_ce);
    CurOut() << new Html::HorizontalLine();

    // produce ...
    // - included services: summary
    dyn_comref_list dpIncludedServices;
    ServiceAttr::Get_IncludedServices(dpIncludedServices, i_ce);
    if ( (*dpIncludedServices).operator bool() )
    {
        produce_CommentedRelations( i_ce, *dpIncludedServices,
                         C_sList_IncludedServices_Heading,
                         C_sList_IncludedServices_Label,
                         C_sList_IncludedServicesDetails_Heading,
                         C_sList_IncludedServicesDetails_Label,
                         doctype_summaryOnly );
        pNaviSubRow->SwitchOn(sli_IncludedServicesSummary);
    }

    // - exported interfaces: summary
    dyn_comref_list dpExportedInterfaces;
    ServiceAttr::Get_ExportedInterfaces(dpExportedInterfaces, i_ce);
    if ( (*dpExportedInterfaces).operator bool() )
    {
        produce_CommentedRelations( i_ce, *dpExportedInterfaces,
                         C_sList_ExportedInterfaces_Heading,
                         C_sList_ExportedInterfaces_Label,
                         C_sList_ExportedInterfacesDetails_Heading,
                         C_sList_ExportedInterfacesDetails_Label,
                         doctype_summaryOnly );
        pNaviSubRow->SwitchOn(sli_InterfacesSummary);
    }

    // - supported properties: summary
    dyn_ce_list dpProperties;
    ServiceAttr::Get_Properties(dpProperties, i_ce);
    if ( (*dpProperties).operator bool() )
    {
        produce_Members( *dpProperties,
                         C_sList_Properties,
                         C_sList_Properties_Label,
                         C_sList_PropertiesDetails,
                         C_sList_PropertiesDetails_Label,
                         viewtype_summary );
        pNaviSubRow->SwitchOn(sli_PropertiesSummary);
    }

    // - included services: details
    ServiceAttr::Get_IncludedServices(dpIncludedServices, i_ce);
    if ( (*dpIncludedServices).operator bool() )
    {
        produce_CommentedRelations( i_ce, *dpIncludedServices,
                         C_sList_IncludedServices_Heading,
                         C_sList_IncludedServices_Label,
                         C_sList_IncludedServicesDetails_Heading,
                         C_sList_IncludedServicesDetails_Label,
                         doctype_complete );
        pNaviSubRow->SwitchOn(sli_IncludedServicesDetails);
    }

    // - exported interfaces: details
    ServiceAttr::Get_ExportedInterfaces(dpExportedInterfaces, i_ce);
    if ( (*dpExportedInterfaces).operator bool() )
    {
        produce_CommentedRelations( i_ce, *dpExportedInterfaces,
                         C_sList_ExportedInterfaces_Heading,
                         C_sList_ExportedInterfaces_Label,
                         C_sList_ExportedInterfacesDetails_Heading,
                         C_sList_ExportedInterfacesDetails_Label,
                         doctype_complete );
        pNaviSubRow->SwitchOn(sli_InterfacesDetails);
    }

    // supported properties: details
    ServiceAttr::Get_Properties(dpProperties, i_ce);
    if ( (*dpProperties).operator bool() )
    {
        produce_Members( *dpProperties,
                         C_sList_Properties,
                         C_sList_Properties_Label,
                         C_sList_PropertiesDetails,
                         C_sList_PropertiesDetails_Label,
                         viewtype_details );
        pNaviSubRow->SwitchOn(sli_PropertiesDetails);
    }

    pNaviSubRow->Produce_Row();
    CurOut() << new Xml::XmlCode("<br>&nbsp;");
}

typedef ary::idl::ifc_property::attr    PropertyAttr;

void
HF_IdlService::produce_SummaryDeclaration( Xml::Element &      o_row,
                                           const client &      i_property ) const
{
    // KORR_FUTURE
    // Put this in to HF_IdlProperty!

    Xml::Element &
        rCell = o_row
                    >> *new Html::TableCell
                        << new Html::ClassAttr( C_sCellStyle_SummaryLeft );

    if (PropertyAttr::HasAnyStereotype(i_property))
    {
        rCell << "[ ";
        if (PropertyAttr::IsReadOnly(i_property))
            rCell << "readonly ";
        if (PropertyAttr::IsBound(i_property))
            rCell << "bound ";
        if (PropertyAttr::IsConstrained(i_property))
            rCell << "constrained ";
        if (PropertyAttr::IsMayBeAmbiguous(i_property))
            rCell << "maybeambiguous ";
        if (PropertyAttr::IsMayBeDefault(i_property))
            rCell << "maybedefault ";
        if (PropertyAttr::IsMayBeVoid(i_property))
            rCell << "maybevoid ";
        if (PropertyAttr::IsRemovable(i_property))
            rCell << "removable ";
        if (PropertyAttr::IsTransient(i_property))
            rCell << "transient ";
        rCell << "] ";
    }   // end if

    HF_IdlTypeText
        aType( Env(), rCell );
    aType.Produce_byData( PropertyAttr::Type(i_property) );

    StreamLock aLocalLink(100);
    aLocalLink() << "#" << i_property.LocalName();
    rCell
        << new Html::LineBreak
        >> *new Html::Link( aLocalLink().c_str() )
            << i_property.LocalName();
}

DYN HF_NaviSubRow &
HF_IdlService::make_Navibar( const client & i_ce ) const
{
    HF_IdlNavigationBar
        aNaviBar(Env(), CurOut());
    aNaviBar.Produce_CeMainRow(i_ce);

    DYN HF_NaviSubRow &
        ret = aNaviBar.Add_SubRow();
    ret.AddItem(C_sList_IncludedServices, C_sList_IncludedServices_Label, false);
    ret.AddItem(C_sList_ExportedInterfaces, C_sList_ExportedInterfaces_Label, false);
    ret.AddItem(C_sList_Properties, C_sList_Properties_Label, false);
    ret.AddItem(C_sList_IncludedServicesDetails, C_sList_IncludedServicesDetails_Label, false);
    ret.AddItem(C_sList_ExportedInterfacesDetails, C_sList_ExportedInterfacesDetails_Label, false);
    ret.AddItem(C_sList_PropertiesDetails, C_sList_PropertiesDetails_Label, false);

    CurOut() << new Html::HorizontalLine();
    return ret;
}

void
HF_IdlService::produce_Link( Xml::Element &     o_row,
                             type_id            i_type ) const
{
    Xml::Element &
        rCell = o_row
                >> *new Html::TableCell
                    << new Html::ClassAttr(C_sCellStyle_SummaryLeft);
    HF_IdlTypeText
        aText(Env(), rCell);
    aText.Produce_byData(i_type);
}

void
HF_IdlService::produce_LinkSummary( const client &  i_ce,
                                    Xml::Element &  o_row,
                                    const comref &  i_commentedRef ) const
{
    Xml::Element &
        rCell = o_row
                >> *new Html::TableCell
                    << new Html::ClassAttr(C_sCellStyle_SummaryRight);

    HF_IdlCommentedRelationElement::produce_LinkDoc( Env(), i_ce, rCell, i_commentedRef, doctype_summaryOnly );
}

void
HF_IdlService::produce_MemberDetails( HF_SubTitleTable &  o_table,
                                      const client &      i_ce ) const
{
    HF_IdlProperty
        aProperty( Env(), o_table);
    aProperty.Produce_byData( i_ce );
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
