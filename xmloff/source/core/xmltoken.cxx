/*************************************************************************
 *
 *  $RCSfile: xmltoken.cxx,v $
 *
 *  $Revision: 1.63 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 12:57:18 $
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

#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

using ::rtl::OUString;

namespace xmloff { namespace token {

    // keep the tokens (and their length)
    struct XMLTokenEntry
    {
        sal_Int32 nLength;
        const sal_Char* pChar;
        ::rtl::OUString* pOUString;
    };




#define TOKEN( s ) { sizeof(s)-1, s, NULL }

    struct XMLTokenEntry aTokenList[] =
    {
        { 0, NULL, NULL },                            // XML_TOKEN_START

        // common XML
        TOKEN( "CDATA" ),            // XML_CDATA
        TOKEN( " " ),                // XML_WS
        TOKEN( "xml" ),              // XML_xml
        TOKEN( "xmlns" ),            // XML_xmlns
        TOKEN( "version=\"1.0\" encoding=\"UTF-8\"" ), // XML_xml_pi
        TOKEN( "<!DOCTYPE " ),       // XML_xml_doctype_prefix
        TOKEN( " PUBLIC \"-//OpenOffice.org//DTD OfficeDocument 1.0//EN\" \"office.dtd\">" ), // XML_xml_doctype_suffix

        // namespace prefixes and names
        TOKEN( "office" ),           // XML_np_office
        TOKEN( "urn:oasis:names:tc:openoffice:xmlns:office:1.0" ), // XML_n_office
        TOKEN( "http://sun.com/xmlns/staroffice/office" ), // XML_n_office_old
        TOKEN( "meta" ),             // XML_np_meta
        TOKEN( "urn:oasis:names:tc:openoffice:xmlns:meta:1.0" ), // XML_n_meta
        TOKEN( "http://sun.com/xmlns/staroffice/meta" ), // XML_n_meta_old
        TOKEN( "style" ),            // XML_np_style
        TOKEN( "urn:oasis:names:tc:openoffice:xmlns:style:1.0" ), // XML_n_style
        TOKEN( "http://sun.com/xmlns/staroffice/style" ), // XML_n_style_old
        TOKEN( "number" ),           // XML_np_number
        TOKEN( "urn:oasis:names:tc:openoffice:xmlns:datastyle:1.0" ), // XML_n_number
        TOKEN( "http://sun.com/xmlns/staroffice/number" ), // XML_n_number_old
        TOKEN( "text" ),             // XML_np_text
        TOKEN( "urn:oasis:names:tc:openoffice:xmlns:text:1.0" ), // XML_n_text
        TOKEN( "http://sun.com/xmlns/staroffice/text" ), // XML_n_text_old
        TOKEN( "table" ),            // XML_np_table
        TOKEN( "urn:oasis:names:tc:openoffice:xmlns:table:1.0" ), // XML_n_table
        TOKEN( "http://sun.com/xmlns/staroffice/table" ), // XML_n_table_old
        TOKEN( "draw" ),             // XML_np_draw
        TOKEN( "urn:oasis:names:tc:openoffice:xmlns:drawing:1.0" ), // XML_n_draw
        TOKEN( "dr3d" ),             // XML_np_dr3d
        TOKEN( "urn:oasis:names:tc:openoffice:xmlns:dr3d:1.0" ), // XML_n_dr3d
        TOKEN( "http://sun.com/xmlns/staroffice/draw" ), // XML_n_draw_old
        TOKEN( "presentation" ),        // XML_np_presentation
        TOKEN( "urn:oasis:names:tc:openoffice:xmlns:presentation:1.0" ), // XML_n_presentation
        TOKEN( "http://sun.com/xmlns/staroffice/presentation" ), // XML_n_presentation_old
        TOKEN( "chart" ),                // XML_np_chart
        TOKEN( "urn:oasis:names:tc:openoffice:xmlns:chart:1.0" ), // XML_n_chart
        TOKEN( "config" ),               // XML_np_config
        TOKEN( "urn:oasis:names:tc:openoffice:xmlns:config:1.0" ), // XML_n_config
        TOKEN( "http://sun.com/xmlns/staroffice/chart" ), // XML_n_chart_old
        TOKEN( "fo" ),                   // XML_np_fo
        TOKEN( "http://www.w3.org/1999/XSL/Format/" ), // XML_n_fo_old
        TOKEN( "http://www.w3.org/1999/XSL/Format" ), // XML_n_fo
        TOKEN( "xlink" ),                // XML_np_xlink
        TOKEN( "http://www.w3.org/1999/xlink" ), // XML_n_xlink
        TOKEN( "http://www.w3.org/1999/xlink/namespace" ), // XML_n_xlink_old
        TOKEN( "dc" ),                   // XML_np_dc
        TOKEN( "http://purl.org/dc/elements/1.1/" ), // XML_n_dc
        TOKEN( "svg" ),                  // XML_np_svg
        TOKEN( "http://www.w3.org/2000/svg" ), // XML_n_svg
        TOKEN( "form" ),                 // XML_NP_form
        TOKEN( "urn:oasis:names:tc:openoffice:xmlns:form:1.0" ), // XML_N_form
        TOKEN( "script" ),               // XML_NP_script
        TOKEN( "urn:oasis:names:tc:openoffice:xmlns:script:1.0" ), // XML_N_script


        TOKEN( "block-list" ),           // XML_np_block_list
        TOKEN( "http://openoffice.org/2001/block-list" ), // XML_n_block_list

        TOKEN( "math" ),                 // XML_np_math
        TOKEN( "http://www.w3.org/1998/Math/MathML" ), // XML_n_math

        TOKEN( "VL" ),                   // XML_np_versions_list
        TOKEN( "http://openoffice.org/2001/versions-list" ), // XML_n_versions_list

        // units
        TOKEN( "mm" ),                         // XML_unit_mm
        TOKEN( "m" ),                          // XML_unit_m
        TOKEN( "km" ),                         // XML_unit_km
        TOKEN( "cm" ),                         // XML_unit_cm
        TOKEN( "pt" ),                         // XML_unit_pt
        TOKEN( "pc" ),                         // XML_unit_pc
        TOKEN( "ft" ),                         // XML_unit_foot
        TOKEN( "mi" ),                         // XML_unit_miles
        TOKEN( "in" ),                       // XML_unit_inch

        // any other
        TOKEN( "1" ),                          // XML_1
        TOKEN( "10" ),                         // XML_10
        TOKEN( "2" ),                          // XML_2
        TOKEN( "3" ),                          // XML_3
        TOKEN( "4" ),                          // XML_4
        TOKEN( "5" ),                          // XML_5
        TOKEN( "6" ),                          // XML_6
        TOKEN( "7" ),                          // XML_7
        TOKEN( "8" ),                          // XML_8
        TOKEN( "9" ),                          // XML_9
        TOKEN( "A" ),                          // XML_A_UPCASE
        TOKEN( "I" ),                          // XML_I_UPCASE
        TOKEN( "IBM437" ),                     // XML_IBM437
        TOKEN( "IBM850" ),                     // XML_IBM850
        TOKEN( "IBM860" ),                     // XML_IBM860
        TOKEN( "IBM861" ),                     // XML_IBM861
        TOKEN( "IBM863" ),                     // XML_IBM863
        TOKEN( "IBM865" ),                     // XML_IBM865
        TOKEN( "ISO-8859-1" ),                 // XML_ISO_8859_1
        TOKEN( "ole2" ),                       // XML_OLE2
        TOKEN( ":" ),                          // XML__COLON
        TOKEN( "" ),                           // XML__EMPTY
        TOKEN( "_unknown_" ),                  // XML__UNKNOWN_

        TOKEN( "a" ),                          // XML_A
        TOKEN( "abbreviated-name" ),           // XML_ABBREVIATED_NAME
        TOKEN( "above" ),                      // XML_ABOVE
        TOKEN( "abs" ),                        // XML_ABS
        TOKEN( "accent" ),                     // XML_ACCENT
        TOKEN( "accentunder" ),                // XML_ACCENTUNDER
        TOKEN( "acceptance-state" ),           // XML_ACCEPTANCE_STATE
        TOKEN( "accepted" ),                   // XML_ACCEPTED
        TOKEN( "action" ),                     // XML_ACTION
        TOKEN( "active" ),                     // XML_ACTIVE
        TOKEN( "active-split-range" ),         // XML_ACTIVE_SPLIT_RANGE
        TOKEN( "active-table" ),               // XML_ACTIVE_TABLE
        TOKEN( "actuate" ),                    // XML_ACTUATE
        TOKEN( "add-in" ),                     // XML_ADD_IN
        TOKEN( "add-in-name" ),                // XML_ADD_IN_NAME
        TOKEN( "address" ),                    // XML_ADDRESS
        TOKEN( "adjustment" ),                 // XML_ADJUSTMENT
        TOKEN( "algorithm" ),                  // XML_ALGORITHM
        TOKEN( "align" ),                      // XML_ALIGN
        TOKEN( "all" ),                        // XML_ALL
        TOKEN( "allow-empty-cell" ),           // XML_ALLOW_EMPTY_CELL
        TOKEN( "alphabetical-index" ),         // XML_ALPHABETICAL_INDEX
        TOKEN( "alphabetical-index-auto-mark-file" ),  // XML_ALPHABETICAL_INDEX_AUTO_MARK_FILE
        TOKEN( "alphabetical-index-entry-template" ),  // XML_ALPHABETICAL_INDEX_ENTRY_TEMPLATE
        TOKEN( "alphabetical-index-mark" ),    // XML_ALPHABETICAL_INDEX_MARK
        TOKEN( "alphabetical-index-mark-end" ),// XML_ALPHABETICAL_INDEX_MARK_END
        TOKEN( "alphabetical-index-mark-start" ),  // XML_ALPHABETICAL_INDEX_MARK_START
        TOKEN( "alphabetical-index-source" ),  // XML_ALPHABETICAL_INDEX_SOURCE
        TOKEN( "alphabetical-separators" ),    // XML_ALPHABETICAL_SEPARATORS
        TOKEN( "alternate" ),                  // XML_ALTERNATE
        TOKEN( "am-pm" ),                      // XML_AM_PM
        TOKEN( "ambient-color" ),              // XML_AMBIENT_COLOR
        TOKEN( "anchor-page-number" ),         // XML_ANCHOR_PAGE_NUMBER
        TOKEN( "anchor-type" ),                // XML_ANCHOR_TYPE
        TOKEN( "and" ),                        // XML_AND
        TOKEN( "animation" ),                  // XML_ANIMATION
        TOKEN( "animation-delay" ),            // XML_ANIMATION_DELAY
        TOKEN( "animation-direction" ),        // XML_ANIMATION_DIRECTION
        TOKEN( "animation-repeat" ),           // XML_ANIMATION_REPEAT
        TOKEN( "animation-start-inside" ),     // XML_ANIMATION_START_INSIDE
        TOKEN( "animation-steps" ),            // XML_ANIMATION_STEPS
        TOKEN( "animation-stop-inside" ),      // XML_ANIMATION_STOP_INSIDE
        TOKEN( "animations" ),                 // XML_ANIMATIONS
        TOKEN( "annotation" ),                 // XML_ANNOTATION
        TOKEN( "annotations" ),                // XML_ANNOTATIONS
        TOKEN( "annote" ),                     // XML_ANNOTE
        TOKEN( "appear" ),                     // XML_APPEAR
        TOKEN( "applet" ),                     // XML_APPLET
        TOKEN( "applet-name" ),                // XML_APPLET_NAME
        TOKEN( "application-data" ),           // XML_APPLICATION_DATA
        TOKEN( "application-xml" ),            // XML_APPLICATION_XML
        TOKEN( "apply" ),                      // XML_APPLY
        TOKEN( "apply-style-name" ),           // XML_APPLY_STYLE_NAME
        TOKEN( "aqua" ),                       // XML_AQUA
        TOKEN( "arc" ),                        // XML_ARC
        TOKEN( "arccos" ),                     // XML_ARCCOS
        TOKEN( "archive" ),                    // XML_ARCHIVE
        TOKEN( "arcsin" ),                     // XML_ARCSIN
        TOKEN( "arctan" ),                     // XML_ARCTAN
        TOKEN( "area" ),                       // XML_AREA
        TOKEN( "area-circle" ),                // XML_AREA_CIRCLE
        TOKEN( "area-polygon" ),               // XML_AREA_POLYGON
        TOKEN( "area-rectangle" ),             // XML_AREA_RECTANGLE
        TOKEN( "article" ),                    // XML_ARTICLE
        TOKEN( "as-char" ),                    // XML_AS_CHAR
        TOKEN( "ascending" ),                  // XML_ASCENDING
        TOKEN( "attached-axis" ),              // XML_ATTACHED_AXIS
        TOKEN( "attractive" ),                 // XML_ATTRACTIVE
        TOKEN( "author" ),                     // XML_AUTHOR
        TOKEN( "author-initials" ),            // XML_AUTHOR_INITIALS
        TOKEN( "author-name" ),                // XML_AUTHOR_NAME
        TOKEN( "auto" ),                       // XML_AUTO
        TOKEN( "auto-grow-height" ),           // XML_AUTO_GROW_HEIGHT
        TOKEN( "auto-grow-width" ),            // XML_AUTO_GROW_WIDTH
        TOKEN( "auto-reload" ),                // XML_AUTO_RELOAD
        TOKEN( "auto-text" ),                  // XML_AUTO_TEXT
        TOKEN( "auto-text-events" ),           // XML_AUTO_TEXT_EVENTS
        TOKEN( "auto-text-group" ),            // XML_AUTO_TEXT_GROUP
        TOKEN( "auto-text-indent" ),           // XML_AUTO_TEXT_INDENT
        TOKEN( "auto-update" ),                // XML_AUTO_UPDATE
        TOKEN( "automatic" ),                  // XML_AUTOMATIC
        TOKEN( "automatic-find-labels" ),      // XML_AUTOMATIC_FIND_LABELS
        TOKEN( "automatic-order" ),            // XML_AUTOMATIC_ORDER
        TOKEN( "automatic-styles" ),           // XML_AUTOMATIC_STYLES
        TOKEN( "automatic-update" ),           // XML_AUTOMATIC_UPDATE
        TOKEN( "autosize" ),                   // XML_AUTOSIZE
        TOKEN( "average" ),                    // XML_AVERAGE
        TOKEN( "axis" ),                       // XML_AXIS
        TOKEN( "back-scale" ),                 // XML_BACK_SCALE
        TOKEN( "backface-culling" ),           // XML_BACKFACE_CULLING
        TOKEN( "background" ),                 // XML_BACKGROUND
        TOKEN( "background-color" ),           // XML_BACKGROUND_COLOR
        TOKEN( "background-image" ),           // XML_BACKGROUND_IMAGE
        TOKEN( "no-repeat" ),                  // XML_BACKGROUND_NO_REPEAT
        TOKEN( "repeat" ),                     // XML_BACKGROUND_REPEAT
        TOKEN( "stretch" ),                    // XML_BACKGROUND_STRETCH
        TOKEN( "bar" ),                        // XML_BAR
        TOKEN( "base64Binary" ),               // XML_BASE64BINARY
        TOKEN( "base-cell-address" ),          // XML_BASE_CELL_ADDRESS
        TOKEN( "baseline" ),                   // XML_BASELINE
        TOKEN( "before-date-time" ),           // XML_BEFORE_DATE_TIME
        TOKEN( "below" ),                      // XML_BELOW
        TOKEN( "between-date-times" ),         // XML_BETWEEN_DATE_TIMES
        TOKEN( "bevel" ),                      // XML_BEVEL
        TOKEN( "bibiliographic-type" ),        // XML_BIBILIOGRAPHIC_TYPE
        TOKEN( "bibliography" ),               // XML_BIBLIOGRAPHY
        TOKEN( "bibliography-configuration" ), // XML_BIBLIOGRAPHY_CONFIGURATION
        TOKEN( "bibliography-data-field" ),    // XML_BIBLIOGRAPHY_DATA_FIELD
        TOKEN( "bibliography-entry-template" ),// XML_BIBLIOGRAPHY_ENTRY_TEMPLATE
        TOKEN( "bibliography-mark" ),          // XML_BIBLIOGRAPHY_MARK
        TOKEN( "bibliography-source" ),        // XML_BIBLIOGRAPHY_SOURCE
        TOKEN( "bibliography-type" ),          // XML_BIBLIOGRAPHY_TYPE
        TOKEN( "bind-styles-to-content" ),     // XML_BIND_STYLES_TO_CONTENT
        TOKEN( "bitmap" ),                     // XML_BITMAP
        TOKEN( "black" ),                      // XML_BLACK
        TOKEN( "blend" ),                      // XML_BLEND
        TOKEN( "blinking" ),                   // XML_BLINKING
        TOKEN( "block" ),                      // XML_BLOCK
        TOKEN( "block-list" ),                 // XML_BLOCK_LIST
        TOKEN( "blue" ),                       // XML_BLUE
        TOKEN( "body" ),                       // XML_BODY
        TOKEN( "bold" ),                       // XML_BOLD
        TOKEN( "book" ),                       // XML_BOOK
        TOKEN( "booklet" ),                    // XML_BOOKLET
        TOKEN( "bookmark" ),                   // XML_BOOKMARK
        TOKEN( "bookmark-end" ),               // XML_BOOKMARK_END
        TOKEN( "bookmark-ref" ),               // XML_BOOKMARK_REF
        TOKEN( "bookmark-start" ),             // XML_BOOKMARK_START
        TOKEN( "booktitle" ),                  // XML_BOOKTITLE
        TOKEN( "boolean" ),                    // XML_BOOLEAN
        TOKEN( "boolean-style" ),              // XML_BOOLEAN_STYLE
        TOKEN( "boolean-value" ),              // XML_BOOLEAN_VALUE
        TOKEN( "border" ),                     // XML_BORDER
        TOKEN( "border-bottom" ),              // XML_BORDER_BOTTOM
        TOKEN( "border-color" ),               // XML_BORDER_COLOR
        TOKEN( "border-left" ),                // XML_BORDER_LEFT
        TOKEN( "border-line-width" ),          // XML_BORDER_LINE_WIDTH
        TOKEN( "border-line-width-bottom" ),   // XML_BORDER_LINE_WIDTH_BOTTOM
        TOKEN( "border-line-width-left" ),     // XML_BORDER_LINE_WIDTH_LEFT
        TOKEN( "border-line-width-right" ),    // XML_BORDER_LINE_WIDTH_RIGHT
        TOKEN( "border-line-width-top" ),      // XML_BORDER_LINE_WIDTH_TOP
        TOKEN( "border-right" ),               // XML_BORDER_RIGHT
        TOKEN( "border-top" ),                 // XML_BORDER_TOP
        TOKEN( "both" ),                       // XML_BOTH
        TOKEN( "bottom" ),                     // XML_BOTTOM
        TOKEN( "bottom-left" ),                // XML_BOTTOM_LEFT
        TOKEN( "bottom percent" ),             // XML_BOTTOM_PERCENT
        TOKEN( "bottom-right" ),               // XML_BOTTOM_RIGHT
        TOKEN( "bottom values" ),              // XML_BOTTOM_VALUES
        TOKEN( "bottom-arc" ),                 // XML_BOTTOMARC
        TOKEN( "bottom-circle" ),              // XML_BOTTOMCIRCLE
        TOKEN( "break-after" ),                // XML_BREAK_AFTER
        TOKEN( "break-before" ),               // XML_BREAK_BEFORE
        TOKEN( "break-inside" ),               // XML_BREAK_INSIDE
        TOKEN( "bubble" ),                     // XML_BUBBLE
        TOKEN( "bullet-char" ),                // XML_BULLET_CHAR
        TOKEN( "bullet-relative-size" ),       // XML_BULLET_RELATIVE_SIZE
        TOKEN( "button1" ),                    // XML_BUTTON1
        TOKEN( "button2" ),                    // XML_BUTTON2
        TOKEN( "button3" ),                    // XML_BUTTON3
        TOKEN( "button4" ),                    // XML_BUTTON4
        TOKEN( "buttons" ),                    // XML_BUTTONS
        TOKEN( "bvar" ),                       // XML_BVAR
        TOKEN( "c" ),                          // XML_C
        TOKEN( "calculation-settings" ),       // XML_CALCULATION_SETTINGS
        TOKEN( "calendar" ),                   // XML_CALENDAR
        TOKEN( "capitalize-entries" ),         // XML_CAPITALIZE_ENTRIES
        TOKEN( "caption" ),                    // XML_CAPTION
        TOKEN( "caption-point-x" ),            // XML_CAPTION_POINT_X
        TOKEN( "caption-point-y" ),            // XML_CAPTION_POINT_Y
        TOKEN( "caption-sequence-format" ),    // XML_CAPTION_SEQUENCE_FORMAT
        TOKEN( "caption-sequence-name" ),      // XML_CAPTION_SEQUENCE_NAME
        TOKEN( "case-sensitive" ),             // XML_CASE_SENSITIVE
        TOKEN( "capitalize" ),                 // XML_CASEMAP_CAPITALIZE
        TOKEN( "lowercase" ),                  // XML_CASEMAP_LOWERCASE
        TOKEN( "normal" ),                     // XML_CASEMAP_NORMAL
        TOKEN( "small-caps" ),                 // XML_CASEMAP_SMALL_CAPS
        TOKEN( "uppercase" ),                  // XML_CASEMAP_UPPERCASE
        TOKEN( "categories" ),                 // XML_CATEGORIES
        TOKEN( "category" ),                   // XML_CATEGORY
        TOKEN( "category-and-value" ),         // XML_CATEGORY_AND_VALUE
        TOKEN( "cell-address" ),               // XML_CELL_ADDRESS
        TOKEN( "cell-content-change" ),        // XML_CELL_CONTENT_CHANGE
        TOKEN( "cell-content-deletion" ),      // XML_CELL_CONTENT_DELETION
        TOKEN( "cell-count" ),                 // XML_CELL_COUNT
        TOKEN( "cell-protect" ),               // XML_CELL_PROTECT
        TOKEN( "cell-range-address" ),         // XML_CELL_RANGE_ADDRESS
        TOKEN( "cell-range-address-list" ),    // XML_CELL_RANGE_ADDRESS_LIST
        TOKEN( "cell-range-source" ),          // XML_CELL_RANGE_SOURCE
        TOKEN( "center" ),                     // XML_CENTER
        TOKEN( "chain-next-name" ),            // XML_CHAIN_NEXT_NAME
        TOKEN( "change" ),                     // XML_CHANGE
        TOKEN( "change-deletion" ),            // XML_CHANGE_DELETION
        TOKEN( "change-end" ),                 // XML_CHANGE_END
        TOKEN( "change-id" ),                  // XML_CHANGE_ID
        TOKEN( "change-info" ),                // XML_CHANGE_INFO
        TOKEN( "change-start" ),               // XML_CHANGE_START
        TOKEN( "change-track-table-cell" ),    // XML_CHANGE_TRACK_TABLE_CELL
        TOKEN( "change-view-conditions" ),     // XML_CHANGE_VIEW_CONDITIONS
        TOKEN( "change-view-settings" ),       // XML_CHANGE_VIEW_SETTINGS
        TOKEN( "changed-region" ),             // XML_CHANGED_REGION
        TOKEN( "chapter" ),                    // XML_CHAPTER
        TOKEN( "char" ),                       // XML_CHAR
        TOKEN( "character-count" ),            // XML_CHARACTER_COUNT
        TOKEN( "chart" ),                      // XML_CHART
        TOKEN( "charts" ),                     // XML_CHARTS
        TOKEN( "checkerboard" ),               // XML_CHECKERBOARD
        TOKEN( "chg-author" ),                 // XML_CHG_AUTHOR
        TOKEN( "chg-comment" ),                // XML_CHG_COMMENT
        TOKEN( "chg-date-time" ),              // XML_CHG_DATE_TIME
        TOKEN( "ci" ),                         // XML_CI
        TOKEN( "circle" ),                     // XML_CIRCLE
        TOKEN( "citation-body-style-name" ),   // XML_CITATION_BODY_STYLE_NAME
        TOKEN( "citation-style-name" ),        // XML_CITATION_STYLE_NAME
        TOKEN( "class" ),                      // XML_CLASS
        TOKEN( "class-id" ),                   // XML_CLASS_ID
        TOKEN( "clip" ),                       // XML_CLIP
        TOKEN( "clockwise" ),                  // XML_CLOCKWISE
        TOKEN( "close" ),                      // XML_CLOSE
        TOKEN( "close-horizontal" ),           // XML_CLOSE_HORIZONTAL
        TOKEN( "close-vertical" ),             // XML_CLOSE_VERTICAL
        TOKEN( "cm" ),                         // XML_CM
        TOKEN( "cn" ),                         // XML_CN
        TOKEN( "code" ),                       // XML_CODE
        TOKEN( "codebase" ),                   // XML_CODEBASE
        TOKEN( "collapse" ),                   // XML_COLLAPSE
        TOKEN( "color" ),                      // XML_COLOR
        TOKEN( "color-inversion" ),            // XML_COLOR_INVERSION
        TOKEN( "color-mode" ),                 // XML_COLOR_MODE
        TOKEN( "column" ),                     // XML_COLUMN
        TOKEN( "column-count" ),               // XML_COLUMN_COUNT
        TOKEN( "column-gap" ),                 // XML_COLUMN_GAP
        TOKEN( "column-name" ),                // XML_COLUMN_NAME
        TOKEN( "column-sep" ),                 // XML_COLUMN_SEP
        TOKEN( "column-width" ),               // XML_COLUMN_WIDTH
        TOKEN( "columns" ),                    // XML_COLUMNS
        TOKEN( "auto" ),                       // XML_COLUMNSPLIT_AUTO
        TOKEN( "avoid" ),                      // XML_COLUMNSPLIT_AVOID
        TOKEN( "combine-entries" ),            // XML_COMBINE_ENTRIES
        TOKEN( "combine-entries-with-dash" ),  // XML_COMBINE_ENTRIES_WITH_DASH
        TOKEN( "combine-entries-with-pp" ),    // XML_COMBINE_ENTRIES_WITH_PP
        TOKEN( "comma-separated" ),            // XML_COMMA_SEPARATED
        TOKEN( "command" ),                    // XML_COMMAND
        TOKEN( "comment" ),                    // XML_COMMENT
        TOKEN( "compose" ),                    // XML_COMPOSE
        TOKEN( "cond-style-name" ),            // XML_COND_STYLE_NAME
        TOKEN( "condition" ),                  // XML_CONDITION
        TOKEN( "condition-source" ),           // XML_CONDITION_SOURCE
        TOKEN( "condition-source-range-address" ), // XML_CONDITION_SOURCE_RANGE_ADDRESS
        TOKEN( "conditional-text" ),           // XML_CONDITIONAL_TEXT
        TOKEN( "cone" ),                       // XML_CONE
        TOKEN( "conference" ),                 // XML_CONFERENCE
        TOKEN( "config-item" ),                // XML_CONFIG_ITEM
        TOKEN( "config-item-map-entry" ),      // XML_CONFIG_ITEM_MAP_ENTRY
        TOKEN( "config-item-map-indexed" ),    // XML_CONFIG_ITEM_MAP_INDEXED
        TOKEN( "config-item-map-named" ),      // XML_CONFIG_ITEM_MAP_NAMED
        TOKEN( "config-item-set" ),            // XML_CONFIG_ITEM_SET
        TOKEN( "configuration-settings" ),     // XML_CONFIGURATION_SETTINGS
        TOKEN( "conjugate" ),                  // XML_CONJUGATE
        TOKEN( "connect-bars" ),               // XML_CONNECT_BARS
        TOKEN( "connection-name" ),            // XML_CONNECTION_NAME
        TOKEN( "connector" ),                  // XML_CONNECTOR
        TOKEN( "consecutive-numbering" ),      // XML_CONSECUTIVE_NUMBERING
        TOKEN( "consolidation" ),              // XML_CONSOLIDATION
        TOKEN( "constant" ),                   // XML_CONSTANT
        TOKEN( "contains-error" ),             // XML_CONTAINS_ERROR
        TOKEN( "contains-header" ),            // XML_CONTAINS_HEADER
        TOKEN( "content" ),                    // XML_CONTENT
        TOKEN( "content-validation" ),         // XML_CONTENT_VALIDATION
        TOKEN( "validation-name" ),            // XML_CONTENT_VALIDATION_NAME
        TOKEN( "content-validations" ),        // XML_CONTENT_VALIDATIONS
        TOKEN( "continue" ),                   // XML_CONTINUE
        TOKEN( "continue-numbering" ),         // XML_CONTINUE_NUMBERING
        TOKEN( "contour-path" ),               // XML_CONTOUR_PATH
        TOKEN( "contour-polygon" ),            // XML_CONTOUR_POLYGON
        TOKEN( "contrast" ),                   // XML_CONTRAST
        TOKEN( "control" ),                    // XML_CONTROL
        TOKEN( "conversion-mode" ),            // XML_CONVERSION_MODE
        TOKEN( "copy-back" ),                  // XML_COPY_BACK
        TOKEN( "copy-formulas" ),              // XML_COPY_FORMULAS
        TOKEN( "copy-outline-levels" ),        // XML_COPY_OUTLINE_LEVELS
        TOKEN( "copy-results-only" ),          // XML_COPY_RESULTS_ONLY
        TOKEN( "copy-styles" ),                // XML_COPY_STYLES
        TOKEN( "corner-radius" ),              // XML_CORNER_RADIUS
        TOKEN( "correct" ),                    // XML_CORRECT
        TOKEN( "cos" ),                        // XML_COS
        TOKEN( "cosh" ),                       // XML_COSH
        TOKEN( "cot" ),                        // XML_COT
        TOKEN( "coth" ),                       // XML_COTH
        TOKEN( "count" ),                      // XML_COUNT
        TOKEN( "count-empty-lines" ),          // XML_COUNT_EMPTY_LINES
        TOKEN( "count-in-floating-frames" ),   // XML_COUNT_IN_FLOATING_FRAMES
        TOKEN( "counter-clockwise" ),          // XML_COUNTER_CLOCKWISE
        TOKEN( "counterclockwise" ),           // XML_COUNTERCLOCKWISE
        TOKEN( "countnums" ),                  // XML_COUNTNUMS
        TOKEN( "country" ),                    // XML_COUNTRY
        TOKEN( "country-asian" ),              // XML_COUNTRY_ASIAN
        TOKEN( "country-complex" ),            // XML_COUNTRY_COMPLEX
        TOKEN( "covered-table-cell" ),         // XML_COVERED_TABLE_CELL
        TOKEN( "create-date" ),                // XML_CREATE_DATE
        TOKEN( "create-date-string" ),         // XML_CREATE_DATE_STRING
        TOKEN( "creation-date" ),              // XML_CREATION_DATE
        TOKEN( "creation-time" ),              // XML_CREATION_TIME
        TOKEN( "creator" ),                    // XML_CREATOR
        TOKEN( "csc" ),                        // XML_CSC
        TOKEN( "csch" ),                       // XML_CSCH
        TOKEN( "cube" ),                       // XML_CUBE
        TOKEN( "cuboid" ),                     // XML_CUBOID
        TOKEN( "currency" ),                   // XML_CURRENCY
        TOKEN( "currency-style" ),             // XML_CURRENCY_STYLE
        TOKEN( "currency-symbol" ),            // XML_CURRENCY_SYMBOL
        TOKEN( "current" ),                    // XML_CURRENT
        TOKEN( "current-value" ),              // XML_CURRENT_VALUE
        TOKEN( "cursor-position" ),            // XML_CURSOR_POSITION
        TOKEN( "cursor-position-x" ),          // XML_CURSOR_POSITION_X
        TOKEN( "cursor-position-y" ),          // XML_CURSOR_POSITION_Y
        TOKEN( "curve" ),                      // XML_CURVE
        TOKEN( "custom1" ),                    // XML_CUSTOM1
        TOKEN( "custom2" ),                    // XML_CUSTOM2
        TOKEN( "custom3" ),                    // XML_CUSTOM3
        TOKEN( "custom4" ),                    // XML_CUSTOM4
        TOKEN( "custom5" ),                    // XML_CUSTOM5
        TOKEN( "cut" ),                        // XML_CUT
        TOKEN( "cut-offs" ),                   // XML_CUT_OFFS
        TOKEN( "cx" ),                         // XML_CX
        TOKEN( "cy" ),                         // XML_CY
        TOKEN( "cylinder" ),                   // XML_CYLINDER
        TOKEN( "d" ),                          // XML_D
        TOKEN( "dash" ),                       // XML_DASH
        TOKEN( "dashed" ),                     // XML_DASHED
        TOKEN( "data" ),                       // XML_DATA
        TOKEN( "data-cell-range-address" ),    // XML_DATA_CELL_RANGE_ADDRESS
        TOKEN( "data-label-number" ),          // XML_DATA_LABEL_NUMBER
        TOKEN( "data-label-symbol" ),          // XML_DATA_LABEL_SYMBOL
        TOKEN( "data-label-text" ),            // XML_DATA_LABEL_TEXT
        TOKEN( "data-pilot-field" ),           // XML_DATA_PILOT_FIELD
        TOKEN( "data-pilot-level" ),           // XML_DATA_PILOT_LEVEL
        TOKEN( "data-pilot-member" ),          // XML_DATA_PILOT_MEMBER
        TOKEN( "data-pilot-members" ),         // XML_DATA_PILOT_MEMBERS
        TOKEN( "data-pilot-subtotal" ),        // XML_DATA_PILOT_SUBTOTAL
        TOKEN( "data-pilot-subtotals" ),       // XML_DATA_PILOT_SUBTOTALS
        TOKEN( "data-pilot-table" ),           // XML_DATA_PILOT_TABLE
        TOKEN( "data-pilot-tables" ),          // XML_DATA_PILOT_TABLES
        TOKEN( "data-point" ),                 // XML_DATA_POINT
        TOKEN( "data-style" ),                 // XML_DATA_STYLE
        TOKEN( "data-style-name" ),            // XML_DATA_STYLE_NAME
        TOKEN( "data-type" ),                  // XML_DATA_TYPE
        TOKEN( "database-display" ),           // XML_DATABASE_DISPLAY
        TOKEN( "database-name" ),              // XML_DATABASE_NAME
        TOKEN( "database-next" ),              // XML_DATABASE_NEXT
        TOKEN( "database-range" ),             // XML_DATABASE_RANGE
        TOKEN( "database-ranges" ),            // XML_DATABASE_RANGES
        TOKEN( "database-row-number" ),        // XML_DATABASE_ROW_NUMBER
        TOKEN( "database-select" ),            // XML_DATABASE_SELECT
        TOKEN( "database-source-query" ),      // XML_DATABASE_SOURCE_QUERY
        TOKEN( "database-source-sql" ),        // XML_DATABASE_SOURCE_SQL
        TOKEN( "database-source-table" ),      // XML_DATABASE_SOURCE_TABLE
        TOKEN( "date" ),                       // XML_DATE
        TOKEN( "date-adjust" ),                // XML_DATE_ADJUST
        TOKEN( "date-style" ),                 // XML_DATE_STYLE
        TOKEN( "date-time" ),                  // XML_DATE_TIME
        TOKEN( "date-value" ),                 // XML_DATE_VALUE
        TOKEN( "datetime" ),                   // XML_DATETIME
        TOKEN( "day" ),                        // XML_DAY
        TOKEN( "day-of-week" ),                // XML_DAY_OF_WEEK
        TOKEN( "dde-application" ),            // XML_DDE_APPLICATION
        TOKEN( "dde-connection" ),             // XML_DDE_CONNECTION
        TOKEN( "dde-connection-decl" ),        // XML_DDE_CONNECTION_DECL
        TOKEN( "dde-connection-decls" ),       // XML_DDE_CONNECTION_DECLS
        TOKEN( "dde-item" ),                   // XML_DDE_ITEM
        TOKEN( "dde-link" ),                   // XML_DDE_LINK
        TOKEN( "dde-links" ),                  // XML_DDE_LINKS
        TOKEN( "dde-source" ),                 // XML_DDE_SOURCE
        TOKEN( "dde-topic" ),                  // XML_DDE_TOPIC
        TOKEN( "decimal-places" ),             // XML_DECIMAL_PLACES
        TOKEN( "decimal-replacement" ),        // XML_DECIMAL_REPLACEMENT
        TOKEN( "declare" ),                    // XML_DECLARE
        TOKEN( "decorate-words-only" ),        // XML_DECORATE_WORDS_ONLY
        TOKEN( "decorative" ),                 // XML_DECORATIVE
        TOKEN( "deep" ),                       // XML_DEEP
        TOKEN( "default" ),                    // XML_DEFAULT
        TOKEN( "default-cell-style-name" ),    // XML_DEFAULT_CELL_STYLE_NAME
        TOKEN( "default-style" ),              // XML_DEFAULT_STYLE
        TOKEN( "default-style-name" ),         // XML_DEFAULT_STYLE_NAME
        TOKEN( "degree" ),                     // XML_DEGREE
        TOKEN( "delay" ),                      // XML_DELAY
        TOKEN( "deletion" ),                   // XML_DELETION
        TOKEN( "deletions" ),                  // XML_DELETIONS
        TOKEN( "dependence" ),                 // XML_DEPENDENCE
        TOKEN( "dependences" ),                // XML_DEPENDENCES
        TOKEN( "dependencies" ),               // XML_DEPENDENCIES
        TOKEN( "depth" ),                      // XML_DEPTH
        TOKEN( "desc" ),                       // XML_DESC
        TOKEN( "descending" ),                 // XML_DESCENDING
        TOKEN( "description" ),                // XML_DESCRIPTION
        TOKEN( "detective" ),                  // XML_DETECTIVE
        TOKEN( "determinant" ),                // XML_DETERMINANT
        TOKEN( "diff" ),                       // XML_DIFF
        TOKEN( "diffuse-color" ),              // XML_DIFFUSE_COLOR
        TOKEN( "dim" ),                        // XML_DIM
        TOKEN( "direction" ),                  // XML_DIRECTION
        TOKEN( "disabled" ),                   // XML_DISABLED
        TOKEN( "disc" ),                       // XML_DISC
        TOKEN( "display" ),                    // XML_DISPLAY
        TOKEN( "display-border" ),             // XML_DISPLAY_BORDER
        TOKEN( "display-details" ),            // XML_DISPLAY_DETAILS
        TOKEN( "display-duplicates" ),         // XML_DISPLAY_DUPLICATES
        TOKEN( "display-empty" ),              // XML_DISPLAY_EMPTY
        TOKEN( "display-filter-buttons" ),     // XML_DISPLAY_FILTER_BUTTONS
        TOKEN( "display-formula" ),            // XML_DISPLAY_FORMULA
        TOKEN( "display-label" ),              // XML_DISPLAY_LABEL
        TOKEN( "display-levels" ),             // XML_DISPLAY_LEVELS
        TOKEN( "display-name" ),               // XML_DISPLAY_NAME
        TOKEN( "display-outline-level" ),      // XML_DISPLAY_OUTLINE_LEVEL
        TOKEN( "dissolve" ),                   // XML_DISSOLVE
        TOKEN( "distance" ),                   // XML_DISTANCE
        TOKEN( "distance-after-sep" ),         // XML_DISTANCE_AFTER_SEP
        TOKEN( "distance-before-sep" ),        // XML_DISTANCE_BEFORE_SEP
        TOKEN( "distribute-letter" ),          // XML_DISTRIBUTE_LETTER
        TOKEN( "distribute-space" ),           // XML_DISTRIBUTE_SPACE
        TOKEN( "divide" ),                     // XML_DIVIDE
        TOKEN( "document" ),                   // XML_DOCUMENT
        TOKEN( "document-content" ),           // XML_DOCUMENT_CONTENT
        TOKEN( "document-meta" ),              // XML_DOCUMENT_META
        TOKEN( "document-settings" ),          // XML_DOCUMENT_SETTINGS
        TOKEN( "document-statistic" ),         // XML_DOCUMENT_STATISTIC
        TOKEN( "document-styles" ),            // XML_DOCUMENT_STYLES
        TOKEN( "domain" ),                     // XML_DOMAIN
        TOKEN( "dot" ),                        // XML_DOT
        TOKEN( "dots1" ),                      // XML_DOTS1
        TOKEN( "dots1-length" ),               // XML_DOTS1_LENGTH
        TOKEN( "dots2" ),                      // XML_DOTS2
        TOKEN( "dots2-length" ),               // XML_DOTS2_LENGTH
        TOKEN( "dotted" ),                     // XML_DOTTED
        TOKEN( "double" ),                     // XML_DOUBLE
        TOKEN( "double-sided" ),               // XML_DOUBLE_SIDED
        TOKEN( "down" ),                       // XML_DOWN
        TOKEN( "draft" ),                      // XML_DRAFT
        TOKEN( "draw" ),                       // XML_DRAW
        TOKEN( "draw-aspect" ),                // XML_DRAW_ASPECT
        TOKEN( "drawing" ),                    // XML_DRAWING
        TOKEN( "drawings" ),                   // XML_DRAWINGS
        TOKEN( "drawpool" ),                   // XML_DRAWPOOL
        TOKEN( "drop-cap" ),                   // XML_DROP_CAP
        TOKEN( "dynamic" ),                    // XML_DYNAMIC
        TOKEN( "edge-rounding" ),              // XML_EDGE_ROUNDING
        TOKEN( "editable" ),                   // XML_EDITABLE
        TOKEN( "editing-cycles" ),             // XML_EDITING_CYCLES
        TOKEN( "editing-duration" ),           // XML_EDITING_DURATION
        TOKEN( "edition" ),                    // XML_EDITION
        TOKEN( "editor" ),                     // XML_EDITOR
        TOKEN( "effect" ),                     // XML_EFFECT
        TOKEN( "ellipse" ),                    // XML_ELLIPSE
        TOKEN( "email" ),                      // XML_EMAIL
        TOKEN( "embed" ),                      // XML_EMBED
        TOKEN( "embedded-visible-area" ),      // XML_EMBEDDED_VISIBLE_AREA
        TOKEN( "embossed" ),                   // XML_EMBOSSED
        TOKEN( "emissive-color" ),             // XML_EMISSIVE_COLOR
        TOKEN( "empty" ),                      // XML_EMPTY
        TOKEN( "enable-numbering" ),           // XML_ENABLE_NUMBERING
        TOKEN( "enabled" ),                    // XML_ENABLED
        TOKEN( "encoding" ),                   // XML_ENCODING
        TOKEN( "end" ),                        // XML_END
        TOKEN( "end-angle" ),                  // XML_END_ANGLE
        TOKEN( "end-cell-address" ),           // XML_END_CELL_ADDRESS
        TOKEN( "end-color" ),                  // XML_END_COLOR
        TOKEN( "end-column" ),                 // XML_END_COLUMN
        TOKEN( "end-glue-point" ),             // XML_END_GLUE_POINT
        TOKEN( "end-guide" ),                  // XML_END_GUIDE
        TOKEN( "end-intensity" ),              // XML_END_INTENSITY
        TOKEN( "end-line-spacing-horizontal" ),// XML_END_LINE_SPACING_HORIZONTAL
        TOKEN( "end-line-spacing-vertical" ),  // XML_END_LINE_SPACING_VERTICAL
        TOKEN( "end-position" ),               // XML_END_POSITION
        TOKEN( "end-row" ),                    // XML_END_ROW
        TOKEN( "end-shape" ),                  // XML_END_SHAPE
        TOKEN( "end-table" ),                  // XML_END_TABLE
        TOKEN( "end-x" ),                      // XML_END_X
        TOKEN( "end-y" ),                      // XML_END_Y
        TOKEN( "endless" ),                    // XML_ENDLESS
        TOKEN( "endnote" ),                    // XML_ENDNOTE
        TOKEN( "endnote-body" ),               // XML_ENDNOTE_BODY
        TOKEN( "endnote-citation" ),           // XML_ENDNOTE_CITATION
        TOKEN( "endnote-ref" ),                // XML_ENDNOTE_REF
        TOKEN( "endnotes-configuration" ),     // XML_ENDNOTES_CONFIGURATION
        TOKEN( "engraved" ),                   // XML_ENGRAVED
        TOKEN( "eq" ),                         // XML_EQ
        TOKEN( "equal-author" ),               // XML_EQUAL_AUTHOR
        TOKEN( "equal-comment" ),              // XML_EQUAL_COMMENT
        TOKEN( "equal-date" ),                 // XML_EQUAL_DATE
        TOKEN( "era" ),                        // XML_ERA
        TOKEN( "ergo-sum" ),                   // XML_ERGO_SUM
        TOKEN( "error-category" ),             // XML_ERROR_CATEGORY
        TOKEN( "error-lower-indicator" ),      // XML_ERROR_LOWER_INDICATOR
        TOKEN( "error-lower-limit" ),          // XML_ERROR_LOWER_LIMIT
        TOKEN( "error-macro" ),                // XML_ERROR_MACRO
        TOKEN( "error-margin" ),               // XML_ERROR_MARGIN
        TOKEN( "error-message" ),              // XML_ERROR_MESSAGE
        TOKEN( "error-percentage" ),           // XML_ERROR_PERCENTAGE
        TOKEN( "error-upper-indicator" ),      // XML_ERROR_UPPER_INDICATOR
        TOKEN( "error-upper-limit" ),          // XML_ERROR_UPPER_LIMIT
        TOKEN( "sub" ),                        // XML_ESCAPEMENT_SUB
        TOKEN( "super" ),                      // XML_ESCAPEMENT_SUPER
        TOKEN( "even-page" ),                  // XML_EVEN_PAGE
        TOKEN( "event" ),                      // XML_EVENT
        TOKEN( "event-name" ),                 // XML_EVENT_NAME
        TOKEN( "events" ),                     // XML_EVENTS
        TOKEN( "execute" ),                    // XML_EXECUTE
        TOKEN( "execute-macro" ),              // XML_EXECUTE_MACRO
        TOKEN( "exists" ),                     // XML_EXISTS
        TOKEN( "exp" ),                        // XML_EXP
        TOKEN( "exponential" ),                // XML_EXPONENTIAL
        TOKEN( "expression" ),                 // XML_EXPRESSION
        TOKEN( "extra" ),                      // XML_EXTRA
        TOKEN( "extrude" ),                    // XML_EXTRUDE
        TOKEN( "factorial" ),                  // XML_FACTORIAL
        TOKEN( "fade" ),                       // XML_FADE
        TOKEN( "fade-from-bottom" ),           // XML_FADE_FROM_BOTTOM
        TOKEN( "fade-from-center" ),           // XML_FADE_FROM_CENTER
        TOKEN( "fade-from-left" ),             // XML_FADE_FROM_LEFT
        TOKEN( "fade-from-lowerleft" ),        // XML_FADE_FROM_LOWERLEFT
        TOKEN( "fade-from-lowerright" ),       // XML_FADE_FROM_LOWERRIGHT
        TOKEN( "fade-from-right" ),            // XML_FADE_FROM_RIGHT
        TOKEN( "fade-from-top" ),              // XML_FADE_FROM_TOP
        TOKEN( "fade-from-upperleft" ),        // XML_FADE_FROM_UPPERLEFT
        TOKEN( "fade-from-upperright" ),       // XML_FADE_FROM_UPPERRIGHT
        TOKEN( "fade-out" ),                   // XML_FADE_OUT
        TOKEN( "fade-to-center" ),             // XML_FADE_TO_CENTER
        TOKEN( "false" ),                      // XML_FALSE
        TOKEN( "family" ),                     // XML_FAMILY
        TOKEN( "fast" ),                       // XML_FAST
        TOKEN( "field-number" ),               // XML_FIELD_NUMBER
        TOKEN( "file-name" ),                  // XML_FILE_NAME
        TOKEN( "fill" ),                       // XML_FILL
        TOKEN( "fill-color" ),                 // XML_FILL_COLOR
        TOKEN( "fill-gradient-name" ),         // XML_FILL_GRADIENT_NAME
        TOKEN( "fill-hatch-name" ),            // XML_FILL_HATCH_NAME
        TOKEN( "fill-hatch-solid" ),           // XML_FILL_HATCH_SOLID
        TOKEN( "fill-image" ),                 // XML_FILL_IMAGE
        TOKEN( "fill-image-height" ),          // XML_FILL_IMAGE_HEIGHT
        TOKEN( "fill-image-name" ),            // XML_FILL_IMAGE_NAME
        TOKEN( "fill-image-ref-point" ),       // XML_FILL_IMAGE_REF_POINT
        TOKEN( "fill-image-ref-point-x" ),     // XML_FILL_IMAGE_REF_POINT_X
        TOKEN( "fill-image-ref-point-y" ),     // XML_FILL_IMAGE_REF_POINT_Y
        TOKEN( "fill-image-width" ),           // XML_FILL_IMAGE_WIDTH
        TOKEN( "filter" ),                     // XML_FILTER
        TOKEN( "filter-and" ),                 // XML_FILTER_AND
        TOKEN( "filter-condition" ),           // XML_FILTER_CONDITION
        TOKEN( "filter-name" ),                // XML_FILTER_NAME
        TOKEN( "filter-options" ),             // XML_FILTER_OPTIONS
        TOKEN( "filter-or" ),                  // XML_FILTER_OR
        TOKEN( "first-date-time" ),            // XML_FIRST_DATE_TIME
        TOKEN( "first-page" ),                 // XML_FIRST_PAGE
        TOKEN( "first-page-number" ),          // XML_FIRST_PAGE_NUMBER
        TOKEN( "fit-to-contour" ),             // XML_FIT_TO_CONTOUR
        TOKEN( "fit-to-size" ),                // XML_FIT_TO_SIZE
        TOKEN( "fix" ),                        // XML_FIX
        TOKEN( "fixed" ),                      // XML_FIXED
        TOKEN( "flat" ),                       // XML_FLAT
        TOKEN( "float" ),                      // XML_FLOAT
        TOKEN( "floating-frame" ),             // XML_FLOATING_FRAME
        TOKEN( "floor" ),                      // XML_FLOOR
        TOKEN( "fn" ),                         // XML_FN
        TOKEN( "focal-length" ),               // XML_FOCAL_LENGTH
        TOKEN( "font-char-width" ),            // XML_FONT_CHAR_WIDTH
        TOKEN( "font-charset" ),               // XML_FONT_CHARSET
        TOKEN( "font-charset-asian" ),         // XML_FONT_CHARSET_ASIAN
        TOKEN( "font-charset-complex" ),       // XML_FONT_CHARSET_COMPLEX
        TOKEN( "font-color" ),                 // XML_FONT_COLOR
        TOKEN( "font-decl" ),                  // XML_FONT_DECL
        TOKEN( "font-decls" ),                 // XML_FONT_DECLS
        TOKEN( "font-family" ),                // XML_FONT_FAMILY
        TOKEN( "font-family-asian" ),          // XML_FONT_FAMILY_ASIAN
        TOKEN( "font-family-complex" ),        // XML_FONT_FAMILY_COMPLEX
        TOKEN( "font-family-generic" ),        // XML_FONT_FAMILY_GENERIC
        TOKEN( "font-family-generic-asian" ),  // XML_FONT_FAMILY_GENERIC_ASIAN
        TOKEN( "font-family-generic-complex" ),// XML_FONT_FAMILY_GENERIC_COMPLEX
        TOKEN( "font-kerning" ),               // XML_FONT_KERNING
        TOKEN( "font-name" ),                  // XML_FONT_NAME
        TOKEN( "font-name-asian" ),            // XML_FONT_NAME_ASIAN
        TOKEN( "font-name-complex" ),          // XML_FONT_NAME_COMPLEX
        TOKEN( "font-pitch" ),                 // XML_FONT_PITCH
        TOKEN( "font-pitch-asian" ),           // XML_FONT_PITCH_ASIAN
        TOKEN( "font-pitch-complex" ),         // XML_FONT_PITCH_COMPLEX
        TOKEN( "font-relief" ),                // XML_FONT_RELIEF
        TOKEN( "font-size" ),                  // XML_FONT_SIZE
        TOKEN( "font-size-asian" ),            // XML_FONT_SIZE_ASIAN
        TOKEN( "font-size-complex" ),          // XML_FONT_SIZE_COMPLEX
        TOKEN( "font-size-rel" ),              // XML_FONT_SIZE_REL
        TOKEN( "font-size-rel-asian" ),        // XML_FONT_SIZE_REL_ASIAN
        TOKEN( "font-size-rel-complex" ),      // XML_FONT_SIZE_REL_COMPLEX
        TOKEN( "font-style" ),                 // XML_FONT_STYLE
        TOKEN( "font-style-asian" ),           // XML_FONT_STYLE_ASIAN
        TOKEN( "font-style-complex" ),         // XML_FONT_STYLE_COMPLEX
        TOKEN( "font-style-name" ),            // XML_FONT_STYLE_NAME
        TOKEN( "font-style-name-asian" ),      // XML_FONT_STYLE_NAME_ASIAN
        TOKEN( "font-style-name-complex" ),    // XML_FONT_STYLE_NAME_COMPLEX
        TOKEN( "font-variant" ),               // XML_FONT_VARIANT
        TOKEN( "font-weight" ),                // XML_FONT_WEIGHT
        TOKEN( "font-weight-asian" ),          // XML_FONT_WEIGHT_ASIAN
        TOKEN( "font-weight-complex" ),        // XML_FONT_WEIGHT_COMPLEX
        TOKEN( "font-width" ),                 // XML_FONT_WIDTH
        TOKEN( "font-word-line-mode" ),        // XML_FONT_WORD_LINE_MODE
        TOKEN( "fontfamily" ),                 // XML_FONTFAMILY
        TOKEN( "fontsize" ),                   // XML_FONTSIZE
        TOKEN( "fontstyle" ),                  // XML_FONTSTYLE
        TOKEN( "fontweight" ),                 // XML_FONTWEIGHT
        TOKEN( "fontwork-adjust" ),            // XML_FONTWORK_ADJUST
        TOKEN( "fontwork-distance" ),          // XML_FONTWORK_DISTANCE
        TOKEN( "fontwork-form" ),              // XML_FONTWORK_FORM
        TOKEN( "fontwork-hide-form" ),         // XML_FONTWORK_HIDE_FORM
        TOKEN( "fontwork-mirror" ),            // XML_FONTWORK_MIRROR
        TOKEN( "fontwork-outline" ),           // XML_FONTWORK_OUTLINE
        TOKEN( "fontwork-shadow" ),            // XML_FONTWORK_SHADOW
        TOKEN( "fontwork-shadow-color" ),      // XML_FONTWORK_SHADOW_COLOR
        TOKEN( "fontwork-shadow-offset-x" ),   // XML_FONTWORK_SHADOW_OFFSET_X
        TOKEN( "fontwork-shadow-offset-y" ),   // XML_FONTWORK_SHADOW_OFFSET_Y
        TOKEN( "fontwork-shadow-transparence" ),   // XML_FONTWORK_SHADOW_TRANSPARENCE
        TOKEN( "fontwork-start" ),             // XML_FONTWORK_START
        TOKEN( "fontwork-style" ),             // XML_FONTWORK_STYLE
        TOKEN( "footer" ),                     // XML_FOOTER
        TOKEN( "footer-left" ),                // XML_FOOTER_LEFT
        TOKEN( "footer-style" ),               // XML_FOOTER_STYLE
        TOKEN( "footnote" ),                   // XML_FOOTNOTE
        TOKEN( "footnote-body" ),              // XML_FOOTNOTE_BODY
        TOKEN( "footnote-citation" ),          // XML_FOOTNOTE_CITATION
        TOKEN( "footnote-continuation-notice-backward" ),  // XML_FOOTNOTE_CONTINUATION_NOTICE_BACKWARD
        TOKEN( "footnote-continuation-notice-forward" ),   // XML_FOOTNOTE_CONTINUATION_NOTICE_FORWARD
        TOKEN( "footnote-max-height" ),        // XML_FOOTNOTE_MAX_HEIGHT
        TOKEN( "footnote-ref" ),               // XML_FOOTNOTE_REF
        TOKEN( "footnote-sep" ),               // XML_FOOTNOTE_SEP
        TOKEN( "footnotes-configuration" ),    // XML_FOOTNOTES_CONFIGURATION
        TOKEN( "footnotes-position" ),         // XML_FOOTNOTES_POSITION
        TOKEN( "forall" ),                     // XML_FORALL
        TOKEN( "force-manual" ),               // XML_FORCE_MANUAL
        TOKEN( "foreground" ),                 // XML_FOREGROUND
        TOKEN( "foreign-object" ),             // XML_FOREIGN_OBJECT
        TOKEN( "format-change" ),              // XML_FORMAT_CHANGE
        TOKEN( "format-source" ),              // XML_FORMAT_SOURCE
        TOKEN( "forms" ),                      // XML_FORMS
        TOKEN( "formula" ),                    // XML_FORMULA
        TOKEN( "formula-hidden" ),             // XML_FORMULA_HIDDEN
        TOKEN( "formulas" ),                   // XML_FORMULAS
        TOKEN( "fraction" ),                   // XML_FRACTION
        TOKEN( "frame" ),                      // XML_FRAME
        TOKEN( "frame-content" ),              // XML_FRAME_CONTENT
        TOKEN( "frame-display-border" ),       // XML_FRAME_DISPLAY_BORDER
        TOKEN( "frame-display-scrollbar" ),    // XML_FRAME_DISPLAY_SCROLLBAR
        TOKEN( "frame-end-margin" ),           // XML_FRAME_END_MARGIN
        TOKEN( "frame-margin-horizontal" ),    // XML_FRAME_MARGIN_HORIZONTAL
        TOKEN( "frame-margin-vertical" ),      // XML_FRAME_MARGIN_VERTICAL
        TOKEN( "frame-name" ),                 // XML_FRAME_NAME
        TOKEN( "frame-start-margin" ),         // XML_FRAME_START_MARGIN
        TOKEN( "freeze" ),                     // XML_FREEZE
        TOKEN( "freeze-position" ),            // XML_FREEZE_POSITION
        TOKEN( "from-another-table" ),         // XML_FROM_ANOTHER_TABLE
        TOKEN( "from-bottom" ),                // XML_FROM_BOTTOM
        TOKEN( "from-center" ),                // XML_FROM_CENTER
        TOKEN( "from-inside" ),                // XML_FROM_INSIDE
        TOKEN( "from-left" ),                  // XML_FROM_LEFT
        TOKEN( "from-lower-left" ),            // XML_FROM_LOWER_LEFT
        TOKEN( "from-lower-right" ),           // XML_FROM_LOWER_RIGHT
        TOKEN( "from-right" ),                 // XML_FROM_RIGHT
        TOKEN( "from-same-table" ),            // XML_FROM_SAME_TABLE
        TOKEN( "from-top" ),                   // XML_FROM_TOP
        TOKEN( "from-upper-left" ),            // XML_FROM_UPPER_LEFT
        TOKEN( "from-upper-right" ),           // XML_FROM_UPPER_RIGHT
        TOKEN( "fuchsia" ),                    // XML_FUCHSIA
        TOKEN( "full" ),                       // XML_FULL
        TOKEN( "full-screen" ),                // XML_FULL_SCREEN
        TOKEN( "function" ),                   // XML_FUNCTION
        TOKEN( "g" ),                          // XML_G
        TOKEN( "gamma" ),                      // XML_GAMMA
        TOKEN( "gap" ),                        // XML_GAP
        TOKEN( "gap-width" ),                  // XML_GAP_WIDTH
        TOKEN( "gcd" ),                        // XML_GCD
        TOKEN( "generator" ),                  // XML_GENERATOR
        TOKEN( "geq" ),                        // XML_GEQ
        TOKEN( "gouraud" ),                    // XML_GOURAUD
        TOKEN( "gradient" ),                   // XML_GRADIENT
        TOKEN( "angle" ),                      // XML_GRADIENT_ANGLE
        TOKEN( "border" ),                     // XML_GRADIENT_BORDER
        TOKEN( "gradient-step-count" ),        // XML_GRADIENT_STEP_COUNT
        TOKEN( "gradient-style" ),             // XML_GRADIENT_STYLE
        TOKEN( "axial" ),                      // XML_GRADIENTSTYLE_AXIAL
        TOKEN( "ellipsoid" ),                  // XML_GRADIENTSTYLE_ELLIPSOID
        TOKEN( "linear" ),                     // XML_GRADIENTSTYLE_LINEAR
        TOKEN( "radial" ),                     // XML_GRADIENTSTYLE_RADIAL
        TOKEN( "rectangular" ),                // XML_GRADIENTSTYLE_RECTANGULAR
        TOKEN( "square" ),                     // XML_GRADIENTSTYLE_SQUARE
        TOKEN( "grand-total" ),                // XML_GRAND_TOTAL
        TOKEN( "graphic" ),                    // XML_GRAPHIC
        TOKEN( "gray" ),                       // XML_GRAY
        TOKEN( "green" ),                      // XML_GREEN
        TOKEN( "greyscale" ),                  // XML_GREYSCALE
        TOKEN( "grid" ),                       // XML_GRID
        TOKEN( "groove" ),                     // XML_GROOVE
        TOKEN( "group-by-field-number" ),      // XML_GROUP_BY_FIELD_NUMBER
        TOKEN( "group-name" ),                 // XML_GROUP_NAME
        TOKEN( "grouping" ),                   // XML_GROUPING
        TOKEN( "gt" ),                         // XML_GT
        TOKEN( "guide-distance" ),             // XML_GUIDE_DISTANCE
        TOKEN( "guide-overhang" ),             // XML_GUIDE_OVERHANG
        TOKEN( "h" ),                          // XML_H
        TOKEN( "hanging" ),                    // XML_HANGING
        TOKEN( "has-persistent-data" ),        // XML_HAS_PERSISTENT_DATA
        TOKEN( "hatch" ),                      // XML_HATCH
        TOKEN( "distance" ),                   // XML_HATCH_DISTANCE
        TOKEN( "style" ),                      // XML_HATCH_STYLE
        TOKEN( "double" ),                     // XML_HATCHSTYLE_DOUBLE
        TOKEN( "single" ),                     // XML_HATCHSTYLE_SINGLE
        TOKEN( "triple" ),                     // XML_HATCHSTYLE_TRIPLE
        TOKEN( "header" ),                     // XML_HEADER
        TOKEN( "header-left" ),                // XML_HEADER_LEFT
        TOKEN( "header-style" ),               // XML_HEADER_STYLE
        TOKEN( "headers" ),                    // XML_HEADERS
        TOKEN( "height" ),                     // XML_HEIGHT
        TOKEN( "help-file-name" ),             // XML_HELP_FILE_NAME
        TOKEN( "help-id" ),                    // XML_HELP_ID
        TOKEN( "help-message" ),               // XML_HELP_MESSAGE
        TOKEN( "hidden" ),                     // XML_HIDDEN
        TOKEN( "hidden-and-protected" ),       // XML_HIDDEN_AND_PROTECTED
        TOKEN( "hidden-paragraph" ),           // XML_HIDDEN_PARAGRAPH
        TOKEN( "hidden-text" ),                // XML_HIDDEN_TEXT
        TOKEN( "hide" ),                       // XML_HIDE
        TOKEN( "hide-shape" ),                 // XML_HIDE_SHAPE
        TOKEN( "hide-text" ),                  // XML_HIDE_TEXT
        TOKEN( "highlighted-range" ),          // XML_HIGHLIGHTED_RANGE
        TOKEN( "horizontal" ),                 // XML_HORIZONTAL
        TOKEN( "horizontal-lines" ),           // XML_HORIZONTAL_LINES
        TOKEN( "horizontal-on-left-pages" ),   // XML_HORIZONTAL_ON_LEFT_PAGES
        TOKEN( "horizontal-on-right-pages" ),  // XML_HORIZONTAL_ON_RIGHT_PAGES
        TOKEN( "horizontal-pos" ),             // XML_HORIZONTAL_POS
        TOKEN( "horizontal-rel" ),             // XML_HORIZONTAL_REL
        TOKEN( "horizontal-scrollbar-width" ), // XML_HORIZONTAL_SCROLLBAR_WIDTH
        TOKEN( "horizontal-segments" ),        // XML_HORIZONTAL_SEGMENTS
        TOKEN( "horizontal-split-mode" ),      // XML_HORIZONTAL_SPLIT_MODE
        TOKEN( "horizontal-split-position" ),  // XML_HORIZONTAL_SPLIT_POSITION
        TOKEN( "horizontal-stripes" ),         // XML_HORIZONTAL_STRIPES
        TOKEN( "hours" ),                      // XML_HOURS
        TOKEN( "howpublished" ),               // XML_HOWPUBLISHED
        TOKEN( "href" ),                       // XML_HREF
        TOKEN( "html" ),                       // XML_HTML
        TOKEN( "hyperlink-behaviour" ),        // XML_HYPERLINK_BEHAVIOUR
        TOKEN( "hyphenate" ),                  // XML_HYPHENATE
        TOKEN( "hyphenation-keep" ),           // XML_HYPHENATION_KEEP
        TOKEN( "hyphenation-ladder-count" ),   // XML_HYPHENATION_LADDER_COUNT
        TOKEN( "hyphenation-push-char-count" ),// XML_HYPHENATION_PUSH_CHAR_COUNT
        TOKEN( "hyphenation-remain-char-count" ),  // XML_HYPHENATION_REMAIN_CHAR_COUNT
        TOKEN( "i" ),                          // XML_I
        TOKEN( "icon" ),                       // XML_ICON
        TOKEN( "id" ),                         // XML_ID
        TOKEN( "ident" ),                      // XML_IDENT
        TOKEN( "identifier" ),                 // XML_IDENTIFIER
        TOKEN( "identify-categories" ),        // XML_IDENTIFY_CATEGORIES
        TOKEN( "ideograph-alpha" ),            // XML_IDEOGRAPH_ALPHA
        TOKEN( "ignore-case" ),                // XML_IGNORE_CASE
        TOKEN( "ignore-empty-rows" ),          // XML_IGNORE_EMPTY_ROWS
        TOKEN( "illustration-index" ),         // XML_ILLUSTRATION_INDEX
        TOKEN( "illustration-index-entry-template" ),  // XML_ILLUSTRATION_INDEX_ENTRY_TEMPLATE
        TOKEN( "illustration-index-source" ),  // XML_ILLUSTRATION_INDEX_SOURCE
        TOKEN( "image" ),                      // XML_IMAGE
        TOKEN( "image-count" ),                // XML_IMAGE_COUNT
        TOKEN( "image-map" ),                  // XML_IMAGE_MAP
        TOKEN( "implies" ),                    // XML_IMPLIES
        TOKEN( "in" ),                         // XML_IN
        TOKEN( "in-range" ),                   // XML_IN_RANGE
        TOKEN( "inbook" ),                     // XML_INBOOK
        TOKEN( "incollection" ),               // XML_INCOLLECTION
        TOKEN( "increment" ),                  // XML_INCREMENT
        TOKEN( "index" ),                      // XML_INDEX
        TOKEN( "index-body" ),                 // XML_INDEX_BODY
        TOKEN( "index-entry-bibliography" ),   // XML_INDEX_ENTRY_BIBLIOGRAPHY
        TOKEN( "index-entry-chapter" ),        // XML_INDEX_ENTRY_CHAPTER
        TOKEN( "index-entry-chapter-number" ), // XML_INDEX_ENTRY_CHAPTER_NUMBER
        TOKEN( "index-entry-link-end" ),       // XML_INDEX_ENTRY_LINK_END
        TOKEN( "index-entry-link-start" ),     // XML_INDEX_ENTRY_LINK_START
        TOKEN( "index-entry-page-number" ),    // XML_INDEX_ENTRY_PAGE_NUMBER
        TOKEN( "index-entry-span" ),           // XML_INDEX_ENTRY_SPAN
        TOKEN( "index-entry-tab-stop" ),       // XML_INDEX_ENTRY_TAB_STOP
        TOKEN( "index-entry-template" ),       // XML_INDEX_ENTRY_TEMPLATE
        TOKEN( "index-entry-text" ),           // XML_INDEX_ENTRY_TEXT
        TOKEN( "index-name" ),                 // XML_INDEX_NAME
        TOKEN( "index-scope" ),                // XML_INDEX_SCOPE
        TOKEN( "index-source-style" ),         // XML_INDEX_SOURCE_STYLE
        TOKEN( "index-source-styles" ),        // XML_INDEX_SOURCE_STYLES
        TOKEN( "index-title" ),                // XML_INDEX_TITLE
        TOKEN( "index-title-template" ),       // XML_INDEX_TITLE_TEMPLATE
        TOKEN( "information" ),                // XML_INFORMATION
        TOKEN( "initial-creator" ),            // XML_INITIAL_CREATOR
        TOKEN( "inproceedings" ),              // XML_INPROCEEDINGS
        TOKEN( "insertion" ),                  // XML_INSERTION
        TOKEN( "insertion-cut-off" ),          // XML_INSERTION_CUT_OFF
        TOKEN( "inset" ),                      // XML_INSET
        TOKEN( "inside" ),                     // XML_INSIDE
        TOKEN( "institution" ),                // XML_INSTITUTION
        TOKEN( "int" ),                        // XML_INT
        TOKEN( "intensity" ),                  // XML_INTENSITY
        TOKEN( "intersect" ),                  // XML_INTERSECT
        TOKEN( "interval" ),                   // XML_INTERVAL
        TOKEN( "interval-major" ),             // XML_INTERVAL_MAJOR
        TOKEN( "interval-minor" ),             // XML_INTERVAL_MINOR
        TOKEN( "into-english-number" ),        // XML_INTO_ENGLISH_NUMBER
        TOKEN( "inverse" ),                    // XML_INVERSE
        TOKEN( "is-active" ),                  // XML_IS_ACTIVE
        TOKEN( "is-data-layout-field" ),       // XML_IS_DATA_LAYOUT_FIELD
        TOKEN( "is-hidden" ),                  // XML_IS_HIDDEN
        TOKEN( "is-selection" ),               // XML_IS_SELECTION
        TOKEN( "isbn" ),                       // XML_ISBN
        TOKEN( "italic" ),                     // XML_ITALIC
        TOKEN( "iteration" ),                  // XML_ITERATION
        TOKEN( "journal" ),                    // XML_JOURNAL
        TOKEN( "justified" ),                  // XML_JUSTIFIED
        TOKEN( "justify" ),                    // XML_JUSTIFY
        TOKEN( "justify-single-word" ),        // XML_JUSTIFY_SINGLE_WORD
        TOKEN( "keep-with-next" ),             // XML_KEEP_WITH_NEXT
        TOKEN( "normal" ),                     // XML_KERNING_NORMAL
        TOKEN( "key" ),                        // XML_KEY
        TOKEN( "key1" ),                       // XML_KEY1
        TOKEN( "key2" ),                       // XML_KEY2
        TOKEN( "keyword" ),                    // XML_KEYWORD
        TOKEN( "keywords" ),                   // XML_KEYWORDS
        TOKEN( "kind" ),                       // XML_KIND
        TOKEN( "km" ),                         // XML_KM
        TOKEN( "label" ),                      // XML_LABEL
        TOKEN( "label-arrangement" ),          // XML_LABEL_ARRANGEMENT
        TOKEN( "label-cell-address" ),         // XML_LABEL_CELL_ADDRESS
        TOKEN( "label-cell-range-address" ),   // XML_LABEL_CELL_RANGE_ADDRESS
        TOKEN( "label-range" ),                // XML_LABEL_RANGE
        TOKEN( "label-ranges" ),               // XML_LABEL_RANGES
        TOKEN( "lambda" ),                     // XML_LAMBDA
        TOKEN( "landscape" ),                  // XML_LANDSCAPE
        TOKEN( "language" ),                   // XML_LANGUAGE
        TOKEN( "language-asian" ),             // XML_LANGUAGE_ASIAN
        TOKEN( "language-complex" ),           // XML_LANGUAGE_COMPLEX
        TOKEN( "laser" ),                      // XML_LASER
        TOKEN( "last-column-spanned" ),        // XML_LAST_COLUMN_SPANNED
        TOKEN( "last-page" ),                  // XML_LAST_PAGE
        TOKEN( "last-row-spanned" ),           // XML_LAST_ROW_SPANNED
        TOKEN( "layer" ),                      // XML_LAYER
        TOKEN( "layer-set" ),                  // XML_LAYER_SET
        TOKEN( "leader-char" ),                // XML_LEADER_CHAR
        TOKEN( "left" ),                       // XML_LEFT
        TOKEN( "left-outside" ),               // XML_LEFT_OUTSIDE
        TOKEN( "left-top-position" ),          // XML_LEFT_TOP_POSITION
        TOKEN( "left-arc" ),                   // XML_LEFTARC
        TOKEN( "left-circle" ),                // XML_LEFTCIRCLE
        TOKEN( "legend" ),                     // XML_LEGEND
        TOKEN( "legend-position" ),            // XML_LEGEND_POSITION
        TOKEN( "length" ),                     // XML_LENGTH
        TOKEN( "leq" ),                        // XML_LEQ
        TOKEN( "let-text" ),                   // XML_LET_TEXT
        TOKEN( "keep-text" ),                  // XML_KEEP_TEXT
        TOKEN( "letter-kerning" ),             // XML_LETTER_KERNING
        TOKEN( "letter-spacing" ),             // XML_LETTER_SPACING
        TOKEN( "letters" ),                    // XML_LETTERS
        TOKEN( "level" ),                      // XML_LEVEL
        TOKEN( "library" ),                    // XML_LIBRARY
        TOKEN( "library-embedded" ),           // XML_LIBRARY_EMBEDDED
        TOKEN( "library-linked" ),             // XML_LIBRARY_LINKED
        TOKEN( "light" ),                      // XML_LIGHT
        TOKEN( "lighting-mode" ),              // XML_LIGHTING_MODE
        TOKEN( "lime" ),                       // XML_LIME
        TOKEN( "limit" ),                      // XML_LIMIT
        TOKEN( "line" ),                       // XML_LINE
        TOKEN( "line-break" ),                 // XML_LINE_BREAK
        TOKEN( "line-distance" ),              // XML_LINE_DISTANCE
        TOKEN( "line-height" ),                // XML_LINE_HEIGHT
        TOKEN( "line-height-at-least" ),       // XML_LINE_HEIGHT_AT_LEAST
        TOKEN( "line-number" ),                // XML_LINE_NUMBER
        TOKEN( "line-skew" ),                  // XML_LINE_SKEW
        TOKEN( "line-spacing" ),               // XML_LINE_SPACING
        TOKEN( "linear" ),                     // XML_LINEAR
        TOKEN( "linenumbering-configuration" ),// XML_LINENUMBERING_CONFIGURATION
        TOKEN( "linenumbering-separator" ),    // XML_LINENUMBERING_SEPARATOR
        TOKEN( "lines" ),                      // XML_LINES
        TOKEN( "lines-used" ),                 // XML_LINES_USED
        TOKEN( "link-to-source-data" ),        // XML_LINK_TO_SOURCE_DATA
        TOKEN( "list" ),                       // XML_LIST
        TOKEN( "list-block" ),                 // XML_LIST_BLOCK
        TOKEN( "list-header" ),                // XML_LIST_HEADER
        TOKEN( "list-info" ),                  // XML_LIST_INFO
        TOKEN( "list-item" ),                  // XML_LIST_ITEM
        TOKEN( "list-level" ),                 // XML_LIST_LEVEL
        TOKEN( "list-level-style-bullet" ),    // XML_LIST_LEVEL_STYLE_BULLET
        TOKEN( "list-level-style-image" ),     // XML_LIST_LEVEL_STYLE_IMAGE
        TOKEN( "list-level-style-number" ),    // XML_LIST_LEVEL_STYLE_NUMBER
        TOKEN( "list-name" ),                  // XML_LIST_NAME
        TOKEN( "list-style" ),                 // XML_LIST_STYLE
        TOKEN( "list-style-name" ),            // XML_LIST_STYLE_NAME
        TOKEN( "ln" ),                         // XML_LN
        TOKEN( "locked" ),                     // XML_LOCKED
        TOKEN( "log" ),                        // XML_LOG
        TOKEN( "logarithmic" ),                // XML_LOGARITHMIC
        TOKEN( "logbase" ),                    // XML_LOGBASE
        TOKEN( "long" ),                       // XML_LONG
        TOKEN( "lowlimit" ),                   // XML_LOWLIMIT
        TOKEN( "lr-tb" ),                      // XML_LR_TB
        TOKEN( "lt" ),                         // XML_LT
        TOKEN( "ltr" ),                        // XML_LTR
        TOKEN( "luminance" ),                  // XML_LUMINANCE
        TOKEN( "macro-name" ),                 // XML_MACRO_NAME
        TOKEN( "maction" ),                    // XML_MACTION
        TOKEN( "main-entry-style-name" ),      // XML_MAIN_ENTRY_STYLE_NAME
        TOKEN( "major" ),                      // XML_MAJOR
        TOKEN( "maligngroup" ),                // XML_MALIGNGROUP
        TOKEN( "malignmark" ),                 // XML_MALIGNMARK
        TOKEN( "manual" ),                     // XML_MANUAL
        TOKEN( "map" ),                        // XML_MAP
        TOKEN( "margin-bottom" ),              // XML_MARGIN_BOTTOM
        TOKEN( "margin-left" ),                // XML_MARGIN_LEFT
        TOKEN( "margin-right" ),               // XML_MARGIN_RIGHT
        TOKEN( "margin-top" ),                 // XML_MARGIN_TOP
        TOKEN( "margins" ),                    // XML_MARGINS
        TOKEN( "marker" ),                     // XML_MARKER
        TOKEN( "marker-end" ),                 // XML_MARKER_END
        TOKEN( "marker-end-center" ),          // XML_MARKER_END_CENTER
        TOKEN( "marker-end-width" ),           // XML_MARKER_END_WIDTH
        TOKEN( "marker-start" ),               // XML_MARKER_START
        TOKEN( "marker-start-center" ),        // XML_MARKER_START_CENTER
        TOKEN( "marker-start-width" ),         // XML_MARKER_START_WIDTH
        TOKEN( "maroon" ),                     // XML_MAROON
        TOKEN( "master-page" ),                // XML_MASTER_PAGE
        TOKEN( "master-page-name" ),           // XML_MASTER_PAGE_NAME
        TOKEN( "master-styles" ),              // XML_MASTER_STYLES
        TOKEN( "mastersthesis" ),              // XML_MASTERSTHESIS
        TOKEN( "match" ),                      // XML_MATCH
        TOKEN( "math" ),                       // XML_MATH
        TOKEN( "matrix" ),                     // XML_MATRIX
        TOKEN( "matrix-covered" ),             // XML_MATRIX_COVERED
        TOKEN( "matrixrow" ),                  // XML_MATRIXROW
        TOKEN( "max" ),                        // XML_MAX
        TOKEN( "max-edge" ),                   // XML_MAX_EDGE
        TOKEN( "max-height" ),                 // XML_MAX_HEIGHT
        TOKEN( "max-width" ),                  // XML_MAX_WIDTH
        TOKEN( "maximum" ),                    // XML_MAXIMUM
        TOKEN( "maximum-difference" ),         // XML_MAXIMUM_DIFFERENCE
        TOKEN( "may-break-between-rows" ),     // XML_MAY_BREAK_BETWEEN_ROWS
        TOKEN( "may-script" ),                 // XML_MAY_SCRIPT
        TOKEN( "mean" ),                       // XML_MEAN
        TOKEN( "mean-value" ),                 // XML_MEAN_VALUE
        TOKEN( "measure" ),                    // XML_MEASURE
        TOKEN( "measure-align" ),              // XML_MEASURE_ALIGN
        TOKEN( "measure-vertical-align" ),     // XML_MEASURE_VERTICAL_ALIGN
        TOKEN( "median" ),                     // XML_MEDIAN
        TOKEN( "medium" ),                     // XML_MEDIUM
        TOKEN( "merror" ),                     // XML_MERROR
        TOKEN( "message-type" ),               // XML_MESSAGE_TYPE
        TOKEN( "meta" ),                       // XML_META
        TOKEN( "mfenced" ),                    // XML_MFENCED
        TOKEN( "mfrac" ),                      // XML_MFRAC
        TOKEN( "mi" ),                         // XML_MI
        TOKEN( "middle" ),                     // XML_MIDDLE
        TOKEN( "mime-type" ),                  // XML_MIME_TYPE
        TOKEN( "min" ),                        // XML_MIN
        TOKEN( "min-denominator-digits" ),     // XML_MIN_DENOMINATOR_DIGITS
        TOKEN( "min-edge" ),                   // XML_MIN_EDGE
        TOKEN( "min-exponent-digits" ),        // XML_MIN_EXPONENT_DIGITS
        TOKEN( "min-height" ),                 // XML_MIN_HEIGHT
        TOKEN( "min-integer-digits" ),         // XML_MIN_INTEGER_DIGITS
        TOKEN( "min-label-distance" ),         // XML_MIN_LABEL_DISTANCE
        TOKEN( "min-label-width" ),            // XML_MIN_LABEL_WIDTH
        TOKEN( "min-line-height" ),            // XML_MIN_LINE_HEIGHT
        TOKEN( "min-numerator-digits" ),       // XML_MIN_NUMERATOR_DIGITS
        TOKEN( "min-row-height" ),             // XML_MIN_ROW_HEIGHT
        TOKEN( "min-width" ),                  // XML_MIN_WIDTH
        TOKEN( "minimum" ),                    // XML_MINIMUM
        TOKEN( "minor" ),                      // XML_MINOR
        TOKEN( "minus" ),                      // XML_MINUS
        TOKEN( "minutes" ),                    // XML_MINUTES
        TOKEN( "mirror" ),                     // XML_MIRROR
        TOKEN( "mirrored" ),                   // XML_MIRRORED
        TOKEN( "misc" ),                       // XML_MISC
        TOKEN( "miter" ),                      // XML_MITER
        TOKEN( "mm" ),                         // XML_MM
        TOKEN( "mmultiscripts" ),              // XML_MMULTISCRIPTS
        TOKEN( "mn" ),                         // XML_MN
        TOKEN( "mo" ),                         // XML_MO
        TOKEN( "mode" ),                       // XML_MODE
        TOKEN( "modern" ),                     // XML_MODERN
        TOKEN( "modification-date" ),          // XML_MODIFICATION_DATE
        TOKEN( "modification-time" ),          // XML_MODIFICATION_TIME
        TOKEN( "modulate" ),                   // XML_MODULATE
        TOKEN( "module" ),                     // XML_MODULE
        TOKEN( "moment" ),                     // XML_MOMENT
        TOKEN( "mono" ),                       // XML_MONO
        TOKEN( "month" ),                      // XML_MONTH
        TOKEN( "mouse-as-pen" ),               // XML_MOUSE_AS_PEN
        TOKEN( "mouse-visible" ),              // XML_MOUSE_VISIBLE
        TOKEN( "move" ),                       // XML_MOVE
        TOKEN( "move-from-bottom" ),           // XML_MOVE_FROM_BOTTOM
        TOKEN( "move-from-left" ),             // XML_MOVE_FROM_LEFT
        TOKEN( "move-from-right" ),            // XML_MOVE_FROM_RIGHT
        TOKEN( "move-from-top" ),              // XML_MOVE_FROM_TOP
        TOKEN( "move-protect" ),               // XML_MOVE_PROTECT
        TOKEN( "move-short" ),                 // XML_MOVE_SHORT
        TOKEN( "movement" ),                   // XML_MOVEMENT
        TOKEN( "movement-cut-off" ),           // XML_MOVEMENT_CUT_OFF
        TOKEN( "mover" ),                      // XML_MOVER
        TOKEN( "mpadded" ),                    // XML_MPADDED
        TOKEN( "mphantom" ),                   // XML_MPHANTOM
        TOKEN( "mprescripts" ),                // XML_MPRESCRIPTS
        TOKEN( "mroot" ),                      // XML_MROOT
        TOKEN( "mrow" ),                       // XML_MROW
        TOKEN( "ms" ),                         // XML_MS
        TOKEN( "mspace" ),                     // XML_MSPACE
        TOKEN( "msqrt" ),                      // XML_MSQRT
        TOKEN( "mstyle" ),                     // XML_MSTYLE
        TOKEN( "msub" ),                       // XML_MSUB
        TOKEN( "msubsup" ),                    // XML_MSUBSUP
        TOKEN( "msup" ),                       // XML_MSUP
        TOKEN( "mtable" ),                     // XML_MTABLE
        TOKEN( "mtd" ),                        // XML_MTD
        TOKEN( "mtext" ),                      // XML_MTEXT
        TOKEN( "mtr" ),                        // XML_MTR
        TOKEN( "multi-deletion-spanned" ),     // XML_MULTI_DELETION_SPANNED
        TOKEN( "munder" ),                     // XML_MUNDER
        TOKEN( "munderover" ),                 // XML_MUNDEROVER
        TOKEN( "name" ),                       // XML_NAME
        TOKEN( "name-and-extension" ),         // XML_NAME_AND_EXTENSION
        TOKEN( "named-expression" ),           // XML_NAMED_EXPRESSION
        TOKEN( "named-expressions" ),          // XML_NAMED_EXPRESSIONS
        TOKEN( "named-range" ),                // XML_NAMED_RANGE
        TOKEN( "navy" ),                       // XML_NAVY
        TOKEN( "neq" ),                        // XML_NEQ
        TOKEN( "new" ),                        // XML_NEW
        TOKEN( "next" ),                       // XML_NEXT
        TOKEN( "next-page" ),                  // XML_NEXT_PAGE
        TOKEN( "next-style-name" ),            // XML_NEXT_STYLE_NAME
        TOKEN( "no-limit" ),                   // XML_NO_LIMIT
        TOKEN( "no-wrap" ),                    // XML_NO_WRAP
        TOKEN( "!empty" ),                     // XML_NOEMPTY
        TOKEN( "nohref" ),                     // XML_NOHREF
        TOKEN( "!match" ),                     // XML_NOMATCH
        TOKEN( "none" ),                       // XML_NONE
        TOKEN( "notprsubset" ),                // XML_NOPRTSUBSET
        TOKEN( "normal" ),                     // XML_NORMAL
        TOKEN( "normals-direction" ),          // XML_NORMALS_DIRECTION
        TOKEN( "normals-kind" ),               // XML_NORMALS_KIND
        TOKEN( "not" ),                        // XML_NOT
        TOKEN( "not-equal-date" ),             // XML_NOT_EQUAL_DATE
        TOKEN( "note" ),                       // XML_NOTE
        TOKEN( "notes" ),                      // XML_NOTES
        TOKEN( "notin" ),                      // XML_NOTIN
        TOKEN( "notsubset" ),                  // XML_NOTSUBSET
        TOKEN( "null-date" ),                  // XML_NULL_DATE
        TOKEN( "null-year" ),                  // XML_NULL_YEAR
        TOKEN( "num-format" ),                 // XML_NUM_FORMAT
        TOKEN( "num-letter-sync" ),            // XML_NUM_LETTER_SYNC
        TOKEN( "num-prefix" ),                 // XML_NUM_PREFIX
        TOKEN( "num-suffix" ),                 // XML_NUM_SUFFIX
        TOKEN( "number" ),                     // XML_NUMBER
        TOKEN( "number-and-name" ),            // XML_NUMBER_AND_NAME
        TOKEN( "number-columns-repeated" ),    // XML_NUMBER_COLUMNS_REPEATED
        TOKEN( "number-columns-spanned" ),     // XML_NUMBER_COLUMNS_SPANNED
        TOKEN( "number-lines" ),               // XML_NUMBER_LINES
        TOKEN( "number-matrix-columns-spanned" ),  // XML_NUMBER_MATRIX_COLUMNS_SPANNED
        TOKEN( "number-matrix-rows-spanned" ), // XML_NUMBER_MATRIX_ROWS_SPANNED
        TOKEN( "number-position" ),            // XML_NUMBER_POSITION
        TOKEN( "number-rows-repeated" ),       // XML_NUMBER_ROWS_REPEATED
        TOKEN( "number-rows-spanned" ),        // XML_NUMBER_ROWS_SPANNED
        TOKEN( "number-style" ),               // XML_NUMBER_STYLE
        TOKEN( "number-wrapped-paragraphs" ),  // XML_NUMBER_WRAPPED_PARAGRAPHS
        TOKEN( "numbered-entries" ),           // XML_NUMBERED_ENTRIES
        TOKEN( "object" ),                     // XML_OBJECT
        TOKEN( "object-count" ),               // XML_OBJECT_COUNT
        TOKEN( "object-index" ),               // XML_OBJECT_INDEX
        TOKEN( "object-index-entry-template" ),// XML_OBJECT_INDEX_ENTRY_TEMPLATE
        TOKEN( "object-index-source" ),        // XML_OBJECT_INDEX_SOURCE
        TOKEN( "object-name" ),                // XML_OBJECT_NAME
        TOKEN( "object-ole" ),                 // XML_OBJECT_OLE
        TOKEN( "objects" ),                    // XML_OBJECTS
        TOKEN( "odd-page" ),                   // XML_ODD_PAGE
        TOKEN( "offset" ),                     // XML_OFFSET
        TOKEN( "olive" ),                      // XML_OLIVE
        TOKEN( "onLoad" ),                     // XML_ONLOAD
        TOKEN( "onRequest" ),                  // XML_ONREQUEST
        TOKEN( "on-update-keep-size" ),        // XML_ON_UPDATE_KEEP_SIZE
        TOKEN( "on-update-keep-styles" ),      // XML_ON_UPDATE_KEEP_STYLES
        TOKEN( "online" ),                     // XML_ONLINE
        TOKEN( "online-text" ),                // XML_ONLINE_TEXT
        TOKEN( "background" ),                 // XML_OPAQUE_BACKGROUND
        TOKEN( "foreground" ),                 // XML_OPAQUE_FOREGROUND
        TOKEN( "open" ),                       // XML_OPEN
        TOKEN( "open-horizontal" ),            // XML_OPEN_HORIZONTAL
        TOKEN( "open-vertical" ),              // XML_OPEN_VERTICAL
        TOKEN( "operation" ),                  // XML_OPERATION
        TOKEN( "operator" ),                   // XML_OPERATOR
        TOKEN( "optimal" ),                    // XML_OPTIMAL
        TOKEN( "or" ),                         // XML_OR
        TOKEN( "order" ),                      // XML_ORDER
        TOKEN( "ordered-list" ),               // XML_ORDERED_LIST
        TOKEN( "organizations" ),              // XML_ORGANIZATIONS
        TOKEN( "orientation" ),                // XML_ORIENTATION
        TOKEN( "orientation-landscape" ),      // XML_ORIENTATION_LANDSCAPE
        TOKEN( "orientation-portrait" ),       // XML_ORIENTATION_PORTRAIT
        TOKEN( "origin" ),                     // XML_ORIGIN
        TOKEN( "orphans" ),                    // XML_ORPHANS
        TOKEN( "outline-level" ),              // XML_OUTLINE_LEVEL
        TOKEN( "outline-level-style" ),        // XML_OUTLINE_LEVEL_STYLE
        TOKEN( "outline-style" ),              // XML_OUTLINE_STYLE
        TOKEN( "outset" ),                     // XML_OUTSET
        TOKEN( "outside" ),                    // XML_OUTSIDE
        TOKEN( "overlap" ),                    // XML_OVERLAP
        TOKEN( "p" ),                          // XML_P
        TOKEN( "package-name" ),               // XML_PACKAGE_NAME
        TOKEN( "padding" ),                    // XML_PADDING
        TOKEN( "padding-bottom" ),             // XML_PADDING_BOTTOM
        TOKEN( "padding-left" ),               // XML_PADDING_LEFT
        TOKEN( "padding-right" ),              // XML_PADDING_RIGHT
        TOKEN( "padding-top" ),                // XML_PADDING_TOP
        TOKEN( "page" ),                       // XML_PAGE
        TOKEN( "page-adjust" ),                // XML_PAGE_ADJUST
        TOKEN( "page-breaks-on-group-change" ),// XML_PAGE_BREAKS_ON_GROUP_CHANGE
        TOKEN( "page-content" ),               // XML_PAGE_CONTENT
        TOKEN( "page-continuation-string" ),   // XML_PAGE_CONTINUATION_STRING
        TOKEN( "page-count" ),                 // XML_PAGE_COUNT
        TOKEN( "page-end-margin" ),            // XML_PAGE_END_MARGIN
        TOKEN( "page-height" ),                // XML_PAGE_HEIGHT
        TOKEN( "page-master" ),                // XML_PAGE_MASTER
        TOKEN( "page-master-name" ),           // XML_PAGE_MASTER_NAME
        TOKEN( "page-number" ),                // XML_PAGE_NUMBER
        TOKEN( "page-start-margin" ),          // XML_PAGE_START_MARGIN
        TOKEN( "page-style-name" ),            // XML_PAGE_STYLE_NAME
        TOKEN( "page-thumbnail" ),             // XML_PAGE_THUMBNAIL
        TOKEN( "page-usage" ),                 // XML_PAGE_USAGE
        TOKEN( "page-variable-get" ),          // XML_PAGE_VARIABLE_GET
        TOKEN( "page-variable-set" ),          // XML_PAGE_VARIABLE_SET
        TOKEN( "page-view-zoom-value" ),       // XML_PAGE_VIEW_ZOOM_VALUE
        TOKEN( "page-width" ),                 // XML_PAGE_WIDTH
        TOKEN( "pages" ),                      // XML_PAGES
        TOKEN( "paper-tray-number" ),          // XML_PAPER_TRAY_NUMBER
        TOKEN( "paragraph" ),                  // XML_PARAGRAPH
        TOKEN( "paragraph-content" ),          // XML_PARAGRAPH_CONTENT
        TOKEN( "paragraph-count" ),            // XML_PARAGRAPH_COUNT
        TOKEN( "paragraph-end-margin" ),       // XML_PARAGRAPH_END_MARGIN
        TOKEN( "paragraph-start-margin" ),     // XML_PARAGRAPH_START_MARGIN
        TOKEN( "parallel" ),                   // XML_PARALLEL
        TOKEN( "param" ),                      // XML_PARAM
        TOKEN( "parent-style-name" ),          // XML_PARENT_STYLE_NAME
        TOKEN( "parse-sql-statement" ),        // XML_PARSE_SQL_STATEMENT
        TOKEN( "parsed" ),                     // XML_PARSED
        TOKEN( "partialdiff" ),                // XML_PARTIALDIFF
        TOKEN( "password" ),                   // XML_PASSWORD
        TOKEN( "passwort" ),                   // XML_PASSWORT
        TOKEN( "path" ),                       // XML_PATH
        TOKEN( "path-id" ),                    // XML_PATH_ID
        TOKEN( "pause" ),                      // XML_PAUSE
        TOKEN( "pending" ),                    // XML_PENDING
        TOKEN( "percentage" ),                 // XML_PERCENTAGE
        TOKEN( "percentage-style" ),           // XML_PERCENTAGE_STYLE
        TOKEN( "perspective" ),                // XML_PERSPECTIVE
        TOKEN( "phdthesis" ),                  // XML_PHDTHESIS
        TOKEN( "phong" ),                      // XML_PHONG
        TOKEN( "pie-offset" ),                 // XML_PIE_OFFSET
        TOKEN( "placeholder" ),                // XML_PLACEHOLDER
        TOKEN( "placeholder-type" ),           // XML_PLACEHOLDER_TYPE
        TOKEN( "placing" ),                    // XML_PLACING
        TOKEN( "plain-number" ),               // XML_PLAIN_NUMBER
        TOKEN( "plain-number-and-name" ),      // XML_PLAIN_NUMBER_AND_NAME
        TOKEN( "play-full" ),                  // XML_PLAY_FULL
        TOKEN( "plot-area" ),                  // XML_PLOT_AREA
        TOKEN( "plugin" ),                     // XML_PLUGIN
        TOKEN( "plus" ),                       // XML_PLUS
        TOKEN( "points" ),                     // XML_POINTS
        TOKEN( "polygon" ),                    // XML_POLYGON
        TOKEN( "polyline" ),                   // XML_POLYLINE
        TOKEN( "polynomial" ),                 // XML_POLYNOMIAL
        TOKEN( "pool-id" ),                    // XML_POOL_ID
        TOKEN( "portrait" ),                   // XML_PORTRAIT
        TOKEN( "position" ),                   // XML_POSITION
        TOKEN( "position-bottom" ),            // XML_POSITION_BOTTOM
        TOKEN( "position-left" ),              // XML_POSITION_LEFT
        TOKEN( "position-right" ),             // XML_POSITION_RIGHT
        TOKEN( "position-top" ),               // XML_POSITION_TOP
        TOKEN( "italic" ),                     // XML_POSTURE_ITALIC
        TOKEN( "normal" ),                     // XML_POSTURE_NORMAL
        TOKEN( "oblique" ),                    // XML_POSTURE_OBLIQUE
        TOKEN( "power" ),                      // XML_POWER
        TOKEN( "precision-as-shown" ),         // XML_PRECISION_AS_SHOWN
        TOKEN( "prefix" ),                     // XML_PREFIX
        TOKEN( "presentation" ),               // XML_PRESENTATION
        TOKEN( "chart" ),                      // XML_PRESENTATION_CHART
        TOKEN( "graphic" ),                    // XML_PRESENTATION_GRAPHIC
        TOKEN( "notes" ),                      // XML_PRESENTATION_NOTES
        TOKEN( "object" ),                     // XML_PRESENTATION_OBJECT
        TOKEN( "orgchart" ),                   // XML_PRESENTATION_ORGCHART
        TOKEN( "outline" ),                    // XML_PRESENTATION_OUTLINE
        TOKEN( "page" ),                       // XML_PRESENTATION_PAGE
        TOKEN( "presentation-page-layout" ),   // XML_PRESENTATION_PAGE_LAYOUT
        TOKEN( "presentation-page-layout-name" ),  // XML_PRESENTATION_PAGE_LAYOUT_NAME
        TOKEN( "subtitle" ),                   // XML_PRESENTATION_SUBTITLE
        TOKEN( "table" ),                      // XML_PRESENTATION_TABLE
        TOKEN( "title" ),                      // XML_PRESENTATION_TITLE
        TOKEN( "previous" ),                   // XML_PREVIOUS
        TOKEN( "previous-page" ),              // XML_PREVIOUS_PAGE
        TOKEN( "print" ),                      // XML_PRINT
        TOKEN( "print-content" ),              // XML_PRINT_CONTENT
        TOKEN( "print-date" ),                 // XML_PRINT_DATE
        TOKEN( "print-orientation" ),          // XML_PRINT_ORIENTATION
        TOKEN( "print-page-order" ),           // XML_PRINT_PAGE_ORDER
        TOKEN( "print-range" ),                // XML_PRINT_RANGE
        TOKEN( "print-ranges" ),               // XML_PRINT_RANGES
        TOKEN( "print-time" ),                 // XML_PRINT_TIME
        TOKEN( "printable" ),                  // XML_PRINTABLE
        TOKEN( "printed-by" ),                 // XML_PRINTED_BY
        TOKEN( "proceedings" ),                // XML_PROCEEDINGS
        TOKEN( "product" ),                    // XML_PRODUCT
        TOKEN( "projection" ),                 // XML_PROJECTION
        TOKEN( "properties" ),                 // XML_PROPERTIES
        TOKEN( "protect" ),                    // XML_PROTECT
        TOKEN( "content" ),                    // XML_PROTECT_CONTENT
        TOKEN( "position" ),                   // XML_PROTECT_POSITION
        TOKEN( "size" ),                       // XML_PROTECT_SIZE
        TOKEN( "protected" ),                  // XML_PROTECTED
        TOKEN( "protection-key" ),             // XML_PROTECTION_KEY
        TOKEN( "prsubset" ),                   // XML_PRSUBSET
        TOKEN( "publisher" ),                  // XML_PUBLISHER
        TOKEN( "punctuation-wrap" ),           // XML_PUNCTUATION_WRAP
        TOKEN( "purple" ),                     // XML_PURPLE
        TOKEN( "pyramid" ),                    // XML_PYRAMID
        TOKEN( "quarter" ),                    // XML_QUARTER
        TOKEN( "query-name" ),                 // XML_QUERY_NAME
        TOKEN( "quo-vadis" ),                  // XML_QUO_VADIS
        TOKEN( "quotient" ),                   // XML_QUOTIENT
        TOKEN( "r" ),                          // XML_R
        TOKEN( "radar" ),                      // XML_RADAR
        TOKEN( "random" ),                     // XML_RANDOM
        TOKEN( "range-address" ),              // XML_RANGE_ADDRESS
        TOKEN( "range-usable-as" ),            // XML_RANGE_USABLE_AS
        TOKEN( "recreate-on-edit" ),           // XML_RECREATE_ON_EDIT
        TOKEN( "rect" ),                       // XML_RECT
        TOKEN( "red" ),                        // XML_RED
        TOKEN( "ref-name" ),                   // XML_REF_NAME
        TOKEN( "reference" ),                  // XML_REFERENCE
        TOKEN( "reference-end" ),              // XML_REFERENCE_END
        TOKEN( "reference-format" ),           // XML_REFERENCE_FORMAT
        TOKEN( "reference-mark" ),             // XML_REFERENCE_MARK
        TOKEN( "reference-mark-end" ),         // XML_REFERENCE_MARK_END
        TOKEN( "reference-mark-start" ),       // XML_REFERENCE_MARK_START
        TOKEN( "reference-ref" ),              // XML_REFERENCE_REF
        TOKEN( "reference-start" ),            // XML_REFERENCE_START
        TOKEN( "reference-type" ),             // XML_REFERENCE_TYPE
        TOKEN( "refresh-delay" ),              // XML_REFRESH_DELAY
        TOKEN( "region-center" ),              // XML_REGION_CENTER
        TOKEN( "region-left" ),                // XML_REGION_LEFT
        TOKEN( "region-right" ),               // XML_REGION_RIGHT
        TOKEN( "register-true" ),              // XML_REGISTER_TRUE
        TOKEN( "register-truth-ref-style-name" ),  // XML_REGISTER_TRUTH_REF_STYLE_NAME
        TOKEN( "regression-type" ),            // XML_REGRESSION_TYPE
        TOKEN( "rejected" ),                   // XML_REJECTED
        TOKEN( "rejecting-change-id" ),        // XML_REJECTING_CHANGE_ID
        TOKEN( "rejection" ),                  // XML_REJECTION
        TOKEN( "rel-column-width" ),           // XML_REL_COLUMN_WIDTH
        TOKEN( "rel-height" ),                 // XML_REL_HEIGHT
        TOKEN( "rel-width" ),                  // XML_REL_WIDTH
        TOKEN( "relative" ),                   // XML_RELATIVE
        TOKEN( "relative-tab-stop-position" ), // XML_RELATIVE_TAB_STOP_POSITION
        TOKEN( "reln" ),                       // XML_RELN
        TOKEN( "rem" ),                        // XML_REM
        TOKEN( "remove-dependents" ),          // XML_REMOVE_DEPENDENTS
        TOKEN( "remove-precedents" ),          // XML_REMOVE_PRECEDENTS
        TOKEN( "repeat" ),                     // XML_REPEAT
        TOKEN( "repeat-column" ),              // XML_REPEAT_COLUMN
        TOKEN( "repeat-row" ),                 // XML_REPEAT_ROW
        TOKEN( "repeated" ),                   // XML_REPEATED
        TOKEN( "replace" ),                    // XML_REPLACE
        TOKEN( "report-type" ),                // XML_REPORT_TYPE
        TOKEN( "restart-on-page" ),            // XML_RESTART_ON_PAGE
        TOKEN( "revision" ),                   // XML_REVISION
        TOKEN( "ridge" ),                      // XML_RIDGE
        TOKEN( "right" ),                      // XML_RIGHT
        TOKEN( "right-outside" ),              // XML_RIGHT_OUTSIDE
        TOKEN( "right-arc" ),                  // XML_RIGHTARC
        TOKEN( "right-circle" ),               // XML_RIGHTCIRCLE
        TOKEN( "ring" ),                       // XML_RING
        TOKEN( "role" ),                       // XML_ROLE
        TOKEN( "roll-from-bottom" ),           // XML_ROLL_FROM_BOTTOM
        TOKEN( "roll-from-left" ),             // XML_ROLL_FROM_LEFT
        TOKEN( "roll-from-right" ),            // XML_ROLL_FROM_RIGHT
        TOKEN( "roman" ),                      // XML_ROMAN
        TOKEN( "root" ),                       // XML_ROOT
        TOKEN( "rotate" ),                     // XML_ROTATE
        TOKEN( "rotation" ),                   // XML_ROTATION
        TOKEN( "rotation-align" ),             // XML_ROTATION_ALIGN
        TOKEN( "rotation-angle" ),             // XML_ROTATION_ANGLE
        TOKEN( "round" ),                      // XML_ROUND
        TOKEN( "row" ),                        // XML_ROW
        TOKEN( "row-height" ),                 // XML_ROW_HEIGHT
        TOKEN( "row-number" ),                 // XML_ROW_NUMBER
        TOKEN( "rows" ),                       // XML_ROWS
        TOKEN( "ruby" ),                       // XML_RUBY
        TOKEN( "ruby-align" ),                 // XML_RUBY_ALIGN
        TOKEN( "ruby-base" ),                  // XML_RUBY_BASE
        TOKEN( "ruby-position" ),              // XML_RUBY_POSITION
        TOKEN( "ruby-text" ),                  // XML_RUBY_TEXT
        TOKEN( "run-through" ),                // XML_RUN_THROUGH
        TOKEN( "rx" ),                         // XML_RX
        TOKEN( "ry" ),                         // XML_RY
        TOKEN( "s" ),                          // XML_S
        TOKEN( "scale" ),                      // XML_SCALE
        TOKEN( "scale-min" ),                  // XML_SCALE_MIN
        TOKEN( "scale-text" ),                 // XML_SCALE_TEXT
        TOKEN( "scale-to" ),                   // XML_SCALE_TO
        TOKEN( "scale-to-pages" ),             // XML_SCALE_TO_PAGES
        TOKEN( "scatter" ),                    // XML_SCATTER
        TOKEN( "scenario" ),                   // XML_SCENARIO
        TOKEN( "scenario-ranges" ),            // XML_SCENARIO_RANGES
        TOKEN( "scene" ),                      // XML_SCENE
        TOKEN( "school" ),                     // XML_SCHOOL
        TOKEN( "scientific-number" ),          // XML_SCIENTIFIC_NUMBER
        TOKEN( "score-spaces" ),               // XML_SCORE_SPACES
        TOKEN( "script" ),                     // XML_SCRIPT
        TOKEN( "scroll" ),                     // XML_SCROLL
        TOKEN( "sdev" ),                       // XML_SDEV
        TOKEN( "search-criteria-must-apply-to-whole-cell" ),   // XML_SEARCH_CRITERIA_MUST_APPLY_TO_WHOLE_CELL
        TOKEN( "sec" ),                        // XML_SEC
        TOKEN( "sech" ),                       // XML_SECH
        TOKEN( "second-date-time" ),           // XML_SECOND_DATE_TIME
        TOKEN( "seconds" ),                    // XML_SECONDS
        TOKEN( "section" ),                    // XML_SECTION
        TOKEN( "section-desc" ),               // XML_SECTION_DESC
        TOKEN( "section-name" ),               // XML_SECTION_NAME
        TOKEN( "section-source" ),             // XML_SECTION_SOURCE
        TOKEN( "select-page" ),                // XML_SELECT_PAGE
        TOKEN( "selector" ),                   // XML_SELECTOR
        TOKEN( "semantics" ),                  // XML_SEMANTICS
        TOKEN( "semi-automatic" ),             // XML_SEMI_AUTOMATIC
        TOKEN( "sender-city" ),                // XML_SENDER_CITY
        TOKEN( "sender-company" ),             // XML_SENDER_COMPANY
        TOKEN( "sender-country" ),             // XML_SENDER_COUNTRY
        TOKEN( "sender-email" ),               // XML_SENDER_EMAIL
        TOKEN( "sender-fax" ),                 // XML_SENDER_FAX
        TOKEN( "sender-firstname" ),           // XML_SENDER_FIRSTNAME
        TOKEN( "sender-initials" ),            // XML_SENDER_INITIALS
        TOKEN( "sender-lastname" ),            // XML_SENDER_LASTNAME
        TOKEN( "sender-phone-private" ),       // XML_SENDER_PHONE_PRIVATE
        TOKEN( "sender-phone-work" ),          // XML_SENDER_PHONE_WORK
        TOKEN( "sender-position" ),            // XML_SENDER_POSITION
        TOKEN( "sender-postal-code" ),         // XML_SENDER_POSTAL_CODE
        TOKEN( "sender-state-or-province" ),   // XML_SENDER_STATE_OR_PROVINCE
        TOKEN( "sender-street" ),              // XML_SENDER_STREET
        TOKEN( "sender-title" ),               // XML_SENDER_TITLE
        TOKEN( "sep" ),                        // XML_SEP
        TOKEN( "separation-character" ),       // XML_SEPARATION_CHARACTER
        TOKEN( "separator" ),                  // XML_SEPARATOR
        TOKEN( "sequence" ),                   // XML_SEQUENCE
        TOKEN( "sequence-decl" ),              // XML_SEQUENCE_DECL
        TOKEN( "sequence-decls" ),             // XML_SEQUENCE_DECLS
        TOKEN( "sequence-ref" ),               // XML_SEQUENCE_REF
        TOKEN( "series" ),                     // XML_SERIES
        TOKEN( "series-source" ),              // XML_SERIES_SOURCE
        TOKEN( "server-map" ),                 // XML_SERVER_MAP
        TOKEN( "set" ),                        // XML_SET
        TOKEN( "setdiff" ),                    // XML_SETDIFF
        TOKEN( "settings" ),                   // XML_SETTINGS
        TOKEN( "shade-mode" ),                 // XML_SHADE_MODE
        TOKEN( "shadow" ),                     // XML_SHADOW
        TOKEN( "shadow-color" ),               // XML_SHADOW_COLOR
        TOKEN( "shadow-offset-x" ),            // XML_SHADOW_OFFSET_X
        TOKEN( "shadow-offset-y" ),            // XML_SHADOW_OFFSET_Y
        TOKEN( "shadow-slant" ),               // XML_SHADOW_SLANT
        TOKEN( "shadow-transparency" ),        // XML_SHADOW_TRANSPARENCY
        TOKEN( "shape" ),                      // XML_SHAPE
        TOKEN( "shape-id" ),                   // XML_SHAPE_ID
        TOKEN( "shapes" ),                     // XML_SHAPES
        TOKEN( "sheet-name" ),                 // XML_SHEET_NAME
        TOKEN( "shininess" ),                  // XML_SHININESS
        TOKEN( "short" ),                      // XML_SHORT
        TOKEN( "show" ),                       // XML_SHOW
        TOKEN( "show-accepted-changes" ),      // XML_SHOW_ACCEPTED_CHANGES
        TOKEN( "show-changes" ),               // XML_SHOW_CHANGES
        TOKEN( "show-changes-by-author" ),     // XML_SHOW_CHANGES_BY_AUTHOR
        TOKEN( "show-changes-by-author-name" ),// XML_SHOW_CHANGES_BY_AUTHOR_NAME
        TOKEN( "show-changes-by-comment" ),    // XML_SHOW_CHANGES_BY_COMMENT
        TOKEN( "show-changes-by-comment-text" ),   // XML_SHOW_CHANGES_BY_COMMENT_TEXT
        TOKEN( "show-changes-by-datetime" ),   // XML_SHOW_CHANGES_BY_DATETIME
        TOKEN( "show-changes-by-datetime-first-datetime" ),// XML_SHOW_CHANGES_BY_DATETIME_FIRST_DATETIME
        TOKEN( "show-changes-by-datetime-mode" ),  // XML_SHOW_CHANGES_BY_DATETIME_MODE
        TOKEN( "show-changes-by-datetime-second-datetime" ),   // XML_SHOW_CHANGES_BY_DATETIME_SECOND_DATETIME
        TOKEN( "show-changes-by-ranges" ),     // XML_SHOW_CHANGES_BY_RANGES
        TOKEN( "show-changes-by-ranges-list" ),// XML_SHOW_CHANGES_BY_RANGES_LIST
        TOKEN( "show-logo" ),                  // XML_SHOW_LOGO
        TOKEN( "show-rejected-changes" ),      // XML_SHOW_REJECTED_CHANGES
        TOKEN( "show-shape" ),                 // XML_SHOW_SHAPE
        TOKEN( "show-text" ),                  // XML_SHOW_TEXT
        TOKEN( "show-unit" ),                  // XML_SHOW_UNIT
        TOKEN( "shows" ),                      // XML_SHOWS
        TOKEN( "side-by-side" ),               // XML_SIDE_BY_SIDE
        TOKEN( "silver" ),                     // XML_SILVER
        TOKEN( "simple" ),                     // XML_SIMPLE
        TOKEN( "sin" ),                        // XML_SIN
        TOKEN( "since-date-time" ),            // XML_SINCE_DATE_TIME
        TOKEN( "since-save" ),                 // XML_SINCE_SAVE
        TOKEN( "sinh" ),                       // XML_SINH
        TOKEN( "size" ),                       // XML_SIZE
        TOKEN( "size-protect" ),               // XML_SIZE_PROTECT
        TOKEN( "slant" ),                      // XML_SLANT
        TOKEN( "slant-x" ),                    // XML_SLANT_X
        TOKEN( "slant-y" ),                    // XML_SLANT_Y
        TOKEN( "slide" ),                      // XML_SLIDE
        TOKEN( "slow" ),                       // XML_SLOW
        TOKEN( "solid" ),                      // XML_SOLID
        TOKEN( "solid-type" ),                 // XML_SOLID_TYPE
        TOKEN( "sort" ),                       // XML_SORT
        TOKEN( "sort-ascending" ),             // XML_SORT_ASCENDING
        TOKEN( "sort-by" ),                    // XML_SORT_BY
        TOKEN( "sort-by-position" ),           // XML_SORT_BY_POSITION
        TOKEN( "sort-groups" ),                // XML_SORT_GROUPS
        TOKEN( "sort-key" ),                   // XML_SORT_KEY
        TOKEN( "sound" ),                      // XML_SOUND
        TOKEN( "source-cell-range" ),          // XML_SOURCE_CELL_RANGE
        TOKEN( "source-cell-range-addresses" ),// XML_SOURCE_CELL_RANGE_ADDRESSES
        TOKEN( "source-field-name" ),          // XML_SOURCE_FIELD_NAME
        TOKEN( "source-name" ),                // XML_SOURCE_NAME
        TOKEN( "source-range-address" ),       // XML_SOURCE_RANGE_ADDRESS
        TOKEN( "source-service" ),             // XML_SOURCE_SERVICE
        TOKEN( "space-before" ),               // XML_SPACE_BEFORE
        TOKEN( "span" ),                       // XML_SPAN
        TOKEN( "specular" ),                   // XML_SPECULAR
        TOKEN( "specular-color" ),             // XML_SPECULAR_COLOR
        TOKEN( "speed" ),                      // XML_SPEED
        TOKEN( "sphere" ),                     // XML_SPHERE
        TOKEN( "spiral" ),                     // XML_SPIRAL
        TOKEN( "spiral-in" ),                  // XML_SPIRAL_IN
        TOKEN( "spiral-inward-left" ),         // XML_SPIRAL_INWARD_LEFT
        TOKEN( "spiral-inward-right" ),        // XML_SPIRAL_INWARD_RIGHT
        TOKEN( "spiral-out" ),                 // XML_SPIRAL_OUT
        TOKEN( "spiral-outward-left" ),        // XML_SPIRAL_OUTWARD_LEFT
        TOKEN( "spiral-outward-right" ),       // XML_SPIRAL_OUTWARD_RIGHT
        TOKEN( "spiralin-left" ),              // XML_SPIRALIN_LEFT
        TOKEN( "spiralin-right" ),             // XML_SPIRALIN_RIGHT
        TOKEN( "spiralout-left" ),             // XML_SPIRALOUT_LEFT
        TOKEN( "spiralout-right" ),            // XML_SPIRALOUT_RIGHT
        TOKEN( "splines" ),                    // XML_SPLINES
        TOKEN( "split" ),                      // XML_SPLIT
        TOKEN( "split-column" ),               // XML_SPLIT_COLUMN
        TOKEN( "split-position" ),             // XML_SPLIT_POSITION
        TOKEN( "split-row" ),                  // XML_SPLIT_ROW
        TOKEN( "spreadsheet" ),                // XML_SPREADSHEET
        TOKEN( "sql-statement" ),              // XML_SQL_STATEMENT
        TOKEN( "stacked" ),                    // XML_STACKED
        TOKEN( "stagger-even" ),               // XML_STAGGER_EVEN
        TOKEN( "stagger-odd" ),                // XML_STAGGER_ODD
        TOKEN( "standard" ),                   // XML_STANDARD
        TOKEN( "standard-deviation" ),         // XML_STANDARD_DEVIATION
        TOKEN( "starbasic" ),                  // XML_STARBASIC
        TOKEN( "start" ),                      // XML_START
        TOKEN( "start-angle" ),                // XML_START_ANGLE
        TOKEN( "start-color" ),                // XML_START_COLOR
        TOKEN( "start-column" ),               // XML_START_COLUMN
        TOKEN( "start-glue-point" ),           // XML_START_GLUE_POINT
        TOKEN( "start-guide" ),                // XML_START_GUIDE
        TOKEN( "start-intensity" ),            // XML_START_INTENSITY
        TOKEN( "start-line-spacing-horizontal" ),  // XML_START_LINE_SPACING_HORIZONTAL
        TOKEN( "start-line-spacing-vertical" ),// XML_START_LINE_SPACING_VERTICAL
        TOKEN( "start-numbering-at" ),         // XML_START_NUMBERING_AT
        TOKEN( "start-page" ),                 // XML_START_PAGE
        TOKEN( "start-position" ),             // XML_START_POSITION
        TOKEN( "start-row" ),                  // XML_START_ROW
        TOKEN( "start-scale" ),                // XML_START_SCALE
        TOKEN( "start-shape" ),                // XML_START_SHAPE
        TOKEN( "start-table" ),                // XML_START_TABLE
        TOKEN( "start-value" ),                // XML_START_VALUE
        TOKEN( "start-with-navigator" ),       // XML_START_WITH_NAVIGATOR
        TOKEN( "statistics" ),                 // XML_STATISTICS
        TOKEN( "status" ),                     // XML_STATUS
        TOKEN( "stay-on-top" ),                // XML_STAY_ON_TOP
        TOKEN( "stdev" ),                      // XML_STDEV
        TOKEN( "stdevp" ),                     // XML_STDEVP
        TOKEN( "steps" ),                      // XML_STEPS
        TOKEN( "stock" ),                      // XML_STOCK
        TOKEN( "stock-updown-bars" ),          // XML_STOCK_UPDOWN_BARS
        TOKEN( "stock-with-volume" ),          // XML_STOCK_WITH_VOLUME
        TOKEN( "stop" ),                       // XML_STOP
        TOKEN( "stretch" ),                    // XML_STRETCH
        TOKEN( "stretch-from-bottom" ),        // XML_STRETCH_FROM_BOTTOM
        TOKEN( "stretch-from-left" ),          // XML_STRETCH_FROM_LEFT
        TOKEN( "stretch-from-right" ),         // XML_STRETCH_FROM_RIGHT
        TOKEN( "stretch-from-top" ),           // XML_STRETCH_FROM_TOP
        TOKEN( "stretchy" ),                   // XML_STRETCHY
        TOKEN( "strict" ),                     // XML_STRICT
        TOKEN( "string" ),                     // XML_STRING
        TOKEN( "string-value" ),               // XML_STRING_VALUE
        TOKEN( "string-value-if-false" ),      // XML_STRING_VALUE_IF_FALSE
        TOKEN( "string-value-if-true" ),       // XML_STRING_VALUE_IF_TRUE
        TOKEN( "stripes" ),                    // XML_STRIPES
        TOKEN( "stroke" ),                     // XML_STROKE
        TOKEN( "stroke-color" ),               // XML_STROKE_COLOR
        TOKEN( "stroke-dash" ),                // XML_STROKE_DASH
        TOKEN( "stroke-linejoin" ),            // XML_STROKE_LINEJOIN
        TOKEN( "stroke-opacity" ),             // XML_STROKE_OPACITY
        TOKEN( "stroke-width" ),               // XML_STROKE_WIDTH
        TOKEN( "structure-protected" ),        // XML_STRUCTURE_PROTECTED
        TOKEN( "style" ),                      // XML_STYLE
        TOKEN( "style-name" ),                 // XML_STYLE_NAME
        TOKEN( "styles" ),                     // XML_STYLES
        TOKEN( "stylesheet" ),                 // XML_STYLESHEET
        TOKEN( "sub-table" ),                  // XML_SUB_TABLE
        TOKEN( "subject" ),                    // XML_SUBJECT
        TOKEN( "subset" ),                     // XML_SUBSET
        TOKEN( "subtitle" ),                   // XML_SUBTITLE
        TOKEN( "subtotal-field" ),             // XML_SUBTOTAL_FIELD
        TOKEN( "subtotal-rule" ),              // XML_SUBTOTAL_RULE
        TOKEN( "subtotal-rules" ),             // XML_SUBTOTAL_RULES
        TOKEN( "suffix" ),                     // XML_SUFFIX
        TOKEN( "sum" ),                        // XML_SUM
        TOKEN( "swiss" ),                      // XML_SWISS
        TOKEN( "symbol" ),                     // XML_SYMBOL
        TOKEN( "symbol-height" ),              // XML_SYMBOL_HEIGHT
        TOKEN( "symbol-image-name" ),          // XML_SYMBOL_IMAGE_NAME
        TOKEN( "symbol-width" ),               // XML_SYMBOL_WIDTH
        TOKEN( "system" ),                     // XML_SYSTEM
        TOKEN( "tab-stop" ),                   // XML_TAB_STOP
        TOKEN( "tab-stop-distance" ),          // XML_TAB_STOP_DISTANCE
        TOKEN( "tab-stops" ),                  // XML_TAB_STOPS
        TOKEN( "table" ),                      // XML_TABLE
        TOKEN( "table-background" ),           // XML_TABLE_BACKGROUND
        TOKEN( "table-cell" ),                 // XML_TABLE_CELL
        TOKEN( "table-centering" ),            // XML_TABLE_CENTERING
        TOKEN( "table-column" ),               // XML_TABLE_COLUMN
        TOKEN( "table-column-group" ),         // XML_TABLE_COLUMN_GROUP
        TOKEN( "table-columns" ),              // XML_TABLE_COLUMNS
        TOKEN( "table-count" ),                // XML_TABLE_COUNT
        TOKEN( "table-header" ),               // XML_TABLE_HEADER
        TOKEN( "table-header-columns" ),       // XML_TABLE_HEADER_COLUMNS
        TOKEN( "table-header-rows" ),          // XML_TABLE_HEADER_ROWS
        TOKEN( "table-index" ),                // XML_TABLE_INDEX
        TOKEN( "table-index-entry-template" ), // XML_TABLE_INDEX_ENTRY_TEMPLATE
        TOKEN( "table-index-source" ),         // XML_TABLE_INDEX_SOURCE
        TOKEN( "table-name" ),                 // XML_TABLE_NAME
        TOKEN( "table-number-list" ),          // XML_TABLE_NUMBER_LIST
        TOKEN( "table-of-content" ),           // XML_TABLE_OF_CONTENT
        TOKEN( "table-of-content-entry-template" ),// XML_TABLE_OF_CONTENT_ENTRY_TEMPLATE
        TOKEN( "table-of-content-source" ),    // XML_TABLE_OF_CONTENT_SOURCE
        TOKEN( "table-page" ),                 // XML_TABLE_PAGE
        TOKEN( "table-row" ),                  // XML_TABLE_ROW
        TOKEN( "table-row-group" ),            // XML_TABLE_ROW_GROUP
        TOKEN( "table-rows" ),                 // XML_TABLE_ROWS
        TOKEN( "table-source" ),               // XML_TABLE_SOURCE
        TOKEN( "table-view" ),                 // XML_TABLE_VIEW
        TOKEN( "tables" ),                     // XML_TABLES
        TOKEN( "tan" ),                        // XML_TAN
        TOKEN( "tanh" ),                       // XML_TANH
        TOKEN( "target-cell-address" ),        // XML_TARGET_CELL_ADDRESS
        TOKEN( "target-frame-name" ),          // XML_TARGET_FRAME_NAME
        TOKEN( "target-range-address" ),       // XML_TARGET_RANGE_ADDRESS
        TOKEN( "tb-rl" ),                      // XML_TB_RL
        TOKEN( "teal" ),                       // XML_TEAL
        TOKEN( "techreport" ),                 // XML_TECHREPORT
        TOKEN( "template" ),                   // XML_TEMPLATE
        TOKEN( "template-name" ),              // XML_TEMPLATE_NAME
        TOKEN( "tendsto" ),                    // XML_TENDSTO
        TOKEN( "texture-filter" ),             // XML_TEX_FILTER
        TOKEN( "texture-generation-mode-x" ),  // XML_TEX_GENERATION_MODE_X
        TOKEN( "texture-generation-mode-y" ),  // XML_TEX_GENERATION_MODE_Y
        TOKEN( "texture-kind" ),               // XML_TEX_KIND
        TOKEN( "texture-mode" ),               // XML_TEX_MODE
        TOKEN( "text" ),                       // XML_TEXT
        TOKEN( "text-align" ),                 // XML_TEXT_ALIGN
        TOKEN( "text-align-last" ),            // XML_TEXT_ALIGN_LAST
        TOKEN( "text-align-source" ),          // XML_TEXT_ALIGN_SOURCE
        TOKEN( "text-autospace" ),             // XML_TEXT_AUTOSPACE
        TOKEN( "text-background-color" ),      // XML_TEXT_BACKGROUND_COLOR
        TOKEN( "text-blinking" ),              // XML_TEXT_BLINKING
        TOKEN( "text-box" ),                   // XML_TEXT_BOX
        TOKEN( "text-combine" ),               // XML_TEXT_COMBINE
        TOKEN( "text-combine-end-char" ),      // XML_TEXT_COMBINE_END_CHAR
        TOKEN( "text-combine-start-char" ),    // XML_TEXT_COMBINE_START_CHAR
        TOKEN( "text-content" ),               // XML_TEXT_CONTENT
        TOKEN( "text-crossing-out" ),          // XML_TEXT_CROSSING_OUT
        TOKEN( "text-emphasize" ),             // XML_TEXT_EMPHASIZE
        TOKEN( "text-global" ),                // XML_TEXT_GLOBAL
        TOKEN( "text-indent" ),                // XML_TEXT_INDENT
        TOKEN( "text-input" ),                 // XML_TEXT_INPUT
        TOKEN( "text-outline" ),               // XML_TEXT_OUTLINE
        TOKEN( "text-position" ),              // XML_TEXT_POSITION
        TOKEN( "text-rotation-angle" ),        // XML_TEXT_ROTATION_ANGLE
        TOKEN( "text-rotation-scale" ),        // XML_TEXT_ROTATION_SCALE
        TOKEN( "text-scale" ),                 // XML_TEXT_SCALE
        TOKEN( "text-shadow" ),                // XML_TEXT_SHADOW
        TOKEN( "text-style" ),                 // XML_TEXT_STYLE
        TOKEN( "text-transform" ),             // XML_TEXT_TRANSFORM
        TOKEN( "text-underline" ),             // XML_TEXT_UNDERLINE
        TOKEN( "text-underline-color" ),       // XML_TEXT_UNDERLINE_COLOR
        TOKEN( "textarea-horizontal-align" ),  // XML_TEXTAREA_HORIZONTAL_ALIGN
        TOKEN( "textarea-vertical-align" ),    // XML_TEXTAREA_VERTICAL_ALIGN
        TOKEN( "textual" ),                    // XML_TEXTUAL
        TOKEN( "thick" ),                      // XML_THICK
        TOKEN( "thin" ),                       // XML_THIN
        TOKEN( "three-dimensional" ),          // XML_THREE_DIMENSIONAL
        TOKEN( "thumbnail" ),                  // XML_THUMBNAIL
        TOKEN( "tick-marks-major-inner" ),     // XML_TICK_MARKS_MAJOR_INNER
        TOKEN( "tick-marks-major-outer" ),     // XML_TICK_MARKS_MAJOR_OUTER
        TOKEN( "tick-marks-minor-inner" ),     // XML_TICK_MARKS_MINOR_INNER
        TOKEN( "tick-marks-minor-outer" ),     // XML_TICK_MARKS_MINOR_OUTER
        TOKEN( "tile-repeat-offset" ),         // XML_TILE_REPEAT_OFFSET
        TOKEN( "time" ),                       // XML_TIME
        TOKEN( "time-adjust" ),                // XML_TIME_ADJUST
        TOKEN( "time-style" ),                 // XML_TIME_STYLE
        TOKEN( "time-value" ),                 // XML_TIME_VALUE
        TOKEN( "times" ),                      // XML_TIMES
        TOKEN( "title" ),                      // XML_TITLE
        TOKEN( "to-another-table" ),           // XML_TO_ANOTHER_TABLE
        TOKEN( "to-bottom" ),                  // XML_TO_BOTTOM
        TOKEN( "to-center" ),                  // XML_TO_CENTER
        TOKEN( "to-left" ),                    // XML_TO_LEFT
        TOKEN( "to-lower-left" ),              // XML_TO_LOWER_LEFT
        TOKEN( "to-lower-right" ),             // XML_TO_LOWER_RIGHT
        TOKEN( "to-right" ),                   // XML_TO_RIGHT
        TOKEN( "to-top" ),                     // XML_TO_TOP
        TOKEN( "to-upper-left" ),              // XML_TO_UPPER_LEFT
        TOKEN( "to-upper-right" ),             // XML_TO_UPPER_RIGHT
        TOKEN( "toc-mark" ),                   // XML_TOC_MARK
        TOKEN( "toc-mark-end" ),               // XML_TOC_MARK_END
        TOKEN( "toc-mark-start" ),             // XML_TOC_MARK_START
        TOKEN( "top" ),                        // XML_TOP
        TOKEN( "top-left" ),                   // XML_TOP_LEFT
        TOKEN( "top percent" ),                // XML_TOP_PERCENT
        TOKEN( "top-right" ),                  // XML_TOP_RIGHT
        TOKEN( "top values" ),                 // XML_TOP_VALUES
        TOKEN( "top-arc" ),                    // XML_TOPARC
        TOKEN( "top-circle" ),                 // XML_TOPCIRCLE
        TOKEN( "trace-dependents" ),           // XML_TRACE_DEPENDENTS
        TOKEN( "trace-errors" ),               // XML_TRACE_ERRORS
        TOKEN( "trace-precedents" ),           // XML_TRACE_PRECEDENTS
        TOKEN( "track-changes" ),              // XML_TRACK_CHANGES
        TOKEN( "tracked-changes" ),            // XML_TRACKED_CHANGES
        TOKEN( "tracked-changes-view-settings" ),  // XML_TRACKED_CHANGES_VIEW_SETTINGS
        TOKEN( "transform" ),                  // XML_TRANSFORM
        TOKEN( "transition-on-click" ),        // XML_TRANSITION_ON_CLICK
        TOKEN( "transparency" ),               // XML_TRANSPARENCY
        TOKEN( "transparency-name" ),          // XML_TRANSPARENCY_NAME
        TOKEN( "transparent" ),                // XML_TRANSPARENT
        TOKEN( "transpose" ),                  // XML_TRANSPOSE
        TOKEN( "true" ),                       // XML_TRUE
        TOKEN( "truncate-on-overflow" ),       // XML_TRUNCATE_ON_OVERFLOW
        TOKEN( "ttb" ),                        // XML_TTB
        TOKEN( "type" ),                       // XML_TYPE
        TOKEN( "dot-dash" ),                   // XML_DOT_DASH
        TOKEN( "dot-dot-dash" ),               // XML_DOT_DOT_DASH
        TOKEN( "long-dash" ),                  // XML_LONG_DASH
        TOKEN( "single" ),                     // XML_SINGLE
        TOKEN( "small-wave" ),                 // XML_SMALL_WAVE
        TOKEN( "wave" ),                       // XML_WAVE
        TOKEN( "unformatted-text" ),           // XML_UNFORMATTED_TEXT
        TOKEN( "union" ),                      // XML_UNION
        TOKEN( "unit" ),                       // XML_UNIT
        TOKEN( "unordered-list" ),             // XML_UNORDERED_LIST
        TOKEN( "unpublished" ),                // XML_UNPUBLISHED
        TOKEN( "up" ),                         // XML_UP
        TOKEN( "uplimit" ),                    // XML_UPLIMIT
        TOKEN( "upright" ),                    // XML_UPRIGHT
        TOKEN( "url" ),                        // XML_URL
        TOKEN( "use-caption" ),                // XML_USE_CAPTION
        TOKEN( "use-cell-protection" ),        // XML_USE_CELL_PROTECTION
        TOKEN( "use-chart-objects" ),          // XML_USE_CHART_OBJECTS
        TOKEN( "use-condition" ),              // XML_USE_CONDITION
        TOKEN( "use-draw-objects" ),           // XML_USE_DRAW_OBJECTS
        TOKEN( "use-floating-frames" ),        // XML_USE_FLOATING_FRAMES
        TOKEN( "use-graphics" ),               // XML_USE_GRAPHICS
        TOKEN( "use-image-objects" ),          // XML_USE_IMAGE_OBJECTS
        TOKEN( "use-index-marks" ),            // XML_USE_INDEX_MARKS
        TOKEN( "use-index-source-styles" ),    // XML_USE_INDEX_SOURCE_STYLES
        TOKEN( "use-keys-as-entries" ),        // XML_USE_KEYS_AS_ENTRIES
        TOKEN( "use-label" ),                  // XML_USE_LABEL
        TOKEN( "use-math-objects" ),           // XML_USE_MATH_OBJECTS
        TOKEN( "use-objects" ),                // XML_USE_OBJECTS
        TOKEN( "use-optimal-column-width" ),   // XML_USE_OPTIMAL_COLUMN_WIDTH
        TOKEN( "use-optimal-row-height" ),     // XML_USE_OPTIMAL_ROW_HEIGHT
        TOKEN( "use-other-objects" ),          // XML_USE_OTHER_OBJECTS
        TOKEN( "use-spreadsheet-objects" ),    // XML_USE_SPREADSHEET_OBJECTS
        TOKEN( "use-styles" ),                 // XML_USE_STYLES
        TOKEN( "use-tables" ),                 // XML_USE_TABLES
        TOKEN( "use-window-font-color" ),      // XML_USE_WINDOW_FONT_COLOR
        TOKEN( "used-hierarchy" ),             // XML_USED_HIERARCHY
        TOKEN( "user-defined" ),               // XML_USER_DEFINED
        TOKEN( "user-field-decl" ),            // XML_USER_FIELD_DECL
        TOKEN( "user-field-decls" ),           // XML_USER_FIELD_DECLS
        TOKEN( "user-field-get" ),             // XML_USER_FIELD_GET
        TOKEN( "user-field-input" ),           // XML_USER_FIELD_INPUT
        TOKEN( "user-index" ),                 // XML_USER_INDEX
        TOKEN( "user-index-entry-template" ),  // XML_USER_INDEX_ENTRY_TEMPLATE
        TOKEN( "user-index-mark" ),            // XML_USER_INDEX_MARK
        TOKEN( "user-index-mark-end" ),        // XML_USER_INDEX_MARK_END
        TOKEN( "user-index-mark-start" ),      // XML_USER_INDEX_MARK_START
        TOKEN( "user-index-source" ),          // XML_USER_INDEX_SOURCE
        TOKEN( "user-info-0" ),                // XML_USER_INFO_0
        TOKEN( "user-info-1" ),                // XML_USER_INFO_1
        TOKEN( "user-info-2" ),                // XML_USER_INFO_2
        TOKEN( "user-info-3" ),                // XML_USER_INFO_3
        TOKEN( "user-transformed" ),           // XML_USER_TRANSFORMED
        TOKEN( "username" ),                   // XML_USERNAME
        TOKEN( "value" ),                      // XML_VALUE
        TOKEN( "value-type" ),                 // XML_VALUE_TYPE
        TOKEN( "values-cell-range-address" ),  // XML_VALUES_CELL_RANGE_ADDRESS
        TOKEN( "var" ),                        // XML_VAR
        TOKEN( "variable" ),                   // XML_VARIABLE
        TOKEN( "variable-decl" ),              // XML_VARIABLE_DECL
        TOKEN( "variable-decls" ),             // XML_VARIABLE_DECLS
        TOKEN( "variable-get" ),               // XML_VARIABLE_GET
        TOKEN( "variable-input" ),             // XML_VARIABLE_INPUT
        TOKEN( "variable-set" ),               // XML_VARIABLE_SET
        TOKEN( "variance" ),                   // XML_VARIANCE
        TOKEN( "varp" ),                       // XML_VARP
        TOKEN( "vector" ),                     // XML_VECTOR
        TOKEN( "verb" ),                       // XML_VERB
        TOKEN( "version" ),                    // XML_VERSION
        TOKEN( "version-entry" ),              // XML_VERSION_ENTRY
        TOKEN( "version-list" ),               // XML_VERSION_LIST
        TOKEN( "vertical" ),                   // XML_VERTICAL
        TOKEN( "vertical-align" ),             // XML_VERTICAL_ALIGN
        TOKEN( "vertical-lines" ),             // XML_VERTICAL_LINES
        TOKEN( "vertical-pos" ),               // XML_VERTICAL_POS
        TOKEN( "vertical-rel" ),               // XML_VERTICAL_REL
        TOKEN( "vertical-segments" ),          // XML_VERTICAL_SEGMENTS
        TOKEN( "vertical-split-mode" ),        // XML_VERTICAL_SPLIT_MODE
        TOKEN( "vertical-split-position" ),    // XML_VERTICAL_SPLIT_POSITION
        TOKEN( "vertical-stripes" ),           // XML_VERTICAL_STRIPES
        TOKEN( "view" ),                       // XML_VIEW
        TOKEN( "viewBox" ),                    // XML_VIEWBOX
        TOKEN( "view-id" ),                    // XML_VIEW_ID
        TOKEN( "view-settings" ),              // XML_VIEW_SETTINGS
        TOKEN( "visibility" ),                 // XML_VISIBILITY
        TOKEN( "visible" ),                    // XML_VISIBLE
        TOKEN( "visible-area" ),               // XML_VISIBLE_AREA
        TOKEN( "visible-area-height" ),        // XML_VISIBLE_AREA_HEIGHT
        TOKEN( "visible-area-left" ),          // XML_VISIBLE_AREA_LEFT
        TOKEN( "visible-area-top" ),           // XML_VISIBLE_AREA_TOP
        TOKEN( "visible-area-width" ),         // XML_VISIBLE_AREA_WIDTH
        TOKEN( "visited-style-name" ),         // XML_VISITED_STYLE_NAME
        TOKEN( "volatile" ),                   // XML_VOLATILE
        TOKEN( "volume" ),                     // XML_VOLUME
        TOKEN( "vpn" ),                        // XML_VPN
        TOKEN( "vrp" ),                        // XML_VRP
        TOKEN( "vup" ),                        // XML_VUP
        TOKEN( "wall" ),                       // XML_WALL
        TOKEN( "warning" ),                    // XML_WARNING
        TOKEN( "watermark" ),                  // XML_WATERMARK
        TOKEN( "wavyline" ),                   // XML_WAVYLINE
        TOKEN( "wavyline-from-bottom" ),       // XML_WAVYLINE_FROM_BOTTOM
        TOKEN( "wavyline-from-left" ),         // XML_WAVYLINE_FROM_LEFT
        TOKEN( "wavyline-from-right" ),        // XML_WAVYLINE_FROM_RIGHT
        TOKEN( "wavyline-from-top" ),          // XML_WAVYLINE_FROM_TOP
        TOKEN( "week-of-year" ),               // XML_WEEK_OF_YEAR
        TOKEN( "bold" ),                       // XML_WEIGHT_BOLD
        TOKEN( "normal" ),                     // XML_WEIGHT_NORMAL
        TOKEN( "white" ),                      // XML_WHITE
        TOKEN( "whole-page" ),                 // XML_WHOLE_PAGE
        TOKEN( "widows" ),                     // XML_WIDOWS
        TOKEN( "width" ),                      // XML_WIDTH
        TOKEN( "word" ),                       // XML_WORD
        TOKEN( "word-count" ),                 // XML_WORD_COUNT
        TOKEN( "wrap" ),                       // XML_WRAP
        TOKEN( "wrap-contour" ),               // XML_WRAP_CONTOUR
        TOKEN( "wrap-contour-mode" ),          // XML_WRAP_CONTOUR_MODE
        TOKEN( "wrap-option" ),                // XML_WRAP_OPTION
        TOKEN( "writing-mode" ),               // XML_WRITING_MODE
        TOKEN( "www" ),                        // XML_WWW
        TOKEN( "x" ),                          // XML_X
        TOKEN( "x1" ),                         // XML_X1
        TOKEN( "x2" ),                         // XML_X2
        TOKEN( "x-mac-roman" ),                // XML_X_MAC_ROMAN
        TOKEN( "x-symbol" ),                   // XML_X_SYMBOL
        TOKEN( "x-system" ),                   // XML_X_SYSTEM
        TOKEN( "xor" ),                        // XML_XOR
        TOKEN( "y" ),                          // XML_Y
        TOKEN( "y1" ),                         // XML_Y1
        TOKEN( "y2" ),                         // XML_Y2
        TOKEN( "year" ),                       // XML_YEAR
        TOKEN( "yellow" ),                     // XML_YELLOW
        TOKEN( "zero-values" ),                // XML_ZERO_VALUES
        TOKEN( "z-index" ),                    // XML_ZINDEX
        TOKEN( "zoom-type" ),                  // XML_ZOOM_TYPE
        TOKEN( "zoom-value" ),                 // XML_ZOOM_VALUE

        TOKEN( "enable" ),                     // XML_ENABLE
        TOKEN( "use-regular-expressions" ),    // XML_USE_REGULAR_EXPRESSIONS
        TOKEN( "data-source-has-labels" ),     // XML_DATA_SOURCE_HAS_LABELS
        TOKEN( "link-data-style-to-source" ),  // XML_LINK_DATA_STYLE_TO_SOURCE
        TOKEN( "sort-algorithm" ),             // XML_SORT_ALGORITHM
        TOKEN( "straight-line" ),              // XML_STRAIGHT_LINE
        TOKEN( "angled-line" ),                // XML_ANGLED_LINE
        TOKEN( "angled-connector-line" ),      // XML_ANGLED_CONNECTOR_LINE

        TOKEN( "application/x-www-form-urlencoded" ), // XML_APPLICATION_X_WWW_FORM_URLENCODED
        TOKEN( "multipart/formdata" ),         // XML_MULTIPART_FORMDATA
        TOKEN( "application/text" ),           // XML_APPLICATION_TEXT
        TOKEN( "get" ),                        // XML_GET
        TOKEN( "post" ),                       // XML_POST
        TOKEN( "query" ),                      // XML_QUERY
        TOKEN( "parent" ),                     // XML_PARENT
        TOKEN( "records" ),                    // XML_RECORDS
        TOKEN( "push" ),                       // XML_PUSH
        TOKEN( "submit" ),                     // XML_SUBMIT
        TOKEN( "reset" ),                      // XML_RESET
        TOKEN( "value-list" ),                 // XML_VALUE_LIST
        TOKEN( "sql" ),                        // XML_SQL
        TOKEN( "sql-pass-through" ),           // XML_SQL_PASS_THROUGH
        TOKEN( "table-fields" ),               // XML_TABLE_FIELDS
        TOKEN( "unchecked" ),                  // XML_UNCHECKED
        TOKEN( "checked" ),                    // XML_CHECKED
        TOKEN( "unknown" ),                    // XML_UNKNOWN
        TOKEN( "roll-from-top" ),             // XML_ROLL_FROM_TOP

        TOKEN( "binary-data" ),                // XML_BINARY_DATA
        TOKEN( "notify-on-update-of-table" ),  // XML_NOTIFY_ON_UPDATE_OF_TABLE

        TOKEN( "0" ),                          // XML_0
        TOKEN( "play" ),                       // XML_PLAY
        TOKEN( "handout-master" ),             // XML_HANDOUT_MASTER
        TOKEN( "text-style-name" ),            // XML_TEXT_STYLE_NAME
        TOKEN( "escape-direction" ),           // XML_ESCAPE_DIRECTION
        TOKEN( "glue-point" ),                 // XML_GLUE_POINT
        TOKEN( "primary-x" ),                  // XML_PRIMARY_X
        TOKEN( "secondary-x" ),                // XML_SECONDARY_X
        TOKEN( "primary-y" ),                  // XML_PRIMARY_Y
        TOKEN( "secondary-y" ),                // XML_SECONDARY_Y
        TOKEN( "primary-z" ),                  // XML_PRIMARY_Z

        TOKEN( "caption-type" ),               // XML_CAPTION_TYPE
        TOKEN( "caption-angle-type" ),         // XML_CAPTION_ANGLE_TYPE
        TOKEN( "caption-angle" ),              // XML_CAPTION_ANGLE
        TOKEN( "caption-gap" ),                // XML_CAPTION_GAP
        TOKEN( "caption-escape-direction" ),   // XML_CAPTION_ESCAPE_DIRECTION
        TOKEN( "caption-escape" ),             // XML_CAPTION_ESCAPE
        TOKEN( "caption-line-length" ),        // XML_CAPTION_LINE_LENGTH
        TOKEN( "caption-fit-line-length" ),    // XML_CAPTION_FIT_LINE_LENGTH
        TOKEN( "free" ),                       // XML_FREE
        TOKEN( "transition-type" ),            // XML_TRANSITION_TYPE,
        TOKEN( "transition-style" ),           // XML_TRANSITION_STYLE
        TOKEN( "transition-speed" ),           // XML_TRANSITION_SPEED
        TOKEN( "duration" ),                   // XML_DURATION,
        TOKEN( "background-size" ),            // XML_BACKGROUND_SIZE,
        TOKEN( "background-objects-visible" ), // XML_BACKGROUND_OBJECTS_VISIBLE
        TOKEN( "background-visible" ),         // XML_BACKGROUND_VISIBLE

        TOKEN( "move-from-upperleft" ),        // XML_MOVE_FROM_UPPERLEFT,
        TOKEN( "move-from-upperright" ),       // XML_MOVE_FROM_UPPERRIGHT,
        TOKEN( "move-from-lowerright" ),       // XML_MOVE_FROM_LOWERRIGHT,
        TOKEN( "move-from-lowerleft" ),        // XML_MOVE_FROM_LOWERLEFT,
        TOKEN( "uncover-to-left" ),            // XML_UNCOVER_TO_LEFT,
        TOKEN( "uncover-to-upperleft" ),       // XML_UNCOVER_TO_UPPERLEFT,
        TOKEN( "uncover-to-top" ),             // XML_UNCOVER_TO_TOP,
        TOKEN( "uncover-to-upperright" ),      // XML_UNCOVER_TO_UPPERRIGHT,
        TOKEN( "uncover-to-right" ),           // XML_UNCOVER_TO_RIGHT,
        TOKEN( "uncover-to-lowerright" ),      // XML_UNCOVER_TO_LOWERRIGHT,
        TOKEN( "uncover-to-bottom" ),          // XML_UNCOVER_TO_BOTTOM,
        TOKEN( "uncover-to-lowerleft" ),       // XML_UNCOVER_TO_LOWERLEFT,
        TOKEN( "vertical-checkerboard" ),      // XML_VERTICAL_CHECKERBOARD,
        TOKEN( "horizontal-checkerboard" ),    // XML_HORIZONTAL_CHECKERBOARD,

        TOKEN( "notify-on-update-of-ranges" ),  // XML_NOTIFY_ON_UPDATE_OF_RANGES

        TOKEN( "byte" ),                        // XML_BYTE
        TOKEN( "macro" ),                       // XML_MACRO
        TOKEN( "location" ),                   // XML_LOCATION
        TOKEN( "application" ),                // XML_APPLICATION

        TOKEN( "symbol-image" ),               // XML_SYMBOL_IMAGE
        TOKEN( "text-overlap" ),               // XML_TEXT_OVERLAP
        TOKEN( "spline-order" ),               // XML_SPLINE_ORDER
        TOKEN( "spline-resolution" ),          // XML_SPLINE_RESOLUTION
        TOKEN( "paper-tray-name" ),            // XML_PAPER_TRAY_NAME

        TOKEN( "column-mapping" ),             // XML_COLUMN_MAPPING
        TOKEN( "row-mapping" ),                // XML_ROW_MAPPING

        TOKEN( "table-formula" ),               // XML_TABLE_FORMULA

        TOKEN( "embedded-text" ),              // XML_EMBEDDED_TEXT

        TOKEN( "merge-last-paragraph"),        // XML_MERGE_LAST_PARAGRAPH

        TOKEN( "stock-loss-marker" ),          // XML_STOCK_LOSS_MARKER
        TOKEN( "stock-gain-marker" ),          // XML_STOCK_GAIN_MARKER
        TOKEN( "stock-range-line" ),           // XML_STOCK_RANGE_LINE

        TOKEN( "rl-tb" ),                      // XML_RL_TB
        TOKEN( "tb-lr" ),                      // XML_TB_LR
        TOKEN( "lr" ),                         // XML_LR
        TOKEN( "rl" ),                         // XML_RL
        TOKEN( "tb" ),                         // XML_TB

        TOKEN( "layout-grid-color" ),          // XML_LAYOUT_GRID_COLOR
        TOKEN( "layout-grid-lines" ),          // XML_LAYOUT_GRID_LINES
        TOKEN( "layout-grid-base-height" ),    // XML_LAYOUT_GRID_BASE_HEIGHT
        TOKEN( "layout-grid-ruby-height" ),    // XML_LAYOUT_GRID_RUBY_HEIGHT
        TOKEN( "layout-grid-mode" ),           // XML_LAYOUT_GRID_MODE
        TOKEN( "layout-grid-ruby-below" ),     // XML_LAYOUT_GRID_RUBY_BELOW
        TOKEN( "layout-grid-print" ),          // XML_LAYOUT_GRID_PRINT
        TOKEN( "layout-grid-display" ),        // XML_LAYOUT_GRID_DISPLAY

        TOKEN( "snap-to-layout-grid" ),        // XML_SNAP_TO_LAYOUT_GRID

        TOKEN( "dont-balance-text-columns" ),  // XML_DONT_BALANCE_TEXT_COLUMNS

        TOKEN( "glyph-orientation-vertical" ), // XML_GLYPH_ORIENTATION_VERTICAL

        TOKEN( "marked-invalid" ),             //XML_MARKED_INVALID

        TOKEN( "regression-curve" ),           // XML_REGRESSION_CURVE
        TOKEN( "error-indicator" ),            // XML_ERROR_INDICATOR

        TOKEN( "table-type" ),                 // XML_TABLE_TYPE

        TOKEN( "display-factor" ),             // XML_DISPLAY_FACTOR

        TOKEN( "transliteration-format" ),     // XML_TRANSLITERATION_FORMAT
        TOKEN( "transliteration-language" ),   // XML_TRANSLITERATION_LANGUAGE
        TOKEN( "transliteration-country" ),    // XML_TRANSLITERATION_COUNTRY
        TOKEN( "transliteration-style" ),      // XML_TRANSLITERATION_STYLE

        TOKEN( "key1-phonetic" ),              // XML_KEY1_PHONETIC
        TOKEN( "key2-phonetic" ),              // XML_KEY2_PHONETIC
        TOKEN( "string-value-phonetic" ),      // XML_STRING_VALUE_PHONETIC

        TOKEN( "background-transparency" ),    // XML_BACKGROUND_OPACITY
        TOKEN( "background-image-transparency" ), // XML_BACKGROUND_IMAGE_OPACITY

        TOKEN( "dynamic-spacing" ),            // XML_DYNAMIC_SPACING

        TOKEN( "main-etry" ),                 // XML_MAIN_ENTRY

        TOKEN( "use-outline-level" ),          // XML_USE_OUTLINE_LEVEL,

        // #107245# New 3D properties which are possible for lathe and extrude 3d objects
        TOKEN( "close-front" ),                 // XML_CLOSE_FRONT
        TOKEN( "close-back" ),                  // XML_CLOSE_BACK

        TOKEN( "drop-down" ),                   // XML_DROPDOWN
        TOKEN( "current-selected" ),            // XML_CURRENT_SELECTED

        TOKEN( "join-border" ),                // XML_JOIN_BORDER

        TOKEN( "show-list" ),                   // XML_SHOW_LIST
        TOKEN( "no" ),                          // XML_NO
        TOKEN( "unsorted" ),                    // XML_UNSORTED
        TOKEN( "sorted-ascending" ),            // XML_SORTED_ASCENDING

        TOKEN( "automatic-print-range" ),       // XML_AUTOMATIC_PRINT_RANGE

        TOKEN( "font-independent-line-spacing" ),   // XML_FONT_INDEPENDENT_LINE_SPACING
        TOKEN( "selection" ),                  // XML_SELECTION
        TOKEN( "selection-indexes" ),          // XML_SELECTION_INDEXES

        TOKEN( "scale-to-X" ),                  //XML_SCALE_TO_X
        TOKEN( "scale-to-Y" ),                  //XML_SCALE_TO_Y

        TOKEN( "keep-together" ),              // XML_KEEP_TOGETHER

        TOKEN( "header-visible" ),              // XML_HEADER_VISIBLE
        TOKEN( "header-text"),                  // XML_HEADER_TEXT
        TOKEN( "footer-visible" ),              // XML_FOOTER_VISIBLE
        TOKEN( "footer-text" ),                 // XML_FOOTER_TEXT
        TOKEN( "page-number-visible" ),         // XML_PAGE_NUMBER_VISIBLE
        TOKEN( "date-time-visible" ),           // XML_DATE_TIME_VISIBLE
        TOKEN( "date-time-format" ),            // XML_DATE_TIME_FORMAT
        TOKEN( "date-time-update" ),            // XML_DATE_TIME_UPDATE
        TOKEN( "date-time-text" ),              // XML_DATE_TIME_TEXT


        TOKEN( "selected-page" ),                   // XML_SELECTED_PAGE

        // DVO, OD 01.10.2003 #i18732#
        TOKEN( "flow-with-text" ),   // XML_FLOW_WITH_TEXT
        TOKEN( "with-tab"),          // XML_WITH_TAB #i21237#

        TOKEN( "custom-shape" ),                    // XML_CUSTOM_SHAPE
        TOKEN( "engine" ),                          // XML_ENGINE
        TOKEN( "enhanced-geometry" ),               // XML_ENHANCED_GEOMETRY
        TOKEN( "predefined-type" ),                 // XML_PREDEFINED_TYPE
        TOKEN( "coordinate-origin-x" ),             // XML_COORDINATE_ORIGIN_X
        TOKEN( "coordinate-origin-y" ),             // XML_COORDINATE_ORIGIN_Y
        TOKEN( "coordinate-width" ),                // XML_COORDINATE_WIDTH
        TOKEN( "coordinate-height" ),               // XML_COORDINATE_HEIGHT
        TOKEN( "text-rotate-angle" ),               // XML_TEXT_ROTATE_ANGLE
        TOKEN( "mirror-vertical" ),                 // XML_MIRROR_VERTICAL
        TOKEN( "mirror-horizontal" ),               // XML_MIRROR_HORIZONTAL
        TOKEN( "extrusion-allowed" ),               // XML_EXTRUSION_ALLOWED
        TOKEN( "text-path-allowed" ),               // XML_TEXT_PATH_ALLOWED
        TOKEN( "conentric-gradient-fill-allowed" ), // XML_CONCENTRIC_GRADIENT_FILL_ALLOWED
        TOKEN( "extrusion" ),                       // XML_EXTRUSION
        TOKEN( "extrusion-auto-rotation-center" ),  // XML_EXTRUSION_AUTO_ROTATION_CENTER
        TOKEN( "extrusion-backward-depth" ),        // XML_EXTRUSION_BACKWARD_DEPTH
        TOKEN( "extrusion-brightness" ),            // XML_EXTRUSION_BRIGHTNESS
        TOKEN( "extrusion-diffusity" ),             // XML_EXTRUSION_DIFFUSITY
        TOKEN( "extrusion-edge" ),                  // XML_EXTRUSION_EDGE
        TOKEN( "extrusion-facet" ),                 // XML_EXTRUSION_FACET
        TOKEN( "extrusion-foreward-depth" ),        // XML_EXTRUSION_FOREWARD_DEPTH
        TOKEN( "extrusion-light-face" ),            // XML_EXTRUSION_LIGHT_FACE
        TOKEN( "extrusion-light-harsh1" ),          // XML_EXTRUSION_LIGHT_HARSH1
        TOKEN( "extrusion-light-harsh2" ),          // XML_EXTRUSION_LIGHT_HARSH2
        TOKEN( "extrusion-light-level1" ),          // XML_EXTRUSION_LIGHT_LEVEL1
        TOKEN( "extrusion-light-level2" ),          // XML_EXTRUSION_LIGHT_LEVEL2
        TOKEN( "extrusion-light-direction1" ),      // XML_EXTRUSION_LIGHT_DIRECTION1
        TOKEN( "extrusion-light-direction2" ),      // XML_EXTRUSION_LIGHT_DIRECTION2
        TOKEN( "extrusion-metal" ),                 // XML_EXTRUSION_METAL
        TOKEN( "extrusion-plane" ),                 // XML_EXTRUSION_PLANE
        TOKEN( "extrusion-render-mode" ),           // XML_EXTRUSION_RENDER_MODE
        TOKEN( "extrusion-rotation-angle-x" ),      // XML_EXTRUSION_ROTATION_ANGLE_X
        TOKEN( "extrusion-rotation-angle-y" ),      // XML_EXTRUSION_ROTATION_ANGLE_Y
        TOKEN( "extrusion-rotation-center-x" ),     // XML_EXTRUSION_ROTATION_CENTER_X
        TOKEN( "extrusion-rotation-center-y" ),     // XML_EXTRUSION_ROTATION_CENTER_Y
        TOKEN( "extrusion-rotation-center-z" ),     // XML_EXTRUSION_ROTATION_CENTER_Z
        TOKEN( "extrusion-shininess" ),             // XML_EXTRUSION_SHININESS
        TOKEN( "extrusion-skew" ),                  // XML_EXTRUSION_SKEW
        TOKEN( "extrusion-skew-angle" ),            // XML_EXTRUSION_SKEW_ANGLE
        TOKEN( "extrusion-specularity" ),           // XML_EXTRUSION_SPECULARITY
        TOKEN( "extrusion-parallel" ),              // XML_EXTRUSION_PARALLEL
        TOKEN( "extrusion-viewpoint" ),             // XML_EXTRUSION_VIEWPOINT
        TOKEN( "extrusion-origin-x" ),              // XML_EXTRUSION_ORIGIN_X
        TOKEN( "extrusion-origin-y" ),              // XML_EXTRUSION_ORIGIN_Y
        TOKEN( "extrusion-color" ),                 // XML_EXTRUSION_COLOR
        TOKEN( "secondary-fill-color" ),            // XML_SECONDARY_FILL_COLOR
        TOKEN( "enhanced-path" ),                   // XML_ENHANCED_PATH
        TOKEN( "path-stretchpoint-x" ),             // XML_PATH_STRETCHPOINT_X
        TOKEN( "path-stretchpoint-y" ),             // XML_PATH_STRETCHPOINT_Y
        TOKEN( "text-frames" ),                     // XML_TEXT_FRAMES
        TOKEN( "glue-points" ),                     // XML_GLUE_POINTS
        TOKEN( "glue-point-type" ),                 // XML_GLUE_POINT_TYPE
        TOKEN( "glue-point-leaving-direction" ),    // XML_GLUE_POINT_LEAVING_DIRECTIONS
        TOKEN( "text-path" ),                       // XML_TEXT_PATH
        TOKEN( "text-path-fit-text" ),              // XML_TEXT_PATH_FIT_TEXT
        TOKEN( "text-path-fit-shape" ),             // XML_TEXT_PATH_FIT_SHAPE
        TOKEN( "text-path-scale-x" ),               // XML_TEXT_PATH_SCALE_X
        TOKEN( "text-path-same-letter-heights" ),   // XML_TEXT_PATH_SAME_LETTER_HEIGHTS
        TOKEN( "adjustments" ),                     // XML_ADJUSTMENTS
        TOKEN( "equation" ),                        // XML_EQUATION
        TOKEN( "handle" ),                          // XML_HANDLE
        TOKEN( "handle-mirror-vertical" ),          // XML_HANDLE_MIRROR_VERTICAL
        TOKEN( "handle-mirror-horizontal" ),        // XML_HANDLE_MIRROR_HORIZONTAL
        TOKEN( "handle-switched" ),                 // XML_HANDLE_SWITCHED
        TOKEN( "handle-position" ),                 // XML_HANDLE_POSITION
        TOKEN( "handle-range-x-minimum" ),          // XML_HANDLE_RANGE_X_MINIMUM
        TOKEN( "handle-range-x-maximum" ),          // XML_HANDLE_RANGE_X_MAXIMUM
        TOKEN( "handle-range-y-minimum" ),          // XML_HANDLE_RANGE_Y_MINIMUM
        TOKEN( "handle-range-y-maximum" ),          // XML_HANDLE_RANGE_Y_MAXIMUM
        TOKEN( "handle-polar" ),                    // XML_HANDLE_POLAR
        TOKEN( "handle-radius-range-minimum" ),     // XML_HANDLE_RADIUS_RANGE_MINIMUM
        TOKEN( "handle-radius-range-maximum" ),     // XML_HANDLE_RADIUS_RANGE_MAXIMUM
        TOKEN( "callout" ),                         // XML_CALLOUT
        TOKEN( "callout-accent-bar" ),              // XML_CALLOUT_ACCENT_BAR
        TOKEN( "callout-angle" ),                   // XML_CALLOUT_ANGLE
        TOKEN( "callout-drop-distance" ),           // XML_CALLOUT_DROP_DISTANCE
        TOKEN( "callout-drop" ),                    // XML_CALLOUT_DROP
        TOKEN( "callout-drop-automatic" ),          // XML_CALLOUT_DROP_AUTOMATIC
        TOKEN( "callout-gap" ),                     // XML_CALLOUT_GAP
        TOKEN( "callout-length" ),                  // XML_CALLOUT_LENGTH
        TOKEN( "callout-length-specified" ),        // XML_CALLOUT_LENGTH_SPECIFIED
        TOKEN( "callout-flip-x" ),                  // XML_CALLOUT_FLIP_X
        TOKEN( "callout-flip-y" ),                  // XML_CALLOUT_FLIP_Y
        TOKEN( "callout-text-border" ),             // XML_CALLOUT_TEXT_BORDER
        TOKEN( "callout-type" ),                    // XML_CALLOUT_TYPE
        TOKEN( "rectangle" ),                       // XML_RECTANGLE
        TOKEN( "roundrectangle" ),                  // XML_ROUNDRECTANGLE
        TOKEN( "oval" ),                            // XML_OVAL
        TOKEN( "cloud" ),                           // XML_CLOUD
        TOKEN( "on" ),                              // XML_ON
        TOKEN( "off" ),                             // XML_OFF
        TOKEN( "xy" ),                              // XML_XY
        TOKEN( "zx" ),                              // XML_ZX
        TOKEN( "yz" ),                              // XML_YZ
        TOKEN( "boundingcube" ),                    // XML_BOUNDINGCUBE
        TOKEN( "wireframe" ),                       // XML_WIREFRAME
        TOKEN( "mid" ),                             // XML_MID
        TOKEN( "if" ),                              // XML_IF
        TOKEN( "mod" ),                             // XML_MOD
        TOKEN( "atan2" ),                           // XML_ATAN2
        TOKEN( "cosatan2" ),                        // XML_COSATAN2
        TOKEN( "sinatan2" ),                        // XML_SINATAN2
        TOKEN( "sqrt" ),                            // XML_SQRT
        TOKEN( "sumangle" ),                        // XML_SUMANGLE
        TOKEN( "segments" ),                        // XML_SEGMENTS
        TOKEN( "word-wrap" ),                       // XML_WORD_WRAP
        TOKEN( "auto-grow-size" ),                  // XML_AUTO_GROW_SIZE

        TOKEN( "collapsing" ),                      // XML_COLLAPSING
        TOKEN( "separating" ),                      // XML_SEPARATING
        TOKEN( "border-model" ),                    // XML_BORDER_MODEL

        TOKEN( "data-pilot-field-reference"),       // XML_DATA_PILOT_FIELD_REFERENCE
        TOKEN( "member-difference"),                // XML_MEMBER_DIFFERENCE
        TOKEN( "member-percentage"),                // XML_MEMBER_PERCENTAGE
        TOKEN( "member-percentage-difference"),     // XML_MEMBER_PERCENTAGE_DIFFERENCE
        TOKEN( "running-total"),                    // XML_RUNNING_TOTAL
        TOKEN( "row-percentage"),                   // XML_ROW_PERCENTAGE
        TOKEN( "column-percentage"),                // XML_COLUMN_PERCENTAGE
        TOKEN( "total-percentage"),                 // XML_TOTAL_PERCENTAGE
        TOKEN( "field-name"),                       // XML_FIELD_NAME
        TOKEN( "member-type"),                      // XML_MEMBER_TYPE
        TOKEN( "named"),                            // XML_NAMED
        TOKEN( "member-name"),                      // XML_MEMBER_NAME
        TOKEN( "display-member-mode"),              // XML_DISPLAY_MEMBER_MODE
        TOKEN( "member-count"),                     // XML_MEMBER_COUNT
        TOKEN( "data-field"),                       // XML_DATA_FIELD
        TOKEN( "data-pilot-display-info"),          // XML_DATA_PILOT_DISPLAY_INFO
        TOKEN( "sort-mode"),                        // XML_SORT_MODE
        TOKEN( "data-pilot-sort-info"),             // XML_DATA_PILOT_SORT_INFO
        TOKEN( "add-empty-line"),                   // XML_ADD_EMPTY_LINE
        TOKEN( "tabular-layout"),                   // XML_TABULAR_LAYOUT
        TOKEN( "outline-subtotals-top"),            // XML_OUTLINE_SUBTOTALS_TOP
        TOKEN( "outline-subtotals-bottom"),         // XML_OUTLINE_SUBTOTALS_BOTTOM
        TOKEN( "layout-mode"),                      // XML_LAYOUT_MODE
        TOKEN( "data-pilot-layout-info"),           // XML_DATA_PILOT_LAYOUT_INFO

        TOKEN( "symbol-color"),                     // XML_SYMBOL_COLOR

        TOKEN( "3d" ),                          // XML_3D
        TOKEN( "image-position" ),              // XML_IMAGE_POSITION
        TOKEN( "image-align" ),                 // XML_IMAGE_ALIGN

        // Names for OOo format only
        TOKEN( "http://openoffice.org/2000/office" ), // XML_n_office
        TOKEN( "http://openoffice.org/2000/meta" ), // XML_n_meta
        TOKEN( "http://openoffice.org/2000/style" ), // XML_n_style
        TOKEN( "http://openoffice.org/2000/datastyle" ), // XML_n_number
        TOKEN( "http://openoffice.org/2000/text" ), // XML_n_text
        TOKEN( "http://openoffice.org/2000/table" ), // XML_n_table
        TOKEN( "http://openoffice.org/2000/drawing" ), // XML_n_draw
        TOKEN( "http://openoffice.org/2000/dr3d" ), // XML_n_dr3d
        TOKEN( "http://openoffice.org/2000/presentation" ), // XML_n_presentation
        TOKEN( "http://openoffice.org/2000/chart" ), // XML_n_chart
        TOKEN( "http://openoffice.org/2001/config" ), // XML_n_config
        TOKEN( "http://openoffice.org/2000/form" ), // XML_N_form
        TOKEN( "http://openoffice.org/2000/script" ), // XML_N_script

        TOKEN( "global" ),                // XML_GLOBAL

        TOKEN( "note-class" ),                   // XML_NOTE_CLASS
        TOKEN( "note-citation" ),                   // XML_NOTE_CITATION
        TOKEN( "note-body" ),                   // XML_NOTE_BODY
        TOKEN( "notes-configuration" ),          // XML_NOTES_CONFIGURATION
        TOKEN( "note-ref" ),                   // XML_NOTE_REF
        TOKEN( "is-sub-table" ),                   // XML_IS_SUB_TABLE
        TOKEN( "page-layout" ),                   // XML_PAGE_LAYOUT
        TOKEN( "page-layout-name" ),                  // XML_PAGE_LAYOUT_NAME
        TOKEN( "graphic-properties" ),         // XML_GRAPHIC_PROPERTIES,
        TOKEN( "drawing-page-properties" ),    // XML_DRAWING_PAGE_PROPERTIES,
        TOKEN( "page-layout-properties" ),     // XML_PAGE_LAYOUT_PROPERTIES,
        TOKEN( "header-footer-properties" ),   // XML_HEADER_FOOTER_PROPERTIES
        TOKEN( "text-properties" ),            // XML_TEXT_PROPERTIES,
        TOKEN( "paragraph-properties" ),       // XML_PARAGRAPH_PROPERTIES,
        TOKEN( "ruby-properties" ),            // XML_RUBY_PROPERTIES,
        TOKEN( "section-properties" ),         // XML_SECTION_PROPERTIES,
        TOKEN( "table-properties" ),           // XML_TABLE_PROPERTIES,
        TOKEN( "table-column-properties" ),    // XML_TABLE_COLUMN_PROPERTIES,
        TOKEN( "table-row-properties" ),       // XML_TABLE_ROW_PROPERTIES,
        TOKEN( "table-cell-properties" ),      // XML_TABLE_CELL_PROPERTIES
        TOKEN( "list-level-properties" ),      // XML_LIST_LEVEL_PROPERTIES
        TOKEN( "chart-properties" ),           // XML_CHART_PROPERTIES
        TOKEN( "drawing-page" ),               // XML_DRAWING_PAGE
        TOKEN( "graphics" ),                   // XML_GRAPHICS
        TOKEN( "tab" ),                         // XML_TAB
        TOKEN( "text-underline-mode" ),         // XML_TEXT_UNDERLINE_MODE
        TOKEN( "text-line-though-mode" ),       // XML_TEXT_LINE_THROUGH_MODE
        TOKEN( "continuous" ),                  // XML_CONTINUOUS
        TOKEN( "skip-white-space" ),            // XML_SKIP_WHITE_SPACE
        TOKEN( "scripts" ),                     // XML_SCRIPT
        TOKEN( "font-face-decls" ),            // XML_FONT_FACE_DECLS
        TOKEN( "font-face" ),                   // XML_FONT_FACE
        TOKEN( "font-adornments" ),            // XML_FONT_ADORNMENTS
        TOKEN( "inch" ),                        // XML_INCH
        TOKEN( "space-after" ),                 // XML_SPACE_AFTER
        TOKEN( "start-indent" ),                // XML_START_INDENT
        TOKEN( "end-indent" ),                  // XML_END_INDENT

        // chart Oasis format additions
        TOKEN( "interval-minor-divisor" ),      // XML_INTERVAL_MINOR_DIVISOR
        TOKEN( "date-string" ),                 // XML_DATE_STRING

        TOKEN( "text-underline-style" ),        // XML_TEXT_UNDERLINE_STYLE
        TOKEN( "text-underline-type" ),         // XML_TEXT_UNDERLINE_TYPE
        TOKEN( "text-underline-width" ),        // XML_TEXT_UNDERLINE_WIDTH

        TOKEN( "text-line-through-style" ),     // XML_TEXT_LINE_THROUGH_STYLE
        TOKEN( "text-line-through-type" ),      // XML_TEXT_LINE_THROUGH_TYPE
        TOKEN( "text-line-through-width" ),     // XML_TEXT_LINE_THROUGH_WIDTH
        TOKEN( "text-line-through-text" ),      // XML_TEXT_LINE_THROUGH_TEXT

        TOKEN( "leader-style" ),                // XML_LEADER_STYLE
        TOKEN( "leader-text" ),                 // XML_LEADER_TEXT

        TOKEN( "bold-dotted" ),                 // XML_BOLD_DOTTED,
        TOKEN( "bold-dash" ),                   // XML_BOLD_DASH,
        TOKEN( "bold-long-dash" ),              // XML_BOLD_LONG_DASH,
        TOKEN( "bold-dot-dash" ),               // XML_BOLD_DOT_DASH,
        TOKEN( "bold-dot-dot-dash" ),           // XML_BOLD_DOT_DOT_DASH,
        TOKEN( "bold-wave" ),                   // XML_BOLD_WAVE,
        TOKEN( "double-wave" ),                 // XML_DOUBLE_WAVE,
        TOKEN( "double-line" ),                 // XML_DOUBLE_LINE,
        TOKEN( "thick-line" ),                  // XML_THICK_LINE,
        TOKEN( "single-line" ),                 // XML_SINGLE_LINE,
        TOKEN( "slash" ),                       // XML_SLASH,
        TOKEN( "text-line-through-color" ),     // XML_TEXT_LINE_THROUGH_COLOR,
        TOKEN( "text-line-through-text-style" ),// XML TEXT_LINE_THROUGH_TEXT_STYLE
        TOKEN( "leader-color" ),                // XML_LEADER_COLOR
        TOKEN( "leader-type" ),                 // XML_LEADER_TYPE
        TOKEN( "leader-width" ),                // XML_LEADER_WIDTH
        TOKEN( "leader-text-style" ),           // XML_LEADER_TEXT_STYLE

        TOKEN( "opacity" ),                     // XML_OPACITY
        TOKEN( "opacity-name" ),                // XML_OPACITY_NAME
        TOKEN( "shadow-opacity" ),              // XML_SHADOW_OPACITY
        TOKEN( "always" ),                      // XML_ALWAYS
        TOKEN( "count-in-text-boxes" ),         // XML_COUNT_IN_TEXT_BOXES

        TOKEN( "ooo" ),                         // XML_NP_OOO
        TOKEN( "http://openoffice.org/2004/office" ), // XML_N_OOO
        TOKEN( "ooow" ),                        // XML_NP_OOOW
        TOKEN( "http://openoffice.org/2004/writer" ), // XML_N_OOOW
        TOKEN( "oooc" ),                        // XML_NP_OOOC
        TOKEN( "http://openoffice.org/2004/calc" ), // XML_N_OOOC
        TOKEN( "dom" ),                         // XML_NP_DOM
        TOKEN( "http://www.w3.org/2001/xml-events" ), // XML_N_DOM

        TOKEN( "event-listeners" ), // XML_EVENT_LISTENERS
        TOKEN( "event-listener" ), // XML_EVENT_LISTENER

        TOKEN( "form" ),            // XML_FORM
        TOKEN( "void" ),            // XML_VOID
        TOKEN( "property" ),            // XML_PROPERTY
        TOKEN( "property-name" ),           // XML_PROPERTY_NAME
        TOKEN( "list-property" ),           // XML_LIST_PROPERTY
        TOKEN( "list-value" ),          // XML_LIST_VALUE
        TOKEN( "column-style-name" ),       // XML_COLUMN_STYLE_NAME
        TOKEN( "textarea" ),                // XML_TEXTAREA,
        TOKEN( "fixed-text" ),              // XML_FIXED_TEXT,
        TOKEN( "file" ),                    // XML_FILE,
        TOKEN( "formatted-text" ),          // XML_FORMATTED_TEXT,
        TOKEN( "button" ),                  // XML_BUTTON,
        TOKEN( "checkbox" ),                // XML_CHECKBOX,
        TOKEN( "radio" ),                   // XML_RADIO,
        TOKEN( "listbox" ),                 // XML_LISTBOX,
        TOKEN( "combobox" ),                // XML_COMBOBOX,
        TOKEN( "image-frame" ),             // XML_IMAGE_FRAME,
        TOKEN( "value-range" ),             // XML_VALUE_RANGE,
        TOKEN( "generic-control" ),         // XML_GENERIC_CONTROL,
        TOKEN( "service-name" ),            // XML_SERVICE_NAME,
        TOKEN( "property-type" ),           // XML_PROPERTY_TYPE,
        TOKEN( "integer" ),                 // XML_INTEGER,
        TOKEN( "property-is-void" ),        // XML_PROPERTY_IS_VOID,
        TOKEN( "property-is-list" ),        // XML_PROPERTY_IS_LIST,
        TOKEN( "property-value" ),          // XML_PROPERTY_VALUE,
        TOKEN( "mimetype" ),                // XML_MIMETYPE
        TOKEN( "database-row-select" ),     // XML_DATABASE_ROW_SELECT
        TOKEN( "control-implementation" ),  // XML_CONTROL_IMPLEMENTATION
        TOKEN( "interpolation" ),           // XML_INTERPOLATION
        TOKEN( "cubic-spline" ),            // XML_CUBIC_SPLINE
        TOKEN( "b-spline" ),                // XML_B_SPLINE

        TOKEN( "show-filter-button"),               // XML_SHOW_FILTER_BUTTON,
        TOKEN( "drill-down-on-double-click"),       // XML_DRILL_DOWN_ON_DOUBLE_CLICK,

        { 0, NULL, NULL }                       // XML_TOKEN_END
    };



    // get OUString representation of token
    const OUString& GetXMLToken( enum XMLTokenEnum eToken )
    {
        DBG_ASSERT( eToken > XML_TOKEN_INVALID, "token value too low!" );
        DBG_ASSERT( eToken < XML_TOKEN_END, "token value too high!" );

        XMLTokenEntry* pToken = &aTokenList[(sal_uInt16)eToken];
        if (!pToken->pOUString)
            pToken->pOUString = new OUString( pToken->pChar, pToken->nLength,
                                RTL_TEXTENCODING_ASCII_US );
        return *pToken->pOUString;
    }

    // does rString represent eToken?
    sal_Bool IsXMLToken(
        const OUString& rString,
        enum XMLTokenEnum eToken )
    {
        DBG_ASSERT( eToken > XML_TOKEN_INVALID, "token value too low!" );
        DBG_ASSERT( eToken < XML_TOKEN_END, "token value too high!" );

        const XMLTokenEntry* pToken = &aTokenList[(sal_uInt16)eToken];
        return rString.equalsAsciiL( pToken->pChar, pToken->nLength );
    }

    // gives all allocated memory for OUString* back
    void ResetTokens()
    {
        for (sal_Int16 i=0, nEnd = sizeof ( aTokenList ) / sizeof ( XMLTokenEntry );
             i < nEnd;
             i++)
        {
            delete aTokenList[i].pOUString;
            aTokenList[i].pOUString = NULL;
        }
    }
}
}

