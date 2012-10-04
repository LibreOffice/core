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


#include "htmlex.hxx"
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>

#include <rtl/uri.hxx>
#include <comphelper/processfactory.hxx>
#include <osl/file.hxx>
#include <tools/fsys.hxx>
#include <unotools/pathoptions.hxx>
#include <svtools/FilterConfigItem.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/localfilehelper.hxx>
#include <com/sun/star/frame/XStorable.hpp>
#include <sfx2/progress.hxx>
#include <vcl/wrkwin.hxx>
#include <svl/aeitem.hxx>
#include <svx/svditer.hxx>
#include <svtools/imaprect.hxx>
#include <svtools/imapcirc.hxx>
#include <svtools/imappoly.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/app.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/editobj.hxx>
#include <svx/svdopath.hxx>
#include <svx/xoutbmp.hxx>
#include <svtools/htmlout.hxx>
#include <sfx2/docfile.hxx>
#include <vcl/cvtgrf.hxx>
#include <svtools/colorcfg.hxx>
#include <svtools/filter.hxx>
#include <editeng/colritem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/crsditem.hxx>
#include <editeng/flditem.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/fcontnr.hxx>
#include <svl/style.hxx>
#include <editeng/frmdiritem.hxx>
#include <svx/svdoutl.hxx>
#include <tools/urlobj.hxx>               // INetURLObject
#include <vcl/bmpacc.hxx>
#include <svtools/sfxecode.hxx>
#include <com/sun/star/beans/PropertyState.hpp>
#include <tools/resmgr.hxx>
#include "comphelper/anytostring.hxx"
#include "cppuhelper/exc_hlp.hxx"

#include "drawdoc.hxx"
#include "Outliner.hxx"
#include "sdpage.hxx"
#include "sdattr.hxx"
#include "glob.hrc"
#include "anminfo.hxx"
#include "imapinfo.hxx"
#include "sdresid.hxx"
#include "buttonset.hxx"
#include <basegfx/polygon/b2dpolygon.hxx>

using ::rtl::OUString;
using ::rtl::OString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::document;

#define KEY_QUALITY     "JPG-EXPORT-QUALITY"

// Parameter aus Itemset abfragen

#define RESTOHTML( res ) StringToHTMLString(String(SdResId(res)))
#define S2H( str ) StringToHTMLString( str )

// bei Aenderungen auch NUM_BUTTONS in pubdlg.hxx aendern!!
const char *pButtonNames[NUM_BUTTONS] =
{
    "first-inactive.png",
    "first.png",
    "left-inactive.png",
    "left.png",
    "right-inactive.png",
    "right.png",
    "last-inactive.png",
    "last.png",
    "home.png",
    "text.png",
    "expand.png",
    "collapse.png",
};

#define BTN_FIRST_0 0
#define BTN_FIRST_1 1
#define BTN_PREV_0  2
#define BTN_PREV_1  3
#define BTN_NEXT_0  4
#define BTN_NEXT_1  5
#define BTN_LAST_0  6
#define BTN_LAST_1  7
#define BTN_INDEX   8
#define BTN_TEXT    9
#define BTN_MORE    10
#define BTN_LESS    11

// *********************************************************************
// Hilfsklasse fuer das simple erzeugen von Dateien lokal/remote
// *********************************************************************
class EasyFile
{
private:
    SvStream*   pOStm;
    SfxMedium*  pMedium;
    bool        bOpen;

public:

    EasyFile();
    ~EasyFile();

    sal_uLong createStream( const String& rUrl, SvStream*& rpStr );
    sal_uLong createFileName(  const String& rUrl, String& rFileName );
    sal_uLong close();
};

// *********************************************************************
// Hilfsklasse fuer das einbinden von Textattributen in die Html-Ausgabe
// *********************************************************************
class HtmlState
{
private:
    bool mbColor;
    bool mbWeight;
    bool mbItalic;
    bool mbUnderline;
    bool mbStrike;
    bool mbLink;
    Color maColor;
    Color maDefColor;
    String maLink;
    String maTarget;

public:
    HtmlState( Color aDefColor );

    String SetWeight( bool bWeight );
    String SetItalic( bool bItalic );
    String SetUnderline( bool bUnderline );
    String SetColor( Color aColor );
    String SetStrikeout( bool bStrike );
    String SetLink( const String& aLink, const String& aTarget );
    String Flush();
};

// =====================================================================
// alle noch offennen Tags schliessen
// =====================================================================
String HtmlState::Flush()
{
    String aStr, aEmpty;

    aStr += SetWeight(false);
    aStr += SetItalic(false);
    aStr += SetUnderline(false);
    aStr += SetStrikeout(false);
    aStr += SetColor(maDefColor);
    aStr += SetLink(aEmpty,aEmpty);

    return aStr;
}

// =====================================================================
// c'tor mit Defaultfarbe fuer die Seite
// =====================================================================
HtmlState::HtmlState( Color aDefColor )
{
    mbColor = false;
    mbWeight = false;
    mbItalic = false;
    mbUnderline = false;
    mbLink = false;
    mbStrike = false;
    maDefColor = aDefColor;
}

// =====================================================================
// aktiviert/deaktiviert Fettdruck
// =====================================================================
String HtmlState::SetWeight( bool bWeight )
{
    String aStr;

    if(bWeight && !mbWeight)
        aStr.AppendAscii( "<b>" );
    else if(!bWeight && mbWeight)
        aStr.AppendAscii( "</b>" );

    mbWeight = bWeight;
    return aStr;
}

// =====================================================================
// aktiviert/deaktiviert Italic
// =====================================================================
String HtmlState::SetItalic( bool bItalic )
{
    String aStr;

    if(bItalic && !mbItalic)
        aStr.AppendAscii( "<i>" );
    else if(!bItalic && mbItalic)
        aStr.AppendAscii( "</i>" );

    mbItalic = bItalic;
    return aStr;
}

// =====================================================================
// aktiviert/deaktiviert Unterstrichen
// =====================================================================
String HtmlState::SetUnderline( bool bUnderline )
{
    String aStr;

    if(bUnderline && !mbUnderline)
        aStr.AppendAscii( "<u>" );
    else if(!bUnderline && mbUnderline)
        aStr.AppendAscii( "</u>" );

    mbUnderline = bUnderline;
    return aStr;
}

// =====================================================================
// aktiviert/deaktiviert Durchstreichen
// =====================================================================
String HtmlState::SetStrikeout( bool bStrike )
{
    String aStr;

    if(bStrike && !mbStrike)
        aStr.AppendAscii( "<strike>" );
    else if(!bStrike && mbStrike)
        aStr.AppendAscii( "</strike>" );

    mbStrike = bStrike;
    return aStr;
}

// =====================================================================
// Setzt die angegebenne Textfarbe
// =====================================================================
String HtmlState::SetColor( Color aColor )
{
    String aStr;

    if(mbColor && aColor == maColor)
        return aStr;

    if(mbColor)
    {
        aStr.AppendAscii( "</font>" );
        mbColor = false;
    }

    if(aColor != maDefColor)
    {
        maColor = aColor;

        aStr.AppendAscii( "<font color=\"" );
        aStr += HtmlExport::ColorToHTMLString(aColor);
        aStr.AppendAscii( "\">" );

        mbColor = true;
    }

    return aStr;
}

// =====================================================================
// aktiviert/deaktiviert einen Hyperlink
// =====================================================================
String HtmlState::SetLink( const String& aLink, const String& aTarget )
{
    String aStr;

    if(mbLink&&maLink == aLink&&maTarget==aTarget)
        return aStr;

    if(mbLink)
    {
        aStr.AppendAscii( "</a>" );
        mbLink = false;
    }

    if(aLink.Len())
    {
        aStr.AppendAscii( "<a href=\"" );
        aStr += HtmlExport::StringToURL(aLink);
        if(aTarget.Len())
        {
            aStr.AppendAscii( "\" target=\"" );
            aStr += aTarget;
        }
        aStr.AppendAscii( "\">" );
        mbLink = true;
        maLink = aLink;
        maTarget = aTarget;
    }

    return aStr;
}

// *********************************************************************
// class HtmlExport Methoden
// *********************************************************************

static String getParagraphStyle( SdrOutliner* pOutliner, sal_uInt16 nPara )
{
    SfxItemSet aParaSet( pOutliner->GetParaAttribs( nPara ) );

    String sStyle( RTL_CONSTASCII_USTRINGPARAM("direction:") );
    if( static_cast<const SvxFrameDirectionItem*>(aParaSet.GetItem( EE_PARA_WRITINGDIR ))->GetValue() == FRMDIR_HORI_RIGHT_TOP )
    {
        sStyle += String( RTL_CONSTASCII_USTRINGPARAM("rtl;") );
    }
    else
    {
         sStyle += String( RTL_CONSTASCII_USTRINGPARAM("ltr;") );
    }
    return sStyle;
}

// =====================================================================
// Konstruktor fuer die Html Export Hilfsklasse
// =====================================================================
HtmlExport::HtmlExport(
    OUString aPath,
    const Sequence< PropertyValue >& rParams,
    SdDrawDocument* pExpDoc,
    ::sd::DrawDocShell* pDocShell )
    :   maPath( aPath ),
        mpDoc(pExpDoc),
        mpDocSh( pDocShell ),
        meEC(NULL),
        meMode( PUBLISH_HTML ),
        mbContentsPage(false),
        mnButtonThema(-1),
        mnWidthPixel( PUB_MEDRES_WIDTH ),
        meFormat( FORMAT_JPG ),
        mbNotes(false),
        mnCompression( -1 ),
        mbDownload( false ),
        mbSlideSound(true),
        mbHiddenSlides(true),
        mbUserAttr(false),
        mbDocColors(false),
        maHTMLExtension(SdResId(STR_HTMLEXP_DEFAULT_EXTENSION)),
        mpHTMLFiles(NULL),
        mpImageFiles(NULL),
        mpThumbnailFiles(NULL),
        mpPageNames(NULL),
        mpTextFiles(NULL),
        maIndexUrl("index"),
        meScript( SCRIPT_ASP ),
        maHTMLHeader(
            "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\"\r\n"
            "     \"http://www.w3.org/TR/html4/transitional.dtd\">\r\n"
            "<html>\r\n<head>\r\n" ),
        mpButtonSet( new ButtonSet() )
{
    bool bChange = mpDoc->IsChanged();

    maIndexUrl += maHTMLExtension;

    InitExportParameters( rParams );

    switch( meMode )
    {
    case PUBLISH_HTML:
    case PUBLISH_FRAMES:
        ExportHtml();
        break;
    case PUBLISH_WEBCAST:
        ExportWebCast();
        break;
    case PUBLISH_KIOSK:
        ExportKiosk();
        break;
    }

    mpDoc->SetChanged(bChange);
}

HtmlExport::~HtmlExport()
{
    // ------------------------------------------------------------------
    // Listen loeschen
    // ------------------------------------------------------------------
    if(mpImageFiles && mpHTMLFiles && mpThumbnailFiles && mpPageNames && mpTextFiles )
    {
        for ( sal_uInt16 nSdPage = 0; nSdPage < mnSdPageCount; nSdPage++)
        {
            delete mpImageFiles[nSdPage];
            delete mpHTMLFiles[nSdPage];
            delete mpThumbnailFiles[nSdPage];
            delete mpPageNames[nSdPage];
            delete mpTextFiles[nSdPage];
        }
    }

    delete[] mpImageFiles;
    delete[] mpHTMLFiles;
    delete[] mpThumbnailFiles;
    delete[] mpPageNames;
    delete[] mpTextFiles;
}

/** get common export parameters from item set */
void HtmlExport::InitExportParameters( const Sequence< PropertyValue >& rParams )
{
    mbImpress = mpDoc && mpDoc->GetDocumentType() == DOCUMENT_TYPE_IMPRESS;

    sal_Int32 nArgs = rParams.getLength();
    const PropertyValue* pParams = rParams.getConstArray();
    OUString aStr;
    while( nArgs-- )
    {
        if ( pParams->Name == "PublishMode" )
        {
            sal_Int32 temp = 0;
            pParams->Value >>= temp;
            meMode = (HtmlPublishMode)temp;
        }
        else if ( pParams->Name == "IndexURL" )
        {
            pParams->Value >>= aStr;
            maIndexUrl = aStr;
        }
        else if ( pParams->Name == "Format" )
        {
            sal_Int32 temp = 0;
            pParams->Value >>= temp;
            meFormat = (PublishingFormat)temp;
        }
        else if ( pParams->Name == "Compression" )
        {
            pParams->Value >>= aStr;
            String aTmp( aStr );
            if(aTmp.Len())
            {
                xub_StrLen nPos = aTmp.Search( '%' );
                if(nPos != STRING_NOTFOUND)
                    aTmp.Erase(nPos,1);
                mnCompression = (sal_Int16)aTmp.ToInt32();
            }
        }
        else if ( pParams->Name == "Width" )
        {
            sal_Int32 temp = 0;
            pParams->Value >>= temp;
            mnWidthPixel = (sal_uInt16)temp;
        }
        else if ( pParams->Name == "UseButtonSet" )
        {
            sal_Int32 temp = 0;
            pParams->Value >>= temp;
            mnButtonThema = (sal_Int16)temp;
        }
        else if ( pParams->Name == "IsExportNotes" )
        {
            if( mbImpress )
            {
                sal_Bool temp = sal_False;
                pParams->Value >>= temp;
                mbNotes = temp;
            }
        }
        else if ( pParams->Name == "IsExportContentsPage" )
        {
            sal_Bool temp = sal_False;
            pParams->Value >>= temp;
            mbContentsPage = temp;
        }
        else if ( pParams->Name == "Author" )
        {
            pParams->Value >>= aStr;
            maAuthor = aStr;
        }
        else if ( pParams->Name == "EMail" )
        {
            pParams->Value >>= aStr;
            maEMail = aStr;
        }
        else if ( pParams->Name == "HomepageURL" )
        {
            pParams->Value >>= aStr;
            maHomePage = aStr;
        }
        else if ( pParams->Name == "UserText" )
        {
            pParams->Value >>= aStr;
            maInfo = aStr;
        }
        else if ( pParams->Name == "EnableDownload" )
        {
            sal_Bool temp = sal_False;
            pParams->Value >>= temp;
            mbDownload = temp;
        }
        else if ( pParams->Name == "SlideSound" )
        {
            sal_Bool temp = sal_True;
            pParams->Value >>= temp;
            mbSlideSound = temp;
        }
        else if ( pParams->Name == "HiddenSlides" )
        {
            sal_Bool temp = sal_True;
            pParams->Value >>= temp;
            mbHiddenSlides = temp;
        }
        else if ( pParams->Name == "BackColor" )
        {
            sal_Int32 temp = 0;
            pParams->Value >>= temp;
            maBackColor = temp;
            mbUserAttr = true;
        }
        else if ( pParams->Name == "TextColor" )
        {
            sal_Int32 temp = 0;
            pParams->Value >>= temp;
            maTextColor = temp;
            mbUserAttr = true;
        }
        else if ( pParams->Name == "LinkColor" )
        {
            sal_Int32 temp = 0;
            pParams->Value >>= temp;
            maLinkColor = temp;
            mbUserAttr = true;
        }
        else if ( pParams->Name == "VLinkColor" )
        {
            sal_Int32 temp = 0;
            pParams->Value >>= temp;
            maVLinkColor = temp;
            mbUserAttr = true;
        }
        else if ( pParams->Name == "ALinkColor" )
        {
            sal_Int32 temp = 0;
            pParams->Value >>= temp;
            maALinkColor = temp;
            mbUserAttr = true;
        }
        else if ( pParams->Name == "IsUseDocumentColors" )
        {
            sal_Bool temp = sal_False;
            pParams->Value >>= temp;
            mbDocColors = temp;
        }
        else if ( pParams->Name == "KioskSlideDuration" )
        {
            sal_Int32 temp = sal_False;
            pParams->Value >>= temp;
            mnSlideDuration = temp;
            mbAutoSlide = true;
        }
        else if ( pParams->Name == "KioskEndless" )
        {
            sal_Bool temp = sal_False;
            pParams->Value >>= temp;
            mbEndless = temp;
        }
        else if ( pParams->Name == "WebCastCGIURL" )
        {
            pParams->Value >>= aStr;
            maCGIPath = aStr;
        }
        else if ( pParams->Name == "WebCastTargetURL" )
        {
            pParams->Value >>= aStr;
            maURLPath = aStr;
        }
        else if ( pParams->Name == "WebCastScriptLanguage" )
        {
            pParams->Value >>= aStr;
            if ( aStr == "asp" )
            {
                meScript = SCRIPT_ASP;
            }
            else
            {
                meScript = SCRIPT_PERL;
            }
        }
        else
        {
            OSL_FAIL("Unknown property for html export detected!");
        }

        pParams++;
    }

    if( meMode == PUBLISH_KIOSK )
    {
        mbContentsPage = false;
        mbNotes = false;

    }

    // calculate image sizes
    SdPage* pPage = mpDoc->GetSdPage(0, PK_STANDARD);
    Size aTmpSize( pPage->GetSize() );
    double dRatio=((double)aTmpSize.Width())/aTmpSize.Height();

    mnHeightPixel = (sal_uInt16)(mnWidthPixel/dRatio);

    //------------------------------------------------------------------
    // Ziel ausklamuestern...

    INetURLObject aINetURLObj( maPath );
    DBG_ASSERT( aINetURLObj.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

    maExportPath = aINetURLObj.GetPartBeforeLastName(); // with trailing '/'
    maIndex = aINetURLObj.GetLastName();

    mnSdPageCount = mpDoc->GetSdPageCount( PK_STANDARD );
    for( sal_uInt16 nPage = 0; nPage < mnSdPageCount; nPage++ )
    {
        pPage = mpDoc->GetSdPage( nPage, PK_STANDARD );

        if( mbHiddenSlides || !pPage->IsExcluded() )
        {
            maPages.push_back( pPage );
            maNotesPages.push_back( mpDoc->GetSdPage( nPage, PK_NOTES ) );
        }
    }
    mnSdPageCount = maPages.size();

    mbFrames = meMode == PUBLISH_FRAMES;

    maDocFileName = maIndex;
}

///////////////////////////////////////////////////////////////////////
// Exportiert das im Konstruktor angegebenne Impress Dokument nach HTML
///////////////////////////////////////////////////////////////////////
void HtmlExport::ExportHtml()
{
    if(mbUserAttr)
    {
        if( maTextColor == COL_AUTO )
        {
            if( !maBackColor.IsDark() )
                maTextColor = COL_BLACK;
        }
    }
    else if( mbDocColors )
    {
        // Standard Farben fuer das Farbschema 'Aus Dokument'
        SetDocColors();
        maFirstPageColor = maBackColor;
    }

    // get name for downloadable presentation if needed
    if( mbDownload )
    {
        // Separator such und Extension ausblenden
        sal_uInt16 nSepPos = maDocFileName.Search( sal_Unicode('.') );

        if(nSepPos != STRING_NOTFOUND)
            maDocFileName.Erase(nSepPos);

        maDocFileName.AppendAscii( ".odp" );
    }

    //////

    sal_uInt16 nProgrCount = mnSdPageCount;
    nProgrCount += mbImpress?mnSdPageCount:0;
    nProgrCount += mbContentsPage?1:0;
    nProgrCount += (mbFrames && mbNotes)?mnSdPageCount:0;
    nProgrCount += (mbFrames)?8:0;
    InitProgress( nProgrCount );

    mpDocSh->SetWaitCursor( true );

    //------------------------------------------------------------------
    // Exceptions sind doch was schoennes...

    CreateFileNames();

    // this is not a true while
    while( 1 )
    {
        if( checkForExistingFiles() )
            break;

        if( !CreateImagesForPresPages() )
            break;

        if( mbContentsPage &&
           !CreateImagesForPresPages( true ) )
            break;


        if( !CreateHtmlForPresPages() )
            break;

        if( mbImpress )
            if( !CreateHtmlTextForPresPages() )
                break;

        if( mbFrames )
        {
            if( !CreateFrames() )
                break;

            if( !CreateOutlinePages() )
                break;

            if( !CreateNavBarFrames() )
                break;

            if( mbNotes && mbImpress )
                if( !CreateNotesPages() )
                    break;

        }

        if( mbContentsPage )
            if( !CreateContentPage() )
                break;

        if( !CreateBitmaps() )
            break;

        mpDocSh->SetWaitCursor( false );
        ResetProgress();

        if( mbDownload )
            SavePresentation();

        return;
    }

    // if we get to this point the export was
    // canceled by the user after an error
    mpDocSh->SetWaitCursor( false );
    ResetProgress();
}

///////////////////////////////////////////////////////////////////////

void HtmlExport::SetDocColors( SdPage* pPage )
{
    if( pPage == NULL )
        pPage = mpDoc->GetSdPage(0, PK_STANDARD);

    svtools::ColorConfig aConfig;
    maVLinkColor = Color(aConfig.GetColorValue(svtools::LINKSVISITED).nColor);
    maALinkColor = Color(aConfig.GetColorValue(svtools::LINKS).nColor);
    maLinkColor  = Color(aConfig.GetColorValue(svtools::LINKS).nColor);
    maTextColor  = Color(COL_BLACK);

    SfxStyleSheet* pSheet = NULL;

    if( mpDoc->GetDocumentType() == DOCUMENT_TYPE_IMPRESS )
    {
        // Standard Textfarbe aus Outline-Vorlage der ersten Seite
        pSheet = pPage->GetStyleSheetForPresObj(PRESOBJ_OUTLINE);
        if(pSheet == NULL)
            pSheet = pPage->GetStyleSheetForPresObj(PRESOBJ_TEXT);
        if(pSheet == NULL)
            pSheet = pPage->GetStyleSheetForPresObj(PRESOBJ_TITLE);
    }

    if(pSheet == NULL)
        pSheet = mpDoc->GetDefaultStyleSheet();

    if(pSheet)
    {
        SfxItemSet& rSet = pSheet->GetItemSet();
        if(rSet.GetItemState(EE_CHAR_COLOR,sal_True) == SFX_ITEM_ON)
            maTextColor = ((SvxColorItem*)rSet.GetItem(EE_CHAR_COLOR,sal_True))->GetValue();
    }

    // Standard Hintergrundfarbe aus Background der MasterPage der ersten Seite
    maBackColor = pPage->GetPageBackgroundColor();

    if( maTextColor == COL_AUTO )
    {
        if( !maBackColor.IsDark() )
            maTextColor = COL_BLACK;
    }
}

///////////////////////////////////////////////////////////////////////

void HtmlExport::InitProgress( sal_uInt16 nProgrCount )
{
    String aStr(SdResId(STR_CREATE_PAGES));
    mpProgress = new SfxProgress( mpDocSh, aStr, nProgrCount );
}

///////////////////////////////////////////////////////////////////////

void HtmlExport::ResetProgress()
{
    delete mpProgress;
    mpProgress = NULL;
}

///////////////////////////////////////////////////////////////////////

void HtmlExport::ExportKiosk()
{
    mnPagesWritten = 0;
    InitProgress( 2*mnSdPageCount );

    CreateFileNames();
    if( !checkForExistingFiles() )
    {
        if( CreateImagesForPresPages() )
            CreateHtmlForPresPages();
    }

    ResetProgress();
}

///////////////////////////////////////////////////////////////////////
// Export Document with WebCast (TM) Technology
///////////////////////////////////////////////////////////////////////
void HtmlExport::ExportWebCast()
{
    mnPagesWritten = 0;
    InitProgress( mnSdPageCount + 9 );

    mpDocSh->SetWaitCursor( sal_True );

    CreateFileNames();

    String aEmpty;
    if(maCGIPath.Len() == 0)
        maCGIPath.Assign( sal_Unicode('.') );

    if( maCGIPath.GetChar( maCGIPath.Len() - 1 ) != sal_Unicode('/') )
        maCGIPath.Append( sal_Unicode('/') );

    if( meScript == SCRIPT_ASP )
    {
        maURLPath.AssignAscii( "./" );
    }
    else
    {
           String aEmpty2;
        if(maURLPath.Len() == 0)
            maURLPath.Assign( sal_Unicode('.') );

        if( maURLPath.GetChar( maURLPath.Len() - 1 ) != sal_Unicode('/') )
            maURLPath.Append( sal_Unicode('/') );
    }

    // this is not a true while
    while(1)
    {
        if( checkForExistingFiles() )
            break;

        if(!CreateImagesForPresPages())
            break;

        if( meScript == SCRIPT_ASP )
        {
            if(!CreateASPScripts())
                break;
        }
        else
        {
            if(!CreatePERLScripts())
                break;
        }

        if(!CreateImageFileList())
            break;

        if(!CreateImageNumberFile())
            break;

        break;
    }

    mpDocSh->SetWaitCursor( false );
    ResetProgress();
}

///////////////////////////////////////////////////////////////////////
// Save the presentation as a downloadable file in the dest directory
///////////////////////////////////////////////////////////////////////

bool HtmlExport::SavePresentation()
{
    meEC.SetContext( STR_HTMLEXP_ERROR_CREATE_FILE, maDocFileName );

    OUString aURL( maExportPath );
    aURL += maDocFileName;


    mpDocSh->EnableSetModified( true );

    try
    {
        uno::Reference< frame::XStorable > xStorable( mpDoc->getUnoModel(), uno::UNO_QUERY );
        if( xStorable.is() )
        {
            uno::Sequence< beans::PropertyValue > aProperties( 2 );
            aProperties[ 0 ].Name = "Overwrite";
            aProperties[ 0 ].Value <<= (sal_Bool)sal_True;
            aProperties[ 1 ].Name = "FilterName";
            aProperties[ 1 ].Value <<= rtl::OUString("impress8");
            xStorable->storeToURL( aURL, aProperties );

            mpDocSh->EnableSetModified( false );

            return true;
        }
    }
    catch( Exception& )
    {
    }

    mpDocSh->EnableSetModified( false );

    return false;
}

// =====================================================================
// Image-Dateien anlegen
// =====================================================================
bool HtmlExport::CreateImagesForPresPages( bool bThumbnail)
{
    try
    {
        Reference < XMultiServiceFactory > xMSF( ::comphelper::getProcessServiceFactory() );
        if( !xMSF.is() )
            return false;

        Reference< XExporter > xGraphicExporter( xMSF->createInstance( "com.sun.star.drawing.GraphicExportFilter" ), UNO_QUERY );
        Reference< XFilter > xFilter( xGraphicExporter, UNO_QUERY );

        DBG_ASSERT( xFilter.is(), "no com.sun.star.drawing.GraphicExportFilter?" );
        if( !xFilter.is() )
            return false;

        Sequence< PropertyValue > aFilterData(((meFormat==FORMAT_JPG)&&(mnCompression != -1))? 3 : 2);
        aFilterData[0].Name = "PixelWidth";
        aFilterData[0].Value <<= (sal_Int32)(bThumbnail ? PUB_THUMBNAIL_WIDTH : mnWidthPixel );
        aFilterData[1].Name = "PixelHeight";
        aFilterData[1].Value <<= (sal_Int32)(bThumbnail ? PUB_THUMBNAIL_HEIGHT : mnHeightPixel);
        if((meFormat==FORMAT_JPG)&&(mnCompression != -1))
        {
            aFilterData[2].Name = "Quality";
            aFilterData[2].Value <<= (sal_Int32)mnCompression;
        }

        Sequence< PropertyValue > aDescriptor( 3 );
        aDescriptor[0].Name = "URL";
        aDescriptor[1].Name = "FilterName";
        OUString sFormat;
        if( meFormat == FORMAT_PNG )
            sFormat = "PNG";
        else if( meFormat == FORMAT_GIF )
            sFormat = "GIF";
        else
            sFormat = "JPG";

        aDescriptor[1].Value <<= sFormat;
        aDescriptor[2].Name = "FilterData";
        aDescriptor[2].Value <<= aFilterData;

        for (sal_uInt16 nSdPage = 0; nSdPage < mnSdPageCount; nSdPage++)
        {
            SdPage* pPage = maPages[ nSdPage ];

            OUString aFull(maExportPath);
            if (bThumbnail)
                aFull += *mpThumbnailFiles[nSdPage];
            else
                aFull += *mpImageFiles[nSdPage];


            aDescriptor[0].Value <<= aFull;

            Reference< XComponent > xPage( pPage->getUnoPage(), UNO_QUERY );
            xGraphicExporter->setSourceDocument( xPage );
            xFilter->filter( aDescriptor );

            if (mpProgress)
                mpProgress->SetState(++mnPagesWritten);
        }
    }
    catch( Exception& )
    {
        return false;
    }

    return true;
}

// =====================================================================
// Ermittelt das SdrTextObject mit dem Layout Text dieser Seite
// =====================================================================
SdrTextObj* HtmlExport::GetLayoutTextObject(SdrPage* pPage)
{
    sal_uLong           nObjectCount = pPage->GetObjCount();
    SdrObject*      pObject      = NULL;
    SdrTextObj*     pResult      = NULL;

    for (sal_uLong nObject = 0; nObject < nObjectCount; nObject++)
    {
        pObject = pPage->GetObj(nObject);
        if (pObject->GetObjInventor() == SdrInventor &&
            pObject->GetObjIdentifier() == OBJ_OUTLINETEXT)
        {
            pResult = (SdrTextObj*)pObject;
            break;
        }
    }
    return pResult;
}

// =====================================================================
// HTML-Text Versionen fuer Impress Seiten erzeugen
// =====================================================================

String HtmlExport::WriteMetaCharset() const
{
    String aStr;
    const sal_Char *pCharSet = rtl_getBestMimeCharsetFromTextEncoding( RTL_TEXTENCODING_UTF8 );
    if ( pCharSet )
    {
        aStr.AppendAscii( "  <meta HTTP-EQUIV=CONTENT-TYPE CONTENT=\"text/html; charset=" );
        aStr.AppendAscii( pCharSet );
        aStr.AppendAscii( "\">\r\n" );
    }
    return aStr;
}

bool HtmlExport::CreateHtmlTextForPresPages()
{
    bool bOk = true;

    SdrOutliner* pOutliner = mpDoc->GetInternalOutliner();

    for(sal_uInt16 nSdPage = 0; nSdPage < mnSdPageCount && bOk; nSdPage++)
    {
        SdPage* pPage = maPages[ nSdPage ];

        if( mbDocColors )
        {
            SetDocColors( pPage );
        }

// HTML Kopf
        String aStr(maHTMLHeader);
        aStr += WriteMetaCharset();
        aStr.AppendAscii( "  <title>" );
        aStr += StringToHTMLString( *mpPageNames[nSdPage] );
        aStr.AppendAscii( "</title>\r\n" );
        aStr.AppendAscii( "</head>\r\n" );
        aStr += CreateBodyTag();

// Navigationsleiste
        aStr += CreateNavBar(nSdPage, true);

// Seitentitel
        String sTitleText( CreateTextForTitle(pOutliner,pPage, pPage->GetPageBackgroundColor()) );
        aStr.AppendAscii( "<h1 style=\"");
        aStr.Append( getParagraphStyle( pOutliner, 0 ) );
        aStr.AppendAscii( "\">" );
        aStr += sTitleText;
        aStr.AppendAscii( "</h1>\r\n" );

// Gliederungstext schreiben
        aStr += CreateTextForPage( pOutliner, pPage, true, pPage->GetPageBackgroundColor() );

// Notizen
        if(mbNotes)
        {
            SdPage* pNotesPage = maNotesPages[ nSdPage ];
            String aNotesStr( CreateTextForNotesPage( pOutliner, pNotesPage, true, maBackColor) );

            if( aNotesStr.Len() )
            {
                aStr.AppendAscii( "<br>\r\n<h3>" );
                aStr += RESTOHTML(STR_HTMLEXP_NOTES);
                aStr.AppendAscii( ":</h3>\r\n" );

                aStr += aNotesStr;
            }
        }

// Seite beenden
        aStr.AppendAscii( "</body>\r\n</html>" );

        bOk = WriteHtml( *mpTextFiles[nSdPage], false, aStr );

        if (mpProgress)
            mpProgress->SetState(++mnPagesWritten);

    }

    pOutliner->Clear();

    return bOk;
}

/** exports the given html data into a non unicode file in the current export path with
    the given filename */
bool HtmlExport::WriteHtml( const String& rFileName, bool bAddExtension, const String& rHtmlData )
{
    sal_uLong nErr = 0;

    String aFileName( rFileName );
    if( bAddExtension )
        aFileName += maHTMLExtension;

    meEC.SetContext( STR_HTMLEXP_ERROR_CREATE_FILE, rFileName );
    EasyFile aFile;
    SvStream* pStr;
    String aFull( maExportPath );
    aFull += aFileName;
    nErr = aFile.createStream(aFull , pStr);
    if(nErr == 0)
    {
        rtl::OString aStr(rtl::OUStringToOString(rHtmlData,
            RTL_TEXTENCODING_UTF8));
        *pStr << aStr.getStr();
        nErr = aFile.close();
    }

    if( nErr != 0 )
        ErrorHandler::HandleError(nErr);

    return nErr == 0;
}

// =====================================================================

/** Erzeugt den Outliner Text fuer das Titelobjekt einer Seite
 */
String HtmlExport::CreateTextForTitle( SdrOutliner* pOutliner, SdPage* pPage, const Color& rBackgroundColor )
{
    SdrTextObj* pTO = (SdrTextObj*)pPage->GetPresObj(PRESOBJ_TITLE);
    if(!pTO)
        pTO = GetLayoutTextObject(pPage);

    if (pTO && !pTO->IsEmptyPresObj())
    {
        OutlinerParaObject* pOPO = pTO->GetOutlinerParaObject();
        if(pOPO && pOutliner->GetParagraphCount() != 0)
        {
            pOutliner->Clear();
            pOutliner->SetText(*pOPO);
            return ParagraphToHTMLString(pOutliner,0, rBackgroundColor);
        }
    }

    return String();
}

// =====================================================================
// Erzeugt den Outliner Text fuer eine Seite
// =====================================================================
String HtmlExport::CreateTextForPage( SdrOutliner* pOutliner,
                                      SdPage* pPage,
                                      bool bHeadLine, const Color& rBackgroundColor )
{
    String aStr;

    SdrTextObj* pTO = (SdrTextObj*)pPage->GetPresObj(PRESOBJ_TEXT);
    if(!pTO)
        pTO = GetLayoutTextObject(pPage);

    if (pTO && !pTO->IsEmptyPresObj())
    {
        OutlinerParaObject* pOPO = pTO->GetOutlinerParaObject();
        if (pOPO)
        {
            pOutliner->Clear();
            pOutliner->SetText( *pOPO );

            sal_uLong nCount = pOutliner->GetParagraphCount();

            Paragraph* pPara = NULL;
            sal_Int16 nActDepth = -1;

            String aParaText;
            for (sal_uLong nPara = 0; nPara < nCount; nPara++)
            {
                pPara = pOutliner->GetParagraph(nPara);
                if(pPara == 0)
                    continue;

                const sal_Int16 nDepth = (sal_uInt16) pOutliner->GetDepth( (sal_uInt16) nPara );
                aParaText = ParagraphToHTMLString(pOutliner,nPara,rBackgroundColor);

                if(aParaText.Len() == 0)
                    continue;

                if(nDepth < nActDepth )
                {
                    do
                    {
                        aStr.AppendAscii( "</ul>" );
                        nActDepth--;
                    }
                    while(nDepth < nActDepth);
                }
                else if(nDepth > nActDepth )
                {
                    do
                    {
                        aStr.AppendAscii( "<ul>" );
                        nActDepth++;
                    }
                    while( nDepth > nActDepth );
                }

                String sStyle( getParagraphStyle( pOutliner, nPara ) );
                if(nActDepth >= 0 )
                {
                    aStr.AppendAscii( "<li style=\"");
                    aStr.Append( sStyle );
                    aStr.AppendAscii( "\">" );
                }

                if(nActDepth <= 0 && bHeadLine)
                {
                    if( nActDepth == 0 )
                    {
                        aStr.AppendAscii( "<h2>" );
                    }
                    else
                    {
                        aStr.AppendAscii( "<h2 style=\"");
                        aStr.Append( sStyle );
                        aStr.AppendAscii( "\">" );
                    }
                }
                aStr += aParaText;
                if(nActDepth == 0 && bHeadLine)
                    aStr.AppendAscii( "</h2>" );
                if(nActDepth >= 0 )
                    aStr.AppendAscii( "</li>" );
                aStr.AppendAscii( "\r\n" );
            }

            while( nActDepth >= 0 )
            {
                aStr.AppendAscii( "</ul>" );
                nActDepth--;
            };
        }
    }

    return aStr;
}

// =====================================================================
// Erzeugt den Outliner Text fuer eine Notizseite
// =====================================================================
String HtmlExport::CreateTextForNotesPage( SdrOutliner* pOutliner,
                                           SdPage* pPage,
                                           bool,
                                           const Color& rBackgroundColor )
{
    String aStr;

    SdrTextObj* pTO = (SdrTextObj*)pPage->GetPresObj(PRESOBJ_NOTES);

    if (pTO && !pTO->IsEmptyPresObj())
    {
        OutlinerParaObject* pOPO = pTO->GetOutlinerParaObject();
        if (pOPO)
        {
            pOutliner->Clear();
            pOutliner->SetText( *pOPO );

            sal_uLong nCount = pOutliner->GetParagraphCount();
            for (sal_uLong nPara = 0; nPara < nCount; nPara++)
            {
                aStr.AppendAscii("<p style=\"");
                aStr.Append( getParagraphStyle( pOutliner, nPara ) );
                aStr.AppendAscii("\">");
                aStr += ParagraphToHTMLString( pOutliner, nPara,rBackgroundColor );
                aStr.AppendAscii( "</p>\r\n" );
            }
        }
    }

    return aStr;
}

// =====================================================================
// Wandelt einen Paragraphen des Outliners in Html
// =====================================================================
String HtmlExport::ParagraphToHTMLString( SdrOutliner* pOutliner, sal_uLong nPara, const Color& rBackgroundColor )
{
    String aStr;

    if(NULL == pOutliner)
        return aStr;

    // TODO: MALTE!!!
    EditEngine& rEditEngine = *(EditEngine*)&pOutliner->GetEditEngine();
    bool bOldUpdateMode = rEditEngine.GetUpdateMode();
    rEditEngine.SetUpdateMode(sal_True);

    Paragraph* pPara = pOutliner->GetParagraph(nPara);
    if(NULL == pPara)
        return aStr;

    HtmlState aState( (mbUserAttr || mbDocColors)  ? maTextColor : Color(COL_BLACK) );
    std::vector<sal_uInt16> aPortionList;
    rEditEngine.GetPortions( (sal_uInt16) nPara, aPortionList );

    sal_uInt16 nPos1 = 0;
    for( std::vector<sal_uInt16>::const_iterator it( aPortionList.begin() ); it != aPortionList.end(); ++it )
    {
        sal_uInt16 nPos2 = *it;

        ESelection aSelection( (sal_uInt16) nPara, nPos1, (sal_uInt16) nPara, nPos2);

        SfxItemSet aSet( rEditEngine.GetAttribs( aSelection ) );

        String aPortion(StringToHTMLString(rEditEngine.GetText( aSelection )));

        aStr += TextAttribToHTMLString( &aSet, &aState, rBackgroundColor );
        aStr += aPortion;

        nPos1 = nPos2;
    }
    aStr += aState.Flush();
    rEditEngine.SetUpdateMode(bOldUpdateMode);

    return aStr;
}

// =====================================================================
// Erzeugt anhand der Attribute im angegebennen Set und dem gegebennen
// HtmlState die noetigen Html-Tags um die Attribute zu uebernehmen
// =====================================================================
String HtmlExport::TextAttribToHTMLString( SfxItemSet* pSet, HtmlState* pState, const Color& rBackgroundColor )
{
    String aStr;

    if(NULL == pSet)
        return aStr;

    String aLink, aTarget;
    if ( pSet->GetItemState( EE_FEATURE_FIELD ) == SFX_ITEM_ON )
    {
        SvxFieldItem* pItem = (SvxFieldItem*)pSet->GetItem( EE_FEATURE_FIELD );
        if(pItem)
        {
            SvxURLField* pURL = PTR_CAST(SvxURLField, pItem->GetField());
            if(pURL)
            {
                aLink = pURL->GetURL();
                aTarget = pURL->GetTargetFrame();
            }
        }
    }

    bool bTemp;
    String aTemp;

    if ( pSet->GetItemState( EE_CHAR_WEIGHT ) == SFX_ITEM_ON )
    {
        bTemp = ((const SvxWeightItem&)pSet->Get( EE_CHAR_WEIGHT )).GetWeight() == WEIGHT_BOLD;
        aTemp = pState->SetWeight( bTemp );
        if( bTemp )
            aStr.Insert( aTemp, 0 );
        else
            aStr += aTemp;
    }

    if ( pSet->GetItemState( EE_CHAR_UNDERLINE ) == SFX_ITEM_ON )
    {
        bTemp = ((const SvxUnderlineItem&)pSet->Get( EE_CHAR_UNDERLINE )).GetLineStyle() != UNDERLINE_NONE;
        aTemp = pState->SetUnderline( bTemp );
        if( bTemp )
            aStr.Insert( aTemp, 0 );
        else
            aStr += aTemp;
    }

    if ( pSet->GetItemState( EE_CHAR_STRIKEOUT ) == SFX_ITEM_ON )
    {
        bTemp = ((const SvxCrossedOutItem&)pSet->Get( EE_CHAR_STRIKEOUT )).GetStrikeout() != STRIKEOUT_NONE;
        aTemp = pState->SetStrikeout( bTemp );
        if( bTemp )
            aStr.Insert( aTemp, 0 );
        else
            aStr += aTemp;
    }

    if ( pSet->GetItemState( EE_CHAR_ITALIC ) == SFX_ITEM_ON )
    {
        bTemp = ((const SvxPostureItem&)pSet->Get( EE_CHAR_ITALIC )).GetPosture() != ITALIC_NONE;
        aTemp = pState->SetItalic( bTemp );
        if( bTemp )
            aStr.Insert( aTemp, 0 );
        else
            aStr += aTemp;
    }

    if(mbDocColors)
    {
        if ( pSet->GetItemState( EE_CHAR_COLOR ) == SFX_ITEM_ON )
        {
            Color aTextColor = ((const SvxColorItem&) pSet->Get( EE_CHAR_COLOR )).GetValue();
            if( aTextColor == COL_AUTO )
            {
                if( !rBackgroundColor.IsDark() )
                    aTextColor = COL_BLACK;
            }
            aStr += pState->SetColor( aTextColor );
        }
    }

    if( aLink.Len() )
        aStr.Insert( pState->SetLink(aLink, aTarget), 0 );
    else
        aStr += pState->SetLink(aLink, aTarget);

    return aStr;
}

// =====================================================================
// HTML-Wrapper fuer Bild-Dateien erzeugen
// =====================================================================
bool HtmlExport::CreateHtmlForPresPages()
{
    bool bOk = true;

    std::vector<SdrObject*> aClickableObjects;

    for(sal_uInt16 nSdPage = 0; nSdPage < mnSdPageCount && bOk; nSdPage++)
    {
        // Klickbare Objekte finden (auch auf der Masterpage) und
        // in Liste stellen. In umgekehrter Zeichenreihenfolge in
        // die Liste stellen, da in HTML bei Ueberlappungen die
        // _erstgenannte_ Area wirkt.

        SdPage* pPage = maPages[ nSdPage ];

        if( mbDocColors )
        {
            SetDocColors( pPage );
        }

        bool    bMasterDone = false;

        while (!bMasterDone)
        {
            // sal_True = rueckwaerts
            SdrObjListIter aIter(*pPage, IM_DEEPWITHGROUPS, sal_True);

            SdrObject* pObject = aIter.Next();
            while (pObject)
            {
                SdAnimationInfo* pInfo     = mpDoc->GetAnimationInfo(pObject);
                SdIMapInfo*      pIMapInfo = mpDoc->GetIMapInfo(pObject);

                if ((pInfo &&
                     (pInfo->meClickAction == presentation::ClickAction_BOOKMARK  ||
                      pInfo->meClickAction == presentation::ClickAction_DOCUMENT  ||
                      pInfo->meClickAction == presentation::ClickAction_PREVPAGE  ||
                      pInfo->meClickAction == presentation::ClickAction_NEXTPAGE  ||
                      pInfo->meClickAction == presentation::ClickAction_FIRSTPAGE ||
                      pInfo->meClickAction == presentation::ClickAction_LASTPAGE)) ||
                     pIMapInfo)
                {
                    aClickableObjects.push_back(pObject);
                }

                pObject = aIter.Next();
            }
            // jetzt zur Masterpage oder beenden
            if (!pPage->IsMasterPage())
                pPage = (SdPage*)(&(pPage->TRG_GetMasterPage()));
            else
                bMasterDone = true;
        }

// HTML Head
        String aStr(maHTMLHeader);
        aStr += WriteMetaCharset();
        aStr.AppendAscii( "  <title>" );
        aStr += StringToHTMLString(*mpPageNames[nSdPage]);
        aStr.AppendAscii( "</title>\r\n" );

// insert timing information
        pPage = maPages[ nSdPage ];
        if( meMode == PUBLISH_KIOSK )
        {
            sal_uLong nSecs = 0;
            bool bEndless = false;
            if( !mbAutoSlide )
            {
                if( pPage->GetPresChange() != PRESCHANGE_MANUAL )
                {
                    nSecs = pPage->GetTime();
                    bEndless = mpDoc->getPresentationSettings().mbEndless;
                }
            }
            else
            {
                nSecs = mnSlideDuration;
                bEndless = mbEndless;
            }

            if( nSecs != 0 )
            {
                if( nSdPage < (mnSdPageCount-1) || bEndless )
                {
                    aStr.AppendAscii( "<meta http-equiv=\"refresh\" content=\"" );
                    aStr += String::CreateFromInt32(nSecs);
                    aStr.AppendAscii( "; URL=" );

                    int nPage = nSdPage + 1;
                    if( nPage == mnSdPageCount )
                        nPage = 0;

                    aStr += StringToURL(*mpHTMLFiles[nPage]);

                    aStr.AppendAscii( "\">\r\n" );
                }
            }
        }

        aStr.AppendAscii( "</head>\r\n" );

// HTML Body
        aStr += CreateBodyTag();

        if( mbSlideSound && pPage->IsSoundOn() )
            aStr += InsertSound( pPage->GetSoundFile() );

// Navigationsleiste
        if(!mbFrames )
            aStr += CreateNavBar( nSdPage, false );
// Image
        aStr.AppendAscii( "<center>" );
        aStr.AppendAscii( "<img src=\"" );
        aStr += StringToURL( *mpImageFiles[nSdPage] );
        aStr.AppendAscii( "\" alt=\"\"" );

        if (!aClickableObjects.empty())
            aStr.AppendAscii( " USEMAP=\"#map0\"" );

        aStr.AppendAscii( "></center>\r\n" );

// Notizen
        if(mbNotes && !mbFrames)
        {
            SdrOutliner* pOutliner = mpDoc->GetInternalOutliner();
            SdPage* pNotesPage = maNotesPages[ nSdPage ];
            String aNotesStr( CreateTextForNotesPage( pOutliner, pNotesPage, true, maBackColor) );
            pOutliner->Clear();

            if( aNotesStr.Len() )
            {
                aStr.AppendAscii( "<h3>" );
                aStr += RESTOHTML(STR_HTMLEXP_NOTES);
                aStr.AppendAscii( ":</h3><br>\r\n\r\n<p>" );

                aStr += aNotesStr;
                aStr.AppendAscii( "\r\n</p>\r\n" );
            }
        }

// ggfs. Imagemap erzeugen
        if (!aClickableObjects.empty())
        {
            aStr.AppendAscii( "<map name=\"map0\">\r\n" );

            for (sal_uInt32 nObject = 0, n = aClickableObjects.size(); nObject < n; nObject++)
            {
                SdrObject* pObject = aClickableObjects[nObject];
                SdAnimationInfo* pInfo     = mpDoc->GetAnimationInfo(pObject);
                SdIMapInfo*      pIMapInfo = mpDoc->GetIMapInfo(pObject);

                Rectangle aRect(pObject->GetCurrentBoundRect());
                Point     aLogPos(aRect.TopLeft());
                bool      bIsSquare = aRect.GetWidth() == aRect.GetHeight();

                sal_uLong nPageWidth = pPage->GetSize().Width() - pPage->GetLftBorder() -
                                   pPage->GetRgtBorder();

                // das BoundRect bezieht sich auf den physikalischen
                // Seitenursprung, nicht auf den Koordinatenursprung
                aRect.Move(-pPage->GetLftBorder(), -pPage->GetUppBorder());

                double fLogicToPixel = ((double)mnWidthPixel) / nPageWidth;
                aRect.Left()   = (long)(aRect.Left() * fLogicToPixel);
                aRect.Top()    = (long)(aRect.Top() * fLogicToPixel);
                aRect.Right()  = (long)(aRect.Right() * fLogicToPixel);
                aRect.Bottom() = (long)(aRect.Bottom() * fLogicToPixel);
                long nRadius = aRect.GetWidth() / 2;


                /*************************************************************
                |* wenn das Objekt eine eigene Imagemap enthaelt, werden ihre
                |* Areas in diese Imagemap eingefuegt
                \************************************************************/
                if (pIMapInfo)
                {
                    const ImageMap& rIMap = pIMapInfo->GetImageMap();
                    sal_uInt16 nAreaCount = rIMap.GetIMapObjectCount();
                    for (sal_uInt16 nArea = 0; nArea < nAreaCount; nArea++)
                    {
                        IMapObject* pArea = rIMap.GetIMapObject(nArea);
                        sal_uInt16 nType = pArea->GetType();
                        String aURL( pArea->GetURL() );

                        // ggfs. Seiten- oder Objektnamen umwandeln in den
                        // Namen der entsprechenden HTML-Datei
                        sal_Bool        bIsMasterPage;
                        sal_uInt16      nPgNum = mpDoc->GetPageByName( aURL, bIsMasterPage );
                        SdrObject*  pObj = NULL;

                        if (nPgNum == SDRPAGE_NOTFOUND)
                        {
                            // Ist das Bookmark ein Objekt?
                            pObj = mpDoc->GetObj( aURL );
                            if (pObj)
                                nPgNum = pObj->GetPage()->GetPageNum();
                        }
                        if (nPgNum != SDRPAGE_NOTFOUND)
                        {
                            nPgNum = (nPgNum - 1) / 2;  // SdrPageNum --> SdPageNum
                            aURL = CreatePageURL(nPgNum);
                        }

                        switch(nType)
                        {
                            case IMAP_OBJ_RECTANGLE:
                            {
                                Rectangle aArea(((IMapRectangleObject*)pArea)->
                                                 GetRectangle(false));

                                // Umrechnung in Pixelkoordinaten
                                aArea.Move(aLogPos.X() - pPage->GetLftBorder(),
                                           aLogPos.Y() - pPage->GetUppBorder());
                                aArea.Left()   = (long)(aArea.Left() * fLogicToPixel);
                                aArea.Top()    = (long)(aArea.Top() * fLogicToPixel);
                                aArea.Right()  = (long)(aArea.Right() * fLogicToPixel);
                                aArea.Bottom() = (long)(aArea.Bottom() * fLogicToPixel);

                                aStr += CreateHTMLRectArea(aArea, aURL);
                            }
                            break;

                            case IMAP_OBJ_CIRCLE:
                            {
                                Point aCenter(((IMapCircleObject*)pArea)->
                                                 GetCenter(false));
                                aCenter += Point(aLogPos.X() - pPage->GetLftBorder(),
                                                 aLogPos.Y() - pPage->GetUppBorder());
                                aCenter.X() = (long)(aCenter.X() * fLogicToPixel);
                                aCenter.Y() = (long)(aCenter.Y() * fLogicToPixel);

                                sal_uLong nCircleRadius = (((IMapCircleObject*)pArea)->
                                                 GetRadius(false));
                                nCircleRadius = (sal_uLong)(nCircleRadius * fLogicToPixel);
                                aStr += CreateHTMLCircleArea(nCircleRadius,
                                                            aCenter.X(), aCenter.Y(),
                                                            aURL);
                            }
                            break;

                            case IMAP_OBJ_POLYGON:
                            {
                                Polygon aArea(((IMapPolygonObject*)pArea)->GetPolygon(false));
                                aStr += CreateHTMLPolygonArea(::basegfx::B2DPolyPolygon(aArea.getB2DPolygon()), Size(aLogPos.X() - pPage->GetLftBorder(), aLogPos.Y() - pPage->GetUppBorder()), fLogicToPixel, aURL);
                            }
                            break;

                            default:
                            {
                                DBG_WARNING("unbekannter IMAP_OBJ_Typ");
                            }
                            break;
                        }
                    }
                }



                /*************************************************************
                |* wenn es eine presentation::ClickAction gibt, Bookmark bestimmen und eine
                |* Area fuer das ganze Objekt erzeugen
                \************************************************************/
                if( pInfo )
                {
                    String      aHRef;
                    presentation::ClickAction eClickAction = pInfo->meClickAction;

                    switch( eClickAction )
                    {
                        case presentation::ClickAction_BOOKMARK:
                        {
                            sal_Bool        bIsMasterPage;
                            sal_uInt16      nPgNum = mpDoc->GetPageByName( pInfo->GetBookmark(), bIsMasterPage );
                            SdrObject*  pObj = NULL;

                            if( nPgNum == SDRPAGE_NOTFOUND )
                            {
                                // Ist das Bookmark ein Objekt?
                                pObj = mpDoc->GetObj(pInfo->GetBookmark());
                                if (pObj)
                                    nPgNum = pObj->GetPage()->GetPageNum();
                            }

                            if( SDRPAGE_NOTFOUND != nPgNum )
                                aHRef = CreatePageURL(( nPgNum - 1 ) / 2 );
                        }
                        break;

                        case presentation::ClickAction_DOCUMENT:
                            aHRef = pInfo->GetBookmark();
                        break;

                        case presentation::ClickAction_PREVPAGE:
                        {
                            sal_uLong nPage = nSdPage;
                            if (nSdPage == 0)
                                nPage = 0;
                            else
                                nPage = nSdPage - 1;

                            aHRef = CreatePageURL( (sal_uInt16) nPage);
                        }
                        break;

                        case presentation::ClickAction_NEXTPAGE:
                        {
                            sal_uLong nPage = nSdPage;
                            if (nSdPage == mnSdPageCount - 1)
                                nPage = mnSdPageCount - 1;
                            else
                                nPage = nSdPage + 1;

                            aHRef = CreatePageURL( (sal_uInt16) nPage);
                        }
                        break;

                        case presentation::ClickAction_FIRSTPAGE:
                            aHRef = CreatePageURL(0);
                        break;

                        case presentation::ClickAction_LASTPAGE:
                            aHRef = CreatePageURL(mnSdPageCount - 1);
                        break;

                        default:
                            break;
                    }

                    // jetzt die Areas
                    if( aHRef.Len() )
                    {
                        // ein Kreis?
                        if (pObject->GetObjInventor() == SdrInventor &&
                            pObject->GetObjIdentifier() == OBJ_CIRC  &&
                            bIsSquare )
                        {
                            aStr += CreateHTMLCircleArea(aRect.GetWidth() / 2,
                                                    aRect.Left() + nRadius,
                                                    aRect.Top() + nRadius,
                                                    aHRef);
                        }
                        // ein Polygon?
                        else if (pObject->GetObjInventor() == SdrInventor &&
                                 (pObject->GetObjIdentifier() == OBJ_PATHLINE ||
                                  pObject->GetObjIdentifier() == OBJ_PLIN ||
                                  pObject->GetObjIdentifier() == OBJ_POLY))
                        {
                            aStr += CreateHTMLPolygonArea(((SdrPathObj*)pObject)->GetPathPoly(), Size(-pPage->GetLftBorder(), -pPage->GetUppBorder()), fLogicToPixel, aHRef);
                        }
                        // was anderes: das BoundRect nehmen
                        else
                        {
                            aStr += CreateHTMLRectArea(aRect, aHRef);
                        }

                    }
                }
            }

            aStr.AppendAscii( "</map>\r\n" );
        }
        aClickableObjects.clear();

        aStr.AppendAscii( "</body>\r\n</html>" );

        bOk = WriteHtml( *mpHTMLFiles[nSdPage], false, aStr );

        if (mpProgress)
            mpProgress->SetState(++mnPagesWritten);
    }

    return bOk;
}

// =====================================================================
// Uebersichtsseite erzeugen
// =====================================================================
bool HtmlExport::CreateContentPage()
{
    // Parameter
    String aEmpty;

    if( mbDocColors )
        SetDocColors();

    // Html Kopf
    String aStr(maHTMLHeader);
    aStr += WriteMetaCharset();
    aStr.AppendAscii( "  <title>" );
    aStr += StringToHTMLString(*mpPageNames[0]);
    aStr.AppendAscii( "</title>\r\n</head>\r\n" );
    aStr += CreateBodyTag();

    // Seitenkopf
    aStr.AppendAscii( "<center>\r\n" );

    if(mbHeader)
    {
        aStr.AppendAscii( "<h1>" );
        aStr += getDocumentTitle();
        aStr.AppendAscii( "</h1><br>\r\n" );
    }

    aStr.AppendAscii( "<h2>" );

    // Solaris compiler bug workaround
    if( mbFrames )
        aStr += CreateLink( maFramePage,
                            RESTOHTML(STR_HTMLEXP_CLICKSTART) );
    else
        aStr += CreateLink( StringToHTMLString(*mpHTMLFiles[0]),
                            RESTOHTML(STR_HTMLEXP_CLICKSTART) );

    aStr.AppendAscii( "</h2>\r\n</center>\r\n" );

    aStr.AppendAscii( "<center><table width=\"90%\"><tr>\r\n" );

    // Inhaltsverzeichnis
    aStr.AppendAscii( "<td valign=\"top\" align=\"left\" width=\"25%\">\r\n" );
    aStr.AppendAscii( "<h3>" );
    aStr += RESTOHTML(STR_HTMLEXP_CONTENTS);
    aStr.AppendAscii( "</h3>" );

    for(sal_uInt16 nSdPage = 0; nSdPage < mnSdPageCount; nSdPage++)
    {
        String aPageName = *mpPageNames[nSdPage];
        aStr.AppendAscii( "<div align=\"left\">" );
        if(mbFrames)
            aStr += StringToHTMLString(aPageName);
        else
            aStr += CreateLink(*mpHTMLFiles[nSdPage], aPageName);
        aStr.AppendAscii( "</div>\r\n" );
    }
    aStr.AppendAscii( "</td>\r\n" );

    // Dokument Infos
    aStr.AppendAscii( "<td valign=\"top\" align=\"left\" width=\"75%\">\r\n" );

    if(maAuthor.Len())
    {
        aStr.AppendAscii( "<p><strong>" );
        aStr += RESTOHTML(STR_HTMLEXP_AUTHOR);
        aStr.AppendAscii( ":</strong> " );
        aStr += StringToHTMLString(maAuthor);
        aStr.AppendAscii( "</p>\r\n" );
    }

    if(maEMail.Len())
    {
        aStr.AppendAscii( "<p><strong>" );
        aStr += RESTOHTML(STR_HTMLEXP_EMAIL);
        aStr.AppendAscii( ":</strong> <a href=\"mailto:" );
        aStr += StringToURL(maEMail);
        aStr.AppendAscii( "\">" );
        aStr += StringToHTMLString(maEMail);
        aStr.AppendAscii( "</a></p>\r\n" );
    }

    if(maHomePage.Len())
    {
        aStr.AppendAscii( "<p><strong>" );
        aStr += RESTOHTML(STR_HTMLEXP_HOMEPAGE);
        aStr.AppendAscii( ":</strong> <a href=\"" );
        aStr += StringToURL(maHomePage);
        aStr.AppendAscii( "\">" );
        aStr += StringToHTMLString(maHomePage);
        aStr.AppendAscii( "</a> </p>\r\n" );
    }

    if(maInfo.Len())
    {
        aStr.AppendAscii( "<p><strong>" );
        aStr += RESTOHTML(STR_HTMLEXP_INFO);
        aStr.AppendAscii( ":</strong><br>\r\n" );
        aStr += StringToHTMLString(maInfo);
        aStr.AppendAscii( "</p>\r\n" );
    }

    if(mbDownload)
    {
        aStr.AppendAscii( "<p><a href=\"" );
        aStr += StringToURL(maDocFileName);
        aStr.AppendAscii( "\">" );
        aStr += RESTOHTML(STR_HTMLEXP_DOWNLOAD);
        aStr.AppendAscii( "</a></p>\r\n" );
    }

    for(sal_uInt16 nSdPage = 0; nSdPage < mnSdPageCount; nSdPage++)
    {
        String aText;

        aText.AppendAscii( "<img src=\"" );
        aText += StringToURL( *mpThumbnailFiles[nSdPage] );
        aText.AppendAscii( "\" width=\"256\" height=\"192\" alt=\"" );
        aText += StringToHTMLString( *mpPageNames[nSdPage] );
        aText.AppendAscii( "\">" );

        aStr += CreateLink(*mpHTMLFiles[nSdPage], aText);
        aStr.AppendAscii( "\r\n" );
    }


    aStr.AppendAscii( "</td></tr></table></center>\r\n" );

    aStr.AppendAscii( "</body>\r\n</html>" );

    bool bOk = WriteHtml( maIndex, false, aStr );

    if (mpProgress)
        mpProgress->SetState(++mnPagesWritten);

    return bOk;
}

// =====================================================================
// Notiz Seiten erzeugen (fuer Frames)
// =====================================================================
bool HtmlExport::CreateNotesPages()
{
    bool bOk = true;

    SdrOutliner* pOutliner = mpDoc->GetInternalOutliner();
    for( sal_uInt16 nSdPage = 0; bOk && nSdPage < mnSdPageCount; nSdPage++ )
    {
        SdPage* pPage = maNotesPages[nSdPage];
        if( mbDocColors )
            SetDocColors( pPage );

        // Html Kopf
        String aStr(maHTMLHeader);
        aStr += WriteMetaCharset();
        aStr.AppendAscii( "  <title>" );
        aStr += StringToHTMLString(*mpPageNames[0]);
        aStr.AppendAscii( "</title>\r\n</head>\r\n" );
        aStr += CreateBodyTag();

        if(pPage)
            aStr += CreateTextForNotesPage( pOutliner, pPage, true, maBackColor );

        aStr.AppendAscii( "</body>\r\n</html>" );

        OUString aFileName( "note" );
        aFileName += OUString::valueOf(nSdPage);
        bOk = WriteHtml( aFileName, true, aStr );



        if (mpProgress)
            mpProgress->SetState(++mnPagesWritten);
    }

    pOutliner->Clear();

    return bOk;
}

// =====================================================================
// Outline Seiten erzeugen (fuer Frames)
// =====================================================================
bool HtmlExport::CreateOutlinePages()
{
    bool bOk = true;

    if( mbDocColors )
    {
        SetDocColors();
    }

    // Seite 0 wird der zugeklappte Outline, Seite 1 der aufgeklappte
    for (sal_Int32 nPage = 0; nPage < (mbImpress?2:1) && bOk; ++nPage)
    {
        // Html Kopf
        String aStr(maHTMLHeader);
        aStr += WriteMetaCharset();
        aStr.AppendAscii( "  <title>" );
        aStr += StringToHTMLString(*mpPageNames[0]);
        aStr.AppendAscii( "</title>\r\n</head>\r\n" );
        aStr += CreateBodyTag();

        SdrOutliner* pOutliner = mpDoc->GetInternalOutliner();
        for(sal_uInt16 nSdPage = 0; nSdPage < mnSdPageCount; nSdPage++)
        {
            SdPage* pPage = maPages[ nSdPage ];

            aStr.AppendAscii( "<div align=\"left\">" );
            String aLink( RTL_CONSTASCII_USTRINGPARAM( "JavaScript:parent.NavigateAbs(" ) );
            aLink += String::CreateFromInt32(nSdPage);
            aLink.Append( sal_Unicode(')') );

            String aTitle = CreateTextForTitle(pOutliner,pPage, maBackColor);
            if(aTitle.Len() == 0)
                aTitle = *mpPageNames[nSdPage];

            aStr.AppendAscii("<p style=\"");
            aStr.Append( getParagraphStyle( pOutliner, 0 ) );
            aStr.AppendAscii("\">");
            aStr += CreateLink(aLink, aTitle);
            aStr.AppendAscii("</p>");

            if(nPage==1)
            {
                aStr += CreateTextForPage( pOutliner, pPage, false, maBackColor );
            }
            aStr.AppendAscii( "</div>\r\n" );
        }
        pOutliner->Clear();

        aStr.AppendAscii( "</body>\r\n</html>" );

        OUString aFileName( "outline" );
        aFileName += OUString::valueOf(nPage);
        bOk = WriteHtml( aFileName, true, aStr );

        if (mpProgress)
            mpProgress->SetState(++mnPagesWritten);
    }

    return bOk;
}

// =====================================================================
// Dateinamen festlegen
// =====================================================================
void HtmlExport::CreateFileNames()
{
    // Listen mit neuen Dateinamen anlegen
    mpHTMLFiles = new String*[mnSdPageCount];
    mpImageFiles = new String*[mnSdPageCount];
    mpThumbnailFiles = new String*[mnSdPageCount];
    mpPageNames = new String*[mnSdPageCount];
    mpTextFiles = new String*[mnSdPageCount];

    mbHeader = false;   // Ueberschrift auf Uebersichtsseite?

    for (sal_uInt16 nSdPage = 0; nSdPage < mnSdPageCount; nSdPage++)
    {
        String* pName;
        if(nSdPage == 0 && !mbContentsPage && !mbFrames )
            pName = new String(maIndex);
        else
        {
            pName = new String( RTL_CONSTASCII_USTRINGPARAM("img") );
            *pName += String::CreateFromInt32(nSdPage);
            *pName += maHTMLExtension;
        }

        mpHTMLFiles[nSdPage] = pName;

        pName = new String( RTL_CONSTASCII_USTRINGPARAM("img") );
        *pName += String::CreateFromInt32(nSdPage);
        if( meFormat==FORMAT_GIF )
            pName->AppendAscii( ".gif" );
        else if( meFormat==FORMAT_JPG )
            pName->AppendAscii( ".jpg" );
        else
            pName->AppendAscii( ".png" );

        mpImageFiles[nSdPage] = pName;

        pName = new String( RTL_CONSTASCII_USTRINGPARAM("thumb") );
        *pName += String::CreateFromInt32(nSdPage);
        if( meFormat!=FORMAT_JPG )
            pName->AppendAscii( ".png" );
        else
            pName->AppendAscii( ".jpg" );

        mpThumbnailFiles[nSdPage] = pName;

        pName = new String( RTL_CONSTASCII_USTRINGPARAM("text"));
        *pName += String::CreateFromInt32(nSdPage);
        *pName += maHTMLExtension;
        mpTextFiles[nSdPage] = pName;

        SdPage* pSdPage = maPages[ nSdPage ];

        // get slide title from page name
        String* pPageTitle = new String();
        *pPageTitle = pSdPage->GetName();
        mpPageNames[nSdPage] = pPageTitle;
    }

    if(!mbContentsPage && mbFrames)
        maFramePage = maIndex;
    else
    {
        maFramePage.AssignAscii( "siframes" );
        maFramePage += maHTMLExtension;
    }
}

String HtmlExport::getDocumentTitle()
{
    // check for a title object in this page, if its the first
    // title it becomes this documents title for the content
    // page
    if( !mbHeader )
    {
        if(mbImpress)
        {
            // falls es ein nicht-leeres Titelobjekt gibt, dessen ersten Absatz
            // als Seitentitel benutzen
            SdPage* pSdPage = mpDoc->GetSdPage(0, PK_STANDARD);
            SdrObject* pTitleObj = pSdPage->GetPresObj(PRESOBJ_TITLE);
            if (pTitleObj && !pTitleObj->IsEmptyPresObj())
            {
                OutlinerParaObject* pParaObject = pTitleObj->GetOutlinerParaObject();
                if (pParaObject)
                {
                    const EditTextObject& rEditTextObject =
                        pParaObject->GetTextObject();
                    if (&rEditTextObject)
                    {
                        String aTest(rEditTextObject.GetText(0));
                        if (aTest.Len() > 0)
                            mDocTitle = aTest;
                    }
                }
            }

            for( sal_uInt16 i = 0; i < mDocTitle.Len(); i++ )
                if( mDocTitle.GetChar(i) == (sal_Unicode)0xff)
                    mDocTitle.SetChar(i, sal_Unicode(' ') );
        }

        if( !mDocTitle.Len() )
        {
            mDocTitle = maDocFileName;
            int nDot = mDocTitle.Search( '.' );
            if( nDot > 0 )
                mDocTitle.Erase( (sal_uInt16)nDot );
        }
        mbHeader = true;
    }

    return mDocTitle;
}

static const char* JS_NavigateAbs =
    "function NavigateAbs( nPage )\r\n"
    "{\r\n"
    "  frames[\"show\"].location.href = \"img\" + nPage + \".$EXT\";\r\n"
    "  //frames[\"notes\"].location.href = \"note\" + nPage + \".$EXT\";\r\n"
    "  nCurrentPage = nPage;\r\n"
    "  if(nCurrentPage==0)\r\n"
    "  {\r\n"
    "    frames[\"navbar1\"].location.href = \"navbar0.$EXT\";\r\n"
    "  }\r\n"
    "  else if(nCurrentPage==nPageCount-1)\r\n"
    "  {\r\n"
    "    frames[\"navbar1\"].location.href = \"navbar2.$EXT\";\r\n"
    "  }\r\n"
    "  else\r\n"
    "  {\r\n"
    "    frames[\"navbar1\"].location.href = \"navbar1.$EXT\";\r\n"
    "  }\r\n"
    "}\r\n\r\n";

static const char* JS_NavigateRel =
    "function NavigateRel( nDelta )\r\n"
    "{\r\n"
    "  var nPage = parseInt(nCurrentPage) + parseInt(nDelta);\r\n"
    "  if( (nPage >= 0) && (nPage < nPageCount) )\r\n"
    "  {\r\n"
    "    NavigateAbs( nPage );\r\n"
    "  }\r\n"
    "}\r\n\r\n";

static const char* JS_ExpandOutline =
    "function ExpandOutline()\r\n"
    "{\r\n"
    "  frames[\"navbar2\"].location.href = \"navbar4.$EXT\";\r\n"
    "  frames[\"outline\"].location.href = \"outline1.$EXT\";\r\n"
    "}\r\n\r\n";

static const char * JS_CollapseOutline =
    "function CollapseOutline()\r\n"
    "{\r\n"
    "  frames[\"navbar2\"].location.href = \"navbar3.$EXT\";\r\n"
    "  frames[\"outline\"].location.href = \"outline0.$EXT\";\r\n"
    "}\r\n\r\n";

// ====================================================================
// Seite mit den Frames erzeugen
// ====================================================================
bool HtmlExport::CreateFrames()
{
    String aTmp;
    String aStr( RTL_CONSTASCII_USTRINGPARAM(
                    "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Frameset//EN\"\r\n"
                    "    \"http://www.w3.org/TR/html4/frameset.dtd\">\r\n"
            "<html>\r\n<head>\r\n" ) );

    aStr += WriteMetaCharset();
    aStr.AppendAscii( "  <title>" );
    aStr += StringToHTMLString(*mpPageNames[0]);
    aStr.AppendAscii( "</title>\r\n" );

    aStr.AppendAscii( "<script type=\"text/javascript\">\r\n<!--\r\n" );

    aStr.AppendAscii( "var nCurrentPage = 0;\r\nvar nPageCount = " );
    aStr += String::CreateFromInt32(mnSdPageCount);
    aStr.AppendAscii( ";\r\n\r\n" );

    String aFunction;
    aFunction.AssignAscii(JS_NavigateAbs);

    if(mbNotes)
    {
        String aEmpty;
        String aSlash( RTL_CONSTASCII_USTRINGPARAM( "//" ) );
        aFunction.SearchAndReplaceAll( aSlash, aEmpty);
    }

    // substitute HTML file extension
    String aPlaceHolder(RTL_CONSTASCII_USTRINGPARAM(".$EXT"));
    aFunction.SearchAndReplaceAll(aPlaceHolder, maHTMLExtension);
    aStr += aFunction;

    aTmp.AssignAscii( JS_NavigateRel );
    aTmp.SearchAndReplaceAll(aPlaceHolder, maHTMLExtension);
    aStr += aTmp;

    if(mbImpress)
    {
        aTmp.AssignAscii( JS_ExpandOutline );
        aTmp.SearchAndReplaceAll(aPlaceHolder, maHTMLExtension);
        aStr += aTmp;

        aTmp.AssignAscii( JS_CollapseOutline );
        aTmp.SearchAndReplaceAll(aPlaceHolder, maHTMLExtension);
        aStr += aTmp;
    }
    aStr.AppendAscii( "// -->\r\n</script>\r\n" );

    aStr.AppendAscii( "</head>\r\n" );

    aStr.AppendAscii( "<frameset cols=\"*," );
    aStr += String::CreateFromInt32((mnWidthPixel + 16));
    aStr.AppendAscii( "\">\r\n" );
    if(mbImpress)
    {
        aStr.AppendAscii( "  <frameset rows=\"42,*\">\r\n" );
        aStr.AppendAscii( "    <frame src=\"navbar3" );
        aStr += StringToURL(maHTMLExtension);
        aStr.AppendAscii( "\" name=\"navbar2\" marginwidth=\"4\" marginheight=\"4\" scrolling=\"no\">\r\n" );
    }
    aStr.AppendAscii( "    <frame src=\"outline0" );
    aStr += StringToURL(maHTMLExtension);
    aStr.AppendAscii( "\" name=\"outline\">\r\n" );
    if(mbImpress)
        aStr.AppendAscii( "  </frameset>\r\n" );

    if(mbNotes)
    {
        aStr.AppendAscii( "  <frameset rows=\"42," );
        aStr += String::CreateFromInt32((int)((double)mnWidthPixel * 0.75) + 16);
        aStr.AppendAscii( ",*\">\r\n" );
    }
    else
        aStr.AppendAscii( "  <frameset rows=\"42,*\">\r\n" );

    aStr.AppendAscii( "    <frame src=\"navbar0" );
    aStr += StringToURL(maHTMLExtension);
    aStr.AppendAscii( "\" name=\"navbar1\" marginwidth=\"4\" marginheight=\"4\" scrolling=\"no\">\r\n" );

    aStr.AppendAscii( "    <frame src=\"" );
    aStr += StringToURL(*mpHTMLFiles[0]);
    aStr.AppendAscii( "\" name=\"show\" marginwidth=\"4\" marginheight=\"4\">\r\n" );

    if(mbNotes)
    {
        aStr.AppendAscii( "    <frame src=\"note0" );
        aStr += StringToURL(maHTMLExtension);
        aStr.AppendAscii( "\" name=\"notes\">\r\n" );
    }
    aStr.AppendAscii( "  </frameset>\r\n" );

    aStr.AppendAscii( "<noframes>\r\n" );
    aStr += CreateBodyTag();
    aStr += RESTOHTML(STR_HTMLEXP_NOFRAMES);
    aStr.AppendAscii( "\r\n</noframes>\r\n</frameset>\r\n</html>" );

    bool bOk = WriteHtml( maFramePage, false, aStr );

    if (mpProgress)
        mpProgress->SetState(++mnPagesWritten);

    return bOk;
}

// ====================================================================
// Buttonleiste fuer Standard ausgeben
// Es werden 4 html files erstellt
// navbar0.htm Navigationsleiste Grafik fuer erste Seite
// navbar1.htm Navigationsleiste Grafik fuer zweite bis vorletzte Seite
// navbar2.htm Navigationsleiste Grafik fuer letzte Seite
// navbar3.htm Navigationsleiste Outline zugeklappt
// navbar4.htm Navigationsleiste Outline aufgeklappt
// ====================================================================
bool HtmlExport::CreateNavBarFrames()
{
    bool bOk = true;
    String aButton;

    if( mbDocColors )
    {
        SetDocColors();
        maBackColor = maFirstPageColor;
    }

    for( int nFile = 0; nFile < 3 && bOk; nFile++ )
    {
        String aStr(maHTMLHeader);
        aStr += WriteMetaCharset();
        aStr.AppendAscii( "  <title>" );
        aStr += StringToHTMLString(*mpPageNames[0]);
        aStr.AppendAscii( "</title>\r\n</head>\r\n" );
        aStr += CreateBodyTag();
        aStr.AppendAscii( "<center>\r\n" );

    // erste Seite
        aButton = String(SdResId(STR_HTMLEXP_FIRSTPAGE));
        if(mnButtonThema != -1)
            aButton = CreateImage(GetButtonName((nFile == 0 || mnSdPageCount == 1?
                                  BTN_FIRST_0:BTN_FIRST_1)), aButton);

        if(nFile != 0 && mnSdPageCount > 1)
            aButton = CreateLink( String(RTL_CONSTASCII_USTRINGPARAM("JavaScript:parent.NavigateAbs(0)")), aButton);

        aStr += aButton;
        aStr.AppendAscii( "\r\n" );

    // zur vorherigen Seite
        aButton = String(SdResId(STR_PUBLISH_BACK));
        if(mnButtonThema != -1)
            aButton = CreateImage(GetButtonName((nFile == 0 || mnSdPageCount == 1?
                                    BTN_PREV_0:BTN_PREV_1)), aButton);

        if(nFile != 0 && mnSdPageCount > 1)
            aButton = CreateLink( String(RTL_CONSTASCII_USTRINGPARAM("JavaScript:parent.NavigateRel(-1)")), aButton);

        aStr += aButton;
        aStr.AppendAscii( "\r\n" );

    // zur naechsten Seite
        aButton = String(SdResId(STR_PUBLISH_NEXT));
        if(mnButtonThema != -1)
            aButton = CreateImage(GetButtonName((nFile ==2 || mnSdPageCount == 1?
                                    BTN_NEXT_0:BTN_NEXT_1)), aButton);

        if(nFile != 2 && mnSdPageCount > 1)
            aButton = CreateLink(String(RTL_CONSTASCII_USTRINGPARAM("JavaScript:parent.NavigateRel(1)")), aButton);

        aStr += aButton;
        aStr.AppendAscii( "\r\n" );

    // zur letzten Seite
        aButton = String(SdResId(STR_HTMLEXP_LASTPAGE));
        if(mnButtonThema != -1)
            aButton = CreateImage(GetButtonName((nFile ==2 || mnSdPageCount == 1?
                                  BTN_LAST_0:BTN_LAST_1)), aButton);

        if(nFile != 2 && mnSdPageCount > 1)
        {
            String aLink(RTL_CONSTASCII_USTRINGPARAM("JavaScript:parent.NavigateAbs("));
            aLink += String::CreateFromInt32(mnSdPageCount-1);
            aLink.AppendAscii( ")" );
            aButton = CreateLink( aLink, aButton);
        }

        aStr += aButton;
        aStr.AppendAscii( "\r\n" );

    // Inhalt
        if (mbContentsPage)
        {
            aButton = String(SdResId(STR_PUBLISH_OUTLINE));
            if(mnButtonThema != -1)
                aButton = CreateImage(GetButtonName(BTN_INDEX), aButton);

            // zur Uebersicht
            aStr += CreateLink(maIndex, aButton, String(RTL_CONSTASCII_USTRINGPARAM("_top")));
            aStr.AppendAscii( "\r\n" );
        }

    // Textmodus
        if(mbImpress)
        {
            aButton = String(SdResId(STR_HTMLEXP_SETTEXT));
            if(mnButtonThema != -1)
                aButton = CreateImage(GetButtonName(BTN_TEXT), aButton);

            String  aText0( RTL_CONSTASCII_USTRINGPARAM("text0"));
            aText0 += maHTMLExtension;
            aStr += CreateLink( aText0, aButton, String(RTL_CONSTASCII_USTRINGPARAM("_top")));
            aStr.AppendAscii( "\r\n" );
        }

    // Und fertich...
        aStr.AppendAscii( "</center>\r\n" );
        aStr.AppendAscii( "</body>\r\n</html>" );

        String aFileName( RTL_CONSTASCII_USTRINGPARAM("navbar") );
        aFileName += String::CreateFromInt32(nFile);

        bOk = WriteHtml( aFileName, true, aStr );

        if (mpProgress)
            mpProgress->SetState(++mnPagesWritten);
    }

    // Jetzt kommt die Navigatonsleiste Outliner zugeklappt...
    if(bOk)
    {
        String aStr(maHTMLHeader);
        aStr += WriteMetaCharset();
        aStr.AppendAscii( "  <title>" );
        aStr += StringToHTMLString(*mpPageNames[0]);
        aStr.AppendAscii( "</title>\r\n</head>\r\n" );
        aStr += CreateBodyTag();

        aButton = String(SdResId(STR_HTMLEXP_OUTLINE));
        if(mnButtonThema != -1)
            aButton = CreateImage(GetButtonName(BTN_MORE), aButton);

        aStr += CreateLink(String(RTL_CONSTASCII_USTRINGPARAM("JavaScript:parent.ExpandOutline()")), aButton);
        aStr.AppendAscii( "</body>\r\n</html>" );

        String aFileName( RTL_CONSTASCII_USTRINGPARAM("navbar3") );

        bOk = WriteHtml( aFileName, true, aStr );

        if (mpProgress)
            mpProgress->SetState(++mnPagesWritten);
    }

    // ... und jetzt Outliner aufgeklappt
    if( bOk )
    {
        String aStr(maHTMLHeader);
        aStr += WriteMetaCharset();
        aStr.AppendAscii( "  <title>" );
        aStr += StringToHTMLString(*mpPageNames[0]);
        aStr.AppendAscii( "</title>\r\n</head>\r\n" );
        aStr += CreateBodyTag();

        aButton = String(SdResId(STR_HTMLEXP_NOOUTLINE));
        if(mnButtonThema != -1)
            aButton = CreateImage(GetButtonName(BTN_LESS), aButton);

        aStr += CreateLink(String(RTL_CONSTASCII_USTRINGPARAM("JavaScript:parent.CollapseOutline()")), aButton);
        aStr.AppendAscii( "</body>\r\n</html>" );

        String aFileName( RTL_CONSTASCII_USTRINGPARAM("navbar4") );
        bOk = WriteHtml( aFileName, true, aStr );

        if (mpProgress)
            mpProgress->SetState(++mnPagesWritten);

    }

    return bOk;
}

// ====================================================================
// Buttonleiste fuer Standard ausgeben
// ====================================================================
String HtmlExport::CreateNavBar( sal_uInt16 nSdPage, bool bIsText ) const
{
    // Navigationsleiste vorbereiten
    String aStrNavFirst( SdResId(STR_HTMLEXP_FIRSTPAGE) );
    String aStrNavPrev( SdResId(STR_PUBLISH_BACK) );
    String aStrNavNext( SdResId(STR_PUBLISH_NEXT) );
    String aStrNavLast( SdResId(STR_HTMLEXP_LASTPAGE) );
    String aStrNavContent( SdResId(STR_PUBLISH_OUTLINE) );
    String aStrNavText;
    if( bIsText )
    {
        aStrNavText = String( SdResId(STR_HTMLEXP_SETGRAPHIC) );
    }
    else
    {
        aStrNavText = String( SdResId(STR_HTMLEXP_SETTEXT) );
    }

    if(!bIsText && mnButtonThema != -1)
    {
        if(nSdPage<1 || mnSdPageCount == 1)
        {
            aStrNavFirst = CreateImage(GetButtonName(BTN_FIRST_0), aStrNavFirst);
            aStrNavPrev  = CreateImage(GetButtonName(BTN_PREV_0), aStrNavPrev);
        }
        else
        {
            aStrNavFirst = CreateImage(GetButtonName(BTN_FIRST_1), aStrNavFirst);
            aStrNavPrev  = CreateImage(GetButtonName(BTN_PREV_1), aStrNavPrev);
        }

        if(nSdPage == mnSdPageCount-1 || mnSdPageCount == 1)
        {
            aStrNavNext    = CreateImage(GetButtonName(BTN_NEXT_0), aStrNavNext);
            aStrNavLast    = CreateImage(GetButtonName(BTN_LAST_0), aStrNavLast);
        }
        else
        {
            aStrNavNext    = CreateImage(GetButtonName(BTN_NEXT_1), aStrNavNext);
            aStrNavLast    = CreateImage(GetButtonName(BTN_LAST_1), aStrNavLast);
        }

        aStrNavContent = CreateImage(GetButtonName(BTN_INDEX), aStrNavContent);
        aStrNavText    = CreateImage(GetButtonName(BTN_TEXT), aStrNavText);
    }

    String aStr( RTL_CONSTASCII_USTRINGPARAM("<center>\r\n")); //<table><tr>\r\n");

    // erste Seite
    if(nSdPage > 0)
        aStr += CreateLink(bIsText?*mpTextFiles[0]:*mpHTMLFiles[0],aStrNavFirst);
    else
        aStr += aStrNavFirst;
    aStr.Append(sal_Unicode(' '));

    // to Previous page
    if(nSdPage > 0)
        aStr += CreateLink( bIsText?*mpTextFiles[nSdPage-1]:
                                    *mpHTMLFiles[nSdPage-1],    aStrNavPrev);
    else
        aStr += aStrNavPrev;
    aStr.Append(sal_Unicode(' '));

    // to Next page
    if(nSdPage < mnSdPageCount-1)
        aStr += CreateLink( bIsText?*mpTextFiles[nSdPage+1]:
                                    *mpHTMLFiles[nSdPage+1], aStrNavNext);
    else
        aStr += aStrNavNext;
    aStr.Append(sal_Unicode(' '));

    // to Last page
    if(nSdPage < mnSdPageCount-1)
        aStr += CreateLink( bIsText?*mpTextFiles[mnSdPageCount-1]:
                                    *mpHTMLFiles[mnSdPageCount-1],
                                    aStrNavLast );
    else
        aStr += aStrNavLast;
    aStr.Append(sal_Unicode(' '));

    // to Index page
    if (mbContentsPage)
    {
        aStr += CreateLink(maIndex, aStrNavContent);
        aStr.Append(sal_Unicode(' '));
    }

    // Text/Graphics
    if(mbImpress)
    {
        aStr += CreateLink( bIsText?(mbFrames?maFramePage:*mpHTMLFiles[nSdPage]):
                                    *mpTextFiles[nSdPage], aStrNavText);

    }

    aStr.AppendAscii( "</center><br>\r\n" );

    return aStr;
}

/** export navigation graphics from button set */
bool HtmlExport::CreateBitmaps()
{
    if(mnButtonThema != -1 && mpButtonSet.get() )
    {
        for( int nButton = 0; nButton < NUM_BUTTONS; nButton++ )
        {
            if(!mbFrames && (nButton == BTN_MORE || nButton == BTN_LESS))
                continue;

            if(!mbImpress && (nButton == BTN_TEXT || nButton == BTN_MORE || nButton == BTN_LESS ))
                continue;

            OUString aFull(maExportPath);
            aFull += GetButtonName(nButton);
            mpButtonSet->exportButton( mnButtonThema, aFull, GetButtonName(nButton) );
        }
    }
    return true;
}

// =====================================================================
// Erzeugt den <body> Tag, inkl. der eingestellten Farbattribute
// =====================================================================
String HtmlExport::CreateBodyTag() const
{
    String aStr( RTL_CONSTASCII_USTRINGPARAM("<body") );

    if( mbUserAttr || mbDocColors )
    {
        Color aTextColor( maTextColor );
        if( (aTextColor == COL_AUTO) && (!maBackColor.IsDark()) )
            aTextColor = COL_BLACK;

        aStr.AppendAscii( " text=\"" );
        aStr += ColorToHTMLString( aTextColor );
        aStr.AppendAscii( "\" bgcolor=\"" );
        aStr += ColorToHTMLString( maBackColor );
        aStr.AppendAscii( "\" link=\"" );
        aStr += ColorToHTMLString( maLinkColor );
        aStr.AppendAscii( "\" vlink=\"" );
        aStr += ColorToHTMLString( maVLinkColor );
        aStr.AppendAscii( "\" alink=\"" );
        aStr += ColorToHTMLString( maALinkColor );
        aStr.AppendAscii( "\"" );
    }

    aStr.AppendAscii( ">\r\n" );

    return aStr;
}

// =====================================================================
// Erzeugt einen Hyperlink
// =====================================================================
String HtmlExport::CreateLink( const String& aLink,
                               const String& aText,
                               const String& aTarget ) const
{
    String aStr( RTL_CONSTASCII_USTRINGPARAM("<a href=\""));
    aStr += StringToURL(aLink);
    if(aTarget.Len())
    {
        aStr.AppendAscii( "\" target=\"" );
        aStr += aTarget;
    }
    aStr.AppendAscii( "\">" );
    aStr += aText;
    aStr.AppendAscii( "</a>" );

    return aStr;
}

// =====================================================================
// Erzeugt ein Image-tag
// =====================================================================
String HtmlExport::CreateImage( const String& aImage, const String& aAltText,
                                sal_Int16 nWidth,
                                sal_Int16 nHeight ) const
{
    String aStr( RTL_CONSTASCII_USTRINGPARAM("<img src=\""));
    aStr += StringToURL(aImage);
    aStr.AppendAscii( "\" border=0" );

    if( aAltText.Len())
    {
        aStr.AppendAscii( " alt=\"" );
        aStr += aAltText;
        aStr.Append(sal_Unicode('"'));
    }
    else
    {
        // Agerskov: HTML 4.01 has to have an alt attribut even if it is an empty string
        aStr.AppendAscii( " alt=\"\"" );
     }

    if(nWidth > -1)
    {
        aStr.AppendAscii( " width=" );
        aStr += String::CreateFromInt32(nWidth);
    }

    if(nHeight > -1)
    {
        aStr.AppendAscii( " height=" );
        aStr += String::CreateFromInt32(nHeight);
    }

    aStr.Append(sal_Unicode('>'));

    return aStr;
}

// =====================================================================
// Area fuer Kreis erzeugen; es werden Pixelkoordinaten erwartet
// =====================================================================
String HtmlExport::ColorToHTMLString( Color aColor )
{
    static char hex[] = "0123456789ABCDEF";
    String aStr( RTL_CONSTASCII_USTRINGPARAM("#xxxxxx"));
    aStr.SetChar(1, hex[(aColor.GetRed() >> 4) & 0xf] );
    aStr.SetChar(2, hex[aColor.GetRed()   & 0xf] );
    aStr.SetChar(3, hex[(aColor.GetGreen() >> 4) & 0xf] );
    aStr.SetChar(4, hex[aColor.GetGreen() & 0xf] );
    aStr.SetChar(5, hex[(aColor.GetBlue() >> 4) & 0xf] );
    aStr.SetChar(6, hex[aColor.GetBlue()  & 0xf] );

    return aStr;
}

// =====================================================================
// Area fuer Kreis erzeugen; es werden Pixelkoordinaten erwartet
// =====================================================================
String HtmlExport::CreateHTMLCircleArea( sal_uLong nRadius,
                                         sal_uLong nCenterX,
                                         sal_uLong nCenterY,
                                         const String& rHRef ) const
{
    String aStr( RTL_CONSTASCII_USTRINGPARAM("<area shape=\"circle\" alt=\"\" coords=\"" ));

    aStr += String::CreateFromInt32(nCenterX);
    aStr.Append(sal_Unicode(','));
    aStr += String::CreateFromInt32(nCenterY);
    aStr.Append(sal_Unicode(','));
    aStr += String::CreateFromInt32(nRadius);
    aStr.AppendAscii( "\" href=\"" );
    aStr += StringToURL(rHRef);
    aStr.AppendAscii( "\">\n" );

    return aStr;
}


// =====================================================================
// Area fuer Polygon erzeugen; es werden Pixelkoordinaten erwartet
// =====================================================================
String HtmlExport::CreateHTMLPolygonArea( const ::basegfx::B2DPolyPolygon& rPolyPolygon,
    Size aShift, double fFactor, const String& rHRef ) const
{
    String          aStr;
    const sal_uInt32 nNoOfPolygons(rPolyPolygon.count());

    for ( sal_uInt32 nXPoly = 0L; nXPoly < nNoOfPolygons; nXPoly++ )
    {
        const ::basegfx::B2DPolygon& aPolygon = rPolyPolygon.getB2DPolygon(nXPoly);
        const sal_uInt32 nNoOfPoints(aPolygon.count());

        aStr.AppendAscii( "<area shape=\"polygon\" alt=\"\" coords=\"" );

        for ( sal_uInt32 nPoint = 0L; nPoint < nNoOfPoints; nPoint++ )
        {
            const ::basegfx::B2DPoint aB2DPoint(aPolygon.getB2DPoint(nPoint));
            Point aPnt(FRound(aB2DPoint.getX()), FRound(aB2DPoint.getY()));
            // das Koordinaten beziehen sich auf den
            // physikalischen Seitenursprung, nicht auf den
            // Koordinatenursprung
            aPnt.Move(aShift.Width(), aShift.Height());

            aPnt.X() = (long)(aPnt.X() * fFactor);
            aPnt.Y() = (long)(aPnt.Y() * fFactor);
            aStr += String::CreateFromInt32(aPnt.X());
            aStr.Append(sal_Unicode(','));
            aStr += String::CreateFromInt32(aPnt.Y());

            if (nPoint < nNoOfPoints - 1)
                aStr.Append( sal_Unicode(',') );
        }
        aStr.AppendAscii( "\" href=\"" );
        aStr += StringToURL(rHRef);
        aStr.AppendAscii( "\">\n" );
    }

    return aStr;
}

// =====================================================================
// Area fuer Rechteck erzeugen; es werden Pixelkoordinaten erwartet
// =====================================================================
String HtmlExport::CreateHTMLRectArea( const Rectangle& rRect,
                                       const String& rHRef ) const
{
    String aStr( RTL_CONSTASCII_USTRINGPARAM("<area shape=\"rect\" alt=\"\" coords=\"") );

    aStr += String::CreateFromInt32(rRect.Left());
    aStr.Append(sal_Unicode(','));
    aStr += String::CreateFromInt32(rRect.Top());
    aStr.Append(sal_Unicode(','));
    aStr += String::CreateFromInt32(rRect.Right());
    aStr.Append(sal_Unicode(','));
    aStr += String::CreateFromInt32(rRect.Bottom());
    aStr.AppendAscii( "\" href=\"" );
    aStr += StringToURL(rHRef);
    aStr.AppendAscii( "\">\n" );

    return aStr;
}

// =====================================================================
// StringToHTMLString, konvertiert einen String in
// seine HTML-Repraesentation (Umlaute etc.)
// =====================================================================
String HtmlExport::StringToHTMLString( const String& rString )
{
    SvMemoryStream aMemStm;
    HTMLOutFuncs::Out_String( aMemStm, rString, RTL_TEXTENCODING_UTF8 );
    aMemStm << (char) 0;
    return String( (char*)aMemStm.GetData(), RTL_TEXTENCODING_UTF8 );
}

// =====================================================================
// Erzeugt die URL einer bestimmten Seite
// =====================================================================
String HtmlExport::CreatePageURL( sal_uInt16 nPgNum )
{
    if(mbFrames)
    {
        String aUrl( RTL_CONSTASCII_USTRINGPARAM("JavaScript:parent.NavigateAbs("));
        aUrl += String::CreateFromInt32(nPgNum);
        aUrl.Append(sal_Unicode(')'));
        return aUrl;
    }
    else
        return *mpHTMLFiles[nPgNum];
}

bool HtmlExport::CopyScript( const String& rPath, const String& rSource, const String& rDest, bool bUnix /* = false */ )
{
    INetURLObject   aURL( SvtPathOptions().GetConfigPath() );
    String      aScript;

    aURL.Append( rtl::OUString("webcast") );
    aURL.Append( rSource );

    meEC.SetContext( STR_HTMLEXP_ERROR_OPEN_FILE, rSource );

    sal_uLong       nErr = 0;
    SvStream*   pIStm = ::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ );

    if( pIStm )
    {
        rtl::OString aLine;

        while( pIStm->ReadLine( aLine ) )
        {
            aScript.AppendAscii( aLine.getStr() );
            if( bUnix )
            {
                aScript.AppendAscii( "\n" );
            }
            else
            {
                aScript.AppendAscii( "\r\n" );
            }
        }

        nErr = pIStm->GetError();
        delete pIStm;
    }

    if( nErr != 0 )
    {
        ErrorHandler::HandleError( nErr );
        return (bool) nErr;
    }


    aScript.SearchAndReplaceAll( String(RTL_CONSTASCII_USTRINGPARAM("$$1")), getDocumentTitle() );

    const String aSaveStr( RESTOHTML( STR_WEBVIEW_SAVE ));
    aScript.SearchAndReplaceAll( String(RTL_CONSTASCII_USTRINGPARAM("$$2")), aSaveStr );

    aScript.SearchAndReplaceAll( String(RTL_CONSTASCII_USTRINGPARAM("$$3")), maCGIPath );

    aScript.SearchAndReplaceAll( String(RTL_CONSTASCII_USTRINGPARAM("$$4")), String::CreateFromInt32(mnWidthPixel) );
    aScript.SearchAndReplaceAll( String(RTL_CONSTASCII_USTRINGPARAM("$$5")), String::CreateFromInt32(mnHeightPixel) );


    String aDest( rPath );
    aDest += rDest;

    meEC.SetContext( STR_HTMLEXP_ERROR_CREATE_FILE, rDest );
    // write script file
    {
        EasyFile aFile;
        SvStream* pStr;
        nErr = aFile.createStream(aDest, pStr);
        if(nErr == 0)
        {
            rtl::OString aStr(rtl::OUStringToOString(aScript,
                RTL_TEXTENCODING_UTF8));
            *pStr << aStr.getStr();

            nErr = aFile.close();
        }
    }

    if (mpProgress)
        mpProgress->SetState(++mnPagesWritten);

    if( nErr != 0 )
        ErrorHandler::HandleError( nErr );

    return nErr == 0;
}

static const char * ASP_Scripts[] = { "common.inc", "webcast.asp", "show.asp", "savepic.asp", "poll.asp", "editpic.asp" };

/** erzeugt und speichert die f�r WebShow ben�tigte ASP Scripte */
bool HtmlExport::CreateASPScripts()
{
    for( sal_uInt16 n = 0; n < (sizeof( ASP_Scripts ) / sizeof(char *)); n++ )
    {
        String aScript;

        aScript.AssignAscii( ASP_Scripts[n] );
        if(!CopyScript(maExportPath, aScript, aScript))
            return false;
    }

    if(!CopyScript(maExportPath, String(RTL_CONSTASCII_USTRINGPARAM("edit.asp")), maIndex ))
        return false;

    return true;
}


static const char *PERL_Scripts[] = { "webcast.pl", "common.pl", "editpic.pl", "poll.pl", "savepic.pl", "show.pl" };

/** erzeugt und speichert die f�r WebShow ben�tigte PERL Scripte */
bool HtmlExport::CreatePERLScripts()
{
    for( sal_uInt16 n = 0; n < (sizeof( PERL_Scripts ) / sizeof(char *)); n++ )
    {
        String aScript;
        aScript.AssignAscii( PERL_Scripts[n] );
        if(!CopyScript(maExportPath, aScript, aScript, true))
            return false;
    }

    if(!CopyScript(maExportPath, rtl::OUString("edit.pl"), maIndex, true ))
        return false;

    if(!CopyScript(maExportPath, rtl::OUString("index.pl"), maIndexUrl, true ))
        return false;

    return true;
}

/** Erzeugt eine Liste mit den Namen der gespeicherten Images */
bool HtmlExport::CreateImageFileList()
{
    String aStr;
    for( sal_uInt16 nSdPage = 0; nSdPage < mnSdPageCount; nSdPage++)
    {
        aStr += String::CreateFromInt32( nSdPage + 1 );
        aStr.Append(sal_Unicode(';'));
        aStr += maURLPath;
        aStr += *mpImageFiles[nSdPage];
        aStr.AppendAscii( "\r\n" );
    }

    String aFileName( RTL_CONSTASCII_USTRINGPARAM("picture.txt") );
    bool bOk = WriteHtml( aFileName, false, aStr );

    if (mpProgress)
        mpProgress->SetState(++mnPagesWritten);

    return bOk;
}

/** Erzeugt das File mit der aktuellen Seitennumer */
bool HtmlExport::CreateImageNumberFile()
{
    String aFull( maExportPath );
    String aFileName( RTL_CONSTASCII_USTRINGPARAM("currpic.txt") );
    aFull += aFileName;

    meEC.SetContext( STR_HTMLEXP_ERROR_CREATE_FILE, aFileName );
    EasyFile aFile;
    SvStream* pStr;
    sal_uLong nErr = aFile.createStream(aFull, pStr);
    if(nErr == 0)
    {
        *pStr << (const char *)"1";
        nErr = aFile.close();
    }

    if (mpProgress)
        mpProgress->SetState(++mnPagesWritten);

    if( nErr != 0 )
        ErrorHandler::HandleError( nErr );

    return nErr == 0;
}

// =====================================================================

String HtmlExport::InsertSound( const String& rSoundFile )
{
    if( rSoundFile.Len() == 0 )
        return rSoundFile;

    String      aStr( RTL_CONSTASCII_USTRINGPARAM("<embed src=\"") );
    INetURLObject   aURL( rSoundFile );

    DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

    aStr += String(aURL.getName());
    aStr.AppendAscii( "\" hidden=\"true\" autostart=\"true\">" );

    CopyFile( rSoundFile, maExportPath );

    return aStr;
}

// =====================================================================

bool HtmlExport::CopyFile( const String& rSourceFile, const String& rDestPath )
{
    DirEntry aSourceEntry( rSourceFile );
    DirEntry aDestEntry( rDestPath );

    meEC.SetContext( STR_HTMLEXP_ERROR_COPY_FILE, aSourceEntry.GetName(), rDestPath );
    FSysError nError = aSourceEntry.CopyTo( aDestEntry, FSYS_ACTION_COPYFILE );

    if( nError != FSYS_ERR_OK )
    {
        ErrorHandler::HandleError(nError);
        return false;
    }
    else
    {
        return true;
    }
}

// =====================================================================

bool HtmlExport::checkFileExists( Reference< ::com::sun::star::ucb::XSimpleFileAccess2 >& xFileAccess, String const & aFileName )
{
    try
    {
        OUString url( maExportPath );
        url += aFileName;
        return xFileAccess->exists( url );
    }
    catch( com::sun::star::uno::Exception& )
    {
        OSL_FAIL((OString("sd::HtmlExport::checkFileExists(), exception caught: ") +
             rtl::OUStringToOString( comphelper::anyToString( cppu::getCaughtException() ), RTL_TEXTENCODING_UTF8 )).getStr() );
    }

    return false;
}

// ---------------------------------------------------------------------

bool HtmlExport::checkForExistingFiles()
{
    bool bFound = false;

    try
    {
        Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
        uno::Reference<ucb::XSimpleFileAccess2> xFA(ucb::SimpleFileAccess::create(xContext));

        sal_uInt16 nSdPage;
        for( nSdPage = 0; !bFound && (nSdPage < mnSdPageCount); nSdPage++)
        {
            if( (mpImageFiles[nSdPage] && checkFileExists( xFA, *mpImageFiles[nSdPage] )) ||
                (mpHTMLFiles[nSdPage] && checkFileExists( xFA, *mpHTMLFiles[nSdPage] )) ||
                (mpThumbnailFiles[nSdPage] && checkFileExists( xFA, *mpThumbnailFiles[nSdPage] )) ||
                (mpPageNames[nSdPage] && checkFileExists( xFA, *mpPageNames[nSdPage] )) ||
                (mpTextFiles[nSdPage] && checkFileExists( xFA, *mpTextFiles[nSdPage] )) )
            {
                bFound = true;
            }
        }

        if( !bFound && mbDownload )
            bFound = checkFileExists( xFA, maDocFileName );

        if( !bFound && mbFrames )
            bFound = checkFileExists( xFA, maFramePage );

        if( bFound )
        {
            ResMgr *pResMgr = CREATERESMGR( dbw );
            if( pResMgr )
            {
                ResId aResId( 4077, *pResMgr );
                String aMsg( aResId );

                OUString aSystemPath;
                osl::FileBase::getSystemPathFromFileURL( maExportPath, aSystemPath );
                aMsg.SearchAndReplaceAscii( "%FILENAME", aSystemPath );
                WarningBox aWarning( 0, WB_YES_NO | WB_DEF_YES, aMsg );
                aWarning.SetImage( WarningBox::GetStandardImage() );
                bFound = ( RET_NO == aWarning.Execute() );

                delete pResMgr;
            }
            else
            {
                bFound = false;
            }
        }
    }
    catch( Exception& )
    {
        OSL_FAIL((OString("sd::HtmlExport::checkForExistingFiles(), exception caught: ") +
             rtl::OUStringToOString( comphelper::anyToString( cppu::getCaughtException() ), RTL_TEXTENCODING_UTF8 )).getStr() );
        bFound = false;
    }

    return bFound;
}

// ---------------------------------------------------------------------

String HtmlExport::StringToURL( const String& rURL )
{
    return rURL;
}

String HtmlExport::GetButtonName( int nButton ) const
{
    String aName;
    aName.AssignAscii( pButtonNames[nButton] );
    return aName;
}

// =====================================================================
EasyFile::EasyFile()
{
    pMedium = NULL;
    pOStm = NULL;
    bOpen = false;
}

// =====================================================================
EasyFile::~EasyFile()
{
    if( bOpen )
        close();
}

// =====================================================================
sal_uLong EasyFile::createStream(  const String& rUrl, SvStream* &rpStr )
{
    sal_uLong nErr = 0;

    if(bOpen)
        nErr = close();

    String aFileName;

    if( nErr == 0 )
        nErr = createFileName( rUrl, aFileName );

    if( nErr == 0 )
    {
        pOStm = ::utl::UcbStreamHelper::CreateStream( aFileName, STREAM_WRITE | STREAM_TRUNC );
        if( pOStm )
        {
            bOpen = true;
            nErr = pOStm->GetError();
        }
        else
        {
            nErr = ERRCODE_SFX_CANTCREATECONTENT;
        }
    }

    if( nErr != 0 )
    {
        bOpen = false;
        delete pMedium;
        delete pOStm;
        pOStm = NULL;
    }

    rpStr = pOStm;

    return nErr;
}

// =====================================================================
sal_uLong EasyFile::createFileName(  const String& rURL, String& rFileName )
{
    sal_uLong nErr = 0;

    if( bOpen )
        nErr = close();

    if( nErr == 0 )
    {
        INetURLObject aURL( rURL );

        if( aURL.GetProtocol() == INET_PROT_NOT_VALID )
        {
            rtl::OUString aURLStr;
            ::utl::LocalFileHelper::ConvertPhysicalNameToURL( rURL, aURLStr );
            aURL = INetURLObject( aURLStr );
        }
        DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );
        rFileName = aURL.GetMainURL( INetURLObject::NO_DECODE );
    }

    return nErr;
}

// =====================================================================
sal_uLong EasyFile::close()
{
    sal_uLong nErr = 0;

    delete pOStm;
    pOStm = NULL;

    bOpen = false;

    if( pMedium )
    {
        // uebertragen
        pMedium->Close();
        pMedium->Commit();

        nErr = pMedium->GetError();

        delete pMedium;
        pMedium = NULL;
    }

    return nErr;
}

// =====================================================================
// This class helps reporting errors during file i/o
// =====================================================================

HtmlErrorContext::HtmlErrorContext(Window *_pWin)
: ErrorContext(_pWin)
{
    mnResId = 0;
}

// =====================================================================

sal_Bool HtmlErrorContext::GetString( sal_uLong, OUString& rCtxStr )
{
    DBG_ASSERT( mnResId != 0, "No error context set" );
    if( mnResId == 0 )
        return false;

    rCtxStr = SdResId( mnResId );

    rCtxStr = rCtxStr.replaceAll( rtl::OUString("$(URL1)"), maURL1 );
    rCtxStr = rCtxStr.replaceAll( rtl::OUString("$(URL2)"), maURL2 );

    return true;
}

// =====================================================================

void HtmlErrorContext::SetContext( sal_uInt16 nResId, const String& rURL )
{
    mnResId = nResId;
    maURL1 = rURL;
    maURL2.Erase();
}

// =====================================================================

void HtmlErrorContext::SetContext( sal_uInt16 nResId, const String& rURL1, const String& rURL2 )
{
    mnResId = nResId;
    maURL1 = rURL1;
    maURL2 = rURL2;
}

// =====================================================================


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
