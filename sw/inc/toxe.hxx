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
#ifndef INCLUDED_SW_INC_TOXE_HXX
#define INCLUDED_SW_INC_TOXE_HXX

enum SwTOXSearch
{
    TOX_NXT,
    TOX_PRV,
    TOX_SAME_NXT,
    TOX_SAME_PRV
};

enum SwTOIKeyType
{
    TOI_PRIMARY,
    TOI_SECONDARY
};

enum TOXTypes
{
    TOX_INDEX,
    TOX_USER,
    TOX_CONTENT,
    TOX_ILLUSTRATIONS,
    TOX_OBJECTS,
    TOX_TABLES,
    TOX_AUTHORITIES,
    TOX_BIBLIOGRAPHY,
    TOX_CITATION
};
// this enum contains all types of sources
enum ToxAuthorityType
{
    //BiBTeX - fields
    AUTH_TYPE_ARTICLE,
    AUTH_TYPE_BOOK,
    AUTH_TYPE_BOOKLET,
    AUTH_TYPE_CONFERENCE,
    AUTH_TYPE_INBOOK,
    AUTH_TYPE_INCOLLECTION,
    AUTH_TYPE_INPROCEEDINGS,
    AUTH_TYPE_JOURNAL,
    AUTH_TYPE_MANUAL,
    AUTH_TYPE_MASTERSTHESIS,
    AUTH_TYPE_MISC,
    AUTH_TYPE_PHDTHESIS,
    AUTH_TYPE_PROCEEDINGS,
    AUTH_TYPE_TECHREPORT,
    AUTH_TYPE_UNPUBLISHED,
    //additional types
    AUTH_TYPE_EMAIL,
    AUTH_TYPE_WWW,
    AUTH_TYPE_CUSTOM1,
    AUTH_TYPE_CUSTOM2,
    AUTH_TYPE_CUSTOM3,
    AUTH_TYPE_CUSTOM4,
    AUTH_TYPE_CUSTOM5,
    AUTH_TYPE_END
};

//this enum contains the fields of an authority entry
enum ToxAuthorityField
{
    AUTH_FIELD_IDENTIFIER,
    AUTH_FIELD_AUTHORITY_TYPE, //ToxAuthorityType
    //BibTex types
    AUTH_FIELD_ADDRESS,
    AUTH_FIELD_ANNOTE,
    AUTH_FIELD_AUTHOR,
    AUTH_FIELD_BOOKTITLE,
    AUTH_FIELD_CHAPTER,
    AUTH_FIELD_EDITION,
    AUTH_FIELD_EDITOR,
    AUTH_FIELD_HOWPUBLISHED,
    AUTH_FIELD_INSTITUTION,
    AUTH_FIELD_JOURNAL,
    AUTH_FIELD_MONTH,
    AUTH_FIELD_NOTE,
    AUTH_FIELD_NUMBER,
    AUTH_FIELD_ORGANIZATIONS,
    AUTH_FIELD_PAGES,
    AUTH_FIELD_PUBLISHER,
    AUTH_FIELD_SCHOOL,
    AUTH_FIELD_SERIES,
    AUTH_FIELD_TITLE,
    AUTH_FIELD_REPORT_TYPE,
    AUTH_FIELD_VOLUME,
    AUTH_FIELD_YEAR,
    //additional types
    AUTH_FIELD_URL,
    AUTH_FIELD_CUSTOM1,
    AUTH_FIELD_CUSTOM2,
    AUTH_FIELD_CUSTOM3,
    AUTH_FIELD_CUSTOM4,
    AUTH_FIELD_CUSTOM5,
    AUTH_FIELD_ISBN,
    AUTH_FIELD_END
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
