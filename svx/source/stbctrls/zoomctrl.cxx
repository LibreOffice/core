/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: zoomctrl.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 05:46:10 $
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

#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif
#ifndef _STATUS_HXX //autogen
#include <vcl/status.hxx>
#endif
#ifndef _MENU_HXX //autogen
#include <vcl/menu.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#include <tools/urlobj.hxx>

#include "dialogs.hrc"

#include "zoomctrl.hxx"
//CHINA001 #include "zoom.hxx"
#include "zoomitem.hxx"
#include "stbctrls.h"
#include "dialmgr.hxx"

SFX_IMPL_STATUSBAR_CONTROL(SvxZoomStatusBarControl,SvxZoomItem);

// class ZoomPopup_Impl --------------------------------------------------

class ZoomPopup_Impl : public PopupMenu
{
public:
    ZoomPopup_Impl( USHORT nZ, USHORT nValueSet );

    USHORT          GetZoom() const { return nZoom; }
    USHORT          GetCurId() const { return nCurId; }

private:
    USHORT          nZoom;
    USHORT          nCurId;

    virtual void    Select();
};

// -----------------------------------------------------------------------

ZoomPopup_Impl::ZoomPopup_Impl( USHORT nZ, USHORT nValueSet )

:   PopupMenu( ResId( RID_SVXMNU_ZOOM, DIALOG_MGR() ) ),

    nZoom( nZ )
{
    static USHORT aTable[] =
    {
        SVX_ZOOM_ENABLE_50,         ZOOM_50,
        SVX_ZOOM_ENABLE_100,        ZOOM_100,
        SVX_ZOOM_ENABLE_150,        ZOOM_150,
        SVX_ZOOM_ENABLE_200,        ZOOM_200,
        SVX_ZOOM_ENABLE_OPTIMAL,    ZOOM_OPTIMAL,
        SVX_ZOOM_ENABLE_WHOLEPAGE,  ZOOM_WHOLE_PAGE,
        SVX_ZOOM_ENABLE_PAGEWIDTH,  ZOOM_PAGE_WIDTH
    };

    for ( USHORT nPos = 0; nPos < sizeof(aTable) / sizeof(USHORT); nPos += 2 )
        if ( ( aTable[nPos] != ( aTable[nPos] & nValueSet ) ) )
            EnableItem( aTable[nPos+1], FALSE );
}

// -----------------------------------------------------------------------

void ZoomPopup_Impl::Select()
{
    nCurId = GetCurItemId();

    switch ( nCurId )
    {
        case ZOOM_200:          nZoom = 200; break;
        case ZOOM_150:          nZoom = 150; break;
        case ZOOM_100:          nZoom = 100; break;
        case ZOOM_75:           nZoom =  75; break;
        case ZOOM_50:           nZoom =  50; break;

        case ZOOM_OPTIMAL:
        case ZOOM_PAGE_WIDTH:
        case ZOOM_WHOLE_PAGE:   nZoom = 0; break;

    }
}

// class SvxZoomStatusBarControl ------------------------------------------

SvxZoomStatusBarControl::SvxZoomStatusBarControl( USHORT nSlotId,
                                                  USHORT nId,
                                                  StatusBar& rStb ) :

    SfxStatusBarControl( nSlotId, nId, rStb ),
    nZoom( 100 ),
    nValueSet( SVX_ZOOM_ENABLE_ALL )
{
}

// -----------------------------------------------------------------------

void SvxZoomStatusBarControl::StateChanged( USHORT nSID, SfxItemState eState,
                                            const SfxPoolItem* pState )
{
    if( SFX_ITEM_AVAILABLE != eState )
    {
        GetStatusBar().SetItemText( GetId(), String() );
        nValueSet = 0;
    }
    else if ( pState->ISA( SfxUInt16Item) )
    {
        const SfxUInt16Item* pItem = (const SfxUInt16Item*)pState;
        nZoom = pItem->GetValue();
        String aStr( String::CreateFromInt32(nZoom) );
        aStr += '%';
        GetStatusBar().SetItemText( GetId(), aStr );

        if ( pState->ISA(SvxZoomItem) )
        {
            nValueSet = ((const SvxZoomItem*)pState)->GetValueSet();
            SvxZoomType eType = ((const SvxZoomItem*)pState)->GetType();

/*!!!
            switch ( eType )
            {
                case SVX_ZOOM_OPTIMAL:
                    GetStatusBar().SetItemText( GetId(), "Opt." );
                    break;
                case SVX_ZOOM_WHOLEPAGE:
                    GetStatusBar().SetItemText( GetId(), "Page" );
                    break;
                case SVX_ZOOM_PAGEWIDTH:
                    GetStatusBar().SetItemText( GetId(), "Width" );
                    break;
            }
*/
        }
        else
        {
            DBG_WARNING( "use SfxZoomItem for SID_ATTR_ZOOM" );
            nValueSet = SVX_ZOOM_ENABLE_ALL;
        }
    }
}

// -----------------------------------------------------------------------

void SvxZoomStatusBarControl::Paint( const UserDrawEvent& rUsrEvt )
{
    String aStr( String::CreateFromInt32( nZoom ));
    aStr += '%';
    GetStatusBar().SetItemText( GetId(), aStr );
}

// -----------------------------------------------------------------------

void SvxZoomStatusBarControl::Command( const CommandEvent& rCEvt )
{
    if ( COMMAND_CONTEXTMENU & rCEvt.GetCommand() && 0 != nValueSet )
    {
        CaptureMouse();
        ZoomPopup_Impl aPop( nZoom, nValueSet );
        StatusBar& rStatusbar = GetStatusBar();

        if ( aPop.Execute( &rStatusbar, rCEvt.GetMousePosPixel() ) && ( nZoom != aPop.GetZoom() || !nZoom ) )
        {
            nZoom = aPop.GetZoom();
            SvxZoomItem aZoom( SVX_ZOOM_PERCENT, nZoom, GetId() );

            USHORT nId = aPop.GetCurId();

            if ( ZOOM_OPTIMAL == nId )
                aZoom.SetType( SVX_ZOOM_OPTIMAL );
            else if ( ZOOM_PAGE_WIDTH == nId )
                aZoom.SetType( SVX_ZOOM_PAGEWIDTH );
            else if ( ZOOM_WHOLE_PAGE == nId )
                aZoom.SetType( SVX_ZOOM_WHOLEPAGE );

            ::com::sun::star::uno::Any a;
            INetURLObject aObj( m_aCommandURL );

            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aArgs( 1 );
            aArgs[0].Name  = aObj.GetURLPath();
            aZoom.QueryValue( a );
            aArgs[0].Value = a;

            execute( aArgs );
        }
        ReleaseMouse();
    }
    else
        SfxStatusBarControl::Command( rCEvt );
}

ULONG SvxZoomStatusBarControl::GetDefItemWidth(const StatusBar& rStb)
{
    long nWidth1 =  rStb.GetTextWidth(String::CreateFromAscii("XXXXX%"));
    return nWidth1;
}


