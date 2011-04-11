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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"


#include "drawattr.hxx"
#include "global.hxx"

//------------------------------------------------------------------------

String SvxDrawToolItem::GetValueText() const
{
    return GetValueText(GetValue());
}

//------------------------------------------------------------------------

String SvxDrawToolItem::GetValueText( sal_uInt16 nVal ) const
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

SfxPoolItem* SvxDrawToolItem::Clone( SfxItemPool * ) const
{
    return new SvxDrawToolItem(*this);
}

//------------------------------------------------------------------------

SfxPoolItem* SvxDrawToolItem::Create( SvStream& rStream, sal_uInt16 nVer ) const
{
    sal_uInt16 nVal;
    rStream >> nVal;
    return new SvxDrawToolItem(nVal);
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
