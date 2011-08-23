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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#include "unoedhlp.hxx"
#include "editeng.hxx"
namespace binfilter {

//------------------------------------------------------------------------

sal_Bool SvxEditSourceHelper::GetAttributeRun( USHORT& nStartIndex, USHORT& nEndIndex, const EditEngine& rEE, USHORT nPara, USHORT nIndex )
{
    EECharAttribArray aCharAttribs;
    
    rEE.GetCharAttribs( nPara, aCharAttribs );
    
    // find closest index in front of nIndex
    USHORT nAttr, nCurrIndex;
    sal_Int32 nClosestStartIndex;
    for( nAttr=0, nClosestStartIndex=0; nAttr<aCharAttribs.Count(); ++nAttr )
    {
        nCurrIndex = aCharAttribs[nAttr].nStart;
        
        if( nCurrIndex > nIndex )
            break; // aCharAttribs array is sorted in increasing order for nStart values

        if( nCurrIndex > nClosestStartIndex )
        {
            nClosestStartIndex = nCurrIndex;
        }
    }

    // find closest index behind of nIndex
    sal_Int32 nClosestEndIndex;
    for( nAttr=0, nClosestEndIndex=rEE.GetTextLen(nPara); nAttr<aCharAttribs.Count(); ++nAttr )
    {
        nCurrIndex = aCharAttribs[nAttr].nEnd;

        if( nCurrIndex > nIndex &&
            nCurrIndex < nClosestEndIndex )
        {
            nClosestEndIndex = nCurrIndex;
        }
    }

    nStartIndex = static_cast<USHORT>( nClosestStartIndex );
    nEndIndex = static_cast<USHORT>( nClosestEndIndex );

    return sal_True;
}

Point SvxEditSourceHelper::EEToUserSpace( const Point& rPoint, const Size& rEESize, bool bIsVertical )
{
    return bIsVertical ? Point( -rPoint.Y() + rEESize.Height(), rPoint.X() ) : rPoint;
}

Point SvxEditSourceHelper::UserSpaceToEE( const Point& rPoint, const Size& rEESize, bool bIsVertical )
{
    return bIsVertical ? Point( rPoint.Y(), -rPoint.X() + rEESize.Height() ) : rPoint;
}

Rectangle SvxEditSourceHelper::EEToUserSpace( const Rectangle& rRect, const Size& rEESize, bool bIsVertical )
{
    // #106775# Don't touch rect if not vertical
    return bIsVertical ? Rectangle( EEToUserSpace(rRect.BottomLeft(), rEESize, bIsVertical),
                                    EEToUserSpace(rRect.TopRight(), rEESize, bIsVertical) ) : rRect;
}


}
