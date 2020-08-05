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

#include "PropertyHelper.hxx"
#include "FastPropertyIdRanges.hxx"
#include "charttoolsdllapi.hxx"
#include <com/sun/star/awt/FontDescriptor.hpp>

#include <vector>

namespace com::sun::star::beans { class XMultiPropertySet; }
namespace com::sun::star::beans { struct Property; }

namespace chart
{

// implements services
//   com.sun.star.style.CharacterProperties
//   com.sun.star.style.CharacterPropertiesAsian
//   com.sun.star.style.CharacterPropertiesComplex
namespace CharacterProperties
{
    // FastProperty Ids for properties
    enum
    {
        // com.sun.star.style.CharacterProperties
        PROP_CHAR_FONT_NAME = FAST_PROPERTY_ID_START_CHAR_PROP, // ?
        PROP_CHAR_FONT_STYLE_NAME,
        PROP_CHAR_FONT_FAMILY,
        PROP_CHAR_FONT_CHAR_SET,
        PROP_CHAR_FONT_PITCH,
        PROP_CHAR_COLOR,
//      PROP_CHAR_BACKGROUND_COLOR,
//      PROP_CHAR_BACKGROUND_TRANSPARENCY, alpha channel in COLOR
        PROP_CHAR_ESCAPEMENT,
        PROP_CHAR_CHAR_HEIGHT,
        PROP_CHAR_UNDERLINE,
        PROP_CHAR_UNDERLINE_COLOR,
        PROP_CHAR_UNDERLINE_HAS_COLOR,
        PROP_CHAR_OVERLINE,
        PROP_CHAR_OVERLINE_COLOR,
        PROP_CHAR_OVERLINE_HAS_COLOR,
        PROP_CHAR_WEIGHT,
        PROP_CHAR_POSTURE,
        PROP_CHAR_AUTO_KERNING,
        PROP_CHAR_KERNING,
//         PROP_CHAR_CASE_MAPPING,
//         PROP_CHAR_ROTATION,
//--         PROP_CHAR_ROTATION_IS_FIT_TO_LINE,
//        PROP_CHAR_SCALE_WIDTH,
        PROP_CHAR_ESCAPEMENT_HEIGHT,

//         PROP_CHAR_CROSSED_OUT,
        PROP_CHAR_STRIKE_OUT,
        PROP_CHAR_WORD_MODE,
//         PROP_CHAR_FLASH,
        PROP_CHAR_LOCALE,
//--         PROP_CHAR_KEEP_TOGETHER,
//--         PROP_CHAR_NO_LINE_BREAK,
        PROP_CHAR_SHADOWED,
        PROP_CHAR_CONTOURED,
        PROP_CHAR_RELIEF,
//--         PROP_CHAR_COMBINE_IS_ON,
//--         PROP_CHAR_COMBINE_PREFIX,
//--         PROP_CHAR_COMBINE_SUFFIX,

        PROP_CHAR_EMPHASIS,
//         PROP_CHAR_RUBY_TEXT,
//         PROP_CHAR_RUBY_ADJUST,
//         PROP_CHAR_RUBY_STYLE_NAME,
//         PROP_CHAR_RUBY_IS_ABOVE,
//         PROP_CHAR_INHIBIT_HYPHENATION,

        // Asian (com.sun.star.style.CharacterPropertiesAsian)
        PROP_CHAR_ASIAN_FONT_NAME,
        PROP_CHAR_ASIAN_FONT_STYLE_NAME,
        PROP_CHAR_ASIAN_FONT_FAMILY,
        PROP_CHAR_ASIAN_CHAR_SET,
        PROP_CHAR_ASIAN_FONT_PITCH,
        PROP_CHAR_ASIAN_CHAR_HEIGHT,
        PROP_CHAR_ASIAN_WEIGHT,
        PROP_CHAR_ASIAN_POSTURE,
        PROP_CHAR_ASIAN_LOCALE,
//--         PROP_CHAR_ASIAN_USE_SCRIPT_TYPE_DISTANCE,
//--         PROP_CHAR_ASIAN_USE_FORBIDDEN_RULES,
//--         PROP_CHAR_ASIAN_HANGING_PUNCTUATION_ALLOWED,

        // Complex Text Layout (com.sun.star.style.CharacterPropertiesComplex)
        PROP_CHAR_COMPLEX_FONT_NAME,
        PROP_CHAR_COMPLEX_FONT_STYLE_NAME,
        PROP_CHAR_COMPLEX_FONT_FAMILY,
        PROP_CHAR_COMPLEX_CHAR_SET,
        PROP_CHAR_COMPLEX_FONT_PITCH,
        PROP_CHAR_COMPLEX_CHAR_HEIGHT,
        PROP_CHAR_COMPLEX_WEIGHT,
        PROP_CHAR_COMPLEX_POSTURE,
        PROP_CHAR_COMPLEX_LOCALE,
        PROP_PARA_IS_CHARACTER_DISTANCE,

        PROP_WRITING_MODE,

        // don't use
        FAST_PROPERTY_ID_END_CHAR_PROP
    };

    OOO_DLLPUBLIC_CHARTTOOLS void AddPropertiesToVector(
        std::vector< css::beans::Property > & rOutProperties );

    OOO_DLLPUBLIC_CHARTTOOLS void AddDefaultsToMap( ::chart::tPropertyValueMap & rOutMap );

    OOO_DLLPUBLIC_CHARTTOOLS bool IsCharacterPropertyHandle( sal_Int32 nHandle );

    OOO_DLLPUBLIC_CHARTTOOLS css::awt::FontDescriptor createFontDescriptorFromPropertySet(
        const css::uno::Reference< css::beans::XMultiPropertySet > & xMultiPropSet );
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
