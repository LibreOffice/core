/*************************************************************************
 *
 *  $RCSfile: hd_chlst.hxx,v $
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

#ifndef ADC_DISPLAY_HTML_HD_CHLST_HXX
#define ADC_DISPLAY_HTML_HD_CHLST_HXX



// USED SERVICES
    // BASE CLASSES
#include <ary/cpp/cpp_disp.hxx>
#include <ary/cpp/c_etypes.hxx>
#include "hdimpl.hxx"
    // COMPONENTS
    // PARAMETERS

namespace ary
{
    namespace cpp
    {
        class Namespace;
        class Class;
        class Enum;
        class Typedef;
        class Function;
        class Variable;
        class EnumValue;
    }
    namespace info
    {
        class DocuText;
    }
}

class Docu_Display;
class ProtectionArea;

class ChildList_Display : public ary::cpp::Display,
                          private HtmlDisplay_Impl
{
  public:
    struct Area_Result
    {
        bool &              rChildrenExist;
        csi::xml::Element & rOut;

                            Area_Result(
                                bool &                  o_rChildrenExist,
                                csi::xml::Element &     o_rOut )
                                :   rChildrenExist(o_rChildrenExist),
                                    rOut(o_rOut) {}
    };


                        ChildList_Display(
                            OuputPage_Environment &
                                                io_rEnv );
                        ChildList_Display(
                            OuputPage_Environment &
                                                io_rEnv,
                            const ary::cpp::Class &
                                                i_rClass );
                        ChildList_Display(
                            OuputPage_Environment &
                                                io_rEnv,
                            const ary::cpp::Enum &
                                                i_rEnum );

    virtual             ~ChildList_Display();

    void                Run_Simple(
                            Area_Result &       o_rResult,
                            ary::SlotAccessId   i_nSlot,
                            const char *        i_sListLabel,
                            const char *        i_sListTitle );
    void                Run_GlobalClasses(
                            Area_Result &       o_rResult,
                            ary::SlotAccessId   i_nSlot,
                            const char *        i_sListLabel,
                            const char *        i_sListTitle,
                            ary::cpp::E_ClassKey
                                                i_eFilter );
#if 0
    void                Run_GlobalClasses(
                            Area_Result &       o_rResult,
                            ary::SlotAccessId   i_nSlot,
                            const char *        i_sListLabel,
                            const char *        i_sListTitle_classes,
                            const char *        i_sListTitle_structs,
                            const char *        i_sListTitle_unions );
#endif
    void                Run_Members(
                            Area_Result &       o_rResult_public,
                            Area_Result &       o_rResult_protected,
                            Area_Result &       o_rResult_private,
                            ary::SlotAccessId   i_nSlot,
                            const char *        i_sListLabel_public,
                            const char *        i_sListLabel_protected,
                            const char *        i_sListLabel_private,
                            const char *        i_sListTitle );
    void                Run_MemberClasses(
                            Area_Result &       o_rResult_public,
                            Area_Result &       o_rResult_protected,
                            Area_Result &       o_rResult_private,
                            ary::SlotAccessId   i_nSlot,
                            const char *        i_sListLabel_public,
                            const char *        i_sListLabel_protected,
                            const char *        i_sListLabel_private,
                            const char *        i_sListTitle,
                            ary::cpp::E_ClassKey
                                                i_eFilter );
#if 0
    void                Run_MemberClasses(
                            Area_Result &       o_rResult_public,
                            Area_Result &       o_rResult_protected,
                            Area_Result &       o_rResult_private,
                            ary::SlotAccessId   i_nSlot,
                            const char *        i_sListLabel_public,
                            const char *        i_sListLabel_protected,
                            const char *        i_sListLabel_private,
                            const char *        i_sListTitle_classes,
                            const char *        i_sListTitle_structs,
                            const char *        i_sListTitle_unions );
#endif

    virtual void        Display_Namespace(
                            const ary::cpp::Namespace &
                                                i_rData );
    /** i_rData is showed only, if it passes two filters:
        it must have the right protection, checked with pFilter,
        and the right class key (class,struct,union), checked with
        pClassFilter. A not exsting filter allows i_rData to be
        displayed.
    */
    virtual void        Display_Class(
                            const ary::cpp::Class &
                                                i_rData );
    virtual void        Display_Enum(
                            const ary::cpp::Enum &
                                                i_rData );
    virtual void        Display_Typedef(
                            const ary::cpp::Typedef &
                                                i_rData );
    virtual void        Display_Function(
                            const ary::cpp::Function &
                                                i_rData );
    virtual void        Display_Variable(
                            const ary::cpp::Variable &
                                                i_rData );
    virtual void        Display_EnumValue(
                            const ary::cpp::EnumValue &
                                                i_rData );
  private:
    // Interface ary::Display:
    virtual void        do_StartSlot();
    virtual void        do_FinishSlot();
    virtual const ary::DisplayGate *
                        inq_Get_ReFinder() const;

    // Locals
    struct S_AreaCo;
    void                Write_ListItem(
                            const udmstri &     i_sLeftText,
                            const char *        i_sLink,
                            const ary::info::DocuText &
                                                i_rRightText,
                            csi::xml::Element & rOut );
    const ary::AryGroup &
                        ActiveParent();
    ProtectionArea &    GetArea();
    ProtectionArea &    GetArea(
                            ary::cpp::E_Protection
                                                i_eProtection );
    void                SetClassesFilter(
                            ary::cpp::E_ClassKey
                                                i_eFilter )
                                                { peClassesFilter = new ary::cpp::E_ClassKey(i_eFilter); }
    void                UnsetClassesFilter()    { peClassesFilter = 0; }

    // DATA
    Dyn<Docu_Display>   pShortDocu_Display;
    const ary::cpp::Class *
                        pActiveParentClass;
    const ary::cpp::Enum *
                        pActiveParentEnum;

    Dyn<S_AreaCo>       pSglArea;
    Dyn<S_AreaCo>       aMemberAreas[3];

    Dyn<ary::cpp::E_ClassKey>
                        peClassesFilter;
};



#endif

