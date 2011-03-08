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
#include <prewin.h>
#include <shlobj.h>
#include <postwin.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif

#endif
#include <sot/exchange.hxx>

#define SOT_FORMAT_SYSTEM_START                 ((ULONG)0)
#define SOT_FORMAT_STRING                       ((ULONG)FORMAT_STRING)
#define SOT_FORMAT_BITMAP                       ((ULONG)FORMAT_BITMAP)
#define SOT_FORMAT_GDIMETAFILE                  ((ULONG)FORMAT_GDIMETAFILE)
#define SOT_FORMAT_PRIVATE                      ((ULONG)FORMAT_PRIVATE)
#define SOT_FORMAT_FILE                         ((ULONG)FORMAT_FILE)
#define SOT_FORMAT_FILE_LIST                    ((ULONG)FORMAT_FILE_LIST)
#define SOT_FORMAT_RTF                          ((ULONG)FORMAT_RTF)

#define SOT_FORMATSTR_ID_DRAWING                ((ULONG)11)
#define SOT_FORMATSTR_ID_SVXB                   ((ULONG)12)
#define SOT_FORMATSTR_ID_SVIM                   ((ULONG)13)
#define SOT_FORMATSTR_ID_XFA                    ((ULONG)14)
#define SOT_FORMATSTR_ID_EDITENGINE             ((ULONG)15)
#define SOT_FORMATSTR_ID_INTERNALLINK_STATE     ((ULONG)16)
#define SOT_FORMATSTR_ID_SOLK                   ((ULONG)17)
#define SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK      ((ULONG)18)
#define SOT_FORMATSTR_ID_TREELISTBOX            ((ULONG)19)
#define SOT_FORMATSTR_ID_NATIVE                 ((ULONG)20)
#define SOT_FORMATSTR_ID_OWNERLINK              ((ULONG)21)
#define SOT_FORMATSTR_ID_STARSERVER             ((ULONG)22)
#define SOT_FORMATSTR_ID_STAROBJECT             ((ULONG)23)
#define SOT_FORMATSTR_ID_APPLETOBJECT           ((ULONG)24)
#define SOT_FORMATSTR_ID_PLUGIN_OBJECT          ((ULONG)25)
#define SOT_FORMATSTR_ID_STARWRITER_30          ((ULONG)26)
#define SOT_FORMATSTR_ID_STARWRITER_40          ((ULONG)27)
#define SOT_FORMATSTR_ID_STARWRITER_50          ((ULONG)28)
#define SOT_FORMATSTR_ID_STARWRITERWEB_40       ((ULONG)29)
#define SOT_FORMATSTR_ID_STARWRITERWEB_50       ((ULONG)30)
#define SOT_FORMATSTR_ID_STARWRITERGLOB_40      ((ULONG)31)
#define SOT_FORMATSTR_ID_STARWRITERGLOB_50      ((ULONG)32)
#define SOT_FORMATSTR_ID_STARDRAW               ((ULONG)33)
#define SOT_FORMATSTR_ID_STARDRAW_40            ((ULONG)34)
#define SOT_FORMATSTR_ID_STARIMPRESS_50         ((ULONG)35)
#define SOT_FORMATSTR_ID_STARDRAW_50            ((ULONG)36)
#define SOT_FORMATSTR_ID_STARCALC               ((ULONG)37)
#define SOT_FORMATSTR_ID_STARCALC_40            ((ULONG)38)
#define SOT_FORMATSTR_ID_STARCALC_50            ((ULONG)39)
#define SOT_FORMATSTR_ID_STARCHART              ((ULONG)40)
#define SOT_FORMATSTR_ID_STARCHART_40           ((ULONG)41)
#define SOT_FORMATSTR_ID_STARCHART_50           ((ULONG)42)
#define SOT_FORMATSTR_ID_STARIMAGE              ((ULONG)43)
#define SOT_FORMATSTR_ID_STARIMAGE_40           ((ULONG)44)
#define SOT_FORMATSTR_ID_STARIMAGE_50           ((ULONG)45)
#define SOT_FORMATSTR_ID_STARMATH               ((ULONG)46)
#define SOT_FORMATSTR_ID_STARMATH_40            ((ULONG)47)
#define SOT_FORMATSTR_ID_STARMATH_50            ((ULONG)48)
#define SOT_FORMATSTR_ID_STAROBJECT_PAINTDOC    ((ULONG)49)
#define SOT_FORMATSTR_ID_FILLED_AREA            ((ULONG)50)
#define SOT_FORMATSTR_ID_HTML                   ((ULONG)51)
#define SOT_FORMATSTR_ID_HTML_SIMPLE            ((ULONG)52)
#define SOT_FORMATSTR_ID_CHAOS                  ((ULONG)53)
#define SOT_FORMATSTR_ID_CNT_MSGATTACHFILE      ((ULONG)54)
#define SOT_FORMATSTR_ID_BIFF_5                 ((ULONG)55)
#define SOT_FORMATSTR_ID_BIFF__5                ((ULONG)56)
#define SOT_FORMATSTR_ID_SYLK                   ((ULONG)57)
#define SOT_FORMATSTR_ID_SYLK_BIGCAPS           ((ULONG)58)
#define SOT_FORMATSTR_ID_LINK                   ((ULONG)59)
#define SOT_FORMATSTR_ID_DIF                    ((ULONG)60)
#define SOT_FORMATSTR_ID_STARDRAW_TABBAR        ((ULONG)61)
#define SOT_FORMATSTR_ID_SONLK                  ((ULONG)62)
#define SOT_FORMATSTR_ID_MSWORD_DOC             ((ULONG)63)
#define SOT_FORMATSTR_ID_STAR_FRAMESET_DOC      ((ULONG)64)
#define SOT_FORMATSTR_ID_OFFICE_DOC             ((ULONG)65)
#define SOT_FORMATSTR_ID_NOTES_DOCINFO          ((ULONG)66)
#define SOT_FORMATSTR_ID_NOTES_HNOTE            ((ULONG)67)
#define SOT_FORMATSTR_ID_NOTES_NATIVE           ((ULONG)68)
#define SOT_FORMATSTR_ID_SFX_DOC                ((ULONG)69)
#define SOT_FORMATSTR_ID_EVDF                   ((ULONG)70)
#define SOT_FORMATSTR_ID_ESDF                   ((ULONG)71)
#define SOT_FORMATSTR_ID_IDF                    ((ULONG)72)
#define SOT_FORMATSTR_ID_EFTP                   ((ULONG)73)
#define SOT_FORMATSTR_ID_EFD                    ((ULONG)74)
#define SOT_FORMATSTR_ID_SVX_FORMFIELDEXCH      ((ULONG)75)
#define SOT_FORMATSTR_ID_EXTENDED_TABBAR        ((ULONG)76)
#define SOT_FORMATSTR_ID_SBA_DATAEXCHANGE       ((ULONG)77)
#define SOT_FORMATSTR_ID_SBA_FIELDDATAEXCHANGE  ((ULONG)78)
#define SOT_FORMATSTR_ID_SBA_PRIVATE_URL        ((ULONG)79)
#define SOT_FORMATSTR_ID_SBA_TABED              ((ULONG)80)
#define SOT_FORMATSTR_ID_SBA_TABID              ((ULONG)81)
#define SOT_FORMATSTR_ID_SBA_JOIN               ((ULONG)82)
#define SOT_FORMATSTR_ID_OBJECTDESCRIPTOR       ((ULONG)83)
#define SOT_FORMATSTR_ID_LINKSRCDESCRIPTOR      ((ULONG)84)
#define SOT_FORMATSTR_ID_EMBED_SOURCE           ((ULONG)85)
#define SOT_FORMATSTR_ID_LINK_SOURCE            ((ULONG)86)
#define SOT_FORMATSTR_ID_EMBEDDED_OBJ           ((ULONG)87)
#define SOT_FORMATSTR_ID_FILECONTENT            ((ULONG)88)
#define SOT_FORMATSTR_ID_FILEGRPDESCRIPTOR      ((ULONG)89)
#define SOT_FORMATSTR_ID_FILENAME               ((ULONG)90)
#define SOT_FORMATSTR_ID_SD_OLE                 ((ULONG)91)
#define SOT_FORMATSTR_ID_EMBEDDED_OBJ_OLE       ((ULONG)92)
#define SOT_FORMATSTR_ID_EMBED_SOURCE_OLE       ((ULONG)93)
#define SOT_FORMATSTR_ID_OBJECTDESCRIPTOR_OLE   ((ULONG)94)
#define SOT_FORMATSTR_ID_LINKSRCDESCRIPTOR_OLE  ((ULONG)95)
#define SOT_FORMATSTR_ID_LINK_SOURCE_OLE        ((ULONG)96)
#define SOT_FORMATSTR_ID_SBA_CTRLDATAEXCHANGE   ((ULONG)97)
#define SOT_FORMATSTR_ID_OUTPLACE_OBJ           ((ULONG)98)
#define SOT_FORMATSTR_ID_CNT_OWN_CLIP           ((ULONG)99)
#define SOT_FORMATSTR_ID_INET_IMAGE             ((ULONG)100)
#define SOT_FORMATSTR_ID_NETSCAPE_IMAGE         ((ULONG)101)
#define SOT_FORMATSTR_ID_SBA_FORMEXCHANGE       ((ULONG)102)
#define SOT_FORMATSTR_ID_SBA_REPORTEXCHANGE     ((ULONG)103)
#define SOT_FORMATSTR_ID_UNIFORMRESOURCELOCATOR ((ULONG)104)
#define SOT_FORMATSTR_ID_STARCHARTDOCUMENT_50   ((ULONG)105)
#define SOT_FORMATSTR_ID_GRAPHOBJ               ((ULONG)106)
#define SOT_FORMATSTR_ID_STARWRITER_60          ((ULONG)107)
#define SOT_FORMATSTR_ID_STARWRITERWEB_60       ((ULONG)108)
#define SOT_FORMATSTR_ID_STARWRITERGLOB_60      ((ULONG)109)
#define SOT_FORMATSTR_ID_STARDRAW_60            ((ULONG)110)
#define SOT_FORMATSTR_ID_STARIMPRESS_60         ((ULONG)111)
#define SOT_FORMATSTR_ID_STARCALC_60            ((ULONG)112)
#define SOT_FORMATSTR_ID_STARCHART_60           ((ULONG)113)
#define SOT_FORMATSTR_ID_STARMATH_60            ((ULONG)114)
#define SOT_FORMATSTR_ID_WMF                    ((ULONG)115)
#define SOT_FORMATSTR_ID_DBACCESS_QUERY         ((ULONG)116)
#define SOT_FORMATSTR_ID_DBACCESS_TABLE         ((ULONG)117)
#define SOT_FORMATSTR_ID_DBACCESS_COMMAND       ((ULONG)118)
#define SOT_FORMATSTR_ID_DIALOG_60              ((ULONG)119)
#define SOT_FORMATSTR_ID_EMF                    ((ULONG)120)
#define SOT_FORMATSTR_ID_BIFF_8                 ((ULONG)121)
#define SOT_FORMATSTR_ID_BMP                    ((ULONG)122)
#define SOT_FORMATSTR_ID_HTML_NO_COMMENT        ((ULONG)123)
#define SOT_FORMATSTR_ID_STARWRITER_8           ((ULONG)124)
#define SOT_FORMATSTR_ID_STARWRITERWEB_8        ((ULONG)125)
#define SOT_FORMATSTR_ID_STARWRITERGLOB_8       ((ULONG)126)
#define SOT_FORMATSTR_ID_STARDRAW_8             ((ULONG)127)
#define SOT_FORMATSTR_ID_STARIMPRESS_8          ((ULONG)128)
#define SOT_FORMATSTR_ID_STARCALC_8             ((ULONG)129)
#define SOT_FORMATSTR_ID_STARCHART_8            ((ULONG)130)
#define SOT_FORMATSTR_ID_STARMATH_8             ((ULONG)131)
#define SOT_FORMATSTR_ID_XFORMS                 ((ULONG)132)
#define SOT_FORMATSTR_ID_STARWRITER_8_TEMPLATE  ((ULONG)133)
#define SOT_FORMATSTR_ID_STARDRAW_8_TEMPLATE    ((ULONG)134)
#define SOT_FORMATSTR_ID_STARIMPRESS_8_TEMPLATE ((ULONG)135)
#define SOT_FORMATSTR_ID_STARCALC_8_TEMPLATE    ((ULONG)136)
#define SOT_FORMATSTR_ID_STARCHART_8_TEMPLATE   ((ULONG)137)
#define SOT_FORMATSTR_ID_STARMATH_8_TEMPLATE    ((ULONG)138)
#define SOT_FORMATSTR_ID_STARBASE_8             ((ULONG)139)
#define SOT_FORMATSTR_ID_HC_GDIMETAFILE         ((ULONG)140)
#define SOT_FORMATSTR_ID_USER_END               SOT_FORMATSTR_ID_HC_GDIMETAFILE

#endif // _SOT_FORMATS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
