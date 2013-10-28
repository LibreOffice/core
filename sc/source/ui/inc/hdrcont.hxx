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

#ifndef SC_HDRCONT_HXX
#define SC_HDRCONT_HXX

#include <vcl/window.hxx>
#include <vcl/seleng.hxx>
#include "address.hxx"

#define HDR_SIZE_OPTIMUM    0xFFFF

// Size of the sliders
#define HDR_SLIDERSIZE      2

class ScHeaderControl : public Window
{
private:
    SelectionEngine*    pSelEngine;
    Font                aNormFont;
    Font                aBoldFont;
    bool                bBoldSet;

    bool            bVertical;              // Vertical = Row header

    long            nWidth;
    long            nSmallWidth;
    long            nBigWidth;

    SCCOLROW        nSize;

    SCCOLROW        nMarkStart;
    SCCOLROW        nMarkEnd;
    bool            bMarkRange;

    bool            bDragging;              // Change size
    SCCOLROW        nDragNo;
    long            nDragStart;
    long            nDragPos;
    bool            bDragMoved;

    bool            bIgnoreMove;

    long            GetScrPos( SCCOLROW nEntryNo );
    SCCOLROW        GetMousePos( const MouseEvent& rMEvt, bool& rBorder );
    bool            IsSelectionAllowed(SCCOLROW nPos) const;
    void            ShowDragHelp();

    void            DoPaint( SCCOLROW nStart, SCCOLROW nEnd );

    void            DrawShadedRect( long nStart, long nEnd, const Color& rBaseColor );

protected:
                    // Overloaded by Window

    virtual void    Paint( const Rectangle& rRect );

    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    Tracking( const TrackingEvent& rTEvt );

    virtual void    RequestHelp( const HelpEvent& rHEvt );

                    // new methods

    virtual SCCOLROW    GetPos() = 0;                               // current position (Scrolling)
    virtual sal_uInt16  GetEntrySize( SCCOLROW nEntryNo ) = 0;      // width / height (Pixel)
    virtual OUString  GetEntryText( SCCOLROW nEntryNo ) = 0;

    virtual SCCOLROW GetHiddenCount( SCCOLROW nEntryNo );
    virtual bool     IsLayoutRTL();
    virtual bool     IsMirrored();

    virtual void    SetEntrySize( SCCOLROW nPos, sal_uInt16 nNewWidth ) = 0;
    virtual void    HideEntries( SCCOLROW nStart, SCCOLROW nEnd ) = 0;

    virtual void    SetMarking( bool bSet );
    virtual void    SelectWindow();
    virtual bool    IsDisabled();
    virtual bool    ResizeAllowed();
    virtual OUString  GetDragHelp( long nVal );

    virtual void    DrawInvert( long nDragPos );
    virtual void    Command( const CommandEvent& rCEvt );

public:
            ScHeaderControl( Window* pParent, SelectionEngine* pSelectionEngine,
                                SCCOLROW nNewSize, bool bNewVertical );
            ~ScHeaderControl();

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
