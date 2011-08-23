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

#ifdef _MSC_VER
#pragma hdrstop
#endif


#include "glob.hxx"
#include "schgroup.hxx"
#include <math.h>
namespace binfilter {

/*N*/ TYPEINIT1(SchObjGroup, SdrObjGroup);


/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

/*N*/ SchObjGroup::SchObjGroup(ChartModel *pChmodel) :
/*N*/ 	SdrObjGroup(),
/*N*/ 	bAskForLogicRect(TRUE),
/*N*/     mbUseChartInventor( true )
/*N*/ {
/*N*/ 	// FG: Damit soll es Objekten im chart ermoeglicht werden sich wie ein
/*N*/ 	//     Objekt im Draw zu verhalten falls gewünscht. Nicht alles was
/*N*/ 	//     prinzipiell geht soll man auch koennen.
/*N*/ 	aInfo.bResizeFreeAllowed    = TRUE;
/*N*/ 	aInfo.bResizePropAllowed    = TRUE;
/*N*/ 	aInfo.bRotateFreeAllowed    = TRUE;
/*N*/ 	aInfo.bRotate90Allowed      = TRUE;
/*N*/ 	aInfo.bMirrorFreeAllowed    = FALSE;
/*N*/ 	aInfo.bMirror45Allowed      = FALSE;
/*N*/ 	aInfo.bMirror90Allowed      = FALSE;
/*N*/ 	aInfo.bShearAllowed         = TRUE;
/*N*/ 
/*N*/ 	eChartGroupType = NOTHING;
/*N*/ 	pChartmodel = pChmodel;
/*N*/ 	SetModel( pChartmodel );
/*N*/ }

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

/*N*/ SchObjGroup::~SchObjGroup()
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|* Identifier zurueckgeben
|*
\************************************************************************/

/*N*/ UINT32 SchObjGroup::GetObjInventor() const
/*N*/ {
/*N*/     if( mbUseChartInventor )
/*N*/         return SchInventor;
/*N*/     else
/*?*/         return SdrInventor;
/*N*/ }

/*************************************************************************
|*
|* Identifier zurueckgeben
|*
\************************************************************************/

/*N*/ UINT16 SchObjGroup::GetObjIdentifier() const
/*N*/ {
/*N*/ 	return SCH_OBJGROUP_ID;
/*N*/ }

/*************************************************************************
|*
|* Handle-Anzahl bestimmen
|*
\************************************************************************/


/*************************************************************************
|*
|* Handle erzeugen
|*
\************************************************************************/


/*************************************************************************
|*
|* Handle-Liste fuellen
|*
\************************************************************************/


/*************************************************************************
|*
|* Identifier zurueckgeben
|*
\************************************************************************/



/*************************************************************************
|*
|* Faehigkeiten der Chart-Gruppe feststellen
|*
\************************************************************************/


/*************************************************************************
|*
|* Faehigkeiten der Chart-Gruppe zuweisen
|*
\************************************************************************/

/*N*/ void	SchObjGroup::SetObjInfo(SdrObjTransformInfoRec aMyInfo)
/*N*/ {
/*N*/ 	// FG: Damit soll es Objekten im chart ermoeglicht werden sich wie ein
/*N*/ 	//     Objekt im Draw zu verhalten falls gewünscht.
/*N*/ 	aInfo.bResizeFreeAllowed    = aMyInfo.bResizeFreeAllowed;
/*N*/ 	aInfo.bResizePropAllowed    = aMyInfo.bResizePropAllowed;
/*N*/ 	aInfo.bRotateFreeAllowed    = aMyInfo.bRotateFreeAllowed;
/*N*/ 	aInfo.bRotate90Allowed      = aMyInfo.bRotate90Allowed;
/*N*/ 	aInfo.bMirrorFreeAllowed    = aMyInfo.bMirrorFreeAllowed;
/*N*/ 	aInfo.bMirror45Allowed      = aMyInfo.bMirror45Allowed;
/*N*/ 	aInfo.bMirror90Allowed      = aMyInfo.bMirror90Allowed;
/*N*/ 	aInfo.bShearAllowed         = aMyInfo.bShearAllowed;
/*N*/ }

/*************************************************************************
|*
|* Wirkliches Resize einer Chart-Gruppe (ohne Broadcast)
|* Bei manchen Chart-Gruppen ist eine Neuberechnung bei einem Resize
|* besser als ein hartes Resize.
|*
\************************************************************************/


/*************************************************************************
|*
|* Um das Verhalten bei einem Resize in Abhaehngigkeit des selektierten
|* Objektes zu ernoeglichen. Bei Diagramme werden nicht die Teilobjekte
|* Resized sondern das Chart in den neuen Grenzen aufgebaut.
|*
\************************************************************************/


/*************************************************************************
|*
|* Um mitzubekommen welche Objekte bewegt werden. Leider wird das
|* nicht nur vom Benutzer verursacht.
|*
\************************************************************************/

/*N*/ void SchObjGroup::Move(const Size& rSiz)
/*N*/ {
/*N*/ 
/*N*/ 	if (eChartGroupType == DIAGRAM)
/*N*/ 	{
/*?*/ 		DBG_ASSERT( pChartmodel, "No Model" );
/*?*/ 		if (pChartmodel != NULL)
/*?*/ 		{
/*?*/ 			Rectangle aRectChart = pChartmodel->GetChartRect();
/*?*/ 			Rectangle aRect = GetBoundRect();
/*?*/ 			Point aPointBottomLeftRectChart = aRectChart.BottomLeft();
/*?*/ 			Point aPointBottomLeftBoundRect = aRect.BottomLeft();
/*?*/ 			Point aPointTopRightRectChart = aRectChart.TopRight();
/*?*/ 			Point aPointTopRightBoundRect = aRect.TopRight();
/*?*/ 			Point aDifferenceLeftBottom = aPointBottomLeftRectChart - aPointBottomLeftBoundRect;
/*?*/ 			Point aDifferenceTopRight = aPointTopRightRectChart - aPointTopRightBoundRect;
/*?*/ 			aRect.Left() += rSiz.Width();
/*?*/ 			aRect.Right() += rSiz.Width();
/*?*/ 			aRect.Top() += rSiz.Height();
/*?*/ 			aRect.Bottom() += rSiz.Height();
/*?*/ 			bAskForLogicRect = FALSE;
/*?*/ 			aRect.Left() += aDifferenceLeftBottom.X();
/*?*/ 			aRect.Bottom() += aDifferenceLeftBottom.Y();
/*?*/ 			aRect.Right() += aDifferenceTopRight.X();
/*?*/ 			aRect.Top() += aDifferenceTopRight.Y();
/*?*/ 			pChartmodel->SetDiagramRectangle(aRect);
/*?*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		SdrObjGroup::Move(rSiz);
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Um mitzubekommen welche Objekte bewegt werden. Leider wird das
|* nicht nur vom Benutzer verursacht.
|*
\************************************************************************/


/*N*/ void SchObjGroup::SetUseChartInventor( bool bUseChartInventor )
/*N*/ {
/*N*/     mbUseChartInventor = bUseChartInventor;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
