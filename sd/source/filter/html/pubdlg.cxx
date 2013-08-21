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
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/FilterConfigItem.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/combobox.hxx>
#include <svtools/svmedit.hxx>
#include <svl/intitem.hxx>
#include <svl/aeitem.hxx>
#include <svl/itemset.hxx>
#include <svl/stritem.hxx>
#include <vcl/msgbox.hxx>
#include <svtools/valueset.hxx>
#include <vcl/graph.hxx>
#include <svl/eitem.hxx>
#include <svtools/colrdlg.hxx>
#include <editeng/colritem.hxx>
#include <tools/urlobj.hxx>
#include <sdiocmpt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/app.hxx>
#include <pres.hxx>
#include <unotools/useroptions.hxx>
#include <unotools/pathoptions.hxx>

#include "sdresid.hxx"
#include "sdattr.hxx"
#include "pubdlg.hrc"
#include "pubdlg.hxx"
#include "htmlattr.hxx"
#include "htmlex.hxx"
#include "htmlpublishmode.hxx"
#include "helpids.h"
#include "buttonset.hxx"

using namespace std;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;


#define NOOFPAGES 6

extern void InterpolateFixedBitmap( FixedBitmap * pBitmap );

//ID for the config-data with the HTML-settings
const sal_uInt16 nMagic = (sal_uInt16)0x1977;

// Key for the soffice.ini
#define KEY_QUALITY     "JPG-EXPORT-QUALITY"

// The Help-IDs of the pages
const char* aPageHelpIds[NOOFPAGES] =
{
    HID_SD_HTMLEXPORT_PAGE1,
    HID_SD_HTMLEXPORT_PAGE2,
    HID_SD_HTMLEXPORT_PAGE3,
    HID_SD_HTMLEXPORT_PAGE4,
    HID_SD_HTMLEXPORT_PAGE5,
    HID_SD_HTMLEXPORT_PAGE6
};

// This class has all the settings for the HTML-export autopilot
class SdPublishingDesign
{
public:
    String  m_aDesignName;

    HtmlPublishMode m_eMode;

    // special WebCast options
    PublishingScript m_eScript;
    String           m_aCGI;
    String           m_aURL;

    // special Kiosk options
    sal_Bool    m_bAutoSlide;
    sal_uInt32  m_nSlideDuration;
    sal_Bool    m_bEndless;

    // special HTML options
    sal_Bool    m_bContentPage;
    sal_Bool    m_bNotes;

    // misc options
    sal_uInt16  m_nResolution;
    String  m_aCompression;
    PublishingFormat m_eFormat;
    sal_Bool    m_bSlideSound;
    sal_Bool    m_bHiddenSlides;

    // titel page information
    String  m_aAuthor;
    String  m_aEMail;
    String  m_aWWW;
    String  m_aMisc;
    sal_Bool    m_bDownload;
    sal_Bool    m_bCreated;         // not used

    // buttons and colorscheme
    sal_Int16   m_nButtonThema;
    sal_Bool    m_bUserAttr;
    Color   m_aBackColor;
    Color   m_aTextColor;
    Color   m_aLinkColor;
    Color   m_aVLinkColor;
    Color   m_aALinkColor;
    sal_Bool    m_bUseAttribs;
    sal_Bool    m_bUseColor;

    SdPublishingDesign();

    int operator ==(const SdPublishingDesign & rDesign) const;
    friend SvStream& operator >> (SvStream& rIn, SdPublishingDesign& rDesign);
    friend SvStream& operator << (SvStream& rOut, const SdPublishingDesign& rDesign);
};

// load Default-settings
SdPublishingDesign::SdPublishingDesign()
{
    m_eMode = PUBLISH_HTML;
    m_bContentPage = sal_True;
    m_bNotes = sal_True;

    m_eFormat = FORMAT_PNG;

    String  aFilterConfigPath( "Office.Common/Filter/Graphic/Export/JPG" );
    FilterConfigItem aFilterConfigItem( aFilterConfigPath );
    sal_Int32 nCompression = aFilterConfigItem.ReadInt32( OUString( KEY_QUALITY ), 75 );
    m_aCompression = OUString::number( nCompression );
    m_aCompression.Append( sal_Unicode('%') );

    SvtUserOptions aUserOptions;

    m_nResolution   = PUB_LOWRES_WIDTH;
    m_aAuthor       = aUserOptions.GetFirstName();
    if( m_aAuthor.Len() && !aUserOptions.GetLastName().isEmpty() )
        m_aAuthor      += sal_Unicode(' ');
    m_aAuthor      += (String)aUserOptions.GetLastName();
    m_aEMail        = aUserOptions.GetEmail();
    m_bDownload     = sal_False;
    m_nButtonThema  = -1;

    m_bUserAttr     = sal_False;
    m_bUseAttribs   = sal_True;
    m_bUseColor     = sal_True;

    m_aBackColor    = COL_WHITE;
    m_aTextColor    = COL_BLACK;
    m_aLinkColor    = COL_BLUE;
    m_aVLinkColor   = COL_LIGHTBLUE;
    m_aALinkColor   = COL_GRAY;

    m_eScript       = SCRIPT_ASP;

    m_bAutoSlide     = sal_True;
    m_nSlideDuration = 15;
    m_bEndless       = sal_True;

    m_bSlideSound    = sal_True;
    m_bHiddenSlides  = sal_False;
}

// Compares the values without paying attention to the name
int SdPublishingDesign::operator ==(const SdPublishingDesign & rDesign) const
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
    SdIOCompat aIO(rIn, STREAM_READ);

    sal_uInt16 nTemp16;

    rDesign.m_aDesignName = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rIn,
        RTL_TEXTENCODING_UTF8);
    rIn >> nTemp16;
    rDesign.m_eMode = (HtmlPublishMode)nTemp16;
    rIn >> rDesign.m_bContentPage;
    rIn >> rDesign.m_bNotes;
    rIn >> rDesign.m_nResolution;
    rDesign.m_aCompression = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rIn,
        RTL_TEXTENCODING_UTF8);
    rIn >> nTemp16;
    rDesign.m_eFormat = (PublishingFormat)nTemp16;
    rDesign.m_aAuthor = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rIn,
        RTL_TEXTENCODING_UTF8);
    rDesign.m_aEMail = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rIn,
        RTL_TEXTENCODING_UTF8);
    rDesign.m_aWWW = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rIn,
        RTL_TEXTENCODING_UTF8);
    rDesign.m_aMisc = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rIn,
        RTL_TEXTENCODING_UTF8);
    rIn >> rDesign.m_bDownload;
    rIn >> rDesign.m_bCreated;      // not used
    rIn >> rDesign.m_nButtonThema;
    rIn >> rDesign.m_bUserAttr;
    rIn >> rDesign.m_aBackColor;
    rIn >> rDesign.m_aTextColor;
    rIn >> rDesign.m_aLinkColor;
    rIn >> rDesign.m_aVLinkColor;
    rIn >> rDesign.m_aALinkColor;
    rIn >> rDesign.m_bUseAttribs;
    rIn >> rDesign.m_bUseColor;

    rIn >> nTemp16;
    rDesign.m_eScript = (PublishingScript)nTemp16;
    rDesign.m_aURL = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rIn,
        RTL_TEXTENCODING_UTF8);
    rDesign.m_aCGI = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rIn,
        RTL_TEXTENCODING_UTF8);

    rIn >> rDesign.m_bAutoSlide;
    rIn >> rDesign.m_nSlideDuration;
    rIn >> rDesign.m_bEndless;
    rIn >> rDesign.m_bSlideSound;
    rIn >> rDesign.m_bHiddenSlides;

    return rIn;
}

// Set the design to the stream
SvStream& operator << (SvStream& rOut, const SdPublishingDesign& rDesign)
{
    // The last parameter is the versionnumber of the code
    SdIOCompat aIO(rOut, STREAM_WRITE, 0);

    // Name
    write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rOut, rDesign.m_aDesignName,
        RTL_TEXTENCODING_UTF8);

    rOut << (sal_uInt16)rDesign.m_eMode;
    rOut << rDesign.m_bContentPage;
    rOut << rDesign.m_bNotes;
    rOut << rDesign.m_nResolution;
    write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rOut, rDesign.m_aCompression,
        RTL_TEXTENCODING_UTF8);
    rOut << (sal_uInt16)rDesign.m_eFormat;
    write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rOut, rDesign.m_aAuthor,
        RTL_TEXTENCODING_UTF8);
    write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rOut, rDesign.m_aEMail,
        RTL_TEXTENCODING_UTF8);
    write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rOut, rDesign.m_aWWW,
        RTL_TEXTENCODING_UTF8);
    write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rOut, rDesign.m_aMisc,
        RTL_TEXTENCODING_UTF8);
    rOut << rDesign.m_bDownload;
    rOut << rDesign.m_bCreated;     // not used
    rOut << rDesign.m_nButtonThema;
    rOut << rDesign.m_bUserAttr;
    rOut << rDesign.m_aBackColor;
    rOut << rDesign.m_aTextColor;
    rOut << rDesign.m_aLinkColor;
    rOut << rDesign.m_aVLinkColor;
    rOut << rDesign.m_aALinkColor;
    rOut << rDesign.m_bUseAttribs;
    rOut << rDesign.m_bUseColor;

    rOut << (sal_uInt16)rDesign.m_eScript;
    write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rOut, rDesign.m_aURL,
        RTL_TEXTENCODING_UTF8);
    write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rOut, rDesign.m_aCGI,
        RTL_TEXTENCODING_UTF8);

    rOut << rDesign.m_bAutoSlide;
    rOut << rDesign.m_nSlideDuration;
    rOut << rDesign.m_bEndless;
    rOut << rDesign.m_bSlideSound;
    rOut << rDesign.m_bHiddenSlides;

    return rOut;
}

// Dialog for the entry of the name of the design
class SdDesignNameDlg : public ModalDialog
{
private:
    Edit            m_aEdit;
    OKButton        m_aBtnOK;
    CancelButton    m_aBtnCancel;

public:
    SdDesignNameDlg(Window* pWindow, const String& aName );

    String GetDesignName();
    DECL_LINK(ModifyHdl, void *);
};

// SdPublishingDlg Methods

SdPublishingDlg::SdPublishingDlg(Window* pWindow, DocumentType eDocType)
:   ModalDialog(pWindow, SdResId( DLG_PUBLISHING ))
,   mpButtonSet( new ButtonSet() )
,   aBottomLine( this, SdResId( BOTTOM_LINE ) )
,   aHelpButton(this,SdResId(BUT_HELP))
,   aCancelButton(this,SdResId(BUT_CANCEL))
,   aLastPageButton(this,SdResId(BUT_LAST))
,   aNextPageButton(this,SdResId(BUT_NEXT))
,   aFinishButton(this,SdResId(BUT_FINISH))
,   aAssistentFunc(NOOFPAGES)
,   m_bButtonsDirty(sal_True)
,   m_bDesignListDirty(sal_False)
,   m_pDesign(NULL)
{
    m_bImpress = eDocType == DOCUMENT_TYPE_IMPRESS;

    CreatePages();
    Load();

    // sets the output page
    aAssistentFunc.GotoPage(1);
    aLastPageButton.Disable();

    // button assignment
    aFinishButton.SetClickHdl(LINK(this,SdPublishingDlg,FinishHdl));
    aLastPageButton.SetClickHdl(LINK(this,SdPublishingDlg,LastPageHdl));
    aNextPageButton.SetClickHdl(LINK(this,SdPublishingDlg,NextPageHdl));

    pPage1_NewDesign->SetClickHdl(LINK(this,SdPublishingDlg,DesignHdl));
    pPage1_OldDesign->SetClickHdl(LINK(this,SdPublishingDlg,DesignHdl));
    pPage1_Designs->SetSelectHdl(LINK(this,SdPublishingDlg,DesignSelectHdl));
    pPage1_DelDesign->SetClickHdl(LINK(this,SdPublishingDlg,DesignDeleteHdl));

    pPage2_Standard->SetClickHdl(LINK(this,SdPublishingDlg,BaseHdl));
    pPage2_Standard_FB->SetBorderStyle(WINDOW_BORDER_MONO);
    pPage2_Frames->SetClickHdl(LINK(this,SdPublishingDlg,BaseHdl));
    pPage2_Frames_FB->SetBorderStyle(WINDOW_BORDER_MONO);
    pPage2_Kiosk->SetClickHdl(LINK(this,SdPublishingDlg,BaseHdl));
    pPage2_Kiosk_FB->SetBorderStyle(WINDOW_BORDER_MONO);
    pPage2_WebCast->SetClickHdl(LINK(this,SdPublishingDlg,BaseHdl));
    pPage2_WebCast_FB->SetBorderStyle(WINDOW_BORDER_MONO);

    pPage2_Content->SetClickHdl(LINK(this,SdPublishingDlg,ContentHdl));

    pPage2_ASP->SetClickHdl(LINK(this,SdPublishingDlg,WebServerHdl));
    pPage2_PERL->SetClickHdl(LINK(this,SdPublishingDlg,WebServerHdl));
    String  aText( OUString("index") );
    aText += String(SdResId(STR_HTMLEXP_DEFAULT_EXTENSION));
    pPage2_Index->SetText(aText);
    pPage2_CGI->SetText( OUString( "/cgi-bin/" ) );

    pPage3_Png->SetClickHdl(LINK(this,SdPublishingDlg, GfxFormatHdl));
    pPage3_Gif->SetClickHdl(LINK(this,SdPublishingDlg, GfxFormatHdl));
    pPage3_Jpg->SetClickHdl(LINK(this,SdPublishingDlg, GfxFormatHdl));
    pPage3_Quality->Enable(sal_False);

    pPage3_Resolution_1->SetClickHdl(LINK(this,SdPublishingDlg, ResolutionHdl ));
    pPage3_Resolution_2->SetClickHdl(LINK(this,SdPublishingDlg, ResolutionHdl ));
    pPage3_Resolution_3->SetClickHdl(LINK(this,SdPublishingDlg, ResolutionHdl ));

    pPage2_ChgDefault->SetClickHdl(LINK(this,SdPublishingDlg, SlideChgHdl));
    pPage2_ChgAuto->SetClickHdl(LINK(this,SdPublishingDlg, SlideChgHdl));
    pPage2_Duration->SetFormat( TIMEF_SEC );

    pPage5_Buttons->SetSelectHdl(LINK(this,SdPublishingDlg, ButtonsHdl ));
    pPage5_Buttons->SetStyle( pPage5_Buttons->GetStyle() | WB_VSCROLL );

    pPage6_Back->SetClickHdl(LINK(this,SdPublishingDlg, ColorHdl ));
    pPage6_Text->SetClickHdl(LINK(this,SdPublishingDlg, ColorHdl ));
    pPage6_Link->SetClickHdl(LINK(this,SdPublishingDlg, ColorHdl ));
    pPage6_VLink->SetClickHdl(LINK(this,SdPublishingDlg, ColorHdl ));
    pPage6_ALink->SetClickHdl(LINK(this,SdPublishingDlg, ColorHdl ));

    pPage6_DocColors->Check();

    FreeResource();

    pPage3_Quality->InsertEntry( OUString( "25%" ) );
    pPage3_Quality->InsertEntry( OUString( "50%" ) );
    pPage3_Quality->InsertEntry( OUString( "75%" ) );
    pPage3_Quality->InsertEntry( OUString( "100%" ) );

    pPage5_Buttons->SetColCount( 1 );
    pPage5_Buttons->SetLineCount( 4 );
    pPage5_Buttons->SetExtraSpacing( 1 );

    boost::ptr_vector<SdPublishingDesign>::iterator it;
    for( it = m_aDesignList.begin(); it != m_aDesignList.end(); ++it )
        pPage1_Designs->InsertEntry(it->m_aDesignName);

    pPage6_Preview->SetBorderStyle(WINDOW_BORDER_MONO);

    SetDefaults();

    SetHelpId(aPageHelpIds[0]);

    aNextPageButton.GrabFocus();
}

SdPublishingDlg::~SdPublishingDlg()
{
    RemovePages();
}

// Generate dialog controls and embed them in the pages
void SdPublishingDlg::CreatePages()
{
    // Page 1
    aAssistentFunc.InsertControl(1,
        pPage1_Titel = new FixedLine(this,SdResId(PAGE1_TITEL)));
    aAssistentFunc.InsertControl(1,
        pPage1_NewDesign = new RadioButton(this,SdResId(PAGE1_NEW_DESIGN)));
    aAssistentFunc.InsertControl(1,
        pPage1_OldDesign = new RadioButton(this,SdResId(PAGE1_OLD_DESIGN)));
    aAssistentFunc.InsertControl(1,
        pPage1_Designs = new ListBox(this,SdResId(PAGE1_DESIGNS)));
    aAssistentFunc.InsertControl(1,
        pPage1_DelDesign = new PushButton(this,SdResId(PAGE1_DEL_DESIGN)));
    aAssistentFunc.InsertControl(1,
        pPage1_Desc = new FixedText(this,SdResId(PAGE1_DESC)));


    // Page 2
    aAssistentFunc.InsertControl(2,
        pPage2_Titel = new FixedLine(this,SdResId(PAGE2_TITEL )));
    aAssistentFunc.InsertControl(2,
        pPage2_Standard = new RadioButton(this,SdResId(PAGE2_STANDARD)));
    aAssistentFunc.InsertControl(2,
        pPage2_Frames = new RadioButton(this,SdResId(PAGE2_FRAMES)));
    aAssistentFunc.InsertControl(2,
        pPage2_Kiosk = new RadioButton(this,SdResId(PAGE2_KIOSK)));
    aAssistentFunc.InsertControl(2,
        pPage2_WebCast = new RadioButton(this,SdResId(PAGE2_WEBCAST)));
    aAssistentFunc.InsertControl(2,
        pPage2_Standard_FB = new FixedBitmap(this,SdResId(PAGE2_NOFRAMES_FB)));
    aAssistentFunc.InsertControl(2,
        pPage2_Frames_FB = new FixedBitmap(this,SdResId(PAGE2_FRAMES_FB)));
    aAssistentFunc.InsertControl(2,
        pPage2_Kiosk_FB = new FixedBitmap(this,SdResId(PAGE2_KIOSK_FB)));
    aAssistentFunc.InsertControl(2,
        pPage2_WebCast_FB = new FixedBitmap(this,SdResId(PAGE2_WEBCAST_FB)));

    aAssistentFunc.InsertControl(2,
        pPage2_Titel_Html = new FixedLine(this,SdResId(PAGE2_TITEL_HTML)));
    aAssistentFunc.InsertControl(2,
        pPage2_Content = new CheckBox(this,SdResId(PAGE2_CONTENT)));
    if(m_bImpress)
        aAssistentFunc.InsertControl(2,
            pPage2_Notes = new CheckBox(this,SdResId(PAGE2_NOTES)));

    aAssistentFunc.InsertControl(2,
        pPage2_Titel_WebCast = new FixedLine(this,SdResId(PAGE2_TITEL_WEBCAST)));
    aAssistentFunc.InsertControl(2,
        pPage2_Index_txt = new FixedText(this,SdResId(PAGE2_INDEX_TXT)));
    aAssistentFunc.InsertControl(2,
        pPage2_Index = new Edit(this,SdResId(PAGE2_INDEX)));
    aAssistentFunc.InsertControl(2,
        pPage2_ASP = new RadioButton(this,SdResId(PAGE2_ASP)));
    aAssistentFunc.InsertControl(2,
        pPage2_PERL = new RadioButton(this,SdResId(PAGE2_PERL)));
    aAssistentFunc.InsertControl(2,
        pPage2_URL_txt = new FixedText(this,SdResId(PAGE2_URL_TXT)));
    aAssistentFunc.InsertControl(2,
        pPage2_URL = new Edit(this,SdResId(PAGE2_URL)));
    aAssistentFunc.InsertControl(2,
        pPage2_CGI_txt = new FixedText(this,SdResId(PAGE2_CGI_TXT)));
    aAssistentFunc.InsertControl(2,
        pPage2_CGI = new Edit(this,SdResId(PAGE2_CGI)));
    aAssistentFunc.InsertControl(2,
        pPage2_Vert = new FixedLine( this,SdResId( PAGE2_VERT )));
    aAssistentFunc.InsertControl(2,
        pPage2_Titel_Kiosk = new FixedLine(this,SdResId(PAGE2_TITEL_KIOSK)));
    aAssistentFunc.InsertControl(2,
        pPage2_ChgDefault = new RadioButton(this,SdResId(PAGE2_CHG_DEFAULT)));
    aAssistentFunc.InsertControl(2,
        pPage2_ChgAuto = new RadioButton(this,SdResId(PAGE2_CHG_AUTO)));
    aAssistentFunc.InsertControl(2,
        pPage2_Duration_txt = new FixedText(this,SdResId(PAGE2_DURATION_TXT)));
    aAssistentFunc.InsertControl(2,
        pPage2_Duration = new TimeField(this,SdResId(PAGE2_DURATION_TMF)));
    aAssistentFunc.InsertControl(2,
        pPage2_Endless = new CheckBox(this,SdResId(PAGE2_ENDLESS)));

    // Page 3
    aAssistentFunc.InsertControl(3,
        pPage3_Titel1 = new FixedLine(this,SdResId(PAGE3_TITEL_1)));
    aAssistentFunc.InsertControl(3,
        pPage3_Png = new RadioButton(this,SdResId(PAGE3_PNG)));
    aAssistentFunc.InsertControl(3,
        pPage3_Gif = new RadioButton(this,SdResId(PAGE3_GIF)));
    aAssistentFunc.InsertControl(3,
        pPage3_Jpg = new RadioButton(this,SdResId(PAGE3_JPG)));
    aAssistentFunc.InsertControl(3,
        pPage3_Quality_txt = new FixedText(this,SdResId(PAGE3_QUALITY_TXT)));
    aAssistentFunc.InsertControl(3,
        pPage3_Quality = new ComboBox(this,SdResId(PAGE3_QUALITY)));
    aAssistentFunc.InsertControl(3,
        pPage3_Vert = new FixedLine( this,SdResId( PAGE3_VERT )));
    aAssistentFunc.InsertControl(3,
        pPage3_Titel2 = new FixedLine(this,SdResId(PAGE3_TITEL_2)));
    aAssistentFunc.InsertControl(3,
        pPage3_Resolution_1 = new RadioButton(this,SdResId(PAGE3_RESOLUTION_1)));
    aAssistentFunc.InsertControl(3,
        pPage3_Resolution_2 = new RadioButton(this,SdResId(PAGE3_RESOLUTION_2)));
    aAssistentFunc.InsertControl(3,
        pPage3_Resolution_3 = new RadioButton(this,SdResId(PAGE3_RESOLUTION_3)));
    aAssistentFunc.InsertControl(3,
        pPage3_Titel3 = new FixedLine(this,SdResId(PAGE3_TITEL_3)));
    aAssistentFunc.InsertControl(3,
        pPage3_SldSound = new CheckBox(this,SdResId(PAGE3_SLD_SOUND)));
    aAssistentFunc.InsertControl(3,
        pPage3_HiddenSlides = new CheckBox(this,SdResId(PAGE3_HIDDEN_SLIDES)));

    // Page 4
    aAssistentFunc.InsertControl(4,
        pPage4_Titel1 = new FixedLine(this,SdResId(PAGE4_TITEL_1)));
    aAssistentFunc.InsertControl(4,
        pPage4_Author_txt = new FixedText(this,SdResId(PAGE4_AUTHOR_TXT)));
    aAssistentFunc.InsertControl(4,
        pPage4_Author = new Edit(this,SdResId(PAGE4_AUTHOR)));
    aAssistentFunc.InsertControl(4,
        pPage4_Email_txt = new FixedText(this,SdResId(PAGE4_EMAIL_TXT)));
    aAssistentFunc.InsertControl(4,
        pPage4_Email = new Edit(this,SdResId(PAGE4_EMAIL_EDIT)));
    aAssistentFunc.InsertControl(4,
        pPage4_WWW_txt = new FixedText(this,SdResId(PAGE4_WWW_TXT)));
    aAssistentFunc.InsertControl(4,
        pPage4_WWW = new Edit(this,SdResId(PAGE4_WWW_EDIT)));
    aAssistentFunc.InsertControl(4,
        pPage4_Titel2 = new FixedText(this,SdResId(PAGE4_TITEL_2)));
    aAssistentFunc.InsertControl(4,
        pPage4_Misc = new MultiLineEdit(this,SdResId(PAGE4_MISC)));
    if(m_bImpress)
        aAssistentFunc.InsertControl(4,
            pPage4_Download = new CheckBox(this,SdResId(PAGE4_DOWNLOAD)));

    // Page 5
    aAssistentFunc.InsertControl(5,
        pPage5_Titel = new FixedLine(this,SdResId(PAGE5_TITEL)));
    aAssistentFunc.InsertControl(5,
        pPage5_TextOnly = new CheckBox(this, SdResId(PAGE5_TEXTONLY)));
    aAssistentFunc.InsertControl(5,
        pPage5_Buttons = new ValueSet(this,SdResId(PAGE5_BUTTONS)));

    // Page 6
    aAssistentFunc.InsertControl(6,
        pPage6_Titel = new FixedLine(this,SdResId(PAGE6_TITEL)));
    aAssistentFunc.InsertControl(6,
        pPage6_DocColors = new RadioButton(this,SdResId(PAGE6_DOCCOLORS)));
    aAssistentFunc.InsertControl(6,
        pPage6_Default = new RadioButton(this,SdResId(PAGE6_DEFAULT)));
    aAssistentFunc.InsertControl(6,
        pPage6_User = new RadioButton(this,SdResId(PAGE6_USER)));
    aAssistentFunc.InsertControl(6,
        pPage6_Text = new PushButton(this,SdResId(PAGE6_TEXT)));
    aAssistentFunc.InsertControl(6,
        pPage6_Link = new PushButton(this,SdResId(PAGE6_LINK)));
    aAssistentFunc.InsertControl(6,
        pPage6_ALink = new PushButton(this,SdResId(PAGE6_ALINK)));
    aAssistentFunc.InsertControl(6,
        pPage6_VLink = new PushButton(this,SdResId(PAGE6_VLINK)));
    aAssistentFunc.InsertControl(6,
        pPage6_Back = new PushButton(this,SdResId(PAGE6_BACK)));
    aAssistentFunc.InsertControl(6,
        pPage6_Preview = new SdHtmlAttrPreview(this,SdResId(PAGE6_PREVIEW)));

    InterpolateFixedBitmap(pPage2_Standard_FB);
    InterpolateFixedBitmap(pPage2_Frames_FB);
    InterpolateFixedBitmap(pPage2_Kiosk_FB);
    InterpolateFixedBitmap(pPage2_WebCast_FB);
}

// Delete the controls of the dialog
void SdPublishingDlg::RemovePages()
{
    delete pPage1_Titel;
    delete pPage1_NewDesign;
    delete pPage1_OldDesign;
    delete pPage1_Designs;
    delete pPage1_DelDesign;
    delete pPage1_Desc;

    delete pPage2_Titel;
    delete pPage2_Standard;
    delete pPage2_Frames;
    delete pPage2_Kiosk;
    delete pPage2_WebCast;
    delete pPage2_Standard_FB;
    delete pPage2_Frames_FB;
    delete pPage2_Kiosk_FB;
    delete pPage2_WebCast_FB;

    delete pPage2_Titel_Html;
    delete pPage2_Content;
    if(m_bImpress)
        delete pPage2_Notes;

    delete pPage2_Vert;
    delete pPage2_Titel_WebCast;
    delete pPage2_Index_txt;
    delete pPage2_Index;
    delete pPage2_ASP;
    delete pPage2_PERL;
    delete pPage2_URL_txt;
    delete pPage2_URL;
    delete pPage2_CGI_txt;
    delete pPage2_CGI;

    delete pPage2_Titel_Kiosk;
    delete pPage2_ChgDefault;
    delete pPage2_ChgAuto;
    delete pPage2_Duration_txt;
    delete pPage2_Duration;
    delete pPage2_Endless;

    delete pPage3_Titel1;
    delete pPage3_Png;
    delete pPage3_Gif;
    delete pPage3_Jpg;
    delete pPage3_Quality_txt;
    delete pPage3_Quality;
    delete pPage3_Vert;
    delete pPage3_Titel2;
    delete pPage3_Resolution_1;
    delete pPage3_Resolution_2;
    delete pPage3_Resolution_3;
    delete pPage3_Titel3;
    delete pPage3_SldSound;
    delete pPage3_HiddenSlides;

    delete pPage4_Titel1;
    delete pPage4_Author_txt;
    delete pPage4_Author;
    delete pPage4_Email_txt;
    delete pPage4_Email;
    delete pPage4_WWW_txt;
    delete pPage4_WWW;
    delete pPage4_Titel2;
    delete pPage4_Misc;
    if(m_bImpress)
        delete pPage4_Download;

    delete pPage5_Titel;
    delete pPage5_TextOnly;
    delete pPage5_Buttons;

    delete pPage6_Titel;
    delete pPage6_Default;
    delete pPage6_User;
    delete pPage6_Back;
    delete pPage6_Text;
    delete pPage6_Link;
    delete pPage6_VLink;
    delete pPage6_ALink;
    delete pPage6_DocColors;
    delete pPage6_Preview;
}

// Initialize dialog with default-values
void SdPublishingDlg::SetDefaults()
{
    SdPublishingDesign aDefault;
    SetDesign(&aDefault);

    pPage1_NewDesign->Check(sal_True);
    pPage1_OldDesign->Check(sal_False);
    UpdatePage();
}

// Feed the SfxItemSet with the settings of the dialog
void SdPublishingDlg::GetParameterSequence( Sequence< PropertyValue >& rParams )
{
    std::vector< PropertyValue > aProps;

    PropertyValue aValue;


    // Page 2
    aValue.Name = "PublishMode";
    aValue.Value <<= (sal_Int32)(pPage2_Standard->IsChecked()?PUBLISH_HTML:
                                          pPage2_Frames->IsChecked()?PUBLISH_FRAMES:
                                          pPage2_Kiosk->IsChecked()?PUBLISH_KIOSK:PUBLISH_WEBCAST);
    aProps.push_back( aValue );

    aValue.Name = "IsExportContentsPage";
    aValue.Value <<= (sal_Bool)pPage2_Content->IsChecked();
    aProps.push_back( aValue );

    if(m_bImpress)
    {
        aValue.Name = "IsExportNotes";
        aValue.Value <<= (sal_Bool)pPage2_Notes->IsChecked();
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
        aValue.Value <<= OUString( pPage2_CGI->GetText() );
        aProps.push_back( aValue );

        aValue.Name = "WebCastTargetURL";
        aValue.Value <<= OUString( pPage2_URL->GetText() );
        aProps.push_back( aValue );
    }
    aValue.Name = "IndexURL";
    aValue.Value <<= OUString( pPage2_Index->GetText() );
    aProps.push_back( aValue );


    if( pPage2_Kiosk->IsChecked() && pPage2_ChgAuto->IsChecked() )
    {
        aValue.Name = "KioskSlideDuration";
        aValue.Value <<= (sal_uInt32)pPage2_Duration->GetTime().GetMSFromTime() / 1000;
        aProps.push_back( aValue );

        aValue.Name = "KioskEndless";
        aValue.Value <<= (sal_Bool)pPage2_Endless->IsChecked();
        aProps.push_back( aValue );
    }

    // Page 3

    aValue.Name = "Width";
    sal_Int32 nTmpWidth = 512;
    if( pPage3_Resolution_2->IsChecked() )
        nTmpWidth = 640;
    else if( pPage3_Resolution_3->IsChecked() )
        nTmpWidth = 800;

    aValue.Value <<= nTmpWidth;
    aProps.push_back( aValue );

    aValue.Name = "Compression";
    aValue.Value <<= OUString( pPage3_Quality->GetText() );
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
    aValue.Value <<= pPage3_SldSound->IsChecked() ? sal_True : sal_False;
    aProps.push_back( aValue );

    aValue.Name = "HiddenSlides";
    aValue.Value <<= pPage3_HiddenSlides->IsChecked() ? sal_True : sal_False;
    aProps.push_back( aValue );

    // Page 4
    aValue.Name = "Author";
    aValue.Value <<= OUString( pPage4_Author->GetText() );
    aProps.push_back( aValue );

    aValue.Name = "EMail";
    aValue.Value <<= OUString( pPage4_Email->GetText() );
    aProps.push_back( aValue );

    // try to guess protocol for user's homepage
    INetURLObject aHomeURL( pPage4_WWW->GetText(),
                            INET_PROT_HTTP,     // default proto is HTTP
                            INetURLObject::ENCODE_ALL );

    aValue.Name = "HomepageURL";
    aValue.Value <<= OUString( aHomeURL.GetMainURL( INetURLObject::NO_DECODE ) );
    aProps.push_back( aValue );

    aValue.Name = "UserText";
    aValue.Value <<= OUString( pPage4_Misc->GetText() );
    aProps.push_back( aValue );

    if( m_bImpress )
    {
        aValue.Name = "EnableDownload";
        aValue.Value <<= (sal_Bool)pPage4_Download->IsChecked();
        aProps.push_back( aValue );
    }

    // Page 5
    if( !pPage5_TextOnly->IsChecked() )
    {
        aValue.Name = "UseButtonSet";
        aValue.Value <<= (sal_Int32)(pPage5_Buttons->GetSelectItemId() - 1);
        aProps.push_back( aValue );
    }

    // Page 6
    if( pPage6_User->IsChecked() )
    {
        aValue.Name = "BackColor";
        aValue.Value <<= (sal_Int32)m_aBackColor.GetColor();
        aProps.push_back( aValue );

        aValue.Name = "TextColor";
        aValue.Value <<= (sal_Int32)m_aTextColor.GetColor();
        aProps.push_back( aValue );

        aValue.Name = "LinkColor";
        aValue.Value <<= (sal_Int32)m_aLinkColor.GetColor();
        aProps.push_back( aValue );

        aValue.Name = "VLinkColor";
        aValue.Value <<= (sal_Int32)m_aVLinkColor.GetColor();
        aProps.push_back( aValue );

        aValue.Name = "ALinkColor";
        aValue.Value <<= (sal_Int32)m_aALinkColor.GetColor();
        aProps.push_back( aValue );
    }

    if( pPage6_DocColors->IsChecked() )
    {
        aValue.Name = "IsUseDocumentColors";
        aValue.Value <<= (sal_Bool)sal_True;
        aProps.push_back( aValue );
    }

    rParams.realloc( aProps.size() );
    PropertyValue* pParams = rParams.getArray();

    for( std::vector< PropertyValue >::iterator i = aProps.begin(); i != aProps.end(); ++i )
    {
        *pParams++ = (*i);
    }
}

// Clickhandler for the radiobuttons of the design-selection
IMPL_LINK( SdPublishingDlg, DesignHdl, RadioButton *, pButton )
{
    if(pButton == pPage1_NewDesign)
    {
        pPage1_NewDesign->Check(sal_True); // because of DesignDeleteHdl
        pPage1_OldDesign->Check(sal_False);
        pPage1_Designs->Disable();
        pPage1_DelDesign->Disable();
        m_pDesign = NULL;

        SdPublishingDesign aDefault;
        SetDesign(&aDefault);
    }
    else
    {
        pPage1_NewDesign->Check(sal_False);
        pPage1_Designs->Enable();
        pPage1_DelDesign->Enable();

        if(pPage1_Designs->GetSelectEntryCount() == 0)
            pPage1_Designs->SelectEntryPos(0);

        sal_uInt16 nPos = pPage1_Designs->GetSelectEntryPos();
        m_pDesign = &m_aDesignList[nPos];
        DBG_ASSERT(m_pDesign, "No Design? That's not allowed (CL)");

        if(m_pDesign)
            SetDesign(m_pDesign);
    }

    return 0;
}

// Clickhandler for the choise of one design
IMPL_LINK_NOARG(SdPublishingDlg, DesignSelectHdl)
{
    sal_uInt16 nPos = pPage1_Designs->GetSelectEntryPos();
    m_pDesign = &m_aDesignList[nPos];
    DBG_ASSERT(m_pDesign, "No Design? That's not allowed (CL)");

    if(m_pDesign)
        SetDesign(m_pDesign);

    UpdatePage();

    return 0;
}

// Clickhandler for the delete of one design
IMPL_LINK_NOARG(SdPublishingDlg, DesignDeleteHdl)
{
    sal_uInt16 nPos = pPage1_Designs->GetSelectEntryPos();

    boost::ptr_vector<SdPublishingDesign>::iterator iter = m_aDesignList.begin()+nPos;

    DBG_ASSERT(iter != m_aDesignList.end(), "No Design? That's not allowed (CL)");

    pPage1_Designs->RemoveEntry(nPos);

    if(m_pDesign == &(*iter))
        DesignHdl( pPage1_NewDesign );

    m_aDesignList.erase(iter);

    m_bDesignListDirty = sal_True;

    UpdatePage();

    return 0;
}

// Clickhandler for the other servertypess
IMPL_LINK( SdPublishingDlg, WebServerHdl, RadioButton *, pButton )
{
    sal_Bool bASP = pButton == pPage2_ASP;

    pPage2_ASP->Check( bASP );
    pPage2_PERL->Check( !bASP );
    UpdatePage();

    return 0;
}

// Clickhandler for the Radiobuttons of the graphicformat choice
IMPL_LINK( SdPublishingDlg, GfxFormatHdl, RadioButton *, pButton )
{
    pPage3_Png->Check( pButton == pPage3_Png );
    pPage3_Gif->Check( pButton == pPage3_Gif );
    pPage3_Jpg->Check( pButton == pPage3_Jpg );
    pPage3_Quality->Enable(pButton == pPage3_Jpg);
    return 0;
}

// Clickhandler for the Radiobuttons Standard/Frames
IMPL_LINK_NOARG(SdPublishingDlg, BaseHdl)
{
    UpdatePage();

    return 0;
}

// Clickhandler for the Checkbox of the Title page
IMPL_LINK_NOARG(SdPublishingDlg, ContentHdl)
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
    return 0;
}

// Clickhandler for the Resolution Radiobuttons
IMPL_LINK( SdPublishingDlg, ResolutionHdl, RadioButton *, pButton )
{
    pPage3_Resolution_1->Check(pButton == pPage3_Resolution_1);
    pPage3_Resolution_2->Check(pButton == pPage3_Resolution_2);
    pPage3_Resolution_3->Check(pButton == pPage3_Resolution_3);

    return 0;
}

// Clickhandler for the ValueSet with the bitmap-buttons
IMPL_LINK_NOARG(SdPublishingDlg, ButtonsHdl)
{
    // if one bitmap-button is chosen, then disable TextOnly
    pPage5_TextOnly->Check(sal_False);
    return 0;
}

// Fill the SfxItemSet with the settings of the dialog
IMPL_LINK( SdPublishingDlg, ColorHdl, PushButton *, pButton)
{
    SvColorDialog aDlg(this);

    if(pButton == pPage6_Back)
    {
        aDlg.SetColor( m_aBackColor );
        if(aDlg.Execute() == RET_OK )
            m_aBackColor = aDlg.GetColor();
    }
    else if(pButton == pPage6_Text)
    {
        aDlg.SetColor( m_aTextColor );
        if(aDlg.Execute() == RET_OK )
            m_aTextColor = aDlg.GetColor();
    }
    else if(pButton == pPage6_Link)
    {
        aDlg.SetColor( m_aLinkColor );
        if(aDlg.Execute() == RET_OK )
            m_aLinkColor = aDlg.GetColor();
    }
    else if(pButton == pPage6_VLink)
    {
        aDlg.SetColor( m_aVLinkColor );
        if(aDlg.Execute() == RET_OK )
            m_aVLinkColor = aDlg.GetColor();
    }
    else if(pButton == pPage6_ALink)
    {
        aDlg.SetColor( m_aALinkColor );
        if(aDlg.Execute() == RET_OK )
            m_aALinkColor = aDlg.GetColor();
    }

    pPage6_User->Check(sal_True);
    pPage6_Preview->SetColors( m_aBackColor, m_aTextColor, m_aLinkColor,
                               m_aVLinkColor, m_aALinkColor );
    pPage6_Preview->Invalidate();
    return 0;
}

IMPL_LINK_NOARG(SdPublishingDlg, SlideChgHdl)
{
    UpdatePage();
    return 0;
}

// Clickhandler for the Ok Button
IMPL_LINK_NOARG(SdPublishingDlg, FinishHdl)
{
    //End
    SdPublishingDesign* pDesign = new SdPublishingDesign();
    GetDesign(pDesign);

    sal_Bool bSave = sal_False;

    if(pPage1_OldDesign->IsChecked() && m_pDesign)
    {
        // are there changes?
        if(!(*pDesign == *m_pDesign))
            bSave = sal_True;
    }
    else
    {
        SdPublishingDesign aDefaultDesign;
        if(!(aDefaultDesign == *pDesign))
            bSave = sal_True;
    }

    if(bSave)
    {
        String aName;
        if(m_pDesign)
            aName = m_pDesign->m_aDesignName;

        sal_Bool bRetry;
        do
        {
            bRetry = sal_False;

            SdDesignNameDlg aDlg(this, aName );

            if ( aDlg.Execute() == RET_OK )
            {
                pDesign->m_aDesignName = aDlg.GetDesignName();

                boost::ptr_vector<SdPublishingDesign>::iterator iter;
                for (iter = m_aDesignList.begin(); iter != m_aDesignList.end(); ++iter)
                {
                    if (iter->m_aDesignName == pDesign->m_aDesignName)
                        break;
                }

                if (iter != m_aDesignList.end())
                {
                    ErrorBox aErrorBox(this, WB_YES_NO,
                                       String(SdResId(STR_PUBDLG_SAMENAME)));
                    bRetry = aErrorBox.Execute() == RET_NO;

                    if(!bRetry)
                        m_aDesignList.erase(iter);
                }

                if(!bRetry)
                {
                    m_aDesignList.push_back(pDesign);
                    m_bDesignListDirty = sal_True;
                    pDesign = NULL;
                }
            }
        }
        while(bRetry);
    }

    delete pDesign;

    if(m_bDesignListDirty)
        Save();

    EndDialog(RET_OK);
    return 0;
}

// Refresh the dialogs when changing from pages
void SdPublishingDlg::ChangePage()
{
    int nPage = aAssistentFunc.GetCurrentPage();
    SetHelpId(aPageHelpIds[nPage-1]);

    UpdatePage();

    if( aNextPageButton.IsEnabled() )
        aNextPageButton.GrabFocus();
    else
        aFinishButton.GrabFocus();
}

void SdPublishingDlg::UpdatePage()
{
    aNextPageButton.Enable(!aAssistentFunc.IsLastPage());
    aLastPageButton.Enable(!aAssistentFunc.IsFirstPage());

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
            pPage2_Titel_WebCast->Show();
            pPage2_ASP->Show();
            pPage2_PERL->Show();
            pPage2_URL_txt->Show();
            pPage2_URL->Show();
            pPage2_CGI_txt->Show();
            pPage2_CGI->Show();
            pPage2_Index_txt->Show();
            pPage2_Index->Show();

            sal_Bool bPerl = pPage2_PERL->IsChecked();
            pPage2_Index->Enable(bPerl);
            pPage2_Index_txt->Enable(bPerl);
            pPage2_URL_txt->Enable(bPerl);
            pPage2_URL->Enable(bPerl);
            pPage2_CGI_txt->Enable(bPerl);
            pPage2_CGI->Enable(bPerl);
        }
        else
        {
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
            pPage2_Titel_Kiosk->Show();
            pPage2_ChgDefault->Show();
            pPage2_ChgAuto->Show();
            pPage2_Duration_txt->Show();
            pPage2_Duration->Show();
            pPage2_Endless->Show();
            sal_Bool bAuto = pPage2_ChgAuto->IsChecked();
            pPage2_Duration->Enable(bAuto);
            pPage2_Endless->Enable(bAuto);
        }
        else
        {
            pPage2_Titel_Kiosk->Hide();
            pPage2_ChgDefault->Hide();
            pPage2_ChgAuto->Hide();
            pPage2_Duration->Hide();
            pPage2_Duration_txt->Hide();
            pPage2_Endless->Hide();
        }

        if( pPage2_Standard->IsChecked() || pPage2_Frames->IsChecked() )
        {
            pPage2_Titel_Html->Show();
            pPage2_Content->Show();
            if(m_bImpress)
                pPage2_Notes->Show();
        }
        else
        {
            pPage2_Titel_Html->Hide();
            pPage2_Content->Hide();
            if(m_bImpress)
                pPage2_Notes->Hide();
        }
        break;
    case 3:
        if( pPage2_Kiosk->IsChecked() || pPage2_WebCast->IsChecked() )
            aNextPageButton.Disable();

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
    if( mpButtonSet.get() )
    {
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
        for( int i = 0; i < nButtonCount; ++i )
            aButtonNames.push_back( OUString::createFromAscii( pButtonNames[i] ) );

        int nSetCount = mpButtonSet->getCount();

        int nHeight = 32;
        Image aImage;
        for( int nSet = 0; nSet < nSetCount; ++nSet )
        {
            if( mpButtonSet->getPreview( nSet, aButtonNames, aImage ) )
            {
                pPage5_Buttons->InsertItem( (sal_uInt16)nSet+1, aImage );
                if( nHeight < aImage.GetSizePixel().Height() )
                    nHeight = aImage.GetSizePixel().Height();
            }
        }

        pPage5_Buttons->SetItemHeight( nHeight );
        m_bButtonsDirty = sal_False;
    }
}

// Clickhandler for the Forward Button
IMPL_LINK_NOARG(SdPublishingDlg, NextPageHdl)
{
    aAssistentFunc.NextPage();
    ChangePage();
    return 0;
}

// Sets the Controlls in the dialog to the settings in the design
void SdPublishingDlg::SetDesign( SdPublishingDesign* pDesign )
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

    Time aTime( Time::EMPTY );
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
    pDesign->m_bDownload = m_bImpress?pPage4_Download->IsChecked():sal_False;

    if(pPage5_TextOnly->IsChecked())
        pDesign->m_nButtonThema = -1;
    else
        pDesign->m_nButtonThema = pPage5_Buttons->GetSelectItemId() - 1;

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
    pDesign->m_nSlideDuration = (sal_uInt32)pPage2_Duration->GetTime().GetMSFromTime() / 1000;
    pDesign->m_bEndless = pPage2_Endless->IsChecked();
}

// Clickhandler for the back Button
IMPL_LINK_NOARG(SdPublishingDlg, LastPageHdl)
{
    aAssistentFunc.PreviousPage();
    ChangePage();
    return 0;
}

// Load Designs
sal_Bool SdPublishingDlg::Load()
{
    m_bDesignListDirty = sal_False;

    INetURLObject aURL( SvtPathOptions().GetUserConfigPath() );
    aURL.Append( OUString( "designs.sod" ) );

    // check if file exists, SfxMedium shows an errorbox else
    {
        com::sun::star::uno::Reference < com::sun::star::task::XInteractionHandler > xHandler;
        SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ, xHandler );

        sal_Bool bOk = pIStm && ( pIStm->GetError() == 0);

        if( pIStm )
            delete pIStm;

        if( !bOk )
            return sal_False;
    }

    SfxMedium aMedium( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ | STREAM_NOCREATE );

    SvStream* pStream = aMedium.GetInStream();

    if( !pStream )
        return( sal_False );

    sal_uInt16 aCheck;
    *pStream >> aCheck;

    if(aCheck != nMagic)
        return sal_False;

    SdIOCompat aIO(*pStream, STREAM_READ);

    sal_uInt16 nDesigns;
    *pStream >> nDesigns;

    for( sal_uInt16 nIndex = 0;
         pStream->GetError() == SVSTREAM_OK && nIndex < nDesigns;
         nIndex++ )
    {
        SdPublishingDesign* pDesign = new SdPublishingDesign();
        *pStream >> *pDesign;

        m_aDesignList.push_back(pDesign);
    }

    return( pStream->GetError() == SVSTREAM_OK );
}

// Save Designs
sal_Bool SdPublishingDlg::Save()
{
    INetURLObject aURL( SvtPathOptions().GetUserConfigPath() );
    aURL.Append( OUString( "designs.sod" ) );
    SfxMedium aMedium( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_WRITE | STREAM_TRUNC );
    aMedium.IsRemote();

    SvStream* pStream = aMedium.GetOutStream();

    if( !pStream )
        return( sal_False );

    sal_uInt16 aCheck = nMagic;
    *pStream << aCheck;

    // Destroys the SdIOCompat before the Stream is being destributed
    {
        SdIOCompat aIO(*pStream, STREAM_WRITE, 0);

        sal_uInt16 nDesigns = (sal_uInt16) m_aDesignList.size();
        *pStream << nDesigns;

        for( sal_uInt16 nIndex = 0;
             pStream->GetError() == SVSTREAM_OK && nIndex < nDesigns;
             nIndex++ )
            *pStream << m_aDesignList[nIndex];
    }

    aMedium.Close();
    aMedium.Commit();

    return( aMedium.GetError() == 0 );
}

// SdDesignNameDlg Methods
SdDesignNameDlg::SdDesignNameDlg(Window* pWindow, const String& aName):
                ModalDialog             (pWindow, SdResId( DLG_DESIGNNAME )),
                m_aEdit                 (this, SdResId(EDT_NAME)),
                m_aBtnOK                (this, SdResId(BTN_SAVE)),
                m_aBtnCancel            (this, SdResId(BTN_NOSAVE))
{
    FreeResource();
    m_aEdit.SetModifyHdl(LINK(this, SdDesignNameDlg, ModifyHdl ));
    m_aEdit.SetText(aName);
    m_aBtnOK.Enable(aName.Len() != 0);
}

String SdDesignNameDlg::GetDesignName()
{
    return m_aEdit.GetText();
}

IMPL_LINK_NOARG(SdDesignNameDlg, ModifyHdl)
{
    m_aBtnOK.Enable(!m_aEdit.GetText().isEmpty());

    return 0;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
