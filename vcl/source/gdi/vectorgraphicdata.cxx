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

#include <tools/diagnose_ex.h>
#include <tools/stream.hxx>
#include <sal/log.hxx>
#include <vcl/vectorgraphicdata.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/graphic/PdfTools.hpp>
#include <com/sun/star/graphic/SvgTools.hpp>
#include <com/sun/star/graphic/EmfTools.hpp>
#include <com/sun/star/graphic/Primitive2DTools.hpp>
#include <com/sun/star/rendering/XIntegerReadOnlyBitmap.hpp>
#include <com/sun/star/util/XAccounting.hpp>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <vcl/canvastools.hxx>
#include <comphelper/seqstream.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/propertyvalue.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>
#include <vcl/wmfexternal.hxx>
#include <vcl/pdfread.hxx>
#include <unotools/streamwrap.hxx>

using namespace ::com::sun::star;

BitmapEx convertPrimitive2DSequenceToBitmapEx(
    const std::deque< css::uno::Reference< css::graphic::XPrimitive2D > >& rSequence,
    const basegfx::B2DRange& rTargetRange,
    const sal_uInt32 nMaximumQuadraticPixels)
{
    BitmapEx aRetval;

    if(!rSequence.empty())
    {
        // create replacement graphic from maSequence
        // create XPrimitive2DRenderer
        try
        {
            uno::Reference< uno::XComponentContext > xContext(::comphelper::getProcessComponentContext());
            const uno::Reference< graphic::XPrimitive2DRenderer > xPrimitive2DRenderer = graphic::Primitive2DTools::create(xContext);

            uno::Sequence< beans::PropertyValue > aViewParameters;
            geometry::RealRectangle2D aRealRect;

            aRealRect.X1 = rTargetRange.getMinX();
            aRealRect.Y1 = rTargetRange.getMinY();
            aRealRect.X2 = rTargetRange.getMaxX();
            aRealRect.Y2 = rTargetRange.getMaxY();

            // get system DPI
            const Size aDPI(Application::GetDefaultDevice()->LogicToPixel(Size(1, 1), MapMode(MapUnit::MapInch)));

            const uno::Reference< rendering::XBitmap > xBitmap(
                xPrimitive2DRenderer->rasterize(
                    comphelper::containerToSequence(rSequence),
                    aViewParameters,
                    aDPI.getWidth(),
                    aDPI.getHeight(),
                    aRealRect,
                    nMaximumQuadraticPixels));

            if(xBitmap.is())
            {
                const uno::Reference< rendering::XIntegerReadOnlyBitmap> xIntBmp(xBitmap, uno::UNO_QUERY_THROW);
                aRetval = vcl::unotools::bitmapExFromXBitmap(xIntBmp);
            }
        }
        catch (const uno::Exception&)
        {
            TOOLS_WARN_EXCEPTION("vcl", "Got no graphic::XPrimitive2DRenderer!");
        }
        catch (const std::exception& e)
        {
            SAL_WARN("vcl", "Got no graphic::XPrimitive2DRenderer! : " << e.what());
        }
    }

    return aRetval;
}

static size_t estimateSize(
    std::deque<uno::Reference<graphic::XPrimitive2D>> const& rSequence)
{
    size_t nRet(0);
    for (auto& it : rSequence)
    {
        uno::Reference<util::XAccounting> const xAcc(it, uno::UNO_QUERY);
        assert(xAcc.is()); // we expect only BasePrimitive2D from SVG parser
        nRet += xAcc->estimateUsage();
    }
    return nRet;
}

bool VectorGraphicData::operator==(const VectorGraphicData& rCandidate) const
{
    if (getVectorGraphicDataType() == rCandidate.getVectorGraphicDataType())
    {
        if (getVectorGraphicDataArrayLength() == rCandidate.getVectorGraphicDataArrayLength())
        {
            if (0 == memcmp(
                maDataContainer.getData(),
                rCandidate.maDataContainer.getData(),
                maDataContainer.getSize()))
            {
                return true;
            }
        }
    }

    return false;
}

void VectorGraphicData::setWmfExternalHeader(const WmfExternal& aExtHeader)
{
    if (!mpExternalHeader)
    {
        mpExternalHeader.reset( new WmfExternal );
    }

    *mpExternalHeader = aExtHeader;
}

void VectorGraphicData::ensurePdfReplacement()
{
    assert(getVectorGraphicDataType() == VectorGraphicDataType::Pdf);

    if (!maReplacement.IsEmpty())
        return; // nothing to do

    // use PDFium directly
    std::vector<BitmapEx> aBitmaps;
    sal_Int32 nUsePageIndex = 0;
    if (mnPageIndex >= 0)
        nUsePageIndex = mnPageIndex;
    vcl::RenderPDFBitmaps(maDataContainer.getData(),
                          maDataContainer.getSize(), aBitmaps, nUsePageIndex, 1,
                          &maSizeHint);
    if (!aBitmaps.empty())
        maReplacement = aBitmaps[0];
}

void VectorGraphicData::ensureReplacement()
{
    if (!maReplacement.IsEmpty())
        return; // nothing to do

    // shortcut for PDF - PDFium can generate the replacement bitmap for us
    // directly
    if (getVectorGraphicDataType() == VectorGraphicDataType::Pdf)
    {
        ensurePdfReplacement();
        return;
    }

    ensureSequenceAndRange();

    if (!maSequence.empty())
    {
        maReplacement = convertPrimitive2DSequenceToBitmapEx(maSequence, getRange());
    }
}

void VectorGraphicData::ensureSequenceAndRange()
{
    printf ("VectorGraphicData::ensureSequenceAndRange\n");

    if (mbSequenceCreated || maDataContainer.isEmpty())
        return;

    // import SVG to maSequence, also set maRange
    maRange.reset();

    // create Vector Graphic Data interpreter
    uno::Reference<uno::XComponentContext> xContext(::comphelper::getProcessComponentContext());

    switch (getVectorGraphicDataType())
    {
        case VectorGraphicDataType::Svg:
        {
            css::uno::Sequence<sal_Int8> aDataSequence(maDataContainer.getSize());
            std::copy(maDataContainer.cbegin(), maDataContainer.cend(), aDataSequence.begin());
            const uno::Reference<io::XInputStream> xInputStream(new comphelper::SequenceInputStream(aDataSequence));


            const uno::Reference< graphic::XSvgParser > xSvgParser = graphic::SvgTools::create(xContext);

            if (xInputStream.is())
                maSequence = comphelper::sequenceToContainer<std::deque<css::uno::Reference< css::graphic::XPrimitive2D >>>(xSvgParser->getDecomposition(xInputStream, OUString()));

            break;
        }
        case VectorGraphicDataType::Emf:
        case VectorGraphicDataType::Wmf:
        {
            const uno::Reference< graphic::XEmfParser > xEmfParser = graphic::EmfTools::create(xContext);

            css::uno::Sequence<sal_Int8> aDataSequence(maDataContainer.getSize());
            std::copy(maDataContainer.cbegin(), maDataContainer.cend(), aDataSequence.begin());
            const uno::Reference<io::XInputStream> xInputStream(new comphelper::SequenceInputStream(aDataSequence));

            uno::Sequence< ::beans::PropertyValue > aSequence;

            if (mpExternalHeader)
            {
                aSequence = mpExternalHeader->getSequence();
            }

            if (xInputStream.is())
            {
                // Pass the size hint of the graphic to the EMF parser.
                geometry::RealPoint2D aSizeHint;
                aSizeHint.X = maSizeHint.getX();
                aSizeHint.Y = maSizeHint.getY();
                xEmfParser->setSizeHint(aSizeHint);

                if (!mbEnableEMFPlus)
                {
                    auto aVector = comphelper::sequenceToContainer<std::vector<beans::PropertyValue>>(aSequence);
                    aVector.push_back(comphelper::makePropertyValue("EMFPlusEnable", uno::makeAny(false)));
                    aSequence = comphelper::containerToSequence(aVector);
                }

                maSequence = comphelper::sequenceToContainer<std::deque<css::uno::Reference< css::graphic::XPrimitive2D >>>(xEmfParser->getDecomposition(xInputStream, OUString(), aSequence));
            }

            break;
        }
        case VectorGraphicDataType::Pdf:
        {
            const uno::Reference<graphic::XPdfDecomposer> xPdfDecomposer = graphic::PdfTools::create(xContext);
            uno::Sequence<beans::PropertyValue> aDecompositionParameters = comphelper::InitPropertySequence({
                {"PageIndex", uno::makeAny<sal_Int32>(mnPageIndex)},
            });
            // TODO: change xPdfDecomposer to use BinaryDataContainer directly
            css::uno::Sequence<sal_Int8> aDataSequence(maDataContainer.getSize());
            std::copy(maDataContainer.cbegin(), maDataContainer.cend(), aDataSequence.begin());
            auto xPrimitive2D = xPdfDecomposer->getDecomposition(aDataSequence, aDecompositionParameters);
            maSequence = comphelper::sequenceToContainer<std::deque<uno::Reference<graphic::XPrimitive2D>>>(xPrimitive2D);

            break;
        }
    }

    if(!maSequence.empty())
    {
        const sal_Int32 nCount(maSequence.size());
        geometry::RealRectangle2D aRealRect;
        uno::Sequence< beans::PropertyValue > aViewParameters;

        for(sal_Int32 a(0); a < nCount; a++)
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
    mNestedBitmapSize = estimateSize(maSequence);
    mbSequenceCreated = true;
}

std::pair<VectorGraphicData::State, size_t> VectorGraphicData::getSizeBytes() const
{
    if (!maSequence.empty() && !maDataContainer.isEmpty())
    {
        return std::make_pair(State::PARSED, maDataContainer.getSize() + mNestedBitmapSize);
    }
    else
    {
        return std::make_pair(State::UNPARSED, maDataContainer.getSize());
    }
}

VectorGraphicData::VectorGraphicData(
    const BinaryDataContainer& rDataContainer,
    VectorGraphicDataType eVectorDataType,
    sal_Int32 nPageIndex)
:   maDataContainer(rDataContainer),
    mbSequenceCreated(false),
    maRange(),
    maSequence(),
    maReplacement(),
    mNestedBitmapSize(0),
    meVectorGraphicDataType(eVectorDataType),
    mnPageIndex(nPageIndex)
{
}

VectorGraphicData::~VectorGraphicData()
{
}

const basegfx::B2DRange& VectorGraphicData::getRange() const
{
    const_cast< VectorGraphicData* >(this)->ensureSequenceAndRange();

    return maRange;
}

const std::deque< css::uno::Reference< css::graphic::XPrimitive2D > >& VectorGraphicData::getPrimitive2DSequence() const
{
    const_cast< VectorGraphicData* >(this)->ensureSequenceAndRange();

    return maSequence;
}

const BitmapEx& VectorGraphicData::getReplacement() const
{
    const_cast< VectorGraphicData* >(this)->ensureReplacement();

    return maReplacement;
}

BitmapChecksum VectorGraphicData::GetChecksum() const
{
    return vcl_get_checksum(0, maDataContainer.getData(), maDataContainer.getSize());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
