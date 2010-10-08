/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
