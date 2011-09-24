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

// Helper for other sub-classes to have easy-to-read constructors
Color RGB_Color( ColorData nColorName )
{
    Color aColor( nColorName );
    Color aRGBColor( aColor.GetRed(), aColor.GetGreen(), aColor.GetBlue() );
    return aRGBColor;
}

static int count = 0;

XPropertyList::XPropertyList(
    XPropertyListType type,
    const char *pDefaultExtension,
    const String& rPath,
    XOutdevItemPool* pInPool
) : eType           ( type ),
    aName           ( RTL_CONSTASCII_USTRINGPARAM( "standard" ) ),
    aPath           ( rPath ),
    pXPool          ( pInPool ),
    pDefaultExt     ( pDefaultExtension ),
    pBmpList        ( NULL ),
    bListDirty      ( true ),
    bBitmapsDirty   ( true ),
    bOwnPool        ( false ),
    bEmbedInDocument( false )
{
    if( !pXPool )
    {
        bOwnPool = true;
        pXPool = new XOutdevItemPool;
        DBG_ASSERT( pXPool, "XOutPool konnte nicht erzeugt werden!" );
    }
//    fprintf (stderr, "Create type %d count %d\n", (int)eType, count++);
}

XPropertyList::~XPropertyList()
{
//    fprintf (stderr, "Destroy type %d count %d\n", (int)eType, --count);
    for( size_t i = 0, n = aList.size(); i < n; ++i )
        delete aList[ i ];

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
        SfxItemPool::Free(pXPool);
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
        if( !( (XPropertyList*) this )->Load() )
            ( (XPropertyList*) this )->Create();
    }
    return( aList.size() );
}

XPropertyEntry* XPropertyList::Get( long nIndex, sal_uInt16 /*nDummy*/) const
{
    if( bListDirty )
    {
        if( !( (XPropertyList*) this )->Load() )
            ( (XPropertyList*) this )->Create();
    }
    return ( (size_t)nIndex < aList.size() ) ? aList[ nIndex ] : NULL;
}

long XPropertyList::Get(const XubString& rName)
{
    if( bListDirty )
    {
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
            ( (XPropertyList*) this )->bBitmapsDirty = false;
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

bool XPropertyList::Load()
{
    if( bListDirty )
    {
        bListDirty = false;

        INetURLObject aURL( aPath );

        if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
        {
            DBG_ASSERT( !aPath.Len(), "invalid URL" );
            return false;
        }

        aURL.Append( aName );

        if( !aURL.getExtension().getLength() )
            aURL.setExtension( rtl::OUString::createFromAscii( pDefaultExt ) );

        return SvxXMLXTableImport::load( aURL.GetMainURL( INetURLObject::NO_DECODE ),
                                         uno::Reference < embed::XStorage >(),
                                         createInstance(), NULL );

    }
    return false;
}

bool XPropertyList::LoadFrom( const uno::Reference < embed::XStorage > &xStorage,
                                  const rtl::OUString &rURL )
{
    if( !bListDirty )
        return false;
    bListDirty = false;
    return SvxXMLXTableImport::load( rURL, xStorage, createInstance(), &bEmbedInDocument );
}

bool XPropertyList::Save()
{
    INetURLObject aURL( aPath );

    if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
    {
        DBG_ASSERT( !aPath.Len(), "invalid URL" );
        return false;
    }

    aURL.Append( aName );

    if( !aURL.getExtension().getLength() )
        aURL.setExtension( rtl::OUString::createFromAscii( pDefaultExt ) );

    return SvxXMLXTableExportComponent::save( aURL.GetMainURL( INetURLObject::NO_DECODE ),
                                              createInstance(),
                                              uno::Reference< embed::XStorage >(), NULL );
}

bool XPropertyList::SaveTo( const uno::Reference< embed::XStorage > &xStorage,
                            const rtl::OUString &rURL, rtl::OUString *pOptName )
{
    return SvxXMLXTableExportComponent::save( rURL, createInstance(), xStorage, pOptName );
}

XPropertyListRef XPropertyList::CreatePropertyList( XPropertyListType t,
                                                    const String& rPath,
                                                    XOutdevItemPool* pXPool )
{
    XPropertyListRef pRet;

#define MAP(e,c) \
        case e: pRet = XPropertyListRef (new c( rPath, pXPool ) ); break
    switch (t) {
        MAP( XCOLOR_LIST, XColorList );
        MAP( XLINE_END_LIST, XLineEndList );
        MAP( XDASH_LIST, XDashList );
        MAP( XHATCH_LIST, XHatchList );
        MAP( XGRADIENT_LIST, XGradientList );
        MAP( XBITMAP_LIST, XBitmapList );
    default:
        OSL_FAIL("unknown xproperty type");
        break;
    }
#undef MAP
    OSL_ASSERT( !pRet.is() || pRet->eType == t );

    return pRet;
}

// catch people being silly with ref counting ...

void* XPropertyList::operator new (size_t nCount)
{
    return rtl_allocateMemory( nCount );
}

void XPropertyList::operator delete(void *pPtr)
{
    return rtl_freeMemory( pPtr );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
