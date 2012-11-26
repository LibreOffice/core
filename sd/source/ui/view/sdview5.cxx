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
        SdPage& rPage = static_cast< SdPage& >( pPV->getSdrPageFromSdrPageView() );
        if( !rPage.IsMasterPage() )
        {
            // first try selected shape
            SdrObject* pSelectedObj = getSelectedIfSingle();

                /**********************************************************
                * Is an empty graphic object available?
                **********************************************************/
            if( pSelectedObj
                && pSelectedObj->IsEmptyPresObj()
                && implIsMultiPresObj( rPage.GetPresObjKind(pSelectedObj) ) )
                {
                pEmptyObj = pSelectedObj;
            }

            // try to find empty pres obj of same type
            if( !pEmptyObj )
            {
                int nIndex = 1;
                do
                {
                    pEmptyObj = rPage.GetPresObj(eKind, nIndex++ );
                }
                while( (pEmptyObj != 0) && (!pEmptyObj->IsEmptyPresObj()) );
            }

            // last try to find empty pres obj of multiple type
            if( !pEmptyObj )
            {
                const std::list< const SdrObject* >& rShapes = rPage.GetPresentationShapeList().getList();

                for( std::list< const SdrObject* >::const_iterator iter( rShapes.begin() ); iter != rShapes.end(); iter++ )
                {
                    if( (*iter)->IsEmptyPresObj() && implIsMultiPresObj(rPage.GetPresObjKind(*iter)) )
                    {
                        pEmptyObj = const_cast< SdrObject* >(*iter);
                        break;
                    }
                }
            }
        }
    }

    return pEmptyObj;
}

}
