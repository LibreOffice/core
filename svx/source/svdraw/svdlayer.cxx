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
#include "svx/strings.hrc"

bool SdrLayerIDSet::IsEmpty() const
{
    for(sal_uInt8 i : aData)
    {
        if(i != 0)
            return false;
    }

    return true;
}

void SdrLayerIDSet::operator&=(const SdrLayerIDSet& r2ndSet)
{
    for(sal_uInt16 i(0); i < 32; i++)
    {
        aData[i] &= r2ndSet.aData[i];
    }
}

/** initialize this set with a uno sequence of sal_Int8
*/
void SdrLayerIDSet::PutValue( const css::uno::Any & rAny )
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
void SdrLayerIDSet::QueryValue( css::uno::Any & rAny ) const
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
    maName(rNewName), pModel(nullptr), nType(0), nID(nNewID)
{
}

void SdrLayer::SetStandardLayer()
{
    nType=(sal_uInt16)true;
    maName = ImpGetResStr(STR_StandardLayerName);
    if (pModel!=nullptr) {
        SdrHint aHint(SdrHintKind::LayerChange);
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
        SdrHint aHint(SdrHintKind::LayerChange);
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
    pModel(nullptr),
    maControlLayerName("Controls")
{
}

SdrLayerAdmin::SdrLayerAdmin(const SdrLayerAdmin& rSrcLayerAdmin):
    aLayer(),
    pParent(nullptr),
    pModel(nullptr),
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

SdrLayerAdmin& SdrLayerAdmin::operator=(const SdrLayerAdmin& rSrcLayerAdmin)
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
    if (pModel!=nullptr) {
        SdrHint aHint(SdrHintKind::LayerOrderChange);
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

void SdrLayerAdmin::NewStandardLayer(sal_uInt16 nPos)
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
}

sal_uInt16 SdrLayerAdmin::GetLayerPos(SdrLayer* pLayer) const
{
    sal_uInt16 nRet=SDRLAYERPOS_NOTFOUND;
    if (pLayer!=nullptr) {
        std::vector<SdrLayer*>::const_iterator it = std::find(aLayer.begin(), aLayer.end(), pLayer);
        if (it==aLayer.end()) {
            nRet=SDRLAYERPOS_NOTFOUND;
        } else {
            nRet=it - aLayer.begin();
        }
    }
    return nRet;
}

SdrLayer* SdrLayerAdmin::GetLayer(const OUString& rName)
{
    return const_cast<SdrLayer*>(const_cast<const SdrLayerAdmin*>(this)->GetLayer(rName));
}

const SdrLayer* SdrLayerAdmin::GetLayer(const OUString& rName) const
{
    sal_uInt16 i(0);
    const SdrLayer* pLay = nullptr;

    while(i < GetLayerCount() && !pLay)
    {
        if (rName == GetLayer(i)->GetName())
            pLay = GetLayer(i);
        else
            i++;
    }

    if(!pLay && pParent)
    {
        pLay = pParent->GetLayer(rName);
    }

    return pLay;
}

SdrLayerID SdrLayerAdmin::GetLayerID(const OUString& rName) const
{
    SdrLayerID nRet=SDRLAYER_NOTFOUND;
    const SdrLayer* pLay=GetLayer(rName);
    if (pLay!=nullptr) nRet=pLay->GetID();
    return nRet;
}

const SdrLayer* SdrLayerAdmin::GetLayerPerID(SdrLayerID nID) const
{
    for (SdrLayer* pLayer : aLayer)
        if (pLayer->GetID() == nID)
            return pLayer;
    return nullptr;
}

// Global LayerIDs begin at 0 and increase,
// local LayerIDs begin at 254 and decrease;
// 255 is reserved for SDRLAYER_NOTFOUND.

SdrLayerID SdrLayerAdmin::GetUniqueLayerID() const
{
    SdrLayerIDSet aSet;
    for (sal_uInt16 j=0; j<GetLayerCount(); j++)
    {
        aSet.Set(GetLayer(j)->GetID());
    }
    sal_uInt8 i;
    if (pParent != nullptr)
    {
        i = 254;
        while (i && aSet.IsSet(SdrLayerID(i)))
            --i;
        assert(i != 0);
        if (i == 0)
            i = 254;
    }
    else
    {
        i = 0;
        while (i<=254 && aSet.IsSet(SdrLayerID(i)))
            i++;
        assert(i <= 254);
        if (i>254)
            i = 0;
    }
    return SdrLayerID(i);
}

void SdrLayerAdmin::SetControlLayerName(const OUString& rNewName)
{
    maControlLayerName = rNewName;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
