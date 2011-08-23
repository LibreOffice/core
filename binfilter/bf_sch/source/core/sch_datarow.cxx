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
#include <bf_svx/svditer.hxx>

#include "datarow.hxx"
#include "glob.hxx"
#include <tools/debug.hxx>
namespace binfilter {
/*************************************************************************
|*
|* Datenreihen-Index ermitteln
|*
\************************************************************************/

/*N*/ SchDataRow* GetDataRow(const SdrObject& rObj)
/*N*/ {
/*N*/ 	USHORT i=rObj.GetUserDataCount();//#63904 2x Schleife (15%), 50% pData, 18% id
/*N*/ 	while(i--)
/*N*/ 	{
/*N*/ 		SdrObjUserData *pData = rObj.GetUserData(i);
/*N*/ 		if (pData && pData->GetId() == SCH_DATAROW_ID)
/*N*/ 			return (SchDataRow*)pData;
/*N*/ 	}
/*N*/ 
/*N*/ 	return NULL;
/*N*/ }

/*************************************************************************
|*
|* Objekt mit Datenreihen-Index suchen;
|* liefert NULL, wenn kein Objekt gefunden wurde.
|*
\************************************************************************/

/*N*/ SdrObject* GetObjWithRow(short nRow, const SdrObjList& rObjList,
/*N*/ 						 ULONG* pIndex)
/*N*/ {
/*N*/ 	ULONG nIndex = 0;
/*N*/ 
/*N*/ 	SdrObjListIter aIterator(rObjList, IM_FLAT);
/*N*/ 	while (aIterator.IsMore())
/*N*/ 	{
/*N*/ 		SdrObject* pObj = aIterator.Next();
/*N*/ 		SchDataRow* pDataRow = GetDataRow(*pObj);
/*N*/ 		if (pDataRow && pDataRow->GetRow() == nRow)
/*N*/ 		{
/*N*/ 			if (pIndex)
/*N*/ 				*pIndex = nIndex;
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
|* Konstruktor
|*
\************************************************************************/

/*N*/ SchDataRow::SchDataRow() :
/*N*/ 	SdrObjUserData(SchInventor, SCH_DATAROW_ID, 0),
/*N*/ 	nRow(0)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

/*N*/ SchDataRow::SchDataRow(short nR) :
/*N*/ 	SdrObjUserData(SchInventor, SCH_DATAROW_ID, 0),
/*N*/ 	nRow(nR)
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

/*N*/ SdrObjUserData* SchDataRow::Clone(SdrObject *pObj) const
/*N*/ {
/*?*/  	DBG_BF_ASSERT(0, "STRIP"); return NULL;//STRIP001 return new SchDataRow(*this);
/*N*/ }

/*************************************************************************
|*
|* Daten in Stream schreiben
|*
\************************************************************************/

/*N*/ void SchDataRow::WriteData(SvStream& rOut)
/*N*/ {
/*N*/ 	SdrObjUserData::WriteData(rOut);
/*N*/ 
/*N*/ 	rOut << (INT16)nRow;
/*N*/ }

/*************************************************************************
|*
|* Daten aus Stream lesen
|*
\************************************************************************/

/*N*/ void SchDataRow::ReadData(SvStream& rIn)
/*N*/ {
/*N*/ 	SdrObjUserData::ReadData(rIn);
/*N*/ 
/*N*/ 	INT16 nInt16;
/*N*/ 
/*N*/ 	rIn >> nInt16; nRow = (short)nInt16;
/*N*/ }



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
