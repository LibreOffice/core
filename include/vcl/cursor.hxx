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

#ifndef _SV_CURSOR_HXX
#define _SV_CURSOR_HXX

#include <tools/gen.hxx>
#include <tools/link.hxx>
#include <tools/solar.h>
#include <vcl/dllapi.h>

class AutoTimer;
struct ImplCursorData;
class Window;

// -----------------
// - Cursor-Styles -
// -----------------

#define CURSOR_SHADOW                   ((sal_uInt16)0x0001)
#define CURSOR_DIRECTION_NONE           ((unsigned char)0x00)
#define CURSOR_DIRECTION_LTR            ((unsigned char)0x01)
#define CURSOR_DIRECTION_RTL            ((unsigned char)0x02)

// ----------
// - Cursor -
// ----------

class VCL_DLLPUBLIC Cursor
{
private:
    ImplCursorData* mpData;             // Interne Daten
    Window*         mpWindow;           // Window (only for shadow cursor)
    long            mnSlant;            // Schraegstellung
    long            mnOffsetY;          // Offset fuer Rotation
    Size            maSize;             // Groesse
    Point           maPos;              // Position
    short           mnOrientation;      // Rotation
    sal_uInt16          mnStyle;            // Style
    bool            mbVisible;          // Ist Cursor sichtbar
    unsigned char   mnDirection;        // indicates direction

public:
    SAL_DLLPRIVATE void         ImplDraw();
    SAL_DLLPRIVATE void         ImplRestore();
    DECL_DLLPRIVATE_LINK(       ImplTimerHdl, void* );
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
    sal_uInt16          GetStyle() const { return mnStyle; }

    void            Show();
    void            Hide();
    bool            IsVisible() const { return mbVisible; }

    void            SetWindow( Window* pWindow );
    Window*         GetWindow() const { return mpWindow; }

    void            SetPos( const Point& rNewPos );
    const Point&    GetPos() const { return maPos; }
    long            GetOffsetY() const { return mnOffsetY; }

    void            SetSize( const Size& rNewSize );
    const Size&     GetSize() const { return maSize; }
    void            SetWidth( long nNewWidth );
    long            GetWidth() const { return maSize.Width(); }
    long            GetHeight() const { return maSize.Height(); }

    long            GetSlant() const { return mnSlant; }

    void            SetOrientation( short nOrientation = 0 );
    short           GetOrientation() const { return mnOrientation; }

    void            SetDirection( unsigned char nDirection = 0 );
    unsigned char   GetDirection() const { return mnDirection; }

    Cursor&         operator=( const Cursor& rCursor );
    bool            operator==( const Cursor& rCursor ) const;
    bool            operator!=( const Cursor& rCursor ) const
                        { return !(Cursor::operator==( rCursor )); }

private:
    void ImplDoShow( bool bDrawDirect, bool bRestore );
    bool ImplDoHide( bool bStop );
};

#endif  // _SV_CURSOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
