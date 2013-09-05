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
#include "htmlpublishmode.hxx"

#include <vector>
#include <boost/scoped_ptr.hpp>

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
    OUString  maURL1;
    OUString  maURL2;

public:
                    HtmlErrorContext(Window *pWin=0);
                    ~HtmlErrorContext() {};

    virtual bool    GetString( sal_uLong nErrId, OUString& rCtxStr );

    void            SetContext( sal_uInt16 nResId, const OUString& rURL );
    void            SetContext( sal_uInt16 nResId, const OUString& rURL1, const OUString& rURL2 );
};

/// this class exports an Impress Document as a HTML Presentation.
class HtmlExport
{
    std::vector< SdPage* > maPages;
    std::vector< SdPage* > maNotesPages;

    OUString maPath;

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
    OUString maIndex;
    OUString maEMail;
    OUString maAuthor;
    OUString maHomePage;
    OUString maInfo;
    sal_Int16 mnCompression;
    OUString maDocFileName;
    OUString maFramePage;
    OUString mDocTitle;
    bool mbDownload;

    bool mbAutoSlide;
    double  mfSlideDuration;
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

    OUString   maHTMLExtension;
    OUString** mpHTMLFiles;
    OUString** mpImageFiles;
    OUString** mpThumbnailFiles;
    OUString** mpPageNames;
    OUString** mpTextFiles;

    OUString maExportPath; ///< output directory or URL.
    OUString maIndexUrl;
    OUString maURLPath;
    OUString maCGIPath;
    PublishingScript meScript;

    const OUString maHTMLHeader;

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
    bool    checkFileExists( ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess3 >& xFileAccess, OUString const & aFileName );

    OUString getDocumentTitle();
    bool    SavePresentation();

    OUString CreateLink( const OUString& aLink, const OUString& aText,
                        const OUString& aTarget = OUString()) const;
    OUString CreateImage( const OUString& aImage, const OUString& aAltText, sal_Int16 nWidth = -1, sal_Int16 nHeight = -1 ) const;
    OUString CreateNavBar( sal_uInt16 nSdPage, bool bIsText ) const;
    OUString CreateBodyTag() const;

    OUString ParagraphToHTMLString( SdrOutliner* pOutliner, sal_Int32 nPara, const Color& rBackgroundColor );
    OUString TextAttribToHTMLString( SfxItemSet* pSet, HtmlState* pState, const Color& rBackgroundColor );

    OUString CreateTextForTitle( SdrOutliner* pOutliner, SdPage* pPage, const Color& rBackgroundColor );
    OUString CreateTextForPage( SdrOutliner* pOutliner, SdPage* pPage, bool bHeadLine, const Color& rBackgroundColor );
    OUString CreateTextForNotesPage( SdrOutliner* pOutliner, SdPage* pPage, bool bHeadLine, const Color& rBackgroundColor );

    OUString CreateHTMLCircleArea( sal_uLong nRadius, sal_uLong nCenterX,
                                  sal_uLong nCenterY, const OUString& rHRef ) const;
    OUString CreateHTMLPolygonArea( const ::basegfx::B2DPolyPolygon& rPolyPoly, Size aShift, double fFactor, const OUString& rHRef ) const;
    OUString CreateHTMLRectArea( const Rectangle& rRect,
                                const OUString& rHRef ) const;

    OUString CreatePageURL( sal_uInt16 nPgNum );

    OUString InsertSound( const OUString& rSoundFile );
    bool CopyFile( const OUString& rSourceFile, const OUString& rDestFile );
    bool CopyScript( const OUString& rPath, const OUString& rSource, const OUString& rDest, bool bUnix = false );

    void InitProgress( sal_uInt16 nProgrCount );
    void ResetProgress();

    OUString CreateMetaCharset() const;

    void InitExportParameters( const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& rParams);
    void ExportHtml();
    void ExportKiosk();
    void ExportWebCast();

    bool WriteHtml( const OUString& rFileName, bool bAddExtension, const OUString& rHtmlData );
    OUString GetButtonName( int nButton ) const;

 public:
     HtmlExport( OUString aPath, const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& rParams, SdDrawDocument* pExpDoc, ::sd::DrawDocShell* pDocShell );
    virtual ~HtmlExport();

    static OUString ColorToHTMLString( Color aColor );
    static OUString StringToHTMLString( const OUString& rString );
    static OUString StringToURL( const OUString& rURL );
};

#endif // _SD_HTMLEX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
