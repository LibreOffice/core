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
#include <boost/shared_ptr.hpp>

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

ImageMap *SwHTMLParser::FindImageMap( const String& rName ) const
{
    ImageMap *pMap = 0;

    OSL_ENSURE( rName.GetChar(0) != '#', "FindImageName: Name beginnt mit #!" );

    if( pImageMaps )
    {
        for( sal_uInt16 i=0; i<pImageMaps->size(); i++ )
        {
            ImageMap *pIMap = &(*pImageMaps)[i];
            if( rName.EqualsIgnoreCaseAscii( pIMap->GetName() ) )
            {
                pMap = pIMap;
                break;
            }
        }
    }
    return pMap;
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
            SwFrmFmt *pFmt = pGrfNd->GetFlyFmt();
            SwFmtURL aURL( pFmt->GetURL() );
            const ImageMap *pIMap = aURL.GetMap();
            if( pIMap && pIMap->GetIMapObjectCount()==0 )
            {
                // Die (leere) Image-Map des Nodes wird entweder
                // durch die jetzt gefundene Image-Map ersetzt
                // oder geloescht.
                ImageMap *pNewIMap =
                    FindImageMap( pIMap->GetName() );
                aURL.SetMap( pNewIMap );
                pFmt->SetFmtAttr( aURL );
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
    sal_uInt16 i = aContexts.size();
    while( !pCntnrItemSet && i > nContextStMin )
        pCntnrItemSet = aContexts[--i]->GetFrmItemSet();

    if( pCntnrItemSet )
    {
        // Wenn wir und in einem Container befinden wird die Verankerung
        // des Containers uebernommen.
        rFrmItemSet.Put( *pCntnrItemSet );
    }
    else if( pCSS1Parser->MayBePositioned( rCSS1PropInfo, sal_True ) )
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
                                           sal_Bool bDontAppend )
{
    sal_Bool bMoveBackward = sal_False;
    SwFmtAnchor aAnchor( FLY_AS_CHAR );
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
        if( !bDontAppend && HasCurrentParaFlys( sal_True ) )
        {
            // Wenn der Absatz nur Grafiken enthaelt, braucht er
            // auch keinen unteren Absatz-Abstand. Da hier auch bei
            // Verwendung von Styles kein Abstand enstehen soll, wird
            // hier auch geweohnlich attributiert !!!
            sal_uInt16 nUpper=0, nLower=0;
            GetULSpaceFromContext( nUpper, nLower );
            InsertAttr( SvxULSpaceItem( nUpper, 0, RES_UL_SPACE ), sal_False, sal_True );

            AppendTxtNode( AM_NOSPACE );

            if( nUpper )
            {
                NewAttr( &aAttrTab.pULSpace, SvxULSpaceItem( 0, nLower, RES_UL_SPACE ) );
                aParaAttrs.push_back( aAttrTab.pULSpace );
                EndAttr( aAttrTab.pULSpace, 0, sal_False );
            }
        }

        // Vertikale Ausrichtung und Verankerung bestimmen.
        xub_StrLen nCntnt = pPam->GetPoint()->nContent.GetIndex();
        if( nCntnt )
        {
            aAnchor.SetType( FLY_AT_CHAR );
            bMoveBackward = sal_True;
            eVertOri = text::VertOrientation::CHAR_BOTTOM;
            eVertRel = text::RelOrientation::CHAR;
        }
        else
        {
            aAnchor.SetType( FLY_AT_PARA );
            eVertOri = text::VertOrientation::TOP;
            eVertRel = text::RelOrientation::PRINT_AREA;
        }

        rFrmSet.Put( SwFmtHoriOrient( 0, eHoriOri, eHoriRel) );

        rFrmSet.Put( SwFmtSurround( eSurround ) );
    }
    rFrmSet.Put( SwFmtVertOrient( 0, eVertOri, eVertRel) );

    if( bMoveBackward )
        pPam->Move( fnMoveBackward );

    aAnchor.SetAnchor( pPam->GetPoint() );

    if( bMoveBackward )
        pPam->Move( fnMoveForward );

    rFrmSet.Put( aAnchor );
}

void SwHTMLParser::RegisterFlyFrm( SwFrmFmt *pFlyFmt )
{
    // automatisch verankerte Rahmen muessen noch um eine Position
    // nach vorne verschoben werden.
    if( RES_DRAWFRMFMT != pFlyFmt->Which() &&
        (FLY_AT_PARA == pFlyFmt->GetAnchor().GetAnchorId()) &&
        SURROUND_THROUGHT == pFlyFmt->GetSurround().GetSurround() )
    {
        aMoveFlyFrms.push_back( pFlyFmt );
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
    String sAltNm, aId, aClass, aStyle, aMap, sHTMLGrfName;
    OUString sGrfNm;
    sal_Int16 eVertOri = text::VertOrientation::TOP;
    sal_Int16 eHoriOri = text::HoriOrientation::NONE;
    long nWidth=0, nHeight=0;
    long nVSpace=0, nHSpace=0;

    sal_uInt16 nBorder = (aAttrTab.pINetFmt ? 1 : 0);
    sal_Bool bIsMap = sal_False;
    sal_Bool bPrcWidth = sal_False;
    sal_Bool bPrcHeight = sal_False;
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
                    rOption.GetEnum( aHTMLImgHAlignTable,
                                                    text::HoriOrientation::NONE );
                break;
            case HTML_O_WIDTH:
                // erstmal nur als Pixelwerte merken!
                nWidth = rOption.GetNumber();
                bPrcWidth = (rOption.GetString().indexOf('%') != -1);
                if( bPrcWidth && nWidth>100 )
                    nWidth = 100;
                break;
            case HTML_O_HEIGHT:
                // erstmal nur als Pixelwerte merken!
                nHeight = rOption.GetNumber();
                bPrcHeight = (rOption.GetString().indexOf('%') != -1);
                if( bPrcHeight && nHeight>100 )
                    nHeight = 100;
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
                bIsMap = sal_True;
                break;
            case HTML_O_USEMAP:
                aMap = rOption.GetString();
                break;
            case HTML_O_NAME:
                sHTMLGrfName = rOption.GetString();
                break;

            case HTML_O_SDONLOAD:
                eScriptType2 = STARBASIC;
            case HTML_O_ONLOAD:
                nEvent = SVX_EVENT_IMAGE_LOAD;
                goto IMAGE_SETEVENT;

            case HTML_O_SDONABORT:
                eScriptType2 = STARBASIC;
            case HTML_O_ONABORT:
                nEvent = SVX_EVENT_IMAGE_ABORT;
                goto IMAGE_SETEVENT;

            case HTML_O_SDONERROR:
                eScriptType2 = STARBASIC;
            case HTML_O_ONERROR:
                nEvent = SVX_EVENT_IMAGE_ERROR;
                goto IMAGE_SETEVENT;
IMAGE_SETEVENT:
                {
                    String sTmp( rOption.GetString() );
                    if( sTmp.Len() )
                    {
                        sTmp = convertLineEnd(sTmp, GetSystemLineEnd());
                        String sScriptType;
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
        SwTxtNode* pTxtNode = pPam->GetNode()->GetTxtNode();

        if( pTxtNode && ! pTxtNode->IsCountedInList())
        {
            OSL_ENSURE( pTxtNode->GetActualListLevel() == GetNumInfo().GetLevel(),
                    "Numerierungs-Ebene stimmt nicht" );

            pTxtNode->SetCountedInList( true );

            // Rule invalisieren ist noetig, weil zwischem dem einlesen
            // des LI und der Grafik ein EndAction gerufen worden sein kann.
            if( GetNumInfo().GetNumRule() )
                GetNumInfo().GetNumRule()->SetInvalidRule( sal_True );

            // Die Vorlage novh mal setzen. Ist noetig, damit der
            // Erstzeilen-Einzug stimmt.
            SetTxtCollAttrs();

            return;
        }
    }

    SfxItemSet aItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
    SvxCSS1PropertyInfo aPropInfo;
    if( HasStyleOptions( aStyle, aId, aClass ) )
        ParseStyleOptions( aStyle, aId, aClass, aItemSet, aPropInfo );

    SfxItemSet aFrmSet( pDoc->GetAttrPool(),
                        RES_FRMATR_BEGIN, RES_FRMATR_END-1 );
    if( !IsNewDoc() )
        Reader::ResetFrmFmtAttrs( aFrmSet );

    // Umrandung setzen
    long nHBorderWidth = 0, nVBorderWidth = 0;
    if( nBorder )
    {
        nHBorderWidth = (long)nBorder;
        nVBorderWidth = (long)nBorder;
        SvxCSS1Parser::PixelToTwip( nVBorderWidth, nHBorderWidth );

        ::editeng::SvxBorderLine aHBorderLine( NULL, nHBorderWidth );
        ::editeng::SvxBorderLine aVBorderLine( NULL, nVBorderWidth );

        if( aAttrTab.pINetFmt )
        {
            const String& rURL =
                ((const SwFmtINetFmt&)aAttrTab.pINetFmt->GetItem()).GetValue();

            pCSS1Parser->SetATagStyles();
            sal_uInt16 nPoolId =  static_cast< sal_uInt16 >(pDoc->IsVisitedURL( rURL )
                                    ? RES_POOLCHR_INET_VISIT
                                    : RES_POOLCHR_INET_NORMAL);
            const SwCharFmt *pCharFmt = pCSS1Parser->GetCharFmtFromPool( nPoolId );
            aHBorderLine.SetColor( pCharFmt->GetColor().GetValue() );
            aVBorderLine.SetColor( aHBorderLine.GetColor() );
        }
        else
        {
            const SvxColorItem& rColorItem = aAttrTab.pFontColor ?
              (const SvxColorItem &)aAttrTab.pFontColor->GetItem() :
              (const SvxColorItem &)pDoc->GetDefault(RES_CHRATR_COLOR);
            aHBorderLine.SetColor( rColorItem.GetValue() );
            aVBorderLine.SetColor( aHBorderLine.GetColor() );
        }


        SvxBoxItem aBoxItem( RES_BOX );
        aBoxItem.SetLine( &aHBorderLine, BOX_LINE_TOP );
        aBoxItem.SetLine( &aHBorderLine, BOX_LINE_BOTTOM );
        aBoxItem.SetLine( &aVBorderLine, BOX_LINE_LEFT );
        aBoxItem.SetLine( &aVBorderLine, BOX_LINE_RIGHT );
        aFrmSet.Put( aBoxItem );
    }

    // Ausrichtung setzen
    SetAnchorAndAdjustment( eVertOri, eHoriOri, aItemSet, aPropInfo, aFrmSet );

    // Abstaende setzen
    SetSpace( Size( nHSpace, nVSpace), aItemSet, aPropInfo, aFrmSet );

    // Sonstige CSS1-Attribute Setzen
    SetFrmFmtAttrs( aItemSet, aPropInfo, HTML_FF_BOX, aFrmSet );

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
            bPrcWidth = sal_False;
            break;
        case SVX_CSS1_LTYPE_PERCENTAGE:
            aTwipSz.Width() = 0;
            nWidth = aPropInfo.nWidth;
            bPrcWidth = sal_True;
            break;
        default:
            ;
    }
    switch( aPropInfo.eHeightType )
    {
        case SVX_CSS1_LTYPE_TWIP:
            aTwipSz.Height() = aPropInfo.nHeight;
            nHeight = 1;    // != 0
            bPrcHeight = sal_False;
            break;
        case SVX_CSS1_LTYPE_PERCENTAGE:
            aTwipSz.Height() = 0;
            nHeight = aPropInfo.nHeight;
            bPrcHeight = sal_True;
            break;
        default:
            ;
    }

    Size aGrfSz( 0, 0 );
    sal_Bool bSetTwipSize = sal_True;       // Twip-Size am Node setzen?
    sal_Bool bChangeFrmSize = sal_False;    // Frame-Format nachtraeglich anpassen?
    sal_Bool bRequestGrfNow = sal_False;
    sal_Bool bSetScaleImageMap = sal_False;
    sal_uInt8 nPrcWidth = 0, nPrcHeight = 0;

    if( !nWidth || !nHeight )
    {
        // Es fehlt die Breite oder die Hoehe
        // Wenn die Grfik in einer Tabelle steht, wird sie gleich
        // angefordert, damit sie eventuell schon da ist, bevor die
        // Tabelle layoutet wird.
        if( pTable!=0 && !nWidth )
        {
            bRequestGrfNow = sal_True;
            IncGrfsThatResizeTable();
        }

        // Die Groesse des Rahmens wird nachtraeglich gesetzt
        bChangeFrmSize = sal_True;
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
        bSetTwipSize = sal_False;

        if( bPrcWidth )
            nPrcWidth = (sal_uInt8)nWidth;

        if( bPrcHeight )
            nPrcHeight = (sal_uInt8)nHeight;
    }

    // Image-Map setzen
    aMap = comphelper::string::stripEnd(aMap, ' ');
    if( aMap.Len() )
    {
        // Da wir nur lokale Image-Maps kennen nehmen wireinfach alles
        // hinter dem # als Namen
        xub_StrLen nPos = aMap.Search( '#' );
        String aName;
        if ( STRING_NOTFOUND==nPos )
            aName = aMap ;
        else
            aName = aMap.Copy(nPos+1);

        ImageMap *pImgMap = FindImageMap( aName );
        if( pImgMap )
        {
            SwFmtURL aURL; aURL.SetMap( pImgMap );//wird kopieiert

            bSetScaleImageMap = !nPrcWidth || !nPrcHeight;
            aFrmSet.Put( aURL );
        }
        else
        {
            ImageMap aEmptyImgMap( aName );
            SwFmtURL aURL; aURL.SetMap( &aEmptyImgMap );//wird kopieiert
            aFrmSet.Put( aURL );
            nMissingImgMaps++;          // es fehlen noch Image-Maps

            // die Grafik muss beim SetTwipSize skaliert werden, wenn
            // wir keine Groesse am Node gesetzt haben oder die Groesse
            // nicht der Grafikgroesse entsprach.
            bSetScaleImageMap = sal_True;
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

    SwFmtFrmSize aFrmSize( ATT_FIX_SIZE, aTwipSz.Width(), aTwipSz.Height() );
    aFrmSize.SetWidthPercent( nPrcWidth );
    aFrmSize.SetHeightPercent( nPrcHeight );
    aFrmSet.Put( aFrmSize );

    Graphic aEmptyGrf;
    aEmptyGrf.SetDefaultType();
    SwFrmFmt *pFlyFmt = pDoc->Insert( *pPam, sGrfNm, aEmptyStr, &aEmptyGrf,
                                      &aFrmSet, NULL, NULL );
    SwGrfNode *pGrfNd = pDoc->GetNodes()[ pFlyFmt->GetCntnt().GetCntntIdx()
                                  ->GetIndex()+1 ]->GetGrfNode();

    if( sHTMLGrfName.Len() )
    {
        pFlyFmt->SetName( sHTMLGrfName );

        // ggfs. eine Grafik anspringen
        if( JUMPTO_GRAPHIC == eJumpTo && sHTMLGrfName == sJmpMark )
        {
            bChkJumpMark = sal_True;
            eJumpTo = JUMPTO_NONE;
        }
    }

    if( sAltNm.Len() )
        pGrfNd->SetTitle( sAltNm );

    if( bSetTwipSize )
        pGrfNd->SetTwipSize( aGrfSz );

    pGrfNd->SetChgTwipSize( bChangeFrmSize, bChangeFrmSize );

    if( bSetScaleImageMap )
        pGrfNd->SetScaleImageMap( sal_True );

    if( aAttrTab.pINetFmt )
    {
        const SwFmtINetFmt &rINetFmt =
            (const SwFmtINetFmt&)aAttrTab.pINetFmt->GetItem();

        SwFmtURL aURL( pFlyFmt->GetURL() );

        aURL.SetURL( rINetFmt.GetValue(), bIsMap );
        aURL.SetTargetFrameName( rINetFmt.GetTargetFrame() );
        aURL.SetName( rINetFmt.GetName() );
        pFlyFmt->SetFmtAttr( aURL );

        {
            const SvxMacro *pMacro;
            static const sal_uInt16 aEvents[] = {
                SFX_EVENT_MOUSEOVER_OBJECT,
                SFX_EVENT_MOUSECLICK_OBJECT,
                SFX_EVENT_MOUSEOUT_OBJECT,
                0 };

            for( sal_uInt16 n = 0; aEvents[ n ]; ++n )
                if( 0 != ( pMacro = rINetFmt.GetMacro( aEvents[ n ] ) ))
                    aMacroItem.SetMacro( aEvents[ n ], *pMacro );
        }

        if ((FLY_AS_CHAR == pFlyFmt->GetAnchor().GetAnchorId()) &&
            aAttrTab.pINetFmt->GetSttPara() ==
                        pPam->GetPoint()->nNode &&
            aAttrTab.pINetFmt->GetSttCnt() ==
                        pPam->GetPoint()->nContent.GetIndex() - 1 )
        {
            // das Attribut wurde unmitellbar vor einer zeichengeb.
            // Grafik eingefuegt, also verschieben wir es
            aAttrTab.pINetFmt->SetStart( *pPam->GetPoint() );

            // Wenn das Attribut auch ein Sprungziel ist, fuegen
            // wir noch eine Bookmark vor der Grafik ein, weil das
            // SwFmtURL kein Sprungziel ist.
            if( !rINetFmt.GetName().isEmpty() )
            {
                pPam->Move( fnMoveBackward );
                InsertBookmark( rINetFmt.GetName() );
                pPam->Move( fnMoveForward );
            }
        }

    }

    if( !aMacroItem.GetMacroTable().empty() )
        pFlyFmt->SetFmtAttr( aMacroItem );

    // Wenn die Grafik gleich angeforder wird, muss dies geschehen,
    // nachdem das Format vollstaendig aufgebaut ist, weil es evtl.
    // gleich (synchron) angepasst wird (war bug #40983#)
    if( bRequestGrfNow )
    {
        pGrfNd->SwapIn();
    }

    // Ggf. Frames anlegen und Auto-gebundenen Rahmen registrieren
    RegisterFlyFrm( pFlyFmt );

    if( aId.Len() )
        InsertBookmark( aId );
}

/*  */

void SwHTMLParser::InsertBodyOptions()
{
    pDoc->SetTxtFmtColl( *pPam,
                         pCSS1Parser->GetTxtCollFromPool( RES_POOLCOLL_TEXT ) );

    String aBackGround, aId, aStyle, aLang, aDir;
    Color aBGColor, aTextColor, aLinkColor, aVLinkColor;
    sal_Bool bBGColor=sal_False, bTextColor=sal_False;
    sal_Bool bLinkColor=sal_False, bVLinkColor=sal_False;

    ScriptType eDfltScriptType;
    OUString sDfltScriptType;
    GetDefaultScriptType( eDfltScriptType, sDfltScriptType );

    const HTMLOptions& rHTMLOptions = GetOptions();
    for (size_t i = rHTMLOptions.size(); i; )
    {
        const HTMLOption& rOption = rHTMLOptions[--i];
        ScriptType eScriptType2 = eDfltScriptType;
        OUString aEvent;
        sal_Bool bSetEvent = sal_False;

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
                bBGColor = sal_True;
                break;
            case HTML_O_TEXT:
                rOption.GetColor( aTextColor );
                bTextColor = sal_True;
                break;
            case HTML_O_LINK:
                rOption.GetColor( aLinkColor );
                bLinkColor = sal_True;
                break;
            case HTML_O_VLINK:
                rOption.GetColor( aVLinkColor );
                bVLinkColor = sal_True;
                break;

            case HTML_O_SDONLOAD:
                eScriptType2 = STARBASIC;
            case HTML_O_ONLOAD:
                aEvent = GlobalEventConfig::GetEventName( STR_EVENT_OPENDOC );
                bSetEvent = sal_True;
                break;

            case HTML_O_SDONUNLOAD:
                eScriptType2 = STARBASIC;
            case HTML_O_ONUNLOAD:
                aEvent = GlobalEventConfig::GetEventName( STR_EVENT_PREPARECLOSEDOC );
                bSetEvent = sal_True;
                break;

            case HTML_O_SDONFOCUS:
                eScriptType2 = STARBASIC;
            case HTML_O_ONFOCUS:
                aEvent = GlobalEventConfig::GetEventName( STR_EVENT_ACTIVATEDOC );
                bSetEvent = sal_True;
                break;

            case HTML_O_SDONBLUR:
                eScriptType2 = STARBASIC;
            case HTML_O_ONBLUR:
                aEvent = GlobalEventConfig::GetEventName( STR_EVENT_DEACTIVATEDOC );
                bSetEvent = sal_True;
                break;

            case HTML_O_ONERROR:
                break;

            case HTML_O_STYLE:
                aStyle = rOption.GetString();
                bTextColor = sal_True;
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
        pCSS1Parser->GetTxtCollFromPool( RES_POOLCOLL_STANDARD )
            ->SetFmtAttr( SvxColorItem(aTextColor, RES_CHRATR_COLOR) );
        pCSS1Parser->SetBodyTextSet();
    }


    // Die Item fuer die Seitenvorlage vorbereiten (Hintergrund, Umrandung)
    // Beim BrushItem muessen schon gesetzte werte erhalten bleiben!
    SvxBrushItem aBrushItem( pCSS1Parser->GetPageDescBackground() );
    sal_Bool bSetBrush = sal_False;

    if( bBGColor && !pCSS1Parser->IsBodyBGColorSet() )
    {
        // Hintergrundfarbe aus "BGCOLOR"
        String aLink;
        if( aBrushItem.GetGraphicLink() )
            aLink = *aBrushItem.GetGraphicLink();
        SvxGraphicPosition ePos = aBrushItem.GetGraphicPos();

        aBrushItem.SetColor( aBGColor );

        if( aLink.Len() )
        {
            aBrushItem.SetGraphicLink( aLink );
            aBrushItem.SetGraphicPos( ePos );
        }
        bSetBrush = sal_True;
        pCSS1Parser->SetBodyBGColorSet();
    }

    if( aBackGround.Len() && !pCSS1Parser->IsBodyBackgroundSet() )
    {
        // Hintergrundgrafik aus "BACKGROUND"
        aBrushItem.SetGraphicLink( INetURLObject::GetAbsURL( sBaseURL, aBackGround ) );
        aBrushItem.SetGraphicPos( GPOS_TILED );
        bSetBrush = sal_True;
        pCSS1Parser->SetBodyBackgroundSet();
    }

    if( aStyle.Len() || aDir.Len() )
    {
        SfxItemSet aItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;
        String aDummy;
        ParseStyleOptions( aStyle, aDummy, aDummy, aItemSet, aPropInfo, 0, &aDir );

        // Ein par Attribute muessen an der Seitenvorlage gesetzt werden,
        // und zwar die, die nicht vererbit werden
        pCSS1Parser->SetPageDescAttrs( bSetBrush ? &aBrushItem : 0,
                                       &aItemSet );

        const SfxPoolItem *pItem;
        static const sal_uInt16 aWhichIds[3] = { RES_CHRATR_FONTSIZE,
                                       RES_CHRATR_CJK_FONTSIZE,
                                       RES_CHRATR_CTL_FONTSIZE };
        for( sal_uInt16 i=0; i<3; i++ )
        {
            if( SFX_ITEM_SET == aItemSet.GetItemState( aWhichIds[i], sal_False,
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
        pCSS1Parser->GetTxtCollFromPool( RES_POOLCOLL_STANDARD )
            ->SetFmtAttr( aItemSet );
    }
    else if( bSetBrush )
    {
        pCSS1Parser->SetPageDescAttrs( &aBrushItem );
    }

    if( bLinkColor && !pCSS1Parser->IsBodyLinkSet() )
    {
        SwCharFmt *pCharFmt =
            pCSS1Parser->GetCharFmtFromPool(RES_POOLCHR_INET_NORMAL);
        pCharFmt->SetFmtAttr( SvxColorItem(aLinkColor, RES_CHRATR_COLOR) );
        pCSS1Parser->SetBodyLinkSet();
    }
    if( bVLinkColor && !pCSS1Parser->IsBodyVLinkSet() )
    {
        SwCharFmt *pCharFmt =
            pCSS1Parser->GetCharFmtFromPool(RES_POOLCHR_INET_VISIT);
        pCharFmt->SetFmtAttr( SvxColorItem(aVLinkColor, RES_CHRATR_COLOR) );
        pCSS1Parser->SetBodyVLinkSet();
    }
    if( aLang.Len() )
    {
        LanguageType eLang = LanguageTag::convertToLanguageType( aLang );
        if( LANGUAGE_DONTKNOW != eLang )
        {
            sal_uInt16 nWhich = 0;
            switch( SvtLanguageOptions::GetScriptTypeOfLanguage( eLang ) )
            {
            case SCRIPTTYPE_LATIN:
                nWhich = RES_CHRATR_LANGUAGE;
                break;
            case SCRIPTTYPE_ASIAN:
                nWhich = RES_CHRATR_CJK_LANGUAGE;
                break;
            case SCRIPTTYPE_COMPLEX:
                nWhich = RES_CHRATR_CTL_LANGUAGE;
                break;
            }
            if( nWhich )
            {
                SvxLanguageItem aLanguage( eLang, nWhich );
                aLanguage.SetWhich( nWhich );
                pDoc->SetDefault( aLanguage );
            }
        }
    }

    if( aId.Len() )
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

    SvxMacroTableDtor aMacroTbl;
    String sHRef, aName, sTarget;
    String aId, aStyle, aClass, aLang, aDir;
    sal_Bool bHasHRef = sal_False, bFixed = sal_False;

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
                bHasHRef = sal_True;
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
                bFixed = sal_True;
                break;
            case HTML_O_LANG:
                aLang = rOption.GetString();
                break;
            case HTML_O_DIR:
                aDir = rOption.GetString();
                break;

            case HTML_O_SDONCLICK:
                eScriptType2 = STARBASIC;
            case HTML_O_ONCLICK:
                nEvent = SFX_EVENT_MOUSECLICK_OBJECT;
                goto ANCHOR_SETEVENT;

            case HTML_O_SDONMOUSEOVER:
                eScriptType2 = STARBASIC;
            case HTML_O_ONMOUSEOVER:
                nEvent = SFX_EVENT_MOUSEOVER_OBJECT;
                goto ANCHOR_SETEVENT;

            case HTML_O_SDONMOUSEOUT:
                eScriptType2 = STARBASIC;
            case HTML_O_ONMOUSEOUT:
                nEvent = SFX_EVENT_MOUSEOUT_OBJECT;
                goto ANCHOR_SETEVENT;
ANCHOR_SETEVENT:
                {
                    String sTmp( rOption.GetString() );
                    if( sTmp.Len() )
                    {
                        sTmp = convertLineEnd(sTmp, GetSystemLineEnd());
                        String sScriptType;
                        if( EXTENDED_STYPE == eScriptType2 )
                            sScriptType = sDfltScriptType;
                        aMacroTbl.Insert( nEvent, SvxMacro( sTmp, sScriptType, eScriptType2 ));
                    }
                }
                break;

        }
    }

    // Sprungziele, die unseren ipmliziten Zielen entsprechen, schmeissen
    // wir hier ganz rigoros raus.
    if( aName.Len() )
    {
        String sDecoded( INetURLObject::decode( aName, INET_HEX_ESCAPE,
                                           INetURLObject::DECODE_UNAMBIGUOUS,
                                        RTL_TEXTENCODING_UTF8 ));
        xub_StrLen nPos = sDecoded.SearchBackward( cMarkSeparator );
        if( STRING_NOTFOUND != nPos )
        {
            String sCmp(comphelper::string::remove(sDecoded.Copy(nPos+1), ' '));
            if( sCmp.Len() )
            {
                sCmp.ToLowerAscii();
                if( sCmp.EqualsAscii( pMarkToRegion ) ||
                    sCmp.EqualsAscii( pMarkToFrame ) ||
                    sCmp.EqualsAscii( pMarkToGraphic ) ||
                    sCmp.EqualsAscii( pMarkToOLE ) ||
                    sCmp.EqualsAscii( pMarkToTable ) ||
                    sCmp.EqualsAscii( pMarkToOutline ) ||
                    sCmp.EqualsAscii( pMarkToText ) )
                {
                    aName.Erase();
                }
            }
        }
    }

    // einen neuen Kontext anlegen
    _HTMLAttrContext *pCntxt = new _HTMLAttrContext( HTML_ANCHOR_ON );

    sal_Bool bEnAnchor = sal_False, bFtnAnchor = sal_False, bFtnEnSymbol = sal_False;
    OUString aFtnName;
    OUString aStrippedClass( aClass );
    SwCSS1Parser::GetScriptFromClass( aStrippedClass, sal_False );
    if( aStrippedClass.getLength() >=9  && bHasHRef && sHRef.Len() > 1 &&
        ('s' == aStrippedClass[0] || 'S' == aStrippedClass[0]) &&
        ('d' == aStrippedClass[1] || 'D' == aStrippedClass[1]) )
    {
        if( aStrippedClass.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_sdendnote_anc ) )
            bEnAnchor = sal_True;
        else if( aStrippedClass.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_sdfootnote_anc ) )
            bFtnAnchor = sal_True;
        else if( aStrippedClass.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_sdendnote_sym ) ||
                 aStrippedClass.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_sdfootnote_sym ) )
            bFtnEnSymbol = sal_True;
        if( bEnAnchor || bFtnAnchor || bFtnEnSymbol )
        {
            aFtnName = sHRef.Copy( 1 );
            aClass = aStrippedClass = aName = aEmptyStr;
            bHasHRef = sal_False;
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
            InsertAttrs( aItemSet, aPropInfo, pCntxt, sal_True );
        }
    }

    if( bHasHRef )
    {
        if( sHRef.Len() )
        {
            sHRef = URIHelper::SmartRel2Abs( INetURLObject(sBaseURL), sHRef, Link(), false );
        }
        else
        {
            // Bei leerer URL das Directory nehmen
            INetURLObject aURLObj( aPathToFile );
            sHRef = aURLObj.GetPartBeforeLastName();
        }

        pCSS1Parser->SetATagStyles();
        SwFmtINetFmt aINetFmt( sHRef, sTarget );
        aINetFmt.SetName( aName );

        if( !aMacroTbl.empty() )
            aINetFmt.SetMacroTbl( &aMacroTbl );

        // das Default-Attribut setzen
        InsertAttr( &aAttrTab.pINetFmt, aINetFmt, pCntxt );
    }
    else if( aName.Len() )
    {
        InsertBookmark( aName );
    }

    if( bEnAnchor || bFtnAnchor )
    {
        InsertFootEndNote( aFtnName, bEnAnchor, bFixed );
        bInFootEndNoteAnchor = bCallNextToken = sal_True;
    }
    else if( bFtnEnSymbol )
    {
        bInFootEndNoteSymbol = bCallNextToken = sal_True;
    }

    // den Kontext merken
    PushContext( pCntxt );
}

void SwHTMLParser::EndAnchor()
{
    if( bInFootEndNoteAnchor )
    {
        FinishFootEndNote();
        bInFootEndNoteAnchor = sal_False;
    }
    else if( bInFootEndNoteSymbol )
    {
        bInFootEndNoteSymbol = sal_False;
    }

    EndTag( HTML_ANCHOR_OFF );
}

/*  */

void SwHTMLParser::InsertBookmark( const String& rName )
{
    _HTMLAttr* pTmp = new _HTMLAttr( *pPam->GetPoint(),
            SfxStringItem( RES_FLTR_BOOKMARK, rName ));
    aSetAttrTab.push_back( pTmp );
}

sal_Bool SwHTMLParser::HasCurrentParaBookmarks( sal_Bool bIgnoreStack ) const
{
    sal_Bool bHasMarks = sal_False;
    sal_uLong nNodeIdx = pPam->GetPoint()->nNode.GetIndex();

    // first step: are there still bookmark in the attribute-stack?
    // bookmarks are added to the end of the stack - thus we only have
    // to check the last bookmark
    if( !bIgnoreStack )
    {
        _HTMLAttr* pAttr;
        for( sal_uInt16 i = aSetAttrTab.size(); i; )
        {
            pAttr = aSetAttrTab[ --i ];
            if( RES_FLTR_BOOKMARK == pAttr->pItem->Which() )
            {
                if( pAttr->GetSttParaIdx() == nNodeIdx )
                    bHasMarks = sal_True;
                break;
            }
        }
    }

    if( !bHasMarks )
    {
        // second step: when we didnt find a bookmark, check if there is one
        // set already
        IDocumentMarkAccess* const pMarkAccess = pDoc->getIDocumentMarkAccess();
        for(IDocumentMarkAccess::const_iterator_t ppMark = pMarkAccess->getMarksBegin();
            ppMark != pMarkAccess->getMarksEnd();
            ++ppMark)
        {
            const ::sw::mark::IMark* pBookmark = ppMark->get();
            sal_uLong nBookNdIdx = pBookmark->GetMarkPos().nNode.GetIndex();
            if( nBookNdIdx==nNodeIdx )
            {
                bHasMarks = sal_True;
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
    sal_Bool bSetSmallFont = sal_False;

    SwCntntNode* pCNd = pPam->GetCntntNode();
    if( !pPam->GetPoint()->nContent.GetIndex() )
    {
        if( pCNd && pCNd->StartOfSectionIndex()+2 <
            pCNd->EndOfSectionIndex() )
        {
            sal_uLong nNodeIdx = pPam->GetPoint()->nNode.GetIndex();

            const SwFrmFmts& rFrmFmtTbl = *pDoc->GetSpzFrmFmts();

            for( sal_uInt16 i=0; i<rFrmFmtTbl.size(); i++ )
            {
                SwFrmFmt const*const pFmt = rFrmFmtTbl[i];
                SwFmtAnchor const*const pAnchor = &pFmt->GetAnchor();
                SwPosition const*const pAPos = pAnchor->GetCntntAnchor();
                if (pAPos &&
                    ((FLY_AT_PARA == pAnchor->GetAnchorId()) ||
                     (FLY_AT_CHAR == pAnchor->GetAnchorId())) &&
                    pAPos->nNode == nNodeIdx )

                    return;     // den Knoten duerfen wir nicht loeschen
            }

            SetAttr( sal_False );   // die noch offenen Attribute muessen
                                // beendet werden, bevor der Node
                                // geloescht wird, weil sonst der
                                // End-Index in die Botanik zeigt

            if( pCNd->Len() && pCNd->IsTxtNode() )
            {
                // es wurden Felder in den Node eingefuegt, die muessen
                // wir jetzt verschieben
                SwTxtNode *pPrvNd = pDoc->GetNodes()[nNodeIdx-1]->GetTxtNode();
                if( pPrvNd )
                {
                    SwIndex aSrc( pCNd, 0 );
                    pCNd->GetTxtNode()->CutText( pPrvNd, aSrc, pCNd->Len() );
                }
            }

            // jetz muessen wir noch eventuell vorhandene Bookmarks
            // verschieben
            IDocumentMarkAccess* const pMarkAccess = pDoc->getIDocumentMarkAccess();
            for(IDocumentMarkAccess::const_iterator_t ppMark = pMarkAccess->getMarksBegin();
                ppMark != pMarkAccess->getMarksEnd();
                ++ppMark)
            {
                ::sw::mark::IMark* pMark = ppMark->get();
                sal_uLong nBookNdIdx = pMark->GetMarkPos().nNode.GetIndex();
                if(nBookNdIdx==nNodeIdx)
                {
                    SwNodeIndex nNewNdIdx(pPam->GetPoint()->nNode);
                    SwCntntNode* pNd = pDoc->GetNodes().GoPrevious(&nNewNdIdx);
                    if(!pNd)
                    {
                        OSL_ENSURE(!this, "Hoppla, wo ist mein Vorgaenger-Node");
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
        else if( pCNd && pCNd->IsTxtNode() && pTable )
        {
            // In leeren Zellen stellen wir einen kleinen Font ein, damit die
            // Zelle nicht hoeher wird als die Grafik bzw. so niedrig wie
            // moeglich bleibt.
            bSetSmallFont = sal_True;
        }
    }
    else if( pCNd && pCNd->IsTxtNode() && pTable &&
             pCNd->StartOfSectionIndex()+2 ==
             pCNd->EndOfSectionIndex() )
    {
        // Wenn die Zelle nur zeichengebundene Grafiken/Rahmen enthaelt
        // stellen wir ebenfalls einen kleinen Font ein.
        bSetSmallFont = sal_True;
        SwTxtNode* pTxtNd = pCNd->GetTxtNode();

        xub_StrLen nPos = pPam->GetPoint()->nContent.GetIndex();
        while( bSetSmallFont && nPos>0 )
        {
            --nPos;
            bSetSmallFont =
                (CH_TXTATR_BREAKWORD == pTxtNd->GetTxt()[nPos]) &&
                (0 != pTxtNd->GetTxtAttrForCharAt( nPos, RES_TXTATR_FLYCNT ));
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
