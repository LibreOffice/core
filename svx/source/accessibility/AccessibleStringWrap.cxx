/*************************************************************************
 *
 *  $RCSfile: AccessibleStringWrap.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-04 16:56:21 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
               "AccessibleStringWrap::GetCharacterBounds: index value overflow");

    mrFont.SetPhysFont( &mrDev );

    long aXArray[2];
    mrDev.GetCaretPositions( maText, aXArray, static_cast< USHORT >(nIndex), 1 );
    rRect.Left() = 0;
    rRect.Right() = 0;
    rRect.SetSize( Size(mrDev.GetTextHeight(), labs(aXArray[0] - aXArray[1])) );
    rRect.Move( ::std::min(aXArray[0], aXArray[1]), 0 );

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
