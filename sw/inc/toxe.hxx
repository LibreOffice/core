/*************************************************************************
 *
 *  $RCSfile: toxe.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:28 $
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
#ifndef _TOXE_HXX
#define _TOXE_HXX

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
//  AUTH_FIELD_KEY, // BiBTeX - Field
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
