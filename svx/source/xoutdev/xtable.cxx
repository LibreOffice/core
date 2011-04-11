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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/xtable.hxx>
#include <svx/xpool.hxx>

#define GLOBALOVERFLOW

// Vergleichsstrings
sal_Unicode pszStandard[] = { 's', 't', 'a', 'n', 'd', 'a', 'r', 'd', 0 };

// Konvertiert in echte RGB-Farben, damit in den Listboxen
// endlich mal richtig selektiert werden kann.
Color RGB_Color( ColorData nColorName )
{
    Color aColor( nColorName );
    Color aRGBColor( aColor.GetRed(), aColor.GetGreen(), aColor.GetBlue() );
    return aRGBColor;
}

// ---------------------
// class XPropertyTable
// ---------------------

/*************************************************************************
|*
|* XPropertyTable::XPropertyTable()
|*
*************************************************************************/

XPropertyTable::XPropertyTable( const String& rPath,
                                XOutdevItemPool* pInPool,
                                sal_uInt16 nInitSize, sal_uInt16 nReSize ) :
            aName           ( pszStandard, 8 ),
            aPath           ( rPath ),
            pXPool          ( pInPool ),
            aTable          ( nInitSize, nReSize ),
            pBmpTable       ( NULL ),
            bTableDirty     ( sal_True ),
            bBitmapsDirty   ( sal_True ),
            bOwnPool        ( sal_False )
{
    if( !pXPool )
    {
        bOwnPool = sal_True;
        pXPool = new XOutdevItemPool;
        DBG_ASSERT( pXPool, "XOutPool konnte nicht erzeugt werden!" );
    }
}

/*************************************************************************
|*
|* XPropertyTable::XPropertyTable( SvStraem& )
|*
*************************************************************************/

XPropertyTable::XPropertyTable( SvStream& /*rIn*/) :
            pBmpTable   ( NULL )
{
}

/*************************************************************************
|*
|* XPropertyTable::~XPropertyTable()
|*
*************************************************************************/

XPropertyTable::~XPropertyTable()
{
    XPropertyEntry* pEntry = (XPropertyEntry*)aTable.First();
    Bitmap* pBitmap = NULL;
    for (sal_uIntPtr nIndex = 0; nIndex < aTable.Count(); nIndex++)
    {
        delete pEntry;
        pEntry = (XPropertyEntry*)aTable.Next();
    }
    // Hier wird die Bitmaptabelle geloescht
    if( pBmpTable )
    {
        pBitmap = (Bitmap*) pBmpTable->First();

        for( sal_uIntPtr nIndex = 0; nIndex < pBmpTable->Count(); nIndex++ )
        {
            delete pBitmap;
            pBitmap = (Bitmap*) pBmpTable->Next();
        }
        delete pBmpTable;
        pBmpTable = NULL;
    }
    // Eigener Pool wird geloescht
    if( bOwnPool && pXPool )
    {
        SfxItemPool::Free(pXPool);
    }
}

/*************************************************************************
|*
|* XPropertyTable::Clear()
|*
*************************************************************************/

void XPropertyTable::Clear()
{
    aTable.Clear();
    if( pBmpTable )
        pBmpTable->Clear();
}

/************************************************************************/

long XPropertyTable::Count() const
{
    if( bTableDirty )
    {
        // ( (XPropertyTable*) this )->bTableDirty = sal_False; <- im Load()
        if( !( (XPropertyTable*) this )->Load() )
            ( (XPropertyTable*) this )->Create();
    }
    return( aTable.Count() );
}

/*************************************************************************
|*
|* XPropertyEntry* XPropertyTable::Get()
|*
*************************************************************************/

XPropertyEntry* XPropertyTable::Get( long nIndex, sal_uInt16 /*nDummy*/) const
{
    if( bTableDirty )
    {
        // ( (XPropertyTable*) this )->bTableDirty = sal_False; <- im Load()
        if( !( (XPropertyTable*) this )->Load() )
            ( (XPropertyTable*) this )->Create();
    }
    return (XPropertyEntry*) aTable.GetObject( (sal_uIntPtr) nIndex );
}

/*************************************************************************
|*
|* long XPropertyTable::Get(const String& rName)
|*
*************************************************************************/

long XPropertyTable::Get(const XubString& rName)
{
    if( bTableDirty )
    {
        // bTableDirty = sal_False;
        if( !Load() )
            Create();
    }
    long nPos = 0;
    XPropertyEntry* pEntry = (XPropertyEntry*)aTable.First();
    while (pEntry && pEntry->GetName() != rName)
    {
        nPos++;
        pEntry = (XPropertyEntry*)aTable.Next();
    }
    if (!pEntry) nPos = -1;
    return nPos;
}

/*************************************************************************
|*
|* Bitmap* XPropertyTable::GetBitmap()
|*
*************************************************************************/

Bitmap* XPropertyTable::GetBitmap( long nIndex ) const
{
    if( pBmpTable )
    {
        if( bBitmapsDirty )
        {
            ( (XPropertyTable*) this )->bBitmapsDirty = sal_False;
            ( (XPropertyTable*) this )->CreateBitmapsForUI();
        }

        if( pBmpTable->Count() >= (sal_uIntPtr) nIndex )
            return (Bitmap*) pBmpTable->GetObject( (sal_uIntPtr) nIndex );
    }
    return( NULL );
}

/*************************************************************************
|*
|* void XPropertyTable::Insert()
|*
*************************************************************************/

sal_Bool XPropertyTable::Insert( long nIndex, XPropertyEntry* pEntry )
{
    sal_Bool bReturn = aTable.Insert( (sal_uIntPtr) nIndex, pEntry );

    if( pBmpTable && !bBitmapsDirty )
    {
        Bitmap* pBmp = CreateBitmapForUI( (sal_uIntPtr) nIndex );
        pBmpTable->Insert( (sal_uIntPtr) nIndex, pBmp );
    }
    return bReturn;
}

/*************************************************************************
|*
|* void XPropertyTable::Replace()
|*
*************************************************************************/

XPropertyEntry* XPropertyTable::Replace( long nIndex, XPropertyEntry* pEntry )
{
    XPropertyEntry* pOldEntry = (XPropertyEntry*) aTable.Replace( (sal_uIntPtr) nIndex, pEntry );

    if( pBmpTable && !bBitmapsDirty )
    {
        Bitmap* pBmp = CreateBitmapForUI( (sal_uIntPtr) nIndex );
        Bitmap* pOldBmp = (Bitmap*) pBmpTable->Replace( (sal_uIntPtr) nIndex, pBmp );
        if( pOldBmp )
            delete pOldBmp;
    }
    return pOldEntry;
}

/*************************************************************************
|*
|* void XPropertyTable::Remove()
|*
*************************************************************************/

XPropertyEntry* XPropertyTable::Remove( long nIndex, sal_uInt16 /*nDummy*/)
{
    if( pBmpTable && !bBitmapsDirty )
    {
        Bitmap* pOldBmp = (Bitmap*) pBmpTable->Remove( (sal_uIntPtr) nIndex );
        if( pOldBmp )
            delete pOldBmp;
    }
    return (XPropertyEntry*) aTable.Remove((sal_uIntPtr)nIndex);
}

/************************************************************************/

void XPropertyTable::SetName( const String& rString )
{
    if(rString.Len())
    {
        aName = rString;
    }
}

// --------------------
// class XPropertyList
// --------------------


/*************************************************************************
|*
|* XPropertyList::XPropertyList()
|*
*************************************************************************/

XPropertyList::XPropertyList( const String& rPath,
                                XOutdevItemPool* pInPool,
                                sal_uInt16 nInitSize, sal_uInt16 nReSize ) :
            aName           ( pszStandard, 8 ),
            aPath           ( rPath ),
            pXPool          ( pInPool ),
            aList           ( nInitSize, nReSize ),
            pBmpList        ( NULL ),
            bListDirty      ( sal_True ),
            bBitmapsDirty   ( sal_True ),
            bOwnPool        ( sal_False )
{
    if( !pXPool )
    {
        bOwnPool = sal_True;
        pXPool = new XOutdevItemPool;
        DBG_ASSERT( pXPool, "XOutPool konnte nicht erzeugt werden!" );
    }
}

/*************************************************************************
|*
|* XPropertyList::XPropertyList( SvStraem& )
|*
*************************************************************************/

XPropertyList::XPropertyList( SvStream& /*rIn*/) :
            pBmpList    ( NULL )
{
}

/*************************************************************************
|*
|* XPropertyList::~XPropertyList()
|*
*************************************************************************/

XPropertyList::~XPropertyList()
{
    XPropertyEntry* pEntry = (XPropertyEntry*)aList.First();
    Bitmap* pBitmap = NULL;
    for( sal_uIntPtr nIndex = 0; nIndex < aList.Count(); nIndex++ )
    {
        delete pEntry;
        pEntry = (XPropertyEntry*)aList.Next();
    }

    if( pBmpList )
    {
        pBitmap = (Bitmap*) pBmpList->First();

        for( sal_uIntPtr nIndex = 0; nIndex < pBmpList->Count(); nIndex++ )
        {
            delete pBitmap;
            pBitmap = (Bitmap*) pBmpList->Next();
        }
        delete pBmpList;
        pBmpList = NULL;
    }

    if( bOwnPool && pXPool )
    {
        SfxItemPool::Free(pXPool);
    }
}

/*************************************************************************
|*
|* XPropertyList::Clear()
|*
*************************************************************************/

void XPropertyList::Clear()
{
    aList.Clear();
    if( pBmpList )
        pBmpList->Clear();
}

/************************************************************************/

long XPropertyList::Count() const
{
    if( bListDirty )
    {
        // ( (XPropertyList*) this )->bListDirty = sal_False; <- im Load()
        if( !( (XPropertyList*) this )->Load() )
            ( (XPropertyList*) this )->Create();
    }
    return( aList.Count() );
}

/*************************************************************************
|*
|* XPropertyEntry* XPropertyList::Get()
|*
*************************************************************************/

XPropertyEntry* XPropertyList::Get( long nIndex, sal_uInt16 /*nDummy*/) const
{
    if( bListDirty )
    {
        // ( (XPropertyList*) this )->bListDirty = sal_False; <- im Load()
        if( !( (XPropertyList*) this )->Load() )
            ( (XPropertyList*) this )->Create();
    }
    return (XPropertyEntry*) aList.GetObject( (sal_uIntPtr) nIndex );
}

/*************************************************************************
|*
|* XPropertyList::Get()
|*
*************************************************************************/

long XPropertyList::Get(const XubString& rName)
{
    if( bListDirty )
    {
        //bListDirty = sal_False;
        if( !Load() )
            Create();
    }
    long nPos = 0;
    XPropertyEntry* pEntry = (XPropertyEntry*)aList.First();
    while (pEntry && pEntry->GetName() != rName)
    {
        nPos++;
        pEntry = (XPropertyEntry*)aList.Next();
    }
    if (!pEntry) nPos = -1;
    return nPos;
}

/*************************************************************************
|*
|* Bitmap* XPropertyList::GetBitmap()
|*
*************************************************************************/

Bitmap* XPropertyList::GetBitmap( long nIndex ) const
{
    if( pBmpList )
    {
        if( bBitmapsDirty )
        {
            ( (XPropertyList*) this )->bBitmapsDirty = sal_False;
            ( (XPropertyList*) this )->CreateBitmapsForUI();
        }
        if( pBmpList->Count() >= (sal_uIntPtr) nIndex )
            return (Bitmap*) pBmpList->GetObject( (sal_uIntPtr) nIndex );
    }
    return( NULL );
}

/*************************************************************************
|*
|* void XPropertyList::Insert()
|*
*************************************************************************/

void XPropertyList::Insert( XPropertyEntry* pEntry, long nIndex )
{
    aList.Insert( pEntry, (sal_uIntPtr) nIndex );

    if( pBmpList && !bBitmapsDirty )
    {
        Bitmap* pBmp = CreateBitmapForUI(
                (sal_uIntPtr) nIndex < aList.Count() ? nIndex : aList.Count() - 1 );
        pBmpList->Insert( pBmp, (sal_uIntPtr) nIndex );
    }
}

/*************************************************************************
|*
|* void XPropertyList::Replace()
|*
*************************************************************************/

XPropertyEntry* XPropertyList::Replace( XPropertyEntry* pEntry, long nIndex )
{
    XPropertyEntry* pOldEntry = (XPropertyEntry*) aList.Replace( pEntry, (sal_uIntPtr) nIndex );

    if( pBmpList && !bBitmapsDirty )
    {
        Bitmap* pBmp = CreateBitmapForUI( (sal_uIntPtr) nIndex );
        Bitmap* pOldBmp = (Bitmap*) pBmpList->Replace( pBmp, (sal_uIntPtr) nIndex );
        if( pOldBmp )
            delete pOldBmp;
    }
    return pOldEntry;
}

/*************************************************************************
|*
|* void XPropertyList::Remove()
|*
*************************************************************************/

XPropertyEntry* XPropertyList::Remove( long nIndex, sal_uInt16 /*nDummy*/)
{
    if( pBmpList && !bBitmapsDirty )
    {
        Bitmap* pOldBmp = (Bitmap*) pBmpList->Remove( (sal_uIntPtr) nIndex );
        if( pOldBmp )
            delete pOldBmp;
    }
    return (XPropertyEntry*) aList.Remove( (sal_uIntPtr) nIndex );
}

/************************************************************************/

void XPropertyList::SetName( const String& rString )
{
    if(rString.Len())
    {
        aName = rString;
    }
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
