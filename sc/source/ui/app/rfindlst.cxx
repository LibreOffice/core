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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



#include "rfindlst.hxx"

// STATIC DATA -----------------------------------------------------------

#define SC_RANGECOLORS  8

static ColorData aColNames[SC_RANGECOLORS] =
    { COL_LIGHTBLUE, COL_LIGHTRED, COL_LIGHTMAGENTA, COL_GREEN,
        COL_BLUE, COL_RED, COL_MAGENTA, COL_BROWN };

//==================================================================

ScRangeFindList::ScRangeFindList(const String& rName) :
    aDocName( rName ),
    bHidden( sal_False )
{
}

ScRangeFindList::~ScRangeFindList()
{
    void* pEntry = aEntries.First();
    while ( pEntry )
    {
        delete (ScRangeFindData*) aEntries.Remove( pEntry );
        pEntry = aEntries.Next();
    }
}

ColorData ScRangeFindList::GetColorName( sal_uInt16 nIndex )        // static
{
    return aColNames[nIndex % SC_RANGECOLORS];
}

//==================================================================

