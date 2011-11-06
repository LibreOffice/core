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



#ifndef ARY_CPP_CA_TYPE_HXX
#define ARY_CPP_CA_TYPE_HXX

// USED SERVICES
    // BASE CLASSES
#include <ary/cpp/cp_type.hxx>
    // OTHER
#include "cs_type.hxx"



namespace ary
{
namespace cpp
{
    class RepositoryPartition;
}
}





namespace ary
{
namespace cpp
{



/** Administrates all C++ types as uses in user declarations
    as return-, parameter- or variable-types.
*/
class TypeAdmin : public TypePilot
{
  public:
    // LIFECYCLE
                        TypeAdmin(
                            RepositoryPartition &
                                                io_myReposyPartition );
    virtual             ~TypeAdmin();

    // INQUIRY
    /// @return A list of all stored types that are not C++ or STL builtin types.
    const Type_Storage &
                        Storage() const;

    // ACCESS
    Type_Storage &      Storage();

    // INHERITED
    // Interface TypePilot:
    virtual const Type &
                        CheckIn_UsedType(
                            const InputContext &
                                                i_context,
                            DYN UsedType &      pass_type );
    virtual const Type &
                        Find_Type(
                            Type_id             i_type ) const;
    virtual bool        Get_TypeText(
                            StreamStr &         o_result,
                            Type_id             i_type ) const;
    virtual bool        Get_TypeText(
                            StreamStr &         o_preName,              ///  ::ary::cpp::
                            StreamStr &         o_name,                 ///  MyClass
                            StreamStr &         o_postName,             ///  <TplArgument> * const &
                            Type_id             i_type ) const;
    virtual Type_id     Tid_Ellipse() const;

  private:
    // DATA
    Type_Storage        aStorage;
    RepositoryPartition *
                        pCppRepositoryPartition;
};




// IMPLEMENTATION
inline const Type_Storage &
TypeAdmin::Storage() const
{
    return aStorage;
}

inline Type_Storage &
TypeAdmin::Storage()
{
    return aStorage;
}





}   // namespace cpp
}   // namespace ary
#endif
