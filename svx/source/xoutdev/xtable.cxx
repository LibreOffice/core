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

#include <svx/XPropertyTable.hxx>
#include "xmlxtexp.hxx"
#include "xmlxtimp.hxx"
#include <tools/urlobj.hxx>
#include <svx/xtable.hxx>
#include <svx/xpool.hxx>

using namespace com::sun::star;

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

// --------------------
// class XPropertyList
// --------------------

XPropertyList::XPropertyList(
    const char *pDefaultExtension,
    const String& rPath,
    XOutdevItemPool* pInPool
) : aName           ( pszStandard, 8 ),
    aPath           ( rPath ),
    pXPool          ( pInPool ),
    pDefaultExt     ( pDefaultExtension ),
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

XPropertyList::~XPropertyList()
{
    for( size_t i = 0, n = aList.size(); i < n; ++i ) {
        delete aList[ i ];
    }
    aList.clear();

    if( pBmpList )
    {
        for ( size_t i = 0, n = pBmpList->size(); i < n; ++i ) {
            delete (*pBmpList)[ i ];
        }
        pBmpList->clear();
        delete pBmpList;
        pBmpList = NULL;
    }

    if( bOwnPool && pXPool )
    {
        SfxItemPool::Free(pXPool);
    }
}

void XPropertyList::Clear()
{
    for( size_t i = 0, n = aList.size(); i < n; ++i ) {
        delete aList[ i ];
    }
    aList.clear();
    if( pBmpList )
    {
        for ( size_t i = 0, n = pBmpList->size(); i < n; ++i ) {
            delete (*pBmpList)[ i ];
        }
        pBmpList->clear();
    }
}

long XPropertyList::Count() const
{
    if( bListDirty )
    {
        // ( (XPropertyList*) this )->bListDirty = sal_False; <- im Load()
        if( !( (XPropertyList*) this )->Load() )
            ( (XPropertyList*) this )->Create();
    }
    return( aList.size() );
}

XPropertyEntry* XPropertyList::Get( long nIndex, sal_uInt16 /*nDummy*/) const
{
    if( bListDirty )
    {
        // ( (XPropertyList*) this )->bListDirty = sal_False; <- im Load()
        if( !( (XPropertyList*) this )->Load() )
            ( (XPropertyList*) this )->Create();
    }
    return ( (size_t)nIndex < aList.size() ) ? aList[ nIndex ] : NULL;
}

long XPropertyList::Get(const XubString& rName)
{
    if( bListDirty )
    {
        //bListDirty = sal_False;
        if( !Load() )
            Create();
    }

    for( long i = 0, n = aList.size(); i < n; ++i ) {
        if ( aList[ i ]->GetName() == rName ) {
            return i;
        }
    }
    return -1;
}

Bitmap* XPropertyList::GetBitmap( long nIndex ) const
{
    if( pBmpList )
    {
        if( bBitmapsDirty )
        {
            ( (XPropertyList*) this )->bBitmapsDirty = sal_False;
            ( (XPropertyList*) this )->CreateBitmapsForUI();
        }
        if( (size_t)nIndex < pBmpList->size() )
            return (*pBmpList)[ nIndex ];
    }
    return NULL;
}

void XPropertyList::Insert( XPropertyEntry* pEntry, long nIndex )
{
    if ( (size_t)nIndex < aList.size() ) {
        aList.insert( aList.begin() + nIndex, pEntry );
    } else {
        aList.push_back( pEntry );
    }

    if( pBmpList && !bBitmapsDirty )
    {
        Bitmap* pBmp = CreateBitmapForUI(
            (size_t)nIndex < aList.size() ? nIndex : aList.size() - 1
        );
        if ( (size_t)nIndex < pBmpList->size() ) {
            pBmpList->insert( pBmpList->begin() + nIndex, pBmp );
        } else {
            pBmpList->push_back( pBmp );
        }
    }
}

XPropertyEntry* XPropertyList::Replace( XPropertyEntry* pEntry, long nIndex )
{
    XPropertyEntry* pOldEntry = (size_t)nIndex < aList.size() ? aList[ nIndex ] : NULL;
    if ( pOldEntry ) {
        aList[ nIndex ] = pEntry;
    }

    if( pBmpList && !bBitmapsDirty )
    {
        Bitmap* pBmp = CreateBitmapForUI( (sal_uIntPtr) nIndex );
        if ( (size_t)nIndex < pBmpList->size() )
        {
            delete (*pBmpList)[ nIndex ];
            (*pBmpList)[ nIndex ] = pBmp;
        }
        else {
            pBmpList->push_back( pBmp );
        }
    }
    return pOldEntry;
}

XPropertyEntry* XPropertyList::Remove( long nIndex )
{
    if( pBmpList && !bBitmapsDirty )
    {
        if ( (size_t)nIndex < pBmpList->size() )
        {
            delete (*pBmpList)[ nIndex ];
            pBmpList->erase( pBmpList->begin() + nIndex );
        }
    }

    XPropertyEntry* pEntry = NULL;
    if ( (size_t)nIndex < aList.size() ) {
        pEntry = aList[ nIndex ];
        aList.erase( aList.begin() + nIndex );
    }
    return pEntry;
}

void XPropertyList::SetName( const String& rString )
{
    if(rString.Len())
    {
        aName = rString;
    }
}

sal_Bool XPropertyList::Load()
{
    if( bListDirty )
    {
        bListDirty = sal_False;

        INetURLObject aURL( aPath );

        if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
        {
            DBG_ASSERT( !aPath.Len(), "invalid URL" );
            return sal_False;
        }

        aURL.Append( aName );

        if( !aURL.getExtension().getLength() )
            aURL.setExtension( rtl::OUString::createFromAscii( pDefaultExt ) );

        return SvxXMLXTableImport::load( aURL.GetMainURL( INetURLObject::NO_DECODE ), createInstance() );

    }
    return sal_False;
}

sal_Bool XPropertyList::Save()
{
    INetURLObject aURL( aPath );

    if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
    {
        DBG_ASSERT( !aPath.Len(), "invalid URL" );
        return sal_False;
    }

    aURL.Append( aName );

    if( !aURL.getExtension().getLength() )
        aURL.setExtension( rtl::OUString::createFromAscii( pDefaultExt ) );

    return SvxXMLXTableExportComponent::save( aURL.GetMainURL( INetURLObject::NO_DECODE ), createInstance() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
