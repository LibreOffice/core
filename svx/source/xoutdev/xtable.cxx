/*************************************************************************
 *
 *  $RCSfile: xtable.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:28 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

#include "xtable.hxx"
#include "xpool.hxx"

#define GLOBALOVERFLOW

// Vergleichsstrings
sal_Unicode __FAR_DATA pszStandard[] = { 's', 't', 'a', 'n', 'd', 'a', 'r', 'd', '0' };

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
                                USHORT nInitSize, USHORT nReSize ) :
            aTable          ( nInitSize, nReSize ),
            aPath           ( rPath ),
            aName           ( pszStandard, 8 ),
            pXPool          ( pInPool ),
            pBmpTable       ( NULL ),
            bTableDirty     ( TRUE ),
            bBitmapsDirty   ( TRUE ),
            bOwnPool        ( FALSE )
{
    if( !pXPool )
    {
        bOwnPool = TRUE;
        pXPool = new XOutdevItemPool;
        DBG_ASSERT( pXPool, "XOutPool konnte nicht erzeugt werden!" );
    }
}

/*************************************************************************
|*
|* XPropertyTable::XPropertyTable( SvStraem& )
|*
*************************************************************************/

XPropertyTable::XPropertyTable( SvStream& rIn ) :
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
    for (ULONG nIndex = 0; nIndex < aTable.Count(); nIndex++)
    {
        delete pEntry;
        pEntry = (XPropertyEntry*)aTable.Next();
    }
    // Hier wird die Bitmaptabelle geloescht
    if( pBmpTable )
    {
        pBitmap = (Bitmap*) pBmpTable->First();

        for( ULONG nIndex = 0; nIndex < pBmpTable->Count(); nIndex++ )
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
        delete pXPool;
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
        // ( (XPropertyTable*) this )->bTableDirty = FALSE; <- im Load()
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

XPropertyEntry* XPropertyTable::Get( long nIndex, USHORT nDummy ) const
{
    if( bTableDirty )
    {
        // ( (XPropertyTable*) this )->bTableDirty = FALSE; <- im Load()
        if( !( (XPropertyTable*) this )->Load() )
            ( (XPropertyTable*) this )->Create();
    }
    return (XPropertyEntry*) aTable.GetObject( (ULONG) nIndex );
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
        // bTableDirty = FALSE;
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
            ( (XPropertyTable*) this )->bBitmapsDirty = FALSE;
            ( (XPropertyTable*) this )->CreateBitmapsForUI();
        }

        if( pBmpTable->Count() >= (ULONG) nIndex )
            return (Bitmap*) pBmpTable->GetObject( (ULONG) nIndex );
    }
    return( NULL );
}

/*************************************************************************
|*
|* void XPropertyTable::Insert()
|*
*************************************************************************/

BOOL XPropertyTable::Insert( long nIndex, XPropertyEntry* pEntry )
{
    BOOL bReturn = aTable.Insert( (ULONG) nIndex, pEntry );

    if( pBmpTable && !bBitmapsDirty )
    {
        Bitmap* pBmp = CreateBitmapForUI( (ULONG) nIndex );
        pBmpTable->Insert( (ULONG) nIndex, pBmp );
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
    XPropertyEntry* pOldEntry = (XPropertyEntry*) aTable.Replace( (ULONG) nIndex, pEntry );

    if( pBmpTable && !bBitmapsDirty )
    {
        Bitmap* pBmp = CreateBitmapForUI( (ULONG) nIndex );
        Bitmap* pOldBmp = (Bitmap*) pBmpTable->Replace( (ULONG) nIndex, pBmp );
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

XPropertyEntry* XPropertyTable::Remove( long nIndex, USHORT nDummy )
{
    if( pBmpTable && !bBitmapsDirty )
    {
        Bitmap* pOldBmp = (Bitmap*) pBmpTable->Remove( (ULONG) nIndex );
        if( pOldBmp )
            delete pOldBmp;
    }
    return (XPropertyEntry*) aTable.Remove((ULONG)nIndex);
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
                                USHORT nInitSize, USHORT nReSize ) :
            aList           ( nInitSize, nReSize ),
            aPath           ( rPath ),
            aName           ( pszStandard, 8 ),
            pXPool          ( pInPool ),
            pBmpList        ( NULL ),
            bListDirty      ( TRUE ),
            bBitmapsDirty   ( TRUE ),
            bOwnPool        ( FALSE )
{
    if( !pXPool )
    {
        bOwnPool = TRUE;
        pXPool = new XOutdevItemPool;
        DBG_ASSERT( pXPool, "XOutPool konnte nicht erzeugt werden!" );
    }
}

/*************************************************************************
|*
|* XPropertyList::XPropertyList( SvStraem& )
|*
*************************************************************************/

XPropertyList::XPropertyList( SvStream& rIn ) :
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
    for( ULONG nIndex = 0; nIndex < aList.Count(); nIndex++ )
    {
        delete pEntry;
        pEntry = (XPropertyEntry*)aList.Next();
    }

    if( pBmpList )
    {
        pBitmap = (Bitmap*) pBmpList->First();

        for( ULONG nIndex = 0; nIndex < pBmpList->Count(); nIndex++ )
        {
            delete pBitmap;
            pBitmap = (Bitmap*) pBmpList->Next();
        }
        delete pBmpList;
        pBmpList = NULL;
    }

    if( bOwnPool && pXPool )
    {
        delete pXPool;
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
        // ( (XPropertyList*) this )->bListDirty = FALSE; <- im Load()
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

XPropertyEntry* XPropertyList::Get( long nIndex, USHORT nDummy ) const
{
    if( bListDirty )
    {
        // ( (XPropertyList*) this )->bListDirty = FALSE; <- im Load()
        if( !( (XPropertyList*) this )->Load() )
            ( (XPropertyList*) this )->Create();
    }
    return (XPropertyEntry*) aList.GetObject( (ULONG) nIndex );
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
        //bListDirty = FALSE;
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
            ( (XPropertyList*) this )->bBitmapsDirty = FALSE;
            ( (XPropertyList*) this )->CreateBitmapsForUI();
        }
        if( pBmpList->Count() >= (ULONG) nIndex )
            return (Bitmap*) pBmpList->GetObject( (ULONG) nIndex );
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
    aList.Insert( pEntry, (ULONG) nIndex );

    if( pBmpList && !bBitmapsDirty )
    {
        Bitmap* pBmp = CreateBitmapForUI(
                (ULONG) nIndex < aList.Count() ? nIndex : aList.Count() - 1 );
        pBmpList->Insert( pBmp, (ULONG) nIndex );
    }
}

/*************************************************************************
|*
|* void XPropertyList::Replace()
|*
*************************************************************************/

XPropertyEntry* XPropertyList::Replace( XPropertyEntry* pEntry, long nIndex )
{
    XPropertyEntry* pOldEntry = (XPropertyEntry*) aList.Replace( pEntry, (ULONG) nIndex );

    if( pBmpList && !bBitmapsDirty )
    {
        Bitmap* pBmp = CreateBitmapForUI( (ULONG) nIndex );
        Bitmap* pOldBmp = (Bitmap*) pBmpList->Replace( pBmp, (ULONG) nIndex );
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

XPropertyEntry* XPropertyList::Remove( long nIndex, USHORT nDummy )
{
    if( pBmpList && !bBitmapsDirty )
    {
        Bitmap* pOldBmp = (Bitmap*) pBmpList->Remove( (ULONG) nIndex );
        if( pOldBmp )
            delete pOldBmp;
    }
    return (XPropertyEntry*) aList.Remove( (ULONG) nIndex );
}

/************************************************************************/

void XPropertyList::SetName( const String& rString )
{
    if(rString.Len())
    {
        aName = rString;
    }
}



