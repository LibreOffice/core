/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef ARY_SORTEDIDS_HXX
#define ARY_SORTEDIDS_HXX


// USED SERVICES
#include <algorithm>
#include <cosv/tpl/range.hxx>




namespace ary
{


/** Implementation of a set of children to an entity in the Autodoc
    repository. The children are sorted.

    @tpl COMPARE
    Needs to provide types:
        entity_base_type
        id_type
        key_type

    and functions:
        static entity_base_type &
                        EntityOf_(
                            id_type             i_id );
        static const key_type &
                        KeyOf_(
                            const entity_type & i_entity );
        static bool     Lesser_(
                            const key_type &    i_1,
                            const key_type &    i_2 );
*/
template<class COMPARE>
class SortedIds
{
  public:
    typedef typename COMPARE::id_type                   element_t;
    typedef typename COMPARE::key_type                  key_t;
    typedef std::vector<element_t>                      data_t;
    typedef typename data_t::const_iterator             const_iterator;
    typedef typename data_t::iterator                   iterator;
    typedef csv::range<const_iterator>                  search_result_t;

    // LIFECYCLE
    explicit            SortedIds(
                            std::size_t         i_reserve = 0 );
                        ~SortedIds();

    // OPERATIONS
    void                Add(
                            element_t           i_elem );
    // INQUIRY
    const_iterator      Begin() const;
    const_iterator      End() const;

    element_t           Search(
                            const key_t &       i_key ) const;
    search_result_t     SearchAll(
                            const key_t &       i_key ) const;
    const_iterator      LowerBound(
                            const key_t &       i_key ) const;

  private:
    typedef typename COMPARE::entity_base_type      entity_t;

    // Locals
    iterator            LowerBound(
                            const key_t &       i_key );

    static const key_t  &
                        KeyOf_(
                            element_t           i_child );
    template <class ITER>
    static ITER         impl_LowerBound_(
                            ITER                i_begin,
                            ITER                i_end,
                            const key_t &       i_key );

    // DATA
    data_t              aData;
};




// IMPLEMENTATION
template<class COMPARE>
inline const typename SortedIds<COMPARE>::key_t &
SortedIds<COMPARE>::KeyOf_(element_t i_child)
{
    return COMPARE::KeyOf_(COMPARE::EntityOf_(i_child));
}

template<class COMPARE>
SortedIds<COMPARE>::SortedIds(std::size_t i_reserve)
    :   aData()
{
    if (i_reserve > 0)
        aData.reserve(i_reserve);
}

template<class COMPARE>
SortedIds<COMPARE>::~SortedIds()
{
}

template<class COMPARE>
void
SortedIds<COMPARE>::Add(element_t i_elem)
{
    aData.insert(   LowerBound( KeyOf_(i_elem) ),
                    i_elem );
}

template<class COMPARE>
inline typename SortedIds<COMPARE>::const_iterator
SortedIds<COMPARE>::Begin() const
{
    return aData.begin();
}

template<class COMPARE>
inline typename SortedIds<COMPARE>::const_iterator
SortedIds<COMPARE>::End() const
{
    return aData.end();
}

template<class COMPARE>
typename SortedIds<COMPARE>::element_t
SortedIds<COMPARE>::Search(const key_t & i_key) const
{
    const_iterator
        ret = LowerBound(i_key);
    return ret != aData.end() AND NOT COMPARE::Lesser_(i_key, KeyOf_(*ret))
            ?   *ret
            :   element_t(0);
}

template<class COMPARE>
typename SortedIds<COMPARE>::search_result_t
SortedIds<COMPARE>::SearchAll(const key_t & i_key) const
{
    const_iterator
        r1 = LowerBound(i_key);
    const_iterator
        r2 = r1;
    while (     r2 != aData.end()
            AND NOT COMPARE::Lesser_(i_key, KeyOf_(*r2)) )
    {
        ++r2;
    }

    return csv::make_range(r1,r2);
}

template<class COMPARE>
inline typename SortedIds<COMPARE>::const_iterator
SortedIds<COMPARE>::LowerBound(const key_t & i_key) const
{
    return impl_LowerBound_( aData.begin(),
                             aData.end(),
                             i_key );
}

template<class COMPARE>
inline typename SortedIds<COMPARE>::iterator
SortedIds<COMPARE>::LowerBound(const key_t & i_key)
{
    return impl_LowerBound_( aData.begin(),
                             aData.end(),
                             i_key );
}

template<class COMPARE>
template <class ITER>
ITER
SortedIds<COMPARE>::impl_LowerBound_( ITER            i_begin,
                                      ITER            i_end,
                                      const key_t &   i_key )
{
    ITER i1 = i_begin;
    ITER i2 = i_end;

    for ( ITER it = i1 + (i2-i1)/2;
          i1 != i2;
          it = i1 + (i2-i1)/2 )
    {
        if ( COMPARE::Lesser_(KeyOf_(*it), i_key) )
        {
            i1 = it;
            ++i1;
        }
        else
        {
            i2 = it;
        }
    }   // end for

    return i1;
}




}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
