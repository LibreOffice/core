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
#include <tools/solar.h>
#include <tools/globname.hxx>
#include <sot/exchange.hxx>
#include <sot/formats.hxx>
#include <sysformats.hxx>
#include <comphelper/classids.hxx>
#include <rtl/instance.hxx>
#include <com/sun/star/datatransfer/DataFlavor.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <comphelper/documentconstants.hxx>

#include <memory>
#include <vector>

using namespace::com::sun::star::uno;
using namespace::com::sun::star::datatransfer;

/*
 *  These tables contain all MimeTypes, format identifiers, and types used in
 *  the Office. The table is sorted by the format string ID, and each ID is
 *  exactly 1 greater than its predecessor ID, so that the ID can be used as a
 *  table index.
 */
struct DataFlavorRepresentation
{
    const char*            pMimeType;
    const char*            pName;
    const css::uno::Type*  pType;
};

namespace
{
    struct ImplFormatArray_Impl
    {
        const DataFlavorRepresentation* operator()()
        {
            static const DataFlavorRepresentation aInstance[] =
            {
            /*  0 SOT_FORMAT_SYSTEM_START*/                 { "", "", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*  1 SotClipboardFormatId::STRING*/                       { "text/plain;charset=utf-16", "Text", &cppu::UnoType<OUString>::get()},
            /*  2 SotClipboardFormatId::BITMAP*/                       { "application/x-openoffice-bitmap;windows_formatname=\"Bitmap\"", "Bitmap", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*  3 SotClipboardFormatId::GDIMETAFILE*/                  { "application/x-openoffice-gdimetafile;windows_formatname=\"GDIMetaFile\"", "GDIMetaFile", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*  4 SotClipboardFormatId::PRIVATE*/                      { "application/x-openoffice-private;windows_formatname=\"Private\"", "Private", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*  5 SotClipboardFormatId::SIMPLE_FILE*/                         { "application/x-openoffice-file;windows_formatname=\"FileName\"", "FileName", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*  6 SotClipboardFormatId::FILE_LIST*/                    { "application/x-openoffice-filelist;windows_formatname=\"FileList\"", "FileList", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*  7 EMPTY*/                                   { "", "", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*  8 EMPTY*/                                   { "", "", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*  9 EMPTY*/                                   { "", "", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 10 SotClipboardFormatId::RTF*/                          { "text/rtf", "Rich Text Format", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 11 SotClipboardFormatId::DRAWING*/                { "application/x-openoffice-drawing;windows_formatname=\"Drawing Format\"", "Drawing Format", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 12 SotClipboardFormatId::SVXB*/                   { "application/x-openoffice-svxb;windows_formatname=\"SVXB (StarView Bitmap/Animation)\"", "SVXB (StarView Bitmap/Animation)", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 13 SotClipboardFormatId::SVIM*/                   { "application/x-openoffice-svim;windows_formatname=\"SVIM (StarView ImageMap)\"", "SVIM (StarView ImageMap)", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 14 SotClipboardFormatId::XFA*/                    { "application/x-libreoffice-xfa;windows_formatname=\"XFA (XOutDev FillAttr Any)\"", "XFA (XOutDev FillAttr Any)", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 15 SotClipboardFormatId::EDITENGINE_ODF_TEXT_FLAT*/ { "application/vnd.oasis.opendocument.text-flat-xml", "Flat XML format (EditEngine ODF)", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 16 SotClipboardFormatId::INTERNALLINK_STATE*/     { "application/x-openoffice-internallink-state;windows_formatname=\"StatusInfo vom SvxInternalLink\"", "StatusInfo vom SvxInternalLink", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 17 SotClipboardFormatId::SOLK*/                   { "application/x-openoffice-solk;windows_formatname=\"SOLK (StarOffice Link)\"", "SOLK (StarOffice Link)", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 18 SotClipboardFormatId::NETSCAPE_BOOKMARK*/      { "application/x-openoffice-netscape-bookmark;windows_formatname=\"Netscape Bookmark\"", "Netscape Bookmark", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 19 SotClipboardFormatId::TREELISTBOX*/            { "application/x-openoffice-treelistbox;windows_formatname=\"SV_LBOX_DD_FORMAT\"", "SV_LBOX_DD_FORMAT", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 20 SotClipboardFormatId::NATIVE*/                 { "application/x-openoffice-native;windows_formatname=\"Native\"", "Native", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 21 SotClipboardFormatId::OWNERLINK*/              { "application/x-openoffice-ownerlink;windows_formatname=\"OwnerLink\"", "OwnerLink", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 22 SotClipboardFormatId::STARSERVER*/             { "application/x-openoffice-starserver;windows_formatname=\"StarServerFormat\"", "StarServerFormat", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 23 SotClipboardFormatId::STAROBJECT*/             { "application/x-openoffice-starobject;windows_formatname=\"StarObjectFormat\"", "StarObjectFormat", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 24 SotClipboardFormatId::APPLETOBJECT*/           { "application/x-openoffice-appletobject;windows_formatname=\"Applet Object\"", "Applet Object", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 25 SotClipboardFormatId::PLUGIN_OBJECT*/          { "application/x-openoffice-plugin-object;windows_formatname=\"PlugIn Object\"", "PlugIn Object", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 26 SotClipboardFormatId::STARWRITER_30*/          { "application/x-openoffice-starwriter-30;windows_formatname=\"StarWriter 3.0\"", "StarWriter 3.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 27 SotClipboardFormatId::STARWRITER_40*/          { "application/x-openoffice-starwriter-40;windows_formatname=\"StarWriter 4.0\"", "StarWriter 4.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 28 SotClipboardFormatId::STARWRITER_50*/          { "application/x-openoffice-starwriter-50;windows_formatname=\"StarWriter 5.0\"", "StarWriter 5.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 29 SotClipboardFormatId::STARWRITERWEB_40*/       { "application/x-openoffice-starwriterweb-40;windows_formatname=\"StarWriter/Web 4.0\"", "StarWriter/Web 4.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 30 SotClipboardFormatId::STARWRITERWEB_50*/       { "application/x-openoffice-starwriterweb-50;windows_formatname=\"StarWriter/Web 5.0\"", "StarWriter/Web 5.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 31 SotClipboardFormatId::STARWRITERGLOB_40*/      { "application/x-openoffice-starwriterglob-40;windows_formatname=\"StarWriter/Global 4.0\"", "StarWriter/Global 4.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 32 SotClipboardFormatId::STARWRITERGLOB_50*/      { "application/x-openoffice-starwriterglob-50;windows_formatname=\"StarWriter/Global 5.0\"", "StarWriter/Global 5.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 33 SotClipboardFormatId::STARDRAW*/               { "application/x-openoffice-stardraw;windows_formatname=\"StarDrawDocument\"", "StarDrawDocument", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 34 SotClipboardFormatId::STARDRAW_40*/            { "application/x-openoffice-stardraw-40;windows_formatname=\"StarDrawDocument 4.0\"", "StarDrawDocument 4.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 35 SotClipboardFormatId::STARIMPRESS_50*/         { "application/x-openoffice-starimpress-50;windows_formatname=\"StarImpress 5.0\"", "StarImpress 5.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 36 SotClipboardFormatId::STARDRAW_50*/            { "application/x-openoffice-stardraw-50;windows_formatname=\"StarDraw 5.0\"", "StarDraw 5.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 37 SotClipboardFormatId::STARCALC*/               { "application/x-openoffice-starcalc;windows_formatname=\"StarCalcDocument\"", "StarCalcDocument", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 38 SotClipboardFormatId::STARCALC_40*/            { "application/x-openoffice-starcalc-40;windows_formatname=\"StarCalc 4.0\"", "StarCalc 4.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 39 SotClipboardFormatId::STARCALC_50*/            { "application/x-openoffice-starcalc-50;windows_formatname=\"StarCalc 5.0\"", "StarCalc 5.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 40 SotClipboardFormatId::STARCHART*/              { "application/x-openoffice-starchart;windows_formatname=\"StarChartDocument\"", "StarChartDocument", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 41 SotClipboardFormatId::STARCHART_40*/           { "application/x-openoffice-starchart-40;windows_formatname=\"StarChartDocument 4.0\"", "StarChartDocument 4.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 42 SotClipboardFormatId::STARCHART_50*/           { "application/x-openoffice-starchart-50;windows_formatname=\"StarChart 5.0\"", "StarChart 5.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 43 SotClipboardFormatId::STARIMAGE*/              { "application/x-openoffice-starimage;windows_formatname=\"StarImageDocument\"", "StarImageDocument", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 44 SotClipboardFormatId::STARIMAGE_40*/           { "application/x-openoffice-starimage-40;windows_formatname=\"StarImageDocument 4.0\"", "StarImageDocument 4.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 45 SotClipboardFormatId::STARIMAGE_50*/           { "application/x-openoffice-starimage-50;windows_formatname=\"StarImage 5.0\"", "StarImage 5.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 46 SotClipboardFormatId::STARMATH*/               { "application/x-openoffice-starmath;windows_formatname=\"StarMath\"", "StarMath", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 47 SotClipboardFormatId::STARMATH_40*/            { "application/x-openoffice-starmath-40;windows_formatname=\"StarMathDocument 4.0\"", "StarMathDocument 4.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 48 SotClipboardFormatId::STARMATH_50*/            { "application/x-openoffice-starmath-50;windows_formatname=\"StarMath 5.0\"", "StarMath 5.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 49 SotClipboardFormatId::STAROBJECT_PAINTDOC*/    { "application/x-openoffice-starobject-paintdoc;windows_formatname=\"StarObjectPaintDocument\"", "StarObjectPaintDocument", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 50 SotClipboardFormatId::FILLED_AREA*/            { "application/x-openoffice-filled-area;windows_formatname=\"FilledArea\"", "FilledArea", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 51 SotClipboardFormatId::HTML*/                   { "text/html", "HTML (HyperText Markup Language)", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 52 SotClipboardFormatId::HTML_SIMPLE*/            { "application/x-openoffice-html-simple;windows_formatname=\"HTML Format\"", "HTML Format", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 53 SotClipboardFormatId::CHAOS*/                  { "application/x-openoffice-chaos;windows_formatname=\"FORMAT_CHAOS\"", "FORMAT_CHAOS", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 54 SotClipboardFormatId::CNT_MSGATTACHFILE*/      { "application/x-openoffice-cnt-msgattachfile;windows_formatname=\"CNT_MSGATTACHFILE_FORMAT\"", "CNT_MSGATTACHFILE_FORMAT", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 55 SotClipboardFormatId::BIFF_5*/                 { "application/x-openoffice-biff5;windows_formatname=\"Biff5\"", "Biff5", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 56 SotClipboardFormatId::BIFF__5*/                { "application/x-openoffice-biff-5;windows_formatname=\"Biff 5\"", "Biff 5", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 57 SotClipboardFormatId::SYLK*/                   { "application/x-openoffice-sylk;windows_formatname=\"Sylk\"", "Sylk", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 58 SotClipboardFormatId::SYLK_BIGCAPS*/           { "application/x-openoffice-sylk-bigcaps;windows_formatname=\"SYLK\"", "SYLK", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 59 SotClipboardFormatId::LINK*/                   { "application/x-openoffice-link;windows_formatname=\"Link\"", "Link", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 60 SotClipboardFormatId::DIF*/                    { "application/x-openoffice-dif;windows_formatname=\"DIF\"", "DIF", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 61 SotClipboardFormatId::STARDRAW_TABBAR*/        { "application/x-openoffice-stardraw-tabbar;windows_formatname=\"StarDraw TabBar\"", "StarDraw TabBar", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 62 SotClipboardFormatId::SONLK*/                  { "application/x-openoffice-sonlk;windows_formatname=\"SONLK (StarOffice Navi Link)\"", "SONLK (StarOffice Navi Link)", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 63 SotClipboardFormatId::MSWORD_DOC*/             { "application/msword", "MSWordDoc", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 64 SotClipboardFormatId::STAR_FRAMESET_DOC*/      { "application/x-openoffice-star-frameset-doc;windows_formatname=\"StarFrameSetDocument\"", "StarFrameSetDocument", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 65 SotClipboardFormatId::OFFICE_DOC*/             { "application/x-openoffice-office-doc;windows_formatname=\"OfficeDocument\"", "OfficeDocument", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 66 SotClipboardFormatId::NOTES_DOCINFO*/          { "application/x-openoffice-notes-docinfo;windows_formatname=\"NotesDocInfo\"", "NotesDocInfo", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 67 SotClipboardFormatId::NOTES_HNOTE*/            { "application/x-openoffice-notes-hnote;windows_formatname=\"NoteshNote\"", "NoteshNote", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 68 SotClipboardFormatId::NOTES_NATIVE*/           { "application/x-openoffice-notes-native;windows_formatname=\"Native\"", "Native", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 69 SotClipboardFormatId::SFX_DOC*/                { "application/x-openoffice-sfx-doc;windows_formatname=\"SfxDocument\"", "SfxDocument", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 70 SotClipboardFormatId::EVDF*/                   { "application/x-openoffice-evdf;windows_formatname=\"EVDF (Explorer View Dummy Format)\"", "EVDF (Explorer View Dummy Format)", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 71 SotClipboardFormatId::ESDF*/                   { "application/x-openoffice-esdf;windows_formatname=\"ESDF (Explorer Search Dummy Format)\"", "ESDF (Explorer Search Dummy Format)", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 72 SotClipboardFormatId::IDF*/                    { "application/x-openoffice-idf;windows_formatname=\"IDF (Iconview Dummy Format)\"", "IDF (Iconview Dummy Format)", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 73 SotClipboardFormatId::EFTP*/                   { "application/x-openoffice-eftp;windows_formatname=\"EFTP (Explorer Ftp File)\"", "EFTP (Explorer Ftp File)", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 74 SotClipboardFormatId::EFD*/                    { "application/x-openoffice-efd;windows_formatname=\"EFD (Explorer Ftp Dir)\"", "EFD (Explorer Ftp Dir)", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 75 SotClipboardFormatId::SVX_FORMFIELDEXCH*/      { "application/x-openoffice-svx-formfieldexch;windows_formatname=\"SvxFormFieldExch\"", "SvxFormFieldExch", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 76 SotClipboardFormatId::EXTENDED_TABBAR*/        { "application/x-openoffice-extended-tabbar;windows_formatname=\"ExtendedTabBar\"", "ExtendedTabBar", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 77 SotClipboardFormatId::SBA_DATAEXCHANGE*/       { "application/x-openoffice-sba-dataexchange;windows_formatname=\"SBA-DATAFORMAT\"", "SBA-DATAFORMAT", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 78 SotClipboardFormatId::SBA_FIELDDATAEXCHANGE*/  { "application/x-openoffice-sba-fielddataexchange;windows_formatname=\"SBA-FIELDFORMAT\"", "SBA-FIELDFORMAT", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 79 SotClipboardFormatId::SBA_PRIVATE_URL*/        { "application/x-openoffice-sba-private-url;windows_formatname=\"SBA-PRIVATEURLFORMAT\"", "SBA-PRIVATEURLFORMAT", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 80 SotClipboardFormatId::SBA_TABED*/              { "application/x-openofficesba-tabed;windows_formatname=\"Tabed\"", "Tabed", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 81 SotClipboardFormatId::SBA_TABID*/              { "application/x-openoffice-sba-tabid;windows_formatname=\"Tabid\"", "Tabid", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 82 SotClipboardFormatId::SBA_JOIN*/               { "application/x-openoffice-sba-join;windows_formatname=\"SBA-JOINFORMAT\"", "SBA-JOINFORMAT", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 83 SotClipboardFormatId::OBJECTDESCRIPTOR*/       { "application/x-openoffice-objectdescriptor-xml;windows_formatname=\"Star Object Descriptor (XML)\"", "Star Object Descriptor (XML)", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 84 SotClipboardFormatId::LINKSRCDESCRIPTOR*/      { "application/x-openoffice-linksrcdescriptor-xml;windows_formatname=\"Star Link Source Descriptor (XML)\"", "Star Link Source Descriptor (XML)", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 85 SotClipboardFormatId::EMBED_SOURCE*/           { "application/x-openoffice-embed-source-xml;windows_formatname=\"Star Embed Source (XML)\"", "Star Embed Source (XML)", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 86 SotClipboardFormatId::LINK_SOURCE*/            { "application/x-openoffice-link-source-xml;windows_formatname=\"Star Link Source (XML)\"", "Star Link Source (XML)", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 87 SotClipboardFormatId::EMBEDDED_OBJ*/           { "application/x-openoffice-embedded-obj-xml;windows_formatname=\"Star Embedded Object (XML)\"", "Star Embedded Object (XML)", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 88 SotClipboardFormatId::FILECONTENT*/            { "application/x-openoffice-filecontent;windows_formatname=\"FileContents\"", "FileContents", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 89 SotClipboardFormatId::FILEGRPDESCRIPTOR*/      { "application/x-openoffice-filegrpdescriptor;windows_formatname=\"FileGroupDescriptor\"", "FileGroupDescriptor", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 90 SotClipboardFormatId::FILENAME*/               { "application/x-openoffice-filename;windows_formatname=\"FileName\"", "FileName", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 91 SotClipboardFormatId::SD_OLE*/                 { "application/x-openoffice-sd-ole;windows_formatname=\"SD-OLE\"", "SD-OLE", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 92 SotClipboardFormatId::EMBEDDED_OBJ_OLE*/       { "application/x-openoffice-embedded-obj-ole;windows_formatname=\"Embedded Object\"", "Embedded Object", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 93 SotClipboardFormatId::EMBED_SOURCE_OLE*/       { "application/x-openoffice-embed-source-ole;windows_formatname=\"Embed Source\"", "Embed Source", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 94 SotClipboardFormatId::OBJECTDESCRIPTOR_OLE*/   { "application/x-openoffice-objectdescriptor-ole;windows_formatname=\"Object Descriptor\"", "Object Descriptor", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 95 SotClipboardFormatId::LINKSRCDESCRIPTOR_OLE*/  { "application/x-openoffice-linkdescriptor-ole;windows_formatname=\"Link Source Descriptor\"", "Link Source Descriptor", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 96 SotClipboardFormatId::LINK_SOURCE_OLE*/        { "application/x-openoffice-link-source-ole;windows_formatname=\"Link Source\"", "Link Source", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 97 SotClipboardFormatId::SBA_CTRLDATAEXCHANGE*/   { "application/x-openoffice-sba-ctrldataexchange;windows_formatname=\"SBA-CTRLFORMAT\"", "SBA-CTRLFORMAT", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 98 SotClipboardFormatId::OUTPLACE_OBJ*/           { "application/x-openoffice-outplace-obj;windows_formatname=\"OutPlace Object\"", "OutPlace Object", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /* 99 SotClipboardFormatId::CNT_OWN_CLIP*/           { "application/x-openoffice-cnt-own-clip;windows_formatname=\"CntOwnClipboard\"", "CntOwnClipboard", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*100 SotClipboardFormatId::INET_IMAGE*/             { "application/x-openoffice-inet-image;windows_formatname=\"SO-INet-Image\"", "SO-INet-Image", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*101 SotClipboardFormatId::NETSCAPE_IMAGE*/         { "application/x-openoffice-netscape-image;windows_formatname=\"Netscape Image Format\"", "Netscape Image Format", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*102 SotClipboardFormatId::SBA_FORMEXCHANGE*/       { "application/x-openoffice-sba-formexchange;windows_formatname=\"SBA_FORMEXCHANGE\"", "SBA_FORMEXCHANGE", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*103 SotClipboardFormatId::SBA_REPORTEXCHANGE*/     { "application/x-openoffice-sba-reportexchange;windows_formatname=\"SBA_REPORTEXCHANGE\"", "SBA_REPORTEXCHANGE", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*104 SotClipboardFormatId::UNIFORMRESOURCELOCATOR*/ { "application/x-openoffice-uniformresourcelocator;windows_formatname=\"UniformResourceLocator\"", "UniformResourceLocator", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*105 SotClipboardFormatId::STARCHARTDOCUMENT_50*/   { "application/x-openoffice-starchartdocument-50;windows_formatname=\"StarChartDocument 5.0\"", "StarChartDocument 5.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*106 SotClipboardFormatId::GRAPHOBJ*/               { "application/x-openoffice-graphobj;windows_formatname=\"Graphic Object\"", "Graphic Object", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*107 SotClipboardFormatId::STARWRITER_60*/          { MIMETYPE_VND_SUN_XML_WRITER_ASCII, "Writer 6.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*108 SotClipboardFormatId::STARWRITERWEB_60*/       { MIMETYPE_VND_SUN_XML_WRITER_WEB_ASCII, "Writer/Web 6.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*109 SotClipboardFormatId::STARWRITERGLOB_60*/      { MIMETYPE_VND_SUN_XML_WRITER_GLOBAL_ASCII, "Writer/Global 6.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*110 SotClipboardFormatId::STARWDRAW_60*/           { MIMETYPE_VND_SUN_XML_DRAW_ASCII, "Draw 6.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*111 SotClipboardFormatId::STARIMPRESS_60*/         { MIMETYPE_VND_SUN_XML_IMPRESS_ASCII, "Impress 6.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*112 SotClipboardFormatId::STARCALC_60*/            { MIMETYPE_VND_SUN_XML_CALC_ASCII, "Calc 6.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*113 SotClipboardFormatId::STARCHART_60*/           { MIMETYPE_VND_SUN_XML_CHART_ASCII, "Chart 6.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*114 SotClipboardFormatId::STARMATH_60*/            { MIMETYPE_VND_SUN_XML_MATH_ASCII, "Math 6.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*115 SotClipboardFormatId::WMF*/                    { "application/x-openoffice-wmf;windows_formatname=\"Image WMF\"", "Windows MetaFile", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*116 SotClipboardFormatId::DBACCESS_QUERY*/         { "application/x-openoffice-dbaccess-query;windows_formatname=\"Data source Query Object\"", "Data source Query Object", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*117 SotClipboardFormatId::DBACCESS_TABLE*/         { "application/x-openoffice-dbaccess-table;windows_formatname=\"Data source Table\"", "Data source Table", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*118 SotClipboardFormatId::DBACCESS_COMMAND*/       { "application/x-openoffice-dbaccess-command;windows_formatname=\"SQL query\"", "SQL query", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*119 SotClipboardFormatId::DIALOG_60*/              { "application/vnd.sun.xml.dialog", "Dialog 6.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*120 SotClipboardFormatId::EMF*/                    { "application/x-openoffice-emf;windows_formatname=\"Image EMF\"", "Windows Enhanced MetaFile", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*121 SotClipboardFormatId::BIFF_8*/                 { "application/x-openoffice-biff-8;windows_formatname=\"Biff8\"", "Biff8", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*122 SotClipboardFormatId::BMP*/                    { "image/bmp", "Windows Bitmap", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*123 SotClipboardFormatId::HTML_NO_COMMENT */       { "application/x-openoffice-html-no-comment;windows_formatname=\"HTML Format\"", "HTML (no comment)", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*124 SotClipboardFormatId::STARWRITER_8*/          { MIMETYPE_OASIS_OPENDOCUMENT_TEXT_ASCII, "Writer 8", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*125 SotClipboardFormatId::STARWRITERWEB_8*/       { MIMETYPE_OASIS_OPENDOCUMENT_TEXT_WEB_ASCII, "Writer/Web 8", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*126 SotClipboardFormatId::STARWRITERGLOB_8*/      { MIMETYPE_OASIS_OPENDOCUMENT_TEXT_GLOBAL_ASCII, "Writer/Global 8", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*127 SotClipboardFormatId::STARWDRAW_8*/           { MIMETYPE_OASIS_OPENDOCUMENT_DRAWING_ASCII, "Draw 8", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*128 SotClipboardFormatId::STARIMPRESS_8*/         { MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION_ASCII, "Impress 8", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*129 SotClipboardFormatId::STARCALC_8*/            { MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET_ASCII, "Calc 8", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*130 SotClipboardFormatId::STARCHART_8*/           { MIMETYPE_OASIS_OPENDOCUMENT_CHART_ASCII, "Chart 8", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*131 SotClipboardFormatId::STARMATH_8*/            { MIMETYPE_OASIS_OPENDOCUMENT_FORMULA_ASCII, "Math 8", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*132 SotClipboardFormatId::XFORMS */               { "application/x-openoffice-xforms;windows_formatname=\"??? Format\"", "???", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*133 SotClipboardFormatId::STARWRITER_8_TEMPLATE*/          { MIMETYPE_OASIS_OPENDOCUMENT_TEXT_TEMPLATE_ASCII, "Writer 8 Template", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*134 SotClipboardFormatId::STARWDRAW_8_TEMPLATE*/           { MIMETYPE_OASIS_OPENDOCUMENT_DRAWING_TEMPLATE_ASCII, "Draw 8 Template", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*135 SotClipboardFormatId::STARIMPRESS_8_TEMPLATE*/         { MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION_TEMPLATE_ASCII, "Impress 8 Template", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*136 SotClipboardFormatId::STARCALC_8_TEMPLATE*/            { MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET_TEMPLATE_ASCII, "Calc 8 Template", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*137 SotClipboardFormatId::STARCHART_8_TEMPLATE*/           { MIMETYPE_OASIS_OPENDOCUMENT_CHART_TEMPLATE_ASCII, "Chart 8 Template", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*138 SotClipboardFormatId::STARMATH_8_TEMPLATE*/            { MIMETYPE_OASIS_OPENDOCUMENT_FORMULA_TEMPLATE_ASCII, "Math 8 Template", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*139 SotClipboardFormatId::STARBASE_8*/             { MIMETYPE_OASIS_OPENDOCUMENT_DATABASE_ASCII, "StarBase 8", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*140 SotClipboardFormatId::HC_GDIMETAFILE*/         { "application/x-openoffice-highcontrast-gdimetafile;windows_formatname=\"GDIMetaFile\"", "High Contrast GDIMetaFile", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*141 SotClipboardFormatId::PNG*/                    { "image/png", "PNG Bitmap", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*142 SotClipboardFormatId::STARWRITERGLOB_8_TEMPLATE*/      { MIMETYPE_OASIS_OPENDOCUMENT_TEXT_GLOBAL_TEMPLATE_ASCII, "Writer/Global 8 Template", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*143 SotClipboardFormatId::MATHML*/   { "application/mathml+xml", "MathML", &::cppu::UnoType<const Sequence< sal_Int8 >>::get() },
            /*144 SotClipboardFormatId::JPEG*/ { "image/jpeg", "JPEG Bitmap", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*145 SotClipboardFormatId::RICHTEXT*/ { "text/richtext", "Richtext Format", &cppu::UnoType<Sequence<sal_Int8>>::get() },
            /*146 SotClipboardFormatId::STRING_TSVC*/            { "application/x-libreoffice-tsvc", "Text TSV-Calc", &cppu::UnoType<OUString>::get() }
            };
            return &aInstance[0];
        }
    };

    struct FormatArray_Impl
        : public rtl::StaticAggregate<
            const DataFlavorRepresentation, ImplFormatArray_Impl > {};


    typedef std::vector<std::unique_ptr<css::datatransfer::DataFlavor>> tDataFlavorList;

    struct SotData_Impl
    {
        std::unique_ptr<tDataFlavorList> pDataFlavorList;
    };

    struct ImplData : public rtl::Static<SotData_Impl, ImplData> {};
}

static tDataFlavorList& InitFormats_Impl()
{
    SotData_Impl *pSotData = &ImplData::get();
    if( !pSotData->pDataFlavorList )
        pSotData->pDataFlavorList.reset(new tDataFlavorList);
    return *pSotData->pDataFlavorList;
}

/*************************************************************************
|*
|*    SotExchange::RegisterFormatName()
|*
|*    Description       CLIP.SDW
*************************************************************************/
SotClipboardFormatId SotExchange::RegisterFormatName( const OUString& rName )
{
    const DataFlavorRepresentation *pFormatArray_Impl = FormatArray_Impl::get();
    // test the default first - name
    for( SotClipboardFormatId i = SotClipboardFormatId::STRING; i <= SotClipboardFormatId::FILE_LIST;  ++i )
        if( rName.equalsAscii( pFormatArray_Impl[ static_cast<int>(i) ].pName ) )
            return i;

    // BM: the chart format 105 ("StarChartDocument 5.0") was written
    // only into 5.1 chart documents - in 5.0 and 5.2 it was 42 ("StarChart 5.0")
    // The registry only contains the entry for the 42 format id.
    for( SotClipboardFormatId i = SotClipboardFormatId::RTF; i <= SotClipboardFormatId::USER_END;  ++i )
        if( rName.equalsAscii( pFormatArray_Impl[ static_cast<int>(i) ].pName ) )
            return ( (i == SotClipboardFormatId::STARCHARTDOCUMENT_50)
                     ? SotClipboardFormatId::STARCHART_50
                     : i );

    // then in the dynamic list
    tDataFlavorList& rL = InitFormats_Impl();
    for( tDataFlavorList::size_type i = 0; i < rL.size(); i++ )
    {
        auto const& pFlavor = rL[ i ];
        if( pFlavor && rName == pFlavor->HumanPresentableName )
            return static_cast<SotClipboardFormatId>(i + static_cast<int>(SotClipboardFormatId::USER_END) + 1);
    }

    std::unique_ptr<DataFlavor> pNewFlavor(new DataFlavor);
    pNewFlavor->MimeType = rName;
    pNewFlavor->HumanPresentableName = rName;
    pNewFlavor->DataType = cppu::UnoType<OUString>::get();

    rL.push_back( std::move(pNewFlavor) );

    return static_cast<SotClipboardFormatId>(static_cast<int>(rL.size()-1) + static_cast<int>(SotClipboardFormatId::USER_END) + 1);
}

SotClipboardFormatId SotExchange::RegisterFormatMimeType( const OUString& rMimeType )
{
    const DataFlavorRepresentation *pFormatArray_Impl = FormatArray_Impl::get();
    // test the default first - name
    for( SotClipboardFormatId i = SotClipboardFormatId::STRING; i <= SotClipboardFormatId::FILE_LIST;  ++i )
        if( rMimeType.equalsAscii( pFormatArray_Impl[ static_cast<int>(i) ].pMimeType ) )
            return i;

    for( SotClipboardFormatId i = SotClipboardFormatId::RTF; i <= SotClipboardFormatId::USER_END;  ++i )
        if( rMimeType.equalsAscii( pFormatArray_Impl[ static_cast<int>(i) ].pMimeType ) )
            return i;

    // then in the dynamic list
    tDataFlavorList& rL = InitFormats_Impl();
    for( tDataFlavorList::size_type i = 0; i < rL.size(); i++ )
    {
        auto const& pFlavor = rL[ i ];
        if( pFlavor && rMimeType == pFlavor->MimeType )
            return static_cast<SotClipboardFormatId>(i + static_cast<int>(SotClipboardFormatId::USER_END) + 1);
    }

    std::unique_ptr<DataFlavor> pNewFlavor(new DataFlavor);
    pNewFlavor->MimeType = rMimeType;
    pNewFlavor->HumanPresentableName = rMimeType;
    pNewFlavor->DataType = cppu::UnoType<OUString>::get();

    rL.push_back( std::move(pNewFlavor) );

    return static_cast<SotClipboardFormatId>(static_cast<int>(rL.size()-1) + static_cast<int>(SotClipboardFormatId::USER_END) + 1);
}

/*************************************************************************
|*
|*    SotExchange::RegisterFormatName()
|*
|*    Description       CLIP.SDW
*************************************************************************/
SotClipboardFormatId SotExchange::RegisterFormat( const DataFlavor& rFlavor )
{
    SotClipboardFormatId nRet = GetFormat( rFlavor );

    if( nRet == SotClipboardFormatId::NONE )
    {
        tDataFlavorList& rL = InitFormats_Impl();
        nRet = static_cast<SotClipboardFormatId>(rL.size() + static_cast<int>(SotClipboardFormatId::USER_END) + 1);
        rL.emplace_back( new DataFlavor( rFlavor ) );
    }

    return nRet;
}

/*************************************************************************
|*
|*    SotExchange::GetFormatDataFlavor()
|*
*************************************************************************/

bool SotExchange::GetFormatDataFlavor( SotClipboardFormatId nFormat, DataFlavor& rFlavor )
{
    bool bRet;

    if( SotClipboardFormatId::USER_END >= nFormat )
    {
        const DataFlavorRepresentation& rData = FormatArray_Impl::get()[static_cast<int>(nFormat)];
        rFlavor.MimeType = OUString::createFromAscii( rData.pMimeType );
        rFlavor.HumanPresentableName = OUString::createFromAscii( rData.pName );
        rFlavor.DataType = *rData.pType;

        bRet = true;
    }
    else
    {
        tDataFlavorList& rL = InitFormats_Impl();

        unsigned i = static_cast<int>(nFormat) - static_cast<int>(SotClipboardFormatId::USER_END) - 1;

        if( rL.size() > i )
        {
            rFlavor = *rL[ i ];
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

/*************************************************************************
|*
|*    SotExchange::GetFormatMimeType( sal_uLong nFormat )
|*
*************************************************************************/

OUString SotExchange::GetFormatMimeType( SotClipboardFormatId nFormat )
{
    OUString sMimeType;
    if( SotClipboardFormatId::USER_END >= nFormat )
        sMimeType = OUString::createFromAscii( FormatArray_Impl::get()[static_cast<int>(nFormat)].pMimeType );
    else
    {
        tDataFlavorList& rL = InitFormats_Impl();

        unsigned i = static_cast<int>(nFormat) - static_cast<int>(SotClipboardFormatId::USER_END) - 1;

        if( rL.size() > i )
            sMimeType = rL[ i ]->MimeType;
    }

    DBG_ASSERT( !sMimeType.isEmpty(), "SotExchange::GetFormatMimeType(): DataFlavor not initialized" );

    return sMimeType;
}

/*************************************************************************
|*
|*    SotExchange::GetFormatIdFromMimeType( const String& rMimeType )
|*
*************************************************************************/

SotClipboardFormatId SotExchange::GetFormatIdFromMimeType( const OUString& rMimeType )
{
    const DataFlavorRepresentation *pFormatArray_Impl = FormatArray_Impl::get();
    for( SotClipboardFormatId i = SotClipboardFormatId::STRING; i <= SotClipboardFormatId::FILE_LIST;  ++i )
        if( rMimeType.equalsAscii( pFormatArray_Impl[ static_cast<int>(i) ].pMimeType ) )
            return i;

    // BM: the chart format 105 ("StarChartDocument 5.0") was written
    // only into 5.1 chart documents - in 5.0 and 5.2 it was 42 ("StarChart 5.0")
    // The registry only contains the entry for the 42 format id.
    for( SotClipboardFormatId i = SotClipboardFormatId::RTF; i <= SotClipboardFormatId::USER_END;  ++i )
        if( rMimeType.equalsAscii( pFormatArray_Impl[ static_cast<int>(i) ].pMimeType ) )
            return ( (i == SotClipboardFormatId::STARCHARTDOCUMENT_50)
                     ? SotClipboardFormatId::STARCHART_50
                     : i );

    // then in the dynamic list
    tDataFlavorList& rL = InitFormats_Impl();

    for( tDataFlavorList::size_type i = 0; i < rL.size(); i++ )
    {
        auto const& pFlavor = rL[ i ];
        if( pFlavor && rMimeType == pFlavor->MimeType )
            return static_cast<SotClipboardFormatId>(i + static_cast<int>(SotClipboardFormatId::USER_END) + 1);
    }

    return SotClipboardFormatId::NONE;
}

/*************************************************************************
|*
|*    SotExchange::GetFormatName()
|*
|*    Description       CLIP.SDW
*************************************************************************/
SotClipboardFormatId SotExchange::GetFormat( const DataFlavor& rFlavor )
{
    // test the default first - name
    const OUString& rMimeType = rFlavor.MimeType;

    const DataFlavorRepresentation *pFormatArray_Impl = FormatArray_Impl::get();
    for( SotClipboardFormatId i = SotClipboardFormatId::STRING; i <= SotClipboardFormatId::FILE_LIST;  ++i )
        if( rMimeType.equalsAscii( pFormatArray_Impl[ static_cast<int>(i) ].pMimeType ) )
            return i;

    // BM: the chart format 105 ("StarChartDocument 5.0") was written
    // only into 5.1 chart documents - in 5.0 and 5.2 it was 42 ("StarChart 5.0")
    // The registry only contains the entry for the 42 format id.
    for( SotClipboardFormatId i = SotClipboardFormatId::RTF; i <= SotClipboardFormatId::USER_END;  ++i )
        if( rMimeType.equalsAscii( pFormatArray_Impl[ static_cast<int>(i) ].pMimeType ) )
            return ( (i == SotClipboardFormatId::STARCHARTDOCUMENT_50)
                     ? SotClipboardFormatId::STARCHART_50
                     : i );

    // then in the dynamic list
    tDataFlavorList& rL = InitFormats_Impl();
    for( tDataFlavorList::size_type i = 0; i < rL.size(); i++ )
    {
        auto const& pFlavor = rL[ i ];
        if( pFlavor && rMimeType == pFlavor->MimeType )
            return static_cast<SotClipboardFormatId>(i + static_cast<int>(SotClipboardFormatId::USER_END) + 1);
    }

    return SotClipboardFormatId::NONE;
}

/*************************************************************************
|*
|*    SotExchange::GetFormatName()
|*
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
