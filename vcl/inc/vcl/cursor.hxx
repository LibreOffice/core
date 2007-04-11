/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cursor.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 17:50:22 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SV_CURSOR_HXX
#define _SV_CURSOR_HXX

#ifndef _GEN_HXX
#include <tools/gen.hxx>
#endif

#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

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

//#if 0 // _SOLAR__PRIVATE
public:
    SAL_DLLPRIVATE void         ImplDraw();
    SAL_DLLPRIVATE void         ImplRestore();
    DECL_DLLPRIVATE_LINK(       ImplTimerHdl, AutoTimer* );
    SAL_DLLPRIVATE void         ImplShow( BOOL bDrawDirect = TRUE );
    SAL_DLLPRIVATE void         ImplHide();
    SAL_DLLPRIVATE void         ImplNew();
//#endif

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
