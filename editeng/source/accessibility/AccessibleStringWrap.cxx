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
#include "precompiled_editeng.hxx"

#include <algorithm>
#include <tools/debug.hxx>
#include <vcl/outdev.hxx>

#include <editeng/svxfont.hxx>
#include <editeng/AccessibleStringWrap.hxx>

//------------------------------------------------------------------------
//
// AccessibleStringWrap implementation
//
//------------------------------------------------------------------------

AccessibleStringWrap::AccessibleStringWrap( OutputDevice& rDev, SvxFont& rFont, const String& rText ) :
    mrDev( rDev ),
    mrFont( rFont ),
    maText( rText )
{
}

sal_Bool AccessibleStringWrap::GetCharacterBounds( sal_Int32 nIndex, Rectangle& rRect )
{
    DBG_ASSERT(nIndex >= 0 && nIndex <= USHRT_MAX,
               "SvxAccessibleStringWrap::GetCharacterBounds: index value overflow");

    mrFont.SetPhysFont( &mrDev );

    // #108900# Handle virtual position one-past-the end of the string
    if( nIndex >= maText.Len() )
    {
        // create a caret bounding rect that has the height of the
        // current font and is one pixel wide.
        rRect.Left() = mrDev.GetTextWidth(maText);
        rRect.Top() = 0;
        rRect.SetSize( Size(mrDev.GetTextHeight(), 1) );
    }
    else
    {
        sal_Int32 aXArray[2];
        mrDev.GetCaretPositions( maText, aXArray, static_cast< sal_uInt16 >(nIndex), 1 );
        rRect.Left() = 0;
        rRect.Top() = 0;
        rRect.SetSize( Size(mrDev.GetTextHeight(), labs(aXArray[0] - aXArray[1])) );
        rRect.Move( ::std::min(aXArray[0], aXArray[1]), 0 );
    }

    if( mrFont.IsVertical() )
    {
        // #101701# Rotate to vertical
        rRect = Rectangle( Point(-rRect.Top(), rRect.Left()),
                           Point(-rRect.Bottom(), rRect.Right()));
    }

    return sal_True;
}

sal_Int32 AccessibleStringWrap::GetIndexAtPoint( const Point& rPoint )
{
    // search for character bounding box containing given point
    Rectangle aRect;
    sal_Int32 i, nLen = maText.Len();
    for( i=0; i<nLen; ++i )
    {
        GetCharacterBounds(i, aRect);
        if( aRect.IsInside(rPoint) )
            return i;
    }

    return -1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
