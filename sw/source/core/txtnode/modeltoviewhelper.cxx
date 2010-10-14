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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
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

            const sal_uInt32 nFieldLengthExpand = nLengthExpand - nLengthModel + 1;
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
