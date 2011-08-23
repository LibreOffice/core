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

#ifndef _SVDITER_HXX
#include "svditer.hxx"
#endif

#ifndef _SVDIO_HXX
#include "svdio.hxx"
#endif

#if defined( UNX ) || defined( ICC )
#include <stdlib.h>
#endif

#ifndef _SFXSTYLE_HXX
#include <bf_svtools/style.hxx>
#endif

#ifndef _E3D_PLIGHT3D_HXX
#include "plight3d.hxx"
#endif

#ifndef _E3D_DLIGHT3D_HXX
#include "dlight3d.hxx"
#endif

#ifndef _SVDTRANS_HXX
#include "svdtrans.hxx"
#endif

#ifndef _SVX_SVXIDS_HRC
#include "svxids.hrc"
#endif

#ifndef _SFX_WHITER_HXX
#include <bf_svtools/whiter.hxx>
#endif

#include "scene3d.hxx"

namespace binfilter {

#define ITEMVALUE(ItemSet,Id,Cast)	((const Cast&)(ItemSet).Get(Id)).GetValue()

/*N*/ TYPEINIT1(E3dScene, E3dObject);

/*************************************************************************
|*
|* E3dScene-Konstruktor
|*
\************************************************************************/

/*N*/ E3dScene::E3dScene()
/*N*/ :	E3dObject(),
/*N*/ 	aCamera(Vector3D(0,0,4), Vector3D()),
/*N*/ 	bDoubleBuffered(FALSE),
/*N*/ 	bClipping(FALSE),
/*N*/ 	nSaveStatus (0),
/*N*/ 	nRestStatus (0),
/*N*/ 	bFitInSnapRect(TRUE),
/*N*/ 	aPaintTime(),
/*N*/ 	nDisplayQuality(255),
/*N*/ 	bDrawOnlySelected(FALSE)
/*N*/ {
/*N*/ 	// Defaults setzen
/*N*/ 	E3dDefaultAttributes aDefault;
/*N*/ 	SetDefaultAttributes(aDefault);
/*N*/ }

/*N*/ void E3dScene::SetDefaultAttributes(E3dDefaultAttributes& rDefault)
/*N*/ {
/*N*/ 	// Fuer OS/2 die FP-Exceptions abschalten
/*N*/ #if defined(OS2)
/*N*/ #define SC_FPEXCEPTIONS_ON()	_control87( MCW_EM, 0 )
/*N*/ #define SC_FPEXCEPTIONS_OFF()	_control87( MCW_EM, MCW_EM )
/*N*/ 	SC_FPEXCEPTIONS_OFF();
/*N*/ #endif
/*N*/
/*N*/ 	// Fuer WIN95/NT die FP-Exceptions abschalten
/*N*/ #if defined(WNT) || defined(WIN)
/*N*/ #define SC_FPEXCEPTIONS_ON()	_control87( _MCW_EM, 0 )
/*N*/ #define SC_FPEXCEPTIONS_OFF()	_control87( _MCW_EM, _MCW_EM )
/*N*/ 	SC_FPEXCEPTIONS_OFF();
/*N*/ #endif
/*N*/
/*N*/ 	// Defaults setzen
/*N*/
/*N*/ 	// set defaults for LightGroup from ItemPool
/*N*/ 	aLightGroup.SetModelTwoSide(GetTwoSidedLighting());
/*N*/ 	aLightGroup.SetIntensity( GetLightColor1(), Base3DMaterialDiffuse, Base3DLight0);
/*N*/ 	aLightGroup.SetIntensity( GetLightColor2(), Base3DMaterialDiffuse, Base3DLight1);
/*N*/ 	aLightGroup.SetIntensity( GetLightColor3(), Base3DMaterialDiffuse, Base3DLight2);
/*N*/ 	aLightGroup.SetIntensity( GetLightColor4(), Base3DMaterialDiffuse, Base3DLight3);
/*N*/ 	aLightGroup.SetIntensity( GetLightColor5(), Base3DMaterialDiffuse, Base3DLight4);
/*N*/ 	aLightGroup.SetIntensity( GetLightColor6(), Base3DMaterialDiffuse, Base3DLight5);
/*N*/ 	aLightGroup.SetIntensity( GetLightColor7(), Base3DMaterialDiffuse, Base3DLight6);
/*N*/ 	aLightGroup.SetIntensity( GetLightColor8(), Base3DMaterialDiffuse, Base3DLight7);
/*N*/ 	aLightGroup.SetGlobalAmbientLight(GetGlobalAmbientColor());
/*N*/ 	aLightGroup.Enable( GetLightOnOff1(), Base3DLight0);
/*N*/ 	aLightGroup.Enable( GetLightOnOff2(), Base3DLight1);
/*N*/ 	aLightGroup.Enable( GetLightOnOff3(), Base3DLight2);
/*N*/ 	aLightGroup.Enable( GetLightOnOff4(), Base3DLight3);
/*N*/ 	aLightGroup.Enable( GetLightOnOff5(), Base3DLight4);
/*N*/ 	aLightGroup.Enable( GetLightOnOff6(), Base3DLight5);
/*N*/ 	aLightGroup.Enable( GetLightOnOff7(), Base3DLight6);
/*N*/ 	aLightGroup.Enable( GetLightOnOff8(), Base3DLight7);
/*N*/ 	aLightGroup.SetDirection( GetLightDirection1(), Base3DLight0);
/*N*/ 	aLightGroup.SetDirection( GetLightDirection2(), Base3DLight1);
/*N*/ 	aLightGroup.SetDirection( GetLightDirection3(), Base3DLight2);
/*N*/ 	aLightGroup.SetDirection( GetLightDirection4(), Base3DLight3);
/*N*/ 	aLightGroup.SetDirection( GetLightDirection5(), Base3DLight4);
/*N*/ 	aLightGroup.SetDirection( GetLightDirection6(), Base3DLight5);
/*N*/ 	aLightGroup.SetDirection( GetLightDirection7(), Base3DLight6);
/*N*/ 	aLightGroup.SetDirection( GetLightDirection8(), Base3DLight7);
/*N*/
/*N*/ 	bDither = rDefault.GetDefaultDither();
/*N*/
/*N*/ 	// Alte Werte initialisieren
/*N*/ 	aCamera.SetViewWindow(-2, -2, 4, 4);
/*N*/ 	aCameraSet.SetDeviceRectangle(-2, 2, -2, 2);
/*N*/ 	aCamera.SetDeviceWindow(Rectangle(0, 0, 10, 10));
/*N*/ 	Rectangle aRect(0, 0, 10, 10);
/*N*/ 	aCameraSet.SetViewportRectangle(aRect);
/*N*/ 	nSortingMode = E3D_SORT_FAST_SORTING | E3D_SORT_IN_PARENTS | E3D_SORT_TEST_LENGTH;
/*N*/
/*N*/ 	// set defaults for Camera from ItemPool
/*N*/ 	aCamera.SetProjection(GetPerspective());
/*N*/ 	Vector3D aActualPosition = aCamera.GetPosition();
/*N*/ 	double fNew = GetDistance();
/*N*/ 	if(fabs(fNew - aActualPosition.Z()) > 1.0)
/*N*/ 		aCamera.SetPosition( Vector3D( aActualPosition.X(), aActualPosition.Y(), fNew) );
/*N*/ 	fNew = GetFocalLength() / 100.0;
/*N*/ 	aCamera.SetFocalLength(fNew);
/*N*/ }

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

/*N*/ E3dScene::~E3dScene()
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|* SetSnapRect
|*
\************************************************************************/

/*N*/ void E3dScene::NbcSetSnapRect(const Rectangle& rRect)
/*N*/ {
/*N*/ 	SetRectsDirty();
/*N*/ 	E3dObject::NbcSetSnapRect(rRect);
/*N*/ 	aCamera.SetDeviceWindow(rRect);
/*N*/ 	aCameraSet.SetViewportRectangle((Rectangle&)rRect);
/*N*/ }

/*************************************************************************
|*
|* Objekt Resizen
|*
\************************************************************************/

/*N*/ void E3dScene::NbcResize(const Point& rRef, const Fraction& rXFact,
/*N*/ 											const Fraction& rYFact)
/*N*/ {
/*N*/ 	Rectangle aNewSnapRect = GetSnapRect();
/*N*/ 	ResizeRect(aNewSnapRect, rRef, rXFact, rYFact);
/*N*/ 	NbcSetSnapRect(aNewSnapRect);
/*N*/ }

/*************************************************************************
|*
|* Neue Kamera setzen, und dabei die Szene und ggf. das BoundVolume
|* als geaendert markieren
|*
\************************************************************************/

/*N*/ void E3dScene::SetCamera(const Camera3D& rNewCamera)
/*N*/ {
/*N*/ 	// Alte Kamera setzen
/*N*/ 	aCamera = rNewCamera;
/*N*/ 	ImpSetSceneItemsFromCamera();
/*N*/ 	SetRectsDirty();
/*N*/
/*N*/ 	// Neue Kamera aus alter fuellen
/*N*/ 	Camera3D& rCam = (Camera3D&)GetCamera();
/*N*/
/*N*/ 	// Ratio abschalten
/*N*/ 	if(rCam.GetAspectMapping() == AS_NO_MAPPING)
/*N*/ 		GetCameraSet().SetRatio(0.0);
/*N*/
/*N*/ 	// Abbildungsgeometrie setzen
/*N*/ 	Vector3D aVRP = rCam.GetViewPoint();
/*N*/ 	Vector3D aVPN = aVRP - rCam.GetVRP();
/*N*/ 	Vector3D aVUV = rCam.GetVUV();
/*N*/
/*N*/ 	// #91047# use SetViewportValues() to set VRP, VPN and VUV as vectors, too.
/*N*/ 	// Else these values would not be exported/imported correctly.
/*N*/ 	//GetCameraSet().SetOrientation(aVRP, aVPN, aVUV);
/*N*/ 	GetCameraSet().SetViewportValues(aVRP, aVPN, aVUV);
/*N*/
/*N*/ 	// Perspektive setzen
/*N*/ 	GetCameraSet().SetPerspective(rCam.GetProjection() == PR_PERSPECTIVE);
/*N*/ 	GetCameraSet().SetViewportRectangle((Rectangle&)rCam.GetDeviceWindow());
/*N*/
/*N*/ 	// E3dLabel-Objekte muessen neu an die Projektion angepasst werden
/*N*/ 	if ( aLabelList.Count() > 0 )
/*N*/ 	{
/*N*/ 		SetBoundVolInvalid();
/*N*/ 		SetRectsDirty();
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* 3D-Objekt einfuegen
|*
\************************************************************************/

/*N*/ void E3dScene::NewObjectInserted(const E3dObject* p3DObj)
/*N*/ {
/*N*/ 	E3dObject::NewObjectInserted(p3DObj);
/*N*/
/*N*/ 	if ( p3DObj == this )
/*N*/ 		return;
/*N*/
/*N*/ 	if ( p3DObj->ISA(E3dLabelObj) )
/*N*/ 	{
/*N*/ 		aLabelList.Insert((E3dLabelObj*) p3DObj, LIST_APPEND);
/*N*/ 	}
/*N*/
/*N*/ 	// falls Unterobjekte vorhanden sind, auch diese pruefen
/*N*/ 	if ( p3DObj->IsGroupObject() )
/*N*/ 	{
/*N*/ 		SdrObjListIter a3DIterator(*p3DObj, IM_DEEPWITHGROUPS);
/*N*/
/*N*/ 		while ( a3DIterator.IsMore() )
/*N*/ 		{
/*N*/ 			SdrObject* pObj = a3DIterator.Next();
/*N*/
/*N*/ 			if ( pObj->ISA(E3dLabelObj) )
/*N*/ 			{
/*N*/ 				aLabelList.Insert((E3dLabelObj*) pObj, LIST_APPEND);
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Parent ueber Aenderung eines Childs informieren
|*
\************************************************************************/

/*N*/ void E3dScene::StructureChanged(const E3dObject* p3DObj)
/*N*/ {
/*N*/ 	E3dObject::StructureChanged(p3DObj);
/*N*/ 	SetRectsDirty();
/*N*/ }

/*************************************************************************
|*
|* Einpassen der Projektion aller Szenenobjekte in das
|* umschliessende Rechteck
|*
\************************************************************************/

/*N*/ Volume3D E3dScene::FitInSnapRect()
/*N*/ {
/*N*/ 	// Alter Kram
/*N*/ 	Matrix4D aFullTrans = GetFullTransform();
/*N*/ 	aCamera.FitViewToVolume(GetBoundVolume(), aFullTrans);
/*N*/
/*N*/ 	// Neuer Kram
/*N*/ 	// Maximas holen in Augkoordinaten zwecks Z-Werten
/*N*/ 	Volume3D aNewVol;
/*N*/ 	Vector3D aTfVec;
/*N*/ 	Vol3DPointIterator aIter(GetBoundVolume());
/*N*/
/*N*/ 	GetCameraSet().SetObjectTrans(aFullTrans);
/*N*/ 	while ( aIter.Next(aTfVec) )
/*N*/ 	{
/*N*/ 		aTfVec = GetCameraSet().ObjectToEyeCoor(aTfVec);
/*N*/ 		aNewVol.Union(aTfVec);
/*N*/ 	}
/*N*/
/*N*/ 	// ... und merken
/*N*/ 	double fZMin = -aNewVol.MaxVec().Z();
/*N*/ 	double fZMax = -aNewVol.MinVec().Z();
/*N*/
/*N*/ 	// Jetzt XY-Werte projizieren auf Projektionsflaeche
/*N*/ 	// in Device-Koordinaten
/*N*/ 	Matrix4D aWorldToDevice = GetCameraSet().GetOrientation();
/*N*/ 	if(aCamera.GetProjection() == PR_PERSPECTIVE)
/*N*/ 		aWorldToDevice.Frustum(-1.0, 1.0, -1.0, 1.0, fZMin, fZMax);
/*N*/ 	else
/*N*/ 		aWorldToDevice.Ortho(-1.0, 1.0, -1.0, 1.0, fZMin, fZMax);
/*N*/ 	aNewVol.Reset();
/*N*/ 	aIter.Reset();
/*N*/ 	while ( aIter.Next(aTfVec) )
/*N*/ 	{
/*N*/ 		aTfVec = GetCameraSet().ObjectToWorldCoor(aTfVec);
/*N*/ 		aTfVec *= aWorldToDevice;
/*N*/ 		aNewVol.Union(aTfVec);
/*N*/ 	}
/*N*/
/*N*/ 	// Labels behandeln
/*N*/ 	ULONG nLabelCnt = aLabelList.Count();
/*N*/ 	if ( nLabelCnt > 0 )
/*N*/ 	{
/*N*/ 		// Vorlaeufige Projektion bestimmen und Transformation in
/*N*/ 		// ViewKoordinaten bestimmen
/*N*/ 		Matrix4D aMatWorldToView = GetCameraSet().GetOrientation();
/*N*/ 		if(aCamera.GetProjection() == PR_PERSPECTIVE)
/*N*/ 			aMatWorldToView.Frustum(aNewVol.MinVec().X(), aNewVol.MaxVec().X(),
/*N*/ 			aNewVol.MinVec().Y(), aNewVol.MaxVec().Y(), fZMin, fZMax);
/*N*/ 		else
/*N*/ 			aMatWorldToView.Ortho(aNewVol.MinVec().X(), aNewVol.MaxVec().X(),
/*N*/ 			aNewVol.MinVec().Y(), aNewVol.MaxVec().Y(), fZMin, fZMax);
/*N*/
/*N*/ 		// Logische Abmessungen der Szene holen
/*N*/ 		Rectangle aSceneRect = GetSnapRect();
/*N*/
/*N*/ 		// Matrix DeviceToView aufbauen
/*N*/ 		Vector3D aTranslate, aScale;
/*N*/
/*N*/ 		aTranslate[0] = (double)aSceneRect.Left() + (aSceneRect.GetWidth() / 2.0);
/*N*/ 		aTranslate[1] = (double)aSceneRect.Top() + (aSceneRect.GetHeight() / 2.0);
/*N*/ 		aTranslate[2] = ZBUFFER_DEPTH_RANGE / 2.0;
/*N*/
/*N*/ 		// Skalierung
/*N*/ 		aScale[0] = (aSceneRect.GetWidth() - 1) / 2.0;
/*N*/ 		aScale[1] = (aSceneRect.GetHeight() - 1) / -2.0;
/*N*/ 		aScale[2] = ZBUFFER_DEPTH_RANGE / 2.0;
/*N*/
/*N*/ 		aMatWorldToView.Scale(aScale);
/*N*/ 		aMatWorldToView.Translate(aTranslate);
/*N*/
/*N*/ 		// Inverse Matrix ViewToDevice aufbauen
/*N*/ 		Matrix4D aMatViewToWorld(aMatWorldToView);
/*N*/ 		aMatViewToWorld.Invert();
/*N*/
/*N*/ 		for (ULONG i = 0; i < nLabelCnt; i++)
/*N*/ 		{
/*N*/ 			E3dLabelObj* p3DObj = aLabelList.GetObject(i);
/*N*/ 			const SdrObject* pObj = p3DObj->Get2DLabelObj();
/*N*/
/*N*/ 			// View- Abmessungen des Labels holen
/*N*/ 			const Rectangle& rObjRect = pObj->GetLogicRect();
/*N*/
/*N*/ 			// Position des Objektes in Weltkoordinaten ermitteln
/*N*/ 			Matrix4D aObjTrans = p3DObj->GetFullTransform();
/*N*/ 			Vector3D aObjPos = aObjTrans * p3DObj->GetPosition();
/*N*/
/*N*/ 			// View-Position des Objektes feststellen
/*N*/ 			// nach ViewKoordinaten
/*N*/ 			aObjPos *= aMatWorldToView;
/*N*/
/*N*/ 			// Relative Position des Labels in View-Koordinaten
/*N*/ 			Vector3D aRelPosOne(pObj->GetRelativePos(), aObjPos.Z());
/*N*/ 			aRelPosOne.X() += aObjPos.X();
/*N*/ 			aRelPosOne.Y() += aObjPos.Y();
/*N*/ 			Vector3D aRelPosTwo(aRelPosOne);
/*N*/ 			aRelPosTwo.X() += rObjRect.GetWidth();
/*N*/ 			aRelPosTwo.Y() += rObjRect.GetHeight();
/*N*/
/*N*/ 			// Jetzt Eckpunkte in DeviceKoordinaten bestimmen und
/*N*/ 			// den Abmessungen hinzufuegen
/*N*/ 			aRelPosOne *= aMatViewToWorld;
/*N*/ 			aRelPosOne *= aWorldToDevice;
/*N*/ 			aNewVol.Union(aRelPosOne);
/*N*/
/*N*/ 			aRelPosTwo *= aMatViewToWorld;
/*N*/ 			aRelPosTwo *= aWorldToDevice;
/*N*/ 			aNewVol.Union(aRelPosTwo);
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	// Z-Werte eintragen
/*N*/ 	aNewVol.MinVec().Z() = fZMin;
/*N*/ 	aNewVol.MaxVec().Z() = fZMax;
/*N*/
/*N*/ 	// Rueckgabewert setzen
/*N*/ 	return aNewVol;
/*N*/ }

/*************************************************************************
|*
|* Uebergeordnetes Szenenobjekt bestimmen
|*
\************************************************************************/

/*N*/ E3dScene* E3dScene::GetScene() const
/*N*/ {
/*N*/ 	if(GetParentObj())
/*N*/ 		return GetParentObj()->GetScene();
/*N*/ 	else
/*N*/ 		return (E3dScene*)this;
/*N*/ }

/*************************************************************************
|*
|* TransformationSet vorbereiten
|*
\************************************************************************/

/*N*/ void E3dScene::InitTransformationSet()
/*N*/ {
/*N*/ 	Rectangle aBound(GetSnapRect());
/*N*/
/*N*/ 	// GeometricSet reset und mit pBase3D assoziieren
/*N*/ 	B3dCamera& rSet = GetCameraSet();
/*N*/
/*N*/ 	// Transformation auf Weltkoordinaten holen
/*N*/ 	Matrix4D mTransform = GetFullTransform();
/*N*/ 	rSet.SetObjectTrans(mTransform);
/*N*/
/*N*/ 	// 3D Ausgabe vorbereiten, Maximas holen in DeviceKoordinaten
/*N*/ 	Volume3D aVolume = FitInSnapRect();
/*N*/
/*N*/ 	// Maximas fuer Abbildung verwenden
/*N*/ 	rSet.SetDeviceVolume(aVolume, FALSE);
/*N*/ 	rSet.SetViewportRectangle(aBound);
/*N*/ }

/*************************************************************************
|*
|* sichern mit neuer Methode und zukunftskompatibilitaet
|* Die Zahl 3560 ist die Major-Update-Nummer * 10 zu der die Umstellung
|* erfolgte. Dies ist leider das korrekte Verhalten, die 3d-Engine hat keine
|* eigene Versionsnummer sondern ist an die der Drawing-Engine gekoppelt.
|* Probleme gibt es immer dann wenn einen neue Version ein altes Format
|* schreiben soll: Hier wird von der Drawing-Engine trotzdem die neue Nummer
|* verwendet.
|*
\************************************************************************/

/*N*/ void E3dScene::WriteData(SvStream& rOut) const
/*N*/ {
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	long nVersion = rOut.GetVersion(); // Build_Nr * 10 z.B. 3810
/*N*/ 	if(nVersion < 3830)
/*N*/ 	{
/*N*/ 		// Hier die Lichtobjekte erzeugen, um im alten Format schreiben zu koennen
/*N*/ 		((E3dScene*)(this))->CreateLightObjectsFromLightGroup();
/*N*/ 	}
/*N*/
/*N*/ 	// Schreiben
/*N*/ 	E3dObject::WriteData(rOut);
/*N*/
/*N*/ 	if(nVersion < 3830)
/*N*/ 	{
/*N*/ 		// Lichtobjekte wieder wegnehmen
/*N*/ 		((E3dScene*)(this))->RemoveLightObjects();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ #ifdef E3D_STREAMING
/*N*/ 		SdrDownCompat aCompat(rOut, STREAM_WRITE);
/*N*/ #ifdef DBG_UTIL
/*N*/ 		aCompat.SetID("B3dLightGroup");
/*N*/ #endif
/*N*/ 		// LightGroup schreiben
/*N*/ 		aLightGroup.WriteData(rOut);
/*N*/
/*N*/ #endif
/*N*/ 	}
/*N*/
/*N*/ #ifdef E3D_STREAMING
/*N*/ 	SdrDownCompat aCompat(rOut, STREAM_WRITE);
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("E3dScene");
/*N*/ #endif
/*N*/
/*N*/ 	DBG_ASSERT (rOut.GetVersion(),"3d-Engine: Keine Version am Stream gesetzt!");
/*N*/ 	if (rOut.GetVersion() < 3560) // FG: Das ist der Zeitpunkt der Umstellung
/*N*/ 	{
/*N*/ 		rOut << aCamera;
/*N*/ 	}
/*N*/ 	if (rOut.GetVersion() >= 3560)
/*N*/ 	{
/*N*/ 		aCamera.WriteData(rOut);
/*N*/ 	}
/*N*/
/*N*/ 	rOut << BOOL(bDoubleBuffered);
/*N*/ 	rOut << BOOL(bClipping);
/*N*/ 	rOut << BOOL(bFitInSnapRect);
/*N*/ 	rOut << nSortingMode;
/*N*/
/*N*/ 	// neu ab 377:
/*N*/ 	Vector3D aPlaneDirection = GetShadowPlaneDirection();
/*N*/ 	rOut << aPlaneDirection;
/*N*/
/*N*/ 	// neu ab 383:
/*N*/ 	rOut << (BOOL)bDither;
/*N*/
/*N*/ 	// neu ab 384:
/*N*/ 	sal_uInt16 nShadeMode = GetShadeMode();
/*N*/ 	if(nShadeMode == 0)
/*N*/ 		rOut << (sal_uInt16)Base3DFlat;
/*N*/ 	else if(nShadeMode == 1)
/*N*/ 		rOut << (sal_uInt16)Base3DPhong;
/*N*/ 	else
/*N*/ 		rOut << (sal_uInt16)Base3DSmooth;
/*N*/ 	rOut << (BOOL)(nShadeMode > 2);
/*N*/
/*N*/ #endif
/*N*/ #endif	// #ifndef SVX_LIGHT
/*N*/ }

/*************************************************************************
|*
|* laden
|*
\************************************************************************/

/*N*/ void E3dScene::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
/*N*/ {
/*N*/ 	if (ImpCheckSubRecords (rHead, rIn))
/*N*/ 	{
/*N*/ 		E3dObject::ReadData(rHead, rIn);
/*N*/
/*N*/ 		if(CountNumberOfLights())
/*N*/ 		{
/*N*/ 			// An dieser Stelle die gelesenen Lampen ausmerzen
/*N*/ 			// und in die neue Struktur ueberfuehren
/*N*/ 			FillLightGroup();
/*N*/ 			RemoveLightObjects();
/*N*/ 		}
/*N*/ 		long nVersion = rIn.GetVersion(); // Build_Nr * 10 z.B. 3810
/*N*/ 		if(nVersion >= 3830)
/*N*/ 		{
/*N*/ 			SdrDownCompat aCompat(rIn, STREAM_READ);
/*N*/ #ifdef DBG_UTIL
/*N*/ 			aCompat.SetID("B3dLightGroup");
/*N*/ #endif
/*N*/ 			if(aCompat.GetBytesLeft())
/*N*/ 			{
/*N*/ 				// LightGroup lesen
/*N*/ 				aLightGroup.ReadData(rIn);
/*N*/ 			}
/*N*/ 		}
/*N*/
/*N*/ 		SdrDownCompat aCompat(rIn, STREAM_READ);
/*N*/ #ifdef DBG_UTIL
/*N*/ 		aCompat.SetID("E3dScene");
/*N*/ #endif
/*N*/ 		BOOL bTmp;
/*N*/
/*N*/ 		DBG_ASSERT (rIn.GetVersion(),"3d-Engine: Keine Version am Stream gesetzt!");
/*N*/
/*N*/ 		if ((rIn.GetVersion() < 3560) || (rHead.GetVersion() <= 12))
/*N*/ 		{
/*N*/ 			rIn >> aCamera;
/*N*/ 		}
/*N*/ 		if ((rIn.GetVersion() >= 3560) && (rHead.GetVersion() >= 13))
/*N*/ 		{
/*N*/ 			aCamera.ReadData(rHead, rIn);
/*N*/ 		}
/*N*/
/*N*/ 		// Neue Kamera aus alter fuellen
/*N*/ 		Camera3D& rCam = (Camera3D&)GetCamera();
/*N*/
/*N*/ 		// Ratio abschalten
/*N*/ 		if(rCam.GetAspectMapping() == AS_NO_MAPPING)
/*N*/ 			GetCameraSet().SetRatio(0.0);
/*N*/
/*N*/ 		// Abbildungsgeometrie setzen
/*N*/ 		Vector3D aVRP = rCam.GetViewPoint();
/*N*/ 		Vector3D aVPN = aVRP - rCam.GetVRP();
/*N*/ 		Vector3D aVUV = rCam.GetVUV();
/*N*/ 		GetCameraSet().SetOrientation(aVRP, aVPN, aVUV);
/*N*/
/*N*/ 		// Perspektive setzen
/*N*/ 		GetCameraSet().SetPerspective(rCam.GetProjection() == PR_PERSPECTIVE);
/*N*/ 		GetCameraSet().SetViewportRectangle((Rectangle&)rCam.GetDeviceWindow());
/*N*/
/*N*/ 		rIn >> bTmp; bDoubleBuffered = bTmp;
/*N*/ 		rIn >> bTmp; bClipping = bTmp;
/*N*/ 		rIn >> bTmp; bFitInSnapRect = bTmp;
/*N*/
/*N*/ 		if (aCompat.GetBytesLeft() >= sizeof(UINT32))
/*N*/ 		{
/*N*/ 			rIn >> nSortingMode;
/*N*/ 		}
/*N*/
/*N*/ 		// neu ab 377:
/*N*/ 		if (aCompat.GetBytesLeft() >= sizeof(Vector3D))
/*N*/ 		{
/*N*/ 			Vector3D aShadowVec;
/*N*/ 			rIn >> aShadowVec;
/*N*/ 			SetShadowPlaneDirection(aShadowVec);
/*N*/ 		}
/*N*/
/*N*/ 		// neu ab 383:
/*N*/ 		if (aCompat.GetBytesLeft() >= sizeof(BOOL))
/*N*/ 		{
/*N*/ 			rIn >> bTmp; bDither = bTmp;
/*N*/ 		}
/*N*/
/*N*/ 		// neu ab 384:
/*N*/ 		if (aCompat.GetBytesLeft() >= sizeof(UINT16))
/*N*/ 		{
/*N*/ 			UINT16 nTmp;
/*N*/ 			rIn >> nTmp;
/*N*/ 			if(nTmp == (Base3DShadeModel)Base3DFlat)
/*N*/ 				mpObjectItemSet->Put(Svx3DShadeModeItem(0));
/*N*/ 			else if(nTmp == (Base3DShadeModel)Base3DPhong)
/*N*/ 				mpObjectItemSet->Put(Svx3DShadeModeItem(1));
/*N*/ 			else
/*N*/ 				mpObjectItemSet->Put(Svx3DShadeModeItem(2));
/*N*/ 		}
/*N*/ 		if (aCompat.GetBytesLeft() >= sizeof(BOOL))
/*N*/ 		{
/*N*/ 			rIn >> bTmp;
/*N*/ 			if(bTmp)
/*N*/ 				mpObjectItemSet->Put(Svx3DShadeModeItem(3));
/*N*/ 		}
/*N*/
/*N*/ 		// SnapRects der Objekte ungueltig
/*N*/ 		SetRectsDirty();
/*N*/
/*N*/ 		// Transformationen initialisieren, damit bei RecalcSnapRect()
/*N*/ 		// richtig gerechnet wird
/*N*/ 		InitTransformationSet();
/*N*/
/*N*/ 		RebuildLists();
/*N*/
/*N*/ 		// set items from combined read objects like lightgroup and camera
/*N*/ 		ImpSetLightItemsFromLightGroup();
/*N*/ 		ImpSetSceneItemsFromCamera();
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Einpassen der Objekte in umschliessendes Rechteck aus-/einschalten
|*
\************************************************************************/

/*N*/ void E3dScene::FitSnapRectToBoundVol()
/*N*/ {
/*N*/ 	Vector3D aTfVec;
/*N*/ 	Volume3D aFitVol;
/*N*/
/*N*/ 	SetBoundVolInvalid();
/*N*/ 	Matrix4D aTransform = GetFullTransform() * aCamera.GetViewTransform();
/*N*/ 	Vol3DPointIterator aIter(GetBoundVolume(), &aTransform);
/*N*/ 	Rectangle aRect;
/*N*/
/*N*/ 	while ( aIter.Next(aTfVec) )
/*N*/ 	{
/*N*/ 		aCamera.DoProjection(aTfVec);
/*N*/ 		aFitVol.Union(aTfVec);
/*N*/ 		Vector3D aZwi = aCamera.MapToDevice(aTfVec);
/*N*/ 		Point aP((long)aZwi.X(), (long)aZwi.Y());
/*N*/ 		aRect.Union(Rectangle(aP, aP));
/*N*/ 	}
/*N*/ 	aCamera.SetViewWindow(aFitVol.MinVec().X(), aFitVol.MinVec().Y(),
/*N*/ 		aFitVol.GetWidth(), aFitVol.GetHeight());
/*N*/ 	SetSnapRect(aRect);
/*N*/
/*N*/ 	// Die SnapRects aller beteiligten Objekte muessen auf dieser
/*N*/ 	// veraenderten Basis aufgebaut werden, invalidiere diese. Das
/*N*/ 	// eigene kann auch invalidiert werden, da ein RecalcSnapRect
/*N*/ 	// an einer Szene nur aus der Kamera liest
/*N*/ 	SetRectsDirty();
/*N*/ }

/*************************************************************************
|*
|* Falls die Geometrie einer Szene sich ausgedehnt/vermindert hat,
|* muss das Volume und das SnapRect angepasst werden
|*
\************************************************************************/

/*N*/ void E3dScene::CorrectSceneDimensions()
/*N*/ {
/*N*/ 	// SnapRects der Objekte ungueltig
/*N*/ 	SetRectsDirty();
/*N*/
/*N*/ 	// SnapRect anpassen, invalidiert auch die SnapRects
/*N*/ 	// der enthaltenen Objekte
/*N*/ 	FitSnapRectToBoundVol();
/*N*/
/*N*/ 	// Neues BoundVolume der Kamera holen
/*N*/ 	Volume3D aVolume = FitInSnapRect();
/*N*/
/*N*/ 	// Neues BoundVolume an der Kamera setzen
/*N*/ 	GetCameraSet().SetDeviceVolume(aVolume, FALSE);
/*N*/
/*N*/ 	// Danach noch die SnapRects der enthaltenen Objekte
/*N*/ 	// invalidieren, um diese auf der neuen Grundlage berechnen
/*N*/ 	// zu lassen (falls diese von FitInSnapRect() berechnet wurden)
/*N*/ 	SetRectsDirty();
/*N*/ }

/*************************************************************************
|*
|* Licht- und Labelobjektlisten neu aufbauen (nach Laden, Zuweisung)
|*
\************************************************************************/

/*N*/ void E3dScene::RebuildLists()
/*N*/ {
/*N*/ 	// zuerst loeschen
/*N*/ 	aLabelList.Clear();
/*N*/ 	SdrLayerID nLayerID = GetLayer();
/*N*/
/*N*/ 	SdrObjListIter a3DIterator(*pSub, IM_FLAT);
/*N*/
/*N*/ 	// dann alle Objekte in der Szene pruefen
/*N*/ 	while ( a3DIterator.IsMore() )
/*N*/ 	{
/*N*/ 		E3dObject* p3DObj = (E3dObject*) a3DIterator.Next();
/*N*/ 		p3DObj->NbcSetLayer(nLayerID);
/*N*/ 		NewObjectInserted(p3DObj);
/*N*/ 	}
/*N*/
/*N*/ }

/*************************************************************************
|*
|* erstelle neues GeoData-Objekt
|*
\************************************************************************/

/*N*/ SdrObjGeoData *E3dScene::NewGeoData() const
/*N*/ {
/*N*/ 	return new E3DSceneGeoData;
/*N*/ }

/*************************************************************************
|*
|* uebergebe aktuelle werte an das GeoData-Objekt
|*
\************************************************************************/

/*?*/ void E3dScene::SaveGeoData(SdrObjGeoData& rGeo) const
/*?*/ {
/*?*/ 	E3dObject::SaveGeoData (rGeo);
/*?*/
/*?*/ 	((E3DSceneGeoData &) rGeo).aCamera                = aCamera;
/*?*/ 	((E3DSceneGeoData &) rGeo).aLabelList             = aLabelList;
/*?*/ }

/*************************************************************************
|*
|* uebernehme werte aus dem GeoData-Objekt
|*
\************************************************************************/

/*?*/ void E3dScene::RestGeoData(const SdrObjGeoData& rGeo)
/*?*/ {
/*?*/ 	E3dObject::RestGeoData (rGeo);
/*?*/
/*?*/ 	aLabelList = ((E3DSceneGeoData &) rGeo).aLabelList;
/*?*/ 	SetCamera (((E3DSceneGeoData &) rGeo).aCamera);
/*?*/ 	FitSnapRectToBoundVol();
/*?*/ }

/*************************************************************************
|*
|* Am StyleSheet wurde etwas geaendert, also Scene aendern
|*
\************************************************************************/

/*N*/ void E3dScene::SFX_NOTIFY(SfxBroadcaster &rBC,
/*N*/ 						  const TypeId   &rBCType,
/*N*/ 						  const SfxHint  &rHint,
/*N*/ 						  const TypeId   &rHintType)
/*N*/ {
/*N*/ 	SetRectsDirty();
/*N*/ 	E3dObject::SFX_NOTIFY(rBC, rBCType, rHint, rHintType);
/*N*/ }

/*************************************************************************
|*
|* Compounds brauchen Defaults
|*
\************************************************************************/

/*N*/ void E3dScene::ForceDefaultAttr()
/*N*/ {
/*N*/ 	SdrAttrObj::ForceDefaultAttr();
/*N*/ }

/*************************************************************************
|*
|* Attribute setzen
|*
\************************************************************************/

/*N*/ void E3dScene::NbcSetStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr)
/*N*/ {
/*N*/ 	E3dObjList* pOL = pSub;
/*N*/ 	ULONG nObjCnt = pOL->GetObjCount();
/*N*/
/*N*/ 	for ( ULONG i = 0; i < nObjCnt; i++ )
/*?*/ 		pOL->GetObj(i)->NbcSetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);
/*N*/
/*N*/ 	StructureChanged(this);
/*N*/ }

/*************************************************************************
|*
|* Attribute abfragen
|*
\************************************************************************/

/*N*/ SfxStyleSheet* E3dScene::GetStyleSheet() const
/*N*/ {
/*N*/ 	E3dObjList    *pOL          = pSub;
/*N*/ 	ULONG         nObjCnt       = pOL->GetObjCount();
/*N*/ 	SfxStyleSheet *pRet         = 0;
/*N*/
/*N*/ 	for ( ULONG i = 0; i < nObjCnt; i++ )
/*N*/ 	{
/*N*/ 		SfxStyleSheet *pSheet =	pOL->GetObj(i)->GetStyleSheet();
/*N*/
/*N*/ 		if (!pRet)
/*N*/ 			pRet = pSheet;
/*N*/ 		else if (pSheet)
/*N*/ 		{
/*N*/ 			if(!pSheet->GetName().Equals(pRet->GetName()))
/*N*/ 				return 0;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return pRet;
/*N*/ }

/*************************************************************************
|*
|* Licht-Objekte rauswerfen
|*
\************************************************************************/

/*N*/ void E3dScene::RemoveLightObjects()
/*N*/ {
/*N*/ 	SdrObjList* pSubList = GetSubList();
/*N*/ 	if(pSubList)
/*N*/ 	{
/*N*/ 		SdrObjListIter a3DIterator(*pSubList, IM_DEEPWITHGROUPS);
/*N*/ 		while ( a3DIterator.IsMore() )
/*N*/ 		{
/*N*/ 			E3dObject* pObj = (E3dObject*) a3DIterator.Next();
/*N*/ 			DBG_ASSERT(pObj->ISA(E3dObject), "AW: In Szenen sind nur 3D-Objekte erlaubt!");
/*N*/ 			if(pObj->ISA(E3dLight))
/*N*/ 			{
/*N*/ 				// Weg damit
/*N*/ 				Remove3DObj(pObj);
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Licht-Objekte erzeugen, um kompatibel zur 4.0
|* speichern zu koennen
|*
\************************************************************************/

/*N*/ void E3dScene::CreateLightObjectsFromLightGroup()
/*N*/ {
/*N*/ 	if(aLightGroup.IsLightingEnabled())
/*N*/ 	{
/*N*/ 		// Global Ambient Light
/*N*/ 		const Color& rAmbient = aLightGroup.GetGlobalAmbientLight();
/*N*/ 		if(rAmbient != Color(COL_BLACK))
/*N*/ 			Insert3DObj(new E3dLight(Vector3D(), rAmbient, 1.0));
/*N*/
/*N*/ 		// Andere Lichter
/*N*/ 		for(UINT16 a=0;a<BASE3D_MAX_NUMBER_LIGHTS;a++)
/*N*/ 		{
/*N*/ 			B3dLight& rLight = aLightGroup.GetLightObject((Base3DLightNumber)(Base3DLight0 + a));
/*N*/ 			if(rLight.IsEnabled())
/*N*/ 			{
/*N*/ 				if(rLight.IsDirectionalSource())
/*N*/ 				{
/*N*/ 					// erzeuge E3dDistantLight
/*N*/ 					Insert3DObj(new E3dDistantLight(Vector3D(),
/*N*/ 						rLight.GetPosition(),
/*N*/ 						rLight.GetIntensity(Base3DMaterialDiffuse), 1.0));
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					// erzeuge E3dPointLight
/*N*/ 					Insert3DObj(new E3dPointLight(rLight.GetPosition(),
/*N*/ 						rLight.GetIntensity(Base3DMaterialDiffuse), 1.0));
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Beleuchtung aus dem alten Beleuchtungsmodell uebernehmen
|*
\************************************************************************/

/*N*/ void E3dScene::FillLightGroup()
/*N*/ {
/*N*/ 	SdrObjList* pSubList = GetSubList();
/*N*/ 	BOOL bLampFound = FALSE;
/*N*/
/*N*/ 	if(pSubList)
/*N*/ 	{
/*N*/ 		SdrObjListIter a3DIterator(*pSubList, IM_DEEPWITHGROUPS);
/*N*/ 		Base3DLightNumber eLight = Base3DLight0;
/*N*/
/*N*/ 		// AmbientLight aus
/*N*/ 		aLightGroup.SetGlobalAmbientLight(Color(COL_BLACK));
/*N*/
/*N*/ 		while ( a3DIterator.IsMore() )
/*N*/ 		{
/*N*/ 			E3dObject* pObj = (E3dObject*) a3DIterator.Next();
/*N*/ 			DBG_ASSERT(pObj->ISA(E3dObject), "AW: In Szenen sind nur 3D-Objekte erlaubt!");
/*N*/ 			if(pObj->ISA(E3dLight) && eLight <= Base3DLight7)
/*N*/ 			{
/*N*/ 				E3dLight* pLight = (E3dLight*)pObj;
/*N*/ 				bLampFound = TRUE;
/*N*/
/*N*/ 				// pLight in Base3D Konvention aktivieren
/*N*/ 				if(pLight->IsOn())
/*N*/ 				{
/*?*/ 					if(pLight->ISA(E3dPointLight))
/*?*/ 					{
/*?*/ 						// ist ein E3dPointLight
/*?*/ 						// Position, keine Richtung
/*?*/ 						B3dColor aCol(pLight->GetColor().GetColor());
/*?*/ 						aCol *= pLight->GetIntensity();
/*?*/ 						aLightGroup.SetIntensity(aCol, Base3DMaterialDiffuse, eLight);
/*?*/ 						aLightGroup.SetIntensity(Color(COL_WHITE), Base3DMaterialSpecular, eLight);
/*?*/ 						Vector3D aPos = pLight->GetPosition();
/*?*/ 						aLightGroup.SetPosition(aPos, eLight);
/*?*/
/*?*/ 						// Lichtquelle einschalten
/*?*/ 						aLightGroup.Enable(TRUE, eLight);
/*?*/
/*?*/ 						// Naechstes Licht in Base3D
/*?*/ 						eLight = (Base3DLightNumber)(eLight + 1);
/*?*/ 					}
/*N*/ 					else if(pLight->ISA(E3dDistantLight))
/*N*/ 					{
/*N*/ 						// ist ein E3dDistantLight
/*N*/ 						// Richtung, keine Position
/*N*/ 						B3dColor aCol(pLight->GetColor().GetColor());
/*N*/ 						aCol *= pLight->GetIntensity();
/*N*/ 						aLightGroup.SetIntensity(aCol, Base3DMaterialDiffuse, eLight);
/*N*/ 						aLightGroup.SetIntensity(Color(COL_WHITE), Base3DMaterialSpecular, eLight);
/*N*/ 						Vector3D aDir = ((E3dDistantLight *)pLight)->GetDirection();
/*N*/ 						aLightGroup.SetDirection(aDir, eLight);
/*N*/
/*N*/ 						// Lichtquelle einschalten
/*N*/ 						aLightGroup.Enable(TRUE, eLight);
/*N*/
/*N*/ 						// Naechstes Licht in Base3D
/*N*/ 						eLight = (Base3DLightNumber)(eLight + 1);
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*N*/ 						// nur ein E3dLight, gibt ein
/*N*/ 						// ambientes licht, auf globales aufaddieren
/*N*/ 						B3dColor aCol(pLight->GetColor().GetColor());
/*N*/ 						aCol *= pLight->GetIntensity();
/*N*/ 						aCol += (const B3dColor &)(aLightGroup.GetGlobalAmbientLight());
/*N*/ 						aLightGroup.SetGlobalAmbientLight(aCol);
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/
/*N*/ 		// Alle anderen Lichter ausschalten
/*N*/ 		while(eLight <= Base3DLight7)
/*N*/ 		{
/*N*/ 			aLightGroup.Enable(FALSE, eLight);
/*N*/ 			eLight = (Base3DLightNumber)(eLight + 1);
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	// Beleuchtung einschalten, falls Lampen vorhanden
/*N*/ 	aLightGroup.EnableLighting(bLampFound);
/*N*/ }

/*************************************************************************
|*
|* Lichter zaehlen
|*
\************************************************************************/

/*N*/ UINT16 E3dScene::CountNumberOfLights()
/*N*/ {
/*N*/ 	UINT16 nNumLights = 0;
/*N*/
/*N*/ 	SdrObjList* pSubList = GetSubList();
/*N*/ 	if(pSubList)
/*N*/ 	{
/*N*/ 		SdrObjListIter a3DIterator(*pSubList, IM_DEEPWITHGROUPS);
/*N*/ 		while ( a3DIterator.IsMore() )
/*N*/ 		{
/*N*/ 			E3dObject* pObj = (E3dObject*) a3DIterator.Next();
/*N*/ 			DBG_ASSERT(pObj->ISA(E3dObject), "AW: In Szenen sind nur 3D-Objekte erlaubt!");
/*N*/ 			if(pObj->ISA(E3dLight))
/*N*/ 			{
/*N*/ 				// Zaehlen...
/*N*/ 				nNumLights++;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return nNumLights;
/*N*/ }

/*************************************************************************
|*
|* SnapRect berechnen
|*
\************************************************************************/

/*N*/ void E3dScene::RecalcSnapRect()
/*N*/ {
/*N*/ 	E3dScene* pScene = GetScene();
/*N*/ 	if(pScene == this)
/*N*/ 	{
/*N*/ 		// Szene wird als 2D-Objekt benutzt, nimm SnapRect aus der
/*N*/ 		// 2D Bildschrimdarstellung
/*N*/ 		Camera3D& rCam = (Camera3D&)pScene->GetCamera();
/*N*/ 		maSnapRect = rCam.GetDeviceWindow();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// Szene ist selbst Mitglied einer anderen Szene, hole das
/*N*/ 		// SnapRect als zusammengesetztes Objekt
/*N*/ 		E3dObject::RecalcSnapRect();
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Attribute abfragen
|*
\************************************************************************/

/*N*/ void E3dScene::ImpSetLightItemsFromLightGroup()
/*N*/ {
/*N*/ 	ImpForceItemSet();
/*N*/
/*N*/ 	// TwoSidedLighting
/*N*/ 	mpObjectItemSet->Put(Svx3DTwoSidedLightingItem(aLightGroup.GetModelTwoSide()));
/*N*/
/*N*/ 	// LightColors
/*N*/ 	mpObjectItemSet->Put(Svx3DLightcolor1Item(aLightGroup.GetIntensity(Base3DMaterialDiffuse, Base3DLight0)));
/*N*/ 	mpObjectItemSet->Put(Svx3DLightcolor2Item(aLightGroup.GetIntensity(Base3DMaterialDiffuse, Base3DLight1)));
/*N*/ 	mpObjectItemSet->Put(Svx3DLightcolor3Item(aLightGroup.GetIntensity(Base3DMaterialDiffuse, Base3DLight2)));
/*N*/ 	mpObjectItemSet->Put(Svx3DLightcolor4Item(aLightGroup.GetIntensity(Base3DMaterialDiffuse, Base3DLight3)));
/*N*/ 	mpObjectItemSet->Put(Svx3DLightcolor5Item(aLightGroup.GetIntensity(Base3DMaterialDiffuse, Base3DLight4)));
/*N*/ 	mpObjectItemSet->Put(Svx3DLightcolor6Item(aLightGroup.GetIntensity(Base3DMaterialDiffuse, Base3DLight5)));
/*N*/ 	mpObjectItemSet->Put(Svx3DLightcolor7Item(aLightGroup.GetIntensity(Base3DMaterialDiffuse, Base3DLight6)));
/*N*/ 	mpObjectItemSet->Put(Svx3DLightcolor8Item(aLightGroup.GetIntensity(Base3DMaterialDiffuse, Base3DLight7)));
/*N*/
/*N*/ 	// AmbientColor
/*N*/ 	mpObjectItemSet->Put(Svx3DAmbientcolorItem(aLightGroup.GetGlobalAmbientLight()));
/*N*/
/*N*/ 	// LightOn
/*N*/ 	mpObjectItemSet->Put(Svx3DLightOnOff1Item(aLightGroup.IsEnabled(Base3DLight0)));
/*N*/ 	mpObjectItemSet->Put(Svx3DLightOnOff2Item(aLightGroup.IsEnabled(Base3DLight1)));
/*N*/ 	mpObjectItemSet->Put(Svx3DLightOnOff3Item(aLightGroup.IsEnabled(Base3DLight2)));
/*N*/ 	mpObjectItemSet->Put(Svx3DLightOnOff4Item(aLightGroup.IsEnabled(Base3DLight3)));
/*N*/ 	mpObjectItemSet->Put(Svx3DLightOnOff5Item(aLightGroup.IsEnabled(Base3DLight4)));
/*N*/ 	mpObjectItemSet->Put(Svx3DLightOnOff6Item(aLightGroup.IsEnabled(Base3DLight5)));
/*N*/ 	mpObjectItemSet->Put(Svx3DLightOnOff7Item(aLightGroup.IsEnabled(Base3DLight6)));
/*N*/ 	mpObjectItemSet->Put(Svx3DLightOnOff8Item(aLightGroup.IsEnabled(Base3DLight7)));
/*N*/
/*N*/ 	// LightDirection
/*N*/ 	mpObjectItemSet->Put(Svx3DLightDirection1Item(aLightGroup.GetDirection( Base3DLight0 )));
/*N*/ 	mpObjectItemSet->Put(Svx3DLightDirection2Item(aLightGroup.GetDirection( Base3DLight1 )));
/*N*/ 	mpObjectItemSet->Put(Svx3DLightDirection3Item(aLightGroup.GetDirection( Base3DLight2 )));
/*N*/ 	mpObjectItemSet->Put(Svx3DLightDirection4Item(aLightGroup.GetDirection( Base3DLight3 )));
/*N*/ 	mpObjectItemSet->Put(Svx3DLightDirection5Item(aLightGroup.GetDirection( Base3DLight4 )));
/*N*/ 	mpObjectItemSet->Put(Svx3DLightDirection6Item(aLightGroup.GetDirection( Base3DLight5 )));
/*N*/ 	mpObjectItemSet->Put(Svx3DLightDirection7Item(aLightGroup.GetDirection( Base3DLight6 )));
/*N*/ 	mpObjectItemSet->Put(Svx3DLightDirection8Item(aLightGroup.GetDirection( Base3DLight7 )));
/*N*/ }

/*N*/ void E3dScene::ImpSetSceneItemsFromCamera()
/*N*/ {
/*N*/ 	ImpForceItemSet();
/*N*/ 	Camera3D aSceneCam (GetCamera());
/*N*/
/*N*/ 	// ProjectionType
/*N*/ 	mpObjectItemSet->Put(Svx3DPerspectiveItem((UINT16)aSceneCam.GetProjection()));
/*N*/
/*N*/ 	// CamPos
/*N*/ 	mpObjectItemSet->Put(Svx3DDistanceItem((UINT32)(aSceneCam.GetPosition().Z() + 0.5)));
/*N*/
/*N*/ 	// FocalLength
/*N*/ 	mpObjectItemSet->Put(Svx3DFocalLengthItem((UINT32)((aSceneCam.GetFocalLength() * 100.0) + 0.5)));
/*N*/ }

//////////////////////////////////////////////////////////////////////////////
// ItemSet access

/*N*/ const SfxItemSet& E3dScene::GetItemSet() const
/*N*/ {
/*N*/ 	// prepare ItemSet
/*N*/ 	if(mpObjectItemSet)
/*N*/ 	{
/*N*/ 		SfxItemSet aNew(*mpObjectItemSet->GetPool(), SDRATTR_3DSCENE_FIRST, SDRATTR_3DSCENE_LAST);
/*N*/ 		aNew.Put(*mpObjectItemSet);
/*N*/ 		mpObjectItemSet->ClearItem();
/*N*/ 		mpObjectItemSet->Put(aNew);
/*N*/ 	}
/*N*/ 	else
/*N*/ 		((E3dScene*)this)->ImpForceItemSet();
/*N*/
/*N*/ 	// collect all ItemSets in mpGroupItemSet
/*N*/ 	sal_uInt32 nCount(pSub->GetObjCount());
/*N*/ 	for(sal_uInt32 a(0); a < nCount; a++)
/*N*/ 	{
/*N*/ 		const SfxItemSet& rSet = pSub->GetObj(a)->GetItemSet();
/*N*/ 		SfxWhichIter aIter(rSet);
/*N*/ 		sal_uInt16 nWhich(aIter.FirstWhich());
/*N*/
/*N*/ 		while(nWhich)
/*N*/ 		{
/*N*/ 			if(SFX_ITEM_DONTCARE == rSet.GetItemState(nWhich, FALSE))
/*N*/ 				mpObjectItemSet->InvalidateItem(nWhich);
/*N*/ 			else
/*N*/ 				mpObjectItemSet->MergeValue(rSet.Get(nWhich), TRUE);
/*N*/
/*N*/ 			nWhich = aIter.NextWhich();
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	return *mpObjectItemSet;
/*N*/ }

//////////////////////////////////////////////////////////////////////////////
// private support routines for ItemSet access

/*N*/ void E3dScene::ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem)
/*N*/ {
/*N*/ 	// handle local value change
/*N*/ 	if(!nWhich || (nWhich >= SDRATTR_3DSCENE_FIRST && nWhich <= SDRATTR_3DSCENE_LAST))
/*N*/ 		SdrAttrObj::ItemChange(nWhich, pNewItem);
/*N*/
/*N*/ 	// ItemChange at all contained objects
/*N*/ 	List aPostItemChangeList;
/*N*/ 	sal_uInt32 nCount(pSub->GetObjCount());
        sal_uInt32 a;
/*N*/
/*N*/ 	for( a=0; a < nCount; a++)
/*N*/ 	{
/*N*/ 		SdrObject* pObj = pSub->GetObj(a);
/*N*/ 		if(pObj->AllowItemChange(nWhich, pNewItem))
/*N*/ 		{
/*N*/ 			pObj->ItemChange(nWhich, pNewItem);
/*N*/ 			aPostItemChangeList.Insert((void*)pObj, LIST_APPEND);
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	for(a = 0; a < aPostItemChangeList.Count(); a++)
/*N*/ 	{
/*N*/ 		SdrObject* pObj = (SdrObject*)aPostItemChangeList.GetObject(a);
/*N*/ 		pObj->PostItemChange(nWhich);
/*N*/ 	}
/*N*/ }

/*N*/ void E3dScene::PostItemChange(const sal_uInt16 nWhich)
/*N*/ {
/*N*/ 	// call parent
/*N*/ 	if(!nWhich || (nWhich >= SDRATTR_3DSCENE_FIRST && nWhich <= SDRATTR_3DSCENE_LAST))
/*N*/ 		SdrAttrObj::PostItemChange(nWhich);
/*N*/
/*N*/ 	// local changes
/*N*/ 	StructureChanged(this);
/*N*/
/*N*/ 	switch(nWhich)
/*N*/ 	{
/*N*/ 		case SDRATTR_3DSCENE_PERSPECTIVE			:
/*N*/ 		case SDRATTR_3DSCENE_DISTANCE				:
/*N*/ 		case SDRATTR_3DSCENE_FOCAL_LENGTH			:
/*N*/ 		{
/*N*/ 			// #83387#, #83391#
/*N*/ 			// one common function for the camera attributes
/*N*/ 			// since SetCamera() sets all three back to the ItemSet
/*N*/ 			Camera3D aSceneCam(GetCamera());
/*N*/ 			BOOL bChange(FALSE);
/*N*/
/*N*/ 			// for SDRATTR_3DSCENE_PERSPECTIVE:
/*N*/ 			if(aSceneCam.GetProjection() != GetPerspective())
/*N*/ 			{
/*N*/ 				aSceneCam.SetProjection(GetPerspective());
/*N*/ 				bChange = TRUE;
/*N*/ 			}
/*N*/
/*N*/ 			// for SDRATTR_3DSCENE_DISTANCE:
/*N*/ 			Vector3D aActualPosition = aSceneCam.GetPosition();
/*N*/ 			double fNew = GetDistance();
/*N*/ 			if(fNew != aActualPosition.Z())
/*N*/ 			{
/*N*/ 				aSceneCam.SetPosition( Vector3D( aActualPosition.X(), aActualPosition.Y(), fNew) );
/*N*/ 				bChange = TRUE;
/*N*/ 			}
/*N*/
/*N*/ 			// for SDRATTR_3DSCENE_FOCAL_LENGTH:
/*N*/ 			fNew = GetFocalLength() / 100.0;
/*N*/ 			if(aSceneCam.GetFocalLength() != fNew)
/*N*/ 			{
/*N*/ 				aSceneCam.SetFocalLength(fNew);
/*N*/ 				bChange = TRUE;
/*N*/ 			}
/*N*/
/*N*/ 			// for all
/*N*/ 			if(bChange)
/*N*/ 				SetCamera(aSceneCam);
/*N*/
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case SDRATTR_3DSCENE_TWO_SIDED_LIGHTING		:
/*N*/ 		{
/*N*/ 			aLightGroup.SetModelTwoSide(GetTwoSidedLighting());
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case SDRATTR_3DSCENE_LIGHTCOLOR_1			:
/*N*/ 		{
/*N*/ 			aLightGroup.SetIntensity( GetLightColor1(), Base3DMaterialDiffuse, Base3DLight0);
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case SDRATTR_3DSCENE_LIGHTCOLOR_2			:
/*N*/ 		{
/*N*/ 			aLightGroup.SetIntensity( GetLightColor2(), Base3DMaterialDiffuse, Base3DLight1);
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case SDRATTR_3DSCENE_LIGHTCOLOR_3			:
/*N*/ 		{
/*N*/ 			aLightGroup.SetIntensity( GetLightColor3(), Base3DMaterialDiffuse, Base3DLight2);
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case SDRATTR_3DSCENE_LIGHTCOLOR_4			:
/*N*/ 		{
/*N*/ 			aLightGroup.SetIntensity( GetLightColor4(), Base3DMaterialDiffuse, Base3DLight3);
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case SDRATTR_3DSCENE_LIGHTCOLOR_5			:
/*N*/ 		{
/*N*/ 			aLightGroup.SetIntensity( GetLightColor5(), Base3DMaterialDiffuse, Base3DLight4);
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case SDRATTR_3DSCENE_LIGHTCOLOR_6			:
/*N*/ 		{
/*N*/ 			aLightGroup.SetIntensity( GetLightColor6(), Base3DMaterialDiffuse, Base3DLight5);
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case SDRATTR_3DSCENE_LIGHTCOLOR_7			:
/*N*/ 		{
/*N*/ 			aLightGroup.SetIntensity( GetLightColor7(), Base3DMaterialDiffuse, Base3DLight6);
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case SDRATTR_3DSCENE_LIGHTCOLOR_8			:
/*N*/ 		{
/*N*/ 			aLightGroup.SetIntensity( GetLightColor8(), Base3DMaterialDiffuse, Base3DLight7);
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case SDRATTR_3DSCENE_AMBIENTCOLOR			:
/*N*/ 		{
/*N*/ 			aLightGroup.SetGlobalAmbientLight(GetGlobalAmbientColor());
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case SDRATTR_3DSCENE_LIGHTON_1				:
/*N*/ 		{
/*N*/ 			aLightGroup.Enable( GetLightOnOff1(), Base3DLight0);
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case SDRATTR_3DSCENE_LIGHTON_2				:
/*N*/ 		{
/*N*/ 			aLightGroup.Enable( GetLightOnOff2(), Base3DLight1);
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case SDRATTR_3DSCENE_LIGHTON_3				:
/*N*/ 		{
/*N*/ 			aLightGroup.Enable( GetLightOnOff3(), Base3DLight2);
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case SDRATTR_3DSCENE_LIGHTON_4				:
/*N*/ 		{
/*N*/ 			aLightGroup.Enable( GetLightOnOff4(), Base3DLight3);
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case SDRATTR_3DSCENE_LIGHTON_5				:
/*N*/ 		{
/*N*/ 			aLightGroup.Enable( GetLightOnOff5(), Base3DLight4);
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case SDRATTR_3DSCENE_LIGHTON_6				:
/*N*/ 		{
/*N*/ 			aLightGroup.Enable( GetLightOnOff6(), Base3DLight5);
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case SDRATTR_3DSCENE_LIGHTON_7				:
/*N*/ 		{
/*N*/ 			aLightGroup.Enable( GetLightOnOff7(), Base3DLight6);
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case SDRATTR_3DSCENE_LIGHTON_8				:
/*N*/ 		{
/*N*/ 			aLightGroup.Enable( GetLightOnOff8(), Base3DLight7);
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case SDRATTR_3DSCENE_LIGHTDIRECTION_1		:
/*N*/ 		{
/*N*/ 			aLightGroup.SetDirection( GetLightDirection1(), Base3DLight0);
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case SDRATTR_3DSCENE_LIGHTDIRECTION_2		:
/*N*/ 		{
/*N*/ 			aLightGroup.SetDirection( GetLightDirection2(), Base3DLight1);
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case SDRATTR_3DSCENE_LIGHTDIRECTION_3		:
/*N*/ 		{
/*N*/ 			aLightGroup.SetDirection( GetLightDirection3(), Base3DLight2);
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case SDRATTR_3DSCENE_LIGHTDIRECTION_4		:
/*N*/ 		{
/*N*/ 			aLightGroup.SetDirection( GetLightDirection4(), Base3DLight3);
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case SDRATTR_3DSCENE_LIGHTDIRECTION_5		:
/*N*/ 		{
/*N*/ 			aLightGroup.SetDirection( GetLightDirection5(), Base3DLight4);
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case SDRATTR_3DSCENE_LIGHTDIRECTION_6		:
/*N*/ 		{
/*N*/ 			aLightGroup.SetDirection( GetLightDirection6(), Base3DLight5);
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case SDRATTR_3DSCENE_LIGHTDIRECTION_7		:
/*N*/ 		{
/*N*/ 			aLightGroup.SetDirection( GetLightDirection7(), Base3DLight6);
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case SDRATTR_3DSCENE_LIGHTDIRECTION_8		:
/*N*/ 		{
/*N*/ 			aLightGroup.SetDirection( GetLightDirection8(), Base3DLight7);
/*N*/ 			break;
/*N*/ 		}
/*N*/ 	}
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
// ItemSet was changed, maybe user wants to react

/*N*/ void E3dScene::ItemSetChanged( const SfxItemSet& rSet )
/*N*/ {
/*N*/ 	// call parent
/*N*/ 	E3dObject::ItemSetChanged( rSet );
/*N*/
/*N*/ 	// set at all contained objects
/*N*/ 	sal_uInt32 nCount(pSub->GetObjCount());
/*N*/ 	for(sal_uInt32 a(0); a < nCount; a++)
/*N*/ 		pSub->GetObj(a)->ItemSetChanged( rSet );
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
// pre- and postprocessing for objects for saving

/*N*/ void E3dScene::PreSave()
/*N*/ {
/*N*/ 	// call parent
/*N*/ 	E3dObject::PreSave();
/*N*/
/*N*/ 	// set at all contained objects
/*N*/ 	sal_uInt32 nCount(pSub->GetObjCount());
/*N*/ 	for(sal_uInt32 a(0); a < nCount; a++)
/*N*/ 		pSub->GetObj(a)->PreSave();
/*N*/ }

/*N*/ void E3dScene::PostSave()
/*N*/ {
/*N*/ 	// call parent
/*N*/ 	E3dObject::PostSave();
/*N*/
/*N*/ 	// set at all contained objects
/*N*/ 	sal_uInt32 nCount(pSub->GetObjCount());
/*N*/ 	for(sal_uInt32 a(0); a < nCount; a++)
/*N*/ 		pSub->GetObj(a)->PostSave();
/*N*/ }

/*************************************************************************
|*
|* ItemPool fuer dieses Objekt wechseln
|*
\************************************************************************/

/*N*/ void E3dScene::MigrateItemPool(SfxItemPool* pSrcPool, SfxItemPool* pDestPool, SdrModel* pNewModel )
/*N*/ {
/*N*/ 	if(pSrcPool && pDestPool && (pSrcPool != pDestPool))
/*N*/ 	{
/*N*/ 		// call parent
/*N*/ 		E3dObject::MigrateItemPool(pSrcPool, pDestPool, pNewModel);
/*N*/
/*N*/ 		// own reaction, but only with outmost scene
/*N*/ 		SdrObjList* pSubList = GetSubList();
/*N*/ 		if(pSubList && GetScene() == this)
/*N*/ 		{
/*N*/ 			SdrObjListIter a3DIterator(*pSubList, IM_DEEPWITHGROUPS);
/*N*/ 			while ( a3DIterator.IsMore() )
/*N*/ 			{
/*?*/ 				E3dObject* pObj = (E3dObject*) a3DIterator.Next();
/*?*/ 				DBG_ASSERT(pObj->ISA(E3dObject), "AW: In Szenen sind nur 3D-Objekte erlaubt!");
/*?*/ 				pObj->MigrateItemPool(pSrcPool, pDestPool, pNewModel);
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void E3dScene::SetShadowPlaneDirection(const Vector3D& rVec)
/*N*/ {
/*N*/ 	UINT16 nSceneShadowSlant = (UINT16)((atan2(rVec.Y(), rVec.Z()) / F_PI180) + 0.5);
/*N*/ 	ImpForceItemSet();
/*N*/ 	mpObjectItemSet->Put(Svx3DShadowSlantItem(nSceneShadowSlant));
/*N*/ }

}
