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
#include <comphelper/string.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <svx/svxids.hrc>
#include <sfx2/sfx.hrc>
#include <i18nlangtag/languagetag.hxx>
#include <svl/stritem.hxx>
#include <svl/urihelper.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <sfx2/docfile.hxx>
#include <svtools/imap.hxx>
#include <svtools/htmltokn.h>
#include <svtools/htmlkywd.hxx>
#include <unotools/eventcfg.hxx>

#include <fmtornt.hxx>
#include <fmturl.hxx>
#include <fmtsrnd.hxx>
#include <fmtinfmt.hxx>
#include <fmtcntnt.hxx>
#include <fmtanchr.hxx>
#include <fmtfsize.hxx>
#include "frmatr.hxx"
#include "charatr.hxx"
#include <frmfmt.hxx>
#include <charfmt.hxx>
#include <docary.hxx>
#include <docsh.hxx>
#include <pam.hxx>
#include <doc.hxx>
#include <ndtxt.hxx>
#include <shellio.hxx>
#include <poolfmt.hxx>
#include <IMark.hxx>
#include <ndgrf.hxx>
#include <htmlnum.hxx>
#include <swcss1.hxx>
#include <swhtml.hxx>
#include <numrule.hxx>

#include <vcl/graphicfilter.hxx>
#include <tools/urlobj.hxx>

using namespace ::com::sun::star;

HTMLOptionEnum aHTMLImgHAlignTable[] =
{
    { OOO_STRING_SVTOOLS_HTML_AL_left,    text::HoriOrientation::LEFT       },
    { OOO_STRING_SVTOOLS_HTML_AL_right,   text::HoriOrientation::RIGHT      },
    { 0,                0               }
};

HTMLOptionEnum aHTMLImgVAlignTable[] =
{
    { OOO_STRING_SVTOOLS_HTML_VA_top,         text::VertOrientation::LINE_TOP       },
    { OOO_STRING_SVTOOLS_HTML_VA_texttop,     text::VertOrientation::CHAR_TOP       },
    { OOO_STRING_SVTOOLS_HTML_VA_middle,      text::VertOrientation::CENTER         },
    { OOO_STRING_SVTOOLS_HTML_AL_center,      text::VertOrientation::CENTER         },
    { OOO_STRING_SVTOOLS_HTML_VA_absmiddle,   text::VertOrientation::LINE_CENTER    },
    { OOO_STRING_SVTOOLS_HTML_VA_bottom,      text::VertOrientation::TOP            },
    { OOO_STRING_SVTOOLS_HTML_VA_baseline,    text::VertOrientation::TOP            },
    { OOO_STRING_SVTOOLS_HTML_VA_absbottom,   text::VertOrientation::LINE_BOTTOM    },
    { 0,                    0                   }
};

ImageMap *SwHTMLParser::FindImageMap( const OUString& rName ) const
{
    OSL_ENSURE( rName[0] != '#', "FindImageMap: name begins with '#'!" );

    if (m_pImageMaps)
    {
        for (auto &rpIMap : *m_pImageMaps)
        {
            if (rName.equalsIgnoreAsciiCase(rpIMap->GetName()))
            {
                return rpIMap.get();
            }
        }
    }
    return nullptr;
}

void SwHTMLParser::ConnectImageMaps()
{
    SwNodes& rNds = pDoc->GetNodes();
    // auf den Start-Node der 1. Section
    sal_uLong nIdx = rNds.GetEndOfAutotext().StartOfSectionIndex() + 1;
    sal_uLong nEndIdx = rNds.GetEndOfAutotext().GetIndex();

    SwGrfNode* pGrfNd;
    while( nMissingImgMaps > 0 && nIdx < nEndIdx )
    {
        SwNode *pNd = rNds[nIdx + 1];
        if( 0 != (pGrfNd = pNd->GetGrfNode()) )
        {
            SwFrameFormat *pFormat = pGrfNd->GetFlyFormat();
            SwFormatURL aURL( pFormat->GetURL() );
            const ImageMap *pIMap = aURL.GetMap();
            if( pIMap && pIMap->GetIMapObjectCount()==0 )
            {
                // Die (leere) Image-Map des Nodes wird entweder
                // durch die jetzt gefundene Image-Map ersetzt
                // oder geloescht.
                ImageMap *pNewIMap =
                    FindImageMap( pIMap->GetName() );
                aURL.SetMap( pNewIMap );
                pFormat->SetFormatAttr( aURL );
                if( !pGrfNd->IsScaleImageMap() )
                {
                    // die Grafikgroesse ist mitlerweile da oder dir
                    // Grafik muss nicht skaliert werden
                    pGrfNd->ScaleImageMap();
                }
                nMissingImgMaps--;  // eine Map weniger suchen
            }
        }
        nIdx = rNds[nIdx]->EndOfSectionIndex() + 1;
    }
}

/*  */

void SwHTMLParser::SetAnchorAndAdjustment( sal_Int16 eVertOri,
                                           sal_Int16 eHoriOri,
                                           const SfxItemSet &rCSS1ItemSet,
                                           const SvxCSS1PropertyInfo &rCSS1PropInfo,
                                           SfxItemSet& rFrmItemSet )
{
    const SfxItemSet *pCntnrItemSet = 0;
    auto i = aContexts.size();
    while( !pCntnrItemSet && i > nContextStMin )
        pCntnrItemSet = aContexts[--i]->GetFrmItemSet();

    if( pCntnrItemSet )
    {
        // Wenn wir und in einem Container befinden wird die Verankerung
        // des Containers uebernommen.
        rFrmItemSet.Put( *pCntnrItemSet );
    }
    else if( SwCSS1Parser::MayBePositioned( rCSS1PropInfo, true ) )
    {
        // Wenn die Ausrichtung anhand der CSS1-Optionen gesetzt werden kann
        // werden die benutzt.
        SetAnchorAndAdjustment( rCSS1ItemSet, rCSS1PropInfo, rFrmItemSet );
    }
    else
    {
        // Sonst wird die Ausrichtung entsprechend der normalen HTML-Optionen
        // gesetzt.
        SetAnchorAndAdjustment( eVertOri, eHoriOri, rFrmItemSet );
    }
}

void SwHTMLParser::SetAnchorAndAdjustment( sal_Int16 eVertOri,
                                           sal_Int16 eHoriOri,
                                           SfxItemSet& rFrmSet,
                                           bool bDontAppend )
{
    bool bMoveBackward = false;
    SwFormatAnchor aAnchor( FLY_AS_CHAR );
    sal_Int16 eVertRel = text::RelOrientation::FRAME;

    if( text::HoriOrientation::NONE != eHoriOri )
    {
        // den Absatz-Einzug bestimmen
        sal_uInt16 nLeftSpace = 0, nRightSpace = 0;
        short nIndent = 0;
        GetMarginsFromContextWithNumBul( nLeftSpace, nRightSpace, nIndent );

        // Horizonale Ausrichtung und Umlauf bestimmen.
        sal_Int16 eHoriRel;
        SwSurround eSurround;
        switch( eHoriOri )
        {
        case text::HoriOrientation::LEFT:
            eHoriRel = nLeftSpace ? text::RelOrientation::PRINT_AREA : text::RelOrientation::FRAME;
            eSurround = SURROUND_RIGHT;
            break;
        case text::HoriOrientation::RIGHT:
            eHoriRel = nRightSpace ? text::RelOrientation::PRINT_AREA : text::RelOrientation::FRAME;
            eSurround = SURROUND_LEFT;
            break;
        case text::HoriOrientation::CENTER:   // fuer Tabellen
            eHoriRel = text::RelOrientation::FRAME;
            eSurround = SURROUND_NONE;
            break;
        default:
            eHoriRel = text::RelOrientation::FRAME;
            eSurround = SURROUND_PARALLEL;
            break;
        }

        // Einen neuen Absatz aufmachen, wenn der aktuelle
        // absatzgebundene Rahmen ohne Umlauf enthaelt.
        if( !bDontAppend && HasCurrentParaFlys( true ) )
        {
            // Wenn der Absatz nur Grafiken enthaelt, braucht er
            // auch keinen unteren Absatz-Abstand. Da hier auch bei
            // Verwendung von Styles kein Abstand enstehen soll, wird
            // hier auch geweohnlich attributiert !!!
            sal_uInt16 nUpper=0, nLower=0;
            GetULSpaceFromContext( nUpper, nLower );
            InsertAttr( SvxULSpaceItem( nUpper, 0, RES_UL_SPACE ), false, true );

            AppendTextNode( AM_NOSPACE );

            if( nUpper )
            {
                NewAttr( &aAttrTab.pULSpace, SvxULSpaceItem( 0, nLower, RES_UL_SPACE ) );
                aParaAttrs.push_back( aAttrTab.pULSpace );
                EndAttr( aAttrTab.pULSpace, 0, false );
            }
        }

        // Vertikale Ausrichtung und Verankerung bestimmen.
        const sal_Int32 nContent = pPam->GetPoint()->nContent.GetIndex();
        if( nContent )
        {
            aAnchor.SetType( FLY_AT_CHAR );
            bMoveBackward = true;
            eVertOri = text::VertOrientation::CHAR_BOTTOM;
            eVertRel = text::RelOrientation::CHAR;
        }
        else
        {
            aAnchor.SetType( FLY_AT_PARA );
            eVertOri = text::VertOrientation::TOP;
            eVertRel = text::RelOrientation::PRINT_AREA;
        }

        rFrmSet.Put( SwFormatHoriOrient( 0, eHoriOri, eHoriRel) );

        rFrmSet.Put( SwFormatSurround( eSurround ) );
    }
    rFrmSet.Put( SwFormatVertOrient( 0, eVertOri, eVertRel) );

    if( bMoveBackward )
        pPam->Move( fnMoveBackward );

    aAnchor.SetAnchor( pPam->GetPoint() );

    if( bMoveBackward )
        pPam->Move( fnMoveForward );

    rFrmSet.Put( aAnchor );
}

void SwHTMLParser::RegisterFlyFrm( SwFrameFormat *pFlyFormat )
{
    // automatisch verankerte Rahmen muessen noch um eine Position
    // nach vorne verschoben werden.
    if( RES_DRAWFRMFMT != pFlyFormat->Which() &&
        (FLY_AT_PARA == pFlyFormat->GetAnchor().GetAnchorId()) &&
        SURROUND_THROUGHT == pFlyFormat->GetSurround().GetSurround() )
    {
        aMoveFlyFrms.push_back( pFlyFormat );
        aMoveFlyCnts.push_back( pPam->GetPoint()->nContent.GetIndex() );
    }
}

/*  */

void SwHTMLParser::GetDefaultScriptType( ScriptType& rType,
                                         OUString& rTypeStr ) const
{
    SwDocShell *pDocSh = pDoc->GetDocShell();
    SvKeyValueIterator* pHeaderAttrs = pDocSh ? pDocSh->GetHeaderAttributes()
                                              : 0;
    rType = GetScriptType( pHeaderAttrs );
    rTypeStr = GetScriptTypeString( pHeaderAttrs );
}

/*  */

void SwHTMLParser::InsertImage()
{
    // und jetzt auswerten
    OUString sAltNm, aId, aClass, aStyle, aMap, sHTMLGrfName;
    OUString sGrfNm;
    sal_Int16 eVertOri = text::VertOrientation::TOP;
    sal_Int16 eHoriOri = text::HoriOrientation::NONE;
    bool bWidthProvided=false, bHeightProvided=false;
    long nWidth=0, nHeight=0;
    long nVSpace=0, nHSpace=0;

    sal_uInt16 nBorder = (aAttrTab.pINetFormat ? 1 : 0);
    bool bIsMap = false;
    bool bPrcWidth = false;
    bool bPrcHeight = false;
    SvxMacroItem aMacroItem(RES_FRMMACRO);

    ScriptType eDfltScriptType;
    OUString sDfltScriptType;
    GetDefaultScriptType( eDfltScriptType, sDfltScriptType );

    const HTMLOptions& rHTMLOptions = GetOptions();
    for (size_t i = rHTMLOptions.size(); i; )
    {
        sal_uInt16 nEvent = 0;
        ScriptType eScriptType2 = eDfltScriptType;
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
            case HTML_O_SRC:
                sGrfNm = rOption.GetString();
                if( !InternalImgToPrivateURL(sGrfNm) )
                    sGrfNm = INetURLObject::GetAbsURL( sBaseURL, sGrfNm );
                break;
            case HTML_O_ALIGN:
                eVertOri =
                    rOption.GetEnum( aHTMLImgVAlignTable,
                                                    text::VertOrientation::TOP );
                eHoriOri =
                    rOption.GetEnum( aHTMLImgHAlignTable );
                break;
            case HTML_O_WIDTH:
                // erstmal nur als Pixelwerte merken!
                nWidth = rOption.GetNumber();
                bPrcWidth = (rOption.GetString().indexOf('%') != -1);
                if( bPrcWidth && nWidth>100 )
                    nWidth = 100;
                bWidthProvided = true;
                break;
            case HTML_O_HEIGHT:
                // erstmal nur als Pixelwerte merken!
                nHeight = rOption.GetNumber();
                bPrcHeight = (rOption.GetString().indexOf('%') != -1);
                if( bPrcHeight && nHeight>100 )
                    nHeight = 100;
                bHeightProvided = true;
                break;
            case HTML_O_VSPACE:
                nVSpace = rOption.GetNumber();
                break;
            case HTML_O_HSPACE:
                nHSpace = rOption.GetNumber();
                break;
            case HTML_O_ALT:
                sAltNm = rOption.GetString();
                break;
            case HTML_O_BORDER:
                nBorder = (sal_uInt16)rOption.GetNumber();
                break;
            case HTML_O_ISMAP:
                bIsMap = true;
                break;
            case HTML_O_USEMAP:
                aMap = rOption.GetString();
                break;
            case HTML_O_NAME:
                sHTMLGrfName = rOption.GetString();
                break;

            case HTML_O_SDONLOAD:
                eScriptType2 = STARBASIC;
                //fallthrough
            case HTML_O_ONLOAD:
                nEvent = SVX_EVENT_IMAGE_LOAD;
                goto IMAGE_SETEVENT;

            case HTML_O_SDONABORT:
                eScriptType2 = STARBASIC;
                //fallthrough
            case HTML_O_ONABORT:
                nEvent = SVX_EVENT_IMAGE_ABORT;
                goto IMAGE_SETEVENT;

            case HTML_O_SDONERROR:
                eScriptType2 = STARBASIC;
                //fallthrough
            case HTML_O_ONERROR:
                nEvent = SVX_EVENT_IMAGE_ERROR;
                goto IMAGE_SETEVENT;
IMAGE_SETEVENT:
                {
                    OUString sTmp( rOption.GetString() );
                    if( !sTmp.isEmpty() )
                    {
                        sTmp = convertLineEnd(sTmp, GetSystemLineEnd());
                        OUString sScriptType;
                        if( EXTENDED_STYPE == eScriptType2 )
                            sScriptType = sDfltScriptType;
                        aMacroItem.SetMacro( nEvent,
                            SvxMacro( sTmp, sScriptType, eScriptType2 ));
                    }
                }
                break;
        }
    }

    if( sGrfNm.isEmpty() )
        return;

    // Wenn wir in einer Numerierung stehen und der Absatz noch leer und
    // nicht numeriert ist, handelt es sich vielleicht um die Grafik
    // einer Bullet-Liste
    if( !pPam->GetPoint()->nContent.GetIndex() &&
        GetNumInfo().GetDepth() > 0 && GetNumInfo().GetDepth() <= MAXLEVEL &&
        !aBulletGrfs[GetNumInfo().GetDepth()-1].isEmpty() &&
        aBulletGrfs[GetNumInfo().GetDepth()-1]==sGrfNm )
    {
        SwTextNode* pTextNode = pPam->GetNode().GetTextNode();

        if( pTextNode && ! pTextNode->IsCountedInList())
        {
            OSL_ENSURE( pTextNode->GetActualListLevel() == GetNumInfo().GetLevel(),
                    "Numbering level is wrong" );

            pTextNode->SetCountedInList( true );

            // Rule invalisieren ist noetig, weil zwischem dem einlesen
            // des LI und der Grafik ein EndAction gerufen worden sein kann.
            if( GetNumInfo().GetNumRule() )
                GetNumInfo().GetNumRule()->SetInvalidRule( true );

            // Die Vorlage novh mal setzen. Ist noetig, damit der
            // Erstzeilen-Einzug stimmt.
            SetTextCollAttrs();

            return;
        }
    }

    Graphic aGraphic;
    INetURLObject aGraphicURL( sGrfNm );
    if( aGraphicURL.GetProtocol() == INetProtocol::Data )
    {
        std::unique_ptr<SvMemoryStream> const pStream(aGraphicURL.getData());
        if (pStream)
        {
            if (GRFILTER_OK == GraphicFilter::GetGraphicFilter().ImportGraphic(aGraphic, "", *pStream))
                sGrfNm.clear();
        }
    }
    else if (sBaseURL.isEmpty()) // sBaseURL is empty if the source is clipboard
    {
        if (GRFILTER_OK == GraphicFilter::GetGraphicFilter().ImportGraphic(aGraphic, aGraphicURL))
            sGrfNm.clear();
    }

    if (!sGrfNm.isEmpty())
    {
        aGraphic.SetDefaultType();
    }

    if (!bHeightProvided || !bWidthProvided)
    {
        Size aPixelSize = aGraphic.GetSizePixel(Application::GetDefaultDevice());
        if (!bWidthProvided)
            nWidth = aPixelSize.Width();
        if (!bHeightProvided)
            nHeight = aPixelSize.Height();
    }

    SfxItemSet aItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
    SvxCSS1PropertyInfo aPropInfo;
    if( HasStyleOptions( aStyle, aId, aClass ) )
        ParseStyleOptions( aStyle, aId, aClass, aItemSet, aPropInfo );

    SfxItemSet aFrmSet( pDoc->GetAttrPool(),
                        RES_FRMATR_BEGIN, RES_FRMATR_END-1 );
    if( !IsNewDoc() )
        Reader::ResetFrameFormatAttrs( aFrmSet );

    // Umrandung setzen
    long nHBorderWidth = 0, nVBorderWidth = 0;
    if( nBorder )
    {
        nHBorderWidth = (long)nBorder;
        nVBorderWidth = (long)nBorder;
        SvxCSS1Parser::PixelToTwip( nVBorderWidth, nHBorderWidth );

        ::editeng::SvxBorderLine aHBorderLine( NULL, nHBorderWidth );
        ::editeng::SvxBorderLine aVBorderLine( NULL, nVBorderWidth );

        if( aAttrTab.pINetFormat )
        {
            const OUString& rURL =
                static_cast<const SwFormatINetFormat&>(aAttrTab.pINetFormat->GetItem()).GetValue();

            pCSS1Parser->SetATagStyles();
            sal_uInt16 nPoolId =  static_cast< sal_uInt16 >(pDoc->IsVisitedURL( rURL )
                                    ? RES_POOLCHR_INET_VISIT
                                    : RES_POOLCHR_INET_NORMAL);
            const SwCharFormat *pCharFormat = pCSS1Parser->GetCharFormatFromPool( nPoolId );
            aHBorderLine.SetColor( pCharFormat->GetColor().GetValue() );
            aVBorderLine.SetColor( aHBorderLine.GetColor() );
        }
        else
        {
            const SvxColorItem& rColorItem = aAttrTab.pFontColor ?
              static_cast<const SvxColorItem &>(aAttrTab.pFontColor->GetItem()) :
              static_cast<const SvxColorItem &>(pDoc->GetDefault(RES_CHRATR_COLOR));
            aHBorderLine.SetColor( rColorItem.GetValue() );
            aVBorderLine.SetColor( aHBorderLine.GetColor() );
        }

        SvxBoxItem aBoxItem( RES_BOX );
        aBoxItem.SetLine( &aHBorderLine, SvxBoxItemLine::TOP );
        aBoxItem.SetLine( &aHBorderLine, SvxBoxItemLine::BOTTOM );
        aBoxItem.SetLine( &aVBorderLine, SvxBoxItemLine::LEFT );
        aBoxItem.SetLine( &aVBorderLine, SvxBoxItemLine::RIGHT );
        aFrmSet.Put( aBoxItem );
    }

    // Ausrichtung setzen
    SetAnchorAndAdjustment( eVertOri, eHoriOri, aItemSet, aPropInfo, aFrmSet );

    // Abstaende setzen
    SetSpace( Size( nHSpace, nVSpace), aItemSet, aPropInfo, aFrmSet );

    // Sonstige CSS1-Attribute Setzen
    SetFrameFormatAttrs( aItemSet, aPropInfo, HTML_FF_BOX, aFrmSet );

    Size aTwipSz( bPrcWidth ? 0 : nWidth, bPrcHeight ? 0 : nHeight );
    if( (aTwipSz.Width() || aTwipSz.Height()) && Application::GetDefaultDevice() )
    {
        aTwipSz = Application::GetDefaultDevice()
                    ->PixelToLogic( aTwipSz, MapMode( MAP_TWIP ) );
    }

    // CSS1-Groesse auf "normale" Groesse umrechnen
    switch( aPropInfo.eWidthType )
    {
        case SVX_CSS1_LTYPE_TWIP:
            aTwipSz.Width() = aPropInfo.nWidth;
            nWidth = 1; // != 0
            bPrcWidth = false;
            break;
        case SVX_CSS1_LTYPE_PERCENTAGE:
            aTwipSz.Width() = 0;
            nWidth = aPropInfo.nWidth;
            bPrcWidth = true;
            break;
        default:
            ;
    }
    switch( aPropInfo.eHeightType )
    {
        case SVX_CSS1_LTYPE_TWIP:
            aTwipSz.Height() = aPropInfo.nHeight;
            nHeight = 1;    // != 0
            bPrcHeight = false;
            break;
        case SVX_CSS1_LTYPE_PERCENTAGE:
            aTwipSz.Height() = 0;
            nHeight = aPropInfo.nHeight;
            bPrcHeight = true;
            break;
        default:
            ;
    }

    Size aGrfSz( 0, 0 );
    bool bSetTwipSize = true;       // Twip-Size am Node setzen?
    bool bChangeFrmSize = false;    // Frame-Format nachtraeglich anpassen?
    bool bRequestGrfNow = false;
    bool bSetScaleImageMap = false;
    sal_uInt8 nPrcWidth = 0, nPrcHeight = 0;

    if( !nWidth || !nHeight )
    {
        // Es fehlt die Breite oder die Hoehe
        // Wenn die Grfik in einer Tabelle steht, wird sie gleich
        // angefordert, damit sie eventuell schon da ist, bevor die
        // Tabelle layoutet wird.
        if( pTable!=0 && !nWidth )
        {
            bRequestGrfNow = true;
            IncGrfsThatResizeTable();
        }

        // Die Groesse des Rahmens wird nachtraeglich gesetzt
        bChangeFrmSize = true;
        aGrfSz = aTwipSz;
        if( !nWidth && !nHeight )
        {
            aTwipSz.Width() = HTML_DFLT_IMG_WIDTH;
            aTwipSz.Height() = HTML_DFLT_IMG_HEIGHT;
        }
        else if( nWidth )
        {
            // eine %-Angabe
            if( bPrcWidth )
            {
                nPrcWidth = (sal_uInt8)nWidth;
                nPrcHeight = 255;
            }
            else
            {
                aTwipSz.Height() = HTML_DFLT_IMG_HEIGHT;
            }
        }
        else if( nHeight )
        {
            if( bPrcHeight )
            {
                nPrcHeight = (sal_uInt8)nHeight;
                nPrcWidth = 255;
            }
            else
            {
                aTwipSz.Width() = HTML_DFLT_IMG_WIDTH;
            }
        }
    }
    else
    {
        // Breite und Hoehe wurden angegeben und brauchen nicht gesetzt
        // zu werden
        bSetTwipSize = false;

        if( bPrcWidth )
            nPrcWidth = (sal_uInt8)nWidth;

        if( bPrcHeight )
            nPrcHeight = (sal_uInt8)nHeight;
    }

    // Image-Map setzen
    aMap = comphelper::string::stripEnd(aMap, ' ');
    if( !aMap.isEmpty() )
    {
        // Da wir nur lokale Image-Maps kennen nehmen wireinfach alles
        // hinter dem # als Namen
        sal_Int32 nPos = aMap.indexOf( '#' );
        OUString aName;
        if ( -1 == nPos )
            aName = aMap ;
        else
            aName = aMap.copy(nPos+1);

        ImageMap *pImgMap = FindImageMap( aName );
        if( pImgMap )
        {
            SwFormatURL aURL; aURL.SetMap( pImgMap );//wird kopieiert

            bSetScaleImageMap = !nPrcWidth || !nPrcHeight;
            aFrmSet.Put( aURL );
        }
        else
        {
            ImageMap aEmptyImgMap( aName );
            SwFormatURL aURL; aURL.SetMap( &aEmptyImgMap );//wird kopieiert
            aFrmSet.Put( aURL );
            nMissingImgMaps++;          // es fehlen noch Image-Maps

            // die Grafik muss beim SetTwipSize skaliert werden, wenn
            // wir keine Groesse am Node gesetzt haben oder die Groesse
            // nicht der Grafikgroesse entsprach.
            bSetScaleImageMap = true;
        }
    }

    // min. Werte einhalten !!
    if( nPrcWidth )
    {
        OSL_ENSURE( !aTwipSz.Width(),
                "Wieso ist da trotz %-Angabe eine Breite gesetzt?" );
        aTwipSz.Width() = aGrfSz.Width() ? aGrfSz.Width()
                                         : HTML_DFLT_IMG_WIDTH;
    }
    else
    {
        aTwipSz.Width() += 2*nVBorderWidth;
        if( aTwipSz.Width() < MINFLY )
            aTwipSz.Width() = MINFLY;
    }
    if( nPrcHeight )
    {
        OSL_ENSURE( !aTwipSz.Height(),
                "Wieso ist da trotz %-Angabe eine Hoehe gesetzt?" );
        aTwipSz.Height() = aGrfSz.Height() ? aGrfSz.Height()
                                           : HTML_DFLT_IMG_HEIGHT;
    }
    else
    {
        aTwipSz.Height() += 2*nHBorderWidth;
        if( aTwipSz.Height() < MINFLY )
            aTwipSz.Height() = MINFLY;
    }

    SwFormatFrmSize aFrmSize( ATT_FIX_SIZE, aTwipSz.Width(), aTwipSz.Height() );
    aFrmSize.SetWidthPercent( nPrcWidth );
    aFrmSize.SetHeightPercent( nPrcHeight );
    aFrmSet.Put( aFrmSize );

    // passing empty sGrfNm here, means we don't want the graphic to be linked
    SwFrameFormat *pFlyFormat = pDoc->getIDocumentContentOperations().Insert( *pPam, sGrfNm, aEmptyOUStr, &aGraphic,
                                      &aFrmSet, NULL, NULL );
    SwGrfNode *pGrfNd = pDoc->GetNodes()[ pFlyFormat->GetContent().GetContentIdx()
                                  ->GetIndex()+1 ]->GetGrfNode();

    if( !sHTMLGrfName.isEmpty() )
    {
        pFlyFormat->SetName( sHTMLGrfName );

        // ggfs. eine Grafik anspringen
        if( JUMPTO_GRAPHIC == eJumpTo && sHTMLGrfName == sJmpMark )
        {
            bChkJumpMark = true;
            eJumpTo = JUMPTO_NONE;
        }
    }

    if (pGrfNd)
    {
        if( !sAltNm.isEmpty() )
            pGrfNd->SetTitle( sAltNm );

        if( bSetTwipSize )
            pGrfNd->SetTwipSize( aGrfSz );

        pGrfNd->SetChgTwipSize( bChangeFrmSize );

        if( bSetScaleImageMap )
            pGrfNd->SetScaleImageMap( true );
    }

    if( aAttrTab.pINetFormat )
    {
        const SwFormatINetFormat &rINetFormat =
            static_cast<const SwFormatINetFormat&>(aAttrTab.pINetFormat->GetItem());

        SwFormatURL aURL( pFlyFormat->GetURL() );

        aURL.SetURL( rINetFormat.GetValue(), bIsMap );
        aURL.SetTargetFrameName( rINetFormat.GetTargetFrame() );
        aURL.SetName( rINetFormat.GetName() );
        pFlyFormat->SetFormatAttr( aURL );

        {
            static const sal_uInt16 aEvents[] = {
                SFX_EVENT_MOUSEOVER_OBJECT,
                SFX_EVENT_MOUSECLICK_OBJECT,
                SFX_EVENT_MOUSEOUT_OBJECT,
                0 };

            for( int n = 0; aEvents[ n ]; ++n )
            {
                const SvxMacro *pMacro = rINetFormat.GetMacro( aEvents[ n ] );
                if( 0 != pMacro )
                    aMacroItem.SetMacro( aEvents[ n ], *pMacro );
            }
        }

        if ((FLY_AS_CHAR == pFlyFormat->GetAnchor().GetAnchorId()) &&
            aAttrTab.pINetFormat->GetSttPara() ==
                        pPam->GetPoint()->nNode &&
            aAttrTab.pINetFormat->GetSttCnt() ==
                        pPam->GetPoint()->nContent.GetIndex() - 1 )
        {
            // das Attribut wurde unmitellbar vor einer zeichengeb.
            // Grafik eingefuegt, also verschieben wir es
            aAttrTab.pINetFormat->SetStart( *pPam->GetPoint() );

            // Wenn das Attribut auch ein Sprungziel ist, fuegen
            // wir noch eine Bookmark vor der Grafik ein, weil das
            // SwFormatURL kein Sprungziel ist.
            if( !rINetFormat.GetName().isEmpty() )
            {
                pPam->Move( fnMoveBackward );
                InsertBookmark( rINetFormat.GetName() );
                pPam->Move( fnMoveForward );
            }
        }

    }

    if( !aMacroItem.GetMacroTable().empty() )
        pFlyFormat->SetFormatAttr( aMacroItem );

    // tdf#87083 If the graphic has not been loaded yet, then load it now.
    // Otherwise it may be loaded during the first paint of the object and it
    // will be too late to adapt the size of the graphic at that point.
    if (bRequestGrfNow && pGrfNd)
    {
        Size aUpdatedSize = pGrfNd->GetTwipSize();  //trigger a swap-in
        SAL_WARN_IF(!aUpdatedSize.Width() || !aUpdatedSize.Height(), "sw.html", "html image with no width or height");
    }

    // Ggf. Frames anlegen und Auto-gebundenen Rahmen registrieren
    RegisterFlyFrm( pFlyFormat );

    if( !aId.isEmpty() )
        InsertBookmark( aId );
}

/*  */

void SwHTMLParser::InsertBodyOptions()
{
    pDoc->SetTextFormatColl( *pPam,
                         pCSS1Parser->GetTextCollFromPool( RES_POOLCOLL_TEXT ) );

    OUString aBackGround, aId, aStyle, aLang, aDir;
    Color aBGColor, aTextColor, aLinkColor, aVLinkColor;
    bool bBGColor=false, bTextColor=false;
    bool bLinkColor=false, bVLinkColor=false;

    ScriptType eDfltScriptType;
    OUString sDfltScriptType;
    GetDefaultScriptType( eDfltScriptType, sDfltScriptType );

    const HTMLOptions& rHTMLOptions = GetOptions();
    for (size_t i = rHTMLOptions.size(); i; )
    {
        const HTMLOption& rOption = rHTMLOptions[--i];
        ScriptType eScriptType2 = eDfltScriptType;
        OUString aEvent;
        bool bSetEvent = false;

        switch( rOption.GetToken() )
        {
            case HTML_O_ID:
                aId = rOption.GetString();
                break;
            case HTML_O_BACKGROUND:
                aBackGround = rOption.GetString();
                break;
            case HTML_O_BGCOLOR:
                rOption.GetColor( aBGColor );
                bBGColor = true;
                break;
            case HTML_O_TEXT:
                rOption.GetColor( aTextColor );
                bTextColor = true;
                break;
            case HTML_O_LINK:
                rOption.GetColor( aLinkColor );
                bLinkColor = true;
                break;
            case HTML_O_VLINK:
                rOption.GetColor( aVLinkColor );
                bVLinkColor = true;
                break;

            case HTML_O_SDONLOAD:
                eScriptType2 = STARBASIC;
                //fallthrough
            case HTML_O_ONLOAD:
                aEvent = GlobalEventConfig::GetEventName( GlobalEventId::OPENDOC );
                bSetEvent = true;
                break;

            case HTML_O_SDONUNLOAD:
                eScriptType2 = STARBASIC;
                //fallthrough
            case HTML_O_ONUNLOAD:
                aEvent = GlobalEventConfig::GetEventName( GlobalEventId::PREPARECLOSEDOC );
                bSetEvent = true;
                break;

            case HTML_O_SDONFOCUS:
                eScriptType2 = STARBASIC;
                //fallthrough
            case HTML_O_ONFOCUS:
                aEvent = GlobalEventConfig::GetEventName( GlobalEventId::ACTIVATEDOC );
                bSetEvent = true;
                break;

            case HTML_O_SDONBLUR:
                eScriptType2 = STARBASIC;
                //fallthrough
            case HTML_O_ONBLUR:
                aEvent = GlobalEventConfig::GetEventName( GlobalEventId::DEACTIVATEDOC );
                bSetEvent = true;
                break;

            case HTML_O_ONERROR:
                break;

            case HTML_O_STYLE:
                aStyle = rOption.GetString();
                bTextColor = true;
                break;
            case HTML_O_LANG:
                aLang = rOption.GetString();
                break;
            case HTML_O_DIR:
                aDir = rOption.GetString();
                break;
        }

        if( bSetEvent )
        {
            const OUString& rEvent = rOption.GetString();
            if( !rEvent.isEmpty() )
                InsertBasicDocEvent( aEvent, rEvent, eScriptType2,
                                     sDfltScriptType );
        }
    }

    if( bTextColor && !pCSS1Parser->IsBodyTextSet() )
    {
        // Die Textfarbe wird an der Standard-Vorlage gesetzt
        pCSS1Parser->GetTextCollFromPool( RES_POOLCOLL_STANDARD )
            ->SetFormatAttr( SvxColorItem(aTextColor, RES_CHRATR_COLOR) );
        pCSS1Parser->SetBodyTextSet();
    }

    // Die Item fuer die Seitenvorlage vorbereiten (Hintergrund, Umrandung)
    // Beim BrushItem muessen schon gesetzte werte erhalten bleiben!
    SvxBrushItem aBrushItem( pCSS1Parser->makePageDescBackground() );
    bool bSetBrush = false;

    if( bBGColor && !pCSS1Parser->IsBodyBGColorSet() )
    {
        // Hintergrundfarbe aus "BGCOLOR"
        OUString aLink;
        if( !aBrushItem.GetGraphicLink().isEmpty() )
            aLink = aBrushItem.GetGraphicLink();
        SvxGraphicPosition ePos = aBrushItem.GetGraphicPos();

        aBrushItem.SetColor( aBGColor );

        if( !aLink.isEmpty() )
        {
            aBrushItem.SetGraphicLink( aLink );
            aBrushItem.SetGraphicPos( ePos );
        }
        bSetBrush = true;
        pCSS1Parser->SetBodyBGColorSet();
    }

    if( !aBackGround.isEmpty() && !pCSS1Parser->IsBodyBackgroundSet() )
    {
        // Hintergrundgrafik aus "BACKGROUND"
        aBrushItem.SetGraphicLink( INetURLObject::GetAbsURL( sBaseURL, aBackGround ) );
        aBrushItem.SetGraphicPos( GPOS_TILED );
        bSetBrush = true;
        pCSS1Parser->SetBodyBackgroundSet();
    }

    if( !aStyle.isEmpty() || !aDir.isEmpty() )
    {
        SfxItemSet aItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;
        OUString aDummy;
        ParseStyleOptions( aStyle, aDummy, aDummy, aItemSet, aPropInfo, 0, &aDir );

        // Ein par Attribute muessen an der Seitenvorlage gesetzt werden,
        // und zwar die, die nicht vererbit werden
        pCSS1Parser->SetPageDescAttrs( bSetBrush ? &aBrushItem : 0,
                                       &aItemSet );

        const SfxPoolItem *pItem;
        static const sal_uInt16 aWhichIds[3] = { RES_CHRATR_FONTSIZE,
                                       RES_CHRATR_CJK_FONTSIZE,
                                       RES_CHRATR_CTL_FONTSIZE };
        for( size_t i=0; i<SAL_N_ELEMENTS(aWhichIds); ++i )
        {
            if( SfxItemState::SET == aItemSet.GetItemState( aWhichIds[i], false,
                                                       &pItem ) &&
                static_cast <const SvxFontHeightItem * >(pItem)->GetProp() != 100)
            {
                sal_uInt32 nHeight =
                    ( aFontHeights[2] *
                     static_cast <const SvxFontHeightItem * >(pItem)->GetProp() ) / 100;
                SvxFontHeightItem aNewItem( nHeight, 100, aWhichIds[i] );
                aItemSet.Put( aNewItem );
            }
        }

        // alle noch uebrigen Optionen koennen an der Standard-Vorlage
        // gesetzt werden und gelten dann automatisch als defaults
        pCSS1Parser->GetTextCollFromPool( RES_POOLCOLL_STANDARD )
            ->SetFormatAttr( aItemSet );
    }
    else if( bSetBrush )
    {
        pCSS1Parser->SetPageDescAttrs( &aBrushItem );
    }

    if( bLinkColor && !pCSS1Parser->IsBodyLinkSet() )
    {
        SwCharFormat *pCharFormat =
            pCSS1Parser->GetCharFormatFromPool(RES_POOLCHR_INET_NORMAL);
        pCharFormat->SetFormatAttr( SvxColorItem(aLinkColor, RES_CHRATR_COLOR) );
        pCSS1Parser->SetBodyLinkSet();
    }
    if( bVLinkColor && !pCSS1Parser->IsBodyVLinkSet() )
    {
        SwCharFormat *pCharFormat =
            pCSS1Parser->GetCharFormatFromPool(RES_POOLCHR_INET_VISIT);
        pCharFormat->SetFormatAttr( SvxColorItem(aVLinkColor, RES_CHRATR_COLOR) );
        pCSS1Parser->SetBodyVLinkSet();
    }
    if( !aLang.isEmpty() )
    {
        LanguageType eLang = LanguageTag::convertToLanguageTypeWithFallback( aLang );
        if( LANGUAGE_DONTKNOW != eLang )
        {
            sal_uInt16 nWhich = 0;
            switch( SvtLanguageOptions::GetScriptTypeOfLanguage( eLang ) )
            {
            case SvtScriptType::LATIN:
                nWhich = RES_CHRATR_LANGUAGE;
                break;
            case SvtScriptType::ASIAN:
                nWhich = RES_CHRATR_CJK_LANGUAGE;
                break;
            case SvtScriptType::COMPLEX:
                nWhich = RES_CHRATR_CTL_LANGUAGE;
                break;
            default: break;
            }
            if( nWhich )
            {
                SvxLanguageItem aLanguage( eLang, nWhich );
                aLanguage.SetWhich( nWhich );
                pDoc->SetDefault( aLanguage );
            }
        }
    }

    if( !aId.isEmpty() )
        InsertBookmark( aId );
}

/*  */

void SwHTMLParser::NewAnchor()
{
    // den voherigen Link beenden, falls es einen gab
    _HTMLAttrContext *pOldCntxt = PopContext( HTML_ANCHOR_ON );
    if( pOldCntxt )
    {
        // und ggf. die Attribute beenden
        EndContext( pOldCntxt );
        delete pOldCntxt;
    }

    SvxMacroTableDtor aMacroTable;
    OUString sHRef, aName, sTarget;
    OUString aId, aStyle, aClass, aLang, aDir;
    bool bHasHRef = false, bFixed = false;

    ScriptType eDfltScriptType;
    OUString sDfltScriptType;
    GetDefaultScriptType( eDfltScriptType, sDfltScriptType );

    const HTMLOptions& rHTMLOptions = GetOptions();
    for (size_t i = rHTMLOptions.size(); i; )
    {
        sal_uInt16 nEvent = 0;
        ScriptType eScriptType2 = eDfltScriptType;
        const HTMLOption& rOption = rHTMLOptions[--i];
        switch( rOption.GetToken() )
        {
            case HTML_O_NAME:
                aName = rOption.GetString();
                break;

            case HTML_O_HREF:
                sHRef = rOption.GetString();
                bHasHRef = true;
                break;
            case HTML_O_TARGET:
                sTarget = rOption.GetString();
                break;

            case HTML_O_STYLE:
                aStyle = rOption.GetString();
                break;
            case HTML_O_ID:
                aId = rOption.GetString();
                break;
            case HTML_O_CLASS:
                aClass = rOption.GetString();
                break;
            case HTML_O_SDFIXED:
                bFixed = true;
                break;
            case HTML_O_LANG:
                aLang = rOption.GetString();
                break;
            case HTML_O_DIR:
                aDir = rOption.GetString();
                break;

            case HTML_O_SDONCLICK:
                eScriptType2 = STARBASIC;
                //fall-through
            case HTML_O_ONCLICK:
                nEvent = SFX_EVENT_MOUSECLICK_OBJECT;
                goto ANCHOR_SETEVENT;

            case HTML_O_SDONMOUSEOVER:
                eScriptType2 = STARBASIC;
                //fall-through
            case HTML_O_ONMOUSEOVER:
                nEvent = SFX_EVENT_MOUSEOVER_OBJECT;
                goto ANCHOR_SETEVENT;

            case HTML_O_SDONMOUSEOUT:
                eScriptType2 = STARBASIC;
                //fall-through
            case HTML_O_ONMOUSEOUT:
                nEvent = SFX_EVENT_MOUSEOUT_OBJECT;
                goto ANCHOR_SETEVENT;
ANCHOR_SETEVENT:
                {
                    OUString sTmp( rOption.GetString() );
                    if( !sTmp.isEmpty() )
                    {
                        sTmp = convertLineEnd(sTmp, GetSystemLineEnd());
                        OUString sScriptType;
                        if( EXTENDED_STYPE == eScriptType2 )
                            sScriptType = sDfltScriptType;
                        aMacroTable.Insert( nEvent, SvxMacro( sTmp, sScriptType, eScriptType2 ));
                    }
                }
                break;

        }
    }

    // Sprungziele, die unseren ipmliziten Zielen entsprechen, schmeissen
    // wir hier ganz rigoros raus.
    if( !aName.isEmpty() )
    {
        OUString sDecoded( INetURLObject::decode( aName,
                                           INetURLObject::DECODE_UNAMBIGUOUS ));
        sal_Int32 nPos = sDecoded.lastIndexOf( cMarkSeparator );
        if( nPos != -1 )
        {
            OUString sCmp(comphelper::string::remove(sDecoded.copy(nPos+1), ' '));
            if( !sCmp.isEmpty() )
            {
                sCmp = sCmp.toAsciiLowerCase();
                if( sCmp == "region" ||
                    sCmp == "frame" ||
                    sCmp == "graphic" ||
                    sCmp == "ole" ||
                    sCmp == "table" ||
                    sCmp == "outline" ||
                    sCmp == "text" )
                {
                    aName.clear();
                }
            }
        }
    }

    // einen neuen Kontext anlegen
    _HTMLAttrContext *pCntxt = new _HTMLAttrContext( HTML_ANCHOR_ON );

    bool bEnAnchor = false, bFootnoteAnchor = false, bFootnoteEnSymbol = false;
    OUString aFootnoteName;
    OUString aStrippedClass( aClass );
    SwCSS1Parser::GetScriptFromClass( aStrippedClass, false );
    if( aStrippedClass.getLength() >=9  && bHasHRef && sHRef.getLength() > 1 &&
        ('s' == aStrippedClass[0] || 'S' == aStrippedClass[0]) &&
        ('d' == aStrippedClass[1] || 'D' == aStrippedClass[1]) )
    {
        if( aStrippedClass.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_sdendnote_anc ) )
            bEnAnchor = true;
        else if( aStrippedClass.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_sdfootnote_anc ) )
            bFootnoteAnchor = true;
        else if( aStrippedClass.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_sdendnote_sym ) ||
                 aStrippedClass.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_sdfootnote_sym ) )
            bFootnoteEnSymbol = true;
        if( bEnAnchor || bFootnoteAnchor || bFootnoteEnSymbol )
        {
            aFootnoteName = sHRef.copy( 1 );
            aClass = aStrippedClass = aName = aEmptyOUStr;
            bHasHRef = false;
        }
    }

    // Styles parsen
    if( HasStyleOptions( aStyle, aId, aStrippedClass, &aLang, &aDir ) )
    {
        SfxItemSet aItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        if( ParseStyleOptions( aStyle, aId, aClass, aItemSet, aPropInfo, &aLang, &aDir ) )
        {
            DoPositioning( aItemSet, aPropInfo, pCntxt );
            InsertAttrs( aItemSet, aPropInfo, pCntxt, true );
        }
    }

    if( bHasHRef )
    {
        if( !sHRef.isEmpty() )
        {
            sHRef = URIHelper::SmartRel2Abs( INetURLObject(sBaseURL), sHRef, Link<OUString *, bool>(), false );
        }
        else
        {
            // Bei leerer URL das Directory nehmen
            INetURLObject aURLObj( aPathToFile );
            sHRef = aURLObj.GetPartBeforeLastName();
        }

        pCSS1Parser->SetATagStyles();
        SwFormatINetFormat aINetFormat( sHRef, sTarget );
        aINetFormat.SetName( aName );

        if( !aMacroTable.empty() )
            aINetFormat.SetMacroTable( &aMacroTable );

        // das Default-Attribut setzen
        InsertAttr( &aAttrTab.pINetFormat, aINetFormat, pCntxt );
    }
    else if( !aName.isEmpty() )
    {
        InsertBookmark( aName );
    }

    if( bEnAnchor || bFootnoteAnchor )
    {
        InsertFootEndNote( aFootnoteName, bEnAnchor, bFixed );
        bInFootEndNoteAnchor = bCallNextToken = true;
    }
    else if( bFootnoteEnSymbol )
    {
        bInFootEndNoteSymbol = bCallNextToken = true;
    }

    // den Kontext merken
    PushContext( pCntxt );
}

void SwHTMLParser::EndAnchor()
{
    if( bInFootEndNoteAnchor )
    {
        FinishFootEndNote();
        bInFootEndNoteAnchor = false;
    }
    else if( bInFootEndNoteSymbol )
    {
        bInFootEndNoteSymbol = false;
    }

    EndTag( HTML_ANCHOR_OFF );
}

/*  */

void SwHTMLParser::InsertBookmark( const OUString& rName )
{
    _HTMLAttr* pTmp = new _HTMLAttr( *pPam->GetPoint(),
            SfxStringItem( RES_FLTR_BOOKMARK, rName ));
    aSetAttrTab.push_back( pTmp );
}

bool SwHTMLParser::HasCurrentParaBookmarks( bool bIgnoreStack ) const
{
    bool bHasMarks = false;
    sal_uLong nNodeIdx = pPam->GetPoint()->nNode.GetIndex();

    // first step: are there still bookmark in the attribute-stack?
    // bookmarks are added to the end of the stack - thus we only have
    // to check the last bookmark
    if( !bIgnoreStack )
    {
        for( auto i = aSetAttrTab.size(); i; )
        {
            _HTMLAttr* pAttr = aSetAttrTab[ --i ];
            if( RES_FLTR_BOOKMARK == pAttr->pItem->Which() )
            {
                if( pAttr->GetSttParaIdx() == nNodeIdx )
                    bHasMarks = true;
                break;
            }
        }
    }

    if( !bHasMarks )
    {
        // second step: when we didn't find a bookmark, check if there is one set already
        IDocumentMarkAccess* const pMarkAccess = pDoc->getIDocumentMarkAccess();
        for(IDocumentMarkAccess::const_iterator_t ppMark = pMarkAccess->getAllMarksBegin();
            ppMark != pMarkAccess->getAllMarksEnd();
            ++ppMark)
        {
            const ::sw::mark::IMark* pBookmark = ppMark->get();

            const sal_uLong nBookNdIdx = pBookmark->GetMarkPos().nNode.GetIndex();
            if( nBookNdIdx==nNodeIdx )
            {
                bHasMarks = true;
                break;
            }
            else if( nBookNdIdx > nNodeIdx )
                break;
        }
    }

    return bHasMarks;
}

/*  */

void SwHTMLParser::StripTrailingPara()
{
    bool bSetSmallFont = false;

    SwContentNode* pCNd = pPam->GetContentNode();
    if( !pPam->GetPoint()->nContent.GetIndex() )
    {
        if( pCNd && pCNd->StartOfSectionIndex()+2 <
            pCNd->EndOfSectionIndex() )
        {
            sal_uLong nNodeIdx = pPam->GetPoint()->nNode.GetIndex();

            const SwFrameFormats& rFrameFormatTable = *pDoc->GetSpzFrameFormats();

            for( auto pFormat : rFrameFormatTable )
            {
                SwFormatAnchor const*const pAnchor = &pFormat->GetAnchor();
                SwPosition const*const pAPos = pAnchor->GetContentAnchor();
                if (pAPos &&
                    ((FLY_AT_PARA == pAnchor->GetAnchorId()) ||
                     (FLY_AT_CHAR == pAnchor->GetAnchorId())) &&
                    pAPos->nNode == nNodeIdx )

                    return;     // den Knoten duerfen wir nicht loeschen
            }

            SetAttr( false );   // die noch offenen Attribute muessen
                                // beendet werden, bevor der Node
                                // geloescht wird, weil sonst der
                                // End-Index in die Botanik zeigt

            if( pCNd->Len() && pCNd->IsTextNode() )
            {
                // es wurden Felder in den Node eingefuegt, die muessen
                // wir jetzt verschieben
                SwTextNode *pPrvNd = pDoc->GetNodes()[nNodeIdx-1]->GetTextNode();
                if( pPrvNd )
                {
                    SwIndex aSrc( pCNd, 0 );
                    pCNd->GetTextNode()->CutText( pPrvNd, aSrc, pCNd->Len() );
                }
            }

            // jetz muessen wir noch eventuell vorhandene Bookmarks verschieben
            IDocumentMarkAccess* const pMarkAccess = pDoc->getIDocumentMarkAccess();
            for(IDocumentMarkAccess::const_iterator_t ppMark = pMarkAccess->getAllMarksBegin();
                ppMark != pMarkAccess->getAllMarksEnd();
                ++ppMark)
            {
                ::sw::mark::IMark* pMark = ppMark->get();

                sal_uLong nBookNdIdx = pMark->GetMarkPos().nNode.GetIndex();
                if(nBookNdIdx==nNodeIdx)
                {
                    SwNodeIndex nNewNdIdx(pPam->GetPoint()->nNode);
                    SwContentNode* pNd = SwNodes::GoPrevious(&nNewNdIdx);
                    if(!pNd)
                    {
                        OSL_ENSURE(false, "Oops, where is my predecessor node?");
                        return;
                    }
                    // #i81002# - refactoring
                    // Do not directly manipulate member of <SwBookmark>
                    {
                        SwPosition aNewPos(*pNd);
                        aNewPos.nContent.Assign(pNd, pNd->Len());
                        const SwPaM aPaM(aNewPos);
                        pMarkAccess->repositionMark(ppMark->get(), aPaM);
                    }
                }
                else if( nBookNdIdx > nNodeIdx )
                    break;
            }

            pPam->GetPoint()->nContent.Assign( 0, 0 );
            pPam->SetMark();
            pPam->DeleteMark();
            pDoc->GetNodes().Delete( pPam->GetPoint()->nNode );
            pPam->Move( fnMoveBackward, fnGoNode );
        }
        else if( pCNd && pCNd->IsTextNode() && pTable )
        {
            // In leeren Zellen stellen wir einen kleinen Font ein, damit die
            // Zelle nicht hoeher wird als die Grafik bzw. so niedrig wie
            // moeglich bleibt.
            bSetSmallFont = true;
        }
    }
    else if( pCNd && pCNd->IsTextNode() && pTable &&
             pCNd->StartOfSectionIndex()+2 ==
             pCNd->EndOfSectionIndex() )
    {
        // Wenn die Zelle nur zeichengebundene Grafiken/Rahmen enthaelt
        // stellen wir ebenfalls einen kleinen Font ein.
        bSetSmallFont = true;
        SwTextNode* pTextNd = pCNd->GetTextNode();

        sal_Int32 nPos = pPam->GetPoint()->nContent.GetIndex();
        while( bSetSmallFont && nPos>0 )
        {
            --nPos;
            bSetSmallFont =
                (CH_TXTATR_BREAKWORD == pTextNd->GetText()[nPos]) &&
                (0 != pTextNd->GetTextAttrForCharAt( nPos, RES_TXTATR_FLYCNT ));
        }
    }

    if( bSetSmallFont )
    {
        //Added default to CJK and CTL
        SvxFontHeightItem aFontHeight( 40, 100, RES_CHRATR_FONTSIZE );
        pCNd->SetAttr( aFontHeight );
        SvxFontHeightItem aFontHeightCJK( 40, 100, RES_CHRATR_CJK_FONTSIZE );
        pCNd->SetAttr( aFontHeightCJK );
        SvxFontHeightItem aFontHeightCTL( 40, 100, RES_CHRATR_CTL_FONTSIZE );
        pCNd->SetAttr( aFontHeightCTL );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
