/*************************************************************************
 *
 *  $RCSfile: unoprnms.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _UNOPRNMS_HXX
#define _UNOPRNMS_HXX


#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

struct SwPropNameLen
{
    const char*                         pName;
    USHORT                              nNameLen;
    SwPropNameLen(const char* pString, USHORT nLen) :
        pName(pString),
        nNameLen(nLen){}

    operator const char*()const {return pName;}
};
#define SW_PROP_NAME(aPropName) aPropName.pName, aPropName.nNameLen

extern const SwPropNameLen  __FAR_DATA  UNO_NAME_FOLLOW_STYLE;

//extern const sal_Char* __FAR_DATA UNO_NAME_FOLLOW_STYLE                       ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_IS_PHYSICAL                      ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_IS_AUTO_UPDATE                       ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_DISPLAY_NAME                     ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CHAR_FONT_NAME                       ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CHAR_FONT_STYLE_NAME               ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CHAR_FONT_FAMILY                   ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CHAR_FONT_CHAR_SET                 ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CHAR_FONT_PITCH                    ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PARA_GRAPHIC_URL                   ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PARA_GRAPHIC_FILTER                ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_HEADER_GRAPHIC_URL                 ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_HEADER_GRAPHIC_FILTER              ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_FOOTER_GRAPHIC_URL                 ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_FOOTER_GRAPHIC_FILTER              ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_BACK_GRAPHIC_URL                   ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_BACK_GRAPHIC_FILTER                ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_BACK_GRAPHIC_LOCATION            ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_BACK_GRAPHIC_BITMAP                  ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_GRAPHIC_URL                          ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_GRAPHIC_FILTER                       ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_GRAPHIC_LOCATION                     ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_GRAPHIC_SIZE                         ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_GRAPHIC_BITMAP                   ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_BULLET_FONT                      ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PARA_GRAPHIC_LOCATION            ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_HEADER_GRAPHIC_LOCATION              ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_FOOTER_GRAPHIC_LOCATION              ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PARA_LEFT_PARA_MARGIN                ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PARA_RIGHT_PARA_MARGIN               ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PARA_LEFT_MARGIN                 ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PARA_RIGHT_MARGIN            ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PARA_LEFT_MARGIN_RELATIVE    ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PARA_RIGHT_MARGIN_RELATIVE   ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PARA_FIRST_LINE_INDENT       ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PARA_FIRST_LINE_INDENT_RELATIVE  ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PARA_IS_HYPHENATION                    ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PARA_HYPHENATION_MAX_LEADING_CHARS       ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PARA_HYPHENATION_MAX_TRAILING_CHARS    ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PARA_HYPHENATION_MAX_HYPHENS           ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_LEFT_MARGIN                      ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_RIGHT_MARGIN                     ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_HEADER_LEFT_MARGIN           ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_HEADER_RIGHT_MARGIN          ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_FOOTER_LEFT_MARGIN           ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_FOOTER_RIGHT_MARGIN          ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_TEXT_RANGE                       ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_NAME                         ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_NUMBERING_ALIGNMENT          ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_BULLET_FONT_NAME               ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_BULLET_ID                      ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CHAR_STYLE_NAME                ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_ANCHOR_CHAR_STYLE_NAME                ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SUFFIX                         ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PREFIX                         ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PARENT_NUMBERING               ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_START_WITH                       ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CHAR_POSTURE                   ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CHAR_WEIGHT                      ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CHAR_AUTO_KERNING                ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CHAR_UNDERLINE                       ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CHAR_ESCAPEMENT                      ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CHAR_CASE_MAP                        ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CHAR_SHADOWED                        ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CHAR_STRIKEOUT                   ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CHAR_CROSSED_OUT                     ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CHAR_NO_HYPHENATION                  ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CHAR_AUTO_ESCAPEMENT                 ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CHAR_HEIGHT                          ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CHAR_PROP_FONT_HEIGHT              ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CHAR_DIFF_FONT_HEIGHT              ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CHAR_ESCAPEMENT_HEIGHT             ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CHAR_COLOR                       ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CHAR_FLASH                           ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CHAR_KERNING                     ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CHAR_LOCALE                          ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CHAR_BACK_COLOR                  ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CHAR_TRANSPARENT_BACKGROUND      ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PARA_LINE_SPACING                    ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PARA_TOP_MARGIN              ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PARA_BOTTOM_MARGIN             ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PARA_TOP_MARGIN_RELATIVE     ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PARA_BOTTOM_MARGIN_RELATIVE    ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PARA_EXPAND_SINGLE_WORD        ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_END_NOTICE                       ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_EMBEDDED_OBJECTS                 ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_ALPHABETICAL_SEPARATORS        ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_BACKGROUND_COLOR               ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_BEGIN_NOTICE                   ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CASE_SENSITIVE                 ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_FRAME_STYLE_NAME               ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_NUMBERING_STYLE_NAME         ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_NUMBERING_LEVEL                ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_NUMBERING_START_VALUE            ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_NUMBERING_IS_NUMBER          ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_COMBINE_ENTRIES                ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_COUNT_LINES_IN_FRAMES          ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_DDE_COMMAND_TYPE               ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_DDE_COMMAND_FILE               ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_DDE_COMMAND_ELEMENT            ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_IS_AUTOMATIC_UPDATE          ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_DEFAULT_TABSTOP_DISTANCE       ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_DISTANCE                       ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_DROP_CAP_FORMAT              ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_DROP_CAP_WHOLE_WORD            ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_DROP_CAP_CHAR_STYLE_NAME       ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_FILE_LINK                      ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_GRAPHIC                        ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_GRAPHICS                       ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_IS_PROTECTED                   ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_KEY_AS_ENTRY                   ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PARA_KEEP_TOGETHER             ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_KEEP_TOGETHER                ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_LANDSCAPE                      ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_LINE_SEPARATOR                 ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_MARKS                          ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_NUMBER_POSITION                ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_OUTLINES                       ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PAGE_STYLE_NAME                ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PAGE_STYLE_LAYOUT              ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PARA_STYLES                    ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PARA_ADJUST                  ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PARA_REGISTER_MODE_ACTIVE      ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PARA_STYLE_NAME                ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PARA_LAST_LINE_ADJUST        ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PARA_LINE_NUMBER_COUNT               ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PARA_LINE_NUMBER_START_VALUE         ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_BACK_COLOR                       ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PARA_BACK_COLOR              ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PARA_WIDOWS                  ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PARA_ORPHANS                   ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PARA_TRANSPARENT_BACKGROUND    ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_POSITION_END_OF_DOC            ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_POSITION_PROTECTED             ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_ALTERNATIVE_TEXT               ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PRIMARY_KEY                    ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PRINT_TABLES                   ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PRINT_GRAPHICS                 ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PRINT_REVERSED                 ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PRINT_PROSPECT                 ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PRINT_CONTROLS                 ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PRINT_DRAWINGS                 ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PRING_RIGHT_PAGES              ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PRINT_BLACK_FONTS              ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PRINTER_PAPER_TRAY             ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PRINT_PAGE_BACKGROUND          ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PRINT_ANNOTATION_MODE          ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_REGISTER_MODE_ACTIVE           ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_RELATIVE_WIDTH                 ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_RELATIVE_HEIGHT                ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_REPEAT_HEADLINE                ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SEARCH_STYLES                  ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SEARCH_BACKWARDS               ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SEARCH_SIMILARITY              ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SEARCH_IN_SELECTION            ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SEARCH_CASE_SENSITIVE          ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SEARCH_SIMILARITY_ADD          ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SEARCH_SIMILARITY_RELAX        ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SEARCH_SIMILARITY_REMOVE       ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SEARCH_REGULAR_EXPRESSION      ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SEARCH_SIMILARITY_EXCHANGE     ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SECONDARY_KEY                  ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SEPARATOR_LINE_DISTANCE        ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SHOW_BREAKS                    ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SHOW_SPACES                    ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SHOW_TABLES                    ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SHOW_GRAPHICS                  ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SHOW_DRAWINGS                  ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SHOW_TABSTOPS                  ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SHOW_VERT_RULER                ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SHOW_PARA_BREAKS               ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SHOW_HIDDEN_TEXT               ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SHOW_ANNOTATIONS               ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SHOW_SOFT_HYPHENS              ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SHOW_VERT_SCROLL_BAR           ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SHOW_HORI_SCROLL_BAR           ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SHOW_FIELD_COMMANDS            ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SHOW_TEXT_BOUNDARIES           ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SHOW_PROTECTED_SPACES          ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SHOW_TABLE_BOUNDARIES          ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SHOW_HIDDEN_PARAGRAPHS         ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SHOW_INDEX_MARK_BACKGROUND     ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SHOW_FOOTNOTE_BACKGROUND       ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SHOW_TEXT_FIELD_BACKGROUND     ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SIZE_RELATIVE                  ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SIZE_PROTECTED                 ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SMOOTH_SCROLLING               ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SOLID_MARK_HANDLES             ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_TABLES                         ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_TEXT_FRAMES                    ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_TEXT_COLUMNS                   ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_TRANSPARENT_BACKGROUND         ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_USE_PP                         ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_USER_METRIC                    ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_ANCHOR_TYPE                  ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_ANCHOR_TYPES                 ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_ANCHOR_PAGE_NO                   ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_AUTHOR                       ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_BREAK_TYPE                   ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CHAIN_NEXT_NAME              ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CHAIN_PREV_NAME              ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CHAPTER_FORMAT               ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CLIENT_MAP                       ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CONDITION                        ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CONTENT                      ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CHAR_CONTOURED                   ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CONTOUR_OUTSIDE              ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CONTENT_PROTECTED                ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_COUNT_EMPTY_LINES                ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_DATA_BASE_NAME                   ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_DATA_TABLE_NAME              ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_DATA_COLUMN_NAME             ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_IS_DATA_BASE_FORMAT          ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_DATE                         ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_DATETIME                         ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_IS_DATE                      ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_EDIT_IN_READONLY             ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_FALSE_CONTENT                    ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_FILE_FORMAT                  ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_IS_FIXED                         ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_FOOTNOTE_COUNTING                ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_FORMULA                      ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_FRAME_NAME                   ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_GRAPHIC_NAME                   ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_FULL_NAME                    ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_HEIGHT                           ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_IS_AUTO_HEIGHT                   ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SIZE_TYPE                        ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_HINT                         ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_HORI_ORIENT                  ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_HORI_MIRRORED                ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_HORI_ORIENT_RELATION             ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_HORI_ORIENT_POSITION             ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_HYPER_LINK_U_R_L                 ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_HYPER_LINK_TARGET            ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_HYPER_LINK_NAME              ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_INFO_TYPE                        ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_INFO_FORMAT                  ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_IS_INPUT                     ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_LEVEL                            ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_LINE_INTERVAL                    ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_LINK_REGION                  ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_MACRO                            ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_MIRROR_PAGE_TOGGLE           ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SPLIT                        ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PARA_SPLIT                       ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_NUMBER_FORMAT                    ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_NUMBERING_TYPE                   ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_NUMBERING_RULES              ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_OFFSET                       ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_ON                           ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_OPAQUE                       ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PAGE_TOGGLE                  ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PAGE_DESC_NAME               ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PAGE_NUMBER_OFFSET               ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PLACEHOLDER                  ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PLACEHOLDER_TYPE             ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PRINT                        ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PRINT_LEFT_PAGES             ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_REFERENCE_FIELD_PART           ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_REFERENCE_FIELD_SOURCE         ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_REGISTER_PARAGRAPH_STYLE     ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SCRIPT_TYPE                  ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SEARCH_ALL                       ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SEARCH_WORDS                 ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SEQUENCE_VALUE                   ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SERVER_MAP                       ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SET_NUMBER                   ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SHADOW_FORMAT                ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SHOW_HORI_RULER              ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SIZE                         ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_ACTUAL_SIZE                      ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SOURCE_NAME                  ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_START_AT                     ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_STATISTIC_TYPE_ID                ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SUB_TYPE                     ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SURROUND                     ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_IS_EXPRESSION                    ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_IS_SHOW_FORMULA              ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_TEXT_WRAP                        ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SURROUND_CONTOUR                 ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SURROUND_ANCHORONLY          ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_TABLE_NAME                     ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_TABSTOPS                         ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_TITLE                            ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_TOP_MARGIN                       ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_BOTTOM_MARGIN                    ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_TRUE_CONTENT                     ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_URL_CONTENT                      ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_USERTEXT                     ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_USER_DATA_TYPE                   ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_VALUE                            ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_VARIABLE_NAME                    ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_VARIABLE_SUBTYPE             ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_VERT_ORIENT                  ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_VERT_MIRRORED                ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_VERT_ORIENT_POSITION             ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_VERT_ORIENT_RELATION             ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_IS_VISIBLE                       ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_WIDTH                            ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_WORD_MODE                    ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_GRAPHIC_CROP                 ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CHARACTER_FORMAT_NONE          ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_TEXT_POSITION                ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_DOCUMENT_INDEX_MARK          ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_DOCUMENT_INDEX                 ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_TEXT_FIELD                     ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_BOOKMARK                       ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_TEXT_TABLE                     ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CELL                           ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_TEXT_FRAME                     ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_REFERENCE_MARK                 ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_TEXT_SECTION                   ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_FOOTNOTE                       ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_ENDNOTE                          ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CHART_ROW_AS_LABEL               ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CHART_COLUMN_AS_LABEL          ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_TABLE_COLUMS                 ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_LEFT_BORDER                  ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_RIGHT_BORDER                 ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_TOP_BORDER                       ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_BOTTOM_BORDER                  ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_BORDER_DISTANCE                ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_LEFT_BORDER_DISTANCE         ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_RIGHT_BORDER_DISTANCE            ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_TOP_BORDER_DISTANCE          ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_BOTTOM_BORDER_DISTANCE           ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_TABLE_BORDER                     ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_TABLE_COLUMN_SEPARATORS      ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_TABLE_COLUMN_RELATIVE_SUM        ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_HEADER_TEXT                      ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_HEADER_TEXT_LEFT               ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_HEADER_TEXT_RIGHT              ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_FOOTER_TEXT                    ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_FOOTER_TEXT_LEFT               ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_FOOTER_TEXT_RIGHT              ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_HEADER_BACK_COLOR                ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_HEADER_GRAPHIC                 ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_HEADER_TRANSPARENT_BACKGROUND  ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_HEADER_LEFT_BORDER               ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_HEADER_RIGHT_BORDER          ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_HEADER_TOP_BORDER                ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_HEADER_BOTTOM_BORDER           ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_HEADER_BORDER_DISTANCE         ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_HEADER_SHADOW_FORMAT             ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_HEADER_BODY_DISTANCE         ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_HEADER_IS_DYNAMIC_DISTANCE     ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_HEADER_SHARE_CONTENT           ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_HEADER_HEIGHT                  ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_HEADER_ON                        ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_FOOTER_BACK_COLOR                ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_FOOTER_GRAPHIC                 ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_FOOTER_TRANSPARENT_BACKGROUND  ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_FOOTER_LEFT_BORDER               ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_FOOTER_RIGHT_BORDER          ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_FOOTER_TOP_BORDER                ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_FOOTER_BOTTOM_BORDER           ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_FOOTER_BORDER_DISTANCE         ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_FOOTER_SHADOW_FORMAT             ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_FOOTER_BODY_DISTANCE         ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_FOOTER_IS_DYNAMIC_DISTANCE     ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_FOOTER_SHARE_CONTENT         ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_FOOTER_HEIGHT                  ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_FOOTER_ON                        ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_OVERWRITE_STYLES               ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_LOAD_NUMBERING_STYLES          ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_LOAD_PAGE_STYLES               ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_LOAD_FRAME_STYLES              ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_LOAD_TEXT_STYLES               ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_FILE_NAME                        ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_FILTER_NAME                  ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_FILTER_OPTION                    ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PASSWORD                     ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_COPY_COUNT                     ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_COLLATE                      ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SORT                         ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PAGES                            ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_FIRST_LINE_OFFSET                ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SYMBOL_TEXT_DISTANCE           ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_USER_INDEX_NAME                ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_REVISION                       ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_UNVISITED_CHAR_STYLE_NAME      ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_VISITED_CHAR_STYLE_NAME        ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PARAGRAPH_COUNT              ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_WORD_COUNT                     ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_WORD_SEPARATOR                 ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CHARACTER_COUNT                ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_ZOOM_VALUE                     ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_ZOOM_TYPE                        ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CREATE_FROM_MARKS                  ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CREATE_FROM_OUTLINE                ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PARAGRAPH_STYLE_NAMES              ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CREATE_FROM_CHAPTER                ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CREATE_FROM_LABELS                 ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_USE_ALPHABETICAL_SEPARATORS        ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_USE_KEY_AS_ENTRY                   ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_USE_COMBINED_ENTRIES               ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_IS_CASE_SENSITIVE                  ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_USE_P_P                            ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_USE_DASH                           ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_USE_UPPER_CASE                     ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_LABEL_CATEGORY                     ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_LABEL_DISPLAY_TYPE                 ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_USE_LEVEL_FROM_SOURCE              ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_LEVEL_FORMAT                       ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_LEVEL_PARAGRAPH_STYLES             ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_RECALC_TAB_STOPS                   ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_MAIN_ENTRY_CHARACTER_STYLE_NAME    ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CREATE_FROM_TABLES                 ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CREATE_FROM_TEXT_FRAMES            ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CREATE_FROM_GRAPHIC_OBJECTS        ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CREATE_FROM_EMBEDDED_OBJECTS       ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CREATE_FROM_STAR_MATH              ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CREATE_FROM_STAR_IMAGE             ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CREATE_FROM_STAR_CHART             ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CREATE_FROM_STAR_CALC              ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CREATE_FROM_STAR_DRAW              ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CREATE_FROM_OTHER_EMBEDDED_OBJECTS ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_INDEX_AUTO_MARK_FILE_U_R_L           ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_IS_COMMA_SEPARATED                   ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SHOW_CHANGES                       ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_RECORD_CHANGES                     ;
extern const SwPropNameLen __FAR_DATA UNO_LINK_DISPLAY_NAME                       ;
extern const SwPropNameLen __FAR_DATA UNO_LINK_DISPLAY_BITMAP                     ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_HEADING_STYLE_NAME                   ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SHOW_ONLINE_LAYOUT                   ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_USER_DEFINED_ATTRIBUTES          ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PARA_USER_DEFINED_ATTRIBUTES     ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CHAR_USER_DEFINED_ATTRIBUTES     ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_FILE_PATH                            ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PARA_CHAPTER_NUMBERING_LEVEL     ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_PARA_CONDITIONAL_STYLE_NAME        ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CHAPTER_NUMBERING_LEVEL            ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_NUMBERING_SEPARATOR                ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_IS_CONTINUOUS_NUMBERING            ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_IS_AUTOMATIC                     ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_IS_ABSOLUTE_MARGINS              ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CATEGORY                         ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_DEPENDENT_TEXT_FIELDS                ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CURRENT_PRESENTATION             ;
extern const SwPropNameLen __FAR_DATA UNO_NAME_ADJUST                           ;

extern const SwPropNameLen __FAR_DATA UNO_NAME_TEXT_PORTION_TYPE;
extern const SwPropNameLen __FAR_DATA UNO_NAME_CONTROL_CHARACTER;
extern const SwPropNameLen __FAR_DATA UNO_NAME_IS_COLLAPSED;
extern const SwPropNameLen __FAR_DATA UNO_NAME_IS_START;
extern const SwPropNameLen __FAR_DATA UNO_NAME_SEQUENCE_NUMBER;
extern const SwPropNameLen __FAR_DATA UNO_NAME_REFERENCE_ID;
#endif


