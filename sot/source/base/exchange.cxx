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

#define _SOT_FORMATS_INCLUDE_SYSTEMFORMATS
#include <tools/debug.hxx>
#include <tools/solar.h>
#include <tools/globname.hxx>
#include <sot/sotdata.hxx>
#include <sot/exchange.hxx>
#include <sot/formats.hxx>
#include <comphelper/classids.hxx>
#include <rtl/instance.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <comphelper/documentconstants.hxx>

#ifdef GetObject
#undef GetObject
#endif

using namespace::com::sun::star::uno;
using namespace::com::sun::star::datatransfer;

/*
    In diesen Tabellen stehen alle im Office verwendeten MimeTypes,
    Format-Bezeichner und Types.
    Die Tabelle ist nach den Formatstring-Ids sortiert und jede Id
    ist um genau 1 groesser als ihre Vorgaenger-Id, damit die Id als
    Tabellenindex benutzt werden kann.
*/
struct DataFlavorRepresentation
{
    const char*                         pMimeType;
    const char*                         pName;
    const ::com::sun::star::uno::Type*  pType;
};



namespace
{
    struct ImplFormatArray_Impl
    {
        const DataFlavorRepresentation* operator()()
        {
            static const DataFlavorRepresentation aInstance[] =
            {
            /*  0 SOT_FORMAT_SYSTEM_START*/                 { "", "", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*  1 SOT_FORMAT_STRING*/                       { "text/plain;charset=utf-16", "Text", &::getCppuType( (const OUString*) 0 ) },
            /*  2 SOT_FORMAT_BITMAP*/                       { "application/x-openoffice-bitmap;windows_formatname=\"Bitmap\"", "Bitmap", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*  3 SOT_FORMAT_GDIMETAFILE*/                  { "application/x-openoffice-gdimetafile;windows_formatname=\"GDIMetaFile\"", "GDIMetaFile", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*  4 SOT_FORMAT_PRIVATE*/                      { "application/x-openoffice-private;windows_formatname=\"Private\"", "Private", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*  5 SOT_FORMAT_FILE*/                         { "application/x-openoffice-file;windows_formatname=\"FileName\"", "FileName", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*  6 SOT_FORMAT_FILE_LIST*/                    { "application/x-openoffice-filelist;windows_formatname=\"FileList\"", "FileList", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*  7 EMPTY*/                                   { "", "", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*  8 EMPTY*/                                   { "", "", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*  9 EMPTY*/                                   { "", "", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 10 SOT_FORMAT_RTF*/                          { "text/richtext", "Rich Text Format", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 11 SOT_FORMATSTR_ID_DRAWING*/                { "application/x-openoffice-drawing;windows_formatname=\"Drawing Format\"", "Drawing Format", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 12 SOT_FORMATSTR_ID_SVXB*/                   { "application/x-openoffice-svxb;windows_formatname=\"SVXB (StarView Bitmap/Animation)\"", "SVXB (StarView Bitmap/Animation)", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 13 SOT_FORMATSTR_ID_SVIM*/                   { "application/x-openoffice-svim;windows_formatname=\"SVIM (StarView ImageMap)\"", "SVIM (StarView ImageMap)", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 14 SOT_FORMATSTR_ID_XFA*/                    { "application/x-openoffice-xfa;windows_formatname=\"XFA (XOutDev FillAttr)\"", "XFA (XOutDev FillAttr)", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 15 SOT_FORMATSTR_ID_EDITENGINE*/             { "application/x-openoffice-editengine;windows_formatname=\"EditEngineFormat\"", "EditEngineFormat", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 16 SOT_FORMATSTR_ID_INTERNALLINK_STATE*/     { "application/x-openoffice-internallink-state;windows_formatname=\"StatusInfo vom SvxInternalLink\"", "StatusInfo vom SvxInternalLink", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 17 SOT_FORMATSTR_ID_SOLK*/                   { "application/x-openoffice-solk;windows_formatname=\"SOLK (StarOffice Link)\"", "SOLK (StarOffice Link)", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 18 SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK*/      { "application/x-openoffice-netscape-bookmark;windows_formatname=\"Netscape Bookmark\"", "Netscape Bookmark", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 19 SOT_FORMATSTR_ID_TREELISTBOX*/            { "application/x-openoffice-treelistbox;windows_formatname=\"SV_LBOX_DD_FORMAT\"", "SV_LBOX_DD_FORMAT", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 20 SOT_FORMATSTR_ID_NATIVE*/                 { "application/x-openoffice-native;windows_formatname=\"Native\"", "Native", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 21 SOT_FORMATSTR_ID_OWNERLINK*/              { "application/x-openoffice-ownerlink;windows_formatname=\"OwnerLink\"", "OwnerLink", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 22 SOT_FORMATSTR_ID_STARSERVER*/             { "application/x-openoffice-starserver;windows_formatname=\"StarServerFormat\"", "StarServerFormat", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 23 SOT_FORMATSTR_ID_STAROBJECT*/             { "application/x-openoffice-starobject;windows_formatname=\"StarObjectFormat\"", "StarObjectFormat", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 24 SOT_FORMATSTR_ID_APPLETOBJECT*/           { "application/x-openoffice-appletobject;windows_formatname=\"Applet Object\"", "Applet Object", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 25 SOT_FORMATSTR_ID_PLUGIN_OBJECT*/          { "application/x-openoffice-plugin-object;windows_formatname=\"PlugIn Object\"", "PlugIn Object", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 26 SOT_FORMATSTR_ID_STARWRITER_30*/          { "application/x-openoffice-starwriter-30;windows_formatname=\"StarWriter 3.0\"", "StarWriter 3.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 27 SOT_FORMATSTR_ID_STARWRITER_40*/          { "application/x-openoffice-starwriter-40;windows_formatname=\"StarWriter 4.0\"", "StarWriter 4.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 28 SOT_FORMATSTR_ID_STARWRITER_50*/          { "application/x-openoffice-starwriter-50;windows_formatname=\"StarWriter 5.0\"", "StarWriter 5.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 29 SOT_FORMATSTR_ID_STARWRITERWEB_40*/       { "application/x-openoffice-starwriterweb-40;windows_formatname=\"StarWriter/Web 4.0\"", "StarWriter/Web 4.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 30 SOT_FORMATSTR_ID_STARWRITERWEB_50*/       { "application/x-openoffice-starwriterweb-50;windows_formatname=\"StarWriter/Web 5.0\"", "StarWriter/Web 5.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 31 SOT_FORMATSTR_ID_STARWRITERGLOB_40*/      { "application/x-openoffice-starwriterglob-40;windows_formatname=\"StarWriter/Global 4.0\"", "StarWriter/Global 4.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 32 SOT_FORMATSTR_ID_STARWRITERGLOB_50*/      { "application/x-openoffice-starwriterglob-50;windows_formatname=\"StarWriter/Global 5.0\"", "StarWriter/Global 5.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 33 SOT_FORMATSTR_ID_STARDRAW*/               { "application/x-openoffice-stardraw;windows_formatname=\"StarDrawDocument\"", "StarDrawDocument", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 34 SOT_FORMATSTR_ID_STARDRAW_40*/            { "application/x-openoffice-stardraw-40;windows_formatname=\"StarDrawDocument 4.0\"", "StarDrawDocument 4.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 35 SOT_FORMATSTR_ID_STARIMPRESS_50*/         { "application/x-openoffice-starimpress-50;windows_formatname=\"StarImpress 5.0\"", "StarImpress 5.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 36 SOT_FORMATSTR_ID_STARDRAW_50*/            { "application/x-openoffice-stardraw-50;windows_formatname=\"StarDraw 5.0\"", "StarDraw 5.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 37 SOT_FORMATSTR_ID_STARCALC*/               { "application/x-openoffice-starcalc;windows_formatname=\"StarCalcDocument\"", "StarCalcDocument", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 38 SOT_FORMATSTR_ID_STARCALC_40*/            { "application/x-openoffice-starcalc-40;windows_formatname=\"StarCalc 4.0\"", "StarCalc 4.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 39 SOT_FORMATSTR_ID_STARCALC_50*/            { "application/x-openoffice-starcalc-50;windows_formatname=\"StarCalc 5.0\"", "StarCalc 5.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 40 SOT_FORMATSTR_ID_STARCHART*/              { "application/x-openoffice-starchart;windows_formatname=\"StarChartDocument\"", "StarChartDocument", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 41 SOT_FORMATSTR_ID_STARCHART_40*/           { "application/x-openoffice-starchart-40;windows_formatname=\"StarChartDocument 4.0\"", "StarChartDocument 4.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 42 SOT_FORMATSTR_ID_STARCHART_50*/           { "application/x-openoffice-starchart-50;windows_formatname=\"StarChart 5.0\"", "StarChart 5.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 43 SOT_FORMATSTR_ID_STARIMAGE*/              { "application/x-openoffice-starimage;windows_formatname=\"StarImageDocument\"", "StarImageDocument", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 44 SOT_FORMATSTR_ID_STARIMAGE_40*/           { "application/x-openoffice-starimage-40;windows_formatname=\"StarImageDocument 4.0\"", "StarImageDocument 4.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 45 SOT_FORMATSTR_ID_STARIMAGE_50*/           { "application/x-openoffice-starimage-50;windows_formatname=\"StarImage 5.0\"", "StarImage 5.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 46 SOT_FORMATSTR_ID_STARMATH*/               { "application/x-openoffice-starmath;windows_formatname=\"StarMath\"", "StarMath", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 47 SOT_FORMATSTR_ID_STARMATH_40*/            { "application/x-openoffice-starmath-40;windows_formatname=\"StarMathDocument 4.0\"", "StarMathDocument 4.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 48 SOT_FORMATSTR_ID_STARMATH_50*/            { "application/x-openoffice-starmath-50;windows_formatname=\"StarMath 5.0\"", "StarMath 5.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 49 SOT_FORMATSTR_ID_STAROBJECT_PAINTDOC*/    { "application/x-openoffice-starobject-paintdoc;windows_formatname=\"StarObjectPaintDocument\"", "StarObjectPaintDocument", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 50 SOT_FORMATSTR_ID_FILLED_AREA*/            { "application/x-openoffice-filled-area;windows_formatname=\"FilledArea\"", "FilledArea", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 51 SOT_FORMATSTR_ID_HTML*/                   { "text/html", "HTML (HyperText Markup Language)", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 52 SOT_FORMATSTR_ID_HTML_SIMPLE*/            { "application/x-openoffice-html-simple;windows_formatname=\"HTML Format\"", "HTML Format", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 53 SOT_FORMATSTR_ID_CHAOS*/                  { "application/x-openoffice-chaos;windows_formatname=\"FORMAT_CHAOS\"", "FORMAT_CHAOS", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 54 SOT_FORMATSTR_ID_CNT_MSGATTACHFILE*/      { "application/x-openoffice-cnt-msgattachfile;windows_formatname=\"CNT_MSGATTACHFILE_FORMAT\"", "CNT_MSGATTACHFILE_FORMAT", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 55 SOT_FORMATSTR_ID_BIFF_5*/                 { "application/x-openoffice-biff5;windows_formatname=\"Biff5\"", "Biff5", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 56 SOT_FORMATSTR_ID_BIFF__5*/                { "application/x-openoffice-biff-5;windows_formatname=\"Biff 5\"", "Biff 5", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 57 SOT_FORMATSTR_ID_SYLK*/                   { "application/x-openoffice-sylk;windows_formatname=\"Sylk\"", "Sylk", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 58 SOT_FORMATSTR_ID_SYLK_BIGCAPS*/           { "application/x-openoffice-sylk-bigcaps;windows_formatname=\"SYLK\"", "SYLK", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 59 SOT_FORMATSTR_ID_LINK*/                   { "application/x-openoffice-link;windows_formatname=\"Link\"", "Link", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 60 SOT_FORMATSTR_ID_DIF*/                    { "application/x-openoffice-dif;windows_formatname=\"DIF\"", "DIF", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 61 SOT_FORMATSTR_ID_STARDRAW_TABBAR*/        { "application/x-openoffice-stardraw-tabbar;windows_formatname=\"StarDraw TabBar\"", "StarDraw TabBar", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 62 SOT_FORMATSTR_ID_SONLK*/                  { "application/x-openoffice-sonlk;windows_formatname=\"SONLK (StarOffice Navi Link)\"", "SONLK (StarOffice Navi Link)", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 63 SOT_FORMATSTR_ID_MSWORD_DOC*/             { "application/msword", "MSWordDoc", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 64 SOT_FORMATSTR_ID_STAR_FRAMESET_DOC*/      { "application/x-openoffice-star-frameset-doc;windows_formatname=\"StarFrameSetDocument\"", "StarFrameSetDocument", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 65 SOT_FORMATSTR_ID_OFFICE_DOC*/             { "application/x-openoffice-office-doc;windows_formatname=\"OfficeDocument\"", "OfficeDocument", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 66 SOT_FORMATSTR_ID_NOTES_DOCINFO*/          { "application/x-openoffice-notes-docinfo;windows_formatname=\"NotesDocInfo\"", "NotesDocInfo", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 67 SOT_FORMATSTR_ID_NOTES_HNOTE*/            { "application/x-openoffice-notes-hnote;windows_formatname=\"NoteshNote\"", "NoteshNote", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 68 SOT_FORMATSTR_ID_NOTES_NATIVE*/           { "application/x-openoffice-notes-native;windows_formatname=\"Native\"", "Native", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 69 SOT_FORMATSTR_ID_SFX_DOC*/                { "application/x-openoffice-sfx-doc;windows_formatname=\"SfxDocument\"", "SfxDocument", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 70 SOT_FORMATSTR_ID_EVDF*/                   { "application/x-openoffice-evdf;windows_formatname=\"EVDF (Explorer View Dummy Format)\"", "EVDF (Explorer View Dummy Format)", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 71 SOT_FORMATSTR_ID_ESDF*/                   { "application/x-openoffice-esdf;windows_formatname=\"ESDF (Explorer Search Dummy Format)\"", "ESDF (Explorer Search Dummy Format)", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 72 SOT_FORMATSTR_ID_IDF*/                    { "application/x-openoffice-idf;windows_formatname=\"IDF (Iconview Dummy Format)\"", "IDF (Iconview Dummy Format)", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 73 SOT_FORMATSTR_ID_EFTP*/                   { "application/x-openoffice-eftp;windows_formatname=\"EFTP (Explorer Ftp File)\"", "EFTP (Explorer Ftp File)", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 74 SOT_FORMATSTR_ID_EFD*/                    { "application/x-openoffice-efd;windows_formatname=\"EFD (Explorer Ftp Dir)\"", "EFD (Explorer Ftp Dir)", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 75 SOT_FORMATSTR_ID_SVX_FORMFIELDEXCH*/      { "application/x-openoffice-svx-formfieldexch;windows_formatname=\"SvxFormFieldExch\"", "SvxFormFieldExch", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 76 SOT_FORMATSTR_ID_EXTENDED_TABBAR*/        { "application/x-openoffice-extended-tabbar;windows_formatname=\"ExtendedTabBar\"", "ExtendedTabBar", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 77 SOT_FORMATSTR_ID_SBA_DATAEXCHANGE*/       { "application/x-openoffice-sba-dataexchange;windows_formatname=\"SBA-DATAFORMAT\"", "SBA-DATAFORMAT", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 78 SOT_FORMATSTR_ID_SBA_FIELDDATAEXCHANGE*/  { "application/x-openoffice-sba-fielddataexchange;windows_formatname=\"SBA-FIELDFORMAT\"", "SBA-FIELDFORMAT", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 79 SOT_FORMATSTR_ID_SBA_PRIVATE_URL*/        { "application/x-openoffice-sba-private-url;windows_formatname=\"SBA-PRIVATEURLFORMAT\"", "SBA-PRIVATEURLFORMAT", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 80 SOT_FORMATSTR_ID_SBA_TABED*/              { "application/x-openofficesba-tabed;windows_formatname=\"Tabed\"", "Tabed", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 81 SOT_FORMATSTR_ID_SBA_TABID*/              { "application/x-openoffice-sba-tabid;windows_formatname=\"Tabid\"", "Tabid", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 82 SOT_FORMATSTR_ID_SBA_JOIN*/               { "application/x-openoffice-sba-join;windows_formatname=\"SBA-JOINFORMAT\"", "SBA-JOINFORMAT", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 83 SOT_FORMATSTR_ID_OBJECTDESCRIPTOR*/       { "application/x-openoffice-objectdescriptor-xml;windows_formatname=\"Star Object Descriptor (XML)\"", "Star Object Descriptor (XML)", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 84 SOT_FORMATSTR_ID_LINKSRCDESCRIPTOR*/      { "application/x-openoffice-linksrcdescriptor-xml;windows_formatname=\"Star Link Source Descriptor (XML)\"", "Star Link Source Descriptor (XML)", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 85 SOT_FORMATSTR_ID_EMBED_SOURCE*/           { "application/x-openoffice-embed-source-xml;windows_formatname=\"Star Embed Source (XML)\"", "Star Embed Source (XML)", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 86 SOT_FORMATSTR_ID_LINK_SOURCE*/            { "application/x-openoffice-link-source-xml;windows_formatname=\"Star Link Source (XML)\"", "Star Link Source (XML)", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 87 SOT_FORMATSTR_ID_EMBEDDED_OBJ*/           { "application/x-openoffice-embedded-obj-xml;windows_formatname=\"Star Embedded Object (XML)\"", "Star Embedded Object (XML)", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 88 SOT_FORMATSTR_ID_FILECONTENT*/            { "application/x-openoffice-filecontent;windows_formatname=\"FileContents\"", "FileContents", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 89 SOT_FORMATSTR_ID_FILEGRPDESCRIPTOR*/      { "application/x-openoffice-filegrpdescriptor;windows_formatname=\"FileGroupDescriptor\"", "FileGroupDescriptor", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 90 SOT_FORMATSTR_ID_FILENAME*/               { "application/x-openoffice-filename;windows_formatname=\"FileName\"", "FileName", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 91 SOT_FORMATSTR_ID_SD_OLE*/                 { "application/x-openoffice-sd-ole;windows_formatname=\"SD-OLE\"", "SD-OLE", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 92 SOT_FORMATSTR_ID_EMBEDDED_OBJ_OLE*/       { "application/x-openoffice-embedded-obj-ole;windows_formatname=\"Embedded Object\"", "Embedded Object", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 93 SOT_FORMATSTR_ID_EMBED_SOURCE_OLE*/       { "application/x-openoffice-embed-source-ole;windows_formatname=\"Embed Source\"", "Embed Source", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 94 SOT_FORMATSTR_ID_OBJECTDESCRIPTOR_OLE*/   { "application/x-openoffice-objectdescriptor-ole;windows_formatname=\"Object Descriptor\"", "Object Descriptor", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 95 SOT_FORMATSTR_ID_LINKSRCDESCRIPTOR_OLE*/  { "application/x-openoffice-linkdescriptor-ole;windows_formatname=\"Link Source Descriptor\"", "Link Source Descriptor", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 96 SOT_FORMATSTR_ID_LINK_SOURCE_OLE*/        { "application/x-openoffice-link-source-ole;windows_formatname=\"Link Source\"", "Link Source", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 97 SOT_FORMATSTR_ID_SBA_CTRLDATAEXCHANGE*/   { "application/x-openoffice-sba-ctrldataexchange;windows_formatname=\"SBA-CTRLFORMAT\"", "SBA-CTRLFORMAT", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 98 SOT_FORMATSTR_ID_OUTPLACE_OBJ*/           { "application/x-openoffice-outplace-obj;windows_formatname=\"OutPlace Object\"", "OutPlace Object", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /* 99 SOT_FORMATSTR_ID_CNT_OWN_CLIP*/           { "application/x-openoffice-cnt-own-clip;windows_formatname=\"CntOwnClipboard\"", "CntOwnClipboard", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*100 SOT_FORMATSTR_ID_INET_IMAGE*/             { "application/x-openoffice-inet-image;windows_formatname=\"SO-INet-Image\"", "SO-INet-Image", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*101 SOT_FORMATSTR_ID_NETSCAPE_IMAGE*/         { "application/x-openoffice-netscape-image;windows_formatname=\"Netscape Image Format\"", "Netscape Image Format", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*102 SOT_FORMATSTR_ID_SBA_FORMEXCHANGE*/       { "application/x-openoffice-sba-formexchange;windows_formatname=\"SBA_FORMEXCHANGE\"", "SBA_FORMEXCHANGE", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*103 SOT_FORMATSTR_ID_SBA_REPORTEXCHANGE*/     { "application/x-openoffice-sba-reportexchange;windows_formatname=\"SBA_REPORTEXCHANGE\"", "SBA_REPORTEXCHANGE", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*104 SOT_FORMATSTR_ID_UNIFORMRESOURCELOCATOR*/ { "application/x-openoffice-uniformresourcelocator;windows_formatname=\"UniformResourceLocator\"", "UniformResourceLocator", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*105 SOT_FORMATSTR_ID_STARCHARTDOCUMENT_50*/   { "application/x-openoffice-starchartdocument-50;windows_formatname=\"StarChartDocument 5.0\"", "StarChartDocument 5.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*106 SOT_FORMATSTR_ID_GRAPHOBJ*/               { "application/x-openoffice-graphobj;windows_formatname=\"Graphic Object\"", "Graphic Object", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*107 SOT_FORMATSTR_ID_STARWRITER_60*/          { MIMETYPE_VND_SUN_XML_WRITER_ASCII, "Writer 6.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*108 SOT_FORMATSTR_ID_STARWRITERWEB_60*/       { MIMETYPE_VND_SUN_XML_WRITER_WEB_ASCII, "Writer/Web 6.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*109 SOT_FORMATSTR_ID_STARWRITERGLOB_60*/      { MIMETYPE_VND_SUN_XML_WRITER_GLOBAL_ASCII, "Writer/Global 6.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*110 SOT_FORMATSTR_ID_STARWDRAW_60*/           { MIMETYPE_VND_SUN_XML_DRAW_ASCII, "Draw 6.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*111 SOT_FORMATSTR_ID_STARIMPRESS_60*/         { MIMETYPE_VND_SUN_XML_IMPRESS_ASCII, "Impress 6.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*112 SOT_FORMATSTR_ID_STARCALC_60*/            { MIMETYPE_VND_SUN_XML_CALC_ASCII, "Calc 6.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*113 SOT_FORMATSTR_ID_STARCHART_60*/           { MIMETYPE_VND_SUN_XML_CHART_ASCII, "Chart 6.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*114 SOT_FORMATSTR_ID_STARMATH_60*/            { MIMETYPE_VND_SUN_XML_MATH_ASCII, "Math 6.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*115 SOT_FORMATSTR_ID_WMF*/                    { "application/x-openoffice-wmf;windows_formatname=\"Image WMF\"", "Windows MetaFile", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*116 SOT_FORMATSTR_ID_DBACCESS_QUERY*/         { "application/x-openoffice-dbaccess-query;windows_formatname=\"Data source Query Object\"", "Data source Query Object", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*117 SOT_FORMATSTR_ID_DBACCESS_TABLE*/         { "application/x-openoffice-dbaccess-table;windows_formatname=\"Data source Table\"", "Data source Table", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*118 SOT_FORMATSTR_ID_DBACCESS_COMMAND*/       { "application/x-openoffice-dbaccess-command;windows_formatname=\"SQL query\"", "SQL query", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*119 SOT_FORMATSTR_ID_DIALOG_60*/              { "application/vnd.sun.xml.dialog", "Dialog 6.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*120 SOT_FORMATSTR_ID_EMF*/                    { "application/x-openoffice-emf;windows_formatname=\"Image EMF\"", "Windows Enhanced MetaFile", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*121 SOT_FORMATSTR_ID_BIFF_8*/                 { "application/x-openoffice-biff-8;windows_formatname=\"Biff8\"", "Biff8", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*122 SOT_FORMATSTR_ID_BMP*/                    { "image/bmp", "Windows Bitmap", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*123 SOT_FORMATSTR_ID_HTML_NO_COMMENT */       { "application/x-openoffice-html-no-comment;windows_formatname=\"HTML Format\"", "HTML (no comment)", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*124 SOT_FORMATSTR_ID_STARWRITER_8*/          { MIMETYPE_OASIS_OPENDOCUMENT_TEXT_ASCII, "Writer 8", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*125 SOT_FORMATSTR_ID_STARWRITERWEB_8*/       { MIMETYPE_OASIS_OPENDOCUMENT_TEXT_WEB_ASCII, "Writer/Web 8", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*126 SOT_FORMATSTR_ID_STARWRITERGLOB_8*/      { MIMETYPE_OASIS_OPENDOCUMENT_TEXT_GLOBAL_ASCII, "Writer/Global 8", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*127 SOT_FORMATSTR_ID_STARWDRAW_8*/           { MIMETYPE_OASIS_OPENDOCUMENT_DRAWING_ASCII, "Draw 8", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*128 SOT_FORMATSTR_ID_STARIMPRESS_8*/         { MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION_ASCII, "Impress 8", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*129 SOT_FORMATSTR_ID_STARCALC_8*/            { MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET_ASCII, "Calc 8", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*130 SOT_FORMATSTR_ID_STARCHART_8*/           { MIMETYPE_OASIS_OPENDOCUMENT_CHART_ASCII, "Chart 8", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*131 SOT_FORMATSTR_ID_STARMATH_8*/            { MIMETYPE_OASIS_OPENDOCUMENT_FORMULA_ASCII, "Math 8", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*132 SOT_FORMATSTR_ID_XFORMS */               { "application/x-openoffice-xforms;windows_formatname=\"??? Format\"", "???", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*133 SOT_FORMATSTR_ID_STARWRITER_8_TEMPLATE*/          { MIMETYPE_OASIS_OPENDOCUMENT_TEXT_TEMPLATE_ASCII, "Writer 8 Template", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*134 SOT_FORMATSTR_ID_STARWDRAW_8_TEMPLATE*/           { MIMETYPE_OASIS_OPENDOCUMENT_DRAWING_TEMPLATE_ASCII, "Draw 8 Template", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*135 SOT_FORMATSTR_ID_STARIMPRESS_8_TEMPLATE*/         { MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION_TEMPLATE_ASCII, "Impress 8 Template", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*136 SOT_FORMATSTR_ID_STARCALC_8_TEMPLATE*/            { MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET_TEMPLATE_ASCII, "Calc 8 Template", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*137 SOT_FORMATSTR_ID_STARCHART_8_TEMPLATE*/           { MIMETYPE_OASIS_OPENDOCUMENT_CHART_TEMPLATE_ASCII, "Chart 8 Template", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*138 SOT_FORMATSTR_ID_STARMATH_8_TEMPLATE*/            { MIMETYPE_OASIS_OPENDOCUMENT_FORMULA_TEMPLATE_ASCII, "Math 8 Template", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*139 SOT_FORMATSTR_ID_STARBASE_8*/             { MIMETYPE_OASIS_OPENDOCUMENT_DATABASE_ASCII, "StarBase 8", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*140 SOT_FORMAT_GDIMETAFILE*/                  { "application/x-openoffice-highcontrast-gdimetafile;windows_formatname=\"GDIMetaFile\"", "High Contrast GDIMetaFile", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            /*141 SOT_FORMATSTR_ID_PNG*/                    { "image/png", "PNG Bitmap", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
            };
        return &aInstance[0];
        }
    };

    struct FormatArray_Impl
        : public rtl::StaticAggregate<
            const DataFlavorRepresentation, ImplFormatArray_Impl > {};
}



static tDataFlavorList& InitFormats_Impl()
{
    SotData_Impl * pSotData = SOTDATA();
    if( !pSotData->pDataFlavorList )
        pSotData->pDataFlavorList = new tDataFlavorList();
    return *pSotData->pDataFlavorList;
}

/*************************************************************************
|*
|*    SotExchange::RegisterFormatName()
|*
|*    Beschreibung      CLIP.SDW
*************************************************************************/
sal_uLong SotExchange::RegisterFormatName( const OUString& rName )
{
    const DataFlavorRepresentation *pFormatArray_Impl = FormatArray_Impl::get();
    // teste zuerst die Standard - Name
    sal_uLong i, nMax = SOT_FORMAT_FILE_LIST;
    for( i = SOT_FORMAT_STRING; i <= nMax;  ++i )
        if( rName.equalsAscii( pFormatArray_Impl[ i ].pName ) )
            return i;

    // BM: the chart format 105 ("StarChartDocument 5.0") was written
    // only into 5.1 chart documents - in 5.0 and 5.2 it was 42 ("StarChart 5.0")
    // The registry only contains the entry for the 42 format id.
    nMax = SOT_FORMATSTR_ID_USER_END;
    for( i = SOT_FORMAT_RTF; i <= nMax;  ++i )
        if( rName.equalsAscii( pFormatArray_Impl[ i ].pName ) )
            return ( (i == SOT_FORMATSTR_ID_STARCHARTDOCUMENT_50)
                     ? SOT_FORMATSTR_ID_STARCHART_50
                     : i );

    // dann in der dynamischen Liste
    tDataFlavorList& rL = InitFormats_Impl();
    for( i = 0, nMax = rL.size(); i < nMax; i++ )
    {
        DataFlavor* pFlavor = rL[ i ];
        if( pFlavor && rName == pFlavor->HumanPresentableName )
            return i + SOT_FORMATSTR_ID_USER_END + 1;
    }

    // nMax ist der neue Platz
    DataFlavor* pNewFlavor = new DataFlavor;

    pNewFlavor->MimeType = rName;
    pNewFlavor->HumanPresentableName = rName;
    pNewFlavor->DataType = ::getCppuType( (const OUString*) 0 );

    rL.push_back( pNewFlavor );

    return nMax + SOT_FORMATSTR_ID_USER_END + 1;
}

sal_uLong SotExchange::RegisterFormatMimeType( const OUString& rMimeType )
{
    const DataFlavorRepresentation *pFormatArray_Impl = FormatArray_Impl::get();
    // teste zuerst die Standard - Name
    sal_uLong i, nMax = SOT_FORMAT_FILE_LIST;
    for( i = SOT_FORMAT_STRING; i <= nMax;  ++i )
        if( rMimeType.equalsAscii( pFormatArray_Impl[ i ].pMimeType ) )
            return i;

    nMax = SOT_FORMATSTR_ID_USER_END;
    for( i = SOT_FORMAT_RTF; i <= nMax;  ++i )
        if( rMimeType.equalsAscii( pFormatArray_Impl[ i ].pMimeType ) )
            return i;

    // dann in der dynamischen Liste
    tDataFlavorList& rL = InitFormats_Impl();
    for( i = 0, nMax = rL.size(); i < nMax; i++ )
    {
        DataFlavor* pFlavor = rL[ i ];
        if( pFlavor && rMimeType == pFlavor->MimeType )
            return i + SOT_FORMATSTR_ID_USER_END + 1;
    }

    // nMax ist der neue Platz
    DataFlavor* pNewFlavor = new DataFlavor;

    pNewFlavor->MimeType = rMimeType;
    pNewFlavor->HumanPresentableName = rMimeType;
    pNewFlavor->DataType = ::getCppuType( (const OUString*) 0 );

    rL.push_back( pNewFlavor );

    return nMax + SOT_FORMATSTR_ID_USER_END + 1;
}

/*************************************************************************
|*
|*    SotExchange::RegisterFormatName()
|*
|*    Beschreibung      CLIP.SDW
*************************************************************************/
sal_uLong SotExchange::RegisterFormat( const DataFlavor& rFlavor )
{
    sal_uLong nRet = GetFormat( rFlavor );

    if( !nRet )
    {
        tDataFlavorList& rL = InitFormats_Impl();
        nRet = rL.size() + SOT_FORMATSTR_ID_USER_END + 1;
        rL.push_back( new DataFlavor( rFlavor ) );
    }

    return nRet;
}

/*************************************************************************
|*
|*    SotExchange::GetFormatDataFlavor()
|*
*************************************************************************/

bool SotExchange::GetFormatDataFlavor( sal_uLong nFormat, DataFlavor& rFlavor )
{
    bool bRet;

    if( SOT_FORMATSTR_ID_USER_END >= nFormat )
    {
        const DataFlavorRepresentation& rData = FormatArray_Impl::get()[nFormat];
        rFlavor.MimeType = OUString::createFromAscii( rData.pMimeType );
        rFlavor.HumanPresentableName = OUString::createFromAscii( rData.pName );
        rFlavor.DataType = *rData.pType;

        bRet = true;
    }
    else
    {
        tDataFlavorList& rL = InitFormats_Impl();

        nFormat -= SOT_FORMATSTR_ID_USER_END + 1;

        if( rL.size() > nFormat )
        {
            rFlavor = *rL[ nFormat ];
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

OUString SotExchange::GetFormatMimeType( sal_uLong nFormat )
{
    OUString sMimeType;
    if( SOT_FORMATSTR_ID_USER_END >= nFormat )
        sMimeType = OUString::createFromAscii( FormatArray_Impl::get()[nFormat].pMimeType );
    else
    {
        tDataFlavorList& rL = InitFormats_Impl();

        nFormat -= SOT_FORMATSTR_ID_USER_END + 1;

        if( rL.size() > nFormat )
            sMimeType = rL[ nFormat ]->MimeType;
    }

    DBG_ASSERT( !sMimeType.isEmpty(), "SotExchange::GetFormatMimeType(): DataFlavor not initialized" );

    return sMimeType;
}

/*************************************************************************
|*
|*    SotExchange::GetFormatIdFromMimeType( const String& rMimeType )
|*
*************************************************************************/

sal_uLong SotExchange::GetFormatIdFromMimeType( const OUString& rMimeType )
{
    const DataFlavorRepresentation *pFormatArray_Impl = FormatArray_Impl::get();
    sal_uLong i, nMax = SOT_FORMAT_FILE_LIST;
    for( i = SOT_FORMAT_STRING; i <= nMax;  ++i )
        if( rMimeType.equalsAscii( pFormatArray_Impl[ i ].pMimeType ) )
            return i;

    // BM: the chart format 105 ("StarChartDocument 5.0") was written
    // only into 5.1 chart documents - in 5.0 and 5.2 it was 42 ("StarChart 5.0")
    // The registry only contains the entry for the 42 format id.
    nMax = SOT_FORMATSTR_ID_USER_END;
    for( i = SOT_FORMAT_RTF; i <= nMax;  ++i )
        if( rMimeType.equalsAscii( pFormatArray_Impl[ i ].pMimeType ) )
            return ( (i == SOT_FORMATSTR_ID_STARCHARTDOCUMENT_50)
                     ? SOT_FORMATSTR_ID_STARCHART_50
                     : i );

    // dann in der dynamischen Liste
    tDataFlavorList& rL = InitFormats_Impl();

    for( i = 0, nMax = rL.size(); i < nMax; i++ )
    {
        DataFlavor* pFlavor = rL[ i ];
        if( pFlavor && rMimeType == pFlavor->MimeType )
            return i + SOT_FORMATSTR_ID_USER_END + 1;
    }

    return 0;
}

/*************************************************************************
|*
|*    SotExchange::GetFormatName()
|*
|*    Beschreibung      CLIP.SDW
*************************************************************************/
sal_uLong SotExchange::GetFormat( const DataFlavor& rFlavor )
{
    // teste zuerst die Standard - Name
    const OUString& rMimeType = rFlavor.MimeType;

    sal_uLong i, nMax = SOT_FORMAT_FILE_LIST;
    const DataFlavorRepresentation *pFormatArray_Impl = FormatArray_Impl::get();
    for( i = SOT_FORMAT_STRING; i <= nMax;  ++i )
        if( rMimeType.equalsAscii( pFormatArray_Impl[ i ].pMimeType ) )
            return i;

    // BM: the chart format 105 ("StarChartDocument 5.0") was written
    // only into 5.1 chart documents - in 5.0 and 5.2 it was 42 ("StarChart 5.0")
    // The registry only contains the entry for the 42 format id.
    nMax = SOT_FORMATSTR_ID_USER_END;
    for( i = SOT_FORMAT_RTF; i <= nMax;  ++i )
        if( rMimeType.equalsAscii( pFormatArray_Impl[ i ].pMimeType ) )
            return ( (i == SOT_FORMATSTR_ID_STARCHARTDOCUMENT_50)
                     ? SOT_FORMATSTR_ID_STARCHART_50
                     : i );

    // dann in der dynamischen Liste
    tDataFlavorList& rL = InitFormats_Impl();
    for( i = 0, nMax = rL.size(); i < nMax; i++ )
    {
        DataFlavor* pFlavor = rL[ i ];
        if( pFlavor && rMimeType == pFlavor->MimeType )
            return i + SOT_FORMATSTR_ID_USER_END + 1;
    }

    return 0;
}

/*************************************************************************
|*
|*    SotExchange::GetFormatName()
|*
|*    Beschreibung      CLIP.SDW
*************************************************************************/
OUString SotExchange::GetFormatName( sal_uLong nFormat )
{
    DataFlavor  aFlavor;
    OUString      aRet;

    if( GetFormatDataFlavor( nFormat, aFlavor ) )
        aRet = aFlavor.HumanPresentableName;

    return aRet;
}

bool SotExchange::IsInternal( const SvGlobalName& rName )
{
    if ( rName == SvGlobalName(SO3_SW_CLASSID_60) ||
         rName == SvGlobalName(SO3_SC_CLASSID_60) ||
         rName == SvGlobalName(SO3_SIMPRESS_CLASSID_60) ||
         rName == SvGlobalName(SO3_SDRAW_CLASSID_60) ||
         rName == SvGlobalName(SO3_SCH_CLASSID_60) ||
         rName == SvGlobalName(SO3_SM_CLASSID_60) ||
         rName == SvGlobalName(SO3_SWWEB_CLASSID_60) ||
         rName == SvGlobalName(SO3_SWGLOB_CLASSID_60) )
        return true;
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
