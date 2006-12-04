/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PropertyIds.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: os $ $Date: 2006-12-04 15:42:28 $
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
#ifndef INCLUDED_DMAPPER_PROPERTYIDS_HXX
#define INCLUDED_DMAPPER_PROPERTYIDS_HXX

namespace rtl{ class OUString;}
namespace dmapper{
enum PropertyIds
{
        PROP_ID_START = 1
 /* 1*/ ,PROP_CHAR_WEIGHT = PROP_ID_START
 /* 2*/ ,PROP_CHAR_POSTURE
 /* 3*/ ,PROP_CHAR_STRIKEOUT
 /* 4*/ ,PROP_CHAR_CONTOURED
 /* 5*/ ,PROP_CHAR_SHADOWED
 /* 6*/ ,PROP_CHAR_CASEMAP
 /* 7*/ ,PROP_CHAR_COLOR
 /* 8*/ ,PROP_CHAR_RELIEF
 /* 9*/ ,PROP_CHAR_UNDERLINE
 /*10*/ ,PROP_CHAR_WORD_MODE
 /*11*/ ,PROP_CHAR_ESCAPEMENT
 /*12*/ ,PROP_CHAR_ESCAPEMENT_HEIGHT
 /*13*/ ,PROP_CHAR_HEIGHT
 /*14*/ ,PROP_CHAR_HEIGHT_COMPLEX
 /*15*/ ,PROP_CHAR_LOCALE
 /*16*/ ,PROP_CHAR_LOCALE_ASIAN
 /*17*/ ,PROP_CHAR_WEIGHT_COMPLEX
 /*18*/ ,PROP_CHAR_POSTURE_COMPLEX
 /*19*/ ,PROP_CHAR_AUTO_KERNING
 /*20*/ ,PROP_CHAR_CHAR_KERNING
 /*21*/ ,PROP_CHAR_SCALE_WIDTH
 /*22*/ ,PROP_CHAR_LOCALE_COMPLEX
 /*23*/ ,PROP_CHAR_UNDERLINE_COLOR
/*24*/ , PROP_CHAR_UNDERLINE_HAS_COLOR
/*25*/ , PROP_CHAR_FONT_NAME
/*26*/ , PROP_CHAR_FONT_STYLE
/*27*/ , PROP_CHAR_FONT_FAMILY
/*28*/ , PROP_CHAR_FONT_CHAR_SET
/*29*/ , PROP_CHAR_FONT_PITCH
/*30*/ , PROP_CHAR_FONT_NAME_ASIAN
/*31*/ , PROP_CHAR_FONT_STYLE_ASIAN
/*32*/ , PROP_CHAR_FONT_FAMILY_ASIAN
/*33*/ , PROP_CHAR_FONT_CHAR_SET_ASIAN
/*34*/ , PROP_CHAR_FONT_PITCH_ASIAN
/*35*/ , PROP_CHAR_FONT_NAME_COMPLEX
/*36*/ , PROP_CHAR_FONT_STYLE_COMPLEX
/*37*/ , PROP_CHAR_FONT_FAMILY_COMPLEX
/*38*/ , PROP_CHAR_FONT_CHAR_SET_COMPLEX
/*39*/ , PROP_CHAR_FONT_PITCH_COMPLEX
/*40*/ , PROP_CHAR_HIDDEN
/*41*/ , PROP_PARA_STYLE_NAME
/*42*/ , PROP_CHAR_STYLE_NAME
/*43*/ , PROP_PARA_ADJUST
/*44*/ , PROP_PARA_LAST_LINE_ADJUST
/*45*/ , PROP_PARA_RIGHT_MARGIN
/*46*/ , PROP_PARA_LEFT_MARGIN
/*47*/ , PROP_PARA_FIRST_LINE_INDENT
/*48*/ , PROP_PARA_KEEP_TOGETHER
/*49*/ , PROP_PARA_TOP_MARGIN
/*50*/ , PROP_PARA_BOTTOM_MARGIN
/*51*/ , PROP_PARA_IS_HYPHENATION
/*52*/ , PROP_PARA_LINE_NUMBER_COUNT
/*53*/ , PROP_PARA_IS_HANGING_PUNCTUATION
/*54*/ , PROP_PARA_LINE_SPACING
/*55*/ , PROP_PARA_TAB_STOPS
/*56*/ , PROP_NUMBERING_LEVEL
/*57*/ , PROP_NUMBERING_RULES
/*58*/ , PROP_NUMBERING_TYPE
/*59*/ , PROP_START_WITH
/*60*/ , PROP_ADJUST
/*61*/ , PROP_PARENT_NUMBERING
/*62*/ , PROP_LEFT_MARGIN
/*63*/ , PROP_RIGHT_MARGIN
/*64*/ , PROP_TOP_MARGIN
/*65*/ , PROP_BOTTOM_MARGIN
/*66*/ , PROP_FIRST_LINE_OFFSET
/*67*/ , PROP_LEFT_BORDER
/*68*/ , PROP_RIGHT_BORDER
/*69*/ , PROP_TOP_BORDER
/*70*/ , PROP_BOTTOM_BORDER
/*71*/ , PROP_LEFT_BORDER_DISTANCE
/*72*/ , PROP_RIGHT_BORDER_DISTANCE
/*73*/ , PROP_TOP_BORDER_DISTANCE
/*74*/ , PROP_BOTTOM_BORDER_DISTANCE
/*75*/ , PROP_CURRENT_PRESENTATION
/*76*/ , PROP_IS_FIXED
/*77*/ , PROP_SUB_TYPE
/*78*/ , PROP_FILE_FORMAT
/*79*/ , PROP_HYPER_LINK_U_R_L
/*80*/ , PROP_NUMBER_FORMAT
/*81*/ , PROP_NAME
/*82*/ , PROP_IS_INPUT
/*83*/ , PROP_HINT
/*84*/ , PROP_FULL_NAME
/*85*/ , PROP_KEYWORDS
/*86*/ , PROP_DESCRIPTION
/*87*/ , PROP_MACRO_NAME
/*88*/ , PROP_SUBJECT
/*89*/ , PROP_USER_DATA_TYPE
/*90*/ , PROP_TITLE
/*91*/ , PROP_CONTENT
/*92*/ , PROP_DATA_COLUMN_NAME
/*93*/ , PROP_INPUT_STREAM
/*94*/ , PROP_GRAPHIC
/*95*/ , PROP_ANCHOR_TYPE
/*96*/ , PROP_SIZE
/*97*/ , PROP_HORI_ORIENT
/*98*/ , PROP_HORI_ORIENT_POSITION
/*99*/ , PROP_HORI_ORIENT_RELATION
/*100*/, PROP_VERT_ORIENT
/*101*/, PROP_VERT_ORIENT_POSITION
/*102*/ ,PROP_VERT_ORIENT_RELATION
/*103*/ , PROP_GRAPHIC_CROP
/*104*/ , PROP_SIZE100th_M_M
/*105*/ , PROP_SIZE_PIXEL
/*106*/ , PROP_SURROUND
/*107*/ , PROP_SURROUND_CONTOUR
/*108*/ , PROP_ADJUST_CONTRAST
/*109*/ , PROP_ADJUST_LUMINANCE
/*110*/ , PROP_GRAPHIC_COLOR_MODE
/*111*/ , PROP_GAMMA
/*112*/ , PROP_HORI_MIRRORED_ON_EVEN_PAGES
/*113*/ , PROP_HORI_MIRRORED_ON_ODD_PAGES
/*114*/ , PROP_VERT_MIRRORED
/*115*/ , PROP_CONTOUR_OUTSIDE
/*116*/ , PROP_CONTOUR_POLY_POLYGON
/*117*/ , PROP_PAGE_TOGGLE
///*118*/ ,
///*119*/ , PROP_
///*120*/ , PROP_
///*121*/ , PROP_
///*122*/ , PROP_
///*123*/ , PROP_
///*124*/ , PROP_
///*125*/ , PROP_
///*126*/ , PROP_
///*127*/ , PROP_
///*128*/ , PROP_
///*129*/ , PROP_
///*130*/ , PROP_
///*131*/ , PROP_

};
struct PropertyNameSupplier_Impl;
class PropertyNameSupplier
{
    PropertyNameSupplier_Impl* m_pImpl;
public:
    PropertyNameSupplier();
    ~PropertyNameSupplier();
    const rtl::OUString& GetName( PropertyIds eId );

    static PropertyNameSupplier& GetPropertyNameSupplier();
};
} //namespace dmapper
#endif
