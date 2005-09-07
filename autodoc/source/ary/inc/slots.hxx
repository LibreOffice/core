/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: slots.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:01:05 $
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
#include <ary/opertype.hxx>
    // PARAMETERS
#include <ary/idlists.hxx>



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

class Slot_LidSet : public Slot
{
  public:
                        Slot_LidSet(
                            const Set_Lid &     i_rData );
    virtual             ~Slot_LidSet();
    virtual uintt       Size() const;

  private:
    virtual void        StoreEntries(
                            Display &           o_rDestination ) const;
    // DATA
    const Set_Lid *     pData;
};

class Slot_MapLocalCe : public Slot
{
  public:
                        Slot_MapLocalCe(
                            const Map_LocalCe & i_rData );
    virtual             ~Slot_MapLocalCe();
    virtual uintt       Size() const;

  private:
    virtual void        StoreEntries(
                            Display &           o_rDestination ) const;
    // DATA
    const Map_LocalCe * pData;
};

class Slot_OperationSet : public Slot
{
  public:
                        Slot_OperationSet(
                            const Set_LocalOperation &
                                                i_rData );
    virtual             ~Slot_OperationSet();

    virtual uintt       Size() const;

  private:
    virtual void        StoreEntries(
                            Display &           o_rDestination ) const;
    // DATA
    const Set_LocalOperation *
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
                            const List_LocalCe& i_rData );
    virtual             ~Slot_ListLocalCe();

    virtual uintt       Size() const;

  private:
    virtual void        StoreEntries(
                            Display &           o_rDestination ) const;
    // DATA
    const List_LocalCe* pData;
};

class Slot_OperationList : public Slot
{
  public:
                        Slot_OperationList(
                            const List_LocalOperation &
                                                i_rData );
    virtual             ~Slot_OperationList();

    virtual uintt       Size() const;

  private:
    virtual void        StoreEntries(
                            Display &           o_rDestination ) const;
    // DATA
    const List_LocalOperation *
                        pData;
};



}   // namespace ary


#endif



