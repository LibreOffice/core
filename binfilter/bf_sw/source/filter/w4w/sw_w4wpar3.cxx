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

#include <stdlib.h>
#include <stdio.h>

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
#ifndef _SVX_SPLTITEM_HXX //autogen
#include <bf_svx/spltitem.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX //autogen
#include <bf_svx/langitem.hxx>
#endif
#ifndef _SVX_KERNITEM_HXX //autogen
#include <bf_svx/kernitem.hxx>
#endif
#ifndef _SVX_HYZNITEM_HXX //autogen
#include <bf_svx/hyznitem.hxx>
#endif
#ifndef _SVX_WIDWITEM_HXX //autogen
#include <bf_svx/widwitem.hxx>
#endif
#ifndef _SVX_ORPHITEM_HXX //autogen
#include <bf_svx/orphitem.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <bf_svx/unolingu.hxx>
#endif
#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif

#ifndef _FMTHDFT_HXX //autogen
#include <fmthdft.hxx>
#endif
#ifndef _FMTCOL_HXX //autogen
#include <fmtcol.hxx>
#endif
#ifndef _DOCUFLD_HXX		// fuer Hidden Text
#include <docufld.hxx>
#endif
#ifndef _FMTFLD_HXX			// fuer Hidden Text
#include <fmtfld.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>		// GET_POOLID_TXTCOLL
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>		// RES_POOLCOLL_STANDARD
#endif
#ifndef _W4WSTK_HXX
#include <w4wstk.hxx>			// fuer den Attribut Stack
#endif
#ifndef _W4WPAR_HXX
#include <w4wpar.hxx>			// GetDecimal()
#endif
#ifndef _SWSTYLENAMEMAPPER_HXX
#include <SwStyleNameMapper.hxx>
#endif
namespace binfilter {


SV_IMPL_PTRARR( W4WPDTInfos, W4WPDTInfo_Ptr )
            // fuer PDT beim FrameMaker

SV_IMPL_PTRARR( W4WFLOInfos, W4WFLOInfo_Ptr )
            // fuer FLO beim FrameMaker

SV_DECL_PTRARR_SORT( W4WStyleIdTab, W4WStyleIdTabEntryPtr, 0, 1 )
            // Implementation der W4WStyleIdTab ist in w4wpar1.cxx




// -------- Methoden fuer die StyleSheet-Verarbeitung: ------------


W4WStyleIdTabEntry::W4WStyleIdTabEntry(	SwW4WParser& rParser, USHORT nId,
                                        const sal_Unicode* pName )
    :  nStyleId( nId ), pColl( 0 ), bSetAttributes( TRUE )
{
    String aName( pName );

    if( !aName.Len() )
    {
        do
        {
            rParser.MakeUniqueStyleName( aName );
            pColl = rParser.GetDoc().FindTxtFmtCollByName( aName );
        } while( pColl );
    }
    else
        pColl = rParser.GetDoc().FindTxtFmtCollByName( aName ); // Suche zuerst in den Doc-Styles

    if( !pColl )           // Collection nicht gefunden, vielleicht im Pool ?
    {
        USHORT n = SwStyleNameMapper::GetPoolIdFromUIName( aName , GET_POOLID_TXTCOLL );
        if ( nId == USHRT_MAX )		// USHRT_MAX bedeutet Standard
                                    // z.B. Nr. 0 bei WinWord2
            n = RES_POOLCOLL_STANDARD;
        if ( n != USHRT_MAX )		// gefunden oder Standard
            pColl = rParser.GetDoc().GetTxtCollFromPoolSimple( n, FALSE );
    }
    SwTxtFmtColl* pStandardColl = 
        rParser.GetDoc().GetTxtCollFromPoolSimple( RES_POOLCOLL_STANDARD, 
                                                   FALSE );
    if( !pColl )
    {
        // die Collection nicht gefunden, also erzeuge neue
        pColl = rParser.GetDoc().MakeTxtFmtColl( aName, pStandardColl );
    }
    else
    {
        if( !rParser.IsNewDocument() )
            bSetAttributes = FALSE;
        if( pColl != pStandardColl )
        {
            // definierte Collection soll von Standard-Coll abgeleitet sein
            pColl->SetDerivedFrom( pStandardColl );
        }
    }
}

inline BOOL W4WStyleIdTabEntry::operator==( const W4WStyleIdTabEntry& rEntry ) const
{   return nStyleId == rEntry.nStyleId; }
inline BOOL W4WStyleIdTabEntry::operator<( const W4WStyleIdTabEntry& rEntry ) const
{   return nStyleId < rEntry.nStyleId;  }

SV_IMPL_OP_PTRARR_SORT( W4WStyleIdTab, W4WStyleIdTabEntryPtr )


// TranslateDefaultStyle gibt der Stylenummer, die den Defaultstyle darstellt,
// die Nummer USHRT_MAX. WW2 kennt #0 aks Default, rtf kennt keinen Default.

inline void TranslateDefaultStyle( long& nStyleId, USHORT nDocType )
{
    if ( 0 == nStyleId )			  			// != 0 ist niemals Default
    {
        if ( nDocType == 44 || nDocType == 49 )	// WW2 und WW6 benutzen Default
            nStyleId = USHRT_MAX;				// Default-Style
                                                //!! .. WP???
    }
}


void SwW4WParser::MakeUniqueStyleName( String& rName )
{
    nNoNameStylesCount++;
    rName.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "NoName(" ));
    rName += String::CreateFromInt32( nNoNameStylesCount );
    rName += ')';
}


void SwW4WParser::Read_StyleBasedOn()			// (SBO)
{
    long nStyNo;

    if ( !bStyleDef ) return;

    if(   !GetNextName() || nError
//	if( W4WR_TXTERM != GetNextRecord() || nError
        || W4WR_TXTERM != GetDecimal( nStyNo ) ) return;

    TranslateDefaultStyle ( nStyNo, nDocType );

#ifdef NIE				// Es wird sich nur noch an die Nummer gehalten,
                        // der Name wird ignoriert.

    Flush();							// '\0' hinter Dateinamen setzen
    String aName( aCharBuffer );

    pColl = pDoc->FindTxtFmtCollByName( aName )
#endif // NIE

    SwTxtFmtColl* pColl = 0;
    {
        W4WStyleIdTabEntry aEntry( (USHORT)nStyNo );
        W4WStyleIdTabEntry* pTmp =  &aEntry;
        USHORT nPos;
        if( pStyleTab->Seek_Entry( pTmp, &nPos ))
            pColl = (*pStyleTab)[ nPos ]->pColl;
    }
    SwTxtFmtColl* pAktColl = GetAktColl();
    ASSERT( pAktColl, "StyleId ungueltig" );

    // nicht gefunden -> in Liste aufnehmen
    if( pColl == 0 )
    {
        // Liste anlegen ?
        if( !pStyleBaseTab )
            pStyleBaseTab = new W4WStyleIdTab( 1, 1 );

        W4WStyleIdTabEntry* pNewStyle =
            new	W4WStyleIdTabEntry( (USHORT)nStyNo );
        // Abhaengigkeit merken
        pNewStyle->pColl = pAktColl;
        pStyleBaseTab->Insert( pNewStyle );
//		const W4WStyleIdTabEntry* &rpNewStyle = pNewStyle;
//		pStyleBaseTab->Insert( rpNewStyle );

    }
    else
    {
        // gefunden -> Abhaengigkeit eintragen
        if ( pAktColl )
            pAktColl->SetDerivedFrom( pColl );
    }
}

void SwW4WParser::Read_StyleTable()		// (SYT)
{
    bDefFontSet = TRUE;				// keine globalen Einstellungen mehr
    bPageDefRdy = TRUE;

    BOOL bOldTxtInDoc = bTxtInDoc;
    bTxtInDoc = FALSE;
    bStyleDef = TRUE;
    bSepar = FALSE;
    bIsTab = FALSE;

    StyleAnfang();				// Lesen des StyleSheet-Headers

    if( nError ) return;

    Flush();					// Haenge \0 an String an
    if( !pStyleTab )
        pStyleTab = new W4WStyleIdTab( 1, 1 );
    W4WStyleIdTabEntry * pNewStyle = new
        W4WStyleIdTabEntry( *this, nAktStyleId, aCharBuffer );

    pStyleTab->Insert( pNewStyle );
//	const W4WStyleIdTabEntry* &rpNewStyle = pNewStyle;
//	pStyleTab->Insert( rpNewStyle );

//	NOTE3( "[Style Def: %s, Id: %ld]", aCharBuffer, nAktStyleId );

    // lese alle Records vor RED
    // ignoriere dabei alle Records nach (SEP)
    BYTE c;
    while( !nError )
//	while( !nError && !pInput->get(c).eof() && W4WR_RED != c )
    {
        c = ReadChar();
        if ( rInp.IsEof() || W4WR_RED == c )
            break;
        rInp.SeekRel( - 1 );
//		pInput->putback(c);
        if (EOF==GetNextRecord())
            break;
    }

    if( rInp.IsEof() )
        nError = ERR_RECORD;

    Flush();					// loesche Pufferinhalt
    bStyleDef = FALSE;
    bSepar = FALSE;
    bTxtInDoc = bOldTxtInDoc;

    SwTxtFmtColl* pAktColl = GetAktColl();
//	if ( 0 == nAktStyleId )		// 0 : Dieser Style ist der Default
//		pDoc->SetTxtFmtColl( *pCurPaM, pAktColl, FALSE );//!!!???

    if ( pStyleBaseTab ){		// Es sind noch Verbindungen herzustellen
        W4WStyleIdTabEntry aEntry( nAktStyleId );
        W4WStyleIdTabEntry* pTmp =  &aEntry;
        USHORT nPos;
        if( pStyleBaseTab->Seek_Entry( pTmp, &nPos ))
        {
            // loesbar
            W4WStyleIdTabEntry* pOldStyle = (*pStyleBaseTab)[ nPos ];
            // stelle Verbindung her
            pOldStyle->pColl->SetDerivedFrom( pAktColl );
            pStyleBaseTab->DeleteAndDestroy( nPos );
        }
    }
//	if( 0 == pNewStyle->pColl->DerivedFrom() )
//	{
        // falls kein SBO kam,
        // wird der neue Style vom Default abgeleitet
//		pNewStyle->pColl->SetDerivedFrom( 0 );
//	}
    nAktStyleId = 0;		// Starte normalen Text mit Default-Style
}



SwTxtFmtColl* SwW4WParser::GetAktColl( BOOL* pSetAttrFlag ) const
{
    if( pSetAttrFlag )
        *pSetAttrFlag = TRUE;
    // hole die durche nAktStyleId angegebe Style
    W4WStyleIdTabEntry aEntry( nAktStyleId );
    W4WStyleIdTabEntry* pTmp =  &aEntry;
    USHORT nPos;
    if( !pStyleTab || !pStyleTab->Seek_Entry( pTmp, &nPos ))
        return pDoc->GetTxtCollFromPoolSimple( RES_POOLCOLL_STANDARD, FALSE );

    W4WStyleIdTabEntry* pStyle = (*pStyleTab)[ nPos ];
    if( pSetAttrFlag )
        *pSetAttrFlag = pStyle->bSetAttributes;

    return pStyle->pColl;
}


BOOL SwW4WParser::GetNextName()	// liest Namen (inc. Sonderzeichen)
{								// und legt ihn im aCharBuffer ab
    BYTE c;
    while( !nError )
    {
        c = ReadChar();

        if( rInp.IsEof() || W4WR_TXTERM == c ) break;

        if( W4WR_BEGICF == c )
        {
            rInp.SeekRel( - 1 );

            if( EOF == GetNextRecord() ) break;

            continue;
        }
        else
            FlushChar( c );
    }
    if( rInp.IsEof() )
    {
        nError = ERR_RECORD;
        return FALSE;
    }
    else
        return TRUE;
}


void SwW4WParser::StyleAnfang()
{
    if( !GetNextName() || nError ) return;

    long nStyleId;
    if( !nError && GetDecimal( nStyleId ) && !nError )
    {
        TranslateDefaultStyle( nStyleId, nDocType );
        nAktStyleId = (USHORT)nStyleId;
    }

    // Ab hier Umpopelungen dafuer, dass W4W Die Umlaute in den Stylenamen
    // anscheinend nur bei WP richtig quotet. Bei WW stimmt's bei OS/2 nicht,
    // bei AMI stimmt's nie

//JP 11.05.00: UNICODE-CHANGE  - stimmt das noch so ??
#if 0
    if ( nDocType == 44 || nDocType == 33 )    // Hier stimmen die Umlaute nicht
    {
        rtl_TextEncoding eCS = RTL_TEXTENCODING_IBM_437;  // Wandle Umlaute in ALay - Namen
        if ( nDocType == 44 )
            eCS = RTL_TEXTENCODING_MS_1252;  					// WinWord hat Ansi-Charset
        register BYTE ch, ch2;
        for ( USHORT i=0; i<nChrCnt; i++ )
        {
            ch = aCharBuffer[i];
            if ( ch < 32 || ch > 127 )
            {
                ch2 = String::Convert( ch, eCS, eSysCharSet );
                aCharBuffer[i] = ( ch2 == 0 ) ? ch : ch2;
            }
            else if ( nDocType == 33 && ch == '<'          // AMI-Pro-Umlaute
                    && aCharBuffer[i+1] == '\\' )
            {
                ch = aCharBuffer[i+2];
                switch ( ch )
                {
                case 'd':  ch = 0x84; break;
                case 'v':  ch = 0x94; break;
                case '|':  ch = 0x81; break;
                case '_':  ch = 0xe1; break;
                case 'D':  ch = 0x8e; break;
                case 'V':  ch = 0x99; break;
                case '\\': ch = 0x9a; break;
                default:   ch = 'X' ; break;
                }
                ch2 = String::Convert( ch, eCS, eSysCharSet ); // Umlaut in's System
                aCharBuffer[i] = ch2;
                for ( USHORT j=i+1; j+3<nChrCnt; j++ )      // loesche Klammern
                    aCharBuffer[j] = aCharBuffer[j+3];
                nChrCnt -= 3;
            }
        }
    }   // Ende Umpopelungen
#endif
}


void SwW4WParser::Read_Separator()      // (SEP)
{
    bSepar=TRUE;
}


void SwW4WParser::Read_StyleOn()        // (STY)
{
    BOOL bOldTxtInDoc = bTxtInDoc;
    bTxtInDoc = FALSE;
    bStyleOnOff = TRUE;
    bStyleOn = TRUE;
    if ( bIsColMode ) bIsSTYInTab = TRUE;

    StyleAnfang();      // lies den StyleSheet-Header
    if( !nError )		// Damit Flags zurueckgesetzt werden
    {

        Flush();
//    	NOTE3( "[Style On: %s, Id: %ld]", aCharBuffer, nAktStyleId );

        if( !pStyleTab )
        {
            ASSERT( !this, "+keine StyleTabelle definiert" );
        }
        else
        {
        //    pCtrlStck->StealAttr( RES_PARATR_TABSTOP, W4WR_NODE, pCurPaM->GetPoint() );
        //    pCtrlStck->StealAttr( RES_FLTR_STYLESHEET, W4WR_NODE, pCurPaM->GetPoint() );
        //	  const SwPosition& rPos = *pCurPaM->GetPoint();
        //    pCtrlStck->SetAttr( rPos, RES_LR_SPACE );
        //    pCtrlStck->SetAttr( rPos, RES_UL_SPACE );
        //    pCtrlStck->SetAttr( rPos, RES_CHRATR_FONT );
        //    pCtrlStck->SetAttr( rPos, RES_CHRATR_FONTSIZE );

            SetAttr( SwW4WStyle( nAktStyleId ));

            bStyleEndRec = FALSE;

            while( !nError && !bStyleEndRec     // lies alles bis zum SEP
                && EOF!=GetNextRecord())     // und ignoriere alle bis zum Ende
                ;

            nChrCnt = 0;
        }
    }
    bStyleOn  = bStyleOnOff = bStyleEndRec = FALSE;
    bTxtInDoc = bOldTxtInDoc;
    bStyleOff = FALSE;
}


void SwW4WParser::Read_StyleOff()       // (STF)
{
    BOOL bOldTxtInDoc = bTxtInDoc;
    bTxtInDoc = FALSE;
    bStyleOnOff = TRUE;
    bStyleOff=TRUE;

// zumindest bei WinWord2-Dokumenten wird beim Ausschalten des Styles
// erwartet, dass der hart gesetzte Font mit ausgeschaltet wird.
// Dieses sieht man aus Dokument k:\sw6\testplan.71\bugdoc\4295.doc

    if ( bOldTxtInDoc && !bStyleDef ){
        const SwPosition& rPos = *pCurPaM->GetPoint();
        pCtrlStck->SetAttr( rPos, RES_CHRATR_FONT );
        pCtrlStck->SetAttr( rPos, RES_CHRATR_FONTSIZE );
    }

    StyleAnfang();      // lese den StyleSheet-Header
    if( nError )
        return;

    Flush();

    bStyleEndRec = FALSE;
    while( !nError && !bStyleEndRec
           && EOF!=GetNextRecord())
        ;
    nChrCnt = 0;

    if( nDocType == 44 && bIsColMode ){	// Hier kommen STFs immer erst am
                                            // Anfang der naechsten Zelle
        if ( pCurPaM->GetPoint()->nContent.GetIndex() == 0 )
        {
            // Umpopele WinWord2-Fehler: Ausschalten
            // erfolgt oft erst am Anfang der folgenden Tab-Zelle
            BOOL bForward = pCurPaM->Move( fnMoveBackward, fnGoCntnt );

            pCtrlStck->SetAttr( *pCurPaM->GetPoint(),
                                RES_FLTR_STYLESHEET, TRUE, bForward );

            if( bForward )
                pCurPaM->Move( fnMoveForward, fnGoCntnt );
        }
        else
            pCtrlStck->SetAttr( *pCurPaM->GetPoint(), RES_FLTR_STYLESHEET );

    }
    else
    {  	// normale Behandlung
        pCtrlStck->SetAttr( *pCurPaM->GetPoint(), RES_FLTR_STYLESHEET );
    }

    nAktStyleId = USHRT_MAX;	// Default ist ab jetzt gueltig

    bStyleOnOff = bStyleEndRec = FALSE;
    bTxtInDoc = bOldTxtInDoc;
    if( bIsColMode ) bIsSTYInTab = FALSE;
}


void SwW4WParser::Read_StyleEnd()       // (STE)
{
    bStyleEndRec = TRUE;
}


// -------- andere Methoden: -----------------------------------



void SwW4WParser::Read_LangGroup()			// (SLG)
{
    long nLang;

    if ( bStyleOnOff ) return;

    if ( W4WR_TXTERM != GetDecimal( nLang ) || nError )
        return;

                                        //!!! Eigentlich muesste auf sinnvolle
                                        // Sprache getestet werden, aber ich
                                        // weiss nicht, wie das geht.

    if( !nLang || 0x400 == nLang )		// No Language || No Proofing
        nLang = LANGUAGE_DONTKNOW;

    if ( !bWasSLG && !bIsTxtInDoc ){	// 1. Angabe ist der Default
                                        // (heuristisch erwiesen)
        nDefLanguage = (USHORT)nLang;	// Merke Default
        bWasSLG = TRUE;					// genau einmal !??
    }

    pCtrlStck->SetAttr( *pCurPaM->GetPoint(), RES_CHRATR_LANGUAGE );

    if ( (USHORT)nLang != nDefLanguage ){ // Es gibt keinen "Ausschalter" bzw.
                                          // Endezeichen fuer das Sprachattribut.
                                          // Deshalb wird dann der Defaultwert
                                          // geschickt, d.h. wenn der Defaultwert
                                          // ankommt, ist Attributende gemeint.
                                          // Schade eigentlich!
        SetAttr( SvxLanguageItem( (const LanguageType)nLang ) );
    }
}

void SwW4WParser::Read_BeginKerning()		// (KER)
{
    long nPos, nDist;

    if( bStyleOnOff ) return;

    if ( W4WR_TXTERM != GetDecimal( nPos ) || nError
         || W4WR_TXTERM != GetDecimal( nDist ) || nError )
        return;

    pCtrlStck->SetAttr( *pCurPaM->GetPoint(), RES_CHRATR_KERNING );
    if ( nPos == 0 ) nDist = - nDist;
    SetAttr( SvxKerningItem( (const USHORT)nDist ));
}

void SwW4WParser::Read_EndKerning()			// (EKR)
{
    if( !bStyleOnOff )
        pCtrlStck->SetAttr( *pCurPaM->GetPoint(), RES_CHRATR_KERNING );
}

void SwW4WParser::Read_BeginHyphen()		// (BHP)
{
    if( bStyleOnOff ) return;

    SvxHyphenZoneItem aHyph( TRUE );
    pCtrlStck->SetAttr( *pCurPaM->GetPoint(), RES_PARATR_HYPHENZONE );
    SetAttr( aHyph );
}

void SwW4WParser::Read_EndHyphen()			// (EHP)
{
    if( !bStyleOnOff )
        pCtrlStck->SetAttr( *pCurPaM->GetPoint(), RES_PARATR_HYPHENZONE );
}

void SwW4WParser::Read_Split()					// (KEP)
{
    if( !bStyleOnOff )
    {
        SetAttr( SvxFmtSplitItem( FALSE ) );
    }
}


void SwW4WParser::Read_WidowOrphOn()			// (WON)
{
    if( !bStyleOnOff )
    {
        long nLines;

        if( W4WR_TXTERM != GetDecimal( nLines ) || nError )
            nLines = 2;

        SetAttr( SvxWidowsItem(  (const BYTE)nLines ) );
        SetAttr( SvxOrphansItem( (const BYTE)nLines ) );
    }
}


void SwW4WParser::Read_WidowOrphOff()			// (WOY)
{
    if( !bStyleOnOff )
    {
        const SwPosition& rPos = *pCurPaM->GetPoint();
        pCtrlStck->SetAttr( rPos, RES_PARATR_WIDOWS  );
        pCtrlStck->SetAttr( rPos, RES_PARATR_ORPHANS );
    }
}


/**************************
 *                        *
 *  Hidden Text einlesen  *
 *                        *
 **************************/

void SwW4WParser::Read_BeginHiddenText()		// (BHT)
{
    Read_ProcessHiddenText( FALSE );
}


void SwW4WParser::Read_EndHiddenText()			// (EHT)
{
    /*NOP*/		// hier keine Befehle!
}


void SwW4WParser::Read_CommentLine()			// (CMT)
{
    Read_ProcessHiddenText( TRUE );
}


void SwW4WParser::ProcessRecordInHiddenText()
{
    if( pActW4WRecord->fnReadRec == &SwW4WParser::Read_HardNewLine )
    {
        *pReadTxtString += '\n';
    }
    else
    if(    ( pActW4WRecord->fnReadRec == &SwW4WParser::Read_ExtendCharSet )
        || ( pActW4WRecord->fnReadRec == &SwW4WParser::Read_UpperCharSet  )
        || ( pActW4WRecord->fnReadRec == &SwW4WParser::Read_Hex           ) )
    {
        rInp.Seek( nStreamPosInRec );
        (this->*pActW4WRecord->fnReadRec)();
        rInp.SeekRel( - 1 );
        SkipEndRecord();
    }
}


void SwW4WParser::Read_ProcessHiddenText( BOOL bInline )
{
    /*
        Schade: der Writer kennt Hidden Text nur als *Feld*
                daher gehen wechselnde Formatierungen innerhalb
                des Hidden Textes verloren.
    */
    String aHiddenTxt;
    BOOL bOldNixExec      = bNixNoughtNothingExec;
    bNixNoughtNothingExec = TRUE;
    bReadTxtIntoString    = TRUE;
    pReadTxtString        = &aHiddenTxt;
    if( bInline )
    {
        register BYTE c;
        while( !nError )
        {
            c = ReadChar();

            if( rInp.IsEof() || W4WR_TXTERM == c ) break;

            if( W4WR_BEGICF == c )
            {
                rInp.SeekRel( - 1 );

                if( EOF == GetNextRecord() ) return;

                if( pActW4WRecord ) ProcessRecordInHiddenText();

                continue;
            }
            else
            {
                // naechsten Buchstaben nach XCS ueberlesen
                if( bWasXCS )
                    bWasXCS = FALSE;
                else
                    *pReadTxtString += (char)c;
            }
        }
        if( rInp.IsEof() ) return;
    }
    else
    {
        while(     ( !nError )
                && ( EOF != GetNextRecord() )
                &&   pActW4WRecord
                && ( pActW4WRecord->fnReadRec != &SwW4WParser::Read_EndHiddenText))
        {
            ProcessRecordInHiddenText();
        }
    }
    bNixNoughtNothingExec = bOldNixExec;
    bReadTxtIntoString    = FALSE;
    pReadTxtString        = 0;
    if( aHiddenTxt.Len() )
    {
        SwHiddenTxtField aFld( (SwHiddenTxtFieldType*)
                    pDoc->GetSysFldType( RES_HIDDENTXTFLD ),
                    TRUE, aEmptyStr, aHiddenTxt, TRUE );

        pDoc->Insert( *pCurPaM, SwFmtFld( aFld ) );
    }
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 *                                                                   *
 *                                                                   *
 *     besondere Methoden fuer  F r a m e M a k e r  - Dokumente     *
 *                                                                   *
 *                                                                   *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/*
    Datenstrukturen:
    - Page: steckt in einem SV_DECL_PTRARR_DEL
    - Page: enthaellt ein normales array mit den R-Ids

    - Rechteck: steck in 'SwTable'
         siehe: DECLARE_TABLE() in w4wpar.hxx,
                Key ist die R-Id.
*/


 /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
*                                                                     *
*    Vorbereitung:  lesen und analysieren der Strukturinformationen   *
*                                                                     *
 \* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

BOOL SwW4WParser::Read_Analyze_FLO_PDT()// analysieren aller PDTs und ggfs.
{										// erzeugen entsprechender PageDesc
    BOOL bOk = TRUE;

    if( pFLOInfos )
    {
        for( USHORT iFLO = 0; iFLO < pFLOInfos->Count(); ++iFLO )
        {
            // diese PDT analysieren
            W4WFLOInfo& rFLOInfo = *pFLOInfos->GetObject( iFLO );
            for(int i = 0; i < rFLOInfo.nRects; i++)
            {
                W4WRectInfo* pRectInfo = pRDTInfos->Get(rFLOInfo.pIds[ i ]);
                if( pRectInfo )
                    pRectInfo->nFLOId = rFLOInfo.nFlowId;
            }
        }
    }

    if( pPDTInfos )
    {
        for( USHORT iPDT = 0; iPDT < pPDTInfos->Count(); ++iPDT )
        {
            // diese PDT analysieren (falls er Rects hat)
            W4WPDTInfo& rPDTInfo = *pPDTInfos->GetObject( iPDT );
            if( rPDTInfo.nRects )
            {
                USHORT nBodyRect= USHRT_MAX;
                USHORT nHdRect	= USHRT_MAX;
                USHORT nFtRect	= USHRT_MAX;

                // Hilfs-Array mit den Rect-Grenzen anlegen

                W4WRectInfo** ppTmpRects = new W4WRectInfo*[ rPDTInfo.nRects ];


                for(int i = 0; i < rPDTInfo.nRects; i++)
                {
                    W4WRectInfo* pRectInfo = pRDTInfos->Get(rPDTInfo.pIds[ i ]);
                    if( pRectInfo )
                    {
                        int nInsert = i;
                        if( i )
                        {
                            // 0. einsortieren nach Y (innerhalb gleicher Y nach X)
                            for(int i2 = 0; i2 < i; i2++)
                            {
                                if(		(      pRectInfo->aRect.Pos().Y()
                                                <  ppTmpRects[ i2 ]->aRect.Pos().Y()
                                        )
                                    ||	(    ( pRectInfo->aRect.Pos().Y()
                                                == ppTmpRects[ i2 ]->aRect.Pos().Y() )
                                          && ( pRectInfo->aRect.Pos().X()
                                                <= ppTmpRects[ i2 ]->aRect.Pos().X() )
                                        ) )
                                {
                                    memmove( ppTmpRects+i2+1, ppTmpRects+i2,
                                             sizeof( void* ) * (i-i2) );
                                    nInsert = i2;
                                    break;
                                }
                            }
                        }
                        ppTmpRects[ nInsert ] = pRectInfo;
                        // Seitennummer auch im globalen Rect-array vermerken
                        pRectInfo->nPDTId = iPDT;
                    }
                    else
                    {
                        bOk = FALSE;
                        break;
                    }
                }
                 // cleverFrames: vertraute Strukturen zu erkennen
                const BOOL bOnlyFrames = 0 != (W4WFL_FM_onlyFrames & nIniFMFlags);
                const BOOL bCleverFrames =
                        (0 == (   (   W4WFL_FM_onlyFrames
                                    | W4WFL_FM_neverFrames )
                                & nIniFMFlags));

                if( bOk )
                {
                    /**********************************************************\
                        Regeln der Analyse:
                        ^^^^^^^^^^^^^^^^^^^
                    1.  Rects, die in anderen drinnen stecken, werden ignoriert

                    2.  nebeneinanderliegende Rects, die auf gleicher Hoehe
                        beginnen *und* enden, werden zu einer spaltigen Einheit

                    3.  zur Erkennung, ob ein Rect als Kopf- (bzw. Fuss-)bereich
                        anzusehen ist, genuegt es, dass EINE seiner Kanten
                        buendig mit dem Rumpfbereich ist
                    \**********************************************************/
                    /*
                        Realisierung:

                        0. sortiere nach Y (innerhalb gleicher Y nach X)  (s.o.!)
                        1. raus mit allen /innerhalb/ anderer liegenden Rects
                        2. erkenne Spaltigkeit
                        3. finde den Bodybereich
                        4. suche ggfs. nach dem Kopf- und dem Fussbereich
                    */
                    const int maxDelta = 287;// Spalten: knapp 1/2 cm Versatz der
                                            //           Ober-/Unterkante ist Ok.
                    int i;
                    for(i = 0; i < rPDTInfo.nRects; i++)
                    {
                        W4WRectInfo& rRectInfo = *(ppTmpRects[ i ]);
                        if( rRectInfo.IsActive() )
                        {
                            const long nXa1 = rRectInfo.aRect.Pos().X();
                            const long nYa1 = rRectInfo.aRect.Pos().Y();
                            const long nXa2 = nXa1 + rRectInfo.aRect.SSize().Width();
                            const long nYa2 = nYa1 + rRectInfo.aRect.SSize().Height();
                            for(int i2 = i+1; i2 < rPDTInfo.nRects; i2++)
                            {
                                W4WRectInfo& rRectInfo2 = *(ppTmpRects[ i2 ]);
                                if( rRectInfo2.IsActive() )
                                {
                                    const long nXb1 = rRectInfo2.aRect.Pos().X();
                                    const long nYb1 = rRectInfo2.aRect.Pos().Y();
                                    const long nXb2
                                        = nXb1 + rRectInfo2.aRect.SSize().Width();
                                    const long nYb2
                                        = nYb1 + rRectInfo2.aRect.SSize().Height();

                                    // 1. raus mit /innerhalb/ liegenden Rects
                                    if(    (nXa1 <= nXb1)
                                        && (nXa2 >= nXb2)
                                        && (nYa1 <= nYb1)
                                        && (nYa2 >= nYb2) )
                                    {
                                        ppTmpRects[ i2 ]->bInside = TRUE;
                                    }
                                    else

                                    // 2. erkenne Spaltigkeit ( im gleichen FLO ! )
                                    if(	   bCleverFrames
                                        && (maxDelta > (abs(nYa1 - nYb1)))
                                        && (maxDelta > (abs(nYa2 - nYb2)))
                                        && (   ppTmpRects[ i ]->nFLOId
                                            == ppTmpRects[ i2]->nFLOId) )
                                    {
                                        ppTmpRects[ i2 ]->nSWId
                                            = ppTmpRects[ i ]->nFMId;
                                        // linkes Rect waechst ggfs. nach rechts
                                        if( nXa2 < nXb2 )
                                        {
                                            ppTmpRects[ i ]->aRect.Width( nXb2
                                                                        - nXa1 );
                                            ppTmpRects[ i ]->bColXYZ = TRUE;
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // 3. finde den Bodybereich
                    long nBodyRectVol=-1;
                    for(i = 0; i < rPDTInfo.nRects; i++)
                    {
                        W4WRectInfo& rRect = *(ppTmpRects[ i ]);
                        if( rRect.IsActive() )
                        {
                            const long nVol = rRect.aRect.SSize().Width()
                                            * rRect.aRect.SSize().Height();
                            // ermittele das groesste Rect
                            if( nBodyRectVol < nVol )
                            {
                                rRect.bBody	= TRUE;
                                nBodyRectVol= nVol;
                                nBodyRect	= rRect.nSWId;
                            }
                        }
                    }

                    /*
                        Body-Rect merken
                    */
                    if( USHRT_MAX != nBodyRect )
                        rPDTInfo.nBodyRectId = nBodyRect;

                    // 4. suche ggfs. nach dem Kopf- und dem Fussbereich
                    long nHdRectVol	=-1;
                    long nFtRectVol	=-1;
                    if( bCleverFrames && (USHRT_MAX != nBodyRect) )
                    {
                        W4WRectInfo& rRect = *pRDTInfos->Get( nBodyRect );
                        const long nXa1 = rRect.aRect.Pos().X();
                        const long nXa2
                            = nXa1 + rRect.aRect.SSize().Width();
                        const long nYa1 = rRect.aRect.Pos().Y();
                        const long nYa2
                            = nYa1 + rRect.aRect.SSize().Height();

                        for(i = 0; i < rPDTInfo.nRects; i++)
                        {
                            if( i != nBodyRect )
                            {
                                W4WRectInfo& rRect2 = *(ppTmpRects[ i ]);
                                if( rRect2.IsActive() )
                                {
                                    /*
                                        falls mehrere(!) Frames links oder rechts
                                        buendig ueber bzw. unter dem Body-Frame stehen,
                                        nehmen wir den jeweils groessten von diesen als
                                        Kopf- bzw. Fuss-Frame
                                    */
                                    const long nXb1 = rRect2.aRect.Pos().X();
                                    const long nXb2
                                        = nXb1 + rRect2.aRect.SSize().Width();

                                    if(    (maxDelta > (abs(nXa1 - nXb1)))
                                        || (maxDelta > (abs(nXa2 - nXb2))) )
                                    {
                                        const long nVol
                                            =   rRect2.aRect.SSize().Width()
                                              * rRect2.aRect.SSize().Height();
                                        const long nYb1 = rRect2.aRect.Pos().Y();
                                        const long nYb2
                                            = nYb1 + rRect2.aRect.SSize().Height();
                                        if( nYa1 >= nYb1 )
                                        {
                                            if( nHdRectVol < nVol )
                                            {
                                                nHdRectVol	= nVol;
                                                nHdRect		= rRect.nSWId;
                                            }
                                        }
                                        else
                                        if( nYa2 <= nYb2 )
                                        {
                                            if( nFtRectVol < nVol )
                                            {
                                                nFtRectVol	= nVol;
                                                nFtRect		= rRect.nSWId;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        /*
                            Sonder-Rects merken
                        */
                        if(USHRT_MAX != nHdRect)
                        {
                            rPDTInfo.nHdRectId = nHdRect;
                            pRDTInfos->Get( nHdRect )->bHeader = TRUE;
                        }
                        if(USHRT_MAX != nFtRect)
                        {
                            rPDTInfo.nFtRectId = nFtRect;
                            pRDTInfos->Get( nFtRect )->bFooter = TRUE;
                        }
                    }
                    // Ok, wir haben getan, was wir konnten, also versuchen wir nun,
                    //     den entsprechenden Pagedesk aufzubauen...
                    String aNm( String::CreateFromAscii(
                                RTL_CONSTASCII_STRINGPARAM( "Convert " )));
                    if( 1 < rPDTInfo.nPageSubType )	// 2,3 == Left, Right Page
                        aNm.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "Follow " ));
                    aNm += String::CreateFromInt32( pDoc->GetPageDescCnt() );

                    rPDTInfo.nSwPdId = pDoc->MakePageDesc( aNm, 0, FALSE );

                    pPageDesc = &((SwPageDesc&)pDoc->GetPageDesc( rPDTInfo.nSwPdId ));

                    pPageDesc->SetFollow( pPageDesc );// Follow-PgDesc erstmal self

                    if( bCleverFrames )
                    {
                        /*
                            PageDesc soweit moeglich einrichten:

                            wir koennen hier schon obere, linke Ecke
                            des Rumpf, Kopf- und Fussbereichs definieren.

                            was noch fehlt, ist die Groesse, da diese sich
                            auch aus der Seitengroesse ergibt, die noch
                            unbekant ist.
                        */

                        // Upper und Left der PrtArea definieren
                        USHORT nMinTop, nMinLeft, nMinBottom, nMinRight;
                        if( MEASURE_METRIC == GetAppLocaleData().getMeasurementSystemEnum() )
                        {
                            nMinTop    = nMinLeft  =
                            nMinBottom = nMinRight = 1134; //2 cm
                        }
                        else
                        {
                            nMinTop  = nMinBottom= 1440; //wie zB. in WW: 1    Inch
                            nMinLeft = nMinRight = 1800; //               1,25 Inch
                        }
                        USHORT nTop  = nMinTop;
                        USHORT nLeft = nMinLeft;
                        W4WRectInfo& rBodyInfo =
                                        *pRDTInfos->Get( rPDTInfo.nBodyRectId);
                        W4WRectInfo* pHdInfo   =
                                          (USHRT_MAX != rPDTInfo.nHdRectId)
                                        ? pRDTInfos->Get( rPDTInfo.nHdRectId )
                                        : 0;
                        W4WRectInfo* pFtInfo   =
                                          (USHRT_MAX != rPDTInfo.nFtRectId)
                                        ? pRDTInfos->Get( rPDTInfo.nFtRectId )
                                        : 0;

                        SwRect& rBodyRect = rBodyInfo.aRect;
                        Point& rUpPos = pHdInfo ? pHdInfo->aRect.Pos()
                                                : rBodyRect.Pos();
                        Point& rLoPos = pFtInfo ? pFtInfo->aRect.Pos()
                                                : rBodyRect.Pos();

                        nTop  = (USHORT)rUpPos.Y();
                        nLeft = (USHORT)rUpPos.X();

                        SwFrmFmt& rFrmFmt = pPageDesc->GetMaster();

                        rFrmFmt.SetAttr( SvxULSpaceItem( nTop, nMinBottom ) );
                        rFrmFmt.SetAttr( SvxLRSpaceItem( nLeft, nMinRight ) );

                        // Header und Footer definieren
                        SwFrmFmt* pHdFmt = 0;
                        SwFrmFmt* pFtFmt = 0;
                        W4WRectInfo& rRectInfo = *pRDTInfos->Get( rPDTInfo.nHdRectId );
                        if( pHdInfo )
                        {
                            rFrmFmt.SetAttr( SwFmtHeader( TRUE ));

                            pHdFmt = (SwFrmFmt*)rFrmFmt.GetHeader().GetHeaderFmt();
                            SwRect& rUpRect = pHdInfo->aRect;
                            UpdateHdFtMarginSettings(
                                pHdFmt,
                                rUpRect.Height(),                 // Kopf-Hoehe
                                rBodyRect.Top() -rUpRect.Bottom(),// Kopf-Lower
                                TRUE);
                        }
                        if(USHRT_MAX != rPDTInfo.nFtRectId)
                        {
                            rFrmFmt.SetAttr( SwFmtFooter( TRUE ));

                            pFtFmt = (SwFrmFmt*)rFrmFmt.GetFooter().GetFooterFmt();
                            SwRect& rLoRect = pFtInfo->aRect;
                            UpdateHdFtMarginSettings(
                                pFtFmt,
                                rLoRect.Height(),                 // Fuss-Hoehe
                                rLoRect.Top() -rBodyRect.Bottom(),// Fuss-Upper
                                FALSE);
                        }
                        /*
                            UEber pPDTInfo->PageSizeKnown() koennen wir jederzeit
                            nachsehen, ob der PgDesc noch unvollstaendig ist,
                            damit spaetere Erst-AEnderungen nicht
                            zur Erstellung eines *neuen* PgDesc fuehren
                        */
                    }
                }
                // Hilfsliste mit den *Rects dieser Seite wieder loeschen
                delete ppTmpRects;
            }
        }
    }
    return bOk;
}

void SwW4WParser::Read_PageDefinitionTable()			// PDT
{
    // FrameMaker (oder aehnl.) ohne Flag 'W4WFL_FM_neverFrames'
    if( !(W4WFL_FM_neverFrames & nIniFMFlags) )
    {
        if( !pPDTInfos )
            pPDTInfos = new W4WPDTInfos;

        W4WPDTInfo_Ptr pInfo = new W4WPDTInfo();

        BOOL bOk = FALSE;
        if(    GetDecimal( pInfo->nPageType )			&& !nError
            && GetDecimal( pInfo->nPageSubType )		&& !nError
            && GetString(  pInfo->sPageName,
                            W4WR_TXTERM, W4WR_TXTERM )	&& !nError
            && GetDecimal( pInfo->nRects )				&& !nError )
        {
            pInfo->pIds = new long[ pInfo->nRects ];

            bOk = TRUE;
            for (int i=0; i < pInfo->nRects; i++)
                if( !GetDecimal(pInfo->pIds[ i ])  || nError )
                {
                    bOk = FALSE;
                    break;
                }
        }
        if( bOk )
            pPDTInfos->Insert( pInfo, pPDTInfos->Count() );
        else
            delete pInfo;
    }
}

void SwW4WParser::Read_RectangleDefinitionTable()		// RDT
{
    // FrameMaker (oder aehnl.) ohne Flag 'W4WFL_FM_neverFrames'
    if( !(W4WFL_FM_neverFrames & nIniFMFlags) )
    {
        if( !pRDTInfos )
            pRDTInfos = new W4WRDTInfos;

        SwRect aRect;
        ULONG nId;

        if(    GetDecimal( (long&)nId )				&& !nError
            && GetDecimal( aRect.Pos().X() )		&& !nError
            && GetDecimal( aRect.Pos().Y() )		&& !nError
            && GetDecimal( aRect.SSize().Width() )	&& !nError
            && GetDecimal( aRect.SSize().Height() )	&& !nError )
        {
            W4WRectInfo* pRectInfo = new W4WRectInfo( aRect, nId );
            if( !pRDTInfos->Insert( nId, pRectInfo ) )
                delete pRectInfo;
        }
    }
}

void SwW4WParser::Read_Flow()							// FLO
{
    // FrameMaker (oder aehnl.) ohne Flag 'W4WFL_FM_neverFrames'
    if( !(W4WFL_FM_neverFrames & nIniFMFlags) )
    {
        if( !pFLOInfos )
            pFLOInfos = new W4WFLOInfos;

        W4WFLOInfo_Ptr pInfo = new W4WFLOInfo();

        BOOL bOk = FALSE;
        if(    GetDecimal( pInfo->nFlowId )			&& !nError
            && GetDecimal( pInfo->nRects )			&& !nError )
        {
            pInfo->pIds = new long[ pInfo->nRects ];

            bOk = TRUE;
            for (int i=0; i < pInfo->nRects; i++)
                if( !GetDecimal(pInfo->pIds[ i ])  || nError )
                {
                    bOk = FALSE;
                    break;
                }
        }
        if( bOk )
            pFLOInfos->Insert( pInfo, pFLOInfos->Count() );
        else
            delete pInfo;
    }
}


 /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
*                                                                     *
*     es wird ernst:  Reaktion auf Start-/Endmarken im Fliesstext     *
*                                                                     *
 \* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void SwW4WParser::Read_BeginTextFlow()					// BTF
{
    if( !pFLOInfos ) return;

    if( !bPDTanalyzed )
        bPDTanalyzed = Read_Analyze_FLO_PDT();

    long nFlowId;
    if( GetDecimal( nFlowId ) && !nError )
    {
        W4WFLOInfo_Ptr pFLO = 0;
        for(int i = 0; i < pFLOInfos->Count(); i++)
        {
            if( nFlowId == pFLOInfos->GetObject( i )->nFlowId )
            {
                pFLO = pFLOInfos->GetObject( i );
                break;
            }
        }
        if( pFLO )
        {
            W4WRectInfo* pRectInfo = pRDTInfos->Get(pFLO->pIds[ 0 ]);
            //  Rechteck 1 ist bekannt, also ggfs. PaM in die Seite setzen!
            if( pRectInfo )
            {
                // ACHTUNG: PgDesk darf ich immer in den Body setzen!!
                //
                // daher lieber nachsehen, ob dieses neue Rect noch
                // im aktuellen Hd-/Ft-Abschnitt
                // auf der gleichen Seite steckt
                if(    bHeadFootDef
                    && !(    pActRectInfo
                          && (     (    pActRectInfo->BodyHdFtMode()
                                     != pRectInfo->BodyHdFtMode() )
                                || (    (1 < pActRectInfo->BodyHdFtMode())
                                     && (    pActRectInfo->nPDTId
                                          != pRectInfo->nPDTId ) ) ) ) )
                {
                    // wichtig: zurueck VOR den Beginn des .<BTF..>
                    //          gehen, damit dieser NACH dem Verlassen
                    //          des K-/F-Bereichs nochmals gelesen wird
                    rInp.Seek( nStartOfActRecord - 1 );

                    bHeadFootDef = FALSE;
                }
                else
                {
                    W4WRectInfo* pLastRectInfo	= pActRectInfo;
                    pActRectInfo				= pRectInfo;
                    if( pLastRectInfo != pActRectInfo )
                    {
                        /*
                            anderes Rect ist jetzt aktiv, also nachschauen, ob
                            wir vielleicht einen neuen PageDesc benoetigen
                        */
                        if( 0 <= pRectInfo->nPDTId )
                        {
                            // global die PDTInfo merken
                            pPDTInfo = pPDTInfos->GetObject( pRectInfo->nPDTId );

                            SwPageDesc& rNextPageDesc =
                                (SwPageDesc&)pDoc->GetPageDesc( pPDTInfo->nSwPdId );

                            if( pPageDesc != &rNextPageDesc )
                            {
                                // dies ist der neue PageDesc
                                pPageDesc = &rNextPageDesc;

                                // Ok, neuen PageDesc ins Doc setzen...
                                pDoc->Insert( *pCurPaM, SwFmtPageDesc( pPageDesc ));
                                bIsTxtInPgDesc = FALSE;

                                // Up to now there was no .<HNP> within this Page-Descriptor's scope.
                                // ( We need this information in SwW4WParser::Read_HardNewPage() )
                                bIsTxtInPara
                                = bIsTxtInPgDesc
                                = bWasTxtSince_BREAK_PAGE
                                = bWasHNPInPgDesc
                                = FALSE;

                                //UpdateCacheVars();
                            }
                        }
                    }
                }
            }
        }
    }
}

void SwW4WParser::Read_EndTextFlow()					// ETF
{
    if( !pFLOInfos ) return;

}

void SwW4WParser::Read_TextRectangelId()				// TRI
{
    if( !pFLOInfos ) return;

    if( !bPDTanalyzed )
        bPDTanalyzed = Read_Analyze_FLO_PDT();

    long nRectId;
    if( GetDecimal( nRectId ) && !nError )
    {
        W4WRectInfo* pRectInfo = pRDTInfos->Get( nRectId );
        if( pRectInfo )
        {
            W4WRectInfo* pLastRectInfo	= pActRectInfo;
            pActRectInfo				= pRectInfo;
            if( pLastRectInfo != pActRectInfo )
            {
                // anderes Rect ist jetzt aktiv, also nachschauen, ob wir
                // vielleicht einen neuen PageDesc benoetigen

                if( 0 <= pRectInfo->nPDTId )
                {
                    // global die PDTInfo merken
                    pPDTInfo = pPDTInfos->GetObject( pRectInfo->nPDTId );
                    SwPageDesc& rNextPageDesc =
                        (SwPageDesc&)pDoc->GetPageDesc( pPDTInfo->nSwPdId );
                    if(    (pPageDesc != &rNextPageDesc)
                        && (USHRT_MAX != pPDTInfo->nBodyRectId) )
                    {
                        // Ok, Werte fuer neuen PageDesc setzen...

                        // Upper und Left der PrtArea definieren
                        USHORT nMinTop, nMinLeft, nMinBottom, nMinRight;
                        if( MEASURE_METRIC == GetAppLocaleData().getMeasurementSystemEnum() )
                        {
                            nMinTop    = nMinLeft  =
                            nMinBottom = nMinRight = 1134; //2 cm
                        }
                        else
                        {
                            nMinTop  = nMinBottom= 1440; //wie zB. in WW: 1    Inch
                            nMinLeft = nMinRight = 1800; //               1,25 Inch
                        }
                        USHORT nTop  = nMinTop;
                        USHORT nLeft = nMinLeft;
                        W4WRectInfo& rBodyInfo =
                                        *pRDTInfos->Get( pPDTInfo->nBodyRectId);
                        W4WRectInfo* pHdInfo   =
                                          (USHRT_MAX != pPDTInfo->nHdRectId)
                                        ? pRDTInfos->Get( pPDTInfo->nHdRectId )
                                        : 0;
                        W4WRectInfo* pFtInfo   =
                                          (USHRT_MAX != pPDTInfo->nFtRectId)
                                        ? pRDTInfos->Get( pPDTInfo->nFtRectId )
                                        : 0;

                        SwRect& rBodyRect = rBodyInfo.aRect;
                        Point& rUpPos = pHdInfo ? pHdInfo->aRect.Pos()
                                                : rBodyRect.Pos();
                        Point& rLoPos = pFtInfo ? pFtInfo->aRect.Pos()
                                                : rBodyRect.Pos();

                        nTop  = (USHORT)rUpPos.Y();
                        nLeft = (USHORT)rUpPos.X();

                        SwFrmFmt& rFrmFmt = pPageDesc->GetMaster();

                        rFrmFmt.SetAttr( SvxULSpaceItem( nTop, nMinBottom ) );
                        rFrmFmt.SetAttr( SvxLRSpaceItem( nLeft, nMinRight ) );

                        SwFrmFmt* pHdFmt = 0;
                        SwFrmFmt* pFtFmt = 0;
                        W4WRectInfo& rRectInfo = *pRDTInfos->Get( pPDTInfo->nHdRectId );
                        if( pHdInfo )
                        {
                            pHdFmt = (SwFrmFmt*)pPageDesc->GetMaster().GetHeader().GetHeaderFmt();
                            SwRect& rUpRect = pHdInfo->aRect;
                            UpdateHdFtMarginSettings(
                                pHdFmt,
                                rUpRect.Height(),                 // Kopf-Hoehe
                                rBodyRect.Top() -rUpRect.Bottom(),// Kopf-Lower
                                TRUE);
                        }
                        if(USHRT_MAX != pPDTInfo->nFtRectId)
                        {
                            pFtFmt = (SwFrmFmt*)pPageDesc->GetMaster().GetFooter().GetFooterFmt();
                            SwRect& rLoRect = pFtInfo->aRect;
                            UpdateHdFtMarginSettings(
                                pFtFmt,
                                rLoRect.Height(),                 // Fuss-Hoehe
                                rLoRect.Top() -rBodyRect.Bottom(),// Fuss-Upper
                                FALSE);
                        }

                        // PgDesk immer nur im Body setzen!!

                    }
                }
            }
        }
    }
}


}
