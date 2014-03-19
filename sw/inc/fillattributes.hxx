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

#ifndef FILLATTRIBUTES_HXX
#define FILLATTRIBUTES_HXX

#include <drawinglayer/attribute/fillgradientattribute.hxx>
#include <drawinglayer/attribute/sdrfillattribute.hxx>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <boost/shared_ptr.hpp>

//////////////////////////////////////////////////////////////////////////////

class Color;
class SfxItemSet;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
