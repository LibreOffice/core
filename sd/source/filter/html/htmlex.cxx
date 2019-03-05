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
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/drawing/GraphicExportFilter.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>

#include <sal/log.hxx>
#include <rtl/tencinfo.h>
#include <comphelper/processfactory.hxx>
#include <osl/file.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <com/sun/star/frame/XStorable.hpp>
#include <sfx2/frmhtmlw.hxx>
#include <sfx2/progress.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <svx/svditer.hxx>
#include <vcl/imaprect.hxx>
#include <vcl/imapcirc.hxx>
#include <vcl/imappoly.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/editobj.hxx>
#include <svx/svdopath.hxx>
#include <svtools/htmlout.hxx>
#include <svtools/colorcfg.hxx>
#include <editeng/colritem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/flditem.hxx>
#include <svl/style.hxx>
#include <editeng/frmdiritem.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdotable.hxx>
#include <tools/urlobj.hxx>
#include <svtools/sfxecode.hxx>
#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <tools/diagnose_ex.h>

#include <drawdoc.hxx>
#include <DrawDocShell.hxx>
#include "htmlpublishmode.hxx"
#include <Outliner.hxx>
#include <sdpage.hxx>
#include <strings.hrc>
#include <strings.hxx>
#include <anminfo.hxx>
#include <imapinfo.hxx>
#include <sdresid.hxx>
#include "buttonset.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::document;

using namespace sdr::table;

// get parameter from Itemset
#define RESTOHTML( res ) StringToHTMLString(SdResId(res))

const char * const pButtonNames[] =
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

// Helper class for the simple creation of files local/remote
class EasyFile
{
private:
    std::unique_ptr<SvStream> pOStm;
    bool        bOpen;

public:

    EasyFile();
    ~EasyFile();

    ErrCode   createStream( const OUString& rUrl, SvStream*& rpStr );
    void      createFileName(  const OUString& rUrl, OUString& rFileName );
    void      close();
};

// Helper class for the embedding of text attributes into the html output
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
    OUString maLink;
    OUString maTarget;

public:
    explicit HtmlState( Color aDefColor );

    OUString SetWeight( bool bWeight );
    OUString SetItalic( bool bItalic );
    OUString SetUnderline( bool bUnderline );
    OUString SetColor( Color aColor );
    OUString SetStrikeout( bool bStrike );
    OUString SetLink( const OUString& aLink, const OUString& aTarget );
    OUString Flush();
};

// close all still open tags
OUString HtmlState::Flush()
{
    OUString aStr = SetWeight(false)
                  + SetItalic(false)
                  + SetUnderline(false)
                  + SetStrikeout(false)
                  + SetColor(maDefColor)
                  + SetLink("","");

    return aStr;
}

// c'tor with default color for the page
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

// enables/disables bold print
OUString HtmlState::SetWeight( bool bWeight )
{
    OUString aStr;

    if(bWeight && !mbWeight)
        aStr = "<b>";
    else if(!bWeight && mbWeight)
        aStr = "</b>";

    mbWeight = bWeight;
    return aStr;
}

// enables/disables italic

OUString HtmlState::SetItalic( bool bItalic )
{
    OUString aStr;

    if(bItalic && !mbItalic)
        aStr = "<i>";
    else if(!bItalic && mbItalic)
        aStr = "</i>";

    mbItalic = bItalic;
    return aStr;
}

// enables/disables underlines

OUString HtmlState::SetUnderline( bool bUnderline )
{
    OUString aStr;

    if(bUnderline && !mbUnderline)
        aStr = "<u>";
    else if(!bUnderline && mbUnderline)
        aStr = "</u>";

    mbUnderline = bUnderline;
    return aStr;
}

// enables/disables strike through
OUString HtmlState::SetStrikeout( bool bStrike )
{
    OUString aStr;

    if(bStrike && !mbStrike)
        aStr = "<strike>";
    else if(!bStrike && mbStrike)
        aStr = "</strike>";

    mbStrike = bStrike;
    return aStr;
}

// Sets the specified text color
OUString HtmlState::SetColor( Color aColor )
{
    OUString aStr;

    if(mbColor && aColor == maColor)
        return aStr;

    if(mbColor)
    {
        aStr = "</font>";
        mbColor = false;
    }

    if(aColor != maDefColor)
    {
        maColor = aColor;
        aStr += "<font color=\"" + HtmlExport::ColorToHTMLString(aColor) + "\">";
        mbColor = true;
    }

    return aStr;
}

// enables/disables a hyperlink
OUString HtmlState::SetLink( const OUString& aLink, const OUString& aTarget )
{
    OUString aStr;

    if(mbLink&&maLink == aLink&&maTarget==aTarget)
        return aStr;

    if(mbLink)
    {
        aStr = "</a>";
        mbLink = false;
    }

    if (!aLink.isEmpty())
    {
        aStr += "<a href=\"" + aLink;
        if (!aTarget.isEmpty())
        {
            aStr += "\" target=\"" + aTarget;
        }
        aStr += "\">";
        mbLink = true;
        maLink = aLink;
        maTarget = aTarget;
    }

    return aStr;
}
namespace
{

OUString getParagraphStyle( SdrOutliner* pOutliner, sal_Int32 nPara )
{
    SfxItemSet aParaSet( pOutliner->GetParaAttribs( nPara ) );

    OUString sStyle;

    if( aParaSet.GetItem<SvxFrameDirectionItem>( EE_PARA_WRITINGDIR )->GetValue() == SvxFrameDirection::Horizontal_RL_TB )
    {

        sStyle = "direction: rtl;";
    }
    else
    {
        // This is the default so don't write it out
        // sStyle += "direction: ltr;";
    }
    return sStyle;
}

void lclAppendStyle(OUStringBuffer& aBuffer, const OUString& aTag, const OUString& aStyle)
{
    if (aStyle.isEmpty())
        aBuffer.append("<").append(aTag).append(">");
    else
        aBuffer.append("<").append(aTag).append(" style=\"").append(aStyle).append("\">");
}

} // anonymous namespace

static constexpr OUStringLiteral gaHTMLHeader(
            "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\"\r\n"
            "     \"http://www.w3.org/TR/html4/transitional.dtd\">\r\n"
            "<html>\r\n<head>\r\n" );

static constexpr OUStringLiteral gaHTMLExtension = STR_HTMLEXP_DEFAULT_EXTENSION;

// constructor for the html export helper classes
HtmlExport::HtmlExport(
    const OUString& aPath,
    const Sequence< PropertyValue >& rParams,
    SdDrawDocument* pExpDoc,
    sd::DrawDocShell* pDocShell )
    :   maPath( aPath ),
        mpDoc(pExpDoc),
        mpDocSh( pDocShell ),
        meEC(),
        meMode( PUBLISH_SINGLE_DOCUMENT ),
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
        maTextColor(COL_BLACK),
        maBackColor(COL_WHITE),
        mbDocColors(false),
        maIndexUrl("index"),
        meScript( SCRIPT_ASP ),
        mpButtonSet( new ButtonSet() )
{
    bool bChange = mpDoc->IsChanged();

    maIndexUrl += gaHTMLExtension;

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
    case PUBLISH_SINGLE_DOCUMENT:
        ExportSingleDocument();
        break;
    }

    mpDoc->SetChanged(bChange);
}

HtmlExport::~HtmlExport()
{
}

// get common export parameters from item set
void HtmlExport::InitExportParameters( const Sequence< PropertyValue >& rParams )
{
    mbImpress = mpDoc->GetDocumentType() == DocumentType::Impress;

    sal_Int32 nArgs = rParams.getLength();
    const PropertyValue* pParams = rParams.getConstArray();
    OUString aStr;
    while( nArgs-- )
    {
        if ( pParams->Name == "PublishMode" )
        {
            sal_Int32 temp = 0;
            pParams->Value >>= temp;
            meMode = static_cast<HtmlPublishMode>(temp);
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
            meFormat = static_cast<PublishingFormat>(temp);
        }
        else if ( pParams->Name == "Compression" )
        {
            pParams->Value >>= aStr;
            OUString aTmp( aStr );
            if(!aTmp.isEmpty())
            {
                aTmp = aTmp.replaceFirst("%", "");
                mnCompression = static_cast<sal_Int16>(aTmp.toInt32());
            }
        }
        else if ( pParams->Name == "Width" )
        {
            sal_Int32 temp = 0;
            pParams->Value >>= temp;
            mnWidthPixel = static_cast<sal_uInt16>(temp);
        }
        else if ( pParams->Name == "UseButtonSet" )
        {
            sal_Int32 temp = 0;
            pParams->Value >>= temp;
            mnButtonThema = static_cast<sal_Int16>(temp);
        }
        else if ( pParams->Name == "IsExportNotes" )
        {
            if( mbImpress )
            {
                bool temp = false;
                pParams->Value >>= temp;
                mbNotes = temp;
            }
        }
        else if ( pParams->Name == "IsExportContentsPage" )
        {
            bool temp = false;
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
            bool temp = false;
            pParams->Value >>= temp;
            mbDownload = temp;
        }
        else if ( pParams->Name == "SlideSound" )
        {
            bool temp = true;
            pParams->Value >>= temp;
            mbSlideSound = temp;
        }
        else if ( pParams->Name == "HiddenSlides" )
        {
            bool temp = true;
            pParams->Value >>= temp;
            mbHiddenSlides = temp;
        }
        else if ( pParams->Name == "BackColor" )
        {
            sal_Int32 temp = 0;
            pParams->Value >>= temp;
            maBackColor = Color(temp);
            mbUserAttr = true;
        }
        else if ( pParams->Name == "TextColor" )
        {
            sal_Int32 temp = 0;
            pParams->Value >>= temp;
            maTextColor = Color(temp);
            mbUserAttr = true;
        }
        else if ( pParams->Name == "LinkColor" )
        {
            sal_Int32 temp = 0;
            pParams->Value >>= temp;
            maLinkColor = Color(temp);
            mbUserAttr = true;
        }
        else if ( pParams->Name == "VLinkColor" )
        {
            sal_Int32 temp = 0;
            pParams->Value >>= temp;
            maVLinkColor = Color(temp);
            mbUserAttr = true;
        }
        else if ( pParams->Name == "ALinkColor" )
        {
            sal_Int32 temp = 0;
            pParams->Value >>= temp;
            maALinkColor = Color(temp);
            mbUserAttr = true;
        }
        else if ( pParams->Name == "IsUseDocumentColors" )
        {
            bool temp = false;
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
            bool temp = false;
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
    SdPage* pPage = mpDoc->GetSdPage(0, PageKind::Standard);
    Size aTmpSize( pPage->GetSize() );
    double dRatio=static_cast<double>(aTmpSize.Width())/aTmpSize.Height();

    mnHeightPixel = static_cast<sal_uInt16>(mnWidthPixel/dRatio);

    // we come up with a destination...

    INetURLObject aINetURLObj( maPath );
    DBG_ASSERT( aINetURLObj.GetProtocol() != INetProtocol::NotValid, "invalid URL" );

    maExportPath = aINetURLObj.GetPartBeforeLastName(); // with trailing '/'
    maIndex = aINetURLObj.GetLastName();

    mnSdPageCount = mpDoc->GetSdPageCount( PageKind::Standard );
    for( sal_uInt16 nPage = 0; nPage < mnSdPageCount; nPage++ )
    {
        pPage = mpDoc->GetSdPage( nPage, PageKind::Standard );

        if( mbHiddenSlides || !pPage->IsExcluded() )
        {
            maPages.push_back( pPage );
            maNotesPages.push_back( mpDoc->GetSdPage( nPage, PageKind::Notes ) );
        }
    }
    mnSdPageCount = maPages.size();

    mbFrames = meMode == PUBLISH_FRAMES;

    maDocFileName = maIndex;
}

void HtmlExport::ExportSingleDocument()
{
    SdrOutliner* pOutliner = mpDoc->GetInternalOutliner();

    maPageNames.resize(mnSdPageCount);

    mnPagesWritten = 0;
    InitProgress(mnSdPageCount);

    OUStringBuffer aStr(gaHTMLHeader);
    aStr.append(DocumentMetadata());
    aStr.append("\r\n");
    aStr.append("</head>\r\n");
    aStr.append(CreateBodyTag());

    for(sal_uInt16 nSdPage = 0; nSdPage < mnSdPageCount; ++nSdPage)
    {
        SdPage* pPage = maPages[nSdPage];
        maPageNames[nSdPage] = pPage->GetName();

        if( mbDocColors )
        {
            SetDocColors( pPage );
        }

        // page title
        OUString sTitleText(CreateTextForTitle(pOutliner, pPage, pPage->GetPageBackgroundColor()));
        OUString sStyle;

        if (nSdPage != 0) // First page - no need for a page brake here
            sStyle += "page-break-before:always; ";
        sStyle += getParagraphStyle(pOutliner, 0);

        lclAppendStyle(aStr, "h1", sStyle);

        aStr.append(sTitleText);
        aStr.append("</h1>\r\n");

        // write outline text
        aStr.append(CreateTextForPage( pOutliner, pPage, true, pPage->GetPageBackgroundColor() ));

        // notes
        if(mbNotes)
        {
            SdPage* pNotesPage = maNotesPages[ nSdPage ];
            OUString aNotesStr( CreateTextForNotesPage( pOutliner, pNotesPage, maBackColor) );

            if (!aNotesStr.isEmpty())
            {
                aStr.append("<br>\r\n<h3>");
                aStr.append(RESTOHTML(STR_HTMLEXP_NOTES));
                aStr.append(":</h3>\r\n");

                aStr.append(aNotesStr);
            }
        }

        if (mpProgress)
            mpProgress->SetState(++mnPagesWritten);

    }

    // close page
    aStr.append("</body>\r\n</html>");

    WriteHtml(maDocFileName, false, aStr.makeStringAndClear());

    pOutliner->Clear();
    ResetProgress();
}

// exports the (in the c'tor specified impress document) to html
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
        sal_Int32 nSepPos = maDocFileName.indexOf('.');
        if (nSepPos != -1)
            maDocFileName = maDocFileName.copy(0, nSepPos);

        maDocFileName += ".odp";
    }

    sal_uInt16 nProgrCount = mnSdPageCount;
    nProgrCount += mbImpress?mnSdPageCount:0;
    nProgrCount += mbContentsPage?1:0;
    nProgrCount += (mbFrames && mbNotes)?mnSdPageCount:0;
    nProgrCount += mbFrames ? 8 : 0;
    InitProgress( nProgrCount );

    mpDocSh->SetWaitCursor( true );

    // Exceptions are cool...

    CreateFileNames();

    // this is not a true while
    while( true )
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

        CreateBitmaps();

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

void HtmlExport::SetDocColors( SdPage* pPage )
{
    if( pPage == nullptr )
        pPage = mpDoc->GetSdPage(0, PageKind::Standard);

    svtools::ColorConfig aConfig;
    maVLinkColor = aConfig.GetColorValue(svtools::LINKSVISITED).nColor;
    maALinkColor = aConfig.GetColorValue(svtools::LINKS).nColor;
    maLinkColor  = aConfig.GetColorValue(svtools::LINKS).nColor;
    maTextColor  = COL_BLACK;

    SfxStyleSheet* pSheet = nullptr;

    if( mpDoc->GetDocumentType() == DocumentType::Impress )
    {
        // default text color from the outline template of the first page
        pSheet = pPage->GetStyleSheetForPresObj(PRESOBJ_OUTLINE);
        if(pSheet == nullptr)
            pSheet = pPage->GetStyleSheetForPresObj(PRESOBJ_TEXT);
        if(pSheet == nullptr)
            pSheet = pPage->GetStyleSheetForPresObj(PRESOBJ_TITLE);
    }

    if(pSheet == nullptr)
        pSheet = mpDoc->GetDefaultStyleSheet();

    if(pSheet)
    {
        SfxItemSet& rSet = pSheet->GetItemSet();
        if(rSet.GetItemState(EE_CHAR_COLOR) == SfxItemState::SET)
            maTextColor = rSet.GetItem<SvxColorItem>(EE_CHAR_COLOR)->GetValue();
    }

    // default background from the background of the master page of the first page
    maBackColor = pPage->GetPageBackgroundColor();

    if( maTextColor == COL_AUTO )
    {
        if( !maBackColor.IsDark() )
            maTextColor = COL_BLACK;
    }
}

void HtmlExport::InitProgress( sal_uInt16 nProgrCount )
{
    mpProgress.reset(new SfxProgress( mpDocSh, SdResId(STR_CREATE_PAGES), nProgrCount ));
}

void HtmlExport::ResetProgress()
{
    mpProgress.reset();
}

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

// Export Document with WebCast (TM) Technology
void HtmlExport::ExportWebCast()
{
    mnPagesWritten = 0;
    InitProgress( mnSdPageCount + 9 );

    mpDocSh->SetWaitCursor( true );

    CreateFileNames();

    if (maCGIPath.isEmpty())
        maCGIPath = ".";

    if (!maCGIPath.endsWith("/"))
        maCGIPath += "/";

    if( meScript == SCRIPT_ASP )
    {
        maURLPath = "./";
    }
    else
    {
        if (maURLPath.isEmpty())
            maURLPath = ".";

        if (!maURLPath.endsWith("/"))
            maURLPath += "/";
    }

    // this is not a true while
    while(true)
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

// Save the presentation as a downloadable file in the dest directory
bool HtmlExport::SavePresentation()
{
    meEC.SetContext( STR_HTMLEXP_ERROR_CREATE_FILE, maDocFileName );

    OUString aURL(maExportPath + maDocFileName);

    mpDocSh->EnableSetModified();

    try
    {
        uno::Reference< frame::XStorable > xStorable( mpDoc->getUnoModel(), uno::UNO_QUERY );
        if( xStorable.is() )
        {
            uno::Sequence< beans::PropertyValue > aProperties( 2 );
            aProperties[ 0 ].Name = "Overwrite";
            aProperties[ 0 ].Value <<= true;
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

// create image files
bool HtmlExport::CreateImagesForPresPages( bool bThumbnail)
{
    try
    {
        Reference < XComponentContext > xContext = ::comphelper::getProcessComponentContext();

        Reference< drawing::XGraphicExportFilter > xGraphicExporter = drawing::GraphicExportFilter::create( xContext );

        Sequence< PropertyValue > aFilterData(((meFormat==FORMAT_JPG)&&(mnCompression != -1))? 3 : 2);
        aFilterData[0].Name = "PixelWidth";
        aFilterData[0].Value <<= static_cast<sal_Int32>(bThumbnail ? PUB_THUMBNAIL_WIDTH : mnWidthPixel );
        aFilterData[1].Name = "PixelHeight";
        aFilterData[1].Value <<= static_cast<sal_Int32>(bThumbnail ? PUB_THUMBNAIL_HEIGHT : mnHeightPixel);
        if((meFormat==FORMAT_JPG)&&(mnCompression != -1))
        {
            aFilterData[2].Name = "Quality";
            aFilterData[2].Value <<= static_cast<sal_Int32>(mnCompression);
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
                aFull += maThumbnailFiles[nSdPage];
            else
                aFull += maImageFiles[nSdPage];

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

// get SdrTextObject with layout text of this page
SdrTextObj* HtmlExport::GetLayoutTextObject(SdrPage const * pPage)
{
    const size_t nObjectCount = pPage->GetObjCount();
    SdrTextObj*     pResult      = nullptr;

    for (size_t nObject = 0; nObject < nObjectCount; ++nObject)
    {
        SdrObject* pObject = pPage->GetObj(nObject);
        if (pObject->GetObjInventor() == SdrInventor::Default &&
            pObject->GetObjIdentifier() == OBJ_OUTLINETEXT)
        {
            pResult = static_cast<SdrTextObj*>(pObject);
            break;
        }
    }
    return pResult;
}

// create HTML text version of impress pages
OUString HtmlExport::CreateMetaCharset()
{
    OUString aStr;
    const sal_Char *pCharSet = rtl_getBestMimeCharsetFromTextEncoding( RTL_TEXTENCODING_UTF8 );
    if ( pCharSet )
    {
        aStr = "  <meta HTTP-EQUIV=CONTENT-TYPE CONTENT=\"text/html; charset=" +
               OUString::createFromAscii(pCharSet) + "\">\r\n";
    }
    return aStr;
}

OUString HtmlExport::DocumentMetadata() const
{
    SvMemoryStream aStream;

    uno::Reference<document::XDocumentProperties> xDocProps;
    if (mpDocSh)
    {
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
            mpDocSh->GetModel(), uno::UNO_QUERY_THROW);
        xDocProps.set(xDPS->getDocumentProperties());
    }

    OUString aNonConvertableCharacters;

    SfxFrameHTMLWriter::Out_DocInfo(aStream, maDocFileName, xDocProps,
            "  ", RTL_TEXTENCODING_UTF8,
            &aNonConvertableCharacters);

    const sal_uInt64 nLen = aStream.GetSize();
    OSL_ENSURE(nLen < static_cast<sal_uInt64>(SAL_MAX_INT32), "Stream can't fit in OString");
    OString aData(static_cast<const char*>(aStream.GetData()), static_cast<sal_Int32>(nLen));

    return OStringToOUString(aData, RTL_TEXTENCODING_UTF8);
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
        OUStringBuffer aStr(gaHTMLHeader);
        aStr.append(CreateMetaCharset());
        aStr.append("  <title>");
        aStr.append(StringToHTMLString(maPageNames[nSdPage]));
        aStr.append("</title>\r\n");
        aStr.append("</head>\r\n");
        aStr.append(CreateBodyTag());

        // navigation bar
        aStr.append(CreateNavBar(nSdPage, true));

        // page title
        OUString sTitleText( CreateTextForTitle(pOutliner,pPage, pPage->GetPageBackgroundColor()) );
        lclAppendStyle(aStr, "h1", getParagraphStyle(pOutliner, 0));
        aStr.append(sTitleText);
        aStr.append("</h1>\r\n");

        // write outline text
        aStr.append(CreateTextForPage( pOutliner, pPage, true, pPage->GetPageBackgroundColor() ));

        // notes
        if(mbNotes)
        {
            SdPage* pNotesPage = maNotesPages[ nSdPage ];
            OUString aNotesStr( CreateTextForNotesPage( pOutliner, pNotesPage, maBackColor) );

            if (!aNotesStr.isEmpty())
            {
                aStr.append("<br>\r\n<h3>");
                aStr.append(RESTOHTML(STR_HTMLEXP_NOTES));
                aStr.append(":</h3>\r\n");

                aStr.append(aNotesStr);
            }
        }

        // close page
        aStr.append("</body>\r\n</html>");

        bOk = WriteHtml(maTextFiles[nSdPage], false, aStr.makeStringAndClear());

        if (mpProgress)
            mpProgress->SetState(++mnPagesWritten);

    }

    pOutliner->Clear();

    return bOk;
}

/** exports the given html data into a non unicode file in the current export path with
    the given filename */
bool HtmlExport::WriteHtml( const OUString& rFileName, bool bAddExtension, const OUString& rHtmlData )
{
    ErrCode nErr = ERRCODE_NONE;

    OUString aFileName( rFileName );
    if( bAddExtension )
        aFileName += gaHTMLExtension;

    meEC.SetContext( STR_HTMLEXP_ERROR_CREATE_FILE, rFileName );
    EasyFile aFile;
    SvStream* pStr;
    OUString aFull(maExportPath + aFileName);
    nErr = aFile.createStream(aFull , pStr);
    if(nErr == ERRCODE_NONE)
    {
        OString aStr(OUStringToOString(rHtmlData,
            RTL_TEXTENCODING_UTF8));
        pStr->WriteCharPtr( aStr.getStr() );
        aFile.close();
    }

    if( nErr != ERRCODE_NONE )
        ErrorHandler::HandleError(nErr);

    return nErr == ERRCODE_NONE;
}

/** creates a outliner text for the title objects of a page
 */
OUString HtmlExport::CreateTextForTitle( SdrOutliner* pOutliner, SdPage* pPage, const Color& rBackgroundColor )
{
    SdrTextObj* pTO = static_cast<SdrTextObj*>(pPage->GetPresObj(PRESOBJ_TITLE));
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

    return OUString();
}

// creates a outliner text for a page
OUString HtmlExport::CreateTextForPage(SdrOutliner* pOutliner, SdPage const * pPage,
                                       bool bHeadLine, const Color& rBackgroundColor)
{
    OUStringBuffer aStr;

    for (size_t i = 0; i <pPage->GetObjCount(); ++i )
    {
        SdrObject* pObject = pPage->GetObj(i);
        PresObjKind eKind = pPage->GetPresObjKind(pObject);

        switch (eKind)
        {
            case PRESOBJ_NONE:
            {
                if (pObject->GetObjIdentifier() == OBJ_GRUP)
                {
                    SdrObjGroup* pObjectGroup = static_cast<SdrObjGroup*>(pObject);
                    WriteObjectGroup(aStr, pObjectGroup, pOutliner, rBackgroundColor, false);
                }
                else if (pObject->GetObjIdentifier() == OBJ_TABLE)
                {
                    SdrTableObj* pTableObject = static_cast<SdrTableObj*>(pObject);
                    WriteTable(aStr, pTableObject, pOutliner, rBackgroundColor);
                }
                else
                {
                    if (pObject->GetOutlinerParaObject())
                    {
                        WriteOutlinerParagraph(aStr, pOutliner, pObject->GetOutlinerParaObject(), rBackgroundColor, false);
                    }
                }
            }
            break;

            case PRESOBJ_TABLE:
            {
                SdrTableObj* pTableObject = static_cast<SdrTableObj*>(pObject);
                WriteTable(aStr, pTableObject, pOutliner, rBackgroundColor);
            }
            break;

            case PRESOBJ_TEXT:
            case PRESOBJ_OUTLINE:
            {
                SdrTextObj* pTextObject = static_cast<SdrTextObj*>(pObject);
                if (pTextObject->IsEmptyPresObj())
                    continue;
                WriteOutlinerParagraph(aStr, pOutliner, pTextObject->GetOutlinerParaObject(), rBackgroundColor, bHeadLine);
            }
            break;

            default:
                break;
        }
    }
    return aStr.makeStringAndClear();
}

void HtmlExport::WriteTable(OUStringBuffer& aStr, SdrTableObj const * pTableObject, SdrOutliner* pOutliner, const Color& rBackgroundColor)
{
    CellPos aStart, aEnd;

    aStart = SdrTableObj::getFirstCell();
    aEnd = pTableObject->getLastCell();

    sal_Int32 nColCount = pTableObject->getColumnCount();
    aStr.append("<table>\r\n");
    for (sal_Int32 nRow = aStart.mnRow; nRow <= aEnd.mnRow; nRow++)
    {
        aStr.append("  <tr>\r\n");
        for (sal_Int32 nCol = aStart.mnCol; nCol <= aEnd.mnCol; nCol++)
        {
            aStr.append("    <td>\r\n");
            sal_Int32 nCellIndex = nRow * nColCount + nCol;
            SdrText* pText = pTableObject->getText(nCellIndex);

            if (pText == nullptr)
                continue;
            WriteOutlinerParagraph(aStr, pOutliner, pText->GetOutlinerParaObject(), rBackgroundColor, false);
            aStr.append("    </td>\r\n");
        }
        aStr.append("  </tr>\r\n");
    }
    aStr.append("</table>\r\n");
}

void HtmlExport::WriteObjectGroup(OUStringBuffer& aStr, SdrObjGroup const * pObjectGroup, SdrOutliner* pOutliner,
                                  const Color& rBackgroundColor, bool bHeadLine)
{
    SdrObjListIter aGroupIterator(pObjectGroup->GetSubList(), SdrIterMode::DeepNoGroups);
    while (aGroupIterator.IsMore())
    {
        SdrObject* pCurrentObject = aGroupIterator.Next();
        if (pCurrentObject->GetObjIdentifier() == OBJ_GRUP)
        {
            SdrObjGroup* pCurrentGroupObject = static_cast<SdrObjGroup*>(pCurrentObject);
            WriteObjectGroup(aStr, pCurrentGroupObject, pOutliner, rBackgroundColor, bHeadLine);
        }
        else
        {
            OutlinerParaObject* pOutlinerParagraphObject = pCurrentObject->GetOutlinerParaObject();
            if (pOutlinerParagraphObject != nullptr)
            {
                WriteOutlinerParagraph(aStr, pOutliner, pOutlinerParagraphObject, rBackgroundColor, bHeadLine);
            }
        }
    }
}

void HtmlExport::WriteOutlinerParagraph(OUStringBuffer& aStr, SdrOutliner* pOutliner,
                                        OutlinerParaObject const * pOutlinerParagraphObject,
                                        const Color& rBackgroundColor, bool bHeadLine)
{
    if (pOutlinerParagraphObject == nullptr)
        return;

    pOutliner->SetText(*pOutlinerParagraphObject);

    sal_Int32 nCount = pOutliner->GetParagraphCount();


    sal_Int16 nCurrentDepth = -1;

    for (sal_Int32 nIndex = 0; nIndex < nCount; nIndex++)
    {
        Paragraph* pParagraph = pOutliner->GetParagraph(nIndex);
        if(pParagraph == nullptr)
            continue;

        const sal_Int16 nDepth = static_cast<sal_uInt16>(pOutliner->GetDepth(nIndex));
        OUString aParaText = ParagraphToHTMLString(pOutliner, nIndex, rBackgroundColor);

        if (aParaText.isEmpty())
            continue;

        if (nDepth < 0)
        {
            OUString aTag = bHeadLine ? OUString("h2") : OUString("p");
            lclAppendStyle(aStr, aTag, getParagraphStyle(pOutliner, nIndex));

            aStr.append(aParaText);
            aStr.append("</").append(aTag).append(">\r\n");
        }
        else
        {
            while(nCurrentDepth < nDepth)
            {
                aStr.append("<ul>\r\n");
                nCurrentDepth++;
            }
            while(nCurrentDepth > nDepth)
            {
                aStr.append("</ul>\r\n");
                nCurrentDepth--;
            }
            lclAppendStyle(aStr, "li", getParagraphStyle(pOutliner, nIndex));
            aStr.append(aParaText);
            aStr.append("</li>\r\n");
        }
    }
    while(nCurrentDepth >= 0)
    {
        aStr.append("</ul>\r\n");
        nCurrentDepth--;
    }
    pOutliner->Clear();
}

// creates a outliner text for a note page
OUString HtmlExport::CreateTextForNotesPage( SdrOutliner* pOutliner,
                                           SdPage* pPage,
                                           const Color& rBackgroundColor )
{
    OUStringBuffer aStr;

    SdrTextObj* pTO = static_cast<SdrTextObj*>(pPage->GetPresObj(PRESOBJ_NOTES));

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
                lclAppendStyle(aStr, "p", getParagraphStyle(pOutliner, nPara));
                aStr.append(ParagraphToHTMLString(pOutliner, nPara, rBackgroundColor));
                aStr.append("</p>\r\n");
            }
        }
    }

    return aStr.makeStringAndClear();
}

// converts a paragraph of the outliner to html
OUString HtmlExport::ParagraphToHTMLString( SdrOutliner const * pOutliner, sal_Int32 nPara, const Color& rBackgroundColor )
{
    OUStringBuffer aStr;

    if(nullptr == pOutliner)
        return OUString();

    // TODO: MALTE!!!
    EditEngine& rEditEngine = *const_cast<EditEngine*>(&pOutliner->GetEditEngine());
    bool bOldUpdateMode = rEditEngine.GetUpdateMode();
    rEditEngine.SetUpdateMode(true);

    Paragraph* pPara = pOutliner->GetParagraph(nPara);
    if(nullptr == pPara)
        return OUString();

    HtmlState aState( (mbUserAttr || mbDocColors)  ? maTextColor : COL_BLACK );
    std::vector<sal_Int32> aPortionList;
    rEditEngine.GetPortions( nPara, aPortionList );

    sal_Int32 nPos1 = 0;
    for( sal_Int32 nPos2 : aPortionList )
    {
        ESelection aSelection( nPara, nPos1, nPara, nPos2);

        SfxItemSet aSet( rEditEngine.GetAttribs( aSelection ) );

        OUString aPortion(StringToHTMLString(rEditEngine.GetText( aSelection )));

        aStr.append(TextAttribToHTMLString( &aSet, &aState, rBackgroundColor ));
        aStr.append(aPortion);

        nPos1 = nPos2;
    }
    aStr.append(aState.Flush());
    rEditEngine.SetUpdateMode(bOldUpdateMode);

    return aStr.makeStringAndClear();
}

// Depending on the attributes of the specified set and the specified
// HtmlState, it creates the needed html tags in order to get the
// attributes
OUString HtmlExport::TextAttribToHTMLString( SfxItemSet const * pSet, HtmlState* pState, const Color& rBackgroundColor )
{
    OUStringBuffer aStr;

    if(nullptr == pSet)
        return OUString();

    OUString aLink, aTarget;
    if ( pSet->GetItemState( EE_FEATURE_FIELD ) == SfxItemState::SET )
    {
        const SvxFieldItem* pItem = pSet->GetItem<SvxFieldItem>( EE_FEATURE_FIELD );
        if(pItem)
        {
            const SvxURLField* pURL = dynamic_cast<const SvxURLField*>( pItem->GetField() );
            if(pURL)
            {
                aLink = pURL->GetURL();
                aTarget = pURL->GetTargetFrame();
            }
        }
    }

    bool bTemp;
    OUString aTemp;

    if ( pSet->GetItemState( EE_CHAR_WEIGHT ) == SfxItemState::SET )
    {
        bTemp = pSet->Get( EE_CHAR_WEIGHT ).GetWeight() == WEIGHT_BOLD;
        aTemp = pState->SetWeight( bTemp );
        if( bTemp )
            aStr.insert(0, aTemp);
        else
            aStr.append(aTemp);
    }

    if ( pSet->GetItemState( EE_CHAR_UNDERLINE ) == SfxItemState::SET )
    {
        bTemp = pSet->Get( EE_CHAR_UNDERLINE ).GetLineStyle() != LINESTYLE_NONE;
        aTemp = pState->SetUnderline( bTemp );
        if( bTemp )
            aStr.insert(0, aTemp);
        else
            aStr.append(aTemp);
    }

    if ( pSet->GetItemState( EE_CHAR_STRIKEOUT ) == SfxItemState::SET )
    {
        bTemp = pSet->Get( EE_CHAR_STRIKEOUT ).GetStrikeout() != STRIKEOUT_NONE;
        aTemp = pState->SetStrikeout( bTemp );
        if( bTemp )
            aStr.insert(0, aTemp);
        else
            aStr.append(aTemp);
    }

    if ( pSet->GetItemState( EE_CHAR_ITALIC ) == SfxItemState::SET )
    {
        bTemp = pSet->Get( EE_CHAR_ITALIC ).GetPosture() != ITALIC_NONE;
        aTemp = pState->SetItalic( bTemp );
        if( bTemp )
            aStr.insert(0, aTemp);
        else
            aStr.append(aTemp);
    }

    if(mbDocColors)
    {
        if ( pSet->GetItemState( EE_CHAR_COLOR ) == SfxItemState::SET )
        {
            Color aTextColor = pSet->Get( EE_CHAR_COLOR ).GetValue();
            if( aTextColor == COL_AUTO )
            {
                if( !rBackgroundColor.IsDark() )
                    aTextColor = COL_BLACK;
            }
            aStr.append(pState->SetColor( aTextColor ));
        }
    }

    if (!aLink.isEmpty())
        aStr.insert(0, pState->SetLink(aLink, aTarget));
    else
        aStr.append(pState->SetLink(aLink, aTarget));

    return aStr.makeStringAndClear();
}

// create HTML wrapper for picture files
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
            SdrObjListIter aIter(pPage, SdrIterMode::DeepWithGroups, true);

            SdrObject* pObject = aIter.Next();
            while (pObject)
            {
                SdAnimationInfo* pInfo     = SdDrawDocument::GetAnimationInfo(pObject);
                SdIMapInfo*      pIMapInfo = SdDrawDocument::GetIMapInfo(pObject);

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
                pPage = static_cast<SdPage*>(&(pPage->TRG_GetMasterPage()));
            else
                bMasterDone = true;
        }

        // HTML Head
        OUStringBuffer aStr(gaHTMLHeader);
        aStr.append(CreateMetaCharset());
        aStr.append("  <title>" ).append( StringToHTMLString(maPageNames[nSdPage]) ).append("</title>\r\n");

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
                    aStr.append("<meta http-equiv=\"refresh\" content=\"");
                    aStr.append(OUString::number(fSecs));
                    aStr.append("; URL=");

                    int nPage = nSdPage + 1;
                    if( nPage == mnSdPageCount )
                        nPage = 0;

                    aStr.append(maHTMLFiles[nPage]);

                    aStr.append("\">\r\n");
                }
            }
        }

        aStr.append("</head>\r\n");

        // HTML Body
        aStr.append(CreateBodyTag());

        if( mbSlideSound && pPage->IsSoundOn() )
            aStr.append(InsertSound(pPage->GetSoundFile()));

        // navigation bar
        if(!mbFrames )
            aStr.append(CreateNavBar(nSdPage, false));
        // Image
        aStr.append("<center>");
        aStr.append("<img src=\"");
        aStr.append(maImageFiles[nSdPage]);
        aStr.append("\" alt=\"\"");

        if (!aClickableObjects.empty())
            aStr.append(" USEMAP=\"#map0\"");

        aStr.append("></center>\r\n");

        // notes
        if(mbNotes && !mbFrames)
        {
            SdrOutliner* pOutliner = mpDoc->GetInternalOutliner();
            SdPage* pNotesPage = maNotesPages[ nSdPage ];
            OUString aNotesStr( CreateTextForNotesPage( pOutliner, pNotesPage, maBackColor) );
            pOutliner->Clear();

            if (!aNotesStr.isEmpty())
            {
                aStr.append("<h3>");
                aStr.append(RESTOHTML(STR_HTMLEXP_NOTES));
                aStr.append(":</h3><br>\r\n\r\n<p>");

                aStr.append(aNotesStr);
                aStr.append("\r\n</p>\r\n");
            }
        }

        // create Imagemap if necessary
        if (!aClickableObjects.empty())
        {
            aStr.append("<map name=\"map0\">\r\n");

            for (SdrObject* pObject : aClickableObjects)
            {
                SdAnimationInfo* pInfo     = SdDrawDocument::GetAnimationInfo(pObject);
                SdIMapInfo*      pIMapInfo = SdDrawDocument::GetIMapInfo(pObject);

                ::tools::Rectangle aRect(pObject->GetCurrentBoundRect());
                Point     aLogPos(aRect.TopLeft());
                bool      bIsSquare = aRect.GetWidth() == aRect.GetHeight();

                sal_uLong nPageWidth = pPage->GetSize().Width() - pPage->GetLeftBorder() -
                                   pPage->GetRightBorder();

                // BoundRect is relative to the physical page origin, not the
                // origin of ordinates
                aRect.Move(-pPage->GetLeftBorder(), -pPage->GetUpperBorder());

                double fLogicToPixel = static_cast<double>(mnWidthPixel) / nPageWidth;
                aRect.SetLeft( static_cast<long>(aRect.Left() * fLogicToPixel) );
                aRect.SetTop( static_cast<long>(aRect.Top() * fLogicToPixel) );
                aRect.SetRight( static_cast<long>(aRect.Right() * fLogicToPixel) );
                aRect.SetBottom( static_cast<long>(aRect.Bottom() * fLogicToPixel) );
                long nRadius = aRect.GetWidth() / 2;

                /**
                    insert areas into Imagemap of the object, if the object has
                    such a Imagemap
                */
                if (pIMapInfo)
                {
                    const ImageMap& rIMap = pIMapInfo->GetImageMap();
                    sal_uInt16 nAreaCount = rIMap.GetIMapObjectCount();
                    for (sal_uInt16 nArea = 0; nArea < nAreaCount; nArea++)
                    {
                        IMapObject* pArea = rIMap.GetIMapObject(nArea);
                        sal_uInt16 nType = pArea->GetType();
                        OUString aURL( pArea->GetURL() );

                        // if necessary, convert page and object names into the
                        // corresponding names of the html file
                        bool        bIsMasterPage;
                        sal_uInt16  nPgNum = mpDoc->GetPageByName( aURL, bIsMasterPage );
                        SdrObject*  pObj = nullptr;

                        if (nPgNum == SDRPAGE_NOTFOUND)
                        {
                            // is the bookmark a object?
                            pObj = mpDoc->GetObj( aURL );
                            if (pObj)
                                nPgNum = pObj->getSdrPageFromSdrObject()->GetPageNum();
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
                                ::tools::Rectangle aArea(static_cast<IMapRectangleObject*>(pArea)->
                                                 GetRectangle(false));

                                // conversion into pixel coordinates
                                aArea.Move(aLogPos.X() - pPage->GetLeftBorder(),
                                           aLogPos.Y() - pPage->GetUpperBorder());
                                aArea.SetLeft( static_cast<long>(aArea.Left() * fLogicToPixel) );
                                aArea.SetTop( static_cast<long>(aArea.Top() * fLogicToPixel) );
                                aArea.SetRight( static_cast<long>(aArea.Right() * fLogicToPixel) );
                                aArea.SetBottom( static_cast<long>(aArea.Bottom() * fLogicToPixel) );

                                aStr.append(CreateHTMLRectArea(aArea, aURL));
                            }
                            break;

                            case IMAP_OBJ_CIRCLE:
                            {
                                Point aCenter(static_cast<IMapCircleObject*>(pArea)->
                                                 GetCenter(false));
                                aCenter += Point(aLogPos.X() - pPage->GetLeftBorder(),
                                                 aLogPos.Y() - pPage->GetUpperBorder());
                                aCenter.setX( static_cast<long>(aCenter.X() * fLogicToPixel) );
                                aCenter.setY( static_cast<long>(aCenter.Y() * fLogicToPixel) );

                                sal_uLong nCircleRadius = static_cast<IMapCircleObject*>(pArea)->
                                                 GetRadius(false);
                                nCircleRadius = static_cast<sal_uLong>(nCircleRadius * fLogicToPixel);
                                aStr.append(CreateHTMLCircleArea(nCircleRadius,
                                                            aCenter.X(), aCenter.Y(),
                                                            aURL));
                            }
                            break;

                            case IMAP_OBJ_POLYGON:
                            {
                                tools::Polygon aArea(static_cast<IMapPolygonObject*>(pArea)->GetPolygon(false));
                                aStr.append(CreateHTMLPolygonArea(::basegfx::B2DPolyPolygon(aArea.getB2DPolygon()),
                                                                  Size(aLogPos.X() - pPage->GetLeftBorder(),
                                                                       aLogPos.Y() - pPage->GetUpperBorder()),
                                                                  fLogicToPixel, aURL));
                            }
                            break;

                            default:
                            {
                                SAL_INFO("sd", "unknown IMAP_OBJ_type");
                            }
                            break;
                        }
                    }
                }

                /**
                    if there is a presentation::ClickAction, determine bookmark
                    and create area for the whole object
                */
                if( pInfo )
                {
                    OUString aHRef;
                    presentation::ClickAction eClickAction = pInfo->meClickAction;

                    switch( eClickAction )
                    {
                        case presentation::ClickAction_BOOKMARK:
                        {
                            bool        bIsMasterPage;
                            sal_uInt16  nPgNum = mpDoc->GetPageByName( pInfo->GetBookmark(), bIsMasterPage );
                            SdrObject*  pObj = nullptr;

                            if( nPgNum == SDRPAGE_NOTFOUND )
                            {
                                // is the bookmark a object?
                                pObj = mpDoc->GetObj(pInfo->GetBookmark());
                                if (pObj)
                                    nPgNum = pObj->getSdrPageFromSdrObject()->GetPageNum();
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
                            sal_uLong nPage;

                            if (nSdPage == 0)
                                nPage = 0;
                            else
                                nPage = nSdPage - 1;

                            aHRef = CreatePageURL( static_cast<sal_uInt16>(nPage));
                        }
                        break;

                        case presentation::ClickAction_NEXTPAGE:
                        {
                            sal_uLong nPage;
                            if (nSdPage == mnSdPageCount - 1)
                                nPage = mnSdPageCount - 1;
                            else
                                nPage = nSdPage + 1;

                            aHRef = CreatePageURL( static_cast<sal_uInt16>(nPage));
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
                    if (!aHRef.isEmpty())
                    {
                        // a circle?
                        if (pObject->GetObjInventor() == SdrInventor::Default &&
                            pObject->GetObjIdentifier() == OBJ_CIRC  &&
                            bIsSquare )
                        {
                            aStr.append(CreateHTMLCircleArea(aRect.GetWidth() / 2,
                                                    aRect.Left() + nRadius,
                                                    aRect.Top() + nRadius,
                                                    aHRef));
                        }
                        // a polygon?
                        else if (pObject->GetObjInventor() == SdrInventor::Default &&
                                 (pObject->GetObjIdentifier() == OBJ_PATHLINE ||
                                  pObject->GetObjIdentifier() == OBJ_PLIN ||
                                  pObject->GetObjIdentifier() == OBJ_POLY))
                        {
                            aStr.append(CreateHTMLPolygonArea(static_cast<SdrPathObj*>(pObject)->GetPathPoly(), Size(-pPage->GetLeftBorder(), -pPage->GetUpperBorder()), fLogicToPixel, aHRef));
                        }
                        // something completely different: use the BoundRect
                        else
                        {
                            aStr.append(CreateHTMLRectArea(aRect, aHRef));
                        }

                    }
                }
            }

            aStr.append("</map>\r\n");
        }
        aClickableObjects.clear();

        aStr.append("</body>\r\n</html>");

        bOk = WriteHtml(maHTMLFiles[nSdPage], false, aStr.makeStringAndClear());

        if (mpProgress)
            mpProgress->SetState(++mnPagesWritten);
    }

    return bOk;
}

// create overview pages
bool HtmlExport::CreateContentPage()
{
    if( mbDocColors )
        SetDocColors();

    // html head
    OUStringBuffer aStr(gaHTMLHeader);
    aStr.append(CreateMetaCharset());
    aStr.append("  <title>");
    aStr.append(StringToHTMLString(maPageNames[0]));
    aStr.append("</title>\r\n</head>\r\n");
    aStr.append(CreateBodyTag());

    // page head
    aStr.append("<center>\r\n");

    if(mbHeader)
    {
        aStr.append("<h1>");
        aStr.append(getDocumentTitle());
        aStr.append("</h1><br>\r\n");
    }

    aStr.append("<h2>");

    // Solaris compiler bug workaround
    if( mbFrames )
        aStr.append(CreateLink(maFramePage,
                               RESTOHTML(STR_HTMLEXP_CLICKSTART)));
    else
        aStr.append(CreateLink(StringToHTMLString(maHTMLFiles[0]),
                               RESTOHTML(STR_HTMLEXP_CLICKSTART)));

    aStr.append("</h2>\r\n</center>\r\n");

    aStr.append("<center><table width=\"90%\"><tr>\r\n");

    // table of content
    aStr.append("<td valign=\"top\" align=\"left\" width=\"25%\">\r\n");
    aStr.append("<h3>");
    aStr.append(RESTOHTML(STR_HTMLEXP_CONTENTS));
    aStr.append("</h3>");

    for(sal_uInt16 nSdPage = 0; nSdPage < mnSdPageCount; nSdPage++)
    {
        OUString aPageName = maPageNames[nSdPage];
        aStr.append("<div align=\"left\">");
        if(mbFrames)
            aStr.append(StringToHTMLString(aPageName));
        else
            aStr.append(CreateLink(maHTMLFiles[nSdPage], aPageName));
        aStr.append("</div>\r\n");
    }
    aStr.append("</td>\r\n");

    // document information
    aStr.append("<td valign=\"top\" align=\"left\" width=\"75%\">\r\n");

    if (!maAuthor.isEmpty())
    {
        aStr.append("<p><strong>");
        aStr.append(RESTOHTML(STR_HTMLEXP_AUTHOR));
        aStr.append(":</strong> ");
        aStr.append(StringToHTMLString(maAuthor));
        aStr.append("</p>\r\n");
    }

    if (!maEMail.isEmpty())
    {
        aStr.append("<p><strong>");
        aStr.append(RESTOHTML(STR_HTMLEXP_EMAIL));
        aStr.append(":</strong> <a href=\"mailto:");
        aStr.append(maEMail);
        aStr.append("\">");
        aStr.append(StringToHTMLString(maEMail));
        aStr.append("</a></p>\r\n");
    }

    if (!maHomePage.isEmpty())
    {
        aStr.append("<p><strong>");
        aStr.append(RESTOHTML(STR_HTMLEXP_HOMEPAGE));
        aStr.append(":</strong> <a href=\"");
        aStr.append(maHomePage);
        aStr.append("\">");
        aStr.append(StringToHTMLString(maHomePage));
        aStr.append("</a> </p>\r\n");
    }

    if (!maInfo.isEmpty())
    {
        aStr.append("<p><strong>");
        aStr.append(RESTOHTML(STR_HTMLEXP_INFO));
        aStr.append(":</strong><br>\r\n");
        aStr.append(StringToHTMLString(maInfo));
        aStr.append("</p>\r\n");
    }

    if(mbDownload)
    {
        aStr.append("<p><a href=\"");
        aStr.append(maDocFileName);
        aStr.append("\">");
        aStr.append(RESTOHTML(STR_HTMLEXP_DOWNLOAD));
        aStr.append("</a></p>\r\n");
    }

    for(sal_uInt16 nSdPage = 0; nSdPage < mnSdPageCount; nSdPage++)
    {
        OUString aText(
            "<img src=\"" +
            maThumbnailFiles[nSdPage] +
            "\" width=\"256\" height=\"192\" alt=\"" +
            StringToHTMLString(maPageNames[nSdPage]) +
            "\">");

        aStr.append(CreateLink(maHTMLFiles[nSdPage], aText));
        aStr.append("\r\n");
    }

    aStr.append("</td></tr></table></center>\r\n");

    aStr.append("</body>\r\n</html>");

    bool bOk = WriteHtml(maIndex, false, aStr.makeStringAndClear());

    if (mpProgress)
        mpProgress->SetState(++mnPagesWritten);

    return bOk;
}

// create note pages (for frames)

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
        OUStringBuffer aStr(gaHTMLHeader);
        aStr.append(CreateMetaCharset());
        aStr.append("  <title>");
        aStr.append(StringToHTMLString(maPageNames[0]));
        aStr.append("</title>\r\n</head>\r\n");
        aStr.append(CreateBodyTag());

        if(pPage)
            aStr.append(CreateTextForNotesPage( pOutliner, pPage, maBackColor ));

        aStr.append("</body>\r\n</html>");

        OUString aFileName("note" + OUString::number(nSdPage));
        bOk = WriteHtml(aFileName, true, aStr.makeStringAndClear());

        if (mpProgress)
            mpProgress->SetState(++mnPagesWritten);
    }

    pOutliner->Clear();

    return bOk;
}

// create outline pages (for frames)

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
        OUStringBuffer aStr(gaHTMLHeader);
        aStr.append(CreateMetaCharset());
        aStr.append("  <title>");
        aStr.append(StringToHTMLString(maPageNames[0]));
        aStr.append("</title>\r\n</head>\r\n");
        aStr.append(CreateBodyTag());

        SdrOutliner* pOutliner = mpDoc->GetInternalOutliner();
        for(sal_uInt16 nSdPage = 0; nSdPage < mnSdPageCount; nSdPage++)
        {
            SdPage* pPage = maPages[ nSdPage ];

            aStr.append("<div align=\"left\">");
            OUString aLink("JavaScript:parent.NavigateAbs(" +
                OUString::number(nSdPage) + ")");

            OUString aTitle = CreateTextForTitle(pOutliner, pPage, maBackColor);
            if (aTitle.isEmpty())
                aTitle = maPageNames[nSdPage];

            lclAppendStyle(aStr, "p", getParagraphStyle(pOutliner, 0));
            aStr.append(CreateLink(aLink, aTitle));
            aStr.append("</p>");

            if(nPage==1)
            {
                aStr.append(CreateTextForPage( pOutliner, pPage, false, maBackColor ));
            }
            aStr.append("</div>\r\n");
        }
        pOutliner->Clear();

        aStr.append("</body>\r\n</html>");

        OUString aFileName("outline" + OUString::number(nPage));
        bOk = WriteHtml(aFileName, true, aStr.makeStringAndClear());

        if (mpProgress)
            mpProgress->SetState(++mnPagesWritten);
    }

    return bOk;
}

// set file name
void HtmlExport::CreateFileNames()
{
    // create lists with new file names
    maHTMLFiles.resize(mnSdPageCount);
    maImageFiles.resize(mnSdPageCount);
    maThumbnailFiles.resize(mnSdPageCount);
    maPageNames.resize(mnSdPageCount);
    maTextFiles.resize(mnSdPageCount);

    mbHeader = false;   // headline on overview page?

    for (sal_uInt16 nSdPage = 0; nSdPage < mnSdPageCount; nSdPage++)
    {
        OUString aHTMLFileName;
        if(nSdPage == 0 && !mbContentsPage && !mbFrames )
            aHTMLFileName = maIndex;
        else
        {
            aHTMLFileName = "img" + OUString::number(nSdPage) + gaHTMLExtension;
        }

        maHTMLFiles[nSdPage] = aHTMLFileName;

        OUString aImageFileName = "img" + OUString::number(nSdPage);
        if( meFormat==FORMAT_GIF )
            aImageFileName += ".gif";
        else if( meFormat==FORMAT_JPG )
            aImageFileName += ".jpg";
        else
            aImageFileName += ".png";

        maImageFiles[nSdPage] = aImageFileName;

        OUString aThumbnailFileName = "thumb" + OUString::number(nSdPage);
        if( meFormat!=FORMAT_JPG )
            aThumbnailFileName += ".png";
        else
            aThumbnailFileName += ".jpg";

        maThumbnailFiles[nSdPage] = aThumbnailFileName;

        maTextFiles[nSdPage] = "text" + OUString::number(nSdPage) + gaHTMLExtension;

        SdPage* pSdPage = maPages[ nSdPage ];

        // get slide title from page name
        maPageNames[nSdPage] = pSdPage->GetName();
    }

    if(!mbContentsPage && mbFrames)
        maFramePage = maIndex;
    else
    {
        maFramePage = "siframes" + gaHTMLExtension;
    }
}

OUString const & HtmlExport::getDocumentTitle()
{
    // check for a title object in this page, if it's the first
    // title it becomes this documents title for the content
    // page
    if( !mbHeader )
    {
        if(mbImpress)
        {
            // if there is a non-empty title object, use their first passage
            // as page title
            SdPage* pSdPage = mpDoc->GetSdPage(0, PageKind::Standard);
            SdrObject* pTitleObj = pSdPage->GetPresObj(PRESOBJ_TITLE);
            if (pTitleObj && !pTitleObj->IsEmptyPresObj())
            {
                OutlinerParaObject* pParaObject = pTitleObj->GetOutlinerParaObject();
                if (pParaObject)
                {
                    const EditTextObject& rEditTextObject =
                        pParaObject->GetTextObject();
                    OUString aTest(rEditTextObject.GetText(0));
                    if (!aTest.isEmpty())
                        mDocTitle = aTest;
                }
            }

            mDocTitle = mDocTitle.replace(0xff, ' ');
        }

        if (mDocTitle.isEmpty())
        {
            mDocTitle = maDocFileName;
            sal_Int32 nDot = mDocTitle.indexOf('.');
            if (nDot > 0)
                mDocTitle = mDocTitle.copy(0, nDot);
        }
        mbHeader = true;
    }

    return mDocTitle;
}

static const char JS_NavigateAbs[] =
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

static const char JS_NavigateRel[] =
    "function NavigateRel( nDelta )\r\n"
    "{\r\n"
    "  var nPage = parseInt(nCurrentPage) + parseInt(nDelta);\r\n"
    "  if( (nPage >= 0) && (nPage < nPageCount) )\r\n"
    "  {\r\n"
    "    NavigateAbs( nPage );\r\n"
    "  }\r\n"
    "}\r\n\r\n";

static const char JS_ExpandOutline[] =
    "function ExpandOutline()\r\n"
    "{\r\n"
    "  frames[\"navbar2\"].location.href = \"navbar4.$EXT\";\r\n"
    "  frames[\"outline\"].location.href = \"outline1.$EXT\";\r\n"
    "}\r\n\r\n";

static const char JS_CollapseOutline[] =
    "function CollapseOutline()\r\n"
    "{\r\n"
    "  frames[\"navbar2\"].location.href = \"navbar3.$EXT\";\r\n"
    "  frames[\"outline\"].location.href = \"outline0.$EXT\";\r\n"
    "}\r\n\r\n";

// create page with the frames

bool HtmlExport::CreateFrames()
{
    OUString aTmp;
    OUStringBuffer aStr(
        "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Frameset//EN\"\r\n"
        "    \"http://www.w3.org/TR/html4/frameset.dtd\">\r\n"
        "<html>\r\n<head>\r\n");

    aStr.append(CreateMetaCharset());
    aStr.append("  <title>");
    aStr.append(StringToHTMLString(maPageNames[0]));
    aStr.append("</title>\r\n");

    aStr.append("<script type=\"text/javascript\">\r\n<!--\r\n");

    aStr.append("var nCurrentPage = 0;\r\nvar nPageCount = ");
    aStr.append(OUString::number(mnSdPageCount));
    aStr.append(";\r\n\r\n");

    OUString aFunction = JS_NavigateAbs;

    if(mbNotes)
    {
        aFunction = aFunction.replaceAll("//", "");
    }

    // substitute HTML file extension
    OUString aPlaceHolder(".$EXT");
    aFunction = aFunction.replaceAll(aPlaceHolder, gaHTMLExtension);
    aStr.append(aFunction);

    aTmp = JS_NavigateRel;
    aTmp = aTmp.replaceAll(aPlaceHolder, gaHTMLExtension);
    aStr.append(aTmp);

    if(mbImpress)
    {
        aTmp = JS_ExpandOutline;
        aTmp = aTmp.replaceAll(aPlaceHolder, gaHTMLExtension);
        aStr.append(aTmp);

        aTmp = JS_CollapseOutline;
        aTmp = aTmp.replaceAll(aPlaceHolder, gaHTMLExtension);
        aStr.append(aTmp);
    }
    aStr.append("// -->\r\n</script>\r\n");

    aStr.append("</head>\r\n");

    aStr.append("<frameset cols=\"*,");
    aStr.append(OUString::number((mnWidthPixel + 16)));
    aStr.append("\">\r\n");
    if(mbImpress)
    {
        aStr.append("  <frameset rows=\"42,*\">\r\n");
        aStr.append("    <frame src=\"navbar3");
        aStr.append(gaHTMLExtension);
        aStr.append("\" name=\"navbar2\" marginwidth=\"4\" marginheight=\"4\" scrolling=\"no\">\r\n");
    }
    aStr.append("    <frame src=\"outline0");
    aStr.append(gaHTMLExtension);
    aStr.append("\" name=\"outline\">\r\n");
    if(mbImpress)
        aStr.append("  </frameset>\r\n");

    if(mbNotes)
    {
        aStr.append("  <frameset rows=\"42,");
        aStr.append(OUString::number(static_cast<int>(static_cast<double>(mnWidthPixel) * 0.75) + 16));
        aStr.append(",*\">\r\n");
    }
    else
        aStr.append("  <frameset rows=\"42,*\">\r\n");

    aStr.append("    <frame src=\"navbar0");
    aStr.append(gaHTMLExtension);
    aStr.append("\" name=\"navbar1\" marginwidth=\"4\" marginheight=\"4\" scrolling=\"no\">\r\n");

    aStr.append("    <frame src=\"");
    aStr.append(maHTMLFiles[0]);
    aStr.append("\" name=\"show\" marginwidth=\"4\" marginheight=\"4\">\r\n");

    if(mbNotes)
    {
        aStr.append("    <frame src=\"note0");
        aStr.append(gaHTMLExtension);
        aStr.append("\" name=\"notes\">\r\n");
    }
    aStr.append("  </frameset>\r\n");

    aStr.append("<noframes>\r\n");
    aStr.append(CreateBodyTag());
    aStr.append(RESTOHTML(STR_HTMLEXP_NOFRAMES));
    aStr.append("\r\n</noframes>\r\n</frameset>\r\n</html>");

    bool bOk = WriteHtml(maFramePage, false, aStr.makeStringAndClear());

    if (mpProgress)
        mpProgress->SetState(++mnPagesWritten);

    return bOk;
}

// create button bar for standard
// we create the following html files
// navbar0.htm navigation bar graphic for the first page
// navbar1.htm navigation bar graphic for the second until second last page
// navbar2.htm navigation bar graphic for the last page
// navbar3.htm navigation outline closed
// navbar4.htm navigation outline open
bool HtmlExport::CreateNavBarFrames()
{
    bool bOk = true;
    OUString aButton;

    if( mbDocColors )
    {
        SetDocColors();
        maBackColor = maFirstPageColor;
    }

    for( int nFile = 0; nFile < 3 && bOk; nFile++ )
    {
        OUStringBuffer aStr(gaHTMLHeader);
        aStr.append(CreateMetaCharset());
        aStr.append("  <title>");
        aStr.append(StringToHTMLString(maPageNames[0]));
        aStr.append("</title>\r\n</head>\r\n");
        aStr.append(CreateBodyTag());
        aStr.append("<center>\r\n");

        // first page
        aButton = SdResId(STR_HTMLEXP_FIRSTPAGE);
        if(mnButtonThema != -1)
            aButton = CreateImage(GetButtonName(nFile == 0 || mnSdPageCount == 1 ? BTN_FIRST_0 : BTN_FIRST_1),
                                  aButton);

        if(nFile != 0 && mnSdPageCount > 1)
            aButton = CreateLink("JavaScript:parent.NavigateAbs(0)", aButton);

        aStr.append(aButton);
        aStr.append("\r\n");

        // to the previous page
        aButton = SdResId(STR_PUBLISH_BACK);
        if(mnButtonThema != -1)
            aButton = CreateImage(GetButtonName(nFile == 0 || mnSdPageCount == 1?
                                    BTN_PREV_0:BTN_PREV_1),
                                  aButton);

        if(nFile != 0 && mnSdPageCount > 1)
            aButton = CreateLink("JavaScript:parent.NavigateRel(-1)", aButton);

        aStr.append(aButton);
        aStr.append("\r\n");

        // to the next page
        aButton = SdResId(STR_PUBLISH_NEXT);
        if(mnButtonThema != -1)
            aButton = CreateImage(GetButtonName(nFile ==2 || mnSdPageCount == 1?
                                    BTN_NEXT_0:BTN_NEXT_1),
                                  aButton);

        if(nFile != 2 && mnSdPageCount > 1)
            aButton = CreateLink("JavaScript:parent.NavigateRel(1)", aButton);

        aStr.append(aButton);
        aStr.append("\r\n");

        // to the last page
        aButton = SdResId(STR_HTMLEXP_LASTPAGE);
        if(mnButtonThema != -1)
            aButton = CreateImage(GetButtonName(nFile ==2 || mnSdPageCount == 1?
                                      BTN_LAST_0:BTN_LAST_1),
                                  aButton);

        if(nFile != 2 && mnSdPageCount > 1)
        {
            OUString aLink("JavaScript:parent.NavigateAbs(" +
                OUString::number(mnSdPageCount-1) + ")");

            aButton = CreateLink(aLink, aButton);
        }

        aStr.append(aButton);
        aStr.append("\r\n");

        // content
        if (mbContentsPage)
        {
            aButton = SdResId(STR_PUBLISH_OUTLINE);
            if(mnButtonThema != -1)
                aButton = CreateImage(GetButtonName(BTN_INDEX), aButton);

            // to the overview
            aStr.append(CreateLink(maIndex, aButton, "_top"));
            aStr.append("\r\n");
        }

        // text mode
        if(mbImpress)
        {
            aButton = SdResId(STR_HTMLEXP_SETTEXT);
            if(mnButtonThema != -1)
                aButton = CreateImage(GetButtonName(BTN_TEXT), aButton);

            OUString aText0("text0" + gaHTMLExtension);
            aStr.append(CreateLink(aText0, aButton, "_top"));
            aStr.append("\r\n");
        }

        // and finished...
        aStr.append("</center>\r\n");
        aStr.append("</body>\r\n</html>");

        OUString aFileName("navbar" + OUString::number(nFile));

        bOk = WriteHtml(aFileName, true, aStr.makeStringAndClear());

        if (mpProgress)
            mpProgress->SetState(++mnPagesWritten);
    }

    // the navigation bar outliner closed...
    if(bOk)
    {
        OUStringBuffer aStr(gaHTMLHeader);
        aStr.append(CreateMetaCharset());
        aStr.append("  <title>");
        aStr.append(StringToHTMLString(maPageNames[0]));
        aStr.append("</title>\r\n</head>\r\n");
        aStr.append(CreateBodyTag());

        aButton = SdResId(STR_HTMLEXP_OUTLINE);
        if(mnButtonThema != -1)
            aButton = CreateImage(GetButtonName(BTN_MORE), aButton);

        aStr.append(CreateLink("JavaScript:parent.ExpandOutline()", aButton));
        aStr.append("</body>\r\n</html>");

        bOk = WriteHtml("navbar3", true, aStr.makeStringAndClear());

        if (mpProgress)
            mpProgress->SetState(++mnPagesWritten);
    }

    // ... and the outliner open
    if( bOk )
    {
        OUStringBuffer aStr(gaHTMLHeader);
        aStr.append(CreateMetaCharset());
        aStr.append("  <title>");
        aStr.append(StringToHTMLString(maPageNames[0]));
        aStr.append("</title>\r\n</head>\r\n");
        aStr.append(CreateBodyTag());

        aButton = SdResId(STR_HTMLEXP_NOOUTLINE);
        if(mnButtonThema != -1)
            aButton = CreateImage(GetButtonName(BTN_LESS), aButton);

        aStr.append(CreateLink("JavaScript:parent.CollapseOutline()", aButton));
        aStr.append("</body>\r\n</html>");

        bOk = WriteHtml("navbar4", true, aStr.makeStringAndClear());

        if (mpProgress)
            mpProgress->SetState(++mnPagesWritten);

    }

    return bOk;
}

// create button bar for standard
OUString HtmlExport::CreateNavBar( sal_uInt16 nSdPage, bool bIsText ) const
{
    // prepare button bar
    OUString aStrNavFirst(SdResId(STR_HTMLEXP_FIRSTPAGE));
    OUString aStrNavPrev(SdResId(STR_PUBLISH_BACK));
    OUString aStrNavNext(SdResId(STR_PUBLISH_NEXT));
    OUString aStrNavLast(SdResId(STR_HTMLEXP_LASTPAGE));
    OUString aStrNavContent(SdResId(STR_PUBLISH_OUTLINE));
    OUString aStrNavText;
    if( bIsText )
    {
        aStrNavText = SdResId(STR_HTMLEXP_SETGRAPHIC);
    }
    else
    {
        aStrNavText = SdResId(STR_HTMLEXP_SETTEXT);
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

    OUStringBuffer aStr("<center>\r\n"); //<table><tr>\r\n");

    // first page
    if(nSdPage > 0)
        aStr.append(CreateLink( bIsText ? maTextFiles[0] : maHTMLFiles[0],aStrNavFirst));
    else
        aStr.append(aStrNavFirst);
    aStr.append(' ');

    // to Previous page
    if(nSdPage > 0)
        aStr.append(CreateLink( bIsText ? maTextFiles[nSdPage-1]
                                        : maHTMLFiles[nSdPage-1], aStrNavPrev));
    else
        aStr.append(aStrNavPrev);
    aStr.append(' ');

    // to Next page
    if(nSdPage < mnSdPageCount-1)
        aStr.append(CreateLink( bIsText ? maTextFiles[nSdPage+1]
                                        : maHTMLFiles[nSdPage+1], aStrNavNext));
    else
        aStr.append(aStrNavNext);
    aStr.append(' ');

    // to Last page
    if(nSdPage < mnSdPageCount-1)
        aStr.append(CreateLink( bIsText ? maTextFiles[mnSdPageCount-1]
                                        : maHTMLFiles[mnSdPageCount-1],
                                aStrNavLast));
    else
        aStr.append(aStrNavLast);
    aStr.append(' ');

    // to Index page
    if (mbContentsPage)
    {
        aStr.append(CreateLink(maIndex, aStrNavContent));
        aStr.append(' ');
    }

    // Text/Graphics
    if(mbImpress)
    {
        aStr.append(CreateLink( bIsText ? (mbFrames ? maFramePage : maHTMLFiles[nSdPage])
                                        : maTextFiles[nSdPage], aStrNavText));

    }

    aStr.append("</center><br>\r\n");

    return aStr.makeStringAndClear();
}

// export navigation graphics from button set
void HtmlExport::CreateBitmaps()
{
    if(mnButtonThema == -1 || !mpButtonSet.get())
        return;

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

// creates the <body> tag, including the specified color attributes
OUString HtmlExport::CreateBodyTag() const
{
    OUStringBuffer aStr( "<body" );

    if( mbUserAttr || mbDocColors )
    {
        Color aTextColor( maTextColor );
        if( (aTextColor == COL_AUTO) && (!maBackColor.IsDark()) )
            aTextColor = COL_BLACK;

        aStr.append(" text=\"");
        aStr.append(ColorToHTMLString( aTextColor ));
        aStr.append("\" bgcolor=\"");
        aStr.append(ColorToHTMLString( maBackColor ));
        aStr.append("\" link=\"");
        aStr.append(ColorToHTMLString( maLinkColor ));
        aStr.append("\" vlink=\"");
        aStr.append(ColorToHTMLString( maVLinkColor ));
        aStr.append("\" alink=\"");
        aStr.append(ColorToHTMLString( maALinkColor ));
        aStr.append("\"");
    }

    aStr.append(">\r\n");

    return aStr.makeStringAndClear();
}

// creates a hyperlink
OUString HtmlExport::CreateLink( const OUString& aLink,
                                 const OUString& aText,
                                 const OUString& aTarget )
{
    OUStringBuffer aStr( "<a href=\"" );
    aStr.append(aLink);
    if (!aTarget.isEmpty())
    {
        aStr.append("\" target=\"");
        aStr.append(aTarget);
    }
    aStr.append("\">");
    aStr.append(aText);
    aStr.append("</a>");

    return aStr.makeStringAndClear();
}

// creates a image tag
OUString HtmlExport::CreateImage( const OUString& aImage, const OUString& aAltText )
{
    OUStringBuffer aStr( "<img src=\"");
    aStr.append(aImage);
    aStr.append("\" border=0");

    if (!aAltText.isEmpty())
    {
        aStr.append(" alt=\"");
        aStr.append(aAltText);
        aStr.append('"');
    }
    else
    {
        // Agerskov: HTML 4.01 has to have an alt attribute even if it is an empty string
        aStr.append(" alt=\"\"");
    }

    aStr.append('>');

    return aStr.makeStringAndClear();
}

// create area for a circle; we expect pixel coordinates
OUString HtmlExport::ColorToHTMLString( Color aColor )
{
    static const char hex[] = "0123456789ABCDEF";
    OUStringBuffer aStr( "#xxxxxx" );
    aStr[1] = hex[(aColor.GetRed() >> 4) & 0xf];
    aStr[2] = hex[aColor.GetRed() & 0xf];
    aStr[3] = hex[(aColor.GetGreen() >> 4) & 0xf];
    aStr[4] = hex[aColor.GetGreen() & 0xf];
    aStr[5] = hex[(aColor.GetBlue() >> 4) & 0xf];
    aStr[6] = hex[aColor.GetBlue() & 0xf];

    return aStr.makeStringAndClear();
}

// create area for a circle; we expect pixel coordinates
OUString HtmlExport::CreateHTMLCircleArea( sal_uLong nRadius,
                                         sal_uLong nCenterX,
                                         sal_uLong nCenterY,
                                         const OUString& rHRef )
{
    OUString aStr(
        "<area shape=\"circle\" alt=\"\" coords=\"" +
        OUString::number(nCenterX) + "," +
        OUString::number(nCenterY) + "," +
        OUString::number(nRadius) +
        "\" href=\"" + rHRef + "\">\n");

    return aStr;
}

// create area for a polygon; we expect pixel coordinates
OUString HtmlExport::CreateHTMLPolygonArea( const ::basegfx::B2DPolyPolygon& rPolyPolygon,
    Size aShift, double fFactor, const OUString& rHRef )
{
    OUStringBuffer aStr;
    const sal_uInt32 nNoOfPolygons(rPolyPolygon.count());

    for ( sal_uInt32 nXPoly = 0; nXPoly < nNoOfPolygons; nXPoly++ )
    {
        const ::basegfx::B2DPolygon& aPolygon = rPolyPolygon.getB2DPolygon(nXPoly);
        const sal_uInt32 nNoOfPoints(aPolygon.count());

        aStr.append("<area shape=\"polygon\" alt=\"\" coords=\"");

        for ( sal_uInt32 nPoint = 0; nPoint < nNoOfPoints; nPoint++ )
        {
            const ::basegfx::B2DPoint aB2DPoint(aPolygon.getB2DPoint(nPoint));
            Point aPnt(FRound(aB2DPoint.getX()), FRound(aB2DPoint.getY()));
            // coordinates are relative to the physical page origin, not the
            // origin of ordinates
            aPnt.Move(aShift.Width(), aShift.Height());

            aPnt.setX( static_cast<long>(aPnt.X() * fFactor) );
            aPnt.setY( static_cast<long>(aPnt.Y() * fFactor) );
            aStr.append(OUString::number(aPnt.X())).append(",").append(OUString::number(aPnt.Y()));

            if (nPoint < nNoOfPoints - 1)
                aStr.append(',');
        }
        aStr.append("\" href=\"").append(rHRef).append("\">\n");
    }

    return aStr.makeStringAndClear();
}

// create area for a rectangle; we expect pixel coordinates
OUString HtmlExport::CreateHTMLRectArea( const ::tools::Rectangle& rRect,
                                       const OUString& rHRef )
{
    OUString aStr(
        "<area shape=\"rect\" alt=\"\" coords=\"" +
        OUString::number(rRect.Left()) + "," +
        OUString::number(rRect.Top()) + "," +
        OUString::number(rRect.Right()) + "," +
        OUString::number(rRect.Bottom()) +
        "\" href=\"" + rHRef + "\">\n");

    return aStr;
}

// escapes a string for html
OUString HtmlExport::StringToHTMLString( const OUString& rString )
{
    SvMemoryStream aMemStm;
    HTMLOutFuncs::Out_String( aMemStm, rString, RTL_TEXTENCODING_UTF8 );
    aMemStm.WriteChar( char(0) );
    sal_Int32 nLength = strlen(static_cast<char const *>(aMemStm.GetData()));
    return OUString( static_cast<char const *>(aMemStm.GetData()), nLength, RTL_TEXTENCODING_UTF8 );
}

// creates a url for a specific page
OUString HtmlExport::CreatePageURL( sal_uInt16 nPgNum )
{
    if(mbFrames)
    {
        return OUString("JavaScript:parent.NavigateAbs(" +
                        OUString::number(nPgNum) + ")");
    }
    else
        return maHTMLFiles[nPgNum];
}

bool HtmlExport::CopyScript( const OUString& rPath, const OUString& rSource, const OUString& rDest, bool bUnix /* = false */ )
{
    INetURLObject   aURL( SvtPathOptions().GetConfigPath() );
    OUStringBuffer aScriptBuf;

    aURL.Append( "webcast" );
    aURL.Append( rSource );

    meEC.SetContext( STR_HTMLEXP_ERROR_OPEN_FILE, rSource );

    ErrCode     nErr = ERRCODE_NONE;
    std::unique_ptr<SvStream> pIStm = ::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), StreamMode::READ );

    if( pIStm )
    {
        OString aLine;

        while( pIStm->ReadLine( aLine ) )
        {
            aScriptBuf.appendAscii( aLine.getStr() );
            if( bUnix )
            {
                aScriptBuf.append("\n");
            }
            else
            {
                aScriptBuf.append("\r\n");
            }
        }

        nErr = pIStm->GetError();
        pIStm.reset();
    }

    if( nErr != ERRCODE_NONE )
    {
        ErrorHandler::HandleError( nErr );
        return static_cast<bool>(nErr);
    }

    OUString aScript(aScriptBuf.makeStringAndClear());
    aScript = aScript.replaceAll("$$1", getDocumentTitle());
    aScript = aScript.replaceAll("$$2", RESTOHTML(STR_WEBVIEW_SAVE));
    aScript = aScript.replaceAll("$$3", maCGIPath);
    aScript = aScript.replaceAll("$$4", OUString::number(mnWidthPixel));
    aScript = aScript.replaceAll("$$5", OUString::number(mnHeightPixel));

    OUString aDest(rPath + rDest);

    meEC.SetContext( STR_HTMLEXP_ERROR_CREATE_FILE, rDest );
    // write script file
    {
        EasyFile aFile;
        SvStream* pStr;
        nErr = aFile.createStream(aDest, pStr);
        if(nErr == ERRCODE_NONE)
        {
            OString aStr(OUStringToOString(aScript,
                RTL_TEXTENCODING_UTF8));
            pStr->WriteCharPtr( aStr.getStr() );
            aFile.close();
        }
    }

    if (mpProgress)
        mpProgress->SetState(++mnPagesWritten);

    if( nErr != ERRCODE_NONE )
        ErrorHandler::HandleError( nErr );

    return nErr == ERRCODE_NONE;
}

static const char * ASP_Scripts[] = { "common.inc", "webcast.asp", "show.asp", "savepic.asp", "poll.asp", "editpic.asp" };

/** creates and saves the ASP scripts for WebShow */
bool HtmlExport::CreateASPScripts()
{
    for(const char * p : ASP_Scripts)
    {
        OUString aScript = OUString::createFromAscii(p);

        if(!CopyScript(maExportPath, aScript, aScript))
            return false;
    }

    return CopyScript(maExportPath, "edit.asp", maIndex);
}

static const char *PERL_Scripts[] = { "webcast.pl", "common.pl", "editpic.pl", "poll.pl", "savepic.pl", "show.pl" };

// creates and saves the PERL scripts for WebShow
bool HtmlExport::CreatePERLScripts()
{
    for(const char * p : PERL_Scripts)
    {
        OUString aScript = OUString::createFromAscii(p);

        if(!CopyScript(maExportPath, aScript, aScript, true))
            return false;
    }

    if (!CopyScript(maExportPath, "edit.pl", maIndex, true))
        return false;

    if (!CopyScript(maExportPath, "index.pl", maIndexUrl, true))
        return false;

    return true;
}

// creates a list with names of the saved images
bool HtmlExport::CreateImageFileList()
{
    OUStringBuffer aStr;
    for( sal_uInt16 nSdPage = 0; nSdPage < mnSdPageCount; nSdPage++)
    {
        aStr.append(OUString::number(nSdPage + 1));
        aStr.append(';');
        aStr.append(maURLPath);
        aStr.append(maImageFiles[nSdPage]);
        aStr.append("\r\n");
    }

    bool bOk = WriteHtml("picture.txt", false, aStr.makeStringAndClear());

    if (mpProgress)
        mpProgress->SetState(++mnPagesWritten);

    return bOk;
}

// creates a file with the actual page number
bool HtmlExport::CreateImageNumberFile()
{
    OUString aFileName("currpic.txt");
    OUString aFull(maExportPath + aFileName);

    meEC.SetContext( STR_HTMLEXP_ERROR_CREATE_FILE, aFileName );
    EasyFile aFile;
    SvStream* pStr;
    ErrCode nErr = aFile.createStream(aFull, pStr);
    if(nErr == ERRCODE_NONE)
    {
        pStr->WriteCharPtr( "1" );
        aFile.close();
    }

    if (mpProgress)
        mpProgress->SetState(++mnPagesWritten);

    if( nErr != ERRCODE_NONE )
        ErrorHandler::HandleError( nErr );

    return nErr == ERRCODE_NONE;
}

OUString HtmlExport::InsertSound( const OUString& rSoundFile )
{
    if (rSoundFile.isEmpty())
        return rSoundFile;

    INetURLObject   aURL( rSoundFile );
    OUString aSoundFileName = aURL.getName();

    DBG_ASSERT( aURL.GetProtocol() != INetProtocol::NotValid, "invalid URL" );

    OUString aStr("<embed src=\"" + aSoundFileName +
        "\" hidden=\"true\" autostart=\"true\">");

    CopyFile(rSoundFile, maExportPath + aSoundFileName);

    return aStr;
}

bool HtmlExport::CopyFile( const OUString& rSourceFile, const OUString& rDestFile )
{
    meEC.SetContext( STR_HTMLEXP_ERROR_COPY_FILE, rSourceFile, rDestFile );
    osl::FileBase::RC Error = osl::File::copy( rSourceFile, rDestFile );

    if( Error != osl::FileBase::E_None )
    {
        ErrorHandler::HandleError(ErrCode(Error));
        return false;
    }
    else
    {
        return true;
    }
}

bool HtmlExport::checkFileExists( Reference< css::ucb::XSimpleFileAccess3 > const & xFileAccess, OUString const & aFileName )
{
    try
    {
        OUString url( maExportPath );
        url += aFileName;
        return xFileAccess->exists( url );
    }
    catch( css::uno::Exception& )
    {
        SAL_WARN( "sd", "sd::HtmlExport::checkFileExists(), exception caught: "
                    << exceptionToString( cppu::getCaughtException() ) );
    }

    return false;
}

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
            if( checkFileExists( xFA, maImageFiles[nSdPage] ) ||
                checkFileExists( xFA, maHTMLFiles[nSdPage] ) ||
                checkFileExists( xFA, maThumbnailFiles[nSdPage] ) ||
                checkFileExists( xFA, maPageNames[nSdPage] ) ||
                checkFileExists( xFA, maTextFiles[nSdPage] ) )
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
            OUString aSystemPath;
            osl::FileBase::getSystemPathFromFileURL( maExportPath, aSystemPath );
            OUString aMsg(SdResId(STR_OVERWRITE_WARNING));
            aMsg = aMsg.replaceFirst( "%FILENAME", aSystemPath );

            std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(nullptr,
                                                       VclMessageType::Warning, VclButtonsType::YesNo,
                                                       aMsg));
            xWarn->set_default_response(RET_YES);
            bFound = (RET_NO == xWarn->run());
        }
    }
    catch( Exception& )
    {
        SAL_WARN( "sd", "sd::HtmlExport::checkForExistingFiles(), exception caught: "
                    << exceptionToString( cppu::getCaughtException() ) );
        bFound = false;
    }

    return bFound;
}

OUString HtmlExport::GetButtonName( int nButton )
{
    return OUString::createFromAscii(pButtonNames[nButton]);
}

EasyFile::EasyFile()
{
    pOStm = nullptr;
    bOpen = false;
}

EasyFile::~EasyFile()
{
    if( bOpen )
        close();
}

ErrCode EasyFile::createStream(  const OUString& rUrl, SvStream* &rpStr )
{
    if(bOpen)
        close();

    OUString aFileName;
    createFileName( rUrl, aFileName );

    ErrCode nErr = ERRCODE_NONE;
    pOStm = ::utl::UcbStreamHelper::CreateStream( aFileName, StreamMode::WRITE | StreamMode::TRUNC );
    if( pOStm )
    {
        bOpen = true;
        nErr = pOStm->GetError();
    }
    else
    {
        nErr = ERRCODE_SFX_CANTCREATECONTENT;
    }

    if( nErr != ERRCODE_NONE )
    {
        bOpen = false;
        pOStm.reset();
    }

    rpStr = pOStm.get();

    return nErr;
}

void EasyFile::createFileName(  const OUString& rURL, OUString& rFileName )
{
    if( bOpen )
        close();

    INetURLObject aURL( rURL );

    if( aURL.GetProtocol() == INetProtocol::NotValid )
    {
        OUString aURLStr;
        osl::FileBase::getFileURLFromSystemPath( rURL, aURLStr );
        aURL = INetURLObject( aURLStr );
    }
    DBG_ASSERT( aURL.GetProtocol() != INetProtocol::NotValid, "invalid URL" );
    rFileName = aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE );
}

void EasyFile::close()
{
    pOStm.reset();
    bOpen = false;
}

// This class helps reporting errors during file i/o
HtmlErrorContext::HtmlErrorContext()
    : ErrorContext(nullptr)
    , mpResId(nullptr)
{
}

bool HtmlErrorContext::GetString( ErrCode, OUString& rCtxStr )
{
    DBG_ASSERT(mpResId, "No error context set");
    if (!mpResId)
        return false;

    rCtxStr = SdResId(mpResId);

    rCtxStr = rCtxStr.replaceAll( "$(URL1)", maURL1 );
    rCtxStr = rCtxStr.replaceAll( "$(URL2)", maURL2 );

    return true;
}

void HtmlErrorContext::SetContext(const char* pResId, const OUString& rURL)
{
    mpResId = pResId;
    maURL1 = rURL;
    maURL2.clear();
}

void HtmlErrorContext::SetContext(const char* pResId, const OUString& rURL1, const OUString& rURL2 )
{
    mpResId = pResId;
    maURL1 = rURL1;
    maURL2 = rURL2;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
