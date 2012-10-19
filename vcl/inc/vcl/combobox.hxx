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

#ifndef _SV_COMBOBOX_HXX
#define _SV_COMBOBOX_HXX

#include <vcl/dllapi.h>
#include <vcl/ctrl.hxx>
#include <vcl/combobox.h>
#include <vcl/edit.hxx>

class UserDrawEvent;

class ImplListBoxFloatingWindow;
class ImplListBox;
class ImplBtn;

// ------------
// - ComboBox -
// ------------

class VCL_DLLPUBLIC ComboBox : public Edit
{
private:
    Edit*                       mpSubEdit;
    ImplListBox*                mpImplLB;
    ImplBtn*                    mpBtn;
    ImplListBoxFloatingWindow*  mpFloatWin;
    sal_uInt16                      mnDDHeight;
    sal_Unicode                 mcMultiSep;
    sal_Bool                        mbDDAutoSize        : 1;
    sal_Bool                        mbSyntheticModify   : 1;
    sal_Bool                        mbMatchCase         : 1;
    Link                        maSelectHdl;
    Link                        maDoubleClickHdl;

private:
    SAL_DLLPRIVATE void     ImplInitComboBoxData();
    SAL_DLLPRIVATE void     ImplUpdateFloatSelection();

    DECL_DLLPRIVATE_LINK(   ImplSelectHdl, void* );
    DECL_DLLPRIVATE_LINK(   ImplCancelHdl, void* );
    DECL_DLLPRIVATE_LINK(   ImplDoubleClickHdl, void* );
    DECL_DLLPRIVATE_LINK(   ImplClickBtnHdl, void* );
    DECL_DLLPRIVATE_LINK(   ImplPopupModeEndHdl, void* );
    DECL_DLLPRIVATE_LINK(   ImplSelectionChangedHdl, void* );
    DECL_DLLPRIVATE_LINK(   ImplUserDrawHdl, UserDrawEvent* );
    DECL_DLLPRIVATE_LINK(   ImplAutocompleteHdl, Edit* );

protected:
    using Window::ImplInit;
    SAL_DLLPRIVATE void     ImplInit( Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE WinBits  ImplInitStyle( WinBits nStyle );
    SAL_DLLPRIVATE void     ImplLoadRes( const ResId& rResId );
    SAL_DLLPRIVATE void     ImplCalcEditHeight();
    SAL_DLLPRIVATE long     getMaxWidthScrollBarAndDownButton() const;

protected:
                    ComboBox( WindowType nType );
    sal_Bool            IsDropDownBox() const { return mpFloatWin ? sal_True : sal_False; }

    virtual void  FillLayoutData() const;
public:
                    ComboBox( Window* pParent, WinBits nStyle = 0 );
                    ComboBox( Window* pParent, const ResId& rResId );
                    ~ComboBox();

    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags );
    virtual void    Resize();
    virtual long    PreNotify( NotifyEvent& rNEvt );
    virtual long    Notify( NotifyEvent& rNEvt );
    virtual void    StateChanged( StateChangedType nType );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    virtual void    UserDraw( const UserDrawEvent& rUDEvt );
    virtual void    Select();
    virtual void    DoubleClick();

    virtual void    Modify();

    virtual const Wallpaper& GetDisplayBackground() const;

    virtual void    setPosSizePixel( long nX, long nY, long nWidth, long nHeight, sal_uInt16 nFlags = WINDOW_POSSIZE_ALL );
    void            SetPosSizePixel( const Point& rNewPos, const Size& rNewSize )
                        { Edit::SetPosSizePixel( rNewPos, rNewSize ); }
    void            SetDropDownSizePixel( const Size& rNewSize )
    { if( IsDropDownBox() ) setPosSizePixel( 0, 0, rNewSize.Width(), rNewSize.Height(), WINDOW_POSSIZE_SIZE | WINDOW_POSSIZE_DROPDOWN ); }

    Rectangle       GetDropDownPosSizePixel() const;

    void            SetDropDownLineCount( sal_uInt16 nLines );
    sal_uInt16      GetDropDownLineCount() const;
    void            SetBestDropDownLineCount() { SetDropDownLineCount(16); }

    void            EnableAutoSize( sal_Bool bAuto );
    sal_Bool            IsAutoSizeEnabled() const               { return mbDDAutoSize; }

    void            EnableDDAutoWidth( sal_Bool b );

    void            SetText( const XubString& rStr );
    void            SetText( const XubString& rStr, const Selection& rNewSelection );

    sal_uInt16          InsertEntry( const XubString& rStr, sal_uInt16 nPos = COMBOBOX_APPEND );
    sal_uInt16          InsertEntry( const XubString& rStr, const Image& rImage, sal_uInt16 nPos = COMBOBOX_APPEND );

    void            RemoveEntry( const XubString& rStr );
    void            RemoveEntry( sal_uInt16 nPos );

    void            Clear();

    sal_uInt16          GetEntryPos( const XubString& rStr ) const;
    Image           GetEntryImage( sal_uInt16 nPos ) const;
    XubString       GetEntry( sal_uInt16 nPos ) const;
    sal_uInt16          GetEntryCount() const;

    sal_Bool            IsTravelSelect() const;
    sal_Bool            IsInDropDown() const;
    void            ToggleDropDown();

    long            CalcWindowSizePixel( sal_uInt16 nLines ) const;

    void            SetUserItemSize( const Size& rSz );
    void            EnableUserDraw( sal_Bool bUserDraw );

    void            DrawEntry( const UserDrawEvent& rEvt, sal_Bool bDrawImage, sal_Bool bDrawText, sal_Bool bDrawTextAtImagePos = sal_False );
    void            SetBorderStyle( sal_uInt16 nBorderStyle );

    void            SetSeparatorPos( sal_uInt16 n = LISTBOX_ENTRY_NOTFOUND );

    void            EnableAutocomplete( sal_Bool bEnable, sal_Bool bMatchCase = sal_False );
    sal_Bool            IsAutocompleteEnabled() const;

    void            EnableMultiSelection( sal_Bool bMulti );
    sal_Bool            IsMultiSelectionEnabled() const;
    void            SetMultiSelectionSeparator( sal_Unicode cSep ) { mcMultiSep = cSep; }
    sal_Unicode     GetMultiSelectionSeparator() const { return mcMultiSep; }

    void            SetSelectHdl( const Link& rLink )       { maSelectHdl = rLink; }
    const Link&     GetSelectHdl() const                    { return maSelectHdl; }
    void            SetDoubleClickHdl( const Link& rLink )  { maDoubleClickHdl = rLink; }
    const Link&     GetDoubleClickHdl() const               { return maDoubleClickHdl; }

    Size            CalcMinimumSize() const;
    virtual Size    GetOptimalSize(WindowSizeType eType) const;
    Size            CalcAdjustedSize( const Size& rPrefSize ) const;
    using Edit::CalcSize;
    Size            CalcSize( sal_uInt16 nColumns, sal_uInt16 nLines ) const;
    void            GetMaxVisColumnsAndLines( sal_uInt16& rnCols, sal_uInt16& rnLines ) const;

    void            SetMRUEntries( const XubString& rEntries, sal_Unicode cSep = ';' );
    XubString       GetMRUEntries( sal_Unicode cSep = ';' ) const;
    void            SetMaxMRUCount( sal_uInt16 n );
    sal_uInt16          GetMaxMRUCount() const;

    void            SetEntryData( sal_uInt16 nPos, void* pNewData );
    void*           GetEntryData( sal_uInt16 nPos ) const;

    sal_uInt16          GetTopEntry() const;

    void            SetProminentEntryType( ProminentEntry eType );

    sal_uInt16          GetDisplayLineCount() const;

    sal_uInt16          GetSelectEntryCount() const;
    sal_uInt16          GetSelectEntryPos( sal_uInt16 nSelIndex = 0 ) const;
    sal_Bool            IsEntryPosSelected( sal_uInt16 nPos ) const;
    void            SelectEntryPos( sal_uInt16 nPos, sal_Bool bSelect = sal_True );
    void            SetNoSelection();
    Rectangle       GetBoundingRectangle( sal_uInt16 nItem ) const;

    /** checks whether a certain point lies within the bounds of
        a list item and returns the item as well as the character position
        the point is at.

        <p>If the point is inside an item the item pos is put into <code>rPos</code> and
        the item-relative character index is returned. If the point is not inside
        an item -1 is returned and rPos is unchanged.</p>

        @param rPoint
        tells the point for which an item is requested.

        @param rPos
        gets the item at the specified point <code>rPoint</code>

        @returns
        the item-relative character index at point <code>rPos</code> or -1
        if no item is at that point.
     */
    using Control::GetIndexForPoint;
    long GetIndexForPoint( const Point& rPoint, sal_uInt16& rPos ) const;
};

#endif  // _COMBOBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
