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
#include <pubdlg.hxx>
#include "htmlattr.hxx"
#include "htmlex.hxx"
#include "htmlpublishmode.hxx"
#include <helpids.h>
#include "buttonset.hxx"
#include <strings.hxx>

using namespace std;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;

#define NOOFPAGES 6

//ID for the config-data with the HTML-settings
const sal_uInt16 nMagic = sal_uInt16(0x1977);

// Key for the soffice.ini
#define KEY_QUALITY     "JPG-EXPORT-QUALITY"

// The Help-IDs of the pages
const char* const aPageHelpIds[NOOFPAGES] =
{
    HID_SD_HTMLEXPORT_PAGE1,
    HID_SD_HTMLEXPORT_PAGE2,
    HID_SD_HTMLEXPORT_PAGE3,
    HID_SD_HTMLEXPORT_PAGE4,
    HID_SD_HTMLEXPORT_PAGE5,
    HID_SD_HTMLEXPORT_PAGE6
};

static SvStream& operator >> (SvStream& rIn, SdPublishingDesign& rDesign);

static SvStream& WriteSdPublishingDesign(SvStream& rOut, const SdPublishingDesign& rDesign);

// This class has all the settings for the HTML-export autopilot
class SdPublishingDesign
{
public:
    OUString m_aDesignName;

    HtmlPublishMode m_eMode;

    // special WebCast options
    PublishingScript m_eScript;
    OUString         m_aCGI;
    OUString         m_aURL;

    // special Kiosk options
    bool    m_bAutoSlide;
    sal_uInt32  m_nSlideDuration;
    bool    m_bEndless;

    // special HTML options
    bool    m_bContentPage;
    bool    m_bNotes;

    // misc options
    sal_uInt16  m_nResolution;
    OUString    m_aCompression;
    PublishingFormat m_eFormat;
    bool    m_bSlideSound;
    bool    m_bHiddenSlides;

    // title page information
    OUString    m_aAuthor;
    OUString    m_aEMail;
    OUString    m_aWWW;
    OUString    m_aMisc;
    bool    m_bDownload;
    bool    m_bCreated;         // not used

    // buttons and colorscheme
    sal_Int16   m_nButtonThema;
    bool    m_bUserAttr;
    Color   m_aBackColor;
    Color   m_aTextColor;
    Color   m_aLinkColor;
    Color   m_aVLinkColor;
    Color   m_aALinkColor;
    bool    m_bUseAttribs;
    bool    m_bUseColor;

    SdPublishingDesign();

    bool operator ==(const SdPublishingDesign & rDesign) const;
    friend SvStream& operator >> (SvStream& rIn, SdPublishingDesign& rDesign);
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
    sal_Int32 nCompression = aFilterConfigItem.ReadInt32( KEY_QUALITY, 75 );
    m_aCompression = OUString::number(nCompression) + "%";

    SvtUserOptions aUserOptions;
    m_aAuthor       = aUserOptions.GetFirstName();
    if (!m_aAuthor.isEmpty() && !aUserOptions.GetLastName().isEmpty())
        m_aAuthor += " ";
    m_aAuthor      += aUserOptions.GetLastName();
    m_aEMail        = aUserOptions.GetEmail();
}

// Compares the values without paying attention to the name
bool SdPublishingDesign::operator ==(const SdPublishingDesign & rDesign) const
{
    return
    (
        m_eMode        == rDesign.m_eMode &&
        m_nResolution  == rDesign.m_nResolution &&
        m_aCompression == rDesign.m_aCompression &&
        m_eFormat      == rDesign.m_eFormat &&
        m_bHiddenSlides == rDesign.m_bHiddenSlides &&
        (  // compare html options
            (m_eMode != PUBLISH_HTML && m_eMode != PUBLISH_FRAMES) ||
            (
                m_bContentPage == rDesign.m_bContentPage &&
                m_bNotes       == rDesign.m_bNotes &&
                m_aAuthor      == rDesign.m_aAuthor &&
                m_aEMail       == rDesign.m_aEMail &&
                m_aWWW         == rDesign.m_aWWW &&
                m_aMisc        == rDesign.m_aMisc &&
                m_bDownload    == rDesign.m_bDownload &&
                m_nButtonThema == rDesign.m_nButtonThema &&
                m_bUserAttr    == rDesign.m_bUserAttr &&
                m_aBackColor   == rDesign.m_aBackColor &&
                m_aTextColor   == rDesign.m_aTextColor &&
                m_aLinkColor   == rDesign.m_aLinkColor &&
                m_aVLinkColor  == rDesign.m_aVLinkColor &&
                m_aALinkColor  == rDesign.m_aALinkColor &&
                m_bUseAttribs  == rDesign.m_bUseAttribs &&
                m_bSlideSound  == rDesign.m_bSlideSound &&
                m_bUseColor    == rDesign.m_bUseColor
            )
        ) &&
        (   // compare kiosk options
            (m_eMode != PUBLISH_KIOSK) ||
            (
                m_bAutoSlide  == rDesign.m_bAutoSlide &&
                m_bSlideSound == rDesign.m_bSlideSound &&
                (
                    !m_bAutoSlide ||
                    (
                        m_nSlideDuration == rDesign.m_nSlideDuration &&
                        m_bEndless == rDesign.m_bEndless
                    )
                )
            )
        ) &&
        (   // compare WebCast options
            (m_eMode != PUBLISH_WEBCAST) ||
            (
                m_eScript == rDesign.m_eScript &&
                (
                    m_eScript != SCRIPT_PERL ||
                    (
                        m_aURL == rDesign.m_aURL &&
                        m_aCGI == rDesign.m_aCGI
                    )
                )
            )
        )
    );
}

// Load the design from the stream
SvStream& operator >> (SvStream& rIn, SdPublishingDesign& rDesign)
{
    SdIOCompat aIO(rIn, StreamMode::READ);

    sal_uInt16 nTemp16;
    tools::GenericTypeSerializer aSerializer(rIn);

    rDesign.m_aDesignName = read_uInt16_lenPrefixed_uInt8s_ToOUString(rIn,
        RTL_TEXTENCODING_UTF8);
    rIn.ReadUInt16( nTemp16 );
    rDesign.m_eMode = static_cast<HtmlPublishMode>(nTemp16);
    rIn.ReadCharAsBool( rDesign.m_bContentPage );
    rIn.ReadCharAsBool( rDesign.m_bNotes );
    rIn.ReadUInt16( rDesign.m_nResolution );
    rDesign.m_aCompression = read_uInt16_lenPrefixed_uInt8s_ToOUString(rIn,
        RTL_TEXTENCODING_UTF8);
    rIn.ReadUInt16( nTemp16 );
    rDesign.m_eFormat = static_cast<PublishingFormat>(nTemp16);
    rDesign.m_aAuthor = read_uInt16_lenPrefixed_uInt8s_ToOUString(rIn,
        RTL_TEXTENCODING_UTF8);
    rDesign.m_aEMail = read_uInt16_lenPrefixed_uInt8s_ToOUString(rIn,
        RTL_TEXTENCODING_UTF8);
    rDesign.m_aWWW = read_uInt16_lenPrefixed_uInt8s_ToOUString(rIn,
        RTL_TEXTENCODING_UTF8);
    rDesign.m_aMisc = read_uInt16_lenPrefixed_uInt8s_ToOUString(rIn,
        RTL_TEXTENCODING_UTF8);
    rIn.ReadCharAsBool( rDesign.m_bDownload );
    rIn.ReadCharAsBool( rDesign.m_bCreated );      // not used
    rIn.ReadInt16( rDesign.m_nButtonThema );
    rIn.ReadCharAsBool( rDesign.m_bUserAttr );
    aSerializer.readColor(rDesign.m_aBackColor);
    aSerializer.readColor(rDesign.m_aTextColor);
    aSerializer.readColor(rDesign.m_aLinkColor);
    aSerializer.readColor(rDesign.m_aVLinkColor);
    aSerializer.readColor(rDesign.m_aALinkColor);
    rIn.ReadCharAsBool( rDesign.m_bUseAttribs );
    rIn.ReadCharAsBool( rDesign.m_bUseColor );

    rIn.ReadUInt16( nTemp16 );
    rDesign.m_eScript = static_cast<PublishingScript>(nTemp16);
    rDesign.m_aURL = read_uInt16_lenPrefixed_uInt8s_ToOUString(rIn,
        RTL_TEXTENCODING_UTF8);
    rDesign.m_aCGI = read_uInt16_lenPrefixed_uInt8s_ToOUString(rIn,
        RTL_TEXTENCODING_UTF8);

    rIn.ReadCharAsBool( rDesign.m_bAutoSlide );
    rIn.ReadUInt32( rDesign.m_nSlideDuration );
    rIn.ReadCharAsBool( rDesign.m_bEndless );
    rIn.ReadCharAsBool( rDesign.m_bSlideSound );
    rIn.ReadCharAsBool( rDesign.m_bHiddenSlides );

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

    rOut.WriteUInt16( rDesign.m_eMode );
    rOut.WriteBool( rDesign.m_bContentPage );
    rOut.WriteBool( rDesign.m_bNotes );
    rOut.WriteUInt16( rDesign.m_nResolution );
    write_uInt16_lenPrefixed_uInt8s_FromOUString(rOut, rDesign.m_aCompression,
        RTL_TEXTENCODING_UTF8);
    rOut.WriteUInt16( rDesign.m_eFormat );
    write_uInt16_lenPrefixed_uInt8s_FromOUString(rOut, rDesign.m_aAuthor,
        RTL_TEXTENCODING_UTF8);
    write_uInt16_lenPrefixed_uInt8s_FromOUString(rOut, rDesign.m_aEMail,
        RTL_TEXTENCODING_UTF8);
    write_uInt16_lenPrefixed_uInt8s_FromOUString(rOut, rDesign.m_aWWW,
        RTL_TEXTENCODING_UTF8);
    write_uInt16_lenPrefixed_uInt8s_FromOUString(rOut, rDesign.m_aMisc,
        RTL_TEXTENCODING_UTF8);
    rOut.WriteBool( rDesign.m_bDownload );
    rOut.WriteBool( rDesign.m_bCreated );     // not used
    rOut.WriteInt16( rDesign.m_nButtonThema );
    rOut.WriteBool( rDesign.m_bUserAttr );
    aSerializer.writeColor(rDesign.m_aBackColor);
    aSerializer.writeColor(rDesign.m_aTextColor);
    aSerializer.writeColor(rDesign.m_aLinkColor);
    aSerializer.writeColor(rDesign.m_aVLinkColor);
    aSerializer.writeColor(rDesign.m_aALinkColor);
    rOut.WriteBool( rDesign.m_bUseAttribs );
    rOut.WriteBool( rDesign.m_bUseColor );

    rOut.WriteUInt16( rDesign.m_eScript );
    write_uInt16_lenPrefixed_uInt8s_FromOUString(rOut, rDesign.m_aURL,
        RTL_TEXTENCODING_UTF8);
    write_uInt16_lenPrefixed_uInt8s_FromOUString(rOut, rDesign.m_aCGI,
        RTL_TEXTENCODING_UTF8);

    rOut.WriteBool( rDesign.m_bAutoSlide );
    rOut.WriteUInt32( rDesign.m_nSlideDuration );
    rOut.WriteBool( rDesign.m_bEndless );
    rOut.WriteBool( rDesign.m_bSlideSound );
    rOut.WriteBool( rDesign.m_bHiddenSlides );

    return rOut;
}

namespace {

// Dialog for the entry of the name of the design
class SdDesignNameDlg : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::Entry> m_xEdit;
    std::unique_ptr<weld::Button> m_xBtnOK;

public:
    SdDesignNameDlg(weld::Window* pWindow, const OUString& aName );
    OUString GetDesignName() const;
    DECL_LINK(ModifyHdl, weld::Entry&, void);
};

}

// SdPublishingDlg Methods

SdPublishingDlg::SdPublishingDlg(weld::Window* pWindow, DocumentType eDocType)
    : GenericDialogController(pWindow, "modules/simpress/ui/publishingdialog.ui", "PublishingDialog")
    , m_xPage1_Designs(m_xBuilder->weld_tree_view("designsTreeview"))
    , m_xPage2_Standard_FB(m_xBuilder->weld_image("standardFBImage"))
    , m_xPage2_Frames_FB(m_xBuilder->weld_image("framesFBImage"))
    , m_xPage2_Kiosk_FB(m_xBuilder->weld_image("kioskFBImage"))
    , m_xPage2_WebCast_FB(m_xBuilder->weld_image("webCastFBImage"))
    , m_xPage4_Misc(m_xBuilder->weld_text_view("miscTextview"))
    , m_xButtonSet(new ButtonSet())
    , m_xLastPageButton(m_xBuilder->weld_button("lastPageButton"))
    , m_xNextPageButton(m_xBuilder->weld_button("nextPageButton"))
    , m_xFinishButton(m_xBuilder->weld_button("finishButton"))
    , aAssistentFunc(NOOFPAGES)
    , m_bButtonsDirty(true)
    , m_bDesignListDirty(false)
    , m_pDesign(nullptr)
{
    m_bImpress = eDocType == DocumentType::Impress;

    Size aSize(m_xPage2_Standard_FB->get_approximate_digit_width() * 12,
               m_xPage2_Standard_FB->get_text_height() * 6);
    m_xPage2_Standard_FB->set_size_request(aSize.Width(), aSize.Height());
    m_xPage2_Frames_FB->set_size_request(aSize.Width(), aSize.Height());
    m_xPage2_Kiosk_FB->set_size_request(aSize.Width(), aSize.Height());
    m_xPage2_WebCast_FB->set_size_request(aSize.Width(), aSize.Height());

    m_xPage4_Misc->set_size_request(m_xPage4_Misc->get_approximate_digit_width() * 40,
                                    m_xPage4_Misc->get_height_rows(8));

    m_xPage1_Designs->set_size_request(m_xPage4_Misc->get_approximate_digit_width() * 40,
                                       m_xPage4_Misc->get_height_rows(8));

    //Lock down the preferred size based on the
    //initial max-size configuration
    aSize = m_xDialog->get_preferred_size();
    m_xDialog->set_size_request(aSize.Width(), aSize.Height());

    CreatePages();
    Load();

    // sets the output page
    aAssistentFunc.GotoPage(1);
    m_xLastPageButton->set_sensitive(false);

    // button assignment
    m_xFinishButton->connect_clicked(LINK(this,SdPublishingDlg,FinishHdl));
    m_xLastPageButton->connect_clicked(LINK(this,SdPublishingDlg,LastPageHdl));
    m_xNextPageButton->connect_clicked(LINK(this,SdPublishingDlg,NextPageHdl));

    m_xPage1_NewDesign->connect_clicked(LINK(this,SdPublishingDlg,DesignHdl));
    m_xPage1_OldDesign->connect_clicked(LINK(this,SdPublishingDlg,DesignHdl));
    m_xPage1_Designs->connect_changed(LINK(this,SdPublishingDlg,DesignSelectHdl));
    m_xPage1_DelDesign->connect_clicked(LINK(this,SdPublishingDlg,DesignDeleteHdl));

    m_xPage2_Standard->connect_clicked(LINK(this,SdPublishingDlg,BaseHdl));
    m_xPage2_Frames->connect_clicked(LINK(this,SdPublishingDlg,BaseHdl));
    m_xPage2_SingleDocument->connect_clicked(LINK(this,SdPublishingDlg,BaseHdl));
    m_xPage2_Kiosk->connect_clicked(LINK(this,SdPublishingDlg,BaseHdl));
    m_xPage2_WebCast->connect_clicked(LINK(this,SdPublishingDlg,BaseHdl));

    m_xPage2_Content->connect_clicked(LINK(this,SdPublishingDlg,ContentHdl));

    m_xPage2_ASP->connect_clicked(LINK(this,SdPublishingDlg,WebServerHdl));
    m_xPage2_PERL->connect_clicked(LINK(this,SdPublishingDlg,WebServerHdl));
    m_xPage2_Index->set_text("index" STR_HTMLEXP_DEFAULT_EXTENSION);
    m_xPage2_CGI->set_text("/cgi-bin/");

    m_xPage3_Png->connect_clicked(LINK(this,SdPublishingDlg, GfxFormatHdl));
    m_xPage3_Gif->connect_clicked(LINK(this,SdPublishingDlg, GfxFormatHdl));
    m_xPage3_Jpg->connect_clicked(LINK(this,SdPublishingDlg, GfxFormatHdl));
    m_xPage3_Quality->set_sensitive(false);

    m_xPage3_Resolution_1->connect_clicked(LINK(this,SdPublishingDlg, ResolutionHdl ));
    m_xPage3_Resolution_2->connect_clicked(LINK(this,SdPublishingDlg, ResolutionHdl ));
    m_xPage3_Resolution_3->connect_clicked(LINK(this,SdPublishingDlg, ResolutionHdl ));
    m_xPage3_Resolution_4->connect_clicked(LINK(this, SdPublishingDlg, ResolutionHdl));

    m_xPage2_ChgDefault->connect_clicked(LINK(this,SdPublishingDlg, SlideChgHdl));
    m_xPage2_ChgAuto->connect_clicked(LINK(this,SdPublishingDlg, SlideChgHdl));

    m_xPage5_Buttons->SetSelectHdl(LINK(this,SdPublishingDlg, ButtonsHdl ));
    m_xPage5_Buttons->SetStyle( m_xPage5_Buttons->GetStyle() | WB_VSCROLL );

    m_xPage6_Back->connect_clicked(LINK(this,SdPublishingDlg, ColorHdl ));
    m_xPage6_Text->connect_clicked(LINK(this,SdPublishingDlg, ColorHdl ));
    m_xPage6_Link->connect_clicked(LINK(this,SdPublishingDlg, ColorHdl ));
    m_xPage6_VLink->connect_clicked(LINK(this,SdPublishingDlg, ColorHdl ));
    m_xPage6_ALink->connect_clicked(LINK(this,SdPublishingDlg, ColorHdl ));

    m_xPage6_DocColors->set_active(true);

    m_xPage3_Quality->append_text( "25%" );
    m_xPage3_Quality->append_text( "50%" );
    m_xPage3_Quality->append_text( "75%" );
    m_xPage3_Quality->append_text( "100%" );

    m_xPage5_Buttons->SetColCount();
    m_xPage5_Buttons->SetLineCount( 4 );
    m_xPage5_Buttons->SetExtraSpacing( 1 );

    for( const auto& rDesign : m_aDesignList )
        m_xPage1_Designs->append_text(rDesign.m_aDesignName);

    SetDefaults();

    m_xDialog->set_help_id(aPageHelpIds[0]);

    m_xNextPageButton->grab_focus();
}

SdPublishingDlg::~SdPublishingDlg()
{
}

// Generate dialog controls and embed them in the pages
void SdPublishingDlg::CreatePages()
{
    // Page 1
    m_xPage1 = m_xBuilder->weld_container("page1");
    m_xPage1_Title = m_xBuilder->weld_label("assignLabel");
    m_xPage1_NewDesign = m_xBuilder->weld_radio_button("newDesignRadiobutton");
    m_xPage1_OldDesign = m_xBuilder->weld_radio_button("oldDesignRadiobutton");
    m_xPage1_DelDesign = m_xBuilder->weld_button("delDesingButton");
    m_xPage1_Desc = m_xBuilder->weld_label("descLabel");
    aAssistentFunc.InsertControl(1, m_xPage1.get());
    aAssistentFunc.InsertControl(1, m_xPage1_Title.get());
    aAssistentFunc.InsertControl(1, m_xPage1_NewDesign.get());
    aAssistentFunc.InsertControl(1, m_xPage1_OldDesign.get());
    aAssistentFunc.InsertControl(1, m_xPage1_Designs.get());
    aAssistentFunc.InsertControl(1, m_xPage1_DelDesign.get());
    aAssistentFunc.InsertControl(1, m_xPage1_Desc.get());

    // Page 2
    m_xPage2 = m_xBuilder->weld_container("page2");
    m_xPage2Frame2 = m_xBuilder->weld_container("page2.2");
    m_xPage2Frame3 = m_xBuilder->weld_container("page2.3");
    m_xPage2Frame4 = m_xBuilder->weld_container("page2.4");
    m_xPage2_Title = m_xBuilder->weld_label("publicationLabel");
    m_xPage2_Standard = m_xBuilder->weld_radio_button("standardRadiobutton");
    m_xPage2_Frames = m_xBuilder->weld_radio_button("framesRadiobutton");
    m_xPage2_SingleDocument = m_xBuilder->weld_radio_button("singleDocumentRadiobutton");
    m_xPage2_Kiosk = m_xBuilder->weld_radio_button("kioskRadiobutton");
    m_xPage2_WebCast = m_xBuilder->weld_radio_button("webCastRadiobutton");
    aAssistentFunc.InsertControl(2, m_xPage2.get());
    aAssistentFunc.InsertControl(2, m_xPage2Frame2.get());
    aAssistentFunc.InsertControl(2, m_xPage2Frame3.get());
    aAssistentFunc.InsertControl(2, m_xPage2Frame4.get());
    aAssistentFunc.InsertControl(2, m_xPage2_Title.get());
    aAssistentFunc.InsertControl(2, m_xPage2_Standard.get());
    aAssistentFunc.InsertControl(2, m_xPage2_Frames.get());
    aAssistentFunc.InsertControl(2, m_xPage2_SingleDocument.get());
    aAssistentFunc.InsertControl(2, m_xPage2_Kiosk.get());
    aAssistentFunc.InsertControl(2, m_xPage2_WebCast.get());
    aAssistentFunc.InsertControl(2, m_xPage2_Standard_FB.get());
    aAssistentFunc.InsertControl(2, m_xPage2_Frames_FB.get());
    aAssistentFunc.InsertControl(2, m_xPage2_Kiosk_FB.get());
    aAssistentFunc.InsertControl(2, m_xPage2_WebCast_FB.get());

    m_xPage2_Title_Html = m_xBuilder->weld_label( "htmlOptionsLabel");
    m_xPage2_Content = m_xBuilder->weld_check_button("contentCheckbutton");
    m_xPage2_Notes = m_xBuilder->weld_check_button("notesCheckbutton");
    aAssistentFunc.InsertControl(2, m_xPage2_Title_Html.get());
    aAssistentFunc.InsertControl(2, m_xPage2_Content.get());
    if (m_bImpress)
        aAssistentFunc.InsertControl(2, m_xPage2_Notes.get());

    m_xPage2_Title_WebCast = m_xBuilder->weld_label("webCastLabel");
    m_xPage2_ASP = m_xBuilder->weld_radio_button("ASPRadiobutton");
    m_xPage2_PERL = m_xBuilder->weld_radio_button("perlRadiobutton");
    m_xPage2_URL_txt = m_xBuilder->weld_label("URLTxtLabel");
    m_xPage2_URL = m_xBuilder->weld_entry("URLEntry");
    m_xPage2_CGI_txt = m_xBuilder->weld_label("CGITxtLabel");
    m_xPage2_CGI = m_xBuilder->weld_entry("CGIEntry");
    m_xPage2_Index_txt = m_xBuilder->weld_label("indexTxtLabel");
    m_xPage2_Index = m_xBuilder->weld_entry("indexEntry");
    m_xPage2_Title_Kiosk = m_xBuilder->weld_label("kioskLabel");
    m_xPage2_ChgDefault = m_xBuilder->weld_radio_button("chgDefaultRadiobutton");
    m_xPage2_ChgAuto = m_xBuilder->weld_radio_button("chgAutoRadiobutton");
    m_xPage2_Duration_txt = m_xBuilder->weld_label("durationTxtLabel");
    m_xPage2_Duration = m_xBuilder->weld_formatted_spin_button("durationSpinbutton");
    m_xFormatter.reset(new weld::TimeFormatter(*m_xPage2_Duration));
    m_xFormatter->SetExtFormat(ExtTimeFieldFormat::LongDuration);
    m_xPage2_Endless = m_xBuilder->weld_check_button("endlessCheckbutton");
    aAssistentFunc.InsertControl(2, m_xPage2_Title_WebCast.get());
    aAssistentFunc.InsertControl(2, m_xPage2_Index_txt.get());
    aAssistentFunc.InsertControl(2, m_xPage2_Index.get());
    aAssistentFunc.InsertControl(2, m_xPage2_ASP.get());
    aAssistentFunc.InsertControl(2, m_xPage2_PERL.get());
    aAssistentFunc.InsertControl(2, m_xPage2_URL_txt.get());
    aAssistentFunc.InsertControl(2, m_xPage2_URL.get());
    aAssistentFunc.InsertControl(2, m_xPage2_CGI_txt.get());
    aAssistentFunc.InsertControl(2, m_xPage2_CGI.get());
    aAssistentFunc.InsertControl(2, m_xPage2_Title_Kiosk.get());
    aAssistentFunc.InsertControl(2, m_xPage2_ChgDefault.get());
    aAssistentFunc.InsertControl(2, m_xPage2_ChgAuto.get());
    aAssistentFunc.InsertControl(2, m_xPage2_Duration_txt.get());
    aAssistentFunc.InsertControl(2, m_xPage2_Duration.get());
    aAssistentFunc.InsertControl(2, m_xPage2_Endless.get());

    // Page 3
    m_xPage3 = m_xBuilder->weld_container("page3");
    m_xPage3_Title1 = m_xBuilder->weld_label("saveImgAsLabel");
    m_xPage3_Png = m_xBuilder->weld_radio_button("pngRadiobutton");
    m_xPage3_Gif = m_xBuilder->weld_radio_button("gifRadiobutton");
    m_xPage3_Jpg = m_xBuilder->weld_radio_button("jpgRadiobutton");
    m_xPage3_Quality_txt = m_xBuilder->weld_label("qualityTxtLabel");
    m_xPage3_Quality= m_xBuilder->weld_combo_box("qualityCombobox");
    m_xPage3_Title2 = m_xBuilder->weld_label("monitorResolutionLabel");
    m_xPage3_Resolution_1 = m_xBuilder->weld_radio_button("resolution1Radiobutton");
    m_xPage3_Resolution_2 = m_xBuilder->weld_radio_button("resolution2Radiobutton");
    m_xPage3_Resolution_3 = m_xBuilder->weld_radio_button("resolution3Radiobutton");
    m_xPage3_Resolution_4 = m_xBuilder->weld_radio_button("resolution4Radiobutton");
    m_xPage3_Title3 = m_xBuilder->weld_label("effectsLabel");
    m_xPage3_SldSound = m_xBuilder->weld_check_button("sldSoundCheckbutton");
    m_xPage3_HiddenSlides = m_xBuilder->weld_check_button("hiddenSlidesCheckbutton");
    aAssistentFunc.InsertControl(3, m_xPage3.get());
    aAssistentFunc.InsertControl(3, m_xPage3_Title1.get());
    aAssistentFunc.InsertControl(3, m_xPage3_Png.get());
    aAssistentFunc.InsertControl(3, m_xPage3_Gif.get());
    aAssistentFunc.InsertControl(3, m_xPage3_Jpg.get());
    aAssistentFunc.InsertControl(3, m_xPage3_Quality_txt.get());
    aAssistentFunc.InsertControl(3, m_xPage3_Quality.get());
    aAssistentFunc.InsertControl(3, m_xPage3_Title2.get());
    aAssistentFunc.InsertControl(3, m_xPage3_Resolution_1.get());
    aAssistentFunc.InsertControl(3, m_xPage3_Resolution_2.get());
    aAssistentFunc.InsertControl(3, m_xPage3_Resolution_3.get());
    aAssistentFunc.InsertControl(3, m_xPage3_Resolution_4.get());
    aAssistentFunc.InsertControl(3, m_xPage3_Title3.get());
    aAssistentFunc.InsertControl(3, m_xPage3_SldSound.get());
    aAssistentFunc.InsertControl(3, m_xPage3_HiddenSlides.get());

    // Page 4
    m_xPage4 = m_xBuilder->weld_container("page4");
    m_xPage4_Title1 = m_xBuilder->weld_label("infTitlePageLabel");
    m_xPage4_Author_txt = m_xBuilder->weld_label("authorTxtLabel");
    m_xPage4_Author = m_xBuilder->weld_entry("authorEntry");
    m_xPage4_Email_txt = m_xBuilder->weld_label("emailTxtLabel");
    m_xPage4_Email = m_xBuilder->weld_entry("emailEntry");
    m_xPage4_WWW_txt = m_xBuilder->weld_label("wwwTxtLabel");
    m_xPage4_WWW = m_xBuilder->weld_entry("wwwEntry");
    m_xPage4_Title2 = m_xBuilder->weld_label("addInformLabel");
    m_xPage4_Download = m_xBuilder->weld_check_button("downloadCheckbutton");
    aAssistentFunc.InsertControl(4, m_xPage4.get());
    aAssistentFunc.InsertControl(4, m_xPage4_Title1.get());
    aAssistentFunc.InsertControl(4, m_xPage4_Author_txt.get());
    aAssistentFunc.InsertControl(4, m_xPage4_Author.get());
    aAssistentFunc.InsertControl(4, m_xPage4_Email_txt.get());
    aAssistentFunc.InsertControl(4, m_xPage4_Email.get());
    aAssistentFunc.InsertControl(4, m_xPage4_WWW_txt.get());
    aAssistentFunc.InsertControl(4, m_xPage4_WWW.get());
    aAssistentFunc.InsertControl(4, m_xPage4_Title2.get());
    aAssistentFunc.InsertControl(4, m_xPage4_Misc.get());
    if(m_bImpress)
        aAssistentFunc.InsertControl(4, m_xPage4_Download.get());

    // Page 5
    m_xPage5 = m_xBuilder->weld_container("page5");
    m_xPage5_Title = m_xBuilder->weld_label("buttonStyleLabel");
    m_xPage5_TextOnly = m_xBuilder->weld_check_button("textOnlyCheckbutton");
    m_xPage5_Buttons.reset(new ValueSet(m_xBuilder->weld_scrolled_window("buttonsDrawingareawin", true)));
    m_xPage5_ButtonsWnd.reset(new weld::CustomWeld(*m_xBuilder, "buttonsDrawingarea", *m_xPage5_Buttons));
    aAssistentFunc.InsertControl(5, m_xPage5.get());
    aAssistentFunc.InsertControl(5, m_xPage5_Title.get());
    aAssistentFunc.InsertControl(5, m_xPage5_TextOnly.get());
    aAssistentFunc.InsertControl(5, m_xPage5_Buttons->GetDrawingArea());

    // Page 6
    m_xPage6 = m_xBuilder->weld_container("page6");
    m_xPage6_Title = m_xBuilder->weld_label("selectColorLabel");
    m_xPage6_Default = m_xBuilder->weld_radio_button("defaultRadiobutton");
    m_xPage6_User = m_xBuilder->weld_radio_button("userRadiobutton");
    m_xPage6_Back = m_xBuilder->weld_button("backButton");
    m_xPage6_Text = m_xBuilder->weld_button("textButton");
    m_xPage6_Link = m_xBuilder->weld_button("linkButton");
    m_xPage6_VLink = m_xBuilder->weld_button("vLinkButton");
    m_xPage6_ALink = m_xBuilder->weld_button("aLinkButton");
    m_xPage6_DocColors = m_xBuilder->weld_radio_button("docColorsRadiobutton");
    m_xPage6_Preview.reset(new SdHtmlAttrPreview);
    m_xPage6_PreviewWnd.reset(new weld::CustomWeld(*m_xBuilder, "previewDrawingarea", *m_xPage6_Preview));
    aAssistentFunc.InsertControl(6, m_xPage6.get());
    aAssistentFunc.InsertControl(6, m_xPage6_Title.get());
    aAssistentFunc.InsertControl(6, m_xPage6_DocColors.get());
    aAssistentFunc.InsertControl(6, m_xPage6_Default.get());
    aAssistentFunc.InsertControl(6, m_xPage6_User.get());
    aAssistentFunc.InsertControl(6, m_xPage6_Text.get());
    aAssistentFunc.InsertControl(6, m_xPage6_Link.get());
    aAssistentFunc.InsertControl(6, m_xPage6_ALink.get());
    aAssistentFunc.InsertControl(6, m_xPage6_VLink.get());
    aAssistentFunc.InsertControl(6, m_xPage6_Back.get());
    aAssistentFunc.InsertControl(6, m_xPage6_Preview->GetDrawingArea());
}

// Initialize dialog with default-values
void SdPublishingDlg::SetDefaults()
{
    SdPublishingDesign aDefault;
    SetDesign(&aDefault);

    m_xPage1_NewDesign->set_active(true);
    m_xPage1_OldDesign->set_active(false);
    UpdatePage();
}

// Feed the SfxItemSet with the settings of the dialog
void SdPublishingDlg::GetParameterSequence( Sequence< PropertyValue >& rParams )
{
    std::vector< PropertyValue > aProps;

    PropertyValue aValue;

    // Page 2
    aValue.Name = "PublishMode";

    HtmlPublishMode ePublishMode;
    if (m_xPage2_Frames->get_active())
        ePublishMode = PUBLISH_FRAMES;
    else if (m_xPage2_SingleDocument->get_active())
        ePublishMode = PUBLISH_SINGLE_DOCUMENT;
    else if (m_xPage2_Kiosk->get_active())
        ePublishMode  = PUBLISH_KIOSK;
    else if (m_xPage2_WebCast->get_active())
        ePublishMode  = PUBLISH_WEBCAST;
    else
        ePublishMode  = PUBLISH_HTML;

    aValue.Value <<= static_cast<sal_Int32>(ePublishMode);
    aProps.push_back( aValue );

    aValue.Name = "IsExportContentsPage";
    aValue.Value <<= m_xPage2_Content->get_active();
    aProps.push_back( aValue );

    if(m_bImpress)
    {
        aValue.Name = "IsExportNotes";
        aValue.Value <<= m_xPage2_Notes->get_active();
        aProps.push_back( aValue );
    }

    if( m_xPage2_WebCast->get_active() )
    {
        aValue.Name = "WebCastScriptLanguage";
        if( m_xPage2_ASP->get_active() )
            aValue.Value <<= OUString( "asp" );
        else
            aValue.Value <<= OUString( "perl" );
        aProps.push_back( aValue );

        aValue.Name = "WebCastCGIURL";
        aValue.Value <<= m_xPage2_CGI->get_text();
        aProps.push_back( aValue );

        aValue.Name = "WebCastTargetURL";
        aValue.Value <<= m_xPage2_URL->get_text();
        aProps.push_back( aValue );
    }
    aValue.Name = "IndexURL";
    aValue.Value <<= m_xPage2_Index->get_text();
    aProps.push_back( aValue );

    if( m_xPage2_Kiosk->get_active() && m_xPage2_ChgAuto->get_active() )
    {
        aValue.Name = "KioskSlideDuration";
        aValue.Value <<= static_cast<sal_uInt32>(m_xFormatter->GetTime().GetMSFromTime()) / 1000;
        aProps.push_back( aValue );

        aValue.Name = "KioskEndless";
        aValue.Value <<= m_xPage2_Endless->get_active();
        aProps.push_back( aValue );
    }

    // Page 3

    aValue.Name = "Width";
    sal_Int32 nTmpWidth = PUB_LOWRES_WIDTH;
    if( m_xPage3_Resolution_2->get_active() )
        nTmpWidth = PUB_MEDRES_WIDTH;
    else if( m_xPage3_Resolution_3->get_active() )
        nTmpWidth = PUB_HIGHRES_WIDTH;
    else if (m_xPage3_Resolution_4->get_active())
        nTmpWidth = PUB_FHDRES_WIDTH;

    aValue.Value <<= nTmpWidth;
    aProps.push_back( aValue );

    aValue.Name = "Compression";
    aValue.Value <<= m_xPage3_Quality->get_active_text();
    aProps.push_back( aValue );

    aValue.Name = "Format";
    sal_Int32 nFormat;
    if( m_xPage3_Png->get_active() )
        nFormat = static_cast<sal_Int32>(FORMAT_PNG);
    else if( m_xPage3_Gif->get_active() )
        nFormat = static_cast<sal_Int32>(FORMAT_GIF);
    else
        nFormat = static_cast<sal_Int32>(FORMAT_JPG);
    aValue.Value <<= nFormat;
    aProps.push_back( aValue );

    aValue.Name = "SlideSound";
    aValue.Value <<= m_xPage3_SldSound->get_active();
    aProps.push_back( aValue );

    aValue.Name = "HiddenSlides";
    aValue.Value <<= m_xPage3_HiddenSlides->get_active();
    aProps.push_back( aValue );

    // Page 4
    aValue.Name = "Author";
    aValue.Value <<= m_xPage4_Author->get_text();
    aProps.push_back( aValue );

    aValue.Name = "EMail";
    aValue.Value <<= m_xPage4_Email->get_text();
    aProps.push_back( aValue );

    // try to guess protocol for user's homepage
    INetURLObject aHomeURL( m_xPage4_WWW->get_text(),
                            INetProtocol::Http,     // default proto is HTTP
                            INetURLObject::EncodeMechanism::All );

    aValue.Name = "HomepageURL";
    aValue.Value <<= aHomeURL.GetMainURL( INetURLObject::DecodeMechanism::NONE );
    aProps.push_back( aValue );

    aValue.Name = "UserText";
    aValue.Value <<= m_xPage4_Misc->get_text();
    aProps.push_back( aValue );

    if( m_bImpress )
    {
        aValue.Name = "EnableDownload";
        aValue.Value <<= m_xPage4_Download->get_active();
        aProps.push_back( aValue );
    }

    // Page 5
    if( !m_xPage5_TextOnly->get_active() )
    {
        aValue.Name = "UseButtonSet";
        aValue.Value <<= static_cast<sal_Int32>(m_xPage5_Buttons->GetSelectedItemId() - 1);
        aProps.push_back( aValue );
    }

    // Page 6
    if( m_xPage6_User->get_active() )
    {
        aValue.Name = "BackColor";
        aValue.Value <<= m_aBackColor;
        aProps.push_back( aValue );

        aValue.Name = "TextColor";
        aValue.Value <<= m_aTextColor;
        aProps.push_back( aValue );

        aValue.Name = "LinkColor";
        aValue.Value <<= m_aLinkColor;
        aProps.push_back( aValue );

        aValue.Name = "VLinkColor";
        aValue.Value <<= m_aVLinkColor;
        aProps.push_back( aValue );

        aValue.Name = "ALinkColor";
        aValue.Value <<= m_aALinkColor;
        aProps.push_back( aValue );
    }

    if( m_xPage6_DocColors->get_active() )
    {
        aValue.Name = "IsUseDocumentColors";
        aValue.Value <<= true;
        aProps.push_back( aValue );
    }

    rParams = comphelper::containerToSequence(aProps);
}

// Clickhandler for the radiobuttons of the design-selection
IMPL_LINK( SdPublishingDlg, DesignHdl, weld::Button&, rButton, void )
{
    if (&rButton == m_xPage1_NewDesign.get())
    {
        m_xPage1_NewDesign->set_active(true); // because of DesignDeleteHdl
        m_xPage1_OldDesign->set_active(false);
        m_xPage1_Designs->set_sensitive(false);
        m_xPage1_DelDesign->set_sensitive(false);
        m_pDesign = nullptr;

        SdPublishingDesign aDefault;
        SetDesign(&aDefault);
    }
    else
    {
        m_xPage1_NewDesign->set_active(false);
        m_xPage1_Designs->set_sensitive(true);
        m_xPage1_DelDesign->set_sensitive(true);

        if (m_xPage1_Designs->get_selected_index() == -1)
            m_xPage1_Designs->select(0);

        const sal_Int32 nPos = m_xPage1_Designs->get_selected_index();
        m_pDesign = &m_aDesignList[nPos];
        DBG_ASSERT(m_pDesign, "No Design? That's not allowed (CL)");

        if(m_pDesign)
            SetDesign(m_pDesign);
    }
}

// Clickhandler for the choice of one design
IMPL_LINK_NOARG(SdPublishingDlg, DesignSelectHdl, weld::TreeView&, void)
{
    const sal_Int32 nPos = m_xPage1_Designs->get_selected_index();
    m_pDesign = &m_aDesignList[nPos];
    DBG_ASSERT(m_pDesign, "No Design? That's not allowed (CL)");

    if(m_pDesign)
        SetDesign(m_pDesign);

    UpdatePage();
}

// Clickhandler for the delete of one design
IMPL_LINK_NOARG(SdPublishingDlg, DesignDeleteHdl, weld::Button&, void)
{
    const sal_Int32 nPos = m_xPage1_Designs->get_selected_index();

    std::vector<SdPublishingDesign>::iterator iter = m_aDesignList.begin()+nPos;

    DBG_ASSERT(iter != m_aDesignList.end(), "No Design? That's not allowed (CL)");

    m_xPage1_Designs->remove(nPos);

    if(m_pDesign == &(*iter))
        DesignHdl(*m_xPage1_NewDesign);

    m_aDesignList.erase(iter);

    m_bDesignListDirty = true;

    UpdatePage();
}

// Clickhandler for the other servertypes
IMPL_LINK( SdPublishingDlg, WebServerHdl, weld::Button&, rButton, void )
{
    bool bASP = &rButton == m_xPage2_ASP.get();

    m_xPage2_ASP->set_sensitive( bASP );
    m_xPage2_PERL->set_sensitive( !bASP );
    UpdatePage();
}

// Clickhandler for the Radiobuttons of the graphicformat choice
IMPL_LINK( SdPublishingDlg, GfxFormatHdl, weld::Button&, rButton, void )
{
    m_xPage3_Png->set_sensitive( &rButton == m_xPage3_Png.get() );
    m_xPage3_Gif->set_sensitive( &rButton == m_xPage3_Gif.get() );
    m_xPage3_Jpg->set_sensitive( &rButton == m_xPage3_Jpg.get() );
    m_xPage3_Quality->set_sensitive(&rButton == m_xPage3_Jpg.get());
}

// Clickhandler for the Radiobuttons Standard/Frames
IMPL_LINK_NOARG(SdPublishingDlg, BaseHdl, weld::Button&, void)
{
    UpdatePage();
}

// Clickhandler for the Checkbox of the Title page
IMPL_LINK_NOARG(SdPublishingDlg, ContentHdl, weld::Button&, void)
{
    if(m_xPage2_Content->get_active())
    {
        if(!aAssistentFunc.IsEnabled(4))
        {
            aAssistentFunc.EnablePage(4);
            UpdatePage();
        }
    }
    else
    {
        if(aAssistentFunc.IsEnabled(4))
        {
            aAssistentFunc.DisablePage(4);
            UpdatePage();
        }
    }
}

// Clickhandler for the Resolution Radiobuttons
IMPL_LINK( SdPublishingDlg, ResolutionHdl, weld::Button&, rButton, void )
{
    m_xPage3_Resolution_1->set_sensitive(&rButton == m_xPage3_Resolution_1.get());
    m_xPage3_Resolution_2->set_sensitive(&rButton == m_xPage3_Resolution_2.get());
    m_xPage3_Resolution_3->set_sensitive(&rButton == m_xPage3_Resolution_3.get());
    m_xPage3_Resolution_4->set_sensitive(&rButton == m_xPage3_Resolution_4.get());
}

// Clickhandler for the ValueSet with the bitmap-buttons
IMPL_LINK_NOARG(SdPublishingDlg, ButtonsHdl, ValueSet*, void)
{
    // if one bitmap-button is chosen, then disable TextOnly
    m_xPage5_TextOnly->set_active(false);
}

// Fill the SfxItemSet with the settings of the dialog
IMPL_LINK( SdPublishingDlg, ColorHdl, weld::Button&, rButton, void)
{
    SvColorDialog aDlg;

    if (&rButton == m_xPage6_Back.get())
    {
        aDlg.SetColor( m_aBackColor );
        if(aDlg.Execute(m_xDialog.get()) == RET_OK )
            m_aBackColor = aDlg.GetColor();
    }
    else if (&rButton == m_xPage6_Text.get())
    {
        aDlg.SetColor( m_aTextColor );
        if(aDlg.Execute(m_xDialog.get()) == RET_OK )
            m_aTextColor = aDlg.GetColor();
    }
    else if (&rButton == m_xPage6_Link.get())
    {
        aDlg.SetColor( m_aLinkColor );
        if(aDlg.Execute(m_xDialog.get()) == RET_OK )
            m_aLinkColor = aDlg.GetColor();
    }
    else if (&rButton == m_xPage6_VLink.get())
    {
        aDlg.SetColor( m_aVLinkColor );
        if(aDlg.Execute(m_xDialog.get()) == RET_OK )
            m_aVLinkColor = aDlg.GetColor();
    }
    else if (&rButton == m_xPage6_ALink.get())
    {
        aDlg.SetColor( m_aALinkColor );
        if(aDlg.Execute(m_xDialog.get()) == RET_OK )
            m_aALinkColor = aDlg.GetColor();
    }

    m_xPage6_User->set_active(true);
    m_xPage6_Preview->SetColors( m_aBackColor, m_aTextColor, m_aLinkColor,
                               m_aVLinkColor, m_aALinkColor );
    m_xPage6_Preview->Invalidate();
}

IMPL_LINK_NOARG(SdPublishingDlg, SlideChgHdl, weld::Button&, void)
{
    UpdatePage();
}

// Clickhandler for the Ok Button
IMPL_LINK_NOARG(SdPublishingDlg, FinishHdl, weld::Button&, void)
{
    //End
    SdPublishingDesign aDesign;
    GetDesign(&aDesign);

    bool bSave = false;

    if(m_xPage1_OldDesign->get_active() && m_pDesign)
    {
        // are there changes?
        if(!(aDesign == *m_pDesign))
            bSave = true;
    }
    else
    {
        SdPublishingDesign aDefaultDesign;
        if(!(aDefaultDesign == aDesign))
            bSave = true;
    }

    if(bSave)
    {
        OUString aName;
        if(m_pDesign)
            aName = m_pDesign->m_aDesignName;

        bool bRetry;
        do
        {
            bRetry = false;

            SdDesignNameDlg aDlg(m_xDialog.get(), aName);

            if (aDlg.run() == RET_OK)
            {
                aDesign.m_aDesignName = aDlg.GetDesignName();

                auto iter = std::find_if(m_aDesignList.begin(), m_aDesignList.end(),
                    [&aDesign](const SdPublishingDesign& rDesign) { return rDesign.m_aDesignName == aDesign.m_aDesignName; });

                if (iter != m_aDesignList.end())
                {
                    std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                                   VclMessageType::Error, VclButtonsType::YesNo,
                                                                   SdResId(STR_PUBDLG_SAMENAME)));
                    bRetry = xErrorBox->run() == RET_NO;

                    if(!bRetry)
                        m_aDesignList.erase(iter);
                }

                if(!bRetry)
                {
                    m_aDesignList.push_back(aDesign);
                    m_bDesignListDirty = true;
                }
            }
        }
        while(bRetry);
    }

    if(m_bDesignListDirty)
        Save();

    m_xDialog->response(RET_OK);
}

// Refresh the dialogs when changing from pages
void SdPublishingDlg::ChangePage()
{
    int nPage = aAssistentFunc.GetCurrentPage();
    m_xDialog->set_help_id(aPageHelpIds[nPage-1]);

    UpdatePage();

    if (m_xNextPageButton->get_sensitive())
        m_xNextPageButton->grab_focus();
    else
        m_xFinishButton->grab_focus();
}

void SdPublishingDlg::UpdatePage()
{
    m_xNextPageButton->set_sensitive(!aAssistentFunc.IsLastPage());
    m_xLastPageButton->set_sensitive(!aAssistentFunc.IsFirstPage());

    int nPage = aAssistentFunc.GetCurrentPage();

    switch( nPage )
    {
    case 1:
        if(m_xPage1_NewDesign->get_active())
        {
            m_xPage1_Designs->set_sensitive(false);
            m_xPage1_DelDesign->set_sensitive(false);
        }

        if(m_aDesignList.empty())
            m_xPage1_OldDesign->set_sensitive(false);
        break;
    case 2:
        m_xPage2_Frames_FB->set_visible(m_xPage2_Frames->get_active());
        m_xPage2_Standard_FB->set_visible(m_xPage2_Standard->get_active());
        m_xPage2_Kiosk_FB->set_visible(m_xPage2_Kiosk->get_active());
        m_xPage2_WebCast_FB->set_visible(m_xPage2_WebCast->get_active());

        if( m_xPage2_WebCast->get_active() )
        {
            m_xPage2Frame4->show();
            m_xPage2_Title_WebCast->show();
            m_xPage2_ASP->show();
            m_xPage2_PERL->show();
            m_xPage2_URL_txt->show();
            m_xPage2_URL->show();
            m_xPage2_CGI_txt->show();
            m_xPage2_CGI->show();
            m_xPage2_Index_txt->show();
            m_xPage2_Index->show();

            bool bPerl = m_xPage2_PERL->get_active();
            m_xPage2_Index->set_sensitive(bPerl);
            m_xPage2_Index_txt->set_sensitive(bPerl);
            m_xPage2_URL_txt->set_sensitive(bPerl);
            m_xPage2_URL->set_sensitive(bPerl);
            m_xPage2_CGI_txt->set_sensitive(bPerl);
            m_xPage2_CGI->set_sensitive(bPerl);
        }
        else
        {
            m_xPage2Frame4->hide();
            m_xPage2_Title_WebCast->hide();
            m_xPage2_ASP->hide();
            m_xPage2_PERL->hide();
            m_xPage2_URL_txt->hide();
            m_xPage2_URL->hide();
            m_xPage2_CGI_txt->hide();
            m_xPage2_CGI->hide();
            m_xPage2_Index->hide();
            m_xPage2_Index_txt->hide();
        }

        if( m_xPage2_Kiosk->get_active() )
        {
            m_xPage2Frame3->show();
            m_xPage2_Title_Kiosk->show();
            m_xPage2_ChgDefault->show();
            m_xPage2_ChgAuto->show();
            m_xPage2_Duration_txt->show();
            m_xPage2_Duration->show();
            m_xPage2_Endless->show();
            bool bAuto = m_xPage2_ChgAuto->get_active();
            m_xPage2_Duration->set_sensitive(bAuto);
            m_xPage2_Endless->set_sensitive(bAuto);
        }
        else
        {
            m_xPage2Frame3->hide();
            m_xPage2_Title_Kiosk->hide();
            m_xPage2_ChgDefault->hide();
            m_xPage2_ChgAuto->hide();
            m_xPage2_Duration->hide();
            m_xPage2_Duration_txt->hide();
            m_xPage2_Endless->hide();
        }

        if( m_xPage2_Standard->get_active() || m_xPage2_Frames->get_active() )
        {
            m_xPage2Frame2->show();
            m_xPage2_Title_Html->show();
            m_xPage2_Content->show();
            if(m_bImpress)
                m_xPage2_Notes->show();
        }
        else
        {
            m_xPage2Frame2->hide();
            m_xPage2_Title_Html->hide();
            m_xPage2_Content->hide();
            if(m_bImpress)
                m_xPage2_Notes->hide();
        }
        break;
    case 3:
        if( m_xPage2_Kiosk->get_active() || m_xPage2_WebCast->get_active() )
            m_xNextPageButton->set_sensitive(false);

        if( m_xPage2_WebCast->get_active() )
            m_xPage3_SldSound->set_sensitive(false);

        m_xPage3_Quality->set_sensitive(m_xPage3_Jpg->get_active());

        break;
    case 5:
        if( m_bButtonsDirty )
            LoadPreviewButtons();
        break;
    }
}

/** loads the html buttons from the button sets, creates a preview and fills the
    itemset for page 5
 */
void SdPublishingDlg::LoadPreviewButtons()
{
    if (!m_xButtonSet)
        return;

    const int nButtonCount = 8;
    static const char *pButtonNames[nButtonCount] =
    {
        "first.png",
        "left.png",
        "right.png",
        "last.png",
        "home.png",
        "text.png",
        "expand.png",
        "collapse.png",
    };

    std::vector< OUString > aButtonNames;
    for(const char * p : pButtonNames)
        aButtonNames.push_back( OUString::createFromAscii( p ) );

    int nSetCount = m_xButtonSet->getCount();

    int nHeight = 32;
    Image aImage;
    for( int nSet = 0; nSet < nSetCount; ++nSet )
    {
        if( m_xButtonSet->getPreview( nSet, aButtonNames, aImage ) )
        {
            m_xPage5_Buttons->InsertItem( static_cast<sal_uInt16>(nSet)+1, aImage );
            if( nHeight < aImage.GetSizePixel().Height() )
                nHeight = aImage.GetSizePixel().Height();
        }
    }

    m_xPage5_Buttons->SetItemHeight( nHeight );
    m_bButtonsDirty = false;
}

// Clickhandler for the Forward Button
IMPL_LINK_NOARG(SdPublishingDlg, NextPageHdl, weld::Button&, void)
{
    aAssistentFunc.NextPage();
    ChangePage();
}

// Sets the Controls in the dialog to the settings in the design
void SdPublishingDlg::SetDesign( SdPublishingDesign const * pDesign )
{
    if(!pDesign)
        return;

    m_xPage2_Standard->set_sensitive(pDesign->m_eMode == PUBLISH_HTML);
    m_xPage2_Frames->set_sensitive(pDesign->m_eMode == PUBLISH_FRAMES);
    m_xPage2_Kiosk->set_sensitive(pDesign->m_eMode == PUBLISH_KIOSK );
    m_xPage2_WebCast->set_sensitive(pDesign->m_eMode == PUBLISH_WEBCAST );

    m_xPage2_Content->set_sensitive(pDesign->m_bContentPage);
    if(pDesign->m_bContentPage)
        aAssistentFunc.EnablePage(4);
    else
        aAssistentFunc.DisablePage(4);

    if(m_bImpress)
        m_xPage2_Notes->set_sensitive(pDesign->m_bNotes);

    m_xPage2_ASP->set_sensitive(pDesign->m_eScript == SCRIPT_ASP);
    m_xPage2_PERL->set_sensitive(pDesign->m_eScript == SCRIPT_PERL);
    m_xPage2_CGI->set_text(pDesign->m_aCGI);
    m_xPage2_URL->set_text(pDesign->m_aURL);

    m_xPage2_ChgDefault->set_sensitive( !pDesign->m_bAutoSlide );
    m_xPage2_ChgAuto->set_sensitive( pDesign->m_bAutoSlide );

    tools::Time aTime( tools::Time::EMPTY );
    aTime.MakeTimeFromMS( pDesign->m_nSlideDuration * 1000 );
    m_xFormatter->SetTime(aTime);

    m_xPage2_Endless->set_sensitive( pDesign->m_bEndless );

    m_xPage3_Png->set_sensitive(pDesign->m_eFormat == FORMAT_PNG);
    m_xPage3_Gif->set_sensitive(pDesign->m_eFormat == FORMAT_GIF);
    m_xPage3_Jpg->set_sensitive(pDesign->m_eFormat == FORMAT_JPG);
    m_xPage3_Quality->set_sensitive(pDesign->m_eFormat == FORMAT_JPG);

    m_xPage3_Quality->set_entry_text(pDesign->m_aCompression);
    m_xPage3_Resolution_1->set_sensitive(pDesign->m_nResolution == PUB_LOWRES_WIDTH);
    m_xPage3_Resolution_2->set_sensitive(pDesign->m_nResolution == PUB_MEDRES_WIDTH);
    m_xPage3_Resolution_3->set_sensitive(pDesign->m_nResolution == PUB_HIGHRES_WIDTH);
    m_xPage3_Resolution_4->set_sensitive(pDesign->m_nResolution == PUB_FHDRES_WIDTH);

    m_xPage3_SldSound->set_sensitive( pDesign->m_bSlideSound );
    m_xPage3_HiddenSlides->set_sensitive( pDesign->m_bHiddenSlides );

    m_xPage4_Author->set_text(pDesign->m_aAuthor);
    m_xPage4_Email->set_text(pDesign->m_aEMail);
    m_xPage4_WWW->set_text(pDesign->m_aWWW);
    m_xPage4_Misc->set_text(pDesign->m_aMisc);
    if(m_bImpress)
        m_xPage4_Download->set_sensitive(pDesign->m_bDownload);

    m_xPage5_TextOnly->set_sensitive(pDesign->m_nButtonThema == -1);
    if(pDesign->m_nButtonThema != -1)
    {
        if(m_bButtonsDirty)
            LoadPreviewButtons();
        m_xPage5_Buttons->SelectItem(pDesign->m_nButtonThema + 1);
    }
    else
        m_xPage5_Buttons->SetNoSelection();

    m_xPage6_User->set_sensitive(pDesign->m_bUserAttr);
    m_aBackColor = pDesign->m_aBackColor;
    m_aTextColor = pDesign->m_aTextColor;
    m_aLinkColor = pDesign->m_aLinkColor;
    m_aVLinkColor = pDesign->m_aVLinkColor;
    m_aALinkColor  = pDesign->m_aALinkColor;

    m_xPage6_DocColors->set_sensitive(pDesign->m_bUseColor);

    m_xPage6_Preview->SetColors( m_aBackColor, m_aTextColor, m_aLinkColor,
                               m_aVLinkColor, m_aALinkColor );
    m_xPage6_Preview->Invalidate();

    UpdatePage();
}

// Transfer the status of the Design Dialog Controls
void SdPublishingDlg::GetDesign( SdPublishingDesign* pDesign )
{
    if(!pDesign)
        return;

    pDesign->m_eMode =  m_xPage2_Standard->get_active()?PUBLISH_HTML:
                        m_xPage2_Frames->get_active()?PUBLISH_FRAMES:
                        m_xPage2_Kiosk->get_active()?PUBLISH_KIOSK:
                        PUBLISH_WEBCAST;

    pDesign->m_bContentPage = m_xPage2_Content->get_active();
    if(m_bImpress)
        pDesign->m_bNotes = m_xPage2_Notes->get_active();

    if( m_xPage3_Gif->get_active() )
        pDesign->m_eFormat = FORMAT_GIF;
    else if( m_xPage3_Jpg->get_active() )
        pDesign->m_eFormat = FORMAT_JPG;
    else
        pDesign->m_eFormat = FORMAT_PNG;

    pDesign->m_aCompression = m_xPage3_Quality->get_active_text();

    if (m_xPage3_Resolution_1->get_active())
        pDesign->m_nResolution = PUB_LOWRES_WIDTH;
    else if (m_xPage3_Resolution_2->get_active())
        pDesign->m_nResolution = PUB_MEDRES_WIDTH;
    else if (m_xPage3_Resolution_3->get_active())
        pDesign->m_nResolution = PUB_HIGHRES_WIDTH;
    else
        pDesign->m_nResolution = PUB_FHDRES_WIDTH;

    pDesign->m_bSlideSound = m_xPage3_SldSound->get_active();
    pDesign->m_bHiddenSlides = m_xPage3_HiddenSlides->get_active();

    pDesign->m_aAuthor = m_xPage4_Author->get_text();
    pDesign->m_aEMail = m_xPage4_Email->get_text();
    pDesign->m_aWWW = m_xPage4_WWW->get_text();
    pDesign->m_aMisc = m_xPage4_Misc->get_text();
    pDesign->m_bDownload = m_bImpress && m_xPage4_Download->get_active();

    if(m_xPage5_TextOnly->get_active())
        pDesign->m_nButtonThema = -1;
    else
        pDesign->m_nButtonThema = m_xPage5_Buttons->GetSelectedItemId() - 1;

    pDesign->m_bUserAttr = m_xPage6_User->get_active();
    pDesign->m_aBackColor = m_aBackColor;
    pDesign->m_aTextColor = m_aTextColor;
    pDesign->m_aLinkColor = m_aLinkColor;
    pDesign->m_aVLinkColor = m_aVLinkColor;
    pDesign->m_aALinkColor  = m_aALinkColor;
    pDesign->m_bUseColor   = m_xPage6_DocColors->get_active();

    pDesign->m_eScript = m_xPage2_ASP->get_active()?SCRIPT_ASP:SCRIPT_PERL;
    pDesign->m_aCGI = m_xPage2_CGI->get_text();
    pDesign->m_aURL = m_xPage2_URL->get_text();

    pDesign->m_bAutoSlide = m_xPage2_ChgAuto->get_active();
    pDesign->m_nSlideDuration = static_cast<sal_uInt32>(m_xFormatter->GetTime().GetMSFromTime()) / 1000;
    pDesign->m_bEndless = m_xPage2_Endless->get_active();
}

// Clickhandler for the back Button
IMPL_LINK_NOARG(SdPublishingDlg, LastPageHdl, weld::Button&, void)
{
    aAssistentFunc.PreviousPage();
    ChangePage();
}

// Load Designs
void SdPublishingDlg::Load()
{
    m_bDesignListDirty = false;

    INetURLObject aURL( SvtPathOptions().GetUserConfigPath() );
    aURL.Append( u"designs.sod" );

    // check if file exists, SfxMedium shows an errorbox else
    {
        std::unique_ptr<SvStream> pIStm = ::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), StreamMode::READ );

        bool bOk = pIStm && ( pIStm->GetError() == ERRCODE_NONE);

        if( !bOk )
            return;
    }

    SfxMedium aMedium( aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), StreamMode::READ | StreamMode::NOCREATE );

    SvStream* pStream = aMedium.GetInStream();

    if( !pStream )
        return;

    sal_uInt16 aCheck;
    pStream->ReadUInt16( aCheck );

    if(aCheck != nMagic)
        return;

    SdIOCompat aIO(*pStream, StreamMode::READ);

    sal_uInt16 nDesigns(0);
    pStream->ReadUInt16(nDesigns);

    // there has to at least be a sal_uInt16 header in each design
    const size_t nMaxRecords = pStream->remainingSize() / sizeof(sal_uInt16);
    if (nDesigns > nMaxRecords)
    {
        SAL_WARN("sd", "Parsing error: " << nMaxRecords <<
                 " max possible entries, but " << nDesigns << " claimed, truncating");
        nDesigns = nMaxRecords;
    }

    for( sal_uInt16 nIndex = 0;
         pStream->GetError() == ERRCODE_NONE && nIndex < nDesigns;
         nIndex++ )
    {
        SdPublishingDesign aDesign;
        *pStream >> aDesign;

        m_aDesignList.push_back(aDesign);
    }
}

// Save Designs
bool SdPublishingDlg::Save()
{
    INetURLObject aURL( SvtPathOptions().GetUserConfigPath() );
    aURL.Append( u"designs.sod" );
    SfxMedium aMedium( aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), StreamMode::WRITE | StreamMode::TRUNC );

    SvStream* pStream = aMedium.GetOutStream();

    if( !pStream )
        return false;

    pStream->WriteUInt16( nMagic );

    // Destroys the SdIOCompat before the Stream is being distributed
    {
        SdIOCompat aIO(*pStream, StreamMode::WRITE, 0);

        sal_uInt16 nDesigns = static_cast<sal_uInt16>(m_aDesignList.size());
        pStream->WriteUInt16( nDesigns );

        for( sal_uInt16 nIndex = 0;
             pStream->GetError() == ERRCODE_NONE && nIndex < nDesigns;
             nIndex++ )
            WriteSdPublishingDesign( *pStream, m_aDesignList[nIndex] );
    }

    aMedium.Close();
    aMedium.Commit();

    return( aMedium.GetError() == ERRCODE_NONE );
}

// SdDesignNameDlg Methods
SdDesignNameDlg::SdDesignNameDlg(weld::Window* pWindow, const OUString& rName)
    : GenericDialogController(pWindow, "modules/sdraw/ui/namedesign.ui", "NameDesignDialog")
    , m_xEdit(m_xBuilder->weld_entry("entry"))
    , m_xBtnOK(m_xBuilder->weld_button("ok"))
{
    m_xEdit->connect_changed(LINK(this, SdDesignNameDlg, ModifyHdl ));
    m_xEdit->set_text(rName);
    m_xBtnOK->set_sensitive(!rName.isEmpty());
}

OUString SdDesignNameDlg::GetDesignName() const
{
    return m_xEdit->get_text();
}

IMPL_LINK_NOARG(SdDesignNameDlg, ModifyHdl, weld::Entry&, void)
{
    m_xBtnOK->set_sensitive(!m_xEdit->get_text().isEmpty());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
