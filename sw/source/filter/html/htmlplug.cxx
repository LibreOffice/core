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

#include "hintids.hxx"
#include <rtl/strbuf.hxx>
#include <svl/urihelper.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/frmhtml.hxx>
#include <sfx2/frmhtmlw.hxx>
#include <vcl/wrkwin.hxx>
#include <sot/storage.hxx>
#include <svx/xoutbmp.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <svtools/htmlkywd.hxx>
#include <svtools/htmltokn.h>
#include <SwAppletImpl.hxx>
#include <fmtornt.hxx>
#include <fmtfsize.hxx>
#include <fmtsrnd.hxx>
#include <fmtanchr.hxx>
#include <fmtcntnt.hxx>
#include <frmfmt.hxx>

#include <svl/ownlist.hxx>
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
#include <com/sun/star/embed/XClassifiedObject.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <comphelper/embeddedobjectcontainer.hxx>
#include <comphelper/classids.hxx>

using namespace com::sun::star;

#define HTML_DFLT_EMBED_WIDTH ((MM50*5)/2)
#define HTML_DFLT_EMBED_HEIGHT ((MM50*5)/2)

#define HTML_DFLT_APPLET_WIDTH ((MM50*5)/2)
#define HTML_DFLT_APPLET_HEIGHT ((MM50*5)/2)

namespace {

static char const sHTML_O_Hidden_False[] = "FALSE";

}

const sal_uLong HTML_FRMOPTS_EMBED_ALL      =
    HTML_FRMOPT_ALT |
    HTML_FRMOPT_SIZE |
    HTML_FRMOPT_NAME;
const sal_uLong HTML_FRMOPTS_EMBED_CNTNR    =
    HTML_FRMOPTS_EMBED_ALL |
    HTML_FRMOPT_ABSSIZE;
const sal_uLong HTML_FRMOPTS_EMBED          =
    HTML_FRMOPTS_EMBED_ALL |
    HTML_FRMOPT_ALIGN |
    HTML_FRMOPT_SPACE |
    HTML_FRMOPT_BRCLEAR |
    HTML_FRMOPT_NAME;
const sal_uLong HTML_FRMOPTS_HIDDEN_EMBED   =
    HTML_FRMOPT_ALT |
    HTML_FRMOPT_NAME;

const sal_uLong HTML_FRMOPTS_APPLET_ALL     =
    HTML_FRMOPT_ALT |
    HTML_FRMOPT_SIZE;
const sal_uLong HTML_FRMOPTS_APPLET_CNTNR   =
    HTML_FRMOPTS_APPLET_ALL |
    HTML_FRMOPT_ABSSIZE;
const sal_uLong HTML_FRMOPTS_APPLET         =
    HTML_FRMOPTS_APPLET_ALL |
    HTML_FRMOPT_ALIGN |
    HTML_FRMOPT_SPACE |
    HTML_FRMOPT_BRCLEAR;

const sal_uLong HTML_FRMOPTS_IFRAME_ALL     =
    HTML_FRMOPT_ALT |
    HTML_FRMOPT_SIZE;
const sal_uLong HTML_FRMOPTS_IFRAME_CNTNR   =
    HTML_FRMOPTS_IFRAME_ALL |
    HTML_FRMOPT_ABSSIZE;
const sal_uLong HTML_FRMOPTS_IFRAME         =
    HTML_FRMOPTS_IFRAME_ALL |
    HTML_FRMOPT_ALIGN |
    HTML_FRMOPT_SPACE |
    HTML_FRMOPT_BORDER |
    HTML_FRMOPT_BRCLEAR;

const sal_uLong HTML_FRMOPTS_OLE_CSS1       =
    HTML_FRMOPT_S_ALIGN |
    HTML_FRMOPT_S_SPACE;


void SwHTMLParser::SetFixSize( const Size& rPixSize,
                               const Size& rTwipDfltSize,
                               sal_Bool bPrcWidth, sal_Bool bPrcHeight,
                               SfxItemSet& /*rCSS1ItemSet*/,
                               SvxCSS1PropertyInfo& rCSS1PropInfo,
                               SfxItemSet& rFlyItemSet )
{
    // absolulte Groessenangaben in Twip umrechnen
    sal_uInt8 nPrcWidth = 0, nPrcHeight = 0;
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
        nPrcWidth = (sal_uInt8)rCSS1PropInfo.nWidth;
        aTwipSz.Width() = rTwipDfltSize.Width();
    }
    else if( SVX_CSS1_LTYPE_TWIP== rCSS1PropInfo.eWidthType )
    {
        aTwipSz.Width() = rCSS1PropInfo.nWidth;
    }
    else if( bPrcWidth && rPixSize.Width() )
    {
        nPrcWidth = (sal_uInt8)rPixSize.Width();
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
        nPrcHeight = (sal_uInt8)rCSS1PropInfo.nHeight;
        aTwipSz.Height() = rTwipDfltSize.Height();
    }
    else if( SVX_CSS1_LTYPE_TWIP== rCSS1PropInfo.eHeightType )
    {
        aTwipSz.Height() = rCSS1PropInfo.nHeight;
    }
    else if( bPrcHeight && rPixSize.Height() )
    {
        nPrcHeight = (sal_uInt8)rPixSize.Height();
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
    sal_Int32 nLeftSpace = 0, nRightSpace = 0;
    sal_uInt16 nUpperSpace = 0, nLowerSpace = 0;
    if( (rPixSpace.Width() || rPixSpace.Height()) && Application::GetDefaultDevice() )
    {
        Size aTwipSpc( rPixSpace.Width(), rPixSpace.Height() );
        aTwipSpc =
            Application::GetDefaultDevice()->PixelToLogic( aTwipSpc,
                                                MapMode(MAP_TWIP) );
        nLeftSpace = nRightSpace = aTwipSpc.Width();
        nUpperSpace = nLowerSpace = (sal_uInt16)aTwipSpc.Height();
    }

    // linken/rechten Rand setzen
    const SfxPoolItem *pItem;
    if( SFX_ITEM_SET==rCSS1ItemSet.GetItemState( RES_LR_SPACE, sal_True, &pItem ) )
    {
        // Ggf. den Erstzeilen-Einzug noch plaetten
        const SvxLRSpaceItem *pLRItem = (const SvxLRSpaceItem *)pItem;
        SvxLRSpaceItem aLRItem( *pLRItem );
        aLRItem.SetTxtFirstLineOfst( 0 );
        if( rCSS1PropInfo.bLeftMargin )
        {
            nLeftSpace = aLRItem.GetLeft();
            rCSS1PropInfo.bLeftMargin = sal_False;
        }
        if( rCSS1PropInfo.bRightMargin )
        {
            nRightSpace = aLRItem.GetRight();
            rCSS1PropInfo.bRightMargin = sal_False;
        }
        rCSS1ItemSet.ClearItem( RES_LR_SPACE );
    }
    if( nLeftSpace > 0 || nRightSpace > 0 )
    {
        SvxLRSpaceItem aLRItem( RES_LR_SPACE );
        aLRItem.SetLeft( nLeftSpace > 0 ? nLeftSpace : 0 );
        aLRItem.SetRight( nRightSpace > 0 ? nRightSpace : 0 );
        rFlyItemSet.Put( aLRItem );
        if( nLeftSpace )
        {
            const SwFmtHoriOrient& rHoriOri =
                (const SwFmtHoriOrient&)rFlyItemSet.Get( RES_HORI_ORIENT );
            if( text::HoriOrientation::NONE == rHoriOri.GetHoriOrient() )
            {
                SwFmtHoriOrient aHoriOri( rHoriOri );
                aHoriOri.SetPos( aHoriOri.GetPos() + nLeftSpace );
                rFlyItemSet.Put( aHoriOri );
            }
        }
    }

    // oberen/unteren Rand setzen
    if( SFX_ITEM_SET==rCSS1ItemSet.GetItemState( RES_UL_SPACE, sal_True, &pItem ) )
    {
        // Ggf. den Erstzeilen-Einzug noch plaetten
        const SvxULSpaceItem *pULItem = (const SvxULSpaceItem *)pItem;
        if( rCSS1PropInfo.bTopMargin )
        {
            nUpperSpace = pULItem->GetUpper();
            rCSS1PropInfo.bTopMargin = sal_False;
        }
        if( rCSS1PropInfo.bBottomMargin )
        {
            nLowerSpace = pULItem->GetLower();
            rCSS1PropInfo.bBottomMargin = sal_False;
        }
        rCSS1ItemSet.ClearItem( RES_UL_SPACE );
    }
    if( nUpperSpace || nLowerSpace )
    {
        SvxULSpaceItem aULItem( RES_UL_SPACE );
        aULItem.SetUpper( nUpperSpace );
        aULItem.SetLower( nLowerSpace );
        rFlyItemSet.Put( aULItem );
        if( nUpperSpace )
        {
            const SwFmtVertOrient& rVertOri =
                (const SwFmtVertOrient&)rFlyItemSet.Get( RES_VERT_ORIENT );
            if( text::VertOrientation::NONE == rVertOri.GetVertOrient() )
            {
                SwFmtVertOrient aVertOri( rVertOri );
                aVertOri.SetPos( aVertOri.GetPos() + nUpperSpace );
                rFlyItemSet.Put( aVertOri );
            }
        }
    }
}


void SwHTMLParser::InsertEmbed()
{
    String aURL, aType, aName, aAlt, aId, aStyle, aClass;
    Size aSize( USHRT_MAX, USHRT_MAX );
    Size aSpace( USHRT_MAX, USHRT_MAX );
    sal_Bool bPrcWidth = sal_False, bPrcHeight = sal_False, bHidden = sal_False;
    sal_Int16 eVertOri = text::VertOrientation::NONE;
    sal_Int16 eHoriOri = text::HoriOrientation::NONE;
    SvCommandList aCmdLst;
    const HTMLOptions& rHTMLOptions = GetOptions();

    // Die Optionen werden vorwaerts gelesen, weil die Plugins sie in
    // dieser Reihenfolge erwarten. Trotzdem darf immer nur der erste
    // Wert einer Option beruecksichtigt werden.
    for (size_t i = 0, n = rHTMLOptions.size(); i < n; ++i)
    {
        const HTMLOption& rOption = rHTMLOptions[i];
        switch( rOption.GetToken() )
        {
        case HTML_O_ID:
            aId = rOption.GetString();
            break;
        case HTML_O_STYLE:
            aStyle = rOption.GetString();
            break;
        case HTML_O_CLASS:
            aClass = rOption.GetString();
            break;
        case HTML_O_NAME:
            aName = rOption.GetString();
            break;
        case HTML_O_SRC:
            if( !aURL.Len() )
                aURL = rOption.GetString();
            break;
        case HTML_O_ALT:
            aAlt = rOption.GetString();
            break;
        case HTML_O_TYPE:
            if( !aType.Len() )
                aType = rOption.GetString();
            break;
        case HTML_O_ALIGN:
            if( eVertOri==text::VertOrientation::NONE && eHoriOri==text::HoriOrientation::NONE )
            {
                eVertOri = rOption.GetEnum( aHTMLImgVAlignTable, eVertOri );
                eHoriOri = rOption.GetEnum( aHTMLImgHAlignTable, eHoriOri );
            }
            break;
        case HTML_O_WIDTH:
            if( USHRT_MAX==aSize.Width() )
            {
                bPrcWidth = (rOption.GetString().Search('%') != STRING_NOTFOUND);
                aSize.Width() = (long)rOption.GetNumber();
            }
            break;
        case HTML_O_HEIGHT:
            if( USHRT_MAX==aSize.Height() )
            {
                bPrcHeight = (rOption.GetString().Search('%') != STRING_NOTFOUND);
                aSize.Height() = (long)rOption.GetNumber();
            }
            break;
        case HTML_O_HSPACE:
            if( USHRT_MAX==aSpace.Width() )
                aSpace.Width() = (long)rOption.GetNumber();
            break;
        case HTML_O_VSPACE:
            if( USHRT_MAX==aSpace.Height() )
                aSpace.Height() = (long)rOption.GetNumber();
            break;
        case HTML_O_UNKNOWN:
            if( rOption.GetTokenString().EqualsIgnoreCaseAscii( OOO_STRING_SW_HTML_O_Hidden ) )
                bHidden =
                    !rOption.GetString().EqualsIgnoreCaseAscii( sHTML_O_Hidden_False );
            break;
        }

        // Es werden alle Parameter an das Plugin weitergereicht
        aCmdLst.Append( rOption.GetTokenString(), rOption.GetString() );
    }

    SfxItemSet aItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
    SvxCSS1PropertyInfo aPropInfo;
    if( HasStyleOptions( aStyle, aId, aClass ) )
        ParseStyleOptions( aStyle, aId, aClass, aItemSet, aPropInfo );

    // Die Default-Werte umsetzen (ausser Hoehe/Breite, das macht schon
    // SetFrmSize() fuer uns)
    if( eVertOri==text::VertOrientation::NONE && eHoriOri==text::HoriOrientation::NONE )
        eVertOri = text::VertOrientation::TOP;
    if( USHRT_MAX==aSpace.Width() )
        aSpace.Width() = 0;
    if( USHRT_MAX==aSpace.Height() )
        aSpace.Height() = 0;
    if( bHidden )
    {
        // Size (0,0) wird in SetFrmSize auf (MINFLY, MINFLY) umgebogen
        aSize.Width() = 0; aSize.Height() = 0;
        aSpace.Width() = 0; aSpace.Height() = 0;
        bPrcWidth = bPrcHeight = sal_False;
    }

    // die URL aufbereiten
    INetURLObject aURLObj;
    bool bHasURL = aURL.Len() &&
                   aURLObj.SetURL(
                       URIHelper::SmartRel2Abs(
                           INetURLObject(sBaseURL), aURL,
                           URIHelper::GetMaybeFileHdl()) );

    // do not insert plugin if it has neither URL nor type
    bool bHasType = aType.Len() != 0;
    if( !bHasURL && !bHasType )
        return;

    // das Plugin anlegen
    comphelper::EmbeddedObjectContainer aCnt;
    OUString aObjName;
    uno::Reference < embed::XEmbeddedObject > xObj = aCnt.CreateEmbeddedObject( SvGlobalName( SO3_PLUGIN_CLASSID ).GetByteSequence(), aObjName );
    if ( svt::EmbeddedObjectRef::TryRunningState( xObj ) )
    {
        uno::Reference < beans::XPropertySet > xSet( xObj->getComponent(), uno::UNO_QUERY );
        if ( xSet.is() )
        {
            if( bHasURL )
                xSet->setPropertyValue("PluginURL",
                    uno::makeAny( OUString( aURL ) ) );
            if( bHasType )
                xSet->setPropertyValue("PluginMimeType",
                    uno::makeAny( OUString( aType ) ) );

            uno::Sequence < beans::PropertyValue > aProps;
            aCmdLst.FillSequence( aProps );
            xSet->setPropertyValue("PluginCommands", uno::makeAny( aProps ) );

        }
    }

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
        SwFmtAnchor aAnchor( FLY_AT_PARA );
        aAnchor.SetAnchor( pPam->GetPoint() );
        aFrmSet.Put( aAnchor );
        aFrmSet.Put( SwFmtHoriOrient( 0, text::HoriOrientation::LEFT, text::RelOrientation::FRAME) );
        aFrmSet.Put( SwFmtSurround( SURROUND_THROUGHT ) );
        aFrmSet.Put( SwFmtVertOrient( 0, text::VertOrientation::TOP, text::RelOrientation::PRINT_AREA ) );
    }

    // und noch die Groesse des Rahmens
    Size aDfltSz( HTML_DFLT_EMBED_WIDTH, HTML_DFLT_EMBED_HEIGHT );
    SetFixSize( aSize, aDfltSz, bPrcWidth, bPrcHeight, aItemSet, aPropInfo,
                aFrmSet );
    SetSpace( aSpace, aItemSet, aPropInfo, aFrmSet );

    // und in das Dok einfuegen
    SwFrmFmt* pFlyFmt =
        pDoc->Insert( *pPam, ::svt::EmbeddedObjectRef( xObj, embed::Aspects::MSOLE_CONTENT ), &aFrmSet, NULL, NULL );

    // Namen am FrmFmt setzen
    if( aName.Len() )
        pFlyFmt->SetName( aName );

    // den alternativen Text setzen
    SwNoTxtNode *pNoTxtNd =
        pDoc->GetNodes()[ pFlyFmt->GetCntnt().GetCntntIdx()
                          ->GetIndex()+1 ]->GetNoTxtNode();
    pNoTxtNd->SetTitle( aAlt );

    // Ggf Frames anlegen und auto-geb. Rahmen registrieren
    if( !bHidden )
    {
        // HIDDEN-Plugins sollen absatzgebunden bleiben. Da RegisterFlyFrm
        // absatzgebundene Rahmen mit DUrchlauf in am Zeichen gebundene
        // Rahmen umwandelt, muessen die Frames hier von Hand angelegt werden.
        RegisterFlyFrm( pFlyFmt );
    }
}


#ifdef SOLAR_JAVA
void SwHTMLParser::NewObject()
{
    String aClassID, aName, aStandBy, aId, aStyle, aClass;
    Size aSize( USHRT_MAX, USHRT_MAX );
    Size aSpace( 0, 0 );
    sal_Int16 eVertOri = text::VertOrientation::TOP;
    sal_Int16 eHoriOri = text::HoriOrientation::NONE;

    sal_Bool bPrcWidth = sal_False, bPrcHeight = sal_False,
             bDeclare = sal_False;
    // Eine neue Command-List anlegen
    delete pAppletImpl;
    pAppletImpl = new SwApplet_Impl( pDoc->GetAttrPool(),
                                     RES_FRMATR_BEGIN, RES_FRMATR_END-1 );

    const HTMLOptions& rHTMLOptions = GetOptions();
    for (size_t i = rHTMLOptions.size(); i; )
    {
        const HTMLOption& rOption = rHTMLOptions[--i];
        switch( rOption.GetToken() )
        {
        case HTML_O_ID:
            aId = rOption.GetString();
            break;
        case HTML_O_STYLE:
            aStyle = rOption.GetString();
            break;
        case HTML_O_CLASS:
            aClass = rOption.GetString();
            break;
        case HTML_O_DECLARE:
            bDeclare = sal_True;
            break;
        case HTML_O_CLASSID:
            aClassID = rOption.GetString();
            break;
        case HTML_O_CODEBASE:
            break;
        case HTML_O_DATA:
            break;
        case HTML_O_TYPE:
            break;
        case HTML_O_CODETYPE:
            break;
        case HTML_O_ARCHIVE:
        case HTML_O_UNKNOWN:
            break;
        case HTML_O_STANDBY:
            aStandBy = rOption.GetString();
            break;
        case HTML_O_WIDTH:
            bPrcWidth = (rOption.GetString().Search('%') != STRING_NOTFOUND);
            aSize.Width() = (long)rOption.GetNumber();
            break;
        case HTML_O_HEIGHT:
            bPrcHeight = (rOption.GetString().Search('%') != STRING_NOTFOUND);
            aSize.Height() = (long)rOption.GetNumber();
            break;
        case HTML_O_ALIGN:
            eVertOri = rOption.GetEnum( aHTMLImgVAlignTable, eVertOri );
            eHoriOri = rOption.GetEnum( aHTMLImgHAlignTable, eHoriOri );
            break;
        case HTML_O_USEMAP:
            break;
        case HTML_O_NAME:
            aName = rOption.GetString();
            break;
        case HTML_O_HSPACE:
            aSpace.Width() = (long)rOption.GetNumber();
            break;
        case HTML_O_VSPACE:
            aSpace.Height() = (long)rOption.GetNumber();
            break;
        case HTML_O_BORDER:
            break;

        case HTML_O_SDONCLICK:
        case HTML_O_ONCLICK:
        case HTML_O_SDONMOUSEOVER:
        case HTML_O_ONMOUSEOVER:
        case HTML_O_SDONMOUSEOUT:
        case HTML_O_ONMOUSEOUT:
            break;
        }
        // Es werden alle Parameter auch an das Applet weitergereicht
        pAppletImpl->AppendParam( rOption.GetTokenString(),
                                  rOption.GetString() );

    }

    // Objects that are declared only are not evaluated. Moreover, only
    // Java applets are supported.
    sal_Bool bIsApplet = sal_False;

    if( !bDeclare && aClassID.Len() == 42 &&
        aClassID.EqualsAscii( "clsid:", 0, 6 ) )
    {
        aClassID.Erase( 0, 6 );
        SvGlobalName aCID;
        if( aCID.MakeId( aClassID ) )
        {
            SvGlobalName aJavaCID( 0x8AD9C840UL, 0x044EU, 0x11D1U, 0xB3U, 0xE9U,
                                   0x00U, 0x80U, 0x5FU, 0x49U, 0x9DU, 0x93U );

            bIsApplet = aJavaCID == aCID;
        }
    }

    if( !bIsApplet )
    {
        delete pAppletImpl;
        pAppletImpl = 0;
        return;
    }

    pAppletImpl->SetAltText( aStandBy );

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
}
#endif

void SwHTMLParser::EndObject()
{
#ifdef SOLAR_JAVA
    if( !pAppletImpl )
        return;
    if( pAppletImpl->CreateApplet( sBaseURL ) )
    {
        pAppletImpl->FinishApplet();

        // und in das Dok einfuegen
        SwFrmFmt* pFlyFmt =
            pDoc->Insert( *pPam,
                    ::svt::EmbeddedObjectRef( pAppletImpl->GetApplet(), embed::Aspects::MSOLE_CONTENT ),
                    &pAppletImpl->GetItemSet(),
                    NULL,
                    NULL );

        // den alternativen Namen setzen
        SwNoTxtNode *pNoTxtNd =
            pDoc->GetNodes()[ pFlyFmt->GetCntnt().GetCntntIdx()
                              ->GetIndex()+1 ]->GetNoTxtNode();
        pNoTxtNd->SetTitle( pAppletImpl->GetAltText() );

        // Ggf Frames anlegen und auto-geb. Rahmen registrieren
        RegisterFlyFrm( pFlyFmt );

        delete pAppletImpl;
        pAppletImpl = 0;
    }
#endif
}

#ifdef SOLAR_JAVA
void SwHTMLParser::InsertApplet()
{
    String aCodeBase, aCode, aName, aAlt, aId, aStyle, aClass;
    Size aSize( USHRT_MAX, USHRT_MAX );
    Size aSpace( 0, 0 );
    sal_Bool bPrcWidth = sal_False, bPrcHeight = sal_False, bMayScript = sal_False;
    sal_Int16 eVertOri = text::VertOrientation::TOP;
    sal_Int16 eHoriOri = text::HoriOrientation::NONE;

    // Eine neue Command-List anlegen
    delete pAppletImpl;
    pAppletImpl = new SwApplet_Impl( pDoc->GetAttrPool(), RES_FRMATR_BEGIN, RES_FRMATR_END-1 );

    const HTMLOptions& rHTMLOptions = GetOptions();
    for (size_t i = rHTMLOptions.size(); i; )
    {
        const HTMLOption& rOption = rHTMLOptions[--i];
        switch( rOption.GetToken() )
        {
        case HTML_O_ID:
            aId = rOption.GetString();
            break;
        case HTML_O_STYLE:
            aStyle = rOption.GetString();
            break;
        case HTML_O_CLASS:
            aClass = rOption.GetString();
            break;
        case HTML_O_CODEBASE:
            aCodeBase = rOption.GetString();
            break;
        case HTML_O_CODE:
            aCode = rOption.GetString();
            break;
        case HTML_O_NAME:
            aName = rOption.GetString();
            break;
        case HTML_O_ALT:
            aAlt = rOption.GetString();
            break;
        case HTML_O_ALIGN:
            eVertOri = rOption.GetEnum( aHTMLImgVAlignTable, eVertOri );
            eHoriOri = rOption.GetEnum( aHTMLImgHAlignTable, eHoriOri );
            break;
        case HTML_O_WIDTH:
            bPrcWidth = (rOption.GetString().Search('%') != STRING_NOTFOUND);
            aSize.Width() = (long)rOption.GetNumber();
            break;
        case HTML_O_HEIGHT:
            bPrcHeight = (rOption.GetString().Search('%') != STRING_NOTFOUND);
            aSize.Height() = (long)rOption.GetNumber();
            break;
        case HTML_O_HSPACE:
            aSpace.Width() = (long)rOption.GetNumber();
            break;
        case HTML_O_VSPACE:
            aSpace.Height() = (long)rOption.GetNumber();
            break;
        case HTML_O_MAYSCRIPT:
            bMayScript = sal_True;
            break;
        }

        // Es werden alle Parameter auch an das Applet weitergereicht
        pAppletImpl->AppendParam( rOption.GetTokenString(),
                                  rOption.GetString() );
    }

    if( !aCode.Len() )
    {
        delete pAppletImpl;
        pAppletImpl = 0;
        return;
    }

    if ( aCodeBase.Len() )
        aCodeBase = INetURLObject::GetAbsURL( sBaseURL, aCodeBase );
    pAppletImpl->CreateApplet( aCode, aName, bMayScript, aCodeBase, sBaseURL );//, aAlt );
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
}
#endif

void SwHTMLParser::EndApplet()
{
#ifdef SOLAR_JAVA
    if( !pAppletImpl )
        return;

    pAppletImpl->FinishApplet();

    // und in das Dok einfuegen
    SwFrmFmt* pFlyFmt =
        pDoc->Insert( *pPam,
                    ::svt::EmbeddedObjectRef( pAppletImpl->GetApplet(), embed::Aspects::MSOLE_CONTENT ),
                    &pAppletImpl->GetItemSet(),
                    NULL,
                    NULL );

    // den alternativen Namen setzen
    SwNoTxtNode *pNoTxtNd =
        pDoc->GetNodes()[ pFlyFmt->GetCntnt().GetCntntIdx()
                          ->GetIndex()+1 ]->GetNoTxtNode();
    pNoTxtNd->SetTitle( pAppletImpl->GetAltText() );

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

    const HTMLOptions& rHTMLOptions = GetOptions();
    for (size_t i = rHTMLOptions.size(); i; )
    {
        const HTMLOption& rOption = rHTMLOptions[--i];
        switch( rOption.GetToken() )
        {
        case HTML_O_NAME:
            aName = rOption.GetString();
            break;
        case HTML_O_VALUE:
            aValue = rOption.GetString();
            break;
        }
    }

    if( !aName.Len() )
        return;

    pAppletImpl->AppendParam( aName, aValue );
#endif
}



void SwHTMLParser::InsertFloatingFrame()
{
    String aAlt, aId, aStyle, aClass;
    Size aSize( USHRT_MAX, USHRT_MAX );
    Size aSpace( 0, 0 );
    sal_Bool bPrcWidth = sal_False, bPrcHeight = sal_False;
    sal_Int16 eVertOri = text::VertOrientation::TOP;
    sal_Int16 eHoriOri = text::HoriOrientation::NONE;

    const HTMLOptions& rHTMLOptions = GetOptions();

    // Erstmal die Optionen f?r das Writer-Frame-Format holen
    for (size_t i = 0, n = rHTMLOptions.size(); i < n; ++i)
    {
        const HTMLOption& rOption = rHTMLOptions[i];
        switch( rOption.GetToken() )
        {
        case HTML_O_ID:
            aId = rOption.GetString();
            break;
        case HTML_O_STYLE:
            aStyle = rOption.GetString();
            break;
        case HTML_O_CLASS:
            aClass = rOption.GetString();
            break;
        case HTML_O_ALT:
            aAlt = rOption.GetString();
            break;
        case HTML_O_ALIGN:
            eVertOri = rOption.GetEnum( aHTMLImgVAlignTable, eVertOri );
            eHoriOri = rOption.GetEnum( aHTMLImgHAlignTable, eHoriOri );
            break;
        case HTML_O_WIDTH:
            bPrcWidth = (rOption.GetString().Search('%') != STRING_NOTFOUND);
            aSize.Width() = (long)rOption.GetNumber();
            break;
        case HTML_O_HEIGHT:
            bPrcHeight = (rOption.GetString().Search('%') != STRING_NOTFOUND);
            aSize.Height() = (long)rOption.GetNumber();
            break;
        case HTML_O_HSPACE:
            aSpace.Width() = (long)rOption.GetNumber();
            break;
        case HTML_O_VSPACE:
            aSpace.Height() = (long)rOption.GetNumber();
            break;
        }
    }

    // und jetzt die fuer den SfxFrame
    SfxFrameDescriptor aFrameDesc;

    SfxFrameHTMLParser::ParseFrameOptions( &aFrameDesc, rHTMLOptions, sBaseURL );

    // den Floating-Frame anlegen
    comphelper::EmbeddedObjectContainer aCnt;
    OUString aObjName;
    uno::Reference < embed::XEmbeddedObject > xObj = aCnt.CreateEmbeddedObject( SvGlobalName( SO3_IFRAME_CLASSID ).GetByteSequence(), aObjName );

    try
    {
        // TODO/MBA: testing
        if ( svt::EmbeddedObjectRef::TryRunningState( xObj ) )
        {
            uno::Reference < beans::XPropertySet > xSet( xObj->getComponent(), uno::UNO_QUERY );
            if ( xSet.is() )
            {
                OUString aName = aFrameDesc.GetName();
                ScrollingMode eScroll = aFrameDesc.GetScrollingMode();
                sal_Bool bHasBorder = aFrameDesc.HasFrameBorder();
                Size aMargin = aFrameDesc.GetMargin();

                xSet->setPropertyValue("FrameURL", uno::makeAny( OUString( aFrameDesc.GetURL().GetMainURL( INetURLObject::NO_DECODE ) ) ) );
                xSet->setPropertyValue("FrameName", uno::makeAny( aName ) );

                if ( eScroll == ScrollingAuto )
                    xSet->setPropertyValue("FrameIsAutoScroll",
                        uno::makeAny( sal_True ) );
                else
                    xSet->setPropertyValue("FrameIsScrollingMode",
                        uno::makeAny( (sal_Bool) ( eScroll == ScrollingYes) ) );

                xSet->setPropertyValue("FrameIsBorder",
                        uno::makeAny( bHasBorder ) );

                xSet->setPropertyValue("FrameMarginWidth",
                    uno::makeAny( sal_Int32( aMargin.Width() ) ) );

                xSet->setPropertyValue("FrameMarginHeight",
                    uno::makeAny( sal_Int32( aMargin.Height() ) ) );
            }
        }
    }
    catch ( uno::Exception& )
    {
    }

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
    SwFrmFmt* pFlyFmt =
        pDoc->Insert( *pPam, ::svt::EmbeddedObjectRef( xObj, embed::Aspects::MSOLE_CONTENT ), &aFrmSet, NULL, NULL );

    // den alternativen Namen setzen
    SwNoTxtNode *pNoTxtNd =
        pDoc->GetNodes()[ pFlyFmt->GetCntnt().GetCntntIdx()
                          ->GetIndex()+1 ]->GetNoTxtNode();
    pNoTxtNd->SetTitle( aAlt );

    // Ggf Frames anlegen und auto-geb. Rahmen registrieren
    RegisterFlyFrm( pFlyFmt );

    bInFloatingFrame = sal_True;
}


sal_uInt16 SwHTMLWriter::GuessOLENodeFrmType( const SwNode& rNode )
{
    SwOLEObj& rObj = ((SwOLENode*)rNode.GetOLENode())->GetOLEObj();

    SwHTMLFrmType eType = HTML_FRMTYPE_OLE;

    uno::Reference < embed::XClassifiedObject > xClass ( rObj.GetOleRef(), uno::UNO_QUERY );
    SvGlobalName aClass( xClass->getClassID() );
    if( aClass == SvGlobalName( SO3_PLUGIN_CLASSID ) )
    {
        eType = HTML_FRMTYPE_PLUGIN;
    }
    else if( aClass == SvGlobalName( SO3_IFRAME_CLASSID ) )
    {
        eType = HTML_FRMTYPE_IFRAME;
    }
#ifdef SOLAR_JAVA
    else if( aClass == SvGlobalName( SO3_APPLET_CLASSID ) )
    {
        eType = HTML_FRMTYPE_APPLET;
    }
#endif

    return static_cast< sal_uInt16 >(eType);
}

Writer& OutHTML_FrmFmtOLENode( Writer& rWrt, const SwFrmFmt& rFrmFmt,
                               sal_Bool bInCntnr )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    const SwFmtCntnt& rFlyCntnt = rFrmFmt.GetCntnt();
    sal_uLong nStt = rFlyCntnt.GetCntntIdx()->GetIndex()+1;
    SwOLENode *pOLENd = rHTMLWrt.pDoc->GetNodes()[ nStt ]->GetOLENode();

    OSL_ENSURE( pOLENd, "OLE-Node erwartet" );
    if( !pOLENd )
        return rWrt;

    SwOLEObj &rObj = pOLENd->GetOLEObj();

    uno::Reference < embed::XEmbeddedObject > xObj( rObj.GetOleRef() );
    if ( !svt::EmbeddedObjectRef::TryRunningState( xObj ) )
        return rWrt;

    uno::Reference < beans::XPropertySet > xSet( xObj->getComponent(), uno::UNO_QUERY );
    sal_Bool bHiddenEmbed = sal_False;

    if( !xSet.is() )
    {
        OSL_FAIL("Unknown Object" );
        return rWrt;
    }

    sal_uLong nFrmOpts;

    // wenn meoglich vor dem "Objekt" einen Zeilen-Umbruch ausgeben
    if( rHTMLWrt.bLFPossible )
        rHTMLWrt.OutNewLine( sal_True );

    if( !rFrmFmt.GetName().isEmpty() )
        rHTMLWrt.OutImplicitMark( rFrmFmt.GetName(),
                                  pMarkToOLE );
    uno::Any aAny;
    SvGlobalName aGlobName( xObj->getClassID() );
    OStringBuffer sOut;
    sOut.append('<');
    if( aGlobName == SvGlobalName( SO3_PLUGIN_CLASSID ) )
    {
        // erstmal das Plug-spezifische
        sOut.append(OOO_STRING_SVTOOLS_HTML_embed);

        OUString aStr;
        String aURL;
        aAny = xSet->getPropertyValue("PluginURL");
        if( (aAny >>= aStr) && !aStr.isEmpty() )
        {
            aURL = URIHelper::simpleNormalizedMakeRelative( rWrt.GetBaseURL(),
                      aStr);
        }

        if( aURL.Len() )
        {
            sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_src)
                .append("=\"");
            rWrt.Strm() << sOut.makeStringAndClear().getStr();
            HTMLOutFuncs::Out_String( rWrt.Strm(), aURL, rHTMLWrt.eDestEnc, &rHTMLWrt.aNonConvertableCharacters );
            sOut.append('\"');
        }

        OUString aType;
        aAny = xSet->getPropertyValue("PluginMimeType");
        if( (aAny >>= aType) && !aType.isEmpty() )
        {
            sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_type)
                .append("=\"");
            rWrt.Strm() << sOut.makeStringAndClear().getStr();
            HTMLOutFuncs::Out_String( rWrt.Strm(), aType, rHTMLWrt.eDestEnc, &rHTMLWrt.aNonConvertableCharacters );
            sOut.append('\"');
        }

        if ((FLY_AT_PARA == rFrmFmt.GetAnchor().GetAnchorId()) &&
            SURROUND_THROUGHT == rFrmFmt.GetSurround().GetSurround() )
        {
            // Das Plugin ist HIDDEN
            sOut.append(' ').append(OOO_STRING_SW_HTML_O_Hidden);
            nFrmOpts = HTML_FRMOPTS_HIDDEN_EMBED;
            bHiddenEmbed = sal_True;
        }
        else
        {
            nFrmOpts = bInCntnr ? HTML_FRMOPTS_EMBED_CNTNR
                                : HTML_FRMOPTS_EMBED;
        }
    }
    else if( aGlobName == SvGlobalName( SO3_APPLET_CLASSID ) )
    {
        // oder das Applet-Spezifische

        sOut.append(OOO_STRING_SVTOOLS_HTML_applet);

        // CODEBASE
        OUString aCd;
        aAny = xSet->getPropertyValue("AppletCodeBase");
        if( (aAny >>= aCd) && !aCd.isEmpty() )
        {
            String sCodeBase( URIHelper::simpleNormalizedMakeRelative(rWrt.GetBaseURL(), aCd) );
            if( sCodeBase.Len() )
            {
                sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_codebase)
                    .append("=\"");
                rWrt.Strm() << sOut.makeStringAndClear().getStr();
                HTMLOutFuncs::Out_String( rWrt.Strm(), sCodeBase, rHTMLWrt.eDestEnc, &rHTMLWrt.aNonConvertableCharacters );
                sOut.append('\"');
            }
        }

        // CODE
        OUString aClass;
        aAny = xSet->getPropertyValue("AppletCode");
        aAny >>= aClass;
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_code)
            .append("=\"");
        rWrt.Strm() << sOut.makeStringAndClear().getStr();
        HTMLOutFuncs::Out_String( rWrt.Strm(), aClass, rHTMLWrt.eDestEnc, &rHTMLWrt.aNonConvertableCharacters );
        sOut.append('\"');

        // NAME
        OUString aAppletName;
        aAny = xSet->getPropertyValue("AppletName");
        aAny >>= aAppletName;
        if( !aAppletName.isEmpty() )
        {
            sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_name)
                .append("=\"");
            rWrt.Strm() << sOut.makeStringAndClear().getStr();
            HTMLOutFuncs::Out_String( rWrt.Strm(), aAppletName, rHTMLWrt.eDestEnc, &rHTMLWrt.aNonConvertableCharacters );
            sOut.append('\"');
        }

        sal_Bool bScript = sal_False;
        aAny = xSet->getPropertyValue("AppletIsScript");
        aAny >>= bScript;
        if( bScript )
            sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_mayscript);

        nFrmOpts = bInCntnr ? HTML_FRMOPTS_APPLET_CNTNR
                            : HTML_FRMOPTS_APPLET;
    }
    else
    {
        // oder das Flating-Frame spezifische

        sOut.append(OOO_STRING_SVTOOLS_HTML_iframe);
        rWrt.Strm() << sOut.makeStringAndClear().getStr();

        SfxFrameHTMLWriter::Out_FrameDescriptor( rWrt.Strm(), rWrt.GetBaseURL(),
                                        xSet,
                                        rHTMLWrt.eDestEnc,
                                        &rHTMLWrt.aNonConvertableCharacters );

        nFrmOpts = bInCntnr ? HTML_FRMOPTS_IFRAME_CNTNR
                            : HTML_FRMOPTS_IFRAME;
    }

    rWrt.Strm() << sOut.makeStringAndClear().getStr();

    // ALT, WIDTH, HEIGHT, HSPACE, VSPACE, ALIGN
    if( rHTMLWrt.IsHTMLMode( HTMLMODE_ABS_POS_FLY ) && !bHiddenEmbed )
        nFrmOpts |= HTML_FRMOPTS_OLE_CSS1;
    OString aEndTags = rHTMLWrt.OutFrmFmtOptions( rFrmFmt, pOLENd->GetTitle(), nFrmOpts );
    if( rHTMLWrt.IsHTMLMode( HTMLMODE_ABS_POS_FLY ) && !bHiddenEmbed )
        rHTMLWrt.OutCSS1_FrmFmtOptions( rFrmFmt, nFrmOpts );

    if( aGlobName == SvGlobalName( SO3_APPLET_CLASSID ) )
    {
        // fuer Applets die Parameter als eigene Tags ausgeben
        // und ein </APPLET> schreiben

        uno::Sequence < beans::PropertyValue > aProps;
        aAny = xSet->getPropertyValue("AppletCommands");
        aAny >>= aProps;

        SvCommandList aCommands;
        aCommands.FillFromSequence( aProps );
        std::vector<sal_uLong> aParams;
        size_t i = aCommands.size();
        while( i > 0 )
        {
            const SvCommand& rCommand = aCommands[ --i ];
            const String& rName = rCommand.GetCommand();
            sal_uInt16 nType = SwApplet_Impl::GetOptionType( rName, sal_True );
            if( SWHTML_OPTTYPE_TAG == nType )
            {
                const String& rValue = rCommand.GetArgument();
                rWrt.Strm() << ' ';
                HTMLOutFuncs::Out_String( rWrt.Strm(), rName, rHTMLWrt.eDestEnc, &rHTMLWrt.aNonConvertableCharacters );
                rWrt.Strm() << "=\"";
                HTMLOutFuncs::Out_String( rWrt.Strm(), rValue, rHTMLWrt.eDestEnc, &rHTMLWrt.aNonConvertableCharacters ) << '\"';
            }
            else if( SWHTML_OPTTYPE_PARAM == nType )
            {
                aParams.push_back( i );
            }
        }

        rHTMLWrt.Strm() << '>';

        rHTMLWrt.IncIndentLevel(); // Inhalt von Applet einruecken

        sal_uInt16 ii = aParams.size();
        while( ii > 0  )
        {
            const SvCommand& rCommand = aCommands[ aParams[--ii] ];
            const String& rName = rCommand.GetCommand();
            const String& rValue = rCommand.GetArgument();
            rHTMLWrt.OutNewLine();
            OStringBuffer sBuf;
            sBuf.append('<').append(OOO_STRING_SVTOOLS_HTML_param)
                .append(' ').append(OOO_STRING_SVTOOLS_HTML_O_name)
                .append("=\"");
            rWrt.Strm() << sBuf.makeStringAndClear().getStr();
            HTMLOutFuncs::Out_String( rWrt.Strm(), rName, rHTMLWrt.eDestEnc, &rHTMLWrt.aNonConvertableCharacters );
            sBuf.append("\" ").append(OOO_STRING_SVTOOLS_HTML_O_value)
                .append("=\"");
            rWrt.Strm() << sBuf.makeStringAndClear().getStr();
            HTMLOutFuncs::Out_String( rWrt.Strm(), rValue, rHTMLWrt.eDestEnc, &rHTMLWrt.aNonConvertableCharacters ) << "\">";
        }

        rHTMLWrt.DecIndentLevel(); // Inhalt von Applet einruecken
        if( aCommands.size() )
            rHTMLWrt.OutNewLine();
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_applet, sal_False );
    }
    else if( aGlobName == SvGlobalName( SO3_PLUGIN_CLASSID ) )
    {
        // fuer Plugins die Parameter als Optionen schreiben

        uno::Sequence < beans::PropertyValue > aProps;
        aAny = xSet->getPropertyValue("PluginCommands");
        aAny >>= aProps;

        SvCommandList aCommands;
        aCommands.FillFromSequence( aProps );
        for( size_t i = 0; i < aCommands.size(); i++ )
        {
            const SvCommand& rCommand = aCommands[ i ];
            const String& rName = rCommand.GetCommand();

            if( SwApplet_Impl::GetOptionType( rName, sal_False ) == SWHTML_OPTTYPE_TAG )
            {
                const String& rValue = rCommand.GetArgument();
                rWrt.Strm() << ' ';
                HTMLOutFuncs::Out_String( rWrt.Strm(), rName, rHTMLWrt.eDestEnc, &rHTMLWrt.aNonConvertableCharacters );
                rWrt.Strm() << "=\"";
                HTMLOutFuncs::Out_String( rWrt.Strm(), rValue, rHTMLWrt.eDestEnc, &rHTMLWrt.aNonConvertableCharacters ) << '\"';
            }
        }
        rHTMLWrt.Strm() << '>';
    }
    else
    {
        // und fuer Floating-Frames einfach noch ein </IFRAME>
        // ausgeben

        rHTMLWrt.Strm() << '>';
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_iframe, sal_False );
    }

    if( !aEndTags.isEmpty() )
        rWrt.Strm() << aEndTags.getStr();

    return rWrt;
}

Writer& OutHTML_FrmFmtOLENodeGrf( Writer& rWrt, const SwFrmFmt& rFrmFmt,
                                  sal_Bool bInCntnr )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    const SwFmtCntnt& rFlyCntnt = rFrmFmt.GetCntnt();
    sal_uLong nStt = rFlyCntnt.GetCntntIdx()->GetIndex()+1;
    SwOLENode *pOLENd = rHTMLWrt.pDoc->GetNodes()[ nStt ]->GetOLENode();

    OSL_ENSURE( pOLENd, "OLE-Node erwartet" );
    if( !pOLENd )
        return rWrt;

    {
        Graphic aGraphic( *pOLENd->GetGraphic() );
        sal_uLong nFlags = bInCntnr ? HTML_FRMOPTS_GENIMG_CNTNR
                                  : HTML_FRMOPTS_GENIMG;
        OutHTML_Image( rWrt, rFrmFmt, aGraphic,
                       pOLENd->GetTitle(), pOLENd->GetTwipSize(),
                       nFlags, pMarkToOLE );
    }

    return rWrt;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
