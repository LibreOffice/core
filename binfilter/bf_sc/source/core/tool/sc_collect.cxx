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

#include <string.h>


#include "rechead.hxx"
#include "collect.hxx"
#include "document.hxx"	
#include <tools/debug.hxx>// fuer TypedStrData Konstruktor
namespace binfilter {

// -----------------------------------------------------------------------

/*N*/ DataObject::~DataObject()
/*N*/ {
/*N*/ }

//------------------------------------------------------------------------
// Collection
//------------------------------------------------------------------------

/*N*/ void lcl_DeleteDataObjects( DataObject** p, USHORT nCount )
/*N*/ {
/*N*/ 	if ( p )
/*N*/ 	{
/*N*/ 		for (USHORT i = 0; i < nCount; i++) delete p[i];
/*N*/ 		delete[] p;
/*N*/ 		p = NULL;
/*N*/ 	}
/*N*/ }

/*N*/ Collection::Collection(USHORT nLim, USHORT nDel) :
/*N*/ 	nCount ( 0 ),
/*N*/ 	nLimit ( nLim ),
/*N*/ 	nDelta ( nDel ),
/*N*/ 	pItems ( NULL )
/*N*/ {
/*N*/ 	if (nDelta > MAXDELTA)
/*N*/ 		nDelta = MAXDELTA;
/*N*/ 	else if (nDelta == 0)
/*N*/ 		nDelta = 1;
/*N*/ 	if (nLimit > MAXCOLLECTIONSIZE)
/*N*/ 		nLimit = MAXCOLLECTIONSIZE;
/*N*/ 	else if (nLimit < nDelta)
/*N*/ 		nLimit = nDelta;
/*N*/ 	pItems = new DataObject*[nLimit];
/*N*/ }

/*N*/ Collection::Collection(const Collection& rCollection)
/*N*/ 	:	nCount ( 0 ),
/*N*/ 		nLimit ( 0 ),
/*N*/ 		nDelta ( 0 ),
/*N*/ 		pItems ( NULL )
/*N*/ {
/*N*/ 	*this = rCollection;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ Collection::~Collection()
/*N*/ {
/*N*/ 	lcl_DeleteDataObjects( pItems, nCount );
/*N*/ }

//------------------------------------------------------------------------

/*N*/ void Collection::AtFree(USHORT nIndex)
/*N*/ {
/*N*/ 	if ((pItems) && (nIndex < nCount))
/*N*/ 	{
/*N*/ 		delete pItems[nIndex];
/*N*/ 		--nCount;				// before memmove
/*N*/ 		memmove ( &pItems[nIndex], &pItems[nIndex + 1], (nCount - nIndex) * sizeof(DataObject*));
/*N*/ 		pItems[nCount] = NULL;
/*N*/ 	}
/*N*/ }

//------------------------------------------------------------------------

/*N*/ void Collection::Free(DataObject* pDataObject)
/*N*/ {
/*N*/ 	AtFree(IndexOf(pDataObject));
/*N*/ }

//------------------------------------------------------------------------

/*N*/ void Collection::FreeAll()
/*N*/ {
/*N*/ 	lcl_DeleteDataObjects( pItems, nCount );
/*N*/ 	nCount = 0;
/*N*/ 	pItems = new DataObject*[nLimit];
/*N*/ }

//------------------------------------------------------------------------

/*N*/ BOOL Collection::AtInsert(USHORT nIndex, DataObject* pDataObject)
/*N*/ {
/*N*/ 	if ((nCount < MAXCOLLECTIONSIZE) && (nIndex <= nCount) && pItems)
/*N*/ 	{
/*N*/ 		if (nCount == nLimit)
/*N*/ 		{
/*N*/ 			DataObject** pNewItems = new DataObject*[nLimit + nDelta];
/*N*/ 			if (!pNewItems)
/*N*/ 				return FALSE;
/*N*/ 			nLimit += nDelta;
/*N*/ 			memmove(pNewItems, pItems, nCount * sizeof(DataObject*));
/*N*/ 			delete[] pItems;
/*N*/ 			pItems = pNewItems;
/*N*/ 		}
/*N*/ 		if (nCount > nIndex)
/*N*/ 			memmove(&pItems[nIndex + 1], &pItems[nIndex], (nCount - nIndex) * sizeof(DataObject*));
/*N*/ 		pItems[nIndex] = pDataObject;
/*N*/ 		nCount++;
/*N*/ 		return TRUE;
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ BOOL Collection::Insert(DataObject* pDataObject)
/*N*/ {
/*N*/ 	return AtInsert(nCount, pDataObject);
/*N*/ }

//------------------------------------------------------------------------

/*N*/ DataObject* Collection::At(USHORT nIndex) const
/*N*/ {
/*N*/ 	if (nIndex < nCount)
/*N*/ 		return pItems[nIndex];
/*N*/ 	else
/*N*/ 		return NULL;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ USHORT Collection::IndexOf(DataObject* pDataObject) const
/*N*/ {
/*N*/ 	USHORT nIndex = 0xffff;
/*N*/ 	for (USHORT i = 0; ((i < nCount) && (nIndex == 0xffff)); i++)
/*N*/ 	{
/*N*/ 		if (pItems[i] == pDataObject) nIndex = i;
/*N*/ 	}
/*N*/ 	return nIndex;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ Collection& Collection::operator=( const Collection& r )
/*N*/ {
/*N*/ 	lcl_DeleteDataObjects( pItems, nCount );
/*N*/ 
/*N*/ 	nCount = r.nCount;
/*N*/ 	nLimit = r.nLimit;
/*N*/ 	nDelta = r.nDelta;
/*N*/ 	pItems = new DataObject*[nLimit];
/*N*/ 	for ( USHORT i=0; i<nCount; i++ )
/*N*/ 		pItems[i] = r.pItems[i]->Clone();
/*N*/ 
/*N*/ 	return *this;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ DataObject*	Collection::Clone() const
/*N*/ {
        DBG_BF_ASSERT(0, "STRIP"); return NULL; //STRIP001 	return new Collection(*this);
/*N*/ }

//------------------------------------------------------------------------
// SortedCollection
//------------------------------------------------------------------------

/*N*/ SortedCollection::SortedCollection(USHORT nLim, USHORT nDel, BOOL bDup) :
/*N*/ 	Collection (nLim, nDel),
/*N*/ 	bDuplicates ( bDup)
/*N*/ {
/*N*/ }

//------------------------------------------------------------------------


//------------------------------------------------------------------------

/*N*/ BOOL SortedCollection::Search(DataObject* pDataObject, USHORT& rIndex) const
/*N*/ {
/*N*/ 	rIndex = nCount;
/*N*/ 	BOOL bFound = FALSE;
/*N*/ 	short nLo = 0;
/*N*/ 	short nHi = nCount - 1;
/*N*/ 	short nIndex;
/*N*/ 	short nCompare;
/*N*/ 	while (nLo <= nHi)
/*N*/ 	{
/*N*/ 		nIndex = (nLo + nHi) / 2;
/*N*/ 		nCompare = Compare(pItems[nIndex], pDataObject);
/*N*/ 		if (nCompare < 0)
/*N*/ 			nLo = nIndex + 1;
/*N*/ 		else
/*N*/ 		{
/*N*/ 			nHi = nIndex - 1;
/*N*/ 			if (nCompare == 0)
/*N*/ 			{
/*N*/ 				bFound = TRUE;
/*N*/ 				nLo = nIndex;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	rIndex = nLo;
/*N*/ 	return bFound;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ BOOL SortedCollection::Insert(DataObject* pDataObject)
/*N*/ {
/*N*/ 	USHORT nIndex;
/*N*/ 	BOOL bFound = Search(pDataObject, nIndex);
/*N*/ 	if (bFound)
/*N*/ 	{
/*N*/ 		if (bDuplicates)
/*N*/ 			return AtInsert(nIndex, pDataObject);
/*N*/ 		else
/*N*/ 			return FALSE;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		return AtInsert(nIndex, pDataObject);
/*N*/ }

//------------------------------------------------------------------------


//------------------------------------------------------------------------


//------------------------------------------------------------------------

//	IsEqual - komplette Inhalte vergleichen


//------------------------------------------------------------------------

/*N*/ DataObject*	StrData::Clone() const
/*N*/ {
            return new StrData(*this);      // #i49161# this is needed
/*N*/ }

//------------------------------------------------------------------------

/*N*/ short StrCollection::Compare(DataObject* pKey1, DataObject* pKey2) const
/*N*/ {
/*N*/ 	StringCompare eComp = ((StrData*)pKey1)->aStr.CompareTo(((StrData*)pKey2)->aStr);
/*N*/ 	if (eComp == COMPARE_EQUAL)
/*N*/ 		return 0;
/*N*/ 	else if (eComp == COMPARE_LESS)
/*N*/ 		return -1;
/*N*/ 	else
/*N*/ 		return 1;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ DataObject*	StrCollection::Clone() const
/*N*/ {
            DBG_BF_ASSERT(0, "STRIP"); return NULL;//STRIP001 	return new StrCollection(*this);
/*N*/ }

//------------------------------------------------------------------------

/*N*/ void StrCollection::Load( SvStream& rStream ) // Changetracking.sdc
/*N*/ {
/*N*/ 	ScReadHeader aHdr( rStream );
/*N*/ 	lcl_DeleteDataObjects( pItems, nCount );
/*N*/ 	BOOL bDups;
/*N*/ 	rStream >> bDups;
/*N*/ 	SetDups( bDups );
/*N*/ 	rStream >> nCount >> nLimit >> nDelta;
/*N*/ 	pItems = new DataObject*[nLimit];
/*N*/ 	String aStr;
/*N*/ 	rtl_TextEncoding eSet = rStream.GetStreamCharSet();
/*N*/ 	for ( USHORT i=0; i<nCount; i++ )
/*N*/ 	{
/*N*/ 		rStream.ReadByteString( aStr, eSet );
/*N*/ 		pItems[i] = new StrData( aStr );
/*N*/ 	}
/*N*/ }

/*N*/ void StrCollection::Store( SvStream& rStream ) const
/*N*/ {
/*N*/ 	ScWriteHeader aHdr( rStream );
/*N*/ 	BOOL bDups = IsDups();
/*N*/ 	rStream << bDups << nCount << nLimit << nDelta;
/*N*/ 	rtl_TextEncoding eSet = rStream.GetStreamCharSet();
/*N*/ 	for ( USHORT i=0; i<nCount; i++ )
/*N*/ 	{
/*N*/ 		rStream.WriteByteString( ((StrData*)pItems[i])->GetString(), eSet );
/*N*/ 	}
/*N*/ }

//------------------------------------------------------------------------
// TypedStrCollection
//------------------------------------------------------------------------

/*N*/ TypedStrData::TypedStrData( ScDocument* pDoc, USHORT nCol, USHORT nRow, USHORT nTab,
/*N*/ 								BOOL bAllStrings )
/*N*/ {
/*N*/ 	if ( pDoc->HasValueData( nCol, nRow, nTab ) )
/*N*/ 	{
/*?*/ 		pDoc->GetValue( nCol, nRow, nTab, nValue );
/*?*/ 		if (bAllStrings)
/*?*/ 			pDoc->GetString( nCol, nRow, nTab, aStrValue );
/*?*/ 		nStrType = 0;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		pDoc->GetString( nCol, nRow, nTab, aStrValue );
/*N*/ 		nValue = 0.0;
/*N*/ 		nStrType = 1;		//! Typ uebergeben ?
/*N*/ 	}
/*N*/ }

/*N*/ DataObject*	TypedStrData::Clone() const
/*N*/ {
/*N*/ 	return new TypedStrData(*this);
/*N*/ }


/*N*/ short TypedStrCollection::Compare( DataObject* pKey1, DataObject* pKey2 ) const
/*N*/ {
/*N*/ 	short nResult = 0;
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 //STRIP001 	if ( pKey1 && pKey2 )
/*N*/ return nResult;
/*N*/ }


        // Gross-/Kleinschreibung anpassen




}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
