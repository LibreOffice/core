/*************************************************************************
 *
 *  $RCSfile: hfi_property.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-11-01 17:14:39 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following li
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
#include "hfi_property.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/idl/i_ce.hxx>
#include <ary/idl/ik_attribute.hxx>
#include <ary/idl/ik_constant.hxx>
#include <ary/idl/ik_enumvalue.hxx>
#include <ary/idl/ik_property.hxx>
#include <ary/idl/ik_structelem.hxx>
#include <toolkit/hf_title.hxx>
#include "hfi_typetext.hxx"

void
HF_IdlDataMember::Produce_byData( const client & ce ) const
{
    // Title:
    CurOut()
        >> *new Html::Label(ce.LocalName())
            << new Html::ClassAttr(C_sMemberTitle)
            << ce.LocalName();

    enter_ContentCell();
    write_Declaration(ce);
    CurOut() << new Html::HorizontalLine;
    write_Docu(CurOut(), ce);
    leave_ContentCell();
}

HF_IdlDataMember::HF_IdlDataMember( Environment &       io_rEnv,
                                    HF_SubTitleTable &  o_table )
    :   HtmlFactory_Idl( io_rEnv,
                         &(o_table.Add_Row()
                            >> *new Html::TableCell
                                << new Html::ClassAttr(C_sCellStyle_MDetail))
                       )
{
}

const String sContentBorder("0");
const String sContentWidth("96%");
const String sContentPadding("5");
const String sContentSpacing("0");

const String sBgWhite("#ffffff");
const String sCenter("center");

void
HF_IdlDataMember::enter_ContentCell() const
{

    Xml::Element &
        rContentCell = CurOut()
                        >> *new Html::Table( sContentBorder,
                                             sContentWidth,
                                             sContentPadding,
                                             sContentSpacing )
                            << new Html::BgColorAttr(sBgWhite)
                            << new Html::AlignAttr(sCenter)
                            >> *new Html::TableRow
                                >> *new Html::TableCell;
    Out().Enter(rContentCell);
}


void
HF_IdlDataMember::leave_ContentCell() const
{
    Out().Leave();
}


HF_IdlProperty::~HF_IdlProperty()
{
}

typedef ary::idl::ifc_property::attr    PropertyAttr;

void
HF_IdlProperty::write_Declaration( const client & i_ce ) const
{
    if (PropertyAttr::HasAnyStereotype(i_ce))
        CurOut() << "[ ";
    if (PropertyAttr::IsReadOnly(i_ce))
        CurOut() << "readonly ";
    if (PropertyAttr::IsBound(i_ce))
        CurOut() << "bound ";
    if (PropertyAttr::IsConstrained(i_ce))
        CurOut() << "constrained ";
    if (PropertyAttr::IsMayBeAmbigious(i_ce))
        CurOut() << "maybeambigious ";
    if (PropertyAttr::IsMayBeDefault(i_ce))
        CurOut() << "maybedefault ";
    if (PropertyAttr::IsMayBeVoid(i_ce))
        CurOut() << "maybevoid ";
    if (PropertyAttr::IsRemovable(i_ce))
        CurOut() << "removable ";
    if (PropertyAttr::IsTransient(i_ce))
        CurOut() << "transient ";
    if (PropertyAttr::HasAnyStereotype(i_ce))
        CurOut() << " ] ";

    HF_IdlTypeText
        aType( Env(), CurOut(), true );
    aType.Produce_byData( PropertyAttr::Type(i_ce) );

    CurOut() << " " >> *new Html::Bold << i_ce.LocalName();
    CurOut() << ";";
}




HF_IdlAttribute::~HF_IdlAttribute()
{
}

typedef ary::idl::ifc_attribute::attr    AttributeAttr;

void
HF_IdlAttribute::write_Declaration( const client & i_ce ) const
{
    if (AttributeAttr::IsReadOnly(i_ce))
        CurOut() << "[ readonly ] ";

    HF_IdlTypeText
        aType( Env(), CurOut(), true );
    aType.Produce_byData( AttributeAttr::Type(i_ce) );

    CurOut() << " " >> *new Html::Bold << i_ce.LocalName();
    CurOut() << ";";
}




HF_IdlEnumValue::~HF_IdlEnumValue()
{
}

typedef ary::idl::ifc_enumvalue::attr    EnumValueAttr;

void
HF_IdlEnumValue::write_Declaration( const client & i_ce ) const
{
    CurOut()
        >> *new Html::Bold
            << i_ce.LocalName();

    const String &
        rValue = EnumValueAttr::Value(i_ce);
    if ( NOT rValue.empty() )
    {    CurOut() << " " // << " = "    // In the moment this is somehow in the value
                 << rValue;
        // CurOut() << ",";             // In the moment this is somehow in the value
    }
    else
        CurOut() << ",";
}


HF_IdlConstant::~HF_IdlConstant()
{
}

typedef ary::idl::ifc_constant::attr    ConstantAttr;

void
HF_IdlConstant::write_Declaration( const client & i_ce ) const
{
    CurOut() << "const ";
    HF_IdlTypeText
        aType( Env(), CurOut(), true );
    aType.Produce_byData(ConstantAttr::Type(i_ce));
    CurOut()
        << " "
        >> *new Html::Bold
            << i_ce.LocalName();
    const String &
        rValue = ConstantAttr::Value(i_ce);
    CurOut() << " "     // << " = "    // In the moment this is somehow in the value
             << rValue;
    // << ";";  // In the moment this is somehow in the value
}


HF_IdlStructElement::~HF_IdlStructElement()
{
}

typedef ary::idl::ifc_structelement::attr    StructElementAttr;

void
HF_IdlStructElement::write_Declaration( const client & i_ce ) const
{
    HF_IdlTypeText
        aType( Env(), CurOut(), true );
    aType.Produce_byData(StructElementAttr::Type(i_ce));
    CurOut()
        << " "
        >> *new Html::Bold
            << i_ce.LocalName();
    CurOut()
        << ";";
}

