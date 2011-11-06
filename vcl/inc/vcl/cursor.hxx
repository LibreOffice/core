/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    sal_Bool            mbVisible;          // Ist Cursor sichtbar
    unsigned char   mnDirection;        // indicates direction

public:
    SAL_DLLPRIVATE void         ImplDraw();
    SAL_DLLPRIVATE void         ImplRestore();
    DECL_DLLPRIVATE_LINK(       ImplTimerHdl, AutoTimer* );
    SAL_DLLPRIVATE void         ImplShow( bool bDrawDirect = true, bool bRestore = false );
    SAL_DLLPRIVATE bool         ImplHide( bool bStopTimer );
    SAL_DLLPRIVATE void         ImplNew();

public:
                    Cursor();
                    Cursor( const Cursor& rCursor );
                    ~Cursor();

    void            SetStyle( sal_uInt16 nStyle );
    sal_uInt16          GetStyle() const { return mnStyle; }

    void            Show();
    void            Hide();
    sal_Bool            IsVisible() const { return mbVisible; }

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
    sal_Bool            operator==( const Cursor& rCursor ) const;
    sal_Bool            operator!=( const Cursor& rCursor ) const
                        { return !(Cursor::operator==( rCursor )); }
};

#endif  // _SV_CURSOR_HXX
