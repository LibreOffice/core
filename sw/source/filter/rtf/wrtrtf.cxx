/*************************************************************************
 *
 *  $RCSfile: wrtrtf.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: jp $ $Date: 2001-02-13 09:24:35 $
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
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
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

#include <stdlib.h>

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _DATETIME_HXX //autogen
#include <tools/datetime.hxx>
#endif
#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif
#ifndef _RTFKEYWD_HXX //autogen
#include <svtools/rtfkeywd.hxx>
#endif
#ifndef _RTFOUT_HXX
#include <svtools/rtfout.hxx>
#endif
#ifndef _SFXDOCINF_HXX //autogen
#include <sfx2/docinf.hxx>
#endif
#ifndef _SVX_PAPERINF_HXX //autogen
#include <svx/paperinf.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_SHADITEM_HXX //autogen
#include <svx/shaditem.hxx>
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
#ifndef _SVX_TSTPITEM_HXX //autogen
#include <svx/tstpitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_BRKITEM_HXX //autogen
#include <svx/brkitem.hxx>
#endif


#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTHDFT_HXX //autogen
#include <fmthdft.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _WRTRTF_HXX
#include <wrtrtf.hxx>
#endif
#ifndef _FMTCOL_HXX
#include <fmtcol.hxx>
#endif
#ifndef _FLYPOS_HXX
#include <flypos.hxx>
#endif
#ifndef _BOOKMRK_HXX
#include <bookmrk.hxx>      // fuer SwBookmark ...
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>     // fuer SwPageDesc...
#endif
#ifndef _NUMRULE_HXX
#include <numrule.hxx>
#endif
#ifndef _FTNINFO_HXX
#include <ftninfo.hxx>
#endif
#ifndef _CHARFMT_HXX
#include <charfmt.hxx>
#endif
#ifndef _SECTION_HXX //autogen
#include <section.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>      // fuer SwPageDesc ...
#endif

#ifndef _SWSWERROR_H
#include <swerror.h>
#endif
#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>       // ...Percent()
#endif
#ifndef _STATSTR_HRC
#include <statstr.hrc>      // ResId fuer Statusleiste
#endif

#if defined(MAC)
const sal_Char SwRTFWriter::sNewLine = '\015';
#elif defined(UNX)
const sal_Char SwRTFWriter::sNewLine = '\012';
#else
const sal_Char __FAR_DATA SwRTFWriter::sNewLine[] = "\015\012";
#endif



SV_DECL_VARARR( RTFColorTbl, Color, 5, 8 )
SV_IMPL_VARARR( RTFColorTbl, Color )


SwRTFWriter::SwRTFWriter( const String& rFltName )
{
    // schreibe Win-RTF-HelpFileFmt
    bWriteHelpFmt = 'W' == rFltName.GetChar( 0 );
    // schreibe nur Gliederungs Absaetze
    bOutOutlineOnly = 'O' == rFltName.GetChar( 0 );
}


SwRTFWriter::~SwRTFWriter() {}



ULONG SwRTFWriter::WriteStream()
{
    bOutPageAttr = bOutSection = TRUE;

    bOutStyleTab = bOutTable = bOutPageDesc = bOutPageDescTbl =
    bAutoAttrSet = bOutListNumTxt = bOutLeftHeadFoot = bIgnoreNextPgBreak =
        bTxtAttr = FALSE;

    pCurEndPosLst = 0;
    nBkmkTabPos = USHRT_MAX;
    pAktPageDesc = 0;
    pAttrSet = 0;
    pFlyFmt = 0;        // kein FlyFrmFormat gesetzt

    pColTbl = new RTFColorTbl;

    BYTE nSz = (BYTE)Min( pDoc->GetSpzFrmFmts()->Count(), USHORT(255) );
    SwPosFlyFrms aFlyPos( nSz, nSz );

    //Abkuerzung wenn nur Gliederung ausgegeben werden soll, und keine
    //Gliederungsabsaetze ausserhalb des Body stehen.
    if ( bOutOutlineOnly &&
         pDoc->GetNodes().GetOutLineNds().Count() &&
         pDoc->GetNodes().GetOutLineNds()[0]->GetIndex() >
         pDoc->GetNodes().GetEndOfExtras().GetIndex() )
    {
        nAktFlyPos = 0;
        pFlyPos = 0;
        MakeHeader();

        const SwOutlineNodes &rOutLine = pDoc->GetNodes().GetOutLineNds();
        for ( USHORT i = 0; i < rOutLine.Count(); ++i )
        {
            const SwNode *pNd = pDoc->GetNodes()[*rOutLine[i]];
            ASSERT( pNd->IsCntntNode(), "Outlinenode and no CntNode!?" );

            SwCntntNode* pCNd = (SwCntntNode*)pNd;

            // erfrage den aktuellen PageDescriptor.
            const SwPageDesc* pTmp = pCNd->GetSwAttrSet().GetPageDesc().GetPageDesc();
            if( pTmp )
                pAktPageDesc = pTmp;
            pCurPam->GetPoint()->nContent.Assign( pCNd, 0 );
            Out( aRTFNodeFnTab, *pCNd, *this );
        }
    }
    else
    {
        long nMaxNode = pDoc->GetNodes().Count();

        if( bShowProgress )
            ::StartProgress( STR_STATSTR_W4WWRITE, 0, nMaxNode, pDoc->GetDocShell() );

        // Tabelle am Doc.-Anfang beachten
        {
            SwTableNode * pTNd = pCurPam->GetNode()->FindTableNode();
            if( pTNd && bWriteAll )
            {
                // mit dem TabellenNode anfangen !!
                pCurPam->GetPoint()->nNode = *pTNd;

                if( bWriteOnlyFirstTable )
                    pCurPam->GetMark()->nNode = *pTNd->EndOfSectionNode();
            }
        }

        // Tabelle fuer die freifliegenden Rahmen erzeugen, aber nur wenn
        // das gesamte Dokument geschrieben wird
        nAktFlyPos = 0;
        pDoc->GetAllFlyFmts( aFlyPos, bWriteAll ? 0 : pOrigPam );

        // sollten nur Rahmen vorhanden sein?
        // (Moeglich, wenn eine Rahmen-Selektion ins Clipboard
        // gestellt wurde)
        if( bWriteAll &&
            // keine Laenge
            *pCurPam->GetPoint() == *pCurPam->GetMark() &&
            // Rahmen vorhanden
            pDoc->GetSpzFrmFmts()->Count() && !aFlyPos.Count() &&
            // nur ein Node im Array
            pDoc->GetNodes().GetEndOfExtras().GetIndex() + 3 ==
            pDoc->GetNodes().GetEndOfContent().GetIndex() &&
            // und genau der ist selektiert
            pDoc->GetNodes().GetEndOfContent().GetIndex() - 1 ==
            pCurPam->GetPoint()->nNode.GetIndex() )
        {
            // dann den Inhalt vom Rahmen ausgeben.
            // dieser steht immer an Position 0 !!
            SwFrmFmt* pFmt = (*pDoc->GetSpzFrmFmts())[ 0 ];
            const SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
            if( pIdx )
            {
                SwPosFlyFrm* pFPos = new SwPosFlyFrm( pCurPam->GetPoint()->nNode,
                                                        pFmt, aFlyPos.Count() );
                aFlyPos.Insert( pFPos );
            }
        }

        pFlyPos = &aFlyPos;

        // schreibe die StyleTabelle, allgemeine Angaben,Header/Footer/Footnotes
        MakeHeader();

        Out_SwDoc( pOrigPam );

        if( bShowProgress )
            ::EndProgress( pDoc->GetDocShell() );
    }

    Strm() << '}';

    // loesche die Tabelle mit den freifliegenden Rahmen
    for( USHORT i = aFlyPos.Count(); i > 0; )
        delete aFlyPos[ --i ];

    pFlyPos = 0;
    delete pColTbl;

    // schreibe Win-RTF-HelpFileFmt
    bOutOutlineOnly = bWriteHelpFmt = FALSE;
    pAttrSet = 0;

    return 0;
}


void SwRTFWriter::Out_SwDoc( SwPaM* pPam )
{
    BOOL bSaveWriteAll = bWriteAll;     // sichern
    // suche die naechste Bookmark-Position aus der Bookmark-Tabelle
    nBkmkTabPos = bWriteAll ? FindPos_Bkmk( *pCurPam->GetPoint() ) : USHRT_MAX;

    // gebe alle Bereiche des Pams in das RTF-File aus.
    do {
        bWriteAll = bSaveWriteAll;
        bFirstLine = TRUE;

        // suche den ersten am Pam-auszugebenen FlyFrame
        // fehlt noch:

        while( pCurPam->GetPoint()->nNode < pCurPam->GetMark()->nNode ||
              (pCurPam->GetPoint()->nNode == pCurPam->GetMark()->nNode &&
               pCurPam->GetPoint()->nContent.GetIndex() <= pCurPam->GetMark()->nContent.GetIndex()) )
        {
            SwNode& rNd = pCurPam->GetPoint()->nNode.GetNode();

            if( &rNd == &pDoc->GetNodes().GetEndOfContent() )
                break;

            if( rNd.IsCntntNode() )
            {
                SwCntntNode& rCNd = (SwCntntNode&)rNd;

                OutBreaks( rCNd.GetSwAttrSet() );
                OutFlyFrm();

                if( !bFirstLine )
                    pCurPam->GetPoint()->nContent.Assign( &rCNd, 0 );

                if( !bOutOutlineOnly ||
                    ( rCNd.IsTxtNode() && NO_NUMBERING !=
                        ((SwTxtNode&)rCNd).GetTxtColl()->GetOutlineLevel() ))
                    Out( aRTFNodeFnTab, rCNd, *this );

            }
            else if( !bOutOutlineOnly )
            {
                if( rNd.IsTableNode() )
                {
                    OutBreaks( ((SwTableNode&)rNd).GetTable().GetFrmFmt()
                                ->GetAttrSet() );
                    OutRTF_SwTblNode( *this, (SwTableNode&)rNd );
                }
                else if( rNd.IsSectionNode() )
                {
                    OutBreaks( ((SwSectionNode&)rNd).GetSection().GetFmt()
                                ->GetAttrSet() );
                    OutRTF_SwSectionNode( *this, (SwSectionNode&)rNd );
                }
                else if( rNd.IsEndNode() )
                    CheckEndNodeForSection( rNd );
            }

            ULONG nPos = pCurPam->GetPoint()->nNode++;  // Bewegen

            if( bShowProgress )
                ::SetProgressState( nPos, pDoc->GetDocShell() );   // Wie weit ?

            /* sollen nur die Selectierten Bereiche gesichert werden, so
             * duerfen nur die vollstaendigen Nodes gespeichert werde,
             * d.H. der 1. und n. Node teilweise, der 2. bis n-1. Node
             * vollstaendig. (vollstaendig heisst mit allen Formaten! )
             */
            bWriteAll = bSaveWriteAll ||
                    pCurPam->GetPoint()->nNode != pCurPam->GetMark()->nNode;
            bFirstLine = FALSE;
        }
    } while( CopyNextPam( &pPam ) );        // bis alle PaM's bearbeitet

    bWriteAll = bSaveWriteAll;          // wieder auf alten Wert zurueck
}


// schreibe die StyleTabelle, algemeine Angaben,Header/Footer/Footnotes


void SwRTFWriter::MakeHeader()
{
    // baue den Vorspann wie Header, ColorTbl, FontTbl
    Strm() << '{' << sRTF_RTF << '1'
#ifdef MAC
        << sRTF_MAC
#else
        << sRTF_ANSI
#endif
        ;
    if( bWriteAll )
    {
        Strm() << sRTF_DEFF;
        OutULong( GetId( (SvxFontItem&)pDoc->GetAttrPool().GetDefaultItem(
                        RES_CHRATR_FONT ) ));
    }


    OutRTFFontTab();
    OutRTFColorTab();
    OutRTFStyleTab();
    OutRTFListTab();

    Strm() << SwRTFWriter::sNewLine;        // ein Trenner

    // wenn teilweise ausgegeben wird, die globalen Daten nicht speichern
    if( !bWriteAll )
        return;

    // Ausgeben der Doc-Info/-Statistik
    OutDocInfoStat();

    // einige globale Daten Schreiben
    {   // Default-TabSize
        const SvxTabStopItem& rTabs = (const SvxTabStopItem&)
                    pDoc->GetAttrPool().GetDefaultItem( RES_PARATR_TABSTOP );
        Strm() << sRTF_DEFTAB;
        OutLong( rTabs[0].GetTabPos() );
    }

    // PageDescriptor ausgeben (??nur wenn das gesamte Dokument??)
    OutPageDesc();

    // gebe die Groesse und die Raender der Seite aus
    if( pDoc->GetPageDescCnt() )
    {
        //JP 06.04.99: Bug 64361 - suche den ersten SwFmtPageDesc. Ist
        //              keiner gesetzt, so ist der Standard gueltig
        const SwFmtPageDesc* pSttPgDsc = 0;
        {
            const SwNode& rSttNd = *pDoc->GetNodes()[
                        pDoc->GetNodes().GetEndOfExtras().GetIndex() + 2 ];
            const SfxItemSet* pSet = 0;

            if( rSttNd.IsCntntNode() )
                pSet = &rSttNd.GetCntntNode()->GetSwAttrSet();
            else if( rSttNd.IsTableNode() )
                pSet = &rSttNd.GetTableNode()->GetTable().
                            GetFrmFmt()->GetAttrSet();
            else if( rSttNd.IsSectionNode() )
                pSet = &rSttNd.GetSectionNode()->GetSection().
                            GetFmt()->GetAttrSet();

            if( pSet )
            {
                USHORT nPosInDoc;
                pSttPgDsc = (SwFmtPageDesc*)&pSet->Get( RES_PAGEDESC );
                if( !pSttPgDsc->GetPageDesc() )
                    pSttPgDsc = 0;
                else if( pDoc->FindPageDescByName( pSttPgDsc->
                                    GetPageDesc()->GetName(), &nPosInDoc ))
                {
                                                // FALSE wegen schliessender Klammer !!
                    OutComment( *this, sRTF_PGDSCNO, FALSE );
                    OutULong( nPosInDoc ) << '}';
                }
            }
        }
        const SwPageDesc& rPageDesc = pSttPgDsc ? *pSttPgDsc->GetPageDesc()
                                                : pDoc->GetPageDesc( 0 );
        const SwFrmFmt &rFmtPage = rPageDesc.GetMaster();

        {
            if( rPageDesc.GetLandscape() )
                Strm() << sRTF_LANDSCAPE;

            const SwFmtFrmSize& rSz = rFmtPage.GetFrmSize();
            // Clipboard-Dokument wird immer ohne Drucker angelegt, so ist
            // der Std.PageDesc immer aug LONG_MAX !! Mappe dann auf DIN A4
            if( LONG_MAX == rSz.GetHeight() || LONG_MAX == rSz.GetWidth() )
            {
                Strm() << sRTF_PAPERH;
                OutULong( lA4Height ) << sRTF_PAPERW;
                OutULong( lA4Width );
            }
            else
            {
                Strm() << sRTF_PAPERH;
                OutULong( rSz.GetHeight() ) << sRTF_PAPERW;
                OutULong( rSz.GetWidth() );
            }
        }

        {
            const SvxLRSpaceItem& rLR = rFmtPage.GetLRSpace();
            Strm() << sRTF_MARGL;
            OutLong( rLR.GetLeft() ) << sRTF_MARGR;
            OutLong( rLR.GetRight() );
        }

        {
            const SvxULSpaceItem& rUL = rFmtPage.GetULSpace();
            Strm() << sRTF_MARGT;
            OutLong( rUL.GetUpper() ) << sRTF_MARGB;
            OutLong( rUL.GetLower() );
        }

        Strm() << sRTF_SECTD << sRTF_SBKNONE;
        OutRTFPageDescription( rPageDesc, FALSE, FALSE );
        if( pSttPgDsc )
        {
            bIgnoreNextPgBreak = TRUE;
            pAktPageDesc = &rPageDesc;
        }
    }


    {
        // schreibe die Fussnoten- und Endnoten-Info raus
        const SwFtnInfo& rFtnInfo = pDoc->GetFtnInfo();

        const char* pOut = FTNPOS_CHAPTER == rFtnInfo.ePos
                            ? sRTF_ENDDOC
                            : sRTF_FTNBJ;
        Strm() << pOut << sRTF_FTNSTART;
        OutLong( rFtnInfo.nFtnOffset + 1 );

        switch( rFtnInfo.eNum )
        {
        case FTNNUM_PAGE:       pOut = sRTF_FTNRSTPG;   break;
        case FTNNUM_DOC:        pOut = sRTF_FTNRSTCONT; break;
//      case FTNNUM_CHAPTER:
        default:                pOut = sRTF_FTNRESTART; break;
        }
        Strm() << pOut;

        switch( rFtnInfo.aFmt.eType )
        {
        case SVX_NUM_CHARS_LOWER_LETTER:
        case SVX_NUM_CHARS_LOWER_LETTER_N:  pOut = sRTF_FTNNALC;    break;
        case SVX_NUM_CHARS_UPPER_LETTER:
        case SVX_NUM_CHARS_UPPER_LETTER_N:  pOut = sRTF_FTNNAUC;    break;
        case SVX_NUM_ROMAN_LOWER:           pOut = sRTF_FTNNRLC;    break;
        case SVX_NUM_ROMAN_UPPER:           pOut = sRTF_FTNNRUC;    break;
        case SVX_NUM_CHAR_SPECIAL:          pOut = sRTF_FTNNCHI;    break;
//      case SVX_NUM_ARABIC:
        default:                    pOut = sRTF_FTNNAR;     break;
        }
        Strm() << pOut;


        const SwEndNoteInfo& rEndNoteInfo = pDoc->GetEndNoteInfo();

        Strm() << sRTF_AENDDOC << sRTF_AFTNRSTCONT
               << sRTF_AFTNSTART;
        OutLong( rEndNoteInfo.nFtnOffset + 1 );

        switch( rEndNoteInfo.aFmt.eType )
        {
        case SVX_NUM_CHARS_LOWER_LETTER:
        case SVX_NUM_CHARS_LOWER_LETTER_N:  pOut = sRTF_AFTNNALC;   break;
        case SVX_NUM_CHARS_UPPER_LETTER:
        case SVX_NUM_CHARS_UPPER_LETTER_N:  pOut = sRTF_AFTNNAUC;   break;
        case SVX_NUM_ROMAN_LOWER:           pOut = sRTF_AFTNNRLC;   break;
        case SVX_NUM_ROMAN_UPPER:           pOut = sRTF_AFTNNRUC;   break;
        case SVX_NUM_CHAR_SPECIAL:          pOut = sRTF_AFTNNCHI;   break;
//      case SVX_NUM_ARABIC:
        default:                    pOut = sRTF_AFTNNAR;    break;
        }
        Strm() << pOut;
    }

    if( pDoc->_GetDBDesc().Len() )
    {
        // stelle erstmal fest, ob ueberhaupt Datenbankfelder benutzt werden!
        const SwFldTypes* pTypes = pDoc->GetFldTypes();
        for( USHORT nCnt = pTypes->Count(); nCnt >= INIT_FLDTYPES; )
            if( RES_DBFLD == (*pTypes)[ --nCnt ]->Which() &&
                (*pTypes)[ nCnt ]->GetDepends() )
            {
                Strm() << '{' << sRTF_FIELD;
                OutComment( *this, sRTF_FLDINST ) << " DATA ";
                RTFOutFuncs::Out_String( Strm(), pDoc->GetDBName(),
                                        DEF_ENCODING, bWriteHelpFmt );
                Strm() << "}{" << sRTF_FLDRSLT << " }}";
                break;
            }
    }

    pAttrSet = 0;

    Strm() << SwRTFWriter::sNewLine;        // ein Trenner
}

void SwRTFWriter::OutInfoDateTime( const DateTime& rDT, const char* pStr )
{
    Strm() << '{' << pStr << sRTF_YR;
    OutLong( Strm(), rDT.GetYear() ) << sRTF_MO;
    OutLong( Strm(), rDT.GetMonth() ) << sRTF_DY;
    OutLong( Strm(), rDT.GetDay() ) << sRTF_HR;
    OutLong( Strm(), rDT.GetHour() ) << sRTF_MIN;
    OutLong( Strm(), rDT.GetMin() ) << '}';
}

void SwRTFWriter::OutDocInfoStat()
{
    Strm() << '{' << sRTF_INFO;

    const SfxDocumentInfo* pInfo = pDoc->GetpInfo();
    if( pInfo )
    {
        const String* pStr = &pInfo->GetTitle();
        if( pStr->Len() )
        {
            Strm() << '{' << sRTF_TITLE << ' ';
            RTFOutFuncs::Out_String( Strm(), *pStr, DEF_ENCODING,
                                        bWriteHelpFmt ) << '}';
        }
        if( (pStr = &pInfo->GetTheme())->Len() )
        {
            Strm() << '{' << sRTF_SUBJECT << ' ';
            RTFOutFuncs::Out_String( Strm(), *pStr, DEF_ENCODING,
                                        bWriteHelpFmt ) << '}';
        }
        if( (pStr = &pInfo->GetKeywords())->Len() )
        {
            Strm() << '{' << sRTF_KEYWORDS << ' ';
            RTFOutFuncs::Out_String( Strm(), *pStr, DEF_ENCODING,
                                        bWriteHelpFmt ) << '}';
        }
        if( (pStr = &pInfo->GetComment())->Len() )
        {
            Strm() << '{' << sRTF_DOCCOMM << ' ';
            RTFOutFuncs::Out_String( Strm(), *pStr, DEF_ENCODING,
                                        bWriteHelpFmt ) << '}';
        }

        const SfxStamp* pStamp = &pInfo->GetCreated();
        if( ( pStr = &pStamp->GetName())->Len() )
        {
            Strm() << '{' << sRTF_AUTHOR << ' ';
            RTFOutFuncs::Out_String( Strm(), *pStr, DEF_ENCODING,
                                        bWriteHelpFmt ) << '}';
        }
        OutInfoDateTime( pStamp->GetTime(), sRTF_CREATIM );

        pStamp = &pInfo->GetChanged();
        if( ( pStr = &pStamp->GetName())->Len() )
        {
            Strm() << '{' << sRTF_OPERATOR << ' ';
            RTFOutFuncs::Out_String( Strm(), *pStr, DEF_ENCODING,
                                        bWriteHelpFmt ) << '}';
        }
        OutInfoDateTime( pStamp->GetTime(), sRTF_REVTIM );

        OutInfoDateTime( pInfo->GetPrinted().GetTime(), sRTF_PRINTIM );
    }


    // fuer interne Zwecke - Versionsnummer rausschreiben
    Strm() << '{' << sRTF_COMMENT << " StarWriter}{" << sRTF_VERN;
    OutLong( Strm(), SUPD*10 ) << '}';

    Strm() << '}';
}


static void InsColor( RTFColorTbl& rTbl, const Color& rCol )
{
    for( USHORT n = 0; n < rTbl.Count(); ++n )
        if( rTbl[n] == rCol )
            return;         // schon vorhanden, zurueck

    rTbl.Insert( rCol, rTbl.Count() );
}

static void InsColorLine( RTFColorTbl& rTbl, const SvxBoxItem& rBox )
{
    const SvxBorderLine* pLine = 0;

    if( rBox.GetTop() )
        InsColor( rTbl, (pLine = rBox.GetTop())->GetColor() );
    if( rBox.GetBottom() && pLine != rBox.GetBottom() )
        InsColor( rTbl, (pLine = rBox.GetBottom())->GetColor() );
    if( rBox.GetLeft() && pLine != rBox.GetLeft()  )
        InsColor( rTbl, (pLine = rBox.GetLeft())->GetColor() );
    if( rBox.GetRight() && pLine != rBox.GetRight()  )
        InsColor( rTbl, rBox.GetRight()->GetColor() );
}


void SwRTFWriter::OutRTFColorTab()
{
    ASSERT( pColTbl, "Wo ist meine Color-Tabelle?" );

    // dann baue die ColorTabelle aus allen Attributen, die Colors
    // enthalten und im Pool angemeldet sind auf.
    USHORT n, nMaxItem;
    const SfxItemPool& rPool = pDoc->GetAttrPool();

    // das Charakter - Color Attribut
    {
        const SvxColorItem* pCol = (const SvxColorItem*)GetDfltAttr(
                                                RES_CHRATR_COLOR );
        InsColor( *pColTbl, pCol->GetValue() );
        if( 0 != ( pCol = (const SvxColorItem*)rPool.GetPoolDefaultItem(
                RES_CHRATR_COLOR ) ))
            InsColor( *pColTbl, pCol->GetValue() );
        nMaxItem = rPool.GetItemCount(RES_CHRATR_COLOR);
        for( n = 0; n < nMaxItem; ++n )
            if( 0 != (pCol = (const SvxColorItem*)rPool.GetItem(
                RES_CHRATR_COLOR, n ) ) )
                InsColor( *pColTbl, pCol->GetValue() );
    }

    // das Frame Hintergrund - Attribut
    {
        const SvxBrushItem* pBkgrd = (const SvxBrushItem*)GetDfltAttr(
                                                            RES_BACKGROUND );
        InsColor( *pColTbl, pBkgrd->GetColor() );
//      InsColor( *pColTbl, pBkgrd->GetBrush().GetFillColor() );
        if( 0 != ( pBkgrd = (const SvxBrushItem*)rPool.GetPoolDefaultItem(
                        RES_BACKGROUND ) ))
        {
            InsColor( *pColTbl, pBkgrd->GetColor() );
//          InsColor( *pColTbl, pBkgrd->GetBrush().GetFillColor() );
        }
        nMaxItem = rPool.GetItemCount(RES_BACKGROUND);
        for( n = 0; n < nMaxItem; ++n )
            if( 0 != (pBkgrd = (const SvxBrushItem*)rPool.GetItem(
                RES_BACKGROUND, n ) ))
            {
                InsColor( *pColTbl, pBkgrd->GetColor() );
//              InsColor( *pColTbl, pBkgrd->GetBrush().GetFillColor() );
            }
    }

    // das Frame Schatten - Attribut
    {
        const SvxShadowItem* pShadow = (const SvxShadowItem*)GetDfltAttr(
                                                            RES_SHADOW );
        InsColor( *pColTbl, pShadow->GetColor() );
//      InsColor( *pColTbl, pShadow->GetBrush().GetFillColor() );
        if( 0 != ( pShadow = (const SvxShadowItem*)rPool.GetPoolDefaultItem(
                        RES_SHADOW ) ))
        {
            InsColor( *pColTbl, pShadow->GetColor() );
//          InsColor( *pColTbl, pShadow->GetBrush().GetFillColor() );
        }
        nMaxItem = rPool.GetItemCount(RES_SHADOW);
        for( n = 0; n < nMaxItem; ++n )
            if( 0 != (pShadow = (const SvxShadowItem*)rPool.GetItem(
                RES_SHADOW, n ) ) )
            {
                InsColor( *pColTbl, pShadow->GetColor() );
//              InsColor( *pColTbl, pShadow->GetBrush().GetFillColor() );
            }
    }

    // das Frame Umrandungs - Attribut
    {
        // Box muesste noch gemacht werden, aber default nie eine Line gesetzt!
        const SvxBoxItem* pBox;
        if( 0 != ( pBox = (const SvxBoxItem*)rPool.GetPoolDefaultItem(
                        RES_BOX ) ))
            InsColorLine( *pColTbl, *pBox );
        nMaxItem = rPool.GetItemCount(RES_BOX);
        for( n = 0; n < nMaxItem; ++n )
            if( 0 != (pBox = (const SvxBoxItem*)rPool.GetItem( RES_BOX, n ) ))
                InsColorLine( *pColTbl, *pBox );
    }

    // und raus damit
    Strm() << SwRTFWriter::sNewLine << '{' << sRTF_COLORTBL;

    for( n = 0; n < pColTbl->Count(); n++ )
    {
        const Color& rCol = (*pColTbl)[ n ];
        Strm() << sRTF_RED;
        OutULong( rCol.GetRed() ) << sRTF_GREEN;
        OutULong( rCol.GetGreen() ) << sRTF_BLUE;
        OutULong( rCol.GetBlue() ) << ';';
    }
    Strm() << '}';
}


static void _OutFont( SwRTFWriter& rWrt, const SvxFontItem& rFont, USHORT nNo )
{
    rWrt.Strm() << '{' << sRTF_F;

    const char* pStr = sRTF_FNIL;
    switch( rFont.GetFamily() )
    {
    case FAMILY_ROMAN:          pStr = sRTF_FROMAN;     break;
    case FAMILY_SWISS:          pStr = sRTF_FSWISS;     break;
    case FAMILY_MODERN:         pStr = sRTF_FMODERN;    break;
    case FAMILY_SCRIPT:         pStr = sRTF_FSCRIPT;    break;
    case FAMILY_DECORATIVE:     pStr = sRTF_FDECOR;     break;
    }
    rWrt.OutULong( nNo ) << pStr << sRTF_FPRQ;

    USHORT nVal = 0;
    switch( rFont.GetPitch() )
    {
    case PITCH_FIXED:       nVal = 1;       break;
    case PITCH_VARIABLE:    nVal = 2;       break;
    }
    rWrt.OutULong( nVal );

    ULONG nChSet = 0;
    rtl_TextEncoding eChrSet = rFont.GetCharSet();
    if( RTL_TEXTENCODING_DONTKNOW != eChrSet )
        nChSet = rtl_getBestWindowsCharsetFromTextEncoding( eChrSet );
    rWrt.Strm() << sRTF_FCHARSET;
    rWrt.OutULong( nChSet );

    rWrt.Strm() << ' ';
    RTFOutFuncs::Out_String( rWrt.Strm(), rFont.GetFamilyName(), DEF_ENCODING,
                            rWrt.bWriteHelpFmt ) << ";}";
}

void SwRTFWriter::OutRTFFontTab()
{
    USHORT n = 0;
    const SfxItemPool& rPool = pDoc->GetAttrPool();
    const SvxFontItem* pFont = (const SvxFontItem*)GetDfltAttr( RES_CHRATR_FONT );

    Strm() << SwRTFWriter::sNewLine << '{' << sRTF_FONTTBL;
    _OutFont( *this, *pFont, n++ );

    if( 0 != ( pFont = (const SvxFontItem*)rPool.GetPoolDefaultItem(
                                                        RES_CHRATR_FONT )))
        _OutFont( *this, *pFont, n++ );

    PutNumFmtFontsInAttrPool();
    PutCJKandCTLFontsInAttrPool();

    USHORT nMaxItem = rPool.GetItemCount( RES_CHRATR_FONT );
    for( USHORT nGet = 0; nGet < nMaxItem; ++nGet )
        if( 0 != (pFont = (const SvxFontItem*)rPool.GetItem(
            RES_CHRATR_FONT, nGet )) )
        _OutFont( *this, *pFont, n++ );

    Strm() << '}';
}



void SwRTFWriter::OutRTFStyleTab()
{
    // das 0-Style ist das Default, wird nie ausgegeben !!
    USHORT nArrLen = pDoc->GetTxtFmtColls()->Count();
    if( nArrLen <= 1 && pDoc->GetCharFmts()->Count() <= 1 )
        return;

    bOutStyleTab = TRUE;
    Strm() << SwRTFWriter::sNewLine << '{' << sRTF_STYLESHEET;

    // das Default-TextStyle wird nicht mit ausgegeben !!
    for( USHORT n = 1; n < nArrLen; ++n )
    {
        const SwTxtFmtColl* pColl = (*pDoc->GetTxtFmtColls())[ n ];
        pAttrSet = &pColl->GetAttrSet();

        Strm() << '{';
         // gebe Attribute aus
        OutRTF_SwFmt( *this, *pColl );

        if( pColl->DerivedFrom() )
            // suche die Id vom "Parent" Format
            for( USHORT nBasedOn=1; nBasedOn < nArrLen; nBasedOn++)
                if( (*pDoc->GetTxtFmtColls())[ nBasedOn ] ==
                        pColl->DerivedFrom() )
                {
                    // die Ableitung vom Format
                    Strm() << sRTF_SBASEDON;
                    OutULong( nBasedOn );
                    break;
                }

        if( pColl == &pColl->GetNextTxtFmtColl() )
        {
            Strm() << sRTF_SNEXT;
            OutULong( n );
        }
        else
            // suche die Id vom "Naechsten" Format
            for( USHORT nNext=1; nNext < nArrLen; nNext++)
                if( (*pDoc->GetTxtFmtColls())[ nNext ] ==
                        &pColl->GetNextTxtFmtColl() )
                {
                    // die Ableitung vom Format
                    Strm() << sRTF_SNEXT;
                    OutULong( nNext );
                    break;
                }

        if( NO_NUMBERING != pColl->GetOutlineLevel() )
        {
            Strm() << '{' << sRTF_IGNORE << sRTF_SOUTLVL;
            OutULong( pColl->GetOutlineLevel() ) << '}';
        }

        Strm() << ' ';
        RTFOutFuncs::Out_String( Strm(), pColl->GetName(), DEF_ENCODING,
                        bWriteHelpFmt ) << ";}" << SwRTFWriter::sNewLine;
    }

    USHORT nChrArrLen = pDoc->GetCharFmts()->Count();
    for( n = 1; n < nChrArrLen; ++n )
    {
        const SwCharFmt* pFmt = (*pDoc->GetCharFmts())[ n ];
        pAttrSet = &pFmt->GetAttrSet();

        Strm() << '{';
         // gebe Attribute aus
        OutRTF_SwFmt( *this, *pFmt );

        if( pFmt->DerivedFrom() )
            // suche die Id vom "Parent" Format
            for( USHORT nBasedOn=1; nBasedOn < nChrArrLen; nBasedOn++)
                if( (*pDoc->GetCharFmts())[ nBasedOn ] ==
                        pFmt->DerivedFrom() )
                {
                    // die Ableitung vom Format
                    Strm() << sRTF_SBASEDON;
                    OutULong( nArrLen + nBasedOn );
                    break;
                }

        Strm() << ' ';
        RTFOutFuncs::Out_String( Strm(), pFmt->GetName(), DEF_ENCODING,
                    bWriteHelpFmt ) << ";}" << SwRTFWriter::sNewLine;
    }

    Strm() << '}';
    bOutStyleTab = FALSE;
}


void SwRTFWriter::OutRTFFlyFrms( const SwFlyFrmFmt& rFlyFrmFmt )
{
    // ein FlyFrame wurde erkannt, gebe erstmal den aus

    // Hole vom Node und vom letzten Node die Position in der Section
    const SwFmtCntnt& rFlyCntnt = rFlyFrmFmt.GetCntnt();

    ULONG nStt = rFlyCntnt.GetCntntIdx()->GetIndex()+1;
    ULONG nEnd = pDoc->GetNodes()[ nStt - 1 ]->EndOfSectionIndex();

    if( nStt >= nEnd )      // kein Bereich, also kein gueltiger Node
        return;

    Strm() << SwRTFWriter::sNewLine << sRTF_PARD << sRTF_PLAIN;

    {
        RTFSaveData aSaveData( *this, nStt, nEnd );
        pFlyFmt = (SwFlyFrmFmt*)&rFlyFrmFmt;
        Out_SwDoc( pCurPam );
    }

    Strm() << sRTF_PARD << SwRTFWriter::sNewLine;
}


void SwRTFWriter::OutBookmarks( xub_StrLen nCntntPos )
{
    // hole das aktuelle Bookmark
    const SwBookmark* pBookmark = USHRT_MAX != nBkmkTabPos ?
                            pDoc->GetBookmarks()[ nBkmkTabPos ] : 0;
    // Ausgabe von Bookmarks
    while( USHRT_MAX != nBkmkTabPos &&
        pBookmark->GetPos().nNode.GetIndex() ==
            pCurPam->GetPoint()->nNode.GetIndex() &&
        pBookmark->GetPos().nContent.GetIndex() == nCntntPos )
    {
        // zur Zeit umspannt das SwBookmark keinen Bereich also kann
        // es hier vollstaendig ausgegeben werden.

        // erst die SWG spezifischen Daten:
        if( pBookmark->GetShortName().Len() ||
            pBookmark->GetKeyCode().GetCode() )
        {
            OutComment( *this, sRTF_BKMKKEY );
            OutULong( ( pBookmark->GetKeyCode().GetCode() |
                     pBookmark->GetKeyCode().GetModifier() ));
            if( !pBookmark->GetShortName().Len() )
                Strm() << "  " ;
            else
            {
                Strm() << ' ';
                OutRTF_AsByteString( *this, pBookmark->GetShortName() );
            }
            Strm() << '}';
        }
        OutComment( *this, sRTF_BKMKSTART ) << ' ';
        RTFOutFuncs::Out_String( Strm(), pBookmark->GetName(),
                                DEF_ENCODING, bWriteHelpFmt ) << '}';

        OutComment( *this, sRTF_BKMKEND ) << ' ';
        RTFOutFuncs::Out_String( Strm(), pBookmark->GetName(),
                                DEF_ENCODING, bWriteHelpFmt ) << '}';

        if( ++nBkmkTabPos >= pDoc->GetBookmarks().Count() )
            nBkmkTabPos = USHRT_MAX;
        else
            pBookmark = pDoc->GetBookmarks()[ nBkmkTabPos ];
    }
}


void SwRTFWriter::OutFlyFrm()
{
    if( !pFlyPos )
        return;

    // gebe alle freifliegenden Rahmen die sich auf den akt. Absatz
    // und evt. auf das aktuelle Zeichen beziehen, aus.

    // suche nach dem Anfang der FlyFrames
    for( USHORT n = 0; n < pFlyPos->Count() &&
            (*pFlyPos)[n]->GetNdIndex().GetIndex() <
                pCurPam->GetPoint()->nNode.GetIndex(); ++n )
        ;
    if( n < pFlyPos->Count() )
        while( n < pFlyPos->Count() &&
                pCurPam->GetPoint()->nNode.GetIndex() ==
                    (*pFlyPos)[n]->GetNdIndex().GetIndex() )
        {
            // den Array-Iterator weiterschalten, damit nicht doppelt
            // ausgegeben wird !!
            OutRTFFlyFrms( (const SwFlyFrmFmt&)(*pFlyPos)[n++]->GetFmt() );
        }
}


USHORT SwRTFWriter::GetId( const Color& rColor ) const
{
    ASSERT( pColTbl, "Wo ist meine Color-Tabelle?" );
    for( USHORT n = 0; n < pColTbl->Count(); n++ )
        if( rColor == (*pColTbl)[ n ] )
            return n;

    ASSERT( FALSE, "Color nicht in der Tabelle" );
    return 0;
}

USHORT SwRTFWriter::GetId( const SvxFontItem& rFont ) const
{
    const SfxItemPool& rPool = pDoc->GetAttrPool();
    const SvxFontItem* pFont = (const SvxFontItem*)GetDfltAttr( RES_CHRATR_FONT );
    if( rFont == *pFont )
        return 0;

    USHORT n = 1;
    if( 0 != ( pFont = (const SvxFontItem*)rPool.GetPoolDefaultItem(
                                                        RES_CHRATR_FONT )))
    {
        if( rFont == *pFont )
            return 1;
        ++n;
    }

    USHORT nMaxItem = rPool.GetItemCount( RES_CHRATR_FONT );
    for( USHORT nGet = 0; nGet < nMaxItem; ++nGet )
        if( 0 != (pFont = (const SvxFontItem*)rPool.GetItem(
            RES_CHRATR_FONT, nGet )) )
        {
            if( rFont == *pFont )
                return n;
            ++n;
        }

    ASSERT( FALSE, "Font nicht in der Tabelle" );
    return 0;
}

USHORT SwRTFWriter::GetId( const Font& rFont ) const
{
    return GetId( SvxFontItem( rFont.GetFamily(), rFont.GetName(),
                                rFont.GetStyleName(), rFont.GetPitch(),
                                rFont.GetCharSet() ) );
}

USHORT SwRTFWriter::GetId( const SwTxtFmtColl& rColl ) const
{
    // suche das angegebene Format
    const SvPtrarr & rArr = *pDoc->GetTxtFmtColls();
    for( USHORT n = 0; n < rArr.Count(); n++ )
        if( (SwTxtFmtColl*)rArr[ n ] == &rColl )
            return n;
    ASSERT( FALSE, "TextCollection nicht in der Tabelle" );
    return 0;
}

USHORT SwRTFWriter::GetId( const SwCharFmt& rFmt ) const
{
    // suche das angegebene Format
    const SvPtrarr & rArr = *pDoc->GetCharFmts();
    for( USHORT n = 0; n < rArr.Count(); n++ )
        if( (SwCharFmt*)rArr[ n ] == &rFmt )
            return n + pDoc->GetTxtFmtColls()->Count();
    ASSERT( FALSE, "CharDFFormat nicht in der Tabelle" );
    return 0;
}

void SwRTFWriter::OutPageDesc()
{
    // Ausgabe der Page-Descriptoren
    USHORT nSize = pDoc->GetPageDescCnt();
    if( !nSize )
        return;

    Strm() << SwRTFWriter::sNewLine;        // ein Trenner
    bOutPageDesc = bOutPageDescTbl = TRUE;
    OutComment( *this, sRTF_PGDSCTBL );
    for( USHORT n = 0; n < nSize; ++n )
    {
        const SwPageDesc& rPageDesc = pDoc->GetPageDesc( n );

        Strm() << SwRTFWriter::sNewLine << '{' << sRTF_PGDSC;
        OutULong( n ) << sRTF_PGDSCUSE;
        OutULong( rPageDesc.ReadUseOn() );

        OutRTFPageDescription( rPageDesc, FALSE, FALSE );

        // suche den Folge-PageDescriptor:
        USHORT i = nSize;
        while( i  )
            if( rPageDesc.GetFollow() == &pDoc->GetPageDesc( --i ) )
                break;
        Strm() << sRTF_PGDSCNXT;
        OutULong( i ) << ' ';
        RTFOutFuncs::Out_String( Strm(), rPageDesc.GetName(),
                                DEF_ENCODING, bWriteHelpFmt ) << ";}";
    }
    Strm() << '}' << SwRTFWriter::sNewLine;
    bOutPageDesc = bOutPageDescTbl = FALSE;
}

void SwRTFWriter::OutRTFPageDescription( const SwPageDesc& rPgDsc,
                                            BOOL bWriteReset,
                                            BOOL bCheckForFirstPage )
{
    // jetzt noch den Teil fuer alle anderen Applikationen:
    const SwPageDesc *pSave = pAktPageDesc;
    BOOL bOldOut = bOutPageDesc, bOldHDFT = bOutLeftHeadFoot;

    // falls es einen Follow gibt,
    pAktPageDesc = &rPgDsc;
    if( bCheckForFirstPage && pAktPageDesc->GetFollow() &&
        pAktPageDesc->GetFollow() != pAktPageDesc )
        pAktPageDesc = pAktPageDesc->GetFollow();

    bOutPageDesc = TRUE;
    bOutLeftHeadFoot = FALSE;

    if( bWriteReset )
    {
        if( bFirstLine && bWriteAll &&
            pCurPam->GetPoint()->nNode == pOrigPam->Start()->nNode )
            Strm() << sRTF_SECTD << sRTF_SBKNONE;
        else
            Strm() << sRTF_SECT << sRTF_SECTD;
    }

    if( pAktPageDesc->GetLandscape() )
        Strm() << sRTF_LNDSCPSXN;

    const SwFmt *pFmt = &pAktPageDesc->GetMaster(); //GetLeft();
    OutRTF_SwFmt( *this, *pFmt );

    // falls es gesharte Heaer/Footer gibt, so gebe diese auch noch aus
    if( PD_MIRROR & pAktPageDesc->GetUseOn() &&
        !pAktPageDesc->IsFooterShared() || !pAktPageDesc->IsHeaderShared() )
    {
        bOutLeftHeadFoot = TRUE;
        const SfxPoolItem* pHt;
        if( !pAktPageDesc->IsHeaderShared() &&
            SFX_ITEM_SET == pAktPageDesc->GetLeft().GetAttrSet().
                    GetItemState( RES_HEADER, FALSE, &pHt ))
            OutRTF_SwFmtHeader( *this, *pHt );

        if( !pAktPageDesc->IsFooterShared() &&
            SFX_ITEM_SET == pAktPageDesc->GetLeft().GetAttrSet().
                    GetItemState( RES_FOOTER, FALSE, &pHt ))
            OutRTF_SwFmtFooter( *this, *pHt );
        bOutLeftHeadFoot = FALSE;
    }

    if( pAktPageDesc != &rPgDsc )
    {
        pAktPageDesc = &rPgDsc;
        Strm() << sRTF_TITLEPG;

        // die Header/Footer der 1. Seite ausgeben
        const SfxPoolItem* pHt;
        if( SFX_ITEM_SET == pAktPageDesc->GetMaster().GetAttrSet().
                    GetItemState( RES_HEADER, FALSE, &pHt ))
            OutRTF_SwFmtHeader( *this, *pHt );

        if( SFX_ITEM_SET == pAktPageDesc->GetMaster().GetAttrSet().
                                GetItemState( RES_FOOTER, FALSE, &pHt ))
            OutRTF_SwFmtFooter( *this, *pHt );
    }

    pAktPageDesc = pSave;
    bOutPageDesc = bOldOut;
    bOutLeftHeadFoot = bOldHDFT;
}

BOOL SwRTFWriter::OutBreaks( const SfxItemSet& rSet )
{
    // dann nie Seitenumbrueche ausgeben
    BOOL bPgDscWrite = FALSE;

    if( !bOutOutlineOnly && bOutPageAttr && !bIgnoreNextPgBreak)
    {
        const SfxPoolItem *pItem;
        if( SFX_ITEM_SET == rSet.GetItemState( RES_PAGEDESC, TRUE, &pItem )
            && ((SwFmtPageDesc*)pItem)->GetPageDesc() )
        {
            const SwFmtPageDesc& rPgDsc = *(SwFmtPageDesc*)pItem;
            for( USHORT nPos = pDoc->GetPageDescCnt(); nPos; )
                if( &pDoc->GetPageDesc( --nPos ) == rPgDsc.GetPageDesc() )
                {
                    pAktPageDesc = ((SwFmtPageDesc*)pItem)->GetPageDesc();
                                                // FALSE wegen schliessender Klammer !!
                    OutComment( *this, sRTF_PGDSCNO, FALSE );
                    OutULong( nPos ) << '}';

                    // nicht weiter, in Styles gibts keine SectionControls !!
                    if( !bOutStyleTab )
                        OutRTFPageDescription( *rPgDsc.GetPageDesc(),
                                                        TRUE, TRUE );
                    bPgDscWrite = TRUE;
                    break;
                }
        }
        else if( SFX_ITEM_SET == rSet.GetItemState( RES_BREAK, TRUE, &pItem ) )
        {
            const SvxFmtBreakItem &rBreak = *(SvxFmtBreakItem*)pItem;
            if( bWriteHelpFmt )
            {
                if( SVX_BREAK_PAGE_BEFORE == rBreak.GetBreak() ||
                    SVX_BREAK_PAGE_AFTER == rBreak.GetBreak() ||
                    SVX_BREAK_PAGE_BOTH == rBreak.GetBreak() )
                {
                    bOutFmtAttr = TRUE;
                    Strm() << sRTF_PAGE;
                }
            }
            else
                switch( rBreak.GetBreak() )
                {
                case SVX_BREAK_COLUMN_BEFORE:
                case SVX_BREAK_COLUMN_AFTER:
                case SVX_BREAK_COLUMN_BOTH:
                    break;

                case SVX_BREAK_PAGE_BEFORE:
                    bOutFmtAttr = TRUE;
                    Strm() << sRTF_PAGE;
                    break;

                case SVX_BREAK_PAGE_AFTER:
                    OutComment( *this, sRTF_PGBRK, FALSE ) << "0}";
                    break;

                case SVX_BREAK_PAGE_BOTH:
                    OutComment( *this, sRTF_PGBRK, FALSE ) << "1}";
                    break;
                }
        }
    }
    bIgnoreNextPgBreak = FALSE;
    return bPgDscWrite;
}


void SwRTFWriter::CheckEndNodeForSection( const SwNode& rNd )
{
    const SwSectionNode* pSectNd = rNd.FindStartNode()->GetSectionNode();
    if( pSectNd /*&& CONTENT_SECTION == pSectNd->GetSection().GetType()*/ )
    {
        const SwSectionFmt* pSectFmt = pSectNd->GetSection().GetFmt();

        // diese Section hatte den akt. Abschnitt bestimmt
        // wer bestimmt den nachsten??
        SwNodeIndex aIdx( rNd, 1 );
        pSectNd = aIdx.GetNode().GetSectionNode();
        if( !( ( pSectNd || aIdx.GetNode().IsEndNode() &&
            0 != ( pSectNd = aIdx.GetNode().FindStartNode()->GetSectionNode() ))
            /*&& CONTENT_SECTION == pSectNd->GetSection().GetType()*/ ))
        {
            // wer bestimmt denn nun den neuen Abschnitt?
            // PageDesc oder eine uebergeordnete Section?
            SwSection* pParent = pSectFmt->GetParentSection();
//          while( pParent /*&& CONTENT_SECTION != pParent->GetType()*/ )
//              pParent = pParent->GetParent();

            if( pParent /*&& CONTENT_SECTION == pParent->GetType()*/ )
                OutRTF_SwSectionNode( *this, *pParent->
                        GetFmt()->GetSectionNode( TRUE ) );
            else
            {
                Strm() << sRTF_SECT << sRTF_SECTD << sRTF_SBKNONE;
                OutRTFPageDescription( ( pAktPageDesc
                                    ? *pAktPageDesc
                                    : pDoc->GetPageDesc(0) ),
                                FALSE, TRUE );
                Strm() << SwRTFWriter::sNewLine;
            }
        }
        // else
            // weiter machen, der naechste definiert den neuen Abschnitt
    }
}

// Struktur speichert die aktuellen Daten des Writers zwischen, um
// einen anderen Dokument-Teil auszugeben, wie z.B. Header/Footer
RTFSaveData::RTFSaveData( SwRTFWriter& rWriter, ULONG nStt, ULONG nEnd )
    : rWrt( rWriter ),
    pOldPam( rWrt.pCurPam ), pOldEnd( rWrt.GetEndPaM() ),
    pOldFlyFmt( rWrt.pFlyFmt ), pOldPageDesc( rWrt.pAktPageDesc ),
    pOldAttrSet( rWrt.GetAttrSet() )
{
    bOldWriteAll = rWrt.bWriteAll;
    bOldOutTable = rWrt.bOutTable;
    bOldOutPageAttr = rWrt.bOutPageAttr;
    bOldAutoAttrSet = rWrt.bAutoAttrSet;
    bOldOutSection = rWrt.bOutSection;

    rWrt.pCurPam = rWrt.NewSwPaM( *rWrt.pDoc, nStt, nEnd );

    // Tabelle in Sonderbereichen erkennen
    if( nStt != rWrt.pCurPam->GetMark()->nNode.GetIndex() &&
        rWrt.pDoc->GetNodes()[ nStt ]->IsTableNode() )
        rWrt.pCurPam->GetMark()->nNode = nStt;

    rWrt.SetEndPaM( rWrt.pCurPam );
    rWrt.pCurPam->Exchange( );
    rWrt.bWriteAll = TRUE;
    rWrt.bOutTable = FALSE;
    rWrt.bOutPageAttr = FALSE;
    rWrt.SetAttrSet( 0 );
    rWrt.bAutoAttrSet = FALSE;
    rWrt.bOutSection = FALSE;
}


RTFSaveData::~RTFSaveData()
{
    delete rWrt.pCurPam;                    // Pam wieder loeschen

    rWrt.pCurPam = pOldPam;
    rWrt.SetEndPaM( pOldEnd );
    rWrt.bWriteAll = bOldWriteAll;
    rWrt.bOutTable = bOldOutTable;
    rWrt.pFlyFmt = pOldFlyFmt;
    rWrt.pAktPageDesc = pOldPageDesc;
    rWrt.SetAttrSet( pOldAttrSet );
    rWrt.bAutoAttrSet = bOldAutoAttrSet;
    rWrt.bOutPageAttr = bOldOutPageAttr;
    rWrt.bOutSection = bOldOutSection;
}


void GetRTFWriter( const String& rFltName, WriterRef& xRet )
{
    xRet = new SwRTFWriter( rFltName );
}


/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/rtf/wrtrtf.cxx,v 1.5 2001-02-13 09:24:35 jp Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.4  2001/02/06 15:55:19  mib
      real 6.0 file format

      Revision 1.3  2000/11/16 09:57:36  jp
      export CJK attributes

      Revision 1.2  2000/10/09 13:31:40  jp
      Bug #78626#: _OutFont - dontknow is a valid value

      Revision 1.1.1.1  2000/09/18 17:14:56  hr
      initial import

      Revision 1.202  2000/09/18 16:04:51  willem.vandorp
      OpenOffice header added.

      Revision 1.201  2000/08/22 20:51:18  jp
      OutHeader: don't ask for DBName

      Revision 1.200  2000/08/04 10:48:32  jp
      Soft-/HardHyphens & HardBlanks changed from attribute to unicode character; use rtfout functions

      Revision 1.199  2000/07/04 16:23:30  jp
      AMA_TEST define removed

      Revision 1.198  2000/05/09 17:23:08  jp
      Changes for Unicode

      Revision 1.197  2000/03/03 16:22:05  pl
      #73771# workaround for c50 intel compiler

      Revision 1.196  2000/03/03 15:21:02  os
      StarView remainders removed

      Revision 1.195  2000/02/24 18:30:25  jp
      Bug #73485#: write right follow of pagedesc, initialize the akt. pagedesc

      Revision 1.194  2000/01/25 20:10:34  jp
      Bug #72146#: _OutFont - replace system/dontknow charset to the current charset

      Revision 1.193  1999/11/22 18:00:05  jp
      OutSectionNode: dont skip to top parent section

      Revision 1.192  1999/10/25 12:16:40  jp
      get font charset over RTL-Function

      Revision 1.191  1999/08/05 20:53:50  JP
      write flyfrms before the paragraph and not behind


      Rev 1.190   05 Aug 1999 22:53:50   JP
   write flyfrms before the paragraph and not behind

      Rev 1.189   22 Jul 1999 20:01:10   JP
   read&write footer and header height

      Rev 1.188   23 Jun 1999 19:12:30   JP
   optimize for NumRules, interface of FindPos_Bkmk has changed

      Rev 1.187   16 Jun 1999 19:49:06   JP
   Change interface of base class Writer

      Rev 1.186   04 May 1999 15:00:02   JP
   FilterExportklasse Writer von SvRef abgeleitet, damit sie immer zerstoert wird

      Rev 1.185   06 Apr 1999 17:00:18   JP
   Bug #64361#: MakeHeader - ggfs. ersten PageDesc vom 1.Node besorgen

      Rev 1.184   19 Mar 1999 17:08:06   JP
   Bug #63772#: Spaltige Bereiche im-/exportieren

      Rev 1.183   18 Mar 1999 09:51:54   JP
   Task #63049#: Numerierung mit rel. Einzuegen

      Rev 1.182   16 Mar 1999 23:20:00   JP
   Task #63049#: Einzuege bei NumRules relativ

      Rev 1.181   05 Mar 1999 14:44:34   JP
   Bug #57749#: spaltige Bereiche schreiben - alle Section-Typen

      Rev 1.180   04 Mar 1999 19:59:22   JP
   Bug #57749#: spaltige Bereiche schreiben

      Rev 1.179   17 Nov 1998 10:45:52   OS
   #58263# NumType durch SvxExtNumType ersetzt

      Rev 1.178   09 Nov 1998 17:28:46   JP
   Bug #58817#: StyleTabelle wieder schreiben

      Rev 1.177   30 Oct 1998 18:30:26   JP
   Task #58596#: neues Flag an der Writerklasse -> schreibe nur die 1. Tabelle

      Rev 1.176   28 Oct 1998 10:36:12   JP
   Bug #58565#: Assert im OutHeader behoben

      Rev 1.175   11 Aug 1998 12:16:36   JP
   Bug #54796#: fehlender Numerierunstyp und Bugfixes

      Rev 1.174   06 Aug 1998 21:45:56   JP
   Bug #54796#: neue NumerierungsTypen (WW97 kompatibel)

      Rev 1.173   04 Jun 1998 19:28:08   JP
   Bug #50887#: Font bei Grafik-NumFormaten ist 0, also default Font schreiben

      Rev 1.172   13 May 1998 17:15:22   JP
   OutRTFPageDesc: beim 1.PageDesc keinen Sectionbreak schreiben

      Rev 1.171   11 May 1998 22:50:32   JP
   Zeichenvorlagen schreiben

      Rev 1.170   11 May 1998 16:46:50   JP
   RTF-CharFormate ausgeben

      Rev 1.169   05 May 1998 14:00:26   JP
   linke/rechte Header/Footer korrekt rausschreiben

      Rev 1.168   20 Apr 1998 17:43:20   JP
   neu: Numerierung lesen/schreiben

      Rev 1.167   03 Apr 1998 18:51:56   JP
   RTF-Parser um neue Tokens erweitert

      Rev 1.166   20 Feb 1998 13:36:40   MA
   headerfiles gewandert

      Rev 1.165   29 Jan 1998 21:35:24   JP
   GetEndOfIcons ersetzt durch GetEndOfExtras, das auf GetEndOfRedlines mappt

      Rev 1.164   27 Jan 1998 21:50:18   JP
   GetNumDepend durch GetDepends ersetzt

      Rev 1.163   26 Nov 1997 15:05:30   MA
   headerfiles

      Rev 1.162   09 Oct 1997 14:27:52   JP
   Umstellung NodeIndex/-Array/BigPtrArray

      Rev 1.161   12 Sep 1997 10:57:22   OS
   ITEMID_* definiert

      Rev 1.160   03 Sep 1997 08:58:52   OS
   Header

      Rev 1.159   29 Aug 1997 13:53:24   JP
   VCL Color Anpassung

      Rev 1.158   15 Aug 1997 12:51:42   OS
   charatr/frmatr/txtatr aufgeteilt

      Rev 1.157   11 Aug 1997 17:48:44   OS
   Header-Umstellung

      Rev 1.156   07 Aug 1997 15:06:36   OM
   Headerfile-Umstellung

      Rev 1.155   07 Apr 1997 16:35:10   JP
   Das ShowProgressFlag der Writerklasse auswerten

      Rev 1.154   17 Feb 1997 12:03:44   MA
   opt: Outline schneller schreiben wenn Gliederung nur im Body

      Rev 1.153   15 Feb 1997 17:57:14   JP
   OutStyleTab: pAttrSet Pointer wieder clearen

      Rev 1.152   15 Feb 1997 17:00:22   JP
   neu: nur Gliederungsabsaetze schreiben

      Rev 1.151   14 Jan 1997 08:51:42   MA
   includes

*************************************************************************/


