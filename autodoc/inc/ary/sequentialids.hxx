/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sequentialids.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 14:43:11 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
