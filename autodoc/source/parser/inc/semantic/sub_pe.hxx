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



#ifndef ADC_CPP_SUB_PE_HXX
#define ADC_CPP_SUB_PE_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS


class ParseEnvironment;

template <class PE, class SUB>
class SubPe
{
  public:
    typedef SubPe< PE, SUB >    self;

                        SubPe(
                            PE &                i_rParent );
    PE &                Parent() const;
    SUB &               Child() const;

    ParseEnvironment &  Get() const;

  private:
    SUB &               CreateChild() const;

    PE &                rParent;
    Dyn<SUB>            pChild;
};



// IMPLEMENTATION


//  SubPe

template <class PE, class SUB>
SubPe<PE,SUB>::SubPe( PE & i_rParent )
    :   rParent(i_rParent)
{
}

template <class PE, class SUB>
PE &
SubPe<PE,SUB>::Parent() const
{
     return rParent;
}

template <class PE, class SUB>
inline SUB &
SubPe<PE,SUB>::Child() const
{
    return pChild ? *pChild.MutablePtr() : CreateChild();
}

template <class PE, class SUB>
ParseEnvironment &
SubPe<PE,SUB>::Get() const
{
    return Child();
}

template <class PE, class SUB>
SUB &
SubPe<PE,SUB>::CreateChild() const
{
    self * pThis = const_cast< self* >(this);

    SUB * pNewChild = new SUB( &rParent);

    pThis->pChild = pNewChild;

    return *pChild.MutablePtr();
}




#endif

