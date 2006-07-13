/************************************************************************
 *
 *  XMLString.java
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
 *  Copyright: 2002-2005 by Henrik Just
 *
 *  All Rights Reserved.
 *
 *  Version 0.3.3j (2005-03-07)
 *
 */

package writer2latex.office;

/* XML strings (tags and attributes) in the OOo XML namespaces
 * typosafe but not typesafe :-)
 */

public class XMLString {
    // draw namespace - elements
    public static final String DRAW_PAGE="draw:page";
    public static final String DRAW_A="draw:a";
    public static final String DRAW_IMAGE="draw:image";
    public static final String DRAW_OBJECT="draw:object";
    public static final String DRAW_TEXT_BOX="draw:text-box";
    // draw namespace - attributes
    public static final String DRAW_NAME="draw:name";
    public static final String DRAW_STYLE_NAME="draw:style-name";
    // dc namespace - elements
    public static final String DC_CREATOR="dc:creator";
    public static final String DC_DATE="dc:date";
    public static final String DC_DESCRIPTION="dc:description";
    public static final String DC_LANGUAGE="dc:language";
    public static final String DC_SUBJECT="dc:subject";
    public static final String DC_TITLE="dc:title";
    // meta namespace - elements
    public static final String META_INITIAL_CREATOR="meta:initial-creator";
    public static final String META_KEYWORDS="meta:keywords";
    public static final String META_KEYWORD="meta:keyword";
    // manifest namespace
    public static final String MANIFEST_FILE_ENTRY="manifest:file-entry";
    public static final String MANIFEST_MEDIA_TYPE="manifest:media-type";
    public static final String MANIFEST_FULL_PATH="manifest:full-path";
    // office namespace - elements
    public static final String OFFICE_DOCUMENT_CONTENT="office:document-content";
    public static final String OFFICE_MASTER_STYLES="office:master-styles";
    public static final String OFFICE_STYLES="office:styles";
    public static final String OFFICE_AUTOMATIC_STYLES="office:automatic-styles";
    public static final String OFFICE_FONT_DECLS="office:font-decls";
    public static final String OFFICE_BODY="office:body";
    public static final String OFFICE_ANNOTATION="office:annotation";
    public static final String OFFICE_BINARY_DATA="office:binary-data";
    public static final String OFFICE_META="office:meta";

    // style namespace - elements
    public static final String STYLE_PAGE_MASTER="style:page-master";
    public static final String STYLE_MASTER_PAGE="style:master-page";
    public static final String STYLE_FONT_DECL="style:font-decl";
    public static final String STYLE_STYLE="style:style";
    public static final String STYLE_DEFAULT_STYLE="style:default-style";
    public static final String STYLE_PROPERTIES="style:properties";
    public static final String STYLE_COLUMNS="style:columns";
    public static final String STYLE_HEADER="style:header";
    public static final String STYLE_HEADER_LEFT="style:header-left";
    public static final String STYLE_FOOTER="style:footer";
    public static final String STYLE_FOOTER_LEFT="style:footer-left";
    public static final String STYLE_FOOTNOTE_SEP="style:footnote-sep";
    public static final String STYLE_HEADER_STYLE="style:header-style";
    public static final String STYLE_FOOTER_STYLE="style:footer-style";
    // style namespace - attributes
    public static final String STYLE_NEXT_STYLE_NAME="style:next-style-name";
    public static final String STYLE_PAGE_MASTER_NAME="style:page-master-name";
    public static final String STYLE_MASTER_PAGE_NAME="style:master-page-name";
    public static final String STYLE_PAGE_USAGE="style:page-usage";
    //public static final String STYLE_FONT_FAMILY="style:font-family";
    public static final String STYLE_FONT_FAMILY_COMPLEX="style:font-family-complex";
    public static final String STYLE_FONT_NAME="style:font-name";
    public static final String STYLE_FONT_NAME_COMPLEX="style:font-name-complex";
    public static final String STYLE_FONT_PITCH="style:font-pitch";
    public static final String STYLE_FONT_FAMILY_GENERIC="style:font-family-generic";
    public static final String STYLE_TEXT_BACKGROUND_COLOR="style:text-background-color";
    public static final String STYLE_TEXT_CROSSING_OUT="style:text-crossing-out";
    public static final String STYLE_TEXT_UNDERLINE="style:text-underline";
    public static final String STYLE_TEXT_BLINKING="style:text-blinking";
    public static final String STYLE_AUTO_TEXT_INDENT="style:auto-text-indent";
    public static final String STYLE_TEXT_ALIGN_SOURCE="style:text-align-source";
    public static final String STYLE_NAME="style:name";
    public static final String STYLE_PARENT_STYLE_NAME="style:parent-style-name";
    public static final String STYLE_FAMILY="style:family";
    public static final String STYLE_TEXT_POSITION="style:text-position";
    public static final String STYLE_NUM_PREFIX="style:num-prefix";
    public static final String STYLE_NUM_SUFFIX="style:num-suffix";
    public static final String STYLE_NUM_FORMAT="style:num-format";
    public static final String STYLE_MAY_BREAK_BETWEEN_ROWS="style:may-break-between-rows";
    public static final String STYLE_HORIZONTAL_POS="style:horizontal-pos";
    public static final String STYLE_WRAP="style:wrap";
    public static final String STYLE_COLUMN_WIDTH="style:column-width";
    public static final String STYLE_ROW_HEIGHT="style:row-height";
    public static final String STYLE_MIN_ROW_HEIGHT="style:min-row-height";
    public static final String STYLE_FIRST_PAGE_NUMBER="style:first-page-number";
    public static final String STYLE_DISTANCE_BEFORE_SEP="style:distance-before-sep";
    public static final String STYLE_DISTANCE_AFTER_SEP="style:distance-after-sep";
    public static final String STYLE_WIDTH="style:width";
    public static final String STYLE_REL_WIDTH="style:rel-width";
    public static final String STYLE_COLOR="style:color";
    public static final String STYLE_WRITING_MODE="style:writing-mode";

    // table namespace - elements
    public static final String TABLE_TABLE="table:table";
    public static final String TABLE_SUB_TABLE="table:sub-table";
    public static final String TABLE_TABLE_COLUMN="table:table-column";
    public static final String TABLE_TABLE_COLUMNS="table:table-columns";
    public static final String TABLE_TABLE_COLUMN_GROUP="table:table-column-group";
    public static final String TABLE_TABLE_HEADER_COLUMNS="table:table-header-columns";
    public static final String TABLE_TABLE_ROW="table:table-row";
    public static final String TABLE_TABLE_ROWS="table:table-rows";
    public static final String TABLE_TABLE_ROW_GROUP="table:table-row-group";
    public static final String TABLE_TABLE_HEADER_ROWS="table:table-header-rows";
    public static final String TABLE_TABLE_CELL="table:table-cell";
    public static final String TABLE_COVERED_TABLE_CELL="table:covered-table-cell";
    // table namespace - attributes
    public static final String TABLE_NAME="table:name";
    public static final String TABLE_STYLE_NAME="table:style-name";
    public static final String TABLE_VISIBILITY="table:visibility";
    public static final String TABLE_DISPLAY="table:display";
    public static final String TABLE_DEFAULT_CELL_STYLE_NAME="table:default-cell-style-name";
    public static final String TABLE_VALUE_TYPE="table:value-type";
    public static final String TABLE_NUMBER_COLUMNS_REPEATED="table:number-columns-repeated";
    public static final String TABLE_NUMBER_ROWS_REPEATED="table:number-rows-repeated";
    public static final String TABLE_NUMBER_ROWS_SPANNED="table:number-rows-spanned";
    public static final String TABLE_NUMBER_COLUMNS_SPANNED="table:number-columns-spanned";
    public static final String TABLE_ALIGN="table:align";

    // text namespace - elements (declarations)
    public static final String TEXT_="text:";
    public static final String TEXT_FOOTNOTES_CONFIGURATION="text:footnotes-configuration";
    public static final String TEXT_ENDNOTES_CONFIGURATION="text:endnotes-configuration";
    public static final String TEXT_SEQUENCE_DECLS="text:sequence_decls";
    public static final String TEXT_SEQUENCE_DECL="text:sequence_decl";
    public static final String TEXT_OUTLINE_STYLE="text:outline-style";
    public static final String TEXT_LIST_STYLE="text:list-style";
    public static final String TEXT_LIST_LEVEL_STYLE_NUMBER="text:list-level-style-number";
    public static final String TEXT_LIST_LEVEL_STYLE_BULLET="text:list-level-style-bullet";
    public static final String TEXT_LIST_LEVEL_STYLE_IMAGE="text:list-level-style-image";
    // text namespace - elements (block text)
    public static final String TEXT_SECTION="text:section";
    public static final String TEXT_P="text:p";
    public static final String TEXT_H="text:h";
    public static final String TEXT_ORDERED_LIST="text:ordered-list";
    public static final String TEXT_UNORDERED_LIST="text:unordered-list";
    public static final String TEXT_LIST_ITEM="text:list-item";
    public static final String TEXT_LIST_HEADER="text:list-header";
    public static final String TEXT_ALPHABETICAL_INDEX="text:alphabetical-index";
    public static final String TEXT_ALPHABETICAL_INDEX_SOURCE="text:alphabetical-index-source";
    public static final String TEXT_ALPHABETICAL_INDEX_ENTRY_TEMPLATE="text:alphabetical-index-entry-template";
    public static final String TEXT_TABLE_OF_CONTENT="text:table-of-content";
    public static final String TEXT_TABLE_OF_CONTENT_SOURCE="text:table-of-content-source";
    public static final String TEXT_TABLE_OF_CONTENT_ENTRY_TEMPLATE="text:table-of-content-entry-template";
    public static final String TEXT_ILLUSTRATION_INDEX="text:illustration-index";
    public static final String TEXT_TABLE_INDEX="text:table-index";
    public static final String TEXT_OBJECT_INDEX="text:object-index";
    public static final String TEXT_USER_INDEX="text:user-index";
    public static final String TEXT_BIBLIOGRAPHY="text:bibliography";
    public static final String TEXT_INDEX_TITLE_TEMPLATE="text:index-title-template";
    public static final String TEXT_INDEX_BODY="text:index-body";
    public static final String TEXT_INDEX_TITLE="text:index-title";
    public static final String TEXT_INDEX_SOURCE="text:index-source";
    // text namespace - elements (inline text)
    public static final String TEXT_SPAN="text:span";
    public static final String TEXT_FOOTNOTE="text:footnote";
    public static final String TEXT_ENDNOTE="text:endnote";
    public static final String TEXT_FOOTNOTE_CITATION="text:footnote-citation";
    public static final String TEXT_FOOTNOTE_BODY="text:footnote-body";
    public static final String TEXT_ENDNOTE_CITATION="text:endnote-citation";
    public static final String TEXT_ENDNOTE_BODY="text:endnote-body";
    public static final String TEXT_S="text:s";
    public static final String TEXT_TAB_STOP="text:tab-stop";
    public static final String TEXT_A="text:a";
    public static final String TEXT_LINE_BREAK="text:line-break";
    public static final String TEXT_PAGE_NUMBER="text:page-number";
    public static final String TEXT_PAGE_COUNT="text:page-count";
    public static final String TEXT_CHAPTER="text:chapter";
    public static final String TEXT_SEQUENCE="text:sequence";
    public static final String TEXT_SEQUENCE_REF="text:sequence-ref";
    public static final String TEXT_BIBLIOGRAPHY_MARK="text:bibliography-mark";
    public static final String TEXT_ALPHABETICAL_INDEX_MARK="text:alphabetical-index-mark";
    public static final String TEXT_ALPHABETICAL_INDEX_MARK_START="text:alphabetical-index-mark-start";
    public static final String TEXT_ALPHABETICAL_INDEX_MARK_END="text:alphabetical-index-mark-end";
    public static final String TEXT_REFERENCE_MARK="text:reference-mark";
    public static final String TEXT_REFERENCE_MARK_START="text:reference-mark-start";
    public static final String TEXT_REFERENCE_REF="text:reference-ref";
    public static final String TEXT_BOOKMARK="text:bookmark";
    public static final String TEXT_BOOKMARK_START="text:bookmark-start";
    public static final String TEXT_BOOKMARK_REF="text:bookmark-ref";
    public static final String TEXT_FOOTNOTE_REF="text:footnote-ref";
    public static final String TEXT_ENDNOTE_REF="text:endnote-ref";
    // text namespace - attributes
    public static final String TEXT_OUTLINE_LEVEL="text:outline-level";
    public static final String TEXT_STRING_VALUE="text:string-value";
    public static final String TEXT_KEY1="text:key1";
    public static final String TEXT_KEY2="text:key2";
    public static final String TEXT_LEVEL="text:level";
    public static final String TEXT_SPACE_BEFORE="text:space-before";
    public static final String TEXT_MIN_LABEL_WIDTH="text:min-label-width";
    public static final String TEXT_MIN_LABEL_DISTANCE="text:min-label-distance";
    public static final String TEXT_STYLE_NAME="text:style-name";
    public static final String TEXT_VISITED_STYLE_NAME="text:visited-style-name";
    public static final String TEXT_DISPLAY_LEVELS="text:display-levels";
    public static final String TEXT_CONTINUE_NUMBERING="text:continue-numbering";
    public static final String TEXT_C="text:c";
    public static final String TEXT_ID="text:id";
    public static final String TEXT_LABEL="text:label";
    public static final String TEXT_NAME="text:name";
    public static final String TEXT_REFERENCE_FORMAT="text:reference-format";
    public static final String TEXT_REF_NAME="text:ref-name";
    public static final String TEXT_CITATION_BODY_STYLE_NAME="text:citation-body-style-name";
    public static final String TEXT_CITATION_STYLE_NAME="text:citation-style-name";
    public static final String TEXT_DEFAULT_STYLE_NAME="text:default-style-name";
    public static final String TEXT_START_VALUE="text:start-value";
    public static final String TEXT_START_NUMBERING_AT="text:start-numbering-at";
    public static final String TEXT_RESTART_NUMBERING="text:restart-numbering";
    public static final String TEXT_ANCHOR_TYPE="text:anchor-type";
    public static final String TEXT_BULLET_CHAR="text:bullet-char";
    public static final String TEXT_DISPLAY="text:display";

    public static final String TEXT_IDENTIFIER="text:identifier";
    public static final String TEXT_BIBLIOGRAPHY_TYPE="text:bibliography-type";
    public static final String TEXT_BIBILIOGRAPHIC_TYPE="text:bibiliographic-type"; // bug in OOo 1.0
    public static final String TEXT_ADDRESS="text:address";
    public static final String TEXT_ANNOTE="text:annote";
    public static final String TEXT_AUTHOR="text:author";
    public static final String TEXT_BOOKTITLE="text:booktitle";
    //public static final String TEXT_CHAPTER="text:chapter";
    public static final String TEXT_EDITION="text:edition";
    public static final String TEXT_EDITOR="text:editor";
    public static final String TEXT_HOWPUBLISHED="text:howpublished";
    public static final String TEXT_INSTITUTION="text:institution";
    public static final String TEXT_JOURNAL="text:journal";
    public static final String TEXT_MONTH="text:month";
    public static final String TEXT_NOTE="text:note";
    public static final String TEXT_NUMBER="text:number";
    public static final String TEXT_ORGANIZATIONS="text:organizations";
    public static final String TEXT_PAGES="text:pages";
    public static final String TEXT_PUBLISHER="text:publisher";
    public static final String TEXT_SCHOOL="text:school";
    public static final String TEXT_SERIES="text:series";
    public static final String TEXT_TITLE="text:title";
    public static final String TEXT_REPORT_TYPE="text:report-type";
    public static final String TEXT_VOLUME="text:volume";
    public static final String TEXT_YEAR="text:year";
    public static final String TEXT_URL="text:url";
    public static final String TEXT_CUSTOM1="text:custom1";
    public static final String TEXT_CUSTOM2="text:custom2";
    public static final String TEXT_CUSTOM3="text:custom3";
    public static final String TEXT_CUSTOM4="text:custom4";
    public static final String TEXT_CUSTOM5="text:custom5";
    public static final String TEXT_ISBN="text:isbn";

    // fo namespace
    public static final String FO_LANGUAGE="fo:language";
    public static final String FO_COUNTRY="fo:country";
    public static final String FO_TEXT_SHADOW="fo:text-shadow";
    public static final String FO_COLOR="fo:color";
    public static final String FO_BACKGROUND_COLOR="fo:background-color";
    public static final String FO_TEXT_TRANSFORM="fo:text-transform";
    public static final String FO_FONT_FAMILY="fo:font-family";
    public static final String FO_FONT_SIZE="fo:font-size";
    public static final String FO_FONT_WEIGHT="fo:font-weight";
    public static final String FO_FONT_VARIANT="fo:font-variant";
    public static final String FO_FONT_STYLE="fo:font-style";
    public static final String FO_LETTER_SPACING="fo:letter-spacing";
    public static final String FO_VERTICAL_ALIGN="fo:vertical-align";
    public static final String FO_TEXT_ALIGN="fo:text-align";
    public static final String FO_TEXT_ALIGN_LAST="fo:text-align-last";
    public static final String FO_BREAK_BEFORE="fo:break-before";
    public static final String FO_BREAK_AFTER="fo:break-after";
    public static final String FO_MARGIN_LEFT="fo:margin-left";
    public static final String FO_MARGIN_RIGHT="fo:margin-right";
    public static final String FO_MARGIN_TOP="fo:margin-top";
    public static final String FO_MARGIN_BOTTOM="fo:margin-bottom";
    public static final String FO_PAGE_WIDTH="fo:page-width";
    public static final String FO_PAGE_HEIGHT="fo:page-height";
    public static final String FO_BORDER="fo:border";
    public static final String FO_BORDER_LEFT="fo:border-left";
    public static final String FO_BORDER_RIGHT="fo:border-right";
    public static final String FO_BORDER_TOP="fo:border-top";
    public static final String FO_BORDER_BOTTOM="fo:border-bottom";
    public static final String FO_PADDING="fo:padding";
    public static final String FO_PADDING_LEFT="fo:padding-left";
    public static final String FO_PADDING_RIGHT="fo:padding-right";
    public static final String FO_PADDING_TOP="fo:padding-top";
    public static final String FO_PADDING_BOTTOM="fo:padding-bottom";
    public static final String FO_LINE_HEIGHT="fo:line-height";
    public static final String FO_TEXT_INDENT="fo:text-indent";
    public static final String FO_WRAP_OPTION="fo:wrap-option";
    public static final String FO_COLUMN_COUNT="fo:column-count";

    // svg namespace
    public static final String SVG_X="svg:x";
    public static final String SVG_Y="svg:y";
    public static final String SVG_HEIGHT="svg:height";
    public static final String SVG_WIDTH="svg:width";
    // xlink namespace
    public static final String XLINK_HREF="xlink:href";
    // math namespace
    public static final String MATH_MATH="math:math";
    public static final String MATH_SEMANTICS="math:semantics";
    public static final String MATH_ANNOTATION="math:annotation";

}
