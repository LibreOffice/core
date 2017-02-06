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
#ifndef INCLUDED_REPORTDESIGN_SOURCE_FILTER_XML_XMLENUMS_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_FILTER_XML_XMLENUMS_HXX

#define PROGRESS_BAR_STEP 20

namespace rptxml
{
    enum XMLDocTokens
    {
        XML_TOK_DOC_AUTOSTYLES,
        XML_TOK_DOC_SETTINGS,
        XML_TOK_DOC_REPORT,
        XML_TOK_DOC_STYLES,
        XML_TOK_DOC_FONTDECLS,
        XML_TOK_DOC_MASTERSTYLES,
        XML_TOK_DOC_META
    };
    enum XMLReportToken
    {
        XML_TOK_REPORT_HEADER,
        XML_TOK_PAGE_HEADER ,
        XML_TOK_GROUP,
        XML_TOK_DETAIL      ,
        XML_TOK_PAGE_FOOTER ,
        XML_TOK_REPORT_FOOTER,
        XML_TOK_HEADER_ON_NEW_PAGE      ,
        XML_TOK_FOOTER_ON_NEW_PAGE      ,
        XML_TOK_COMMAND_TYPE            ,
        XML_TOK_COMMAND                 ,
        XML_TOK_FILTER                  ,
        XML_TOK_CAPTION                 ,
        XML_TOK_ESCAPE_PROCESSING       ,
        XML_TOK_REPORT_FUNCTION         ,
        XML_TOK_REPORT_ELEMENT          ,
        XML_TOK_REPORT_MIMETYPE         ,
        XML_TOK_REPORT_NAME             ,
        XML_TOK_MASTER_DETAIL_FIELDS    ,
        XML_TOK_SUB_FRAME
    };
    enum XMLGroup
    {
        XML_TOK_START_NEW_COLUMN            ,
        XML_TOK_RESET_PAGE_NUMBER           ,
        XML_TOK_PRINT_HEADER_ON_EACH_PAGE   ,
        XML_TOK_SORT_EXPRESSION             ,
        XML_TOK_GROUP_EXPRESSION            ,
        XML_TOK_GROUP_HEADER                ,
        XML_TOK_GROUP_GROUP                 ,
        XML_TOK_GROUP_DETAIL                ,
        XML_TOK_GROUP_FOOTER                ,
        XML_TOK_SORT_ASCENDING              ,
        XML_TOK_GROUP_FUNCTION              ,
        XML_TOK_GROUP_KEEP_TOGETHER
    };
    enum XMLSection
    {
        XML_TOK_SECTION_NAME,
        XML_TOK_TABLE,
        XML_TOK_VISIBLE             ,
        XML_TOK_FORCE_NEW_PAGE      ,
        XML_TOK_FORCE_NEW_COLUMN    ,
        XML_TOK_KEEP_TOGETHER       ,
        XML_TOK_REPEAT_SECTION,
           XML_TOK_PAGE_PRINT_OPTION,
        XML_TOK_SECT_STYLE_NAME
    };

    enum XMLCell
    {
        XML_TOK_P,
        XML_TOK_FIXED_CONTENT,
        XML_TOK_FORMATTED_TEXT,
        XML_TOK_IMAGE,
        XML_TOK_SUB_DOCUMENT,
        XML_TOK_CUSTOM_SHAPE,
        XML_TOK_PAGE_NUMBER,
        XML_TOK_PAGE_COUNT,
        XML_TOK_TEXT_TAB_STOP,
        XML_TOK_TEXT_LINE_BREAK,
        XML_TOK_TEXT_S,
        XML_TOK_FRAME
    };

    enum XMLComponent
    {
        XML_TOK_NAME
        ,XML_TOK_TABLE_COLUMNS
        ,XML_TOK_TABLE_ROWS
        ,XML_TOK_COLUMN_STYLE_NAME
        ,XML_TOK_COLUMN
        ,XML_TOK_ROW
        ,XML_TOK_CELL
        ,XML_TOK_COV_CELL
        ,XML_TOK_NUMBER_COLUMNS_SPANNED
        ,XML_TOK_NUMBER_ROWS_SPANNED
        ,XML_TOK_TEXT_STYLE_NAME
        ,XML_TOK_TRANSFORM
        ,XML_TOK_CONDITIONAL_PRINT_EXPRESSION
    };
    enum XMLReportElement
    {
         XML_TOK_PRINT_WHEN_GROUP_CHANGE
        ,XML_TOK_REP_CONDITIONAL_PRINT_EXPRESSION
        ,XML_TOK_PRINT_REPEATED_VALUES
        ,XML_TOK_COMPONENT
        ,XML_TOK_FORMATCONDITION
    };
    enum XMLControlProperty
    {
        XML_TOK_PROPERTY_NAME
        ,XML_TOK_VALUE_TYPE
        ,XML_TOK_LIST_PROPERTY
        ,XML_TOK_VALUE
        ,XML_TOK_CURRENCY
        ,XML_TOK_DATE_VALUE
        ,XML_TOK_TIME_VALUE
        ,XML_TOK_STRING_VALUE
        ,XML_TOK_BOOLEAN_VALUE
        ,XML_TOK_PROPERTIES
        ,XML_TOK_SIZE
        ,XML_TOK_IMAGE_DATA
        ,XML_TOK_SCALE
        ,XML_TOK_LABEL
        ,XML_TOK_DATA_FORMULA
        ,XML_TOK_PRESERVE_IRI
        ,XML_TOK_SELECT_PAGE
    };
    enum XMLFunction
    {
         XML_TOK_FUNCTION_NAME
        ,XML_TOK_FUNCTION_FORMULA
        ,XML_TOK_PRE_EVALUATED
        ,XML_TOK_INITIAL_FORMULA
        ,XML_TOK_DEEP_TRAVERSING
    };
    enum XMLSubDocument
    {
        XML_TOK_MASTER_DETAIL_FIELD
        ,XML_TOK_MASTER
        ,XML_TOK_SUB_DETAIL
    };
    enum XMLFormatConidition
    {
         XML_TOK_FORMAT_STYLE_NAME
        ,XML_TOK_ENABLED
        ,XML_TOK_FORMULA
    };


} // namespace rptxml

#endif // INCLUDED_REPORTDESIGN_SOURCE_FILTER_XML_XMLENUMS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
