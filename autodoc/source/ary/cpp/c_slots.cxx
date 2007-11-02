/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: c_slots.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:27:11 $
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

#include <precomp.h>
#include <c_slots.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/ary_disp.hxx>
#include <ary/cpp/c_namesp.hxx>



namespace ary
{
namespace cpp
{



//***********************       Slot_SubNamespaces     ********************//


Slot_SubNamespaces::Slot_SubNamespaces( const Map_NamespacePtr & i_rData )
    :   pData( &i_rData )
{
}

Slot_SubNamespaces::~Slot_SubNamespaces()
{
}

uintt
Slot_SubNamespaces::Size() const
{
     return pData->size();
}

void
Slot_SubNamespaces::StoreEntries( ary::Display &  o_rDestination ) const
{
    for ( Map_NamespacePtr::const_iterator it = pData->begin();
          it != pData->end();
          ++it )
    {
        (*(*it).second).Accept(o_rDestination);
    }
}


//***********************       Slot_BaseClass     ********************//

Slot_BaseClass::Slot_BaseClass( const List_Bases & i_rData )
    :   pData( &i_rData )
{
}

Slot_BaseClass::~Slot_BaseClass()
{
}

uintt
Slot_BaseClass::Size() const
{
     return pData->size();
}

void
Slot_BaseClass::StoreEntries( ary::Display &  o_rDestination ) const
{
    for ( List_Bases::const_iterator it = pData->begin();
          it != pData->end();
          ++it )
    {
        csv::CheckedCall(o_rDestination, *it);
    }
}



}   // namespace cpp
}   // namespace ary
