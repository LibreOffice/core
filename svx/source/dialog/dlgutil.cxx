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
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------

#include "svx/dlgutil.hxx"
#include <svl/itemset.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/module.hxx>
#include <svl/intitem.hxx>

// -----------------------------------------------------------------------

FieldUnit GetModuleFieldUnit( const SfxItemSet& rSet )
{
    FieldUnit eUnit = FUNIT_INCH;
    const SfxPoolItem* pItem = NULL;
    if ( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_METRIC, sal_False, &pItem ) )
        eUnit = (FieldUnit)( (const SfxUInt16Item*)pItem )->GetValue();
    else
    {
        return SfxModule::GetCurrentFieldUnit();
    }

    return eUnit;
}

// -----------------------------------------------------------------------

FieldUnit GetModuleFieldUnit()
{
    return SfxModule::GetCurrentFieldUnit();
}

// -----------------------------------------------------------------------
// eof
