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



#ifndef ARY_ARYGROUP_HXX
#define ARY_ARYGROUP_HXX
//  KORR_DEPRECATED_3.0


// USED SERVICES
    // BASE CLASSES
    // OTHER
#include <ary/types.hxx>


namespace ary
{
namespace cpp
{
    class CppEntity;
}

class Slot;

namespace group
{
    typedef std::vector< SlotAccessId > SlotList;
}
}




namespace ary
{



class AryGroup
{
  public:
    // LIFECYCLE
    virtual             ~AryGroup() {}

    // INQUIRY
    Gid                 Id_Group() const;
    const cpp::CppEntity &
                        RE_Group() const;
    const group::SlotList &
                        Slots() const;
    DYN Slot *          Create_Slot(
                            SlotAccessId        i_nSlot ) const;


  private:
    virtual Gid         inq_Id_Group() const = 0;
    virtual const cpp::CppEntity &
                        inq_RE_Group() const = 0;
    virtual const group::SlotList &
                        inq_Slots() const = 0;
    virtual DYN Slot *  inq_Create_Slot(
                            SlotAccessId        i_nSlot ) const = 0;
};



// IMPLEMENTATION
inline Gid
AryGroup::Id_Group() const
    { return inq_Id_Group(); }
inline const cpp::CppEntity &
AryGroup::RE_Group() const
    { return inq_RE_Group(); }
inline const group::SlotList &
AryGroup::Slots() const
    { return inq_Slots(); }
inline DYN Slot *
AryGroup::Create_Slot( SlotAccessId i_nSlot ) const
    { return inq_Create_Slot(i_nSlot); }



}   // namespace ary
#endif
