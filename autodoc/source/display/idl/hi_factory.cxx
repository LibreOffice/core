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
#include "hi_factory.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/idl/i_ce.hxx>
#include <toolkit/hf_title.hxx>
#include "hfi_doc.hxx"
#include "hfi_navibar.hxx"
#include "hfi_tag.hxx"
#include "hfi_typetext.hxx"
#include "hi_linkhelper.hxx"


extern const String
    C_sCellStyle_SummaryLeft("imsum_left");
extern const String
    C_sCellStyle_SummaryRight("imsum_right");
extern const String
    C_sCellStyle_MDetail("imdetail");
extern const String
    C_sMemberTitle("membertitle");


namespace
{

const char C_sSpace[92] = "                              "
                          "                              "
                          "                               ";
}


void
HtmlFactory_Idl::produce_SummaryDeclaration( Xml::Element &      o_row,
                                             const client &      i_ce ) const
{
    produce_InternalLink(o_row, i_ce);
}

void
HtmlFactory_Idl::produce_InternalLink( Xml::Element &  o_screen,
                                       const client &  i_ce ) const
{
    StreamLock aLocalLink(100);
    aLocalLink() << "#" << i_ce.LocalName();

    o_screen
        >> *new Html::TableCell
            << new Html::ClassAttr( C_sCellStyle_SummaryLeft )
            >> *new Html::Link( aLocalLink().c_str() )
                << i_ce.LocalName();
}

void
HtmlFactory_Idl::produce_ShortDoc( Xml::Element &   o_screen,
                                   const client &   i_ce ) const
{
    Xml::Element &
        rDetailsRowCell = o_screen
                            >> *new Html::TableCell
                                << new Html::ClassAttr( C_sCellStyle_SummaryRight );
    HF_IdlShortDocu
        aLinkDoc(Env(), rDetailsRowCell);
    aLinkDoc.Produce_byData( i_ce );

    rDetailsRowCell << new Xml::XmlCode("&nbsp;");
}

// KORR_FUTURE: Does not belong here (implementation inheritance)!
void
HtmlFactory_Idl::produce_Bases( Xml::Element &   o_screen,
                                const client &   i_ce,
                                const String &   i_sLabel ) const
{
    ary::idl::Type_id nBaseT = baseOf(i_ce);
    if ( nBaseT.IsValid() )
    {
        HF_DocEntryList
            aDocList( o_screen );
        aDocList.Produce_Term(i_sLabel);

        int nDepth = 0;
        Xml::Element &
            rBaseList = aDocList.Produce_Definition()
                                >> *new Xml::AnElement("pre")
                                    << new Xml::AnAttribute("style","font-family:monospace;");
        rBaseList
            >> *new Html::Strong
                << i_ce.LocalName();
        rBaseList
            << "\n";
        recursive_ShowBases( rBaseList,
                             nBaseT,
                             nDepth );
    }
}

void
HtmlFactory_Idl::produce_Members( ce_list &               it_list,
                                  const String &          i_summaryTitle,
                                  const String &          i_summaryLabel,
                                  const String &          i_detailsTitle,
                                  const String &          i_detailsLabel,
                                  const E_MemberViewType  i_viewType ) const
{
    csv_assert( it_list );

    Dyn< HF_SubTitleTable > pSummary;
    if  (   ( i_viewType == viewtype_summary )
        ||  ( i_viewType == viewtype_complete )
        )
    {
        pSummary = new HF_SubTitleTable(
                CurOut(),
                i_summaryLabel,
                i_summaryTitle,
                2 );
    }

    Dyn< HF_SubTitleTable > pDetails;
    if  (   ( i_viewType == viewtype_details )
        ||  ( i_viewType == viewtype_complete )
        )
    {
        pDetails = new HF_SubTitleTable(
                CurOut(),
                i_detailsLabel,
                i_detailsTitle,
                1 );
    }

    for ( ; it_list.operator bool(); ++it_list )
    {
        const ary::idl::CodeEntity &
            rCe = Env().Data().Find_Ce(*it_list);

        if ( pSummary )
        {
            Xml::Element &
                rSummaryRow = pSummary->Add_Row();
            produce_SummaryDeclaration(rSummaryRow, rCe);
//            produce_InternalLink(rSummaryRow, rCe);
            produce_ShortDoc(rSummaryRow, rCe);
        }

        if ( pDetails )
            produce_MemberDetails(*pDetails, rCe);
    }
}

void
HtmlFactory_Idl::produce_Title( HF_TitleTable &     o_title,
                                const String &      i_label,
                                const client &      i_ce ) const
{
    StreamLock
        slAnnotations(200);
    get_Annotations(slAnnotations(), i_ce);
    StreamLock
        slTitle(200);
    slTitle() << i_label << " " << i_ce.LocalName();
    o_title.Produce_Title( slAnnotations().c_str(),
                           slTitle().c_str() );
}

void
HtmlFactory_Idl::get_Annotations( StreamStr &         o_out,
                                  const client &      i_ce ) const
{
    const ary::doc::OldIdlDocu *
        doc = Get_IdlDocu(i_ce.Docu());
    if (doc != 0)
    {
        if (doc->IsDeprecated())
            o_out << "deprecated ";
        if (NOT doc->IsPublished())
            o_out << "unpublished ";
    }

    // KORR
    //  Need to display "unpublished", if there is no docu.
}

void
HtmlFactory_Idl::write_Docu( Xml::Element &     o_screen,
                             const client &     i_ce ) const
{
    const ary::doc::OldIdlDocu *
        doc = Get_IdlDocu(i_ce.Docu());
    if (doc != 0)
    {
        HF_DocEntryList
            aDocuList( o_screen );
        HF_IdlDocu
            aDocu( Env(), aDocuList );
        aDocu.Produce_fromCodeEntity(i_ce);
    }

    write_ManualLinks(o_screen, i_ce);
}

void
HtmlFactory_Idl::write_ManualLinks( Xml::Element &  o_screen,
                                    const client &  i_ce ) const
{
    const StringVector &
        rLinks2Descrs = i_ce.Secondaries().Links2DescriptionInManual();
    if ( rLinks2Descrs.empty() )
        return;

    o_screen
        >> *new Html::Label(C_sLocalManualLinks.c_str()+1)  // Leave out the leading '#'.
            << " ";
    HF_DocEntryList
        aDocuList( o_screen );
    aDocuList.Produce_Term("Developers Guide");
    csv_assert(rLinks2Descrs.size() % 2 == 0);
    for ( StringVector::const_iterator it = rLinks2Descrs.begin();
          it != rLinks2Descrs.end();
          ++it )
    {
        Xml::Element &
            rLink = aDocuList.Produce_Definition() >> *new Html::Link( Env().Link2Manual(*it));
        if ( (*(it+1)).empty() )
            // HACK KORR_FUTURE
            // Research what happens with manual links which contain normal characters
            // in non-utf-8 texts. And research, why utfF-8 does not work here.
            rLink << new Xml::XmlCode(*it);
        else
            rLink << new Xml::XmlCode( *(it+1) );
        ++it;
    }   // end for
}

void
HtmlFactory_Idl::produce_MemberDetails(  HF_SubTitleTable &  ,
                                         const client &      ) const
{
    // Dummy, which does not need to do anything.
}

void
HtmlFactory_Idl::recursive_ShowBases( Xml::Element &     o_screen,
                                      type_id            i_baseType,
                                      int &              io_nDepth ) const
{
    // Show this base
    ++io_nDepth;
    const ary::idl::CodeEntity *
        pCe = Env().Linker().Search_CeFromType(i_baseType);

    csv_assert(io_nDepth > 0);
    if (io_nDepth > 30)
        io_nDepth = 30;
    o_screen
        << (C_sSpace + 93 - 3*io_nDepth)
        << new csi::xml::XmlCode("&#x2517")
        << " ";

    if (pCe == 0)
    {
        HF_IdlTypeText
            aText( Env(), o_screen );
        aText.Produce_byData( i_baseType );
        o_screen
            << "\n";
        --io_nDepth;
        return;
    }

    HF_IdlTypeText
        aBaseLink( Env(), o_screen );
    aBaseLink.Produce_byData(pCe->CeId());
    o_screen
        << "\n";

    // Bases
    ary::idl::Type_id
        nBaseT = baseOf(*pCe);
    if (nBaseT.IsValid())
        recursive_ShowBases(o_screen,nBaseT,io_nDepth);

    --io_nDepth;
    return;
}

HtmlFactory_Idl::type_id
HtmlFactory_Idl::inq_BaseOf( const client & ) const
{
    // Unused dummy.
    return type_id(0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
