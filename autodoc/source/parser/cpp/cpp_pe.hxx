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



#ifndef ADC_CPP_CPP_PE_HXX
#define ADC_CPP_CPP_PE_HXX



// USED SERVICES
    // BASE CLASSES
#include <semantic/parseenv.hxx>
#include "tokintpr.hxx"
    // COMPONENTS
#include "pev.hxx"
    // PARAMETERS
#include <ary/cpp/c_types4cpp.hxx>


namespace cpp {

class Cpp_PE : public ::ParseEnvironment,
               public TokenInterpreter
{
  public:
    typedef cpp::PeEnvironment  EnvData;

    void                SetTokenResult(
                            E_TokenDone         i_eDone,
                            E_EnvStackAction    i_eWhat2DoWithEnvStack,
                            ParseEnvironment *  i_pParseEnv2Push = 0 );

    virtual Cpp_PE *    Handle_ChildFailure();  // Defaulted to 0.

  protected:
                        Cpp_PE(
                            Cpp_PE *            io_pParent );
                        Cpp_PE(
                            EnvData &           i_rEnv );

    EnvData &           Env() const;

    void                StdHandlingOfSyntaxError(
                            const char *        i_sText );

  private:
    // DATA
    EnvData &           rMyEnv;
};

inline  Cpp_PE::EnvData &
Cpp_PE::Env() const
    { return rMyEnv; }

}   // namespace cpp

#endif

