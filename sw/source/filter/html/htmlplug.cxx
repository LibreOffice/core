/*************************************************************************
 *
 *  $RCSfile: htmlplug.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: mtg $ $Date: 2001-02-22 16:14:17 $
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

#ifdef PRECOMPILED
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#include "hintids.hxx"

#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#define _SVSTDARR_ULONGS
#include <svtools/svstdarr.hxx>
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _FRMHTML_HXX //autogen
#include <sfx2/frmhtml.hxx>
#endif
#ifndef _FRAMEOBJ_HXX //autogen
#include <sfx2/frameobj.hxx>
#endif
#ifndef _FRMHTMLW_HXX //autogen
#include <sfx2/frmhtmlw.hxx>
#endif
#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _APPLET_HXX //autogen
#include <so3/applet.hxx>
#endif
#ifndef _PLUGIN_HXX //autogen
#include <so3/plugin.hxx>
#endif
#ifndef _XOUTBMP_HXX //autogen
#include <svx/xoutbmp.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _HTMLTOKN_H
#include <svtools/htmltokn.h>
#endif

#ifndef _SW_APPLET_IMPL_HXX
#include <SwAppletImpl.hxx>
#endif

#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTSRND_HXX //autogen
#include <fmtsrnd.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif


#include "pam.hxx"
#include "doc.hxx"
#include "ndtxt.hxx"
#include "swerror.h"
#include "ndole.hxx"
#include "swtable.hxx"
#include "swhtml.hxx"
#include "wrthtml.hxx"
#include "htmlfly.hxx"
#include "swcss1.hxx"

#define HTML_DFLT_EMBED_WIDTH ((MM50*5)/2)
#define HTML_DFLT_EMBED_HEIGHT ((MM50*5)/2)

#define HTML_DFLT_APPLET_WIDTH ((MM50*5)/2)
#define HTML_DFLT_APPLET_HEIGHT ((MM50*5)/2)

const ULONG HTML_FRMOPTS_EMBED_ALL      =
    HTML_FRMOPT_ALT |
    HTML_FRMOPT_SIZE |
    HTML_FRMOPT_NAME;
const ULONG HTML_FRMOPTS_EMBED_CNTNR    =
    HTML_FRMOPTS_EMBED_ALL |
    HTML_FRMOPT_ABSSIZE;
const ULONG HTML_FRMOPTS_EMBED          =
    HTML_FRMOPTS_EMBED_ALL |
    HTML_FRMOPT_ALIGN |
    HTML_FRMOPT_SPACE |
    HTML_FRMOPT_BRCLEAR |
    HTML_FRMOPT_NAME;
const ULONG HTML_FRMOPTS_HIDDEN_EMBED   =
    HTML_FRMOPT_ALT |
    HTML_FRMOPT_NAME;

const ULONG HTML_FRMOPTS_APPLET_ALL     =
    HTML_FRMOPT_ALT |
    HTML_FRMOPT_SIZE;
const ULONG HTML_FRMOPTS_APPLET_CNTNR   =
    HTML_FRMOPTS_APPLET_ALL |
    HTML_FRMOPT_ABSSIZE;
const ULONG HTML_FRMOPTS_APPLET         =
    HTML_FRMOPTS_APPLET_ALL |
    HTML_FRMOPT_ALIGN |
    HTML_FRMOPT_SPACE |
    HTML_FRMOPT_BRCLEAR;

const ULONG HTML_FRMOPTS_IFRAME_ALL     =
    HTML_FRMOPT_ALT |
    HTML_FRMOPT_SIZE;
const ULONG HTML_FRMOPTS_IFRAME_CNTNR   =
    HTML_FRMOPTS_IFRAME_ALL |
    HTML_FRMOPT_ABSSIZE;
const ULONG HTML_FRMOPTS_IFRAME         =
    HTML_FRMOPTS_IFRAME_ALL |
    HTML_FRMOPT_ALIGN |
    HTML_FRMOPT_SPACE |
    HTML_FRMOPT_BORDER |
    HTML_FRMOPT_BRCLEAR;

const ULONG HTML_FRMOPTS_OLE_CSS1       =
    HTML_FRMOPT_S_ALIGN |
    HTML_FRMOPT_S_SPACE;

/*  */

void SwHTMLParser::SetFixSize( const Size& rPixSize,
                               const Size& rTwipDfltSize,
                               BOOL bPrcWidth, BOOL bPrcHeight,
                               SfxItemSet& rCSS1ItemSet,
                               SvxCSS1PropertyInfo& rCSS1PropInfo,
                               SfxItemSet& rFlyItemSet )
{
    // absolulte Groessenangaben in Twip umrechnen
    BYTE nPrcWidth = 0, nPrcHeight = 0;
    Size aTwipSz( bPrcWidth || USHRT_MAX==rPixSize.Width() ? 0 : rPixSize.Width(),
                  bPrcHeight || USHRT_MAX==rPixSize.Height() ? 0 : rPixSize.Height() );
    if( (aTwipSz.Width() || aTwipSz.Height()) && Application::GetDefaultDevice() )
    {
        aTwipSz =
            Application::GetDefaultDevice()->PixelToLogic( aTwipSz,
                                                MapMode(MAP_TWIP) );
    }

    // die Breite bearbeiten
    if( SVX_CSS1_LTYPE_PERCENTAGE == rCSS1PropInfo.eWidthType )
    {
        nPrcWidth = (BYTE)rCSS1PropInfo.nWidth;
        aTwipSz.Width() = rTwipDfltSize.Width();
    }
    else if( SVX_CSS1_LTYPE_TWIP== rCSS1PropInfo.eWidthType )
    {
        aTwipSz.Width() = rCSS1PropInfo.nWidth;
    }
    else if( bPrcWidth && rPixSize.Width() )
    {
        nPrcWidth = (BYTE)rPixSize.Width();
        if( nPrcWidth > 100 )
            nPrcWidth = 100;

        aTwipSz.Width() = rTwipDfltSize.Width();
    }
    else if( USHRT_MAX==rPixSize.Width() )
    {
        aTwipSz.Width() = rTwipDfltSize.Width();
    }
    if( aTwipSz.Width() < MINFLY )
    {
        aTwipSz.Width() = MINFLY;
    }

    // Hoehe bearbeiten
    if( SVX_CSS1_LTYPE_PERCENTAGE == rCSS1PropInfo.eHeightType )
    {
        nPrcHeight = (BYTE)rCSS1PropInfo.nHeight;
        aTwipSz.Height() = rTwipDfltSize.Height();
    }
    else if( SVX_CSS1_LTYPE_TWIP== rCSS1PropInfo.eHeightType )
    {
        aTwipSz.Height() = rCSS1PropInfo.nHeight;
    }
    else if( bPrcHeight && rPixSize.Height() )
    {
        nPrcHeight = (BYTE)rPixSize.Height();
        if( nPrcHeight > 100 )
            nPrcHeight = 100;

        aTwipSz.Height() = rTwipDfltSize.Height();
    }
    else if( USHRT_MAX==rPixSize.Height() )
    {
        aTwipSz.Height() = rTwipDfltSize.Height();
    }
    if( aTwipSz.Height() < MINFLY )
    {
        aTwipSz.Height() = MINFLY;
    }

    // Size setzen
    SwFmtFrmSize aFrmSize( ATT_FIX_SIZE, aTwipSz.Width(), aTwipSz.Height() );
    aFrmSize.SetWidthPercent( nPrcWidth );
    aFrmSize.SetHeightPercent( nPrcHeight );
    rFlyItemSet.Put( aFrmSize );
}

void SwHTMLParser::SetSpace( const Size& rPixSpace,
                             SfxItemSet& rCSS1ItemSet,
                             SvxCSS1PropertyInfo& rCSS1PropInfo,
                             SfxItemSet& rFlyItemSet )
{
    USHORT nLeftSpace = 0, nRightSpace = 0, nUpperSpace = 0, nLowerSpace = 0;
    if( (rPixSpace.Width() || rPixSpace.Height()) && Application::GetDefaultDevice() )
    {
        Size aTwipSpc( rPixSpace.Width(), rPixSpace.Height() );
        aTwipSpc =
            Application::GetDefaultDevice()->PixelToLogic( aTwipSpc,
                                                MapMode(MAP_TWIP) );
        nLeftSpace = nRightSpace = (USHORT)aTwipSpc.Width();
        nUpperSpace = nLowerSpace = (USHORT)aTwipSpc.Height();
    }

    // linken/rechten Rand setzen
    const SfxPoolItem *pItem;
    if( SFX_ITEM_SET==rCSS1ItemSet.GetItemState( RES_LR_SPACE, TRUE, &pItem ) )
    {
        // Ggf. den Erstzeilen-Einzug noch plaetten
        const SvxLRSpaceItem *pLRItem = (const SvxLRSpaceItem *)pItem;
        SvxLRSpaceItem aLRItem( *pLRItem );
        aLRItem.SetTxtFirstLineOfst( 0 );
        if( rCSS1PropInfo.bLeftMargin )
        {
            nLeftSpace = aLRItem.GetLeft();
            rCSS1PropInfo.bLeftMargin = FALSE;
        }
        if( rCSS1PropInfo.bRightMargin )
        {
            nRightSpace = aLRItem.GetRight();
            rCSS1PropInfo.bRightMargin = FALSE;
        }
        rCSS1ItemSet.ClearItem( RES_LR_SPACE );
    }
    if( nLeftSpace || nRightSpace )
    {
        SvxLRSpaceItem aLRItem;
        aLRItem.SetLeft( nLeftSpace );
        aLRItem.SetRight( nRightSpace );
        rFlyItemSet.Put( aLRItem );
        if( nLeftSpace )
        {
            const SwFmtHoriOrient& rHoriOri =
                (const SwFmtHoriOrient&)rFlyItemSet.Get( RES_HORI_ORIENT );
            if( HORI_NONE == rHoriOri.GetHoriOrient() )
            {
                SwFmtHoriOrient aHoriOri( rHoriOri );
                aHoriOri.SetPos( aHoriOri.GetPos() + nLeftSpace );
                rFlyItemSet.Put( aHoriOri );
            }
        }
    }

    // oberen/unteren Rand setzen
    if( SFX_ITEM_SET==rCSS1ItemSet.GetItemState( RES_UL_SPACE, TRUE, &pItem ) )
    {
        // Ggf. den Erstzeilen-Einzug noch plaetten
        const SvxULSpaceItem *pULItem = (const SvxULSpaceItem *)pItem;
        if( rCSS1PropInfo.bTopMargin )
        {
            nUpperSpace = pULItem->GetUpper();
            rCSS1PropInfo.bTopMargin = FALSE;
        }
        if( rCSS1PropInfo.bBottomMargin )
        {
            nLowerSpace = pULItem->GetLower();
            rCSS1PropInfo.bBottomMargin = FALSE;
        }
        rCSS1ItemSet.ClearItem( RES_UL_SPACE );
    }
    if( nUpperSpace || nLowerSpace )
    {
        SvxULSpaceItem aULItem;
        aULItem.SetUpper( nUpperSpace );
        aULItem.SetLower( nLowerSpace );
        rFlyItemSet.Put( aULItem );
        if( nUpperSpace )
        {
            const SwFmtVertOrient& rVertOri =
                (const SwFmtVertOrient&)rFlyItemSet.Get( RES_VERT_ORIENT );
            if( VERT_NONE == rVertOri.GetVertOrient() )
            {
                SwFmtVertOrient aVertOri( rVertOri );
                aVertOri.SetPos( aVertOri.GetPos() + nUpperSpace );
                rFlyItemSet.Put( aVertOri );
            }
        }
    }
}

/*  */

void SwHTMLParser::InsertEmbed()
{
    String aURL, aType, aName, aAlt, aId, aStyle, aClass;
    Size aSize( USHRT_MAX, USHRT_MAX );
    Size aSpace( USHRT_MAX, USHRT_MAX );
    BOOL bPrcWidth = FALSE, bPrcHeight = FALSE, bHidden = FALSE;
    SwVertOrient eVertOri = VERT_NONE;
    SwHoriOrient eHoriOri = HORI_NONE;
    SvCommandList aCmdLst;
    const HTMLOptions *pOptions = GetOptions();

    // Die Optionen werden vorwaerts gelesen, weil die Plugins sie in
    // dieser Reihenfolge erwarten. Trotzdem darf immer nur der erste
    // Wert einer Option beruecksichtigt werden.
    USHORT nArrLen = pOptions->Count();
    for( USHORT i=0; i<nArrLen; i++ )
    {
        const HTMLOption *pOption = (*pOptions)[i];
        switch( pOption->GetToken() )
        {
        case HTML_O_ID:
            aId = pOption->GetString();
            break;
        case HTML_O_STYLE:
            aStyle = pOption->GetString();
            break;
        case HTML_O_CLASS:
            aClass = pOption->GetString();
            break;
        case HTML_O_NAME:
            aName = pOption->GetString();
            break;
        case HTML_O_SRC:
            if( !aURL.Len() )
                aURL = pOption->GetString();
            break;
        case HTML_O_ALT:
            aAlt = pOption->GetString();
            break;
        case HTML_O_TYPE:
            if( !aType.Len() )
                aType = pOption->GetString();
            break;
        case HTML_O_ALIGN:
            if( eVertOri==VERT_NONE && eHoriOri==HORI_NONE )
            {
                eVertOri = (SwVertOrient)pOption->GetEnum( aHTMLImgVAlignTable, eVertOri );
                eHoriOri = (SwHoriOrient)pOption->GetEnum( aHTMLImgHAlignTable, eHoriOri );
            }
            break;
        case HTML_O_WIDTH:
            if( USHRT_MAX==aSize.Width() )
            {
                bPrcWidth = (pOption->GetString().Search('%') != STRING_NOTFOUND);
                aSize.Width() = (long)pOption->GetNumber();
            }
            break;
        case HTML_O_HEIGHT:
            if( USHRT_MAX==aSize.Height() )
            {
                bPrcHeight = (pOption->GetString().Search('%') != STRING_NOTFOUND);
                aSize.Height() = (long)pOption->GetNumber();
            }
            break;
        case HTML_O_HSPACE:
            if( USHRT_MAX==aSpace.Width() )
                aSpace.Width() = (long)pOption->GetNumber();
            break;
        case HTML_O_VSPACE:
            if( USHRT_MAX==aSpace.Height() )
                aSpace.Height() = (long)pOption->GetNumber();
            break;
        case HTML_O_UNKNOWN:
            if( pOption->GetTokenString().EqualsIgnoreCaseAscii( sHTML_O_hidden ) )
                bHidden =
                    !pOption->GetString().EqualsIgnoreCaseAscii( sHTML_HIDDEN_false );
            break;
        }

        // Es werden alle Parameter an das Plugin weitergereicht
        aCmdLst.Append( pOption->GetTokenString(), pOption->GetString() );
    }

    SfxItemSet aItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
    SvxCSS1PropertyInfo aPropInfo;
    if( HasStyleOptions( aStyle, aId, aClass ) )
        ParseStyleOptions( aStyle, aId, aClass, aItemSet, aPropInfo );

    // Die Default-Werte umsetzen (ausser Hoehe/Breite, das macht schon
    // SetFrmSize() fuer uns)
    if( eVertOri==VERT_NONE && eHoriOri==HORI_NONE )
        eVertOri = VERT_TOP;
    if( USHRT_MAX==aSpace.Width() )
        aSpace.Width() = 0;
    if( USHRT_MAX==aSpace.Height() )
        aSpace.Height() = 0;
    if( bHidden )
    {
        // Size (0,0) wird in SetFrmSize auf (MINFLY, MINFLY) umgebogen
        aSize.Width() = 0; aSize.Height() = 0;
        aSpace.Width() = 0; aSpace.Height() = 0;
        bPrcWidth = bPrcHeight = FALSE;
    }

    // die URL aufbereiten
    INetURLObject aURLObj;
    if( aURL.Len() && !aURLObj.SetURL( INetURLObject::RelToAbs(aURL) ) )
        return;

    // das Plugin anlegen
    SvStorageRef pStor = new SvStorage( aEmptyStr, STREAM_STD_READWRITE);
    SvFactory *pPlugInFactory = SvFactory::GetDefaultPlugInFactory();
    SvPlugInObjectRef pPlugin =
        &pPlugInFactory->CreateAndInit( *pPlugInFactory, pStor );

    pPlugin->EnableSetModified( FALSE );
    pPlugin->SetPlugInMode( (USHORT)PLUGIN_EMBEDED );
    pPlugin->SetURL( aURLObj );
    pPlugin->SetMimeType( aType );
    pPlugin->SetCommandList( aCmdLst );
    pPlugin->EnableSetModified( TRUE );

    SfxItemSet aFrmSet( pDoc->GetAttrPool(),
                        RES_FRMATR_BEGIN, RES_FRMATR_END-1 );
    if( !IsNewDoc() )
        Reader::ResetFrmFmtAttrs( aFrmSet );

    // den Anker setzen
    if( !bHidden )
    {
        SetAnchorAndAdjustment( eVertOri, eHoriOri, aItemSet, aPropInfo, aFrmSet );
    }
    else
    {
        SwFmtAnchor aAnchor( FLY_AT_CNTNT );
        aAnchor.SetAnchor( pPam->GetPoint() );
        aFrmSet.Put( aAnchor );
        aFrmSet.Put( SwFmtHoriOrient( 0, HORI_LEFT, FRAME) );
        aFrmSet.Put( SwFmtSurround( SURROUND_THROUGHT ) );
        aFrmSet.Put( SwFmtVertOrient( 0, VERT_TOP, PRTAREA ) );
    }

    // und noch die Groesse des Rahmens
    Size aDfltSz( HTML_DFLT_EMBED_WIDTH, HTML_DFLT_EMBED_HEIGHT );
    SetFixSize( aSize, aDfltSz, bPrcWidth, bPrcHeight, aItemSet, aPropInfo,
                aFrmSet );
    SetSpace( aSpace, aItemSet, aPropInfo, aFrmSet );

    // und in das Dok einfuegen
    SwFrmFmt* pFlyFmt = pDoc->Insert( *pPam, pPlugin, &aFrmSet );

    // Namen am FrmFmt setzen
    if( aName.Len() )
        pFlyFmt->SetName( aName );

    // den alternativen Text setzen
    SwNoTxtNode *pNoTxtNd =
        pDoc->GetNodes()[ pFlyFmt->GetCntnt().GetCntntIdx()
                          ->GetIndex()+1 ]->GetNoTxtNode();
    pNoTxtNd->SetAlternateText( aAlt );

    // Ggf Frames anlegen und auto-geb. Rahmen registrieren
    if( !bHidden )
    {
        // HIDDEN-Plugins sollen absatzgebunden bleiben. Da RegisterFlyFrm
        // absatzgebundene Rahmen mit DUrchlauf in am Zeichen gebundene
        // Rahmen umwandelt, muessen die Frames hier von Hand angelegt werden.
        RegisterFlyFrm( pFlyFmt );
    }
}

/*  */

#if 0
class SwHTMLApplet_Impl
{
    SvAppletObjectRef xApplet;      // das aktuelle Applet
    SvCommandList     aCommandList; // und die szugehorige Command-List
    SfxItemSet        aItemSet;
    String            sAlt;

public:

    SwHTMLApplet_Impl( SfxItemPool& rPool, USHORT nWhich1, USHORT nWhich2 ) :
        aItemSet( rPool, nWhich1, nWhich2 )
    {
    }

    ~SwHTMLApplet_Impl()
    {
        xApplet.Clear();
    }

#ifndef DEBUG
inline
#endif
    void CreateApplet( const String& rCode, const String& rName,
                       BOOL bMayScript, const String& rCodeBase,
                       const String& rAlt );

    void FinishApplet()
    {
        xApplet->SetCommandList( aCommandList );
        xApplet->EnableSetModified( TRUE );
    }


    void AppendParam( const String& rName, const String& rValue )
    {
        aCommandList.Append( rName, rValue );
    }


    SvAppletObject* GetApplet() { return &xApplet; }
    SfxItemSet& GetItemSet() { return aItemSet; }
    const String& GetAltText() { return sAlt; }
};

#ifndef DEBUG
inline
#endif
void SwHTMLApplet_Impl::CreateApplet( const String& rCode, const String& rName,
                                      BOOL bMayScript, const String& rCodeBase,
                                      const String& rAlt )
{
    SvStorageRef pStor = new SvStorage( aEmptyStr, STREAM_STD_READWRITE );
    xApplet = new SvAppletObject();
    xApplet->DoInitNew( pStor );

    xApplet->EnableSetModified( FALSE );
    xApplet->SetClass( rCode );
    xApplet->SetName( rName );
    xApplet->SetMayScript( bMayScript );
    xApplet->SetDocBase( INetURLObject::GetBaseURL() );

    String sCodeBase;
    if( rCodeBase.Len() )
    {
        INetURLObject aTmpURL;

        INetProtocol eProt = aTmpURL.CompareProtocolScheme( rCodeBase );
        if( eProt==INET_PROT_NOT_VALID &&
            rCodeBase.Search( ':' ) != STRING_NOTFOUND  )
        {
            // The codebase contains an unknown protocol rather than
            // a relative URL.
            sCodeBase = rCodeBase;
        }
        else
        {
            sCodeBase = INetURLObject::RelToAbs( rCodeBase );
        }
    }
    else
    {
        INetURLObject aTmpURL( INetURLObject::GetBaseURL() );
        sCodeBase = aTmpURL.GetPartBeforeLastName();
    }

    xApplet->SetCodeBase( sCodeBase );

    sAlt = rAlt;
}

#endif

void SwHTMLParser::InsertApplet()
{
#ifdef SOLAR_JAVA
    String aCodeBase, aCode, aName, aAlt, aId, aStyle, aClass;
    Size aSize( USHRT_MAX, USHRT_MAX );
    Size aSpace( 0, 0 );
    BOOL bPrcWidth = FALSE, bPrcHeight = FALSE, bMayScript = FALSE;
    SwVertOrient eVertOri = VERT_TOP;
    SwHoriOrient eHoriOri = HORI_NONE;

    // Eine neue Command-List anlegen
    if( pAppletImpl )
        delete pAppletImpl;
    pAppletImpl = new SwApplet_Impl( pDoc->GetAttrPool(), RES_FRMATR_BEGIN, RES_FRMATR_END-1 );

    const HTMLOptions *pOptions = GetOptions();
    for( USHORT i = pOptions->Count(); i; )
    {
        const HTMLOption *pOption = (*pOptions)[--i];
        switch( pOption->GetToken() )
        {
        case HTML_O_ID:
            aId = pOption->GetString();
            break;
        case HTML_O_STYLE:
            aStyle = pOption->GetString();
            break;
        case HTML_O_CLASS:
            aClass = pOption->GetString();
            break;
        case HTML_O_CODEBASE:
            aCodeBase = pOption->GetString();
            break;
        case HTML_O_CODE:
            aCode = pOption->GetString();
            break;
        case HTML_O_NAME:
            aName = pOption->GetString();
            break;
        case HTML_O_ALT:
            aAlt = pOption->GetString();
            break;
        case HTML_O_ALIGN:
            eVertOri = (SwVertOrient)pOption->GetEnum( aHTMLImgVAlignTable, eVertOri );
            eHoriOri = (SwHoriOrient)pOption->GetEnum( aHTMLImgHAlignTable, eHoriOri );
            break;
        case HTML_O_WIDTH:
            bPrcWidth = (pOption->GetString().Search('%') != STRING_NOTFOUND);
            aSize.Width() = (long)pOption->GetNumber();
            break;
        case HTML_O_HEIGHT:
            bPrcHeight = (pOption->GetString().Search('%') != STRING_NOTFOUND);
            aSize.Height() = (long)pOption->GetNumber();
            break;
        case HTML_O_HSPACE:
            aSpace.Width() = (long)pOption->GetNumber();
            break;
        case HTML_O_VSPACE:
            aSpace.Height() = (long)pOption->GetNumber();
            break;
        case HTML_O_MAYSCRIPT:
            bMayScript = TRUE;
            break;
        }

        // Es werden alle Parameter auch an das Applet weitergereicht
        pAppletImpl->AppendParam( pOption->GetTokenString(),
                                  pOption->GetString() );
    }

    if( !aCode.Len() )
    {
        delete pAppletImpl;
        pAppletImpl = 0;
        return;
    }

    pAppletImpl->CreateApplet( aCode, aName, bMayScript, aCodeBase);//, aAlt );
    pAppletImpl->SetAltText( aAlt );

    SfxItemSet aItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
    SvxCSS1PropertyInfo aPropInfo;
    if( HasStyleOptions( aStyle, aId, aClass ) )
        ParseStyleOptions( aStyle, aId, aClass, aItemSet, aPropInfo );

    SfxItemSet& rFrmSet = pAppletImpl->GetItemSet();
    if( !IsNewDoc() )
        Reader::ResetFrmFmtAttrs( rFrmSet );

    // den Anker und die Ausrichtung setzen
    SetAnchorAndAdjustment( eVertOri, eHoriOri, aItemSet, aPropInfo, rFrmSet );

    // und noch die Groesse des Rahmens
    Size aDfltSz( HTML_DFLT_APPLET_WIDTH, HTML_DFLT_APPLET_HEIGHT );
    SetFixSize( aSize, aDfltSz, bPrcWidth, bPrcHeight, aItemSet, aPropInfo,
                rFrmSet );
    SetSpace( aSpace, aItemSet, aPropInfo, rFrmSet );
#endif
}

void SwHTMLParser::EndApplet()
{
#ifdef SOLAR_JAVA
    if( !pAppletImpl )
        return;

    pAppletImpl->FinishApplet();

    // und in das Dok einfuegen
    SwFrmFmt* pFlyFmt = pDoc->Insert( *pPam, pAppletImpl->GetApplet(),
                                      &pAppletImpl->GetItemSet() );

    // den alternativen Namen setzen
    SwNoTxtNode *pNoTxtNd =
        pDoc->GetNodes()[ pFlyFmt->GetCntnt().GetCntntIdx()
                          ->GetIndex()+1 ]->GetNoTxtNode();
    pNoTxtNd->SetAlternateText( pAppletImpl->GetAltText() );

    // Ggf Frames anlegen und auto-geb. Rahmen registrieren
    RegisterFlyFrm( pFlyFmt );

    delete pAppletImpl;
    pAppletImpl = 0;
#endif
}

void SwHTMLParser::InsertParam()
{
#ifdef SOLAR_JAVA
    if( !pAppletImpl )
        return;

    String aName, aValue;

    const HTMLOptions *pOptions = GetOptions();
    for( USHORT i = pOptions->Count(); i; )
    {
        const HTMLOption *pOption = (*pOptions)[--i];
        switch( pOption->GetToken() )
        {
        case HTML_O_NAME:
            aName = pOption->GetString();
            break;
        case HTML_O_VALUE:
            aValue = pOption->GetString();
            break;
        }
    }

    if( !aName.Len() )
        return;

    pAppletImpl->AppendParam( aName, aValue );
#endif
}


/*  */

void SwHTMLParser::InsertFloatingFrame()
{
    String aAlt, aId, aStyle, aClass;
    Size aSize( USHRT_MAX, USHRT_MAX );
    Size aSpace( 0, 0 );
    BOOL bPrcWidth = FALSE, bPrcHeight = FALSE;
    SwVertOrient eVertOri = VERT_TOP;
    SwHoriOrient eHoriOri = HORI_NONE;

    const HTMLOptions *pOptions = GetOptions();

    // Erstmal die Optionen fr das Writer-Frame-Format holen
    USHORT nArrLen = pOptions->Count();
    for ( USHORT i=0; i<nArrLen; i++ )
    {
        const HTMLOption *pOption = (*pOptions)[i];
        switch( pOption->GetToken() )
        {
        case HTML_O_ID:
            aId = pOption->GetString();
            break;
        case HTML_O_STYLE:
            aStyle = pOption->GetString();
            break;
        case HTML_O_CLASS:
            aClass = pOption->GetString();
            break;
        case HTML_O_ALT:
            aAlt = pOption->GetString();
            break;
        case HTML_O_ALIGN:
            eVertOri = (SwVertOrient)pOption->GetEnum( aHTMLImgVAlignTable, eVertOri );
            eHoriOri = (SwHoriOrient)pOption->GetEnum( aHTMLImgHAlignTable, eHoriOri );
            break;
        case HTML_O_WIDTH:
            bPrcWidth = (pOption->GetString().Search('%') != STRING_NOTFOUND);
            aSize.Width() = (long)pOption->GetNumber();
            break;
        case HTML_O_HEIGHT:
            bPrcHeight = (pOption->GetString().Search('%') != STRING_NOTFOUND);
            aSize.Height() = (long)pOption->GetNumber();
            break;
        case HTML_O_HSPACE:
            aSpace.Width() = (long)pOption->GetNumber();
            break;
        case HTML_O_VSPACE:
            aSpace.Height() = (long)pOption->GetNumber();
            break;
        }
    }

    // und jetzt die fuer den SfxFrame
    SfxFrameDescriptor *pFrameDesc = new SfxFrameDescriptor( 0 );

    SfxFrameHTMLParser::ParseFrameOptions( pFrameDesc, pOptions );

    // den Floating-Frame anlegen
    SvStorageRef pStor = new SvStorage( aEmptyStr, STREAM_STD_READWRITE );
    SfxFrameObjectRef pFrame = new SfxFrameObject();
    pFrame->DoInitNew( pStor );

    pFrame->EnableSetModified( FALSE );
    pFrame->SetFrameDescriptor( pFrameDesc );
    pFrame->EnableSetModified( TRUE );

    SfxItemSet aItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
    SvxCSS1PropertyInfo aPropInfo;
    if( HasStyleOptions( aStyle, aId, aClass ) )
        ParseStyleOptions( aStyle, aId, aClass, aItemSet, aPropInfo );

    // den Itemset holen
    SfxItemSet aFrmSet( pDoc->GetAttrPool(),
                        RES_FRMATR_BEGIN, RES_FRMATR_END-1 );
    if( !IsNewDoc() )
        Reader::ResetFrmFmtAttrs( aFrmSet );

    // den Anker und die Ausrichtung setzen
    SetAnchorAndAdjustment( eVertOri, eHoriOri, aItemSet, aPropInfo, aFrmSet );

    // und noch die Groesse des Rahmens
    Size aDfltSz( HTML_DFLT_APPLET_WIDTH, HTML_DFLT_APPLET_HEIGHT );
    SetFixSize( aSize, aDfltSz, bPrcWidth, bPrcHeight, aItemSet, aPropInfo,
                aFrmSet );
    SetSpace( aSpace, aItemSet, aPropInfo, aFrmSet );

    // und in das Dok einfuegen
    SwFrmFmt* pFlyFmt = pDoc->Insert( *pPam, pFrame, &aFrmSet );

    // den alternativen Namen setzen
    SwNoTxtNode *pNoTxtNd =
        pDoc->GetNodes()[ pFlyFmt->GetCntnt().GetCntntIdx()
                          ->GetIndex()+1 ]->GetNoTxtNode();
    pNoTxtNd->SetAlternateText( aAlt );

    // Ggf Frames anlegen und auto-geb. Rahmen registrieren
    RegisterFlyFrm( pFlyFmt );

    bInFloatingFrame = TRUE;
}

/*  */

/*
#define SWHTML_OPTTYPE_IGNORE 0
#define SWHTML_OPTTYPE_TAG 1
#define SWHTML_OPTTYPE_PARAM 2


static USHORT GetOptionType( const String& rName, BOOL bApplet )
{
    USHORT nType = bApplet ? SWHTML_OPTTYPE_PARAM : SWHTML_OPTTYPE_TAG;

    switch( rName.GetChar(0) )
    {
    case 'A':
    case 'a':
        if( rName.EqualsIgnoreCaseAscii( sHTML_O_align ) ||
            rName.EqualsIgnoreCaseAscii( sHTML_O_alt ) )
            nType = SWHTML_OPTTYPE_IGNORE;
        else if( bApplet &&
                 (rName.EqualsIgnoreCaseAscii( sHTML_O_archive ) ||
                 rName.EqualsIgnoreCaseAscii( sHTML_O_archives )) )
            nType = SWHTML_OPTTYPE_TAG;
        break;
    case 'C':
    case 'c':
        if( rName.EqualsIgnoreCaseAscii( sHTML_O_class ) ||
            (bApplet && (rName.EqualsIgnoreCaseAscii( sHTML_O_code ) ||
                         rName.EqualsIgnoreCaseAscii( sHTML_O_codebase ))) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'H':
    case 'h':
        if( rName.EqualsIgnoreCaseAscii( sHTML_O_height ) ||
            rName.EqualsIgnoreCaseAscii( sHTML_O_hspace ) ||
            (!bApplet && rName.EqualsIgnoreCaseAscii( sHTML_O_hidden )) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'I':
    case 'i':
        if( rName.EqualsIgnoreCaseAscii( sHTML_O_id ) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'M':
    case 'm':
        if( bApplet && rName.EqualsIgnoreCaseAscii( sHTML_O_mayscript ) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'N':
    case 'n':
        if( rName.EqualsIgnoreCaseAscii( sHTML_O_name ) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'O':
    case 'o':
        if( bApplet && rName.EqualsIgnoreCaseAscii( sHTML_O_object ) )
            nType = SWHTML_OPTTYPE_TAG;
        break;
    case 'S':
    case 's':
        if( rName.EqualsIgnoreCaseAscii( sHTML_O_style ) ||
            (!bApplet && rName.EqualsIgnoreCaseAscii( sHTML_O_src )) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'T':
    case 't':
        if( !bApplet && rName.EqualsIgnoreCaseAscii( sHTML_O_type ) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'V':
    case 'v':
        if( rName.EqualsIgnoreCaseAscii( sHTML_O_vspace ) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'W':
    case 'w':
        if( rName.EqualsIgnoreCaseAscii( sHTML_O_width ) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    }

    return nType;
}
*/

USHORT SwHTMLWriter::GuessOLENodeFrmType( const SwNode& rNode )
{
    SwOLEObj& rObj = ((SwOLENode*)rNode.GetOLENode())->GetOLEObj();

    SwHTMLFrmType eType = HTML_FRMTYPE_OLE;

    SvPlugInObjectRef pPlugin( rObj.GetOleRef() );
    if( pPlugin.Is() )
    {
        eType = HTML_FRMTYPE_PLUGIN;
    }
    else
    {
        SfxFrameObjectRef pFrame( rObj.GetOleRef() );
        if( pFrame.Is() )
        {
            eType = HTML_FRMTYPE_IFRAME;
        }
#ifdef SOLAR_JAVA
        else
        {
            SvAppletObjectRef pApplet( rObj.GetOleRef() );
            if( pApplet.Is() )
                eType = HTML_FRMTYPE_APPLET;
        }
#endif
    }

    return eType;
}

Writer& OutHTML_FrmFmtOLENode( Writer& rWrt, const SwFrmFmt& rFrmFmt,
                               BOOL bInCntnr )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    const SwFmtCntnt& rFlyCntnt = rFrmFmt.GetCntnt();
    ULONG nStt = rFlyCntnt.GetCntntIdx()->GetIndex()+1;
    SwOLENode *pOLENd = rHTMLWrt.pDoc->GetNodes()[ nStt ]->GetOLENode();

    ASSERT( pOLENd, "OLE-Node erwartet" );
    if( !pOLENd )
        return rWrt;

    SwOLEObj &rObj = pOLENd->GetOLEObj();
    SvPlugInObjectRef pPlugin( rObj.GetOleRef() );
#ifdef SOLAR_JAVA
    SvAppletObjectRef pApplet( rObj.GetOleRef() );
#endif
    SfxFrameObjectRef pFrame( rObj.GetOleRef() );
    BOOL bHiddenEmbed = FALSE;

    ASSERT( !pFrame.Is() || rHTMLWrt.IsHTMLMode(HTMLMODE_FLOAT_FRAME),
            "Floating-Frame wird exportiert, aber Modus ist nicht aktiviert" );

    if( !pPlugin.Is() &&
#ifdef SOLAR_JAVA
        !pApplet.Is() &&
#endif
        !pFrame.Is() )
    {
        ASSERT( pPlugin.Is(), "unbekanntes Inplace-Object" );
        return rWrt;
    }

    ByteString aEndTags;
    ULONG nFrmOpts;

    // wenn meoglich vor dem "Objekt" einen Zeilen-Umbruch ausgeben
    if( rHTMLWrt.bLFPossible )
        rHTMLWrt.OutNewLine( TRUE );

    if( rFrmFmt.GetName().Len() )
        rHTMLWrt.OutImplicitMark( rFrmFmt.GetName(),
                                  pMarkToOLE );

    ByteString sOut('<');
    if( pPlugin.Is() )
    {
        // erstmal das Plug-spezifische
        sOut += sHTML_embed;

        String aURL(
            INetURLObject::AbsToRel(pPlugin->GetURL()->GetMainURL(),
                                    INetURLObject::WAS_ENCODED,
                                    INetURLObject::DECODE_UNAMBIGUOUS) );

        if( aURL.Len() )
        {
            ((sOut += ' ') += sHTML_O_src) += "=\"";
            rWrt.Strm() << sOut.GetBuffer();
            HTMLOutFuncs::Out_String( rWrt.Strm(), aURL, rHTMLWrt.eDestEnc );
            sOut = '\"';
        }

        const String& aType = pPlugin->GetMimeType();
        if( aType.Len() )
        {
            ((sOut += ' ') += sHTML_O_type) += "=\"";
            rWrt.Strm() << sOut.GetBuffer();
            HTMLOutFuncs::Out_String( rWrt.Strm(), aType, rHTMLWrt.eDestEnc );
            sOut = '\"';
        }

        if( FLY_AT_CNTNT == rFrmFmt.GetAnchor().GetAnchorId() &&
            SURROUND_THROUGHT == rFrmFmt.GetSurround().GetSurround() )
        {
            // Das Plugin ist HIDDEN
            (sOut += ' ') += sHTML_O_hidden;
            nFrmOpts = HTML_FRMOPTS_HIDDEN_EMBED;
            bHiddenEmbed = TRUE;
        }
        else
        {
            nFrmOpts = bInCntnr ? HTML_FRMOPTS_EMBED_CNTNR
                                : HTML_FRMOPTS_EMBED;
        }
    }
#ifdef SOLAR_JAVA
    else if( pApplet.Is() )
    {
        // oder das Applet-Spezifische

        sOut += sHTML_applet;

        // CODEBASE
        const XubString& rURL = pApplet->GetCodeBase();
        if( rURL.Len() )
        {
#ifdef DEBUG
            String sTmp( INetURLObject::GetBaseURL() );
#endif
            String sCodeBase( INetURLObject::AbsToRel(rURL,
                                    INetURLObject::WAS_ENCODED,
                                    INetURLObject::DECODE_UNAMBIGUOUS) );
            if( sCodeBase.Len() )
            {
                ((sOut += ' ') += sHTML_O_codebase) += "=\"";
                rWrt.Strm() << sOut.GetBuffer();
                HTMLOutFuncs::Out_String( rWrt.Strm(), sCodeBase, rHTMLWrt.eDestEnc );
                sOut = '\"';
            }
        }

        // CODE
        ((sOut += ' ') += sHTML_O_code) += "=\"";
        rWrt.Strm() << sOut.GetBuffer();
        HTMLOutFuncs::Out_String( rWrt.Strm(), pApplet->GetClass(), rHTMLWrt.eDestEnc );
        sOut = '\"';

        // NAME
        const String &rName = pApplet->GetName();
        if( rName.Len() )
        {
            ((sOut += ' ') += sHTML_O_name) += "=\"";
            rWrt.Strm() << sOut.GetBuffer();
            HTMLOutFuncs::Out_String( rWrt.Strm(), rName, rHTMLWrt.eDestEnc );
            sOut = '\"';
        }

        if( pApplet->IsMayScript() )
            (sOut += ' ') += sHTML_O_mayscript;

        nFrmOpts = bInCntnr ? HTML_FRMOPTS_APPLET_CNTNR
                            : HTML_FRMOPTS_APPLET;
    }
#endif
    else
    {
        // oder das Flating-Frame spezifische

        sOut += sHTML_iframe;
        rWrt.Strm() << sOut.GetBuffer();

        SfxFrameHTMLWriter::Out_FrameDescriptor( rWrt.Strm(),
                                        pFrame->GetFrameDescriptor(),
                                        FALSE, 0, rHTMLWrt.eDestEnc  );
        sOut.Erase();

        nFrmOpts = bInCntnr ? HTML_FRMOPTS_IFRAME_CNTNR
                            : HTML_FRMOPTS_IFRAME;
    }

    rWrt.Strm() << sOut.GetBuffer();

    // ALT, WIDTH, HEIGHT, HSPACE, VSPACE, ALIGN
    if( rHTMLWrt.IsHTMLMode( HTMLMODE_ABS_POS_FLY ) && !bHiddenEmbed )
        nFrmOpts |= HTML_FRMOPTS_OLE_CSS1;
    rHTMLWrt.OutFrmFmtOptions( rFrmFmt, pOLENd->GetAlternateText(),
                               aEndTags, nFrmOpts );
    if( rHTMLWrt.IsHTMLMode( HTMLMODE_ABS_POS_FLY ) && !bHiddenEmbed )
        rHTMLWrt.OutCSS1_FrmFmtOptions( rFrmFmt, nFrmOpts );

#ifdef SOLAR_JAVA
    if( pApplet.Is() )
    {
        // fuer Applets die Parameter als eigene Tags ausgeben
        // und ein </APPLET> schreiben

        const SvCommandList& rCommands = pApplet->GetCommandList();
        SvULongs aParams;
        ULONG i = rCommands.Count();
        while( i > 0 )
        {
            const SvCommand& rCommand = rCommands[ --i ];
            const String& rName = rCommand.GetCommand();
            USHORT nType = SwApplet_Impl::GetOptionType( rName, TRUE );
            if( SWHTML_OPTTYPE_TAG == nType )
            {
                const String& rValue = rCommand.GetArgument();
                rWrt.Strm() << ' ';
                HTMLOutFuncs::Out_String( rWrt.Strm(), rName, rHTMLWrt.eDestEnc );
                rWrt.Strm() << "=\"";
                HTMLOutFuncs::Out_String( rWrt.Strm(), rValue, rHTMLWrt.eDestEnc ) << '\"';
            }
            else if( SWHTML_OPTTYPE_PARAM == nType )
            {
                aParams.Insert( i, aParams.Count() );
            }
        }

        rHTMLWrt.Strm() << '>';

        rHTMLWrt.IncIndentLevel(); // Inhalt von Applet einruecken

        USHORT ii = aParams.Count();
        while( ii > 0  )
        {
            const SvCommand& rCommand = rCommands[ aParams[--ii] ];
            const String& rName = rCommand.GetCommand();
            const String& rValue = rCommand.GetArgument();
            rHTMLWrt.OutNewLine();
            ((((sOut = '<') += sHTML_param) += ' ') += sHTML_O_name)
                += "=\"";
            rWrt.Strm() << sOut.GetBuffer();
            HTMLOutFuncs::Out_String( rWrt.Strm(), rName, rHTMLWrt.eDestEnc );
            ((sOut = "\" ") += sHTML_O_value) += "=\"";
            rWrt.Strm() << sOut.GetBuffer();
            HTMLOutFuncs::Out_String( rWrt.Strm(), rValue, rHTMLWrt.eDestEnc ) << "\">";
        }

        rHTMLWrt.DecIndentLevel(); // Inhalt von Applet einruecken
        if( rCommands.Count() )
            rHTMLWrt.OutNewLine();
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sHTML_applet, FALSE );
    }
    else
#endif
    if( pPlugin.Is() )
    {
        // fuer Plugins die Paramater als Optionen schreiben

        const SvCommandList& rCommands = pPlugin->GetCommandList();
        for( ULONG i=0; i<rCommands.Count(); i++ )
        {
            const SvCommand& rCommand = rCommands[ i ];
            const String& rName = rCommand.GetCommand();

            if( SwApplet_Impl::GetOptionType( rName, FALSE ) == SWHTML_OPTTYPE_TAG )
            {
                const String& rValue = rCommand.GetArgument();
                rWrt.Strm() << ' ';
                HTMLOutFuncs::Out_String( rWrt.Strm(), rName, rHTMLWrt.eDestEnc );
                rWrt.Strm() << "=\"";
                HTMLOutFuncs::Out_String( rWrt.Strm(), rValue, rHTMLWrt.eDestEnc ) << '\"';
            }
        }
        rHTMLWrt.Strm() << '>';
    }
    else
    {
        // und fuer Floating-Frames einfach noch ein </IFRAME>
        // ausgeben

        rHTMLWrt.Strm() << '>';
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sHTML_iframe, FALSE );
    }

    if( aEndTags.Len() )
        rWrt.Strm() << aEndTags.GetBuffer();

    return rWrt;
}

Writer& OutHTML_FrmFmtOLENodeGrf( Writer& rWrt, const SwFrmFmt& rFrmFmt,
                                  BOOL bInCntnr )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    const SwFmtCntnt& rFlyCntnt = rFrmFmt.GetCntnt();
    ULONG nStt = rFlyCntnt.GetCntntIdx()->GetIndex()+1;
    SwOLENode *pOLENd = rHTMLWrt.pDoc->GetNodes()[ nStt ]->GetOLENode();

    ASSERT( pOLENd, "OLE-Node erwartet" );
    if( !pOLENd )
        return rWrt;

    // Inhalt des Nodes als Grafik speichern
    SvInPlaceObjectRef xRef = pOLENd->GetOLEObj().GetOleRef();;
    GDIMetaFile* pPic = NULL;
    SvData aData( FORMAT_GDIMETAFILE );
    if( xRef->GetData( &aData ) )
        aData.GetData( &pPic, TRANSFER_REFERENCE );
    if( pPic )
    {
        Graphic aGrf( *pPic );
        String aGrfNm;
        const String* pTempFileName = rHTMLWrt.GetOrigFileName();
        if(pTempFileName)
            aGrfNm = *pTempFileName;

        USHORT nErr = XOutBitmap::WriteGraphic( aGrf, aGrfNm,
                                    String::CreateFromAscii("JPG"),
                                    (XOUTBMP_USE_GIF_IF_POSSIBLE |
                                     XOUTBMP_USE_NATIVE_IF_POSSIBLE) );
        if( nErr )              // fehlerhaft, da ist nichts auszugeben
        {
            rHTMLWrt.nWarn = WARN_SWG_POOR_LOAD | WARN_SW_WRITE_BASE;
            return rWrt;
        }
        aGrfNm = URIHelper::SmartRelToAbs( aGrfNm );
        ULONG nFlags = bInCntnr ? HTML_FRMOPTS_GENIMG_CNTNR
                                  : HTML_FRMOPTS_GENIMG;
        OutHTML_Image( rWrt, rFrmFmt, aGrfNm,
                       pOLENd->GetAlternateText(), pOLENd->GetTwipSize(),
                       nFlags, pMarkToOLE );
    }

    return rWrt;
}

/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/html/htmlplug.cxx,v 1.5 2001-02-22 16:14:17 mtg Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.4  2000/12/21 16:21:48  jp
      writegraphic optional in original format and not general as JPG

      Revision 1.3  2000/11/01 19:23:14  jp
      export of mail graphics removed

      Revision 1.2  2000/10/20 13:43:09  jp
      use correct INetURL-Decode enum

      Revision 1.1.1.1  2000/09/18 17:14:55  hr
      initial import

      Revision 1.95  2000/09/18 16:04:45  willem.vandorp
      OpenOffice header added.

      Revision 1.94  2000/08/18 13:03:22  jp
      don't export escaped URLs

      Revision 1.93  2000/06/26 12:55:06  os
      INetURLObject::SmartRelToAbs removed

      Revision 1.92  2000/06/26 09:52:22  jp
      must change: GetAppWindow->GetDefaultDevice

      Revision 1.91  2000/05/15 10:06:56  os
      Chg: GetOrigFileName()

      Revision 1.90  2000/04/28 14:29:11  mib
      unicode

      Revision 1.89  2000/04/10 12:20:56  mib
      unicode

      Revision 1.88  2000/02/11 14:37:20  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.87  1999/09/21 09:49:53  mib
      multiple text encodings

      Revision 1.86  1999/09/17 12:14:02  mib
      support of multiple and non system text encodings

      Revision 1.85  1999/07/12 07:01:58  MIB
      #34644#: Applet\'s codebase is set to document base if not specified


*************************************************************************/

