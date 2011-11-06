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

