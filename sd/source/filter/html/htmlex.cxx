/*************************************************************************
 *
 *  $RCSfile: htmlex.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 10:57:30 $
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


#ifndef _COM_SUN_STAR_DOCUMENT_XEXPORTER_HPP_
#include <com/sun/star/document/XExporter.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XFILTER_HPP_
#include <com/sun/star/document/XFilter.hpp>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _FSYS_HXX
#include <tools/fsys.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _FILTER_CONFIG_ITEM_HXX_
#include <svtools/FilterConfigItem.hxx>
#endif
#ifndef _UNOTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif
#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif
#ifndef _SFX_PROGRESS_HXX
#include <sfx2/progress.hxx>
#endif
#ifndef _SFX_PROGRESS_HXX
#include <sfx2/progress.hxx>
#endif
#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _AEITEM_HXX
#include <svtools/aeitem.hxx>
#endif
#ifndef _SVDITER_HXX
#include <svx/svditer.hxx>
#endif
#ifndef _GOODIES_IMAPRECT_HXX
#include <svtools/imaprect.hxx>
#endif
#ifndef _GOODIES_IMAPCIRC_HXX
#include <svtools/imapcirc.hxx>
#endif
#ifndef _GOODIES_IMAPPOLY_HXX
#include <svtools/imappoly.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXSTBMGR_HXX
#include <sfx2/stbmgr.hxx>
#endif
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _OUTLOBJ_HXX
#include <svx/outlobj.hxx>
#endif
#ifndef _EDITOBJ_HXX
#include <svx/editobj.hxx>
#endif
#ifndef _SVDOPATH_HXX
#include <svx/svdopath.hxx>
#endif
#ifndef _XOUTBMP_HXX
#include <svx/xoutbmp.hxx>
#endif
#ifndef _HTMLOUT_HXX
#include <svtools/htmlout.hxx>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#ifndef _SV_CVTGRF_HXX
#include <vcl/cvtgrf.hxx>
#endif
#ifndef _GALLERY_HXX_
#include <svx/gallery.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif
#ifndef _FILTER_HXX
#include <svtools/filter.hxx>
#endif
#ifndef _EEITEMID_HXX
#include <svx/eeitemid.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX
#include <svx/colritem.hxx>
#endif
#ifndef _MyEDITENG_HXX
#include <svx/editeng.hxx>
#endif
#ifndef _SVX_WGHTITEM_HXX
#include <svx/wghtitem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX
#include <svx/udlnitem.hxx>
#endif
#ifndef _SVX_POSTITEM_HXX
#include <svx/postitem.hxx>
#endif
#ifndef _SVX_CRSDITEM_HXX
#include <svx/crsditem.hxx>
#endif
#ifndef _SVX_FLDITEM_HXX
#include <svx/flditem.hxx>
#endif
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFX_FCONTNR_HXX
#include <sfx2/fcontnr.hxx>
#endif
#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif
#define _SVSTDARR_USHORTS
#include <svtools/svstdarr.hxx>
#ifndef _SVDETC_HXX
#include <svx/svdetc.hxx>
#endif
#ifndef _SVDOUTL_HXX
#include <svx/svdoutl.hxx>
#endif
#include <svx/impgrf.hxx>               // FillFilter()
#include <tools/urlobj.hxx>               // INetURLObject
#ifndef _SV_BMPACC_HXX //autogen
#include <vcl/bmpacc.hxx>
#endif
#ifndef _SFXECODE_HXX //autogen
#include <svtools/sfxecode.hxx>
#endif
#ifndef _SVX_IMPGRF_HXX
#include <svx/impgrf.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATE_HPP_
#include <com/sun/star/beans/PropertyState.hpp>
#endif

#include "drawdoc.hxx"
#include "sdoutl.hxx"
#include "sdpage.hxx"
#include "sdattr.hxx"
#include "glob.hrc"
#include "anminfo.hxx"
#include "imapinfo.hxx"
#include "sdresid.hxx"
#include "htmlex.hxx"

using namespace ::rtl;
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
char *pButtonNames[NUM_BUTTONS] =
{
    "first0.gif", "first1.gif", "prev0.gif", "prev1.gif",
    "next0.gif", "next1.gif", "last0.gif", "last1.gif",
    "index.gif", "text.gif", "more.gif", "less.gif"
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

// Fuer Detectfilter
#define CALC_OPTIONS        "9,34,SYSTEM"

// *********************************************************************
// Hilfsklasse fuer das simple erzeugen von Dateien lokal/remote
// *********************************************************************
class EasyFile
{
private:
    SvStream*   pOStm;
    SfxMedium*  pMedium;
    BOOL        bOpen;

public:

    EasyFile();
    ~EasyFile();

    ULONG createStream( const ByteString& rUrl, SvStream*& rpStr );
    ULONG createFileName(  const ByteString& rUrl, String& rFileName );
    ULONG close();
};

// *********************************************************************
// Hilfsklasse fuer das einbinden von Textattributen in die Html-Ausgabe
// *********************************************************************
class HtmlState
{
private:
    BOOL m_bColor;
    BOOL m_bWeight;
    BOOL m_bItalic;
    BOOL m_bUnderline;
    BOOL m_bStrike;
    BOOL m_bLink;
    Color m_aColor;
    Color m_aDefColor;
    ByteString m_aLink;
    ByteString m_aTarget;

public:
    HtmlState( Color aDefColor );

    ByteString SetWeight( BOOL bWeight );
    ByteString SetItalic( BOOL bItalic );
    ByteString SetUnderline( BOOL bUnderline );
    ByteString SetColor( Color aColor );
    ByteString SetStrikeout( BOOL bStrike );
    ByteString SetLink( const ByteString& aLink, const ByteString& aTarget );
    ByteString Flush();
};

// =====================================================================
// alle noch offennen Tags schliessen
// =====================================================================
ByteString HtmlState::Flush()
{
    ByteString aStr;

    aStr += SetWeight(FALSE);
    aStr += SetItalic(FALSE);
    aStr += SetUnderline(FALSE);
    aStr += SetStrikeout(FALSE);
    aStr += SetColor(m_aDefColor);
    aStr += SetLink("","");

    return aStr;
}

// =====================================================================
// c'tor mit Defaultfarbe fuer die Seite
// =====================================================================
HtmlState::HtmlState( Color aDefColor )
{
    m_bColor = FALSE;
    m_bWeight = FALSE;
    m_bItalic = FALSE;
    m_bUnderline = FALSE;
    m_bLink = FALSE;
    m_bStrike = FALSE;
    m_aDefColor = aDefColor;
}

// =====================================================================
// aktiviert/deaktiviert Fettdruck
// =====================================================================
ByteString HtmlState::SetWeight( BOOL bWeight )
{
    ByteString aStr;

    if(bWeight && !m_bWeight)
        aStr += "<b>";
    else if(!bWeight && m_bWeight)
        aStr += "</b>";

    m_bWeight = bWeight;
    return aStr;
}

// =====================================================================
// aktiviert/deaktiviert Italic
// =====================================================================
ByteString HtmlState::SetItalic( BOOL bItalic )
{
    ByteString aStr;

    if(bItalic && !m_bItalic)
        aStr += "<i>";
    else if(!bItalic && m_bItalic)
        aStr += "</i>";

    m_bItalic = bItalic;
    return aStr;
}

// =====================================================================
// aktiviert/deaktiviert Unterstrichen
// =====================================================================
ByteString HtmlState::SetUnderline( BOOL bUnderline )
{
    ByteString aStr;

    if(bUnderline && !m_bUnderline)
        aStr += "<u>";
    else if(!bUnderline && m_bUnderline)
        aStr += "</u>";

    m_bUnderline = bUnderline;
    return aStr;
}

// =====================================================================
// aktiviert/deaktiviert Durchstreichen
// =====================================================================
ByteString HtmlState::SetStrikeout( BOOL bStrike )
{
    ByteString aStr;

    if(bStrike && !m_bStrike)
        aStr += "<strike>";
    else if(!bStrike && m_bStrike)
        aStr += "</strike>";

    m_bStrike = bStrike;
    return aStr;
}

// =====================================================================
// Setzt die angegebenne Textfarbe
// =====================================================================
ByteString HtmlState::SetColor( Color aColor )
{
    ByteString aStr;

    if(m_bColor && aColor == m_aColor)
        return aStr;

    if(m_bColor)
    {
        aStr += "</font>";
        m_bColor = FALSE;
    }

    if(aColor != m_aDefColor)
    {
        m_aColor = aColor;

        aStr += "<font color=";
        aStr += HtmlExport::ColorToHTMLString(aColor);
        aStr += '>';

        m_bColor = TRUE;
    }

    return aStr;
}

// =====================================================================
// aktiviert/deaktiviert einen Hyperlink
// =====================================================================
ByteString HtmlState::SetLink( const ByteString& aLink, const ByteString& aTarget )
{
    ByteString aStr;

    if(m_bLink&&m_aLink == aLink&&m_aTarget==aTarget)
        return aStr;

    if(m_bLink)
    {
        aStr += "</a>";
        m_bLink = FALSE;
    }

    if(aLink.Len())
    {
        aStr += "<a href=\"";
        aStr += aLink;
        if(aTarget.Len())
        {
            aStr += "\" target=\"";
            aStr += aTarget;
        }
        aStr += "\">";
        m_bLink = TRUE;
        m_aLink = aLink;
        m_aTarget = aTarget;
    }

    return aStr;
}

// *********************************************************************
// class HtmlExport Methoden
// *********************************************************************


// =====================================================================
// Konstruktor fuer die Html Export Hilfsklasse
// =====================================================================
HtmlExport::HtmlExport( OUString aPath, const Sequence< PropertyValue >& rParams, SdDrawDocument* pExpDoc, SdDrawDocShell* pDocShell )
:   pDoc(pExpDoc),
    m_aPath( aPath ),
    m_aHTMLExtension(SdResId(STR_HTMLEXP_DEFAULT_EXTENSION),  gsl_getSystemTextEncoding()),
    m_pImageFiles(NULL),
    m_pHTMLFiles(NULL),
    m_pPageNames(NULL),
    m_pTextFiles(NULL),
    m_bUserAttr(FALSE),
    m_bDocColors(FALSE),
    m_bContentsPage(FALSE),
    m_nButtonThema(-1),
    m_bNotes(FALSE),
    m_eEC(NULL),
    pDocSh( pDocShell ),
    m_eMode( PUBLISH_HTML ),
    m_eFormat( FORMAT_JPG ),
    m_nCompression( -1 ),
    m_nWidthPixel( PUB_LOWRES_WIDTH ),
    m_bDownload( false ),
    m_eScript( SCRIPT_ASP )
{
    FASTBOOL bChange = pDoc->IsChanged();

    USHORT nError = 0;

    String  aIndex(RTL_CONSTASCII_USTRINGPARAM("index"));
    aIndex += UniString(m_aHTMLExtension, gsl_getSystemTextEncoding());
    m_aIndexUrl = ByteString( aIndex, RTL_TEXTENCODING_UTF8 );

    InitExportParameters( rParams );

    switch( m_eMode )
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

    pDoc->SetChanged(bChange);
}

HtmlExport::~HtmlExport()
{
    // ------------------------------------------------------------------
    // Listen loeschen
    // ------------------------------------------------------------------
    if(m_pImageFiles && m_pHTMLFiles && m_pPageNames && m_pTextFiles)
    {
        for ( USHORT nSdPage = 0; nSdPage < m_nSdPageCount; nSdPage++)
        {
            delete m_pImageFiles[nSdPage];
            delete m_pHTMLFiles[nSdPage];
            delete m_pPageNames[nSdPage];
            delete m_pTextFiles[nSdPage];
        }
    }

    delete[] m_pImageFiles;
    delete[] m_pHTMLFiles;
    delete[] m_pPageNames;
    delete[] m_pTextFiles;
}

/** get common export parameters from item set */
void HtmlExport::InitExportParameters( const Sequence< PropertyValue >& rParams )
{
    m_bImpress = pDoc && pDoc->GetDocumentType() == DOCUMENT_TYPE_IMPRESS;

    sal_Int32 nArgs = rParams.getLength();
    const PropertyValue* pParams = rParams.getConstArray();
    while( nArgs-- )
    {
        if( pParams->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "PublishMode" ) ) )
        {
            sal_Int32 temp;
            pParams->Value >>= temp;
            m_eMode = (HtmlPublishMode)temp;
        }
        else if( pParams->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "IndexURL" ) ) )
        {
            OUString aStr;
            pParams->Value >>= aStr;
            String aStr2( aStr );
            m_aIndexUrl = ByteString( aStr2, RTL_TEXTENCODING_UTF8 );
        }
        else if( pParams->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Format" ) ) )
        {
            sal_Int32 temp;
            pParams->Value >>= temp;
            m_eFormat = (PublishingFormat)temp;
        }
        else if( pParams->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Compression" ) ) )
        {
            OUString temp;
            pParams->Value >>= temp;
            String temp2( temp );
            ByteString aStr( temp2, RTL_TEXTENCODING_UTF8 );
            if(aStr.Len())
            {
                USHORT nPos = aStr.Search( '%' );
                if(nPos != -1)
                    aStr.Erase(nPos,1);
                m_nCompression = (INT16)aStr.ToInt32();
            }
        }
        else if( pParams->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Width" ) ) )
        {
            sal_Int32 temp;
            pParams->Value >>= temp;
            m_nWidthPixel = (sal_uInt16)temp;
        }
        else if( pParams->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "UseButtonSet" ) ) )
        {
            sal_Int32 temp;
            pParams->Value >>= temp;
            m_nButtonThema = (sal_Int16)temp;
        }
        else if( pParams->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "IsExportNotes" ) ) )
        {
            if( m_bImpress )
            {
                sal_Bool temp;
                pParams->Value >>= temp;
                m_bNotes = temp;
            }
        }
        else if( pParams->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "IsExportContentsPage" ) ) )
        {
            sal_Bool temp;
            pParams->Value >>= temp;
            m_bContentsPage = temp;
        }
        else if( pParams->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Author" ) ) )
        {
            OUString aStr;
            pParams->Value >>= aStr;
            String aStr2( aStr );
            m_aAuthor = ByteString( aStr2, RTL_TEXTENCODING_UTF8 );
        }
        else if( pParams->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "EMail" ) ) )
        {
            OUString aStr;
            pParams->Value >>= aStr;
            String aStr2( aStr );
            m_aEMail = ByteString( aStr2, RTL_TEXTENCODING_UTF8 );
        }
        else if( pParams->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "HomepageURL" ) ) )
        {
            OUString aStr;
            pParams->Value >>= aStr;
            String aStr2( aStr );
            m_aHomePage = ByteString( aStr2, RTL_TEXTENCODING_UTF8 );
        }
        else if( pParams->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "UserText" ) ) )
        {
            OUString aStr;
            pParams->Value >>= aStr;
            String aStr2( aStr );
            m_aInfo = ByteString( aStr2, RTL_TEXTENCODING_UTF8 );
        }
        else if( pParams->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "EnableDownload" ) ) )
        {
            sal_Bool temp;
            pParams->Value >>= temp;
            m_bDownload = temp;
        }
        else if( pParams->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "BackColor" ) ) )
        {
            sal_Int32 temp;
            pParams->Value >>= temp;
            m_aBackColor = temp;
            m_bUserAttr = true;
        }
        else if( pParams->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "TextColor" ) ) )
        {
            sal_Int32 temp;
            pParams->Value >>= temp;
            m_aTextColor = temp;
            m_bUserAttr = true;
        }
        else if( pParams->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "LinkColor" ) ) )
        {
            sal_Int32 temp;
            pParams->Value >>= temp;
            m_aLinkColor = temp;
            m_bUserAttr = true;
        }
        else if( pParams->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "VLinkColor" ) ) )
        {
            sal_Int32 temp;
            pParams->Value >>= temp;
            m_aVLinkColor = temp;
            m_bUserAttr = true;
        }
        else if( pParams->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ALinkColor" ) ) )
        {
            sal_Int32 temp;
            pParams->Value >>= temp;
            m_aALinkColor = temp;
            m_bUserAttr = true;
        }
        else if( pParams->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "IsUseDocumentColors" ) ) )
        {
            sal_Bool temp;
            pParams->Value >>= temp;
            m_bDocColors = temp;
        }
        else if( pParams->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "KioskSlideDuration" ) ) )
        {
            sal_Int32 temp;
            pParams->Value >>= temp;
            m_nSlideDuration = temp;
            m_bAutoSlide = true;
        }
        else if( pParams->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "KioskEndless" ) ) )
        {
            sal_Bool temp;
            pParams->Value >>= temp;
            m_bEndless = temp;
        }
        else if( pParams->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "WebCastCGIURL" ) ) )
        {
            OUString aStr;
            pParams->Value >>= aStr;
            String aStr2( aStr );
            m_aCGIPath = ByteString( aStr2, RTL_TEXTENCODING_UTF8 );
        }
        else if( pParams->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "WebCastTargetURL" ) ) )
        {
            OUString aStr;
            pParams->Value >>= aStr;
            String aStr2( aStr );
            m_aURLPath = ByteString( aStr2, RTL_TEXTENCODING_UTF8 );
        }
        else if( pParams->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "WebCastScriptLanguage" ) ) )
        {
            OUString aStr;
            pParams->Value >>= aStr;
            if( aStr.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM( "asp" ) ) )
            {
                m_eScript = SCRIPT_ASP;
            }
            else
            {
                m_eScript = SCRIPT_PERL;
            }
        }
        else
        {
            DBG_ERROR("Unknown property for html export detected!");
        }

        pParams++;
    }

    if( m_eMode == PUBLISH_KIOSK )
    {
        m_bContentsPage = false;
        m_bNotes = false;

    }

    // calculate image sizes
    SdPage* pPage = pDoc->GetSdPage(0, PK_STANDARD);
    Size aTmpSize( pPage->GetSize() );
    double dRatio=((double)aTmpSize.Width())/aTmpSize.Height();

    switch( m_nWidthPixel )
    {
        case 800:
            m_nWidthPixel = 640;
            break;
        case 1024:
            m_nWidthPixel = 800;
            break;
        case 640:
        default:
            m_nWidthPixel = 512;
            break;
    }
    m_nHeightPixel = (USHORT)(m_nWidthPixel/dRatio);

    //------------------------------------------------------------------
    // Ziel ausklamuestern...

    INetURLObject aINetURLObj( m_aPath );
    DBG_ASSERT( aINetURLObj.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

    m_aExportPath = ByteString( aINetURLObj.GetPartBeforeLastName(), RTL_TEXTENCODING_UTF8 );   // mit '/' am Ende
    m_aIndex = ByteString( aINetURLObj.GetLastName(), RTL_TEXTENCODING_UTF8 );

    m_nSdPageCount = pDoc->GetSdPageCount( PK_STANDARD );
    m_bFrames = m_eMode == PUBLISH_FRAMES;

    m_aDocFileName = m_aIndex;
}

///////////////////////////////////////////////////////////////////////
// Exportiert das im Konstruktor angegebenne Impress Dokument nach HTML
///////////////////////////////////////////////////////////////////////
void HtmlExport::ExportHtml()
{

    if(m_bUserAttr)
    {
        if( m_aTextColor == COL_AUTO )
        {
            if( !m_aBackColor.IsDark() )
                m_aTextColor = COL_BLACK;
        }
    }
    else if( m_bDocColors )
    {
        // Standard Farben fuer das Farbschema 'Aus Dokument'
        SetDocColors();
        m_aFirstPageColor = m_aBackColor;
    }

    // get name for downloadable presentation if needed
    if( m_bDownload )
    {
        // Separator such und Extension ausblenden
        USHORT nSepPos = m_aDocFileName.Search('.');

        if(nSepPos != STRING_NOTFOUND)
            m_aDocFileName.Erase(nSepPos);

        m_aDocFileName += ".sxi";
    }

    //////

    USHORT nProgrCount = m_nSdPageCount;
    nProgrCount += m_bImpress?m_nSdPageCount:0;
    nProgrCount += m_bContentsPage?1:0;
    nProgrCount += (m_bFrames && m_bNotes)?m_nSdPageCount:0;
    nProgrCount += (m_bFrames)?8:0;
    InitProgress( nProgrCount );

    pDocSh->SetWaitCursor( TRUE );

    //------------------------------------------------------------------
    // Exceptions sind doch was schoennes...

    CreateFileNames();

    // this is not a true while
    while( 1 )
    {
        if( !CreateImagesForPresPages() )
            break;

        if( !CreateHtmlForPresPages() )
            break;

        if( m_bImpress )
            if( !CreateHtmlTextForPresPages() )
                break;

        if( m_bFrames )
        {
            if( !CreateFrames() )
                break;

            if( !CreateOutlinePages() )
                break;

            if( !CreateNavBarFrames() )
                break;

            if( m_bNotes && m_bImpress )
                if( !CreateNotesPages() )
                    break;

        }

        if( m_bContentsPage )
            if( !CreateContentPage() )
                break;

        if( !CreateBitmaps() )
            break;

        pDocSh->SetWaitCursor( FALSE );
        ResetProgress();

        if( m_bDownload )
            SavePresentation();

        return;
    }

    // if we get to this point the export was
    // canceled by the user after an error
    pDocSh->SetWaitCursor( FALSE );
    ResetProgress();
}

///////////////////////////////////////////////////////////////////////

void HtmlExport::SetDocColors( SdPage* pPage )
{
    if( pPage == NULL )
        pPage = pDoc->GetSdPage(0, PK_STANDARD);

    svtools::ColorConfig aConfig;
    m_aVLinkColor = Color(aConfig.GetColorValue(svtools::LINKSVISITED).nColor);
    m_aALinkColor = Color(aConfig.GetColorValue(svtools::LINKS).nColor);
    m_aLinkColor  = Color(aConfig.GetColorValue(svtools::LINKS).nColor);
    m_aTextColor  = Color(COL_BLACK);

    SfxStyleSheet* pSheet = NULL;

    if( pDoc->GetDocumentType() == DOCUMENT_TYPE_IMPRESS )
    {
        // Standard Textfarbe aus Outline-Vorlage der ersten Seite
        pSheet = pPage->GetStyleSheetForPresObj(PRESOBJ_OUTLINE);
        if(pSheet == NULL)
            pSheet = pPage->GetStyleSheetForPresObj(PRESOBJ_TEXT);
        if(pSheet == NULL)
            pSheet = pPage->GetStyleSheetForPresObj(PRESOBJ_TITLE);
    }

    if(pSheet == NULL)
        pSheet = pDoc->GetDefaultStyleSheet();

    if(pSheet)
    {
        SfxItemSet& rSet = pSheet->GetItemSet();
        if(rSet.GetItemState(ITEMID_COLOR,TRUE) == SFX_ITEM_ON)
            m_aTextColor = ((SvxColorItem*)rSet.GetItem(ITEMID_COLOR,TRUE))->GetValue();
    }

    // Standard Hintergrundfarbe aus Background der MasterPage der ersten Seite
    m_aBackColor = pPage->GetBackgroundColor();

    if( m_aTextColor == COL_AUTO )
    {
        if( !m_aBackColor.IsDark() )
            m_aTextColor = COL_BLACK;
    }
}

///////////////////////////////////////////////////////////////////////

void HtmlExport::InitProgress( USHORT nProgrCount )
{
    String aStr(SdResId(STR_CREATE_PAGES));
    mpProgress = new SfxProgress( pDocSh, aStr, nProgrCount );
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
    m_nPagesWritten = 0;
    InitProgress( 2*m_nSdPageCount );

    CreateFileNames();

    if( CreateImagesForPresPages() )
        CreateHtmlForPresPages();

    ResetProgress();
}

///////////////////////////////////////////////////////////////////////
// Export Document with WebCast (TM) Technology
///////////////////////////////////////////////////////////////////////
void HtmlExport::ExportWebCast()
{
    m_nPagesWritten = 0;
    InitProgress( m_nSdPageCount + 9 );

    pDocSh->SetWaitCursor( TRUE );

    CreateFileNames();

    String aEmpty;
    if(m_aCGIPath.Len() == 0)
        m_aCGIPath = ".";

    if( m_aCGIPath.GetChar( m_aCGIPath.Len() - 1 ) != '/' )
        m_aCGIPath += '/';

    if( m_eScript == SCRIPT_ASP )
    {
        m_aURLPath = "./";
    }
    else
    {
           String aEmpty2;
        if(m_aURLPath.Len() == 0)
            m_aURLPath = ".";

        if( m_aURLPath.GetChar( m_aURLPath.Len() - 1 ) != '/' )
            m_aURLPath += '/';
    }

    // this is not a true while
    while(1)
    {
        if(!CreateImagesForPresPages())
            break;

        if( m_eScript == SCRIPT_ASP )
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

    pDocSh->SetWaitCursor( FALSE );
    ResetProgress();
}

///////////////////////////////////////////////////////////////////////
// Save the presentation as a downloadable file in the dest directory
///////////////////////////////////////////////////////////////////////

BOOL HtmlExport::SavePresentation()
{
    m_eEC.SetContext( STR_HTMLEXP_ERROR_CREATE_FILE, m_aDocFileName );

    ByteString aFull( m_aExportPath );
    aFull += m_aDocFileName;


    pDocSh->EnableSetModified( true );

    String aURL( aFull, RTL_TEXTENCODING_UTF8 );
    try
    {
        uno::Reference< frame::XStorable > xStorable( pDoc->getUnoModel(), uno::UNO_QUERY );
        if( xStorable.is() )
        {
            uno::Sequence< beans::PropertyValue > aProperties( 2 );
            aProperties[ 0 ].Name = OUString(RTL_CONSTASCII_USTRINGPARAM("Overwrite"));
            aProperties[ 0 ].Value <<= (sal_Bool)sal_True;
            aProperties[ 1 ].Name = OUString(RTL_CONSTASCII_USTRINGPARAM("FilterName"));
            aProperties[ 1 ].Value <<= OUString(RTL_CONSTASCII_USTRINGPARAM("StarOffice XML (Impress)"));
            xStorable->storeToURL( OUString( aURL ), aProperties );

            pDocSh->EnableSetModified( false );

            return true;
        }
    }
    catch( Exception& )
    {
    }

    pDocSh->EnableSetModified( false );

    return false;
}

// =====================================================================
// Image-Dateien anlegen
// =====================================================================
BOOL HtmlExport::CreateImagesForPresPages()
{
    try
    {
        Reference < XMultiServiceFactory > xMSF( ::comphelper::getProcessServiceFactory() );
        if( !xMSF.is() )
            return false;

        Reference< XExporter > xGraphicExporter( xMSF->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.GraphicExportFilter") ) ), UNO_QUERY );
        Reference< XFilter > xFilter( xGraphicExporter, UNO_QUERY );

        DBG_ASSERT( xFilter.is(), "no com.sun.star.drawing.GraphicExportFilter?" );
        if( !xFilter.is() )
            return false;

        Sequence< PropertyValue > aFilterData(((m_eFormat==FORMAT_JPG)&&(m_nCompression != -1))? 3 : 2);
        aFilterData[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM("PixelWidth") );
        aFilterData[0].Value <<= (sal_Int32)m_nWidthPixel;
        aFilterData[1].Name = OUString( RTL_CONSTASCII_USTRINGPARAM("PixelHeight") );
        aFilterData[1].Value <<= (sal_Int32)m_nHeightPixel;
        if((m_eFormat==FORMAT_JPG)&&(m_nCompression != -1))
        {
            aFilterData[2].Name = OUString( RTL_CONSTASCII_USTRINGPARAM("Quality") );
            aFilterData[2].Value <<= (sal_Int32)m_nCompression;
        }

        Sequence< PropertyValue > aDescriptor( 3 );
        aDescriptor[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM("URL") );
        aDescriptor[1].Name = OUString( RTL_CONSTASCII_USTRINGPARAM("FilterName") );
        aDescriptor[1].Value <<= OUString( RTL_CONSTASCII_USTRINGPARAM(m_eFormat==FORMAT_GIF ? "GIF" : "JPG") );
        aDescriptor[2].Name = OUString( RTL_CONSTASCII_USTRINGPARAM("FilterData") );
        aDescriptor[2].Value <<= aFilterData;

        for (USHORT nSdPage = 0; nSdPage < m_nSdPageCount; nSdPage++)
        {
            SdPage* pPage = pDoc->GetSdPage( nSdPage, PK_STANDARD );

            ByteString aFull(m_aExportPath);
            aFull += *m_pImageFiles[nSdPage];

            aDescriptor[0].Value <<= OUString::createFromAscii( aFull.GetBuffer() );

            Reference< XComponent > xPage( pPage->getUnoPage(), UNO_QUERY );
            xGraphicExporter->setSourceDocument( xPage );
            xFilter->filter( aDescriptor );

            if (mpProgress)
                mpProgress->SetState(++m_nPagesWritten);
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
    ULONG           nObjectCount = pPage->GetObjCount();
    SdrObject*      pObject      = NULL;
    SdrTextObj*     pResult      = NULL;

    for (ULONG nObject = 0; nObject < nObjectCount; nObject++)
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

ByteString HtmlExport::WriteMetaCharset() const
{
    ByteString aStr;
    const sal_Char *pCharSet = rtl_getBestMimeCharsetFromTextEncoding( RTL_TEXTENCODING_UTF8 );
    if ( pCharSet )
    {
        ByteString aCharSet( pCharSet );
        aStr += "<meta HTTP-EQUIV=CONTENT-TYPE CONTENT=\"text/html; charset=";
        aStr += aCharSet;
        aStr += "\">\r\n";
    }
    return aStr;
}

BOOL HtmlExport::CreateHtmlTextForPresPages()
{
    ULONG nErr = 0;

    SdrOutliner* pOutliner = pDoc->GetInternalOutliner();

    for(USHORT nSdPage = 0; nSdPage < m_nSdPageCount && nErr == 0; nSdPage++)
    {
        SdPage* pPage = pDoc->GetSdPage(nSdPage, PK_STANDARD);

        if( m_bDocColors )
        {
            SetDocColors( pPage );
//          m_aBackColor = pPage->GetBackgroundColor();
        }

// HTML Kopf
        ByteString aStr("<html>\r\n<head>\r\n" );
        aStr += WriteMetaCharset();
        aStr += "<title>";
        aStr += *m_pPageNames[nSdPage];
        aStr += "</title>\r\n";
        aStr += "</head>\r\n";
        aStr += CreateBodyTag();

// Navigationsleiste
        aStr += CreateNavBar(nSdPage, TRUE);

// Seitentitel
        aStr += "<h1>";
        aStr += CreateTextForTitle(pOutliner,pPage, pPage->GetBackgroundColor());
        aStr += "</h1><p>\r\n";

// Gliederungstext schreiben
        aStr += CreateTextForPage( pOutliner, pPage, TRUE, pPage->GetBackgroundColor() );

// Notizen
        if(m_bNotes)
        {
            aStr += "<br>\r\n<h3>";
            aStr += RESTOHTML(STR_HTMLEXP_NOTES);
            aStr += ":</h3>\r\n";

            SdPage* pNotesPage = pDoc->GetSdPage(nSdPage, PK_NOTES);

/*
            if( m_bDocColors )
            {
                SdPage* pMaster = (SdPage*)pNotesPage->GetMasterPage(0);
                aStr += "<table bgcolor=";
                aStr += ColorToHTMLString(GetPageFillColor( pMaster ));
                aStr += "><tr><td>\r\n";
            }
*/
            aStr += CreateTextForNotesPage( pOutliner, pNotesPage, TRUE, m_aBackColor);

/*
            if( m_bDocColors )
                aStr += "\r\n</td></tr></table>\r\n";
*/
        }

// Seite beenden
        aStr += "</body>\r\n</html>";

        m_eEC.SetContext( STR_HTMLEXP_ERROR_CREATE_FILE, *m_pTextFiles[nSdPage] );
        EasyFile aFile;
        SvStream* pStr;
        ByteString aFull( m_aExportPath );
        aFull += *m_pTextFiles[nSdPage];
        nErr = aFile.createStream(aFull , pStr);
        if(nErr == 0)
        {
            *pStr << aStr.GetBuffer();
            nErr = aFile.close();
        }

        if (mpProgress)
            mpProgress->SetState(++m_nPagesWritten);
    }

    pOutliner->Clear();

    if( nErr != 0 )
        ErrorHandler::HandleError(nErr);

    return nErr == 0;
}

// =====================================================================

/** Erzeugt den Outliner Text fuer das Titelobjekt einer Seite
 */
ByteString HtmlExport::CreateTextForTitle( SdrOutliner* pOutliner, SdPage* pPage, const Color& rBackgroundColor )
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

    return ByteString();
}

// =====================================================================
// Erzeugt den Outliner Text fuer eine Seite
// =====================================================================
ByteString HtmlExport::CreateTextForPage( SdrOutliner* pOutliner,
                                      SdPage* pPage,
                                      BOOL bHeadLine, const Color& rBackgroundColor )
{
    ByteString aStr;

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

            ULONG nCount = pOutliner->GetParagraphCount();

            Paragraph* pPara = NULL;
            USHORT nActDepth = 1;

            ByteString aParaText;
            aStr += "<ul>";
            for (ULONG nPara = 0; nPara < nCount; nPara++)
                {
                    pPara = pOutliner->GetParagraph(nPara);
                    if(pPara)
                    {
                        const USHORT nDepth = (USHORT) pOutliner->GetDepth( (USHORT) nPara );
                        if(nDepth < nActDepth )
                        {
                            do
                            {
                              aStr += "</ul>";
                              nActDepth--;
                            }
                            while(nDepth < nActDepth);
                        }
                        else if(nDepth > nActDepth )
                        {
                            aStr += "<ul>";
                            nActDepth = nDepth;
                        }

                        aParaText = ParagraphToHTMLString(pOutliner,nPara,rBackgroundColor);

                        if(aParaText.Len() != 0)
                        {
                            aStr += "<li>";
                            if(nActDepth == 1 && bHeadLine)
                                aStr += "<h2>";
                            aStr += aParaText;
                            if(nActDepth == 1 && bHeadLine)
                                aStr += "</h2>";
                            aStr += "\r\n";
                        }
                    }
                }

                if( nActDepth > 0 ) do
                {
                    aStr += "</ul>";
                    nActDepth--;
                }
                while( nActDepth );
            }
        }

    return aStr;
}

// =====================================================================
// Erzeugt den Outliner Text fuer eine Notizseite
// =====================================================================
ByteString HtmlExport::CreateTextForNotesPage( SdrOutliner* pOutliner,
                                           SdPage* pPage,
                                           BOOL bHeadLine,
                                           const Color& rBackgroundColor )
{
    ByteString aStr;

    SdrTextObj* pTO = (SdrTextObj*)pPage->GetPresObj(PRESOBJ_NOTES);

    if (pTO && !pTO->IsEmptyPresObj())
    {
        OutlinerParaObject* pOPO = pTO->GetOutlinerParaObject();
        if (pOPO)
        {
            pOutliner->Clear();
            pOutliner->SetText( *pOPO );

            ULONG nCount = pOutliner->GetParagraphCount();
            for (ULONG nPara = 0; nPara < nCount; nPara++)
            {
                aStr += ParagraphToHTMLString( pOutliner, nPara,rBackgroundColor );
                aStr += "<br>\r\n";
            }
        }
    }

    return aStr;
}

// =====================================================================
// Wandelt einen Paragraphen des Outliners in Html
// =====================================================================
ByteString HtmlExport::ParagraphToHTMLString( SdrOutliner* pOutliner, ULONG nPara, const Color& rBackgroundColor )
{
    ByteString aStr;

    if(NULL == pOutliner)
        return aStr;

    // TODO: MALTE!!!
    EditEngine& rEditEngine = *(EditEngine*)&pOutliner->GetEditEngine();
    BOOL bOldUpdateMode = rEditEngine.GetUpdateMode();
    rEditEngine.SetUpdateMode(TRUE);

    Paragraph* pPara = pOutliner->GetParagraph(nPara);
    if(NULL == pPara)
        return aStr;

    HtmlState aState( (m_bUserAttr || m_bDocColors)  ? m_aTextColor : Color(COL_BLACK) );
    SvUShorts aPortionList;
    rEditEngine.GetPortions( (USHORT) nPara, aPortionList );
    USHORT nPortionCount = aPortionList.Count();

    USHORT nPos1 = 0;
    for( USHORT nPortion = 0; nPortion < nPortionCount; nPortion++ )
    {
        USHORT nPos2 = aPortionList.GetObject(nPortion);

        ESelection aSelection( (USHORT) nPara, nPos1, (USHORT) nPara, nPos2);

        SfxItemSet aSet( rEditEngine.GetAttribs( aSelection ) );

        ByteString aPortion(StringToHTMLString(rEditEngine.GetText( aSelection )));

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
ByteString HtmlExport::TextAttribToHTMLString( SfxItemSet* pSet, HtmlState* pState, const Color& rBackgroundColor )
{
    ByteString aStr;

    if(NULL == pSet)
        return aStr;

    ByteString aLink, aTarget;
    if ( pSet->GetItemState( EE_FEATURE_FIELD ) == SFX_ITEM_ON )
    {
        SvxFieldItem* pItem = (SvxFieldItem*)pSet->GetItem( EE_FEATURE_FIELD );
        if(pItem)
        {
            SvxURLField* pURL = PTR_CAST(SvxURLField, pItem->GetField());
            if(pURL)
            {
                aLink = ByteString( pURL->GetURL(), RTL_TEXTENCODING_UTF8 ) ;
                aTarget = ByteString( pURL->GetTargetFrame(), RTL_TEXTENCODING_UTF8 ) ;
            }
        }
    }
    aStr += pState->SetLink(aLink, aTarget);

    if ( pSet->GetItemState( EE_CHAR_WEIGHT ) == SFX_ITEM_ON )
        aStr += pState->SetWeight( ((const SvxWeightItem&)
            pSet->Get( EE_CHAR_WEIGHT )).GetWeight() == WEIGHT_BOLD );

    if ( pSet->GetItemState( EE_CHAR_UNDERLINE ) == SFX_ITEM_ON )
        aStr += pState->SetUnderline( ((const SvxUnderlineItem&)
            pSet->Get( EE_CHAR_UNDERLINE )).GetUnderline() != UNDERLINE_NONE );

    if ( pSet->GetItemState( EE_CHAR_STRIKEOUT ) == SFX_ITEM_ON )
        aStr += pState->SetStrikeout( ((const SvxCrossedOutItem&)
            pSet->Get( EE_CHAR_STRIKEOUT )).GetStrikeout() != STRIKEOUT_NONE);

    if ( pSet->GetItemState( EE_CHAR_ITALIC ) == SFX_ITEM_ON )
        aStr += pState->SetItalic( ((const SvxPostureItem&)
            pSet->Get( EE_CHAR_ITALIC )).GetPosture() != ITALIC_NONE);

    if(m_bDocColors)
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

    return aStr;
}

// =====================================================================
// HTML-Wrapper fuer Bild-Dateien erzeugen
// =====================================================================
BOOL HtmlExport::CreateHtmlForPresPages()
{
    ULONG nErr = 0;

    List aClickableObjects;

    for(USHORT nSdPage = 0; nSdPage < m_nSdPageCount && nErr == 0; nSdPage++)
    {
        // Klickbare Objekte finden (auch auf der Masterpage) und
        // in Liste stellen. In umgekehrter Zeichenreihenfolge in
        // die Liste stellen, da in HTML bei Ueberlappungen die
        // _erstgenannte_ Area wirkt.

        SdPage* pPage = pDoc->GetSdPage(nSdPage, PK_STANDARD);

        if( m_bDocColors )
        {
            SetDocColors( pPage );
//          m_aBackColor = pPage->GetBackgroundColor();
        }

        BOOL    bMasterDone = FALSE;

        while (!bMasterDone)
        {
            // TRUE = rueckwaerts
            SdrObjListIter aIter(*pPage, IM_DEEPWITHGROUPS, TRUE);

            SdrObject* pObject = aIter.Next();
            while (pObject)
            {
                SdAnimationInfo* pInfo     = pDoc->GetAnimationInfo(pObject);
                SdIMapInfo*      pIMapInfo = pDoc->GetIMapInfo(pObject);

                if ((pInfo &&
                     (pInfo->eClickAction == presentation::ClickAction_BOOKMARK  ||
                      pInfo->eClickAction == presentation::ClickAction_DOCUMENT  ||
                      pInfo->eClickAction == presentation::ClickAction_PREVPAGE  ||
                      pInfo->eClickAction == presentation::ClickAction_NEXTPAGE  ||
                      pInfo->eClickAction == presentation::ClickAction_FIRSTPAGE ||
                      pInfo->eClickAction == presentation::ClickAction_LASTPAGE)) ||
                     pIMapInfo)
                {
                    aClickableObjects.Insert(pObject, LIST_APPEND);
                }

                pObject = aIter.Next();
            }
            // jetzt zur Masterpage oder beenden
            if (!pPage->IsMasterPage())
                pPage = (SdPage*)pPage->GetMasterPage(0);
            else
                bMasterDone = TRUE;
        }
        ULONG nClickableObjectCount = aClickableObjects.Count();

// HTML Head
        ByteString aStr("<html>\r\n<head>\r\n" );
        aStr += WriteMetaCharset();
        aStr += "<title>";
        aStr += *m_pPageNames[nSdPage];
        aStr += "</title>\r\n";

// insert timing information
        pPage = pDoc->GetSdPage(nSdPage, PK_STANDARD);
        if( m_eMode == PUBLISH_KIOSK )
        {
            ULONG nSecs = 0;
            BOOL bEndless = FALSE;
            if( !m_bAutoSlide )
            {
                if( pPage->GetPresChange() != PRESCHANGE_MANUAL )
                {
                    nSecs = pPage->GetTime();
                    bEndless = pDoc->GetPresEndless();
                }
            }
            else
            {
                nSecs = m_nSlideDuration;
                bEndless = m_bEndless;
            }

            if( nSecs != 0 )
            {
                if( nSdPage < (m_nSdPageCount-1) || bEndless )
                {
                    aStr += "<meta http-equiv=\"refresh\" content=\"";
                    aStr += ByteString::CreateFromInt32(nSecs);
                    aStr += "; URL=";

                    int nPage = nSdPage + 1;
                    if( nPage == m_nSdPageCount )
                        nPage = 0;

                    aStr += *m_pHTMLFiles[nPage];

                    aStr += "\">\r\n";
                }
            }
        }

        aStr += "</head>\r\n";

// HTML Body
        aStr += CreateBodyTag();

        if( m_bSlideSound && pPage->IsSoundOn() )
            aStr += InsertSound( ByteString( pPage->GetSoundFile(), RTL_TEXTENCODING_UTF8 ) );

// Navigationsleiste
        if(!m_bFrames )
            aStr += CreateNavBar( nSdPage, FALSE );
// Image
        aStr += "<center>";
        aStr += "<img src=\"";
        aStr += *m_pImageFiles[nSdPage];
        aStr += '"';

        if (nClickableObjectCount > 0)
            aStr += " USEMAP=\"#map0\"";

        aStr += "></center>\r\n";

// Notizen
        if(m_bNotes && !m_bFrames)
        {
            SdrOutliner* pOutliner = pDoc->GetInternalOutliner();
            aStr += "<p><h3>";
            aStr += RESTOHTML(STR_HTMLEXP_NOTES);
            aStr += ":</h3><br>\r\n";

            SdPage* pNotesPage = pDoc->GetSdPage(nSdPage, PK_NOTES);
/*
            if( m_bDocColors )
            {
                SdPage* pMaster = (SdPage*)pNotesPage->GetMasterPage(0);
                aStr += "<table bgcolor=";
                aStr += ColorToHTMLString(GetPageFillColor( pMaster ));
                aStr += "><tr><td>\r\n";
            }
*/
            aStr += CreateTextForNotesPage( pOutliner, pNotesPage, TRUE, m_aBackColor);

/*
            if( m_bDocColors )
                aStr += "\r\n</td></tr></table>\r\n";
*/

            pOutliner->Clear();
        }

// ggfs. Imagemap erzeugen
        if (nClickableObjectCount > 0)
        {
            aStr += "<map name=\"map0\">\r\n";

            for (ULONG nObject = 0; nObject < nClickableObjectCount; nObject++)
            {
                SdrObject* pObject = (SdrObject*)aClickableObjects.GetObject(nObject);
                SdAnimationInfo* pInfo     = pDoc->GetAnimationInfo(pObject);
                SdIMapInfo*      pIMapInfo = pDoc->GetIMapInfo(pObject);

                Rectangle aRect(pObject->GetBoundRect());
                Point     aLogPos(aRect.TopLeft());
                BOOL      bIsSquare = aRect.GetWidth() == aRect.GetHeight();

                ULONG nPageWidth = pPage->GetSize().Width() - pPage->GetLftBorder() -
                                   pPage->GetRgtBorder();

                // das BoundRect bezieht sich auf den physikalischen
                // Seitenursprung, nicht auf den Koordinatenursprung
                aRect.Move(-pPage->GetLftBorder(), -pPage->GetUppBorder());

                double fLogicToPixel = ((double)m_nWidthPixel) / nPageWidth;
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
                    UINT16 nAreaCount = rIMap.GetIMapObjectCount();
                    for (UINT16 nArea = 0; nArea < nAreaCount; nArea++)
                    {
                        IMapObject* pArea = rIMap.GetIMapObject(nArea);
                        UINT16 nType = pArea->GetType();
                        ByteString aURL( pArea->GetURL(), RTL_TEXTENCODING_UTF8 );

                        // ggfs. Seiten- oder Objektnamen umwandeln in den
                        // Namen der entsprechenden HTML-Datei
                        BOOL        bIsMasterPage;
                        USHORT      nPgNum = pDoc->GetPageByName( String( aURL, RTL_TEXTENCODING_UTF8 ), bIsMasterPage );
                        SdrObject*  pObj = NULL;

                        if (nPgNum == SDRPAGE_NOTFOUND)
                        {
                            // Ist das Bookmark ein Objekt?
                            pObj = pDoc->GetObj( String( aURL, RTL_TEXTENCODING_UTF8 ));
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
                                                 GetRectangle(FALSE));

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
                                                 GetCenter(FALSE));
                                aCenter += Point(aLogPos.X() - pPage->GetLftBorder(),
                                                 aLogPos.Y() - pPage->GetUppBorder());
                                aCenter.X() = (long)(aCenter.X() * fLogicToPixel);
                                aCenter.Y() = (long)(aCenter.Y() * fLogicToPixel);

                                ULONG nRadius = (((IMapCircleObject*)pArea)->
                                                 GetRadius(FALSE));
                                nRadius = (ULONG)(nRadius * fLogicToPixel);
                                aStr += CreateHTMLCircleArea(nRadius,
                                                            aCenter.X(), aCenter.Y(),
                                                            aURL);
                            }
                            break;

                            case IMAP_OBJ_POLYGON:
                            {
                                Polygon aArea(((IMapPolygonObject*)pArea)->
                                               GetPolygon(FALSE));
                                XPolygon aXPoly(aArea);
                                XPolyPolygon aXPolyPoly(aXPoly);
                                aStr += CreateHTMLPolygonArea(aXPolyPoly,
                                                Size(aLogPos.X() - pPage->GetLftBorder(),
                                                     aLogPos.Y() - pPage->GetUppBorder()),
                                                fLogicToPixel, aURL);
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
                    ByteString      aHRef;
                    presentation::ClickAction eClickAction = pInfo->eClickAction;

                    switch( eClickAction )
                    {
                        case presentation::ClickAction_BOOKMARK:
                        {
                            BOOL        bIsMasterPage;
                            USHORT      nPgNum = pDoc->GetPageByName( pInfo->aBookmark, bIsMasterPage );
                            SdrObject*  pObj = NULL;

                            if( nPgNum == SDRPAGE_NOTFOUND )
                            {
                                // Ist das Bookmark ein Objekt?
                                pObj = pDoc->GetObj(pInfo->aBookmark);
                                if (pObj)
                                    nPgNum = pObj->GetPage()->GetPageNum();
                            }

                            if( SDRPAGE_NOTFOUND != nPgNum )
                                aHRef = CreatePageURL(( nPgNum - 1 ) / 2 );
                        }
                        break;

                        case presentation::ClickAction_DOCUMENT:
                            aHRef = ByteString( pInfo->aBookmark, RTL_TEXTENCODING_UTF8 );
                        break;

                        case presentation::ClickAction_PREVPAGE:
                        {
                            ULONG nPage = nSdPage;
                            if (nSdPage == 0)
                                nPage = 0;
                            else
                                nPage = nSdPage - 1;

                            aHRef = CreatePageURL( (USHORT) nPage);
                        }
                        break;

                        case presentation::ClickAction_NEXTPAGE:
                        {
                            ULONG nPage = nSdPage;
                            if (nSdPage == m_nSdPageCount - 1)
                                nPage = m_nSdPageCount - 1;
                            else
                                nPage = nSdPage + 1;

                            aHRef = CreatePageURL( (USHORT) nPage);
                        }
                        break;

                        case presentation::ClickAction_FIRSTPAGE:
                            aHRef = CreatePageURL(0);
                        break;

                        case presentation::ClickAction_LASTPAGE:
                            aHRef = CreatePageURL(m_nSdPageCount - 1);
                        break;
                    }

                    // jetzt die Areas
                    UINT32 nObjIdent = pObject->GetObjIdentifier();
                    UINT32 nObjInventor = pObject->GetObjInventor();

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
                            const XPolyPolygon& rXPolyPoly =
                                ((SdrPathObj*)pObject)->GetPathPoly();
                            aStr += CreateHTMLPolygonArea(rXPolyPoly,
                                                     Size(-pPage->GetLftBorder(),
                                                          -pPage->GetUppBorder()),
                                                     fLogicToPixel,
                                                     aHRef);
                        }
                        // was anderes: das BoundRect nehmen
                        else
                        {
                            aStr += CreateHTMLRectArea(aRect, aHRef);
                        }

                    }
                }
            }

            aStr += "</map>\r\n";
        }
        aClickableObjects.Clear();

        aStr += "</body>\r\n</html>";

        m_eEC.SetContext( STR_HTMLEXP_ERROR_CREATE_FILE, *m_pHTMLFiles[nSdPage] );
        EasyFile aFile;
        SvStream* pStr;
        ByteString aFull( m_aExportPath );
        aFull += *m_pHTMLFiles[nSdPage];
        nErr = aFile.createStream( aFull, pStr);
        if(nErr == 0)
        {
            *pStr << aStr.GetBuffer();
            nErr = aFile.close();
        }

        if (mpProgress)
            mpProgress->SetState(++m_nPagesWritten);
    }

    if( nErr != 0 )
        ErrorHandler::HandleError( nErr );

    return nErr == 0;
}

// =====================================================================
// Uebersichtsseite erzeugen
// =====================================================================
BOOL HtmlExport::CreateContentPage()
{

    // Parameter
    String aEmpty;

    if( m_bDocColors )
    {
        SetDocColors();
    }

    // Html Kopf
    ByteString aStr( "<html>\r\n<head>\r\n" );
    aStr += WriteMetaCharset();
    aStr += "<title>";
    aStr += *m_pPageNames[0];
    aStr += "</title>\r\n</head>\r\n";
    aStr += CreateBodyTag();

    // Seitenkopf
    aStr += "<center>\r\n";

    if(m_bHeader)
    {
        aStr += "<h1>";
        aStr += getDocumentTitle();
        aStr += "</h1><br>\r\n";
    }

    aStr += "<h2>";

    // #92564# Solaris compiler bug workaround
    if( m_bFrames )
        aStr += CreateLink( m_aFramePage,
                            RESTOHTML(STR_HTMLEXP_CLICKSTART) );
    else
        aStr += CreateLink( StringToHTMLString(*m_pHTMLFiles[0]),
                            RESTOHTML(STR_HTMLEXP_CLICKSTART) );

    aStr += "</h2>\r\n</center>\r\n";

    aStr += "<center><table width=90%><TR>\r\n";

    // Inhaltsverzeichnis
    aStr += "<td valign=top align=left width=50%>\r\n";
    aStr += "<h3><u>";
    aStr += RESTOHTML(STR_HTMLEXP_CONTENTS);
    aStr += "</u></h3>";

    for(USHORT nSdPage = 0; nSdPage < m_nSdPageCount; nSdPage++)
    {
        SdPage* pPage = pDoc->GetSdPage(nSdPage, PK_STANDARD);

        ByteString aPageName = *m_pPageNames[nSdPage];
        aStr += "<p align=left>";
        if(m_bFrames)
            aStr += aPageName;
        else
            aStr += CreateLink(*m_pHTMLFiles[nSdPage], aPageName);
        aStr += "</p>\r\n";
    }
    aStr += "</td>\r\n";

    // Dokument Infos
    aStr += "<td valign=top width=50%>\r\n";

    if(m_aAuthor.Len())
    {
        aStr += "<p><strong>";
        aStr += RESTOHTML(STR_HTMLEXP_AUTHOR);
        aStr += ":</strong> ";
        aStr += m_aAuthor;
        aStr += "</p>\r\n";
    }

    if(m_aEMail.Len())
    {
        aStr += "<p><strong>";
        aStr += RESTOHTML(STR_HTMLEXP_EMAIL);
        aStr += ":</strong> <a href=\"mailto:";
        aStr += m_aEMail;
        aStr += "\">";
        aStr += m_aEMail;
        aStr += "</a></p>\r\n";
    }

    if(m_aHomePage.Len())
    {
        aStr += "<p><strong>";
        aStr += RESTOHTML(STR_HTMLEXP_HOMEPAGE);
        aStr += ":</strong> <a href=\"";
        aStr += m_aHomePage;
        aStr += "\">";
        aStr += m_aHomePage;
        aStr += "</a> </p>\r\n";
    }

    if(m_aInfo.Len())
    {
        aStr += "<p><strong>";
        aStr += RESTOHTML(STR_HTMLEXP_INFO);
        aStr += ":</strong><br>\r\n";
        aStr += m_aInfo;
        aStr += "</p>\r\n";
    }

    if(m_bDownload)
    {
        aStr += "<p><a href=\"";
        aStr += m_aDocFileName;
        aStr += "\">";
        aStr += RESTOHTML(STR_HTMLEXP_DOWNLOAD);
        aStr += "</a></p>\r\n";
    }

    aStr += "</td></tr></table></center>\r\n";

    aStr += "</body>\r\n</html>";

    ULONG nErr = 0;
    m_eEC.SetContext( STR_HTMLEXP_ERROR_CREATE_FILE, m_aIndex );
    EasyFile aFile;
    ByteString aFull(m_aExportPath);
    aFull += m_aIndex;
    SvStream* pStr;
    nErr = aFile.createStream(aFull, pStr);
    if(nErr == 0)
    {
        *pStr << aStr.GetBuffer();
        nErr = aFile.close();
    }

    if (mpProgress)
        mpProgress->SetState(++m_nPagesWritten);

    if( nErr != 0 )
        ErrorHandler::HandleError( nErr );

    return nErr == 0;
}

// =====================================================================
// Notiz Seiten erzeugen (fuer Frames)
// =====================================================================
BOOL HtmlExport::CreateNotesPages()
{
    ULONG nErr = 0;

    SdrOutliner* pOutliner = pDoc->GetInternalOutliner();
    for( USHORT nSdPage = 0; nErr == 0 && nSdPage < m_nSdPageCount; nSdPage++ )
    {
        SdPage* pPage = pDoc->GetSdPage(nSdPage, PK_NOTES);
        if( m_bDocColors )
        {
            SetDocColors( pPage );
//          m_aBackColor = pPage->GetBackgroundColor();
        }

        // Html Kopf
        ByteString aStr("<html>\r\n<head>\r\n" );
        aStr += WriteMetaCharset();
        aStr += "<title>";
        aStr += *m_pPageNames[0];
        aStr += "</title>\r\n</head>\r\n";
        aStr += CreateBodyTag();

        if(pPage)
            aStr += CreateTextForNotesPage( pOutliner, pPage, TRUE, m_aBackColor );

        aStr += "</body>\r\n</html>";

        ByteString aFull(m_aExportPath);
        ByteString aFileName( "note" );
        aFileName += ByteString::CreateFromInt32(nSdPage);
        aFileName += m_aHTMLExtension;

        aFull += aFileName;

        m_eEC.SetContext( STR_HTMLEXP_ERROR_CREATE_FILE, aFileName );
        EasyFile aFile;
        SvStream* pStr;
        nErr = aFile.createStream(aFull, pStr);
        if(nErr == 0)
        {
            *pStr << aStr.GetBuffer();
            nErr = aFile.close();
        }

        if (mpProgress)
            mpProgress->SetState(++m_nPagesWritten);
    }

    pOutliner->Clear();

    if( nErr != 0 )
        ErrorHandler::HandleError( nErr );

    return nErr == 0;
}

// =====================================================================
// Outline Seiten erzeugen (fuer Frames)
// =====================================================================
BOOL HtmlExport::CreateOutlinePages()
{
    ULONG nErr = 0;

    if( m_bDocColors )
    {
        SetDocColors();
//      m_aBackColor = m_aFirstPageColor;
    }

    // Seite 0 wird der zugeklappte Outline, Seite 1 der aufgeklappte
    for( int nPage = 0; nPage < (m_bImpress?2:1) && nErr == 0; nPage++ )
    {
        // Html Kopf
        ByteString aStr("<html>\r\n<head>\r\n" );
        aStr += WriteMetaCharset();
        aStr += "<title>";
        aStr += *m_pPageNames[0];
        aStr += "</title>\r\n</head>\r\n";
        aStr += CreateBodyTag();

        SdrOutliner* pOutliner = pDoc->GetInternalOutliner();
        for(USHORT nSdPage = 0; nSdPage < m_nSdPageCount; nSdPage++)
        {
            SdPage* pPage = pDoc->GetSdPage(nSdPage, PK_STANDARD);

            aStr += "<p align=left>";
            ByteString aLink("JavaScript:parent.NavigateAbs(");
            aLink += ByteString::CreateFromInt32(nSdPage);
            aLink += ")";

            ByteString aTitle = CreateTextForTitle(pOutliner,pPage, m_aBackColor);
            if(aTitle.Len() == 0)
                aTitle = *m_pPageNames[nSdPage];
            aStr += CreateLink(aLink, aTitle);

            if(nPage==1)
            {
                aStr +="<br>";
                aStr += CreateTextForPage( pOutliner, pPage, FALSE, m_aBackColor );
            }
            aStr += "</p>\r\n";
        }
        pOutliner->Clear();

        aStr += "</body>\r\n</html>";

        ByteString aFull(m_aExportPath);
        ByteString aFileName( "outline" );
        aFileName += ByteString::CreateFromInt32(nPage);
        aFileName += m_aHTMLExtension;

        aFull += aFileName;

        m_eEC.SetContext( STR_HTMLEXP_ERROR_CREATE_FILE, aFileName );
        EasyFile aFile;
        SvStream* pStr;
        nErr = aFile.createStream(aFull, pStr);
        if(nErr == 0)
        {
            *pStr << aStr.GetBuffer();
            nErr = aFile.close();
        }

        if (mpProgress)
            mpProgress->SetState(++m_nPagesWritten);
    }

    if( nErr != 0 )
        ErrorHandler::HandleError( nErr );

    return nErr == 0;
}

// =====================================================================
// Dateinamen festlegen
// =====================================================================
void HtmlExport::CreateFileNames()
{
    // Listen mit neuen Dateinamen anlegen
    m_pHTMLFiles = new ByteString*[m_nSdPageCount];
    m_pImageFiles = new ByteString*[m_nSdPageCount];
    m_pPageNames = new ByteString*[m_nSdPageCount];
    m_pTextFiles = new ByteString*[m_nSdPageCount];

    m_bHeader = FALSE;  // Ueberschrift auf Uebersichtsseite?

    for (USHORT nSdPage = 0; nSdPage < m_nSdPageCount; nSdPage++)
    {
        ByteString* pName;
        if(nSdPage == 0 && !m_bContentsPage && !m_bFrames )
            pName = new ByteString(m_aIndex);
        else
        {
            pName = new ByteString("img");
            *pName += ByteString::CreateFromInt32(nSdPage);
            *pName += m_aHTMLExtension;
        }

        m_pHTMLFiles[nSdPage] = pName;

        pName = new ByteString("img");
        *pName += ByteString::CreateFromInt32(nSdPage);
        *pName += m_eFormat==FORMAT_GIF?".gif":".jpg";
        m_pImageFiles[nSdPage] = pName;

        pName = new ByteString("text");
        *pName += ByteString::CreateFromInt32(nSdPage);
        *pName += m_aHTMLExtension;
        m_pTextFiles[nSdPage] = pName;

        SdPage* pSdPage = pDoc->GetSdPage(nSdPage, PK_STANDARD);

        // get slide title from page name
        ByteString* pPageTitle = new ByteString();
        *pPageTitle = StringToHTMLString(pSdPage->GetName());
        m_pPageNames[nSdPage] = pPageTitle;
    }

    if(!m_bContentsPage && m_bFrames)
        m_aFramePage = m_aIndex;
    else
    {
        m_aFramePage = "siframes";
        m_aFramePage += m_aHTMLExtension;
    }
}

ByteString HtmlExport::getDocumentTitle()
{
    // check for a title object in this page, if its the first
    // title it becomes this documents title for the content
    // page
    if( !m_bHeader )
    {
        if(m_bImpress)
        {
            // falls es ein nicht-leeres Titelobjekt gibt, dessen ersten Absatz
            // als Seitentitel benutzen
            SdPage* pSdPage = pDoc->GetSdPage(0, PK_STANDARD);
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
                        ByteString aTest(rEditTextObject.GetText(0), RTL_TEXTENCODING_UTF8 );
                        if (aTest.Len() > 0)
                            m_DocTitle = aTest;
                    }
                }
            }

            for( UINT16 i = 0; i < m_DocTitle.Len(); i++ )
                if( m_DocTitle.GetChar(i) == (char)0xff)
                    m_DocTitle.SetChar(i, ' ' );

            m_DocTitle = StringToHTMLString(m_DocTitle);

        }

        if( !m_DocTitle.Len() )
        {
            m_DocTitle = StringToHTMLString(m_aDocFileName);
            int nDot = m_DocTitle.Search( '.' );
            if( nDot > 0 )
                m_DocTitle.Erase( nDot );
        }
        m_bHeader = TRUE;
    }

    return m_DocTitle;
}

/*
var nCurrentPage = 0;
var nPageCount = JSCRIPT2;

function NavigateAbs( nPage )
{
    frames[\"show\"].location.href = \"img\" + nPage + \".htm\";
    frames[\"notes\"].location.href = \"note\" + nPage + \".htm\";
    nCurrentPage = nPage;
    if(nCurrentPage==0)
    {
        frames[\"navbar1\"].location.href = \"navbar0.htm\";
    }
    else if(nCurrentPage==nPageCount-1)
    {
        frames[\"navbar1\"].location.href = \"navbar2.htm\";
    }
    else
        frames[\"navbar1\"].location.href = \"navbar1.htm\";
    }
}

function NavigateRel( nDelta )
{
    var nPage = parseInt(nCurrentPage) + parseInt(nDelta);
    if( (nPage >= 0) && (nPage < nPageCount) )
    {
        NavigateAbs( nPage );
    }
}

function ExpandOutline()
{
    frames[\"navbar2\"].location.href = \"navbar4.htm\";
    frames[\"outline\"].location.href = \"outline1.htm\";
}

function CollapseOutline()
{
    frames[\"navbar2\"].location.href = \"navbar3.htm\";
    frames[\"outline\"].location.href = \"outline0.htm\";
}
*/

char* JS_NavigateAbs =
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

char* JS_NavigateRel =
    "function NavigateRel( nDelta )\r\n"
    "{\r\n"
    "  var nPage = parseInt(nCurrentPage) + parseInt(nDelta);\r\n"
    "  if( (nPage >= 0) && (nPage < nPageCount) )\r\n"
    "  {\r\n"
    "    NavigateAbs( nPage );\r\n"
    "  }\r\n"
    "}\r\n\r\n";

char* JS_ExpandOutline =
    "function ExpandOutline()\r\n"
    "{\r\n"
    "  frames[\"navbar2\"].location.href = \"navbar4.$EXT\";\r\n"
    "  frames[\"outline\"].location.href = \"outline1.$EXT\";\r\n"
    "}\r\n\r\n";

char * JS_CollapseOutline =
    "function CollapseOutline()\r\n"
    "{\r\n"
    "  frames[\"navbar2\"].location.href = \"navbar3.$EXT\";\r\n"
    "  frames[\"outline\"].location.href = \"outline0.$EXT\";\r\n"
    "}\r\n\r\n";

// ====================================================================
// Seite mit den Frames erzeugen
// ====================================================================
BOOL HtmlExport::CreateFrames()
{
    ByteString aTmp;
    ByteString aStr("<html>\r\n<head>" );
    aStr += WriteMetaCharset();
    aStr += "<title>";
    aStr += *m_pPageNames[0];
    aStr += "</title>\r\n";

    aStr += "<script language=\"JavaScript\">\r\n<!--\r\n";

    aStr += "var nCurrentPage = 0;\r\nvar nPageCount = ";
    aStr += ByteString::CreateFromInt32(pDoc->GetSdPageCount( PK_STANDARD ));
    aStr += ";\r\n\r\n";

    ByteString aFunction(JS_NavigateAbs);

    if(m_bNotes)
        aFunction.SearchAndReplaceAll( "//", "");

    // substitute HTML file extension
    aFunction.SearchAndReplaceAll(".$EXT", m_aHTMLExtension);
    aStr += aFunction;

    aTmp = JS_NavigateRel;
    aTmp.SearchAndReplaceAll(".$EXT", m_aHTMLExtension);
    aStr += aTmp;

    if(m_bImpress)
    {
        aTmp = JS_ExpandOutline;
        aTmp.SearchAndReplaceAll(".$EXT", m_aHTMLExtension);
        aStr += aTmp;

        aTmp = JS_CollapseOutline;
        aTmp.SearchAndReplaceAll(".$EXT", m_aHTMLExtension);
        aStr += aTmp;
    }
    aStr += "// -->\r\n</SCRIPT>\r\n";

    aStr += "</head>\r\n";

    aStr += "<frameset cols=\"*,";
    aStr += ByteString::CreateFromInt32((m_nWidthPixel + 16));
    aStr += "\">\r\n";
    if(m_bImpress)
    {
        aStr += "  <frameset rows=\"42,*\">\r\n";
        aStr += "    <frame src=\"navbar3";
        aStr += m_aHTMLExtension;
        aStr += "\" name=\"navbar2\" marginwidth=4 marginheight=4 scrolling=no>\r\n";
    }
    aStr += "    <frame src=\"outline0";
    aStr += m_aHTMLExtension;
    aStr += "\" name=\"outline\">\r\n";
    if(m_bImpress)
        aStr += "  </frameset>\r\n";

    if(m_bNotes)
    {
        aStr += "  <frameset rows=\"42,";
        aStr += ByteString::CreateFromInt32((int)((double)m_nWidthPixel * 0.75) + 16);
        aStr += ",*\">\r\n";
    }
    else
        aStr += "  <frameset rows=\"42,*\">\r\n";

    aStr += "    <frame src=\"navbar0";
    aStr += m_aHTMLExtension;
    aStr += "\" name=\"navbar1\" marginwidth=4 marginheight=4 scrolling=no>\r\n";

    aStr += "    <frame src=\"";
    aStr += StringToHTMLString(*m_pHTMLFiles[0]);
    aStr += "\" name=\"show\" marginwidth=4 marginheight=4>\r\n";

    if(m_bNotes)
    {
        aStr += "    <frame src=\"note0";
        aStr += m_aHTMLExtension;
        aStr += "\" name=\"notes\">\r\n";
    }
    aStr += "  </frameset>\r\n";

    aStr += "</frameset>\r\n";

    aStr += "<noframes>";
    aStr += CreateBodyTag();
    aStr += RESTOHTML(STR_HTMLEXP_NOFRAMES);
    aStr += "\r\n</noframes></body>\r\n</html>";

    ByteString aFull(m_aExportPath);
    aFull += m_aFramePage;

    m_eEC.SetContext( STR_HTMLEXP_ERROR_CREATE_FILE, m_aFramePage );
    EasyFile aFile;
    SvStream* pStr;
    ULONG nErr = aFile.createStream(aFull, pStr);
    if(nErr == 0)
    {
        *pStr << aStr.GetBuffer();
        nErr = aFile.close();
    }

    if (mpProgress)
        mpProgress->SetState(++m_nPagesWritten);

    if( nErr != 0 )
        ErrorHandler::HandleError( nErr );

    return nErr == 0;
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
BOOL HtmlExport::CreateNavBarFrames()
{
    ULONG nErr = 0;
    ByteString aButton;

    if( m_bDocColors )
    {
        SetDocColors();
        m_aBackColor = m_aFirstPageColor;
    }

    for( int nFile = 0; nFile < 3 && nErr == 0; nFile++ )
    {
        ByteString aStr("<html>\r\n<head>\r\n" );
        aStr += WriteMetaCharset();
        aStr += "<title>";
        aStr += *m_pPageNames[0];
        aStr += "</title>\r\n</head>\r\n";
        aStr += CreateBodyTag();
        aStr += "<center>\r\n";

    // erste Seite
        aButton = RESTOHTML(STR_HTMLEXP_FIRSTPAGE);
        if(m_nButtonThema != -1)
            aButton = CreateImage(pButtonNames[(nFile == 0 || m_nSdPageCount == 1?
                                  BTN_FIRST_0:BTN_FIRST_1)], aButton);

        if(nFile != 0 && m_nSdPageCount > 1)
            aButton = CreateLink("JavaScript:parent.NavigateAbs(0)", aButton);

        aStr += aButton;
        aStr += "\r\n";

    // zur vorherigen Seite
        aButton = RESTOHTML(STR_PUBLISH_BACK);
        if(m_nButtonThema != -1)
            aButton = CreateImage(pButtonNames[(nFile == 0 || m_nSdPageCount == 1?
                                    BTN_PREV_0:BTN_PREV_1)], aButton);

        if(nFile != 0 && m_nSdPageCount > 1)
            aButton = CreateLink("JavaScript:parent.NavigateRel(-1)", aButton);

        aStr += aButton;
        aStr += "\r\n";

    // zur naechsten Seite
        aButton = RESTOHTML(STR_PUBLISH_NEXT);
        if(m_nButtonThema != -1)
            aButton = CreateImage(pButtonNames[(nFile ==2 || m_nSdPageCount == 1?
                                    BTN_NEXT_0:BTN_NEXT_1)], aButton);

        if(nFile != 2 && m_nSdPageCount > 1)
            aButton = CreateLink("JavaScript:parent.NavigateRel(1)", aButton);

        aStr += aButton;
        aStr += "\r\n";

    // zur letzten Seite
        aButton = RESTOHTML(STR_HTMLEXP_LASTPAGE);
        if(m_nButtonThema != -1)
            aButton = CreateImage(pButtonNames[(nFile ==2 || m_nSdPageCount == 1?
                                  BTN_LAST_0:BTN_LAST_1)], aButton);

        if(nFile != 2 && m_nSdPageCount > 1)
        {
            ByteString aLink("JavaScript:parent.NavigateAbs(");
            aLink += ByteString::CreateFromInt32(pDoc->GetSdPageCount( PK_STANDARD )-1);
            aLink += ")";
            aButton = CreateLink( aLink, aButton);
        }

        aStr += aButton;
        aStr += "\r\n";

    // Inhalt
        if (m_bContentsPage)
        {
            aButton = RESTOHTML(STR_PUBLISH_OUTLINE);
            if(m_nButtonThema != -1)
                aButton = CreateImage(pButtonNames[BTN_INDEX], aButton);

            // zur Uebersicht
            aStr += CreateLink(m_aIndex, aButton, "_top");
            aStr += "\r\n";
        }

    // Textmodus
        if(m_bImpress)
        {
            aButton = RESTOHTML(STR_HTMLEXP_SETTEXT);
            if(m_nButtonThema != -1)
                aButton = CreateImage(pButtonNames[BTN_TEXT], aButton);

            ByteString  aText0("text0");
            aText0 += m_aHTMLExtension;
            aStr += CreateLink( aText0, aButton, "_top");
            aStr += "\r\n";
        }

    // Und fertich...
        aStr += "</center>\r\n";
        aStr += "</body>\r\n</html>";

        ByteString aFull(m_aExportPath);
        ByteString aFileName( "navbar" );
        aFileName += ByteString::CreateFromInt32(nFile);
        aFileName += m_aHTMLExtension;
        aFull += aFileName;

        EasyFile aFile;
        SvStream* pStr;
        m_eEC.SetContext( STR_HTMLEXP_ERROR_CREATE_FILE, aFileName );
        nErr = aFile.createStream(aFull, pStr);
        if(nErr == 0)
        {
            *pStr << aStr.GetBuffer();
            nErr = aFile.close();
        }

        if (mpProgress)
            mpProgress->SetState(++m_nPagesWritten);
    }

    // Jetzt kommt die Navigatonsleiste Outliner zugeklappt...
    if(nErr == 0)
    {
        ByteString aStr("<html>\r\n<head>\r\n" );
        aStr += WriteMetaCharset();
        aStr += "<title>";
        aStr += *m_pPageNames[0];
        aStr += "</title>\r\n</head>\r\n";
        aStr += CreateBodyTag();

        ByteString aButton;

        aButton = RESTOHTML(STR_HTMLEXP_OUTLINE);
        if(m_nButtonThema != -1)
            aButton = CreateImage(pButtonNames[BTN_LESS], aButton);

        aStr += CreateLink("JavaScript:parent.ExpandOutline()", aButton);
        aStr += "</body>\r\n</html>";

        ByteString aFull(m_aExportPath);
        ByteString aFileName( "navbar3" );
        aFileName += m_aHTMLExtension;
        aFull += aFileName;

        m_eEC.SetContext( STR_HTMLEXP_ERROR_CREATE_FILE, aFileName );
        EasyFile aFile;
        SvStream* pStr;
        nErr = aFile.createStream(aFull,pStr);
        if(nErr == 0)
        {
            *pStr << aStr.GetBuffer();
            nErr = aFile.close();
        }

        if (mpProgress)
            mpProgress->SetState(++m_nPagesWritten);
    }

    // ... und jetzt Outliner aufgeklappt
    if( nErr == 0 )
    {
        ByteString aStr("<html>\r\n<head>\r\n" );
        aStr += WriteMetaCharset();
        aStr += "<title>";
        aStr += *m_pPageNames[0];
        aStr += "</title>\r\n</head>\r\n";
        aStr += CreateBodyTag();

        ByteString aButton;

        aButton = RESTOHTML(STR_HTMLEXP_NOOUTLINE);
        if(m_nButtonThema != -1)
            aButton = CreateImage(pButtonNames[BTN_MORE], aButton);

        aStr += CreateLink("JavaScript:parent.CollapseOutline()", aButton);
        aStr += "</body>\r\n</html>";

        ByteString aFull(m_aExportPath);
        ByteString aFileName( "navbar4" );
        aFileName += m_aHTMLExtension;
        aFull += aFileName;

        m_eEC.SetContext( STR_HTMLEXP_ERROR_CREATE_FILE, aFileName );
        EasyFile aFile;
        SvStream* pStr;
        nErr = aFile.createStream(aFull, pStr);
        if(pStr)
        {
            *pStr << aStr.GetBuffer();
            nErr = aFile.close();
        }

        if (mpProgress)
            mpProgress->SetState(++m_nPagesWritten);

    }

    if( nErr != 0 )
        ErrorHandler::HandleError( nErr );

    return nErr == 0;
}

// ====================================================================
// Buttonleiste fuer Standard ausgeben
// ====================================================================
ByteString HtmlExport::CreateNavBar( USHORT nSdPage, BOOL bIsText ) const
{
    // Navigationsleiste vorbereiten
    ByteString aStrNavFirst, aStrNavPrev, aStrNavNext, aStrNavContent;
    ByteString aStrNavLast, aStrNavText, aStrNavEMail;

    aStrNavFirst   = RESTOHTML(STR_HTMLEXP_FIRSTPAGE);
    aStrNavPrev    = RESTOHTML(STR_PUBLISH_BACK);
    aStrNavNext    = RESTOHTML(STR_PUBLISH_NEXT);
    aStrNavLast    = RESTOHTML(STR_HTMLEXP_LASTPAGE);
    aStrNavContent = RESTOHTML(STR_PUBLISH_OUTLINE);
    aStrNavText    = bIsText?RESTOHTML(STR_HTMLEXP_SETGRAPHIC):
                                 RESTOHTML(STR_HTMLEXP_SETTEXT);

    if(!bIsText && m_nButtonThema != -1)
    {
        if(nSdPage<1 || m_nSdPageCount == 1)
        {
            aStrNavFirst = CreateImage(pButtonNames[BTN_FIRST_0], aStrNavFirst);
            aStrNavPrev  = CreateImage(pButtonNames[BTN_PREV_0], aStrNavPrev);
        }
        else
        {
            aStrNavFirst = CreateImage(pButtonNames[BTN_FIRST_1], aStrNavFirst);
            aStrNavPrev  = CreateImage(pButtonNames[BTN_PREV_1], aStrNavPrev);
        }

        if(nSdPage == m_nSdPageCount-1 || m_nSdPageCount == 1)
        {
            aStrNavNext    = CreateImage(pButtonNames[BTN_NEXT_0], aStrNavNext);
            aStrNavLast    = CreateImage(pButtonNames[BTN_LAST_0], aStrNavLast);
        }
        else
        {
            aStrNavNext    = CreateImage(pButtonNames[BTN_NEXT_1], aStrNavNext);
            aStrNavLast    = CreateImage(pButtonNames[BTN_LAST_1], aStrNavLast);
        }

        aStrNavContent = CreateImage(pButtonNames[BTN_INDEX], aStrNavContent);
        aStrNavText    = CreateImage(pButtonNames[BTN_TEXT], aStrNavText);
    }

    ByteString aStr("<center>\r\n"); //<table><tr>\r\n");

    // erste Seite
    if(nSdPage > 0)
        aStr += CreateLink(bIsText?*m_pTextFiles[0]:*m_pHTMLFiles[0],aStrNavFirst);
    else
        aStr += aStrNavFirst;
    aStr += ' ';

    // zur vorherigen Seite
    if(nSdPage > 0)
        aStr += CreateLink( bIsText?*m_pTextFiles[nSdPage-1]:
                                    *m_pHTMLFiles[nSdPage-1],   aStrNavPrev);
    else
        aStr += aStrNavPrev;
    aStr += ' ';

    // zur naechsten Seite
    if(nSdPage < m_nSdPageCount-1)
        aStr += CreateLink( bIsText?*m_pTextFiles[nSdPage+1]:
                                    *m_pHTMLFiles[nSdPage+1], aStrNavNext);
    else
        aStr += aStrNavNext;
    aStr += ' ';

    // letzte Seite
    if(nSdPage < m_nSdPageCount-1)
        aStr += CreateLink( bIsText?*m_pTextFiles[m_nSdPageCount-1]:
                                    *m_pHTMLFiles[m_nSdPageCount-1],
                                    aStrNavLast );
    else
        aStr += aStrNavLast;
    aStr += ' ';

    // Indexseite
    if (m_bContentsPage)
    {
        aStr += CreateLink(m_aIndex, aStrNavContent);
        aStr += ' ';
    }

    // Text/Grafik
    if(m_bImpress)
    {
        aStr += CreateLink( bIsText?(m_bFrames?m_aFramePage:*m_pHTMLFiles[nSdPage]):
                                    *m_pTextFiles[nSdPage], aStrNavText);

    }

    aStr += "</center><br>\r\n";

    return aStr;
}

// ====================================================================
// Schaltflaechen aus der Gallery exportieren
// ====================================================================
BOOL HtmlExport::CreateBitmaps()
{
    ULONG nErr = 0;

    m_eEC.SetContext( STR_HTMLEXP_ERROR_GALLERY );

    if(m_nButtonThema != -1)
    {
        if( GalleryExplorer::BeginLocking( GALLERY_THEME_HTMLBUTTONS ) )
        {
            Graphic aButton;
            INT16 nButtons = m_nButtonThema * NUM_BUTTONS + 1;
            for( INT16 nButton = 0; nButton < NUM_BUTTONS && nErr == 0; nButton++ )
            {
                if(!m_bFrames && (nButton == BTN_MORE || nButton == BTN_LESS))
                    continue;

                if(!m_bImpress && (nButton == BTN_TEXT || nButton == BTN_MORE || nButton == BTN_LESS ))
                    continue;

                nErr = CreateBitmap(GALLERY_THEME_HTMLBUTTONS, nButtons + nButton, pButtonNames[nButton]);
            }

            GalleryExplorer::EndLocking( GALLERY_THEME_HTMLBUTTONS );
        }
    }

    if( nErr != 0 )
        ErrorHandler::HandleError( nErr );

    return nErr == 0;
}

// ====================================================================
// Eine beliebige Schaltflaeche aus der Gallery exportieren
// ====================================================================
ULONG HtmlExport::CreateBitmap( ULONG nThemeId, INT16 nImage, const ByteString& aName ) const
{
    ByteString aFull(m_aExportPath);
    aFull += aName;

    Graphic aGraphic;
    EasyFile aFile;
    ByteString aJPG;
    SvStream* pStrm;
    ULONG nErr = aFile.createStream(aFull, pStrm);
    if(nErr == 0)
    {
        nErr = GalleryExplorer::GetGraphicObj( nThemeId, nImage, &aGraphic )?0:1;
        if( nErr == 0 )
        {
            if( m_bUserAttr || m_bDocColors )
            {
                BitmapEx aBitmap( aGraphic.GetBitmapEx() );

                if( aBitmap.GetTransparentType() != TRANSPARENT_NONE )
                    SmoothBitmap( aBitmap, m_bUserAttr?m_aBackColor:m_aFirstPageColor );
                aGraphic = Graphic( aBitmap );
            }
            nErr = GraphicConverter::Export( *pStrm, aGraphic, CVT_GIF );
        }

        if( nErr == 0 )
            nErr = aFile.close();
        else
            aFile.close();
    }

    return nErr;
}

/** Kantenglättung der übergebennen BitmapEx mit der angegebennen Hintergrundfarbe
 */
void HtmlExport::SmoothBitmap( BitmapEx& aBmp, Color aBackCol ) const
{
    BYTE                cBackR = aBackCol.GetRed();
    BYTE                cBackG = aBackCol.GetGreen();
    BYTE                cBackB = aBackCol.GetBlue();

    Bitmap              aSrcBmp( aBmp.GetBitmap() );
    Bitmap              aMask( aBmp.GetMask() );

    BitmapReadAccess*   pRAcc = aSrcBmp.AcquireReadAccess();
    BitmapReadAccess*   pMAcc = aMask.AcquireReadAccess();

    if( !pRAcc || !pMAcc )
        return;

    const long          nWidth = pRAcc->Width();
    const long          nHeight = pRAcc->Height();
    const long          nLastX = nWidth - 1;
    const long          nLastY = nWidth - 1;

    BYTE*               pAlpha = new BYTE[nWidth*nHeight];
    BYTE*               pAlphaLine;

    memset( pAlpha, 0, nWidth*nHeight );

    // Alpha Maske fuellen
    const BitmapColor   aWhite( pMAcc->GetBestMatchingColor( Color( COL_WHITE ) ));

    for( long nY = 0; nY < nHeight; nY++ )
    {
        for( long nX = 0; nX < nWidth; nX++ )
        {
            if( pMAcc->GetPixel( nY, nX ) == aWhite )
            {
                if( nY > 0 )
                {
                    pAlphaLine = &pAlpha[(nY-1) * nWidth + nX];
                    if( nX > 0 )
                        pAlphaLine[-1] +=1;
                    *pAlphaLine += 1;
                    if( nX < nLastX)
                        pAlphaLine[1] += 1;
                    pAlphaLine += nWidth;
                }
                else
                    pAlphaLine = &pAlpha[nY * nWidth + nX];

                if( nX > 0 )
                    pAlphaLine[-1] +=1;
                if( nX < nLastX)
                    pAlphaLine[1] += 1;

                pAlphaLine += nWidth;

                if( nY < nLastY )
                {
                    if( nX > 0 )
                        pAlphaLine[-1] +=1;
                    *pAlphaLine += 1;
                    if( nX < nLastX)
                        pAlphaLine[1] += 1;
                }
            }

            if( nX == 0 || nX == nLastX || nY == 0 || nY == nLastY  )
            {
                pAlpha[nY * nWidth + nX] += 3;
            }
        }
    }

    // Kanten glätten
    const Size          aSize( nWidth, nHeight );
    Bitmap              aDstBmp( aSize, 24 );
    BitmapWriteAccess*  pWAcc = aDstBmp.AcquireWriteAccess();

    pAlphaLine = pAlpha;
    if( !pRAcc->HasPalette() )
    {
        for( long nY = 0; nY < nHeight; nY++ )
        {
            for( long nX = 0; nX < nWidth; nX++ )
            {
                USHORT nAlpha = *pAlphaLine++;
                if( pMAcc->GetPixel( nY, nX ) != aWhite )
                {
                    BitmapColor aPixel = pRAcc->GetPixel( nY, nX );
                    if( nAlpha != 0)
                    {
                        nAlpha <<= 4;
                        aPixel.SetRed( (BYTE)(((USHORT)cBackR * nAlpha + (USHORT)aPixel.GetRed() * (128-nAlpha)) >> 7) );
                        aPixel.SetGreen( (BYTE)(((USHORT)cBackR * nAlpha + (USHORT)aPixel.GetRed() * (128-nAlpha)) >> 7) );
                        aPixel.SetBlue( (BYTE)(((USHORT)cBackR * nAlpha + (USHORT)aPixel.GetRed() * (128-nAlpha)) >> 7) );
                    }
                    pWAcc->SetPixel( nY, nX, aPixel );
                }
                else
                {
                    pWAcc->SetPixel( nY, nX, aBackCol );
                }

            }
        }
    }
    else
    {
        for( long nY = 0; nY < nHeight; nY++ )
        {
            for( long nX = 0; nX < nWidth; nX++ )
            {
                USHORT nAlpha = *pAlphaLine++;
                if( pMAcc->GetPixel( nY, nX ) != aWhite )
                {
                    BitmapColor aPixel = pRAcc->GetPaletteColor( pRAcc->GetPixel( nY, nX ) );
                    if( nAlpha != 0)
                    {
                        nAlpha <<= 4;
                        aPixel.SetRed( (BYTE)(((USHORT)cBackR * nAlpha + (USHORT)aPixel.GetRed() * (128-nAlpha)) >> 7) );
                        aPixel.SetGreen( (BYTE)(((USHORT)cBackR * nAlpha + (USHORT)aPixel.GetRed() * (128-nAlpha)) >> 7) );
                        aPixel.SetBlue( (BYTE)(((USHORT)cBackR * nAlpha + (USHORT)aPixel.GetRed() * (128-nAlpha)) >> 7) );
                    }
                    pWAcc->SetPixel( nY, nX, aPixel );
                }
                else
                {
                    pWAcc->SetPixel( nY, nX, aBackCol );
                }

            }
        }
    }

    delete[] pAlpha;
    aMask.ReleaseAccess( pMAcc );
    aSrcBmp.ReleaseAccess( pRAcc );
    aDstBmp.ReleaseAccess( pWAcc );


    aBmp = BitmapEx( aDstBmp, aMask );
}

// =====================================================================
// Erzeugt den <body> Tag, inkl. der eingestellten Farbattribute
// =====================================================================
ByteString HtmlExport::CreateBodyTag() const
{
    ByteString aStr("<body");

    if( m_bUserAttr || m_bDocColors )
    {
        Color aTextColor( m_aTextColor );
        if( (aTextColor == COL_AUTO) && (!m_aBackColor.IsDark()) )
            aTextColor = COL_BLACK;

        aStr += " text=";
        aStr += ColorToHTMLString( aTextColor );
        aStr += " bgcolor=";
        aStr += ColorToHTMLString( m_aBackColor );
        aStr += " link=";
        aStr += ColorToHTMLString( m_aLinkColor );
        aStr += " vlink=";
        aStr += ColorToHTMLString( m_aVLinkColor );
        aStr += " alink=";
        aStr += ColorToHTMLString( m_aALinkColor );
    }

    aStr += ">\r\n";

    return aStr;
}

// =====================================================================
// Erzeugt einen Hyperlink
// =====================================================================
ByteString HtmlExport::CreateLink( const ByteString& aLink,
                               const ByteString& aText,
                               const ByteString& aTarget ) const
{
    ByteString aStr("<a href=\"");
    aStr += aLink;
    if(aTarget.Len())
    {
        aStr += "\" target=\"";
        aStr += aTarget;
    }
    aStr += "\">";
    aStr += aText;
    aStr += "</a>";

    return aStr;
}

// =====================================================================
// Erzeugt ein Image-tag
// =====================================================================
ByteString HtmlExport::CreateImage( const ByteString& aImage, const ByteString& aAltText,
                                INT16 nWidth,
                                INT16 nHeight ) const
{
    ByteString aStr("<img src=\"");
    aStr += aImage;
    aStr += "\" border=0";

    if( aAltText.Len())
    {
        aStr += " alt=\"";
        aStr += aAltText;
        aStr += '"';
    }

    if(nWidth > -1)
    {
        aStr += " width=";
        aStr += ByteString::CreateFromInt32(nWidth);
    }

    if(nHeight > -1)
    {
        aStr += " height=";
        aStr += ByteString::CreateFromInt32(nHeight);
    }

    aStr += '>';

    return aStr;
}

// =====================================================================
// Area fuer Kreis erzeugen; es werden Pixelkoordinaten erwartet
// =====================================================================
ByteString HtmlExport::ColorToHTMLString( Color aColor )
{
    static char hex[] = "0123456789ABCDEF";
    ByteString aStr("#xxxxxx");
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
ByteString HtmlExport::CreateHTMLCircleArea( ULONG nRadius,
                                         ULONG nCenterX,
                                         ULONG nCenterY,
                                         const ByteString& rHRef ) const
{
    ByteString aStr( "<area shape=circle coords=\"" );

    aStr += ByteString::CreateFromInt32(nCenterX);
    aStr += ',';
    aStr += ByteString::CreateFromInt32(nCenterY);
    aStr += ',';
    aStr += ByteString::CreateFromInt32(nRadius);
    aStr += "\" href=\"";
    aStr += rHRef;
    aStr += "\">\n";

    return aStr;
}


// =====================================================================
// Area fuer Polygon erzeugen; es werden Pixelkoordinaten erwartet
// =====================================================================
ByteString HtmlExport::CreateHTMLPolygonArea( const XPolyPolygon& rXPolyPoly,
                                          Size aShift,
                                          double fFactor,
                                          const ByteString& rHRef ) const
{
    ByteString          aStr;
    const USHORT    nNoOfXPolygons = rXPolyPoly.Count();

    for ( USHORT nXPoly= 0; nXPoly < nNoOfXPolygons; nXPoly++ )
    {
        const XPolygon& rXPoly = rXPolyPoly.GetObject(nXPoly);
        const USHORT    nNoOfPoints = rXPoly.GetPointCount();

        aStr += "<area shape=polygon coords=\"";

        for ( USHORT nPoint = 0; nPoint < nNoOfPoints; nPoint++ )
        {
            Point aPnt(rXPoly[nPoint]);
            // das Koordinaten beziehen sich auf den
            // physikalischen Seitenursprung, nicht auf den
            // Koordinatenursprung
            aPnt.Move(aShift.Width(), aShift.Height());

            aPnt.X() = (long)(aPnt.X() * fFactor);
            aPnt.Y() = (long)(aPnt.Y() * fFactor);
            aStr += ByteString::CreateFromInt32(aPnt.X());
            aStr += ',';
            aStr += ByteString::CreateFromInt32(aPnt.Y());

            if (nPoint < nNoOfPoints - 1)
                aStr += ",";
        }
        aStr += "\" href=\"";
        aStr += rHRef;
        aStr += "\">\n";
    }

    return aStr;
}

// =====================================================================
// Area fuer Rechteck erzeugen; es werden Pixelkoordinaten erwartet
// =====================================================================
ByteString HtmlExport::CreateHTMLRectArea( const Rectangle& rRect,
                                       const ByteString& rHRef ) const
{
    ByteString aStr( "<area shape=rect coords=\"" );

    aStr += ByteString::CreateFromInt32(rRect.Left());
    aStr += ',';
    aStr += ByteString::CreateFromInt32(rRect.Top());
    aStr += ',';
    aStr += ByteString::CreateFromInt32(rRect.Right());
    aStr += ',';
    aStr += ByteString::CreateFromInt32(rRect.Bottom());
    aStr += "\" href=\"";
    aStr += rHRef;
    aStr += "\">\n";

    return aStr;
}

// =====================================================================
// StringToHTMLString, konvertiert einen String in
// seine HTML-Repraesentation (Umlaute etc.)
// =====================================================================
ByteString HtmlExport::StringToHTMLString( const ByteString& rString )
{
    return StringToHTMLString( String( rString, RTL_TEXTENCODING_UTF8 ) );
}

ByteString HtmlExport::StringToHTMLString( const String& rString )
{
    SvMemoryStream aMemStm;
    HTMLOutFuncs::Out_String( aMemStm, rString, RTL_TEXTENCODING_UTF8 );
    aMemStm << (char) 0;
    return ByteString( (char*)aMemStm.GetData() );
}

// =====================================================================
// Erzeugt die URL einer bestimmten Seite
// =====================================================================
ByteString HtmlExport::CreatePageURL( USHORT nPgNum )
{
    if(m_bFrames)
    {
        ByteString aUrl ("JavaScript:parent.NavigateAbs(");
        aUrl += ByteString::CreateFromInt32(nPgNum);
        aUrl += ')';
        return aUrl;
    }
    else
        return *m_pHTMLFiles[nPgNum];
}

BOOL HtmlExport::CopyScript( const ByteString& rPath, const ByteString& rSource, const ByteString& rDest, bool bUnix /* = false */ )
{
    INetURLObject   aURL( SvtPathOptions().GetConfigPath() );
    ByteString      aScript;

    aURL.Append( String( RTL_CONSTASCII_USTRINGPARAM("webcast") ) );
    aURL.Append( String( rSource, RTL_TEXTENCODING_UTF8 ) );

    m_eEC.SetContext( STR_HTMLEXP_ERROR_OPEN_FILE, rSource );

    ULONG       nErr;
    SvStream*   pIStm = ::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ );

    if( pIStm )
    {
        ByteString aLine;

        while( pIStm->ReadLine( aLine ) )
        {
            aScript += aLine;
            if( bUnix )
            {
                aScript += "\n";
            }
            else
            {
                aScript += "\r\n";
            }
        }

        nErr = pIStm->GetError();
        delete pIStm;
    }

    if( nErr != 0 )
    {
        ErrorHandler::HandleError( nErr );
        return (BOOL) nErr;
    }


    aScript.SearchAndReplaceAll( "$$1", getDocumentTitle() );

    const ByteString aSaveStr( RESTOHTML( STR_WEBVIEW_SAVE ));
    aScript.SearchAndReplaceAll( "$$2", aSaveStr );

    aScript.SearchAndReplaceAll( "$$3", m_aCGIPath );

    aScript.SearchAndReplaceAll( "$$4", ByteString::CreateFromInt32(m_nWidthPixel) );
    aScript.SearchAndReplaceAll( "$$5", ByteString::CreateFromInt32(m_nHeightPixel) );


    ByteString aDest( rPath );
    aDest += rDest;

    m_eEC.SetContext( STR_HTMLEXP_ERROR_CREATE_FILE, rDest );
    // write script file
    {
        EasyFile aFile;
        SvStream* pStr;
        nErr = aFile.createStream(aDest, pStr);
        if(nErr == 0)
        {
            *pStr << aScript.GetBuffer();

            nErr = aFile.close();
        }
    }

    if (mpProgress)
        mpProgress->SetState(++m_nPagesWritten);

    if( nErr != 0 )
        ErrorHandler::HandleError( nErr );

    return nErr == 0;
}

char * ASP_Scripts[] = { "common.inc", "webcast.asp", "show.asp", "savepic.asp", "poll.asp", "editpic.asp" };

/** erzeugt und speichert die für WebShow benötigte ASP Scripte */
BOOL HtmlExport::CreateASPScripts()
{
    for( USHORT n = 0; n < (sizeof( ASP_Scripts ) / sizeof(char *)); n++ )
    {
        if(!CopyScript(m_aExportPath, ASP_Scripts[n], ASP_Scripts[n]))
            return FALSE;
    }

    if(!CopyScript(m_aExportPath, "edit.asp", m_aIndex ))
        return FALSE;

    return TRUE;
}


char *PERL_Scripts[] = { "webcast.pl", "common.pl", "editpic.pl", "poll.pl", "savepic.pl", "show.pl" };

/** erzeugt und speichert die für WebShow benötigte PERL Scripte */
BOOL HtmlExport::CreatePERLScripts()
{
    for( USHORT n = 0; n < (sizeof( PERL_Scripts ) / sizeof(char *)); n++ )
    {
        if(!CopyScript(m_aExportPath, PERL_Scripts[n], PERL_Scripts[n], true))
            return FALSE;
    }

    if(!CopyScript(m_aExportPath, "edit.pl", m_aIndex, true ))
        return FALSE;

    if(!CopyScript(m_aExportPath, "index.pl", m_aIndexUrl, true ))
        return FALSE;

    return TRUE;
}

/** Erzeugt eine Liste mit den Namen der gespeicherten Images */
BOOL HtmlExport::CreateImageFileList()
{
    ByteString aFull( m_aExportPath );
    ByteString aFileName( "picture.txt" );
    aFull += aFileName;

    m_eEC.SetContext( STR_HTMLEXP_ERROR_CREATE_FILE, aFileName );
    EasyFile aFile;
    SvStream* pStr;
    ULONG nErr = aFile.createStream(aFull, pStr);
    if(nErr == 0)
    {
        for( USHORT nSdPage = 0; nSdPage < m_nSdPageCount; nSdPage++)
        {
            ByteString aLine( ByteString::CreateFromInt32( nSdPage + 1 ) );
            aLine += ';';
            aLine += m_aURLPath;
            aLine += *m_pImageFiles[nSdPage];
            aLine += "\r\n";

            *pStr << aLine.GetBuffer();
        }

        nErr = aFile.close();
    }

    if (mpProgress)
        mpProgress->SetState(++m_nPagesWritten);

    if( nErr != 0 )
        ErrorHandler::HandleError( nErr );

    return nErr == 0;
}

/** Erzeugt das File mit der aktuellen Seitennumer */
BOOL HtmlExport::CreateImageNumberFile()
{
    ByteString aFull( m_aExportPath );
    ByteString aFileName( "currpic.txt" );
    aFull += aFileName;

    m_eEC.SetContext( STR_HTMLEXP_ERROR_CREATE_FILE, aFileName );
    EasyFile aFile;
    SvStream* pStr;
    ULONG nErr = aFile.createStream(aFull, pStr);
    if(nErr == 0)
    {
        *pStr << (const char *)"1";
        nErr = aFile.close();
    }

    if (mpProgress)
        mpProgress->SetState(++m_nPagesWritten);

    if( nErr != 0 )
        ErrorHandler::HandleError( nErr );

    return nErr == 0;
}

// =====================================================================

ByteString HtmlExport::InsertSound( const ByteString& rSoundFile )
{
    if( rSoundFile.Len() == 0 )
        return rSoundFile;

    ByteString      aStr( "<embed src=\"" );
    INetURLObject   aURL( String( rSoundFile, RTL_TEXTENCODING_UTF8 ) );

    DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

    aStr += ByteString( aURL.getName(), RTL_TEXTENCODING_UTF8 );
    aStr += "\" hidden=\"true\" autostart=\"true\">";

    CopyFile( rSoundFile, m_aExportPath );

    return aStr;
}

// =====================================================================

BOOL HtmlExport::CopyFile( const ByteString& rSourceFile, const ByteString& rDestPath )
{
    DirEntry aSourceEntry( String( rSourceFile, RTL_TEXTENCODING_UTF8 ) );
    DirEntry aDestEntry( String( rDestPath, RTL_TEXTENCODING_UTF8 ));

    m_eEC.SetContext( STR_HTMLEXP_ERROR_COPY_FILE, ByteString( aSourceEntry.GetName(), RTL_TEXTENCODING_UTF8 ), rDestPath );
    FSysError nError = aSourceEntry.CopyTo( aDestEntry, FSYS_ACTION_COPYFILE );

    if( nError != FSYS_ERR_OK )
    {
        ErrorHandler::HandleError(nError);
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

// ---

class HideSpecialObjectsInfo
{
public:
    HideSpecialObjectsInfo( SdrObject* pObj ) : mpObj( pObj ) {}

    SdrObject* mpObj;
    USHORT mnLineStyleState;
    USHORT mnFillStyleState;
    XLineStyle meLineStyle;
    XFillStyle meFillStyle;
};

// ---

void HtmlExport::HideSpecialObjects( SdPage* pPage )
{
    DBG_ASSERT( aSpecialObjects.Count() == 0, "SpecialObjects not shown after hide!" );

    SdrObject* pObj = NULL;
    SdAnimationInfo* pInfo = NULL;

    SdrObjListIter aIter(*pPage, IM_DEEPWITHGROUPS);

    while(aIter.IsMore())
    {
        pObj = aIter.Next();

        pInfo = pDoc->GetAnimationInfo(pObj);
        if(pInfo)
        {
            SdrPathObj* pPath = pInfo->pPathObj;

            if( pPath )
            {
                HideSpecialObjectsInfo* pHSOI = (HideSpecialObjectsInfo*)aSpecialObjects.First();
                while( pHSOI )
                {
                    if( pHSOI->mpObj == pPath )
                        break;
                    pHSOI = (HideSpecialObjectsInfo*)aSpecialObjects.Next();
                }

                if( pHSOI == NULL )
                {
                    SfxItemSet aSet(pDoc->GetPool());

                    aSet.Put(pPath->GetItemSet());

                    // not hided yet, so hide it
                    pHSOI = new HideSpecialObjectsInfo( pPath );

                    // remember LineStyle and FillStyle state and value
                    pHSOI->mnLineStyleState = aSet.GetItemState( XATTR_FILLSTYLE, FALSE );
                    if( pHSOI->mnLineStyleState == SFX_ITEM_SET )
                        pHSOI->meLineStyle = ( (const XLineStyleItem&) aSet.Get( XATTR_LINESTYLE ) ).GetValue();


                    pHSOI->mnFillStyleState = aSet.GetItemState( XATTR_LINESTYLE, FALSE );
                    if( pHSOI->mnFillStyleState == SFX_ITEM_SET )
                        pHSOI->meFillStyle = ( (const XFillStyleItem&) aSet.Get( XATTR_FILLSTYLE ) ).GetValue();

                    // enter stealth mode
                    XFillStyleItem aFillStyleItem(XFILL_NONE);
                    aSet.Put(aFillStyleItem);

                    XLineStyleItem aLineStyleItem(XLINE_NONE);
                    aSet.Put(aLineStyleItem);

                    aSpecialObjects.Insert( (void*)pHSOI );

                    pPath->SetItemSetAndBroadcast(aSet);
                }
            }
        }
    }

}

// =====================================================================

void HtmlExport::ShowSpecialObjects()
{
    HideSpecialObjectsInfo* pHSOI = (HideSpecialObjectsInfo*)aSpecialObjects.First();
    while( pHSOI )
    {
        SdrObject* pPath = pHSOI->mpObj;

        SfxItemSet aSet(pDoc->GetPool());
        aSet.Put(pPath->GetItemSet());

        if( pHSOI->mnLineStyleState == SFX_ITEM_SET )
        {
            XLineStyleItem aLineStyleItem( pHSOI->meLineStyle );
            aSet.Put( aLineStyleItem );
        }
        else
        {
            aSet.ClearItem( XATTR_LINESTYLE );
        }

        if( pHSOI->mnFillStyleState == SFX_ITEM_SET )
        {
            XFillStyleItem aFillStyleItem( pHSOI->meFillStyle );
            aSet.Put( aFillStyleItem );
        }
        else
        {
            aSet.ClearItem( XATTR_LINESTYLE );
        }

        pPath->SetItemSetAndBroadcast(aSet);

        delete pHSOI;

        pHSOI = (HideSpecialObjectsInfo*)aSpecialObjects.Next();
    }

    aSpecialObjects.Clear();
}

// =====================================================================
EasyFile::EasyFile()
{
    pMedium = NULL;
    pOStm = NULL;
    bOpen = FALSE;
}

// =====================================================================
EasyFile::~EasyFile()
{
    if( bOpen )
        close();
}

// =====================================================================
ULONG EasyFile::createStream(  const ByteString& rUrl, SvStream* &rpStr )
{
    ULONG nErr = 0;

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
            bOpen = TRUE;
            nErr = pOStm->GetError();
        }
        else
        {
            nErr = ERRCODE_SFX_CANTCREATECONTENT;
        }
    }

    if( nErr != 0 )
    {
        bOpen = FALSE;
        delete pMedium;
        delete pOStm;
        pOStm = NULL;
    }

    rpStr = pOStm;

    return nErr;
}

// =====================================================================
ULONG EasyFile::createFileName(  const ByteString& rURL, String& rFileName )
{
    ULONG nErr = 0;

    if( bOpen )
        nErr = close();

    if( nErr == 0 )
    {
        INetURLObject aURL( rURL );

        if( aURL.GetProtocol() == INET_PROT_NOT_VALID )
        {
            String aURLStr;
            ::utl::LocalFileHelper::ConvertPhysicalNameToURL( String( rURL, RTL_TEXTENCODING_UTF8 ), aURLStr );
            aURL = INetURLObject( aURLStr );
        }
        DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );
        rFileName = aURL.GetMainURL( INetURLObject::NO_DECODE );
    }

    return nErr;
}

// =====================================================================
ULONG EasyFile::close()
{
    ULONG nErr = 0;

    delete pOStm;
    pOStm = NULL;

    bOpen = FALSE;

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

HtmlErrorContext::HtmlErrorContext(Window *pWin)
: ErrorContext(pWin)
{
    m_nResId = 0;
}

// =====================================================================

BOOL HtmlErrorContext::GetString( ULONG nErrId, String& rCtxStr )
{
    DBG_ASSERT( m_nResId != 0, "No error context set" );
    if( m_nResId == 0 )
        return FALSE;

    rCtxStr = String( SdResId( m_nResId ) );

    rCtxStr.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM("$(URL1)")), String( m_aURL1, RTL_TEXTENCODING_UTF8 ));
    rCtxStr.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM("$(URL2)")), String( m_aURL2, RTL_TEXTENCODING_UTF8 ));

    return TRUE;
}

// =====================================================================

void HtmlErrorContext::SetContext( USHORT nResId )
{
    m_nResId = nResId;
    m_aURL1.Erase();
    m_aURL2.Erase();
}

// =====================================================================

void HtmlErrorContext::SetContext( USHORT nResId, const ByteString& rURL )
{
    m_nResId = nResId;
    m_aURL1 = rURL;
    m_aURL2.Erase();
}

// =====================================================================

void HtmlErrorContext::SetContext( USHORT nResId, const ByteString& rURL1, const ByteString& rURL2 )
{
    m_nResId = nResId;
    m_aURL1 = rURL1;
    m_aURL2 = rURL2;
}

// =====================================================================
