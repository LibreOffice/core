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

#ifndef _MODELTOVIEWHELPER_HXX
#define _MODELTOVIEWHELPER_HXX

#include <sal/types.h>

#include <vector>

/** Some helpers for converting model strings to view strings.

    A paragraph string does not have its fields expanded, i.e., they are
    represented by a special character inside the string with an additional
    attribute assigned to it. For some tasks (e.g., SmartTags) it is required
    to expand the fields to get the string as it appears in the view. Two
    helper functions are provided to convert model positions to view positions
    and vice versa.
*/
namespace ModelToViewHelper
{
    /** For each field in the model string, there is an entry in the conversion
        map. The first value of the ConversionMapEntry points to the field
        position in the model string, the second value points to the associated
        position in the view string. The last entry in the conversion map
        denotes the lengths of the model resp. view string.
    */
    typedef std::pair< sal_uInt32 , sal_uInt32 > ConversionMapEntry;
    typedef std::vector< ConversionMapEntry > ConversionMap;

    /** This struct defines a position in the model string.

        The 'main' position is given by mnPos. If there's a field located at
        this position, mbIsField is set and mnSubPos denotes the position inside
        that field.
    */
    struct ModelPosition
    {
        sal_uInt32 mnPos;
        sal_uInt32 mnSubPos;
        bool mbIsField;

        ModelPosition() : mnPos(0), mnSubPos(0), mbIsField(false) {}
    };

    /** Converts a model position into a view position

        @param pMap
            pMap is the conversion map required for the calculation. If pMap is
            0, no conversion takes place, i.e., it is assumed that the model
            string is identical to the view string.

        @param nPos
            nPos denotes a position in the model string which should be
            converted. Note that converting model positions inside fields is
            not supported, therefore nPos is not of type ModelPosition.

        @return
            the position of nPos in the view string. In case the conversion
            could not be performed (e.g., because there is not ConversionMap or
            nPos is behind the last entry in the conversion map) nPos will
            be returned.
    */
    sal_uInt32 ConvertToViewPosition( const ConversionMap* pMap, sal_uInt32 nModelPos );

    /** Converts a view position into a model position

        @param pMap
            pMap is the conversion map required for the calculation. If pMap is
            0, no conversion takes place, i.e., it is assumed that the model
            string is identical to the view string.

        @param nPos
            nPos denotes a position in the view string which should be
            converted.

        @return
            the position of nPos in the model string. In case the conversion
            could not be performed (e.g., because there is not ConversionMap or
            nPos is behind the last entry in the conversion map) a model
            model position with mnPos = nPos and mnIsField = false will be
            returned.
    */
    ModelPosition ConvertToModelPosition( const ConversionMap* pMap, sal_uInt32 nViewPos );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
