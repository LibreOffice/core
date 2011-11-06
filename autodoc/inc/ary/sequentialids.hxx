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



#ifndef ARY_SEQUENTIALIDS_HXX
#define ARY_SEQUENTIALIDS_HXX


// USED SERVICES
    // BASE CLASSES
    // OTHER
#include <algorithm>



namespace ary
{


/** Implementation of a set of children to an entity in the Autodoc
    repository. The children are in the sequence of addition.
*/
template<class ID>
class SequentialIds
{
  public:
    typedef std::vector<ID>                     data_t;
    typedef typename data_t::const_iterator     const_iterator;

    // LIFECYCLE
    explicit            SequentialIds(
                            std::size_t         i_reserve = 0 );
                        ~SequentialIds();

    // OPERATIONS
    void                Add(
                            const ID &       i_child );
    // INQUIRY
    const_iterator      Begin() const;
    const_iterator      End() const;
    std::size_t         Size() const;

    template <class IDENTIFY>
    ID                  Find(
                            IDENTIFY            i_find ) const;
    template <class IDENTIFY>
    // Workaround for Solaris8 compiler: return type has to match alphabetically
    typename std::vector<ID>::const_iterator
                        Search(
                            IDENTIFY            i_find ) const;
  private:
    // DATA
    data_t              aData;
};







// IMPLEMENTATION

template <class ID>
SequentialIds<ID>::SequentialIds(std::size_t i_reserve)
    :   aData()
{
    if (i_reserve > 0)
        aData.reserve(i_reserve);
}

template <class ID>
SequentialIds<ID>::~SequentialIds()
{
}

template <class ID>
inline void
SequentialIds<ID>::Add(const ID & i_child)
{
    aData.push_back(i_child);
}

template <class ID>
inline typename SequentialIds<ID>::const_iterator
SequentialIds<ID>::Begin() const
{
    return aData.begin();
}

template <class ID>
inline typename SequentialIds<ID>::const_iterator
SequentialIds<ID>::End() const
{
    return aData.end();
}

template <class ID>
inline std::size_t
SequentialIds<ID>::Size() const
{
    return aData.size();
}

template <class ID>
template <class IDENTIFY>
ID
SequentialIds<ID>::Find(IDENTIFY i_find) const
{
    const_iterator
        ret = std::find_if(aData.begin(), aData.end(), i_find);
    csv_assert(ret != aData.end());
    return *ret;
}

template <class ID>
template <class IDENTIFY>
// Workaround for Solaris8 compiler: return type has to match alphabetically
// typename SequentialIds<ID>::const_iterator
typename std::vector<ID>::const_iterator
SequentialIds<ID>::Search(IDENTIFY i_find) const
{
    return std::find_if(aData.begin(), aData.end(), i_find);
}




}   // namespace ary
#endif
