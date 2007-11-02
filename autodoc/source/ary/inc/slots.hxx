/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: slots.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:03:13 $
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
    virtual             ~Slot_Null();

    virtual void        StoreAt(
                            Display &           o_rDestination ) const;
    virtual uintt       Size() const;

  private:
    virtual void        StoreEntries(
                            Display &           o_rDestination ) const;
};


class Slot_RidSet : public Slot
{
  public:
                        Slot_RidSet(
                            const Set_Rid &     i_rData );
    virtual             ~Slot_RidSet();

    virtual uintt       Size() const;

  private:
    virtual void        StoreEntries(
                            Display &           o_rDestination ) const;
    // DATA
    const Set_Rid *     pData;
};

class Slot_MapLocalCe : public Slot
{
  public:
                        Slot_MapLocalCe(
                            const cpp::Map_LocalCe & i_rData );
    virtual             ~Slot_MapLocalCe();
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
    virtual             ~Slot_MapOperations();
    virtual uintt       Size() const;

  private:
    virtual void        StoreEntries(
                            Display &           o_rDestination ) const;
    // DATA
    const std::multimap<String, cpp::Ce_id> *
                        pData;
};

class Slot_RidList : public Slot
{
  public:
                        Slot_RidList(
                            const List_Rid &    i_rData );
    virtual             ~Slot_RidList();

    virtual uintt       Size() const;

  private:
    virtual void        StoreEntries(
                            Display &           o_rDestination ) const;
    // DATA
    const List_Rid *    pData;
};

class Slot_ListLocalCe : public Slot
{
  public:
                        Slot_ListLocalCe(
                            const cpp::List_LocalCe &
                                                i_rData );
    virtual             ~Slot_ListLocalCe();

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
    virtual             ~Slot_SequentialIds();

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
