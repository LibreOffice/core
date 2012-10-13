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

#include <float.h>
#include <hintids.hxx>
#include <hints.hxx>
#include <fmtfld.hxx>
#include <txtfld.hxx>
#include <frmfmt.hxx>
#include <layfrm.hxx>
#include <cntfrm.hxx>
#include <tabfrm.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <ndtxt.hxx>
#include <swtable.hxx>
#include <tblsel.hxx>
#include <cellfml.hxx>
#include <calc.hxx>
#include <expfld.hxx>
#include <usrfld.hxx>
#include <flddat.hxx>
#include <cellatr.hxx>
#include <ndindex.hxx>
#include <comphelper/string.hxx>

const sal_Unicode cRelTrenner = ',';
const sal_Unicode cRelKennung = '';        // CTRL-R

const sal_uInt16 cMAXSTACKSIZE = 50;

static const SwFrm* lcl_GetBoxFrm( const SwTableBox& rBox );
static long lcl_GetLongBoxNum( String& rStr );
static const SwTableBox* lcl_RelToBox( const SwTable&, const SwTableBox*, const String& );
static String lcl_BoxNmToRel( const SwTable&, const SwTableNode&,
                        const String& , const String& , bool );


/*************************************************************************
|*
|*  double SwTableBox::GetValue() const
|*      gebe den Wert dieser Box zurueck. Der Wert ergibt sich aus dem 1.
|*      TextNode. Beginnt dieser mit einer Zahl/Formel, so berechne diese;
|*      oder mit einem Feld, dann hole den Wert.
|*      Alle anderen Bedingungen returnen einen Fehler (oder 0 ?)
|*
|*************************************************************************/

double SwTableBox::GetValue( SwTblCalcPara& rCalcPara ) const
{
    double nRet = 0;

    if( rCalcPara.rCalc.IsCalcError() )
        return nRet;            // schon ein Fehler in der Berechnung

    rCalcPara.rCalc.SetCalcError( CALC_SYNTAX );    // default immer Fehler

    // keine Content Box ?
    if( !pSttNd  )
        return nRet;

    if( rCalcPara.IncStackCnt() )
        return nRet;

    rCalcPara.SetLastTblBox( this );

    // wird eine Rekursion erzeugt ?
    SwTableBox* pBox = (SwTableBox*)this;
    if( rCalcPara.pBoxStk->find( pBox ) != rCalcPara.pBoxStk->end() )
        return nRet;            // steht schon auf dem Stack: FEHLER

    // bei dieser Box nochmal aufsetzen
    rCalcPara.SetLastTblBox( this );

    rCalcPara.pBoxStk->insert( pBox );      // eintragen
    do {        // Middle-Check-Loop, damit aus dieser gesprungen werden kann
                // hier aufgespannt, damit am Ende der Box-Pointer aus dem
                // Stack ausgetragen wird
        SwDoc* pDoc = GetFrmFmt()->GetDoc();

        const SfxPoolItem* pItem;
        if( SFX_ITEM_SET == GetFrmFmt()->GetItemState(
                                RES_BOXATR_FORMULA, sal_False, &pItem ) )
        {
            rCalcPara.rCalc.SetCalcError( CALC_NOERR ); // wieder zuruecksetzen
            if( !((SwTblBoxFormula*)pItem)->IsValid() )
            {
                // dann berechnen
                const SwTable* pTmp = rCalcPara.pTbl;
                rCalcPara.pTbl = &pBox->GetSttNd()->FindTableNode()->GetTable();
                ((SwTblBoxFormula*)pItem)->Calc( rCalcPara, nRet );

                if( !rCalcPara.IsStackOverFlow() )
                {
                    SwFrmFmt* pFmt = pBox->ClaimFrmFmt();
                    SfxItemSet aTmp( pDoc->GetAttrPool(),
                                        RES_BOXATR_BEGIN,RES_BOXATR_END-1 );
                    aTmp.Put( SwTblBoxValue( nRet ) );
                    if( SFX_ITEM_SET != pFmt->GetItemState( RES_BOXATR_FORMAT ))
                        aTmp.Put( SwTblBoxNumFormat( 0 ));
                    pFmt->SetFmtAttr( aTmp );
                }
                rCalcPara.pTbl = pTmp;
            }
            else
                nRet = GetFrmFmt()->GetTblBoxValue().GetValue();
            break;
        }
        else if( SFX_ITEM_SET == pBox->GetFrmFmt()->GetItemState(
                                RES_BOXATR_VALUE, sal_False, &pItem ) )
        {
            rCalcPara.rCalc.SetCalcError( CALC_NOERR ); // wieder zuruecksetzen
            nRet = ((SwTblBoxValue*)pItem)->GetValue();
            break;
        }

        SwTxtNode* pTxtNd = pDoc->GetNodes()[ pSttNd->GetIndex() + 1 ]->GetTxtNode();
        if( !pTxtNd )
            break;

        xub_StrLen nSttPos = 0;
        const String& rTxt = pTxtNd->GetTxt();
        while( nSttPos < rTxt.Len() &&
                ( ' ' ==  rTxt.GetChar( nSttPos ) || '\t' ==  rTxt.GetChar( nSttPos ) ) )
            ++nSttPos;

        // beginnt an erster Position ein "RechenFeld", dann erfrage den Wert
        // von diesem
        sal_Unicode const Char = rTxt.GetChar(nSttPos);
        if ( nSttPos < rTxt.Len() &&
             ( CH_TXTATR_BREAKWORD == Char || CH_TXTATR_INWORD == Char ) )
        {
            SwIndex aIdx( pTxtNd, nSttPos );
            SwTxtFld * const pTxtFld = static_cast<SwTxtFld*>(
                pTxtNd->GetTxtAttrForCharAt(aIdx.GetIndex(), RES_TXTATR_FIELD));
            if( !pTxtFld )
                break;

            rCalcPara.rCalc.SetCalcError( CALC_NOERR ); // wieder zuruecksetzen

            const SwField* pFld = pTxtFld->GetFld().GetFld();
            switch( pFld->GetTyp()->Which()  )
            {
            case RES_SETEXPFLD:
                nRet = ((SwSetExpField*)pFld)->GetValue();
                break;
            case RES_USERFLD:
                nRet = ((SwUserFieldType*)pFld)->GetValue();
                break;
            case RES_TABLEFLD:
                {
                    SwTblField* pTblFld = (SwTblField*)pFld;
                    if( !pTblFld->IsValid() )       // ist der Wert gueltig ??
                    {
                        // die richtige Tabelle mitgeben!
                        const SwTable* pTmp = rCalcPara.pTbl;
                        rCalcPara.pTbl = &pTxtNd->FindTableNode()->GetTable();
                        pTblFld->CalcField( rCalcPara );
                        rCalcPara.pTbl = pTmp;
                    }
                    nRet = pTblFld->GetValue();
                }
                break;

            case RES_DATETIMEFLD:
                nRet = ((SwDateTimeField*)pFld)->GetValue();
                break;

            case RES_JUMPEDITFLD:
                //JP 14.09.98: Bug 56112 - der Platzhalter kann nie einen
                //              gueltigen Inhalt haben!
                nRet = 0;
                break;

            default:
                String const value(pFld->ExpandField(true));
                nRet = rCalcPara.rCalc.Calculate(value).GetDouble();
            }
        }
        else
        {
            // Ergebnis ist 0 und kein Fehler!
            rCalcPara.rCalc.SetCalcError( CALC_NOERR ); // wieder zuruecksetzen

            double aNum;
            String sTxt( rTxt.Copy( nSttPos ) );
            sal_uInt32 nFmtIndex = GetFrmFmt()->GetTblBoxNumFmt().GetValue();

            SvNumberFormatter* pNumFmtr = pDoc->GetNumberFormatter();

            if( NUMBERFORMAT_TEXT == nFmtIndex )
                nFmtIndex = 0;
            // JP 22.04.98: Bug 49659 - Sonderbehandlung fuer Prozent
            else if( sTxt.Len() &&
                    NUMBERFORMAT_PERCENT == pNumFmtr->GetType( nFmtIndex ))
            {
                sal_uInt32 nTmpFmt = 0;
                if( pNumFmtr->IsNumberFormat( sTxt, nTmpFmt, aNum ) &&
                    NUMBERFORMAT_NUMBER == pNumFmtr->GetType( nTmpFmt ))
                    sTxt += '%';
            }

            if( pNumFmtr->IsNumberFormat( sTxt, nFmtIndex, aNum ))
                nRet = aNum;
        }

// ?? sonst ist das ein Fehler
    } while( sal_False );

    if( !rCalcPara.IsStackOverFlow() )
    {
        rCalcPara.pBoxStk->erase( pBox );      // raus aus dem Stack
        rCalcPara.DecStackCnt();
    }

    //JP 12.01.99: mit Fehlererkennung, Bug 60794
    if( DBL_MAX == nRet )
        rCalcPara.rCalc.SetCalcError( CALC_SYNTAX );    // Fehler setzen

    return nRet;
}


// Struktur, die zum TabelleRechnen benoetigt wird

SwTblCalcPara::SwTblCalcPara( SwCalc& rCalculator, const SwTable& rTable )
    : pLastTblBox( 0 ), nStackCnt( 0 ), nMaxSize( cMAXSTACKSIZE ),
    rCalc( rCalculator ), pTbl( &rTable )
{
    pBoxStk = new SwTableSortBoxes;
}

SwTblCalcPara::~SwTblCalcPara()
{
    delete pBoxStk;
}

sal_Bool SwTblCalcPara::CalcWithStackOverflow()
{
    // falls ein StackUeberlauf erkannt wurde, sollte mit
    // der letzten Box noch mal aufgesetzt werden. Irgend
    // ein Weg sollte dann
    sal_uInt16 nSaveMaxSize = nMaxSize;

    nMaxSize = cMAXSTACKSIZE - 5;
    sal_uInt16 nCnt = 0;
    SwTableBoxes aStackOverFlows;
    do {
        SwTableBox* pBox = (SwTableBox*)pLastTblBox;
        nStackCnt = 0;
        rCalc.SetCalcError( CALC_NOERR );
        aStackOverFlows.insert( aStackOverFlows.begin() + nCnt++, pBox );

        pBoxStk->erase( pBox );
        pBox->GetValue( *this );
    } while( IsStackOverFlow() );

    nMaxSize = cMAXSTACKSIZE - 3;       // es muss mind. 1 Stufe tiefer gehen!

    // falls Rekursionen erkannt wurden
    nStackCnt = 0;
    rCalc.SetCalcError( CALC_NOERR );
    pBoxStk->clear();

    while( !rCalc.IsCalcError() && nCnt )
    {
        aStackOverFlows[ --nCnt ]->GetValue( *this );
        if( IsStackOverFlow() && !CalcWithStackOverflow() )
            break;
    }

    nMaxSize = nSaveMaxSize;
    aStackOverFlows.clear();
    return !rCalc.IsCalcError();
}


SwTableFormula::SwTableFormula( const String& rFormel )
    : sFormel( rFormel )
{
    eNmType = EXTRNL_NAME;
    bValidValue = false;
}

SwTableFormula::~SwTableFormula()
{
}

void SwTableFormula::_MakeFormula( const SwTable& rTbl, String& rNewStr,
                    String& rFirstBox, String* pLastBox, void* pPara ) const
{
    SwTblCalcPara* pCalcPara = (SwTblCalcPara*)pPara;
    if( pCalcPara->rCalc.IsCalcError() )        // ist schon Fehler gesetzt ?
        return;

    SwTableBox* pSttBox, *pEndBox = 0;

    rFirstBox.Erase(0,1);       // Kennung fuer Box loeschen
    // ein Bereich in dieser Klammer ?
    if( pLastBox )
    {
        pEndBox = reinterpret_cast<SwTableBox*>(sal::static_int_cast<sal_IntPtr>(pLastBox->ToInt64()));

        // ist das ueberhaupt ein gueltiger Pointer ??
        if( rTbl.GetTabSortBoxes().find( pEndBox ) == rTbl.GetTabSortBoxes().end() )
            pEndBox = 0;
        rFirstBox.Erase( 0, pLastBox->Len()+1 );
    }
    pSttBox = reinterpret_cast<SwTableBox*>(sal::static_int_cast<sal_IntPtr>(rFirstBox.ToInt64()));
    // ist das ueberhaupt ein gueltiger Pointer ??
    if( rTbl.GetTabSortBoxes().find( pSttBox ) == rTbl.GetTabSortBoxes().end() )
        pSttBox = 0;

    rNewStr += ' ';
    if( pEndBox && pSttBox )    // Bereich ?
    {
        // hole ueber das Layout alle "selectierten" Boxen und berechne
        // deren Werte
        SwSelBoxes aBoxes;
        GetBoxes( *pSttBox, *pEndBox, aBoxes );

        rNewStr += '(';
        bool bDelim = false;
        for (size_t n = 0; n < aBoxes.size() &&
                           !pCalcPara->rCalc.IsCalcError(); ++n)
        {
            const SwTableBox* pTblBox = aBoxes[n];
            if ( pTblBox->getRowSpan() >= 1 )
            {
                if( bDelim )
                    rNewStr += cListDelim;
                bDelim = true;
                rNewStr += pCalcPara->rCalc.GetStrResult(
                            pTblBox->GetValue( *pCalcPara ), sal_False );
            }
        }
        rNewStr += ')';
    }
    else if( pSttBox && !pLastBox )         // nur die StartBox ?
    {
                            //JP 12.01.99: und keine EndBox in der Formel!
        // Berechne den Wert der Box
        if ( pSttBox->getRowSpan() >= 1 )
        {
            rNewStr += pCalcPara->rCalc.GetStrResult(
                            pSttBox->GetValue( *pCalcPara ), sal_False );
        }
    }
    else
        pCalcPara->rCalc.SetCalcError( CALC_SYNTAX );   // Fehler setzen
    rNewStr += ' ';
}

void SwTableFormula::RelNmsToBoxNms( const SwTable& rTbl, String& rNewStr,
            String& rFirstBox, String* pLastBox, void* pPara ) const
{
    // relativen Namen zu Box-Namen (externe Darstellung)
    SwNode* pNd = (SwNode*)pPara;
    OSL_ENSURE( pNd, "Feld steht in keinem TextNode" );
    const SwTableBox *pRelBox, *pBox = (SwTableBox *)rTbl.GetTblBox(
                    pNd->FindTableBoxStartNode()->GetIndex() );

    rNewStr += rFirstBox.Copy(0,1);     // Kennung fuer Box erhalten
    rFirstBox.Erase(0,1);
    if( pLastBox )
    {
        if( 0 != ( pRelBox = lcl_RelToBox( rTbl, pBox, *pLastBox )) )
            rNewStr += pRelBox->GetName();
        else
            rNewStr.AppendAscii("A1");
        rNewStr += ':';
        rFirstBox.Erase( 0, pLastBox->Len()+1 );
    }

    if( 0 != ( pRelBox = lcl_RelToBox( rTbl, pBox, rFirstBox )) )
        rNewStr += pRelBox->GetName();
    else
        rNewStr.AppendAscii("A1");

    // Kennung fuer Box erhalten
    rNewStr += rFirstBox.GetChar( rFirstBox.Len() - 1 );
}

void SwTableFormula::RelBoxNmsToPtr( const SwTable& rTbl, String& rNewStr,
            String& rFirstBox, String* pLastBox, void* pPara ) const
{
    // relativen Namen zu Box-Pointern (interne Darstellung)
    SwNode* pNd = (SwNode*)pPara;
    OSL_ENSURE( pNd, "Feld steht in keinem Node" );
    const SwTableBox *pRelBox, *pBox = (SwTableBox*)rTbl.GetTblBox(
                    pNd->FindTableBoxStartNode()->GetIndex() );

    rNewStr += rFirstBox.Copy(0,1);     // Kennung fuer Box erhalten
    rFirstBox.Erase(0,1);
    if( pLastBox )
    {
        if( 0 != ( pRelBox = lcl_RelToBox( rTbl, pBox, *pLastBox )) )
            rNewStr += rtl::OUString::valueOf(static_cast<sal_Int64>((sal_PtrDiff)pRelBox));
        else
            rNewStr += '0';
        rNewStr += ':';
        rFirstBox.Erase( 0, pLastBox->Len()+1 );
    }

    if( 0 != ( pRelBox = lcl_RelToBox( rTbl, pBox, rFirstBox )) )
        rNewStr += rtl::OUString::valueOf(static_cast<sal_Int64>((sal_PtrDiff)pRelBox));
    else
        rNewStr += '0';

    // Kennung fuer Box erhalten
    rNewStr += rFirstBox.GetChar( rFirstBox.Len() - 1 );
}


void SwTableFormula::BoxNmsToRelNm( const SwTable& rTbl, String& rNewStr,
                    String& rFirstBox, String* pLastBox, void* pPara ) const
{
    // Box-Namen (externe Darstellung) zu relativen Namen
    SwNode* pNd = (SwNode*)pPara;
    OSL_ENSURE( pNd, "Feld steht in keinem Node" );
    const SwTableNode* pTblNd = pNd->FindTableNode();

    String sRefBoxNm;
    if( &pTblNd->GetTable() == &rTbl )
    {
        const SwTableBox *pBox = rTbl.GetTblBox(
                pNd->FindTableBoxStartNode()->GetIndex() );
        OSL_ENSURE( pBox, "Feld steht in keiner Tabelle" );
        sRefBoxNm = pBox->GetName();
    }

    rNewStr += rFirstBox.Copy(0,1);     // Kennung fuer Box erhalten
    rFirstBox.Erase(0,1);
    if( pLastBox )
    {
        rNewStr += lcl_BoxNmToRel( rTbl, *pTblNd, sRefBoxNm, *pLastBox,
                                eNmType == EXTRNL_NAME );
        rNewStr += ':';
        rFirstBox.Erase( 0, pLastBox->Len()+1 );
    }

    rNewStr += lcl_BoxNmToRel( rTbl, *pTblNd, sRefBoxNm, rFirstBox,
                            eNmType == EXTRNL_NAME );

    // Kennung fuer Box erhalten
    rNewStr += rFirstBox.GetChar( rFirstBox.Len() - 1 );
}


void SwTableFormula::PtrToBoxNms( const SwTable& rTbl, String& rNewStr,
                        String& rFirstBox, String* pLastBox, void* ) const
{
    // ein Bereich in dieser Klammer ?
    SwTableBox* pBox;

    rNewStr += rFirstBox.Copy(0,1);     // Kennung fuer Box erhalten
    rFirstBox.Erase(0,1);
    if( pLastBox )
    {
        pBox = reinterpret_cast<SwTableBox*>(sal::static_int_cast<sal_IntPtr>(pLastBox->ToInt64()));

        // ist das ueberhaupt ein gueltiger Pointer ??
        if( rTbl.GetTabSortBoxes().find( pBox ) != rTbl.GetTabSortBoxes().end() )
            rNewStr += pBox->GetName();
        else
            rNewStr += '?';
        rNewStr += ':';
        rFirstBox.Erase( 0, pLastBox->Len()+1 );
    }

    pBox = reinterpret_cast<SwTableBox*>(sal::static_int_cast<sal_IntPtr>(rFirstBox.ToInt64()));
    // ist das ueberhaupt ein gueltiger Pointer ??
    if( rTbl.GetTabSortBoxes().find( pBox ) != rTbl.GetTabSortBoxes().end() )
        rNewStr += pBox->GetName();
    else
        rNewStr += '?';

    // Kennung fuer Box erhalten
    rNewStr += rFirstBox.GetChar( rFirstBox.Len() - 1 );
}

void SwTableFormula::BoxNmsToPtr( const SwTable& rTbl, String& rNewStr,
                        String& rFirstBox, String* pLastBox, void* ) const
{
    // ein Bereich in dieser Klammer ?
    const SwTableBox* pBox;

    rNewStr += rFirstBox.Copy(0,1);     // Kennung fuer Box erhalten
    rFirstBox.Erase(0,1);
    if( pLastBox )
    {
        pBox = rTbl.GetTblBox( *pLastBox );
        rNewStr += rtl::OUString::valueOf(static_cast<sal_Int64>((sal_PtrDiff)pBox));
        rNewStr += ':';
        rFirstBox.Erase( 0, pLastBox->Len()+1 );
    }

    pBox = rTbl.GetTblBox( rFirstBox );
    rNewStr += rtl::OUString::valueOf(static_cast<sal_Int64>((sal_PtrDiff)pBox));

    // Kennung fuer Box erhalten
    rNewStr += rFirstBox.GetChar( rFirstBox.Len() - 1 );
}

    // erzeuge die externe (fuer UI) Formel
void SwTableFormula::PtrToBoxNm( const SwTable* pTbl )
{
    const SwNode* pNd = 0;
    FnScanFormel fnFormel = 0;
    switch( eNmType)
    {
    case INTRNL_NAME:
        if( pTbl )
            fnFormel = &SwTableFormula::PtrToBoxNms;
        break;
    case REL_NAME:
        if( pTbl )
        {
            fnFormel = &SwTableFormula::RelNmsToBoxNms;
            pNd = GetNodeOfFormula();
        }
        break;
    case EXTRNL_NAME:
        return;
    }
    sFormel = ScanString( fnFormel, *pTbl, (void*)pNd );
    eNmType = EXTRNL_NAME;
}

    // erzeuge die interne (in CORE) Formel
void SwTableFormula::BoxNmToPtr( const SwTable* pTbl )
{
    const SwNode* pNd = 0;
    FnScanFormel fnFormel = 0;
    switch( eNmType)
    {
    case EXTRNL_NAME:
        if( pTbl )
            fnFormel = &SwTableFormula::BoxNmsToPtr;
        break;
    case REL_NAME:
        if( pTbl )
        {
            fnFormel = &SwTableFormula::RelBoxNmsToPtr;
            pNd = GetNodeOfFormula();
        }
        break;
    case INTRNL_NAME:
        return;
    }
    sFormel = ScanString( fnFormel, *pTbl, (void*)pNd );
    eNmType = INTRNL_NAME;
}

    // erzeuge die relative (fuers Kopieren) Formel
void SwTableFormula::ToRelBoxNm( const SwTable* pTbl )
{
    const SwNode* pNd = 0;
    FnScanFormel fnFormel = 0;
    switch( eNmType)
    {
    case INTRNL_NAME:
    case EXTRNL_NAME:
        if( pTbl )
        {
            fnFormel = &SwTableFormula::BoxNmsToRelNm;
            pNd = GetNodeOfFormula();
        }
        break;
    case REL_NAME:
        return;
    }
    sFormel = ScanString( fnFormel, *pTbl, (void*)pNd );
    eNmType = REL_NAME;
}


String SwTableFormula::ScanString( FnScanFormel fnFormel, const SwTable& rTbl,
                                    void* pPara ) const
{
    String aStr;
    sal_uInt16 nFml = 0, nStt = 0, nEnd = 0, nTrenner;

    do {
        // falls der Formel ein Name vorangestellt ist, diese Tabelle
        // benutzen !!
        const SwTable* pTbl = &rTbl;

        nStt = sFormel.Search( '<', nFml );
        if( STRING_NOTFOUND != nStt )
        {
            while( STRING_NOTFOUND != nStt &&
                ( ' ' == sFormel.GetChar( nStt + 1 ) ||
                  '=' == sFormel.GetChar( nStt + 1 ) ) )
                nStt = sFormel.Search( '<', nStt + 1 );

            if( STRING_NOTFOUND != nStt )
                nEnd = sFormel.Search( '>', nStt+1 );
        }
        if( STRING_NOTFOUND == nStt || STRING_NOTFOUND == nEnd )
        {
            // den Rest setzen und beenden
            aStr.Insert( sFormel, nFml, sFormel.Len() - nFml );
            break;
        }
        aStr.Insert( sFormel, nFml, nStt - nFml );  // Anfang schreiben

        if( fnFormel != 0 )
        {
            // ist ein TabellenName vorangestellt ??
            // JP 16.02.99: SplitMergeBoxNm behandeln den Namen selbst
            // JP 22.02.99: der CAST muss fuer den Linux-Compiler sein
            // JP 28.06.99: rel. BoxName have no preceding tablename!
            if( fnFormel != (FnScanFormel)&SwTableFormula::_SplitMergeBoxNm &&
                1 < sFormel.Len() && cRelKennung != sFormel.GetChar( 1 ) &&
                STRING_NOTFOUND != ( nTrenner = sFormel.Search( '.', nStt ))
                && nTrenner < nEnd )
            {
                String sTblNm( sFormel.Copy( nStt, nEnd - nStt ));

                // falls im Namen schon die Punkte enthalten sind,
                // treten diese immer paarig auf!!! (A1.1.1 !!)
                if( (comphelper::string::getTokenCount(sTblNm, '.') - 1) & 1 )
                {
                    sTblNm.Erase( nTrenner - nStt );

                    // beim Bauen der Formel ist der TabellenName unerwuenscht
                    //JP 22.02.99: der CAST muss fuer den Linux-Compiler sein
                    if( fnFormel != (FnScanFormel)&SwTableFormula::_MakeFormula )
                        aStr += sTblNm;
                    nStt = nTrenner;

                    sTblNm.Erase( 0, 1 );   // Trenner loeschen
                    if( sTblNm != rTbl.GetFrmFmt()->GetName() )
                    {
                        // dann suchen wir uns mal unsere Tabelle:
                        const SwTable* pFnd = FindTable(
                                                *rTbl.GetFrmFmt()->GetDoc(),
                                                sTblNm );
                        if( pFnd )
                            pTbl = pFnd;
                        // ??
                        OSL_ENSURE( pFnd, "Tabelle nicht gefunden, was nun?" );
                    }
                }
            }

            String sBox( sFormel.Copy( nStt, nEnd - nStt + 1 ));
            // ein Bereich in dieser Klammer ?
            if( STRING_NOTFOUND != ( nTrenner = sFormel.Search( ':', nStt ))
                && nTrenner < nEnd )
            {
                // ohne die Anfangsklammer
                String aFirstBox( sFormel.Copy( nStt+1, nTrenner - nStt - 1 ));
                (this->*fnFormel)( *pTbl, aStr, sBox, &aFirstBox, pPara );
            }
            else
                (this->*fnFormel)( *pTbl, aStr, sBox, 0, pPara );
        }

        nFml = nEnd+1;
    } while( sal_True );
    return aStr;
}

const SwTable* SwTableFormula::FindTable( SwDoc& rDoc, const String& rNm ) const
{
    const SwFrmFmts& rTblFmts = *rDoc.GetTblFrmFmts();
    const SwTable* pTmpTbl, *pRet = 0;
    for( sal_uInt16 nFmtCnt = rTblFmts.size(); nFmtCnt; )
    {
        SwFrmFmt* pFmt = rTblFmts[ --nFmtCnt ];
        // falls wir von Sw3Writer gerufen werden, dann ist dem
        // FormatNamen eine Nummer anhaengig
        SwTableBox* pFBox;
        if( COMPARE_EQUAL == rNm.CompareTo( pFmt->GetName(),
                                        pFmt->GetName().Search( 0x0a ) ) &&
            0 != ( pTmpTbl = SwTable::FindTable( pFmt ) ) &&
            0 != (pFBox = pTmpTbl->GetTabSortBoxes()[0] ) &&
            pFBox->GetSttNd() &&
            pFBox->GetSttNd()->GetNodes().IsDocNodes() )
        {
            // eine Tabelle im normalen NodesArr
            pRet = pTmpTbl;
            break;
        }
    }
    return pRet;
}

static const SwFrm* lcl_GetBoxFrm( const SwTableBox& rBox )
{
    SwNodeIndex aIdx( *rBox.GetSttNd() );
    SwCntntNode* pCNd = aIdx.GetNodes().GoNext( &aIdx );
    OSL_ENSURE( pCNd, "Box hat keinen TextNode" );
    Point aPt;      // den im Layout 1. Frame returnen - Tab.Kopfzeile !!
    return pCNd->getLayoutFrm( pCNd->GetDoc()->GetCurrentLayout(), &aPt, NULL, sal_False );
}

static long lcl_GetLongBoxNum( String& rStr )
{
    sal_uInt16 nPos;
    long nRet;
    if( STRING_NOTFOUND == ( nPos = rStr.Search( cRelTrenner ) ))
    {
        nRet = rStr.ToInt32();
        rStr.Erase();
    }
    else
    {
        nRet = rStr.Copy( 0, nPos ).ToInt32();
        rStr.Erase( 0, nPos+1 );
    }
    return nRet;
}

static const SwTableBox* lcl_RelToBox( const SwTable& rTbl,
                                    const SwTableBox* pRefBox,
                                    const String& rGetName )
{
    // hole die Line
    const SwTableBox* pBox = 0;
    String sGetName( rGetName );

    // ist es denn wirklich eine relative Angabe??
    if( cRelKennung == sGetName.GetChar(0) )            // ja, ...
    {
        if( !pRefBox )
            return 0;

        sGetName.Erase( 0, 1 );

        const SwTableLines* pLines = (SwTableLines*)&rTbl.GetTabLines();
        const SwTableBoxes* pBoxes;
        const SwTableLine* pLine;

        // bestimme erst mal die Start-Werte der Box:
        pBox = (SwTableBox*)pRefBox;
        pLine = pBox->GetUpper();
        while( pLine->GetUpper() )
        {
            pBox = pLine->GetUpper();
            pLine = pBox->GetUpper();
        }
        sal_uInt16 nSttBox = pLine->GetTabBoxes().GetPos( pBox );
        sal_uInt16 nSttLine = rTbl.GetTabLines().GetPos( pLine );

        long nBoxOffset = lcl_GetLongBoxNum( sGetName ) + nSttBox;
        long nLineOffset = lcl_GetLongBoxNum( sGetName ) + nSttLine;

        if( nBoxOffset < 0 || nBoxOffset >= USHRT_MAX ||
            nLineOffset < 0 || nLineOffset >= USHRT_MAX )
            return 0;

        if( nLineOffset >= long(pLines->size()) )
            return 0;

        pLine = (*pLines)[ sal_uInt16(nLineOffset) ];

        // dann suche die Box
        pBoxes = &pLine->GetTabBoxes();
        if( nBoxOffset >= long(pBoxes->size()) )
            return 0;
        pBox = (*pBoxes)[ sal_uInt16(nBoxOffset) ];

        while( sGetName.Len() )
        {
            nSttBox = SwTable::_GetBoxNum( sGetName );
            pLines = &pBox->GetTabLines();
            if( nSttBox )
                --nSttBox;

            nSttLine = SwTable::_GetBoxNum( sGetName );

            // bestimme die Line
            if( !nSttLine || nSttLine > pLines->size() )
                break;
            pLine = (*pLines)[ nSttLine-1 ];

            // bestimme die Box
            pBoxes = &pLine->GetTabBoxes();
            if( nSttBox >= pBoxes->size() )
                break;
            pBox = (*pBoxes)[ nSttBox ];
        }

        if( pBox )
        {
            if( !pBox->GetSttNd() )
                // "herunterfallen lassen" bis zur ersten Box
                while( !pBox->GetTabLines().empty() )
                    pBox = pBox->GetTabLines().front()->GetTabBoxes().front();
        }
    }
    else
    {
        // sonst ist es eine absolute externe Darstellung:
        pBox = rTbl.GetTblBox( sGetName );
    }
    return pBox;
}

static String lcl_BoxNmToRel( const SwTable& rTbl, const SwTableNode& rTblNd,
                            const String& rRefBoxNm, const String& rGetStr,
                            bool bExtrnlNm )
{
    String sCpy( rRefBoxNm );
    String sTmp( rGetStr );
    if( !bExtrnlNm )
    {
        // in die Externe Darstellung umwandeln.
        SwTableBox* pBox = reinterpret_cast<SwTableBox*>(sal::static_int_cast<sal_IntPtr>(sTmp.ToInt64()));
        if( rTbl.GetTabSortBoxes().find( pBox ) == rTbl.GetTabSortBoxes().end() )
            return rtl::OUString('?');
        sTmp = pBox->GetName();
    }

    // sollte die es eine Tabellen uebergreifende Formel sein, dann behalte
    // die externe Darstellung bei:
    if( &rTbl == &rTblNd.GetTable() )
    {
        long nBox = SwTable::_GetBoxNum( sTmp, sal_True );
        nBox -= SwTable::_GetBoxNum( sCpy, sal_True );
        long nLine = SwTable::_GetBoxNum( sTmp );
        nLine -= SwTable::_GetBoxNum( sCpy );

        sCpy = sTmp;        //JP 01.11.95: den Rest aus dem BoxNamen anhaengen

        sTmp = cRelKennung;
        sTmp += String::CreateFromInt32( nBox );
        sTmp += cRelTrenner;
        sTmp += String::CreateFromInt32( nLine );

        if( sCpy.Len() )
        {
            sTmp += cRelTrenner;
            sTmp += sCpy;
        }
    }

    if( sTmp.Len() && '>' == sTmp.GetChar( sTmp.Len() - 1 ))
        sTmp.Erase( sTmp.Len()-1 );

    return sTmp;
}

void SwTableFormula::GetBoxesOfFormula( const SwTable& rTbl,
                                        SwSelBoxes& rBoxes )
{
    rBoxes.clear();

    BoxNmToPtr( &rTbl );
    ScanString( &SwTableFormula::_GetFmlBoxes, rTbl, &rBoxes );
}

void SwTableFormula::_GetFmlBoxes( const SwTable& rTbl, String& ,
                    String& rFirstBox, String* pLastBox, void* pPara ) const
{
    SwSelBoxes* pBoxes = (SwSelBoxes*)pPara;
    SwTableBox* pSttBox, *pEndBox = 0;

    rFirstBox.Erase(0,1);       // Kennung fuer Box loeschen
    // ein Bereich in dieser Klammer ?
    if( pLastBox )
    {
        pEndBox = reinterpret_cast<SwTableBox*>(sal::static_int_cast<sal_IntPtr>(pLastBox->ToInt64()));

        // ist das ueberhaupt ein gueltiger Pointer ??
        if( rTbl.GetTabSortBoxes().find( pEndBox ) == rTbl.GetTabSortBoxes().end() )
            pEndBox = 0;
        rFirstBox.Erase( 0, pLastBox->Len()+1 );
    }

    pSttBox = reinterpret_cast<SwTableBox*>(sal::static_int_cast<sal_IntPtr>(rFirstBox.ToInt64()));
    // ist das ueberhaupt ein gueltiger Pointer ??
    if( rTbl.GetTabSortBoxes().find( pSttBox ) == rTbl.GetTabSortBoxes().end() )
        pSttBox = 0;

    if( pEndBox && pSttBox )    // Bereich ?
    {
        // ueber das Layout alle "selectierten" Boxen und berechne
        // deren Werte
        SwSelBoxes aBoxes;
        GetBoxes( *pSttBox, *pEndBox, aBoxes );
        pBoxes->insert( aBoxes );
    }
    else if( pSttBox )          // nur die StartBox ?
        pBoxes->insert( pSttBox );
}

void SwTableFormula::GetBoxes( const SwTableBox& rSttBox,
                                const SwTableBox& rEndBox,
                                SwSelBoxes& rBoxes ) const
{
    // hole ueber das Layout alle "selektierten" Boxen
    const SwLayoutFrm *pStt, *pEnd;
    const SwFrm* pFrm = lcl_GetBoxFrm( rSttBox );
    pStt = pFrm ? pFrm->GetUpper() : 0;
    pEnd = ( 0 != (pFrm = lcl_GetBoxFrm( rEndBox ))) ? pFrm->GetUpper() : 0;
    if( !pStt || !pEnd )
        return ;                        // no valid selection

    GetTblSel( pStt, pEnd, rBoxes, 0 );

    const SwTable* pTbl = pStt->FindTabFrm()->GetTable();

    // filter die Kopfzeilen-Boxen heraus:
    if( pTbl->GetRowsToRepeat() > 0 )
    {
        do {    // middle-check loop
            const SwTableLine* pLine = rSttBox.GetUpper();
            while( pLine->GetUpper() )
                pLine = pLine->GetUpper()->GetUpper();

            if( pTbl->IsHeadline( *pLine ) )
                break;      // Headline mit im Bereich !

            // vielleicht ist ja Start und Ende vertauscht
            pLine = rEndBox.GetUpper();
            while ( pLine->GetUpper() )
                pLine = pLine->GetUpper()->GetUpper();

            if( pTbl->IsHeadline( *pLine ) )
                break;      // Headline mit im Bereich !

            const SwTabFrm *pTable = pStt->FindTabFrm();
            const SwTabFrm *pEndTable = pEnd->FindTabFrm();

            if( pTable == pEndTable )       // keine gespl. Tabelle
                break;

            // dann mal die Tabellenkoepfe raus:
            for (size_t n = 0; n < rBoxes.size(); ++n)
            {
                pLine = rBoxes[n]->GetUpper();
                while( pLine->GetUpper() )
                    pLine = pLine->GetUpper()->GetUpper();

                if( pTbl->IsHeadline( *pLine ) )
                    rBoxes.erase( rBoxes.begin() + n-- );
            }
        } while( sal_False );
    }
}

    // sind alle Boxen gueltig, auf die sich die Formel bezieht?
void SwTableFormula::_HasValidBoxes( const SwTable& rTbl, String& ,
                    String& rFirstBox, String* pLastBox, void* pPara ) const
{
    bool* pBValid = (bool*)pPara;
    if( *pBValid )      // einmal falsch, immer falsch
    {
        SwTableBox* pSttBox = 0, *pEndBox = 0;
        rFirstBox.Erase(0,1);       // Kennung fuer Box loeschen

        // ein Bereich in dieser Klammer ?
        if( pLastBox )
            rFirstBox.Erase( 0, pLastBox->Len()+1 );

        switch( eNmType)
        {
        case INTRNL_NAME:
            if( pLastBox )
                pEndBox = reinterpret_cast<SwTableBox*>(sal::static_int_cast<sal_IntPtr>(pLastBox->ToInt64()));
            pSttBox = reinterpret_cast<SwTableBox*>(sal::static_int_cast<sal_IntPtr>(rFirstBox.ToInt64()));
            break;

        case REL_NAME:
            {
                const SwNode* pNd = GetNodeOfFormula();
                const SwTableBox* pBox = !pNd ? 0
                                               : (SwTableBox *)rTbl.GetTblBox(
                                    pNd->FindTableBoxStartNode()->GetIndex() );
                if( pLastBox )
                    pEndBox = (SwTableBox*)lcl_RelToBox( rTbl, pBox, *pLastBox );
                pSttBox = (SwTableBox*)lcl_RelToBox( rTbl, pBox, rFirstBox );
            }
            break;

        case EXTRNL_NAME:
            if( pLastBox )
                pEndBox = (SwTableBox*)rTbl.GetTblBox( *pLastBox );
            pSttBox = (SwTableBox*)rTbl.GetTblBox( rFirstBox );
            break;
        }

        // sind das gueltige Pointer ?
        if( ( pLastBox &&
              ( !pEndBox || rTbl.GetTabSortBoxes().find( pEndBox ) == rTbl.GetTabSortBoxes().end() ) ) ||
            ( !pSttBox || rTbl.GetTabSortBoxes().find( pSttBox ) == rTbl.GetTabSortBoxes().end() ) )
                *pBValid = false;
    }
}

bool SwTableFormula::HasValidBoxes() const
{
    bool bRet = true;
    const SwNode* pNd = GetNodeOfFormula();
    if( pNd && 0 != ( pNd = pNd->FindTableNode() ) )
        ScanString( &SwTableFormula::_HasValidBoxes,
                        ((SwTableNode*)pNd)->GetTable(), &bRet );
    return bRet;
}


sal_uInt16 SwTableFormula::GetLnPosInTbl( const SwTable& rTbl, const SwTableBox* pBox )
{
    sal_uInt16 nRet = USHRT_MAX;
    if( pBox )
    {
        const SwTableLine* pLn = pBox->GetUpper();
        while( pLn->GetUpper() )
            pLn = pLn->GetUpper()->GetUpper();
        nRet = rTbl.GetTabLines().GetPos( pLn );
    }
    return nRet;
}

void SwTableFormula::_SplitMergeBoxNm( const SwTable& rTbl, String& rNewStr,
                    String& rFirstBox, String* pLastBox, void* pPara ) const
{
    SwTableFmlUpdate& rTblUpd = *(SwTableFmlUpdate*)pPara;

    rNewStr += rFirstBox.Copy(0,1);     // Kennung fuer Box erhalten
    rFirstBox.Erase(0,1);

    String sTblNm;
    const SwTable* pTbl = &rTbl;

    String* pTblNmBox = pLastBox ? pLastBox : &rFirstBox;

    sal_uInt16 nLastBoxLen = pTblNmBox->Len();
    sal_uInt16 nTrenner = pTblNmBox->Search( '.' );
    if( STRING_NOTFOUND != nTrenner &&
        // falls im Namen schon die Punkte enthalten sind,
        // treten diese immer paarig auf!!! (A1.1.1 !!)
        (comphelper::string::getTokenCount(*pTblNmBox, '.') - 1) & 1 )
    {
        sTblNm = pTblNmBox->Copy( 0, nTrenner );
        pTblNmBox->Erase( 0, nTrenner + 1);// den Punkt entfernen
        const SwTable* pFnd = FindTable( *rTbl.GetFrmFmt()->GetDoc(), sTblNm );
        if( pFnd )
            pTbl = pFnd;

        if( TBL_MERGETBL == rTblUpd.eFlags )
        {
            if( pFnd )
            {
                if( pFnd == rTblUpd.DATA.pDelTbl )
                {
                    if( rTblUpd.pTbl != &rTbl )         // es ist nicht die akt.
                        (rNewStr += rTblUpd.pTbl->GetFrmFmt()->GetName() )
                            += '.'; // den neuen Tabellen Namen setzen
                    rTblUpd.bModified = sal_True;
                }
                else if( pFnd != rTblUpd.pTbl ||
                    ( rTblUpd.pTbl != &rTbl && &rTbl != rTblUpd.DATA.pDelTbl))
                    (rNewStr += sTblNm ) += '.';    // den Tabellen Namen behalten
                else
                    rTblUpd.bModified = sal_True;
            }
            else
                (rNewStr += sTblNm ) += '.';    // den Tabellen Namen behalten

        }
    }
    if( pTblNmBox == pLastBox )
        rFirstBox.Erase( 0, nLastBoxLen + 1 );

    SwTableBox* pSttBox = 0, *pEndBox = 0;
    switch( eNmType )
    {
    case INTRNL_NAME:
        if( pLastBox )
            pEndBox = reinterpret_cast<SwTableBox*>(sal::static_int_cast<sal_IntPtr>(pLastBox->ToInt64()));
        pSttBox = reinterpret_cast<SwTableBox*>(sal::static_int_cast<sal_IntPtr>(rFirstBox.ToInt64()));
        break;

    case REL_NAME:
        {
            const SwNode* pNd = GetNodeOfFormula();
            const SwTableBox* pBox = pNd ? pTbl->GetTblBox(
                            pNd->FindTableBoxStartNode()->GetIndex() ) : 0;
            if( pLastBox )
                pEndBox = (SwTableBox*)lcl_RelToBox( *pTbl, pBox, *pLastBox );
            pSttBox = (SwTableBox*)lcl_RelToBox( *pTbl, pBox, rFirstBox );
        }
        break;

    case EXTRNL_NAME:
        if( pLastBox )
            pEndBox = (SwTableBox*)pTbl->GetTblBox( *pLastBox );
        pSttBox = (SwTableBox*)pTbl->GetTblBox( rFirstBox );
        break;
    }

    if( pLastBox && pTbl->GetTabSortBoxes().find( pEndBox ) == pTbl->GetTabSortBoxes().end() )
        pEndBox = 0;
    if( pTbl->GetTabSortBoxes().find( pSttBox ) == pTbl->GetTabSortBoxes().end() )
        pSttBox = 0;

    if( TBL_SPLITTBL == rTblUpd.eFlags )
    {
        // wo liegen die Boxen, in der "alten" oder in der neuen Tabelle?
        bool bInNewTbl = false;
        if( pLastBox )
        {
            // das ist die "erste" Box in der Selektion. Die bestimmt ob die
            // Formel in der alten oder neuen Tabelle steht.
            sal_uInt16 nEndLnPos = SwTableFormula::GetLnPosInTbl( *pTbl, pEndBox ),
                    nSttLnPos = SwTableFormula::GetLnPosInTbl( *pTbl, pSttBox );

            if( USHRT_MAX != nSttLnPos && USHRT_MAX != nEndLnPos &&
                ((rTblUpd.nSplitLine <= nSttLnPos) ==
                (rTblUpd.nSplitLine <= nEndLnPos)) )
            {
                // bleiben in der gleichen Tabelle
                bInNewTbl = rTblUpd.nSplitLine <= nEndLnPos &&
                                    pTbl == rTblUpd.pTbl;
            }
            else
            {
                // das ist aufjedenfall eine ungueltige Formel, also fuers
                // Undo auf Modified setzen
                rTblUpd.bModified = sal_True;
                if( pEndBox )
                    bInNewTbl = USHRT_MAX != nEndLnPos &&
                                    rTblUpd.nSplitLine <= nEndLnPos &&
                                    pTbl == rTblUpd.pTbl;
            }
        }
        else
        {
            sal_uInt16 nSttLnPos = SwTableFormula::GetLnPosInTbl( *pTbl, pSttBox );
            // dann landet das Teil in der neuen Tabelle?
            bInNewTbl = USHRT_MAX != nSttLnPos &&
                            rTblUpd.nSplitLine <= nSttLnPos &&
                            pTbl == rTblUpd.pTbl;
        }

        // wenn die Formel selbst in der neuen Tabellen landet
        if( rTblUpd.bBehindSplitLine )
        {
            if( !bInNewTbl )
            {
                rTblUpd.bModified = sal_True;
                ( rNewStr += rTblUpd.pTbl->GetFrmFmt()->GetName() ) += '.';
            }
            else if( sTblNm.Len() )
                ( rNewStr += sTblNm ) += '.';
        }
        else if( bInNewTbl )
        {
            rTblUpd.bModified = sal_True;
            ( rNewStr += *rTblUpd.DATA.pNewTblNm ) += '.';
        }
        else if( sTblNm.Len() )
            ( rNewStr += sTblNm ) += '.';
    }

    if( pLastBox )
        ( rNewStr += rtl::OUString::valueOf(static_cast<sal_Int64>((sal_PtrDiff)pEndBox)) ) += ':';
    ( rNewStr += rtl::OUString::valueOf(static_cast<sal_Int64>((sal_PtrDiff)pSttBox)) )
              += rFirstBox.GetChar( rFirstBox.Len() - 1 );
}

    // erzeuge die externe Formel, beachte aber das die Formel
    // in einer gesplitteten/gemergten Tabelle landet
void SwTableFormula::ToSplitMergeBoxNm( SwTableFmlUpdate& rTblUpd )
{
    const SwTable* pTbl;
    const SwNode* pNd = GetNodeOfFormula();
    if( pNd && 0 != ( pNd = pNd->FindTableNode() ))
        pTbl = &((SwTableNode*)pNd)->GetTable();
    else
        pTbl = rTblUpd.pTbl;

    sFormel = ScanString( &SwTableFormula::_SplitMergeBoxNm, *pTbl, (void*)&rTblUpd );
    eNmType = INTRNL_NAME;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
