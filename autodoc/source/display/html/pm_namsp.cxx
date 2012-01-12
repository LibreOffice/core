/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#include <precomp.h>
#include "pm_namsp.hxx"


// NOT FULLY DEFINED SERVICES
#include "hd_chlst.hxx"
#include "hd_docu.hxx"
#include "html_kit.hxx"
#include "navibar.hxx"
#include "opageenv.hxx"
#include "pagemake.hxx"
#include "strconst.hxx"


using namespace csi;
using csi::html::HorizontalLine;
using csi::html::Link;
using csi::html::Table;
using csi::html::TableRow;
using csi::html::TableCell;



PageMaker_Namespace::PageMaker_Namespace( PageDisplay & io_rPage )
    :   SpecializedPageMaker(io_rPage),
        pMe( io_rPage.Env().CurNamespace() ),
        pChildDisplay( new ChildList_Display(io_rPage.Env()) ),
        pNavi(0)
{
    csv_assert( pMe != 0 );
}

PageMaker_Namespace::~PageMaker_Namespace()
{
}

void
PageMaker_Namespace::MakePage()
{
    pNavi = new NavigationBar( Env(), Me() );

    Write_NavBar();
    Write_TopArea();
    Write_DocuArea();

    pNavi->MakeSubRow("");
    Write_ChildList( ary::cpp::Namespace::SLOT_SubNamespaces, C_sTitle_SubNamespaces, C_sLabel_SubNamespaces );

    Write_ChildLists_forClasses( C_sTitle_Classes,
                                 C_sLabel_Classes,
                                 ary::cpp::CK_class );
    Write_ChildLists_forClasses( C_sTitle_Structs,
                                 C_sLabel_Structs,
                                 ary::cpp::CK_struct );
    Write_ChildLists_forClasses( C_sTitle_Unions,
                                 C_sLabel_Unions,
                                 ary::cpp::CK_union );

    Write_ChildList( ary::cpp::Namespace::SLOT_Enums, C_sTitle_Enums, C_sLabel_Enums  );
    Write_ChildList( ary::cpp::Namespace::SLOT_Typedefs, C_sTitle_Typedefs, C_sLabel_Typedefs  );
    Write_ChildList( ary::cpp::Namespace::SLOT_Operations, C_sTitle_Operations, C_sLabel_Operations  );
    Write_ChildList( ary::cpp::Namespace::SLOT_Constants, C_sTitle_Constants, C_sLabel_Constants  );
    Write_ChildList( ary::cpp::Namespace::SLOT_Variables, C_sTitle_Variables, C_sLabel_Variables  );

    pNavi->Write_SubRows();
}

void
PageMaker_Namespace::Write_NavBar()
{
    pNavi->Write( CurOut() );
    CurOut() << new HorizontalLine;
}

void
PageMaker_Namespace::Write_TopArea()
{
    Page().Write_NameChainWithLinks( Me() );

    adcdisp::PageTitle_Std fTitle;
    xml::Element & rH3 = fTitle( CurOut() );
    if ( Env().CurNamespace()->Owner().IsValid() )
    {
        rH3 << C_sHFTypeTitle_Namespace
            << " "
            << Env().CurNamespace()->LocalName();
    }
    else
    {
        rH3 << C_sHFTitle_GlobalNamespaceCpp;
    }
    CurOut() << new HorizontalLine;
}

void
PageMaker_Namespace::Write_DocuArea()
{
    Docu_Display aDocuShow( Env() );

    aDocuShow.Assign_Out(CurOut());
    aDocuShow.Process(Me().Docu());
    aDocuShow.Unassign_Out();

    CurOut() << new HorizontalLine;
}

void
PageMaker_Namespace::Write_ChildList( ary::SlotAccessId   i_nSlot,
                                      const char *        i_sListTitle,
                                      const char *        i_sLabel )

{
    bool    bChildrenExist = false;
    ChildList_Display::Area_Result
            aResult( bChildrenExist, CurOut() );

    pChildDisplay->Run_Simple( aResult,
                               i_nSlot,
                               i_sLabel,
                               i_sListTitle );

    pNavi->AddItem(i_sListTitle, i_sLabel, bChildrenExist);
    if (bChildrenExist)
        CurOut() << new HorizontalLine;
}

void
PageMaker_Namespace::Write_ChildLists_forClasses( const char *         i_sListTitle,
                                                  const char *         i_sLabel,
                                                  ary::cpp::E_ClassKey i_eFilter )

{
    bool    bChildrenExist = false;
    ChildList_Display::Area_Result
            aResult( bChildrenExist, CurOut() );

    pChildDisplay->Run_GlobalClasses( aResult,
                                      ary::cpp::Namespace::SLOT_Classes,
                                      i_sLabel,
                                      i_sListTitle,
                                      i_eFilter );

    pNavi->AddItem(i_sListTitle, i_sLabel, bChildrenExist);
    if ( bChildrenExist )
        CurOut() << new HorizontalLine;
}
