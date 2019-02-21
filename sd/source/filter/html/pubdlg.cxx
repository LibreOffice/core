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
#include <vcl/lstbox.hxx>
#include <vcl/FilterConfigItem.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/vclmedit.hxx>
#include <vcl/weld.hxx>
#include <svtools/valueset.hxx>
#include <svtools/colrdlg.hxx>
#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
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
    FilterConfigItem aFilterConfigItem("Office.Common/Filter/Graphic/Export/JPG");
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
    ReadColor( rIn, rDesign.m_aBackColor );
    ReadColor( rIn, rDesign.m_aTextColor );
    ReadColor( rIn, rDesign.m_aLinkColor );
    ReadColor( rIn, rDesign.m_aVLinkColor );
    ReadColor( rIn, rDesign.m_aALinkColor );
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
    WriteColor( rOut, rDesign.m_aBackColor );
    WriteColor( rOut, rDesign.m_aTextColor );
    WriteColor( rOut, rDesign.m_aLinkColor );
    WriteColor( rOut, rDesign.m_aVLinkColor );
    WriteColor( rOut, rDesign.m_aALinkColor );
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

// Dialog for the entry of the name of the design
class SdDesignNameDlg : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::Entry> m_xEdit;
    std::unique_ptr<weld::Button> m_xBtnOK;

public:
    SdDesignNameDlg(weld::Window* pWindow, const OUString& aName );
    OUString GetDesignName();
    DECL_LINK(ModifyHdl, weld::Entry&, void);
};

// SdPublishingDlg Methods

SdPublishingDlg::SdPublishingDlg(vcl::Window* pWindow, DocumentType eDocType)
:   ModalDialog(pWindow, "PublishingDialog", "modules/simpress/ui/publishingdialog.ui")
,   mpButtonSet( new ButtonSet() )
,   aAssistentFunc(NOOFPAGES)
,   m_bButtonsDirty(true)
,   m_bDesignListDirty(false)
,   m_pDesign(nullptr)
{
    get(pLastPageButton, "lastPageButton");
    get(pNextPageButton, "nextPageButton");
    get(pFinishButton, "finishButton");

    m_bImpress = eDocType == DocumentType::Impress;

    Size aSize(LogicToPixel(Size(60, 50), MapMode(MapUnit::MapAppFont)));
    get(pPage2_Standard_FB, "standardFBImage");
    pPage2_Standard_FB->set_width_request(aSize.Width());
    pPage2_Standard_FB->set_height_request(aSize.Height());
    get(pPage2_Frames_FB, "framesFBImage");
    pPage2_Frames_FB->set_width_request(aSize.Width());
    pPage2_Frames_FB->set_height_request(aSize.Height());
    get(pPage2_Kiosk_FB, "kioskFBImage");
    pPage2_Kiosk_FB->set_width_request(aSize.Width());
    pPage2_Kiosk_FB->set_height_request(aSize.Height());
    get(pPage2_WebCast_FB, "webCastFBImage");
    pPage2_WebCast_FB->set_width_request(aSize.Width());
    pPage2_WebCast_FB->set_height_request(aSize.Height());

    get(pPage4_Misc, "miscTextview");
    pPage4_Misc->set_height_request(pPage4_Misc->GetTextHeight() * 8);
    pPage4_Misc->set_width_request(pPage4_Misc->approximate_char_width() * 52);

    get(pPage1_Designs, "designsTreeview");
    pPage1_Designs->set_height_request(pPage4_Misc->GetTextHeight() * 8);
    pPage1_Designs->set_width_request(pPage4_Misc->approximate_char_width() * 52);

    //Lock down the preferred size based on the
    //initial max-size configuration
    aSize = get_preferred_size();
    set_width_request(aSize.Width());
    set_height_request(aSize.Height());

    CreatePages();
    Load();

    // sets the output page
    aAssistentFunc.GotoPage(1);
    pLastPageButton->Disable();

    // button assignment
    pFinishButton->SetClickHdl(LINK(this,SdPublishingDlg,FinishHdl));
    pLastPageButton->SetClickHdl(LINK(this,SdPublishingDlg,LastPageHdl));
    pNextPageButton->SetClickHdl(LINK(this,SdPublishingDlg,NextPageHdl));

    pPage1_NewDesign->SetClickHdl(LINK(this,SdPublishingDlg,DesignHdl));
    pPage1_OldDesign->SetClickHdl(LINK(this,SdPublishingDlg,DesignHdl));
    pPage1_Designs->SetSelectHdl(LINK(this,SdPublishingDlg,DesignSelectHdl));
    pPage1_DelDesign->SetClickHdl(LINK(this,SdPublishingDlg,DesignDeleteHdl));

    pPage2_Standard->SetClickHdl(LINK(this,SdPublishingDlg,BaseHdl));
    pPage2_Standard_FB->SetBorderStyle(WindowBorderStyle::MONO);
    pPage2_Frames->SetClickHdl(LINK(this,SdPublishingDlg,BaseHdl));
    pPage2_Frames_FB->SetBorderStyle(WindowBorderStyle::MONO);
    pPage2_SingleDocument->SetClickHdl(LINK(this,SdPublishingDlg,BaseHdl));
    pPage2_Kiosk->SetClickHdl(LINK(this,SdPublishingDlg,BaseHdl));
    pPage2_Kiosk_FB->SetBorderStyle(WindowBorderStyle::MONO);
    pPage2_WebCast->SetClickHdl(LINK(this,SdPublishingDlg,BaseHdl));
    pPage2_WebCast_FB->SetBorderStyle(WindowBorderStyle::MONO);

    pPage2_Content->SetClickHdl(LINK(this,SdPublishingDlg,ContentHdl));

    pPage2_ASP->SetClickHdl(LINK(this,SdPublishingDlg,WebServerHdl));
    pPage2_PERL->SetClickHdl(LINK(this,SdPublishingDlg,WebServerHdl));
    pPage2_Index->SetText("index" STR_HTMLEXP_DEFAULT_EXTENSION);
    pPage2_CGI->SetText( "/cgi-bin/" );

    pPage3_Png->SetClickHdl(LINK(this,SdPublishingDlg, GfxFormatHdl));
    pPage3_Gif->SetClickHdl(LINK(this,SdPublishingDlg, GfxFormatHdl));
    pPage3_Jpg->SetClickHdl(LINK(this,SdPublishingDlg, GfxFormatHdl));
    pPage3_Quality->Enable(false);

    pPage3_Resolution_1->SetClickHdl(LINK(this,SdPublishingDlg, ResolutionHdl ));
    pPage3_Resolution_2->SetClickHdl(LINK(this,SdPublishingDlg, ResolutionHdl ));
    pPage3_Resolution_3->SetClickHdl(LINK(this,SdPublishingDlg, ResolutionHdl ));

    pPage2_ChgDefault->SetClickHdl(LINK(this,SdPublishingDlg, SlideChgHdl));
    pPage2_ChgAuto->SetClickHdl(LINK(this,SdPublishingDlg, SlideChgHdl));
    pPage2_Duration->SetFormat( TimeFieldFormat::F_SEC );

    pPage5_Buttons->SetSelectHdl(LINK(this,SdPublishingDlg, ButtonsHdl ));
    pPage5_Buttons->SetStyle( pPage5_Buttons->GetStyle() | WB_VSCROLL );

    pPage6_Back->SetClickHdl(LINK(this,SdPublishingDlg, ColorHdl ));
    pPage6_Text->SetClickHdl(LINK(this,SdPublishingDlg, ColorHdl ));
    pPage6_Link->SetClickHdl(LINK(this,SdPublishingDlg, ColorHdl ));
    pPage6_VLink->SetClickHdl(LINK(this,SdPublishingDlg, ColorHdl ));
    pPage6_ALink->SetClickHdl(LINK(this,SdPublishingDlg, ColorHdl ));

    pPage6_DocColors->Check();

    pPage3_Quality->InsertEntry( "25%" );
    pPage3_Quality->InsertEntry( "50%" );
    pPage3_Quality->InsertEntry( "75%" );
    pPage3_Quality->InsertEntry( "100%" );

    pPage5_Buttons->SetColCount();
    pPage5_Buttons->SetLineCount( 4 );
    pPage5_Buttons->SetExtraSpacing( 1 );

    for( const auto& rDesign : m_aDesignList )
        pPage1_Designs->InsertEntry(rDesign.m_aDesignName);

    pPage6_Preview->SetBorderStyle(WindowBorderStyle::MONO);

    SetDefaults();

    SetHelpId(aPageHelpIds[0]);

    pNextPageButton->GrabFocus();
}

SdPublishingDlg::~SdPublishingDlg()
{
    disposeOnce();
}

void SdPublishingDlg::dispose()
{
    pPage1.clear();
    pPage1_Titel.clear();
    pPage1_NewDesign.clear();
    pPage1_OldDesign.clear();
    pPage1_Designs.clear();
    pPage1_DelDesign.clear();
    pPage1_Desc.clear();
    pPage2.clear();
    pPage2Frame2.clear();
    pPage2Frame3.clear();
    pPage2Frame4.clear();
    pPage2_Titel.clear();
    pPage2_Standard.clear();
    pPage2_Frames.clear();
    pPage2_SingleDocument.clear();
    pPage2_Kiosk.clear();
    pPage2_WebCast.clear();
    pPage2_Standard_FB.clear();
    pPage2_Frames_FB.clear();
    pPage2_Kiosk_FB.clear();
    pPage2_WebCast_FB.clear();
    pPage2_Titel_Html.clear();
    pPage2_Content.clear();
    pPage2_Notes.clear();
    pPage2_Titel_WebCast.clear();
    pPage2_ASP.clear();
    pPage2_PERL.clear();
    pPage2_URL_txt.clear();
    pPage2_URL.clear();
    pPage2_CGI_txt.clear();
    pPage2_CGI.clear();
    pPage2_Index_txt.clear();
    pPage2_Index.clear();
    pPage2_Titel_Kiosk.clear();
    pPage2_ChgDefault.clear();
    pPage2_ChgAuto.clear();
    pPage2_Duration_txt.clear();
    pPage2_Duration.clear();
    pPage2_Endless.clear();
    pPage3.clear();
    pPage3_Titel1.clear();
    pPage3_Png.clear();
    pPage3_Gif.clear();
    pPage3_Jpg.clear();
    pPage3_Quality_txt.clear();
    pPage3_Quality.clear();
    pPage3_Titel2.clear();
    pPage3_Resolution_1.clear();
    pPage3_Resolution_2.clear();
    pPage3_Resolution_3.clear();
    pPage3_Titel3.clear();
    pPage3_SldSound.clear();
    pPage3_HiddenSlides.clear();
    pPage4.clear();
    pPage4_Titel1.clear();
    pPage4_Author_txt.clear();
    pPage4_Author.clear();
    pPage4_Email_txt.clear();
    pPage4_Email.clear();
    pPage4_WWW_txt.clear();
    pPage4_WWW.clear();
    pPage4_Titel2.clear();
    pPage4_Misc.clear();
    pPage4_Download.clear();
    pPage5.clear();
    pPage5_Titel.clear();
    pPage5_TextOnly.clear();
    pPage5_Buttons.clear();
    pPage6.clear();
    pPage6_Titel.clear();
    pPage6_Default.clear();
    pPage6_User.clear();
    pPage6_Back.clear();
    pPage6_Text.clear();
    pPage6_Link.clear();
    pPage6_VLink.clear();
    pPage6_ALink.clear();
    pPage6_DocColors.clear();
    pPage6_Preview.clear();
    pLastPageButton.clear();
    pNextPageButton.clear();
    pFinishButton.clear();
    ModalDialog::dispose();
}


// Generate dialog controls and embed them in the pages
void SdPublishingDlg::CreatePages()
{
    // Page 1
    get(pPage1, "page1");
    get(pPage1_Titel, "assignLabel");
    get(pPage1_NewDesign, "newDesignRadiobutton");
    get(pPage1_OldDesign, "oldDesignRadiobutton");
    get(pPage1_DelDesign, "delDesingButton");
    get(pPage1_Desc, "descLabel");
    aAssistentFunc.InsertControl(1, pPage1);
    aAssistentFunc.InsertControl(1, pPage1_Titel);
    aAssistentFunc.InsertControl(1, pPage1_NewDesign);
    aAssistentFunc.InsertControl(1, pPage1_OldDesign);
    aAssistentFunc.InsertControl(1, pPage1_Designs);
    aAssistentFunc.InsertControl(1, pPage1_DelDesign);
    aAssistentFunc.InsertControl(1, pPage1_Desc);

    // Page 2
    get(pPage2, "page2");
    get(pPage2Frame2, "page2.2");
    get(pPage2Frame3, "page2.3");
    get(pPage2Frame4, "page2.4");
    get(pPage2_Titel, "publicationLabel");
    get(pPage2_Standard, "standardRadiobutton");
    get(pPage2_Frames, "framesRadiobutton");
    get(pPage2_SingleDocument, "singleDocumentRadiobutton");
    get(pPage2_Kiosk, "kioskRadiobutton");
    get(pPage2_WebCast, "webCastRadiobutton");
    aAssistentFunc.InsertControl(2, pPage2);
    aAssistentFunc.InsertControl(2, pPage2Frame2);
    aAssistentFunc.InsertControl(2, pPage2Frame3);
    aAssistentFunc.InsertControl(2, pPage2Frame4);
    aAssistentFunc.InsertControl(2, pPage2_Titel);
    aAssistentFunc.InsertControl(2, pPage2_Standard);
    aAssistentFunc.InsertControl(2, pPage2_Frames);
    aAssistentFunc.InsertControl(2, pPage2_SingleDocument);
    aAssistentFunc.InsertControl(2, pPage2_Kiosk);
    aAssistentFunc.InsertControl(2, pPage2_WebCast);
    aAssistentFunc.InsertControl(2, pPage2_Standard_FB);
    aAssistentFunc.InsertControl(2, pPage2_Frames_FB);
    aAssistentFunc.InsertControl(2, pPage2_Kiosk_FB);
    aAssistentFunc.InsertControl(2, pPage2_WebCast_FB);

    get(pPage2_Titel_Html, "htmlOptionsLabel");
    get(pPage2_Content, "contentCheckbutton");
    get(pPage2_Notes, "notesCheckbutton");
    aAssistentFunc.InsertControl(2, pPage2_Titel_Html);
    aAssistentFunc.InsertControl(2, pPage2_Content);
    if(m_bImpress)
        aAssistentFunc.InsertControl(2, pPage2_Notes);

    get(pPage2_Titel_WebCast, "webCastLabel");
    get(pPage2_ASP, "ASPRadiobutton");
    get(pPage2_PERL, "perlRadiobutton");
    get(pPage2_URL_txt, "URLTxtLabel");
    get(pPage2_URL, "URLEntry");
    get(pPage2_CGI_txt, "CGITxtLabel");
    get(pPage2_CGI, "CGIEntry");
    get(pPage2_Index_txt, "indexTxtLabel");
    get(pPage2_Index, "indexEntry");
    get(pPage2_Titel_Kiosk, "kioskLabel");
    get(pPage2_ChgDefault, "chgDefaultRadiobutton");
    get(pPage2_ChgAuto, "chgAutoRadiobutton");
    get(pPage2_Duration_txt, "durationTxtLabel");
    get(pPage2_Duration, "durationSpinbutton");
    get(pPage2_Endless, "endlessCheckbutton");
    aAssistentFunc.InsertControl(2, pPage2_Titel_WebCast);
    aAssistentFunc.InsertControl(2, pPage2_Index_txt);
    aAssistentFunc.InsertControl(2, pPage2_Index);
    aAssistentFunc.InsertControl(2, pPage2_ASP);
    aAssistentFunc.InsertControl(2, pPage2_PERL);
    aAssistentFunc.InsertControl(2, pPage2_URL_txt);
    aAssistentFunc.InsertControl(2, pPage2_URL);
    aAssistentFunc.InsertControl(2, pPage2_CGI_txt);
    aAssistentFunc.InsertControl(2, pPage2_CGI);
    aAssistentFunc.InsertControl(2, pPage2_Titel_Kiosk);
    aAssistentFunc.InsertControl(2, pPage2_ChgDefault);
    aAssistentFunc.InsertControl(2, pPage2_ChgAuto);
    aAssistentFunc.InsertControl(2, pPage2_Duration_txt);
    aAssistentFunc.InsertControl(2, pPage2_Duration);
    aAssistentFunc.InsertControl(2, pPage2_Endless);

    // Page 3
    get(pPage3, "page3");
    get(pPage3_Titel1, "saveImgAsLabel");
    get(pPage3_Png, "pngRadiobutton");
    get(pPage3_Gif, "gifRadiobutton");
    get(pPage3_Jpg, "jpgRadiobutton");
    get(pPage3_Quality_txt, "qualityTxtLabel");
    get(pPage3_Quality, "qualityCombobox");
    get(pPage3_Titel2, "monitorResolutionLabel");
    get(pPage3_Resolution_1, "resolution1Radiobutton");
    get(pPage3_Resolution_2, "resolution2Radiobutton");
    get(pPage3_Resolution_3, "resolution3Radiobutton");
    get(pPage3_Titel3, "effectsLabel");
    get(pPage3_SldSound, "sldSoundCheckbutton");
    get(pPage3_HiddenSlides, "hiddenSlidesCheckbutton");
    aAssistentFunc.InsertControl(3, pPage3);
    aAssistentFunc.InsertControl(3, pPage3_Titel1);
    aAssistentFunc.InsertControl(3, pPage3_Png);
    aAssistentFunc.InsertControl(3, pPage3_Gif);
    aAssistentFunc.InsertControl(3, pPage3_Jpg);
    aAssistentFunc.InsertControl(3, pPage3_Quality_txt);
    aAssistentFunc.InsertControl(3, pPage3_Quality);
    aAssistentFunc.InsertControl(3, pPage3_Titel2);
    aAssistentFunc.InsertControl(3, pPage3_Resolution_1);
    aAssistentFunc.InsertControl(3, pPage3_Resolution_2);
    aAssistentFunc.InsertControl(3, pPage3_Resolution_3);
    aAssistentFunc.InsertControl(3, pPage3_Titel3);
    aAssistentFunc.InsertControl(3, pPage3_SldSound);
    aAssistentFunc.InsertControl(3, pPage3_HiddenSlides);

    // Page 4
    get(pPage4, "page4");
    get(pPage4_Titel1, "infTitlePageLabel");
    get(pPage4_Author_txt, "authorTxtLabel");
    get(pPage4_Author, "authorEntry");
    get(pPage4_Email_txt, "emailTxtLabel");
    get(pPage4_Email, "emailEntry");
    get(pPage4_WWW_txt, "wwwTxtLabel");
    get(pPage4_WWW, "wwwEntry");
    get(pPage4_Titel2, "addInformLabel");
    get(pPage4_Download, "downloadCheckbutton");
    aAssistentFunc.InsertControl(4, pPage4);
    aAssistentFunc.InsertControl(4, pPage4_Titel1);
    aAssistentFunc.InsertControl(4, pPage4_Author_txt);
    aAssistentFunc.InsertControl(4, pPage4_Author);
    aAssistentFunc.InsertControl(4, pPage4_Email_txt);
    aAssistentFunc.InsertControl(4, pPage4_Email);
    aAssistentFunc.InsertControl(4, pPage4_WWW_txt);
    aAssistentFunc.InsertControl(4, pPage4_WWW);
    aAssistentFunc.InsertControl(4, pPage4_Titel2);
    aAssistentFunc.InsertControl(4, pPage4_Misc);
    if(m_bImpress)
        aAssistentFunc.InsertControl(4, pPage4_Download);

    // Page 5
    get(pPage5, "page5");
    get(pPage5_Titel, "buttonStyleLabel");
    get(pPage5_TextOnly, "textOnlyCheckbutton");
    get(pPage5_Buttons, "buttonsDrawingarea");
    aAssistentFunc.InsertControl(5, pPage5);
    aAssistentFunc.InsertControl(5, pPage5_Titel);
    aAssistentFunc.InsertControl(5, pPage5_TextOnly);
    aAssistentFunc.InsertControl(5, pPage5_Buttons);

    // Page 6
    get(pPage6, "page6");
    get(pPage6_Titel, "selectColorLabel");
    get(pPage6_Default, "defaultRadiobutton");
    get(pPage6_User, "userRadiobutton");
    get(pPage6_Back, "backButton");
    get(pPage6_Text, "textButton");
    get(pPage6_Link, "linkButton");
    get(pPage6_VLink, "vLinkButton");
    get(pPage6_ALink, "aLinkButton");
    get(pPage6_DocColors, "docColorsRadiobutton");
    get(pPage6_Preview, "previewDrawingarea");
    aAssistentFunc.InsertControl(6, pPage6);
    aAssistentFunc.InsertControl(6, pPage6_Titel);
    aAssistentFunc.InsertControl(6, pPage6_DocColors);
    aAssistentFunc.InsertControl(6, pPage6_Default);
    aAssistentFunc.InsertControl(6, pPage6_User);
    aAssistentFunc.InsertControl(6, pPage6_Text);
    aAssistentFunc.InsertControl(6, pPage6_Link);
    aAssistentFunc.InsertControl(6, pPage6_ALink);
    aAssistentFunc.InsertControl(6, pPage6_VLink);
    aAssistentFunc.InsertControl(6, pPage6_Back);
    aAssistentFunc.InsertControl(6, pPage6_Preview);
}

// Initialize dialog with default-values
void SdPublishingDlg::SetDefaults()
{
    SdPublishingDesign aDefault;
    SetDesign(&aDefault);

    pPage1_NewDesign->Check();
    pPage1_OldDesign->Check(false);
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
    if (pPage2_Frames->IsChecked())
        ePublishMode = PUBLISH_FRAMES;
    else if (pPage2_SingleDocument->IsChecked())
        ePublishMode = PUBLISH_SINGLE_DOCUMENT;
    else if (pPage2_Kiosk->IsChecked())
        ePublishMode  = PUBLISH_KIOSK;
    else if (pPage2_WebCast->IsChecked())
        ePublishMode  = PUBLISH_WEBCAST;
    else
        ePublishMode  = PUBLISH_HTML;

    aValue.Value <<= static_cast<sal_Int32>(ePublishMode);
    aProps.push_back( aValue );

    aValue.Name = "IsExportContentsPage";
    aValue.Value <<= pPage2_Content->IsChecked();
    aProps.push_back( aValue );

    if(m_bImpress)
    {
        aValue.Name = "IsExportNotes";
        aValue.Value <<= pPage2_Notes->IsChecked();
        aProps.push_back( aValue );
    }

    if( pPage2_WebCast->IsChecked() )
    {
        aValue.Name = "WebCastScriptLanguage";
        if( pPage2_ASP->IsChecked() )
            aValue.Value <<= OUString( "asp" );
        else
            aValue.Value <<= OUString( "perl" );
        aProps.push_back( aValue );

        aValue.Name = "WebCastCGIURL";
        aValue.Value <<= pPage2_CGI->GetText();
        aProps.push_back( aValue );

        aValue.Name = "WebCastTargetURL";
        aValue.Value <<= pPage2_URL->GetText();
        aProps.push_back( aValue );
    }
    aValue.Name = "IndexURL";
    aValue.Value <<= pPage2_Index->GetText();
    aProps.push_back( aValue );

    if( pPage2_Kiosk->IsChecked() && pPage2_ChgAuto->IsChecked() )
    {
        aValue.Name = "KioskSlideDuration";
        aValue.Value <<= static_cast<sal_uInt32>(pPage2_Duration->GetTime().GetMSFromTime()) / 1000;
        aProps.push_back( aValue );

        aValue.Name = "KioskEndless";
        aValue.Value <<= pPage2_Endless->IsChecked();
        aProps.push_back( aValue );
    }

    // Page 3

    aValue.Name = "Width";
    sal_Int32 nTmpWidth = 640;
    if( pPage3_Resolution_2->IsChecked() )
        nTmpWidth = 800;
    else if( pPage3_Resolution_3->IsChecked() )
        nTmpWidth = 1024;

    aValue.Value <<= nTmpWidth;
    aProps.push_back( aValue );

    aValue.Name = "Compression";
    aValue.Value <<= pPage3_Quality->GetText();
    aProps.push_back( aValue );

    aValue.Name = "Format";
    sal_Int32 nFormat;
    if( pPage3_Png->IsChecked() )
        nFormat = static_cast<sal_Int32>(FORMAT_PNG);
    else if( pPage3_Gif->IsChecked() )
        nFormat = static_cast<sal_Int32>(FORMAT_GIF);
    else
        nFormat = static_cast<sal_Int32>(FORMAT_JPG);
    aValue.Value <<= nFormat;
    aProps.push_back( aValue );

    aValue.Name = "SlideSound";
    aValue.Value <<= pPage3_SldSound->IsChecked();
    aProps.push_back( aValue );

    aValue.Name = "HiddenSlides";
    aValue.Value <<= pPage3_HiddenSlides->IsChecked();
    aProps.push_back( aValue );

    // Page 4
    aValue.Name = "Author";
    aValue.Value <<= pPage4_Author->GetText();
    aProps.push_back( aValue );

    aValue.Name = "EMail";
    aValue.Value <<= pPage4_Email->GetText();
    aProps.push_back( aValue );

    // try to guess protocol for user's homepage
    INetURLObject aHomeURL( pPage4_WWW->GetText(),
                            INetProtocol::Http,     // default proto is HTTP
                            INetURLObject::EncodeMechanism::All );

    aValue.Name = "HomepageURL";
    aValue.Value <<= aHomeURL.GetMainURL( INetURLObject::DecodeMechanism::NONE );
    aProps.push_back( aValue );

    aValue.Name = "UserText";
    aValue.Value <<= pPage4_Misc->GetText();
    aProps.push_back( aValue );

    if( m_bImpress )
    {
        aValue.Name = "EnableDownload";
        aValue.Value <<= pPage4_Download->IsChecked();
        aProps.push_back( aValue );
    }

    // Page 5
    if( !pPage5_TextOnly->IsChecked() )
    {
        aValue.Name = "UseButtonSet";
        aValue.Value <<= static_cast<sal_Int32>(pPage5_Buttons->GetSelectedItemId() - 1);
        aProps.push_back( aValue );
    }

    // Page 6
    if( pPage6_User->IsChecked() )
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

    if( pPage6_DocColors->IsChecked() )
    {
        aValue.Name = "IsUseDocumentColors";
        aValue.Value <<= true;
        aProps.push_back( aValue );
    }

    rParams = comphelper::containerToSequence(aProps);
}

// Clickhandler for the radiobuttons of the design-selection
IMPL_LINK( SdPublishingDlg, DesignHdl, Button *, pButton, void )
{
    if(pButton == pPage1_NewDesign)
    {
        pPage1_NewDesign->Check(); // because of DesignDeleteHdl
        pPage1_OldDesign->Check(false);
        pPage1_Designs->Disable();
        pPage1_DelDesign->Disable();
        m_pDesign = nullptr;

        SdPublishingDesign aDefault;
        SetDesign(&aDefault);
    }
    else
    {
        pPage1_NewDesign->Check(false);
        pPage1_Designs->Enable();
        pPage1_DelDesign->Enable();

        if(pPage1_Designs->GetSelectedEntryCount() == 0)
            pPage1_Designs->SelectEntryPos(0);

        const sal_Int32 nPos = pPage1_Designs->GetSelectedEntryPos();
        m_pDesign = &m_aDesignList[nPos];
        DBG_ASSERT(m_pDesign, "No Design? That's not allowed (CL)");

        if(m_pDesign)
            SetDesign(m_pDesign);
    }
}

// Clickhandler for the choice of one design
IMPL_LINK_NOARG(SdPublishingDlg, DesignSelectHdl, ListBox&, void)
{
    const sal_Int32 nPos = pPage1_Designs->GetSelectedEntryPos();
    m_pDesign = &m_aDesignList[nPos];
    DBG_ASSERT(m_pDesign, "No Design? That's not allowed (CL)");

    if(m_pDesign)
        SetDesign(m_pDesign);

    UpdatePage();
}

// Clickhandler for the delete of one design
IMPL_LINK_NOARG(SdPublishingDlg, DesignDeleteHdl, Button*, void)
{
    const sal_Int32 nPos = pPage1_Designs->GetSelectedEntryPos();

    std::vector<SdPublishingDesign>::iterator iter = m_aDesignList.begin()+nPos;

    DBG_ASSERT(iter != m_aDesignList.end(), "No Design? That's not allowed (CL)");

    pPage1_Designs->RemoveEntry(nPos);

    if(m_pDesign == &(*iter))
        DesignHdl( pPage1_NewDesign );

    m_aDesignList.erase(iter);

    m_bDesignListDirty = true;

    UpdatePage();
}

// Clickhandler for the other servertypes
IMPL_LINK( SdPublishingDlg, WebServerHdl, Button *, pButton, void )
{
    bool bASP = pButton == pPage2_ASP;

    pPage2_ASP->Check( bASP );
    pPage2_PERL->Check( !bASP );
    UpdatePage();
}

// Clickhandler for the Radiobuttons of the graphicformat choice
IMPL_LINK( SdPublishingDlg, GfxFormatHdl, Button *, pButton, void )
{
    pPage3_Png->Check( pButton == pPage3_Png );
    pPage3_Gif->Check( pButton == pPage3_Gif );
    pPage3_Jpg->Check( pButton == pPage3_Jpg );
    pPage3_Quality->Enable(pButton == pPage3_Jpg);
}

// Clickhandler for the Radiobuttons Standard/Frames
IMPL_LINK_NOARG(SdPublishingDlg, BaseHdl, Button*, void)
{
    UpdatePage();
}

// Clickhandler for the Checkbox of the Title page
IMPL_LINK_NOARG(SdPublishingDlg, ContentHdl, Button*, void)
{
    if(pPage2_Content->IsChecked())
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
IMPL_LINK( SdPublishingDlg, ResolutionHdl, Button *, pButton, void )
{
    pPage3_Resolution_1->Check(pButton == pPage3_Resolution_1);
    pPage3_Resolution_2->Check(pButton == pPage3_Resolution_2);
    pPage3_Resolution_3->Check(pButton == pPage3_Resolution_3);
}

// Clickhandler for the ValueSet with the bitmap-buttons
IMPL_LINK_NOARG(SdPublishingDlg, ButtonsHdl, ValueSet*, void)
{
    // if one bitmap-button is chosen, then disable TextOnly
    pPage5_TextOnly->Check(false);
}

// Fill the SfxItemSet with the settings of the dialog
IMPL_LINK( SdPublishingDlg, ColorHdl, Button *, pButton, void)
{
    SvColorDialog aDlg;

    if(pButton == pPage6_Back)
    {
        aDlg.SetColor( m_aBackColor );
        if(aDlg.Execute(GetFrameWeld()) == RET_OK )
            m_aBackColor = aDlg.GetColor();
    }
    else if(pButton == pPage6_Text)
    {
        aDlg.SetColor( m_aTextColor );
        if(aDlg.Execute(GetFrameWeld()) == RET_OK )
            m_aTextColor = aDlg.GetColor();
    }
    else if(pButton == pPage6_Link)
    {
        aDlg.SetColor( m_aLinkColor );
        if(aDlg.Execute(GetFrameWeld()) == RET_OK )
            m_aLinkColor = aDlg.GetColor();
    }
    else if(pButton == pPage6_VLink)
    {
        aDlg.SetColor( m_aVLinkColor );
        if(aDlg.Execute(GetFrameWeld()) == RET_OK )
            m_aVLinkColor = aDlg.GetColor();
    }
    else if(pButton == pPage6_ALink)
    {
        aDlg.SetColor( m_aALinkColor );
        if(aDlg.Execute(GetFrameWeld()) == RET_OK )
            m_aALinkColor = aDlg.GetColor();
    }

    pPage6_User->Check();
    pPage6_Preview->SetColors( m_aBackColor, m_aTextColor, m_aLinkColor,
                               m_aVLinkColor, m_aALinkColor );
    pPage6_Preview->Invalidate();
}

IMPL_LINK_NOARG(SdPublishingDlg, SlideChgHdl, Button*, void)
{
    UpdatePage();
}

// Clickhandler for the Ok Button
IMPL_LINK_NOARG(SdPublishingDlg, FinishHdl, Button*, void)
{
    //End
    SdPublishingDesign aDesign;
    GetDesign(&aDesign);

    bool bSave = false;

    if(pPage1_OldDesign->IsChecked() && m_pDesign)
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

            SdDesignNameDlg aDlg(GetFrameWeld(), aName);

            if (aDlg.run() == RET_OK)
            {
                aDesign.m_aDesignName = aDlg.GetDesignName();

                auto iter = std::find_if(m_aDesignList.begin(), m_aDesignList.end(),
                    [&aDesign](const SdPublishingDesign& rDesign) { return rDesign.m_aDesignName == aDesign.m_aDesignName; });

                if (iter != m_aDesignList.end())
                {
                    std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(GetFrameWeld(),
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

    EndDialog(RET_OK);
}

// Refresh the dialogs when changing from pages
void SdPublishingDlg::ChangePage()
{
    int nPage = aAssistentFunc.GetCurrentPage();
    SetHelpId(aPageHelpIds[nPage-1]);

    UpdatePage();

    if( pNextPageButton->IsEnabled() )
        pNextPageButton->GrabFocus();
    else
        pFinishButton->GrabFocus();
}

void SdPublishingDlg::UpdatePage()
{
    pNextPageButton->Enable(!aAssistentFunc.IsLastPage());
    pLastPageButton->Enable(!aAssistentFunc.IsFirstPage());

    int nPage = aAssistentFunc.GetCurrentPage();

    switch( nPage )
    {
    case 1:
        if(pPage1_NewDesign->IsChecked())
        {
            pPage1_Designs->Disable();
            pPage1_DelDesign->Disable();
        }

        if(m_aDesignList.empty())
            pPage1_OldDesign->Disable();
        break;
    case 2:
        pPage2_Frames_FB->Show(pPage2_Frames->IsChecked());
        pPage2_Standard_FB->Show(pPage2_Standard->IsChecked());
        pPage2_Kiosk_FB->Show(pPage2_Kiosk->IsChecked());
        pPage2_WebCast_FB->Show(pPage2_WebCast->IsChecked());

        if( pPage2_WebCast->IsChecked() )
        {
            pPage2Frame4->Show();
            pPage2_Titel_WebCast->Show();
            pPage2_ASP->Show();
            pPage2_PERL->Show();
            pPage2_URL_txt->Show();
            pPage2_URL->Show();
            pPage2_CGI_txt->Show();
            pPage2_CGI->Show();
            pPage2_Index_txt->Show();
            pPage2_Index->Show();

            bool bPerl = pPage2_PERL->IsChecked();
            pPage2_Index->Enable(bPerl);
            pPage2_Index_txt->Enable(bPerl);
            pPage2_URL_txt->Enable(bPerl);
            pPage2_URL->Enable(bPerl);
            pPage2_CGI_txt->Enable(bPerl);
            pPage2_CGI->Enable(bPerl);
        }
        else
        {
            pPage2Frame4->Hide();
            pPage2_Titel_WebCast->Hide();
            pPage2_ASP->Hide();
            pPage2_PERL->Hide();
            pPage2_URL_txt->Hide();
            pPage2_URL->Hide();
            pPage2_CGI_txt->Hide();
            pPage2_CGI->Hide();
            pPage2_Index->Hide();
            pPage2_Index_txt->Hide();
        }

        if( pPage2_Kiosk->IsChecked() )
        {
            pPage2Frame3->Show();
            pPage2_Titel_Kiosk->Show();
            pPage2_ChgDefault->Show();
            pPage2_ChgAuto->Show();
            pPage2_Duration_txt->Show();
            pPage2_Duration->Show();
            pPage2_Endless->Show();
            bool bAuto = pPage2_ChgAuto->IsChecked();
            pPage2_Duration->Enable(bAuto);
            pPage2_Endless->Enable(bAuto);
        }
        else
        {
            pPage2Frame3->Hide();
            pPage2_Titel_Kiosk->Hide();
            pPage2_ChgDefault->Hide();
            pPage2_ChgAuto->Hide();
            pPage2_Duration->Hide();
            pPage2_Duration_txt->Hide();
            pPage2_Endless->Hide();
        }

        if( pPage2_Standard->IsChecked() || pPage2_Frames->IsChecked() )
        {
            pPage2Frame2->Show();
            pPage2_Titel_Html->Show();
            pPage2_Content->Show();
            if(m_bImpress)
                pPage2_Notes->Show();
        }
        else
        {
            pPage2Frame2->Hide();
            pPage2_Titel_Html->Hide();
            pPage2_Content->Hide();
            if(m_bImpress)
                pPage2_Notes->Hide();
        }
        break;
    case 3:
        if( pPage2_Kiosk->IsChecked() || pPage2_WebCast->IsChecked() )
            pNextPageButton->Disable();

        if( pPage2_WebCast->IsChecked() )
            pPage3_SldSound->Disable();

        pPage3_Quality->Enable(pPage3_Jpg->IsChecked());

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
    if (!mpButtonSet)
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

    int nSetCount = mpButtonSet->getCount();

    int nHeight = 32;
    Image aImage;
    for( int nSet = 0; nSet < nSetCount; ++nSet )
    {
        if( mpButtonSet->getPreview( nSet, aButtonNames, aImage ) )
        {
            pPage5_Buttons->InsertItem( static_cast<sal_uInt16>(nSet)+1, aImage );
            if( nHeight < aImage.GetSizePixel().Height() )
                nHeight = aImage.GetSizePixel().Height();
        }
    }

    pPage5_Buttons->SetItemHeight( nHeight );
    m_bButtonsDirty = false;
}

// Clickhandler for the Forward Button
IMPL_LINK_NOARG(SdPublishingDlg, NextPageHdl, Button*, void)
{
    aAssistentFunc.NextPage();
    ChangePage();
}

// Sets the Controls in the dialog to the settings in the design
void SdPublishingDlg::SetDesign( SdPublishingDesign const * pDesign )
{
    if(!pDesign)
        return;

    pPage2_Standard->Check(pDesign->m_eMode == PUBLISH_HTML);
    pPage2_Frames->Check(pDesign->m_eMode == PUBLISH_FRAMES);
    pPage2_Kiosk->Check(pDesign->m_eMode == PUBLISH_KIOSK );
    pPage2_WebCast->Check(pDesign->m_eMode == PUBLISH_WEBCAST );

    pPage2_Content->Check(pDesign->m_bContentPage);
    if(pDesign->m_bContentPage)
        aAssistentFunc.EnablePage(4);
    else
        aAssistentFunc.DisablePage(4);

    if(m_bImpress)
        pPage2_Notes->Check(pDesign->m_bNotes);

    pPage2_ASP->Check(pDesign->m_eScript == SCRIPT_ASP);
    pPage2_PERL->Check(pDesign->m_eScript == SCRIPT_PERL);
    pPage2_CGI->SetText(pDesign->m_aCGI);
    pPage2_URL->SetText(pDesign->m_aURL);

    pPage2_ChgDefault->Check( !pDesign->m_bAutoSlide );
    pPage2_ChgAuto->Check( pDesign->m_bAutoSlide );

    tools::Time aTime( tools::Time::EMPTY );
    aTime.MakeTimeFromMS( pDesign->m_nSlideDuration * 1000 );
    pPage2_Duration->SetTime( aTime );

    pPage2_Endless->Check( pDesign->m_bEndless );

    pPage3_Png->Check(pDesign->m_eFormat == FORMAT_PNG);
    pPage3_Gif->Check(pDesign->m_eFormat == FORMAT_GIF);
    pPage3_Jpg->Check(pDesign->m_eFormat == FORMAT_JPG);
    pPage3_Quality->Enable(pDesign->m_eFormat == FORMAT_JPG);

    pPage3_Quality->SetText(pDesign->m_aCompression);
    pPage3_Resolution_1->Check(pDesign->m_nResolution == PUB_LOWRES_WIDTH);
    pPage3_Resolution_2->Check(pDesign->m_nResolution == PUB_MEDRES_WIDTH);
    pPage3_Resolution_3->Check(pDesign->m_nResolution == PUB_HIGHRES_WIDTH);

    pPage3_SldSound->Check( pDesign->m_bSlideSound );
    pPage3_HiddenSlides->Check( pDesign->m_bHiddenSlides );

    pPage4_Author->SetText(pDesign->m_aAuthor);
    pPage4_Email->SetText(pDesign->m_aEMail);
    pPage4_WWW->SetText(pDesign->m_aWWW);
    pPage4_Misc->SetText(pDesign->m_aMisc);
    if(m_bImpress)
        pPage4_Download->Check(pDesign->m_bDownload);

    pPage5_TextOnly->Check(pDesign->m_nButtonThema == -1);
    if(pDesign->m_nButtonThema != -1)
    {
        if(m_bButtonsDirty)
            LoadPreviewButtons();
        pPage5_Buttons->SelectItem(pDesign->m_nButtonThema + 1);
    }
    else
        pPage5_Buttons->SetNoSelection();

    pPage6_User->Check(pDesign->m_bUserAttr);
    m_aBackColor = pDesign->m_aBackColor;
    m_aTextColor = pDesign->m_aTextColor;
    m_aLinkColor = pDesign->m_aLinkColor;
    m_aVLinkColor = pDesign->m_aVLinkColor;
    m_aALinkColor  = pDesign->m_aALinkColor;

    pPage6_DocColors->Check(pDesign->m_bUseColor);

    pPage6_Preview->SetColors( m_aBackColor, m_aTextColor, m_aLinkColor,
                               m_aVLinkColor, m_aALinkColor );
    pPage6_Preview->Invalidate();

    UpdatePage();
}

// Transfer the status of the Design Dialog Controls
void SdPublishingDlg::GetDesign( SdPublishingDesign* pDesign )
{
    if(!pDesign)
        return;

    pDesign->m_eMode =  pPage2_Standard->IsChecked()?PUBLISH_HTML:
                        pPage2_Frames->IsChecked()?PUBLISH_FRAMES:
                        pPage2_Kiosk->IsChecked()?PUBLISH_KIOSK:
                        PUBLISH_WEBCAST;

    pDesign->m_bContentPage = pPage2_Content->IsChecked();
    if(m_bImpress)
        pDesign->m_bNotes = pPage2_Notes->IsChecked();

    if( pPage3_Gif->IsChecked() )
        pDesign->m_eFormat = FORMAT_GIF;
    else if( pPage3_Jpg->IsChecked() )
        pDesign->m_eFormat = FORMAT_JPG;
    else
        pDesign->m_eFormat = FORMAT_PNG;

    pDesign->m_aCompression = pPage3_Quality->GetText();

    pDesign->m_nResolution = pPage3_Resolution_1->IsChecked()?PUB_LOWRES_WIDTH:
                            (pPage3_Resolution_2->IsChecked()?PUB_MEDRES_WIDTH:PUB_HIGHRES_WIDTH);

    pDesign->m_bSlideSound = pPage3_SldSound->IsChecked();
    pDesign->m_bHiddenSlides = pPage3_HiddenSlides->IsChecked();

    pDesign->m_aAuthor = pPage4_Author->GetText();
    pDesign->m_aEMail = pPage4_Email->GetText();
    pDesign->m_aWWW = pPage4_WWW->GetText();
    pDesign->m_aMisc = pPage4_Misc->GetText();
    pDesign->m_bDownload = m_bImpress && pPage4_Download->IsChecked();

    if(pPage5_TextOnly->IsChecked())
        pDesign->m_nButtonThema = -1;
    else
        pDesign->m_nButtonThema = pPage5_Buttons->GetSelectedItemId() - 1;

    pDesign->m_bUserAttr = pPage6_User->IsChecked();
    pDesign->m_aBackColor = m_aBackColor;
    pDesign->m_aTextColor = m_aTextColor;
    pDesign->m_aLinkColor = m_aLinkColor;
    pDesign->m_aVLinkColor = m_aVLinkColor;
    pDesign->m_aALinkColor  = m_aALinkColor;
    pDesign->m_bUseColor   = pPage6_DocColors->IsChecked();

    pDesign->m_eScript = pPage2_ASP->IsChecked()?SCRIPT_ASP:SCRIPT_PERL;
    pDesign->m_aCGI = pPage2_CGI->GetText();
    pDesign->m_aURL = pPage2_URL->GetText();

    pDesign->m_bAutoSlide = pPage2_ChgAuto->IsChecked();
    pDesign->m_nSlideDuration = static_cast<sal_uInt32>(pPage2_Duration->GetTime().GetMSFromTime()) / 1000;
    pDesign->m_bEndless = pPage2_Endless->IsChecked();
}

// Clickhandler for the back Button
IMPL_LINK_NOARG(SdPublishingDlg, LastPageHdl, Button*, void)
{
    aAssistentFunc.PreviousPage();
    ChangePage();
}

// Load Designs
void SdPublishingDlg::Load()
{
    m_bDesignListDirty = false;

    INetURLObject aURL( SvtPathOptions().GetUserConfigPath() );
    aURL.Append( "designs.sod" );

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

    sal_uInt16 nDesigns;
    pStream->ReadUInt16( nDesigns );

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
    aURL.Append( "designs.sod" );
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

std::vector<OString> SdPublishingDlg::getAllPageUIXMLDescriptions() const
{
    // this dialog has a hard number of pages
    std::vector<OString> aRetval;

    for (sal_uInt32 a(0); a < 6; a++)
    {
        aRetval.push_back(OString::number(a));
    }

    return aRetval;
}

bool SdPublishingDlg::selectPageByUIXMLDescription(const OString& rUIXMLDescription)
{
    // rUIXMLDescription contains one of the values above, make use of it
    const sal_uInt32 nPage(rUIXMLDescription.toUInt32());

    if (nPage < 6)
    {
        aAssistentFunc.GotoPage(nPage + 1);
        return true;
    }

    return false;
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

OUString SdDesignNameDlg::GetDesignName()
{
    return m_xEdit->get_text();
}

IMPL_LINK_NOARG(SdDesignNameDlg, ModifyHdl, weld::Entry&, void)
{
    m_xBtnOK->set_sensitive(!m_xEdit->get_text().isEmpty());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
