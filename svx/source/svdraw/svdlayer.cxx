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
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>

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

/** initialize this set with a uno sequence of sal_Int8 (e.g. as stored in settings.xml)
*/
void SdrLayerIDSet::PutValue( const css::uno::Any & rAny )
{
    css::uno::Sequence< sal_Int8 > aSeq;
    if( rAny >>= aSeq )
    {
        sal_Int16 nCount = static_cast<sal_Int16>(aSeq.getLength());
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

SdrLayer::SdrLayer(SdrLayerID nNewID, const OUString& rNewName) :
    maName(rNewName), pModel(nullptr), nID(nNewID)
{
    // ODF default values
    mbVisibleODF = true;
    mbPrintableODF = true;
    mbLockedODF = false;
}

void SdrLayer::SetName(const OUString& rNewName)
{
    if (rNewName == maName)
        return;

    maName = rNewName;

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
        && maName == rCmpLayer.maName);
}

SdrLayerAdmin::SdrLayerAdmin(SdrLayerAdmin* pNewParent):
    pParent(pNewParent),
    pModel(nullptr),
    maControlLayerName("controls")
{
}

SdrLayerAdmin::SdrLayerAdmin(const SdrLayerAdmin& rSrcLayerAdmin):
    pParent(nullptr),
    pModel(nullptr),
    maControlLayerName("controls")
{
    *this = rSrcLayerAdmin;
}

SdrLayerAdmin::~SdrLayerAdmin()
{
}

void SdrLayerAdmin::ClearLayers()
{
    maLayers.clear();
}

SdrLayerAdmin& SdrLayerAdmin::operator=(const SdrLayerAdmin& rSrcLayerAdmin)
{
    if (this != &rSrcLayerAdmin)
    {
        maLayers.clear();
        pParent=rSrcLayerAdmin.pParent;
        sal_uInt16 i;
        sal_uInt16 nCount=rSrcLayerAdmin.GetLayerCount();
        for (i=0; i<nCount; i++) {
            maLayers.emplace_back(new SdrLayer(*rSrcLayerAdmin.GetLayer(i)));
        }
    }
    return *this;
}

void SdrLayerAdmin::SetModel(SdrModel* pNewModelel)
{
    if (pNewModelel!=pModel) {
        pModel=pNewModelel;
        sal_uInt16 nCount=GetLayerCount();
        sal_uInt16 i;
        for (i=0; i<nCount; i++) {
            GetLayer(i)->SetModel(pNewModelel);
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

void SdrLayerAdmin::InsertLayer(std::unique_ptr<SdrLayer> pLayer, sal_uInt16 nPos)
{
        pLayer->SetModel(pModel);
        if(nPos==0xFFFF)
            maLayers.push_back(std::move(pLayer));
        else
            maLayers.insert(maLayers.begin() + nPos, std::move(pLayer));
        Broadcast();
}

std::unique_ptr<SdrLayer> SdrLayerAdmin::RemoveLayer(sal_uInt16 nPos)
{
    std::unique_ptr<SdrLayer> pRetLayer = std::move(maLayers[nPos]);
    maLayers.erase(maLayers.begin()+nPos);
    Broadcast();
    return pRetLayer;
}

SdrLayer* SdrLayerAdmin::NewLayer(const OUString& rName, sal_uInt16 nPos)
{
    SdrLayerID nID=GetUniqueLayerID();
    SdrLayer* pLay=new SdrLayer(nID,rName);
    pLay->SetModel(pModel);
    if(nPos==0xFFFF)
        maLayers.push_back(std::unique_ptr<SdrLayer>(pLay));
    else
        maLayers.insert(maLayers.begin() + nPos, std::unique_ptr<SdrLayer>(pLay));
    Broadcast();
    return pLay;
}

sal_uInt16 SdrLayerAdmin::GetLayerPos(const SdrLayer* pLayer) const
{
    sal_uInt16 nRet=SDRLAYERPOS_NOTFOUND;
    if (pLayer!=nullptr) {
        auto it = std::find_if(maLayers.begin(), maLayers.end(),
                    [&](const std::unique_ptr<SdrLayer> & p) { return p.get() == pLayer; });
        if (it!=maLayers.end()) {
            nRet=it - maLayers.begin();
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
    for (auto const & pLayer : maLayers)
        if (pLayer->GetID() == nID)
            return pLayer.get();
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

void  SdrLayerAdmin::getVisibleLayersODF( SdrLayerIDSet& rOutSet) const
{
    rOutSet.ClearAll();
    for( auto & pCurrentLayer : maLayers )
    {
        if ( pCurrentLayer->IsVisibleODF() )
            rOutSet.Set( pCurrentLayer->GetID() );
    }
}

void SdrLayerAdmin::getPrintableLayersODF( SdrLayerIDSet& rOutSet) const
{
    rOutSet.ClearAll();
    for( auto & pCurrentLayer : maLayers )
    {
        if ( pCurrentLayer->IsPrintableODF() )
            rOutSet.Set( pCurrentLayer->GetID() );
    }
}

void SdrLayerAdmin::getLockedLayersODF( SdrLayerIDSet& rOutSet) const
{
    rOutSet.ClearAll();
    for( auto& pCurrentLayer : maLayers )
    {
        if ( pCurrentLayer->IsLockedODF() )
            rOutSet.Set( pCurrentLayer->GetID() );
    }
}

    // Generates a bitfield for settings.xml from the SdrLayerIDSet.
    // Output is a uno sequence of BYTE (which is 'short' in API).
void SdrLayerAdmin::QueryValue(const SdrLayerIDSet& rViewLayerSet, css::uno::Any& rAny)
{
    // tdf#119392 The SdrLayerIDSet in a view is ordered according LayerID, but in file
    // the bitfield is interpreted in order of layers in <draw:layer-set>.
    // First generate a new bitfield based on rViewLayerSet in the needed order.
    sal_uInt8 aTmp[32]; // 256 bits in settings.xml makes byte 0 to 31
    for (auto nIndex = 0; nIndex <32; nIndex++)
    {
        aTmp[nIndex] = 0;
    }
    sal_uInt8 nByteIndex = 0;
    sal_uInt8 nBitpos = 0;
    sal_uInt16 nLayerPos = 0; // Position of the layer in member aLayer and in <draw:layer-set> in file
    sal_uInt16 nLayerIndex = 0;
    for( auto& pCurrentLayer : maLayers )
    {
        SdrLayerID nCurrentID = pCurrentLayer->GetID();
        if ( rViewLayerSet.IsSet(nCurrentID) )
        {
            nLayerPos = nLayerIndex;
            nByteIndex = nLayerPos / 8;
            if (nByteIndex > 31)
                continue; // skip position, if too large for bitfield
            nBitpos = nLayerPos % 8;
            aTmp[nByteIndex] |= (1 << nBitpos);
        }
        ++nLayerIndex;
    }

    // Second transform the bitfield to byte sequence, same as in previous version of QueryValue
    sal_uInt8 nNumBytesSet = 0;
    for( auto nIndex = 31; nIndex >= 0; nIndex--)
    {
        if( 0 != aTmp[nIndex] )
        {
            nNumBytesSet = nIndex + 1;
            break;
        }
    }
    css::uno::Sequence< sal_Int8 > aSeq( nNumBytesSet );
    for( auto nIndex = 0; nIndex < nNumBytesSet; nIndex++ )
    {
        aSeq[nIndex] = static_cast<sal_Int8>(aTmp[nIndex]);
    }
    rAny <<= aSeq;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
