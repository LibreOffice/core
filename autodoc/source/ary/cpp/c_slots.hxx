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



#ifndef ARY_CPP_C_SLOTS_HXX
#define ARY_CPP_C_SLOTS_HXX

// BASE CLASSES
#include <ary/ceslot.hxx>
// USED SERVICES
#include <ary/cpp/c_slntry.hxx>




namespace ary
{
namespace cpp
{


class Slot_SubNamespaces : public ary::Slot
{
  public:
                        Slot_SubNamespaces(
                            const Map_NamespacePtr &
                                                i_rData );
    virtual             ~Slot_SubNamespaces();

    virtual uintt       Size() const;

  private:
    virtual void        StoreEntries(
                            ary::Display &      o_rDestination ) const;
    // DATA
    const Map_NamespacePtr *
                        pData;
};

class Slot_BaseClass : public ary::Slot
{
  public:
                        Slot_BaseClass(
                            const List_Bases &  i_rData );
    virtual             ~Slot_BaseClass();

    virtual uintt       Size() const;

  private:
    virtual void        StoreEntries(
                            ary::Display &      o_rDestination ) const;
    // DATA
    const List_Bases *  pData;
};




}   // namespace cpp
}   // namespace ary
#endif
