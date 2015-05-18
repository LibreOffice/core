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

#ifndef INCLUDED_VCL_CURSOR_HXX
#define INCLUDED_VCL_CURSOR_HXX

#include <tools/gen.hxx>
#include <tools/link.hxx>
#include <vcl/dllapi.h>
#include <vcl/vclptr.hxx>

class Timer;
struct ImplCursorData;
namespace vcl { class Window; }

// Cursor styles
#define CURSOR_SHADOW                   ((sal_uInt16)0x0001)

enum class CursorDirection
{
    NONE, LTR, RTL
};

namespace vcl
{

class VCL_DLLPUBLIC Cursor
{
private:
    ImplCursorData* mpData;
    VclPtr<vcl::Window> mpWindow;           // only for shadow cursor
    long            mnSlant;
    Size            maSize;
    Point           maPos;
    short           mnOrientation;
    sal_uInt16      mnStyle;
    bool            mbVisible;
    CursorDirection mnDirection;

public:
    SAL_DLLPRIVATE void         ImplDraw();
    SAL_DLLPRIVATE void         ImplRestore();
    DECL_DLLPRIVATE_LINK_TYPED( ImplTimerHdl, Timer*, void );
    SAL_DLLPRIVATE void         ImplShow( bool bDrawDirect = true );
    SAL_DLLPRIVATE void         ImplHide( bool bStopTimer );
    SAL_DLLPRIVATE void         ImplResume( bool bRestore = false );
    SAL_DLLPRIVATE bool         ImplSuspend();
    SAL_DLLPRIVATE void         ImplNew();

public:
                    Cursor();
                    Cursor( const Cursor& rCursor );
                    ~Cursor();

    void            SetStyle( sal_uInt16 nStyle );
    sal_uInt16      GetStyle() const { return mnStyle; }

    void            Show();
    void            Hide();
    bool            IsVisible() const { return mbVisible; }

    void            SetWindow( vcl::Window* pWindow );
    vcl::Window*         GetWindow() const { return mpWindow; }

    void            SetPos( const Point& rNewPos );
    const Point&    GetPos() const { return maPos; }

    void            SetSize( const Size& rNewSize );
    const Size&     GetSize() const { return maSize; }
    void            SetWidth( long nNewWidth );
    long            GetWidth() const { return maSize.Width(); }
    long            GetHeight() const { return maSize.Height(); }

    long            GetSlant() const { return mnSlant; }

    void            SetOrientation( short nOrientation = 0 );
    short           GetOrientation() const { return mnOrientation; }

    void            SetDirection( CursorDirection nDirection = CursorDirection::NONE );
    CursorDirection GetDirection() const { return mnDirection; }

    Cursor&         operator=( const Cursor& rCursor );
    bool            operator==( const Cursor& rCursor ) const;
    bool            operator!=( const Cursor& rCursor ) const
                        { return !(Cursor::operator==( rCursor )); }

private:
    void ImplDoShow( bool bDrawDirect, bool bRestore );
    bool ImplDoHide( bool bStop );
};

}

#endif // INCLUDED_VCL_CURSOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
