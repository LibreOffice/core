/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: modeltoviewhelper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 13:21:07 $
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
