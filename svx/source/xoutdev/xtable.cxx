/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <svx/XPropertyTable.hxx>
#include "xmlxtexp.hxx"
#include "xmlxtimp.hxx"
#include <tools/urlobj.hxx>
#include <svx/xtable.hxx>
#include <svx/xpool.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpool.hxx>

using namespace com::sun::star;

// Helper for other sub-classes to have easy-to-read constructors
Color RGB_Color( ColorData nColorName )
{
    Color aColor( nColorName );
    Color aRGBColor( aColor.GetRed(), aColor.GetGreen(), aColor.GetBlue() );
    return aRGBColor;
}

// class XColorEntry

XColorEntry::XColorEntry(const Color& rColor, const String& rName)
:   XPropertyEntry(rName),
    aColor(rColor)
{
}

XColorEntry::XColorEntry(const XColorEntry& rOther)
:   XPropertyEntry(rOther),
aColor(rOther.aColor)
{
}

// class XLineEndEntry

XLineEndEntry::XLineEndEntry(const basegfx::B2DPolyPolygon& rB2DPolyPolygon, const String& rName)
:   XPropertyEntry(rName),
    aB2DPolyPolygon(rB2DPolyPolygon)
{
}

XLineEndEntry::XLineEndEntry(const XLineEndEntry& rOther)
:   XPropertyEntry(rOther),
    aB2DPolyPolygon(rOther.aB2DPolyPolygon)
{
}

// class XDashEntry

XDashEntry::XDashEntry(const XDash& rDash, const String& rName)
:   XPropertyEntry(rName),
    aDash(rDash)
{
}

XDashEntry::XDashEntry(const XDashEntry& rOther)
:   XPropertyEntry(rOther),
aDash(rOther.aDash)
{
}

// class XHatchEntry

XHatchEntry::XHatchEntry(const XHatch& rHatch, const String& rName)
:   XPropertyEntry(rName),
    aHatch(rHatch)
{
}

XHatchEntry::XHatchEntry(const XHatchEntry& rOther)
:   XPropertyEntry(rOther),
    aHatch(rOther.aHatch)
{
}

// class XGradientEntry

XGradientEntry::XGradientEntry(const XGradient& rGradient, const String& rName)
:   XPropertyEntry(rName),
    aGradient(rGradient)
{
}

XGradientEntry::XGradientEntry(const XGradientEntry& rOther)
:   XPropertyEntry(rOther),
    aGradient(rOther.aGradient)
{
}

// class XBitmapEntry

XBitmapEntry::XBitmapEntry(const GraphicObject& rGraphicObject, const String& rName)
:   XPropertyEntry(rName),
    maGraphicObject(rGraphicObject)
{
}

XBitmapEntry::XBitmapEntry(const XBitmapEntry& rOther)
:   XPropertyEntry(rOther),
    maGraphicObject(rOther.maGraphicObject)
{
}

XPropertyList::XPropertyList(
    XPropertyListType type,
    const String& rPath
) : meType           ( type ),
    maName           ( RTL_CONSTASCII_USTRINGPARAM( "standard" ) ),
    maPath           ( rPath ),
    mbListDirty      ( true ),
    mbEmbedInDocument( false )
{
//    fprintf (stderr, "Create type %d count %d\n", (int)meType, count++);
}

XPropertyList::~XPropertyList()
{
//    fprintf (stderr, "Destroy type %d count %d\n", (int)meType, --count);
    for( size_t i = 0, n = maList.size(); i < n; ++i )
        delete maList[ i ];

    maList.clear();
}

long XPropertyList::Count() const
{
    if( mbListDirty )
    {
        if( !( (XPropertyList*) this )->Load() )
            ( (XPropertyList*) this )->Create();
    }
    return maList.size();
}

XPropertyEntry* XPropertyList::Get( long nIndex ) const
{
    if( mbListDirty )
    {
        if( !( (XPropertyList*) this )->Load() )
            ( (XPropertyList*) this )->Create();
    }
    return ( (size_t)nIndex < maList.size() ) ? maList[ nIndex ] : NULL;
}

long XPropertyList::GetIndex(const OUString& rName) const
{
    if( mbListDirty )
    {
        if( !( (XPropertyList*) this )->Load() )
            ( (XPropertyList*) this )->Create();
    }

    for( long i = 0, n = maList.size(); i < n; ++i ) {
        if (rName.equals(maList[ i ]->GetName())) {
            return i;
        }
    }
    return -1;
}

Bitmap XPropertyList::GetUiBitmap( long nIndex ) const
{
    Bitmap aRetval;
    XPropertyEntry* pEntry = ( (size_t)nIndex < maList.size() ) ? maList[ nIndex ] : NULL;
    if(pEntry)
    {
        aRetval = pEntry->GetUiBitmap();

        if(aRetval.IsEmpty())
        {
            aRetval = const_cast< XPropertyList* >(this)->CreateBitmapForUI(nIndex);
            pEntry->SetUiBitmap(aRetval);
        }
    }
    return aRetval;
}

void XPropertyList::Insert( XPropertyEntry* pEntry, long nIndex )
{
    if ( (size_t)nIndex < maList.size() ) {
        maList.insert( maList.begin() + nIndex, pEntry );
    } else {
        maList.push_back( pEntry );
    }
}

XPropertyEntry* XPropertyList::Replace( XPropertyEntry* pEntry, long nIndex )
{
    XPropertyEntry* pOldEntry = (size_t)nIndex < maList.size() ? maList[ nIndex ] : NULL;
    if ( pOldEntry ) {
        maList[ nIndex ] = pEntry;
    }
    return pOldEntry;
}

XPropertyEntry* XPropertyList::Remove( long nIndex )
{
    XPropertyEntry* pEntry = NULL;
    if ( (size_t)nIndex < maList.size() ) {
        pEntry = maList[ nIndex ];
        maList.erase( maList.begin() + nIndex );
    }
    return pEntry;
}

void XPropertyList::SetName( const String& rString )
{
    if(rString.Len())
    {
        maName = rString;
    }
}

bool XPropertyList::Load()
{
    if( mbListDirty )
    {
        mbListDirty = false;

        INetURLObject aURL( maPath );

        if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
        {
            DBG_ASSERT( !maPath.Len(), "invalid URL" );
            return false;
        }

        aURL.Append( maName );

        if( aURL.getExtension().isEmpty() )
            aURL.setExtension( GetDefaultExt() );

        return SvxXMLXTableImport::load( aURL.GetMainURL( INetURLObject::NO_DECODE ),
                                         uno::Reference < embed::XStorage >(),
                                         createInstance(), NULL );
    }
    return false;
}

bool XPropertyList::LoadFrom( const uno::Reference < embed::XStorage > &xStorage,
                                  const OUString &rURL )
{
    if( !mbListDirty )
        return false;
    mbListDirty = false;
    return SvxXMLXTableImport::load( rURL, xStorage, createInstance(), &mbEmbedInDocument );
}

bool XPropertyList::Save()
{
    INetURLObject aURL( maPath );

    if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
    {
        DBG_ASSERT( !maPath.Len(), "invalid URL" );
        return false;
    }

    aURL.Append( maName );

    if( aURL.getExtension().isEmpty() )
        aURL.setExtension( GetDefaultExt() );

    return SvxXMLXTableExportComponent::save( aURL.GetMainURL( INetURLObject::NO_DECODE ),
                                              createInstance(),
                                              uno::Reference< embed::XStorage >(), NULL );
}

bool XPropertyList::SaveTo( const uno::Reference< embed::XStorage > &xStorage,
                            const OUString &rURL, OUString *pOptName )
{
    return SvxXMLXTableExportComponent::save( rURL, createInstance(), xStorage, pOptName );
}

XPropertyListRef XPropertyList::CreatePropertyList( XPropertyListType t,
                                                    const String& rPath )
{
    XPropertyListRef pRet;

#define MAP(e,c) \
        case e: pRet = XPropertyListRef (new c( rPath ) ); break
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
    OSL_ASSERT( !pRet.is() || pRet->meType == t );

    return pRet;
}

XPropertyListRef
XPropertyList::CreatePropertyListFromURL( XPropertyListType t,
                                          const OUString & rURLStr )
{
    INetURLObject aURL( rURLStr );
    INetURLObject aPathURL( aURL );

    aPathURL.removeSegment();
    aPathURL.removeFinalSlash();

    XPropertyListRef pList = XPropertyList::CreatePropertyList(
        t, aPathURL.GetMainURL( INetURLObject::NO_DECODE ) );
    pList->SetName( aURL.getName() );

    return pList;
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

static struct {
    XPropertyListType t;
    const char *pExt;
} pExtnMap[] = {
    { XCOLOR_LIST,    "soc" },
    { XLINE_END_LIST, "soe" },
    { XDASH_LIST,     "sod" },
    { XHATCH_LIST,    "soh" },
    { XGRADIENT_LIST, "sog" },
    { XBITMAP_LIST,   "sob" }
};

OUString XPropertyList::GetDefaultExt( XPropertyListType t )
{
    for (size_t i = 0; i < SAL_N_ELEMENTS (pExtnMap); i++)
    {
        if( pExtnMap[i].t == t )
            return OUString::createFromAscii( pExtnMap[ i ].pExt );
    }
    return OUString();
}

OUString XPropertyList::GetDefaultExtFilter( XPropertyListType t )
{
    OUString aFilter( "*." );
    return aFilter + GetDefaultExt( t );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
