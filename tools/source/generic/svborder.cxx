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
#include "precompiled_tools.hxx"

#include <tools/svborder.hxx>
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
                "SvBorder::SvBorder: sal_False == aOuter.IsInside( aInner )" );
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

