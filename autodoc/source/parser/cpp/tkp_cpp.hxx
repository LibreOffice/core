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



#ifndef ADC_TKP_CPP_HXX
#define ADC_TKP_CPP_HXX

// USED SERVICES
    // BASE CLASSES
#include <tokens/tkp.hxx>
    // COMPONENTS
    // PARAMETRS

namespace autodoc
{
    class TkpDocuContext;
}

namespace csv
{
    namespace ploc
    {
        class Path;
        class DirectoryChain;
    }
}


namespace cpp {

class Context_CppStd;
class DefineDescription;
class Distributor;


/** This is a TokenParser which is able to parse tokens from
    C++ source code.
*/
class TokenParser_Cpp : public TokenParser
{
  public:
    typedef std::map< String, DefineDescription* >     MacroMap;

    // LIFECYCLE
                        TokenParser_Cpp(
                            DYN autodoc::TkpDocuContext &
                                                let_drDocuContext );
    virtual             ~TokenParser_Cpp();

    // OPERATIONS
    void                AssignPartners(
                            CharacterSource &   io_rCharacterSource,
                            cpp::Distributor &  o_rDealer );
    void                StartNewFile(
                            const csv::ploc::Path &
                                                i_file );
  private:
    virtual void        SetStartContext();
    virtual void        SetCurrentContext(
                            TkpContext &        io_rContext );
    virtual TkpContext &
                        CurrentContext();
    // DATA
    Dyn<Context_CppStd> pBaseContext;
    TkpContext *        pCurContext;
    Distributor *       pDealer;
    CharacterSource *   pCharacterSource;
};


}   // namespace cpp


#endif


