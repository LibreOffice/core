/*************************************************************************
 *
 *  $RCSfile: hd_chlst.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:23 $
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
#include "hd_chlst.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/ceslot.hxx>
#include <ary/cpp/c_disply.hxx>
#include <ary/cpp/c_namesp.hxx>
#include <ary/cpp/c_class.hxx>
#include <ary/cpp/c_enum.hxx>
#include <ary/cpp/c_tydef.hxx>
#include <ary/cpp/c_funct.hxx>
#include <ary/cpp/c_vari.hxx>
#include <ary/cpp/c_enuval.hxx>
#include <ary/cpp/cg_file.hxx>
#include <ary/cpp/crog_grp.hxx>
#include <ary/info/codeinfo.hxx>
#include <ary/info/ci_attag.hxx>
#include <ary/info/ci_text.hxx>
#include <ary/info/all_dts.hxx>
#include "hd_docu.hxx"
#include "opageenv.hxx"
#include "protarea.hxx"
#include "strconst.hxx"


using namespace csi;
using html::Table;
using html::TableRow;
using html::TableCell;
using html::Font;
using html::SizeAttr;
using html::BgColorAttr;
using html::WidthAttr;


const int   ixPublic = 0;
const int   ixProtected = 1;
const int   ixPrivate = 2;

struct ChildList_Display::S_AreaCo
{
  public:
    ProtectionArea      aArea;
    Area_Result *       pResult;

                        S_AreaCo(
                            Area_Result &       o_rResult,
                            const char *        i_sLabel,
                            const char *        i_sTitle );
                        S_AreaCo(
                            Area_Result &       o_rResult,
                            const char *        i_sLabel,
                            const char *        i_sTitle_classes,
                            const char *        i_sTitle_structs,
                            const char *        i_sTitle_unions );
                        ~S_AreaCo();

    void                PerformResult();

  private:
    csi::xml::Element & Out()                   { return pResult->rOut; }
};


const ary::info::DocuText &
ShortDocu( const ary::CodeEntity & i_rCe )
{
    static const ary::info::DocuText aNull_;

    const ary::info::CodeInfo *
            pInfo = dynamic_cast< const ary::info::CodeInfo* >( &i_rCe.Info() );
    if (pInfo == 0)
        return aNull_;

    return pInfo->Short().CText();
}


ChildList_Display::ChildList_Display( OuputPage_Environment & io_rEnv )
    :   HtmlDisplay_Impl( io_rEnv ),
        pShortDocu_Display( new Docu_Display(io_rEnv) ),
        pActiveParentClass(0),
        pActiveParentEnum(0),
        // pSglArea,
        // aMemberAreas,
        peClassesFilter(0)
{
}

ChildList_Display::ChildList_Display( OuputPage_Environment & io_rEnv,
                                      const ary::cpp::Class & i_rClass )
    :   HtmlDisplay_Impl( io_rEnv ),
        pShortDocu_Display( new Docu_Display(io_rEnv) ),
        pActiveParentClass(&i_rClass),
        pActiveParentEnum(0),
        // pSglArea,
        // aMemberAreas,
        peClassesFilter(0)
{
}

ChildList_Display::ChildList_Display( OuputPage_Environment & io_rEnv,
                                      const ary::cpp::Enum &  i_rEnum )
    :   HtmlDisplay_Impl( io_rEnv ),
        pShortDocu_Display( new Docu_Display(io_rEnv) ),
        pActiveParentClass(0),
        pActiveParentEnum(&i_rEnum),
        // pSglArea,
        // aMemberAreas,
        peClassesFilter(0)
{
}

ChildList_Display::~ChildList_Display()
{
}

void
ChildList_Display::Run_Simple( Area_Result &       o_rResult,
                               ary::SlotAccessId   i_nSlot,
                               const char *        i_sListLabel,
                               const char *        i_sListTitle )
{
    ary::Slot_AutoPtr
            pSlot( ActiveParent().Create_Slot( i_nSlot ) );
    if ( pSlot->Size() == 0 )
        return;

    pSglArea = new S_AreaCo( o_rResult,
                             i_sListLabel,
                             i_sListTitle );

    pSlot->StoreAt(*this);

    pSglArea->PerformResult();
    pSglArea = 0;
}

void
ChildList_Display::Run_GlobalClasses( Area_Result &        o_rResult,
                                      ary::SlotAccessId    i_nSlot,
                                      const char *         i_sListLabel,
                                      const char *         i_sListTitle,
                                      ary::cpp::E_ClassKey i_eFilter )
{
    ary::Slot_AutoPtr
            pSlot( ActiveParent().Create_Slot( i_nSlot ) );
    if ( pSlot->Size() == 0 )
        return;

    pSglArea = new S_AreaCo( o_rResult,
                             i_sListLabel,
                             i_sListTitle );

    SetClassesFilter(i_eFilter);
    pSlot->StoreAt(*this);
    UnsetClassesFilter();

    pSglArea->PerformResult();
    pSglArea = 0;
}

#if 0   // old
void
ChildList_Display::Run_GlobalClasses( Area_Result &       o_rResult,
                                      ary::SlotAccessId   i_nSlot,
                                      const char *        i_sListLabel,
                                      const char *        i_sListTitle_classes,
                                      const char *        i_sListTitle_structs,
                                      const char *        i_sListTitle_unions )
{
    ary::Slot_AutoPtr
            pSlot( ActiveParent().Create_Slot( i_nSlot ) );
    if ( pSlot->Size() == 0 )
        return;

    pSglArea = new S_AreaCo( o_rResult,
                           i_sListLabel,
                           i_sListTitle_classes,
                           i_sListTitle_structs,
                           i_sListTitle_unions );

    pSlot->StoreAt(*this);

    pSglArea->PerformResult();
    pSglArea = 0;
}

#endif // 0,  old

void
ChildList_Display::Run_Members( Area_Result &           o_rResult_public,
                                Area_Result &           o_rResult_protected,
                                Area_Result &           o_rResult_private,
                                ary::SlotAccessId       i_nSlot,
                                const char *            i_sListLabel_public,
                                const char *            i_sListLabel_protected,
                                const char *            i_sListLabel_private,
                                const char *            i_sListTitle )
{
    ary::Slot_AutoPtr
            pSlot( ActiveParent().Create_Slot(i_nSlot) );
    if ( pSlot->Size() == 0 )
        return;

    aMemberAreas[ixPublic] = new S_AreaCo( o_rResult_public,
                                         i_sListLabel_public,
                                         i_sListTitle );
    aMemberAreas[ixProtected] = new S_AreaCo( o_rResult_protected,
                                            i_sListLabel_protected,
                                            i_sListTitle );
    aMemberAreas[ixPrivate] = new S_AreaCo( o_rResult_private,
                                          i_sListLabel_private,
                                          i_sListTitle );

    pSlot->StoreAt(*this);

    aMemberAreas[ixPublic]->PerformResult();
    aMemberAreas[ixProtected]->PerformResult();
    aMemberAreas[ixPrivate]->PerformResult();

    aMemberAreas[ixPublic] = 0;
    aMemberAreas[ixProtected] = 0;
    aMemberAreas[ixPrivate] = 0;
}

void
ChildList_Display::Run_MemberClasses( Area_Result &         o_rResult_public,
                                      Area_Result &         o_rResult_protected,
                                      Area_Result &         o_rResult_private,
                                      ary::SlotAccessId     i_nSlot,
                                      const char *          i_sListLabel_public,
                                      const char *          i_sListLabel_protected,
                                      const char *          i_sListLabel_private,
                                      const char *          i_sListTitle,
                                      ary::cpp::E_ClassKey  i_eFilter )
{
    ary::Slot_AutoPtr
            pSlot( ActiveParent().Create_Slot(i_nSlot) );
    if ( pSlot->Size() == 0 )
        return;

    aMemberAreas[ixPublic] = new S_AreaCo( o_rResult_public,
                                           i_sListLabel_public,
                                           i_sListTitle );
    aMemberAreas[ixProtected] = new S_AreaCo( o_rResult_protected,
                                              i_sListLabel_protected,
                                              i_sListTitle );
    aMemberAreas[ixPrivate] = new S_AreaCo( o_rResult_private,
                                            i_sListLabel_private,
                                            i_sListTitle );

    SetClassesFilter(i_eFilter);
    pSlot->StoreAt(*this);
    UnsetClassesFilter();

    aMemberAreas[ixPublic]->PerformResult();
    aMemberAreas[ixProtected]->PerformResult();
    aMemberAreas[ixPrivate]->PerformResult();

    aMemberAreas[ixPublic] = 0;
    aMemberAreas[ixProtected] = 0;
    aMemberAreas[ixPrivate] = 0;
}


#if 0 // old
void
ChildList_Display::Run_MemberClasses( Area_Result &       o_rResult_public,
                                      Area_Result &       o_rResult_protected,
                                      Area_Result &       o_rResult_private,
                                      ary::SlotAccessId   i_nSlot,
                                      const char *        i_sListLabel_public,
                                      const char *        i_sListLabel_protected,
                                      const char *        i_sListLabel_private,
                                      const char *        i_sListTitle_classes,
                                      const char *        i_sListTitle_structs,
                                      const char *        i_sListTitle_unions )
{
    ary::Slot_AutoPtr
            pSlot( ActiveParent().Create_Slot(i_nSlot) );
    if ( pSlot->Size() == 0 )
        return;

    aMemberAreas[ixPublic] = new S_AreaCo( o_rResult_public,
                                         i_sListLabel_public,
                                         i_sListTitle_classes,
                                         i_sListTitle_structs,
                                         i_sListTitle_unions );
    aMemberAreas[ixProtected] = new S_AreaCo( o_rResult_protected,
                                            i_sListLabel_protected,
                                         i_sListTitle_classes,
                                         i_sListTitle_structs,
                                         i_sListTitle_unions );
    aMemberAreas[ixPrivate] = new S_AreaCo( o_rResult_private,
                                          i_sListLabel_private,
                                         i_sListTitle_classes,
                                         i_sListTitle_structs,
                                         i_sListTitle_unions );

    pSlot->StoreAt(*this);

    aMemberAreas[ixPublic]->PerformResult();
    aMemberAreas[ixProtected]->PerformResult();
    aMemberAreas[ixPrivate]->PerformResult();

    aMemberAreas[ixPublic] = 0;
    aMemberAreas[ixProtected] = 0;
    aMemberAreas[ixPrivate] = 0;
}

#endif // 0,  old


void
ChildList_Display::Display_Namespace( const ary::cpp::Namespace & i_rData )
{
    Write_ListItem( i_rData.LocalName(),
                    Path2ChildNamespace(i_rData.LocalName()),
                    ShortDocu( i_rData ),
                    GetArea().GetTable() );
}

void
ChildList_Display::Display_Class( const ary::cpp::Class & i_rData )
{
    if ( Ce_IsInternal(i_rData) )
        return;

    if (peClassesFilter)
    {
         if (*peClassesFilter != i_rData.ClassKey() )
            return;
    }

    udmstri sLink;
    if ( i_rData.Protection() == ary::cpp::PROTECT_global )
    {
        sLink = ClassFileName(i_rData.LocalName());
    }
    else
    {
        csv_assert( pActiveParentClass != 0 );
        sLink = Path2Child( ClassFileName(i_rData.LocalName()), pActiveParentClass->LocalName() );
    }

    if (peClassesFilter)
    {
        Write_ListItem( i_rData.LocalName(),
                        sLink,
                        ShortDocu( i_rData ),
                        GetArea(i_rData.Protection())
                            .GetTable() );
    }
    else
    {
        Write_ListItem( i_rData.LocalName(),
                        sLink,
                        ShortDocu( i_rData ),
                        GetArea(i_rData.Protection())
                            .GetTable(i_rData.ClassKey()) );
    }
}

void
ChildList_Display::Display_Enum( const ary::cpp::Enum & i_rData )
{
    if ( Ce_IsInternal(i_rData) )
        return;

    udmstri sLink;
    if ( i_rData.Protection() == ary::cpp::PROTECT_global )
    {
        sLink = EnumFileName(i_rData.LocalName());
    }
    else
    {
        csv_assert( pActiveParentClass != 0 );
        sLink = Path2Child( EnumFileName(i_rData.LocalName()),
                            pActiveParentClass->LocalName() );
    }

    Write_ListItem( i_rData.LocalName(),
                    sLink,
                    ShortDocu( i_rData ),
                    GetArea(i_rData.Protection()).GetTable() );
}

void
ChildList_Display::Display_Typedef( const ary::cpp::Typedef & i_rData )
{
    if ( Ce_IsInternal(i_rData) )
        return;

    udmstri sLink;
    if ( i_rData.Protection() == ary::cpp::PROTECT_global )
    {
        sLink = TypedefFileName(i_rData.LocalName());
    }
    else
    {
        csv_assert( pActiveParentClass != 0 );
        sLink = Path2Child( TypedefFileName(i_rData.LocalName()),
                            pActiveParentClass->LocalName() );
    }

    Write_ListItem( i_rData.LocalName(),
                    sLink,
                    ShortDocu( i_rData ),
                    GetArea(i_rData.Protection()).GetTable() );
}

void
ChildList_Display::Display_Function( const ary::cpp::Function & i_rData )
{
    if ( Ce_IsInternal(i_rData) )
        return;

    udmstri sLinkPrePath;
    if ( i_rData.Protection() == ary::cpp::PROTECT_global )
    {
        const ary::cpp::FileGroup *
                pFgr = Env().Gate().RoGroups().Search_FileGroup( i_rData.Location() );
        csv_assert( pFgr != 0 );
        sLinkPrePath = HtmlFileName( "o-", pFgr->FileName() );
    }
    else
    {
        csv_assert( pActiveParentClass != 0 );
        sLinkPrePath = Path2Child( HtmlFileName( "o", "" ),
                                   pActiveParentClass->LocalName() );
    }

    // Out
    Table & rOut = GetArea(i_rData.Protection()).GetTable();
    TableRow * dpRow = new TableRow;
    rOut << dpRow;
    TableCell & rCell1 = dpRow->AddCell();

    rCell1
        << SyntaxText_PreName( i_rData, Env().Gate() )
        << new html::LineBreak;
    rCell1
          >> *new html::Link( OperationLink(i_rData.LocalName(),
                              i_rData.Signature(),
                              sLinkPrePath) )
          << i_rData.LocalName();
    rCell1
        << SyntaxText_PostName( i_rData, Env().Gate() );
    TableCell & rCell2 = dpRow->AddCell();
    rCell2
        << new WidthAttr("50%")
        << " ";

    pShortDocu_Display->Assign_Out( rCell2 );
    ShortDocu( i_rData ).StoreAt( *pShortDocu_Display );
    pShortDocu_Display->Unassign_Out();
}

void
ChildList_Display::Display_Variable( const ary::cpp::Variable & i_rData )
{
    if ( Ce_IsInternal(i_rData) )
        return;

    udmstri sLinkPrePath;
    if ( i_rData.Protection() == ary::cpp::PROTECT_global )
    {
        const ary::cpp::FileGroup *
                pFgr = Env().Gate().RoGroups().Search_FileGroup( i_rData.Location() );
        csv_assert( pFgr != 0 );
        sLinkPrePath = HtmlFileName( "d-", pFgr->FileName() );
    }
    else
    {
        csv_assert( pActiveParentClass != 0 );
        sLinkPrePath = Path2Child( HtmlFileName( "d", "" ),
                                   pActiveParentClass->LocalName() );
    }

    TableRow * dpRow = new TableRow;
    GetArea(i_rData.Protection()).GetTable() << dpRow;

    *dpRow << new html::BgColorAttr("white");
    csi::xml::Element &
        rCell1 = dpRow->AddCell();

    dshelp::Get_LinkedTypeText( rCell1, Env(), i_rData.Type() );
    rCell1
        << " "
        >> *new html::Link( DataLink(i_rData.LocalName(), sLinkPrePath.c_str()) )
            >> *new html::Strong
                << i_rData.LocalName()
                << ";";

    TableCell & rShortDocu = dpRow->AddCell();
    pShortDocu_Display->Assign_Out( rShortDocu );
    ShortDocu( i_rData ).StoreAt( *pShortDocu_Display );
    pShortDocu_Display->Unassign_Out();
}

void
ChildList_Display::Display_EnumValue( const ary::cpp::EnumValue & i_rData )
{
    if ( Ce_IsInternal(i_rData) )
        return;

    Table & rOut = GetArea().GetTable();

    TableRow * dpRow = new TableRow;
    rOut << dpRow;

    *dpRow << new html::BgColorAttr("white");
    dpRow->AddCell()
        << new WidthAttr("20%")
        << new xml::AnAttribute("valign", "top")
        >> *new html::Label(i_rData.LocalName())
            >> *new html::Bold
                << i_rData.LocalName();

    TableCell & rValueDocu = dpRow->AddCell();
    pShortDocu_Display->Assign_Out( rValueDocu );
    i_rData.Info().StoreAt( *pShortDocu_Display );
    pShortDocu_Display->Unassign_Out();
}

void
ChildList_Display::do_StartSlot()
{
}

void
ChildList_Display::do_FinishSlot()
{
}

const ary::DisplayGate *
ChildList_Display::inq_Get_ReFinder() const
{
    return & Env().Gate();
}

void
ChildList_Display::Write_ListItem( const udmstri &              i_sLeftText,
                                   const char *                 i_sLink,
                                   const ary::info::DocuText &  i_rRightText,
                                   csi::xml::Element &          o_rOut )
{
    TableRow * dpRow = new TableRow;
    o_rOut << dpRow;

    *dpRow << new html::BgColorAttr("white");
    dpRow->AddCell()
        << new WidthAttr("20%")
        >> *new html::Link( i_sLink )
                >> *new html::Bold
                        << i_sLeftText;

    TableCell & rShortDocu = dpRow->AddCell();
    pShortDocu_Display->Assign_Out( rShortDocu );
    i_rRightText.StoreAt( *pShortDocu_Display );
    pShortDocu_Display->Unassign_Out();
}

const ary::AryGroup &
ChildList_Display::ActiveParent()
{
    return pActiveParentClass != 0
                ?   static_cast< const ary::AryGroup& >(*pActiveParentClass)
                :   pActiveParentEnum != 0
                        ?   static_cast< const ary::AryGroup& >(*pActiveParentEnum)
                        :   static_cast< const ary::AryGroup& >(*Env().CurNamespace());
}

ProtectionArea &
ChildList_Display::GetArea()
{
    return pSglArea->aArea;
}

ProtectionArea &
ChildList_Display::GetArea( ary::cpp::E_Protection i_eProtection )
{
    switch ( i_eProtection )
    {
         case ary::cpp::PROTECT_public:
                        return aMemberAreas[ixPublic]->aArea;
         case ary::cpp::PROTECT_protected:
                        return aMemberAreas[ixProtected]->aArea;
         case ary::cpp::PROTECT_private:
                        return aMemberAreas[ixPrivate]->aArea;
        default:
                        return pSglArea->aArea;
    }
}


//*******************               ********************//

ChildList_Display::
S_AreaCo::S_AreaCo( Area_Result &       o_rResult,
                    const char *        i_sLabel,
                    const char *        i_sTitle )
    :   aArea(i_sLabel, i_sTitle),
        pResult(&o_rResult)
{
}

ChildList_Display::
S_AreaCo::S_AreaCo( Area_Result &       o_rResult,
                    const char *        i_sLabel,
                    const char *        i_sTitle_classes,
                    const char *        i_sTitle_structs,
                    const char *        i_sTitle_unions )
    :   aArea(i_sLabel, i_sTitle_classes, i_sTitle_structs, i_sTitle_unions ),
        pResult(&o_rResult)
{
}

ChildList_Display::
S_AreaCo::~S_AreaCo()
{
}

void
ChildList_Display::
S_AreaCo::PerformResult()
{
    bool bUsed = aArea.WasUsed_Area();
    pResult->rChildrenExist = bUsed;
    if ( bUsed )
    {
        Create_ChildListLabel( Out(), aArea.Label() );

        if ( aArea.Size() == 1 )
        {
            Out() << aArea.ReleaseTable();
        }
        else
        {
             Table * pTable = aArea.ReleaseTable( ary::cpp::CK_class );
            if (pTable != 0)
                Out() << pTable;
             pTable = aArea.ReleaseTable( ary::cpp::CK_struct );
            if (pTable != 0)
                Out() << pTable;
             pTable = aArea.ReleaseTable( ary::cpp::CK_union );
            if (pTable != 0)
                Out() << pTable;
        }
    }
}



