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
