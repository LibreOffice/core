/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: selctrl.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-22 15:56:32 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

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
#include <tools/urlobj.hxx>

#define _SVX_SELCTRL_CXX

#include "selctrl.hxx"
#include <svx/dialmgr.hxx>

#include <svx/dialogs.hrc>

#define PAINT_OFFSET    5

SFX_IMPL_STATUSBAR_CONTROL(SvxSelectionModeControl, SfxUInt16Item);

// class SvxSelectionModeControl -----------------------------------------

SvxSelectionModeControl::SvxSelectionModeControl( USHORT _nSlotId,
                                                  USHORT _nId,
                                                  StatusBar& rStb ) :
    SfxStatusBarControl( _nSlotId, _nId, rStb ),
    nState( 0 )
{
}

// -----------------------------------------------------------------------

void SvxSelectionModeControl::StateChanged( USHORT, SfxItemState eState,
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
    if ( nState > 3 )
        nState = 0;

    ::com::sun::star::uno::Any a;
    SfxUInt16Item aState( GetSlotId(), nState );
    INetURLObject aObj( m_aCommandURL );

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aArgs( 1 );
    aArgs[0].Name  = aObj.GetURLPath();
    aState.QueryValue( a );
    aArgs[0].Value = a;

    execute( aArgs );
}

// -----------------------------------------------------------------------

void SvxSelectionModeControl::Paint( const UserDrawEvent& )
{
    DrawItemText_Impl();
}

// -----------------------------------------------------------------------

void SvxSelectionModeControl::DrawItemText_Impl()
{
    String sTxt;
    USHORT _nId = 0;

    switch ( nState )
    {
        case 0:
            _nId = RID_SVXSTR_SELMODE_STD;
            break;
        case 1:
            _nId = RID_SVXSTR_SELMODE_ER;
            break;
        case 2:
            _nId = RID_SVXSTR_SELMODE_ERG;
            break;
        case 3:
            _nId = RID_SVXSTR_SELMODE_BLK;
            break;
        default: DBG_ERROR( "invalid selection mode!" );
    }

    if ( _nId )
        sTxt = SVX_RESSTR( _nId );
    GetStatusBar().SetItemText( GetId(), sTxt );
}

ULONG SvxSelectionModeControl::GetDefItemWidth(const StatusBar& rStb)
{
    long nWidth1 =  rStb.GetTextWidth(SVX_RESSTR(RID_SVXSTR_SELMODE_STD));
    long nWidth2 =  rStb.GetTextWidth(SVX_RESSTR(RID_SVXSTR_SELMODE_ER));
    long nWidth3 =  rStb.GetTextWidth(SVX_RESSTR(RID_SVXSTR_SELMODE_ERG));
    long nWidth4 =  rStb.GetTextWidth(SVX_RESSTR(RID_SVXSTR_SELMODE_BLK));

    if(nWidth1<nWidth2)
        nWidth1=nWidth2;

    if(nWidth1<nWidth3)
        nWidth1=nWidth3;

    if(nWidth1<nWidth4)
        nWidth1=nWidth4;

    return nWidth1+PAINT_OFFSET;
}


