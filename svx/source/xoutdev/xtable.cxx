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

#include <memory>
#include <svx/XPropertyTable.hxx>
#include <xmlxtexp.hxx>
#include <xmlxtimp.hxx>
#include <tools/urlobj.hxx>
#include <svx/xtable.hxx>
#include <svx/xpool.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpool.hxx>
#include <vcl/outdev.hxx>
#include <tools/debug.hxx>
#include <stack>

using namespace com::sun::star;

XColorEntry::XColorEntry(const Color& rColor, const OUString& rName)
:   XPropertyEntry(rName),
    aColor(rColor)
{
}

XLineEndEntry::XLineEndEntry(const basegfx::B2DPolyPolygon& rB2DPolyPolygon, const OUString& rName)
:   XPropertyEntry(rName),
    aB2DPolyPolygon(rB2DPolyPolygon)
{
}

XLineEndEntry::XLineEndEntry(const XLineEndEntry& rOther)
:   XPropertyEntry(rOther),
    aB2DPolyPolygon(rOther.aB2DPolyPolygon)
{
}

XDashEntry::XDashEntry(const XDash& rDash, const OUString& rName)
:   XPropertyEntry(rName),
    aDash(rDash)
{
}

XDashEntry::XDashEntry(const XDashEntry& rOther)
:   XPropertyEntry(rOther),
aDash(rOther.aDash)
{
}

XHatchEntry::XHatchEntry(const XHatch& rHatch, const OUString& rName)
:   XPropertyEntry(rName),
    aHatch(rHatch)
{
}

XHatchEntry::XHatchEntry(const XHatchEntry& rOther)
:   XPropertyEntry(rOther),
    aHatch(rOther.aHatch)
{
}

XGradientEntry::XGradientEntry(const XGradient& rGradient, const OUString& rName)
:   XPropertyEntry(rName),
    aGradient(rGradient)
{
}

XGradientEntry::XGradientEntry(const XGradientEntry& rOther)
:   XPropertyEntry(rOther),
    aGradient(rOther.aGradient)
{
}

XBitmapEntry::XBitmapEntry(const GraphicObject& rGraphicObject, const OUString& rName)
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
    const OUString& rPath, const OUString& rReferer
) : meType           ( type ),
    maName           ( "standard" ),
    maPath           ( rPath ),
    maReferer        ( rReferer ),
    mbListDirty      ( true ),
    mbEmbedInDocument( false )
{
//    fprintf (stderr, "Create type %d count %d\n", (int)meType, count++);
}

bool XPropertyList::isValidIdx(long nIndex) const
{
    return (static_cast<size_t>(nIndex) < maList.size() && nIndex >= 0);
}


XPropertyList::~XPropertyList()
{
}

long XPropertyList::Count() const
{
    if( mbListDirty )
    {
        if( !const_cast<XPropertyList*>(this)->Load() )
            const_cast<XPropertyList*>(this)->Create();
    }
    return maList.size();
}

XPropertyEntry* XPropertyList::Get( long nIndex ) const
{
    if( mbListDirty )
    {
        if( !const_cast<XPropertyList*>(this)->Load() )
            const_cast<XPropertyList*>(this)->Create();
    }
    if (!isValidIdx(nIndex))
        return nullptr;

    return maList[nIndex].get();
}

long XPropertyList::GetIndex(const OUString& rName) const
{
    if( mbListDirty )
    {
        if( !const_cast<XPropertyList*>(this)->Load() )
            const_cast<XPropertyList*>(this)->Create();
    }

    for( long i = 0, n = maList.size(); i < n; ++i ) {
        if (rName == maList[ i ]->GetName()) {
            return i;
        }
    }
    return -1;
}

BitmapEx XPropertyList::GetUiBitmap( long nIndex ) const
{
    BitmapEx aRetval;
    if (!isValidIdx(nIndex))
        return aRetval;

    XPropertyEntry* pEntry = maList[nIndex].get();
    aRetval = pEntry->GetUiBitmap();

    if(aRetval.IsEmpty())
    {
        aRetval = const_cast< XPropertyList* >(this)->CreateBitmapForUI(nIndex);
        pEntry->SetUiBitmap(aRetval);
    }
    return aRetval;
}

void XPropertyList::Insert(std::unique_ptr<XPropertyEntry> pEntry, long nIndex)
{
    if (!pEntry)
    {
        assert(!"empty XPropertyEntry not allowed in XPropertyList");
        return;
    }

    if (isValidIdx(nIndex)) {
        maList.insert( maList.begin()+nIndex, std::move(pEntry) );
    } else {
        maList.push_back( std::move(pEntry) );
    }
}

void XPropertyList::Replace(std::unique_ptr<XPropertyEntry> pEntry, long nIndex)
{
    if (!pEntry)
    {
        assert(!"empty XPropertyEntry not allowed in XPropertyList");
        return;
    }
    if (!isValidIdx(nIndex))
    {
        assert(!"trying to replace invalid entry in XPropertyList");
        return;
    }

    maList[nIndex] = std::move(pEntry);
}

void XPropertyList::Remove(long nIndex)
{
    if (!isValidIdx(nIndex))
    {
        assert(!"trying to remove invalid entry in XPropertyList");
        return;
    }

    maList.erase(maList.begin() + nIndex);
}

void XPropertyList::SetName( const OUString& rString )
{
    if(!rString.isEmpty())
    {
        maName = rString;
    }
}

bool XPropertyList::Load()
{
    if( mbListDirty )
    {
        mbListDirty = false;
        std::stack<OUString> aDirs;

        sal_Int32 nIndex = 0;
        do
        {
            aDirs.push(maPath.getToken(0, ';', nIndex));
        }
        while (nIndex >= 0);

        //try all entries palette path list working back to front until one
        //succeeds
        while (!aDirs.empty())
        {
            OUString aPath(aDirs.top());
            aDirs.pop();

            INetURLObject aURL(aPath);

            if( INetProtocol::NotValid == aURL.GetProtocol() )
            {
                DBG_ASSERT( aPath.isEmpty(), "invalid URL" );
                return false;
            }

            aURL.Append( maName );

            if( aURL.getExtension().isEmpty() )
                aURL.setExtension( GetDefaultExt() );

            bool bRet = SvxXMLXTableImport::load(aURL.GetMainURL(INetURLObject::DecodeMechanism::NONE),
                                             maReferer, uno::Reference < embed::XStorage >(),
                                             createInstance(), nullptr );
            if (bRet)
                return bRet;
        }
    }
    return false;
}

bool XPropertyList::LoadFrom( const uno::Reference < embed::XStorage > &xStorage,
                              const OUString &rURL, const OUString &rReferer )
{
    if( !mbListDirty )
        return false;
    mbListDirty = false;
    return SvxXMLXTableImport::load( rURL, rReferer, xStorage, createInstance(), &mbEmbedInDocument );
}

bool XPropertyList::Save()
{
    //save to the last path in the palette path list
    OUString aLastDir;
    sal_Int32 nIndex = 0;
    do
    {
        aLastDir = maPath.getToken(0, ';', nIndex);
    }
    while (nIndex >= 0);

    INetURLObject aURL(aLastDir);

    if( INetProtocol::NotValid == aURL.GetProtocol() )
    {
        DBG_ASSERT( aLastDir.isEmpty(), "invalid URL" );
        return false;
    }

    aURL.Append( maName );

    if( aURL.getExtension().isEmpty() )
        aURL.setExtension( GetDefaultExt() );

    return SvxXMLXTableExportComponent::save( aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ),
                                              createInstance(),
                                              uno::Reference< embed::XStorage >(), nullptr );
}

bool XPropertyList::SaveTo( const uno::Reference< embed::XStorage > &xStorage,
                            const OUString &rURL, OUString *pOptName )
{
    return SvxXMLXTableExportComponent::save( rURL, createInstance(), xStorage, pOptName );
}

XPropertyListRef XPropertyList::CreatePropertyList( XPropertyListType aType,
                                                    const OUString& rPath,
                                                    const OUString& rReferer )
{
    XPropertyListRef pRet;

    switch (aType) {
        case XPropertyListType::Color:
            pRet = XPropertyListRef(new XColorList(rPath, rReferer));
            break;
        case XPropertyListType::LineEnd:
            pRet = XPropertyListRef(new XLineEndList(rPath, rReferer));
            break;
        case XPropertyListType::Dash:
            pRet = XPropertyListRef(new XDashList(rPath, rReferer));
            break;
        case XPropertyListType::Hatch:
            pRet = XPropertyListRef(new XHatchList(rPath, rReferer));
            break;
        case XPropertyListType::Gradient:
            pRet = XPropertyListRef(new XGradientList(rPath, rReferer));
            break;
        case XPropertyListType::Bitmap:
            pRet = XPropertyListRef(new XBitmapList(rPath, rReferer));
            break;
        case XPropertyListType::Pattern:
            pRet = XPropertyListRef(new XPatternList(rPath, rReferer));
            break;
    default:
        OSL_FAIL("unknown xproperty type");
        break;
    }
    OSL_ASSERT( !pRet.is() || pRet->meType == aType );

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
        t, aPathURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), "" );
    pList->SetName( aURL.getName() );

    return pList;
}

static struct {
    XPropertyListType const t;
    const char *pExt;
} const pExtnMap[] = {
    { XPropertyListType::Color,    "soc" },
    { XPropertyListType::LineEnd, "soe" },
    { XPropertyListType::Dash,     "sod" },
    { XPropertyListType::Hatch,    "soh" },
    { XPropertyListType::Gradient, "sog" },
    { XPropertyListType::Bitmap,   "sob" },
    { XPropertyListType::Pattern,  "sop"}
};

OUString XPropertyList::GetDefaultExt( XPropertyListType t )
{
    for (const auto & i : pExtnMap)
    {
        if( i.t == t )
            return OUString::createFromAscii( i.pExt );
    }
    return OUString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
