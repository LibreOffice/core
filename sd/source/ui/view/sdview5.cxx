/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


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

SdPage* View::GetPage()
{
    SdPage* pPage = NULL;
    SdrPageView*    pPV = GetSdrPageView();
    if( pPV )
    {
        pPage = static_cast< SdPage* >( pPV->GetPage() );
    }

    return pPage;
}


SdrObject* View::GetSelectedSingleObject(SdPage* pPage)
{
    SdrObject* pRet = NULL;
    if( pPage )
    {
        
        if ( AreObjectsMarked() )
        {
            const SdrMarkList& rMarkList = GetMarkedObjectList();

            if (rMarkList.GetMarkCount() == 1)
            {
                SdrMark* pMark = rMarkList.GetMark(0);
                pRet = pMark->GetMarkedSdrObj();
            }
        }
    }

    return pRet;
}

SdrObject* View::GetEmptyPresentationObject( PresObjKind eKind )
{
    SdPage* pPage = GetPage();
    SdrObject* pEmptyObj = NULL;

    if ( pPage && !pPage->IsMasterPage() ) {
        SdrObject* pObj = GetSelectedSingleObject( pPage );

        if( pObj && pObj->IsEmptyPresObj() && implIsMultiPresObj( pPage->GetPresObjKind(pObj) ) )
            pEmptyObj = pObj;

        
        if( !pEmptyObj )
        {
            int nIndex = 1;
            do
            {
                pEmptyObj = pPage->GetPresObj(eKind, nIndex++ );
            }
            while( (pEmptyObj != 0) && (!pEmptyObj->IsEmptyPresObj()) );
        }

        
        if( !pEmptyObj )
        {
            const std::list< SdrObject* >& rShapes = pPage->GetPresentationShapeList().getList();

            for( std::list< SdrObject* >::const_iterator iter( rShapes.begin() ); iter != rShapes.end(); ++iter )
            {
                if( (*iter)->IsEmptyPresObj() && implIsMultiPresObj(pPage->GetPresObjKind(*iter)) )
                {
                    pEmptyObj = (*iter);
                    break;
                }
            }
        }
    }

    return pEmptyObj;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
