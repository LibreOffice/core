/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_svx.hxx"
#include <com/sun/star/uno/Sequence.hxx>

#include <svx/svdlayer.hxx>
#include <svx/svdmodel.hxx> // fuer Broadcasting
#include "svx/svdglob.hxx"  // StringCache
#include "svx/svdstr.hrc"   // Namen aus der Resource

////////////////////////////////////////////////////////////////////////////////////////////////////
// SetOfByte
////////////////////////////////////////////////////////////////////////////////////////////////////

sal_Bool SetOfByte::IsEmpty() const
{
    for(sal_uInt16 i(0); i < 32; i++)
    {
        if(aData[i] != 0)
            return sal_False;
    }

    return sal_True;
}

sal_Bool SetOfByte::IsFull() const
{
    for(sal_uInt16 i(0); i < 32; i++)
    {
        if(aData[i] != 0xFF)
            return sal_False;
    }

    return sal_True;
}

sal_uInt16 SetOfByte::GetSetCount() const
{
    sal_uInt16 nRet(0);

    for(sal_uInt16 i(0); i < 32; i++)
    {
        sal_uInt8 a(aData[i]);

        if(a != 0)
        {
            if(a & 0x80) nRet++;
            if(a & 0x40) nRet++;
            if(a & 0x20) nRet++;
            if(a & 0x10) nRet++;
            if(a & 0x08) nRet++;
            if(a & 0x04) nRet++;
            if(a & 0x02) nRet++;
            if(a & 0x01) nRet++;
        }
    }

    return nRet;
}

sal_uInt8 SetOfByte::GetSetBit(sal_uInt16 nNum) const
{
    nNum++;
    sal_uInt16 i(0), j(0);
    sal_uInt16 nRet(0);

    while(j < nNum && i < 256)
    {
        if(IsSet(sal_uInt8(i)))
            j++;
        i++;
    }

    if(j == nNum)
        nRet = i - 1;

    return sal_uInt8(nRet);
}

sal_uInt16 SetOfByte::GetClearCount() const
{
    return sal_uInt16(256 - GetSetCount());
}

sal_uInt8 SetOfByte::GetClearBit(sal_uInt16 nNum) const
{
    nNum++;
    sal_uInt16 i(0), j(0);
    sal_uInt16 nRet(0);

    while(j < nNum && i < 256)
    {
        if(!IsSet(sal_uInt8(i)))
            j++;
        i++;
    }

    if(j == nNum)
        nRet = i - 1;

    return sal_uInt8(nRet);
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
void SetOfByte::PutValue( const com::sun::star::uno::Any & rAny )
{
    com::sun::star::uno::Sequence< sal_Int8 > aSeq;
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
void SetOfByte::QueryValue( com::sun::star::uno::Any & rAny ) const
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

    com::sun::star::uno::Sequence< sal_Int8 > aSeq( nNumBytesSet );

    for( nIndex = 0; nIndex < nNumBytesSet; nIndex++ )
    {
        aSeq[nIndex] = static_cast<sal_Int8>(aData[nIndex]);
    }

    rAny <<= aSeq;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// SdrLayer
////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrLayer::SetStandardLayer(bool bStd)
{
    nType=(sal_uInt16)bStd;
    if (bStd) {
        aName=ImpGetResStr(STR_StandardLayerName);
    }
    if (pModel!=NULL) {
        SdrHint aHint(HINT_LAYERCHG);
        pModel->Broadcast(aHint);
        pModel->SetChanged();
    }
}

void SdrLayer::SetName(const XubString& rNewName)
{
    if(!rNewName.Equals(aName))
    {
        aName = rNewName;
        nType = 0; // Userdefined

        if(pModel)
        {
            SdrHint aHint(HINT_LAYERCHG);

            pModel->Broadcast(aHint);
            pModel->SetChanged();
        }
    }
}

bool SdrLayer::operator==(const SdrLayer& rCmpLayer) const
{
    return (nID == rCmpLayer.nID
        && nType == rCmpLayer.nType
        && aName.Equals(rCmpLayer.aName));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// SdrLayerAdmin
////////////////////////////////////////////////////////////////////////////////////////////////////

SdrLayerAdmin::SdrLayerAdmin(SdrLayerAdmin* pNewParent):
    aLayer(1024,16,16),
    aLSets(1024,16,16),
    pModel(NULL)
{
    aControlLayerName = String(RTL_CONSTASCII_USTRINGPARAM("Controls"));
    pParent=pNewParent;
}

SdrLayerAdmin::SdrLayerAdmin(const SdrLayerAdmin& rSrcLayerAdmin):
    aLayer(1024,16,16),
    aLSets(1024,16,16),
    pParent(NULL),
    pModel(NULL)
{
    aControlLayerName = String(RTL_CONSTASCII_USTRINGPARAM("Controls"));
    *this = rSrcLayerAdmin;
}

SdrLayerAdmin::~SdrLayerAdmin()
{
    ClearLayer();
}

void SdrLayerAdmin::ClearLayer()
{
    SdrLayer* pL;
    pL=(SdrLayer*)aLayer.First();
    while (pL!=NULL) {
        delete pL;
        pL=(SdrLayer*)aLayer.Next();
    }
    aLayer.Clear();
}

const SdrLayerAdmin& SdrLayerAdmin::operator=(const SdrLayerAdmin& rSrcLayerAdmin)
{
    ClearLayer();
    pParent=rSrcLayerAdmin.pParent;
    sal_uInt16 i;
    sal_uInt16 nAnz=rSrcLayerAdmin.GetLayerCount();
    for (i=0; i<nAnz; i++) {
        aLayer.Insert(new SdrLayer(*rSrcLayerAdmin.GetLayer(i)),CONTAINER_APPEND);
    }
    return *this;
}

bool SdrLayerAdmin::operator==(const SdrLayerAdmin& rCmpLayerAdmin) const
{
    if (pParent!=rCmpLayerAdmin.pParent ||
        aLayer.Count()!=rCmpLayerAdmin.aLayer.Count() ||
        aLSets.Count()!=rCmpLayerAdmin.aLSets.Count()) return sal_False;
    bool bOk = true;
    sal_uInt16 nAnz=GetLayerCount();
    sal_uInt16 i=0;
    while (bOk && i<nAnz) {
        bOk=*GetLayer(i)==*rCmpLayerAdmin.GetLayer(i);
        i++;
    }
    return bOk;
}

void SdrLayerAdmin::SetModel(SdrModel* pNewModel)
{
    if (pNewModel!=pModel) {
        pModel=pNewModel;
        sal_uInt16 nAnz=GetLayerCount();
        sal_uInt16 i;
        for (i=0; i<nAnz; i++) {
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
    SdrLayer* pRetLayer=(SdrLayer*)(aLayer.Remove(nPos));
    Broadcast();
    return pRetLayer;
}

SdrLayer* SdrLayerAdmin::NewLayer(const XubString& rName, sal_uInt16 nPos)
{
    SdrLayerID nID=GetUniqueLayerID();
    SdrLayer* pLay=new SdrLayer(nID,rName);
    pLay->SetModel(pModel);
    aLayer.Insert(pLay,nPos);
    Broadcast();
    return pLay;
}

SdrLayer* SdrLayerAdmin::NewStandardLayer(sal_uInt16 nPos)
{
    SdrLayerID nID=GetUniqueLayerID();
    SdrLayer* pLay=new SdrLayer(nID,String());
    pLay->SetStandardLayer();
    pLay->SetModel(pModel);
    aLayer.Insert(pLay,nPos);
    Broadcast();
    return pLay;
}

SdrLayer* SdrLayerAdmin::MoveLayer(sal_uInt16 nPos, sal_uInt16 nNewPos)
{
    SdrLayer* pLayer=(SdrLayer*)(aLayer.Remove(nPos));
    if (pLayer!=NULL) {
        aLayer.Insert(pLayer,nNewPos);
    }

    Broadcast();
    return pLayer;
}

void SdrLayerAdmin::MoveLayer(SdrLayer* pLayer, sal_uInt16 nNewPos)
{
    sal_uIntPtr nPos=aLayer.GetPos(pLayer);
    if (nPos!=CONTAINER_ENTRY_NOTFOUND) {
        aLayer.Remove(nPos);
        aLayer.Insert(pLayer,nNewPos);
        Broadcast();
    }
}

sal_uInt16 SdrLayerAdmin::GetLayerPos(SdrLayer* pLayer) const
{
    sal_uIntPtr nRet=SDRLAYER_NOTFOUND;
    if (pLayer!=NULL) {
        nRet=aLayer.GetPos(pLayer);
        if (nRet==CONTAINER_ENTRY_NOTFOUND) {
            nRet=SDRLAYER_NOTFOUND;
        }
    }
    return sal_uInt16(nRet);
}

const SdrLayer* SdrLayerAdmin::GetLayer(const XubString& rName, bool /*bInherited*/) const
{
    sal_uInt16 i(0);
    const SdrLayer* pLay = NULL;

    while(i < GetLayerCount() && !pLay)
    {
        if(rName.Equals(GetLayer(i)->GetName()))
            pLay = GetLayer(i);
        else
            i++;
    }

    if(!pLay && pParent)
    {
        pLay = pParent->GetLayer(rName, sal_True);
    }

    return pLay;
}

SdrLayerID SdrLayerAdmin::GetLayerID(const XubString& rName, bool bInherited) const
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

// Globale LayerID's beginnen mit 0 aufsteigend.
// Lokale LayerID's beginnen mit 254 absteigend.
// 255 ist reserviert fuer SDRLAYER_NOTFOUND

SdrLayerID SdrLayerAdmin::GetUniqueLayerID() const
{
    SetOfByte aSet;
    sal_Bool bDown = (pParent == NULL);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
