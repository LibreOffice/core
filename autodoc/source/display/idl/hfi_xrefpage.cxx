/*************************************************************************
 *
 *  $RCSfile: hfi_xrefpage.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: np $ $Date: 2002-11-14 18:01:57 $
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
#include "hfi_xrefpage.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/idl/i_ce.hxx>
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
    if ( rLinks2Refs.size() == 0 )
    {
        rContentDirectory
            << "References in Developer Manual"
            << new Html::LineBreak
            << C_sCRLF;
        return;
    }


    rContentDirectory
        >> *new Html::Link("#devmanrefs")
            << "References in Developer Manual"
            << new Html::LineBreak
            << C_sCRLF;

    HF_SubTitleTable
        aList(CurOut(), "devmanrefs", "References in Developer Manual", 1);
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
            rLink << *it;
        else
            rLink << *(it+1);
        rOutCell
            << new Html::LineBreak
            << C_sCRLF;
        ++it;
    }   // end for

    CurOut() << new Html::HorizontalLine();
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
        aTypeWriter(Env(), rOutCell, true, pClient);
    for ( ce_list & it = i_iterator; it; ++it )
    {
        aTypeWriter.Produce_byData(*it);
        rOutCell << new Html::LineBreak;
    }   // end for
    CurOut() << new Html::HorizontalLine();
}

void
HF_IdlXrefs::produce_Main( const String &        i_prefix,
                           const client &        i_ce ) const
{
    make_Navibar(i_ce);

    HF_TitleTable
        aTitle(CurOut());
    aTitle.Produce_Title( StreamLock(200)()
                          << C_sTitleStart
                          << i_prefix
                          << " "
                          << i_ce.LocalName()
                          << c_str );

    aTitle.Add_Row() << &rContentDirectory;
    rContentDirectory
        >> *new Html::Link( StreamLock(200)()
                                    << i_ce.LocalName()
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
