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



#ifndef ADC_DISPLAY_HTML_PM_INDEX_HXX
#define ADC_DISPLAY_HTML_PM_INDEX_HXX

// BASE CLASSES
#include "pm_base.hxx"
#include <ary/ary_disp.hxx>
#include <cosv/tpl/processor.hxx>
// USED SERVICES
namespace adcdisp
{
     class IndexList;
}
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
        class Define;
        class Macro;
    }
}

class NavigationBar;




class PageMaker_Index : public SpecializedPageMaker,
                        public ary::Display,
                        public csv::ConstProcessor<ary::cpp::Namespace>,
                        public csv::ConstProcessor<ary::cpp::Class>,
                        public csv::ConstProcessor<ary::cpp::Enum>,
                        public csv::ConstProcessor<ary::cpp::Typedef>,
                        public csv::ConstProcessor<ary::cpp::Function>,
                        public csv::ConstProcessor<ary::cpp::Variable>,
                        public csv::ConstProcessor<ary::cpp::EnumValue>,
                        public csv::ConstProcessor<ary::cpp::Define>,
                        public csv::ConstProcessor<ary::cpp::Macro>
{
  public:
                        PageMaker_Index(
                            PageDisplay &       io_rPage,
                            char                i_c );

    virtual             ~PageMaker_Index();

    virtual void        MakePage();

  private:
    // Interface csv::ConstProcessor<>
    virtual void        do_Process(
                            const ary::cpp::Namespace &
                                                i_rData );
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
    virtual void        do_Process(
                            const ary::cpp::Define &
                                                i_rData );
    virtual void        do_Process(
                            const ary::cpp::Macro &
                                                i_rData );
    // Interface ary::cpp::Display:
    virtual const ary::cpp::Gate *
                        inq_Get_ReFinder() const;
    // Locals
    virtual void        Write_NavBar();
    virtual void        Write_TopArea();
    virtual void        Write_CompleteAlphabeticalList();

    void                Write_CeIndexEntry(
                            const ary::cpp::CodeEntity &
                                                i_rCe,
                            const char *        i_sType,
                            const char *        i_sOwnerType );

    // DATA
    Dyn<NavigationBar>  pNavi;
    char                c;
    adcdisp::IndexList *
                        pCurIndex;
};




#endif
