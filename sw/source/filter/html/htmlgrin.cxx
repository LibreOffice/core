/*************************************************************************
 *
 *  $RCSfile: htmlgrin.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:55 $
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

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX //autogen
#include <svx/adjitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _IMAP_HXX //autogen
#include <svtools/imap.hxx>
#endif
#ifndef _HTMLTOKN_H
#include <svtools/htmltokn.h>
#endif
#ifndef _HTMLKYWD_HXX
#include <svtools/htmlkywd.hxx>
#endif


#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTURL_HXX //autogen
#include <fmturl.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTSRND_HXX //autogen
#include <fmtsrnd.hxx>
#endif
#ifndef _FMTINFMT_HXX //autogen
#include <fmtinfmt.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTINFMT_HXX //autogen
#include <fmtinfmt.hxx>
#endif
#ifndef _FRMATR_HXX
#include "frmatr.hxx"
#endif
#ifndef _CHARATR_HXX
#include "charatr.hxx"
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _CHARFMT_HXX //autogen
#include <charfmt.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _SWDOCSH_HXX //autogen
#include <docsh.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _BOOKMRK_HXX
#include <bookmrk.hxx>
#endif
#ifndef _NDGRF_HXX
#include <ndgrf.hxx>
#endif
#ifndef _HTMLNUM_HXX
#include <htmlnum.hxx>
#endif
#ifndef _SWCSS1_HXX
#include <swcss1.hxx>
#endif
#ifndef _SWHTML_HXX
#include <swhtml.hxx>
#endif


HTMLOptionEnum __FAR_DATA aHTMLImgHAlignTable[] =
{
    { sHTML_AL_left,    HORI_LEFT       },
    { sHTML_AL_right,   HORI_RIGHT      },
    { 0,                0               }
};


HTMLOptionEnum __FAR_DATA aHTMLImgVAlignTable[] =
{
    { sHTML_VA_top,         VERT_LINE_TOP       },
    { sHTML_VA_texttop,     VERT_CHAR_TOP       },
    { sHTML_VA_middle,      VERT_CENTER         },
    { sHTML_AL_center,      VERT_CENTER         },
    { sHTML_VA_absmiddle,   VERT_LINE_CENTER    },
    { sHTML_VA_bottom,      VERT_TOP            },
    { sHTML_VA_baseline,    VERT_TOP            },
    { sHTML_VA_absbottom,   VERT_LINE_BOTTOM    },
    { 0,                    0                   }
};

SV_IMPL_PTRARR( ImageMaps, ImageMapPtr )

ImageMap *SwHTMLParser::FindImageMap( const String& rName ) const
{
    ImageMap *pImageMap = 0;

    ASSERT( rName.GetChar(0) != '#', "FindImageName: Name beginnt mit #!" );

    if( pImageMaps )
    {
        for( USHORT i=0; i<pImageMaps->Count(); i++ )
        {
            ImageMap *pIMap = (*pImageMaps)[i];
            if( rName.EqualsIgnoreCaseAscii( pIMap->GetName() ) )
            {
                pImageMap = pIMap;
                break;
            }
        }
    }
    return pImageMap;
}

void SwHTMLParser::ConnectImageMaps()
{
    SwNodes& rNds = pDoc->GetNodes();
    // auf den Start-Node der 1. Section
    ULONG nIdx = rNds.GetEndOfAutotext().StartOfSectionIndex() + 1;
    ULONG nEndIdx = rNds.GetEndOfAutotext().GetIndex();

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
                pFmt->SetAttr( aURL );
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


/*  */

void SwHTMLParser::SetAnchorAndAdjustment( SwVertOrient eVertOri,
                                           SwHoriOrient eHoriOri,
                                           const SfxItemSet &rCSS1ItemSet,
                                           const SvxCSS1PropertyInfo &rCSS1PropInfo,
                                           SfxItemSet& rFrmItemSet )
{
    const SfxItemSet *pCntnrItemSet = 0;
    USHORT i = aContexts.Count();
    while( !pCntnrItemSet && i > nContextStMin )
        pCntnrItemSet = aContexts[--i]->GetFrmItemSet();

    if( pCntnrItemSet )
    {
        // Wenn wir und in einem Container befinden wird die Verankerung
        // des Containers uebernommen.
        rFrmItemSet.Put( *pCntnrItemSet );
    }
    else if( pCSS1Parser->MayBePositioned( rCSS1PropInfo, TRUE ) )
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

void SwHTMLParser::SetAnchorAndAdjustment( SwVertOrient eVertOri,
                                           SwHoriOrient eHoriOri,
                                           SfxItemSet& rFrmSet,
                                           BOOL bDontAppend )
{
    BOOL bMoveBackward = FALSE;
    SwFmtAnchor aAnchor( FLY_IN_CNTNT );
    SwRelationOrient eVertRel = FRAME;

    if( HORI_NONE != eHoriOri )
    {
        // den Absatz-Einzug bestimmen
        USHORT nLeftSpace = 0, nRightSpace = 0;
        short nIndent = 0;
        GetMarginsFromContextWithNumBul( nLeftSpace, nRightSpace, nIndent );

        // Horizonale Ausrichtung und Umlauf bestimmen.
        SwRelationOrient eHoriRel;
        SwSurround eSurround;
        switch( eHoriOri )
        {
        case HORI_LEFT:
            eHoriRel = nLeftSpace ? PRTAREA : FRAME;
            eSurround = SURROUND_RIGHT;
            break;
        case HORI_RIGHT:
            eHoriRel = nRightSpace ? PRTAREA : FRAME;
            eSurround = SURROUND_LEFT;
            break;
        case HORI_CENTER:   // fuer Tabellen
            eHoriRel = FRAME;
            eSurround = SURROUND_NONE;
            break;
        default:
            eHoriRel = FRAME;
            eSurround = SURROUND_PARALLEL;
            break;
        }

        // Einen neuen Absatz aufmachen, wenn der aktuelle
        // absatzgebundene Rahmen ohne Umlauf enthaelt.
        if( !bDontAppend && HasCurrentParaFlys( TRUE ) )
        {
            // Wenn der Absatz nur Grafiken enthaelt, braucht er
            // auch keinen unteren Absatz-Abstand. Da hier auch bei
            // Verwendung von Styles kein Abstand enstehen soll, wird
            // hier auch geweohnlich attributiert !!!
            USHORT nUpper=0, nLower=0;
            GetULSpaceFromContext( nUpper, nLower );
            InsertAttr( SvxULSpaceItem( nUpper, 0 ), FALSE, TRUE );

            AppendTxtNode( AM_NOSPACE );

            if( nUpper )
            {
                NewAttr( &aAttrTab.pULSpace, SvxULSpaceItem( 0, nLower ) );
                aParaAttrs.Insert( aAttrTab.pULSpace, aParaAttrs.Count() );
                EndAttr( aAttrTab.pULSpace, 0, FALSE );
            }
        }

        // Vertikale Ausrichtung und Verankerung bestimmen.
        xub_StrLen nCntnt = pPam->GetPoint()->nContent.GetIndex();
        if( nCntnt )
        {
            aAnchor.SetType( FLY_AUTO_CNTNT );
            bMoveBackward = TRUE;
            eVertOri = VERT_CHAR_BOTTOM;
            eVertRel = REL_CHAR;
        }
        else
        {
            aAnchor.SetType( FLY_AT_CNTNT );
            eVertOri = VERT_TOP;
            eVertRel = PRTAREA;
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
        FLY_AT_CNTNT == pFlyFmt->GetAnchor().GetAnchorId() &&
        SURROUND_THROUGHT == pFlyFmt->GetSurround().GetSurround() )
    {
        aMoveFlyFrms.Insert( pFlyFmt, aMoveFlyFrms.Count() );
        aMoveFlyCnts.Insert( pPam->GetPoint()->nContent.GetIndex(),
                             aMoveFlyCnts.Count() );
    }
}

/*  */

void SwHTMLParser::GetDefaultScriptType( ScriptType& rType,
                                         String& rTypeStr ) const
{
    SwDocShell *pDocSh = pDoc->GetDocShell();
    SvKeyValueIterator* pHeaderAttrs = pDocSh ? pDocSh->GetHeaderAttributes()
                                              : 0;
    rType = GetScriptType( pHeaderAttrs );
    rTypeStr = GetScriptTypeString( pHeaderAttrs );
}

/*  */

void SwHTMLParser::InsertImage()
{
    // und jetzt auswerten
    String sGrfNm, sAltNm, aId, aClass, aStyle, aMap, sHTMLGrfName;
    SwVertOrient eVertOri = VERT_TOP;
    SwHoriOrient eHoriOri = HORI_NONE;
    long nWidth=0, nHeight=0;
    long nVSpace=0, nHSpace=0;

    USHORT nBorder = (aAttrTab.pINetFmt ? 1 : 0);
    BOOL bIsMap = FALSE;
    BOOL bPrcWidth = FALSE;
    BOOL bPrcHeight = FALSE;
    SvxMacroItem aMacroItem;

    ScriptType eDfltScriptType;
    String sDfltScriptType;
    GetDefaultScriptType( eDfltScriptType, sDfltScriptType );

    const HTMLOptions *pOptions = GetOptions();
    for( USHORT i = pOptions->Count(); i; )
    {
        USHORT nEvent = 0;
        ScriptType eScriptType = eDfltScriptType;
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
            case HTML_O_SRC:
                ASSERT( INetURLObject::GetBaseURL() == sBaseURL,
                        "<IMG>: Base URL ist zerschossen" );
                sGrfNm = pOption->GetString();
                if( !InternalImgToPrivateURL(sGrfNm) )
                    sGrfNm = INetURLObject::RelToAbs( sGrfNm );
                break;
            case HTML_O_ALIGN:
                eVertOri =
                    (SwVertOrient)pOption->GetEnum( aHTMLImgVAlignTable,
                                                    VERT_TOP );
                eHoriOri =
                    (SwHoriOrient)pOption->GetEnum( aHTMLImgHAlignTable,
                                                    HORI_NONE );
                break;
            case HTML_O_WIDTH:
                // erstmal nur als Pixelwerte merken!
                nWidth = pOption->GetNumber();
                bPrcWidth = (pOption->GetString().Search('%') != STRING_NOTFOUND);
                if( bPrcWidth && nWidth>100 )
                    nWidth = 100;
                break;
            case HTML_O_HEIGHT:
                // erstmal nur als Pixelwerte merken!
                nHeight = pOption->GetNumber();
                bPrcHeight = (pOption->GetString().Search('%') != STRING_NOTFOUND);
                if( bPrcHeight && nHeight>100 )
                    nHeight = 100;
                break;
            case HTML_O_VSPACE:
                nVSpace = pOption->GetNumber();
                break;
            case HTML_O_HSPACE:
                nHSpace = pOption->GetNumber();
                break;
            case HTML_O_ALT:
                sAltNm = pOption->GetString();
                break;
            case HTML_O_BORDER:
                nBorder = (USHORT)pOption->GetNumber();
                break;
            case HTML_O_ISMAP:
                bIsMap = TRUE;
                break;
            case HTML_O_USEMAP:
                aMap = pOption->GetString();
                break;
            case HTML_O_NAME:
                sHTMLGrfName = pOption->GetString();
                break;

            case HTML_O_SDONLOAD:
                eScriptType = STARBASIC;
            case HTML_O_ONLOAD:
                nEvent = SVX_EVENT_IMAGE_LOAD;
                goto IMAGE_SETEVENT;

            case HTML_O_SDONABORT:
                eScriptType = STARBASIC;
            case HTML_O_ONABORT:
                nEvent = SVX_EVENT_IMAGE_ABORT;
                goto IMAGE_SETEVENT;

            case HTML_O_SDONERROR:
                eScriptType = STARBASIC;
            case HTML_O_ONERROR:
                nEvent = SVX_EVENT_IMAGE_ERROR;
                goto IMAGE_SETEVENT;
IMAGE_SETEVENT:
                {
                    String sTmp( pOption->GetString() );
                    if( sTmp.Len() )
                    {
                        sTmp.ConvertLineEnd();
                        String sScriptType;
                        if( EXTENDED_STYPE == eScriptType )
                            sScriptType = sDfltScriptType;
                        aMacroItem.SetMacro( nEvent,
                            SvxMacro( sTmp, sScriptType, eScriptType ));
                    }
                }
                break;
        }
    }

    if( !sGrfNm.Len() )
        return;

    // Wenn wir in einer Numerierung stehen und der Absatz noch leer und
    // nicht numeriert ist, handelt es sich vielleicht um die Grafik
    // einer Bullet-Liste
    if( !pPam->GetPoint()->nContent.GetIndex() &&
        GetNumInfo().GetDepth() > 0 && GetNumInfo().GetDepth() <= MAXLEVEL &&
        aBulletGrfs[GetNumInfo().GetDepth()-1].Len() &&
        aBulletGrfs[GetNumInfo().GetDepth()-1]==sGrfNm )
    {
        SwTxtNode* pTxtNode = pPam->GetNode()->GetTxtNode();
        if( pTxtNode && pTxtNode->GetNum() &&
            NO_NUMLEVEL & pTxtNode->GetNum()->GetLevel() )
        {
            SwNodeNum aNum( *pTxtNode->GetNum() );
            aNum.SetLevel( aNum.GetLevel() & ~NO_NUMLEVEL );
            ASSERT( aNum.GetLevel() == GetNumInfo().GetLevel(),
                    "Numerierungs-Ebene stimmt nicht" );
            pTxtNode->UpdateNum( aNum );

            // Rule invalisieren ist noetig, weil zwischem dem einlesen
            // des LI und der Grafik ein EndAction gerufen worden sein kann.
            if( GetNumInfo().GetNumRule() )
                GetNumInfo().GetNumRule()->SetInvalidRule( TRUE );

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

        SvxBorderLine aHBorderLine;
        SvxBorderLine aVBorderLine;

        SvxCSS1Parser::SetBorderWidth( aHBorderLine,
                                       (USHORT)nHBorderWidth, FALSE );
        if( nHBorderWidth == nVBorderWidth )
            aVBorderLine.SetOutWidth( aHBorderLine.GetOutWidth() );
        else
            SvxCSS1Parser::SetBorderWidth( aVBorderLine,
                                           (USHORT)nVBorderWidth, FALSE );

        // die tatsaechlich gesetzter Rahmenbreite benutzen und nicht die
        // Wunschbreite!
        nHBorderWidth = aHBorderLine.GetOutWidth();
        nVBorderWidth = aVBorderLine.GetOutWidth();

        if( aAttrTab.pINetFmt )
        {
            const String& rURL =
                ((const SwFmtINetFmt&)aAttrTab.pINetFmt->GetItem()).GetValue();

            pCSS1Parser->SetATagStyles();
            USHORT nPoolId =  pDoc->IsVisitedURL( rURL )
                                    ? RES_POOLCHR_INET_VISIT
                                    : RES_POOLCHR_INET_NORMAL;
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


        SvxBoxItem aBoxItem;
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
            bPrcWidth = FALSE;
            break;
        case SVX_CSS1_LTYPE_PERCENTAGE:
            aTwipSz.Width() = 0;
            nWidth = aPropInfo.nWidth;
            bPrcWidth = TRUE;
            break;
    }
    switch( aPropInfo.eHeightType )
    {
        case SVX_CSS1_LTYPE_TWIP:
            aTwipSz.Height() = aPropInfo.nHeight;
            nHeight = 1;    // != 0
            bPrcHeight = FALSE;
            break;
        case SVX_CSS1_LTYPE_PERCENTAGE:
            aTwipSz.Height() = 0;
            nHeight = aPropInfo.nHeight;
            bPrcHeight = TRUE;
            break;
    }

    Size aGrfSz( 0, 0 );
    BOOL bSetTwipSize = TRUE;       // Twip-Size am Node setzen?
    BOOL bChangeFrmSize = FALSE;    // Frame-Format nachtraeglich anpassen?
    BOOL bGrfSzValid = FALSE;       // aGrfSz enthaelt die Grafik-Groesse
    BOOL bRequestGrfNow = FALSE;
    BOOL bSetScaleImageMap = FALSE;
    BYTE nPrcWidth = 0, nPrcHeight = 0;

    if( !nWidth || !nHeight )
    {
        // Es fehlt die Breite oder die Hoehe

        // JP 28.05.96: dann suche doch mal in unserem QuickDraw-Cache, ob
        //              die Groesse dieser Grafik schon vorhanden ist.

// Code for the new GraphicObject
#ifdef USE_GRFOBJECT

        bGrfSzValid =
#ifdef NEW_GRFOBJ
            FindGrfSizeFromCache( sGrfNm, aGrfSz )
#else
                FALSE;
#endif
            ;

#else
// USE_GRFOBJECT

        bGrfSzValid = FindGrfSizeFromCache( sGrfNm, aGrfSz );

#endif
// USE_GRFOBJECT

        // Wenn die Grfik in einer Tabelle steht, wird sie gleich
        // angefordert, damit sie eventuell schon da ist, bevor die
        // Tabelle layoutet wird.
        if( !bGrfSzValid && pTable!=0 && !nWidth )
        {
            bRequestGrfNow = TRUE;
            IncGrfsThatResizeTable();
        }

        if( bGrfSzValid && aGrfSz.Width() && aGrfSz.Height() )
        {
            // wir haben eine Groesse und koenen sie setzen
            if( !nWidth && !nHeight )
            {
                // Hoehe und Breite fehlen
                aTwipSz = aGrfSz;
            }
            else if( nWidth )
            {
                // nur die Breite fehlt
                if( bPrcWidth )
                {
                    // %-Breite uebernehmen und Hoehe skalieren
                    nPrcWidth = (BYTE)nWidth;
                    nPrcHeight = 255;
                }
                else
                {
                    // Breite uebernehmen und Hoehe berechnen
                    aTwipSz.Height() = (aGrfSz.Height() * aTwipSz.Width())
                                        / aGrfSz.Width();
                }
            }
            else if( nHeight )
            {
                // nur die Hoehe fehlt
                if( bPrcHeight )
                {
                    // %-Hoehe lassen und Breite skalieren
                    nPrcHeight = (BYTE)nHeight;
                    nPrcWidth = 255;
                }
                else
                {
                    // Hoehe uebernehmen und Breite berechnen
                    aTwipSz.Width() = (aGrfSz.Width() * aTwipSz.Height())
                                        / aGrfSz.Height();
                }
            }
        }
        else
        {
            // wir haben keine Groesse gefunden (und sind in keiner
            // Tabelle oder haben zumindest die Breite der Grafik)

            // Die Groesse des Rahmens wird nachtraeglich gesetzt
            bChangeFrmSize = TRUE;
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
                    nPrcWidth = (BYTE)nWidth;
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
                    nPrcHeight = (BYTE)nHeight;
                    nPrcWidth = 255;
                }
                else
                {
                    aTwipSz.Width() = HTML_DFLT_IMG_WIDTH;
                }
            }
        }
    }
    else
    {
        // Breite und Hoehe wurden angegeben und brauchen nicht gesetzt
        // zu werden
// Code for the new GraphicObject
#ifdef USE_GRFOBJECT

        bGrfSzValid =
#ifdef NEW_GRFOBJ
        FindGrfSizeFromCache( sGrfNm, aGrfSz )
#else
            FALSE;
#endif
        ;


#else
// USE_GRFOBJECT

        bGrfSzValid = FindGrfSizeFromCache( sGrfNm, aGrfSz );
#endif
// USE_GRFOBJECT

        bSetTwipSize = FALSE;

        if( bPrcWidth )
            nPrcWidth = (BYTE)nWidth;

        if( bPrcHeight )
            nPrcHeight = (BYTE)nHeight;
    }

    // Image-Map setzen
    aMap.EraseTrailingChars();
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
            if( bGrfSzValid )
            {
                BOOL bScale = FALSE;

                Fraction aScaleX( 1, 1 ), aScaleY( 1, 1);

                if( !nPrcWidth && aGrfSz.Width() && aTwipSz.Width() &&
                    aGrfSz.Width() != aTwipSz.Width() )
                {
                    aScaleX = Fraction( aGrfSz.Width(), aTwipSz.Width() );
                    bScale = TRUE;
                }

                if( !nPrcHeight && aGrfSz.Height() && aTwipSz.Height() &&
                    aGrfSz.Height() != aTwipSz.Height() )
                {
                    aScaleY = Fraction( aGrfSz.Height(), aTwipSz.Height() );
                    bScale = TRUE;
                }

                if( bScale )
                    aURL.GetMap()->Scale( aScaleX, aScaleY );
            }
            else
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
            bSetScaleImageMap = !bSetTwipSize || !bGrfSzValid ||
                                !nPrcWidth || !nPrcHeight;
        }
    }

    // min. Werte einhalten !!
    if( nPrcWidth )
    {
        ASSERT( !aTwipSz.Width(),
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
        ASSERT( !aTwipSz.Height(),
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
                                      &aFrmSet );
    SwGrfNode *pGrfNd = pDoc->GetNodes()[ pFlyFmt->GetCntnt().GetCntntIdx()
                                  ->GetIndex()+1 ]->GetGrfNode();

    if( sHTMLGrfName.Len() )
    {
        pFlyFmt->SetName( sHTMLGrfName );

        // ggfs. eine Grafik anspringen
        if( JUMPTO_GRAPHIC == eJumpTo && sHTMLGrfName == sJmpMark )
        {
            bChkJumpMark = TRUE;
            eJumpTo = JUMPTO_NONE;
        }
    }

    if( sAltNm.Len() )
        pGrfNd->SetAlternateText( sAltNm );

    if( bSetTwipSize )
        pGrfNd->SetTwipSize( aGrfSz );

    pGrfNd->SetChgTwipSize( bChangeFrmSize );

    if( bSetScaleImageMap )
        pGrfNd->SetScaleImageMap( TRUE );

    if( aAttrTab.pINetFmt )
    {
        const SwFmtINetFmt &rINetFmt =
            (const SwFmtINetFmt&)aAttrTab.pINetFmt->GetItem();

        SwFmtURL aURL( pFlyFmt->GetURL() );

        aURL.SetURL( rINetFmt.GetValue(), bIsMap );
        aURL.SetTargetFrameName( rINetFmt.GetTargetFrame() );
        aURL.SetName( rINetFmt.GetName() );
        pFlyFmt->SetAttr( aURL );

        {
            const SvxMacro *pMacro;
            static USHORT __READONLY_DATA aEvents[] = {
                SFX_EVENT_MOUSEOVER_OBJECT,
                SFX_EVENT_MOUSECLICK_OBJECT,
                SFX_EVENT_MOUSEOUT_OBJECT,
                0 };

            for( USHORT n = 0; aEvents[ n ]; ++n )
                if( 0 != ( pMacro = rINetFmt.GetMacro( aEvents[ n ] ) ))
                    aMacroItem.SetMacro( aEvents[ n ], *pMacro );
        }

        if( FLY_IN_CNTNT == pFlyFmt->GetAnchor().GetAnchorId() &&
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
            if( rINetFmt.GetName().Len() )
            {
                pPam->Move( fnMoveBackward );
                InsertBookmark( rINetFmt.GetName() );
                pPam->Move( fnMoveForward );
            }
        }

    }

    if( aMacroItem.GetMacroTable().Count() )
        pFlyFmt->SetAttr( aMacroItem );

    // Wenn die Grafik gleich angeforder wird, muss dies geschehen,
    // nachdem das Format vollstaendig aufgebaut ist, weil es evtl.
    // gleich (synchron) angepasst wird (war bug #40983#)
    if( bRequestGrfNow )
    {
        pGrfNd->SetTransferPriority( SFX_TFPRIO_VISIBLE_LOWRES_GRAPHIC );
        pGrfNd->SwapIn();
    }

    // Ggf. Frames anlegen und Auto-gebundenen Rahmen registrieren
    RegisterFlyFrm( pFlyFmt );

    if( aId.Len() )
        InsertBookmark( aId );
}

/*  */

void SwHTMLParser::InsertBodyOptions()
{
    pDoc->SetTxtFmtColl( *pPam,
                         pCSS1Parser->GetTxtCollFromPool( RES_POOLCOLL_TEXT ) );

    String aBackGround, aId, aStyle;
    Color aBGColor, aTextColor, aLinkColor, aVLinkColor;
    BOOL bBGColor=FALSE, bTextColor=FALSE;
    BOOL bLinkColor=FALSE, bVLinkColor=FALSE;

    ScriptType eDfltScriptType;
    String sDfltScriptType;
    GetDefaultScriptType( eDfltScriptType, sDfltScriptType );

    const HTMLOptions *pOptions = GetOptions();
    for( USHORT i = pOptions->Count(); i; )
    {
        const HTMLOption *pOption = (*pOptions)[--i];
        ScriptType eScriptType = eDfltScriptType;
        USHORT nEvent;
        BOOL bSetEvent = FALSE;

        switch( pOption->GetToken() )
        {
            case HTML_O_ID:
                aId = pOption->GetString();
                break;
            case HTML_O_BACKGROUND:
                aBackGround = pOption->GetString();
                break;
            case HTML_O_BGCOLOR:
                pOption->GetColor( aBGColor );
                bBGColor = TRUE;
                break;
            case HTML_O_TEXT:
                pOption->GetColor( aTextColor );
                bTextColor = TRUE;
                break;
            case HTML_O_LINK:
                pOption->GetColor( aLinkColor );
                bLinkColor = TRUE;
                break;
            case HTML_O_VLINK:
                pOption->GetColor( aVLinkColor );
                bVLinkColor = TRUE;
                break;

            case HTML_O_SDONLOAD:
                eScriptType = STARBASIC;
            case HTML_O_ONLOAD:
                nEvent = SFX_EVENT_OPENDOC;
                bSetEvent = TRUE;
                break;

            case HTML_O_SDONUNLOAD:
                eScriptType = STARBASIC;
            case HTML_O_ONUNLOAD:
                nEvent = SFX_EVENT_PREPARECLOSEDOC;
                bSetEvent = TRUE;
                break;

            case HTML_O_SDONFOCUS:
                eScriptType = STARBASIC;
            case HTML_O_ONFOCUS:
                nEvent = SFX_EVENT_ACTIVATEDOC;
                bSetEvent = TRUE;
                break;

            case HTML_O_SDONBLUR:
                eScriptType = STARBASIC;
            case HTML_O_ONBLUR:
                nEvent = SFX_EVENT_DEACTIVATEDOC;
                bSetEvent = TRUE;
                break;

            case HTML_O_ONERROR:
//              if( bAnyStarBasic )
//                  InsertBasicDocEvent( SFX_EVENT_ACTIVATEDOC,
//                                       pOption->GetString() );
                break;

            case HTML_O_STYLE:
                aStyle = pOption->GetString();
                bTextColor = TRUE;
                break;
        }

        if( bSetEvent )
        {
            const String& rEvent = pOption->GetString();
            if( rEvent.Len() )
                InsertBasicDocEvent( nEvent, rEvent, eScriptType,
                                     sDfltScriptType );
        }
    }

    if( bTextColor && !pCSS1Parser->IsBodyTextSet() )
    {
        // Die Textfarbe wird an der Standard-Vorlage gesetzt
        pCSS1Parser->GetTxtCollFromPool( RES_POOLCOLL_STANDARD )
            ->SetAttr( SvxColorItem(aTextColor) );
        pCSS1Parser->SetBodyTextSet();
    }


    // Die Item fuer die Seitenvorlage vorbereiten (Hintergrund, Umrandung)
    // Beim BrushItem muessen schon gesetzte werte erhalten bleiben!
    SvxBrushItem aBrushItem( pCSS1Parser->GetPageDescBackground() );
    BOOL bSetBrush = FALSE;

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
        bSetBrush = TRUE;
        pCSS1Parser->SetBodyBGColorSet();
    }

    if( aBackGround.Len() && !pCSS1Parser->IsBodyBackgroundSet() )
    {
        // Hintergrundgrafik aus "BACKGROUND"
        ASSERT( INetURLObject::GetBaseURL() == sBaseURL,
                "<BODY>: Base URL ist zerschossen" );
        aBrushItem.SetGraphicLink( INetURLObject::RelToAbs( aBackGround ) );
        aBrushItem.SetGraphicPos( GPOS_TILED );
        bSetBrush = TRUE;
        pCSS1Parser->SetBodyBackgroundSet();
    }

    if( aStyle.Len() )
    {
        SfxItemSet aItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;
        pCSS1Parser->ParseStyleOption( aStyle, aItemSet, aPropInfo );

        // Ein par Attribute muessen an der Seitenvorlage gesetzt werden,
        // und zwar die, die nicht vererbit werden
        pCSS1Parser->SetPageDescAttrs( bSetBrush ? &aBrushItem : 0,
                                       &aItemSet );

        // alle noch uebrigen Optionen koennen an der Standard-Vorlage
        // gesetzt werden und gelten dann automatisch als defaults
        pCSS1Parser->GetTxtCollFromPool( RES_POOLCOLL_STANDARD )
            ->SetAttr( aItemSet );
    }
    else if( bSetBrush )
    {
        pCSS1Parser->SetPageDescAttrs( &aBrushItem );
    }

    if( bLinkColor && !pCSS1Parser->IsBodyLinkSet() )
    {
        SwCharFmt *pCharFmt =
            pCSS1Parser->GetCharFmtFromPool(RES_POOLCHR_INET_NORMAL);
        pCharFmt->SetAttr( SvxColorItem(aLinkColor) );
        pCSS1Parser->SetBodyLinkSet();
    }
    if( bVLinkColor && !pCSS1Parser->IsBodyVLinkSet() )
    {
        SwCharFmt *pCharFmt =
            pCSS1Parser->GetCharFmtFromPool(RES_POOLCHR_INET_VISIT);
        pCharFmt->SetAttr( SvxColorItem(aVLinkColor) );
        pCSS1Parser->SetBodyVLinkSet();
    }

    if( aId.Len() )
        InsertBookmark( aId );
}

/*  */

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
    String aId, aStyle, aClass;
    BOOL bHasHRef = FALSE, bFixed = FALSE;

    ScriptType eDfltScriptType;
    String sDfltScriptType;
    GetDefaultScriptType( eDfltScriptType, sDfltScriptType );

    const HTMLOptions *pOptions = GetOptions();
    for( USHORT i = pOptions->Count(); i; )
    {
        USHORT nEvent = 0;
        ScriptType eScriptType = eDfltScriptType;
        const HTMLOption *pOption = (*pOptions)[--i];
        switch( pOption->GetToken() )
        {
            case HTML_O_NAME:
                aName = pOption->GetString();
                break;

            case HTML_O_HREF:
                sHRef = pOption->GetString();
                bHasHRef = TRUE;
                break;
            case HTML_O_TARGET:
                sTarget = pOption->GetString();
                break;

            case HTML_O_STYLE:
                aStyle = pOption->GetString();
                break;
            case HTML_O_ID:
                aId = pOption->GetString();
                break;
            case HTML_O_CLASS:
                aClass = pOption->GetString();
                break;
            case HTML_O_SDFIXED:
                bFixed = TRUE;
                break;

            case HTML_O_SDONCLICK:
                eScriptType = STARBASIC;
            case HTML_O_ONCLICK:
                nEvent = SFX_EVENT_MOUSECLICK_OBJECT;
                goto ANCHOR_SETEVENT;

            case HTML_O_SDONMOUSEOVER:
                eScriptType = STARBASIC;
            case HTML_O_ONMOUSEOVER:
                nEvent = SFX_EVENT_MOUSEOVER_OBJECT;
                goto ANCHOR_SETEVENT;

            case HTML_O_SDONMOUSEOUT:
                eScriptType = STARBASIC;
            case HTML_O_ONMOUSEOUT:
                nEvent = SFX_EVENT_MOUSEOUT_OBJECT;
                goto ANCHOR_SETEVENT;
ANCHOR_SETEVENT:
                {
                    String sTmp( pOption->GetString() );
                    if( sTmp.Len() )
                    {
                        sTmp.ConvertLineEnd();
                        String sScriptType;
                        if( EXTENDED_STYPE == eScriptType )
                            sScriptType = sDfltScriptType;
                        aMacroTbl.Insert( nEvent,
                            new SvxMacro( sTmp, sScriptType, eScriptType ));
                    }
                }
                break;

        }
    }

    // Sprungziele, die unseren ipmliziten Zielen entsprechen, schmeissen
    // wir hier ganz rigoros raus.
    if( aName.Len() )
    {
        xub_StrLen nLastPos, nPos = aName.Search( cMarkSeperator );
        if( STRING_NOTFOUND != nPos )
        {
            while( STRING_NOTFOUND !=
                        ( nLastPos = aName.Search( cMarkSeperator, nPos + 1 )) )
                nPos = nLastPos;

            String sCmp( aName.Copy( nPos+1 ) );
            sCmp.EraseAllChars();
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

    BOOL bEnAnchor = FALSE, bFtnAnchor = FALSE, bFtnEnSymbol = FALSE;
    String aFtnName;
    if( aClass.Len() >=9  && bHasHRef && sHRef.Len() > 1 &&
        ('s' == aClass.GetChar(0) || 'S' == aClass.GetChar(0)) &&
        ('d' == aClass.GetChar(1) || 'D' == aClass.GetChar(1)) )
    {
        if( aClass.EqualsIgnoreCaseAscii( sHTML_sdendnote_anc ) )
            bEnAnchor = TRUE;
        else if( aClass.EqualsIgnoreCaseAscii( sHTML_sdfootnote_anc ) )
            bFtnAnchor = TRUE;
        else if( aClass.EqualsIgnoreCaseAscii( sHTML_sdendnote_sym ) ||
                 aClass.EqualsIgnoreCaseAscii( sHTML_sdfootnote_sym ) )
            bFtnEnSymbol = TRUE;
        if( bEnAnchor || bFtnAnchor || bFtnEnSymbol )
        {
            aFtnName = sHRef.Copy( 1 );
            aClass = aName = aEmptyStr;
            bHasHRef = FALSE;
        }
    }

    // Styles parsen
    if( HasStyleOptions( aStyle, aId, aClass ) )
    {
        SfxItemSet aItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        if( ParseStyleOptions( aStyle, aId, aClass, aItemSet, aPropInfo ) )
        {
            DoPositioning( aItemSet, aPropInfo, pCntxt );
            InsertAttrs( aItemSet, aPropInfo, pCntxt, TRUE );
        }
    }

    if( bHasHRef )
    {
        if( sHRef.Len() )
        {
            ASSERT( INetURLObject::GetBaseURL() == sBaseURL,
                    "<A>: Base URL ist zerschossen" );
            sHRef = INetURLObject::RelToAbs( sHRef );
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

        if( aMacroTbl.Count() )
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
        bInFootEndNoteAnchor = bCallNextToken = TRUE;
    }
    else if( bFtnEnSymbol )
    {
        bInFootEndNoteSymbol = bCallNextToken = TRUE;
    }

    // den Kontext merken
    PushContext( pCntxt );
}

void SwHTMLParser::EndAnchor()
{
    if( bInFootEndNoteAnchor )
    {
        FinishFootEndNote();
        bInFootEndNoteAnchor = FALSE;
    }
    else if( bInFootEndNoteSymbol )
    {
        bInFootEndNoteSymbol = FALSE;
    }

    EndTag( HTML_ANCHOR_OFF );
}

/*  */

void SwHTMLParser::InsertBookmark( const String& rName )
{
    _HTMLAttr* pTmp = new _HTMLAttr( *pPam->GetPoint(),
            SfxStringItem( RES_FLTR_BOOKMARK, rName ));
    aSetAttrTab.Insert( pTmp, aSetAttrTab.Count() );
}

BOOL SwHTMLParser::HasCurrentParaBookmarks( BOOL bIgnoreStack ) const
{
    BOOL bHasMarks = FALSE;
    ULONG nNodeIdx = pPam->GetPoint()->nNode.GetIndex();

    // 1. Schritt: befinden sich noch Bookmarks m Attribut-Stack?
    // Bookmarks werden hinten in den Stack geschrieben. Wir muessen
    // also nur die letzte Bookmark betrachten
    if( !bIgnoreStack )
    {
        _HTMLAttr* pAttr;
        for( USHORT i = aSetAttrTab.Count(); i; )
        {
            pAttr = aSetAttrTab[ --i ];
            if( RES_FLTR_BOOKMARK == pAttr->pItem->Which() )
            {
                if( pAttr->GetSttParaIdx() == nNodeIdx )
                    bHasMarks = TRUE;
                break;
            }
        }
    }

    if( !bHasMarks )
    {
        // 2. Schritt: Wenn wir keine Bookmark gefunden haben, schauen wir,
        // ob schon eine gesetzt ist
        const SwBookmarks& rBookmarks = pDoc->GetBookmarks();
        for( USHORT i=0; i<rBookmarks.Count(); i++ )
        {
            const SwBookmark* pBookmark = rBookmarks[i];
            ULONG nBookNdIdx = pBookmark->GetPos().nNode.GetIndex();
            if( nBookNdIdx==nNodeIdx )
            {
                bHasMarks = TRUE;
                break;
            }
            else if( nBookNdIdx > nNodeIdx )
                break;
        }
    }

    return bHasMarks;
}

/*  */

void SwHTMLParser::StripTrailingPara()
{
    BOOL bSetSmallFont = FALSE;

    SwCntntNode* pCNd = pPam->GetCntntNode();
    if( !pPam->GetPoint()->nContent.GetIndex() )
    {
        if( pCNd && pCNd->StartOfSectionIndex()+2 <
            pCNd->EndOfSectionIndex() )
        {
            ULONG nNodeIdx = pPam->GetPoint()->nNode.GetIndex();

            USHORT i;

            const SwFrmFmt* pFmt;
            const SwFmtAnchor* pAnchor;
            const SwPosition* pAPos;
            const SwSpzFrmFmts& rFrmFmtTbl = *pDoc->GetSpzFrmFmts();

            for( i=0; i<rFrmFmtTbl.Count(); i++ )
            {
                pFmt = rFrmFmtTbl[i];
                pAnchor = &pFmt->GetAnchor();
                if( 0 != ( pAPos = pAnchor->GetCntntAnchor()) &&
                    (FLY_AT_CNTNT == pAnchor->GetAnchorId() ||
                     FLY_AUTO_CNTNT == pAnchor->GetAnchorId()) &&
                    pAPos->nNode == nNodeIdx )

                    return;     // den Knoten duerfen wir nicht loeschen
            }

            SetAttr( FALSE );   // die noch offenen Attribute muessen
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
                    pCNd->GetTxtNode()->Cut( pPrvNd, aSrc, pCNd->Len() );
                }
            }

            // jetz muessen wir noch eventuell vorhandene Bookmarks
            // verschieben
            const SwBookmarks& rBookmarks = pDoc->GetBookmarks();
            for( i=0; i<rBookmarks.Count(); i++ )
            {
                const SwBookmark* pBookmark = rBookmarks[i];
                ULONG nBookNdIdx = pBookmark->GetPos().nNode.GetIndex();
                if( nBookNdIdx==nNodeIdx )
                {
                    SwPosition &rBookmkPos =
                        (SwPosition&)pBookmark->GetPos();

                    SwNodeIndex nNewNdIdx( pPam->GetPoint()->nNode );
                    SwCntntNode* pCNd = pDoc->GetNodes().GoPrevious( &nNewNdIdx );
                    if( !pCNd )
                    {
                        ASSERT( !this, "Hoppla, wo ist mein Vorgaenger-Node" );
                        return;
                    }

                    rBookmkPos.nNode = nNewNdIdx;
                    rBookmkPos.nContent.Assign( pCNd, pCNd->Len() );
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
            bSetSmallFont = TRUE;
        }
    }
    else if( pCNd && pCNd->IsTxtNode() && pTable &&
             pCNd->StartOfSectionIndex()+2 ==
             pCNd->EndOfSectionIndex() )
    {
        // Wenn die Zelle nur zeichengebundene Grafiken/Rahmen enthaelt
        // stellen wir ebenfalls einen kleinen Font ein.
        bSetSmallFont = TRUE;
        SwTxtNode* pTxtNd = pCNd->GetTxtNode();

        xub_StrLen nPos = pPam->GetPoint()->nContent.GetIndex();
        while( bSetSmallFont && nPos>0 )
        {
            bSetSmallFont = CH_TXTATR_BREAKWORD ==
                                        pTxtNd->GetTxt().GetChar( --nPos ) &&
                        0 != pTxtNd->GetTxtAttr( nPos, RES_TXTATR_FLYCNT );
        }
    }

    if( bSetSmallFont )
    {
        pCNd->SetAttr( SvxFontHeightItem(40) );
    }
}

/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/html/htmlgrin.cxx,v 1.1.1.1 2000-09-18 17:14:55 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.130  2000/09/18 16:04:45  willem.vandorp
      OpenOffice header added.

      Revision 1.129  2000/08/31 10:59:55  jp
      add missing include

      Revision 1.128  2000/07/20 13:16:10  jp
      change old txtatr-character to the two new characters

      Revision 1.127  2000/06/26 09:52:14  jp
      must change: GetAppWindow->GetDefaultDevice

      Revision 1.126  2000/04/10 12:20:56  mib
      unicode

      Revision 1.125  2000/03/03 15:21:01  os
      StarView remainders removed

      Revision 1.124  2000/03/03 12:44:32  mib
      Removed JavaScript

      Revision 1.123  2000/02/11 14:37:12  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.122  1999/12/03 08:40:07  jp
      Task #70407#: use new GrfObj instead of Grafik with Cache (first only with define)

      Revision 1.121  1999/11/19 16:40:20  os
      modules renamed

      Revision 1.120  1999/09/17 12:13:28  mib
      support of multiple and non system text encodings

      Revision 1.119  1999/06/10 08:34:12  JP
      have to change: no AppWin from SfxApp


      Rev 1.118   10 Jun 1999 10:34:12   JP
   have to change: no AppWin from SfxApp

*************************************************************************/

