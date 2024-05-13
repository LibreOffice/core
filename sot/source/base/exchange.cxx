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
#include <sot/exchange.hxx>
#include <sot/formats.hxx>
#include <comphelper/classids.hxx>
#include <com/sun/star/datatransfer/DataFlavor.hpp>
#include <comphelper/documentconstants.hxx>

#include <memory>
#include <vector>

using namespace::com::sun::star::uno;
using namespace::com::sun::star::datatransfer;

namespace {

/*
 *  These tables contain all MimeTypes, format identifiers, and types used in
 *  the Office. The table is sorted by the format string ID, and each ID is
 *  exactly 1 greater than its predecessor ID, so that the ID can be used as a
 *  table index.
 */
struct DataFlavorRepresentation
{
    OUString               pMimeType;
    const char*            pName;
    const css::uno::Type*  pType;
};

const DataFlavorRepresentation* FormatArray_Impl()
{
    static const DataFlavorRepresentation aInstance[] =
    {
        /*  0 SOT_FORMAT_SYSTEM_START*/                 { u""_ustr, "", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*  1 SotClipboardFormatId::STRING*/                       { u"text/plain;charset=utf-16"_ustr, "Text", &cppu::UnoType<OUString>::get()},
        /*  2 SotClipboardFormatId::BITMAP*/                       { u"application/x-openoffice-bitmap;windows_formatname=\"Bitmap\""_ustr, "Bitmap", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*  3 SotClipboardFormatId::GDIMETAFILE*/                  { u"application/x-openoffice-gdimetafile;windows_formatname=\"GDIMetaFile\""_ustr, "GDIMetaFile", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*  4 SotClipboardFormatId::PRIVATE*/                      { u"application/x-openoffice-private;windows_formatname=\"Private\""_ustr, "Private", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*  5 SotClipboardFormatId::SIMPLE_FILE*/                         { u"application/x-openoffice-file;windows_formatname=\"FileNameW\""_ustr, "FileName", &cppu::UnoType<OUString>::get() },
        /*  6 SotClipboardFormatId::FILE_LIST*/                    { u"application/x-openoffice-filelist;windows_formatname=\"FileList\""_ustr, "FileList", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*  7 EMPTY*/                                   { u""_ustr, "", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*  8 EMPTY*/                                   { u""_ustr, "", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*  9 EMPTY*/                                   { u""_ustr, "", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 10 SotClipboardFormatId::RTF*/                          { u"text/rtf"_ustr, "Rich Text Format", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 11 SotClipboardFormatId::DRAWING*/                { u"application/x-openoffice-drawing;windows_formatname=\"Drawing Format\""_ustr, "Drawing Format", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 12 SotClipboardFormatId::SVXB*/                   { u"application/x-openoffice-svxb;windows_formatname=\"SVXB (StarView Bitmap/Animation)\""_ustr, "SVXB (StarView Bitmap/Animation)", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 13 SotClipboardFormatId::SVIM*/                   { u"application/x-openoffice-svim;windows_formatname=\"SVIM (StarView ImageMap)\""_ustr, "SVIM (StarView ImageMap)", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 14 SotClipboardFormatId::XFA*/                    { u"application/x-libreoffice-xfa;windows_formatname=\"XFA (XOutDev FillAttr Any)\""_ustr, "XFA (XOutDev FillAttr Any)", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 15 SotClipboardFormatId::EDITENGINE_ODF_TEXT_FLAT*/ { u"application/vnd.oasis.opendocument.text-flat-xml"_ustr, "Flat XML format (EditEngine ODF)", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 16 SotClipboardFormatId::INTERNALLINK_STATE*/     { u"application/x-openoffice-internallink-state;windows_formatname=\"StatusInfo vom SvxInternalLink\""_ustr, "StatusInfo vom SvxInternalLink", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 17 SotClipboardFormatId::SOLK*/                   { u"application/x-openoffice-solk;windows_formatname=\"SOLK (StarOffice Link)\""_ustr, "SOLK (StarOffice Link)", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 18 SotClipboardFormatId::NETSCAPE_BOOKMARK*/      { u"application/x-openoffice-netscape-bookmark;windows_formatname=\"Netscape Bookmark\""_ustr, "Netscape Bookmark", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 19 SotClipboardFormatId::TREELISTBOX*/            { u"application/x-openoffice-treelistbox;windows_formatname=\"SV_LBOX_DD_FORMAT\""_ustr, "SV_LBOX_DD_FORMAT", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 20 SotClipboardFormatId::NATIVE*/                 { u"application/x-openoffice-native;windows_formatname=\"Native\""_ustr, "Native", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 21 SotClipboardFormatId::OWNERLINK*/              { u"application/x-openoffice-ownerlink;windows_formatname=\"OwnerLink\""_ustr, "OwnerLink", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 22 SotClipboardFormatId::STARSERVER*/             { u"application/x-openoffice-starserver;windows_formatname=\"StarServerFormat\""_ustr, "StarServerFormat", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 23 SotClipboardFormatId::STAROBJECT*/             { u"application/x-openoffice-starobject;windows_formatname=\"StarObjectFormat\""_ustr, "StarObjectFormat", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 24 SotClipboardFormatId::APPLETOBJECT*/           { u"application/x-openoffice-appletobject;windows_formatname=\"Applet Object\""_ustr, "Applet Object", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 25 SotClipboardFormatId::PLUGIN_OBJECT*/          { u"application/x-openoffice-plugin-object;windows_formatname=\"PlugIn Object\""_ustr, "PlugIn Object", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 26 SotClipboardFormatId::STARWRITER_30*/          { u"application/x-openoffice-starwriter-30;windows_formatname=\"StarWriter 3.0\""_ustr, "StarWriter 3.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 27 SotClipboardFormatId::STARWRITER_40*/          { u"application/x-openoffice-starwriter-40;windows_formatname=\"StarWriter 4.0\""_ustr, "StarWriter 4.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 28 SotClipboardFormatId::STARWRITER_50*/          { u"application/x-openoffice-starwriter-50;windows_formatname=\"StarWriter 5.0\""_ustr, "StarWriter 5.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 29 SotClipboardFormatId::STARWRITERWEB_40*/       { u"application/x-openoffice-starwriterweb-40;windows_formatname=\"StarWriter/Web 4.0\""_ustr, "StarWriter/Web 4.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 30 SotClipboardFormatId::STARWRITERWEB_50*/       { u"application/x-openoffice-starwriterweb-50;windows_formatname=\"StarWriter/Web 5.0\""_ustr, "StarWriter/Web 5.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 31 SotClipboardFormatId::STARWRITERGLOB_40*/      { u"application/x-openoffice-starwriterglob-40;windows_formatname=\"StarWriter/Global 4.0\""_ustr, "StarWriter/Global 4.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 32 SotClipboardFormatId::STARWRITERGLOB_50*/      { u"application/x-openoffice-starwriterglob-50;windows_formatname=\"StarWriter/Global 5.0\""_ustr, "StarWriter/Global 5.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 33 SotClipboardFormatId::STARDRAW*/               { u"application/x-openoffice-stardraw;windows_formatname=\"StarDrawDocument\""_ustr, "StarDrawDocument", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 34 SotClipboardFormatId::STARDRAW_40*/            { u"application/x-openoffice-stardraw-40;windows_formatname=\"StarDrawDocument 4.0\""_ustr, "StarDrawDocument 4.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 35 SotClipboardFormatId::STARIMPRESS_50*/         { u"application/x-openoffice-starimpress-50;windows_formatname=\"StarImpress 5.0\""_ustr, "StarImpress 5.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 36 SotClipboardFormatId::STARDRAW_50*/            { u"application/x-openoffice-stardraw-50;windows_formatname=\"StarDraw 5.0\""_ustr, "StarDraw 5.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 37 SotClipboardFormatId::STARCALC*/               { u"application/x-openoffice-starcalc;windows_formatname=\"StarCalcDocument\""_ustr, "StarCalcDocument", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 38 SotClipboardFormatId::STARCALC_40*/            { u"application/x-openoffice-starcalc-40;windows_formatname=\"StarCalc 4.0\""_ustr, "StarCalc 4.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 39 SotClipboardFormatId::STARCALC_50*/            { u"application/x-openoffice-starcalc-50;windows_formatname=\"StarCalc 5.0\""_ustr, "StarCalc 5.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 40 SotClipboardFormatId::STARCHART*/              { u"application/x-openoffice-starchart;windows_formatname=\"StarChartDocument\""_ustr, "StarChartDocument", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 41 SotClipboardFormatId::STARCHART_40*/           { u"application/x-openoffice-starchart-40;windows_formatname=\"StarChartDocument 4.0\""_ustr, "StarChartDocument 4.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 42 SotClipboardFormatId::STARCHART_50*/           { u"application/x-openoffice-starchart-50;windows_formatname=\"StarChart 5.0\""_ustr, "StarChart 5.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 43 SotClipboardFormatId::STARIMAGE*/              { u"application/x-openoffice-starimage;windows_formatname=\"StarImageDocument\""_ustr, "StarImageDocument", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 44 SotClipboardFormatId::STARIMAGE_40*/           { u"application/x-openoffice-starimage-40;windows_formatname=\"StarImageDocument 4.0\""_ustr, "StarImageDocument 4.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 45 SotClipboardFormatId::STARIMAGE_50*/           { u"application/x-openoffice-starimage-50;windows_formatname=\"StarImage 5.0\""_ustr, "StarImage 5.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 46 SotClipboardFormatId::STARMATH*/               { u"application/x-openoffice-starmath;windows_formatname=\"StarMath\""_ustr, "StarMath", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 47 SotClipboardFormatId::STARMATH_40*/            { u"application/x-openoffice-starmath-40;windows_formatname=\"StarMathDocument 4.0\""_ustr, "StarMathDocument 4.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 48 SotClipboardFormatId::STARMATH_50*/            { u"application/x-openoffice-starmath-50;windows_formatname=\"StarMath 5.0\""_ustr, "StarMath 5.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 49 SotClipboardFormatId::STAROBJECT_PAINTDOC*/    { u"application/x-openoffice-starobject-paintdoc;windows_formatname=\"StarObjectPaintDocument\""_ustr, "StarObjectPaintDocument", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 50 SotClipboardFormatId::FILLED_AREA*/            { u"application/x-openoffice-filled-area;windows_formatname=\"FilledArea\""_ustr, "FilledArea", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 51 SotClipboardFormatId::HTML*/                   { u"text/html"_ustr, "HTML (HyperText Markup Language)", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 52 SotClipboardFormatId::HTML_SIMPLE*/            { u"application/x-openoffice-html-simple;windows_formatname=\"HTML Format\""_ustr, "HTML Format", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 53 SotClipboardFormatId::CHAOS*/                  { u"application/x-openoffice-chaos;windows_formatname=\"FORMAT_CHAOS\""_ustr, "FORMAT_CHAOS", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 54 SotClipboardFormatId::CNT_MSGATTACHFILE*/      { u"application/x-openoffice-cnt-msgattachfile;windows_formatname=\"CNT_MSGATTACHFILE_FORMAT\""_ustr, "CNT_MSGATTACHFILE_FORMAT", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 55 SotClipboardFormatId::BIFF_5*/                 { u"application/x-openoffice-biff5;windows_formatname=\"Biff5\""_ustr, "Biff5", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 56 SotClipboardFormatId::BIFF__5*/                { u"application/x-openoffice-biff-5;windows_formatname=\"Biff 5\""_ustr, "Biff 5", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 57 SotClipboardFormatId::SYLK*/                   { u"application/x-openoffice-sylk;windows_formatname=\"Sylk\""_ustr, "Sylk", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 58 SotClipboardFormatId::SYLK_BIGCAPS*/           { u"application/x-openoffice-sylk-bigcaps;windows_formatname=\"SYLK\""_ustr, "SYLK", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 59 SotClipboardFormatId::LINK*/                   { u"application/x-openoffice-link;windows_formatname=\"Link\""_ustr, "Link", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 60 SotClipboardFormatId::DIF*/                    { u"application/x-openoffice-dif;windows_formatname=\"DIF\""_ustr, "DIF", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 61 SotClipboardFormatId::STARDRAW_TABBAR*/        { u"application/x-openoffice-stardraw-tabbar;windows_formatname=\"StarDraw TabBar\""_ustr, "StarDraw TabBar", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 62 SotClipboardFormatId::SONLK*/                  { u"application/x-openoffice-sonlk;windows_formatname=\"SONLK (StarOffice Navi Link)\""_ustr, "SONLK (StarOffice Navi Link)", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 63 SotClipboardFormatId::MSWORD_DOC*/             { u"application/msword"_ustr, "MSWordDoc", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 64 SotClipboardFormatId::STAR_FRAMESET_DOC*/      { u"application/x-openoffice-star-frameset-doc;windows_formatname=\"StarFrameSetDocument\""_ustr, "StarFrameSetDocument", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 65 SotClipboardFormatId::OFFICE_DOC*/             { u"application/x-openoffice-office-doc;windows_formatname=\"OfficeDocument\""_ustr, "OfficeDocument", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 66 SotClipboardFormatId::NOTES_DOCINFO*/          { u"application/x-openoffice-notes-docinfo;windows_formatname=\"NotesDocInfo\""_ustr, "NotesDocInfo", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 67 SotClipboardFormatId::NOTES_HNOTE*/            { u"application/x-openoffice-notes-hnote;windows_formatname=\"NoteshNote\""_ustr, "NoteshNote", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 68 SotClipboardFormatId::NOTES_NATIVE*/           { u"application/x-openoffice-notes-native;windows_formatname=\"Native\""_ustr, "Native", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 69 SotClipboardFormatId::SFX_DOC*/                { u"application/x-openoffice-sfx-doc;windows_formatname=\"SfxDocument\""_ustr, "SfxDocument", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 70 SotClipboardFormatId::EVDF*/                   { u"application/x-openoffice-evdf;windows_formatname=\"EVDF (Explorer View Dummy Format)\""_ustr, "EVDF (Explorer View Dummy Format)", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 71 SotClipboardFormatId::ESDF*/                   { u"application/x-openoffice-esdf;windows_formatname=\"ESDF (Explorer Search Dummy Format)\""_ustr, "ESDF (Explorer Search Dummy Format)", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 72 SotClipboardFormatId::IDF*/                    { u"application/x-openoffice-idf;windows_formatname=\"IDF (Iconview Dummy Format)\""_ustr, "IDF (Iconview Dummy Format)", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 73 SotClipboardFormatId::EFTP*/                   { u"application/x-openoffice-eftp;windows_formatname=\"EFTP (Explorer Ftp File)\""_ustr, "EFTP (Explorer Ftp File)", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 74 SotClipboardFormatId::EFD*/                    { u"application/x-openoffice-efd;windows_formatname=\"EFD (Explorer Ftp Dir)\""_ustr, "EFD (Explorer Ftp Dir)", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 75 SotClipboardFormatId::SVX_FORMFIELDEXCH*/      { u"application/x-openoffice-svx-formfieldexch;windows_formatname=\"SvxFormFieldExch\""_ustr, "SvxFormFieldExch", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 76 SotClipboardFormatId::EXTENDED_TABBAR*/        { u"application/x-openoffice-extended-tabbar;windows_formatname=\"ExtendedTabBar\""_ustr, "ExtendedTabBar", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 77 SotClipboardFormatId::SBA_DATAEXCHANGE*/       { u"application/x-openoffice-sba-dataexchange;windows_formatname=\"SBA-DATAFORMAT\""_ustr, "SBA-DATAFORMAT", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 78 SotClipboardFormatId::SBA_FIELDDATAEXCHANGE*/  { u"application/x-openoffice-sba-fielddataexchange;windows_formatname=\"SBA-FIELDFORMAT\""_ustr, "SBA-FIELDFORMAT", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 79 SotClipboardFormatId::SBA_PRIVATE_URL*/        { u"application/x-openoffice-sba-private-url;windows_formatname=\"SBA-PRIVATEURLFORMAT\""_ustr, "SBA-PRIVATEURLFORMAT", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 80 SotClipboardFormatId::SBA_TABED*/              { u"application/x-openofficesba-tabed;windows_formatname=\"Tabed\""_ustr, "Tabed", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 81 SotClipboardFormatId::SBA_TABID*/              { u"application/x-openoffice-sba-tabid;windows_formatname=\"Tabid\""_ustr, "Tabid", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 82 SotClipboardFormatId::SBA_JOIN*/               { u"application/x-openoffice-sba-join;windows_formatname=\"SBA-JOINFORMAT\""_ustr, "SBA-JOINFORMAT", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 83 SotClipboardFormatId::OBJECTDESCRIPTOR*/       { u"application/x-openoffice-objectdescriptor-xml;windows_formatname=\"Star Object Descriptor (XML)\""_ustr, "Star Object Descriptor (XML)", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 84 SotClipboardFormatId::LINKSRCDESCRIPTOR*/      { u"application/x-openoffice-linksrcdescriptor-xml;windows_formatname=\"Star Link Source Descriptor (XML)\""_ustr, "Star Link Source Descriptor (XML)", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 85 SotClipboardFormatId::EMBED_SOURCE*/           { u"application/x-openoffice-embed-source-xml;windows_formatname=\"Star Embed Source (XML)\""_ustr, "Star Embed Source (XML)", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 86 SotClipboardFormatId::LINK_SOURCE*/            { u"application/x-openoffice-link-source-xml;windows_formatname=\"Star Link Source (XML)\""_ustr, "Star Link Source (XML)", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 87 SotClipboardFormatId::EMBEDDED_OBJ*/           { u"application/x-openoffice-embedded-obj-xml;windows_formatname=\"Star Embedded Object (XML)\""_ustr, "Star Embedded Object (XML)", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 88 SotClipboardFormatId::FILECONTENT*/            { u"application/x-openoffice-filecontent;windows_formatname=\"FileContents\""_ustr, "FileContents", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 89 SotClipboardFormatId::FILEGRPDESCRIPTOR*/      { u"application/x-openoffice-filegrpdescriptor;windows_formatname=\"FileGroupDescriptorW\""_ustr, "FileGroupDescriptor", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 90 SotClipboardFormatId::FILENAME*/               { u"application/x-openoffice-filename;windows_formatname=\"FileNameW\""_ustr, "FileName", &cppu::UnoType<OUString>::get() },
        /* 91 SotClipboardFormatId::SD_OLE*/                 { u"application/x-openoffice-sd-ole;windows_formatname=\"SD-OLE\""_ustr, "SD-OLE", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 92 SotClipboardFormatId::EMBEDDED_OBJ_OLE*/       { u"application/x-openoffice-embedded-obj-ole;windows_formatname=\"Embedded Object\""_ustr, "Embedded Object", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 93 SotClipboardFormatId::EMBED_SOURCE_OLE*/       { u"application/x-openoffice-embed-source-ole;windows_formatname=\"Embed Source\""_ustr, "Embed Source", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 94 SotClipboardFormatId::OBJECTDESCRIPTOR_OLE*/   { u"application/x-openoffice-objectdescriptor-ole;windows_formatname=\"Object Descriptor\""_ustr, "Object Descriptor", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 95 SotClipboardFormatId::LINKSRCDESCRIPTOR_OLE*/  { u"application/x-openoffice-linkdescriptor-ole;windows_formatname=\"Link Source Descriptor\""_ustr, "Link Source Descriptor", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 96 SotClipboardFormatId::LINK_SOURCE_OLE*/        { u"application/x-openoffice-link-source-ole;windows_formatname=\"Link Source\""_ustr, "Link Source", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 97 SotClipboardFormatId::SBA_CTRLDATAEXCHANGE*/   { u"application/x-openoffice-sba-ctrldataexchange;windows_formatname=\"SBA-CTRLFORMAT\""_ustr, "SBA-CTRLFORMAT", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 98 SotClipboardFormatId::OUTPLACE_OBJ*/           { u"application/x-openoffice-outplace-obj;windows_formatname=\"OutPlace Object\""_ustr, "OutPlace Object", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /* 99 SotClipboardFormatId::CNT_OWN_CLIP*/           { u"application/x-openoffice-cnt-own-clip;windows_formatname=\"CntOwnClipboard\""_ustr, "CntOwnClipboard", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*100 SotClipboardFormatId::INET_IMAGE*/             { u"application/x-openoffice-inet-image;windows_formatname=\"SO-INet-Image\""_ustr, "SO-INet-Image", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*101 SotClipboardFormatId::NETSCAPE_IMAGE*/         { u"application/x-openoffice-netscape-image;windows_formatname=\"Netscape Image Format\""_ustr, "Netscape Image Format", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*102 SotClipboardFormatId::SBA_FORMEXCHANGE*/       { u"application/x-openoffice-sba-formexchange;windows_formatname=\"SBA_FORMEXCHANGE\""_ustr, "SBA_FORMEXCHANGE", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*103 SotClipboardFormatId::SBA_REPORTEXCHANGE*/     { u"application/x-openoffice-sba-reportexchange;windows_formatname=\"SBA_REPORTEXCHANGE\""_ustr, "SBA_REPORTEXCHANGE", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*104 SotClipboardFormatId::UNIFORMRESOURCELOCATOR*/ { u"application/x-openoffice-uniformresourcelocator;windows_formatname=\"UniformResourceLocatorW\""_ustr, "UniformResourceLocator", &cppu::UnoType<OUString>::get() },
        /*105 SotClipboardFormatId::STARCHARTDOCUMENT_50*/   { u"application/x-openoffice-starchartdocument-50;windows_formatname=\"StarChartDocument 5.0\""_ustr, "StarChartDocument 5.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*106 SotClipboardFormatId::GRAPHOBJ*/               { u"application/x-openoffice-graphobj;windows_formatname=\"Graphic Object\""_ustr, "Graphic Object", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*107 SotClipboardFormatId::STARWRITER_60*/          { MIMETYPE_VND_SUN_XML_WRITER_ASCII, "Writer 6.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*108 SotClipboardFormatId::STARWRITERWEB_60*/       { MIMETYPE_VND_SUN_XML_WRITER_WEB_ASCII, "Writer/Web 6.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*109 SotClipboardFormatId::STARWRITERGLOB_60*/      { MIMETYPE_VND_SUN_XML_WRITER_GLOBAL_ASCII, "Writer/Global 6.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*110 SotClipboardFormatId::STARWDRAW_60*/           { MIMETYPE_VND_SUN_XML_DRAW_ASCII, "Draw 6.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*111 SotClipboardFormatId::STARIMPRESS_60*/         { MIMETYPE_VND_SUN_XML_IMPRESS_ASCII, "Impress 6.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*112 SotClipboardFormatId::STARCALC_60*/            { MIMETYPE_VND_SUN_XML_CALC_ASCII, "Calc 6.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*113 SotClipboardFormatId::STARCHART_60*/           { MIMETYPE_VND_SUN_XML_CHART_ASCII, "Chart 6.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*114 SotClipboardFormatId::STARMATH_60*/            { MIMETYPE_VND_SUN_XML_MATH_ASCII, "Math 6.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*115 SotClipboardFormatId::WMF*/                    { u"application/x-openoffice-wmf;windows_formatname=\"Image WMF\""_ustr, "Windows MetaFile", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*116 SotClipboardFormatId::DBACCESS_QUERY*/         { u"application/x-openoffice-dbaccess-query;windows_formatname=\"Data source Query Object\""_ustr, "Data source Query Object", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*117 SotClipboardFormatId::DBACCESS_TABLE*/         { u"application/x-openoffice-dbaccess-table;windows_formatname=\"Data source Table\""_ustr, "Data source Table", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*118 SotClipboardFormatId::DBACCESS_COMMAND*/       { u"application/x-openoffice-dbaccess-command;windows_formatname=\"SQL query\""_ustr, "SQL query", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*119 SotClipboardFormatId::DIALOG_60*/              { u"application/vnd.sun.xml.dialog"_ustr, "Dialog 6.0", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*120 SotClipboardFormatId::EMF*/                    { u"application/x-openoffice-emf;windows_formatname=\"Image EMF\""_ustr, "Windows Enhanced MetaFile", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*121 SotClipboardFormatId::BIFF_8*/                 { u"application/x-openoffice-biff-8;windows_formatname=\"Biff8\""_ustr, "Biff8", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*122 SotClipboardFormatId::BMP*/                    { u"image/bmp"_ustr, "Windows Bitmap", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*123 SotClipboardFormatId::HTML_NO_COMMENT */       { u"application/x-openoffice-html-no-comment;windows_formatname=\"HTML Format\""_ustr, "HTML (no comment)", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*124 SotClipboardFormatId::STARWRITER_8*/          { MIMETYPE_OASIS_OPENDOCUMENT_TEXT_ASCII, "Writer 8", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*125 SotClipboardFormatId::STARWRITERWEB_8*/       { MIMETYPE_OASIS_OPENDOCUMENT_TEXT_WEB_ASCII, "Writer/Web 8", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*126 SotClipboardFormatId::STARWRITERGLOB_8*/      { MIMETYPE_OASIS_OPENDOCUMENT_TEXT_GLOBAL_ASCII, "Writer/Global 8", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*127 SotClipboardFormatId::STARWDRAW_8*/           { MIMETYPE_OASIS_OPENDOCUMENT_DRAWING_ASCII, "Draw 8", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*128 SotClipboardFormatId::STARIMPRESS_8*/         { MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION_ASCII, "Impress 8", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*129 SotClipboardFormatId::STARCALC_8*/            { MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET_ASCII, "Calc 8", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*130 SotClipboardFormatId::STARCHART_8*/           { MIMETYPE_OASIS_OPENDOCUMENT_CHART_ASCII, "Chart 8", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*131 SotClipboardFormatId::STARMATH_8*/            { MIMETYPE_OASIS_OPENDOCUMENT_FORMULA_ASCII, "Math 8", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*132 SotClipboardFormatId::XFORMS */               { u"application/x-openoffice-xforms;windows_formatname=\"??? Format\""_ustr, "???", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*133 SotClipboardFormatId::STARWRITER_8_TEMPLATE*/          { MIMETYPE_OASIS_OPENDOCUMENT_TEXT_TEMPLATE_ASCII, "Writer 8 Template", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*134 SotClipboardFormatId::STARWDRAW_8_TEMPLATE*/           { MIMETYPE_OASIS_OPENDOCUMENT_DRAWING_TEMPLATE_ASCII, "Draw 8 Template", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*135 SotClipboardFormatId::STARIMPRESS_8_TEMPLATE*/         { MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION_TEMPLATE_ASCII, "Impress 8 Template", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*136 SotClipboardFormatId::STARCALC_8_TEMPLATE*/            { MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET_TEMPLATE_ASCII, "Calc 8 Template", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*137 SotClipboardFormatId::STARCHART_8_TEMPLATE*/           { MIMETYPE_OASIS_OPENDOCUMENT_CHART_TEMPLATE_ASCII, "Chart 8 Template", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*138 SotClipboardFormatId::STARMATH_8_TEMPLATE*/            { MIMETYPE_OASIS_OPENDOCUMENT_FORMULA_TEMPLATE_ASCII, "Math 8 Template", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*139 SotClipboardFormatId::STARBASE_8*/             { MIMETYPE_OASIS_OPENDOCUMENT_DATABASE_ASCII, "StarBase 8", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*140 SotClipboardFormatId::HC_GDIMETAFILE*/         { u"application/x-openoffice-highcontrast-gdimetafile;windows_formatname=\"GDIMetaFile\""_ustr, "High Contrast GDIMetaFile", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*141 SotClipboardFormatId::PNG*/                    { u"image/png"_ustr, "PNG Bitmap", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*142 SotClipboardFormatId::STARWRITERGLOB_8_TEMPLATE*/      { MIMETYPE_OASIS_OPENDOCUMENT_TEXT_GLOBAL_TEMPLATE_ASCII, "Writer/Global 8 Template", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*143 SotClipboardFormatId::MATHML*/   { u"application/mathml+xml"_ustr, "MathML", &::cppu::UnoType<const Sequence< sal_Int8 >>::get() },
        /*144 SotClipboardFormatId::JPEG*/ { u"image/jpeg"_ustr, "JPEG Bitmap", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*145 SotClipboardFormatId::RICHTEXT*/ { u"text/richtext"_ustr, "Richtext Format", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*146 SotClipboardFormatId::STRING_TSVC*/            { u"application/x-libreoffice-tsvc"_ustr, "Text TSV-Calc", &cppu::UnoType<OUString>::get() },
        /*147 SotClipboardFormatId::PDF*/            { u"application/pdf"_ustr, "PDF Document", &cppu::UnoType<Sequence<sal_Int8>>::get() },
        /*148 SotClipboardFormatId::SVG*/            { u"image/svg+xml;windows_formatname=\"image/svg+xml\""_ustr, "SVG", &cppu::UnoType<Sequence<sal_Int8>>::get() },
    };
    return &aInstance[0];
};

    typedef std::vector<css::datatransfer::DataFlavor> tDataFlavorList;
}

static tDataFlavorList& InitFormats_Impl()
{
    static tDataFlavorList gImplData;

    return gImplData;
}

/*************************************************************************
|*
|*    SotExchange::RegisterFormatName()
|*
|*    Description       CLIP.SDW
*************************************************************************/
SotClipboardFormatId SotExchange::RegisterFormatName( const OUString& rName )
{
    const DataFlavorRepresentation *pFormatArray_Impl = FormatArray_Impl();
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
        auto const& rFlavor = rL[ i ];
        if( rName == rFlavor.HumanPresentableName )
            return static_cast<SotClipboardFormatId>(i + static_cast<int>(SotClipboardFormatId::USER_END) + 1);
    }

    DataFlavor aNewFlavor;
    aNewFlavor.MimeType = rName;
    aNewFlavor.HumanPresentableName = rName;
    aNewFlavor.DataType = cppu::UnoType<OUString>::get();

    rL.push_back( std::move(aNewFlavor) );

    return static_cast<SotClipboardFormatId>(static_cast<int>(rL.size()-1) + static_cast<int>(SotClipboardFormatId::USER_END) + 1);
}

SotClipboardFormatId SotExchange::RegisterFormatMimeType( const OUString& rMimeType )
{
    const DataFlavorRepresentation *pFormatArray_Impl = FormatArray_Impl();
    // test the default first - name
    for( SotClipboardFormatId i = SotClipboardFormatId::STRING; i <= SotClipboardFormatId::FILE_LIST;  ++i )
        if( rMimeType.equals( pFormatArray_Impl[ static_cast<int>(i) ].pMimeType ) )
            return i;

    for( SotClipboardFormatId i = SotClipboardFormatId::RTF; i <= SotClipboardFormatId::USER_END;  ++i )
        if( rMimeType.equals( pFormatArray_Impl[ static_cast<int>(i) ].pMimeType ) )
            return i;

    // then in the dynamic list
    tDataFlavorList& rL = InitFormats_Impl();
    for( tDataFlavorList::size_type i = 0; i < rL.size(); i++ )
    {
        auto const& rFlavor = rL[ i ];
        if( rMimeType == rFlavor.MimeType )
            return static_cast<SotClipboardFormatId>(i + static_cast<int>(SotClipboardFormatId::USER_END) + 1);
    }

    DataFlavor aNewFlavor;
    aNewFlavor.MimeType = rMimeType;
    aNewFlavor.HumanPresentableName = rMimeType;
    aNewFlavor.DataType = cppu::UnoType<OUString>::get();

    rL.push_back( std::move(aNewFlavor) );

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
        rL.emplace_back( rFlavor );
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
        const DataFlavorRepresentation& rData = FormatArray_Impl()[static_cast<int>(nFormat)];
        rFlavor.MimeType = rData.pMimeType;
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

/*************************************************************************
|*
|*    SotExchange::GetFormatMimeType( SotClipboardFormatId nFormat )
|*
*************************************************************************/

OUString SotExchange::GetFormatMimeType( SotClipboardFormatId nFormat )
{
    OUString sMimeType;
    if( SotClipboardFormatId::USER_END >= nFormat )
        sMimeType = FormatArray_Impl()[static_cast<int>(nFormat)].pMimeType;
    else
    {
        tDataFlavorList& rL = InitFormats_Impl();

        unsigned i = static_cast<int>(nFormat) - static_cast<int>(SotClipboardFormatId::USER_END) - 1;

        if( rL.size() > i )
            sMimeType = rL[ i ].MimeType;
    }

    DBG_ASSERT( !sMimeType.isEmpty(), "SotExchange::GetFormatMimeType(): DataFlavor not initialized" );

    return sMimeType;
}

/*************************************************************************
|*
|*    SotExchange::GetFormatIdFromMimeType( const String& rMimeType )
|*
*************************************************************************/

SotClipboardFormatId SotExchange::GetFormatIdFromMimeType( std::u16string_view rMimeType )
{
    const DataFlavorRepresentation *pFormatArray_Impl = FormatArray_Impl();
    for( SotClipboardFormatId i = SotClipboardFormatId::STRING; i <= SotClipboardFormatId::FILE_LIST;  ++i )
        if( rMimeType == pFormatArray_Impl[ static_cast<int>(i) ].pMimeType )
            return i;

    // BM: the chart format 105 ("StarChartDocument 5.0") was written
    // only into 5.1 chart documents - in 5.0 and 5.2 it was 42 ("StarChart 5.0")
    // The registry only contains the entry for the 42 format id.
    for( SotClipboardFormatId i = SotClipboardFormatId::RTF; i <= SotClipboardFormatId::USER_END;  ++i )
        if( rMimeType == pFormatArray_Impl[ static_cast<int>(i) ].pMimeType )
            return ( (i == SotClipboardFormatId::STARCHARTDOCUMENT_50)
                     ? SotClipboardFormatId::STARCHART_50
                     : i );

    // then in the dynamic list
    tDataFlavorList& rL = InitFormats_Impl();

    for( tDataFlavorList::size_type i = 0; i < rL.size(); i++ )
    {
        auto const& rFlavor = rL[ i ];
        if( rMimeType == rFlavor.MimeType )
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

    const DataFlavorRepresentation *pFormatArray_Impl = FormatArray_Impl();
    for( SotClipboardFormatId i = SotClipboardFormatId::STRING; i <= SotClipboardFormatId::FILE_LIST;  ++i )
        if( rMimeType.equals( pFormatArray_Impl[ static_cast<int>(i) ].pMimeType ) )
            return i;

    // BM: the chart format 105 ("StarChartDocument 5.0") was written
    // only into 5.1 chart documents - in 5.0 and 5.2 it was 42 ("StarChart 5.0")
    // The registry only contains the entry for the 42 format id.
    for( SotClipboardFormatId i = SotClipboardFormatId::RTF; i <= SotClipboardFormatId::USER_END;  ++i )
    {
        const OUString& pFormatMimeType = pFormatArray_Impl[ static_cast<int>(i) ].pMimeType;
        const sal_Int32 nFormatMimeTypeLen = pFormatMimeType.getLength();
        if( rMimeType.match( pFormatMimeType ) &&
            ( rMimeType.getLength() == nFormatMimeTypeLen ||
              rMimeType[ nFormatMimeTypeLen ] == ';' ) )
            return ( (i == SotClipboardFormatId::STARCHARTDOCUMENT_50)
                     ? SotClipboardFormatId::STARCHART_50
                     : i );
    }

    // then in the dynamic list
    tDataFlavorList& rL = InitFormats_Impl();
    for( tDataFlavorList::size_type i = 0; i < rL.size(); i++ )
    {
        if( rMimeType == rL[ i ].MimeType )
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
