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



#ifndef ADC_DISPLAY_HTML_PM_NAMSP_HXX
#define ADC_DISPLAY_HTML_PM_NAMSP_HXX



// USED SERVICES
    // BASE CLASSES
#include "pm_base.hxx"
    // COMPONENTS
    // PARAMETERS

class ChildList_Display;
class NavigationBar;

class PageMaker_Namespace : public SpecializedPageMaker
{
  public:
                        PageMaker_Namespace(
                            PageDisplay &       io_rPage );

    virtual             ~PageMaker_Namespace();

    virtual void        MakePage();

  private:
    virtual void        Write_NavBar();
    virtual void        Write_TopArea();
    virtual void        Write_DocuArea();
    virtual void        Write_ChildList(
                            ary::SlotAccessId   i_nSlot,
                            const char *        i_nListTitle,
                            const char *        i_nLabel );
    void                Write_ChildLists_forClasses(
                            const char *         i_sListTitle,
                            const char *         i_sLabel,
                            ary::cpp::E_ClassKey i_eFilter );

    const ary::cpp::Namespace &
                        Me() const              { return *pMe; }
    // DATA
    const ary::cpp::Namespace *
                        pMe;
    Dyn<ChildList_Display>
                        pChildDisplay;
    Dyn<NavigationBar>  pNavi;
};



#endif

