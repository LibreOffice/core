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

#ifndef _SD_HTMLEX_HXX
#define _SD_HTMLEX_HXX

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess3.hpp>
#include <vcl/gdimtf.hxx>
#include <svl/itemset.hxx>
#include "resltn.hxx"       // enum PublishingResolution
#include <svtools/colrdlg.hxx>
#include <svtools/ehdl.hxx>

#include "strings.hrc"
#include "DrawDocShell.hxx"
#include "Window.hxx"
#include "ViewShell.hxx"
#include "assclass.hxx"

#include "sdresid.hxx"
#include "pubdlg.hxx"

#include <vector>
#include <boost/scoped_ptr.hpp>

#define NUM_BUTTONS 12

#define PUB_LOWRES_WIDTH    640
#define PUB_LOWRES_HEIGHT   480
#define PUB_MEDRES_WIDTH    800
#define PUB_MEDRES_HEIGHT   600
#define PUB_HIGHRES_WIDTH   1024
#define PUB_HIGHRES_HEIGHT  768

#define PUB_THUMBNAIL_WIDTH  256
#define PUB_THUMBNAIL_HEIGHT 192

class SfxProgress;
class SdrOutliner;
class SdPage;
class HtmlState;
class SdrTextObj;
class SdrPage;
class SdDrawDocument;
class ButtonSet;

class HtmlErrorContext : public ErrorContext
{
private:
    sal_uInt16  mnResId;
    String  maURL1;
    String  maURL2;

public:
                    HtmlErrorContext(Window *pWin=0);
                    ~HtmlErrorContext() {};

    virtual sal_Bool    GetString( sal_uLong nErrId, OUString& rCtxStr );

    void            SetContext( sal_uInt16 nResId, const String& rURL );
    void            SetContext( sal_uInt16 nResId, const String& rURL1, const String& rURL2 );
};

/// this class exports an Impress Document as a HTML Presentation.
class HtmlExport
{
    std::vector< SdPage* > maPages;
    std::vector< SdPage* > maNotesPages;

    String maPath;

    SdDrawDocument* mpDoc;
    ::sd::DrawDocShell* mpDocSh;

    HtmlErrorContext meEC;

    HtmlPublishMode meMode;
    SfxProgress* mpProgress;
    bool mbImpress;
    sal_uInt16 mnSdPageCount;
    sal_uInt16 mnPagesWritten;
    bool mbContentsPage;
    sal_Int16 mnButtonThema;
    sal_uInt16 mnWidthPixel;
    sal_uInt16 mnHeightPixel;
    PublishingFormat meFormat;
    bool mbHeader;
    bool mbNotes;
    bool mbFrames;
    String maIndex;
    String maEMail;
    String maAuthor;
    String maHomePage;
    String maInfo;
    sal_Int16 mnCompression;
    String maDocFileName;
    String maFramePage;
    String mDocTitle;
    bool mbDownload;

    bool mbAutoSlide;
    sal_uInt32  mnSlideDuration;
    bool mbSlideSound;
    bool mbHiddenSlides;
    bool mbEndless;

    bool mbUserAttr;
    Color maTextColor; ///< The following colors are used for the <body> tag if mbUserAttr is true.
    Color maBackColor;
    Color maLinkColor;
    Color maVLinkColor;
    Color maALinkColor;
    Color maFirstPageColor;
    bool mbDocColors;

    String   maHTMLExtension;
    String** mpHTMLFiles;
    String** mpImageFiles;
    String** mpThumbnailFiles;
    String** mpPageNames;
    String** mpTextFiles;

    String maExportPath; ///< output directory or URL.
    String maIndexUrl;
    String maURLPath;
    String maCGIPath;
    PublishingScript meScript;

    const String maHTMLHeader;

    boost::scoped_ptr< ButtonSet > mpButtonSet;

    SdrTextObj* GetLayoutTextObject(SdrPage* pPage);

    void SetDocColors( SdPage* pPage = NULL );

    bool        CreateImagesForPresPages( bool bThumbnails = false );
    bool    CreateHtmlTextForPresPages();
    bool    CreateHtmlForPresPages();
    bool    CreateContentPage();
    void    CreateFileNames();
    bool    CreateBitmaps();
    bool    CreateOutlinePages();
    bool    CreateFrames();
    bool    CreateNotesPages();
    bool    CreateNavBarFrames();

    bool    CreateASPScripts();
    bool    CreatePERLScripts();
    bool    CreateImageFileList();
    bool    CreateImageNumberFile();

    bool    checkForExistingFiles();
    bool    checkFileExists( ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess3 >& xFileAccess, String const & aFileName );

    String getDocumentTitle();
    bool    SavePresentation();

    String  CreateLink( const String& aLink, const String& aText,
                        const String& aTarget = String()) const;
    String  CreateImage( const String& aImage, const String& aAltText, sal_Int16 nWidth = -1, sal_Int16 nHeight = -1 ) const;
    String  CreateNavBar( sal_uInt16 nSdPage, bool bIsText ) const;
    String  CreateBodyTag() const;

    String  ParagraphToHTMLString( SdrOutliner* pOutliner, sal_uLong nPara, const Color& rBackgroundColor );
    String  TextAttribToHTMLString( SfxItemSet* pSet, HtmlState* pState, const Color& rBackgroundColor );

    String  CreateTextForTitle( SdrOutliner* pOutliner, SdPage* pPage, const Color& rBackgroundColor );
    String  CreateTextForPage( SdrOutliner* pOutliner, SdPage* pPage, bool bHeadLine, const Color& rBackgroundColor );
    String  CreateTextForNotesPage( SdrOutliner* pOutliner, SdPage* pPage, bool bHeadLine, const Color& rBackgroundColor );

    String  CreateHTMLCircleArea( sal_uLong nRadius, sal_uLong nCenterX,
                                  sal_uLong nCenterY, const String& rHRef ) const;
    String  CreateHTMLPolygonArea( const ::basegfx::B2DPolyPolygon& rPolyPoly, Size aShift, double fFactor, const String& rHRef ) const;
    String  CreateHTMLRectArea( const Rectangle& rRect,
                                const String& rHRef ) const;

    String  CreatePageURL( sal_uInt16 nPgNum );

    String InsertSound( const String& rSoundFile );
    bool CopyFile( const OUString& rSourceFile, const OUString& rDestFile );
    bool CopyScript( const String& rPath, const String& rSource, const String& rDest, bool bUnix = false );

    void InitProgress( sal_uInt16 nProgrCount );
    void ResetProgress();

    String WriteMetaCharset() const;

    void InitExportParameters( const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& rParams);
    void ExportHtml();
    void ExportKiosk();
    void ExportWebCast();

    bool WriteHtml( const String& rFileName, bool bAddExtension, const String& rHtmlData );
    String GetButtonName( int nButton ) const;

 public:
     HtmlExport( rtl::OUString aPath, const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& rParams, SdDrawDocument* pExpDoc, ::sd::DrawDocShell* pDocShell );
    virtual ~HtmlExport();

    static String   ColorToHTMLString( Color aColor );
    static String   StringToHTMLString( const String& rString );
    static String   StringToURL( const String& rURL );
};

#endif // _SD_HTMLEX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
