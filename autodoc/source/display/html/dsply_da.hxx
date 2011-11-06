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



#ifndef ADC_DISPLAY_HTML_DSPLY_DA_HXX
#define ADC_DISPLAY_HTML_DSPLY_DA_HXX

// BASE CLASSES
#include <ary/ary_disp.hxx>
#include <cosv/tpl/processor.hxx>
// USED SERVICES
#include <ary/cpp/c_ce.hxx>

namespace ary
{
    namespace cpp
    {
        class Variable;
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

class DataDisplay : public ary::Display,
                    public csv::ConstProcessor<ary::cpp::Variable>
{
  public:
                        DataDisplay(
                            OuputPage_Environment &
                                                io_rInfo );
    virtual             ~DataDisplay();

    void                PrepareForConstants();
    void                PrepareForVariables();
    void                PrepareForStdMembers();
    void                PrepareForStaticMembers();

    void                Create_Files();

 private:
    // Interface csv::ConstProcessor<>:
    virtual void        do_Process(
                            const ary::cpp::Variable &
                                                i_rData );
    // Interface ary::cpp::Display:
    virtual const ary::cpp::Gate *
                        inq_Get_ReFinder() const;

    // Locals
    typedef ary::cpp::Lid                               SourceFileId;
    typedef std::map< SourceFileId, DYN PageDisplay* >  Map_FileId2PagePtr;

    PageDisplay &       FindPage_for(
                            const ary::cpp::Variable &
                                                i_rData );
    void                Display_SglDatum(
                            csi::xml::Element & rOut,
                            const ary::cpp::Variable &
                                                i_rData );
    // DATA
    Map_FileId2PagePtr  aMap_GlobalDataDisplay;
    Dyn<PageDisplay>    pClassMembersDisplay;

    OuputPage_Environment *
                        pEnv;
    Dyn<Docu_Display>   pDocuShow;
};




#endif
