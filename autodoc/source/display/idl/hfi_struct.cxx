/*************************************************************************
 *
 *  $RCSfile: hfi_struct.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:29:41 $
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
#include "hfi_struct.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/idl/i_ce.hxx>
#include <ary/idl/ik_exception.hxx>
#include <ary/idl/ik_struct.hxx>
#include <toolkit/hf_docentry.hxx>
#include <toolkit/hf_linachain.hxx>
#include <toolkit/hf_navi_sub.hxx>
#include <toolkit/hf_title.hxx>
#include "hfi_navibar.hxx"
#include "hfi_property.hxx"
#include "hfi_typetext.hxx"
#include "hi_linkhelper.hxx"


extern const String
    C_sCePrefix_Struct("struct");
extern const String
    C_sCePrefix_Exception("exception");


namespace
{

const String
    C_sBaseStruct("Base Hierarchy");
const String
    C_sBaseException("Base Hierarchy");

const String
    C_sList_Elements("Elements' Summary");
const String
    C_sList_Elements_Label("Elements");

const String
    C_sList_ElementDetails("Elements' Details");
const String
    C_sList_ElementDetails_Label("ElementDetails");

enum E_SubListIndices
{
    sli_ElementsSummary = 0,
    sli_ElementsDetails = 1
};

}   // anonymous namespace



HF_IdlStruct::HF_IdlStruct( Environment &         io_rEnv,
                            Xml::Element &        o_rOut,
                            bool                  i_bIsException )
    :   HtmlFactory_Idl(io_rEnv, &o_rOut),
        bIsException(i_bIsException)
{
}

HF_IdlStruct::~HF_IdlStruct()
{
}

void
HF_IdlStruct::Produce_byData( const client & i_ce ) const
{
    Dyn<HF_NaviSubRow>
        pNaviSubRow( &make_Navibar(i_ce) );

    HF_TitleTable
        aTitle(CurOut());
    HF_LinkedNameChain
        aNameChain(aTitle.Add_Row());

    aNameChain.Produce_CompleteChain(Env().CurPosition(), nameChainLinker);
    aTitle.Produce_Title( StreamLock(200)()
                          << (bIsException
                                ?   C_sCePrefix_Exception
                                :   C_sCePrefix_Struct)
                          << " "
                          << i_ce.LocalName()
                          << c_str );
    produce_Bases( aTitle.Add_Row(),
                   i_ce,
                   bIsException
                    ?   C_sBaseException
                    :   C_sBaseStruct );

    write_Docu(aTitle.Add_Row(), i_ce);
    CurOut() << new Html::HorizontalLine();

    dyn_ce_list
        dpElements;
    if (bIsException)
        ary::idl::ifc_exception::attr::Get_Elements(dpElements, i_ce);
    else
        ary::idl::ifc_struct::attr::Get_Elements(dpElements, i_ce);

    if ( (*dpElements).operator bool() )
    {
        produce_Members( *dpElements,
                         C_sList_Elements,
                         C_sList_Elements_Label,
                         C_sList_ElementDetails,
                         C_sList_ElementDetails_Label );
        pNaviSubRow->SwitchOn(sli_ElementsSummary);
        pNaviSubRow->SwitchOn(sli_ElementsDetails);
    }
    pNaviSubRow->Produce_Row();
}

HtmlFactory_Idl::type_id
HF_IdlStruct::inq_BaseOf( const client & i_ce ) const
{
    return bIsException
                ?   ary::idl::ifc_exception::attr::Base(i_ce)
                :   ary::idl::ifc_struct::attr::Base(i_ce);
}

HF_NaviSubRow &
HF_IdlStruct::make_Navibar( const client & i_ce ) const
{
    HF_IdlNavigationBar
        aNaviBar(Env(), CurOut());
    aNaviBar.Produce_CeMainRow(i_ce);

    DYN HF_NaviSubRow &
        ret = aNaviBar.Add_SubRow();
    ret.AddItem(C_sList_Elements, C_sList_Elements_Label, false);
    ret.AddItem(C_sList_ElementDetails, C_sList_ElementDetails_Label, false);

    CurOut() << new Html::HorizontalLine();
    return ret;
}

void
HF_IdlStruct::produce_MemberDetails( HF_SubTitleTable &  o_table,
                                     const client &      i_ce) const
{
    HF_IdlStructElement
        aElement( Env(), o_table );
    aElement.Produce_byData(i_ce);
}

