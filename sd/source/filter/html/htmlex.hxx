/*************************************************************************
 *
 *  $RCSfile: htmlex.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2003-06-04 11:02:33 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SD_HTMLEX_HXX
#define _SD_HTMLEX_HXX

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _XPOLY_HXX
#include <svx/xpoly.hxx>
#endif
#ifndef _SV_GDIMTF_HXX //autogen
#include <vcl/gdimtf.hxx>
#endif
#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif
#ifndef _SD_RESLTN_HXX
#include "resltn.hxx"       // enum PublishingResolution
#endif
#ifndef _SV_COLRDLG_HXX
#include <svtools/colrdlg.hxx>
#endif
#ifndef _EHDL_HXX //autogen
#include <svtools/ehdl.hxx>
#endif


#ifndef MAC
#include "../../ui/inc/strings.hrc"
#include "../../ui/inc/docshell.hxx"
#include "../../ui/inc/sdwindow.hxx"
#include "../../ui/inc/viewshel.hxx"
#include "../../ui/inc/assclass.hxx"
#else  //MAC
#include "strings.hrc"
#include "docshell.hxx"
#include "sdwindow.hxx"
#include "viewshel.hxx"
#include "assclass.hxx"
#endif //!MAC

#include "sdresid.hxx"
#include "pubdlg.hxx"

#define NUM_BUTTONS 12

#define PUB_LOWRES_WIDTH    640
#define PUB_LOWRES_HEIGHT   480
#define PUB_MEDRES_WIDTH    800
#define PUB_MEDRES_HEIGHT   600
#define PUB_HIGHRES_WIDTH   1024
#define PUB_HIGHRES_HEIGHT  768

#define HtmlButtonThemaStr = "private://gallery/hidden/HtmlExportButtons";

class List;
class SfxProgress;
class SdrOutliner;
class SdPage;
class HtmlState;
class SdrTextObj;
class SdrPage;
class SdDrawDocument;
class SdView;

class HtmlErrorContext : public ErrorContext
{
private:
    USHORT  m_nResId;
    String  m_aURL1;
    String  m_aURL2;

public:
                    HtmlErrorContext(Window *pWin=0);
                    ~HtmlErrorContext() {};

    virtual BOOL    GetString( ULONG nErrId, String& rCtxStr );

    void            SetContext( USHORT nResId );
    void            SetContext( USHORT nResId, const String& rURL );
    void            SetContext( USHORT nResId, const String& rURL1, const String& rURL2 );
};

// =====================================================================
// this class exports an Impress Document as a HTML Presentation
// =====================================================================
class HtmlExport
{
    String m_aPath;

    SdDrawDocument* pDoc;
    SdDrawDocShell* pDocSh;

    HtmlErrorContext m_eEC;

    HtmlPublishMode m_eMode;
    SfxProgress* mpProgress;
    bool m_bImpress;
    USHORT m_nSdPageCount;
    USHORT m_nPagesWritten;
    bool m_bContentsPage;
    INT16 m_nButtonThema;
    UINT16 m_nWidthPixel;
    UINT16 m_nHeightPixel;
    PublishingFormat m_eFormat;
    bool m_bHeader;
    bool m_bNotes;
    bool m_bFrames;
    bool m_bKiosk;
//-/    bool m_bCreated;
    String m_aIndex;
    String m_aEMail;
    String m_aAuthor;
    String m_aHomePage;
    String m_aInfo;
    INT16 m_nCompression;
    String m_aDocFileName;
    String m_aFramePage;
    String m_DocTitle;
    bool m_bDownload;

    bool m_bAutoSlide;
    UINT32  m_nSlideDuration;
    bool m_bSlideSound;
    bool m_bEndless;

    bool m_bUserAttr;           // die folgenden Farben werden fuer das <body>
    Color m_aTextColor;         // tag genutzt, wenn m_bUserAttr true ist
    Color m_aBackColor;
    Color m_aLinkColor;
    Color m_aVLinkColor;
    Color m_aALinkColor;
    Color m_aFirstPageColor;
    bool m_bDocColors;

    String   m_aHTMLExtension;
    String** m_pHTMLFiles;
    String** m_pImageFiles;
    String** m_pPageNames;
    String** m_pTextFiles;

    String m_aExportPath;           // Das Ausgabeverzeichnes bzw. die URL
    String m_aIndexUrl;
    String m_aURLPath;
    String m_aCGIPath;
    PublishingScript m_eScript;

    SdrTextObj* GetLayoutTextObject(SdrPage* pPage);

    void SetDocColors( SdPage* pPage = NULL );

    bool    CreateImagesForPresPages();
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

    ULONG   CreateBitmap( ULONG nThemeId, INT16 nImage, const String& aName ) const;
    void    SmoothBitmap( BitmapEx& aBmp, Color aBackCol ) const;
    String getDocumentTitle();
    bool    SavePresentation();

    String  CreateLink( const String& aLink, const String& aText,
                        const String& aTarget = String()) const;
    String  CreateImage( const String& aImage, const String& aAltText, INT16 nWidth = -1, INT16 nHeight = -1 ) const;
    String  CreateNavBar( USHORT nSdPage, bool bIsText ) const;
    String  CreateBodyTag() const;

    String  ParagraphToHTMLString( SdrOutliner* pOutliner, ULONG nPara, const Color& rBackgroundColor );
    String  TextAttribToHTMLString( SfxItemSet* pSet, HtmlState* pState, const Color& rBackgroundColor );

    String  CreateTextForTitle( SdrOutliner* pOutliner, SdPage* pPage, const Color& rBackgroundColor );
    String  CreateTextForPage( SdrOutliner* pOutliner, SdPage* pPage, bool bHeadLine, const Color& rBackgroundColor );
    String  CreateTextForNotesPage( SdrOutliner* pOutliner, SdPage* pPage, bool bHeadLine, const Color& rBackgroundColor );

    String  CreateHTMLCircleArea( ULONG nRadius, ULONG nCenterX,
                                  ULONG nCenterY, const String& rHRef ) const;
    String  CreateHTMLPolygonArea( const XPolyPolygon& rXPolyPoly,
                                   Size aShift, double fFactor,
                                   const String& rHRef ) const;
    String  CreateHTMLRectArea( const Rectangle& rRect,
                                const String& rHRef ) const;

    String  CreatePageURL( USHORT nPgNum );

    String InsertSound( const String& rSoundFile );
    bool CopyFile( const String& rSourceFile, const String& rDestPath );
    bool CopyScript( const String& rPath, const String& rSource, const String& rDest, bool bUnix = false );

    void InitProgress( USHORT nProgrCount );
    void ResetProgress();

    String WriteMetaCharset() const;

    void InitExportParameters( const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& rParams);
    void ExportHtml();
    void ExportKiosk();
    void ExportWebCast();

    List aSpecialObjects;
    void HideSpecialObjects( SdPage* pPage );
    void ShowSpecialObjects();

    bool WriteHtml( const String& rFileName, bool bAddExtension, const String& rHtmlData );
    String GetButtonName( USHORT nButton ) const;

 public:
     HtmlExport( rtl::OUString aPath, const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& rParams, SdDrawDocument* pExpDoc, SdDrawDocShell* pDocShell );
    virtual ~HtmlExport();

    static String   ColorToHTMLString( Color aColor );
    static String   StringToHTMLString( const String& rString );
    static String   StringToURL( const String& rURL );
};

#endif // _SD_HTMLEX_HXX
