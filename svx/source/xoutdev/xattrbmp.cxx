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

#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <tools/stream.hxx>
#include <vcl/window.hxx>
#include <vcl/virdev.hxx>
#include <vcl/bitmapex.hxx>
#include <svl/style.hxx>
#include <editeng/memberids.h>
#include <svx/strings.hrc>
#include <svx/xattr.hxx>
#include <svx/xtable.hxx>
#include <svx/xdef.hxx>
#include <svx/unomid.hxx>
#include <editeng/unoprnms.hxx>
#include <svx/unoapi.hxx>
#include <svx/svdmodel.hxx>
#include <svx/xbitmap.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <vcl/salbtype.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/BitmapTools.hxx>
#include <vcl/GraphicLoader.hxx>
#include <vcl/dibtools.hxx>

#include <libxml/xmlwriter.h>

using namespace ::com::sun::star;

XOBitmap::XOBitmap( const BitmapEx& rBmp ) :
    xGraphicObject  (new GraphicObject(rBmp)),
    bGraphicDirty   ( false )
{
}

XOBitmap::~XOBitmap()
{
}

BitmapEx XOBitmap::GetBitmap() const
{
    return GetGraphicObject().GetGraphic().GetBitmapEx();
}

const GraphicObject& XOBitmap::GetGraphicObject() const
{
    if( bGraphicDirty )
        const_cast<XOBitmap*>(this)->Array2Bitmap();

    return *xGraphicObject;
}

void XOBitmap::Bitmap2Array()
{
    ScopedVclPtrInstance< VirtualDevice > pVDev;
    bool            bPixelColor = false;
    const BitmapEx  aBitmap( GetBitmap() );
    const sal_Int32 nLines = 8; // type dependent

    if( !pPixelArray )
        pPixelArray.reset( new sal_uInt16[ nLines * nLines ] );

    pVDev->SetOutputSizePixel( aBitmap.GetSizePixel() );
    pVDev->DrawBitmapEx( Point(), aBitmap );
    aPixelColor = aBckgrColor = pVDev->GetPixel( Point() );

    // create array and determine foreground and background color
    for (sal_Int32 i = 0; i < nLines; ++i)
    {
        for (sal_Int32 j = 0; j < nLines; ++j)
        {
            if ( pVDev->GetPixel( Point( j, i ) ) == aBckgrColor )
                pPixelArray[ j + i * nLines ] = 0;
            else
            {
                pPixelArray[ j + i * nLines ] = 1;
                if( !bPixelColor )
                {
                    aPixelColor = pVDev->GetPixel( Point( j, i ) );
                    bPixelColor = true;
                }
            }
        }
    }
}

/// convert array, fore- and background color into a bitmap
void XOBitmap::Array2Bitmap()
{
    if (!pPixelArray)
        return;

    ScopedVclPtrInstance< VirtualDevice > pVDev;
    const sal_Int32 nLines = 8; // type dependent

    pVDev->SetOutputSizePixel( Size( nLines, nLines ) );

    // create bitmap
    for (sal_Int32 i = 0; i < nLines; ++i)
    {
        for (sal_Int32 j = 0; j < nLines; ++j)
        {
            if( pPixelArray[ j + i * nLines ] == 0 )
                pVDev->DrawPixel( Point( j, i ), aBckgrColor );
            else
                pVDev->DrawPixel( Point( j, i ), aPixelColor );
        }
    }

    xGraphicObject.reset(new GraphicObject(pVDev->GetBitmapEx(Point(), Size(nLines, nLines))));
    bGraphicDirty = false;
}


SfxPoolItem* XFillBitmapItem::CreateDefault() { return new XFillBitmapItem; }

XFillBitmapItem::XFillBitmapItem(const OUString& rName, const GraphicObject& rGraphicObject)
:   NameOrIndex(XATTR_FILLBITMAP, rName),
    maGraphicObject(rGraphicObject)
{
}

XFillBitmapItem::XFillBitmapItem(const XFillBitmapItem& rItem)
:   NameOrIndex(rItem),
    maGraphicObject(rItem.maGraphicObject)
{
}

XFillBitmapItem::XFillBitmapItem(const GraphicObject& rGraphicObject)
    : NameOrIndex(XATTR_FILLBITMAP, -1)
    , maGraphicObject(rGraphicObject)
{
}

SfxPoolItem* XFillBitmapItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFillBitmapItem(*this);
}

bool XFillBitmapItem::operator==(const SfxPoolItem& rItem) const
{
    return (NameOrIndex::operator==(rItem)
        && maGraphicObject == static_cast<const XFillBitmapItem&>(rItem).maGraphicObject);
}


bool XFillBitmapItem::isPattern() const
{
    BitmapColor aBack, aFront;
    return vcl::bitmap::isHistorical8x8(GetGraphicObject().GetGraphic().GetBitmapEx(), aBack, aFront);
}

sal_uInt16 XFillBitmapItem::GetVersion(sal_uInt16 /*nFileFormatVersion*/) const
{
    return 2;
}

bool XFillBitmapItem::GetPresentation(
    SfxItemPresentation /*ePres*/,
    MapUnit /*eCoreUnit*/,
    MapUnit /*ePresUnit*/,
    OUString& rText,
    const IntlWrapper&) const
{
    rText += GetName();
    return true;
}

bool XFillBitmapItem::QueryValue(css::uno::Any& rVal, sal_uInt8 nMemberId) const
{
    nMemberId &= ~CONVERT_TWIPS;

    // needed for MID_NAME
    OUString aApiName;
    // needed for complete item (MID 0)
    OUString aInternalName;

    css::uno::Reference< css::awt::XBitmap > xBmp;

    if( nMemberId == MID_NAME )
    {
         aApiName = SvxUnogetApiNameForItem(Which(), GetName());
    }
    else if( nMemberId == 0  )
    {
        aInternalName = GetName();
    }

    if (nMemberId == MID_BITMAP ||
        nMemberId == 0)
    {
        xBmp.set(GetGraphicObject().GetGraphic().GetXGraphic(), uno::UNO_QUERY);
    }

    if( nMemberId == MID_NAME )
        rVal <<= aApiName;
    else if( nMemberId == MID_BITMAP )
        rVal <<= xBmp;
    else
    {
        // member-id 0 => complete item (e.g. for toolbars)
        DBG_ASSERT( nMemberId == 0, "invalid member-id" );
        uno::Sequence< beans::PropertyValue > aPropSeq( 2 );

        aPropSeq[0].Name  = "Name";
        aPropSeq[0].Value <<= aInternalName;
        aPropSeq[1].Name  = "Bitmap";
        aPropSeq[1].Value <<= xBmp;

        rVal <<= aPropSeq;
    }

    return true;
}

bool XFillBitmapItem::PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;

    OUString aName;
    OUString aURL;
    css::uno::Reference< css::awt::XBitmap > xBmp;
    css::uno::Reference< css::graphic::XGraphic > xGraphic;

    bool bSetURL    = false;
    bool bSetName   = false;
    bool bSetBitmap = false;

    if( nMemberId == MID_NAME )
        bSetName = (rVal >>= aName);
    else if( nMemberId == MID_BITMAP )
    {
        if (rVal.has<OUString>())
        {
            bSetURL = true;
            aURL = rVal.get<OUString>();
        }
        else if (rVal.has<uno::Reference<awt::XBitmap>>())
        {
            bSetBitmap = true;
            xBmp = rVal.get<uno::Reference<awt::XBitmap>>();
        }
        else if (rVal.has<uno::Reference<graphic::XGraphic>>())
        {
            bSetBitmap = true;
            xGraphic = rVal.get<uno::Reference<graphic::XGraphic>>();
        }
    }
    else
    {
        DBG_ASSERT( nMemberId == 0, "invalid member-id" );
        uno::Sequence< beans::PropertyValue >   aPropSeq;
        if( rVal >>= aPropSeq )
        {
            for ( sal_Int32 n = 0; n < aPropSeq.getLength(); n++ )
            {
                if ( aPropSeq[n].Name == "Name" )
                    bSetName = (aPropSeq[n].Value >>= aName);
                else if ( aPropSeq[n].Name == "Bitmap" )
                    bSetBitmap = (aPropSeq[n].Value >>= xBmp);
                else if ( aPropSeq[n].Name == "FillBitmapURL" )
                    bSetURL = (aPropSeq[n].Value >>= aURL);
            }
        }
    }

    if( bSetName )
    {
        SetName( aName );
    }
    if (bSetURL && !aURL.isEmpty())
    {
        Graphic aGraphic = vcl::graphic::loadFromURL(aURL);
        if (aGraphic)
        {
            maGraphicObject.SetGraphic(aGraphic.GetXGraphic());
        }
    }
    else if( bSetBitmap )
    {
        if (xBmp.is())
        {
            xGraphic.set(xBmp, uno::UNO_QUERY);
        }
        if (xGraphic.is())
        {
            maGraphicObject.SetGraphic(xGraphic);
        }
    }

    return (bSetName || bSetBitmap);
}

bool XFillBitmapItem::CompareValueFunc( const NameOrIndex* p1, const NameOrIndex* p2 )
{
    const GraphicObject& aGraphicObjectA(static_cast<const XFillBitmapItem*>(p1)->GetGraphicObject());
    const GraphicObject& aGraphicObjectB(static_cast<const XFillBitmapItem*>(p2)->GetGraphicObject());

    return aGraphicObjectA == aGraphicObjectB;
}

std::unique_ptr<XFillBitmapItem> XFillBitmapItem::checkForUniqueItem( SdrModel* pModel ) const
{
    if( pModel )
    {
        XPropertyListType aListType = XPropertyListType::Bitmap;
        if(isPattern())
            aListType = XPropertyListType::Pattern;
        const OUString aUniqueName = NameOrIndex::CheckNamedItem(
                this, XATTR_FILLBITMAP, &pModel->GetItemPool(),
                XFillBitmapItem::CompareValueFunc, RID_SVXSTR_BMP21,
                pModel->GetPropertyList( aListType ) );

        // if the given name is not valid, replace it!
        if( aUniqueName != GetName() )
        {
            return std::make_unique<XFillBitmapItem>(aUniqueName, maGraphicObject);
        }
    }

    return nullptr;
}

void XFillBitmapItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("XFillBitmapItem"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));

    NameOrIndex::dumpAsXml(pWriter);

    xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
