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

#ifndef ARY_STORE_S_STORAGE_HXX
#define ARY_STORE_S_STORAGE_HXX

// USED SERVICES
#include <ary/types.hxx>
#include "s_iterator.hxx"




namespace ary
{
namespace stg
{


/** The storage unit of one class of commomly stored repository
    entities.
*/
template <class ENTITY>
class Storage
{
  public:
    typedef Base<ENTITY>                        container_type;
    typedef ary::TypedId<ENTITY>                key_type;
    typedef stg::const_iterator<ENTITY>         c_iter;
    typedef stg::iterator<ENTITY>               iter;

    // LIFECYCLE
    virtual             ~Storage() {}

    // OPERATORS
    const ENTITY &      operator[](
                            key_type            i_id ) const;
    ENTITY &            operator[](
                            key_type            i_id );
    const ENTITY &      operator[](
                            Rid                 i_index ) const;
    ENTITY &            operator[](
                            Rid                 i_index );
    // OPERATIONS
    /// Sets the id of the new entity.
    key_type            Store_Entity(
                            DYN ENTITY &        pass_newEntity );
    /// Sets the id of the new entity.
    void                Set_Reserved(
                            uintt               i_index,
                            DYN ENTITY &        pass_newEntity );
    /// Sets the id of the new entity.
    void                Replace_Entity(
                            key_type            i_index,
                            DYN ENTITY &        pass_newEntity );
    // INQUIRY
    bool                Exists(
                            key_type            i_id ) const;
    bool                Exists(
                            Rid                 i_index ) const;

    c_iter              Begin() const;
    c_iter              BeginUnreserved() const;
    c_iter              End() const;

    // ACCESS
    iter                Begin();
    iter                BeginUnreserved();
    iter                End();

  protected:
                        Storage(
                            uintt               i_nrOfReservedItems );
  private:
    // DATA
    container_type      aData;
};






// IMPLEMENTATION

// Used later, so implemented first.
template <class ENTITY>
inline bool
Storage<ENTITY>::Exists(Rid i_index) const
{
    return 0 < i_index AND i_index < aData.Size();
}

template <class ENTITY>
inline bool
Storage<ENTITY>::Exists(key_type i_id) const
{
    return Exists(i_id.Value());
}

template <class ENTITY>
inline const ENTITY &
Storage<ENTITY>::operator[](Rid i_index) const
{
    csv_assert(Exists(i_index));
    return * aData[i_index];
}

template <class ENTITY>
inline ENTITY &
Storage<ENTITY>::operator[](Rid i_index)
{
    csv_assert(Exists(i_index));
    return * aData[i_index];
}

template <class ENTITY>
inline const ENTITY &
Storage<ENTITY>::operator[](key_type i_id) const
{
    return operator[](i_id.Value());
}

template <class ENTITY>
inline ENTITY &
Storage<ENTITY>::operator[](key_type i_id)
{
    return operator[](i_id.Value());
}

template <class ENTITY>
typename Storage<ENTITY>::key_type
Storage<ENTITY>::Store_Entity(DYN ENTITY & pass_newEntity)
{
    csv_assert( aData.Size() >= aData.ReservedSize() );
    Rid
        ret( aData.Add_Entity(pass_newEntity) );
    pass_newEntity.Set_Id(ret);
    return key_type(ret);
}

template <class ENTITY>
void
Storage<ENTITY>::Set_Reserved(uintt           i_index,
                              DYN ENTITY &    pass_newEntity)
{
    // 0 must not be used.
    csv_assert( i_index != 0 );
    // Make sure, i_index actually is the id of a reserved item.
    csv_assert( i_index < aData.ReservedSize() );

    // If there was a previous entity, it will be deleted by
    // the destructor of pOldEntity.
    Dyn<ENTITY>
        pOldEntity(aData.Set_Entity(i_index, pass_newEntity));
    pass_newEntity.Set_Id(i_index);
}

template <class ENTITY>
void
Storage<ENTITY>::Replace_Entity( key_type       i_index,
                                 DYN ENTITY &   pass_newEntity )
{
    uintt
        nIndex = i_index.Value();
    // Make sure, i_index actually is the id of an existing,
    // non reserved entity.
    csv_assert( csv::in_range(aData.ReservedSize(), nIndex, aData.Size()) );

    // If there was a previous entity, it will be deleted by
    // the destructor of pOldEntity.
    Dyn<ENTITY>
        pOldEntity(aData.Set_Entity(nIndex, pass_newEntity));
    pass_newEntity.Set_Id(nIndex);
}

template <class ENTITY>
inline
typename Storage<ENTITY>::c_iter
Storage<ENTITY>::Begin() const
{
    return c_iter(aData.Begin());
}

template <class ENTITY>
inline
typename Storage<ENTITY>::c_iter
Storage<ENTITY>::BeginUnreserved() const
{
    return c_iter(aData.BeginUnreserved());
}

template <class ENTITY>
inline
typename Storage<ENTITY>::c_iter
Storage<ENTITY>::End() const
{
    return c_iter(aData.End());
}

template <class ENTITY>
inline
typename Storage<ENTITY>::iter
Storage<ENTITY>::Begin()
{
    return iter(aData.Begin());
}

template <class ENTITY>
inline
typename Storage<ENTITY>::iter
Storage<ENTITY>::BeginUnreserved()
{
    return iter(aData.BeginUnreserved());
}

template <class ENTITY>
inline
typename Storage<ENTITY>::iter
Storage<ENTITY>::End()
{
    return iter(aData.End());
}

template <class ENTITY>
inline
Storage<ENTITY>::Storage(uintt i_nrOfReservedItems)
    :   aData(i_nrOfReservedItems)
{
    // Make sure Rid and uintt are the same type, because
    // the interface of this uses Rid, but the interface of
    // container_type uses uintt.
    csv_assert( sizeof(uintt) == sizeof(Rid) );
}




// HELPER FUNCTIONS

/** @return 0, if data are not there.
*/
template <class ENTITY>
inline const ENTITY *
Search( const Storage<ENTITY> &     i_storage,
        Rid                         i_id )
{
    if (NOT i_storage.Exists(i_id))
        return 0;
    return &i_storage[i_id];
}

/** @return 0, if data are not there.
*/
template <class ENTITY>
inline ENTITY *
SearchAccess( const Storage<ENTITY> &   i_storage,
              Rid                       i_id )
{
    if (NOT i_storage.Exists(i_id))
        return 0;
    return &i_storage[i_id];
}




}   // namespace stg
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
