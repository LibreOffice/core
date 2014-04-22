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

#ifndef FILLATTRIBUTES_HXX
#define FILLATTRIBUTES_HXX

#include <drawinglayer/attribute/fillgradientattribute.hxx>
#include <drawinglayer/attribute/sdrfillattribute.hxx>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <boost/shared_ptr.hpp>
#include <tools/color.hxx>
#include <svl/itemset.hxx>

//////////////////////////////////////////////////////////////////////////////

class FillAttributes
{
private:
    basegfx::B2DRange                                                   maLastPaintRange;
    basegfx::B2DRange                                                   maLastDefineRange;
    boost::shared_ptr< drawinglayer::attribute::SdrFillAttribute >      maFillAttribute;
    boost::shared_ptr< drawinglayer::attribute::FillGradientAttribute > maFillGradientAttribute;
    drawinglayer::primitive2d::Primitive2DSequence                      maPrimitives;

    void createPrimitive2DSequence(
        const basegfx::B2DRange& rPaintRange,
        const basegfx::B2DRange& rDefineRange);

protected:
public:
    FillAttributes();
    FillAttributes(const Color& rColor);
    FillAttributes(const SfxItemSet& rSet);
    ~FillAttributes();

    bool isUsed() const;
    bool hasSdrFillAttribute() const { return maFillAttribute.get(); }
    bool hasFillGradientAttribute() const { return maFillGradientAttribute.get(); }
    bool isTransparent() const;

    const drawinglayer::attribute::SdrFillAttribute& getFillAttribute() const;
    const drawinglayer::attribute::FillGradientAttribute& getFillGradientAttribute() const;
    const drawinglayer::primitive2d::Primitive2DSequence& getPrimitive2DSequence(
        const basegfx::B2DRange& rPaintRange,
        const basegfx::B2DRange& rDefineRange) const;
};

//////////////////////////////////////////////////////////////////////////////

typedef boost::shared_ptr< FillAttributes > FillAttributesPtr;

//////////////////////////////////////////////////////////////////////////////

#endif // FILLATTRIBUTES_HXX

// eof
