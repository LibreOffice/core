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

#include <memory>
#include <vcl/dllapi.h>
#include <vcl/edit.hxx>

#define COMBOBOX_APPEND             (SAL_MAX_INT32)
#define COMBOBOX_ENTRY_NOTFOUND     (SAL_MAX_INT32)
#define COMBOBOX_MAX_ENTRIES        (SAL_MAX_INT32 - 1)

class UserDrawEvent;

class VCL_DLLPUBLIC ComboBox : public Edit
{
private:
    struct Impl;
    std::unique_ptr<Impl> m_pImpl;

protected:
    using Window::ImplInit;
    SAL_DLLPRIVATE void     ImplInit( vcl::Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE static WinBits ImplInitStyle( WinBits nStyle );
    SAL_DLLPRIVATE void     ImplCalcEditHeight();
    SAL_DLLPRIVATE long     getMaxWidthScrollBarAndDownButton() const;

protected:
    bool            IsDropDownBox() const;

    virtual void    FillLayoutData() const override;

public:
    explicit        ComboBox( vcl::Window* pParent, WinBits nStyle = 0 );
    virtual         ~ComboBox() override;
    virtual void    dispose() override;

    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, DrawFlags nFlags ) override;
    virtual void    Resize() override;
    virtual bool    EventNotify( NotifyEvent& rNEvt ) override;
    virtual void    StateChanged( StateChangedType nType ) override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;

    virtual void    UserDraw( const UserDrawEvent& rUDEvt );
    virtual void    Select();
    void            DoubleClick();

    virtual void    Modify() override;

    virtual const Wallpaper& GetDisplayBackground() const override;

    virtual void    setPosSizePixel( long nX, long nY, long nWidth, long nHeight, PosSizeFlags nFlags = PosSizeFlags::All ) override;

    tools::Rectangle       GetDropDownPosSizePixel() const;

    void            AdaptDropDownLineCountToMaximum();
    void            SetDropDownLineCount( sal_uInt16 nLines );
    sal_uInt16      GetDropDownLineCount() const;

    void            EnableAutoSize( bool bAuto );
    bool            IsAutoSizeEnabled() const;

    void            EnableDDAutoWidth( bool b );

    virtual void    SetText( const OUString& rStr ) override;
    virtual void    SetText( const OUString& rStr, const Selection& rNewSelection ) override;

    virtual sal_Int32  InsertEntry(const OUString& rStr, sal_Int32  nPos = COMBOBOX_APPEND);
    sal_Int32       InsertEntryWithImage( const OUString& rStr, const Image& rImage, sal_Int32  nPos = COMBOBOX_APPEND );

    void            RemoveEntry( const OUString& rStr );
    virtual void    RemoveEntryAt(sal_Int32  nPos);

    void            Clear();
    void            EnableSelectAll();
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

    void            SetSeparatorPos( sal_Int32  n );

    void            EnableAutocomplete( bool bEnable, bool bMatchCase = false );
    bool            IsAutocompleteEnabled() const;

    void            EnableMultiSelection( bool bMulti );
    bool            IsMultiSelectionEnabled() const;

    void            SetSelectHdl(const Link<ComboBox&,void>& rLink);
    const Link<ComboBox&,void>&   GetSelectHdl() const;
    void            SetDoubleClickHdl(const Link<ComboBox&,void>& rLink);
    const Link<ComboBox&,void>&   GetDoubleClickHdl() const;
    void            SetEntryActivateHdl(const Link<Edit&,void>& rLink);

    Size            CalcMinimumSize() const override;
    virtual Size    GetOptimalSize() const override;
    Size            CalcAdjustedSize( const Size& rPrefSize ) const;
    Size            CalcBlockSize( sal_uInt16 nColumns, sal_uInt16 nLines ) const;
    void            GetMaxVisColumnsAndLines( sal_uInt16& rnCols, sal_uInt16& rnLines ) const;

    void            SetMRUEntries( const OUString& rEntries );
    OUString        GetMRUEntries() const;
    void            SetMaxMRUCount( sal_Int32  n );
    sal_Int32       GetMaxMRUCount() const;
    void            SetEntryData( sal_Int32  nPos, void* pNewData );
    void*           GetEntryData( sal_Int32  nPos ) const;

    sal_Int32       GetTopEntry() const;

    void            SetProminentEntryType( ProminentEntry eType );

    sal_uInt16      GetDisplayLineCount() const;

    sal_Int32       GetSelectedEntryCount() const;
    sal_Int32       GetSelectedEntryPos( sal_Int32  nSelIndex = 0 ) const;
    OUString        GetSelectedEntry() const { return GetEntry( GetSelectedEntryPos() ); }
    bool            IsEntryPosSelected( sal_Int32  nPos ) const;
    void            SelectEntryPos( sal_Int32  nPos, bool bSelect = true );
    void            SetNoSelection();
    tools::Rectangle       GetBoundingRectangle( sal_Int32  nItem ) const;

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

    virtual bool set_property(const OString &rKey, const OUString &rValue) override;

    virtual FactoryFunction GetUITestFactory() const override;
};

#endif  // _COMBOBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
