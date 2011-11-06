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



#ifndef ARY_CESLOT_HXX
#define ARY_CESLOT_HXX
//  KORR_DEPRECATED_3.0


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS

namespace ary
{

class Display;

/** Unterscheidungen von Slots

    Slots:
    -   ReadWrite or ReadOnly
    -   ContentType
        -   Groups
        -   MemberLink              'EnumValue from Enum' or 'Class from Namespace'
        -   MemberData              'Parameter from Operation'
        -   SimpleLink
        -   CommentedLink           'Baseclass from Class'
        -   DefaultCommentedLink    'Class from GlobaIndex'
*/

class Slot
{
  public:
    virtual             ~Slot() {}

    virtual void        StoreAt(
                            Display     &       o_rDestination ) const;
    virtual uintt       Size() const = 0;

  private:
    virtual void        StoreEntries(
                            Display     &       o_rDestination ) const = 0;
};

class Slot_AutoPtr
{
  public:
                        Slot_AutoPtr(
                            Slot *              i_pSlot = 0 )
                                                :   pSlot(i_pSlot) {}
                        ~Slot_AutoPtr()         { if (pSlot != 0) delete pSlot; }

    Slot_AutoPtr &      operator=(
                            Slot *              i_pSlot )
                                                { if (pSlot != 0) delete pSlot;
                                                  pSlot = i_pSlot;
                                                  return *this; }
                        operator bool() const   { return pSlot != 0; }

    const Slot &        operator*()             { csv_assert(pSlot != 0);
                                                  return *pSlot; }
    const Slot *        operator->()            { csv_assert(pSlot != 0);
                                                  return pSlot; }

  private:
    // Forbidden functions
                        Slot_AutoPtr(const Slot_AutoPtr &);
    Slot_AutoPtr &      operator=(const Slot_AutoPtr &);

    // DATA
    Slot *              pSlot;
};




}   // namespace ary
#endif
