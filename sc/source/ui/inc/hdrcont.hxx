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

#ifndef INCLUDED_SC_SOURCE_UI_INC_HDRCONT_HXX
#define INCLUDED_SC_SOURCE_UI_INC_HDRCONT_HXX

#include <vcl/window.hxx>
#include <vcl/seleng.hxx>
#include "address.hxx"

#define HDR_SIZE_OPTIMUM    0xFFFF

// Size of the sliders
#define HDR_SLIDERSIZE      2

class ScHeaderControl : public vcl::Window
{
private:
    SelectionEngine*    pSelEngine;
    vcl::Font           aNormFont;
    vcl::Font           aBoldFont;
    bool                bBoldSet;

    bool            bVertical;              // Vertical = Row header

    long            nWidth;
    long            nSmallWidth;
    long            nBigWidth;

    SCCOLROW        nSize;

    SCCOLROW        nMarkStart;
    SCCOLROW        nMarkEnd;
    bool            bMarkRange;

    bool            bDragging;              // Resizing
    SCCOLROW        nDragNo;
    long            nDragStart;
    long            nDragPos;
    bool            bDragMoved;

    bool            bIgnoreMove;

    long            GetScrPos( SCCOLROW nEntryNo ) const;
    SCCOLROW        GetMousePos( const MouseEvent& rMEvt, bool& rBorder ) const;
    bool            IsSelectionAllowed(SCCOLROW nPos) const;
    void            ShowDragHelp();

    void            DoPaint( SCCOLROW nStart, SCCOLROW nEnd );

    void            DrawShadedRect( long nStart, long nEnd, const Color& rBaseColor );

protected:
                    // Window overrides

    virtual void    Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect ) override;

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
    virtual OUString  GetDragHelp( long nVal );

    virtual void    DrawInvert( long nDragPos );
    virtual void    Command( const CommandEvent& rCEvt ) override;

public:
            ScHeaderControl( vcl::Window* pParent, SelectionEngine* pSelectionEngine,
                                SCCOLROW nNewSize, bool bNewVertical );
            virtual ~ScHeaderControl();

    void    SetIgnoreMove(bool bSet)            { bIgnoreMove = bSet; }

    void    StopMarking();

    void    SetMark( bool bNewSet, SCCOLROW nNewStart, SCCOLROW nNewEnd );

    long    GetWidth() const                    { return nWidth; }
    long    GetSmallWidth() const               { return nSmallWidth; }
    long    GetBigWidth() const                 { return nBigWidth; }
    void    SetWidth( long nNew );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
