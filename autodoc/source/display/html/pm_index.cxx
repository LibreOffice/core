/*************************************************************************
 *
 *  $RCSfile: pm_index.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-15 18:45:24 $
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
#include "pm_index.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/cpp/c_disply.hxx>
#include <ary/cpp/c_class.hxx>
#include <ary/cpp/c_define.hxx>
#include <ary/cpp/c_enum.hxx>
#include <ary/cpp/c_tydef.hxx>
#include <ary/cpp/c_funct.hxx>
#include <ary/cpp/c_macro.hxx>
#include <ary/cpp/c_namesp.hxx>
#include <ary/cpp/c_vari.hxx>
#include <ary/cpp/c_enuval.hxx>
#include <ary/info/codeinfo.hxx>
#include "aryattrs.hxx"
#include "hd_chlst.hxx"
#include "hd_docu.hxx"
#include "html_kit.hxx"
#include "navibar.hxx"
#include "opageenv.hxx"
#include "pagemake.hxx"
#include "strconst.hxx"


using namespace csi;


namespace
{

inline const char *
F_CK_Text( ary::cpp::E_ClassKey i_eCK )
{
    switch (i_eCK)
    {
         case ary::cpp::CK_class:    return "class";
         case ary::cpp::CK_struct:   return "struct";
         case ary::cpp::CK_union:    return "union";
    }   // end switch
    return "";
}

template <class CE>
inline const char *
F_OwnerType( const CE & i_rData, const ary::cpp::DisplayGate & i_rGate )
{
     if ( i_rData.Protection() == ary::cpp::PROTECT_global )
        return "namespace ";

    const ary::cpp::Class *
        pClass = dynamic_cast< const ary::cpp::Class* >(
                    i_rGate.Find_Ce(i_rData.Owner()) );
    if (pClass != 0)
        return F_CK_Text(pClass->ClassKey());
    return "";
}

}   // anonymous namespace

PageMaker_Index::PageMaker_Index( PageDisplay & io_rPage,
                                  char          i_c )
    :   SpecializedPageMaker(io_rPage),
        pNavi(0),
        c(i_c),
        pCurIndex(0)
{
}

PageMaker_Index::~PageMaker_Index()
{
}

void
PageMaker_Index::MakePage()
{
    pNavi = new NavigationBar( Env(), NavigationBar::LOC_Index );

    Write_NavBar();
    Write_TopArea();
    Write_CompleteAlphabeticalList();
}

void
PageMaker_Index::Display_Namespace( const ary::cpp::Namespace & i_rData )
{
    Write_CeIndexEntry( i_rData, "namespace", "namespace" );
}

void
PageMaker_Index::Display_Class( const ary::cpp::Class & i_rData )
{
    Write_CeIndexEntry( i_rData,
                        F_CK_Text(i_rData.ClassKey()),
                        F_OwnerType(i_rData, Env().Gate()) );
}

void
PageMaker_Index::Display_Enum( const ary::cpp::Enum & i_rData )
{
    Write_CeIndexEntry( i_rData, "enum", F_OwnerType(i_rData, Env().Gate()) );
}

void
PageMaker_Index::Display_Typedef( const ary::cpp::Typedef & i_rData )
{
    Write_CeIndexEntry( i_rData, "typedef", F_OwnerType(i_rData, Env().Gate()) );
}

void
PageMaker_Index::Display_Function( const ary::cpp::Function & i_rData )
{
    Write_CeIndexEntry( i_rData, "function", F_OwnerType(i_rData, Env().Gate()) );
}

void
PageMaker_Index::Display_Variable( const ary::cpp::Variable & i_rData )
{
    Write_CeIndexEntry( i_rData, "variable", F_OwnerType(i_rData, Env().Gate()) );
}

void
PageMaker_Index::Display_EnumValue( const ary::cpp::EnumValue & i_rData )
{
    Write_CeIndexEntry( i_rData, "enum value", "" );
}

void
PageMaker_Index::Display_Define( const ary::cpp::Define & i_rData )
{
    udmstri sFileName;

    pCurIndex->AddEntry();
    pCurIndex->Term()
        >> *new html::Link( Link2CppDefinition(Env(), i_rData) )
            >> *new html::Bold
                <<  i_rData.DefinedName()
                << " - define";

    // KORR FUTURE
//    pCurIndex->Term()
//                <<  " - "
//                <<  " define in file "
//                << sFileName;

    pCurIndex->Def() << " ";
}

void
PageMaker_Index::Display_Macro( const ary::cpp::Macro & i_rData )
{
    udmstri sFileName;

    pCurIndex->AddEntry();
    pCurIndex->Term()
        >> *new html::Link( Link2CppDefinition(Env(), i_rData) )
            >> *new html::Bold
                <<  i_rData.DefinedName()
                << " - macro";

    // KORR FUTURE
//    pCurIndex->Term()
//                <<  " - "
//                <<  " macro in file "
//                << sFileName;

    pCurIndex->Def() << " ";
}

void
PageMaker_Index::Write_NavBar()
{
    pNavi->Write( CurOut() );
    CurOut() << new html::HorizontalLine;
}


const udmstri  C_sAlphabet(
"<a href=\"index-1.html\"><B>A</B></a> <a href=\"index-2.html\"><B>B</B></a> <a href=\"index-3.html\"><B>C</B></a> <a href=\"index-4.html\"><B>D</B></a> <a href=\"index-5.html\"><B>E</B></a> "
"<a href=\"index-6.html\"><B>F</B></a> <a href=\"index-7.html\"><B>G</B></a> <a href=\"index-8.html\"><B>H</B></a> <a href=\"index-9.html\"><B>I</B></a> <a href=\"index-10.html\"><B>J</B></a> "
"<a href=\"index-11.html\"><B>K</B></a> <a href=\"index-12.html\"><B>L</B></a> <a href=\"index-13.html\"><B>M</B></a> <a href=\"index-14.html\"><B>N</B></a> <a href=\"index-15.html\"><B>O</B></a> "
"<a href=\"index-16.html\"><B>P</B></a> <a href=\"index-17.html\"><B>Q</B></a> <a href=\"index-18.html\"><B>R</B></a> <a href=\"index-19.html\"><B>S</B></a> <a href=\"index-20.html\"><B>T</B></a> "
"<a href=\"index-21.html\"><B>U</B></a> <a href=\"index-22.html\"><B>V</B></a> <a href=\"index-23.html\"><B>W</B></a> <a href=\"index-24.html\"><B>X</B></a> <a href=\"index-25.html\"><B>Y</B></a> "
"<a href=\"index-26.html\"><B>Z</B></a> <a href=\"index-27.html\"><B>_</B></a>" );

void
PageMaker_Index::Write_TopArea()
{
    udmstri sLetter(&c, 1);

    adcdisp::PageTitle_Std fTitle;
    fTitle( CurOut(), "Global Index", sLetter );

    CurOut() >>* new html::Paragraph
                 << new html::AlignAttr("center")
                 << new xml::XmlCode(C_sAlphabet);

    CurOut() << new html::HorizontalLine;
}

void
PageMaker_Index::Write_CompleteAlphabeticalList()
{
    std::vector<ary::Rid>
        aThisPagesItems;
    const ary::cpp::DisplayGate &
        rGate = Env().Gate();

    static char sBegin[] = "X";
    static char sEnd[] = "Y";

    switch ( c )
    {
         case 'Z':   sBegin[0] = 'Z';
                    sEnd[0] = '_';
                    break;
        case '_':   sBegin[0] = '_';
                    sEnd[0] = '0';
                    break;
        default:    sBegin[0] = c;
                    sEnd[0] = char(c + 1);
                    break;
    }

    uintt nCount = rGate.Get_AlphabeticalList( aThisPagesItems, sBegin, sEnd );
    if (nCount > 0 )
    {
        adcdisp::IndexList
            aIndex(CurOut());
        pCurIndex = &aIndex;

        std::vector<ary::Rid>::const_iterator itEnd = aThisPagesItems.end();
        for ( std::vector<ary::Rid>::const_iterator it = aThisPagesItems.begin();
              it != itEnd;
              ++it )
        {
            const ary::RepositoryEntity *
                pRe = rGate.Find_Re( *it );
            if ( pRe != 0 )
                pRe->StoreAt(*this);
        }   // end for

        pCurIndex = 0;
    }   // endif (nCount > 0)
}

void
PageMaker_Index::Write_CeIndexEntry( const ary::CodeEntity & i_rCe,
                                     const char *            i_sType,
                                     const char *            i_sOwnerType )
{
    if ( Ce_IsInternal(i_rCe) )
        return;

    static csv::StreamStr aQualification(500);

    const ary::CodeEntity & rOwner = Env().Gate().Ref_Ce(i_rCe.Owner());

    pCurIndex->AddEntry();
    pCurIndex->Term()
        >> *new html::Link( Link2Ce(Env(), i_rCe) )
            >> *new html::Bold
                <<  i_rCe.LocalName();
    pCurIndex->Term()
        <<  " - "
        <<  i_sType;

    if ( rOwner.Owner() != 0 )
    {
        aQualification.seekp(0);
        Env().Gate().Get_QualifiedName( aQualification,
                                        rOwner.LocalName(),
                                        rOwner.Owner() );

        pCurIndex->Term()
            <<  " in "
            << i_sOwnerType
            << " "
            << aQualification.c_str();
    }

    pCurIndex->Def() << " ";
}



