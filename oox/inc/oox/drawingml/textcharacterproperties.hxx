/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef OOX_DRAWINGML_TEXTCHARACTERPROPERTIES_HXX
#define OOX_DRAWINGML_TEXTCHARACTERPROPERTIES_HXX

#include "oox/helper/helper.hxx"
#include "oox/helper/propertymap.hxx"
#include "oox/drawingml/color.hxx"
#include "oox/drawingml/textfont.hxx"

namespace oox { class PropertySet; }

namespace oox {
namespace drawingml {

// ============================================================================

struct TextCharacterProperties
{
    PropertyMap         maHyperlinkPropertyMap;
    TextFont            maLatinFont;
    TextFont            maAsianFont;
    TextFont            maComplexFont;
    TextFont            maSymbolFont;
    Color               maCharColor;
    Color               maUnderlineColor;
    Color               maHighlightColor;
    OptValue< ::rtl::OUString > moLang;
    OptValue< sal_Int32 > moHeight;
    OptValue< sal_Int32 > moSpacing;
    OptValue< sal_Int32 > moUnderline;
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
                            const ::oox::core::XmlFilterBase& rFilter ) const;

    /** Writes the properties to the passed property set. */
    void                pushToPropSet(
                            PropertySet& rPropSet,
                            const ::oox::core::XmlFilterBase& rFilter ) const;
};

// ============================================================================

} // namespace drawingml
} // namespace oox

#endif

