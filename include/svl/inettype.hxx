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

#ifndef INCLUDED_SVL_INETTYPE_HXX
#define INCLUDED_SVL_INETTYPE_HXX

#include <svl/svldllapi.h>
#include <tools/inetmime.hxx>

/** Definitions for frequently used media type names.
 */
inline constexpr OUString CONTENT_TYPE_STR_APP_OCTSTREAM = u"application/octet-stream"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_PDF = u"application/pdf"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_RTF = u"application/rtf"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_VND_CALC = u"application/vnd.stardivision.calc"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_VND_CHART
    = u"application/vnd.stardivision.chart"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_VND_DRAW = u"application/vnd.stardivision.draw"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_VND_IMAGE
    = u"application/vnd.stardivision.image"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_VND_IMPRESSPACKED
    = u"application/vnd.stardivision.impress-packed"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_VND_IMPRESS
    = u"application/vnd.stardivision.impress"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_VND_MAIL = u"application/vnd.stardivision.mail"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_VND_MATH = u"application/vnd.stardivision.math"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_VND_NEWS = u"application/vnd.stardivision.news"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_VND_OUTTRAY
    = u"application/vnd.stardivision.outtray"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_VND_TEMPLATE
    = u"application/vnd.stardivision.template"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_VND_WRITER_GLOBAL
    = u"application/vnd.stardivision.writer-global"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_VND_WRITER_WEB
    = u"application/vnd.stardivision.writer-web"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_VND_WRITER
    = u"application/vnd.stardivision.writer"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_FRAMESET = u"application/x-frameset"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_GALLERY_THEME = u"application/x-gallery-theme"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_GALLERY = u"application/x-gallery"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_JAR = u"application/x-jar"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_MACRO = u"application/x-macro"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_MSEXCEL_TEMPL
    = u"application/x-msexcel-template"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_MSEXCEL = u"application/x-msexcel"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_MSPPOINT_TEMPL
    = u"application/x-mspowerpoint-template"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_MSPPOINT = u"application/x-mspowerpoint"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_MSWORD_TEMPL = u"application/x-msword-template"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_MSWORD = u"application/x-msword"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_STARCALC = u"application/x-starcalc"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_STARCHART = u"application/x-starchart;"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_STARDRAW = u"application/x-stardraw"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_STARHELP = u"application/x-starhelp"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_STARIMAGE = u"application/x-starimage"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_STARIMPRESS = u"application/x-starimpress"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_STARMAIL_SDM = u"application/x-starmail-sdm"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_STARMAIL_SMD = u"application/x-starmail-smd"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_STARMATH = u"application/x-starmath"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_STARWRITER_GLOB
    = u"application/x-starwriter-global"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_STARWRITER = u"application/x-starwriter"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_CDE_CALENDAR_APP = u"application/x-sun-ae-file"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_ZIP = u"application/x-zip-compressed"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_AUDIO_AIFF = u"audio/aiff"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_AUDIO_BASIC = u"audio/basic"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_AUDIO_MIDI = u"audio/midi"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_AUDIO_VORBIS = u"audio/vorbis"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_AUDIO_WAV = u"audio/wav"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_AUDIO_WEBM = u"audio/webm"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_X_CNT_FSYSBOX = u".chaos/fsys-box"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_X_CNT_FSYSFOLDER = u".chaos/fsys-folder"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_X_CNT_FSYSSPECIALFOLDER
    = u".chaos/fsys-special-folder"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_IMAGE_GENERIC = u"image/generic"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_IMAGE_GIF = u"image/gif"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_IMAGE_JPEG = u"image/jpeg"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_IMAGE_PCX = u"image/pcx"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_IMAGE_PNG = u"image/png"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_IMAGE_TIFF = u"image/tiff"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_IMAGE_BMP = u"image/x-MS-bmp"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_INET_MSG_RFC822 = u"message/rfc822"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_INET_MULTI_ALTERNATIVE = u"multipart/alternative"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_INET_MULTI_DIGEST = u"multipart/digest"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_INET_MULTI_MIXED = u"multipart/mixed"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_INET_MULTI_PARALLEL = u"multipart/parallel"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_INET_MULTI_RELATED = u"multipart/related"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_TEXT_ICALENDAR = u"text/calendar"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_TEXT_HTML = u"text/html"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_TEXT_PLAIN = u"text/plain"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_TEXT_XMLICALENDAR = u"text/x-icalxml"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_TEXT_URL = u"text/x-url"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_TEXT_VCALENDAR = u"text/x-vCalendar"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_TEXT_VCARD = u"text/x-vCard"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_VIDEO_MSVIDEO = u"video/x-msvideo"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_VIDEO_THEORA = u"video/theora"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_VIDEO_VDO = u"video/vdo"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_VIDEO_WEBM = u"audio/webm"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_X_STARMAIL = u"x-starmail"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_X_VRML = u"x-world/x-vrml"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_VND_SUN_XML_WRITER
    = u"application/vnd.sun.xml.writer"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_VND_SUN_XML_CALC
    = u"application/vnd.sun.xml.calc"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_VND_SUN_XML_IMPRESS
    = u"application/vnd.sun.xml.impress"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_VND_SUN_XML_DRAW
    = u"application/vnd.sun.xml.draw"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_VND_SUN_XML_CHART
    = u"application/vnd.sun.xml.chart"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_VND_SUN_XML_MATH
    = u"application/vnd.sun.xml.math"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_VND_SUN_XML_WRITER_GLOBAL
    = u"application/vnd.sun.xml.writer-global"_ustr;
inline constexpr OUString CONTENT_TYPE_STR_APP_VND_SUN_XML_IMPRESSPACKED
    = u"application/vnd.sun.xml.impress-packed"_ustr;

/** Definitions for matching parts of URIs.
 */
#define INETTYPE_URL_PROT_DATA "data"
#define INETTYPE_URL_PROT_FILE "file"
#define INETTYPE_URL_PROT_HTTP "http"
#define INETTYPE_URL_PROT_HTTPS "https"
#define INETTYPE_URL_PROT_MACRO "macro"
#define INETTYPE_URL_PROT_MAILTO "mailto"
#define INETTYPE_URL_PROT_PRIVATE "private"

enum INetContentType
{
    CONTENT_TYPE_UNKNOWN,
    CONTENT_TYPE_APP_OCTSTREAM,
    CONTENT_TYPE_APP_PDF,
    CONTENT_TYPE_APP_RTF,
    CONTENT_TYPE_APP_MSWORD,
    CONTENT_TYPE_APP_MSWORD_TEMPL,
    CONTENT_TYPE_APP_STARCALC,
    CONTENT_TYPE_APP_STARCHART,
    CONTENT_TYPE_APP_STARDRAW,
    CONTENT_TYPE_APP_STARHELP,
    CONTENT_TYPE_APP_STARIMAGE,
    CONTENT_TYPE_APP_STARIMPRESS,
    CONTENT_TYPE_APP_STARMATH,
    CONTENT_TYPE_APP_STARWRITER,
    CONTENT_TYPE_APP_ZIP,
    CONTENT_TYPE_AUDIO_AIFF,
    CONTENT_TYPE_AUDIO_BASIC,
    CONTENT_TYPE_AUDIO_MIDI,
    CONTENT_TYPE_AUDIO_VORBIS,
    CONTENT_TYPE_AUDIO_WAV,
    CONTENT_TYPE_AUDIO_WEBM,
    CONTENT_TYPE_IMAGE_GIF,
    CONTENT_TYPE_IMAGE_JPEG,
    CONTENT_TYPE_IMAGE_PCX,
    CONTENT_TYPE_IMAGE_PNG,
    CONTENT_TYPE_IMAGE_TIFF,
    CONTENT_TYPE_IMAGE_BMP,
    CONTENT_TYPE_IMAGE_WEBP,
    CONTENT_TYPE_TEXT_HTML,
    CONTENT_TYPE_TEXT_PLAIN,
    CONTENT_TYPE_TEXT_URL,
    CONTENT_TYPE_TEXT_VCARD,
    CONTENT_TYPE_VIDEO_MSVIDEO,
    CONTENT_TYPE_VIDEO_THEORA,
    CONTENT_TYPE_VIDEO_VDO,
    CONTENT_TYPE_VIDEO_WEBM,
    CONTENT_TYPE_X_CNT_FSYSBOX,
    CONTENT_TYPE_X_CNT_FSYSFOLDER,
    CONTENT_TYPE_X_STARMAIL,
    CONTENT_TYPE_X_VRML,
    CONTENT_TYPE_APP_GALLERY,
    CONTENT_TYPE_APP_GALLERY_THEME,
    CONTENT_TYPE_APP_STARWRITER_GLOB,
    CONTENT_TYPE_APP_STARMAIL_SDM,
    CONTENT_TYPE_APP_STARMAIL_SMD,
    CONTENT_TYPE_APP_VND_CALC,
    CONTENT_TYPE_APP_VND_CHART,
    CONTENT_TYPE_APP_VND_DRAW,
    CONTENT_TYPE_APP_VND_IMAGE,
    CONTENT_TYPE_APP_VND_IMPRESS,
    CONTENT_TYPE_APP_VND_MAIL,
    CONTENT_TYPE_APP_VND_MATH,
    CONTENT_TYPE_APP_VND_WRITER,
    CONTENT_TYPE_APP_VND_WRITER_GLOBAL,
    CONTENT_TYPE_APP_VND_WRITER_WEB,
    CONTENT_TYPE_APP_FRAMESET,
    CONTENT_TYPE_APP_MACRO,
    CONTENT_TYPE_X_CNT_FSYSSPECIALFOLDER,
    CONTENT_TYPE_APP_VND_TEMPLATE,
    CONTENT_TYPE_IMAGE_GENERIC,
    CONTENT_TYPE_APP_VND_NEWS,
    CONTENT_TYPE_APP_VND_OUTTRAY,
    CONTENT_TYPE_APP_MSEXCEL,
    CONTENT_TYPE_APP_MSEXCEL_TEMPL,
    CONTENT_TYPE_APP_MSPPOINT,
    CONTENT_TYPE_APP_MSPPOINT_TEMPL,
    CONTENT_TYPE_TEXT_VCALENDAR,
    CONTENT_TYPE_TEXT_ICALENDAR,
    CONTENT_TYPE_TEXT_XMLICALENDAR,
    CONTENT_TYPE_APP_CDE_CALENDAR_APP,
    CONTENT_TYPE_INET_MESSAGE_RFC822,
    CONTENT_TYPE_INET_MULTIPART_ALTERNATIVE,
    CONTENT_TYPE_INET_MULTIPART_DIGEST,
    CONTENT_TYPE_INET_MULTIPART_PARALLEL,
    CONTENT_TYPE_INET_MULTIPART_RELATED,
    CONTENT_TYPE_INET_MULTIPART_MIXED,
    CONTENT_TYPE_APP_VND_IMPRESSPACKED,
    CONTENT_TYPE_APP_JAR,
    CONTENT_TYPE_APP_VND_SUN_XML_WRITER,
    CONTENT_TYPE_APP_VND_SUN_XML_CALC,
    CONTENT_TYPE_APP_VND_SUN_XML_IMPRESS,
    CONTENT_TYPE_APP_VND_SUN_XML_DRAW,
    CONTENT_TYPE_APP_VND_SUN_XML_CHART,
    CONTENT_TYPE_APP_VND_SUN_XML_MATH,
    CONTENT_TYPE_APP_VND_SUN_XML_WRITER_GLOBAL,
    CONTENT_TYPE_APP_VND_SUN_XML_IMPRESSPACKED,
    CONTENT_TYPE_LAST = CONTENT_TYPE_APP_VND_SUN_XML_IMPRESSPACKED
};

class SVL_DLLPUBLIC INetContentTypes
{
public:
    static INetContentType GetContentType(OUString const& rTypeName);

    static const OUString& GetContentType(INetContentType eTypeID);

    static INetContentType GetContentType4Extension(OUString const& rExtension);

    static INetContentType GetContentTypeFromURL(std::u16string_view aURL);

    static bool GetExtensionFromURL(std::u16string_view rURL, OUString& rExtension);

    static bool parse(OUString const& rMediaType, OUString& rType, OUString& rSubType,
                      INetContentTypeParameterList* pParameters = nullptr);
};

#endif // INCLUDED_SVL_INETTYPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
