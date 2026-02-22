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

#include <sal/config.h>

#include <tools/debug.hxx>
#include <tools/globname.hxx>
#include <tools/lazydelete.hxx>
#include <sot/exchange.hxx>
#include <sot/formats.hxx>
#include <comphelper/classids.hxx>
#include <com/sun/star/datatransfer/DataFlavor.hpp>
#include <comphelper/documentconstants.hxx>
#include <o3tl/string_view.hxx>

#include <memory>
#include <vector>

using namespace::com::sun::star::uno;
using namespace::com::sun::star::datatransfer;

namespace {

/*
 *  These tables contain all MimeTypes, format identifiers, and types used in
 *  the Office. The table is sorted by SotClipboardFormatId, and each ID is
 *  exactly 1 greater than its predecessor ID, so that the ID can be used as a
 *  table index.
 */

const DataFlavor* FormatArray_Impl()
{
    static const DataFlavor aInstance[] =
    {
        /*  0 NONE*/                      { u""_ustr, u""_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*  1 STRING*/                    { u"text/plain;charset=utf-16"_ustr, u"Text"_ustr, cppu::UnoType<OUString>::get()},
        /*  2 BITMAP*/                    { u"application/x-openoffice-bitmap;windows_formatname=\"Bitmap\""_ustr, u"Bitmap"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*  3 GDIMETAFILE*/               { u"application/x-openoffice-gdimetafile;windows_formatname=\"GDIMetaFile\""_ustr, u"GDIMetaFile"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*  4 PRIVATE*/                   { u"application/x-openoffice-private;windows_formatname=\"Private\""_ustr, u"Private"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*  5 SIMPLE_FILE*/               { u"application/x-openoffice-file;windows_formatname=\"FileNameW\""_ustr, u"FileNameW"_ustr, cppu::UnoType<OUString>::get() },
        /*  6 FILE_LIST*/                 { u"application/x-openoffice-filelist;windows_formatname=\"FileList\""_ustr, u"FileList"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*  7 EMPTY*/                     { u""_ustr, u""_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*  8 EMPTY*/                     { u""_ustr, u""_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*  9 EMPTY*/                     { u""_ustr, u""_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 10 RTF*/                       { u"text/rtf"_ustr, u"Rich Text Format"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 11 DRAWING*/                   { u"application/x-openoffice-drawing;windows_formatname=\"Drawing Format\""_ustr, u"Drawing Format"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 12 SVXB*/                      { u"application/x-openoffice-svxb;windows_formatname=\"SVXB (StarView Bitmap/Animation)\""_ustr, u"SVXB (StarView Bitmap/Animation)"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 13 SVIM*/                      { u"application/x-openoffice-svim;windows_formatname=\"SVIM (StarView ImageMap)\""_ustr, u"SVIM (StarView ImageMap)"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 14 XFA*/                       { u"application/x-libreoffice-xfa;windows_formatname=\"XFA (XOutDev FillAttr Any)\""_ustr, u"XFA (XOutDev FillAttr Any)"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 15 EDITENGINE_ODF_TEXT_FLAT*/  { u"application/vnd.oasis.opendocument.text-flat-xml"_ustr, u"Flat XML format (EditEngine ODF)"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 16 INTERNALLINK_STATE*/        { u"application/x-openoffice-internallink-state;windows_formatname=\"StatusInfo of SvxInternalLink\""_ustr, u"StatusInfo of SvxInternalLink"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 17 SOLK*/                      { u"application/x-openoffice-solk;windows_formatname=\"SOLK (StarOffice Link)\""_ustr, u"SOLK (StarOffice Link)"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 18 NETSCAPE_BOOKMARK*/         { u"application/x-openoffice-netscape-bookmark;windows_formatname=\"Netscape Bookmark\""_ustr, u"Netscape Bookmark"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 19 TREELISTBOX*/               { u"application/x-openoffice-treelistbox;windows_formatname=\"SV_LBOX_DD_FORMAT\""_ustr, u"SV_LBOX_DD_FORMAT"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 20 NATIVE*/                    { u"application/x-openoffice-native;windows_formatname=\"Native\""_ustr, u"Native"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 21 OWNERLINK*/                 { u"application/x-openoffice-ownerlink;windows_formatname=\"OwnerLink\""_ustr, u"OwnerLink"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 22 STARSERVER*/                { u"application/x-openoffice-starserver;windows_formatname=\"StarServerFormat\""_ustr, u"StarServerFormat"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 23 STAROBJECT*/                { u"application/x-openoffice-starobject;windows_formatname=\"StarObjectFormat\""_ustr, u"StarObjectFormat"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 24 APPLETOBJECT*/              { u"application/x-openoffice-appletobject;windows_formatname=\"Applet Object\""_ustr, u"Applet Object"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 25 PLUGIN_OBJECT*/             { u"application/x-openoffice-plugin-object;windows_formatname=\"PlugIn Object\""_ustr, u"PlugIn Object"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 26 STARWRITER_30*/             { u"application/x-openoffice-starwriter-30;windows_formatname=\"StarWriter 3.0\""_ustr, u"StarWriter 3.0"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 27 STARWRITER_40*/             { u"application/x-openoffice-starwriter-40;windows_formatname=\"StarWriter 4.0\""_ustr, u"StarWriter 4.0"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 28 STARWRITER_50*/             { u"application/x-openoffice-starwriter-50;windows_formatname=\"StarWriter 5.0\""_ustr, u"StarWriter 5.0"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 29 STARWRITERWEB_40*/          { u"application/x-openoffice-starwriterweb-40;windows_formatname=\"StarWriter/Web 4.0\""_ustr, u"StarWriter/Web 4.0"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 30 STARWRITERWEB_50*/          { u"application/x-openoffice-starwriterweb-50;windows_formatname=\"StarWriter/Web 5.0\""_ustr, u"StarWriter/Web 5.0"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 31 STARWRITERGLOB_40*/         { u"application/x-openoffice-starwriterglob-40;windows_formatname=\"StarWriter/Global 4.0\""_ustr, u"StarWriter/Global 4.0"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 32 STARWRITERGLOB_50*/         { u"application/x-openoffice-starwriterglob-50;windows_formatname=\"StarWriter/Global 5.0\""_ustr, u"StarWriter/Global 5.0"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 33 STARDRAW*/                  { u"application/x-openoffice-stardraw;windows_formatname=\"StarDrawDocument\""_ustr, u"StarDrawDocument"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 34 STARDRAW_40*/               { u"application/x-openoffice-stardraw-40;windows_formatname=\"StarDrawDocument 4.0\""_ustr, u"StarDrawDocument 4.0"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 35 STARIMPRESS_50*/            { u"application/x-openoffice-starimpress-50;windows_formatname=\"StarImpress 5.0\""_ustr, u"StarImpress 5.0"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 36 STARDRAW_50*/               { u"application/x-openoffice-stardraw-50;windows_formatname=\"StarDraw 5.0\""_ustr, u"StarDraw 5.0"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 37 STARCALC*/                  { u"application/x-openoffice-starcalc;windows_formatname=\"StarCalcDocument\""_ustr, u"StarCalcDocument"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 38 STARCALC_40*/               { u"application/x-openoffice-starcalc-40;windows_formatname=\"StarCalc 4.0\""_ustr, u"StarCalc 4.0"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 39 STARCALC_50*/               { u"application/x-openoffice-starcalc-50;windows_formatname=\"StarCalc 5.0\""_ustr, u"StarCalc 5.0"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 40 STARCHART*/                 { u"application/x-openoffice-starchart;windows_formatname=\"StarChartDocument\""_ustr, u"StarChartDocument"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 41 STARCHART_40*/              { u"application/x-openoffice-starchart-40;windows_formatname=\"StarChartDocument 4.0\""_ustr, u"StarChartDocument 4.0"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 42 STARCHART_50*/              { u"application/x-openoffice-starchart-50;windows_formatname=\"StarChart 5.0\""_ustr, u"StarChart 5.0"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 43 STARIMAGE*/                 { u"application/x-openoffice-starimage;windows_formatname=\"StarImageDocument\""_ustr, u"StarImageDocument"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 44 STARIMAGE_40*/              { u"application/x-openoffice-starimage-40;windows_formatname=\"StarImageDocument 4.0\""_ustr, u"StarImageDocument 4.0"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 45 STARIMAGE_50*/              { u"application/x-openoffice-starimage-50;windows_formatname=\"StarImage 5.0\""_ustr, u"StarImage 5.0"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 46 STARMATH*/                  { u"application/x-openoffice-starmath;windows_formatname=\"StarMath\""_ustr, u"StarMath"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 47 STARMATH_40*/               { u"application/x-openoffice-starmath-40;windows_formatname=\"StarMathDocument 4.0\""_ustr, u"StarMathDocument 4.0"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 48 STARMATH_50*/               { u"application/x-openoffice-starmath-50;windows_formatname=\"StarMath 5.0\""_ustr, u"StarMath 5.0"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 49 STAROBJECT_PAINTDOC*/       { u"application/x-openoffice-starobject-paintdoc;windows_formatname=\"StarObjectPaintDocument\""_ustr, u"StarObjectPaintDocument"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 50 FILLED_AREA*/               { u"application/x-openoffice-filled-area;windows_formatname=\"FilledArea\""_ustr, u"FilledArea"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 51 HTML*/                      { u"text/html"_ustr, u"HTML (HyperText Markup Language)"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 52 HTML_SIMPLE*/               { u"application/x-openoffice-html-simple;windows_formatname=\"HTML Format\""_ustr, u"HTML Format"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 53 CHAOS*/                     { u"application/x-openoffice-chaos;windows_formatname=\"FORMAT_CHAOS\""_ustr, u"FORMAT_CHAOS"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 54 CNT_MSGATTACHFILE*/         { u"application/x-openoffice-cnt-msgattachfile;windows_formatname=\"CNT_MSGATTACHFILE_FORMAT\""_ustr, u"CNT_MSGATTACHFILE_FORMAT"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 55 BIFF_5*/                    { u"application/x-openoffice-biff5;windows_formatname=\"Biff5\""_ustr, u"Biff5"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 56 BIFF__5*/                   { u"application/x-openoffice-biff-5;windows_formatname=\"Biff 5\""_ustr, u"Biff 5"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 57 SYLK*/                      { u"application/x-openoffice-sylk;windows_formatname=\"Sylk\""_ustr, u"Sylk"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 58 SYLK_BIGCAPS*/              { u"application/x-openoffice-sylk-bigcaps;windows_formatname=\"SYLK\""_ustr, u"SYLK"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 59 LINK*/                      { u"application/x-openoffice-link;windows_formatname=\"Link\""_ustr, u"Link"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 60 DIF*/                       { u"application/x-openoffice-dif;windows_formatname=\"DIF\""_ustr, u"DIF"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 61 STARDRAW_TABBAR*/           { u"application/x-openoffice-stardraw-tabbar;windows_formatname=\"StarDraw TabBar\""_ustr, u"StarDraw TabBar"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 62 SONLK*/                     { u"application/x-openoffice-sonlk;windows_formatname=\"SONLK (StarOffice Navi Link)\""_ustr, u"SONLK (StarOffice Navi Link)"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 63 MSWORD_DOC*/                { u"application/msword"_ustr, u"MSWordDoc"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 64 STAR_FRAMESET_DOC*/         { u"application/x-openoffice-star-frameset-doc;windows_formatname=\"StarFrameSetDocument\""_ustr, u"StarFrameSetDocument"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 65 OFFICE_DOC*/                { u"application/x-openoffice-office-doc;windows_formatname=\"OfficeDocument\""_ustr, u"OfficeDocument"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 66 NOTES_DOCINFO*/             { u"application/x-openoffice-notes-docinfo;windows_formatname=\"NotesDocInfo\""_ustr, u"NotesDocInfo"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 67 NOTES_HNOTE*/               { u"application/x-openoffice-notes-hnote;windows_formatname=\"NoteshNote\""_ustr, u"NoteshNote"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 68 NOTES_NATIVE*/              { u"application/x-openoffice-notes-native;windows_formatname=\"Native\""_ustr, u"Native"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 69 SFX_DOC*/                   { u"application/x-openoffice-sfx-doc;windows_formatname=\"SfxDocument\""_ustr, u"SfxDocument"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 70 EVDF*/                      { u"application/x-openoffice-evdf;windows_formatname=\"EVDF (Explorer View Dummy Format)\""_ustr, u"EVDF (Explorer View Dummy Format)"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 71 ESDF*/                      { u"application/x-openoffice-esdf;windows_formatname=\"ESDF (Explorer Search Dummy Format)\""_ustr, u"ESDF (Explorer Search Dummy Format)"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 72 IDF*/                       { u"application/x-openoffice-idf;windows_formatname=\"IDF (Iconview Dummy Format)\""_ustr, u"IDF (Iconview Dummy Format)"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 73 EFTP*/                      { u"application/x-openoffice-eftp;windows_formatname=\"EFTP (Explorer Ftp File)\""_ustr, u"EFTP (Explorer Ftp File)"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 74 EFD*/                       { u"application/x-openoffice-efd;windows_formatname=\"EFD (Explorer Ftp Dir)\""_ustr, u"EFD (Explorer Ftp Dir)"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 75 SVX_FORMFIELDEXCH*/         { u"application/x-openoffice-svx-formfieldexch;windows_formatname=\"SvxFormFieldExch\""_ustr, u"SvxFormFieldExch"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 76 EXTENDED_TABBAR*/           { u"application/x-openoffice-extended-tabbar;windows_formatname=\"ExtendedTabBar\""_ustr, u"ExtendedTabBar"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 77 SBA_DATAEXCHANGE*/          { u"application/x-openoffice-sba-dataexchange;windows_formatname=\"SBA-DATAFORMAT\""_ustr, u"SBA-DATAFORMAT"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 78 SBA_FIELDDATAEXCHANGE*/     { u"application/x-openoffice-sba-fielddataexchange;windows_formatname=\"SBA-FIELDFORMAT\""_ustr, u"SBA-FIELDFORMAT"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 79 SBA_PRIVATE_URL*/           { u"application/x-openoffice-sba-private-url;windows_formatname=\"SBA-PRIVATEURLFORMAT\""_ustr, u"SBA-PRIVATEURLFORMAT"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 80 SBA_TABED*/                 { u"application/x-openoffice-sba-tabed;windows_formatname=\"Tabed\""_ustr, u"Tabed"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 81 SBA_TABID*/                 { u"application/x-openoffice-sba-tabid;windows_formatname=\"Tabid\""_ustr, u"Tabid"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 82 SBA_JOIN*/                  { u"application/x-openoffice-sba-join;windows_formatname=\"SBA-JOINFORMAT\""_ustr, u"SBA-JOINFORMAT"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 83 OBJECTDESCRIPTOR*/          { u"application/x-openoffice-objectdescriptor-xml;windows_formatname=\"Star Object Descriptor (XML)\""_ustr, u"Star Object Descriptor (XML)"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 84 LINKSRCDESCRIPTOR*/         { u"application/x-openoffice-linksrcdescriptor-xml;windows_formatname=\"Star Link Source Descriptor (XML)\""_ustr, u"Star Link Source Descriptor (XML)"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 85 EMBED_SOURCE*/              { u"application/x-openoffice-embed-source-xml;windows_formatname=\"Star Embed Source (XML)\""_ustr, u"Star Embed Source (XML)"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 86 LINK_SOURCE*/               { u"application/x-openoffice-link-source-xml;windows_formatname=\"Star Link Source (XML)\""_ustr, u"Star Link Source (XML)"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 87 EMBEDDED_OBJ*/              { u"application/x-openoffice-embedded-obj-xml;windows_formatname=\"Star Embedded Object (XML)\""_ustr, u"Star Embedded Object (XML)"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 88 FILECONTENT*/               { u"application/x-openoffice-filecontent;windows_formatname=\"FileContents\""_ustr, u"FileContents"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 89 FILEGRPDESCRIPTOR*/         { u"application/x-openoffice-filegrpdescriptor;windows_formatname=\"FileGroupDescriptorW\""_ustr, u"FileGroupDescriptorW"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 90 FILENAME*/                  { u"application/x-openoffice-filename;windows_formatname=\"FileNameW\""_ustr, u"FileNameW"_ustr, cppu::UnoType<OUString>::get() },
        /* 91 SD_OLE*/                    { u"application/x-openoffice-sd-ole;windows_formatname=\"SD-OLE\""_ustr, u"SD-OLE"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 92 EMBEDDED_OBJ_OLE*/          { u"application/x-openoffice-embedded-obj-ole;windows_formatname=\"Embedded Object\""_ustr, u"Embedded Object"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 93 EMBED_SOURCE_OLE*/          { u"application/x-openoffice-embed-source-ole;windows_formatname=\"Embed Source\""_ustr, u"Embed Source"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 94 OBJECTDESCRIPTOR_OLE*/      { u"application/x-openoffice-objectdescriptor-ole;windows_formatname=\"Object Descriptor\""_ustr, u"Object Descriptor"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 95 LINKSRCDESCRIPTOR_OLE*/     { u"application/x-openoffice-linkdescriptor-ole;windows_formatname=\"Link Source Descriptor\""_ustr, u"Link Source Descriptor"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 96 LINK_SOURCE_OLE*/           { u"application/x-openoffice-link-source-ole;windows_formatname=\"Link Source\""_ustr, u"Link Source"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 97 SBA_CTRLDATAEXCHANGE*/      { u"application/x-openoffice-sba-ctrldataexchange;windows_formatname=\"SBA-CTRLFORMAT\""_ustr, u"SBA-CTRLFORMAT"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 98 OUTPLACE_OBJ*/              { u"application/x-openoffice-outplace-obj;windows_formatname=\"OutPlace Object\""_ustr, u"OutPlace Object"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 99 CNT_OWN_CLIP*/              { u"application/x-openoffice-cnt-own-clip;windows_formatname=\"CntOwnClipboard\""_ustr, u"CntOwnClipboard"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*100 INET_IMAGE*/                { u"application/x-openoffice-inet-image;windows_formatname=\"SO-INet-Image\""_ustr, u"SO-INet-Image"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*101 NETSCAPE_IMAGE*/            { u"application/x-openoffice-netscape-image;windows_formatname=\"Netscape Image Format\""_ustr, u"Netscape Image Format"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*102 SBA_FORMEXCHANGE*/          { u"application/x-openoffice-sba-formexchange;windows_formatname=\"SBA_FORMEXCHANGE\""_ustr, u"SBA_FORMEXCHANGE"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*103 SBA_REPORTEXCHANGE*/        { u"application/x-openoffice-sba-reportexchange;windows_formatname=\"SBA_REPORTEXCHANGE\""_ustr, u"SBA_REPORTEXCHANGE"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*104 UNIFORMRESOURCELOCATOR*/    { u"application/x-openoffice-uniformresourcelocator;windows_formatname=\"UniformResourceLocatorW\""_ustr, u"UniformResourceLocatorW"_ustr, cppu::UnoType<OUString>::get() },
        /*105 STARCHARTDOCUMENT_50*/      { u"application/x-openoffice-starchartdocument-50;windows_formatname=\"StarChartDocument 5.0\""_ustr, u"StarChartDocument 5.0"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*106 GRAPHOBJ*/                  { u"application/x-openoffice-graphobj;windows_formatname=\"Graphic Object\""_ustr, u"Graphic Object"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*107 STARWRITER_60*/             { MIMETYPE_VND_SUN_XML_WRITER, u"Writer 6.0"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*108 STARWRITERWEB_60*/          { MIMETYPE_VND_SUN_XML_WRITER_WEB, u"Writer/Web 6.0"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*109 STARWRITERGLOB_60*/         { MIMETYPE_VND_SUN_XML_WRITER_GLOBAL, u"Writer/Global 6.0"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*110 STARDRAW_60*/               { MIMETYPE_VND_SUN_XML_DRAW, u"Draw 6.0"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*111 STARIMPRESS_60*/            { MIMETYPE_VND_SUN_XML_IMPRESS, u"Impress 6.0"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*112 STARCALC_60*/               { MIMETYPE_VND_SUN_XML_CALC, u"Calc 6.0"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*113 STARCHART_60*/              { MIMETYPE_VND_SUN_XML_CHART, u"Chart 6.0"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*114 STARMATH_60*/               { MIMETYPE_VND_SUN_XML_MATH, u"Math 6.0"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*115 WMF*/                       { u"application/x-openoffice-wmf;windows_formatname=\"Image WMF\""_ustr, u"Windows MetaFile"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*116 DBACCESS_QUERY*/            { u"application/x-openoffice-dbaccess-query;windows_formatname=\"Data source Query Object\""_ustr, u"Data source Query Object"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*117 DBACCESS_TABLE*/            { u"application/x-openoffice-dbaccess-table;windows_formatname=\"Data source Table\""_ustr, u"Data source Table"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*118 DBACCESS_COMMAND*/          { u"application/x-openoffice-dbaccess-command;windows_formatname=\"SQL query\""_ustr, u"SQL query"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*119 DIALOG_60*/                 { u"application/vnd.sun.xml.dialog"_ustr, u"Dialog 6.0"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*120 EMF*/                       { u"application/x-openoffice-emf;windows_formatname=\"Image EMF\""_ustr, u"Windows Enhanced MetaFile"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*121 BIFF_8*/                    { u"application/x-openoffice-biff-8;windows_formatname=\"Biff8\""_ustr, u"Biff8"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*122 BMP*/                       { u"image/bmp"_ustr, u"Windows Bitmap"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*123 HTML_NO_COMMENT */          { u"application/x-openoffice-html-no-comment;windows_formatname=\"HTML Format\""_ustr, u"HTML (no comment)"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*124 STARWRITER_8*/              { MIMETYPE_OASIS_OPENDOCUMENT_TEXT, u"Writer 8"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*125 STARWRITERWEB_8*/           { MIMETYPE_OASIS_OPENDOCUMENT_TEXT_WEB, u"Writer/Web 8"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*126 STARWRITERGLOB_8*/          { MIMETYPE_OASIS_OPENDOCUMENT_TEXT_GLOBAL, u"Writer/Global 8"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*127 STARWDRAW_8*/               { MIMETYPE_OASIS_OPENDOCUMENT_DRAWING, u"Draw 8"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*128 STARIMPRESS_8*/             { MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION, u"Impress 8"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*129 STARCALC_8*/                { MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET, u"Calc 8"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*130 STARCHART_8*/               { MIMETYPE_OASIS_OPENDOCUMENT_CHART, u"Chart 8"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*131 STARMATH_8*/                { MIMETYPE_OASIS_OPENDOCUMENT_FORMULA, u"Math 8"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*132 XFORMS */                   { u"application/x-openoffice-xforms;windows_formatname=\"??? Format\""_ustr, u"???"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*133 STARWRITER_8_TEMPLATE*/     { MIMETYPE_OASIS_OPENDOCUMENT_TEXT_TEMPLATE, u"Writer 8 Template"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*134 STARWDRAW_8_TEMPLATE*/      { MIMETYPE_OASIS_OPENDOCUMENT_DRAWING_TEMPLATE, u"Draw 8 Template"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*135 STARIMPRESS_8_TEMPLATE*/    { MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION_TEMPLATE, u"Impress 8 Template"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*136 STARCALC_8_TEMPLATE*/       { MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET_TEMPLATE, u"Calc 8 Template"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*137 STARCHART_8_TEMPLATE*/      { MIMETYPE_OASIS_OPENDOCUMENT_CHART_TEMPLATE, u"Chart 8 Template"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*138 STARMATH_8_TEMPLATE*/       { MIMETYPE_OASIS_OPENDOCUMENT_FORMULA_TEMPLATE, u"Math 8 Template"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*139 STARBASE_8*/                { MIMETYPE_OASIS_OPENDOCUMENT_DATABASE, u"StarBase 8"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*140 HC_GDIMETAFILE*/            { u"application/x-openoffice-highcontrast-gdimetafile;windows_formatname=\"GDIMetaFile\""_ustr, u"High Contrast GDIMetaFile"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*141 PNG*/                       { u"image/png"_ustr, u"PNG"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*142 STARWRITERGLOB_8_TEMPLATE*/ { MIMETYPE_OASIS_OPENDOCUMENT_TEXT_GLOBAL_TEMPLATE, u"Writer/Global 8 Template"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*143 MATHML*/                    { u"application/mathml+xml"_ustr, u"MathML"_ustr, ::cppu::UnoType<const Sequence< sal_Int8 >>::get() },
        /*144 JPEG*/                      { u"image/jpeg"_ustr, u"JPEG Bitmap"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*145 RICHTEXT*/                  { u"text/richtext"_ustr, u"Richtext Format"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*146 STRING_TSVC*/               { u"application/x-libreoffice-tsvc"_ustr, u"Text TSV-Calc"_ustr, cppu::UnoType<OUString>::get() },
        /*147 PDF*/                       { u"application/pdf"_ustr, u"PDF Document"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*148 SVG*/                       { u"image/svg+xml;windows_formatname=\"image/svg+xml\""_ustr, u"SVG"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*149 MARKDOWN*/                  { u"text/markdown"_ustr, u"Markdown"_ustr, cppu::UnoType<OUString>::get() },
        /*150 BIFF_12*/                   { u"application/x-openoffice-biff-12;windows_formatname=\"Biff12\""_ustr, u"Biff12"_ustr, cppu::UnoType<Sequence<sal_Int8>>::get() },
    };
    static_assert(std::size(aInstance) == size_t(SotClipboardFormatId::USER_END) + 1);
    return &aInstance[0];
};

    typedef std::vector<css::datatransfer::DataFlavor> tDataFlavorList;

tDataFlavorList& InitFormats_Impl()
{
    static tools::DeleteOnDeinit<tDataFlavorList> gImplData;

    return *gImplData.get();
}

SotClipboardFormatId FormatIdOfDynamicFormat(size_t i)
{
    assert(i < InitFormats_Impl().size());
    return static_cast<SotClipboardFormatId>(i + static_cast<int>(SotClipboardFormatId::USER_END) + 1);
}

size_t PosOfDynamicFormat(SotClipboardFormatId nFormat)
{
    assert(nFormat > SotClipboardFormatId::USER_END);
    return static_cast<size_t>(nFormat) - static_cast<size_t>(SotClipboardFormatId::USER_END) - 1;
}

SotClipboardFormatId AddDynamicFormat(const css::datatransfer::DataFlavor& rFlavor)
{
    tDataFlavorList& rL = InitFormats_Impl();
    rL.push_back(rFlavor);
    return FormatIdOfDynamicFormat(rL.size() - 1);
}

SotClipboardFormatId GetFormatIdFromMimeType_impl(std::u16string_view rMimeType,
                                                  bool allowMoreParams)
{
    const DataFlavor* pFormatArray_Impl = FormatArray_Impl();
    for( SotClipboardFormatId i = SotClipboardFormatId::STRING; i <= SotClipboardFormatId::FILE_LIST;  ++i )
        if( rMimeType == pFormatArray_Impl[ static_cast<int>(i) ].MimeType )
            return i;

    // BM: the chart format 105 ("StarChartDocument 5.0") was written
    // only into 5.1 chart documents - in 5.0 and 5.2 it was 42 ("StarChart 5.0")
    // The registry only contains the entry for the 42 format id.
    for( SotClipboardFormatId i = SotClipboardFormatId::RTF; i <= SotClipboardFormatId::USER_END;  ++i )
        if (std::u16string_view rest;
            o3tl::starts_with(rMimeType, pFormatArray_Impl[static_cast<int>(i)].MimeType, &rest)
            && (rest.empty() || (allowMoreParams && rest.starts_with(';'))))
            return ( (i == SotClipboardFormatId::STARCHARTDOCUMENT_50)
                     ? SotClipboardFormatId::STARCHART_50
                     : i );

    // then in the dynamic list
    tDataFlavorList& rL = InitFormats_Impl();
    for( tDataFlavorList::size_type i = 0; i < rL.size(); i++ )
    {
        if( rMimeType == rL[ i ].MimeType )
            return FormatIdOfDynamicFormat(i);
    }

    return SotClipboardFormatId::NONE;
}
}

/*************************************************************************
|*    Description       CLIP.SDW
*************************************************************************/
SotClipboardFormatId SotExchange::RegisterFormatName( const OUString& rName )
{
    const DataFlavor* pFormatArray_Impl = FormatArray_Impl();
    // test the default first - name
    for( SotClipboardFormatId i = SotClipboardFormatId::STRING; i <= SotClipboardFormatId::FILE_LIST;  ++i )
        if( rName == pFormatArray_Impl[ static_cast<int>(i) ].HumanPresentableName )
            return i;

    // BM: the chart format 105 ("StarChartDocument 5.0") was written
    // only into 5.1 chart documents - in 5.0 and 5.2 it was 42 ("StarChart 5.0")
    // The registry only contains the entry for the 42 format id.
    for( SotClipboardFormatId i = SotClipboardFormatId::RTF; i <= SotClipboardFormatId::USER_END;  ++i )
        if( rName == pFormatArray_Impl[ static_cast<int>(i) ].HumanPresentableName )
            return ( (i == SotClipboardFormatId::STARCHARTDOCUMENT_50)
                     ? SotClipboardFormatId::STARCHART_50
                     : i );

    // then in the dynamic list
    tDataFlavorList& rL = InitFormats_Impl();
    for( tDataFlavorList::size_type i = 0; i < rL.size(); i++ )
    {
        auto const& rFlavor = rL[ i ];
        if( rName == rFlavor.HumanPresentableName )
            return FormatIdOfDynamicFormat(i);
    }

    return AddDynamicFormat({ rName, rName, cppu::UnoType<OUString>::get() });
}

SotClipboardFormatId SotExchange::RegisterFormatMimeType( const OUString& rMimeType )
{
    SotClipboardFormatId nRet = GetFormatIdFromMimeType(rMimeType);

    if( nRet == SotClipboardFormatId::NONE )
        nRet = AddDynamicFormat({ rMimeType, rMimeType, cppu::UnoType<OUString>::get() });

    return nRet;
}

/*************************************************************************
|*    Description       CLIP.SDW
*************************************************************************/
SotClipboardFormatId SotExchange::RegisterFormat( const DataFlavor& rFlavor )
{
    SotClipboardFormatId nRet = GetFormat( rFlavor );

    if( nRet == SotClipboardFormatId::NONE )
        nRet = AddDynamicFormat(rFlavor);

    return nRet;
}

bool SotExchange::GetFormatDataFlavor( SotClipboardFormatId nFormat, DataFlavor& rFlavor )
{
    bool bRet;

    if( SotClipboardFormatId::USER_END >= nFormat )
    {
        rFlavor = FormatArray_Impl()[static_cast<int>(nFormat)];

        bRet = true;
    }
    else
    {
        tDataFlavorList& rL = InitFormats_Impl();

        unsigned i = PosOfDynamicFormat(nFormat);

        if( rL.size() > i )
        {
            rFlavor = rL[ i ];
            bRet = true;
        }
        else
        {
            rFlavor = DataFlavor();
            bRet = false;
        }
    }

    DBG_ASSERT( bRet, "SotExchange::GetFormatDataFlavor(): DataFlavor not initialized" );

    return bRet;
}

OUString SotExchange::GetFormatMimeType( SotClipboardFormatId nFormat )
{
    OUString sMimeType;
    if( SotClipboardFormatId::USER_END >= nFormat )
        sMimeType = FormatArray_Impl()[static_cast<int>(nFormat)].MimeType;
    else
    {
        tDataFlavorList& rL = InitFormats_Impl();

        unsigned i = PosOfDynamicFormat(nFormat);

        if( rL.size() > i )
            sMimeType = rL[ i ].MimeType;
    }

    DBG_ASSERT( !sMimeType.isEmpty(), "SotExchange::GetFormatMimeType(): DataFlavor not initialized" );

    return sMimeType;
}

SotClipboardFormatId SotExchange::GetFormatIdFromMimeType( std::u16string_view rMimeType )
{
    return GetFormatIdFromMimeType_impl(rMimeType, false);
}

/*************************************************************************
|*    Description       CLIP.SDW
*************************************************************************/
SotClipboardFormatId SotExchange::GetFormat( const DataFlavor& rFlavor )
{
    return GetFormatIdFromMimeType_impl(rFlavor.MimeType, true);
}

/*************************************************************************
|*    Description       CLIP.SDW
*************************************************************************/
OUString SotExchange::GetFormatName( SotClipboardFormatId nFormat )
{
    DataFlavor  aFlavor;
    OUString      aRet;

    if( GetFormatDataFlavor( nFormat, aFlavor ) )
        aRet = aFlavor.HumanPresentableName;

    return aRet;
}

bool SotExchange::IsInternal( const SvGlobalName& rName )
{
    return rName == SvGlobalName(SO3_SW_CLASSID_60) ||
         rName == SvGlobalName(SO3_SC_CLASSID_60) ||
         rName == SvGlobalName(SO3_SIMPRESS_CLASSID_60) ||
         rName == SvGlobalName(SO3_SDRAW_CLASSID_60) ||
         rName == SvGlobalName(SO3_SCH_CLASSID_60) ||
         rName == SvGlobalName(SO3_SM_CLASSID_60) ||
         rName == SvGlobalName(SO3_SWWEB_CLASSID_60) ||
         rName == SvGlobalName(SO3_SWGLOB_CLASSID_60);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
