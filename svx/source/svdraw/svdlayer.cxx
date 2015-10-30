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

#include <com/sun/star/uno/Sequence.hxx>

#include <svx/svdlayer.hxx>
#include <svx/svdmodel.hxx>
#include "svdglob.hxx"
#include "svx/svdstr.hrc"

bool SetOfByte::IsEmpty() const
{
    for(sal_uInt16 i(0); i < 32; i++)
    {
        if(aData[i] != 0)
            return false;
    }

    return true;
}

void SetOfByte::operator&=(const SetOfByte& r2ndSet)
{
    for(sal_uInt16 i(0); i < 32; i++)
    {
        aData[i] &= r2ndSet.aData[i];
    }
}

void SetOfByte::operator|=(const SetOfByte& r2ndSet)
{
    for(sal_uInt16 i(0); i < 32; i++)
    {
        aData[i] |= r2ndSet.aData[i];
    }
}

/** initialize this set with a uno sequence of sal_Int8
*/
void SetOfByte::PutValue( const css::uno::Any & rAny )
{
    css::uno::Sequence< sal_Int8 > aSeq;
    if( rAny >>= aSeq )
    {
        sal_Int16 nCount = (sal_Int16)aSeq.getLength();
        if( nCount > 32 )
            nCount = 32;

        sal_Int16 nIndex;
        for( nIndex = 0; nIndex < nCount; nIndex++ )
        {
            aData[nIndex] = static_cast<sal_uInt8>(aSeq[nIndex]);
        }

        for( ; nIndex < 32; nIndex++ )
        {
            aData[nIndex] = 0;
        }
    }
}

/** returns a uno sequence of sal_Int8
*/
void SetOfByte::QueryValue( css::uno::Any & rAny ) const
{
    sal_Int16 nNumBytesSet = 0;
    sal_Int16 nIndex;
    for( nIndex = 31; nIndex >= 00; nIndex-- )
    {
        if( 0 != aData[nIndex] )
        {
            nNumBytesSet = nIndex + 1;
            break;
        }
    }

    css::uno::Sequence< sal_Int8 > aSeq( nNumBytesSet );

    for( nIndex = 0; nIndex < nNumBytesSet; nIndex++ )
    {
        aSeq[nIndex] = static_cast<sal_Int8>(aData[nIndex]);
    }

    rAny <<= aSeq;
}

SdrLayer::SdrLayer(SdrLayerID nNewID, const OUString& rNewName) :
    maName(rNewName), pModel(NULL), nType(0), nID(nNewID)
{
}

void SdrLayer::SetStandardLayer(bool bStd)
{
    nType=(sal_uInt16)bStd;
    if (bStd) {
        maName = ImpGetResStr(STR_StandardLayerName);
    }
    if (pModel!=NULL) {
        SdrHint aHint(HINT_LAYERCHG);
        pModel->Broadcast(aHint);
        pModel->SetChanged();
    }
}

void SdrLayer::SetName(const OUString& rNewName)
{
    if (rNewName == maName)
        return;

    maName = rNewName;
    nType = 0; // user defined

    if (pModel)
    {
        SdrHint aHint(HINT_LAYERCHG);
        pModel->Broadcast(aHint);
        pModel->SetChanged();
    }
}

bool SdrLayer::operator==(const SdrLayer& rCmpLayer) const
{
    return (nID == rCmpLayer.nID
        && nType == rCmpLayer.nType
        && maName == rCmpLayer.maName);
}

SdrLayerAdmin::SdrLayerAdmin(SdrLayerAdmin* pNewParent):
    aLayer(),
    pParent(pNewParent),
    pModel(NULL),
    maControlLayerName("Controls")
{
}

SdrLayerAdmin::SdrLayerAdmin(const SdrLayerAdmin& rSrcLayerAdmin):
    aLayer(),
    pParent(NULL),
    pModel(NULL),
    maControlLayerName("Controls")
{
    *this = rSrcLayerAdmin;
}

SdrLayerAdmin::~SdrLayerAdmin()
{
    ClearLayer();
}

void SdrLayerAdmin::ClearLayer()
{
    for( std::vector<SdrLayer*>::const_iterator it = aLayer.begin(); it != aLayer.end(); ++it )
        delete *it;
    aLayer.clear();
}

const SdrLayerAdmin& SdrLayerAdmin::operator=(const SdrLayerAdmin& rSrcLayerAdmin)
{
    ClearLayer();
    pParent=rSrcLayerAdmin.pParent;
    sal_uInt16 i;
    sal_uInt16 nCount=rSrcLayerAdmin.GetLayerCount();
    for (i=0; i<nCount; i++) {
        aLayer.push_back(new SdrLayer(*rSrcLayerAdmin.GetLayer(i)));
    }
    return *this;
}

bool SdrLayerAdmin::operator==(const SdrLayerAdmin& rCmpLayerAdmin) const
{
    if (pParent!=rCmpLayerAdmin.pParent ||
        aLayer.size()!=rCmpLayerAdmin.aLayer.size())
        return false;
    bool bOk = true;
    sal_uInt16 nCount=GetLayerCount();
    sal_uInt16 i=0;
    while (bOk && i<nCount) {
        bOk=*GetLayer(i)==*rCmpLayerAdmin.GetLayer(i);
        i++;
    }
    return bOk;
}

void SdrLayerAdmin::SetModel(SdrModel* pNewModel)
{
    if (pNewModel!=pModel) {
        pModel=pNewModel;
        sal_uInt16 nCount=GetLayerCount();
        sal_uInt16 i;
        for (i=0; i<nCount; i++) {
            GetLayer(i)->SetModel(pNewModel);
        }
    }
}

void SdrLayerAdmin::Broadcast() const
{
    if (pModel!=NULL) {
        SdrHint aHint(HINT_LAYERORDERCHG);
        pModel->Broadcast(aHint);
        pModel->SetChanged();
    }
}

SdrLayer* SdrLayerAdmin::RemoveLayer(sal_uInt16 nPos)
{
    SdrLayer* pRetLayer=aLayer[nPos];
    aLayer.erase(aLayer.begin()+nPos);
    Broadcast();
    return pRetLayer;
}

SdrLayer* SdrLayerAdmin::NewLayer(const OUString& rName, sal_uInt16 nPos)
{
    SdrLayerID nID=GetUniqueLayerID();
    SdrLayer* pLay=new SdrLayer(nID,rName);
    pLay->SetModel(pModel);
    if(nPos==0xFFFF)
        aLayer.push_back(pLay);
    else
        aLayer.insert(aLayer.begin() + nPos, pLay);
    Broadcast();
    return pLay;
}

SdrLayer* SdrLayerAdmin::NewStandardLayer(sal_uInt16 nPos)
{
    SdrLayerID nID=GetUniqueLayerID();
    SdrLayer* pLay=new SdrLayer(nID,OUString());
    pLay->SetStandardLayer();
    pLay->SetModel(pModel);
    if(nPos==0xFFFF)
        aLayer.push_back(pLay);
    else
        aLayer.insert(aLayer.begin() + nPos, pLay);
    Broadcast();
    return pLay;
}

sal_uInt16 SdrLayerAdmin::GetLayerPos(SdrLayer* pLayer) const
{
    sal_uIntPtr nRet=SDRLAYER_NOTFOUND;
    if (pLayer!=NULL) {
        std::vector<SdrLayer*>::const_iterator it = std::find(aLayer.begin(), aLayer.end(), pLayer);
        if (it==aLayer.end()) {
            nRet=SDRLAYER_NOTFOUND;
        } else {
            nRet=it - aLayer.begin();
        }
    }
    return sal_uInt16(nRet);
}

SdrLayer* SdrLayerAdmin::GetLayer(const OUString& rName, bool bInherited)
{
    return const_cast<SdrLayer*>(const_cast<const SdrLayerAdmin*>(this)->GetLayer(rName, bInherited));
}

const SdrLayer* SdrLayerAdmin::GetLayer(const OUString& rName, bool /*bInherited*/) const
{
    sal_uInt16 i(0);
    const SdrLayer* pLay = NULL;

    while(i < GetLayerCount() && !pLay)
    {
        if (rName == GetLayer(i)->GetName())
            pLay = GetLayer(i);
        else
            i++;
    }

    if(!pLay && pParent)
    {
        pLay = pParent->GetLayer(rName, true);
    }

    return pLay;
}

SdrLayerID SdrLayerAdmin::GetLayerID(const OUString& rName, bool bInherited) const
{
    SdrLayerID nRet=SDRLAYER_NOTFOUND;
    const SdrLayer* pLay=GetLayer(rName,bInherited);
    if (pLay!=NULL) nRet=pLay->GetID();
    return nRet;
}

const SdrLayer* SdrLayerAdmin::GetLayerPerID(sal_uInt16 nID) const
{
    sal_uInt16 i=0;
    const SdrLayer* pLay=NULL;
    while (i<GetLayerCount() && pLay==NULL) {
        if (nID==GetLayer(i)->GetID()) pLay=GetLayer(i);
        else i++;
    }
    return pLay;
}

// Global LayerIDs begin at 0 and increase,
// local LayerIDs begin at 254 and decrease;
// 255 is reserved for SDRLAYER_NOTFOUND.

SdrLayerID SdrLayerAdmin::GetUniqueLayerID() const
{
    SetOfByte aSet;
    bool bDown = (pParent == NULL);
    sal_uInt16 j;
    for (j=0; j<GetLayerCount(); j++)
    {
        aSet.Set(GetLayer((sal_uInt16)j)->GetID());
    }
    SdrLayerID i;
    if (!bDown)
    {
        i=254;
        while (i && aSet.IsSet(sal_uInt8(i)))
            --i;
        if (i == 0)
            i=254;
    }
    else
    {
        i=0;
        while (i<=254 && aSet.IsSet(sal_uInt8(i)))
            i++;
        if (i>254)
            i=0;
    }
    return i;
}

void SdrLayerAdmin::SetControlLayerName(const OUString& rNewName)
{
    maControlLayerName = rNewName;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
