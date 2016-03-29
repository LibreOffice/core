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

#include <utility>
#include <map>

#include <tools/wldcrd.hxx>
#include <tools/inetmime.hxx>
#include <rtl/instance.hxx>
#include <osl/diagnose.h>
#include <svl/inettype.hxx>
#include <svl/svl.hrc>

#include "getstringresource.hxx"

namespace
{

struct MediaTypeEntry
{
    sal_Char const * m_pTypeName;
    INetContentType m_eTypeID;
};

struct TypeIDMapEntry
{
    OUString m_aTypeName;
    OUString m_aPresentation;
    OUString m_aSystemFileType;
};

struct TypeNameMapEntry
{
    OUString m_aExtension;
    INetContentType m_eTypeID;

    TypeNameMapEntry(INetContentType const eTypeID, OUString const*const pExtension)
        : m_aExtension((pExtension) ? *pExtension : OUString())
        , m_eTypeID(eTypeID)
    {}
};

struct ExtensionMapEntry
{
    INetContentType m_eTypeID;

    explicit ExtensionMapEntry(INetContentType const eTypeID)
        : m_eTypeID(eTypeID) {}
};

class Registration
{
    typedef std::map<OUString, TypeNameMapEntry>  TypeNameMap;
    typedef std::map<OUString, ExtensionMapEntry> ExtensionMap;
    typedef std::map<INetContentType, TypeIDMapEntry*>   TypeIDMap;

    TypeIDMap    m_aTypeIDMap;    // map ContentType to TypeID
    TypeNameMap  m_aTypeNameMap;  // map TypeName to TypeID, Extension
    ExtensionMap m_aExtensionMap; // map Extension to TypeID
    sal_uInt32 m_nNextDynamicID;

public:
    Registration(): m_nNextDynamicID(CONTENT_TYPE_LAST + 1) {}

    ~Registration();
public:
    static inline TypeIDMapEntry * getEntry(INetContentType eTypeID);

    static TypeNameMapEntry * getExtensionEntry(OUString const & rTypeName);

    static INetContentType RegisterContentType(OUString const & rTypeName,
                                               OUString const & rPresentation,
                                               OUString const * pExtension,
                                               OUString const * pSystemFileType);

    static INetContentType GetContentType(OUString const & rTypeName);

    static OUString GetContentType(INetContentType eTypeID);

    static OUString GetPresentation(INetContentType eTypeID);

    static INetContentType GetContentType4Extension(OUString const & rExtension);

};

namespace
{
    struct theRegistration
        : public rtl::Static< Registration, theRegistration > {};
}

// static
inline TypeIDMapEntry * Registration::getEntry(INetContentType eTypeID)
{
    Registration &rRegistration = theRegistration::get();

    TypeIDMap::const_iterator it = rRegistration.m_aTypeIDMap.find( eTypeID );
    if( it != rRegistration.m_aTypeIDMap.end() )
        return it->second;
    else
        return nullptr;
}

MediaTypeEntry const * seekEntry(OUString const & rTypeName,
                                 MediaTypeEntry const * pMap, sal_Size nSize);

/** A mapping from type names to type ids and extensions.  Sorted by type
    name.
 */
MediaTypeEntry const aStaticTypeNameMap[CONTENT_TYPE_LAST + 1]
    = { { " ", CONTENT_TYPE_UNKNOWN },
        { CONTENT_TYPE_STR_X_CNT_FSYSBOX, CONTENT_TYPE_X_CNT_FSYSBOX },
        { CONTENT_TYPE_STR_X_CNT_FSYSFOLDER, CONTENT_TYPE_X_CNT_FSYSFOLDER },
        { CONTENT_TYPE_STR_X_CNT_FSYSSPECIALFOLDER, CONTENT_TYPE_X_CNT_FSYSSPECIALFOLDER },
        { CONTENT_TYPE_STR_APP_OCTSTREAM, CONTENT_TYPE_APP_OCTSTREAM },
        { CONTENT_TYPE_STR_APP_PDF, CONTENT_TYPE_APP_PDF },
        { CONTENT_TYPE_STR_APP_RTF, CONTENT_TYPE_APP_RTF },
        { CONTENT_TYPE_STR_APP_VND_CALC, CONTENT_TYPE_APP_VND_CALC },
        { CONTENT_TYPE_STR_APP_VND_CHART, CONTENT_TYPE_APP_VND_CHART },
        { CONTENT_TYPE_STR_APP_VND_DRAW, CONTENT_TYPE_APP_VND_DRAW },
        { CONTENT_TYPE_STR_APP_VND_IMAGE, CONTENT_TYPE_APP_VND_IMAGE },
        { CONTENT_TYPE_STR_APP_VND_IMPRESS, CONTENT_TYPE_APP_VND_IMPRESS },
        { CONTENT_TYPE_STR_APP_VND_IMPRESSPACKED, CONTENT_TYPE_APP_VND_IMPRESSPACKED },
        { CONTENT_TYPE_STR_APP_VND_MAIL, CONTENT_TYPE_APP_VND_MAIL },
        { CONTENT_TYPE_STR_APP_VND_MATH, CONTENT_TYPE_APP_VND_MATH },
        { CONTENT_TYPE_STR_APP_VND_NEWS, CONTENT_TYPE_APP_VND_NEWS },
        { CONTENT_TYPE_STR_APP_VND_OUTTRAY, CONTENT_TYPE_APP_VND_OUTTRAY },
        { CONTENT_TYPE_STR_APP_VND_TEMPLATE, CONTENT_TYPE_APP_VND_TEMPLATE },
        { CONTENT_TYPE_STR_APP_VND_WRITER, CONTENT_TYPE_APP_VND_WRITER },
        { CONTENT_TYPE_STR_APP_VND_WRITER_GLOBAL, CONTENT_TYPE_APP_VND_WRITER_GLOBAL },
        { CONTENT_TYPE_STR_APP_VND_WRITER_WEB, CONTENT_TYPE_APP_VND_WRITER_WEB },
        { CONTENT_TYPE_STR_APP_VND_SUN_XML_CALC, CONTENT_TYPE_APP_VND_SUN_XML_CALC },
        { CONTENT_TYPE_STR_APP_VND_SUN_XML_CHART, CONTENT_TYPE_APP_VND_SUN_XML_CHART },
        { CONTENT_TYPE_STR_APP_VND_SUN_XML_DRAW, CONTENT_TYPE_APP_VND_SUN_XML_DRAW },
         { CONTENT_TYPE_STR_APP_VND_SUN_XML_IMPRESS, CONTENT_TYPE_APP_VND_SUN_XML_IMPRESS },
         { CONTENT_TYPE_STR_APP_VND_SUN_XML_IMPRESSPACKED, CONTENT_TYPE_APP_VND_SUN_XML_IMPRESSPACKED },
        { CONTENT_TYPE_STR_APP_VND_SUN_XML_MATH, CONTENT_TYPE_APP_VND_SUN_XML_MATH },
        { CONTENT_TYPE_STR_APP_VND_SUN_XML_WRITER, CONTENT_TYPE_APP_VND_SUN_XML_WRITER },
        { CONTENT_TYPE_STR_APP_VND_SUN_XML_WRITER_GLOBAL, CONTENT_TYPE_APP_VND_SUN_XML_WRITER_GLOBAL },
        { CONTENT_TYPE_STR_APP_FRAMESET, CONTENT_TYPE_APP_FRAMESET },
        { CONTENT_TYPE_STR_APP_GALLERY, CONTENT_TYPE_APP_GALLERY },
        { CONTENT_TYPE_STR_APP_GALLERY_THEME, CONTENT_TYPE_APP_GALLERY_THEME },
        { CONTENT_TYPE_STR_APP_JAR, CONTENT_TYPE_APP_JAR },
        { CONTENT_TYPE_STR_APP_MACRO, CONTENT_TYPE_APP_MACRO },
        { CONTENT_TYPE_STR_APP_MSEXCEL, CONTENT_TYPE_APP_MSEXCEL },
        { CONTENT_TYPE_STR_APP_MSEXCEL_TEMPL, CONTENT_TYPE_APP_MSEXCEL_TEMPL },
        { CONTENT_TYPE_STR_APP_MSPPOINT, CONTENT_TYPE_APP_MSPPOINT },
        { CONTENT_TYPE_STR_APP_MSPPOINT_TEMPL, CONTENT_TYPE_APP_MSPPOINT_TEMPL },
        { CONTENT_TYPE_STR_APP_MSWORD, CONTENT_TYPE_APP_MSWORD },
        { CONTENT_TYPE_STR_APP_MSWORD_TEMPL, CONTENT_TYPE_APP_MSWORD_TEMPL },
        { CONTENT_TYPE_STR_APP_STARCALC, CONTENT_TYPE_APP_STARCALC },
        { CONTENT_TYPE_STR_APP_STARCHART, CONTENT_TYPE_APP_STARCHART },
        { CONTENT_TYPE_STR_APP_STARDRAW, CONTENT_TYPE_APP_STARDRAW },
        { CONTENT_TYPE_STR_APP_STARHELP, CONTENT_TYPE_APP_STARHELP },
        { CONTENT_TYPE_STR_APP_STARIMAGE, CONTENT_TYPE_APP_STARIMAGE },
        { CONTENT_TYPE_STR_APP_STARIMPRESS, CONTENT_TYPE_APP_STARIMPRESS },
        { CONTENT_TYPE_STR_APP_STARMAIL_SDM, CONTENT_TYPE_APP_STARMAIL_SDM },
        { CONTENT_TYPE_STR_APP_STARMAIL_SMD, CONTENT_TYPE_APP_STARMAIL_SMD },
        { CONTENT_TYPE_STR_APP_STARMATH, CONTENT_TYPE_APP_STARMATH },
        { CONTENT_TYPE_STR_APP_STARWRITER, CONTENT_TYPE_APP_STARWRITER },
        { CONTENT_TYPE_STR_APP_STARWRITER_GLOB, CONTENT_TYPE_APP_STARWRITER_GLOB },
        { CONTENT_TYPE_STR_APP_CDE_CALENDAR_APP, CONTENT_TYPE_APP_CDE_CALENDAR_APP },
        { CONTENT_TYPE_STR_APP_ZIP, CONTENT_TYPE_APP_ZIP },
        { CONTENT_TYPE_STR_AUDIO_AIFF, CONTENT_TYPE_AUDIO_AIFF },
        { CONTENT_TYPE_STR_AUDIO_BASIC, CONTENT_TYPE_AUDIO_BASIC },
        { CONTENT_TYPE_STR_AUDIO_MIDI, CONTENT_TYPE_AUDIO_MIDI },
        { CONTENT_TYPE_STR_AUDIO_VORBIS, CONTENT_TYPE_AUDIO_VORBIS },
        { CONTENT_TYPE_STR_AUDIO_WAV, CONTENT_TYPE_AUDIO_WAV },
        { CONTENT_TYPE_STR_AUDIO_WEBM, CONTENT_TYPE_AUDIO_WEBM },
        { CONTENT_TYPE_STR_IMAGE_GENERIC, CONTENT_TYPE_IMAGE_GENERIC },
        { CONTENT_TYPE_STR_IMAGE_GIF, CONTENT_TYPE_IMAGE_GIF },
        { CONTENT_TYPE_STR_IMAGE_JPEG, CONTENT_TYPE_IMAGE_JPEG },
        { CONTENT_TYPE_STR_IMAGE_PCX, CONTENT_TYPE_IMAGE_PCX },
        { CONTENT_TYPE_STR_IMAGE_PNG, CONTENT_TYPE_IMAGE_PNG },
        { CONTENT_TYPE_STR_IMAGE_TIFF, CONTENT_TYPE_IMAGE_TIFF },
        { CONTENT_TYPE_STR_IMAGE_BMP, CONTENT_TYPE_IMAGE_BMP },
        { CONTENT_TYPE_STR_INET_MSG_RFC822, CONTENT_TYPE_INET_MESSAGE_RFC822 },
        { CONTENT_TYPE_STR_INET_MULTI_ALTERNATIVE, CONTENT_TYPE_INET_MULTIPART_ALTERNATIVE },
        { CONTENT_TYPE_STR_INET_MULTI_DIGEST, CONTENT_TYPE_INET_MULTIPART_DIGEST },
        { CONTENT_TYPE_STR_INET_MULTI_MIXED, CONTENT_TYPE_INET_MULTIPART_MIXED },
        { CONTENT_TYPE_STR_INET_MULTI_PARALLEL, CONTENT_TYPE_INET_MULTIPART_PARALLEL },
        { CONTENT_TYPE_STR_INET_MULTI_RELATED, CONTENT_TYPE_INET_MULTIPART_RELATED },
        { CONTENT_TYPE_STR_TEXT_ICALENDAR, CONTENT_TYPE_TEXT_ICALENDAR },
        { CONTENT_TYPE_STR_TEXT_HTML, CONTENT_TYPE_TEXT_HTML },
        { CONTENT_TYPE_STR_TEXT_PLAIN, CONTENT_TYPE_TEXT_PLAIN },
        { CONTENT_TYPE_STR_TEXT_XMLICALENDAR, CONTENT_TYPE_TEXT_XMLICALENDAR },
        { CONTENT_TYPE_STR_TEXT_URL, CONTENT_TYPE_TEXT_URL },
        { CONTENT_TYPE_STR_TEXT_VCALENDAR, CONTENT_TYPE_TEXT_VCALENDAR },
        { CONTENT_TYPE_STR_TEXT_VCARD, CONTENT_TYPE_TEXT_VCARD },
        { CONTENT_TYPE_STR_VIDEO_MSVIDEO, CONTENT_TYPE_VIDEO_MSVIDEO },
        { CONTENT_TYPE_STR_VIDEO_THEORA, CONTENT_TYPE_VIDEO_THEORA },
        { CONTENT_TYPE_STR_VIDEO_VDO, CONTENT_TYPE_VIDEO_VDO },
        { CONTENT_TYPE_STR_VIDEO_WEBM, CONTENT_TYPE_VIDEO_WEBM },
        { CONTENT_TYPE_STR_X_STARMAIL, CONTENT_TYPE_X_STARMAIL },
        { CONTENT_TYPE_STR_X_VRML, CONTENT_TYPE_X_VRML }
};

/** A mapping from type IDs to presentation resource IDs.  Sorted by type ID.
 */
sal_uInt16 const aStaticResourceIDMap[CONTENT_TYPE_LAST + 1]
    = { STR_SVT_MIMETYPE_APP_OCTSTREAM, // CONTENT_TYPE_UNKNOWN
        STR_SVT_MIMETYPE_APP_OCTSTREAM, // CONTENT_TYPE_APP_OCTSTREAM
        STR_SVT_MIMETYPE_APP_PDF, // CONTENT_TYPE_APP_PDF
        STR_SVT_MIMETYPE_APP_RTF, // CONTENT_TYPE_APP_RTF
        STR_SVT_MIMETYPE_APP_MSWORD, // CONTENT_TYPE_APP_MSWORD
        STR_SVT_MIMETYPE_APP_MSWORD, // CONTENT_TYPE_APP_MSWORD_TEMPL //TODO: new presentation string?
        STR_SVT_MIMETYPE_APP_STARCALC, // CONTENT_TYPE_APP_STARCALC
        STR_SVT_MIMETYPE_APP_STARCHART, // CONTENT_TYPE_APP_STARCHART
        STR_SVT_MIMETYPE_APP_STARDRAW, // CONTENT_TYPE_APP_STARDRAW
        STR_SVT_MIMETYPE_APP_STARHELP, // CONTENT_TYPE_APP_STARHELP
        STR_SVT_MIMETYPE_APP_STARIMAGE, // CONTENT_TYPE_APP_STARIMAGE
        STR_SVT_MIMETYPE_APP_STARIMPRESS, // CONTENT_TYPE_APP_STARIMPRESS
        STR_SVT_MIMETYPE_APP_STARMATH, // CONTENT_TYPE_APP_STARMATH
        STR_SVT_MIMETYPE_APP_STARWRITER, // CONTENT_TYPE_APP_STARWRITER
        STR_SVT_MIMETYPE_APP_ZIP, // CONTENT_TYPE_APP_ZIP
        STR_SVT_MIMETYPE_AUDIO_AIFF, // CONTENT_TYPE_AUDIO_AIFF
        STR_SVT_MIMETYPE_AUDIO_BASIC, // CONTENT_TYPE_AUDIO_BASIC
        STR_SVT_MIMETYPE_AUDIO_MIDI, // CONTENT_TYPE_AUDIO_MIDI
        STR_SVT_MIMETYPE_AUDIO_VORBIS, // CONTENT_TYPE_AUDIO_VORBIS
        STR_SVT_MIMETYPE_AUDIO_WAV, // CONTENT_TYPE_AUDIO_WAV
        STR_SVT_MIMETYPE_AUDIO_WEBM, // CONTENT_TYPE_AUDIO_WEBM
        STR_SVT_MIMETYPE_IMAGE_GIF, // CONTENT_TYPE_IMAGE_GIF
        STR_SVT_MIMETYPE_IMAGE_JPEG, // CONTENT_TYPE_IMAGE_JPEG
        STR_SVT_MIMETYPE_IMAGE_PCX, // CONTENT_TYPE_IMAGE_PCX
        STR_SVT_MIMETYPE_IMAGE_PNG, // CONTENT_TYPE_IMAGE_PNG
        STR_SVT_MIMETYPE_IMAGE_TIFF, // CONTENT_TYPE_IMAGE_TIFF
        STR_SVT_MIMETYPE_IMAGE_BMP, // CONTENT_TYPE_IMAGE_BMP
        STR_SVT_MIMETYPE_TEXT_HTML, // CONTENT_TYPE_TEXT_HTML
        STR_SVT_MIMETYPE_TEXT_PLAIN, // CONTENT_TYPE_TEXT_PLAIN
        STR_SVT_MIMETYPE_TEXT_URL, // CONTENT_TYPE_TEXT_URL
        STR_SVT_MIMETYPE_TEXT_VCARD, // CONTENT_TYPE_TEXT_VCARD
        STR_SVT_MIMETYPE_VIDEO_MSVIDEO, // CONTENT_TYPE_VIDEO_MSVIDEO
        STR_SVT_MIMETYPE_VIDEO_THEORA, // CONTENT_TYPE_VIDEO_THEORA
        STR_SVT_MIMETYPE_VIDEO_VDO, // CONTENT_TYPE_VIDEO_VDO
        STR_SVT_MIMETYPE_VIDEO_WEBM, // CONTENT_TYPE_VIDEO_WEBM
        STR_SVT_MIMETYPE_CNT_FSYSBOX, // CONTENT_TYPE_X_CNT_FSYSBOX
        STR_SVT_MIMETYPE_CNT_FSYSFLD, // CONTENT_TYPE_X_CNT_FSYSFOLDER
        STR_SVT_MIMETYPE_X_STARMAIL, // CONTENT_TYPE_X_STARMAIL
        STR_SVT_MIMETYPE_X_VRML, // CONTENT_TYPE_X_VRML
        STR_SVT_MIMETYPE_APP_GAL, // CONTENT_TYPE_APP_GALLERY
        STR_SVT_MIMETYPE_APP_GAL_THEME, // CONTENT_TYPE_APP_GALLERY_THEME
        STR_SVT_MIMETYPE_APP_STARW_GLOB, // CONTENT_TYPE_APP_STARWRITER_GLOB
        STR_SVT_MIMETYPE_APP_SDM, // CONTENT_TYPE_APP_STARMAIL_SDM
        STR_SVT_MIMETYPE_APP_SMD, // CONTENT_TYPE_APP_STARMAIL_SMD
        STR_SVT_MIMETYPE_APP_STARCALC, // CONTENT_TYPE_APP_VND_CALC
        STR_SVT_MIMETYPE_APP_STARCHART, // CONTENT_TYPE_APP_VND_CHART
        STR_SVT_MIMETYPE_APP_STARDRAW, // CONTENT_TYPE_APP_VND_DRAW
        STR_SVT_MIMETYPE_APP_STARIMAGE, // CONTENT_TYPE_APP_VND_IMAGE
        STR_SVT_MIMETYPE_APP_STARIMPRESS, // CONTENT_TYPE_APP_VND_IMPRESS
        STR_SVT_MIMETYPE_X_STARMAIL, // CONTENT_TYPE_APP_VND_MAIL
        STR_SVT_MIMETYPE_APP_STARMATH, // CONTENT_TYPE_APP_VND_MATH
        STR_SVT_MIMETYPE_APP_STARWRITER, // CONTENT_TYPE_APP_VND_WRITER
        STR_SVT_MIMETYPE_APP_STARW_GLOB, // CONTENT_TYPE_APP_VND_WRITER_GLOBAL
        STR_SVT_MIMETYPE_APP_STARW_WEB, // CONTENT_TYPE_APP_VND_WRITER_WEB
        STR_SVT_MIMETYPE_FRAMESET, // CONTENT_TYPE_APP_FRAMESET
        STR_SVT_MIMETYPE_MACRO, // CONTENT_TYPE_APP_MACRO
        STR_SVT_MIMETYPE_CNT_SFSYSFOLDER,
            // CONTENT_TYPE_X_CNT_FSYSSPECIALFOLDER
        STR_SVT_MIMETYPE_APP_TEMPLATE, // CONTENT_TYPE_APP_VND_TEMPLATE
        STR_SVT_MIMETYPE_IMAGE_GENERIC, // CONTENT_TYPE_IMAGE_GENERIC
        STR_SVT_MIMETYPE_X_STARMAIL, // CONTENT_TYPE_APP_VND_NEWS
        STR_SVT_MIMETYPE_X_STARMAIL, // CONTENT_TYPE_APP_VND_OUTTRAY
        STR_SVT_MIMETYPE_APP_MSEXCEL, // CONTENT_TYPE_APP_MSEXCEL
        STR_SVT_MIMETYPE_APP_MSEXCEL_TEMPL, // CONTENT_TYPE_APP_MSEXCEL_TEMPL
        STR_SVT_MIMETYPE_APP_MSPPOINT, // CONTENT_TYPE_APP_MSPPOINT
        STR_SVT_MIMETYPE_APP_MSPPOINT, // CONTENT_TYPE_APP_MSPPOINT_TEMPL //@todo new presentation string?
        STR_SVT_MIMETYPE_TEXT_VCALENDAR, // CONTENT_TYPE_TEXT_VCALENDAR
        STR_SVT_MIMETYPE_TEXT_ICALENDAR, // CONTENT_TYPE_TEXT_ICALENDAR
        STR_SVT_MIMETYPE_TEXT_XMLICALENDAR, // CONTENT_TYPE_TEXT_XMLICALENDAR
        STR_SVT_MIMETYPE_TEXT_CDE_CALENDAR_APP,
            // CONTENT_TYPE_APP_CDE_CALENDAR_APP
        STR_SVT_MIMETYPE_INET_MSG_RFC822, // CONTENT_TYPE_INET_MESSAGE_RFC822
        STR_SVT_MIMETYPE_INET_MULTI_ALTERNATIVE,
            // CONTENT_TYPE_INET_MULTIPART_ALTERNATIVE
        STR_SVT_MIMETYPE_INET_MULTI_DIGEST,
            // CONTENT_TYPE_INET_MULTIPART_DIGEST
        STR_SVT_MIMETYPE_INET_MULTI_PARALLEL,
            // CONTENT_TYPE_INET_MULTIPART_PARALLEL
        STR_SVT_MIMETYPE_INET_MULTI_RELATED,
            // CONTENT_TYPE_INET_MULTIPART_RELATED
        STR_SVT_MIMETYPE_INET_MULTI_MIXED,
            // CONTENT_TYPE_INET_MULTIPART_MIXED
        STR_SVT_MIMETYPE_APP_IMPRESSPACKED,
            // CONTENT_TYPE_APP_VND_IMPRESSPACKED
        STR_SVT_MIMETYPE_APP_JAR, // CONTENT_TYPE_APP_JAR
        STR_SVT_MIMETYPE_APP_SXWRITER, // CONTENT_TYPE_APP_VND_SUN_XML_WRITER
        STR_SVT_MIMETYPE_APP_SXCALC, // CONTENT_TYPE_APP_VND_SUN_XML_CALC
        STR_SVT_MIMETYPE_APP_SXIMPRESS, // CONTENT_TYPE_APP_VND_SUN_XML_IMPRESS
        STR_SVT_MIMETYPE_APP_SXDRAW, // CONTENT_TYPE_APP_VND_SUN_XML_DRAW
        STR_SVT_MIMETYPE_APP_SXCHART, // CONTENT_TYPE_APP_VND_SUN_XML_CHART
        STR_SVT_MIMETYPE_APP_SXMATH, // CONTENT_TYPE_APP_VND_SUN_XML_MATH
        STR_SVT_MIMETYPE_APP_SXGLOBAL, // CONTENT_TYPE_APP_VND_SUN_XML_WRITER_GLOBAL
        STR_SVT_MIMETYPE_APP_SXIPACKED, // CONTENT_TYPE_APP_VND_SUN_XML_IMPRESSPACKED
 };

/** A mapping from extensions to type IDs.  Sorted by extension.
 */
MediaTypeEntry const aStaticExtensionMap[]
    = { { "aif", CONTENT_TYPE_AUDIO_AIFF },
        { "aiff", CONTENT_TYPE_AUDIO_AIFF },
        { "appt", CONTENT_TYPE_APP_CDE_CALENDAR_APP },
        { "au", CONTENT_TYPE_AUDIO_BASIC },
        { "avi", CONTENT_TYPE_VIDEO_MSVIDEO },
        { "bmp", CONTENT_TYPE_IMAGE_BMP },
        { "cgm", CONTENT_TYPE_IMAGE_GENERIC },
        { "doc", CONTENT_TYPE_APP_MSWORD },
        { "dot", CONTENT_TYPE_APP_MSWORD_TEMPL },
        { "dxf", CONTENT_TYPE_IMAGE_GENERIC },
        { "eps", CONTENT_TYPE_IMAGE_GENERIC },
        { "gal", CONTENT_TYPE_APP_GALLERY },
        { "gif", CONTENT_TYPE_IMAGE_GIF },
        { "htm", CONTENT_TYPE_TEXT_HTML },
        { "html", CONTENT_TYPE_TEXT_HTML },
        { "ics", CONTENT_TYPE_TEXT_ICALENDAR },
        { "jar", CONTENT_TYPE_APP_JAR },
        { "jpeg", CONTENT_TYPE_IMAGE_JPEG },
        { "jpg", CONTENT_TYPE_IMAGE_JPEG },
        { "met", CONTENT_TYPE_IMAGE_GENERIC },
        { "mid", CONTENT_TYPE_AUDIO_MIDI },
        { "midi", CONTENT_TYPE_AUDIO_MIDI },
        { "ogg", CONTENT_TYPE_AUDIO_VORBIS },
        { "pbm", CONTENT_TYPE_IMAGE_GENERIC },
        { "pcd", CONTENT_TYPE_IMAGE_GENERIC },
        { "pct", CONTENT_TYPE_IMAGE_GENERIC },
        { "pcx", CONTENT_TYPE_IMAGE_PCX },
        { "pdf", CONTENT_TYPE_APP_PDF },
        { "pgm", CONTENT_TYPE_IMAGE_GENERIC },
        { "png", CONTENT_TYPE_IMAGE_PNG },
        { "pot", CONTENT_TYPE_APP_MSPPOINT_TEMPL },
        { "ppm", CONTENT_TYPE_IMAGE_GENERIC },
        { "ppt", CONTENT_TYPE_APP_MSPPOINT },
        { "psd", CONTENT_TYPE_IMAGE_GENERIC },
        { "ras", CONTENT_TYPE_IMAGE_GENERIC },
        { "rtf", CONTENT_TYPE_APP_RTF },
        { "sda", CONTENT_TYPE_APP_VND_DRAW },
        { "sdc", CONTENT_TYPE_APP_VND_CALC },
        { "sdd", CONTENT_TYPE_APP_VND_IMPRESS },
        { "sdm", CONTENT_TYPE_APP_VND_MAIL },
        { "sdp", CONTENT_TYPE_APP_VND_IMPRESSPACKED },
        { "sds", CONTENT_TYPE_APP_VND_CHART },
        { "sdw", CONTENT_TYPE_APP_VND_WRITER },
        { "sd~", CONTENT_TYPE_X_STARMAIL },
        { "sfs", CONTENT_TYPE_APP_FRAMESET  },
        { "sgl", CONTENT_TYPE_APP_VND_WRITER_GLOBAL  },
        { "sim", CONTENT_TYPE_APP_VND_IMAGE },
        { "smd", CONTENT_TYPE_APP_STARMAIL_SMD }, //CONTENT_TYPE_X_STARMAIL
        { "smf", CONTENT_TYPE_APP_VND_MATH },
        { "svh", CONTENT_TYPE_APP_STARHELP },
        { "svm", CONTENT_TYPE_IMAGE_GENERIC },
        { "sxc", CONTENT_TYPE_APP_VND_SUN_XML_CALC },
        { "sxd", CONTENT_TYPE_APP_VND_SUN_XML_DRAW },
        { "sxg", CONTENT_TYPE_APP_VND_SUN_XML_WRITER_GLOBAL },
        { "sxi", CONTENT_TYPE_APP_VND_SUN_XML_IMPRESS },
        { "sxm", CONTENT_TYPE_APP_VND_SUN_XML_MATH },
        { "sxp", CONTENT_TYPE_APP_VND_SUN_XML_IMPRESSPACKED },
        { "sxs", CONTENT_TYPE_APP_VND_SUN_XML_CHART },
        { "sxw", CONTENT_TYPE_APP_VND_SUN_XML_WRITER },
        { "tga", CONTENT_TYPE_IMAGE_GENERIC },
        { "thm", CONTENT_TYPE_APP_GALLERY_THEME },
        { "tif", CONTENT_TYPE_IMAGE_TIFF },
        { "tiff", CONTENT_TYPE_IMAGE_TIFF },
        { "txt", CONTENT_TYPE_TEXT_PLAIN },
        { "url", CONTENT_TYPE_TEXT_URL },
        { "vcf", CONTENT_TYPE_TEXT_VCARD },
        { "vcs", CONTENT_TYPE_TEXT_VCALENDAR },
        { "vdo", CONTENT_TYPE_VIDEO_VDO },
        { "vor", CONTENT_TYPE_APP_VND_TEMPLATE },
        { "wav", CONTENT_TYPE_AUDIO_WAV },
        { "webm", CONTENT_TYPE_VIDEO_WEBM },
        { "wmf", CONTENT_TYPE_IMAGE_GENERIC },
        { "wrl", CONTENT_TYPE_X_VRML },
        { "xbm", CONTENT_TYPE_IMAGE_GENERIC },
        { "xcs", CONTENT_TYPE_TEXT_XMLICALENDAR },
        { "xls", CONTENT_TYPE_APP_MSEXCEL },
        { "xlt", CONTENT_TYPE_APP_MSEXCEL_TEMPL },
        { "xlw", CONTENT_TYPE_APP_MSEXCEL },
        { "xpm", CONTENT_TYPE_IMAGE_GENERIC },
        { "zip", CONTENT_TYPE_APP_ZIP } };

}


//  Registration


Registration::~Registration()
{
    for ( TypeIDMap::iterator it = m_aTypeIDMap.begin(); it != m_aTypeIDMap.end(); ++it )
        delete it->second;
}

// static
TypeNameMapEntry * Registration::getExtensionEntry(OUString const & rTypeName)
{
    OUString aTheTypeName = rTypeName.toAsciiLowerCase();
    Registration &rRegistration = theRegistration::get();
    TypeNameMap::iterator it = rRegistration.m_aTypeNameMap.find(aTheTypeName);
    if (it != rRegistration.m_aTypeNameMap.end())
        return & it->second;
    return nullptr;
}

// static
INetContentType Registration::RegisterContentType(OUString const & rTypeName,
                                                  OUString const & rPresentation,
                                                  OUString const * pExtension,
                                                  OUString const * pSystemFileType)
{
    Registration &rRegistration = theRegistration::get();

    DBG_ASSERT(GetContentType(rTypeName) == CONTENT_TYPE_UNKNOWN,
               "Registration::RegisterContentType(): Already registered");

    INetContentType eTypeID = INetContentType(rRegistration.m_nNextDynamicID++);
    OUString aTheTypeName = rTypeName.toAsciiLowerCase();

    TypeIDMapEntry * pTypeIDMapEntry = new TypeIDMapEntry;
    pTypeIDMapEntry->m_aTypeName = aTheTypeName;
    pTypeIDMapEntry->m_aPresentation = rPresentation;
    if (pSystemFileType)
        pTypeIDMapEntry->m_aSystemFileType = *pSystemFileType;
    rRegistration.m_aTypeIDMap.insert( ::std::make_pair( eTypeID, pTypeIDMapEntry ) );

    rRegistration.m_aTypeNameMap.insert(std::make_pair(aTheTypeName,
                TypeNameMapEntry(eTypeID, pExtension)));

    if (pExtension)
    {
        rRegistration.m_aExtensionMap.insert(std::make_pair(*pExtension, ExtensionMapEntry(eTypeID)));
    }

    return eTypeID;
}

// static
INetContentType Registration::GetContentType(OUString const & rTypeName)
{
    Registration &rRegistration = theRegistration::get();

    OUString aTheTypeName = rTypeName.toAsciiLowerCase();
    TypeNameMap::const_iterator it = rRegistration.m_aTypeNameMap.find(aTheTypeName);
    return it != rRegistration.m_aTypeNameMap.end()
        ? it->second.m_eTypeID
        : CONTENT_TYPE_UNKNOWN;
}

// static
OUString Registration::GetContentType(INetContentType eTypeID)
{
    Registration &rRegistration = theRegistration::get();

    TypeIDMap::const_iterator pEntry = rRegistration.m_aTypeIDMap.find( eTypeID );
    if( pEntry != rRegistration.m_aTypeIDMap.end() )
        return pEntry->second->m_aTypeName;
    return OUString();
}

// static
OUString Registration::GetPresentation(INetContentType eTypeID)
{
    Registration &rRegistration = theRegistration::get();

    TypeIDMap::const_iterator pEntry = rRegistration.m_aTypeIDMap.find( eTypeID );
    if( pEntry != rRegistration.m_aTypeIDMap.end() )
        return pEntry->second->m_aPresentation;
    else
        return  OUString();
}

// static
INetContentType Registration::GetContentType4Extension(OUString const & rExtension)
{
    Registration &rRegistration = theRegistration::get();

    ExtensionMap::const_iterator it = rRegistration.m_aExtensionMap.find(rExtension);
    return it != rRegistration.m_aExtensionMap.end()
        ? it->second.m_eTypeID
        : CONTENT_TYPE_UNKNOWN;
}


//  seekEntry


namespace
{

MediaTypeEntry const * seekEntry(OUString const & rTypeName,
                                 MediaTypeEntry const * pMap, sal_Size nSize)
{
#if defined DBG_UTIL
    for (sal_Size i = 0; i < nSize - 1; ++i)
        DBG_ASSERT(
            rtl_str_compare(
                pMap[i].m_pTypeName, pMap[i + 1].m_pTypeName) < 0,
            "seekEntry(): Bad map");
#endif

    sal_Size nLow = 0;
    sal_Size nHigh = nSize;
    while (nLow != nHigh)
    {
        sal_Size nMiddle = (nLow + nHigh) / 2;
        MediaTypeEntry const * pEntry = pMap + nMiddle;
        sal_Int32 nCmp = rTypeName.compareToIgnoreAsciiCaseAscii(pEntry->m_pTypeName);
        if (nCmp < 0)
            nHigh = nMiddle;
        else if (nCmp == 0)
            return pEntry;

        else
            nLow = nMiddle + 1;
    }
    return nullptr;
}

}

//static
INetContentType INetContentTypes::RegisterContentType(OUString const & rTypeName,
                                                      OUString const & rPresentation,
                                                      OUString const * pExtension)
{
    INetContentType eTypeID = GetContentType(rTypeName);
    if (eTypeID == CONTENT_TYPE_UNKNOWN)
        eTypeID = Registration::RegisterContentType(rTypeName, rPresentation,
                                                    pExtension,
                                                    nullptr/*pSystemFileType*/);
    else if (eTypeID > CONTENT_TYPE_LAST)
    {
        TypeIDMapEntry * pTypeEntry = Registration::getEntry(eTypeID);
        if (pTypeEntry)
        {
            if (!rPresentation.isEmpty())
                pTypeEntry->m_aPresentation = rPresentation;
        }
        if (pExtension)
        {
            TypeNameMapEntry * pEntry = Registration::getExtensionEntry(rTypeName);
            if (pEntry)
                pEntry->m_aExtension = *pExtension;
        }
    }
    return eTypeID;
}

// static
INetContentType INetContentTypes::GetContentType(OUString const & rTypeName)
{
    OUString aType;
    OUString aSubType;
    if (parse(rTypeName, aType, aSubType))
    {
        aType += "/";
        aType += aSubType;
        MediaTypeEntry const * pEntry = seekEntry(aType, aStaticTypeNameMap,
                                                  CONTENT_TYPE_LAST + 1);
        return pEntry ? pEntry->m_eTypeID : Registration::GetContentType(aType);
    }
    else
        return rTypeName.equalsIgnoreAsciiCase(CONTENT_TYPE_STR_X_STARMAIL) ?
            CONTENT_TYPE_X_STARMAIL : CONTENT_TYPE_UNKNOWN;
            // the content type "x-starmail" has no sub type
}

//static
OUString INetContentTypes::GetContentType(INetContentType eTypeID)
{
    static sal_Char const * aMap[CONTENT_TYPE_LAST + 1];
    static bool bInitialized = false;
    if (!bInitialized)
    {
        for (sal_Size i = 0; i <= CONTENT_TYPE_LAST; ++i)
            aMap[aStaticTypeNameMap[i].m_eTypeID] = aStaticTypeNameMap[i].m_pTypeName;
        aMap[CONTENT_TYPE_UNKNOWN] = CONTENT_TYPE_STR_APP_OCTSTREAM;
        aMap[CONTENT_TYPE_TEXT_PLAIN] = CONTENT_TYPE_STR_TEXT_PLAIN
                                            "; charset=iso-8859-1";
        bInitialized = true;
    }

    OUString aTypeName = eTypeID <= CONTENT_TYPE_LAST ? OUString::createFromAscii(aMap[eTypeID])
                                                      : Registration::GetContentType(eTypeID);
    if (aTypeName.isEmpty())
    {
        OSL_FAIL("INetContentTypes::GetContentType(): Bad ID");
        return OUString(CONTENT_TYPE_STR_APP_OCTSTREAM);
    }
    return aTypeName;
}

//static
OUString INetContentTypes::GetPresentation(INetContentType eTypeID,
                                            const LanguageTag& aLocale)
{
    sal_uInt16 nResID = sal_uInt16();
    if (eTypeID <= CONTENT_TYPE_LAST)
        nResID = aStaticResourceIDMap[eTypeID];
    else
    {
        OUString aPresentation = Registration::GetPresentation(eTypeID);
        if (aPresentation.isEmpty())
            nResID = STR_SVT_MIMETYPE_APP_OCTSTREAM;
        else
            return aPresentation;
    }
    return svl::getStringResource(nResID, aLocale);
}

//static
INetContentType INetContentTypes::GetContentType4Extension(OUString const & rExtension)
{
    MediaTypeEntry const * pEntry = seekEntry(rExtension, aStaticExtensionMap,
                                              sizeof aStaticExtensionMap / sizeof (MediaTypeEntry));
    if (pEntry)
        return pEntry->m_eTypeID;
    INetContentType eTypeID = Registration::GetContentType4Extension(rExtension);
    return eTypeID == CONTENT_TYPE_UNKNOWN ? CONTENT_TYPE_APP_OCTSTREAM
                                           : eTypeID;
}

//static
INetContentType INetContentTypes::GetContentTypeFromURL(OUString const & rURL)
{
    INetContentType eTypeID = CONTENT_TYPE_UNKNOWN;
    OUString aToken = rURL.getToken(0, ':');
    if (!aToken.isEmpty())
    {
        if (aToken.equalsIgnoreAsciiCase(INETTYPE_URL_PROT_FILE))
            if (rURL[ rURL.getLength() - 1 ] == (sal_Unicode)'/') // folder
                if (rURL.getLength() > RTL_CONSTASCII_LENGTH("file:///"))
                    if (WildCard("*/{*}/").Matches(rURL)) // special folder
                        eTypeID = CONTENT_TYPE_X_CNT_FSYSSPECIALFOLDER;
                    else
                        // drive? -> "file:///?|/"
                        if (rURL.getLength() == 11
                            && rURL[ rURL.getLength() - 2 ] == '|')
                        {
                            // Drives need further processing, because of
                            // dynamic type according to underlying volume,
                            // which cannot be determined here.
                        }
                        else // normal folder
                            eTypeID = CONTENT_TYPE_X_CNT_FSYSFOLDER;
                else // file system root
                    eTypeID = CONTENT_TYPE_X_CNT_FSYSBOX;
            else // file
            {
                //@@@
            }
        else if (aToken.equalsIgnoreAsciiCase(INETTYPE_URL_PROT_HTTP)
                 || aToken.equalsIgnoreAsciiCase(INETTYPE_URL_PROT_HTTPS))
            eTypeID = CONTENT_TYPE_TEXT_HTML;
        else if (aToken.equalsIgnoreAsciiCase(INETTYPE_URL_PROT_PRIVATE))
        {
            OUString aSecondPart = rURL.getToken(1, ':');
            aToken = aSecondPart.getToken(0, '/');
            if (aToken == "factory")
            {
                aToken = aSecondPart.getToken(1, '/');
                if (aToken == "swriter")
                {
                    aToken = aSecondPart.getToken(2, '/');
                    eTypeID = aToken == "web" ?
                                  CONTENT_TYPE_APP_VND_WRITER_WEB :
                              aToken == "GlobalDocument" ?
                                  CONTENT_TYPE_APP_VND_WRITER_GLOBAL :
                                  CONTENT_TYPE_APP_VND_WRITER;
                }
                else if (aToken == "scalc")
                    eTypeID = CONTENT_TYPE_APP_VND_CALC;
                else if (aToken == "sdraw")
                    eTypeID = CONTENT_TYPE_APP_VND_DRAW;
                else if (aToken == "simpress")
                    eTypeID = CONTENT_TYPE_APP_VND_IMPRESS;
                else if (aToken == "schart")
                    eTypeID = CONTENT_TYPE_APP_VND_CHART;
                else if (aToken == "simage")
                    eTypeID = CONTENT_TYPE_APP_VND_IMAGE;
                else if (aToken == "smath")
                    eTypeID = CONTENT_TYPE_APP_VND_MATH;
                else if (aToken == "frameset")
                    eTypeID = CONTENT_TYPE_APP_FRAMESET;
            }
            else if (aToken == "helpid")
                eTypeID = CONTENT_TYPE_APP_STARHELP;
        }
        else if (aToken.equalsIgnoreAsciiCase(INETTYPE_URL_PROT_MAILTO))
            eTypeID = CONTENT_TYPE_APP_VND_OUTTRAY;
        else if (aToken.equalsIgnoreAsciiCase(INETTYPE_URL_PROT_MACRO))
            eTypeID = CONTENT_TYPE_APP_MACRO;
        else if (aToken.equalsIgnoreAsciiCase(INETTYPE_URL_PROT_DATA))
        {
            OUString aSecondPart = rURL.getToken(1, ':');
            aToken = aSecondPart.getToken(0, ',');
            eTypeID = GetContentType(aToken);
        }
    }
    if (eTypeID == CONTENT_TYPE_UNKNOWN)
    {
        OUString aExtension;
        if (GetExtensionFromURL(rURL, aExtension))
            eTypeID = GetContentType4Extension(aExtension);
    }
    return eTypeID;
}

//static
bool INetContentTypes::GetExtensionFromURL(OUString const & rURL,
                                           OUString & rExtension)
{
    sal_Int32 nSlashPos = 0;
    sal_Int32 i = 0;
    while (i >= 0)
    {
        nSlashPos = i;
        i = rURL.indexOf((sal_Unicode)'/', i + 1);
    }
    if (nSlashPos != 0)
    {
        sal_Int32 nLastDotPos = i = rURL.indexOf((sal_Unicode)'.', nSlashPos);
        while (i >= 0)
        {
            nLastDotPos = i;
            i = rURL.indexOf((sal_Unicode)'.', i + 1);
        }
        if (nLastDotPos >- 0)
            rExtension = rURL.copy(nLastDotPos + 1);
        return true;
    }
    return false;
}

bool INetContentTypes::parse(
    OUString const & rMediaType, OUString & rType, OUString & rSubType,
    INetContentTypeParameterList * pParameters)
{
    sal_Unicode const * b = rMediaType.getStr();
    sal_Unicode const * e = b + rMediaType.getLength();
    OUString t;
    OUString s;
    INetContentTypeParameterList p;
    if (INetMIME::scanContentType(b, e, &t, &s, pParameters == nullptr ? nullptr : &p) == e) {
        rType = t;
        rSubType = s;
        if (pParameters != nullptr) {
            *pParameters = p;
        }
        return true;
    } else {
        return false;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
