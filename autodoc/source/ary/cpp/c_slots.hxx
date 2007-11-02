/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: c_slots.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:27:28 $
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

#ifndef ARY_CPP_C_SLOTS_HXX
#define ARY_CPP_C_SLOTS_HXX

// BASE CLASSES
#include <ary/ceslot.hxx>
// USED SERVICES
#include <ary/cpp/c_slntry.hxx>




namespace ary
{
namespace cpp
{


class Slot_SubNamespaces : public ary::Slot
{
  public:
                        Slot_SubNamespaces(
                            const Map_NamespacePtr &
                                                i_rData );
    virtual             ~Slot_SubNamespaces();

    virtual uintt       Size() const;

  private:
    virtual void        StoreEntries(
                            ary::Display &      o_rDestination ) const;
    // DATA
    const Map_NamespacePtr *
                        pData;
};

class Slot_BaseClass : public ary::Slot
{
  public:
                        Slot_BaseClass(
                            const List_Bases &  i_rData );
    virtual             ~Slot_BaseClass();

    virtual uintt       Size() const;

  private:
    virtual void        StoreEntries(
                            ary::Display &      o_rDestination ) const;
    // DATA
    const List_Bases *  pData;
};




}   // namespace cpp
}   // namespace ary
#endif
