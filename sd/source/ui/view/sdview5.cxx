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
#include "precompiled_sd.hxx"

#include "sdpage.hxx"
#include "View.hxx"
#include "pres.hxx"

namespace sd {

static bool implIsMultiPresObj( PresObjKind eKind )
{
    switch( eKind )
    {
    case PRESOBJ_OUTLINE:
    case PRESOBJ_GRAPHIC:
    case PRESOBJ_OBJECT:
    case PRESOBJ_CHART:
    case PRESOBJ_ORGCHART:
    case PRESOBJ_TABLE:
    case PRESOBJ_IMAGE:
    case PRESOBJ_MEDIA:
        return true;
    default:
        return false;
    }
}

SdrObject* View::GetEmptyPresentationObject( PresObjKind eKind )
{
    SdrObject* pEmptyObj = 0;

    SdrPageView*    pPV = GetSdrPageView();
    if( pPV )
    {
        SdPage* pPage = static_cast< SdPage* >( pPV->GetPage() );
        if( pPage && !pPage->IsMasterPage() )
        {
            // first try selected shape
            if ( AreObjectsMarked() )
            {
                /**********************************************************
                * Is an empty graphic object available?
                **********************************************************/
                const SdrMarkList& rMarkList = GetMarkedObjectList();

                if (rMarkList.GetMarkCount() == 1)
                {
                    SdrMark* pMark = rMarkList.GetMark(0);
                    SdrObject* pObj = pMark->GetMarkedSdrObj();

                    if( pObj->IsEmptyPresObj() && implIsMultiPresObj( pPage->GetPresObjKind(pObj) ) )
                        pEmptyObj = pObj;
                }
            }

            // try to find empty pres obj of same type
            if( !pEmptyObj )
            {
                int nIndex = 1;
                do
                {
                    pEmptyObj = pPage->GetPresObj(eKind, nIndex++ );
                }
                while( (pEmptyObj != 0) && (!pEmptyObj->IsEmptyPresObj()) );
            }

            // last try to find empty pres obj of multiple type
            if( !pEmptyObj )
            {
                const std::list< SdrObject* >& rShapes = pPage->GetPresentationShapeList().getList();

                for( std::list< SdrObject* >::const_iterator iter( rShapes.begin() ); iter != rShapes.end(); iter++ )
                {
                    if( (*iter)->IsEmptyPresObj() && implIsMultiPresObj(pPage->GetPresObjKind(*iter)) )
                    {
                        pEmptyObj = (*iter);
                        break;
                    }
                }
            }
        }
    }

    return pEmptyObj;
}

}
