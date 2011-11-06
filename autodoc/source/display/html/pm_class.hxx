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



#ifndef ADC_DISPLAY_HTML_PM_CLASS_HXX
#define ADC_DISPLAY_HTML_PM_CLASS_HXX



// USED SERVICES
    // BASE CLASSES
#include "pm_base.hxx"
    // COMPONENTS
    // PARAMETERS


namespace ary
{
     namespace cpp
    {
         class Class;
    }
}

class ChildList_Display;
class NavigationBar;


class PageMaker_Class : public SpecializedPageMaker
{
  public:
    enum E_ChidList
    {
        cl_NestedClasses = 0,
        cl_NestedStructs,
        cl_NestedUnions,
        cl_Enums,
        cl_Typedefs,
        cl_Operations,
        cl_StaticOperations,
        cl_Data,
        cl_StaticData,
        cl_MAX
    };
                        PageMaker_Class(
                            PageDisplay &       io_rPage,
                            const ary::cpp::Class &
                                                i_rClass );

    virtual             ~PageMaker_Class();

    virtual void        MakePage();

  private:
    enum E_MemberProtection { mp_public = 0, mp_protected, mp_private, mp_MAX };

    virtual void        Write_NavBar();
    virtual void        Write_TopArea();
    virtual void        Write_DocuArea();
    virtual void        Write_ChildList(
                            ary::SlotAccessId   i_nSlot,
                            E_ChidList          i_eChildListIndex,
                            const char *        i_sLabel,
                            const char *        i_sListTitle,
                            csi::xml::Element & o_rPublic,
                            csi::xml::Element & o_rProtected,
                            csi::xml::Element & o_rPrivate );
    void                Write_ChildList_forClasses(
                            csi::xml::Element & o_rPublic,
                            csi::xml::Element & o_rProtected,
                            csi::xml::Element & o_rPrivate,
                            const char *        i_sLabel,
                            const char *        i_sListTitle,
                            ary::cpp::E_ClassKey
                                                i_eFilter  );
    void                Write_ChildLists();
    static const char * ChildListLabel(
                            const char *        i_sLabel,
                            E_MemberProtection  i_eMpr );
    csi::html::DefListDefinition &
                        Setup_MemberSegment_Out(
                            E_MemberProtection  i_eMpr );
    void                Create_NaviSubRow(
                            E_MemberProtection  i_eMpr );
    void                Write_BaseHierarchy();
    void                Write_DerivedList();

    const ary::cpp::Class &
                        Me() const              { return *pMe; }
    // DATA
    const ary::cpp::Class *
                        pMe;
    Dyn<ChildList_Display>
                        pChildDisplay;
    Dyn<NavigationBar>  pNavi;

    Dyn<csi::xml::Element>
                        pProtectionArea[mp_MAX];

    bool                bChildLists_Exist[3*cl_MAX];
};





#endif

