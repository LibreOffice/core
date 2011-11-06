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



#ifndef ARY_LOC_LE_HXX
#define ARY_LOC_LE_HXX

// USED SERVICES
    // BASE CLASSES
#include <ary/entity.hxx>
    // COMPONENTS
    // PARAMETERS
#include <ary/loc/loc_types4loc.hxx>
#include <ary/loc/loc_traits.hxx>



namespace ary
{
namespace loc
{


/** Base class for all file locations in the Autodoc repository.
*/
class LocationEntity : public ::ary::Entity
{
  public:
    typedef Le_Traits       traits_t;

    virtual             ~LocationEntity() {}

    Le_id               LeId() const;
    const String &      LocalName() const;
    Le_id               ParentDirectory() const;

  private:
    virtual const String &
                        inq_LocalName() const = 0;
    virtual Le_id       inq_ParentDirectory() const = 0;
};




// IMPLEMENTATION
inline Le_id
LocationEntity::LeId() const
{
    return TypedId<LocationEntity>(Id());
}

inline const String &
LocationEntity::LocalName() const
{
    return inq_LocalName();
}

inline Le_id
LocationEntity::ParentDirectory() const
{
    return inq_ParentDirectory();
}





} // namespace loc
} // namespace ary
#endif
