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
#include <tools/prewin.h>
#include <shlobj.h>
#include <tools/postwin.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif

#endif
#include <sot/exchange.hxx>

#define SOT_FORMAT_SYSTEM_START                 ((sal_uIntPtr)0)
#define SOT_FORMAT_STRING                       ((sal_uIntPtr)FORMAT_STRING)
#define SOT_FORMAT_BITMAP                       ((sal_uIntPtr)FORMAT_BITMAP)
#define SOT_FORMAT_GDIMETAFILE                  ((sal_uIntPtr)FORMAT_GDIMETAFILE)
#define SOT_FORMAT_PRIVATE                      ((sal_uIntPtr)FORMAT_PRIVATE)
#define SOT_FORMAT_FILE                         ((sal_uIntPtr)FORMAT_FILE)
#define SOT_FORMAT_FILE_LIST                    ((sal_uIntPtr)FORMAT_FILE_LIST)
#define SOT_FORMAT_RTF                          ((sal_uIntPtr)FORMAT_RTF)

#define SOT_FORMATSTR_ID_DRAWING                ((sal_uIntPtr)11)
#define SOT_FORMATSTR_ID_SVXB                   ((sal_uIntPtr)12)
#define SOT_FORMATSTR_ID_SVIM                   ((sal_uIntPtr)13)
#define SOT_FORMATSTR_ID_XFA                    ((sal_uIntPtr)14)
#define SOT_FORMATSTR_ID_EDITENGINE             ((sal_uIntPtr)15)
#define SOT_FORMATSTR_ID_INTERNALLINK_STATE     ((sal_uIntPtr)16)
#define SOT_FORMATSTR_ID_SOLK                   ((sal_uIntPtr)17)
#define SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK      ((sal_uIntPtr)18)
#define SOT_FORMATSTR_ID_TREELISTBOX            ((sal_uIntPtr)19)
#define SOT_FORMATSTR_ID_NATIVE                 ((sal_uIntPtr)20)
#define SOT_FORMATSTR_ID_OWNERLINK              ((sal_uIntPtr)21)
#define SOT_FORMATSTR_ID_STARSERVER             ((sal_uIntPtr)22)
#define SOT_FORMATSTR_ID_STAROBJECT             ((sal_uIntPtr)23)
#define SOT_FORMATSTR_ID_APPLETOBJECT           ((sal_uIntPtr)24)
#define SOT_FORMATSTR_ID_PLUGIN_OBJECT          ((sal_uIntPtr)25)
#define SOT_FORMATSTR_ID_STARWRITER_30          ((sal_uIntPtr)26)
#define SOT_FORMATSTR_ID_STARWRITER_40          ((sal_uIntPtr)27)
#define SOT_FORMATSTR_ID_STARWRITER_50          ((sal_uIntPtr)28)
#define SOT_FORMATSTR_ID_STARWRITERWEB_40       ((sal_uIntPtr)29)
#define SOT_FORMATSTR_ID_STARWRITERWEB_50       ((sal_uIntPtr)30)
#define SOT_FORMATSTR_ID_STARWRITERGLOB_40      ((sal_uIntPtr)31)
#define SOT_FORMATSTR_ID_STARWRITERGLOB_50      ((sal_uIntPtr)32)
#define SOT_FORMATSTR_ID_STARDRAW               ((sal_uIntPtr)33)
#define SOT_FORMATSTR_ID_STARDRAW_40            ((sal_uIntPtr)34)
#define SOT_FORMATSTR_ID_STARIMPRESS_50         ((sal_uIntPtr)35)
#define SOT_FORMATSTR_ID_STARDRAW_50            ((sal_uIntPtr)36)
#define SOT_FORMATSTR_ID_STARCALC               ((sal_uIntPtr)37)
#define SOT_FORMATSTR_ID_STARCALC_40            ((sal_uIntPtr)38)
#define SOT_FORMATSTR_ID_STARCALC_50            ((sal_uIntPtr)39)
#define SOT_FORMATSTR_ID_STARCHART              ((sal_uIntPtr)40)
#define SOT_FORMATSTR_ID_STARCHART_40           ((sal_uIntPtr)41)
#define SOT_FORMATSTR_ID_STARCHART_50           ((sal_uIntPtr)42)
#define SOT_FORMATSTR_ID_STARIMAGE              ((sal_uIntPtr)43)
#define SOT_FORMATSTR_ID_STARIMAGE_40           ((sal_uIntPtr)44)
#define SOT_FORMATSTR_ID_STARIMAGE_50           ((sal_uIntPtr)45)
#define SOT_FORMATSTR_ID_STARMATH               ((sal_uIntPtr)46)
#define SOT_FORMATSTR_ID_STARMATH_40            ((sal_uIntPtr)47)
#define SOT_FORMATSTR_ID_STARMATH_50            ((sal_uIntPtr)48)
#define SOT_FORMATSTR_ID_STAROBJECT_PAINTDOC    ((sal_uIntPtr)49)
#define SOT_FORMATSTR_ID_FILLED_AREA            ((sal_uIntPtr)50)
#define SOT_FORMATSTR_ID_HTML                   ((sal_uIntPtr)51)
#define SOT_FORMATSTR_ID_HTML_SIMPLE            ((sal_uIntPtr)52)
#define SOT_FORMATSTR_ID_CHAOS                  ((sal_uIntPtr)53)
#define SOT_FORMATSTR_ID_CNT_MSGATTACHFILE      ((sal_uIntPtr)54)
#define SOT_FORMATSTR_ID_BIFF_5                 ((sal_uIntPtr)55)
#define SOT_FORMATSTR_ID_BIFF__5                ((sal_uIntPtr)56)
#define SOT_FORMATSTR_ID_SYLK                   ((sal_uIntPtr)57)
#define SOT_FORMATSTR_ID_SYLK_BIGCAPS           ((sal_uIntPtr)58)
#define SOT_FORMATSTR_ID_LINK                   ((sal_uIntPtr)59)
#define SOT_FORMATSTR_ID_DIF                    ((sal_uIntPtr)60)
#define SOT_FORMATSTR_ID_STARDRAW_TABBAR        ((sal_uIntPtr)61)
#define SOT_FORMATSTR_ID_SONLK                  ((sal_uIntPtr)62)
#define SOT_FORMATSTR_ID_MSWORD_DOC             ((sal_uIntPtr)63)
#define SOT_FORMATSTR_ID_STAR_FRAMESET_DOC      ((sal_uIntPtr)64)
#define SOT_FORMATSTR_ID_OFFICE_DOC             ((sal_uIntPtr)65)
#define SOT_FORMATSTR_ID_NOTES_DOCINFO          ((sal_uIntPtr)66)
#define SOT_FORMATSTR_ID_NOTES_HNOTE            ((sal_uIntPtr)67)
#define SOT_FORMATSTR_ID_NOTES_NATIVE           ((sal_uIntPtr)68)
#define SOT_FORMATSTR_ID_SFX_DOC                ((sal_uIntPtr)69)
#define SOT_FORMATSTR_ID_EVDF                   ((sal_uIntPtr)70)
#define SOT_FORMATSTR_ID_ESDF                   ((sal_uIntPtr)71)
#define SOT_FORMATSTR_ID_IDF                    ((sal_uIntPtr)72)
#define SOT_FORMATSTR_ID_EFTP                   ((sal_uIntPtr)73)
#define SOT_FORMATSTR_ID_EFD                    ((sal_uIntPtr)74)
#define SOT_FORMATSTR_ID_SVX_FORMFIELDEXCH      ((sal_uIntPtr)75)
#define SOT_FORMATSTR_ID_EXTENDED_TABBAR        ((sal_uIntPtr)76)
#define SOT_FORMATSTR_ID_SBA_DATAEXCHANGE       ((sal_uIntPtr)77)
#define SOT_FORMATSTR_ID_SBA_FIELDDATAEXCHANGE  ((sal_uIntPtr)78)
#define SOT_FORMATSTR_ID_SBA_PRIVATE_URL        ((sal_uIntPtr)79)
#define SOT_FORMATSTR_ID_SBA_TABED              ((sal_uIntPtr)80)
#define SOT_FORMATSTR_ID_SBA_TABID              ((sal_uIntPtr)81)
#define SOT_FORMATSTR_ID_SBA_JOIN               ((sal_uIntPtr)82)
#define SOT_FORMATSTR_ID_OBJECTDESCRIPTOR       ((sal_uIntPtr)83)
#define SOT_FORMATSTR_ID_LINKSRCDESCRIPTOR      ((sal_uIntPtr)84)
#define SOT_FORMATSTR_ID_EMBED_SOURCE           ((sal_uIntPtr)85)
#define SOT_FORMATSTR_ID_LINK_SOURCE            ((sal_uIntPtr)86)
#define SOT_FORMATSTR_ID_EMBEDDED_OBJ           ((sal_uIntPtr)87)
#define SOT_FORMATSTR_ID_FILECONTENT            ((sal_uIntPtr)88)
#define SOT_FORMATSTR_ID_FILEGRPDESCRIPTOR      ((sal_uIntPtr)89)
#define SOT_FORMATSTR_ID_FILENAME               ((sal_uIntPtr)90)
#define SOT_FORMATSTR_ID_SD_OLE                 ((sal_uIntPtr)91)
#define SOT_FORMATSTR_ID_EMBEDDED_OBJ_OLE       ((sal_uIntPtr)92)
#define SOT_FORMATSTR_ID_EMBED_SOURCE_OLE       ((sal_uIntPtr)93)
#define SOT_FORMATSTR_ID_OBJECTDESCRIPTOR_OLE   ((sal_uIntPtr)94)
#define SOT_FORMATSTR_ID_LINKSRCDESCRIPTOR_OLE  ((sal_uIntPtr)95)
#define SOT_FORMATSTR_ID_LINK_SOURCE_OLE        ((sal_uIntPtr)96)
#define SOT_FORMATSTR_ID_SBA_CTRLDATAEXCHANGE   ((sal_uIntPtr)97)
#define SOT_FORMATSTR_ID_OUTPLACE_OBJ           ((sal_uIntPtr)98)
#define SOT_FORMATSTR_ID_CNT_OWN_CLIP           ((sal_uIntPtr)99)
#define SOT_FORMATSTR_ID_INET_IMAGE             ((sal_uIntPtr)100)
#define SOT_FORMATSTR_ID_NETSCAPE_IMAGE         ((sal_uIntPtr)101)
#define SOT_FORMATSTR_ID_SBA_FORMEXCHANGE       ((sal_uIntPtr)102)
#define SOT_FORMATSTR_ID_SBA_REPORTEXCHANGE     ((sal_uIntPtr)103)
#define SOT_FORMATSTR_ID_UNIFORMRESOURCELOCATOR ((sal_uIntPtr)104)
#define SOT_FORMATSTR_ID_STARCHARTDOCUMENT_50   ((sal_uIntPtr)105)
#define SOT_FORMATSTR_ID_GRAPHOBJ               ((sal_uIntPtr)106)
#define SOT_FORMATSTR_ID_STARWRITER_60          ((sal_uIntPtr)107)
#define SOT_FORMATSTR_ID_STARWRITERWEB_60       ((sal_uIntPtr)108)
#define SOT_FORMATSTR_ID_STARWRITERGLOB_60      ((sal_uIntPtr)109)
#define SOT_FORMATSTR_ID_STARDRAW_60            ((sal_uIntPtr)110)
#define SOT_FORMATSTR_ID_STARIMPRESS_60         ((sal_uIntPtr)111)
#define SOT_FORMATSTR_ID_STARCALC_60            ((sal_uIntPtr)112)
#define SOT_FORMATSTR_ID_STARCHART_60           ((sal_uIntPtr)113)
#define SOT_FORMATSTR_ID_STARMATH_60            ((sal_uIntPtr)114)
#define SOT_FORMATSTR_ID_WMF                    ((sal_uIntPtr)115)
#define SOT_FORMATSTR_ID_DBACCESS_QUERY         ((sal_uIntPtr)116)
#define SOT_FORMATSTR_ID_DBACCESS_TABLE         ((sal_uIntPtr)117)
#define SOT_FORMATSTR_ID_DBACCESS_COMMAND       ((sal_uIntPtr)118)
#define SOT_FORMATSTR_ID_DIALOG_60              ((sal_uIntPtr)119)
#define SOT_FORMATSTR_ID_EMF                    ((sal_uIntPtr)120)
#define SOT_FORMATSTR_ID_BIFF_8                 ((sal_uIntPtr)121)
#define SOT_FORMATSTR_ID_BMP                    ((sal_uIntPtr)122)
#define SOT_FORMATSTR_ID_HTML_NO_COMMENT        ((sal_uIntPtr)123)
#define SOT_FORMATSTR_ID_STARWRITER_8           ((sal_uIntPtr)124)
#define SOT_FORMATSTR_ID_STARWRITERWEB_8        ((sal_uIntPtr)125)
#define SOT_FORMATSTR_ID_STARWRITERGLOB_8       ((sal_uIntPtr)126)
#define SOT_FORMATSTR_ID_STARDRAW_8             ((sal_uIntPtr)127)
#define SOT_FORMATSTR_ID_STARIMPRESS_8          ((sal_uIntPtr)128)
#define SOT_FORMATSTR_ID_STARCALC_8             ((sal_uIntPtr)129)
#define SOT_FORMATSTR_ID_STARCHART_8            ((sal_uIntPtr)130)
#define SOT_FORMATSTR_ID_STARMATH_8             ((sal_uIntPtr)131)
#define SOT_FORMATSTR_ID_XFORMS                 ((sal_uIntPtr)132)
#define SOT_FORMATSTR_ID_STARWRITER_8_TEMPLATE  ((sal_uIntPtr)133)
#define SOT_FORMATSTR_ID_STARDRAW_8_TEMPLATE    ((sal_uIntPtr)134)
#define SOT_FORMATSTR_ID_STARIMPRESS_8_TEMPLATE ((sal_uIntPtr)135)
#define SOT_FORMATSTR_ID_STARCALC_8_TEMPLATE    ((sal_uIntPtr)136)
#define SOT_FORMATSTR_ID_STARCHART_8_TEMPLATE   ((sal_uIntPtr)137)
#define SOT_FORMATSTR_ID_STARMATH_8_TEMPLATE    ((sal_uIntPtr)138)
#define SOT_FORMATSTR_ID_STARBASE_8             ((sal_uIntPtr)139)
#define SOT_FORMATSTR_ID_HC_GDIMETAFILE         ((sal_uIntPtr)140)
#define SOT_FORMATSTR_ID_USER_END               SOT_FORMATSTR_ID_HC_GDIMETAFILE

#endif // _SOT_FORMATS_HXX

