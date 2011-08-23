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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <stdio.h>		//dort ist EOF (-1) definiert

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SVX_PAPERINF_HXX //autogen
#include <bf_svx/paperinf.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <bf_svx/lrspitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <bf_svx/ulspitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <bf_svx/brshitem.hxx>
#endif
#ifndef _SVX_SHADITEM_HXX //autogen
#include <bf_svx/shaditem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <bf_svx/boxitem.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX
#include <bf_svx/adjitem.hxx>
#endif
#ifndef _SVX_BRKITEM_HXX
#include <bf_svx/brkitem.hxx>
#endif
#ifndef _SVARRAY_HXX //autogen
#include <bf_svtools/svarray.hxx>
#endif
#ifndef _URLOBJ_HXX  //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <bf_svx/unolingu.hxx>
#endif
#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif

#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTFLCNT_HXX //autogen
#include <fmtflcnt.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FMTHDFT_HXX //autogen
#include <fmthdft.hxx>
#endif
#ifndef _TXTFTN_HXX //autogen
#include <txtftn.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _FMTFTN_HXX //autogen
#include <fmtftn.hxx>
#endif
#ifndef _FMTSRND_HXX //autogen
#include <fmtsrnd.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _NUMRULE_HXX
#include <numrule.hxx>
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>				// fuer SwPam
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _NODE_HXX
#include <node.hxx>				// SwTableNode
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _FMTCOL_HXX
#include <fmtcol.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _FTNINFO_HXX
#include <ftninfo.hxx>			// SwFtnInfo
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>			// GetTabLines(), ...
#endif
#ifndef _GRFATR_HXX
#include <grfatr.hxx>
#endif
#ifndef _TOX_HXX
#include <tox.hxx>
#endif
#ifndef _NDGRF_HXX
#include <ndgrf.hxx>
#endif
#ifndef _W4WSTK_HXX
#include <w4wstk.hxx>			// fuer den Attribut Stack
#endif
#ifndef _W4WPAR_HXX
#include <w4wpar.hxx>
#endif
#ifndef _W4WGRAF_HXX
#include <w4wgraf.hxx>
#endif
#ifndef _TBLSEL_HXX
#include <tblsel.hxx>			// SwSelBoxes
#endif
#ifndef _VIEWSH_HXX		// for the pagedescname from the ShellRes
#include <viewsh.hxx>
#endif
#ifndef _SHELLRES_HXX	// for the pagedescname from the ShellRes
#include <shellres.hxx>
#endif
#include "bf_so3/staticbaseurl.hxx"
namespace binfilter {


// Aktivierung der LR-Rand-Einstellungen fuer Kopf-/Fusszeilen
#define HDFT_MARGIN



#define MOGEL_WW2

                            // fuer Berechnung der minimalen FrameSize
#define MAX_BORDER_SIZE 210			// so breit ist max. der Border
#define MAX_EMPTY_BORDER 10			// fuer +-1-Fehler, mindestens 1

#define DEFAULT_TAB_CELL_DISTANCE 42	// Default: 0.7 mm Abstand


SV_DECL_PTRARR_DEL(W4WTabBorders, UShortPtr, 64, 64)//STRIP008 ;
SV_IMPL_PTRARR(    W4WTabBorders, UShortPtr        );

typedef SwSelBoxes_SAR* SwSelBoxes_SARPtr;

SV_DECL_PTRARR_DEL(W4WMergeGroups, SwSelBoxes_SARPtr, 16,16)//STRIP008 ;
SV_IMPL_PTRARR(    W4WMergeGroups, SwSelBoxes_SARPtr       );

inline const SwNodeIndex & PtNd( const SwPaM * pPam )
    { return pPam->GetPoint()->nNode; }
inline SwNodeIndex & PtNd( SwPaM* pPam )
    { return pPam->GetPoint()->nNode; }
inline const SwIndex & PtCnt( const SwPaM * pPam )
    { return pPam->GetPoint()->nContent; }
inline SwIndex & PtCnt( SwPaM* pPam )
    { return pPam->GetPoint()->nContent; }


static BOOL JoinNode( SwPaM* pPam )
{
    pPam->GetPoint()->nContent = 0;
    // an den Anfang der Zeile gehen
    if ( !pPam->Move( fnMoveBackward, fnGoCntnt ) )
    {
        return FALSE;		// Cursor geht nicht zurueck
    }

    SwNode* pNd = pPam->GetDoc()->GetNodes()[ PtNd(pPam) ];

    if( pNd->FindTableNode() || !pNd->IsCntntNode() )
    {
        return FALSE;		// hier wuerde JoinNext abstuerzen
    }

    ((SwCntntNode*)pNd)->JoinNext();

    return TRUE;
}


/****************************************************************************
Indicex: Gliederung, Stichwortverzeichnis ...
*****************************************************************************/

void SwW4WParser::Read_BeginMarkedText()	// (BMT)
{
    BYTE nType;
    USHORT nStrLen = 0;
    bToxOpen = TRUE;

    if ( bStyleDef || bStyleOnOff ) return;

    if( GetHexByte( nType ) && !nError )
    {
        BOOL bOldTxtInDoc = bTxtInDoc; bTxtInDoc = FALSE;

        if( W4WR_TXTERM == GetNextRecord() && !nError ){	// lese opt. Namen
            nStrLen = nChrCnt;						// Stringlaenge merken
            Flush();								// '\0' hinter Namen setzen
        }
        bTxtInDoc = bOldTxtInDoc;

        TOXTypes eTox;
        if      ( nType <= 0x1f ) eTox = TOX_CONTENT;
        else if ( nType <= 0x3f ) eTox = TOX_INDEX;
        else                      eTox = TOX_USER;	// TOX_USER eigentlich


        BYTE nNum = 0;

        if( 33 == nDocType ) 			// W4W ist bei anderen Doc-Typen
        {                               // noch nicht konsistent
            nNum = (nType & 0x1f);

            const USHORT nMaxLevel = SwForm::GetFormMaxLevel( eTox );

            if( nNum >= nMaxLevel )
                nNum  = nMaxLevel-1;

            /*
            * Achtung - AmiPro-Hack:
            *
            * wenn ein .<BMT> in einem Bereich auftaucht, der ueber
            * einen Style formatiert ist, werten wir eTox aus:
            * Falls es ein Gliederungs-Eintrag sein soll, weisen
            * wir dem Style die entsprechende Gliederungs-Stufe zu.
            *
            */
            if( (USHRT_MAX != nAktStyleId) && (TOX_CONTENT == eTox) )
            {
                SwTxtFmtColl* pAktColl = GetAktColl();
                ASSERT( pAktColl, "StyleId ungueltig" );

                const SwTxtFmtColls* pDocTxtFmtColls = pDoc->GetTxtFmtColls();

                BOOL bSchonDa = FALSE;
                for(USHORT i = 0; i < pDocTxtFmtColls->Count(); i++ )
                {
                    if( nNum == pDocTxtFmtColls->GetObject( i )->GetOutlineLevel() )
                    {
                        if( pAktColl == pDocTxtFmtColls->GetObject( i ) )
                        {
                          return;  // Style hat schon diese Gliederungstiefe
                        }
                        bSchonDa = TRUE;
                        break;
                    }
                }
                if( !bSchonDa )
                {
                    pAktColl->SetOutlineLevel( nNum );
                    return;
                    // untenstehende 'harte' Verz.-Markierung nicht noetig
                }
            }
        }

        USHORT nTOXCount = pDoc->GetTOXTypeCount( eTox );
        if( !nTOXCount )
            pDoc->InsertTOXType(
                SwTOXType( eTox, SwTOXBase::GetTOXName( eTox ) ) );

        const SwTOXType* pT = pDoc->GetTOXType( eTox, 0 );

        SwTOXMark aM( pT );

        if( TOX_INDEX != eTox ) // SetLevel geht nicht bei Stichwortverz.
        {
            aM.SetLevel( nNum );
        }

        if ( nStrLen != 0 )                        // Text mit angegeben
        {
            aM.SetAlternativeText( String( aCharBuffer ) );
            pDoc->Insert( *pCurPaM, aM );
        }else
        {
            pCtrlStck->NewAttr( *pCurPaM->GetPoint(), aM );
            bToxOpen = TRUE;
        }
    }
}


void SwW4WParser::Read_EndMarkedText()	//(EMT)
{
    BYTE nType;

    if( bToxOpen && GetHexByte( nType ) && !nError )
    {
        pCtrlStck->SetAttr( *pCurPaM->GetPoint(), RES_TXTATR_TOXMARK );
        bToxOpen = FALSE;
    }
}


// returnt den abs. Rand der Seite (SeitenRand+NodeRand) ( fuer Tabs )


long SwW4WParser::GetLeftMargin() const
{
    // gebe die Groesse und die Raender der Seite aus
    const SvxLRSpaceItem * pNdLR;
    if ( bStyleDef )
        pNdLR = &GetAktColl()->GetLRSpace();
    else
        pNdLR = (const SvxLRSpaceItem*)pCtrlStck->GetFmtAttr( *pCurPaM,
                                                              RES_LR_SPACE );
        //!!! findet manchmal nicht das richtige LRSpace
        // anscheinend dann, wenn das Attr nicht direkt am Absatz klebt,
        // sondern ueber ein Style gueltig ist.
    return nPgLeft + pNdLR->GetTxtLeft();
}



/***************************************************************************************
nummerierte Listen
***************************************************************************************/

void SwW4WParser::Read_ParagraphNumber()	   		// (PGN)
{
    if( !bFootnoteDef )
    {
        if(    ( 44 != nDocType ) 			// nur WinWord 2
            && ( 48 != nDocType ) ) return;	// und WordPerfekt 6-8

        // ACHTUNG: in Style-Definitionen alles ignorieren und bis
        // zum .<EPN> ueberlesen, denn der Kode wird von W4W in den
        // ueber diesen Style definierten Absaetzen wiederholt...
        if( !bStyleDef )
        {
            // falls kein .<PND>-Kode , alles ignorieren !!!
            if( pActNumRule )
            {
                BYTE nLevel;
                if( GetDeciByte( nLevel ) && !nError )
                {
                    if( MAXLEVEL < nLevel )
                        nActNumLevel = MAXLEVEL-1;
                    else
                        nActNumLevel = nLevel  -1;

                    SwTxtNode* pTxtNode = pCurPaM->GetNode()->GetTxtNode();
                    ASSERT( pTxtNode, "Kein Text-Node an PaM-Position" );

                    pTxtNode->SwCntntNode::SetAttr(
                        SwNumRuleItem( pActNumRule->GetName() ) );

                    pTxtNode->UpdateNum( SwNodeNum( nActNumLevel ) );

                    if( 44 == nDocType )
                        pTxtNode->SetNumLSpace( FALSE ); // TRUE ist Defaultwert

                    bWasPGNAfterPND = TRUE;
                    bIsNumListPara  = TRUE;
                }
            }
        }
    }
    /*
        Das wars: nun alles bis zum naechsten .<EPN> ueberspringen
    */
    BOOL bOldNoExec   = bNoExec;
    BOOL bOldTxtInDoc = bTxtInDoc;
    bNoExec   = TRUE;
    bTxtInDoc = FALSE;
    while(     ( !nError )
            && ( EOF != GetNextRecord() )
            && !(    pActW4WRecord
                  && (    pActW4WRecord->fnReadRec
                        == &SwW4WParser::Read_EndOfParaNum )))
    {
        /*NOP*/;
    }
    /*
        In Fussnoten wird ein evtl. unmittelbar
        nach der Nummerierung folgender TAB ignoriert
    */
    if( bFootnoteDef && !nError )
    {
        ULONG nOldPos = rInp.Tell();	// FilePos merken
        GetNextRecord();
        if(      pActW4WRecord
            && ( pActW4WRecord->fnReadRec != &SwW4WParser::Read_Tab ) )
        {
            rInp.Seek( nOldPos ); 	 	// FilePos restaurieren
        }
    }
    bNoExec   = bOldNoExec;
    bTxtInDoc = bOldTxtInDoc;
    nChrCnt   = 0;
    aCharBuffer[ nChrCnt ] = 0;
}


void SwW4WParser::Read_EndOfParaNum()	   		// (EPN)
{
    /*NOP*/;
}


void SwW4WParser::Read_ParaNumberDef() 				// (PND)
{
    if( bFootnoteDef || bStyleDef ) return;

    if(    ( 44 != nDocType ) 			// nur WinWord 2
        && ( 48 != nDocType ) ) return;	// und WordPerfekt 6-8 funktionieren

    BYTE nLevels = 0;

    // Lies die Anzahl der Nummerierungs-Stufen
    if(    ( !GetDeciByte( nLevels ) )
        || nError
        || !nLevels )
    {
        return;
    }

    if( MAXLEVEL < nLevels ) nLevels = MAXLEVEL;

    /*
        Pruefe, ob nach dem vorigen .<PND> ueberhaupt ein .<PGN> kam.
    */
    SwNumRule* pMyNumRule;
    if( pActNumRule && !bWasPGNAfterPND )
    {
        // alte Liste muss geloescht werden
        // ( geht nur, weil die Rule noch von niemand benutzt wird! )
        pDoc->DelNumRule( pActNumRule->GetName() );
        pActNumRule = 0;
    }

    // neue NumRule anlegen
    String aPrefix( String::CreateFromAscii(
                        RTL_CONSTASCII_STRINGPARAM( "W4WNum" )));
    USHORT nPos = pDoc->MakeNumRule( pDoc->GetUniqueNumRuleName( &aPrefix ) );
    pMyNumRule  = pDoc->GetNumRuleTbl()[ nPos ];

    /*
        Lies die Parameter fuer alle Nummerierungs-Stufen
        und setze sie in pMyNumRule ein
    */
    long nW4WStartNo;	   	// b
    String aPrefixTxt;		// c
    BYTE nW4WType;			// d
    long nNoWidth; 			// e
    String aSuffixTxt;		// f
    BYTE nW4WAttachPrev;	// g

    USHORT nStartNo;	// Start-Nr. fuer den Writer
    BYTE nUpperLevel;	// aktuelle Anzeigetiefe fuer den Writer
    SvxExtNumType eType;	   	// Writer-Num-Typ

    BOOL bError = TRUE;
    for( USHORT nActLevel = 0; nActLevel < nLevels; nActLevel++ )
    {
        bError = TRUE;

        // Parameter lesen
        if( ( !GetDecimal( nW4WStartNo     ) )  || nError ) break;
        if( ( !GetString( aPrefixTxt,
                W4WR_TXTERM, W4WR_TXTERM   ) )            ) break;
        if( ( !GetDeciByte( nW4WType       ) )  || nError ) break;
        if( ( !GetDecimal( nNoWidth        ) )  || nError ) break;
        if( ( !GetString( aSuffixTxt,
                W4WR_TXTERM, W4WR_TXTERM   ) )            ) break;
        if( ( !GetDeciByte( nW4WAttachPrev ) )  || nError ) break;

        // Parameter auswerten
        if( 0 > nW4WStartNo )
            nStartNo = 0;
        else
            nStartNo = (USHORT)nW4WStartNo;
        if( nW4WAttachPrev )
        {
            // Achtung: sind auch vorige Numm.-Zeichen anzuzeigen,
            // so darf das Prefix-Zeichen NICHT gezeigt werden,
            // da der Writer es sonst vor die GESAMTE Zeichenkette malt.
            aPrefixTxt = aEmptyStr;
            nUpperLevel++  ;
        }
        else
            nUpperLevel = 1;
        switch( nW4WType )
        {
        case 1: eType = SVX_NUM_NUMBER_NONE;        break;
        case 2: eType = SVX_NUM_CHARS_LOWER_LETTER; break;
        case 3: eType = SVX_NUM_CHARS_UPPER_LETTER; break;
        case 4: eType = SVX_NUM_ROMAN_LOWER;        break;
        case 5: eType = SVX_NUM_ROMAN_UPPER;        break;
        default:
            // pruefen auf Werte groesser 5
            ASSERT( !nW4WType, "Value of nW4WType is not supported" );
            // sonst default-Wert
            eType = SVX_NUM_ARABIC;
            break;
        }

        // aNumFmt erzeugen,
        SwNumFmt aNumFmt( pMyNumRule->Get( nActLevel ) );
        // konfigurieren
        aNumFmt.SetStart( nStartNo    );
        aNumFmt.SetPrefix(     aPrefixTxt  );
        aNumFmt.SetNumberingType(eType );
        aNumFmt.SetSuffix(    aSuffixTxt  );
        aNumFmt.SetIncludeUpperLevels( nUpperLevel );
        // und in die pMyNumRule aufnehmen
        pMyNumRule->Set( nActLevel, aNumFmt );

        bError = FALSE;
    }

    // Dumm: bei WinWord 2 wird vor jedem .<PGN> nochmals
    //       die komplette .<PND>-Geschichte wiederholt.
    // Wir duerfen dann natuerlich nicht jedesmal eine neue Liste
    // aufmachen, sondern muessen die alte Liste weiterfuehren.
    if( ( 44 == nDocType ) && pActNumRule && !bError )
    {
        BOOL bIdentical = TRUE;
        for( USHORT nLev = 0; nLev < nLevels; nLev++ )
        {
            if( pActNumRule->Get( nLev ) != pMyNumRule-> Get( nLev ) )
            {
                bIdentical = FALSE;
                break;
            }
        }
        if( bIdentical ) bError = TRUE;
    }

    if( bError )
    {
        // bearbeitete Liste loeschen
        // ( geht nur, weil die Rule noch von niemand benutzt wird! )
        pDoc->DelNumRule( pMyNumRule->GetName() );
    }
    else
    {
        // bearbeitete Liste erhalten und zur aktuellen Liste erklaeren
        pActNumRule = pMyNumRule;
        pActNumRule->SetInvalidRule( TRUE );
        bWasPGNAfterPND = FALSE;
    }
}




/***************************************************************************************

***************************************************************************************/



BOOL SwW4WParser::GetLRULHint( const SfxPoolItem*& rpH, RES_FRMATR eTyp )
{
    if ( bStyleDef )
    {
        rpH = &GetAktColl()->GetAttr( eTyp );
    }
    else
    {
        USHORT nIdx;
        SfxPoolItem* pH = pCtrlStck->GetFmtStkAttr( eTyp, &nIdx );

        if (pH && (*pCtrlStck)[nIdx]->nPtNode.GetIndex()+1
                    == pCurPaM->GetPoint()->nNode.GetIndex())
        {
            // LRUL-Space zu diesem Absatz bereits auf Stack
            //				-> modifizieren
            rpH = pH;
            return FALSE;				// new( rpLR ) nicht noetig
        }
        else
        {
            // LRUL-Space zu diesem Absatz nicht auf Stack
            //				-> neuen Copy-Konstruieren
            rpH = (SfxPoolItem*)pCtrlStck->GetFmtAttr( *pCurPaM, eTyp );
        }
    }
    return TRUE;                        // new( rpH ) noetig
}


BOOL SwW4WParser::GetULSpace( SvxULSpaceItem*& rpUL )
{
    const SfxPoolItem* pH = 0;
    BOOL bRet = GetLRULHint( pH, RES_UL_SPACE );
    if ( bRet ) rpUL = new SvxULSpaceItem( *((SvxULSpaceItem*)pH) );
    else rpUL = (SvxULSpaceItem*)pH;
    return bRet;                        // TRUE -> delete( rpUL ) noetig
}


void SwW4WParser::Read_SetSpaceBefore()         // (SBF)
{
    long nSpaceTw;

    if ( bStyleOnOff ){		// in Style An / Aus -> Ende des Attributes
        Read_HardAttrOff( RES_UL_SPACE );
        return;
    }
    if ( !bPageDefRdy ) return;

    if( GetDecimal( nSpaceTw ) && !nError ){
        SvxULSpaceItem* pUL = 0;
        BOOL bRet = GetULSpace( pUL );
        pUL->SetUpper( USHORT(nSpaceTw) );
        if ( bRet ){
            SetAttr( *pUL );
            DELETEZ( pUL );
        }
    }
}


void SwW4WParser::Read_SetSpaceAfter()			// (SAF)
{
    long nSpaceTw;

    if ( bStyleOnOff ){		// in Style An / Aus -> Ende des Attributes
        Read_HardAttrOff( RES_UL_SPACE );
        return;
    }
    if ( !bPageDefRdy ) return;

    if( GetDecimal( nSpaceTw ) && !nError ){
        SvxULSpaceItem* pUL = 0;
        BOOL bRet = GetULSpace( pUL );
        pUL->SetLower( USHORT(nSpaceTw) );
        if ( bRet ){
            SetAttr( *pUL );
            DELETEZ( pUL );
        }
    }
}

/***************************************************************************************
Absolute Positioned Objects und Graphiken
***************************************************************************************/


void lcl_SetFlyAttr( SfxItemSet& rSet,
                     SwHoriOrient eHAlign,
                     SwVertOrient eVAlign,
                     long nWidthTw, long nHeightTw, long nULHCx, long nULHCy,
                     BOOL bAuto = FALSE,
                     SwRelationOrient eHRel = PRTAREA,
                     SwRelationOrient eVRel = PRTAREA )
{
    if ( nHeightTw <= MINFLY ) bAuto = TRUE;

    // Groesse einstellen
    rSet.Put( SwFmtFrmSize( ( bAuto ) ? ATT_MIN_SIZE : ATT_FIX_SIZE,
                            nWidthTw, nHeightTw ));

    rSet.Put( SwFmtVertOrient( nULHCy, eVAlign, eVRel ) );
    rSet.Put( SwFmtHoriOrient( nULHCx, eHAlign, eHRel ) );
}



SwFmtFrmSize& lcl_AdjustFlySize( Size& rSize,
                        SwFmtFrmSize& rFlySize,
                        BOOL bSetGrfTwipWidth,
                        BOOL bSetGrfTwipHeight,
                        long nGrWidthTw,
                        long nGrHeightTw,
                        USHORT nHoriDelta  = 0,
                        USHORT nVertiDelta = 0 )
{
    if( !rSize.Width() )
        rSize = Size( 567*2, 567/2 );
    if( bSetGrfTwipWidth && bSetGrfTwipHeight )
    {
        // Breite *und* Hoehe anpassen...
        rFlySize.SetWidth( rSize.Width() +nHoriDelta );
        rFlySize.SetHeight(rSize.Height()+nVertiDelta);
    }
    else
    {
        // Breite anpassen und Proportionen erhalten...
        if( bSetGrfTwipWidth )
        {
            long nSzH = rSize.Height();
            rFlySize.SetWidth(
                rSize.Width() * nGrHeightTw
                                / (nSzH ? nSzH : nGrHeightTw)
                +nHoriDelta );
        }
        // Hoehe anpassen und Proportionen erhalten...
        else
        {
            long nSzW = rSize.Width();
            rFlySize.SetHeight(
                rSize.Height() * nGrWidthTw
                                    / (nSzW ? nSzW : nGrWidthTw)
                +nVertiDelta );
        }
    }
    return rFlySize;
}



void SwW4WParser::Read_Picture()				// (PCT)
{
    UpdatePercent( rInp.Tell(), nW4WFileSize );

    long nBadLen;

    bPic = TRUE;			// PCT-Komando angekommen
    if ( !bPicPossible ) return;
    if ( !pGraf)
        pGraf = new SwW4WGraf( rInp );

    // Wenn Type != 0  dann ueberlies falsche Laengenangabe
    if( ( nGrafPDSType == 1 )
        && ( !GetDecimal( nBadLen ) || nError ) )
        return;
    pGraf->Read( nGrafPDSType, nGrWidthTw, nGrHeightTw );
    pGraphic = pGraf->GetGraphic();
}



SwFlyFrmFmt* SwW4WParser::MakeTxtFly( RndStdIds eAnchor,
                                    const SfxItemSet& rSet )
{
    SwFlyFrmFmt* pFlyFmt = pDoc->MakeFlySection( eAnchor,
                                                pCurPaM->GetPoint(), &rSet );

    // der Anker ist der SPoint vom Pam. Dieser wird beim Einfugen
    // von Text usw. veraendert, darum speicher ihn auf dem Attribut
    // Stack. Das Attribut muss vom Format erfragt werden,
    // damit das Format auch spaeter bekannt ist !!!
    if( FLY_IN_CNTNT != eAnchor )
        pCtrlStck->NewAttr( *pCurPaM->GetPoint(), SwW4WAnchor( pFlyFmt ) );
    return pFlyFmt;
}

void SwW4WParser::FlySecur( BOOL bAlignCol,
                            long& rXPos, long& rYPos,
                            long& rWidthTw, long& rHeightTw,
                            RndStdIds& rAnchor,
                            long* pTop, long* pLeft, long* pBot,
                            long* pRight, USHORT nBorderCode )
{
    if ( rYPos < 0 )
        rYPos = 0;
    if ( rXPos < 0 )
    {
        switch ( rAnchor )
        {
        case FLY_IN_CNTNT:	rAnchor = FLY_AT_CNTNT;	break;
        case FLY_PAGE:		rXPos = 0;				break;
        }
    }

    if ( nBorderCode )	// mit Umrandung
    {
        if( rWidthTw < MINFLY + 2 * MAX_BORDER_SIZE )
        {
            if( !rWidthTw )				// 0 == volle Breite (z.B. WW2)
                rWidthTw = (long)(nPgRight - rXPos) - 2 * MAX_BORDER_SIZE;
            else
                rWidthTw = MINFLY + 2 * MAX_BORDER_SIZE;
        }

        if ( rHeightTw < MINFLY + MAX_BORDER_SIZE )
            rHeightTw = MINFLY + MAX_BORDER_SIZE;
    }
    else				// ohne Umrandung
    {
        if ( rWidthTw < MINFLY + MAX_EMPTY_BORDER )
        {
            if( !rWidthTw )				// 0 == volle Breite (z.B. WW2)
                rWidthTw = (long)(nPgRight - rXPos) - 2 * MAX_EMPTY_BORDER;
            else
                rWidthTw = MINFLY + 2 * MAX_EMPTY_BORDER;
        }
        if ( rHeightTw < MINFLY + MAX_EMPTY_BORDER )
            rHeightTw = MINFLY + MAX_EMPTY_BORDER;
    }
}


void SwW4WParser::Read_IncludeGraphic()			// (IGR)
{
    String aPath;
    BYTE   nType;

    long nWidthTw  = 0;
    long nHeightTw = 0;

    if(    GetString(  aPath, W4WR_TXTERM, W4WR_TXTERM ) && !nError
        && GetChar(    nType       )                     && !nError )
    {
        // versuchen, ebenfalls die Breite und Hoehe zu lesen
        BOOL bSetGrfTwipWidth
            = ( ( W4WR_TXTERM != GetDecimal( nWidthTw  ) ) || !nError );

        BOOL bSetGrfTwipHeight = FALSE;

        if( !bSetGrfTwipWidth )
        {
            bSetGrfTwipHeight
                = ( ( W4WR_TXTERM != GetDecimal( nHeightTw  ) ) || !nError );
        }
        bSetGrfTwipWidth  |= ( !nWidthTw  );
        bSetGrfTwipHeight |= ( !nHeightTw );

        // wichtig: eins zurueck gehen
        rInp.SeekRel( - 1 );

        // ggfs fuehrende und anhaengende Blanks entfernen
        aPath.EraseAllChars();
        // ggfs. Anfuehrungszeichen entfernen
        if( '"' == aPath.GetChar(      0      ) )
            aPath.Erase( 0, 1 );
        if( '"' == aPath.GetChar(aPath.Len()-1) )
            aPath.Erase( aPath.Len()-1 );

        // Stil der uebergebenen Zeichenkette beruecksichtigen
        INetURLObject::FSysStyle eParser = INetURLObject::FSYS_DETECT;
        switch( nType )
        {
        case 1: eParser = INetURLObject::FSYS_DOS; break;
        case 2: eParser = INetURLObject::FSYS_MAC;   break;
        case 3: eParser = INetURLObject::FSYS_UNX;   break;
        }
        INetURLObject aEntry( aPath, eParser );
        aPath = ::binfilter::StaticBaseUrl::RelToAbs( aEntry.GetFull() );

        SfxItemSet aFlySet( pDoc->GetAttrPool(),
                            RES_FRMATR_BEGIN, RES_FRMATR_END-1);

        // Art der Bindung einstellen
        SwFmtAnchor aAnchor( FLY_IN_CNTNT );	// als Buchstaben
        aAnchor.SetAnchor( pCurPaM->GetPoint() );
        aFlySet.Put( aAnchor );

        // Groesse einstellen
        if( bSetGrfTwipWidth  ) nWidthTw  = 567*2;
        if( bSetGrfTwipHeight ) nHeightTw = 567/2;
        aFlySet.Put( SwFmtFrmSize( ATT_FIX_SIZE, nWidthTw, nHeightTw ) );

        // Link auf die Grafik einsetzen
        SwFlyFrmFmt* pFlyFmt
             = pDoc->Insert( *pCurPaM,
                        aPath, aEmptyStr,		// Name der Grafik !!
                        0,                      // Zeiger auf die Grafik
                        &aFlySet,             	// Attr. fuer FlyFrm
                        0 );			        // Attr. fuer die Grafik

        // ggfs Laenge und/oder Breite korrigieren
        if( bSetGrfTwipWidth || bSetGrfTwipHeight )
        {
            SwGrfNode *pGrfNd
                = pDoc->GetNodes()[ pFlyFmt->GetCntnt().
                                        GetCntntIdx()->GetIndex()+1 ]
                            ->GetGrfNode();
            if( pGrfNd->IsGrafikArrived() )
            {
                Size aSize( pGrfNd->GetTwipSize() );
                SwFmtFrmSize aFlySize( pFlyFmt->GetFrmSize() );
                pFlyFmt->SetAttr(
                    lcl_AdjustFlySize( aSize, aFlySize,
                                        bSetGrfTwipWidth, bSetGrfTwipHeight,
                                        nWidthTw, nHeightTw ) );
            }
            else
                pGrfNd->SetChgTwipSize( TRUE );
        }
    }
}

void SwW4WParser::Read_PictureDef()				// (PDS)
{
    long nType, nFixFloat, nXPos, nYPos, nOptWidthOrig = 0, nOptHeightOrig = 0;

    long nOldW = nGrWidthTw;
    long nOldH = nGrHeightTw;

    if( GetDecimal( nType ) && !nError && GetDecimal( nFixFloat ) && !nError
        && GetDecimal( nXPos ) && !nError && GetDecimal( nYPos ) && !nError
        && GetDecimal( nGrWidthTw ) && !nError && GetDecimal( nGrHeightTw )
        && !nError )
    {
        if( W4WR_TXTERM != GetDecimal( nOptWidthOrig ) || nError	// Fehlen Opt. Parameter ?
            || W4WR_TXTERM != GetDecimal( nOptHeightOrig ) || nError)
        {}// do nothing
        nGrafPDSType = nType;

        if ( bPicPossible )		// APO mit genaueren Angaben bereits vorhanden
            return;

        BOOL bOldPoss = bPicPossible; bPicPossible = TRUE;
        BOOL bOldTxtInDoc = bTxtInDoc; bTxtInDoc = FALSE;
        bPic = FALSE;

        while (!nError && !bPic					    // lies Grafik ein
                && (EOF != GetNextRecord()))		// bis PCT //$ EOF
            ;

        bPicPossible = bOldPoss;
        bPic = FALSE;

        RndStdIds eAnchor = ( nFixFloat ) ? FLY_PAGE : FLY_IN_CNTNT;

        if ( ( nIniFlags & W4WFL_NO_GRAF_IN_CNTNT )
             && ( eAnchor == FLY_IN_CNTNT ) )
            eAnchor = FLY_AT_CNTNT;  //!!! Workaround fuer u.a. Panne


        FlySecur( FALSE, nXPos, nYPos, nGrWidthTw, nGrHeightTw, eAnchor );

        SfxItemSet aFlySet( pDoc->GetAttrPool(),
                            RES_FRMATR_BEGIN, RES_FRMATR_END-1);
        if( !bNew )
            Reader::ResetFrmFmtAttrs( aFlySet );
        SwFmtAnchor aAnchor( eAnchor );
        aAnchor.SetAnchor( pCurPaM->GetPoint() );
        aFlySet.Put( aAnchor );
        lcl_SetFlyAttr( aFlySet, HORI_LEFT, VERT_TOP,
                        nGrWidthTw, nGrHeightTw, nXPos, nYPos, FALSE,
                        PRTAREA,
                        FRAME );

        if( pGraphic )								// Grafik korrekt eingelesen
            pDoc->Insert( *pCurPaM,
                            aEmptyStr, aEmptyStr,   // Name der Grafik !!
                            pGraphic,
                            &aFlySet,          		// Attribute fuer den FlyFrm
                            0 );		          	// Attribute fuer die Grafik

        else		// Keine Grafik -> Text-FlyFrame
            MakeTxtFly( eAnchor, aFlySet );

        if ( pGraf ) DELETEZ( pGraf );

        if ( nOldW < nGrWidthTw ) nOldW = nGrWidthTw;	// umgebenden Rahmen evtl.
        if ( nOldH < nGrHeightTw ) nOldH = nGrHeightTw;	// vergroessern bis er
                                                    // passt
        bTxtInDoc = bOldTxtInDoc;
    }
    nGrWidthTw = nOldW;
    nGrHeightTw = nOldH;
}


void SwW4WParser::Read_BeginAbsPosObj()				// (APO)
{
    long nAnchor, nVAlign, nHAlign, nAlignType, nWrap,
         nULHCx, nULHCy, nTop, nLeft, nBot, nRight,
         nOptShade = 0, nOptVPos, nOptPathFormat = 1;
    BYTE nType;

    if ( bStyleDef || bStyleOnOff ) return;

    long nOldW = nGrWidthTw;
    long nOldH = nGrHeightTw;
    BOOL bOldTxtInDoc = bTxtInDoc;
    bTxtInDoc = FALSE;

    if(    GetHexByte( nType )      && !nError						// a
        && GetDecimal( nAnchor )    && !nError                      // b
        && GetDecimal( nVAlign )    && !nError                      // c
        && GetDecimal( nHAlign )    && !nError                      // d
        && GetDecimal( nAlignType ) && !nError                      // e
        && GetDecimal( nWrap )      && !nError                      // f
        && GetDecimal( nGrWidthTw ) && !nError                      // g
        && GetDecimal( nGrHeightTw) && !nError                      // h
        && GetDecimal( nULHCx )     && !nError                      // i
        && GetDecimal( nULHCy )     && !nError                      // j
        && GetDecimal( nTop )       && !nError                      // k
        && GetDecimal( nLeft )      && !nError                      // l
        && GetDecimal( nBot )       && !nError                      // m
        && GetDecimal( nRight )     && !nError                      // n
        && GetHexUShort( nApoBorderCode ) && !nError )              // o
    {
        if (    W4WR_TXTERM == GetDecimal( nOptShade ) && !nError   // p
             && W4WR_TXTERM == GetDecimal( nOptVPos )  && !nError   // q
             && W4WR_TXTERM == GetNextRecord()         && !nError   // r
             && W4WR_TXTERM != GetDecimal( nOptPathFormat ) )       // s
        {}// do nothing


        // '\0x00' hinter Dateinamen setzen
        Flush();

        // eventuell noch folgende Parameter ueberlesen
        rInp.SeekRel( - 1 );
        SkipEndRecord();


        BOOL bSetGrfTwipWidth  = !nGrWidthTw;
        BOOL bSetGrfTwipHeight = !nGrHeightTw;

        static const RndStdIds nAnchorTab[]={ FLY_AT_CNTNT,		// Absatz
                                              FLY_PAGE, 	 	// Seite
                                              FLY_IN_CNTNT }; 	// Buchstabe
        if ( nAnchor >= sizeof(nAnchorTab)/sizeof(RndStdIds)) nAnchor = 0;

        RndStdIds eAnchor = nAnchorTab[ nAnchor ];

        if ( ( nIniFlags & W4WFL_NO_FLY_IN_CNTNT )
             && ( eAnchor == FLY_IN_CNTNT ) )
            eAnchor = FLY_AT_CNTNT;  // Workaround fuer u.a. Panne

        // anchor "at page" in header / footer must change to "at Content"
        if( FLY_PAGE == eAnchor &&
            pDoc->IsInHeaderFooter( pCurPaM->GetPoint()->nNode ))
        {
            eAnchor = FLY_AT_CNTNT;
            nULHCy = 0;
        }

        FlySecur( (1 == nAlignType), nULHCx, nULHCy, nGrWidthTw, nGrHeightTw, eAnchor,
                  &nTop, &nLeft, &nBot, &nRight, nApoBorderCode );

        nGrafPDSType = 0;

        static const SwVertOrient nVAlignTab[]={ VERT_NONE, VERT_TOP,
                                                VERT_CENTER, VERT_BOTTOM,
                                                VERT_NONE };
        if ( nVAlign >= sizeof(nVAlignTab)/sizeof(SwVertOrient)) nVAlign = 0;
        SwVertOrient eVAlign = nVAlignTab[ nVAlign ];

        static const SwHoriOrient nHAlignTab[]={ HORI_LEFT, HORI_RIGHT,
                                                  HORI_CENTER, HORI_NONE};
        if ( nHAlign >= sizeof(nHAlignTab)/sizeof(SwHoriOrient))
            nHAlign = 3;
        SwHoriOrient eHAlign = nHAlignTab[ nHAlign ];

        SwRelationOrient eHRel = ( 2 == nAlignType ) ? FRAME : PRTAREA;
        SwRelationOrient eVRel = eHRel;

        if( ( eHRel == FRAME ) && ( eAnchor == FLY_AT_CNTNT ) )
            nULHCx -= nPgLeft;

        if(     (    ( nULHCx >  50 )
                  || ( nULHCx < -50 )  )
            &&  ( eHAlign == HORI_LEFT ) )
        {
            eHAlign = HORI_NONE;		// linksbuendig mit Offset
        }

        if(     (    ( nULHCy >  50 )
                  || ( nULHCy < -50 ) )
            &&  ( eVAlign == VERT_TOP ) )
        {
            eVAlign = VERT_NONE;		// obenbuendig mit Offset
        }

        if( HORI_NONE == eHAlign )
            eHRel = FRAME;

        if( VERT_NONE == eVAlign )
            eVRel = FRAME;

        SfxItemSet aFlySet( pDoc->GetAttrPool(),
                            RES_FRMATR_BEGIN, RES_FRMATR_END-1);
        lcl_SetFlyAttr( aFlySet, eHAlign, eVAlign, nGrWidthTw, nGrHeightTw,
                        nULHCx, nULHCy,
                        ( bSetGrfTwipWidth || bSetGrfTwipHeight ),
                        eHRel,
                        eVRel );

        aFlySet.Put( SwFmtSurround( (nWrap) ? SURROUND_PARALLEL
                                            : SURROUND_NONE ) );

        USHORT nHoriDelta  = 0;
        USHORT nVertiDelta = 0;
        {
            SvxBoxItem aBox;

            // Raender nach *innen*
            aBox.SetDistance( (USHORT)nLeft, BOX_LINE_LEFT	);
            aBox.SetDistance( (USHORT)nTop,  BOX_LINE_TOP	);
            aBox.SetDistance( (USHORT)nRight, BOX_LINE_RIGHT	);
            aBox.SetDistance( (USHORT)nBot,   BOX_LINE_BOTTOM	);

            USHORT nRet = Read_SetBorder( nApoBorderCode, aBox );
            if( nRet )
            {
                // evtl Umrandung setzen
                if( 0 != ( 0x1 & nRet ))
                    aFlySet.Put( aBox );
                //	evtl Hintergrund setzen
                if( 0 != ( 0x2 & nRet ))
                {
                    const Color aCol( COL_LIGHTGRAY );
                    aFlySet.Put( SvxBrushItem( aCol, RES_BACKGROUND ));
                }

                // ggfs. Schatten setzen
                if( nOptShade )
                {	// fShadow
                    SvxShadowItem aS;
                    aS.SetColor( Color( COL_GRAY ));
                    aS.SetWidth( (USHORT)nOptShade*3 );
                    aS.SetLocation( SVX_SHADOW_BOTTOMRIGHT );
                    aFlySet.Put( aS );
                }

                /*
                    horizontale Randbreite errechnen
                */
                nHoriDelta += ( 2 * aBox.GetDistance() );
                const SvxBorderLine* pBoxLn = aBox.GetLeft();
                if( pBoxLn )
                {
                    nHoriDelta  += pBoxLn->GetOutWidth()
                                +  pBoxLn->GetInWidth()
                                +  pBoxLn->GetDistance();
                }
                pBoxLn = aBox.GetRight();
                if( pBoxLn )
                {
                    nHoriDelta  += pBoxLn->GetOutWidth()
                                +  pBoxLn->GetInWidth()
                                +  pBoxLn->GetDistance();
                }
                /*
                    vertikale Randbreite errechnen
                */
                nVertiDelta += ( 2 * aBox.GetDistance() );
                pBoxLn = aBox.GetTop();
                if( pBoxLn )
                {
                    nVertiDelta += pBoxLn->GetOutWidth()
                                +  pBoxLn->GetInWidth()
                                +  pBoxLn->GetDistance();
                }
                nVertiDelta += aBox.GetDistance();
                pBoxLn = aBox.GetBottom();
                if( pBoxLn )
                {
                    nVertiDelta += pBoxLn->GetOutWidth()
                                +  pBoxLn->GetInWidth()
                                +  pBoxLn->GetDistance();
                }
            }
        }

        SwFlyFrmFmt* pFlyFmt;
        if ( nType == 2 )
        {									// Graphik verknuepfen/einlesen
            // Raender nach *innen*
            if ( nTop || nBot )
            {
                SvxULSpaceItem aUL;
                aUL.SetUpper( USHORT(nTop) );
                aUL.SetLower( USHORT(nBot) );
                aFlySet.Put( aUL );
            }
            if ( nLeft || nRight )
            {
                SvxLRSpaceItem aLR;
                aLR.SetTxtLeft( USHORT(nLeft) );
                aLR.SetRight( USHORT(nRight) );
                aFlySet.Put( aLR );
            }

            BOOL bOldPoss = bPicPossible;
            bPicPossible  = TRUE;

            String aGrfFileName( aCharBuffer );
            aGrfFileName.EraseLeadingChars();

            /*
                Wir lesen den *Inhalt* des Bildes nur dann,
                wenn wir keinen Namen angegeben bekamen.
                Haben wir einen Namen, so setzen wir
                statt dem Bild einen Link ein.
            */
            BOOL bOldNix          = bNixNoughtNothingExec;
            bNixNoughtNothingExec = ( 0 < aGrfFileName.Len() );
            pGraphic = 0;

            while(     ( !nError )
                    && ( EOF != GetNextRecord() )
                    && !(   pActW4WRecord
                          && (    pActW4WRecord->fnReadRec
                               == &SwW4WParser::Read_EndAbsPosObj )))
            {
                /*NOP*/;
            }

            bNixNoughtNothingExec = bOldNix;
            bPicPossible          = bOldPoss;
            bTxtInDoc             = bOldTxtInDoc;

            if( pGraphic || aGrfFileName.Len() ){

                if( pGraphic && ( bSetGrfTwipWidth || bSetGrfTwipHeight ) )
                {
                    Size aSize( GetGraphicSizeTwip( *pGraphic, 0 ));
                    SwFmtFrmSize aFlySize( (SwFmtFrmSize&)aFlySet.Get( RES_FRM_SIZE ));
                    aFlySet.Put(
                        lcl_AdjustFlySize( aSize, aFlySize,
                                            bSetGrfTwipWidth, bSetGrfTwipHeight,
                                            nGrWidthTw, nGrHeightTw,
                                            nHoriDelta, nVertiDelta ) );
                }

                SwFmtAnchor aAnchor( eAnchor );
                aAnchor.SetAnchor( pCurPaM->GetPoint() );
                aFlySet.Put( aAnchor );

                pFlyFmt = pDoc->Insert( *pCurPaM,
                                aGrfFileName, aEmptyStr,// Name der Grafik !!
                                pGraphic,
                                &aFlySet,             	// Attr. fuer FlyFrm
                                0 );			        // Attr. fuer die Grafik
                if( !pGraphic && ( bSetGrfTwipWidth || bSetGrfTwipHeight ) )
                {
                    SwGrfNode *pGrfNd
                        = pDoc->GetNodes()[ pFlyFmt->GetCntnt().
                                                GetCntntIdx()->GetIndex()+1 ]
                                    ->GetGrfNode();

                    if( pGrfNd->IsGrafikArrived() )
                    {
                        Size aSize( pGrfNd->GetTwipSize() );
                        SwFmtFrmSize aFlySize( pFlyFmt->GetFrmSize() );
                        pFlyFmt->SetAttr(
                            lcl_AdjustFlySize( aSize, aFlySize,
                                                bSetGrfTwipWidth,
                                                bSetGrfTwipHeight,
                                                nGrWidthTw, nGrHeightTw,
                                                nHoriDelta, nVertiDelta ) );
                    }
                    else
                        pGrfNd->SetChgTwipSize( TRUE );
                }

            }
            else
                MakeTxtFly( eAnchor, aFlySet );

            if ( pGraf ) DELETEZ( pGraf );

        }
        else
        {								// Text-FlyFrame oder Formel oder leer

            bTxtInDoc = bOldTxtInDoc;
            pFlyFmt = MakeTxtFly( eAnchor, aFlySet );

            Flush();

            SwPosition aTmpPos( *pCurPaM->GetPoint() );

            const SwFmtCntnt& rCntnt = pFlyFmt->GetCntnt();
            ASSERT( rCntnt.GetCntntIdx(), "Kein Inhalt vorbereitet." );
            PtNd( pCurPaM ) = rCntnt.GetCntntIdx()->GetIndex() + 1;
            SwCntntNode *pNode = pCurPaM->GetCntntNode();
            PtCnt( pCurPaM ).Assign( pNode, 0 );

            USHORT nOldStyleId = nAktStyleId;
            W4WCtrlStack* pOldStack = pCtrlStck;
            pCtrlStck = new W4WCtrlStack( *pOldStack, *pCurPaM->GetPoint() );

            SetAttr( SwW4WStyle( nAktStyleId ));    // Override implizites
                                                    // Style in Frames
            pActFlySection = pFlyFmt;
            while(     ( !nError )
                    && ( EOF != GetNextRecord() )
                    && !(   pActW4WRecord
                          && (    pActW4WRecord->fnReadRec
                               == &SwW4WParser::Read_EndAbsPosObj )))
            {
                /*NOP*/;
            }
            pActFlySection = 0;

#ifdef MOGEL_WW2
            if ( !( nIniFlags & W4WFL_NO_APO_HNL_REMOVE )
                 && nDocType == 44				// Bit 4 : HNL nicht klauen
                 && !bIsTxtInPara )				// bevor die Attribute
                JoinNode( pCurPaM );			// gesetzt sind
#endif

            DeleteZStk( pCtrlStck );
            pCtrlStck   = pOldStack;
            nAktStyleId = nOldStyleId;
            pCtrlStck->SetEndForClosedEntries( aTmpPos );
            *pCurPaM->GetPoint() = aTmpPos;
        }
    }
    bTxtInDoc = bOldTxtInDoc;
    nGrWidthTw = nOldW;
    nGrHeightTw = nOldH;
    nApoBorderCode = 0;
}


void SwW4WParser::Read_EndAbsPosObj()				// (APF)
{
}


/***************************************************************************************
Umrandungen
***************************************************************************************/

// Setze Umrandung, nach SetPamInCell() aufrufen
// Ret: Bit 0: Umrandung setzen
//		Bit 1: Schatten setzen

USHORT SwW4WParser::Read_SetBorder( USHORT nBor, SvxBoxItem& rFmtBox )
{
// W4W nuetzt ihren ohnehin schon mageren Bereich der Breiten nicht aus,
// zumindest bei WP und WW2 nicht, d.h. "extra thick" und "hairline"
// wird nie benutzt. Ich habe die Tabelle jetzt auf WW2 optimiert.

    static USHORT __READONLY_DATA nOutTab[] = { 		// Aussenlinie :
        DEF_LINE_WIDTH_0, DEF_LINE_WIDTH_1,				// none, single
        DEF_DOUBLE_LINE1_OUT, DEF_LINE_WIDTH_0,			// double, dashed
        DEF_LINE_WIDTH_0, DEF_LINE_WIDTH_3,				// dotted, sick
        DEF_LINE_WIDTH_4, DEF_LINE_WIDTH_0 };			// extra thick, hairline

    static USHORT __READONLY_DATA nInTab[] = {			// Innenlinie,
        0, 0, DEF_DOUBLE_LINE1_IN, 0, 0, 0, 0, 0 };		// Index siehe nOutTab

    static USHORT __READONLY_DATA nDistTab[] = { 		// Abstand der Linien
        0, 0, DEF_DOUBLE_LINE1_DIST, 0, 0, 0, 0, 0 };   // Index siehe nOutTab

    static USHORT __READONLY_DATA nLinePlace[] = { 		// Art der Linien
                        BOX_LINE_RIGHT,
                        BOX_LINE_BOTTOM,
                        BOX_LINE_LEFT,
                        BOX_LINE_TOP };

    if ( nBor == 0 )
        return 0;						// nix zu tun

    USHORT nRet = 0;
    USHORT i;
    Color aCol( COL_BLACK );
    SvxBorderLine aBrd( &aCol );

    for ( i = 0; i < 4; i++ ){
        BYTE nW4WTyp = (BYTE)(nBor & 0xf);
        // ist diese Linie vorhanden ?
        if( nW4WTyp  )
        {

            nRet |= 1;           // mindestens 1 Linie ist vorhanden
            if ( nW4WTyp & 0x8 ) // oder == 8 ??
                nRet |= 2;		 // Shadow
            if( nW4WTyp > 7 )	 // unbekannt
                nW4WTyp = 7;	 // ->hairline

            aBrd.SetOutWidth( nOutTab[nW4WTyp] );
            aBrd.SetInWidth( nInTab[nW4WTyp] );
            aBrd.SetDistance( nDistTab[nW4WTyp] );
            rFmtBox.SetLine( &aBrd, nLinePlace[ i ] );
        }
        nBor >>= 4;
    }
    return nRet;
}

void SwW4WParser::Read_ParaBorder()				// (PBC)
{
    USHORT nBorder;

    if ( bStyleOnOff )
    {
        // in Style An / Aus -> Ende des Attributes
        Read_HardAttrOff( RES_BOX );
        return;
    }
    if ( GetHexUShort( nBorder ) && !nError )
    {
        if ( nBorder && nBorder == nApoBorderCode
             && !( nIniFlags & W4WFL_NO_DBL_APO_BORDER_REMOVE ) )
            return;			// Vermeide doppelte Umrandungen um APOs

        SvxBoxItem aBox;
        USHORT nRet = Read_SetBorder( nBorder, aBox );

        // einschalten ?
        if( nRet )
        {
            // Umrandung setzen ?
            if( 0x1 & nRet )
                SetAttr( aBox );

            //	Hintergrund setzen ?
            if( 0x2 & nRet )
            {
                const Color aCol( COL_LIGHTGRAY );
                const SvxBrushItem aBack( aCol, RES_BACKGROUND );
                SetAttr( aBack );
            }
        }
        else
        // ausschalten
        {
            // Umpopele W4W-Fehler:
            // Ausschalten erfolgt oft erst am Anfang des folgenden Absatzes
            // dies geschieht in normalen UND in Tabellen-Absaetzen...
            if( bIsTxtInDoc && !bIsTxtInPara )
            {
                // gehe 1 Char zurueck
                BOOL bForward = pCurPaM->Move( fnMoveBackward, fnGoCntnt );

                pCtrlStck->SetAttr( *pCurPaM->GetPoint(), RES_BOX, TRUE, bForward );

                if( bForward )
                    pCurPaM->Move( fnMoveForward, fnGoCntnt );
            }
            else
            {
                pCtrlStck->SetAttr( *pCurPaM->GetPoint(), RES_BOX );
            }
        }
    }
}


/***************************************************************************************
Tabellen und mehrspaltiges
***************************************************************************************/

#define TAB_KULANZ 72			// so viele Twips Kulanz ( 1/20 " )
#define TAB_KULANZ2 350			// so viele Twips Kulanz, wenn Bit2=0



void SwW4WParser::Adjust_pTabDefs()	// Aufrufe siehe .<CDS> und .<BRO>
{
    int i;

    // Spalten mit Breite kleinergleich NULL werden auf Kosten der
    // vorangegangenen Spalte(n) auf 144 Twips verbreitert.
    // Tabelle muss hier von rechts nach links durchlaufen werden.
    for(i=nTabCols-1; i>0; i--)
    {
        if( pTabDefs[ i ].nRightTw - pTabDefs[ i ].nLeftTw < 1 )
        {
            pTabDefs[ i ].nLeftTw = pTabDefs[ i ].nRightTw-144;
            if( pTabDefs[ i-1 ].nRightTw
                > pTabDefs[ i ].nLeftTw - nTabDeltaSpace )
            {
                pTabDefs[ i-1 ].nRightTw =
                    pTabDefs[ i ].nLeftTw - nTabDeltaSpace;
            }
        }
    }

    // And now what we all like so much: File Format depending hacks ;-)
    switch( nDocType )
    {
    case 33:
        {   // AmiPro:
            // Alle Parameter ab 2. Spalte korrigieren.
            // Linker Zwischenraum muss der Spaltenbreite hinzugerechnet
            // werden, dadurch verschieben sich alle weiteren Raender.
            // Beruecksichtige hierbei aber auch die Distanz zwischen
            // dem Inhalt und dem Rand einer jeden Zelle.
            // Regel: Versuche, die gesamte TABELLEN-Breite moeglichst
            //        nicht groesser als im Original werden zu lassen !
            long nMinPlus = 2 * DEFAULT_TAB_CELL_DISTANCE;

            // Die 1. Spalte wird um genau 2 Distanzen verbreitert
            long nDelta = nMinPlus;
            pTabDefs[0].nRightTw += nDelta;

            // Da Verbreiterung speziell der 1. Spalte ist zwar wichtig,
            // sollte aber moeglichst nicht die Gesamtbreite der
            // Tabelle erhoehen, daher VERSUCHEN wir, die zusaetzliche
            // Breite bei den folgenden Spalten wieder abzuknapsen...
            long nToWide = nDelta;

            for (i=1; i<(int)nTabCols; i++)
            {
                pTabDefs[i].nLeftTw += nDelta;

                long nAktPlus = (   pTabDefs[ i   ].nLeftTw
                                  - pTabDefs[ i-1 ].nRightTw );

                // alle uebrigen Spalten werden um ihren Zwischenraum
                // mindestens jedoch um 2 Distanzen verbreitert
                if( nAktPlus > nMinPlus )
                {
                    if( nToWide ) // noch Korrektur wg. 1. Spalte noetig?
                    {
                        if( nAktPlus - nToWide >= nMinPlus )
                        {
                            nAktPlus -= nToWide;
                            nToWide   = 0;
                        }
                        else
                        {
                            nToWide  -= (nAktPlus - nMinPlus);
                            nAktPlus  = nMinPlus;
                        }
                    }
                }
                else
                {
                    // wir verbreitern die Spalte um MEHR als ihren
                    // Zwischenraum, merken uns diese Tat jedoch,
                    // indem wir nToWide entsprechend vergroessern,
                    // damit Wiedergutmachung bei der Bearbeitung
                    // der Folgespalten erfolgen kann...
                    nToWide += (nMinPlus - nAktPlus);
                    nAktPlus = nMinPlus;
                }
                nDelta += nAktPlus;
                pTabDefs[i].nRightTw += nDelta;
            }
            // linke Raender nach links ziehen: Luecken verschwinden
            for (i=1; i<(int)nTabCols; i++)
            {
                pTabDefs[i].nLeftTw = pTabDefs[i-1].nRightTw;
            }
        }
        break;
    case 44:
        {	// WW2:
            // Die rechten Raender aller Spalten korrigieren.
            // Rechter Zwischenraum muss der Spaltenbreite hinzugerechnet
            // werden. An letzte Spalte muss dementsprechend der
            // durchschnittliche Zwischenraum angehaengt werden.
            for(i=0; i<(int)nTabCols-1; i++)
            {
                pTabDefs[i].nRightTw = pTabDefs[i+1].nLeftTw;
            }
            pTabDefs[ nTabCols-1 ].nRightTw += nTabDeltaSpace;
        }
        break;
    case 07:
    case 48:
        {	// WP:
            // Breite der Tabelle ist RRand der letzten Zelle - LRand der
            // ersten Zelle, d.h. der Text-Text-Abstand zaehlt nicht hinzu.
            // Der Text-Text-Abstand ist hier immer 144 tw, egal was in WP
            // eingestellt wird.
            // Jede Zelle hat die Breite RRand(i)-LRand(i)+Abstand*(n-1)/n
            // zur Ergaenzung:
            // W4W meldet jetzt sogar Twips(!)-Werte, die hoehere Genauigkeit
            // suggerieren. Leider sind diese Unsinn: in den Feldern stehen
            // bloss die alten Werte mit 144 multipliziert. (khz, 9.3.1998)
            long nKorr = 144;
            if (nTabCols > 1)
                nKorr = (pTabDefs[1].nLeftTw - pTabDefs[0].nRightTw)
                        / nTabCols;
            long nKorr1 = 0;
            for(i=1; i<(int)nTabCols; i++)
            {
                nKorr1 += nKorr;
                pTabDefs[i].nRightTw -= nKorr1;
                pTabDefs[i].nLeftTw  -= nKorr1;
            }
        }
        break;
    }


    nTabWidthTw = pTabDefs[nTabCols-1].nRightTw-pTabDefs[0].nLeftTw;

    // Falls nach unseren obigen Korrektur-Aktionen
    // noch immer Spaltenzwischenraeume existieren,
    // werden diese nun ausgewertet, damit die Tabellenbreite
    // entsprechend vergroessert werden kann...
    if( ( 33 != nDocType ) && ( 44 != nDocType ) )
    {
        long nAbstand = 0;
        if(nTabCols > 1)
        {
            // Zaehle alle Abstaende zwischen den Zellen zusammen
            // ACHTUNG: iteriere ueber nTabCols, nicht ueber nCols!!
            for (i=1; i<nTabCols; i++)
                nAbstand += (  pTabDefs[ i   ].nLeftTw
                                - pTabDefs[ i-1 ].nRightTw );
            // Nimm den durchschnittl. Zwischenraum aller Zellen
            nAbstand = nAbstand / ( nTabCols-1 );
        }
        else
            nAbstand = (nDocType == 07) ? 0  : 140;	// Defaults
        nTabWidthTw += nAbstand;
    }

    // hilfsweise naechste Zelle extrapoliert
    pTabDefs[nTabCols].nLeftTw = pTabDefs[0].nLeftTw + nTabWidthTw;

    long nWidthTw;		// Groesse der aktuellen Zelle
    long nMittel;		// zur Erkennung, ob alle Zellen gleich gross

    for (i=0; i<(int)nTabCols; i++)
    {
        nWidthTw = pTabDefs[i+1].nLeftTw - pTabDefs[i].nLeftTw;

        if( 0 == i )
            nMittel = nWidthTw;
        pTabDefs[ i ].nWidthFrac = nWidthTw;
        pTabDefs[ i ].nRemainingMergeRows = 0;

        if ( Abs( nMittel - nWidthTw ) > TAB_KULANZ )
            bTabOwnFrm = TRUE;  // Breite der Zellen unterscheidet sich
                                // deutlich
                                // -> alle Zellen erhalten eigene
                                // Frame-Formate
    }

    if( !bTabOwnFrm )// alle Zellen gleich gross ?
    {				 // -> gemeinsames Frameformat mit
                        //  Zellenbreite = Tabellenbreite / Zellenzahl
        for (i=0; i<(int)nTabCols; i++)
            pTabDefs[i].nWidthFrac = nTabWidthTw / nTabCols;
    }
}


void SwW4WParser::ReadTabDefs( BYTE nCode, long nCols, W4W_ColdT* pActTabDefs )
{
    register W4W_ColdT *pColI;

    // alles ausser dem ersten Parametersatz fehlt
    if( nCode & 0x1 )
        nCols = 1;

    int i;
    // lies Parameter
    for (i=0, pColI=pActTabDefs; i<nCols; i++, pColI++)
        if(    !GetDecimal( pColI->nLeft )  || nError
            || !GetDecimal( pColI->nRight ) || nError
            || !GetDecimal( pColI->nNC )    || nError ) return;

    // lies opt. Parameter in Twips
    for (i=0, pColI=pActTabDefs; i<nCols; i++, pColI++)
        if(    !GetDecimal( pColI->nLeftTw )  || nError
            || !GetDecimal( pColI->nRightTw ) || nError
            || !GetDecimal( pColI->nNC2 )     || nError ) break;

    // Optionale Parameter nicht vorhanden -> berechne Twip-Werte
    if ( nError || ( i != nCols ) )
    {
        for (i=0, pColI=pActTabDefs; i<nCols; i++, pColI++)
        {
            pColI->nLeftTw  = pColI->nLeft  * 144;	// in 1/10 Inch
            pColI->nRightTw = pColI->nRight * 144;	// in 1/10 Inch
            pColI->nNC2     = pColI->nNC;
        }
    }

    // nur 1. Zelle angegeben, -> errechne alle weiteren
    if ( nCode & 0x1 )                       // Parametersaetze
    {
        long nWidth   = pActTabDefs->nRight   - pActTabDefs->nLeft;
        long nWidthTw = pActTabDefs->nRightTw - pActTabDefs->nLeftTw;
        for (i=1; i<(int)nTabCols; i++)
        {
            pActTabDefs[i]          = pActTabDefs[  0];
            pActTabDefs[i].nLeft    = pActTabDefs[i-1].nRight   + 2;
            pActTabDefs[i].nRight   = pActTabDefs[i  ].nLeft    + nWidth;
            pActTabDefs[i].nLeftTw  = pActTabDefs[i-1].nRightTw + 2 * 144;
            pActTabDefs[i].nRightTw = pActTabDefs[i  ].nLeftTw  + nWidthTw;
        }
    }
}


void SetCols( SwFrmFmt &rFmt, long nCols, W4W_ColdT* pActTabDefs, long nNettoWidth )
{
    if( nCols < 2 )
        return; // keine oder bloedsinnige Spalten

    SwFmtCol aCol;

    BOOL bEqual		= TRUE;
    long nAveDist	= 0;
    long nLastDist	= 0;
    long nLastWidth	= 0;

    for( int i=0; bEqual, i<(int)nCols; i++ )
    {
        // Spalten-Breite
        long nActWidth = pActTabDefs[ i ].nRightTw - pActTabDefs[ i ].nLeftTw;
        if( 0 < i )
        {
            if( nActWidth != nLastWidth )
                bEqual = FALSE;
            // Spalten-Abstand
            long nActDelta = pActTabDefs[ i ].nLeftTw - pActTabDefs[ i-1 ].nRightTw;
            if( (1 < i) && (nActDelta != nLastDist) )
                bEqual = FALSE;
            nAveDist += nActDelta;
            nLastDist = nActDelta;
        }
        nLastWidth = nActWidth;
    }
    nAveDist /= (nCols - 1);

    // ULONG nNettoWidth = pActTabDefs[ nCols-1 ].nRightTw - pActTabDefs[ 0 ].nLeftTw;



    // Zwischen-Linie
    /*
        aCol.SetLineAdj( COLADJ_TOP );
        aCol.SetLineHeight( 100 );
        aCol.SetLinePen( Pen( Color( COL_BLACK ), 1, PEN_SOLID ) );
    */



    if( bEqual )   // alle Spalten gleich ?
        aCol.Init( (USHORT)nCols, (USHORT)nAveDist, (USHORT)nNettoWidth );
    else
    {
        aCol.Init( (USHORT)nCols, (USHORT)nAveDist, USHRT_MAX );
        // Spalten unterschiedlich breit: fein, das kann der Writer inzwischen!
        USHORT nWishWidth = 0, nLeftDist = 0, nRightDist = 0;
        USHORT i;
        for( i = 0; i < nCols; i++ )
        {
            SwColumn* pCol = aCol.GetColumns()[ i ];
            pCol->SetLeft( nLeftDist );

            long nWidth = pActTabDefs[ i ].nRightTw - pActTabDefs[ i ].nLeftTw;
            if( i < nCols-1 )
            {
                long nRightDist = pActTabDefs[ i ].nRightTw - pActTabDefs[ i+1 ].nLeftTw;
                nRightDist = nWidth / 2;
                pCol->SetRight( (USHORT)nRightDist );
            }
            else
                nRightDist = 0; // letzte Spalte hat keinen Zwischenraum mehr

            pCol->SetWishWidth( nWidth + nLeftDist + pCol->GetRight() );

            // aufsummierte Spaltenbreiten ergeben Gesamtbreite
            nWishWidth += pCol->GetWishWidth();
            // Halber Abstand ist an naechster Spalte noch zu setzen
            nLeftDist = nRightDist;
        }
        aCol.SetWishWidth( nWishWidth );
    }
    rFmt.SetAttr( aCol );
}


void SwW4WParser::Read_EndSection()
{
    if( pBehindSection )
    {
        // vorigen Absatz entfernen,
        // falls unmittelbar hiervor ein 0x0d kam
        if( 0 == pCurPaM->GetPoint()->nContent.GetIndex() )
            JoinNode( pCurPaM );

        pCurPaM->GetPoint()->nNode = *pBehindSection;
        pCurPaM->GetPoint()->nContent.Assign(
            pCurPaM->GetCntntNode(), 0 );
        DELETEZ( pBehindSection );
    }
}


void SwW4WParser::Read_ColumnsDefinition()		// (CDS)
{
    if( bNoExec && !bBCMStep1 )
    {
        return;	// Jetzt aber nix wie weg hier!
    }

    if( pBehindSection )
        Read_EndSection();

    if( nTablInTablDepth && !bCheckTabAppendMode )
    {
        return;	// keine Tabelle bzw. Mehrspaltigkeit IN Tabelle
    }

    BYTE nCode;
    long nCols;
    W4W_ColdT* pActTabDefs = 0;	// unten wird auf Initialwert geprueft!

    bIsColDefTab = FALSE;
    if( GetHexByte( nCode ) && !nError && GetDecimal( nCols ) && !nError  )
    {
        /*
            bitte beachten:
            ***************
            im Gegensatz zu den anderen Spalten-Code-Methoden
            ist hier das Flag bBCMStep1 noch nicht fuer die
            ERSTE sondern nur fuer die Folge-Tabellen gesetzt.
            Das Flag bBCMStep2 hingegen stimmt immer.
            Hier im .<CDS> ist deshalb auf ( !bBCMStep2  ) zu pruefen,
            wenn man alle Teiltabellen (also auch die Start-Tabelle)
            des ersten Durchgangs braucht
        */
        if ( ( (int)nCode & 0x08 ) == 0x08 )	// Tabellendefinition
        {
            bIsColDefTab = TRUE;

            if( 48 == nDocType )				//!! Umpople Bug WP-W4W-Filter:
                nCode &= ~1;				   	// Bit 0 faelschlicherweise gesetzt
                                                // loesche es !
            if( bCheckTabAppendMode )
            {
                bIsTabAppendMode = BOOL(   ( nCols == nTabCols )
                                        && ( nCode == nTabCode ) );
            }

            // Durchgang 2 - Starttabelle
            if( bBCMStep2 && !bIsTabAppendMode )
            {
                // Tabellendefinition wurde im Durchgang 1 fix und fertig
                // aufgebaut, daher ist die Definition der Starttabelle
                // im Durchgang 2 komplett zu ueberspringen
                return;
            }

            // Durchgang 1 - Folgetabellen
            if( bIsTabAppendMode && bBCMStep1 )
            {
                // Dummy-Array zur Ermittlung der Spaltenbreiten
                pActTabDefs = new W4W_ColdT[nTabCols+1];
            }
            else
            {
                // Durchgang 1 - Starttabelle
                if( !bIsTabAppendMode )
                {
                    nTabCode   = nCode;
                    nTabCols   = (USHORT)nCols;
                    nTabRows   = 0;
                    bTabOwnFrm = FALSE;
                    if( pMergeGroups ) DELETEZ( pMergeGroups );
                }

                // Durchgang 1 - nur Starttabelle und
                // Durchgang 2 - nur Folgetabellen
                if( ! ( bBCMStep1 && bIsTabAppendMode ) )
                {
                    // globales Array neu anlegen
                    if( pTabDefs )
                        DELETEZ( pTabDefs );
                    pTabDefs = new W4W_ColdT[nTabCols+1];	// +1 wird nur nLeft... benutzt
                }

                // Parameter in globales Array einlesen
                pActTabDefs = pTabDefs;
            }


            /*
                Und auf gehts: Spalten-Parameter aus Input-Stream lesen...
            */
            ReadTabDefs( nCode, nCols, pActTabDefs );

            // Durchgang 1 - Start- und Folgetabellen
            // durchschn. Zwischenraum ermitteln
            if( !bBCMStep2 )
            {
                // Starttabelle
                if( !bIsTabAppendMode )
                    nTabDeltaSpace = 0;	// globalen Wert initialisieren

                // lokalen Wert ermitteln
                long   nActDeltaSpace = 0;
                USHORT nActDeltaCount = 0;
                for (int i=1; i<(int)nTabCols; i++)
                {
                    if( pActTabDefs[ i ].nRightTw > pActTabDefs[ i ].nLeftTw+10 )
                    {
                        nActDeltaSpace +=
                            pActTabDefs[ i ].nLeftTw - pActTabDefs[ i-1 ].nRightTw;
                        nActDeltaCount++;
                    }
                }
                // ggfs. globalen Wert korrigieren
                if( nActDeltaCount )
                {
                    nActDeltaSpace = nActDeltaSpace / nActDeltaCount;
                    if( nTabDeltaSpace < nActDeltaSpace )
                        nTabDeltaSpace = nActDeltaSpace;
                }

                // falls noetig das Dummy-Array freigeben
                if(    pActTabDefs
                    && ( pActTabDefs != pTabDefs ) )
                    delete[] pActTabDefs;
            }
            // Durchgang 2: Tabellenspaltendefinitionen anpassen
            else
                Adjust_pTabDefs();
        }
        else
        {
            // mehrspaltiger Bereich ODER mehrspaltiger Rahmen
            pActTabDefs = new W4W_ColdT[nCols+1];

            ReadTabDefs( nCode, nCols, pActTabDefs );

            if( nCols && pActTabDefs )
            {
                // mehrspaltiger Rahmen
                if( pActFlySection )
                {
                    long nAveDist = 0;
                    for (int i=1; i<(int)nCols; i++)
                        nAveDist +=
                            pActTabDefs[ i ].nLeftTw - pActTabDefs[ i-1 ].nRightTw;
                    nAveDist /= (nCols - 1);

                    SwFmtCol aCol;

// wozu braucht man denn dies?  const SvxLRSpaceItem& rLR = (SvxLRSpaceItem&)pSet->Get( RES_LR_SPACE );

                    ULONG nWidth = USHRT_MAX;
                    aCol.Init( USHORT( nCols ), USHORT( nAveDist ), USHORT( nWidth ) );
                    /*
                    spaeter nachruesten: unterschiedliche Spaltenbreiten und -Abstaende

                    if( nCols == ( aColumns.Count() / 2 ) )
                    {
                        for( USHORT n = 0, i = 0; n < aColumns.Count(); n += 2, ++i )
                        {
                            SwColumn* pCol = aCol.GetColumns()[ i ];
                            ULONG nTmp = aColumns[ n ];
                            nTmp *= USHRT_MAX;
                            nTmp /= nWidth;
                            pCol->SetWishWidth( USHORT(nTmp) );
            /*
                JP 07.07.95: der Dialog kennt nur eine Breite fuer alle Spalten
                             darum hier nicht weiter beachten
                            nTmp = aColumns[ n+1 ];
                            if( nTmp )
                                pCol->SetRight( USHORT(nTmp) );
                            else
                                pCol->SetRight( 0 );
                            pCol->SetLeft( 0 );
            *
                        }
                    }
                    */
                    pActFlySection->SetAttr( aCol );
                }
                else
                {
                    // normale Mehrspaltigkeit: Bereich einfuegen
                    SwSection aSection( CONTENT_SECTION,
                                        pDoc->GetUniqueSectionName() );

                    SwSection* pNewSection = pDoc->Insert( *pCurPaM, aSection );
                    pBehindSection = new SwNodeIndex( pCurPaM->GetPoint()->nNode );

                    // Anzahl der Spalten einstellen
                    SwFrmFmt& rFmt = pPageDesc->GetMaster();
                    const SwFmtFrmSize&   rSz = rFmt.GetFrmSize();
                    const SvxLRSpaceItem& rLR = rFmt.GetLRSpace();
                    SwTwips nWidth = rSz.GetWidth();
                    USHORT  nLeft  = rLR.GetTxtLeft();
                    USHORT  nRight = rLR.GetRight();

                    SetCols( *pNewSection->GetFmt(), nCols,
                             pActTabDefs,            nWidth - nLeft - nRight );

                    // PaM in Node der Section setzen
                    const SwSectionNode* pSectionNode =
                        pNewSection->GetFmt()->GetSectionNode();
                    ASSERT( pSectionNode, "Kein Inhalt vorbereitet." );
                    pCurPaM->GetPoint()->nNode =
                        pSectionNode->GetIndex()+1;
                    pCurPaM->GetPoint()->nContent.Assign(
                        pCurPaM->GetCntntNode(), 0 );
                }
            }
            delete[] pActTabDefs;
        }
    }
}


// folgende pragma Anweisung nicht entfernen: die Complier Optimierung fuehrt
// ansonsten in NICHT-DEBUG Versionen zu fehlerhaften Tabellen-Zellen-Raendern

#ifdef _MSC_VER
#pragma optimize("", off)
#endif
// ======================
void SwW4WParser::Read_BeginColumnMode()		// (BCM)
{
    if( bNoExec && !bBCMStep1 )
    {
        return;	// Jetzt aber nix wie weg hier!
    }

    if( bIsTabAppendMode )
    {
        return;	// .<BCM>-Codes der angehaengten Tabellen werden ueberlesen
    }

    if( !bIsColDefTab )
    {
        return;	// derzeit noch keine Mehrspaltigkeit ohne Tabellen
    }

    // erzwinge AbsatzEnde, falls Content-Node nicht leer
    if( pCurPaM->GetPoint()->nContent.GetIndex() != 0 )
        pDoc->SplitNode( *pCurPaM->GetPoint() );

    if( bBCMStep2 && pDoc->IsIdxInTbl( pCurPaM->GetPoint()->nNode ) )
    {
        // Tabellen-Daten IN einer anderen Tabelle als Rohtext lesen
        Flush();
        BOOL bOldIsColMode = bIsColMode;
        BOOL bOldIsTxtInPgDesc = bIsTxtInPgDesc;
        bIsColMode         = TRUE;
        nTablInTablDepth++;
        while(     !nError
                && bIsColMode
                && EOF != GetNextRecord() )
        {
            /*NOP*/;
        }
        bIsColMode = bOldIsColMode;
        bIsTxtInPgDesc = bOldIsTxtInPgDesc;
        nTablInTablDepth--;
        return;					// und raus hier!
    }

    bIsSTYInTab = FALSE;
    nTabStyleId = nAktStyleId;
    nTabRows = nTabRow = 0;
    bTabBorder = bTabSpezBorder = FALSE;

    Flush();

    /*
        Durchgang 1 starten:
        Suche Ende der Tabelle oder EOF, zaehle dabei Zeilen und Spalten
        und speichere die .<BRO>-Border-Infos aller Zellen in pTabBorders
        und ermittle durchschnittl. Zwischenraum
    */
    ULONG nOldPos       = rInp.Tell();	 	// merke FilePos
    BOOL bOldTxtInDoc   = bTxtInDoc;
    BOOL bOldIsTxtInPgDesc = bIsTxtInPgDesc;
    BOOL bOldNoExec     = bNoExec;
    BYTE nOldErr        = nError;
    if( !pTabBorders )
        pTabBorders = new W4WTabBorders;	// Array fuer .<BRO>-Raender
    bTxtInDoc           = FALSE;
    bWasCellAfterCBreak = FALSE;
    bNoExec             = TRUE;
    bBCMStep1           = TRUE;
    bIsColMode          = TRUE;
    int iRet=0;
    while(     !nError
            && bIsColMode
            && EOF != iRet )
    {
        iRet = GetNextRecord();
    }

    nParaLen  = 0;	// Neuer TextNode ist aktiv -> noch KEIN Zeichen drin
    Flush();
    nError    = nOldErr;
    bTxtInDoc = bOldTxtInDoc;
    bIsTxtInPgDesc = bOldIsTxtInPgDesc;
    bNoExec   = bOldNoExec;
    bBCMStep1 = FALSE;

    if (   ( EOF == iRet ) 	// W4W-Fehler: Ende der Tabelle fehlt (WinWord 2)
        || !nTabRows 		// leere Tabelle ( z.B. AmiPro ) geht schief
        || !nTabCols )
    {
        // Spule zurueck an TabellenAnfang
        // und betrachte Tabelleninhalt als normalen Text
        rInp.Seek( nOldPos + 1 );
        bIsColMode = FALSE;
    }
    else
    {
        // Column-Mode-Flag setzen (war durch .<ECM> ausgeschaltet worden)
        bIsColMode = TRUE;

        // Inputstream zurueckspulen (wird im 2. Durchgang erneut gelesen)
        rInp.Seek( nOldPos );

        // ggfs. durchschn. Zellen-Zwischenraum ueber Defaults definieren
        if( !nTabDeltaSpace )
            nTabDeltaSpace = (nDocType == 07) ? 0 : 140;

        // Tabellenspaltendefinitionen anpassen
        Adjust_pTabDefs();

        /*
            wichtig: Korrektur der Werte in pTabBorders
        */
        for( int iR=0; iR < nTabRows; iR++ )	 // ueber alle Zeilen
        {
            // Hilfsvaris zum uebersichtlicheren Zugriff auf Zeilen-Arrays
            USHORT* aThisRow  = (*pTabBorders)[ iR   ];
            USHORT* aAboveRow;
            if( iR > 0 )
                aAboveRow = (*pTabBorders)[ iR-1 ];
            USHORT* aBelowRow;
            if( iR < nTabRows-1 )
                aBelowRow = (*pTabBorders)[ iR+1 ];

            for ( int iC=0; iC < nTabCols; iC++ )// ueber alle Spalten
            {
                //
                //  1. Korrigiere senkrechte Raender
                //
                //  1.1 Verhindere doppelten (identischen) Rand
                //      aneinanderstossender Zellen
                //
                if(    (iC > 0)
                    && (       (  aThisRow[ iC-1 ] & 0x000f       )
                            == ( (aThisRow[ iC   ] & 0x0f00) >> 8 ) ) )
                {
                    // loesche li. Rand des aktuellen Feldes
                    aThisRow[ iC ] &= 0xf0ff;
                }
                //
                //  1.2 Vermeide versetzte Kanten
                //
                if( iR > 0 )
                {
                    // pruefe, ob der re. Rand des aktuellen Feldes
                    // gleich ist dem li. Rand des Feldes oben rechts davon
                    if(    (iC < nTabCols-1)
                        && (aThisRow[ iC ] & 0x000f)
                        && (       (  aAboveRow[ iC+1 ] & 0x0f00       )
                                == ( (aThisRow [ iC   ] & 0x000f) << 8 )
                           )
                        )
                    {
                        // setze li. Rand des Folge-Feldes und
                        // loesche re. Rand des aktuellen Feldes
                        aThisRow[ iC+1 ] &= 0xf0ff;
                        aThisRow[ iC+1 ] |= ((aThisRow[ iC ] & 0x000f) << 8);
                        aThisRow[ iC   ] &= 0xfff0;
                    }
                    // pruefe, ob der li. Rand des aktuellen Feldes
                    // gleich ist dem re. Rand des Feldes oben links davon
                    if(    (iC > 0)
                        && (aThisRow[ iC ] & 0x0f00)
                        && (       ( aAboveRow[ iC-1 ] & 0x000f       )
                                == ((aThisRow [ iC   ] & 0x0f00) >> 8 )
                           )
                        )
                    {
                        // setze re. Rand des vorigen Feldes und
                        // loesche li. Rand des aktuellen Feldes
                        aThisRow[ iC-1 ] &= 0xfff0;
                        aThisRow[ iC-1 ] |= ((aThisRow[ iC ] & 0x0f00) >> 8);
                        aThisRow[ iC   ] &= 0xf0ff;
                    }
                }
                //
                //  2. Korrigiere waagerechte Raender
                //
                //  2.1 Verhindere doppelten (identischen) Rand
                //      aneinanderstossender Zellen
                //
                if(    (iR > 0)
                    && (       ( (aAboveRow[ iC ] & 0x00f0) >> 4 )
                            == ( (aThisRow[  iC ] & 0xf000) >>12 ) ) )
                {
                    // loesche oberen Rand des aktuellen Feldes
                    aThisRow[ iC ] &= 0x0fff;
                }
                //
                //  2.2 Vermeide versetzte Kanten
                //
                if( iC > 0 )
                {
                    // pruefe, ob der unt. Rand des aktuellen Feldes
                    // gleich ist dem ob. Rand des Feldes links unten davon
                    if(    (iR < nTabRows-1)
                        && (aThisRow[ iC ] & 0x00f0)
                        && (       (  aBelowRow[ iC-1 ] & 0xf000       )
                                == ( (aThisRow [ iC   ] & 0x00f0) << 8 )
                           )
                        )
                    {
                        // setze ob. Rand des darunter liegenden Feldes und
                        // loesche unt. Rand des aktuellen Feldes
                        aBelowRow[ iC ] &= 0x0fff;
                        aBelowRow[ iC ] |= ((aThisRow[ iC ] & 0x00f0) << 8);
                        aThisRow[  iC ] &= 0xff0f;
                    }
                    // pruefe, ob der ob. Rand des aktuellen Feldes
                    // gleich ist dem unt. Rand des Feldes links oben davon
                    if(    (iR > 0)
                        && (aThisRow[ iC ] & 0xf000)
                        && (       ( aAboveRow[ iC-1 ] & 0x00f0       )
                                == ((aThisRow [ iC   ] & 0xf000) >> 8 )
                           )
                        )
                    {
                        // setze unt. Rand des darueber liegenden Feldes und
                        // loesche ob. Rand des aktuellen Feldes
                        aAboveRow[ iC ] &= 0xff0f;
                        aAboveRow[ iC ] |= ((aThisRow[ iC ] & 0xf000) >> 8);
                        aThisRow[  iC ] &= 0x0fff;
                    }
                }
            }
        }

        // merke aktuelle Point-Position des pCurPaM
        SwPosition aTmpPos( *pCurPaM->GetPoint() );

        static const SwHoriOrient aOrientTab[5] =
            { HORI_LEFT, HORI_FULL, HORI_CENTER, HORI_RIGHT, HORI_NONE };
        USHORT nIdx = nTabCode >> 4;
        if ( nIdx > 4 ) nIdx = 0;
        SwHoriOrient eOri = aOrientTab[ nIdx ];

        /*
            Einrueckung der Tabelle ermitteln
        */
        long nNewLeft = 0; 	// Default: keine Einrueckung
        long nNewRight;

        if(( eOri == HORI_LEFT ) || ( eOri == HORI_NONE ))
        {
            nNewLeft = pTabDefs[0].nLeftTw;
            if( nTabCols > 1 )
                nNewLeft -= (pTabDefs[1].nLeftTw - pTabDefs[0].nRightTw) /2;

            // Umrechnen auf SW-Koordinaten
            nNewRight = nPgRight - nNewLeft - nTabWidthTw;
            nNewLeft -= nPgLeft;
            if( nNewRight < 0 ) nNewRight = 0;
            if( nNewLeft  < 0 ) nNewLeft  = 0;

            // ggfs. Linksbuendigkeit abschalten,
            // damit korrekt positioniert wird
            if( nNewLeft )
            {
                eOri = HORI_LEFT_AND_WIDTH;
            }
        }

        // schalte evtl. aktuell gesetzten Style aus
        BOOL bWasStyleBeforeTable = (USHRT_MAX != nAktStyleId);
        if( bWasStyleBeforeTable )
        {
            pCtrlStck->SetAttr( *pCurPaM->GetPoint(), RES_FLTR_STYLESHEET );
        }

        // Tabelle im pDoc anlegen
        // und Table-Node aus dem Rueckgabewert ermitteln
        pTblNd= (SwTableNode*)
                ( pDoc->InsertTable( aTmpPos, nTabRows, nTabCols, eOri )
                    ->GetTabSortBoxes()[ 0 ]
                    ->GetSttNd()
                    ->FindTableNode() );
        ASSERT( pTblNd, "wo ist mein TabellenNode" );

        // lokale Variable zum bequemeren Zugriff
        SwTable& rTable = pTblNd->GetTable();

        // schalte ggfs. vorigen Style wieder ein
        if( bWasStyleBeforeTable )
        {
            SetAttr( SwW4WStyle( nAktStyleId ) );
        }

        SwFrmFmt* pFrmFmt = rTable.GetFrmFmt();

        // setze Gesamtbreite der Tabelle
        SwFmtFrmSize aSize;
        aSize.SetWidth( nTabWidthTw );
        pFrmFmt->SetAttr( aSize );

        // Tabelle einruecken (falls noetig)
        if( HORI_LEFT_AND_WIDTH == eOri )
        {
            SvxLRSpaceItem aLR;
            aLR.SetLeft( USHORT(nNewLeft) );
            // aLR.SetRight(   USHORT(nNewRight) );
            pFrmFmt->SetAttr( aLR );
        }

        // Init glob. Vars
        nTabRow = -1;
        nLastProcessedCol = -1;
        SetPamInCell( 0, 0 );

        // setze pCurPaM-Point auf den TabellenNode
        rTable.SetHeadlineRepeat( FALSE );


        /*
            Durchgang 2 starten:
            Suche Ende der Tabelle oder EOF
            und lies dabei den Tabellen-Inhalt in rTable ein
        */
        bWasCellAfterCBreak = FALSE;
        BOOL bOldIsTxtInPgDesc = bIsTxtInPgDesc;
        bBCMStep2 = TRUE;
        while (    !nError
                && bIsColMode
                && (EOF != GetNextRecord()) )
        {
            /*NOP*/;
        }
        bBCMStep2 = FALSE;
        bIsTxtInPgDesc = bOldIsTxtInPgDesc;

        // pCurPaM-Point zuruecksetzen
        *pCurPaM->GetPoint() = aTmpPos;

        // falls noetig, zu mergende Zellen gruppenweise zusammenfassen
        if( pMergeGroups )
        {
            // bearbeite alle Merge-Gruppen nacheinander
            SwTableBox*     pTargetBox;
            SwSelBoxes_SAR* pActMGroup;
            USHORT          nActBoxCount;
            for( USHORT iGr = 0; iGr < pMergeGroups->Count(); iGr++ )
            {
                pActMGroup   = (*pMergeGroups)[ iGr ];
                nActBoxCount = pActMGroup->Count();

                if( 1 < nActBoxCount )
                {
                    // beachten: der 0. Eintrag ist die
                    // bereits fix und fertig formatierte Target-Box !!!
                    pTargetBox = (*pActMGroup)[ 0 ];

                    // oeffne Merge-Array passender Groesse
                    SwSelBoxes aBoxes( nActBoxCount-1 );

                    // alle Boxen dieser Gruppe in aBoxes versammeln
                    aBoxes.Insert( pActMGroup->GetData()+1, nActBoxCount-1 );

                    // Vorsicht: erstmal pruefen, ob sich die aBoxes mergen
                    // lassen wuerden, ansonsten den Inhalt der Target-Box
                    // in die obere linke Box verschieben und die Target-Box
                    // den Hasen geben...

                    switch( CheckMergeSel( aBoxes ) )	// siehe TBLSEL.HXX
                    {
                    case TBLMERGE_OK:
                        // das praktische Merge()
                        rTable.Merge( pDoc, aBoxes, pTargetBox );
                        break;
                    case TBLMERGE_NOSELECTION:
                        /*NOP*/;	// war wohl nix?
                        break;
                    case TBLMERGE_TOOCOMPLEX:
                        // hoppla: die Daten der Gruppe schlummern jetzt
                        // in der Target-Box. Da kein Merge moeglich ist,
                        // setzen wir die Target-Box in die Tabelle an
                        // die Stelle der oberen linken Box.
                        {
                            const SwTableBox* pBox  = (*pActMGroup)[ 1 ];
                            SwTableLine* pLine = (SwTableLine*)pBox->GetUpper();
                            USHORT nPos = pLine->GetTabBoxes().GetPos( pBox );
                            ASSERT( USHRT_MAX != nPos, "GetPos fehlgeschlagen");
                            SwStartNode* pSttNd = (SwStartNode*)pBox->GetSttNd();
                            ASSERT( pSttNd, "Box ohne Start-Node ?!");

                            pTargetBox->ChgFrmFmt(
                                (SwTableBoxFmt*)pBox->GetFrmFmt() );
                            pTargetBox->SetUpper(  pLine );

                            // erst die Box loeschen!!
                            pLine->GetTabBoxes().DeleteAndDestroy( nPos );
                            // dann die pTargetBox einfuegen
                            pLine->GetTabBoxes().C40_INSERT( SwTableBox, pTargetBox, nPos );

//							pLine->GetTabBoxes().Insert( pTargetBox, nPos );
                            // dann die Nodes loeschen!!
                            pDoc->DeleteSection( pSttNd );
                        }
                        break;
                    default:			// was wollen wir denn hier ???
                        ASSERT( !this, "CheckMergeSel() with undefined return value" );
                        break;
                    }
                }
            }
        }
    }
    DELETEZ( pTabBorders );
}
#ifdef _MSC_VER
#pragma optimize("", off)
#endif
// ======================


void SwW4WParser::Read_EndColumnMode()			// (ECM) End Column Mode
{
    if( pBehindSection )
    {
        Read_EndSection(); // spaltiger Bereich zu Ende
        return;
    }

    if( bNoExec && !bBCMStep1 )
    {
        return;	// Jetzt aber nix wie weg hier!
    }

    if ( !bIsColMode ) return;	// W4W-Fehler

    if( nTablInTablDepth )	// Tabelle / Mehrspaltigkeit IN Tabelle ?
    {
        bIsColMode = FALSE;
    }
    else
    {
        // sieh nach, ob .<CDS> folgt, das
        // noch zu DIESER Tabelle gehoert
        ULONG nOldPos       = rInp.Tell();		// FilePos merken
        bIsTabAppendMode    = FALSE;
        BOOL bOldIsTxtInPgDesc = bIsTxtInPgDesc;
        bCheckTabAppendMode = TRUE;
            SkipEndRecord();
            GetNextRecord();
        bCheckTabAppendMode = FALSE;
        bIsTxtInPgDesc = bOldIsTxtInPgDesc;

        // tatsaechliches Tabellen-Ende erreicht?
        if( !bIsTabAppendMode )
        {
            rInp.Seek( nOldPos ); 				// FilePos restaurieren
            bIsColMode = FALSE;
            if ( bBCMStep2 && pTabDefs )
                DELETEZ( pTabDefs );
        }
    }
}


/*
     SwW4WParser::SetPamInCell() ist eine Mehrzweck-Methode:
     1. sorgt immer fuer gueltige Werte in den globalen Variablen
        pTabLine, pTabBoxes und pTabBox
     2. setzt den PaM (falls bSetPaM wahr ist),
*/
void SwW4WParser::SetPamInCell( USHORT nRow, USHORT nCol, BOOL bSetPaM )
{
    SwTableLines& rTheTabLines = pTblNd->GetTable().GetTabLines();

    // nRow gegen max. Zeilenzahl im Zeilen-Array pruefen
    if(nRow >= rTheTabLines.Count()) nRow = rTheTabLines.Count() - 1;

    // akt. Zeile holen
    pTabLine = rTheTabLines[ nRow ];

    // akt. Boxen-Array holen
    pTabBoxes = &pTabLine->GetTabBoxes();

    // nCol gegen max. Boxenzahl im Boxen-Array pruefen
    if (nCol >= pTabBoxes->Count()) nCol = pTabBoxes->Count() - 1;

    // akt. Box holen
    pTabBox = (*pTabBoxes)[nCol];

    // Start-Node der akt. Box holen
    const SwNode* pSttNd = pTabBox->GetSttNd();

    ASSERT(pSttNd, "Probleme beim Aufbau der Tabelle");

    // JETZT den PaM-Point auf den Start-Node der Box setzen
    if (bSetPaM)
    {
        PtNd(  pCurPaM ) = pSttNd->GetIndex()+ 1;

        PtCnt( pCurPaM ).Assign( pCurPaM->GetCntntNode(), 0 );
    }
}


void SwW4WParser::Read_BeginTabRow()			// (BRO) Header einer Zeile
{
    if( bNoExec && !bBCMStep1 )
    {
        return;	// Jetzt aber nix wie weg hier!
    }

    if( nTablInTablDepth )	// Tabelle / Mehrspaltigkeit IN Tabelle ?
    {
        return;
    }

    long nCols, nHeight, nHeightTw, nJust;
    int i;

    if( !bIsColMode ) return;					// W4W-Fehler

    if( GetDecimal( nCols     ) && !nError &&
        GetDecimal( nHeight   ) && !nError &&
        GetDecimal( nHeightTw ) && !nError )
    {
        nTabRow++;	// aktuelle Zeilen-Nr.

        // Merge: nTabCols aus .<CDS> ist der Maximalwert,
        //        der in .<BRO> nie ueberschritten werden darf.
        if( (USHORT)nCols > nTabCols )
            nCols = nTabCols;

        if( bBCMStep1 )
        {
            /*
                Durchgang 1: Anzahl der Zeilen ermitteln,
                             Border-Codes fuer alle Spalten einlesen,
                             aber den Tabelleninhalt noch nicht einlesen
            */
            nTabRows++;

            // Border-Zeile an pTabBorders anhaengen
            UShortPtr pActRow = new USHORT[ nTabCols ];
            pTabBorders->Insert( pActRow, pTabBorders->Count() );

            // Codes einlesen
            for ( i=0; i < nCols; i++ )
            {
                //
                // Hinweis: LEIDER schreibt W4W bei WordPerfekt 8 des oefteren
                // den Wert 0x0000 anstatt der richtigen Border-Flags.
                // Dies geschieht z.B. oft, wenn in WP der Zellen-Rand mit
                // der Maus nach oben o. unten geschoben wurde.
                //
                // Abhilfe: leider unbekannt!
                //
                USHORT & rActBFlag = pActRow[ i ];

                if ( !GetHexUShort( rActBFlag ) || nError )
                    return;

                // nachschauen, ob spezielle Borders definiert
                bTabBorder |= ( rActBFlag != 0x0000 );
                if ( nTabRow==1 && i==0 )
                    nTabDefBorder = rActBFlag;
                else
                if ( rActBFlag != nTabDefBorder )
                    bTabSpezBorder = TRUE;
            }
            if( GetDecimal( nJust     ) && !nError )
            {
                /*NOP*/		// nJust wird noch ignoriert
            }
        }
        else
        {
            /*
                Durchgang 2: einlesen der Daten vorbereiten
            */
            SetPamInCell( nTabRow, 0 );		// noetig ???
            UpdatePercent( rInp.Tell(), nW4WFileSize );
            nLastProcessedCol   = -1;
            bWasCellAfterCBreak = FALSE;
        }
    }
}

// Setze Umrandung, nach SetPamInCell() aufrufen

USHORT SwW4WParser::Read_SetTabBorder( USHORT nW4WRow,
                                       USHORT nW4WCol,
                                       SvxBoxItem& rFmtBox )
{
    rFmtBox.SetDistance( DEFAULT_TAB_CELL_DISTANCE );

    if ( bTabBorder )
    {
        USHORT nBor = (*pTabBorders)[ nW4WRow ][ nW4WCol ];

        if( nBor )
            return Read_SetBorder( nBor, rFmtBox );
    }
    return 1;
}


SwTableBox* SwW4WParser::UpdateTableMergeGroup( SwSelBoxes_SAR* pActGroup,
                                                SwTableBox*     pActBox,
                                                USHORT          nCol )
{
    // ggfs. die Box in fuer diese Col offene Merge-Gruppe eintragen
    if( 0 < pTabDefs[ nCol ].nRemainingMergeRows )
    {
        // passende Merge-Gruppe uebernehmen bzw. ermitteln
        SwSelBoxes_SAR* pTheMergeGroup;
        if( pActGroup )
            pTheMergeGroup = pActGroup;
        else
        {
            USHORT nMGrIdx = pTabDefs[ nCol ].nMergeGroupIdx;
            ASSERT( nMGrIdx < pMergeGroups->Count(),
                    "Merge Group Idx zu gross" );
            pTheMergeGroup = (*pMergeGroups)[ nMGrIdx ];
        }

        // aktuelle Box der Merge-Gruppe hinzufuegen
        pTheMergeGroup->Insert( pActBox, pTheMergeGroup->Count() );

        // Merge-Row-Zaehler vermindern
        pTabDefs[ nCol ].nRemainingMergeRows--;

        // Target-Box zurueckmelden
        return (*pTheMergeGroup)[ 0 ];
    }
    else
    {
        // NULL zurueckmelden
        return 0;
    }
}


void SwW4WParser::Read_BeginTabCell()			// (BCO)	Header eines Feldes
{
    if( bNoExec && !bBCMStep1 )
    {
        return;	// Jetzt aber nix wie weg hier!
    }

    if( nTablInTablDepth )	// Tabelle / Mehrspaltigkeit IN Tabelle ?
    {
        return;
    }

    long nCol, nCellSpan, nRowSpan, nNotExist, nDecimals;
    long nHAlign = 0;
    long nVAlign = 0;
    SwTableBox* pTargetBox;


    if (!bIsColMode) return; // W4W-Fehler

    if(    GetDecimal( nCol )      && !nError
        && GetDecimal( nCellSpan ) && !nError
        && GetDecimal( nRowSpan )  && !nError
        && GetDecimal( nNotExist ) && !nError
        && GetDecimal( nHAlign )   && !nError
        && GetDecimal( nDecimals ) && !nError
        && GetDecimal( nVAlign )   && !nError )
    {

        // Zelle 0 gilt nicht
        if( !nCol )
            return;

        // Spalten-Nr. gegen maximale Zahl der Spalten pruefen
        if( nTabCols < (USHORT)nCol )
            return;

        bWasCellAfterCBreak = TRUE;

        // falschen Wert korrigieren
        if( nRowSpan < 1 )
            nRowSpan = 1;

        if( bBCMStep1 )
        {
            /*
                Durchgang 1: ggfs. Borders von Merge-Zellen anpassen
            */
            if( nCellSpan > 1 )
            {
                USHORT* aThisRow  = (*pTabBorders)[ nTabRow-1 ];
                USHORT nActColIdx = (USHORT)nCol-1;
                if( nDocType == 44 )
                    // Macke WW2: alle Border muessen verodert werden
                    for (USHORT j=nActColIdx; j < nActColIdx+nCellSpan; j++ )
                    {
                        aThisRow[ nActColIdx ] |= aThisRow[ j ];
                    }
                else
                    // rechte Border der re. Zelle in Zelle 1 hineinodern
                    aThisRow[ nActColIdx ]
                        |= (aThisRow[ nActColIdx+nCellSpan-1 ] & 0x000f);
                // Re. Border  der re. Zelle entsprechend setzen.
                // Diese wird nie sichtbar, ermoeglicht aber die
                // Randkorrektur in SwW4WParser::Read_BeginColumnMode()
                aThisRow[ nActColIdx+nCellSpan-1 ] &=  0xfff0;
                aThisRow[ nActColIdx+nCellSpan-1 ] |=
                            ( aThisRow[ nActColIdx ] & 0x000f );
            }
        }
        else
        {
            /*
                Durchgang 2: Tabelleninhalt einlesen
            */

            nParaLen = 0; // Neuer TextNode aktiv -> noch kein Zeichen drin

            // korrigiere nCol (tatsaechliche Zellen-Nr beginnt ja mit 0)
            nCol--;

            // merke nCol in nTabCol (auch in W4WPAR1.CXX mehrfach benoetigt)
            nTabCol = (USHORT)nCol;

            SetPamInCell( nTabRow, nTabCol );
            if ( bTabOwnFrm )
            {
                pTabBox->ClaimFrmFmt();
                pTabBox->GetFrmFmt()->SetAttr( SwFmtFrmSize( ATT_VAR_SIZE,
                            pTabDefs[ nTabCol ].nWidthFrac ));
            }
            SvxBoxItem aFmtBox;
            USHORT nRet
                = Read_SetTabBorder( nTabRow, nTabCol, aFmtBox );
            // eigenes Format, falls noch nicht vorhanden
            if ( 0 != nRet )
                pTabBox->ClaimFrmFmt();
            // Setze evtl Umrandung
            if ( 0 != ( 0x1 & nRet ))
                pTabBox->GetFrmFmt()->SetAttr( aFmtBox );
            // setze evtl Hintergrund
            if ( 0 != ( 0x2 & nRet ))
            {
                const Color aCol( COL_LIGHTGRAY );
                const SvxBrushItem aBack( aCol, RES_BACKGROUND );
                pTabBox->GetFrmFmt()->SetAttr( aBack );
            }

            SwSelBoxes_SAR* pActMGroup = 0;

            /*
                ggfs. eine neue Merge-Gruppe beginnen
            */
            if(    ( 0 == pTabDefs[ nTabCol ].nRemainingMergeRows )
                && (   (nCellSpan > 1)
                    || (nRowSpan  > 1) ) )
            {
                // 0. falls noetig das Array fuer die Merge-Gruppen anlegen
                if( !pMergeGroups )
                    pMergeGroups = new W4WMergeGroups;

                // 1. aktuelle Merge-Gruppe anlegen
                //    und in Gruppen-Array eintragen
                pActMGroup = new SwSelBoxes_SAR( BYTE(nCellSpan * nRowSpan));

                pMergeGroups->Insert( pActMGroup, pMergeGroups->Count() );
//				pMergeGroups->Insert( pActMGroup, pMergeGroups->Count() );

                // 3. Index dieser Merge-Gruppe und Anzahl der betroffenen
                //    Zeilen in allen betroffenen Spalten vermerken
                USHORT nMGrIdx = pMergeGroups->Count()-1;
                long nSizCell = 0;

                for( USHORT i = 0; i < nCellSpan; i++ )
                {
                    pTabDefs[ nTabCol+i ].nMergeGroupIdx      = nMGrIdx;
                    pTabDefs[ nTabCol+i ].nRemainingMergeRows = nRowSpan;
                    // dabei auch gleich die Gesamtbreite berechnen
                    nSizCell += pTabDefs[ nTabCol+i ].nWidthFrac;
                }

                /*
                    4. Target-Box anlegen und als 0. in Merge-Group setzen
                */
                pDoc->GetNodes().InsBoxen(
                                    pTblNd, pTabLine,
                                    (SwTableBoxFmt*)pTabBox->GetFrmFmt(),
                                    (SwTxtFmtColl*)pDoc->GetDfltTxtFmtColl(),
                    0,
                                    nTabCol );
                pTargetBox = (*pTabBoxes)[ nTabCol ];
                // eingefuegte Box wieder aus der Row entfernen
                // (wird vom Merge() dann endgueltig richtig eingebaut)
                pTabBoxes->Remove( nTabCol );
                // und ab damit in die Merge-Group
                pActMGroup->Insert( pTargetBox, pActMGroup->Count() );
                /*
                    5. Target-Box formatieren
                */
                pTargetBox->SetUpper( 0 );
                // eigenes Frame-Format fuer diese Box anlegen
                SwFrmFmt* pNewFrmFmt = pTargetBox->ClaimFrmFmt();
                // Border der O-L-Box der Gruppe wird Border der Targetbox
                pNewFrmFmt->SetAttr(
                    pTabBox->GetFrmFmt()->GetBox() );
                // Gesamtbreite zuweisen
                pNewFrmFmt->SetAttr( SwFmtFrmSize( ATT_VAR_SIZE, nSizCell ));
            }

            // ggfs. akt. Box zu einer Merge-Group hinzufuegen
            pTargetBox = UpdateTableMergeGroup( pActMGroup,pTabBox,nTabCol );
            if( pTargetBox )
            {
                // den pCurPaM jetzt ans Ende der Target-Box setzen,
                // damit der Text direkt in die richtige Box gelesen wird.

                const SwNode* pEndNd =
                        pTargetBox->GetSttNd()->EndOfSectionNode();

                ASSERT(pEndNd, "Gruppen-TargetBox ohne Start-Node ?");

                PtNd( pCurPaM ) = pEndNd->GetIndex();

                pCurPaM->Move( fnMoveBackward );
            }


            const SwPosition& rPos = *pCurPaM->GetPoint();
            // Override Style "Tabellenkopf", "Tabelleninhalt" mit
            // vorher gueltigem Style wenn kein Style hart gesetzt ist
            if ( !bIsSTYInTab )
            {
                pCtrlStck->NewAttr( rPos,  SwW4WStyle( nTabStyleId ) );
                pCtrlStck->SetAttr( rPos, RES_FLTR_STYLESHEET );
            }
            // setze die horizontale Zellenausrichtung
            switch( nHAlign )
            {
            case 0:pCtrlStck->NewAttr( rPos, SvxAdjustItem( SVX_ADJUST_LEFT   )); break;
            case 1:pCtrlStck->NewAttr( rPos, SvxAdjustItem( SVX_ADJUST_BLOCK  )); break;
            case 2:pCtrlStck->NewAttr( rPos, SvxAdjustItem( SVX_ADJUST_CENTER )); break;
            case 3:pCtrlStck->NewAttr( rPos, SvxAdjustItem( SVX_ADJUST_RIGHT  )); break;
            case 4:pCtrlStck->NewAttr( rPos, SvxAdjustItem( SVX_ADJUST_RIGHT  )); break;
            }
            pCtrlStck->SetAttr( rPos, RES_PARATR_ADJUST );
            nLastProcessedCol = nCol;
        }
    }
}


void SwW4WParser::Read_ColumnBreak()			// (HCB, SCB)
{
    if( pBehindSection ) // im spaltigen Bereich
    {
        pDoc->AppendTxtNode( *pCurPaM->GetPoint() );
        pDoc->Insert(*pCurPaM, SvxFmtBreakItem( SVX_BREAK_COLUMN_BEFORE ));
    }

    if( !bIsColDefTab ) return;	// W4W-Fehler;

    if( nTablInTablDepth )	// Tabelle / Mehrspaltigkeit IN Tabelle ?
    {
        return;
    }

    if( !bIsColMode )
    {
        return; 	// noch keine Mehrspaltigkeit ohne Tabelle
    }


#ifdef MOGEL_WW2
    if ( nDocType == 44 )   //!!! Schlechte Umpopelung eines Fehlers :
                            // Beim Import von WW2-Tabellen mit CTX, AFR in der
                            // 1. Zelle ( Spalte? ) der Tabelle wird 2mal das
                            // Attribut ausgeschaltet. Dadurch erkenne ich eine
                            // harte Attribut-Ausschaltung, was in diesem Fall
                            // nicht stimmt.
                            // Diese Umpopelung sollte verschwinden, sobald MASO
                            // den Fehler beseitigt hat, da hiermit in Tabellen
                            // die Moeglichkeit einer harten Attributausschaltung
                            // abgeklemmt wird.
        pCtrlStck->StealWWTabAttr( *pCurPaM->GetPoint() );
#endif

    // sicherstellen, dass wir keine Zelle uebersprungen haben
    if( bIsColDefTab && !bWasCellAfterCBreak )
    {
        nLastProcessedCol++;

        // Spalten-Nr. (mit Offs. 0) gegen max. Spalten-Zahl pruefen
        if( nTabCols-1 < (USHORT)nLastProcessedCol )
            return;

        SetPamInCell( nTabRow, (USHORT)nLastProcessedCol );
        SvxBoxItem aFmtBox;
        if ( bTabOwnFrm )
        {
            pTabBox->ClaimFrmFmt();
            pTabBox->GetFrmFmt()->SetAttr( SwFmtFrmSize( ATT_VAR_SIZE,
                        pTabDefs[ nLastProcessedCol ].nWidthFrac ));
        }
        USHORT nRet = Read_SetTabBorder(nTabRow,
                                        (USHORT)nLastProcessedCol,
                                        aFmtBox );

        UpdateTableMergeGroup( 0, pTabBox, (USHORT)nLastProcessedCol );

        // eigenes Format, falls noch nicht vorhanden
        if ( 0 != nRet )
            pTabBox->ClaimFrmFmt();
        // Setze evtl Umrandung
        if ( 0 != ( 0x1 & nRet ))
            pTabBox->GetFrmFmt()->SetAttr( aFmtBox );
        //	setze evtl Hintergrund
        if ( 0 != ( 0x2 & nRet ))
        {
            const Color aCol( COL_LIGHTGRAY );
            const SvxBrushItem aBack( aCol, RES_BACKGROUND );
            pTabBox->GetFrmFmt()->SetAttr( aBack );
        }
        // Override Style "Tabellenkopf", "Tabelleninhalt" mit
        // vorher gueltigem Style wenn kein Style hart gesetzt ist
        if ( !bIsSTYInTab )
        {
            const SwPosition& rPos = *pCurPaM->GetPoint();
            pCtrlStck->NewAttr( rPos,  SwW4WStyle( nTabStyleId ) );
            pCtrlStck->SetAttr( rPos, RES_FLTR_STYLESHEET );
        }
        /*
        const Color aCol( COL_LIGHTRED );
        const Brush aBrush( aCol );
        const SvxBrushItem aBack( aBrush, RES_BACKGROUND );
        pTabBox->GetFrmFmt()->SetAttr( aBack );

#ifdef TEST_BOX
        String sHlp = "forgotten Box now inserted\n\nat Row #";
        sHlp += nTabRow;
        sHlp += " Col #";
        sHlp += nLastProcessedCol;
        InfoBox(0, sHlp).Execute();
#endif
        */
    }
    ActivateTxtFlags();					// .<HCB> gilt als 'Text ist aufgetreten'

    /*
    BOOL bOldTxtInPgD = bIsTxtInPgDesc;


    if( bHeadFootDef )
        bIsTxtInPgDesc = bOldTxtInPgD;	// beruecksichtige, dass es auch Tabellen in Hd/Ft geben kann!
    */

    bIsTxtInPara        = FALSE;
    bIsSTMInPara        = FALSE;
    bIsNumListPara      = FALSE;
    bWasCellAfterCBreak = FALSE;
}




/***************************************************************************************
Fuss- und Endnoten
***************************************************************************************/


void SwW4WParser::Read_FootNoteStart(char nType, BYTE   nNoLow,
                                                 BYTE   nNoHigh,
                                                 USHORT nFootNo)
{
    if ( pCurPaM->GetPoint()->nNode < pDoc->GetNodes().GetEndOfExtras().GetIndex() )
    {
        BOOL bOldTxtInDoc = bTxtInDoc;
        BOOL bOldNExec = bNoExec;
        bTxtInDoc = FALSE;
        bNoExec = TRUE;
        bFootnoteDef = TRUE;

        while(     (!nError)
                && bFootnoteDef			    	// ignoriere Footnote-Text
                && (EOF != GetNextRecord()))	//$ EOF
            ;
        bTxtInDoc = bOldTxtInDoc;
        bNoExec = bOldNExec;
        return;
    }

    nFtnType = nType;

    const SwEndNoteInfo* pInfo;

    SwFmtFtn aFtn;

    /*
        TYP der Fuss-/Endnote ermitteln
    */
    if( 2 == nFtnType )
    {
        // End-Note
        aFtn.SetEndNote( TRUE );
        SwEndNoteInfo aInfo( pDoc->GetEndNoteInfo() );
        if( !bEndNoteInfoAlreadySet )
        {
            // am liebsten kleine, roemische Ziffern (i, ii, iii, iv...)
            aInfo.aFmt.SetNumberingType(SVX_NUM_ROMAN_LOWER);
            // ansonsten Kleinbuchstaben (a, b, c, d...)
            if(    bFtnInfoAlreadySet
                && ( SVX_NUM_ROMAN_LOWER == pDoc->GetFtnInfo().aFmt.GetNumberingType() ) )
                aInfo.aFmt.SetNumberingType(SVX_NUM_CHARS_LOWER_LETTER);
            bEndNoteInfoAlreadySet = TRUE;
        }
        pDoc->SetEndNoteInfo( aInfo );
        pInfo = &pDoc->GetEndNoteInfo();
    }
    else
    {
        // Fuss-Note
        SwFtnInfo aInfo( pDoc->GetFtnInfo() );
        // aInfo.ePos = ( nFtnType!=2 ) ? FTNPOS_PAGE : FTNPOS_CHAPTER;
        aInfo.ePos = FTNPOS_PAGE;
        if( !bFtnInfoAlreadySet )
        {
            // am liebsten arabische Ziffern (1, 2, 3, 4...)
            aInfo.aFmt.SetNumberingType(SVX_NUM_ARABIC);
            // ansonsten Kleinbuchstaben (a, b, c, d...)
            if(    bEndNoteInfoAlreadySet
                && ( SVX_NUM_ARABIC == pDoc->GetEndNoteInfo().aFmt.GetNumberingType() ) )
                aInfo.aFmt.SetNumberingType(SVX_NUM_CHARS_LOWER_LETTER);
            bFtnInfoAlreadySet = TRUE;
        }
        pDoc->SetFtnInfo( aInfo );
        pInfo = &pDoc->GetFtnInfo();
    }

    /*
        abzuziehenden Differenzwert ermitteln
    */
    BOOL bSingleLetter      = FALSE;
    BYTE nSingleLetterDelta = 0;
    if( !nFootNo )
    {
        if( SVX_NUM_CHARS_LOWER_LETTER == pInfo->aFmt.GetNumberingType() )
        {
            bSingleLetter      = TRUE;
            nSingleLetterDelta = 0x60;
        }
        else if( SVX_NUM_CHARS_UPPER_LETTER == pInfo->aFmt.GetNumberingType() )
        {
            bSingleLetter      = TRUE;
            nSingleLetterDelta = 0x40;
        }
    }

    /*
        Ueberlauf pruefen
    */
    if( bSingleLetter )
    {
        // vorlaeufiger Wert setzen
        nFootNo = nNoLow;
        if( 2 == nFtnType )
        {
            // Ende-Note
            nFootNo += 0x100 * nEndNoOverflow;
            // Ueberlaufpruefung
            if( 0xFF == ( nFootNo & 0xFF ) )
                nEndNoOverflow++;
        }
        else
        {
            // Fuss-Note
            nFootNo += 0x100 * nFootNoOverflow;
            // Ueberlaufpruefung
            if( 0xFF == ( nFootNo & 0xFF ) )
                nFootNoOverflow++;
        }
        // endgueltiger Wert
        nFootNo -= nSingleLetterDelta;
    }
    else
    {
        // gleich den richtigen Wert setzen
        nFootNo = (nNoHigh << 8) | nNoLow;
    }

    /*
        Kontinuitaet pruefen
    */
    if( 2 == nFtnType )
    {
        if(    ( USHRT_MAX != nLastReadEndNo   )
            && ( nFootNo   != nLastReadEndNo+1 ) )
        {
            bEndNoteNumberAutomatic = FALSE;
        }
        nLastReadEndNo = nFootNo;
    }
    else
    {
        // Bei Fussnoten auf Neu-Zaehlung je Seite pruefen
        if(    ( USHRT_MAX != nLastReadFootNo   )
            && ( nFootNo   != nLastReadFootNo+1 ) )
        {
            if(    (           1 == nFootNo              )
                && ( VALUE_FALSE != eFootNotePageRestart )
                && !bWasFootNoteOnW4WPage )
            {
                eFootNotePageRestart = VALUE_TRUE;
            }
            else
            {
                bFootNoteNumberAutomatic = FALSE;
                eFootNotePageRestart     = VALUE_FALSE;
            }
        }
        else
        {   // es gab *keinen* Bruch, also
            // nachschauen, ob jede/s Seite mit 1 anfaengt
            if( ( 1 != nFootNo ) && !bWasFootNoteOnW4WPage )
            {
                eFootNotePageRestart = VALUE_FALSE;
            }
        }
        bWasFootNoteOnW4WPage = TRUE;
        nLastReadFootNo       = nFootNo;
    }

    /*
        ACHTUNG: die NUMMER wird HIER hart eingesetzt.
                 Die automatische Berechnung durch den Writer wird ggfs.
                 erst am Ende von ::CallParser aktiviert, wenn dort
                 zweifelsfrei feststeht, dass die Zaehlung im gesamten
                 Dokument kontinuierlich verlief.
    */
    String sMyStr( pInfo->GetPrefix() );
    sMyStr += pInfo->aFmt.GetNumStr( nFootNo );
    sMyStr += pInfo->GetSuffix();

    aFtn.SetNumStr( sMyStr );

    pDoc->Insert( *pCurPaM, aFtn );

    // merke alte Cursorposition
    SwPosition aTmpPos( *pCurPaM->GetPoint() );

    pCurPaM->Move( fnMoveBackward, fnGoCntnt );

    SwTxtNode* pTxt = pCurPaM->GetNode()->GetTxtNode();
    SwTxtAttr* pFN = pTxt->GetTxtAttr( pCurPaM->GetPoint()->nContent, RES_TXTATR_FTN );
    ASSERT(pFN, "Probleme beim Anlegen des Fussnoten-Textes");

    const SwNodeIndex* pSttIdx = ((SwTxtFtn*)pFN)->GetStartNode();
    ASSERT(pSttIdx, "Probleme beim Anlegen des Fussnoten-Textes");

    PtNd( pCurPaM ) = pSttIdx->GetIndex() + 1;
    PtCnt( pCurPaM ).Assign( pCurPaM->GetCntntNode(), 0 );

    USHORT nOldStyleId = nAktStyleId;
    W4WCtrlStack* pOldStack = pCtrlStck;
    pCtrlStck = new W4WCtrlStack( *pOldStack, *pCurPaM->GetPoint() );

    // grosser Mist: eingeschalteter SuperScript-Modus kann die gesamte
    // Fussnote vermurksen, daher hier auf jeden Falle ausschalten ...
    pCtrlStck->SetAttr( *pCurPaM->GetPoint(), SVX_ESCAPEMENT_OFF );

    /*
        lies Footnote-Text ein, dabei ersten, fuehrenden TAB uebergehen
    */
    bFootnoteDef  = TRUE;
    bIsTxtInFNote = FALSE;
    while(!nError && bFootnoteDef
            && (EOF!=GetNextRecord()))
        ;

    // UEble Sache: W4W liefert manchmal ein ueberfluessiges
    // .<HNL> am Ende der Fussnoten!
    // Hilfskonstruktion: Wir pruefen, ob der letzte Absatz
    // der Fussnote Text enthaelt und entfernen ihn andernfalls:
    if( !bIsTxtInPara )
        JoinNode( pCurPaM );

    DeleteZStk( pCtrlStck );
    pCtrlStck   = pOldStack;
    nAktStyleId = nOldStyleId;
    pCtrlStck->SetEndForClosedEntries( aTmpPos );
    *pCurPaM->GetPoint() = aTmpPos;
}


void SwW4WParser::Read_FootNoteStart1()			// (FNT)
{
    BYTE nDef;
    BYTE nNumberHi, nNumberLo;

    if( GetHexByte( nDef ) && !nError && GetHexByte( nNumberHi ) && !nError
        && GetHexByte( nNumberLo ) && !nError )
    {
        SkipEndRecord();		// restliche Parameter interessieren nicht
        Read_FootNoteStart( nDef, nNumberLo, nNumberHi, 0 );
    }
}


void SwW4WParser::Read_FootNoteStart2()			// (FTN)
{
    long nNumber;

    if( GetDecimal( nNumber ) && !nError)
    {
        SkipEndRecord(); 		// restliche Parameter interessieren nicht
        Read_FootNoteStart( 0, 0, 0, (USHORT)nNumber);
    }
}


void SwW4WParser::Read_FootNoteEnd()			// (EFN, EFT)
{
    bFootnoteDef = FALSE;
}


void SwW4WParser::SetFtnInfoIntoDoc(BOOL bEndNote,
                                    BYTE nRestart,
                                    BYTE nStyle,
                                    String& rPrefixTxt,
                                    String& rSuffixTxt )
{
    static SvxExtNumType __READONLY_DATA aNumArr[]
            = { SVX_NUM_ARABIC, SVX_NUM_ARABIC, SVX_NUM_CHARS_LOWER_LETTER, SVX_NUM_CHARS_UPPER_LETTER,
                SVX_NUM_ROMAN_LOWER,        SVX_NUM_ROMAN_UPPER };
    if( bEndNote )
    {
        // End-Noten
        SwEndNoteInfo aInfo;
        aInfo = pDoc->GetEndNoteInfo();
        if (nStyle <= 5)
            aInfo.aFmt.SetNumberingType(aNumArr[ nStyle ]);
        // sicherstellen, dass Typ anders als Fussnoten-Typ
        if(    bFtnInfoAlreadySet
            && ( aInfo.aFmt.GetNumberingType() == pDoc->GetFtnInfo().aFmt.GetNumberingType() ) )
        {
            if( SVX_NUM_ROMAN_LOWER == aInfo.aFmt.GetNumberingType() )
                aInfo.aFmt.SetNumberingType(SVX_NUM_CHARS_LOWER_LETTER);
            else
                aInfo.aFmt.SetNumberingType(SVX_NUM_ROMAN_LOWER);
        }
        aInfo.SetPrefix( rPrefixTxt );
        aInfo.SetSuffix( rSuffixTxt );
        pDoc->SetEndNoteInfo( aInfo );
        bEndNoteInfoAlreadySet = TRUE;
    }
    else
    {
        // Fuss-Noten
        SwFtnInfo aInfo;
        aInfo = pDoc->GetFtnInfo();
        aInfo.ePos = nRestart ? FTNPOS_PAGE : FTNPOS_CHAPTER;
        if (nStyle <= 5)
            aInfo.aFmt.SetNumberingType(aNumArr[ nStyle ]);
        // sicherstellen, dass Typ anders als Endnoten-Typ
        if(    bEndNoteInfoAlreadySet
            && ( aInfo.aFmt.GetNumberingType() == pDoc->GetEndNoteInfo().aFmt.GetNumberingType() ) )
        {
            if( SVX_NUM_ARABIC == aInfo.aFmt.GetNumberingType() )
                aInfo.aFmt.SetNumberingType(SVX_NUM_CHARS_LOWER_LETTER);
            else
                aInfo.aFmt.SetNumberingType(SVX_NUM_ARABIC);
        }
        aInfo.SetPrefix( rPrefixTxt );
        aInfo.SetSuffix( rSuffixTxt );
        pDoc->SetFtnInfo( aInfo );
        bFtnInfoAlreadySet = TRUE;
    }
}


void SwW4WParser::Read_FootNoteInfo()			// (FNI)
{
    BYTE nDef;
    BYTE nRestart, nStyle;

    if( GetHexByte( nDef ) && !nError && GetHexByte( nRestart ) && !nError
        && GetHexByte( nStyle ) && !nError )
    {
        String aPrefixTxt;
        String aSuffixTxt;
        if( GetString( aPrefixTxt, W4WR_TXTERM, W4WR_TXTERM ) )
            GetString( aSuffixTxt, W4WR_TXTERM, W4WR_TXTERM );
        SetFtnInfoIntoDoc(  ( 2 == nDef ),
                            nRestart,
                            nStyle,
                            aPrefixTxt,
                            aSuffixTxt );
    }
}



void SwW4WParser::UpdateCacheVars()
{
#if 0
    const SwFrmFmt &rFmt = pPageDesc->GetMaster();
    ASSERT( pPageDesc->GetMaster().GetFrmSize().GetFixSize() != LONG_MAX ,
            "Seitenbreite (Master) falsch" );
    ASSERT( pPageDesc->GetLeft().GetFrmSize().GetFixSize() != LONG_MAX ,
            "Seitenbreite (Left) falsch" );
    ASSERT( pPageDesc->GetMaster().GetFrmSize().GetVarSize() != LONG_MAX ,
            "Seitenlaenge (Master) falsch" );
    ASSERT( pPageDesc->GetLeft().GetFrmSize().GetVarSize() != LONG_MAX ,
            "Seitenlaenge (Left) falsch" );
#else
    SwFrmFmt *pFmt = &pPageDesc->GetLeft();
    SwFmtFrmSize aSz( pFmt->GetFrmSize() );
    BOOL bSet = FALSE;
    if (( aSz.GetWidth() > 2 * lA3Height )			// Kein Drucker
        ||( aSz.GetWidth() <  lA5Width / 2 ))		// Kein Drucker
    {
        aSz.SetWidth( lA4Width );
        bSet = TRUE;
    }

    if (( aSz.GetHeight() > 2 * lA3Height )			// Kein Drucker
       ||( aSz.GetHeight() < lA5Width / 2 ))		// Kein Drucker
    {						 						// Dann A4
        aSz.SetHeight( lA4Height );
        bSet = TRUE;
    }
    if( bSet )
        pFmt->SetAttr( aSz );

    pFmt = &pPageDesc->GetMaster();
    aSz = pFmt->GetFrmSize();
    bSet = FALSE;
    if (( aSz.GetWidth() > 2 * lA3Height )			// Kein Drucker
        ||( aSz.GetWidth() <  lA5Width / 2 ))		// Kein Drucker
    {
        aSz.SetWidth( lA4Width );
        bSet = TRUE;
    }

    if (( aSz.GetHeight() > 2 * lA3Height )			// Kein Drucker
       ||( aSz.GetHeight() < lA5Width / 2 ))		// Kein Drucker
    {						 						// Dann A4
        aSz.SetHeight( lA4Height );
        bSet = TRUE;
    }
    if( bSet )
        pFmt->SetAttr( aSz );

#endif
    const SvxLRSpaceItem& rPageLR = pFmt->GetLRSpace();
    nPgLeft       = (USHORT)(rPageLR.GetTxtLeft() + nLeftMgnCorr);
    nPgRightDelta = (USHORT)( rPageLR.GetRight() );
    nPgRight      = (USHORT)( aSz.GetWidth() - rPageLR.GetRight() );
    //!! nRSMRight + Left auch Updaten ????
}


/***************************************************************************************
Kopf- und Fusszeilen
***************************************************************************************/

#define W4W_EVEN	   0x1		// linke Seiten
#define W4W_ODD 	   0x2		// rechte Seiten
#define W4W_EVENODD    0x3      // alle Seiten == EVEN | ODD
#define W4W_MASK1 	   0x3		// Maske fuer links/rechts
#define W4W_FIRST	  0x10		// 1. Seite ?
#define W4W_NFIRST	  0x20      // Vorsicht bei AEndern dieser Konstanten:
#define W4W_ALL		  0x30		// FIRST | NFIRST == ALL
#define W4W_MASK2	  0x30		// Maske fuer erste / andere / alle Seiten
#define W4W_HEADER   0x100
#define W4W_FOOTER   0x200
#define W4W_FOOTNOTE 0x300
#define W4W_MASK3 	 0x300		// Maske fuer Header / Footer / Footnote
#define W4W_NO2		0x1000		// Hd/Ft #2 aus W4W-Datei


SwFrmFmt* lcl_GetMasterLeft( SwPageDesc& rPgDsc, USHORT nType )
{
    SwFrmFmt* pFmt = 0;
    switch ( nType & W4W_MASK1 ){
    case W4W_EVENODD: pFmt = &rPgDsc.GetMaster();
                      break;
    case W4W_EVEN:    pFmt = &rPgDsc.GetLeft();
                      break;
    case W4W_ODD:     pFmt = &rPgDsc.GetMaster();
                      break;
    }
    ASSERT( 0 != pFmt, "header/footer ohne entsprechendes Format im PgDesc" );

    return pFmt;
}


void lcl_GetMstLftHdFt( SwPageDesc& rPgDsc, USHORT nType,
                        BOOL bDoTheHeader,
                        SwFrmFmt*& rpMasterLeft,
                        SwFrmFmt*& rpHdFtFmt )
{
    rpMasterLeft = lcl_GetMasterLeft( rPgDsc, nType );
    if( bDoTheHeader )
        rpHdFtFmt = (SwFrmFmt*)rpMasterLeft->GetHeader().GetHeaderFmt();
    else
        rpHdFtFmt = (SwFrmFmt*)rpMasterLeft->GetFooter().GetFooterFmt();
}


BOOL SwW4WParser::ContinueHdFtDefinition( BOOL bFollow,
                                          long nLMarg,
                                          long nRMarg )
{
    BOOL        bDoCreatePageDesc;
    BOOL        bRet          = FALSE;
    SwFrmFmt*   pMasterLeft   = 0;
    SwFrmFmt*   pHdFtFmt      = 0;
    SwPageDesc* pThisPageDesc = 0;
    UseOnPage   eOldHdFtShare = PD_NONE;

    BOOL    bDoTheHeader  = ( W4W_HEADER == (nHdFtType & W4W_MASK3) );
    ASSERT( bDoTheHeader != ( W4W_FOOTER == (nHdFtType & W4W_MASK3) ),
            "Hier muss Header ODER Footer definiert werden" );

    // something special:
    //
    // when a Follow was allready defined
    // and now we are to define a START-PgDesc
    // we will use the old(!) Start-PgDesc
    // that was active before the definition of it's Follow
    //
    if(    pOpen1stPgPageDesc
        && ( nPDType == W4W_FIRST )
        && !bIsTxtInPgDesc )
    {
        pThisPageDesc = pOpen1stPgPageDesc;
        lcl_GetMstLftHdFt( *pThisPageDesc, nHdFtType,
                            bDoTheHeader,
                            pMasterLeft,   pHdFtFmt );
        /*
         * Do NOT create a new PgDesc !!!
         */
        bDoCreatePageDesc = FALSE;
    }
    else
    {
        pThisPageDesc = pPageDesc;
        // Let's see if the current Page-Descriptor
        // has got a Header/Footer of this type
        lcl_GetMstLftHdFt( *pThisPageDesc, nHdFtType,
                            bDoTheHeader,
                            pMasterLeft,   pHdFtFmt );

        /*
         * in case of PageDesc still waiting for next chance
         * use this one and do NOT create a new PgDesc !!!
         */
        if( pPageDescForNextHNP )
            bDoCreatePageDesc = FALSE;
        else
        if(    pOpen1stPgPageDesc
            && ( 33 == nDocType ) )
            /*
             * Do NOT create a new PgDesc !!!
             */
            bDoCreatePageDesc = FALSE;
        else
            /*
             * We need a new PgDesc, when there's allready text in the current
             * PgDesc and a Header/Footer of the same kind is also defined.
             */
            bDoCreatePageDesc = (    (     bIsTxtInPgDesc
                                        && pHdFtFmt )
                                  || (     bFollow
                                        && !pOpen1stPgPageDesc ) );
    }

    UseOnPage eOldUse = pThisPageDesc->ReadUseOn();
    if( bDoTheHeader )
        eOldHdFtShare = (UseOnPage)
                        (eOldHdFtShare | ( eOldUse & PD_FOOTERSHARE ));
    else
        eOldHdFtShare = (UseOnPage)
                        (eOldHdFtShare | ( eOldUse & PD_HEADERSHARE ));

    // create [and insert] the new Page Descriptor
    if( bDoCreatePageDesc )
    {
        USHORT eCreateMode = 0;

        if( bFollow )
            eCreateMode |= CRPGD_AS_FOLLOW;

        // Should the new PgDesc be instantly inserted into the pDoc ?
        if( bIsTxtInPgDesc && !bFollow)
            eCreateMode |= CRPGD_AND_INSERT;

        if( bDoTheHeader )
            eCreateMode |= CRPGD_REMOVE_HD;
        else
            eCreateMode |= CRPGD_REMOVE_FT;


        if(    bFollow
            && !(eCreateMode & CRPGD_AND_INSERT)
            && !pOpen1stPgPageDesc )
        {
            pOpen1stPgPageDesc = pThisPageDesc;
        }

        BOOL bAmiProHdFtMustWait = ( 33 == nDocType );
        if ( bAmiProHdFtMustWait )
        {
            // with AmiPro NEVER insert Hd/Ft instantly into the pDoc
            // but store the PgDesc elswhere and insert it
            // when the next .<HNP> occurs ( see W4WPAR1.CXX )
            eCreateMode &= ~CRPGD_AND_INSERT;
        }

        CreatePageDesc( eCreateMode );

        // Attention: pPageDesc points NOW to the newly created PgDesc
        //            no matter if it was inserted into the pDoc or not
        if( bAmiProHdFtMustWait )
        {
            pPageDescForNextHNP = pPageDesc;
        }
        pThisPageDesc = pPageDesc;
        lcl_GetMstLftHdFt( *pThisPageDesc, nHdFtType,
                            bDoTheHeader,
                            pMasterLeft,   pHdFtFmt );
        bRet = TRUE;
    }

    // create HEADER
    if( bDoTheHeader )
    {
        bWasTxtSinceLastHF2 = FALSE;
        // if old header exists we remove it to erase it's content section
        if( pHdFtFmt )
            pMasterLeft->ResetAttr( RES_HEADER );
        // our new header
        pMasterLeft->SetAttr( SwFmtHeader( TRUE ));
        pHdFtFmt = (SwFrmFmt*)pMasterLeft->GetHeader().GetHeaderFmt();

    }
    // create FOOTER
    else
    {
        bWasTxtSinceLastHF1 = FALSE;
        // if old footer exists we remove it to erase it's content section
        if( pHdFtFmt )
            pMasterLeft->ResetAttr( RES_FOOTER );
        // our new footer
        pMasterLeft->SetAttr( SwFmtFooter( TRUE ));
        pHdFtFmt = (SwFrmFmt*)pMasterLeft->GetFooter().GetFooterFmt();
    }

    //JP 29.5.2001: Bug #74471#:
    // "share" the content of the header / footer of the first and the
    // following pagedesc.
    BOOL bShareHdFtCntntOfFirstPgDsc = FALSE;
    if( pOpen1stPgPageDesc && W4W_EVENODD == (nHdFtType & W4W_MASK1) )
    {
        SwFrmFmt* pTmp = &pOpen1stPgPageDesc->GetMaster();
        if( SFX_ITEM_SET != pTmp->GetItemState(
                    bDoTheHeader ? RES_HEADER : RES_FOOTER ))
        {
            if( bDoTheHeader )
                pTmp->SetAttr( SwFmtHeader( TRUE ));
            else
                pTmp->SetAttr( SwFmtFooter( TRUE ));
            bShareHdFtCntntOfFirstPgDsc = TRUE;
        }
    }

    // Define the PgDesc's eUse flag: PD_ALL with/without PD_xxSHARE
    UseOnPage eUse = PD_ALL;
    if( ( nHdFtType & W4W_MASK1 ) == W4W_EVENODD )
    {
        if( bDoTheHeader )
            eUse = (UseOnPage)(eUse | PD_HEADERSHARE);
        else
            eUse = (UseOnPage)(eUse | PD_FOOTERSHARE);
    }
    pThisPageDesc->WriteUseOn( (UseOnPage)(eUse | eOldHdFtShare) );


#ifdef HDFT_MARGIN
    /*
     * Raender gemaess rudimentaeren Angaben aus w4w-File setzen
     * wird meistens ungenau, da Angabe in Spalten (d.h. in 10tel Inch)
     * erst neuerdings gibt W4W uns hier Twips durch
     * (Werte werden unten in Read_HdFtDefinition() hochgerechnet)
     */
    long nHdFtLeft;
    long nHdFtRight;

    // die W4W-Angaben gelten ab Seitenrand !!
    const SvxLRSpaceItem& rPageLR = pMasterLeft->GetLRSpace();

    nHdFtLeft  = nLMarg - rPageLR.GetLeft();			// Twips

    nHdFtRight =   pMasterLeft->GetFrmSize().GetWidth()	// Twips
                 - rPageLR.GetRight()                   // Twips
                 - nRMarg;                              // Twips

    if ( nHdFtLeft  < 0 ) nHdFtLeft  = 0;
    if ( nHdFtRight < 0 ) nHdFtRight = 0;

    SvxLRSpaceItem aLR( pHdFtFmt->GetLRSpace() ); 		// LR-Rand

    aLR.SetLeft(  USHORT( nHdFtLeft  ) );
    aLR.SetRight( USHORT( nHdFtRight ) );

    pHdFtFmt->SetAttr( aLR );
#endif // HDFT_MARGIN

    if( bDoTheHeader )
        UpdatePageMarginSettings( CALLED_BY_HF2 );
    else
        UpdatePageMarginSettings( CALLED_BY_HF1 );

    const SwNodeIndex* pSttIdx = pHdFtFmt->GetCntnt().GetCntntIdx();
    if( !pSttIdx ) return bRet;

    USHORT nOldStyleId = nAktStyleId;
    W4WCtrlStack* pOldStack = pCtrlStck;

    // alte Cursorposition merken
    SwPosition aTmpPos( *pCurPaM->GetPoint() );

    PtNd( pCurPaM ) = pSttIdx->GetIndex() + 1;
    PtCnt( pCurPaM ).Assign( pCurPaM->GetCntntNode(), 0 );

    if(    ( nIniFlags && W4WFL_ALL_HDFT_MAINATTR )
        || (    ( nDocType == 15)
            && !( nIniFlags & W4WFL_NO_PCTEXT4_HDFT_MAINATTR ) ) )
        pCtrlStck = new W4WCtrlStack( *pOldStack, *pCurPaM->GetPoint() );
    else
        pCtrlStck = new W4WCtrlStack( *this );

    bHeadFootDef = TRUE;

    // K/F-Text einlesen
    while (    !nError
            && bHeadFootDef
            && ( EOF != GetNextRecord() ) )
    {
        ;//NOP
    }

#ifdef MOGEL_WW2
    if ( !(nIniFlags & W4WFL_NO_APO_HNL_REMOVE) && nDocType == 44
         && !bIsTxtInPara )
    {
        const SwNode* pNd = pCurPaM->GetNode();
        if( 2 < ( pNd->EndOfSectionIndex() - pNd->StartOfSectionIndex() ))
        {	// Bit 4 : HNL nicht klauen, bevor die Attribute gesetzt sind
            pCtrlStck->StealAttr( *pCurPaM->GetPoint() );
            JoinNode( pCurPaM );
        }
    }
#endif

    DeleteZStk( pCtrlStck );
    pCtrlStck   = pOldStack;
    nAktStyleId = nOldStyleId;
    pCtrlStck->SetEndForClosedEntries( aTmpPos );
    *pCurPaM->GetPoint() = aTmpPos;

    if(	bShareHdFtCntntOfFirstPgDsc )
    {
        if( bDoTheHeader )
            pDoc->CopyHeader( pThisPageDesc->GetMaster(),
                              pOpen1stPgPageDesc->GetMaster() );
        else
            pDoc->CopyFooter( pThisPageDesc->GetMaster(),
                              pOpen1stPgPageDesc->GetMaster() );
    }

    return bRet;
}


void SwW4WParser::Read_HdFtDefinition( BOOL bHeader )
{
    BOOL bFollow = FALSE;
    BYTE nOldDef;
    long nOldLines, nLMarg, nRMarg, nPos, nLinePos;

    if(    GetHexByte( nOldDef )    && !nError
        && GetDecimal( nOldLines )  && !nError
        && GetDecimal( nLMarg )     && !nError
        && GetDecimal( nRMarg )     && !nError
        && GetDecimal( nPos )       && !nError
        && GetDecimal( nLinePos )   && !nError )
    {
        long nHdFtUl = nLinePos * 240;		// Grundlage: Zeilenabstand 6 LPI
                                            // stimmt so zumindest fuer WW2
        // was tun wir mit diesem Wert?
        // vielleicht sollten wir nHdFtUl
        // ggfs. in nNewValueHTM umwandeln ???

        // gibt es die optionale Angabe ?
        long nOptNew1, nOptNew2;
        if( W4WR_TXTERM == GetDecimal( nOptNew1 ) && !nError &&
            W4WR_TXTERM == GetDecimal( nOptNew2 ) && !nError )
        {
            nLMarg = nOptNew1; 		// in Twip
            nRMarg = nOptNew2;
        }else
        {
            nLMarg *= nColSize;		// in 1/10 Inch / ??
            nRMarg *= nColSize;
        }

        USHORT nOldPDT = nPDType;

        Flush();

        ULONG nOldPos = rInp.Tell();	// merke FilePos
        // ========================

        BOOL bOldTxtInDoc=bTxtInDoc;
        BOOL bOldNoExec=bNoExec;
        BYTE nOldErr=nError;

        bTxtInDoc=FALSE;
        bHeadFootDef = TRUE;
        bNoExec = TRUE;

        // Suche das Ende der Kopf-/Fusstext-Definition  (HFX)
        // und lies die Parameter
        // siehe:  SwW4WParser::Read_HeadFootEnd()
        while (!nError && bHeadFootDef
                && (EOF!=GetNextRecord()))
            ;

        Flush();
        nError=nOldErr;
        bTxtInDoc=bOldTxtInDoc;
        bNoExec = bOldNoExec;

        if (( nHdFtType & W4W_MASK1 ) == 0 )	// WP 5.1 : Kopf/Fusstext
        {                                       // ist ausgeschaltet
            return;
        }

        rInp.Seek( nOldPos );				// Spule zurueck an den Anfang der
                                            // Kopf/Fusstexte, um den K/F-Text zu lesen

        // Falls im HFX-Befehl Mist steht, hier berichtigen:
        nHdFtType &= ~W4W_MASK3;				// Weder Header noch Footer
        if ( bHeader ) nHdFtType |= W4W_HEADER;	// Header oder Footer ??
        else 		   nHdFtType |= W4W_FOOTER;

        SwPageDesc* pOldPgD = pPageDesc;

        // Falls bereits ein Follow definiert wurde, ist nOldPDT inzwischen
        // ungueltig, wird aber auch nicht mehr benoetigt, da wir ja am
        // gesetzten pOpen1stPgPageDesc erkennen, dass der Vorgaenger-PgDesc
        // nur fuer Startseiten gilt und der Follow-PgDesc noch nicht ins
        // pDoc inserted wurde.
        //
        // Was bedeutet nun bFollow?
        //
        // "Folge-PgDesc des derzeit tatsaechlich aktiven Page-Descriptors"
        // also nicht immer Folge von pPageDesc sondern gegebenenfalls
        // halt Folge von pOpen1stPgPageDesc.
        //
        bFollow = (    (    ( pOpen1stPgPageDesc   )
                         || ( nOldPDT == W4W_FIRST )
                         || ( nOldPDT == 0 )
                       )
                    && ( nPDType == W4W_NFIRST ) );
        /*
         * Lies K/F-Zeilen ein, mache evtl neuen PageDesc
         */
        BOOL bNewPD = ContinueHdFtDefinition( bFollow, nLMarg, nRMarg );

        if( bFollow && bNewPD )
        {
            pOldPgD->SetFollow( pPageDesc );
            // Spaeter neu kreierte Pagedescs
            // bekommen die Header / Footer vom Follow
            pLastActPageDesc = pPageDesc;
        }
    }
}


void SwW4WParser::Read_FooterStart()			// (HF1)
{
    Read_HdFtDefinition( FALSE );
}


void SwW4WParser::Read_HeaderStart()			// (HF2)
{
    Read_HdFtDefinition( TRUE );
}


void SwW4WParser::Read_HeadFootEnd()			// (HFX)
{
    long nNewLines, nOptHeight=0, nOptFirstP=0;
    BYTE nNewDef;

    if( bNoExec )		// das Auswerten nur beim 1. Durchlauf reicht aus
    {
        if(    GetDecimal( nNewLines ) && !nError
            && GetHexByte( nNewDef )   && !nError )
        {
            // Fehlen optionale Parameter ?
            if(    (W4WR_TXTERM != GetDecimal( nOptHeight )) || nError
                || (W4WR_TXTERM != GetDecimal( nOptFirstP )) || nError )
            {
                // Default: alle Seiten
                nOptFirstP = 3;
            }
            if (0==nOptFirstP) nOptFirstP=3;

            nPDType = nHdFtType = 0;

            // Test auf Header/Footer No. 2
            if (nNewDef & 0x1) nHdFtType |= W4W_NO2;

            // Entscheidung, ob Header oder Footer
            if (nNewDef & 0x2) nHdFtType |= W4W_FOOTER;
            else               nHdFtType |= W4W_HEADER;

            // auf welchen Seiten solls erscheinen (gerade/ungerade)
                 if( nNewDef & 0x4  ) nHdFtType |= W4W_EVENODD;
            else if( nNewDef & 0x8  ) nHdFtType |= W4W_ODD;
            else if( nNewDef & 0x10 ) nHdFtType |= W4W_EVEN;

            // auf welchen Seiten solls erscheinen (erste/folgende)
            switch( nOptFirstP )
            {
            case 0:
            case 3: nHdFtType |= W4W_ALL;
                    nPDType    = W4W_ALL;
                    break;
            case 1: nHdFtType |= W4W_FIRST;
                    nPDType    = W4W_FIRST;
                    break;
            case 2: nHdFtType |= W4W_NFIRST;
                    nPDType    = W4W_NFIRST;
                    break;
            }
        }
    }
    bHeadFootDef=FALSE;
}

/***************************************************************************
*     Hilfsroutinen fuer W4WPAR1-Formatierungen und Kopf- / Fusszeilen     *
***************************************************************************/


SwPageDesc* SwW4WParser::CreatePageDesc( USHORT eCreateMode )
{
    USHORT     nPos;
    UseOnPage  eOldHdFtShare  = PD_NONE;
    const BOOL bRemoveAllHdFt =    (eCreateMode & CRPGD_REMOVE_HD)
                                && (eCreateMode & CRPGD_REMOVE_FT);

    /*
     * If PageDesc follows to SvxFmtBreakItem( SVX_BREAK_PAGE_BEFORE )
     * that Break-Item must be erased.
     */
    if( (eCreateMode & CRPGD_AND_INSERT) && !bWasTxtSince_BREAK_PAGE )
    {
        SfxItemSet* pSet = pCurPaM->GetCntntNode()->GetpSwAttrSet();
        if(    pSet
            && (SFX_ITEM_SET == pSet->GetItemState( RES_BREAK, FALSE )) )
        {
            // PageBreak was set in same paragraph (same Content-Node),
            // and is to be deleted now to avoid creating an empty page.
            pSet->ClearItem( RES_BREAK );
        }
    }

    if( bIsTxtInPgDesc && (eCreateMode & CRPGD_AND_INSERT) )
        DoSplit();


    /*
     *  Now create new Page-Descriptor:
     *
     *  copy information of Last Active PageDesc (without Header/Footer)
     */
    nPos = pDoc->MakePageDesc( ViewShell::GetShellRes()->GetPageDescName(
                                       pDoc->GetPageDescCnt(), FALSE,
                                       eCreateMode & CRPGD_AS_FOLLOW ),
                                pLastActPageDesc, FALSE );

    pPageDesc = &((SwPageDesc&)pDoc->GetPageDesc( nPos ));

    /*
     *  Now copy header/footer info
     *  avoiding the info of the header/footer that is to be removed.
     */
    if( pLastActPageDesc )
    {
        // store old HdFtShare value
        UseOnPage eOldUse = pLastActPageDesc->ReadUseOn();

        // copy even header content section
        if( ! (    (eCreateMode & CRPGD_REMOVE_HD)
                && (nHdFtType   & W4W_EVEN) ) )
        {
            pDoc->CopyHeader( pLastActPageDesc->GetMaster(),
                              pPageDesc->GetMaster() );
        }
        // copy even footer content section
        if( ! (    (eCreateMode & CRPGD_REMOVE_FT)
                && (nHdFtType   & W4W_EVEN) ) )
        {
            pDoc->CopyFooter( pLastActPageDesc->GetMaster(),
                              pPageDesc->GetMaster() );
        }
        // copy odd header content section
        if( ! (    (eCreateMode & CRPGD_REMOVE_HD)
                && (nHdFtType   & W4W_ODD) ) )
        {
            if( eOldUse & PD_HEADERSHARE )
                pPageDesc->GetLeft().SetAttr(
                    pLastActPageDesc->GetMaster().GetHeader() );
             else
                pDoc->CopyHeader( pLastActPageDesc->GetLeft(),
                                  pPageDesc->GetLeft() );
        }
        // copy odd footer content section
        if( ! (    (eCreateMode & CRPGD_REMOVE_FT)
                && (nHdFtType   & W4W_ODD) ) )
        {
            if( eOldUse & PD_FOOTERSHARE )
                pPageDesc->GetLeft().SetAttr(
                    pLastActPageDesc->GetMaster().GetFooter() );
             else
                pDoc->CopyFooter( pLastActPageDesc->GetLeft(),
                                  pPageDesc->GetLeft() );
        }

        // Look which old UseOn values shall continue
        if( !(eCreateMode & CRPGD_REMOVE_HD) )
            eOldHdFtShare = (UseOnPage)
                            (eOldHdFtShare | ( eOldUse & PD_HEADERSHARE ));
        if( !(eCreateMode & CRPGD_REMOVE_FT) )
            eOldHdFtShare = (UseOnPage)
                            (eOldHdFtShare | ( eOldUse & PD_FOOTERSHARE ));
    }
    else
    {
        USHORT nMinTopBottom, nMinLeftRight;
        if ( MEASURE_METRIC == GetAppLocaleData().getMeasurementSystemEnum() )
            nMinTopBottom = nMinLeftRight = 1134;	//2 Zentimeter
        else
        {
            nMinTopBottom = 1440;	//al la WW: 1Inch
            nMinLeftRight = 1800;	//			1,25 Inch
        }
        //SvxULSpaceItem aUL( nMinTopBottom, nMinTopBottom );
        pPageDesc->GetMaster().SetAttr( SvxULSpaceItem( nMinTopBottom, nMinTopBottom ) );
        pPageDesc->GetMaster().SetAttr( SvxLRSpaceItem( nMinLeftRight, nMinLeftRight ) );
    }

    if( eCreateMode & CRPGD_AND_INSERT )
    {
        pLastActPageDesc   = pPageDesc;
        pOpen1stPgPageDesc = 0;
    }

    if( bRemoveAllHdFt )
    {
        pPageDesc->GetMaster().ResetAttr( RES_HEADER, RES_FOOTER );
        pPageDesc->GetLeft  ().ResetAttr( RES_HEADER, RES_FOOTER );
    }
    else if( (eCreateMode & (CRPGD_REMOVE_HD | CRPGD_REMOVE_FT)) )
    {
        /*
         * Find out which Hd or Ft is to be removed
         */
        SwFrmFmt* pMasterLeft = lcl_GetMasterLeft( *pPageDesc, nHdFtType );

        if( eCreateMode & CRPGD_REMOVE_HD )
            pMasterLeft->ResetAttr( RES_HEADER );
        else
            pMasterLeft->ResetAttr( RES_FOOTER );
    }

    pPageDesc->WriteUseOn( (UseOnPage)(PD_ALL | eOldHdFtShare) );

    nAktPgDesc = nPos;	  	// merke als aktuelles SeitenLayout

    // den Follow-PageDesc immer auf sich selbst setzen
    pPageDesc->SetFollow( pPageDesc );

    // ggfs. den neuen PgDesc-Attr ins Doc setzen
    if( eCreateMode & CRPGD_AND_INSERT )
        pDoc->Insert( *pCurPaM, SwFmtPageDesc( &pDoc->GetPageDesc( nPos )));

    /* Der neue Page-Descriptor ist noch ohne Text

        2 Faelle:
            a.  Page-Descriptor wurde soeben aktiviert (s. vorige Anweisung)
            b.  PgDesc greift erst mit naechstem Seitenumbruch
        Beide Male ist sie in pPageDesc gespeichert.
        Daher duerfen wir das bool-Flag getrost auf FALSE setzen:
        alle Abfragen darauf stehen naemlich IMMER in Zusammenhang mit
        Zugriffen auf die Variable pPageDesc
    */
    bIsTxtInPgDesc = FALSE;


    // Up to now there was no .<HNP> within this Page-Descriptor's scope.
    // ( We need this information in SwW4WParser::Read_HardNewPage() )
    bWasHNPInPgDesc = FALSE;

    if(    (eCreateMode & CRPGD_UPDT_MRGN)
        && bPgMgnChanged )
        SetPageMgn();			       // neue Raender gueltig

    UpdateCacheVars();

    return pPageDesc;
}


}
