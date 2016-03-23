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

#ifndef INCLUDED_SC_SOURCE_UI_INC_OLINEWIN_HXX
#define INCLUDED_SC_SOURCE_UI_INC_OLINEWIN_HXX

#include "viewdata.hxx"

class ScOutlineEntry;
class ScOutlineArray;

enum ScOutlineMode { SC_OUTLINE_HOR, SC_OUTLINE_VER };

/** The window left of or above the spreadsheet containing the outline groups
    and controls to expand/collapse them. */
class ScOutlineWindow : public vcl::Window
{
private:
    ScViewData&                 mrViewData;         /// View data containing the document.
    ScSplitPos                  meWhich;            /// Which area in split window.
    bool                        mbHoriz;            /// true = Horizontal orientation.
    bool                        mbMirrorEntries;    /// true = mirror the order of entries (including header)
    bool                        mbMirrorLevels;     /// true = mirror the order of levels, including the border

    ImageList*                  mpSymbols;          /// Symbols for buttons.
    Color                       maLineColor;        /// Line color for expanded groups.
    long                        mnHeaderSize;       /// Size of the header area in entry direction.
    long                        mnHeaderPos;        /// Position of the header area in entry direction.
    long                        mnMainFirstPos;     /// First position of main area in entry direction.
    long                        mnMainLastPos;      /// Last position of main area in entry direction.

    size_t                      mnMTLevel;          /// Mouse tracking: Level of active button.
    size_t                      mnMTEntry;          /// Mouse tracking: Entry index of active button.
    bool                        mbMTActive;         /// Mouse tracking active?
    bool                        mbMTPressed;        /// Mouse tracking: Button currently drawed pressed?

    Rectangle                   maFocusRect;        /// Focus rectangle on screen.
    size_t                      mnFocusLevel;       /// Level of focused button.
    size_t                      mnFocusEntry;       /// Entry index of focused button.
    bool                        mbDontDrawFocus;    /// Do not redraw focus in next Paint().

public:
                                ScOutlineWindow(
                                    vcl::Window* pParent,
                                    ScOutlineMode eMode,
                                    ScViewData* pViewData,
                                    ScSplitPos eWhich );
    virtual                     ~ScOutlineWindow();
    virtual void                dispose() override;

    /** Sets the size of the header area (width/height dep. on window type). */
    void                        SetHeaderSize( long nNewSize );
    /** Returns the width/height the window needs to show all levels. */
    long                        GetDepthSize() const;

    /** Scrolls the window content by the specified amount of pixels. */
    void                        ScrollPixel( long nDiff );

    using Window::ShowFocus;

private:
    /** Initializes color and image settings. */
    void                        InitSettings();

    /** Returns the calc document. */
    inline ScDocument&          GetDoc() const { return *mrViewData.GetDocument(); }
    /** Returns the current sheet index. */
    inline SCTAB                GetTab() const { return mrViewData.GetTabNo(); }
    /** Returns the outline array of the corresponding document. */
    const ScOutlineArray*       GetOutlineArray() const;
    /** Returns the specified outline entry. */
    const ScOutlineEntry*       GetOutlineEntry( size_t nLevel, size_t nEntry ) const;

    /** Returns true, if the column/row is hidden. */
    bool                        IsHidden( SCCOLROW nColRowIndex ) const;
    /** Returns true, if the column/row is filtered. */
    bool                        IsFiltered( SCCOLROW nColRowIndex ) const;
    /** Returns true, if all columns/rows before nColRowIndex are hidden. */
    bool                        IsFirstVisible( SCCOLROW nColRowIndex ) const;
    /** Returns the currently visible column/row range. */
    void                        GetVisibleRange( SCCOLROW& rnColRowStart, SCCOLROW& rnColRowEnd ) const;

    /** Returns the point in the window of the specified position. */
    Point                       GetPoint( long nLevelPos, long nEntryPos ) const;
    /** Returns the rectangle in the window of the specified position. */
    Rectangle                   GetRectangle(
                                    long nLevelStart, long nEntryStart,
                                    long nLevelEnd, long nEntryEnd ) const;

    /** Returns the window size for the level coordinate. */
    long                        GetOutputSizeLevel() const;
    /** Returns the window size for the entry coordinate. */
    long                        GetOutputSizeEntry() const;

    /** Returns the count of levels of the outline array. 0 means no outlines. */
    size_t                      GetLevelCount() const;
    /** Returns the pixel position of the specified level. */
    long                        GetLevelPos( size_t nLevel ) const;
    /** Returns the level of the passed pixel position. */
    size_t                      GetLevelFromPos( long nLevelPos ) const;

    /** Returns the start coordinate of the specified column/row in the window. */
    long                        GetColRowPos( SCCOLROW nColRowIndex ) const;
    /** Returns the entry position of header images. */
    long                        GetHeaderEntryPos() const;
    /** Calculates the coordinates the outline entry takes in the window.
        @return  false = no part of the group is visible (outside window or collapsed by parent group). */
    bool                        GetEntryPos(
                                    size_t nLevel, size_t nEntry,
                                    long& rnStartPos, long& rnEndPos, long& rnImagePos ) const;
    /** Calculates the absolute position of the image of the specified outline entry.
        @param nLevel  The level of the entry.
        @param nEntry  The entry index or SC_OL_HEADERENTRY for the header image.
        @return  false = image is not visible. */
    bool                        GetImagePos( size_t nLevel, size_t nEntry, Point& rPos ) const;
    /** Returns true, if the button of the specified entry is visible in the window. */
    bool                        IsButtonVisible( size_t nLevel, size_t nEntry ) const;

    /** Returns true, if rPos is inside of a button or over the line of an expanded
        group. The outline entry data is stored in the passed variables. */
    bool                        ItemHit( const Point& rPos, size_t& rnLevel, size_t& rnEntry, bool& rbButton ) const;
    /** Returns true, if rPos is inside of a button.
        The button data is stored in the passed variables. */
    bool                        ButtonHit( const Point& rPos, size_t& rnLevel, size_t& rnEntry ) const;
    /** Returns true, if rPos is over the line of an expanded group.
        The outline entry data is stored in the passed variables. */
    bool                        LineHit( const Point& rPos, size_t& rnLevel, size_t& rnEntry ) const;

    /** Performs an action with the specified item.
        @param nLevel  The level of the entry.
        @param nEntry  The entry index or SC_OL_HEADERENTRY for the header entry. */
    void                        DoFunction( size_t nLevel, size_t nEntry ) const;
    /** Expands the specified entry (does nothing with header entries). */
    void                        DoExpand( size_t nLevel, size_t nEntry ) const;
    /** Collapses the specified entry (does nothing with header entries). */
    void                        DoCollapse( size_t nLevel, size_t nEntry ) const;

    /** Returns true, if the focused button is visible in the window. */
    bool                        IsFocusButtonVisible() const;

    /** Calculates index of next/previous focus button in the current level (no paint).
        @param bFindVisible  true = repeats until a visible button has been found.
        @return  true = focus wrapped from end to start or vice versa. */
    bool                        ImplMoveFocusByEntry( bool bForward, bool bFindVisible );
    /** Calculates position of focus button in next/previous level (no paint).
        @return  true = focus wrapped from end to start or vice versa. */
    bool                        ImplMoveFocusByLevel( bool bForward );
    /** Calculates position of focus button in tab order.
        @param bFindVisible  true = repeats until a visible button has been found.
        @return  true = focus wrapped from end to start or vice versa. */
    bool                        ImplMoveFocusByTabOrder( bool bForward, bool bFindVisible );

    /** If the focused entry is invisible, tries to move to visible position. */
    void                        ImplMoveFocusToVisible( bool bForward );

    /** Focuses next/previous button in the current level. */
    void                        MoveFocusByEntry( bool bForward );
    /** Focuses button in next/previous level. */
    void                        MoveFocusByLevel( bool bForward );
    /** Focuses next/previous button in tab order. */
    void                        MoveFocusByTabOrder( bool bForward );

    /** Starts mouse tracking after click on a button. */
    void                        StartMouseTracking( size_t nLevel, size_t nEntry );
    /** Returns whether mouse tracking mode is active. */
    inline bool                 IsMouseTracking() const { return mbMTActive; }
    /** Ends mouse tracking. */
    void                        EndMouseTracking();

    /** Sets a clip region for the window area without header. */
    void                        SetEntryAreaClipRegion();
    /** Converts coordinates to real window points and draws the line. */
    void                        DrawLineRel(
                                    long nLevelStart, long nEntryStart,
                                    long nLevelEnd, long nEntryEnd );
    /** Converts coordinates to real window points and draws the rectangle. */
    void                        DrawRectRel(
                                    long nLevelStart, long nEntryStart,
                                    long nLevelEnd, long nEntryEnd );
    /** Draws the specified image unpressed. */
    void                        DrawImageRel( long nLevelPos, long nEntryPos, sal_uInt16 nId );
    /** Draws a pressed or unpressed border. */
    void                        DrawBorderRel( size_t nLevel, size_t nEntry, bool bPressed );

    /** Draws the focus rectangle into the focused button. */
    void                        ShowFocus();
    /** Erases the focus rectangle from the focused button. */
    void                        HideFocus();

    /** Scrolls the specified range of the window in entry-relative direction. */
    void                        ScrollRel( long nEntryDiff, long nEntryStart, long nEntryEnd );

protected:
    virtual void                Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect ) override;

    virtual void                Resize() override;
    virtual void                GetFocus() override;
    virtual void                LoseFocus() override;

    virtual void                MouseMove( const MouseEvent& rMEvt ) override;
    virtual void                MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void                MouseButtonDown( const MouseEvent& rMEvt ) override;

    virtual void                KeyInput( const KeyEvent& rKEvt ) override;

public:
    virtual void                DataChanged( const DataChangedEvent& rDCEvt ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
