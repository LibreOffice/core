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

#pragma once

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/BufferedDecompositionPrimitive2D.hxx>
#include <drawinglayer/attribute/lineattribute.hxx>
#include <drawinglayer/attribute/strokeattribute.hxx>

namespace drawinglayer::primitive2d
{
/** BorderLine class
    Helper class holding the style definition for a single part of a full BorderLine definition.
    Line extends are for start/end and for Left/Right, seen in vector direction. If
    Left != Right that means the line has a diagonal start/end.
    Think about it similar to a trapezoid, but not aligned to X-Axis and using the
    perpendicular vector to the given one in a right-handed coordinate system.
*/
class DRAWINGLAYER_DLLPUBLIC BorderLine
{
private:
    // line attribute containing Width, Color and others
    drawinglayer::attribute::LineAttribute maLineAttribute;

    // line extends
    double mfStartLeft;
    double mfStartRight;
    double mfEndLeft;
    double mfEndRight;

    // if this is a gap, this is set to true
    bool mbIsGap;

public:
    // Constructor for visible BorderLine segments
    BorderLine(const drawinglayer::attribute::LineAttribute& rLineAttribute,
               double fStartLeft = 0.0, double fStartRight = 0.0, double fEndLeft = 0.0,
               double fEndRight = 0.0);

    // Constructor for gap BorderLine segments
    BorderLine(double fWidth);

    ~BorderLine();

    BorderLine(BorderLine const&) = default;
    BorderLine(BorderLine&&) = default;
    BorderLine& operator=(BorderLine const&) = default;
    BorderLine& operator=(BorderLine&&) = default;

    const drawinglayer::attribute::LineAttribute& getLineAttribute() const
    {
        return maLineAttribute;
    }
    double getStartLeft() const { return mfStartLeft; }
    double getStartRight() const { return mfStartRight; }
    double getEndLeft() const { return mfEndLeft; }
    double getEndRight() const { return mfEndRight; }
    bool isGap() const { return mbIsGap; }

    /// compare operator
    bool operator==(const BorderLine& rBorderLine) const;
};

/** BorderLinePrimitive2D class

    This is the basic primitive to build frames around objects, e.g. tables.
    It defines a single or double line from Start to End using the LeftWidth,
    Distance and RightWidth definitions.
    The LineStart/End overlap is defined in the BorderLines definitions (see
    class BorderLine above).
*/
class DRAWINGLAYER_DLLPUBLIC BorderLinePrimitive2D final : public BufferedDecompositionPrimitive2D
{
private:
    /// the line definition
    basegfx::B2DPoint maStart;
    basegfx::B2DPoint maEnd;

    /// the single BorderLine style definition(s), one or three mostly used
    std::vector<BorderLine> maBorderLines;

    /// common style definitions
    const drawinglayer::attribute::StrokeAttribute maStrokeAttribute;

    /// create local decomposition
    virtual Primitive2DReference
    create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;

    /// helper to get the full width from maBorderLines
    double getFullWidth() const;

public:
    /// simplified constructor for BorderLine with single edge
    BorderLinePrimitive2D(const basegfx::B2DPoint& rStart, const basegfx::B2DPoint& rEnd,
                          std::vector<BorderLine>&& rBorderLines,
                          drawinglayer::attribute::StrokeAttribute aStrokeAttribute);

    /// data read access
    const basegfx::B2DPoint& getStart() const { return maStart; }
    const basegfx::B2DPoint& getEnd() const { return maEnd; }
    const std::vector<BorderLine>& getBorderLines() const { return maBorderLines; }
    const drawinglayer::attribute::StrokeAttribute& getStrokeAttribute() const
    {
        return maStrokeAttribute;
    }

    /// helper to decide if AntiAliasing should be used
    bool isHorizontalOrVertical(const geometry::ViewInformation2D& rViewInformation) const;

    /// compare operator
    virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

    /// provide unique ID
    virtual sal_uInt32 getPrimitive2DID() const override;
};

/// helper to try to merge two instances of BorderLinePrimitive2D. If it was possible,
/// a merged version is in the returned Primitive2DReference. Lots of preconditions
/// have to be met to allow that, see implementation (and maybe even expand)
Primitive2DReference DRAWINGLAYER_DLLPUBLIC tryMergeBorderLinePrimitive2D(
    const BorderLinePrimitive2D* pCandidateA, const BorderLinePrimitive2D* pCandidateB);

} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
