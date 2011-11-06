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



#ifndef X2C_HEAP_HXX
#define X2C_HEAP_HXX

#include "sistr.hxx"

class HeapItem;

class Heap
{
  public:
                        Heap(
                            unsigned            i_nWidth );
                        ~Heap();

    void                InsertValue(
                            const char *        i_sKey,
                            const char *        i_sValue );
    HeapItem *          ReleaseTop();           /// @return must be deleted by caller of method.

  private:
    typedef HeapItem * Column;

    void                IncColumn();
    Column &            ActiveColumn()          { return dpColumnsArray[nActiveColumn]; }

    Column *            dpColumnsArray;
    unsigned            nColumnsArraySize;
    unsigned            nActiveColumn;
};


class HeapItem
{
  public:
                        HeapItem(
                            const char *        i_sKey,
                            const char *        i_sValue );
                        ~HeapItem(  );

    bool                operator<(
                            const HeapItem &    i_rOther ) const;
    bool                operator<=(
                            const HeapItem &    i_rOther ) const
                                                { return ! (i_rOther < *this); }
    const Simstr &      Value() const;
    const Simstr &      Key() const;
    HeapItem *          Next() const;

    void                SetNext(
                            HeapItem *          i_pNext );
  private:
    Simstr              sValue;
    Simstr              sKey;
    HeapItem *          pNext;
};






#endif


