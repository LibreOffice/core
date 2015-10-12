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

#ifndef INCLUDED_SVTOOLS_SCRWIN_HXX
#define INCLUDED_SVTOOLS_SCRWIN_HXX

#include <svtools/svtdllapi.h>
#include <vcl/scrbar.hxx>
#include <o3tl/typed_flags_set.hxx>
#include <vcl/vclptr.hxx>

class DataChangedEvent;


// - ScrollableWindow-Type -


enum class ScrollableWindowFlags
{
    THUMBDRAGGING = 1,
    VCENTER       = 2,
    HCENTER       = 4,
    DEFAULT       = THUMBDRAGGING | VCENTER | HCENTER,
};
namespace o3tl
{
    template<> struct typed_flags<ScrollableWindowFlags> : is_typed_flags<ScrollableWindowFlags, 0x07> {};
}


// - ScrollableWindow -


class SVT_DLLPUBLIC ScrollableWindow: public vcl::Window
{
private:
    Point           aPixOffset;         // offset to virtual window (pixel)
    Size            aTotPixSz;          // total size of virtual window (pixel)
    long            nLinePixH;          // size of a line/column (pixel)
    long            nColumnPixW;

    VclPtr<ScrollBar>    aVScroll;           // the scrollbars
    VclPtr<ScrollBar>    aHScroll;
    VclPtr<ScrollBarBox> aCornerWin;         // window in the bottom right corner
    bool            bScrolling:1,       // user controlled scrolling
                    bHandleDragging:1,  // scroll window while dragging
                    bHCenter:1,
                    bVCenter:1;

    SVT_DLLPRIVATE void         ImpInitialize( ScrollableWindowFlags nFlags );
    DECL_DLLPRIVATE_LINK_TYPED( ScrollHdl, ScrollBar *, void );
    DECL_DLLPRIVATE_LINK_TYPED( EndScrollHdl, ScrollBar *, void );

public:
                    ScrollableWindow( vcl::Window* pParent, WinBits nBits = 0,
                                      ScrollableWindowFlags = ScrollableWindowFlags::DEFAULT );
    virtual         ~ScrollableWindow();
    virtual void    dispose() override;

    virtual void    Resize() override;
    virtual void    Command( const CommandEvent& rCEvt ) override;
    virtual void    DataChanged( const DataChangedEvent& rDEvt ) override;

    using OutputDevice::SetMapMode;
    virtual void    SetMapMode( const MapMode& rNewMapMode ) override;
    MapMode GetMapMode() const;

    void            SetTotalSize( const Size& rNewSize );
    Size            GetTotalSize() { return PixelToLogic( aTotPixSz ); }

    using Window::Scroll;
    virtual void    Scroll( long nDeltaX, long nDeltaY, ScrollFlags nFlags = ScrollFlags::NONE ) override;

private:
    SVT_DLLPRIVATE Size         GetOutputSizePixel() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
