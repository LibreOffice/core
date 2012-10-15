/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <hintids.hxx>

#include <stack>

#include <tools/errinf.hxx>
#include <tools/stream.hxx>
#include <tools/helpers.hxx>
#include <svl/itemiter.hxx>
#include <svtools/rtftoken.h>
#include <svtools/miscopt.hxx>
#include <svl/intitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/escpitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/hyznitem.hxx>
#include <fmtpdsc.hxx>
#include <fmtfld.hxx>
#include <fmthdft.hxx>
#include <fmtcntnt.hxx>
#include <txtftn.hxx>
#include <fmtclds.hxx>
#include <fmtftn.hxx>
#include <fmtfsize.hxx>
#include <fmtflcnt.hxx>
#include <fmtanchr.hxx>
#include <frmatr.hxx>
#include <docstat.hxx>
#include <swtable.hxx>
#include <shellio.hxx>
#include <swtypes.hxx>
#include <ndtxt.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <pam.hxx>
#include <mdiexp.hxx>           // ...Percent()
#include <swparrtf.hxx>
#include <charfmt.hxx>
#include <pagedesc.hxx>
#include <ftninfo.hxx>
#include <docufld.hxx>
#include <flddat.hxx>
#include <fltini.hxx>
#include <fchrfmt.hxx>
#include <paratr.hxx>
#include <section.hxx>
#include <fmtclbl.hxx>
#include <viewsh.hxx>
#include <shellres.hxx>
#include <hfspacingitem.hxx>
#include <tox.hxx>
#include <swerror.h>
#include <cmdid.h>
#include <statstr.hrc>          // ResId fuer Statusleiste
#include <SwStyleNameMapper.hxx>
#include <tblsel.hxx>           // SwSelBoxes

#include <docsh.hxx>
#include <fmtlsplt.hxx> // SwLayoutSplit
#include <editeng/keepitem.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdorect.hxx>


#include <fmtsrnd.hxx>
#include <fmtfollowtextflow.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <editeng/opaqitem.hxx>
#include "svx/svdograf.hxx"
#include <svx/xflclit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/svdoutl.hxx>
#include <unotools/streamwrap.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/paperinf.hxx>

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/range/b2drange.hxx>

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XExporter.hpp>


using namespace ::com::sun::star;


// einige Hilfs-Funktionen
// char
inline const SvxFontHeightItem& GetSize(const SfxItemSet& rSet,sal_Bool bInP=sal_True)
    { return (const SvxFontHeightItem&)rSet.Get( RES_CHRATR_FONTSIZE,bInP); }
inline const SvxLRSpaceItem& GetLRSpace(const SfxItemSet& rSet,sal_Bool bInP=sal_True)
    { return (const SvxLRSpaceItem&)rSet.Get( RES_LR_SPACE,bInP); }


/// Glue class to call RtfImport as an internal filter, needed by copy&paste support.
class SwRTFReader : public Reader
{
    virtual sal_uLong Read( SwDoc &, const String& rBaseURL, SwPaM &,const String &);
};

sal_uLong SwRTFReader::Read( SwDoc &rDoc, const String& /*rBaseURL*/, SwPaM& /*rPam*/, const String &)
{
    if (!pStrm)
        return ERR_SWG_READ_ERROR;

    SwDocShell *pDocShell(rDoc.GetDocShell());
    uno::Reference<lang::XMultiServiceFactory> xMultiServiceFactory(comphelper::getProcessServiceFactory());
    uno::Reference<uno::XInterface> xInterface(xMultiServiceFactory->createInstance(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Writer.RtfFilter"))), uno::UNO_QUERY_THROW);

    uno::Reference<document::XImporter> xImporter(xInterface, uno::UNO_QUERY_THROW);
    uno::Reference<lang::XComponent> xDstDoc(pDocShell->GetModel(), uno::UNO_QUERY_THROW);
    xImporter->setTargetDocument(xDstDoc);

    uno::Reference<document::XFilter> xFilter(xInterface, uno::UNO_QUERY_THROW);
    uno::Sequence<beans::PropertyValue> aDescriptor(2);
    aDescriptor[0].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("InputStream"));
    uno::Reference<io::XStream> xStream(new utl::OStreamWrapper(*pStrm));
    aDescriptor[0].Value <<= xStream;
    aDescriptor[1].Name = "IsNewDoc";
    aDescriptor[1].Value <<= sal_False;
    xFilter->filter(aDescriptor);

    return 0;
}

extern "C" SAL_DLLPUBLIC_EXPORT Reader* SAL_CALL ImportRTF()
{
    // Use the old rtf importer by default for paste, till the new one supports
    // undo stack and PaM.
    SvtMiscOptions aMiscOptions;
    if (!aMiscOptions.IsExperimentalMode())
        return new RtfReader();
    return new SwRTFReader();
}

// Aufruf fuer die allg. Reader-Schnittstelle
sal_uLong RtfReader::Read( SwDoc &rDoc, const String& rBaseURL, SwPaM &rPam, const String &)
{
    if( !pStrm )
    {
        OSL_FAIL( "RTF-Read ohne Stream" );
        return ERR_SWG_READ_ERROR;
    }

    if( !bInsertMode )
    {
        // MIB 27.09.96: Umrandung uns Abstaende aus Frm-Vorlagen entf.
        Reader::ResetFrmFmts( rDoc );
    }

    sal_uLong nRet = 0;
    SwDocShell *pDocShell(rDoc.GetDocShell());
    OSL_ENSURE(pDocShell, "no SwDocShell");
    uno::Reference<document::XDocumentProperties> xDocProps;
    if (pDocShell) {
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
            pDocShell->GetModel(), uno::UNO_QUERY_THROW);
        xDocProps.set(xDPS->getDocumentProperties());
    }

    SvParserRef xParser = new SwRTFParser( &rDoc, xDocProps,
                                rPam, *pStrm, rBaseURL, !bInsertMode );
    SvParserState eState = xParser->CallParser();
    if( SVPAR_PENDING != eState && SVPAR_ACCEPTED != eState )
    {
        String sErr( String::CreateFromInt32( xParser->GetLineNr() ));
        sErr += ',';
        sErr += String::CreateFromInt32( xParser->GetLinePos() );

        nRet = *new StringErrorInfo( ERR_FORMAT_ROWCOL, sErr,
                                    ERRCODE_BUTTON_OK | ERRCODE_MSG_ERROR );
    }


    return nRet;
}

sal_uLong RtfReader::Read(SvStream* pStream, SwDoc& rDoc, const String& rBaseURL, SwPaM& rPam)
{
    pStrm = pStream;
    return Read(rDoc, rBaseURL, rPam, rBaseURL);
}

SwRTFParser::SwRTFParser(SwDoc* pD,
        uno::Reference<document::XDocumentProperties> i_xDocProps,
        const SwPaM& rCrsr, SvStream& rIn, const String& rBaseURL,
        int bReadNewDoc) :
    SvxRTFParser(pD->GetAttrPool(), rIn, i_xDocProps, bReadNewDoc),
    maParaStyleMapper(*pD),
    maCharStyleMapper(*pD),
    maSegments(*this),
    maInsertedTables(*pD),
    mpBookmarkStart(0),
    mpRedlineStack(0),
    pGrfAttrSet(0),
    pTableNode(0),
    pOldTblNd(0),
    pSttNdIdx(0),
    pRegionEndIdx(0),
    pDoc(pD),
    pRelNumRule(new SwRelNumRuleSpaces(*pD, static_cast< sal_Bool >(bReadNewDoc))),
    pRedlineInsert(0),
    pRedlineDelete(0),
    sBaseURL( rBaseURL ),
    nAktPageDesc(0),
    nAktFirstPageDesc(0),
    m_nCurrentBox(0),
    nInsTblRow(USHRT_MAX),
    nNewNumSectDef(USHRT_MAX),
    nRowsToRepeat(0),
    mbReadCellWhileReadSwFly( false ), // #i83368#

    bTrowdRead(0),
    nReadFlyDepth(0),
    nZOrder(0)
{
    mbIsFootnote = mbReadNoTbl = bReadSwFly = bSwPageDesc = bStyleTabValid =
    bInPgDscTbl = bNewNumList = false;
    bFirstContinue = true;
    bContainsPara = false;
    bContainsTablePara = false;
    bNestedField = false;
    bForceNewTable = false;

    pPam = new SwPaM( *rCrsr.GetPoint() );
    SetInsPos( SwxPosition( pPam ) );
    SetChkStyleAttr( 0 != bReadNewDoc );
    SetCalcValue( sal_False );
    SetReadDocInfo( sal_True );

    // diese sollen zusaetzlich ueber \pard zurueck gesetzt werden
    sal_uInt16 temp;
    temp = RES_TXTATR_CHARFMT;      AddPlainAttr( temp );
    temp = RES_PAGEDESC;            AddPardAttr( temp );
    temp = RES_BREAK;               AddPardAttr( temp );
    temp = RES_PARATR_NUMRULE;      AddPardAttr( temp );
    temp = FN_PARAM_NUM_LEVEL;          AddPardAttr( temp );
}

// Aufruf des Parsers
SvParserState SwRTFParser::CallParser()
{
    mbReadNoTbl = false;
    bFirstContinue = true;

    rInput.Seek(STREAM_SEEK_TO_BEGIN);
    rInput.ResetError();

    mpRedlineStack = new sw::util::RedlineStack(*pDoc);

    return SvxRTFParser::CallParser();
}

static bool lcl_UsedPara(SwPaM &rPam)
{
    const SwCntntNode* pCNd;
    const SfxItemSet* pSet;
    if( rPam.GetPoint()->nContent.GetIndex() ||
        ( 0 != ( pCNd = rPam.GetCntntNode()) &&
          0 != ( pSet = pCNd->GetpSwAttrSet()) &&
         ( SFX_ITEM_SET == pSet->GetItemState( RES_BREAK, sal_False ) ||
           SFX_ITEM_SET == pSet->GetItemState( RES_PAGEDESC, sal_False ))))
        return true;
    return false;
}

void SwRTFParser::Continue( int nToken )
{
    if( bFirstContinue )
    {
        bFirstContinue = sal_False;

        if (IsNewDoc())
        {
            //
            // COMPATIBILITY FLAGS START
            //
            pDoc->set(IDocumentSettingAccess::PARA_SPACE_MAX, true);
            pDoc->set(IDocumentSettingAccess::PARA_SPACE_MAX_AT_PAGES, true);
            pDoc->set(IDocumentSettingAccess::TAB_COMPAT, true);
            pDoc->set(IDocumentSettingAccess::USE_VIRTUAL_DEVICE, true);
            pDoc->set(IDocumentSettingAccess::USE_HIRES_VIRTUAL_DEVICE, true);
            pDoc->set(IDocumentSettingAccess::ADD_FLY_OFFSETS, true);
            pDoc->set(IDocumentSettingAccess::ADD_EXT_LEADING, true);
            pDoc->set(IDocumentSettingAccess::OLD_NUMBERING, false);
            pDoc->set(IDocumentSettingAccess::IGNORE_FIRST_LINE_INDENT_IN_NUMBERING, false );
            pDoc->set(IDocumentSettingAccess::DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK, false);
            pDoc->set(IDocumentSettingAccess::OLD_LINE_SPACING, false);
            pDoc->set(IDocumentSettingAccess::ADD_PARA_SPACING_TO_TABLE_CELLS, true);
            pDoc->set(IDocumentSettingAccess::USE_FORMER_OBJECT_POS, false);
            pDoc->set(IDocumentSettingAccess::USE_FORMER_TEXT_WRAPPING, false);
            pDoc->set(IDocumentSettingAccess::CONSIDER_WRAP_ON_OBJECT_POSITION, true);
            pDoc->set(IDocumentSettingAccess::DO_NOT_RESET_PARA_ATTRS_FOR_NUM_FONT, false); // #i53199#

            pDoc->set(IDocumentSettingAccess::TABLE_ROW_KEEP, true);
            pDoc->set(IDocumentSettingAccess::IGNORE_TABS_AND_BLANKS_FOR_LINE_CALCULATION, true);
        pDoc->set(IDocumentSettingAccess::INVERT_BORDER_SPACING, true);
            //
            // COMPATIBILITY FLAGS END
            //
        }

        // einen temporaeren Index anlegen, auf Pos 0 so wird er nicht bewegt!
        pSttNdIdx = new SwNodeIndex( pDoc->GetNodes() );
        if( !IsNewDoc() )       // in ein Dokument einfuegen ?
        {
            const SwPosition* pPos = pPam->GetPoint();
            SwTxtNode* pSttNd = pPos->nNode.GetNode().GetTxtNode();

            pDoc->SplitNode( *pPos, false );

            *pSttNdIdx = pPos->nNode.GetIndex()-1;
            pDoc->SplitNode( *pPos, false );

            SwPaM aInsertionRangePam( *pPos );

            pPam->Move( fnMoveBackward );

            // split any redline over the insertion point
            aInsertionRangePam.SetMark();
            *aInsertionRangePam.GetPoint() = *pPam->GetPoint();
            aInsertionRangePam.Move( fnMoveBackward );
            pDoc->SplitRedline( aInsertionRangePam );

            pDoc->SetTxtFmtColl( *pPam, pDoc->GetTxtCollFromPool
                                 ( RES_POOLCOLL_STANDARD, false ));

            // verhinder das einlesen von Tabellen in Fussnoten / Tabellen
            sal_uLong nNd = pPos->nNode.GetIndex();
            mbReadNoTbl = 0 != pSttNd->FindTableNode() ||
                        ( nNd < pDoc->GetNodes().GetEndOfInserts().GetIndex() &&
                        pDoc->GetNodes().GetEndOfInserts().StartOfSectionIndex() < nNd );
        }

        // Laufbalken anzeigen, aber nur bei synchronem Call
        sal_uLong nCurrPos = rInput.Tell();
        rInput.Seek(STREAM_SEEK_TO_END);
        rInput.ResetError();
        ::StartProgress( STR_STATSTR_W4WREAD, 0, rInput.Tell(), pDoc->GetDocShell());
        rInput.Seek( nCurrPos );
        rInput.ResetError();
    }

    SvxRTFParser::Continue( nToken );

    if( SVPAR_PENDING == GetStatus() )
        return ;                // weiter gehts beim naechsten mal

    pRelNumRule->SetNumRelSpaces( *pDoc );

    // den Start wieder korrigieren
    if( !IsNewDoc() && pSttNdIdx->GetIndex() )
    {
        //die Flys muessen zuerst zurecht gerueckt werden, denn sonst wird
        // ein am 1. Absatz verankerter Fly falsch eingefuegt
        if( SVPAR_ACCEPTED == eState )
        {
            if( !aFlyArr.empty() )
                SetFlysInDoc();
            pRelNumRule->SetOultineRelSpaces( *pSttNdIdx, pPam->GetPoint()->nNode );
        }

        SwTxtNode* pTxtNode = pSttNdIdx->GetNode().GetTxtNode();
        SwNodeIndex aNxtIdx( *pSttNdIdx );
        if( pTxtNode && pTxtNode->CanJoinNext( &aNxtIdx ))
        {
            xub_StrLen nStt = pTxtNode->GetTxt().Len();
            // wenn der Cursor noch in dem Node steht, dann setze in an das Ende
            if( pPam->GetPoint()->nNode == aNxtIdx )
            {
                pPam->GetPoint()->nNode = *pSttNdIdx;
                pPam->GetPoint()->nContent.Assign( pTxtNode, nStt );
            }

#if OSL_DEBUG_LEVEL > 0
            // !!! sollte nicht moeglich sein, oder ??
            OSL_ENSURE( pSttNdIdx->GetIndex()+1 != pPam->GetBound( sal_True ).nNode.GetIndex(),
                "Pam.Bound1 steht noch im Node" );
            OSL_ENSURE( pSttNdIdx->GetIndex()+1 != pPam->GetBound( sal_False ).nNode.GetIndex(),
                "Pam.Bound2 steht noch im Node" );

if( pSttNdIdx->GetIndex()+1 == pPam->GetBound( sal_True ).nNode.GetIndex() )
{
    xub_StrLen nCntPos = pPam->GetBound( sal_True ).nContent.GetIndex();
    pPam->GetBound( sal_True ).nContent.Assign( pTxtNode,
                    pTxtNode->GetTxt().Len() + nCntPos );
}
if( pSttNdIdx->GetIndex()+1 == pPam->GetBound( sal_False ).nNode.GetIndex() )
{
    xub_StrLen nCntPos = pPam->GetBound( sal_False ).nContent.GetIndex();
    pPam->GetBound( sal_False ).nContent.Assign( pTxtNode,
                    pTxtNode->GetTxt().Len() + nCntPos );
}
#endif
            // Zeichen Attribute beibehalten!
            SwTxtNode* pDelNd = aNxtIdx.GetNode().GetTxtNode();
            if( pTxtNode->GetTxt().Len() )
                pDelNd->FmtToTxtAttr( pTxtNode );
            else
                pTxtNode->ChgFmtColl( pDelNd->GetTxtColl() );
            pTxtNode->JoinNext();
        }
    }

    if( SVPAR_ACCEPTED == eState )
    {
        // den letzen Bereich wieder zumachen
        if( pRegionEndIdx )
        {
            // the last section in WW are not a balanced Section.
            if( !GetVersionNo() )
            {
                SwSectionNode* pSectNd = pRegionEndIdx->GetNode().
                                    StartOfSectionNode()->GetSectionNode();
                if( pSectNd )
                    pSectNd->GetSection().GetFmt()->SetFmtAttr(
                                    SwFmtNoBalancedColumns( sal_True ) );
            }

            DelLastNode();
            pPam->GetPoint()->nNode = *pRegionEndIdx;
            pPam->Move( fnMoveForward, fnGoNode );
            delete pRegionEndIdx, pRegionEndIdx = 0;
        }

        sal_uInt16 nPageDescOffset = pDoc->GetPageDescCnt();
        maSegments.InsertSegments(IsNewDoc());
        UpdatePageDescs(*pDoc, nPageDescOffset);
        // following garbage collecting code has been moved from the previous
        // procedure UpdatePageDescs to here in order to fix a bug
        rtfSections::myrDummyIter aDEnd = maSegments.maDummyPageNos.rend();
        for (rtfSections::myrDummyIter aI = maSegments.maDummyPageNos.rbegin(); aI != aDEnd; ++aI)
            pDoc->DelPageDesc(*aI);

        if( !aFlyArr.empty() )
            SetFlysInDoc();

        // jetzt noch den letzten ueberfluessigen Absatz loeschen
        SwPosition* pPos = pPam->GetPoint();
        if( !pPos->nContent.GetIndex() )
        {
            SwTxtNode* pAktNd;
            sal_uLong nNodeIdx = pPos->nNode.GetIndex();
            if( IsNewDoc() )
            {
                SwNode* pTmp = pDoc->GetNodes()[ nNodeIdx -1 ];
                if( pTmp->IsCntntNode() && !pTmp->FindTableNode() )
                {
                    // Do not delete the paragraph if it has anchored objects:
                    bool bAnchoredObjs = false;
                    const SwFrmFmts* pFrmFmts = pDoc->GetSpzFrmFmts();
                    if ( pFrmFmts && !pFrmFmts->empty() )
                    {
                        for ( sal_uInt16 nI = pFrmFmts->size(); nI; --nI )
                        {
                            const SwFmtAnchor & rAnchor = (*pFrmFmts)[ nI - 1 ]->GetAnchor();
                            if ((FLY_AT_PARA == rAnchor.GetAnchorId()) ||
                                (FLY_AT_CHAR == rAnchor.GetAnchorId()))
                            {
                                const SwPosition * pObjPos = rAnchor.GetCntntAnchor();
                                if ( pObjPos && nNodeIdx == pObjPos->nNode.GetIndex() )
                                {
                                    bAnchoredObjs = true;
                                    break;
                                }
                            }
                        }
                    }

                    if ( !bAnchoredObjs )
                        DelLastNode();
                }
            }
            else if (0 != (pAktNd = pDoc->GetNodes()[nNodeIdx]->GetTxtNode()))
            {
                if( pAktNd->CanJoinNext( &pPos->nNode ))
                {
                    SwTxtNode* pNextNd = pPos->nNode.GetNode().GetTxtNode();
                    pPos->nContent.Assign( pNextNd, 0 );
                    pPam->SetMark(); pPam->DeleteMark();
                    pNextNd->JoinPrev();
                }
                else if( !pAktNd->GetTxt().Len() &&
                        pAktNd->StartOfSectionIndex()+2 <
                        pAktNd->EndOfSectionIndex() )
                {
                    pPos->nContent.Assign( 0, 0 );
                    pPam->SetMark(); pPam->DeleteMark();
                    pDoc->GetNodes().Delete( pPos->nNode, 1 );
                    pPam->Move( fnMoveBackward );
                }
            }
        }
        // nun noch das SplitNode vom Ende aufheben
        else if( !IsNewDoc() )
        {
            if( pPos->nContent.GetIndex() )     // dann gabs am Ende kein \par,
                pPam->Move( fnMoveForward, fnGoNode );  // als zum naechsten Node
            SwTxtNode* pTxtNode = pPos->nNode.GetNode().GetTxtNode();
            SwNodeIndex aPrvIdx( pPos->nNode );
            if( pTxtNode && pTxtNode->CanJoinPrev( &aPrvIdx ) &&
                *pSttNdIdx <= aPrvIdx )
            {
                // eigentlich muss hier ein JoinNext erfolgen, aber alle Cursor
                // usw. sind im pTxtNode angemeldet, so dass der bestehen
                // bleiben MUSS.

                // Absatz in Zeichen-Attribute umwandeln, aus dem Prev die
                // Absatzattribute und die Vorlage uebernehmen!
                SwTxtNode* pPrev = aPrvIdx.GetNode().GetTxtNode();
                pTxtNode->ChgFmtColl( pPrev->GetTxtColl() );
                pTxtNode->FmtToTxtAttr( pPrev );
                pTxtNode->ResetAllAttr();

                if( pPrev->HasSwAttrSet() )
                    pTxtNode->SetAttr( *pPrev->GetpSwAttrSet() );

                if( &pPam->GetBound(sal_True).nNode.GetNode() == pPrev )
                    pPam->GetBound(sal_True).nContent.Assign( pTxtNode, 0 );
                if( &pPam->GetBound(sal_False).nNode.GetNode() == pPrev )
                    pPam->GetBound(sal_False).nContent.Assign( pTxtNode, 0 );

                pTxtNode->JoinPrev();
            }
        }
    }
    delete pSttNdIdx, pSttNdIdx = 0;
    delete pRegionEndIdx, pRegionEndIdx = 0;
    RemoveUnusedNumRules();

    pDoc->SetUpdateExpFldStat(true);
    pDoc->SetInitDBFields(true);

    // Laufbalken bei asynchronen Call nicht einschalten !!!
    ::EndProgress( pDoc->GetDocShell() );
}

bool rtfSections::SetCols(SwFrmFmt &rFmt, const rtfSection &rSection,
    sal_uInt16 nNettoWidth)
{
    //sprmSCcolumns - Anzahl der Spalten - 1
    sal_uInt16 nCols = static_cast< sal_uInt16 >(rSection.NoCols());

    if (nCols < 2)
        return false;                   // keine oder bloedsinnige Spalten

    SwFmtCol aCol;                      // Erzeuge SwFmtCol

    //sprmSDxaColumns   - Default-Abstand 1.25 cm
    sal_uInt16 nColSpace = static_cast< sal_uInt16 >(rSection.StandardColSeperation());

    aCol.Init( nCols, nColSpace, nNettoWidth );

    // not SFEvenlySpaced
    if (rSection.maPageInfo.maColumns.size())
    {
        aCol._SetOrtho(false);
        sal_uInt16 nWishWidth = 0, nHalfPrev = 0;
        for (sal_uInt16 n=0, i=0;
             (static_cast<size_t>(n)+1) < rSection.maPageInfo.maColumns.size() && i < nCols;
             n += 2, ++i)
        {
            SwColumn* pCol = &aCol.GetColumns()[ i ];
            pCol->SetLeft( nHalfPrev );
            sal_uInt16 nSp = static_cast< sal_uInt16 >(rSection.maPageInfo.maColumns[ n+1 ]);
            nHalfPrev = nSp / 2;
            pCol->SetRight( nSp - nHalfPrev );
            pCol->SetWishWidth( static_cast< sal_uInt16 >(rSection.maPageInfo.maColumns[ n ]) +
                pCol->GetLeft() + pCol->GetRight());
            nWishWidth = nWishWidth + pCol->GetWishWidth();
        }
        aCol.SetWishWidth( nWishWidth );
    }

    rFmt.SetFmtAttr(aCol);
    return true;
}

void rtfSections::SetPage(SwPageDesc &rInPageDesc, SwFrmFmt &rFmt,
    const rtfSection &rSection, bool bIgnoreCols)
{
    // 1. Orientierung
    rInPageDesc.SetLandscape(rSection.IsLandScape());

    // 2. Papiergroesse
    SwFmtFrmSize aSz(rFmt.GetFrmSize());
    aSz.SetWidth(rSection.GetPageWidth());
    aSz.SetHeight(rSection.GetPageHeight());
    rFmt.SetFmtAttr(aSz);

    rFmt.SetFmtAttr(
        SvxLRSpaceItem(rSection.GetPageLeft(), rSection.GetPageRight(), 0, 0, RES_LR_SPACE));

    if (!bIgnoreCols)
    {
        SetCols(rFmt, rSection, static_cast< sal_uInt16 >(rSection.GetPageWidth() -
            rSection.GetPageLeft() - rSection.GetPageRight()));
    }

    rFmt.SetFmtAttr(rSection.maPageInfo.maBox);
}

bool HasHeader(const SwFrmFmt &rFmt)
{
    const SfxPoolItem *pHd;
    if (SFX_ITEM_SET == rFmt.GetItemState(RES_HEADER, false, &pHd))
        return ((const SwFmtHeader *)(pHd))->IsActive();
    return false;
}

bool HasFooter(const SwFrmFmt &rFmt)
{
    const SfxPoolItem *pFt;
    if (SFX_ITEM_SET == rFmt.GetItemState(RES_FOOTER, false, &pFt))
        return ((const SwFmtFooter *)(pFt))->IsActive();
    return false;
}

void rtfSections::GetPageULData(const rtfSection &rSection, bool bFirst,
    rtfSections::wwULSpaceData& rData)
{
    short nWWUp     = static_cast< short >(rSection.maPageInfo.mnMargtsxn);
    short nWWLo     = static_cast< short >(rSection.maPageInfo.mnMargbsxn);
    short nWWHTop   = static_cast< short >(rSection.maPageInfo.mnHeadery);
    short nWWFBot   = static_cast< short >(rSection.maPageInfo.mnFootery);

    if (bFirst)
    {
        if (
            rSection.mpTitlePage && HasHeader(rSection.mpTitlePage->GetMaster())
           )
        {
            rData.bHasHeader = true;
        }
    }
    else
    {
        if (rSection.mpPage &&
               (
               HasHeader(rSection.mpPage->GetMaster())
               || HasHeader(rSection.mpPage->GetLeft())
               )
           )
        {
            rData.bHasHeader = true;
        }
    }

    if( rData.bHasHeader )
    {
        rData.nSwUp  = nWWHTop;             // Header -> umrechnen, see ww8par6.cxx

        if ( nWWUp > 0 && nWWUp >= nWWHTop )
            rData.nSwHLo = nWWUp - nWWHTop;
        else
            rData.nSwHLo = 0;

        if (rData.nSwHLo < cMinHdFtHeight)
            rData.nSwHLo = cMinHdFtHeight;
    }
    else // kein Header -> Up einfach uebernehmen
        rData.nSwUp = Abs(nWWUp);

    if (bFirst)
    {
        if (
                rSection.mpTitlePage &&
                HasFooter(rSection.mpTitlePage->GetMaster())
           )
        {
            rData.bHasFooter = true;
        }
    }
    else
    {
        if (rSection.mpPage &&
           (
               HasFooter(rSection.mpPage->GetMaster())
               || HasFooter(rSection.mpPage->GetLeft())
           )
           )
        {
            rData.bHasFooter = true;
        }
    }

    if( rData.bHasFooter )
    {
        rData.nSwLo = nWWFBot;              // Footer -> Umrechnen
        if ( nWWLo > 0 && nWWLo >= nWWFBot )
            rData.nSwFUp = nWWLo - nWWFBot;
        else
            rData.nSwFUp = 0;

        if (rData.nSwFUp < cMinHdFtHeight)
            rData.nSwFUp = cMinHdFtHeight;
    }
    else // kein Footer -> Lo einfach uebernehmen
        rData.nSwLo = Abs(nWWLo);
}

void rtfSections::SetPageULSpaceItems(SwFrmFmt &rFmt,
    rtfSections::wwULSpaceData& rData)
{
    if (rData.bHasHeader)               // ... und Header-Lower setzen
    {
        //Kopfzeilenhoehe minimal sezten
        if (SwFrmFmt* pHdFmt = (SwFrmFmt*)rFmt.GetHeader().GetHeaderFmt())
        {
            pHdFmt->SetFmtAttr(SwFmtFrmSize(ATT_MIN_SIZE, 0, rData.nSwHLo));
            SvxULSpaceItem aHdUL(pHdFmt->GetULSpace());
            aHdUL.SetLower( rData.nSwHLo - cMinHdFtHeight );
            pHdFmt->SetFmtAttr(aHdUL);
            pHdFmt->SetFmtAttr(SwHeaderAndFooterEatSpacingItem(
                RES_HEADER_FOOTER_EAT_SPACING, true));
        }
    }

    if (rData.bHasFooter)               // ... und Footer-Upper setzen
    {
        if (SwFrmFmt* pFtFmt = (SwFrmFmt*)rFmt.GetFooter().GetFooterFmt())
        {
            pFtFmt->SetFmtAttr(SwFmtFrmSize(ATT_MIN_SIZE, 0, rData.nSwFUp));
            SvxULSpaceItem aFtUL(pFtFmt->GetULSpace());
            aFtUL.SetUpper( rData.nSwFUp - cMinHdFtHeight );
            pFtFmt->SetFmtAttr(aFtUL);
            pFtFmt->SetFmtAttr(SwHeaderAndFooterEatSpacingItem(
                RES_HEADER_FOOTER_EAT_SPACING, true));
        }
    }

    SvxULSpaceItem aUL(rData.nSwUp, rData.nSwLo, RES_UL_SPACE ); // Page-UL setzen
    rFmt.SetFmtAttr(aUL);
}

void rtfSections::SetSegmentToPageDesc(const rtfSection &rSection,
    bool bTitlePage, bool bIgnoreCols)
{
    SwPageDesc &rPage = bTitlePage ? *rSection.mpTitlePage : *rSection.mpPage;

    SwFrmFmt &rFmt = rPage.GetMaster();

    wwULSpaceData aULData;
    GetPageULData(rSection, bTitlePage, aULData);
    SetPageULSpaceItems(rFmt, aULData);

    SetPage(rPage, rFmt, rSection, bIgnoreCols);

    UseOnPage ePage = rPage.ReadUseOn();
    if(ePage & nsUseOnPage::PD_ALL)
    {
        SwFrmFmt &rFmtLeft = rPage.GetLeft();
        SetPageULSpaceItems(rFmtLeft, aULData);
        SetPage(rPage, rFmtLeft, rSection, bIgnoreCols);
    }

}

void rtfSections::CopyFrom(const SwPageDesc &rFrom, SwPageDesc &rDest)
{
    UseOnPage ePage = rFrom.ReadUseOn();
    rDest.WriteUseOn(ePage);

    mrReader.pDoc->CopyHeader(rFrom.GetMaster(), rDest.GetMaster());
    SwFrmFmt &rDestFmt = rDest.GetMaster();
    rDestFmt.SetFmtAttr(rFrom.GetMaster().GetHeader());
    mrReader.pDoc->CopyHeader(rFrom.GetLeft(), rDest.GetLeft());
    mrReader.pDoc->CopyFooter(rFrom.GetMaster(), rDest.GetMaster());
    mrReader.pDoc->CopyFooter(rFrom.GetLeft(), rDest.GetLeft());
}

void rtfSections::MoveFrom(SwPageDesc &rFrom, SwPageDesc &rDest)
{
    UseOnPage ePage = rFrom.ReadUseOn();
    rDest.WriteUseOn(ePage);

    SwFrmFmt &rDestMaster = rDest.GetMaster();
    SwFrmFmt &rFromMaster = rFrom.GetMaster();
    rDestMaster.SetFmtAttr(rFromMaster.GetHeader());
    rDestMaster.SetFmtAttr(rFromMaster.GetFooter());

    SwFrmFmt &rDestLeft = rDest.GetLeft();
    SwFrmFmt &rFromLeft = rFrom.GetLeft();
    rDestLeft.SetFmtAttr(rFromLeft.GetHeader());
    rDestLeft.SetFmtAttr(rFromLeft.GetFooter());
}

void rtfSections::SetHdFt(rtfSection &rSection)
{
    OSL_ENSURE(rSection.mpPage, "makes no sense to call without a main page");
    if (rSection.mpPage && rSection.maPageInfo.mpPageHdFt)
    {
        if (rSection.maPageInfo.mbPageHdFtUsed)
        {
            MoveFrom(*rSection.maPageInfo.mpPageHdFt, *rSection.mpPage);
            rSection.maPageInfo.mbPageHdFtUsed = false;
            rSection.maPageInfo.mpPageHdFt = rSection.mpPage;
        }
        else
            CopyFrom(*rSection.maPageInfo.mpPageHdFt, *rSection.mpPage);
    }

    if (rSection.mpTitlePage && rSection.maPageInfo.mpTitlePageHdFt)
    {
        if (rSection.maPageInfo.mbTitlePageHdFtUsed)
        {
            MoveFrom(*rSection.maPageInfo.mpTitlePageHdFt,
                    *rSection.mpTitlePage);
            rSection.maPageInfo.mbTitlePageHdFtUsed = false;
            rSection.maPageInfo.mpTitlePageHdFt = rSection.mpTitlePage;
        }
        else
        {
            CopyFrom(*rSection.maPageInfo.mpTitlePageHdFt,
                    *rSection.mpTitlePage);
        }
    }
}

SwSectionFmt *rtfSections::InsertSection(SwPaM& rMyPaM, rtfSection &rSection)
{
    SwSectionData aSectionData(CONTENT_SECTION,
            mrReader.pDoc->GetUniqueSectionName());

    SfxItemSet aSet( mrReader.pDoc->GetAttrPool(), aFrmFmtSetRange );

    sal_uInt8 nRTLPgn = maSegments.empty() ? 0 : maSegments.back().IsBiDi();
    aSet.Put(SvxFrameDirectionItem(
        nRTLPgn ? FRMDIR_HORI_RIGHT_TOP : FRMDIR_HORI_LEFT_TOP, RES_FRAMEDIR));

    rSection.mpSection =
        mrReader.pDoc->InsertSwSection( rMyPaM, aSectionData, 0, &aSet );
    OSL_ENSURE(rSection.mpSection, "section not inserted!");
    if (!rSection.mpSection)
        return 0;

    SwPageDesc *pPage = 0;
    mySegrIter aEnd = maSegments.rend();
    for (mySegrIter aIter = maSegments.rbegin(); aIter != aEnd; ++aIter)
    {
        pPage = aIter->mpPage;
        if (pPage)
            break;
    }

    OSL_ENSURE(pPage, "no page outside this section!");

    if (!pPage)
        pPage = &mrReader.pDoc->GetPageDesc(0);

    if (!pPage)
        return 0;

    SwFrmFmt& rFmt = pPage->GetMaster();
    const SwFmtFrmSize&   rSz = rFmt.GetFrmSize();
    const SvxLRSpaceItem& rLR = rFmt.GetLRSpace();
    SwTwips nWidth = rSz.GetWidth();
    long nLeft  = rLR.GetTxtLeft();
    long nRight = rLR.GetRight();

    SwSectionFmt *pFmt = rSection.mpSection->GetFmt();
    OSL_ENSURE(pFmt, "impossible");
    if (!pFmt)
        return 0;
    SetCols(*pFmt, rSection, (sal_uInt16)(nWidth - nLeft - nRight) );

    return pFmt;
}

void rtfSections::InsertSegments(bool bNewDoc)
{
    sal_uInt16 nDesc(0);
    mySegIter aEnd = maSegments.end();
    mySegIter aStart = maSegments.begin();
    for (mySegIter aIter = aStart; aIter != aEnd; ++aIter)
    {
        mySegIter aNext = aIter+1;

        bool bInsertSection = aIter != aStart ? aIter->IsContinous() : false;

        if (!bInsertSection)
        {
            /*
             If a cont section follow this section then we won't be
             creating a page desc with 2+ cols as we cannot host a one
             col section in a 2+ col pagedesc and make it look like
             word. But if the current section actually has columns then
             we are forced to insert a section here as well as a page
             descriptor.
            */

            /*
             Note for the future:
             If we want to import "protected sections" the here is
             where we would also test for that and force a section
             insertion if that was true.
            */
            bool bIgnoreCols = false;
            if (aNext != aEnd && aNext->IsContinous())
            {
                bIgnoreCols = true;
                if (aIter->NoCols() > 1)
                    bInsertSection = true;
            }

            if (aIter->HasTitlePage())
            {
                if (bNewDoc && aIter == aStart)
                {
                    aIter->mpTitlePage =
                        mrReader.pDoc->GetPageDescFromPool(RES_POOLPAGE_FIRST);
                }
                else
                {
                    sal_uInt16 nPos = mrReader.pDoc->MakePageDesc(
                        ViewShell::GetShellRes()->GetPageDescName(nDesc, ShellResource::NORMAL_PAGE)
                        , 0, false);
                    aIter->mpTitlePage = &mrReader.pDoc->GetPageDesc(nPos);
                }
                OSL_ENSURE(aIter->mpTitlePage, "no page!");
                if (!aIter->mpTitlePage)
                    continue;

                SetSegmentToPageDesc(*aIter, true, bIgnoreCols);
            }

            if (!bNewDoc && aIter == aStart)
                continue;
            else if (bNewDoc && aIter == aStart)
            {
                aIter->mpPage =
                    mrReader.pDoc->GetPageDescFromPool(RES_POOLPAGE_STANDARD);
            }
            else
            {
                sal_uInt16 nPos = mrReader.pDoc->MakePageDesc(
                    ViewShell::GetShellRes()->GetPageDescName(nDesc,
                        aIter->HasTitlePage() ? ShellResource::FIRST_PAGE : ShellResource::NORMAL_PAGE),
                        aIter->mpTitlePage, false);
                aIter->mpPage = &mrReader.pDoc->GetPageDesc(nPos);
            }
            OSL_ENSURE(aIter->mpPage, "no page!");
            if (!aIter->mpPage)
                continue;

            SetHdFt(*aIter);

            if (aIter->mpTitlePage)
                SetSegmentToPageDesc(*aIter, true, bIgnoreCols);
            SetSegmentToPageDesc(*aIter, false, bIgnoreCols);

            SwFmtPageDesc aPgDesc(aIter->HasTitlePage() ?
                 aIter->mpTitlePage : aIter->mpPage);

            if (aIter->mpTitlePage)
                aIter->mpTitlePage->SetFollow(aIter->mpPage);

            if (aIter->PageRestartNo() ||
                ((aIter == aStart) && aIter->PageStartAt() != 1))
                aPgDesc.SetNumOffset( static_cast< sal_uInt16 >(aIter->PageStartAt()) );

            /*
            If its a table here, apply the pagebreak to the table
            properties, otherwise we add it to the para at this
            position
            */
            if (aIter->maStart.GetNode().IsTableNode())
            {
                SwTable& rTable =
                    aIter->maStart.GetNode().GetTableNode()->GetTable();
                SwFrmFmt* pApply = rTable.GetFrmFmt();
                OSL_ENSURE(pApply, "impossible");
                if (pApply)
                    pApply->SetFmtAttr(aPgDesc);
            }
            else
            {
                SwPosition aPamStart(aIter->maStart);
                aPamStart.nContent.Assign(
                    aIter->maStart.GetNode().GetCntntNode(), 0);
                SwPaM aPage(aPamStart);

                mrReader.pDoc->InsertPoolItem(aPage, aPgDesc, 0);
            }
            ++nDesc;
        }

        SwTxtNode* pTxtNd = 0;
        if (bInsertSection)
        {
            SwPaM aSectPaM(*mrReader.pPam);
            SwNodeIndex aAnchor(aSectPaM.GetPoint()->nNode);
            if (aNext != aEnd)
            {
                aAnchor = aNext->maStart;
                aSectPaM.GetPoint()->nNode = aAnchor;
                aSectPaM.GetPoint()->nContent.Assign(
                    aNext->maStart.GetNode().GetCntntNode(), 0);
                aSectPaM.Move(fnMoveBackward);
            }

            const SwPosition* pPos  = aSectPaM.GetPoint();
            SwTxtNode const*const pSttNd = pPos->nNode.GetNode().GetTxtNode();
            const SwTableNode* pTableNd = pSttNd ? pSttNd->FindTableNode() : 0;
            if (pTableNd)
            {
                pTxtNd =
                    mrReader.pDoc->GetNodes().MakeTxtNode(aAnchor,
                    mrReader.pDoc->GetTxtCollFromPool( RES_POOLCOLL_TEXT ));

                aSectPaM.GetPoint()->nNode = SwNodeIndex(*pTxtNd);
                aSectPaM.GetPoint()->nContent.Assign(
                    aSectPaM.GetCntntNode(), 0);
            }

            aSectPaM.SetMark();

            aSectPaM.GetPoint()->nNode = aIter->maStart;
            aSectPaM.GetPoint()->nContent.Assign(
                aSectPaM.GetCntntNode(), 0);

            SwSectionFmt *pRet = InsertSection(aSectPaM, *aIter);
            //The last section if continous is always unbalanced
            if (aNext == aEnd && pRet)
                pRet->SetFmtAttr(SwFmtNoBalancedColumns(true));
        }

        if (pTxtNd)
        {
            SwNodeIndex aIdx(*pTxtNd);
            SwPosition aPos(aIdx);
            SwPaM aTest(aPos);
            mrReader.pDoc->DelFullPara(aTest);
            pTxtNd = 0;
        }
    }
}

namespace sw{
    namespace util{

InsertedTableClient::InsertedTableClient(SwTableNode & rNode)
{
    rNode.Add(this);
}

SwTableNode * InsertedTableClient::GetTableNode()
{
    return dynamic_cast<SwTableNode *> (GetRegisteredInNonConst());
}

InsertedTablesManager::InsertedTablesManager(const SwDoc &rDoc)
    : mbHasRoot(rDoc.GetCurrentLayout())    //swmod 080218
{
}

void InsertedTablesManager::DelAndMakeTblFrms()
{
    if (!mbHasRoot)
        return;
    TblMapIter aEnd = maTables.end();
    for (TblMapIter aIter = maTables.begin(); aIter != aEnd; ++aIter)
    {
        // exitiert schon ein Layout, dann muss an dieser Tabelle die BoxFrames
        // neu erzeugt
        SwTableNode *pTable = aIter->first->GetTableNode();
        OSL_ENSURE(pTable, "Why no expected table");
        if (pTable)
        {
            SwFrmFmt * pFrmFmt = pTable->GetTable().GetFrmFmt();

            if (pFrmFmt != NULL)
            {
                SwNodeIndex *pIndex = aIter->second;
                pTable->DelFrms();
                pTable->MakeFrms(pIndex);
            }
        }
    }
}

void InsertedTablesManager::InsertTable(SwTableNode &rTableNode, SwPaM &rPaM)
{
    if (!mbHasRoot)
        return;
    //Associate this tablenode with this after position, replace an //old
    //node association if necessary

    InsertedTableClient * pClient = new InsertedTableClient(rTableNode);

    maTables.insert(TblMap::value_type(pClient, &(rPaM.GetPoint()->nNode)));
}
}
}

SwRTFParser::~SwRTFParser()
{
    maInsertedTables.DelAndMakeTblFrms();
    mpRedlineStack->closeall(*pPam->GetPoint());
    delete mpRedlineStack;

    delete pSttNdIdx;
    delete pRegionEndIdx;
    delete pPam;
    delete pRelNumRule;

    if (pGrfAttrSet)
        DELETEZ( pGrfAttrSet );
}

//i19718
void SwRTFParser::ReadShpRslt()
{
    int nToken;
    while ('}' != (nToken = GetNextToken() ) && IsParserWorking())
    {
        switch(nToken)
        {
            case RTF_PAR:
                break;
            default:
                NextToken(nToken);
                break;
        }
    }
    SkipToken(-1);
}

void SwRTFParser::ReadShpTxt(String& s)
{
    int nToken;
    int level=1;
    s.AppendAscii("{\\rtf");
    while (level>0 && IsParserWorking())
    {
        nToken = GetNextToken();
        switch(nToken)
        {
            case RTF_SN:
            case RTF_SV:
                SkipGroup();
                break;
            case RTF_TEXTTOKEN:
                s.Append(aToken);
                break;
            case '{':
                level++;
                s.Append(rtl::OUString("{"));
                break;
            case '}':
                level--;
                s.Append(rtl::OUString("}"));
                break;
            default:
                s.Append(aToken);
                if (bTokenHasValue)
                    s.Append(rtl::OUString::valueOf(static_cast<sal_Int64>(nTokenValue)));
                s.Append(rtl::OUString(" "));
                break;
        }
    }
    SkipToken(-1);
}

/*
 * Very basic support for the Z-line.
 */
void SwRTFParser::ReadDrawingObject()
{
    int nToken;
    int level;
    level=1;
    Rectangle aRect;
    ::basegfx::B2DPolygon aPolygon;
    ::basegfx::B2DPoint aPoint;
    bool bPolygonActive(false);

    SwFmtHoriOrient aHori( 0, text::HoriOrientation::NONE, text::RelOrientation::PAGE_FRAME );
    SwFmtVertOrient aVert( 0, text::VertOrientation::NONE, text::RelOrientation::PAGE_FRAME );

    while (level>0 && IsParserWorking())
    {
        nToken = GetNextToken();
        switch(nToken)
        {
            case '}':
                level--;
                break;
            case '{':
                level++;
                break;
            case RTF_DOBXMARGIN:
                aHori.SetRelationOrient( text::RelOrientation::PAGE_PRINT_AREA );
                break;
            case RTF_DOBYMARGIN:
                aVert.SetRelationOrient( text::RelOrientation::PAGE_PRINT_AREA );
                break;
            case RTF_DPX:
                aRect.setX(nTokenValue);
                break;
            case RTF_DPXSIZE:
                aRect.setWidth(nTokenValue);
                break;
            case RTF_DPY:
                aRect.setY(nTokenValue);
                break;
            case RTF_DPYSIZE:
                aRect.setHeight(nTokenValue);
                break;
            case RTF_DPLINE:
            case RTF_DPPOLYCOUNT:
                bPolygonActive = true;
                break;
            case RTF_DPPTX:
                aPoint.setX(nTokenValue);
                break;
            case RTF_DPPTY:
                aPoint.setY(nTokenValue);

                if(bPolygonActive)
                {
                    aPolygon.append(aPoint);
                }

                break;
            default:
                break;
        }
    }
    SkipToken(-1);

    if(bPolygonActive && aPolygon.count())
    {
        SdrPathObj* pStroke = new SdrPathObj(OBJ_PLIN, ::basegfx::B2DPolyPolygon(aPolygon));
        SfxItemSet aFlySet(pDoc->GetAttrPool(), RES_FRMATR_BEGIN, RES_FRMATR_END-1);
        SwFmtSurround aSur( SURROUND_PARALLEL );
        aSur.SetContour( false );
        aSur.SetOutside(true);
        aFlySet.Put( aSur );
        SwFmtFollowTextFlow aFollowTextFlow( sal_False );
        aFlySet.Put( aFollowTextFlow );

        SwFmtAnchor aAnchor( FLY_AT_PARA );
        aAnchor.SetAnchor( pPam->GetPoint() );
        aFlySet.Put( aAnchor );

        aFlySet.Put( aHori );
        aFlySet.Put( aVert );

        pDoc->GetOrCreateDrawModel();
        SdrModel* pDrawModel  = pDoc->GetDrawModel();
        SdrPage* pDrawPg = pDrawModel->GetPage(0);
        pDrawPg->InsertObject(pStroke, 0);

        pStroke->SetSnapRect(aRect);

        pDoc->Insert(*pPam, *pStroke, &aFlySet, NULL);
    }
}

void SwRTFParser::InsertShpObject(SdrObject* pStroke, int _nZOrder)
{
        SfxItemSet aFlySet(pDoc->GetAttrPool(), RES_FRMATR_BEGIN, RES_FRMATR_END-1);
        SwFmtSurround aSur( SURROUND_THROUGHT );
        aSur.SetContour( false );
        aSur.SetOutside(true);
        aFlySet.Put( aSur );
        SwFmtFollowTextFlow aFollowTextFlow( sal_False );
        aFlySet.Put( aFollowTextFlow );

        SwFmtAnchor aAnchor( FLY_AT_PARA );
        aAnchor.SetAnchor( pPam->GetPoint() );
        aFlySet.Put( aAnchor );


        SwFmtHoriOrient aHori( 0, text::HoriOrientation::NONE, text::RelOrientation::PAGE_FRAME );
        aFlySet.Put( aHori );

        SwFmtVertOrient aVert( 0, text::VertOrientation::NONE, text::RelOrientation::PAGE_FRAME );
        aFlySet.Put( aVert );

        aFlySet.Put(SvxOpaqueItem(RES_OPAQUE,false));

        pDoc->GetOrCreateDrawModel();
        SdrModel* pDrawModel  = pDoc->GetDrawModel();
        SdrPage* pDrawPg = pDrawModel->GetPage(0);
        pDrawPg->InsertObject(pStroke);
        pDrawPg->SetObjectOrdNum(pStroke->GetOrdNum(), _nZOrder);
        pDoc->Insert(*pPam, *pStroke, &aFlySet, NULL);
}

::basegfx::B2DPoint rotate(const ::basegfx::B2DPoint& rStart, const ::basegfx::B2DPoint& rEnd)
{
    const ::basegfx::B2DVector aVector(rStart - rEnd);
    return ::basegfx::B2DPoint(aVector.getY() + rEnd.getX(), -aVector.getX() + rEnd.getY());
}


void SwRTFParser::ReadShapeObject()
{
    int nToken;
    int level;
    level=1;
    ::basegfx::B2DPoint aPointLeftTop;
    ::basegfx::B2DPoint aPointRightBottom;
    String sn;
    sal_Int32 shapeType=-1;
    Graphic aGrf;
    bool bGrfValid=false;
    bool fFilled=true;
    Color fillColor(255, 255, 255);
    bool fLine=true;
    int lineWidth=9525/360;
    String shpTxt;
    bool bshpTxt=false;
    int txflTextFlow=0;
    ::rtl::OUString sDescription, sName;


    while (level>0 && IsParserWorking())
    {
        nToken = GetNextToken();
        switch(nToken)
        {
            case '}':
                level--;
                break;
            case '{':
                level++;
                break;
            case RTF_SHPLEFT:
                aPointLeftTop.setX(nTokenValue);
                break;
            case RTF_SHPTOP:
                aPointLeftTop.setY(nTokenValue);
                break;
            case RTF_SHPBOTTOM:
                aPointRightBottom.setY(nTokenValue);
                break;
            case RTF_SHPRIGHT:
                aPointRightBottom.setX(nTokenValue);
                break;
            case RTF_SN:
                nToken = GetNextToken();
                OSL_ENSURE(nToken==RTF_TEXTTOKEN, "expected name");
                sn=aToken;
                break;
            case RTF_SV:
                nToken = GetNextToken();
                if (nToken==RTF_TEXTTOKEN)
                {
                    if (sn.EqualsAscii("shapeType"))
                    {
                        shapeType=aToken.ToInt32();

                    } else if (sn.EqualsAscii("fFilled"))
                    {
                        fFilled=aToken.ToInt32();

                    } else if (sn.EqualsAscii("fLine"))
                    {
                            fLine=aToken.ToInt32();
                    } else if (sn.EqualsAscii("lineWidth"))
                    {
                            lineWidth=aToken.ToInt32()/360;

                    } else if (sn.EqualsAscii("fillColor"))
                    {
                        sal_uInt32 nColor=aToken.ToInt32();
                        fillColor=Color( (sal_uInt8)nColor, (sal_uInt8)( nColor >> 8 ), (sal_uInt8)( nColor >> 16 ) );
                    }else if (sn.EqualsAscii("txflTextFlow"))
                      {
                        txflTextFlow=aToken.ToInt32();
                      }
                    else if (sn.EqualsAscii("wzDescription"))
                    {
                        sDescription = aToken;
                    }
                    else if(sn.EqualsAscii("wzName"))
                    {
                        sName = aToken;
                    }
                }
                break;
            case RTF_PICT:
                {
                        SvxRTFPictureType aPicType;
                        bGrfValid=ReadBmpData( aGrf, aPicType );
                }
                break;
            case RTF_SHPRSLT:
                if (shapeType!=1 && shapeType!=20 && shapeType!=75)
                {
                    ReadShpRslt();
                }
                break;
                     case RTF_SHPTXT:
               ReadShpTxt(shpTxt);
               bshpTxt=true;
               break;

            default:
                break;
        }
    }
    SkipToken(-1);

    SdrObject* pSdrObject = 0;
    switch(shapeType)
    {
        case 202: /* Text Box */
    case 1: /* Rectangle */
        {
            ::basegfx::B2DRange aRange(aPointLeftTop);
            aRange.expand(aPointRightBottom);

          if (txflTextFlow==2) {
            const ::basegfx::B2DPoint a(rotate(aRange.getMinimum(), aRange.getCenter()));
            const ::basegfx::B2DPoint b(rotate(aRange.getMaximum(), aRange.getCenter()));

            aRange.reset();
            aRange.expand(a);
            aRange.expand(b);
          }

            const Rectangle aRect(FRound(aRange.getMinX()), FRound(aRange.getMinY()), FRound(aRange.getMaxX()), FRound(aRange.getMaxY()));
            SdrRectObj* pStroke = new SdrRectObj(aRect);
            pSdrObject = pStroke;
            pStroke->SetSnapRect(aRect);
            pDoc->GetOrCreateDrawModel(); // create model
            InsertShpObject(pStroke, this->nZOrder++);
            SfxItemSet aSet(pStroke->GetMergedItemSet());
            if (fFilled)
            {
                aSet.Put(XFillStyleItem(XFILL_SOLID));
                aSet.Put(XFillColorItem( String(), fillColor ) );
            }
            else
            {
                aSet.Put(XFillStyleItem(XFILL_NONE));
            }
            if (!fLine) {
              aSet.Put(XLineStyleItem(XLINE_NONE));
            } else {
              aSet.Put( XLineWidthItem( lineWidth/2 ) ); // lineWidth are in 1000th mm, seems that XLineWidthItem expects 1/2 the line width
            }

            pStroke->SetMergedItemSet(aSet);
            if (bshpTxt) {
              SdrOutliner& rOutliner=pDoc->GetDrawModel()->GetDrawOutliner(pStroke);
              rOutliner.Clear();
              rtl::OString bs(rtl::OUStringToOString(shpTxt,
                RTL_TEXTENCODING_ASCII_US));
              SvMemoryStream aStream(const_cast<sal_Char*>(bs.getStr()),
                bs.getLength(), STREAM_READ);
              rOutliner.Read(aStream, rtl::OUString(), EE_FORMAT_RTF);
              OutlinerParaObject* pParaObject=rOutliner.CreateParaObject();
              pStroke->NbcSetOutlinerParaObject(pParaObject);
              rOutliner.Clear();
            }
            if (txflTextFlow==2) {
              long nAngle = 90;
              double a = nAngle*100*nPi180;
              pStroke->Rotate(pStroke->GetCurrentBoundRect().Center(), nAngle*100, sin(a), cos(a) );

            }

        }
        break;
    case 20: /* Line */
        {
            ::basegfx::B2DPolygon aLine;
            aLine.append(aPointLeftTop);
            aLine.append(aPointRightBottom);

            SdrPathObj* pStroke = new SdrPathObj(OBJ_PLIN, ::basegfx::B2DPolyPolygon(aLine));
            pSdrObject = pStroke;

            InsertShpObject(pStroke, this->nZOrder++);
            SfxItemSet aSet(pStroke->GetMergedItemSet());
            if (!fLine) {
              aSet.Put(XLineStyleItem(XLINE_NONE));
            } else {
              aSet.Put( XLineWidthItem( lineWidth/2 ) ); // lineWidth are in 1000th mm, seems that XLineWidthItem expects 1/2 the line width
            }

            pStroke->SetMergedItemSet(aSet);
        }
        break;
    case 75 : /* Picture */
        if (bGrfValid) {
            ::basegfx::B2DRange aRange(aPointLeftTop);
            aRange.expand(aPointRightBottom);
            const Rectangle aRect(FRound(aRange.getMinX()), FRound(aRange.getMinY()), FRound(aRange.getMaxX()), FRound(aRange.getMaxY()));

            SdrRectObj* pStroke = new SdrGrafObj(aGrf);
            pSdrObject = pStroke;
            pStroke->SetSnapRect(aRect);

            InsertShpObject(pStroke, this->nZOrder++);
        }
    }
    if( pSdrObject )
    {
        pSdrObject->SetDescription(sDescription);
        pSdrObject->SetTitle(sName);
    }
}

extern void sw3io_ConvertFromOldField( SwDoc& rDoc, sal_uInt16& rWhich,
                                sal_uInt16& rSubType, sal_uLong &rFmt,
                                sal_uInt16 nVersion );

sal_uInt16 SwRTFParser::ReadRevTbl()
{
    int nNumOpenBrakets = 1, nToken;        // die erste wurde schon vorher erkannt !!
    sal_uInt16 nAuthorTableIndex = 0;

    while( nNumOpenBrakets && IsParserWorking() )
    {
        switch( nToken = GetNextToken() )
        {
        case '}':   --nNumOpenBrakets;  break;
        case '{':
            {
                if( RTF_IGNOREFLAG != GetNextToken() )
                    nToken = SkipToken( -1 );
                else if( RTF_UNKNOWNCONTROL != GetNextToken() )
                    nToken = SkipToken( -2 );
                else
                {
                    ReadUnknownData();
                    nToken = GetNextToken();
                    if( '}' != nToken )
                        eState = SVPAR_ERROR;
                    break;
                }
                ++nNumOpenBrakets;
            }
            break;

        case RTF_TEXTTOKEN:
            aToken = comphelper::string::stripEnd(aToken, ';');

            sal_uInt16 nSWId = pDoc->InsertRedlineAuthor(aToken);
            // Store matchpair
            m_aAuthorInfos[nAuthorTableIndex] = nSWId;

            aRevTbl.push_back(aToken);
            nAuthorTableIndex++;
            break;
        }
    }
    SkipToken( -1 );
    return nAuthorTableIndex;
}

void SwRTFParser::NextToken( int nToken )
{
    sal_uInt16 eDateFmt;

    switch( nToken )
    {
    case RTF_FOOTNOTE:
    {
        //We can only insert a footnote if we're not inside a footnote. e.g.
        //#i7713#

        // in insert mode it's also possible to be inside of a footnote!
        bool bInsertIntoFootnote = false;
        if( !IsNewDoc() )
        {
            SwStartNode* pSttNode = pPam->GetNode()->StartOfSectionNode();
            while(pSttNode && pSttNode->IsSectionNode())
            {
                pSttNode = pSttNode->StartOfSectionNode();
            }
            if( SwFootnoteStartNode == pSttNode->GetStartNodeType() )
                bInsertIntoFootnote = true;
        }
        if (!mbIsFootnote && !bInsertIntoFootnote)
        {
            ReadHeaderFooter( nToken );
            SkipToken( -1 );        // Klammer wieder zurueck
        }
    }
    break;
    case RTF_SWG_PRTDATA:
        ReadPrtData();
        break;
    case RTF_XE:
        ReadXEField();
        break;
    case RTF_FIELD:
        ReadField();
        break;
    case RTF_SHPRSLT:
        ReadShpRslt();
        break;
    case RTF_DO:
        ReadDrawingObject();
        break;
    case RTF_SHP:
        ReadShapeObject();
        break;
    case RTF_SHPPICT:
    case RTF_PICT:
        ReadBitmapData();
        break;
#ifdef READ_OLE_OBJECT
    case RTF_OBJECT:
        ReadOLEData();
        break;
#endif
    case RTF_TROWD:                 ReadTable( nToken );        break;
    case RTF_PGDSCTBL:
        if( !IsNewDoc() )
            SkipPageDescTbl();
        else
            ReadPageDescTbl();
        break;
    case RTF_LISTTABLE:             ReadListTable();            break;
    case RTF_LISTOVERRIDETABLE:     ReadListOverrideTable();    break;

    case RTF_LISTTEXT:
        GetAttrSet().Put( SfxUInt16Item( FN_PARAM_NUM_LEVEL, 0 ));
        SkipGroup();
        break;

    case RTF_PN:
        if( bNewNumList )
            SkipGroup();
        else
        {
            bStyleTabValid = sal_True;
            if (SwNumRule* pRule = ReadNumSecLevel( nToken ))
            {
                GetAttrSet().Put( SwNumRuleItem( pRule->GetName() ));

                if( SFX_ITEM_SET != GetAttrSet().GetItemState( FN_PARAM_NUM_LEVEL, sal_False ))
                    GetAttrSet().Put( SfxUInt16Item( FN_PARAM_NUM_LEVEL, 0 ));
            }
        }
        break;


    case RTF_BKMKSTART:
        if(RTF_TEXTTOKEN == GetNextToken())
            mpBookmarkStart = new BookmarkPosition(*pPam);
        else
            SkipToken(-1);

        SkipGroup();
        break;

    case RTF_BKMKEND:
        if(RTF_TEXTTOKEN == GetNextToken())
        {
            const String& sBookmark = aToken;
            KeyCode aEmptyKeyCode;
            if (mpBookmarkStart)
            {
                BookmarkPosition aBookmarkEnd(*pPam);
                SwPaM aBookmarkRegion(  mpBookmarkStart->maMkNode, mpBookmarkStart->mnMkCntnt,
                                        aBookmarkEnd.maMkNode, aBookmarkEnd.mnMkCntnt);
                if (*mpBookmarkStart == aBookmarkEnd)
                    aBookmarkRegion.DeleteMark();
                pDoc->getIDocumentMarkAccess()->makeMark(aBookmarkRegion, sBookmark, IDocumentMarkAccess::BOOKMARK);
            }
            delete mpBookmarkStart, mpBookmarkStart = 0;
        }
        else
            SkipToken(-1);

        SkipGroup();
        break;


    case RTF_PNSECLVL:{
        if( bNewNumList)
            SkipGroup();
        else
            ReadNumSecLevel( nToken );
        break;
                      }

    case RTF_PNTEXT:
    case RTF_NONSHPPICT:
        SkipGroup();
        break;

    case RTF_DEFFORMAT:
    case RTF_DEFTAB:
    case RTF_DEFLANG:
        // sind zwar Dok-Controls, werden aber manchmal auch vor der
        // Font/Style/Color-Tabelle gesetzt!
        SvxRTFParser::NextToken( nToken );
        break;

    case RTF_PAGE:
        if (pTableNode==NULL) { //A \page command within a table is ignored by Word.
            if (lcl_UsedPara(*pPam))
                InsertPara();
            CheckInsNewTblLine();
            pDoc->InsertPoolItem(*pPam,
                SvxFmtBreakItem(SVX_BREAK_PAGE_BEFORE, RES_BREAK), 0);
        }
        break;

    case RTF_SECT:
        ReadSectControls( nToken );
        break;
    case RTF_CELL:
        mbReadCellWhileReadSwFly = bReadSwFly; // #i83368#
        if (CantUseTables())
            InsertPara();
        else
        {
            // Tabelle nicht mehr vorhanden ?
            if (USHRT_MAX != nInsTblRow && !pTableNode)
                NewTblLine();               // evt. Line copieren
            GotoNextBox();
        }
        break;

    case RTF_ROW:
        bTrowdRead=false;
        if (!CantUseTables())
        {
            // aus der Line raus
            m_nCurrentBox = 0;
            pTableNode = 0;
            // noch in der Tabelle drin?
            SwNodeIndex& rIdx = pPam->GetPoint()->nNode;
            const SwTableNode* pTblNd = rIdx.GetNode().FindTableNode();
            if( pTblNd )
            {
                // search the end of this row
                const SwStartNode* pBoxStt =
                                    rIdx.GetNode().FindTableBoxStartNode();
                const SwTableBox* pBox = pTblNd->GetTable().GetTblBox(
                                                pBoxStt->GetIndex() );
                const SwTableLine* pLn = pBox->GetUpper();
                pBox = pLn->GetTabBoxes().back();
                rIdx = *pBox->GetSttNd()->EndOfSectionNode();
                pPam->Move( fnMoveForward, fnGoNode );
            }
            nInsTblRow = static_cast< sal_uInt16 >(GetOpenBrakets());
            SetPardTokenRead( sal_False );
            SwPaM aTmp(*pPam);
            aTmp.Move( fnMoveBackward, fnGoNode );
        }
        ::SetProgressState( rInput.Tell(), pDoc->GetDocShell() );
        break;

    case RTF_INTBL:
        if (!CantUseTables())
        {
            if( !pTableNode )           // Tabelle nicht mehr vorhanden ?
            {
                if (RTF_TROWD != GetNextToken())
                    NewTblLine();           // evt. Line copieren
                SkipToken(-1);
            }
            else
            {
                // Crsr nicht mehr in der Tabelle ?
                if( !pPam->GetNode()->FindTableNode() )
                {
                    // dann wieder in die letzte Box setzen
                    // (kann durch einlesen von Flys geschehen!)
                    pPam->GetPoint()->nNode = *pTableNode->EndOfSectionNode();
                    pPam->Move( fnMoveBackward );
                }
            }
        }
        break;

    case RTF_REVTBL:
        ReadRevTbl();
        break;

    case RTF_REVISED:
        pRedlineInsert = new SwFltRedline(nsRedlineType_t::REDLINE_INSERT, 0, DateTime(Date( 0 ), Time( 0 )));
        break;

    case RTF_DELETED:
        pRedlineDelete = new SwFltRedline(nsRedlineType_t::REDLINE_DELETE, 0, DateTime(Date( 0 ), Time( 0 )));
        break;

    case RTF_REVAUTH:
        if (pRedlineInsert)
        {
            sal_uInt16 nRevAuth = static_cast<sal_uInt16>(nTokenValue);
            pRedlineInsert->nAutorNo = m_aAuthorInfos[nRevAuth];
        }
        break;

    case RTF_REVAUTHDEL:
        if(pRedlineDelete)
        {
            sal_uInt16 nRevAuthDel = static_cast<sal_uInt16>(nTokenValue);
            pRedlineDelete->nAutorNo = m_aAuthorInfos[nRevAuthDel];
        }
        break;

    case RTF_REVDTTM:
        if (pRedlineInsert != NULL)
            pRedlineInsert->aStamp = msfilter::util::DTTM2DateTime(nTokenValue);

        break;

    case RTF_REVDTTMDEL:
        pRedlineDelete->aStamp = msfilter::util::DTTM2DateTime(nTokenValue);
        break;


    case RTF_FLY_INPARA:
            // \pard  und plain ueberlesen !
        if( '}' != GetNextToken() && '}' != GetNextToken() )
        {
            // Zeichengebundener Fly in Fly
            ReadHeaderFooter( nToken );
            SetPardTokenRead( sal_False );
        }
        break;

    case RTF_PGDSCNO:
        if( IsNewDoc() && bSwPageDesc &&
            sal_uInt16(nTokenValue) < pDoc->GetPageDescCnt() )
        {
            const SwPageDesc* pPgDsc =
                &const_cast<const SwDoc *>(pDoc)
                ->GetPageDesc( sal_uInt16(nTokenValue) );
            CheckInsNewTblLine();
            pDoc->InsertPoolItem(*pPam, SwFmtPageDesc( pPgDsc ), 0);
        }
        break;

    case RTF_COLUM:
        pDoc->InsertPoolItem(*pPam,
                SvxFmtBreakItem( SVX_BREAK_COLUMN_BEFORE, RES_BREAK ), 0);
        break;

    case RTF_DXFRTEXT:      // werden nur im Zusammenhang mit Flys ausgewertet
    case RTF_DFRMTXTX:
    case RTF_DFRMTXTY:
        break;

    case RTF_CHDATE:    eDateFmt = DF_SHORT;    goto SETCHDATEFIELD;
    case RTF_CHDATEA:   eDateFmt = DF_SSYS;     goto SETCHDATEFIELD;
    case RTF_CHDATEL:   eDateFmt = DF_LSYS;     goto SETCHDATEFIELD;
SETCHDATEFIELD:
        {
            sal_uInt16 nSubType = DATEFLD, nWhich = RES_DATEFLD;
            sal_uLong nFormat = eDateFmt;
            sw3io_ConvertFromOldField( *pDoc, nWhich, nSubType, nFormat, 0x0110 );

            SwDateTimeField aDateFld( (SwDateTimeFieldType*)
                                        pDoc->GetSysFldType( RES_DATETIMEFLD ), DATEFLD, nFormat);
            CheckInsNewTblLine();
            pDoc->InsertPoolItem(*pPam, SwFmtFld( aDateFld ), 0);
        }
        break;

    case RTF_CHTIME:
        {
            sal_uInt16 nSubType = TIMEFLD, nWhich = RES_TIMEFLD;
            sal_uLong nFormat = TF_SSMM_24;
            sw3io_ConvertFromOldField( *pDoc, nWhich, nSubType, nFormat, 0x0110 );
            SwDateTimeField aTimeFld( (SwDateTimeFieldType*)
                    pDoc->GetSysFldType( RES_DATETIMEFLD ), TIMEFLD, nFormat);
            CheckInsNewTblLine();
            pDoc->InsertPoolItem(*pPam, SwFmtFld( aTimeFld ), 0);
        }
        break;

    case RTF_CHPGN:
        {
            SwPageNumberField aPageFld( (SwPageNumberFieldType*)
                                    pDoc->GetSysFldType( RES_PAGENUMBERFLD ),
                                    PG_RANDOM, SVX_NUM_ARABIC );
            CheckInsNewTblLine();
            pDoc->InsertPoolItem(*pPam, SwFmtFld(aPageFld), 0);
        }
        break;

    case RTF_CHFTN:
        bFootnoteAutoNum = sal_True;
        break;

    case RTF_NOFPAGES:
        if( IsNewDoc() && nTokenValue && -1 != nTokenValue )
            ((SwDocStat&)pDoc->GetDocStat()).nPage = (sal_uInt16)nTokenValue;
        break;

    case RTF_NOFWORDS:
        if( IsNewDoc() && nTokenValue && -1 != nTokenValue )
            ((SwDocStat&)pDoc->GetDocStat()).nWord = (sal_uInt16)nTokenValue;
        break;
    case RTF_NOFCHARS:
        if( IsNewDoc() && nTokenValue && -1 != nTokenValue )
            ((SwDocStat&)pDoc->GetDocStat()).nChar = (sal_uInt16)nTokenValue;
        break;
    case RTF_LYTPRTMET:
        if (IsNewDoc())
            pDoc->set(IDocumentSettingAccess::USE_VIRTUAL_DEVICE, false);
        break;
    case RTF_U:
        {
            CheckInsNewTblLine();
            if( nTokenValue )
                aToken = (sal_Unicode )nTokenValue;
            pDoc->InsertString( *pPam, aToken );
        }
        break;

    case RTF_USERPROPS:
        ReadUserProperties();       // #i28758 For now we don't support user properties
        break;

// RTF_SUBENTRYINDEX

    default:
        switch( nToken & ~(0xff | RTF_SWGDEFS) )
        {
        case RTF_DOCFMT:
            ReadDocControls( nToken );
            break;
        case RTF_SECTFMT:
            ReadSectControls( nToken );
            break;
        case RTF_APOCTL:
            if (nReadFlyDepth < 10)
            {
                nReadFlyDepth++;
                ReadFly( nToken );
                nReadFlyDepth--;
            }
            break;

        case RTF_BRDRDEF | RTF_TABLEDEF:
        case RTF_SHADINGDEF | RTF_TABLEDEF:
        case RTF_TABLEDEF:
            ReadTable( nToken );
            break;

        case RTF_INFO:
            ReadInfo();
            break;

        default:
            if( USHRT_MAX != nInsTblRow &&
                (nInsTblRow > GetOpenBrakets() || IsPardTokenRead() ))
                nInsTblRow = USHRT_MAX;

            SvxRTFParser::NextToken( nToken );
            break;
        }
    }
    if( USHRT_MAX != nInsTblRow &&
        (nInsTblRow > GetOpenBrakets() || IsPardTokenRead() ))
        nInsTblRow = USHRT_MAX;
}



void SwRTFParser::InsertText()
{
    bContainsPara = false;
    // dann fuege den String ein, ohne das Attribute am Ende
    // aufgespannt werden.
    CheckInsNewTblLine();

    if(pRedlineInsert)
        mpRedlineStack->open(*pPam->GetPoint(), *pRedlineInsert);
    if(pRedlineDelete)
        mpRedlineStack->open(*pPam->GetPoint(), *pRedlineDelete);

    pDoc->InsertString( *pPam, aToken );

    if(pRedlineDelete)
    {
        mpRedlineStack->close(*pPam->GetPoint(), pRedlineDelete->eType);
    }

    if(pRedlineInsert)
    {
        mpRedlineStack->close(*pPam->GetPoint(), pRedlineInsert->eType);
    }


}


void SwRTFParser::InsertPara()
{
    bContainsPara = true;
    CheckInsNewTblLine();
    pDoc->AppendTxtNode(*pPam->GetPoint());

    // setze das default Style
    if( !bStyleTabValid )
        MakeStyleTab();

    SwTxtFmtColl* pColl = NULL;
    std::map<sal_Int32,SwTxtFmtColl*>::iterator iter = aTxtCollTbl.find(0);

    if( iter == aTxtCollTbl.end() )
        pColl = pDoc->GetTxtCollFromPool( RES_POOLCOLL_STANDARD, false );
    else
        pColl = iter->second;

    pDoc->SetTxtFmtColl( *pPam, pColl );

    ::SetProgressState( rInput.Tell(), pDoc->GetDocShell() );
}



void SwRTFParser::MovePos( int bForward )
{
    if( bForward )
        pPam->Move( fnMoveForward );
    else
        pPam->Move( fnMoveBackward );
}

int SwRTFParser::IsEndPara( SvxNodeIdx* pNd, xub_StrLen nCnt ) const
{
    SwCntntNode *pNode = pDoc->GetNodes()[pNd->GetIdx()]->GetCntntNode();
    return pNode && pNode->Len() == nCnt;
}

bool SwRTFParser::UncompressableStackEntry(const SvxRTFItemStackType &rSet)
    const
{
    /*
    #i21961#
    Seeing as CHARFMT sets all the properties of the charfmt itself, its not
    good enough to just see it as a single property from the point of
    compressing property sets. If bold and charfmt are in a child, and bold is
    in the parent, removing bold from the child will not result in the same
    thing, if the charfmt removes bold itself for example
    */
    bool bRet = false;
    if (rSet.GetAttrSet().Count())
    {

        if (SFX_ITEM_SET ==
                rSet.GetAttrSet().GetItemState(RES_TXTATR_CHARFMT, sal_False))
        {
            bRet = true;
        }
    }
    return bRet;
}

void SwRTFParser::SetEndPrevPara( SvxNodeIdx*& rpNodePos, xub_StrLen& rCntPos )
{
    SwNodeIndex aIdx( pPam->GetPoint()->nNode );
    SwCntntNode* pNode = pDoc->GetNodes().GoPrevious( &aIdx );
    if( !pNode )
    {
        OSL_FAIL( "keinen vorherigen ContentNode gefunden" );
    }

    rpNodePos = new SwNodeIdx( aIdx );
    rCntPos = pNode->Len();
}

void SwRTFParser::SetAttrInDoc( SvxRTFItemStackType &rSet )
{
    sal_uLong nSNd = rSet.GetSttNodeIdx(), nENd = rSet.GetEndNodeIdx();
    xub_StrLen nSCnt = rSet.GetSttCnt(), nECnt = rSet.GetEndCnt();

    SwPaM aPam( *pPam->GetPoint() );

#if OSL_DEBUG_LEVEL > 0
    OSL_ENSURE( nSNd <= nENd, "Start groesser als Ende" );
    SwNode* pDebugNd = pDoc->GetNodes()[ nSNd ];
    OSL_ENSURE( pDebugNd->IsCntntNode(), "Start kein ContentNode" );
    pDebugNd = pDoc->GetNodes()[ nENd ];
    OSL_ENSURE( pDebugNd->IsCntntNode(), "Ende kein ContentNode" );
#endif

    SwCntntNode* pCNd = pDoc->GetNodes()[ nSNd ]->GetCntntNode();
    aPam.GetPoint()->nNode = nSNd;
    aPam.GetPoint()->nContent.Assign( pCNd, nSCnt );
    aPam.SetMark();
    if( nENd == nSNd )
        aPam.GetPoint()->nContent = nECnt;
    else
    {
        aPam.GetPoint()->nNode = nENd;
        pCNd = aPam.GetCntntNode();
        aPam.GetPoint()->nContent.Assign( pCNd, nECnt );
    }

    // setze ueber den Bereich das entsprechende Style
    if( rSet.StyleNo() )
    {
        // setze jetzt das Style
        if( !bStyleTabValid )
            MakeStyleTab();

        std::map<sal_Int32,SwTxtFmtColl*>::iterator iter = aTxtCollTbl.find(rSet.StyleNo());

        if (iter != aTxtCollTbl.end())
            pDoc->SetTxtFmtColl( aPam, iter->second, false );
    }

    const SfxPoolItem* pItem;
    const SfxPoolItem* pCharFmt;
    if (rSet.GetAttrSet().Count() )
    {

        // falls eine Zeichenvorlage im Set steht, deren Attribute
        // aus dem Set loeschen. Sonst sind diese doppelt, was man ja
        // nicht will.
        if( SFX_ITEM_SET == rSet.GetAttrSet().GetItemState(
            RES_TXTATR_CHARFMT, sal_False, &pCharFmt ) &&
            ((SwFmtCharFmt*)pCharFmt)->GetCharFmt() )
        {
            const String& rName = ((SwFmtCharFmt*)pCharFmt)->GetCharFmt()->GetName();
            for (SvxRTFStyleTbl::iterator it = GetStyleTbl().begin(); it != GetStyleTbl().end(); ++it)
            {
                SvxRTFStyleType* pStyle = it->second;
                if( pStyle->bIsCharFmt && pStyle->sName == rName )
                {
                    // alle Attribute, die schon vom Style definiert sind, aus dem
                    // akt. AttrSet entfernen
                    SfxItemSet &rAttrSet = rSet.GetAttrSet(),
                               &rStyleSet = pStyle->aAttrSet;
                    SfxItemIter aIter( rAttrSet );
                    sal_uInt16 nWhich = aIter.GetCurItem()->Which();
                    while( sal_True )
                    {
                        const SfxPoolItem* pI;
                        if( SFX_ITEM_SET == rStyleSet.GetItemState(
                            nWhich, sal_False, &pI ) && *pI == *aIter.GetCurItem())
                            rAttrSet.ClearItem( nWhich );       // loeschen

                        if( aIter.IsAtEnd() )
                            break;
                        nWhich = aIter.NextItem()->Which();
                    }
                    break;
                }
            }

            pDoc->InsertPoolItem(aPam, *pCharFmt, 0);
            rSet.GetAttrSet().ClearItem(RES_TXTATR_CHARFMT);     //test hack
        }
        if (rSet.GetAttrSet().Count())
        {
            // dann setze ueber diesen Bereich die Attrbiute
            SetSwgValues(rSet.GetAttrSet());
            pDoc->InsertItemSet(aPam, rSet.GetAttrSet(),
                    nsSetAttrMode::SETATTR_DONTCHGNUMRULE);
        }
    }

    if( SFX_ITEM_SET == rSet.GetAttrSet().GetItemState(
        FN_PARAM_NUM_LEVEL, sal_False, &pItem ))
    {
        // dann ueber den Bereich an den Nodes das NodeNum setzen
        for( sal_uLong n = nSNd; n <= nENd; ++n )
        {
            SwTxtNode* pTxtNd = pDoc->GetNodes()[ n ]->GetTxtNode();
            if( pTxtNd )
            {
                pTxtNd->SetAttrListLevel((sal_uInt8) ((SfxUInt16Item*)pItem)->GetValue());
                // Update vom LR-Space abschalten?
            }
        }
    }

    if( SFX_ITEM_SET == rSet.GetAttrSet().GetItemState(
        RES_PARATR_NUMRULE, sal_False, &pItem ))
    {
        const SwNumRule* pRule = pDoc->FindNumRulePtr(
                                    ((SwNumRuleItem*)pItem)->GetValue() );
        if( pRule && ( pRule->IsContinusNum() || !bNewNumList ))
        {
            // diese Rule hat keinen Level, also muss die Einrueckung
            // erhalten bleiben!
            // dann ueber den Bereich an den Nodes das Flag zuruecksetzen
            for( sal_uLong n = nSNd; n <= nENd; ++n )
            {
                SwTxtNode* pTxtNd = pDoc->GetNodes()[ n ]->GetTxtNode();
                if( pTxtNd )
                {
                    // Update vom LR-Space abschalten
                    pTxtNd->SetNumLSpace( sal_False );
                }
            }
        }
    }

    bool bNoNum = true;
    if (
        (SFX_ITEM_SET == rSet.GetAttrSet().GetItemState(RES_PARATR_NUMRULE))
     || (SFX_ITEM_SET == rSet.GetAttrSet().GetItemState(FN_PARAM_NUM_LEVEL))
       )
    {
        bNoNum = false;
    }

    if (bNoNum)
    {
        for( sal_uLong n = nSNd; n <= nENd; ++n )
        {
            SwTxtNode* pTxtNd = pDoc->GetNodes()[ n ]->GetTxtNode();
            if( pTxtNd )
            {
                pTxtNd->SetAttr(
                    *GetDfltAttr(RES_PARATR_NUMRULE));
            }
        }
    }
}

DocPageInformation::DocPageInformation()
    : maBox( RES_BOX ),
    mnPaperw(12240), mnPaperh(15840), mnMargl(1800), mnMargr(1800),
    mnMargt(1440), mnMargb(1440), mnGutter(0), mnPgnStart(1), mbFacingp(false),
    mbLandscape(false), mbRTLdoc(false)
{
}

SectPageInformation::SectPageInformation(const DocPageInformation &rDoc)
    : maBox(rDoc.maBox), mpTitlePageHdFt(0), mpPageHdFt(0),
    mnPgwsxn(rDoc.mnPaperw), mnPghsxn(rDoc.mnPaperh), mnMarglsxn(rDoc.mnMargl),
    mnMargrsxn(rDoc.mnMargr), mnMargtsxn(rDoc.mnMargt),
    mnMargbsxn(rDoc.mnMargb), mnGutterxsn(rDoc.mnGutter), mnHeadery(720),
    mnFootery(720), mnPgnStarts(rDoc.mnPgnStart), mnCols(1), mnColsx(720),
    mnStextflow(rDoc.mbRTLdoc ? 3 : 0), mnBkc(2), mbLndscpsxn(rDoc.mbLandscape),
    mbTitlepg(false), mbFacpgsxn(rDoc.mbFacingp), mbRTLsection(rDoc.mbRTLdoc),
    mbPgnrestart(false), mbTitlePageHdFtUsed(false), mbPageHdFtUsed(false)
{
};

SectPageInformation::SectPageInformation(const SectPageInformation &rSect)
    : maColumns(rSect.maColumns), maBox(rSect.maBox),
    maNumType(rSect.maNumType), mpTitlePageHdFt(rSect.mpTitlePageHdFt),
    mpPageHdFt(rSect.mpPageHdFt), mnPgwsxn(rSect.mnPgwsxn),
    mnPghsxn(rSect.mnPghsxn), mnMarglsxn(rSect.mnMarglsxn),
    mnMargrsxn(rSect.mnMargrsxn), mnMargtsxn(rSect.mnMargtsxn),
    mnMargbsxn(rSect.mnMargbsxn), mnGutterxsn(rSect.mnGutterxsn),
    mnHeadery(rSect.mnHeadery), mnFootery(rSect.mnFootery),
    mnPgnStarts(rSect.mnPgnStarts), mnCols(rSect.mnCols),
    mnColsx(rSect.mnColsx), mnStextflow(rSect.mnStextflow), mnBkc(rSect.mnBkc),
    mbLndscpsxn(rSect.mbLndscpsxn), mbTitlepg(rSect.mbTitlepg),
    mbFacpgsxn(rSect.mbFacpgsxn), mbRTLsection(rSect.mbRTLsection),
    mbPgnrestart(rSect.mbPgnrestart),
    mbTitlePageHdFtUsed(rSect.mbTitlePageHdFtUsed),
    mbPageHdFtUsed(rSect.mbPageHdFtUsed)
{
};

rtfSection::rtfSection(const SwPosition &rPos,
    const SectPageInformation &rPageInfo)
    : maStart(rPos.nNode), maPageInfo(rPageInfo), mpSection(0), mpTitlePage(0),
    mpPage(0)
{
}

void rtfSections::push_back(const rtfSection &rSect)
{
    if (!maSegments.empty() && (maSegments.back().maStart == rSect.maStart))
        maSegments.pop_back();
    maSegments.push_back(rSect);
}

// lese alle Dokument-Controls ein
void SwRTFParser::SetPageInformationAsDefault(const DocPageInformation &rInfo)
{
    //If we are at the beginning of the document then start the document with
    //a segment with these properties. See #i14982# for this requirement
    rtfSection aSect(*pPam->GetPoint(), SectPageInformation(rInfo));
    if (maSegments.empty() || (maSegments.back().maStart == aSect.maStart))
        maSegments.push_back(aSect);

    if (!bSwPageDesc && IsNewDoc())
    {
        SwFmtFrmSize aFrmSize(ATT_FIX_SIZE, rInfo.mnPaperw, rInfo.mnPaperh);

        SvxLRSpaceItem aLR( static_cast< sal_uInt16 >(rInfo.mnMargl), static_cast< sal_uInt16 >(rInfo.mnMargr), 0, 0, RES_LR_SPACE );
        SvxULSpaceItem aUL( static_cast< sal_uInt16 >(rInfo.mnMargt), static_cast< sal_uInt16 >(rInfo.mnMargb), RES_UL_SPACE );

        UseOnPage eUseOn;
        if (rInfo.mbFacingp)
            eUseOn = UseOnPage(nsUseOnPage::PD_MIRROR | nsUseOnPage::PD_HEADERSHARE | nsUseOnPage::PD_FOOTERSHARE);
        else
            eUseOn = UseOnPage(nsUseOnPage::PD_ALL | nsUseOnPage::PD_HEADERSHARE | nsUseOnPage::PD_FOOTERSHARE);

        sal_uInt16 nPgStart = static_cast< sal_uInt16 >(rInfo.mnPgnStart);

        SvxFrameDirectionItem aFrmDir(rInfo.mbRTLdoc ?
            FRMDIR_HORI_RIGHT_TOP : FRMDIR_HORI_LEFT_TOP, RES_FRAMEDIR);

        // direkt an der Standartseite drehen
        SwPageDesc& rPg = pDoc->GetPageDesc( 0 );
        rPg.WriteUseOn( eUseOn );

        if (rInfo.mbLandscape)
            rPg.SetLandscape(true);

        SwFrmFmt &rFmt1 = rPg.GetMaster(), &rFmt2 = rPg.GetLeft();

        rFmt1.SetFmtAttr( aFrmSize );   rFmt2.SetFmtAttr( aFrmSize );
        rFmt1.SetFmtAttr( aLR );        rFmt2.SetFmtAttr( aLR );
        rFmt1.SetFmtAttr( aUL );       rFmt2.SetFmtAttr( aUL );
        rFmt1.SetFmtAttr( aFrmDir );   rFmt2.SetFmtAttr( aFrmDir );

        // StartNummer der Seiten setzen
        if (nPgStart  != 1)
        {
            SwFmtPageDesc aPgDsc( &rPg );
            aPgDsc.SetNumOffset( nPgStart );
            pDoc->InsertPoolItem( *pPam, aPgDsc, 0 );
        }
    }
}

void SwRTFParser::SetBorderLine(SvxBoxItem& rBox, sal_uInt16 nLine)
{
    int bWeiter = true;
    short nLineThickness = 1;
    short nPageDistance = 0;
    sal_uInt8 nCol = 0;
    short nIdx = 0;

    int nToken = GetNextToken();
    do {
        switch( nToken )
        {
        case RTF_BRDRS:
            nIdx = 1;
            break;

        case RTF_BRDRDB:
            nIdx = 3;
            break;

        case RTF_BRDRTRIPLE:
            nIdx = 10;
            break;

        case RTF_BRDRTNTHSG:
            nIdx = 11;
            break;

        case RTF_BRDRTHTNSG:
            nIdx = 12;
            break;

        case RTF_BRDRTNTHTNSG:
            nIdx = 13;
            break;

        case RTF_BRDRTNTHMG:
            nIdx = 14;
            break;

        case RTF_BRDRTHTNMG:
            nIdx = 15;
            break;

        case RTF_BRDRTNTHTNMG:
            nIdx = 16;
            break;

        case RTF_BRDRTNTHLG:
            nIdx = 17;
            break;

        case RTF_BRDRTHTNLG:
            nIdx = 18;
            break;

        case RTF_BRDRTNTHTNLG:
            nIdx = 19;
            break;

        case RTF_BRDRWAVY:
            nIdx = 20;
            break;

        case RTF_BRDRWAVYDB:
            nIdx = 21;
            break;

        case RTF_BRDREMBOSS:
            nIdx = 24;
            break;

        case RTF_BRDRENGRAVE:
            nIdx = 25;
            break;

        case RTF_BRSP:
            nPageDistance = static_cast< short >(nTokenValue);
            break;

        case RTF_BRDRDOT:           // SO does not have dashed or dotted lines
            nIdx = 6;
            break;
        case RTF_BRDRDASH:
            nIdx = 7;
            break;
        case RTF_BRDRDASHSM:
        case RTF_BRDRDASHD:
        case RTF_BRDRDASHDD:
        case RTF_BRDRDASHDOTSTR:
        case RTF_BRDRSH:            // shading not supported
        case RTF_BRDRCF:            // colors not supported
            break;

        case RTF_BRDRW:
            nLineThickness = static_cast< short >(nTokenValue);
            break;
        default:
            bWeiter = false;
            SkipToken(-1);
            break;
        }
        if (bWeiter)
            nToken = GetNextToken();
    } while (bWeiter && IsParserWorking());

    GetLineIndex(rBox, nLineThickness, nPageDistance, nCol, nIdx, nLine, nLine, 0);
}

// lese alle Dokument-Controls ein
void SwRTFParser::ReadDocControls( int nToken )
{
    int bWeiter = true;

    SwFtnInfo aFtnInfo;
    SwEndNoteInfo aEndInfo;
    bool bSetHyph = false;

    sal_Bool bEndInfoChgd = sal_False, bFtnInfoChgd = sal_False;

    do {
        sal_uInt16 nValue = sal_uInt16( nTokenValue );
        switch( nToken )
        {
        case RTF_RTLDOC:
            maPageDefaults.mbRTLdoc = true;
            break;
        case RTF_LTRDOC:
            maPageDefaults.mbRTLdoc = false;
            break;
        case RTF_LANDSCAPE:
            maPageDefaults.mbLandscape = true;
            break;
        case RTF_PAPERW:
            if( 0 < nTokenValue )
                maPageDefaults.mnPaperw = nTokenValue;
            break;
        case RTF_PAPERH:
            if( 0 < nTokenValue )
                maPageDefaults.mnPaperh = nTokenValue;
            break;
        case RTF_MARGL:
            if( 0 <= nTokenValue )
                maPageDefaults.mnMargl = nTokenValue;
            break;
        case RTF_MARGR:
            if( 0 <= nTokenValue )
                maPageDefaults.mnMargr = nTokenValue;
            break;
        case RTF_MARGT:
            if( 0 <= nTokenValue )
                maPageDefaults.mnMargt = nTokenValue;
            break;
        case RTF_MARGB:
            if( 0 <= nTokenValue )
                maPageDefaults.mnMargb = nTokenValue;
            break;
        case RTF_FACINGP:
            maPageDefaults.mbFacingp = true;
            break;
        case RTF_PGNSTART:
            maPageDefaults.mnPgnStart = nTokenValue;
            break;
        case RTF_ENDDOC:
        case RTF_ENDNOTES:
            aFtnInfo.ePos = FTNPOS_CHAPTER; bFtnInfoChgd = sal_True;
            break;
        case RTF_FTNTJ:
        case RTF_FTNBJ:
            aFtnInfo.ePos = FTNPOS_PAGE; bFtnInfoChgd = sal_True;
            break;

        case RTF_AENDDOC:
        case RTF_AENDNOTES:
        case RTF_AFTNTJ:
        case RTF_AFTNBJ:
        case RTF_AFTNRESTART:
        case RTF_AFTNRSTCONT:
            break;      // wir kenn nur am Doc Ende und Doc weite Num.!

        case RTF_FTNSTART:
            if( nValue )
            {
                aFtnInfo.nFtnOffset = nValue-1;
                bFtnInfoChgd = sal_True;
            }
            break;
        case RTF_AFTNSTART:
            if( nValue )
            {
                aEndInfo.nFtnOffset = nValue-1;
                bEndInfoChgd = sal_True;
            }
            break;
        case RTF_FTNRSTPG:
            aFtnInfo.eNum = FTNNUM_PAGE; bFtnInfoChgd = sal_True;
            break;
        case RTF_FTNRESTART:
            aFtnInfo.eNum = FTNNUM_CHAPTER; bFtnInfoChgd = sal_True;
            break;
        case RTF_FTNRSTCONT:
            aFtnInfo.eNum = FTNNUM_DOC; bFtnInfoChgd = sal_True;
            break;

        case RTF_FTNNAR:
            aFtnInfo.aFmt.SetNumberingType(SVX_NUM_ARABIC); bFtnInfoChgd = sal_True; break;
        case RTF_FTNNALC:
            aFtnInfo.aFmt.SetNumberingType(SVX_NUM_CHARS_LOWER_LETTER_N); bFtnInfoChgd = sal_True; break;
        case RTF_FTNNAUC:
            aFtnInfo.aFmt.SetNumberingType(SVX_NUM_CHARS_UPPER_LETTER_N); bFtnInfoChgd = sal_True; break;
        case RTF_FTNNRLC:
            aFtnInfo.aFmt.SetNumberingType(SVX_NUM_ROMAN_LOWER); bFtnInfoChgd = sal_True; break;
        case RTF_FTNNRUC:
            aFtnInfo.aFmt.SetNumberingType(SVX_NUM_ROMAN_UPPER); bFtnInfoChgd = sal_True; break;
        case RTF_FTNNCHI:
            aFtnInfo.aFmt.SetNumberingType(SVX_NUM_CHAR_SPECIAL); bFtnInfoChgd = sal_True; break;

        case RTF_AFTNNAR:
            aEndInfo.aFmt.SetNumberingType(SVX_NUM_ARABIC); bEndInfoChgd = sal_True; break;
        case RTF_AFTNNALC:
            aEndInfo.aFmt.SetNumberingType(SVX_NUM_CHARS_LOWER_LETTER_N);
            bEndInfoChgd = sal_True;
            break;
        case RTF_AFTNNAUC:
            aEndInfo.aFmt.SetNumberingType(SVX_NUM_CHARS_UPPER_LETTER_N);
            bEndInfoChgd = sal_True;
            break;
        case RTF_AFTNNRLC:
            aEndInfo.aFmt.SetNumberingType(SVX_NUM_ROMAN_LOWER);
            bEndInfoChgd = sal_True;
            break;
        case RTF_AFTNNRUC:
            aEndInfo.aFmt.SetNumberingType(SVX_NUM_ROMAN_UPPER);
            bEndInfoChgd = sal_True;
            break;
        case RTF_AFTNNCHI:
            aEndInfo.aFmt.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
            bEndInfoChgd = sal_True;
            break;
        case RTF_HYPHAUTO:
            if (nTokenValue)
                bSetHyph = true;
            //FOO//
            break;
        case RTF_PGBRDRT:
            SetBorderLine(maPageDefaults.maBox, BOX_LINE_TOP);
            break;

        case RTF_PGBRDRB:
            SetBorderLine(maPageDefaults.maBox, BOX_LINE_BOTTOM);
            break;

        case RTF_PGBRDRL:
            SetBorderLine(maPageDefaults.maBox, BOX_LINE_LEFT);
            break;

        case RTF_PGBRDRR:
            SetBorderLine(maPageDefaults.maBox, BOX_LINE_RIGHT);
            break;

        case '{':
            {
                short nSkip = 0;
                if( RTF_IGNOREFLAG != GetNextToken() )
                    nSkip = -1;
                else if( RTF_DOCFMT != (( nToken = GetNextToken() )
                        & ~(0xff | RTF_SWGDEFS)) )
                    nSkip = -2;
                else
                {
                    SkipGroup();        // erstmal komplett ueberlesen
                    // ueberlese noch die schliessende Klammer
                    GetNextToken();
                }
                if( nSkip )
                {
                    SkipToken( nSkip );     // Ignore wieder zurueck
                    bWeiter = sal_False;
                }
            }
            break;

        default:
            if( RTF_DOCFMT == (nToken & ~(0xff | RTF_SWGDEFS)) ||
                RTF_UNKNOWNCONTROL == nToken )
                SvxRTFParser::NextToken( nToken );
            else
                bWeiter = sal_False;
            break;
        }
        if( bWeiter )
            nToken = GetNextToken();
    } while( bWeiter && IsParserWorking() );

    if (IsNewDoc())
    {
        if( bEndInfoChgd )
            pDoc->SetEndNoteInfo( aEndInfo );
        if( bFtnInfoChgd )
            pDoc->SetFtnInfo( aFtnInfo );
    }

    if (!bSwPageDesc)
    {
        SetPageInformationAsDefault(maPageDefaults);

        MakeStyleTab();

        SwTxtFmtColl *pColl = NULL;
        std::map<sal_Int32,SwTxtFmtColl*>::iterator iter = aTxtCollTbl.find(0);

        if (iter == aTxtCollTbl.end())
            pColl = pDoc->GetTxtCollFromPool(RES_POOLCOLL_STANDARD, false );
        else
            pColl = iter->second;

        OSL_ENSURE(pColl, "impossible to have no standard style");

        if (pColl)
        {
            if (
                IsNewDoc() && bSetHyph &&
                SFX_ITEM_SET != pColl->GetItemState(RES_PARATR_HYPHENZONE,
                false)
               )
            {
                pColl->SetFmtAttr(SvxHyphenZoneItem(true, RES_PARATR_HYPHENZONE));
            }

            pDoc->SetTxtFmtColl( *pPam, pColl );
        }
    }

    SkipToken( -1 );
}

void SwRTFParser::MakeStyleTab()
{
    // dann erzeuge aus der SvxStyle-Tabelle die Swg-Collections
    if( !GetStyleTbl().empty() )
    {
        sal_uInt16 nValidOutlineLevels = 0;
        if( !IsNewDoc() )
        {
            // search all outlined collections
            const SwTxtFmtColls& rColls = *pDoc->GetTxtFmtColls();
            for( sal_uInt16 n = rColls.size(); n; )
                if( rColls[ --n ]->IsAssignedToListLevelOfOutlineStyle())
                    nValidOutlineLevels |= 1 << rColls[ n ]->GetAssignedOutlineStyleLevel();//<-end,zhaojianwei
        }

        for (SvxRTFStyleTbl::iterator it = GetStyleTbl().begin(); it != GetStyleTbl().end(); ++it)
        {
            sal_uInt16 nNo = it->first;
            SvxRTFStyleType* pStyle = it->second;
            if( pStyle->bIsCharFmt )
            {
                if(aCharFmtTbl.find( nNo ) == aCharFmtTbl.end())
                    // existiert noch nicht, also anlegen
                    MakeCharStyle( nNo, *pStyle );
            }
            else if( aTxtCollTbl.find( nNo ) == aTxtCollTbl.end() )
            {
                // existiert noch nicht, also anlegen
                MakeStyle( nNo, *pStyle );
            }

        }
        bStyleTabValid = sal_True;
    }
}

static sal_Bool lcl_SetFmtCol( SwFmt& rFmt, sal_uInt16 nCols, sal_uInt16 nColSpace,
                    const std::vector<sal_uInt16>& rColumns )
{
    sal_Bool bSet = sal_False;
    if( nCols && USHRT_MAX != nCols )
    {
        SwFmtCol aCol;
        if( USHRT_MAX == nColSpace )
            nColSpace = 720;

        aCol.Init( nCols, nColSpace, USHRT_MAX );
        if( nCols == ( rColumns.size() / 2 ) )
        {
            aCol._SetOrtho( sal_False );
            sal_uInt16 nWishWidth = 0, nHalfPrev = 0;
            for (sal_uInt16 n = 0, i = 0; static_cast<size_t>(n+1) < rColumns.size(); n += 2, ++i)
            {
                SwColumn* pCol = &aCol.GetColumns()[ i ];
                pCol->SetLeft( nHalfPrev );
                sal_uInt16 nSp = rColumns[ n+1 ];
                nHalfPrev = nSp / 2;
                pCol->SetRight( nSp - nHalfPrev );
                pCol->SetWishWidth( rColumns[ n ] +
                                    pCol->GetLeft() + pCol->GetRight() );
                nWishWidth = nWishWidth + pCol->GetWishWidth();
            }
            aCol.SetWishWidth( nWishWidth );
        }
        rFmt.SetFmtAttr( aCol );
        bSet = sal_True;
    }
    return bSet;
}

void SwRTFParser::DoHairyWriterPageDesc(int nToken)
{
    int bWeiter = sal_True;
    do {
        if( '{' == nToken )
        {
            switch( nToken = GetNextToken() )
            {
            case RTF_IGNOREFLAG:
                if( RTF_SECTFMT != (( nToken = GetNextToken() )
                    & ~(0xff | RTF_SWGDEFS)) )
                {
                    SkipToken( -2 );    // Ignore und Token wieder zurueck
                    bWeiter = sal_False;
                    break;
                }
                // kein break, Gruppe ueberspringen

            case RTF_FOOTER:
            case RTF_HEADER:
            case RTF_FOOTERR:
            case RTF_HEADERR:
            case RTF_FOOTERL:
            case RTF_HEADERL:
            case RTF_FOOTERF:
            case RTF_HEADERF:
                SkipGroup();        // erstmal komplett ueberlesen
                // ueberlese noch die schliessende Klammer
                GetNextToken();
                break;

            default:
                SkipToken( -1 );            // Ignore wieder zurueck
                bWeiter = sal_False;
                break;
            }
        }
        else if( RTF_SECTFMT == (nToken & ~(0xff | RTF_SWGDEFS)) ||
            RTF_UNKNOWNCONTROL == nToken )
            SvxRTFParser::NextToken( nToken );
        else
            bWeiter = sal_False;
        if( bWeiter )
            nToken = GetNextToken();
    } while( bWeiter && IsParserWorking() );
    SkipToken( -1 );                    // letztes Token wieder zurueck
    return;
}

void SwRTFParser::ReadSectControls( int nToken )
{
    //this is some hairy stuff to try and retain writer style page descriptors
    //in rtf, almost certainy a bad idea, but we've inherited it, so here it
    //stays
    if (bInPgDscTbl)
    {
        DoHairyWriterPageDesc(nToken);
        return;
    }

    OSL_ENSURE(!maSegments.empty(), "suspicious to have a section with no "
        "page info, though probably legal");
    if (maSegments.empty())
    {
        maSegments.push_back(rtfSection(*pPam->GetPoint(),
            SectPageInformation(maPageDefaults)));
    }

    SectPageInformation aNewSection(maSegments.back().maPageInfo);

    bool bNewSection = false;
    bool bNewSectionHeader = false;
    const SwFmtHeader* _pKeepHeader = NULL;
    const SwFmtFooter* _pKeepFooter = NULL;
    int bWeiter = true;
    bool bKeepFooter = false;
    do {
        sal_uInt16 nValue = sal_uInt16( nTokenValue );
        switch( nToken )
        {
            case RTF_SECT:
                bNewSection = true;
                bForceNewTable = true;
                break;
            case RTF_SECTD: {
                //Reset to page defaults
                SwPageDesc* oldPageDesc=aNewSection.mpPageHdFt;
                aNewSection = SectPageInformation(maPageDefaults);
                aNewSection.mpPageHdFt=oldPageDesc;
                _pKeepHeader = NULL;
                _pKeepFooter = NULL;
                } break;
            case RTF_PGWSXN:
                if (0 < nTokenValue)
                    aNewSection.mnPgwsxn = nTokenValue;
                break;
            case RTF_PGHSXN:
                if (0 < nTokenValue)
                    aNewSection.mnPghsxn = nTokenValue;
                break;
            case RTF_MARGLSXN:
                if (0 <= nTokenValue)
                    aNewSection.mnMarglsxn = nTokenValue;
                break;
            case RTF_MARGRSXN:
                if (0 <= nTokenValue)
                    aNewSection.mnMargrsxn = nTokenValue;
                break;
            case RTF_MARGTSXN:
                if (0 <= nTokenValue)
                    aNewSection.mnMargtsxn = nTokenValue;
                break;
            case RTF_MARGBSXN:
                if (0 <= nTokenValue)
                    aNewSection.mnMargbsxn = nTokenValue;
                break;
            case RTF_FACPGSXN:
                aNewSection.mbFacpgsxn = true;
                break;
            case RTF_HEADERY:
                aNewSection.mnHeadery = nTokenValue;
                break;
            case RTF_FOOTERY:
                aNewSection.mnFootery = nTokenValue;
                break;
            case RTF_LNDSCPSXN:
                aNewSection.mbLndscpsxn = true;
                break;
            case RTF_PGNSTARTS:
                aNewSection.mnPgnStarts = nTokenValue;
                break;
            case RTF_PGNDEC:
                aNewSection.maNumType.SetNumberingType(SVX_NUM_ARABIC);
                break;
            case RTF_PGNUCRM:
                aNewSection.maNumType.SetNumberingType(SVX_NUM_ROMAN_UPPER);
                break;
            case RTF_PGNLCRM:
                aNewSection.maNumType.SetNumberingType(SVX_NUM_ROMAN_LOWER);
                break;
            case RTF_PGNUCLTR:
                aNewSection.maNumType.SetNumberingType(
                    SVX_NUM_CHARS_UPPER_LETTER_N);
                break;
            case RTF_PGNLCLTR:
                aNewSection.maNumType.SetNumberingType(
                    SVX_NUM_CHARS_LOWER_LETTER_N);
                break;
            case RTF_SBKNONE:
                aNewSection.mnBkc = 0;
                break;
            case RTF_SBKCOL:
                aNewSection.mnBkc = 1;
                break;
            case RTF_PGBRDRT:
                SetBorderLine(aNewSection.maBox, BOX_LINE_TOP);
                break;

            case RTF_PGBRDRB:
                SetBorderLine(aNewSection.maBox, BOX_LINE_BOTTOM);
                break;

            case RTF_PGBRDRL:
                SetBorderLine(aNewSection.maBox, BOX_LINE_LEFT);
                break;

            case RTF_PGBRDRR:
                SetBorderLine(aNewSection.maBox, BOX_LINE_RIGHT);
                break;

            case RTF_PGBRDROPT:
            case RTF_ENDNHERE:
            case RTF_BINFSXN:
            case RTF_BINSXN:
            case RTF_SBKPAGE:
            case RTF_SBKEVEN:
            case RTF_SBKODD:
            case RTF_LINEBETCOL:
            case RTF_LINEMOD:
            case RTF_LINEX:
            case RTF_LINESTARTS:
            case RTF_LINERESTART:
            case RTF_LINEPAGE:
            case RTF_LINECONT:
            case RTF_GUTTERSXN:
            case RTF_PGNCONT:
            case RTF_PGNRESTART:
            case RTF_PGNX:
            case RTF_PGNY:
            case RTF_VERTALT:
            case RTF_VERTALB:
            case RTF_VERTALC:
            case RTF_VERTALJ:
                break;
            case RTF_TITLEPG:
                aNewSection.mbTitlepg = true;
                break;
            case RTF_HEADER:
            case RTF_HEADERL:
            case RTF_HEADERR:
                if (aNewSection.mpPageHdFt!=NULL)
                {
                    _pKeepHeader = NULL;
                    bKeepFooter = true; // #i82008
                    _pKeepFooter = &aNewSection.mpPageHdFt->GetMaster().GetFooter();
                }
            case RTF_FOOTER:
            case RTF_FOOTERL:
            case RTF_FOOTERR:
                if (aNewSection.mpPageHdFt!=NULL && !bKeepFooter )
                {
                    _pKeepFooter = NULL;
                    _pKeepHeader = &aNewSection.mpPageHdFt->GetMaster().GetHeader();
                }
                bKeepFooter = false;
                if (!bNewSectionHeader) { // If a header is redefined in a section
                    bNewSectionHeader=true;           //  a new header must be created.
                    aNewSection.mpPageHdFt=NULL;
                }
                if (!aNewSection.mpPageHdFt)
                {
                    String aName(RTL_CONSTASCII_USTRINGPARAM("rtfHdFt"));
                    aName += String::CreateFromInt32(maSegments.size());
                    sal_uInt16 nPageNo = pDoc->MakePageDesc(aName);
                    aNewSection.mpPageHdFt = &pDoc->GetPageDesc(nPageNo);
                    aNewSection.mbPageHdFtUsed = true;
                    maSegments.maDummyPageNos.push_back(nPageNo);
                }
                ReadHeaderFooter(nToken, aNewSection.mpPageHdFt);
                if (_pKeepHeader) aNewSection.mpPageHdFt->GetMaster().SetFmtAttr(*_pKeepHeader);
                if (_pKeepFooter) aNewSection.mpPageHdFt->GetMaster().SetFmtAttr(*_pKeepFooter);
                break;
            case RTF_FOOTERF:
            case RTF_HEADERF:
                if (!aNewSection.mpTitlePageHdFt)
                {
                    String aTitle(RTL_CONSTASCII_USTRINGPARAM("rtfTitleHdFt"));
                    aTitle += String::CreateFromInt32(maSegments.size());
                    sal_uInt16 nPageNo = pDoc->MakePageDesc(aTitle);
                    aNewSection.mpTitlePageHdFt = &pDoc->GetPageDesc(nPageNo);
                    aNewSection.mbTitlePageHdFtUsed = true;
                    maSegments.maDummyPageNos.push_back(nPageNo);
                }
                ReadHeaderFooter(nToken, aNewSection.mpTitlePageHdFt);
                break;
            case RTF_COLS:
                aNewSection.mnCols = nTokenValue;
                break;
            case RTF_COLSX:
                aNewSection.mnColsx = nTokenValue;
                break;
            case RTF_COLNO:
                {
                    // next token must be either colw or colsr
                    unsigned long nAktCol = nValue;
                    long nWidth = 0, nSpace = 0;
                    int nColToken = GetNextToken();
                    if (RTF_COLW == nColToken)
                    {
                        // next token could be colsr (but not required)
                        nWidth = nTokenValue;
                        if( RTF_COLSR == GetNextToken() )
                            nSpace = nTokenValue;
                        else
                            SkipToken( -1 );        // put back token
                    }
                    else if (RTF_COLSR == nColToken)
                    {
                        // next token must be colw (what sense should it make to have colsr only?!)
                        nSpace = nTokenValue;
                        if( RTF_COLW == GetNextToken() )
                            nWidth = nTokenValue;
                        else
                            // what should we do if an isolated colsr without colw is found? Doesn't make sense!
                            SkipToken( -1 );        // put back token
                    }
                    else
                        break;

                    if (--nAktCol == (aNewSection.maColumns.size() / 2))
                    {
                        aNewSection.maColumns.push_back(nWidth);
                        aNewSection.maColumns.push_back(nSpace);
                    }
                }
                break;
            case RTF_STEXTFLOW:
                aNewSection.mnStextflow = nTokenValue;
                break;
            case RTF_RTLSECT:
                aNewSection.mbRTLsection = true;
                break;
            case RTF_LTRSECT:
                aNewSection.mbRTLsection = false;
                break;
            case '{':
                {
                    short nSkip = 0;
                    if( RTF_IGNOREFLAG != ( nToken = GetNextToken() ))
                        nSkip = -1;
                    else if( RTF_SECTFMT != (( nToken = GetNextToken() )
                             & ~(0xff | RTF_SWGDEFS)) &&
                            ( RTF_DOCFMT != ( nToken & ~(0xff | RTF_SWGDEFS))) )
                        nSkip = -2;
                    else
                    {
                        // erstmal komplett ueberlesen
                        SkipGroup();
                        // ueberlese noch die schliessende Klammer
                        GetNextToken();
                    }
                    if (nSkip)
                    {
                        bWeiter = ((-1 == nSkip) &&
                            (
                              RTF_FOOTER == nToken || RTF_HEADER == nToken ||
                              RTF_FOOTERR == nToken || RTF_HEADERR == nToken ||
                              RTF_FOOTERL == nToken || RTF_HEADERL == nToken ||
                              RTF_FOOTERF == nToken || RTF_HEADERF == nToken
                            ));
                        SkipToken (nSkip);      // Ignore wieder zurueck
                    }
                }
                break;
            case RTF_PAPERW:
            case RTF_PAPERH:
            case RTF_MARGL:
            case RTF_MARGR:
            case RTF_MARGT:
            case RTF_MARGB:
            case RTF_FACINGP:
                OSL_ENSURE(!this, "why are these tokens found in this section?");
                ReadDocControls( nToken );
                break;
            default:
                if (RTF_DOCFMT == (nToken & ~(0xff | RTF_SWGDEFS)))
                    ReadDocControls( nToken );
                else if (RTF_SECTFMT == (nToken & ~(0xff | RTF_SWGDEFS)) ||
                         RTF_UNKNOWNCONTROL == nToken)
                {
                    SvxRTFParser::NextToken(nToken);
                }
                else
                    bWeiter = false;
                break;
        }

        if (bWeiter)
            nToken = GetNextToken();
    } while (bWeiter && IsParserWorking());

    if (bNewSection || maSegments.empty())
    {
        AttrGroupEnd();
        if(!bContainsPara && !bContainsTablePara) // bContainsTablePara is set in rtftbl.cxx
            pDoc->AppendTxtNode(*pPam->GetPoint());
        bContainsPara = false;
        bContainsTablePara = false;
        maSegments.push_back(rtfSection(*pPam->GetPoint(), aNewSection));
    }
    else //modifying/replacing the current section
    {
        SwPaM aPamStart(maSegments.back().maStart);
        maSegments.pop_back();
        maSegments.push_back(rtfSection(*aPamStart.GetPoint(), aNewSection));
    }

    SkipToken(-1);
}

void SwRTFParser::EnterEnvironment()
{
}


void SwRTFParser::LeaveEnvironment()
{
    if(pRedlineDelete)
    {
        delete pRedlineDelete;
        pRedlineDelete = 0;
    }

    if(pRedlineInsert)
    {
        delete pRedlineInsert;
        pRedlineInsert = 0;
    }
}

void SwRTFParser::SkipPageDescTbl()
{
    // I have to use this glorified SkipGroup because the
    // SvParser SkipGroup uses nNextCh which is not set correctly <groan>
    int nNumOpenBrakets = 1;

    while( nNumOpenBrakets && IsParserWorking() )
    {
        switch( GetNextToken() )
        {
        case '}':
            {
                --nNumOpenBrakets;
            }
            break;

        case '{':
            {
                nNumOpenBrakets++;
            }
            break;
        }
    }

    SkipToken( -1 );
}


#define SETPAGEDESC_DEFAULTS() \
    do {\
        aSz.SetWidth( a4.Width() ); aSz.SetHeight( a4.Height() );\
        aLR.SetLeft( 0 );   aLR.SetRight( 0 ); \
        aUL.SetLower( 0 );  aUL.SetUpper( 0 ); \
        aHLR.SetLeft( 0 );  aHLR.SetRight( 0 ); \
        aHUL.SetLower( 0 ); aHUL.SetUpper( 0 ); \
        aFLR.SetLeft( 0 );  aFLR.SetRight( 0 ); \
        aFUL.SetLower( 0 ); aFUL.SetUpper( 0 ); \
        nCols = USHRT_MAX; nColSpace = USHRT_MAX; nAktCol = 0; \
        aFSz.SetHeightSizeType( ATT_MIN_SIZE ); aFSz.SetHeight( 0 ); \
        aHSz.SetHeightSizeType( ATT_MIN_SIZE ); aHSz.SetHeight( 0 ); \
    } while (0)

void SwRTFParser::ReadPageDescTbl()
{
    // dann erzeuge aus der SvxStyle-Tabelle die Swg-Collections, damit
    // diese auch in den Headers/Footer benutzt werden koennen!
    MakeStyleTab();
    // das default-Style schon gleich am ersten Node setzen
    SwTxtFmtColl* pColl = NULL;
    std::map<sal_Int32,SwTxtFmtColl*>::iterator iter = aTxtCollTbl.find( 0 );

    if( iter == aTxtCollTbl.end() )
        pColl = pDoc->GetTxtCollFromPool( RES_POOLCOLL_STANDARD, false );
    else
        pColl = iter->second;

    pDoc->SetTxtFmtColl( *pPam, pColl );

    int nToken, bSaveChkStyleAttr = IsChkStyleAttr();
    int nNumOpenBrakets = 1;        // die erste wurde schon vorher erkannt !!

    SetChkStyleAttr(sal_False);     // Attribute nicht gegen die Styles checken

    bInPgDscTbl = true;
    sal_uInt16 nPos = 0;
    SwPageDesc* pPg = 0;
    SwFrmFmt* pPgFmt = 0;

    SvxULSpaceItem aUL( RES_UL_SPACE ), aHUL( RES_UL_SPACE ), aFUL( RES_UL_SPACE );
    SvxLRSpaceItem aLR( RES_LR_SPACE ), aHLR( RES_LR_SPACE ), aFLR( RES_LR_SPACE );
    Size a4 = SvxPaperInfo::GetPaperSize(PAPER_A4);
    SwFmtFrmSize aSz( ATT_FIX_SIZE, a4.Width(), a4.Height() );     // DIN A4 defaulten
    SwFmtFrmSize aFSz( ATT_MIN_SIZE ), aHSz( ATT_MIN_SIZE );

    SvxFrameDirectionItem aFrmDir(FRMDIR_HORI_LEFT_TOP, RES_FRAMEDIR);

    sal_uInt16 nCols = USHRT_MAX, nColSpace = USHRT_MAX, nAktCol = 0;
    std::vector<sal_uInt16> aColumns;
    ::std::map< const SwPageDesc*, sal_uInt16 > aFollowMap; //store index of following page descriptors

    while( nNumOpenBrakets && IsParserWorking() )
    {
        switch( nToken = GetNextToken() )
        {
        case '{':
            ++nNumOpenBrakets;
            break;
        case '}':
            if (1 == --nNumOpenBrakets)
            {
                OSL_ENSURE(pPgFmt && pPg, "Serious problem here");
                if (pPgFmt && pPg)
                {
                    // PageDesc ist fertig, setze am Doc
                    pPgFmt->SetFmtAttr(aFrmDir);
                    pPgFmt->SetFmtAttr(aLR);
                    pPgFmt->SetFmtAttr(aUL);
                    pPgFmt->SetFmtAttr(aSz);
                    ::lcl_SetFmtCol(*pPgFmt, nCols, nColSpace, aColumns);
                    if (pPgFmt->GetHeader().GetHeaderFmt())
                    {
                        SwFrmFmt* pHFmt =
                            (SwFrmFmt*)pPgFmt->GetHeader().GetHeaderFmt();
                        pHFmt->SetFmtAttr(aHUL);
                        pHFmt->SetFmtAttr(aHLR);
                        pHFmt->SetFmtAttr(aHSz);
                    }
                    if (pPgFmt->GetFooter().GetFooterFmt())
                    {
                        SwFrmFmt* pFFmt =
                            (SwFrmFmt*)pPgFmt->GetFooter().GetFooterFmt();
                        pFFmt->SetFmtAttr(aHUL);
                        pFFmt->SetFmtAttr(aHLR);
                        pFFmt->SetFmtAttr(aHSz);
                    }
                    if( nPos < pDoc->GetPageDescCnt() )
                        pDoc->ChgPageDesc(nPos++, *pPg);
                }
            }
            break;
        case RTF_PGDSC:
            if (nPos)   // kein && wg MAC
            {
                if (nPos != pDoc->MakePageDesc(
                    String::CreateFromInt32(nTokenValue)))
                {
                    OSL_FAIL( "PageDesc an falscher Position" );
                }
            }
            pPg = &pDoc->GetPageDesc(nPos);
            pPg->SetLandscape( sal_False );
            pPgFmt = &pPg->GetMaster();

            SETPAGEDESC_DEFAULTS();
            break;

        case RTF_PGDSCUSE:
            pPg->WriteUseOn( (UseOnPage)nTokenValue );
            break;

        case RTF_PGDSCNXT:
            // store index of follow in map; will be fixed up later
            if( nTokenValue )
                aFollowMap.insert( ::std::pair<const SwPageDesc*, sal_uInt16>( pPg, nTokenValue ));
            else
                pPg->SetFollow( &pDoc->GetPageDesc( 0 ) );
            break;

        case RTF_FORMULA:   /* Zeichen "\|" !!! */
            pPgFmt->SetFmtAttr( aLR );
            pPgFmt->SetFmtAttr( aUL );
            pPgFmt->SetFmtAttr( aSz );
            ::lcl_SetFmtCol( *pPgFmt, nCols, nColSpace, aColumns );
            if( pPgFmt->GetHeader().GetHeaderFmt() )
            {
                SwFrmFmt* pHFmt = (SwFrmFmt*)pPgFmt->GetHeader().GetHeaderFmt();
                pHFmt->SetFmtAttr( aHUL );
                pHFmt->SetFmtAttr( aHLR );
                pHFmt->SetFmtAttr( aHSz );
            }
            if( pPgFmt->GetFooter().GetFooterFmt() )
            {
                SwFrmFmt* pFFmt = (SwFrmFmt*)pPgFmt->GetFooter().GetFooterFmt();
                pFFmt->SetFmtAttr( aHUL );
                pFFmt->SetFmtAttr( aHLR );
                pFFmt->SetFmtAttr( aHSz );
            }

            pPgFmt = &pPg->GetLeft();

            SETPAGEDESC_DEFAULTS();
            break;

        case RTF_RTLSECT:
            aFrmDir.SetValue(FRMDIR_HORI_RIGHT_TOP);
            break;

        case RTF_LTRSECT:
            aFrmDir.SetValue(FRMDIR_HORI_LEFT_TOP);
            break;

        // alt: LI/RI/SA/SB, neu: MARG?SXN
        case RTF_MARGLSXN:
        case RTF_LI:        aLR.SetLeft( (sal_uInt16)nTokenValue );     break;
        case RTF_MARGRSXN:
        case RTF_RI:        aLR.SetRight( (sal_uInt16)nTokenValue );    break;
        case RTF_MARGTSXN:
        case RTF_SA:        aUL.SetUpper( (sal_uInt16)nTokenValue );    break;
        case RTF_MARGBSXN:
        case RTF_SB:        aUL.SetLower( (sal_uInt16)nTokenValue );    break;
        case RTF_PGWSXN:    aSz.SetWidth( nTokenValue );            break;
        case RTF_PGHSXN:    aSz.SetHeight( nTokenValue );           break;

        case RTF_HEADERY:       aHUL.SetUpper( (sal_uInt16)nTokenValue );   break;
        case RTF_HEADER_YB:     aHUL.SetLower( (sal_uInt16)nTokenValue );   break;
        case RTF_HEADER_XL:     aHLR.SetLeft( (sal_uInt16)nTokenValue );    break;
        case RTF_HEADER_XR:     aHLR.SetRight( (sal_uInt16)nTokenValue );   break;
        case RTF_FOOTERY:       aFUL.SetLower( (sal_uInt16)nTokenValue );   break;
        case RTF_FOOTER_YT:     aFUL.SetUpper( (sal_uInt16)nTokenValue );   break;
        case RTF_FOOTER_XL:     aFLR.SetLeft( (sal_uInt16)nTokenValue );    break;
        case RTF_FOOTER_XR:     aFLR.SetRight( (sal_uInt16)nTokenValue );   break;

        case RTF_HEADER_YH:
                if( 0 > nTokenValue )
                {
                    aHSz.SetHeightSizeType( ATT_FIX_SIZE );
                    nTokenValue = -nTokenValue;
                }
                aHSz.SetHeight( (sal_uInt16)nTokenValue );
                break;

        case RTF_FOOTER_YH:
                if( 0 > nTokenValue )
                {
                    aFSz.SetHeightSizeType( ATT_FIX_SIZE );
                    nTokenValue = -nTokenValue;
                }
                aFSz.SetHeight( (sal_uInt16)nTokenValue );
                break;


        case RTF_LNDSCPSXN:     pPg->SetLandscape( sal_True );          break;

        case RTF_COLS:          nCols = (sal_uInt16)nTokenValue;        break;
        case RTF_COLSX:         nColSpace = (sal_uInt16)nTokenValue;    break;

        case RTF_COLNO:
            nAktCol = (sal_uInt16)nTokenValue;
            if( RTF_COLW == GetNextToken() )
            {
                sal_uInt16 nWidth = sal_uInt16( nTokenValue ), nSpace = 0;
                if( RTF_COLSR == GetNextToken() )
                    nSpace = sal_uInt16( nTokenValue );
                else
                    SkipToken( -1 );        // wieder zurueck

                if( --nAktCol == ( aColumns.size() / 2 ) )
                {
                    aColumns.push_back( nWidth );
                    aColumns.push_back( nSpace );
                }
            }
            break;

        case RTF_PAGEBB:
            {
                pPgFmt->SetFmtAttr( SvxFmtBreakItem( SVX_BREAK_PAGE_BEFORE, RES_BREAK ) );
            }
            break;

        case RTF_HEADER:
        case RTF_HEADERL:
        case RTF_HEADERR:
        case RTF_FOOTER:
        case RTF_FOOTERL:
        case RTF_FOOTERR:
        case RTF_FOOTERF:
        case RTF_HEADERF:
            ReadHeaderFooter(nToken, pPg);
            --nNumOpenBrakets;      // Klammer wird im ReadAttr ueberlesen!
            break;
        case RTF_TEXTTOKEN:
            if (!DelCharAtEnd(aToken, ';' ).Len())
                break;
            OSL_ENSURE(pPg, "Unexpected missing pPg");
            if (pPg)
            {
                pPg->SetName(aToken);

                // sollte es eine Vorlage aus dem Pool sein ??
                sal_uInt16 n = SwStyleNameMapper::GetPoolIdFromUIName(aToken,
                    nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC);
                if (USHRT_MAX != n)
                {
                    // dann setze bei der Neuen die entsp. PoolId
                    pPg->SetPoolFmtId(n);
                }
            }
            break;
        case RTF_BRDBOX:
            if (3 == nNumOpenBrakets)
            {
                ReadBorderAttr(SkipToken(-2),
                    (SfxItemSet&)pPgFmt->GetAttrSet());
                --nNumOpenBrakets;      // Klammer wird im ReadAttr ueberlesen!
            }
            break;
        case RTF_SHADOW:
            if( 3 == nNumOpenBrakets )
            {
                ReadAttr( SkipToken( -2 ), (SfxItemSet*)&pPgFmt->GetAttrSet() );
                --nNumOpenBrakets;      // Klammer wird im ReadAttr ueberlesen!
            }
            break;


        default:
            if( (nToken & ~0xff ) == RTF_SHADINGDEF )
                ReadBackgroundAttr( nToken, (SfxItemSet&)pPgFmt->GetAttrSet() );
            break;
        }
    }


    // setze jetzt noch bei allen die entsprechenden Follows !!
    // Die, die ueber die Tabelle eingelesen wurden und einen
    // Follow definiert haben, ist dieser als Tabposition im
    // Follow schon gesetzt.
    for( nPos = 0; nPos < pDoc->GetPageDescCnt(); ++nPos )
    {
        SwPageDesc* pPgDsc = &pDoc->GetPageDesc( nPos );
        std::map< const SwPageDesc*, sal_uInt16 >::const_iterator aIter =
            aFollowMap.find( pPgDsc );
        if (aIter != aFollowMap.end())
        {
            if ((*aIter).second < pDoc->GetPageDescCnt())
                pPgDsc->SetFollow(& pDoc->GetPageDesc((*aIter).second));
        }
    }

    SetChkStyleAttr( bSaveChkStyleAttr );

    bInPgDscTbl = false;
    nAktPageDesc = 0;
    nAktFirstPageDesc = 0;
    bSwPageDesc = true;
    SkipToken( -1 );
}

void SwRTFParser::ReadPrtData()
{
    while( IsParserWorking() )
    {
        int nToken = GetNextToken();
        if( (RTF_TEXTTOKEN != nToken) && ('}' == nToken) )
            break;
    }

    SkipToken( -1 );        // schliessende Klammer wieder zurueck!!
}

static const SwNodeIndex* SetHeader(SwFrmFmt* pHdFtFmt, sal_Bool bReuseOld)
{
    OSL_ENSURE(pHdFtFmt, "Impossible, no header");
    const SwFrmFmt* pExisting = bReuseOld ?
        pHdFtFmt->GetHeader().GetHeaderFmt() : 0;
    if (!pExisting)
    {
        //No existing header, create a new one
        pHdFtFmt->SetFmtAttr(SwFmtHeader(sal_True));
        pExisting = pHdFtFmt->GetHeader().GetHeaderFmt();
    }
    return pExisting->GetCntnt().GetCntntIdx();
}

static const SwNodeIndex* SetFooter(SwFrmFmt* pHdFtFmt, sal_Bool bReuseOld)
{
    OSL_ENSURE(pHdFtFmt, "Impossible, no footer");
    const SwFrmFmt* pExisting = bReuseOld ?
        pHdFtFmt->GetFooter().GetFooterFmt() : 0;
    if (!pExisting)
    {
        //No exist footer, create a new one
        pHdFtFmt->SetFmtAttr(SwFmtFooter(sal_True));
        pExisting = pHdFtFmt->GetFooter().GetFooterFmt();
    }
    return pExisting->GetCntnt().GetCntntIdx();
}


void SwRTFParser::ReadHeaderFooter( int nToken, SwPageDesc* pPageDesc )
{
    OSL_ENSURE( RTF_FOOTNOTE == nToken ||
            RTF_FLY_INPARA == nToken ||
            pPageDesc, "PageDesc is missing" );

    bool bContainsParaCache = bContainsPara;
    // backup all important data
    SwPosition aSavePos( *pPam->GetPoint() );
    SvxRTFItemStack aSaveStack(GetAttrStack());
    GetAttrStack().clear();

    // save the fly array - after read, all flys may be set into
    // the header/footer
    SwFlySaveArr aSaveArray(aFlyArr);
    aSaveArray.reserve(255);
    aFlyArr.clear();
    sal_Bool bSetFlyInDoc = sal_True;

    const SwNodeIndex* pSttIdx = 0;
    SwFrmFmt* pHdFtFmt = 0;
    SwTxtAttr* pTxtAttr = 0;
    int bDelFirstChar = sal_False;
    bool bOldIsFootnote = mbIsFootnote;
    sal_Bool bOldGrpStt = sal::static_int_cast< sal_Bool, int >(IsNewGroup());

    int nNumOpenBrakets = GetOpenBrakets() - 1;

    switch( nToken )
    {
    case RTF_FOOTNOTE:
        {
            bool bIsEndNote = RTF_FTNALT == GetNextToken();
            if (!bIsEndNote)
                SkipToken(-1);

            SwTxtNode* pTxtNd = pPam->GetNode()->GetTxtNode();
            SwFmtFtn aFtnNote(bIsEndNote);
            xub_StrLen nPos = pPam->GetPoint()->nContent.GetIndex();

            if (nPos && !bFootnoteAutoNum)
            {
                pPam->GetPoint()->nContent--;
                nPos--;
                aFtnNote.SetNumStr(rtl::OUString(pTxtNd->GetTxt().GetChar(nPos)));
                ((String&)pTxtNd->GetTxt()).SetChar( nPos, CH_TXTATR_BREAKWORD );
                bDelFirstChar = sal_True;
            }

            pTxtAttr = pTxtNd->InsertItem( aFtnNote, nPos, nPos,
                        bDelFirstChar ? nsSetAttrMode::SETATTR_NOTXTATRCHR : 0 );

            OSL_ENSURE( pTxtAttr, "konnte die Fussnote nicht einfuegen/finden" );

            if( pTxtAttr )
                pSttIdx = ((SwTxtFtn*)pTxtAttr)->GetStartNode();
            mbIsFootnote = true;

            // wurde an der Position ein Escapement aufgespannt, so entferne
            // das jetzt. Fussnoten sind bei uns immer hochgestellt.
            SvxRTFItemStackType* pTmp = aSaveStack.empty() ? 0 : aSaveStack.back();
            if( pTmp && pTmp->GetSttNodeIdx() ==
                pPam->GetPoint()->nNode.GetIndex() &&
                pTmp->GetSttCnt() == nPos )
                pTmp->GetAttrSet().ClearItem( RES_CHRATR_ESCAPEMENT );
        }
        break;

    case RTF_FLY_INPARA:
        {
            xub_StrLen nPos = pPam->GetPoint()->nContent.GetIndex();
            SfxItemSet aSet( pDoc->GetAttrPool(), RES_FRMATR_BEGIN,
                                            RES_FRMATR_END-1 );
            aSet.Put( SwFmtAnchor( FLY_AS_CHAR ));
            pHdFtFmt = pDoc->MakeFlySection( FLY_AS_CHAR,
                            pPam->GetPoint(), &aSet );

            pTxtAttr = pPam->GetNode()->GetTxtNode()->GetTxtAttrForCharAt(
                                                nPos, RES_TXTATR_FLYCNT );
            OSL_ENSURE( pTxtAttr, "konnte den Fly nicht einfuegen/finden" );

            pSttIdx = pHdFtFmt->GetCntnt().GetCntntIdx();
            bSetFlyInDoc = sal_False;
        }
        break;

    case RTF_HEADERF:
    case RTF_HEADER:
        pPageDesc->WriteUseOn( (UseOnPage)(pPageDesc->ReadUseOn() | nsUseOnPage::PD_HEADERSHARE) );
        pHdFtFmt = &pPageDesc->GetMaster();
        pSttIdx = SetHeader( pHdFtFmt, sal_False );
        break;

    case RTF_HEADERL:
        // we cannot have left or right, must have always both
        pPageDesc->WriteUseOn( (UseOnPage)((pPageDesc->ReadUseOn() & ~nsUseOnPage::PD_HEADERSHARE) | nsUseOnPage::PD_ALL));
        SetHeader( pPageDesc->GetRightFmt(), sal_True );
        pHdFtFmt = pPageDesc->GetLeftFmt();
        pSttIdx = SetHeader(pHdFtFmt, sal_False );
        break;

    case RTF_HEADERR:
        // we cannot have left or right, must have always both
        pPageDesc->WriteUseOn( (UseOnPage)((pPageDesc->ReadUseOn() & ~nsUseOnPage::PD_HEADERSHARE) | nsUseOnPage::PD_ALL));
        SetHeader( pPageDesc->GetLeftFmt(), sal_True );
        pHdFtFmt = pPageDesc->GetRightFmt();
        pSttIdx = SetHeader(pHdFtFmt, sal_False );
        break;

    case RTF_FOOTERF:
    case RTF_FOOTER:
        pPageDesc->WriteUseOn( (UseOnPage)(pPageDesc->ReadUseOn() | nsUseOnPage::PD_FOOTERSHARE) );
        pHdFtFmt = &pPageDesc->GetMaster();
        pSttIdx = SetFooter(pHdFtFmt, sal_False );
        break;

    case RTF_FOOTERL:
        // we cannot have left or right, must have always both
        pPageDesc->WriteUseOn( (UseOnPage)((pPageDesc->ReadUseOn() & ~nsUseOnPage::PD_FOOTERSHARE) | nsUseOnPage::PD_ALL));
        SetFooter( pPageDesc->GetRightFmt(), sal_True );
        pHdFtFmt = pPageDesc->GetLeftFmt();
        pSttIdx = SetFooter(pHdFtFmt, sal_False );
        break;

    case RTF_FOOTERR:
        // we cannot have left or right, must have always both
        pPageDesc->WriteUseOn( (UseOnPage)((pPageDesc->ReadUseOn() & ~nsUseOnPage::PD_FOOTERSHARE) | nsUseOnPage::PD_ALL));
        SetFooter( pPageDesc->GetLeftFmt(), sal_True );
        pHdFtFmt = pPageDesc->GetRightFmt();
        pSttIdx = SetFooter(pHdFtFmt, sal_False );
        break;
    }

    sal_uInt16 nOldFlyArrCnt = aFlyArr.size();
    if( !pSttIdx )
        SkipGroup();
    else
    {
        // es ist auf jedenfall jetzt ein TextNode im Kopf/Fusszeilen-Bereich
        // vorhanden. Dieser muss jetzt nur noch gefunden und der neue Cursor
        // dort hinein gesetzt werden.
        SwCntntNode *pNode = pDoc->GetNodes()[ pSttIdx->GetIndex()+1 ]->
                                GetCntntNode();

        // immer ans Ende der Section einfuegen !!
        pPam->GetPoint()->nNode = *pNode->EndOfSectionNode();
        pPam->Move( fnMoveBackward );

        SwTxtFmtColl* pColl = NULL;
        std::map<sal_Int32,SwTxtFmtColl*>::iterator iter = aTxtCollTbl.find( 0 );

        if( iter == aTxtCollTbl.end() )
            pColl = pDoc->GetTxtCollFromPool( RES_POOLCOLL_STANDARD, false );
        else
            pColl = iter->second;

        pDoc->SetTxtFmtColl( *pPam, pColl );

        SetNewGroup( sal_True );

        while( !( nNumOpenBrakets == GetOpenBrakets() && !GetStackPos()) && IsParserWorking() )
        {
            switch( nToken = GetNextToken() )
            {
            case RTF_U:
                if( bDelFirstChar )
                {
                    bDelFirstChar = sal_False;
                    nToken = 0;
                }
                break;

            case RTF_TEXTTOKEN:
                if( bDelFirstChar )
                {
                    if( !aToken.Erase( 0, 1 ).Len() )
                        nToken = 0;
                    bDelFirstChar = sal_False;
                }
                break;
            }
            if( nToken )
                NextToken( nToken );
        }

        SetAllAttrOfStk();
        if( !aFlyArr.empty() && bSetFlyInDoc )
            SetFlysInDoc();

        // sollte der letze Node leer sein, dann loesche ihn
        // (\par heisst ja Absatzende und nicht neuer Absatz!)
        DelLastNode();
    }

    // vom FlyFmt noch die richtigen Attribute setzen
    if( pTxtAttr && RES_TXTATR_FLYCNT == pTxtAttr->Which() )
    {
        // is add a new fly ?
        if( nOldFlyArrCnt < aFlyArr.size() )
        {
            SwFlySave* pFlySave = aFlyArr.back();
            pFlySave->aFlySet.ClearItem( RES_ANCHOR );
            pHdFtFmt->SetFmtAttr( pFlySave->aFlySet );
            delete aFlyArr.back();
            aFlyArr.pop_back();
        }
        else
        {
            // no, so remove the created textattribute
            SwFrmFmt* pFlyFmt = pTxtAttr->GetFlyCnt().GetFrmFmt();
            // remove the pam from the flynode
            *pPam->GetPoint() = aSavePos;
            pDoc->DelLayoutFmt( pFlyFmt );
        }
    }

    bFootnoteAutoNum = sal_False;       // default auf aus!

    // und alles wieder zurueck
    *pPam->GetPoint() = aSavePos;
    if (mbIsFootnote)
        SetNewGroup( bOldGrpStt );      // Status wieder zurueck
    else
        SetNewGroup( sal_False );           // { - Klammer war kein Group-Start!
    mbIsFootnote = bOldIsFootnote;
    GetAttrStack() = aSaveStack;

    std::copy(aSaveArray.begin(), aSaveArray.end(), aFlyArr.begin() );
    aSaveArray.clear();
    bContainsPara = bContainsParaCache;
}

void SwRTFParser::SetSwgValues( SfxItemSet& rSet )
{
    const SfxPoolItem* pItem;
    // Escapement korrigieren
    if( SFX_ITEM_SET == rSet.GetItemState( RES_CHRATR_ESCAPEMENT, sal_False, &pItem ))
    {
        /* prozentuale Veraenderung errechnen !
            * Formel :      (FontSize * 1/20 ) pts      Escapement * 2
            *               -----------------------  = ----------------
            *                     100%                          x
            */

        // die richtige
        long nEsc = ((SvxEscapementItem*)pItem)->GetEsc();

        // automatische Ausrichtung wurde schon richtig berechnet
        if( DFLT_ESC_AUTO_SUPER != nEsc && DFLT_ESC_AUTO_SUB != nEsc )
        {
            const SvxFontHeightItem& rFH = GetSize( rSet );
            nEsc *= 1000L;
            if(rFH.GetHeight()) nEsc /= long(rFH.GetHeight()); // #i77256#

            SvxEscapementItem aEsc( (short) nEsc,
                                ((SvxEscapementItem*)pItem)->GetProp(), RES_CHRATR_ESCAPEMENT);
            rSet.Put( aEsc );
        }
    }

    // TabStops anpassen
    if( SFX_ITEM_SET == rSet.GetItemState( RES_PARATR_TABSTOP, sal_False, &pItem ))
    {
        const SvxLRSpaceItem& rLR = GetLRSpace( rSet );
        SvxTabStopItem aTStop( *(SvxTabStopItem*)pItem );

        long nOffset = rLR.GetTxtLeft();
        if( nOffset )
        {
            // Tabs anpassen !!
            for( sal_uInt16 n = 0; n < aTStop.Count(); ++n)
                if( SVX_TAB_ADJUST_DEFAULT != aTStop[n].GetAdjustment() )
                {
                    const_cast<SvxTabStop&>(aTStop[n]).GetTabPos() -= nOffset;
                }

            // negativer Einzug, dann auf 0 Pos einen Tab setzen
            if( rLR.GetTxtFirstLineOfst() < 0 )
                aTStop.Insert( SvxTabStop() );
        }

        if( !aTStop.Count() )
        {
            const SvxTabStopItem& rDflt = (const SvxTabStopItem&)rSet.
                                GetPool()->GetDefaultItem(RES_PARATR_TABSTOP);
            if( rDflt.Count() )
                aTStop.Insert( &rDflt, 0 );
        }
        rSet.Put( aTStop );
    }
    else if( SFX_ITEM_SET == rSet.GetItemState( RES_LR_SPACE, sal_False, &pItem )
            && ((SvxLRSpaceItem*)pItem)->GetTxtFirstLineOfst() < 0 )
    {
        // negativer Einzug, dann auf 0 Pos einen Tab setzen
        rSet.Put( SvxTabStopItem( 1, 0, SVX_TAB_ADJUST_DEFAULT, RES_PARATR_TABSTOP ));
    }

    // NumRules anpassen
    if( !bStyleTabValid &&
        SFX_ITEM_SET == rSet.GetItemState( RES_PARATR_NUMRULE, sal_False, &pItem ))
    {
        // dann steht im Namen nur ein Verweis in das ListArray
        SwNumRule* pRule = GetNumRuleOfListNo( ((SwNumRuleItem*)pItem)->
                                                GetValue().ToInt32() );
        if( pRule )
            rSet.Put( SwNumRuleItem( pRule->GetName() ));
        else
            rSet.ClearItem( RES_PARATR_NUMRULE );

    }

}


SwTxtFmtColl* SwRTFParser::MakeColl(const String& rName, sal_uInt16 nPos,
    sal_uInt8 nOutlineLevel, bool& rbCollExist)
{
    if( sal_uInt8(-1) == nOutlineLevel )
        nOutlineLevel = MAXLEVEL;

    rbCollExist = false;
    SwTxtFmtColl* pColl;
    String aNm( rName );
    if( !aNm.Len() )
    {
        OSL_ENSURE(!this, "not a bug, but I (cmc) want to see an example of this");
        if( !nPos )
        {
            pColl = pDoc->GetTxtCollFromPool( RES_POOLCOLL_STANDARD, false );
            if(nOutlineLevel < MAXLEVEL )
                pColl->AssignToListLevelOfOutlineStyle( nOutlineLevel );
            else
                pColl->DeleteAssignmentToListLevelOfOutlineStyle();
            return pColl;
        }

        // erzeuge einen Namen
        aNm.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "NoName(" ));
        aNm += String::CreateFromInt32( nPos );
        aNm += ')';
    }
    ww::sti eSti = ww::GetCanonicalStiFromEnglishName(rName);
    sw::util::ParaStyleMapper::StyleResult aResult =
        maParaStyleMapper.GetStyle(rName, eSti);
    pColl = aResult.first;
    rbCollExist = aResult.second;
    if (IsNewDoc() && rbCollExist)
    {
        pColl->ResetAllFmtAttr(); // #i73790# - method renamed
        rbCollExist = false;
    }

    if (!rbCollExist)
    {
        if(nOutlineLevel < MAXLEVEL)
            pColl->AssignToListLevelOfOutlineStyle( nOutlineLevel );
        else
            pColl->DeleteAssignmentToListLevelOfOutlineStyle();
    }

    return pColl;
}

SwCharFmt* SwRTFParser::MakeCharFmt(const String& rName, sal_uInt16 nPos,
                                    int& rbCollExist)
{
    rbCollExist = sal_False;
    SwCharFmt* pFmt;
    String aNm( rName );
    if( !aNm.Len() )
    {
        OSL_ENSURE(!this, "not a bug, but I (cmc) want to see an example of this");
        aNm.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "NoName(" ));
        aNm += String::CreateFromInt32( nPos );
        aNm += ')';
    }

    ww::sti eSti = ww::GetCanonicalStiFromEnglishName(rName);
    sw::util::CharStyleMapper::StyleResult aResult =
        maCharStyleMapper.GetStyle(rName, eSti);
    pFmt = aResult.first;
    rbCollExist = aResult.second;
    if (IsNewDoc() && rbCollExist)
    {
        pFmt->ResetAllFmtAttr(); // #i73790# - method renamed
        rbCollExist = false;
    }
    return pFmt;
}

void SwRTFParser::SetStyleAttr( SfxItemSet& rCollSet,
                                const SfxItemSet& rStyleSet,
                                const SfxItemSet& rDerivedSet )
{
    rCollSet.Put( rStyleSet );
    if( rDerivedSet.Count() )
    {
        // suche alle Attribute, die neu gesetzt werden:
        const SfxPoolItem* pItem;
        SfxItemIter aIter( rDerivedSet );
        sal_uInt16 nWhich = aIter.GetCurItem()->Which();
        while( sal_True )
        {
            switch( rStyleSet.GetItemState( nWhich, sal_False, &pItem ) )
            {
            case SFX_ITEM_DEFAULT:
                // auf default zuruecksetzen
                if( RES_FRMATR_END > nWhich )
                    rCollSet.Put( rCollSet.GetPool()->GetDefaultItem( nWhich ));
                break;
            case SFX_ITEM_SET:
                if( *pItem == *aIter.GetCurItem() )     // gleiches Attribut?
                    // definition kommt aus dem Parent
                    rCollSet.ClearItem( nWhich );       // loeschen
                break;
            }

            if( aIter.IsAtEnd() )
                break;
            nWhich = aIter.NextItem()->Which();
        }
    }
    // und jetzt noch auf unsere Werte abgleichen
    SetSwgValues( rCollSet );
}

SwTxtFmtColl* SwRTFParser::MakeStyle( sal_uInt16 nNo, const SvxRTFStyleType& rStyle)
{
    bool bCollExist;
    SwTxtFmtColl* pColl = MakeColl( rStyle.sName, sal_uInt16(nNo),
        rStyle.nOutlineNo, bCollExist);
    aTxtCollTbl.insert(std::make_pair(nNo,pColl));

    // in bestehendes Dok einfuegen, dann keine Ableitung usw. setzen
    if( bCollExist )
        return pColl;

    sal_uInt16 nStyleNo = rStyle.nBasedOn;
    if( rStyle.bBasedOnIsSet && nStyleNo != nNo )
    {
        SvxRTFStyleTbl::iterator styleIter = GetStyleTbl().find( nStyleNo );
        SvxRTFStyleType* pDerivedStyle = NULL;
        if ( styleIter != GetStyleTbl().end() )
            pDerivedStyle = styleIter->second;

        SwTxtFmtColl* pDerivedColl = NULL;
        std::map<sal_Int32,SwTxtFmtColl*>::iterator iter = aTxtCollTbl.find(nStyleNo);

        if(iter == aTxtCollTbl.end() )         // noch nicht vorhanden, also anlegen
        {
            // ist die ueberhaupt als Style vorhanden ?
            pDerivedColl = pDerivedStyle
                    ? MakeStyle( nStyleNo, *pDerivedStyle )
                    : pDoc->GetTxtCollFromPool( RES_POOLCOLL_STANDARD, false );
        }
        else
            pDerivedColl = iter->second;

        if( pColl == pDerivedColl )
            ((SfxItemSet&)pColl->GetAttrSet()).Put( rStyle.aAttrSet );
        else
        {
            pColl->SetDerivedFrom( pDerivedColl );

            // setze die richtigen Attribute
            const SfxItemSet* pDerivedSet;
            if( pDerivedStyle )
                pDerivedSet = &pDerivedStyle->aAttrSet;
            else
                pDerivedSet = &pDerivedColl->GetAttrSet();

            SetStyleAttr( (SfxItemSet&)pColl->GetAttrSet(),
                            rStyle.aAttrSet, *pDerivedSet );
        }
    }
    else
        ((SfxItemSet&)pColl->GetAttrSet()).Put( rStyle.aAttrSet );


    nStyleNo = rStyle.nNext;
    if( nStyleNo != nNo )
    {
        SwTxtFmtColl* pNext = NULL;
        std::map<sal_Int32,SwTxtFmtColl*>::iterator iter = aTxtCollTbl.find( nStyleNo );

        if( iter == aTxtCollTbl.end())            // noch nicht vorhanden, also anlegen
        {
            // ist die ueberhaupt als Style vorhanden ?
            SvxRTFStyleTbl::iterator styleIter = GetStyleTbl().find( nStyleNo );
            if ( styleIter != GetStyleTbl().end() )
                pNext = MakeStyle( nStyleNo, *styleIter->second );
            else
                pNext = pDoc->GetTxtCollFromPool( RES_POOLCOLL_STANDARD, false );
        }
        else
            pNext = iter->second;

        pColl->SetNextTxtFmtColl( *pNext );
    }
    return pColl;
}

SwCharFmt* SwRTFParser::MakeCharStyle( sal_uInt16 nNo, const SvxRTFStyleType& rStyle )
{
    int bCollExist;
    SwCharFmt* pFmt = MakeCharFmt( rStyle.sName, sal_uInt16(nNo), bCollExist );
    aCharFmtTbl.insert(std::make_pair(nNo,pFmt));

    // in bestehendes Dok einfuegen, dann keine Ableitung usw. setzen
    if( bCollExist )
        return pFmt;

    sal_uInt16 nStyleNo = rStyle.nBasedOn;
    if( rStyle.bBasedOnIsSet && nStyleNo != nNo )
    {
        SvxRTFStyleTbl::iterator styleIter = GetStyleTbl().find( nStyleNo );
        SvxRTFStyleType* pDerivedStyle = NULL;
        if ( styleIter != GetStyleTbl().end() )
            pDerivedStyle = styleIter->second;

        SwCharFmt* pDerivedFmt = NULL;
        std::map<sal_Int32,SwCharFmt*>::iterator iter = aCharFmtTbl.find( nStyleNo );

        if(iter == aCharFmtTbl.end())          // noch nicht vorhanden, also anlegen
        {
            // ist die ueberhaupt als Style vorhanden ?
            pDerivedFmt = pDerivedStyle
                    ? MakeCharStyle( nStyleNo, *pDerivedStyle )
                    : pDoc->GetDfltCharFmt();
        }
        else
            pDerivedFmt = iter->second;

        if( pFmt == pDerivedFmt )
            ((SfxItemSet&)pFmt->GetAttrSet()).Put( rStyle.aAttrSet );
        else
        {
            pFmt->SetDerivedFrom( pDerivedFmt );

            // setze die richtigen Attribute
            const SfxItemSet* pDerivedSet;
            if( pDerivedStyle )
                pDerivedSet = &pDerivedStyle->aAttrSet;
            else
                pDerivedSet = &pDerivedFmt->GetAttrSet();

            SetStyleAttr( (SfxItemSet&)pFmt->GetAttrSet(),
                            rStyle.aAttrSet, *pDerivedSet );
        }
    }
    else
        ((SfxItemSet&)pFmt->GetAttrSet()).Put( rStyle.aAttrSet );

    return pFmt;
}

// loesche den letzten Node (Tabelle/Fly/Ftn/..)
void SwRTFParser::DelLastNode()
{
    // sollte der letze Node leer sein, dann loesche ihn
    // (\par heisst ja Absatzende und nicht neuer Absatz!)

    if( !pPam->GetPoint()->nContent.GetIndex() )
    {
        sal_uLong nNodeIdx = pPam->GetPoint()->nNode.GetIndex();
        SwCntntNode* pCNd = pDoc->GetNodes()[ nNodeIdx ]->GetCntntNode();
        // paragraphs with page break information are not empty!)
        if(const SfxPoolItem* pItem=&(pCNd->GetAttr( RES_PAGEDESC, sal_False)))
        {
            SwFmtPageDesc* pPageDescItem = ((SwFmtPageDesc*)pItem);
            if (pPageDescItem->GetPageDesc()!=NULL)
            return;
        }

        if( pCNd && pCNd->StartOfSectionIndex()+2 <
            pCNd->EndOfSectionIndex() )
        {
            if( !GetAttrStack().empty() )
            {
                // Attribut Stack-Eintraege, muessen ans Ende des vorherigen
                // Nodes verschoben werden.
                sal_Bool bMove = sal_False;
                for( size_t n = GetAttrStack().size(); n; )
                {
                    SvxRTFItemStackType* pStkEntry = (SvxRTFItemStackType*)
                                                    GetAttrStack()[ --n ];
                    if( nNodeIdx == pStkEntry->GetSttNode().GetIdx() )
                    {
                        if( !bMove )
                        {
                            pPam->Move( fnMoveBackward );
                            bMove = sal_True;
                        }
                        pStkEntry->SetStartPos( SwxPosition( pPam ) );
                    }
                }
                if( bMove )
                    pPam->Move( fnMoveForward );
            }
            pPam->GetPoint()->nContent.Assign( 0, 0 );
            pPam->SetMark();
            pPam->DeleteMark();

            pDoc->GetNodes().Delete( pPam->GetPoint()->nNode );
        }
    }
}

    // fuer Tokens, die im ReadAttr nicht ausgewertet werden
void SwRTFParser::UnknownAttrToken( int nToken, SfxItemSet* pSet )
{
    switch( nToken )
    {
    case RTF_INTBL:
        {
            if( !pTableNode )           // Tabelle nicht mehr vorhanden ?
                NewTblLine();           // evt. Line copieren
            else
            {
                static int _do=0;
                // Crsr nicht mehr in der Tabelle ?
                if( !pPam->GetNode()->FindTableNode() && _do )
                {
                    sal_uLong nOldPos = pPam->GetPoint()->nNode.GetIndex();

                    // dann wieder in die letzte Box setzen
                    // (kann durch einlesen von Flys geschehen!)
                    pPam->GetPoint()->nNode = *pTableNode->EndOfSectionNode();
                    pPam->Move( fnMoveBackward );

                    // alle Attribute, die schon auf den nachfolgen zeigen
                    // auf die neue Box umsetzen !!
                    SvxRTFItemStack& rAttrStk = GetAttrStack();
                    const SvxRTFItemStackType* pStk;
                    for( size_t n = 0; n < rAttrStk.size(); ++n )
                        if( ( pStk = rAttrStk[ n ])->GetSttNodeIdx() == nOldPos &&
                            !pStk->GetSttCnt() )
                            ((SvxRTFItemStackType*)pStk)->SetStartPos( SwxPosition( pPam ) );
                }
            }
        }
        break;

    case RTF_PAGEBB:
        {
            pSet->Put( SvxFmtBreakItem( SVX_BREAK_PAGE_BEFORE, RES_BREAK ));
        }
        break;

    case RTF_PGBRK:
        {
            pSet->Put( SvxFmtBreakItem( 1 == nTokenValue ?
                                SVX_BREAK_PAGE_BOTH : SVX_BREAK_PAGE_AFTER, RES_BREAK ));
        }
        break;

    case RTF_PGDSCNO:
        if( IsNewDoc() && bSwPageDesc &&
            sal_uInt16(nTokenValue) < pDoc->GetPageDescCnt() )
        {
            const SwPageDesc* pPgDsc = &pDoc->GetPageDesc( (sal_uInt16)nTokenValue );
            pDoc->InsertPoolItem( *pPam, SwFmtPageDesc( pPgDsc ), 0);
        }
        break;
    case RTF_CS:
        {
            std::map<sal_Int32,SwCharFmt*>::iterator iter = aCharFmtTbl.find( nTokenValue );

            if(iter != aCharFmtTbl.end())
                pSet->Put( SwFmtCharFmt(iter->second));
        }
        break;

    case RTF_LS:
        if( -1 != nTokenValue )
        {
            if( bStyleTabValid )
            {
                // dann ist auch die ListTabelle gueltig, also suche die
                // enstprechende NumRule
                SwNumRule* pRule = GetNumRuleOfListNo( nTokenValue );
                if( pRule )
                    pSet->Put( SwNumRuleItem( pRule->GetName() ));

                if( SFX_ITEM_SET != pSet->GetItemState( FN_PARAM_NUM_LEVEL, sal_False ))
                    pSet->Put( SfxUInt16Item( FN_PARAM_NUM_LEVEL, 0 ));
            }
            else
            {
                // wir sind in der Style-Definitions - Phase. Der Name
                // wird dann spaeter umgesetzt
                                // pSet->Put( SwNumRuleItem( String::CreateFromInt32( nTokenValue )));
            }

        }
        break;

    case RTF_ILVL:
    case RTF_SOUTLVL:
        {
            sal_uInt8 nLevel = MAXLEVEL <= nTokenValue ? MAXLEVEL - 1
                                                  : sal_uInt8( nTokenValue );
            pSet->Put( SfxUInt16Item( FN_PARAM_NUM_LEVEL, nLevel ));
        }
        break;

    }
}

void SwRTFParser::ReadInfo( const sal_Char* pChkForVerNo )
{
sal_Char const aChkForVerNo[] = "StarWriter";

    // falls nicht schon was vorgegeben wurde, setzen wir unseren Namen
    // rein. Wenn das im Kommentar match, wird im Parser die VersionNummer
    // gelesen und gesetzt
    if( !pChkForVerNo )
        pChkForVerNo = aChkForVerNo;

    SvxRTFParser::ReadInfo( pChkForVerNo );
}

void SwRTFParser::ReadUserProperties()
{
    // For now we don't support user properties but at least the parser is here.
    // At the moment it just swallows the tokens to prevent them being displayed
    int nNumOpenBrakets = 1, nToken;

    while( nNumOpenBrakets && IsParserWorking() )
    {
        switch( nToken = GetNextToken() )
        {
        case '}':
             --nNumOpenBrakets;
             break;
        case '{':
            {
                if( RTF_IGNOREFLAG != GetNextToken() )
                    nToken = SkipToken( -1 );
                else if( RTF_UNKNOWNCONTROL != GetNextToken() )
                    nToken = SkipToken( -2 );
                else
                {
                    // gleich herausfiltern
                    ReadUnknownData();
                    nToken = GetNextToken();
                    if( '}' != nToken )
                        eState = SVPAR_ERROR;
                    break;
                }
                ++nNumOpenBrakets;
            }
            break;

        case RTF_PROPNAME:
            SkipGroup();
            break;

        case RTF_PROPTYPE:
            break;

        case RTF_STATICVAL:
            SkipGroup();
             break;

//      default:
        }
    }

    SkipToken( -1 );
}


#ifdef USED
void SwRTFParser::SaveState( int nToken )
{
    SvxRTFParser::SaveState( nToken );
}

void SwRTFParser::RestoreState()
{
    SvxRTFParser::RestoreState();
}
#endif

/**/

BookmarkPosition::BookmarkPosition(const SwPaM &rPaM)
    : maMkNode(rPaM.GetMark()->nNode),
    mnMkCntnt(rPaM.GetMark()->nContent.GetIndex())
{
}

BookmarkPosition::BookmarkPosition(const BookmarkPosition &rEntry)
    : maMkNode(rEntry.maMkNode), mnMkCntnt(rEntry.mnMkCntnt)
{
}

bool BookmarkPosition::operator==(const BookmarkPosition rhs)
{
    return(maMkNode.GetIndex() == rhs.maMkNode.GetIndex() && mnMkCntnt == rhs.mnMkCntnt);
}

sal_uLong SwNodeIdx::GetIdx() const
{
    return aIdx.GetIndex();
}

SvxNodeIdx* SwNodeIdx::Clone() const
{
    return new SwNodeIdx( aIdx );
}

SvxPosition* SwxPosition::Clone() const
{
    return new SwxPosition( pPam );
}

SvxNodeIdx* SwxPosition::MakeNodeIdx() const
{
    return new SwNodeIdx( pPam->GetPoint()->nNode );
}

sal_uLong   SwxPosition::GetNodeIdx() const
{
    return pPam->GetPoint()->nNode.GetIndex();
}

xub_StrLen SwxPosition::GetCntIdx() const
{
    return pPam->GetPoint()->nContent.GetIndex();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
