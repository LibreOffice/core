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

#ifndef ARY_SLOTS_HXX
#define ARY_SLOTS_HXX


// USED SERVICES
    // BASE CLASSES
#include <ary/ceslot.hxx>
    // COMPONENTS
    // PARAMETERS
#include <ary/ary_disp.hxx>
#include <ary/types.hxx>
#include <ary/sequentialids.hxx>
#include <ary/cpp/c_types4cpp.hxx>
#include <ary/cpp/c_slntry.hxx>



namespace ary
{


class Slot_Null : public Slot
{
  public:
    virtual			   	~Slot_Null();

    virtual void        StoreAt(
                            Display &           o_rDestination ) const;
    virtual uintt       Size() const;

  private:
    virtual void        StoreEntries(
                            Display &           o_rDestination ) const;
};

class Slot_MapLocalCe : public Slot
{
  public:
                        Slot_MapLocalCe(
                            const cpp::Map_LocalCe & i_rData );
    virtual			   	~Slot_MapLocalCe();
    virtual uintt       Size() const;

  private:
    virtual void        StoreEntries(
                            Display &           o_rDestination ) const;
    // DATA
    const cpp::Map_LocalCe *
                        pData;
};

class Slot_MapOperations : public Slot
{
  public:
                        Slot_MapOperations(
                            const std::multimap<String, cpp::Ce_id> &
                                                i_rData );
    virtual			   	~Slot_MapOperations();
    virtual uintt       Size() const;

  private:
    virtual void        StoreEntries(
                            Display &           o_rDestination ) const;
    // DATA
    const std::multimap<String, cpp::Ce_id> *
                        pData;
};

class Slot_ListLocalCe : public Slot
{
  public:
                        Slot_ListLocalCe(
                            const cpp::List_LocalCe &
                                                i_rData );
    virtual			   	~Slot_ListLocalCe();

    virtual uintt       Size() const;

  private:
    virtual void        StoreEntries(
                            Display &           o_rDestination ) const;
    // DATA
    const cpp::List_LocalCe *
                        pData;
};

template <class ID>
class Slot_SequentialIds : public Slot
{
  public:
                        Slot_SequentialIds(
                            const SequentialIds<ID> &
                                                i_rData )
                            :   pData(&i_rData) {}
    virtual			   	~Slot_SequentialIds();

    virtual uintt       Size() const;

  private:
    virtual void        StoreEntries(
                            Display &           o_rDestination ) const;
    // DATA
    const SequentialIds<ID> *
                        pData;
};


template <class ID>
Slot_SequentialIds<ID>::~Slot_SequentialIds()
{
}

template <class ID>
uintt
Slot_SequentialIds<ID>::Size() const
{
     return pData->Size();
}

template <class ID>
void
Slot_SequentialIds<ID>::StoreEntries( Display     & o_rDestination ) const
{
    for ( typename SequentialIds<ID>::const_iterator it = pData->Begin();
          it != pData->End();
          ++it )
    {
        o_rDestination.DisplaySlot_Rid( (*it).Value() );
    }
}




}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
