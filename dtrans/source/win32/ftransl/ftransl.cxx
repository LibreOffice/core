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

#include <osl/diagnose.h>

#include "ftransl.hxx"
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/datatransfer/XMimeContentType.hpp>
#include <com/sun/star/datatransfer/MimeContentTypeFactory.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/supportsservice.hxx>
#include "../misc/ImplHelper.hxx"

#if defined _MSC_VER
#pragma warning(push,1)
#pragma warning(disable:4917)
#endif
#include <shlobj.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

#define IMPL_NAME  "com.sun.star.datatransfer.DataFormatTranslator"

#define CPPUTYPE_SEQSALINT8       cppu::UnoType<Sequence< sal_Int8 >>::get()
#define CPPUTYPE_DEFAULT          CPPUTYPE_SEQSALINT8
#define CPPUTYPE_OUSTR            cppu::UnoType<OUString>::get()
#define CPPUTYPE_SALINT32         cppu::UnoType<sal_Int32>::get()
#define EMPTY_OUSTR               OUString()

const OUString Windows_FormatName ("windows_formatname");
const css::uno::Type CppuType_ByteSequence = cppu::UnoType<css::uno::Sequence<sal_Int8>>::get();
const css::uno::Type CppuType_String       = ::cppu::UnoType<OUString>::get();

using namespace osl;
using namespace cppu;
using namespace std;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::datatransfer;
using namespace com::sun::star::container;

namespace
{
    Sequence< OUString > SAL_CALL DataFormatTranslator_getSupportedServiceNames( )
    {
        Sequence< OUString > aRet { "com.sun.star.datatransfer.DataFormatTranslator" };
        return aRet;
    }
}

struct FormatEntry
{
    FormatEntry(
        const char *mime_content_type,
        const char *human_presentable_name,
        const char *native_format_name,
        CLIPFORMAT std_clipboard_format_id,
        css::uno::Type const & cppu_type
    );

    css::datatransfer::DataFlavor aDataFlavor;
    OUString                      aNativeFormatName;
    sal_Int32                     aStandardFormatId;
};

FormatEntry::FormatEntry(
    const char *mime_content_type,
    const char *human_presentable_name,
    const char *native_format_name,
    CLIPFORMAT std_clipboard_format_id,
    css::uno::Type const & cppu_type)
  : aDataFlavor( OUString::createFromAscii(mime_content_type),  OUString::createFromAscii(human_presentable_name), cppu_type)
{
    if (native_format_name)
        aNativeFormatName =  OUString::createFromAscii(native_format_name);
    else
        aNativeFormatName =  OUString::createFromAscii(human_presentable_name);

    aStandardFormatId = std_clipboard_format_id;
}

// to optimize searching we add all entries with a
// standard clipboard format number first, in the
// table before the entries with CF_INVALID
// if we are searching for a standard clipboard
// format number we can stop if we find the first
// CF_INVALID

static const std::vector< FormatEntry > g_TranslTable {
    //SotClipboardFormatId::DIF
        FormatEntry("application/x-openoffice-dif;windows_formatname=\"DIF\"", "DIF", "DIF", CF_DIF, CPPUTYPE_DEFAULT),
    // SotClipboardFormatId::BITMAP

    // #i124085# CF_DIBV5 disabled, leads to problems at export. To fully support, using
    // an own mime-type may be necessary. I have tried that, but saw no real advantages
    // with different apps when exchanging bitmap-based data. So, disabled for now. At
    // the same time increased png format exchange for better interoperability
    //    FormatEntry("application/x-openoffice-bitmap;windows_formatname=\"Bitmap\"", "Bitmap", "Bitmap", CF_DIBV5, CPPUTYPE_DEFAULT),

        FormatEntry("application/x-openoffice-bitmap;windows_formatname=\"Bitmap\"", "Bitmap", "Bitmap", CF_DIB, CPPUTYPE_DEFAULT),
        FormatEntry("application/x-openoffice-bitmap;windows_formatname=\"Bitmap\"", "Bitmap", "Bitmap", CF_BITMAP, CPPUTYPE_DEFAULT),
    // SotClipboardFormatId::STRING
        FormatEntry("text/plain;charset=utf-16", "Unicode-Text", "", CF_UNICODETEXT, CppuType_String),
    // Format Locale - for internal use
        FormatEntry("application/x-openoffice-locale;windows_formatname=\"Locale\"", "Locale", "Locale", CF_LOCALE, CPPUTYPE_DEFAULT),
    // SOT_FORMAT_WMF
        FormatEntry("application/x-openoffice-wmf;windows_formatname=\"Image WMF\"", "Windows MetaFile", "Image WMF", CF_METAFILEPICT, CPPUTYPE_DEFAULT),
    // SOT_FORMAT_EMF
        FormatEntry("application/x-openoffice-emf;windows_formatname=\"Image EMF\"", "Windows Enhanced MetaFile", "Image EMF", CF_ENHMETAFILE, CPPUTYPE_DEFAULT),
    // SotClipboardFormatId::FILE_LIST
        FormatEntry("application/x-openoffice-filelist;windows_formatname=\"FileList\"", "FileList", "FileList", CF_HDROP, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::SYLK
        FormatEntry("application/x-openoffice-sylk;windows_formatname=\"Sylk\"", "Sylk", "Sylk", CF_SYLK, CPPUTYPE_DEFAULT ),
    // SotClipboardFormatId::GDIMETAFILE
        FormatEntry("application/x-openoffice-gdimetafile;windows_formatname=\"GDIMetaFile\"", "GDIMetaFile", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    // SotClipboardFormatId::PRIVATE
        FormatEntry("application/x-openoffice-private;windows_formatname=\"Private\"", "Private", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    // SotClipboardFormatId::SIMPLE_FILE
        FormatEntry("application/x-openoffice-file;windows_formatname=\"FileName\"", "FileName", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    // SotClipboardFormatId::RTF
        FormatEntry("text/rtf", "Rich Text Format", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    // SotClipboardFormatId::DRAWING
        FormatEntry("application/x-openoffice-drawing;windows_formatname=\"Drawing Format\"", "Drawing Format", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    // SotClipboardFormatId::SVXB
        FormatEntry("application/x-openoffice-svbx;windows_formatname=\"SVXB (StarView Bitmap/Animation)\"", "SVXB (StarView Bitmap/Animation)", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    // SotClipboardFormatId::SVIM
        FormatEntry("application/x-openoffice-svim;windows_formatname=\"SVIM (StarView ImageMap)\"", "SVIM (StarView ImageMap)", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    // SotClipboardFormatId::XFA
        FormatEntry("application/x-openoffice-xfa;windows_formatname=\"XFA (XOutDev FillAttr)\"", "XFA (XOutDev FillAttr)", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    // SotClipboardFormatId::EDITENGINE
        FormatEntry("application/x-openoffice-editengine;windows_formatname=\"EditEngineFormat\"", "EditEngineFormat", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    // SotClipboardFormatId::INTERNALLINK_STATE
        FormatEntry("application/x-openoffice-internallink-state;windows_formatname=\"StatusInfo vom SvxInternalLink\"", "StatusInfo vom SvxInternalLink", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    // SotClipboardFormatId::SOLK
        FormatEntry("application/x-openoffice-solk;windows_formatname=\"SOLK (StarOffice Link)\"", "SOLK (StarOffice Link)", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    // SotClipboardFormatId::NETSCAPE_BOOKMARK
        FormatEntry("application/x-openoffice-netscape-bookmark;windows_formatname=\"Netscape Bookmark\"", "Netscape Bookmark", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    // SotClipboardFormatId::TREELISTBOX
        FormatEntry("application/x-openoffice-treelistbox;windows_formatname=\"SV_LBOX_DD_FORMAT\"", "SV_LBOX_DD_FORMAT", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    // SotClipboardFormatId::NATIVE
        FormatEntry("application/x-openoffice-native;windows_formatname=\"Native\"", "Native", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    // SotClipboardFormatId::OWNERLINK
        FormatEntry("application/x-openoffice-ownerlink;windows_formatname=\"OwnerLink\"", "OwnerLink", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    // SotClipboardFormatId::STARSERVER
        FormatEntry("application/x-openoffice-starserver;windows_formatname=\"StarServerFormat\"", "StarServerFormat", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    // SotClipboardFormatId::STAROBJECT
        FormatEntry("application/x-openoffice-starobject;windows_formatname=\"StarObjectFormat\"", "StarObjectFormat", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    // SotClipboardFormatId::APPLETOBJECT
        FormatEntry("application/x-openoffice-appletobject;windows_formatname=\"Applet Object\"", "Applet Object", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    // SotClipboardFormatId::PLUGIN_OBJECT
        FormatEntry("application/x-openoffice-plugin-object;windows_formatname=\"PlugIn Object\"", "PlugIn Object", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    // SotClipboardFormatId::STARWRITER_30
        FormatEntry("application/x-openoffice-starwriter-30;windows_formatname=\"StarWriter 3.0\"", "StarWriter 3.0", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::STARWRITER_40
        FormatEntry("application/x-openoffice-starwriter-40;windows_formatname=\"StarWriter 4.0\"", "StarWriter 4.0", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::STARWRITER_50
        FormatEntry("application/x-openoffice-starwriter-50;windows_formatname=\"StarWriter 5.0\"", "StarWriter 5.0", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::STARWRITERWEB_40
        FormatEntry("application/x-openoffice-starwriterweb-40;windows_formatname=\"StarWriter/Web 4.0\"", "StarWriter/Web 4.0", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::STARWRITERWEB_50
        FormatEntry("application/x-openoffice-starwriterweb-50;windows_formatname=\"StarWriter/Web 5.0\"", "StarWriter/Web 5.0", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::STARWRITERGLOB_40
        FormatEntry("application/x-openoffice-starwriterglob-40;windows_formatname=\"StarWriter/Global 4.0\"", "StarWriter/Global 4.0", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    // SotClipboardFormatId::STARWRITERGLOB_50
        FormatEntry("application/x-openoffice-starwriterglob-50;windows_formatname=\"StarWriter/Global 5.0\"", "StarWriter/Global 5.0", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::STARDRAW
        FormatEntry("application/x-openoffice-stardraw;windows_formatname=\"StarDrawDocument\"", "StarDrawDocument", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::STARDRAW_40
        FormatEntry("application/x-openoffice-stardraw-40;windows_formatname=\"StarDrawDocument 4.0\"", "StarDrawDocument 4.0", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::STARIMPRESS_50
        FormatEntry("application/x-openoffice-starimpress-50;windows_formatname=\"StarImpress 5.0\"", "StarImpress 5.0", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    // SotClipboardFormatId::STARDRAW_50
        FormatEntry("application/x-openoffice-stardraw-50;windows_formatname=\"StarDraw 5.0\"", "StarDraw 5.0", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::STARCALC
        FormatEntry("application/x-openoffice-starcalc;windows_formatname=\"StarCalcDocument\"", "StarCalcDocument", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::STARCALC_40
        FormatEntry("application/x-openoffice-starcalc-40;windows_formatname=\"StarCalc 4.0\"", "StarCalc 4.0", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    // SotClipboardFormatId::STARCALC_50
        FormatEntry("application/x-openoffice-starcalc-50;windows_formatname=\"StarCalc 5.0\"", "StarCalc 5.0", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    // SotClipboardFormatId::STARCHART
        FormatEntry("application/x-openoffice-starchart;windows_formatname=\"StarChartDocument\"", "StarChartDocument", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    // SotClipboardFormatId::STARCHART_40
        FormatEntry("application/x-openoffice-starchart-40;windows_formatname=\"StarChartDocument 4.0\"", "StarChartDocument 4.0", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    // SotClipboardFormatId::STARCHART_50
        FormatEntry("application/x-openoffice-starchart-50;windows_formatname=\"StarChart 5.0\"", "StarChart 5.0", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::STARIMAGE
        FormatEntry("application/x-openoffice-starimage;windows_formatname=\"StarImageDocument\"", "StarImageDocument", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::STARIMAGE_40
        FormatEntry("application/x-openoffice-starimage-40;windows_formatname=\"StarImageDocument 4.0\"", "StarImageDocument 4.0", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::STARIMAGE_50
        FormatEntry("application/x-openoffice-starimage-50;windows_formatname=\"StarImage 5.0\"",  "StarImage 5.0", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::STARMATH
        FormatEntry("application/x-openoffice-starmath;windows_formatname=\"StarMath\"", "StarMath", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::STARMATH_40
        FormatEntry("application/x-openoffice-starmath-40;windows_formatname=\"StarMathDocument 4.0\"", "StarMathDocument 4.0", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::STARMATH_50
        FormatEntry("application/x-openoffice-starmath-50;windows_formatname=\"StarMath 5.0\"", "StarMath 5.0", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::STAROBJECT_PAINTDOC
        FormatEntry("application/x-openoffice-starobject-paintdoc;windows_formatname=\"StarObjectPaintDocument\"", "StarObjectPaintDocument", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::FILLED_AREA
        FormatEntry("application/x-openoffice-filled-area;windows_formatname=\"FilledArea\"", "FilledArea", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::HTML
        FormatEntry("text/html", "HTML (HyperText Markup Language)", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::HTML_SIMPLE
        FormatEntry("application/x-openoffice-html-simple;windows_formatname=\"HTML Format\"", "HTML Format", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::CHAOS
        FormatEntry("application/x-openoffice-chaos;windows_formatname=\"FORMAT_CHAOS\"", "FORMAT_CHAOS", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::CNT_MSGATTACHFILE
        FormatEntry("application/x-openoffice-msgattachfile;windows_formatname=\"CNT_MSGATTACHFILE_FORMAT\"", "CNT_MSGATTACHFILE_FORMAT", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::BIFF_5
        FormatEntry("application/x-openoffice-biff5;windows_formatname=\"Biff5\"", "Biff5", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::BIFF__5
        FormatEntry("application/x-openoffice-biff-5;windows_formatname=\"Biff 5\"", "Biff 5", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::BIFF_8
        FormatEntry("application/x-openoffice-biff-8;windows_formatname=\"Biff8\"", "Biff8", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::SYLK_BIGCAPS
        FormatEntry("application/x-openoffice-sylk-bigcaps;windows_formatname=\"SYLK\"", "SYLK", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::LINK
        FormatEntry("application/x-openoffice-link;windows_formatname=\"Link\"", "Link", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::STARDRAW_TABBAR
        FormatEntry("application/x-openoffice-stardraw-tabbar;windows_formatname=\"StarDraw TabBar\"", "StarDraw TabBar", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::SONLK
        FormatEntry("application/x-openoffice-sonlk;windows_formatname=\"SONLK (StarOffice Navi Link)\"", "SONLK (StarOffice Navi Link)", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::MSWORD_DOC
        FormatEntry("application/msword", "MSWordDoc", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::STAR_FRAMESET_DOC
        FormatEntry("application/x-openoffice-star-frameset-doc;windows_formatname=\"StarFrameSetDocument\"", "StarFrameSetDocument", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::OFFICE_DOC
        FormatEntry("application/x-openoffice-office-doc;windows_formatname=\"OfficeDocument\"", "OfficeDocument", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::NOTES_DOCINFO
        FormatEntry("application/x-openoffice-notes-docinfo;windows_formatname=\"NotesDocInfo\"", "NotesDocInfo", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::NOTES_HNOTE
        FormatEntry("application/x-openoffice-notes-hnote;windows_formatname=\"NoteshNote\"", "NoteshNote", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::NOTES_NATIVE
        FormatEntry("application/x-openoffice-notes-native;windows_formatname=\"Native\"", "Native", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::SFX_DOC
        FormatEntry("application/x-openoffice-sfx-doc;windows_formatname=\"SfxDocument\"", "SfxDocument", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::EVDF
        FormatEntry("application/x-openoffice-evdf;windows_formatname=\"EVDF (Explorer View Dummy Format)\"", "EVDF (Explorer View Dummy Format)", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::ESDF
        FormatEntry("application/x-openoffice-esdf;windows_formatname=\"ESDF (Explorer Search Dummy Format)\"", "ESDF (Explorer Search Dummy Format)", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::IDF
        FormatEntry("application/x-openoffice-idf;windows_formatname=\"IDF (Iconview Dummy Format)\"", "IDF (Iconview Dummy Format)", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::EFTP
        FormatEntry("application/x-openoffice-eftp;windows_formatname=\"EFTP (Explorer Ftp File)\"", "EFTP (Explorer Ftp File)", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::EFD
        FormatEntry("application/x-openoffice-efd;windows_formatname=\"EFD (Explorer Ftp Dir)\"", "EFD (Explorer Ftp Dir)", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::SVX_FORMFIELDEXCH
        FormatEntry("application/x-openoffice-svx-formfieldexch;windows_formatname=\"SvxFormFieldExch\"", "SvxFormFieldExch", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::EXTENDED_TABBAR
        FormatEntry("application/x-openoffice-extended-tabbar;windows_formatname=\"ExtendedTabBar\"", "ExtendedTabBar", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::SBA_DATAEXCHANGE
        FormatEntry("application/x-openoffice-sba-dataexchange;windows_formatname=\"SBA-DATAFORMAT\"", "SBA-DATAFORMAT", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::SBA_FIELDDATAEXCHANGE
        FormatEntry("application/x-openoffice-sba-fielddataexchange;windows_formatname=\"SBA-FIELDFORMAT\"", "SBA-FIELDFORMAT", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::SBA_PRIVATE_URL
        FormatEntry("application/x-openoffice-sba-private-url;windows_formatname=\"SBA-PRIVATEURLFORMAT\"", "SBA-PRIVATEURLFORMAT", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::SBA_TABED
        FormatEntry("application/x-openoffice-sba-tabed;windows_formatname=\"Tabed\"", "Tabed", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::SBA_TABID
        FormatEntry("application/x-openoffice-sba-tabid;windows_formatname=\"Tabid\"", "Tabid", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::SBA_JOIN
        FormatEntry("application/x-openoffice-sba-join;windows_formatname=\"SBA-JOINFORMAT\"", "SBA-JOINFORMAT", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::OBJECTDESCRIPTOR
        FormatEntry("application/x-openoffice-objectdescriptor-xml;windows_formatname=\"Star Object Descriptor (XML)\"", "Star Object Descriptor (XML)", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::LINKSRCDESCRIPTOR
        FormatEntry("application/x-openoffice-linksrcdescriptor-xml;windows_formatname=\"Star Link Source Descriptor (XML)\"", "Star Link Source Descriptor (XML)", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::EMBED_SOURCE
        FormatEntry("application/x-openoffice-embed-source-xml;windows_formatname=\"Star Embed Source (XML)\"", "Star Embed Source (XML)", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::LINK_SOURCE
        FormatEntry("application/x-openoffice-link-source-xml;windows_formatname=\"Star Link Source (XML)\"", "Star Link Source (XML)", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::EMBEDDED_OBJ
        FormatEntry("application/x-openoffice-embedded-obj-xml;windows_formatname=\"Star Embedded Object (XML)\"", "Star Embedded Object (XML)", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::FILECONTENT
        FormatEntry("application/x-openoffice-filecontent;windows_formatname=\"" CFSTR_FILECONTENTS "\"", CFSTR_FILECONTENTS, nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::FILEGRPDESCRIPTOR
        FormatEntry("application/x-openoffice-filegrpdescriptor;windows_formatname=\"" CFSTR_FILEDESCRIPTOR "\"", CFSTR_FILEDESCRIPTOR, nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::FILENAME
        FormatEntry("application/x-openoffice-filename;windows_formatname=\"" CFSTR_FILENAME "\"", CFSTR_FILENAME, nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::SD_OLE
        FormatEntry("application/x-openoffice-sd-ole;windows_formatname=\"SD-OLE\"", "SD-OLE", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::EMBEDDED_OBJ_OLE
        FormatEntry("application/x-openoffice-embedded-obj-ole;windows_formatname=\"Embedded Object\"", "Embedded Object", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::EMBED_SOURCE_OLE
        FormatEntry("application/x-openoffice-embed-source-ole;windows_formatname=\"Embed Source\"", "Embed Source", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::OBJECTDESCRIPTOR_OLE
        FormatEntry("application/x-openoffice-objectdescriptor-ole;windows_formatname=\"Object Descriptor\"", "Object Descriptor", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::LINKSRCDESCRIPTOR_OLE
        FormatEntry("application/x-openoffice-linkdescriptor-ole;windows_formatname=\"Link Source Descriptor\"", "Link Source Descriptor", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::LINK_SOURCE_OLE
        FormatEntry("application/x-openoffice-link-source-ole;windows_formatname=\"Link Source\"", "Link Source", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::SBA_CTRLDATAEXCHANGE
        FormatEntry("application/x-openoffice-sba-ctrldataexchange;windows_formatname=\"SBA-CTRLFORMAT\"", "SBA-CTRLFORMAT", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::OUTPLACE_OBJ
        FormatEntry("application/x-openoffice-outplace-obj;windows_formatname=\"OutPlace Object\"", "OutPlace Object", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::CNT_OWN_CLIP
        FormatEntry("application/x-openoffice-cnt-own-clip;windows_formatname=\"CntOwnClipboard\"", "CntOwnClipboard", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::INET_IMAGE
        FormatEntry("application/x-openoffice-inet-image;windows_formatname=\"SO-INet-Image\"", "SO-INet-Image", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::NETSCAPE_IMAGE
        FormatEntry("application/x-openoffice-netscape-image;windows_formatname=\"Netscape Image Format\"", "Netscape Image Format", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::SBA_FORMEXCHANGE
        FormatEntry("application/x-openoffice-sba-formexchange;windows_formatname=\"SBA_FORMEXCHANGE\"", "SBA_FORMEXCHANGE", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),  //SotClipboardFormatId::SBA_REPORTEXCHANGE
        FormatEntry("application/x-openoffice-sba-reportexchange;windows_formatname=\"SBA_REPORTEXCHANGE\"", "SBA_REPORTEXCHANGE", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::UNIFORMRESOURCELOCATOR
        FormatEntry("application/x-openoffice-uniformresourcelocator;windows_formatname=\"UniformResourceLocator\"", "UniformResourceLocator", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::STARCHARTDOCUMENT_50
        FormatEntry("application/x-openoffice-starchartdocument-50;windows_formatname=\"StarChartDocument 5.0\"", "StarChartDocument 5.0", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::GRAPHOBJ
        FormatEntry("application/x-openoffice-graphobj;windows_formatname=\"Graphic Object\"", "Graphic Object", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::STARWRITER_60
        FormatEntry("application/vnd.sun.xml.writer", "Writer 6.0", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::STARWRITERWEB_60
        FormatEntry("application/vnd.sun.xml.writer.web", "Writer/Web 6.0", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::STARWRITERGLOB_60
        FormatEntry("application/vnd.sun.xml.writer.global", "Writer/Global 6.0", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::STARWDRAW_60
        FormatEntry("application/vnd.sun.xml.draw", "Draw 6.0", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::STARIMPRESS_60
        FormatEntry("application/vnd.sun.xml.impress", "Impress 6.0", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::STARCALC_60
        FormatEntry("application/vnd.sun.xml.calc", "Calc 6.0", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::STARCHART_60
        FormatEntry("application/vnd.sun.xml.chart", "Chart 6.0", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::STARMATH_60
        FormatEntry("application/vnd.sun.xml.math", "Math 6.0", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::DIALOG_60
        FormatEntry("application/vnd.sun.xml.dialog", "Dialog 6.0", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::BMP
        FormatEntry("image/bmp", "Windows Bitmap", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::PNG
        FormatEntry("image/png", "PNG", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::MATHML
        FormatEntry("application/mathml+xml", "MathML", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::DUMMY3
        FormatEntry("application/x-openoffice-dummy3;windows_formatname=\"SO_DUMMYFORMAT_3\"", "SO_DUMMYFORMAT_3", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    //SotClipboardFormatId::DUMMY4
        FormatEntry("application/x-openoffice-dummy4;windows_formatname=\"SO_DUMMYFORMAT_4\"", "SO_DUMMYFORMAT_4", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    // SotClipboardFormatId::RICHTEXT
        FormatEntry("text/richtext", "Richtext Format", nullptr, CF_INVALID, CPPUTYPE_DEFAULT),
    };

namespace {

void SAL_CALL findDataFlavorForStandardFormatId( sal_Int32 aStandardFormatId, DataFlavor& aDataFlavor )
{
    /*
        we break the for loop if we find the first CF_INVALID
        because in the translation table the entries with a
        standard clipboard format id appear before the other
        entries with CF_INVALID
    */
    vector< FormatEntry >::const_iterator citer_end = g_TranslTable.end( );
    for ( vector< FormatEntry >::const_iterator citer = g_TranslTable.begin( ); citer != citer_end; ++citer )
    {
        sal_Int32 stdId = citer->aStandardFormatId;
        if ( aStandardFormatId == stdId )
        {
            aDataFlavor = citer->aDataFlavor;
            break;
        }
        else if ( stdId == CF_INVALID )
            break;
    }
}

void SAL_CALL findDataFlavorForNativeFormatName( const OUString& aNativeFormatName, DataFlavor& aDataFlavor )
{
    vector< FormatEntry >::const_iterator citer_end = g_TranslTable.end( );
    for ( vector< FormatEntry >::const_iterator citer = g_TranslTable.begin( );
          citer != citer_end;
          ++citer )
    {
        if ( aNativeFormatName.equalsIgnoreAsciiCase( citer->aNativeFormatName ) )
        {
            aDataFlavor = citer->aDataFlavor;
            break;
        }
    }
}

void SAL_CALL findStandardFormatIdForCharset( const OUString& aCharset, Any& aAny )
{
    if ( aCharset.equalsIgnoreAsciiCase( "utf-16" ) )
        aAny <<= static_cast< sal_Int32 >( CF_UNICODETEXT );
    else
    {
        sal_Int32 wincp = getWinCPFromMimeCharset( aCharset );
        if ( IsOEMCP ( wincp ) )
            aAny <<= static_cast< sal_Int32 >( CF_OEMTEXT );
    }
}

void SAL_CALL setStandardFormatIdForNativeFormatName( const OUString& aNativeFormatName, Any& aAny )
{
    vector< FormatEntry >::const_iterator citer_end = g_TranslTable.end( );
    for ( vector< FormatEntry >::const_iterator citer = g_TranslTable.begin( ); citer != citer_end; ++citer )
    {
        if ( aNativeFormatName.equalsIgnoreAsciiCase( citer->aNativeFormatName ) &&
             (CF_INVALID != citer->aStandardFormatId) )
        {
            aAny <<= citer->aStandardFormatId;
            break;
        }
    }
}

void SAL_CALL findStdFormatIdOrNativeFormatNameForFullMediaType(
    const Reference< XMimeContentTypeFactory >& aRefXMimeFactory,
    const OUString& aFullMediaType,
    Any& aAny )
{
    vector< FormatEntry >::const_iterator citer_end = g_TranslTable.end( );
    for ( vector< FormatEntry >::const_iterator citer = g_TranslTable.begin( ); citer != citer_end; ++citer )
    {
        Reference< XMimeContentType >
        refXMime( aRefXMimeFactory->createMimeContentType( citer->aDataFlavor.MimeType ) );
        if ( aFullMediaType.equalsIgnoreAsciiCase( refXMime->getFullMediaType( ) ) )
        {
            sal_Int32 cf = citer->aStandardFormatId;
            if ( CF_INVALID != cf )
                aAny <<= cf;
            else
            {
                OSL_ENSURE( citer->aNativeFormatName.getLength( ),
                    "Invalid standard format id and empty native format name in translation table" );
                aAny <<= citer->aNativeFormatName;
            }
            break;
        }
    }
}

inline bool isTextPlainMediaType( const OUString& fullMediaType )
{
    return fullMediaType.equalsIgnoreAsciiCase("text/plain");
}

DataFlavor SAL_CALL mkDataFlv(const OUString& cnttype, const OUString& hpname, Type dtype)
{
    DataFlavor dflv;
    dflv.MimeType             = cnttype;
    dflv.HumanPresentableName = hpname;
    dflv.DataType             = dtype;
    return dflv;
}

}

CDataFormatTranslatorUNO::CDataFormatTranslatorUNO( const Reference< XComponentContext >& rxContext ) :
    m_xContext( rxContext )
{
}

Any SAL_CALL CDataFormatTranslatorUNO::getSystemDataTypeFromDataFlavor( const DataFlavor& aDataFlavor )
{
    Any aAny;

    try
    {
        Reference< XMimeContentTypeFactory > refXMimeCntFactory = MimeContentTypeFactory::create( m_xContext );

        Reference< XMimeContentType >
            refXMimeCntType( refXMimeCntFactory->createMimeContentType( aDataFlavor.MimeType ) );

        OUString fullMediaType = refXMimeCntType->getFullMediaType( );
        if ( isTextPlainMediaType( fullMediaType ) )
        {
            // default is CF_TEXT
            aAny <<= static_cast< sal_Int32 >( CF_TEXT );

            if ( refXMimeCntType->hasParameter( "charset" ) )
            {
                // but maybe it is unicode text or oem text
                OUString charset = refXMimeCntType->getParameterValue( "charset" );
                findStandardFormatIdForCharset( charset, aAny );
            }
        }
        else
        {
            if (refXMimeCntType->hasParameter(Windows_FormatName))
            {
                OUString winfmtname = refXMimeCntType->getParameterValue(Windows_FormatName);
                aAny <<= winfmtname;

                setStandardFormatIdForNativeFormatName( winfmtname, aAny );
            }
            else
                findStdFormatIdOrNativeFormatNameForFullMediaType( refXMimeCntFactory, fullMediaType, aAny );
        }
    }
    catch( IllegalArgumentException& )
    {
        OSL_FAIL( "Invalid content-type detected!" );
    }
    catch( NoSuchElementException& )
    {
        OSL_FAIL( "Illegal content-type parameter" );
    }
    catch( ... )
    {
        OSL_FAIL( "Unexpected error" );
        throw;
    }

    return aAny;
}

DataFlavor SAL_CALL CDataFormatTranslatorUNO::getDataFlavorFromSystemDataType( const Any& aSysDataType )
{
    OSL_PRECOND( aSysDataType.hasValue( ), "Empty system data type delivered" );

    DataFlavor aFlavor = mkDataFlv( EMPTY_OUSTR, EMPTY_OUSTR, CPPUTYPE_SEQSALINT8 );

    if ( aSysDataType.getValueType( ) == CPPUTYPE_SALINT32 )
    {
        sal_Int32 clipformat = CF_INVALID;
        aSysDataType >>= clipformat;
        if ( CF_INVALID != clipformat )
            findDataFlavorForStandardFormatId( clipformat, aFlavor );
    }
    else if ( aSysDataType.getValueType( ) == CPPUTYPE_OUSTR )
    {
        OUString nativeFormatName;
        aSysDataType >>= nativeFormatName;

        findDataFlavorForNativeFormatName( nativeFormatName, aFlavor );
    }
    else
        OSL_FAIL( "Invalid data type received" );

    return aFlavor;
}

// XServiceInfo

OUString SAL_CALL CDataFormatTranslatorUNO::getImplementationName(  )
{
    return OUString( IMPL_NAME );
}

sal_Bool SAL_CALL CDataFormatTranslatorUNO::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL CDataFormatTranslatorUNO::getSupportedServiceNames( )
{
    return DataFormatTranslator_getSupportedServiceNames( );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
