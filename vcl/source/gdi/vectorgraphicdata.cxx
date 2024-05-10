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

#include <comphelper/diagnose_ex.hxx>
#include <tools/stream.hxx>
#include <sal/log.hxx>
#include <utility>
#include <vcl/vectorgraphicdata.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/graphic/PdfTools.hpp>
#include <com/sun/star/graphic/SvgTools.hpp>
#include <com/sun/star/graphic/EmfTools.hpp>
#include <com/sun/star/graphic/Primitive2DTools.hpp>
#include <com/sun/star/rendering/XIntegerReadOnlyBitmap.hpp>
#include <com/sun/star/util/XAccounting.hpp>
#include <com/sun/star/util/XBinaryDataContainer.hpp>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <vcl/canvastools.hxx>
#include <comphelper/seqstream.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/propertyvalue.hxx>
#include <rtl/crc.h>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>
#include <vcl/wmfexternal.hxx>
#include <vcl/pdfread.hxx>
#include <unotools/streamwrap.hxx>
#include <graphic/UnoBinaryDataContainer.hxx>

using namespace ::com::sun::star;

BitmapEx convertPrimitive2DSequenceToBitmapEx(
    const std::deque< css::uno::Reference< css::graphic::XPrimitive2D > >& rSequence,
    const basegfx::B2DRange& rTargetRange,
    const sal_uInt32 nMaximumQuadraticPixels,
    const o3tl::Length eTargetUnit,
    const std::optional<Size>& rTargetDPI)
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

            uno::Sequence< beans::PropertyValue > aViewParameters = {
                comphelper::makePropertyValue(u"RangeUnit"_ustr, static_cast<sal_Int32>(eTargetUnit)),
            };
            geometry::RealRectangle2D aRealRect;

            aRealRect.X1 = rTargetRange.getMinX();
            aRealRect.Y1 = rTargetRange.getMinY();
            aRealRect.X2 = rTargetRange.getMaxX();
            aRealRect.Y2 = rTargetRange.getMaxY();

            // get system DPI
            Size aDPI(Application::GetDefaultDevice()->LogicToPixel(Size(1, 1), MapMode(MapUnit::MapInch)));
            if (rTargetDPI.has_value())
            {
                aDPI = *rTargetDPI;
            }

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
    if (getType() == rCandidate.getType())
    {
        if (maDataContainer.getSize() == rCandidate.maDataContainer.getSize())
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

void VectorGraphicData::ensurePdfReplacement()
{
    assert(getType() == VectorGraphicDataType::Pdf);

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
    if (getType() == VectorGraphicDataType::Pdf)
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
    if (mbSequenceCreated || maDataContainer.isEmpty())
        return;

    // import SVG to maSequence, also set maRange
    maRange.reset();

    // create Vector Graphic Data interpreter
    uno::Reference<uno::XComponentContext> xContext(::comphelper::getProcessComponentContext());

    switch (getType())
    {
        case VectorGraphicDataType::Svg:
        {
            const uno::Reference<io::XInputStream> xInputStream = maDataContainer.getAsXInputStream();

            const uno::Reference< graphic::XSvgParser > xSvgParser = graphic::SvgTools::create(xContext);

            if (xInputStream.is())
                maSequence = comphelper::sequenceToContainer<std::deque<css::uno::Reference< css::graphic::XPrimitive2D >>>(xSvgParser->getDecomposition(xInputStream, OUString()));

            break;
        }
        case VectorGraphicDataType::Emf:
        case VectorGraphicDataType::Wmf:
        {
            const uno::Reference< graphic::XEmfParser > xEmfParser = graphic::EmfTools::create(xContext);

            const uno::Reference<io::XInputStream> xInputStream = maDataContainer.getAsXInputStream();

            if (xInputStream.is())
            {
                uno::Sequence< ::beans::PropertyValue > aPropertySequence;

                // Pass the size hint of the graphic to the EMF parser.
                geometry::RealPoint2D aSizeHint;
                aSizeHint.X = maSizeHint.getX();
                aSizeHint.Y = maSizeHint.getY();
                xEmfParser->setSizeHint(aSizeHint);

                if (!mbEnableEMFPlus)
                {
                    aPropertySequence = { comphelper::makePropertyValue(u"EMFPlusEnable"_ustr, uno::Any(false)) };
                }

                maSequence = comphelper::sequenceToContainer<std::deque<css::uno::Reference< css::graphic::XPrimitive2D >>>(xEmfParser->getDecomposition(xInputStream, OUString(), aPropertySequence));
            }

            break;
        }
        case VectorGraphicDataType::Pdf:
        {
            const uno::Reference<graphic::XPdfDecomposer> xPdfDecomposer = graphic::PdfTools::create(xContext);
            uno::Sequence<beans::PropertyValue> aDecompositionParameters = comphelper::InitPropertySequence({
                {"PageIndex", uno::Any(sal_Int32(mnPageIndex))},
            });

            rtl::Reference<UnoBinaryDataContainer> xDataContainer = new UnoBinaryDataContainer(getBinaryDataContainer());

            auto xPrimitive2D = xPdfDecomposer->getDecomposition(xDataContainer, aDecompositionParameters);
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
    BinaryDataContainer aDataContainer,
    VectorGraphicDataType eVectorDataType,
    sal_Int32 nPageIndex)
:   maDataContainer(std::move(aDataContainer)),
    mbSequenceCreated(false),
    mNestedBitmapSize(0),
    meType(eVectorDataType),
    mnPageIndex(nPageIndex)
{
}

VectorGraphicData::VectorGraphicData(
    const OUString& rPath,
    VectorGraphicDataType eVectorDataType)
:   mbSequenceCreated(false),
    mNestedBitmapSize(0),
    meType(eVectorDataType),
    mnPageIndex(-1)
{
    SvFileStream rIStm(rPath, StreamMode::STD_READ);
    if(rIStm.GetError())
        return;
    const sal_uInt32 nStmLen(rIStm.remainingSize());
    if (nStmLen)
    {
        BinaryDataContainer aData(rIStm, nStmLen);

        if (!rIStm.GetError())
        {
            maDataContainer = std::move(aData);
        }
    }
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
    return rtl_crc32(0, maDataContainer.getData(), maDataContainer.getSize());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
