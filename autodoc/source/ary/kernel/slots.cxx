/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: slots.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:15:23 $
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
#include <slots.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/ary_disp.hxx>



namespace ary
{


//***********************       Slot     ********************//


void
Slot::StoreAt( Display & o_rDestination ) const
{
    o_rDestination.StartSlot();
    StoreEntries(o_rDestination);
    o_rDestination.FinishSlot();
}


//***********************       Slot_Null     ********************//

Slot_Null::~Slot_Null()
{
}

void
Slot_Null::StoreAt( Display     & ) const
{
    // Does nothing
}

uintt
Slot_Null::Size() const
{
     return 0;
}

void
Slot_Null::StoreEntries( Display     & ) const
{
    // Does nothing
}


//***********************       Slot_RidSet     ********************//

Slot_RidSet::Slot_RidSet( const Set_Rid & i_rData )
    :   pData(&i_rData)
{
}

Slot_RidSet::~Slot_RidSet()
{
}

uintt
Slot_RidSet::Size() const
{
     return pData->size();;
}

void
Slot_RidSet::StoreEntries( Display     & o_rDestination ) const
{
    for ( Set_Rid::const_iterator it = pData->begin();
          it != pData->end();
          ++it )
    {
        o_rDestination.DisplaySlot_Rid( *it );
    }
}



//***********************       Slot_LidSet     ********************//

Slot_LidSet::Slot_LidSet( const Set_Lid &             i_rData )
    :   pData(&i_rData)
{
}

Slot_LidSet::~Slot_LidSet()
{
}

uintt
Slot_LidSet::Size() const
{
     return pData->size();;
}

void
Slot_LidSet::StoreEntries( Display     & o_rDestination ) const
{
    for ( Set_Lid::const_iterator it = pData->begin();
          it != pData->end();
          ++it )
    {
        o_rDestination.DisplaySlot_Lid( *it );
    }
}

//***********************       Slot_MapLocalCe     ********************//

Slot_MapLocalCe::Slot_MapLocalCe( const Map_LocalCe & i_rData )
    :   pData(&i_rData)
{
}

Slot_MapLocalCe::~Slot_MapLocalCe()
{
}

uintt
Slot_MapLocalCe::Size() const
{
     return pData->size();;
}

void
Slot_MapLocalCe::StoreEntries( Display & o_rDestination ) const
{
    for ( Map_LocalCe::const_iterator it = pData->begin();
          it != pData->end();
          ++it )
    {
        o_rDestination.DisplaySlot_LocalCe( (*it).second, (*it).first );
    }
}



//***********************       Slot_OperationSet     ********************//

Slot_OperationSet::Slot_OperationSet( const Set_LocalOperation & i_rData )
    :   pData(&i_rData)
{
}

Slot_OperationSet::~Slot_OperationSet()
{
}

uintt
Slot_OperationSet::Size() const
{
     return pData->size();;
}

void
Slot_OperationSet::StoreEntries( Display     & o_rDestination ) const
{
    for ( Set_LocalOperation::const_iterator it = pData->begin();
          it != pData->end();
          ++it )
    {
        o_rDestination.DisplaySlot_Rid( (*it).nId );
    }
}



//***********************       Slot_RidList     ********************//

Slot_RidList::Slot_RidList( const List_Rid & i_rData )
    :   pData(&i_rData)
{
}

Slot_RidList::~Slot_RidList()
{
}

uintt
Slot_RidList::Size() const
{
     return pData->size();;
}

void
Slot_RidList::StoreEntries( Display     & o_rDestination ) const
{
    for ( List_Rid::const_iterator it = pData->begin();
          it != pData->end();
          ++it )
    {
        o_rDestination.DisplaySlot_Rid( *it );
    }
}



//***********************       Slot_ListLocalCe      ********************//

Slot_ListLocalCe::Slot_ListLocalCe( const List_LocalCe & i_rData )
    :   pData(&i_rData)
{
}

Slot_ListLocalCe::~Slot_ListLocalCe()
{
}

uintt
Slot_ListLocalCe::Size() const
{
     return pData->size();;
}

void
Slot_ListLocalCe::StoreEntries( Display     & o_rDestination ) const
{
    for ( List_LocalCe::const_iterator it = pData->begin();
          it != pData->end();
          ++it )
    {
        o_rDestination.DisplaySlot_LocalCe( (*it).nId, (*it).sLocalName );
    }
}



//***********************       Slot_OperationList     ********************//

Slot_OperationList::Slot_OperationList( const List_LocalOperation & i_rData )
    :   pData(&i_rData)
{
}

Slot_OperationList::~Slot_OperationList()
{
}

uintt
Slot_OperationList::Size() const
{
     return pData->size();;
}

void
Slot_OperationList::StoreEntries( Display     & o_rDestination ) const
{
    for ( List_LocalOperation::const_iterator it = pData->begin();
          it != pData->end();
          ++it )
    {
        o_rDestination.DisplaySlot_Rid( (*it).nId );
    }
}




}   // namespace ary



