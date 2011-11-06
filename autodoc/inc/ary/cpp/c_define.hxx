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



#ifndef ARY_CPP_C_DEFINE_HXX
#define ARY_CPP_C_DEFINE_HXX


// USED SERVICES
    // BASE CLASSES
#include <ary/cpp/c_de.hxx>
    // OTHER




namespace ary
{
namespace cpp
{


/** A C/C++ #define ("#define DEF") statement, but no macro.

    @see Macro
*/
class Define : public DefineEntity
{
  public:
    enum E_ClassId { class_id = 1601 };

                        Define(      /// Used for: #define DEFINE xyz
                            const String  &     i_name,
                            const StringVector &
                                                i_definition,
                            loc::Le_id          i_declaringFile );
    virtual             ~Define();
  private:
    // Interface csv::ConstProcessorClient
    virtual void        do_Accept(
                            csv::ProcessorIfc & io_processor ) const;

    // Interface ary::Object:
    virtual ClassId     get_AryClass() const;

    // Interface DefineEntity:
    virtual const StringVector &
                        inq_DefinitionText() const;
    // DATA
    StringVector        aDefinition;
};





}   // namespace cpp
}   // namespace ary
#endif
