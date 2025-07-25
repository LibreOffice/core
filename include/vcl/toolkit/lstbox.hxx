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

#include <config_options.h>
#include <sal/types.h>
#include <o3tl/typed_flags_set.hxx>
#include <vcl/dllapi.h>
#include <vcl/ctrl.hxx>

class DataChangedEvent;
class NotifyEvent;
class UserDrawEvent;

#define LISTBOX_APPEND              (SAL_MAX_INT32)
#define LISTBOX_ENTRY_NOTFOUND      (SAL_MAX_INT32)
#define LISTBOX_MAX_ENTRIES         (SAL_MAX_INT32 - 1)

// the following defines can be used for the SetEntryFlags()
// and GetEntryFlags() methods

// !! Do not use these flags for user data as they are reserved      !!
// !! to change the internal behaviour of the ListBox implementation !!
// !! for specific entries.                                          !!

enum class ListBoxEntryFlags
{
    NONE                    = 0x0000,
/** this flag disables a selection of an entry completely. It is not
    possible to select such entries either from the user interface
    nor from the ListBox methods. Cursor traveling is handled correctly.
    This flag can be used to add titles to a ListBox.
*/
    DisableSelection        = 0x0001,

/** this flag can be used to make an entry multiline capable
    A normal entry is single line and will therefore be clipped
    at the right listbox border. Setting this flag enables
    word breaks for the entry text.
*/
    MultiLine               = 0x0002,

/** this flags lets the item be drawn disabled (e.g. in grey text)
    usage only guaranteed with ListBoxEntryFlags::DisableSelection
*/
    DrawDisabled            = 0x0004,
};
namespace o3tl
{
    template<> struct typed_flags<ListBoxEntryFlags> : is_typed_flags<ListBoxEntryFlags, 0x0007> {};
}

class Image;
class ImplListBox;
class ImplListBoxFloatingWindow;
class ImplBtn;
class ImplWin;
class ImplListBoxWindow;

/// A widget used to choose from a list of items and which has no entry.
class VCL_DLLPUBLIC ListBox : public Control
{
private:
    VclPtr<ImplListBox>                mpImplLB;
    VclPtr<ImplListBoxFloatingWindow>  mpFloatWin;
    VclPtr<ImplWin>                    mpImplWin;
    VclPtr<ImplBtn>                    mpBtn;
    sal_uInt16                  mnDDHeight;
    sal_Int32                   m_nMaxWidthChars;
    Link<ListBox&,void>          maSelectHdl;
    sal_uInt16                  mnLineCount;

    bool            mbDDAutoSize : 1;

private:
    SAL_DLLPRIVATE void    ImplInitListBoxData();

    DECL_DLLPRIVATE_LINK( ImplSelectHdl, LinkParamNone*, void );
    DECL_DLLPRIVATE_LINK( ImplScrollHdl, ImplListBox*, void );
    DECL_DLLPRIVATE_LINK( ImplCancelHdl, LinkParamNone*, void );
    DECL_DLLPRIVATE_LINK( ImplDoubleClickHdl, ImplListBoxWindow*, void );
    DECL_DLLPRIVATE_LINK( ImplPopupModeEndHdl, FloatingWindow*, void );
    DECL_DLLPRIVATE_LINK( ImplSelectionChangedHdl, sal_Int32, void );
    DECL_DLLPRIVATE_LINK( ImplFocusHdl, sal_Int32, void );
    DECL_DLLPRIVATE_LINK( ImplListItemSelectHdl, LinkParamNone*, void );

    DECL_DLLPRIVATE_LINK( ImplClickBtnHdl, void*, void );

protected:
    using Window::ImplInit;
    SAL_DLLPRIVATE void    ImplInit( vcl::Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE static WinBits ImplInitStyle( WinBits nStyle );
    bool               IsDropDownBox() const { return mpFloatWin != nullptr; }

protected:
    SAL_DLLPRIVATE explicit ListBox( WindowType eType );

    SAL_DLLPRIVATE virtual void FillLayoutData() const override;

public:
    explicit            ListBox( vcl::Window* pParent, WinBits nStyle = WB_BORDER );
    SAL_DLLPRIVATE virtual ~ListBox() override;
    SAL_DLLPRIVATE virtual void dispose() override;

    virtual rtl::Reference<comphelper::OAccessible> CreateAccessible() override;

    SAL_DLLPRIVATE virtual void ApplySettings(vcl::RenderContext& rRenderContext) override;
    SAL_DLLPRIVATE virtual void Draw( OutputDevice* pDev, const Point& rPos, SystemTextColorFlags nFlags ) override;
    SAL_DLLPRIVATE virtual void Resize() override;
    SAL_DLLPRIVATE virtual bool PreNotify( NotifyEvent& rNEvt ) override;
    SAL_DLLPRIVATE virtual void StateChanged( StateChangedType nType ) override;
    SAL_DLLPRIVATE virtual void DataChanged( const DataChangedEvent& rDCEvt ) override;

    void                Select();
    SAL_DLLPRIVATE void DoubleClick();
    SAL_DLLPRIVATE virtual void GetFocus() override;
    SAL_DLLPRIVATE virtual void LoseFocus() override;

    SAL_DLLPRIVATE virtual const Wallpaper& GetDisplayBackground() const override;

    SAL_DLLPRIVATE virtual void setPosSizePixel( tools::Long nX, tools::Long nY,
                                         tools::Long nWidth, tools::Long nHeight, PosSizeFlags nFlags = PosSizeFlags::All ) override;

    tools::Rectangle           GetDropDownPosSizePixel() const;

    SAL_DLLPRIVATE void AdaptDropDownLineCountToMaximum();
    void                SetDropDownLineCount( sal_uInt16 nLines );
    sal_uInt16          GetDropDownLineCount() const;

    void                EnableAutoSize( bool bAuto );
    bool                IsAutoSizeEnabled() const { return mbDDAutoSize; }

    sal_Int32           InsertEntry( const OUString& rStr, sal_Int32  nPos = LISTBOX_APPEND );
    sal_Int32           InsertEntry( const OUString& rStr, const Image& rImage, sal_Int32  nPos = LISTBOX_APPEND );
    void                RemoveEntry( sal_Int32  nPos );

    void                Clear();

    sal_Int32           GetEntryPos( std::u16string_view rStr ) const;
    Image               GetEntryImage( sal_Int32  nPos ) const;
    OUString            GetEntry( sal_Int32  nPos ) const;
    sal_Int32           GetEntryCount() const;

    SAL_DLLPRIVATE void SelectEntry( std::u16string_view rStr, bool bSelect = true );
    void                SelectEntryPos( sal_Int32  nPos, bool bSelect = true );
    void                SelectEntriesPos( const std::vector<sal_Int32>& rPositions, bool bSelect = true );

    sal_Int32           GetSelectedEntryCount() const;
    OUString            GetSelectedEntry( sal_Int32  nSelIndex = 0 ) const;
    sal_Int32           GetSelectedEntryPos( sal_Int32  nSelIndex = 0 ) const;

    bool                IsEntryPosSelected( sal_Int32  nPos ) const;
    void                SetNoSelection();

    SAL_DLLPRIVATE void  SetEntryData( sal_Int32  nPos, void* pNewData );
    SAL_DLLPRIVATE void* GetEntryData( sal_Int32  nPos ) const;

    /** this methods stores a combination of flags from the
        ListBoxEntryFlags::* defines at the given entry.
        See description of the possible ListBoxEntryFlags::* flags
        for details.
        Do not use these flags for user data as they are reserved
        to change the internal behaviour of the ListBox implementation
        for specific entries.
    */
    SAL_DLLPRIVATE void SetEntryFlags( sal_Int32  nPos, ListBoxEntryFlags nFlags );

    void                SetTopEntry( sal_Int32  nPos );
    sal_Int32           GetTopEntry() const;

    /**
     * Removes existing separators, and sets the position of the
     * one and only separator.
     */
    void                SetSeparatorPos( sal_Int32 n );
    /**
     * Gets the position of the separator which was added first.
     * Returns LISTBOX_ENTRY_NOTFOUND if there is no separator.
     */
    sal_Int32           GetSeparatorPos() const;

    /**
     * Adds a new separator at the given position n.
     */
    SAL_DLLPRIVATE void AddSeparator( sal_Int32 n );

    SAL_DLLPRIVATE bool IsTravelSelect() const;
    bool                IsInDropDown() const;
    void                ToggleDropDown();

    void                EnableMultiSelection( bool bMulti );
    bool                IsMultiSelectionEnabled() const;
    void                SetHighlightColor(const Color& rColor);
    void                SetHighlightTextColor(const Color& rColor);

    void                SetReadOnly( bool bReadOnly = true );
    bool                IsReadOnly() const;

    tools::Rectangle           GetBoundingRectangle( sal_Int32  nItem ) const;

    void                SetSelectHdl( const Link<ListBox&,void>& rLink )     { maSelectHdl = rLink; }

    SAL_DLLPRIVATE Size CalcSubEditSize() const;    //size of area inside lstbox, i.e. no scrollbar/dropdown
    Size                CalcMinimumSize() const;    //size of lstbox area, i.e. including scrollbar/dropdown
    SAL_DLLPRIVATE virtual Size GetOptimalSize() const override;
    Size                CalcAdjustedSize( const Size& rPrefSize ) const;
    Size                CalcBlockSize( sal_uInt16 nColumns, sal_uInt16 nLines ) const;
    void                GetMaxVisColumnsAndLines( sal_uInt16& rnCols, sal_uInt16& rnLines ) const;

    sal_uInt16          GetDisplayLineCount() const;

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
    tools::Long GetIndexForPoint( const Point& rPoint, sal_Int32 & rPos ) const;

    SAL_DLLPRIVATE void setMaxWidthChars(sal_Int32 nWidth);

    SAL_DLLPRIVATE virtual bool set_property(const OUString &rKey, const OUString &rValue) override;

    SAL_DLLPRIVATE virtual FactoryFunction GetUITestFactory() const override;

    SAL_DLLPRIVATE virtual void DumpAsPropertyTree(tools::JsonWriter&) override;
};

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) MultiListBox final : public ListBox
{
public:
    explicit        MultiListBox( vcl::Window* pParent, WinBits nStyle );

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
