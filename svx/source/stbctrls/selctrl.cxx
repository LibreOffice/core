/*************************************************************************
 *
 *  $RCSfile: selctrl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:23 $
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
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#pragma hdrstop

#define _SVX_SELCTRL_CXX

#include "selctrl.hxx"
#include "dialmgr.hxx"

#include "dialogs.hrc"

#define PAINT_OFFSET    5

SFX_IMPL_STATUSBAR_CONTROL(SvxSelectionModeControl, SfxUInt16Item);

// class SvxSelectionModeControl -----------------------------------------

SvxSelectionModeControl::SvxSelectionModeControl( USHORT nId,
                                                  StatusBar& rStb,
                                                  SfxBindings& rBind ) :
    SfxStatusBarControl( nId, rStb, rBind ),

    nState( 0 )
{
}

// -----------------------------------------------------------------------

void SvxSelectionModeControl::StateChanged( USHORT nSID, SfxItemState eState,
                                            const SfxPoolItem* pState )
{
    if ( SFX_ITEM_AVAILABLE != eState )
        GetStatusBar().SetItemText( GetId(), String() );
    else
    {
        DBG_ASSERT( pState->ISA( SfxUInt16Item ), "invalid item type" );
        SfxUInt16Item* pItem = (SfxUInt16Item*)pState;
        nState = pItem->GetValue();
        DrawItemText_Impl();
    }
}

// -----------------------------------------------------------------------

void SvxSelectionModeControl::Click()
{
    if ( !GetStatusBar().GetItemText( GetId() ).Len() )
        return;
    nState++;
    if ( nState > 2 )
        nState = 0;
    SfxUInt16Item aState( GetId(), nState );
    GetBindings().GetDispatcher()->Execute( GetId(), SFX_CALLMODE_RECORD, &aState, 0L );
}

// -----------------------------------------------------------------------

void SvxSelectionModeControl::Paint( const UserDrawEvent& rUsrEvt )
{
    DrawItemText_Impl();
}

// -----------------------------------------------------------------------

void SvxSelectionModeControl::DrawItemText_Impl()
{
    String sTxt;
    USHORT nId = 0;

    switch ( nState )
    {
        case 0:
            nId = RID_SVXSTR_SELMODE_STD;
            break;
        case 1:
            nId = RID_SVXSTR_SELMODE_ER;
            break;
        case 2:
            nId = RID_SVXSTR_SELMODE_ERG;
            break;
        default: DBG_ERROR( "invalid selection mode!" );
    }

    if ( nId )
        sTxt = SVX_RESSTR( nId );
    GetStatusBar().SetItemText( GetId(), sTxt );
}

ULONG SvxSelectionModeControl::GetDefItemWidth(const StatusBar& rStb)
{
    long nWidth1 =  rStb.GetTextWidth(SVX_RESSTR(RID_SVXSTR_SELMODE_STD));
    long nWidth2 =  rStb.GetTextWidth(SVX_RESSTR(RID_SVXSTR_SELMODE_ER));
    long nWidth3 =  rStb.GetTextWidth(SVX_RESSTR(RID_SVXSTR_SELMODE_ERG));

    if(nWidth1<nWidth2)
        nWidth1=nWidth2;

    if(nWidth1<nWidth3)
        nWidth1=nWidth3;

    return nWidth1+PAINT_OFFSET;
}


