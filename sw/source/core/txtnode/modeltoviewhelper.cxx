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
