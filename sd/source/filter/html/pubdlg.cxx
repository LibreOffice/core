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

#include <com/sun/star/beans/PropertyValue.hpp>
#include <comphelper/sequence.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/FilterConfigItem.hxx>
#include <vcl/image.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <sal/log.hxx>
#include <svtools/valueset.hxx>
#include <svtools/colrdlg.hxx>
#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
#include <tools/GenericTypeSerializer.hxx>
#include <sdiocmpt.hxx>
#include <sfx2/docfile.hxx>
#include <pres.hxx>
#include <unotools/useroptions.hxx>
#include <unotools/pathoptions.hxx>

#include <sdresid.hxx>
#include <strings.hrc>
#include "htmlattr.hxx"
#include "htmlpublishmode.hxx"
#include <helpids.h>
#include "buttonset.hxx"
#include <strings.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::beans;

#define NOOFPAGES 6

//ID for the config-data with the HTML-settings
const sal_uInt16 nMagic = sal_uInt16(0x1977);

// Key for the soffice.ini
constexpr OUStringLiteral KEY_QUALITY = u"JPG-EXPORT-QUALITY";

static SvStream& operator>>(SvStream& rIn, SdPublishingDesign& rDesign);

static SvStream& WriteSdPublishingDesign(SvStream& rOut, const SdPublishingDesign& rDesign);

// This class has all the settings for the HTML-export autopilot
class SdPublishingDesign
{
public:
    OUString m_aDesignName;

    HtmlPublishMode m_eMode;

    // special WebCast options
    PublishingScript m_eScript;
    OUString m_aCGI;
    OUString m_aURL;

    // special Kiosk options
    bool m_bAutoSlide;
    sal_uInt32 m_nSlideDuration;
    bool m_bEndless;

    // special HTML options
    bool m_bContentPage;
    bool m_bNotes;

    // misc options
    sal_uInt16 m_nResolution;
    OUString m_aCompression;
    PublishingFormat m_eFormat;
    bool m_bSlideSound;
    bool m_bHiddenSlides;

    // title page information
    OUString m_aAuthor;
    OUString m_aEMail;
    OUString m_aWWW;
    OUString m_aMisc;
    bool m_bDownload;
    bool m_bCreated; // not used

    // buttons and colorscheme
    sal_Int16 m_nButtonThema;
    bool m_bUserAttr;
    Color m_aBackColor;
    Color m_aTextColor;
    Color m_aLinkColor;
    Color m_aVLinkColor;
    Color m_aALinkColor;
    bool m_bUseAttribs;
    bool m_bUseColor;

    SdPublishingDesign();

    bool operator==(const SdPublishingDesign& rDesign) const;
    friend SvStream& operator>>(SvStream& rIn, SdPublishingDesign& rDesign);
    friend SvStream& WriteSdPublishingDesign(SvStream& rOut, const SdPublishingDesign& rDesign);
};

// load Default-settings
SdPublishingDesign::SdPublishingDesign()
    : m_eMode(PUBLISH_HTML)
    , m_eScript(SCRIPT_ASP)
    , m_bAutoSlide(true)
    , m_nSlideDuration(15)
    , m_bEndless(true)
    , m_bContentPage(true)
    , m_bNotes(true)
    , m_nResolution(PUB_LOWRES_WIDTH)
    , m_eFormat(FORMAT_PNG)
    , m_bSlideSound(true)
    , m_bHiddenSlides(false)
    , m_bDownload(false)
    , m_bCreated(false)
    , m_nButtonThema(-1)
    , m_bUserAttr(false)
    , m_aBackColor(COL_WHITE)
    , m_aTextColor(COL_BLACK)
    , m_aLinkColor(COL_BLUE)
    , m_aVLinkColor(COL_LIGHTGRAY)
    , m_aALinkColor(COL_GRAY)
    , m_bUseAttribs(true)
    , m_bUseColor(true)
{
    FilterConfigItem aFilterConfigItem(u"Office.Common/Filter/Graphic/Export/JPG");
    sal_Int32 nCompression = aFilterConfigItem.ReadInt32(KEY_QUALITY, 75);
    m_aCompression = OUString::number(nCompression) + "%";

    SvtUserOptions aUserOptions;
    m_aAuthor = aUserOptions.GetFirstName();
    if (!m_aAuthor.isEmpty() && !aUserOptions.GetLastName().isEmpty())
        m_aAuthor += " ";
    m_aAuthor += aUserOptions.GetLastName();
    m_aEMail = aUserOptions.GetEmail();
}

// Compares the values without paying attention to the name
bool SdPublishingDesign::operator==(const SdPublishingDesign& rDesign) const
{
    return (
        m_eMode == rDesign.m_eMode && m_nResolution == rDesign.m_nResolution
        && m_aCompression == rDesign.m_aCompression && m_eFormat == rDesign.m_eFormat
        && m_bHiddenSlides == rDesign.m_bHiddenSlides
        && ( // compare html options
               (m_eMode != PUBLISH_HTML && m_eMode != PUBLISH_FRAMES)
               || (m_bContentPage == rDesign.m_bContentPage && m_bNotes == rDesign.m_bNotes
                   && m_aAuthor == rDesign.m_aAuthor && m_aEMail == rDesign.m_aEMail
                   && m_aWWW == rDesign.m_aWWW && m_aMisc == rDesign.m_aMisc
                   && m_bDownload == rDesign.m_bDownload && m_nButtonThema == rDesign.m_nButtonThema
                   && m_bUserAttr == rDesign.m_bUserAttr && m_aBackColor == rDesign.m_aBackColor
                   && m_aTextColor == rDesign.m_aTextColor && m_aLinkColor == rDesign.m_aLinkColor
                   && m_aVLinkColor == rDesign.m_aVLinkColor
                   && m_aALinkColor == rDesign.m_aALinkColor
                   && m_bUseAttribs == rDesign.m_bUseAttribs
                   && m_bSlideSound == rDesign.m_bSlideSound && m_bUseColor == rDesign.m_bUseColor))
        && ( // compare kiosk options
               (m_eMode != PUBLISH_KIOSK)
               || (m_bAutoSlide == rDesign.m_bAutoSlide && m_bSlideSound == rDesign.m_bSlideSound
                   && (!m_bAutoSlide
                       || (m_nSlideDuration == rDesign.m_nSlideDuration
                           && m_bEndless == rDesign.m_bEndless))))
        && ( // compare WebCast options
               (m_eMode != PUBLISH_WEBCAST)
               || (m_eScript == rDesign.m_eScript
                   && (m_eScript != SCRIPT_PERL
                       || (m_aURL == rDesign.m_aURL && m_aCGI == rDesign.m_aCGI)))));
}

// Load the design from the stream
SvStream& operator>>(SvStream& rIn, SdPublishingDesign& rDesign)
{
    SdIOCompat aIO(rIn, StreamMode::READ);

    sal_uInt16 nTemp16;
    tools::GenericTypeSerializer aSerializer(rIn);

    rDesign.m_aDesignName = read_uInt16_lenPrefixed_uInt8s_ToOUString(rIn, RTL_TEXTENCODING_UTF8);
    rIn.ReadUInt16(nTemp16);
    rDesign.m_eMode = static_cast<HtmlPublishMode>(nTemp16);
    rIn.ReadCharAsBool(rDesign.m_bContentPage);
    rIn.ReadCharAsBool(rDesign.m_bNotes);
    rIn.ReadUInt16(rDesign.m_nResolution);
    rDesign.m_aCompression = read_uInt16_lenPrefixed_uInt8s_ToOUString(rIn, RTL_TEXTENCODING_UTF8);
    rIn.ReadUInt16(nTemp16);
    rDesign.m_eFormat = static_cast<PublishingFormat>(nTemp16);
    rDesign.m_aAuthor = read_uInt16_lenPrefixed_uInt8s_ToOUString(rIn, RTL_TEXTENCODING_UTF8);
    rDesign.m_aEMail = read_uInt16_lenPrefixed_uInt8s_ToOUString(rIn, RTL_TEXTENCODING_UTF8);
    rDesign.m_aWWW = read_uInt16_lenPrefixed_uInt8s_ToOUString(rIn, RTL_TEXTENCODING_UTF8);
    rDesign.m_aMisc = read_uInt16_lenPrefixed_uInt8s_ToOUString(rIn, RTL_TEXTENCODING_UTF8);
    rIn.ReadCharAsBool(rDesign.m_bDownload);
    rIn.ReadCharAsBool(rDesign.m_bCreated); // not used
    rIn.ReadInt16(rDesign.m_nButtonThema);
    rIn.ReadCharAsBool(rDesign.m_bUserAttr);
    aSerializer.readColor(rDesign.m_aBackColor);
    aSerializer.readColor(rDesign.m_aTextColor);
    aSerializer.readColor(rDesign.m_aLinkColor);
    aSerializer.readColor(rDesign.m_aVLinkColor);
    aSerializer.readColor(rDesign.m_aALinkColor);
    rIn.ReadCharAsBool(rDesign.m_bUseAttribs);
    rIn.ReadCharAsBool(rDesign.m_bUseColor);

    rIn.ReadUInt16(nTemp16);
    rDesign.m_eScript = static_cast<PublishingScript>(nTemp16);
    rDesign.m_aURL = read_uInt16_lenPrefixed_uInt8s_ToOUString(rIn, RTL_TEXTENCODING_UTF8);
    rDesign.m_aCGI = read_uInt16_lenPrefixed_uInt8s_ToOUString(rIn, RTL_TEXTENCODING_UTF8);

    rIn.ReadCharAsBool(rDesign.m_bAutoSlide);
    rIn.ReadUInt32(rDesign.m_nSlideDuration);
    rIn.ReadCharAsBool(rDesign.m_bEndless);
    rIn.ReadCharAsBool(rDesign.m_bSlideSound);
    rIn.ReadCharAsBool(rDesign.m_bHiddenSlides);

    return rIn;
}

// Set the design to the stream
SvStream& WriteSdPublishingDesign(SvStream& rOut, const SdPublishingDesign& rDesign)
{
    // The last parameter is the versionnumber of the code
    SdIOCompat aIO(rOut, StreamMode::WRITE, 0);

    tools::GenericTypeSerializer aSerializer(rOut);

    // Name
    write_uInt16_lenPrefixed_uInt8s_FromOUString(rOut, rDesign.m_aDesignName,
                                                 RTL_TEXTENCODING_UTF8);

    rOut.WriteUInt16(rDesign.m_eMode);
    rOut.WriteBool(rDesign.m_bContentPage);
    rOut.WriteBool(rDesign.m_bNotes);
    rOut.WriteUInt16(rDesign.m_nResolution);
    write_uInt16_lenPrefixed_uInt8s_FromOUString(rOut, rDesign.m_aCompression,
                                                 RTL_TEXTENCODING_UTF8);
    rOut.WriteUInt16(rDesign.m_eFormat);
    write_uInt16_lenPrefixed_uInt8s_FromOUString(rOut, rDesign.m_aAuthor, RTL_TEXTENCODING_UTF8);
    write_uInt16_lenPrefixed_uInt8s_FromOUString(rOut, rDesign.m_aEMail, RTL_TEXTENCODING_UTF8);
    write_uInt16_lenPrefixed_uInt8s_FromOUString(rOut, rDesign.m_aWWW, RTL_TEXTENCODING_UTF8);
    write_uInt16_lenPrefixed_uInt8s_FromOUString(rOut, rDesign.m_aMisc, RTL_TEXTENCODING_UTF8);
    rOut.WriteBool(rDesign.m_bDownload);
    rOut.WriteBool(rDesign.m_bCreated); // not used
    rOut.WriteInt16(rDesign.m_nButtonThema);
    rOut.WriteBool(rDesign.m_bUserAttr);
    aSerializer.writeColor(rDesign.m_aBackColor);
    aSerializer.writeColor(rDesign.m_aTextColor);
    aSerializer.writeColor(rDesign.m_aLinkColor);
    aSerializer.writeColor(rDesign.m_aVLinkColor);
    aSerializer.writeColor(rDesign.m_aALinkColor);
    rOut.WriteBool(rDesign.m_bUseAttribs);
    rOut.WriteBool(rDesign.m_bUseColor);

    rOut.WriteUInt16(rDesign.m_eScript);
    write_uInt16_lenPrefixed_uInt8s_FromOUString(rOut, rDesign.m_aURL, RTL_TEXTENCODING_UTF8);
    write_uInt16_lenPrefixed_uInt8s_FromOUString(rOut, rDesign.m_aCGI, RTL_TEXTENCODING_UTF8);

    rOut.WriteBool(rDesign.m_bAutoSlide);
    rOut.WriteUInt32(rDesign.m_nSlideDuration);
    rOut.WriteBool(rDesign.m_bEndless);
    rOut.WriteBool(rDesign.m_bSlideSound);
    rOut.WriteBool(rDesign.m_bHiddenSlides);

    return rOut;
}

namespace
{
// Dialog for the entry of the name of the design
class SdDesignNameDlg : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::Entry> m_xEdit;
    std::unique_ptr<weld::Button> m_xBtnOK;

public:
    SdDesignNameDlg(weld::Window* pWindow, const OUString& aName);
    OUString GetDesignName() const;
    DECL_LINK(ModifyHdl, weld::Entry&, void);
};
}

// SdDesignNameDlg Methods
SdDesignNameDlg::SdDesignNameDlg(weld::Window* pWindow, const OUString& rName)
    : GenericDialogController(pWindow, "modules/sdraw/ui/namedesign.ui", "NameDesignDialog")
    , m_xEdit(m_xBuilder->weld_entry("entry"))
    , m_xBtnOK(m_xBuilder->weld_button("ok"))
{
    m_xEdit->connect_changed(LINK(this, SdDesignNameDlg, ModifyHdl));
    m_xEdit->set_text(rName);
    m_xBtnOK->set_sensitive(!rName.isEmpty());
}

OUString SdDesignNameDlg::GetDesignName() const { return m_xEdit->get_text(); }

IMPL_LINK_NOARG(SdDesignNameDlg, ModifyHdl, weld::Entry&, void)
{
    m_xBtnOK->set_sensitive(!m_xEdit->get_text().isEmpty());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
