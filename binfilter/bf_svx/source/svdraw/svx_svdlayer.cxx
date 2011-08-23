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


#include "svdlayer.hxx"
#include "svdio.hxx"
#include "svdmodel.hxx" // fuer Broadcasting
#include "svdstr.hrc"   // Namen aus der Resource
namespace binfilter {

////////////////////////////////////////////////////////////////////////////////////////////////////
// SetOfByte
////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ FASTBOOL SetOfByte::IsEmpty() const
/*N*/ {
/*N*/ 	for (USHORT i=0; i<32; i++) {
/*N*/ 		if (aData[i]!=0) return FALSE;
/*N*/ 	}
/*N*/ 	return TRUE;
/*N*/ }






/*N*/ void SetOfByte::operator&=(const SetOfByte& r2ndSet)
/*N*/ {
/*N*/ 	for (unsigned i=0; i<32; i++) {
/*N*/ 		aData[i]&=r2ndSet.aData[i];
/*N*/ 	}
/*N*/ }


/** initialize this set with a uno sequence of sal_Int8
*/
/*N*/  void SetOfByte::PutValue( const ::com::sun::star::uno::Any & rAny )
/*N*/  {
/*N*/  	::com::sun::star::uno::Sequence< sal_Int8 > aSeq;
/*N*/  	if( rAny >>= aSeq )
/*N*/  	{
/*N*/  		sal_Int16 nCount = (sal_Int16)aSeq.getLength();
/*N*/  		if( nCount > 32 )
/*N*/  			nCount = 32;
/*N*/  
/*N*/  		sal_Int16 nIndex;
/*N*/  		for( nIndex = 0; nIndex < nCount; nIndex++ )
/*N*/  		{
/*N*/  			aData[nIndex] = static_cast<BYTE>(aSeq[nIndex]);
/*N*/  		}
/*N*/  
/*N*/  		for( ; nIndex < 32; nIndex++ )
/*N*/  		{
/*N*/  			aData[nIndex] = 0;
/*N*/  		}
/*N*/  	}
/*N*/  }

/** returns a uno sequence of sal_Int8
*/
/*N*/ void SetOfByte::QueryValue( ::com::sun::star::uno::Any & rAny ) const
/*N*/ {
/*N*/ 	sal_Int16 nNumBytesSet = 0;
/*N*/ 	sal_Int16 nIndex;
/*N*/ 	for( nIndex = 31; nIndex >= 00; nIndex-- )
/*N*/ 	{
/*N*/ 		if( 0 != aData[nIndex] )
/*N*/ 		{
/*N*/ 			nNumBytesSet = nIndex + 1;
/*N*/ 			break;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	::com::sun::star::uno::Sequence< sal_Int8 > aSeq( nNumBytesSet );
/*N*/ 
/*N*/ 	for( nIndex = 0; nIndex < nNumBytesSet; nIndex++ )
/*N*/ 	{
/*N*/ 		aSeq[nIndex] = static_cast<sal_Int8>(aData[nIndex]);
/*N*/ 	}
/*N*/ 
/*N*/ 	rAny <<= aSeq;
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
// SdrLayer
////////////////////////////////////////////////////////////////////////////////////////////////////


/*N*/ void SdrLayer::SetName(const XubString& rNewName)
/*N*/ {
/*N*/ 	if(!rNewName.Equals(aName)) 
/*N*/ 	{
/*N*/ 		aName = rNewName;
/*N*/ 		nType = 0; // Userdefined
/*N*/ 		
/*N*/ 		if(pModel) 
/*N*/ 		{
/*N*/ 			SdrHint aHint(HINT_LAYERCHG);
/*N*/ 
/*N*/ 			pModel->Broadcast(aHint);
/*N*/ 			pModel->SetChanged();
/*N*/ 		}
/*N*/ 	}
/*N*/ }


/*N*/ SvStream& operator>>(SvStream& rIn, SdrLayer& rLayer)
/*N*/ {
/*N*/ 	if(rIn.GetError()) 
/*?*/ 		return rIn;
/*N*/ 
/*N*/ 	SdrIOHeader aHead(rIn, STREAM_READ);
/*N*/ 
/*N*/ 	rIn >> rLayer.nID;
/*N*/ 	
/*N*/ 	// UNICODE: rIn >> rLayer.aName;
/*N*/ 	rIn.ReadByteString(rLayer.aName);
/*N*/ 	
/*N*/ 	if(aHead.GetVersion() >= 1) 
/*N*/ 	{
/*N*/ 		// Das Standardlayerflag kam direkt nach der Betalieferung dazu
/*N*/ 		rIn >> rLayer.nType; 
/*N*/ 
/*N*/ 		if(rLayer.nType == 1) 
/*N*/ 		{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if(aHead.GetVersion() <= 12) 
/*N*/ 	{
/*N*/ 		// nType war lange Zeit nicht initiallisiert!
/*N*/ 		if(rLayer.nType > 1) 
/*N*/ 			rLayer.nType = 0;
/*N*/ 	}
/*N*/ 
/*N*/ 	return rIn;
/*N*/ }

/*N*/ SvStream& operator<<(SvStream& rOut, const SdrLayer& rLayer)
/*N*/ {
/*N*/ 	SdrIOHeader aHead(rOut, STREAM_WRITE, SdrIOLayrID);
/*N*/ 
/*N*/ 	rOut << rLayer.nID;
/*N*/ 	
/*N*/ 	// UNICODE: rOut << rLayer.aName;
/*N*/ 	rOut.WriteByteString(rLayer.aName);
/*N*/ 	
/*N*/ 	rOut << rLayer.nType;
/*N*/ 
/*N*/ 	return rOut;
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
// SdrLayerSet
////////////////////////////////////////////////////////////////////////////////////////////////////



/*?*/ SvStream& operator>>(SvStream& rIn, SdrLayerSet& rSet)
/*?*/ {
/*?*/ 	if(rIn.GetError()) 
/*?*/ 		return rIn;
/*?*/ 
/*?*/ 	SdrIOHeader aHead(rIn, STREAM_READ);
/*?*/ 	
/*?*/ 	rIn >> rSet.aMember;
/*?*/ 	rIn >> rSet.aExclude;
/*?*/ 
/*?*/ 	// UNICODE: rIn >> rSet.aName;
/*?*/ 	rIn.ReadByteString(rSet.aName);
/*?*/ 
/*?*/ 	return rIn;
/*?*/ }

/*?*/ SvStream& operator<<(SvStream& rOut, const SdrLayerSet& rSet)
/*?*/ {
/*?*/ 	SdrIOHeader aHead(rOut, STREAM_WRITE, SdrIOLSetID);
/*?*/ 
/*?*/ 	rOut << rSet.aMember;
/*?*/ 	rOut << rSet.aExclude;
/*?*/ 	
/*?*/ 	// UNICODE: rOut << rSet.aName;
/*?*/ 	rOut.WriteByteString(rSet.aName);
/*?*/ 	
/*?*/ 	return rOut;
/*?*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
// SdrLayerAdmin
////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ SdrLayerAdmin::SdrLayerAdmin(SdrLayerAdmin* pNewParent):
/*N*/ 	aLayer(1024,16,16),
/*N*/ 	aLSets(1024,16,16),
/*N*/ 	pModel(NULL)
/*N*/ {
/*N*/ 	sal_Char aTextControls[] = "Controls";
/*N*/ 	aControlLayerName = String(aTextControls, sizeof(aTextControls-1));
/*N*/ 	pParent=pNewParent;
/*N*/ }

/*?*/ SdrLayerAdmin::SdrLayerAdmin(const SdrLayerAdmin& rSrcLayerAdmin):
/*?*/ 	aLayer(1024,16,16),
/*?*/ 	aLSets(1024,16,16),
/*?*/ 	pParent(NULL),
/*?*/ 	pModel(NULL)
/*?*/ {
/*?*/ 	sal_Char aTextControls[] = "Controls";
/*?*/ 	aControlLayerName = String(aTextControls, sizeof(aTextControls-1));
/*?*/ 	*this = rSrcLayerAdmin;
/*?*/ }

/*N*/ SdrLayerAdmin::~SdrLayerAdmin()
/*N*/ {
/*N*/ 	ClearLayer();
/*N*/ 	ClearLayerSets();
/*N*/ }

/*N*/ void SdrLayerAdmin::ClearLayer()
/*N*/ {
/*N*/ 	SdrLayer* pL;
/*N*/ 	pL=(SdrLayer*)aLayer.First();
/*N*/ 	while (pL!=NULL) {
/*N*/ 		delete pL;
/*N*/ 		pL=(SdrLayer*)aLayer.Next();
/*N*/ 	}
/*N*/ 	aLayer.Clear();
/*N*/ }

/*N*/ void SdrLayerAdmin::ClearLayerSets()
/*N*/ {
/*N*/ 	SdrLayerSet* pL;
/*N*/ 	pL=(SdrLayerSet*)aLSets.First();
/*N*/ 	while (pL!=NULL) {
/*?*/ 		delete pL;
/*?*/ 		pL=(SdrLayerSet*)aLSets.Next();
/*N*/ 	}
/*N*/ 	aLSets.Clear();
/*N*/ }



/*N*/ void SdrLayerAdmin::SetModel(SdrModel* pNewModel)
/*N*/ {
/*N*/ 	if (pNewModel!=pModel) {
/*N*/ 		pModel=pNewModel;
/*N*/ 		USHORT nAnz=GetLayerCount();
/*N*/ 		USHORT i;
/*N*/ 		for (i=0; i<nAnz; i++) {
/*?*/ 			GetLayer(i)->SetModel(pNewModel);
/*N*/ 		}
/*N*/ 		nAnz=GetLayerSetCount();
/*N*/ 		for (i=0; i<nAnz; i++) {
/*?*/ 			GetLayerSet(i)->SetModel(pNewModel);
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void SdrLayerAdmin::Broadcast(FASTBOOL bLayerSet) const
/*N*/ {
/*N*/ 	if (pModel!=NULL) {
/*N*/ 		SdrHint aHint(bLayerSet ? HINT_LAYERSETORDERCHG : HINT_LAYERORDERCHG);
/*N*/ 		pModel->Broadcast(aHint);
/*N*/ 		pModel->SetChanged();
/*N*/ 	}
/*N*/ }

/*N*/ SdrLayer* SdrLayerAdmin::NewLayer(const XubString& rName, USHORT nPos)
/*N*/ {
/*N*/ 	SdrLayerID nID=GetUniqueLayerID();
/*N*/ 	SdrLayer* pLay=new SdrLayer(nID,rName);
/*N*/ 	pLay->SetModel(pModel);
/*N*/ 	aLayer.Insert(pLay,nPos);
/*N*/ 	Broadcast(FALSE);
/*N*/ 	return pLay;
/*N*/ }





/*N*/ const SdrLayer* SdrLayerAdmin::GetLayer(const XubString& rName, FASTBOOL bInherited) const
/*N*/ {
/*N*/ 	UINT16 i(0);
/*N*/ 	const SdrLayer* pLay = NULL;
/*N*/ 
/*N*/ 	while(i < GetLayerCount() && !pLay) 
/*N*/ 	{
/*N*/ 		if(rName.Equals(GetLayer(i)->GetName()))
/*N*/ 			pLay = GetLayer(i);
/*N*/ 		else 
/*N*/ 			i++;
/*N*/ 	}
/*N*/ 
/*N*/ 	if(!pLay && pParent) 
/*N*/ 	{
/*N*/ 		pLay = pParent->GetLayer(rName, TRUE);
/*N*/ 	}
/*N*/ 
/*N*/ 	return pLay;
/*N*/ }

/*N*/ SdrLayerID SdrLayerAdmin::GetLayerID(const XubString& rName, FASTBOOL bInherited) const
/*N*/ {
/*N*/ 	SdrLayerID nRet=SDRLAYER_NOTFOUND;
/*N*/ 	const SdrLayer* pLay=GetLayer(rName,bInherited);
/*N*/ 	if (pLay!=NULL) nRet=pLay->GetID();
/*N*/ 	return nRet;
/*N*/ }

/*N*/ const SdrLayer* SdrLayerAdmin::GetLayerPerID(USHORT nID) const
/*N*/ {
/*N*/ 	USHORT i=0;
/*N*/ 	const SdrLayer* pLay=NULL;
/*N*/ 	while (i<GetLayerCount() && pLay==NULL) {
/*N*/ 		if (nID==GetLayer(i)->GetID()) pLay=GetLayer(i);
/*N*/ 		else i++;
/*N*/ 	}
/*N*/ 	return pLay;
/*N*/ }

// Globale LayerID's beginnen mit 0 aufsteigend.
// Lokale LayerID's beginnen mit 254 absteigend.
// 255 ist reserviert fuer SDRLAYER_NOTFOUND

/*N*/ SdrLayerID SdrLayerAdmin::GetUniqueLayerID() const
/*N*/ {
/*N*/ 	SetOfByte aSet;
/*N*/ 	sal_Bool bDown = (pParent == NULL);
/*N*/ 	sal_Int32 j;
/*N*/ 	for (j=0; j<GetLayerCount(); j++) 
/*N*/     {
/*N*/ 		aSet.Set(GetLayer(j)->GetID());
/*N*/ 	}
/*N*/ 	SdrLayerID i;
/*N*/ 	if (!bDown) 
/*N*/     {
/*N*/ 		i=254;
/*N*/ 		while (i && aSet.IsSet(BYTE(i))) 
/*N*/             --i;
/*N*/ 		if (i == 0) 
/*N*/             i=254;
/*N*/ 	} 
/*N*/     else 
/*N*/     {
/*N*/ 		i=0;
/*N*/ 		while (i<=254 && aSet.IsSet(BYTE(i))) 
/*N*/             i++;
/*N*/ 		if (i>254) 
/*N*/             i=0;
/*N*/ 	}
/*N*/ 	return i;
/*N*/ }





}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
