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
#include "hfi_property.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/idl/i_ce.hxx>
#include <ary/idl/ik_attribute.hxx>
#include <ary/idl/ik_constant.hxx>
#include <ary/idl/ik_enumvalue.hxx>
#include <ary/idl/ik_property.hxx>
#include <ary/idl/ik_structelem.hxx>
#include <toolkit/hf_docentry.hxx>
#include <toolkit/hf_title.hxx>
#include "hfi_typetext.hxx"
#include "hfi_doc.hxx"
#include "hfi_tag.hxx"
#include "hi_env.hxx"
#include "hi_ary.hxx"
#include "hi_linkhelper.hxx"

void
HF_IdlDataMember::Produce_byData( const client & ce ) const
{
    write_Title(ce);
    enter_ContentCell();
    write_Declaration(ce);
    write_Description(ce);
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
HF_IdlDataMember::write_Title( const client & i_ce ) const
{
    CurOut()
        >> *new Html::Label(i_ce.LocalName())
            << new Html::ClassAttr(C_sMemberTitle)
            << i_ce.LocalName();
}

void
HF_IdlDataMember::write_Description( const client & i_ce ) const
{
    CurOut() << new Html::HorizontalLine;
    write_Docu(CurOut(), i_ce);
}

void
HF_IdlDataMember::enter_ContentCell() const
{

    Xml::Element &
        rContentCell = CurOut()
                        >> *new Html::Table( sContentBorder,
                                             sContentWidth,
                                             sContentPadding,
                                             sContentSpacing )
                            << new Html::ClassAttr("table-in-data")
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
    {
        CurOut() << "[ ";
        if (PropertyAttr::IsReadOnly(i_ce))
            CurOut() << "readonly ";
        if (PropertyAttr::IsBound(i_ce))
            CurOut() << "bound ";
        if (PropertyAttr::IsConstrained(i_ce))
            CurOut() << "constrained ";
        if (PropertyAttr::IsMayBeAmbiguous(i_ce))
            CurOut() << "maybeambiguous ";
        if (PropertyAttr::IsMayBeDefault(i_ce))
            CurOut() << "maybedefault ";
        if (PropertyAttr::IsMayBeVoid(i_ce))
            CurOut() << "maybevoid ";
        if (PropertyAttr::IsRemovable(i_ce))
            CurOut() << "removable ";
        if (PropertyAttr::IsTransient(i_ce))
            CurOut() << "transient ";
        CurOut() << "] ";
    }   // end if

    HF_IdlTypeText
        aType( Env(), CurOut() );
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
    if (AttributeAttr::HasAnyStereotype(i_ce))
    {
        CurOut() << "[ ";
        if (AttributeAttr::IsReadOnly(i_ce))
            CurOut() << "readonly ";
        if (AttributeAttr::IsBound(i_ce))
            CurOut() << "bound ";
        CurOut() << "] ";
    }

    HF_IdlTypeText
        aType( Env(), CurOut() );
    aType.Produce_byData( AttributeAttr::Type(i_ce) );

    CurOut()
        << " "
        >> *new Html::Bold
            << i_ce.LocalName();

    dyn_type_list pGetExceptions;
    dyn_type_list pSetExceptions;
    AttributeAttr::Get_GetExceptions(pGetExceptions, i_ce);
    AttributeAttr::Get_SetExceptions(pSetExceptions, i_ce);

    bool bGetRaises = (*pGetExceptions).IsValid();
    bool bSetRaises = (*pSetExceptions).IsValid();
    bool bRaises = bGetRaises OR bSetRaises;
    if (bRaises)
    {
        HF_DocEntryList aSub(CurOut());

        if (bGetRaises)
        {
            Xml::Element &
                rGet = aSub.Produce_Definition();
            HF_IdlTypeText
                aExc(Env(), rGet);
            type_list & itExc = *pGetExceptions;

            rGet << "get raises (";
            aExc.Produce_byData(*itExc);
            for (++itExc; itExc.operator bool(); ++itExc)
            {
                rGet
                    << ",";
                aExc.Produce_byData(*itExc);
            }   // end for
            rGet << ")";
            if (NOT bSetRaises)
                rGet << ";";
        }   // end if (bGetRaises)

        if (bSetRaises)
        {
            Xml::Element &
                rSet = aSub.Produce_Definition();
            HF_IdlTypeText
                aExc(Env(), rSet);
            type_list & itExc = *pSetExceptions;

            rSet << "set raises (";
            aExc.Produce_byData(*itExc);
            for (++itExc; itExc.operator bool(); ++itExc)
            {
                rSet
                    << ",";
                aExc.Produce_byData(*itExc);
            }   // end for
            rSet << ");";
        }   // end if (bSetRaises)
    }
    else
    {
        CurOut() << ";";
    }
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
        aType( Env(), CurOut() );
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
        aType( Env(), CurOut() );
    aType.Produce_byData(StructElementAttr::Type(i_ce));
    CurOut()
        << " "
        >> *new Html::Bold
            << i_ce.LocalName();
    CurOut()
        << ";";
}

HF_IdlCommentedRelationElement::~HF_IdlCommentedRelationElement()
{
}

void
HF_IdlCommentedRelationElement::produce_Summary( Environment &   io_env,
                                                 Xml::Element &  io_context,
                                                 const comref &  i_commentedRef,
                                                 const client &  i_rScopeGivingCe )
{
    csv_assert( i_commentedRef.Info() );

    const ce_info &         rDocu = *i_commentedRef.Info();

    bool bShort = NOT rDocu.Short().IsEmpty();
    bool bDescr = NOT rDocu.Description().IsEmpty();

    if ( bShort )
    {
        HF_IdlDocuTextDisplay
                aDescription(io_env, 0, i_rScopeGivingCe);

        Xml::Element& rPara = io_context >> *new Html::Paragraph;
        aDescription.Out().Enter( rPara );
        rDocu.Short().DisplayAt( aDescription );

        // if there's more than just the summary - i.e. a description, or usage restrictions, or tags -,
        // then add a link to the details section
        if ( bDescr OR rDocu.IsDeprecated() OR rDocu.IsOptional() OR NOT rDocu.Tags().empty() )
        {
            StreamLock aLocalLink(100);
            aLocalLink() << "#" << get_LocalLinkName(io_env, i_commentedRef);

            aDescription.Out().Out() << "(";
            aDescription.Out().Out()
                >> *new Html::Link( aLocalLink().c_str() )
                    << "details";
            aDescription.Out().Out() << ")";
        }

        aDescription.Out().Leave();
    }
}

void
HF_IdlCommentedRelationElement::produce_LinkDoc( Environment &   io_env,
                                                 const client &  i_ce,
                                                 Xml::Element &  io_context,
                                                 const comref &  i_commentedRef,
                                                 const E_DocType i_docType )
{
    if ( i_commentedRef.Info() != 0 )
    {
        if ( i_docType == doctype_complete )
        {
            HF_DocEntryList aDocList(io_context);
            HF_IdlDocu aDocuDisplay(io_env, aDocList);

            aDocuDisplay.Produce_fromReference(*i_commentedRef.Info(), i_ce);
        }
        else
        {
            produce_Summary(io_env, io_context, i_commentedRef, i_ce);
        }
    }
    else
    {
        HF_DocEntryList aDocList(io_context);

        const client *
            pCe = io_env.Linker().Search_CeFromType(i_commentedRef.Type());
        const ce_info *
            pShort = pCe != 0
                        ?   Get_IdlDocu(pCe->Docu())
                        :   (const ce_info *)(0);
        if ( pShort != 0 )
        {
            aDocList.Produce_NormalTerm("(referenced entity's summary:)");
            Xml::Element &
                rDef = aDocList.Produce_Definition();
            HF_IdlDocuTextDisplay
                aShortDisplay( io_env, &rDef, *pCe);
            pShort->Short().DisplayAt(aShortDisplay);
        }   // end if (pShort != 0)
    }   // endif ( (*i_commentedRef).Info() != 0 ) else
}


String
HF_IdlCommentedRelationElement::get_LocalLinkName( Environment &  io_env,
                                                   const comref & i_commentedRef )
{
    StringVector        aModules;
    String              sLocalName;
    ce_id               nCe;
    int                 nSequenceCount = 0;

    const ary::idl::Type &
        rType = io_env.Data().Find_Type(i_commentedRef.Type());
    io_env.Data().Get_TypeText(aModules, sLocalName, nCe, nSequenceCount, rType);

    // speaking strictly, this is not correct: If we have two interfaces with the same local
    // name, but in different modules, then the link name will be ambiguous. However, this should
    // be too seldom a case to really make the link names that ugly by adding the module information.
    return sLocalName;
}

void
HF_IdlCommentedRelationElement::write_Title( const client & /*i_ce*/ ) const
{

    Xml::Element &
        rAnchor = CurOut()
                    >> *new Html::Label(get_LocalLinkName(Env(), m_relation))
                        << new Html::ClassAttr(C_sMemberTitle);

    HF_IdlTypeText
        aText(Env(), rAnchor);
    aText.Produce_byData(m_relation.Type());
}

void
HF_IdlCommentedRelationElement::write_Declaration( const client & /*i_ce*/ ) const
{
    // nothing to do here - an entity which is a commented relation does not have a declaration
}

void
HF_IdlCommentedRelationElement::write_Description( const client & i_ce ) const
{
    produce_LinkDoc( Env(), i_ce, CurOut(), m_relation, doctype_complete );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
