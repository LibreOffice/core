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


#include "tools/rc.h"
#include "tools/debug.hxx"


#include "vcl/decoview.hxx"
#include "vcl/dialog.hxx"
#include "vcl/event.hxx"
#include "vcl/scrbar.hxx"
#include "vcl/button.hxx"
#include "vcl/edit.hxx"
#include "vcl/lstbox.hxx"
#include "vcl/combobox.hxx"

#include "svdata.hxx"
#include "controldata.hxx"
#include "ilstbox.hxx"
#include "dndevdis.hxx"

#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>

// =======================================================================

ListBox::ListBox( WindowType nType ) : Control( nType )
{
    ImplInitListBoxData();
}

// -----------------------------------------------------------------------

ListBox::ListBox( Window* pParent, WinBits nStyle ) : Control( WINDOW_LISTBOX )
{
    ImplInitListBoxData();
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

ListBox::ListBox( Window* pParent, const ResId& rResId ) :
    Control( WINDOW_LISTBOX )
{
    rResId.SetRT( RSC_LISTBOX );
    WinBits nStyle = ImplInitRes( rResId );

    if (VclBuilderContainer::replace_buildable(pParent, rResId, *this))
        return;

    ImplInitListBoxData();
    ImplInit( pParent, nStyle );

    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE ) )
        Show();
}

void ListBox::take_properties(Window &rOther)
{
    if (!GetParent())
    {
        ImplInitListBoxData();
        ImplInit(rOther.GetParent(), rOther.GetStyle());
    }

    Control::take_properties(rOther);

    ListBox &rOtherListBox = static_cast<ListBox&>(rOther);
    mnDDHeight = rOtherListBox.mnDDHeight;
    mnSaveValue = rOtherListBox.mnSaveValue;
    EnableAutoSize(rOtherListBox.mbDDAutoSize);
    SetDropDownLineCount(rOtherListBox.GetDropDownLineCount());
    mpImplLB->take_properties(*rOtherListBox.mpImplLB);
    if (mpImplWin && rOtherListBox.mpImplWin)
        mpImplWin->take_properties(*rOtherListBox.mpImplWin);
}

// -----------------------------------------------------------------------

ListBox::~ListBox()
{
    //#109201#
    ImplCallEventListeners( VCLEVENT_OBJECT_DYING );

    // Beim zerstoeren des FloatWins macht TH ein GrabFocus auf den Parent,
    // also diese ListBox => PreNotify()...
    ImplListBox *pImplLB = mpImplLB;
    mpImplLB = NULL;
    delete pImplLB;

    delete mpFloatWin;
    delete mpImplWin;
    delete mpBtn;
}

// -----------------------------------------------------------------------

void ListBox::ImplInitListBoxData()
{
    mpFloatWin      = NULL;
    mpImplWin       = NULL;
    mpBtn           = NULL;

    mnDDHeight      = 0;
    mbDDAutoSize    = sal_True;
    mnSaveValue     = LISTBOX_ENTRY_NOTFOUND;
    mnLineCount     = 0;
}

// -----------------------------------------------------------------------

void ListBox::ImplInit( Window* pParent, WinBits nStyle )
{
    nStyle = ImplInitStyle( nStyle );
    if ( !(nStyle & WB_NOBORDER) && ( nStyle & WB_DROPDOWN ) )
        nStyle |= WB_BORDER;

    Control::ImplInit( pParent, nStyle, NULL );
    SetBackground();

    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTargetListener> xDrop = new DNDEventDispatcher(this);

    if( nStyle & WB_DROPDOWN )
    {
        sal_Int32 nLeft, nTop, nRight, nBottom;
        GetBorder( nLeft, nTop, nRight, nBottom );
        mnDDHeight = (sal_uInt16)(GetTextHeight() + nTop + nBottom + 4);

        if( IsNativeWidgetEnabled() &&
            IsNativeControlSupported( CTRL_LISTBOX, PART_ENTIRE_CONTROL ) )
        {
                ImplControlValue aControlValue;
                Rectangle aCtrlRegion( Point( 0, 0 ), Size( 20, mnDDHeight ) );
                Rectangle aBoundingRgn( aCtrlRegion );
                Rectangle aContentRgn( aCtrlRegion );
                if( GetNativeControlRegion( CTRL_LISTBOX, PART_ENTIRE_CONTROL, aCtrlRegion,
                                            CTRL_STATE_ENABLED, aControlValue, rtl::OUString(),
                                            aBoundingRgn, aContentRgn ) )
                {
                    sal_Int32 nHeight = aBoundingRgn.GetHeight();
                    if( nHeight > mnDDHeight )
                        mnDDHeight = static_cast<sal_uInt16>(nHeight);
                }
        }

        mpFloatWin = new ImplListBoxFloatingWindow( this );
        mpFloatWin->SetAutoWidth( sal_True );
        mpFloatWin->SetPopupModeEndHdl( LINK( this, ListBox, ImplPopupModeEndHdl ) );
        mpFloatWin->GetDropTarget()->addDropTargetListener(xDrop);

        mpImplWin = new ImplWin( this, (nStyle & (WB_LEFT|WB_RIGHT|WB_CENTER))|WB_NOBORDER );
        mpImplWin->SetMBDownHdl( LINK( this, ListBox, ImplClickBtnHdl ) );
        mpImplWin->SetUserDrawHdl( LINK( this, ListBox, ImplUserDrawHdl ) );
        mpImplWin->Show();
        mpImplWin->GetDropTarget()->addDropTargetListener(xDrop);

        mpBtn = new ImplBtn( this, WB_NOLIGHTBORDER | WB_RECTSTYLE );
        ImplInitDropDownButton( mpBtn );
        mpBtn->SetMBDownHdl( LINK( this, ListBox, ImplClickBtnHdl ) );
        mpBtn->Show();
        mpBtn->GetDropTarget()->addDropTargetListener(xDrop);

    }

    Window* pLBParent = this;
    if ( mpFloatWin )
        pLBParent = mpFloatWin;
    mpImplLB = new ImplListBox( pLBParent, nStyle&(~WB_BORDER) );
    mpImplLB->SetSelectHdl( LINK( this, ListBox, ImplSelectHdl ) );
    mpImplLB->SetScrollHdl( LINK( this, ListBox, ImplScrollHdl ) );
    mpImplLB->SetCancelHdl( LINK( this, ListBox, ImplCancelHdl ) );
    mpImplLB->SetDoubleClickHdl( LINK( this, ListBox, ImplDoubleClickHdl ) );
    mpImplLB->SetUserDrawHdl( LINK( this, ListBox, ImplUserDrawHdl ) );
    mpImplLB->SetPosPixel( Point() );
    mpImplLB->Show();

    mpImplLB->GetDropTarget()->addDropTargetListener(xDrop);
    mpImplLB->SetDropTraget(xDrop);

    if ( mpFloatWin )
    {
        mpFloatWin->SetImplListBox( mpImplLB );
        mpImplLB->SetSelectionChangedHdl( LINK( this, ListBox, ImplSelectionChangedHdl ) );
    }
    else
        mpImplLB->GetMainWindow()->AllowGrabFocus( sal_True );

    SetCompoundControl( sal_True );
}

// -----------------------------------------------------------------------

WinBits ListBox::ImplInitStyle( WinBits nStyle )
{
    if ( !(nStyle & WB_NOTABSTOP) )
        nStyle |= WB_TABSTOP;
    if ( !(nStyle & WB_NOGROUP) )
        nStyle |= WB_GROUP;
    return nStyle;
}

// -----------------------------------------------------------------------

void ListBox::ImplLoadRes( const ResId& rResId )
{
    Control::ImplLoadRes( rResId );

    sal_uInt16 nSelPos = ReadShortRes();
    sal_uInt16 nNumber = sal::static_int_cast<sal_uInt16>(ReadLongRes());

    for( sal_uInt16 i = 0; i < nNumber; i++ )
    {
        sal_uInt16 nPos = InsertEntry( ReadStringRes(), LISTBOX_APPEND );

        long nId = ReadLongRes();
        if( nId )
            SetEntryData( nPos, (void *)nId );  // ID als UserData
    }

    if( nSelPos < nNumber )
        SelectEntryPos( nSelPos );
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(ListBox, ImplSelectHdl)
{
    sal_Bool bPopup = IsInDropDown();
    if( IsDropDownBox() )
    {
        if( !mpImplLB->IsTravelSelect() )
        {
            mpFloatWin->EndPopupMode();
            mpImplWin->GrabFocus();
        }

        mpImplWin->SetItemPos( GetSelectEntryPos() );
        mpImplWin->SetString( GetSelectEntry() );
        if( mpImplLB->GetEntryList()->HasImages() )
        {
            Image aImage = mpImplLB->GetEntryList()->GetEntryImage( GetSelectEntryPos() );
            mpImplWin->SetImage( aImage );
        }
        mpImplWin->Invalidate();
    }

    if ( ( !IsTravelSelect() || mpImplLB->IsSelectionChanged() ) || ( bPopup && !IsMultiSelectionEnabled() ) )
        Select();

    return 1;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(ListBox, ImplScrollHdl)
{
    ImplCallEventListeners( VCLEVENT_LISTBOX_SCROLLED );
    return 1;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(ListBox, ImplCancelHdl)
{
    if( IsInDropDown() )
        mpFloatWin->EndPopupMode();

    return 1;
}

// -----------------------------------------------------------------------

IMPL_LINK( ListBox, ImplSelectionChangedHdl, void*, n )
{
    if ( !mpImplLB->IsTrackingSelect() )
    {
        sal_uInt16 nChanged = (sal_uInt16)(sal_uLong)n;
        const ImplEntryList* pEntryList = mpImplLB->GetEntryList();
        if ( pEntryList->IsEntryPosSelected( nChanged ) )
        {
            // Sollte mal ein ImplPaintEntry werden...
            if ( nChanged < pEntryList->GetMRUCount() )
                nChanged = pEntryList->FindEntry( pEntryList->GetEntryText( nChanged ) );
            mpImplWin->SetItemPos( nChanged );
            mpImplWin->SetString( mpImplLB->GetEntryList()->GetEntryText( nChanged ) );
            if( mpImplLB->GetEntryList()->HasImages() )
            {
                Image aImage = mpImplLB->GetEntryList()->GetEntryImage( nChanged );
                mpImplWin->SetImage( aImage );
            }
        }
        else
        {
            mpImplWin->SetItemPos( LISTBOX_ENTRY_NOTFOUND );
            mpImplWin->SetString( ImplGetSVEmptyStr() );
            Image aImage;
            mpImplWin->SetImage( aImage );
        }
        mpImplWin->Invalidate();
    }
    return 1;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(ListBox, ImplDoubleClickHdl)
{
    DoubleClick();
    return 1;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(ListBox, ImplClickBtnHdl)
{
    if( !mpFloatWin->IsInPopupMode() )
    {
        ImplCallEventListeners( VCLEVENT_DROPDOWN_PRE_OPEN );
        mpImplWin->GrabFocus();
        mpBtn->SetPressed( sal_True );
        mpFloatWin->StartFloat( sal_True );
        ImplCallEventListeners( VCLEVENT_DROPDOWN_OPEN );

        ImplClearLayoutData();
        if( mpImplLB )
            mpImplLB->GetMainWindow()->ImplClearLayoutData();
        if( mpImplWin )
            mpImplWin->ImplClearLayoutData();
    }

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(ListBox, ImplPopupModeEndHdl)
{
    if( mpFloatWin->IsPopupModeCanceled() )
    {
        if ( ( mpFloatWin->GetPopupModeStartSaveSelection() != LISTBOX_ENTRY_NOTFOUND )
                && !IsEntryPosSelected( mpFloatWin->GetPopupModeStartSaveSelection() ) )
        {
            mpImplLB->SelectEntry( mpFloatWin->GetPopupModeStartSaveSelection(), sal_True );
            sal_Bool bTravelSelect = mpImplLB->IsTravelSelect();
            mpImplLB->SetTravelSelect( sal_True );

            ImplDelData aCheckDelete;
            ImplAddDel( &aCheckDelete );
            Select();
            if ( aCheckDelete.IsDead() )
                return 0;
            ImplRemoveDel( &aCheckDelete );

            mpImplLB->SetTravelSelect( bTravelSelect );
        }
    }

    ImplClearLayoutData();
    if( mpImplLB )
        mpImplLB->GetMainWindow()->ImplClearLayoutData();
    if( mpImplWin )
        mpImplWin->ImplClearLayoutData();

    mpBtn->SetPressed( sal_False );
    ImplCallEventListeners( VCLEVENT_DROPDOWN_CLOSE );
    return 0;
}

// -----------------------------------------------------------------------

void ListBox::ToggleDropDown()
{
    if( IsDropDownBox() )
    {
        if( mpFloatWin->IsInPopupMode() )
            mpFloatWin->EndPopupMode();
        else
        {
            ImplCallEventListeners( VCLEVENT_DROPDOWN_PRE_OPEN );
            mpImplWin->GrabFocus();
            mpBtn->SetPressed( sal_True );
            mpFloatWin->StartFloat( sal_True );
            ImplCallEventListeners( VCLEVENT_DROPDOWN_OPEN );
        }
    }
}

// -----------------------------------------------------------------------

void ListBox::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags )
{
    mpImplLB->GetMainWindow()->ImplInitSettings( sal_True, sal_True, sal_True );

    Point aPos = pDev->LogicToPixel( rPos );
    Size aSize = pDev->LogicToPixel( rSize );
    Font aFont = mpImplLB->GetMainWindow()->GetDrawPixelFont( pDev );
    OutDevType eOutDevType = pDev->GetOutDevType();

    pDev->Push();
    pDev->SetMapMode();
    pDev->SetFont( aFont );
    pDev->SetTextFillColor();

    // Border/Background
    pDev->SetLineColor();
    pDev->SetFillColor();
    sal_Bool bBorder = !(nFlags & WINDOW_DRAW_NOBORDER ) && (GetStyle() & WB_BORDER);
    sal_Bool bBackground = !(nFlags & WINDOW_DRAW_NOBACKGROUND) && IsControlBackground();
    if ( bBorder || bBackground )
    {
        Rectangle aRect( aPos, aSize );
        if ( bBorder )
        {
            ImplDrawFrame( pDev, aRect );
        }
        if ( bBackground )
        {
            pDev->SetFillColor( GetControlBackground() );
            pDev->DrawRect( aRect );
        }
    }

    // Inhalt
    if ( ( nFlags & WINDOW_DRAW_MONO ) || ( eOutDevType == OUTDEV_PRINTER ) )
    {
        pDev->SetTextColor( Color( COL_BLACK ) );
    }
    else
    {
        if ( !(nFlags & WINDOW_DRAW_NODISABLE ) && !IsEnabled() )
        {
            const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
            pDev->SetTextColor( rStyleSettings.GetDisableColor() );
        }
        else
        {
            pDev->SetTextColor( GetTextColor() );
        }
    }

    long        nOnePixel = GetDrawPixel( pDev, 1 );
    sal_uInt16      nTextStyle = TEXT_DRAW_VCENTER;
    Rectangle   aTextRect( aPos, aSize );

    if ( GetStyle() & WB_CENTER )
        nTextStyle |= TEXT_DRAW_CENTER;
    else if ( GetStyle() & WB_RIGHT )
        nTextStyle |= TEXT_DRAW_RIGHT;
    else
        nTextStyle |= TEXT_DRAW_LEFT;

    aTextRect.Left() += 3*nOnePixel;
    aTextRect.Right() -= 3*nOnePixel;

    if ( IsDropDownBox() )
    {
        XubString   aText = GetSelectEntry();
        long        nTextHeight = pDev->GetTextHeight();
        long        nTextWidth = pDev->GetTextWidth( aText );
        long        nOffX = 3*nOnePixel;
        long        nOffY = (aSize.Height()-nTextHeight) / 2;

        // Clipping?
        if ( (nOffY < 0) ||
             ((nOffY+nTextHeight) > aSize.Height()) ||
             ((nOffX+nTextWidth) > aSize.Width()) )
        {
            Rectangle aClip( aPos, aSize );
            if ( nTextHeight > aSize.Height() )
                aClip.Bottom() += nTextHeight-aSize.Height()+1;  // Damit HP-Drucker nicht 'weg-optimieren'
            pDev->IntersectClipRegion( aClip );
        }

        pDev->DrawText( aTextRect, aText, nTextStyle );
    }
    else
    {
        long        nTextHeight = pDev->GetTextHeight();
        sal_uInt16      nLines = (sal_uInt16)(aSize.Height() / nTextHeight);
        Rectangle   aClip( aPos, aSize );

        pDev->IntersectClipRegion( aClip );

        if ( !nLines )
            nLines = 1;

        for ( sal_uInt16 n = 0; n < nLines; n++ )
        {
            sal_uInt16 nEntry = n+mpImplLB->GetTopEntry();
            sal_Bool bSelected = mpImplLB->GetEntryList()->IsEntryPosSelected( nEntry );
            if ( bSelected )
            {
                pDev->SetFillColor( COL_BLACK );
                pDev->DrawRect( Rectangle(  Point( aPos.X(), aPos.Y() + n*nTextHeight ),
                                            Point( aPos.X() + aSize.Width(), aPos.Y() + (n+1)*nTextHeight + 2*nOnePixel ) ) );
                pDev->SetFillColor();
                pDev->SetTextColor( COL_WHITE );
            }

            aTextRect.Top() = aPos.Y() + n*nTextHeight;
            aTextRect.Bottom() = aTextRect.Top() + nTextHeight;

            pDev->DrawText( aTextRect, mpImplLB->GetEntryList()->GetEntryText( nEntry ), nTextStyle );

            if ( bSelected )
                pDev->SetTextColor( COL_BLACK );
        }
    }

    pDev->Pop();
}

// -----------------------------------------------------------------------

void ListBox::GetFocus()
{
    if ( mpImplLB )
    {
        if( IsDropDownBox() )
            mpImplWin->GrabFocus();
        else
            mpImplLB->GrabFocus();
    }

    Control::GetFocus();
}

// -----------------------------------------------------------------------

Window* ListBox::GetPreferredKeyInputWindow()
{
    if ( mpImplLB )
    {
        if( IsDropDownBox() )
            return mpImplWin->GetPreferredKeyInputWindow();
        else
            return mpImplLB->GetPreferredKeyInputWindow();
    }

    return Control::GetPreferredKeyInputWindow();
}

// -----------------------------------------------------------------------

void ListBox::LoseFocus()
{
    if( IsDropDownBox() )
        mpImplWin->HideFocus();
    else
        mpImplLB->HideFocus();

    Control::LoseFocus();
}

// -----------------------------------------------------------------------

void ListBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_FONTS) ||
         (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
          (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
    {
        SetBackground();    // due to a hack in Window::UpdateSettings the background must be reset
                            // otherwise it will overpaint NWF drawn listboxes
        Resize();
        mpImplLB->Resize(); // Wird nicht durch ListBox::Resize() gerufen, wenn sich die ImplLB nicht aendert.

        if ( mpImplWin )
        {
            mpImplWin->SetSettings( GetSettings() );    // Falls noch nicht eingestellt...
            ImplInitFieldSettings( mpImplWin, sal_True, sal_True, sal_True );

            mpBtn->SetSettings( GetSettings() );
            ImplInitDropDownButton( mpBtn );
        }


        if ( IsDropDownBox() )
            Invalidate();
    }
}

// -----------------------------------------------------------------------

void ListBox::EnableAutoSize( sal_Bool bAuto )
{
    mbDDAutoSize = bAuto;
    if ( mpFloatWin )
    {
        if ( bAuto && !mpFloatWin->GetDropDownLineCount() )
            mpFloatWin->SetDropDownLineCount( 5 );
        else if ( !bAuto )
            mpFloatWin->SetDropDownLineCount( 0 );
    }
}

// -----------------------------------------------------------------------

void ListBox::EnableDDAutoWidth( sal_Bool b )
{
    if ( mpFloatWin )
        mpFloatWin->SetAutoWidth( b );
}

// -----------------------------------------------------------------------

void ListBox::SetDropDownLineCount( sal_uInt16 nLines )
{
    mnLineCount = nLines;
    if ( mpFloatWin )
        mpFloatWin->SetDropDownLineCount( mnLineCount );
}

// -----------------------------------------------------------------------

sal_uInt16 ListBox::GetDropDownLineCount() const
{
    if ( mpFloatWin )
        return mpFloatWin->GetDropDownLineCount();
    return mnLineCount;
}

// -----------------------------------------------------------------------

void ListBox::SetPosSizePixel( long nX, long nY, long nWidth, long nHeight, sal_uInt16 nFlags )
{
    if( IsDropDownBox() && ( nFlags & WINDOW_POSSIZE_SIZE ) )
    {
        Size aPrefSz = mpFloatWin->GetPrefSize();
        if ( ( nFlags & WINDOW_POSSIZE_HEIGHT ) && ( nHeight >= 2*mnDDHeight ) )
            aPrefSz.Height() = nHeight-mnDDHeight;
        if ( nFlags & WINDOW_POSSIZE_WIDTH )
            aPrefSz.Width() = nWidth;
        mpFloatWin->SetPrefSize( aPrefSz );

        if ( IsAutoSizeEnabled() && ! (nFlags & WINDOW_POSSIZE_DROPDOWN) )
            nHeight = mnDDHeight;
    }

    Control::SetPosSizePixel( nX, nY, nWidth, nHeight, nFlags );
}

// -----------------------------------------------------------------------

void ListBox::Resize()
{
    Size aOutSz = GetOutputSizePixel();
    if( IsDropDownBox() )
    {
        // initialize the dropdown button size with the standard scrollbar width
        long nSBWidth = GetSettings().GetStyleSettings().GetScrollBarSize();
        long    nBottom = aOutSz.Height();

        // note: in case of no border, pBorder will actually be this
        Window *pBorder = GetWindow( WINDOW_BORDER );
        ImplControlValue aControlValue;
        Point aPoint;
        Rectangle aContent, aBound;

        // use the full extent of the control
        Rectangle aArea( aPoint, pBorder->GetOutputSizePixel() );

        if ( GetNativeControlRegion( CTRL_LISTBOX, PART_BUTTON_DOWN,
                    aArea, 0, aControlValue, rtl::OUString(), aBound, aContent) )
        {
            long nTop = 0;
            // convert back from border space to local coordinates
            aPoint = pBorder->ScreenToOutputPixel( OutputToScreenPixel( aPoint ) );
            aContent.Move( -aPoint.X(), -aPoint.Y() );

            // use the themes drop down size for the button
            aOutSz.Width() = aContent.Left();
            mpBtn->SetPosSizePixel( aContent.Left(), nTop, aContent.Right(), (nBottom-nTop) );

            // adjust the size of the edit field
            if ( GetNativeControlRegion( CTRL_LISTBOX, PART_SUB_EDIT,
                        aArea, 0, aControlValue, rtl::OUString(), aBound, aContent) )
            {
                // convert back from border space to local coordinates
                aContent.Move( -aPoint.X(), -aPoint.Y() );

                // use the themes drop down size
                if( ! (GetStyle() & WB_BORDER) && ImplGetSVData()->maNWFData.mbNoFocusRects )
                {
                    // no border but focus ring behavior -> we have a problem; the
                    // native rect relies on the border to draw the focus
                    // let's do the best we can and center vertically, so it doesn't look
                    // completely wrong.
                    Size aSz( GetOutputSizePixel() );
                    long nDiff = aContent.Top() - (aSz.Height() - aContent.GetHeight())/2;
                    aContent.Top() -= nDiff;
                    aContent.Bottom() -= nDiff;
                }
                mpImplWin->SetPosSizePixel( aContent.TopLeft(), aContent.GetSize() );
            }
            else
                mpImplWin->SetSizePixel( aOutSz );
        }
        else
        {
            nSBWidth = CalcZoom( nSBWidth );
            mpImplWin->SetPosSizePixel( 0, 0, aOutSz.Width() - nSBWidth, aOutSz.Height() );
            mpBtn->SetPosSizePixel( aOutSz.Width() - nSBWidth, 0, nSBWidth, aOutSz.Height() );
        }
    }
    else
    {
        mpImplLB->SetSizePixel( aOutSz );
    }

    // FloatingWindow-Groesse auch im unsichtbare Zustand auf Stand halten,
    // weil KEY_PGUP/DOWN ausgewertet wird...
    if ( mpFloatWin )
        mpFloatWin->SetSizePixel( mpFloatWin->CalcFloatSize() );

    Control::Resize();
}

// -----------------------------------------------------------------------

void ListBox::FillLayoutData() const
{
    mpControlData->mpLayoutData = new vcl::ControlLayoutData();
    const Control* pMainWin = mpImplLB->GetMainWindow();
    if( mpFloatWin )
    {
        // dropdown mode
        AppendLayoutData( *mpImplWin );
        mpImplWin->SetLayoutDataParent( this );
        if( mpFloatWin->IsReallyVisible() )
        {
            AppendLayoutData( *pMainWin );
            pMainWin->SetLayoutDataParent( this );
        }
    }
    else
    {
        AppendLayoutData( *pMainWin );
        pMainWin->SetLayoutDataParent( this );
    }
}

// -----------------------------------------------------------------------

long ListBox::GetIndexForPoint( const Point& rPoint, sal_uInt16& rPos ) const
{
    if( !HasLayoutData() )
        FillLayoutData();

    // check whether rPoint fits at all
    long nIndex = Control::GetIndexForPoint( rPoint );
    if( nIndex != -1 )
    {
        // point must be either in main list window
        // or in impl window (dropdown case)
        ImplListBoxWindow* pMain = mpImplLB->GetMainWindow();

        // convert coordinates to ImplListBoxWindow pixel coordinate space
        Point aConvPoint = LogicToPixel( rPoint );
        aConvPoint = OutputToAbsoluteScreenPixel( aConvPoint );
        aConvPoint = pMain->AbsoluteScreenToOutputPixel( aConvPoint );
        aConvPoint = pMain->PixelToLogic( aConvPoint );

        // try to find entry
        sal_uInt16 nEntry = pMain->GetEntryPosForPoint( aConvPoint );
        if( nEntry == LISTBOX_ENTRY_NOTFOUND )
        {
            // not found, maybe dropdown case
            if( mpImplWin && mpImplWin->IsReallyVisible() )
            {
                // convert to impl window pixel coordinates
                aConvPoint = LogicToPixel( rPoint );
                aConvPoint = OutputToAbsoluteScreenPixel( aConvPoint );
                aConvPoint = mpImplWin->AbsoluteScreenToOutputPixel( aConvPoint );

                // check whether converted point is inside impl window
                Size aImplWinSize = mpImplWin->GetOutputSizePixel();
                if( aConvPoint.X() >= 0 && aConvPoint.Y() >= 0 && aConvPoint.X() < aImplWinSize.Width() && aConvPoint.Y() < aImplWinSize.Height() )
                {
                    // inside the impl window, the position is the current item pos
                    rPos = mpImplWin->GetItemPos();
                }
                else
                    nIndex = -1;
            }
            else
                nIndex = -1;
        }
        else
            rPos = nEntry;

        DBG_ASSERT( nIndex != -1, "found index for point, but relative index failed" );
    }

    // get line relative index
    if( nIndex != -1 )
        nIndex = ToRelativeLineIndex( nIndex );

    return nIndex;
}

// -----------------------------------------------------------------------

void ListBox::StateChanged( StateChangedType nType )
{
    if( nType == STATE_CHANGE_READONLY )
    {
        if( mpImplWin )
            mpImplWin->Enable( !IsReadOnly() );
        if( mpBtn )
            mpBtn->Enable( !IsReadOnly() );
    }
    else if( nType == STATE_CHANGE_ENABLE )
    {
        mpImplLB->Enable( IsEnabled() );
        if( mpImplWin )
        {
            mpImplWin->Enable( IsEnabled() );
            if ( IsNativeControlSupported(CTRL_LISTBOX, PART_ENTIRE_CONTROL)
                    && ! IsNativeControlSupported(CTRL_LISTBOX, PART_BUTTON_DOWN) )
            {
                GetWindow( WINDOW_BORDER )->Invalidate( INVALIDATE_NOERASE );
            }
            else
                mpImplWin->Invalidate();
        }
        if( mpBtn )
            mpBtn->Enable( IsEnabled() );
    }
    else if( nType == STATE_CHANGE_UPDATEMODE )
    {
        mpImplLB->SetUpdateMode( IsUpdateMode() );
    }
    else if ( nType == STATE_CHANGE_ZOOM )
    {
        mpImplLB->SetZoom( GetZoom() );
        if ( mpImplWin )
        {
            mpImplWin->SetZoom( GetZoom() );
            mpImplWin->SetFont( mpImplLB->GetMainWindow()->GetFont() );
            mpImplWin->Invalidate();
        }
        Resize();
    }
    else if ( nType == STATE_CHANGE_CONTROLFONT )
    {
        mpImplLB->SetControlFont( GetControlFont() );
        if ( mpImplWin )
        {
            mpImplWin->SetControlFont( GetControlFont() );
            mpImplWin->SetFont( mpImplLB->GetMainWindow()->GetFont() );
            mpImplWin->Invalidate();
        }
        Resize();
    }
    else if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
    {
        mpImplLB->SetControlForeground( GetControlForeground() );
        if ( mpImplWin )
        {
            mpImplWin->SetControlForeground( GetControlForeground() );
            mpImplWin->SetTextColor( GetControlForeground() );
            mpImplWin->SetFont( mpImplLB->GetMainWindow()->GetFont() );
            mpImplWin->Invalidate();
        }
    }
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        mpImplLB->SetControlBackground( GetControlBackground() );
        if ( mpImplWin )
        {
            if ( mpImplWin->IsNativeControlSupported(CTRL_LISTBOX, PART_ENTIRE_CONTROL) )
            {
                // Transparent background
                mpImplWin->SetBackground();
                mpImplWin->SetControlBackground();
            }
            else
            {
                mpImplWin->SetBackground( mpImplLB->GetMainWindow()->GetControlBackground() );
                mpImplWin->SetControlBackground( mpImplLB->GetMainWindow()->GetControlBackground() );
            }
            mpImplWin->SetFont( mpImplLB->GetMainWindow()->GetFont() );
            mpImplWin->Invalidate();
        }
    }
    else if ( nType == STATE_CHANGE_STYLE )
    {
        SetStyle( ImplInitStyle( GetStyle() ) );
        mpImplLB->GetMainWindow()->EnableSort( ( GetStyle() & WB_SORT ) ? sal_True : sal_False );
        sal_Bool bSimpleMode = ( GetStyle() & WB_SIMPLEMODE ) ? sal_True : sal_False;
        mpImplLB->SetMultiSelectionSimpleMode( bSimpleMode );
    }
    else if( nType == STATE_CHANGE_MIRRORING )
    {
        if( mpBtn )
        {
            mpBtn->EnableRTL( IsRTLEnabled() );
            ImplInitDropDownButton( mpBtn );
        }
        mpImplLB->EnableRTL( IsRTLEnabled() );
        if( mpImplWin )
            mpImplWin->EnableRTL( IsRTLEnabled() );
        Resize();
    }

    Control::StateChanged( nType );
}

// -----------------------------------------------------------------------

long ListBox::PreNotify( NotifyEvent& rNEvt )
{
    long nDone = 0;
    if ( mpImplLB )
    {
        if( ( rNEvt.GetType() == EVENT_KEYINPUT ) && ( rNEvt.GetWindow() == mpImplWin ) )
        {
            KeyEvent aKeyEvt = *rNEvt.GetKeyEvent();
            switch( aKeyEvt.GetKeyCode().GetCode() )
            {
                case KEY_DOWN:
                {
                    if( mpFloatWin && !mpFloatWin->IsInPopupMode() &&
                        aKeyEvt.GetKeyCode().IsMod2() )
                    {
                        ImplCallEventListeners( VCLEVENT_DROPDOWN_PRE_OPEN );
                        mpBtn->SetPressed( sal_True );
                        mpFloatWin->StartFloat( sal_False );
                        ImplCallEventListeners( VCLEVENT_DROPDOWN_OPEN );
                        nDone = 1;
                    }
                    else
                    {
                        nDone = mpImplLB->ProcessKeyInput( aKeyEvt );
                    }
                }
                break;
                case KEY_UP:
                {
                    if( mpFloatWin && mpFloatWin->IsInPopupMode() &&
                        aKeyEvt.GetKeyCode().IsMod2() )
                    {
                        mpFloatWin->EndPopupMode();
                        nDone = 1;
                    }
                    else
                    {
                        nDone = mpImplLB->ProcessKeyInput( aKeyEvt );
                    }
                }
                break;
                case KEY_RETURN:
                {
                    if( IsInDropDown() )
                    {
                        mpImplLB->ProcessKeyInput( aKeyEvt );
                        nDone = 1;
                    }
                }
                break;

                default:
                {
                    nDone = mpImplLB->ProcessKeyInput( aKeyEvt );
                }
            }
        }
        else if ( rNEvt.GetType() == EVENT_LOSEFOCUS )
        {
            if ( IsInDropDown() && !HasChildPathFocus( sal_True ) )
                mpFloatWin->EndPopupMode();
        }
        else if ( (rNEvt.GetType() == EVENT_COMMAND) &&
                  (rNEvt.GetCommandEvent()->GetCommand() == COMMAND_WHEEL) &&
                  (rNEvt.GetWindow() == mpImplWin) )
        {
            sal_uInt16 nWheelBehavior( GetSettings().GetMouseSettings().GetWheelBehavior() );
            if  (   ( nWheelBehavior == MOUSE_WHEEL_ALWAYS )
                ||  (   ( nWheelBehavior == MOUSE_WHEEL_FOCUS_ONLY )
                    &&  HasChildPathFocus()
                    )
                )
            {
                nDone = mpImplLB->HandleWheelAsCursorTravel( *rNEvt.GetCommandEvent() );
            }
            else
            {
                nDone = 0;  // don't eat this event, let the default handling happen (i.e. scroll the context)
            }
        }
    }

    return nDone ? nDone : Control::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

void ListBox::Select()
{
    ImplCallEventListenersAndHandler( VCLEVENT_LISTBOX_SELECT, maSelectHdl, this );
}

// -----------------------------------------------------------------------

void ListBox::DoubleClick()
{
    ImplCallEventListenersAndHandler( VCLEVENT_LISTBOX_DOUBLECLICK, maDoubleClickHdl, this );
}

// -----------------------------------------------------------------------

void ListBox::Clear()
{
    mpImplLB->Clear();
    if( IsDropDownBox() )
    {
        mpImplWin->SetItemPos( LISTBOX_ENTRY_NOTFOUND );
        mpImplWin->SetString( ImplGetSVEmptyStr() );
        Image aImage;
        mpImplWin->SetImage( aImage );
        mpImplWin->Invalidate();
    }
    CallEventListeners( VCLEVENT_LISTBOX_ITEMREMOVED, (void*) sal_IntPtr(-1) );
}

// -----------------------------------------------------------------------

void ListBox::SetNoSelection()
{
    mpImplLB->SetNoSelection();
    if( IsDropDownBox() )
    {
        mpImplWin->SetItemPos( LISTBOX_ENTRY_NOTFOUND );
        mpImplWin->SetString( ImplGetSVEmptyStr() );
        Image aImage;
        mpImplWin->SetImage( aImage );
        mpImplWin->Invalidate();
    }
}

// -----------------------------------------------------------------------

sal_uInt16 ListBox::InsertEntry( const XubString& rStr, sal_uInt16 nPos )
{
    sal_uInt16 nRealPos = mpImplLB->InsertEntry( nPos + mpImplLB->GetEntryList()->GetMRUCount(), rStr );
    nRealPos = sal::static_int_cast<sal_uInt16>(nRealPos - mpImplLB->GetEntryList()->GetMRUCount());
    CallEventListeners( VCLEVENT_LISTBOX_ITEMADDED, (void*) sal_IntPtr(nRealPos) );
    return nRealPos;
}

// -----------------------------------------------------------------------

sal_uInt16 ListBox::InsertEntry( const Image& rImage, sal_uInt16 nPos )
{
    sal_uInt16 nRealPos = mpImplLB->InsertEntry( nPos + mpImplLB->GetEntryList()->GetMRUCount(), rImage );
    nRealPos = sal::static_int_cast<sal_uInt16>(nRealPos - mpImplLB->GetEntryList()->GetMRUCount());
    CallEventListeners( VCLEVENT_LISTBOX_ITEMADDED, (void*) sal_IntPtr(nRealPos) );
    return nRealPos;
}

// -----------------------------------------------------------------------

sal_uInt16 ListBox::InsertEntry( const XubString& rStr, const Image& rImage, sal_uInt16 nPos )
{
    sal_uInt16 nRealPos = mpImplLB->InsertEntry( nPos + mpImplLB->GetEntryList()->GetMRUCount(), rStr, rImage );
    nRealPos = sal::static_int_cast<sal_uInt16>(nRealPos - mpImplLB->GetEntryList()->GetMRUCount());
    CallEventListeners( VCLEVENT_LISTBOX_ITEMADDED, (void*) sal_IntPtr(nRealPos) );
    return nRealPos;
}

// -----------------------------------------------------------------------

void ListBox::RemoveEntry( const XubString& rStr )
{
    RemoveEntry( GetEntryPos( rStr ) );
}

// -----------------------------------------------------------------------

void ListBox::RemoveEntry( sal_uInt16 nPos )
{
    mpImplLB->RemoveEntry( nPos + mpImplLB->GetEntryList()->GetMRUCount() );
    CallEventListeners( VCLEVENT_LISTBOX_ITEMREMOVED, (void*) sal_IntPtr(nPos) );
}

// -----------------------------------------------------------------------

Image ListBox::GetEntryImage( sal_uInt16 nPos ) const
{
    if ( mpImplLB->GetEntryList()->HasEntryImage( nPos ) )
        return mpImplLB->GetEntryList()->GetEntryImage( nPos );
    return Image();
}

// -----------------------------------------------------------------------

sal_uInt16 ListBox::GetEntryPos( const XubString& rStr ) const
{
    sal_uInt16 nPos = mpImplLB->GetEntryList()->FindEntry( rStr );
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        nPos = sal::static_int_cast<sal_uInt16>(nPos - mpImplLB->GetEntryList()->GetMRUCount());
    return nPos;
}

// -----------------------------------------------------------------------

sal_uInt16 ListBox::GetEntryPos( const void* pData ) const
{
    sal_uInt16 nPos = mpImplLB->GetEntryList()->FindEntry( pData );
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        nPos = sal::static_int_cast<sal_uInt16>(nPos - mpImplLB->GetEntryList()->GetMRUCount());
    return nPos;
}

// -----------------------------------------------------------------------

XubString ListBox::GetEntry( sal_uInt16 nPos ) const
{
    return mpImplLB->GetEntryList()->GetEntryText( nPos + mpImplLB->GetEntryList()->GetMRUCount() );
}

// -----------------------------------------------------------------------

sal_uInt16 ListBox::GetEntryCount() const
{
    return mpImplLB->GetEntryList()->GetEntryCount() - mpImplLB->GetEntryList()->GetMRUCount();
}

// -----------------------------------------------------------------------

XubString ListBox::GetSelectEntry( sal_uInt16 nIndex ) const
{
    return GetEntry( GetSelectEntryPos( nIndex ) );
}

// -----------------------------------------------------------------------

sal_uInt16 ListBox::GetSelectEntryCount() const
{
    return mpImplLB->GetEntryList()->GetSelectEntryCount();
}

// -----------------------------------------------------------------------

sal_uInt16 ListBox::GetSelectEntryPos( sal_uInt16 nIndex ) const
{
    sal_uInt16 nPos = mpImplLB->GetEntryList()->GetSelectEntryPos( nIndex );
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        if ( nPos < mpImplLB->GetEntryList()->GetMRUCount() )
            nPos = mpImplLB->GetEntryList()->FindEntry( mpImplLB->GetEntryList()->GetEntryText( nPos ) );
        nPos = sal::static_int_cast<sal_uInt16>(nPos - mpImplLB->GetEntryList()->GetMRUCount());
    }
    return nPos;
}

// -----------------------------------------------------------------------

sal_Bool ListBox::IsEntrySelected( const XubString& rStr ) const
{
    return IsEntryPosSelected( GetEntryPos( rStr ) );
}

// -----------------------------------------------------------------------

sal_Bool ListBox::IsEntryPosSelected( sal_uInt16 nPos ) const
{
    return mpImplLB->GetEntryList()->IsEntryPosSelected( nPos + mpImplLB->GetEntryList()->GetMRUCount() );
}

// -----------------------------------------------------------------------

void ListBox::SelectEntry( const XubString& rStr, sal_Bool bSelect )
{
    SelectEntryPos( GetEntryPos( rStr ), bSelect );
}

// -----------------------------------------------------------------------

void ListBox::SelectEntryPos( sal_uInt16 nPos, sal_Bool bSelect )
{
    if ( nPos < mpImplLB->GetEntryList()->GetEntryCount() )
        mpImplLB->SelectEntry( nPos + mpImplLB->GetEntryList()->GetMRUCount(), bSelect );
}

// -----------------------------------------------------------------------

void ListBox::SetEntryData( sal_uInt16 nPos, void* pNewData )
{
    mpImplLB->SetEntryData( nPos + mpImplLB->GetEntryList()->GetMRUCount(), pNewData );
}

// -----------------------------------------------------------------------

void* ListBox::GetEntryData( sal_uInt16 nPos ) const
{
    return mpImplLB->GetEntryList()->GetEntryData( nPos + mpImplLB->GetEntryList()->GetMRUCount() );
}

// -----------------------------------------------------------------------

void ListBox::SetEntryFlags( sal_uInt16 nPos, long nFlags )
{
    mpImplLB->SetEntryFlags( nPos + mpImplLB->GetEntryList()->GetMRUCount(), nFlags );
}

// -----------------------------------------------------------------------

long ListBox::GetEntryFlags( sal_uInt16 nPos ) const
{
    return mpImplLB->GetEntryList()->GetEntryFlags( nPos + mpImplLB->GetEntryList()->GetMRUCount() );
}

// -----------------------------------------------------------------------

void ListBox::SetTopEntry( sal_uInt16 nPos )
{
    mpImplLB->SetTopEntry( nPos + mpImplLB->GetEntryList()->GetMRUCount() );
}

// -----------------------------------------------------------------------

sal_uInt16 ListBox::GetTopEntry() const
{
    sal_uInt16 nPos = GetEntryCount() ? mpImplLB->GetTopEntry() : LISTBOX_ENTRY_NOTFOUND;
    if ( nPos < mpImplLB->GetEntryList()->GetMRUCount() )
        nPos = 0;
    return nPos;
}

// -----------------------------------------------------------------------

sal_Bool ListBox::IsTravelSelect() const
{
    return mpImplLB->IsTravelSelect();
}

// -----------------------------------------------------------------------

sal_Bool ListBox::IsInDropDown() const
{
    return mpFloatWin && mpFloatWin->IsInPopupMode();
}

// -----------------------------------------------------------------------

Rectangle ListBox::GetBoundingRectangle( sal_uInt16 nItem ) const
{
    Rectangle aRect = mpImplLB->GetMainWindow()->GetBoundingRectangle( nItem );
    Rectangle aOffset = mpImplLB->GetMainWindow()->GetWindowExtentsRelative( (Window*)this );
    aRect.Move( aOffset.TopLeft().X(), aOffset.TopLeft().Y() );
    return aRect;
}

// -----------------------------------------------------------------------

void ListBox::EnableMultiSelection( sal_Bool bMulti )
{
    EnableMultiSelection( bMulti, sal_False );
}

void ListBox::EnableMultiSelection( sal_Bool bMulti, sal_Bool bStackSelection )
{
    mpImplLB->EnableMultiSelection( bMulti, bStackSelection );

    // WB_SIMPLEMODE:
    // Die MultiListBox verh�lt sich wie eine normale ListBox.
    // Die Mehrfachselektion kann nur �ber entsprechende Zusatztasten erfolgen.

    sal_Bool bSimpleMode = ( GetStyle() & WB_SIMPLEMODE ) ? sal_True : sal_False;
    mpImplLB->SetMultiSelectionSimpleMode( bSimpleMode );

    // ohne Focus ist das Traveln in einer MultiSelection nicht zu sehen:
    if ( mpFloatWin )
        mpImplLB->GetMainWindow()->AllowGrabFocus( bMulti );
}

// -----------------------------------------------------------------------

sal_Bool ListBox::IsMultiSelectionEnabled() const
{
    return mpImplLB->IsMultiSelectionEnabled();
}

// -----------------------------------------------------------------------

Size ListBox::CalcMinimumSize() const
{
    Size aSz;

    if (!mpImplLB)
        return aSz;

    if ( !IsDropDownBox() )
        aSz = mpImplLB->CalcSize (mnLineCount ? mnLineCount : mpImplLB->GetEntryList()->GetEntryCount());
    else
    {
        aSz.Height() = mpImplLB->CalcSize( 1 ).Height();
        aSz.Height() += 4; // add a space between entry and border
        // size to maxmimum entry width and add a little breathing space
        aSz.Width() = mpImplLB->GetMaxEntryWidth() + 4;
        // do not create ultrathin ListBoxes, it doesn't look good
        if( aSz.Width() < GetSettings().GetStyleSettings().GetScrollBarSize() )
            aSz.Width() = GetSettings().GetStyleSettings().GetScrollBarSize();

        // try native borders; scrollbar size may not be a good indicator
        // see how large the edit area inside is to estimate what is needed for the dropdown
        ImplControlValue aControlValue;
        Point aPoint;
        Rectangle aContent, aBound;
        Size aTestSize( 100, 20 );
        Rectangle aArea( aPoint, aTestSize );
        if( const_cast<ListBox*>(this)->GetNativeControlRegion(
                       CTRL_LISTBOX, PART_SUB_EDIT, aArea, 0, aControlValue, rtl::OUString(), aBound, aContent) )
        {
            // use the themes drop down size
            aSz.Width() += aTestSize.Width() - aContent.GetWidth();
        }
        else
            aSz.Width() += GetSettings().GetStyleSettings().GetScrollBarSize();
    }

    aSz = CalcWindowSize( aSz );

    if ( IsDropDownBox() ) // check minimum height of dropdown box
    {
        ImplControlValue aControlValue;
        Rectangle aRect( Point( 0, 0 ), aSz );
        Rectangle aContent, aBound;
        if( const_cast<ListBox*>(this)->GetNativeControlRegion(
                       CTRL_LISTBOX, PART_ENTIRE_CONTROL, aRect, 0, aControlValue, rtl::OUString(), aBound, aContent) )
        {
            if( aBound.GetHeight() > aSz.Height() )
                aSz.Height() = aBound.GetHeight();
        }
    }

    return aSz;
}

// -----------------------------------------------------------------------

Size ListBox::GetOptimalSize(WindowSizeType eType) const
{
    switch (eType) {
    case WINDOWSIZE_MINIMUM:
        return CalcMinimumSize();
    default:
        return Control::GetOptimalSize( eType );
    }
}

// -----------------------------------------------------------------------

Size ListBox::CalcAdjustedSize( const Size& rPrefSize ) const
{
    Size aSz = rPrefSize;
    sal_Int32 nLeft, nTop, nRight, nBottom;
    ((Window*)this)->GetBorder( nLeft, nTop, nRight, nBottom );
    aSz.Height() -= nTop+nBottom;
    if ( !IsDropDownBox() )
    {
        long nEntryHeight = CalcSize( 1, 1 ).Height();
        long nLines = aSz.Height() / nEntryHeight;
        if ( nLines < 1 )
            nLines = 1;
        aSz.Height() = nLines * nEntryHeight;
    }
    else
    {
        aSz.Height() = mnDDHeight;
    }
    aSz.Height() += nTop+nBottom;

    aSz = CalcWindowSize( aSz );
    return aSz;
}

// -----------------------------------------------------------------------

Size ListBox::CalcSize( sal_uInt16 nColumns, sal_uInt16 nLines ) const
{
    // ggf. werden ScrollBars eingeblendet
    Size aMinSz = CalcMinimumSize();
//  aMinSz = ImplCalcOutSz( aMinSz );

    Size aSz;

    // Hoehe
    if ( nLines )
    {
        if ( !IsDropDownBox() )
            aSz.Height() = mpImplLB->CalcSize( nLines ).Height();
        else
            aSz.Height() = mnDDHeight;
    }
    else
        aSz.Height() = aMinSz.Height();

    // Breite
    if ( nColumns )
        aSz.Width() = nColumns * GetTextWidth( rtl::OUString('X') );
    else
        aSz.Width() = aMinSz.Width();

    if ( IsDropDownBox() )
        aSz.Width() += GetSettings().GetStyleSettings().GetScrollBarSize();

    if ( !IsDropDownBox() )
    {
        if ( aSz.Width() < aMinSz.Width() )
            aSz.Height() += GetSettings().GetStyleSettings().GetScrollBarSize();
        if ( aSz.Height() < aMinSz.Height() )
            aSz.Width() += GetSettings().GetStyleSettings().GetScrollBarSize();
    }

    aSz = CalcWindowSize( aSz );
    return aSz;
}

// -----------------------------------------------------------------------

void ListBox::GetMaxVisColumnsAndLines( sal_uInt16& rnCols, sal_uInt16& rnLines ) const
{
    long nCharWidth = GetTextWidth( rtl::OUString(static_cast<sal_Unicode>('x')) );
    if ( !IsDropDownBox() )
    {
        Size aOutSz = mpImplLB->GetMainWindow()->GetOutputSizePixel();
        rnCols = (sal_uInt16) (aOutSz.Width()/nCharWidth);
        rnLines = (sal_uInt16) (aOutSz.Height()/mpImplLB->GetEntryHeight());
    }
    else
    {
        Size aOutSz = mpImplWin->GetOutputSizePixel();
        rnCols = (sal_uInt16) (aOutSz.Width()/nCharWidth);
        rnLines = 1;
    }
}

// -----------------------------------------------------------------------

IMPL_LINK( ListBox, ImplUserDrawHdl, UserDrawEvent*, pEvent )
{
    UserDraw( *pEvent );
    return 1;
}

// -----------------------------------------------------------------------

void ListBox::UserDraw( const UserDrawEvent& )
{
}

// -----------------------------------------------------------------------

void ListBox::DrawEntry( const UserDrawEvent& rEvt, sal_Bool bDrawImage, sal_Bool bDrawText, sal_Bool bDrawTextAtImagePos )
{
    if ( rEvt.GetDevice() == mpImplLB->GetMainWindow() )
        mpImplLB->GetMainWindow()->DrawEntry( rEvt.GetItemId(), bDrawImage, bDrawText, bDrawTextAtImagePos );
    else if ( rEvt.GetDevice() == mpImplWin )
        mpImplWin->DrawEntry( bDrawImage, bDrawText, bDrawTextAtImagePos );
}

// -----------------------------------------------------------------------

void ListBox::SetUserItemSize( const Size& rSz )
{
    mpImplLB->GetMainWindow()->SetUserItemSize( rSz );
    if ( mpImplWin )
        mpImplWin->SetUserItemSize( rSz );
}

// -----------------------------------------------------------------------

void ListBox::EnableUserDraw( sal_Bool bUserDraw )
{
    mpImplLB->GetMainWindow()->EnableUserDraw( bUserDraw );
    if ( mpImplWin )
        mpImplWin->EnableUserDraw( bUserDraw );
}

// -----------------------------------------------------------------------

void ListBox::SetReadOnly( sal_Bool bReadOnly )
{
    if ( mpImplLB->IsReadOnly() != bReadOnly )
    {
        mpImplLB->SetReadOnly( bReadOnly );
        StateChanged( STATE_CHANGE_READONLY );
    }
}

// -----------------------------------------------------------------------

sal_Bool ListBox::IsReadOnly() const
{
    return mpImplLB->IsReadOnly();
}

// -----------------------------------------------------------------------

void ListBox::SetSeparatorPos( sal_uInt16 n )
{
    mpImplLB->SetSeparatorPos( n );
}

// -----------------------------------------------------------------------

sal_uInt16 ListBox::GetSeparatorPos() const
{
    return mpImplLB->GetSeparatorPos();
}

// -----------------------------------------------------------------------

sal_uInt16 ListBox::GetDisplayLineCount() const
{
    return mpImplLB->GetDisplayLineCount();
}

// -----------------------------------------------------------------------

// pb: #106948# explicit mirroring for calc

void ListBox::EnableMirroring()
{
    mpImplLB->EnableMirroring();
}

// -----------------------------------------------------------------------

Rectangle ListBox::GetDropDownPosSizePixel() const
{
    return mpFloatWin ? mpFloatWin->GetWindowExtentsRelative( const_cast<ListBox*>(this) ) : Rectangle();
}

// -----------------------------------------------------------------------

const Wallpaper& ListBox::GetDisplayBackground() const
{
    // !!! recursion does not occur because the ImplListBox is default
    // initialized to a nontransparent color in Window::ImplInitData
    return mpImplLB->GetDisplayBackground();
}

// =======================================================================
MultiListBox::MultiListBox( Window* pParent, WinBits nStyle ) :
    ListBox( WINDOW_MULTILISTBOX )
{
    ImplInit( pParent, nStyle );
    EnableMultiSelection( sal_True );
}

// -----------------------------------------------------------------------

MultiListBox::MultiListBox( Window* pParent, const ResId& rResId ) :
    ListBox( WINDOW_MULTILISTBOX )
{
    rResId.SetRT( RSC_MULTILISTBOX );
    WinBits nStyle = ImplInitRes( rResId );

    if (VclBuilderContainer::replace_buildable(pParent, rResId, *this))
        return;

    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE ) )
        Show();
    EnableMultiSelection( sal_True );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
