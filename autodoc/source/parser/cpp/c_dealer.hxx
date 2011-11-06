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



#ifndef ADC_CPP_C_DEALER_HXX
#define ADC_CPP_C_DEALER_HXX



// USED SERVICES
    // BASE CLASSES
#include <cpp/ctokdeal.hxx>
#include <adoc/atokdeal.hxx>
#include <ary/info/docstore.hxx>
    // COMPONENTS
#include "preproc.hxx"
#include "c_rcode.hxx"
#include <adoc/a_rdocu.hxx>
    // PARAMETERS

namespace csv
{
    namespace ploc
    {
        class Path;
        class DirectoryChain;
    }
}


class TokenParser;


namespace cpp
{

class PE_File;
class DefineDescription;


class Distributor : public cpp::TokenDealer,        /// Handle C++ code tokens.
                    public adoc::TokenDealer        /// Handle Autodoc documentation tokens.
{
  public:
    typedef std::map< String, DefineDescription* >     MacroMap;

    // LIFECYCLE
                        Distributor(
                            ary::cpp::Gate &    io_rGate );
                        ~Distributor();
    // OPERATIONS
    void                AssignPartners(
                            CharacterSource &   io_rSourceText,
                            const MacroMap &    i_rValidMacros );
    void                StartNewFile(
                            const csv::ploc::Path &
                                                i_file );
    virtual void        Deal_Eol();
    virtual void        Deal_Eof();

    virtual void        Deal_CppCode(
                            cpp::Token &        let_drToken );
    virtual void        Deal_Cpp_UnblockMacro(
                            Tok_UnblockMacro &  let_drToken );

    virtual void        Deal_AdcDocu(
                            adoc::Token &       let_drToken );
    virtual Distributor *
                        AsDistributor();
  private:
    // DATA
    PreProcessor        aCppPreProcessor;
    CodeExplorer        aCodeExplorer;
    adoc::DocuExplorer  aDocuExplorer;
    ary::cpp::Gate *    pGate;
    FileScope_EventHandler *
                        pFileEventHandler;
    DocuDealer *        pDocuDistributor;
};



}   // namespace cpp
#endif

