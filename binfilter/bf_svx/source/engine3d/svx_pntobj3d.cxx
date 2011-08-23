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

#ifndef _SVDIO_HXX
#include "svdio.hxx"
#endif


#ifndef _E3D_PNTOBJ3D_HXX
#include "pntobj3d.hxx"
#endif
namespace binfilter {

/*N*/ TYPEINIT1(E3dPointObj, E3dObject);

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

/*N*/ E3dPointObj::E3dPointObj(const Vector3D& rPos) :
/*N*/ 	aPosition(rPos),
/*N*/ 	bTransPosValid(FALSE)
/*N*/ {
/*N*/ 	aLocalBoundVol = Volume3D(aPosition, Vector3D(), FALSE);
/*N*/ 	bBoundVolValid = FALSE;
/*N*/ }

/*************************************************************************
|*
|* Leer-Konstruktor
|*
\************************************************************************/

/*N*/ E3dPointObj::E3dPointObj() :
/*N*/ 	aPosition(Vector3D()),
/*N*/ 	bTransPosValid(FALSE)
/*N*/ {
/*N*/ 	aLocalBoundVol = Volume3D(aPosition, Vector3D(), FALSE);
/*N*/ 	bBoundVolValid = FALSE;
/*N*/ }

/*************************************************************************
|*
|* bei Aenderung der Transformation
|*
\************************************************************************/

/*N*/ void E3dPointObj::SetTransformChanged()
/*N*/ {
/*N*/ 	E3dObject::SetTransformChanged();
/*N*/ 	bTransPosValid = FALSE;
/*N*/ }

/*************************************************************************
|*
|* Identifier zurueckgeben
|*
\************************************************************************/


/*************************************************************************
|*
|* neue Position setzen und Aenderung an Parents (Szene) melden
|*
\************************************************************************/

/*N*/ void E3dPointObj::SetPosition(const Vector3D& rNewPos)
/*N*/ {
/*N*/ 	if ( aPosition != rNewPos )
/*N*/ 	{
/*N*/ 		aPosition = rNewPos;
/*N*/ 		bBoundVolValid = FALSE;
/*N*/ 		bTransPosValid = FALSE;
/*N*/ 		StructureChanged(this);
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Transformierte Position zurueckgeben
|*
\************************************************************************/

/*N*/ const Vector3D& E3dPointObj::GetTransPosition()
/*N*/ {
/*N*/ 	if ( !bTransPosValid )
/*N*/ 	{
/*N*/ 		aTransPos = GetFullTransform() * aPosition;
/*N*/ 		bTransPosValid = TRUE;
/*N*/ 	}
/*N*/ 	return aTransPos;
/*N*/ }

/*************************************************************************
|*
|* sichern
|*
\************************************************************************/

/*N*/ void E3dPointObj::WriteData(SvStream& rOut) const
/*N*/ {
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	E3dObject::WriteData(rOut);
/*N*/ 
/*N*/ #ifdef E3D_STREAMING
/*N*/ 
/*N*/ 	SdrDownCompat aCompat(rOut, STREAM_WRITE);
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("E3dPointObj");
/*N*/ #endif
/*N*/ 
/*N*/ 	rOut << aPosition;
/*N*/ #endif
/*N*/ #endif	// #ifndef SVX_LIGHT
/*N*/ }

/*************************************************************************
|*
|* laden
|*
\************************************************************************/

/*N*/ void E3dPointObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
/*N*/ {
/*N*/ 	if (ImpCheckSubRecords (rHead, rIn))
/*N*/ 	{
/*N*/ 		E3dObject::ReadData(rHead, rIn);
/*N*/ 
/*N*/ 		SdrDownCompat aCompat(rIn, STREAM_READ);
/*N*/ #ifdef DBG_UTIL
/*N*/ 		aCompat.SetID("E3dPointObj");
/*N*/ #endif
/*N*/ 
/*N*/ 		rIn >> aPosition;
/*N*/ 
/*N*/ 		bTransPosValid = FALSE;
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Zuweisungsoperator
|*
\************************************************************************/




}
