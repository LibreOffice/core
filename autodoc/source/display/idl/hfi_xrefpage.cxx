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
#include "hfi_xrefpage.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/idl/i_ce.hxx>
#include <ary/idl/i_gate.hxx>
#include <ary/idl/ip_ce.hxx>
#include <toolkit/hf_title.hxx>
#include "hfi_navibar.hxx"
#include "hfi_typetext.hxx"
#include "hi_env.hxx"


namespace
{

const String
    C_sTitleStart("uses of ");
const String
    C_sCRLF("\n");
const String
    C_sDevMan("References in Developers Guide");

}   // anonymous namespace



HF_IdlXrefs::HF_IdlXrefs( Environment &         io_rEnv,
                          Xml::Element &        o_rOut,
                          const String &        i_prefix,
                          const client &        i_ce )
    :   HtmlFactory_Idl(io_rEnv, &o_rOut),
        rContentDirectory(*new Html::Paragraph),
        pClient(&i_ce)
{
    produce_Main(i_prefix, i_ce);
}

HF_IdlXrefs::~HF_IdlXrefs()
{
}

void
HF_IdlXrefs::Write_ManualLinks( const client &  i_ce ) const
{
    const StringVector &
        rLinks2Refs = i_ce.Secondaries().Links2RefsInManual();
    if ( rLinks2Refs.empty() )
    {
        rContentDirectory
            << C_sDevMan
            << new Html::LineBreak
            << C_sCRLF;
        return;
    }


    rContentDirectory
        >> *new Html::Link("#devmanrefs")
            << C_sDevMan
            << new Html::LineBreak
            << C_sCRLF;

    HF_SubTitleTable
        aList(CurOut(), "devmanrefs", C_sDevMan, 1);
    Xml::Element &
        rOutCell = aList.Add_Row() >>* new Html::TableCell;

    csv_assert(rLinks2Refs.size() % 2 == 0);
    for ( StringVector::const_iterator it = rLinks2Refs.begin();
          it != rLinks2Refs.end();
          ++it )
    {
        Xml::Element &
            rLink = rOutCell >> *new Html::Link( Env().Link2Manual(*it));
        if ( (*(it+1)).empty() )

            // HACK KORR_FUTURE
            // Research what happens with manual links which contain normal characters
            // in non-utf-8 texts. And research, why utfF-8 does not work here.
            rLink << new Xml::XmlCode(*it);
        else
            // HACK KORR_FUTURE, see above.
            rLink << new Xml::XmlCode( *(it+1) );
        rOutCell
            << new Html::LineBreak
            << C_sCRLF;
        ++it;
    }   // end for
}

void
HF_IdlXrefs::Produce_List( const char *        i_title,
                           const char *        i_label,
                           ce_list &           i_iterator ) const
{
    if (NOT i_iterator)
    {
        rContentDirectory
            << i_title
            << new Html::LineBreak
            << C_sCRLF;
        return;
    }

    csv_assert(*i_label == '#');

    rContentDirectory
        >> *new Html::Link(i_label)
            << i_title
            << new Html::LineBreak
            << C_sCRLF;

    HF_SubTitleTable
        aList(CurOut(), i_label+1, i_title, 1);
    Xml::Element &
        rOutCell = aList.Add_Row() >>* new Html::TableCell;
    HF_IdlTypeText
        aTypeWriter(Env(), rOutCell, pClient);
    for ( ce_list & it = i_iterator; it; ++it )
    {
        aTypeWriter.Produce_byData(*it);
        rOutCell << new Html::LineBreak;
    }   // end for
}

void
HF_IdlXrefs::Produce_Tree(  const char *        i_title,
                            const char *        i_label,
                            const client &      i_ce,
                            F_GET_SUBLIST       i_sublistcreator ) const
{
    dyn_ce_list pResult;
    (*i_sublistcreator)(pResult, i_ce);

    if (NOT (*pResult).operator bool())
    {
        rContentDirectory
            << i_title
            << new Html::LineBreak
            << C_sCRLF;
        return;
    }

    csv_assert(*i_label == '#');

    rContentDirectory
        >> *new Html::Link(i_label)
            << i_title
            << new Html::LineBreak
            << C_sCRLF;

    HF_SubTitleTable
        aList(CurOut(), i_label+1, i_title, 1);
    Xml::Element &
        rOut = aList.Add_Row()
                    >>* new Html::TableCell
                        >> *new csi::xml::AnElement("pre")
                            << new csi::html::StyleAttr("font-family:monospace;");

    recursive_make_ListInTree( rOut,
                               0,
                               i_ce,
                               *pResult,
                               i_sublistcreator );
}

void
HF_IdlXrefs::produce_Main( const String &        i_prefix,
                           const client &        i_ce ) const
{
    make_Navibar(i_ce);

    HF_TitleTable
        aTitle(CurOut());
    StreamLock sl(200);
    aTitle.Produce_Title( sl()
                          << C_sTitleStart
                          << i_prefix
                          << " "
                          << i_ce.LocalName()
                          << c_str );

    aTitle.Add_Row() << &rContentDirectory;
    sl().reset();
    rContentDirectory
        >> *new Html::Link( sl()    << i_ce.LocalName()
                                    << ".html"
                                    << c_str )
            >> *new Html::Bold
                << "back to "
                << i_prefix
                << " "
                << i_ce.LocalName();
    rContentDirectory
        << new Html::LineBreak
        << new Html::LineBreak
        << C_sCRLF;

    CurOut() << new Html::HorizontalLine();
}

void
HF_IdlXrefs::make_Navibar( const client & i_ce ) const
{
    HF_IdlNavigationBar
        aNaviBar(Env(), CurOut());
    aNaviBar.Produce_CeXrefsMainRow(i_ce);
    CurOut() << new Html::HorizontalLine();
}

void
HF_IdlXrefs::recursive_make_ListInTree( Xml::Element &      o_rDisplay,
                                        uintt               i_level,
                                        const client &      i_ce,
                                        ce_list &           i_iterator,
                                        F_GET_SUBLIST       i_sublistcreator ) const
{
    const char * sLevelIndentation = "    ";

    HF_IdlTypeText
        aTypeWriter(Env(), o_rDisplay, &i_ce);
    for ( ; i_iterator.operator bool(); ++i_iterator )
    {
        for (uintt i = 0; i < i_level; ++i)
        {
            o_rDisplay << sLevelIndentation;
        }   // end for

        aTypeWriter.Produce_byData(*i_iterator);
        o_rDisplay << C_sCRLF;

        dyn_ce_list     pResult;
        const client &  rCe = Env().Gate().Ces().Find_Ce(*i_iterator);
        (*i_sublistcreator)(pResult, rCe);
        if ( (*pResult).operator bool() )
        {
            recursive_make_ListInTree( o_rDisplay,
                                       i_level + 1,
                                       rCe,
                                       *pResult,
                                       i_sublistcreator );
        }
    }   // end for
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
