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
