/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <svdobjplusdata.hxx>
#include <svdobjuserdatalist.hxx>
#include <o3tl/deleter.hxx>
#include <svx/svdglue.hxx>
#include <svl/SfxBroadcaster.hxx>
#include <osl/diagnose.h>

SdrObjPlusData::SdrObjPlusData()
{
}

SdrObjPlusData::~SdrObjPlusData()
{
    o3tl::reset_preserve_ptr_during(pBroadcast);
    pUserDataList.reset();
    pGluePoints.reset();
}

SdrObjPlusData* SdrObjPlusData::Clone(SdrObject* pObj1) const
{
    SdrObjPlusData* pNewPlusData=new SdrObjPlusData;
    if (pUserDataList!=nullptr) {
        sal_uInt16 nCount=pUserDataList->GetUserDataCount();
        if (nCount!=0) {
            pNewPlusData->pUserDataList.reset(new SdrObjUserDataList);
            for (sal_uInt16 i=0; i<nCount; i++) {
                std::unique_ptr<SdrObjUserData> pNewUserData=pUserDataList->GetUserData(i).Clone(pObj1);
                if (pNewUserData!=nullptr) {
                    pNewPlusData->pUserDataList->AppendUserData(std::move(pNewUserData));
                } else {
                    OSL_FAIL("SdrObjPlusData::Clone(): UserData.Clone() returns NULL.");
                }
            }
        }
    }
    if (pGluePoints!=nullptr) pNewPlusData->pGluePoints.reset(new SdrGluePointList(*pGluePoints));
    // MtfAnimator isn't copied either

    // #i68101#
    // copy object name, title and description
    pNewPlusData->aObjName = aObjName;
    pNewPlusData->aObjTitle = aObjTitle;
    pNewPlusData->aObjDescription = aObjDescription;

    return pNewPlusData;
}

void SdrObjPlusData::SetGluePoints(const SdrGluePointList& rPts)
{
    *pGluePoints = rPts;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
