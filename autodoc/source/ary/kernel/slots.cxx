/*************************************************************************
 *
 *  $RCSfile: slots.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:20 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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



