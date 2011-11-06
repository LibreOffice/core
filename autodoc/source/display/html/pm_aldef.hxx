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



#ifndef ADC_DISPLAY_HTML_PM_ALDEF_HXX
#define ADC_DISPLAY_HTML_PM_ALDEF_HXX

// BASE CLASSES
#include "pm_base.hxx"
// USED SERVICES
#include <ary/cpp/c_types4cpp.hxx>
using ary::cpp::De_id;

namespace ary
{
    namespace cpp
    {
        class DefineEntity;
    }
}

class Docu_Display;
class NavigationBar;





class PageMaker_AllDefs : public SpecializedPageMaker
{
  public:
                        PageMaker_AllDefs(
                            PageDisplay &       io_rPage );

    virtual             ~PageMaker_AllDefs();

    virtual void        MakePage();

  private:
    typedef std::vector<De_id>              List_Ids;
    typedef List_Ids::const_iterator        ids_iterator;

    virtual void        Write_NavBar();
    virtual void        Write_TopArea();
    virtual void        Write_DocuArea();
    virtual void        Write_DefinesList();
    virtual void        Write_MacrosList();
    void                Write_Define(
                            De_id               i_nId );
    void                Write_Macro(
                            De_id               i_nId );
    void                Write_DefsDocu(
                            csi::xml::Element & o_rOut,
                            const ary::cpp::DefineEntity &
                                                i_rTextReplacement );

    // DATA
    Dyn<Docu_Display>   pDocuDisplay;
    Dyn<NavigationBar>  pNavi;
};




#endif
