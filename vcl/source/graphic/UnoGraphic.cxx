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

#include <graphic/UnoGraphic.hxx>

#include <tools/stream.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/graphic/GraphicType.hpp>
#include <com/sun/star/graphic/XGraphicTransformer.hpp>
#include <vcl/dibtools.hxx>
#include <vcl/graph.hxx>
#include <vcl/BitmapColor.hxx>
#include <vcl/BitmapDuoToneFilter.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/typeprovider.hxx>

using namespace com::sun::star;

namespace unographic {

Graphic::Graphic()
{
}

Graphic::~Graphic() noexcept
{
}

void Graphic::init(const ::Graphic& rGraphic)
{
    maGraphic = rGraphic;
    unographic::GraphicDescriptor::init(maGraphic);
}

uno::Any SAL_CALL Graphic::queryInterface( const uno::Type & rType )
{
    uno::Any aAny;
    if( rType == cppu::UnoType<graphic::XGraphic>::get())
        aAny <<= uno::Reference< graphic::XGraphic >( this );
    else if( rType == cppu::UnoType<awt::XBitmap>::get())
        aAny <<= uno::Reference< awt::XBitmap >( this );
    else if( rType == cppu::UnoType<graphic::XGraphicTransformer>::get())
        aAny <<= uno::Reference< graphic::XGraphicTransformer >(this);
    else
        aAny = ::unographic::GraphicDescriptor::queryInterface( rType );

    return aAny;
}

void SAL_CALL Graphic::acquire()
    noexcept
{
    unographic::GraphicDescriptor::acquire();
}

void SAL_CALL Graphic::release() noexcept
{
    unographic::GraphicDescriptor::release();
}

OUString SAL_CALL Graphic::getImplementationName()
{
    return u"com.sun.star.comp.graphic.Graphic"_ustr;
}

sal_Bool SAL_CALL Graphic::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService( this, rServiceName );
}

uno::Sequence< OUString > SAL_CALL Graphic::getSupportedServiceNames()
{
    uno::Sequence< OUString >    aRet( ::unographic::GraphicDescriptor::getSupportedServiceNames() );
    const uno::Sequence< OUString >    aNew { u"com.sun.star.graphic.Graphic"_ustr };
    sal_Int32                           nOldCount = aRet.getLength();

    aRet.realloc( nOldCount + aNew.getLength() );

    std::copy(aNew.begin(), aNew.end(), std::next(aRet.getArray(), nOldCount));

    return aRet;
}

uno::Sequence< uno::Type > SAL_CALL Graphic::getTypes()
{
    return cppu::OTypeCollection(
            cppu::UnoType<graphic::XGraphic>::get(),
            cppu::UnoType<awt::XBitmap>::get(),
            ::unographic::GraphicDescriptor::getTypes()
        ).getTypes();
}

uno::Sequence< sal_Int8 > SAL_CALL Graphic::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

sal_Int8 SAL_CALL Graphic::getType()
{
    sal_Int8 cRet = graphic::GraphicType::EMPTY;

    if (!maGraphic.IsNone())
    {
        cRet = (maGraphic.GetType() == ::GraphicType::Bitmap) ? graphic::GraphicType::PIXEL
                                              : graphic::GraphicType::VECTOR;
    }

    return cRet;
}

// XBitmap

awt::Size SAL_CALL Graphic::getSize()
{
    SolarMutexGuard aGuard;

    Size aVclSize;
    if (!maGraphic.IsNone())
    {
        aVclSize = maGraphic.GetSizePixel();
    }
    return awt::Size(aVclSize.Width(), aVclSize.Height());
}

uno::Sequence<sal_Int8> SAL_CALL Graphic::getDIB()
{
    SolarMutexGuard aGuard;

    if (!maGraphic.IsNone())
    {
        SvMemoryStream aMemoryStream;

        WriteDIB(maGraphic.GetBitmapEx().GetBitmap(), aMemoryStream, false, true);
        return css::uno::Sequence<sal_Int8>(static_cast<sal_Int8 const *>(aMemoryStream.GetData()), aMemoryStream.Tell());
    }
    else
    {
        return uno::Sequence<sal_Int8>();
    }
}

uno::Sequence<sal_Int8> SAL_CALL Graphic::getMaskDIB()
{
    SolarMutexGuard aGuard;

    if (!maGraphic.IsNone())
    {
        SvMemoryStream aMemoryStream;

        WriteDIB(maGraphic.GetBitmapEx().GetAlphaMask().GetBitmap(), aMemoryStream, false, true);
        return css::uno::Sequence<sal_Int8>( static_cast<sal_Int8 const *>(aMemoryStream.GetData()), aMemoryStream.Tell() );
    }
    else
    {
        return uno::Sequence<sal_Int8>();
    }
}

// XGraphicTransformer
uno::Reference< graphic::XGraphic > SAL_CALL Graphic::colorChange(
    const uno::Reference< graphic::XGraphic >& rxGraphic, sal_Int32 nColorFrom, sal_Int8 nTolerance, sal_Int32 nColorTo, sal_Int8 nAlphaTo )
{
    ::Graphic aGraphic(rxGraphic);
    ::Graphic aReturnGraphic;

    BitmapColor aBmpColorFrom(Color(ColorTransparency, static_cast<sal_uInt32>(nColorFrom)));
    BitmapColor aBmpColorTo(Color(ColorTransparency, static_cast<sal_uInt32>(nColorTo)));

    Color aColorFrom(aBmpColorFrom);
    Color aColorTo(aBmpColorTo);

    const sal_uInt8 cIndexFrom = aBmpColorFrom.GetIndex();

    //TODO This code convert GdiMetafile(vector graphic) to Bitmap, which cause to information lost
    if (aGraphic.GetType() == GraphicType::Bitmap ||
        aGraphic.GetType() == GraphicType::GdiMetafile)
    {
        BitmapEx aBitmapEx(aGraphic.GetBitmapEx());

        if (aBitmapEx.IsAlpha())
        {
            aBitmapEx.ChangeColorAlpha( cIndexFrom, nAlphaTo );
            aBitmapEx.Replace(aColorFrom, aColorTo, nTolerance);
            aReturnGraphic = ::Graphic(aBitmapEx);
        }
        else
        {
            if ((nAlphaTo == 0) || (nAlphaTo == sal::static_int_cast< sal_Int8 >(0xff)))
            {
                Bitmap aBitmap(aBitmapEx.GetBitmap());
                AlphaMask aMask(aBitmap.CreateAlphaMask(aColorFrom, nTolerance));
                aBitmap.Replace(aColorFrom, aColorTo, nTolerance);
                aReturnGraphic = ::Graphic(BitmapEx(aBitmap, aMask));
            }
            else
            {
                aBitmapEx.Replace(aColorFrom, aColorTo, nTolerance);
                aReturnGraphic = ::Graphic(aBitmapEx);
            }
        }
    }

    aReturnGraphic.setOriginURL(aGraphic.getOriginURL());
    return aReturnGraphic.GetXGraphic();
}

uno::Reference< graphic::XGraphic > SAL_CALL Graphic::applyDuotone(
    const uno::Reference< graphic::XGraphic >& rxGraphic, sal_Int32 nColorOne, sal_Int32 nColorTwo )
{
    ::Graphic aGraphic(rxGraphic);
    ::Graphic aReturnGraphic;

    BitmapEx    aBitmapEx( aGraphic.GetBitmapEx() );
    AlphaMask   aMask( aBitmapEx.GetAlphaMask() );

    BitmapEx    aTmpBmpEx(aBitmapEx.GetBitmap());
    BitmapFilter::Filter(aTmpBmpEx,
                    BitmapDuoToneFilter(
                        Color(ColorTransparency, nColorOne),
                        Color(ColorTransparency, nColorTwo)));

    aReturnGraphic = ::Graphic( BitmapEx( aTmpBmpEx.GetBitmap(), aMask ) );
    aReturnGraphic.setOriginURL(aGraphic.getOriginURL());
    return aReturnGraphic.GetXGraphic();
}

uno::Reference< graphic::XGraphic > SAL_CALL Graphic::applyBrightnessContrast(
    const uno::Reference< graphic::XGraphic >& rxGraphic, sal_Int32 nBrightness, sal_Int32 nContrast, sal_Bool mso )
{
    ::Graphic aGraphic(rxGraphic);
    ::Graphic aReturnGraphic;

    BitmapEx aBitmapEx(aGraphic.GetBitmapEx());
    aBitmapEx.Adjust(nBrightness, nContrast, 0, 0, 0, 0, false, mso);
    aReturnGraphic = ::Graphic(aBitmapEx);
    aReturnGraphic.setOriginURL(aGraphic.getOriginURL());
    return aReturnGraphic.GetXGraphic();
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
