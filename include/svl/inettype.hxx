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

#ifndef _INETTYPE_HXX
#define _INETTYPE_HXX

#include "svl/svldllapi.h"
#include <tools/inetmime.hxx>
#include <i18nlangtag/languagetag.hxx>

//============================================================================
/** Definitions for frequently used media type names.
 */
#define CONTENT_TYPE_STR_APP_OCTSTREAM "application/octet-stream"
#define CONTENT_TYPE_STR_APP_PDF "application/pdf"
#define CONTENT_TYPE_STR_APP_RTF "application/rtf"
#define CONTENT_TYPE_STR_APP_VND_CALC "application/vnd.stardivision.calc"
#define CONTENT_TYPE_STR_APP_VND_CHART "application/vnd.stardivision.chart"
#define CONTENT_TYPE_STR_APP_VND_DRAW "application/vnd.stardivision.draw"
#define CONTENT_TYPE_STR_APP_VND_IMAGE "application/vnd.stardivision.image"
#define CONTENT_TYPE_STR_APP_VND_IMPRESSPACKED "application/vnd.stardivision.impress-packed"
#define CONTENT_TYPE_STR_APP_VND_IMPRESS "application/vnd.stardivision.impress"
#define CONTENT_TYPE_STR_APP_VND_MAIL "application/vnd.stardivision.mail"
#define CONTENT_TYPE_STR_APP_VND_MATH "application/vnd.stardivision.math"
#define CONTENT_TYPE_STR_APP_VND_NEWS "application/vnd.stardivision.news"
#define CONTENT_TYPE_STR_APP_VND_OUTTRAY "application/vnd.stardivision.outtray"
#define CONTENT_TYPE_STR_APP_VND_TEMPLATE "application/vnd.stardivision.template"
#define CONTENT_TYPE_STR_APP_VND_WRITER_GLOBAL "application/vnd.stardivision.writer-global"
#define CONTENT_TYPE_STR_APP_VND_WRITER_WEB "application/vnd.stardivision.writer-web"
#define CONTENT_TYPE_STR_APP_VND_WRITER "application/vnd.stardivision.writer"
#define CONTENT_TYPE_STR_APP_FRAMESET "application/x-frameset"
#define CONTENT_TYPE_STR_APP_GALLERY_THEME "application/x-gallery-theme"
#define CONTENT_TYPE_STR_APP_GALLERY "application/x-gallery"
#define CONTENT_TYPE_STR_APP_JAR "application/x-jar"
#define CONTENT_TYPE_STR_APP_MACRO "application/x-macro"
#define CONTENT_TYPE_STR_APP_MSEXCEL_TEMPL "application/x-msexcel-template"
#define CONTENT_TYPE_STR_APP_MSEXCEL "application/x-msexcel"
#define CONTENT_TYPE_STR_APP_MSPPOINT_TEMPL "application/x-mspowerpoint-template"
#define CONTENT_TYPE_STR_APP_MSPPOINT "application/x-mspowerpoint"
#define CONTENT_TYPE_STR_APP_MSWORD_TEMPL "application/x-msword-template"
#define CONTENT_TYPE_STR_APP_MSWORD "application/x-msword"
#define CONTENT_TYPE_STR_APP_STARCALC "application/x-starcalc"
#define CONTENT_TYPE_STR_APP_STARCHART "application/x-starchart"
#define CONTENT_TYPE_STR_APP_STARDRAW "application/x-stardraw"
#define CONTENT_TYPE_STR_APP_STARHELP "application/x-starhelp"
#define CONTENT_TYPE_STR_APP_STARIMAGE "application/x-starimage"
#define CONTENT_TYPE_STR_APP_STARIMPRESS "application/x-starimpress"
#define CONTENT_TYPE_STR_APP_STARMAIL_SDM "application/x-starmail-sdm"
#define CONTENT_TYPE_STR_APP_STARMAIL_SMD "application/x-starmail-smd"
#define CONTENT_TYPE_STR_APP_STARMATH "application/x-starmath"
#define CONTENT_TYPE_STR_APP_STARWRITER_GLOB "application/x-starwriter-global"
#define CONTENT_TYPE_STR_APP_STARWRITER "application/x-starwriter"
#define CONTENT_TYPE_STR_APP_CDE_CALENDAR_APP "application/x-sun-ae-file"
#define CONTENT_TYPE_STR_APP_ZIP "application/x-zip-compressed"
#define CONTENT_TYPE_STR_AUDIO_AIFF "audio/aiff"
#define CONTENT_TYPE_STR_AUDIO_BASIC "audio/basic"
#define CONTENT_TYPE_STR_AUDIO_MIDI "audio/midi"
#define CONTENT_TYPE_STR_AUDIO_VORBIS "audio/vorbis"
#define CONTENT_TYPE_STR_AUDIO_WAV "audio/wav"
#define CONTENT_TYPE_STR_AUDIO_WEBM "audio/webm"
#define CONTENT_TYPE_STR_X_CNT_FSYSBOX ".chaos/fsys-box"
#define CONTENT_TYPE_STR_X_CNT_FSYSFOLDER ".chaos/fsys-folder"
#define CONTENT_TYPE_STR_X_CNT_FSYSSPECIALFOLDER ".chaos/fsys-special-folder"
#define CONTENT_TYPE_STR_IMAGE_GENERIC "image/generic"
#define CONTENT_TYPE_STR_IMAGE_GIF "image/gif"
#define CONTENT_TYPE_STR_IMAGE_JPEG "image/jpeg"
#define CONTENT_TYPE_STR_IMAGE_PCX "image/pcx"
#define CONTENT_TYPE_STR_IMAGE_PNG "image/png"
#define CONTENT_TYPE_STR_IMAGE_TIFF "image/tiff"
#define CONTENT_TYPE_STR_IMAGE_BMP "image/x-MS-bmp"
#define CONTENT_TYPE_STR_INET_MSG_RFC822 "message/rfc822"
#define CONTENT_TYPE_STR_INET_MULTI_ALTERNATIVE "multipart/alternative"
#define CONTENT_TYPE_STR_INET_MULTI_DIGEST "multipart/digest"
#define CONTENT_TYPE_STR_INET_MULTI_MIXED "multipart/mixed"
#define CONTENT_TYPE_STR_INET_MULTI_PARALLEL "multipart/parallel"
#define CONTENT_TYPE_STR_INET_MULTI_RELATED "multipart/related"
#define CONTENT_TYPE_STR_TEXT_ICALENDAR "text/calendar"
#define CONTENT_TYPE_STR_TEXT_HTML "text/html"
#define CONTENT_TYPE_STR_TEXT_PLAIN "text/plain"
#define CONTENT_TYPE_STR_TEXT_XMLICALENDAR "text/x-icalxml"
#define CONTENT_TYPE_STR_TEXT_URL "text/x-url"
#define CONTENT_TYPE_STR_TEXT_VCALENDAR "text/x-vCalendar"
#define CONTENT_TYPE_STR_TEXT_VCARD "text/x-vCard"
#define CONTENT_TYPE_STR_VIDEO_MSVIDEO "video/x-msvideo"
#define CONTENT_TYPE_STR_VIDEO_THEORA "video/theora"
#define CONTENT_TYPE_STR_VIDEO_VDO "video/vdo"
#define CONTENT_TYPE_STR_VIDEO_WEBM "audio/webm"
#define CONTENT_TYPE_STR_X_STARMAIL "x-starmail"
#define CONTENT_TYPE_STR_X_VRML "x-world/x-vrml"
#define CONTENT_TYPE_STR_APP_VND_SUN_XML_WRITER "application/vnd.sun.xml.writer"
#define CONTENT_TYPE_STR_APP_VND_SUN_XML_CALC "application/vnd.sun.xml.calc"
#define CONTENT_TYPE_STR_APP_VND_SUN_XML_IMPRESS "application/vnd.sun.xml.impress"
#define CONTENT_TYPE_STR_APP_VND_SUN_XML_DRAW "application/vnd.sun.xml.draw"
#define CONTENT_TYPE_STR_APP_VND_SUN_XML_CHART "application/vnd.sun.xml.chart"
#define CONTENT_TYPE_STR_APP_VND_SUN_XML_MATH "application/vnd.sun.xml.math"
#define CONTENT_TYPE_STR_APP_VND_SUN_XML_WRITER_GLOBAL "application/vnd.sun.xml.writer-global"
#define CONTENT_TYPE_STR_APP_VND_SUN_XML_IMPRESSPACKED "application/vnd.sun.xml.impress-packed"

//============================================================================
/** Definitions for matching parts of URIs.
 */
#define INETTYPE_URL_PROT_DATA "data"
#define INETTYPE_URL_PROT_FILE "file"
#define INETTYPE_URL_PROT_HTTP "http"
#define INETTYPE_URL_PROT_HTTPS "https"
#define INETTYPE_URL_PROT_MACRO "macro"
#define INETTYPE_URL_PROT_MAILTO "mailto"
#define INETTYPE_URL_PROT_PRIVATE "private"

#define INETTYPE_URL_SUB_FACTORY "factory"
#define INETTYPE_URL_SUB_HELPID "helpid"

#define INETTYPE_URL_SSUB_FRAMESET "frameset"
#define INETTYPE_URL_SSUB_SCALC "scalc"
#define INETTYPE_URL_SSUB_SCHART "schart"
#define INETTYPE_URL_SSUB_SDRAW "sdraw"
#define INETTYPE_URL_SSUB_SIMAGE "simage"
#define INETTYPE_URL_SSUB_SIMPRESS "simpress"
#define INETTYPE_URL_SSUB_SMATH "smath"
#define INETTYPE_URL_SSUB_SWRITER "swriter"

#define INETTYPE_URL_SSSUB_GLOB "GlobalDocument"
#define INETTYPE_URL_SSSUB_WEB "web"

//============================================================================
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

//============================================================================
class SVL_DLLPUBLIC INetContentTypes
{
public:
    static INetContentType RegisterContentType(OUString const & rTypeName,
                                               OUString const & rPresentation,
                                               OUString const * pExtension = 0,
                                               OUString const * pSystemFileType = 0);

    static INetContentType GetContentType(OUString const & rTypeName);

    static OUString GetContentType(INetContentType eTypeID);

    static OUString GetPresentation(INetContentType eTypeID,
                                    const LanguageTag& aLocale);

    static INetContentType GetContentType4Extension(OUString const & rExtension);

    static INetContentType GetContentTypeFromURL(OUString const & rURL);

    static bool GetExtensionFromURL(OUString const & rURL, OUString & rExtension);

    /** Parse the body of an RFC 2045 Content-Type header field.

        @param rMediaType  The body of the Content-Type header field.  It must
        be of the form

          token "/" token *(";" token "=" (token / quoted-string))

        with intervening linear white space and comments (cf. RFCs 822, 2045).
        The RFC 2231 extension are supported.  The encoding of rMediaType
        should be US-ASCII, but any Unicode values in the range U+0080..U+FFFF
        are interpretet 'as appropriate.'

        @param rType  Returns the type (the first of the above tokens), in US-
        ASCII encoding and converted to lower case.

        @param rSubType  Returns the sub type (the second of the above
        tokens), in US-ASCII encoding and converted to lower case.

        @param rParameters  If not null, returns the parameters as a list of
        INetContentTypeParameters (the attributes are in US-ASCII encoding and
        converted to lower case, the values are in Unicode encoding).  If
        null, only the syntax of the parameters is checked, but they are not
        returned.

        @return  True if the syntax of the field body is correct.  If false is
        returned, none of the output parameters will be modified!
     */
    static bool parse(OUString const & rMediaType, OUString & rType,
                      OUString & rSubType,
                      INetContentTypeParameterList * pParameters = 0);
};

#endif // _INETTYPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
