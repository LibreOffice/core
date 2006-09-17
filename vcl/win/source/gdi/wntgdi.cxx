/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: wntgdi.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 12:46:48 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

// -----------------------------------------------------------------------

extern "C"
{
BOOL WINAPI WIN_Rectangle( HDC hDC, int X1, int Y1, int X2, int Y2 )
{
    return Rectangle( hDC, X1, Y1, X2, Y2 );
}
}

// -----------------------------------------------------------------------

extern "C"
{
BOOL WINAPI WIN_Polygon( HDC hDC, CONST POINT * ppt, int ncnt )
{
    return Polygon( hDC, ppt, ncnt );
}
}

// -----------------------------------------------------------------------

extern "C"
{
BOOL WINAPI WIN_PolyPolygon( HDC hDC, CONST POINT * ppt, LPINT npcnt, int ncnt )
{
    return PolyPolygon( hDC, ppt, npcnt, ncnt );
}
}
