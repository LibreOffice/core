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
