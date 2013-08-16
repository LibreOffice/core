/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
    sal_uInt16                  mnDDHeight;
    sal_uInt16                  mnSaveValue;
    sal_Int32 m_nMaxWidthChars;
    Link                        maSelectHdl;
    Link                        maDoubleClickHdl;
    sal_uInt16                  mnLineCount;

    /// bitfield
    bool            mbDDAutoSize : 1;
    bool            mbEdgeBlending : 1;

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
    sal_Bool               IsDropDownBox() const { return mpFloatWin ? sal_True : sal_False; }

protected:
    explicit            ListBox( WindowType nType );

    virtual void        FillLayoutData() const;

public:
    explicit            ListBox( Window* pParent, WinBits nStyle = WB_BORDER );
    explicit            ListBox( Window* pParent, const ResId& );
    virtual             ~ListBox();

    virtual void        Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags );
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

    virtual void        setPosSizePixel( long nX, long nY,
                                         long nWidth, long nHeight, sal_uInt16 nFlags = WINDOW_POSSIZE_ALL );
    void                SetPosSizePixel( const Point& rNewPos, const Size& rNewSize )
                        { Control::SetPosSizePixel( rNewPos, rNewSize ); }
    void                SetDropDownSizePixel( const Size& rNewSize )
    { if( IsDropDownBox() ) setPosSizePixel( 0, 0, rNewSize.Width(), rNewSize.Height(), WINDOW_POSSIZE_SIZE | WINDOW_POSSIZE_DROPDOWN ); }

    Rectangle           GetDropDownPosSizePixel() const;

    void                AdaptDropDownLineCountToMaximum();
    void                SetDropDownLineCount( sal_uInt16 nLines );
    sal_uInt16          GetDropDownLineCount() const;

    void                EnableAutoSize( bool bAuto );
    bool                IsAutoSizeEnabled() const { return mbDDAutoSize; }

    void                EnableDDAutoWidth( sal_Bool b );

    sal_uInt16          InsertEntry( const OUString& rStr, sal_uInt16 nPos = LISTBOX_APPEND );
    sal_uInt16          InsertEntry( const OUString& rStr, const Image& rImage, sal_uInt16 nPos = LISTBOX_APPEND );
    void                RemoveEntry( const OUString& rStr );
    void                RemoveEntry( sal_uInt16 nPos );

    void                Clear();

    sal_uInt16          GetEntryPos( const OUString& rStr ) const;
    sal_uInt16          GetEntryPos( const void* pData ) const;
    Image               GetEntryImage( sal_uInt16 nPos ) const;
    OUString            GetEntry( sal_uInt16 nPos ) const;
    sal_uInt16          GetEntryCount() const;

    void                SelectEntry( const OUString& rStr, sal_Bool bSelect = sal_True );
    void                SelectEntryPos( sal_uInt16 nPos, sal_Bool bSelect = sal_True );

    sal_uInt16          GetSelectEntryCount() const;
    OUString            GetSelectEntry( sal_uInt16 nSelIndex = 0 ) const;
    sal_uInt16          GetSelectEntryPos( sal_uInt16 nSelIndex = 0 ) const;

    bool                IsEntrySelected(const OUString& rStr) const;
    sal_Bool            IsEntryPosSelected( sal_uInt16 nPos ) const;
    void                SetNoSelection();

    void                SetEntryData( sal_uInt16 nPos, void* pNewData );
    void*               GetEntryData( sal_uInt16 nPos ) const;

    /** this methods stores a combination of flags from the
        LISTBOX_ENTRY_FLAG_* defines at the given entry.
        See description of the possible LISTBOX_ENTRY_FLAG_* flags
        for details.
        Do not use these flags for user data as they are reserved
        to change the internal behaviour of the ListBox implementation
        for specific entries.
    */
    void                SetEntryFlags( sal_uInt16 nPos, long nFlags );

    /** this methods gets the current combination of flags from the
        LISTBOX_ENTRY_FLAG_* defines from the given entry.
        See description of the possible LISTBOX_ENTRY_FLAG_* flags
        for details.
    */
    long                GetEntryFlags( sal_uInt16 nPos ) const;

    void                SetTopEntry( sal_uInt16 nPos );
    sal_uInt16          GetTopEntry() const;

    void                SaveValue() { mnSaveValue = GetSelectEntryPos(); }
    sal_uInt16          GetSavedValue() const { return mnSaveValue; }

    void                SetSeparatorPos( sal_uInt16 n = LISTBOX_ENTRY_NOTFOUND );
    sal_uInt16          GetSeparatorPos() const;

    sal_Bool            IsTravelSelect() const;
    sal_Bool            IsInDropDown() const;
    void                ToggleDropDown();

    void                EnableMultiSelection( sal_Bool bMulti, sal_Bool bStackSelection );
    void                EnableMultiSelection( sal_Bool bMulti );
    sal_Bool            IsMultiSelectionEnabled() const;

    void                SetReadOnly( sal_Bool bReadOnly = sal_True );
    sal_Bool            IsReadOnly() const;

    Rectangle           GetBoundingRectangle( sal_uInt16 nItem ) const;

    void                SetUserItemSize( const Size& rSz );

    void                EnableUserDraw( sal_Bool bUserDraw );

    void                DrawEntry( const UserDrawEvent& rEvt, sal_Bool bDrawImage, sal_Bool bDrawText, sal_Bool bDrawTextAtImagePos = sal_False );

    void                SetSelectHdl( const Link& rLink )       { maSelectHdl = rLink; }
    const Link&         GetSelectHdl() const                    { return maSelectHdl; }
    void                SetDoubleClickHdl( const Link& rLink )  { maDoubleClickHdl = rLink; }
    const Link&         GetDoubleClickHdl() const               { return maDoubleClickHdl; }

    Size                CalcSubEditSize() const;    //size of area inside lstbox, i.e. no scrollbar/dropdown
    Size                CalcMinimumSize() const;    //size of lstbox area, i.e. including scrollbar/dropdown
    virtual Size        GetOptimalSize() const;
    Size                CalcAdjustedSize( const Size& rPrefSize ) const;
    Size                CalcSize( sal_uInt16 nColumns, sal_uInt16 nLines ) const;
    void                GetMaxVisColumnsAndLines( sal_uInt16& rnCols, sal_uInt16& rnLines ) const;

    sal_uInt16          GetDisplayLineCount() const;

    void                EnableMirroring();

    bool                GetEdgeBlending() const { return mbEdgeBlending; }
    void                SetEdgeBlending(bool bNew);

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
    long GetIndexForPoint( const Point& rPoint, sal_uInt16& rPos ) const;

    sal_Int32 getMaxWidthChars() const { return m_nMaxWidthChars; }
    void setMaxWidthChars(sal_Int32 nWidth);

    virtual bool set_property(const OString &rKey, const OString &rValue);
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
    sal_uInt16      GetSavedValue();

public:
    explicit        MultiListBox( Window* pParent, WinBits nStyle = 0 );
    explicit        MultiListBox( Window* pParent, const ResId& rResId );

};

#endif  // _SV_LSTBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
