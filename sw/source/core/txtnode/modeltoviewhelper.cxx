/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <modeltoviewhelper.hxx>

namespace ModelToViewHelper
{

/** Converts a model position into a view position
*/
sal_uInt32 ConvertToViewPosition( const ConversionMap* pMap, sal_uInt32 nModelPos )
{
    sal_uInt32 nRet = nModelPos;

    if ( !pMap )
        return nRet;

    // Search for entry behind nPos:
    ConversionMap::const_iterator aIter;
    for ( aIter = pMap->begin(); aIter != pMap->end(); ++aIter )
    {
        if ( (*aIter).first >= nModelPos )
        {
            const sal_uInt32 nPosModel  = (*aIter).first;
            const sal_uInt32 nPosExpand = (*aIter).second;

            const sal_uInt32 nDistToNextModel  = nPosModel - nModelPos;
            nRet = nPosExpand - nDistToNextModel;
            break;
        }
    }

    return nRet;
}


/** Converts a view position into a model position
*/
ModelPosition ConvertToModelPosition( const ConversionMap* pMap, sal_uInt32 nViewPos )
{
    ModelPosition aRet;
    aRet.mnPos = nViewPos;

    if ( !pMap )
        return aRet;

    // Search for entry behind nPos:
    ConversionMap::const_iterator aIter;
    for ( aIter = pMap->begin(); aIter != pMap->end(); ++aIter )
    {
        if ( (*aIter).second > nViewPos )
        {
            const sal_uInt32 nPosModel  = (*aIter).first;
            const sal_uInt32 nPosExpand = (*aIter).second;

            // If nViewPos is in front of first field, we are finished.
            if ( aIter == pMap->begin() )
                break;

            --aIter;

            // nPrevPosModel is the field position
            const sal_uInt32 nPrevPosModel  = (*aIter).first;
            const sal_uInt32 nPrevPosExpand = (*aIter).second;

            const sal_uInt32 nLengthModel  = nPosModel - nPrevPosModel;
            const sal_uInt32 nLengthExpand = nPosExpand - nPrevPosExpand;

            // For zero-expansion fields (e.g. post-it note fields), the expanded
            // string is smaller than the original string. nLengthExpand (the distance
            // between 2 fields in view-space) can be smaller than nLengthModel (the
            // distance in model-space). It can only be smaller by 1 (field marker character
            // exists in model-space but not in view-space).
            //
            // For a concrete example, consider: "Hello^[Comment Field] ^[Page Number Field]"
            // ^ are field markers. This expands to: "Hello 1". Note that the comment marker
            // was removed. The model-space distance between the two fields is 7 - 5 = 2. The view-space
            // distance between the two fields is 6 - 5 = 1.
            // (Technically, the comment field doesn't exist in the view. It still retains the index
            // of where it used to be, though)
            //
            // So the field length must be computed as "nLengthExpand + 1 - nLengthModel" instead
            // of the intuitive "nLengthExpand - nLengthModel + 1" to avoid underflow with
            // the unsigned ints.
            const sal_uInt32 nFieldLengthExpand = nLengthExpand + 1 - nLengthModel;
            const sal_uInt32 nFieldEndExpand = nPrevPosExpand + nFieldLengthExpand;

            // Check if nPos is outside of field:
            if ( nFieldEndExpand <= nViewPos )
            {
                // nPos is outside of field:
                const sal_uInt32 nDistToField = nViewPos - nFieldEndExpand + 1;
                aRet.mnPos  = nPrevPosModel + nDistToField;
            }
            else
            {
                // nViewPos is inside a field:
                aRet.mnPos = nPrevPosModel;
                aRet.mnSubPos = nViewPos - nPrevPosExpand;
                aRet.mbIsField = true;
            }

            break;
        }
    }

    return aRet;
}

} // namespace ModelToViewStringConverter end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
