/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AccessibleStringWrap.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 04:03:11 $
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
#include "precompiled_svx.hxx"

#include <algorithm>

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif

#include "svxfont.hxx"
#include "AccessibleStringWrap.hxx"


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
        mrDev.GetCaretPositions( maText, aXArray, static_cast< USHORT >(nIndex), 1 );
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
