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

#ifndef _SOT_FORMATS_HXX
#define _SOT_FORMATS_HXX

#ifdef _SOT_FORMATS_INCLUDE_SYSTEMFORMATS

#ifdef WNT
#ifdef _MSC_VER
#pragma warning(push, 1)
#pragma warning(disable: 4917)
#endif
#include <shlobj.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif

#endif
#include <sot/exchange.hxx>

#define SOT_FORMAT_SYSTEM_START                 ((sal_uLong)0)
#define SOT_FORMAT_STRING                       ((sal_uLong)FORMAT_STRING)
#define SOT_FORMAT_BITMAP                       ((sal_uLong)FORMAT_BITMAP)
#define SOT_FORMAT_GDIMETAFILE                  ((sal_uLong)FORMAT_GDIMETAFILE)
#define SOT_FORMAT_PRIVATE                      ((sal_uLong)FORMAT_PRIVATE)
#define SOT_FORMAT_FILE                         ((sal_uLong)FORMAT_FILE)
#define SOT_FORMAT_FILE_LIST                    ((sal_uLong)FORMAT_FILE_LIST)
#define SOT_FORMAT_RTF                          ((sal_uLong)FORMAT_RTF)

#define SOT_FORMATSTR_ID_DRAWING                ((sal_uLong)11)
#define SOT_FORMATSTR_ID_SVXB                   ((sal_uLong)12)
#define SOT_FORMATSTR_ID_SVIM                   ((sal_uLong)13)
#define SOT_FORMATSTR_ID_XFA                    ((sal_uLong)14)
#define SOT_FORMATSTR_ID_EDITENGINE             ((sal_uLong)15)
#define SOT_FORMATSTR_ID_INTERNALLINK_STATE     ((sal_uLong)16)
#define SOT_FORMATSTR_ID_SOLK                   ((sal_uLong)17)
#define SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK      ((sal_uLong)18)
#define SOT_FORMATSTR_ID_TREELISTBOX            ((sal_uLong)19)
#define SOT_FORMATSTR_ID_NATIVE                 ((sal_uLong)20)
#define SOT_FORMATSTR_ID_OWNERLINK              ((sal_uLong)21)
#define SOT_FORMATSTR_ID_STARSERVER             ((sal_uLong)22)
#define SOT_FORMATSTR_ID_STAROBJECT             ((sal_uLong)23)
#define SOT_FORMATSTR_ID_APPLETOBJECT           ((sal_uLong)24)
#define SOT_FORMATSTR_ID_PLUGIN_OBJECT          ((sal_uLong)25)
#define SOT_FORMATSTR_ID_STARWRITER_30          ((sal_uLong)26)
#define SOT_FORMATSTR_ID_STARWRITER_40          ((sal_uLong)27)
#define SOT_FORMATSTR_ID_STARWRITER_50          ((sal_uLong)28)
#define SOT_FORMATSTR_ID_STARWRITERWEB_40       ((sal_uLong)29)
#define SOT_FORMATSTR_ID_STARWRITERWEB_50       ((sal_uLong)30)
#define SOT_FORMATSTR_ID_STARWRITERGLOB_40      ((sal_uLong)31)
#define SOT_FORMATSTR_ID_STARWRITERGLOB_50      ((sal_uLong)32)
#define SOT_FORMATSTR_ID_STARDRAW               ((sal_uLong)33)
#define SOT_FORMATSTR_ID_STARDRAW_40            ((sal_uLong)34)
#define SOT_FORMATSTR_ID_STARIMPRESS_50         ((sal_uLong)35)
#define SOT_FORMATSTR_ID_STARDRAW_50            ((sal_uLong)36)
#define SOT_FORMATSTR_ID_STARCALC               ((sal_uLong)37)
#define SOT_FORMATSTR_ID_STARCALC_40            ((sal_uLong)38)
#define SOT_FORMATSTR_ID_STARCALC_50            ((sal_uLong)39)
#define SOT_FORMATSTR_ID_STARCHART              ((sal_uLong)40)
#define SOT_FORMATSTR_ID_STARCHART_40           ((sal_uLong)41)
#define SOT_FORMATSTR_ID_STARCHART_50           ((sal_uLong)42)
#define SOT_FORMATSTR_ID_STARIMAGE              ((sal_uLong)43)
#define SOT_FORMATSTR_ID_STARIMAGE_40           ((sal_uLong)44)
#define SOT_FORMATSTR_ID_STARIMAGE_50           ((sal_uLong)45)
#define SOT_FORMATSTR_ID_STARMATH               ((sal_uLong)46)
#define SOT_FORMATSTR_ID_STARMATH_40            ((sal_uLong)47)
#define SOT_FORMATSTR_ID_STARMATH_50            ((sal_uLong)48)
#define SOT_FORMATSTR_ID_STAROBJECT_PAINTDOC    ((sal_uLong)49)
#define SOT_FORMATSTR_ID_FILLED_AREA            ((sal_uLong)50)
#define SOT_FORMATSTR_ID_HTML                   ((sal_uLong)51)
#define SOT_FORMATSTR_ID_HTML_SIMPLE            ((sal_uLong)52)
#define SOT_FORMATSTR_ID_CHAOS                  ((sal_uLong)53)
#define SOT_FORMATSTR_ID_CNT_MSGATTACHFILE      ((sal_uLong)54)
#define SOT_FORMATSTR_ID_BIFF_5                 ((sal_uLong)55)
#define SOT_FORMATSTR_ID_BIFF__5                ((sal_uLong)56)
#define SOT_FORMATSTR_ID_SYLK                   ((sal_uLong)57)
#define SOT_FORMATSTR_ID_SYLK_BIGCAPS           ((sal_uLong)58)
#define SOT_FORMATSTR_ID_LINK                   ((sal_uLong)59)
#define SOT_FORMATSTR_ID_DIF                    ((sal_uLong)60)
#define SOT_FORMATSTR_ID_STARDRAW_TABBAR        ((sal_uLong)61)
#define SOT_FORMATSTR_ID_SONLK                  ((sal_uLong)62)
#define SOT_FORMATSTR_ID_MSWORD_DOC             ((sal_uLong)63)
#define SOT_FORMATSTR_ID_STAR_FRAMESET_DOC      ((sal_uLong)64)
#define SOT_FORMATSTR_ID_OFFICE_DOC             ((sal_uLong)65)
#define SOT_FORMATSTR_ID_NOTES_DOCINFO          ((sal_uLong)66)
#define SOT_FORMATSTR_ID_NOTES_HNOTE            ((sal_uLong)67)
#define SOT_FORMATSTR_ID_NOTES_NATIVE           ((sal_uLong)68)
#define SOT_FORMATSTR_ID_SFX_DOC                ((sal_uLong)69)
#define SOT_FORMATSTR_ID_EVDF                   ((sal_uLong)70)
#define SOT_FORMATSTR_ID_ESDF                   ((sal_uLong)71)
#define SOT_FORMATSTR_ID_IDF                    ((sal_uLong)72)
#define SOT_FORMATSTR_ID_EFTP                   ((sal_uLong)73)
#define SOT_FORMATSTR_ID_EFD                    ((sal_uLong)74)
#define SOT_FORMATSTR_ID_SVX_FORMFIELDEXCH      ((sal_uLong)75)
#define SOT_FORMATSTR_ID_EXTENDED_TABBAR        ((sal_uLong)76)
#define SOT_FORMATSTR_ID_SBA_DATAEXCHANGE       ((sal_uLong)77)
#define SOT_FORMATSTR_ID_SBA_FIELDDATAEXCHANGE  ((sal_uLong)78)
#define SOT_FORMATSTR_ID_SBA_PRIVATE_URL        ((sal_uLong)79)
#define SOT_FORMATSTR_ID_SBA_TABED              ((sal_uLong)80)
#define SOT_FORMATSTR_ID_SBA_TABID              ((sal_uLong)81)
#define SOT_FORMATSTR_ID_SBA_JOIN               ((sal_uLong)82)
#define SOT_FORMATSTR_ID_OBJECTDESCRIPTOR       ((sal_uLong)83)
#define SOT_FORMATSTR_ID_LINKSRCDESCRIPTOR      ((sal_uLong)84)
#define SOT_FORMATSTR_ID_EMBED_SOURCE           ((sal_uLong)85)
#define SOT_FORMATSTR_ID_LINK_SOURCE            ((sal_uLong)86)
#define SOT_FORMATSTR_ID_EMBEDDED_OBJ           ((sal_uLong)87)
#define SOT_FORMATSTR_ID_FILECONTENT            ((sal_uLong)88)
#define SOT_FORMATSTR_ID_FILEGRPDESCRIPTOR      ((sal_uLong)89)
#define SOT_FORMATSTR_ID_FILENAME               ((sal_uLong)90)
#define SOT_FORMATSTR_ID_SD_OLE                 ((sal_uLong)91)
#define SOT_FORMATSTR_ID_EMBEDDED_OBJ_OLE       ((sal_uLong)92)
#define SOT_FORMATSTR_ID_EMBED_SOURCE_OLE       ((sal_uLong)93)
#define SOT_FORMATSTR_ID_OBJECTDESCRIPTOR_OLE   ((sal_uLong)94)
#define SOT_FORMATSTR_ID_LINKSRCDESCRIPTOR_OLE  ((sal_uLong)95)
#define SOT_FORMATSTR_ID_LINK_SOURCE_OLE        ((sal_uLong)96)
#define SOT_FORMATSTR_ID_SBA_CTRLDATAEXCHANGE   ((sal_uLong)97)
#define SOT_FORMATSTR_ID_OUTPLACE_OBJ           ((sal_uLong)98)
#define SOT_FORMATSTR_ID_CNT_OWN_CLIP           ((sal_uLong)99)
#define SOT_FORMATSTR_ID_INET_IMAGE             ((sal_uLong)100)
#define SOT_FORMATSTR_ID_NETSCAPE_IMAGE         ((sal_uLong)101)
#define SOT_FORMATSTR_ID_SBA_FORMEXCHANGE       ((sal_uLong)102)
#define SOT_FORMATSTR_ID_SBA_REPORTEXCHANGE     ((sal_uLong)103)
#define SOT_FORMATSTR_ID_UNIFORMRESOURCELOCATOR ((sal_uLong)104)
#define SOT_FORMATSTR_ID_STARCHARTDOCUMENT_50   ((sal_uLong)105)
#define SOT_FORMATSTR_ID_GRAPHOBJ               ((sal_uLong)106)
#define SOT_FORMATSTR_ID_STARWRITER_60          ((sal_uLong)107)
#define SOT_FORMATSTR_ID_STARWRITERWEB_60       ((sal_uLong)108)
#define SOT_FORMATSTR_ID_STARWRITERGLOB_60      ((sal_uLong)109)
#define SOT_FORMATSTR_ID_STARDRAW_60            ((sal_uLong)110)
#define SOT_FORMATSTR_ID_STARIMPRESS_60         ((sal_uLong)111)
#define SOT_FORMATSTR_ID_STARCALC_60            ((sal_uLong)112)
#define SOT_FORMATSTR_ID_STARCHART_60           ((sal_uLong)113)
#define SOT_FORMATSTR_ID_STARMATH_60            ((sal_uLong)114)
#define SOT_FORMATSTR_ID_WMF                    ((sal_uLong)115)
#define SOT_FORMATSTR_ID_DBACCESS_QUERY         ((sal_uLong)116)
#define SOT_FORMATSTR_ID_DBACCESS_TABLE         ((sal_uLong)117)
#define SOT_FORMATSTR_ID_DBACCESS_COMMAND       ((sal_uLong)118)
#define SOT_FORMATSTR_ID_DIALOG_60              ((sal_uLong)119)
#define SOT_FORMATSTR_ID_EMF                    ((sal_uLong)120)
#define SOT_FORMATSTR_ID_BIFF_8                 ((sal_uLong)121)
#define SOT_FORMATSTR_ID_BMP                    ((sal_uLong)122)
#define SOT_FORMATSTR_ID_HTML_NO_COMMENT        ((sal_uLong)123)
#define SOT_FORMATSTR_ID_STARWRITER_8           ((sal_uLong)124)
#define SOT_FORMATSTR_ID_STARWRITERWEB_8        ((sal_uLong)125)
#define SOT_FORMATSTR_ID_STARWRITERGLOB_8       ((sal_uLong)126)
#define SOT_FORMATSTR_ID_STARDRAW_8             ((sal_uLong)127)
#define SOT_FORMATSTR_ID_STARIMPRESS_8          ((sal_uLong)128)
#define SOT_FORMATSTR_ID_STARCALC_8             ((sal_uLong)129)
#define SOT_FORMATSTR_ID_STARCHART_8            ((sal_uLong)130)
#define SOT_FORMATSTR_ID_STARMATH_8             ((sal_uLong)131)
#define SOT_FORMATSTR_ID_XFORMS                 ((sal_uLong)132)
#define SOT_FORMATSTR_ID_STARWRITER_8_TEMPLATE  ((sal_uLong)133)
#define SOT_FORMATSTR_ID_STARDRAW_8_TEMPLATE    ((sal_uLong)134)
#define SOT_FORMATSTR_ID_STARIMPRESS_8_TEMPLATE ((sal_uLong)135)
#define SOT_FORMATSTR_ID_STARCALC_8_TEMPLATE    ((sal_uLong)136)
#define SOT_FORMATSTR_ID_STARCHART_8_TEMPLATE   ((sal_uLong)137)
#define SOT_FORMATSTR_ID_STARMATH_8_TEMPLATE    ((sal_uLong)138)
#define SOT_FORMATSTR_ID_STARBASE_8             ((sal_uLong)139)
#define SOT_FORMATSTR_ID_HC_GDIMETAFILE         ((sal_uLong)140)
#define SOT_FORMATSTR_ID_USER_END               SOT_FORMATSTR_ID_HC_GDIMETAFILE

#endif // _SOT_FORMATS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
