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

#include "xtable.hxx"
#include "xpool.hxx"
namespace binfilter {

#define GLOBALOVERFLOW

// Vergleichsstrings
/*N*/ sal_Unicode __FAR_DATA pszStandard[] = { 's', 't', 'a', 'n', 'd', 'a', 'r', 'd', 0 };

// Konvertiert in echte RGB-Farben, damit in den Listboxen
// endlich mal richtig selektiert werden kann.
/*N*/ Color RGB_Color( ColorData nColorName )
/*N*/ {
/*N*/ 	Color aColor( nColorName );
/*N*/ 	Color aRGBColor( aColor.GetRed(), aColor.GetGreen(), aColor.GetBlue() );
/*N*/ 	return aRGBColor;
/*N*/ }

// ---------------------
// class XPropertyTable
// ---------------------

/*************************************************************************
|*
|* XPropertyTable::XPropertyTable()
|*
*************************************************************************/

/*N*/ XPropertyTable::XPropertyTable( const String& rPath,
/*N*/ 								XOutdevItemPool* pInPool,
/*N*/ 								USHORT nInitSize, USHORT nReSize ) :
/*N*/ 			aTable			( nInitSize, nReSize ),
/*N*/ 			aPath			( rPath ),
/*N*/ 			aName			( pszStandard, 8 ),
/*N*/ 			pXPool			( pInPool ),
/*N*/ 			pBmpTable		( NULL ),
/*N*/ 			bTableDirty		( TRUE ),
/*N*/ 			bBitmapsDirty	( TRUE ),
/*N*/ 			bOwnPool		( FALSE )
/*N*/ {
/*N*/ 	if( !pXPool )
/*N*/ 	{
/*N*/ 		bOwnPool = TRUE;
/*N*/ 		pXPool = new XOutdevItemPool;
/*N*/ 		DBG_ASSERT( pXPool, "XOutPool konnte nicht erzeugt werden!" );
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* XPropertyTable::~XPropertyTable()
|*
*************************************************************************/

/*N*/ XPropertyTable::~XPropertyTable()
/*N*/ {
/*N*/ 	XPropertyEntry* pEntry = (XPropertyEntry*)aTable.First();
/*N*/ 	Bitmap* pBitmap = NULL;
/*N*/ 	for (ULONG nIndex = 0; nIndex < aTable.Count(); nIndex++)
/*N*/ 	{
/*N*/ 		delete pEntry;
/*N*/ 		pEntry = (XPropertyEntry*)aTable.Next();
/*N*/ 	}
/*N*/ 	// Hier wird die Bitmaptabelle geloescht
/*N*/ 	if( pBmpTable )
/*N*/ 	{
/*N*/ 		pBitmap = (Bitmap*) pBmpTable->First();
/*N*/ 
/*N*/ 		for( ULONG nIndex = 0; nIndex < pBmpTable->Count(); nIndex++ )
/*N*/ 		{
/*N*/ 			delete pBitmap;
/*N*/ 			pBitmap = (Bitmap*) pBmpTable->Next();
/*N*/ 		}
/*N*/ 		delete pBmpTable;
/*N*/ 		pBmpTable = NULL;
/*N*/ 	}
/*N*/ 	// Eigener Pool wird geloescht
/*N*/ 	if( bOwnPool && pXPool )
/*N*/ 	{
/*N*/ 		delete pXPool;
/*N*/ 	}
/*N*/ }

/************************************************************************/

/*N*/ long XPropertyTable::Count() const
/*N*/ {
/*N*/ 	if( bTableDirty )
/*N*/ 	{
/*N*/ 		// ( (XPropertyTable*) this )->bTableDirty = FALSE; <- im Load()
/*N*/ 		if( !( (XPropertyTable*) this )->Load() )
/*N*/ 			( (XPropertyTable*) this )->Create();
/*N*/ 	}
/*N*/ 	return( aTable.Count() );
/*N*/ }

/*************************************************************************
|*
|* XPropertyEntry* XPropertyTable::Get()
|*
*************************************************************************/

/*N*/ XPropertyEntry* XPropertyTable::Get( long nIndex, USHORT nDummy ) const
/*N*/ {
/*N*/ 	if( bTableDirty )
/*N*/ 	{
/*N*/ 		// ( (XPropertyTable*) this )->bTableDirty = FALSE; <- im Load()
/*N*/ 		if( !( (XPropertyTable*) this )->Load() )
/*N*/ 			( (XPropertyTable*) this )->Create();
/*N*/ 	}
/*N*/ 	return (XPropertyEntry*) aTable.GetObject( (ULONG) nIndex );
/*N*/ }

/*************************************************************************
|*
|* long XPropertyTable::Get(const String& rName)
|*
*************************************************************************/

/*N*/ long XPropertyTable::Get(const XubString& rName)
/*N*/ {
/*N*/ 	if( bTableDirty )
/*N*/ 	{
/*N*/ 		// bTableDirty = FALSE;
/*N*/ 		if( !Load() )
/*N*/ 			Create();
/*N*/ 	}
/*N*/ 	long nPos = 0;
/*N*/ 	XPropertyEntry* pEntry = (XPropertyEntry*)aTable.First();
/*N*/ 	while (pEntry && pEntry->GetName() != rName)
/*N*/ 	{
/*N*/ 		nPos++;
/*N*/ 		pEntry = (XPropertyEntry*)aTable.Next();
/*N*/ 	}
/*N*/ 	if (!pEntry) nPos = -1;
/*N*/ 	return nPos;
/*N*/ }

/*************************************************************************
|*
|* void XPropertyTable::Insert()
|*
*************************************************************************/

/*N*/ BOOL XPropertyTable::Insert( long nIndex, XPropertyEntry* pEntry )
/*N*/ {
/*N*/ 	BOOL bReturn = aTable.Insert( (ULONG) nIndex, pEntry );
/*N*/ 
/*N*/ 	if( pBmpTable && !bBitmapsDirty )
/*N*/ 	{
/*N*/ 		Bitmap* pBmp = CreateBitmapForUI( (ULONG) nIndex );
/*N*/ 		pBmpTable->Insert( (ULONG) nIndex, pBmp );
/*N*/ 	}
/*N*/ 	return bReturn;
/*N*/ }

/*************************************************************************
|*
|* void XPropertyTable::Replace()
|*
*************************************************************************/

/*N*/ XPropertyEntry* XPropertyTable::Replace( long nIndex, XPropertyEntry* pEntry )
/*N*/ {
/*N*/ 	XPropertyEntry* pOldEntry = (XPropertyEntry*) aTable.Replace( (ULONG) nIndex, pEntry );
/*N*/ 
/*N*/ 	if( pBmpTable && !bBitmapsDirty )
/*N*/ 	{
/*N*/ 		Bitmap* pBmp = CreateBitmapForUI( (ULONG) nIndex );
/*N*/ 		Bitmap* pOldBmp = (Bitmap*) pBmpTable->Replace( (ULONG) nIndex, pBmp );
/*N*/ 		if( pOldBmp )
/*N*/ 			delete pOldBmp;
/*N*/ 	}
/*N*/ 	return pOldEntry;
/*N*/ }

/*************************************************************************
|*
|* void XPropertyTable::Remove()
|*
*************************************************************************/

/*N*/ XPropertyEntry* XPropertyTable::Remove( long nIndex, USHORT nDummy )
/*N*/ {
/*N*/ 	if( pBmpTable && !bBitmapsDirty )
/*N*/ 	{
/*N*/ 		Bitmap* pOldBmp = (Bitmap*) pBmpTable->Remove( (ULONG) nIndex );
/*N*/ 		if( pOldBmp )
/*N*/ 			delete pOldBmp;
/*N*/ 	}
/*N*/ 	return (XPropertyEntry*) aTable.Remove((ULONG)nIndex);
/*N*/ }

/************************************************************************/

/*N*/ void XPropertyTable::SetName( const String& rString )
/*N*/ {
/*N*/ 	if(rString.Len())
/*N*/ 	{
/*N*/ 		aName = rString;
/*N*/ 	}
/*N*/ }

// --------------------
// class XPropertyList
// --------------------


/*************************************************************************
|*
|* XPropertyList::XPropertyList()
|*
*************************************************************************/

/*N*/ XPropertyList::XPropertyList( const String& rPath,
/*N*/ 								XOutdevItemPool* pInPool,
/*N*/ 								USHORT nInitSize, USHORT nReSize ) :
/*N*/ 			aList			( nInitSize, nReSize ),
/*N*/ 			aPath			( rPath ),
/*N*/ 			aName			( pszStandard, 8 ),
/*N*/ 			pXPool			( pInPool ),
/*N*/ 			pBmpList		( NULL ),
/*N*/ 			bListDirty		( TRUE ),
/*N*/ 			bBitmapsDirty	( TRUE ),
/*N*/ 			bOwnPool		( FALSE )
/*N*/ {
/*N*/ 	if( !pXPool )
/*N*/ 	{
/*N*/ 		bOwnPool = TRUE;
/*N*/ 		pXPool = new XOutdevItemPool;
/*N*/ 		DBG_ASSERT( pXPool, "XOutPool konnte nicht erzeugt werden!" );
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* XPropertyList::~XPropertyList()
|*
*************************************************************************/

/*N*/ XPropertyList::~XPropertyList()
/*N*/ {
/*N*/ 	XPropertyEntry* pEntry = (XPropertyEntry*)aList.First();
/*N*/ 	Bitmap* pBitmap = NULL;
/*N*/ 	for( ULONG nIndex = 0; nIndex < aList.Count(); nIndex++ )
/*N*/ 	{
/*N*/ 		delete pEntry;
/*N*/ 		pEntry = (XPropertyEntry*)aList.Next();
/*N*/ 	}
/*N*/ 
/*N*/ 	if( pBmpList )
/*N*/ 	{
/*N*/ 		pBitmap = (Bitmap*) pBmpList->First();
/*N*/ 
/*N*/ 		for( ULONG nIndex = 0; nIndex < pBmpList->Count(); nIndex++ )
/*N*/ 		{
/*N*/ 			delete pBitmap;
/*N*/ 			pBitmap = (Bitmap*) pBmpList->Next();
/*N*/ 		}
/*N*/ 		delete pBmpList;
/*N*/ 		pBmpList = NULL;
/*N*/ 	}
/*N*/ 
/*N*/ 	if( bOwnPool && pXPool )
/*N*/ 	{
/*N*/ 		delete pXPool;
/*N*/ 	}
/*N*/ }

/************************************************************************/

/*N*/ long XPropertyList::Count() const
/*N*/ {
/*N*/ 	if( bListDirty )
/*N*/ 	{
/*N*/ 		// ( (XPropertyList*) this )->bListDirty = FALSE; <- im Load()
/*N*/ 		if( !( (XPropertyList*) this )->Load() )
/*N*/ 			( (XPropertyList*) this )->Create();
/*N*/ 	}
/*N*/ 	return( aList.Count() );
/*N*/ }

/*************************************************************************
|*
|* XPropertyEntry* XPropertyList::Get()
|*
*************************************************************************/

/*N*/ XPropertyEntry* XPropertyList::Get( long nIndex, USHORT nDummy ) const
/*N*/ {
/*N*/ 	if( bListDirty )
/*N*/ 	{
/*N*/ 		// ( (XPropertyList*) this )->bListDirty = FALSE; <- im Load()
/*N*/ 		if( !( (XPropertyList*) this )->Load() )
/*N*/ 			( (XPropertyList*) this )->Create();
/*N*/ 	}
/*N*/ 	return (XPropertyEntry*) aList.GetObject( (ULONG) nIndex );
/*N*/ }

/*************************************************************************
|*
|* XPropertyList::Get()
|*
*************************************************************************/

/*N*/ long XPropertyList::Get(const XubString& rName)
/*N*/ {
/*N*/ 	if( bListDirty )
/*N*/ 	{
/*N*/ 		//bListDirty = FALSE;
/*N*/ 		if( !Load() )
/*N*/ 			Create();
/*N*/ 	}
/*N*/ 	long nPos = 0;
/*N*/ 	XPropertyEntry* pEntry = (XPropertyEntry*)aList.First();
/*N*/ 	while (pEntry && pEntry->GetName() != rName)
/*N*/ 	{
/*N*/ 		nPos++;
/*N*/ 		pEntry = (XPropertyEntry*)aList.Next();
/*N*/ 	}
/*N*/ 	if (!pEntry) nPos = -1;
/*N*/ 	return nPos;
/*N*/ }

/*************************************************************************
|*
|* void XPropertyList::Insert()
|*
*************************************************************************/

/*N*/ void XPropertyList::Insert( XPropertyEntry* pEntry, long nIndex )
/*N*/ {
/*N*/ 	aList.Insert( pEntry, (ULONG) nIndex );
/*N*/ 
/*N*/ 	if( pBmpList && !bBitmapsDirty )
/*N*/ 	{
/*N*/ 		Bitmap* pBmp = CreateBitmapForUI(
/*N*/ 				(ULONG) nIndex < aList.Count() ? nIndex : aList.Count() - 1 );
/*N*/ 		pBmpList->Insert( pBmp, (ULONG) nIndex );
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* void XPropertyList::Replace()
|*
*************************************************************************/

/*N*/ XPropertyEntry* XPropertyList::Replace( XPropertyEntry* pEntry, long nIndex )
/*N*/ {
/*N*/ 	XPropertyEntry* pOldEntry = (XPropertyEntry*) aList.Replace( pEntry, (ULONG) nIndex );
/*N*/ 
/*N*/ 	if( pBmpList && !bBitmapsDirty )
/*N*/ 	{
/*N*/ 		Bitmap* pBmp = CreateBitmapForUI( (ULONG) nIndex );
/*N*/ 		Bitmap* pOldBmp = (Bitmap*) pBmpList->Replace( pBmp, (ULONG) nIndex );
/*N*/ 		if( pOldBmp )
/*N*/ 			delete pOldBmp;
/*N*/ 	}
/*N*/ 	return pOldEntry;
/*N*/ }

/*************************************************************************
|*
|* void XPropertyList::Remove()
|*
*************************************************************************/

/*N*/ XPropertyEntry* XPropertyList::Remove( long nIndex, USHORT nDummy )
/*N*/ {
/*N*/ 	if( pBmpList && !bBitmapsDirty )
/*N*/ 	{
/*N*/ 		Bitmap* pOldBmp = (Bitmap*) pBmpList->Remove( (ULONG) nIndex );
/*N*/ 		if( pOldBmp )
/*N*/ 			delete pOldBmp;
/*N*/ 	}
/*N*/ 	return (XPropertyEntry*) aList.Remove( (ULONG) nIndex );
/*N*/ }

/************************************************************************/

/*N*/ void XPropertyList::SetName( const String& rString )
/*N*/ {
/*N*/ 	if(rString.Len())
/*N*/ 	{
/*N*/ 		aName = rString;
/*N*/ 	}
/*N*/ }



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
