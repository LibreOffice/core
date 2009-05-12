/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: combobox.hxx,v $
 * $Revision: 1.4 $
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
    USHORT                      mnDDHeight;
    xub_Unicode                 mcMultiSep;
    BOOL                        mbDDAutoSize        : 1;
    BOOL                        mbSyntheticModify   : 1;
    BOOL                        mbMatchCase         : 1;
    Link                        maSelectHdl;
    Link                        maDoubleClickHdl;

//#if 0 // _SOLAR__PRIVATE
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
//#endif

protected:
                    ComboBox( WindowType nType );
    BOOL            IsDropDownBox() const { return mpFloatWin ? TRUE : FALSE; }

    virtual void  FillLayoutData() const;
public:
                    ComboBox( Window* pParent, WinBits nStyle = 0 );
                    ComboBox( Window* pParent, const ResId& rResId );
                    ~ComboBox();

    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, ULONG nFlags );
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

    virtual void    SetPosSizePixel( long nX, long nY, long nWidth, long nHeight, USHORT nFlags = WINDOW_POSSIZE_ALL );
    void            SetPosSizePixel( const Point& rNewPos, const Size& rNewSize )
                        { Edit::SetPosSizePixel( rNewPos, rNewSize ); }
    void            SetDropDownSizePixel( const Size& rNewSize )
    { if( IsDropDownBox() ) SetPosSizePixel( 0, 0, rNewSize.Width(), rNewSize.Height(), WINDOW_POSSIZE_SIZE | WINDOW_POSSIZE_DROPDOWN ); }

    Rectangle       GetDropDownPosSizePixel() const;
    Rectangle       GetListPosSizePixel() const;
    // returns empty rectangle in DropDown mode,
    // else it returns the PosSize of the ListBox

    void            SetDropDownLineCount( USHORT nLines );
    USHORT          GetDropDownLineCount() const;

    void            EnableAutoSize( BOOL bAuto );
    BOOL            IsAutoSizeEnabled() const               { return mbDDAutoSize; }

    void            EnableDDAutoWidth( BOOL b );
    BOOL            IsDDAutoWidthEnabled() const;

    void            SetText( const XubString& rStr );
    void            SetText( const XubString& rStr, const Selection& rNewSelection );

    USHORT          InsertEntry( const XubString& rStr, USHORT nPos = COMBOBOX_APPEND );
    USHORT          InsertEntry( const XubString& rStr, const Image& rImage, USHORT nPos = COMBOBOX_APPEND );

    void            RemoveEntry( const XubString& rStr );
    void            RemoveEntry( USHORT nPos );

    void            Clear();

    USHORT          GetEntryPos( const XubString& rStr ) const;
    USHORT          GetEntryPos( const void* pData ) const;
    XubString       GetEntry( USHORT nPos ) const;
    USHORT          GetEntryCount() const;

    BOOL            IsTravelSelect() const;
    BOOL            IsInDropDown() const;
    void            ToggleDropDown();

    long            CalcWindowSizePixel( USHORT nLines ) const;

    void            SetUserItemSize( const Size& rSz );
    const Size&     GetUserItemSize() const;

    void            EnableUserDraw( BOOL bUserDraw );
    BOOL            IsUserDrawEnabled() const;

    void            DrawEntry( const UserDrawEvent& rEvt, BOOL bDrawImage, BOOL bDrawText, BOOL bDrawTextAtImagePos = FALSE );
    void            SetBorderStyle( USHORT nBorderStyle );

    void            SetSeparatorPos( USHORT n );
    void            SetSeparatorPos();
    USHORT          GetSeparatorPos() const;

    void            EnableAutocomplete( BOOL bEnable, BOOL bMatchCase = FALSE );
    BOOL            IsAutocompleteEnabled() const;

    void            EnableMultiSelection( BOOL bMulti );
    BOOL            IsMultiSelectionEnabled() const;
    void            SetMultiSelectionSeparator( xub_Unicode cSep ) { mcMultiSep = cSep; }
    xub_Unicode     GetMultiSelectionSeparator() const { return mcMultiSep; }

    void            SetSelectHdl( const Link& rLink )       { maSelectHdl = rLink; }
    const Link&     GetSelectHdl() const                    { return maSelectHdl; }
    void            SetDoubleClickHdl( const Link& rLink )  { maDoubleClickHdl = rLink; }
    const Link&     GetDoubleClickHdl() const               { return maDoubleClickHdl; }

    Size            CalcMinimumSize() const;
    virtual Size    GetOptimalSize(WindowSizeType eType) const;
    Size            CalcAdjustedSize( const Size& rPrefSize ) const;
    using Edit::CalcSize;
    Size            CalcSize( USHORT nColumns, USHORT nLines ) const;
    void            GetMaxVisColumnsAndLines( USHORT& rnCols, USHORT& rnLines ) const;

    void            SetMRUEntries( const XubString& rEntries, xub_Unicode cSep = ';' );
    XubString       GetMRUEntries( xub_Unicode cSep = ';' ) const;
    void            SetMaxMRUCount( USHORT n );
    USHORT          GetMaxMRUCount() const;

    void            SetEntryData( USHORT nPos, void* pNewData );
    void*           GetEntryData( USHORT nPos ) const;

    void            SetTopEntry( USHORT nPos );
    USHORT          GetTopEntry() const;

    USHORT          GetDisplayLineCount() const;

    USHORT          GetSelectEntryCount() const;
    USHORT          GetSelectEntryPos( USHORT nSelIndex = 0 ) const;
    BOOL            IsEntryPosSelected( USHORT nPos ) const;
    void            SelectEntryPos( USHORT nPos, BOOL bSelect = TRUE );
    void            SetNoSelection();
    Rectangle       GetBoundingRectangle( USHORT nItem ) const;

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
    long GetIndexForPoint( const Point& rPoint, USHORT& rPos ) const;
};

#endif  // _COMBOBOX_HXX
