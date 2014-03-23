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
#include <tools/shl.hxx>
#ifndef _STATUS_HXX //autogen
#include <vcl/status.hxx>
#endif
#ifndef _MENU_HXX //autogen
#include <vcl/menu.hxx>
#endif
#include <sfx2/dispatch.hxx>
#include <tools/urlobj.hxx>

#include <svx/dialogs.hrc>

#include "svx/zoomctrl.hxx"
//CHINA001 #include "zoom.hxx"
#include <svx/zoomitem.hxx>
#include "stbctrls.h"
#include <svx/dialmgr.hxx>

SFX_IMPL_STATUSBAR_CONTROL(SvxZoomStatusBarControl,SvxZoomItem);

// class ZoomPopup_Impl --------------------------------------------------

class ZoomPopup_Impl : public PopupMenu
{
public:
    ZoomPopup_Impl( sal_uInt16 nZ, sal_uInt16 nValueSet );

    sal_uInt16          GetZoom() const { return nZoom; }
    sal_uInt16          GetCurId() const { return nCurId; }

private:
    sal_uInt16          nZoom;
    sal_uInt16          nCurId;

    virtual void    Select();
};

// -----------------------------------------------------------------------

ZoomPopup_Impl::ZoomPopup_Impl( sal_uInt16 nZ, sal_uInt16 nValueSet )

:   PopupMenu( ResId( RID_SVXMNU_ZOOM, DIALOG_MGR() ) ),

    nZoom( nZ )
{
    static sal_uInt16 aTable[] =
    {
        SVX_ZOOM_ENABLE_50,         ZOOM_50,
        SVX_ZOOM_ENABLE_100,        ZOOM_100,
        SVX_ZOOM_ENABLE_150,        ZOOM_150,
        SVX_ZOOM_ENABLE_200,        ZOOM_200,
        SVX_ZOOM_ENABLE_OPTIMAL,    ZOOM_OPTIMAL,
        SVX_ZOOM_ENABLE_WHOLEPAGE,  ZOOM_WHOLE_PAGE,
        SVX_ZOOM_ENABLE_PAGEWIDTH,  ZOOM_PAGE_WIDTH
    };

    for ( sal_uInt16 nPos = 0; nPos < sizeof(aTable) / sizeof(sal_uInt16); nPos += 2 )
        if ( ( aTable[nPos] != ( aTable[nPos] & nValueSet ) ) )
            EnableItem( aTable[nPos+1], sal_False );
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

SvxZoomStatusBarControl::SvxZoomStatusBarControl( sal_uInt16 _nSlotId,
                                                  sal_uInt16 _nId,
                                                  StatusBar& rStb ) :

    SfxStatusBarControl( _nSlotId, _nId, rStb ),
    nZoom( 100 ),
    nValueSet( SVX_ZOOM_ENABLE_ALL )
{
}

// -----------------------------------------------------------------------

void SvxZoomStatusBarControl::StateChanged( sal_uInt16, SfxItemState eState,
                                            const SfxPoolItem* pState )
{
    if( SFX_ITEM_AVAILABLE != eState )
    {
        GetStatusBar().SetItemText( GetId(), String() );
        nValueSet = 0;
    }
    else if ( dynamic_cast< const SfxUInt16Item* >(pState) )
    {
        const SfxUInt16Item* pItem = (const SfxUInt16Item*)pState;
        nZoom = pItem->GetValue();
        String aStr( String::CreateFromInt32(nZoom) );
        aStr.AppendAscii( " %" );
        GetStatusBar().SetItemText( GetId(), aStr );

        if ( dynamic_cast< const SvxZoomItem* >(pState) )
        {
            nValueSet = ((const SvxZoomItem*)pState)->GetValueSet();
/*!!!
            SvxZoomType eType = ((const SvxZoomItem*)pState)->GetType();

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

void SvxZoomStatusBarControl::Paint( const UserDrawEvent& )
{
    String aStr( String::CreateFromInt32( nZoom ));
    aStr.AppendAscii( " %" );
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

            switch( aPop.GetCurId() )
            {
            case ZOOM_OPTIMAL:      aZoom.SetType( SVX_ZOOM_OPTIMAL ); break;
            case ZOOM_PAGE_WIDTH:   aZoom.SetType( SVX_ZOOM_PAGEWIDTH ); break;
            case ZOOM_WHOLE_PAGE:   aZoom.SetType( SVX_ZOOM_WHOLEPAGE ); break;
            }

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

sal_uIntPtr SvxZoomStatusBarControl::GetDefItemWidth(const StatusBar& rStb)
{
    long nWidth1 =  rStb.GetTextWidth(String::CreateFromAscii("XXXXX %"));
    return nWidth1;
}


