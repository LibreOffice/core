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

#include <vcl/window.hxx>
#include <vcl/timer.hxx>
#include <types.hxx>

#define HDR_SIZE_OPTIMUM    0xFFFF

// Size of the sliders
#define HDR_SLIDERSIZE      2

class ScTabView;
class SelectionEngine;

class ScHeaderControl : public vcl::Window
{
private:
    SelectionEngine*    pSelEngine;
    Timer               aShowHelpTimer;
    vcl::Font           aNormFont;
    vcl::Font           aBoldFont;
    vcl::Font           aAutoFilterFont;
    bool                bBoldSet;
    bool                bAutoFilterSet;

    bool            bVertical;              // Vertical = Row header

    tools::Long            nWidth;
    tools::Long            nSmallWidth;
    tools::Long            nBigWidth;

    SCCOLROW        nSize;

    SCCOLROW        nMarkStart;
    SCCOLROW        nMarkEnd;
    bool            bMarkRange;

    bool            bDragging;              // Resizing
    SCCOLROW        nDragNo;
    tools::Long            nDragStart;
    tools::Long            nDragPos;
    void*           nTipVisible;
    bool            bDragMoved;

    bool            bIgnoreMove;

    bool            bInRefMode;

    tools::Long            GetScrPos( SCCOLROW nEntryNo ) const;
    SCCOLROW        GetMousePos(const Point& rPos, bool& rBorder) const;
    bool            IsSelectionAllowed(SCCOLROW nPos) const;
    void            ShowDragHelp();
    void            HideDragHelp();

    void            DoPaint( SCCOLROW nStart, SCCOLROW nEnd );

    DECL_LINK(ShowDragHelpHdl, Timer*, void);

protected:
    ScTabView*      pTabView;

                    // Window overrides

    virtual void    Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;

    virtual void    MouseMove( const MouseEvent& rMEvt ) override;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void    Tracking( const TrackingEvent& rTEvt ) override;

    virtual void    RequestHelp( const HelpEvent& rHEvt ) override;

                    // new methods

    virtual SCCOLROW    GetPos() const = 0;                         // current position (Scrolling)
    virtual sal_uInt16  GetEntrySize( SCCOLROW nEntryNo ) const = 0;      // width / height (Pixel)
    virtual OUString  GetEntryText( SCCOLROW nEntryNo ) const = 0;

    virtual SCCOLROW GetHiddenCount( SCCOLROW nEntryNo ) const;
    virtual bool     IsLayoutRTL() const;
    virtual bool     IsMirrored() const;

    virtual void    SetEntrySize( SCCOLROW nPos, sal_uInt16 nNewWidth ) = 0;
    virtual void    HideEntries( SCCOLROW nStart, SCCOLROW nEnd ) = 0;

    virtual void    SetMarking( bool bSet );
    virtual void    SelectWindow();
    virtual bool    IsDisabled() const;
    virtual bool    ResizeAllowed() const;
    virtual OUString  GetDragHelp( tools::Long nVal );

    virtual void    DrawInvert( tools::Long nDragPos );
    virtual void    Command( const CommandEvent& rCEvt ) override;
    virtual void    dispose() override;

public:
            ScHeaderControl( vcl::Window* pParent, SelectionEngine* pSelectionEngine,
                             SCCOLROW nNewSize, bool bNewVertical, ScTabView* pTab );
            virtual ~ScHeaderControl() override;

    void    SetIgnoreMove(bool bSet)            { bIgnoreMove = bSet; }

    void    StopMarking();

    void    SetMark( bool bNewSet, SCCOLROW nNewStart, SCCOLROW nNewEnd );

    tools::Long    GetWidth() const                    { return nWidth; }
    tools::Long    GetSmallWidth() const               { return nSmallWidth; }
    tools::Long    GetBigWidth() const                 { return nBigWidth; }
    void    SetWidth( tools::Long nNew );
    void    GetMarkRange(SCCOLROW& rStart, SCCOLROW& rEnd) const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
