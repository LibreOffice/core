/*************************************************************************
 *
 *  $RCSfile: insctrl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-09-09 15:40:12 $
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

// include ---------------------------------------------------------------

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _STATUS_HXX //autogen
#include <vcl/status.hxx>
#endif
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#pragma hdrstop

#define _SVX_INSCTRL_CXX

#include "dialogs.hrc"

#include "insctrl.hxx"
#include "dialmgr.hxx"

#define PAINT_OFFSET    5

SFX_IMPL_STATUSBAR_CONTROL(SvxInsertStatusBarControl, SfxBoolItem);

// class SvxInsertStatusBarControl ---------------------------------------

SvxInsertStatusBarControl::SvxInsertStatusBarControl( USHORT nSlotId,
                                                      USHORT nId,
                                                      StatusBar& rStb ) :

    SfxStatusBarControl( nSlotId, nId, rStb ),
    bInsert( TRUE )
{
    rStb.SetHelpId( nId, nSlotId );
}

// -----------------------------------------------------------------------

SvxInsertStatusBarControl::~SvxInsertStatusBarControl()
{
}

// -----------------------------------------------------------------------

void SvxInsertStatusBarControl::StateChanged( USHORT nSID, SfxItemState eState,
                                              const SfxPoolItem* pState )
{
    if ( SFX_ITEM_AVAILABLE != eState )
        GetStatusBar().SetItemText( GetId(), String() );
    else
    {
        DBG_ASSERT( pState->ISA( SfxBoolItem ), "invalid item type" );
        SfxBoolItem* pItem = (SfxBoolItem*)pState;
        bInsert = pItem->GetValue();
        DrawItemText_Impl();
    }
}

// -----------------------------------------------------------------------

void SvxInsertStatusBarControl::Click()
{
    if ( !GetStatusBar().GetItemText( GetId() ).Len() )
        return;
    bInsert = !bInsert;
    SfxBoolItem aIns( GetSlotId(), bInsert );

    ::com::sun::star::uno::Any a;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aArgs( 1 );
    aArgs[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "InsertMode" ));
    aIns.QueryValue( a );
    aArgs[0].Value = a;

    execute( aArgs );
}

// -----------------------------------------------------------------------

void SvxInsertStatusBarControl::Paint( const UserDrawEvent& rUsrEvt )
{
    DrawItemText_Impl();
}

// -----------------------------------------------------------------------

void SvxInsertStatusBarControl::DrawItemText_Impl()
{
    USHORT nId = RID_SVXSTR_OVERWRITE_TEXT;

    if ( bInsert )
        nId = RID_SVXSTR_INSERT_TEXT;
    GetStatusBar().SetItemText( GetId(), SVX_RESSTR( nId ) );
}

ULONG SvxInsertStatusBarControl::GetDefItemWidth(const StatusBar& rStb)
{
    long nWidth1 =  rStb.GetTextWidth(SVX_RESSTR(RID_SVXSTR_OVERWRITE_TEXT));
    long nWidth2 =  rStb.GetTextWidth(SVX_RESSTR(RID_SVXSTR_INSERT_TEXT));

    if(nWidth1<nWidth2)
        nWidth1=nWidth2;

    return nWidth1+PAINT_OFFSET;
}


