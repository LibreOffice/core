/*************************************************************************
 *
 *  $RCSfile: htmlex.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-11 11:11:21 $
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

#include <rtl/uri.hxx>

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
#ifndef SD_OUTLINER_HXX
#include "Outliner.hxx"
#endif
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
    bool        bOpen;

public:

    EasyFile();
    ~EasyFile();

    ULONG createStream( const String& rUrl, SvStream*& rpStr );
    ULONG createFileName(  const String& rUrl, String& rFileName );
    ULONG close();
};

// *********************************************************************
// Hilfsklasse fuer das einbinden von Textattributen in die Html-Ausgabe
// *********************************************************************
class HtmlState
{
private:
    bool m_bColor;
    bool m_bWeight;
    bool m_bItalic;
    bool m_bUnderline;
    bool m_bStrike;
    bool m_bLink;
    Color m_aColor;
    Color m_aDefColor;
    String m_aLink;
    String m_aTarget;

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
    aStr += SetColor(m_aDefColor);
    aStr += SetLink(aEmpty,aEmpty);

    return aStr;
}

// =====================================================================
// c'tor mit Defaultfarbe fuer die Seite
// =====================================================================
HtmlState::HtmlState( Color aDefColor )
{
    m_bColor = false;
    m_bWeight = false;
    m_bItalic = false;
    m_bUnderline = false;
    m_bLink = false;
    m_bStrike = false;
    m_aDefColor = aDefColor;
}

// =====================================================================
// aktiviert/deaktiviert Fettdruck
// =====================================================================
String HtmlState::SetWeight( bool bWeight )
{
    String aStr;

    if(bWeight && !m_bWeight)
        aStr.AppendAscii( "<b>" );
    else if(!bWeight && m_bWeight)
        aStr.AppendAscii( "</b>" );

    m_bWeight = bWeight;
    return aStr;
}

// =====================================================================
// aktiviert/deaktiviert Italic
// =====================================================================
String HtmlState::SetItalic( bool bItalic )
{
    String aStr;

    if(bItalic && !m_bItalic)
        aStr.AppendAscii( "<i>" );
    else if(!bItalic && m_bItalic)
        aStr.AppendAscii( "</i>" );

    m_bItalic = bItalic;
    return aStr;
}

// =====================================================================
// aktiviert/deaktiviert Unterstrichen
// =====================================================================
String HtmlState::SetUnderline( bool bUnderline )
{
    String aStr;

    if(bUnderline && !m_bUnderline)
        aStr.AppendAscii( "<u>" );
    else if(!bUnderline && m_bUnderline)
        aStr.AppendAscii( "</u>" );

    m_bUnderline = bUnderline;
    return aStr;
}

// =====================================================================
// aktiviert/deaktiviert Durchstreichen
// =====================================================================
String HtmlState::SetStrikeout( bool bStrike )
{
    String aStr;

    if(bStrike && !m_bStrike)
        aStr.AppendAscii( "<strike>" );
    else if(!bStrike && m_bStrike)
        aStr.AppendAscii( "</strike>" );

    m_bStrike = bStrike;
    return aStr;
}

// =====================================================================
// Setzt die angegebenne Textfarbe
// =====================================================================
String HtmlState::SetColor( Color aColor )
{
    String aStr;

    if(m_bColor && aColor == m_aColor)
        return aStr;

    if(m_bColor)
    {
        aStr.AppendAscii( "</font>" );
        m_bColor = false;
    }

    if(aColor != m_aDefColor)
    {
        m_aColor = aColor;

        aStr.AppendAscii( "<font color=" );
        aStr += HtmlExport::ColorToHTMLString(aColor);
        aStr.Append( sal_Unicode('>') );

        m_bColor = true;
    }

    return aStr;
}

// =====================================================================
// aktiviert/deaktiviert einen Hyperlink
// =====================================================================
String HtmlState::SetLink( const String& aLink, const String& aTarget )
{
    String aStr;

    if(m_bLink&&m_aLink == aLink&&m_aTarget==aTarget)
        return aStr;

    if(m_bLink)
    {
        aStr.AppendAscii( "</a>" );
        m_bLink = false;
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
        m_bLink = true;
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
HtmlExport::HtmlExport(
    OUString aPath,
    const Sequence< PropertyValue >& rParams,
    SdDrawDocument* pExpDoc,
    ::sd::DrawDocShell* pDocShell )
    :   pDoc(pExpDoc),
        m_aPath( aPath ),
        m_aHTMLExtension(SdResId(STR_HTMLEXP_DEFAULT_EXTENSION)),
        m_aIndexUrl(RTL_CONSTASCII_USTRINGPARAM("index")),
        m_pImageFiles(NULL),
        m_pHTMLFiles(NULL),
        m_pPageNames(NULL),
        m_pTextFiles(NULL),
        m_bUserAttr(false),
        m_bDocColors(false),
        m_bContentsPage(false),
        m_nButtonThema(-1),
        m_bNotes(false),
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

    m_aIndexUrl += m_aHTMLExtension;

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
    OUString aStr;
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
            pParams->Value >>= aStr;
            m_aIndexUrl = aStr;
        }
        else if( pParams->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Format" ) ) )
        {
            sal_Int32 temp;
            pParams->Value >>= temp;
            m_eFormat = (PublishingFormat)temp;
        }
        else if( pParams->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Compression" ) ) )
        {
            pParams->Value >>= aStr;
            String aTmp( aStr );
            if(aTmp.Len())
            {
                xub_StrLen nPos = aTmp.Search( '%' );
                if(nPos != STRING_NOTFOUND)
                    aTmp.Erase(nPos,1);
                m_nCompression = (INT16)aTmp.ToInt32();
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
            pParams->Value >>= aStr;
            m_aAuthor = aStr;
        }
        else if( pParams->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "EMail" ) ) )
        {
            pParams->Value >>= aStr;
            m_aEMail = aStr;
        }
        else if( pParams->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "HomepageURL" ) ) )
        {
            pParams->Value >>= aStr;
            m_aHomePage = aStr;
        }
        else if( pParams->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "UserText" ) ) )
        {
            pParams->Value >>= aStr;
            m_aInfo = aStr;
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
            pParams->Value >>= aStr;
            m_aCGIPath = aStr;
        }
        else if( pParams->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "WebCastTargetURL" ) ) )
        {
            pParams->Value >>= aStr;
            m_aURLPath = aStr;
        }
        else if( pParams->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "WebCastScriptLanguage" ) ) )
        {
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

    m_aExportPath = aINetURLObj.GetPartBeforeLastName();    // with trailing '/'
    m_aIndex = aINetURLObj.GetLastName();

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
        USHORT nSepPos = m_aDocFileName.Search( sal_Unicode('.') );

        if(nSepPos != STRING_NOTFOUND)
            m_aDocFileName.Erase(nSepPos);

        m_aDocFileName.AppendAscii( ".sxi" );
    }

    //////

    USHORT nProgrCount = m_nSdPageCount;
    nProgrCount += m_bImpress?m_nSdPageCount:0;
    nProgrCount += m_bContentsPage?1:0;
    nProgrCount += (m_bFrames && m_bNotes)?m_nSdPageCount:0;
    nProgrCount += (m_bFrames)?8:0;
    InitProgress( nProgrCount );

    pDocSh->SetWaitCursor( true );

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

        pDocSh->SetWaitCursor( false );
        ResetProgress();

        if( m_bDownload )
            SavePresentation();

        return;
    }

    // if we get to this point the export was
    // canceled by the user after an error
    pDocSh->SetWaitCursor( false );
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
        m_aCGIPath.Assign( sal_Unicode('.') );

    if( m_aCGIPath.GetChar( m_aCGIPath.Len() - 1 ) != sal_Unicode('/') )
        m_aCGIPath.Append( sal_Unicode('/') );

    if( m_eScript == SCRIPT_ASP )
    {
        m_aURLPath.AssignAscii( "./" );
    }
    else
    {
           String aEmpty2;
        if(m_aURLPath.Len() == 0)
            m_aURLPath.Assign( sal_Unicode('.') );

        if( m_aURLPath.GetChar( m_aURLPath.Len() - 1 ) != sal_Unicode('/') )
            m_aURLPath.Append( sal_Unicode('/') );
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

    pDocSh->SetWaitCursor( false );
    ResetProgress();
}

///////////////////////////////////////////////////////////////////////
// Save the presentation as a downloadable file in the dest directory
///////////////////////////////////////////////////////////////////////

bool HtmlExport::SavePresentation()
{
    m_eEC.SetContext( STR_HTMLEXP_ERROR_CREATE_FILE, m_aDocFileName );

    OUString aURL( m_aExportPath );
    aURL += m_aDocFileName;


    pDocSh->EnableSetModified( true );

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
            xStorable->storeToURL( aURL, aProperties );

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
bool HtmlExport::CreateImagesForPresPages()
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

            OUString aFull(m_aExportPath);
            aFull += *m_pImageFiles[nSdPage];

            aDescriptor[0].Value <<= aFull;

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

String HtmlExport::WriteMetaCharset() const
{
    String aStr;
    const sal_Char *pCharSet = rtl_getBestMimeCharsetFromTextEncoding( RTL_TEXTENCODING_UTF8 );
    if ( pCharSet )
    {
        aStr.AppendAscii( "<meta HTTP-EQUIV=CONTENT-TYPE CONTENT=\"text/html; charset=" );
        aStr.AppendAscii( pCharSet );
        aStr.AppendAscii( "\">\r\n" );
    }
    return aStr;
}

bool HtmlExport::CreateHtmlTextForPresPages()
{
    bool bOk = true;

    SdrOutliner* pOutliner = pDoc->GetInternalOutliner();

    for(USHORT nSdPage = 0; nSdPage < m_nSdPageCount && bOk; nSdPage++)
    {
        SdPage* pPage = pDoc->GetSdPage(nSdPage, PK_STANDARD);

        if( m_bDocColors )
        {
            SetDocColors( pPage );
//          m_aBackColor = pPage->GetBackgroundColor();
        }

// HTML Kopf
        String aStr(RTL_CONSTASCII_USTRINGPARAM("<html>\r\n<head>\r\n"));
        aStr += WriteMetaCharset();
        aStr.AppendAscii( "<title>" );
        aStr += StringToHTMLString( *m_pPageNames[nSdPage] );
        aStr.AppendAscii( "</title>\r\n" );
        aStr.AppendAscii( "</head>\r\n" );
        aStr += CreateBodyTag();

// Navigationsleiste
        aStr += CreateNavBar(nSdPage, true);

// Seitentitel
        aStr.AppendAscii( "<h1>" );
        aStr += CreateTextForTitle(pOutliner,pPage, pPage->GetBackgroundColor());
        aStr.AppendAscii( "</h1><p>\r\n" );

// Gliederungstext schreiben
        aStr += CreateTextForPage( pOutliner, pPage, true, pPage->GetBackgroundColor() );

// Notizen
        if(m_bNotes)
        {
            aStr.AppendAscii( "<br>\r\n<h3>" );
            aStr += RESTOHTML(STR_HTMLEXP_NOTES);
            aStr.AppendAscii( ":</h3>\r\n" );

            SdPage* pNotesPage = pDoc->GetSdPage(nSdPage, PK_NOTES);

            aStr += CreateTextForNotesPage( pOutliner, pNotesPage, true, m_aBackColor);
        }

// Seite beenden
        aStr.AppendAscii( "</body>\r\n</html>" );

        bOk = WriteHtml( *m_pTextFiles[nSdPage], false, aStr );

        if (mpProgress)
            mpProgress->SetState(++m_nPagesWritten);

    }

    pOutliner->Clear();

    return bOk;
}

/** exports the given html data into a non unicode file in the current export path with
    the given filename */
bool HtmlExport::WriteHtml( const String& rFileName, bool bAddExtension, const String& rHtmlData )
{
    ULONG nErr = 0;

    String aFileName( rFileName );
    if( bAddExtension )
        aFileName += m_aHTMLExtension;

    m_eEC.SetContext( STR_HTMLEXP_ERROR_CREATE_FILE, rFileName );
    EasyFile aFile;
    SvStream* pStr;
    String aFull( m_aExportPath );
    aFull += aFileName;
    nErr = aFile.createStream(aFull , pStr);
    if(nErr == 0)
    {
        ByteString aStr( rHtmlData , RTL_TEXTENCODING_UTF8 ) ;
        *pStr << aStr.GetBuffer();
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

            ULONG nCount = pOutliner->GetParagraphCount();

            Paragraph* pPara = NULL;
            USHORT nActDepth = 1;

            String aParaText;
            aStr.AppendAscii( "<ul>" );
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
                              aStr.AppendAscii( "</ul>" );
                              nActDepth--;
                            }
                            while(nDepth < nActDepth);
                        }
                        else if(nDepth > nActDepth )
                        {
                            aStr.AppendAscii( "<ul>" );
                            nActDepth = nDepth;
                        }

                        aParaText = ParagraphToHTMLString(pOutliner,nPara,rBackgroundColor);

                        if(aParaText.Len() != 0)
                        {
                            aStr.AppendAscii( "<li>" );
                            if(nActDepth == 1 && bHeadLine)
                                aStr.AppendAscii( "<h2>" );
                            aStr += aParaText;
                            if(nActDepth == 1 && bHeadLine)
                                aStr.AppendAscii( "</h2>" );
                            aStr.AppendAscii( "\r\n" );
                        }
                    }
                }

                if( nActDepth > 0 ) do
                {
                    aStr.AppendAscii( "</ul>" );
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
String HtmlExport::CreateTextForNotesPage( SdrOutliner* pOutliner,
                                           SdPage* pPage,
                                           bool bHeadLine,
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

            ULONG nCount = pOutliner->GetParagraphCount();
            for (ULONG nPara = 0; nPara < nCount; nPara++)
            {
                aStr += ParagraphToHTMLString( pOutliner, nPara,rBackgroundColor );
                aStr.AppendAscii( "<br>\r\n" );
            }
        }
    }

    return aStr;
}

// =====================================================================
// Wandelt einen Paragraphen des Outliners in Html
// =====================================================================
String HtmlExport::ParagraphToHTMLString( SdrOutliner* pOutliner, ULONG nPara, const Color& rBackgroundColor )
{
    String aStr;

    if(NULL == pOutliner)
        return aStr;

    // TODO: MALTE!!!
    EditEngine& rEditEngine = *(EditEngine*)&pOutliner->GetEditEngine();
    bool bOldUpdateMode = rEditEngine.GetUpdateMode();
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
bool HtmlExport::CreateHtmlForPresPages()
{
    bool bOk = true;

    List aClickableObjects;

    for(USHORT nSdPage = 0; nSdPage < m_nSdPageCount && bOk; nSdPage++)
    {
        // Klickbare Objekte finden (auch auf der Masterpage) und
        // in Liste stellen. In umgekehrter Zeichenreihenfolge in
        // die Liste stellen, da in HTML bei Ueberlappungen die
        // _erstgenannte_ Area wirkt.

        SdPage* pPage = pDoc->GetSdPage(nSdPage, PK_STANDARD);

        if( m_bDocColors )
        {
            SetDocColors( pPage );
        }

        bool    bMasterDone = false;

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
                pPage = (SdPage*)(&(pPage->TRG_GetMasterPage()));
            else
                bMasterDone = true;
        }
        ULONG nClickableObjectCount = aClickableObjects.Count();

// HTML Head
        String aStr( RTL_CONSTASCII_USTRINGPARAM("<html>\r\n<head>\r\n") );
        aStr += WriteMetaCharset();
        aStr.AppendAscii( "<title>" );
        aStr += StringToHTMLString(*m_pPageNames[nSdPage]);
        aStr.AppendAscii( "</title>\r\n" );

// insert timing information
        pPage = pDoc->GetSdPage(nSdPage, PK_STANDARD);
        if( m_eMode == PUBLISH_KIOSK )
        {
            ULONG nSecs = 0;
            bool bEndless = false;
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
                    aStr.AppendAscii( "<meta http-equiv=\"refresh\" content=\"" );
                    aStr += String::CreateFromInt32(nSecs);
                    aStr.AppendAscii( "; URL=" );

                    int nPage = nSdPage + 1;
                    if( nPage == m_nSdPageCount )
                        nPage = 0;

                    aStr += StringToURL(*m_pHTMLFiles[nPage]);

                    aStr.AppendAscii( "\">\r\n" );
                }
            }
        }

        aStr.AppendAscii( "</head>\r\n" );

// HTML Body
        aStr += CreateBodyTag();

        if( m_bSlideSound && pPage->IsSoundOn() )
            aStr += InsertSound( pPage->GetSoundFile() );

// Navigationsleiste
        if(!m_bFrames )
            aStr += CreateNavBar( nSdPage, false );
// Image
        aStr.AppendAscii( "<center>" );
        aStr.AppendAscii( "<img src=\"" );
        aStr += StringToURL( *m_pImageFiles[nSdPage] );
        aStr.Append(sal_Unicode('"'));

        if (nClickableObjectCount > 0)
            aStr.AppendAscii( " USEMAP=\"#map0\"" );

        aStr.AppendAscii( "></center>\r\n" );

// Notizen
        if(m_bNotes && !m_bFrames)
        {
            SdrOutliner* pOutliner = pDoc->GetInternalOutliner();
            aStr.AppendAscii( "<p><h3>" );
            aStr += RESTOHTML(STR_HTMLEXP_NOTES);
            aStr.AppendAscii( ":</h3><br>\r\n" );

            SdPage* pNotesPage = pDoc->GetSdPage(nSdPage, PK_NOTES);
            aStr += CreateTextForNotesPage( pOutliner, pNotesPage, true, m_aBackColor);
            pOutliner->Clear();
        }

// ggfs. Imagemap erzeugen
        if (nClickableObjectCount > 0)
        {
            aStr.AppendAscii( "<map name=\"map0\">\r\n" );

            for (ULONG nObject = 0; nObject < nClickableObjectCount; nObject++)
            {
                SdrObject* pObject = (SdrObject*)aClickableObjects.GetObject(nObject);
                SdAnimationInfo* pInfo     = pDoc->GetAnimationInfo(pObject);
                SdIMapInfo*      pIMapInfo = pDoc->GetIMapInfo(pObject);

                Rectangle aRect(pObject->GetCurrentBoundRect());
                Point     aLogPos(aRect.TopLeft());
                bool      bIsSquare = aRect.GetWidth() == aRect.GetHeight();

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
                        String aURL( pArea->GetURL() );

                        // ggfs. Seiten- oder Objektnamen umwandeln in den
                        // Namen der entsprechenden HTML-Datei
                        BOOL        bIsMasterPage;
                        USHORT      nPgNum = pDoc->GetPageByName( aURL, bIsMasterPage );
                        SdrObject*  pObj = NULL;

                        if (nPgNum == SDRPAGE_NOTFOUND)
                        {
                            // Ist das Bookmark ein Objekt?
                            pObj = pDoc->GetObj( aURL );
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

                                ULONG nRadius = (((IMapCircleObject*)pArea)->
                                                 GetRadius(false));
                                nRadius = (ULONG)(nRadius * fLogicToPixel);
                                aStr += CreateHTMLCircleArea(nRadius,
                                                            aCenter.X(), aCenter.Y(),
                                                            aURL);
                            }
                            break;

                            case IMAP_OBJ_POLYGON:
                            {
                                Polygon aArea(((IMapPolygonObject*)pArea)->
                                               GetPolygon(false));
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
                    String      aHRef;
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
                            aHRef = pInfo->aBookmark;
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

            aStr.AppendAscii( "</map>\r\n" );
        }
        aClickableObjects.Clear();

        aStr.AppendAscii( "</body>\r\n</html>" );

        bOk = WriteHtml( *m_pHTMLFiles[nSdPage], false, aStr );

        if (mpProgress)
            mpProgress->SetState(++m_nPagesWritten);
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

    if( m_bDocColors )
        SetDocColors();

    // Html Kopf
    String aStr( RTL_CONSTASCII_USTRINGPARAM("<html>\r\n<head>\r\n") );
    aStr += WriteMetaCharset();
    aStr.AppendAscii( "<title>" );
    aStr += StringToHTMLString(*m_pPageNames[0]);
    aStr.AppendAscii( "</title>\r\n</head>\r\n" );
    aStr += CreateBodyTag();

    // Seitenkopf
    aStr.AppendAscii( "<center>\r\n" );

    if(m_bHeader)
    {
        aStr.AppendAscii( "<h1>" );
        aStr += getDocumentTitle();
        aStr.AppendAscii( "</h1><br>\r\n" );
    }

    aStr.AppendAscii( "<h2>" );

    // #92564# Solaris compiler bug workaround
    if( m_bFrames )
        aStr += CreateLink( m_aFramePage,
                            RESTOHTML(STR_HTMLEXP_CLICKSTART) );
    else
        aStr += CreateLink( StringToHTMLString(*m_pHTMLFiles[0]),
                            RESTOHTML(STR_HTMLEXP_CLICKSTART) );

    aStr.AppendAscii( "</h2>\r\n</center>\r\n" );

    aStr.AppendAscii( "<center><table width=90%><TR>\r\n" );

    // Inhaltsverzeichnis
    aStr.AppendAscii( "<td valign=top align=left width=50%>\r\n" );
    aStr.AppendAscii( "<h3><u>" );
    aStr += RESTOHTML(STR_HTMLEXP_CONTENTS);
    aStr.AppendAscii( "</u></h3>" );

    for(USHORT nSdPage = 0; nSdPage < m_nSdPageCount; nSdPage++)
    {
        SdPage* pPage = pDoc->GetSdPage(nSdPage, PK_STANDARD);

        String aPageName = *m_pPageNames[nSdPage];
        aStr.AppendAscii( "<p align=left>" );
        if(m_bFrames)
            aStr += StringToHTMLString(aPageName);
        else
            aStr += CreateLink(*m_pHTMLFiles[nSdPage], aPageName);
        aStr.AppendAscii( "</p>\r\n" );
    }
    aStr.AppendAscii( "</td>\r\n" );

    // Dokument Infos
    aStr.AppendAscii( "<td valign=top width=50%>\r\n" );

    if(m_aAuthor.Len())
    {
        aStr.AppendAscii( "<p><strong>" );
        aStr += RESTOHTML(STR_HTMLEXP_AUTHOR);
        aStr.AppendAscii( ":</strong> " );
        aStr += StringToHTMLString(m_aAuthor);
        aStr.AppendAscii( "</p>\r\n" );
    }

    if(m_aEMail.Len())
    {
        aStr.AppendAscii( "<p><strong>" );
        aStr += RESTOHTML(STR_HTMLEXP_EMAIL);
        aStr.AppendAscii( ":</strong> <a href=\"mailto:" );
        aStr += StringToURL(m_aEMail);
        aStr.AppendAscii( "\">" );
        aStr += StringToHTMLString(m_aEMail);
        aStr.AppendAscii( "</a></p>\r\n" );
    }

    if(m_aHomePage.Len())
    {
        aStr.AppendAscii( "<p><strong>" );
        aStr += RESTOHTML(STR_HTMLEXP_HOMEPAGE);
        aStr.AppendAscii( ":</strong> <a href=\"" );
        aStr += StringToURL(m_aHomePage);
        aStr.AppendAscii( "\">" );
        aStr += StringToHTMLString(m_aHomePage);
        aStr.AppendAscii( "</a> </p>\r\n" );
    }

    if(m_aInfo.Len())
    {
        aStr.AppendAscii( "<p><strong>" );
        aStr += RESTOHTML(STR_HTMLEXP_INFO);
        aStr.AppendAscii( ":</strong><br>\r\n" );
        aStr += StringToHTMLString(m_aInfo);
        aStr.AppendAscii( "</p>\r\n" );
    }

    if(m_bDownload)
    {
        aStr.AppendAscii( "<p><a href=\"" );
        aStr += StringToURL(m_aDocFileName);
        aStr.AppendAscii( "\">" );
        aStr += RESTOHTML(STR_HTMLEXP_DOWNLOAD);
        aStr.AppendAscii( "</a></p>\r\n" );
    }

    aStr.AppendAscii( "</td></tr></table></center>\r\n" );

    aStr.AppendAscii( "</body>\r\n</html>" );

    bool bOk = WriteHtml( m_aIndex, false, aStr );

    if (mpProgress)
        mpProgress->SetState(++m_nPagesWritten);

    return bOk;
}

// =====================================================================
// Notiz Seiten erzeugen (fuer Frames)
// =====================================================================
bool HtmlExport::CreateNotesPages()
{
    bool bOk = true;

    SdrOutliner* pOutliner = pDoc->GetInternalOutliner();
    for( USHORT nSdPage = 0; bOk && nSdPage < m_nSdPageCount; nSdPage++ )
    {
        SdPage* pPage = pDoc->GetSdPage(nSdPage, PK_NOTES);
        if( m_bDocColors )
            SetDocColors( pPage );

        // Html Kopf
        String aStr( RTL_CONSTASCII_USTRINGPARAM("<html>\r\n<head>\r\n") );
        aStr += WriteMetaCharset();
        aStr.AppendAscii( "<title>" );
        aStr += StringToHTMLString(*m_pPageNames[0]);
        aStr.AppendAscii( "</title>\r\n</head>\r\n" );
        aStr += CreateBodyTag();

        if(pPage)
            aStr += CreateTextForNotesPage( pOutliner, pPage, true, m_aBackColor );

        aStr.AppendAscii( "</body>\r\n</html>" );

        String aFileName( RTL_CONSTASCII_USTRINGPARAM("note") );
        aFileName += String::CreateFromInt32(nSdPage);
        bOk = WriteHtml( aFileName, true, aStr );

        if (mpProgress)
            mpProgress->SetState(++m_nPagesWritten);
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

    if( m_bDocColors )
    {
        SetDocColors();
    }

    // Seite 0 wird der zugeklappte Outline, Seite 1 der aufgeklappte
    for( int nPage = 0; nPage < (m_bImpress?2:1) && bOk; nPage++ )
    {
        // Html Kopf
        String aStr( RTL_CONSTASCII_USTRINGPARAM("<html>\r\n<head>\r\n" ));
        aStr += WriteMetaCharset();
        aStr.AppendAscii( "<title>" );
        aStr += StringToHTMLString(*m_pPageNames[0]);
        aStr.AppendAscii( "</title>\r\n</head>\r\n" );
        aStr += CreateBodyTag();

        SdrOutliner* pOutliner = pDoc->GetInternalOutliner();
        for(USHORT nSdPage = 0; nSdPage < m_nSdPageCount; nSdPage++)
        {
            SdPage* pPage = pDoc->GetSdPage(nSdPage, PK_STANDARD);

            aStr.AppendAscii( "<p align=left>" );
            String aLink( RTL_CONSTASCII_USTRINGPARAM( "JavaScript:parent.NavigateAbs(" ) );
            aLink += String::CreateFromInt32(nSdPage);
            aLink.Append( sal_Unicode(')') );

            String aTitle = CreateTextForTitle(pOutliner,pPage, m_aBackColor);
            if(aTitle.Len() == 0)
                aTitle = *m_pPageNames[nSdPage];
            aStr += CreateLink(aLink, aTitle);

            if(nPage==1)
            {
                aStr.AppendAscii( "<br>" );
                aStr += CreateTextForPage( pOutliner, pPage, false, m_aBackColor );
            }
            aStr.AppendAscii( "</p>\r\n" );
        }
        pOutliner->Clear();

        aStr.AppendAscii( "</body>\r\n</html>" );

        String aFileName( RTL_CONSTASCII_USTRINGPARAM("outline") );
        aFileName += String::CreateFromInt32(nPage);
        bOk = WriteHtml( aFileName, true, aStr );

        if (mpProgress)
            mpProgress->SetState(++m_nPagesWritten);
    }

    return bOk;
}

// =====================================================================
// Dateinamen festlegen
// =====================================================================
void HtmlExport::CreateFileNames()
{
    // Listen mit neuen Dateinamen anlegen
    m_pHTMLFiles = new String*[m_nSdPageCount];
    m_pImageFiles = new String*[m_nSdPageCount];
    m_pPageNames = new String*[m_nSdPageCount];
    m_pTextFiles = new String*[m_nSdPageCount];

    m_bHeader = false;  // Ueberschrift auf Uebersichtsseite?

    for (USHORT nSdPage = 0; nSdPage < m_nSdPageCount; nSdPage++)
    {
        String* pName;
        if(nSdPage == 0 && !m_bContentsPage && !m_bFrames )
            pName = new String(m_aIndex);
        else
        {
            pName = new String( RTL_CONSTASCII_USTRINGPARAM("img") );
            *pName += String::CreateFromInt32(nSdPage);
            *pName += m_aHTMLExtension;
        }

        m_pHTMLFiles[nSdPage] = pName;

        pName = new String( RTL_CONSTASCII_USTRINGPARAM("img") );
        *pName += String::CreateFromInt32(nSdPage);
        if( m_eFormat==FORMAT_GIF )
            pName->AppendAscii( ".gif" );
        else
            pName->AppendAscii( ".jpg" );

        m_pImageFiles[nSdPage] = pName;

        pName = new String( RTL_CONSTASCII_USTRINGPARAM("text"));
        *pName += String::CreateFromInt32(nSdPage);
        *pName += m_aHTMLExtension;
        m_pTextFiles[nSdPage] = pName;

        SdPage* pSdPage = pDoc->GetSdPage(nSdPage, PK_STANDARD);

        // get slide title from page name
        String* pPageTitle = new String();
        *pPageTitle = pSdPage->GetName();
        m_pPageNames[nSdPage] = pPageTitle;
    }

    if(!m_bContentsPage && m_bFrames)
        m_aFramePage = m_aIndex;
    else
    {
        m_aFramePage.AssignAscii( "siframes" );
        m_aFramePage += m_aHTMLExtension;
    }
}

String HtmlExport::getDocumentTitle()
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
                        String aTest(rEditTextObject.GetText(0));
                        if (aTest.Len() > 0)
                            m_DocTitle = aTest;
                    }
                }
            }

            for( UINT16 i = 0; i < m_DocTitle.Len(); i++ )
                if( m_DocTitle.GetChar(i) == (sal_Unicode)0xff)
                    m_DocTitle.SetChar(i, sal_Unicode(' ') );
        }

        if( !m_DocTitle.Len() )
        {
            m_DocTitle = m_aDocFileName;
            int nDot = m_DocTitle.Search( '.' );
            if( nDot > 0 )
                m_DocTitle.Erase( nDot );
        }
        m_bHeader = true;
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
bool HtmlExport::CreateFrames()
{
    String aTmp;
    String aStr( RTL_CONSTASCII_USTRINGPARAM("<html>\r\n<head>") );
    aStr += WriteMetaCharset();
    aStr.AppendAscii( "<title>" );
    aStr += StringToHTMLString(*m_pPageNames[0]);
    aStr.AppendAscii( "</title>\r\n" );

    aStr.AppendAscii( "<script language=\"JavaScript\">\r\n<!--\r\n" );

    aStr.AppendAscii( "var nCurrentPage = 0;\r\nvar nPageCount = " );
    aStr += String::CreateFromInt32(pDoc->GetSdPageCount( PK_STANDARD ));
    aStr.AppendAscii( ";\r\n\r\n" );

    String aFunction;
    aFunction.AssignAscii(JS_NavigateAbs);

    if(m_bNotes)
    {
        String aEmpty;
        String aSlash( RTL_CONSTASCII_USTRINGPARAM( "//" ) );
        aFunction.SearchAndReplaceAll( aSlash, aEmpty);
    }

    // substitute HTML file extension
    String aPlaceHolder(RTL_CONSTASCII_USTRINGPARAM(".$EXT"));
    aFunction.SearchAndReplaceAll(aPlaceHolder, m_aHTMLExtension);
    aStr += aFunction;

    aTmp.AssignAscii( JS_NavigateRel );
    aTmp.SearchAndReplaceAll(aPlaceHolder, m_aHTMLExtension);
    aStr += aTmp;

    if(m_bImpress)
    {
        aTmp.AssignAscii( JS_ExpandOutline );
        aTmp.SearchAndReplaceAll(aPlaceHolder, m_aHTMLExtension);
        aStr += aTmp;

        aTmp.AssignAscii( JS_CollapseOutline );
        aTmp.SearchAndReplaceAll(aPlaceHolder, m_aHTMLExtension);
        aStr += aTmp;
    }
    aStr.AppendAscii( "// -->\r\n</SCRIPT>\r\n" );

    aStr.AppendAscii( "</head>\r\n" );

    aStr.AppendAscii( "<frameset cols=\"*," );
    aStr += String::CreateFromInt32((m_nWidthPixel + 16));
    aStr.AppendAscii( "\">\r\n" );
    if(m_bImpress)
    {
        aStr.AppendAscii( "  <frameset rows=\"42,*\">\r\n" );
        aStr.AppendAscii( "    <frame src=\"navbar3" );
        aStr += StringToURL(m_aHTMLExtension);
        aStr.AppendAscii( "\" name=\"navbar2\" marginwidth=4 marginheight=4 scrolling=no>\r\n" );
    }
    aStr.AppendAscii( "    <frame src=\"outline0" );
    aStr += StringToURL(m_aHTMLExtension);
    aStr.AppendAscii( "\" name=\"outline\">\r\n" );
    if(m_bImpress)
        aStr.AppendAscii( "  </frameset>\r\n" );

    if(m_bNotes)
    {
        aStr.AppendAscii( "  <frameset rows=\"42," );
        aStr += String::CreateFromInt32((int)((double)m_nWidthPixel * 0.75) + 16);
        aStr.AppendAscii( ",*\">\r\n" );
    }
    else
        aStr.AppendAscii( "  <frameset rows=\"42,*\">\r\n" );

    aStr.AppendAscii( "    <frame src=\"navbar0" );
    aStr += StringToURL(m_aHTMLExtension);
    aStr.AppendAscii( "\" name=\"navbar1\" marginwidth=4 marginheight=4 scrolling=no>\r\n" );

    aStr.AppendAscii( "    <frame src=\"" );
    aStr += StringToURL(*m_pHTMLFiles[0]);
    aStr.AppendAscii( "\" name=\"show\" marginwidth=4 marginheight=4>\r\n" );

    if(m_bNotes)
    {
        aStr.AppendAscii( "    <frame src=\"note0" );
        aStr += StringToURL(m_aHTMLExtension);
        aStr.AppendAscii( "\" name=\"notes\">\r\n" );
    }
    aStr.AppendAscii( "  </frameset>\r\n" );

    aStr.AppendAscii( "</frameset>\r\n" );

    aStr.AppendAscii( "<noframes>" );
    aStr += CreateBodyTag();
    aStr += RESTOHTML(STR_HTMLEXP_NOFRAMES);
    aStr.AppendAscii( "\r\n</noframes></body>\r\n</html>" );

    bool bOk = WriteHtml( m_aFramePage, false, aStr );

    if (mpProgress)
        mpProgress->SetState(++m_nPagesWritten);

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

    if( m_bDocColors )
    {
        SetDocColors();
        m_aBackColor = m_aFirstPageColor;
    }

    for( int nFile = 0; nFile < 3 && bOk; nFile++ )
    {
        String aStr( RTL_CONSTASCII_USTRINGPARAM("<html>\r\n<head>\r\n") );
        aStr += WriteMetaCharset();
        aStr.AppendAscii( "<title>" );
        aStr += StringToHTMLString(*m_pPageNames[0]);
        aStr.AppendAscii( "</title>\r\n</head>\r\n" );
        aStr += CreateBodyTag();
        aStr.AppendAscii( "<center>\r\n" );

    // erste Seite
        aButton = String(SdResId(STR_HTMLEXP_FIRSTPAGE));
        if(m_nButtonThema != -1)
            aButton = CreateImage(GetButtonName((nFile == 0 || m_nSdPageCount == 1?
                                  BTN_FIRST_0:BTN_FIRST_1)), aButton);

        if(nFile != 0 && m_nSdPageCount > 1)
            aButton = CreateLink( String(RTL_CONSTASCII_USTRINGPARAM("JavaScript:parent.NavigateAbs(0)")), aButton);

        aStr += aButton;
        aStr.AppendAscii( "\r\n" );

    // zur vorherigen Seite
        aButton = String(SdResId(STR_PUBLISH_BACK));
        if(m_nButtonThema != -1)
            aButton = CreateImage(GetButtonName((nFile == 0 || m_nSdPageCount == 1?
                                    BTN_PREV_0:BTN_PREV_1)), aButton);

        if(nFile != 0 && m_nSdPageCount > 1)
            aButton = CreateLink( String(RTL_CONSTASCII_USTRINGPARAM("JavaScript:parent.NavigateRel(-1)")), aButton);

        aStr += aButton;
        aStr.AppendAscii( "\r\n" );

    // zur naechsten Seite
        aButton = String(SdResId(STR_PUBLISH_NEXT));
        if(m_nButtonThema != -1)
            aButton = CreateImage(GetButtonName((nFile ==2 || m_nSdPageCount == 1?
                                    BTN_NEXT_0:BTN_NEXT_1)), aButton);

        if(nFile != 2 && m_nSdPageCount > 1)
            aButton = CreateLink(String(RTL_CONSTASCII_USTRINGPARAM("JavaScript:parent.NavigateRel(1)")), aButton);

        aStr += aButton;
        aStr.AppendAscii( "\r\n" );

    // zur letzten Seite
        aButton = String(SdResId(STR_HTMLEXP_LASTPAGE));
        if(m_nButtonThema != -1)
            aButton = CreateImage(GetButtonName((nFile ==2 || m_nSdPageCount == 1?
                                  BTN_LAST_0:BTN_LAST_1)), aButton);

        if(nFile != 2 && m_nSdPageCount > 1)
        {
            String aLink(RTL_CONSTASCII_USTRINGPARAM("JavaScript:parent.NavigateAbs("));
            aLink += String::CreateFromInt32(pDoc->GetSdPageCount( PK_STANDARD )-1);
            aLink.AppendAscii( ")" );
            aButton = CreateLink( aLink, aButton);
        }

        aStr += aButton;
        aStr.AppendAscii( "\r\n" );

    // Inhalt
        if (m_bContentsPage)
        {
            aButton = String(SdResId(STR_PUBLISH_OUTLINE));
            if(m_nButtonThema != -1)
                aButton = CreateImage(GetButtonName(BTN_INDEX), aButton);

            // zur Uebersicht
            aStr += CreateLink(m_aIndex, aButton, String(RTL_CONSTASCII_USTRINGPARAM("_top")));
            aStr.AppendAscii( "\r\n" );
        }

    // Textmodus
        if(m_bImpress)
        {
            aButton = String(SdResId(STR_HTMLEXP_SETTEXT));
            if(m_nButtonThema != -1)
                aButton = CreateImage(GetButtonName(BTN_TEXT), aButton);

            String  aText0( RTL_CONSTASCII_USTRINGPARAM("text0"));
            aText0 += m_aHTMLExtension;
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
            mpProgress->SetState(++m_nPagesWritten);
    }

    // Jetzt kommt die Navigatonsleiste Outliner zugeklappt...
    if(bOk)
    {
        String aStr( RTL_CONSTASCII_USTRINGPARAM("<html>\r\n<head>\r\n") );
        aStr += WriteMetaCharset();
        aStr.AppendAscii( "<title>" );
        aStr += StringToHTMLString(*m_pPageNames[0]);
        aStr.AppendAscii( "</title>\r\n</head>\r\n" );
        aStr += CreateBodyTag();

        String aButton;

        aButton = String(SdResId(STR_HTMLEXP_OUTLINE));
        if(m_nButtonThema != -1)
            aButton = CreateImage(GetButtonName(BTN_LESS), aButton);

        aStr += CreateLink(String(RTL_CONSTASCII_USTRINGPARAM("JavaScript:parent.ExpandOutline()")), aButton);
        aStr.AppendAscii( "</body>\r\n</html>" );

        String aFileName( RTL_CONSTASCII_USTRINGPARAM("navbar3") );

        bOk = WriteHtml( aFileName, true, aStr );

        if (mpProgress)
            mpProgress->SetState(++m_nPagesWritten);
    }

    // ... und jetzt Outliner aufgeklappt
    if( bOk )
    {
        String aStr( RTL_CONSTASCII_USTRINGPARAM("<html>\r\n<head>\r\n") );
        aStr += WriteMetaCharset();
        aStr.AppendAscii( "<title>" );
        aStr += StringToHTMLString(*m_pPageNames[0]);
        aStr.AppendAscii( "</title>\r\n</head>\r\n" );
        aStr += CreateBodyTag();

        String aButton;

        aButton = String(SdResId(STR_HTMLEXP_NOOUTLINE));
        if(m_nButtonThema != -1)
            aButton = CreateImage(GetButtonName(BTN_MORE), aButton);

        aStr += CreateLink(String(RTL_CONSTASCII_USTRINGPARAM("JavaScript:parent.CollapseOutline()")), aButton);
        aStr.AppendAscii( "</body>\r\n</html>" );

        String aFileName( RTL_CONSTASCII_USTRINGPARAM("navbar4") );
        bOk = WriteHtml( aFileName, true, aStr );

        if (mpProgress)
            mpProgress->SetState(++m_nPagesWritten);

    }

    return bOk;
}

// ====================================================================
// Buttonleiste fuer Standard ausgeben
// ====================================================================
String HtmlExport::CreateNavBar( USHORT nSdPage, bool bIsText ) const
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

    if(!bIsText && m_nButtonThema != -1)
    {
        if(nSdPage<1 || m_nSdPageCount == 1)
        {
            aStrNavFirst = CreateImage(GetButtonName(BTN_FIRST_0), aStrNavFirst);
            aStrNavPrev  = CreateImage(GetButtonName(BTN_PREV_0), aStrNavPrev);
        }
        else
        {
            aStrNavFirst = CreateImage(GetButtonName(BTN_FIRST_1), aStrNavFirst);
            aStrNavPrev  = CreateImage(GetButtonName(BTN_PREV_1), aStrNavPrev);
        }

        if(nSdPage == m_nSdPageCount-1 || m_nSdPageCount == 1)
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
        aStr += CreateLink(bIsText?*m_pTextFiles[0]:*m_pHTMLFiles[0],aStrNavFirst);
    else
        aStr += aStrNavFirst;
    aStr.Append(sal_Unicode(' '));

    // zur vorherigen Seite
    if(nSdPage > 0)
        aStr += CreateLink( bIsText?*m_pTextFiles[nSdPage-1]:
                                    *m_pHTMLFiles[nSdPage-1],   aStrNavPrev);
    else
        aStr += aStrNavPrev;
    aStr.Append(sal_Unicode(' '));

    // zur naechsten Seite
    if(nSdPage < m_nSdPageCount-1)
        aStr += CreateLink( bIsText?*m_pTextFiles[nSdPage+1]:
                                    *m_pHTMLFiles[nSdPage+1], aStrNavNext);
    else
        aStr += aStrNavNext;
    aStr.Append(sal_Unicode(' '));

    // letzte Seite
    if(nSdPage < m_nSdPageCount-1)
        aStr += CreateLink( bIsText?*m_pTextFiles[m_nSdPageCount-1]:
                                    *m_pHTMLFiles[m_nSdPageCount-1],
                                    aStrNavLast );
    else
        aStr += aStrNavLast;
    aStr.Append(sal_Unicode(' '));

    // Indexseite
    if (m_bContentsPage)
    {
        aStr += CreateLink(m_aIndex, aStrNavContent);
        aStr.Append(sal_Unicode(' '));
    }

    // Text/Grafik
    if(m_bImpress)
    {
        aStr += CreateLink( bIsText?(m_bFrames?m_aFramePage:*m_pHTMLFiles[nSdPage]):
                                    *m_pTextFiles[nSdPage], aStrNavText);

    }

    aStr.AppendAscii( "</center><br>\r\n" );

    return aStr;
}

// ====================================================================
// Schaltflaechen aus der Gallery exportieren
// ====================================================================
bool HtmlExport::CreateBitmaps()
{
    ULONG nErr = 0;

    m_eEC.SetContext( STR_HTMLEXP_ERROR_GALLERY );

    if(m_nButtonThema != -1)
    {
        if( GalleryExplorer::BeginLocking( GALLERY_THEME_HTMLBUTTONS ) )
        {
            Graphic aButton;
            INT16 nButtons = m_nButtonThema * NUM_BUTTONS;
            for( INT16 nButton = 0; nButton < NUM_BUTTONS && nErr == 0; nButton++ )
            {
                if(!m_bFrames && (nButton == BTN_MORE || nButton == BTN_LESS))
                    continue;

                if(!m_bImpress && (nButton == BTN_TEXT || nButton == BTN_MORE || nButton == BTN_LESS ))
                    continue;

                nErr = CreateBitmap(GALLERY_THEME_HTMLBUTTONS, nButtons + nButton, GetButtonName(nButton));
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
ULONG HtmlExport::CreateBitmap( ULONG nThemeId, INT16 nImage, const String& aName ) const
{
    String aFull(m_aExportPath);
    aFull += aName;

    Graphic aGraphic;
    EasyFile aFile;
    String aJPG;
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
String HtmlExport::CreateBodyTag() const
{
    String aStr( RTL_CONSTASCII_USTRINGPARAM("<body") );

    if( m_bUserAttr || m_bDocColors )
    {
        Color aTextColor( m_aTextColor );
        if( (aTextColor == COL_AUTO) && (!m_aBackColor.IsDark()) )
            aTextColor = COL_BLACK;

        aStr.AppendAscii( " text=" );
        aStr += ColorToHTMLString( aTextColor );
        aStr.AppendAscii( " bgcolor=" );
        aStr += ColorToHTMLString( m_aBackColor );
        aStr.AppendAscii( " link=" );
        aStr += ColorToHTMLString( m_aLinkColor );
        aStr.AppendAscii( " vlink=" );
        aStr += ColorToHTMLString( m_aVLinkColor );
        aStr.AppendAscii( " alink=" );
        aStr += ColorToHTMLString( m_aALinkColor );
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
                                INT16 nWidth,
                                INT16 nHeight ) const
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
String HtmlExport::CreateHTMLCircleArea( ULONG nRadius,
                                         ULONG nCenterX,
                                         ULONG nCenterY,
                                         const String& rHRef ) const
{
    String aStr( RTL_CONSTASCII_USTRINGPARAM("<area shape=circle coords=\"" ));

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
String HtmlExport::CreateHTMLPolygonArea( const XPolyPolygon& rXPolyPoly,
                                          Size aShift,
                                          double fFactor,
                                          const String& rHRef ) const
{
    String          aStr;
    const USHORT    nNoOfXPolygons = rXPolyPoly.Count();

    for ( USHORT nXPoly= 0; nXPoly < nNoOfXPolygons; nXPoly++ )
    {
        const XPolygon& rXPoly = rXPolyPoly.GetObject(nXPoly);
        const USHORT    nNoOfPoints = rXPoly.GetPointCount();

        aStr.AppendAscii( "<area shape=polygon coords=\"" );

        for ( USHORT nPoint = 0; nPoint < nNoOfPoints; nPoint++ )
        {
            Point aPnt(rXPoly[nPoint]);
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
    String aStr( RTL_CONSTASCII_USTRINGPARAM("<area shape=rect coords=\"") );

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
String HtmlExport::CreatePageURL( USHORT nPgNum )
{
    if(m_bFrames)
    {
        String aUrl( RTL_CONSTASCII_USTRINGPARAM("JavaScript:parent.NavigateAbs("));
        aUrl += String::CreateFromInt32(nPgNum);
        aUrl.Append(sal_Unicode(')'));
        return aUrl;
    }
    else
        return *m_pHTMLFiles[nPgNum];
}

bool HtmlExport::CopyScript( const String& rPath, const String& rSource, const String& rDest, bool bUnix /* = false */ )
{
    INetURLObject   aURL( SvtPathOptions().GetConfigPath() );
    String      aScript;

    aURL.Append( String( RTL_CONSTASCII_USTRINGPARAM("webcast") ) );
    aURL.Append( rSource );

    m_eEC.SetContext( STR_HTMLEXP_ERROR_OPEN_FILE, rSource );

    ULONG       nErr;
    SvStream*   pIStm = ::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ );

    if( pIStm )
    {
        ByteString aLine;

        while( pIStm->ReadLine( aLine ) )
        {
            aScript.AppendAscii( aLine.GetBuffer() );
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

    aScript.SearchAndReplaceAll( String(RTL_CONSTASCII_USTRINGPARAM("$$3")), m_aCGIPath );

    aScript.SearchAndReplaceAll( String(RTL_CONSTASCII_USTRINGPARAM("$$4")), String::CreateFromInt32(m_nWidthPixel) );
    aScript.SearchAndReplaceAll( String(RTL_CONSTASCII_USTRINGPARAM("$$5")), String::CreateFromInt32(m_nHeightPixel) );


    String aDest( rPath );
    aDest += rDest;

    m_eEC.SetContext( STR_HTMLEXP_ERROR_CREATE_FILE, rDest );
    // write script file
    {
        EasyFile aFile;
        SvStream* pStr;
        nErr = aFile.createStream(aDest, pStr);
        if(nErr == 0)
        {
            ByteString aStr( aScript, RTL_TEXTENCODING_UTF8 );
            *pStr << aStr.GetBuffer();

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
bool HtmlExport::CreateASPScripts()
{
    for( USHORT n = 0; n < (sizeof( ASP_Scripts ) / sizeof(char *)); n++ )
    {
        String aScript;

        aScript.AssignAscii( ASP_Scripts[n] );
        if(!CopyScript(m_aExportPath, aScript, aScript))
            return false;
    }

    if(!CopyScript(m_aExportPath, String(RTL_CONSTASCII_USTRINGPARAM("edit.asp")), m_aIndex ))
        return false;

    return true;
}


char *PERL_Scripts[] = { "webcast.pl", "common.pl", "editpic.pl", "poll.pl", "savepic.pl", "show.pl" };

/** erzeugt und speichert die für WebShow benötigte PERL Scripte */
bool HtmlExport::CreatePERLScripts()
{
    for( USHORT n = 0; n < (sizeof( PERL_Scripts ) / sizeof(char *)); n++ )
    {
        String aScript;
        aScript.AssignAscii( PERL_Scripts[n] );
        if(!CopyScript(m_aExportPath, aScript, aScript, true))
            return false;
    }

    if(!CopyScript(m_aExportPath, String( RTL_CONSTASCII_USTRINGPARAM("edit.pl")), m_aIndex, true ))
        return false;

    if(!CopyScript(m_aExportPath, String( RTL_CONSTASCII_USTRINGPARAM("index.pl")), m_aIndexUrl, true ))
        return false;

    return true;
}

/** Erzeugt eine Liste mit den Namen der gespeicherten Images */
bool HtmlExport::CreateImageFileList()
{
    String aStr;
    for( USHORT nSdPage = 0; nSdPage < m_nSdPageCount; nSdPage++)
    {
        aStr += String::CreateFromInt32( nSdPage + 1 );
        aStr.Append(sal_Unicode(';'));
        aStr += m_aURLPath;
        aStr += *m_pImageFiles[nSdPage];
        aStr.AppendAscii( "\r\n" );
    }

    String aFileName( RTL_CONSTASCII_USTRINGPARAM("picture.txt") );
    bool bOk = WriteHtml( aFileName, false, aStr );

    if (mpProgress)
        mpProgress->SetState(++m_nPagesWritten);

    return bOk;
}

/** Erzeugt das File mit der aktuellen Seitennumer */
bool HtmlExport::CreateImageNumberFile()
{
    String aFull( m_aExportPath );
    String aFileName( RTL_CONSTASCII_USTRINGPARAM("currpic.txt") );
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

String HtmlExport::InsertSound( const String& rSoundFile )
{
    if( rSoundFile.Len() == 0 )
        return rSoundFile;

    String      aStr( RTL_CONSTASCII_USTRINGPARAM("<embed src=\"") );
    INetURLObject   aURL( rSoundFile );

    DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

    aStr += aURL.getName();
    aStr.AppendAscii( "\" hidden=\"true\" autostart=\"true\">" );

    CopyFile( rSoundFile, m_aExportPath );

    return aStr;
}

// =====================================================================

bool HtmlExport::CopyFile( const String& rSourceFile, const String& rDestPath )
{
    DirEntry aSourceEntry( rSourceFile );
    DirEntry aDestEntry( rDestPath );

    m_eEC.SetContext( STR_HTMLEXP_ERROR_COPY_FILE, aSourceEntry.GetName(), rDestPath );
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

                    aSet.Put(pPath->GetMergedItemSet());

                    // not hided yet, so hide it
                    pHSOI = new HideSpecialObjectsInfo( pPath );

                    // remember LineStyle and FillStyle state and value
                    pHSOI->mnLineStyleState = aSet.GetItemState( XATTR_FILLSTYLE, false );
                    if( pHSOI->mnLineStyleState == SFX_ITEM_SET )
                        pHSOI->meLineStyle = ( (const XLineStyleItem&) aSet.Get( XATTR_LINESTYLE ) ).GetValue();


                    pHSOI->mnFillStyleState = aSet.GetItemState( XATTR_LINESTYLE, false );
                    if( pHSOI->mnFillStyleState == SFX_ITEM_SET )
                        pHSOI->meFillStyle = ( (const XFillStyleItem&) aSet.Get( XATTR_FILLSTYLE ) ).GetValue();

                    // enter stealth mode
                    XFillStyleItem aFillStyleItem(XFILL_NONE);
                    aSet.Put(aFillStyleItem);

                    XLineStyleItem aLineStyleItem(XLINE_NONE);
                    aSet.Put(aLineStyleItem);

                    aSpecialObjects.Insert( (void*)pHSOI );

                    pPath->SetMergedItemSetAndBroadcast(aSet);
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
        aSet.Put(pPath->GetMergedItemSet());

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

        pPath->SetMergedItemSetAndBroadcast(aSet);

        delete pHSOI;

        pHSOI = (HideSpecialObjectsInfo*)aSpecialObjects.Next();
    }

    aSpecialObjects.Clear();
}

// =====================================================================

String HtmlExport::StringToURL( const String& rURL )
{
    return rURL;
/*
    return StringToHTMLString(rURL);
    OUString aURL( StringToHTMLString(rURL) );

    aURL = Uri::encode( aURL, rtl_UriCharClassUric, rtl_UriEncodeCheckEscapes, RTL_TEXTENCODING_UTF8);
    return String( aURL );
*/
}

String HtmlExport::GetButtonName( USHORT nButton ) const
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
ULONG EasyFile::createStream(  const String& rUrl, SvStream* &rpStr )
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
ULONG EasyFile::createFileName(  const String& rURL, String& rFileName )
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
            ::utl::LocalFileHelper::ConvertPhysicalNameToURL( rURL, aURLStr );
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
        return false;

    rCtxStr = String( SdResId( m_nResId ) );

    rCtxStr.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM("$(URL1)")), m_aURL1 );
    rCtxStr.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM("$(URL2)")), m_aURL2 );

    return true;
}

// =====================================================================

void HtmlErrorContext::SetContext( USHORT nResId )
{
    m_nResId = nResId;
    m_aURL1.Erase();
    m_aURL2.Erase();
}

// =====================================================================

void HtmlErrorContext::SetContext( USHORT nResId, const String& rURL )
{
    m_nResId = nResId;
    m_aURL1 = rURL;
    m_aURL2.Erase();
}

// =====================================================================

void HtmlErrorContext::SetContext( USHORT nResId, const String& rURL1, const String& rURL2 )
{
    m_nResId = nResId;
    m_aURL1 = rURL1;
    m_aURL2 = rURL2;
}

// =====================================================================
