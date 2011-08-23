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

// MARKER(update_precomp.py): autogen include statement, do not remove


#include <tools/table.hxx>
#include <tools/wldcrd.hxx>

#include <bf_svtools/inettype.hxx>
#include <bf_svtools/svtdata.hxx>
#include <bf_svtools/svtools.hrc>

#ifndef _SVSTDARR_STRINGSSORT_DECL
#define _SVSTDARR_STRINGSSORT
#include <bf_svtools/svstdarr.hxx>
#undef _SVSTDARR_STRINGSSORT
#endif

//============================================================================
namespace binfilter
{

//============================================================================
struct MediaTypeEntry
{
    sal_Char const * m_pTypeName;
    INetContentType	m_eTypeID;
    sal_Char const * m_pExtension;
};

//============================================================================
struct TypeIDMapEntry
{
    UniString m_aTypeName;
    UniString m_aPresentation;
    UniString m_aSystemFileType;
};

//============================================================================
struct TypeNameMapEntry: public UniString
{
    UniString m_aExtension;
    INetContentType m_eTypeID;

    TypeNameMapEntry(const UniString & rType):
        UniString(rType), m_eTypeID(CONTENT_TYPE_UNKNOWN) {}
};

//============================================================================
struct ExtensionMapEntry: public UniString
{
    INetContentType	m_eTypeID;

    ExtensionMapEntry(const UniString & rExt):
        UniString(rExt), m_eTypeID(CONTENT_TYPE_UNKNOWN) {}
};

//============================================================================
class Registration
{
    static Registration * m_pRegistration;

    Table m_aTypeIDMap; // map TypeID to TypeName, Presentation
    SvStringsSort m_aTypeNameMap; // map TypeName to TypeID, Extension
    SvStringsSort m_aExtensionMap; // map Extension to TypeID
    sal_uInt32 m_nNextDynamicID;

public:
    Registration(): m_nNextDynamicID(CONTENT_TYPE_LAST + 1) {}

    ~Registration();

    static inline void deinitialize();

    static inline TypeIDMapEntry * getEntry(INetContentType eTypeID);

    static TypeNameMapEntry * getExtensionEntry(UniString const & rTypeName);

    static INetContentType RegisterContentType(UniString const & rTypeName,
                                               UniString const &
                                                   rPresentation,
                                               UniString const * pExtension,
                                               UniString const *
                                                   pSystemFileType);

    static INetContentType GetContentType(UniString const & rTypeName);

    static UniString GetContentType(INetContentType eTypeID);

    static INetContentType GetContentType4Extension(UniString const &
                                                        rExtension);

};

// static
inline void Registration::deinitialize()
{
    delete m_pRegistration;
    m_pRegistration = 0;
}

// static
inline TypeIDMapEntry * Registration::getEntry(INetContentType eTypeID)
{
    return
        m_pRegistration ?
            static_cast< TypeIDMapEntry * >(m_pRegistration->
                                                m_aTypeIDMap.Get(eTypeID)) :
            0;
}

//============================================================================
MediaTypeEntry const * seekEntry(UniString const & rTypeName,
                                 MediaTypeEntry const * pMap, sal_Size nSize);

//============================================================================
/** A mapping from type names to type ids and extensions.  Sorted by type
    name.
 */
MediaTypeEntry const aStaticTypeNameMap[CONTENT_TYPE_LAST + 1]
    = { { " ", CONTENT_TYPE_UNKNOWN, "" },
        { CONTENT_TYPE_STR_X_CNT_DOCUMENT, CONTENT_TYPE_X_CNT_DOCUMENT,
          "tmp" },
        { CONTENT_TYPE_STR_X_CNT_FSYSBOX, CONTENT_TYPE_X_CNT_FSYSBOX, "tmp" },
        { CONTENT_TYPE_STR_X_CNT_CDROM_VOLUME,
          CONTENT_TYPE_X_CNT_CDROM_VOLUME, "tmp" },
        { CONTENT_TYPE_STR_X_CNT_DISK_35, CONTENT_TYPE_X_CNT_DISK_35, "tmp" },
        { CONTENT_TYPE_STR_X_CNT_DISK_525, CONTENT_TYPE_X_CNT_DISK_525,
          "tmp" },
        { CONTENT_TYPE_STR_X_CNT_FSYSFILE, CONTENT_TYPE_X_CNT_FSYSFILE,
          "tmp" },
        { CONTENT_TYPE_STR_X_CNT_FIXED_VOLUME,
          CONTENT_TYPE_X_CNT_FIXED_VOLUME, "tmp" },
        { CONTENT_TYPE_STR_X_CNT_FSYSFOLDER, CONTENT_TYPE_X_CNT_FSYSFOLDER,
          "tmp" },
        { CONTENT_TYPE_STR_X_CNT_RAM_VOLUME, CONTENT_TYPE_X_CNT_RAM_VOLUME,
          "tmp" },
        { CONTENT_TYPE_STR_X_CNT_REMOTE_VOLUME,
          CONTENT_TYPE_X_CNT_REMOTE_VOLUME, "tmp" },
        { CONTENT_TYPE_STR_X_CNT_REMOVEABLE_VOLUME,
          CONTENT_TYPE_X_CNT_REMOVEABLE_VOLUME, "tmp" },
        { CONTENT_TYPE_STR_X_CNT_FSYSSPECIALFILE,
          CONTENT_TYPE_X_CNT_FSYSSPECIALFILE, "tmp" },
        { CONTENT_TYPE_STR_X_CNT_FSYSSPECIALFOLDER,
          CONTENT_TYPE_X_CNT_FSYSSPECIALFOLDER, "tmp" },
        { CONTENT_TYPE_STR_X_CNT_TAPEDRIVE, CONTENT_TYPE_X_CNT_TAPEDRIVE,
          "tmp" },
        { CONTENT_TYPE_STR_X_CNT_FSYSURLFILE, CONTENT_TYPE_X_CNT_FSYSURLFILE,
          "tmp" },
        { CONTENT_TYPE_STR_X_CNT_FTPBOX, CONTENT_TYPE_X_CNT_FTPBOX, "tmp" },
        { CONTENT_TYPE_STR_X_CNT_FTPFILE, CONTENT_TYPE_X_CNT_FTPFILE, "tmp" },
        { CONTENT_TYPE_STR_X_CNT_FTPFOLDER, CONTENT_TYPE_X_CNT_FTPFOLDER,
          "tmp" },
        { CONTENT_TYPE_STR_X_CNT_FTPLINK, CONTENT_TYPE_X_CNT_FTPLINK, "tmp" },
        { CONTENT_TYPE_STR_X_CNT_HTTPBOX, CONTENT_TYPE_X_CNT_HTTPBOX, "tmp" },
        { CONTENT_TYPE_STR_X_CNT_HTTPFILE, CONTENT_TYPE_X_CNT_HTTPFILE,
          "tmp" },
        { CONTENT_TYPE_STR_X_CNT_IMAPBOX, CONTENT_TYPE_X_CNT_IMAPBOX, "tmp" },
        { CONTENT_TYPE_STR_X_CNT_IMAPFOLDER, CONTENT_TYPE_X_CNT_IMAPFOLDER,
          "tmp" },
        { CONTENT_TYPE_STR_X_CNT_MESSAGE, CONTENT_TYPE_X_CNT_MESSAGE, "tmp" },
        { CONTENT_TYPE_STR_X_CNT_NEWSBOX, CONTENT_TYPE_X_CNT_NEWSBOX, "tmp" },
        { CONTENT_TYPE_STR_X_CNT_NEWSGROUP, CONTENT_TYPE_X_CNT_NEWSGROUP,
          "tmp" },
        { CONTENT_TYPE_STR_X_CNT_OUTBOX, CONTENT_TYPE_X_CNT_OUTBOX, "tmp" },
        { CONTENT_TYPE_STR_X_CNT_POP3BOX, CONTENT_TYPE_X_CNT_POP3BOX, "tmp" },
        { CONTENT_TYPE_STR_X_CNT_PUBLISHBOX, CONTENT_TYPE_X_CNT_PUBLISHBOX,
          "tmp" },
        { CONTENT_TYPE_STR_X_CNT_SEARCHBOX, CONTENT_TYPE_X_CNT_SEARCHBOX,
          "tmp" },
        { CONTENT_TYPE_STR_X_CNT_SEPARATOR, CONTENT_TYPE_X_CNT_SEPARATOR,
          "tmp" },
        { CONTENT_TYPE_STR_X_CNT_BOOKMARK, CONTENT_TYPE_X_CNT_BOOKMARK,
          "tmp" },
        { CONTENT_TYPE_STR_X_CNT_SUBSCRIBEBOX,
          CONTENT_TYPE_X_CNT_SUBSCRIBEBOX, "tmp" },
        { CONTENT_TYPE_STR_X_CNT_CDF, CONTENT_TYPE_X_CNT_CDF, "tmp" },
        { CONTENT_TYPE_STR_X_CNT_CDFITEM, CONTENT_TYPE_X_CNT_CDFITEM, "tmp" },
        { CONTENT_TYPE_STR_X_CNT_CDFSUB, CONTENT_TYPE_X_CNT_CDFSUB, "tmp" },
        { CONTENT_TYPE_STR_X_CNT_STARCHANNEL, CONTENT_TYPE_X_CNT_STARCHANNEL,
          "tmp" },
        { CONTENT_TYPE_STR_X_CNT_TRASHBOX, CONTENT_TYPE_X_CNT_TRASHBOX,
          "tmp" },
        { CONTENT_TYPE_STR_X_CNT_TRASH, CONTENT_TYPE_X_CNT_TRASH, "tmp" },
        { CONTENT_TYPE_STR_X_CNT_VIMBBOARD, CONTENT_TYPE_X_CNT_VIMBBOARD,
          "tmp" },
        { CONTENT_TYPE_STR_X_CNT_VIMBBOARDBOX,
          CONTENT_TYPE_X_CNT_VIMBBOARDBOX, "tmp" },
        { CONTENT_TYPE_STR_X_CNT_VIMBOX, CONTENT_TYPE_X_CNT_VIMBOX, "tmp" },
        { CONTENT_TYPE_STR_X_CNT_VIMINBOX, CONTENT_TYPE_X_CNT_VIMINBOX,
          "tmp" },
        { CONTENT_TYPE_STR_APP_OCTSTREAM, CONTENT_TYPE_APP_OCTSTREAM, "tmp" },
        { CONTENT_TYPE_STR_APP_PDF, CONTENT_TYPE_APP_PDF, "pdf" },
        { CONTENT_TYPE_STR_APP_RTF, CONTENT_TYPE_APP_RTF, "rtf" },
        { CONTENT_TYPE_STR_APP_VND_CALC, CONTENT_TYPE_APP_VND_CALC, "sdc" },
        { CONTENT_TYPE_STR_APP_VND_CHART, CONTENT_TYPE_APP_VND_CHART, "sds" },
        { CONTENT_TYPE_STR_APP_VND_DRAW, CONTENT_TYPE_APP_VND_DRAW, "sda" },
        { CONTENT_TYPE_STR_APP_VND_IMAGE, CONTENT_TYPE_APP_VND_IMAGE, "sim" },
        { CONTENT_TYPE_STR_APP_VND_IMPRESS, CONTENT_TYPE_APP_VND_IMPRESS,
          "sdd" },
        { CONTENT_TYPE_STR_APP_VND_IMPRESSPACKED,
          CONTENT_TYPE_APP_VND_IMPRESSPACKED, "sdp" },
        { CONTENT_TYPE_STR_APP_VND_MAIL, CONTENT_TYPE_APP_VND_MAIL, "sdm" },
        { CONTENT_TYPE_STR_APP_VND_MATH, CONTENT_TYPE_APP_VND_MATH, "smf" },
        { CONTENT_TYPE_STR_APP_VND_NEWS, CONTENT_TYPE_APP_VND_NEWS, "sdm" },
        { CONTENT_TYPE_STR_APP_VND_OUTTRAY, CONTENT_TYPE_APP_VND_OUTTRAY,
          "sdm" },
        { CONTENT_TYPE_STR_APP_VND_TEMPLATE, CONTENT_TYPE_APP_VND_TEMPLATE,
          "vor" },
        { CONTENT_TYPE_STR_APP_VND_WRITER, CONTENT_TYPE_APP_VND_WRITER,
          "sdw" },
        { CONTENT_TYPE_STR_APP_VND_WRITER_GLOBAL,
          CONTENT_TYPE_APP_VND_WRITER_GLOBAL, "sgl" },
        { CONTENT_TYPE_STR_APP_VND_WRITER_WEB,
          CONTENT_TYPE_APP_VND_WRITER_WEB, "htm" },
        { CONTENT_TYPE_STR_APP_VND_SUN_XML_CALC, CONTENT_TYPE_APP_VND_SUN_XML_CALC, "sxc" },
        { CONTENT_TYPE_STR_APP_VND_SUN_XML_CHART, CONTENT_TYPE_APP_VND_SUN_XML_CHART, "sxs" },
        { CONTENT_TYPE_STR_APP_VND_SUN_XML_DRAW, CONTENT_TYPE_APP_VND_SUN_XML_DRAW, "sxd" },
         { CONTENT_TYPE_STR_APP_VND_SUN_XML_IMPRESS, CONTENT_TYPE_APP_VND_SUN_XML_IMPRESS, "sxi" },
         { CONTENT_TYPE_STR_APP_VND_SUN_XML_IMPRESSPACKED, CONTENT_TYPE_APP_VND_SUN_XML_IMPRESSPACKED, "sxp" },
        { CONTENT_TYPE_STR_APP_VND_SUN_XML_MATH, CONTENT_TYPE_APP_VND_SUN_XML_MATH, "sxm" },
        { CONTENT_TYPE_STR_APP_VND_SUN_XML_WRITER, CONTENT_TYPE_APP_VND_SUN_XML_WRITER, "sxw" },
        { CONTENT_TYPE_STR_APP_VND_SUN_XML_WRITER_GLOBAL, CONTENT_TYPE_APP_VND_SUN_XML_WRITER_GLOBAL, "sxg" },
        { CONTENT_TYPE_STR_APP_FRAMESET, CONTENT_TYPE_APP_FRAMESET, "sfs" },
        { CONTENT_TYPE_STR_APP_GALLERY, CONTENT_TYPE_APP_GALLERY, "gal" },
        { CONTENT_TYPE_STR_APP_GALLERY_THEME, CONTENT_TYPE_APP_GALLERY_THEME,
          "thm" },
        { CONTENT_TYPE_STR_APP_JAR, CONTENT_TYPE_APP_JAR, "jar" },
        { CONTENT_TYPE_STR_APP_MACRO, CONTENT_TYPE_APP_MACRO, "tmp" },
        { CONTENT_TYPE_STR_APP_MSEXCEL, CONTENT_TYPE_APP_MSEXCEL, "xls" },
        { CONTENT_TYPE_STR_APP_MSEXCEL_TEMPL, CONTENT_TYPE_APP_MSEXCEL_TEMPL,
          "xlt" },
        { CONTENT_TYPE_STR_APP_MSPPOINT, CONTENT_TYPE_APP_MSPPOINT, "ppt" },
        { CONTENT_TYPE_STR_APP_MSPPOINT_TEMPL,
          CONTENT_TYPE_APP_MSPPOINT_TEMPL, "pot" },
        { CONTENT_TYPE_STR_APP_MSWORD, CONTENT_TYPE_APP_MSWORD, "doc" },
        { CONTENT_TYPE_STR_APP_MSWORD_TEMPL, CONTENT_TYPE_APP_MSWORD_TEMPL,
          "dot" },
        { CONTENT_TYPE_STR_APP_SCHEDULE_CMB, CONTENT_TYPE_APP_SCHEDULE,
          "tmp" },
        { CONTENT_TYPE_STR_APP_SCHEDULE_EVT, CONTENT_TYPE_APP_SCHEDULE_EVT,
          "tmp" },
        { CONTENT_TYPE_STR_APP_SCHEDULE_FEVT,
          CONTENT_TYPE_APP_SCHEDULE_FORM_EVT, "tmp" },
        { CONTENT_TYPE_STR_APP_SCHEDULE_FTASK,
          CONTENT_TYPE_APP_SCHEDULE_FORM_TASK, "tmp" },
        { CONTENT_TYPE_STR_APP_SCHEDULE_TASK, CONTENT_TYPE_APP_SCHEDULE_TASK,
          "tmp" },
        { CONTENT_TYPE_STR_APP_STARCALC, CONTENT_TYPE_APP_STARCALC, "sdc" },
        { CONTENT_TYPE_STR_APP_STARCHART, CONTENT_TYPE_APP_STARCHART, "sds" },
        { CONTENT_TYPE_STR_APP_STARDRAW, CONTENT_TYPE_APP_STARDRAW, "sda" },
        { CONTENT_TYPE_STR_APP_STARHELP, CONTENT_TYPE_APP_STARHELP, "svh" },
        { CONTENT_TYPE_STR_APP_STARIMAGE, CONTENT_TYPE_APP_STARIMAGE, "sim" },
        { CONTENT_TYPE_STR_APP_STARIMPRESS, CONTENT_TYPE_APP_STARIMPRESS,
          "sdd" },
        { CONTENT_TYPE_STR_APP_STARMAIL_SDM, CONTENT_TYPE_APP_STARMAIL_SDM,
          "sdm" },
        { CONTENT_TYPE_STR_APP_STARMAIL_SMD, CONTENT_TYPE_APP_STARMAIL_SMD,
          "smd" },
        { CONTENT_TYPE_STR_APP_STARMATH, CONTENT_TYPE_APP_STARMATH, "smf" },
        { CONTENT_TYPE_STR_APP_STARWRITER, CONTENT_TYPE_APP_STARWRITER,
          "sdw" },
        { CONTENT_TYPE_STR_APP_STARWRITER_GLOB,
          CONTENT_TYPE_APP_STARWRITER_GLOB, "sgl" },
        { CONTENT_TYPE_STR_APP_CDE_CALENDAR_APP,
          CONTENT_TYPE_APP_CDE_CALENDAR_APP, "appt" },
        { CONTENT_TYPE_STR_APP_ZIP, CONTENT_TYPE_APP_ZIP, "zip" },
        { CONTENT_TYPE_STR_AUDIO_AIFF, CONTENT_TYPE_AUDIO_AIFF, "aif" },
        { CONTENT_TYPE_STR_AUDIO_BASIC, CONTENT_TYPE_AUDIO_BASIC, "au" },
        { CONTENT_TYPE_STR_AUDIO_MIDI, CONTENT_TYPE_AUDIO_MIDI, "mid" },
        { CONTENT_TYPE_STR_AUDIO_WAV, CONTENT_TYPE_AUDIO_WAV, "wav" },
        { CONTENT_TYPE_STR_IMAGE_GENERIC, CONTENT_TYPE_IMAGE_GENERIC, "tmp" },
        { CONTENT_TYPE_STR_IMAGE_GIF, CONTENT_TYPE_IMAGE_GIF, "gif" },
        { CONTENT_TYPE_STR_IMAGE_JPEG, CONTENT_TYPE_IMAGE_JPEG, "jpg" },
        { CONTENT_TYPE_STR_IMAGE_PCX, CONTENT_TYPE_IMAGE_PCX, "pcx" },
        { CONTENT_TYPE_STR_IMAGE_PNG, CONTENT_TYPE_IMAGE_PNG, "png" },
        { CONTENT_TYPE_STR_IMAGE_TIFF, CONTENT_TYPE_IMAGE_TIFF, "tif" },
        { CONTENT_TYPE_STR_IMAGE_BMP, CONTENT_TYPE_IMAGE_BMP, "bmp" },
        { CONTENT_TYPE_STR_INET_MSG_RFC822, CONTENT_TYPE_INET_MESSAGE_RFC822,
          "tmp" },
        { CONTENT_TYPE_STR_INET_MULTI_ALTERNATIVE,
          CONTENT_TYPE_INET_MULTIPART_ALTERNATIVE, "tmp" },
        { CONTENT_TYPE_STR_INET_MULTI_DIGEST,
          CONTENT_TYPE_INET_MULTIPART_DIGEST, "tmp" },
        { CONTENT_TYPE_STR_INET_MULTI_MIXED,
          CONTENT_TYPE_INET_MULTIPART_MIXED, "tmp" },
        { CONTENT_TYPE_STR_INET_MULTI_PARALLEL,
          CONTENT_TYPE_INET_MULTIPART_PARALLEL, "tmp" },
        { CONTENT_TYPE_STR_INET_MULTI_RELATED,
          CONTENT_TYPE_INET_MULTIPART_RELATED, "tmp" },
        { CONTENT_TYPE_STR_TEXT_ICALENDAR, CONTENT_TYPE_TEXT_ICALENDAR,
          "ics" },
        { CONTENT_TYPE_STR_TEXT_HTML, CONTENT_TYPE_TEXT_HTML, "htm" },
        { CONTENT_TYPE_STR_TEXT_PLAIN, CONTENT_TYPE_TEXT_PLAIN, "txt" },
        { CONTENT_TYPE_STR_TEXT_XMLICALENDAR, CONTENT_TYPE_TEXT_XMLICALENDAR,
          "xcs" },
        { CONTENT_TYPE_STR_TEXT_URL, CONTENT_TYPE_TEXT_URL, "url" },
        { CONTENT_TYPE_STR_TEXT_VCALENDAR, CONTENT_TYPE_TEXT_VCALENDAR,
          "vcs" },
        { CONTENT_TYPE_STR_TEXT_VCARD, CONTENT_TYPE_TEXT_VCARD, "vcf" },
        { CONTENT_TYPE_STR_VIDEO_VDO, CONTENT_TYPE_VIDEO_VDO, "vdo" },
        { CONTENT_TYPE_STR_VIDEO_MSVIDEO, CONTENT_TYPE_VIDEO_MSVIDEO, "avi" },
        { CONTENT_TYPE_STR_X_STARMAIL, CONTENT_TYPE_X_STARMAIL, "smd" },
        { CONTENT_TYPE_STR_X_VRML, CONTENT_TYPE_X_VRML, "wrl" }
};

//============================================================================
/** A mapping from extensions to type IDs.  Sorted by extension.
 */
MediaTypeEntry const aStaticExtensionMap[]
    = { { "aif", CONTENT_TYPE_AUDIO_AIFF, "" },
        { "aiff", CONTENT_TYPE_AUDIO_AIFF, "" },
        { "appt", CONTENT_TYPE_APP_CDE_CALENDAR_APP, "" },
        { "au", CONTENT_TYPE_AUDIO_BASIC, "" },
        { "avi", CONTENT_TYPE_VIDEO_MSVIDEO, "" },
        { "bmp", CONTENT_TYPE_IMAGE_BMP, "" },
        { "cgm", CONTENT_TYPE_IMAGE_GENERIC, "" },
        { "doc", CONTENT_TYPE_APP_MSWORD, "" },
        { "dot", CONTENT_TYPE_APP_MSWORD_TEMPL, "" },
        { "dxf", CONTENT_TYPE_IMAGE_GENERIC, "" },
        { "eps", CONTENT_TYPE_IMAGE_GENERIC, "" },
        { "gal", CONTENT_TYPE_APP_GALLERY, "" },
        { "gif", CONTENT_TYPE_IMAGE_GIF, "" },
        { "htm", CONTENT_TYPE_TEXT_HTML, "" },
        { "html", CONTENT_TYPE_TEXT_HTML, "" },
        { "ics", CONTENT_TYPE_TEXT_ICALENDAR, "" },
        { "jar", CONTENT_TYPE_APP_JAR, "" },
        { "jpeg", CONTENT_TYPE_IMAGE_JPEG, "" },
        { "jpg", CONTENT_TYPE_IMAGE_JPEG, "" },
        { "met", CONTENT_TYPE_IMAGE_GENERIC, "" },
        { "mid", CONTENT_TYPE_AUDIO_MIDI, "" },
        { "midi", CONTENT_TYPE_AUDIO_MIDI, "" },
        { "pbm", CONTENT_TYPE_IMAGE_GENERIC, "" },
        { "pcd", CONTENT_TYPE_IMAGE_GENERIC, "" },
        { "pct", CONTENT_TYPE_IMAGE_GENERIC, "" },
        { "pcx", CONTENT_TYPE_IMAGE_PCX, "" },
        { "pdf", CONTENT_TYPE_APP_PDF, "" },
        { "pgm", CONTENT_TYPE_IMAGE_GENERIC, "" },
        { "png", CONTENT_TYPE_IMAGE_PNG, "" },
        { "pot", CONTENT_TYPE_APP_MSPPOINT_TEMPL, "" },
        { "ppm", CONTENT_TYPE_IMAGE_GENERIC, "" },
        { "ppt", CONTENT_TYPE_APP_MSPPOINT, "" },
        { "psd", CONTENT_TYPE_IMAGE_GENERIC, "" },
        { "ras", CONTENT_TYPE_IMAGE_GENERIC, "" },
        { "rtf", CONTENT_TYPE_APP_RTF, "" },
        { "sda", CONTENT_TYPE_APP_VND_DRAW, "" },
        { "sdc", CONTENT_TYPE_APP_VND_CALC, "" },
        { "sdd", CONTENT_TYPE_APP_VND_IMPRESS, "" },
        { "sdm", CONTENT_TYPE_APP_VND_MAIL, "" },
        { "sdp", CONTENT_TYPE_APP_VND_IMPRESSPACKED, "" },
        { "sds", CONTENT_TYPE_APP_VND_CHART, "" },
        { "sdw", CONTENT_TYPE_APP_VND_WRITER, "" },
        { "sd~", CONTENT_TYPE_X_STARMAIL, "" },
        { "sfs", CONTENT_TYPE_APP_FRAMESET , "" },
        { "sgl", CONTENT_TYPE_APP_VND_WRITER_GLOBAL , "" },
        { "sim", CONTENT_TYPE_APP_VND_IMAGE, "" },
        { "smd", CONTENT_TYPE_APP_STARMAIL_SMD, "" }, //CONTENT_TYPE_X_STARMAIL
        { "smf", CONTENT_TYPE_APP_VND_MATH, "" },
        { "svh", CONTENT_TYPE_APP_STARHELP, "" },
        { "svm", CONTENT_TYPE_IMAGE_GENERIC, "" },
        { "sxc", CONTENT_TYPE_APP_VND_SUN_XML_CALC, "" },
        { "sxd", CONTENT_TYPE_APP_VND_SUN_XML_DRAW, "" },
        { "sxg", CONTENT_TYPE_APP_VND_SUN_XML_WRITER_GLOBAL, "" },
        { "sxi", CONTENT_TYPE_APP_VND_SUN_XML_IMPRESS, "" },
        { "sxm", CONTENT_TYPE_APP_VND_SUN_XML_MATH, "" },
        { "sxp", CONTENT_TYPE_APP_VND_SUN_XML_IMPRESSPACKED, "" },
        { "sxs", CONTENT_TYPE_APP_VND_SUN_XML_CHART, "" },
        { "sxw", CONTENT_TYPE_APP_VND_SUN_XML_WRITER, "" },
        { "tga", CONTENT_TYPE_IMAGE_GENERIC, "" },
        { "thm", CONTENT_TYPE_APP_GALLERY_THEME, "" },
        { "tif", CONTENT_TYPE_IMAGE_TIFF, "" },
        { "tiff", CONTENT_TYPE_IMAGE_TIFF, "" },
        { "txt", CONTENT_TYPE_TEXT_PLAIN, "" },
        { "url", CONTENT_TYPE_TEXT_URL, "" },
        { "vcf", CONTENT_TYPE_TEXT_VCARD, "" },
        { "vcs", CONTENT_TYPE_TEXT_VCALENDAR, "" },
        { "vdo", CONTENT_TYPE_VIDEO_VDO, "" },
        { "vor", CONTENT_TYPE_APP_VND_TEMPLATE, "" },
        { "wav", CONTENT_TYPE_AUDIO_WAV, "" },
        { "wmf", CONTENT_TYPE_IMAGE_GENERIC, "" },
        { "wrl", CONTENT_TYPE_X_VRML, "" },
        { "xbm", CONTENT_TYPE_IMAGE_GENERIC, "" },
        { "xcs", CONTENT_TYPE_TEXT_XMLICALENDAR, "" },
        { "xls", CONTENT_TYPE_APP_MSEXCEL, "" },
        { "xlt", CONTENT_TYPE_APP_MSEXCEL_TEMPL, "" },
        { "xlw", CONTENT_TYPE_APP_MSEXCEL, "" },
        { "xpm", CONTENT_TYPE_IMAGE_GENERIC, "" },
        { "zip", CONTENT_TYPE_APP_ZIP, "" } };

//============================================================================
/** A mapping from presentations to type IDs.  Sorted by presentations.
 */
MediaTypeEntry const aStaticPresentationMap[]
    = { { "Binary Data", CONTENT_TYPE_APP_OCTSTREAM, "" },
        { "Bitmap", CONTENT_TYPE_IMAGE_BMP, "" },
        { "DOS Command File", CONTENT_TYPE_APP_OCTSTREAM, "" },
        { "Digital Video", CONTENT_TYPE_VIDEO_MSVIDEO, "" },
        { "Executable", CONTENT_TYPE_APP_OCTSTREAM, "" },
        { "Grafik", CONTENT_TYPE_IMAGE_GENERIC, "" },
        { "HTM", CONTENT_TYPE_TEXT_HTML, "" },
        { "HTML", CONTENT_TYPE_TEXT_HTML, "" },
        { "Hypertext", CONTENT_TYPE_TEXT_HTML, "" },
        { "Icon", CONTENT_TYPE_IMAGE_GENERIC, "" },
        { "Image File", CONTENT_TYPE_IMAGE_GENERIC, "" },
        { "MIDI", CONTENT_TYPE_AUDIO_MIDI, "" },
        { "Master Document", CONTENT_TYPE_APP_VND_WRITER_GLOBAL, "" },
        { "Plain Text", CONTENT_TYPE_TEXT_PLAIN, "" },
        { "StarCalc", CONTENT_TYPE_APP_VND_CALC, "" },
        { "StarCalc 3.0", CONTENT_TYPE_APP_VND_CALC, "" },
        { "StarCalc 4.0", CONTENT_TYPE_APP_VND_CALC, "" },
        { "StarCalc 5.0", CONTENT_TYPE_APP_VND_CALC, "" },
        { "StarCalc Datei", CONTENT_TYPE_APP_VND_CALC, "" },
        { "StarCalc Document", CONTENT_TYPE_APP_VND_CALC, "" },
        { "StarCalc File", CONTENT_TYPE_APP_VND_CALC, "" },
        { "StarChart 3.0", CONTENT_TYPE_APP_VND_CHART, "" },
        { "StarChart 4.0", CONTENT_TYPE_APP_VND_CHART, "" },
        { "StarChart 5.0", CONTENT_TYPE_APP_VND_CHART, "" },
        { "StarChart Document", CONTENT_TYPE_APP_VND_CHART, "" },
        { "StarDraw 3.0", CONTENT_TYPE_APP_VND_DRAW, "" },
        { "StarDraw 5.0", CONTENT_TYPE_APP_VND_DRAW, "" },
        { "StarDraw", CONTENT_TYPE_APP_VND_DRAW, "" },
        { "StarDraw Document", CONTENT_TYPE_APP_VND_DRAW, "" },
        { "StarImpress 4.0", CONTENT_TYPE_APP_VND_IMPRESS, "" },
        { "StarImpress 5.0 (packed)", CONTENT_TYPE_APP_VND_IMPRESSPACKED, "" },
        { "StarImpress 5.0", CONTENT_TYPE_APP_VND_IMPRESS, "" },
        { "StarImpress Document", CONTENT_TYPE_APP_VND_IMPRESS, "" },
        { "StarMath 3.0", CONTENT_TYPE_APP_VND_MATH, "" },
        { "StarMath 4.0", CONTENT_TYPE_APP_VND_MATH, "" },
        { "StarMath 5.0", CONTENT_TYPE_APP_VND_MATH, "" },
        { "StarMath Document", CONTENT_TYPE_APP_VND_MATH, "" },
        { "StarMessage5", CONTENT_TYPE_APP_VND_MAIL, "" },
        { "StarOffice XML (Calc)", CONTENT_TYPE_APP_VND_SUN_XML_CALC, "" },
        { "StarOffice XML (Impress)", CONTENT_TYPE_APP_VND_SUN_XML_IMPRESS, "" },
        { "StarOffice XML (Draw)", CONTENT_TYPE_APP_VND_SUN_XML_DRAW, "" },
        { "StarOffice XML (Chart)", CONTENT_TYPE_APP_VND_SUN_XML_CHART, "" },
        { "StarOffice XML (Writer)", CONTENT_TYPE_APP_VND_SUN_XML_WRITER, "" },
        { "StarWriter", CONTENT_TYPE_APP_VND_WRITER, "" },
        { "StarWriter 3.0", CONTENT_TYPE_APP_VND_WRITER, "" },
        { "StarWriter 4.0", CONTENT_TYPE_APP_VND_WRITER, "" },
        { "StarWriter 5.0", CONTENT_TYPE_APP_VND_WRITER, "" },
        { "StarWriter Document", CONTENT_TYPE_APP_VND_WRITER, "" },
        { "StarWriter/Global 5.0", CONTENT_TYPE_APP_VND_WRITER_GLOBAL, "" },
        { "StarWriter/Global Document", CONTENT_TYPE_APP_VND_WRITER_GLOBAL, "" },
        { "StarWriter/Web 5.0", CONTENT_TYPE_APP_VND_WRITER_WEB, "" },
        { "StarWriterGlobal Document", CONTENT_TYPE_APP_VND_WRITER_GLOBAL, "" },
        { "StarWriterHtml Document", CONTENT_TYPE_APP_VND_WRITER_WEB, "" },
        { "UniformResourceLocator", CONTENT_TYPE_TEXT_URL, "" },
        { "text/html", CONTENT_TYPE_TEXT_HTML, "" } };

//============================================================================
//
//  Registration
//
//============================================================================

// static
Registration * Registration::m_pRegistration = 0;

//============================================================================
Registration::~Registration()
{
    {for (ULONG i = 0; i < m_aTypeIDMap.Count(); ++i)
        delete static_cast< TypeIDMapEntry * >(m_aTypeIDMap.GetObject(i));
    }
    m_aTypeIDMap.Clear();
    {for (USHORT i = 0; i < m_aTypeNameMap.Count(); ++i)
        delete static_cast< TypeNameMapEntry * >(m_aTypeNameMap.GetObject(i));
    }
    m_aTypeNameMap.Remove(USHORT(0), m_aTypeNameMap.Count());
    {for (USHORT i = 0; i < m_aExtensionMap.Count(); ++i)
        delete
            static_cast< ExtensionMapEntry * >(m_aExtensionMap.GetObject(i));
    }
    m_aExtensionMap.Remove(USHORT(0), m_aExtensionMap.Count());
}

//============================================================================
// static
TypeNameMapEntry * Registration::getExtensionEntry(UniString const &
                                                       rTypeName)
{
    if (m_pRegistration)
    {
        UniString aTheTypeName = rTypeName;
        aTheTypeName.ToLowerAscii();
        USHORT nPos;
        if (m_pRegistration->m_aTypeNameMap.Seek_Entry(&aTheTypeName, &nPos))
            return static_cast< TypeNameMapEntry * >(m_pRegistration->
                                                         m_aTypeNameMap.
                                                             GetObject(nPos));
    }
    return 0;
}

//============================================================================
// static
INetContentType Registration::RegisterContentType(UniString const & rTypeName,
                                                  UniString const &
                                                      rPresentation,
                                                  UniString const *
                                                      pExtension,
                                                  UniString const *
                                                      pSystemFileType)
{
    if (!m_pRegistration)
        m_pRegistration = new Registration;

    DBG_ASSERT(GetContentType(rTypeName) == CONTENT_TYPE_UNKNOWN,
               "Registration::RegisterContentType(): Already registered");

    INetContentType eTypeID
        = INetContentType(m_pRegistration->m_nNextDynamicID++);
    UniString aTheTypeName = rTypeName;
    aTheTypeName.ToLowerAscii();

    TypeIDMapEntry * pTypeIDMapEntry = new TypeIDMapEntry;
    pTypeIDMapEntry->m_aTypeName = aTheTypeName;
    pTypeIDMapEntry->m_aPresentation = rPresentation;
    if (pSystemFileType)
        pTypeIDMapEntry->m_aSystemFileType = *pSystemFileType;
    m_pRegistration->m_aTypeIDMap.Insert(eTypeID, pTypeIDMapEntry);

    TypeNameMapEntry * pTypeNameMapEntry = new TypeNameMapEntry(aTheTypeName);
    if (pExtension)
        pTypeNameMapEntry->m_aExtension = *pExtension;
    pTypeNameMapEntry->m_eTypeID = eTypeID;
    m_pRegistration->m_aTypeNameMap.Insert(pTypeNameMapEntry);

    if (pExtension)
    {
        ExtensionMapEntry * pExtensionMapEntry
            = new ExtensionMapEntry(*pExtension);
        pExtensionMapEntry->m_eTypeID = eTypeID;
        m_pRegistration->m_aExtensionMap.Insert(pExtensionMapEntry);
    }

    return eTypeID;
}

//============================================================================
// static
INetContentType Registration::GetContentType(UniString const & rTypeName)
{
    if (!m_pRegistration)
        m_pRegistration = new Registration;

    UniString aTheTypeName = rTypeName;
    aTheTypeName.ToLowerAscii();
    USHORT nPos;
    return m_pRegistration->m_aTypeNameMap.Seek_Entry(&aTheTypeName, &nPos) ?
               static_cast< TypeNameMapEntry * >(m_pRegistration->
                                                     m_aTypeNameMap.
                                                         GetObject(nPos))->
                   m_eTypeID :
               CONTENT_TYPE_UNKNOWN;
}

//============================================================================
// static
UniString Registration::GetContentType(INetContentType eTypeID)
{
    if (!m_pRegistration)
        m_pRegistration = new Registration;

    TypeIDMapEntry * pEntry
        = static_cast< TypeIDMapEntry * >(m_pRegistration->
                                              m_aTypeIDMap.Get(eTypeID));
    return pEntry ? pEntry->m_aTypeName : UniString();
}

//============================================================================
// static
INetContentType Registration::GetContentType4Extension(UniString const &
                                                           rExtension)
{
    if (!m_pRegistration)
        m_pRegistration = new Registration;

    USHORT nPos;
    return m_pRegistration->
                   m_aExtensionMap.
                       Seek_Entry(const_cast< UniString * >(&rExtension),
                                  &nPos) ?
               static_cast< ExtensionMapEntry * >(m_pRegistration->
                                                      m_aExtensionMap.
                                                          GetObject(nPos))->
                   m_eTypeID :
               CONTENT_TYPE_UNKNOWN;
}

//============================================================================
//
//  seekEntry
//
//============================================================================

MediaTypeEntry const * seekEntry(UniString const & rTypeName,
                                 MediaTypeEntry const * pMap, sal_Size nSize)
{
#if defined DBG_UTIL || defined INETTYPE_DEBUG
    static bool bChecked = false;
    if (!bChecked)
    {
        for (sal_Size i = 0; i < nSize - 1; ++i)
            DBG_ASSERT(pMap[i].m_pTypeName < pMap[i + 1].m_pTypeName,
                       "seekEntry(): Bad map");
        bChecked = true;
    }
#endif // DBG_UTIL, INETTYPE_DEBUG

    sal_Size nLow = 0;
    sal_Size nHigh = nSize;
    while (nLow != nHigh)
    {
        sal_Size nMiddle = (nLow + nHigh) / 2;
        MediaTypeEntry const * pEntry = pMap + nMiddle;
        switch (rTypeName.CompareIgnoreCaseToAscii(pEntry->m_pTypeName))
        {
            case COMPARE_LESS:
                nHigh = nMiddle;
                break;

            case COMPARE_EQUAL:
                return pEntry;

            case COMPARE_GREATER:
                nLow = nMiddle + 1;
                break;
        }
    }
    return 0;
}

//============================================================================
//static
INetContentType INetContentTypes::RegisterContentType(UniString const &
                                                          rTypeName,
                                                      UniString const &
                                                          rPresentation,
                                                      UniString const *
                                                          pExtension,
                                                      UniString const *
                                                          pSystemFileType)
{
    INetContentType eTypeID = GetContentType(rTypeName);
    if (eTypeID == CONTENT_TYPE_UNKNOWN)
        eTypeID = Registration::RegisterContentType(rTypeName, rPresentation,
                                                    pExtension,
                                                    pSystemFileType);
    else if (eTypeID > CONTENT_TYPE_LAST)
    {
        TypeIDMapEntry * pTypeEntry = Registration::getEntry(eTypeID);
        if (pTypeEntry)
        {
            if (rPresentation.Len() != 0)
                pTypeEntry->m_aPresentation = rPresentation;
            if (pSystemFileType)
                pTypeEntry->m_aSystemFileType = *pSystemFileType;
        }
        if (pExtension)
        {
            TypeNameMapEntry * pEntry
                = Registration::getExtensionEntry(rTypeName);
            if (pEntry)
                pEntry->m_aExtension = *pExtension;
        }
    }
    return eTypeID;
}

//============================================================================
// static
INetContentType INetContentTypes::GetContentType(UniString const & rTypeName)
{
    UniString aType;
    UniString aSubType;
    if (parse(rTypeName, aType, aSubType))
    {
        aType += '/';
        aType += aSubType;
        MediaTypeEntry const * pEntry = seekEntry(aType, aStaticTypeNameMap,
                                                  CONTENT_TYPE_LAST + 1);
        return pEntry ? pEntry->m_eTypeID :
                        Registration::GetContentType(aType);
    }
    else
        return
            rTypeName.EqualsIgnoreCaseAscii(CONTENT_TYPE_STR_X_STARMAIL) ?
                CONTENT_TYPE_X_STARMAIL : CONTENT_TYPE_UNKNOWN;
            // the content type "x-starmail" has no sub type
}

//============================================================================
//static
UniString INetContentTypes::GetContentType(INetContentType eTypeID)
{
    static sal_Char const * aMap[CONTENT_TYPE_LAST + 1];
    static bool bInitialized = false;
    if (!bInitialized)
    {
        for (sal_Size i = 0; i <= CONTENT_TYPE_LAST; ++i)
            aMap[aStaticTypeNameMap[i].m_eTypeID]
                = aStaticTypeNameMap[i].m_pTypeName;
        aMap[CONTENT_TYPE_UNKNOWN] = CONTENT_TYPE_STR_APP_OCTSTREAM;
        aMap[CONTENT_TYPE_TEXT_PLAIN] = CONTENT_TYPE_STR_TEXT_PLAIN
                                            "; charset=iso-8859-1";
        bInitialized = true;
    }

    UniString aTypeName = eTypeID <= CONTENT_TYPE_LAST ?
                              UniString::CreateFromAscii(aMap[eTypeID]) :
                              Registration::GetContentType(eTypeID);
    if (aTypeName.Len() == 0)
    {
        DBG_ERROR("INetContentTypes::GetContentType(): Bad ID");
        return
            UniString::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(
                                           CONTENT_TYPE_STR_APP_OCTSTREAM));
    }
    return aTypeName;
}

//============================================================================
//static
UniString INetContentTypes::GetPresentation(INetContentType, const ::com::sun::star::lang::Locale&  )
{
    UniString aPresentation;
    return aPresentation;
}

//============================================================================
//static
INetContentType INetContentTypes::GetContentType4Extension(UniString const &
                                                               rExtension)
{
    MediaTypeEntry const * pEntry = seekEntry(rExtension, aStaticExtensionMap,
                                              sizeof aStaticExtensionMap
                                                  / sizeof (MediaTypeEntry));
    if (pEntry)
        return pEntry->m_eTypeID;
    INetContentType eTypeID
        = Registration::GetContentType4Extension(rExtension);
    return eTypeID == CONTENT_TYPE_UNKNOWN ? CONTENT_TYPE_APP_OCTSTREAM :
                                             eTypeID;
}

//============================================================================
// static
bool INetContentTypes::parse(ByteString const & rMediaType,
                             ByteString & rType, ByteString & rSubType,
                             INetContentTypeParameterList * pParameters)
{
    sal_Char const * p = rMediaType.GetBuffer();
    sal_Char const * pEnd = p + rMediaType.Len();

    p = INetMIME::skipLinearWhiteSpaceComment(p, pEnd);
    sal_Char const * pToken = p;
    bool bDowncase = false;
    while (p != pEnd && INetMIME::isTokenChar(*p))
    {
        bDowncase = bDowncase || INetMIME::isUpperCase(*p);
        ++p;
    }
    if (p == pToken)
        return false;
    rType = ByteString(pToken, sal::static_int_cast< xub_StrLen >(p - pToken));
    if (bDowncase)
        rType.ToLowerAscii();

    p = INetMIME::skipLinearWhiteSpaceComment(p, pEnd);
    if (p == pEnd || *p++ != '/')
        return false;

    p = INetMIME::skipLinearWhiteSpaceComment(p, pEnd);
    pToken = p;
    bDowncase = false;
    while (p != pEnd && INetMIME::isTokenChar(*p))
    {
        bDowncase = bDowncase || INetMIME::isUpperCase(*p);
        ++p;
    }
    if (p == pToken)
        return false;
    rSubType = ByteString(
        pToken, sal::static_int_cast< xub_StrLen >(p - pToken));
    if (bDowncase)
        rSubType.ToLowerAscii();

    return INetMIME::scanParameters(p, pEnd, pParameters) == pEnd;
}

//============================================================================
// static
bool INetContentTypes::parse(UniString const & rMediaType,
                             UniString & rType, UniString & rSubType,
                             INetContentTypeParameterList * pParameters)
{
    sal_Unicode const * p = rMediaType.GetBuffer();
    sal_Unicode const * pEnd = p + rMediaType.Len();

    p = INetMIME::skipLinearWhiteSpaceComment(p, pEnd);
    sal_Unicode const * pToken = p;
    bool bDowncase = false;
    while (p != pEnd && INetMIME::isTokenChar(*p))
    {
        bDowncase = bDowncase || INetMIME::isUpperCase(*p);
        ++p;
    }
    if (p == pToken)
        return false;
    rType = UniString(pToken, sal::static_int_cast< xub_StrLen >(p - pToken));
    if (bDowncase)
        rType.ToLowerAscii();

    p = INetMIME::skipLinearWhiteSpaceComment(p, pEnd);
    if (p == pEnd || *p++ != '/')
        return false;

    p = INetMIME::skipLinearWhiteSpaceComment(p, pEnd);
    pToken = p;
    bDowncase = false;
    while (p != pEnd && INetMIME::isTokenChar(*p))
    {
        bDowncase = bDowncase || INetMIME::isUpperCase(*p);
        ++p;
    }
    if (p == pToken)
        return false;
    rSubType = UniString(
        pToken, sal::static_int_cast< xub_StrLen >(p - pToken));
    if (bDowncase)
        rSubType.ToLowerAscii();

    return INetMIME::scanParameters(p, pEnd, pParameters) == pEnd;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
