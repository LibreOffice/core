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

#ifndef _SCRWIN_HXX
#define _SCRWIN_HXX

#include <svtools/svtdllapi.h>

#include <vcl/scrbar.hxx>

class DataChangedEvent;


// - ScrollableWindow-Type -


typedef sal_uInt16 ScrollableWindowFlags;

#define SCRWIN_THUMBDRAGGING 1
#define SCRWIN_VCENTER       2
#define SCRWIN_HCENTER       4
#define SCRWIN_DEFAULT       (SCRWIN_THUMBDRAGGING | SCRWIN_VCENTER | SCRWIN_HCENTER)


// - ScrollableWindow -


class SVT_DLLPUBLIC ScrollableWindow: public Window
{
private:
    Point           aPixOffset;         // offset to virtual window (pixel)
    Size            aTotPixSz;          // total size of virtual window (pixel)
    long            nLinePixH;          // size of a line/column (pixel)
    long            nColumnPixW;

    ScrollBar       aVScroll;           // the scrollbars
    ScrollBar       aHScroll;
    ScrollBarBox    aCornerWin;         // window in the bottom right corner
    bool            bScrolling:1,       // user controlled scrolling
                    bHandleDragging:1,  // scroll window while dragging
                    bHCenter:1,
                    bVCenter:1;

    SVT_DLLPRIVATE void         ImpInitialize( ScrollableWindowFlags nFlags );
    DECL_DLLPRIVATE_LINK( ScrollHdl, ScrollBar * );
    DECL_DLLPRIVATE_LINK( EndScrollHdl, ScrollBar * );

public:
                    ScrollableWindow( Window* pParent, WinBits nBits = 0,
                                      ScrollableWindowFlags = SCRWIN_DEFAULT );

    virtual void    Resize();
    virtual void    Command( const CommandEvent& rCEvt );
    virtual void    DataChanged( const DataChangedEvent& rDEvt );

    virtual void    StartScroll();
    virtual void    EndScroll( long nDeltaX, long nDeltaY );

    using OutputDevice::SetMapMode;
    virtual void    SetMapMode( const MapMode& rNewMapMode );
    virtual MapMode GetMapMode() const;

    void            SetTotalSize( const Size& rNewSize );
    Size            GetTotalSize() { return PixelToLogic( aTotPixSz ); }

    using Window::Scroll;
    virtual void    Scroll( long nDeltaX, long nDeltaY, sal_uInt16 nFlags = 0 );

private:
    SVT_DLLPRIVATE Size         GetOutputSizePixel() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
