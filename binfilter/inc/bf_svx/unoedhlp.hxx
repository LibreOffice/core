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

#ifndef _SVX_UNOEDHLP_HXX
#define _SVX_UNOEDHLP_HXX

#include <memory>
#include <tools/solar.h>
#include <tools/gen.hxx> 

namespace binfilter {

struct EENotify;
class EditEngine;

/** Helper class for common functionality in edit sources
 */
class SvxEditSourceHelper
{
public:
    /** Calculate attribute run for EditEngines

        Please note that the range returned is half-open: [nStartIndex,nEndIndex)

        @param nStartIndex 
        Herein, the start index of the range of similar attributes is returned

        @param nEndIndex 
        Herein, the end index (exclusive) of the range of similar attributes is returned

        @param rEE
        The EditEngine to query for attributes

        @param nPara
        The paragraph the following index value is to be interpreted in

        @param nIndex
        The character index from which the range of similar attributed characters is requested

        @return sal_True, if the range has been successfully determined
     */
    static sal_Bool GetAttributeRun( USHORT& nStartIndex, USHORT& nEndIndex, const EditEngine& rEE, USHORT nPara, USHORT nIndex );

    /** Convert point from edit engine to user coordinate space

        As the edit engine internally keeps vertical text unrotated,
        all internal edit engine methods return their stuff unrotated,
        too. This method rotates and shifts given point appropriately,
        if vertical writing is on.

        @param rPoint
        Point to transform

        @param rEESize
        Paper size of the edit engine

        @param  bIsVertical
        Whether output text is vertical or not

        @return the possibly transformed point
     */
    static Point EEToUserSpace( const Point& rPoint, const Size& rEESize, bool bIsVertical );

    /** Convert point from user to edit engine coordinate space

        As the edit engine internally keeps vertical text unrotated,
        all internal edit engine methods return their stuff unrotated,
        too. This method rotates and shifts given point appropriately,
        if vertical writing is on.

        @param rPoint
        Point to transform

        @param rEESize
        Paper size of the edit engine

        @param  bIsVertical
        Whether output text is vertical or not

        @return the possibly transformed point
     */
    static Point UserSpaceToEE( const Point& rPoint, const Size& rEESize, bool bIsVertical );

    /** Convert rect from edit engine to user coordinate space

        As the edit engine internally keeps vertical text unrotated,
        all internal edit engine methods return their stuff unrotated,
        too. This method rotates and shifts given rect appropriately,
        if vertical writing is on.

        @param rRect
        Rectangle to transform

        @param rEESize
        Paper size of the edit engine

        @param  bIsVertical
        Whether output text is vertical or not

        @return the possibly transformed rect
     */
    static Rectangle EEToUserSpace( const Rectangle& rRect, const Size& rEESize, bool bIsVertical );
};

}//end of namespace binfilter
#endif

