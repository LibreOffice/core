/*************************************************************************
 *
 *  $RCSfile: hfi_siservice.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:29:10 $
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
#include "hfi_siservice.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/idl/i_ce.hxx>
#include <ary/idl/ik_function.hxx>
#include <ary/idl/ik_siservice.hxx>
#include <ary_i/codeinf2.hxx>
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
    aTitle.Produce_Title( StreamLock(200)() << C_sCePrefix_Service
                                            << " "
                                            << i_ce.LocalName()
                                            << c_str );
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
