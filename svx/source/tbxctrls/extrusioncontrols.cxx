/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: extrusioncontrols.cxx,v $
 * $Revision: 1.16 $
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
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------

#include <string> // HACK: prevent conflict between STLPORT and Workshop headers

#ifndef _TOOLBOX_HXX //autogen
#include <vcl/toolbox.hxx>
#endif
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <svl/eitem.hxx>
#include <vcl/settings.hxx>
#include <svl/intitem.hxx>
#include <svx/colritem.hxx>
#include "chrtitem.hxx"
#include <tools/urlobj.hxx>

#include <svx/dialogs.hrc>
#include "helpid.hrc"

#include <svx/svdtrans.hxx>
#include <svx/dialmgr.hxx>
#include "extrusioncontrols.hxx"
#include "extrusioncontrols.hrc"
#include <svx/sdasitm.hxx>
#include "toolbarmenu.hxx"
#include "colorwindow.hxx"

#include <svx/tbxcolorupdate.hxx>

////////////

using namespace svx;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

SFX_IMPL_TOOLBOX_CONTROL( ExtrusionDirectionControl, SfxBoolItem );

/*************************************************************************
|*
|* ExtrusionDirectionWindow
|*
\************************************************************************/

static sal_Int32 gSkewList[] = { 135, 90, 45, 180, 0, -360, -135, -90, -45 };

ExtrusionDirectionWindow::ExtrusionDirectionWindow(
    USHORT nId,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame ) :
    SfxPopupWindow( nId,
                    rFrame,
                    SVX_RES( RID_SVXFLOAT_EXTRUSION_DIRECTION )),
    mxFrame( rFrame ),
    maImgPerspective( SVX_RES( IMG_PERSPECTIVE ) ),
    maImgPerspectiveH( SVX_RES( IMG_PERSPECTIVE_H ) ),
    maImgParallel( SVX_RES( IMG_PARALLEL ) ),
    maImgParallelH( SVX_RES( IMG_PARALLEL_H ) ),
    mbPopupMode     ( TRUE )
{
    implInit();
}

ExtrusionDirectionWindow::ExtrusionDirectionWindow(
    USHORT nId,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
    Window* pParentWindow ) :
    SfxPopupWindow( nId,
                    rFrame,
                    pParentWindow,
                    SVX_RES( RID_SVXFLOAT_EXTRUSION_DIRECTION )),
    mxFrame( rFrame ),
    maImgPerspective( SVX_RES( IMG_PERSPECTIVE ) ),
    maImgPerspectiveH( SVX_RES( IMG_PERSPECTIVE_H ) ),
    maImgParallel( SVX_RES( IMG_PARALLEL ) ),
    maImgParallelH( SVX_RES( IMG_PARALLEL_H ) ),
    mbPopupMode     ( TRUE )
{
    implInit();
}

void ExtrusionDirectionWindow::implInit()
{
    SetHelpId( HID_POPUP_EXTRUSION_DIRECTION );

    USHORT i;
    for( i = DIRECTION_NW; i <= DIRECTION_SE; i++ )
    {
        maImgDirection[i] = Image( SVX_RES( IMG_DIRECTION + i ) );
        maImgDirectionH[i] = Image( SVX_RES( IMG_DIRECTION_H + i ) );
    }

//  mpDirectionForewarder = new SfxStatusForwarder( SID_EXTRUSION_DIRECTION, *this );
//  mpProjectionForewarder = new SfxStatusForwarder( SID_EXTRUSION_PROJECTION, *this );

    mpMenu = new ToolbarMenu( this, WB_CLIPCHILDREN );
    mpMenu->SetHelpId( HID_MENU_EXTRUSION_DIRECTION );
    mpMenu->SetSelectHdl( LINK( this, ExtrusionDirectionWindow, SelectHdl ) );
    mpDirectionSet = new ValueSet( mpMenu, WB_TABSTOP | WB_MENUSTYLEVALUESET | WB_FLATVALUESET | WB_NOBORDER | WB_NO_DIRECTSELECT );
    mpDirectionSet->SetHelpId( HID_VALUESET_EXTRUSION_DIRECTION );

    mpDirectionSet->SetHelpId( HID_POPUP_LINEEND_CTRL );
    mpDirectionSet->SetSelectHdl( LINK( this, ExtrusionDirectionWindow, SelectHdl ) );
    mpDirectionSet->SetColCount( 3 );
    mpDirectionSet->EnableFullItemMode( FALSE );

    bool bHighContrast = GetDisplayBackground().GetColor().IsDark();

    for( i = DIRECTION_NW; i <= DIRECTION_SE; i++ )
    {
        String aText( SVX_RES( STR_DIRECTION + i ) );
        mpDirectionSet->InsertItem( i+1, bHighContrast ? maImgDirectionH[ i ] : maImgDirection[ i ], aText );
    }

    mpDirectionSet->SetOutputSizePixel( Size( 72, 72 ) );

    mpMenu->appendEntry( 2, mpDirectionSet );
    mpMenu->appendSeparator();
    mpMenu->appendEntry( 0, String( SVX_RES( STR_PERSPECTIVE ) ), bHighContrast ? maImgPerspectiveH : maImgPerspective );
    mpMenu->appendEntry( 1, String( SVX_RES( STR_PARALLEL ) ), bHighContrast ? maImgParallelH : maImgParallel );

    SetOutputSizePixel( mpMenu->getMenuSize() );
    mpMenu->SetOutputSizePixel( GetOutputSizePixel() );

    mpMenu->Show();

    FreeResource();

    AddStatusListener( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:ExtrusionDirection" )));
    AddStatusListener( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:ExtrusionProjection" )));
}

SfxPopupWindow* ExtrusionDirectionWindow::Clone() const
{
    return new ExtrusionDirectionWindow( GetId(), mxFrame );
}

void ExtrusionDirectionWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxPopupWindow::DataChanged( rDCEvt );

    if( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
    {
        bool bHighContrast = GetDisplayBackground().GetColor().IsDark();

        USHORT i;
        for( i = DIRECTION_NW; i <= DIRECTION_SE; i++ )
        {
            mpDirectionSet->SetItemImage( i+1, bHighContrast ? maImgDirectionH[ i ] : maImgDirection[ i ] );
        }

        mpMenu->setEntryImage( 0, bHighContrast ? maImgPerspectiveH : maImgPerspective );
        mpMenu->setEntryImage( 1, bHighContrast ? maImgParallelH : maImgParallel );
    }
}

// -----------------------------------------------------------------------

ExtrusionDirectionWindow::~ExtrusionDirectionWindow()
{
    delete mpMenu;
}

// -----------------------------------------------------------------------

void ExtrusionDirectionWindow::implSetDirection( sal_Int32 nSkew, bool bEnabled )
{
    if( mpDirectionSet )
    {
        USHORT nItemId;
        for( nItemId = DIRECTION_NW; nItemId <= DIRECTION_SE; nItemId++ )
        {
            if( gSkewList[nItemId] == nSkew )
                break;
        }

        if( nItemId <= DIRECTION_SE )
        {
            mpDirectionSet->SelectItem( nItemId+1 );
        }
        else
        {
            mpDirectionSet->SetNoSelection();
        }
    }
    if( mpMenu )
    {
        mpMenu->enableEntry( 2, bEnabled );
    }
}

// -----------------------------------------------------------------------

void ExtrusionDirectionWindow::implSetProjection( sal_Int32 nProjection, bool bEnabled )
{
    if( mpMenu )
    {
        mpMenu->checkEntry( 0, (nProjection == 0) && bEnabled );
        mpMenu->checkEntry( 1, (nProjection == 1 ) && bEnabled );
        mpMenu->enableEntry( 0, bEnabled );
        mpMenu->enableEntry( 1, bEnabled );
    }
}

// -----------------------------------------------------------------------

void ExtrusionDirectionWindow::StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    switch( nSID )
    {
        case SID_EXTRUSION_DIRECTION:
        {
            if( eState == SFX_ITEM_DISABLED )
            {
                implSetDirection( -1, false );
            }
            else
            {
                const SfxInt32Item* pStateItem = PTR_CAST( SfxInt32Item, pState );
                if( pStateItem )
                    implSetDirection( pStateItem->GetValue(), true );
            }
            break;
        }
        case SID_EXTRUSION_PROJECTION:
        {
            if( eState == SFX_ITEM_DISABLED )
            {
                implSetProjection( -1, false );
            }
            else
            {
                const SfxInt32Item* pStateItem = PTR_CAST( SfxInt32Item, pState );
                if( pStateItem )
                    implSetProjection( pStateItem->GetValue(), true );
            }
            break;
        }
    }
}

// -----------------------------------------------------------------------

IMPL_LINK( ExtrusionDirectionWindow, SelectHdl, void *, pControl )
{
    if ( IsInPopupMode() )
        EndPopupMode();

//  SfxDispatcher* pDisp = GetBindings().GetDispatcher();

    if( pControl == mpDirectionSet )
    {
        sal_Int32 nSkew = gSkewList[mpDirectionSet->GetSelectItemId()-1];

        SfxInt32Item    aItem( SID_EXTRUSION_DIRECTION, nSkew );
        rtl::OUString   aCommand( RTL_CONSTASCII_USTRINGPARAM( ".uno:ExtrusionDirection" ));

        Any a;
        INetURLObject aObj( aCommand );
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = aObj.GetURLPath();
        aItem.QueryValue( a );
        aArgs[0].Value = a;

        SfxToolBoxControl::Dispatch( Reference< ::com::sun::star::frame::XDispatchProvider >(
                                     mxFrame->getController(), UNO_QUERY ),
                                     aCommand,
                                     aArgs );

//      pDisp->Execute( SID_EXTRUSION_DIRECTION, SFX_CALLMODE_RECORD, &aItem, 0L , 0L );
    }
    else
    {
        int nProjection = mpMenu->getSelectedEntryId();
        if( (nProjection >= 0) && (nProjection < 2 ) )
        {
            SfxInt32Item    aItem( SID_EXTRUSION_PROJECTION, nProjection );
            rtl::OUString   aCommand( RTL_CONSTASCII_USTRINGPARAM( ".uno:ExtrusionProjection" ));

            Any a;
            INetURLObject aObj( aCommand );
            Sequence< PropertyValue > aArgs( 1 );
            aArgs[0].Name = aObj.GetURLPath();
            aItem.QueryValue( a );
            aArgs[0].Value = a;

            SfxToolBoxControl::Dispatch( Reference< ::com::sun::star::frame::XDispatchProvider >(
                                        mxFrame->getController(), UNO_QUERY ),
                                        aCommand,
                                        aArgs );
            implSetProjection( nProjection, true );
        }
    }

    return 0;
}

// -----------------------------------------------------------------------

void ExtrusionDirectionWindow::FillValueSet()
{
}

// -----------------------------------------------------------------------

void ExtrusionDirectionWindow::StartSelection()
{
    mpDirectionSet->StartSelection();
}

// -----------------------------------------------------------------------

BOOL ExtrusionDirectionWindow::Close()
{
    return SfxPopupWindow::Close();
}

// -----------------------------------------------------------------------

void ExtrusionDirectionWindow::PopupModeEnd()
{
    if ( IsVisible() )
    {
        mbPopupMode = FALSE;
    }
    SfxPopupWindow::PopupModeEnd();
}

// -----------------------------------------------------------------------

void ExtrusionDirectionWindow::GetFocus (void)
{
    SfxPopupWindow::GetFocus();
    // Grab the focus to the line ends value set so that it can be controlled
    // with the keyboard.
    if( mpDirectionSet )
        mpDirectionSet->GrabFocus();
}

/*************************************************************************
|*
|* SvxLineEndToolBoxControl
|*
\************************************************************************/

ExtrusionDirectionControl::ExtrusionDirectionControl(
    USHORT nSlotId, USHORT  nId, ToolBox &rTbx )
: SfxToolBoxControl( nSlotId, nId, rTbx )
{
    rTbx.SetItemBits( nId, TIB_DROPDOWNONLY | rTbx.GetItemBits( nId ) );
}

// -----------------------------------------------------------------------

ExtrusionDirectionControl::~ExtrusionDirectionControl()
{
}

// -----------------------------------------------------------------------

SfxPopupWindowType ExtrusionDirectionControl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONCLICK;
}

// -----------------------------------------------------------------------

SfxPopupWindow* ExtrusionDirectionControl::CreatePopupWindow()
{
    ExtrusionDirectionWindow* pWin = new ExtrusionDirectionWindow( GetId(), m_xFrame, &GetToolBox() );
    pWin->StartPopupMode( &GetToolBox(), TRUE );
    pWin->StartSelection();
    SetPopupWindow( pWin );
    return pWin;
}

// -----------------------------------------------------------------------

void ExtrusionDirectionControl::StateChanged( USHORT, SfxItemState eState, const SfxPoolItem* )
{
    USHORT nId = GetId();
    ToolBox& rTbx = GetToolBox();

    rTbx.EnableItem( nId, SFX_ITEM_DISABLED != eState );
    rTbx.SetItemState( nId, ( SFX_ITEM_DONTCARE == eState ) ? STATE_DONTKNOW : STATE_NOCHECK );
}

// ####################################################################

ExtrusionDepthDialog::ExtrusionDepthDialog( Window* pParent, double fDepth, FieldUnit eDefaultUnit )
:   ModalDialog( pParent, SVX_RES( RID_SVX_MDLG_EXTRUSION_DEPTH ) ),
    maFLDepth( this, SVX_RES( FL_DEPTH ) ),
    maMtrDepth( this, SVX_RES( MTR_DEPTH ) ),
    maOKButton( this, SVX_RES( BTN_OK ) ),
    maCancelButton( this, SVX_RES( BTN_CANCEL ) ),
    maHelpButton( this, SVX_RES( BTN_HELP ) )
{
    bool bMetric = IsMetric( eDefaultUnit );
    maMtrDepth.SetUnit( bMetric ? FUNIT_CM : FUNIT_INCH );
    maMtrDepth.SetValue( (int) fDepth * 100, FUNIT_100TH_MM );

    FreeResource();
}

ExtrusionDepthDialog::~ExtrusionDepthDialog()
{
}

double ExtrusionDepthDialog::getDepth() const
{
//  bool bMetric = IsMetric( meDefaultUnit );
    return (double)( maMtrDepth.GetValue( FUNIT_100TH_MM ) ) / 100.0;
}

// ####################################################################

SFX_IMPL_TOOLBOX_CONTROL( ExtrusionDepthControl, SfxBoolItem );

ExtrusionDepthWindow::ExtrusionDepthWindow(
    USHORT nId,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame ) :
    SfxPopupWindow( nId,
                    rFrame,
                    SVX_RES( RID_SVXFLOAT_EXTRUSION_DEPTH )),
    maImgDepth0( SVX_RES( IMG_DEPTH_0 ) ),
    maImgDepth1( SVX_RES( IMG_DEPTH_1 ) ),
    maImgDepth2( SVX_RES( IMG_DEPTH_2 ) ),
    maImgDepth3( SVX_RES( IMG_DEPTH_3 ) ),
    maImgDepth4( SVX_RES( IMG_DEPTH_4 ) ),
    maImgDepthInfinity( SVX_RES( IMG_DEPTH_INFINITY ) ),
    maImgDepth0h( SVX_RES( IMG_DEPTH_0_H ) ),
    maImgDepth1h( SVX_RES( IMG_DEPTH_1_H ) ),
    maImgDepth2h( SVX_RES( IMG_DEPTH_2_H ) ),
    maImgDepth3h( SVX_RES( IMG_DEPTH_3_H ) ),
    maImgDepth4h( SVX_RES( IMG_DEPTH_4_H ) ),
    maImgDepthInfinityh( SVX_RES( IMG_DEPTH_INFINITY_H ) ),
    mxFrame( rFrame ),
    mbPopupMode     ( true ),
    mfDepth( -1.0 ),
    mbEnabled( false )
{
    implInit();
}

ExtrusionDepthWindow::ExtrusionDepthWindow( USHORT nId,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
    Window* pParentWindow ) :
    SfxPopupWindow( nId,
                    rFrame,
                    pParentWindow,
                    SVX_RES( RID_SVXFLOAT_EXTRUSION_DEPTH )),
    maImgDepth0( SVX_RES( IMG_DEPTH_0 ) ),
    maImgDepth1( SVX_RES( IMG_DEPTH_1 ) ),
    maImgDepth2( SVX_RES( IMG_DEPTH_2 ) ),
    maImgDepth3( SVX_RES( IMG_DEPTH_3 ) ),
    maImgDepth4( SVX_RES( IMG_DEPTH_4 ) ),
    maImgDepthInfinity( SVX_RES( IMG_DEPTH_INFINITY ) ),
    maImgDepth0h( SVX_RES( IMG_DEPTH_0_H ) ),
    maImgDepth1h( SVX_RES( IMG_DEPTH_1_H ) ),
    maImgDepth2h( SVX_RES( IMG_DEPTH_2_H ) ),
    maImgDepth3h( SVX_RES( IMG_DEPTH_3_H ) ),
    maImgDepth4h( SVX_RES( IMG_DEPTH_4_H ) ),
    maImgDepthInfinityh( SVX_RES( IMG_DEPTH_INFINITY_H ) ),
    mxFrame( rFrame ),
    mbPopupMode     ( true ),
    mfDepth( -1.0 ),
    mbEnabled( false )
{
    implInit();
}

void ExtrusionDepthWindow::implInit()
{
    SetHelpId( HID_POPUP_EXTRUSION_DEPTH );

//  mpDepthForewarder = new SfxStatusForwarder( SID_EXTRUSION_DEPTH, *this );
//  mpMetricForewarder = new SfxStatusForwarder( SID_ATTR_METRIC, *this );

    mpMenu = new ToolbarMenu( this, WB_CLIPCHILDREN );
    mpMenu->SetHelpId( HID_MENU_EXTRUSION_DEPTH );

    mpMenu->SetSelectHdl( LINK( this, ExtrusionDepthWindow, SelectHdl ) );

    bool bHighContrast = GetDisplayBackground().GetColor().IsDark();

    String aEmpty;
    mpMenu->appendEntry( 0, aEmpty, bHighContrast ? maImgDepth0h : maImgDepth0 );
    mpMenu->appendEntry( 1, aEmpty, bHighContrast ? maImgDepth1h : maImgDepth1 );
    mpMenu->appendEntry( 2, aEmpty, bHighContrast ? maImgDepth2h : maImgDepth2 );
    mpMenu->appendEntry( 3, aEmpty, bHighContrast ? maImgDepth3h : maImgDepth3 );
    mpMenu->appendEntry( 4, aEmpty, bHighContrast ? maImgDepth4h : maImgDepth4 );
    mpMenu->appendEntry( 5, String( SVX_RES( STR_INFINITY ) ), bHighContrast ? maImgDepthInfinityh : maImgDepthInfinity );
    mpMenu->appendEntry( 6, String( SVX_RES( STR_CUSTOM ) ) );

    SetOutputSizePixel( mpMenu->getMenuSize() );
    mpMenu->SetOutputSizePixel( GetOutputSizePixel() );

    mpMenu->Show();

    FreeResource();

    AddStatusListener( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:ExtrusionDepth" )));
    AddStatusListener( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:MetricUnit" )));
//  rBindings.Update( SID_ATTR_METRIC );
//  rBindings.Update( SID_EXTRUSION_DEPTH );
}

SfxPopupWindow* ExtrusionDepthWindow::Clone() const
{
    return new ExtrusionDepthWindow( GetId(), mxFrame );
}

// -----------------------------------------------------------------------

ExtrusionDepthWindow::~ExtrusionDepthWindow()
{
//  delete mpDepthForewarder;
//  delete mpMetricForewarder;

    delete mpMenu;
}

// -----------------------------------------------------------------------

double aDepthListInch[] = { 0, 1270,2540,5080,10160 };
double aDepthListMM[] = { 0, 1000, 2500, 5000, 10000 };

void ExtrusionDepthWindow::implSetDepth( double fDepth, bool bEnabled )
{
    mbEnabled = bEnabled;
    mfDepth = fDepth;
    if( mpMenu )
    {
        int i;
        for( i = 0; i < 7; i++ )
        {
            if( i == 5 )
            {
                mpMenu->checkEntry( i, (fDepth >= 338666) && bEnabled );
            }
            else if( i != 6 )
            {
                mpMenu->checkEntry( i, (fDepth == (IsMetric( meUnit ) ? aDepthListMM[i] : aDepthListInch[i]) && bEnabled ) );
            }
            mpMenu->enableEntry( i, bEnabled );
        }
    }
}

// -----------------------------------------------------------------------

void ExtrusionDepthWindow::implFillStrings( FieldUnit eUnit )
{
    meUnit = eUnit;
    USHORT nResource = IsMetric( eUnit ) ? RID_SVXSTR_DEPTH_0 : RID_SVXSTR_DEPTH_0_INCH;

    for( int i = 0; i < 5; i++ )
    {
        String aStr( SVX_RES( nResource + i ) );
        mpMenu->setEntryText( i, aStr );
    };
}

// -----------------------------------------------------------------------

void ExtrusionDepthWindow::StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    switch( nSID )
    {
        case SID_EXTRUSION_DEPTH:
        {
            if( eState == SFX_ITEM_DISABLED )
            {
                implSetDepth( 0, false );
            }
            else
            {
                const SvxDoubleItem* pStateItem = PTR_CAST( SvxDoubleItem, pState );
                if( pStateItem )
                    implSetDepth( pStateItem->GetValue(), true );
            }
            break;
        }
        case SID_ATTR_METRIC:
        {
            const SfxUInt16Item* pStateItem = PTR_CAST( SfxUInt16Item, pState );
            if( pStateItem )
            {
                implFillStrings( (FieldUnit)pStateItem->GetValue() );
                if( mfDepth >= 0.0 )
                    implSetDepth( mfDepth, mbEnabled );
            }
        }
    }
}

// -----------------------------------------------------------------------

void ExtrusionDepthWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxPopupWindow::DataChanged( rDCEvt );

    if( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
    {
        bool bHighContrast = GetDisplayBackground().GetColor().IsDark();

        mpMenu->setEntryImage( 0, bHighContrast ? maImgDepth0h : maImgDepth0 );
        mpMenu->setEntryImage( 1, bHighContrast ? maImgDepth1h : maImgDepth1 );
        mpMenu->setEntryImage( 2, bHighContrast ? maImgDepth2h : maImgDepth2 );
        mpMenu->setEntryImage( 3, bHighContrast ? maImgDepth3h : maImgDepth3 );
        mpMenu->setEntryImage( 4, bHighContrast ? maImgDepth4h : maImgDepth4 );
        mpMenu->setEntryImage( 5, bHighContrast ? maImgDepthInfinityh : maImgDepthInfinity );
    }
}


// -----------------------------------------------------------------------

IMPL_LINK( ExtrusionDepthWindow, SelectHdl, void *, EMPTYARG )
{
//  SfxDispatcher* pDisp = GetBindings().GetDispatcher();

    int nSelected = mpMenu->getSelectedEntryId();
    if( nSelected != -1 )
    {
        if( nSelected == 6 )
        {
            if ( IsInPopupMode() )
                EndPopupMode();

            SvxDoubleItem aDepthItem( mfDepth, SID_EXTRUSION_DEPTH );
            SfxUInt16Item aMetricItem(
                SID_ATTR_METRIC, sal::static_int_cast< UINT16 >( meUnit ) );
            rtl::OUString aCommand( RTL_CONSTASCII_USTRINGPARAM( ".uno:ExtrusionDepthDialog" ));

            Any a;
            Sequence< PropertyValue > aArgs( 2 );
            aArgs[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Depth" ));
            aDepthItem.QueryValue( a );
            aArgs[0].Value = a;
            aArgs[1].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Metric" ));
            aMetricItem.QueryValue( a );
            aArgs[1].Value = a;

            SfxToolBoxControl::Dispatch( Reference< ::com::sun::star::frame::XDispatchProvider >(
                                         mxFrame->getController(), UNO_QUERY ),
                                         aCommand,
                                         aArgs );
        }
        else
        {
            double fDepth;

            if( nSelected == 5 )
            {
                fDepth = 338666.6;
            }
            else
            {
                fDepth = IsMetric( meUnit ) ? aDepthListMM[nSelected] : aDepthListInch[nSelected];
            }

            SvxDoubleItem aItem( fDepth, SID_EXTRUSION_DEPTH );
            rtl::OUString aCommand( RTL_CONSTASCII_USTRINGPARAM( ".uno:ExtrusionDepth" ));

            Any a;
            INetURLObject aObj( aCommand );
            Sequence< PropertyValue > aArgs( 1 );
            aArgs[0].Name = aObj.GetURLPath();
            aItem.QueryValue( a );
            aArgs[0].Value = a;

            SfxToolBoxControl::Dispatch( Reference< ::com::sun::star::frame::XDispatchProvider >(
                                         mxFrame->getController(), UNO_QUERY ),
                                         aCommand,
                                         aArgs );
            implSetDepth( fDepth, true );

            if ( IsInPopupMode() )
                EndPopupMode();
        }
    }
    return 0;
}

// -----------------------------------------------------------------------

void ExtrusionDepthWindow::StartSelection()
{
}

// -----------------------------------------------------------------------

BOOL ExtrusionDepthWindow::Close()
{
    return SfxPopupWindow::Close();
}

// -----------------------------------------------------------------------

void ExtrusionDepthWindow::PopupModeEnd()
{
    if ( IsVisible() )
    {
        mbPopupMode = FALSE;
    }
    SfxPopupWindow::PopupModeEnd();
}

// -----------------------------------------------------------------------

void ExtrusionDepthWindow::GetFocus (void)
{
    SfxPopupWindow::GetFocus();
    // Grab the focus to the line ends value set so that it can be controlled
    // with the keyboard.
    if( mpMenu )
        mpMenu->GrabFocus();
}

// ========================================================================

ExtrusionDepthControl::ExtrusionDepthControl(
    USHORT nSlotId, USHORT nId, ToolBox &rTbx )
: SfxToolBoxControl( nSlotId, nId, rTbx )
{
    rTbx.SetItemBits( nId, TIB_DROPDOWNONLY | rTbx.GetItemBits( nId ) );
}

// -----------------------------------------------------------------------

ExtrusionDepthControl::~ExtrusionDepthControl()
{
}

// -----------------------------------------------------------------------

SfxPopupWindowType ExtrusionDepthControl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONCLICK;
}

// -----------------------------------------------------------------------

SfxPopupWindow* ExtrusionDepthControl::CreatePopupWindow()
{
    ExtrusionDepthWindow* pWin = new ExtrusionDepthWindow( GetId(), m_xFrame, &GetToolBox() );
    pWin->StartPopupMode( &GetToolBox(), TRUE );
    pWin->StartSelection();
    SetPopupWindow( pWin );
    return pWin;
}

// -----------------------------------------------------------------------

void ExtrusionDepthControl::StateChanged( USHORT, SfxItemState eState, const SfxPoolItem* )
{
    USHORT nId = GetId();
    ToolBox& rTbx = GetToolBox();

    rTbx.EnableItem( nId, SFX_ITEM_DISABLED != eState );
    rTbx.SetItemState( nId, ( SFX_ITEM_DONTCARE == eState ) ? STATE_DONTKNOW : STATE_NOCHECK );
}

// ####################################################################

SFX_IMPL_TOOLBOX_CONTROL( ExtrusionLightingControl, SfxBoolItem );

ExtrusionLightingWindow::ExtrusionLightingWindow(
    USHORT nId,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame ) :
    SfxPopupWindow( nId,
                    rFrame,
                    SVX_RES( RID_SVXFLOAT_EXTRUSION_LIGHTING ) ),
    maImgBright( SVX_RES( IMG_LIGHTING_BRIGHT ) ),
    maImgNormal( SVX_RES( IMG_LIGHTING_NORMAL ) ),
    maImgDim( SVX_RES( IMG_LIGHTING_DIM ) ),
    maImgBrighth( SVX_RES( IMG_LIGHTING_BRIGHT_H ) ),
    maImgNormalh( SVX_RES( IMG_LIGHTING_NORMAL_H ) ),
    maImgDimh( SVX_RES( IMG_LIGHTING_DIM_H ) ),
    mxFrame( rFrame ),
    mbPopupMode( true ),
    mnLevel( 0 ),
    mbLevelEnabled( false ),
    mnDirection( FROM_FRONT ),
    mbDirectionEnabled( false )
{
    implInit();
}

// -----------------------------------------------------------------------

ExtrusionLightingWindow::ExtrusionLightingWindow(
    USHORT nId,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
    Window* pParentWindow ) :
    SfxPopupWindow( nId,
                    rFrame,
                    pParentWindow,
                    SVX_RES( RID_SVXFLOAT_EXTRUSION_LIGHTING ) ),
    maImgBright( SVX_RES( IMG_LIGHTING_BRIGHT ) ),
    maImgNormal( SVX_RES( IMG_LIGHTING_NORMAL ) ),
    maImgDim( SVX_RES( IMG_LIGHTING_DIM ) ),
    maImgBrighth( SVX_RES( IMG_LIGHTING_BRIGHT_H ) ),
    maImgNormalh( SVX_RES( IMG_LIGHTING_NORMAL_H ) ),
    maImgDimh( SVX_RES( IMG_LIGHTING_DIM_H ) ),
    mxFrame( rFrame ),
    mbPopupMode( true ),
    mnLevel( 0 ),
    mbLevelEnabled( false ),
    mnDirection( FROM_FRONT ),
    mbDirectionEnabled( false )
{
    implInit();
}

// -----------------------------------------------------------------------

void ExtrusionLightingWindow::implInit()
{
    SetHelpId( HID_POPUP_EXTRUSION_LIGHTING );

    USHORT i;
    for( i = FROM_TOP_LEFT; i <= FROM_BOTTOM_RIGHT; i++ )
    {
        if( i != FROM_FRONT )
        {
            maImgLightingOff[i] = Image( SVX_RES( IMG_LIGHT_OFF + i ) );
            maImgLightingOn[i] = Image( SVX_RES( IMG_LIGHT_ON + i ) );
            maImgLightingOffh[i] = Image( SVX_RES( IMG_LIGHT_OFF_H + i ) );
            maImgLightingOnh[i] = Image( SVX_RES( IMG_LIGHT_ON_H + i ) );
        }
        maImgLightingPreview[i] = Image( SVX_RES( IMG_LIGHT_PREVIEW + i ) );
        maImgLightingPreviewh[i] = Image( SVX_RES( IMG_LIGHT_PREVIEW_H + i ) );
    }

//  mpLightingDirectionForewarder = new SfxStatusForwarder( SID_EXTRUSION_LIGHTING_DIRECTION, *this );
//  mpLightingIntensityForewarder = new SfxStatusForwarder( SID_EXTRUSION_LIGHTING_INTENSITY, *this );

    mpMenu = new ToolbarMenu( this, WB_CLIPCHILDREN );
    mpMenu->SetHelpId( HID_MENU_EXTRUSION_LIGHTING );
    mpMenu->SetSelectHdl( LINK( this, ExtrusionLightingWindow, SelectHdl ) );

    mpLightingSet = new ValueSet( mpMenu, WB_TABSTOP | WB_MENUSTYLEVALUESET | WB_FLATVALUESET | WB_NOBORDER | WB_NO_DIRECTSELECT );
    mpLightingSet->SetHelpId( HID_VALUESET_EXTRUSION_LIGHTING );

    mpLightingSet->SetHelpId( HID_POPUP_LINEEND_CTRL );
    mpLightingSet->SetSelectHdl( LINK( this, ExtrusionLightingWindow, SelectHdl ) );
    mpLightingSet->SetColCount( 3 );
    mpLightingSet->EnableFullItemMode( FALSE );

    bool bHighContrast = GetDisplayBackground().GetColor().IsDark();

    for( i = FROM_TOP_LEFT; i <= FROM_BOTTOM_RIGHT; i++ )
    {
        if( i != FROM_FRONT )
        {
            mpLightingSet->InsertItem( i+1, bHighContrast ? maImgLightingOffh[i] : maImgLightingOff[i] );
        }
        else
        {
            mpLightingSet->InsertItem( 5, bHighContrast ? maImgLightingPreviewh[FROM_FRONT] : maImgLightingPreview[FROM_FRONT] );
        }
    }
    mpLightingSet->SetOutputSizePixel( Size( 72, 72 ) );

    mpMenu->appendEntry( 3, mpLightingSet );
    mpMenu->appendSeparator();
    mpMenu->appendEntry( 0, String( SVX_RES( STR_BRIGHT ) ), bHighContrast ? maImgBrighth : maImgBright );
    mpMenu->appendEntry( 1, String( SVX_RES( STR_NORMAL ) ), bHighContrast ? maImgNormalh : maImgNormal );
    mpMenu->appendEntry( 2, String( SVX_RES( STR_DIM ) ), bHighContrast ? maImgDimh : maImgDim );

    SetOutputSizePixel( mpMenu->getMenuSize() );
    mpMenu->SetOutputSizePixel( GetOutputSizePixel() );

    mpMenu->Show();

    FreeResource();

    AddStatusListener( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:ExtrusionLightingDirection" )));
    AddStatusListener( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:ExtrusionLightingIntensity" )));
}

// -----------------------------------------------------------------------

SfxPopupWindow* ExtrusionLightingWindow::Clone() const
{
    return new ExtrusionLightingWindow( GetId(), mxFrame );
}

// -----------------------------------------------------------------------

ExtrusionLightingWindow::~ExtrusionLightingWindow()
{
    delete mpMenu;
}

// -----------------------------------------------------------------------

void ExtrusionLightingWindow::implSetIntensity( int nLevel, bool bEnabled )
{
    mnLevel = nLevel;
    mbLevelEnabled = bEnabled;
    int i = 0;
    for( i = 0; i < 3; i++ )
    {
        mpMenu->checkEntry( i, (i == nLevel) && bEnabled );
        mpMenu->enableEntry( i, bEnabled );
    }
}

// -----------------------------------------------------------------------

void ExtrusionLightingWindow::implSetDirection( int nDirection, bool bEnabled )
{
    mnDirection = nDirection;
    mbDirectionEnabled = bEnabled;

    bool bHighContrast = GetDisplayBackground().GetColor().IsDark();

    if( !bEnabled )
        nDirection = FROM_FRONT;

    USHORT nItemId;
    for( nItemId = FROM_TOP_LEFT; nItemId <= FROM_BOTTOM_RIGHT; nItemId++ )
    {
        if( nItemId == FROM_FRONT )
        {
            mpLightingSet->SetItemImage( nItemId + 1, bHighContrast ? maImgLightingPreviewh[ nDirection ] : maImgLightingPreview[ nDirection ] );
        }
        else
        {
            if( bHighContrast )
            {
                mpLightingSet->SetItemImage( nItemId + 1, (USHORT)nDirection == nItemId ? maImgLightingOnh[nItemId] : maImgLightingOffh[nItemId] );
            }
            else
            {
                mpLightingSet->SetItemImage( nItemId + 1, (USHORT)nDirection == nItemId ? maImgLightingOn[nItemId] : maImgLightingOff[nItemId] );
            }
        }
    }

    mpMenu->enableEntry( 3, bEnabled );
}

// -----------------------------------------------------------------------

void ExtrusionLightingWindow::StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    switch( nSID )
    {
        case SID_EXTRUSION_LIGHTING_INTENSITY:
        {
            if( eState == SFX_ITEM_DISABLED )
            {
                implSetIntensity( 0, false );
            }
            else
            {
                const SfxInt32Item* pStateItem = PTR_CAST( SfxInt32Item, pState );
                if( pStateItem )
                    implSetIntensity( pStateItem->GetValue(), true );
            }
            break;
        }
        case SID_EXTRUSION_LIGHTING_DIRECTION:
        {
            if( eState == SFX_ITEM_DISABLED )
            {
                implSetDirection( 0, false );
            }
            else
            {
                const SfxInt32Item* pStateItem = PTR_CAST( SfxInt32Item, pState );
                if( pStateItem )
                    implSetDirection( pStateItem->GetValue(), true );
            }
            break;
        }
    }
}

// -----------------------------------------------------------------------

void ExtrusionLightingWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxPopupWindow::DataChanged( rDCEvt );

    if( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
    {
        bool bHighContrast = GetDisplayBackground().GetColor().IsDark();

        implSetDirection( mnDirection, mbDirectionEnabled );
        mpMenu->setEntryImage( 0, bHighContrast ? maImgBrighth : maImgBright );
        mpMenu->setEntryImage( 1, bHighContrast ? maImgNormalh : maImgNormal );
        mpMenu->setEntryImage( 2, bHighContrast ? maImgDimh : maImgDim );
    }
}

// -----------------------------------------------------------------------

IMPL_LINK( ExtrusionLightingWindow, SelectHdl, void *, pControl )
{
    if ( IsInPopupMode() )
        EndPopupMode();

//  SfxDispatcher* pDisp = GetBindings().GetDispatcher();

    if( pControl == mpMenu )
    {
        int nLevel = mpMenu->getSelectedEntryId();
        if( nLevel >= 0 )
        {
            if( nLevel != 3 )
            {
                SfxInt32Item    aItem( SID_EXTRUSION_LIGHTING_INTENSITY, nLevel );
                rtl::OUString   aCommand( RTL_CONSTASCII_USTRINGPARAM( ".uno:ExtrusionLightingIntensity" ));

                Any a;
                INetURLObject aObj( aCommand );
                Sequence< PropertyValue > aArgs( 1 );
                aArgs[0].Name = aObj.GetURLPath();
                aItem.QueryValue( a );
                aArgs[0].Value = a;

                SfxToolBoxControl::Dispatch( Reference< ::com::sun::star::frame::XDispatchProvider >(
                                             mxFrame->getController(), UNO_QUERY ),
                                             aCommand,
                                             aArgs );

//              pDisp->Execute( SID_EXTRUSION_LIGHTING_INTENSITY, SFX_CALLMODE_RECORD, &aItem, 0L , 0L );
                implSetIntensity( nLevel, true );
            }
        }
    }
    else
    {
        sal_Int32 nDirection = mpLightingSet->GetSelectItemId();

        if( (nDirection > 0) && (nDirection < 10) )
        {
            nDirection--;

            SfxInt32Item    aItem( SID_EXTRUSION_LIGHTING_DIRECTION, nDirection );
            rtl::OUString   aCommand( RTL_CONSTASCII_USTRINGPARAM( ".uno:ExtrusionLightingDirection" ));

            Any a;
            INetURLObject aObj( aCommand );
            Sequence< PropertyValue > aArgs( 1 );
            aArgs[0].Name = aObj.GetURLPath();
            aItem.QueryValue( a );
            aArgs[0].Value = a;

            SfxToolBoxControl::Dispatch( Reference< ::com::sun::star::frame::XDispatchProvider >(
                                         mxFrame->getController(), UNO_QUERY ),
                                         aCommand,
                                         aArgs );
//          pDisp->Execute( SID_EXTRUSION_LIGHTING_DIRECTION, SFX_CALLMODE_RECORD, &aItem, 0L , 0L );

            implSetDirection( nDirection, true );
        }

    }

    return 0;
}

// -----------------------------------------------------------------------

void ExtrusionLightingWindow::StartSelection()
{
}

// -----------------------------------------------------------------------

BOOL ExtrusionLightingWindow::Close()
{
    return SfxPopupWindow::Close();
}

// -----------------------------------------------------------------------

void ExtrusionLightingWindow::PopupModeEnd()
{
    if ( IsVisible() )
    {
        mbPopupMode = FALSE;
    }
    SfxPopupWindow::PopupModeEnd();
}

// -----------------------------------------------------------------------

void ExtrusionLightingWindow::GetFocus (void)
{
    SfxPopupWindow::GetFocus();
    // Grab the focus to the line ends value set so that it can be controlled
    // with the keyboard.
    if( mpMenu )
        mpMenu->GrabFocus();
}

// ========================================================================

ExtrusionLightingControl::ExtrusionLightingControl( USHORT nSlotId, USHORT nId, ToolBox &rTbx )
: SfxToolBoxControl( nSlotId, nId, rTbx )
{
    rTbx.SetItemBits( nId, TIB_DROPDOWNONLY | rTbx.GetItemBits( nId ) );
}

// -----------------------------------------------------------------------

ExtrusionLightingControl::~ExtrusionLightingControl()
{
}

// -----------------------------------------------------------------------

SfxPopupWindowType ExtrusionLightingControl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONCLICK;
}

// -----------------------------------------------------------------------

SfxPopupWindow* ExtrusionLightingControl::CreatePopupWindow()
{
    ExtrusionLightingWindow* pWin = new ExtrusionLightingWindow( GetId(), m_xFrame, &GetToolBox() );
    pWin->StartPopupMode( &GetToolBox(), TRUE );
    pWin->StartSelection();
    SetPopupWindow( pWin );
    return pWin;
}

// -----------------------------------------------------------------------

void ExtrusionLightingControl::StateChanged( USHORT, SfxItemState eState, const SfxPoolItem* )
{
    USHORT nId = GetId();
    ToolBox& rTbx = GetToolBox();

    rTbx.EnableItem( nId, SFX_ITEM_DISABLED != eState );
    rTbx.SetItemState( nId, ( SFX_ITEM_DONTCARE == eState ) ? STATE_DONTKNOW : STATE_NOCHECK );
}

// ####################################################################

SFX_IMPL_TOOLBOX_CONTROL( ExtrusionSurfaceControl, SfxBoolItem );

ExtrusionSurfaceWindow::ExtrusionSurfaceWindow(
    USHORT nId,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame ) :

    SfxPopupWindow( nId,
                    rFrame,
                    SVX_RES( RID_SVXFLOAT_EXTRUSION_SURFACE )),
    maImgSurface1( SVX_RES( IMG_WIRE_FRAME ) ),
    maImgSurface2( SVX_RES( IMG_MATTE ) ),
    maImgSurface3( SVX_RES( IMG_PLASTIC ) ),
    maImgSurface4( SVX_RES( IMG_METAL ) ),
    maImgSurface1h( SVX_RES( IMG_WIRE_FRAME_H ) ),
    maImgSurface2h( SVX_RES( IMG_MATTE_H ) ),
    maImgSurface3h( SVX_RES( IMG_PLASTIC_H ) ),
    maImgSurface4h( SVX_RES( IMG_METAL_H ) ),
    mxFrame( rFrame ),
    mbPopupMode( true )
{
    implInit();
}

ExtrusionSurfaceWindow::ExtrusionSurfaceWindow(
    USHORT nId,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
    Window* pParentWindow ) :

    SfxPopupWindow( nId,
                    rFrame,
                    pParentWindow,
                    SVX_RES( RID_SVXFLOAT_EXTRUSION_SURFACE )),
    maImgSurface1( SVX_RES( IMG_WIRE_FRAME ) ),
    maImgSurface2( SVX_RES( IMG_MATTE ) ),
    maImgSurface3( SVX_RES( IMG_PLASTIC ) ),
    maImgSurface4( SVX_RES( IMG_METAL ) ),
    maImgSurface1h( SVX_RES( IMG_WIRE_FRAME_H ) ),
    maImgSurface2h( SVX_RES( IMG_MATTE_H ) ),
    maImgSurface3h( SVX_RES( IMG_PLASTIC_H ) ),
    maImgSurface4h( SVX_RES( IMG_METAL_H ) ),
    mxFrame( rFrame ),
    mbPopupMode( true )
{
    implInit();
}

// -----------------------------------------------------------------------

void ExtrusionSurfaceWindow::implInit()
{
    SetHelpId( HID_POPUP_EXTRUSION_SURFACE );

    bool bHighContrast = GetDisplayBackground().GetColor().IsDark();

//  mpSurfaceForewarder = new SfxStatusForwarder( SID_EXTRUSION_SURFACE, *this );

    mpMenu = new ToolbarMenu( this, WB_CLIPCHILDREN );
    mpMenu->SetHelpId( HID_MENU_EXTRUSION_SURFACE );
    mpMenu->SetSelectHdl( LINK( this, ExtrusionSurfaceWindow, SelectHdl ) );

    mpMenu->appendEntry( 0, String( SVX_RES( STR_WIREFRAME ) ), bHighContrast ? maImgSurface1h : maImgSurface1 );
    mpMenu->appendEntry( 1, String( SVX_RES( STR_MATTE ) ), bHighContrast ? maImgSurface2h : maImgSurface2 );
    mpMenu->appendEntry( 2, String( SVX_RES( STR_PLASTIC ) ), bHighContrast ? maImgSurface3h : maImgSurface3 );
    mpMenu->appendEntry( 3, String( SVX_RES( STR_METAL ) ), bHighContrast ? maImgSurface4h : maImgSurface4 );

    SetOutputSizePixel( mpMenu->getMenuSize() );
    mpMenu->SetOutputSizePixel( GetOutputSizePixel() );

    mpMenu->Show();

    FreeResource();

    AddStatusListener( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:ExtrusionSurface" )));
//  rBindings.Update( SID_EXTRUSION_SURFACE );
}

// -----------------------------------------------------------------------

SfxPopupWindow* ExtrusionSurfaceWindow::Clone() const
{
    return new ExtrusionSurfaceWindow( GetId(), mxFrame );
}

// -----------------------------------------------------------------------

ExtrusionSurfaceWindow::~ExtrusionSurfaceWindow()
{
//  delete mpSurfaceForewarder;
    delete mpMenu;
}

// -----------------------------------------------------------------------

void ExtrusionSurfaceWindow::implSetSurface( int nSurface, bool bEnabled )
{
    if( mpMenu )
    {
        int i;
        for( i = 0; i < 4; i++ )
        {
            mpMenu->checkEntry( i, (i == nSurface) && bEnabled );
            mpMenu->enableEntry( i, bEnabled );
        }
    }
}

// -----------------------------------------------------------------------

void ExtrusionSurfaceWindow::StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    switch( nSID )
    {
        case SID_EXTRUSION_SURFACE:
        {
            if( eState == SFX_ITEM_DISABLED )
            {
                implSetSurface( 0, false );
            }
            else
            {
                const SfxInt32Item* pStateItem = PTR_CAST( SfxInt32Item, pState );
                if( pStateItem )
                    implSetSurface( pStateItem->GetValue(), true );
            }
            break;
        }
    }
}

// -----------------------------------------------------------------------

void ExtrusionSurfaceWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxPopupWindow::DataChanged( rDCEvt );

    if( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
    {
        bool bHighContrast = GetDisplayBackground().GetColor().IsDark();

        mpMenu->appendEntry( 0, String( SVX_RES( STR_WIREFRAME ) ), bHighContrast ? maImgSurface1h : maImgSurface1 );
        mpMenu->appendEntry( 1, String( SVX_RES( STR_MATTE ) ), bHighContrast ? maImgSurface2h : maImgSurface2 );
        mpMenu->appendEntry( 2, String( SVX_RES( STR_PLASTIC ) ), bHighContrast ? maImgSurface3h : maImgSurface3 );
        mpMenu->appendEntry( 3, String( SVX_RES( STR_METAL ) ), bHighContrast ? maImgSurface4h : maImgSurface4 );
    }
}

// -----------------------------------------------------------------------

IMPL_LINK( ExtrusionSurfaceWindow, SelectHdl, void *, EMPTYARG )
{
    if ( IsInPopupMode() )
        EndPopupMode();

//  SfxDispatcher* pDisp = GetBindings().GetDispatcher();

    sal_Int32 nSurface = mpMenu->getSelectedEntryId();
    if( nSurface >= 0 )
    {
        SfxInt32Item    aItem( SID_EXTRUSION_SURFACE, nSurface );
        rtl::OUString   aCommand( RTL_CONSTASCII_USTRINGPARAM( ".uno:ExtrusionSurface" ));

        Any a;
        INetURLObject aObj( aCommand );
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = aObj.GetURLPath();
        aItem.QueryValue( a );
        aArgs[0].Value = a;

        SfxToolBoxControl::Dispatch( Reference< ::com::sun::star::frame::XDispatchProvider >(
                                     mxFrame->getController(), UNO_QUERY ),
                                     aCommand,
                                     aArgs );
//      pDisp->Execute( SID_EXTRUSION_SURFACE, SFX_CALLMODE_RECORD, &aItem, 0L , 0L );

        implSetSurface( nSurface, true );
    }

    return 0;
}

// -----------------------------------------------------------------------

void ExtrusionSurfaceWindow::StartSelection()
{
}

// -----------------------------------------------------------------------

BOOL ExtrusionSurfaceWindow::Close()
{
    return SfxPopupWindow::Close();
}

// -----------------------------------------------------------------------

void ExtrusionSurfaceWindow::PopupModeEnd()
{
    if ( IsVisible() )
    {
        mbPopupMode = FALSE;
    }
    SfxPopupWindow::PopupModeEnd();
}

// -----------------------------------------------------------------------

void ExtrusionSurfaceWindow::GetFocus (void)
{
    SfxPopupWindow::GetFocus();
    // Grab the focus to the line ends value set so that it can be controlled
    // with the keyboard.
    if( mpMenu )
        mpMenu->GrabFocus();
}

// ========================================================================

ExtrusionSurfaceControl::ExtrusionSurfaceControl(
    USHORT nSlotId, USHORT nId, ToolBox &rTbx )
: SfxToolBoxControl( nSlotId, nId, rTbx )
{
    rTbx.SetItemBits( nId, TIB_DROPDOWNONLY | rTbx.GetItemBits( nId ) );
}

// -----------------------------------------------------------------------

ExtrusionSurfaceControl::~ExtrusionSurfaceControl()
{
}

// -----------------------------------------------------------------------

SfxPopupWindowType ExtrusionSurfaceControl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONCLICK;
}

// -----------------------------------------------------------------------

SfxPopupWindow* ExtrusionSurfaceControl::CreatePopupWindow()
{
    ExtrusionSurfaceWindow* pWin = new ExtrusionSurfaceWindow( GetId(), m_xFrame, &GetToolBox() );
    pWin->StartPopupMode( &GetToolBox(), TRUE );
    pWin->StartSelection();
    SetPopupWindow( pWin );
    return pWin;
}

// -----------------------------------------------------------------------

void ExtrusionSurfaceControl::StateChanged( USHORT, SfxItemState eState, const SfxPoolItem* )
{
    USHORT nId = GetId();
    ToolBox& rTbx = GetToolBox();

    rTbx.EnableItem( nId, SFX_ITEM_DISABLED != eState );
    rTbx.SetItemState( nId, ( SFX_ITEM_DONTCARE == eState ) ? STATE_DONTKNOW : STATE_NOCHECK );
}

//========================================================================

SFX_IMPL_TOOLBOX_CONTROL( ExtrusionColorControl, SvxColorItem );

ExtrusionColorControl::ExtrusionColorControl(
    USHORT nSlotId, USHORT nId, ToolBox& rTbx )
: SfxToolBoxControl ( nSlotId, nId, rTbx )
{
    rTbx.SetItemBits( nId, TIB_DROPDOWNONLY | rTbx.GetItemBits( nId ) );
    mpBtnUpdater = new ToolboxButtonColorUpdater( nSlotId, nId, &GetToolBox(), TBX_UPDATER_MODE_CHAR_COLOR_NEW );
}

// -----------------------------------------------------------------------

ExtrusionColorControl::~ExtrusionColorControl()
{
    delete mpBtnUpdater;
}

// -----------------------------------------------------------------------

SfxPopupWindowType ExtrusionColorControl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONCLICK;
}

// -----------------------------------------------------------------------

SfxPopupWindow* ExtrusionColorControl::CreatePopupWindow()
{
    SvxColorWindow_Impl* pColorWin = new SvxColorWindow_Impl(
        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:Extrusion3DColor" )),
        SID_EXTRUSION_3D_COLOR,
        m_xFrame,
        SVX_RESSTR( RID_SVXSTR_EXTRUSION_COLOR ),
        &GetToolBox() );
    pColorWin->StartPopupMode( &GetToolBox(), FLOATWIN_POPUPMODE_GRABFOCUS|FLOATWIN_POPUPMODE_ALLOWTEAROFF );
    pColorWin->StartSelection();
    SetPopupWindow( pColorWin );
    return pColorWin;
}

// -----------------------------------------------------------------------

void ExtrusionColorControl::StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    USHORT nId = GetId();
    ToolBox& rTbx = GetToolBox();

    if( nSID == SID_EXTRUSION_3D_COLOR )
    {
        const SvxColorItem* pItem = 0;

        if( SFX_ITEM_DONTCARE != eState )
            pItem = PTR_CAST( SvxColorItem, pState );

        if ( pItem )
            mpBtnUpdater->Update( pItem->GetValue());
    }

    rTbx.EnableItem( nId, SFX_ITEM_DISABLED != eState );
    rTbx.SetItemState( nId, ( SFX_ITEM_DONTCARE == eState ) ? STATE_DONTKNOW : STATE_NOCHECK );
}
