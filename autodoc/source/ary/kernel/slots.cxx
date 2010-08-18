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

//***********************       Slot_MapLocalCe     ********************//

Slot_MapLocalCe::Slot_MapLocalCe( const cpp::Map_LocalCe & i_rData )
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
    for ( cpp::Map_LocalCe::const_iterator it = pData->begin();
          it != pData->end();
          ++it )
    {
        o_rDestination.DisplaySlot_LocalCe( (*it).second, (*it).first );
    }
}



//***********************       Slot_MapOperations     ********************//

Slot_MapOperations::Slot_MapOperations( const std::multimap<String, cpp::Ce_id> & i_rData )
    :   pData(&i_rData)
{
}

Slot_MapOperations::~Slot_MapOperations()
{
}

uintt
Slot_MapOperations::Size() const
{
     return pData->size();;
}

void
Slot_MapOperations::StoreEntries( Display & o_rDestination ) const
{
    for ( std::multimap<String, cpp::Ce_id>::const_iterator it = pData->begin();
          it != pData->end();
          ++it )
    {
        o_rDestination.DisplaySlot_LocalCe( (*it).second, (*it).first );
    }
}

//***********************       Slot_ListLocalCe      ********************//

Slot_ListLocalCe::Slot_ListLocalCe( const cpp::List_LocalCe & i_rData )
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
    for ( cpp::List_LocalCe::const_iterator it = pData->begin();
          it != pData->end();
          ++it )
    {
        o_rDestination.DisplaySlot_LocalCe( (*it).nId, (*it).sLocalName );
    }
}


}   // namespace ary
