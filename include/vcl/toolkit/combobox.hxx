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

#pragma once

#if !defined(VCL_DLLIMPLEMENTATION) && !defined(TOOLKIT_DLLIMPLEMENTATION) && !defined(VCL_INTERNALS)
#error "don't use this in new code"
#endif

#include <memory>
#include <vcl/dllapi.h>
#include <vcl/toolkit/edit.hxx>

#define COMBOBOX_APPEND             (SAL_MAX_INT32)
#define COMBOBOX_ENTRY_NOTFOUND     (SAL_MAX_INT32)
#define COMBOBOX_MAX_ENTRIES        (SAL_MAX_INT32 - 1)

class ImplListBoxWindow;
class UserDrawEvent;

/// A widget used to choose from a list of items and which has an entry.
class VCL_DLLPUBLIC ComboBox : public Edit
{
private:
    struct SAL_DLLPRIVATE Impl;
    std::unique_ptr<Impl> m_pImpl;

protected:
    using Window::ImplInit;
    SAL_DLLPRIVATE void     ImplInit( vcl::Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE static WinBits ImplInitStyle( WinBits nStyle );
    SAL_DLLPRIVATE void     ImplCalcEditHeight();
    SAL_DLLPRIVATE tools::Long     getMaxWidthScrollBarAndDownButton() const;

protected:
    SAL_DLLPRIVATE bool IsDropDownBox() const;

    virtual void    FillLayoutData() const override;

public:
    explicit        ComboBox( vcl::Window* pParent, WinBits nStyle = 0 );
    virtual         ~ComboBox() override;
    virtual void    dispose() override;

    virtual void    Draw( OutputDevice* pDev, const Point& rPos,SystemTextColorFlags nFlags ) override;
    virtual void    Resize() override;
    virtual bool    EventNotify( NotifyEvent& rNEvt ) override;
    virtual void    StateChanged( StateChangedType nType ) override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;

    void            Select();
    SAL_DLLPRIVATE void DoubleClick();

    virtual void    Modify() override;

    virtual const Wallpaper& GetDisplayBackground() const override;

    virtual void    setPosSizePixel( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight, PosSizeFlags nFlags = PosSizeFlags::All ) override;

    tools::Rectangle       GetDropDownPosSizePixel() const;

    SAL_DLLPRIVATE void AdaptDropDownLineCountToMaximum();
    void            SetDropDownLineCount( sal_uInt16 nLines );
    sal_uInt16      GetDropDownLineCount() const;

    void            EnableAutoSize( bool bAuto );
    SAL_DLLPRIVATE bool IsAutoSizeEnabled() const;

    virtual void    SetText( const OUString& rStr ) override;
    virtual void    SetText( const OUString& rStr, const Selection& rNewSelection ) override;

    sal_Int32       InsertEntry(const OUString& rStr, sal_Int32  nPos = COMBOBOX_APPEND);
    sal_Int32       InsertEntryWithImage( const OUString& rStr, const Image& rImage, sal_Int32  nPos = COMBOBOX_APPEND );

    void            RemoveEntryAt(sal_Int32  nPos);

    void            Clear();
    sal_Int32       GetEntryPos( std::u16string_view rStr ) const;
    Image           GetEntryImage( sal_Int32  nPos ) const;
    OUString        GetEntry( sal_Int32  nPos ) const;
    sal_Int32       GetEntryCount() const;

    bool            IsTravelSelect() const;
    bool            IsInDropDown() const;
    void            ToggleDropDown();

    SAL_DLLPRIVATE void SetUserItemSize( const Size& rSz );
    SAL_DLLPRIVATE void EnableUserDraw( bool bUserDraw );
    SAL_DLLPRIVATE bool IsUserDrawEnabled() const;

    SAL_DLLPRIVATE void DrawEntry( const UserDrawEvent& rEvt );
    void            SetBorderStyle( WindowBorderStyle nBorderStyle );
    void            SetHighlightColor(const Color& rColor);
    void            SetHighlightTextColor(const Color& rColor);

    /**
     * Adds a new separator at the given position n.
     */
    SAL_DLLPRIVATE void AddSeparator( sal_Int32 n );

    void            EnableAutocomplete( bool bEnable, bool bMatchCase = false );
    bool            IsAutocompleteEnabled() const;

    bool            IsMultiSelectionEnabled() const;

    SAL_DLLPRIVATE void SetSelectHdl(const Link<ComboBox&,void>& rLink);
    SAL_DLLPRIVATE void SetEntryActivateHdl(const Link<Edit&,bool>& rLink);
    SAL_DLLPRIVATE void SetUserDrawHdl(const Link<UserDrawEvent*, void>& rLink);

    Size            CalcMinimumSize() const override;
    virtual Size    GetOptimalSize() const override;
    Size            CalcAdjustedSize( const Size& rPrefSize ) const;
    Size            CalcBlockSize( sal_uInt16 nColumns, sal_uInt16 nLines ) const;
    void            GetMaxVisColumnsAndLines( sal_uInt16& rnCols, sal_uInt16& rnLines ) const;

    SAL_DLLPRIVATE void     SetMRUEntries( std::u16string_view rEntries );
    SAL_DLLPRIVATE OUString GetMRUEntries() const;
    SAL_DLLPRIVATE void     SetMaxMRUCount( sal_Int32  n );
    SAL_DLLPRIVATE sal_Int32 GetMaxMRUCount() const;
    SAL_DLLPRIVATE void  SetEntryData( sal_Int32  nPos, void* pNewData );
    SAL_DLLPRIVATE void* GetEntryData( sal_Int32  nPos ) const;

    sal_Int32       GetTopEntry() const;

    sal_uInt16      GetDisplayLineCount() const;

    sal_Int32       GetSelectedEntryCount() const;
    sal_Int32       GetSelectedEntryPos( sal_Int32  nSelIndex = 0 ) const;
    bool            IsEntryPosSelected( sal_Int32  nPos ) const;
    void            SelectEntryPos( sal_Int32  nPos, bool bSelect = true );
    void            SetNoSelection();
    tools::Rectangle       GetBoundingRectangle( sal_Int32  nItem ) const;

    // determine if Select was called due to typing or cursoring in the
    // combobox, as opposed to something selected from the menu or via some
    // other route.  e.g. the toolbar fontsize combobox wants to immediately
    // change size only if something is picked from the combobox menu, other
    // changes don't auto-apply until the user presses return
    SAL_DLLPRIVATE bool IsModifyByKeyboard() const;

    // determine if Edit::Modify was called due to the ComboBox changing the edit area
    // itself
    SAL_DLLPRIVATE bool IsSyntheticModify() const;


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
    tools::Long GetIndexForPoint( const Point& rPoint, sal_Int32 & rPos ) const;

    SAL_DLLPRIVATE void setMaxWidthChars(sal_Int32 nWidth);

    SAL_DLLPRIVATE void SetWidthInChars(sal_Int32 nWidthInChars);

    SAL_DLLPRIVATE tools::Long GetDropDownEntryHeight() const;

    // the drop down window container
    SAL_DLLPRIVATE ImplListBoxWindow* GetMainWindow() const;

    virtual bool set_property(const OUString &rKey, const OUString &rValue) override;

    virtual FactoryFunction GetUITestFactory() const override;

    virtual void DumpAsPropertyTree(tools::JsonWriter&) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
