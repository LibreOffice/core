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



#ifndef ARY_CPP_C_CE_HXX
#define ARY_CPP_C_CE_HXX


// USED SERVICES
    // BASE CLASSES
#include <ary/cpp/c_cppentity.hxx>
    // COMPONENTS
#include <ary/doc/d_docu.hxx>
    // PARAMETERS
#include <ary/cpp/c_types4cpp.hxx>
#include <ary/cpp/c_traits.hxx>
#include <ary/loc/loc_types4loc.hxx>


namespace ary
{
namespace cpp
{

typedef loc::Le_id  Lid;



/** Represents a C++ code entity.
*/
class CodeEntity : public ary::cpp::CppEntity
{
  public:
    typedef Ce_Traits       traits_t;

    // LIFECYCLE
    virtual             ~CodeEntity() {}

    // INQUIRY
    Ce_id               CeId() const            { return Ce_id(Id()); }
    const String  &     LocalName() const;
    Cid                 Owner() const;
    Lid                 Location() const;
    bool                IsVisible() const;

    // ACCESS
    void                Set_InVisible()         { bIsVisible = false; }

  protected:
                        CodeEntity() : bIsVisible(true) {}

  private:
    // Locals
    virtual const String  &
                        inq_LocalName() const = 0;
    virtual Cid         inq_Owner() const = 0;
    virtual Lid         inq_Location() const = 0;

    // DATA
    mutable bool        bIsVisible;
};


// IMPLEMENTATION
inline const String  &
CodeEntity::LocalName() const
    { return inq_LocalName(); }
inline Cid
CodeEntity::Owner() const
    { return inq_Owner(); }
inline Lid
CodeEntity::Location() const
    { return inq_Location(); }




}   // namespace cpp
}   // namespace ary
#endif
