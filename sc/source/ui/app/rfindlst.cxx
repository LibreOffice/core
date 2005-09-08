/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rfindlst.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 20:23:39 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include "rfindlst.hxx"

// STATIC DATA -----------------------------------------------------------

#define SC_RANGECOLORS  8

static ColorData aColNames[SC_RANGECOLORS] =
    { COL_LIGHTBLUE, COL_LIGHTRED, COL_LIGHTMAGENTA, COL_GREEN,
        COL_BLUE, COL_RED, COL_MAGENTA, COL_BROWN };

//==================================================================

ScRangeFindList::ScRangeFindList(const String& rName) :
    aDocName( rName ),
    bHidden( FALSE )
{
}

ScRangeFindList::~ScRangeFindList()
{
    void* pEntry = aEntries.First();
    while ( pEntry )
    {
        delete (ScRangeFindData*) aEntries.Remove( pEntry );
        pEntry = aEntries.Next();
    }
}

ColorData ScRangeFindList::GetColorName( USHORT nIndex )        // static
{
    return aColNames[nIndex % SC_RANGECOLORS];
}

//==================================================================

