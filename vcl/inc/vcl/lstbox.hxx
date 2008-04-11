/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: lstbox.hxx,v $
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

#ifndef _SV_LSTBOX_HXX
#define _SV_LSTBOX_HXX

#include <vcl/dllapi.h>
#include <vcl/ctrl.hxx>
#include <vcl/lstbox.h>

class Image;
class ImplListBox;
class ImplListBoxFloatingWindow;
class ImplBtn;
class ImplWin;

// --------------------------------------------------------------------
//  - ListBox -
// --------------------------------------------------------------------

class VCL_DLLPUBLIC ListBox : public Control
{
private:
    ImplListBox*                mpImplLB;
    ImplListBoxFloatingWindow*  mpFloatWin;
    ImplWin*                    mpImplWin;
    ImplBtn*                    mpBtn;
    USHORT                      mnDDHeight;
    USHORT                      mnSaveValue;
    BOOL                        mbDDAutoSize;
    Link                        maSelectHdl;
    Link                        maDoubleClickHdl;

//#if 0 // _SOLAR__PRIVATE
private:
    SAL_DLLPRIVATE void    ImplInitListBoxData();

    DECL_DLLPRIVATE_LINK(  ImplSelectHdl, void* );
    DECL_DLLPRIVATE_LINK(  ImplScrollHdl, void* );
    DECL_DLLPRIVATE_LINK(  ImplCancelHdl, void* );
    DECL_DLLPRIVATE_LINK(  ImplDoubleClickHdl, void* );
    DECL_DLLPRIVATE_LINK(  ImplClickBtnHdl, void* );
    DECL_DLLPRIVATE_LINK(  ImplPopupModeEndHdl, void* );
    DECL_DLLPRIVATE_LINK(  ImplSelectionChangedHdl, void* );
    DECL_DLLPRIVATE_LINK(  ImplUserDrawHdl, UserDrawEvent* );

protected:
    using Window::ImplInit;
    SAL_DLLPRIVATE void    ImplInit( Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE WinBits ImplInitStyle( WinBits nStyle );
    SAL_DLLPRIVATE void    ImplLoadRes( const ResId& rResId );
//#endif
    BOOL                IsDropDownBox() const { return mpFloatWin ? TRUE : FALSE; }

protected:
                        ListBox( WindowType nType );

    virtual void        FillLayoutData() const;

public:
                        ListBox( Window* pParent, WinBits nStyle = WB_BORDER );
                        ListBox( Window* pParent, const ResId& rResId );
                        ~ListBox();

    virtual void        Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, ULONG nFlags );
    virtual void        Resize();
    virtual long        PreNotify( NotifyEvent& rNEvt );
    virtual void        StateChanged( StateChangedType nType );
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );
    virtual void        UserDraw( const UserDrawEvent& rUDEvt );

    virtual void        Select();
    virtual void        DoubleClick();
    virtual void        GetFocus();
    virtual void        LoseFocus();
    virtual Window*     GetPreferredKeyInputWindow();

    virtual const Wallpaper& GetDisplayBackground() const;

    virtual void        SetPosSizePixel( long nX, long nY,
                                         long nWidth, long nHeight, USHORT nFlags = WINDOW_POSSIZE_ALL );
    void                SetPosSizePixel( const Point& rNewPos, const Size& rNewSize )
                        { Control::SetPosSizePixel( rNewPos, rNewSize ); }
    void                SetDropDownSizePixel( const Size& rNewSize )
    { if( IsDropDownBox() ) SetPosSizePixel( 0, 0, rNewSize.Width(), rNewSize.Height(), WINDOW_POSSIZE_SIZE | WINDOW_POSSIZE_DROPDOWN ); }

    Rectangle           GetDropDownPosSizePixel() const;

    void                SetDropDownLineCount( USHORT nLines );
    USHORT              GetDropDownLineCount() const;

    void                EnableAutoSize( BOOL bAuto );
    BOOL                IsAutoSizeEnabled() const { return mbDDAutoSize; }

    void                EnableDDAutoWidth( BOOL b );
    BOOL                IsDDAutoWidthEnabled() const;

    virtual USHORT      InsertEntry( const XubString& rStr, USHORT nPos = LISTBOX_APPEND );
    virtual USHORT      InsertEntry( const Image& rImage, USHORT nPos = LISTBOX_APPEND );
    virtual USHORT      InsertEntry( const XubString& rStr, const Image& rImage, USHORT nPos = LISTBOX_APPEND );
    virtual void        RemoveEntry( const XubString& rStr );
    virtual void        RemoveEntry( USHORT nPos );

    virtual void        Clear();

    virtual USHORT      GetEntryPos( const XubString& rStr ) const;
    virtual USHORT      GetEntryPos( const void* pData ) const;
    virtual XubString   GetEntry( USHORT nPos ) const;
    virtual USHORT      GetEntryCount() const;

    virtual void        SelectEntry( const XubString& rStr, BOOL bSelect = TRUE );
    virtual void        SelectEntryPos( USHORT nPos, BOOL bSelect = TRUE );

    virtual USHORT      GetSelectEntryCount() const;
    virtual XubString   GetSelectEntry( USHORT nSelIndex = 0 ) const;
    virtual USHORT      GetSelectEntryPos( USHORT nSelIndex = 0 ) const;

    virtual BOOL        IsEntrySelected( const XubString& rStr ) const;
    virtual BOOL        IsEntryPosSelected( USHORT nPos ) const;
    virtual void        SetNoSelection();

    void                SetEntryData( USHORT nPos, void* pNewData );
    void*               GetEntryData( USHORT nPos ) const;

    /** this methods stores a combination of flags from the
        LISTBOX_ENTRY_FLAG_* defines at the given entry.
        See description of the possible LISTBOX_ENTRY_FLAG_* flags
        for details.
        Do not use these flags for user data as they are reserved
        to change the internal behaviour of the ListBox implementation
        for specific entries.
    */
    void            SetEntryFlags( USHORT nPos, long nFlags );

    /** this methods gets the current combination of flags from the
        LISTBOX_ENTRY_FLAG_* defines from the given entry.
        See description of the possible LISTBOX_ENTRY_FLAG_* flags
        for details.
    */
    long            GetEntryFlags( USHORT nPos ) const;

    void            SetTopEntry( USHORT nPos );
    void            SetTopEntryStr( const XubString& rStr );
    USHORT          GetTopEntry() const;

    void            SaveValue() { mnSaveValue = GetSelectEntryPos(); }
    USHORT          GetSavedValue() const { return mnSaveValue; }

    void            SetSeparatorPos( USHORT n );
    void            SetSeparatorPos();
    USHORT          GetSeparatorPos() const;

    BOOL            IsTravelSelect() const;
    BOOL            IsInDropDown() const;
    void            ToggleDropDown();

    void            EnableMultiSelection( BOOL bMulti, BOOL bStackSelection );
    void            EnableMultiSelection( BOOL bMulti );
    BOOL            IsMultiSelectionEnabled() const;

    void            SetReadOnly( BOOL bReadOnly = TRUE );
    BOOL            IsReadOnly() const;

    long            CalcWindowSizePixel( USHORT nLines ) const;
    Rectangle       GetBoundingRectangle( USHORT nItem ) const;

    void            SetUserItemSize( const Size& rSz );
    const Size&     GetUserItemSize() const;

    void            EnableUserDraw( BOOL bUserDraw );
    BOOL            IsUserDrawEnabled() const;

    void            DrawEntry( const UserDrawEvent& rEvt, BOOL bDrawImage, BOOL bDrawText, BOOL bDrawTextAtImagePos = FALSE );

    void            SetSelectHdl( const Link& rLink )       { maSelectHdl = rLink; }
    const Link&     GetSelectHdl() const                    { return maSelectHdl; }
    void            SetDoubleClickHdl( const Link& rLink )  { maDoubleClickHdl = rLink; }
    const Link&     GetDoubleClickHdl() const               { return maDoubleClickHdl; }

    Size            CalcMinimumSize() const;
    virtual Size    GetOptimalSize(WindowSizeType eType) const;
    Size            CalcAdjustedSize( const Size& rPrefSize ) const;
    Size            CalcSize( USHORT nColumns, USHORT nLines ) const;
    void            GetMaxVisColumnsAndLines( USHORT& rnCols, USHORT& rnLines ) const;

    void            SetMRUEntries( const XubString& rEntries, xub_Unicode cSep = ';' );
    XubString       GetMRUEntries( xub_Unicode cSep = ';' ) const;
    void            SetMaxMRUCount( USHORT n );
    USHORT          GetMaxMRUCount() const;

    USHORT          GetDisplayLineCount() const;

    void            EnableMirroring();

    /** checks whether a certain point lies within the bounds of
        a listbox item and returns the item as well as the character position
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

// ----------------
// - MultiListBox -
// ----------------

class VCL_DLLPUBLIC MultiListBox : public ListBox
{
public:
    using ListBox::SaveValue;
    using ListBox::GetSavedValue;
private:
    // Bei MultiListBox nicht erlaubt...
    void            SaveValue();
    USHORT          GetSavedValue();

public:
                    MultiListBox( Window* pParent, WinBits nStyle = 0 );
                    MultiListBox( Window* pParent, const ResId& rResId );
};

#endif  // _SV_LSTBOX_HXX
