/*************************************************************************
 *
 *  $RCSfile: hfi_interface.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-18 14:11:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#include <precomp.h>
#include "hfi_interface.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/idl/i_ce.hxx>
#include <ary/idl/ik_function.hxx>
#include <ary/idl/ik_interface.hxx>
#include <toolkit/hf_docentry.hxx>
#include <toolkit/hf_linachain.hxx>
#include <toolkit/hf_navi_sub.hxx>
#include <toolkit/hf_title.hxx>
#include "hfi_method.hxx"
#include "hfi_navibar.hxx"
#include "hfi_property.hxx"
#include "hfi_typetext.hxx"
#include "hi_linkhelper.hxx"


extern const String
    C_sCePrefix_Interface("interface");

namespace
{

const String
    C_sBaseInterface("Base Hierarchy");
//const String
//    C_sList_ExportingServices("Known Services which Export this Interface");
//const String
//    C_sList_ExportingServices_Link("Exporting Services");
//const String
//    C_sList_ExportingServices_Label("ExportingServices");
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
    sli_Methods = 0,
    sli_MethodDetails = 1,
    sli_AttributesSummary = 2,
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

    aTitle.Produce_Title( StreamLock(200)() << C_sCePrefix_Interface
                                         << " "
                                         << i_ce.LocalName()
                                         << c_str );
    produce_Bases(aTitle.Add_Row(),i_ce,C_sBaseInterface);

    write_Docu(aTitle.Add_Row(), i_ce);
    CurOut() << new Html::HorizontalLine();

    dyn_ce_list dpFunctions;
    ary::idl::ifc_interface::attr::Get_Functions(dpFunctions, i_ce);

    if ( BOOL_OF(*dpFunctions) )
    {
        eCurProducedMembers = mem_Functions;

        produce_Members( *dpFunctions,
                         C_sList_Methods,
                         C_sList_Methods_Label,
                         C_sDetails_Methods,
                         C_sDetails_Methods_Label );
        pNaviSubRow->SwitchOn(sli_Methods);
        pNaviSubRow->SwitchOn(sli_MethodDetails);
    }

    dyn_ce_list
        dpAttributes;
    ary::idl::ifc_interface::attr::Get_Attributes(dpAttributes, i_ce);
    if ( BOOL_OF(*dpAttributes) )
    {
        eCurProducedMembers = mem_Attributes;

        produce_Members( *dpAttributes,
                         C_sList_Attributes,
                         C_sList_Attributes_Label,
                         C_sList_AttributesDetails,
                         C_sList_AttributesDetails_Label );
        pNaviSubRow->SwitchOn(sli_AttributesSummary);
        pNaviSubRow->SwitchOn(sli_AttributesDetails);
    }
    eCurProducedMembers = mem_none;

    pNaviSubRow->Produce_Row();
}

HtmlFactory_Idl::type_id
HF_IdlInterface::inq_BaseOf( const client & i_ce ) const
{
    return ary::idl::ifc_interface::attr::Base(i_ce);
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
    ret.AddItem(C_sDetails_Methods, C_sDetails_Methods_Label, false);
    ret.AddItem(C_sList_Attributes, C_sList_Attributes_Label, false);
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
                              i_ce );
}
