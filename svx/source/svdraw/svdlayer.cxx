/*************************************************************************
 *
 *  $RCSfile: svdlayer.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "svdlayer.hxx"
#include "svdio.hxx"
#include "svdmodel.hxx" // fuer Broadcasting
#include "svdglob.hxx"  // StringCache
#include "svdstr.hrc"   // Namen aus der Resource

////////////////////////////////////////////////////////////////////////////////////////////////////
// SetOfByte
////////////////////////////////////////////////////////////////////////////////////////////////////

FASTBOOL SetOfByte::IsEmpty() const
{
    for (USHORT i=0; i<32; i++) {
        if (aData[i]!=0) return FALSE;
    }
    return TRUE;
}

FASTBOOL SetOfByte::IsFull() const
{
    for (USHORT i=0; i<32; i++) {
        if (aData[i]!=0xFF) return FALSE;
    }
    return TRUE;
}

USHORT SetOfByte::GetSetCount() const
{
    USHORT i;
    USHORT nRet=0;
    for (i=0; i<32; i++) {
        BYTE a=aData[i];
        if (a!=0) {
            if (a & 0x80) nRet++;
            if (a & 0x40) nRet++;
            if (a & 0x20) nRet++;
            if (a & 0x10) nRet++;
            if (a & 0x08) nRet++;
            if (a & 0x04) nRet++;
            if (a & 0x02) nRet++;
            if (a & 0x01) nRet++;
        }
    }
    return nRet;
}

BYTE SetOfByte::GetSetBit(USHORT nNum) const
{
    nNum++;
    USHORT i=0,j=0;
    USHORT nRet=0;
    while (j<nNum && i<256) {
        if (IsSet(BYTE(i))) j++;
        i++;
    }
    if (j==nNum) nRet=i-1;
    return BYTE(nRet);
}

USHORT SetOfByte::GetClearCount() const
{
    return USHORT(256-GetSetCount());
}

BYTE SetOfByte::GetClearBit(USHORT nNum) const
{
    nNum++;
    USHORT i=0,j=0;
    USHORT nRet=0;
    while (j<nNum && i<256) {
        if (!IsSet(BYTE(i))) j++;
        i++;
    }
    if (j==nNum) nRet=i-1;
    return BYTE(nRet);
}

void SetOfByte::operator&=(const SetOfByte& r2ndSet)
{
    for (unsigned i=0; i<32; i++) {
        aData[i]&=r2ndSet.aData[i];
    }
}

void SetOfByte::operator|=(const SetOfByte& r2ndSet)
{
    for (unsigned i=0; i<32; i++) {
        aData[i]|=r2ndSet.aData[i];
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// SdrLayer
////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrLayer::SetStandardLayer(FASTBOOL bStd)
{
    nType=(UINT16)bStd;
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

FASTBOOL SdrLayer::operator==(const SdrLayer& rCmpLayer) const
{
    return (nID == rCmpLayer.nID
        && nType == rCmpLayer.nType
        && aName.Equals(rCmpLayer.aName));
}

SvStream& operator>>(SvStream& rIn, SdrLayer& rLayer)
{
    if(rIn.GetError())
        return rIn;

    SdrIOHeader aHead(rIn, STREAM_READ);

    rIn >> rLayer.nID;

    // UNICODE: rIn >> rLayer.aName;
    rIn.ReadByteString(rLayer.aName);

    if(aHead.GetVersion() >= 1)
    {
        // Das Standardlayerflag kam direkt nach der Betalieferung dazu
        rIn >> rLayer.nType;

        if(rLayer.nType == 1)
        {
            rLayer.aName = ImpGetResStr(STR_StandardLayerName);
        }
    }

    if(aHead.GetVersion() <= 12)
    {
        // nType war lange Zeit nicht initiallisiert!
        if(rLayer.nType > 1)
            rLayer.nType = 0;
    }

    return rIn;
}

SvStream& operator<<(SvStream& rOut, const SdrLayer& rLayer)
{
    SdrIOHeader aHead(rOut, STREAM_WRITE, SdrIOLayrID);

    rOut << rLayer.nID;

    // UNICODE: rOut << rLayer.aName;
    rOut.WriteByteString(rLayer.aName);

    rOut << rLayer.nType;

    return rOut;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// SdrLayerSet
////////////////////////////////////////////////////////////////////////////////////////////////////

FASTBOOL SdrLayerSet::operator==(const SdrLayerSet& rCmpLayerSet) const
{
    return(aName.Equals(rCmpLayerSet.aName)
        && aMember == rCmpLayerSet.aMember
        && aExclude == rCmpLayerSet.aExclude);
}

void SdrLayerSet::Broadcast() const
{
    if (pModel!=NULL) {
        SdrHint aHint(HINT_LAYERSETCHG);
        pModel->Broadcast(aHint);
        pModel->SetChanged();
    }
}

SvStream& operator>>(SvStream& rIn, SdrLayerSet& rSet)
{
    if(rIn.GetError())
        return rIn;

    SdrIOHeader aHead(rIn, STREAM_READ);

    rIn >> rSet.aMember;
    rIn >> rSet.aExclude;

    // UNICODE: rIn >> rSet.aName;
    rIn.ReadByteString(rSet.aName);

    return rIn;
}

SvStream& operator<<(SvStream& rOut, const SdrLayerSet& rSet)
{
    SdrIOHeader aHead(rOut, STREAM_WRITE, SdrIOLSetID);

    rOut << rSet.aMember;
    rOut << rSet.aExclude;

    // UNICODE: rOut << rSet.aName;
    rOut.WriteByteString(rSet.aName);

    return rOut;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// SdrLayerAdmin
////////////////////////////////////////////////////////////////////////////////////////////////////

SdrLayerAdmin::SdrLayerAdmin(SdrLayerAdmin* pNewParent):
    aLayer(1024,16,16),
    aLSets(1024,16,16),
    pModel(NULL)
{
    sal_Char aTextControls[] = "Controls";
    aControlLayerName = String(aTextControls, sizeof(aTextControls-1));
    pParent=pNewParent;
}

SdrLayerAdmin::SdrLayerAdmin(const SdrLayerAdmin& rSrcLayerAdmin):
    aLayer(1024,16,16),
    aLSets(1024,16,16),
    pParent(NULL),
    pModel(NULL)
{
    sal_Char aTextControls[] = "Controls";
    aControlLayerName = String(aTextControls, sizeof(aTextControls-1));
    *this = rSrcLayerAdmin;
}

SdrLayerAdmin::~SdrLayerAdmin()
{
    ClearLayer();
    ClearLayerSets();
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

void SdrLayerAdmin::ClearLayerSets()
{
    SdrLayerSet* pL;
    pL=(SdrLayerSet*)aLSets.First();
    while (pL!=NULL) {
        delete pL;
        pL=(SdrLayerSet*)aLSets.Next();
    }
    aLSets.Clear();
}

const SdrLayerAdmin& SdrLayerAdmin::operator=(const SdrLayerAdmin& rSrcLayerAdmin)
{
    ClearLayer();
    ClearLayerSets();
    pParent=rSrcLayerAdmin.pParent;
    USHORT i;
    USHORT nAnz=rSrcLayerAdmin.GetLayerCount();
    for (i=0; i<nAnz; i++) {
        aLayer.Insert(new SdrLayer(*rSrcLayerAdmin.GetLayer(i)),CONTAINER_APPEND);
    }
    nAnz=rSrcLayerAdmin.GetLayerSetCount();
    for (i=0; i<nAnz; i++) {
        aLSets.Insert(new SdrLayerSet(*rSrcLayerAdmin.GetLayerSet(i)),CONTAINER_APPEND);
    }
    return *this;
}

FASTBOOL SdrLayerAdmin::operator==(const SdrLayerAdmin& rCmpLayerAdmin) const
{
    if (pParent!=rCmpLayerAdmin.pParent ||
        aLayer.Count()!=rCmpLayerAdmin.aLayer.Count() ||
        aLSets.Count()!=rCmpLayerAdmin.aLSets.Count()) return FALSE;
    FASTBOOL bOk=TRUE;
    USHORT nAnz=GetLayerCount();
    USHORT i=0;
    while (bOk && i<nAnz) {
        bOk=*GetLayer(i)==*rCmpLayerAdmin.GetLayer(i);
        i++;
    }
    nAnz=GetLayerSetCount();
    i=0;
    while (bOk && i<nAnz) {
        bOk=*GetLayerSet(i)==*rCmpLayerAdmin.GetLayerSet(i);
        i++;
    }
    return bOk;
}

void SdrLayerAdmin::SetModel(SdrModel* pNewModel)
{
    if (pNewModel!=pModel) {
        pModel=pNewModel;
        USHORT nAnz=GetLayerCount();
        USHORT i;
        for (i=0; i<nAnz; i++) {
            GetLayer(i)->SetModel(pNewModel);
        }
        nAnz=GetLayerSetCount();
        for (i=0; i<nAnz; i++) {
            GetLayerSet(i)->SetModel(pNewModel);
        }
    }
}

void SdrLayerAdmin::Broadcast(FASTBOOL bLayerSet) const
{
    if (pModel!=NULL) {
        SdrHint aHint(bLayerSet ? HINT_LAYERSETORDERCHG : HINT_LAYERORDERCHG);
        pModel->Broadcast(aHint);
        pModel->SetChanged();
    }
}

SdrLayer* SdrLayerAdmin::RemoveLayer(USHORT nPos)
{
    SdrLayer* pRetLayer=(SdrLayer*)(aLayer.Remove(nPos));
    Broadcast(FALSE);
    return pRetLayer;
}

SdrLayer* SdrLayerAdmin::NewLayer(const XubString& rName, USHORT nPos)
{
    SdrLayerID nID=GetUniqueLayerID();
    SdrLayer* pLay=new SdrLayer(nID,rName);
    pLay->SetModel(pModel);
    aLayer.Insert(pLay,nPos);
    Broadcast(FALSE);
    return pLay;
}

SdrLayer* SdrLayerAdmin::NewStandardLayer(USHORT nPos)
{
    SdrLayerID nID=GetUniqueLayerID();
    SdrLayer* pLay=new SdrLayer(nID,String());
    pLay->SetStandardLayer();
    pLay->SetModel(pModel);
    aLayer.Insert(pLay,nPos);
    Broadcast(FALSE);
    return pLay;
}

SdrLayer* SdrLayerAdmin::MoveLayer(USHORT nPos, USHORT nNewPos)
{
    SdrLayer* pLayer=(SdrLayer*)(aLayer.Remove(nPos));
    if (pLayer!=NULL) {
        aLayer.Insert(pLayer,nNewPos);
    }
    Broadcast(FALSE);
    return pLayer;
}

void SdrLayerAdmin::MoveLayer(SdrLayer* pLayer, USHORT nNewPos)
{
    ULONG nPos=aLayer.GetPos(pLayer);
    if (nPos!=CONTAINER_ENTRY_NOTFOUND) {
        aLayer.Remove(nPos);
        aLayer.Insert(pLayer,nNewPos);
        Broadcast(FALSE);
    }
}

USHORT SdrLayerAdmin::GetLayerPos(SdrLayer* pLayer) const
{
    ULONG nRet=SDRLAYER_NOTFOUND;
    if (pLayer!=NULL) {
        nRet=aLayer.GetPos(pLayer);
        if (nRet==CONTAINER_ENTRY_NOTFOUND) {
            nRet=SDRLAYER_NOTFOUND;
        }
    }
    return USHORT(nRet);
}

const SdrLayer* SdrLayerAdmin::GetLayer(const XubString& rName, FASTBOOL bInherited) const
{
    UINT16 i(0);
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
        pLay = pParent->GetLayer(rName, TRUE);
    }

    return pLay;
}

SdrLayerID SdrLayerAdmin::GetLayerID(const XubString& rName, FASTBOOL bInherited) const
{
    SdrLayerID nRet=SDRLAYER_NOTFOUND;
    const SdrLayer* pLay=GetLayer(rName,bInherited);
    if (pLay!=NULL) nRet=pLay->GetID();
    return nRet;
}

const SdrLayer* SdrLayerAdmin::GetLayerPerID(USHORT nID) const
{
    USHORT i=0;
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
    FASTBOOL bDown=pParent==NULL;
    USHORT j;
    for (j=0; j<GetLayerCount(); j++) {
        aSet.Set(GetLayer(j)->GetID());
    }
    short i;
    if (!bDown) {
        i=254;
        while (i>0 && aSet.IsSet(BYTE(i))) i++;
        if (i<0) i=254;
    } else {
        i=0;
        while (i<=254 && aSet.IsSet(BYTE(i))) i++;
        if (i>254) i=0;
    }
    return SdrLayerID(i);
}

SdrLayerSet* SdrLayerAdmin::NewLayerSet(const XubString& rName, USHORT nPos)
{
    SdrLayerSet* pSet=new SdrLayerSet(/**this,*/rName);
    pSet->SetModel(pModel);
    aLSets.Insert(pSet,nPos);
    Broadcast(TRUE);
    return pSet;
}

SdrLayerSet* SdrLayerAdmin::RemoveLayerSet(SdrLayerSet* pSet)
{
    SdrLayerSet* pRetSet=(SdrLayerSet*)aLSets.Remove(pSet);
    Broadcast(TRUE);
    return pRetSet;
}

void SdrLayerAdmin::MoveLayerSet(SdrLayerSet* pSet, USHORT nNewPos)
{
    ULONG nPos=aLSets.GetPos(pSet);
    if (nPos!=CONTAINER_ENTRY_NOTFOUND) {
        aLSets.Remove(nPos);
        aLSets.Insert(pSet,nNewPos);
        Broadcast(TRUE);
    }
}

const SdrLayerSet* SdrLayerAdmin::GetLayerSet(const XubString& rName, FASTBOOL bInherited) const
{
    UINT16 i(0);
    const SdrLayerSet* pSet = NULL;

    while(i <= GetLayerSetCount() && !pSet)
    {
        if(rName.Equals(GetLayerSet(i)->GetName()))
            pSet = GetLayerSet(i);
        else
            i++;
    }

    if(!pSet && pParent)
    {
        pSet = pParent->GetLayerSet(rName, TRUE);
    }

    return pSet;
}

