/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: c_slots.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:32:49 $
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
#include <ary/cpp/cpp_disp.hxx>



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
    ary::cpp::Display * pDisp
            = dynamic_cast< ary::cpp::Display* >( &o_rDestination );
    if ( pDisp == 0 )
        return;

    for ( Map_NamespacePtr::const_iterator it = pData->begin();
          it != pData->end();
          ++it )
    {
        pDisp->Display_Namespace( *(*it).second );
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
    ary::cpp::Display * pDisp
            = dynamic_cast< ary::cpp::Display* >( &o_rDestination );
    if ( pDisp == 0 )
        return;

    for ( List_Bases::const_iterator it = pData->begin();
          it != pData->end();
          ++it )
    {
        pDisp->DisplaySlot_BaseClass( *it );
    }
}



}   // namespace cpp
}   // namespace ary


