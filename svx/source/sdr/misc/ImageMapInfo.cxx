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

#include <svx/ImageMapInfo.hxx>

#include <svx/svdobj.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <vcl/imapobj.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>

SvxIMapInfo* SvxIMapInfo::GetIMapInfo(SdrObject const* pObject)
{
    assert(pObject);

    SvxIMapInfo* pIMapInfo = nullptr;
    sal_uInt16 nCount = pObject->GetUserDataCount();

    // Can we find IMap information within the user data?
    for (sal_uInt16 i = 0; i < nCount; i++)
    {
        SdrObjUserData* pUserData = pObject->GetUserData(i);

        if ((pUserData->GetInventor() == SdrInventor::StarDrawUserData)
            && (pUserData->GetId() == SVX_IMAPINFO_ID))
            pIMapInfo = static_cast<SvxIMapInfo*>(pUserData);
    }

    return pIMapInfo;
}

IMapObject* SvxIMapInfo::GetHitIMapObject(const SdrObject* pObj, const Point& rWinPoint,
                                          const OutputDevice* pCmpWnd)
{
    SvxIMapInfo* pIMapInfo = GetIMapInfo(pObj);
    IMapObject* pIMapObj = nullptr;

    if (pIMapInfo)
    {
        const MapMode aMap100(MapUnit::Map100thMM);
        Size aGraphSize;
        Point aRelPoint(rWinPoint);
        ImageMap& rImageMap = const_cast<ImageMap&>(pIMapInfo->GetImageMap());
        tools::Rectangle& rRect = const_cast<tools::Rectangle&>(pObj->GetLogicRect());

        if (pCmpWnd)
        {
            MapMode aWndMode = pCmpWnd->GetMapMode();
            aRelPoint = pCmpWnd->LogicToLogic(rWinPoint, &aWndMode, &aMap100);
            rRect = pCmpWnd->LogicToLogic(pObj->GetLogicRect(), &aWndMode, &aMap100);
        }

        bool bObjSupported = false;

        // execute HitTest
        if (auto pGrafObj = dynamic_cast<const SdrGrafObj*>(pObj)) // simple graphics object
        {
            const GeoStat& rGeo = pGrafObj->GetGeoStat();
            std::unique_ptr<SdrGrafObjGeoData> pGeoData(
                static_cast<SdrGrafObjGeoData*>(pGrafObj->GetGeoData().release()));

            // Undo rotation
            if (rGeo.nRotationAngle)
                RotatePoint(aRelPoint, rRect.TopLeft(), -rGeo.mfSinRotationAngle,
                            rGeo.mfCosRotationAngle);

            // Undo mirroring
            if (pGeoData->bMirrored)
                aRelPoint.setX(rRect.Right() + rRect.Left() - aRelPoint.X());

            // Undo shearing
            if (rGeo.nShearAngle)
                ShearPoint(aRelPoint, rRect.TopLeft(), -rGeo.mfTanShearAngle);

            if (pGrafObj->GetGrafPrefMapMode().GetMapUnit() == MapUnit::MapPixel)
                aGraphSize = Application::GetDefaultDevice()->PixelToLogic(
                    pGrafObj->GetGrafPrefSize(), aMap100);
            else
                aGraphSize = OutputDevice::LogicToLogic(pGrafObj->GetGrafPrefSize(),
                                                        pGrafObj->GetGrafPrefMapMode(), aMap100);

            bObjSupported = true;
        }
        else if (auto pOleObj = dynamic_cast<const SdrOle2Obj*>(pObj)) // OLE object
        {
            aGraphSize = pOleObj->GetOrigObjSize();
            bObjSupported = true;
        }

        // Everything worked out well, thus execute HitTest
        if (bObjSupported)
        {
            // Calculate relative position of mouse cursor
            aRelPoint -= rRect.TopLeft();
            pIMapObj = rImageMap.GetHitIMapObject(aGraphSize, rRect.GetSize(), aRelPoint);

            // We don't care about deactivated objects
            if (pIMapObj && !pIMapObj->IsActive())
                pIMapObj = nullptr;
        }
    }

    return pIMapObj;
}
