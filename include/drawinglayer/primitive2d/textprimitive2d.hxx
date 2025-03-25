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
#include <drawinglayer/attribute/fontattribute.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <rtl/ustring.hxx>
#include <tools/color.hxx>
#include <tools/long.hxx>
#include <basegfx/color/bcolor.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <memory>
#include <vector>

namespace drawinglayer::primitive2d
{
class TextLayouterDevice;
}

class SalLayout;

namespace drawinglayer::primitive2d
{
/** TextSimplePortionPrimitive2D class

    This is the basic primitive for representing a text portion. It contains
    all needed information. If it is not handled by a renderer, its decomposition
    will provide the text tools::PolyPolygon outlines as filled polygons, correctly
    transformed.

    To get better text quality, it is suggested to handle this primitive directly
    in a renderer. In that case, e.g. hintings on the system can be supported.

    @param maTextTransform
    The text transformation contains the text start position (always baselined)
    as translation, the FontSize as scale (where width relative to height defines
    font scaling and width == height means no font scaling) and the font rotation
    and shear.
    When shear is used and a renderer does not support it, it may be better to use
    the decomposition which will do everything correctly. Same is true for mirroring
    which would be expressed as negative scalings.

    @param rText
    The text to be used. Only a part may be used, but a bigger part of the string
    may be necessary for correct layouting (e.g. international)

    @param aTextPosition
    The index to the first character to use from rText

    @param aTextLength
    The number of characters to use from rText

    @param rDXArray
    The distances between the characters. This parameter may be empty, in that case
    the renderer is responsible to do something useful. If it is given, it has to be of
    the size aTextLength. Its values are in logical coordinates and describe the
    distance for each character to use. This is independent from the font width which
    is given with maTextTransform. The first value is the offset to use from the start
    point in FontCoordinateSystem X-Direction (given by maTextTransform) to the start
    point of the second character

    @param rKashidaArray
    The Kashida insertion positions.

    @param rFontAttribute
    The font definition

    @param rLocale
    The locale to use

    @param rFontColor
    The font color to use

    @param bFilled

    @param nWidthToFill

    @param rTextFillColor
    Text background color (has nothing to do with bFilled and nWidthToFill)

 */
class DRAWINGLAYER_DLLPUBLIC TextSimplePortionPrimitive2D : public BufferedDecompositionPrimitive2D
{
private:
    /// text transformation (FontCoordinateSystem)
    basegfx::B2DHomMatrix maTextTransform;

    /// The text, used from maTextPosition up to maTextPosition + maTextLength
    OUString maText;

    /// The index from where on maText is used
    sal_Int32 mnTextPosition;

    /// The length for maText usage, starting from maTextPosition
    sal_Int32 mnTextLength;

    /// The DX array in logic units
    std::vector<double> maDXArray;

    /// The Kashida array
    std::vector<sal_Bool> maKashidaArray;

    /// The font definition
    attribute::FontAttribute maFontAttribute;

    /// The Locale for the text
    css::lang::Locale maLocale;

    /// font color
    basegfx::BColor maFontColor;

    /// The fill color of the text
    Color maTextFillColor;

    /// #i96669# internal: add simple range buffering for this primitive
    basegfx::B2DRange maB2DRange;

protected:
    /// local decomposition.
    virtual Primitive2DReference
    create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;

public:
    /// helpers to create a TextLayouterDevice and SalLayout, e.g. needed for SDPRs
    // and decompose. NOTE: the TextLayouterDevice is filled, but should always only
    // be used temporary (do not try to buffer)
    void createTextLayouter(TextLayouterDevice& rTextLayouter) const;
    std::unique_ptr<SalLayout> createSalLayout(const TextLayouterDevice& rTextLayouter) const;

    /// constructor
    TextSimplePortionPrimitive2D(basegfx::B2DHomMatrix aNewTransform, OUString aText,
                                 sal_Int32 nTextPosition, sal_Int32 nTextLength,
                                 std::vector<double>&& rDXArray,
                                 std::vector<sal_Bool>&& rKashidaArray,
                                 attribute::FontAttribute aFontAttribute, css::lang::Locale aLocale,
                                 const basegfx::BColor& rFontColor,
                                 const Color& rTextFillColor = COL_TRANSPARENT);

    /** get text outlines as polygons and their according ObjectTransformation. Handles all
        the necessary VCL outline extractions, scaling adaptations and other stuff.
     */
    void getTextOutlinesAndTransformation(basegfx::B2DPolyPolygonVector& rTarget,
                                          basegfx::B2DHomMatrix& rTransformation) const;

    /// data read access
    const basegfx::B2DHomMatrix& getTextTransform() const { return maTextTransform; }
    const OUString& getText() const { return maText; }
    sal_Int32 getTextPosition() const { return mnTextPosition; }
    sal_Int32 getTextLength() const { return mnTextLength; }
    const ::std::vector<double>& getDXArray() const { return maDXArray; }
    const ::std::vector<sal_Bool>& getKashidaArray() const { return maKashidaArray; }
    const attribute::FontAttribute& getFontAttribute() const { return maFontAttribute; }
    const css::lang::Locale& getLocale() const { return maLocale; }
    const basegfx::BColor& getFontColor() const { return maFontColor; }
    const Color& getTextFillColor() const { return maTextFillColor; }

    /// helpers for determining various decoration states
    virtual bool hasTextRelief() const;
    virtual bool hasShadow() const;
    virtual bool hasTextDecoration() const;
    bool hasOutline() const;

    /// compare operator
    virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

    /// get range
    virtual basegfx::B2DRange
    getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

    /// provide unique ID
    virtual sal_uInt32 getPrimitive2DID() const override;
};

/// small helper to have a compare operator for Locale
bool DRAWINGLAYER_DLLPUBLIC LocalesAreEqual(const css::lang::Locale& rA,
                                            const css::lang::Locale& rB);

} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
