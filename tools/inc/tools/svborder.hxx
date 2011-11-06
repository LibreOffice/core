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

