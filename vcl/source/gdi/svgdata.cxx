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

#include <tools/stream.hxx>
#include <vcl/svgdata.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/graphic/SvgTools.hpp>
#include <com/sun/star/graphic/Primitive2DTools.hpp>
#include <com/sun/star/rendering/XIntegerReadOnlyBitmap.hpp>
#include <vcl/canvastools.hxx>
#include <comphelper/seqstream.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>

using namespace ::com::sun::star;

BitmapEx convertPrimitive2DSequenceToBitmapEx(
    const css::uno::Sequence< css::uno::Reference< css::graphic::XPrimitive2D > >& rSequence,
    const basegfx::B2DRange& rTargetRange,
    const sal_uInt32 nMaximumQuadraticPixels)
{
    BitmapEx aRetval;

    if(rSequence.hasElements())
    {
        // create replacement graphic from maSequence
        // create XPrimitive2DRenderer
        uno::Reference< uno::XComponentContext > xContext(::comphelper::getProcessComponentContext());

        try
        {
            const uno::Reference< graphic::XPrimitive2DRenderer > xPrimitive2DRenderer = graphic::Primitive2DTools::create(xContext);

            uno::Sequence< beans::PropertyValue > aViewParameters;
            geometry::RealRectangle2D aRealRect;

            aRealRect.X1 = rTargetRange.getMinX();
            aRealRect.Y1 = rTargetRange.getMinY();
            aRealRect.X2 = rTargetRange.getMaxX();
            aRealRect.Y2 = rTargetRange.getMaxY();

            // get system DPI
            const Size aDPI(Application::GetDefaultDevice()->LogicToPixel(Size(1, 1), MAP_INCH));

            const uno::Reference< rendering::XBitmap > xBitmap(
                xPrimitive2DRenderer->rasterize(
                    rSequence,
                    aViewParameters,
                    aDPI.getWidth(),
                    aDPI.getHeight(),
                    aRealRect,
                    nMaximumQuadraticPixels));

            if(xBitmap.is())
            {
                const uno::Reference< rendering::XIntegerReadOnlyBitmap> xIntBmp(xBitmap, uno::UNO_QUERY_THROW);

                if(xIntBmp.is())
                {
                    aRetval = vcl::unotools::bitmapExFromXBitmap(xIntBmp);
                }
            }
        }
        catch(const uno::Exception& e)
        {
            SAL_WARN( "vcl", "Got no graphic::XPrimitive2DRenderer! : " << e.Message);
        }
    }

    return aRetval;
}

void SvgData::ensureReplacement()
{
    ensureSequenceAndRange();

    if(maReplacement.IsEmpty() && maSequence.hasElements())
    {
        maReplacement = convertPrimitive2DSequenceToBitmapEx(maSequence, getRange());
    }
}

void SvgData::ensureSequenceAndRange()
{
    if(!maSequence.hasElements() && maSvgDataArray.hasElements())
    {
        // import SVG to maSequence, also set maRange
        maRange.reset();

        // create stream
        const uno::Reference< io::XInputStream > myInputStream(new comphelper::SequenceInputStream(maSvgDataArray));

        if(myInputStream.is())
        {
            // create SVG interpreter
            uno::Reference< uno::XComponentContext > xContext(::comphelper::getProcessComponentContext());

            try
            {
                const uno::Reference< graphic::XSvgParser > xSvgParser = graphic::SvgTools::create(xContext);

                maSequence = xSvgParser->getDecomposition(myInputStream, maPath);
            }
            catch(const uno::Exception&)
            {
                OSL_ENSURE(false, "Got no graphic::XSvgParser (!)" );
            }
        }

        if(maSequence.hasElements())
        {
            const sal_Int32 nCount(maSequence.getLength());
            geometry::RealRectangle2D aRealRect;
            uno::Sequence< beans::PropertyValue > aViewParameters;

            for(sal_Int32 a(0L); a < nCount; a++)
            {
                // get reference
                const css::uno::Reference< css::graphic::XPrimitive2D > xReference(maSequence[a]);

                if(xReference.is())
                {
                    aRealRect = xReference->getRange(aViewParameters);

                    maRange.expand(
                        basegfx::B2DRange(
                            aRealRect.X1,
                            aRealRect.Y1,
                            aRealRect.X2,
                            aRealRect.Y2));
                }
            }
        }
    }
}

SvgData::SvgData(const SvgDataArray& rSvgDataArray, const OUString& rPath)
:   maSvgDataArray(rSvgDataArray),
    maPath(rPath),
    maRange(),
    maSequence(),
    maReplacement()
{
}

SvgData::SvgData(const OUString& rPath):
    maSvgDataArray(),
    maPath(rPath),
    maRange(),
    maSequence(),
    maReplacement()
{
    SvFileStream rIStm(rPath, STREAM_STD_READ);
    if(rIStm.GetError())
        return;
    const sal_uInt32 nStmLen(rIStm.remainingSize());
    if (nStmLen)
    {
        maSvgDataArray.realloc(nStmLen);
        rIStm.Read(maSvgDataArray.begin(), nStmLen);

        if (rIStm.GetError())
        {
            maSvgDataArray = SvgDataArray();
        }
    }
}

const basegfx::B2DRange& SvgData::getRange() const
{
    const_cast< SvgData* >(this)->ensureSequenceAndRange();

    return maRange;
}

const css::uno::Sequence< css::uno::Reference< css::graphic::XPrimitive2D > >& SvgData::getPrimitive2DSequence() const
{
    const_cast< SvgData* >(this)->ensureSequenceAndRange();

    return maSequence;
}

const BitmapEx& SvgData::getReplacement() const
{
    const_cast< SvgData* >(this)->ensureReplacement();

    return maReplacement;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
