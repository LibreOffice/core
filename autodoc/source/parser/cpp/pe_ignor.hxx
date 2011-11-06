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




#ifndef ADC_CPP_PE_IGNOR_HXX
#define ADC_CPP_PE_IGNOR_HXX



// USED SERVICES
    // BASE CLASSES
#include <cpp_pe.hxx>
    // COMPONENTS
    // PARAMETERS


namespace cpp {


class PE_Ignore : public Cpp_PE
{
  public:
                        PE_Ignore(
                            Cpp_PE *            i_pParent );
                        ~PE_Ignore();

    virtual void        Call_Handler(
                            const cpp::Token &  i_rTok );
  private:
    void                Setup_StatusFunctions();
    virtual void        InitData();
    virtual void        TransferData();

    // DATA
    uintt               nBracketCounter;
    bool                bBlockOpened;
};



// IMPLEMENTATION


}   // namespace cpp




#endif


