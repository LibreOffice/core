/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CharacterProperties.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:37:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef CHART_CHARACTERPROPERTIES_HXX
#define CHART_CHARACTERPROPERTIES_HXX

#include "PropertyHelper.hxx"
#include "FastPropertyIdRanges.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif

#include <vector>

namespace chart
{

// implements services
//   com.sun.star.style.CharacterProperties
//   com.sun.star.style.CharacterPropertiesAsian
//   com.sun.star.style.CharacterPropertiesComplex
class CharacterProperties
{
public:
    // FastProperty Ids for properties
    enum
    {
        PROP_CHAR_STYLE = FAST_PROPERTY_ID_START_CHAR_PROP, // ?

        // com.sun.star.style.CharacterProperties
        PROP_CHAR_FONT_NAME,
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
        PROP_CHAR_WEIGHT,
        PROP_CHAR_POSTURE,
        PROP_CHAR_AUTO_KERNING,
        PROP_CHAR_KERNING,
//         PROP_CHAR_CASE_MAPPING,
//         PROP_CHAR_ROTATION,
//--         PROP_CHAR_ROTATION_IS_FIT_TO_LINE,
        PROP_CHAR_SCALE_WIDTH,
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

        // don't use
        FAST_PROPERTY_ID_END_CHAR_PROP
    };

    static void AddPropertiesToVector(
        ::std::vector< ::com::sun::star::beans::Property > & rOutProperties,
        bool bIncludeStyleProperties = false );

    static void AddDefaultsToMap( ::chart::helper::tPropertyValueMap & rOutMap, bool bIncludeStyleProperties = false );

private:
    // not implemented
    CharacterProperties();
};

} //  namespace chart

// CHART_CHARACTERPROPERTIES_HXX
#endif
