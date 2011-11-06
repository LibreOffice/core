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



#ifndef ADC_DISPLAY_HTML_DSPLY_OP_HXX
#define ADC_DISPLAY_HTML_DSPLY_OP_HXX

// BASE CLASSES
#include <ary/ary_disp.hxx>
#include <cosv/tpl/processor.hxx>
// USED SERVICES
#include <ary/cpp/c_ce.hxx>

namespace ary
{
    namespace cpp
    {
        class Function;
    }
}
namespace csi
{
    namespace xml
    {
        class Element;
    }
}

class OuputPage_Environment;
class PageDisplay;
class Docu_Display;




class OperationsDisplay : public ary::Display,
                          public csv::ConstProcessor<ary::cpp::Function>
{
  public:
                        OperationsDisplay(
                            OuputPage_Environment &
                                                io_rInfo );
    virtual             ~OperationsDisplay();

    void                PrepareForStdMembers();
    void                PrepareForStaticMembers();
    void                Create_Files();

 private:
    // Interface csv::ConstProcessor<>:
    virtual void        do_Process(
                            const ary::cpp::Function &
                                                i_rData );
    // Interface ary::Display:
    virtual const ary::cpp::Gate *
                        inq_Get_ReFinder() const;

    // Locals
    typedef ary::cpp::Lid                               SourceFileId;
    typedef std::map< SourceFileId, DYN PageDisplay* >  Map_FileId2PagePtr;

    PageDisplay &       FindPage_for(
                            const ary::cpp::Function &
                                                i_rData );
    void                Display_SglOperation(
                            csi::xml::Element & rOut,
                            const ary::cpp::Function &
                                                i_rData );
    // DATA
    Map_FileId2PagePtr  aMap_GlobalFunctionsDisplay;
    Dyn<PageDisplay>    pClassMembersDisplay;

    OuputPage_Environment *
                        pEnv;
    Dyn<Docu_Display>   pDocuShow;
};




#endif
