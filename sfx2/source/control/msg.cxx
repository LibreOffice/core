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
    rtl::OString aCmd(GetCommand());
    return rtl::OUString( aCmd, aCmd.getLength(), RTL_TEXTENCODING_UTF8 );
}

