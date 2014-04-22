/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <fillattributes.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <svx/sdr/primitive2d/sdrdecompositiontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/attribute/fillhatchattribute.hxx>
#include <drawinglayer/attribute/sdrfillgraphicattribute.hxx>
#include <svx/xfillit0.hxx>
#include <vcl/graph.hxx>

//////////////////////////////////////////////////////////////////////////////

void FillAttributes::createPrimitive2DSequence(
    const basegfx::B2DRange& rPaintRange,
    const basegfx::B2DRange& rDefineRange)
{
    // reset and remember new target range for object geometry
    maLastPaintRange = rPaintRange;
    maLastDefineRange = rDefineRange;

    if(isUsed())
    {
        maPrimitives.realloc(1);
        maPrimitives[0] = drawinglayer::primitive2d::createPolyPolygonFillPrimitive(
            basegfx::B2DPolyPolygon(
                basegfx::tools::createPolygonFromRect(
                    maLastPaintRange)),
                maLastDefineRange,
            maFillAttribute.get() ? *maFillAttribute.get() : drawinglayer::attribute::SdrFillAttribute(),
            maFillGradientAttribute.get() ? *maFillGradientAttribute.get() : drawinglayer::attribute::FillGradientAttribute());
    }
}

FillAttributes::FillAttributes()
:   maLastPaintRange(),
    maLastDefineRange(),
    maFillAttribute(),
    maFillGradientAttribute(),
    maPrimitives()
{
}

FillAttributes::FillAttributes(const Color& rColor)
:   maLastPaintRange(),
    maLastDefineRange(),
    maFillAttribute(),
    maFillGradientAttribute(),
    maPrimitives()
{
    maFillAttribute.reset(
        new drawinglayer::attribute::SdrFillAttribute(
            0.0,
            Color(rColor.GetRGBColor()).getBColor(),
            drawinglayer::attribute::FillGradientAttribute(),
            drawinglayer::attribute::FillHatchAttribute(),
            drawinglayer::attribute::SdrFillGraphicAttribute()));
}

FillAttributes::FillAttributes(const SfxItemSet& rSet)
:   maLastPaintRange(),
    maLastDefineRange(),
    maFillAttribute(
        new drawinglayer::attribute::SdrFillAttribute(
            drawinglayer::primitive2d::createNewSdrFillAttribute(rSet))),
    maFillGradientAttribute(
        new drawinglayer::attribute::FillGradientAttribute(
            drawinglayer::primitive2d::createNewTransparenceGradientAttribute(rSet))),
    maPrimitives()
{
}

FillAttributes::~FillAttributes()
{
}

bool FillAttributes::isUsed() const
{
    // only depends on fill, FillGradientAttribute alone defines no fill
    return maFillAttribute.get() && !maFillAttribute->isDefault();
}

bool FillAttributes::isTransparent() const
{
    if(hasSdrFillAttribute() && 0.0 != maFillAttribute->getTransparence())
    {
        return true;
    }

    if(hasFillGradientAttribute() && !maFillGradientAttribute->isDefault())
    {
        return true;
    }

    if(hasSdrFillAttribute())
    {
        const Graphic& rGraphic = getFillAttribute().getFillGraphic().getFillGraphic();

        return rGraphic.IsSupportedGraphic() && rGraphic.IsTransparent();
    }

    return false;
}

const drawinglayer::attribute::SdrFillAttribute& FillAttributes::getFillAttribute() const
{
    if(!maFillAttribute.get())
    {
        const_cast< FillAttributes* >(this)->maFillAttribute.reset(new drawinglayer::attribute::SdrFillAttribute());
    }

    return *maFillAttribute.get();
}

const drawinglayer::attribute::FillGradientAttribute& FillAttributes::getFillGradientAttribute() const
{
    if(!maFillGradientAttribute.get())
    {
        const_cast< FillAttributes* >(this)->maFillGradientAttribute.reset(new drawinglayer::attribute::FillGradientAttribute());
    }

    return *maFillGradientAttribute.get();
}

const drawinglayer::primitive2d::Primitive2DSequence& FillAttributes::getPrimitive2DSequence(
    const basegfx::B2DRange& rPaintRange,
    const basegfx::B2DRange& rDefineRange) const
{
    if(maPrimitives.getLength() && (maLastPaintRange != rPaintRange || maLastDefineRange != rDefineRange))
    {
        const_cast< FillAttributes* >(this)->maPrimitives.realloc(0);
    }

    if(!maPrimitives.getLength())
    {
        const_cast< FillAttributes* >(this)->createPrimitive2DSequence(rPaintRange, rDefineRange);
    }

    return maPrimitives;
}

//////////////////////////////////////////////////////////////////////////////
// eof
