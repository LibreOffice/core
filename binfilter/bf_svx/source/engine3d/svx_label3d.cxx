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

#include "svdstr.hrc"

#ifndef _SVDIO_HXX
#include "svdio.hxx"
#endif

#ifndef _E3D_GLOBL3D_HXX
#include "globl3d.hxx"
#endif

#ifndef _E3D_LABEL3D_HXX
#include "label3d.hxx"
#endif
namespace binfilter {

/*N*/ TYPEINIT1(E3dLabelObj, E3dPointObj);

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

/*N*/ E3dLabelObj::~E3dLabelObj()
/*N*/ {
/*N*/ 	// zugehoeriges 2D-SdrObject loeschen
/*N*/ 	delete p2DLabelObj;
/*N*/ }

/*************************************************************************
|*
|* Identifier zurueckgeben
|*
\************************************************************************/

/*N*/ UINT16 E3dLabelObj::GetObjIdentifier() const
/*N*/ {
/*N*/ 	return E3D_LABELOBJ_ID;
/*N*/ }

/*************************************************************************
|*
|* virtuelle Methode, die ein 2D-Objekt zurueckgibt, falls moeglich
|*
\************************************************************************/

//E3dDisplayObj* E3dLabelObj::MakeViewTransform(Viewport3D& rViewpt,
//											  E3dLightList&,
//											  E3dDisplayObj* pDispObj)
//{
//	aViewPos = rViewpt.GetViewTransform() * GetTransPosition();
//
//	// 2D-Position des Labels bestimmen
//	Point a2DPos = rViewpt.ProjectAndMap(aViewPos);
//	aViewPos -= rViewpt.GetPRP();
//
//	if ( p2DLabelObj )
//		p2DLabelObj->SetAnchorPos(a2DPos);
//
//	if ( pDispObj )
//	{
//		pDispObj->Set2DObj(p2DLabelObj);
//		pDispObj->Set3DObj(this);
//	}
//	else
//		pDispObj = new E3dDisplayObj(this, p2DLabelObj);
//
//	SetDisplayObj(pDispObj);
//
//	return pDispObj;
//}

/*************************************************************************
|*
|* SdrPage auch an p2DLabelObj setzen
|*
\************************************************************************/

/*N*/ void E3dLabelObj::SetPage(SdrPage* pNewPage)
/*N*/ {
/*N*/ 	E3dPointObj::SetPage(pNewPage);
/*N*/ 	if ( p2DLabelObj )
/*N*/ 		p2DLabelObj->SetPage(pNewPage);
/*N*/ }

/*************************************************************************
|*
|* SdrModel auch an p2DLabelObj setzen
|*
\************************************************************************/

/*N*/ void E3dLabelObj::SetModel(SdrModel* pNewModel)
/*N*/ {
/*N*/ 	E3dPointObj::SetModel(pNewModel);
/*N*/ 	if ( p2DLabelObj )
/*N*/ 		p2DLabelObj->SetModel(pNewModel);
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
|* In Stream schreiben
|*
\************************************************************************/

/*N*/ void E3dLabelObj::WriteData(SvStream& rOut) const
/*N*/ {
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	E3dPointObj::WriteData(rOut);
/*N*/ 
/*N*/ #ifdef E3D_STREAMING
/*N*/ 
/*N*/ 	SdrDownCompat aCompat(rOut, STREAM_WRITE);
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("E3dLabelObj");
/*N*/ #endif
/*N*/ 	DBG_ASSERT(p2DLabelObj, "p2DLabelObj muss vor dem Speichern definiert sein!");
/*N*/ 
/*N*/ 	rOut << *p2DLabelObj;
/*N*/ #endif
/*N*/ #endif	// #ifndef SVX_LIGHT
/*N*/ }

/*************************************************************************
|*
|* Aus Stream laden
|*
\************************************************************************/

/*N*/ void E3dLabelObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
/*N*/ {
/*N*/ 	if (ImpCheckSubRecords (rHead, rIn))
/*N*/ 	{
/*N*/ 		E3dPointObj::ReadData(rHead, rIn);
/*N*/ 
/*N*/ 		SdrDownCompat aCompat(rIn, STREAM_READ);
/*N*/ #ifdef DBG_UTIL
/*N*/ 		aCompat.SetID("E3dLabelObj");
/*N*/ #endif
/*N*/ 
/*N*/ 	// loeschen, falls schon vorhanden
/*N*/ 		delete p2DLabelObj;
/*N*/ 		p2DLabelObj = NULL;
/*N*/ 
/*N*/ 		SdrObjIOHeaderLookAhead aHead(rIn, STREAM_READ);
/*N*/ 
/*N*/ 		p2DLabelObj = SdrObjFactory::MakeNewObject(aHead.nInventor,
/*N*/ 												   aHead.nIdentifier, GetPage());
/*N*/ 		if ( p2DLabelObj != NULL )
/*N*/ 		{
/*N*/ 			rIn >> *p2DLabelObj;
/*N*/ 			p2DLabelObj->SetModel(GetModel());
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			aHead.SkipRecord();
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Zuweisungsoperator
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



}
