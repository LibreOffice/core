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
#include "hfi_module.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/idl/i_ce.hxx>
#include <ary/idl/i_module.hxx>
#include <ary/idl/ik_module.hxx>
#include <ary/doc/d_oldidldocu.hxx>
#include <ary/getncast.hxx>
#include <toolkit/hf_docentry.hxx>
#include <toolkit/hf_linachain.hxx>
#include <toolkit/hf_navi_sub.hxx>
#include <toolkit/hf_title.hxx>
#include "hfi_doc.hxx"
#include "hfi_navibar.hxx"
#include "hfi_tag.hxx"
#include "hfi_typetext.hxx"
#include "hi_linkhelper.hxx"


extern const String
    C_sCePrefix_Module("module");

namespace
{

const String
    C_sList_NestedModules("Nested Modules");
const String
    C_sList_NestedModules_Label("NestedModules");
const String
    C_sList_Services("Services");
const String
    C_sList_Singletons("Singletons");
const String
    C_sList_Interfaces("Interfaces");
const String
    C_sList_Structs("Structs");
const String
    C_sList_Exceptions("Exceptions");
const String
    C_sList_Enums("Enums");
const String
    C_sList_Typedefs("Typedefs");
const String
    C_sList_ConstGroups("Constant Groups");
const String
    C_sList_ConstGroups_Label("ConstantGroups");


enum E_SubListIndices
{   // In case of changes, also adapt make_Navibar() !!
    sli_NestedModules = 0,
    sli_Services = 1,
    sli_Singletons = 2,
    sli_Interfaces = 3,
    sli_Structs = 4,
    sli_Exceptions = 5,
    sli_Enums = 6,
    sli_Typedefs = 7,
    sli_ConstGroups = 8
};

} //anonymous namespace


HF_IdlModule::HF_IdlModule( Environment &         io_rEnv,
                            Xml::Element &        o_rOut )
    :   HtmlFactory_Idl(io_rEnv, &o_rOut)
{
}

HF_IdlModule::~HF_IdlModule()
{
}

typedef ary::idl::ifc_module::attr  ModuleAttr;


void
HF_IdlModule::Produce_byData( const client & i_ce ) const
{
    Dyn<HF_NaviSubRow>
        pNaviSubRow( &make_Navibar(i_ce) );

    HF_TitleTable
        aTitle(CurOut());
    HF_LinkedNameChain
        aNameChain(aTitle.Add_Row());

    if ( Env().CurPosition().Depth() > 0 )
    {
        aNameChain.Produce_CompleteChain_forModule(Env().CurPosition(), nameChainLinker);

        StreamLock
            sl(200);
        aTitle.Produce_Title( sl()
                                << C_sCePrefix_Module
                                  << " "
                                << i_ce.LocalName()
                                << c_str );
    }
    else
    {
        aTitle.Produce_Title( "Global Module" );
    }

    write_Docu(aTitle.Add_Row(), i_ce);
    CurOut() << new Html::HorizontalLine();


    // Write children lists:
    ce_ptr_list aNestedModules;
    ce_ptr_list aServices;
    ce_ptr_list aInterfaces;
    ce_ptr_list aStructs;
    ce_ptr_list aExceptions;
    ce_ptr_list aEnums;
    ce_ptr_list aTypedefs;
    ce_ptr_list aConstantGroups;
    ce_ptr_list aSingletons;

    ModuleAttr::Get_AllChildrenSeparated(
                                aNestedModules,
                                aServices,
                                aInterfaces,
                                aStructs,
                                aExceptions,
                                aEnums,
                                aTypedefs,
                                aConstantGroups,
                                aSingletons,
                                Env().Data().Ces(),
                                i_ce );

    // Has this to be in the order of enum E_SubListIndices ???
    if (produce_ChildList(C_sList_NestedModules, C_sList_NestedModules_Label, aNestedModules ))
        pNaviSubRow->SwitchOn(sli_NestedModules);
    if (produce_ChildList(C_sList_Services, C_sList_Services, aServices))
        pNaviSubRow->SwitchOn(sli_Services);
    if (produce_ChildList(C_sList_Singletons, C_sList_Singletons, aSingletons))
        pNaviSubRow->SwitchOn(sli_Singletons);
    if (produce_ChildList(C_sList_Interfaces, C_sList_Interfaces, aInterfaces))
        pNaviSubRow->SwitchOn(sli_Interfaces);
    if (produce_ChildList(C_sList_Structs, C_sList_Structs, aStructs))
        pNaviSubRow->SwitchOn(sli_Structs);
    if (produce_ChildList(C_sList_Exceptions, C_sList_Exceptions, aExceptions))
        pNaviSubRow->SwitchOn(sli_Exceptions);
    if (produce_ChildList(C_sList_Enums, C_sList_Enums, aEnums))
        pNaviSubRow->SwitchOn(sli_Enums);
    if (produce_ChildList(C_sList_Typedefs, C_sList_Typedefs, aTypedefs))
        pNaviSubRow->SwitchOn(sli_Typedefs);
    if (produce_ChildList(C_sList_ConstGroups, C_sList_ConstGroups_Label, aConstantGroups))
        pNaviSubRow->SwitchOn(sli_ConstGroups);
    pNaviSubRow->Produce_Row();
}

DYN HF_NaviSubRow &
HF_IdlModule::make_Navibar( const client & i_ce ) const
{
    HF_IdlNavigationBar
        aNaviBar(Env(), CurOut());
    aNaviBar.Produce_ModuleMainRow(i_ce);

    DYN HF_NaviSubRow &
        ret = aNaviBar.Add_SubRow();

    // Has to be in the order of E_SubListIndices:
    ret.AddItem(C_sList_NestedModules, C_sList_NestedModules_Label, false);
    ret.AddItem(C_sList_Services, C_sList_Services, false);
    ret.AddItem(C_sList_Singletons, C_sList_Singletons, false);
    ret.AddItem(C_sList_Interfaces, C_sList_Interfaces, false);
    ret.AddItem(C_sList_Structs, C_sList_Structs, false);
    ret.AddItem(C_sList_Exceptions, C_sList_Exceptions, false);
    ret.AddItem(C_sList_Enums, C_sList_Enums, false);
    ret.AddItem(C_sList_Typedefs, C_sList_Typedefs, false);
    ret.AddItem(C_sList_ConstGroups, C_sList_ConstGroups_Label, false);

    CurOut() << new Html::HorizontalLine();
    return ret;
}

bool
HF_IdlModule::produce_ChildList( const String &      i_sName,
                                 const String &      i_sLabel,
                                 const ce_ptr_list & i_list ) const
{
    if ( i_list.empty() )
        return false;

    HF_SubTitleTable
        aTable( CurOut(),
                i_sLabel,
                i_sName,
                2 );

    ce_ptr_list::const_iterator
        itEnd = i_list.end();
    for ( ce_ptr_list::const_iterator it = i_list.begin();
          it != itEnd;
          ++it )
    {
        Xml::Element &
            rRow = aTable.Add_Row();
        produce_Link(rRow, *it);
        produce_LinkDoc(rRow, *it);
    }   // end for

    return true;
}

void
HF_IdlModule::produce_Link( Xml::Element &      o_row,
                            const client *      i_ce ) const
{
    csv_assert(i_ce != 0);
    Xml::Element &
        rCell = o_row
                >> *new Html::TableCell
                    << new Html::ClassAttr(C_sCellStyle_SummaryLeft);

    if ( NOT ary::is_type<ary::idl::Module>(*i_ce) )
    {
        HF_IdlTypeText
            aText(Env(), rCell);
        aText.Produce_byData(i_ce->CeId());
    }
    else
    {
        StreamLock slBuf(100);
         rCell
            >> *new Html::Link( slBuf() << i_ce->LocalName()
                                        << "/module-ix.html"
                                        << c_str )
                << i_ce->LocalName();
    }
}

void
HF_IdlModule::produce_LinkDoc( Xml::Element &      o_row,
                               const client *      i_ce ) const
{
    csv_assert(i_ce != 0);

    // We need the cell in any case, because, the rendering may be hurt else.
    Xml::Element &
        rCell = o_row
                    >> *new Html::TableCell
                        << new Html::ClassAttr(C_sCellStyle_SummaryRight);

    const client &
        rCe = *i_ce;
    const ce_info *
        pShort = Get_IdlDocu(rCe.Docu());
    if ( pShort == 0 )
        return;


    if (pShort->IsDeprecated())
    {
        rCell << "[ DEPRECATED ]" << new Html::LineBreak;
    }
    if (pShort->IsOptional())
    {
        rCell << "[ OPTIONAL ]" << new Html::LineBreak;
    }

    HF_IdlDocuTextDisplay
        aShortDisplay(Env(), &rCell, *i_ce);
    pShort->Short().DisplayAt(aShortDisplay);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
