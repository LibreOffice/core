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



#ifndef ADC_UNOIDL_HXX
#define ADC_UNOIDL_HXX



// USED SERVICES
    // BASE CLASSES
#include <parser/parser.hxx>
#include <parser/parserinfo.hxx>
    // COMPONENTS
    // PARAMETERS


namespace ary
{
    class Repository;
}
namespace autodoc
{
    class FileCollector_Ifc;
}

namespace autodoc
{


class IdlParser : public ::CodeParser,
                  public ::ParserInfo
{
  public:
                        IdlParser(
                            ary::Repository &       io_rRepository );

    virtual void        Run(
                            const autodoc::FileCollector_Ifc &
                                                i_rFiles );

  private:
    // DATA
    ary::Repository *   pRepository;
};



// IMPLEMENTATION


}   // namespace autodoc


#endif

