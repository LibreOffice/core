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


#include "htmlex.hxx"
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/drawing/GraphicExportFilter.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>

#include <rtl/uri.hxx>
#include <comphelper/processfactory.hxx>
#include <osl/file.hxx>
#include <unotools/pathoptions.hxx>
#include <vcl/FilterConfigItem.hxx>
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
#include <vcl/graphicfilter.hxx>
#include <editeng/colritem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/crossedoutitem.hxx>
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
#include "htmlpublishmode.hxx"
#include "Outliner.hxx"
#include "sdpage.hxx"
#include "sdattr.hxx"
#include "glob.hrc"
#include "anminfo.hxx"
#include "imapinfo.hxx"
#include "sdresid.hxx"
#include "buttonset.hxx"
#include <basegfx/polygon/b2dpolygon.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::document;

// get parameter from Itemset

#define RESTOHTML( res ) StringToHTMLString(SD_RESSTR(res))

const char *pButtonNames[] =
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
// Helper class for the simple creation of files local/remote
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
// Helper class for the embedding of text attributes into the html output
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
// close all still open tags
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
// c'tor with default color for the page
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
// enables/disables bold print
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
// enables/disables italic
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
// enables/disables underlines
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
// enables/disables strike through
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
// Sets the specified text color
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
// enables/disables a hyperlink
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
// methods of the class HtmlExport
// *********************************************************************

static String getParagraphStyle( SdrOutliner* pOutliner, sal_Int32 nPara )
{
    SfxItemSet aParaSet( pOutliner->GetParaAttribs( nPara ) );

    String sStyle( "direction:" );
    if( static_cast<const SvxFrameDirectionItem*>(aParaSet.GetItem( EE_PARA_WRITINGDIR ))->GetValue() == FRMDIR_HORI_RIGHT_TOP )
    {
        sStyle += String( "rtl;" );
    }
    else
    {
         sStyle += String( "ltr;" );
    }
    return sStyle;
}

// =====================================================================
// constructor for the html export helper classes
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
    // delete lists
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
            double temp = 0.0;
            pParams->Value >>= temp;
            mfSlideDuration = temp;
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
    // we come up with a destination...

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
// exports the (in the c'tor specified impress document) to html
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
        // default colors for the color schema 'From Document'
        SetDocColors();
        maFirstPageColor = maBackColor;
    }

    // get name for downloadable presentation if needed
    if( mbDownload )
    {
        // fade out separator search and extension
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
    // Exceptions are cool...

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
        // default text color from the outline template of the first page
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

    // default background from the background of the master page of the first page
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
            aProperties[ 1 ].Value <<= OUString("impress8");
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
// create image files
// =====================================================================
bool HtmlExport::CreateImagesForPresPages( bool bThumbnail)
{
    try
    {
        Reference < XComponentContext > xContext = ::comphelper::getProcessComponentContext();

        Reference< drawing::XGraphicExportFilter > xGraphicExporter = drawing::GraphicExportFilter::create( xContext );

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
            xGraphicExporter->filter( aDescriptor );

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
// get SdrTextObject with layout text of this page
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
// create HTML text version of impress pages
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

// HTML head
        String aStr(maHTMLHeader);
        aStr += WriteMetaCharset();
        aStr.AppendAscii( "  <title>" );
        aStr += StringToHTMLString( *mpPageNames[nSdPage] );
        aStr.AppendAscii( "</title>\r\n" );
        aStr.AppendAscii( "</head>\r\n" );
        aStr += CreateBodyTag();

// navigation bar
        aStr += CreateNavBar(nSdPage, true);

// page title
        String sTitleText( CreateTextForTitle(pOutliner,pPage, pPage->GetPageBackgroundColor()) );
        aStr.AppendAscii( "<h1 style=\"");
        aStr.Append( getParagraphStyle( pOutliner, 0 ) );
        aStr.AppendAscii( "\">" );
        aStr += sTitleText;
        aStr.AppendAscii( "</h1>\r\n" );

// write outline text
        aStr += CreateTextForPage( pOutliner, pPage, true, pPage->GetPageBackgroundColor() );

// notes
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

// clsoe page
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
        OString aStr(OUStringToOString(rHtmlData,
            RTL_TEXTENCODING_UTF8));
        *pStr << aStr.getStr();
        nErr = aFile.close();
    }

    if( nErr != 0 )
        ErrorHandler::HandleError(nErr);

    return nErr == 0;
}

// =====================================================================

/** creates a outliner text for the title objects of a page
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
// creates a outliner text for a page
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

            sal_Int32 nCount = pOutliner->GetParagraphCount();

            Paragraph* pPara = NULL;
            sal_Int16 nActDepth = -1;

            String aParaText;
            for (sal_Int32 nPara = 0; nPara < nCount; nPara++)
            {
                pPara = pOutliner->GetParagraph(nPara);
                if(pPara == 0)
                    continue;

                const sal_Int16 nDepth = (sal_uInt16) pOutliner->GetDepth( nPara );
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
// creates a outliner text for a note page
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

            sal_Int32 nCount = pOutliner->GetParagraphCount();
            for (sal_Int32 nPara = 0; nPara < nCount; nPara++)
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
// converts a paragraph of the outliner to html
// =====================================================================
String HtmlExport::ParagraphToHTMLString( SdrOutliner* pOutliner, sal_Int32 nPara, const Color& rBackgroundColor )
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
    rEditEngine.GetPortions( nPara, aPortionList );

    sal_uInt16 nPos1 = 0;
    for( std::vector<sal_uInt16>::const_iterator it( aPortionList.begin() ); it != aPortionList.end(); ++it )
    {
        sal_uInt16 nPos2 = *it;

        ESelection aSelection( nPara, nPos1, nPara, nPos2);

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
// Depending on the attributes of the specified set and the specified
// HtmlState, it creates the needed html tags in order to get the
// attributes.
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
// create HTML wrapper for picture files
// =====================================================================
bool HtmlExport::CreateHtmlForPresPages()
{
    bool bOk = true;

    std::vector<SdrObject*> aClickableObjects;

    for(sal_uInt16 nSdPage = 0; nSdPage < mnSdPageCount && bOk; nSdPage++)
    {
        // find clickable objects (also on the master page) and put it in the
        // list. This in reverse order character order since in html the first
        // area is taken in the case they overlap.

        SdPage* pPage = maPages[ nSdPage ];

        if( mbDocColors )
        {
            SetDocColors( pPage );
        }

        bool    bMasterDone = false;

        while (!bMasterDone)
        {
            // sal_True = backwards
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
            // now to the master page or finishing
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
            double fSecs = 0;
            bool bEndless = false;
            if( !mbAutoSlide )
            {
                if( pPage->GetPresChange() != PRESCHANGE_MANUAL )
                {
                    fSecs = pPage->GetTime();
                    bEndless = mpDoc->getPresentationSettings().mbEndless;
                }
            }
            else
            {
                fSecs = mfSlideDuration;
                bEndless = mbEndless;
            }

            if( fSecs != 0 )
            {
                if( nSdPage < (mnSdPageCount-1) || bEndless )
                {
                    aStr.AppendAscii( "<meta http-equiv=\"refresh\" content=\"" );
                    aStr += OUString::number(fSecs);
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

// navigation bar
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

// notes
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

// create Imagemap if necessary
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

                // BoundRect is relative to the physical page origin, not the
                // origin of ordinates
                aRect.Move(-pPage->GetLftBorder(), -pPage->GetUppBorder());

                double fLogicToPixel = ((double)mnWidthPixel) / nPageWidth;
                aRect.Left()   = (long)(aRect.Left() * fLogicToPixel);
                aRect.Top()    = (long)(aRect.Top() * fLogicToPixel);
                aRect.Right()  = (long)(aRect.Right() * fLogicToPixel);
                aRect.Bottom() = (long)(aRect.Bottom() * fLogicToPixel);
                long nRadius = aRect.GetWidth() / 2;


                /*************************************************************
                |* insert areas into Imagemap of the object, if the object has
                |* such a Imagemap
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

                        // if necessary, convert page and object names into the
                        // corresponding names of the html file
                        sal_Bool        bIsMasterPage;
                        sal_uInt16      nPgNum = mpDoc->GetPageByName( aURL, bIsMasterPage );
                        SdrObject*  pObj = NULL;

                        if (nPgNum == SDRPAGE_NOTFOUND)
                        {
                            // is the bookmark a object?
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

                                // conversion into pixel coordinates
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
                                DBG_WARNING("unknown IMAP_OBJ_type");
                            }
                            break;
                        }
                    }
                }



                /*************************************************************
                |* if there is a presentation::ClickAction, determine bookmark
                |* and create area for the whole object
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
                                // is the bookmark a object?
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

                    // and now the areas
                    if( aHRef.Len() )
                    {
                        // a circle?
                        if (pObject->GetObjInventor() == SdrInventor &&
                            pObject->GetObjIdentifier() == OBJ_CIRC  &&
                            bIsSquare )
                        {
                            aStr += CreateHTMLCircleArea(aRect.GetWidth() / 2,
                                                    aRect.Left() + nRadius,
                                                    aRect.Top() + nRadius,
                                                    aHRef);
                        }
                        // a polygon?
                        else if (pObject->GetObjInventor() == SdrInventor &&
                                 (pObject->GetObjIdentifier() == OBJ_PATHLINE ||
                                  pObject->GetObjIdentifier() == OBJ_PLIN ||
                                  pObject->GetObjIdentifier() == OBJ_POLY))
                        {
                            aStr += CreateHTMLPolygonArea(((SdrPathObj*)pObject)->GetPathPoly(), Size(-pPage->GetLftBorder(), -pPage->GetUppBorder()), fLogicToPixel, aHRef);
                        }
                        // something completely different: use the BoundRect
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
// create overview pages
// =====================================================================
bool HtmlExport::CreateContentPage()
{
    if( mbDocColors )
        SetDocColors();

    // html head
    String aStr(maHTMLHeader);
    aStr += WriteMetaCharset();
    aStr.AppendAscii( "  <title>" );
    aStr += StringToHTMLString(*mpPageNames[0]);
    aStr.AppendAscii( "</title>\r\n</head>\r\n" );
    aStr += CreateBodyTag();

    // page head
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

    // table of content
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

    // document information
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
// create note pages (for frames)
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

        // Html head
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
// create outline pages (for frames)
// =====================================================================
bool HtmlExport::CreateOutlinePages()
{
    bool bOk = true;

    if( mbDocColors )
    {
        SetDocColors();
    }

    // page 0 will be the closed outline, page 1 the opened
    for (sal_Int32 nPage = 0; nPage < (mbImpress?2:1) && bOk; ++nPage)
    {
        // Html head
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
            String aLink( "JavaScript:parent.NavigateAbs(" );
            aLink += OUString::number(nSdPage);
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
// set file name
// =====================================================================
void HtmlExport::CreateFileNames()
{
    // create lists with new file names
    mpHTMLFiles = new String*[mnSdPageCount];
    mpImageFiles = new String*[mnSdPageCount];
    mpThumbnailFiles = new String*[mnSdPageCount];
    mpPageNames = new String*[mnSdPageCount];
    mpTextFiles = new String*[mnSdPageCount];

    mbHeader = false;   // headline on overview page?

    for (sal_uInt16 nSdPage = 0; nSdPage < mnSdPageCount; nSdPage++)
    {
        String* pName;
        if(nSdPage == 0 && !mbContentsPage && !mbFrames )
            pName = new String(maIndex);
        else
        {
            pName = new String( "img" );
            *pName += OUString::number(nSdPage);
            *pName += maHTMLExtension;
        }

        mpHTMLFiles[nSdPage] = pName;

        pName = new String( "img" );
        *pName += OUString::number(nSdPage);
        if( meFormat==FORMAT_GIF )
            pName->AppendAscii( ".gif" );
        else if( meFormat==FORMAT_JPG )
            pName->AppendAscii( ".jpg" );
        else
            pName->AppendAscii( ".png" );

        mpImageFiles[nSdPage] = pName;

        pName = new String( "thumb" );
        *pName += OUString::number(nSdPage);
        if( meFormat!=FORMAT_JPG )
            pName->AppendAscii( ".png" );
        else
            pName->AppendAscii( ".jpg" );

        mpThumbnailFiles[nSdPage] = pName;

        pName = new String( "text" );
        *pName += OUString::number(nSdPage);
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
            // if there is a non-empty title object, use their first passage
            // as page title
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
// create page with the frames
// ====================================================================
bool HtmlExport::CreateFrames()
{
    String aTmp;
    String aStr( "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Frameset//EN\"\r\n"
                    "    \"http://www.w3.org/TR/html4/frameset.dtd\">\r\n"
            "<html>\r\n<head>\r\n"  );

    aStr += WriteMetaCharset();
    aStr.AppendAscii( "  <title>" );
    aStr += StringToHTMLString(*mpPageNames[0]);
    aStr.AppendAscii( "</title>\r\n" );

    aStr.AppendAscii( "<script type=\"text/javascript\">\r\n<!--\r\n" );

    aStr.AppendAscii( "var nCurrentPage = 0;\r\nvar nPageCount = " );
    aStr += OUString::number(mnSdPageCount);
    aStr.AppendAscii( ";\r\n\r\n" );

    String aFunction;
    aFunction.AssignAscii(JS_NavigateAbs);

    if(mbNotes)
    {
        String aSlash( "//" );
        aFunction.SearchAndReplaceAll(aSlash, OUString());
    }

    // substitute HTML file extension
    String aPlaceHolder(".$EXT");
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
    aStr += OUString::number((mnWidthPixel + 16));
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
        aStr += OUString::number((int)((double)mnWidthPixel * 0.75) + 16);
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
// create button bar for standard
// we create the following html files
// navbar0.htm navigation bar graphic for the first page
// navbar1.htm navigation bar graphic for the second until second last page
// navbar2.htm navigation bar graphic for the last page
// navbar3.htm navigation outline closed
// navbar4.htm navigation outline open
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

    // first page
        aButton = SD_RESSTR(STR_HTMLEXP_FIRSTPAGE);
        if(mnButtonThema != -1)
            aButton = CreateImage(GetButtonName((nFile == 0 || mnSdPageCount == 1?
                                  BTN_FIRST_0:BTN_FIRST_1)), aButton);

        if(nFile != 0 && mnSdPageCount > 1)
            aButton = CreateLink( String("JavaScript:parent.NavigateAbs(0)"), aButton);

        aStr += aButton;
        aStr.AppendAscii( "\r\n" );

    // to the previous page
        aButton = SD_RESSTR(STR_PUBLISH_BACK);
        if(mnButtonThema != -1)
            aButton = CreateImage(GetButtonName((nFile == 0 || mnSdPageCount == 1?
                                    BTN_PREV_0:BTN_PREV_1)), aButton);

        if(nFile != 0 && mnSdPageCount > 1)
            aButton = CreateLink( String("JavaScript:parent.NavigateRel(-1)"), aButton);

        aStr += aButton;
        aStr.AppendAscii( "\r\n" );

    // to the next page
        aButton = SD_RESSTR(STR_PUBLISH_NEXT);
        if(mnButtonThema != -1)
            aButton = CreateImage(GetButtonName((nFile ==2 || mnSdPageCount == 1?
                                    BTN_NEXT_0:BTN_NEXT_1)), aButton);

        if(nFile != 2 && mnSdPageCount > 1)
            aButton = CreateLink(String("JavaScript:parent.NavigateRel(1)"), aButton);

        aStr += aButton;
        aStr.AppendAscii( "\r\n" );

    // to the last page
        aButton = SD_RESSTR(STR_HTMLEXP_LASTPAGE);
        if(mnButtonThema != -1)
            aButton = CreateImage(GetButtonName((nFile ==2 || mnSdPageCount == 1?
                                  BTN_LAST_0:BTN_LAST_1)), aButton);

        if(nFile != 2 && mnSdPageCount > 1)
        {
            String aLink("JavaScript:parent.NavigateAbs(");
            aLink += OUString::number(mnSdPageCount-1);
            aLink.AppendAscii( ")" );
            aButton = CreateLink( aLink, aButton);
        }

        aStr += aButton;
        aStr.AppendAscii( "\r\n" );

    // content
        if (mbContentsPage)
        {
            aButton = SD_RESSTR(STR_PUBLISH_OUTLINE);
            if(mnButtonThema != -1)
                aButton = CreateImage(GetButtonName(BTN_INDEX), aButton);

            // to the overview
            aStr += CreateLink(maIndex, aButton, String("_top"));
            aStr.AppendAscii( "\r\n" );
        }

    // text mode
        if(mbImpress)
        {
            aButton = SD_RESSTR(STR_HTMLEXP_SETTEXT);
            if(mnButtonThema != -1)
                aButton = CreateImage(GetButtonName(BTN_TEXT), aButton);

            String  aText0( "text0" );
            aText0 += maHTMLExtension;
            aStr += CreateLink( aText0, aButton, String("_top"));
            aStr.AppendAscii( "\r\n" );
        }

    // and finished...
        aStr.AppendAscii( "</center>\r\n" );
        aStr.AppendAscii( "</body>\r\n</html>" );

        String aFileName( "navbar" );
        aFileName += OUString::number(nFile);

        bOk = WriteHtml( aFileName, true, aStr );

        if (mpProgress)
            mpProgress->SetState(++mnPagesWritten);
    }

    // the navigation bar outliner closed...
    if(bOk)
    {
        String aStr(maHTMLHeader);
        aStr += WriteMetaCharset();
        aStr.AppendAscii( "  <title>" );
        aStr += StringToHTMLString(*mpPageNames[0]);
        aStr.AppendAscii( "</title>\r\n</head>\r\n" );
        aStr += CreateBodyTag();

        aButton = SD_RESSTR(STR_HTMLEXP_OUTLINE);
        if(mnButtonThema != -1)
            aButton = CreateImage(GetButtonName(BTN_MORE), aButton);

        aStr += CreateLink(String("JavaScript:parent.ExpandOutline()"), aButton);
        aStr.AppendAscii( "</body>\r\n</html>" );

        String aFileName( "navbar3" );

        bOk = WriteHtml( aFileName, true, aStr );

        if (mpProgress)
            mpProgress->SetState(++mnPagesWritten);
    }

    // ... and the outliner open
    if( bOk )
    {
        String aStr(maHTMLHeader);
        aStr += WriteMetaCharset();
        aStr.AppendAscii( "  <title>" );
        aStr += StringToHTMLString(*mpPageNames[0]);
        aStr.AppendAscii( "</title>\r\n</head>\r\n" );
        aStr += CreateBodyTag();

        aButton = SD_RESSTR(STR_HTMLEXP_NOOUTLINE);
        if(mnButtonThema != -1)
            aButton = CreateImage(GetButtonName(BTN_LESS), aButton);

        aStr += CreateLink(String("JavaScript:parent.CollapseOutline()"), aButton);
        aStr.AppendAscii( "</body>\r\n</html>" );

        String aFileName( "navbar4" );
        bOk = WriteHtml( aFileName, true, aStr );

        if (mpProgress)
            mpProgress->SetState(++mnPagesWritten);

    }

    return bOk;
}

// ====================================================================
// create button bar for standard
// ====================================================================
String HtmlExport::CreateNavBar( sal_uInt16 nSdPage, bool bIsText ) const
{
    // prepare button bar
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

    String aStr( "<center>\r\n"); //<table><tr>\r\n");

    // first page
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
        for( int nButton = 0; nButton != SAL_N_ELEMENTS(pButtonNames); nButton++ )
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
// creates the <body> tag, including the specified color attributes
// =====================================================================
String HtmlExport::CreateBodyTag() const
{
    String aStr( "<body" );

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
// creates a hyperlink
// =====================================================================
String HtmlExport::CreateLink( const String& aLink,
                               const String& aText,
                               const String& aTarget ) const
{
    String aStr( "<a href=\"" );
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
// creates a image tag
// =====================================================================
String HtmlExport::CreateImage( const String& aImage, const String& aAltText,
                                sal_Int16 nWidth,
                                sal_Int16 nHeight ) const
{
    String aStr( "<img src=\"");
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
        aStr += OUString::number(nWidth);
    }

    if(nHeight > -1)
    {
        aStr.AppendAscii( " height=" );
        aStr += OUString::number(nHeight);
    }

    aStr.Append(sal_Unicode('>'));

    return aStr;
}

// =====================================================================
// create area for a circle; we expect pixel coordinates
// =====================================================================
String HtmlExport::ColorToHTMLString( Color aColor )
{
    static const char hex[] = "0123456789ABCDEF";
    String aStr( "#xxxxxx" );
    aStr.SetChar(1, hex[(aColor.GetRed() >> 4) & 0xf] );
    aStr.SetChar(2, hex[aColor.GetRed()   & 0xf] );
    aStr.SetChar(3, hex[(aColor.GetGreen() >> 4) & 0xf] );
    aStr.SetChar(4, hex[aColor.GetGreen() & 0xf] );
    aStr.SetChar(5, hex[(aColor.GetBlue() >> 4) & 0xf] );
    aStr.SetChar(6, hex[aColor.GetBlue()  & 0xf] );

    return aStr;
}

// =====================================================================
// create area for a circle; we expect pixel coordinates
// =====================================================================
String HtmlExport::CreateHTMLCircleArea( sal_uLong nRadius,
                                         sal_uLong nCenterX,
                                         sal_uLong nCenterY,
                                         const String& rHRef ) const
{
    String aStr( "<area shape=\"circle\" alt=\"\" coords=\"" );

    aStr += OUString::number(nCenterX);
    aStr.Append(sal_Unicode(','));
    aStr += OUString::number(nCenterY);
    aStr.Append(sal_Unicode(','));
    aStr += OUString::number(nRadius);
    aStr.AppendAscii( "\" href=\"" );
    aStr += StringToURL(rHRef);
    aStr.AppendAscii( "\">\n" );

    return aStr;
}


// =====================================================================
// create area for a polygon; we expect pixel coordinates
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
            // coordinates are relative to the physical page origin, not the
            // origin of ordinates
            aPnt.Move(aShift.Width(), aShift.Height());

            aPnt.X() = (long)(aPnt.X() * fFactor);
            aPnt.Y() = (long)(aPnt.Y() * fFactor);
            aStr += OUString::number(aPnt.X());
            aStr.Append(sal_Unicode(','));
            aStr += OUString::number(aPnt.Y());

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
// create area for a rectangle; we expect pixel coordinates
// =====================================================================
String HtmlExport::CreateHTMLRectArea( const Rectangle& rRect,
                                       const String& rHRef ) const
{
    String aStr( "<area shape=\"rect\" alt=\"\" coords=\"" );

    aStr += OUString::number(rRect.Left());
    aStr.Append(sal_Unicode(','));
    aStr += OUString::number(rRect.Top());
    aStr.Append(sal_Unicode(','));
    aStr += OUString::number(rRect.Right());
    aStr.Append(sal_Unicode(','));
    aStr += OUString::number(rRect.Bottom());
    aStr.AppendAscii( "\" href=\"" );
    aStr += StringToURL(rHRef);
    aStr.AppendAscii( "\">\n" );

    return aStr;
}

// =====================================================================
// escapes a string for html
// =====================================================================
String HtmlExport::StringToHTMLString( const String& rString )
{
    SvMemoryStream aMemStm;
    HTMLOutFuncs::Out_String( aMemStm, rString, RTL_TEXTENCODING_UTF8 );
    aMemStm << (char) 0;
    return String( (char*)aMemStm.GetData(), RTL_TEXTENCODING_UTF8 );
}

// =====================================================================
// creates a url for a specific page
// =====================================================================
String HtmlExport::CreatePageURL( sal_uInt16 nPgNum )
{
    if(mbFrames)
    {
        String aUrl( "JavaScript:parent.NavigateAbs(" );
        aUrl += OUString::number(nPgNum);
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

    aURL.Append( OUString("webcast") );
    aURL.Append( rSource );

    meEC.SetContext( STR_HTMLEXP_ERROR_OPEN_FILE, rSource );

    sal_uLong       nErr = 0;
    SvStream*   pIStm = ::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ );

    if( pIStm )
    {
        OString aLine;

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


    aScript.SearchAndReplaceAll( String("$$1"), getDocumentTitle() );

    const String aSaveStr( RESTOHTML( STR_WEBVIEW_SAVE ));
    aScript.SearchAndReplaceAll( String("$$2"), aSaveStr );

    aScript.SearchAndReplaceAll( String("$$3"), maCGIPath );

    aScript.SearchAndReplaceAll( String("$$4"), OUString::number(mnWidthPixel) );
    aScript.SearchAndReplaceAll( String("$$5"), OUString::number(mnHeightPixel) );


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
            OString aStr(OUStringToOString(aScript,
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

/** creates and saves the ASP scripts for WebShow */
bool HtmlExport::CreateASPScripts()
{
    for( sal_uInt16 n = 0; n < (sizeof( ASP_Scripts ) / sizeof(char *)); n++ )
    {
        String aScript;

        aScript.AssignAscii( ASP_Scripts[n] );
        if(!CopyScript(maExportPath, aScript, aScript))
            return false;
    }

    if(!CopyScript(maExportPath, String("edit.asp"), maIndex ))
        return false;

    return true;
}


static const char *PERL_Scripts[] = { "webcast.pl", "common.pl", "editpic.pl", "poll.pl", "savepic.pl", "show.pl" };

/** creates and saves the PERL scripts for WebShow */
bool HtmlExport::CreatePERLScripts()
{
    for( sal_uInt16 n = 0; n < (sizeof( PERL_Scripts ) / sizeof(char *)); n++ )
    {
        String aScript;
        aScript.AssignAscii( PERL_Scripts[n] );
        if(!CopyScript(maExportPath, aScript, aScript, true))
            return false;
    }

    if(!CopyScript(maExportPath, OUString("edit.pl"), maIndex, true ))
        return false;

    if(!CopyScript(maExportPath, OUString("index.pl"), maIndexUrl, true ))
        return false;

    return true;
}

/** creates a list with names of the saved images */
bool HtmlExport::CreateImageFileList()
{
    String aStr;
    for( sal_uInt16 nSdPage = 0; nSdPage < mnSdPageCount; nSdPage++)
    {
        aStr += OUString::number( nSdPage + 1 );
        aStr.Append(sal_Unicode(';'));
        aStr += maURLPath;
        aStr += *mpImageFiles[nSdPage];
        aStr.AppendAscii( "\r\n" );
    }

    String aFileName( "picture.txt" );
    bool bOk = WriteHtml( aFileName, false, aStr );

    if (mpProgress)
        mpProgress->SetState(++mnPagesWritten);

    return bOk;
}

/** creates a file with the actual page number */
bool HtmlExport::CreateImageNumberFile()
{
    String aFull( maExportPath );
    String aFileName( "currpic.txt" );
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

    String      aStr( "<embed src=\"" );
    INetURLObject   aURL( rSoundFile );
    String aSoundFileName = String(aURL.getName());

    DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

    aStr += aSoundFileName;
    aStr.AppendAscii( "\" hidden=\"true\" autostart=\"true\">" );

    CopyFile( OUString(rSoundFile), OUString(maExportPath) + OUString(aSoundFileName) );

    return aStr;
}

// =====================================================================

bool HtmlExport::CopyFile( const OUString& rSourceFile, const OUString& rDestFile )
{
    meEC.SetContext( STR_HTMLEXP_ERROR_COPY_FILE, rSourceFile, rDestFile );
    osl::FileBase::RC Error = osl::File::copy( rSourceFile, rDestFile );

    if( Error != osl::FileBase::E_None )
    {
        ErrorHandler::HandleError(Error);
        return false;
    }
    else
    {
        return true;
    }
}

// =====================================================================

bool HtmlExport::checkFileExists( Reference< ::com::sun::star::ucb::XSimpleFileAccess3 >& xFileAccess, String const & aFileName )
{
    try
    {
        OUString url( maExportPath );
        url += aFileName;
        return xFileAccess->exists( url );
    }
    catch( com::sun::star::uno::Exception& )
    {
        OSL_FAIL(OString(OString("sd::HtmlExport::checkFileExists(), exception caught: ") +
             OUStringToOString( comphelper::anyToString( cppu::getCaughtException() ), RTL_TEXTENCODING_UTF8 )).getStr() );
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
        uno::Reference<ucb::XSimpleFileAccess3> xFA(ucb::SimpleFileAccess::create(xContext));

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
        OSL_FAIL(OString(OString("sd::HtmlExport::checkForExistingFiles(), exception caught: ") +
             OUStringToOString( comphelper::anyToString( cppu::getCaughtException() ), RTL_TEXTENCODING_UTF8 )).getStr() );
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
            OUString aURLStr;
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
        // transmitted
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

bool HtmlErrorContext::GetString( sal_uLong, OUString& rCtxStr )
{
    DBG_ASSERT( mnResId != 0, "No error context set" );
    if( mnResId == 0 )
        return false;

    rCtxStr = SdResId( mnResId );

    rCtxStr = rCtxStr.replaceAll( OUString("$(URL1)"), maURL1 );
    rCtxStr = rCtxStr.replaceAll( OUString("$(URL2)"), maURL2 );

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
