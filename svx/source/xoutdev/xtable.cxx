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
#include <utility>
#include <xmlxtexp.hxx>
#include <xmlxtimp.hxx>
#include <o3tl/safeint.hxx>
#include <osl/diagnose.h>
#include <tools/urlobj.hxx>
#include <svx/xtable.hxx>
#include <tools/debug.hxx>
#include <stack>

using namespace com::sun::star;

XColorEntry::XColorEntry(const Color& rColor, const OUString& rName)
:   XPropertyEntry(rName),
    m_aColor(rColor)
{
}

XLineEndEntry::XLineEndEntry(basegfx::B2DPolyPolygon _aB2DPolyPolygon, const OUString& rName)
:   XPropertyEntry(rName),
    m_aB2DPolyPolygon(std::move(_aB2DPolyPolygon))
{
}

XLineEndEntry::XLineEndEntry(const XLineEndEntry& rOther)
:   XPropertyEntry(rOther),
    m_aB2DPolyPolygon(rOther.m_aB2DPolyPolygon)
{
}

XDashEntry::XDashEntry(const XDash& rDash, const OUString& rName)
:   XPropertyEntry(rName),
    m_aDash(rDash)
{
}

XDashEntry::XDashEntry(const XDashEntry& rOther)
:   XPropertyEntry(rOther),
m_aDash(rOther.m_aDash)
{
}

XHatchEntry::XHatchEntry(const XHatch& rHatch, const OUString& rName)
:   XPropertyEntry(rName),
    m_aHatch(rHatch)
{
}

XHatchEntry::XHatchEntry(const XHatchEntry& rOther)
:   XPropertyEntry(rOther),
    m_aHatch(rOther.m_aHatch)
{
}

XGradientEntry::XGradientEntry(const basegfx::BGradient& rGradient, const OUString& rName)
:   XPropertyEntry(rName),
    m_aGradient(rGradient)
{
}

XGradientEntry::XGradientEntry(const XGradientEntry& rOther)
:   XPropertyEntry(rOther),
    m_aGradient(rOther.m_aGradient)
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
    OUString aPath, OUString aReferer
) : meType           ( type ),
    maName           ( u"standard"_ustr ),
    maPath           (std::move( aPath )),
    maReferer        (std::move( aReferer )),
    mbListDirty      ( true ),
    mbEmbedInDocument( false )
{
//    fprintf (stderr, "Create type %d count %d\n", (int)meType, count++);
}

bool XPropertyList::isValidIdx(tools::Long nIndex) const
{
    return (nIndex >= 0 && o3tl::make_unsigned(nIndex) < maList.size());
}


XPropertyList::~XPropertyList()
{
}

tools::Long XPropertyList::Count() const
{
    if( mbListDirty )
    {
        if( !const_cast<XPropertyList*>(this)->Load() )
            const_cast<XPropertyList*>(this)->Create();
    }
    return maList.size();
}

XPropertyEntry* XPropertyList::Get( tools::Long nIndex ) const
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

tools::Long XPropertyList::GetIndex(std::u16string_view rName) const
{
    if( mbListDirty )
    {
        if( !const_cast<XPropertyList*>(this)->Load() )
            const_cast<XPropertyList*>(this)->Create();
    }

    for( tools::Long i = 0, n = maList.size(); i < n; ++i ) {
        if (rName == maList[ i ]->GetName()) {
            return i;
        }
    }
    return -1;
}

BitmapEx XPropertyList::GetUiBitmap( tools::Long nIndex ) const
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

void XPropertyList::Insert(std::unique_ptr<XPropertyEntry> pEntry, tools::Long nIndex)
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

void XPropertyList::Replace(std::unique_ptr<XPropertyEntry> pEntry, tools::Long nIndex)
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

void XPropertyList::Remove(tools::Long nIndex)
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
                                          std::u16string_view rURLStr )
{
    INetURLObject aURL( rURLStr );
    INetURLObject aPathURL( aURL );

    aPathURL.removeSegment();
    aPathURL.removeFinalSlash();

    XPropertyListRef pList = XPropertyList::CreatePropertyList(
        t, aPathURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), u""_ustr );
    pList->SetName( aURL.getName() );

    return pList;
}

struct {
    XPropertyListType t;
    OUString aExt;
} constexpr pExtnMap[] = {
    { XPropertyListType::Color,    u"soc"_ustr },
    { XPropertyListType::LineEnd, u"soe"_ustr },
    { XPropertyListType::Dash,     u"sod"_ustr },
    { XPropertyListType::Hatch,    u"soh"_ustr },
    { XPropertyListType::Gradient, u"sog"_ustr },
    { XPropertyListType::Bitmap,   u"sob"_ustr },
    { XPropertyListType::Pattern,  u"sop"_ustr}
};

OUString XPropertyList::GetDefaultExt( XPropertyListType t )
{
    for (const auto & i : pExtnMap)
    {
        if( i.t == t )
            return i.aExt;
    }
    return OUString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
