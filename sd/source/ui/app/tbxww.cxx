/*************************************************************************
 *
 *  $RCSfile: tbxww.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:31 $
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

#pragma hdrstop

#include <svx/svxids.hrc>

#include "sddll.hxx"
#include "grdocsh.hxx"

#include "app.hxx"
#include "app.hrc"
#include "res_bmp.hrc"
#include "tbxitem.hxx"
#include "sdresid.hxx"
#include "tbx_ww.hxx"
#include "tbx_ww.hrc"

SFX_IMPL_TOOLBOX_CONTROL( SdTbxControl, TbxImageItem )

/*************************************************************************
|*
|* PopUp-Window
|*
\************************************************************************/

SdPopupWindowTbx::SdPopupWindowTbx( USHORT nId, WindowAlign eAlign,
                                    SdResId aRIdWin, SdResId aRIdTbx,
                                    SfxBindings& rBindings ) :
                SfxPopupWindow  ( nId, aRIdWin, rBindings ),
                aTbx            ( this, GetBindings(), aRIdTbx ),
                aSdResIdWin       ( aRIdWin ),
                aSdResIdTbx       ( aRIdTbx ),
                eTbxAlign       ( eAlign )
{
    aTbx.UseDefault();
    aSelectLink = aTbx.GetToolBox().GetSelectHdl();
    aTbx.GetToolBox().SetSelectHdl( LINK( this, SdPopupWindowTbx, TbxSelectHdl ) );

    FreeResource();

    if( ( SID_GRAFTBX_FILTERS != nId ) &&
        ( ( eAlign == WINDOWALIGN_TOP ) || ( eAlign == WINDOWALIGN_BOTTOM ) ) )
    {
       aTbx.GetToolBox().SetAlign( WINDOWALIGN_LEFT );
       SetText( String() );
    }

    Size aSize = aTbx.CalcWindowSizePixel();
    aTbx.SetPosSizePixel( Point(), aSize );
    SetOutputSizePixel( aSize );
}


/*-------------------------------------------------------------------------*/

__EXPORT SdPopupWindowTbx::~SdPopupWindowTbx()
{
}

/*-------------------------------------------------------------------------*/

SfxPopupWindow* SdPopupWindowTbx::Clone() const
{
    SfxBindings& rBindings = (SfxBindings&)GetBindings();

    return( new SdPopupWindowTbx( GetId(), eTbxAlign,
                    aSdResIdWin, aSdResIdTbx, rBindings ) );

    //return( SfxPopupWindow::Clone() );
}

/*-------------------------------------------------------------------------*/

void SdPopupWindowTbx::Update()
{
    ToolBox *pBox = &aTbx.GetToolBox();
    aTbx.Activate( pBox );
    aTbx.Deactivate( pBox );
}

/*-------------------------------------------------------------------------*/
void SdPopupWindowTbx::PopupModeEnd()
{
    aTbx.GetToolBox().EndSelection();

    SfxPopupWindow::PopupModeEnd();
}

/*-------------------------------------------------------------------------*/
IMPL_LINK( SdPopupWindowTbx, TbxSelectHdl, void*, EMPTYARG)
{
#ifdef VCL
    if( IsInPopupMode() )
        EndPopupMode();
#endif

    aSelectLink.Call( &aTbx.GetToolBox() );

    return( 0L );
}

/*************************************************************************
|*
|* Klasse fuer Toolbox
|*
\************************************************************************/

SdTbxControl::SdTbxControl( USHORT nId, ToolBox& rTbx, SfxBindings& rBind ) :
        SfxToolBoxControl( nId, rTbx, rBind )
{
}

/*-------------------------------------------------------------------------*/

SfxPopupWindowType __EXPORT SdTbxControl::GetPopupWindowType() const
{
    return( SFX_POPUPWINDOW_ONTIMEOUT );
}

/*************************************************************************
|*
|* Hier wird das Fenster erzeugt
|* Lage der Toolbox mit GetToolBox() abfragbar
|* rItemRect sind die Screen-Koordinaten
|*
\************************************************************************/

SfxPopupWindow* __EXPORT SdTbxControl::CreatePopupWindow()
{
    SdPopupWindowTbx *pWin = NULL;

    switch( GetId() )
    {
        case SID_OBJECT_ALIGN:
        {
            pWin = new SdPopupWindowTbx( GetId(), this->GetToolBox().GetAlign(),
                    SdResId( RID_ALIGNMENT ),
                    SdResId( RID_ALIGNMENT_TBX ), GetBindings() );
        }
        break;

        case SID_ZOOM_TOOLBOX:
        {
            pWin = new SdPopupWindowTbx( GetId(), this->GetToolBox().GetAlign(),
                    SdResId( RID_ZOOM ),
                    SdResId( RID_ZOOM_TBX ), GetBindings() );
        }
        break;

        case SID_OBJECT_CHOOSE_MODE:
        {
            pWin = new SdPopupWindowTbx(  GetId(), this->GetToolBox().GetAlign(),
                    SdResId( RID_CHOOSE_MODE ),
                    SdResId( RID_CHOOSE_MODE_TBX ), GetBindings() );
        }
        break;

        case SID_POSITION:
        {
            pWin = new SdPopupWindowTbx(  GetId(), this->GetToolBox().GetAlign(),
                    SdResId( RID_POSITION ),
                    SdResId( RID_POSITION_TBX ), GetBindings() );
        }
        break;

        case SID_DRAWTBX_TEXT:
        {
            pWin = new SdPopupWindowTbx( GetId(), this->GetToolBox().GetAlign(),
                    SdResId( RID_TEXT ),
                    SdResId( RID_TEXT_TBX ), GetBindings() );
        }
        break;

        case SID_DRAWTBX_RECTANGLES:
        {
            pWin = new SdPopupWindowTbx( GetId(), this->GetToolBox().GetAlign(),
                    SdResId( RID_RECTANGLES ),
                    SdResId( RID_RECTANGLES_TBX ), GetBindings() );
        }
        break;

        case SID_DRAWTBX_ELLIPSES:
        {
            pWin = new SdPopupWindowTbx( GetId(), this->GetToolBox().GetAlign(),
                    SdResId( RID_ELLIPSES ),
                    SdResId( RID_ELLIPSES_TBX ), GetBindings() );
        }
        break;

        case SID_DRAWTBX_LINES:
        {
            pWin = new SdPopupWindowTbx( GetId(), this->GetToolBox().GetAlign(),
                    SdResId( RID_LINES ),
                    SdResId( RID_LINES_TBX ), GetBindings() );
        }
        break;

        case SID_DRAWTBX_ARROWS:
        {
            pWin = new SdPopupWindowTbx( GetId(), this->GetToolBox().GetAlign(),
                    SdResId( RID_ARROWS ),
                    SdResId( RID_ARROWS_TBX ), GetBindings() );
        }
        break;

        case SID_DRAWTBX_3D_OBJECTS:
        {
            pWin = new SdPopupWindowTbx( GetId(), this->GetToolBox().GetAlign(),
                    SdResId( RID_3D_OBJECTS ),
                    SdResId( RID_3D_OBJECTS_TBX ), GetBindings() );
        }
        break;

        case SID_DRAWTBX_CONNECTORS:
        {
            pWin = new SdPopupWindowTbx( GetId(), this->GetToolBox().GetAlign(),
                    SdResId( RID_CONNECTORS ),
                    SdResId( RID_CONNECTORS_TBX ), GetBindings() );
        }
        break;

        case SID_DRAWTBX_INSERT:
        {
            pWin = new SdPopupWindowTbx( GetId(), this->GetToolBox().GetAlign(),
                    SdResId( RID_INSERT ),
                    SdResId( RID_INSERT_TBX ), GetBindings() );
        }
        break;

        case SID_GRAFTBX_FILTERS:
        {
            pWin = new SdPopupWindowTbx( GetId(), this->GetToolBox().GetAlign(),
                    SdResId( RID_GRAFFILTERS ),
                    SdResId( RID_GRAFFILTERS_TBX ), GetBindings() );
        }
        break;

    }
    if( pWin )
    {
        pWin->StartPopupMode( &GetToolBox(), TRUE );
        pWin->Update();
        pWin->StartSelection();
        pWin->Show();
    }
    return( pWin );
}

/*-------------------------------------------------------------------------*/

void __EXPORT SdTbxControl::StateChanged( USHORT nSId,
                        SfxItemState eState, const SfxPoolItem* pState )
{
    SfxToolBoxControl::StateChanged( nSId, eState, pState );

    if( eState == SFX_ITEM_AVAILABLE )
    {
        TbxImageItem* pItem = PTR_CAST( TbxImageItem, pState );
        //DBG_ASSERT( pItem, "TbxImageItem erwartet!" );
        // Im StarDesktop kann jetzt auch ein anderes Item ankommen,
        // das nicht ausgewertet werden darf (#33802# und #33838#)
        if( pItem )
        {
            ToolBox& rTbx = GetToolBox();
            USHORT nImage = pItem->GetValue();
            if( nImage == 0 )
            {
                if( rTbx.IsItemChecked( nSId ) )
                    rTbx.CheckItem( nSId, FALSE );
            }
            else
            {
                Image  aImage = SFX_IMAGEMANAGER()->GetImage( nImage, SD_MOD() );
                // Es kann u.U. ein Default-Image zurueckgegeben werden,
                // welches eine Breite von 0 haben soll
                // if( aImage.GetSizePixel().Width() > 0 )
                // !-Operator prueft, ob Image nicht vorhanden ist
                if( !!aImage )
                {
                    rTbx.SetItemImage( nSId, aImage );
                    rTbx.CheckItem( nSId, IsCheckable( nImage ) );

                    if( nSId != SID_ZOOM_TOOLBOX &&
                        nSId != SID_DRAWTBX_INSERT &&
                        nSId != SID_POSITION &&
                        nSId != SID_GRAFTBX_FILTERS )
                    {
                        if( nSId != SID_OBJECT_CHOOSE_MODE &&
                            rTbx.IsItemChecked( SID_OBJECT_CHOOSE_MODE ) )
                            rTbx.CheckItem( SID_OBJECT_CHOOSE_MODE, FALSE );
                        if( nSId != SID_OBJECT_ALIGN &&
                            rTbx.IsItemChecked( SID_OBJECT_ALIGN ) )
                            rTbx.CheckItem( SID_OBJECT_ALIGN, FALSE );
                        if( nSId != SID_DRAWTBX_TEXT &&
                            rTbx.IsItemChecked( SID_DRAWTBX_TEXT ) )
                             rTbx.CheckItem( SID_DRAWTBX_TEXT, FALSE );
                        if( nSId != SID_DRAWTBX_RECTANGLES &&
                            rTbx.IsItemChecked( SID_DRAWTBX_RECTANGLES ) )
                               rTbx.CheckItem( SID_DRAWTBX_RECTANGLES, FALSE );
                        if( nSId != SID_DRAWTBX_ELLIPSES &&
                            rTbx.IsItemChecked( SID_DRAWTBX_ELLIPSES ) )
                               rTbx.CheckItem( SID_DRAWTBX_ELLIPSES, FALSE );
                        if( nSId != SID_DRAWTBX_LINES &&
                            rTbx.IsItemChecked( SID_DRAWTBX_LINES ) )
                            rTbx.CheckItem( SID_DRAWTBX_LINES, FALSE );
                        if( nSId != SID_DRAWTBX_ARROWS &&
                            rTbx.IsItemChecked( SID_DRAWTBX_ARROWS ) )
                            rTbx.CheckItem( SID_DRAWTBX_ARROWS, FALSE );
                        if( nSId != SID_DRAWTBX_3D_OBJECTS &&
                            rTbx.IsItemChecked( SID_DRAWTBX_3D_OBJECTS ) )
                            rTbx.CheckItem( SID_DRAWTBX_3D_OBJECTS, FALSE );
                        if( nSId != SID_DRAWTBX_CONNECTORS &&
                            rTbx.IsItemChecked( SID_DRAWTBX_CONNECTORS ) )
                            rTbx.CheckItem( SID_DRAWTBX_CONNECTORS, FALSE );
                    }
                }
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

BOOL SdTbxControl::IsCheckable( USHORT nSId )
{
    switch( nSId )
    {
        case SID_OBJECT_ROTATE:
        case SID_OBJECT_MIRROR:
        case SID_OBJECT_TRANSPARENCE:
        case SID_OBJECT_GRADIENT:
        case SID_OBJECT_SHEAR:
        case SID_OBJECT_CROOK_ROTATE:
        case SID_OBJECT_CROOK_SLANT:
        case SID_OBJECT_CROOK_STRETCH:
        case SID_CONVERT_TO_3D_LATHE:

        case SID_ATTR_CHAR:
        case SID_TEXT_FITTOSIZE:
        case SID_DRAW_CAPTION:

        case SID_DRAW_RECT:
        case SID_DRAW_SQUARE:
        case SID_DRAW_RECT_ROUND:
        case SID_DRAW_SQUARE_ROUND:
        case SID_DRAW_RECT_NOFILL:
        case SID_DRAW_SQUARE_NOFILL:
        case SID_DRAW_RECT_ROUND_NOFILL:
        case SID_DRAW_SQUARE_ROUND_NOFILL:

        case SID_DRAW_ELLIPSE:
        case SID_DRAW_CIRCLE:
        case SID_DRAW_PIE:
        case SID_DRAW_CIRCLEPIE:
        case SID_DRAW_ELLIPSECUT:
        case SID_DRAW_CIRCLECUT:
        case SID_DRAW_ARC:
        case SID_DRAW_CIRCLEARC:
        case SID_DRAW_ELLIPSE_NOFILL:
        case SID_DRAW_CIRCLE_NOFILL:
        case SID_DRAW_PIE_NOFILL:
        case SID_DRAW_CIRCLEPIE_NOFILL:
        case SID_DRAW_ELLIPSECUT_NOFILL:
        case SID_DRAW_CIRCLECUT_NOFILL:

        case SID_DRAW_BEZIER_NOFILL:
        case SID_DRAW_POLYGON_NOFILL:
        case SID_DRAW_XPOLYGON_NOFILL:
        case SID_DRAW_BEZIER_FILL:
        case SID_DRAW_POLYGON:
        case SID_DRAW_XPOLYGON:
        case SID_DRAW_FREELINE:
        case SID_DRAW_FREELINE_NOFILL:

        case SID_DRAW_LINE:
        case SID_DRAW_XLINE:
        case SID_DRAW_MEASURELINE:
        case SID_LINE_ARROW_START:
        case SID_LINE_ARROW_END:
        case SID_LINE_ARROWS:
        case SID_LINE_ARROW_CIRCLE:
        case SID_LINE_CIRCLE_ARROW:
        case SID_LINE_ARROW_SQUARE:
        case SID_LINE_SQUARE_ARROW:

        case SID_3D_CUBE:
        case SID_3D_SPHERE:
        case SID_3D_CYLINDER:
        case SID_3D_CONE:
        case SID_3D_PYRAMID:
        case SID_3D_TORUS:
        case SID_3D_SHELL:
        case SID_3D_HALF_SPHERE:

        case SID_TOOL_CONNECTOR:
        case SID_CONNECTOR_ARROW_START:
        case SID_CONNECTOR_ARROW_END:
        case SID_CONNECTOR_ARROWS:
        case SID_CONNECTOR_CIRCLE_START:
        case SID_CONNECTOR_CIRCLE_END:
        case SID_CONNECTOR_CIRCLES:
        case SID_CONNECTOR_LINE:
        case SID_CONNECTOR_LINE_ARROW_START:
        case SID_CONNECTOR_LINE_ARROW_END:
        case SID_CONNECTOR_LINE_ARROWS:
        case SID_CONNECTOR_LINE_CIRCLE_START:
        case SID_CONNECTOR_LINE_CIRCLE_END:
        case SID_CONNECTOR_LINE_CIRCLES:
        case SID_CONNECTOR_CURVE:
        case SID_CONNECTOR_CURVE_ARROW_START:
        case SID_CONNECTOR_CURVE_ARROW_END:
        case SID_CONNECTOR_CURVE_ARROWS:
        case SID_CONNECTOR_CURVE_CIRCLE_START:
        case SID_CONNECTOR_CURVE_CIRCLE_END:
        case SID_CONNECTOR_CURVE_CIRCLES:
        case SID_CONNECTOR_LINES:
        case SID_CONNECTOR_LINES_ARROW_START:
        case SID_CONNECTOR_LINES_ARROW_END:
        case SID_CONNECTOR_LINES_ARROWS:
        case SID_CONNECTOR_LINES_CIRCLE_START:
        case SID_CONNECTOR_LINES_CIRCLE_END:
        case SID_CONNECTOR_LINES_CIRCLES:



        //case SID_ZOOM_OUT:
            return( TRUE );
    }
    return( FALSE );
}



