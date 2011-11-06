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
