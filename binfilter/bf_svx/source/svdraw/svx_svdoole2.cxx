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

#ifndef _COM_SUN_STAR_UTIL_XMODIFYBROADCASTER_HPP_
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#endif


#ifndef _SFX_INTERNO_HXX
#include <bf_sfx2/interno.hxx>
#endif
#ifndef _SO_CLSIDS_HXX
#include <comphelper/classids.hxx>
#endif




#include "svdoole2.hxx"
#include <bf_svtools/solar.hrc>

#include "svdio.hxx"
#include "svdstr.hrc"   // Objektname
#include "svdetc.hxx"
#include "svdview.hxx"
#include "unomlstr.hxx"
#ifndef _CLIENT_HXX
#include <bf_so3/client.hxx>
#endif

namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef SO2_DECL_SVINPLACEOBJECT_DEFINED
#define SO2_DECL_SVINPLACEOBJECT_DEFINED
SO2_DECL_REF(SvInPlaceObject)
#endif

/*N*/ class SdrOle2ObjImpl
/*N*/ {
/*N*/ public:
/*N*/ 	GDIMetaFile*	pMetaFile;          // Metafile fuer GetMtf kopieren und merken
/*N*/ 	BfGraphicObject*	pGraphicObject;
/*N*/ 	String          aPersistName;       // name of object in persist
/*N*/ 
/*N*/ 	// #107645#
/*N*/ 	// New local var to avoid repeated loading if load of OLE2 fails
/*N*/ 	sal_Bool		mbLoadingOLEObjectFailed;
/*N*/ 
/*N*/ 	bool	mbConnected;
/*N*/ 
/*N*/ };

////////////////////////////////////////////////////////////////////////////////////////////////////

// Predicate determining whether the given OLE is an internal math
// object
/*N*/ static bool ImplIsMathObj( const SvInPlaceObjectRef& rObjRef )
/*N*/ {
/*N*/     if( !rObjRef.Is() )
/*N*/         return false;
/*N*/ 
/*N*/     SvGlobalName aClassName( rObjRef->GetClassName() );
/*N*/ 
/*N*/     if( aClassName == SvGlobalName(BF_SO3_SM_CLASSID_30) || 
/*N*/         aClassName == SvGlobalName(BF_SO3_SM_CLASSID_40) || 
/*N*/         aClassName == SvGlobalName(BF_SO3_SM_CLASSID_50) || 
/*N*/         aClassName == SvGlobalName(BF_SO3_SM_CLASSID_60) || 
/*N*/         aClassName == SvGlobalName(BF_SO3_SM_CLASSID) 		)
/*N*/     {
/*N*/         return true;
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         return false;
/*N*/     }
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ TYPEINIT1(SdrOle2Obj,SdrRectObj);
/*N*/ 
/*N*/ SdrOle2Obj::SdrOle2Obj(FASTBOOL bFrame_)
/*N*/ {
/*N*/ 	bInDestruction = FALSE;
/*N*/ 	Init();
/*N*/ 
/*N*/ 	ppObjRef=new SvInPlaceObjectRef;
/*N*/ 	bFrame=bFrame_;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ SdrOle2Obj::SdrOle2Obj(const SvInPlaceObjectRef& rNewObjRef, const XubString& rNewObjName, const Rectangle& rNewRect, FASTBOOL bFrame_):
/*N*/ 	SdrRectObj(rNewRect)
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdrOle2Obj::Init()
/*N*/ {
/*N*/ 	mpImpl = new SdrOle2ObjImpl;
/*N*/ 	pModifyListener = NULL;
/*N*/ 	pGraphic=NULL;
/*N*/ 	mpImpl->pMetaFile=NULL;
/*N*/ 	mpImpl->pGraphicObject=NULL;
/*N*/ 
/*N*/ 	// #107645#
/*N*/ 	// init to start situation, loading did not fail
/*N*/ 	mpImpl->mbLoadingOLEObjectFailed = sal_False;
/*N*/ 
/*N*/ 	mpImpl->mbConnected = false;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ SdrOle2Obj::~SdrOle2Obj()
/*N*/ {
/*N*/ 	bInDestruction = TRUE;
/*N*/ 	// Aus Cache entfernen
/*N*/ 	GetSdrGlobalData().GetOLEObjCache().RemoveObj(this);
/*N*/ 
/*N*/ 	SvInPlaceObjectRef aRef( *ppObjRef );
/*N*/ 
/*N*/ 	Disconnect();
/*N*/ 
/*N*/ 	if(pModel && aRef.Is())
/*N*/ 	{
/*N*/ 		SvPersist* pPers=pModel->GetPersist();
/*N*/ 		if(pPers!=NULL)
/*N*/ 		{
/*?*/ 			pPers->Remove(aRef);
/*N*/ 		}
/*N*/ 	}
/*N*/ 	aRef.Clear();
/*N*/ 
/*N*/ 	delete ppObjRef;
/*N*/ 
/*N*/ 	if(pGraphic!=NULL)
/*?*/ 		delete pGraphic;
/*N*/ 
/*N*/ 	if(mpImpl->pMetaFile!=NULL)
/*?*/ 		delete mpImpl->pMetaFile;
/*N*/ 
/*N*/ 	if(mpImpl->pGraphicObject!=NULL)
/*?*/ 		delete mpImpl->pGraphicObject;
/*N*/ 
/*N*/ 	if(pModifyListener)
/*N*/ 	{
/*N*/ 		pModifyListener->invalidate();
/*N*/ 		pModifyListener->release();
/*N*/ 		pModifyListener = NULL;
/*N*/ 	}
/*N*/ 	delete mpImpl;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdrOle2Obj::SetGraphic(const Graphic* pGrf)
/*N*/ {
/*N*/ 	if ( pGraphic )
/*N*/ 	{
/*?*/ 		delete pGraphic;
/*?*/ 		pGraphic = NULL;
/*?*/ 		delete mpImpl->pGraphicObject;
/*?*/ 		mpImpl->pGraphicObject = NULL;
/*N*/ 	}
/*N*/ 
/*N*/ 	if (pGrf!=NULL)
/*N*/ 	{
/*?*/ 		pGraphic = new Graphic(*pGrf);
/*?*/ 		mpImpl->pGraphicObject = new BfGraphicObject( *pGraphic );
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( ppObjRef->Is() && pGrf )
/*?*/ 		SendRepaintBroadcast();
/*N*/ 
/*N*/ 	SetChanged();
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ FASTBOOL SdrOle2Obj::IsEmpty() const
/*N*/ {
/*N*/ 	return !ppObjRef->Is();
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdrOle2Obj::Connect()
/*N*/ {
/*N*/ 	if(pModel && mpImpl->aPersistName.Len())
/*N*/ 	{
/*N*/ 		SvPersist* pPers=pModel->GetPersist();
/*N*/ 		if (pPers!=NULL)
/*N*/ 		{
/*N*/ 			SvInfoObjectRef xIObj;
/*N*/ 			SvInfoObject* pInfo = pPers->Find(mpImpl->aPersistName);
/*N*/ 
/*N*/ 			if (!pInfo)
/*N*/ 			{
/*?*/ 				if ( !ppObjRef->Is() )
/*?*/ 					GetObjRef();	// try to load inplace object
/*?*/ 
/*?*/ 				xIObj = pInfo = new SvEmbeddedInfoObject(*ppObjRef,mpImpl->aPersistName);
/*N*/ 			}
/*N*/ 
/*N*/ 			if ( !pPers->HasObject(mpImpl->aPersistName) )
/*N*/ 			{
/*?*/ 				pPers->Move(pInfo, mpImpl->aPersistName);
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				pInfo->SetDeleted(FALSE);
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		// In Cache einfuegen
/*N*/ 		GetSdrGlobalData().GetOLEObjCache().InsertObj(this);
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( ppObjRef->Is() && (*ppObjRef)->GetMiscStatus() & SVOBJ_MISCSTATUS_RESIZEONPRINTERCHANGE )
/*N*/ 	{
/*N*/ 		if (pModel && pModel->GetRefDevice() &&
/*N*/ 			pModel->GetRefDevice()->GetOutDevType() == OUTDEV_PRINTER)
/*N*/ 		{
/*?*/ 			// Kein RefDevice oder RefDevice kein Printer
/*?*/ 			BOOL bModified = (*ppObjRef)->IsModified();
/*?*/ 			Printer* pPrinter = (Printer*) pModel->GetRefDevice();
/*?*/ 			(*ppObjRef)->OnDocumentPrinterChanged( pPrinter );
/*?*/ 			(*ppObjRef)->SetModified( bModified );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if( !IsEmpty() )
/*N*/ 	{
/*?*/ 		// register modify listener
/*?*/ 		if( pModifyListener == NULL )
/*?*/ 		{
/*?*/ 			((SdrOle2Obj*)this)->pModifyListener = new SvxUnoShapeModifyListener( (SdrOle2Obj*)this );
/*?*/ 			pModifyListener->acquire();
/*?*/ 		}
/*?*/ 
/*?*/ 		uno::Reference< util::XModifyBroadcaster > xBC( getXModel(), uno::UNO_QUERY );
/*?*/ 		if( xBC.is() && pModifyListener )
/*?*/ 		{
/*?*/ 			uno::Reference< util::XModifyListener > xListener( pModifyListener );
/*?*/ 			xBC->addModifyListener( xListener );
/*?*/ 		}
/*?*/ 
/*?*/ 		mpImpl->mbConnected = true;
/*N*/ 	}
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdrOle2Obj::Disconnect()
/*N*/ {
/*N*/ 	if( !mpImpl->mbConnected )
/*N*/ 		return;
/*N*/ 
/*N*/ 	if( !IsEmpty() && mpImpl->aPersistName.Len() )
/*N*/ 	{
/*N*/ 		uno::Reference< util::XModifyBroadcaster > xBC( getXModel(), uno::UNO_QUERY );
/*N*/ 		if( xBC.is() && pModifyListener )
/*N*/ 		{
/*N*/ 			uno::Reference< util::XModifyListener > xListener( pModifyListener );
/*N*/ 			xBC->removeModifyListener( xListener );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if (pModel && mpImpl->aPersistName.Len() )
/*N*/ 	{
/*N*/ 		if( pModel->IsInDestruction() )
/*N*/ 		{
/*N*/ 			*ppObjRef = NULL;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			SvPersist* pPers = pModel->GetPersist();
/*N*/ 
/*N*/ 			if (pPers)
/*N*/ 			{
/*N*/ 				SvInfoObject* pInfo = pPers->Find(mpImpl->aPersistName);
/*N*/ 
/*N*/ 				if (pInfo)
/*N*/ 				{
/*N*/ 					pInfo->SetDeleted(TRUE);
/*N*/ 					pInfo->SetObj(0);
/*N*/ 				}
/*N*/ 			}
/*N*/ 
/*N*/ 			if ( ppObjRef->Is() )
/*N*/ 				(*ppObjRef)->DoClose();
/*N*/ 		}
/*N*/ 
/*N*/ 		// Aus Cache entfernen
/*N*/ 		GetSdrGlobalData().GetOLEObjCache().RemoveObj(this);
/*N*/ 
/*N*/ 		if ( ppObjRef->Is() )
/*N*/ 			ppObjRef->Clear();
/*N*/ 	}
/*N*/ 
/*N*/ 	mpImpl->mbConnected = false;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdrOle2Obj::SetModel(SdrModel* pNewModel)
/*N*/ {
/*N*/ 	FASTBOOL bChg = pNewModel!=pModel;
/*N*/ 
/*N*/ 	if( bChg )
/*N*/         Disconnect(); // mit dem alten Namen
/*N*/ 
/*N*/ 	if( pModel && pNewModel )
/*N*/ 	{
/*?*/ 		SvPersist* pDestPers = pNewModel->GetPersist();
/*?*/ 		SvPersist* pSrcPers  = pModel->GetPersist();
/*?*/ 
/*?*/ 		if( pDestPers && pSrcPers && ( pDestPers != pSrcPers ) )
/*?*/ 		{
/*?*/				{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 ImpCopyObject( *pSrcPers, *pDestPers, mpImpl->aPersistName );
/*?*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	SdrRectObj::SetModel( pNewModel );
/*N*/ 
/*N*/ 	if( bChg )
/*N*/         Connect();
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdrOle2Obj::SetPage(SdrPage* pNewPage)
/*N*/ {
/*N*/ 	FASTBOOL bRemove=pNewPage==NULL && pPage!=NULL;
/*N*/ 	FASTBOOL bInsert=pNewPage!=NULL && pPage==NULL;
/*N*/ 
/*N*/ 	if (bRemove) Disconnect();
/*N*/ 
/*N*/ 	SdrRectObj::SetPage(pNewPage);
/*N*/ 
/*N*/ 	if (bInsert) Connect();
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdrOle2Obj::SetObjRef(const SvInPlaceObjectRef& rNewObjRef)
/*N*/ {
/*N*/ 	if( rNewObjRef == *ppObjRef )
/*N*/ 		return;
/*N*/ 
/*N*/ 	Disconnect();
/*N*/ 
/*N*/ 	*ppObjRef=rNewObjRef;
/*N*/ 
/*N*/ 	SvInPlaceObjectRef& rIPRef = *ppObjRef;
/*N*/ 
/*N*/ 	if (rIPRef.Is() &&
/*N*/ 		(rIPRef->GetMiscStatus() & SVOBJ_MISCSTATUS_NOTRESIZEABLE) )
/*N*/ 	{
/*N*/ 		SetResizeProtect(TRUE);
/*N*/ 	}
/*N*/ 
/*N*/     // #108759# For math objects, set closed state to transparent
/*N*/     if( ImplIsMathObj( *ppObjRef ) )
/*N*/         SetClosedObj( false );
/*N*/ 
/*N*/ 	Connect();
/*N*/ 	SetChanged();
/*N*/ 	SendRepaintBroadcast();
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdrOle2Obj::SetClosedObj( bool bIsClosed )
/*N*/ {
/*N*/     // #108759# Allow changes to the closed state of OLE objects
/*N*/     bClosedObj = bIsClosed;
/*N*/ }

// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------

/*N*/ XubString SdrOle2Obj::GetName() const
/*N*/ {
/*N*/ 	return aName;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdrOle2Obj::SetPersistName( const String& rPersistName )
/*N*/ { 
/*N*/   mpImpl->aPersistName = rPersistName;
/*N*/ 	SetChanged();
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ String SdrOle2Obj::GetPersistName() const
/*N*/ {
/*N*/     return mpImpl->aPersistName;
/*N*/ }

// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------

/*N*/ UINT16 SdrOle2Obj::GetObjIdentifier() const
/*N*/ {
/*N*/ 	return bFrame ? UINT16(OBJ_FRAME) : UINT16(OBJ_OLE2);
/*N*/ }

// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------

/*N*/ SdrObject* SdrOle2Obj::CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const
/*N*/ {
/*N*/ 	return ImpCheckHit(rPnt,nTol,pVisiLayer,TRUE,TRUE);
/*N*/ }

// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------

/*NBFF*/ void SdrOle2Obj::ImpSetVisAreaSize()
/*NBFF*/ {
/*NBFF*/ 	GetObjRef();	// try to load inplace object
/*NBFF*/ 	SvInPlaceObjectRef& rIPRef=*ppObjRef;
/*NBFF*/ 
/*NBFF*/ 	if (rIPRef.Is())
/*NBFF*/ 	{
/*NBFF*/ 		if (rIPRef->GetMiscStatus() & SVOBJ_MISCSTATUS_SERVERRESIZE)
/*NBFF*/ 		{
/*NBFF*/ 			// Server resized selbst (StarChart)
/*NBFF*/ 			// Neue VisArea setzen
/*NBFF*/ 			Rectangle aVisArea = OutputDevice::LogicToLogic( aRect,
/*NBFF*/ 									pModel->GetScaleUnit(), rIPRef->GetMapUnit() );
/*NBFF*/ 			rIPRef->SetVisArea(aVisArea);
/*NBFF*/ 
/*NBFF*/ 			// Wurde die VisArea akzeptiert?
/*NBFF*/ 			Rectangle aAcceptedVisArea(rIPRef->GetVisArea());
/*NBFF*/ 
/*NBFF*/ 			if (aVisArea.GetSize() != aAcceptedVisArea.GetSize())
/*NBFF*/ 			{
/*NBFF*/ 				// VisArea wurde nicht akzeptiert -> korrigieren
/*NBFF*/ 				aRect.SetSize(OutputDevice::LogicToLogic( aAcceptedVisArea.GetSize(),
/*NBFF*/ 						rIPRef->GetMapUnit(), pModel->GetScaleUnit()));
/*NBFF*/ 			}
/*NBFF*/ 		}
/*NBFF*/ 		else
/*NBFF*/ 		{
/*NBFF*/ 			///**********************************************************************
/*NBFF*/ 			//* Nun wird nicht mehr die VisArea gesetzt, sondern es erfolgt eine
/*NBFF*/ 			//* Skalierung
/*NBFF*/ 			//**********************************************************************/
/*NBFF*/ 			SvEmbeddedClient* pClient = (*ppObjRef)->GetClient();
/*NBFF*/ 
/*NBFF*/ 			if (pClient)
/*NBFF*/ 			{
/*NBFF*/ 				SvClientData* pData = pClient->GetClientData();
/*NBFF*/ 				Size aObjAreaSize = rIPRef->GetVisArea().GetSize();
/*NBFF*/ 				aObjAreaSize = OutputDevice::LogicToLogic( aObjAreaSize,
/*NBFF*/ 														   rIPRef->GetMapUnit(),
/*NBFF*/ 														   pModel->GetScaleUnit() );
/*NBFF*/ 
/*NBFF*/ 				Size aSize = aRect.GetSize();
/*NBFF*/ 				Fraction aScaleWidth (aSize.Width(),  aObjAreaSize.Width() );
/*NBFF*/ 				Fraction aScaleHeight(aSize.Height(), aObjAreaSize.Height() );
/*NBFF*/ 				// Nun auf 10 Binaerstellen kuerzen
/*NBFF*/ 				Kuerzen(aScaleHeight, 10);
/*NBFF*/ 				Kuerzen(aScaleWidth,  10);
/*NBFF*/ 
/*NBFF*/ 				pData->SetSizeScale(aScaleWidth, aScaleHeight);
/*NBFF*/ 
/*NBFF*/ 				Rectangle aScaleRect(aRect.TopLeft(), aObjAreaSize);
/*NBFF*/ 				pData->SetObjArea(aScaleRect);
/*NBFF*/ 			}
/*NBFF*/ 		}
/*NBFF*/ 	}
/*NBFF*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdrOle2Obj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
/*N*/ {
/*N*/ 	SdrRectObj::NbcResize(rRef,xFact,yFact);
/*N*/ 	if (aGeo.nShearWink!=0 || aGeo.nDrehWink!=0) { // kleine Korrekturen
/*?*/ 		if (aGeo.nDrehWink>=9000 && aGeo.nDrehWink<27000) {
/*?*/ 			aRect.Move(aRect.Left()-aRect.Right(),aRect.Top()-aRect.Bottom());
/*?*/ 		}
/*?*/ 		aGeo.nDrehWink=0;
/*?*/ 		aGeo.nShearWink=0;
/*?*/ 		aGeo.nSin=0.0;
/*?*/ 		aGeo.nCos=1.0;
/*?*/ 		aGeo.nTan=0.0;
/*?*/ 		SetRectsDirty();
/*N*/ 	}
/*N*/ 	if( (NULL == pModel) || !pModel->isLocked() )
/*NBFF*/	ImpSetVisAreaSize();
/*N*/ }

// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------

/*N*/ void SdrOle2Obj::NbcSetSnapRect(const Rectangle& rRect)
/*N*/ {
/*N*/ 	SdrRectObj::NbcSetSnapRect(rRect);
/*N*/ 	if( (NULL == pModel) || !pModel->isLocked() )
/*NBFF*/	ImpSetVisAreaSize();
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdrOle2Obj::NbcSetLogicRect(const Rectangle& rRect)
/*N*/ {
/*N*/ 	SdrRectObj::NbcSetLogicRect(rRect);
/*N*/ 	if( (NULL == pModel) || !pModel->isLocked() )
/*NBFF*/	ImpSetVisAreaSize();
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ FASTBOOL SdrOle2Obj::HasGDIMetaFile() const
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); return FALSE;//STRIP001 
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ const GDIMetaFile* SdrOle2Obj::GetGDIMetaFile() const
/*N*/ {DBG_BF_ASSERT(0, "STRIP");return NULL; //STRIP001 
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdrOle2Obj::WriteData(SvStream& rOut) const
/*N*/ {
/*N*/ 	SdrRectObj::WriteData(rOut);
/*N*/ 	SdrDownCompat aCompat(rOut,STREAM_WRITE); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("SdrOle2Obj");
/*N*/ #endif
/*N*/ 
/*N*/ 	// UNICODE: rOut<<mpImpl->aPersistName;
/*N*/ 	rOut.WriteByteString(mpImpl->aPersistName);
/*N*/ 
/*N*/ 	// UNICODE: rOut<<aProgName;
/*N*/ 	rOut.WriteByteString(aProgName);
/*N*/ 
/*N*/ 	GetObjRef();
/*N*/ 	BOOL bObjRefValid=ppObjRef->Is();
/*N*/ 	rOut<<bObjRefValid;
/*N*/ 	BOOL bPreview = FALSE;
/*N*/ 	if( !IsEmptyPresObj() && pModel && pModel->IsSaveOLEPreview() )
/*N*/ 		bPreview = TRUE;
/*N*/ 
/*N*/ 	if( bPreview )
/*N*/ 	{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	}

/*N*/ 	BOOL bHasGraphic=pGraphic!=NULL;

        // #i27418# If there actually is no graphic, do not write one. This will lead
        // to loading an empty bitmap in older OOo versions which sets an error code at 
        // the stream -> load breaks.
        if(bHasGraphic)
        {
            if(GRAPHIC_NONE == pGraphic->GetType())
            {
                bHasGraphic = false;
            }
        }

/*N*/ 	rOut<<bHasGraphic;
/*N*/ 	if (bHasGraphic)
/*N*/ 	{
/*?*/ 		SdrDownCompat aGrafCompat(rOut,STREAM_WRITE); // ab V11 eingepackt
/*?*/ #ifdef DBG_UTIL
/*?*/ 		aGrafCompat.SetID("SdrOle2Obj(Graphic)");
/*?*/ #endif
/*?*/ 		rOut<<*pGraphic;
/*N*/ 	}
/*N*/ 
/*N*/ 	if( bPreview )
/*?*/ 		( (SdrOle2Obj*) this )->SetGraphic( NULL );		// remove preview graphic
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdrOle2Obj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
/*N*/ {
/*N*/ 	rIn.SetError( 0 );
/*N*/ 
/*N*/ 	if (rIn.GetError()!=0) return;
/*N*/ 	SdrRectObj::ReadData(rHead,rIn);
/*N*/ 	SdrDownCompat aCompat(rIn,STREAM_READ); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("SdrOle2Obj");
/*N*/ #endif
/*N*/ 
/*N*/ 	// UNICODE: rIn >> mpImpl->aPersistName;
/*N*/ 	rIn.ReadByteString(mpImpl->aPersistName);
/*N*/ 
/*N*/ 	// UNICODE: rIn >> aProgName;
/*N*/ 	rIn.ReadByteString(aProgName);
/*N*/ 
/*N*/ 	BOOL bObjRefValid;
/*N*/ 	rIn>>bObjRefValid;
/*N*/ 
/*N*/ 	BOOL bHasGraphic;
/*N*/ 	rIn>>bHasGraphic;
/*N*/ 	if (bHasGraphic)
/*N*/ 	{
/*?*/ 		if(pGraphic==NULL)
/*?*/ 			pGraphic=new Graphic;
/*?*/ 

        // #i27418# Remember stream position to decide if something was read
        const sal_Size nFilePosition(rIn.Tell());

/*?*/ 		if(rHead.GetVersion()>=11)
/*?*/ 		{ // ab V11 eingepackt
/*?*/ 			SdrDownCompat aGrafCompat(rIn,STREAM_READ);
/*?*/ #ifdef DBG_UTIL
/*?*/ 			aGrafCompat.SetID("SdrOle2Obj(Graphic)");
/*?*/ #endif
/*?*/ 			rIn>>*pGraphic;
/*?*/ 		}
/*?*/ 		else
/*?*/ 			rIn>>*pGraphic;
/*?*/ 

        // #i27418# If 4 bytes were read but an error is set, it was tried to read an empty
        // bitmap into the Graphic. This a follow-up error from previously wrong saves.
        // Reset error code here, it's not really an error but leads to a crash in SO7
        if(0L != rIn.GetError() && (nFilePosition + 4L) == rIn.Tell())
        {
            rIn.ResetError();
        }

/*?*/ 		if( mpImpl->pGraphicObject )
/*?*/ 			delete mpImpl->pGraphicObject;
/*?*/ 
/*?*/ 		mpImpl->pGraphicObject = new BfGraphicObject( *pGraphic );
/*N*/ 	}
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdrOle2Obj::NbcMove(const Size& rSize)
/*N*/ {
/*N*/ 	SdrRectObj::NbcMove(rSize);
/*N*/ 	if( (NULL == pModel) || !pModel->isLocked() )
/*NBFF*/	ImpSetVisAreaSize();
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ BOOL SdrOle2Obj::Unload()
/*N*/ {
/*N*/ 	BOOL bUnloaded = FALSE;
/*N*/ 
/*N*/ 	if( ppObjRef && ppObjRef->Is() )
/*N*/ 	{
/*N*/ 		//Nicht notwendig im Doc DTor (MM)
/*N*/ 		ULONG nRefCount = (*ppObjRef)->GetRefCount();
/*N*/ 		// prevent Unload if there are external references
/*N*/ 		if( nRefCount > 2 )
/*N*/ 			return FALSE;
/*N*/ 		DBG_ASSERT( nRefCount == 2, "Wrong RefCount for unload" );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		bUnloaded = TRUE;
/*N*/ 
/*N*/ 	if (pModel && ppObjRef && ppObjRef->Is() &&
/*N*/ 		SVOBJ_MISCSTATUS_ALWAYSACTIVATE != (*ppObjRef)->GetMiscStatus() &&
/*N*/ 		1 < (*ppObjRef)->GetRefCount()                                  &&
/*N*/ 		!(*ppObjRef)->IsModified()                                      &&
/*N*/ 		!(*ppObjRef)->GetProtocol().IsInPlaceActive() )
/*N*/ 	{
/*N*/ 		SvPersist* pPersist = pModel->GetPersist();
/*N*/ 
/*N*/ 		if (pPersist)
/*N*/ 		{
/*N*/ 			SvPersist* pO = *ppObjRef;
/*N*/ 			if( pO->IsModified() )
/*N*/ 			{
/*?*/ 				pO->DoSave();
/*?*/ 				pO->DoSaveCompleted();
/*N*/ 			}
/*N*/ 			ppObjRef->Clear();
/*N*/ 			if (pPersist->Unload(pO))
/*N*/ 				bUnloaded = TRUE;
/*N*/ 			else
/*?*/ 				*ppObjRef = pO;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return bUnloaded;
/*N*/ }

// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------

/*N*/ const SvInPlaceObjectRef& SdrOle2Obj::GetObjRef() const
/*N*/ {
/*N*/ 	if ( !ppObjRef->Is() && pModel && pModel->GetPersist() && !pModel->GetPersist()->IsHandsOff() )
/*N*/ 	{
/*N*/ 		// #107645#
/*N*/ 		// Only try loading if it did not wrent wrong up to now
/*N*/ 		if(!mpImpl->mbLoadingOLEObjectFailed)
/*N*/ 		{
/*N*/ 			// Objekt laden
/*N*/ 			(*ppObjRef) = &( pModel->GetPersist()->GetObject( mpImpl->aPersistName ) );
/*N*/ 			
/*N*/ 			// #107645#
/*N*/ 			// If loading of OLE object failed, remember that to not invoke a endless
/*N*/ 			// loop trying to load it again and again.
/*N*/ 			if(!ppObjRef->Is())
/*N*/ 			{
/*N*/ 				mpImpl->mbLoadingOLEObjectFailed = sal_True;
/*N*/ 			}
/*N*/ 
/*N*/             // #108759# For math objects, set closed state to transparent
/*N*/             if( ImplIsMathObj( *ppObjRef ) )
/*N*/                 const_cast<SdrOle2Obj*>(this)->SetClosedObj( false );
/*N*/ 		}
/*N*/ 
/*N*/ 		if ( ppObjRef->Is() )
/*N*/ 		{
/*N*/ 			if( !IsEmptyPresObj() )
/*N*/ 			{
/*N*/ 				// #75637# remember modified status of model
/*N*/ 				BOOL bWasChanged(pModel ? pModel->IsChanged() : FALSE);
/*N*/ 
/*N*/ 				// perhaps preview not valid anymore
/*N*/ 				// #75637# This line changes the modified state of the model
/*N*/ 				( (SdrOle2Obj*) this )->SetGraphic( NULL );
/*N*/ 
/*N*/ 				// #75637# if status was not set before, force it back
/*N*/ 				// to not set, so that SetGraphic(0L) above does not
/*N*/ 				// set the modified state of the model.
/*N*/ 				if(!bWasChanged && pModel && pModel->IsChanged())
/*N*/ 					pModel->SetChanged(FALSE);
/*N*/ 			}
/*N*/ 
/*N*/ 			if ( (*ppObjRef)->GetMiscStatus() & SVOBJ_MISCSTATUS_RESIZEONPRINTERCHANGE )
/*N*/ 			{
/*N*/ 				if (pModel && pModel->GetRefDevice() &&
/*N*/ 					pModel->GetRefDevice()->GetOutDevType() == OUTDEV_PRINTER)
/*N*/ 				{
/*N*/ 					if(!bInDestruction)
/*N*/ 					{
/*N*/ 						// prevent SetModified (don't want no update here)
/*N*/ 						sal_Bool bWasEnabled = (*ppObjRef)->IsEnableSetModified();
/*N*/ 						if ( bWasEnabled )
/*N*/ 							(*ppObjRef)->EnableSetModified( sal_False );
/*N*/ 
/*N*/ 						// Kein RefDevice oder RefDevice kein Printer
/*N*/ 						Printer* pPrinter = (Printer*) pModel->GetRefDevice();
/*N*/ 						(*ppObjRef)->OnDocumentPrinterChanged( pPrinter );
/*N*/ 
/*N*/ 						// reset state
/*N*/ 						(*ppObjRef)->EnableSetModified( bWasEnabled );
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 
/*N*/ 			// register modify listener on initial load
/*N*/ 			if( pModifyListener == NULL )
/*N*/ 			{
/*N*/ 				((SdrOle2Obj*)this)->pModifyListener = new SvxUnoShapeModifyListener( (SdrOle2Obj*)this );
/*N*/ 				pModifyListener->acquire();
/*N*/ 
/*N*/ 				uno::Reference< util::XModifyBroadcaster > xBC( getXModel(), uno::UNO_QUERY );
/*N*/ 				if( xBC.is() && pModifyListener )
/*N*/ 				{
/*N*/ 					uno::Reference< util::XModifyListener > xListener( pModifyListener );
/*N*/ 					xBC->addModifyListener( xListener );
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if (ppObjRef->Is())
/*N*/ 	{
/*N*/ 		// In Cache einfuegen
/*N*/ 		GetSdrGlobalData().GetOLEObjCache().InsertObj((SdrOle2Obj*) this);
/*N*/ 	}
/*N*/ 
/*N*/ 	return *ppObjRef;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ uno::Reference< frame::XModel > SdrOle2Obj::getXModel() const
/*N*/ {
/*N*/ 	uno::Reference< frame::XModel > xModel;
/*N*/ 
/*N*/ 	if( pModel )
/*N*/ 	{
/*N*/ 		SvInPlaceObjectRef xSvIPO( GetObjRef() );
/*N*/ 		SfxInPlaceObjectRef xIPO( xSvIPO );
/*N*/ 		if( xIPO.Is() )
/*N*/ 		{
/*N*/ 			SfxObjectShell* pShell = xIPO->GetObjectShell();
/*N*/ 
/*N*/ 			if( pShell )
/*N*/ 				xModel = pShell->GetModel();
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return xModel;
/*N*/ }

// -----------------------------------------------------------------------------

/// #110015# sets the visible area at the SvInPlaceObject and SvEmbeddedInfoObject
/*N*/ void SdrOle2Obj::SetVisibleArea( const Rectangle& rVisArea )
/*N*/ {
/*N*/ 	const SvInPlaceObjectRef& xInplace = GetObjRef();
/*N*/ 	if( xInplace.Is() )
/*N*/ 	{
/*N*/ 		xInplace->SetVisArea( rVisArea );
/*N*/ 
/*N*/ 		if( pModel && (!pModel->GetPersist()->IsEnableSetModified()) )
/*N*/ 			xInplace->SetModified(FALSE);
/*N*/ 	}
/*N*/ 
/*N*/ 	if(pModel && mpImpl->aPersistName.Len() )
/*N*/ 	{
/*N*/ 		SvPersist* pPers = pModel->GetPersist();
/*N*/ 
/*N*/ 		if (pPers)
/*N*/ 		{
/*N*/ 			SvInfoObject* pInfo = pPers->Find(mpImpl->aPersistName);
/*N*/ 			SvEmbeddedInfoObject * pEmbed = PTR_CAST(SvEmbeddedInfoObject, pInfo );
/*N*/ 
/*N*/ 			if( pEmbed )
/*N*/ 				pEmbed->SetInfoVisArea( rVisArea );
/*N*/ 		}
/*N*/ 	}
/*N*/ }

// -----------------------------------------------------------------------------
}
