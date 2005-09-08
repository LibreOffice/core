/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: drawattr.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 22:55:25 $
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

#include "drawattr.hxx"
#include "global.hxx"

//------------------------------------------------------------------------

String __EXPORT SvxDrawToolItem::GetValueText() const
{
    return GetValueText(GetValue());
}

//------------------------------------------------------------------------

String __EXPORT SvxDrawToolItem::GetValueText( USHORT nVal ) const
{
    const sal_Char* p;

    switch (nVal)
    {
        case 0  : p = "SVX_SNAP_DRAW_SELECT"    ; break;
        //
        case 1  : p = "SVX_SNAP_DRAW_LINE"      ; break;
        case 2  : p = "SVX_SNAP_DRAW_RECT"      ; break;
        case 3  : p = "SVX_SNAP_DRAW_ELLIPSE"   ; break;
        case 4  : p = "SVX_SNAP_DRAW_POLYGON"   ; break;
        case 5  : p = "SVX_SNAP_DRAW_ARC"       ; break;
        case 6  : p = "SVX_SNAP_DRAW_PIE"       ; break;
        case 7  : p = "SVX_SNAP_DRAW_CIRCLECUT" ; break;
        case 8  : p = "SVX_SNAP_DRAW_TEXT"      ; break;
        default : return EMPTY_STRING;
    }
    return String::CreateFromAscii( p );
}

//------------------------------------------------------------------------

SfxPoolItem* __EXPORT SvxDrawToolItem::Clone( SfxItemPool * ) const
{
    return new SvxDrawToolItem(*this);
}

//------------------------------------------------------------------------

SfxPoolItem* __EXPORT SvxDrawToolItem::Create( SvStream& rStream, USHORT nVer ) const
{
    USHORT nVal;
    rStream >> nVal;
    return new SvxDrawToolItem(nVal);
}



