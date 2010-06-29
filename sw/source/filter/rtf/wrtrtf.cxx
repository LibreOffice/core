/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#include <stdlib.h>
#include <hintids.hxx>

#include <comphelper/string.hxx>
#include <tools/stream.hxx>
#include <tools/datetime.hxx>
#include <unotools/fontcvt.hxx>
#include <rtl/tencinfo.h>
#include <svtools/rtfkeywd.hxx>
#include <svtools/rtfout.hxx>
#include <editeng/paperinf.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/brshitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/udlnitem.hxx>
#include <fmtpdsc.hxx>
#include <fmtcntnt.hxx>
#include <fmtfsize.hxx>
#include <fmthdft.hxx>
#include <frmatr.hxx>
#include <fmtanchr.hxx>
#include <docary.hxx>
#include <pam.hxx>
#include <doc.hxx>
#include <paratr.hxx>
#include <fldbas.hxx>
#include <ndtxt.hxx>
#include <wrtrtf.hxx>
#include <flypos.hxx>
#include <IMark.hxx>
#include <pagedesc.hxx>     // fuer SwPageDesc...
#include <ftninfo.hxx>
#include <charfmt.hxx>
#include <SwStyleNameMapper.hxx>
#include <section.hxx>
#include <swtable.hxx>      // fuer SwPageDesc ...
#include <swmodule.hxx>
#include <swerror.h>
#include <mdiexp.hxx>       // ...Percent()
#include <statstr.hrc>      // ResId fuer Statusleiste
#include <docsh.hxx>

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
    // enable non-standard tags for cut and paste
    bNonStandard = '\0' == rFltName.GetChar( 0 );
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
    nBkmkTabPos = -1;
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
    nBkmkTabPos = bWriteAll ? FindPos_Bkmk( *pCurPam->GetPoint() ) : -1;

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
                    //( rCNd.IsTxtNode() && NO_NUMBERING !=     //#outline level,removed by zhaojianwei
                    //((SwTxtNode&)rCNd).GetTxtColl()->GetOutlineLevel() ))
                    ( rCNd.IsTxtNode() &&                       //->add by zhaojianwei
                        ((SwTxtNode&)rCNd).GetTxtColl()->IsAssignedToListLevelOfOutlineStyle()))//<-end,zhaojianwei
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
    Strm() << '{' << OOO_STRING_SVTOOLS_RTF_RTF << '1'
        << OOO_STRING_SVTOOLS_RTF_ANSI;
    if( bWriteAll )
    {
        Strm() << OOO_STRING_SVTOOLS_RTF_DEFF;
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
        Strm() << OOO_STRING_SVTOOLS_RTF_DEFTAB;
        OutLong( rTabs[0].GetTabPos() );
        if ( !pDoc->get(IDocumentSettingAccess::USE_VIRTUAL_DEVICE) )
            Strm() << OOO_STRING_SVTOOLS_RTF_LYTPRTMET;
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
                    OutComment( *this, OOO_STRING_SVTOOLS_RTF_PGDSCNO, FALSE );
                    OutULong( nPosInDoc ) << '}';
                }
            }
        }
        const SwPageDesc& rPageDesc = pSttPgDsc ? *pSttPgDsc->GetPageDesc()
            : const_cast<const SwDoc *>(pDoc)->GetPageDesc( 0 );
        const SwFrmFmt &rFmtPage = rPageDesc.GetMaster();

        {
            if( rPageDesc.GetLandscape() )
                Strm() << OOO_STRING_SVTOOLS_RTF_LANDSCAPE;

            const SwFmtFrmSize& rSz = rFmtPage.GetFrmSize();
            // Clipboard-Dokument wird immer ohne Drucker angelegt, so ist
            // der Std.PageDesc immer aug LONG_MAX !! Mappe dann auf DIN A4
            if( LONG_MAX == rSz.GetHeight() || LONG_MAX == rSz.GetWidth() )
            {
                Strm() << OOO_STRING_SVTOOLS_RTF_PAPERH;
                Size a4 = SvxPaperInfo::GetPaperSize(PAPER_A4);
                OutULong( a4.Height() ) << OOO_STRING_SVTOOLS_RTF_PAPERW;
                OutULong( a4.Width() );
            }
            else
            {
                Strm() << OOO_STRING_SVTOOLS_RTF_PAPERH;
                OutULong( rSz.GetHeight() ) << OOO_STRING_SVTOOLS_RTF_PAPERW;
                OutULong( rSz.GetWidth() );
            }
        }

        {
            const SvxLRSpaceItem& rLR = rFmtPage.GetLRSpace();
            Strm() << OOO_STRING_SVTOOLS_RTF_MARGL;
            OutLong( rLR.GetLeft() ) << OOO_STRING_SVTOOLS_RTF_MARGR;
            OutLong( rLR.GetRight() );
        }

        {
            const SvxULSpaceItem& rUL = rFmtPage.GetULSpace();
            Strm() << OOO_STRING_SVTOOLS_RTF_MARGT;
            OutLong( rUL.GetUpper() ) << OOO_STRING_SVTOOLS_RTF_MARGB;
            OutLong( rUL.GetLower() );
        }

        Strm() << OOO_STRING_SVTOOLS_RTF_SECTD << OOO_STRING_SVTOOLS_RTF_SBKNONE;
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
                            ? OOO_STRING_SVTOOLS_RTF_ENDDOC
                            : OOO_STRING_SVTOOLS_RTF_FTNBJ;
        Strm() << pOut << OOO_STRING_SVTOOLS_RTF_FTNSTART;
        OutLong( rFtnInfo.nFtnOffset + 1 );

        switch( rFtnInfo.eNum )
        {
        case FTNNUM_PAGE:       pOut = OOO_STRING_SVTOOLS_RTF_FTNRSTPG; break;
        case FTNNUM_DOC:        pOut = OOO_STRING_SVTOOLS_RTF_FTNRSTCONT;   break;
//      case FTNNUM_CHAPTER:
        default:                pOut = OOO_STRING_SVTOOLS_RTF_FTNRESTART;   break;
        }
        Strm() << pOut;

        switch( rFtnInfo.aFmt.GetNumberingType() )
        {
        case SVX_NUM_CHARS_LOWER_LETTER:
        case SVX_NUM_CHARS_LOWER_LETTER_N:  pOut = OOO_STRING_SVTOOLS_RTF_FTNNALC;  break;
        case SVX_NUM_CHARS_UPPER_LETTER:
        case SVX_NUM_CHARS_UPPER_LETTER_N:  pOut = OOO_STRING_SVTOOLS_RTF_FTNNAUC;  break;
        case SVX_NUM_ROMAN_LOWER:           pOut = OOO_STRING_SVTOOLS_RTF_FTNNRLC;  break;
        case SVX_NUM_ROMAN_UPPER:           pOut = OOO_STRING_SVTOOLS_RTF_FTNNRUC;  break;
        case SVX_NUM_CHAR_SPECIAL:          pOut = OOO_STRING_SVTOOLS_RTF_FTNNCHI;  break;
//      case SVX_NUM_ARABIC:
        default:                    pOut = OOO_STRING_SVTOOLS_RTF_FTNNAR;       break;
        }
        Strm() << pOut;


        const SwEndNoteInfo& rEndNoteInfo = pDoc->GetEndNoteInfo();

        Strm() << OOO_STRING_SVTOOLS_RTF_AENDDOC << OOO_STRING_SVTOOLS_RTF_AFTNRSTCONT
               << OOO_STRING_SVTOOLS_RTF_AFTNSTART;
        OutLong( rEndNoteInfo.nFtnOffset + 1 );

        switch( rEndNoteInfo.aFmt.GetNumberingType() )
        {
        case SVX_NUM_CHARS_LOWER_LETTER:
        case SVX_NUM_CHARS_LOWER_LETTER_N:  pOut = OOO_STRING_SVTOOLS_RTF_AFTNNALC; break;
        case SVX_NUM_CHARS_UPPER_LETTER:
        case SVX_NUM_CHARS_UPPER_LETTER_N:  pOut = OOO_STRING_SVTOOLS_RTF_AFTNNAUC; break;
        case SVX_NUM_ROMAN_LOWER:           pOut = OOO_STRING_SVTOOLS_RTF_AFTNNRLC; break;
        case SVX_NUM_ROMAN_UPPER:           pOut = OOO_STRING_SVTOOLS_RTF_AFTNNRUC; break;
        case SVX_NUM_CHAR_SPECIAL:          pOut = OOO_STRING_SVTOOLS_RTF_AFTNNCHI; break;
//      case SVX_NUM_ARABIC:
        default:                    pOut = OOO_STRING_SVTOOLS_RTF_AFTNNAR;  break;
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
                Strm() << '{' << OOO_STRING_SVTOOLS_RTF_FIELD;
                OutComment( *this, OOO_STRING_SVTOOLS_RTF_FLDINST ) << " DATA ";
                SwDBData aData = pDoc->GetDBData();
                String sOut(aData.sDataSource);
                sOut += DB_DELIM;
                sOut += (String)aData.sCommand;
                RTFOutFuncs::Out_String( Strm(), sOut,
                                        eDefaultEncoding, bWriteHelpFmt );
                Strm() << "}{" << OOO_STRING_SVTOOLS_RTF_FLDRSLT << " }}";
                break;
            }
    }

    pAttrSet = 0;

    Strm() << SwRTFWriter::sNewLine;        // ein Trenner
}

void SwRTFWriter::OutInfoDateTime( const sal_Char* i_pStr,
    const util::DateTime& i_rDT )
{
    Strm() << '{' << i_pStr << OOO_STRING_SVTOOLS_RTF_YR;
    OutLong( Strm(), i_rDT.Year ) << OOO_STRING_SVTOOLS_RTF_MO;
    OutLong( Strm(), i_rDT.Month ) << OOO_STRING_SVTOOLS_RTF_DY;
    OutLong( Strm(), i_rDT.Day ) << OOO_STRING_SVTOOLS_RTF_HR;
    OutLong( Strm(), i_rDT.Hours ) << OOO_STRING_SVTOOLS_RTF_MIN;
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
            Strm() << '{' << OOO_STRING_SVTOOLS_RTF_UPR;

        Strm() << '{' << pToken << ' ';
        OutRTF_AsByteString(*this, rContent, eDefaultEncoding);
        Strm() << '}';

        if (bNeedUnicodeWrapper)
        {
            OutComment(*this, OOO_STRING_SVTOOLS_RTF_UD);
            Strm() << '{' << pToken << ' ';
            RTFOutFuncs::Out_String(Strm(), rContent, eDefaultEncoding,
                bWriteHelpFmt);
            Strm() << "}}}";
        }

    }
}

void SwRTFWriter::OutDocInfoStat()
{
    Strm() << '{' << OOO_STRING_SVTOOLS_RTF_INFO;

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
        OutUnicodeSafeRecord(OOO_STRING_SVTOOLS_RTF_TITLE,    xDocProps->getTitle());
        OutUnicodeSafeRecord(OOO_STRING_SVTOOLS_RTF_SUBJECT,  xDocProps->getSubject());

        OutUnicodeSafeRecord(OOO_STRING_SVTOOLS_RTF_KEYWORDS,
        ::comphelper::string::convertCommaSeparated(xDocProps->getKeywords()));
        OutUnicodeSafeRecord(OOO_STRING_SVTOOLS_RTF_DOCCOMM,  xDocProps->getDescription());

        OutUnicodeSafeRecord(OOO_STRING_SVTOOLS_RTF_AUTHOR,   xDocProps->getAuthor() );
        OutInfoDateTime(OOO_STRING_SVTOOLS_RTF_CREATIM,       xDocProps->getCreationDate());

        OutUnicodeSafeRecord(OOO_STRING_SVTOOLS_RTF_AUTHOR,   xDocProps->getModifiedBy() );
        OutInfoDateTime(OOO_STRING_SVTOOLS_RTF_REVTIM,        xDocProps->getModificationDate());

        OutInfoDateTime(OOO_STRING_SVTOOLS_RTF_PRINTIM,       xDocProps->getPrintDate());

    }

    // fuer interne Zwecke - Versionsnummer rausschreiben
    Strm() << '{' << OOO_STRING_SVTOOLS_RTF_COMMENT << " StarWriter}{" << OOO_STRING_SVTOOLS_RTF_VERN;
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

        const SvxOverlineItem* pOver = (const SvxOverlineItem*)GetDfltAttr( RES_CHRATR_OVERLINE );
        InsColor( *pColTbl, pOver->GetColor() );
        nMaxItem = rPool.GetItemCount(RES_CHRATR_OVERLINE);
        for( n = 0; n < nMaxItem;n++)
        {
            if( 0 != (pOver = (const SvxOverlineItem*)rPool.GetItem( RES_CHRATR_OVERLINE, n ) ) )
                InsColor( *pColTbl, pOver->GetColor() );

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
    Strm() << SwRTFWriter::sNewLine << '{' << OOO_STRING_SVTOOLS_RTF_COLORTBL;

    for( n = 0; n < pColTbl->Count(); n++ )
    {
        const Color& rCol = (*pColTbl)[ n ];
        if( n || COL_AUTO != rCol.GetColor() )
        {
            Strm() << OOO_STRING_SVTOOLS_RTF_RED;
            OutULong( rCol.GetRed() ) << OOO_STRING_SVTOOLS_RTF_GREEN;
            OutULong( rCol.GetGreen() ) << OOO_STRING_SVTOOLS_RTF_BLUE;
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
    rWrt.Strm() << '{' << OOO_STRING_SVTOOLS_RTF_F;

    const char* pStr = OOO_STRING_SVTOOLS_RTF_FNIL;
    switch (rFont.GetFamily())
    {
        case FAMILY_ROMAN:
            pStr = OOO_STRING_SVTOOLS_RTF_FROMAN;
            break;
        case FAMILY_SWISS:
            pStr = OOO_STRING_SVTOOLS_RTF_FSWISS;
            break;
        case FAMILY_MODERN:
            pStr = OOO_STRING_SVTOOLS_RTF_FMODERN;
            break;
        case FAMILY_SCRIPT:
            pStr = OOO_STRING_SVTOOLS_RTF_FSCRIPT;
            break;
        case FAMILY_DECORATIVE:
            pStr = OOO_STRING_SVTOOLS_RTF_FDECOR;
            break;
        default:
            break;
    }
    rWrt.OutULong(nNo) << pStr << OOO_STRING_SVTOOLS_RTF_FPRQ;

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

    rWrt.Strm() << OOO_STRING_SVTOOLS_RTF_FCHARSET;
    rWrt.OutULong( nChSet );
    rWrt.Strm() << ' ';
    if (aRes.HasDistinctSecondary())
    {
        RTFOutFuncs::Out_Fontname(rWrt.Strm(), aRes.msPrimary, eChrSet,
            rWrt.bWriteHelpFmt);
        OutComment(rWrt, OOO_STRING_SVTOOLS_RTF_FALT) << ' ';
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

    Strm() << SwRTFWriter::sNewLine << '{' << OOO_STRING_SVTOOLS_RTF_FONTTBL;
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

    rRTFWrt.Strm() << '{' << OOO_STRING_SVTOOLS_RTF_IGNORE << OOO_STRING_SVTOOLS_RTF_REVTBL << ' ';
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
    Strm() << SwRTFWriter::sNewLine << '{' << OOO_STRING_SVTOOLS_RTF_STYLESHEET;

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
                    Strm() << OOO_STRING_SVTOOLS_RTF_SBASEDON;
                    OutULong( nBasedOn );
                    break;
                }

        if( pColl == &pColl->GetNextTxtFmtColl() )
        {
            Strm() << OOO_STRING_SVTOOLS_RTF_SNEXT;
            OutULong( n );
        }
        else
            // suche die Id vom "Naechsten" Format
            for( USHORT nNext=1; nNext < nArrLen; nNext++)
                if( (*pDoc->GetTxtFmtColls())[ nNext ] ==
                        &pColl->GetNextTxtFmtColl() )
                {
                    // die Ableitung vom Format
                    Strm() << OOO_STRING_SVTOOLS_RTF_SNEXT;
                    OutULong( nNext );
                    break;
                }

        //if( NO_NUMBERING != pColl->GetOutlineLevel() )//#outline level,zhaojianwei
        if(pColl->IsAssignedToListLevelOfOutlineStyle())//<-end,zhaojianwei
        {
            Strm() << '{' << OOO_STRING_SVTOOLS_RTF_IGNORE << OOO_STRING_SVTOOLS_RTF_SOUTLVL;
            //OutULong( pColl->GetOutlineLevel() ) << '}';//#outline level,zhaojianwei
            OutULong( pColl->GetAssignedOutlineStyleLevel() ) << '}';//<-end,zhaojianwei
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
                    Strm() << OOO_STRING_SVTOOLS_RTF_SBASEDON;
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
    return rFlyFrmFmt.GetAnchor().GetAnchorId() == FLY_AS_CHAR;
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
        Strm() << SwRTFWriter::sNewLine << OOO_STRING_SVTOOLS_RTF_PARD << OOO_STRING_SVTOOLS_RTF_PLAIN;
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
        Strm() << OOO_STRING_SVTOOLS_RTF_PARD << SwRTFWriter::sNewLine;
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
                        Strm() << OOO_STRING_SVTOOLS_RTF_REVISED;
                        Strm() << OOO_STRING_SVTOOLS_RTF_REVAUTH;
                        String sName = SW_MOD()->GetRedlineAuthor(pCurRedline->GetAuthor());
                        OutLong( pRedlAuthors->AddName(sName) );
                        Strm() << OOO_STRING_SVTOOLS_RTF_REVDTTM;
                        OutLong( sw::ms::DateTime2DTTM(pCurRedline->GetTimeStamp()) );
                        Strm() << ' ';
                    }
                    else if(pCurRedline->GetType() == nsRedlineType_t::REDLINE_DELETE)
                    {
                        Strm() << OOO_STRING_SVTOOLS_RTF_DELETED;
                        Strm() << OOO_STRING_SVTOOLS_RTF_REVAUTHDEL;
                        String sDelName = SW_MOD()->GetRedlineAuthor(pCurRedline->GetAuthor());
                        OutLong( pRedlAuthors->AddName(sDelName) );
                        Strm() << OOO_STRING_SVTOOLS_RTF_REVDTTMDEL;
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

void SwRTFWriter::OutBookmarks( xub_StrLen nCntntPos)
{
    IDocumentMarkAccess* const pMarkAccess = pDoc->getIDocumentMarkAccess();
    if (-1 == nBkmkTabPos)
        return;

    const ::sw::mark::IMark* pBookmark = (pMarkAccess->getMarksBegin() + nBkmkTabPos)->get();
    if(!pBookmark)
        return;

    const SwPosition* pStartPos = &pBookmark->GetMarkStart();
    const SwPosition* pEndPos = &pBookmark->GetMarkEnd();

    ASSERT(pStartPos && pEndPos, "Impossible");
    if (!(pStartPos && pEndPos))
        return;

    if (pStartPos->nNode.GetIndex() == pCurPam->GetPoint()->nNode.GetIndex() &&
        pStartPos->nContent.GetIndex() == nCntntPos)
    {
        // zur Zeit umspannt das SwBookmark keinen Bereich also kann
        // es hier vollstaendig ausgegeben werden.

        // erst die SWG spezifischen Daten:
        const ::sw::mark::IBookmark* const pAsBookmark = dynamic_cast< const ::sw::mark::IBookmark* >(pBookmark);
        if(pAsBookmark && (pAsBookmark->GetShortName().getLength() || pAsBookmark->GetKeyCode().GetCode()))
        {
            OutComment( *this, OOO_STRING_SVTOOLS_RTF_BKMKKEY );
            OutULong( ( pAsBookmark->GetKeyCode().GetCode() |
                     pAsBookmark->GetKeyCode().GetModifier() ));
            if( !pAsBookmark->GetShortName().getLength() )
                Strm() << "  " ;
            else
            {
                Strm() << ' ';
                OutRTF_AsByteString( *this, pAsBookmark->GetShortName(), eDefaultEncoding );
            }
            Strm() << '}';
        }
        OutComment( *this, OOO_STRING_SVTOOLS_RTF_BKMKSTART ) << ' ';
        RTFOutFuncs::Out_String( Strm(), pBookmark->GetName(),
                                eDefaultEncoding, bWriteHelpFmt ) << '}';
    }

    if (pEndPos->nNode.GetIndex() == pCurPam->GetPoint()->nNode.GetIndex() &&
        pEndPos->nContent.GetIndex() == nCntntPos)
    {
        // zur Zeit umspannt das SwBookmark keinen Bereich also kann
        // es hier vollstaendig ausgegeben werden.

        // erst die SWG spezifischen Daten:
        const ::sw::mark::IBookmark* const pAsBookmark = dynamic_cast< const ::sw::mark::IBookmark* >(pBookmark);
        if(pAsBookmark && (pAsBookmark->GetShortName().getLength() || pAsBookmark->GetKeyCode().GetCode()))
        {
            OutComment( *this, OOO_STRING_SVTOOLS_RTF_BKMKKEY );
            OutULong( ( pAsBookmark->GetKeyCode().GetCode() |
                     pAsBookmark->GetKeyCode().GetModifier() ));
            if( !pAsBookmark->GetShortName().getLength() )
                Strm() << "  " ;
            else
            {
                Strm() << ' ';
                OutRTF_AsByteString( *this, pAsBookmark->GetShortName(), eDefaultEncoding );
            }
            Strm() << '}';
        }
        OutComment( *this, OOO_STRING_SVTOOLS_RTF_BKMKEND ) << ' ';

        {
            ::rtl::OUString aEmpty;
            ::rtl::OUString & rBookmarkName = aEmpty;

            if (pAsBookmark)
                rBookmarkName = pAsBookmark->GetName();

            RTFOutFuncs::Out_String( Strm(), rBookmarkName,
                                eDefaultEncoding, bWriteHelpFmt ) << '}';
        }

        if(++nBkmkTabPos >= pMarkAccess->getMarksCount())
            nBkmkTabPos = -1;
        else
            pBookmark = (pMarkAccess->getMarksBegin() + nBkmkTabPos)->get();
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
    OutComment( *this, OOO_STRING_SVTOOLS_RTF_PGDSCTBL );
    for( USHORT n = 0; n < nSize; ++n )
    {
        const SwPageDesc& rPageDesc =
            const_cast<const SwDoc*>(pDoc)->GetPageDesc( n );

        Strm() << SwRTFWriter::sNewLine << '{' << OOO_STRING_SVTOOLS_RTF_PGDSC;
        OutULong( n ) << OOO_STRING_SVTOOLS_RTF_PGDSCUSE;
        OutULong( rPageDesc.ReadUseOn() );

        OutRTFPageDescription( rPageDesc, FALSE, FALSE );

        // suche den Folge-PageDescriptor:
        USHORT i = nSize;
        while( i  )
            if( rPageDesc.GetFollow() ==
                &const_cast<const SwDoc *>(pDoc)->GetPageDesc( --i ) )
                break;
        Strm() << OOO_STRING_SVTOOLS_RTF_PGDSCNXT;
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
        Strm() << OOO_STRING_SVTOOLS_RTF_BRDRS;
    else                // Double Line
    {
        if(nOutWidth == nInWidth)
            Strm() << OOO_STRING_SVTOOLS_RTF_BRDRDB;
        else if (nOutWidth > nInWidth)
            Strm() << OOO_STRING_SVTOOLS_RTF_BRDRTNTHSG;
        else if (nOutWidth < nInWidth)
            Strm() << OOO_STRING_SVTOOLS_RTF_BRDRTHTNSG;
    }
    Strm() << OOO_STRING_SVTOOLS_RTF_BRDRW;
    OutULong(nWidth);

    Strm() << OOO_STRING_SVTOOLS_RTF_BRSP;
    OutULong(nSpace);
}

void SwRTFWriter::OutRTFBorders(SvxBoxItem aBox)
{
    const SvxBorderLine *pLine = aBox.GetTop();
    if(pLine)
    {
        Strm() << OOO_STRING_SVTOOLS_RTF_PGBRDRT;
        OutRTFBorder(pLine, aBox.GetDistance(BOX_LINE_TOP));
    }

    pLine = aBox.GetBottom();
    if(pLine)
    {
        Strm() << OOO_STRING_SVTOOLS_RTF_PGBRDRB;
        OutRTFBorder(pLine, aBox.GetDistance(BOX_LINE_BOTTOM));
    }

    pLine = aBox.GetRight();
    if(pLine)
    {
        Strm() << OOO_STRING_SVTOOLS_RTF_PGBRDRR;
        OutRTFBorder(pLine, aBox.GetDistance(BOX_LINE_LEFT));
    }

    pLine = aBox.GetLeft();
    if(pLine)
    {
        Strm() << OOO_STRING_SVTOOLS_RTF_PGBRDRL;
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
            Strm() << OOO_STRING_SVTOOLS_RTF_SECTD << OOO_STRING_SVTOOLS_RTF_SBKNONE;
        else
            Strm() << OOO_STRING_SVTOOLS_RTF_SECT << OOO_STRING_SVTOOLS_RTF_SECTD;
    }

    if( pAktPageDesc->GetLandscape() )
        Strm() << OOO_STRING_SVTOOLS_RTF_LNDSCPSXN;

    const SwFmt *pFmt = &pAktPageDesc->GetMaster(); //GetLeft();
    OutRTF_SwFmt( *this, *pFmt );

    SvxBoxItem aBox = pFmt->GetAttrSet().GetBox();
    OutRTFBorders(pFmt->GetAttrSet().GetBox());

    // falls es gesharte Heaer/Footer gibt, so gebe diese auch noch aus
    if (
        (nsUseOnPage::PD_MIRROR & pAktPageDesc->GetUseOn()) &&
        (!pAktPageDesc->IsFooterShared() || !pAktPageDesc->IsHeaderShared())
       )
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
        Strm() << OOO_STRING_SVTOOLS_RTF_TITLEPG;

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
                    OutComment( *this, OOO_STRING_SVTOOLS_RTF_PGDSCNO, FALSE );
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
                    Strm() << OOO_STRING_SVTOOLS_RTF_PAGE;
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
                        Strm() << OOO_STRING_SVTOOLS_RTF_PAGE;
                        break;
                    case SVX_BREAK_PAGE_AFTER:
                        OutComment(*this, OOO_STRING_SVTOOLS_RTF_PGBRK, false) << "0}";
                        break;
                    case SVX_BREAK_PAGE_BOTH:
                        OutComment(*this, OOO_STRING_SVTOOLS_RTF_PGBRK, false) << "1}";
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
        if( !( ( pSectNd || (aIdx.GetNode().IsEndNode() &&
            0 != ( pSectNd = aIdx.GetNode().StartOfSectionNode()->GetSectionNode() )) )
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
                if (! bOutPageDesc)
                {
                    Strm() << OOO_STRING_SVTOOLS_RTF_SECT << OOO_STRING_SVTOOLS_RTF_SECTD << OOO_STRING_SVTOOLS_RTF_SBKNONE;
                    OutRTFPageDescription( ( pAktPageDesc
                                            ? *pAktPageDesc
                                            : const_cast<const SwDoc *>(pDoc)
                                            ->GetPageDesc(0) ),
                                          FALSE, TRUE );
                    Strm() << SwRTFWriter::sNewLine;
                }
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

extern "C" SAL_DLLPUBLIC_EXPORT void SAL_CALL ExportRTF( const String& rFltName, const String& rBaseURL, WriterRef& xRet )
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
            if ((FLY_AT_PAGE != pAnchor->GetAnchorId()) &&
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
