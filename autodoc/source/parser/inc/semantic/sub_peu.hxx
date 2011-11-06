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



#ifndef ADC_CPP_SUB_PEU_HXX
#define ADC_CPP_SUB_PEU_HXX



// USED SERVICES
    // BASE CLASSES
#include <semantic/parseenv.hxx>
#include <tokens/tokproct.hxx>
    // COMPONENTS
    // PARAMETERS
#include <semantic/sub_pe.hxx>



template <class PE, class SUB>
class SubPeUse  : public SubPeUseIfc,
                  private TokenProcessing_Types
{
  public:
    typedef void (PE::*F_INIT)();
    typedef void (PE::*F_RETURN)();

                        SubPeUse(
                            SubPe<PE,SUB> &     i_rSubPeCreator,
                            F_INIT              i_fInit,
                            F_RETURN            i_fReturn );
                        ~SubPeUse();

    void                Push(
                            E_TokenDone         i_eDone );
    virtual void        InitParse() const;
    virtual void        GetResults() const;

    PE &                Parent() const;
    SUB &               Child() const;

  private:
    // DATA
    SubPe<PE,SUB> &     rSubPeCreator;
    F_INIT              fInit;
    F_RETURN            fReturn;
};


// IMPLEMENTATION


template <class PE, class SUB>
SubPeUse<PE,SUB>::SubPeUse( SubPe<PE,SUB> &     i_rSubPeCreator,
                        F_INIT              i_fInit,
                        F_RETURN            i_fReturn )
    :   rSubPeCreator(i_rSubPeCreator),
        fInit(i_fInit),
        fReturn(i_fReturn)
{
}

template <class PE, class SUB>
SubPeUse<PE,SUB>::~SubPeUse()
{
}

template <class PE, class SUB>
void
SubPeUse<PE,SUB>::Push( E_TokenDone i_eDone )
{
    Parent().SetTokenResult( i_eDone, push, &rSubPeCreator.Get() );
    Parent().SetCurSPU(this);
}

template <class PE, class SUB>
void
SubPeUse<PE,SUB>::InitParse() const
{
    if (fInit != 0)
        (Parent().*fInit)();
}

template <class PE, class SUB>
void
SubPeUse<PE,SUB>::GetResults() const
{
    if (fReturn != 0)
        (Parent().*fReturn)();
}

template <class PE, class SUB>
inline PE &
SubPeUse<PE,SUB>::Parent() const
{
     return rSubPeCreator.Parent();
}

template <class PE, class SUB>
inline SUB &
SubPeUse<PE,SUB>::Child() const
{
     return rSubPeCreator.Child();
}


#endif

