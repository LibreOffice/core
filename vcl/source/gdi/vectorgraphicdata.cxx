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
#include <sal/log.hxx>
#include <vcl/vectorgraphicdata.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/graphic/SvgTools.hpp>
#include <com/sun/star/graphic/EmfTools.hpp>
#include <com/sun/star/graphic/Primitive2DTools.hpp>
#include <com/sun/star/rendering/XIntegerReadOnlyBitmap.hpp>
#include <com/sun/star/util/XAccounting.hpp>
#include <vcl/canvastools.hxx>
#include <comphelper/seqstream.hxx>
#include <comphelper/sequence.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>
#include <vcl/wmfexternal.hxx>

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
        catch (const uno::Exception& e)
        {
            SAL_WARN("vcl", "Got no graphic::XPrimitive2DRenderer! : " << e);
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
                getVectorGraphicDataArray().getConstArray(),
                rCandidate.getVectorGraphicDataArray().getConstArray(),
                getVectorGraphicDataArrayLength()))
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

void VectorGraphicData::ensureReplacement()
{
    ensureSequenceAndRange();

    if(maReplacement.IsEmpty() && !maSequence.empty())
    {
        maReplacement = convertPrimitive2DSequenceToBitmapEx(maSequence, getRange());
    }
}

void VectorGraphicData::ensureSequenceAndRange()
{
    if (!mbSequenceCreated && maVectorGraphicDataArray.hasElements())
    {
        // import SVG to maSequence, also set maRange
        maRange.reset();

        // create stream
        const uno::Reference< io::XInputStream > myInputStream(new comphelper::SequenceInputStream(maVectorGraphicDataArray));

        if(myInputStream.is())
        {
            // create Vector Graphic Data interpreter
            uno::Reference<uno::XComponentContext> xContext(::comphelper::getProcessComponentContext());

            if (VectorGraphicDataType::Emf == getVectorGraphicDataType()
                || VectorGraphicDataType::Wmf == getVectorGraphicDataType())
            {
                const uno::Reference< graphic::XEmfParser > xEmfParser = graphic::EmfTools::create(xContext);
                uno::Sequence< ::beans::PropertyValue > aSequence;

                if (mpExternalHeader)
                {
                    aSequence = mpExternalHeader->getSequence();
                }

                maSequence = comphelper::sequenceToContainer<std::deque<css::uno::Reference< css::graphic::XPrimitive2D >>>(xEmfParser->getDecomposition(myInputStream, maPath, aSequence));
            }
            else
            {
                const uno::Reference< graphic::XSvgParser > xSvgParser = graphic::SvgTools::create(xContext);

                maSequence = comphelper::sequenceToContainer<std::deque<css::uno::Reference< css::graphic::XPrimitive2D >>>(xSvgParser->getDecomposition(myInputStream, maPath));
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
}

auto VectorGraphicData::getSizeBytes() -> std::pair<State, size_t>
{
    if (maSequence.empty() && maVectorGraphicDataArray.hasElements())
    {
        return std::make_pair(State::UNPARSED, maVectorGraphicDataArray.getLength());
    }
    else
    {
        return std::make_pair(State::PARSED, maVectorGraphicDataArray.getLength() + mNestedBitmapSize);
    }
}

VectorGraphicData::VectorGraphicData(
    const VectorGraphicDataArray& rVectorGraphicDataArray,
    const OUString& rPath,
    VectorGraphicDataType eVectorDataType)
:   maVectorGraphicDataArray(rVectorGraphicDataArray),
    maPath(rPath),
    mbSequenceCreated(false),
    maRange(),
    maSequence(),
    maReplacement(),
    mNestedBitmapSize(0),
    meVectorGraphicDataType(eVectorDataType)
{
}

VectorGraphicData::VectorGraphicData(
    const OUString& rPath,
    VectorGraphicDataType eVectorDataType)
:   maVectorGraphicDataArray(),
    maPath(rPath),
    mbSequenceCreated(false),
    maRange(),
    maSequence(),
    maReplacement(),
    mNestedBitmapSize(0),
    meVectorGraphicDataType(eVectorDataType)
{
    SvFileStream rIStm(rPath, StreamMode::STD_READ);
    if(rIStm.GetError())
        return;
    const sal_uInt32 nStmLen(rIStm.remainingSize());
    if (nStmLen)
    {
        maVectorGraphicDataArray.realloc(nStmLen);
        rIStm.ReadBytes(maVectorGraphicDataArray.begin(), nStmLen);

        if (rIStm.GetError())
        {
            maVectorGraphicDataArray = VectorGraphicDataArray();
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
