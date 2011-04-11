/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <string>

#include <vcl/toolbox.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>

#include <svx/dialogs.hrc>
#include "helpid.hrc"

#include "svx/drawitem.hxx"
#include "svx/xattr.hxx"
#include <svx/xtable.hxx>
#include "svx/linectrl.hxx"
#include <svx/itemwin.hxx>
#include <svx/dialmgr.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;

// Fuer Linienenden-Controller
#define MAX_LINES 12

// STATIC DATA -----------------------------------------------------------

#define RESIZE_VALUE_POPUP(value_set)   \
{                                                       \
    Size aSize = GetOutputSizePixel();                  \
    aSize.Width()  -= 4;                                \
    aSize.Height() -= 4;                                \
    (value_set).SetPosSizePixel( Point(2,2), aSize );   \
}

#define CALCSIZE_VALUE_POPUP(value_set,item_size) \
{                                                                   \
    Size aSize = (value_set).CalcWindowSizePixel( (item_size) );    \
    aSize.Width()  += 4;                                            \
    aSize.Height() += 4;                                            \
    SetOutputSizePixel( aSize );                                    \
}


SFX_IMPL_TOOLBOX_CONTROL( SvxLineStyleToolBoxControl, XLineStyleItem );
SFX_IMPL_TOOLBOX_CONTROL( SvxLineWidthToolBoxControl, XLineWidthItem );
SFX_IMPL_TOOLBOX_CONTROL( SvxLineColorToolBoxControl, XLineColorItem );
SFX_IMPL_TOOLBOX_CONTROL( SvxLineEndToolBoxControl,   SfxBoolItem );

/*************************************************************************
|*
|* SvxLineStyleToolBoxControl
|*
\************************************************************************/

SvxLineStyleToolBoxControl::SvxLineStyleToolBoxControl( sal_uInt16 nSlotId,
                                                        sal_uInt16 nId,
                                                        ToolBox& rTbx ) :
    SfxToolBoxControl( nSlotId, nId, rTbx ),
    pStyleItem      ( NULL ),
    pDashItem       ( NULL ),
    bUpdate         ( sal_False )
{
    addStatusListener( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:LineDash" )));
    addStatusListener( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:DashListState" )));
}

//========================================================================

SvxLineStyleToolBoxControl::~SvxLineStyleToolBoxControl()
{
    delete pStyleItem;
    delete pDashItem;
}

//========================================================================

void SvxLineStyleToolBoxControl::StateChanged (

    sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )

{
    SvxLineBox* pBox = (SvxLineBox*)GetToolBox().GetItemWindow( GetId() );
    DBG_ASSERT( pBox, "Window not found!" );

    if( eState == SFX_ITEM_DISABLED )
    {
        pBox->Disable();
        pBox->SetNoSelection();
    }
    else
    {
        pBox->Enable();

        if ( eState == SFX_ITEM_AVAILABLE )
        {
            if( nSID == SID_ATTR_LINE_STYLE )
            {
                delete pStyleItem;
                pStyleItem = (XLineStyleItem*)pState->Clone();
            }
            else if( nSID == SID_ATTR_LINE_DASH )
            {
                delete pDashItem;
                pDashItem = (XLineDashItem*)pState->Clone();
            }

            bUpdate = sal_True;
            Update( pState );
        }
        else if ( nSID != SID_DASH_LIST )
        {
            // kein oder uneindeutiger Status
            pBox->SetNoSelection();
        }
    }
}

//========================================================================

void SvxLineStyleToolBoxControl::Update( const SfxPoolItem* pState )
{
    if ( pState && bUpdate )
    {
        bUpdate = sal_False;

        SvxLineBox* pBox = (SvxLineBox*)GetToolBox().GetItemWindow( GetId() );
        DBG_ASSERT( pBox, "Window not found!" );

        // Da der Timer unerwartet zuschlagen kann, kann es vorkommen, dass
        // die LB noch nicht gefuellt ist. Ein ClearCache() am Control im
        // DelayHdl() blieb ohne Erfolg.
        if( pBox->GetEntryCount() == 0 )
            pBox->FillControl();

        XLineStyle eXLS;

        if ( pStyleItem )
            eXLS = ( XLineStyle )pStyleItem->GetValue();
        else
            eXLS = XLINE_NONE;

        switch( eXLS )
        {
            case XLINE_NONE:
                pBox->SelectEntryPos( 0 );
                break;

            case XLINE_SOLID:
                pBox->SelectEntryPos( 1 );
                break;

            case XLINE_DASH:
            {
                if( pDashItem )
                {
                    String aString( pDashItem->GetName() );
                    pBox->SelectEntry( aString );
                }
                else
                    pBox->SetNoSelection();
            }
            break;

            default:
                OSL_FAIL( "Nicht unterstuetzter Linientyp" );
                break;
        }
    }

    if ( pState && ( pState->ISA( SvxDashListItem ) ) )
    {
        // Die Liste der Linienstile hat sich geaendert
        SvxLineBox* pBox = (SvxLineBox*)GetToolBox().GetItemWindow( GetId() );
        DBG_ASSERT( pBox, "Window not found!" );

        String aString( pBox->GetSelectEntry() );
        pBox->Clear();
        pBox->InsertEntry( SVX_RESSTR(RID_SVXSTR_INVISIBLE) );
        pBox->InsertEntry( SVX_RESSTR(RID_SVXSTR_SOLID) );
        pBox->Fill( ((SvxDashListItem*)pState )->GetDashList() );
        pBox->SelectEntry( aString );
    }
}

//========================================================================

Window* SvxLineStyleToolBoxControl::CreateItemWindow( Window *pParent )
{
    return new SvxLineBox( pParent, m_xFrame );
}

/*************************************************************************
|*
|* SvxLineWidthToolBoxControl
|*
\************************************************************************/

SvxLineWidthToolBoxControl::SvxLineWidthToolBoxControl(
    sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :
    SfxToolBoxControl( nSlotId, nId, rTbx )
{
    addStatusListener( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:MetricUnit" )));
}

//========================================================================

SvxLineWidthToolBoxControl::~SvxLineWidthToolBoxControl()
{
}

//========================================================================

void SvxLineWidthToolBoxControl::StateChanged(
    sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    SvxMetricField* pFld = (SvxMetricField*)
                           GetToolBox().GetItemWindow( GetId() );
    DBG_ASSERT( pFld, "Window not found" );

    if ( nSID == SID_ATTR_METRIC )
    {
        pFld->RefreshDlgUnit();
    }
    else
    {
        if ( eState == SFX_ITEM_DISABLED )
        {
            pFld->Disable();
            pFld->SetText( String() );
        }
        else
        {
            pFld->Enable();

            if ( eState == SFX_ITEM_AVAILABLE )
            {
                DBG_ASSERT( pState->ISA(XLineWidthItem), "falscher ItemType" );

                // Core-Unit an MetricField uebergeben
                // Darf nicht in CreateItemWin() geschehen!
                SfxMapUnit eUnit = SFX_MAPUNIT_100TH_MM; // CD!!! GetCoreMetric();
                pFld->SetCoreUnit( eUnit );

                pFld->Update( (const XLineWidthItem*)pState );
            }
            else
                pFld->Update( NULL );
        }
    }
}

//========================================================================

Window* SvxLineWidthToolBoxControl::CreateItemWindow( Window *pParent )
{
    return( new SvxMetricField( pParent, m_xFrame ) );
}

/*************************************************************************
|*
|* SvxLineColorToolBoxControl
|*
\************************************************************************/

SvxLineColorToolBoxControl::SvxLineColorToolBoxControl(
    sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :
    SfxToolBoxControl( nSlotId, nId, rTbx )
{
    addStatusListener( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:ColorTableState" )));
}

//========================================================================

SvxLineColorToolBoxControl::~SvxLineColorToolBoxControl()
{
}

//========================================================================

void SvxLineColorToolBoxControl::StateChanged(

    sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )

{
    SvxColorBox* pBox = (SvxColorBox*)GetToolBox().GetItemWindow( GetId() );
    DBG_ASSERT( pBox, "Window not found" );

    if ( nSID != SID_COLOR_TABLE )
    {
        if ( eState == SFX_ITEM_DISABLED )
        {
            pBox->Disable();
            pBox->SetNoSelection();
        }
        else
        {
            pBox->Enable();

            if ( eState == SFX_ITEM_AVAILABLE )
            {
                DBG_ASSERT( pState->ISA(XLineColorItem), "falscher ItemTyoe" );
                pBox->Update( (const XLineColorItem*) pState );
            }
            else
                pBox->Update( NULL );
        }
    }
    else
        Update( pState );
}

//========================================================================

void SvxLineColorToolBoxControl::Update( const SfxPoolItem* pState )
{
    if ( pState && ( pState->ISA( SvxColorTableItem ) ) )
    {
        SvxColorBox* pBox = (SvxColorBox*)GetToolBox().GetItemWindow( GetId() );

        DBG_ASSERT( pBox, "Window not found" );

        // Die Liste der Farben (ColorTable) hat sich geaendert:
        ::Color aTmpColor( pBox->GetSelectEntryColor() );
        pBox->Clear();
        pBox->Fill( ( (SvxColorTableItem*)pState )->GetColorTable() );
        pBox->SelectEntry( aTmpColor );
    }
}

//========================================================================

Window* SvxLineColorToolBoxControl::CreateItemWindow( Window *pParent )
{
    return new SvxColorBox( pParent, m_aCommandURL, m_xFrame );
}

/*************************************************************************
|*
|* SvxLineEndWindow
|*
\************************************************************************/

SvxLineEndWindow::SvxLineEndWindow(
    sal_uInt16 nSlotId,
    const Reference< XFrame >& rFrame,
    const String& rWndTitle ) :
    SfxPopupWindow( nSlotId,
                    rFrame,
                    WinBits( WB_STDPOPUP ) ),
    pLineEndList    ( NULL ),
    aLineEndSet     ( this, WinBits( WB_ITEMBORDER | WB_3DLOOK | WB_NO_DIRECTSELECT ) ),
    nCols           ( 2 ),
    nLines          ( 12 ),
    nLineEndWidth   ( 400 ),
    bPopupMode      ( sal_True ),
    mbInResize      ( false ),
    mxFrame         ( rFrame )
{
    SetText( rWndTitle );
    implInit();
}

SvxLineEndWindow::SvxLineEndWindow(
    sal_uInt16 nSlotId,
    const Reference< XFrame >& rFrame,
    Window* pParentWindow,
    const String& rWndTitle ) :
    SfxPopupWindow( nSlotId,
                    rFrame,
                    pParentWindow,
                    WinBits( WB_STDPOPUP ) ),
    pLineEndList    ( NULL ),
    aLineEndSet     ( this, WinBits( WB_ITEMBORDER | WB_3DLOOK | WB_NO_DIRECTSELECT ) ),
    nCols           ( 2 ),
    nLines          ( 12 ),
    nLineEndWidth   ( 400 ),
    bPopupMode      ( sal_True ),
    mbInResize      ( false ),
    mxFrame         ( rFrame )
{
    SetText( rWndTitle );
    implInit();
}

void SvxLineEndWindow::implInit()
{
    SfxObjectShell*     pDocSh  = SfxObjectShell::Current();
    const SfxPoolItem*  pItem   = NULL;

    SetHelpId( HID_POPUP_LINEEND );
    aLineEndSet.SetHelpId( HID_POPUP_LINEEND_CTRL );

    if ( pDocSh )
    {
        pItem = pDocSh->GetItem( SID_LINEEND_LIST );
        if( pItem )
            pLineEndList = ( (SvxLineEndListItem*) pItem )->GetLineEndList();

        pItem = pDocSh->GetItem( SID_ATTR_LINEEND_WIDTH_DEFAULT );
        if( pItem )
            nLineEndWidth = ( (SfxUInt16Item*) pItem )->GetValue();
    }
    DBG_ASSERT( pLineEndList, "LineEndList wurde nicht gefunden" );

    aLineEndSet.SetSelectHdl( LINK( this, SvxLineEndWindow, SelectHdl ) );
    aLineEndSet.SetColCount( nCols );

    // ValueSet mit Eintraegen der LineEndList fuellen
    FillValueSet();

    AddStatusListener( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:LineEndListState" )));

    //ChangeHelpId( HID_POPUP_LINEENDSTYLE );
    aLineEndSet.Show();
}

SfxPopupWindow* SvxLineEndWindow::Clone() const
{
    return new SvxLineEndWindow( GetId(), mxFrame, GetText() );
}

// -----------------------------------------------------------------------

SvxLineEndWindow::~SvxLineEndWindow()
{
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxLineEndWindow, SelectHdl, void *, EMPTYARG )
{
    XLineEndItem*           pLineEndItem = NULL;
    XLineStartItem*         pLineStartItem = NULL;
    sal_uInt16                  nId = aLineEndSet.GetSelectItemId();

    if( nId == 1 )
    {
        pLineStartItem  = new XLineStartItem();
    }
    else if( nId == 2 )
    {
        pLineEndItem    = new XLineEndItem();
    }
    else if( nId % 2 ) // LinienAnfang
    {
        XLineEndEntry* pEntry = pLineEndList->GetLineEnd( ( nId - 1 ) / 2 - 1 );
        pLineStartItem  = new XLineStartItem( pEntry->GetName(), pEntry->GetLineEnd() );
    }
    else // LinienEnde
    {
        XLineEndEntry* pEntry = pLineEndList->GetLineEnd( nId / 2 - 2 );
        pLineEndItem    = new XLineEndItem( pEntry->GetName(), pEntry->GetLineEnd() );
    }

    if ( IsInPopupMode() )
        EndPopupMode();

    Sequence< PropertyValue > aArgs( 1 );
    Any a;

    if ( pLineStartItem )
    {
        aArgs[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LineStart" ));
        pLineStartItem->QueryValue( a );
        aArgs[0].Value = a;
    }
    else
    {
        aArgs[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LineEnd" ));
        pLineEndItem->QueryValue( a );
        aArgs[0].Value = a;
    }

    /*  #i33380# DR 2004-09-03 Moved the following line above the Dispatch() call.
        This instance may be deleted in the meantime (i.e. when a dialog is opened
        while in Dispatch()), accessing members will crash in this case. */
    aLineEndSet.SetNoSelection();

    SfxToolBoxControl::Dispatch( Reference< XDispatchProvider >( mxFrame->getController(), UNO_QUERY ),
                                 ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:LineEndStyle" )),
                                 aArgs );

    delete pLineEndItem;
    delete pLineStartItem;

    return 0;
}

// -----------------------------------------------------------------------

void SvxLineEndWindow::FillValueSet()
{
    if( pLineEndList )
    {
        XLineEndEntry*      pEntry  = NULL;
        Bitmap*             pBmp    = NULL;
        VirtualDevice       aVD;

        long nCount = pLineEndList->Count();

        // Erster Eintrag: kein LinienEnde
        // Temporaer wird ein Eintrag hinzugefuegt, um die UI-Bitmap zu erhalten
        basegfx::B2DPolyPolygon aNothing;
        pLineEndList->Insert( new XLineEndEntry( aNothing, SVX_RESSTR( RID_SVXSTR_NONE ) ) );
        pEntry = pLineEndList->GetLineEnd( nCount );
        pBmp = pLineEndList->GetBitmap( nCount );
        DBG_ASSERT( pBmp, "UI-Bitmap wurde nicht erzeugt" );

        aBmpSize = pBmp->GetSizePixel();
        aVD.SetOutputSizePixel( aBmpSize, sal_False );
        aBmpSize.Width() = aBmpSize.Width() / 2;
        Point aPt0( 0, 0 );
        Point aPt1( aBmpSize.Width(), 0 );

        aVD.DrawBitmap( Point(), *pBmp );
        aLineEndSet.InsertItem( 1, aVD.GetBitmap( aPt0, aBmpSize ), pEntry->GetName() );
        aLineEndSet.InsertItem( 2, aVD.GetBitmap( aPt1, aBmpSize ), pEntry->GetName() );

        delete pLineEndList->Remove( nCount );

        for( long i = 0; i < nCount; i++ )
        {
            pEntry = pLineEndList->GetLineEnd( i );
            DBG_ASSERT( pEntry, "Konnte auf LineEndEntry nicht zugreifen" );
            pBmp = pLineEndList->GetBitmap( i );
            DBG_ASSERT( pBmp, "UI-Bitmap wurde nicht erzeugt" );

            aVD.DrawBitmap( aPt0, *pBmp );
            aLineEndSet.InsertItem( (sal_uInt16)((i+1L)*2L+1L), aVD.GetBitmap( aPt0, aBmpSize ), pEntry->GetName() );
            aLineEndSet.InsertItem( (sal_uInt16)((i+2L)*2L),    aVD.GetBitmap( aPt1, aBmpSize ), pEntry->GetName() );
        }
        nLines = Min( (sal_uInt16)(nCount + 1), (sal_uInt16) MAX_LINES );
        aLineEndSet.SetLineCount( nLines );

        SetSize();
    }
}

// -----------------------------------------------------------------------

void SvxLineEndWindow::Resize()
{
    // since we change the size inside this call, check if we
    // are called recursive
    if( !mbInResize )
    {
        mbInResize = true;
        if ( !IsRollUp() )
        {
            aLineEndSet.SetColCount( nCols );
            aLineEndSet.SetLineCount( nLines );

            SetSize();

            Size aSize = GetOutputSizePixel();
            aSize.Width()  -= 4;
            aSize.Height() -= 4;
            aLineEndSet.SetPosSizePixel( Point( 2, 2 ), aSize );
        }
        //SfxPopupWindow::Resize();
        mbInResize = false;
    }
}

// -----------------------------------------------------------------------

void SvxLineEndWindow::Resizing( Size& rNewSize )
{
    Size aBitmapSize = aBmpSize; // -> Member
    aBitmapSize.Width()  += 6; //
    aBitmapSize.Height() += 6; //

    Size aItemSize = aLineEndSet.CalcItemSizePixel( aBitmapSize );  // -> Member
    //Size aOldSize = GetOutputSizePixel(); // fuer Breite

    sal_uInt16 nItemCount = aLineEndSet.GetItemCount(); // -> Member

    // Spalten ermitteln
    long nItemW = aItemSize.Width();
    long nW = rNewSize.Width();
    nCols = (sal_uInt16) Max( ( (sal_uIntPtr)(( nW + nItemW ) / ( nItemW * 2 ) )),
                                            (sal_uIntPtr) 1L );
    nCols *= 2;

    // Reihen ermitteln
    long nItemH = aItemSize.Height();
    long nH = rNewSize.Height();
    nLines = (sal_uInt16) Max( ( ( nH + nItemH / 2 ) / nItemH ), 1L );

    sal_uInt16 nMaxCols  = nItemCount / nLines;
    if( nItemCount % nLines )
        nMaxCols++;
    if( nCols > nMaxCols )
        nCols = nMaxCols;
    nW = nItemW * nCols;

    // Keine ungerade Anzahl von Spalten
    if( nCols % 2 )
        nCols--;
    nCols = Max( nCols, (sal_uInt16) 2 );

    sal_uInt16 nMaxLines  = nItemCount / nCols;
    if( nItemCount % nCols )
        nMaxLines++;
    if( nLines > nMaxLines )
        nLines = nMaxLines;
    nH = nItemH * nLines;

    rNewSize.Width() = nW;
    rNewSize.Height() = nH;
}
// -----------------------------------------------------------------------

void SvxLineEndWindow::StartSelection()
{
    aLineEndSet.StartSelection();
}

// -----------------------------------------------------------------------

sal_Bool SvxLineEndWindow::Close()
{
    return SfxPopupWindow::Close();
}

// -----------------------------------------------------------------------

void SvxLineEndWindow::StateChanged(
    sal_uInt16 nSID, SfxItemState, const SfxPoolItem* pState )
{
    if ( nSID == SID_LINEEND_LIST )
    {
        // Die Liste der LinienEnden (LineEndList) hat sich geaendert:
        if ( pState && pState->ISA( SvxLineEndListItem ))
        {
            pLineEndList = ((SvxLineEndListItem*)pState)->GetLineEndList();
            DBG_ASSERT( pLineEndList, "LineEndList nicht gefunden" );

            aLineEndSet.Clear();
            FillValueSet();

            Size aSize = GetOutputSizePixel();
            Resizing( aSize );
            Resize();
        }
    }
}

// -----------------------------------------------------------------------

void SvxLineEndWindow::PopupModeEnd()
{
    if ( IsVisible() )
    {
        bPopupMode = sal_False;
        SetSize();
    }
    SfxPopupWindow::PopupModeEnd();
}

// -----------------------------------------------------------------------

void SvxLineEndWindow::SetSize()
{
    //if( !bPopupMode )
    if( !IsInPopupMode() )
    {
        sal_uInt16 nItemCount = aLineEndSet.GetItemCount(); // -> Member
        sal_uInt16 nMaxLines  = nItemCount / nCols; // -> Member ?
        if( nItemCount % nCols )
            nMaxLines++;

        WinBits nBits = aLineEndSet.GetStyle();
        if ( nLines == nMaxLines )
            nBits &= ~WB_VSCROLL;
        else
            nBits |= WB_VSCROLL;
        aLineEndSet.SetStyle( nBits );
    }

    Size aSize( aBmpSize );
    aSize.Width()  += 6;
    aSize.Height() += 6;
    aSize = aLineEndSet.CalcWindowSizePixel( aSize );
    aSize.Width()  += 4;
    aSize.Height() += 4;
    SetOutputSizePixel( aSize );
    aSize.Height() = aBmpSize.Height();
    aSize.Height() += 14;
    //SetMinOutputSizePixel( aSize );
}

void SvxLineEndWindow::GetFocus (void)
{
    SfxPopupWindow::GetFocus();
    // Grab the focus to the line ends value set so that it can be controlled
    // with the keyboard.
    aLineEndSet.GrabFocus();
}

/*************************************************************************
|*
|* SvxLineEndToolBoxControl
|*
\************************************************************************/

SvxLineEndToolBoxControl::SvxLineEndToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox &rTbx ) :
    SfxToolBoxControl( nSlotId, nId, rTbx )
{
    rTbx.SetItemBits( nId, TIB_DROPDOWNONLY | rTbx.GetItemBits( nId ) );
    rTbx.Invalidate();
}

// -----------------------------------------------------------------------

SvxLineEndToolBoxControl::~SvxLineEndToolBoxControl()
{
}

// -----------------------------------------------------------------------

SfxPopupWindowType SvxLineEndToolBoxControl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONCLICK;
}

// -----------------------------------------------------------------------

SfxPopupWindow* SvxLineEndToolBoxControl::CreatePopupWindow()
{
    SvxLineEndWindow* pLineEndWin =
        new SvxLineEndWindow( GetId(), m_xFrame, &GetToolBox(), SVX_RESSTR( RID_SVXSTR_LINEEND ) );
    pLineEndWin->StartPopupMode( &GetToolBox(), FLOATWIN_POPUPMODE_GRABFOCUS | FLOATWIN_POPUPMODE_ALLOWTEAROFF );
    pLineEndWin->StartSelection();
    SetPopupWindow( pLineEndWin );
    return pLineEndWin;
}

// -----------------------------------------------------------------------

void SvxLineEndToolBoxControl::StateChanged( sal_uInt16, SfxItemState eState, const SfxPoolItem* )
{
    sal_uInt16 nId = GetId();
    ToolBox& rTbx = GetToolBox();

    rTbx.EnableItem( nId, SFX_ITEM_DISABLED != eState );
    rTbx.SetItemState( nId, ( SFX_ITEM_DONTCARE == eState ) ? STATE_DONTKNOW : STATE_NOCHECK );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
