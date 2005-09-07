/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: msg.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:05:24 $
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

#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef GCC
#pragma hdrstop
#endif

#include "msg.hxx"

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

USHORT SfxSlot::GetWhich( const SfxItemPool &rPool ) const
{
    if ( !nMasterSlotId || nMasterSlotId == USHRT_MAX )
        ((SfxSlot*) this) -> nMasterSlotId = rPool.GetWhich(nSlotId);
    return nMasterSlotId;
}


