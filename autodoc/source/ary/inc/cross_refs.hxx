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



#ifndef ARY_CROSS_REFS_HXX
#define ARY_CROSS_REFS_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include "sorted_idset.hxx"


template <class VALUE_LIST, class TYPES>
class CrossReferences
{
  public:
    typedef TYPES::element_type         element;

    /// Checks for double occurrences
    void                Add(
                            VALUE_LIST::index_type
                                                i_nPosition
                            const element &     i_rElem );
    void                Get_List(
                            Dyn_StdConstIterator<element> &
                                                o_rResult ) const;
  private:
    SortedIdSet<TYPES>  aData[VALUE_LIST::max];
};



namespace ary
{

template <class TYPES>
class SortedIdSet
{
  public:
    typedef typename TYPES::element_type   element;
    typedef typename TYPES::sort_type      sorter;
    typedef typename TYPES::find_type      finder;

                        SortedIdSet(
                            const finder &      i_rFinder )
                                                : aSorter(i_rFinder),
                                                  aData(aSorter) {}
                        ~SortedIdSet()          {}

    void                Get_Begin(
                            Dyn_StdConstIterator<element> &
                                                o_rResult )
                                                { o_rResult = new SCI_Set<FINDER>(aData); }
    void                Add(
                            const element &     i_rElement )
                                                { aData.insert(i_rElement); }

  private:
    typedef std::set<element, sorter>       Set;

    // DATA
    sorter              aSorter;
    Set                 aData;
};


}   // namespace ary



#endif

