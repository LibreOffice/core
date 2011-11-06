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



#ifndef ADC_CPP_CTOKDEAL_HXX
#define ADC_CPP_CTOKDEAL_HXX



// USED SERVICES
    // BASE CLASSES
#include <tokens/tokdeal.hxx>
    // COMPONENTS
    // PARAMETERS


namespace cpp
{

class Token;
class Tok_UnblockMacro;


class TokenDealer : virtual public ::TokenDealer
{
  public:

    virtual void        Deal_CppCode(
                            cpp::Token &        let_drToken ) = 0;

    /** This is to be used only by the internal macro expander
        ( ::cpp::PreProcessor ).
        These tokens are inserted into the source text temporary to make clear,
        where a specific macro replacement ends and therefore the macro's name
        becomes valid again.

        @see ::cpp::Tok_UnblockMacro
        @see ::cpp::PreProcessor
    */
    virtual void        Deal_Cpp_UnblockMacro(
                            Tok_UnblockMacro &  let_drToken ) = 0;
};



} // namespace cpp



#endif

