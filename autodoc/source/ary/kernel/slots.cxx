/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

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
