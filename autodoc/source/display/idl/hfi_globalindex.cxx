/*************************************************************************
 *
 *  $RCSfile: hfi_globalindex.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:25:21 $
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
#include "hfi_globalindex.hxx"


// NOT FULLY DEFINED SERVICES
#include <cosv/template/tpltools.hxx>
#include <ary/idl/i_ce.hxx>
#include <ary/idl/i_language.hxx>
#include <ary/idl/i_module.hxx>
#include <toolkit/hf_title.hxx>
#include "hfi_navibar.hxx"
#include "hfi_typetext.hxx"
#include "hi_linkhelper.hxx"




namespace
{

/**
*/
enum E_Types
{
        t_service           = 0,
        t_interface         = 1,
        t_struct            = 2,
        t_exception         = 3,
        t_enum              = 4,
        t_typedef           = 5,
        t_constantsgroup    = 6,
        t_property          = 7,
        t_function          = 8,
        t_structelement     = 9,
        t_enumvalue         = 10,
        t_constant          = 11,
        t_module            = 12,
        t_singleton         = 13,
        t_attribute         = 14,
        t_siservice         = 15,
        t_sisingleton       = 16,
        t_MAX
};

String  G_sDummy;
uintt   G_nDummy;


/*  RC-Ids for IDL types (see reposy.cxx):

    Module              2000
    Interface           2001
    Function            2002
    Service             2003
    Property            2004

    Enum                2005
    EnumValue           2006
    Typedef             2007
    Struct              2008
    StructElement       2009

    Exception           2010
    ConstantGroup       2011
    Constant            2012
    Singleton           2013
    Attribute           2014
    SglIfcService       2015
    SglIfcSingleton     2016
*/
const int C_nNumberOfIdlTypes = 17;
const char *    C_sTypeNames[C_nNumberOfIdlTypes] =
                    { "module ",    "interface ",   "function ",    "service ",     "property ",
                      "enum ",      "value ",       "typedef ",     "struct ",      "field ",
                      "exception ", "constants group ", "constant ","singleton ",   "attribute ",
                      "service",    "singleton"
                    };
const char *    C_sOwnerNames[C_nNumberOfIdlTypes] =
                    { "module ",    "module ",      "interface ",   "module ",      "service ",
                      "module ",    "enum ",        "module ",      "module ",      "", // could be struct or exception
                      "module ",    "module ",      "constants group ", "module ",  "interface ",
                      "module",     "module"
                    };
const intt C_nNamesArrayOffset = intt(ary::idl::Module::class_id);
const int C_nIxField = 9;




const char C_cAlphabet[] =
"<a class=\"inverse\" href=\"index-1.html\"><B>A</B></a> <a class=\"inverse\" href=\"index-2.html\"><B>B</B></a> <a class=\"inverse\" href=\"index-3.html\"><B>C</B></a> <a class=\"inverse\" href=\"index-4.html\"><B>D</B></a> <a class=\"inverse\" href=\"index-5.html\"><B>E</B></a> "
"<a class=\"inverse\" href=\"index-6.html\"><B>F</B></a> <a class=\"inverse\" href=\"index-7.html\"><B>G</B></a> <a class=\"inverse\" href=\"index-8.html\"><B>H</B></a> <a class=\"inverse\" href=\"index-9.html\"><B>I</B></a> <a class=\"inverse\" href=\"index-10.html\"><B>J</B></a> "
"<a class=\"inverse\" href=\"index-11.html\"><B>K</B></a> <a class=\"inverse\" href=\"index-12.html\"><B>L</B></a> <a class=\"inverse\" href=\"index-13.html\"><B>M</B></a> <a class=\"inverse\" href=\"index-14.html\"><B>N</B></a> <a class=\"inverse\" href=\"index-15.html\"><B>O</B></a> "
"<a class=\"inverse\" href=\"index-16.html\"><B>P</B></a> <a class=\"inverse\" href=\"index-17.html\"><B>Q</B></a> <a class=\"inverse\" href=\"index-18.html\"><B>R</B></a> <a class=\"inverse\" href=\"index-19.html\"><B>S</B></a> <a class=\"inverse\" href=\"index-20.html\"><B>T</B></a> "
"<a class=\"inverse\" href=\"index-21.html\"><B>U</B></a> <a class=\"inverse\" href=\"index-22.html\"><B>V</B></a> <a class=\"inverse\" href=\"index-23.html\"><B>W</B></a> <a class=\"inverse\" href=\"index-24.html\"><B>X</B></a> <a class=\"inverse\" href=\"index-25.html\"><B>Y</B></a> "
"<a class=\"inverse\" href=\"index-26.html\"><B>Z</B></a>";



HF_IdlGlobalIndex::PageData     G_PageData;

}   // end anonymous namespace


inline void
HF_IdlGlobalIndex::write_EntryItself( Xml::Element &               o_destination,
                                      const ary::idl::CodeEntity & i_ce,
                                      const HF_IdlTypeText &       i_typeLinkWriter ) const
{
    i_typeLinkWriter.Produce_IndexLink(o_destination, i_ce);
    o_destination << " - ";
}


HF_IdlGlobalIndex::HF_IdlGlobalIndex( Environment &     io_rEnv,
                                      Xml::Element &    o_rOut )
    :   HtmlFactory_Idl(io_rEnv, &o_rOut)
{
}

HF_IdlGlobalIndex::~HF_IdlGlobalIndex()
{
}

void
HF_IdlGlobalIndex::Produce_Page(ary::idl::alphabetical_index::E_Letter i_letter) const
{
    make_Navibar();

    HF_TitleTable
        aTitle(CurOut());
    aTitle.Produce_Title( StreamLock(100)()
                          << "Global Index "
                          << ( i_letter != ary::idl::alphabetical_index::non_alpha
                                ?   char(int(i_letter)-'a'+'A')
                                :   '_' )
                          << c_str );

    // Letters Index
    aTitle.Add_Row()
        << new Xml::XmlCode(
            "<p align=\"center\"><a href=\"index-1.html\"><b>A</b></a> <a href=\"index-2.html\"><b>B</b></a> <a href=\"index-3.html\"><b>C</b></a> <a href=\"index-4.html\"><b>D</b></a> <a href=\"index-5.html\"><b>E</b></a> <a href=\"index-6.html\"><b>F</b></a> <a href=\"index-7.html\"><b>G</b></a> <a href=\"index-8.html\"><b>H</b></a> <a href=\"index-9.html\"><b>I</b></a> <a href=\"index-10.html\"><b>J</b></a>"
            " <a href=\"index-11.html\"><b>K</b></a> <a href=\"index-12.html\"><b>L</b></a> <a href=\"index-13.html\"><b>M</b></a> <a href=\"index-14.html\"><b>N</b></a> <a href=\"index-15.html\"><b>O</b></a> <a href=\"index-16.html\"><b>P</b></a> <a href=\"index-17.html\"><b>Q</b></a> <a href=\"index-18.html\"><b>R</b></a> <a href=\"index-19.html\"><b>S</b></a> <a href=\"index-20.html\"><b>T</b></a>"
            " <a href=\"index-21.html\"><b>U</b></a> <a href=\"index-22.html\"><b>V</b></a> <a href=\"index-23.html\"><b>W</b></a> <a href=\"index-24.html\"><b>X</b></a> <a href=\"index-25.html\"><b>Y</b></a> <a href=\"index-26.html\"><b>Z</b></a> <a href=\"index-27.html\"><b>_</b></a></p>" );

    Out().Enter(CurOut() >> *new Html::DefList);

    csv::erase_container(G_PageData);
    Env().Data().Get_IndexData(G_PageData, i_letter);

    // Helper object to produce links to the index Entries.
    HF_IdlTypeText aTypeLinkWriter(Env(),HF_IdlTypeText::use_for_javacompatible_index);

    PageData::const_iterator itEnd = G_PageData.end();
    for ( PageData::const_iterator iter = G_PageData.begin();
          iter != itEnd;
          ++iter )
    {
        produce_Line(iter, aTypeLinkWriter);
    }   // end for

    Out().Leave();
    CurOut() << new Html::HorizontalLine;
}

void
HF_IdlGlobalIndex::make_Navibar() const
{
    HF_IdlNavigationBar
        aNaviBar(Env(), CurOut());
    aNaviBar.Produce_IndexMainRow();

    CurOut() << new Html::HorizontalLine();
}

void
HF_IdlGlobalIndex::produce_Line( PageData::const_iterator i_entry,
                                 const HF_IdlTypeText &   i_typeLinkWriter) const
{
    const client &
        rCe = Env().Data().Find_Ce(*i_entry);
    if (NOT rCe.Owner().IsValid())
        return; // Omit global namespace.

    // The destination for the created output:
    Xml::Element & rDT = CurOut() >> *new Html::DefListTerm;

    /** The following code is intended to produce an output that
        will be recognized by the context help system of Forte.
        That is reached by making it similar to the indices, that
        Javadoc produces.
        If the link to the Entry contains a hashmark, the Forte-Help
        requires following a link to the owner.
        But if there is no hashmark, the following link must go to
        the same Entry again. Doesn't make really sense :-(, but that's
        like it is.
    */
    write_EntryItself(rDT,rCe,i_typeLinkWriter);
    if (rCe.SightLevel() == ary::idl::sl_Member)
        write_OwnerOfEntry(rDT,rCe,i_typeLinkWriter);
    else
        write_EntrySecondTime(rDT,rCe,i_typeLinkWriter);

    // This produces an empty "<dd></dd>", which is also needed to reach
    //   similarity to the Javadoc index:
    CurOut() << new Html::DefListDefinition;
}

void
HF_IdlGlobalIndex::write_OwnerOfEntry( Xml::Element &               o_destination,
                                       const ary::idl::CodeEntity & i_ce,
                                       const HF_IdlTypeText &       i_typeLinkWriter ) const
{
    const client &
        rOwner = Env().Data().Find_Ce(i_ce.Owner());

    int nIx = int(i_ce.ClassId() - C_nNamesArrayOffset);
    csv_assert(csv::in_range(0,nIx,C_nNumberOfIdlTypes));

    o_destination << C_sTypeNames[nIx]
                  << "in ";
    if (nIx != C_nIxField)
    {
        o_destination << C_sOwnerNames[nIx];
    }
    else
    {
        uintt nOwnerIx = rOwner.ClassId() - C_nNamesArrayOffset;
        csv_assert(0 <= nOwnerIx && nOwnerIx < C_nNumberOfIdlTypes);
        o_destination << C_sTypeNames[nOwnerIx];
    }
    i_typeLinkWriter.Produce_IndexOwnerLink(o_destination, rOwner);
}

void
HF_IdlGlobalIndex::write_EntrySecondTime( Xml::Element &                o_destination,
                                          const ary::idl::CodeEntity &  i_ce,
                                          const HF_IdlTypeText &        i_typeLinkWriter ) const
{
    int nIx = int(i_ce.ClassId() - C_nNamesArrayOffset);
    csv_assert(csv::in_range(0,nIx,C_nNumberOfIdlTypes));

    o_destination << C_sTypeNames[nIx]
                  << " ";
    i_typeLinkWriter.Produce_IndexSecondEntryLink(o_destination, i_ce);
}
