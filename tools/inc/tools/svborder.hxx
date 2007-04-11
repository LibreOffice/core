/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svborder.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:18:39 $
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

#ifndef _SVBORDER_HXX
#define _SVBORDER_HXX

#ifndef INCLUDED_TOOLSDLLAPI_H
#include "tools/toolsdllapi.h"
#endif

#ifndef _GEN_HXX
#include <tools/gen.hxx>
#endif

class TOOLS_DLLPUBLIC SvBorder
{
    long nTop, nRight, nBottom, nLeft;
public:
    SvBorder()
    { nTop = nRight = nBottom = nLeft = 0; }
    SvBorder( const Size & rSz )
    { nTop = nBottom = rSz.Height(); nRight = nLeft = rSz.Width(); }
    SvBorder( const Rectangle & rOuter, const Rectangle & rInner );
    SvBorder( long nLeftP, long nTopP, long nRightP, long nBottomP )
    { nLeft = nLeftP; nTop = nTopP; nRight = nRightP; nBottom = nBottomP; }
    BOOL    operator == ( const SvBorder & rObj ) const
            {
                return nTop == rObj.nTop && nRight == rObj.nRight &&
                       nBottom == rObj.nBottom && nLeft == rObj.nLeft;
            }
    BOOL    operator != ( const SvBorder & rObj ) const
            { return !(*this == rObj); }
    SvBorder & operator += ( const SvBorder & rBorder )
            {
                Left()   += rBorder.Left();
                Top()    += rBorder.Top();
                Right()  += rBorder.Right();
                Bottom() += rBorder.Bottom();
                return *this;
            }
    SvBorder & operator -= ( const SvBorder & rBorder )
            {
                Left()   -= rBorder.Left();
                Top()    -= rBorder.Top();
                Right()  -= rBorder.Right();
                Bottom() -= rBorder.Bottom();
                return *this;
            }
    BOOL    IsInside( const SvBorder & rInside )
            {
                return nTop >= rInside.nTop && nRight >= rInside.nRight &&
                       nBottom >= rInside.nBottom && nLeft >= rInside.nLeft;
            }
    long &  Top()    { return nTop; }
    long &  Right()  { return nRight; }
    long &  Bottom() { return nBottom; }
    long &  Left()   { return nLeft; }
    long    Top()    const { return nTop; }
    long    Right()  const { return nRight; }
    long    Bottom() const { return nBottom; }
    long    Left()   const { return nLeft; }
};

TOOLS_DLLPUBLIC Rectangle & operator += ( Rectangle & rRect, const SvBorder & rBorder );
TOOLS_DLLPUBLIC Rectangle & operator -= ( Rectangle & rRect, const SvBorder & rBorder );

//=========================================================================

#endif

