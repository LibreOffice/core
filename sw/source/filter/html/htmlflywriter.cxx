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

#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <comphelper/string.hxx>
#include <svx/svxids.hrc>
#include "hintids.hxx"
#include <svl/urihelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <svtools/htmlkywd.hxx>
#include <svtools/htmlout.hxx>
#include <svtools/imap.hxx>
#include <svtools/imapobj.hxx>
#include <svtools/htmlcfg.hxx>
#include <svx/xoutbmp.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/brushitem.hxx>

#include <fmtanchr.hxx>
#include <fmtornt.hxx>
#include <fmturl.hxx>
#include <fmtfsize.hxx>
#include <fmtclds.hxx>
#include <fmtcntnt.hxx>
#include <fmtsrnd.hxx>
#include <fmtinfmt.hxx>
#include <txtinet.hxx>
#include "frmatr.hxx"
#include <grfatr.hxx>
#include <flypos.hxx>
#include <docary.hxx>
#include <ndgrf.hxx>

#include "doc.hxx"
#include "ndtxt.hxx"
#include "pam.hxx"
#include "swerror.h"
#include "frmfmt.hxx"
#include "wrthtml.hxx"
#include "css1kywd.hxx"
#include "htmlfly.hxx"

using namespace css;

const sal_uLong HTML_FRMOPTS_IMG_ALL        =
    HTML_FRMOPT_ALT |
    HTML_FRMOPT_SIZE |
    HTML_FRMOPT_ANYSIZE |
    HTML_FRMOPT_BORDER |
    HTML_FRMOPT_NAME;
const sal_uLong HTML_FRMOPTS_IMG_CNTNR      =
    HTML_FRMOPTS_IMG_ALL |
    HTML_FRMOPT_ABSSIZE;
const sal_uLong HTML_FRMOPTS_IMG            =
    HTML_FRMOPTS_IMG_ALL |
    HTML_FRMOPT_ALIGN |
    HTML_FRMOPT_SPACE |
    HTML_FRMOPT_BRCLEAR;
const sal_uLong HTML_FRMOPTS_IMG_CSS1       =
    HTML_FRMOPT_S_ALIGN |
    HTML_FRMOPT_S_SPACE;

const sal_uLong HTML_FRMOPTS_DIV            =
    HTML_FRMOPT_ID |
    HTML_FRMOPT_S_ALIGN |
    HTML_FRMOPT_S_SIZE |
    HTML_FRMOPT_ANYSIZE |
    HTML_FRMOPT_ABSSIZE |
    HTML_FRMOPT_S_SPACE |
    HTML_FRMOPT_S_BORDER |
    HTML_FRMOPT_S_BACKGROUND |
    HTML_FRMOPT_BRCLEAR |
    HTML_FRMOPT_DIR;

const sal_uLong HTML_FRMOPTS_MULTICOL       =
    HTML_FRMOPT_ID |
    HTML_FRMOPT_WIDTH |
    HTML_FRMOPT_ANYSIZE |
    HTML_FRMOPT_ABSSIZE |
    HTML_FRMOPT_DIR;
const sal_uLong HTML_FRMOPTS_MULTICOL_CNTNR =
    HTML_FRMOPTS_MULTICOL;
const sal_uLong HTML_FRMOPTS_MULTICOL_CSS1  =
    HTML_FRMOPT_S_ALIGN |
    HTML_FRMOPT_S_SIZE |
    HTML_FRMOPT_S_SPACE |
    HTML_FRMOPT_S_BORDER|
    HTML_FRMOPT_S_BACKGROUND;

const sal_uLong HTML_FRMOPTS_SPACER         =
    HTML_FRMOPT_ALIGN |
    HTML_FRMOPT_SIZE |
    HTML_FRMOPT_ANYSIZE |
    HTML_FRMOPT_BRCLEAR |
    HTML_FRMOPT_MARGINSIZE |
    HTML_FRMOPT_ABSSIZE;

const sal_uLong HTML_FRMOPTS_CNTNR          =
    HTML_FRMOPT_S_ALIGN |
    HTML_FRMOPT_S_SPACE |
    HTML_FRMOPT_S_WIDTH |
    HTML_FRMOPT_ANYSIZE |
    HTML_FRMOPT_ABSSIZE |
    HTML_FRMOPT_S_PIXSIZE;

static Writer& OutHTML_FrmFmtTableNode( Writer& rWrt, const SwFrmFmt& rFrmFmt );
static Writer& OutHTML_FrmFmtAsMulticol( Writer& rWrt, const SwFrmFmt& rFmt,
                                         bool bInCntnr );
static Writer& OutHTML_FrmFmtAsSpacer( Writer& rWrt, const SwFrmFmt& rFmt );
static Writer& OutHTML_FrmFmtAsDivOrSpan( Writer& rWrt,
                                          const SwFrmFmt& rFrmFmt, bool bSpan );
static Writer& OutHTML_FrmFmtAsImage( Writer& rWrt, const SwFrmFmt& rFmt,
                                      bool bInCntnr );

static Writer& OutHTML_FrmFmtGrfNode( Writer& rWrt, const SwFrmFmt& rFmt,
                                      bool bInCntnr );

static Writer& OutHTML_FrmFmtAsMarquee( Writer& rWrt, const SwFrmFmt& rFrmFmt,
                                        const SdrObject& rSdrObj    );

extern HTMLOutEvent aAnchorEventTable[];

static HTMLOutEvent aImageEventTable[] =
{
    { OOO_STRING_SVTOOLS_HTML_O_SDonload,           OOO_STRING_SVTOOLS_HTML_O_onload,       SVX_EVENT_IMAGE_LOAD        },
    { OOO_STRING_SVTOOLS_HTML_O_SDonabort,      OOO_STRING_SVTOOLS_HTML_O_onabort,  SVX_EVENT_IMAGE_ABORT       },
    { OOO_STRING_SVTOOLS_HTML_O_SDonerror,      OOO_STRING_SVTOOLS_HTML_O_onerror,  SVX_EVENT_IMAGE_ERROR       },
    { 0,                        0,                  0                       }
};

static HTMLOutEvent aIMapEventTable[] =
{
    { OOO_STRING_SVTOOLS_HTML_O_SDonmouseover,  OOO_STRING_SVTOOLS_HTML_O_onmouseover,  SFX_EVENT_MOUSEOVER_OBJECT  },
    { OOO_STRING_SVTOOLS_HTML_O_SDonmouseout,       OOO_STRING_SVTOOLS_HTML_O_onmouseout,       SFX_EVENT_MOUSEOUT_OBJECT   },
    { 0,                        0,                      0                           }
};

sal_uInt16 SwHTMLWriter::GuessFrmType( const SwFrmFmt& rFrmFmt,
                                   const SdrObject*& rpSdrObj )
{
    SwHTMLFrmType eType;

    if( RES_DRAWFRMFMT == rFrmFmt.Which() )
    {
        // Als Default irgendein Zeichen-Objekt
        eType = HTML_FRMTYPE_DRAW;

        const SdrObject *pObj =
            SwHTMLWriter::GetMarqueeTextObj( (const SwDrawFrmFmt &)rFrmFmt );
        if( pObj )
        {
            // Laufschrift
            rpSdrObj = pObj;
            eType = HTML_FRMTYPE_MARQUEE;
        }
        else
        {
            pObj = GetHTMLControl( (const SwDrawFrmFmt &)rFrmFmt );

            if( pObj )
            {
                // Form-Control
                rpSdrObj = pObj;
                eType = HTML_FRMTYPE_CONTROL;
            }
        }
    }
    else
    {
        // Als Default ein Textrahmen
        eType = HTML_FRMTYPE_TEXT;

        const SwFmtCntnt& rFlyCntnt = rFrmFmt.GetCntnt();
        sal_uLong nStt = rFlyCntnt.GetCntntIdx()->GetIndex()+1;
        const SwNode* pNd = pDoc->GetNodes()[ nStt ];

        if( pNd->IsGrfNode() )
        {
            // Grafik - Node
            eType = HTML_FRMTYPE_GRF;
        }
        else if( pNd->IsOLENode() )
        {
            // Applet, Plugin, Floating-Frame
            eType = (SwHTMLFrmType)GuessOLENodeFrmType( *pNd );
        }
        else
        {
            sal_uLong nEnd = pDoc->GetNodes()[nStt-1]->EndOfSectionIndex();

            const SfxPoolItem* pItem;
            const SfxItemSet& rItemSet = rFrmFmt.GetAttrSet();
            if( SfxItemState::SET == rItemSet.GetItemState( RES_COL,
                                                       true, &pItem ) &&
                ((const SwFmtCol *)pItem)->GetNumCols() > 1 )
            {
                // spaltiger Rahmen
                eType = HTML_FRMTYPE_MULTICOL;
            }
            else if( pNd->IsTableNode() )
            {
                const SwTableNode *pTblNd = pNd->GetTableNode();
                sal_uLong nTblEnd = pTblNd->EndOfSectionIndex();

                if( nTblEnd+1 == nEnd )
                {
                    // Tabelle
                    eType = HTML_FRMTYPE_TABLE;
                }
                else if( nTblEnd+2 == nEnd )
                {
                    // Tabelle mit Unterschrft
                    eType = HTML_FRMTYPE_TABLE_CAP;
                }
            }
            else if( pNd->IsTxtNode() )
            {
                const SwTxtNode *pTxtNd = pNd->GetTxtNode();

                bool bEmpty = false;
                if( nStt==nEnd-1 && !pTxtNd->Len() )
                {
                    // leerer Rahmen? Nur wenn kein Rahmen am
                    // Text- oder Start-Node verankert ist.
                    bEmpty = true;
                    if( pHTMLPosFlyFrms )
                    {
                        for( sal_uInt16 i=0; i<pHTMLPosFlyFrms->size(); i++ )
                        {
                            sal_uLong nIdx = (*pHTMLPosFlyFrms)[i]
                                                ->GetNdIndex().GetIndex();
                            bEmpty = (nIdx != nStt) && (nIdx != nStt-1);
                            if( !bEmpty || nIdx > nStt )
                                break;
                        }
                    }
                }
                if( bEmpty )
                {
                    SvxBrushItem aBrush = rFrmFmt.makeBackgroundBrushItem();
                    /// background is not empty, if it has a background graphic
                    /// or its background color is not "no fill"/"auto fill".
                    if( GPOS_NONE != aBrush.GetGraphicPos() ||
                        aBrush.GetColor() != COL_TRANSPARENT )
                        bEmpty = false;
                }
                if( bEmpty )
                {
                    // leerer Rahmen
                    eType = HTML_FRMTYPE_EMPTY;
                }
                else if( pDoc->GetNodes()[nStt+1]->IsTableNode() )
                {
                    const SwTableNode *pTblNd =
                        pDoc->GetNodes()[nStt+1]->GetTableNode();
                    if( pTblNd->EndOfSectionIndex()+1 == nEnd )
                    {
                        // Tabelle mit Ueberschrift
                        eType = HTML_FRMTYPE_TABLE_CAP;
                    }
                }
            }
        }
    }

    return static_cast< sal_uInt16 >(eType);
}

void SwHTMLWriter::CollectFlyFrms()
{
    OSL_ENSURE( HTML_CFG_MAX+1 == MAX_BROWSERS,
            "number of browser configurations has changed" );

    SwPosFlyFrms aFlyPos(pDoc->GetAllFlyFmts(bWriteAll ? 0 : pCurPam, true));

    for(SwPosFlyFrms::const_iterator aIter(aFlyPos.begin()); aIter != aFlyPos.end(); ++aIter)
    {
        const SwFrmFmt& rFrmFmt = (*aIter)->GetFmt();
        const SdrObject *pSdrObj = 0;
        const SwPosition *pAPos;
        const SwCntntNode *pACNd;
        SwHTMLFrmType eType = (SwHTMLFrmType)GuessFrmType( rFrmFmt, pSdrObj );

        sal_uInt8 nMode;
        const SwFmtAnchor& rAnchor = rFrmFmt.GetAnchor();
        sal_Int16 eHoriRel = rFrmFmt.GetHoriOrient().GetRelationOrient();
        switch( rAnchor.GetAnchorId() )
        {
        case FLY_AT_PAGE:
        case FLY_AT_FLY:
            nMode = aHTMLOutFrmPageFlyTable[eType][nExportMode];
            break;

        case FLY_AT_PARA:
            // Absatz-gebundene Rahmen werden nur dann vor den
            // Absatz geschrieben, wenn der Absatz einen Abstand
            // hat.
            if( text::RelOrientation::FRAME == eHoriRel &&
                (pAPos = rAnchor.GetCntntAnchor()) != 0 &&
                (pACNd = pAPos->nNode.GetNode().GetCntntNode()) != 0 )
            {
                const SvxLRSpaceItem& rLRItem =
                    (const SvxLRSpaceItem&)pACNd->GetAttr(RES_LR_SPACE);
                if( rLRItem.GetTxtLeft() || rLRItem.GetRight() )
                {
                    nMode = aHTMLOutFrmParaFrameTable[eType][nExportMode];
                    break;
                }
            }
            nMode = aHTMLOutFrmParaPrtAreaTable[eType][nExportMode];
            break;

        case FLY_AT_CHAR:
            if( text::RelOrientation::FRAME == eHoriRel || text::RelOrientation::PRINT_AREA == eHoriRel )
                nMode = aHTMLOutFrmParaPrtAreaTable[eType][nExportMode];
            else
                nMode = aHTMLOutFrmParaOtherTable[eType][nExportMode];
            break;

        default:
            nMode = aHTMLOutFrmParaPrtAreaTable[eType][nExportMode];
            break;
        }

        if( !pHTMLPosFlyFrms )
            pHTMLPosFlyFrms = new SwHTMLPosFlyFrms;

        SwHTMLPosFlyFrm *pNew = new SwHTMLPosFlyFrm(**aIter, pSdrObj, nMode);
        pHTMLPosFlyFrms->insert( pNew );
    }
}

bool SwHTMLWriter::OutFlyFrm( sal_uLong nNdIdx, sal_Int32 nCntntIdx, sal_uInt8 nPos,
                              HTMLOutContext *pContext )
{
    bool bFlysLeft = false; // Noch Flys an aktueller Node-Position da?

    // OutFlyFrm kan rekursiv aufgerufen werden. Deshalb muss man
    // manchmal wieder von vorne anfangen, nachdem ein Fly ausgegeben
    // wurde.
    bool bRestart = true;
    while( pHTMLPosFlyFrms && bRestart )
    {
        bFlysLeft = bRestart = false;

        // suche nach dem Anfang der FlyFrames
        sal_uInt16 i;

        for( i = 0; i < pHTMLPosFlyFrms->size() &&
            (*pHTMLPosFlyFrms)[i]->GetNdIndex().GetIndex() < nNdIdx; i++ )
            ;
        for( ; !bRestart && i < pHTMLPosFlyFrms->size() &&
            (*pHTMLPosFlyFrms)[i]->GetNdIndex().GetIndex() == nNdIdx; i++ )
        {
            SwHTMLPosFlyFrm *pPosFly = (*pHTMLPosFlyFrms)[i];
            if( ( HTML_POS_ANY == nPos ||
                  pPosFly->GetOutPos() == nPos ) &&
                pPosFly->GetCntntIndex() == nCntntIdx )
            {
                // Erst entfernen ist wichtig, weil in tieferen
                // Rekursionen evtl. weitere Eintraege oder das
                // ganze Array geloscht werden koennte.
                pHTMLPosFlyFrms->erase(i);
                i--;
                if( pHTMLPosFlyFrms->empty() )
                {
                    delete pHTMLPosFlyFrms;
                    pHTMLPosFlyFrms = 0;
                    bRestart = true;    // nicht wirklich, nur raus
                                        // aus der Schleife
                }

                if( pContext )
                {
                    HTMLOutFuncs::FlushToAscii(Strm(), *pContext );
                    pContext = 0; // one time only
                }

                OutFrmFmt( pPosFly->GetOutMode(), pPosFly->GetFmt(),
                           pPosFly->GetSdrObject() );
                switch( pPosFly->GetOutFn() )
                {
                case HTML_OUT_DIV:
                case HTML_OUT_SPAN:
                case HTML_OUT_MULTICOL:
                case HTML_OUT_TBLNODE:
                    bRestart = true; // Hier wird's evtl rekursiv
                    break;
                }
                delete pPosFly;
            }
            else
            {
                bFlysLeft = true;
            }
        }
    }

    return bFlysLeft;
}

void SwHTMLWriter::OutFrmFmt( sal_uInt8 nMode, const SwFrmFmt& rFrmFmt,
                              const SdrObject *pSdrObject )
{
    sal_uInt8 nCntnrMode = SwHTMLPosFlyFrm::GetOutCntnr( nMode );
    sal_uInt8 nOutMode = SwHTMLPosFlyFrm::GetOutFn(nMode);
    const sal_Char *pCntnrStr = 0;
    if( HTML_CNTNR_NONE != nCntnrMode )
    {

        if( bLFPossible && HTML_CNTNR_DIV == nCntnrMode )
            OutNewLine();

        OStringBuffer sOut;
        pCntnrStr = (HTML_CNTNR_DIV == nCntnrMode)
                            ? OOO_STRING_SVTOOLS_HTML_division
                            : OOO_STRING_SVTOOLS_HTML_span;
        sOut.append('<').append(pCntnrStr).append(' ')
            .append(OOO_STRING_SVTOOLS_HTML_O_class).append("=\"")
            .append("sd-abs-pos").append('\"');
        Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );

        // Fuer Nicht-Zeichenobekte eine Breite ausgeben
        sal_uLong nFrmFlags = HTML_FRMOPTS_CNTNR;

        // Fuer spaltige Rahmen koennen wir auch noch den Hintergrund ausgeben.
        if( HTML_OUT_MULTICOL == nOutMode )
            nFrmFlags |= HTML_FRMOPT_S_BACKGROUND|HTML_FRMOPT_S_BORDER;

        if( IsHTMLMode( HTMLMODE_BORDER_NONE ) )
           nFrmFlags |= HTML_FRMOPT_S_NOBORDER;
        OutCSS1_FrmFmtOptions( rFrmFmt, nFrmFlags, pSdrObject );
        Strm().WriteChar( '>' );

        if( HTML_CNTNR_DIV == nCntnrMode )
        {
            IncIndentLevel();
            bLFPossible = true;
        }
    }

    switch( nOutMode )
    {
    case HTML_OUT_TBLNODE:      // OK
        OSL_ENSURE( !pCntnrStr, "Table: Container ist hier nicht vorgesehen" );
        OutHTML_FrmFmtTableNode( *this, rFrmFmt );
        break;
    case HTML_OUT_GRFNODE:      // OK
        OutHTML_FrmFmtGrfNode( *this, rFrmFmt, pCntnrStr != 0 );
        break;
    case HTML_OUT_OLENODE:      // OK
        OutHTML_FrmFmtOLENode( *this, rFrmFmt, pCntnrStr != 0 );
        break;
    case HTML_OUT_OLEGRF:       // OK
        OutHTML_FrmFmtOLENodeGrf( *this, rFrmFmt, pCntnrStr != 0 );
        break;
    case HTML_OUT_DIV:
    case HTML_OUT_SPAN:
        OSL_ENSURE( !pCntnrStr, "Div: Container ist hier nicht vorgesehen" );
        OutHTML_FrmFmtAsDivOrSpan( *this, rFrmFmt, HTML_OUT_SPAN==nOutMode );
        break;
    case HTML_OUT_MULTICOL:     // OK
        OutHTML_FrmFmtAsMulticol( *this, rFrmFmt, pCntnrStr != 0 );
        break;
    case HTML_OUT_SPACER:       // OK
        OSL_ENSURE( !pCntnrStr, "Spacer: Container ist hier nicht vorgesehen" );
        OutHTML_FrmFmtAsSpacer( *this, rFrmFmt );
        break;
    case HTML_OUT_CONTROL:      // OK
        OutHTML_DrawFrmFmtAsControl( *this,
                                    (const SwDrawFrmFmt &)rFrmFmt, *pSdrObject,
                                    pCntnrStr != 0 );
        break;
    case HTML_OUT_AMARQUEE:
        OutHTML_FrmFmtAsMarquee( *this, rFrmFmt, *pSdrObject );
        break;
    case HTML_OUT_MARQUEE:
        OSL_ENSURE( !pCntnrStr, "Marquee: Container ist hier nicht vorgesehen" );
        OutHTML_DrawFrmFmtAsMarquee( *this,
                    (const SwDrawFrmFmt &)rFrmFmt, *pSdrObject );
        break;
    case HTML_OUT_GRFFRM:
        OutHTML_FrmFmtAsImage( *this, rFrmFmt, pCntnrStr != 0 );
        break;
    }

    if( HTML_CNTNR_DIV == nCntnrMode )
    {
        DecIndentLevel();
        if( bLFPossible )
            OutNewLine();
        HTMLOutFuncs::Out_AsciiTag( Strm(), OOO_STRING_SVTOOLS_HTML_division, false );
        bLFPossible = true;
    }
    else if( HTML_CNTNR_SPAN == nCntnrMode )
        HTMLOutFuncs::Out_AsciiTag( Strm(), OOO_STRING_SVTOOLS_HTML_span, false );
}

OString SwHTMLWriter::OutFrmFmtOptions( const SwFrmFmt &rFrmFmt,
                                     const OUString& rAlternateTxt,
                                     sal_uInt32 nFrmOpts,
                                     const OString &rEndTags )
{
    OString sRetEndTags(rEndTags);
    OStringBuffer sOut;
    const SfxPoolItem* pItem;
    const SfxItemSet& rItemSet = rFrmFmt.GetAttrSet();

    // Name
    if( (nFrmOpts & (HTML_FRMOPT_ID|HTML_FRMOPT_NAME)) &&
        !rFrmFmt.GetName().isEmpty() )
    {
        const sal_Char *pStr =
            (nFrmOpts & HTML_FRMOPT_ID) ? OOO_STRING_SVTOOLS_HTML_O_id : OOO_STRING_SVTOOLS_HTML_O_name;
        sOut.append(' ').append(pStr).
            append("=\"");
        Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
        HTMLOutFuncs::Out_String( Strm(), rFrmFmt.GetName(), eDestEnc, &aNonConvertableCharacters );
        sOut.append('\"');
    }

    // Name
    if( nFrmOpts & HTML_FRMOPT_DIR )
    {
        sal_uInt16 nDir = GetHTMLDirection( rItemSet );
        Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
        OutDirection( nDir );
    }

    // ALT
    if( (nFrmOpts & HTML_FRMOPT_ALT) && !rAlternateTxt.isEmpty() )
    {
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_alt).
            append("=\"");
        Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
        HTMLOutFuncs::Out_String( Strm(), rAlternateTxt, eDestEnc, &aNonConvertableCharacters );
        sOut.append('\"');
    }

    // ALIGN
    const sal_Char *pStr = 0;
    RndStdIds eAnchorId = rFrmFmt.GetAnchor().GetAnchorId();
    if( (nFrmOpts & HTML_FRMOPT_ALIGN) &&
        ((FLY_AT_PARA == eAnchorId) || (FLY_AT_CHAR == eAnchorId)) )
    {
        // MIB 12.3.98: Ist es nicht schlauer, absatzgebundene
        // Rahmen notfalls links auszurichten als sie
        // zeichengebunden einzufuegen???
        const SwFmtHoriOrient& rHoriOri = rFrmFmt.GetHoriOrient();
        if( !(nFrmOpts & HTML_FRMOPT_S_ALIGN) ||
            text::RelOrientation::FRAME == rHoriOri.GetRelationOrient() ||
            text::RelOrientation::PRINT_AREA == rHoriOri.GetRelationOrient() )
        {
            pStr = text::HoriOrientation::RIGHT == rHoriOri.GetHoriOrient()
                        ? OOO_STRING_SVTOOLS_HTML_AL_right
                        : OOO_STRING_SVTOOLS_HTML_AL_left;
        }
    }
    if( (nFrmOpts & HTML_FRMOPT_ALIGN) && !pStr &&
        ( (nFrmOpts & HTML_FRMOPT_S_ALIGN) == 0 ||
          (FLY_AS_CHAR == eAnchorId) ) &&
        SfxItemState::SET == rItemSet.GetItemState( RES_VERT_ORIENT, true, &pItem ))
    {
        switch( ((SwFmtVertOrient*)pItem)->GetVertOrient() )
        {
        case text::VertOrientation::LINE_TOP:     pStr = OOO_STRING_SVTOOLS_HTML_VA_top;        break;
        case text::VertOrientation::CHAR_TOP:
        case text::VertOrientation::BOTTOM:       pStr = OOO_STRING_SVTOOLS_HTML_VA_texttop;    break;  // geht nicht
        case text::VertOrientation::LINE_CENTER:
        case text::VertOrientation::CHAR_CENTER:  pStr = OOO_STRING_SVTOOLS_HTML_VA_absmiddle;  break;  // geht nicht
        case text::VertOrientation::CENTER:       pStr = OOO_STRING_SVTOOLS_HTML_VA_middle;     break;
        case text::VertOrientation::LINE_BOTTOM:
        case text::VertOrientation::CHAR_BOTTOM:  pStr = OOO_STRING_SVTOOLS_HTML_VA_absbottom;  break;  // geht nicht
        case text::VertOrientation::TOP:          pStr = OOO_STRING_SVTOOLS_HTML_VA_bottom;     break;
        case text::VertOrientation::NONE:     break;
        }
    }
    if( pStr )
    {
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_align).append("=\"").
            append(pStr).append("\"");
    }

    // HSPACE und VSPACE
    Size aTwipSpc( 0, 0 );
    if( (nFrmOpts & (HTML_FRMOPT_SPACE|HTML_FRMOPT_MARGINSIZE)) &&
        SfxItemState::SET == rItemSet.GetItemState( RES_LR_SPACE, true, &pItem ))
    {
        aTwipSpc.Width() =
            ( ((SvxLRSpaceItem*)pItem)->GetLeft() +
                ((SvxLRSpaceItem*)pItem)->GetRight() ) / 2;
        nDfltLeftMargin = nDfltRightMargin = aTwipSpc.Width();
    }
    if( (nFrmOpts & (HTML_FRMOPT_SPACE|HTML_FRMOPT_MARGINSIZE)) &&
        SfxItemState::SET == rItemSet.GetItemState( RES_UL_SPACE, true, &pItem ))
    {
        aTwipSpc.Height()  =
            ( ((SvxULSpaceItem*)pItem)->GetUpper() +
                ((SvxULSpaceItem*)pItem)->GetLower() ) / 2;
        nDfltTopMargin = nDfltBottomMargin = (sal_uInt16)aTwipSpc.Height();
    }

    if( (nFrmOpts & HTML_FRMOPT_SPACE) &&
        (aTwipSpc.Width() || aTwipSpc.Height()) &&
        Application::GetDefaultDevice() )
    {
        Size aPixelSpc =
            Application::GetDefaultDevice()->LogicToPixel( aTwipSpc,
                                                MapMode(MAP_TWIP) );
        if( !aPixelSpc.Width() && aTwipSpc.Width() )
            aPixelSpc.Width() = 1;
        if( !aPixelSpc.Height() && aTwipSpc.Height() )
            aPixelSpc.Height() = 1;

        if( aPixelSpc.Width() )
        {
            sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_hspace).
                append("=\"").append(static_cast<sal_Int32>(aPixelSpc.Width())).append("\"");
        }

        if( aPixelSpc.Height() )
        {
            sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_vspace).
                append("=\"").append(static_cast<sal_Int32>(aPixelSpc.Height())).append("\"");
        }
    }

    // Der Abstand muss bei der Groesse beruecksichtigt, wenn das entsprechende
    // Flag gesetzt ist.
    if( (nFrmOpts & HTML_FRMOPT_MARGINSIZE) )
    {
        aTwipSpc.Width() *= -2;
        aTwipSpc.Height() *= -2;
    }
    else
    {
        aTwipSpc.Width() = 0;
        aTwipSpc.Height() = 0;
    }

    if( !(nFrmOpts & HTML_FRMOPT_ABSSIZE) &&
        SfxItemState::SET == rItemSet.GetItemState( RES_BOX, true, &pItem ))
    {
        const SvxBoxItem* pBoxItem = (const SvxBoxItem*)pItem;

        aTwipSpc.Width() += pBoxItem->CalcLineSpace( BOX_LINE_LEFT );
        aTwipSpc.Width() += pBoxItem->CalcLineSpace( BOX_LINE_RIGHT );
        aTwipSpc.Height() += pBoxItem->CalcLineSpace( BOX_LINE_TOP );
        aTwipSpc.Height() += pBoxItem->CalcLineSpace( BOX_LINE_BOTTOM );
    }

    // WIDTH und/oder HEIGHT
    // ATT_VAR_SIZE/ATT_MIN_SIZE nur ausgeben, wenn ANYSIZE gesezut ist
    if( (nFrmOpts & HTML_FRMOPT_SIZE) &&
        SfxItemState::SET == rItemSet.GetItemState( RES_FRM_SIZE, true, &pItem ) &&
        ( (nFrmOpts & HTML_FRMOPT_ANYSIZE) ||
          ATT_FIX_SIZE == ((const SwFmtFrmSize *)pItem)->GetHeightSizeType()) )
    {
        const SwFmtFrmSize *pFSItem = (const SwFmtFrmSize *)pItem;
        sal_uInt8 nPrcWidth = pFSItem->GetWidthPercent();
        sal_uInt8 nPrcHeight = pFSItem->GetHeightPercent();

        // Groesse des Objekts Twips ohne Raender
        Size aTwipSz( (nPrcWidth ? 0
                                 : pFSItem->GetWidth()-aTwipSpc.Width()),
                      (nPrcHeight ? 0
                                  : pFSItem->GetHeight()-aTwipSpc.Height()) );

        OSL_ENSURE( aTwipSz.Width() >= 0 && aTwipSz.Height() >= 0,
                "Rahmengroesse minus Abstand < 0!!!???" );
        if( aTwipSz.Width() < 0 )
            aTwipSz.Width() = 0;
        if( aTwipSz.Height() < 0 )
            aTwipSz.Height() = 0;

        Size aPixelSz( 0, 0 );
        if( (aTwipSz.Width() || aTwipSz.Height()) &&
            Application::GetDefaultDevice() )
        {
            aPixelSz =
                Application::GetDefaultDevice()->LogicToPixel( aTwipSz,
                                                    MapMode(MAP_TWIP) );
            if( !aPixelSz.Width() && aTwipSz.Width() )
                aPixelSz.Width() = 1;
            if( !aPixelSz.Height() && aTwipSz.Height() )
                aPixelSz.Height() = 1;
        }

        if( (nFrmOpts & HTML_FRMOPT_WIDTH) &&
            ((nPrcWidth && nPrcWidth!=255) || aPixelSz.Width()) )
        {
            sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_width).
                append("=\"");
            if( nPrcWidth )
                sOut.append(static_cast<sal_Int32>(nPrcWidth)).append('%');
            else
                sOut.append(static_cast<sal_Int32>(aPixelSz.Width()));
            sOut.append("\"");
        }

        if( (nFrmOpts & HTML_FRMOPT_HEIGHT) &&
            ((nPrcHeight && nPrcHeight!=255) || aPixelSz.Height()) )
        {
            sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_height).
                append("=\"");
            if( nPrcHeight )
                sOut.append(static_cast<sal_Int32>(nPrcHeight)).append('%');
            else
                sOut.append(static_cast<sal_Int32>(aPixelSz.Height()));
            sOut.append("\"");
        }
    }

    if (!sOut.isEmpty())
        Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );

    // Umlauf fuer absatzgeb. Grafiken als <BR CLEAR=...> in den String
    // schreiben
    if( (nFrmOpts & HTML_FRMOPT_BRCLEAR) &&
        ((FLY_AT_PARA == rFrmFmt.GetAnchor().GetAnchorId()) ||
         (FLY_AT_CHAR == rFrmFmt.GetAnchor().GetAnchorId())) &&
        SfxItemState::SET == rItemSet.GetItemState( RES_SURROUND, true, &pItem ))
    {
        const SwFmtSurround* pSurround = (const SwFmtSurround*)pItem;
        sal_Int16 eHoriOri =    rFrmFmt.GetHoriOrient().GetHoriOrient();
        pStr = 0;
        SwSurround eSurround = pSurround->GetSurround();
        bool bAnchorOnly = pSurround->IsAnchorOnly();
        switch( eHoriOri )
        {
        case text::HoriOrientation::RIGHT:
            {
                switch( eSurround )
                {
                case SURROUND_NONE:
                case SURROUND_RIGHT:
                    pStr = OOO_STRING_SVTOOLS_HTML_AL_right;
                    break;
                case SURROUND_LEFT:
                case SURROUND_PARALLEL:
                    if( bAnchorOnly )
                        bClearRight = true;
                    break;
                default:
                    ;
                }
            }
            break;

        default:
            // If a frame is centered, it gets left aligned. This
            // should be taken into account here, too.
            {
                switch( eSurround )
                {
                case SURROUND_NONE:
                case SURROUND_LEFT:
                    pStr = OOO_STRING_SVTOOLS_HTML_AL_left;
                    break;
                case SURROUND_RIGHT:
                case SURROUND_PARALLEL:
                    if( bAnchorOnly )
                        bClearLeft = true;
                    break;
                default:
                    ;
                }
            }
            break;

        }

        if( pStr )
        {
            sOut.append('<').append(OOO_STRING_SVTOOLS_HTML_linebreak).
                append(' ').append(OOO_STRING_SVTOOLS_HTML_O_clear).
                append("=\"").append(pStr).append("\">").append(rEndTags);
            sRetEndTags = sOut.makeStringAndClear();
        }
    }
    assert(sRetEndTags.endsWith(rEndTags)); // fdo#58286
    return sRetEndTags;
}

void SwHTMLWriter::writeFrameFormatOptions(HtmlWriter& aHtml, const SwFrmFmt& rFrmFmt, const OUString& rAlternateText, sal_uInt32 nFrameOptions)
{
    const SfxPoolItem* pItem;
    const SfxItemSet& rItemSet = rFrmFmt.GetAttrSet();

    // Name
    if( (nFrameOptions & (HTML_FRMOPT_ID|HTML_FRMOPT_NAME)) &&
        !rFrmFmt.GetName().isEmpty() )
    {
        const sal_Char* pAttributeName = (nFrameOptions & HTML_FRMOPT_ID) ? OOO_STRING_SVTOOLS_HTML_O_id : OOO_STRING_SVTOOLS_HTML_O_name;
        aHtml.attribute(pAttributeName, rFrmFmt.GetName());
    }

    // Name
    if (nFrameOptions & HTML_FRMOPT_DIR)
    {
        sal_uInt16 nCurrentDirection = GetHTMLDirection(rItemSet);
        OString sDirection = convertDirection(nCurrentDirection);
        aHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_dir, sDirection);
    }

    // alt
    if( (nFrameOptions & HTML_FRMOPT_ALT) && !rAlternateText.isEmpty() )
    {
        aHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_alt, rAlternateText);
    }

    // align
    const sal_Char* pAlignString = 0;
    RndStdIds eAnchorId = rFrmFmt.GetAnchor().GetAnchorId();
    if( (nFrameOptions & HTML_FRMOPT_ALIGN) &&
        ((FLY_AT_PARA == eAnchorId) || (FLY_AT_CHAR == eAnchorId)) )
    {
        const SwFmtHoriOrient& rHoriOri = rFrmFmt.GetHoriOrient();
        if( !(nFrameOptions & HTML_FRMOPT_S_ALIGN) ||
            text::RelOrientation::FRAME == rHoriOri.GetRelationOrient() ||
            text::RelOrientation::PRINT_AREA == rHoriOri.GetRelationOrient() )
        {
            pAlignString = text::HoriOrientation::RIGHT == rHoriOri.GetHoriOrient()
                        ? OOO_STRING_SVTOOLS_HTML_AL_right
                        : OOO_STRING_SVTOOLS_HTML_AL_left;
        }
    }
    if( (nFrameOptions & HTML_FRMOPT_ALIGN) && !pAlignString &&
        ( (nFrameOptions & HTML_FRMOPT_S_ALIGN) == 0 ||
          (FLY_AS_CHAR == eAnchorId) ) &&
        SfxItemState::SET == rItemSet.GetItemState( RES_VERT_ORIENT, true, &pItem ))
    {
        switch( ((SwFmtVertOrient*)pItem)->GetVertOrient() )
        {
        case text::VertOrientation::LINE_TOP:     pAlignString = OOO_STRING_SVTOOLS_HTML_VA_top;        break;
        case text::VertOrientation::CHAR_TOP:
        case text::VertOrientation::BOTTOM:       pAlignString = OOO_STRING_SVTOOLS_HTML_VA_texttop;    break;
        case text::VertOrientation::LINE_CENTER:
        case text::VertOrientation::CHAR_CENTER:  pAlignString = OOO_STRING_SVTOOLS_HTML_VA_absmiddle;  break;
        case text::VertOrientation::CENTER:       pAlignString = OOO_STRING_SVTOOLS_HTML_VA_middle;     break;
        case text::VertOrientation::LINE_BOTTOM:
        case text::VertOrientation::CHAR_BOTTOM:  pAlignString = OOO_STRING_SVTOOLS_HTML_VA_absbottom;  break;
        case text::VertOrientation::TOP:          pAlignString = OOO_STRING_SVTOOLS_HTML_VA_bottom;     break;
        case text::VertOrientation::NONE:     break;
        }
    }
    if (pAlignString)
    {
        aHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_align, pAlignString);
    }

    // hspace und vspace
    Size aTwipSpc( 0, 0 );
    if( (nFrameOptions & (HTML_FRMOPT_SPACE | HTML_FRMOPT_MARGINSIZE)) &&
        SfxItemState::SET == rItemSet.GetItemState( RES_LR_SPACE, true, &pItem ))
    {
        aTwipSpc.Width() =
            ( ((SvxLRSpaceItem*)pItem)->GetLeft() +
                ((SvxLRSpaceItem*)pItem)->GetRight() ) / 2;
        nDfltLeftMargin = nDfltRightMargin = aTwipSpc.Width();
    }
    if( (nFrameOptions & (HTML_FRMOPT_SPACE|HTML_FRMOPT_MARGINSIZE)) &&
        SfxItemState::SET == rItemSet.GetItemState( RES_UL_SPACE, true, &pItem ))
    {
        aTwipSpc.Height()  =
            ( ((SvxULSpaceItem*)pItem)->GetUpper() +
                ((SvxULSpaceItem*)pItem)->GetLower() ) / 2;
        nDfltTopMargin = nDfltBottomMargin = (sal_uInt16)aTwipSpc.Height();
    }

    if( (nFrameOptions & HTML_FRMOPT_SPACE) &&
        (aTwipSpc.Width() || aTwipSpc.Height()) &&
        Application::GetDefaultDevice() )
    {
        Size aPixelSpc =
            Application::GetDefaultDevice()->LogicToPixel( aTwipSpc,
                                                MapMode(MAP_TWIP) );
        if( !aPixelSpc.Width() && aTwipSpc.Width() )
            aPixelSpc.Width() = 1;
        if( !aPixelSpc.Height() && aTwipSpc.Height() )
            aPixelSpc.Height() = 1;

        if (aPixelSpc.Width())
        {
            aHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_hspace, static_cast<sal_Int32>(aPixelSpc.Width()));
        }

        if (aPixelSpc.Height())
        {
            aHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_vspace, static_cast<sal_Int32>(aPixelSpc.Height()));
        }
    }

    // Der Abstand muss bei der Groesse beruecksichtigt, wenn das entsprechende
    // Flag gesetzt ist.
    if( (nFrameOptions & HTML_FRMOPT_MARGINSIZE) )
    {
        aTwipSpc.Width() *= -2;
        aTwipSpc.Height() *= -2;
    }
    else
    {
        aTwipSpc.Width() = 0;
        aTwipSpc.Height() = 0;
    }

    if( !(nFrameOptions & HTML_FRMOPT_ABSSIZE) &&
        SfxItemState::SET == rItemSet.GetItemState( RES_BOX, true, &pItem ))
    {
        const SvxBoxItem* pBoxItem = (const SvxBoxItem*)pItem;

        aTwipSpc.Width() += pBoxItem->CalcLineSpace( BOX_LINE_LEFT );
        aTwipSpc.Width() += pBoxItem->CalcLineSpace( BOX_LINE_RIGHT );
        aTwipSpc.Height() += pBoxItem->CalcLineSpace( BOX_LINE_TOP );
        aTwipSpc.Height() += pBoxItem->CalcLineSpace( BOX_LINE_BOTTOM );
    }

    // "width" and/or "height"
    // ATT_VAR_SIZE/ATT_MIN_SIZE nur ausgeben, wenn ANYSIZE gesezut ist
    if( (nFrameOptions & HTML_FRMOPT_SIZE) &&
        SfxItemState::SET == rItemSet.GetItemState( RES_FRM_SIZE, true, &pItem ) &&
        ( (nFrameOptions & HTML_FRMOPT_ANYSIZE) ||
          ATT_FIX_SIZE == ((const SwFmtFrmSize *)pItem)->GetHeightSizeType()) )
    {
        const SwFmtFrmSize *pFSItem = (const SwFmtFrmSize *)pItem;
        sal_uInt8 nPrcWidth = pFSItem->GetWidthPercent();
        sal_uInt8 nPrcHeight = pFSItem->GetHeightPercent();

        // Groesse des Objekts Twips ohne Raender
        Size aTwipSz( (nPrcWidth ? 0
                                 : pFSItem->GetWidth()-aTwipSpc.Width()),
                      (nPrcHeight ? 0
                                  : pFSItem->GetHeight()-aTwipSpc.Height()) );

        OSL_ENSURE( aTwipSz.Width() >= 0 && aTwipSz.Height() >= 0,
                "Rahmengroesse minus Abstand < 0!!!???" );
        if( aTwipSz.Width() < 0 )
            aTwipSz.Width() = 0;
        if( aTwipSz.Height() < 0 )
            aTwipSz.Height() = 0;

        Size aPixelSz( 0, 0 );
        if( (aTwipSz.Width() || aTwipSz.Height()) &&
            Application::GetDefaultDevice() )
        {
            aPixelSz =
                Application::GetDefaultDevice()->LogicToPixel( aTwipSz,
                                                    MapMode(MAP_TWIP) );
            if( !aPixelSz.Width() && aTwipSz.Width() )
                aPixelSz.Width() = 1;
            if( !aPixelSz.Height() && aTwipSz.Height() )
                aPixelSz.Height() = 1;
        }

        if( (nFrameOptions & HTML_FRMOPT_WIDTH) &&
            ((nPrcWidth && nPrcWidth!=255) || aPixelSz.Width()) )
        {
            OString sWidth;
            if (nPrcWidth)
                sWidth = OString::number(static_cast<sal_Int32>(nPrcWidth)) + "%";
            else
                sWidth = OString::number(static_cast<sal_Int32>(aPixelSz.Width()));
            aHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_width, sWidth);
        }

        if( (nFrameOptions & HTML_FRMOPT_HEIGHT) &&
            ((nPrcHeight && nPrcHeight!=255) || aPixelSz.Height()) )
        {
            OString sHeight;
            if (nPrcHeight)
                sHeight = OString::number(static_cast<sal_Int32>(nPrcHeight)) + "%";
            else
                sHeight = OString::number(static_cast<sal_Int32>(aPixelSz.Height()));
            aHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_height, sHeight);
        }
    }

    // Umlauf fuer absatzgeb. Grafiken als <BR CLEAR=...> in den String
    // schreiben

    const sal_Char* pSurroundString = 0;
    if( (nFrameOptions & HTML_FRMOPT_BRCLEAR) &&
        ((FLY_AT_PARA == rFrmFmt.GetAnchor().GetAnchorId()) ||
         (FLY_AT_CHAR == rFrmFmt.GetAnchor().GetAnchorId())) &&
        SfxItemState::SET == rItemSet.GetItemState( RES_SURROUND, true, &pItem ))
    {
        const SwFmtSurround* pSurround = (const SwFmtSurround*)pItem;
        sal_Int16 eHoriOri = rFrmFmt.GetHoriOrient().GetHoriOrient();
        SwSurround eSurround = pSurround->GetSurround();
        bool bAnchorOnly = pSurround->IsAnchorOnly();
        switch( eHoriOri )
        {
            case text::HoriOrientation::RIGHT:
            {
                switch( eSurround )
                {
                case SURROUND_NONE:
                case SURROUND_RIGHT:
                    pSurroundString = OOO_STRING_SVTOOLS_HTML_AL_right;
                    break;
                case SURROUND_LEFT:
                case SURROUND_PARALLEL:
                    if( bAnchorOnly )
                        bClearRight = true;
                    break;
                default:
                    ;
                }
            }
            break;

            default:
            // If a frame is centered, it gets left aligned. This
            // should be taken into account here, too.
            {
                switch( eSurround )
                {
                case SURROUND_NONE:
                case SURROUND_LEFT:
                    pSurroundString = OOO_STRING_SVTOOLS_HTML_AL_left;
                    break;
                case SURROUND_RIGHT:
                case SURROUND_PARALLEL:
                    if( bAnchorOnly )
                        bClearLeft = true;
                    break;
                default:
                    break;
                }
            }
            break;
        }

        if (pSurroundString)
        {
            aHtml.start(OOO_STRING_SVTOOLS_HTML_linebreak);
            aHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_clear, pSurroundString);
            aHtml.end();
        }
    }
}

namespace
{

OUString lclWriteOutImap(SwHTMLWriter& rHTMLWrt, const SfxItemSet& rItemSet, const SwFrmFmt& rFrmFmt,
                         const Size& rRealSize, const ImageMap* pAltImgMap, SwFmtURL*& pURLItem)
{
    OUString aIMapName;

    const SfxPoolItem* pItem;

    // das URL-Attribut nur beruecksichtigen, wenn keine Image-Map
    // uebergeben wurde
    if (!pAltImgMap && SfxItemState::SET == rItemSet.GetItemState( RES_URL, true, &pItem))
    {
        pURLItem = (SwFmtURL*) pItem;
    }

    // Image-Map rausschreiben
    const ImageMap* pIMap = pAltImgMap;
    if( !pIMap && pURLItem )
    {
        pIMap = pURLItem->GetMap();
    }

    if (pIMap)
    {
        // den Namen eindeutig machen
        aIMapName = pIMap->GetName();
        OUString aNameBase;
        if (!aIMapName.isEmpty())
            aNameBase = aIMapName;
        else
            aNameBase = OOO_STRING_SVTOOLS_HTML_map;

        if (aIMapName.isEmpty())
            aIMapName = aNameBase + OUString::number(rHTMLWrt.nImgMapCnt);

        bool bFound;
        do
        {
            bFound = false;
            for (size_t i = 0; i < rHTMLWrt.aImgMapNames.size(); ++i)
            {
                // TODO: Unicode: Comparison is case insensitive for ASCII
                // characters only now!
                if (aIMapName.equalsIgnoreAsciiCase(rHTMLWrt.aImgMapNames[i]))
                {
                    bFound = true;
                    break;
                }
            }
            if (bFound)
            {
                rHTMLWrt.nImgMapCnt++;
                aIMapName = aNameBase + OUString::number( rHTMLWrt.nImgMapCnt );
            }
        } while (bFound);

        bool bScale = false;
        boost::rational<long> aScaleX(1, 1);
        boost::rational<long> aScaleY(1, 1);

        const SwFmtFrmSize& rFrmSize = rFrmFmt.GetFrmSize();
        const SvxBoxItem& rBox = rFrmFmt.GetBox();

        if (!rFrmSize.GetWidthPercent() && rRealSize.Width())
        {
            SwTwips nWidth = rFrmSize.GetWidth();
            nWidth -= rBox.CalcLineSpace(BOX_LINE_LEFT) + rBox.CalcLineSpace(BOX_LINE_RIGHT);

            OSL_ENSURE( nWidth > 0, "Gibt es 0 twip breite Grafiken!?" );
            if (nWidth <= 0) // sollte nicht passieren
                nWidth = 1;

            if (rRealSize.Width() != nWidth)
            {
                aScaleX = boost::rational<long>(nWidth, rRealSize.Width());
                bScale = true;
            }
        }

        if (!rFrmSize.GetHeightPercent() && rRealSize.Height())
        {
            SwTwips nHeight = rFrmSize.GetHeight();

            nHeight -= rBox.CalcLineSpace(BOX_LINE_TOP) + rBox.CalcLineSpace(BOX_LINE_BOTTOM);

            OSL_ENSURE( nHeight > 0, "Gibt es 0 twip hohe Grafiken!?" );
            if (nHeight <= 0)
                nHeight = 1;

            if (rRealSize.Height() != nHeight)
            {
                aScaleY = boost::rational<long>(nHeight, rRealSize.Height());
                bScale = true;
            }
        }

        rHTMLWrt.aImgMapNames.push_back(aIMapName);

        OString aIndMap, aIndArea;
        const sal_Char *pIndArea = 0, *pIndMap = 0;

        if (rHTMLWrt.bLFPossible)
        {
            rHTMLWrt.OutNewLine( true );
            aIndMap = rHTMLWrt.GetIndentString();
            aIndArea = rHTMLWrt.GetIndentString(1);
            pIndArea = aIndArea.getStr();
            pIndMap = aIndMap.getStr();
        }

        if (bScale)
        {
            ImageMap aScaledIMap(*pIMap);
            aScaledIMap.Scale(aScaleX, aScaleY);
            HTMLOutFuncs::Out_ImageMap( rHTMLWrt.Strm(), rHTMLWrt.GetBaseURL(), aScaledIMap, aIMapName,
                                        aIMapEventTable,
                                        rHTMLWrt.bCfgStarBasic,
                                        SAL_NEWLINE_STRING, pIndArea, pIndMap,
                                        rHTMLWrt.eDestEnc,
                                        &rHTMLWrt.aNonConvertableCharacters );
        }
        else
        {
            HTMLOutFuncs::Out_ImageMap( rHTMLWrt.Strm(), rHTMLWrt.GetBaseURL(), *pIMap, aIMapName,
                                        aIMapEventTable,
                                        rHTMLWrt.bCfgStarBasic,
                                        SAL_NEWLINE_STRING, pIndArea, pIndMap,
                                         rHTMLWrt.eDestEnc,
                                        &rHTMLWrt.aNonConvertableCharacters );
        }
    }
    return aIMapName;
}

}

Writer& OutHTML_Image( Writer& rWrt, const SwFrmFmt &rFrmFmt,
                       Graphic& rGraphic, const OUString& rAlternateTxt,
                       const Size &rRealSize, sal_uInt32 nFrmOpts,
                       const sal_Char *pMarkType,
                       const ImageMap *pAltImgMap )
{
    SwHTMLWriter& rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);

    if (rHTMLWrt.mbSkipImages)
        return rHTMLWrt;

    // ggf. ein noch offenes Attribut voruebergehend beenden
    if( !rHTMLWrt.aINetFmts.empty() )
    {
        SwFmtINetFmt* pINetFmt = rHTMLWrt.aINetFmts.back();
        OutHTML_INetFmt( rWrt, *pINetFmt, false );
    }

    const SfxPoolItem* pItem;
    const SfxItemSet& rItemSet = rFrmFmt.GetAttrSet();

    SwFmtURL* pURLItem = 0;
    OUString aIMapName = lclWriteOutImap(rHTMLWrt, rItemSet, rFrmFmt, rRealSize, pAltImgMap, pURLItem);

    // put img into new line
    if( rHTMLWrt.bLFPossible )
        rHTMLWrt.OutNewLine( true );

    HtmlWriter aHtml(rWrt.Strm());

    // <a name=...></a>...<img ...>
    if( pMarkType && !rFrmFmt.GetName().isEmpty() )
    {
        rHTMLWrt.OutImplicitMark( rFrmFmt.GetName(), pMarkType );
    }

    // URL -> <a>...<img ... >...</a>
    const SvxMacroItem *pMacItem = 0;
    if (SfxItemState::SET == rItemSet.GetItemState(RES_FRMMACRO, true, &pItem))
    {
        pMacItem = (const SvxMacroItem *)pItem;
    }

    if (pURLItem || pMacItem)
    {
        OUString aMapURL;
        OUString aName;
        OUString aTarget;

        if(pURLItem)
        {
            aMapURL = pURLItem->GetURL();
            aName = pURLItem->GetName();
            aTarget = pURLItem->GetTargetFrameName();
        }

        bool bEvents = pMacItem && !pMacItem->GetMacroTable().empty();

        if( !aMapURL.isEmpty() || !aName.isEmpty() || !aTarget.isEmpty() || bEvents )
        {
            aHtml.start(OOO_STRING_SVTOOLS_HTML_anchor);

            // Output "href" element if a link or macro exists
            if( !aMapURL.isEmpty() || bEvents )
            {
                aHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_href, OUStringToOString(rHTMLWrt.convertHyperlinkHRefValue(aMapURL), RTL_TEXTENCODING_UTF8));
            }

            if( !aName.isEmpty() )
            {
                aHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_name, OUStringToOString(aName, RTL_TEXTENCODING_UTF8));
            }

            if( !aTarget.isEmpty() )
            {
                aHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_target, OUStringToOString(aTarget, RTL_TEXTENCODING_UTF8));
            }

            if( pMacItem )
            {
                const SvxMacroTableDtor& rMacTable = pMacItem->GetMacroTable();
                if (!rMacTable.empty())
                {
                    HtmlWriterHelper::applyEvents(aHtml, rMacTable, aAnchorEventTable, rHTMLWrt.bCfgStarBasic);
                }
            }
        }
    }

    // <font color = ...>...<img ... >...</font>
    sal_uInt16 nBorderWidth = 0;
    if( (nFrmOpts & HTML_FRMOPT_BORDER) &&
        SfxItemState::SET == rItemSet.GetItemState( RES_BOX, true, &pItem ))
    {
        Size aTwipBorder( 0, 0 );
        const SvxBoxItem* pBoxItem = (const SvxBoxItem*)pItem;

        const ::editeng::SvxBorderLine *pColBorderLine = 0;
        const ::editeng::SvxBorderLine *pBorderLine = pBoxItem->GetLeft();
        if( pBorderLine )
        {
            pColBorderLine = pBorderLine;
            aTwipBorder.Width() += pBorderLine->GetOutWidth();
        }

        pBorderLine = pBoxItem->GetRight();
        if( pBorderLine )
        {
            pColBorderLine = pBorderLine;
            aTwipBorder.Width() += pBorderLine->GetOutWidth();
        }

        pBorderLine = pBoxItem->GetTop();
        if( pBorderLine )
        {
            pColBorderLine = pBorderLine;
            aTwipBorder.Height() += pBorderLine->GetOutWidth();
        }

        pBorderLine = pBoxItem->GetBottom();
        if( pBorderLine )
        {
            pColBorderLine = pBorderLine;
            aTwipBorder.Height() += pBorderLine->GetOutWidth();
        }

        aTwipBorder.Width() /= 2;
        aTwipBorder.Height() /= 2;

        if( (aTwipBorder.Width() || aTwipBorder.Height()) &&
            Application::GetDefaultDevice() )
        {
            Size aPixelBorder =
                Application::GetDefaultDevice()->LogicToPixel( aTwipBorder,
                                                    MapMode(MAP_TWIP) );
            if( !aPixelBorder.Width() && aTwipBorder.Width() )
                aPixelBorder.Width() = 1;
            if( !aPixelBorder.Height() && aTwipBorder.Height() )
                aPixelBorder.Height() = 1;

            if( aPixelBorder.Width() )
                aPixelBorder.Height() = 0;

            nBorderWidth =
                (sal_uInt16)(aPixelBorder.Width() + aPixelBorder.Height());
        }

        if( pColBorderLine )
        {
            aHtml.start(OOO_STRING_SVTOOLS_HTML_font);
            HtmlWriterHelper::applyColor(aHtml, OOO_STRING_SVTOOLS_HTML_O_color, pColBorderLine->GetColor());
        }
    }

    aHtml.start(OOO_STRING_SVTOOLS_HTML_image);

    OUString aGraphicInBase64;
    sal_uLong nErr = XOutBitmap::GraphicToBase64(rGraphic, aGraphicInBase64);
    if (nErr)
    {
        rHTMLWrt.nWarn = WARN_SWG_POOR_LOAD | WARN_SW_WRITE_BASE;
    }

    OStringBuffer sBuffer;
    sBuffer.append(OOO_STRING_SVTOOLS_HTML_O_data);
    sBuffer.append(":");
    sBuffer.append(OUStringToOString(aGraphicInBase64, RTL_TEXTENCODING_UTF8));
    aHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_src, sBuffer.makeStringAndClear().getStr());

    // Events
    if (SfxItemState::SET == rItemSet.GetItemState(RES_FRMMACRO, true, &pItem))
    {
        const SvxMacroTableDtor& rMacTable = ((const SvxMacroItem *)pItem)->GetMacroTable();
        if (!rMacTable.empty())
        {
            HtmlWriterHelper::applyEvents(aHtml, rMacTable, aImageEventTable, rHTMLWrt.bCfgStarBasic);
        }
    }

    // alt, align, width, height, hspace, vspace
    rHTMLWrt.writeFrameFormatOptions(aHtml, rFrmFmt, rAlternateTxt, nFrmOpts);
    if( rHTMLWrt.IsHTMLMode( HTMLMODE_ABS_POS_FLY ) )
        rHTMLWrt.OutCSS1_FrmFmtOptions( rFrmFmt, nFrmOpts );

    if( nFrmOpts & HTML_FRMOPT_BORDER )
    {
        aHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_border, nBorderWidth);
    }

    if( pURLItem && pURLItem->IsServerMap() )
    {
        aHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_ismap);
    }

    if( !aIMapName.isEmpty() )
    {
        aHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_usemap, "#" + aIMapName);
    }

    aHtml.flushStack();

    if( !rHTMLWrt.aINetFmts.empty() )
    {
        // es ist noch ein Attribut auf dem Stack, das wieder geoeffnet
        // werden muss
        SwFmtINetFmt *pINetFmt = rHTMLWrt.aINetFmts.back();
        OutHTML_INetFmt( rWrt, *pINetFmt, true );
    }

    return rHTMLWrt;
}

Writer& OutHTML_BulletImage( Writer& rWrt,
                             const sal_Char *pTag,
                             const SvxBrushItem* pBrush )
{
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;

    OUString aGraphicInBase64;
    if( pBrush )
    {
        const Graphic* pGrf = pBrush->GetGraphic();
        if( pGrf )
        {
            sal_uLong nErr = XOutBitmap::GraphicToBase64(*pGrf, aGraphicInBase64);
            if( nErr )
            {
                rHTMLWrt.nWarn = WARN_SWG_POOR_LOAD | WARN_SW_WRITE_BASE;
            }
        }
    }

    OStringBuffer sOut;
    if( pTag )
        sOut.append('<').append(pTag);

    sOut.append(' ');
    sOut.append(OOO_STRING_SVTOOLS_HTML_O_style).append("=\"").
    append("list-style-image: ").append("url(").
    append(OOO_STRING_SVTOOLS_HTML_O_data).append(":");
    rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
    HTMLOutFuncs::Out_String( rWrt.Strm(), aGraphicInBase64, rHTMLWrt.eDestEnc, &rHTMLWrt.aNonConvertableCharacters );
    sOut.append(");").append('\"');

    if (pTag)
        sOut.append('>');
    rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );

    return rWrt;
}

static Writer& OutHTML_FrmFmtTableNode( Writer& rWrt, const SwFrmFmt& rFrmFmt )
{
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;

    const SwFmtCntnt& rFlyCntnt = rFrmFmt.GetCntnt();
    sal_uLong nStt = rFlyCntnt.GetCntntIdx()->GetIndex()+1;
    sal_uLong nEnd = rHTMLWrt.pDoc->GetNodes()[nStt-1]->EndOfSectionIndex();

    OUString aCaption;
    bool bTopCaption = false;

    // Nicht const, weil GetTable spater mal nicht const ist
    SwNode *pNd = rHTMLWrt.pDoc->GetNodes()[ nStt ];
    SwTableNode *pTblNd = pNd->GetTableNode();
    const SwTxtNode *pTxtNd = pNd->GetTxtNode();
    if( !pTblNd && pTxtNd )
    {
        // Tabelle mit Ueberschrift
        bTopCaption = true;
        pTblNd = rHTMLWrt.pDoc->GetNodes()[nStt+1]->GetTableNode();
    }
    OSL_ENSURE( pTblNd, "Rahmen enthaelt keine Tabelle" );
    if( pTblNd )
    {
        sal_uLong nTblEnd = pTblNd->EndOfSectionIndex();
        OSL_ENSURE( nTblEnd == nEnd - 1 ||
                (nTblEnd == nEnd - 2 && !bTopCaption),
                "Ungeuelter Rahmen-Inhalt fuer Tabelle" );

        if( nTblEnd == nEnd - 2 )
            pTxtNd = rHTMLWrt.pDoc->GetNodes()[nTblEnd+1]->GetTxtNode();
    }
    if( pTxtNd )
        aCaption = pTxtNd->GetTxt();

    if( pTblNd )
    {
        HTMLSaveData aSaveData( rHTMLWrt, pTblNd->GetIndex()+1,
                                pTblNd->EndOfSectionIndex(),
                                   true, &rFrmFmt );
        rHTMLWrt.bOutFlyFrame = true;
        OutHTML_SwTblNode( rHTMLWrt, *pTblNd, &rFrmFmt, &aCaption,
                           bTopCaption );
    }

    return rWrt;
}

static Writer & OutHTML_FrmFmtAsMulticol( Writer& rWrt,
                                          const SwFrmFmt& rFrmFmt,
                                          bool bInCntnr )
{
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;

    rHTMLWrt.ChangeParaToken( 0 );

    // Die aktulle <DL> beenden!
    rHTMLWrt.OutAndSetDefList( 0 );

    // als Multicol ausgeben
    if( rHTMLWrt.bLFPossible )
        rHTMLWrt.OutNewLine();

    OStringBuffer sOut;
    sOut.append('<').append(OOO_STRING_SVTOOLS_HTML_multicol);

    const SwFmtCol& rFmtCol = rFrmFmt.GetCol();

    // die Anzahl der Spalten als COLS ausgeben
    sal_uInt16 nCols = rFmtCol.GetNumCols();
    if( nCols )
    {
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_cols).
            append("=\"").append(static_cast<sal_Int32>(nCols)).append("\"");
    }

    // die Gutter-Breite (Minimalwert) als GUTTER
    sal_uInt16 nGutter = rFmtCol.GetGutterWidth( true );
    if( nGutter!=USHRT_MAX )
    {
        if( nGutter && Application::GetDefaultDevice() )
        {
            nGutter = (sal_uInt16)Application::GetDefaultDevice()
                            ->LogicToPixel( Size(nGutter,0),
                                            MapMode(MAP_TWIP) ).Width();
        }
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_gutter).
            append("=\"").append(static_cast<sal_Int32>(nGutter)).append("\"");
    }

    rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );

    // WIDTH
    sal_uLong nFrmFlags = bInCntnr ? HTML_FRMOPTS_MULTICOL_CNTNR
                                : HTML_FRMOPTS_MULTICOL;
    if( rHTMLWrt.IsHTMLMode( HTMLMODE_ABS_POS_FLY ) && !bInCntnr )
        nFrmFlags |= HTML_FRMOPTS_MULTICOL_CSS1;
    rHTMLWrt.OutFrmFmtOptions( rFrmFmt, aEmptyOUStr, nFrmFlags );
    if( rHTMLWrt.IsHTMLMode( HTMLMODE_ABS_POS_FLY ) && !bInCntnr )
        rHTMLWrt.OutCSS1_FrmFmtOptions( rFrmFmt, nFrmFlags );

    rWrt.Strm().WriteChar( '>' );

    rHTMLWrt.bLFPossible = true;
    rHTMLWrt.IncIndentLevel();  // den Inhalt von Multicol einruecken;

    const SwFmtCntnt& rFlyCntnt = rFrmFmt.GetCntnt();
    sal_uLong nStt = rFlyCntnt.GetCntntIdx()->GetIndex();
    const SwStartNode* pSttNd = rWrt.pDoc->GetNodes()[nStt]->GetStartNode();
    OSL_ENSURE( pSttNd, "Wo ist der Start-Node" );

    {
        // in einem Block damit rechtzeitig vor dem Ende der alte Zustand
        // wieder hergestellt wird.
        HTMLSaveData aSaveData( rHTMLWrt, nStt+1,
                                pSttNd->EndOfSectionIndex(),
                                   true, &rFrmFmt );
        rHTMLWrt.bOutFlyFrame = true;
        rHTMLWrt.Out_SwDoc( rWrt.pCurPam );
    }

    rHTMLWrt.DecIndentLevel();  // den Inhalt von Multicol einruecken;
    if( rHTMLWrt.bLFPossible )
        rHTMLWrt.OutNewLine();
    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_multicol, false );
    rHTMLWrt.bLFPossible = true;

    return rWrt;
}

static Writer& OutHTML_FrmFmtAsSpacer( Writer& rWrt, const SwFrmFmt& rFrmFmt )
{
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;

    // wenn meoglich vor der Grafik einen Zeilen-Umbruch ausgeben
    if( rHTMLWrt.bLFPossible )
        rHTMLWrt.OutNewLine( true );

    OStringBuffer sOut;
    sOut.append('<').append(OOO_STRING_SVTOOLS_HTML_spacer).append(' ')
        .append(OOO_STRING_SVTOOLS_HTML_O_type).append("=\"")
        .append(OOO_STRING_SVTOOLS_HTML_SPTYPE_block).append("\"");
    rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );

    // ALIGN, WIDTH, HEIGHT
    OString aEndTags = rHTMLWrt.OutFrmFmtOptions( rFrmFmt, aEmptyOUStr, HTML_FRMOPTS_SPACER );

    rWrt.Strm().WriteChar( '>' );
    if( !aEndTags.isEmpty() )
        rWrt.Strm().WriteCharPtr( aEndTags.getStr() );

    return rWrt;
}

static Writer& OutHTML_FrmFmtAsDivOrSpan( Writer& rWrt,
                                          const SwFrmFmt& rFrmFmt, bool bSpan)
{
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;

    const sal_Char *pStr = 0;
    if( !bSpan )
    {
        rHTMLWrt.ChangeParaToken( 0 );

        // Die aktulle <DL> beenden!
        rHTMLWrt.OutAndSetDefList( 0 );
        pStr = OOO_STRING_SVTOOLS_HTML_division;
    }
    else
        pStr = OOO_STRING_SVTOOLS_HTML_span;

    // als DIV ausgeben
    if( rHTMLWrt.bLFPossible )
        rHTMLWrt.OutNewLine();

    OStringBuffer sOut;
    sOut.append('<').append(pStr);

    rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
    sal_uLong nFrmFlags = HTML_FRMOPTS_DIV;
    if( rHTMLWrt.IsHTMLMode( HTMLMODE_BORDER_NONE ) )
       nFrmFlags |= HTML_FRMOPT_S_NOBORDER;
    OString aEndTags = rHTMLWrt.OutFrmFmtOptions( rFrmFmt, aEmptyOUStr, nFrmFlags );
    rHTMLWrt.OutCSS1_FrmFmtOptions( rFrmFmt, nFrmFlags );
    rWrt.Strm().WriteChar( '>' );

    rHTMLWrt.IncIndentLevel();  // den Inhalt einruecken
    rHTMLWrt.bLFPossible = true;

    const SwFmtCntnt& rFlyCntnt = rFrmFmt.GetCntnt();
    sal_uLong nStt = rFlyCntnt.GetCntntIdx()->GetIndex();

    // Am Start-Node verankerte Rahmen-gebundene Rahmen ausgeben
    rHTMLWrt.OutFlyFrm( nStt, 0, HTML_POS_ANY );

    const SwStartNode* pSttNd = rWrt.pDoc->GetNodes()[nStt]->GetStartNode();
    OSL_ENSURE( pSttNd, "Wo ist der Start-Node" );

    {
        // in einem Block damit rechtzeitig vor dem Ende der alte Zustand
        // wieder hergestellt wird.
        HTMLSaveData aSaveData( rHTMLWrt, nStt+1,
                                pSttNd->EndOfSectionIndex(),
                                   true, &rFrmFmt );
        rHTMLWrt.bOutFlyFrame = true;
        rHTMLWrt.Out_SwDoc( rWrt.pCurPam );
    }

    rHTMLWrt.DecIndentLevel();  // den Inhalt von Multicol einruecken;
    if( rHTMLWrt.bLFPossible )
        rHTMLWrt.OutNewLine();
    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), pStr, false );

    if( !aEndTags.isEmpty() )
        rWrt.Strm().WriteCharPtr( aEndTags.getStr() );

    return rWrt;
}

static Writer & OutHTML_FrmFmtAsImage( Writer& rWrt, const SwFrmFmt& rFrmFmt,
                                       bool /*bInCntnr*/ )
{
    SwHTMLWriter& rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);

    if (rHTMLWrt.mbSkipImages)
        return rWrt;

    ImageMap aIMap;
    Graphic aGraphic( ((SwFrmFmt &)rFrmFmt).MakeGraphic( &aIMap ) );
    Size aSz( 0, 0 );
    OutHTML_Image( rWrt, rFrmFmt, aGraphic, rFrmFmt.GetName(), aSz,
                    HTML_FRMOPTS_GENIMG, "frame",
                    aIMap.GetIMapObjectCount() ? &aIMap : 0 );

    return rWrt;
}

static Writer& OutHTML_FrmFmtGrfNode( Writer& rWrt, const SwFrmFmt& rFrmFmt,
                                      bool bInCntnr )
{
    SwHTMLWriter& rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);

    if (rHTMLWrt.mbSkipImages)
        return rWrt;

    const SwFmtCntnt& rFlyCntnt = rFrmFmt.GetCntnt();
    sal_uLong nStt = rFlyCntnt.GetCntntIdx()->GetIndex()+1;
    SwGrfNode *pGrfNd = rHTMLWrt.pDoc->GetNodes()[ nStt ]->GetGrfNode();
    OSL_ENSURE( pGrfNd, "Grf-Node erwartet" );
    if( !pGrfNd )
        return rWrt;

    sal_uLong nFrmFlags = bInCntnr ? HTML_FRMOPTS_IMG_CNTNR : HTML_FRMOPTS_IMG;
    if( rHTMLWrt.IsHTMLMode( HTMLMODE_ABS_POS_FLY ) && !bInCntnr )
         nFrmFlags |= HTML_FRMOPTS_IMG_CSS1;

    Graphic aGraphic = pGrfNd->GetGraphic();
    OutHTML_Image( rWrt, rFrmFmt, aGraphic, pGrfNd->GetTitle(),
                  pGrfNd->GetTwipSize(), nFrmFlags, "graphic" );

    return rWrt;
}

static Writer& OutHTML_FrmFmtAsMarquee( Writer& rWrt, const SwFrmFmt& rFrmFmt,
                                  const SdrObject& rSdrObj  )
{
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;

    // die Edit-Engine-Attribute des Objekts als SW-Attribute holen
    // und als Hints einsortieren
    const SfxItemSet& rFmtItemSet = rFrmFmt.GetAttrSet();
    SfxItemSet aItemSet( *rFmtItemSet.GetPool(), RES_CHRATR_BEGIN,
                                                 RES_CHRATR_END );
    SwHTMLWriter::GetEEAttrsFromDrwObj( aItemSet, &rSdrObj, true );
    bool bCfgOutStylesOld = rHTMLWrt.bCfgOutStyles;
    rHTMLWrt.bCfgOutStyles = false;
    rHTMLWrt.bTxtAttr = true;
    rHTMLWrt.bTagOn = true;
    Out_SfxItemSet( aHTMLAttrFnTab, rWrt, aItemSet, false );
    rHTMLWrt.bTxtAttr = false;

    OutHTML_DrawFrmFmtAsMarquee( rHTMLWrt,
                                 (const SwDrawFrmFmt &)rFrmFmt,
                                 rSdrObj );
    rHTMLWrt.bTxtAttr = true;
    rHTMLWrt.bTagOn = false;
    Out_SfxItemSet( aHTMLAttrFnTab, rWrt, aItemSet, false );
    rHTMLWrt.bTxtAttr = false;
    rHTMLWrt.bCfgOutStyles = bCfgOutStylesOld;

    return rWrt;
}

Writer& OutHTML_HeaderFooter( Writer& rWrt, const SwFrmFmt& rFrmFmt,
                              bool bHeader )
{
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;

    // als Multicol ausgeben
    rHTMLWrt.OutNewLine();
    OStringBuffer sOut;
    sOut.append(OOO_STRING_SVTOOLS_HTML_division).append(' ')
        .append(OOO_STRING_SVTOOLS_HTML_O_title).append("=\"")
        .append( bHeader ? "header" : "footer" ).append("\"");
    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sOut.makeStringAndClear().getStr() );

    rHTMLWrt.IncIndentLevel();  // den Inhalt von Multicol einruecken;

    // Einen Spacer fuer den Absatnd zusammenbasteln. Da durch das
    // <DL> bzw. </DL> immer einer Absatz-Abstand entsteht, wird der
    // ggf. abgezogen.
    const SvxULSpaceItem& rULSpace = rFrmFmt.GetULSpace();
    sal_uInt16 nSize = bHeader ? rULSpace.GetLower() : rULSpace.GetUpper();
    rHTMLWrt.nHeaderFooterSpace = nSize;

    OString aSpacer;
    if( rHTMLWrt.IsHTMLMode(HTMLMODE_VERT_SPACER) &&
        nSize > HTML_PARSPACE && Application::GetDefaultDevice() )
    {
        nSize -= HTML_PARSPACE;
        nSize = (sal_Int16)Application::GetDefaultDevice()
            ->LogicToPixel( Size(nSize,0), MapMode(MAP_TWIP) ).Width();

        aSpacer = OStringBuffer(OOO_STRING_SVTOOLS_HTML_spacer).
            append(' ').append(OOO_STRING_SVTOOLS_HTML_O_type).
            append("=\"").append(OOO_STRING_SVTOOLS_HTML_SPTYPE_vertical).append("\"").
            append(' ').append(OOO_STRING_SVTOOLS_HTML_O_size).
            append("=\"").append(static_cast<sal_Int32>(nSize)).append("\"").
            makeStringAndClear();
    }

    const SwFmtCntnt& rFlyCntnt = rFrmFmt.GetCntnt();
    sal_uLong nStt = rFlyCntnt.GetCntntIdx()->GetIndex();
    const SwStartNode* pSttNd = rWrt.pDoc->GetNodes()[nStt]->GetStartNode();
    OSL_ENSURE( pSttNd, "Wo ist der Start-Node" );

    if( !bHeader && !aSpacer.isEmpty() )
    {
        rHTMLWrt.OutNewLine();
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), aSpacer.getStr() );
    }

    {
        // in einem Block damit rechtzeitig vor dem Ende der alte Zustand
        // wieder hergestellt wird. pFlyFmt braucht hier nicht gestzt zu
        // werden, denn PageDesc-Attribute koennen hier nicht vorkommen
        HTMLSaveData aSaveData( rHTMLWrt, nStt+1,
                                pSttNd->EndOfSectionIndex() );

        if( bHeader )
            rHTMLWrt.bOutHeader = true;
        else
            rHTMLWrt.bOutFooter = true;

        rHTMLWrt.Out_SwDoc( rWrt.pCurPam );
    }

    if( bHeader && !aSpacer.isEmpty() )
    {
        rHTMLWrt.OutNewLine();
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), aSpacer.getStr() );
    }

    rHTMLWrt.DecIndentLevel();  // den Inhalt von Multicol einruecken;
    rHTMLWrt.OutNewLine();
    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_division, false );

    rHTMLWrt.nHeaderFooterSpace = 0;

    return rWrt;
}

void SwHTMLWriter::AddLinkTarget( const OUString& rURL )
{
    if( rURL.isEmpty() || rURL[0] != '#' )
        return;

    // There might be a '|' as delimiter (if the link has been inserted
    // freshly) or a '%7c' or a '%7C' if the document has been saved and
    // loaded already.
    sal_Int32 nPos = rURL.getLength();
    bool bFound = false, bEncoded = false;
    while( !bFound && nPos > 0 )
    {
        sal_Unicode c = rURL[ --nPos ];
        switch( c )
        {
        case cMarkSeparator:
            bFound = true;
            break;
        case '%':
            bFound = (rURL.getLength() - nPos) >=3 &&
                     rURL[ nPos+1 ] == '7' &&
                     ((c =rURL[ nPos+2 ]) == 'C' || c == 'c');
            if( bFound )
                bEncoded = true;
        }
    }
    if( !bFound || nPos < 2 ) // mindetsens "#a|..."
        return;

    OUString aURL( rURL.copy( 1 ) );

    // nPos-1+1/3 (-1 wg. Erase)
    OUString sCmp(comphelper::string::remove(aURL.copy(bEncoded ? nPos+2 : nPos),
        ' '));
    if( sCmp.isEmpty() )
        return;

    sCmp = sCmp.toAsciiLowerCase();

    if( sCmp == "region" ||
        sCmp == "frame" ||
        sCmp == "graphic" ||
        sCmp == "ole" ||
        sCmp == "table" )
    {
        // Einfach nur in einem sortierten Array merken
        if( bEncoded )
        {
            aURL = aURL.replaceAt( nPos - 1, 3, OUString(cMarkSeparator)  );
        }
        aImplicitMarks.insert( aURL );
    }
    else if( sCmp == "outline" )
    {
        // Hier brauchen wir Position und Name. Deshalb sortieren wir
        // ein sal_uInt16 und ein String-Array selbst
        OUString aOutline( aURL.copy( 0, nPos-1 ) );
        SwPosition aPos( *pCurPam->GetPoint() );
        if( pDoc->GotoOutline( aPos, aOutline ) )
        {
            sal_uInt32 nIdx = aPos.nNode.GetIndex();

            sal_uInt32 nIns=0;
            while( nIns < aOutlineMarkPoss.size() &&
                   aOutlineMarkPoss[nIns] < nIdx )
                nIns++;

            aOutlineMarkPoss.insert( aOutlineMarkPoss.begin()+nIns, nIdx );
            if( bEncoded )
            {
                aURL = aURL.replaceAt( nPos - 1, 3, OUString(cMarkSeparator) );
            }
            aOutlineMarks.insert( aOutlineMarks.begin()+nIns, aURL );
        }
    }
    else if( sCmp == "text" )
    {

    }
}

void SwHTMLWriter::CollectLinkTargets()
{
    const SwFmtINetFmt* pINetFmt;
    const SwTxtINetFmt* pTxtAttr;
    const SwTxtNode* pTxtNd;

    sal_uInt32 n, nMaxItems = pDoc->GetAttrPool().GetItemCount2( RES_TXTATR_INETFMT );
    for( n = 0; n < nMaxItems; ++n )
    {
        if( 0 != (pINetFmt = (SwFmtINetFmt*)pDoc->GetAttrPool().GetItem2(
            RES_TXTATR_INETFMT, n ) ) &&
            0 != ( pTxtAttr = pINetFmt->GetTxtINetFmt()) &&
            0 != ( pTxtNd = pTxtAttr->GetpTxtNode() ) &&
            pTxtNd->GetNodes().IsDocNodes() )
        {
            AddLinkTarget( pINetFmt->GetValue() );
        }
    }

    const SwFmtURL *pURL;
    nMaxItems = pDoc->GetAttrPool().GetItemCount2( RES_URL );
    for( n = 0; n < nMaxItems; ++n )
    {
        if( 0 != (pURL = (SwFmtURL*)pDoc->GetAttrPool().GetItem2(
            RES_URL, n ) ) )
        {
            AddLinkTarget( pURL->GetURL() );
            const ImageMap *pIMap = pURL->GetMap();
            if( pIMap )
            {
                for( sal_uInt16 i=0; i<pIMap->GetIMapObjectCount(); i++ )
                {
                    const IMapObject* pObj = pIMap->GetIMapObject( i );
                    if( pObj )
                    {
                        AddLinkTarget( pObj->GetURL() );
                    }
                }
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
