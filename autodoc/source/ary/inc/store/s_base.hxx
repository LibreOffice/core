/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef ARY_STORE_S_BASE_HXX
#define ARY_STORE_S_BASE_HXX

// USED SERVICES
#include <deque>
#include <cosv/tpl/tpltools.hxx>




namespace ary
{
namespace stg
{


/** The basic storage container of the repository.

    @collab Storage
    Implements Storage. Not used elsewhere.

    @tpl ENTITY
    The type of *it, where it is of type c_iter, has to be ENTITY * const.
*/
template <class ENTITY>
class Base
{
  public:
    // LIFECYCLE
    typedef std::deque< ENTITY* >               impl_type;
    typedef typename impl_type::const_iterator  c_iter;


    /** @param i_nrOfReservedItems
        The number of actual items to reserve, including the item
        at index [0] that is always empty and unused.
    */
                        Base(
                            uintt               i_nrOfReservedItems );
                        ~Base();

    // OPERATORS
    ENTITY *            operator[](
                            uintt               i_index ) const;
    // OPERATIONS
    uintt               Add_Entity(             /// @return the index of the new element.
                            DYN ENTITY &        pass_newEntity );
    DYN ENTITY *        Set_Entity(             /// @return the previous value.
                            uintt               i_index,
                            DYN ENTITY &        pass_newEntity );
    // INQUIRY
    uintt               Size() const;           /// Incl. reserved size.
    uintt               ReservedSize() const;   /// Incl. zero for element at [0].

    c_iter              Begin() const;          /// @return location of index 1, because 0 is always empty.
    c_iter              BeginUnreserved() const;
    c_iter              End() const;

  private:
    // DATA
    impl_type               aData;
    uintt                   nReservedSize;
};



// IMPLEMENTATION

template <class ENTITY>
Base<ENTITY>::Base(uintt i_nrOfReservedItems)
    :   aData(i_nrOfReservedItems, 0),
        nReservedSize(i_nrOfReservedItems)
{
}

template <class ENTITY>
Base<ENTITY>::~Base()
{
    csv::erase_container_of_heap_ptrs(aData);
}


template <class ENTITY>
ENTITY *
Base<ENTITY>::operator[](uintt i_index) const
{
    if (i_index < aData.size())
        return aData[i_index];
    return 0;
}

template <class ENTITY>
uintt
Base<ENTITY>::Add_Entity(DYN ENTITY & pass_newEntity)
{
    aData.push_back(&pass_newEntity);
    return aData.size() - 1;
}

template <class ENTITY>
DYN ENTITY *
Base<ENTITY>::Set_Entity( uintt         i_index,
                          DYN ENTITY &  pass_newEntity )
{
    csv_assert(i_index != 0 AND i_index < aData.size());

    Dyn<ENTITY>
        ret(aData[i_index]);
    aData[i_index] = &pass_newEntity;
    return ret.Release();
}

template <class ENTITY>
uintt
Base<ENTITY>::Size() const
{
    return aData.size();
}

template <class ENTITY>
uintt
Base<ENTITY>::ReservedSize() const
{
    return nReservedSize;
}

template <class ENTITY>
typename Base<ENTITY>::c_iter
Base<ENTITY>::Begin() const
{
    return aData.begin() + 1;
}

template <class ENTITY>
typename Base<ENTITY>::c_iter
Base<ENTITY>::BeginUnreserved() const
{
    return aData.begin() + nReservedSize;
}

template <class ENTITY>
typename Base<ENTITY>::c_iter
Base<ENTITY>::End() const
{
    return aData.end();
}




}   // namespace stg
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
