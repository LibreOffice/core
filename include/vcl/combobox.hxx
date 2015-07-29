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

#ifndef INCLUDED_VCL_COMBOBOX_HXX
#define INCLUDED_VCL_COMBOBOX_HXX

#include <vcl/dllapi.h>
#include <vcl/combobox.h>
#include <vcl/edit.hxx>

class UserDrawEvent;

class ImplListBoxFloatingWindow;
class ImplListBox;
class ImplBtn;

class VCL_DLLPUBLIC ComboBox : public Edit
{
private:
    struct Impl;
    std::unique_ptr<Impl> m_pImpl;

protected:
    using Window::ImplInit;
    SAL_DLLPRIVATE void     ImplInit( vcl::Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE WinBits  ImplInitStyle( WinBits nStyle );
    SAL_DLLPRIVATE void     ImplLoadRes( const ResId& rResId );
    SAL_DLLPRIVATE void     ImplCalcEditHeight();
    SAL_DLLPRIVATE long     getMaxWidthScrollBarAndDownButton() const;

protected:
    bool            IsDropDownBox() const;

    virtual void    FillLayoutData() const SAL_OVERRIDE;

public:
    explicit        ComboBox( vcl::Window* pParent, WinBits nStyle = 0 );
    explicit        ComboBox( vcl::Window* pParent, const ResId& );
    virtual         ~ComboBox();
    virtual void    dispose() SAL_OVERRIDE;

    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, DrawFlags nFlags ) SAL_OVERRIDE;
    virtual void    Resize() SAL_OVERRIDE;
    virtual bool    PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual bool    Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual void    StateChanged( StateChangedType nType ) SAL_OVERRIDE;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

    virtual void    UserDraw( const UserDrawEvent& rUDEvt );
    virtual void    Select();
    virtual void    DoubleClick();

    virtual void    Modify() SAL_OVERRIDE;

    virtual const Wallpaper& GetDisplayBackground() const SAL_OVERRIDE;

    virtual void    setPosSizePixel( long nX, long nY, long nWidth, long nHeight, PosSizeFlags nFlags = PosSizeFlags::All ) SAL_OVERRIDE;
    void            SetPosSizePixel( const Point& rNewPos, const Size& rNewSize ) SAL_OVERRIDE
                        { Edit::SetPosSizePixel( rNewPos, rNewSize ); }

    Rectangle       GetDropDownPosSizePixel() const;

    void            AdaptDropDownLineCountToMaximum();
    void            SetDropDownLineCount( sal_uInt16 nLines );
    sal_uInt16      GetDropDownLineCount() const;

    void            EnableAutoSize( bool bAuto );
    bool            IsAutoSizeEnabled() const;

    void            EnableDDAutoWidth( bool b );

    virtual void    SetText( const OUString& rStr ) SAL_OVERRIDE;
    virtual void    SetText( const OUString& rStr, const Selection& rNewSelection ) SAL_OVERRIDE;

    virtual sal_Int32  InsertEntry(const OUString& rStr, sal_Int32  nPos = COMBOBOX_APPEND);
    sal_Int32       InsertEntryWithImage( const OUString& rStr, const Image& rImage, sal_Int32  nPos = COMBOBOX_APPEND );

    void            RemoveEntry( const OUString& rStr );
    virtual void    RemoveEntryAt(sal_Int32  nPos);

    void            Clear();

    sal_Int32       GetEntryPos( const OUString& rStr ) const;
    sal_Int32       GetEntryPos( const void* pData ) const;
    Image           GetEntryImage( sal_Int32  nPos ) const;
    OUString        GetEntry( sal_Int32  nPos ) const;
    sal_Int32       GetEntryCount() const;

    bool            IsTravelSelect() const;
    bool            IsInDropDown() const;
    void            ToggleDropDown();

    long            CalcWindowSizePixel( sal_uInt16 nLines ) const;

    void            SetUserItemSize( const Size& rSz );
    void            EnableUserDraw( bool bUserDraw );

    void            DrawEntry( const UserDrawEvent& rEvt, bool bDrawImage, bool bDrawText, bool bDrawTextAtImagePos = false );
    void            SetBorderStyle( WindowBorderStyle nBorderStyle );

    void            SetSeparatorPos( sal_Int32  n = COMBOBOX_ENTRY_NOTFOUND );

    void            EnableAutocomplete( bool bEnable, bool bMatchCase = false );
    bool            IsAutocompleteEnabled() const;

    void            EnableMultiSelection( bool bMulti );
    bool            IsMultiSelectionEnabled() const;
    sal_Unicode     GetMultiSelectionSeparator() const;

    void            SetSelectHdl(const Link<>& rLink);
    const Link<>&   GetSelectHdl() const;
    void            SetDoubleClickHdl(const Link<>& rLink);
    const Link<>&   GetDoubleClickHdl() const;

    Size            CalcMinimumSize() const SAL_OVERRIDE;
    virtual Size    GetOptimalSize() const SAL_OVERRIDE;
    Size            CalcAdjustedSize( const Size& rPrefSize ) const;
    Size            CalcBlockSize( sal_uInt16 nColumns, sal_uInt16 nLines ) const;
    void            GetMaxVisColumnsAndLines( sal_uInt16& rnCols, sal_uInt16& rnLines ) const;

    void            SetMRUEntries( const OUString& rEntries, sal_Unicode cSep = ';' );
    OUString        GetMRUEntries( sal_Unicode cSep = ';' ) const;
    void            SetMaxMRUCount( sal_Int32  n );
    sal_Int32       GetMaxMRUCount() const;
    void            SetEntryData( sal_Int32  nPos, void* pNewData );
    void*           GetEntryData( sal_Int32  nPos ) const;

    sal_Int32       GetTopEntry() const;

    void            SetProminentEntryType( ProminentEntry eType );

    sal_uInt16      GetDisplayLineCount() const;

    sal_Int32       GetSelectEntryCount() const;
    sal_Int32       GetSelectEntryPos( sal_Int32  nSelIndex = 0 ) const;
    OUString        GetSelectEntry() const { return GetEntry( GetSelectEntryPos() ); }
    bool            IsEntryPosSelected( sal_Int32  nPos ) const;
    void            SelectEntryPos( sal_Int32  nPos, bool bSelect = true );
    void            SetNoSelection();
    Rectangle       GetBoundingRectangle( sal_Int32  nItem ) const;

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
    long GetIndexForPoint( const Point& rPoint, sal_Int32 & rPos ) const;

    void setMaxWidthChars(sal_Int32 nWidth);

    virtual bool set_property(const OString &rKey, const OString &rValue) SAL_OVERRIDE;
};

#endif  // _COMBOBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
