/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svborder.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 00:57:07 $
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
#include "precompiled_tools.hxx"

#include <svborder.hxx>
#include <osl/diagnose.h>

SvBorder::SvBorder( const Rectangle & rOuter, const Rectangle & rInner )
{
    Rectangle aOuter( rOuter );
    aOuter.Justify();
    Rectangle aInner( rInner );
    if( aInner.IsEmpty() )
        aInner = Rectangle( aOuter.Center(), aOuter.Center() );
    else
        aInner.Justify();

    OSL_ENSURE( aOuter.IsInside( aInner ),
                "SvBorder::SvBorder: FALSE == aOuter.IsInside( aInner )" );
    nTop    = aInner.Top()    - aOuter.Top();
    nRight  = aOuter.Right()  - aInner.Right();
    nBottom = aOuter.Bottom() - aInner.Bottom();
    nLeft   = aInner.Left()   - aOuter.Left();
}

Rectangle & operator += ( Rectangle & rRect, const SvBorder & rBorder )
{
    // wegen Empty-Rect, GetSize muss als erstes gerufen werden
    Size aS( rRect.GetSize() );
    aS.Width()  += rBorder.Left() + rBorder.Right();
    aS.Height() += rBorder.Top() + rBorder.Bottom();

    rRect.Left()   -= rBorder.Left();
    rRect.Top()    -= rBorder.Top();
    rRect.SetSize( aS );
    return rRect;
}

Rectangle & operator -= ( Rectangle & rRect, const SvBorder & rBorder )
{
    // wegen Empty-Rect, GetSize muss als erstes gerufen werden
    Size aS( rRect.GetSize() );
    aS.Width()  -= rBorder.Left() + rBorder.Right();
    aS.Height() -= rBorder.Top() + rBorder.Bottom();

    rRect.Left()   += rBorder.Left();
    rRect.Top()    += rBorder.Top();
    rRect.SetSize( aS );
    return rRect;
}

