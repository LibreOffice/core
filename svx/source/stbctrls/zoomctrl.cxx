/*************************************************************************
 *
 *  $RCSfile: zoomctrl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:24 $
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
#pragma hdrstop

#include "dialogs.hrc"

#include "zoomctrl.hxx"
#include "zoom.hxx"
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

SvxZoomStatusBarControl::SvxZoomStatusBarControl( USHORT nId,
                                                  StatusBar& rStb,
                                                  SfxBindings& rBind ) :

    SfxStatusBarControl( nId, rStb, rBind ),

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

        if ( aPop.Execute( &rStatusbar, rStatusbar.OutputToScreenPixel( rCEvt.GetMousePosPixel() ) ) &&
             ( nZoom != aPop.GetZoom() || !nZoom ) )
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

            GetBindings().GetDispatcher()->Execute( GetId(), SFX_CALLMODE_RECORD, &aZoom, 0L );
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


