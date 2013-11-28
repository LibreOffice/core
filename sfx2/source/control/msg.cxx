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
#include "precompiled_sfx2.hxx"
#include <svl/itempool.hxx>
#include <svl/eitem.hxx>
#include <sfx2/msg.hxx>

//====================================================================

SfxSlotKind SfxSlot::GetKind() const
{
    if( !nMasterSlotId && !nValue)
        return (SfxSlotKind) SFX_KIND_STANDARD;
    if ( nMasterSlotId && fnExec==0 && fnState==0 )
    {
        if ( pType->Type() == TYPE(SfxBoolItem) )
            return (SfxSlotKind) SFX_KIND_ENUM;
        else
        {
            DBG_ERROR( "invalid slot kind detected" );
            return SFX_KIND_ENUM;
        }
    }
    else
        return (SfxSlotKind) SFX_KIND_ATTR;
}

//--------------------------------------------------------------------

sal_uInt16 SfxSlot::GetWhich( const SfxItemPool &rPool ) const
{
    if ( !nMasterSlotId || nMasterSlotId == USHRT_MAX )
        ((SfxSlot*) this) -> nMasterSlotId = rPool.GetWhich(nSlotId);
    return nMasterSlotId;
}

::rtl::OString SfxSlot::GetCommand() const
{
    rtl::OString sRet(".uno:");
    sRet += pUnoName;
    return sRet;
}

::rtl::OUString SfxSlot::GetCommandString() const
{
    return rtl::OStringToOUString( GetCommand(), RTL_TEXTENCODING_UTF8);
}

