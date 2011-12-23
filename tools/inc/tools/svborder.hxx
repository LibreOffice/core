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

#ifndef _SVBORDER_HXX
#define _SVBORDER_HXX

#include "tools/toolsdllapi.h"
#include <tools/gen.hxx>

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
    sal_Bool    operator == ( const SvBorder & rObj ) const
            {
                return nTop == rObj.nTop && nRight == rObj.nRight &&
                       nBottom == rObj.nBottom && nLeft == rObj.nLeft;
            }
    sal_Bool    operator != ( const SvBorder & rObj ) const
            { return !(*this == rObj); }
    SvBorder & operator = ( const SvBorder & rBorder )
            {
                Left()   = rBorder.Left();
                Top()    = rBorder.Top();
                Right()  = rBorder.Right();
                Bottom() = rBorder.Bottom();
                return *this;
            }
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
    sal_Bool    IsInside( const SvBorder & rInside )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
