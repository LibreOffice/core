/************************************************************************
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

package org.openoffice.xmerge.converter.xml;

/**
 *  This interface contains constants for StarOffice XML tags,
 *  attributes (StarCalc cell types, etc.).
 *
 *  @author      Herbie Ong, Paul Rank
 */
public interface OfficeConstants {

    /** Element tag for <i>office:document</i>, this is the root tag. */
    public final static String TAG_OFFICE_DOCUMENT = "office:document";

    /**
     *  Element tag for <i>office:document-content</i>, this is the root
     *  tag in content.xml.
     */
    public final static String TAG_OFFICE_DOCUMENT_CONTENT = "office:document-content";

    /**
     *  Element tag for <i>office:document-styles</i>, this is the root tag
     *  in styles.xml.
     */
    public final static String TAG_OFFICE_DOCUMENT_STYLES = "office:document-styles";

    /**
     *  Attribute tag for <i>office:class</i> of element
     *  <i>office:document</i>.
     */
    public final static String ATTRIBUTE_OFFICE_CLASS = "office:class";

    /** Element tag for <i>office:styles</i>. */
    public final static String TAG_OFFICE_STYLES = "office:styles";

    /** Element tag for <i>office:automatic-styles</i>. */
    public final static String TAG_OFFICE_AUTOMATIC_STYLES = "office:automatic-styles";

    /** Element tag for <i>office:master-styles</i>. */
    public final static String TAG_OFFICE_MASTER_STYLES = "office:master-styles";

    /** Element tag for <i>office:body</i>. */
    public final static String TAG_OFFICE_BODY = "office:body";

    /** Element tag for <i>office:font-decls</i>. */
    public final static String TAG_OFFICE_FONT_DECLS = "office:font-decls";

    /** Element tag for <i>style:font-decl</i>. */
    public final static String TAG_STYLE_FONT_DECL = "style:font-decl";

    /** Attribute tag for <i>style:name</i> of element <i>style:name</i>. */
    public final static String ATTRIBUTE_STYLE_NAME = "style:name";

    /**
     *  Attribute tag for <i>style:font-pitch</i> of element
     *  <i>style:font-pitch</i>.
     */
    public final static String ATTRIBUTE_STYLE_FONT_PITCH = "style:font-pitch";

    /**
     *  Attribute tag for <i>fo:font-family</i> of element
     *  <i>fo:font-family</i>.
     */
    public final static String ATTRIBUTE_FO_FONT_FAMILY = "fo:font-family";

    /** Element tag for <i>text:p</i>. */
    public final static String TAG_PARAGRAPH = "text:p";

    /** Element tag for <i>text:h</i>. */
    public final static String TAG_HEADING = "text:h";

    /** Element tag for <i>text:s</i>. */
    public final static String TAG_SPACE = "text:s";

    /** Element tag for <i>text:tab-stop</i>. */
    public final static String TAG_TAB_STOP = "text:tab-stop";

    /** Element tag for <i>text:line-break</i>. */
    public final static String TAG_LINE_BREAK = "text:line-break";

    /** Element tag for <i>text:span</i>. */
    public final static String TAG_SPAN = "text:span";

    /** Element tag for <i>text:a</i>. */
    public final static String TAG_HYPERLINK = "text:a";

    /** Element tag for <i>text:bookmark</i>. */
    public final static String TAG_BOOKMARK = "text:bookmark";

    /** Element tag for <i>text:bookmark-start</i>. */
    public final static String TAG_BOOKMARK_START = "text:bookmark-start";

    /** Element tag for <i>text:unordered-list</i>. */
    public final static String TAG_UNORDERED_LIST = "text:unordered-list";

    /** Element tag for <i>text:ordered-list</i>. */
    public final static String TAG_ORDERED_LIST = "text:ordered-list";

    /** Element tag for <i>text:list-header</i>. */
    public final static String TAG_LIST_HEADER = "text:list-header";

    /** Element tag for <i>text:list-item</i>. */
    public final static String TAG_LIST_ITEM = "text:list-item";

    /** Attribute tag for <i>text:c</i> of element <i>text:s</i>. */
    public final static String ATTRIBUTE_SPACE_COUNT = "text:c";

    /**
     * Attribute tag for <i>text:style-name</i> of element
     *  <i>text:style-name</i>.
     */
    public final static String ATTRIBUTE_TEXT_STYLE_NAME = "text:style-name";

    /** Element tag for <i>table:table</i>. */
    public final static String TAG_TABLE = "table:table";

    /**
     *  Attribute tag for <i>table:name</i> of element
     *  <i>table:table</i>.
     */
    public final static String ATTRIBUTE_TABLE_NAME = "table:name";

    /** Element tag for <i>table:table-row</i>. */
    public final static String TAG_TABLE_ROW = "table:table-row";

    /** Element tag for <i>table:table-column</i>. */
    public final static String TAG_TABLE_COLUMN = "table:table-column";

    /** Element tag for <i>table:scenario</i>. */
    public final static String TAG_TABLE_SCENARIO = "table:scenario";

    /** Element tag for <i>table:table-cell</i>. */
    public final static String TAG_TABLE_CELL = "table:table-cell";

    /**
     *  Attribute tag for <i>table:value-type</i> of element
     *  <i>table:table-cell</i>.
     */
    public final static String ATTRIBUTE_TABLE_VALUE_TYPE = "table:value-type";

    /**
     *  Attribute tag for <i>table:number-columns-repeated</i>
     *  of element <i>table:table-cell</i>.
     */
    public final static String ATTRIBUTE_TABLE_NUM_COLUMNS_REPEATED =
        "table:number-columns-repeated";

    /**
     *  Attribute tag for <i>table:number-rows-repeated</i>
     *  of element <i>table:table-row</i>.
     */
    public final static String ATTRIBUTE_TABLE_NUM_ROWS_REPEATED =
        "table:number-rows-repeated";

    /**
     *  Attribute tag for <i>table:formula</i> of element
     *  <i>table:table-cell</i>.
     */
    public final static String ATTRIBUTE_TABLE_FORMULA = "table:formula";

    /**
     *  Attribute tag for <i>table:value</i> of element
     *  <i>table:table-cell</i>.
     */
    public final static String ATTRIBUTE_TABLE_VALUE = "table:value";

    /**
     *  Attribute tag for <i>table:date-value</i> of element
     *  <i>table:table-cell</i>.
     */
    public final static String ATTRIBUTE_TABLE_DATE_VALUE = "table:date-value";

    /**
     *  Attribute tag for <i>table:time-value</i> of element
     *  <i>table:table-cell</i>.
     */
    public final static String ATTRIBUTE_TABLE_TIME_VALUE = "table:time-value";

    /**
     *  Attribute tag for <i>table:string-value</i> of element
     *  <i>table:table-cell</i>.
     */
    public final static String ATTRIBUTE_TABLE_STRING_VALUE =
        "table:string-value";

    /**
     *  Attribute tag for <i>table:time-boolean-value</i> of element
     *  <i>table:table-cell</i>.
     */
    public final static String ATTRIBUTE_TABLE_BOOLEAN_VALUE =
        "table:boolean-value";

    /** Attribute tag for <i>table:style-name</i> of table elements. */
    public final static String ATTRIBUTE_TABLE_STYLE_NAME = "table:style-name";

    /**
     *  Attribute tag for <i>table:currency</i> of element
     *  <i>table:table-cell</i>.
     */
    public final static String ATTRIBUTE_TABLE_CURRENCY = "table:currency";

    /** The cell contains data of type <i>string</i>. */
    public final static String CELLTYPE_STRING = "string";

    /** The cell contains data of type <i>float</i>. */
    public final static String CELLTYPE_FLOAT = "float";

    /** The cell contains data of type <i>time</i>. */
    public final static String CELLTYPE_TIME = "time";

    /** The cell contains data of type <i>date</i>. */
    public final static String CELLTYPE_DATE = "date";

    /** The cell contains data of type <i>currency</i>. */
    public final static String CELLTYPE_CURRENCY = "currency";

    /** The cell contains data of type <i>boolean</i>. */
    public final static String CELLTYPE_BOOLEAN = "boolean";

    /** The cell contains data of type <i>percent</i>. */
    public final static String CELLTYPE_PERCENT = "percentage";

    /** StarWriter XML file extension. */
    public final static String SXW_FILE_EXTENSION = ".sxw";

    /** StarWriter XML <i>office:class</i> value. */
    public final static String SXW_TYPE = "text";

    /** StarCalc XML file extension. */
    public final static String SXC_FILE_EXTENSION = ".sxc";

    /** StarCalc XML <i>office:class</i> value. */
    public final static String SXC_TYPE = "spreadsheet";
}

