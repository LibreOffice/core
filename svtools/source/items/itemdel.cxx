/*************************************************************************
 *
 *  $RCSfile: itemdel.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:59:00 $
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

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _ERRCODE_HXX //autogen
#include <tools/errcode.hxx>
#endif
#include <limits.h>

#include <sbx.hxx>

#include "svtdata.hxx"
#include "svarray.hxx"
#include "args.hxx"
#include "itempool.hxx"
#include "itemdel.hxx"

// STATIC DATA -----------------------------------------------------------

DBG_NAME(SfxItemDesruptor_Impl);

// -----------------------------------------------------------------------

class SfxItemDesruptor_Impl
{
    SfxPoolItem *pItem;
    Link         aLink;

private:
                 DECL_LINK( Delete, void * );
                 SfxItemDesruptor_Impl( const SfxItemDesruptor_Impl& ); // n.i.

public:
                 SfxItemDesruptor_Impl( SfxPoolItem *pItemToDesrupt );
                 ~SfxItemDesruptor_Impl();
};

SV_DECL_PTRARR( SfxItemDesruptorList_Impl, SfxItemDesruptor_Impl*, 4, 4 );

// ------------------------------------------------------------------------
SfxItemDesruptor_Impl::SfxItemDesruptor_Impl( SfxPoolItem *pItemToDesrupt ):
    pItem(pItemToDesrupt),
    aLink( LINK(this, SfxItemDesruptor_Impl, Delete) )
{
    DBG_CTOR(SfxItemDesruptor_Impl, 0);

    pItem->SetRef( SFX_ITEMS_DELETEONIDLE );

    // im Idle abarbeiten
    GetpApp()->InsertIdleHdl( aLink, 1 );

    // und in Liste eintragen (damit geflusht werden kann)
    SfxItemDesruptorList_Impl* &rpList
     = ImpSvtData::GetSvtData().pItemDesruptList;
    if ( !rpList )
        rpList = new SfxItemDesruptorList_Impl;
    const SfxItemDesruptor_Impl *pThis = this;
    rpList->Insert( pThis, rpList->Count() );
}

// ------------------------------------------------------------------------
SfxItemDesruptor_Impl::~SfxItemDesruptor_Impl()
{
    DBG_DTOR(SfxItemDesruptor_Impl, 0);

    // aus Idle-Handler austragen
    GetpApp()->RemoveIdleHdl( aLink );

    // und aus Liste austragen
    SfxItemDesruptorList_Impl* &rpList
     = ImpSvtData::GetSvtData().pItemDesruptList;
    DBG_ASSERT( rpList, "no DesruptorList" );
    const SfxItemDesruptor_Impl *pThis = this;
    if ( rpList ) HACK(warum?)
        rpList->Remove( rpList->GetPos(pThis) );

    // Item l"oschen
    pItem->SetRef( 0 );
    DBG_CHKOBJ( pItem, SfxPoolItem, 0 );
    DBG_ASSERT( 0 == pItem->GetRef(), "desrupting pooled item" );
    delete pItem;
}

// ------------------------------------------------------------------------
IMPL_LINK( SfxItemDesruptor_Impl, Delete, void *, pvoid )
{
    {DBG_CHKTHIS(SfxItemDesruptor_Impl, 0);}
    delete this;
    return 0;
}

// ------------------------------------------------------------------------
SfxPoolItem* DeleteItemOnIdle( SfxPoolItem* pItem )
{
    DBG_ASSERT( 0 == pItem->GetRef(), "deleting item in use" );
    new SfxItemDesruptor_Impl( pItem );
    return pItem;
}

// ------------------------------------------------------------------------
void DeleteOnIdleItems()
{
    SfxItemDesruptorList_Impl* &rpList
     = ImpSvtData::GetSvtData().pItemDesruptList;
    if ( rpList )
    {
        USHORT n;
        while ( 0 != ( n = rpList->Count() ) )
            // Remove ist implizit im Dtor
            delete rpList->GetObject( n-1 );
        DELETEZ(rpList);
    }
}


