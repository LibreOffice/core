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



#ifndef ARY_CPP_CP_TYPE_HXX
#define ARY_CPP_CP_TYPE_HXX


// USED SERVICES
    // BASE CLASSES
    // OTHER
#include <ary/cpp/c_types4cpp.hxx>

namespace ary
{
namespace cpp
{
    class InputContext;
    class Type;
    class UsedType;
}
}





namespace ary
{
namespace cpp
{




/** Access to all found C++ types (as they are used in declarations)
    in the repository.
*/
class TypePilot
{
  public:

    // LIFECYCLE
    virtual             ~TypePilot() {}


    // OPERATIONS
    virtual const Type &
                        CheckIn_UsedType(
                            const InputContext &
                                                i_context,
                            DYN UsedType &      pass_type ) = 0;
    // INQUIRY
    virtual const Type &
                        Find_Type(
                            Type_id             i_type ) const = 0;
    virtual bool        Get_TypeText(
                            StreamStr &         o_result,
                            Type_id             i_type ) const = 0;
    virtual bool        Get_TypeText(
                            StreamStr &         o_preName,              ///  ::ary::cpp::
                            StreamStr &         o_name,                 ///  MyClass
                            StreamStr &         o_postName,             ///  <TplArgument> * const &
                            Type_id             i_type ) const = 0;
    virtual Type_id     Tid_Ellipse() const = 0;
};




}   // namespace cpp
}   // namespace ary
#endif
