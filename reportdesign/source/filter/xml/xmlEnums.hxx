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
#ifndef RPT_XMLENUMS_HXX
#define RPT_XMLENUMS_HXX

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
        XML_TOK_PRINT_ONLY_WHEN_GROUP_CHANGE
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
        ,XML_TOK_IMAGE_POSITION
        ,XML_TOK_IMAGE_ALIGN
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

// -----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------
#endif // RPT_XMLENUMS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
