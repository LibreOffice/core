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



#ifndef ADC_CPP_PRS_CPP_HXX
#define ADC_CPP_PRS_CPP_HXX



// USED SERVICES
    // BASE CLASSES
#include <autodoc/prs_code.hxx>
    // COMPONENTS
    // PARAMETERS

namespace cpp
{

struct S_RunningData;

class Cpluplus_Parser : public autodoc::CodeParser_Ifc
{
  public:
                        Cpluplus_Parser();
    virtual             ~Cpluplus_Parser();


    virtual void        Setup(
                            ary::Repository &   o_rRepository,
                            const autodoc::DocumentationParser_Ifc &
                                                i_rDocumentationInterpreter,
                            const ::std::vector<String> &
                                                ignoreDefines );

    virtual void        Run(
                            const autodoc::FileCollector_Ifc &
                                                i_rFiles );
  private:
    Dyn<S_RunningData>  pRunningData;
};




}   // namespace cpp
#endif
