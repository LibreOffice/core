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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <hintids.hxx>
#include <rtl/math.hxx>
#include <unotools/collatorwrapper.hxx>
#include <unotools/localedatawrapper.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/i18n/CollatorOptions.hpp>
#include <comphelper/processfactory.hxx>
#include <editeng/unolingu.hxx>
#include <docary.hxx>
#include <fmtanchr.hxx>
#include <frmfmt.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <node.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <swtable.hxx>
#include <swundo.hxx>
#include <sortopt.hxx>
#include <docsort.hxx>
#include <UndoSort.hxx>
#include <UndoRedline.hxx>
#include <hints.hxx>
#include <tblsel.hxx>
#include <cellatr.hxx>
#include <redline.hxx>
#include <node2lay.hxx>
#include <unochart.hxx>

using namespace ::com::sun::star::lang;

SwSortOptions*      SwSortElement::pOptions = 0;
SwDoc*              SwSortElement::pDoc = 0;
const FlatFndBox*   SwSortElement::pBox = 0;
CollatorWrapper*    SwSortElement::pSortCollator = 0;
Locale*             SwSortElement::pLocale = 0;
String*             SwSortElement::pLastAlgorithm = 0;
LocaleDataWrapper*  SwSortElement::pLclData = 0;

SV_IMPL_OP_PTRARR_SORT( SwSortElements, SwSortElementPtr );

/*--------------------------------------------------------------------
    Beschreibung: Ein Sortierelement fuers Sort konstruieren
 --------------------------------------------------------------------*/
void SwSortElement::Init( SwDoc* pD, const SwSortOptions& rOpt,
                            FlatFndBox* pFltBx )
{
    OSL_ENSURE( !pDoc && !pOptions && !pBox, "wer hat das Finit vergessen?" );
    pDoc = pD;
    pOptions = new SwSortOptions( rOpt );
    pBox = pFltBx;

    LanguageType nLang = rOpt.nLanguage;
    switch ( nLang )
    {
    case LANGUAGE_NONE:
    case LANGUAGE_DONTKNOW:
        nLang = (LanguageType)GetAppLanguage();
        break;
    }
    pLocale = new Locale( SvxCreateLocale( nLang ) );

    pSortCollator = new CollatorWrapper(
                                ::comphelper::getProcessServiceFactory() );
}

void SwSortElement::Finit()
{
    delete pOptions, pOptions = 0;
    delete pLocale, pLocale = 0;
    delete pLastAlgorithm, pLastAlgorithm = 0;
    delete pSortCollator, pSortCollator = 0;
    delete pLclData, pLclData = 0;
    pDoc = 0;
    pBox = 0;
}

SwSortElement::~SwSortElement()
{
}

double SwSortElement::StrToDouble( const String& rStr ) const
{
    if( !pLclData )
        pLclData = new LocaleDataWrapper(
                    ::comphelper::getProcessServiceFactory(), *pLocale );

    rtl_math_ConversionStatus eStatus;
    sal_Int32 nEnd;
    double nRet = ::rtl::math::stringToDouble( rStr,
                                    pLclData->getNumDecimalSep().GetChar(0),
                                    pLclData->getNumThousandSep().GetChar(0),
                                    &eStatus, &nEnd );

    if( rtl_math_ConversionStatus_Ok != eStatus || nEnd == 0 )
        nRet = 0.0;
    return nRet;
}

/*--------------------------------------------------------------------
    Beschreibung: Operatoren zum Vergleichen
 --------------------------------------------------------------------*/
sal_Bool SwSortElement::operator==(const SwSortElement& )
{
    return sal_False;
}

/*--------------------------------------------------------------------
    Beschreibung: Kleiner-Operator fuers sortieren
 --------------------------------------------------------------------*/
sal_Bool SwSortElement::operator<(const SwSortElement& rCmp)
{

    // der eigentliche Vergleich
    //
    for(sal_uInt16 nKey = 0; nKey < pOptions->aKeys.Count(); ++nKey)
    {
        const SwSortElement *pOrig, *pCmp;

        const SwSortKey* pSrtKey = pOptions->aKeys[ nKey ];
        if( pSrtKey->eSortOrder == SRT_ASCENDING )
            pOrig = this, pCmp = &rCmp;
        else
            pOrig = &rCmp, pCmp = this;

        if( pSrtKey->bIsNumeric )
        {
            double n1 = pOrig->GetValue( nKey );
            double n2 = pCmp->GetValue( nKey );

            if( n1 == n2 )
                continue;

            return n1 < n2;
        }
        else
        {
            if( !pLastAlgorithm || *pLastAlgorithm != pSrtKey->sSortType )
            {
                if( pLastAlgorithm )
                    *pLastAlgorithm = pSrtKey->sSortType;
                else
                    pLastAlgorithm = new String( pSrtKey->sSortType );
                pSortCollator->loadCollatorAlgorithm( *pLastAlgorithm,
                        *pLocale,
                        pOptions->bIgnoreCase ? SW_COLLATOR_IGNORES : 0 );
            }

            sal_Int32 nCmp = pSortCollator->compareString(
                        pOrig->GetKey( nKey ), pCmp->GetKey( nKey ));
            if( 0 == nCmp )
                continue;

            return -1 == nCmp;
        }
    }
    return sal_False;
}

double SwSortElement::GetValue( sal_uInt16 nKey ) const
{
    return StrToDouble( GetKey( nKey ));
}

/*--------------------------------------------------------------------
    Beschreibung: SortierElemente fuer Text
 --------------------------------------------------------------------*/
SwSortTxtElement::SwSortTxtElement(const SwNodeIndex& rPos)
    : nOrg(rPos.GetIndex()), aPos(rPos)
{
}

SwSortTxtElement::~SwSortTxtElement()
{
}

/*--------------------------------------------------------------------
    Beschreibung: Key ermitteln
 --------------------------------------------------------------------*/
String SwSortTxtElement::GetKey(sal_uInt16 nId) const
{
    SwTxtNode* pTxtNd = aPos.GetNode().GetTxtNode();
    if( !pTxtNd )
        return aEmptyStr;

    // fuer TextNodes
    const String& rStr = pTxtNd->GetTxt();

    sal_Unicode nDeli = pOptions->cDeli;
    sal_uInt16 nDCount = pOptions->aKeys[nId]->nColumnId, i = 1;
    xub_StrLen nStart = 0;

    // Den Delimitter suchen
    while( nStart != STRING_NOTFOUND && i < nDCount)
        if( STRING_NOTFOUND != ( nStart = rStr.Search( nDeli, nStart ) ) )
        {
            nStart++;
            i++;
        }

    // naechsten Delimitter gefunden oder Ende des Strings und Kopieren
    xub_StrLen nEnd = rStr.Search( nDeli, nStart+1 );
    return rStr.Copy( nStart, nEnd-nStart );
}

/*--------------------------------------------------------------------
    Beschreibung: Sortier-Elemente fuer Tabellen
 --------------------------------------------------------------------*/
SwSortBoxElement::SwSortBoxElement( sal_uInt16 nRC )
    : nRow( nRC )
{
}

SwSortBoxElement::~SwSortBoxElement()
{
}

/*--------------------------------------------------------------------
    Beschreibung: Schluessel zu einer Zelle ermitteln
 --------------------------------------------------------------------*/
String SwSortBoxElement::GetKey(sal_uInt16 nKey) const
{
    const _FndBox* pFndBox;
    sal_uInt16 nCol = pOptions->aKeys[nKey]->nColumnId-1;

    if( SRT_ROWS == pOptions->eDirection )
        pFndBox = pBox->GetBox(nCol, nRow);         // Zeilen sortieren
    else
        pFndBox = pBox->GetBox(nRow, nCol);         // Spalten sortieren

    // Den Text rausfieseln
    String aRetStr;
    if( pFndBox )
    {   // StartNode holen und ueberlesen
        const SwTableBox* pMyBox = pFndBox->GetBox();
        OSL_ENSURE(pMyBox, "Keine atomare Box");

        if( pMyBox->GetSttNd() )
        {
            // ueber alle TextNodes der Box
            const SwNode *pNd = 0, *pEndNd = pMyBox->GetSttNd()->EndOfSectionNode();
            for( sal_uLong nIdx = pMyBox->GetSttIdx() + 1; pNd != pEndNd; ++nIdx )
                if( ( pNd = pDoc->GetNodes()[ nIdx ])->IsTxtNode() )
                    aRetStr += ((SwTxtNode*)pNd)->GetTxt();
        }
    }
    return aRetStr;
}

double SwSortBoxElement::GetValue( sal_uInt16 nKey ) const
{
    const _FndBox* pFndBox;
    sal_uInt16 nCol = pOptions->aKeys[nKey]->nColumnId-1;

    if( SRT_ROWS == pOptions->eDirection )
        pFndBox = pBox->GetBox(nCol, nRow);         // Zeilen sortieren
    else
        pFndBox = pBox->GetBox(nRow, nCol);         // Spalten sortieren

    double nVal;
    if( pFndBox )
    {
        const SwFmt *pFmt = pFndBox->GetBox()->GetFrmFmt();
        if (pFmt->GetTblBoxNumFmt().GetValue() & NUMBERFORMAT_TEXT)
            nVal = SwSortElement::GetValue( nKey );
        else
            nVal = pFmt->GetTblBoxValue().GetValue();
    }
    else
        nVal = 0;

    return nVal;
}

/*--------------------------------------------------------------------
    Beschreibung: Text sortieren im Document
 --------------------------------------------------------------------*/
sal_Bool SwDoc::SortText(const SwPaM& rPaM, const SwSortOptions& rOpt)
{
    // pruefen ob Rahmen im Text
    const SwPosition *pStart = rPaM.Start(), *pEnd = rPaM.End();
    // Index auf den Start der Selektion

    for ( sal_uInt16 n = 0; n < GetSpzFrmFmts()->Count(); ++n )
    {
        SwFrmFmt *const pFmt = static_cast<SwFrmFmt*>((*GetSpzFrmFmts())[n]);
        SwFmtAnchor const*const pAnchor = &pFmt->GetAnchor();
        SwPosition const*const pAPos = pAnchor->GetCntntAnchor();

        if (pAPos && (FLY_AT_PARA == pAnchor->GetAnchorId()) &&
            pStart->nNode <= pAPos->nNode && pAPos->nNode <= pEnd->nNode )
            return sal_False;
    }

    // pruefe ob nur TextNodes in der Selection liegen
    {
        sal_uLong nStart = pStart->nNode.GetIndex(),
                        nEnd = pEnd->nNode.GetIndex();
        while( nStart <= nEnd )
            // Iterieren ueber einen selektierten Bereich
            if( !GetNodes()[ nStart++ ]->IsTxtNode() )
                return sal_False;
    }

    bool const bUndo = GetIDocumentUndoRedo().DoesUndo();
    if( bUndo )
    {
        GetIDocumentUndoRedo().StartUndo( UNDO_START, NULL );
    }

    SwPaM* pRedlPam = 0;
    SwUndoRedlineSort* pRedlUndo = 0;
    SwUndoSort* pUndoSort = 0;

    if( IsRedlineOn() || (!IsIgnoreRedline() && pRedlineTbl->Count() ))
    {
        pRedlPam = new SwPaM( pStart->nNode, pEnd->nNode, -1, 1 );
        SwCntntNode* pCNd = pRedlPam->GetCntntNode( sal_False );
        if( pCNd )
            pRedlPam->GetMark()->nContent = pCNd->Len();

        if( IsRedlineOn() && !IsShowOriginal( GetRedlineMode() ) )
        {
            if( bUndo )
            {
                pRedlUndo = new SwUndoRedlineSort( *pRedlPam,rOpt );
                GetIDocumentUndoRedo().DoUndo(false);
            }
            // erst den Bereich kopieren, dann
            SwNodeIndex aEndIdx( pEnd->nNode, 1 );
            SwNodeRange aRg( pStart->nNode, aEndIdx );
            GetNodes()._Copy( aRg, aEndIdx );

            // Bereich neu ist von pEnd->nNode+1 bis aEndIdx
            DeleteRedline( *pRedlPam, true, USHRT_MAX );

            pRedlPam->GetMark()->nNode.Assign( pEnd->nNode.GetNode(), 1 );
            pCNd = pRedlPam->GetCntntNode( sal_False );
            pRedlPam->GetMark()->nContent.Assign( pCNd, 0 );

            pRedlPam->GetPoint()->nNode.Assign( aEndIdx.GetNode() );
            pCNd = pRedlPam->GetCntntNode( sal_True );
            xub_StrLen nCLen = 0;
            if( !pCNd &&
                0 != (pCNd = GetNodes()[ aEndIdx.GetIndex()-1 ]->GetCntntNode()))
            {
                nCLen = pCNd->Len();
                pRedlPam->GetPoint()->nNode.Assign( *pCNd );
            }
            pRedlPam->GetPoint()->nContent.Assign( pCNd, nCLen );

            if( pRedlUndo )
                pRedlUndo->SetValues( rPaM );
        }
        else
        {
            DeleteRedline( *pRedlPam, true, USHRT_MAX );
            delete pRedlPam, pRedlPam = 0;
        }
    }

    SwNodeIndex aStart(pStart->nNode);
    SwSortElement::Init( this, rOpt );
    SwSortElements aSortArr;
    while( aStart <= pEnd->nNode )
    {
        // Iterieren ueber einen selektierten Bereich
        SwSortTxtElement* pSE = new SwSortTxtElement( aStart );
        aSortArr.Insert(pSE);
        aStart++;
    }

    // Und jetzt der Akt: Verschieben von Nodes und immer schoen auf UNDO
    // achten
    //
    sal_uLong nBeg = pStart->nNode.GetIndex();
    SwNodeRange aRg( aStart, aStart );

    if( bUndo && !pRedlUndo )
    {
        pUndoSort = new SwUndoSort(rPaM, rOpt);
        GetIDocumentUndoRedo().AppendUndo(pUndoSort);
    }

    GetIDocumentUndoRedo().DoUndo(false);

    for ( sal_uInt16 n = 0; n < aSortArr.Count(); ++n )
    {
        SwSortTxtElement* pBox = (SwSortTxtElement*)aSortArr[n];
        aStart      = nBeg + n;
        aRg.aStart  = pBox->aPos.GetIndex();
        aRg.aEnd    = aRg.aStart.GetIndex() + 1;

        // Nodes verschieben
        MoveNodeRange( aRg, aStart,
            IDocumentContentOperations::DOC_MOVEDEFAULT );

        // Undo Verschiebungen einpflegen
        if(pUndoSort)
            pUndoSort->Insert(pBox->nOrg, nBeg + n);
    }
    // Alle Elemente aus dem SortArray loeschen
    aSortArr.DeleteAndDestroy(0, aSortArr.Count());
    SwSortElement::Finit();

    if( pRedlPam )
    {
        if( pRedlUndo )
        {
            pRedlUndo->SetSaveRange( *pRedlPam );
            // UGLY: temp. enable Undo
            GetIDocumentUndoRedo().DoUndo(true);
            GetIDocumentUndoRedo().AppendUndo( pRedlUndo );
            GetIDocumentUndoRedo().DoUndo(false);
        }

        // nBeg is start of sorted range
        SwNodeIndex aSttIdx( GetNodes(), nBeg );

        // the copied range is deleted
        SwRedline *const pDeleteRedline(
            new SwRedline( nsRedlineType_t::REDLINE_DELETE, *pRedlPam ));

        // pRedlPam points to nodes that may be deleted (hidden) by
        // AppendRedline, so adjust it beforehand to prevent ASSERT
        pRedlPam->GetPoint()->nNode = aSttIdx;
        SwCntntNode* pCNd = aSttIdx.GetNode().GetCntntNode();
        pRedlPam->GetPoint()->nContent.Assign( pCNd, 0 );

        AppendRedline(pDeleteRedline, true);

        // the sorted range is inserted
        AppendRedline( new SwRedline( nsRedlineType_t::REDLINE_INSERT, *pRedlPam ), true);

        if( pRedlUndo )
        {
            SwNodeIndex aInsEndIdx( pRedlPam->GetMark()->nNode, -1 );
            pRedlPam->GetMark()->nNode = aInsEndIdx;
            SwCntntNode *const pPrevNode =
                pRedlPam->GetMark()->nNode.GetNode().GetCntntNode();
            pRedlPam->GetMark()->nContent.Assign( pPrevNode, pPrevNode->Len() );

            pRedlUndo->SetValues( *pRedlPam );
        }

        if( pRedlUndo )
            pRedlUndo->SetOffset( aSttIdx );

        delete pRedlPam, pRedlPam = 0;
    }
    GetIDocumentUndoRedo().DoUndo( bUndo );
    if( bUndo )
    {
        GetIDocumentUndoRedo().EndUndo( UNDO_END, NULL );
    }

    return sal_True;
}

/*--------------------------------------------------------------------
    Beschreibung: Tabelle sortieren im Document
 --------------------------------------------------------------------*/
sal_Bool SwDoc::SortTbl(const SwSelBoxes& rBoxes, const SwSortOptions& rOpt)
{
    // uebers SwDoc fuer Undo !!
    OSL_ENSURE( rBoxes.Count(), "keine gueltige Box-Liste" );
    SwTableNode* pTblNd = (SwTableNode*)rBoxes[0]->GetSttNd()->FindTableNode();
    if( !pTblNd )
        return sal_False;

    // Auf gehts sortieren
    // suche alle Boxen / Lines
    _FndBox aFndBox( 0, 0 );
    {
        _FndPara aPara( rBoxes, &aFndBox );
        pTblNd->GetTable().GetTabLines().ForEach( &_FndLineCopyCol, &aPara );;
    }

    if(!aFndBox.GetLines().Count())
        return sal_False;

    if( !IsIgnoreRedline() && GetRedlineTbl().Count() )
        DeleteRedline( *pTblNd, true, USHRT_MAX );

    sal_uInt16 nStart = 0;
    if( pTblNd->GetTable().GetRowsToRepeat() > 0 && rOpt.eDirection == SRT_ROWS )
    {
        // Oberste seleketierte Zeile
        _FndLines& rLines = aFndBox.GetLines();

        while( nStart < rLines.Count() )
        {
            // Verschachtelung durch Split Merge beachten,
            // die oberste rausholen
            SwTableLine* pLine = rLines[nStart]->GetLine();
            while ( pLine->GetUpper() )
                pLine = pLine->GetUpper()->GetUpper();

            if( pTblNd->GetTable().IsHeadline( *pLine ) )
                nStart++;
            else
                break;
        }
        // Alle selektierten in der HeaderLine ?  -> kein Offset
        if( nStart == rLines.Count() )
            nStart = 0;
    }

    // umschalten auf relative Formeln
    SwTableFmlUpdate aMsgHnt( &pTblNd->GetTable() );
    aMsgHnt.eFlags = TBL_RELBOXNAME;
    UpdateTblFlds( &aMsgHnt );

    // Tabelle als flache Array-Struktur
    FlatFndBox aFlatBox(this, aFndBox);

    if(!aFlatBox.IsSymmetric())
        return sal_False;

    // MIB 9.7.97: HTML-Layout loeschen
    pTblNd->GetTable().SetHTMLTableLayout( 0 );

    // --> FME 2004-11-26 #i37739# A simple 'MakeFrms' after the node sorting
    // does not work if the table is inside a frame and has no prev/next.
    SwNode2Layout aNode2Layout( *pTblNd );
    // <--

    // loesche die Frames der Tabelle
    pTblNd->DelFrms();
    // ? TL_CHART2: ?

    SwUndoSort* pUndoSort = 0;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        pUndoSort = new SwUndoSort( rBoxes[0]->GetSttIdx(),
                                    rBoxes[rBoxes.Count()-1]->GetSttIdx(),
                                   *pTblNd, rOpt, aFlatBox.HasItemSets() );
        GetIDocumentUndoRedo().AppendUndo(pUndoSort);
    }
    ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());

    // SchluesselElemente einsortieren
    sal_uInt16 nCount = (rOpt.eDirection == SRT_ROWS) ?
                    aFlatBox.GetRows() : aFlatBox.GetCols();

    // SortList nach Schluessel sortieren
    SwSortElement::Init( this, rOpt, &aFlatBox );
    SwSortElements aSortList;

    // wenn die HeaderLine wiederholt wird und die
    // Zeilen sortiert werden 1.Zeile nicht mitsortieren
    sal_uInt16 i;

    for( i = nStart; i < nCount; ++i)
    {
        SwSortBoxElement* pEle = new SwSortBoxElement( i );
        aSortList.Insert(pEle);
    }

    // nach Sortierung verschieben
    SwMovedBoxes aMovedList;
    for(i=0; i < aSortList.Count(); ++i)
    {
        SwSortBoxElement* pBox = (SwSortBoxElement*)aSortList[i];
        if(rOpt.eDirection == SRT_ROWS)
            MoveRow(this, aFlatBox, pBox->nRow, i + nStart, aMovedList, pUndoSort);
        else
            MoveCol(this, aFlatBox, pBox->nRow, i + nStart, aMovedList, pUndoSort);
    }

    // Restore table frames:
    // --> FME 2004-11-26 #i37739# A simple 'MakeFrms' after the node sorting
    // does not work if the table is inside a frame and has no prev/next.
    const sal_uLong nIdx = pTblNd->GetIndex();
    aNode2Layout.RestoreUpperFrms( GetNodes(), nIdx, nIdx + 1 );
    // <--

    // TL_CHART2: need to inform chart of probably changed cell names
    UpdateCharts( pTblNd->GetTable().GetFrmFmt()->GetName() );

    // Alle Elemente aus dem SortArray loeschen
    aSortList.DeleteAndDestroy( 0, aSortList.Count() );
    SwSortElement::Finit();

    SetModified();
    return sal_True;
}

/*--------------------------------------------------------------------
    Beschreibung: Zeilenweise verschieben
 --------------------------------------------------------------------*/
void MoveRow(SwDoc* pDoc, const FlatFndBox& rBox, sal_uInt16 nS, sal_uInt16 nT,
             SwMovedBoxes& rMovedList, SwUndoSort* pUD)
{
    for( sal_uInt16 i=0; i < rBox.GetCols(); ++i )
    {   // Alte Zellen-Pos bestimmen und merken
        const _FndBox* pSource = rBox.GetBox(i, nS);

        // neue Zellen-Pos
        const _FndBox* pTarget = rBox.GetBox(i, nT);

        const SwTableBox* pT = pTarget->GetBox();
        const SwTableBox* pS = pSource->GetBox();

        sal_Bool bMoved = rMovedList.GetPos(pT) != USHRT_MAX;

        // und verschieben
        MoveCell(pDoc, pS, pT, bMoved, pUD);

        rMovedList.Insert(pS, rMovedList.Count() );

        if( pS != pT )
        {
            SwFrmFmt* pTFmt = (SwFrmFmt*)pT->GetFrmFmt();
            const SfxItemSet* pSSet = rBox.GetItemSet( i, nS );

            if( pSSet ||
                SFX_ITEM_SET == pTFmt->GetItemState( RES_BOXATR_FORMAT ) ||
                SFX_ITEM_SET == pTFmt->GetItemState( RES_BOXATR_FORMULA ) ||
                SFX_ITEM_SET == pTFmt->GetItemState( RES_BOXATR_VALUE ) )
            {
                pTFmt = ((SwTableBox*)pT)->ClaimFrmFmt();
                pTFmt->LockModify();
                if( pTFmt->ResetFmtAttr( RES_BOXATR_FORMAT, RES_BOXATR_VALUE ) )
                    pTFmt->ResetFmtAttr( RES_VERT_ORIENT );

                if( pSSet )
                    pTFmt->SetFmtAttr( *pSSet );
                pTFmt->UnlockModify();
            }
        }
    }
}

/*--------------------------------------------------------------------
    Beschreibung: Spaltenweise verschieben
 --------------------------------------------------------------------*/
void MoveCol(SwDoc* pDoc, const FlatFndBox& rBox, sal_uInt16 nS, sal_uInt16 nT,
             SwMovedBoxes& rMovedList, SwUndoSort* pUD)
{
    for(sal_uInt16 i=0; i < rBox.GetRows(); ++i)
    {   // Alte Zellen-Pos bestimmen und merken
        const _FndBox* pSource = rBox.GetBox(nS, i);

        // neue Zellen-Pos
        const _FndBox* pTarget = rBox.GetBox(nT, i);

        // und verschieben
        const SwTableBox* pT = pTarget->GetBox();
        const SwTableBox* pS = pSource->GetBox();

        // und verschieben
        sal_Bool bMoved = rMovedList.GetPos(pT) != USHRT_MAX;
        MoveCell(pDoc, pS, pT, bMoved, pUD);

        rMovedList.Insert(pS, rMovedList.Count() );

        if( pS != pT )
        {
            SwFrmFmt* pTFmt = (SwFrmFmt*)pT->GetFrmFmt();
            const SfxItemSet* pSSet = rBox.GetItemSet( nS, i );

            if( pSSet ||
                SFX_ITEM_SET == pTFmt->GetItemState( RES_BOXATR_FORMAT ) ||
                SFX_ITEM_SET == pTFmt->GetItemState( RES_BOXATR_FORMULA ) ||
                SFX_ITEM_SET == pTFmt->GetItemState( RES_BOXATR_VALUE ) )
            {
                pTFmt = ((SwTableBox*)pT)->ClaimFrmFmt();
                pTFmt->LockModify();
                if( pTFmt->ResetFmtAttr( RES_BOXATR_FORMAT, RES_BOXATR_VALUE ) )
                    pTFmt->ResetFmtAttr( RES_VERT_ORIENT );

                if( pSSet )
                    pTFmt->SetFmtAttr( *pSSet );
                pTFmt->UnlockModify();
            }
        }
    }
}

/*--------------------------------------------------------------------
    Beschreibung: Eine einzelne Zelle verschieben
 --------------------------------------------------------------------*/
void MoveCell(SwDoc* pDoc, const SwTableBox* pSource, const SwTableBox* pTar,
              sal_Bool bMovedBefore, SwUndoSort* pUD)
{
    OSL_ENSURE(pSource && pTar,"Fehlende Quelle oder Ziel");

    if(pSource == pTar)
        return;

    if(pUD)
        pUD->Insert( pSource->GetName(), pTar->GetName() );

    // Pam Quelle auf den ersten ContentNode setzen
    SwNodeRange aRg( *pSource->GetSttNd(), 0, *pSource->GetSttNd() );
    SwNode* pNd = pDoc->GetNodes().GoNext( &aRg.aStart );

    // wurde die Zelle (Source) nicht verschoben
    // -> einen Leer-Node einfuegen und den Rest verschieben
    // ansonsten steht der Mark auf dem ersten Content-Node
    if( pNd->StartOfSectionNode() == pSource->GetSttNd() )
        pNd = pDoc->GetNodes().MakeTxtNode( aRg.aStart,
                (SwTxtFmtColl*)pDoc->GetDfltTxtFmtColl() );
    aRg.aEnd = *pNd->EndOfSectionNode();

    // Ist das Ziel leer(1 leerer Node vorhanden)
    // -> diesen loeschen und move
    // Ziel
    SwNodeIndex aTar( *pTar->GetSttNd() );
    pNd = pDoc->GetNodes().GoNext( &aTar );     // naechsten ContentNode
    sal_uLong nCount = pNd->EndOfSectionIndex() - pNd->StartOfSectionIndex();

    sal_Bool bDelFirst = sal_False;
    if( nCount == 2 )
    {
        OSL_ENSURE( pNd->GetCntntNode(), "Kein ContentNode");
        bDelFirst = !pNd->GetCntntNode()->Len() && bMovedBefore;
    }

    if(!bDelFirst)
    {   // Es besteht schon Inhalt -> alter I n h a l t  Section Down
        SwNodeRange aRgTar( aTar.GetNode(), 0, *pNd->EndOfSectionNode() );
        pDoc->GetNodes().SectionDown( &aRgTar );
    }

    // Einfuegen der Source
    SwNodeIndex aIns( *pTar->GetSttNd()->EndOfSectionNode() );
    pDoc->MoveNodeRange( aRg, aIns,
        IDocumentContentOperations::DOC_MOVEDEFAULT );

    // Falls erster Node leer -> weg damit
    if(bDelFirst)
        pDoc->GetNodes().Delete( aTar, 1 );
}

/*--------------------------------------------------------------------
    Beschreibung: Zweidimensionales Array aus FndBoxes generieren
 --------------------------------------------------------------------*/
FlatFndBox::FlatFndBox(SwDoc* pDocPtr, const _FndBox& rBox) :
    pDoc(pDocPtr),
    rBoxRef(rBox),
    pArr(0),
    ppItemSets(0),
    nRow(0),
    nCol(0)
{ // Ist das Array symmetrisch
    if((bSym = CheckLineSymmetry(rBoxRef)) != 0)
    {
        // Spalten/Reihen-Anzahl ermitteln
        nCols = GetColCount(rBoxRef);
        nRows = GetRowCount(rBoxRef);

        // lineares Array anlegen
        pArr = new _FndBoxPtr[ nRows * nCols ];
        _FndBox** ppTmp = (_FndBox**)pArr;
        memset( ppTmp, 0, sizeof(_FndBoxPtr) * nRows * nCols );


        FillFlat( rBoxRef );
    }
}

FlatFndBox::~FlatFndBox()
{
    _FndBox** ppTmp = (_FndBox**)pArr;
    delete [] ppTmp;

    if( ppItemSets )
        delete [] ppItemSets;
}

/*--------------------------------------------------------------------
    Beschreibung:   Alle Lines einer Box muessen gleichviel Boxen haben
 --------------------------------------------------------------------*/
sal_Bool FlatFndBox::CheckLineSymmetry(const _FndBox& rBox)
{
    const _FndLines &rLines = rBox.GetLines();
    sal_uInt16 nBoxes(0);

    // UeberLines iterieren
    for(sal_uInt16 i=0; i < rLines.Count(); ++i)
    {   // Die Boxen einer Line
        _FndLine* pLn = rLines[i];
        const _FndBoxes& rBoxes = pLn->GetBoxes();

        // Anzahl der Boxen aller Lines ungleich -> keine Symmetrie
        if( i  && nBoxes != rBoxes.Count())
            return sal_False;

        nBoxes = rBoxes.Count();
        if( !CheckBoxSymmetry( *pLn ) )
            return sal_False;
    }
    return sal_True;
}

/*--------------------------------------------------------------------
    Beschreibung:   Box auf Symmetrie pruefen
                    Alle Boxen einer Line muessen gleichviele Lines haben
 --------------------------------------------------------------------*/
sal_Bool FlatFndBox::CheckBoxSymmetry(const _FndLine& rLn)
{
    const _FndBoxes &rBoxes = rLn.GetBoxes();
    sal_uInt16 nLines(0);

    // Ueber Boxes iterieren
    for(sal_uInt16 i=0; i < rBoxes.Count(); ++i)
    {   // Die Boxen einer Line
        _FndBox* pBox = rBoxes[i];
        const _FndLines& rLines = pBox->GetLines();

        // Anzahl der Boxen aller Lines ungleich -> keine Symmetrie
        if( i && nLines != rLines.Count() )
            return sal_False;

        nLines = rLines.Count();
        if( nLines && !CheckLineSymmetry( *pBox ) )
            return sal_False;
    }
    return sal_True;
}

/*--------------------------------------------------------------------
    Beschreibung: max Anzahl der Spalten (Boxes)
 --------------------------------------------------------------------*/
sal_uInt16 FlatFndBox::GetColCount(const _FndBox& rBox)
{
    const _FndLines& rLines = rBox.GetLines();
    // Ueber Lines iterieren
    if( !rLines.Count() )
        return 1;

    sal_uInt16 nSum = 0;
    for( sal_uInt16 i=0; i < rLines.Count(); ++i )
    {
        // Die Boxen einer Line
        sal_uInt16 nCount = 0;
        const _FndBoxes& rBoxes = rLines[i]->GetBoxes();
        for( sal_uInt16 j=0; j < rBoxes.Count(); ++j )
                //  Rekursiv wirder ueber die Lines Iterieren
            nCount += rBoxes[j]->GetLines().Count()
                        ? GetColCount(*rBoxes[j]) : 1;

        if( nSum < nCount )
            nSum = nCount;
    }
    return nSum;
}

/*--------------------------------------------------------------------
    Beschreibung: max Anzahl der Zeilen (Lines)
 --------------------------------------------------------------------*/
sal_uInt16 FlatFndBox::GetRowCount(const _FndBox& rBox)
{
    const _FndLines& rLines = rBox.GetLines();
    if( !rLines.Count() )
        return 1;

    sal_uInt16 nLines = 0;
    for(sal_uInt16 i=0; i < rLines.Count(); ++i)
    {   // Die Boxen einer Line
        const _FndBoxes& rBoxes = rLines[i]->GetBoxes();
        sal_uInt16 nLn = 1;
        for(sal_uInt16 j=0; j < rBoxes.Count(); ++j)
            if( rBoxes[j]->GetLines().Count() )
                //  Rekursiv ueber die Lines Iterieren
                nLn = Max(GetRowCount(*rBoxes[j]), nLn);

        nLines = nLines + nLn;
    }
    return nLines;
}

/*--------------------------------------------------------------------
    Beschreibung: lineares Array aus atomaren FndBoxes erzeugen
 --------------------------------------------------------------------*/
void FlatFndBox::FillFlat(const _FndBox& rBox, sal_Bool bLastBox)
{
    sal_Bool bModRow = sal_False;
    const _FndLines& rLines = rBox.GetLines();

    // Ueber Lines iterieren
    sal_uInt16 nOldRow = nRow;
    for( sal_uInt16 i=0; i < rLines.Count(); ++i )
    {
        // Die Boxen einer Line
        const _FndBoxes& rBoxes = rLines[i]->GetBoxes();
        sal_uInt16 nOldCol = nCol;
        for( sal_uInt16 j = 0; j < rBoxes.Count(); ++j )
        {
            // Die Box pruefen ob es eine atomare Box ist
            const _FndBox*   pBox   = rBoxes[ j ];

            if( !pBox->GetLines().Count() )
            {
                // peichern
                sal_uInt16 nOff = nRow * nCols + nCol;
                *(pArr + nOff) = pBox;

                // sicher die Formel/Format/Value Werte
                const SwFrmFmt* pFmt = pBox->GetBox()->GetFrmFmt();
                if( SFX_ITEM_SET == pFmt->GetItemState( RES_BOXATR_FORMAT ) ||
                    SFX_ITEM_SET == pFmt->GetItemState( RES_BOXATR_FORMULA ) ||
                    SFX_ITEM_SET == pFmt->GetItemState( RES_BOXATR_VALUE ) )
                {
                    SfxItemSet* pSet = new SfxItemSet( pDoc->GetAttrPool(),
                                    RES_BOXATR_FORMAT, RES_BOXATR_VALUE,
                                    RES_VERT_ORIENT, RES_VERT_ORIENT, 0 );
                    pSet->Put( pFmt->GetAttrSet() );
                    if( !ppItemSets )
                    {
                        ppItemSets = new SfxItemSet*[ nRows * nCols ];
                        memset( ppItemSets, 0, sizeof(SfxItemSet*) * nRows * nCols );
                    }
                    *(ppItemSets + nOff ) = pSet;
                }

                bModRow = sal_True;
            }
            else
            {
                // Rekursiv wieder ueber die Lines einer Box Iterieren
                FillFlat( *pBox, ( j == rBoxes.Count()-1 ) );
            }
            nCol++;
        }
        if(bModRow)
            nRow++;
        nCol = nOldCol;
    }
    if(!bLastBox)
        nRow = nOldRow;
}

/*--------------------------------------------------------------------
    Beschreibung: Zugriff auf eine bestimmte Zelle
 --------------------------------------------------------------------*/
const _FndBox* FlatFndBox::GetBox(sal_uInt16 n_Col, sal_uInt16 n_Row) const
{
    sal_uInt16 nOff = n_Row * nCols + n_Col;
    const _FndBox* pTmp = *(pArr + nOff);

    OSL_ENSURE(n_Col < nCols && n_Row < nRows && pTmp, "unzulaessiger Array-Zugriff");
    return pTmp;
}

const SfxItemSet* FlatFndBox::GetItemSet(sal_uInt16 n_Col, sal_uInt16 n_Row) const
{
    OSL_ENSURE( !ppItemSets || ( n_Col < nCols && n_Row < nRows), "unzulaessiger Array-Zugriff");

    return ppItemSets ? *(ppItemSets + (n_Row * nCols + n_Col )) : 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
