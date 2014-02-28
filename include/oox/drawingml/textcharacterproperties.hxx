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
#include <oox/drawingml/textfont.hxx>

namespace oox { class PropertySet; }

namespace oox {
namespace drawingml {

// ============================================================================

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
    Color               maCharColor;
    Color               maUnderlineColor;
    Color               maHighlightColor;
    OptValue< OUString > moLang;
    OptValue< sal_Int32 > moHeight;
    OptValue< sal_Int32 > moSpacing;
    OptValue< sal_Int32 > moUnderline;
    OptValue< sal_Int32 > moBaseline;
    OptValue< sal_Int32 > moStrikeout;
    OptValue< sal_Int32 > moCaseMap;
    OptValue< bool >    moBold;
    OptValue< bool >    moItalic;
    OptValue< bool >    moUnderlineLineFollowText;
    OptValue< bool >    moUnderlineFillFollowText;

    /** Overwrites all members that are explicitly set in rSourceProps. */
    void                assignUsed( const TextCharacterProperties& rSourceProps );

    /** Returns the current character size. If possible the masterstyle should
        have been applied before, otherwise the character size can be zero and
        the default value is returned. */
    float               getCharHeightPoints( float fDefault ) const;

    /** Writes the properties to the passed property map. */
    void                pushToPropMap(
                            PropertyMap& rPropMap,
                            const ::oox::core::XmlFilterBase& rFilter,
                            bool bUseOptional = false ) const;

    /** Writes the properties to the passed property set. */
    void                pushToPropSet(
                            PropertySet& rPropSet,
                            const ::oox::core::XmlFilterBase& rFilter,
                            bool bUseOptional = false ) const;
};

// ============================================================================

} // namespace drawingml
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
