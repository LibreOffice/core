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

package org.openoffice.xmerge.converter.xml;

/**
 * This interface contains constants for StarOffice XML tags, attributes
 * (StarCalc cell types, etc.).
 */
public interface OfficeConstants {

    /** Element tag for <i>office:document</i>, this is the root tag. */
    String TAG_OFFICE_DOCUMENT = "office:document";

    /**
     * Element tag for <i>office:document-content</i>, this is the root tag in
     * content.xml.
     */
    String TAG_OFFICE_DOCUMENT_CONTENT = "office:document-content";

    /**
     * Element tag for <i>office:document-settings</i>, this is the root tag in
     * content.xml.
     */
    String TAG_OFFICE_DOCUMENT_SETTINGS= "office:document-settings";

    /**
     * Element tag for <i>office:document-meta</i>, this is the root tag in
     * content.xml.
     */
    String TAG_OFFICE_DOCUMENT_META= "office:document-meta";

    /**
     * Element tag for <i>office:document-styles</i>, this is the root tag in
     * styles.xml.
     */
    String TAG_OFFICE_DOCUMENT_STYLES = "office:document-styles";

    /** Element tag for <i>office:styles</i>. */
    String TAG_OFFICE_STYLES = "office:styles";

    /** Element tag for <i>office:meta</i>. */
    String TAG_OFFICE_META = "office:meta";

    /** Element tag for <i>office:automatic-styles</i>. */
    String TAG_OFFICE_AUTOMATIC_STYLES = "office:automatic-styles";

    /** Element tag for <i>office:master-styles</i>. */
    String TAG_OFFICE_MASTER_STYLES = "office:master-styles";

    /** Element tag for <i>office:body</i>. */
    String TAG_OFFICE_BODY = "office:body";

    /** Element tag for <i>office:settings</i>. */
    String TAG_OFFICE_SETTINGS = "office:settings";

    /** Element tag for <i>office:font-decls</i>. */
    String TAG_OFFICE_FONT_DECLS = "office:font-decls";

    /** Element tag for <i>style:font-decl</i>. */
    String TAG_STYLE_FONT_DECL = "style:font-decl";

    /** Attribute tag for <i>style:name</i> of element <i>style:name</i>. */
    String ATTRIBUTE_STYLE_NAME = "style:name";

    /**
     * Attribute tag for <i>style:font-pitch</i> of element
     * <i>style:font-pitch</i>.
     */
    String ATTRIBUTE_STYLE_FONT_PITCH = "style:font-pitch";

    /**
     * Attribute tag for <i>fo:font-family</i> of element
     * <i>fo:font-family</i>.
     */
    String ATTRIBUTE_FO_FONT_FAMILY = "fo:font-family";

    /**
     * Attribute tag for <i>fo:font-family</i> of element
     * <i>fo:font-family</i>.
     */
    String ATTRIBUTE_FO_FONT_FAMILY_GENERIC = "fo:font-family-generic";

    /** Element tag for <i>text:p</i>. */
    String TAG_PARAGRAPH = "text:p";

    /** Element tag for <i>text:h</i>. */
    String TAG_HEADING = "text:h";

    /** Element tag for <i>text:s</i>. */
    String TAG_SPACE = "text:s";

    /** Element tag for <i>text:tab-stop</i>. */
    String TAG_TAB_STOP = "text:tab-stop";

    /** Element tag for <i>text:line-break</i>. */
    String TAG_LINE_BREAK = "text:line-break";

    /** Element tag for <i>text:span</i>. */
    String TAG_SPAN = "text:span";

    /** Element tag for <i>text:a</i>. */
    String TAG_HYPERLINK = "text:a";

    /** Element tag for <i>text:unordered-list</i>. */
    String TAG_UNORDERED_LIST = "text:unordered-list";

    /** Element tag for <i>text:ordered-list</i>. */
    String TAG_ORDERED_LIST = "text:ordered-list";

    /** Element tag for <i>text:list-header</i>. */
    String TAG_LIST_HEADER = "text:list-header";

    /** Element tag for <i>text:list-item</i>. */
    String TAG_LIST_ITEM = "text:list-item";

    /** Attribute tag for <i>text:c</i> of element <i>text:s</i>. */
    String ATTRIBUTE_SPACE_COUNT = "text:c";

    /** Element tag for <i>table:table</i>. */
    String TAG_TABLE = "table:table";

    /** Element tag for <i>table:named-expression</i>. */
    String TAG_NAMED_EXPRESSIONS = "table:named-expressions";

    /** Element tag for <i>table:named-range</i>. */
    String TAG_TABLE_NAMED_RANGE= "table:named-range";

    /** Element tag for <i>table:named-expression</i>. */
    String TAG_TABLE_NAMED_EXPRESSION= "table:named-expression";

    /** Attribute tag for <i>table:name</i> of element <i>table:table</i>. */
    String ATTRIBUTE_TABLE_NAME = "table:name";

    /**
     * Attribute tag for <i>table:expression</i> of element
     * <i>table:named-range</i>.
     */
    String ATTRIBUTE_TABLE_EXPRESSION = "table:expression";

    /**
     * Attribute tag for <i>table:base-cell-address</i> of element
     * <i>table:named-range</i>.
     */
    String ATTRIBUTE_TABLE_BASE_CELL_ADDRESS = "table:base-cell-address";

    /**
     * Attribute tag for <i>table:cell-range-address</i> of element
     * <i>table:named-range</i>.
     */
    String ATTRIBUTE_TABLE_CELL_RANGE_ADDRESS = "table:cell-range-address";

    /** Element tag for <i>table:table-row</i>. */
    String TAG_TABLE_ROW = "table:table-row";

    /** Element tag for <i>table:table-column</i>. */
    String TAG_TABLE_COLUMN = "table:table-column";

   /**
    * Attribute tag for <i>table:default-cell-style-name</i> of element
    * <i>table:table-column</i>.
    */
    String ATTRIBUTE_DEFAULT_CELL_STYLE = "table:default-cell-style-name";

    /** Element tag for <i>table:scenario</i>. */
    String TAG_TABLE_SCENARIO = "table:scenario";

    /** Element tag for <i>table:table-cell</i>. */
    String TAG_TABLE_CELL = "table:table-cell";

    /**
     * Attribute tag for <i>table:value-type</i> of element
     * <i>table:table-cell</i>.
     */
    String ATTRIBUTE_TABLE_VALUE_TYPE = "table:value-type";

    /**
     * Attribute tag for <i>table:number-columns-repeated</i> of element
     * <i>table:table-cell</i>.
     */
    String ATTRIBUTE_TABLE_NUM_COLUMNS_REPEATED =
        "table:number-columns-repeated";

    /**
     * Attribute tag for <i>table:number-rows-repeated</i> of element
     * <i>table:table-row</i>.
     */
    String ATTRIBUTE_TABLE_NUM_ROWS_REPEATED = "table:number-rows-repeated";

    /**
     * Attribute tag for <i>table:formula</i> of element
     * <i>table:table-cell</i>.
     */
    String ATTRIBUTE_TABLE_FORMULA = "table:formula";

    /**
     * Attribute tag for <i>table:value</i> of element <i>table:table-cell</i>.
     */
    String ATTRIBUTE_TABLE_VALUE = "table:value";

    /**
     * Attribute tag for <i>table:date-value</i> of element
     * <i>table:table-cell</i>.
     */
    String ATTRIBUTE_TABLE_DATE_VALUE = "table:date-value";

    /**
     * Attribute tag for <i>table:time-value</i> of element
     * <i>table:table-cell</i>.
     */
    String ATTRIBUTE_TABLE_TIME_VALUE = "table:time-value";

    /**
     * Attribute tag for <i>table:string-value</i> of element
     * <i>table:table-cell</i>.
     */
    String ATTRIBUTE_TABLE_STRING_VALUE = "table:string-value";

    /**
     * Attribute tag for <i>table:time-boolean-value</i> of element
     * <i>table:table-cell</i>.
     */
    String ATTRIBUTE_TABLE_BOOLEAN_VALUE = "table:boolean-value";

    /** Attribute tag for <i>table:style-name</i> of table elements. */
    String ATTRIBUTE_TABLE_STYLE_NAME = "table:style-name";

    /**
     * Attribute tag for <i>table:currency</i> of element
     * <i>table:table-cell</i>.
     */
    String ATTRIBUTE_TABLE_CURRENCY = "table:currency";

    /** The cell contains data of type <i>string</i>. */
    String CELLTYPE_STRING = "string";

    /** The cell contains data of type <i>float</i>. */
    String CELLTYPE_FLOAT = "float";

    /** The cell contains data of type <i>time</i>. */
    String CELLTYPE_TIME = "time";

    /** The cell contains data of type <i>date</i>. */
    String CELLTYPE_DATE = "date";

    /** The cell contains data of type <i>currency</i>. */
    String CELLTYPE_CURRENCY = "currency";

    /** The cell contains data of type <i>boolean</i>. */
    String CELLTYPE_BOOLEAN = "boolean";

    /** The cell contains data of type <i>percent</i>. */
    String CELLTYPE_PERCENT = "percentage";

    /** StarWriter XML file extension. */
    String SXW_FILE_EXTENSION = ".sxw";

    /** StarWriter XML <i>office:class</i> value. */
    String SXW_TYPE = "text";

    /** StarCalc XML file extension. */
    String SXC_FILE_EXTENSION = ".sxc";

    /** StarCalc XML <i>office:class</i> value. */
    String SXC_TYPE = "spreadsheet";

    /** Element tag for <i>manifest:manifest</i>entry in Manifest XML */
    String TAG_MANIFEST_ROOT = "manifest:manifest";

    /** Element tag for <i>manifest:file-entry</i> entry in Manifest XML. */
    String TAG_MANIFEST_FILE = "manifest:file-entry";

    /**
     * Attribute tag for <i>manifest:media-type</i> of element
     * <i>manifest:file-entry</i>.
     */
    String ATTRIBUTE_MANIFEST_FILE_TYPE = "manifest:media-type";

    /**
     * Attribute tag for <i>manifest:full-path</i> of element
     * <i>manifest:file-entry</i>.
     */
    String ATTRIBUTE_MANIFEST_FILE_PATH = "manifest:full-path";

    // Tags and Elements for the settings.xml

    /** Element tag for <i>config:config-item</i>. */
    String TAG_CONFIG_ITEM = "config:config-item";

    /** Element tag for <i>config:config-item-set</i>. */
    String TAG_CONFIG_ITEM_SET = "config:config-item-set";

    /** Element tag for <i>config:config-item-map-indexed</i>. */
    String TAG_CONFIG_ITEM_MAP_INDEXED = "config:config-item-map-indexed";

    /** Element tag for <i>config:config-item-map-named</i>. */
    String TAG_CONFIG_ITEM_MAP_NAMED = "config:config-item-map-named";

    /** Element tag for <i>config:config-item-map-entry</i>. */
    String TAG_CONFIG_ITEM_MAP_ENTRY= "config:config-item-map-entry";

    /**
     * Attribute tag for <i>config:name</i> of element <i>config:config-item</i>.
     */
    String ATTRIBUTE_CONFIG_NAME = "config:name";

    /**
     * Attribute tag for <i>config:type</i> of element <i>config:config-item</i>.
     */
    String ATTRIBUTE_CONFIG_TYPE = "config:type";

    /** StarWriter XML MIME type. */
    String SXW_MIME_TYPE = "application/vnd.sun.xml.writer";

    /** StarCalc XML MIME type. */
    String SXC_MIME_TYPE = "application/vnd.sun.xml.calc";

}