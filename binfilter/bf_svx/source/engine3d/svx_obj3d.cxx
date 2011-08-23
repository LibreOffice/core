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

#define ITEMID_COLOR			SID_ATTR_3D_LIGHTCOLOR

#include "svdstr.hrc"

#ifndef _SVDVIEW_HXX
#include "svdview.hxx"
#endif






#ifndef _E3D_GLOBL3D_HXX
#include "globl3d.hxx"
#endif



#ifndef _E3D_POLYOB3D_HXX
#include "polyob3d.hxx"
#endif


#ifndef _E3D_POLYSC3D_HXX
#include "polysc3d.hxx"
#endif






#ifndef _XOUTX_HXX
#include "xoutx.hxx"
#endif











#ifndef _SVX_XLNWTIT_HXX
#include "xlnwtit.hxx"
#endif


#ifndef _SVDIO_HXX
#include "svdio.hxx"
#endif



#ifndef _SVX_SVXIDS_HRC
#include "svxids.hrc"
#endif











#ifndef _EEITEM_HXX
#include "eeitem.hxx"
#endif


namespace binfilter {

#define ITEMVALUE(ItemSet,Id,Cast)	((const Cast&)(ItemSet).Get(Id)).GetValue()

/*************************************************************************
|*
|* Liste fuer 3D-Objekte
|*
\************************************************************************/

/*N*/ TYPEINIT1(E3dObjList, SdrObjList);

/*N*/ E3dObjList::E3dObjList(SdrModel* pNewModel, SdrPage* pNewPage, E3dObjList* pNewUpList)
/*N*/ :	SdrObjList(pNewModel, pNewPage, pNewUpList)
/*N*/ {
/*N*/ }

/*?*/ E3dObjList::E3dObjList(const E3dObjList& rSrcList)
/*?*/ :	SdrObjList(rSrcList)
/*?*/ {
/*?*/ }

/*N*/ E3dObjList::~E3dObjList()
/*N*/ {
/*N*/ }

/*N*/ void E3dObjList::NbcInsertObject(SdrObject* pObj, ULONG nPos, const SdrInsertReason* pReason)
/*N*/ {
/*N*/ 	// Owner holen
/*N*/ 	DBG_ASSERT(GetOwnerObj()->ISA(E3dObject), "AW: Einfuegen 3DObject in Parent != 3DObject");
/*N*/ 
/*N*/ 	// Ist es ueberhaupt ein 3D-Objekt?
/*N*/ 	if(pObj && pObj->ISA(E3dObject))
/*N*/ 	{
/*N*/ 		// Normales 3D Objekt, einfuegen mittels
/*N*/ 		// call parent
/*N*/ 		SdrObjList::NbcInsertObject(pObj, nPos, pReason);
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*?*/ 		// Kein 3D Objekt, fuege in Seite statt in Szene ein...
/*?*/ 		GetOwnerObj()->GetPage()->InsertObject(pObj, nPos);
/*N*/ 	}
/*N*/ }


/*N*/ SdrObject* E3dObjList::RemoveObject(ULONG nObjNum)
/*N*/ {
/*N*/ 	// Owner holen
/*N*/ 	DBG_ASSERT(GetOwnerObj()->ISA(E3dObject), "AW: Entfernen 3DObject aus Parent != 3DObject");
/*N*/ 	E3dObject* pOwner = (E3dObject*)GetOwnerObj();
/*N*/ 
/*N*/ 	// call parent
/*N*/ 	SdrObject* pRetval = SdrObjList::RemoveObject(nObjNum);
/*N*/ 
/*N*/ 	// FitSnapRectToBoundVol vorbereiten
/*N*/ 	if(GetOwnerObj() && GetOwnerObj()->ISA(E3dScene))
/*N*/ 		((E3dScene*)GetOwnerObj())->CorrectSceneDimensions();
/*N*/ 
/*N*/ 	return pRetval;
/*N*/ }

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

/*N*/ TYPEINIT1(E3dObject, SdrAttrObj);

/*N*/ E3dObject::E3dObject() :
/*N*/ 	nLogicalGroup(0),
/*N*/ 	nObjTreeLevel(0),
/*N*/ 	eDragDetail(E3DDETAIL_ONEBOX),
/*N*/ 	nPartOfParent(0),
/*N*/ 	bTfHasChanged(TRUE),
/*N*/ 	bBoundVolValid(TRUE),
/*N*/ 	bIsSelected(FALSE)
/*N*/ {
/*N*/ 	bIs3DObj = TRUE;
/*N*/ 	pSub = new E3dObjList(NULL, NULL);
/*N*/ 	pSub->SetOwnerObj(this);
/*N*/ 	pSub->SetListKind(SDROBJLIST_GROUPOBJ);
/*N*/ 	bClosedObj = TRUE;
/*N*/ }

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

/*N*/ E3dObject::~E3dObject()
/*N*/ {
/*N*/ 	delete pSub;
/*N*/ 	pSub = NULL;
/*N*/ }

/*************************************************************************
|*
|* Selektions-Flag setzen
|*
\************************************************************************/


/*************************************************************************
|*
|* Aufbrechen, default-Implementierungen
|*
\************************************************************************/


/*N*/ SdrAttrObj* E3dObject::GetBreakObj()
/*N*/ {
/*N*/ 	return 0L;
/*N*/ }

/*************************************************************************
|*
|* SetRectsDirty muss ueber die lokale SdrSubList gehen
|*
\************************************************************************/

/*N*/ void E3dObject::SetRectsDirty(FASTBOOL bNotMyself)
/*N*/ {
/*N*/ 	// call parent
/*N*/ 	SdrAttrObj::SetRectsDirty(bNotMyself);
/*N*/ 
/*N*/ 	// Eigene SubListe AUCH behandeln
/*N*/ 	if(pSub && pSub->GetObjCount())
/*N*/ 	{
/*N*/ 		for (ULONG i = 0; i < pSub->GetObjCount(); i++)
/*N*/ 		{
/*N*/ 			SdrObject* pObj = pSub->GetObj(i);
/*N*/ 			DBG_ASSERT(pObj->ISA(E3dObject), "AW: In E3dObject sind nur 3D-Objekte erlaubt!");
/*N*/ 			((E3dObject*)pObj)->SetRectsDirty(bNotMyself);
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Inventor zurueckgeben
|*
\************************************************************************/

/*N*/ UINT32 E3dObject::GetObjInventor() const
/*N*/ {
/*N*/ 	return E3dInventor;
/*N*/ }

/*************************************************************************
|*
|* Identifier zurueckgeben
|*
\************************************************************************/

/*N*/ UINT16 E3dObject::GetObjIdentifier() const
/*N*/ {
/*N*/ 	return E3D_OBJECT_ID;
/*N*/ }

/*************************************************************************
|*
|* Faehigkeiten des Objektes feststellen
|*
\************************************************************************/


/*************************************************************************
|*
|* Layer abfragen
|*
\************************************************************************/

/*N*/ SdrLayerID E3dObject::GetLayer() const
/*N*/ {
/*N*/ 	FASTBOOL bFirst = TRUE;
/*N*/ 	E3dObjList* pOL = pSub;
/*N*/ 	ULONG       nObjCnt = pOL->GetObjCount();
/*N*/ 	SdrLayerID  nLayer = SdrLayerID(nLayerID);
/*N*/ 
/*N*/ 	for ( ULONG i = 0; i < nObjCnt; i++ )
/*N*/ 	{
/*N*/ 		SdrLayerID nObjLayer;
/*N*/ 		if(pOL->GetObj(i)->ISA(E3dPolyObj))
/*N*/ 			nObjLayer = SdrLayerID(nLayerID);
/*N*/ 		else
/*N*/ 			nObjLayer = pOL->GetObj(i)->GetLayer();
/*N*/ 
/*N*/ 		if (bFirst)
/*N*/ 		{
/*N*/ 			nLayer = nObjLayer;
/*N*/ 			bFirst = FALSE;
/*N*/ 		}
/*N*/ 		else if ( nObjLayer != nLayer )
/*N*/ 			return 0;
/*N*/ 	}
/*N*/ 	return nLayer;
/*N*/ }

/*************************************************************************
|*
|* Layer setzen
|*
\************************************************************************/

/*N*/ void E3dObject::NbcSetLayer(SdrLayerID nLayer)
/*N*/ {
/*N*/ 	SdrAttrObj::NbcSetLayer(nLayer);
/*N*/ 
/*N*/ 	E3dObjList* pOL = pSub;
/*N*/ 	ULONG nObjCnt = pOL->GetObjCount();
/*N*/ 	ULONG i;
/*N*/ 	for ( i = 0; i < nObjCnt; i++ )
/*N*/ 		pOL->GetObj(i)->NbcSetLayer(nLayer);
/*N*/ }

/*************************************************************************
|*
|* ObjList auch an SubList setzen
|*
\************************************************************************/

/*N*/ void E3dObject::SetObjList(SdrObjList* pNewObjList)
/*N*/ {
/*N*/ 	SdrObject::SetObjList(pNewObjList);
/*N*/ 	pSub->SetUpList(pNewObjList);
/*N*/ }

/*************************************************************************
|*
|* Layer setzen
|*
\************************************************************************/

/*N*/ void E3dObject::SetPage(SdrPage* pNewPage)
/*N*/ {
/*N*/ 	SdrAttrObj::SetPage(pNewPage);
/*N*/ 	pSub->SetPage(pNewPage);
/*N*/ }

/*************************************************************************
|*
|* Layer setzen
|*
\************************************************************************/

/*N*/ void E3dObject::SetModel(SdrModel* pNewModel)
/*N*/ {
/*N*/ 	SdrAttrObj::SetModel(pNewModel);
/*N*/ 	pSub->SetModel(pNewModel);
/*N*/ }

/*************************************************************************
|*
|* resize object, used from old 2d interfaces, e.g. in Move/Scale dialog
|* (F4)
|*
\************************************************************************/
/*N*/ void E3dObject::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
/*N*/ {
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	// SdrAttrObj::NbcResize(rRef, xFact, yFact);
/*N*/ 
/*N*/ 	// Bewegung in X,Y im Augkoordinatensystem
/*N*/ 	E3dScene* pScene = GetScene();
/*N*/ 
/*N*/ 	if(pScene)
/*N*/ 	{
/*N*/ 		// pos ermitteln
/*N*/ 		B3dTransformationSet& rTransSet = pScene->GetCameraSet();
/*N*/ 		Vector3D aScaleCenter((double)rRef.X(), (double)rRef.Y(), 32768.0);
/*N*/ 		aScaleCenter = rTransSet.ViewToEyeCoor(aScaleCenter);
/*N*/ 
/*N*/ 		// scale-faktoren holen
/*N*/ 		double fScaleX = xFact;
/*N*/ 		double fScaleY = yFact;
/*N*/ 
/*N*/ 		// build transform
/*N*/ 		Matrix4D mFullTransform(GetFullTransform());
/*N*/ 		Matrix4D mTrans(mFullTransform);
/*N*/ 
/*N*/ 		mTrans *= rTransSet.GetOrientation();
/*N*/ 		mTrans.Translate(-aScaleCenter);
/*N*/ 		mTrans.Scale(fScaleX, fScaleY, 1.0);
/*N*/ 		mTrans.Translate(aScaleCenter);
/*N*/ 		mTrans *= rTransSet.GetInvOrientation();
/*N*/ 		mFullTransform.Invert();
/*N*/ 		mTrans *= mFullTransform;
/*N*/ 
/*N*/ 		// anwenden
/*N*/ 		Matrix4D mObjTrans(GetTransform());
/*N*/ 		mObjTrans *= mTrans;
/*N*/ 		SetTransform(mObjTrans);
/*N*/ 
/*N*/ 		// force new camera and SnapRect on scene, geometry may have really
/*N*/ 		// changed
/*N*/ 		pScene->CorrectSceneDimensions();
/*N*/ 	}
/*N*/ #endif
/*N*/ }

/*************************************************************************
|*
|* Objekt verschieben in 2D, wird bei Cursortasten benoetigt
|*
\************************************************************************/

/*************************************************************************
|*
|* liefere die Sublist, aber nur dann, wenn darin Objekte enthalten sind !
|*
\************************************************************************/

/*N*/ SdrObjList* E3dObject::GetSubList() const
/*N*/ {
/*N*/ 	return pSub;
/*N*/ }

/*************************************************************************
|*
|* Anzahl der Handles zurueckgeben
|*
\************************************************************************/


/*************************************************************************
|*
|* Handle-Liste fuellen
|*
\************************************************************************/


/*************************************************************************
|*
\************************************************************************/


/*************************************************************************
|*
|* Paint; wird z.Z. nicht benutzt, da das Paint ueber die
|* (2D-)Displayliste der Szene laeuft
|*
\************************************************************************/


/*************************************************************************
|*
|* Objekt als Kontur in das Polygon einfuegen
|*
\************************************************************************/


/*************************************************************************
|*
|* Schatten fuer 3D-Objekte zeichnen
|*
\************************************************************************/


/*************************************************************************
|*
|* SnapRect berechnen
|*
\************************************************************************/

/*N*/ void E3dObject::RecalcSnapRect()
/*N*/ {
/*N*/ 	maSnapRect = Rectangle();
/*N*/ 	if(pSub && pSub->GetObjCount())
/*N*/ 	{
/*N*/ 		for (ULONG i = 0; i < pSub->GetObjCount(); i++)
/*N*/ 		{
/*N*/ 			SdrObject* pObj = pSub->GetObj(i);
/*N*/ 			DBG_ASSERT(pObj->ISA(E3dObject), "AW: In E3dObject sind nur 3D-Objekte erlaubt!");
/*N*/ 			Rectangle aSubRect = ((E3dObject*)pObj)->GetSnapRect();
/*N*/ 			maSnapRect.Union(aSubRect);
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* BoundRect berechnen
|*
\************************************************************************/

/*N*/ void E3dObject::RecalcBoundRect()
/*N*/ {
/*N*/ 	// BoundRect aus SnapRect berechnen
/*N*/ 	aOutRect = GetSnapRect();
/*N*/ 
/*N*/ 	if(pSub && pSub->GetObjCount())
/*N*/ 	{
/*N*/ 		for (ULONG i = 0; i < pSub->GetObjCount(); i++)
/*N*/ 		{
/*N*/ 			SdrObject* pObj = pSub->GetObj(i);
/*N*/ 			DBG_ASSERT(pObj->ISA(E3dObject), "AW: In E3dObject sind nur 3D-Objekte erlaubt!");
/*N*/ 			Rectangle aSubRect = ((E3dObject*)pObj)->GetBoundRect();
/*N*/ 			aOutRect.Union(aSubRect);
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Einfuegen eines 3D-Objekts an den Parent weitermelden, damit dieser
|* ggf. eine Sonderbehandlung fuer spezielle Objekte durchfuehren kann
|* (z.B. Light/Label in E3dScene)
|*
\************************************************************************/

/*N*/ void E3dObject::NewObjectInserted(const E3dObject* p3DObj)
/*N*/ {
/*N*/ 	if(GetParentObj())
/*N*/ 		GetParentObj()->NewObjectInserted(p3DObj);
/*N*/ }

/*************************************************************************
|*
|* Parent ueber Aenderung der Struktur (z.B. durch Transformation)
|* informieren; dabei wird das Objekt, in welchem die Aenderung
|* aufgetreten ist, uebergeben
|*
\************************************************************************/

/*N*/ void E3dObject::StructureChanged(const E3dObject* p3DObj)
/*N*/ {
/*N*/ 	if ( GetParentObj() )
/*N*/ 	{
/*N*/ 		// Wenn sich im Child das BoundVolume geaendert hat, muessen
/*N*/ 		// auch die der Parents angepasst werden
/*N*/ 		if ( !p3DObj->bBoundVolValid )
/*N*/ 			GetParentObj()->bBoundVolValid = FALSE;
/*N*/ 
/*N*/ 		GetParentObj()->StructureChanged(p3DObj);
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* 3D-Objekt einfuegen
|*
\************************************************************************/

/*N*/ void E3dObject::Insert3DObj(E3dObject* p3DObj)
/*N*/ {
/*N*/ 	DBG_ASSERT(p3DObj, "Insert3DObj mit NULL-Zeiger!");
/*N*/ 	p3DObj->SetObjTreeLevel(nObjTreeLevel + 1);
/*N*/ 	SdrPage* pPg = pPage;
/*N*/ 	pSub->InsertObject(p3DObj);
/*N*/ 	pPage = pPg;
/*N*/ 	bBoundVolValid = FALSE;
/*N*/ 	NewObjectInserted(p3DObj);
/*N*/ 	StructureChanged(this);
/*N*/ }

/*N*/ void E3dObject::Remove3DObj(E3dObject* p3DObj)
/*N*/ {
/*N*/ 	DBG_ASSERT(p3DObj, "Remove3DObj mit NULL-Zeiger!");
/*N*/ 
/*N*/ 	if(p3DObj->GetParentObj() == this)
/*N*/ 	{
/*N*/ 		SdrPage* pPg = pPage;
/*N*/ 		pSub->RemoveObject(p3DObj->GetOrdNum());
/*N*/ 		pPage = pPg;
/*N*/ 
/*N*/ 		bBoundVolValid = FALSE;
/*N*/ 		StructureChanged(this);
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Parent holen
|*
\************************************************************************/

/*N*/ E3dObject* E3dObject::GetParentObj() const
/*N*/ {
/*N*/ 	E3dObject* pRetval = NULL;
/*N*/ 
/*N*/ 	if(GetObjList()
/*N*/ 		&& GetObjList()->GetOwnerObj()
/*N*/ 		&& GetObjList()->GetOwnerObj()->ISA(E3dObject))
/*N*/ 		pRetval = ((E3dObject*)GetObjList()->GetOwnerObj());
/*N*/ 	return pRetval;
/*N*/ }

/*************************************************************************
|*
|* Uebergeordnetes Szenenobjekt bestimmen
|*
\************************************************************************/

/*N*/ E3dScene* E3dObject::GetScene() const
/*N*/ {
/*N*/ 	if(GetParentObj())
/*N*/ 		return GetParentObj()->GetScene();
/*N*/ 	return NULL;
/*N*/ }

/*************************************************************************
|*
|* umschliessendes Volumen inklusive aller Kindobjekte berechnen
|*
\************************************************************************/

/*N*/ void E3dObject::RecalcBoundVolume()
/*N*/ {
/*N*/ 	E3dObjList* pOL = pSub;
/*N*/ 	ULONG nObjCnt = pOL->GetObjCount();
/*N*/ 
/*N*/ 	if(nObjCnt)
/*N*/ 	{
/*N*/ 		aBoundVol = Volume3D();
/*N*/ 
/*N*/ 		for (ULONG i = 0; i < nObjCnt; i++)
/*N*/ 		{
/*N*/ 			SdrObject* pObj = pOL->GetObj(i);
/*N*/ 
/*N*/ 			DBG_ASSERT(pObj->ISA(E3dObject), "In E3dObject sind nur 3D-Objekte erlaubt!");
/*N*/ 			// Bei den Kindobjekten auch die lokalen Transformationen
/*N*/ 			// beruecksichtigen
/*N*/ 			E3dObject* p3DObj = (E3dObject*) pObj;
/*N*/ 			const Volume3D& rVol = p3DObj->GetBoundVolume();
/*N*/ 			const Matrix4D& rTf  = p3DObj->GetTransform();
/*N*/ 			aBoundVol.Union(rVol.GetTransformVolume(rTf));
/*N*/ 		}
/*N*/ 
/*N*/ 		aLocalBoundVol = aBoundVol;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		aBoundVol = aLocalBoundVol;
/*N*/ 	}
/*N*/ 
/*N*/ 	bBoundVolValid = TRUE;
/*N*/ }

/*************************************************************************
|*
|* umschliessendes Volumen zurueckgeben und ggf. neu berechnen
|*
\************************************************************************/

/*N*/ const Volume3D& E3dObject::GetBoundVolume()
/*N*/ {
/*N*/ 	if ( !bBoundVolValid )
/*N*/ 		RecalcBoundVolume();
/*N*/ 
/*N*/ 	if(!aBoundVol.IsValid())
/*N*/ 		aBoundVol = Volume3D(Vector3D(), Vector3D());
/*N*/ 
/*N*/ 	return aBoundVol;
/*N*/ }

/*************************************************************************
|*
|* Mittelpunkt liefern
|*
\************************************************************************/

/*N*/ Vector3D E3dObject::GetCenter()
/*N*/ {
/*N*/ 	Volume3D aVolume = GetBoundVolume();
/*N*/ 	return (aVolume.MaxVec() + aVolume.MinVec()) / 2.0;
/*N*/ }

/*************************************************************************
|*
|* Aederung des BoundVolumes an alle Kindobjekte weitergeben
|*
\************************************************************************/

/*N*/ void E3dObject::SetBoundVolInvalid()
/*N*/ {
/*N*/ 	bBoundVolValid = FALSE;
/*N*/ 
/*N*/ 	E3dObjList* pOL = pSub;
/*N*/ 	ULONG nObjCnt = pOL->GetObjCount();
/*N*/ 
/*N*/ 	for (ULONG i = 0; i < nObjCnt; i++)
/*N*/ 	{
/*N*/ 		SdrObject* pObj = pOL->GetObj(i);
/*N*/ 		DBG_ASSERT(pObj->ISA(E3dObject), "In E3dObject sind nur 3D-Objekte erlaubt!");
/*N*/ 
/*N*/ 		((E3dObject*) pObj)->SetBoundVolInvalid();
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Aederung der Transformation an alle Kindobjekte weitergeben
|*
\************************************************************************/

/*N*/ void E3dObject::SetTransformChanged()
/*N*/ {
/*N*/ 	bTfHasChanged = TRUE;
/*N*/ 	bBoundVolValid = FALSE;
/*N*/ 
/*N*/ 	E3dObjList* pOL = pSub;
/*N*/ 	ULONG nObjCnt = pOL->GetObjCount();
/*N*/ 
/*N*/ 	for (ULONG i = 0; i < nObjCnt; i++)
/*N*/ 	{
/*N*/ 		SdrObject* pObj = pOL->GetObj(i);
/*N*/ 		DBG_ASSERT(pObj->ISA(E3dObject), "In E3dObject sind nur 3D-Objekte erlaubt!");
/*N*/ 
/*N*/ 		((E3dObject*) pObj)->SetTransformChanged();
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* hierarchische Transformation ueber alle Parents bestimmen und mit
|* der uebergebenen Matrix verketten
|*
\************************************************************************/


/*************************************************************************
|*
|* hierarchische Transformation ueber alle Parents bestimmen, in
|* aFullTfMatrix ablegen und diese zurueckgeben
|*
\************************************************************************/

/*N*/ const Matrix4D& E3dObject::GetFullTransform()
/*N*/ {
/*N*/ 	if ( bTfHasChanged )
/*N*/ 	{
/*N*/ 		aFullTfMatrix = aTfMatrix;
/*N*/ 
/*N*/ 		if ( GetParentObj() )
/*N*/ 			aFullTfMatrix *= GetParentObj()->GetFullTransform();
/*N*/ 
/*N*/ 		bTfHasChanged = FALSE;
/*N*/ 	}
/*N*/ 
/*N*/ 	return aFullTfMatrix;
/*N*/ }

/*************************************************************************
|*
|* Transformationsmatrix abfragen
|*
\************************************************************************/

/*N*/ const Matrix4D& E3dObject::GetTransform() const
/*N*/ {
/*N*/ 	return aTfMatrix;
/*N*/ }

/*************************************************************************
|*
|* Transformationsmatrix setzen
|*
\************************************************************************/

/*N*/ void E3dObject::NbcSetTransform(const Matrix4D& rMatrix)
/*N*/ {
/*N*/ 	aTfMatrix = rMatrix;
/*N*/ 	SetTransformChanged();
/*N*/ 	StructureChanged(this);
/*N*/ }

/*************************************************************************
|*
|* Transformationsmatrix auf Einheitsmatrix zuruecksetzen
|*
\************************************************************************/

/*N*/ void E3dObject::NbcResetTransform()
/*N*/ {
/*N*/ 	aTfMatrix.Identity();
/*N*/ 	SetTransformChanged();
/*N*/ 	StructureChanged(this);
/*N*/ }

/*************************************************************************
|*
|* Transformationsmatrix setzen mit Repaint-Broadcast
|*
\************************************************************************/

/*N*/ void E3dObject::SetTransform(const Matrix4D& rMatrix)
/*N*/ {
/*N*/ 	SendRepaintBroadcast();
/*N*/ 	NbcSetTransform(rMatrix);
/*N*/ 	SetChanged();
/*N*/ 	SendRepaintBroadcast();
/*N*/ 	if (pUserCall != NULL) pUserCall->Changed(*this, SDRUSERCALL_RESIZE, Rectangle());
/*N*/ }

/*************************************************************************
|*
|* Transformationsmatrix zuruecksetzen mit Repaint-Broadcast
|*
\************************************************************************/


/*************************************************************************
|*
|* Translation
|*
\************************************************************************/

/*************************************************************************
|*
|* Translation mit Repaint-Broadcast
|*
\************************************************************************/


/*************************************************************************
|*
|* Skalierungen
|*
\************************************************************************/


/************************************************************************/


/************************************************************************/


/************************************************************************/


/*************************************************************************
|*
|* gleichmaessige Skalierung
|*
\************************************************************************/


/*************************************************************************
|*
|* Skalierungen mit mit Repaint-Broadcast
|*
\************************************************************************/


/************************************************************************/


/************************************************************************/


/************************************************************************/


/************************************************************************/


/*************************************************************************
|*
|* Rotationen mit Winkel in Radiant
|*
\************************************************************************/


/************************************************************************/


/************************************************************************/


/*************************************************************************
|*
|* Rotationen mit Repaint-Broadcast
|*
\************************************************************************/


/************************************************************************/


/************************************************************************/


/*************************************************************************
|*
|* Objektbaum-Ebene des Objekts und aller Children setzen
|*
\************************************************************************/

/*N*/ void E3dObject::SetObjTreeLevel(USHORT nNewLevel)
/*N*/ {
/*N*/ 	nObjTreeLevel = nNewLevel;
/*N*/ 	nNewLevel++;
/*N*/ 
/*N*/ 	E3dObjList* pOL = pSub;
/*N*/ 	ULONG nObjCnt = pOL->GetObjCount();
/*N*/ 
/*N*/ 	for (ULONG i = 0; i < nObjCnt; i++)
/*N*/ 	{
/*N*/ 		SdrObject* pObj = pOL->GetObj(i);
/*N*/ 		DBG_ASSERT(pObj->ISA(E3dObject), "In E3dObject sind nur 3D-Objekte erlaubt!");
/*N*/ 
/*N*/ 		((E3dObject*) pObj)->SetObjTreeLevel(nNewLevel);
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* logische Gruppe setzen
|*
\************************************************************************/


/*************************************************************************
|*
|* Linien fuer die Wireframe-Darstellung des Objekts dem uebergebenen
|* Polygon3D hinzufuegen. Als default wird das BoundVolume verwendet.
|*
\************************************************************************/


/*************************************************************************
|*
|* Get the name of the object (singular)
|*
\************************************************************************/


/*************************************************************************
|*
|* Get the name of the object (plural)
|*
\************************************************************************/


/*************************************************************************
|*
|* Wireframe-XPolyPolygon erzeugen
|*
\************************************************************************/


/*************************************************************************
|*
|* Drag-Polygon zurueckgeben
|*
\************************************************************************/


/*************************************************************************
|*
|* Zuweisungsoperator
|*
\************************************************************************/


//////////////////////////////////////////////////////////////////////////////
// ItemSet access

/*N*/ const SfxItemSet& E3dObject::GetItemSet() const
/*N*/ {
/*N*/ 	// include Items of scene this object belongs to
/*N*/ 	E3dScene* pScene = GetScene();
/*N*/ 	if(pScene && pScene != this)
/*N*/ 	{
/*N*/ 		SfxItemSet& rSet = (SfxItemSet&)SdrAttrObj::GetItemSet();
/*N*/ 		SfxItemSet aSet(*rSet.GetPool(), SDRATTR_3DSCENE_FIRST, SDRATTR_3DSCENE_LAST);
/*N*/ 		aSet.Put(pScene->E3dObject::GetItemSet());
/*N*/ 		rSet.Put(aSet);
/*N*/ 	}
/*N*/ 
/*N*/ 	return SdrAttrObj::GetItemSet();
/*N*/ }

/*N*/ SfxItemSet* E3dObject::CreateNewItemSet(SfxItemPool& rPool)
/*N*/ {
/*N*/ 	// include ALL items, 2D and 3D
/*N*/ 	return new SfxItemSet(rPool,
/*N*/ 		// ranges from SdrAttrObj
/*N*/ 		SDRATTR_START, SDRATTRSET_SHADOW,
/*N*/ 		SDRATTRSET_OUTLINER, SDRATTRSET_MISC,
/*N*/ 
/*N*/ 		// ranges for 3D (object and scene)
/*N*/ 		SDRATTR_3D_FIRST, SDRATTR_3D_LAST,
/*N*/ 
/*N*/ 		// outliner and end
/*N*/ 		EE_ITEMS_START, EE_ITEMS_END,
/*N*/ 		0, 0);
/*N*/ }

// private support routines for ItemSet access. NULL pointer means clear item.
/*N*/ void E3dObject::ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem)
/*N*/ {
/*N*/ 	// propagate item changes to scene
/*N*/ 	if(!nWhich || (nWhich >= SDRATTR_3DSCENE_FIRST && nWhich <= SDRATTR_3DSCENE_LAST))
/*N*/ 	{
/*N*/ 		E3dScene* pScene = GetScene();
/*N*/ 		if(pScene && pScene != this)
/*N*/ 			pScene->E3dObject::ItemChange(nWhich, pNewItem);
/*N*/ 	}
/*N*/ 
/*N*/ 	// call parent
/*N*/ 	SdrAttrObj::ItemChange(nWhich, pNewItem);
/*N*/ }

// #107770# Like propagating ItemChange to the scene if scene items are changed,
// do the same with the PostItemChange calls.
/*N*/ void E3dObject::PostItemChange(const sal_uInt16 nWhich)
/*N*/ {
/*N*/ 	// propagate item changes to scene
/*N*/ 	if(!nWhich || (nWhich >= SDRATTR_3DSCENE_FIRST && nWhich <= SDRATTR_3DSCENE_LAST))
/*N*/ 	{
/*N*/ 		E3dScene* pScene = GetScene();
/*N*/ 		if(pScene && pScene != this)
/*N*/ 			pScene->PostItemChange(nWhich);
/*N*/ 	}
/*N*/ 
/*N*/ 	// call parent
/*N*/ 	SdrAttrObj::PostItemChange(nWhich);
/*N*/ }

/*N*/ void E3dObject::ItemSetChanged( const SfxItemSet& rSet )
/*N*/ {
/*N*/ 	// call parent
/*N*/ 	SdrAttrObj::ItemSetChanged( rSet );
/*N*/ 
/*N*/ 	// local changes
/*N*/ 	StructureChanged(this);
/*N*/ }

/*************************************************************************
|*
|* StyleSheet setzen
|*
\************************************************************************/

/*N*/ void E3dObject::NbcSetStyleSheet(SfxStyleSheet* pNewStyleSheet,
/*N*/ 	FASTBOOL bDontRemoveHardAttr)
/*N*/ {
/*N*/ 	// call parent
/*N*/ 	SdrAttrObj::NbcSetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);
/*N*/ 
/*N*/ 	E3dObjList* pOL = pSub;
/*N*/ 	ULONG nObjCnt = pOL->GetObjCount();
/*N*/ 	for ( ULONG i = 0; i < nObjCnt; i++ )
/*?*/ 		pOL->GetObj(i)->NbcSetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);
/*N*/ }

/*************************************************************************
|*
|* Nur die Member des E3dObjekts in den Stream speichern
|* Dies wird direkt auch von E3dSphere gerufen um zu verhindern dass die
|* Subliste weggeschrieben wird. (FG)
|*
\************************************************************************/

/*N*/ #ifndef SVX_LIGHT
/*N*/ void E3dObject::WriteOnlyOwnMembers(SvStream& rOut) const
/*N*/ {
/*N*/ 	// Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ 	SdrDownCompat aCompat(rOut, STREAM_WRITE);
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("E3dObjectOwnMembers");
/*N*/ #endif
/*N*/ 
/*N*/ 	rOut << aLocalBoundVol;
/*N*/ 
/*N*/ 	Old_Matrix3D aMat3D;
/*N*/ 	aMat3D = aTfMatrix;
/*N*/ 	rOut << aMat3D;
/*N*/ 
/*N*/ 	rOut << nLogicalGroup;
/*N*/ 	rOut << nObjTreeLevel;
/*N*/ 	rOut << nPartOfParent;
/*N*/ 	rOut << UINT16(eDragDetail);
/*N*/ }
/*N*/ #endif

/*************************************************************************
|*
|* Objektdaten in Stream speichern
|*
\************************************************************************/

/*N*/ void E3dObject::WriteData(SvStream& rOut) const
/*N*/ {
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	long position = rOut.Tell();
/*N*/ 	SdrAttrObj::WriteData(rOut);
/*N*/ 	position = rOut.Tell();
/*N*/ 
/*N*/ 	// Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ 	SdrDownCompat aCompat(rOut, STREAM_WRITE);
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("E3dObject");
/*N*/ #endif
/*N*/ 
/*N*/ 	position = rOut.Tell();
/*N*/ 	pSub->Save(rOut);
/*N*/ 	position = rOut.Tell();
/*N*/ 
/*N*/ 	if (rOut.GetVersion() < 3560)
/*N*/ 	{
/*N*/ 		rOut << aLocalBoundVol;
/*N*/ 
/*N*/ 		Old_Matrix3D aMat3D;
/*N*/ 		aMat3D = aTfMatrix;
/*N*/ 		rOut << aMat3D;
/*N*/ 
/*N*/ 		rOut << nLogicalGroup;
/*N*/ 		rOut << nObjTreeLevel;
/*N*/ 		rOut << nPartOfParent;
/*N*/ 		rOut << UINT16(eDragDetail);
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		WriteOnlyOwnMembers(rOut);
/*N*/ 	}
/*N*/ 	position = rOut.Tell();
/*N*/ #endif
/*N*/ }

/*************************************************************************
|*
|* Objektdaten aus Stream laden
|*
\************************************************************************/

/*N*/ void E3dObject::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
/*N*/ {
/*N*/ 	long position = rIn.Tell();
/*N*/ 	if (ImpCheckSubRecords (rHead, rIn))
/*N*/ 	{
/*N*/ 		position = rIn.Tell();
/*N*/ 		SdrAttrObj::ReadData(rHead, rIn);
/*N*/ 		position = rIn.Tell();
/*N*/ 		// Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ 		SdrDownCompat aCompat(rIn, STREAM_READ);
/*N*/ #ifdef DBG_UTIL
/*N*/ 		aCompat.SetID("E3dObject");
/*N*/ #endif
/*N*/ 		pSub->Load(rIn, *pPage);
/*N*/ 
/*N*/ 		position = rIn.Tell();
/*N*/ 		if ((rIn.GetVersion() < 3560) || (rHead.GetVersion() <= 12))
/*N*/ 		{
/*N*/ 			UINT16  nTmp16;
/*N*/ 
/*N*/ 			rIn >> aLocalBoundVol;
/*N*/ 
/*N*/ 			Old_Matrix3D aMat3D;
/*N*/ 			rIn >> aMat3D;
/*N*/ 			aTfMatrix = Matrix4D(aMat3D);
/*N*/ 
/*N*/ 			rIn >> nLogicalGroup;
/*N*/ 			rIn >> nObjTreeLevel;
/*N*/ 			rIn >> nPartOfParent;
/*N*/ 			rIn >> nTmp16; eDragDetail = E3dDragDetail(nTmp16);
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			ReadOnlyOwnMembers(rHead, rIn);
/*N*/ 		}
/*N*/ 		position = rIn.Tell();
/*N*/ 
/*N*/ 		// Wie ein veraendertes Objekt behandeln
/*N*/ 		SetTransformChanged();
/*N*/ 		StructureChanged(this);
/*N*/ 
/*N*/ 		// BoundVolume muss neu berechnet werden
/*N*/ 		bBoundVolValid = FALSE;
/*N*/ 
/*N*/ 		// SnapRect auch
/*N*/ 		bSnapRectDirty = TRUE;
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Nur die Daten des E3dObject aus Stream laden (nicht der Sublisten und
|* der Basisklassen). Wird von E3dSphere auch genutzt. (FileFormat-Optimierung)
|*
\************************************************************************/

/*N*/ void E3dObject::ReadOnlyOwnMembers(const SdrObjIOHeader& rHead, SvStream& rIn)
/*N*/ {
/*N*/ 	// Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ 	SdrDownCompat aCompat(rIn, STREAM_READ);
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("E3dObjectOwnMembers");
/*N*/ #endif
/*N*/ 	UINT16  nTmp16;
/*N*/ 
/*N*/ 	rIn >> aLocalBoundVol;
/*N*/ 
/*N*/ 	Old_Matrix3D aMat3D;
/*N*/ 	rIn >> aMat3D;
/*N*/ 	aTfMatrix = Matrix4D(aMat3D);
/*N*/ 
/*N*/ 	rIn >> nLogicalGroup;
/*N*/ 	rIn >> nObjTreeLevel;
/*N*/ 	rIn >> nPartOfParent;
/*N*/ 	rIn >> nTmp16; eDragDetail = E3dDragDetail(nTmp16);
/*N*/ 
/*N*/ 	bBoundVolValid = FALSE;
/*N*/ }


/*************************************************************************
|*
|* nach dem Laden...
|*
\************************************************************************/

/*N*/ void E3dObject::AfterRead()
/*N*/ {
/*N*/ 	SdrAttrObj::AfterRead();
/*N*/ 	if (pSub)
/*N*/ 		pSub->AfterRead();
/*N*/ }

/*************************************************************************
|*
|* erstelle neues GeoData-Objekt
|*
\************************************************************************/

/*N*/ SdrObjGeoData *E3dObject::NewGeoData() const
/*N*/ {
/*N*/ 	// Theoretisch duerfen auch nur Szenen ihre GeoDatas erstellen und verwalten !!
/*N*/ 	// AW: Dies stimmt nicht mehr, diese Stelle ist mit der neuen Engine OK!
/*N*/ 	return new E3DObjGeoData;
/*N*/ }

/*************************************************************************
|*
|* uebergebe aktuelle werte an das GeoData-Objekt
|*
\************************************************************************/

/*N*/ void E3dObject::SaveGeoData(SdrObjGeoData& rGeo) const
/*N*/ {
/*N*/ 	SdrAttrObj::SaveGeoData (rGeo);
/*N*/ 
/*N*/ 	((E3DObjGeoData &) rGeo).aLocalBoundVol  = aLocalBoundVol;
/*N*/ 	((E3DObjGeoData &) rGeo).aTfMatrix       = aTfMatrix;
/*N*/ }

/*************************************************************************
|*
|* uebernehme werte aus dem GeoData-Objekt
|*
\************************************************************************/

/*N*/ void E3dObject::RestGeoData(const SdrObjGeoData& rGeo)
/*N*/ {
/*N*/ 	aLocalBoundVol = ((E3DObjGeoData &) rGeo).aLocalBoundVol;
/*N*/ 	NbcSetTransform (((E3DObjGeoData &) rGeo).aTfMatrix);
/*N*/ 
/*N*/ 	SdrAttrObj::RestGeoData (rGeo);
/*N*/ 	GetScene()->FitSnapRectToBoundVol();
/*N*/ }

/*************************************************************************
|*
|* Pruefe, ob die SubRecords ok sind und mit der Factory gelesen werden
|* koennen.
|*
\************************************************************************/

/*N*/ BOOL E3dObject::ImpCheckSubRecords (const SdrObjIOHeader& rHead,
/*N*/ 									SvStream&             rIn)
/*N*/ {
/*N*/ 	BOOL bDoRead = FALSE;
/*N*/ 
/*N*/ 	if ( rIn.GetError() == SVSTREAM_OK )
/*N*/ 	{
/*N*/ 		if (rHead.GetVersion () <= 12)
/*N*/ 		{
/*N*/ 			ULONG nPos0 = rIn.Tell();
/*N*/ 			// Einen SubRecord ueberspringen (SdrObject)
/*N*/ 			{ SdrDownCompat aCompat(rIn,STREAM_READ); }
/*N*/ 			// Nocheinen SubRecord ueberspringen (SdrAttrObj)
/*N*/ 			{ SdrDownCompat aCompat(rIn,STREAM_READ); }
/*N*/ 			// Und nun muesste meiner kommen
/*N*/ 			bDoRead = rHead.GetBytesLeft() != 0;
/*N*/ 			rIn.Seek (nPos0); // FilePos wieder restaurieren
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			bDoRead = TRUE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return bDoRead;
/*N*/ }

/*************************************************************************
|*
|* Keine DefaultAttr, zu langsam
|*
\************************************************************************/

/*N*/ void E3dObject::ForceDefaultAttr()
/*N*/ {
/*N*/ }

/*************************************************************************/

/*N*/ TYPEINIT1(E3dCompoundObject, E3dObject);

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

/*N*/ E3dCompoundObject::E3dCompoundObject() : E3dObject()
/*N*/ {
/*N*/ 	// Defaults setzen
/*N*/ 	E3dDefaultAttributes aDefault;
/*N*/ 	SetDefaultAttributes(aDefault);
/*N*/ 
/*N*/ 	bBytesLeft = FALSE;
/*N*/ 	bCreateE3dPolyObj = FALSE;
/*N*/ 	bGeometryValid = FALSE;
/*N*/ }

/*N*/ E3dCompoundObject::E3dCompoundObject(E3dDefaultAttributes& rDefault) : E3dObject()
/*N*/ {
/*N*/ 	// Defaults setzen
/*N*/ 	SetDefaultAttributes(rDefault);
/*N*/ 
/*N*/ 	bBytesLeft = FALSE;
/*N*/ 	bCreateE3dPolyObj = FALSE;
/*N*/ 	bGeometryValid = FALSE;
/*N*/ }

/*N*/ void E3dCompoundObject::SetDefaultAttributes(E3dDefaultAttributes& rDefault)
/*N*/ {
/*N*/ 	// Defaults setzen
/*N*/ 	aMaterialAmbientColor = rDefault.GetDefaultAmbientColor();
/*N*/ 
//is always default, so it needs not to be set /*N*/ 	aBackMaterial = rDefault.GetDefaultBackMaterial();
/*N*/ 	bCreateNormals = rDefault.GetDefaultCreateNormals();
/*N*/ 	bCreateTexture = rDefault.GetDefaultCreateTexture();
/*N*/ 	bUseDifferentBackMaterial = rDefault.GetDefaultUseDifferentBackMaterial();
/*N*/ }

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

/*N*/ E3dCompoundObject::~E3dCompoundObject ()
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|* Start der Geometrieerzeugung ankuendigen
|*
\************************************************************************/

/*N*/ void E3dCompoundObject::StartCreateGeometry()
/*N*/ {
/*N*/ 	// Geometriestart mitteilen
/*N*/ 	aDisplayGeometry.StartDescription();
/*N*/ 
/*N*/ 	// Lokales Volumen reset
/*N*/ 	aLocalBoundVol = Volume3D();
/*N*/ 
/*N*/ 	// Geometrie ist ab jetzt gueltig, um ein rekursives weiteres
/*N*/ 	// Erzeugen zu verhindern
/*N*/ 	bGeometryValid = TRUE;
/*N*/ }

/*************************************************************************
|*
|* Identifier zurueckgeben
|*
\************************************************************************/

/*N*/ UINT16 E3dCompoundObject::GetObjIdentifier() const
/*N*/ {
/*N*/ 	return E3D_COMPOUNDOBJ_ID;
/*N*/ }

/*************************************************************************
|*
|* Compounds brauchen Defaults
|*
\************************************************************************/

/*N*/ void E3dCompoundObject::ForceDefaultAttr()
/*N*/ {
/*N*/ 	SdrAttrObj::ForceDefaultAttr();
/*N*/ }

/*************************************************************************
|*
|* SnapRect berechnen
|*
\************************************************************************/

/*N*/ void E3dCompoundObject::RecalcSnapRect()
/*N*/ {
/*N*/ 	E3dScene* pScene = GetScene();
/*N*/ 	if(pScene)
/*N*/ 	{
/*N*/ 		// Objekttransformation uebernehmen
/*N*/ 		const Volume3D& rBoundVol = GetBoundVolume();
/*N*/ 		maSnapRect = Rectangle();
/*N*/ 
/*N*/ 		if(rBoundVol.IsValid())
/*N*/ 		{
/*N*/ 			const Matrix4D& rTrans = GetFullTransform();
/*N*/ 			Vol3DPointIterator aIter(rBoundVol, &rTrans);
/*N*/ 			Vector3D aTfVec;
/*N*/ 			while ( aIter.Next(aTfVec) )
/*N*/ 			{
/*N*/ 				aTfVec = pScene->GetCameraSet().WorldToViewCoor(aTfVec);
/*N*/ 				Point aPoint((long)(aTfVec.X() + 0.5), (long)(aTfVec.Y() + 0.5));
/*N*/ 				maSnapRect.Union(Rectangle(aPoint, aPoint));
/*N*/ 			}
/*N*/ 		}
/*N*/ 		bSnapRectDirty = FALSE;
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* BoundRect berechnen und evtl. Schatten einbeziehen
|* Dazu muss ein eventueller Schatten des einzelnen 3D-Objektes
|* beruecksichtigt werden
|*
\************************************************************************/

/*N*/ void E3dCompoundObject::RecalcBoundRect()
/*N*/ {
/*N*/ 	// BoundRect aus SnapRect berechnen
/*N*/ 	aOutRect = GetSnapRect();
/*N*/ 
/*N*/ 	E3dScene* pScene = GetScene();
/*N*/ 	if(pScene)
/*N*/ 	{
/*N*/ 		// Schatten beruecksichtigen
/*N*/ 		if(DoDrawShadow())
/*N*/ 		{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 		}

        // Linienbreite beruecksichtigen
/*N*/ 		INT32 nLineWidth = ((const XLineWidthItem&)(GetItem(XATTR_LINEWIDTH))).GetValue();
/*N*/ 		if(nLineWidth)
/*N*/ 		{
/*?*/ 			Rectangle aShadowRect = aOutRect;
/*?*/ 			aShadowRect.Left() -= nLineWidth;
/*?*/ 			aShadowRect.Right() += nLineWidth;
/*?*/ 			aShadowRect.Top() -= nLineWidth;
/*?*/ 			aShadowRect.Bottom() += nLineWidth;
/*?*/ 			aOutRect.Union(aShadowRect);
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* BoundVolume holen. Falls die Geometrie ungueltig ist, diese neu
|* erzeugen und das BoundVol neu berechnen
|*
\************************************************************************/

/*N*/ const Volume3D& E3dCompoundObject::GetBoundVolume()
/*N*/ {
/*N*/ 	// Geometrie aktuell?
/*N*/ 	if(!bGeometryValid)
/*N*/ 	{
/*N*/ 		// Neu erzeugen und eine Neubestimmung des BoundVol erzwingen
/*N*/ 		ReCreateGeometry();
/*N*/ 		bBoundVolValid = FALSE;
/*N*/ 	}
/*N*/ 
/*N*/ 	// call parent
/*N*/ 	return E3dObject::GetBoundVolume();
/*N*/ }

/*************************************************************************
|*
|* Rausschreiben der Datenmember eines E3dCompounds
|*
\************************************************************************/

/*N*/ void E3dCompoundObject::WriteData(SvStream& rOut) const
/*N*/ {
/*N*/ #ifndef SVX_LIGHT
/*N*/ #ifdef E3D_STREAMING
/*N*/ 
/*N*/ 	if (!aLocalBoundVol.IsValid() && aBoundVol.IsValid())
/*N*/ 	{
/*N*/ 		// Das aLocalBoundVol wird gespeichert.
/*N*/ 		// Ist dieses ungueltig, so wird das aBoundVol genommen
/*N*/ 		// (sollten beim E3dCompoundObject sowieso gleich sein)
/*N*/ 		((E3dCompoundObject*) this)->aLocalBoundVol = aBoundVol;
/*N*/ 	}
/*N*/ 
/*N*/ 	E3dObject::WriteData(rOut);
/*N*/ 	if (rOut.GetVersion() < 3560)
/*N*/ 	{
/*N*/ 		// In diesem Fall passiert nichts, da vor der Version 4.0
/*N*/ 		// also im Falle der Revision 3.1
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		SdrDownCompat aCompat(rOut, STREAM_WRITE);
/*N*/ #ifdef DBG_UTIL
/*N*/ 		aCompat.SetID("E3dCompoundObject");
/*N*/ #endif
/*N*/ 		rOut << BOOL(GetDoubleSided());
/*N*/ #endif
/*N*/ 
/*N*/ 		// neue Parameter zur Geometrieerzeugung
/*N*/ 		rOut << BOOL(bCreateNormals);
/*N*/ 		rOut << BOOL(bCreateTexture);
/*N*/ 
/*N*/ 		sal_uInt16 nVal = GetNormalsKind();
/*N*/ 		rOut << BOOL(nVal > 0);
/*N*/ 		rOut << BOOL(nVal > 1);
/*N*/ 
/*N*/ 		nVal = GetTextureProjectionX();
/*N*/ 		rOut << BOOL(nVal > 0);
/*N*/ 		rOut << BOOL(nVal > 1);
/*N*/ 
/*N*/ 		nVal = GetTextureProjectionY();
/*N*/ 		rOut << BOOL(nVal > 0);
/*N*/ 		rOut << BOOL(nVal > 1);
/*N*/ 
/*N*/ 		rOut << BOOL(GetShadow3D());
/*N*/ 
/*N*/ 		// neu al 384:
/*N*/ 		rOut << GetMaterialAmbientColor();
/*N*/ 		rOut << GetMaterialColor();
/*N*/ 		rOut << GetMaterialSpecular();
/*N*/ 		rOut << GetMaterialEmission();
/*N*/ 		rOut << GetMaterialSpecularIntensity();
/*N*/ 
/*N*/ 		aBackMaterial.WriteData(rOut);
/*N*/ 
/*N*/ 		rOut << (UINT16)GetTextureKind();
/*N*/ 
/*N*/ 		rOut << (UINT16)GetTextureMode();
/*N*/ 
/*N*/ 		rOut << BOOL(GetNormalsInvert());
/*N*/ 
/*N*/ 		// neu ab 534: (hat noch gefehlt)
/*N*/ 		rOut << BOOL(GetTextureFilter());
/*N*/ 	}
/*N*/ #endif
/*N*/ }

/*************************************************************************
|*
|* Einlesen der Datenmember eines E3dCompounds
|*
\************************************************************************/

/*N*/ void E3dCompoundObject::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
/*N*/ {
/*N*/ 	if ( rIn.GetError() != SVSTREAM_OK )
/*N*/ 		return;
/*N*/ 
/*N*/ 	E3dObject::ReadData(rHead, rIn);
/*N*/ 
/*N*/ 	  // Vor der Filerevision 13 wurde das Objekt nie geschrieben.
/*N*/ 	  // auch kein Kompatibilitaetsrecord.
/*N*/ 	if ((rHead.GetVersion() < 13) || (rIn.GetVersion() < 3560))
/*N*/ 	{
/*N*/ 		return;
/*N*/ 	}
/*N*/ 
/*N*/ 	SdrDownCompat aCompat(rIn, STREAM_READ);
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("E3dCompoundObject");
/*N*/ #endif
/*N*/ 
/*N*/ 	bBytesLeft = FALSE;
/*N*/ 	if (aCompat.GetBytesLeft () >= sizeof (BOOL))
/*N*/ 	{
/*N*/ 		BOOL bTmp, bTmp2;
/*N*/ 		sal_uInt16 nTmp;
/*N*/ 
/*N*/ 		rIn >> bTmp; 
/*N*/ 		mpObjectItemSet->Put(Svx3DDoubleSidedItem(bTmp));
/*N*/ 
/*N*/ 		// neue Parameter zur Geometrieerzeugung
/*N*/ 		if (aCompat.GetBytesLeft () >= sizeof (BOOL))
/*N*/ 		{
/*N*/ 			rIn >> bTmp; 
/*N*/ 			bCreateNormals = bTmp;
/*N*/ 
/*N*/ 			rIn >> bTmp; 
/*N*/ 			bCreateTexture = bTmp;
/*N*/ 
/*N*/ 			rIn >> bTmp; 
/*N*/ 			rIn >> bTmp2;
/*N*/ 			if(bTmp == FALSE && bTmp2 == FALSE)
/*N*/ 				nTmp = 0;
/*N*/ 			else if(bTmp == TRUE && bTmp2 == FALSE)
/*N*/ 				nTmp = 1;
/*N*/ 			else
/*N*/ 				nTmp = 2;
/*N*/ 			mpObjectItemSet->Put(Svx3DNormalsKindItem(nTmp));
/*N*/ 
/*N*/ 			rIn >> bTmp; 
/*N*/ 			rIn >> bTmp2;
/*N*/ 			if(bTmp == FALSE && bTmp2 == FALSE)
/*N*/ 				nTmp = 0;
/*N*/ 			else if(bTmp == TRUE && bTmp2 == FALSE)
/*N*/ 				nTmp = 1;
/*N*/ 			else
/*N*/ 				nTmp = 2;
/*N*/ 			mpObjectItemSet->Put(Svx3DTextureProjectionXItem(nTmp));
/*N*/ 
/*N*/ 			rIn >> bTmp; 
/*N*/ 			rIn >> bTmp2;
/*N*/ 			if(bTmp == FALSE && bTmp2 == FALSE)
/*N*/ 				nTmp = 0;
/*N*/ 			else if(bTmp == TRUE && bTmp2 == FALSE)
/*N*/ 				nTmp = 1;
/*N*/ 			else
/*N*/ 				nTmp = 2;
/*N*/ 			mpObjectItemSet->Put(Svx3DTextureProjectionYItem(nTmp));
/*N*/ 
/*N*/ 			rIn >> bTmp; 
/*N*/ 			mpObjectItemSet->Put(Svx3DShadow3DItem(bTmp));
/*N*/ 
/*N*/ 			// Setze ein Flag fuer den Aufrufer, dass neues Format
/*N*/ 			// zu lesen ist
/*N*/ 			bBytesLeft = TRUE;
/*N*/ 		}
/*N*/ 
/*N*/ 		// neu al 384:
/*N*/ 		if (aCompat.GetBytesLeft () >= sizeof (B3dMaterial))
/*N*/ 		{
/*N*/ 			UINT16 nTmp;
/*N*/ 
/*N*/ 			Color aCol;
/*N*/ 			
/*N*/ 			rIn >> aCol;
/*N*/ 			SetMaterialAmbientColor(aCol);
/*N*/ 			
/*N*/ 			rIn >> aCol;
/*N*/ 			// do NOT use, this is the old 3D-Color(!)
/*N*/ 			// SetItem(XFillColorItem(String(), aCol));
/*N*/ 			
/*N*/ 			rIn >> aCol;
/*N*/ 			mpObjectItemSet->Put(Svx3DMaterialSpecularItem(aCol));
/*N*/ 			
/*N*/ 			rIn >> aCol;
/*N*/ 			mpObjectItemSet->Put(Svx3DMaterialEmissionItem(aCol));
/*N*/ 			
/*N*/ 			rIn >> nTmp;
/*N*/ 			mpObjectItemSet->Put(Svx3DMaterialSpecularIntensityItem(nTmp));
/*N*/ 
/*N*/ 			aBackMaterial.ReadData(rIn);
/*N*/ 			
/*N*/ 			rIn >> nTmp; 
/*N*/ 			mpObjectItemSet->Put(Svx3DTextureKindItem(nTmp));
/*N*/ 
/*N*/ 			rIn >> nTmp; 
/*N*/ 			mpObjectItemSet->Put(Svx3DTextureModeItem(nTmp));
/*N*/ 
/*N*/ 			rIn >> bTmp; 
/*N*/ 			mpObjectItemSet->Put(Svx3DNormalsInvertItem(bTmp));
/*N*/ 		}
/*N*/ 
/*N*/ 		// neu ab 534: (hat noch gefehlt)
/*N*/ 		if (aCompat.GetBytesLeft () >= sizeof (BOOL))
/*N*/ 		{
/*N*/ 			rIn >> bTmp; 
/*N*/ 			mpObjectItemSet->Put(Svx3DTextureFilterItem(bTmp));
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Bitmaps fuer 3D-Darstellung von Gradients und Hatches holen
|*
\************************************************************************/




/*************************************************************************
|*
|* Give out simple line geometry
|*
\************************************************************************/


/*************************************************************************
|*
|* Geometrieerzeugung
|*
\************************************************************************/

/*N*/ void E3dCompoundObject::DestroyGeometry()
/*N*/ {
/*N*/ 	// Alle Objekte in der Sub-Liste zerstoeren. Dies sind die
/*N*/ 	// zur Visualisierung des Objektes verwendeten Hilfsobjekte
/*N*/ 	pSub->Clear();
/*N*/ 	delete pSub;
/*N*/ 	pSub = new E3dObjList(NULL, NULL);
/*N*/ 	pSub->SetOwnerObj(this);
/*N*/ 	pSub->SetListKind(SDROBJLIST_GROUPOBJ);
/*N*/ 
/*N*/ 	// Neue Geometrie zerstoeren
/*N*/ 	aDisplayGeometry.Erase();
/*N*/ 
/*N*/ 	// BoundVols resetten
/*N*/ 	aLocalBoundVol  = Volume3D();
/*N*/ 	bBoundVolValid = FALSE;
/*N*/ 	StructureChanged(this);
/*N*/ 
/*N*/ 	// Geometrie ist ungueltig
/*N*/ 	bGeometryValid = FALSE;
/*N*/ }

/*N*/ void E3dCompoundObject::CreateGeometry()
/*N*/ {
/*N*/ 	// Geometrie ist gueltig, um rekursion zu verhindern
/*N*/ 	bGeometryValid = TRUE;
/*N*/ 
/*N*/ 	// Eventuell entstandene Geometrie noch korrigieren
/*N*/ 	// und Default -Normalen oder -Texturkoordinaten erzeugen
/*N*/ 	if(bCreateNormals)
/*N*/ 	{
/*N*/ 		if(GetNormalsKind() > 1)
/*?*/ 			GetDisplayGeometry().CreateDefaultNormalsSphere();
/*N*/ 		if(GetNormalsInvert())
/*?*/ 			GetDisplayGeometry().InvertNormals();
/*N*/ 	}
/*N*/ 
/*N*/ 	if(bCreateTexture)
/*N*/ 	{
/*N*/ 		GetDisplayGeometry().CreateDefaultTexture(
/*N*/ 			((GetTextureProjectionX() > 0) ? B3D_CREATE_DEFAULT_X : FALSE)
/*N*/ 			|((GetTextureProjectionY() > 0) ? B3D_CREATE_DEFAULT_Y : FALSE),
/*N*/ 			GetTextureProjectionX() > 1);
/*N*/ 	}
/*N*/ 
/*N*/ 	// Am Ende der Geometrieerzeugung das model an den erzeugten
/*N*/ 	// PolyObj's setzen, d.h. beim ueberladen dieser Funktion
/*N*/ 	// den parent am Ende rufen.
/*N*/ 	if(bCreateE3dPolyObj)
/*N*/ 		SetModel(pModel);
/*N*/ 
/*N*/ 	// Das Ende der Geometrieerzeugung anzeigen
/*N*/ 	aDisplayGeometry.EndDescription();
/*N*/ }

/*N*/ void E3dCompoundObject::ReCreateGeometry(BOOL bCreateOldGeometry)
/*N*/ {
/*N*/ 	// Geometrie zerstoeren
/*N*/ 	DestroyGeometry();
/*N*/ 
/*N*/ 	// Flag fuer Geometrieerzeugung setzen
/*N*/ 	bCreateE3dPolyObj = bCreateOldGeometry;
/*N*/ 
/*N*/ 	// ... und neu erzeugen
/*N*/ 	CreateGeometry();
/*N*/ }

/*************************************************************************
|*
|* Geometrieerzeugung
|*
\************************************************************************/

/*N*/ void E3dCompoundObject::AddGeometry(const PolyPolygon3D& rPolyPolygon3D,
/*N*/ 	BOOL bHintIsComplex, BOOL bOutline)
/*N*/ {
/*N*/ 	if(rPolyPolygon3D.Count())
/*N*/ 	{
/*N*/ 		// eventuell alte Geometrie erzeugen (z.B. zum speichern)
/*N*/ 		if(bCreateE3dPolyObj)
/*N*/ 		{
/*N*/ 			E3dPolyObj* pObj = new E3dPolyObj(
/*N*/ 				rPolyPolygon3D, GetDoubleSided(), TRUE);
/*N*/ 			pObj->SetPartOfParent();
/*N*/ 			Insert3DObj(pObj);
/*N*/ 		}
/*N*/ 
/*N*/ 		// neue Geometrie erzeugen
/*N*/ 		for(USHORT a = 0; a < rPolyPolygon3D.Count(); a++ )
/*N*/ 		{
/*N*/ 			const Polygon3D& rPoly3D = rPolyPolygon3D[a];
/*N*/ 			aDisplayGeometry.StartObject(bHintIsComplex, bOutline);
/*N*/ 			for(USHORT b = 0; b < rPoly3D.GetPointCount(); b++ )
/*N*/ 				aDisplayGeometry.AddEdge(rPoly3D[b]);
/*N*/ 		}
/*N*/ 		aDisplayGeometry.EndObject();
/*N*/ 
/*N*/ 		// LocalBoundVolume pflegen
/*N*/ 		aLocalBoundVol.Union(rPolyPolygon3D.GetPolySize());
/*N*/ 
/*N*/ 		// Eigenes BoundVol nicht mehr gueltig
/*N*/ 		SetBoundVolInvalid();
/*N*/ 		SetRectsDirty();
/*N*/ 	}
/*N*/ }

/*?*/ void E3dCompoundObject::AddGeometry(
/*?*/ 	const PolyPolygon3D& rPolyPolygon3D,
/*?*/ 	const PolyPolygon3D& rPolyNormal3D,
/*?*/ 	BOOL bHintIsComplex, BOOL bOutline)
/*?*/ {
/*?*/ 	if(rPolyPolygon3D.Count())
/*?*/ 	{
/*?*/ 		// eventuell alte Geometrie erzeugen (z.B. zum speichern)
/*?*/ 		if(bCreateE3dPolyObj)
/*?*/ 		{
/*?*/ 			E3dPolyObj* pObj = new E3dPolyObj(
/*?*/ 				rPolyPolygon3D, rPolyNormal3D, GetDoubleSided(), TRUE);
/*?*/ 			pObj->SetPartOfParent();
/*?*/ 			Insert3DObj(pObj);
/*?*/ 		}
/*?*/ 
/*?*/ 		// neue Geometrie erzeugen
/*?*/ 		for(USHORT a = 0; a < rPolyPolygon3D.Count(); a++ )
/*?*/ 		{
/*?*/ 			const Polygon3D& rPoly3D = rPolyPolygon3D[a];
/*?*/ 			const Polygon3D& rNormal3D = rPolyNormal3D[a];
/*?*/ 			aDisplayGeometry.StartObject(bHintIsComplex, bOutline);
/*?*/ 			for(USHORT b = 0; b < rPoly3D.GetPointCount(); b++ )
/*?*/ 				aDisplayGeometry.AddEdge(rPoly3D[b], rNormal3D[b]);
/*?*/ 		}
/*?*/ 		aDisplayGeometry.EndObject();
/*?*/ 
/*?*/ 		// LocalBoundVolume pflegen
/*?*/ 		aLocalBoundVol.Union(rPolyPolygon3D.GetPolySize());
/*?*/ 
/*?*/ 		// Eigenes BoundVol nicht mehr gueltig
/*?*/ 		SetBoundVolInvalid();
/*?*/ 		SetRectsDirty();
/*?*/ 	}
/*?*/ }

/*N*/ void E3dCompoundObject::AddGeometry(
/*N*/ 	const PolyPolygon3D& rPolyPolygon3D,
/*N*/ 	const PolyPolygon3D& rPolyNormal3D,
/*N*/ 	const PolyPolygon3D& rPolyTexture3D,
/*N*/ 	BOOL bHintIsComplex, BOOL bOutline)
/*N*/ {
/*N*/ 	if(rPolyPolygon3D.Count())
/*N*/ 	{
/*N*/ 		// eventuell alte Geometrie erzeugen (z.B. zum speichern)
/*N*/ 		if(bCreateE3dPolyObj)
/*N*/ 		{
/*N*/ 			E3dPolyObj* pObj = new E3dPolyObj(
/*N*/ 				rPolyPolygon3D, rPolyNormal3D,
/*N*/ 				rPolyTexture3D, GetDoubleSided(), TRUE);
/*N*/ 			pObj->SetPartOfParent();
/*N*/ 			Insert3DObj(pObj);
/*N*/ 		}
/*N*/ 
/*N*/ 		// neue Geometrie erzeugen
/*N*/ 		for(USHORT a = 0; a < rPolyPolygon3D.Count(); a++ )
/*N*/ 		{
/*N*/ 			const Polygon3D& rPoly3D = rPolyPolygon3D[a];
/*N*/ 			const Polygon3D& rNormal3D = rPolyNormal3D[a];
/*N*/ 			const Polygon3D& rTexture3D = rPolyTexture3D[a];
/*N*/ 			aDisplayGeometry.StartObject(bHintIsComplex, bOutline);
/*N*/ 			for(USHORT b = 0; b < rPoly3D.GetPointCount(); b++ )
/*N*/ 				aDisplayGeometry.AddEdge(rPoly3D[b], rNormal3D[b], rTexture3D[b]);
/*N*/ 		}
/*N*/ 		aDisplayGeometry.EndObject();
/*N*/ 
/*N*/ 		// LocalBoundVolume pflegen
/*N*/ 		aLocalBoundVol.Union(rPolyPolygon3D.GetPolySize());
/*N*/ 
/*N*/ 		// Eigenes BoundVol nicht mehr gueltig
/*N*/ 		SetBoundVolInvalid();
/*N*/ 		SetRectsDirty();
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Hilfsfunktionen zur Geometrieerzeugung
|*
\************************************************************************/

/*N*/ void E3dCompoundObject::RotatePoly(
/*N*/ 	PolyPolygon3D& rPolyPolyRotate,
/*N*/ 	Matrix4D& rRotMat)
/*N*/ {
/*N*/ 	USHORT nPolyCnt = rPolyPolyRotate.Count();
/*N*/ 
/*N*/ 	for(UINT16 a=0;a<nPolyCnt;a++)
/*N*/ 	{
/*N*/ 		Polygon3D& rPolyRotate = rPolyPolyRotate[a];
/*N*/ 		USHORT nPntCnt = rPolyRotate.GetPointCount();
/*N*/ 
/*N*/ 		for(UINT16 b=0;b<nPntCnt;b++)
/*N*/ 			rPolyRotate[b] *= rRotMat;
/*N*/ 	}
/*N*/ }

/*N*/ void E3dCompoundObject::GrowPoly(
/*N*/ 	PolyPolygon3D& rPolyPolyGrow,
/*N*/ 	PolyPolygon3D& rPolyPolyNormals,
/*N*/ 	double fFactor)
/*N*/ {
/*N*/ 	USHORT nPolyCnt = rPolyPolyGrow.Count();
/*N*/ 
/*N*/ 	for(UINT16 a=0;a<nPolyCnt;a++)
/*N*/ 	{
/*N*/ 		Polygon3D& rPolyGrow = rPolyPolyGrow[a];
/*N*/ 		const Polygon3D& rPolyNormals = rPolyPolyNormals[a];
/*N*/ 		USHORT nPntCnt = rPolyGrow.GetPointCount();
/*N*/ 
/*N*/ 		for(UINT16 b=0;b<nPntCnt;b++)
/*N*/ 			rPolyGrow[b] += rPolyNormals[b] * fFactor;
/*N*/ 	}
/*N*/ }


/*N*/ void E3dCompoundObject::ScalePoly(
/*N*/ 	PolyPolygon3D& rPolyPolyScale,
/*N*/ 	double fFactor)
/*N*/ {
/*N*/ 	USHORT nPolyCnt = rPolyPolyScale.Count();
/*N*/ 	Vector3D aMiddle = rPolyPolyScale.GetMiddle();
/*N*/ 
/*N*/ 	for(UINT16 a=0;a<nPolyCnt;a++)
/*N*/ 	{
/*N*/ 		Polygon3D& rPolyScale = rPolyPolyScale[a];
/*N*/ 		USHORT nPntCnt = rPolyScale.GetPointCount();
/*N*/ 
/*N*/ 		for(UINT16 b=0;b<nPntCnt;b++)
/*N*/ 			rPolyScale[b] = ((rPolyScale[b] - aMiddle) * fFactor) + aMiddle;
/*N*/ 	}
/*N*/ }

/*N*/ void E3dCompoundObject::CreateFront(
/*N*/ 	const PolyPolygon3D& rPolyPoly3D,
/*N*/ 	const PolyPolygon3D& rFrontNormals,
/*N*/ 	BOOL bCreateNormals,
/*N*/ 	BOOL bCreateTexture)
/*N*/ {
/*N*/ 	// Vorderseite
/*N*/ 	if(bCreateNormals)
/*N*/ 	{
/*N*/ 		if(bCreateTexture)
/*N*/ 		{
/*N*/ 			// Polygon fuer die Textur erzeugen
/*N*/ 			PolyPolygon3D aPolyTexture = rPolyPoly3D;
/*N*/ 			Volume3D aSize = aPolyTexture.GetPolySize();
/*N*/ 			Matrix4D aTrans;
/*N*/ 
/*N*/ 			aTrans.Identity();
/*N*/ 			aTrans.Translate(-aSize.MinVec());
/*N*/ 			aPolyTexture.Transform(aTrans);
/*N*/ 
/*N*/ 			double fFactorX(1.0), fFactorY(1.0), fFactorZ(1.0);
/*N*/ 
/*N*/ 			if(aSize.GetWidth() != 0.0)
/*N*/ 				fFactorX = 1.0 / aSize.GetWidth();
/*N*/ 
/*N*/ 			if(aSize.GetHeight() != 0.0)
/*N*/ 				fFactorY = 1.0 / aSize.GetHeight();
/*N*/ 
/*N*/ 			if(aSize.GetDepth() != 0.0)
/*N*/ 				fFactorZ = 1.0 / aSize.GetDepth();
/*N*/ 
/*N*/ 			aTrans.Identity();
/*N*/ 			aTrans.Scale(fFactorX, -fFactorY, fFactorZ);
/*N*/ 			aTrans.Translate(Vector3D(0.0, 1.0, 0.0));
/*N*/ 			aPolyTexture.Transform(aTrans);
/*N*/ 
/*N*/ 			AddGeometry(rPolyPoly3D, rFrontNormals, aPolyTexture, TRUE);
/*N*/ 		}
/*N*/ 		else
/*N*/ 			AddGeometry(rPolyPoly3D, rFrontNormals, TRUE);
/*N*/ 	}
/*N*/ 	else
/*N*/ 		AddGeometry(rPolyPoly3D, TRUE);
/*N*/ }

/*N*/ void E3dCompoundObject::AddFrontNormals(
/*N*/ 	const PolyPolygon3D& rPolyPoly3D,
/*N*/ 	PolyPolygon3D& rNormalsFront,
/*N*/ 	Vector3D &rOffset)
/*N*/ {
/*N*/ 	Vector3D aFrontNormal = -rOffset;
/*N*/ 	aFrontNormal.Normalize();
/*N*/ 	USHORT nPolyCnt = rPolyPoly3D.Count();
/*N*/ 
/*N*/ 	for(UINT16 a=0;a<nPolyCnt;a++)
/*N*/ 	{
/*N*/ 		const Polygon3D& rPoly3D = rPolyPoly3D[a];
/*N*/ 		Polygon3D& rNormalPoly = rNormalsFront[a];
/*N*/ 		USHORT nPntCnt = rPoly3D.GetPointCount();
/*N*/ 
/*N*/ 		for(UINT16 b=0;b<nPntCnt;b++)
/*N*/ 		{
/*N*/ 			rNormalPoly[b] += aFrontNormal;
/*N*/ 			rNormalPoly[b].Normalize();
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void E3dCompoundObject::CreateBack(
/*N*/ 	const PolyPolygon3D& rPolyPoly3D,
/*N*/ 	const PolyPolygon3D& rBackNormals,
/*N*/ 	BOOL bCreateNormals,
/*N*/ 	BOOL bCreateTexture)
/*N*/ {
/*N*/ 	// PolyPolygon umdrehen
/*N*/ 	PolyPolygon3D aLocalPoly = rPolyPoly3D;
/*N*/ 	aLocalPoly.FlipDirections();
/*N*/ 
/*N*/ 	// Rueckseite
/*N*/ 	if(bCreateNormals)
/*N*/ 	{
/*N*/ 		PolyPolygon3D aLocalNormals = rBackNormals;
/*N*/ 		aLocalNormals.FlipDirections();
/*N*/ 		if(bCreateTexture)
/*N*/ 		{
/*N*/ 			// Polygon fuer die Textur erzeugen
/*N*/ 			PolyPolygon3D aPolyTexture(aLocalPoly);
/*N*/ 			Volume3D aSize = aPolyTexture.GetPolySize();
/*N*/ 			Matrix4D aTrans;
/*N*/ 
/*N*/ 			aTrans.Identity();
/*N*/ 			aTrans.Translate(-aSize.MinVec());
/*N*/ 			aPolyTexture.Transform(aTrans);
/*N*/ 
/*N*/ 			double fFactorX(1.0), fFactorY(1.0), fFactorZ(1.0);
/*N*/ 
/*N*/ 			if(aSize.GetWidth() != 0.0)
/*N*/ 				fFactorX = 1.0 / aSize.GetWidth();
/*N*/ 
/*N*/ 			if(aSize.GetHeight() != 0.0)
/*N*/ 				fFactorY = 1.0 / aSize.GetHeight();
/*N*/ 
/*N*/ 			if(aSize.GetDepth() != 0.0)
/*N*/ 				fFactorZ = 1.0 / aSize.GetDepth();
/*N*/ 
/*N*/ 			aTrans.Identity();
/*N*/ 			aTrans.Scale(fFactorX, -fFactorY, fFactorZ);
/*N*/ 			aTrans.Translate(Vector3D(0.0, 1.0, 0.0));
/*N*/ 			aPolyTexture.Transform(aTrans);
/*N*/ 
/*N*/ 			AddGeometry(aLocalPoly, aLocalNormals, aPolyTexture, TRUE);
/*N*/ 		}
/*N*/ 		else
/*N*/ 			AddGeometry(aLocalPoly, aLocalNormals, TRUE);
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		AddGeometry(aLocalPoly, TRUE);
/*N*/ 	}
/*N*/ }

/*N*/ void E3dCompoundObject::AddBackNormals(
/*N*/ 	const PolyPolygon3D& rPolyPoly3D,
/*N*/ 	PolyPolygon3D& rNormalsBack,
/*N*/ 	Vector3D& rOffset)
/*N*/ {
/*N*/ 	Vector3D aBackNormal = rOffset;
/*N*/ 	aBackNormal.Normalize();
/*N*/ 	USHORT nPolyCnt = rPolyPoly3D.Count();
/*N*/ 
/*N*/ 	for(UINT16 a=0;a<nPolyCnt;a++)
/*N*/ 	{
/*N*/ 		const Polygon3D& rPoly3D = rPolyPoly3D[a];
/*N*/ 		Polygon3D& rNormalPoly = rNormalsBack[a];
/*N*/ 		USHORT nPntCnt = rPoly3D.GetPointCount();
/*N*/ 
/*N*/ 		for(UINT16 b=0;b<nPntCnt;b++)
/*N*/ 		{
/*N*/ 			rNormalPoly[b] += aBackNormal;
/*N*/ 			rNormalPoly[b].Normalize();
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void E3dCompoundObject::CreateInBetween(
/*N*/ 	const PolyPolygon3D& rPolyPolyFront,
/*N*/ 	const PolyPolygon3D& rPolyPolyBack,
/*N*/ 	const PolyPolygon3D& rFrontNormals,
/*N*/ 	const PolyPolygon3D& rBackNormals,
/*N*/ 	BOOL bCreateNormals,
/*N*/ 	double fSurroundFactor,
/*N*/ 	double fTextureStart,
/*N*/ 	double fTextureDepth,
/*N*/ 	BOOL bRotateTexture90)
/*N*/ {
/*N*/ 	USHORT nPolyCnt = rPolyPolyFront.Count();
/*N*/ 	BOOL bCreateTexture = (fTextureDepth == 0.0) ? FALSE : TRUE;
/*N*/ 	double fPolyLength, fPolyPos;
/*N*/ 	USHORT nLastIndex;
/*N*/ 
/*N*/ 	// Verbindungsstuecke
/*N*/ 	if(bCreateNormals)
/*N*/ 	{
/*N*/ 		for(UINT16 a=0;a<nPolyCnt;a++)
/*N*/ 		{
/*N*/ 			const Polygon3D& rPoly3DFront = rPolyPolyFront[a];
/*N*/ 			const Polygon3D& rPoly3DBack = rPolyPolyBack[a];
/*N*/ 
/*N*/ 			const Polygon3D& rPolyNormalsFront = rFrontNormals[a];
/*N*/ 			const Polygon3D& rPolyNormalsBack = rBackNormals[a];
/*N*/ 
/*N*/ 			Polygon3D	aRect3D(4);
/*N*/ 			Polygon3D	aNormal3D(4);
/*N*/ 			Polygon3D	aTexture3D(4);
/*N*/ 			USHORT nPntCnt = rPoly3DFront.GetPointCount();
/*N*/ 			USHORT nPrefillIndex = rPoly3DFront.IsClosed() ? nPntCnt - 1 : 0;
/*N*/ 
/*N*/ 			aRect3D[3] = rPoly3DFront[nPrefillIndex];
/*N*/ 			aRect3D[2] = rPoly3DBack[nPrefillIndex];
/*N*/ 			aNormal3D[3] = rPolyNormalsFront[nPrefillIndex];
/*N*/ 			aNormal3D[2] = rPolyNormalsBack[nPrefillIndex];
/*N*/ 
/*N*/ 			if(bCreateTexture)
/*N*/ 			{
/*N*/ 				fPolyLength = rPoly3DFront.GetLength();
/*N*/ 				fPolyPos = 0.0;
/*N*/ 				nLastIndex = rPoly3DFront.IsClosed() ? nPntCnt - 1 : 0;
/*N*/ 
/*N*/ 				if(bRotateTexture90)
/*N*/ 				{
/*N*/ 					// X,Y vertauschen
/*N*/ 					aTexture3D[3].X() = fTextureStart;
/*N*/ 					aTexture3D[3].Y() = (1.0 - fPolyPos) * fSurroundFactor;
/*N*/ 
/*N*/ 					aTexture3D[2].X() = fTextureStart + fTextureDepth;
/*N*/ 					aTexture3D[2].Y() = (1.0 - fPolyPos) * fSurroundFactor;
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					aTexture3D[3].X() = fPolyPos * fSurroundFactor;
/*N*/ 					aTexture3D[3].Y() = fTextureStart;
/*N*/ 
/*N*/ 					aTexture3D[2].X() = fPolyPos * fSurroundFactor;
/*N*/ 					aTexture3D[2].Y() = fTextureStart + fTextureDepth;
/*N*/ 				}
/*N*/ 			}
/*N*/ 
/*N*/ 			for (USHORT i = rPoly3DFront.IsClosed() ? 0 : 1; i < nPntCnt; i++)
/*N*/ 			{
/*N*/ 				aRect3D[0] = aRect3D[3];
/*N*/ 				aRect3D[1] = aRect3D[2];
/*N*/ 
/*N*/ 				aRect3D[3] = rPoly3DFront[i];
/*N*/ 				aRect3D[2] = rPoly3DBack[i];
/*N*/ 
/*N*/ 				aNormal3D[0] = aNormal3D[3];
/*N*/ 				aNormal3D[1] = aNormal3D[2];
/*N*/ 
/*N*/ 				aNormal3D[3] = rPolyNormalsFront[i];
/*N*/ 				aNormal3D[2] = rPolyNormalsBack[i];
/*N*/ 
/*N*/ 				if(bCreateTexture)
/*N*/ 				{
/*N*/ 					// Texturkoordinaten ermitteln
/*N*/ 					Vector3D aPart = rPoly3DFront[i] - rPoly3DFront[nLastIndex];
/*N*/ 					fPolyPos += aPart.GetLength() / fPolyLength;
/*N*/ 					nLastIndex = i;
/*N*/ 
/*N*/ 					// Der Abschnitt am Polygon entspricht dem Teil
/*N*/ 					// von fPolyPos bis fPolyPos+fPartLength
/*N*/ 
/*N*/ 					aTexture3D[0] = aTexture3D[3];
/*N*/ 					aTexture3D[1] = aTexture3D[2];
/*N*/ 
/*N*/ 					if(bRotateTexture90)
/*N*/ 					{
/*N*/ 						// X,Y vertauschen
/*N*/ 						aTexture3D[3].X() = fTextureStart;
/*N*/ 						aTexture3D[3].Y() = (1.0 - fPolyPos) * fSurroundFactor;
/*N*/ 
/*N*/ 						aTexture3D[2].X() = fTextureStart + fTextureDepth;
/*N*/ 						aTexture3D[2].Y() = (1.0 - fPolyPos) * fSurroundFactor;
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*N*/ 						aTexture3D[3].X() = fPolyPos * fSurroundFactor;
/*N*/ 						aTexture3D[3].Y() = fTextureStart;
/*N*/ 
/*N*/ 						aTexture3D[2].X() = fPolyPos * fSurroundFactor;
/*N*/ 						aTexture3D[2].Y() = fTextureStart + fTextureDepth;
/*N*/ 					}
/*N*/ 
/*N*/ 					AddGeometry(aRect3D, aNormal3D, aTexture3D, FALSE);
/*N*/ 				}
/*N*/ 				else
/*N*/ 					AddGeometry(aRect3D, aNormal3D, FALSE);
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		for(UINT16 a=0;a<nPolyCnt;a++)
/*N*/ 		{
/*N*/ 			const Polygon3D& rPoly3DFront = rPolyPolyFront[a];
/*N*/ 			const Polygon3D& rPoly3DBack = rPolyPolyBack[a];
/*N*/ 			Polygon3D	aRect3D(4);
/*N*/ 			USHORT nPntCnt = rPoly3DFront.GetPointCount();
/*N*/ 			USHORT nPrefillIndex = rPoly3DFront.IsClosed() ? nPntCnt - 1 : 0;
/*N*/ 
/*N*/ 			aRect3D[3] = rPoly3DFront[nPrefillIndex];
/*N*/ 			aRect3D[2] = rPoly3DBack[nPrefillIndex];
/*N*/ 
/*N*/ 			for (USHORT i = rPoly3DFront.IsClosed() ? 0 : 1; i < nPntCnt; i++)
/*N*/ 			{
/*N*/ 				aRect3D[0] = aRect3D[3];
/*N*/ 				aRect3D[1] = aRect3D[2];
/*N*/ 
/*N*/ 				aRect3D[3] = rPoly3DFront[i];
/*N*/ 				aRect3D[2] = rPoly3DBack[i];
/*N*/ 
/*N*/ 				AddGeometry(aRect3D, FALSE);
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void E3dCompoundObject::AddInBetweenNormals(
/*N*/ 	const PolyPolygon3D& rPolyPolyFront,
/*N*/ 	const PolyPolygon3D& rPolyPolyBack,
/*N*/ 	PolyPolygon3D& rNormals,
/*N*/ 	BOOL bSmoothed)
/*N*/ {
/*N*/ 	USHORT nPolyCnt = rPolyPolyFront.Count();
/*N*/ 
/*N*/ 	// Verbindungsstuecke
/*N*/ 	for(UINT16 a=0;a<nPolyCnt;a++)
/*N*/ 	{
/*N*/ 		const Polygon3D& rPoly3DFront = rPolyPolyFront[a];
/*N*/ 		const Polygon3D& rPoly3DBack = rPolyPolyBack[a];
/*N*/ 		Polygon3D& rNormalPoly = rNormals[a];
/*N*/ 		USHORT nPntCnt = rPoly3DFront.GetPointCount();
/*N*/ 
/*N*/ 		if(rPoly3DBack.IsClosed())
/*N*/ 		{
/*N*/ 			Vector3D aNormal = (rPoly3DBack[nPntCnt - 1] - rPoly3DFront[nPntCnt - 1])
/*N*/ 					  |(rPoly3DFront[0] - rPoly3DFront[nPntCnt - 1]);
/*N*/ 			aNormal.Normalize();
/*N*/ 			for (USHORT i = 0; i < nPntCnt; i++)
/*N*/ 			{
/*N*/ 				Vector3D aNextNormal = (rPoly3DBack[i] - rPoly3DFront[i])
/*N*/ 					|(rPoly3DFront[(i+1 == nPntCnt) ? 0 : i+1] - rPoly3DFront[i]);
/*N*/ 				aNextNormal.Normalize();
/*N*/ 				if(bSmoothed)
/*N*/ 				{
/*N*/ 					Vector3D aMidNormal = aNormal + aNextNormal;
/*N*/ 					aMidNormal.Normalize();
/*N*/ 					rNormalPoly[i] += aMidNormal;
/*N*/ 				}
/*N*/ 				else
/*N*/ 					rNormalPoly[i] += aNormal;
/*N*/ 				rNormalPoly[i].Normalize();
/*N*/ 				aNormal = aNextNormal;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			Vector3D aNormal;
/*N*/ 			if(rPoly3DBack[0] == rPoly3DFront[0])
/*N*/ 			{
/*N*/ 				aNormal = (rPoly3DBack[1] - rPoly3DFront[1])
/*N*/ 					  |(rPoly3DFront[1] - rPoly3DFront[0]);
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				aNormal = (rPoly3DBack[0] - rPoly3DFront[0])
/*N*/ 					  |(rPoly3DFront[1] - rPoly3DFront[0]);
/*N*/ 			}
/*N*/ 			aNormal.Normalize();
/*N*/ 			rNormalPoly[0] += aNormal; rNormalPoly[0].Normalize();
/*N*/ 			for (USHORT i = 1; i < nPntCnt; i++)
/*N*/ 			{
/*N*/ 				Vector3D aNextNormal;
/*N*/ 				if(i+1 == nPntCnt)
/*N*/ 				{
/*N*/ 					aNextNormal = aNormal;
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					aNextNormal = (rPoly3DBack[i] - rPoly3DFront[i])
/*N*/ 						|(rPoly3DFront[i+1] - rPoly3DFront[i]);
/*N*/ 				}
/*N*/ 				aNextNormal.Normalize();
/*N*/ 				if(bSmoothed)
/*N*/ 				{
/*N*/ 					Vector3D aMidNormal = aNormal + aNextNormal;
/*N*/ 					aMidNormal.Normalize();
/*N*/ 					rNormalPoly[i] += aMidNormal;
/*N*/ 				}
/*N*/ 				else
/*N*/ 					rNormalPoly[i] += aNormal;
/*N*/ 				rNormalPoly[i].Normalize();
/*N*/ 				aNormal = aNextNormal;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Copy-Operator
|*
\************************************************************************/


/*************************************************************************
|*
|* Ausgabeparameter an 3D-Kontext setzen
|*
\************************************************************************/




/*************************************************************************
|*
|* Hittest fuer 3D-Objekte, wird an Geometrie weitergegeben
|*
\************************************************************************/

/*N*/ SdrObject* E3dCompoundObject::CheckHit(const Point& rPnt, USHORT nTol,
/*N*/ 	const SetOfByte* pVisiLayer) const
/*N*/ {
/*N*/ 	E3dPolyScene* pScene = (E3dPolyScene*)GetScene();
/*N*/ 	if(pScene)
/*N*/ 	{
/*N*/ 		// HitLine holen in ObjektKoordinaten
/*N*/ 		// ObjectTrans setzen
/*N*/ 		Matrix4D mTransform = ((E3dCompoundObject*)this)->GetFullTransform();
/*N*/ 		pScene->GetCameraSet().SetObjectTrans(mTransform);
/*N*/ 
/*N*/ 		// HitPoint Front und Back erzeugen und umrechnen
/*N*/ 		Vector3D aFront(rPnt.X(), rPnt.Y(), 0.0);
/*N*/ 		Vector3D aBack(rPnt.X(), rPnt.Y(), ZBUFFER_DEPTH_RANGE);
/*N*/ 		aFront = pScene->GetCameraSet().ViewToObjectCoor(aFront);
/*N*/ 		aBack = pScene->GetCameraSet().ViewToObjectCoor(aBack);
/*N*/ 
/*N*/ 		const Volume3D& rBoundVol = ((E3dCompoundObject*)this)->GetBoundVolume();
/*N*/ 		if(rBoundVol.IsValid())
/*N*/ 		{
/*N*/ 			double fXMax = aFront.X();
/*N*/ 			double fXMin = aBack.X();
/*N*/ 
/*N*/ 			if(fXMax < fXMin)
/*N*/ 			{
/*N*/ 				fXMax = aBack.X();
/*N*/ 				fXMin = aFront.X();
/*N*/ 			}
/*N*/ 
/*N*/ 			if(rBoundVol.MinVec().X() <= fXMax && rBoundVol.MaxVec().X() >= fXMin)
/*N*/ 			{
/*N*/ 				double fYMax = aFront.Y();
/*N*/ 				double fYMin = aBack.Y();
/*N*/ 
/*N*/ 				if(fYMax < fYMin)
/*N*/ 				{
/*N*/ 					fYMax = aBack.Y();
/*N*/ 					fYMin = aFront.Y();
/*N*/ 				}
/*N*/ 
/*N*/ 				if(rBoundVol.MinVec().Y() <= fYMax && rBoundVol.MaxVec().Y() >= fYMin)
/*N*/ 				{
/*N*/ 					double fZMax = aFront.Z();
/*N*/ 					double fZMin = aBack.Z();
/*N*/ 
/*N*/ 					if(fZMax < fZMin)
/*N*/ 					{
/*N*/ 						fZMax = aBack.Z();
/*N*/ 						fZMin = aFront.Z();
/*N*/ 					}
/*N*/ 
/*N*/ 					if(rBoundVol.MinVec().Z() <= fZMax && rBoundVol.MaxVec().Z() >= fZMin)
/*N*/ 					{
/*N*/ 						// Geometrie herstellen
/*N*/ 						if(!bGeometryValid)
/*?*/ 							((E3dCompoundObject*)this)->ReCreateGeometry();
/*N*/ 
/*N*/ 						// 3D Volumes schneiden sich, teste in der Geometrie
/*N*/ 						// auf Basis der Projektion weiter
/*N*/ 						if(((E3dCompoundObject*)this)->aDisplayGeometry.CheckHit(aFront, aBack, nTol) != -1L)
/*?*/ 							return ((E3dCompoundObject*)this);
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return NULL;
/*N*/ }

/*************************************************************************
|*
|* Geometrie des Objektes auf angegebenen Punkt zentrieren
|*
\************************************************************************/


/*************************************************************************
|*
|* Schattenattribute holen
|*
\************************************************************************/






/*N*/ BOOL E3dCompoundObject::DoDrawShadow()
/*N*/ {
/*N*/ 	const SfxItemSet& rSet = GetItemSet();
/*N*/ 	BOOL bRetval(FALSE);
/*N*/ 	BOOL bShadOn = ((SdrShadowItem&)(rSet.Get(SDRATTR_SHADOW))).GetValue();
/*N*/ 
/*N*/ 	if(bShadOn)
/*N*/ 	{
/*?*/ 		bRetval = TRUE;
/*?*/ 
/*?*/ 		if(((XFillStyleItem&)(rSet.Get(XATTR_FILLSTYLE))).GetValue() == XFILL_NONE)
/*?*/ 		{
/*?*/ 			if(((XLineStyleItem&)(rSet.Get(XATTR_LINESTYLE))).GetValue() == XLINE_NONE)
/*?*/ 			{
/*?*/ 				bRetval = FALSE;
/*?*/ 			}
/*?*/ 		}
/*N*/ 	}
/*N*/ 	return bRetval;
/*N*/ }

/*************************************************************************
|*
|* Objekt als WireFrame zeichnen
|*
\************************************************************************/


/*************************************************************************
|*
|* Create vertical polygons for line polygon
|*
\************************************************************************/

// #78972#
/*N*/ void E3dCompoundObject::ImpCompleteLinePolygon(PolyPolygon3D& rLinePolyPoly, 
/*N*/ 	sal_uInt16 nPolysPerRun, BOOL bClosed)
/*N*/ {
/*N*/ 	if(rLinePolyPoly.Count() && nPolysPerRun)
/*N*/ 	{
/*N*/ 		// get number of layers
/*N*/ 		sal_uInt16 nLayers(rLinePolyPoly.Count() / nPolysPerRun);
/*N*/ 		sal_uInt16 a, b, c;
/*N*/ 
/*N*/ 		// add vertical Polygons if at least two horizontal ones exist
/*N*/ 		if(nLayers > 1)
/*N*/ 		{
/*N*/ 			for(a = 0; a < nPolysPerRun; a++)
/*N*/ 			{
/*N*/ 				const sal_uInt16 nPntCnt = rLinePolyPoly[a].GetPointCount();
/*N*/ 
/*N*/ 				for(b = 0; b < nPntCnt; b++)
/*N*/ 				{
/*N*/ 					Polygon3D aNewVerPoly(bClosed ? nLayers + 1 : nLayers);
/*N*/ 					
/*N*/ 					for(c = 0; c < nLayers; c++)
/*N*/ 						aNewVerPoly[c] = rLinePolyPoly[(c * nPolysPerRun) + a][b];
/*N*/ 					
/*N*/ 					// evtl. set first point again to close polygon
/*N*/ 					if(bClosed)
/*N*/ 						aNewVerPoly[aNewVerPoly.GetPointCount()] = aNewVerPoly[0];
/*N*/ 				
/*N*/ 					// insert
/*N*/ 					rLinePolyPoly.Insert(aNewVerPoly);
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		
/*N*/ 		// open closed polygons
/*N*/ 		for(a = 0; a < rLinePolyPoly.Count(); a++)
/*N*/ 		{
/*N*/ 			if(rLinePolyPoly[a].IsClosed())
/*N*/ 			{
/*N*/ 				rLinePolyPoly[a][rLinePolyPoly[a].GetPointCount()] = rLinePolyPoly[a][0];
/*N*/ 				rLinePolyPoly[a].SetClosed(FALSE);
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }
/*N*/ 
/*************************************************************************
|*
|* Ein Segment fuer Extrude oder Lathe erzeugen
|*
\************************************************************************/

/*N*/ void E3dCompoundObject::ImpCreateSegment(
/*N*/ 	const PolyPolygon3D& rFront,		// vorderes Polygon
/*N*/ 	const PolyPolygon3D& rBack,			// hinteres Polygon
/*N*/ 	const PolyPolygon3D* pPrev,			// smooth uebergang zu Vorgaenger
/*N*/ 	const PolyPolygon3D* pNext,			// smooth uebergang zu Nachfolger
/*N*/ 	BOOL bCreateFront,					// vorderen Deckel erzeugen
/*N*/ 	BOOL bCreateBack,					// hinteren Deckel erzeugen
/*N*/ 	double fPercentDiag,				// Anteil des Deckels an der Tiefe
/*N*/ 	BOOL bSmoothLeft,					// Glaetten der umlaufenden Normalen links
/*N*/ 	BOOL bSmoothRight,					// Glaetten der umlaufenden Normalen rechts
/*N*/ 	BOOL bSmoothFrontBack,				// Glaetten der Abschlussflaechen
/*N*/ 	double fSurroundFactor,				// Wertebereich der Texturkoordinaten im Umlauf
/*N*/ 	double fTextureStart,				// TexCoor ueber Extrude-Tiefe
/*N*/ 	double fTextureDepth,				// TexCoor ueber Extrude-Tiefe
/*N*/ 	BOOL bCreateTexture,
/*N*/ 	BOOL bCreateNormals,
/*N*/ 	BOOL bCharacterExtrude,				// FALSE=exakt, TRUE=ohne Ueberschneidungen
/*N*/ 	BOOL bRotateTexture90,				// Textur der Seitenflaechen um 90 Grad kippen
/*N*/ 	PolyPolygon3D* pLineGeometry		// For creation of line geometry
/*N*/ 	)
/*N*/ {
/*N*/ 	PolyPolygon3D aNormalsLeft, aNormalsRight;
/*N*/ 	AddInBetweenNormals(rFront, rBack, aNormalsLeft, bSmoothLeft);
/*N*/ 	AddInBetweenNormals(rFront, rBack, aNormalsRight, bSmoothRight);
/*N*/ 	Vector3D aOffset = rBack.GetMiddle() - rFront.GetMiddle();
/*N*/ 
/*N*/ 	// Ausnahmen: Nicht geschlossen
/*N*/ 	if(!rFront.IsClosed())
/*N*/ 	{
/*N*/ 		bCreateFront = FALSE;
/*N*/ 	}
/*N*/ 	if(!rBack.IsClosed())
/*N*/ 	{
/*N*/ 		bCreateBack = FALSE;
/*N*/ 	}
/*N*/ 
/*N*/ 	// Ausnahmen: Einfache Linie
/*N*/ 	if(rFront[0].GetPointCount() < 3 || (!bCreateFront && !bCreateBack))
/*N*/ 	{
/*N*/ 		fPercentDiag = 0.0;
/*N*/ 	}
/*N*/ 
/*N*/ 	if(fPercentDiag == 0.0)
/*N*/ 	{
/*N*/ 		// Ohne Schraegen, Vorderseite
/*N*/ 		if(bCreateFront)
/*N*/ 		{
/*N*/ 			PolyPolygon3D aNormalsFront;
/*N*/ 			AddFrontNormals(rFront, aNormalsFront, aOffset);
/*N*/ 
/*N*/ 			if(!bSmoothFrontBack)
/*N*/ 				CreateFront(rFront, aNormalsFront, bCreateNormals, bCreateTexture);
/*N*/ 			if(bSmoothLeft)
/*N*/ 				AddFrontNormals(rFront, aNormalsLeft, aOffset);
/*N*/ 			if(bSmoothFrontBack)
/*?*/ 				CreateFront(rFront, aNormalsLeft, bCreateNormals, bCreateTexture);
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			if(pPrev)
/*N*/ 				AddInBetweenNormals(*pPrev, rFront, aNormalsLeft, bSmoothLeft);
/*N*/ 		}
/*N*/ 
/*N*/ 		// Ohne Schraegen, Rueckseite
/*N*/ 		if(bCreateBack)
/*N*/ 		{
/*N*/ 			PolyPolygon3D aNormalsBack;
/*N*/ 			AddBackNormals(rBack, aNormalsBack, aOffset);
/*N*/ 
/*N*/ 			if(!bSmoothFrontBack)
/*N*/ 				CreateBack(rBack, aNormalsBack, bCreateNormals, bCreateTexture);
/*N*/ 			if(bSmoothRight)
/*N*/ 				AddBackNormals(rBack, aNormalsRight, aOffset);
/*N*/ 			if(bSmoothFrontBack)
/*?*/ 				CreateBack(rBack, aNormalsRight, bCreateNormals, bCreateTexture);
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			if(pNext)
/*N*/ 				AddInBetweenNormals(rBack, *pNext, aNormalsRight, bSmoothRight);
/*N*/ 		}
/*N*/ 
/*N*/ 		// eigentliches Zwischenstueck
/*N*/ 		CreateInBetween(rFront, rBack,
/*N*/ 			aNormalsLeft, aNormalsRight,
/*N*/ 			bCreateNormals,
/*N*/ 			fSurroundFactor,
/*N*/ 			fTextureStart,
/*N*/ 			fTextureDepth,
/*N*/ 			bRotateTexture90);
/*N*/ 
/*N*/ 		// #78972#
/*N*/ 		if(pLineGeometry)
/*N*/ 		{
/*N*/ 			pLineGeometry->Insert(rFront);
/*N*/ 			if(bCreateBack)
/*N*/ 				pLineGeometry->Insert(rBack);
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// Mit Scraegen, Vorderseite
/*N*/ 		PolyPolygon3D aLocalFront = rFront;
/*N*/ 		PolyPolygon3D aLocalBack = rBack;
/*N*/ 		double fExtrudeDepth, fDiagLen;
/*N*/ 		double fTexMidStart = fTextureStart;
/*N*/ 		double fTexMidDepth = fTextureDepth;
/*N*/ 
/*N*/ 		if(bCreateFront || bCreateBack)
/*N*/ 		{
/*N*/ 			fExtrudeDepth = aOffset.GetLength();
/*N*/ 			fDiagLen = fPercentDiag * fExtrudeDepth;
/*N*/ 		}
/*N*/ 
/*N*/ 		PolyPolygon3D aOuterFront;
/*N*/ 		PolyPolygon3D aOuterBack;
/*N*/ 
/*N*/ 		if(bCreateFront)
/*N*/ 		{
/*N*/ 			PolyPolygon3D aNormalsOuterFront;
/*N*/ 			AddFrontNormals(aLocalFront, aNormalsOuterFront, aOffset);
/*N*/ 
/*N*/ 			if(bCharacterExtrude)
/*N*/ 			{
/*N*/ 				// Polygon kopieren
/*N*/ 				aOuterFront = aLocalFront;
/*N*/ 
/*N*/ 				// notwendige Normalen erzeugen
/*N*/ 				PolyPolygon3D aGrowDirection;
/*N*/ 				AddInBetweenNormals(aLocalFront, aLocalBack, aGrowDirection, bSmoothLeft);
/*N*/ 
/*N*/ 				// Groesse inneres Polygon merken
/*N*/ 				Volume3D aOldSize(aLocalFront.GetPolySize());
/*N*/ 
/*N*/ 				// Inneres Polygon vergroessern
/*N*/ 				GrowPoly(aLocalFront, aGrowDirection, fDiagLen);
/*N*/ 
/*N*/ 				// Inneres Polygon nach innen verschieben
/*N*/ 				//GrowPoly(aLocalFront, aNormalsOuterFront, -fDiagLen);
/*N*/ 				
/*N*/ 				// Neue Groesse inneres Polygon feststellen
/*N*/ 				Volume3D aNewSize(aLocalFront.GetPolySize());
/*N*/ 
/*N*/ 				// Skalierung feststellen (nur X,Y)
/*N*/ 				Vector3D aScaleVec(
/*N*/ 					(aNewSize.GetWidth() != 0.0) ? aOldSize.GetWidth() / aNewSize.GetWidth() : 1.0,
/*N*/ 					(aNewSize.GetHeight() != 0.0) ? aOldSize.GetHeight() / aNewSize.GetHeight() : 1.0,
/*N*/ 					(aNewSize.GetDepth() != 0.0) ? aOldSize.GetDepth() / aNewSize.GetDepth() : 1.0);
/*N*/ 
/*N*/ 				// Transformation bilden
/*N*/ 				Matrix4D aTransMat;
/*N*/ 				aTransMat.Scale(aScaleVec);
/*N*/ 
/*N*/ 				// aeusseres und inneres Polygon skalieren
/*N*/ 				aLocalFront.Transform(aTransMat);
/*N*/ 				aOuterFront.Transform(aTransMat);
/*N*/ 
/*N*/ 				// Neue Groesse aktualisieren
/*N*/ 				aNewSize = aLocalFront.GetPolySize();
/*N*/ 
/*N*/ 				// Translation feststellen
/*N*/ 				Vector3D aTransVec(
/*N*/ 					aOldSize.MinVec().X() - aNewSize.MinVec().X(),
/*N*/ 					aOldSize.MinVec().Y() - aNewSize.MinVec().Y(),
/*N*/ 					aOldSize.MinVec().Z() - aNewSize.MinVec().Z());
/*N*/ 
/*N*/ 				// Transformation bilden
/*N*/ 				aTransMat.Identity();
/*N*/ 				aTransMat.Translate(aTransVec);
/*N*/ 
/*N*/ 				// aeusseres und inneres Polygon skalieren
/*N*/ 				aLocalFront.Transform(aTransMat);
/*N*/ 				aOuterFront.Transform(aTransMat);
/*N*/ 
/*N*/ 				// move aLocalFront again, scale and translate has moved it back
/*N*/ 				GrowPoly(aLocalFront, aNormalsOuterFront, -fDiagLen);
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				// Polygon kopieren
/*N*/ 				aOuterFront = aLocalFront;
/*N*/ 
/*N*/ 				// notwendige Normalen erzeugen
/*N*/ 				PolyPolygon3D aGrowDirection;
/*N*/ 				AddInBetweenNormals(aLocalFront, aLocalBack, aGrowDirection, bSmoothLeft);
/*N*/ 
/*N*/ 				// Aeusseres Polygon verkleinern
/*N*/ 				GrowPoly(aOuterFront, aGrowDirection, -fDiagLen);
/*N*/ 				aOuterFront.CorrectGrownPoly(aLocalFront);
/*N*/ 
/*N*/ 				// Inneres Polygon nach innen verschieben
/*N*/ 				GrowPoly(aLocalFront, aNormalsOuterFront, -fDiagLen);
/*N*/ 			}
/*N*/ 
/*N*/ 			// eventuell noch glaetten
/*N*/ 			if(bSmoothLeft)
/*N*/ 			{
/*N*/ 				if(bSmoothFrontBack)
/*?*/ 					AddInBetweenNormals(aOuterFront, aLocalFront, aNormalsOuterFront, bSmoothLeft);
/*N*/ 				AddInBetweenNormals(aOuterFront, aLocalFront, aNormalsLeft, bSmoothLeft);
/*N*/ 			}
/*N*/ 
/*N*/ 			// vordere Zwischenstuecke erzeugen
/*N*/ 			CreateInBetween(aOuterFront, aLocalFront,
/*N*/ 				aNormalsOuterFront, aNormalsLeft,
/*N*/ 				bCreateNormals,
/*N*/ 				fSurroundFactor,
/*N*/ 				fTextureStart,
/*N*/ 				fTextureDepth * fPercentDiag,
/*N*/ 				bRotateTexture90);
/*N*/ 
/*N*/ 			// Vorderseite erzeugen
/*N*/ 			CreateFront(aOuterFront, aNormalsOuterFront, bCreateNormals, bCreateTexture);
/*N*/ 
/*N*/ 			// Weitere Texturwerte setzen
/*N*/ 			fTexMidStart += fTextureDepth * fPercentDiag;
/*N*/ 			fTexMidDepth -= fTextureDepth * fPercentDiag;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*?*/ 			if(pPrev)
/*?*/ 				AddInBetweenNormals(*pPrev, rFront, aNormalsLeft, bSmoothLeft);
/*N*/ 		}
/*N*/ 
/*N*/ 		// Mit Scraegen, Rueckseite
/*N*/ 		if(bCreateBack)
/*N*/ 		{
/*N*/ 			PolyPolygon3D aNormalsOuterBack;
/*N*/ 			AddBackNormals(aLocalBack, aNormalsOuterBack, aOffset);
/*N*/ 
/*N*/ 			if(bCharacterExtrude)
/*N*/ 			{
/*N*/ 				// Polygon kopieren
/*N*/ 				aOuterBack = aLocalBack;
/*N*/ 
/*N*/ 				// notwendige Normalen erzeugen
/*N*/ 				PolyPolygon3D aGrowDirection;
/*N*/ 				AddInBetweenNormals(aLocalFront, aLocalBack, aGrowDirection, bSmoothRight);
/*N*/ 
/*N*/ 				// Groesse inneres Polygon merken
/*N*/ 				Volume3D aOldSize(aLocalBack.GetPolySize());
/*N*/ 
/*N*/ 				// Inneres Polygon vergroessern
/*N*/ 				GrowPoly(aLocalBack, aGrowDirection, fDiagLen);
/*N*/ 
/*N*/ 				// Inneres Polygon nach innen verschieben
/*N*/ 				//GrowPoly(aLocalBack, aNormalsOuterBack, -fDiagLen);
/*N*/ 				
/*N*/ 				// Neue Groesse inneres Polygon feststellen
/*N*/ 				Volume3D aNewSize(aLocalBack.GetPolySize());
/*N*/ 
/*N*/ 				// Skalierung feststellen (nur X,Y)
/*N*/ 				Vector3D aScaleVec(
/*N*/ 					(aNewSize.GetWidth() != 0.0) ? aOldSize.GetWidth() / aNewSize.GetWidth() : 1.0,
/*N*/ 					(aNewSize.GetHeight() != 0.0) ? aOldSize.GetHeight() / aNewSize.GetHeight() : 1.0,
/*N*/ 					(aNewSize.GetDepth() != 0.0) ? aOldSize.GetDepth() / aNewSize.GetDepth() : 1.0);
/*N*/ 
/*N*/ 				// Transformation bilden
/*N*/ 				Matrix4D aTransMat;
/*N*/ 				aTransMat.Scale(aScaleVec);
/*N*/ 
/*N*/ 				// aeusseres und inneres Polygon skalieren
/*N*/ 				aLocalBack.Transform(aTransMat);
/*N*/ 				aOuterBack.Transform(aTransMat);
/*N*/ 
/*N*/ 				// Neue Groesse aktualisieren
/*N*/ 				aNewSize = aLocalBack.GetPolySize();
/*N*/ 
/*N*/ 				// Translation feststellen
/*N*/ 				Vector3D aTransVec(
/*N*/ 					aOldSize.MinVec().X() - aNewSize.MinVec().X(),
/*N*/ 					aOldSize.MinVec().Y() - aNewSize.MinVec().Y(),
/*N*/ 					aOldSize.MinVec().Z() - aNewSize.MinVec().Z());
/*N*/ 
/*N*/ 				// Transformation bilden
/*N*/ 				aTransMat.Identity();
/*N*/ 				aTransMat.Translate(aTransVec);
/*N*/ 
/*N*/ 				// aeusseres und inneres Polygon skalieren
/*N*/ 				aLocalBack.Transform(aTransMat);
/*N*/ 				aOuterBack.Transform(aTransMat);
/*N*/ 
/*N*/ 				// move aLocalBack again, scale and translate has moved it back
/*N*/ 				GrowPoly(aLocalBack, aNormalsOuterBack, -fDiagLen);
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				// Polygon kopieren
/*N*/ 				aOuterBack = aLocalBack;
/*N*/ 
/*N*/ 				// notwendige Normalen erzeugen
/*N*/ 				PolyPolygon3D aGrowDirection;
/*N*/ 				AddInBetweenNormals(aLocalFront, aLocalBack, aGrowDirection, bSmoothRight);
/*N*/ 
/*N*/ 				// Aeusseres Polygon verkleinern
/*N*/ 				GrowPoly(aOuterBack, aGrowDirection, -fDiagLen);
/*N*/ 				aOuterBack.CorrectGrownPoly(aLocalBack);
/*N*/ 
/*N*/ 				// Inneres Polygon nach innen verschieben
/*N*/ 				GrowPoly(aLocalBack, aNormalsOuterBack, -fDiagLen);
/*N*/ 			}
/*N*/ 
/*N*/ 			// eventuell noch glaetten
/*N*/ 			if(bSmoothRight)
/*N*/ 			{
/*N*/ 				if(bSmoothFrontBack)
/*?*/ 					AddInBetweenNormals(aLocalBack, aOuterBack, aNormalsOuterBack, bSmoothRight);
/*N*/ 				AddInBetweenNormals(aLocalBack, aOuterBack, aNormalsRight, bSmoothRight);
/*N*/ 			}
/*N*/ 
/*N*/ 			// vordere Zwischenstuecke erzeugen
/*N*/ 			// hintere Zwischenstuecke erzeugen
/*N*/ 			CreateInBetween(aLocalBack, aOuterBack,
/*N*/ 				aNormalsRight, aNormalsOuterBack,
/*N*/ 				bCreateNormals,
/*N*/ 				fSurroundFactor,
/*N*/ 				fTextureStart + (fTextureDepth * (1.0 - fPercentDiag)),
/*N*/ 				fTextureDepth * fPercentDiag,
/*N*/ 				bRotateTexture90);
/*N*/ 
/*N*/ 			// Rueckseite erzeugen
/*N*/ 			CreateBack(aOuterBack, aNormalsOuterBack, bCreateNormals, bCreateTexture);
/*N*/ 
/*N*/ 			// Weitere Texturwerte setzen
/*N*/ 			fTexMidDepth -= fTextureDepth * fPercentDiag;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*?*/ 			if(pNext)
/*?*/ 				AddInBetweenNormals(rBack, *pNext, aNormalsRight, bSmoothRight);
/*?*/ 		}
/*N*/ 
/*N*/ 		// eigentliches Zwischenstueck
/*N*/ 		CreateInBetween(aLocalFront, aLocalBack,
/*N*/ 			aNormalsLeft, aNormalsRight,
/*N*/ 			bCreateNormals,
/*N*/ 			fSurroundFactor,
/*N*/ 			fTexMidStart,
/*N*/ 			fTexMidDepth,
/*N*/ 			bRotateTexture90);
/*N*/ 
/*N*/ 		// #78972#
/*N*/ 		if(pLineGeometry)
/*N*/ 		{
/*N*/ 			if(bCreateFront)
/*N*/ 				pLineGeometry->Insert(aOuterFront);
/*N*/ 			pLineGeometry->Insert(aLocalFront);
/*N*/ 			if(bCreateBack)
/*N*/ 			{
/*N*/ 				pLineGeometry->Insert(aLocalBack);
/*N*/ 				pLineGeometry->Insert(aOuterBack);
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Parameter Geometrieerzeugung setzen
|*
\************************************************************************/



/*************************************************************************
|*
|* DisplayGeometry rausruecken
|*
\************************************************************************/

/*N*/ B3dGeometry& E3dCompoundObject::GetDisplayGeometry()
/*N*/ {
/*N*/ 	// Geometrie herstellen
/*N*/ 	if(!bGeometryValid)
/*?*/ 		ReCreateGeometry();
/*N*/ 
/*N*/ 	return aDisplayGeometry;
/*N*/ }

/*************************************************************************
|*
|* Material des Objektes
|*
\************************************************************************/

/*N*/ void E3dCompoundObject::SetMaterialAmbientColor(const Color& rColor)
/*N*/ {
/*N*/ 	if(aMaterialAmbientColor != rColor)
/*N*/ 	{
/*?*/ 		aMaterialAmbientColor = rColor;
/*N*/ 	}
/*N*/ }



/*************************************************************************
|*
|* 3D Ausgabe
|*
\************************************************************************/


/*************************************************************************
|*
|* Objekt als Kontur in das Polygon einfuegen
|*
\************************************************************************/


/*************************************************************************
|*
|* Schatten fuer 3D-Objekte zeichnen
|*
\************************************************************************/




/*************************************************************************
|*
|* convert given PolyPolygon3D to screen coor
|*
\************************************************************************/


/*************************************************************************
|*
|* Attribute setzen
|*
\************************************************************************/

// private support routines for ItemSet access. NULL pointer means clear item.
/*N*/ void E3dCompoundObject::PostItemChange(const sal_uInt16 nWhich)
/*N*/ {
/*N*/ 	// call parent
/*N*/ 	E3dObject::PostItemChange(nWhich);
/*N*/ 
/*N*/ 	// handle value change
/*N*/ 	switch(nWhich)
/*N*/ 	{
/*N*/ 		case SDRATTR_3DOBJ_DOUBLE_SIDED:
/*N*/ 		{
/*N*/ 			bGeometryValid = FALSE;
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case SDRATTR_3DOBJ_NORMALS_KIND:
/*N*/ 		{
/*N*/ 			bGeometryValid = FALSE;
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case SDRATTR_3DOBJ_NORMALS_INVERT:
/*N*/ 		{
/*N*/ 			bGeometryValid = FALSE;
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case SDRATTR_3DOBJ_TEXTURE_PROJ_X:
/*N*/ 		{
/*N*/ 			bGeometryValid = FALSE;
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case SDRATTR_3DOBJ_TEXTURE_PROJ_Y:
/*N*/ 		{
/*N*/ 			bGeometryValid = FALSE;
/*N*/ 			break;
/*N*/ 		}
/*N*/ 	}
/*N*/ }

// EOF
}
