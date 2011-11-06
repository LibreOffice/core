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



#ifndef ADC_CPP_C_RCODE_HXX
#define ADC_CPP_C_RCODE_HXX

// BASE CLASSES
#include <tokens/tokproct.hxx>
// USED SERVICES
#include <cosv/ploc.hxx>
#include "cxt2ary.hxx"
#include <ary/cpp/c_types4cpp.hxx>
#include <ary/loc/loc_types4loc.hxx>



namespace ary
{
namespace cpp
{
    class Gate;
}
namespace doc
{
    class OldCppDocu;
}
}

namespace cpp
{
    class PE_File;
    class Token;
    class Cpp_PE;




class CodeExplorer : private TokenProcessing_Types

{
  public:
                        CodeExplorer(
                            ary::cpp::Gate &    io_rAryGate );
                        ~CodeExplorer();

    void                StartNewFile();
    void                Process_Token(
                            DYN cpp::Token &    let_drToken );
    // ACCESS
    FileScope_EventHandler &
                        FileEventHandler()      { return aGlobalParseContext; }
    DocuDealer &        DocuDistributor()       { return aGlobalParseContext; }

  private:
    typedef std::vector< cpp::Cpp_PE* >      EnvironmentStack;

    void                AcknowledgeResult();
    const Token &       CurToken() const;
    Cpp_PE &            CurEnv() const;
    Cpp_PE &            PushEnv() const;
    TokenProcessing_Result &
                        CurResult()             { return aGlobalParseContext.CurResult(); }

    // DATA
    ContextForAry       aGlobalParseContext;

    EnvironmentStack    aEnvironments;
    Dyn<PE_File>        pPE_File;

    ary::cpp::Gate *    pGate;
    cpp::Token *        dpCurToken;
};



}   // namespace cpp


#endif

