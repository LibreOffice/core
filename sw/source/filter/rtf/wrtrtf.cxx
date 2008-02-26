/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: wrtrtf.cxx,v $
 *
 *  $Revision: 1.41 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 14:19:37 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#include <stdlib.h>
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#include <comphelper/string.hxx>

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _DATETIME_HXX //autogen
#include <tools/datetime.hxx>
#endif
#ifndef _SV_FONTCVT_HXX
#include <vcl/fontcvt.hxx>
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
#ifndef _SVX_FRMDIRITEM_HXX
#include <svx/frmdiritem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX //autogen
#include <svx/udlnitem.hxx>
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
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
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
#ifndef _FLYPOS_HXX
#include <flypos.hxx>
#endif
#ifndef _BOOKMRK_HXX
#include <bookmrk.hxx>      // fuer SwBookmark ...
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>     // fuer SwPageDesc...
#endif
#ifndef _FTNINFO_HXX
#include <ftninfo.hxx>
#endif
#ifndef _CHARFMT_HXX
#include <charfmt.hxx>
#endif
#ifndef _SWSTYLENAMEMAPPER_HXX
#include <SwStyleNameMapper.hxx>
#endif
#ifndef _SECTION_HXX //autogen
#include <section.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>      // fuer SwPageDesc ...
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
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
#ifndef _SWDOCSH_HXX
#include <docsh.hxx>
#endif

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>

#if defined(UNX)
const sal_Char SwRTFWriter::sNewLine = '\012';
#else
const sal_Char __FAR_DATA SwRTFWriter::sNewLine[] = "\015\012";
#endif



SV_DECL_VARARR( RTFColorTbl, Color, 5, 8 )
SV_IMPL_VARARR( RTFColorTbl, Color )


SwRTFWriter::SwRTFWriter( const String& rFltName, const String & rBaseURL ) :
    eDefaultEncoding(
        rtl_getTextEncodingFromWindowsCharset(
            sw::ms::rtl_TextEncodingToWinCharset(DEF_ENCODING))),
    eCurrentEncoding(eDefaultEncoding)
{
    SetBaseURL( rBaseURL );
    // schreibe Win-RTF-HelpFileFmt
    bWriteHelpFmt = 'W' == rFltName.GetChar( 0 );
    // schreibe nur Gliederungs Absaetze
    bOutOutlineOnly = 'O' == rFltName.GetChar( 0 );
}


SwRTFWriter::~SwRTFWriter()
{}


ULONG SwRTFWriter::WriteStream()
{
    bOutPageAttr = bOutSection = TRUE;

    bOutStyleTab = bOutTable = bOutPageDesc = bOutPageDescTbl =
    bAutoAttrSet = bOutListNumTxt = bOutLeftHeadFoot = bIgnoreNextPgBreak =
        bTxtAttr = bAssociated = FALSE;

    nCurScript = 1;                     // latin - ask the doc??

    nCurRedline = USHRT_MAX;
    if(pDoc->GetRedlineTbl().Count())
        nCurRedline = 0;


    pCurEndPosLst = 0;
    nBkmkTabPos = USHRT_MAX;
    pAktPageDesc = 0;
    pAttrSet = 0;
    pFlyFmt = 0;        // kein FlyFrmFormat gesetzt

    pColTbl = new RTFColorTbl;
    pNumRuleTbl = 0;

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
    if( pNumRuleTbl )
    {
        // don't destroy the numrule pointers in the DTOR.
        pNumRuleTbl->Remove( 0, pNumRuleTbl->Count() );
        delete pNumRuleTbl;
    }
    delete pRedlAuthors;

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
                if (rNd.IsTableNode())
                {
                    bool bAllOk = false;
                    if (const SwTableNode *pNd = rNd.GetTableNode())
                    {
                        if (const SwFrmFmt *pFmt = pNd->GetTable().GetFrmFmt())
                        {
                            OutBreaks(pFmt->GetAttrSet());
                            bAllOk = true;
                        }
                        OutRTF_SwTblNode(*this, *pNd);
                    }
                    ASSERT(bAllOk, "Unexpected missing properties from tables");
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
        << sRTF_ANSI;
    if( bWriteAll )
    {
        Strm() << sRTF_DEFF;
        OutULong( GetId( (SvxFontItem&)pDoc->GetAttrPool().GetDefaultItem(
                        RES_CHRATR_FONT ) ));
    }
    // JP 13.02.2001 - if this not exist, MS don't understand our ansi
    //                  characters (0x80-0xff).
    Strm() << "\\adeflang1025";

    OutRTFFontTab();
    OutRTFColorTab();
    OutRTFStyleTab();
    OutRTFListTab();
    OutRTFRevTab();

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
        if ( !pDoc->get(IDocumentSettingAccess::USE_VIRTUAL_DEVICE) )
            Strm() << sRTF_LYTPRTMET;
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
            : const_cast<const SwDoc *>(pDoc)->GetPageDesc( 0 );
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
        OutRTFPageDescription( rPageDesc, FALSE, TRUE );    // Changed bCheckForFirstPage to TRUE so headers
                                                            // following title page are correctly added - i13107
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

        switch( rFtnInfo.aFmt.GetNumberingType() )
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

        switch( rEndNoteInfo.aFmt.GetNumberingType() )
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

    if( pDoc->_GetDBDesc().sDataSource.getLength() )
    {
        // stelle erstmal fest, ob ueberhaupt Datenbankfelder benutzt werden!
        const SwFldTypes* pTypes = pDoc->GetFldTypes();
        for( USHORT nCnt = pTypes->Count(); nCnt >= INIT_FLDTYPES; )
            if( RES_DBFLD == (*pTypes)[ --nCnt ]->Which() &&
                (*pTypes)[ nCnt ]->GetDepends() )
            {
                Strm() << '{' << sRTF_FIELD;
                OutComment( *this, sRTF_FLDINST ) << " DATA ";
                SwDBData aData = pDoc->GetDBData();
                String sOut(aData.sDataSource);
                sOut += DB_DELIM;
                sOut += (String)aData.sCommand;
                RTFOutFuncs::Out_String( Strm(), sOut,
                                        eDefaultEncoding, bWriteHelpFmt );
                Strm() << "}{" << sRTF_FLDRSLT << " }}";
                break;
            }
    }

    pAttrSet = 0;

    Strm() << SwRTFWriter::sNewLine;        // ein Trenner
}

void SwRTFWriter::OutInfoDateTime( const sal_Char* i_pStr,
    const util::DateTime& i_rDT )
{
    Strm() << '{' << i_pStr << sRTF_YR;
    OutLong( Strm(), i_rDT.Year ) << sRTF_MO;
    OutLong( Strm(), i_rDT.Month ) << sRTF_DY;
    OutLong( Strm(), i_rDT.Day ) << sRTF_HR;
    OutLong( Strm(), i_rDT.Hours ) << sRTF_MIN;
    OutLong( Strm(), i_rDT.Minutes ) << '}';
}

bool CharsetSufficient(const String &rString, rtl_TextEncoding eChrSet)
{
    const sal_uInt32 nFlags =
        RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR |
        RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR;
    rtl::OString sDummy;
    rtl::OUString sStr(rString);
    return sStr.convertToString(&sDummy, eChrSet, nFlags);
}

void SwRTFWriter::OutUnicodeSafeRecord(const sal_Char *pToken,
    const String &rContent)
{
    if (rContent.Len())
    {
        bool bNeedUnicodeWrapper = !CharsetSufficient(rContent, eDefaultEncoding);

        if (bNeedUnicodeWrapper)
            Strm() << '{' << sRTF_UPR;

        Strm() << '{' << pToken << ' ';
        OutRTF_AsByteString(*this, rContent, eDefaultEncoding);
        Strm() << '}';

        if (bNeedUnicodeWrapper)
        {
            OutComment(*this, sRTF_UD);
            Strm() << '{' << pToken << ' ';
            RTFOutFuncs::Out_String(Strm(), rContent, eDefaultEncoding,
                bWriteHelpFmt);
            Strm() << "}}}";
        }

    }
}

void SwRTFWriter::OutDocInfoStat()
{
    Strm() << '{' << sRTF_INFO;

    SwDocShell *pDocShell(pDoc->GetDocShell());
    uno::Reference<document::XDocumentProperties> xDocProps;
    if (pDocShell) {
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
            pDocShell->GetModel(), uno::UNO_QUERY_THROW);
        xDocProps.set(xDPS->getDocumentProperties());
    }

    // may be null (in case of copying)
    if (xDocProps.is())
    {
        OutUnicodeSafeRecord(sRTF_TITLE,    xDocProps->getTitle());
        OutUnicodeSafeRecord(sRTF_SUBJECT,  xDocProps->getSubject());

        OutUnicodeSafeRecord(sRTF_KEYWORDS,
        ::comphelper::string::convertCommaSeparated(xDocProps->getKeywords()));
        OutUnicodeSafeRecord(sRTF_DOCCOMM,  xDocProps->getDescription());

        OutUnicodeSafeRecord(sRTF_AUTHOR,   xDocProps->getAuthor() );
        OutInfoDateTime(sRTF_CREATIM,       xDocProps->getCreationDate());

        OutUnicodeSafeRecord(sRTF_AUTHOR,   xDocProps->getModifiedBy() );
        OutInfoDateTime(sRTF_REVTIM,        xDocProps->getModificationDate());

        OutInfoDateTime(sRTF_PRINTIM,       xDocProps->getPrintDate());

    }

    // fuer interne Zwecke - Versionsnummer rausschreiben
    Strm() << '{' << sRTF_COMMENT << " StarWriter}{" << sRTF_VERN;
    OutLong( Strm(), SUPD*10 ) << '}';

    Strm() << '}';
}

static void InsColor( RTFColorTbl& rTbl, const Color& rCol )
{
    USHORT n;
    for( n = 0; n < rTbl.Count(); ++n )
        if( rTbl[n] == rCol )
            return;         // schon vorhanden, zurueck

    n = COL_AUTO == rCol.GetColor() ? 0 : rTbl.Count();
    rTbl.Insert( rCol, n );
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
        {
            if( 0 != (pCol = (const SvxColorItem*)rPool.GetItem(
                RES_CHRATR_COLOR, n ) ) )
                InsColor( *pColTbl, pCol->GetValue() );
        }

        const SvxUnderlineItem* pUnder = (const SvxUnderlineItem*)GetDfltAttr( RES_CHRATR_UNDERLINE );
        InsColor( *pColTbl, pUnder->GetColor() );
        nMaxItem = rPool.GetItemCount(RES_CHRATR_UNDERLINE);
        for( n = 0; n < nMaxItem;n++)
        {
            if( 0 != (pUnder = (const SvxUnderlineItem*)rPool.GetItem( RES_CHRATR_UNDERLINE, n ) ) )
                InsColor( *pColTbl, pUnder->GetColor() );

        }

    }

    // das Frame Hintergrund - Attribut
    static const USHORT aBrushIds[] = {
                                RES_BACKGROUND, RES_CHRATR_BACKGROUND, 0 };

    for( const USHORT* pIds = aBrushIds; *pIds; ++pIds )
    {
        const SvxBrushItem* pBkgrd = (const SvxBrushItem*)GetDfltAttr( *pIds );
        InsColor( *pColTbl, pBkgrd->GetColor() );
        if( 0 != ( pBkgrd = (const SvxBrushItem*)rPool.GetPoolDefaultItem(
                        *pIds ) ))
        {
            InsColor( *pColTbl, pBkgrd->GetColor() );
        }
        nMaxItem = rPool.GetItemCount( *pIds );
        for( n = 0; n < nMaxItem; ++n )
            if( 0 != (pBkgrd = (const SvxBrushItem*)rPool.GetItem(
                    *pIds , n ) ))
            {
                InsColor( *pColTbl, pBkgrd->GetColor() );
            }
    }

    // das Frame Schatten - Attribut
    {
        const SvxShadowItem* pShadow = (const SvxShadowItem*)GetDfltAttr(
                                                            RES_SHADOW );
        InsColor( *pColTbl, pShadow->GetColor() );
        if( 0 != ( pShadow = (const SvxShadowItem*)rPool.GetPoolDefaultItem(
                        RES_SHADOW ) ))
        {
            InsColor( *pColTbl, pShadow->GetColor() );
        }
        nMaxItem = rPool.GetItemCount(RES_SHADOW);
        for( n = 0; n < nMaxItem; ++n )
            if( 0 != (pShadow = (const SvxShadowItem*)rPool.GetItem(
                RES_SHADOW, n ) ) )
            {
                InsColor( *pColTbl, pShadow->GetColor() );
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
        if( n || COL_AUTO != rCol.GetColor() )
        {
            Strm() << sRTF_RED;
            OutULong( rCol.GetRed() ) << sRTF_GREEN;
            OutULong( rCol.GetGreen() ) << sRTF_BLUE;
            OutULong( rCol.GetBlue() );
        }
        Strm() << ';';
    }
    Strm() << '}';
}

bool FontCharsetSufficient(const String &rFntNm, const String &rAltNm,
    rtl_TextEncoding eChrSet)
{
    bool bRet = CharsetSufficient(rFntNm, eChrSet);
    if (bRet)
        bRet = CharsetSufficient(rAltNm, eChrSet);
    return bRet;
}

static void _OutFont( SwRTFWriter& rWrt, const SvxFontItem& rFont, USHORT nNo )
{
    rWrt.Strm() << '{' << sRTF_F;

    const char* pStr = sRTF_FNIL;
    switch (rFont.GetFamily())
    {
        case FAMILY_ROMAN:
            pStr = sRTF_FROMAN;
            break;
        case FAMILY_SWISS:
            pStr = sRTF_FSWISS;
            break;
        case FAMILY_MODERN:
            pStr = sRTF_FMODERN;
            break;
        case FAMILY_SCRIPT:
            pStr = sRTF_FSCRIPT;
            break;
        case FAMILY_DECORATIVE:
            pStr = sRTF_FDECOR;
            break;
        default:
            break;
    }
    rWrt.OutULong(nNo) << pStr << sRTF_FPRQ;

    USHORT nVal = 0;
    switch (rFont.GetPitch())
    {
        case PITCH_FIXED:
            nVal = 1;
            break;
        case PITCH_VARIABLE:
            nVal = 2;
            break;
        default:
            nVal = 0;
            break;
    }
    rWrt.OutULong(nVal);

    sw::util::FontMapExport aRes(rFont.GetFamilyName());

    /*
     #i10538#
     In rtf the fontname is in the fontcharset, so if that isn't possible
     then bump the charset up to unicode
    */
    sal_uInt8 nChSet = 0;
    rtl_TextEncoding eChrSet = rFont.GetCharSet();
    nChSet = sw::ms::rtl_TextEncodingToWinCharset(eChrSet);
    eChrSet = rtl_getTextEncodingFromWindowsCharset(nChSet);
    if (!FontCharsetSufficient(aRes.msPrimary, aRes.msSecondary, eChrSet))
    {
        eChrSet = RTL_TEXTENCODING_UNICODE;
        nChSet = sw::ms::rtl_TextEncodingToWinCharset(eChrSet);
        eChrSet = rtl_getTextEncodingFromWindowsCharset(nChSet);
    }

    rWrt.Strm() << sRTF_FCHARSET;
    rWrt.OutULong( nChSet );
    rWrt.Strm() << ' ';
    if (aRes.HasDistinctSecondary())
    {
        RTFOutFuncs::Out_Fontname(rWrt.Strm(), aRes.msPrimary, eChrSet,
            rWrt.bWriteHelpFmt);
        OutComment(rWrt, sRTF_FALT) << ' ';
        RTFOutFuncs::Out_Fontname(rWrt.Strm(), aRes.msSecondary, eChrSet,
            rWrt.bWriteHelpFmt) << '}';
    }
    else
    {
        RTFOutFuncs::Out_Fontname(rWrt.Strm(), aRes.msPrimary, eChrSet,
            rWrt.bWriteHelpFmt);
    }
    rWrt.Strm() << ";}";
}

void SwRTFWriter::OutRTFFontTab()
{
    USHORT n = 0;
    const SfxItemPool& rPool = pDoc->GetAttrPool();
    const SvxFontItem* pFont = (const SvxFontItem*)GetDfltAttr(RES_CHRATR_FONT);

    Strm() << SwRTFWriter::sNewLine << '{' << sRTF_FONTTBL;
    _OutFont( *this, *pFont, n++ );

    pFont = (const SvxFontItem*)rPool.GetPoolDefaultItem(RES_CHRATR_FONT);
    if (pFont)
        _OutFont(*this, *pFont, n++);

    PutNumFmtFontsInAttrPool();
    PutCJKandCTLFontsInAttrPool();

    USHORT nMaxItem = rPool.GetItemCount(RES_CHRATR_FONT);
    for (USHORT nGet = 0; nGet < nMaxItem; ++nGet)
    {
        pFont = (const SvxFontItem*)rPool.GetItem(RES_CHRATR_FONT, nGet);
        if (pFont)
            _OutFont(*this, *pFont, n++);
    }

    Strm() << '}';
}

void RTF_WrtRedlineAuthor::Write(Writer &rWrt)
{
    SwRTFWriter & rRTFWrt = (SwRTFWriter&)rWrt;

    rRTFWrt.Strm() << '{' << sRTF_IGNORE << sRTF_REVTBL << ' ';
    typedef std::vector<String>::iterator myiter;

    for(std::vector<String>::iterator aIter = maAuthors.begin(); aIter != maAuthors.end(); ++aIter)
    {
        rRTFWrt.Strm() << '{';
        // rWrt.bWriteHelpFmt
        RTFOutFuncs::Out_String( rRTFWrt.Strm(), *aIter, rRTFWrt.eDefaultEncoding,  rRTFWrt.bWriteHelpFmt  ) << ";}";
    }
    rRTFWrt.Strm() << '}' << SwRTFWriter::sNewLine;
}

bool SwRTFWriter::OutRTFRevTab()
{
    // Writes the revision author table
    int nRevAuthors = pDoc->GetRedlineTbl().Count();

    pRedlAuthors = new RTF_WrtRedlineAuthor;
    // RTF always seems to use Unknown as the default first entry
    String sUnknown(RTL_CONSTASCII_STRINGPARAM("Unknown"));
    pRedlAuthors->AddName(sUnknown);

    if (nRevAuthors < 1)
        return false;

    // pull out all the redlines and make a vector of all the author names
    for( USHORT i = 0; i < pDoc->GetRedlineTbl().Count(); ++i )
    {
        const SwRedline* pRedl = pDoc->GetRedlineTbl()[ i ];
        const String sAuthor = SW_MOD()->GetRedlineAuthor( pRedl->GetAuthor() );
        pRedlAuthors->AddName(sAuthor);
    }

    pRedlAuthors->Write(*this);
    return true;
}

//Takashi Ono for CJK
const rtl::OUString SwRTFWriter::XlateFmtName( const rtl::OUString &rName, SwGetPoolIdFromName eFlags )
{
#define RES_NONE RES_POOLCOLL_DOC_END

    static const RES_POOL_COLLFMT_TYPE aArr[]={
        RES_POOLCOLL_STANDARD, RES_POOLCOLL_HEADLINE1, RES_POOLCOLL_HEADLINE2,
        RES_POOLCOLL_HEADLINE3, RES_POOLCOLL_HEADLINE4, RES_POOLCOLL_HEADLINE5,
        RES_POOLCOLL_HEADLINE6, RES_POOLCOLL_HEADLINE7, RES_POOLCOLL_HEADLINE8,
        RES_POOLCOLL_HEADLINE9,

        RES_POOLCOLL_TOX_IDX1, RES_POOLCOLL_TOX_IDX2, RES_POOLCOLL_TOX_IDX3,
        RES_NONE, RES_NONE, RES_NONE, RES_NONE, RES_NONE, RES_NONE,
        RES_POOLCOLL_TOX_CNTNT1,

        RES_POOLCOLL_TOX_CNTNT2, RES_POOLCOLL_TOX_CNTNT3, RES_POOLCOLL_TOX_CNTNT4,
        RES_POOLCOLL_TOX_CNTNT5, RES_POOLCOLL_TOX_CNTNT6, RES_POOLCOLL_TOX_CNTNT7,
        RES_POOLCOLL_TOX_CNTNT8, RES_POOLCOLL_TOX_CNTNT9,
        RES_POOLCOLL_TEXT_IDENT, RES_POOLCOLL_FOOTNOTE,

        RES_NONE, RES_POOLCOLL_HEADER, RES_POOLCOLL_FOOTER, RES_POOLCOLL_TOX_IDXH,
        RES_POOLCOLL_LABEL, RES_POOLCOLL_TOX_ILLUSH, RES_POOLCOLL_JAKETADRESS, RES_POOLCOLL_SENDADRESS,
        RES_NONE, RES_NONE,

        RES_NONE, RES_NONE, RES_NONE, RES_POOLCOLL_ENDNOTE, RES_POOLCOLL_TOX_AUTHORITIESH, RES_NONE, RES_NONE,
        RES_POOLCOLL_BUL_LEVEL1, RES_POOLCOLL_BUL_LEVEL1, RES_POOLCOLL_NUM_LEVEL1,

        RES_POOLCOLL_BUL_LEVEL2, RES_POOLCOLL_BUL_LEVEL3, RES_POOLCOLL_BUL_LEVEL4, RES_POOLCOLL_BUL_LEVEL5,
        RES_POOLCOLL_BUL_LEVEL2, RES_POOLCOLL_BUL_LEVEL3, RES_POOLCOLL_BUL_LEVEL4, RES_POOLCOLL_BUL_LEVEL5,
        RES_POOLCOLL_NUM_LEVEL2, RES_POOLCOLL_NUM_LEVEL3, RES_POOLCOLL_NUM_LEVEL4, RES_POOLCOLL_NUM_LEVEL5,

        RES_POOLCOLL_DOC_TITEL, RES_NONE, RES_POOLCOLL_SIGNATURE, RES_NONE,
        RES_POOLCOLL_TEXT, RES_POOLCOLL_TEXT_MOVE, RES_NONE, RES_NONE,

        RES_NONE, RES_NONE, RES_NONE, RES_NONE, RES_POOLCOLL_DOC_SUBTITEL };

    static const sal_Char *stiName[] = {
        "Normal", "heading 1", "heading 2",
        "heading 3", "heading 4", "heading 5",
        "heading 6", "heading 7", "heading 8",
        "heading 9",

        "index 1", "index 2", "index 3",
        "index 4", "index 5", "index 6",
        "index 7", "index 8", "index 9",

        "toc 1", "toc 2", "toc 3",
        "toc 4", "toc 5", "toc 6",
        "toc 7", "toc 8", "toc 9",
        "Normal Indent", "footnote text",

        "annotation text", "header", "footer",  "index heading",
        "caption", "table of figures", "envelope address", "envelope return",
        "footnote reference",   "annotation reference",

        "line number", "page number", "endnote reference",  "endnote text", "table of authorities", "macro", "toa heading",
        "List", "List Bullet", "List Number",

        "List 2", "List 3", "List 4", "List 5",
        "List Bullet 2", "List Bullet 3", "List Bullet 4", "List Bullet 5",
        "List Number 2", "List Number 3", "List Number 4", "List Number 5",

        "Title", "Closing", "Signature", "Default Paragraph Font",
        "Body Text", "Body Text Indent", "List Continue",

        "List Continue 2",  "List Continue 3", "List Continue 4", "List Continue 5", "Message Header", "Subtitle"};

    ASSERT( ( sizeof( aArr ) / sizeof( RES_POOL_COLLFMT_TYPE ) == 75 ),
            "Style-UEbersetzungstabelle hat falsche Groesse" );
    ASSERT( ( sizeof( stiName ) / sizeof( *stiName ) == 75 ),
            "Style-UEbersetzungstabelle hat falsche Groesse" );

    sal_uInt16 idcol = ::SwStyleNameMapper::GetPoolIdFromUIName( rName, eFlags );
    if (idcol==USHRT_MAX) //#i40770# user defined style names get lost
        return rName;

    for (size_t i = 0; i < sizeof( aArr ) / sizeof( *aArr ); i++)
    {
        if ( idcol == aArr[i] )
        {
            return rtl::OUString::createFromAscii(stiName[i]);
        }
    }
    return ::SwStyleNameMapper::GetProgName( idcol, String() );
}

void SwRTFWriter::OutRTFStyleTab()
{
    USHORT n;

    // das 0-Style ist das Default, wird nie ausgegeben !!
    USHORT nArrLen = pDoc->GetTxtFmtColls()->Count();
    if( nArrLen <= 1 && pDoc->GetCharFmts()->Count() <= 1 )
        return;

    bOutStyleTab = TRUE;
    Strm() << SwRTFWriter::sNewLine << '{' << sRTF_STYLESHEET;

    // das Default-TextStyle wird nicht mit ausgegeben !!
    for( n = 1; n < nArrLen; ++n )
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
        RTFOutFuncs::Out_String( Strm(), XlateFmtName( pColl->GetName(), nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL ), eDefaultEncoding,
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
        RTFOutFuncs::Out_String( Strm(), XlateFmtName( pFmt->GetName(), nsSwGetPoolIdFromName::GET_POOLID_CHRFMT ), eDefaultEncoding,
                    bWriteHelpFmt ) << ";}" << SwRTFWriter::sNewLine;
    }

    Strm() << '}';
    bOutStyleTab = FALSE;
}

bool ExportAsInline(const SwFlyFrmFmt& rFlyFrmFmt)
{
    //if not an inline element (hack in our limitations here as to only
    //graphics like this!!!!
    return rFlyFrmFmt.GetAnchor().GetAnchorId() == FLY_IN_CNTNT;
}

void SwRTFWriter::OutRTFFlyFrms(const SwFlyFrmFmt& rFlyFrmFmt)
{
    // ein FlyFrame wurde erkannt, gebe erstmal den aus

    // Hole vom Node und vom letzten Node die Position in der Section
    const SwFmtCntnt& rFlyCntnt = rFlyFrmFmt.GetCntnt();

    ULONG nStt = rFlyCntnt.GetCntntIdx()->GetIndex()+1;
    ULONG nEnd = pDoc->GetNodes()[ nStt - 1 ]->EndOfSectionIndex();

    if( nStt >= nEnd )      // kein Bereich, also kein gueltiger Node
        return;

    if (!ExportAsInline(rFlyFrmFmt))
        Strm() << SwRTFWriter::sNewLine << sRTF_PARD << sRTF_PLAIN;
    //If we are only exporting an inline graphic/object then we
    //only need the its pFlyFmt for the duration of exporting it
    //for floating objects its a little more complex at the moment
    const SwFlyFrmFmt *pOldFlyFmt = pFlyFmt;
    pFlyFmt = &rFlyFrmFmt;

    {
        RTFSaveData aSaveData( *this, nStt, nEnd );
        Out_SwDoc( pCurPam );
    }

    if (!ExportAsInline(rFlyFrmFmt))
        Strm() << sRTF_PARD << SwRTFWriter::sNewLine;
//#i46098#:    else
        pFlyFmt = pOldFlyFmt;
}



void SwRTFWriter::OutRedline( xub_StrLen nCntntPos )
{
        const SwRedline *pCurRedline = 0;
        USHORT nCount = pDoc->GetRedlineTbl().Count();

        if (nCurRedline < nCount)
        {
            pCurRedline = pDoc->GetRedlineTbl()[nCurRedline];
            if(pCurRedline)
            {
                const SwPosition* pStartPos = pCurRedline->Start();
                const SwPosition* pEndPos = pStartPos == pCurRedline->GetPoint()
                                            ? pCurRedline->GetMark()
                                            : pCurRedline->GetPoint();

                USHORT nStart = pStartPos->nContent.GetIndex();
                USHORT nEnd = pEndPos->nContent.GetIndex();

                ULONG nCurPam  = pCurPam->GetPoint()->nNode.GetIndex();
                ULONG nStartIndex = pStartPos->nNode.GetIndex();
                ULONG nEndIndex = pEndPos->nNode.GetIndex();
                const String& rStr = pCurPam->GetNode()->GetTxtNode()->GetTxt();
                xub_StrLen nEnde = rStr.Len();

                bool bSpanRedline = (nCurPam >= nStartIndex) && (nCurPam <= nEndIndex) && (nStartIndex != nEndIndex);

                if ((bSpanRedline && nCntntPos == 0) ||
                    (nStartIndex == nCurPam && nStart == nCntntPos))
                {
                    // We are at the start of a redline just need to find out which type
                    Strm() << '{';
                    if(pCurRedline->GetType() == nsRedlineType_t::REDLINE_INSERT)
                    {
                        Strm() << sRTF_REVISED;
                        Strm() << sRTF_REVAUTH;
                        String sName = SW_MOD()->GetRedlineAuthor(pCurRedline->GetAuthor());
                        OutLong( pRedlAuthors->AddName(sName) );
                        Strm() << sRTF_REVDTTM;
                        OutLong( sw::ms::DateTime2DTTM(pCurRedline->GetTimeStamp()) );
                        Strm() << ' ';
                    }
                    else if(pCurRedline->GetType() == nsRedlineType_t::REDLINE_DELETE)
                    {
                        Strm() << sRTF_DELETED;
                        Strm() << sRTF_REVAUTHDEL;
                        String sDelName = SW_MOD()->GetRedlineAuthor(pCurRedline->GetAuthor());
                        OutLong( pRedlAuthors->AddName(sDelName) );
                        Strm() << sRTF_REVDTTMDEL;
                        OutLong( sw::ms::DateTime2DTTM(pCurRedline->GetTimeStamp()) );
                        Strm() << ' ';
                    }
                }

                // this is either then of the end of the node or the end of the redline
                // time to close off this one
                if( (bSpanRedline && nCntntPos == nEnde) ||
                    (nEndIndex == nCurPam && nEnd == nCntntPos) )
                {
                    Strm() << '}';
                }

                // We have come to the end of a redline move to the next one
                // and use resursion to see if another redline starts here
                if (nEndIndex == nCurPam && nEnd == nCntntPos)
                {
                    nCurRedline++;
                    OutRedline(nCntntPos);
                }
            }
        }
}

void SwRTFWriter::OutBookmarks( xub_StrLen nCntntPos )
{
    if (USHRT_MAX == nBkmkTabPos)
        return;

    const SwBookmark* pBookmark = pDoc->getBookmarks()[nBkmkTabPos];
    if (!pBookmark)
        return;

    // hole das aktuelle Bookmark
    const SwPosition* pStartPos = 0;
    const SwPosition* pEndPos = 0;

    if (pBookmark->GetOtherBookmarkPos())   // this bookmark spans text
    {
        // the start and endpoints are different
        SwPaM mPam(pBookmark->GetBookmarkPos(), *pBookmark->GetOtherBookmarkPos());
        pStartPos = mPam.Start();
        pEndPos = mPam.End();
    }
    else                            // this bookmark is a point
    {
        // so the start and endpoints are the same
        pStartPos = pEndPos = &pBookmark->GetBookmarkPos();
    }

    ASSERT(pStartPos && pEndPos, "Impossible");
    if (!(pStartPos && pEndPos))
        return;

    if (pStartPos->nNode.GetIndex() == pCurPam->GetPoint()->nNode.GetIndex() &&
        pStartPos->nContent.GetIndex() == nCntntPos)
    {
        // zur Zeit umspannt das SwBookmark keinen Bereich also kann
        // es hier vollstaendig ausgegeben werden.

        // erst die SWG spezifischen Daten:
        if (
             pBookmark->GetShortName().Len() ||
             pBookmark->GetKeyCode().GetCode()
           )
        {
            OutComment( *this, sRTF_BKMKKEY );
            OutULong( ( pBookmark->GetKeyCode().GetCode() |
                     pBookmark->GetKeyCode().GetModifier() ));
            if( !pBookmark->GetShortName().Len() )
                Strm() << "  " ;
            else
            {
                Strm() << ' ';
                OutRTF_AsByteString( *this, pBookmark->GetShortName(), eDefaultEncoding );
            }
            Strm() << '}';
        }
        OutComment( *this, sRTF_BKMKSTART ) << ' ';
        RTFOutFuncs::Out_String( Strm(), pBookmark->GetName(),
                                eDefaultEncoding, bWriteHelpFmt ) << '}';
    }

    if (pEndPos->nNode.GetIndex() == pCurPam->GetPoint()->nNode.GetIndex() &&
        pEndPos->nContent.GetIndex() == nCntntPos)
    {
        // zur Zeit umspannt das SwBookmark keinen Bereich also kann
        // es hier vollstaendig ausgegeben werden.

        // erst die SWG spezifischen Daten:
        if (
             pBookmark->GetShortName().Len() ||
             pBookmark->GetKeyCode().GetCode()
           )
        {
            OutComment( *this, sRTF_BKMKKEY );
            OutULong( ( pBookmark->GetKeyCode().GetCode() |
                     pBookmark->GetKeyCode().GetModifier() ));
            if( !pBookmark->GetShortName().Len() )
                Strm() << "  " ;
            else
            {
                Strm() << ' ';
                OutRTF_AsByteString( *this, pBookmark->GetShortName(), eDefaultEncoding );
            }
            Strm() << '}';
        }
        OutComment( *this, sRTF_BKMKEND ) << ' ';
        RTFOutFuncs::Out_String( Strm(), pBookmark->GetName(),
                                eDefaultEncoding, bWriteHelpFmt ) << '}';

        if( ++nBkmkTabPos >= pDoc->getBookmarks().Count() )
            nBkmkTabPos = USHRT_MAX;
        else
            pBookmark = pDoc->getBookmarks()[ nBkmkTabPos ];
    }
}

void SwRTFWriter::OutFlyFrm()
{
    USHORT n;

    if( !pFlyPos )
        return;

    // gebe alle freifliegenden Rahmen die sich auf den akt. Absatz
    // und evt. auf das aktuelle Zeichen beziehen, aus.

    // suche nach dem Anfang der FlyFrames
    for( n = 0; n < pFlyPos->Count() &&
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
                                rFont.GetCharSet(),
                                RES_CHRATR_FONT ) );
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
        const SwPageDesc& rPageDesc =
            const_cast<const SwDoc*>(pDoc)->GetPageDesc( n );

        Strm() << SwRTFWriter::sNewLine << '{' << sRTF_PGDSC;
        OutULong( n ) << sRTF_PGDSCUSE;
        OutULong( rPageDesc.ReadUseOn() );

        OutRTFPageDescription( rPageDesc, FALSE, FALSE );

        // suche den Folge-PageDescriptor:
        USHORT i = nSize;
        while( i  )
            if( rPageDesc.GetFollow() ==
                &const_cast<const SwDoc *>(pDoc)->GetPageDesc( --i ) )
                break;
        Strm() << sRTF_PGDSCNXT;
        OutULong( i ) << ' ';
        RTFOutFuncs::Out_String( Strm(), XlateFmtName( rPageDesc.GetName(), nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC ), eDefaultEncoding,
                    bWriteHelpFmt ) << ";}";
    }
    Strm() << '}' << SwRTFWriter::sNewLine;
    bOutPageDesc = bOutPageDescTbl = FALSE;
}

void SwRTFWriter::OutRTFBorder(const SvxBorderLine* aLine, const USHORT nSpace )
{
    // M.M. This function writes out border lines in RTF similar to what
    // WW8_BRC SwWW8Writer::TranslateBorderLine does in the winword filter
    // Eventually it would be nice if all this functionality was in the one place
    int nDistance = aLine->GetDistance();
    int nOutWidth = aLine->GetOutWidth();
    int nInWidth = aLine->GetInWidth();
    int nWidth = aLine->GetOutWidth();

    if(nDistance == 0)  // Single Line
        Strm() << sRTF_BRDRS;
    else                // Double Line
    {
        if(nOutWidth == nInWidth)
            Strm() << sRTF_BRDRDB;
        else if (nOutWidth > nInWidth)
            Strm() << sRTF_BRDRTNTHSG;
        else if (nOutWidth < nInWidth)
            Strm() << sRTF_BRDRTHTNSG;
    }
    Strm() << sRTF_BRDRW;
    OutULong(nWidth);

    Strm() << sRTF_BRSP;
    OutULong(nSpace);
}

void SwRTFWriter::OutRTFBorders(SvxBoxItem aBox)
{
    const SvxBorderLine *pLine = aBox.GetTop();
    if(pLine)
    {
        Strm() << sRTF_PGBRDRT;
        OutRTFBorder(pLine, aBox.GetDistance(BOX_LINE_TOP));
    }

    pLine = aBox.GetBottom();
    if(pLine)
    {
        Strm() << sRTF_PGBRDRB;
        OutRTFBorder(pLine, aBox.GetDistance(BOX_LINE_BOTTOM));
    }

    pLine = aBox.GetRight();
    if(pLine)
    {
        Strm() << sRTF_PGBRDRR;
        OutRTFBorder(pLine, aBox.GetDistance(BOX_LINE_LEFT));
    }

    pLine = aBox.GetLeft();
    if(pLine)
    {
        Strm() << sRTF_PGBRDRL;
        OutRTFBorder(pLine, aBox.GetDistance(BOX_LINE_RIGHT));
    }
}

void SwRTFWriter::OutRTFPageDescription( const SwPageDesc& rPgDsc,
                                            BOOL bWriteReset,
                                            BOOL bCheckForFirstPage )
{
    // jetzt noch den Teil fuer alle anderen Applikationen:
    const SwPageDesc *pSave = pAktPageDesc;
    bool bOldOut = bOutPageDesc;
    bool bOldHDFT = bOutLeftHeadFoot;

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

    SvxBoxItem aBox = pFmt->GetAttrSet().GetBox();
    OutRTFBorders(pFmt->GetAttrSet().GetBox());

    // falls es gesharte Heaer/Footer gibt, so gebe diese auch noch aus
    if( nsUseOnPage::PD_MIRROR & pAktPageDesc->GetUseOn() &&
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
                if( &const_cast<const SwDoc *>(pDoc)
                    ->GetPageDesc( --nPos ) == rPgDsc.GetPageDesc() )
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
                    bOutFmtAttr = true;
                    Strm() << sRTF_PAGE;
                }
            }
            else
            {
                switch( rBreak.GetBreak() )
                {
                    case SVX_BREAK_COLUMN_BEFORE:
                    case SVX_BREAK_COLUMN_AFTER:
                    case SVX_BREAK_COLUMN_BOTH:
                        break;
                    case SVX_BREAK_PAGE_BEFORE:
                        bOutFmtAttr = true;
                        Strm() << sRTF_PAGE;
                        break;
                    case SVX_BREAK_PAGE_AFTER:
                        OutComment(*this, sRTF_PGBRK, false) << "0}";
                        break;
                    case SVX_BREAK_PAGE_BOTH:
                        OutComment(*this, sRTF_PGBRK, false) << "1}";
                        break;
                    default:
                        break;
                }
            }
        }
    }
    bIgnoreNextPgBreak = false;
    return bPgDscWrite;
}


void SwRTFWriter::CheckEndNodeForSection( const SwNode& rNd )
{
    const SwSectionNode* pSectNd = rNd.StartOfSectionNode()->GetSectionNode();
    if( pSectNd /*&& CONTENT_SECTION == pSectNd->GetSection().GetType()*/ )
    {
        const SwSectionFmt* pSectFmt = pSectNd->GetSection().GetFmt();

        // diese Section hatte den akt. Abschnitt bestimmt
        // wer bestimmt den nachsten??
        SwNodeIndex aIdx( rNd, 1 );
        pSectNd = aIdx.GetNode().GetSectionNode();
        if( !( ( pSectNd || aIdx.GetNode().IsEndNode() &&
            0 != ( pSectNd = aIdx.GetNode().StartOfSectionNode()->GetSectionNode() ))
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
                                         : const_cast<const SwDoc *>(pDoc)
                                         ->GetPageDesc(0) ),
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

void GetRTFWriter( const String& rFltName, const String& rBaseURL, WriterRef& xRet )
{
    xRet = new SwRTFWriter( rFltName, rBaseURL );
}

short SwRTFWriter::GetCurrentPageDirection() const
{
    const SwFrmFmt  &rFmt = pAktPageDesc
        ? pAktPageDesc->GetMaster()
        : const_cast<const SwDoc *>(pDoc)
        ->GetPageDesc(0).GetMaster();
    const SvxFrameDirectionItem* pItem = &rFmt.GetFrmDir();

    if (!pItem)
    {
        pItem = (const SvxFrameDirectionItem*)
            &pDoc->GetAttrPool().GetDefaultItem(RES_FRAMEDIR);
    }
    return pItem->GetValue();
}

short SwRTFWriter::TrueFrameDirection(const SwFrmFmt &rFlyFmt) const
{
    const SwFrmFmt *pFlyFmt2 = &rFlyFmt;
    const SvxFrameDirectionItem* pItem = 0;
    while (pFlyFmt2)
    {
        pItem = &pFlyFmt2->GetFrmDir();
        if (FRMDIR_ENVIRONMENT == pItem->GetValue())
        {
            pItem = 0;
            const SwFmtAnchor* pAnchor = &pFlyFmt2->GetAnchor();
            if( FLY_PAGE != pAnchor->GetAnchorId() &&
                pAnchor->GetCntntAnchor() )
            {
                pFlyFmt2 = pAnchor->GetCntntAnchor()->nNode.
                                    GetNode().GetFlyFmt();
            }
            else
                pFlyFmt2 = 0;
        }
        else
            pFlyFmt2 = 0;
    }

    short nRet;
    if (pItem)
        nRet = pItem->GetValue();
    else
        nRet = GetCurrentPageDirection();

    ASSERT(nRet != FRMDIR_ENVIRONMENT, "leaving with environment direction");
    return nRet;
}

/* vi:set tabstop=4 shiftwidth=4 expandtab: */
