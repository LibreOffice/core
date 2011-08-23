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

#include "objid.hxx"
#include "glob.hxx"

#include "defines.hxx"

#include <tools/debug.hxx> //STRIP001
namespace binfilter {
/*************************************************************************
|*
|* Objekt-Id ermitteln;
|* Liefert -1, wenn das Objekt keine Id hat
|*
\************************************************************************/

/*N*/ SchObjectId* GetObjectId(const SdrObject& rObj)
/*N*/ {
/*N*/ 	for (USHORT i = 0; i < rObj.GetUserDataCount(); i++)
/*N*/ 	{
/*N*/ 		SdrObjUserData *pData = rObj.GetUserData(i);
/*N*/ 		if (pData && pData->GetId() == SCH_OBJECTID_ID)
/*N*/ 			return (SchObjectId*)pData;
/*N*/ 	}
/*N*/ 
/*N*/ 	return NULL;
/*N*/ }


/*************************************************************************
|*
|* Objekt mit Id suchen;
|* liefert NULL, wenn kein Objekt gefunden wurde.
|*
\************************************************************************/

/*N*/ SdrObject* GetObjWithId(UINT16 nObjId, const SdrObjList& rObjList,
/*N*/ 						ULONG* pIndex, SdrIterMode eMode)
/*N*/ {
/*N*/ 	ULONG nIndex = 0;
/*N*/ 
/*N*/ 	SdrObjListIter aIterator(rObjList, eMode);
/*N*/ 	while (aIterator.IsMore())
/*N*/ 	{
/*N*/ 		SdrObject* pObj = aIterator.Next();
/*N*/ 		SchObjectId* pObjId = GetObjectId(*pObj);
/*N*/ 		if (pObjId && pObjId->GetObjId() == nObjId)
/*N*/ 		{
/*N*/ 			if (pIndex)
/*?*/ 				*pIndex = nIndex;
/*N*/ 			return pObj;
/*N*/ 		}
/*N*/ 
/*N*/ 		nIndex++;
/*N*/ 	}
/*N*/ 
/*N*/ 	return NULL;
/*N*/ }

/*************************************************************************
|*
|* Alle Objekt mit Id=nObjId Attributieren,
|*
\************************************************************************/
/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

/*N*/ SchObjectId::SchObjectId() :
/*N*/ 	SdrObjUserData(SchInventor, SCH_OBJECTID_ID, 0),
/*N*/ 	nObjId(0)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

/*N*/ SchObjectId::SchObjectId(UINT16 nId) :
/*N*/ 	SdrObjUserData(SchInventor, SCH_OBJECTID_ID, 0),
/*N*/ 	nObjId(nId)
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

/*N*/ SdrObjUserData* SchObjectId::Clone(SdrObject *pObj) const
/*N*/ {
/*?*/  	DBG_BF_ASSERT(0, "STRIP"); return NULL;//STRIP001 return new SchObjectId(*this);
/*N*/ }

/*************************************************************************
|*
|* Daten in Stream schreiben
|*
\************************************************************************/

/*N*/ void SchObjectId::WriteData(SvStream& rOut)
/*N*/ {
/*N*/ 	SdrObjUserData::WriteData(rOut);
/*N*/ 
/*N*/ 	rOut << nObjId;
/*N*/ }


/*************************************************************************
|*
|* Daten aus Stream lesen
|*
\************************************************************************/

/*N*/ void SchObjectId::ReadData(SvStream& rIn)
/*N*/ {
/*N*/ 	SdrObjUserData::ReadData(rIn);
/*N*/ 
/*N*/ 	rIn >> nObjId;
/*N*/ }

/*N*/ #ifdef DBG_UTIL

// this function is for debugging only
// therefore it is ok to use char* instead of UniString
/*N*/ char* GetCHOBJIDName( const long id )
/*N*/ {
/*N*/ 	switch( id )
/*N*/ 	{
/*?*/ 		case CHOBJID_ANY:						return "CHOBJID_ANY";
/*?*/ 		case CHOBJID_TEXT:						return "CHOBJID_TEXT";
/*?*/ 		case CHOBJID_AREA:						return "CHOBJID_AREA";
/*?*/ 		case CHOBJID_LINE:						return "CHOBJID_LINE";
/*?*/ 		case CHOBJID_DIAGRAM_AREA:				return "CHOBJID_DIAGRAM_AREA";
/*?*/ 		case CHOBJID_TITLE_MAIN:				return "CHOBJID_TITLE_MAIN";
/*?*/ 		case CHOBJID_TITLE_SUB:					return "CHOBJID_TITLE_SUB";
/*?*/ 		case CHOBJID_DIAGRAM:					return "CHOBJID_DIAGRAM";
/*?*/ 		case CHOBJID_DIAGRAM_WALL:				return "CHOBJID_DIAGRAM_WALL";
/*?*/ 		case CHOBJID_DIAGRAM_FLOOR:				return "CHOBJID_DIAGRAM_FLOOR";
/*?*/ 		case CHOBJID_DIAGRAM_TITLE_X_AXIS:		return "CHOBJID_DIAGRAM_TITLE_X_AXIS";
/*?*/ 		case CHOBJID_DIAGRAM_TITLE_Y_AXIS:		return "CHOBJID_DIAGRAM_TITLE_Y_AXIS";
/*N*/ 		case CHOBJID_DIAGRAM_TITLE_Z_AXIS:		return "CHOBJID_DIAGRAM_TITLE_Z_AXIS";
/*?*/ 		case CHOBJID_DIAGRAM_X_AXIS:			return "CHOBJID_DIAGRAM_X_AXIS";
/*?*/ 		case CHOBJID_DIAGRAM_Y_AXIS:			return "CHOBJID_DIAGRAM_Y_AXIS";
/*?*/ 		case CHOBJID_DIAGRAM_Z_AXIS:			return "CHOBJID_DIAGRAM_Z_AXIS";
/*?*/ 		case CHOBJID_DIAGRAM_A_AXIS:			return "CHOBJID_DIAGRAM_A_AXIS";
/*?*/ 		case CHOBJID_DIAGRAM_B_AXIS:			return "CHOBJID_DIAGRAM_B_AXIS";
/*?*/ 		case CHOBJID_DIAGRAM_C_AXIS:			return "CHOBJID_DIAGRAM_C_AXIS";
/*?*/ 		case CHOBJID_DIAGRAM_X_GRID_MAIN:		return "CHOBJID_DIAGRAM_X_GRID_MAIN";
/*?*/ 		case CHOBJID_DIAGRAM_Y_GRID_MAIN:		return "CHOBJID_DIAGRAM_Y_GRID_MAIN";
/*?*/ 		case CHOBJID_DIAGRAM_Z_GRID_MAIN:		return "CHOBJID_DIAGRAM_Z_GRID_MAIN";
/*?*/ 		case CHOBJID_DIAGRAM_Y_GRID_HELP:		return "CHOBJID_DIAGRAM_Y_GRID_HELP";
/*?*/ 		case CHOBJID_DIAGRAM_Z_GRID_HELP:		return "CHOBJID_DIAGRAM_Z_GRID_HELP";
/*?*/ 		case CHOBJID_DIAGRAM_ROWGROUP:			return "CHOBJID_DIAGRAM_ROWGROUP";
/*?*/ 		case CHOBJID_DIAGRAM_ROWS:				return "CHOBJID_DIAGRAM_ROWS";
/*?*/ 		case CHOBJID_DIAGRAM_ROWSLINE:			return "CHOBJID_DIAGRAM_ROWSLINE";
/*?*/ 		case CHOBJID_DIAGRAM_DATA:				return "CHOBJID_DIAGRAM_DATA";
/*?*/ 		case CHOBJID_DIAGRAM_DESCRGROUP:		return "CHOBJID_DIAGRAM_DESCRGROUP";
/*?*/ 		case CHOBJID_DIAGRAM_DESCR_ROW:			return "CHOBJID_DIAGRAM_DESCR_ROW";
/*?*/ 		case CHOBJID_DIAGRAM_DESCR_COL:			return "CHOBJID_DIAGRAM_DESCR_COL";
/*?*/ 		case CHOBJID_DIAGRAM_DESCR_SYMBOL:		return "CHOBJID_DIAGRAM_DESCR_SYMBOL";
/*?*/ 		case CHOBJID_LEGEND:					return "CHOBJID_LEGEND";
/*?*/ 		case CHOBJID_LEGEND_BACK:				return "CHOBJID_LEGEND_BACK";
/*?*/ 		case CHOBJID_LEGEND_SYMBOL_ROW:			return "CHOBJID_LEGEND_SYMBOL_ROW";
/*?*/ 		case CHOBJID_LEGEND_SYMBOL_COL:			return "CHOBJID_LEGEND_SYMBOL_COL";
/*?*/ 		case CHOBJID_DIAGRAM_Z_AXIS_GROUP:		return "CHOBJID_DIAGRAM_Z_AXIS_GROUP";
/*?*/ 		case CHOBJID_DIAGRAM_NET:				return "CHOBJID_DIAGRAM_NET";
/*?*/ 		case CHOBJID_DIAGRAM_AVERAGEVALUE:		return "CHOBJID_DIAGRAM_AVERAGEVALUE";
/*?*/ 		case CHOBJID_DIAGRAM_ERROR:				return "CHOBJID_DIAGRAM_ERROR";
/*?*/ 		case CHOBJID_DIAGRAM_REGRESSION:		return "CHOBJID_DIAGRAM_REGRESSION";
/*?*/ 		case CHOBJID_DIAGRAM_STACKEDGROUP:		return "CHOBJID_DIAGRAM_STACKEDGROUP";
/*?*/ 		case CHOBJID_DIAGRAM_STATISTICS_GROUP:	return "CHOBJID_DIAGRAM_STATISTICS_GROUP";
/*?*/ 		case CHOBJID_DIAGRAM_X_GRID_MAIN_GROUP:	return "CHOBJID_DIAGRAM_X_GRID_MAIN_GROUP";
/*?*/ 		case CHOBJID_DIAGRAM_Y_GRID_MAIN_GROUP:	return "CHOBJID_DIAGRAM_Y_GRID_MAIN_GROUP";
/*?*/ 		case CHOBJID_DIAGRAM_Z_GRID_MAIN_GROUP:	return "CHOBJID_DIAGRAM_Z_GRID_MAIN_GROUP";
/*?*/ 		case CHOBJID_DIAGRAM_X_GRID_HELP_GROUP:	return "CHOBJID_DIAGRAM_X_GRID_HELP_GROUP";
/*?*/ 		case CHOBJID_DIAGRAM_Y_GRID_HELP_GROUP:	return "CHOBJID_DIAGRAM_Y_GRID_HELP_GROUP";
/*?*/ 		case CHOBJID_DIAGRAM_Z_GRID_HELP_GROUP:	return "CHOBJID_DIAGRAM_Z_GRID_HELP_GROUP";
/*?*/ 		case CHOBJID_DIAGRAM_SPECIAL_GROUP:		return "CHOBJID_DIAGRAM_SPECIAL_GROUP";
/*?*/ 		case CHOBJID_DIAGRAM_STOCKLINE_GROUP:	return "CHOBJID_DIAGRAM_STOCKLINE_GROUP";
/*?*/ 		case CHOBJID_DIAGRAM_STOCKLOSS_GROUP:	return "CHOBJID_DIAGRAM_STOCKLOSS_GROUP";
/*?*/ 		case CHOBJID_DIAGRAM_STOCKPLUS_GROUP:	return "CHOBJID_DIAGRAM_STOCKPLUS_GROUP";
/*?*/ 
/*?*/ 		default:
/*?*/ 			return "unknown Id";
/*N*/ 	}
/*N*/ }

/*N*/ #endif	// DBG_UTIL



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
