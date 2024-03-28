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

#ifndef INCLUDED_OOX_DRAWINGML_TEXTCHARACTERPROPERTIES_HXX
#define INCLUDED_OOX_DRAWINGML_TEXTCHARACTERPROPERTIES_HXX

#include <oox/helper/helper.hxx>
#include <oox/helper/propertymap.hxx>
#include <oox/drawingml/color.hxx>
#include <drawingml/textfont.hxx>

#include <drawingml/fillproperties.hxx>
#include <drawingml/lineproperties.hxx>

namespace oox { class PropertySet; }

namespace oox::drawingml {


struct TextCharacterProperties
{
    PropertyMap         maHyperlinkPropertyMap;
    TextFont            maLatinFont;
    TextFont            maLatinThemeFont;
    TextFont            maAsianFont;
    TextFont            maAsianThemeFont;
    TextFont            maComplexFont;
    TextFont            maComplexThemeFont;
    TextFont            maSymbolFont;
    Color               maUnderlineColor;
    Color               maHighlightColor;
    std::optional< OUString > moLang;
    std::optional< sal_Int32 > moHeight;
    /// If a font scale has to be applied manually to moHeight.
    std::optional< double > moFontScale;
    std::optional< sal_Int32 > moSpacing;
    std::optional< sal_Int32 > moUnderline;
    std::optional< sal_Int32 > moBaseline;
    std::optional< sal_Int32 > moStrikeout;
    std::optional< sal_Int32 > moCaseMap;
    std::optional< bool >    moBold;
    std::optional< bool >    moItalic;
    std::optional< bool >    moUnderlineLineFollowText;
    std::optional< bool >    moUnderlineFillFollowText;
    std::optional<LineProperties> moTextOutlineProperties;

    FillProperties      maFillProperties;
    /// Set if there was a property set that alters run visually during import
    bool mbHasVisualRunProperties;

    /// Set if there was an empty paragraph property set during import
    /// <a:pPr><a:defRPr/></a:pPr>
    /// In that case we use the default paragraph properties from the
    /// <c:txPr><a:p><a:pPr><a:defRPr>...</a:defRPr>
    bool mbHasEmptyParaProperties;

    std::vector<css::beans::PropertyValue> maTextEffectsProperties;

    /** Overwrites all members that are explicitly set in rSourceProps. */
    void                assignUsed( const TextCharacterProperties& rSourceProps );

    /** Returns the current character size. If possible the masterstyle should
        have been applied before, otherwise the character size can be zero and
        the default value is returned. */
    float               getCharHeightPoints( float fDefault ) const;

    /** Writes the properties to the passed property map. */
    void                pushToPropMap(
                            PropertyMap& rPropMap,
                            const ::oox::core::XmlFilterBase& rFilter ) const;

    /** Writes the properties to the passed property set. */
    void                pushToPropSet(
                            PropertySet& rPropSet,
                            const ::oox::core::XmlFilterBase& rFilter ) const;

    TextCharacterProperties() : mbHasVisualRunProperties(false), mbHasEmptyParaProperties(false) {}
};


} // namespace oox::drawingml

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
