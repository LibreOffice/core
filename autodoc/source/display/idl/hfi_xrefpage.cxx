/*************************************************************************
 *
 *  $RCSfile: hfi_xrefpage.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:30:13 $
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
    if ( rLinks2Refs.size() == 0 )
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
        aTypeWriter(Env(), rOutCell, true, pClient);
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

void
HF_IdlXrefs::recursive_make_ListInTree( Xml::Element &      o_rDisplay,
                                        uintt               i_level,
                                        const client &      i_ce,
                                        ce_list &           i_iterator,
                                        F_GET_SUBLIST       i_sublistcreator ) const
{
    const char * sLevelIndentation = "    ";

    HF_IdlTypeText
        aTypeWriter(Env(), o_rDisplay, true, &i_ce);
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
