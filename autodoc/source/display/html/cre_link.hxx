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



#ifndef ADC_DISPLAY_CRE_LINK_HXX
#define ADC_DISPLAY_CRE_LINK_HXX


// USED SERVICES
    // BASE CLASSES
#include <cosv/tpl/processor.hxx>
    // COMPONENTS
    // PARAMETERS

namespace ary
{
namespace cpp
{
     class CodeEntity;
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


class OuputPage_Environment;



/** Displays links to ->{ary::cpp::CodeEntity CodeEntites}.
*/
class LinkCreator : public csv::ProcessorIfc,
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
                        LinkCreator(
                            char *              o_rOutput,
                            uintt               i_nOutputSize );
                        ~LinkCreator();


    void                SetEnv(
                            const OuputPage_Environment &
                                                i_rEnv );
  private:
    void                Create_PrePath(
                            const ary::cpp::CodeEntity &
                                                i_rData );
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
    // DATA
    char *              pOut;
    uintt               nOutMaxSize;
    const OuputPage_Environment *
                        pEnv;
};




// IMPLEMENTATION
inline void
LinkCreator::SetEnv( const OuputPage_Environment & i_rEnv )
    { pEnv = &i_rEnv; }




#endif
