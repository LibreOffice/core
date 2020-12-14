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
        XML_TOK_SUB_FRAME               ,
        XML_TOK_SUB_BODY                ,
    };

    enum XMLSubDocument
    {
        XML_TOK_MASTER_DETAIL_FIELD
        ,XML_TOK_MASTER
        ,XML_TOK_SUB_DETAIL
    };


} // namespace rptxml

#endif // INCLUDED_REPORTDESIGN_SOURCE_FILTER_XML_XMLENUMS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
