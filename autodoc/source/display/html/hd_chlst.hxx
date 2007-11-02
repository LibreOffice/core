/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hd_chlst.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:26:52 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef ADC_DISPLAY_HTML_HD_CHLST_HXX
#define ADC_DISPLAY_HTML_HD_CHLST_HXX

// BASE CLASSES
#include <ary/ary_disp.hxx>
#include <cosv/tpl/processor.hxx>

// USED SERVICES
#include <ary/cpp/c_types4cpp.hxx>
#include "hdimpl.hxx"



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

class ChildList_Display : public ary::Display,
                          public csv::ConstProcessor<ary::cpp::Namespace>,
                          public csv::ConstProcessor<ary::cpp::Class>,
                          public csv::ConstProcessor<ary::cpp::Enum>,
                          public csv::ConstProcessor<ary::cpp::Typedef>,
                          public csv::ConstProcessor<ary::cpp::Function>,
                          public csv::ConstProcessor<ary::cpp::Variable>,
                          public csv::ConstProcessor<ary::cpp::EnumValue>,
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
  private:
    // Interface csv::ConstProcessor<>:
    virtual void        do_Process(
                            const ary::cpp::Namespace &
                                                i_rData );
    /** i_rData is shown only, if it passes two filters:
        it must have the right protection, checked with pFilter,
        and the right class key (class,struct,union), checked with
        pClassFilter. A not exsting filter allows i_rData to be
        displayed.
    */
    virtual void        do_Process(
                            const ary::cpp::Class &
                                                i_rData );
    virtual void        do_Process(
                            const ary::cpp::Enum &
                                                i_rData );
    virtual void        do_Process(
                            const ary::cpp::Typedef &
                                                i_rData );
    virtual void        do_Process(
                            const ary::cpp::Function &
                                                i_rData );
    virtual void        do_Process(
                            const ary::cpp::Variable &
                                                i_rData );
    virtual void        do_Process(
                            const ary::cpp::EnumValue &
                                                i_rData );
  private:
    // Interface ary::Display:
    virtual void        do_StartSlot();
    virtual void        do_FinishSlot();
    virtual const ary::cpp::Gate *
                        inq_Get_ReFinder() const;
    // Locals
    struct S_AreaCo;
    void                Write_ListItem(
                            const String &      i_sLeftText,
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
