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
#include "precompiled_sd.hxx"

#ifdef SD_DLLIMPLEMENTATION
#undef SD_DLLIMPLEMENTATION
#endif
#include <com/sun/star/beans/PropertyValue.hpp>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/lstbox.hxx>
#include <svtools/FilterConfigItem.hxx>
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
#include <tools/list.hxx>
#include <sdiocmpt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/app.hxx>
#include <pres.hxx>
#include <unotools/useroptions.hxx>
#include <unotools/pathoptions.hxx>

#include "sdresid.hxx"
#include "sdattr.hxx"
#include "pubdlg.hrc"
#include "htmlattr.hxx"
#include "htmlex.hxx"
#include "helpids.h"
#include "buttonset.hxx"

using namespace std;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;

using ::rtl::OUString;

extern void InterpolateFixedBitmap( FixedBitmap * pBitmap );

// Kennung fuer die Config Datei mit den Html Einstellungen
const UINT16 nMagic = (UINT16)0x1977;

// Key fuer die soffice.ini
#define KEY_QUALITY     "JPG-EXPORT-QUALITY"

// Die Help Ids der einzelnen Seiten
ULONG aPageHelpIds[NOOFPAGES] =
{
    HID_SD_HTMLEXPORT_PAGE1,
    HID_SD_HTMLEXPORT_PAGE2,
    HID_SD_HTMLEXPORT_PAGE3,
    HID_SD_HTMLEXPORT_PAGE4,
    HID_SD_HTMLEXPORT_PAGE5,
    HID_SD_HTMLEXPORT_PAGE6
};

// *********************************************************************
// Diese Klasse enthaelt alle Einstellungen des Html-Export Autopiloten
// *********************************************************************
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
    BOOL    m_bAutoSlide;
    UINT32  m_nSlideDuration;
    BOOL    m_bEndless;

    // special HTML options
    BOOL    m_bContentPage;
    BOOL    m_bNotes;

    // misc options
    UINT16  m_nResolution;
    String  m_aCompression;
    PublishingFormat m_eFormat;
    BOOL    m_bSlideSound;
    BOOL    m_bHiddenSlides;

    // titel page information
    String  m_aAuthor;
    String  m_aEMail;
    String  m_aWWW;
    String  m_aMisc;
    BOOL    m_bDownload;
    BOOL    m_bCreated;         // not used

    // buttons and colorscheme
    INT16   m_nButtonThema;
    BOOL    m_bUserAttr;
    Color   m_aBackColor;
    Color   m_aTextColor;
    Color   m_aLinkColor;
    Color   m_aVLinkColor;
    Color   m_aALinkColor;
    BOOL    m_bUseAttribs;
    BOOL    m_bUseColor;

    SdPublishingDesign();

    int operator ==(const SdPublishingDesign & rDesign) const;
    friend SvStream& operator >> (SvStream& rIn, SdPublishingDesign& rDesign);
    friend SvStream& operator << (SvStream& rOut, const SdPublishingDesign& rDesign);
};

// =====================================================================
// Default Einstellungen erzeugen
// =====================================================================
SdPublishingDesign::SdPublishingDesign()
{
    m_eMode = PUBLISH_HTML;
    m_bContentPage = TRUE;
    m_bNotes = TRUE;

    m_eFormat = FORMAT_PNG;

    String  aFilterConfigPath( RTL_CONSTASCII_USTRINGPARAM( "Office.Common/Filter/Graphic/Export/JPG" ) );
    FilterConfigItem aFilterConfigItem( aFilterConfigPath );
    sal_Int32 nCompression = aFilterConfigItem.ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( KEY_QUALITY ) ), 75 );
    m_aCompression = UniString::CreateFromInt32( nCompression );
    m_aCompression.Append( sal_Unicode('%') );

    SvtUserOptions aUserOptions;

    m_nResolution   = PUB_LOWRES_WIDTH;
    m_aAuthor       = aUserOptions.GetFirstName();
    if( m_aAuthor.Len() && aUserOptions.GetLastName().getLength() )
        m_aAuthor      += sal_Unicode(' ');
    m_aAuthor      += (String)aUserOptions.GetLastName();
    m_aEMail        = aUserOptions.GetEmail();
    m_bDownload     = FALSE;
    m_nButtonThema  = -1;

    m_bUserAttr     = FALSE;
    m_bUseAttribs   = TRUE;
    m_bUseColor     = TRUE;

    m_aBackColor    = COL_WHITE;
    m_aTextColor    = COL_BLACK;
    m_aLinkColor    = COL_BLUE;
    m_aVLinkColor   = COL_LIGHTBLUE;
    m_aALinkColor   = COL_GRAY;

    m_eScript       = SCRIPT_ASP;

    m_bAutoSlide     = TRUE;
    m_nSlideDuration = 15;
    m_bEndless       = TRUE;

    m_bSlideSound    = TRUE;
    m_bHiddenSlides  = FALSE;
}

// =====================================================================
// Vergleicht die Member ohne den Namen zu beachten
// =====================================================================
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

// =====================================================================
// Dieses Design aus Stream laden
// =====================================================================
SvStream& operator >> (SvStream& rIn, SdPublishingDesign& rDesign)
{
    SdIOCompat aIO(rIn, STREAM_READ);

    UINT16 nTemp16;

    rIn.ReadByteString( rDesign.m_aDesignName, RTL_TEXTENCODING_UTF8 );
    rIn >> nTemp16;
    rDesign.m_eMode = (HtmlPublishMode)nTemp16;
    rIn >> rDesign.m_bContentPage;
    rIn >> rDesign.m_bNotes;
    rIn >> rDesign.m_nResolution;
    rIn.ReadByteString( rDesign.m_aCompression, RTL_TEXTENCODING_UTF8 );
    rIn >> nTemp16;
    rDesign.m_eFormat = (PublishingFormat)nTemp16;
    rIn.ReadByteString( rDesign.m_aAuthor, RTL_TEXTENCODING_UTF8 );
    rIn.ReadByteString( rDesign.m_aEMail, RTL_TEXTENCODING_UTF8 );
    rIn.ReadByteString( rDesign.m_aWWW, RTL_TEXTENCODING_UTF8 );
    rIn.ReadByteString( rDesign.m_aMisc, RTL_TEXTENCODING_UTF8 );
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
    rIn.ReadByteString( rDesign.m_aURL, RTL_TEXTENCODING_UTF8 );
    rIn.ReadByteString( rDesign.m_aCGI, RTL_TEXTENCODING_UTF8 );

    rIn >> rDesign.m_bAutoSlide;
    rIn >> rDesign.m_nSlideDuration;
    rIn >> rDesign.m_bEndless;
    rIn >> rDesign.m_bSlideSound;
    rIn >> rDesign.m_bHiddenSlides;

    return rIn;
}

// =====================================================================
// Dieses Design in Stream speichern
// =====================================================================
SvStream& operator << (SvStream& rOut, const SdPublishingDesign& rDesign)
{
    // Letzter Parameter ist die aktuelle Versionsnummer des Codes
    SdIOCompat aIO(rOut, STREAM_WRITE, 0);

    // Name
    rOut.WriteByteString( rDesign.m_aDesignName, RTL_TEXTENCODING_UTF8 );

    rOut << (UINT16)rDesign.m_eMode;
    rOut << rDesign.m_bContentPage;
    rOut << rDesign.m_bNotes;
    rOut << rDesign.m_nResolution;
    rOut.WriteByteString( rDesign.m_aCompression, RTL_TEXTENCODING_UTF8 );
    rOut << (UINT16)rDesign.m_eFormat;
    rOut.WriteByteString( rDesign.m_aAuthor, RTL_TEXTENCODING_UTF8 );
    rOut.WriteByteString( rDesign.m_aEMail, RTL_TEXTENCODING_UTF8 );
    rOut.WriteByteString( rDesign.m_aWWW, RTL_TEXTENCODING_UTF8 );
    rOut.WriteByteString( rDesign.m_aMisc, RTL_TEXTENCODING_UTF8 );
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

    rOut << (UINT16)rDesign.m_eScript;
    rOut.WriteByteString( rDesign.m_aURL, RTL_TEXTENCODING_UTF8 );
    rOut.WriteByteString( rDesign.m_aCGI, RTL_TEXTENCODING_UTF8 );

    rOut << rDesign.m_bAutoSlide;
    rOut << rDesign.m_nSlideDuration;
    rOut << rDesign.m_bEndless;
    rOut << rDesign.m_bSlideSound;
    rOut << rDesign.m_bHiddenSlides;

    return rOut;
}

// *********************************************************************
// Dialog zur eingabe eines Namens fuer ein Design
// *********************************************************************
class SdDesignNameDlg : public ModalDialog
{
private:
    Edit            m_aEdit;
    OKButton        m_aBtnOK;
    CancelButton    m_aBtnCancel;

public:
    SdDesignNameDlg(Window* pWindow, const String& aName );

    String GetDesignName();
    DECL_LINK( ModifyHdl, Edit* );
};

// *********************************************************************
// SdPublishingDlg Methoden
// *********************************************************************

// =====================================================================
// Konstruktor des Dialogs
// =====================================================================
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
,   m_bButtonsDirty(TRUE)
,   m_bDesignListDirty(FALSE)
,   m_pDesign(NULL)
{
    m_bImpress = eDocType == DOCUMENT_TYPE_IMPRESS;

    CreatePages();
    Load();

    //setzt die Ausgangsseite
    aAssistentFunc.GotoPage(1);
    aLastPageButton.Disable();

    //Buttonbelegung
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
    String  aText( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM("index")) );
    aText += String(SdResId(STR_HTMLEXP_DEFAULT_EXTENSION));
    pPage2_Index->SetText(aText);
    pPage2_CGI->SetText( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "/cgi-bin/" ) ) );

    pPage3_Png->SetClickHdl(LINK(this,SdPublishingDlg, GfxFormatHdl));
    pPage3_Gif->SetClickHdl(LINK(this,SdPublishingDlg, GfxFormatHdl));
    pPage3_Jpg->SetClickHdl(LINK(this,SdPublishingDlg, GfxFormatHdl));
    pPage3_Quality->Enable(FALSE);

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

    pPage3_Quality->InsertEntry( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "25%" ) ) );
    pPage3_Quality->InsertEntry( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "50%" ) ) );
    pPage3_Quality->InsertEntry( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "75%" ) ) );
    pPage3_Quality->InsertEntry( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "100%" ) ) );

    pPage5_Buttons->SetColCount( 1 );
    pPage5_Buttons->SetLineCount( 4 );
    pPage5_Buttons->SetExtraSpacing( 1 );

    for( UINT16 nIndex = 0; nIndex < m_pDesignList->Count(); nIndex++ )
    {
        SdPublishingDesign *pDesign = (SdPublishingDesign*)
                                        m_pDesignList->GetObject(nIndex);

        pPage1_Designs->InsertEntry(pDesign->m_aDesignName);
    }

    pPage6_Preview->SetBorderStyle(WINDOW_BORDER_MONO);

    SetDefaults();

    SetHelpId(aPageHelpIds[0]);

    aNextPageButton.GrabFocus();
}

// =====================================================================
// Destruktor
// =====================================================================
SdPublishingDlg::~SdPublishingDlg()
{
    if( m_pDesignList )
    {
        for( UINT16 nIndex = 0; nIndex < m_pDesignList->Count(); nIndex++ )
            delete (SdPublishingDesign*)m_pDesignList->GetObject(nIndex);
    }

    delete m_pDesignList;
    RemovePages();
}

// =====================================================================
// Dialog Controls erzeugen und in die Seiten des Assistenten einbinden
// =====================================================================
void SdPublishingDlg::CreatePages()
{
    // Page 1
    aAssistentFunc.InsertControl(1,
        pPage1_Bmp = new FixedBitmap(this,SdResId(PAGE1_BMP)));
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
        pPage2_Bmp = new FixedBitmap(this,SdResId(PAGE2_BMP)));
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
        pPage3_Bmp = new FixedBitmap(this,SdResId(PAGE3_BMP)));
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

    // Seite 4
    aAssistentFunc.InsertControl(4,
        pPage4_Bmp = new FixedBitmap(this,SdResId(PAGE4_BMP)));
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

    // Seite 5
    aAssistentFunc.InsertControl(5,
        pPage5_Bmp = new FixedBitmap(this,SdResId(PAGE5_BMP)));
    aAssistentFunc.InsertControl(5,
        pPage5_Titel = new FixedLine(this,SdResId(PAGE5_TITEL)));
    aAssistentFunc.InsertControl(5,
        pPage5_TextOnly = new CheckBox(this, SdResId(PAGE5_TEXTONLY)));
    aAssistentFunc.InsertControl(5,
        pPage5_Buttons = new ValueSet(this,SdResId(PAGE5_BUTTONS)));

    // Seite 6
    aAssistentFunc.InsertControl(6,
        pPage6_Bmp = new FixedBitmap(this,SdResId(PAGE6_BMP)));
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

    InterpolateFixedBitmap(pPage1_Bmp);

    InterpolateFixedBitmap(pPage2_Bmp);
    InterpolateFixedBitmap(pPage2_Standard_FB);
    InterpolateFixedBitmap(pPage2_Frames_FB);
    InterpolateFixedBitmap(pPage2_Kiosk_FB);
    InterpolateFixedBitmap(pPage2_WebCast_FB);

    InterpolateFixedBitmap(pPage3_Bmp);
    InterpolateFixedBitmap(pPage4_Bmp);

    InterpolateFixedBitmap(pPage5_Bmp);
    InterpolateFixedBitmap(pPage6_Bmp);
}

// =====================================================================
// Dialog Controls wieder entfernen
// =====================================================================
void SdPublishingDlg::RemovePages()
{
    delete pPage1_Bmp;
    delete pPage1_Titel;
    delete pPage1_NewDesign;
    delete pPage1_OldDesign;
    delete pPage1_Designs;
    delete pPage1_DelDesign;
    delete pPage1_Desc;

    delete pPage2_Bmp;
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

    delete pPage3_Bmp;
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

    delete pPage4_Bmp;
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

    delete pPage5_Bmp;
    delete pPage5_Titel;
    delete pPage5_TextOnly;
    delete pPage5_Buttons;

    delete pPage6_Bmp;
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

// =====================================================================
// Dialog mit defaultwerten initialisieren
// =====================================================================
void SdPublishingDlg::SetDefaults()
{
    SdPublishingDesign aDefault;
    SetDesign(&aDefault);

    pPage1_NewDesign->Check(TRUE);
    pPage1_OldDesign->Check(FALSE);
    UpdatePage();
}

// =====================================================================
// Das SfxItemSet mit den Einstellungen des Dialogs fuettern
// =====================================================================
void SdPublishingDlg::GetParameterSequence( Sequence< PropertyValue >& rParams )
{
    std::vector< PropertyValue > aProps;

    PropertyValue aValue;


    // Page 2
    aValue.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "PublishMode" ) );
    aValue.Value <<= (sal_Int32)(pPage2_Standard->IsChecked()?PUBLISH_HTML:
                                          pPage2_Frames->IsChecked()?PUBLISH_FRAMES:
                                          pPage2_Kiosk->IsChecked()?PUBLISH_KIOSK:PUBLISH_WEBCAST);
    aProps.push_back( aValue );

    aValue.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "IsExportContentsPage" ) );
    aValue.Value <<= (sal_Bool)pPage2_Content->IsChecked();
    aProps.push_back( aValue );

    if(m_bImpress)
    {
        aValue.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "IsExportNotes" ) );
        aValue.Value <<= (sal_Bool)pPage2_Notes->IsChecked();
        aProps.push_back( aValue );
    }

    if( pPage2_WebCast->IsChecked() )
    {
        aValue.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "WebCastScriptLanguage" ) );
        if( pPage2_ASP->IsChecked() )
            aValue.Value <<= OUString( RTL_CONSTASCII_USTRINGPARAM( "asp" ) );
        else
            aValue.Value <<= OUString( RTL_CONSTASCII_USTRINGPARAM( "perl" ) );
        aProps.push_back( aValue );

        aValue.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "WebCastCGIURL" ) );
        aValue.Value <<= OUString( pPage2_CGI->GetText() );
        aProps.push_back( aValue );

        aValue.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "WebCastTargetURL" ) );
        aValue.Value <<= OUString( pPage2_URL->GetText() );
        aProps.push_back( aValue );
    }
    aValue.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "IndexURL" ) );
    aValue.Value <<= OUString( pPage2_Index->GetText() );
    aProps.push_back( aValue );


    if( pPage2_Kiosk->IsChecked() && pPage2_ChgAuto->IsChecked() )
    {
        aValue.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "KioskSlideDuration" ) );
        aValue.Value <<= (sal_uInt32)pPage2_Duration->GetTime().GetMSFromTime() / 1000;
        aProps.push_back( aValue );

        aValue.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "KioskEndless" ) );
        aValue.Value <<= (sal_Bool)pPage2_Endless->IsChecked();
        aProps.push_back( aValue );
    }

    // Page 3

    aValue.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "Width" ) );
    sal_Int32 nTmpWidth = 512;
    if( pPage3_Resolution_2->IsChecked() )
        nTmpWidth = 640;
    else if( pPage3_Resolution_3->IsChecked() )
        nTmpWidth = 800;

    aValue.Value <<= nTmpWidth;
    aProps.push_back( aValue );

    aValue.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "Compression" ) );
    aValue.Value <<= OUString( pPage3_Quality->GetText() );
    aProps.push_back( aValue );

    aValue.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "Format" ) );
    sal_Int32 nFormat;
    if( pPage3_Png->IsChecked() )
        nFormat = static_cast<sal_Int32>(FORMAT_PNG);
    else if( pPage3_Gif->IsChecked() )
        nFormat = static_cast<sal_Int32>(FORMAT_GIF);
    else
        nFormat = static_cast<sal_Int32>(FORMAT_JPG);
    aValue.Value <<= nFormat;
    aProps.push_back( aValue );

    aValue.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "SlideSound" ) );
    aValue.Value <<= pPage3_SldSound->IsChecked() ? sal_True : sal_False;
    aProps.push_back( aValue );

    aValue.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "HiddenSlides" ) );
    aValue.Value <<= pPage3_HiddenSlides->IsChecked() ? sal_True : sal_False;
    aProps.push_back( aValue );

    // Page 4
    aValue.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "Author" ) );
    aValue.Value <<= OUString( pPage4_Author->GetText() );
    aProps.push_back( aValue );

    aValue.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "EMail" ) );
    aValue.Value <<= OUString( pPage4_Email->GetText() );
    aProps.push_back( aValue );

    // try to guess protocol for user's homepage
    INetURLObject aHomeURL( pPage4_WWW->GetText(),
                            INET_PROT_HTTP,     // default proto is HTTP
                            INetURLObject::ENCODE_ALL );

    aValue.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "HomepageURL" ) );
    aValue.Value <<= OUString( aHomeURL.GetMainURL( INetURLObject::NO_DECODE ) );
    aProps.push_back( aValue );

    aValue.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "UserText" ) );
    aValue.Value <<= OUString( pPage4_Misc->GetText() );
    aProps.push_back( aValue );

    if( m_bImpress )
    {
        aValue.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "EnableDownload" ) );
        aValue.Value <<= (sal_Bool)pPage4_Download->IsChecked();
        aProps.push_back( aValue );
    }

    // Page 5
    if( !pPage5_TextOnly->IsChecked() )
    {
        aValue.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "UseButtonSet" ) );
        aValue.Value <<= (sal_Int32)(pPage5_Buttons->GetSelectItemId() - 1);
        aProps.push_back( aValue );
    }

    // Page 6
    if( pPage6_User->IsChecked() )
    {
        aValue.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "BackColor" ) );
        aValue.Value <<= (sal_Int32)m_aBackColor.GetColor();
        aProps.push_back( aValue );

        aValue.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "TextColor" ) );
        aValue.Value <<= (sal_Int32)m_aTextColor.GetColor();
        aProps.push_back( aValue );

        aValue.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "LinkColor" ) );
        aValue.Value <<= (sal_Int32)m_aLinkColor.GetColor();
        aProps.push_back( aValue );

        aValue.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "VLinkColor" ) );
        aValue.Value <<= (sal_Int32)m_aVLinkColor.GetColor();
        aProps.push_back( aValue );

        aValue.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "ALinkColor" ) );
        aValue.Value <<= (sal_Int32)m_aALinkColor.GetColor();
        aProps.push_back( aValue );
    }

    if( pPage6_DocColors->IsChecked() )
    {
        aValue.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "IsUseDocumentColors" ) );
        aValue.Value <<= (sal_Bool)sal_True;
        aProps.push_back( aValue );
    }

    // Seite 6


    rParams.realloc( aProps.size() );
    PropertyValue* pParams = rParams.getArray();

    for( std::vector< PropertyValue >::iterator i = aProps.begin(); i != aProps.end(); ++i )
    {
        *pParams++ = (*i);
    }
}

// =====================================================================
// Clickhandler fuer die Radiobuttons zur Designauswahl
// =====================================================================
IMPL_LINK( SdPublishingDlg, DesignHdl, RadioButton *, pButton )
{
    if(pButton == pPage1_NewDesign)
    {
        pPage1_NewDesign->Check(TRUE); // wegen DesignDeleteHdl
        pPage1_OldDesign->Check(FALSE);
        pPage1_Designs->Disable();
        pPage1_DelDesign->Disable();
        m_pDesign = NULL;

        SdPublishingDesign aDefault;
        SetDesign(&aDefault);
    }
    else
    {
        pPage1_NewDesign->Check(FALSE);
        pPage1_Designs->Enable();
        pPage1_DelDesign->Enable();

        if(pPage1_Designs->GetSelectEntryCount() == 0)
            pPage1_Designs->SelectEntryPos(0);

        USHORT nPos = pPage1_Designs->GetSelectEntryPos();
        m_pDesign = (SdPublishingDesign*)m_pDesignList->GetObject(nPos);
        DBG_ASSERT(m_pDesign, "Kein Design? Das darf nicht sein! (CL)");

        if(m_pDesign)
            SetDesign(m_pDesign);
    }

    return 0;
}

// =====================================================================
// Clickhandler fuer die auswahl eines Designs
// =====================================================================
IMPL_LINK( SdPublishingDlg, DesignSelectHdl, ListBox *, EMPTYARG )
{
    USHORT nPos = pPage1_Designs->GetSelectEntryPos();
    m_pDesign = (SdPublishingDesign*)m_pDesignList->GetObject(nPos);
    DBG_ASSERT(m_pDesign, "Kein Design? Das darf nicht sein! (CL)");

    if(m_pDesign)
        SetDesign(m_pDesign);

    UpdatePage();

    return 0;
}

// =====================================================================
// Clickhandler fuer das loeschen eines Designs
// =====================================================================
IMPL_LINK( SdPublishingDlg, DesignDeleteHdl, PushButton *, EMPTYARG )
{
    USHORT nPos = pPage1_Designs->GetSelectEntryPos();
    SdPublishingDesign* pDesign = (SdPublishingDesign*)
                                    m_pDesignList->GetObject(nPos);
    DBG_ASSERT(pDesign, "Kein Design? Das darf nicht sein! (CL)");

    if(pDesign)
    {
        m_pDesignList->Remove(pDesign);
        pPage1_Designs->RemoveEntry(nPos);
    }

    if(m_pDesign == pDesign)
        DesignHdl( pPage1_NewDesign );

    delete pDesign;

    m_bDesignListDirty = TRUE;

    UpdatePage();

    return 0;
}

// =====================================================================
// Clickhandler fuer das ändern des Servertyps
// =====================================================================
IMPL_LINK( SdPublishingDlg, WebServerHdl, RadioButton *, pButton )
{
    BOOL bASP = pButton == pPage2_ASP;

    pPage2_ASP->Check( bASP );
    pPage2_PERL->Check( !bASP );
    UpdatePage();

    return 0;
}

// =====================================================================
// Clickhandler fuer die Radiobuttons der Auswahl des Grafikformates
// =====================================================================
IMPL_LINK( SdPublishingDlg, GfxFormatHdl, RadioButton *, pButton )
{
    pPage3_Png->Check( pButton == pPage3_Png );
    pPage3_Gif->Check( pButton == pPage3_Gif );
    pPage3_Jpg->Check( pButton == pPage3_Jpg );
    pPage3_Quality->Enable(pButton == pPage3_Jpg);
    return 0;
}

// =====================================================================
// Clickhandler fuer die Radiobuttons Stanrard/Frames
// =====================================================================
IMPL_LINK( SdPublishingDlg, BaseHdl, RadioButton *, EMPTYARG )
{
    UpdatePage();

    return 0;
}

// =====================================================================
// Clickhandler fuer die CheckBox der Titelseite
// =====================================================================
IMPL_LINK( SdPublishingDlg, ContentHdl, RadioButton *, EMPTYARG )
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

// =====================================================================
// Clickhandler fuer die Radiobuttons Aufloesung
// =====================================================================
IMPL_LINK( SdPublishingDlg, ResolutionHdl, RadioButton *, pButton )
{
    pPage3_Resolution_1->Check(pButton == pPage3_Resolution_1);
    pPage3_Resolution_2->Check(pButton == pPage3_Resolution_2);
    pPage3_Resolution_3->Check(pButton == pPage3_Resolution_3);

    return 0;
}

// =====================================================================
// Clickhandler fuer das ValueSet mit den Bitmap Schaltflaechen
// =====================================================================
IMPL_LINK( SdPublishingDlg, ButtonsHdl, ValueSet *, EMPTYARG )
{
    // wird eine Bitmap Schaltflaeche gewaehlt, TexOnly ausschalten
    pPage5_TextOnly->Check(FALSE);
    return 0;
}

// =====================================================================
// Das SfxItemSet mit den Einstellungen des Dialogs fuettern
// =====================================================================
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

    pPage6_User->Check(TRUE);
    pPage6_Preview->SetColors( m_aBackColor, m_aTextColor, m_aLinkColor,
                               m_aVLinkColor, m_aALinkColor );
    pPage6_Preview->Invalidate();
    return 0;
}

IMPL_LINK( SdPublishingDlg, SlideChgHdl, RadioButton*, EMPTYARG )
{
    UpdatePage();
    return 0;
}

// =====================================================================
// Clickhandler fuer den Ok Button
// =====================================================================
IMPL_LINK( SdPublishingDlg, FinishHdl, OKButton *, EMPTYARG )
{
    //Ende
    SdPublishingDesign* pDesign = new SdPublishingDesign();
    GetDesign(pDesign);

    BOOL bSave = FALSE;

    if(pPage1_OldDesign->IsChecked() && m_pDesign)
    {
        // aenderungen??
        if(!(*pDesign == *m_pDesign))
            bSave = TRUE;
    }
    else
    {
        SdPublishingDesign aDefaultDesign;
        if(!(aDefaultDesign == *pDesign))
            bSave = TRUE;
    }

    if(bSave)
    {
        String aName;
        if(m_pDesign)
            aName = m_pDesign->m_aDesignName;

        BOOL bRetry;
        do
        {
            bRetry = FALSE;

            SdDesignNameDlg aDlg(this, aName );

            if ( aDlg.Execute() == RET_OK )
            {
                pDesign->m_aDesignName = aDlg.GetDesignName();

                SdPublishingDesign* pSameNameDes = NULL;
                UINT16 nIndex;
                for( nIndex = 0; nIndex < m_pDesignList->Count(); nIndex++ )
                {
                    pSameNameDes = (SdPublishingDesign*)
                                    m_pDesignList->GetObject(nIndex);
                    if(pSameNameDes->m_aDesignName == pDesign->m_aDesignName)
                        break;
                }

                if(nIndex < m_pDesignList->Count())
                {
                    ErrorBox aErrorBox(this, WB_YES_NO,
                                       String(SdResId(STR_PUBDLG_SAMENAME)));
                    bRetry = aErrorBox.Execute() == RET_NO;

                    if(!bRetry)
                    {
                        m_pDesignList->Remove(pSameNameDes);
                        delete pSameNameDes;
                    }
                }

                if(!bRetry)
                {
                    m_pDesignList->Insert(pDesign);
                    m_bDesignListDirty = TRUE;
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

// =====================================================================
// Refresh des Dialogs beim wechsel der Seite
// =====================================================================
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

        if(m_pDesignList && m_pDesignList->Count() == 0)
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

            BOOL bPerl = pPage2_PERL->IsChecked();
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
            BOOL bAuto = pPage2_ChgAuto->IsChecked();
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

        std::vector< rtl::OUString > aButtonNames;
        for( int i = 0; i < nButtonCount; ++i )
            aButtonNames.push_back( rtl::OUString::createFromAscii( pButtonNames[i] ) );

        int nSetCount = mpButtonSet->getCount();

        int nHeight = 32;
        Image aImage;
        for( int nSet = 0; nSet < nSetCount; ++nSet )
        {
            if( mpButtonSet->getPreview( nSet, aButtonNames, aImage ) )
            {
                pPage5_Buttons->InsertItem( (USHORT)nSet+1, aImage );
                if( nHeight < aImage.GetSizePixel().Height() )
                    nHeight = aImage.GetSizePixel().Height();
            }
        }

        pPage5_Buttons->SetItemHeight( nHeight );
        m_bButtonsDirty = FALSE;
    }
}

// =====================================================================
// Clickhandler fuer den Weiter Button
// =====================================================================
IMPL_LINK( SdPublishingDlg, NextPageHdl, PushButton *, EMPTYARG )
{
    aAssistentFunc.NextPage();
    ChangePage();
    return 0;
}

// =====================================================================
// Setzt die Controlls im Dialog gemaess den Einstellungen im Design
// =====================================================================
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

    Time aTime;
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

// =====================================================================
// Uebertraegt den Status der Dialog Controlls in das Design
// =====================================================================
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
    pDesign->m_bDownload = m_bImpress?pPage4_Download->IsChecked():FALSE;

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
    pDesign->m_nSlideDuration = (UINT32)pPage2_Duration->GetTime().GetMSFromTime() / 1000;
    pDesign->m_bEndless = pPage2_Endless->IsChecked();
}

// =====================================================================
// Clickhandler fuer den Zurueck Button
// =====================================================================
IMPL_LINK( SdPublishingDlg, LastPageHdl, PushButton *, EMPTYARG )
{
    aAssistentFunc.PreviousPage();
    ChangePage();
    return 0;
}

// =====================================================================
// Designs laden
// =====================================================================
BOOL SdPublishingDlg::Load()
{
    m_bDesignListDirty = FALSE;

    m_pDesignList = new List();

    INetURLObject aURL( SvtPathOptions().GetUserConfigPath() );
    aURL.Append( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "designs.sod" ) ) );

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

    SfxMedium aMedium( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ | STREAM_NOCREATE, TRUE );

    SvStream* pStream = aMedium.GetInStream();

    if( !pStream )
        return( FALSE );

    UINT16 aCheck;
    *pStream >> aCheck;

    if(aCheck != nMagic)
        return FALSE;

    SdIOCompat aIO(*pStream, STREAM_READ);

    UINT16 nDesigns;
    *pStream >> nDesigns;

    for( UINT16 nIndex = 0;
         pStream->GetError() == SVSTREAM_OK && nIndex < nDesigns;
         nIndex++ )
    {
        SdPublishingDesign* pDesign = new SdPublishingDesign();
        *pStream >> *pDesign;

        m_pDesignList->Insert(pDesign);
    }

    return( pStream->GetError() == SVSTREAM_OK );
}

// =====================================================================
// Designs speichern
// =====================================================================
BOOL SdPublishingDlg::Save()
{
    INetURLObject aURL( SvtPathOptions().GetUserConfigPath() );
    aURL.Append( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "designs.sod" ) ) );
    SfxMedium aMedium( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_WRITE | STREAM_TRUNC, FALSE );
    aMedium.IsRemote();

    SvStream* pStream = aMedium.GetOutStream();

    if( !pStream )
        return( FALSE );

    UINT16 aCheck = nMagic;
    *pStream << aCheck;

    // damit SdIOCompat vor dem Stream destruiert wird
    {
        SdIOCompat aIO(*pStream, STREAM_WRITE, 0);

        UINT16 nDesigns = (UINT16) m_pDesignList->Count();
        *pStream << nDesigns;

        for( UINT16 nIndex = 0;
             pStream->GetError() == SVSTREAM_OK && nIndex < nDesigns;
             nIndex++ )
        {
            SdPublishingDesign* pDesign = (SdPublishingDesign*)
                                            m_pDesignList->GetObject(nIndex);
            *pStream << *pDesign;
        }
    }

    aMedium.Close();
    aMedium.Commit();

    return( aMedium.GetError() == 0 );
}

// *********************************************************************
// SdDesignNameDlg Methoden
// *********************************************************************
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

IMPL_LINK( SdDesignNameDlg, ModifyHdl, Edit*, EMPTYARG )
{
    m_aBtnOK.Enable(m_aEdit.GetText().Len() != 0);

    return 0;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
