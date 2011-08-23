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

#ifdef _MSC_VER
#pragma hdrstop
#endif



#include "axisid.hxx"
#include "chtscene.hxx"
namespace binfilter {

#ifdef _MSC_VER
#pragma optimize ("",off)
#endif


/*N*/ TYPEINIT1(ChartScene, E3dPolyScene);

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

/*N*/ ChartScene::ChartScene(ChartModel* pDocument) :
/*N*/ 	E3dPolyScene(),
/*N*/ 	pDoc(pDocument),
/*N*/ 	bAskForLogicRect(TRUE)
/*N*/ {
/*N*/ 	nSortingMode = E3D_SORT_LOOKUP_FIELD | E3D_SORT_NON_POLYOBJ | E3D_SORT_TEST_LENGTH;
/*N*/     Initialize();
/*N*/     SetModel( pDocument );
/*N*/ }

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

/*N*/ ChartScene::~ChartScene ()
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|* Einpassen der Projektion aller Szenenobjekte in das
|* umschliessende Rechteck
|*
\************************************************************************/

/*N*/ Volume3D ChartScene::FitInSnapRect()
/*N*/ {
/*N*/ 	// untransformiertes BoundVolume holen und parent rufen
/*N*/ 	Volume3D aNewVol = E3dScene::FitInSnapRect();
/*N*/ 
/*N*/ 	// Groesse etwas anpassen, umPlatz am Rand des Charts fu schaffen
/*N*/ 	aNewVol.MinVec () = Vector3D (
/*N*/ 		aNewVol.MinVec ().X () * 1.2,
/*N*/ 		aNewVol.MinVec ().Y () * 1.2,
/*N*/ 		aNewVol.MinVec ().Z ());
/*N*/ 	aNewVol.MaxVec () = Vector3D (
/*N*/ 		aNewVol.MaxVec ().X () * 1.2,
/*N*/ 		aNewVol.MaxVec ().Y () * 1.2,
/*N*/ 		aNewVol.MaxVec ().Z ());
/*N*/ 
/*N*/ 	//pDoc->Position3DAxisTitles(GetLogicRect());
/*N*/ 
/*N*/ 	SetRectsDirty(FALSE);
/*N*/ 	return aNewVol;
/*N*/ }

/*************************************************************************
|*
|* Zuweisungsoperator
|*
\************************************************************************/




/*************************************************************************
|*
|* Speichern
|*
\************************************************************************/

/*N*/ void ChartScene::WriteData(SvStream& rOut) const
/*N*/ {
/*N*/ 	if (rOut.GetVersion() > 3780 && pSub && pSub->GetPage())
/*N*/ 	{
/*N*/ 		// FileFormat 5.0
/*N*/ 		// Die SubList der ChartScene wird nun nicht mehr geschrieben
/*N*/ 
/*N*/ 		//pSub->GetPage()->SetObjectsNotPersistent(TRUE);
/*N*/ 
/*N*/ 		// Scene schreiben
/*N*/ 		E3dPolyScene::WriteData(rOut);
/*N*/ 
/*N*/ 		//pSub->GetPage()->SetObjectsNotPersistent(FALSE);
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// FileFormat 4.0 und aelter
/*N*/ 		E3dPolyScene::WriteData(rOut);
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Laden
|*
\************************************************************************/


/*************************************************************************
|*
|* Erstelle die 3D-Achsenbeschriftung  //war mal in globfunc.cxx
|*
\************************************************************************/

/*N*/ void ChartScene::InsertAllTitleText (DescrList         &rList,
/*N*/ 									 E3dObject *pGroup,
/*N*/ 									 long              nAxisId)
/*N*/ {
/*N*/ 	Rectangle aOldRect;
/*N*/ 
/*N*/ 	for (E3dLabelObj *pLabel = rList.First (); pLabel; pLabel = rList.Next ())
/*N*/ 	{
/*N*/ 		Insert3DObj(pLabel);
/*N*/ 		pLabel->InsertUserData (new SchAxisId (nAxisId));
/*N*/ 	}
/*N*/ }
/*N*/ Rectangle ChartScene::Get3DDescrRect(E3dLabelObj *p3DObj,B3dCamera& rCamSet )
/*N*/ {
/*N*/ 	const SdrObject* pObj = p3DObj->Get2DLabelObj();
/*N*/ 	// View- Abmessungen des Labels holen
/*N*/ 	Rectangle  aRect = pObj->GetLogicRect();
/*N*/ 
/*N*/ 	// Position des Objektes in Weltkoordinaten ermitteln
/*N*/ 	Vector3D aObjPos = p3DObj->GetFullTransform() * p3DObj->GetPosition();
/*N*/ 	aObjPos = rCamSet.WorldToViewCoor(aObjPos);
/*N*/ 	Point aPoint((long)(aObjPos.X() + 0.5), (long)(aObjPos.Y() + 0.5));
/*N*/ 
/*N*/ 	// Relative Position des Labels in View-Koordinaten
/*N*/ 	Point aRelPosOne = pObj->GetRelativePos();
/*N*/ 	aRelPosOne += aPoint;
/*N*/ 
/*N*/ 	aRect.SetPos(aRelPosOne);
/*N*/ 	return aRect;
/*N*/ }

/*N*/ void ChartScene::ReduceDescrList(DescrList & aList)
/*N*/ {
/*N*/ 
/*N*/ 	Rectangle aIntersect(0,0,0,0);
/*N*/ 	Rectangle aPrevRect(0,0,0,0);
/*N*/ 	Rectangle aNextRect(0,0,0,0);
/*N*/ 
/*N*/ 	//Transformation berechnen, die später im Paint ausgeführt wird,
/*N*/ 	//(Derzeit sind die Labelobject-Positionen unbekannt)
/*N*/ 	Rectangle aBound(GetSnapRect());
/*N*/ 	Volume3D aVolume = FitInSnapRect();
/*N*/ 	B3dCamera& rSet = GetCameraSet();
/*N*/ 	rSet.SetDeviceRectangle(aVolume.MinVec().X(), aVolume.MaxVec().X(),
/*N*/ 	aVolume.MinVec().Y(), aVolume.MaxVec().Y(), FALSE);
/*N*/ 	rSet.SetFrontClippingPlane(aVolume.MinVec().Z());
/*N*/ 	rSet.SetBackClippingPlane(aVolume.MaxVec().Z());
/*N*/ 	rSet.SetViewportRectangle(aBound);
/*N*/ 
/*N*/ 
/*N*/ 	E3dLabelObj *p3DObj=aList.First();
/*N*/ 	E3dLabelObj *pOld3DObj=p3DObj;
/*N*/ 	BOOL bGetCurrent=FALSE;
/*N*/ 
/*N*/ 	if(p3DObj)
/*N*/ 	{
/*N*/ 		const SdrTextObj* pObj = (const SdrTextObj*)p3DObj->Get2DLabelObj();
/*N*/ 
/*N*/ 		//Es reicht, die Rotation des ersten Elements zu ermitteln,
/*N*/ 		//alle in der Liste sind gleichermaßen gedreht
/*N*/ 		//GetRotateAngle() gibt 100tel, gebraucht werden 10tel Grad.
/*N*/ 		long nAngle = pObj->GetRotateAngle()/10;
/*N*/ 
/*N*/ 		aPrevRect=Get3DDescrRect(p3DObj,rSet);
/*N*/ 		if(nAngle!=0)
/*N*/ 		{
/*?*/ 			//Um TopLeft drehen, so wie es später gezeichnet wird
/*?*/ 			XPolygon aPoly(aPrevRect);
/*?*/ 			aPoly.Rotate(aPrevRect.TopLeft(),(USHORT)nAngle);
/*?*/ 			//und um den Koordinaten-Ursprung herum zurückdrehen
/*?*/ 			//um wieder Rectangles zu erhalten (für Intersect)
/*?*/ 			aPoly.Rotate(Point(0,0),(USHORT)(3600 - nAngle));
/*?*/ 			aPrevRect=aPoly.GetBoundRect();
/*N*/ 		}
/*N*/ 
/*N*/ 		while(p3DObj)
/*N*/ 		{
/*N*/ 			//nächstes Objekt holen, abhängig davon, ob das zuletzt behandelte
/*N*/ 			//entfernt wurde oder nicht (bGetCurrent)
/*N*/ 			if(bGetCurrent)
/*N*/ 			{
/*?*/ 				p3DObj=aList.GetCurObject();
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				p3DObj=aList.Next();
/*N*/ 			}
/*N*/ 			bGetCurrent=FALSE;
/*N*/ 
/*N*/ 			//Da insbesondere bei Remove() des letzten Objects sowohl Next()
/*N*/ 			//als auch GetCurObject() den alten Pointer zurückgeben,
/*N*/ 			//wird getestet, ob tatsächlich verschiedene Objekte vorliegen
/*N*/ 			DBG_ASSERT(p3DObj!=pOld3DObj,"Chart: pointers equal in Scene:reduce...");
/*N*/ 			if(p3DObj && p3DObj!=pOld3DObj)
/*N*/ 			{
/*N*/ 				pOld3DObj=p3DObj;
/*N*/ 
/*N*/ 				aNextRect=Get3DDescrRect(p3DObj,rSet);
/*N*/ 
/*N*/ 				if(nAngle!=0)
/*N*/ 				{
/*?*/ 					//Um TopLeft drehen (wie oben):
/*?*/ 					XPolygon aPoly(aNextRect);
/*?*/ 					aPoly.Rotate(aNextRect.TopLeft(),(USHORT)nAngle);
/*?*/ 					//und um den Ursprung herum zurückdrehen
/*?*/ 					aPoly.Rotate(Point(0,0),(USHORT)(3600 - nAngle));
/*?*/ 					aNextRect=aPoly.GetBoundRect();
/*N*/ 				}
/*N*/ 
/*N*/ 				aIntersect=aNextRect.GetIntersection(aPrevRect);
/*N*/ 				if( !  (aIntersect.IsEmpty())
/*N*/ 					&& (   (aIntersect.GetHeight()>aNextRect.GetHeight()/100)
/*N*/ 						 ||(aIntersect.GetWidth() >aNextRect.GetHeight()/100)//2% Deckung maximal bezogen auf die Fonthöhe
/*N*/ 						)
/*N*/ 				  )
/*N*/ 				{
/*N*/ 					E3dObject* pParent=p3DObj->GetParentObj();
/*N*/ 					if(pParent)
/*N*/ 					{
/*N*/ 						//aus der Page streichen
/*N*/ 						pParent->Remove3DObj(p3DObj);
/*N*/ 
/*N*/ 
/*N*/ 						//Die Objekte koennen ruhig in der Liste verbleiben, löschen führt
/*N*/ 						//nur zu Problemen
/*N*/ 
/*N*/ 						//Da das Object entfernt wurde, darf nicht Next gerufen werden.
/*N*/ 						//bGetCurrent=TRUE;
/*N*/ 						//und aus der Liste streichen
/*N*/ 						//aList.Remove();
/*N*/ 						//delete p3DObj; (íst offenbar bei Remove() schon geschehen ???)
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*?*/ 						DBG_TRACE("Chart:: Object has no parent (Scene)");
/*N*/ 					}
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					aPrevRect=aNextRect;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ }

/*N*/ void ChartScene::Initialize()
/*N*/ {
/*N*/ 	// #66930# BM  activate second light source and deactivate first one
/*N*/     // reason: the first light source is in contrast to the other seven
/*N*/     //         lightsources specular by default
/*N*/ 
/*N*/     // Note: Use items at the scene instead of methods at the subobjects
/*N*/     //       otherwise settings get overwritten later
/*N*/     
/*N*/     // copy lightsource 1 (Base3DLight0) to lightsource 2
/*N*/     // color
/*N*/ 	SetItem( Svx3DLightcolor2Item( GetLightGroup().GetIntensity( Base3DMaterialDiffuse, Base3DLight0 )));
/*N*/     // direction
/*N*/ 	SetItem( Svx3DLightDirection2Item( GetLightGroup().GetDirection( Base3DLight0 )));
/*N*/ 
/*N*/     // enable light source 2
/*N*/     SetItem( Svx3DLightOnOff2Item( TRUE ));
/*N*/     // disable light source 1
/*N*/     SetItem( Svx3DLightOnOff1Item( FALSE ));
/*N*/ }
}
