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



#ifndef ARY_CPP_CP_DEF_HXX
#define ARY_CPP_CP_DEF_HXX


// USED SERVICES
    // BASE CLASSES
    // OTHER
#include <ary/cpp/c_types4cpp.hxx>

namespace ary
{
namespace cpp
{
    class Define;
    class InputContext;
    class Macro;
}
}




namespace ary
{
namespace cpp
{



/** Access to C++ defines and macros in the repository.
*/
class DefPilot
{
  public:
    typedef DefsList::const_iterator                DefsIterator;



    // LIFECYCLE
    virtual             ~DefPilot() {}


    // OPERATIONS
    virtual Define &    Store_Define(
                            const InputContext& i_rContext,
                            const String  &     i_sName,
                            const StringVector &
                                                i_rDefinition ) = 0;
    virtual Macro &     Store_Macro(
                            const InputContext& i_rContext,
                            const String  &     i_sName,
                            const StringVector &
                                                i_rParams,
                            const StringVector &
                                                i_rDefinition ) = 0;
    // INQUIRY
    virtual const DefineEntity &
                        Find_Def(
                            De_id               i_id ) const = 0;
    virtual DefsResultList
                        AllDefines() const = 0;
    virtual DefsResultList
                        AllMacros() const = 0;
};





}   // namespace cpp
}   // namespace ary
#endif
