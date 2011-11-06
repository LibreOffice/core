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



#ifndef ARY_SLOTS_HXX
#define ARY_SLOTS_HXX


// USED SERVICES
    // BASE CLASSES
#include <ary/ceslot.hxx>
    // COMPONENTS
    // PARAMETERS
#include <ary/ary_disp.hxx>
#include <ary/types.hxx>
#include <ary/sequentialids.hxx>
#include <ary/cpp/c_types4cpp.hxx>
#include <ary/cpp/c_slntry.hxx>



namespace ary
{


class Slot_Null : public Slot
{
  public:
    virtual             ~Slot_Null();

    virtual void        StoreAt(
                            Display &           o_rDestination ) const;
    virtual uintt       Size() const;

  private:
    virtual void        StoreEntries(
                            Display &           o_rDestination ) const;
};

class Slot_MapLocalCe : public Slot
{
  public:
                        Slot_MapLocalCe(
                            const cpp::Map_LocalCe & i_rData );
    virtual             ~Slot_MapLocalCe();
    virtual uintt       Size() const;

  private:
    virtual void        StoreEntries(
                            Display &           o_rDestination ) const;
    // DATA
    const cpp::Map_LocalCe *
                        pData;
};

class Slot_MapOperations : public Slot
{
  public:
                        Slot_MapOperations(
                            const std::multimap<String, cpp::Ce_id> &
                                                i_rData );
    virtual             ~Slot_MapOperations();
    virtual uintt       Size() const;

  private:
    virtual void        StoreEntries(
                            Display &           o_rDestination ) const;
    // DATA
    const std::multimap<String, cpp::Ce_id> *
                        pData;
};

class Slot_ListLocalCe : public Slot
{
  public:
                        Slot_ListLocalCe(
                            const cpp::List_LocalCe &
                                                i_rData );
    virtual             ~Slot_ListLocalCe();

    virtual uintt       Size() const;

  private:
    virtual void        StoreEntries(
                            Display &           o_rDestination ) const;
    // DATA
    const cpp::List_LocalCe *
                        pData;
};

template <class ID>
class Slot_SequentialIds : public Slot
{
  public:
                        Slot_SequentialIds(
                            const SequentialIds<ID> &
                                                i_rData )
                            :   pData(&i_rData) {}
    virtual             ~Slot_SequentialIds();

    virtual uintt       Size() const;

  private:
    virtual void        StoreEntries(
                            Display &           o_rDestination ) const;
    // DATA
    const SequentialIds<ID> *
                        pData;
};


template <class ID>
Slot_SequentialIds<ID>::~Slot_SequentialIds()
{
}

template <class ID>
uintt
Slot_SequentialIds<ID>::Size() const
{
     return pData->Size();
}

template <class ID>
void
Slot_SequentialIds<ID>::StoreEntries( Display     & o_rDestination ) const
{
    for ( typename SequentialIds<ID>::const_iterator it = pData->Begin();
          it != pData->End();
          ++it )
    {
        o_rDestination.DisplaySlot_Rid( (*it).Value() );
    }
}




}   // namespace ary
#endif
