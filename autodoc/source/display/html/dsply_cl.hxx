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



#ifndef ADC_DISPLAY_HTML_HD_PAGE_HXX
#define ADC_DISPLAY_HTML_HD_PAGE_HXX

// BASE CLASSES
#include <ary/ary_disp.hxx>
#include <cosv/tpl/processor.hxx>
// USED SERVICES
#include <ary/types.hxx>

class OuputPage_Environment;

namespace ary
{
namespace cpp
{
    class Class;
}
}

class PageDisplay;




class ClassDisplayer : public ary::Display,
                       public csv::ConstProcessor<ary::cpp::Class>
{
  public:
                        ClassDisplayer(            // TODO
                            OuputPage_Environment &
                                                io_rEnv );
    virtual             ~ClassDisplayer();

 private:
    // Interface csv::ConstProcessor<>:
    virtual void        do_Process(
                            const ary::cpp::Class &
                                                i_data );
    // Interface ary::Display:
    virtual const ary::cpp::Gate *
                        inq_Get_ReFinder() const;

    // Locals
    void                DisplayFiles_InClass(
                            const ary::cpp::Class &
                                                i_rData,
                            PageDisplay &       io_rPageMaker );

    OuputPage_Environment &
                        Env()                   { return *pEnv; }

    // DATA
    OuputPage_Environment *
                        pEnv;
};




#endif
