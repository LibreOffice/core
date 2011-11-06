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



#ifndef ADC_CMD_RUN_HXX
#define ADC_CMD_RUN_HXX



// USED SERVICES
    // BASE CLASSES
#include <cosv/comdline.hxx>
    // COMPONENTS
    // PARAMETERS

namespace ary
{
     class Repository;
}

namespace autodoc
{
    class FileCollector_Ifc;
    class ParseToolsFactory_Ifc;
    class CodeParser_Ifc;
    class DocumentationParser_Ifc;
    class IdlParser;


namespace command
{
    class Parse;
     class S_ProjectData;
     struct S_LanguageInfo;

namespace run
{

/** Performs an ::autodoc::command::Parse .
*/
class Parser
{
  public:
                        Parser(
                            const Parse &       i_command );
                        ~Parser();

    bool                Perform();

  private:
    // Locals
    CodeParser_Ifc &    Get_CppParser();
    IdlParser &         Get_IdlParser();
    void                Create_CppParser();
    void                Create_IdlParser();
    const ParseToolsFactory_Ifc &
                        ParseToolsFactory();
    uintt               GatherFiles(
                            FileCollector_Ifc & o_rFiles,
                            const S_ProjectData &
                                                i_rProject );
    // DATA
    const Parse &       rCommand;

    Dyn<CodeParser_Ifc> pCppParser;
    Dyn<DocumentationParser_Ifc>
                        pCppDocuInterpreter;
    Dyn<IdlParser>      pIdlParser;
};




// IMPLEMENTATION


}   // namespace run
}   // namespace command
}   // namespace autodoc

#endif
