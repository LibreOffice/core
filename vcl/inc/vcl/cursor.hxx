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

#ifndef _SV_CURSOR_HXX
#define _SV_CURSOR_HXX

#include <tools/gen.hxx>
#include <tools/link.hxx>
#include <vcl/sv.h>
#include <vcl/dllapi.h>

class AutoTimer;
struct ImplCursorData;
class Window;

// -----------------
// - Cursor-Styles -
// -----------------

#define CURSOR_SHADOW                   ((USHORT)0x0001)
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
    USHORT          mnStyle;            // Style
    BOOL            mbVisible;          // Ist Cursor sichtbar
    unsigned char   mnDirection;        // indicates direction

public:
    SAL_DLLPRIVATE void         ImplDraw();
    SAL_DLLPRIVATE void         ImplRestore();
    DECL_DLLPRIVATE_LINK(       ImplTimerHdl, AutoTimer* );
    SAL_DLLPRIVATE void         ImplShow( bool bDrawDirect = true, bool bRestore = false );
    SAL_DLLPRIVATE bool         ImplHide();
    SAL_DLLPRIVATE void         ImplNew();

public:
                    Cursor();
                    Cursor( const Cursor& rCursor );
                    ~Cursor();

    void            SetStyle( USHORT nStyle );
    USHORT          GetStyle() const { return mnStyle; }

    void            Show();
    void            Hide();
    BOOL            IsVisible() const { return mbVisible; }

    void            SetWindow( Window* pWindow );
    Window*         GetWindow() const { return mpWindow; }

    void            SetPos( const Point& rNewPos );
    const Point&    GetPos() const { return maPos; }
    void            SetOffsetY( long mnOffsetY = 0 );
    long            GetOffsetY() const { return mnOffsetY; }

    void            SetSize( const Size& rNewSize );
    const Size&     GetSize() const { return maSize; }
    void            SetWidth( long nNewWidth );
    long            GetWidth() const { return maSize.Width(); }
    void            SetHeight( long nNewHeight );
    long            GetHeight() const { return maSize.Height(); }

    void            SetSlant( long nSlant = 0 );
    long            GetSlant() const { return mnSlant; }

    void            SetOrientation( short nOrientation = 0 );
    short           GetOrientation() const { return mnOrientation; }

    void            SetDirection( unsigned char nDirection = 0 );
    unsigned char   GetDirection() const { return mnDirection; }

    Cursor&         operator=( const Cursor& rCursor );
    BOOL            operator==( const Cursor& rCursor ) const;
    BOOL            operator!=( const Cursor& rCursor ) const
                        { return !(Cursor::operator==( rCursor )); }
};

#endif  // _SV_CURSOR_HXX
