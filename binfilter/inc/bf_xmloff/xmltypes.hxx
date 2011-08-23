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


#ifndef _XMLOFF_XMLTYPES_HXX
#define _XMLOFF_XMLTYPES_HXX
namespace binfilter {
///////////////////////////////////////////////////////////////////////////////
// Flags to specify how to im/export the property
//
#define MID_FLAG_MASK					0x0000ffff

// Import only: the xml attribute's value is evaluated by a call to 
// handleSpecialItem instead of calling the property handler's importXML method
#define MID_FLAG_SPECIAL_ITEM_IMPORT	0x80000000

// Export only: the property's value is exported by a call to handleSpecialItem
// instead of calling the property handler's exportXML method
#define MID_FLAG_SPECIAL_ITEM_EXPORT	0x20000000
#define MID_FLAG_SPECIAL_ITEM			0xa0000000	// both import and export

// the map entry denotes the name of an element item
#define MID_FLAG_ELEMENT_ITEM_IMPORT	0x08000000
#define MID_FLAG_ELEMENT_ITEM_EXPORT	0x04000000
#define MID_FLAG_ELEMENT_ITEM			0x0c000000  // both import and export

// the attribute is ignored (not yet implemented)
#define MID_FLAG_NO_PROPERTY_IMPORT		0x40000000

// the property is ignored
#define MID_FLAG_NO_PROPERTY_EXPORT		0x10000000
#define MID_FLAG_NO_PROPERTY			0x50000000	// both import and export

// Import only: there are more entries for the same xml attribute existing
#define MID_FLAG_MULTI_PROPERTY			0x02000000	// 

//#define MID_FLAG_MULTI_ATTRIBUTE		0x01000000	// there are other attributes with same stare one property following

// Import only: If a property with the same name has been set already, supply
// the current value to the importXML call.
#define MID_FLAG_MERGE_PROPERTY			0x00800000

// Export only: If an xml attribute with the same name has been exported
// already, supply its value to teh exportXML call and delete the existing
// atribute afterwards.
#define MID_FLAG_MERGE_ATTRIBUTE		0x00400000

// Import and Export: The property in question must exist. No call to
// hasPropertyByName is required.
#define MID_FLAG_MUST_EXIST				0x00200000

// Export only: When exporting defaults, export this property even if it is
// not set
#define MID_FLAG_DEFAULT_ITEM_EXPORT	0x00100000

// Import only: In general, properties may not throw exception, but
// this one may. If it throws an IllegalArgumentException, it will be
// silently ignored. This should be used for properties whose values
// cannot be checked. (E.g., the printer paper tray: The tray names
// are different for all printers, and we don't know ahead whether a
// printer supports the trays in the current document.)
#define MID_FLAG_PROPERTY_MAY_EXCEPT    0x00010000

///////////////////////////////////////////////////////////////////////////////
//
// XML-data-type-ID's
//

// simple types, no special compare necessary
#define XML_TYPE_BUILDIN_CMP 0x00008000

// basic types (all also XML_TYPE_BUILDIN_CMP)
#define XML_TYPE_BOOL		 		0x00008001			// true/false
#define XML_TYPE_MEASURE	 		0x00008002			// 1cm
#define XML_TYPE_MEASURE8	 		0x00008003			// 1cm
#define XML_TYPE_MEASURE16	 		0x00008004			// 1cm
#define XML_TYPE_PERCENT	 		0x00008005			// 50%
#define XML_TYPE_PERCENT8	 		0x00008006			// 50%
#define XML_TYPE_PERCENT16	 		0x00008007			// 50%
#define XML_TYPE_STRING		 		0x00008008			// "blablabla"
#define XML_TYPE_COLOR		 		0x00008009			// Colors
#define XML_TYPE_NUMBER		 		0x0000800a			// 123
#define XML_TYPE_NUMBER8	 		0x0000800b			// 123
#define XML_TYPE_NUMBER16	 		0x0000800c			// 123
#define XML_TYPE_NUMBER_NONE 		0x0000800d			// 123 and "no-limit" for 0
#define XML_TYPE_DOUBLE		 		0x0000800e			// for doubles
#define XML_TYPE_NBOOL		 		0x0000800f			// !true/false
#define XML_TYPE_COLORTRANSPARENT 	0x00008010			// Colors or "transparent"
#define XML_TYPE_ISTRANSPARENT		0x00008011			// "transparent"
#define XML_TYPE_BUILDIN_CMP_ONLY	0x00008012			// Only buildin comparison is required, so no handler exists
#define XML_TYPE_NUMBER8_NONE 		0x00008013			// 123 and "no-limit" for 0
#define XML_TYPE_NUMBER16_NONE 		0x00008014			// 123 and "no-limit" for 0
#define XML_TYPE_COLOR_MODE 		0x00008015
#define XML_TYPE_DURATION16_MS		0x00008016			// PT00H00M01S to ms in sal_Int16
#define XML_TYPE_MEASURE_PX			0x00008017				// 1px
#define XML_TYPE_COLORAUTO			0x00008018			// color if not -1
#define XML_TYPE_ISAUTOCOLOR		0x00008019			// true if -1

// special basic types
#define XML_TYPE_RECTANGLE_LEFT		0x00000100			// the Left member of a awt::Rectangle as a measure
#define XML_TYPE_RECTANGLE_TOP		0x00000101			// the Top member of a awt::Rectangle as a measure
#define XML_TYPE_RECTANGLE_WIDTH	0x00000102			// the Width member of a awt::Rectangle as a measure
#define XML_TYPE_RECTANGLE_HEIGHT	0x00000103			// the Height member of a awt::Rectangle as a measure

#define XML_SC_TYPES_START	0x00001000
#define XML_SD_TYPES_START	0x00002000

#define XML_TEXT_TYPES_START	0x00003000
#define XML_TYPE_TEXT_CROSSEDOUT	(XML_TEXT_TYPES_START +   0)
#define XML_TYPE_TEXT_CASEMAP		(XML_TEXT_TYPES_START +   1)
#define XML_TYPE_TEXT_FONTFAMILYNAME (XML_TEXT_TYPES_START +   2)
#define XML_TYPE_TEXT_FONTFAMILY	(XML_TEXT_TYPES_START +   3)
#define XML_TYPE_TEXT_FONTENCODING	(XML_TEXT_TYPES_START +   4)
#define XML_TYPE_TEXT_FONTPITCH		(XML_TEXT_TYPES_START +   5)
#define XML_TYPE_TEXT_KERNING		(XML_TEXT_TYPES_START +   6)
#define XML_TYPE_TEXT_POSTURE		(XML_TEXT_TYPES_START +   7)
#define XML_TYPE_TEXT_SHADOWED		(XML_TEXT_TYPES_START +   8)
#define XML_TYPE_TEXT_UNDERLINE		(XML_TEXT_TYPES_START +   9)
#define XML_TYPE_TEXT_WEIGHT		(XML_TEXT_TYPES_START +  10)
#define XML_TYPE_TEXT_SPLIT			(XML_TEXT_TYPES_START +  11)
#define XML_TYPE_TEXT_BREAKBEFORE	(XML_TEXT_TYPES_START +  12)
#define XML_TYPE_TEXT_BREAKAFTER	(XML_TEXT_TYPES_START +  13)
#define XML_TYPE_TEXT_SHADOW		(XML_TEXT_TYPES_START +  14)
#define XML_TYPE_TEXT_ADJUST		(XML_TEXT_TYPES_START +  15)
#define XML_TYPE_TEXT_ADJUSTLAST	(XML_TEXT_TYPES_START +  16)
#define XML_TYPE_TEXT_CASEMAP_VAR	(XML_TEXT_TYPES_START +  17)
#define XML_TYPE_TEXT_ESCAPEMENT	(XML_TEXT_TYPES_START +  18)
#define XML_TYPE_TEXT_ESCAPEMENT_HEIGHT (XML_TEXT_TYPES_START + 19)
#define XML_TYPE_CHAR_HEIGHT		(XML_TEXT_TYPES_START +  20)
#define XML_TYPE_CHAR_HEIGHT_PROP	(XML_TEXT_TYPES_START +  21)
#define XML_TYPE_CHAR_LANGUAGE		(XML_TEXT_TYPES_START +  22)
#define XML_TYPE_CHAR_COUNTRY		(XML_TEXT_TYPES_START +  23)
#define XML_TYPE_LINE_SPACE_FIXED	(XML_TEXT_TYPES_START +  24)
#define XML_TYPE_LINE_SPACE_MINIMUM	(XML_TEXT_TYPES_START +  25)
#define XML_TYPE_LINE_SPACE_DISTANCE (XML_TEXT_TYPES_START +  26)
#define XML_TYPE_BORDER				(XML_TEXT_TYPES_START + 27)
#define XML_TYPE_BORDER_WIDTH		(XML_TEXT_TYPES_START + 28)
#define XML_TYPE_TEXT_DROPCAP		(XML_TEXT_TYPES_START + 29)
#define XML_TYPE_TEXT_TABSTOP		(XML_TEXT_TYPES_START + 30)
#define XML_TYPE_TEXT_BOOLCROSSEDOUT (XML_TEXT_TYPES_START + 31)
#define XML_TYPE_TEXT_WRAP			(XML_TEXT_TYPES_START + 32)
#define XML_TYPE_TEXT_PARAGRAPH_ONLY (XML_TEXT_TYPES_START + 33)
#define XML_TYPE_TEXT_WRAP_OUTSIDE	(XML_TEXT_TYPES_START + 34)
#define XML_TYPE_TEXT_OPAQUE		(XML_TEXT_TYPES_START + 35)
#define XML_TYPE_TEXT_PROTECT_CONTENT	(XML_TEXT_TYPES_START + 36)
#define XML_TYPE_TEXT_PROTECT_SIZE		(XML_TEXT_TYPES_START + 37)
#define XML_TYPE_TEXT_PROTECT_POSITION	(XML_TEXT_TYPES_START + 38)
#define XML_TYPE_TEXT_ANCHOR_TYPE		(XML_TEXT_TYPES_START + 39)
#define XML_TYPE_TEXT_COLUMNS		(XML_TEXT_TYPES_START + 40)
#define XML_TYPE_TEXT_HORIZONTAL_POS	(XML_TEXT_TYPES_START + 41)
#define XML_TYPE_TEXT_HORIZONTAL_POS_MIRRORED	(XML_TEXT_TYPES_START + 42)
#define XML_TYPE_TEXT_HORIZONTAL_REL	(XML_TEXT_TYPES_START + 43)
#define XML_TYPE_TEXT_HORIZONTAL_REL_FRAME	(XML_TEXT_TYPES_START + 44)
#define XML_TYPE_TEXT_HORIZONTAL_MIRROR	(XML_TEXT_TYPES_START + 45)
#define XML_TYPE_TEXT_VERTICAL_POS	(XML_TEXT_TYPES_START + 46)
#define XML_TYPE_TEXT_VERTICAL_REL	(XML_TEXT_TYPES_START + 47)
#define XML_TYPE_TEXT_VERTICAL_REL_PAGE	(XML_TEXT_TYPES_START + 48)
#define XML_TYPE_TEXT_VERTICAL_REL_FRAME	(XML_TEXT_TYPES_START + 49)
#define XML_TYPE_TEXT_VERTICAL_REL_AS_CHAR	(XML_TEXT_TYPES_START + 50)
#define XML_TYPE_TEXT_MIRROR_VERTICAL (XML_TEXT_TYPES_START + 51)
#define XML_TYPE_TEXT_MIRROR_HORIZONTAL_LEFT (XML_TEXT_TYPES_START + 52)
#define XML_TYPE_TEXT_MIRROR_HORIZONTAL_RIGHT (XML_TEXT_TYPES_START + 53)
#define XML_TYPE_TEXT_CLIP	(XML_TEXT_TYPES_START + 54)
#define XML_TYPE_ATTRIBUTE_CONTAINER (XML_TEXT_TYPES_START + 55 )
#define XML_TYPE_CHAR_HEIGHT_DIFF	(XML_TEXT_TYPES_START +  56)
#define XML_TYPE_TEXT_EMPHASIZE (XML_TEXT_TYPES_START +  57)
#define XML_TYPE_TEXT_COMBINE (XML_TEXT_TYPES_START +  58)
#define XML_TYPE_TEXT_COMBINECHAR (XML_TEXT_TYPES_START +  59)
#define XML_TYPE_TEXT_UNDERLINE_COLOR (XML_TEXT_TYPES_START +  60)
#define XML_TYPE_TEXT_UNDERLINE_HASCOLOR (XML_TEXT_TYPES_START +  61)
#define XML_TYPE_TEXT_AUTOSPACE (XML_TEXT_TYPES_START + 62)
#define XML_TYPE_TEXT_PUNCTUATION_WRAP (XML_TEXT_TYPES_START + 63)
#define XML_TYPE_TEXT_LINE_BREAK (XML_TEXT_TYPES_START + 64)

#define XML_TYPE_TEXT_ALIGN				(XML_TEXT_TYPES_START + 65)
#define XML_TYPE_FONT_WIDTH				(XML_TEXT_TYPES_START + 66)
#define XML_TYPE_ROTATION_ANGLE			(XML_TEXT_TYPES_START + 67)
#define XML_TYPE_CONTROL_BORDER			(XML_TEXT_TYPES_START + 68)
#define XML_TYPE_TEXT_REL_WIDTH_HEIGHT	(XML_TEXT_TYPES_START + 69)
#define XML_TYPE_TEXT_SYNC_WIDTH_HEIGHT	(XML_TEXT_TYPES_START + 70)
#define XML_TYPE_TEXT_SYNC_WIDTH_HEIGHT_MIN	(XML_TEXT_TYPES_START + 71)
#define XML_TYPE_TEXT_COMBINE_CHARACTERS (XML_TEXT_TYPES_START +  72)
#define XML_TYPE_TEXT_ANIMATION				(XML_TEXT_TYPES_START + 73)
#define XML_TYPE_TEXT_ANIMATION_DIRECTION	(XML_TEXT_TYPES_START + 74)
#define XML_TYPE_TEXT_HORIZONTAL_ADJUST		(XML_TEXT_TYPES_START + 75)
#define XML_TYPE_TEXT_RUBY_ADJUST		(XML_TEXT_TYPES_START + 76)
#define XML_TYPE_TEXT_FONT_RELIEF		(XML_TEXT_TYPES_START + 77)
#define XML_TYPE_TEXT_ROTATION_ANGLE	(XML_TEXT_TYPES_START + 78)
#define XML_TYPE_TEXT_ROTATION_SCALE	(XML_TEXT_TYPES_START + 79)
#define XML_TYPE_TEXT_VERTICAL_ALIGN	(XML_TEXT_TYPES_START + 80)
#define XML_TYPE_TEXT_DRAW_ASPECT		(XML_TEXT_TYPES_START + 81)
#define XML_TYPE_CONTROL_TEXT_EMPHASIZE	(XML_TEXT_TYPES_START + 82)
#define XML_TYPE_TEXT_RUBY_POSITION		(XML_TEXT_TYPES_START + 83)
#define XML_TYPE_TEXT_VERTICAL_POS_AT_CHAR	(XML_TEXT_TYPES_START + 84)
#define XML_TYPE_TEXT_WRITING_MODE	(XML_TEXT_TYPES_START + 85)
#define XML_TYPE_TEXT_WRITING_MODE_WITH_DEFAULT (XML_TEXT_TYPES_START + 86)
#define XML_TYPE_LAYOUT_GRID_MODE       (XML_TEXT_TYPES_START + 87)

#define XML_SCH_TYPES_START	0x00004000
#define XML_PM_TYPES_START	0x00005000		// page master

}//end of namespace binfilter
#endif		// _XMLOFF_XMLTYPES_HXX
