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



#ifndef ADC_PARSEENV_HXX
#define ADC_PARSEENV_HXX



// USED SERVICES
    // BASE CLASSES
#include <tokens/tokproct.hxx>
    // COMPONENTS
    // PARAMETERS


namespace ary
{
namespace info
{
class CodeInfo;
}   // namespace info
}   // namespace ary)



class SubPeUseIfc;


class ParseEnvironment : protected TokenProcessing_Types
{
  public:
    virtual             ~ParseEnvironment() {}

    // Parsing
    void                Enter(
                            E_EnvStackAction    i_eWayOfEntering );
    void                Leave(
                            E_EnvStackAction    i_eWayOfLeaving );
    void                SetCurSPU(
                            const SubPeUseIfc * i_pCurSPU );

    ParseEnvironment *  Parent() const;


    // ACCESS
  protected:
                        ParseEnvironment(
                            ParseEnvironment *  i_pParent );
    const SubPeUseIfc * CurSubPeUse() const;
  private:
    virtual void        InitData() = 0;
    virtual void        TransferData() = 0;

    ParseEnvironment *  pParent;
    const SubPeUseIfc * pCurSubPe;
};

class SubPeUseIfc
{
  public:
    virtual             ~SubPeUseIfc() {}

    virtual void        InitParse() const = 0;
    virtual void        GetResults() const = 0;
};



// IMPLEMENTATION

inline void
ParseEnvironment::SetCurSPU( const SubPeUseIfc * i_pCurSPU )
    { pCurSubPe = i_pCurSPU; }

inline ParseEnvironment *
ParseEnvironment::Parent() const
    { return pParent; }

inline const SubPeUseIfc *
ParseEnvironment::CurSubPeUse() const
    { return pCurSubPe; }



#endif

