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



#ifndef ARY_CPP_C_DE_HXX
#define ARY_CPP_C_DE_HXX



// USED SERVICES
    // BASE CLASSES
#include <ary/cpp/c_cppentity.hxx>
    // OTHER
#include <ary/loc/loc_types4loc.hxx>
#include <ary/cpp/c_traits.hxx>




namespace ary
{
namespace cpp
{


/** Describes a C/C++ #define statement. May be a define or a macro, for which
    two cases the two different constructors are to be used.

    This class is used by cpp::PreProcessor.
*/
class DefineEntity : public ary::cpp::CppEntity
{
  public:
    typedef Def_Traits       traits_t;

    virtual             ~DefineEntity() {}

    // INQUIRY
    De_id               DefId() const           { return De_id(Id()); }
    const String  &     LocalName() const;
    loc::Le_id          Location() const;
    const StringVector &
                        DefinitionText() const;
    // ACCESS
  protected:
                        DefineEntity(
                            const String  &     i_name,
                            loc::Le_id          i_declaringFile );
  private:
    // Locals
    virtual const StringVector &
                        inq_DefinitionText() const = 0;

    // DATA
    String              sName;
    loc::Le_id          nLocation;
};




// IMPLEMENTATION
inline const String  &
DefineEntity::LocalName() const
    { return sName; }

inline loc::Le_id
DefineEntity::Location() const
    { return nLocation; }

inline const StringVector &
DefineEntity::DefinitionText() const
    { return inq_DefinitionText(); }





}   // end namespace cpp
}   // end namespace ary
#endif
