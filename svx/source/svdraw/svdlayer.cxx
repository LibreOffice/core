/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdlayer.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 05:52:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#include "svdlayer.hxx"
#include "svdmodel.hxx" // fuer Broadcasting
#include "svdglob.hxx"  // StringCache
#include "svdstr.hrc"   // Namen aus der Resource

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
            aData[nIndex] = static_cast<BYTE>(aSeq[nIndex]);
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

//BFS01SvStream& operator>>(SvStream& rIn, SdrLayer& rLayer)
//BFS01{
//BFS01 if(rIn.GetError())
//BFS01     return rIn;
//BFS01
//BFS01 SdrIOHeader aHead(rIn, STREAM_READ);
//BFS01
//BFS01 rIn >> rLayer.nID;
//BFS01
//BFS01 // UNICODE: rIn >> rLayer.aName;
//BFS01 rIn.ReadByteString(rLayer.aName);
//BFS01
//BFS01 if(aHead.GetVersion() >= 1)
//BFS01 {
//BFS01     // Das Standardlayerflag kam direkt nach der Betalieferung dazu
//BFS01     rIn >> rLayer.nType;
//BFS01
//BFS01     if(rLayer.nType == 1)
//BFS01     {
//BFS01         rLayer.aName = ImpGetResStr(STR_StandardLayerName);
//BFS01     }
//BFS01 }
//BFS01
//BFS01 if(aHead.GetVersion() <= 12)
//BFS01 {
//BFS01     // nType war lange Zeit nicht initiallisiert!
//BFS01     if(rLayer.nType > 1)
//BFS01         rLayer.nType = 0;
//BFS01 }
//BFS01
//BFS01 return rIn;
//BFS01}

//BFS01SvStream& operator<<(SvStream& rOut, const SdrLayer& rLayer)
//BFS01{
//BFS01 SdrIOHeader aHead(rOut, STREAM_WRITE, SdrIOLayrID);
//BFS01
//BFS01 rOut << rLayer.nID;
//BFS01
//BFS01 // UNICODE: rOut << rLayer.aName;
//BFS01 rOut.WriteByteString(rLayer.aName);
//BFS01
//BFS01 rOut << rLayer.nType;
//BFS01
//BFS01 return rOut;
//BFS01}

////////////////////////////////////////////////////////////////////////////////////////////////////
// SdrLayerSet
////////////////////////////////////////////////////////////////////////////////////////////////////

//#110094#-10
//FASTBOOL SdrLayerSet::operator==(const SdrLayerSet& rCmpLayerSet) const
//{
//  return(aName.Equals(rCmpLayerSet.aName)
//      && aMember == rCmpLayerSet.aMember
//      && aExclude == rCmpLayerSet.aExclude);
//}

//#110094#-10
//void SdrLayerSet::Broadcast() const
//{
//  if (pModel!=NULL) {
//      SdrHint aHint(HINT_LAYERSETCHG);
//      pModel->Broadcast(aHint);
//      pModel->SetChanged();
//  }
//}

//#110094#-10
//SvStream& operator>>(SvStream& rIn, SdrLayerSet& rSet)
//{
//  if(rIn.GetError())
//      return rIn;
//
//  SdrIOHeader aHead(rIn, STREAM_READ);
//
//  rIn >> rSet.aMember;
//  rIn >> rSet.aExclude;
//
//  // UNICODE: rIn >> rSet.aName;
//  rIn.ReadByteString(rSet.aName);
//
//  return rIn;
//}

//#110094#-10
//SvStream& operator<<(SvStream& rOut, const SdrLayerSet& rSet)
//{
//  SdrIOHeader aHead(rOut, STREAM_WRITE, SdrIOLSetID);
//
//  rOut << rSet.aMember;
//  rOut << rSet.aExclude;
//
//  // UNICODE: rOut << rSet.aName;
//  rOut.WriteByteString(rSet.aName);
//
//  return rOut;
//}

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
    //#110094#-10
    //ClearLayerSets();
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

//#110094#-10
//void SdrLayerAdmin::ClearLayerSets()
//{
//  SdrLayerSet* pL;
//  pL=(SdrLayerSet*)aLSets.First();
//  while (pL!=NULL) {
//      delete pL;
//      pL=(SdrLayerSet*)aLSets.Next();
//  }
//  aLSets.Clear();
//}

const SdrLayerAdmin& SdrLayerAdmin::operator=(const SdrLayerAdmin& rSrcLayerAdmin)
{
    ClearLayer();
    //#110094#-10
    //ClearLayerSets();
    pParent=rSrcLayerAdmin.pParent;
    USHORT i;
    USHORT nAnz=rSrcLayerAdmin.GetLayerCount();
    for (i=0; i<nAnz; i++) {
        aLayer.Insert(new SdrLayer(*rSrcLayerAdmin.GetLayer(i)),CONTAINER_APPEND);
    }
//#110094#-10
//  nAnz=rSrcLayerAdmin.GetLayerSetCount();
//  for (i=0; i<nAnz; i++) {
//      aLSets.Insert(new SdrLayerSet(*rSrcLayerAdmin.GetLayerSet(i)),CONTAINER_APPEND);
//  }
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
    //#110094#-10
    //nAnz=GetLayerSetCount();
    //i=0;
    //while (bOk && i<nAnz) {
    //  bOk=*GetLayerSet(i)==*rCmpLayerAdmin.GetLayerSet(i);
    //  i++;
    //}
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
        //#110094#-10
        //nAnz=GetLayerSetCount();
        //for (i=0; i<nAnz; i++) {
        //  GetLayerSet(i)->SetModel(pNewModel);
        //}
    }
}

//#110094#-10
//void SdrLayerAdmin::Broadcast(FASTBOOL bLayerSet) const
void SdrLayerAdmin::Broadcast() const
{
    if (pModel!=NULL) {
        //#110094#-10
        //SdrHint aHint(bLayerSet ? HINT_LAYERSETORDERCHG : HINT_LAYERORDERCHG);
        SdrHint aHint(HINT_LAYERORDERCHG);
        pModel->Broadcast(aHint);
        pModel->SetChanged();
    }
}

SdrLayer* SdrLayerAdmin::RemoveLayer(USHORT nPos)
{
    SdrLayer* pRetLayer=(SdrLayer*)(aLayer.Remove(nPos));

    //#110094#-10
    Broadcast();
    //Broadcast(FALSE);

    return pRetLayer;
}

SdrLayer* SdrLayerAdmin::NewLayer(const XubString& rName, USHORT nPos)
{
    SdrLayerID nID=GetUniqueLayerID();
    SdrLayer* pLay=new SdrLayer(nID,rName);
    pLay->SetModel(pModel);
    aLayer.Insert(pLay,nPos);

    //#110094#-10
    //Broadcast(FALSE);
    Broadcast();

    return pLay;
}

SdrLayer* SdrLayerAdmin::NewStandardLayer(USHORT nPos)
{
    SdrLayerID nID=GetUniqueLayerID();
    SdrLayer* pLay=new SdrLayer(nID,String());
    pLay->SetStandardLayer();
    pLay->SetModel(pModel);
    aLayer.Insert(pLay,nPos);

    //#110094#-10
    //Broadcast(FALSE);
    Broadcast();

    return pLay;
}

SdrLayer* SdrLayerAdmin::MoveLayer(USHORT nPos, USHORT nNewPos)
{
    SdrLayer* pLayer=(SdrLayer*)(aLayer.Remove(nPos));
    if (pLayer!=NULL) {
        aLayer.Insert(pLayer,nNewPos);
    }

    //#110094#-10
    //Broadcast(FALSE);
    Broadcast();

    return pLayer;
}

void SdrLayerAdmin::MoveLayer(SdrLayer* pLayer, USHORT nNewPos)
{
    ULONG nPos=aLayer.GetPos(pLayer);
    if (nPos!=CONTAINER_ENTRY_NOTFOUND) {
        aLayer.Remove(nPos);
        aLayer.Insert(pLayer,nNewPos);

        //#110094#-10
        //Broadcast(FALSE);
        Broadcast();
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

const SdrLayer* SdrLayerAdmin::GetLayer(const XubString& rName, FASTBOOL /*bInherited*/) const
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
    sal_Bool bDown = (pParent == NULL);
    sal_Int32 j;
    for (j=0; j<GetLayerCount(); j++)
    {
        aSet.Set(GetLayer(j)->GetID());
    }
    SdrLayerID i;
    if (!bDown)
    {
        i=254;
        while (i && aSet.IsSet(BYTE(i)))
            --i;
        if (i == 0)
            i=254;
    }
    else
    {
        i=0;
        while (i<=254 && aSet.IsSet(BYTE(i)))
            i++;
        if (i>254)
            i=0;
    }
    return i;
}

//#110094#-10
//SdrLayerSet* SdrLayerAdmin::NewLayerSet(const XubString& rName, USHORT nPos)
//{
//  SdrLayerSet* pSet=new SdrLayerSet(/**this,*/rName);
//  pSet->SetModel(pModel);
//  aLSets.Insert(pSet,nPos);
//  Broadcast(TRUE);
//  return pSet;
//}

//#110094#-10
//SdrLayerSet* SdrLayerAdmin::RemoveLayerSet(SdrLayerSet* pSet)
//{
//  SdrLayerSet* pRetSet=(SdrLayerSet*)aLSets.Remove(pSet);
//  Broadcast(TRUE);
//  return pRetSet;
//}

//#110094#-10
//void SdrLayerAdmin::MoveLayerSet(SdrLayerSet* pSet, USHORT nNewPos)
//{
//  ULONG nPos=aLSets.GetPos(pSet);
//  if (nPos!=CONTAINER_ENTRY_NOTFOUND) {
//      aLSets.Remove(nPos);
//      aLSets.Insert(pSet,nNewPos);
//      Broadcast(TRUE);
//  }
//}

//#110094#-10
//const SdrLayerSet* SdrLayerAdmin::GetLayerSet(const XubString& rName, FASTBOOL bInherited) const
//{
//  UINT16 i(0);
//  const SdrLayerSet* pSet = NULL;
//
//  while(i <= GetLayerSetCount() && !pSet)
//  {
//      if(rName.Equals(GetLayerSet(i)->GetName()))
//          pSet = GetLayerSet(i);
//      else
//          i++;
//  }
//
//  if(!pSet && pParent)
//  {
//      pSet = pParent->GetLayerSet(rName, TRUE);
//  }
//
//  return pSet;
//}

