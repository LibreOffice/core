/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: shdwcrsr.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 12:09:22 $
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
#ifndef _SHDWCRSR_HXX
#define _SHDWCRSR_HXX


#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif
#include  <limits.h>

class Window;

class SwShadowCursor
{
    Window* pWin;
    Color aCol;
    Point aOldPt;
    long nOldHeight;
    USHORT nOldMode;

    void DrawTri( const Point& rPt, long nHeight, BOOL bLeft );
    void DrawCrsr( const Point& rPt, long nHeight, USHORT nMode );

public:
    SwShadowCursor( Window& rWin, const Color& rCol )
        : pWin( &rWin ), aCol( rCol ), nOldHeight(0), nOldMode( USHRT_MAX ) {}
    ~SwShadowCursor();

    void SetPos( const Point& rPt, long nHeight, USHORT nMode );

    void Paint();

    const Point& GetPoint() const   { return aOldPt; }
    long GetHeight() const          { return nOldHeight; }
    USHORT GetMode() const          { return nOldMode; }

    Rectangle GetRect() const;
};



#endif

