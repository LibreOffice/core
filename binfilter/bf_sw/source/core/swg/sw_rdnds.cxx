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

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _PERSIST_HXX //autogen
#include <bf_so3/persist.hxx>
#endif
#ifndef _EMBOBJ_HXX //autogen
#include <bf_so3/embobj.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <bf_svtools/urihelper.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <bf_svx/fontitem.hxx>
#endif
#ifndef _SVX_CSCOITEM_HXX //autogen
#include <bf_svx/cscoitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <bf_svx/lrspitem.hxx>
#endif

#ifndef _TXTFTN_HXX //autogen
#include <txtftn.hxx>
#endif
#ifndef _FCHRFMT_HXX //autogen
#include <fchrfmt.hxx>
#endif
#ifndef _CHARATR_HXX
#include <charatr.hxx>
#endif
#ifndef _CHARFMT_HXX //autogen
#include <charfmt.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _RDSWG_HXX
#include <rdswg.hxx>
#endif
#ifndef _SW3IO_HXX
#include <sw3io.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _NDGRF_HXX
#include <ndgrf.hxx>
#endif
#ifndef _NDOLE_HXX
#include <ndole.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _FLYPOS_HXX
#include <flypos.hxx>
#endif
#ifndef _BOOKMRK_HXX
#include <bookmrk.hxx>
#endif
#ifndef _SWGPAR_HXX
#include <swgpar.hxx>
#endif
#ifndef _SWSWERROR_H
#include <swerror.h>
#endif
#include "bf_so3/staticbaseurl.hxx"
namespace binfilter {


//////////////////////////////////////////////////////////////////////////////

// Achtung: Jede Routine nimmt an, dass der Record-Header bereits eingelesen
// ist. Sie arbeitet den Record so weit ab wie moeglich und schliesst den
// Vorgang mit end() ab.

//////////////////////////////////////////////////////////////////////////////

// Text-Hints
// nOffset ist ungleich Null, wenn innerhalb eines Nodes eingefuegt werden
// soll. Dann ist nOffset die Start-Position des Textes.

 void SwSwgReader::InTextHints( SwTxtNode& rNd, xub_StrLen nOffset )
 {
    // Dieser Record kann auch leer sein
    // (bei teilweisem Speichern eines Nodes, z.B.)
    short nHt;
    r >> nHt;
    if( nHt < 0 )
    {
        Error(); r.skipnext(); return;
    }
    xub_StrLen nLen = rNd.GetTxt().Len();
    if( nLen ) nLen --;
    for( int i = 0; i < nHt; i++ ) {
        USHORT nOldStart, nOldEnd;
        r >> nOldStart >> nOldEnd;
        r.next();
        xub_StrLen nStart = nOldStart + nOffset;
        xub_StrLen nEnd   = nOldEnd + nOffset;
        if( nOldEnd < nOldStart ) nEnd = nLen;
        USHORT nWhich = InHint( rNd, nStart, nEnd );
        if( nWhich == RES_TXTATR_FTN )
        {
            // Der Footnote-Hint ist somewhat special. Er erhaelt
            // eine Section, wenn er in den TextNode eingefuegt wird.
            // Daher muss der Text getrennt geparst werden
            SwTxtFtn* pFtn = (SwTxtFtn*) rNd.GetTxtAttr( nStart, nWhich );
            SwNodeIndex aIdx( *pFtn->GetStartNode() );
            FillSection( aIdx );
            // den zuletzt eingelesenen Record zuruecksetzen
            r.undonext();
        }
    }
    r.next();
 }

// Bookmarks:

 void SwSwgReader::InNodeBookmark( SwNodeIndex& rPos )
 {
    BYTE cFlags = 0;
    // Flag-Byte: 0x01 = Startmakro, 0x02 = Endmakro
    r >> cFlags;
    String aName = GetText();
    String aShort = GetText();
    if( !r )
        return; // must have
    USHORT nOffset, nKey, nMod;
    r >> nOffset >> nKey >> nMod;
    const SwBookmarks& rMarks = pDoc->GetBookmarks();
    short nArrLen = rMarks.Count();
    SwPaM aPaM( rPos.GetNode(), (xub_StrLen)nOffset );
    pDoc->MakeBookmark( aPaM, KeyCode( nKey, nMod ), aName, aShort );
    if( cFlags )
    {
        SwBookmark* pMark = rMarks[ nArrLen ];
        if( cFlags & 0x01 )
        {
            String aMac = GetText();
            String aLib = GetText();
            SvxMacro aStart( aMac, aLib, STARBASIC );
            pMark->SetStartMacro( aStart );
        }
        if( cFlags & 0x02 )
        {
            String aMac = GetText();
            String aLib = GetText();
            SvxMacro aEnd( aMac, aLib, STARBASIC );
            pMark->SetEndMacro( aEnd );
        }
    }

    r.next();
 }

// Hilfsroutine fuer ConvertText: Suche nach dem naechsten Hint,
// der eine Konversion verbietet. Zur Zeit sind dies Hints, die entweder
// direkt oder indirekt auf einen Font mit CHARSET_SYMBOL hinweisen.

 USHORT SwSwgReader::GetNextSymbolFontHint
 ( SwpHints* pHints, USHORT idx, xub_StrLen& start, xub_StrLen& end )
 {
    if( !pHints ) return 0;
    for( ; idx < pHints->Count(); idx++ )
    {
        const SwTxtAttr* pHnt = (*pHints) [ idx ];
        if( pHnt->Which() == RES_CHRATR_FONT )
        {
            if( pHnt->GetFont().GetCharSet() == RTL_TEXTENCODING_SYMBOL )
            {
                start = *pHnt->GetStart();
                end   = *pHnt->GetEnd() - 1;
                break;
            }
        }
        // Gibt es einen CharFormat-Hint mit einem Symbol-Font?
        if( pHnt->Which() == RES_TXTATR_CHARFMT )
        {
            SwCharFmt* pFmt = pHnt->GetCharFmt().GetCharFmt();
            if( pFmt->GetAttrSet().GetItemState( RES_CHRATR_FONT, FALSE )
                == SFX_ITEM_SET )
            {
                const SvxFontItem& rAttr = pFmt->GetFont();
                if( rAttr.GetCharSet() == RTL_TEXTENCODING_SYMBOL )
                {
                    start = *pHnt->GetStart();
                    end   = *pHnt->GetEnd();
                    break;
                }
            }
        }
    }
    return idx;
 }

 USHORT SwSwgReader::GetNextCharsetColorHint
 ( SwpHints* pHints, USHORT idx, xub_StrLen& start, xub_StrLen& end,
   rtl_TextEncoding eDfltEnc, rtl_TextEncoding& rEnc )
 {
    rEnc = eDfltEnc;

    if( !pHints ) return 0;
    for( ; idx < pHints->Count(); idx++ )
    {
        const SwTxtAttr* pHnt = (*pHints) [ idx ];
        if( pHnt->Which() == RES_CHRATR_CHARSETCOLOR )
        {
            if( pHnt->GetCharSetColor().GetCharSet() != eDfltEnc )
            {
                rEnc = pHnt->GetCharSetColor().GetCharSet();
                start = *pHnt->GetStart();
                end   = *pHnt->GetEnd() - 1;
                break;
            }
        }
    }
    return idx;
 }
// Text Node konvertieren
// Wird aufgerufen, wenn sich die Systeme unterscheiden. Der Text wird
// vom einen in den anderen Zeichensatz konvertiert. Nicht konvertierbare
// Zeichen werden farblich unterlegt; Hints mit CHARSET_SYMBOL-Zeichensaetzen
// werden uebersprungen

 void SwSwgReader::ConvertText( SwTxtNode& rNd, rtl_TextEncoding eSrc )
 {
    const ByteString* pText = GetReadTxt();
    xub_StrLen nLength;
    if( !pText || 0 == ( nLength = pText->Len() ) )
        return;

    // Variablen fuer das Hint-Array
    USHORT hntidx = 0;
    xub_StrLen symfrom = (xub_StrLen)-1, symto = 0;
    SwpHints* pHints = rNd.GetpSwpHints();
    USHORT hntsiz = pHints ? pHints->Count() : 0;
    String& rNdText = (String &)rNd.GetTxt();

    const SvxFontItem& rFont = rNd.GetSwAttrSet().GetFont();
    BOOL bNdSym = rFont.GetCharSet() == RTL_TEXTENCODING_SYMBOL;

    // Step 1: Replace all characters that are symbol font.
    hntidx = GetNextSymbolFontHint( pHints, hntidx, symfrom, symto );
    xub_StrLen pos;
    for( pos = 0; pos < nLength; pos++ )
    {
        sal_Unicode cNew = 0;
        // Eventuell neuen Font-Hint suchen
        if( symfrom != (xub_StrLen)-1 && symto < pos )
        {
            symfrom = (xub_StrLen)-1;
            hntidx = GetNextSymbolFontHint( pHints, hntidx, symfrom, symto );
        }
        sal_Char ch = pText->GetChar( pos );
        // Darf das Zeichen konvertiert werden?
        if( '\xff' == ch )
        {
            // Sonderzeichen: liegt dort ein Hint ohne Ende-Index?
            for( USHORT i = 0; i < hntsiz; ++i )
            {
                const SwTxtAttr *pPos = (*pHints)[ i ];
                const xub_StrLen nStart = *pPos->GetStart();
                if( pos == nStart && !pPos->GetEnd() )
                {
                    // Wenn ja, dann darf ch nicht konvertiert werden!
                    ch = 0;
                    cNew = GetCharOfTxtAttr( *pPos );
                    break;
                }
                if( nStart > pos ) break;
            }
        }
        if( ch == '\t' || ch == '\n' ) ch = 0;
        // Testen, ob man sich im verbotenen Bereich befindet:
        if( ch )
        {
            if( bNdSym || (symfrom != (xub_StrLen)-1 &&
                           pos >= symfrom && pos < symto) )
            {
                // TODO: unicode: What happens with symbol characters?
 //             cNew = String::ConvertToUnicode( ch, eSrc );
                cNew = (sal_Unicode)ch;
            }
        }
        if( cNew )
            rNdText.SetChar( pos, cNew );
    }

    // Step 2: Replace all characters that have a charset color attribute set
    rtl_TextEncoding eEnc = eSrc;
    hntidx = GetNextCharsetColorHint( pHints, hntidx, symfrom, symto,
                                      eSrc, eEnc);

    for( pos = 0; pos < nLength; pos++ )
    {
        // Eventuell neuen Font-Hint suchen
        if( symfrom != (xub_StrLen)-1 && symto < pos )
        {
            symfrom = (xub_StrLen)-1;
            hntidx = GetNextCharsetColorHint( pHints, hntidx, symfrom, symto,
                                              eSrc, eEnc );
        }
        sal_Char ch = pText->GetChar( pos );
        // Darf das Zeichen konvertiert werden?
        if( '\xff' == ch )
        {
            // Sonderzeichen: liegt dort ein Hint ohne Ende-Index?
            for( USHORT i = 0; i < hntsiz; ++i )
            {
                const SwTxtAttr *pPos = (*pHints)[ i ];
                const xub_StrLen nStart = *pPos->GetStart();
                if( pos == nStart && !pPos->GetEnd() )
                {
                    // Wenn ja, dann darf ch nicht konvertiert werden!
                    ch = 0;
                    break;
                }
                if( nStart > pos ) break;
            }
        }
        if( ch == '\t' || ch == '\n' ) ch = 0;
        // Testen, ob man sich im verbotenen Bereich befindet:
        if( ch && (symfrom != (xub_StrLen)-1 &&
                   pos >= symfrom && pos < symto) )
        {
            sal_Unicode cNew = ByteString::ConvertToUnicode( ch, eEnc );
            if( cNew )
                rNdText.SetChar( pos, cNew );
        }
    }
 }

// Attributsack mit einem anderen Sack fuellen. Dies ist der Fall,
// wenn auf Autoformate eines anderen Nodes Bezug genommen wird.

 void SwSwgReader::SetAttrSet( SwCntntNode& rNd, USHORT nIdx )
 {
    if( !rNd.GetpSwAttrSet() )
        rNd.NewAttrSet( pDoc->GetAttrPool() );
    FillAttrSet( *rNd.GetpSwAttrSet(), nIdx );
 }

// all textattributes can ask for here textcharacter. But in the node is the
// converted text, were they don't found the right characters. So they need
// a access to the original text, which will be done by this class. The read
// can be called recursiv!!
 class _SetOrigTextAtReader
 {
    ByteString sReadTxt;
    const ByteString* pOldTxt;
    SwSwgReader& rRdr;

 public:
    _SetOrigTextAtReader( SwSwgReader& rR, xub_StrLen nOffset )
        : rRdr( rR )
    {
        if( nOffset )
            sReadTxt.Expand( nOffset );
        pOldTxt = rRdr.GetReadTxt();
        rRdr.SetReadTxt( &sReadTxt );
    }

    ~_SetOrigTextAtReader()
    {
        rRdr.SetReadTxt( pOldTxt );
    }

    void AppendText( const ByteString& rTxt )   { sReadTxt += rTxt; }
    const ByteString& GetReadText() const       { return sReadTxt; }
 };


// Text Node einlesen
// Falls kein Node angegeben ist, wird ein neuer Node an der angegebenen
// Position erzeugt.
 void lcl_sw3io__ConvertNumTabStop( SwTxtNode& rTxtNd, long nOffset,
                                   BOOL bDeep );
 void SwSwgReader::FillTxtNode( SwTxtNode* pNd, SwNodeIndex& rPos,
                               xub_StrLen nOffset, USHORT nInsFirstPara )
 {
    long nextrec = r.getskip();

    // Flags-Aufbau:
    // Bit 0x01 - hat Collection-Wert
    // Bit 0x02 - hat eigenes Frameformat
    // Bit 0x04 - hat eigenes Zeichenformat
    // Bit 0x08 - hat eigenes Absatzformat
    // Bit 0x10 - hat Numerierung

    BYTE cFlags = 0, cNumLevel = NO_NUMBERING;
    USHORT nNumRule = IDX_NO_VALUE;
    USHORT nColl     = 0,
           nAutoFrm  = IDX_NO_VALUE,
           nAutoChar = IDX_NO_VALUE,
           nAutoPara = IDX_NO_VALUE;

    r >> cFlags;
    if( cFlags & 0x01 ) r >> nColl;
    if( cFlags & 0x02 ) r >> nAutoFrm;
    if( cFlags & 0x04 ) r >> nAutoChar;
    if( cFlags & 0x08 ) r >> nAutoPara;
    if( cFlags & 0x10 ) r >> cNumLevel >> nNumRule;

    // Werden nur Seitenvorlagen eingelesen, muss die Collection
    // immer 0 sein, da andere Coll-Idxe immer falsch sind
    if( ( nOptions != SWGRD_NORMAL ) && !( nOptions & SWGRD_PARAFMTS ) )
        nColl = 0;
    SwTxtFmtColl* pColl = (SwTxtFmtColl*) FindFmt( nColl | IDX_COLLECTION, 0 );
    if( !pColl )
        pColl = (SwTxtFmtColl*) FindFmt( IDX_COLLECTION + 0, 0 );

    BOOL bDone = FALSE;

    // Ggf. den Node neu einrichten
    BOOL bNewNd = FALSE;
    if( !pNd )
    {
        pNd = pDoc->GetNodes().MakeTxtNode( rPos, pColl );
        rPos--;
        nOffset = 0;
        bNewNd = TRUE;
    }
    else if( !nInsFirstPara )
        pNd->ChgFmtColl( pColl );

    _SetOrigTextAtReader aOrigTxtAtReader( *this, nOffset );

    SwAttrSet aTmpSet( pDoc->GetAttrPool(), RES_CHRATR_BEGIN, RES_CHRATR_END - 1 );

    r.next();
    while( !bDone && ( r.tell() < nextrec ) ) switch( r.cur() )
    {
        case SWG_COMMENT:
            if( r.tell() >= nextrec )
                bDone = TRUE;
            else
                r.skipnext();
            break;
        case SWG_FRAMEFMT:
            if( nInsFirstPara )
            {
                r.skipnext();
                nAutoFrm = IDX_NO_VALUE;
            }
            else
            {
                if( InAttrSet( *pNd ) == nAutoFrm ) nAutoFrm = IDX_NO_VALUE;
                // Wurde ein PageDesc-Attr gelesen? Dann das AttrSet des Nodes
                // im PageDescLink noch nachtragen!
                if( pPageLinks && !pPageLinks->pSet )
                {
                    pPageLinks->pSet = pNd->GetpSwAttrSet();
                    pPageLinks->cFmt = FALSE;
                }
            }
            break;
        case SWG_CHARFMT:
            if( nInsFirstPara )
            {
                if( InAttrSet( aTmpSet ) == nAutoChar )
                    nAutoChar = IDX_NO_VALUE;
            }
            else if( InAttrSet( *pNd ) == nAutoChar )
                nAutoChar = IDX_NO_VALUE;
            break;
        case SWG_PARAFMT:
            if( nInsFirstPara )
            {
                r.skipnext();
                nAutoPara = IDX_NO_VALUE;
            }
            else
                if( InAttrSet( *pNd ) == nAutoPara ) nAutoPara = IDX_NO_VALUE;
            break;
        case SWG_TEXT:
            {
                ByteString aTxt( r.text() );    // Hier nicht konvertieren!
                aOrigTxtAtReader.AppendText( aTxt );
                // The conversion that takes place may lead to wrong results.
                // This will be fixed in ConvertText.
                String aText( aTxt, aHdr.cCodeSet );
                r.next();
                if( bNewNd )
                    (String&) pNd->GetTxt() = aText;
                else {
                    SwIndex aOff( pNd, nOffset );
                    pNd->Insert( aText, aOff );
                }
            }
            break;
        case SWG_TEXTHINTS:
            InTextHints( *pNd, nOffset );
            break;
        case SWG_FLYFMT:
            InFlyFrame( &rPos );
            break;
        case SWG_BOOKMARK:
            if( !( nStatus & SWGSTAT_NO_BOOKMARKS ) )
                InNodeBookmark( rPos );
            else
                r.skipnext();
            break;
        case SWG_OUTLINE:
            // alter Numerierungs-Record, jetzt nicht mehr da!
            r.skipnext();
            break;
        default:
            bDone = TRUE;
    }

    if( aTmpSet.Count() )
    {
        if( 2 == nInsFirstPara )
            pNd->SetAttr( aTmpSet, 0, GetReadTxt()->Len() - nOffset );
        else
            pNd->SetAttr( aTmpSet, nOffset, pNd->GetTxt().Len() );
    }

    // Wird noch auf Autoformate Bezug genommen?
    if( nAutoFrm != IDX_NO_VALUE )
        SetAttrSet( *pNd, nAutoFrm );
    if( nAutoChar != IDX_NO_VALUE )
        SetAttrSet( *pNd, nAutoChar );
    if( nAutoPara != IDX_NO_VALUE )
        SetAttrSet( *pNd, nAutoPara );

    // Numerierung uebernehmen
    if( cNumLevel != NO_NUMBERING )
    {
        if( nNumRule != IDX_NO_VALUE )
            UpdateRuleRange( nNumRule, pNd );
        pNd->UpdateNum( SwNodeNum( cNumLevel ));
    }
 #ifdef NUM_RELSPACE
    else if( pNd->GetpSwAttrSet() &&
        NO_NUMBERING != pColl->GetOutlineLevel() &&
        pDoc->GetOutlineNumRule() )
    {
        const SwNumRule *pOutline = pDoc->GetOutlineNumRule();
        const SfxPoolItem *pItem;

        const SwNumFmt& rNumFmt = pOutline->Get(
                GetRealLevel(((const SwTxtFmtColl*)pColl)->GetOutlineLevel()) );
        USHORT nNumLSpace = rNumFmt.GetAbsLSpace();

        if( SFX_ITEM_SET == pNd->GetpSwAttrSet()->GetItemState( RES_LR_SPACE,
                                                              FALSE,
                                                              &pItem ) )
        {
            const SvxLRSpaceItem *pParaLRSpace =
                (const SvxLRSpaceItem *)pItem;

            USHORT nWishLSpace = pParaLRSpace->GetTxtLeft();
            USHORT nNewLSpace =
                        nWishLSpace > nNumLSpace ? nWishLSpace-nNumLSpace : 0U;

            const SvxLRSpaceItem& rCollLRSpace = pColl->GetLRSpace();
            if( nNewLSpace == rCollLRSpace.GetTxtLeft() &&
                pParaLRSpace->GetRight() == rCollLRSpace.GetRight() &&
                pParaLRSpace->GetTxtFirstLineOfst() ==
                                            rCollLRSpace.GetTxtFirstLineOfst() )
            {
                pNd->ResetAttr( RES_LR_SPACE );
            }
            else if( nNewLSpace != pParaLRSpace->GetTxtLeft() )
            {
                SvxLRSpaceItem aLRSpace( *pParaLRSpace );
                short nFirst = aLRSpace.GetTxtFirstLineOfst();
                if( nFirst < 0 && (USHORT)-nFirst > nNewLSpace )
                    aLRSpace.SetTxtFirstLineOfst( -(short)nNewLSpace );
                aLRSpace.SetTxtLeft( nNewLSpace );
                ((SwCntntNode *)pNd)->SetAttr( aLRSpace );
            }

            if( nWishLSpace != nNewLSpace )
                lcl_sw3io__ConvertNumTabStop( *pNd, (long)nWishLSpace -
                                                    (long)nNewLSpace, FALSE );
        }
        else if( nNumLSpace > 0 )
        {
            lcl_sw3io__ConvertNumTabStop( *pNd, nNumLSpace, FALSE   );
        }
    }
 #endif

    // Eventuell den Text konvertieren
    ConvertText( *pNd, (rtl_TextEncoding)aHdr.cCodeSet );
    rPos++;
 }

// Einlesen des puren Textes eines TextNodes. Der Text wird an den vorhandenen
// Text im String angefuegt.

 void SwSwgReader::FillString( String& rText )
 {
    // Flags-Aufbau:
    // Bit 0x01 - hat Collection-Wert
    // Bit 0x02 - hat eigenes Frameformat
    // Bit 0x04 - hat eigenes Zeichenformat
    // Bit 0x08 - hat eigenes Absatzformat
    // Bit 0x10 - hat Numerierung

    BYTE cFlags = 0;
    USHORT nDummyBytes = 0;
    String aTxt;
    long nextrec = r.getskip();

    r >> cFlags;
    if( cFlags & 0x01 ) nDummyBytes += 2;
    if( cFlags & 0x02 ) nDummyBytes += 2;
    if( cFlags & 0x04 ) nDummyBytes += 2;
    if( cFlags & 0x08 ) nDummyBytes += 2;
    if( cFlags & 0x10 ) nDummyBytes += 3;

    BOOL bDone = FALSE;
    r.seek( r.tell() + nDummyBytes );
    r.next();
    while( !bDone ) switch( r.cur() )
    {
        case SWG_PARAFMT:
        case SWG_CHARFMT:
        case SWG_FRAMEFMT:
        case SWG_TEXTHINTS:
        case SWG_FLYFMT:
        case SWG_BOOKMARK:
        case SWG_OUTLINE:
            r.skipnext(); break;
        case SWG_COMMENT:
            if( r.tell() >= nextrec ) bDone = TRUE;
            else r.skipnext(); break;
        case SWG_TEXT:
            aTxt = ParseText();
            if( rText.Len() ) rText += ' ';
            rText += aTxt;
            r.next();
            break;
        default:
            bDone = TRUE;
    }
 }

// Die Formate von Grafik- und OLE-Nodes muessen nicht registriert
// werden; die Layout-Frames erhalten ja eine Node-Referenz.

 void SwSwgReader::InGrfNode( SwNodeIndex& rPos )
 {
    Graphic aGrf;
    Graphic* pGrf = &aGrf;
    String aGrfName, aFltName;
    USHORT nFrmFmt = IDX_NO_VALUE;
    USHORT nGrfFmt = IDX_NO_VALUE;
    long nextrec = r.getskip();

    r >> nFrmFmt >> nGrfFmt;
    r.next();
    BOOL bLink = FALSE;
    BOOL bDone = FALSE;
    SwAttrSet aSet( pDoc->GetAttrPool(), RES_CHRATR_BEGIN, RES_GRFATR_END );
    while( !bDone ) switch( r.cur() )
    {
        case SWG_FRAMEFMT:
        case SWG_GRFFMT:
            InAttrSet( aSet );
            break;
        case SWG_GRAPHIC: {
            long pos = r.getskip();
            aGrfName = GetText();
            aFltName = GetText();
            if( !aGrfName.Len() )
            {
                long nPos = r.tell();
                if( nPos < pos )
                {
                    // Immer sofort einlesen (kann spaeter rausgeswapt werden)
                    if( aHdr.nVersion >= SWG_VER_PORTGRF )
                        aGrf.ReadEmbedded( r.Strm(), FALSE );
                    else
                        r.Strm() >> aGrf;
                }
            }
            else
            {
                pGrf = NULL, bLink = TRUE;
                aGrfName = ::binfilter::StaticBaseUrl::SmartRelToAbs( aGrfName );
            }
            r.skip( pos );
            r.next();
            if( r.good() ) break;
            }
        case SWG_COMMENT:
        case SWG_DATA:
            if( r.tell() >= nextrec ) bDone = TRUE;
            else r.skipnext(); break;
        default:
            bDone = TRUE;
    }

    if( r.good() )
        pDoc->GetNodes().MakeGrfNode( rPos,
                                  aGrfName, aFltName, pGrf,
                                  (SwGrfFmtColl*) pDoc->GetDfltGrfFmtColl(),
                                  &aSet, bLink );
    else
        Error( ERR_SWG_READ_ERROR );

 }

// OLE-Node:

 void SwSwgReader::InOleNode( SwNodeIndex& rPos )
 {
    USHORT nFrmFmt = IDX_NO_VALUE;
    USHORT nGrfFmt = IDX_NO_VALUE;
    long nextrec = r.getskip();

    r >> nFrmFmt >> nGrfFmt;
    r.next();
    SwAttrSet aSet( pDoc->GetAttrPool(), RES_CHRATR_BEGIN, RES_GRFATR_END );
    String aObjName;
    BOOL bDone = FALSE;
    while( !bDone ) switch( r.cur() )
    {
        case SWG_FRAMEFMT:
        case SWG_GRFFMT:
            InAttrSet( aSet );
            break;
        case SWG_OLE:
        {
            SvStorage* pStg = pDoc->GetPersist()->GetStorage();
            aObjName = Sw3Io::UniqueName( pStg, "StarObj" );
            ULONG nStart = r.Strm().Tell();
            if( !SvEmbeddedObject::InsertStarObject
                ( aObjName, aObjName, r.Strm(), pDoc->GetPersist() ) )
            {
                // Vielleicht geht es ja als Grafik...
                GDIMetaFile aMtf;
                r.Strm().Seek( nStart );
                if( SvEmbeddedObject::LoadStarObjectPicture( r.Strm(), aMtf ) )
                {
                    r.Strm().ResetError();
                    aObjName.Erase();
                    Graphic aGrf( aMtf );
                    pDoc->GetNodes().MakeGrfNode( rPos,
                                 aObjName, aObjName, &aGrf,
                                 (SwGrfFmtColl*) pDoc->GetDfltGrfFmtColl(),
                                 &aSet, FALSE );
                    nErrno = WARN_SWG_OLE | WARN_SW_READ_BASE;
                    return;
                }
                else
                    Error( ERR_SWG_READ_ERROR );
            }
            r.skipnext(); break;
        }
        case SWG_COMMENT:
        case SWG_DATA:
            if( r.tell() >= nextrec ) bDone = TRUE;
            else r.skipnext(); break;
        default:
            bDone = TRUE;
    }
    if ( !r )
    {
        Error( ERR_SWG_READ_ERROR );
    }
    else
        pDoc->GetNodes().MakeOLENode( rPos, aObjName,
                        (SwGrfFmtColl*) pDoc->GetDfltGrfFmtColl(),
                        &aSet );
 }

}
