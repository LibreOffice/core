/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef CHART_CHARACTERPROPERTIES_HXX
#define CHART_CHARACTERPROPERTIES_HXX

#include "PropertyHelper.hxx"
#include "FastPropertyIdRanges.hxx"
#include "charttoolsdllapi.hxx"
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>

#include <vector>

namespace chart
{

// implements services
//   com.sun.star.style.CharacterProperties
//   com.sun.star.style.CharacterPropertiesAsian
//   com.sun.star.style.CharacterPropertiesComplex
class OOO_DLLPUBLIC_CHARTTOOLS CharacterProperties
{
public:
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
//         PROP_CHAR_BACKGROUND_COLOR,
//--         PROP_CHAR_BACKGROUND_TRANSPARENCY, alpah channnel in COLOR
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

    static void AddPropertiesToVector(
        ::std::vector< ::com::sun::star::beans::Property > & rOutProperties );

    static void AddDefaultsToMap( ::chart::tPropertyValueMap & rOutMap );

    static bool IsCharacterPropertyHandle( sal_Int32 nHandle );

    static ::com::sun::star::awt::FontDescriptor createFontDescriptorFromPropertySet(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XMultiPropertySet > & xMultiPropSet );

private:
    // not implemented
    CharacterProperties();
};

} //  namespace chart

// CHART_CHARACTERPROPERTIES_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
