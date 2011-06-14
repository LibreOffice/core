/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <vcl/window.hxx>
#include <vector>

#define MARKER_NOMARKER 0xFFFF

class SbModule;
struct Breakpoint;
class ImageList;

class BreakpointWindow : public Window
{
using Window::Scroll;

public:
    BreakpointWindow( Window *pParent );

    void        Reset();

    void        SetModule( SbModule *pMod );
    void        SetBPsInModule();

    void        InsertBreakpoint( sal_uInt32 nLine );
    void        ToggleBreakpoint( sal_uInt32 nLine );
    void        AdjustBreakpoints( sal_uInt32 nLine, bool bInserted );

    void        LoadBreakpoints( String aFilename );
    void        SaveBreakpoints( String aFilename );

private:
    ::std::vector< Breakpoint* > BreakpointList;
    long                nCurYOffset;
    sal_uInt32          nMarkerPos;
    SbModule*           pModule;
    bool                bErrorMarker;
    static ImageList*   pImages;

protected:
    virtual void    Paint( const Rectangle& );
    Breakpoint*     FindBreakpoint( const Point& rMousePos );
    Breakpoint*     FindBreakpoint( sal_uInt32 nLine );
    void            ShowMarker( bool bShow );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );

public:
    void            SetMarkerPos( sal_uInt32 nLine, bool bErrorMarker = false );
    virtual void    Scroll( long nHorzScroll, long nVertScroll, sal_uInt16 nFlags = 0 );
    long&           GetCurYOffset()         { return nCurYOffset; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
