/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: htmlfld.hxx,v $
 * $Revision: 1.4 $
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

#ifndef _HTMLFLD_HXX
#define _HTMLFLD_HXX

#include <svtools/htmlkywd.hxx>

extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FT_author, "AUTHOR" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FT_sender, "SENDER" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FT_date, "DATE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FT_time, "TIME" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FT_datetime, "DATETIME" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FT_page, "PAGE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FT_docinfo, "DOCINFO" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FT_docstat, "DOCSTAT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FT_filename, "FILENAME" );

extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FS_company, "COMPANY" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FS_firstname, "FIRSTNAME" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FS_name, "NAME" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FS_shortcut, "SHORTCUT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FS_street, "STREET" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FS_country, "COUNTRY" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FS_zip, "ZIP" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FS_city, "CITY" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FS_title, "TITLE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FS_position, "POSITION" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FS_pphone, "PPHONE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FS_cphone, "CPHONE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FS_fax, "FAX" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FS_email, "EMAIL" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FS_state, "STATE" );

extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FS_random, "RANDOM" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FS_next, "NEXT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FS_prev, "PREV" );

//sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FS_title, "TITLE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FS_theme, "THEME" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FS_keys, "KEYS" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FS_comment, "COMMENT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FS_info1, "INFO1" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FS_info2, "INFO2" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FS_info3, "INFO3" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FS_info4, "INFO4" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FS_custom, "CUSTOM" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FS_create, "CREATE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FS_change, "CHANGE" );

extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FS_page, "PAGE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FS_para, "PARAGRAPH" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FS_word, "WORD" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FS_char, "CHAR" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FS_tbl, "TABLE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FS_grf, "GRAPHIC" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FS_ole, "OLE" );

extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FF_name, "NAME" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FF_shortcut, "SHORTCUT" );

extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FF_ssys, "SSYS" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FF_lsys, "LSYS" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FF_dmy, "DMY" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FF_dmyy, "DMYY" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FF_dmmy, "DMMY" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FF_dmmyy, "DMMYY" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FF_dmmmy, "DMMMY" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FF_dmmmyy, "DMMMYY" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FF_ddmmy, "DDMMY" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FF_ddmmmy, "DDMMMY" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FF_ddmmmyy, "DDMMMYY" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FF_dddmmmy, "DDDMMMY" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FF_dddmmmyy, "DDDMMMYY" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FF_my, "MY" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FF_md, "MD" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FF_ymd, "YMD" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FF_yymd, "YYMD" );

extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FF_sys, "SYS" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FF_ssmm24, "SSMM24" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FF_ssmm12, "SSMM12" );

extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FF_uletter, "ULETTER" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FF_lletter, "LLETTER" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FF_uroman, "UROMAN" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FF_lroman, "LROMAN" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FF_arabic, "ARABIC" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FF_none, "NONE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FF_char, "CHAR" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FF_page, "PAGE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FF_ulettern, "ULETTERN" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FF_llettern, "LLETTERN" );

extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FF_author, "AUTHOR" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FF_time, "TIME" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FF_date, "DATE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FF_all, "ALL" );

extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FF_pathname, "PATHNAME" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FF_path, "PATH" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FF_name_noext, "NAME-NOEXT" );


#endif


