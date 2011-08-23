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
#ifndef _TOXE_HXX
#define _TOXE_HXX

namespace binfilter {//STRIP009
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
    TOX_AUTHORITIES
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
//	AUTH_FIELD_KEY, // BiBTeX - Field
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
} //namespace binfilter

#endif
