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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <hintids.hxx>

#include <bf_svx/lrspitem.hxx>
#include <vcl/font.hxx>

#include <horiornt.hxx>

#include "doc.hxx"

#include <errhdl.hxx>

#include "pam.hxx"
#include "rdswg.hxx"
#include "ndtxt.hxx"
#include "numrule.hxx"
#include "poolfmt.hxx"
namespace binfilter {



//////////////////////////////////////////////////////////////////////////////

// Achtung: Jede Routine nimmt an, dass der Record-Header bereits eingelesen
// ist. Sie arbeitet den Record so weit ab wie moeglich und schliesst den
// Vorgang mit end() ab.

//////////////////////////////////////////////////////////////////////////////

// Numerierungs-Format
extern BYTE lcl_sw3io__GetIncludeUpperLevel( BOOL bInclUpperLevel );

void SwSwgReader::InNumFmt( SwNumFmt& rFmt )
{
    // Flags:
    // 0x01 - Praefix-String vorhanden
    // 0x02 - Postfix-String vorhanden
    // 0x04 - Bullet-Font vorhanden
    sal_Char bFlags, eType, cBullet, bInclUpperLevel, eNumAdjust;
    USHORT nFmt, nStart;
    String aFontName;
    sal_Char eFamily, ePitch, eCharSet = RTL_TEXTENCODING_SYMBOL;
    long nLSpace, nFirstLineOffset;
    if( aHdr.nVersion >= SWG_VER_COMPAT )
    {
        rFmt.SetPrefix( GetText() );
        rFmt.SetSuffix( GetText() );
        aFontName = GetText();
        BOOL bOk = TRUE;
        if( r.next() != SWG_DATA ) bOk = FALSE, Error();
        r >> bFlags
          >> nFmt
          >> eType
          >> cBullet
          >> bInclUpperLevel
          >> nStart
          >> eNumAdjust
          >> nLSpace
          >> nFirstLineOffset
          >> eFamily
          >> ePitch
          >> eCharSet;
        if( bOk ) r.skip();
    } else {
        r >> bFlags
          >> nFmt
          >> eType
          >> cBullet
          >> bInclUpperLevel
          >> nStart
          >> eNumAdjust
          >> nLSpace
          >> nFirstLineOffset;
        if( bFlags & 0x01 )
            rFmt.SetPrefix( GetText() );
        if( bFlags & 0x02 )
            rFmt.SetSuffix( GetText() );
        if( bFlags & 0x04 )
        {
            aFontName = GetText();
            r >> eFamily >> ePitch >> eCharSet;
        }
    }
    if( RTL_TEXTENCODING_DONTKNOW == eCharSet )
        eCharSet = RTL_TEXTENCODING_SYMBOL;
    sal_Unicode cBull = ByteString::ConvertToUnicode( cBullet, eCharSet );
    if( !cBull )
        cBull = cBulletChar;
    rFmt.SetNumberingType( eType );
    rFmt.SetBulletChar( cBull );
    rFmt.SetIncludeUpperLevels( lcl_sw3io__GetIncludeUpperLevel(bInclUpperLevel) );
    rFmt.SetStart( nStart );
    rFmt.SetNumAdjust( SvxAdjust( eNumAdjust ) );
    rFmt.SetAbsLSpace( (USHORT)nLSpace );
    rFmt.SetFirstLineOffset( (short)nFirstLineOffset );
    if( bFlags & 0x04 )
    {
        Font aFont;
        aFont.SetName( aFontName );
        aFont.SetFamily( FontFamily( eFamily ) );
        aFont.SetPitch( FontPitch( ePitch ) );
        aFont.SetCharSet( rtl_TextEncoding( eCharSet ) );
        rFmt.SetBulletFont( &aFont );
    }
    r.next();
}

// Numerierungs-Regel

SwNumRule* SwSwgReader::InNumRule()
{
    sal_Char eType, nFmt, cFmt[ MAXLEVEL ];
    r >> eType >> nFmt;
    // C 8.0 bug:
    SwNumRuleType eTemp = (SwNumRuleType) eType;
    SwNumRule* pRule = new SwNumRule( pDoc->GetUniqueNumRuleName(), eTemp );
    int i;
    for( i = 0; i < nFmt; i++ )
        r >> cFmt[ i ];
    r.next();
    for( i = 0; r.good() && i < nFmt; i++ )
    {
        SwNumFmt aFmt;
        if( r.cur() != SWG_NUMFMT )
        {
            Error(); delete pRule; return NULL;
        }
        aFmt.SetBulletFont( NULL );
        InNumFmt( aFmt );
        if( !r )
        {
            delete pRule; return NULL;
        }
        pRule->Set( (USHORT) cFmt[ i ], aFmt );
    }
    if( pRule )
    {
        if( aHdr.nVersion < SWG_VER_POOLID3 )
        {
            static short __READONLY_DATA aOldFI[ 5 ] =  { -227, -397, -567, -737, -936 };
            static short __READONLY_DATA aOldLft[ 5 ] = { 567, 964, 1474, 567*4, 3204 };

            // Alte Dokumente: Fehlende Formate nachtragen
            for( i = 0; i < MAXLEVEL; i++ )
                if( !pRule->GetNumFmt( i ) )
                {
                    SwNumFmt aFmt( pRule->Get( i ) );
                    aFmt.SetIncludeUpperLevels( MAXLEVEL );
                    aFmt.SetAbsLSpace( aOldLft[ i ] );
                    aFmt.SetFirstLineOffset( aOldFI[ i ] );
                    aFmt.SetNumberingType(SVX_NUM_ARABIC);
                    if( i )
                        aFmt.SetSuffix( aEmptyStr );
                    pRule->Set( i, aFmt );
                }
        }
    }
    return pRule;
}

// Outline-Numerierung


void SwSwgReader::InOutlineRule()
{
    SwNumRule* pRule = InNumRule();
    if( pRule )
    {
        pRule->SetName( String::CreateFromAscii(
                                SwNumRule::GetOutlineRuleName() ) );
        if( aHdr.nVersion < SWG_VER_POOLIDS )
        {
            // Alte Dokumente: Fehlende Formate nachtragen
            for( short i = 0; i < MAXLEVEL; i++ )
            {
                if( !pRule->GetNumFmt( i ) )
                {
                    SwNumFmt aFmt( pRule->Get( i ) );
                    aFmt.SetNumberingType(SVX_NUM_ARABIC);
                    pRule->Set( i, aFmt );
                }
            }
        }
        pDoc->SetOutlineNumRule( *pRule );
        delete pRule;
    }
}


// Numerierungs-Regelwerk
// Dieses Regelwerk wird nach dem Einlesen der Nodes eingelesen.
// Zu diesem Zeitpunkt sollte das Index-Array pRules bereits mit den
// gueltigen Bereichen fuer die Regeln gefuellt sein

// (siehe SwSwgReader::UpdateRuleRange())

void lcl_sw3io__ConvertNumLRSpace( SwTxtNode& rTxtNd, const SwNumRule& rNumRule,
                                   BYTE nLevel, BOOL bTabStop );

void SwSwgReader::InTxtNumRule()
{
    USHORT n;
    r >> n;
    if( n != nRules )
    {
        Error(); return;
    }
    r.next();
    for( USHORT i = 0; i < n; i++ )
    {
        SwTxtNode* pBgn = pRules[ i ].pBgn;
        SwTxtNode* pEnd = pRules[ i ].pEnd;
        if( r.cur() != SWG_NUMRULE || !pBgn )
        {
            Error(); return;
        }
        // Rules einlesen
        SwNumRule* pRule = InNumRule();
        if( !pRule ) break;

        SwPaM aPaM( *pBgn, 0, *pEnd, 0 );
        SwNodeIndex aBgnIdx( *pBgn );
        const SwNodeIndex& rEndIdx = aPaM.GetPoint()->nNode;
        // jetzt noch die Nodes mit den Rules verbinden
        BYTE nPrevLevel = 0;
        do {
            SwTxtNode* pNd = aBgnIdx.GetNode().GetTxtNode();
            if( pNd )
            {
                if( pNd->GetNum() )
                {
                    SwNodeNum *pNum = (SwNodeNum*) pNd->GetNum();

                    //JP 19.03.96: NO_NUM gibt es nicht mehr, dafuer gibts
                    //              jetzt NO_NUMLEVEL
                    if( NO_NUM == pNum->GetLevel() )
                        pNum->SetLevel( nPrevLevel | NO_NUMLEVEL );
                    else
                        nPrevLevel = pNum->GetLevel();
#ifdef NUM_RELSPACE
                    lcl_sw3io__ConvertNumLRSpace( *pNd, *pRule,
                                                   pNum->GetLevel(), TRUE );
#endif
                }
                else
                {
                    // Hot fix bei Loechern im Bereich:
                    pNd->UpdateNum( SwNodeNum( nPrevLevel | NO_NUMLEVEL ) );
                    ASSERT( FALSE, "SW/G-Reader: Luecke im NumRule-Bereich!" );
                }
            }
            else
            {
                Error(); break;
            }
            aBgnIdx += 1;
        } while( aBgnIdx.GetIndex() <= rEndIdx.GetIndex() );

        pDoc->SetNumRule( aPaM, *pRule );
        delete pRule;
    }
}

// Update des Bereichs einer Numerierungsregel

void SwSwgReader::UpdateRuleRange( USHORT nIdx, SwTxtNode* pNd )
{
    // Evtl. den Puffer fuer die NumRules-Bereiche einrichten
    if( !pRules && aHdr.nNumRules )
    {
        nRules = aHdr.nNumRules;
        pRules = new NumRuleInfo[ nRules ];
        for( USHORT i = 0; i < nRules; i++ )
            pRules[ i ].pBgn =
            pRules[ i ].pEnd = NULL;
    }
    if( nIdx >= nRules ) return;
    // Node-Nummer fuer PaM-SRange updaten:
    NumRuleInfo* p = &pRules[ nIdx ];
    if( !p->pBgn )
       p->pBgn = p->pEnd = pNd;
    else
       p->pEnd = pNd;
}



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
