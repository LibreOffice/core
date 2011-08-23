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

#include "objadj.hxx"
#include "glob.hxx"

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

namespace binfilter {




/*************************************************************************
|*
|* Objekt-Ausrichting ermitteln
|*
\************************************************************************/

/*N*/ SchObjectAdjust* GetObjectAdjust(const SdrObject& rObj)
/*N*/ {
/*N*/ 	for (USHORT i = 0; i < rObj.GetUserDataCount(); i++)
/*N*/ 	{
/*N*/ 		SdrObjUserData *pData = rObj.GetUserData(i);
/*N*/ 		if (pData && pData->GetId() == SCH_OBJECTADJUST_ID)
/*N*/ 			return (SchObjectAdjust*)pData;
/*N*/ 	}
/*N*/ 
/*?*/ 	return NULL;
/*N*/ }

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

/*N*/ SchObjectAdjust::SchObjectAdjust() :
/*N*/ 	SdrObjUserData(SchInventor, SCH_OBJECTADJUST_ID, 1),
/*N*/ 	eAdjust(CHADJUST_TOP_LEFT),
/*N*/ 	eOrient(CHTXTORIENT_STANDARD)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

/*N*/ SchObjectAdjust::SchObjectAdjust(ChartAdjust eAdj,
/*N*/ 								 SvxChartTextOrient eOr) :
/*N*/ 	SdrObjUserData(SchInventor, SCH_OBJECTADJUST_ID, 1),
/*N*/ 	eAdjust(eAdj),
/*N*/ 	eOrient(eOr)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|* Kopier-Konstruktor
|*
\************************************************************************/


/*************************************************************************
|*
|* Kopie erzeugen
|*
\************************************************************************/

/*N*/ SdrObjUserData* SchObjectAdjust::Clone(SdrObject *pObj) const
/*N*/ {
/*?*/  	DBG_BF_ASSERT(0, "STRIP"); return NULL;//STRIP001 return new SchObjectAdjust(*this);
/*N*/ }

/*************************************************************************
|*
|* Daten in Stream schreiben
|*
\************************************************************************/

/*N*/ void SchObjectAdjust::WriteData(SvStream& rOut)
/*N*/ {
/*N*/ 	SdrObjUserData::WriteData(rOut);
/*N*/ 
/*N*/ 	rOut << (INT16)eAdjust;
/*N*/ 	rOut << (INT16)eOrient;
/*N*/ }

/*************************************************************************
|*
|* Daten aus Stream lesen
|*
\************************************************************************/

/*N*/ void SchObjectAdjust::ReadData(SvStream& rIn)
/*N*/ {
/*N*/ 	SdrObjUserData::ReadData(rIn);
/*N*/ 
/*N*/ 	INT16 nInt16;
/*N*/ 
/*N*/ 	rIn >> nInt16; eAdjust = (ChartAdjust)nInt16;
/*N*/ 
/*N*/ 	if (nVersion < 1)
/*?*/ 		eOrient = CHTXTORIENT_STANDARD;
/*N*/ 	else
/*N*/ 	{
/*N*/ 		rIn >> nInt16; eOrient = (SvxChartTextOrient)nInt16;
/*N*/ 	}
/*N*/ 
/*N*/ }



}
