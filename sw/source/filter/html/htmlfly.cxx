/*************************************************************************
 *
 *  $RCSfile: htmlfly.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2000-10-20 13:42:52 $
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

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#include "hintids.hxx"

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif

#ifndef _APP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif

#ifndef _HTMLKYWD_HXX
#include <svtools/htmlkywd.hxx>
#endif
#ifndef _HTMLOUT_HXX //autogen
#include <svtools/htmlout.hxx>
#endif
#ifndef _IMAP_HXX //autogen
#include <svtools/imap.hxx>
#endif
#ifndef _GOODIES_IMAPOBJ_HXX
#include <svtools/imapobj.hxx>
#endif

#ifndef _XOUTBMP_HXX //autogen
#include <svx/xoutbmp.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif


#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTURL_HXX //autogen
#include <fmturl.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FMTSRND_HXX //autogen
#include <fmtsrnd.hxx>
#endif
#ifndef _FMTINFMT_HXX //autogen
#include <fmtinfmt.hxx>
#endif
#ifndef _TXTINET_HXX //autogen
#include <txtinet.hxx>
#endif
#ifndef _FRMATR_HXX
#include "frmatr.hxx"
#endif
#ifndef _GRFATR_HXX //autogen
#include <grfatr.hxx>
#endif
#ifndef _FLYPOS_HXX //autogen
#include <flypos.hxx>
#endif

#ifndef _DOCARY_HXX //autogen
#include <docary.hxx>
#endif

#ifndef _NDGRF_HXX //autogen
#include <ndgrf.hxx>
#endif

#include "doc.hxx"
#include "ndtxt.hxx"
#include "pam.hxx"
#include "swerror.h"
#include "frmfmt.hxx"
#include "wrthtml.hxx"
#include "css1kywd.hxx"
#include "htmlfly.hxx"

const ULONG HTML_FRMOPTS_IMG_ALL        =
    HTML_FRMOPT_ALT |
    HTML_FRMOPT_SIZE |
    HTML_FRMOPT_ANYSIZE |
    HTML_FRMOPT_BORDER |
    HTML_FRMOPT_NAME;
const ULONG HTML_FRMOPTS_IMG_CNTNR      =
    HTML_FRMOPTS_IMG_ALL |
    HTML_FRMOPT_ABSSIZE;
const ULONG HTML_FRMOPTS_IMG            =
    HTML_FRMOPTS_IMG_ALL |
    HTML_FRMOPT_ALIGN |
    HTML_FRMOPT_SPACE |
    HTML_FRMOPT_BRCLEAR;
const ULONG HTML_FRMOPTS_IMG_CSS1       =
    HTML_FRMOPT_S_ALIGN |
    HTML_FRMOPT_S_SPACE;

const ULONG HTML_FRMOPTS_DIV            =
    HTML_FRMOPT_ID |
    HTML_FRMOPT_S_ALIGN |
    HTML_FRMOPT_S_SIZE |
    HTML_FRMOPT_ANYSIZE |
    HTML_FRMOPT_ABSSIZE |
    HTML_FRMOPT_S_SPACE |
    HTML_FRMOPT_S_BORDER |
    HTML_FRMOPT_S_BACKGROUND |
    HTML_FRMOPT_BRCLEAR;

const ULONG HTML_FRMOPTS_MULTICOL       =
    HTML_FRMOPT_ID |
    HTML_FRMOPT_WIDTH |
    HTML_FRMOPT_ANYSIZE |
    HTML_FRMOPT_ABSSIZE;
const ULONG HTML_FRMOPTS_MULTICOL_CNTNR =
    HTML_FRMOPTS_MULTICOL;
const ULONG HTML_FRMOPTS_MULTICOL_CSS1  =
    HTML_FRMOPT_S_ALIGN |
    HTML_FRMOPT_S_SIZE |
    HTML_FRMOPT_S_SPACE |
    HTML_FRMOPT_S_BORDER|
    HTML_FRMOPT_S_BACKGROUND;

const ULONG HTML_FRMOPTS_SPACER         =
    HTML_FRMOPT_ALIGN |
    HTML_FRMOPT_SIZE |
    HTML_FRMOPT_ANYSIZE |
    HTML_FRMOPT_BRCLEAR |
    HTML_FRMOPT_MARGINSIZE |
    HTML_FRMOPT_ABSSIZE;

const ULONG HTML_FRMOPTS_CNTNR          =
    HTML_FRMOPT_S_ALIGN |
    HTML_FRMOPT_S_SPACE |
    HTML_FRMOPT_S_WIDTH |
    HTML_FRMOPT_ANYSIZE |
    HTML_FRMOPT_ABSSIZE |
    HTML_FRMOPT_S_PIXSIZE;


static Writer& OutHTML_FrmFmtTableNode( Writer& rWrt, const SwFrmFmt& rFrmFmt );
static Writer& OutHTML_FrmFmtAsMulticol( Writer& rWrt, const SwFrmFmt& rFmt,
                                         BOOL bInCntnr );
static Writer& OutHTML_FrmFmtAsSpacer( Writer& rWrt, const SwFrmFmt& rFmt );
static Writer& OutHTML_FrmFmtAsDivOrSpan( Writer& rWrt,
                                          const SwFrmFmt& rFrmFmt, BOOL bSpan );
static Writer& OutHTML_FrmFmtAsImage( Writer& rWrt, const SwFrmFmt& rFmt,
                                      BOOL bInCntnr );

static Writer& OutHTML_FrmFmtGrfNode( Writer& rWrt, const SwFrmFmt& rFmt,
                                      BOOL bInCntnr );

static Writer& OutHTML_FrmFmtAsMarquee( Writer& rWrt, const SwFrmFmt& rFrmFmt,
                                        const SdrObject& rSdrObj    );
//-----------------------------------------------------------------------

extern HTMLOutEvent __FAR_DATA aAnchorEventTable[];

static HTMLOutEvent __FAR_DATA aImageEventTable[] =
{
    { sHTML_O_SDonload,         sHTML_O_onload,     SVX_EVENT_IMAGE_LOAD        },
    { sHTML_O_SDonabort,        sHTML_O_onabort,    SVX_EVENT_IMAGE_ABORT       },
    { sHTML_O_SDonerror,        sHTML_O_onerror,    SVX_EVENT_IMAGE_ERROR       },
    { 0,                        0,                  0                       }
};

static HTMLOutEvent __FAR_DATA aIMapEventTable[] =
{
    { sHTML_O_SDonmouseover,    sHTML_O_onmouseover,    SFX_EVENT_MOUSEOVER_OBJECT  },
    { sHTML_O_SDonmouseout,     sHTML_O_onmouseout,     SFX_EVENT_MOUSEOUT_OBJECT   },
    { 0,                        0,                      0                           }
};



SV_IMPL_OP_PTRARR_SORT( SwHTMLPosFlyFrms, SwHTMLPosFlyFrmPtr )

USHORT SwHTMLWriter::GuessFrmType( const SwFrmFmt& rFrmFmt,
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
        ULONG nStt = rFlyCntnt.GetCntntIdx()->GetIndex()+1;
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
            ULONG nEnd = pDoc->GetNodes()[nStt-1]->EndOfSectionIndex();

            const SfxPoolItem* pItem;
            const SfxItemSet& rItemSet = rFrmFmt.GetAttrSet();
            if( SFX_ITEM_SET == rItemSet.GetItemState( RES_COL,
                                                       TRUE, &pItem ) &&
                ((const SwFmtCol *)pItem)->GetNumCols() > 1 )
            {
                // spaltiger Rahmen
                eType = HTML_FRMTYPE_MULTICOL;
            }
            else if( pNd->IsTableNode() )
            {
                const SwTableNode *pTblNd = pNd->GetTableNode();
                ULONG nTblEnd = pTblNd->EndOfSectionIndex();

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

                BOOL bEmpty = FALSE;
                if( nStt==nEnd-1 && !pTxtNd->Len() )
                {
                    // leerer Rahmen? Nur wenn kein Rahmen am
                    // Text- oder Start-Node verankert ist.
                    bEmpty = TRUE;
                    if( pHTMLPosFlyFrms )
                    {
                        for( USHORT i=0; i<pHTMLPosFlyFrms->Count(); i++ )
                        {
                            ULONG nIdx = (*pHTMLPosFlyFrms)[i]
                                                ->GetNdIndex().GetIndex();
                            bEmpty = (nIdx != nStt) && (nIdx != nStt-1);
                            if( !bEmpty || nIdx > nStt )
                                break;
                        }
                    }
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

    return eType;
}

void SwHTMLWriter::CollectFlyFrms()
{
    BYTE nSz = (BYTE)Min( pDoc->GetSpzFrmFmts()->Count(), USHORT(255) );
    SwPosFlyFrms aFlyPos( nSz, nSz );
    pDoc->GetAllFlyFmts( aFlyPos, bWriteAll ? 0 : pCurPam, TRUE );

    for( USHORT i=0; i< aFlyPos.Count(); i++ )
    {
        const SwFrmFmt& rFrmFmt = aFlyPos[i]->GetFmt();
        const SdrObject *pSdrObj = 0;
        const SwPosition *pAPos;
        const SwCntntNode *pACNd;
        SwHTMLFrmType eType = (SwHTMLFrmType)GuessFrmType( rFrmFmt, pSdrObj );

        BYTE nMode;
        const SwFmtAnchor& rAnchor = rFrmFmt.GetAnchor();
        SwRelationOrient eHoriRel = rFrmFmt.GetHoriOrient().GetRelationOrient();
        switch( rAnchor.GetAnchorId() )
        {
        case FLY_PAGE:
        case FLY_AT_FLY:
            nMode = aHTMLOutFrmPageFlyTable[eType][nExportMode];
            break;

        case FLY_AT_CNTNT:
            // Absatz-gebundene Rahmen werden nur dann vor den
            // Absatz geschrieben, wenn der Absatz einen Abstand
            // hat.
            if( FRAME == eHoriRel &&
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

        case FLY_AUTO_CNTNT:
            if( FRAME == eHoriRel || PRTAREA == eHoriRel )
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

        SwHTMLPosFlyFrm *pNew =
            new SwHTMLPosFlyFrm( *aFlyPos[i], pSdrObj, nMode );
        pHTMLPosFlyFrms->Insert( pNew );
    }
}

BOOL SwHTMLWriter::OutFlyFrm( ULONG nNdIdx, xub_StrLen nCntntIdx, BYTE nPos )
{
    BOOL bFlysLeft = FALSE; // Noch Flys an aktueller Node-Position da?

    // OutFlyFrm kan rekursiv aufgerufen werden. Deshalb muss man
    // manchmal wieder von vorne anfangen, nachdem ein Fly ausgegeben
    // wurde.
    BOOL bRestart = TRUE;
    while( pHTMLPosFlyFrms && bRestart )
    {
        bFlysLeft = bRestart = FALSE;

        // suche nach dem Anfang der FlyFrames
        for( USHORT i=0; i<pHTMLPosFlyFrms->Count() &&
            (*pHTMLPosFlyFrms)[i]->GetNdIndex().GetIndex() < nNdIdx; i++ )
            ;
        for( ; !bRestart && i < pHTMLPosFlyFrms->Count() &&
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
                pHTMLPosFlyFrms->Remove( i, 1 );
                i--;
                if( !pHTMLPosFlyFrms->Count() )
                {
                    delete pHTMLPosFlyFrms;
                    pHTMLPosFlyFrms = 0;
                    bRestart = TRUE;    // nicht wirklich, nur raus
                                        // aus der Schleife
                }

                OutFrmFmt( pPosFly->GetOutMode(), pPosFly->GetFmt(),
                           pPosFly->GetSdrObject() );
                switch( pPosFly->GetOutFn() )
                {
                case HTML_OUT_DIV:
                case HTML_OUT_SPAN:
                case HTML_OUT_MULTICOL:
                case HTML_OUT_TBLNODE:
                    bRestart = TRUE; // Hier wird's evtl rekursiv
                    break;
                }
                delete pPosFly;
            }
            else
            {
                bFlysLeft = TRUE;
            }
        }
    }

    return bFlysLeft;
}

void SwHTMLWriter::OutFrmFmt( BYTE nMode, const SwFrmFmt& rFrmFmt,
                              const SdrObject *pSdrObject )
{
    BYTE nCntnrMode = SwHTMLPosFlyFrm::GetOutCntnr( nMode );
    BYTE nOutMode = SwHTMLPosFlyFrm::GetOutFn(nMode);
    const sal_Char *pCntnrStr = 0;
    if( HTML_CNTNR_NONE != nCntnrMode )
    {

        if( bLFPossible && HTML_CNTNR_DIV == nCntnrMode )
            OutNewLine();

        ByteString sOut( '<' );
        pCntnrStr = (HTML_CNTNR_DIV == nCntnrMode)
                            ? sHTML_division
                            : sHTML_span;
        sOut += pCntnrStr;
        ((((sOut += ' ') += sHTML_O_class) += "=\"")
            += sCSS1_class_abs_pos) += '\"';
        Strm() << sOut.GetBuffer();

        // Fuer Nicht-Zeichenobekte eine Breite ausgeben
        ULONG nFrmFlags = HTML_FRMOPTS_CNTNR;

        // Fuer spaltige Rahmen koennen wir auch noch den Hintergrund ausgeben.
        if( HTML_OUT_MULTICOL == nOutMode )
            nFrmFlags |= HTML_FRMOPT_S_BACKGROUND|HTML_FRMOPT_S_BORDER;

        if( IsHTMLMode( HTMLMODE_BORDER_NONE ) )
           nFrmFlags |= HTML_FRMOPT_S_NOBORDER;
        OutCSS1_FrmFmtOptions( rFrmFmt, nFrmFlags, pSdrObject );
        Strm() << '>';

        if( HTML_CNTNR_DIV == nCntnrMode )
        {
            IncIndentLevel();
            bLFPossible = TRUE;
        }
    }

    switch( nOutMode )
    {
    case HTML_OUT_TBLNODE:      // OK
        ASSERT( !pCntnrStr, "Table: Container ist hier nicht vorgesehen" );
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
        ASSERT( !pCntnrStr, "Div: Container ist hier nicht vorgesehen" );
        OutHTML_FrmFmtAsDivOrSpan( *this, rFrmFmt, HTML_OUT_SPAN==nOutMode );
        break;
    case HTML_OUT_MULTICOL:     // OK
        OutHTML_FrmFmtAsMulticol( *this, rFrmFmt, pCntnrStr != 0 );
        break;
    case HTML_OUT_SPACER:       // OK
        ASSERT( !pCntnrStr, "Spacer: Container ist hier nicht vorgesehen" );
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
        ASSERT( !pCntnrStr, "Marquee: Container ist hier nicht vorgesehen" );
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
        HTMLOutFuncs::Out_AsciiTag( Strm(), sHTML_division, FALSE );
        bLFPossible = TRUE;
    }
    else if( HTML_CNTNR_SPAN == nCntnrMode )
        HTMLOutFuncs::Out_AsciiTag( Strm(), sHTML_span, FALSE );
}


void SwHTMLWriter::OutFrmFmtOptions( const SwFrmFmt &rFrmFmt,
                                     const String& rAlternateTxt,
                                     ByteString &rEndTags,
                                     ULONG nFrmOpts )
{
    ByteString sOut;
    const SfxPoolItem* pItem;
    const SfxItemSet& rItemSet = rFrmFmt.GetAttrSet();

    // Name
    if( (nFrmOpts & (HTML_FRMOPT_ID|HTML_FRMOPT_NAME)) &&
        rFrmFmt.GetName().Len() )
    {
        const sal_Char *pStr =
            (nFrmOpts & HTML_FRMOPT_ID) ? sHTML_O_id : sHTML_O_name;
        ((sOut += ' ') += pStr) += "=\"";
        Strm() << sOut.GetBuffer();
        HTMLOutFuncs::Out_String( Strm(), rFrmFmt.GetName(), eDestEnc );
        sOut = '\"';
    }

    // ALT
    if( (nFrmOpts & HTML_FRMOPT_ALT) && rAlternateTxt.Len() )
    {
        ((sOut += ' ') += sHTML_O_alt) += "=\"";
        Strm() << sOut.GetBuffer();
        HTMLOutFuncs::Out_String( Strm(), rAlternateTxt, eDestEnc );
        sOut = '\"';
    }

    // ALIGN
    const sal_Char *pStr = 0;
    RndStdIds eAnchorId = rFrmFmt.GetAnchor().GetAnchorId();
    if( (nFrmOpts & HTML_FRMOPT_ALIGN) &&
        (FLY_AT_CNTNT == eAnchorId || FLY_AUTO_CNTNT == eAnchorId) )
    {
        // MIB 12.3.98: Ist es nicht schlauer, absatzgebundene
        // Rahmen notfalls links auszurichten als sie
        // zeichengebunden einzufuegen???
        const SwFmtHoriOrient& rHoriOri = rFrmFmt.GetHoriOrient();
        if( !(nFrmOpts & HTML_FRMOPT_S_ALIGN) ||
            FRAME == rHoriOri.GetRelationOrient() ||
            PRTAREA == rHoriOri.GetRelationOrient() )
        {
            pStr = HORI_RIGHT == rHoriOri.GetHoriOrient()
                        ? sHTML_AL_right
                        : sHTML_AL_left;
        }
    }
    if( (nFrmOpts & HTML_FRMOPT_ALIGN) && !pStr &&
        ( (nFrmOpts & HTML_FRMOPT_S_ALIGN) == 0 ||
          FLY_IN_CNTNT == eAnchorId ) &&
        SFX_ITEM_SET == rItemSet.GetItemState( RES_VERT_ORIENT, TRUE, &pItem ))
    {
        switch( ((SwFmtVertOrient*)pItem)->GetVertOrient() )
        {
        case VERT_LINE_TOP:     pStr = sHTML_VA_top;        break;
        case VERT_CHAR_TOP:
        case VERT_BOTTOM:       pStr = sHTML_VA_texttop;    break;  // geht nicht
        case VERT_LINE_CENTER:
        case VERT_CHAR_CENTER:  pStr = sHTML_VA_absmiddle;  break;  // geht nicht
        case VERT_CENTER:       pStr = sHTML_VA_middle;     break;
        case VERT_LINE_BOTTOM:
        case VERT_CHAR_BOTTOM:  pStr = sHTML_VA_absbottom;  break;  // geht nicht
        case VERT_TOP:          pStr = sHTML_VA_bottom;     break;
        }
    }
    if( pStr )
        (((sOut += ' ') += sHTML_O_align) += '=') += pStr;


    // HSPACE und VSPACE
    Size aTwipSpc( 0, 0 );
    if( (nFrmOpts & (HTML_FRMOPT_SPACE|HTML_FRMOPT_MARGINSIZE)) &&
        SFX_ITEM_SET == rItemSet.GetItemState( RES_LR_SPACE, TRUE, &pItem ))
    {
        aTwipSpc.Width() =
            ( ((SvxLRSpaceItem*)pItem)->GetLeft() +
                ((SvxLRSpaceItem*)pItem)->GetRight() ) / 2;
        nDfltLeftMargin = nDfltRightMargin = (USHORT)aTwipSpc.Width();
    }
    if( (nFrmOpts & (HTML_FRMOPT_SPACE|HTML_FRMOPT_MARGINSIZE)) &&
        SFX_ITEM_SET == rItemSet.GetItemState( RES_UL_SPACE, TRUE, &pItem ))
    {
        aTwipSpc.Height()  =
            ( ((SvxULSpaceItem*)pItem)->GetUpper() +
                ((SvxULSpaceItem*)pItem)->GetLower() ) / 2;
        nDfltTopMargin = nDfltBottomMargin = (USHORT)aTwipSpc.Height();
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
            (((sOut += ' ') += sHTML_O_hspace) += '=')
                += ByteString::CreateFromInt32( aPixelSpc.Width() );
        }

        if( aPixelSpc.Height() )
        {
            (((sOut += ' ') += sHTML_O_vspace) += '=')
                += ByteString::CreateFromInt32( aPixelSpc.Height() );
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
        SFX_ITEM_SET == rItemSet.GetItemState( RES_BOX, TRUE, &pItem ))
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
        SFX_ITEM_SET == rItemSet.GetItemState( RES_FRM_SIZE, TRUE, &pItem ) &&
        ( (nFrmOpts & HTML_FRMOPT_ANYSIZE) ||
          ATT_FIX_SIZE == ((const SwFmtFrmSize *)pItem)->GetSizeType()) )
    {
        const SwFmtFrmSize *pFSItem = (const SwFmtFrmSize *)pItem;
        BYTE nPrcWidth = pFSItem->GetWidthPercent();
        BYTE nPrcHeight = pFSItem->GetHeightPercent();

        // Groesse des Objekts Twips ohne Raender
        Size aTwipSz( (nPrcWidth ? 0
                                 : pFSItem->GetWidth()-aTwipSpc.Width()),
                      (nPrcHeight ? 0
                                  : pFSItem->GetHeight()-aTwipSpc.Height()) );

        ASSERT( aTwipSz.Width() >= 0 && aTwipSz.Height() >= 0,
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
            ((sOut += ' ') += sHTML_O_width) += '=';
            if( nPrcWidth )
                (sOut += ByteString::CreateFromInt32( nPrcWidth )) += '%';
            else
                sOut += ByteString::CreateFromInt32( aPixelSz.Width() );
        }

        if( (nFrmOpts & HTML_FRMOPT_HEIGHT) &&
            ((nPrcHeight && nPrcHeight!=255) || aPixelSz.Height()) )
        {
            ((sOut += ' ') += sHTML_O_height) += '=';
            if( nPrcHeight )
                (sOut += ByteString::CreateFromInt32( nPrcHeight )) += '%';
            else
                sOut += ByteString::CreateFromInt32( aPixelSz.Height() );
        }
    }

    if( sOut.Len() )
        Strm() << sOut.GetBuffer();

    // Umlauf fuer absatzgeb. Grafiken als <BR CLEAR=...> in den String
    // schreiben
    if( (nFrmOpts & HTML_FRMOPT_BRCLEAR) &&
        (FLY_AT_CNTNT== rFrmFmt.GetAnchor().GetAnchorId() ||
         FLY_AUTO_CNTNT== rFrmFmt.GetAnchor().GetAnchorId()) &&
        SFX_ITEM_SET == rItemSet.GetItemState( RES_SURROUND, TRUE, &pItem ))
    {
        const SwFmtSurround* pSurround = (const SwFmtSurround*)pItem;
        SwHoriOrient eHoriOri = rFrmFmt.GetHoriOrient().GetHoriOrient();
        pStr = 0;
        SwSurround eSurround = pSurround->GetSurround();
        BOOL bAnchorOnly = pSurround->IsAnchorOnly();
        switch( eHoriOri )
        {
        case HORI_RIGHT:
            {
                switch( eSurround )
                {
                case SURROUND_NONE:
                case SURROUND_RIGHT:
                    pStr = sHTML_AL_right;
                    break;
                case SURROUND_LEFT:
                case SURROUND_PARALLEL:
                    if( bAnchorOnly )
                        bClearRight = TRUE;
                    break;
                }
            }
            break;

        default:
            // #67508#: If a frame is centered, it gets left aligned. This
            // should be taken into account here, too.
            {
                switch( eSurround )
                {
                case SURROUND_NONE:
                case SURROUND_LEFT:
                    pStr = sHTML_AL_left;
                    break;
                case SURROUND_RIGHT:
                case SURROUND_PARALLEL:
                    if( bAnchorOnly )
                        bClearLeft = TRUE;
                    break;
                }
            }
            break;

        }

        if( pStr )
        {
            (((((((sOut = '<') += sHTML_linebreak) += ' ')
                += sHTML_O_clear) += '=') += pStr) += '>') += rEndTags;
            rEndTags = sOut;
        }
    }
}


Writer& OutHTML_Image( Writer& rWrt, const SwFrmFmt &rFrmFmt,
                       const String &rGrfName, const String& rAlternateTxt,
                       const Size &rRealSize, ULONG nFrmOpts,
                       const sal_Char *pMarkType,
                       const ImageMap *pAltImgMap )
{
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;

    // ggf. ein noch offenes Attribut voruebergehend beenden
    if( rHTMLWrt.aINetFmts.Count() )
    {
        SwFmtINetFmt *pINetFmt =
            rHTMLWrt.aINetFmts[ rHTMLWrt.aINetFmts.Count()-1 ];
        OutHTML_INetFmt( rWrt, *pINetFmt, FALSE );
    }

    String aGrfNm( rGrfName );
    if( !HTMLOutFuncs::PrivateURLToInternalImg(aGrfNm) )
        aGrfNm = INetURLObject::AbsToRel( aGrfNm, INetURLObject::WAS_ENCODED,
                                        INetURLObject::DECODE_UNAMBIGUOUS);

    const SfxPoolItem* pItem;
    const SfxItemSet& rItemSet = rFrmFmt.GetAttrSet();

    const SwFmtURL *pURLItem = 0;

    // das URL-Attribut nur beruecksichtigen, wenn keine Image-Map
    // uebergeben wurde
    if( !pAltImgMap &&
        SFX_ITEM_SET == rItemSet.GetItemState( RES_URL, TRUE, &pItem ))
    {
        pURLItem = (const SwFmtURL *)pItem;
    }

    // Image-Map rausschreiben
    const ImageMap *pIMap = pAltImgMap;
    if( !pIMap && pURLItem )
    {
        pIMap = pURLItem->GetMap();
    }

    String aIMapName;
    if( pIMap )
    {
        // den Namen eindeutig machen
        aIMapName = pIMap->GetName();
        String aNameBase;
        if( aIMapName.Len() )
            aNameBase = aIMapName;
        else
            aNameBase.AssignAscii( sHTML_map );
        if( !aIMapName.Len() )
            (aIMapName = aNameBase)
                += String::CreateFromInt32( rHTMLWrt.nImgMapCnt );

        BOOL bFound;
        do
        {
            bFound = FALSE;
            for( USHORT i=0; i<rHTMLWrt.aImgMapNames.Count(); i++ )
            {
                // TODO: Unicode: Comparison is case insensitive for ASCII
                // characters only now!
                if( aIMapName.EqualsIgnoreCaseAscii(*rHTMLWrt.aImgMapNames[i]) )
                {
                    bFound = TRUE;
                    break;
                }
            }
            if( bFound )
            {
                rHTMLWrt.nImgMapCnt++;
                (aIMapName = aNameBase)
                    += String::CreateFromInt32( rHTMLWrt.nImgMapCnt );
            }

        } while( bFound );

        BOOL bScale = FALSE;
        //Size aGrfSize( rNode.GetTwipSize() );
        Fraction aScaleX( 1, 1 );
        Fraction aScaleY( 1, 1 );

        const SwFmtFrmSize& rFrmSize = rFrmFmt.GetFrmSize();
        const SvxBoxItem& rBox = rFrmFmt.GetBox();

        if( !rFrmSize.GetWidthPercent() && rRealSize.Width() )
        {
            SwTwips nWidth = rFrmSize.GetWidth();
            nWidth -= ( rBox.CalcLineSpace(BOX_LINE_LEFT) +
                        rBox.CalcLineSpace(BOX_LINE_RIGHT) );

            ASSERT( nWidth>0, "Gibt es 0 twip breite Grafiken!?" );
            if( nWidth<=0 ) // sollte nicht passieren
                nWidth = 1;

            if( rRealSize.Width() != nWidth )
            {
                aScaleX = Fraction( nWidth, rRealSize.Width() );
                bScale = TRUE;
            }
        }
        if( !rFrmSize.GetHeightPercent() && rRealSize.Height() )
        {
            SwTwips nHeight = rFrmSize.GetHeight();
            nHeight -= ( rBox.CalcLineSpace(BOX_LINE_TOP) +
                         rBox.CalcLineSpace(BOX_LINE_BOTTOM) );

            ASSERT( nHeight>0, "Gibt es 0 twip hohe Grafiken!?" );
            if( nHeight<=0 )
                nHeight = 1;

            if( rRealSize.Height() != nHeight )
            {
                aScaleY = Fraction( nHeight, rRealSize.Height() );
                bScale = TRUE;
            }
        }

        rHTMLWrt.aImgMapNames.Insert( new String(aIMapName),
                                          rHTMLWrt.aImgMapNames.Count() );

        ByteString aIndMap, aIndArea;
        const sal_Char *pLF = 0, *pIndArea = 0, *pIndMap = 0;
#if defined(MAC) || defined(UNX)
        sal_Char aLF[2] = "\x00";
#endif

        if( rHTMLWrt.bLFPossible )
        {
            rHTMLWrt.OutNewLine( TRUE );
            rHTMLWrt.GetIndentString( aIndMap );
            rHTMLWrt.GetIndentString( aIndArea, 1 );
#if defined(MAC) || defined(UNX)
            aLF[0]  = SwHTMLWriter::sNewLine;
            pLF = aLF;
#else
            pLF = SwHTMLWriter::sNewLine;
#endif
            pIndArea = aIndArea.GetBuffer();
            pIndMap = aIndMap.GetBuffer();
        }

        if( bScale )
        {
            ImageMap aScaledIMap( *pIMap );
            aScaledIMap.Scale( aScaleX, aScaleY );
            HTMLOutFuncs::Out_ImageMap( rWrt.Strm(), aScaledIMap, aIMapName,
                                        aIMapEventTable,
                                        rHTMLWrt.bCfgStarBasic,
                                        pLF, pIndArea, pIndMap,
                                        rHTMLWrt.eDestEnc );
        }
        else
        {
            HTMLOutFuncs::Out_ImageMap( rWrt.Strm(), *pIMap, aIMapName,
                                        aIMapEventTable,
                                        rHTMLWrt.bCfgStarBasic,
                                        pLF, pIndArea, pIndMap,
                                         rHTMLWrt.eDestEnc );
        }
    }

    // wenn meoglich vor der Grafik einen Zeilen-Umbruch ausgeben
    if( rHTMLWrt.bLFPossible )
        rHTMLWrt.OutNewLine( TRUE );

    // Attribute die ausserhelb der Grafik geschreiben werden muessen sammeln
    ByteString sOut;
    ByteString aEndTags;

    // implizite Sprungmarke -> <A NAME=...></A>...<IMG ...>
    if( pMarkType && rFrmFmt.GetName().Len() )
        rHTMLWrt.OutImplicitMark( rFrmFmt.GetName(), pMarkType );

    // URL -> <A>...<IMG ... >...</A>
    const SvxMacroItem *pMacItem = 0;
    if( SFX_ITEM_SET == rItemSet.GetItemState( RES_FRMMACRO, TRUE, &pItem ))
        pMacItem = (const SvxMacroItem *)pItem;

    if( pURLItem || pMacItem )
    {
        String aMapURL;
        String aName;
        String aTarget;
        if( pURLItem )
        {
            aMapURL = pURLItem->GetURL();
            aName = pURLItem->GetName();
            aTarget = pURLItem->GetTargetFrameName();
        }
        BOOL bEvents = pMacItem && pMacItem->GetMacroTable().Count();

        if( aMapURL.Len() || aName.Len() || aTarget.Len() || bEvents )
        {
            (sOut = '<') += sHTML_anchor;

            // Ein HREF nur Ausgaben, wenn es einen Link oder Makros gibt
            if( aMapURL.Len() || bEvents )
            {
                ((sOut += ' ') += sHTML_O_href) += "=\"";
                rWrt.Strm() << sOut.GetBuffer();
                HTMLOutFuncs::Out_String( rWrt.Strm(),
                            INetURLObject::AbsToRel( aMapURL,
                                        INetURLObject::WAS_ENCODED,
                                        INetURLObject::DECODE_UNAMBIGUOUS),
                            rHTMLWrt.eDestEnc );
                sOut = '\"';
            }

            if( aName.Len() )
            {
                ((sOut += ' ') += sHTML_O_name) += "=\"";
                rWrt.Strm() << sOut.GetBuffer();
                HTMLOutFuncs::Out_String( rWrt.Strm(), aName,
                                          rHTMLWrt.eDestEnc );
                sOut = '\"';
            }

            if( aTarget.Len() )
            {
                ((sOut += ' ') += sHTML_O_target) += "=\"";
                rWrt.Strm() << sOut.GetBuffer();
                HTMLOutFuncs::Out_String( rWrt.Strm(), aTarget,
                                          rHTMLWrt.eDestEnc );
                sOut = '\"';
            }
            if( sOut.Len() )
            {
                rWrt.Strm() << sOut.GetBuffer();
                sOut.Erase();
            }

            if( pMacItem )
            {
                const SvxMacroTableDtor& rMacTable = pMacItem->GetMacroTable();
                if( rMacTable.Count() )
                    HTMLOutFuncs::Out_Events( rWrt.Strm(), rMacTable,
                                              aAnchorEventTable,
                                              rHTMLWrt.bCfgStarBasic,
                                                 rHTMLWrt.eDestEnc );
            }

            rWrt.Strm() << ">";
            (((sOut = "</") += sHTML_anchor) += ">") += aEndTags;
            aEndTags = sOut;
        }
    }

    // Umrandung -> <FONT COLOR = ...>...<IMG ... >...</FONT>
    USHORT nBorderWidth = 0;
    if( (nFrmOpts & HTML_FRMOPT_BORDER) &&
        SFX_ITEM_SET == rItemSet.GetItemState( RES_BOX, TRUE, &pItem ))
    {
        Size aTwipBorder( 0, 0 );
        const SvxBoxItem* pBoxItem = (const SvxBoxItem*)pItem;

        const SvxBorderLine *pColBorderLine = 0;
        const SvxBorderLine *pBorderLine = pBoxItem->GetLeft();
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
                (USHORT)(aPixelBorder.Width() + aPixelBorder.Height());
        }

        if( pColBorderLine )
        {
            sOut = '<';
            (((sOut += sHTML_font) += ' ') += sHTML_O_color) += '=';
            rWrt.Strm() << sOut.GetBuffer();
            HTMLOutFuncs::Out_Color( rWrt.Strm(),
                                     pColBorderLine->GetColor() ) << '>';

            (((sOut = "</" ) += sHTML_font) += '>') += aEndTags;
            aEndTags = sOut;
        }
    }

    sOut = '<';
    (((sOut += sHTML_image) += ' ') += sHTML_O_src) += "=\"";
    rWrt.Strm() << sOut.GetBuffer();
    HTMLOutFuncs::Out_String( rWrt.Strm(), aGrfNm, rHTMLWrt.eDestEnc ) << '\"';

    // Events
    sOut.Erase();
    if( SFX_ITEM_SET == rItemSet.GetItemState( RES_FRMMACRO, TRUE, &pItem ))
    {
        const SvxMacroTableDtor& rMacTable =
            ((const SvxMacroItem *)pItem)->GetMacroTable();
        if( rMacTable.Count() )
            HTMLOutFuncs::Out_Events( rWrt.Strm(), rMacTable, aImageEventTable,
                                      rHTMLWrt.bCfgStarBasic, rHTMLWrt.eDestEnc );
    }

    // ALT, ALIGN, WIDTH, HEIGHT, HSPACE, VSPACE
    rHTMLWrt.OutFrmFmtOptions( rFrmFmt, rAlternateTxt, aEndTags, nFrmOpts );
    if( rHTMLWrt.IsHTMLMode( HTMLMODE_ABS_POS_FLY ) )
        rHTMLWrt.OutCSS1_FrmFmtOptions( rFrmFmt, nFrmOpts );


    if( nFrmOpts & HTML_FRMOPT_BORDER )
    {
        (((sOut = ' ') += sHTML_O_border) += '=')
            += ByteString::CreateFromInt32( nBorderWidth );
        rWrt.Strm() << sOut.GetBuffer();
    }

    if( pURLItem && pURLItem->IsServerMap() )
    {
        (sOut = ' ') += sHTML_O_ismap;
        rWrt.Strm() << sOut.GetBuffer();
    }
    if( aIMapName.Len() )
    {
        ((sOut = ' ') += sHTML_O_usemap) += "=\"#";
        rWrt.Strm() << sOut.GetBuffer();
        HTMLOutFuncs::Out_String( rWrt.Strm(), aIMapName, rHTMLWrt.eDestEnc ) << '\"';
    }

    rHTMLWrt.Strm() << '>';

    if( aEndTags.Len() )
        rWrt.Strm() << aEndTags.GetBuffer();

    if( rHTMLWrt.aINetFmts.Count() )
    {
        // es ist noch ein Attribut auf dem Stack, das wieder geoeffnet
        // werden muss
        SwFmtINetFmt *pINetFmt =
            rHTMLWrt.aINetFmts[ rHTMLWrt.aINetFmts.Count()-1 ];
        OutHTML_INetFmt( rWrt, *pINetFmt, TRUE );
    }

    return rHTMLWrt;
}

Writer& OutHTML_BulletImage( Writer& rWrt,
                             const sal_Char *pTag,
                             const SvxBrushItem* pBrush,
                             String &rGrfName,
                             const Size &rSize,
                             const SwFmtVertOrient* pVertOrient )
{
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;

    // Wenn es ein BrushItem gibt, muss die Grafiknoch exportiert werden
    const String *pLink = 0;
    if( pBrush )
    {
        pLink = pBrush->GetGraphicLink();

        // embeddete Grafik -> WriteEmbedded schreiben
        if( !pLink )
        {
            const Graphic* pGrf = pBrush->GetGraphic();
            if( pGrf )
            {
                // Grafik als (JPG-)File speichern
                if( rHTMLWrt.GetOrigFileName() )
                    rGrfName = *rHTMLWrt.GetOrigFileName();
                USHORT nErr = XOutBitmap::WriteGraphic( *pGrf,  rGrfName,
                        String::CreateFromAscii("JPG"),
                        XOUTBMP_USE_GIF_IF_SENSIBLE);
                if( !nErr )
                {
                    rGrfName = URIHelper::SmartRelToAbs( rGrfName );
                    if( rHTMLWrt.HasCId() )
                        rHTMLWrt.MakeCIdURL( rGrfName );
                    pLink = &rGrfName;
                }
                else
                {
                    rHTMLWrt.nWarn = WARN_SWG_POOR_LOAD | WARN_SW_WRITE_BASE;
                }
            }
        }
        else
        {
            rGrfName = *pLink;
            if( rHTMLWrt.bCfgCpyLinkedGrfs || rHTMLWrt.HasCId() )
            {
                rHTMLWrt.CopyLocalFileToINet( rGrfName, rHTMLWrt.HasCId() );
                if( rHTMLWrt.HasCId() )
                    rHTMLWrt.MakeCIdURL( rGrfName );
                pLink = &rGrfName;
            }
        }
    }
    else
    {
        pLink = &rGrfName;
    }

    ByteString sOut;
    if( pTag )
        (sOut += '<') += pTag;

    if( pLink )
    {
        sOut += ' ';
        String s( *pLink );
        if( !HTMLOutFuncs::PrivateURLToInternalImg(s) )
            s = INetURLObject::AbsToRel( s, INetURLObject::WAS_ENCODED,
                                        INetURLObject::DECODE_UNAMBIGUOUS);
        (sOut += sHTML_O_src) += "=\"";
        rWrt.Strm() << sOut.GetBuffer();
        HTMLOutFuncs::Out_String( rWrt.Strm(), s, rHTMLWrt.eDestEnc );
        sOut = '\"';

        // Groesse des Objekts Twips ohne Raender
        Size aPixelSz( 0, 0 );
        if( (rSize.Width() || rSize.Height()) && Application::GetDefaultDevice() )
        {
            aPixelSz =
                Application::GetDefaultDevice()->LogicToPixel( rSize,
                                                    MapMode(MAP_TWIP) );
            if( !aPixelSz.Width() && rSize.Width() )
                aPixelSz.Width() = 1;
            if( !aPixelSz.Height() && rSize.Height() )
                aPixelSz.Height() = 1;
        }

        if( aPixelSz.Width() )
            (((sOut += ' ') += sHTML_O_width) += '=')
                += ByteString::CreateFromInt32( aPixelSz.Width() );

        if( aPixelSz.Height() )
            (((sOut += ' ') += sHTML_O_height) += '=')
                += ByteString::CreateFromInt32( aPixelSz.Height() );

        if( pVertOrient )
        {
            const sal_Char *pStr = 0;
            switch( pVertOrient->GetVertOrient() )
            {
            case VERT_LINE_TOP:     pStr = sHTML_VA_top;        break;
            case VERT_CHAR_TOP:
            case VERT_BOTTOM:       pStr = sHTML_VA_texttop;    break;  // geht nicht
            case VERT_LINE_CENTER:
            case VERT_CHAR_CENTER:  pStr = sHTML_VA_absmiddle;  break;  // geht nicht
            case VERT_CENTER:       pStr = sHTML_VA_middle;     break;
            case VERT_LINE_BOTTOM:
            case VERT_CHAR_BOTTOM:  pStr = sHTML_VA_absbottom;  break;  // geht nicht
            case VERT_TOP:          pStr = sHTML_VA_bottom;     break;
            }
            if( pStr )
                (((sOut += ' ') += sHTML_O_align) += '=') += pStr;
        }
    }

    if( pTag )
        sOut += '>';
    rWrt.Strm() << sOut.GetBuffer();

    return rWrt;
}


//-----------------------------------------------------------------------

static Writer& OutHTML_FrmFmtTableNode( Writer& rWrt, const SwFrmFmt& rFrmFmt )
{
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;

    const SwFmtCntnt& rFlyCntnt = rFrmFmt.GetCntnt();
    ULONG nStt = rFlyCntnt.GetCntntIdx()->GetIndex()+1;
    ULONG nEnd = rHTMLWrt.pDoc->GetNodes()[nStt-1]->EndOfSectionIndex();

    String aCaption;
    BOOL bTopCaption = FALSE;

    // Nicht const, weil GetTable spater mal nicht const ist
    SwNode *pNd = rHTMLWrt.pDoc->GetNodes()[ nStt ];
    SwTableNode *pTblNd = pNd->GetTableNode();
    const SwTxtNode *pTxtNd = pNd->GetTxtNode();
    if( !pTblNd && pTxtNd )
    {
        // Tabelle mit Ueberschrift
        bTopCaption = TRUE;
        pTblNd = rHTMLWrt.pDoc->GetNodes()[nStt+1]->GetTableNode();
    }
    ASSERT( pTblNd, "Rahmen enthaelt keine Tabelle" );
    if( pTblNd )
    {
        ULONG nTblEnd = pTblNd->EndOfSectionIndex();
        ASSERT( nTblEnd == nEnd - 1 ||
                (nTblEnd == nEnd - 2 && !bTopCaption),
                "Ungeuelter Rahmen-Inhalt fuer Tabelle" );

        if( nTblEnd == nEnd - 2 )
            pTxtNd = rHTMLWrt.pDoc->GetNodes()[nTblEnd+1]->GetTxtNode();
    }
    if( pTxtNd )
        aCaption = pTxtNd->GetTxt();

    {
        HTMLSaveData aSaveData( rHTMLWrt, pTblNd->GetIndex()+1,
                                pTblNd->EndOfSectionIndex(),
                                (SwFlyFrmFmt*)&rFrmFmt );
        OutHTML_SwTblNode( rHTMLWrt, *pTblNd, &rFrmFmt, &aCaption,
                           bTopCaption );
    }

    return rWrt;
}

static Writer & OutHTML_FrmFmtAsMulticol( Writer& rWrt,
                                          const SwFrmFmt& rFrmFmt,
                                          BOOL bInCntnr )
{
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;

    rHTMLWrt.ChangeParaToken( 0 );

    // Die aktulle <DL> beenden!
    rHTMLWrt.OutAndSetDefList( 0 );

    // als Multicol ausgeben
    if( rHTMLWrt.bLFPossible )
        rHTMLWrt.OutNewLine();

    ByteString sOut( '<' );
    sOut += sHTML_multicol;

    const SwFmtCol& rFmtCol = rFrmFmt.GetCol();

    // die Anzahl der Spalten als COLS ausgeben
    USHORT nCols = rFmtCol.GetNumCols();
    if( nCols )
        (((sOut += ' ') += sHTML_O_cols) += '=')
            += ByteString::CreateFromInt32( nCols );

    // die Gutter-Breite (Minimalwert) als GUTTER
    USHORT nGutter = rFmtCol.GetGutterWidth( TRUE );
    if( nGutter!=USHRT_MAX )
    {
        if( nGutter && Application::GetDefaultDevice() )
        {
            nGutter = (USHORT)Application::GetDefaultDevice()
                            ->LogicToPixel( Size(nGutter,0),
                                            MapMode(MAP_TWIP) ).Width();
        }
        (((sOut += ' ') += sHTML_O_gutter) += '=')
            += ByteString::CreateFromInt32( nGutter );
    }

    rWrt.Strm() << sOut.GetBuffer();

    // WIDTH
    ULONG nFrmFlags = bInCntnr ? HTML_FRMOPTS_MULTICOL_CNTNR
                                : HTML_FRMOPTS_MULTICOL;
    if( rHTMLWrt.IsHTMLMode( HTMLMODE_ABS_POS_FLY ) && !bInCntnr )
        nFrmFlags |= HTML_FRMOPTS_MULTICOL_CSS1;
    ByteString aEndTags;
    rHTMLWrt.OutFrmFmtOptions( rFrmFmt, aEmptyStr, aEndTags, nFrmFlags );
    if( rHTMLWrt.IsHTMLMode( HTMLMODE_ABS_POS_FLY ) && !bInCntnr )
        rHTMLWrt.OutCSS1_FrmFmtOptions( rFrmFmt, nFrmFlags );

    rWrt.Strm() << '>';

    rHTMLWrt.bLFPossible = TRUE;
    rHTMLWrt.IncIndentLevel();  // den Inhalt von Multicol einruecken;

    const SwFmtCntnt& rFlyCntnt = rFrmFmt.GetCntnt();
    ULONG nStt = rFlyCntnt.GetCntntIdx()->GetIndex();
    const SwStartNode* pSttNd = rWrt.pDoc->GetNodes()[nStt]->GetStartNode();
    ASSERT( pSttNd, "Wo ist der Start-Node" );

    {
        // in einem Block damit rechtzeitig vor dem Ende der alte Zustand
        // wieder hergestellt wird.
        HTMLSaveData aSaveData( rHTMLWrt, nStt+1,
                                pSttNd->EndOfSectionIndex(),
                                (const SwFlyFrmFmt*)&rFrmFmt );
        rHTMLWrt.Out_SwDoc( rWrt.pCurPam );
    }

//  rHTMLWrt.ChangeParaToken( 0 );  // MIB 8.7.97: Passiert jetzt in Out_SwDoc

    rHTMLWrt.DecIndentLevel();  // den Inhalt von Multicol einruecken;
    if( rHTMLWrt.bLFPossible )
        rHTMLWrt.OutNewLine();
    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sHTML_multicol, FALSE );
    rHTMLWrt.bLFPossible = TRUE;

    return rWrt;
}

static Writer& OutHTML_FrmFmtAsSpacer( Writer& rWrt, const SwFrmFmt& rFrmFmt )
{
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;

    // wenn meoglich vor der Grafik einen Zeilen-Umbruch ausgeben
    if( rHTMLWrt.bLFPossible )
        rHTMLWrt.OutNewLine( TRUE );

    ByteString sOut('<');
    ((((sOut += sHTML_spacer) += ' ') += sHTML_O_type) += '=')
        += sHTML_SPTYPE_block;
    rWrt.Strm() << sOut.GetBuffer();

    // ALIGN, WIDTH, HEIGHT
    ByteString aEndTags;
    rHTMLWrt.OutFrmFmtOptions( rFrmFmt, aEmptyStr, aEndTags,
                                HTML_FRMOPTS_SPACER );

    rWrt.Strm() << '>';
    if( aEndTags.Len() )
        rWrt.Strm() << aEndTags.GetBuffer();

    return rWrt;
}

static Writer& OutHTML_FrmFmtAsDivOrSpan( Writer& rWrt,
                                          const SwFrmFmt& rFrmFmt, BOOL bSpan)
{
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;

    const sal_Char *pStr = 0;
    if( !bSpan )
    {
        rHTMLWrt.ChangeParaToken( 0 );

        // Die aktulle <DL> beenden!
        rHTMLWrt.OutAndSetDefList( 0 );
        pStr = sHTML_division;
    }
    else
        pStr = sHTML_span;

    // als DIV ausgeben
    if( rHTMLWrt.bLFPossible )
        rHTMLWrt.OutNewLine();

    ByteString sOut( '<' );
    sOut += pStr;

    rWrt.Strm() << sOut.GetBuffer();
    ByteString aEndTags;
    ULONG nFrmFlags = HTML_FRMOPTS_DIV;
    if( rHTMLWrt.IsHTMLMode( HTMLMODE_BORDER_NONE ) )
       nFrmFlags |= HTML_FRMOPT_S_NOBORDER;
    rHTMLWrt.OutFrmFmtOptions( rFrmFmt, aEmptyStr, aEndTags, nFrmFlags );
    rHTMLWrt.OutCSS1_FrmFmtOptions( rFrmFmt, nFrmFlags );
    rWrt.Strm() << '>';

    rHTMLWrt.IncIndentLevel();  // den Inhalt einruecken
    rHTMLWrt.bLFPossible = TRUE;

    const SwFmtCntnt& rFlyCntnt = rFrmFmt.GetCntnt();
    ULONG nStt = rFlyCntnt.GetCntntIdx()->GetIndex();

    // Am Start-Node verankerte Rahmen-gebundene Rahmen ausgeben
    rHTMLWrt.OutFlyFrm( nStt, 0, HTML_POS_ANY );

    const SwStartNode* pSttNd = rWrt.pDoc->GetNodes()[nStt]->GetStartNode();
    ASSERT( pSttNd, "Wo ist der Start-Node" );

    {
        // in einem Block damit rechtzeitig vor dem Ende der alte Zustand
        // wieder hergestellt wird.
        HTMLSaveData aSaveData( rHTMLWrt, nStt+1,
                                pSttNd->EndOfSectionIndex(),
                                (const SwFlyFrmFmt*)&rFrmFmt );
        rHTMLWrt.Out_SwDoc( rWrt.pCurPam );
    }

    rHTMLWrt.DecIndentLevel();  // den Inhalt von Multicol einruecken;
    if( rHTMLWrt.bLFPossible )
        rHTMLWrt.OutNewLine();
    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), pStr, FALSE );

    if( aEndTags.Len() )
        rWrt.Strm() << aEndTags.GetBuffer();

    return rWrt;
}

static Writer & OutHTML_FrmFmtAsImage( Writer& rWrt, const SwFrmFmt& rFrmFmt,
                                       BOOL bInCntnr )
{
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;

    ImageMap aIMap;
    Graphic aGrf( ((SwFrmFmt &)rFrmFmt).MakeGraphic( &aIMap ) );

    String aGrfNm;
    if( rHTMLWrt.GetOrigFileName() )
        aGrfNm = *rHTMLWrt.GetOrigFileName();
    if( aGrf.GetType() == GRAPHIC_NONE ||
        XOutBitmap::WriteGraphic( aGrf, aGrfNm,
                                  String::CreateFromAscii( "JPG" ),
                                  XOUTBMP_USE_GIF_IF_POSSIBLE ) != 0 )
    {
        // leer oder fehlerhaft, da ist nichts auszugeben
        rHTMLWrt.nWarn = WARN_SWG_POOR_LOAD | WARN_SW_WRITE_BASE;
        return rWrt;
    }

    aGrfNm = URIHelper::SmartRelToAbs( aGrfNm );
    if( rHTMLWrt.HasCId() )
        rHTMLWrt.MakeCIdURL( aGrfNm );
    Size aSz( 0, 0 );
    ULONG nFrmFlags = bInCntnr ? HTML_FRMOPTS_GENIMG_CNTNR
                                : HTML_FRMOPTS_GENIMG;
    OutHTML_Image( rWrt, rFrmFmt, aGrfNm, rFrmFmt.GetName(), aSz,
                    HTML_FRMOPTS_GENIMG, pMarkToFrame,
                    aIMap.GetIMapObjectCount() ? &aIMap : 0 );
    return rWrt;
}


static Writer& OutHTML_FrmFmtGrfNode( Writer& rWrt, const SwFrmFmt& rFrmFmt,
                                      BOOL bInCntnr )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    const SwFmtCntnt& rFlyCntnt = rFrmFmt.GetCntnt();
    ULONG nStt = rFlyCntnt.GetCntntIdx()->GetIndex()+1;
    SwGrfNode *pGrfNd = rHTMLWrt.pDoc->GetNodes()[ nStt ]->GetGrfNode();
    ASSERT( pGrfNd, "Grf-Node erwartet" );
    if( !pGrfNd )
        return rWrt;

    const SwMirrorGrf& rMirror = pGrfNd->GetSwAttrSet().GetMirrorGrf();

    String aGrfNm;
    if( !pGrfNd->IsLinkedFile() || RES_DONT_MIRROR_GRF != rMirror.GetValue() )
    {
        // Grafik als File-Referenz speichern (als JPEG-Grafik speichern)
        if( rHTMLWrt.GetOrigFileName() )
            aGrfNm = *rHTMLWrt.GetOrigFileName();
        pGrfNd->SwapIn( TRUE );

        ULONG nFlags = XOUTBMP_USE_GIF_IF_SENSIBLE;
        switch( rMirror.GetValue() )
        {
        case RES_MIRROR_GRF_VERT:   nFlags = XOUTBMP_MIRROR_HORZ; break;
        case RES_MIRROR_GRF_HOR:    nFlags = XOUTBMP_MIRROR_VERT; break;
        case RES_MIRROR_GRF_BOTH:
            nFlags = XOUTBMP_MIRROR_VERT | XOUTBMP_MIRROR_HORZ;
            break;
        }

        Size aMM100Size;
        const SwFmtFrmSize& rSize = rFrmFmt.GetFrmSize();
        aMM100Size = OutputDevice::LogicToLogic( rSize.GetSize(),
                        MapMode( MAP_TWIP ), MapMode( MAP_100TH_MM ));

        USHORT nErr = XOutBitmap::WriteGraphic( pGrfNd->GetGrf(), aGrfNm,
                String::CreateFromAscii("JPG"), nFlags, &aMM100Size );
        if( nErr )              // fehlerhaft, da ist nichts auszugeben
        {
            rHTMLWrt.nWarn = WARN_SWG_POOR_LOAD | WARN_SW_WRITE_BASE;
            return rWrt;
        }
        aGrfNm = URIHelper::SmartRelToAbs( aGrfNm );
    }
    else
    {
        pGrfNd->GetFileFilterNms( &aGrfNm, 0 );
        if( rHTMLWrt.bCfgCpyLinkedGrfs || rHTMLWrt.HasCId() )
            rWrt.CopyLocalFileToINet( aGrfNm, rHTMLWrt.HasCId() );
    }
    if( rHTMLWrt.HasCId() )
        rHTMLWrt.MakeCIdURL( aGrfNm );

    ULONG nFrmFlags = bInCntnr ? HTML_FRMOPTS_IMG_CNTNR : HTML_FRMOPTS_IMG;
    if( rHTMLWrt.IsHTMLMode( HTMLMODE_ABS_POS_FLY ) && !bInCntnr )
        nFrmFlags |= HTML_FRMOPTS_IMG_CSS1;
    OutHTML_Image( rWrt, rFrmFmt, aGrfNm, pGrfNd->GetAlternateText(),
                   pGrfNd->GetTwipSize(), nFrmFlags, pMarkToGraphic );

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
    SwHTMLWriter::GetEEAttrsFromDrwObj( aItemSet, &rSdrObj, TRUE );
    BOOL bCfgOutStylesOld = rHTMLWrt.bCfgOutStyles;
    rHTMLWrt.bCfgOutStyles = FALSE;
    rHTMLWrt.bTxtAttr = TRUE;
    rHTMLWrt.bTagOn = TRUE;
    Out_SfxItemSet( aHTMLAttrFnTab, rWrt, aItemSet, FALSE );
    rHTMLWrt.bTxtAttr = FALSE;

    OutHTML_DrawFrmFmtAsMarquee( rHTMLWrt,
                                 (const SwDrawFrmFmt &)rFrmFmt,
                                 rSdrObj );
    rHTMLWrt.bTxtAttr = TRUE;
    rHTMLWrt.bTagOn = FALSE;
    Out_SfxItemSet( aHTMLAttrFnTab, rWrt, aItemSet, FALSE );
    rHTMLWrt.bTxtAttr = FALSE;
    rHTMLWrt.bCfgOutStyles = bCfgOutStylesOld;

    return rWrt;
}

//-----------------------------------------------------------------------

Writer& OutHTML_HeaderFooter( Writer& rWrt, const SwFrmFmt& rFrmFmt,
                              BOOL bHeader )
{
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;

    // als Multicol ausgeben
    rHTMLWrt.OutNewLine();
    ByteString sOut( sHTML_division );
    ((sOut += ' ') += sHTML_O_type) += '=';
    sOut += (bHeader ? "HEADER" : "FOOTER" );
    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sOut.GetBuffer() );

    rHTMLWrt.IncIndentLevel();  // den Inhalt von Multicol einruecken;

    // Einen Spacer fuer den Absatnd zusammenbasteln. Da durch das
    // <DL> bzw. </DL> immer einer Absatz-Abstand entsteht, wird der
    // ggf. abgezogen.
    const SvxULSpaceItem& rULSpace = rFrmFmt.GetULSpace();
    USHORT nSize = bHeader ? rULSpace.GetLower() : rULSpace.GetUpper();
    rHTMLWrt.nHeaderFooterSpace = nSize;

    ByteString aSpacer;
    if( rHTMLWrt.IsHTMLMode(HTMLMODE_VERT_SPACER) &&
        nSize > HTML_PARSPACE && Application::GetDefaultDevice() )
    {
        nSize -= HTML_PARSPACE;
        nSize = (INT16)Application::GetDefaultDevice()
            ->LogicToPixel( Size(nSize,0), MapMode(MAP_TWIP) ).Width();

        ((((((((aSpacer = sHTML_spacer) += ' ')
            += sHTML_O_type) += '=') += sHTML_SPTYPE_vertical) += ' ')
            += sHTML_O_size) += '=') += ByteString::CreateFromInt32(nSize);
    }

    const SwFmtCntnt& rFlyCntnt = rFrmFmt.GetCntnt();
    ULONG nStt = rFlyCntnt.GetCntntIdx()->GetIndex();
    const SwStartNode* pSttNd = rWrt.pDoc->GetNodes()[nStt]->GetStartNode();
    ASSERT( pSttNd, "Wo ist der Start-Node" );

    if( !bHeader && aSpacer.Len() )
    {
        rHTMLWrt.OutNewLine();
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), aSpacer.GetBuffer() );
    }

    {
        // in einem Block damit rechtzeitig vor dem Ende der alte Zustand
        // wieder hergestellt wird. pFlyFmt braucht hier nicht gestzt zu
        // werden, denn PageDesc-Attribute koennen hier nicht vorkommen
        HTMLSaveData aSaveData( rHTMLWrt, nStt+1,
                                pSttNd->EndOfSectionIndex(),
                                rHTMLWrt.GetFlyFrmFmt() );

        if( bHeader )
            rHTMLWrt.bOutHeader = TRUE;
        else
            rHTMLWrt.bOutFooter = TRUE;

        rHTMLWrt.Out_SwDoc( rWrt.pCurPam );
    }

    if( bHeader && aSpacer.Len() )
    {
        rHTMLWrt.OutNewLine();
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), aSpacer.GetBuffer() );
    }

    rHTMLWrt.DecIndentLevel();  // den Inhalt von Multicol einruecken;
    rHTMLWrt.OutNewLine();
    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sHTML_division, FALSE );

    rHTMLWrt.nHeaderFooterSpace = 0;

    return rWrt;
}


void SwHTMLWriter::AddLinkTarget( const String& rURL )
{
    if( !rURL.Len() || rURL.GetChar(0) != '#' )
        return;

    xub_StrLen nLastPos, nPos = rURL.Search( cMarkSeperator );
    if( STRING_NOTFOUND == nPos )
        return;

    while( STRING_NOTFOUND !=
                ( nLastPos = rURL.Search( cMarkSeperator, nPos + 1 )) )
        nPos = nLastPos;

    if( nPos<2 ) // mindetsens "#a|..."
        return;

    String aURL( rURL );
    aURL.Erase( 0, 1 );

    String sCmp( aURL.Copy( nPos ) ); // nPos-1+1 (-1 wg. Erase)
    sCmp.EraseAllChars();
    if( !sCmp.Len() )
        return;

    sCmp.ToLowerAscii();

    if( sCmp.EqualsAscii( pMarkToRegion ) ||
        sCmp.EqualsAscii( pMarkToFrame ) ||
        sCmp.EqualsAscii( pMarkToGraphic ) ||
        sCmp.EqualsAscii( pMarkToOLE ) ||
        sCmp.EqualsAscii( pMarkToTable ) )
    {
        // Einfach nur in einem sortierten Array merken
        aImplicitMarks.Insert( new String( aURL ) );
    }
    else if( sCmp.EqualsAscii( pMarkToOutline ) )
    {
        // Hier brauchen wir Position und Name. Deshalb sortieren wir
        // ein USHORT und ein String-Array selbst
        String aOutline( aURL.Copy( 0, nPos-1 ) );
        SwPosition aPos( *pCurPam->GetPoint() );
        if( pDoc->GotoOutline( aPos, aOutline ) )
        {
            ULONG nIdx = aPos.nNode.GetIndex();

            USHORT nPos=0;
            while( nPos < aOutlineMarkPoss.Count() &&
                   aOutlineMarkPoss[nPos] < nIdx )
                nPos++;

            aOutlineMarkPoss.Insert( nIdx, nPos );
            aOutlineMarks.Insert( new String( aURL ), nPos );
        }
    }
    else if( sCmp.EqualsAscii( pMarkToText ) )
    {
        //
    }
}

void SwHTMLWriter::CollectLinkTargets()
{
    const SwFmtINetFmt* pINetFmt;
    const SwTxtINetFmt* pTxtAttr;
    const SwTxtNode* pTxtNd;

    USHORT n, nMaxItems = pDoc->GetAttrPool().GetItemCount( RES_TXTATR_INETFMT );
    for( n = 0; n < nMaxItems; ++n )
    {
        if( 0 != (pINetFmt = (SwFmtINetFmt*)pDoc->GetAttrPool().GetItem(
            RES_TXTATR_INETFMT, n ) ) &&
            0 != ( pTxtAttr = pINetFmt->GetTxtINetFmt()) &&
            0 != ( pTxtNd = pTxtAttr->GetpTxtNode() ) &&
            pTxtNd->GetNodes().IsDocNodes() )
        {
            AddLinkTarget( pINetFmt->GetValue() );
        }
    }

    const SwFmtURL *pURL;
    nMaxItems = pDoc->GetAttrPool().GetItemCount( RES_URL );
    for( n = 0; n < nMaxItems; ++n )
    {
        if( 0 != (pURL = (SwFmtURL*)pDoc->GetAttrPool().GetItem(
            RES_URL, n ) ) )
        {
            AddLinkTarget( pURL->GetURL() );
            const ImageMap *pIMap = pURL->GetMap();
            if( pIMap )
            {
                for( USHORT i=0; i<pIMap->GetIMapObjectCount(); i++ )
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

//-----------------------------------------------------------------------

SwHTMLPosFlyFrm::SwHTMLPosFlyFrm( const SwPosFlyFrm& rPosFly,
                                  const SdrObject *pSdrObj,
                                  BYTE nOutMode ) :
    pFrmFmt( &rPosFly.GetFmt() ),
    pSdrObject( pSdrObj ),
    pNdIdx( new SwNodeIndex( rPosFly.GetNdIndex() ) ),
    nOrdNum( rPosFly.GetOrdNum() ),
    nCntntIdx( 0 ),
    nOutputMode( nOutMode )
{
    const SwFmtAnchor& rAnchor = rPosFly.GetFmt().GetAnchor();
    if( FLY_AUTO_CNTNT==rAnchor.GetAnchorId() &&
        HTML_POS_INSIDE == GetOutPos() )
    {
        // Auto-gebundene Rahmen werden ein Zeichen weiter hinten
        // ausgegeben, weil dann die Positionierung mit Netscape
        // uebereinstimmt.
        ASSERT( rAnchor.GetCntntAnchor(), "Keine Anker-Position?" );
        if( rAnchor.GetCntntAnchor() )
        {
            nCntntIdx = rAnchor.GetCntntAnchor()->nContent.GetIndex();
            SwRelationOrient eHoriRel = rPosFly.GetFmt().GetHoriOrient().
                                                GetRelationOrient();
            if( FRAME == eHoriRel || PRTAREA == eHoriRel )
            {
                const SwCntntNode *pCNd = pNdIdx->GetNode().GetCntntNode();
                ASSERT( pCNd, "Kein Content-Node an PaM-Position" );
                if( pCNd && nCntntIdx < pCNd->Len() )
                    nCntntIdx++;
            }
        }
    }
}

BOOL SwHTMLPosFlyFrm::operator<( const SwHTMLPosFlyFrm& rFrm ) const
{
    if( pNdIdx->GetIndex() == rFrm.pNdIdx->GetIndex() )
    {
        if( nCntntIdx == rFrm.nCntntIdx )
        {
            if( GetOutPos() == rFrm.GetOutPos() )
                return nOrdNum < rFrm.nOrdNum;
            else
                return GetOutPos() < rFrm.GetOutPos();
        }
        else
            return nCntntIdx < rFrm.nCntntIdx;
    }
    else
        return pNdIdx->GetIndex() < rFrm.pNdIdx->GetIndex();
}

/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/html/htmlfly.cxx,v 1.2 2000-10-20 13:42:52 jp Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.1.1.1  2000/09/18 17:14:55  hr
      initial import

      Revision 1.81  2000/09/18 16:04:44  willem.vandorp
      OpenOffice header added.

      Revision 1.80  2000/08/18 13:03:15  jp
      don't export escaped URLs

      Revision 1.79  2000/06/26 12:55:05  os
      INetURLObject::SmartRelToAbs removed

      Revision 1.78  2000/06/26 09:52:01  jp
      must change: GetAppWindow->GetDefaultDevice

      Revision 1.77  2000/05/15 16:35:52  jp
      Changes for Unicode

      Revision 1.76  2000/04/28 14:29:11  mib
      unicode

      Revision 1.75  2000/04/10 12:20:56  mib
      unicode

      Revision 1.74  2000/02/11 14:36:57  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.73  1999/09/21 09:49:42  mib
      multiple text encodings

      Revision 1.72  1999/07/09 10:40:20  MIB
      #67508#: <BR CLEAR> for Frames that are centered


      Rev 1.71   09 Jul 1999 12:40:20   MIB
   #67508#: <BR CLEAR> for Frames that are centered

      Rev 1.70   16 Jun 1999 19:48:10   JP
   Change interface of base class Writer

*************************************************************************/

