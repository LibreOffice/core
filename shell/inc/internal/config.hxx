/*************************************************************************
 *
 *  $RCSfile: config.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-04-07 10:51:52 $
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

#ifndef CONFIG_HXX_INCLUDED
#define CONFIG_HXX_INCLUDED

#pragma warning (disable : 4786 4503)

#include <tchar.h>

#define MODULE_NAME TEXT("shlxthdl.dll")

#define COLUMN_HANDLER_DESCRIPTIVE_NAME    TEXT("OpenOffice Column Handler")
#define INFOTIP_HANDLER_DESCRIPTIVE_NAME   TEXT("OpenOffice Infotip Handler")
#define PROPSHEET_HANDLER_DESCRIPTIVE_NAME TEXT("OpenOffice Property Sheet Handler")


#define META_CONTENT_NAME               "meta.xml"

#define EMPTY_STRING                    L""
#define SPACE                           L" "
#define META_INFO_TITLE                 L"title"
#define META_INFO_AUTHOR                L"initial-creator"
#define META_INFO_SUBJECT               L"subject"
#define META_INFO_KEYWORDS              L"keywords"
#define META_INFO_KEYWORD               L"keyword"
#define META_INFO_DESCRIPTION           L"description"

#define META_INFO_PAGES                 L"page-count"
#define META_INFO_TABLES                L"table-count"
#define META_INFO_DRAWS                 L"image-count"
#define META_INFO_OBJECTS               L"object-count"
#define META_INFO_OLE_OBJECTS           L"object-count"
#define META_INFO_PARAGRAPHS            L"paragraph-count"
#define META_INFO_WORDS                 L"word-count"
#define META_INFO_CHARACTERS            L"character-count"
#define META_INFO_ROWS                  L"row-count"
#define META_INFO_CELLS                 L"cell-count"
#define META_INFO_DOCUMENT_STATISTIC    L"document-statistic"
#define META_INFO_MODIFIED              L"date"
#define META_INFO_DOCUMENT_NUMBER       L"editing-cycles"
#define META_INFO_EDITING_TIME          L"editing-duration"

#define META_INFO_LANGUAGE              L"language"
#define META_INFO_CREATOR               L"creator"
#define META_INFO_CREATION              L"creation-date"
#define META_INFO_GENERATOR             L"generator"




#endif
