/*************************************************************************
 *
 *  $RCSfile: wrtw8sty.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-21 12:55:21 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#define _SVSTDARR_STRINGSSORTDTOR

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif
#ifndef _SVDOBJ_HXX //autogen wg. SdrObject
#include <svx/svdobj.hxx>
#endif
#ifndef _SVDOTEXT_HXX //autogen wg. SdrTextObj
#include <svx/svdotext.hxx>
#endif
#ifndef _SVDOTEXT_HXX
#include <svx/svdotext.hxx>
#endif
#ifndef _SVX_FMGLOB_HXX
#include <svx/fmglob.hxx>
#endif
#ifndef _WRTWW8_HXX
#include <wrtww8.hxx>
#endif
#ifndef _DOC_HXX //autogen
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX //autogen
#include <docary.hxx>
#endif
#ifndef _POOLFMT_HXX //autogen
#include <poolfmt.hxx>
#endif
#ifndef _FMTCOL_HXX //autogen
#include <fmtcol.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _PAGEDESC_HXX //autogen
#include <pagedesc.hxx>
#endif
#ifndef _NDTXT_HXX //autogen
#include <ndtxt.hxx>
#endif
#ifndef _FTNINFO_HXX //autogen
#include <ftninfo.hxx>
#endif
#ifndef _FMTHDFT_HXX //autogen
#include <fmthdft.hxx>
#endif
#ifndef _SECTION_HXX //autogen
#include <section.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FMTFTN_HXX //autogen
#include <fmtftn.hxx>
#endif
#ifndef _NDINDEX_HXX //autogen
#include <ndindex.hxx>
#endif
#ifndef _TXTFTN_HXX
#include <txtftn.hxx>
#endif
#ifndef _CHARFMT_HXX
#include <charfmt.hxx>
#endif
#ifndef _DOCUFLD_HXX
#include <docufld.hxx>
#endif
#ifndef _DCONTACT_HXX
#include <dcontact.hxx>
#endif
#ifndef _FMTCNCT_HXX
#include <fmtcnct.hxx>
#endif
#ifndef _FTNIDX_HXX
#include <ftnidx.hxx>
#endif

#ifndef SW_LINEINFO_HXX
#include <lineinfo.hxx>
#endif

#ifndef _WW8PAR_HXX
#include <ww8par.hxx>
#endif


struct WW8_SED
{
    SVBT16 aBits1;      // orientation change + internal, Default: 6
    SVBT32 fcSepx;      //  FC  file offset to beginning of SEPX for section.
                        //  0xFFFFFFFF for no Sprms
    SVBT16 fnMpr;       //  used internally by Windows Word, Default: 0
    SVBT32 fcMpr;       //  FC, points to offset in FC space for MacWord
                        // Default: 0xffffffff ( nothing )
                        //  cbSED is 12 (decimal)), C (hex).
};

struct WW8_PdAttrDesc
{
    BYTE* pData;
    USHORT nLen;
    WW8_FC nSepxFcPos;
};


SV_IMPL_VARARR( WW8_WrSepInfoPtrs, WW8_SepInfo )

// class WW8_WrPlc0 ist erstmal nur fuer Header / Footer-Positionen, d.h. es gibt
// keine inhaltstragende Struktur.
class WW8_WrPlc0
{
    SvULongs aPos;      // PTRARR von CPs / FCs
    ULONG nOfs;
public:
    WW8_WrPlc0( ULONG nOffset );
    USHORT Count() const                { return aPos.Count(); }
    void Append( ULONG nStartCpOrFc );
    void Write( SvStream& rStrm );
    ULONG GetCP( USHORT n ) const       { return aPos[ n ]; }
};


//------------------------------------------------------------
//  Styles
//------------------------------------------------------------

#define WW8_RESERVED_SLOTS 15

// GetId( SwCharFmt ) zur Benutzung im Text -> nil verboten,
// "Default Char Style" stattdessen
USHORT SwWW8Writer::GetId( const SwCharFmt& rFmt ) const
{
    USHORT nRet = pStyles->Sty_GetWWSlot( rFmt );
    return ( nRet != 0xfff ) ? nRet : 10;       // Default Char Style
}

// GetId( SwTxtFmtColl ) zur Benutzung an TextNodes -> nil verboten,
// "Standard" stattdessen
USHORT SwWW8Writer::GetId( const SwTxtFmtColl& rColl ) const
{
    USHORT nRet = pStyles->Sty_GetWWSlot( rColl );
    return ( nRet != 0xfff ) ? nRet : 0;        // Default TxtFmtColl
}



//typedef pFmtT
WW8WrtStyle::WW8WrtStyle( SwWW8Writer& rWr )
    : rWrt( rWr ), nPOPosStdLen1( 0 ), nPOPosStdLen2( 0 )
{
    rWrt.pO->Remove( 0, rWrt.pO->Count() );             // leeren
    SwDoc& rDoc = *rWrt.pDoc;

    // if exist any Foot-/End-Notes then get from the EndNoteInfo struct
    // the CharFormats. They will create it!
    if( rDoc.GetFtnIdxs().Count() )
    {
        rDoc.GetEndNoteInfo().GetAnchorCharFmt( rDoc );
        rDoc.GetEndNoteInfo().GetCharFmt( rDoc );
        rDoc.GetFtnInfo().GetAnchorCharFmt( rDoc );
        rDoc.GetFtnInfo().GetCharFmt( rDoc );
    }
    USHORT nAlloc = WW8_RESERVED_SLOTS + rDoc.GetCharFmts()->Count() - 1 +
                                        rDoc.GetTxtFmtColls()->Count() - 1;
                    // etwas grosszuegig ( bis zu 15 frei )
    pFmtA = new SwFmt*[ nAlloc ];
    memset( pFmtA, 0, nAlloc * sizeof( SwFmt* ) );
    BuildStyleTab();
}

WW8WrtStyle::~WW8WrtStyle()
{
    delete[]( pFmtA );
    rWrt.pO->Remove( 0, rWrt.pO->Count() );             // leeren
}

// Sty_SetWWSlot() fuer Abhaengigkeiten der Styles -> nil ist erlaubt
USHORT WW8WrtStyle::Sty_GetWWSlot( const SwFmt& rFmt ) const
{
    register USHORT n;
    for( n = 0; n < nUsedSlots; n++ )
        if( pFmtA[n] == &rFmt )
            return n;
    return 0xfff;                   // 0xfff: WW: nil
}

USHORT WW8WrtStyle::Build_GetWWSlot( const SwFmt& rFmt )
{
    USHORT nRet;
    switch( nRet = rFmt.GetPoolFmtId() )
    {
    case RES_POOLCOLL_STANDARD:     nRet = 0;       break;

    case RES_POOLCOLL_HEADLINE1:
    case RES_POOLCOLL_HEADLINE2:
    case RES_POOLCOLL_HEADLINE3:
    case RES_POOLCOLL_HEADLINE4:
    case RES_POOLCOLL_HEADLINE5:
    case RES_POOLCOLL_HEADLINE6:
    case RES_POOLCOLL_HEADLINE7:
    case RES_POOLCOLL_HEADLINE8:
    case RES_POOLCOLL_HEADLINE9:    nRet -= RES_POOLCOLL_HEADLINE1-1;   break;

//  case RES_POOLCHR_FOOTNOTE_ANCHOR:   nRet =
//  case RES_POOLCHR_ENDNOTE_ANCHOR:
    default:
        nRet = nUsedSlots++;
        break;
    }
    return nRet;
}

USHORT WW8WrtStyle::GetWWId( const SwFmt& rFmt ) const
{
    USHORT nRet = 0xffe;        // User-Style als default
    USHORT nPoolId = rFmt.GetPoolFmtId();
    if( nPoolId == RES_POOLCOLL_STANDARD )
        nRet = 0;
    else if( nPoolId >= RES_POOLCOLL_HEADLINE1 &&
             nPoolId <= RES_POOLCOLL_HEADLINE9 )
        nRet = nPoolId + 1 - RES_POOLCOLL_HEADLINE1;
    else if( nPoolId >= RES_POOLCOLL_TOX_IDX1 &&
             nPoolId <= RES_POOLCOLL_TOX_IDX3 )
        nRet = nPoolId + 10 - RES_POOLCOLL_TOX_IDX1;
    else if( nPoolId >= RES_POOLCOLL_TOX_CNTNT1 &&
             nPoolId <= RES_POOLCOLL_TOX_CNTNT5 )
        nRet = nPoolId + 19 - RES_POOLCOLL_TOX_CNTNT1;
    else if( nPoolId >= RES_POOLCOLL_TOX_CNTNT6 &&
             nPoolId <= RES_POOLCOLL_TOX_CNTNT9 )
        nRet = nPoolId + 24 - RES_POOLCOLL_TOX_CNTNT6;
    else
        switch( nPoolId )
        {
        case RES_POOLCOLL_FOOTNOTE:         nRet = 29;  break;
        case RES_POOLCOLL_HEADER:           nRet = 31;  break;
        case RES_POOLCOLL_FOOTER:           nRet = 32;  break;
        case RES_POOLCOLL_TOX_IDXH:         nRet = 33;  break;
        case RES_POOLCOLL_JAKETADRESS:      nRet = 36;  break;
        case RES_POOLCOLL_SENDADRESS:       nRet = 37;  break;
        case RES_POOLCOLL_ENDNOTE:          nRet = 43;  break;
        case RES_POOLCOLL_LISTS_BEGIN:      nRet = 47;  break;
        case RES_POOLCOLL_DOC_TITEL:        nRet = 62;  break;
        case RES_POOLCOLL_SIGNATURE:        nRet = 64;  break;
        case RES_POOLCOLL_TEXT:             nRet = 66;  break;
        case RES_POOLCOLL_TEXT_MOVE:        nRet = 67;  break;
        case RES_POOLCOLL_DOC_SUBTITEL:     nRet = 74;  break;
        case RES_POOLCOLL_TEXT_IDENT:       nRet = 77;  break;

        case RES_POOLCHR_FOOTNOTE_ANCHOR:   nRet = 38;  break;
        case RES_POOLCHR_ENDNOTE_ANCHOR:    nRet = 42;  break;
        case RES_POOLCHR_INET_NORMAL:       nRet = 85;  break;
        case RES_POOLCHR_INET_VISIT:        nRet = 86;  break;
        case RES_POOLCHR_HTML_STRONG:       nRet = 87;  break;
        case RES_POOLCHR_HTML_EMPHASIS:     nRet = 88;  break;
        case RES_POOLCHR_LINENUM:           nRet = 40;  break;
        case RES_POOLCHR_PAGENO:            nRet = 41;  break;
        }
    return nRet;
}

void WW8WrtStyle::BuildStyleTab()
{
    nUsedSlots = WW8_RESERVED_SLOTS;    // soviele sind reserviert fuer
                                        // Standard und HeadingX u.a.
    SwFmt* pFmt;
    USHORT n;
    const SvPtrarr& rArr = *rWrt.pDoc->GetCharFmts();       // erst CharFmt
        // das Default-ZeichenStyle ( 0 ) wird nicht mit ausgegeben !
    for( n = 1; n < rArr.Count(); n++ )
    {
        pFmt = (SwFmt*)rArr[n];
        pFmtA[ Build_GetWWSlot( *pFmt ) ] = pFmt;
    }
    const SvPtrarr& rArr2 = *rWrt.pDoc->GetTxtFmtColls();   // dann TxtFmtColls
        // das Default-TextStyle ( 0 ) wird nicht mit ausgegeben !
    for( n = 1; n < rArr2.Count(); n++ )
    {
        pFmt = (SwFmt*)rArr2[n];
        pFmtA[ Build_GetWWSlot( *pFmt ) ] = pFmt;
    }
}

void WW8WrtStyle::WriteStyle( SvStream& rStrm )
{
    WW8Bytes* pO = rWrt.pO;

    short nLen = pO->Count() - 2;            // Laenge des Styles
    BYTE* p = (BYTE*)pO->GetData() + nPOPosStdLen1;
    ShortToSVBT16( nLen, p );               // nachtragen
    p = (BYTE*)pO->GetData() + nPOPosStdLen2;
    ShortToSVBT16( nLen, p );               // dito

    rStrm.Write( pO->GetData(), pO->Count() );      // ins File damit
    pO->Remove( 0, pO->Count() );                   // leeren fuer naechsten
}


void WW8WrtStyle::BuildStd( const String& rName, BOOL bPapFmt, short nWwBase,
                             short nWwNext, USHORT nWwId )
{
    BYTE aWW8_STD[ sizeof( WW8_STD ) ];
    BYTE* pData = aWW8_STD;
    memset( &aWW8_STD, 0, sizeof( WW8_STD ) );

    UINT16 nBit16 = 0x1000;         // fInvalHeight
    nBit16 |= (0x0FFF & nWwId);
    Set_UInt16( pData, nBit16 );

    nBit16 = nWwBase << 4;          // istdBase
    nBit16 |= bPapFmt ? 1 : 2;      // sgc
    Set_UInt16( pData, nBit16 );

    nBit16 = nWwNext << 4;          // istdNext
    nBit16 |= bPapFmt ? 2 : 1;      // cupx
    Set_UInt16( pData, nBit16 );

    pData += sizeof( UINT16 );      // bchUpe

    if( rWrt.bWrtWW8 )
    {
        //-------- jetzt neu:
        // ab Ver8 gibts zwei Felder mehr:
        //UINT16    fAutoRedef : 1;    /* auto redefine style when appropriate */
        //UINT16    fHidden : 1;       /* hidden from UI? */
        //UINT16    : 14;              /* unused bits */
        pData += sizeof( UINT16 );
    }


    UINT16 nLen = ( pData - aWW8_STD ) + 1 +
                ((rWrt.bWrtWW8 ? 2 : 1 ) * (rName.Len() + 1));  // vorlaeufig

    WW8Bytes* pO = rWrt.pO;
    nPOPosStdLen1 = pO->Count();        // Adr1 zum nachtragen der Laenge

    SwWW8Writer::InsUInt16( *pO, nLen );
    pO->Insert( aWW8_STD, ( pData - aWW8_STD ), pO->Count() );

    nPOPosStdLen2 = nPOPosStdLen1 + 8;  // Adr2 zum nachtragen von "end of upx"

    // Namen schreiben
    if( rWrt.bWrtWW8 )
    {
        SwWW8Writer::InsUInt16( *pO, rName.Len() ); // Laenge
        SwWW8Writer::InsAsString16( *pO, rName );
    }
    else
    {
        pO->Insert( (BYTE)rName.Len(), pO->Count() );       // Laenge
        SwWW8Writer::InsAsString8( *pO, rName, RTL_TEXTENCODING_MS_1252 );
    }
    pO->Insert( (BYTE)0, pO->Count() );             // Trotz P-String 0 am Ende!
}

void WW8WrtStyle::SkipOdd()     // Ruecke zu gerader Adresse vor
{
    WW8Bytes* pO = rWrt.pO;
    if( ( rWrt.pTableStrm->Tell() + pO->Count() ) & 1 )     // Start auf gerader
        pO->Insert( (BYTE)0, pO->Count() );         // Adresse
}

void WW8WrtStyle::Set1StyleDefaults( const SwFmt& rFmt, BOOL bPap )
{
    BOOL aFlags[ RES_FRMATR_END - RES_CHRATR_BEGIN ];
    USHORT nStt, nEnd, n;
    if( bPap )
       nStt = RES_PARATR_BEGIN, nEnd = RES_FRMATR_END;
    else
       nStt = RES_CHRATR_BEGIN, nEnd = RES_TXTATR_END;

    // dynamic defaults
    const SfxItemPool& rPool = *rFmt.GetAttrSet().GetPool();
    for( n = nStt; n < nEnd; ++n )
        aFlags[ n - RES_CHRATR_BEGIN ] = 0 != rPool.GetPoolDefaultItem( n );

    // static defaults, that differs between WinWord and SO
    if( bPap )
    {
        aFlags[ RES_PARATR_WIDOWS - RES_CHRATR_BEGIN ] = 1;
        aFlags[ RES_PARATR_HYPHENZONE - RES_CHRATR_BEGIN ] = 1;
    }
    else
    {
           aFlags[ RES_CHRATR_FONTSIZE - RES_CHRATR_BEGIN ] = 1;
        aFlags[ RES_CHRATR_LANGUAGE - RES_CHRATR_BEGIN ] = 1;
    }

    const SfxItemSet* pOldI = rWrt.GetCurItemSet();
    rWrt.SetCurItemSet( &rFmt.GetAttrSet() );

    const BOOL* pFlags = aFlags + ( nStt - RES_CHRATR_BEGIN );
    for( n = nStt; n < nEnd; ++n, ++pFlags )
        if( *pFlags && SFX_ITEM_SET != rFmt.GetItemState( n, FALSE ))
            Out( aWW8AttrFnTab, rFmt.GetAttr( n, TRUE ), rWrt );

    rWrt.SetCurItemSet( pOldI );
}

void WW8WrtStyle::BuildUpx( const SwFmt* pFmt, BOOL bPap, USHORT nPos,
                            BOOL bInsDefCharSiz )
{
    WW8Bytes* pO = rWrt.pO;

    SkipOdd();
    UINT16 nLen = ( bPap ) ? 2 : 0;             // Default-Laenge
    USHORT nLenPos = pO->Count();               // Laenge zum Nachtragen
                                    // Keinen Pointer merken, da sich bei
                                    // _grow der Pointer aendert !

    SwWW8Writer::InsUInt16( *pO, nLen );        // Style-Len

    UINT16 nStartSiz = pO->Count();

    if( bPap )
        SwWW8Writer::InsUInt16( *pO, nPos);     // Style-Nummer

    rWrt.Out_SwFmt( *pFmt, bPap, !bPap );

    if( bInsDefCharSiz  )                   // nicht abgeleitet v. anderem Style
        Set1StyleDefaults( *pFmt, bPap );

    nLen = pO->Count() - nStartSiz;
    BYTE* pUpxLen = (BYTE*)pO->GetData() + nLenPos; // Laenge zum Nachtragen
    ShortToSVBT16( nLen, pUpxLen );                 // Default-Laenge eintragen
}

// Out1Style geht fuer TxtFmtColls und CharFmts
void WW8WrtStyle::Out1Style( SwFmt* pFmt, USHORT nPos )
{
    if( pFmt )
    {
        BOOL bFmtColl = pFmt->Which() == RES_TXTFMTCOLL ||
                        pFmt->Which() == RES_CONDTXTFMTCOLL;
        short nWwBase = 0xfff;                  // Default: none

        if( !pFmt->IsDefault() )                // Abgeleitet von ?
            nWwBase = Sty_GetWWSlot( *pFmt->DerivedFrom() );

        SwFmt* pNext;
        if( bFmtColl )
            pNext = &((SwTxtFmtColl*)pFmt)->GetNextTxtFmtColl();
        else
            pNext = pFmt;       // CharFmt: Naechstes CharFmt == Selbes

        short nWwNext = Sty_GetWWSlot( *pNext );

        BuildStd( pFmt->GetName(), bFmtColl, nWwBase, nWwNext,
                    GetWWId( *pFmt ) );
        if( bFmtColl )
            BuildUpx( pFmt, TRUE, nPos, nWwBase==0xfff );           // UPX.papx
        BuildUpx( pFmt, FALSE, nPos, bFmtColl && nWwBase==0xfff );  // UPX.chpx

        SkipOdd();
        WriteStyle( *rWrt.pTableStrm );
    }
    else if( nPos == 10 )           // Default Char-Style ( nur WW )
    {
        if( rWrt.bWrtWW8 )
        {
            static BYTE __READONLY_DATA aDefCharSty[] = {
                0x42, 0x00,
                0x41, 0x40, 0xF2, 0xFF, 0xA1, 0x00, 0x42, 0x00,
                0x00, 0x00, 0x19, 0x00, 0x41, 0x00, 0x62, 0x00,
                0x73, 0x00, 0x61, 0x00, 0x74, 0x00, 0x7A, 0x00,
                0x2D, 0x00, 0x53, 0x00, 0x74, 0x00, 0x61, 0x00,
                0x6E, 0x00, 0x64, 0x00, 0x61, 0x00, 0x72, 0x00,
                0x64, 0x00, 0x73, 0x00, 0x63, 0x00, 0x68, 0x00,
                0x72, 0x00, 0x69, 0x00, 0x66, 0x00, 0x74, 0x00,
                0x61, 0x00, 0x72, 0x00, 0x74, 0x00, 0x00, 0x00,
                0x00, 0x00 };
            rWrt.pTableStrm->Write( &aDefCharSty, sizeof( aDefCharSty ) );
        }
        else
        {
            static BYTE __READONLY_DATA aDefCharSty[] = {
                0x26, 0x00,
                0x41, 0x40, 0xF2, 0xFF, 0xA1, 0x00, 0x26, 0x00,
                0x19, 0x41, 0x62, 0x73, 0x61, 0x74, 0x7A, 0x2D,
                0x53, 0x74, 0x61, 0x6E, 0x64, 0x61, 0x72, 0x64,
                0x73, 0x63, 0x68, 0x72, 0x69, 0x66, 0x74, 0x61,
                0x72, 0x74, 0x00, 0x00, 0x00, 0x00 };
            rWrt.pTableStrm->Write( &aDefCharSty, sizeof( aDefCharSty ) );
        }
    }
/*  else if( nPos == WW8_CSTY_FTN )     // Ftn-CharStyle ( nur WW )
    {
        if( rWrt.bWrtWW8 )
        {
            static BYTE __READONLY_DATA aFtnCharSty[] = {
                0x32, 0x00,
                0x26, 0x40, 0xA2, 0x00, 0x01, 0x01, 0x32, 0x00,
                0x00, 0x00, 0x0F, 0x00, 0x46, 0x00, 0x75, 0x00,
                0xDF, 0x00, 0x6E, 0x00, 0x6F, 0x00, 0x74, 0x00,
                0x65, 0x00, 0x6E, 0x00, 0x7A, 0x00, 0x65, 0x00,
                0x69, 0x00, 0x63, 0x00, 0x68, 0x00, 0x65, 0x00,
                0x6E, 0x00, 0x00, 0x00, 0x03, 0x00, 0x48, 0x2A,
                0x01, 0x00 };
            rWrt.pTableStrm->Write( &aFtnCharSty, sizeof( aFtnCharSty ) );
        }
        else
        {
            static BYTE __READONLY_DATA aFtnCharSty[] = {
                0x1E, 0x00,
                0x26, 0x40, 0xA2, 0x00, 0x01, 0x01, 0x1E, 0x00,
                0x0F, 0x46, 0x75, 0xDF, 0x6E, 0x6F, 0x74, 0x65,
                0x6E, 0x7A, 0x65, 0x69, 0x63, 0x68, 0x65, 0x6E,
                0x00, 0x00, 0x02, 0x00, 0x68, 0x01 };
            rWrt.pTableStrm->Write( &aFtnCharSty, sizeof( aFtnCharSty ) );
        }
    }
*/  else
    {
        UINT16 n = 0;
        rWrt.pTableStrm->Write( &n , 2 );   // leerer Style
    }
}

void WW8WrtStyle::OutStyleTab()
{
    WW8Fib& rFib = *rWrt.pFib;

    ULONG nCurPos = rWrt.pTableStrm->Tell();
    if( nCurPos & 1 )                   // Start auf gerader
    {
        *rWrt.pTableStrm << (char)0;        // Adresse
        ++nCurPos;
    }
    rWrt.bStyDef = TRUE;
    rFib.fcStshfOrig = rFib.fcStshf = nCurPos;
    ULONG nStyAnzPos = nCurPos + 2;     // Anzahl wird nachgetragen

    if( rWrt.bWrtWW8 )
    {
        static BYTE __READONLY_DATA aStShi[] = {
            0x12, 0x00,
            0x0F, 0x00, 0x0A, 0x00, 0x01, 0x00, 0x5B, 0x00,
            0x0F, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00 };

        rWrt.pTableStrm->Write( &aStShi, sizeof( aStShi ) );
    }
    else
    {
        static BYTE __READONLY_DATA aStShi[] = {
            0x0E, 0x00,
            0x0F, 0x00, 0x08, 0x00, 0x01, 0x00, 0x4B, 0x00,
            0x0F, 0x00, 0x00, 0x00, 0x00, 0x00 };
        rWrt.pTableStrm->Write( &aStShi, sizeof( aStShi ) );
    }

    USHORT n;
    for( n = 0; n < nUsedSlots; n++ )
        Out1Style( pFmtA[n], n );

    rFib.lcbStshfOrig = rFib.lcbStshf = rWrt.pTableStrm->Tell() - rFib.fcStshf;
    SwWW8Writer::WriteShort( *rWrt.pTableStrm, nStyAnzPos, nUsedSlots );
    rWrt.bStyDef = FALSE;
}


/*  */

//---------------------------------------------------------------------------
//          Fonts
//---------------------------------------------------------------------------

USHORT SwWW8Writer::GetId( const SvxFontItem& rFont ) const
{
    const SfxItemPool& rPool = pDoc->GetAttrPool();
    const SvxFontItem* pFont = (const SvxFontItem*)GetDfltAttr( RES_CHRATR_FONT );
    USHORT n = 3;
    if( rFont == *pFont )
        return n;

    ++n;
    if( 0 != ( pFont = (const SvxFontItem*)rPool.GetPoolDefaultItem(
                                                        RES_CHRATR_FONT )))
    {
        if( rFont == *pFont )
            return n;
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

    ASSERT( !this, "Font nicht in der Tabelle" );
    return 0;
}

USHORT SwWW8Writer::GetId( const Font& rFont ) const
{
    return GetId( SvxFontItem( rFont.GetFamily(), rFont.GetName(),
                                rFont.GetStyleName(), rFont.GetPitch(),
                                rFont.GetCharSet() ) );
}

static void _OutFont( SwWW8Writer& rWrt, const SvxFontItem& rFont )
{
    BYTE aWW8_FFN[ 6 ];
    memset( aWW8_FFN, 0, 6 );                       // 6 == len of fixed Part
    if( rWrt.bWrtWW8 )
        aWW8_FFN[0] = (BYTE)( 6 - 1 + 0x22 + ( 2 *
                                    ( 1 + rFont.GetFamilyName().Len() ) ));
    else
        aWW8_FFN[0] = (BYTE)( 6 - 1 + 1 + rFont.GetFamilyName().Len() );

    BYTE aB = 0;
    switch( rFont.GetPitch() )
    {
    case PITCH_VARIABLE: aB |= 2; break;    // aF.prg = 2
    case PITCH_FIXED:    aB |= 1; break;
    default:             break;         // aF.prg = 0 : DEFAULT_PITCH (windows.h)
    }
    aB |= 1 << 2;                       // aF.fTrueType = 1; weiss ich nicht besser;

    switch( rFont.GetFamily() )
    {
    case FAMILY_ROMAN:      aB |= 1 << 4; break; // aF.ff = 1;
    case FAMILY_SWISS:      aB |= 2 << 4; break; // aF.ff = 2;
    case FAMILY_MODERN:     aB |= 3 << 4; break; // aF.ff = 3;
    case FAMILY_SCRIPT:     aB |= 4 << 4; break; // aF.ff = 4;
    case FAMILY_DECORATIVE: aB |= 5 << 4; break; // aF.ff = 5;
    default:                              break; // aF.ff = 0; FF_DONTCARE (windows.h)
    }
    aWW8_FFN[1] = aB;

    ShortToSVBT16( 400, &aWW8_FFN[2] );         // weiss ich nicht besser
                                                // 400 == FW_NORMAL (windows.h)
    if( RTL_TEXTENCODING_SYMBOL == rFont.GetCharSet() )
        aWW8_FFN[4] = 2;
    else
        aWW8_FFN[4] = 0;

    rWrt.pTableStrm->Write( aWW8_FFN, 6 );                  // fixed part
    if( rWrt.bWrtWW8 )
    {
        // ab Ver8 sind folgende beiden Felder eingeschoben,
        // werden von uns ignoriert.
        //char  panose[ 10 ];       //  0x6   PANOSE
        //char  fs[ 24     ];       //  0x10  FONTSIGNATURE
        SwWW8Writer::FillCount( *rWrt.pTableStrm, 0x22 );
        SwWW8Writer::WriteString16( *rWrt.pTableStrm, rFont.GetFamilyName(),
                                    TRUE );
    }
    else
        SwWW8Writer::WriteString8( *rWrt.pTableStrm, rFont.GetFamilyName(),
                                    TRUE, RTL_TEXTENCODING_MS_1252 );
}

void SwWW8Writer::OutFontTab( WW8Fib& rFib )
{
    rFib.fcSttbfffn = pTableStrm->Tell();
    if( bWrtWW8 )
    {
        SwWW8Writer::WriteLong( *pTableStrm, 0 );       // hier wird Laenge nachgetragen ( cbFFN )

        // ab Ver8 sind folgende beiden Felder eingeschoben,
        // werden von uns ignoriert.
        //char  panose[ 10 ];       //  0x6   PANOSE
        //char  fs[ 24     ];       //  0x10  FONTSIGNATURE

        static BYTE __READONLY_DATA aBase_10[] = {
                    0x47,0x16,0x90,0x01,0x00,0x00 };
        static BYTE __READONLY_DATA aBase_11[] = {
            'T',0,'i',0,'m',0,'e',0,'s',0,' ',0,
            'N',0,'e',0,'w',0,' ',0,'R',0,'o',0,'m',0,'a',0,'n',0,0,0 };

        static BYTE __READONLY_DATA aBase_20[] = {
                    0x35,0x16,0x90,0x01,0x02,0x00 };
        static BYTE __READONLY_DATA aBase_21[] = {
            'S',0,'y',0,'m',0,'b',0,'o',0,'l',0,0,0 };

        static BYTE __READONLY_DATA aBase_30[] = {
                    0x33,0x26,0x90,0x01,0x00,0x00 };
        static BYTE __READONLY_DATA aBase_31[] = {
            'A',0,'r',0,'i',0,'a',0,'l',0,0,0 };

        // Base part
        pTableStrm->Write( aBase_10, sizeof( aBase_10 ) );
        SwWW8Writer::FillCount( *pTableStrm, 0x22 );
        pTableStrm->Write( aBase_11, sizeof( aBase_11 ) );

        pTableStrm->Write( aBase_20, sizeof( aBase_20 ) );
        SwWW8Writer::FillCount( *pTableStrm, 0x22 );
        pTableStrm->Write( aBase_21, sizeof( aBase_21 ) );

        pTableStrm->Write( aBase_30, sizeof( aBase_30 ) );
        SwWW8Writer::FillCount( *pTableStrm, 0x22 );
        pTableStrm->Write( aBase_31, sizeof( aBase_31 ) );
    }
    else
    {
        SwWW8Writer::WriteShort( *pTableStrm, 0 );      // hier wird Laenge nachgetragen ( cbFFN )

        static BYTE __READONLY_DATA aBase[] = {
            0x15,0x16,0x90,0x01,0x00,0x00,'T','i','m','e','s',' ',
                                    'N','e','w',' ','R','o','m','a','n',0,
            0x0c,0x16,0x90,0x01,0x02,0x00,'S','y','m','b','o','l',0,
            0x0b,0x26,0x90,0x01,0x00,0x00,'A','r','i','a','l',0 };
        pTableStrm->Write( aBase, sizeof( aBase ) );        // Base part
    }

    const SfxItemPool& rPool = pDoc->GetAttrPool();
    const SvxFontItem* pFont = (const SvxFontItem*)GetDfltAttr( RES_CHRATR_FONT );
    const SvxFontItem* pDfltFont = (const SvxFontItem*)rPool.GetPoolDefaultItem(
                                    RES_CHRATR_FONT );

    _OutFont( *this, *pFont );
    USHORT nFontCount = 4;

    if( pDfltFont )
    {
        _OutFont( *this, *pDfltFont );
        ++nFontCount;
    }

    USHORT nMaxItem = rPool.GetItemCount( RES_CHRATR_FONT );
    for( USHORT nGet = 0; nGet < nMaxItem; ++nGet )
    {
        pFont = (const SvxFontItem*)rPool.GetItem( RES_CHRATR_FONT, nGet );
        if( pFont )
        {
            _OutFont( *this, *pFont );
            ++nFontCount;
        }
    }
    rFib.lcbSttbfffn = pTableStrm->Tell() - rFib.fcSttbfffn;
    if( bWrtWW8 )
        SwWW8Writer::WriteLong( *pTableStrm, rFib.fcSttbfffn, nFontCount );
    else
        SwWW8Writer::WriteShort( *pTableStrm, rFib.fcSttbfffn,
                                (INT16)pFib->lcbSttbfffn );
}


/*  */

WW8_WrPlc0::WW8_WrPlc0( ULONG nOffset )
    : aPos( 4, 4 ), nOfs( nOffset )
{
}

void WW8_WrPlc0::Append( ULONG nStartCpOrFc )
{
    aPos.Insert( nStartCpOrFc - nOfs, aPos.Count() );
}

void WW8_WrPlc0::Write( SvStream& rStrm )
{
    USHORT nLen = aPos.Count();
    for( USHORT i = 0; i < nLen; ++i )
    {
        SVBT32 nP;
        LongToSVBT32( aPos[i], nP );
        rStrm.Write( nP, 4 );
    }
}

//------------------------------------------------------------------------------

/*  */
//------------------------------------------------------------------------------
// class WW8_WrPlcSepx : Uebersetzung PageDescs in Sections
//      behandelt auch Header und Footer
//------------------------------------------------------------------------------

WW8_WrPlcSepx::WW8_WrPlcSepx()
    : pTxtPos( 0 ), aCps( 4, 4 ), aSects( 4, 4 ), pAttrs( 0 )
{
}

WW8_WrPlcSepx::~WW8_WrPlcSepx()
{
    USHORT nLen = aSects.Count();
    if( pAttrs )
    {
        while( nLen )
        {
            delete[] pAttrs[ --nLen ].pData;
        }
        delete[] pAttrs;
    }
    delete pTxtPos;
}

void WW8_WrPlcSepx::AppendSep( WW8_CP nStartCp,
                               const SwPageDesc* pPd,
                               const SwSectionFmt* pSectionFmt,
                               ULONG nLnNumRestartNo )
{
    aCps.Insert( nStartCp, aCps.Count() );
    aSects.Insert( WW8_SepInfo( pPd, pSectionFmt, nLnNumRestartNo ),
                   aSects.Count() );
}

void WW8_WrPlcSepx::AppendSep( WW8_CP nStartCp, const SwFmtPageDesc& rPD,
                               const SwNode& rNd, ULONG nLnNumRestartNo )
{
    aCps.Insert( nStartCp, aCps.Count() );
    WW8_SepInfo aI( rPD.GetPageDesc(), 0, nLnNumRestartNo );
    aI.nPgRestartNo = rPD.GetNumOffset();
    aI.pPDNd = &rNd;
    aSects.Insert( aI, aSects.Count() );
}

// WW8_WrPlcSepx::SetNum() setzt in jeder Section beim 1. Aufruf den
// Num-Pointer, alle folgenden Aufrufe werden ignoriert. Damit wird
// die erste Aufzaehlung einer Section uebernommen.

void WW8_WrPlcSepx::SetNum( const SwTxtNode* pNumNd )
{
    WW8_SepInfo& rInfo = aSects[ aSects.Count() - 1 ];
    if( !rInfo.pNumNd ) // noch nicht belegt
        rInfo.pNumNd = pNumNd;
}

void WW8_WrPlcSepx::WriteOlst( SwWW8Writer& rWrt, USHORT i )
{
    if( !rWrt.bWrtWW8 )
    {
        const SwNumRule* pRule;
        const SwTxtNode* pNd = aSects[i].pNumNd;
        if( pNd && 0 != ( pRule = pNd->GetNumRule() ))
            rWrt.Out_Olst( *pRule );
    }
}


void WW8_WrPlcSepx::WriteFtnEndTxt( SwWW8Writer& rWrt, ULONG nCpStt )
{
    BYTE nInfoFlags = 0;
    const SwFtnInfo& rInfo = rWrt.pDoc->GetFtnInfo();
    if( rInfo.aErgoSum.Len() )  nInfoFlags |= 0x02;
    if( rInfo.aQuoVadis.Len() ) nInfoFlags |= 0x04;

    BYTE nEmptyStt = rWrt.bWrtWW8 ? 0 : 6;
    if( nInfoFlags )
    {
        if( rWrt.bWrtWW8 )
            pTxtPos->Append( nCpStt );  // empty footenote separator

        if( 0x02 & nInfoFlags )         // Footenote contiunation separator
        {
            pTxtPos->Append( nCpStt );
            rWrt.WriteStringAsPara( rInfo.aErgoSum );
            rWrt.WriteStringAsPara( aEmptyStr );
            nCpStt = rWrt.Fc2Cp( rWrt.Strm().Tell() );
        }
        else if( rWrt.bWrtWW8 )
            pTxtPos->Append( nCpStt );

        if( 0x04 & nInfoFlags )         // Footenote contiunation notice
        {
            pTxtPos->Append( nCpStt );
            rWrt.WriteStringAsPara( rInfo.aQuoVadis );
            rWrt.WriteStringAsPara( aEmptyStr );
            nCpStt = rWrt.Fc2Cp( rWrt.Strm().Tell() );
        }
        else if( rWrt.bWrtWW8 )
            pTxtPos->Append( nCpStt );

        if( rWrt.bWrtWW8 )
            nEmptyStt = 3;
        else
            rWrt.pDop->grpfIhdt = nInfoFlags;
    }

    while( 6 > nEmptyStt++ )
        pTxtPos->Append( nCpStt );

    // gleich die Flags am Dop setzen
    WW8Dop& rDop = *rWrt.pDop;
    // Footnote Info
    switch( rInfo.eNum )
    {
    case FTNNUM_PAGE:       rDop.rncFtn = 2; break;
    case FTNNUM_CHAPTER:    rDop.rncFtn  = 1; break;
    default: rDop.rncFtn  = 0; break;
    }                                   // rncFtn
    rDop.nfcFtnRef = SwWW8Writer::GetNumId( rInfo.aFmt.eType );
    rDop.nFtn = rInfo.nFtnOffset + 1;
    rDop.fpc = rWrt.bFtnAtTxtEnd ? 2 : 1;

    // Endnote Info
    rDop.rncEdn = 0;                        // rncEdn: Don't Restart
    const SwEndNoteInfo& rEndInfo = rWrt.pDoc->GetEndNoteInfo();
    rDop.nfcEdnRef = SwWW8Writer::GetNumId( rEndInfo.aFmt.eType );
    rDop.nEdn = rEndInfo.nFtnOffset + 1;
    rDop.epc = rWrt.bEndAtTxtEnd ? 3 : 0;
}


void WW8_WrPlcSepx::SetHeaderFlag( BYTE& rHeadFootFlags, const SwFmt& rFmt,
                                   BYTE nFlag )
{
    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET == rFmt.GetItemState( RES_HEADER, TRUE, &pItem )
        && ((SwFmtHeader*)pItem)->IsActive() &&
        ((SwFmtHeader*)pItem)->GetHeaderFmt() )
        rHeadFootFlags |= nFlag;
}
void WW8_WrPlcSepx::SetFooterFlag( BYTE& rHeadFootFlags, const SwFmt& rFmt,
                                   BYTE nFlag )
{
    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET == rFmt.GetItemState( RES_FOOTER, TRUE, &pItem )
        && ((SwFmtFooter*)pItem)->IsActive() &&
        ((SwFmtFooter*)pItem)->GetFooterFmt() )
        rHeadFootFlags |= nFlag;
}

void WW8_WrPlcSepx::OutHeader( SwWW8Writer& rWrt, const SwFmt& rFmt,
                                ULONG& rCpPos, BYTE nHFFlags, BYTE nFlag )
{
    if( nFlag & nHFFlags )
    {
        const SwFmtHeader& rHd = rFmt.GetHeader();
        ASSERT( rHd.GetHeaderFmt(), "KopfText nicht richtig da" );
        const SwFmtCntnt& rCntnt = rHd.GetHeaderFmt()->GetCntnt();
        pTxtPos->Append( rCpPos );
        rWrt.WriteKFTxt1( rCntnt );
        rWrt.WriteStringAsPara( aEmptyStr ); // CR ans Ende ( sonst mault WW )
        rCpPos = rWrt.Fc2Cp( rWrt.Strm().Tell() );
    }
    else if( rWrt.bWrtWW8 )
        pTxtPos->Append( rCpPos );
}
void WW8_WrPlcSepx::OutFooter( SwWW8Writer& rWrt, const SwFmt& rFmt,
                                ULONG& rCpPos, BYTE nHFFlags, BYTE nFlag )
{
    if( nFlag & nHFFlags )
    {
        const SwFmtFooter& rFt = rFmt.GetFooter();
        ASSERT( rFt.GetFooterFmt(), "KopfText nicht richtig da" );
        const SwFmtCntnt& rCntnt = rFt.GetFooterFmt()->GetCntnt();
        pTxtPos->Append( rCpPos );
        rWrt.WriteKFTxt1( rCntnt );
        rWrt.WriteStringAsPara( aEmptyStr ); // CR ans Ende ( sonst mault WW )
        rCpPos = rWrt.Fc2Cp( rWrt.Strm().Tell() );
    }
    else if( rWrt.bWrtWW8 )
        pTxtPos->Append( rCpPos );
}

void WW8_WrPlcSepx::CheckForFacinPg( SwWW8Writer& rWrt ) const
{
    // 2 Werte werden gesetzt
    //      Dop.fFacingPages            == Kopf-/Fusszeilen unterschiedlich
    //      Dop.fSwapBordersFacingPgs   == gespiegelte Raender
    for( USHORT i = 0, nEnde = 0; i < aSects.Count(); ++i )
    {
        WW8_SepInfo& rSepInfo = aSects[i];
        if( !rSepInfo.pSectionFmt )
        {
            const SwPageDesc* pPd = rSepInfo.pPageDesc;
            if( pPd->GetFollow() && pPd != pPd->GetFollow() &&
                pPd->GetFollow()->GetFollow() == pPd->GetFollow() &&
                rSepInfo.pPDNd &&
                pPd->IsFollowNextPageOfNode( *rSepInfo.pPDNd ) )
                // das ist also 1.Seite und nachfolgende, also nur den
                // follow beachten
                pPd = pPd->GetFollow();

            // left-/right chain of pagedescs ?
            else if( !( 1 & nEnde ) &&
                pPd->GetFollow() && pPd != pPd->GetFollow() &&
                pPd->GetFollow()->GetFollow() == pPd &&
                (( PD_LEFT == ( PD_ALL & pPd->ReadUseOn() ) &&
                   PD_RIGHT == ( PD_ALL & pPd->GetFollow()->ReadUseOn() )) ||
                 ( PD_RIGHT == ( PD_ALL & pPd->ReadUseOn() ) &&
                   PD_LEFT == ( PD_ALL & pPd->GetFollow()->ReadUseOn() )) ))
            {
                rWrt.pDop->fFacingPages = rWrt.pDop->fMirrorMargins = TRUE;
                nEnde |= 1;
            }

            if( !( 1 & nEnde ) &&
                ( !pPd->IsHeaderShared() || !pPd->IsFooterShared() ))
            {
                rWrt.pDop->fFacingPages = TRUE;
                nEnde |= 1;
            }
            if( !( 2 & nEnde ) &&
                PD_MIRROR == ( PD_MIRROR & pPd->ReadUseOn() ))
            {
                rWrt.pDop->fSwapBordersFacingPgs = rWrt.pDop->fMirrorMargins = TRUE;
                nEnde |= 2;
            }

            if( 3 == nEnde )
                break;      // weiter brauchen wird nicht
        }
    }
}


int WW8_WrPlcSepx::HasBorderItem( const SwFmt& rFmt )
{
    const SfxPoolItem* pItem;
    return SFX_ITEM_SET == rFmt.GetItemState( RES_BOX,TRUE, &pItem ) &&
            (   ((SvxBoxItem*)pItem)->GetTop() ||
                ((SvxBoxItem*)pItem)->GetBottom()  ||
                ((SvxBoxItem*)pItem)->GetLeft()  ||
                ((SvxBoxItem*)pItem)->GetRight() );
}

void WW8_WrPlcSepx::WriteKFTxt( SwWW8Writer& rWrt )
{
    pAttrs = new WW8_PdAttrDesc[ aSects.Count() ];
    WW8Bytes* pO = rWrt.pO;
    ULONG nCpStart = rWrt.Fc2Cp( rWrt.Strm().Tell() );
    BOOL bOldPg = rWrt.bOutPageDescs;
    rWrt.bOutPageDescs = TRUE;

    ASSERT( !pTxtPos, "wer hat den Pointer gesetzt?" );
    pTxtPos = new WW8_WrPlc0( nCpStart );

    WriteFtnEndTxt( rWrt, nCpStart );
    CheckForFacinPg( rWrt );

    WW8Bytes aLineNum;
    const SwLineNumberInfo& rLnNumInfo = rWrt.pDoc->GetLineNumberInfo();
    if( rLnNumInfo.IsPaintLineNumbers() )
    {
        // sprmSNLnnMod - activate Line Numbering and define Modulo
        if( rWrt.bWrtWW8 )
            SwWW8Writer::InsUInt16( aLineNum, 0x5015 );
        else
            aLineNum.Insert( 154, aLineNum.Count() );
        SwWW8Writer::InsUInt16( aLineNum, (UINT16)rLnNumInfo.GetCountBy() );

        // sprmSDxaLnn - xPosition of Line Number
        if( rWrt.bWrtWW8 )
            SwWW8Writer::InsUInt16( aLineNum, 0x9016 );
        else
            aLineNum.Insert( 155, aLineNum.Count() );
        SwWW8Writer::InsUInt16( aLineNum, (UINT16)rLnNumInfo.GetPosFromLeft() );

        //
    }

    USHORT i;
    for( i = 0; i < aSects.Count(); ++i )
    {
        WW8_PdAttrDesc* pA = pAttrs + i;
        WW8_SepInfo& rSepInfo = aSects[i];
        const SwPageDesc* pPd = rSepInfo.pPageDesc;

        if( rSepInfo.pSectionFmt && !pPd )
            pPd = &rWrt.pDoc->GetPageDesc(0);

        rWrt.pAktPageDesc = pPd;
        pA->nSepxFcPos = 0xffffffff;                // Default: none

        if( !pPd )
        {
            pA->pData = 0;
            pA->nLen  = 0;
            continue;
        }

        pO->Remove( 0, pO->Count() );       // leeren
        rWrt.bOutPageDescs = TRUE;


        if( aLineNum.Count() )
        {
            pO->Insert( &aLineNum, pO->Count() );

            // sprmSLnc - restart number: 0 per page, 1 per section, 2 never restart
            if( rSepInfo.nLnNumRestartNo || !rLnNumInfo.IsRestartEachPage() )
            {
                if( rWrt.bWrtWW8 )
                    SwWW8Writer::InsUInt16( *pO, 0x3013 );
                else
                    pO->Insert( 152, pO->Count() );
                pO->Insert( rSepInfo.nLnNumRestartNo ? 1 : 2, pO->Count() );
            }

            // sprmSLnnMin - Restart the Line Number with given value
            if( rSepInfo.nLnNumRestartNo )
            {
                if( rWrt.bWrtWW8 )
                    SwWW8Writer::InsUInt16( *pO, 0x501B );
                else
                    pO->Insert( 160, pO->Count() );
                SwWW8Writer::InsUInt16( *pO, (UINT16)rSepInfo.nLnNumRestartNo - 1 );
            }
        }


        /*  sprmSBkc, break code:   0 No break, 1 New column
                                    2 New page, 3 Even page, 4 Odd page
        */
        BYTE nBreakCode = 2;            // default neue Seite beginnen
        BOOL bOutPgDscSet = TRUE, bLeftRightPgChain = FALSE;
        const SwFrmFmt* pPdFmt = &pPd->GetMaster();
        const SwFrmFmt* pPdFirstPgFmt = pPdFmt;
        if( rSepInfo.pSectionFmt )
        {
            // ist pSectionFmt gesetzt, dann gab es einen SectionNode
            //  gueltiger Pointer -> Section beginnt,
            //  0xfff -> Section wird beendet
            nBreakCode = 0;         // fortlaufender Abschnitt
            if( (SwSectionFmt*)0xFFFFFFFF != rSepInfo.pSectionFmt )
            {
                // Itemset erzeugen, das das PgDesk-AttrSet beerbt:
                // als Nachkomme wird bei 'deep'-Out_SfxItemSet
                // auch der Vorfahr abgeklappert
                const SfxItemSet* pPdSet = &pPdFmt->GetAttrSet();
                SfxItemSet aSet( *pPdSet->GetPool(), pPdSet->GetRanges() );
                aSet.SetParent( pPdSet );

                // am Nachkommen NUR  die Spaltigkeit gemaess Sect-Attr. umsetzen
                aSet.Put( rSepInfo.pSectionFmt->GetAttr( RES_COL ) );

                // und raus damit ins WW-File
                const SfxItemSet* pOldI = rWrt.pISet;
                rWrt.pISet = &aSet;
                Out_SfxItemSet( aWW8AttrFnTab, rWrt, aSet, TRUE );
                rWrt.pISet = pOldI;

                bOutPgDscSet = FALSE;
            }
        }

        if( bOutPgDscSet )
        {
            // es ist ein Follow gesetzt und dieser zeigt nicht auf sich
            // selbst, so liegt eine Seitenverkettung vor.
            // Falls damit eine "Erste Seite" simuliert werden soll, so
            // koennen wir das auch als solches schreiben.
            // Anders sieht es mit Links/Rechts wechseln aus. Dafuer muss
            // erkannt werden, wo der Seitenwechsel statt findet. Hier ist
            // es aber dafuer zuspaet!
            if( pPd->GetFollow() && pPd != pPd->GetFollow() &&
                pPd->GetFollow()->GetFollow() == pPd->GetFollow() &&
                rSepInfo.pPDNd &&
                pPd->IsFollowNextPageOfNode( *rSepInfo.pPDNd ) )
            {
                if( rSepInfo.pPDNd )
                    pPdFirstPgFmt = pPd->GetPageFmtOfNode( *rSepInfo.pPDNd );
                else
                    pPdFirstPgFmt = &pPd->GetMaster();

                rWrt.pAktPageDesc = pPd = pPd->GetFollow();
                pPdFmt = &pPd->GetMaster();

                // sprmSFTitlePage
                if( rWrt.bWrtWW8 )
                    SwWW8Writer::InsUInt16( *pO, 0x300A );
                else
                    pO->Insert( 143, pO->Count() );
                pO->Insert( 1, pO->Count() );
            }

            const SfxItemSet* pOldI = rWrt.pISet;

            if( rWrt.bWrtWW8 )              // Seitenumrandung schreiben
            {
                USHORT nPgBorder = HasBorderItem( *pPdFmt ) ? 0 : USHRT_MAX;
                if( pPdFmt != pPdFirstPgFmt )
                {
                    if( HasBorderItem( *pPdFirstPgFmt ))
                    {
                        if( USHRT_MAX == nPgBorder )
                        {
                            nPgBorder = 1;
                            // nur die 1. Seite umrandet -> BoxItem aus dem
                            // richtigen Format besorgen
                            rWrt.pISet = &pPdFirstPgFmt->GetAttrSet();
                            Out( aWW8AttrFnTab, pPdFirstPgFmt->GetAttr(
                                        RES_BOX ), rWrt );
                        }
                    }
                    else if( !nPgBorder )
                        nPgBorder = 2;
                }

                if( USHRT_MAX != nPgBorder )
                {
                    // Flag und das Border Attribut schreiben
                    SwWW8Writer::InsUInt16( *pO, 0x522F );
                    SwWW8Writer::InsUInt16( *pO, nPgBorder );
                }
            }

            const SfxPoolItem* pItem;
            if( pPdFmt != pPdFirstPgFmt && SFX_ITEM_SET ==
                pPdFirstPgFmt->GetItemState( RES_PAPER_BIN, TRUE, &pItem ))
            {
                rWrt.pISet = &pPdFirstPgFmt->GetAttrSet();
                rWrt.bOutFirstPage = TRUE;
                Out( aWW8AttrFnTab, *pItem, rWrt );
                rWrt.bOutFirstPage = FALSE;
            }


            // left-/right chain of pagedescs ?
            if( pPd->GetFollow() && pPd != pPd->GetFollow() &&
                pPd->GetFollow()->GetFollow() == pPd &&
                (( PD_LEFT == ( PD_ALL & pPd->ReadUseOn() ) &&
                   PD_RIGHT == ( PD_ALL & pPd->GetFollow()->ReadUseOn() )) ||
                 ( PD_RIGHT == ( PD_ALL & pPd->ReadUseOn() ) &&
                   PD_LEFT == ( PD_ALL & pPd->GetFollow()->ReadUseOn() )) ))
            {
                bLeftRightPgChain = TRUE;

                // welches ist der Bezugspunkt ????? (links oder rechts?)
                // annahme die rechte Seite!
                if( PD_LEFT == ( PD_ALL & pPd->ReadUseOn() ))
                {
                    nBreakCode = 3;
                    pPd = pPd->GetFollow();
                    pPdFmt = &pPd->GetMaster();
                }
                else
                    nBreakCode = 4;
            }

            rWrt.pISet = &pPdFmt->GetAttrSet();
            Out_SfxItemSet( aWW8AttrFnTab, rWrt, pPdFmt->GetAttrSet(),
                                TRUE, FALSE );
            rWrt.pISet = pOldI;

            // dann noch die restlichen Einstellungen aus dem PageDesc

            // sprmSNfcPgn
            BYTE nb = SwWW8Writer::GetNumId( pPd->GetNumType().eType );
            if( rWrt.bWrtWW8 )
                SwWW8Writer::InsUInt16( *pO, 0x300E );
            else
                pO->Insert( 147, pO->Count() );
            pO->Insert( nb, pO->Count() );

//???           const SwPageFtnInfo& rFtnInfo = pPd->GetFtnInfo();

            if( rSepInfo.nPgRestartNo )
            {
                // sprmSFPgnRestart
                if( rWrt.bWrtWW8 )
                    SwWW8Writer::InsUInt16( *pO, 0x3011 );
                else
                    pO->Insert( 150, pO->Count() );
                pO->Insert( 1, pO->Count() );
                // sprmSPgnStart
                if( rWrt.bWrtWW8 )
                    SwWW8Writer::InsUInt16( *pO, 0x501C );
                else
                    pO->Insert( 161, pO->Count() );
                SwWW8Writer::InsUInt16( *pO, rSepInfo.nPgRestartNo );
            }

            // werden es nur linke oder nur rechte Seiten?
            if( 2 == nBreakCode )
            {
                if( PD_LEFT == ( PD_ALL & pPd->ReadUseOn() ))
                    nBreakCode = 3;
                else if( PD_RIGHT == ( PD_ALL & pPd->ReadUseOn() ))
                    nBreakCode = 4;
            }
        }

        if( 2 != nBreakCode )           // neue Seite ist default
        {
            if( rWrt.bWrtWW8 )
                SwWW8Writer::InsUInt16( *pO, 0x3009 );
            else
                pO->Insert( 142, pO->Count() );
            pO->Insert( nBreakCode, pO->Count() );
        }

        WriteOlst( rWrt, i );


        // Header oder Footer
        BYTE nHeadFootFlags = 0;

        const SwFrmFmt* pPdLeftFmt = bLeftRightPgChain
                                        ? &pPd->GetFollow()->GetMaster()
                                        : &pPd->GetLeft();

        SetHeaderFlag( nHeadFootFlags, *pPdFmt, WW8_HEADER_ODD );
        SetFooterFlag( nHeadFootFlags, *pPdFmt, WW8_FOOTER_ODD );
        if( !pPd->IsHeaderShared() || bLeftRightPgChain )
            SetHeaderFlag( nHeadFootFlags, *pPdLeftFmt, WW8_HEADER_EVEN );
        if( !pPd->IsFooterShared() || bLeftRightPgChain )
            SetFooterFlag( nHeadFootFlags, *pPdLeftFmt, WW8_FOOTER_EVEN );
        if( pPdFmt != pPdFirstPgFmt )
        {
            // es gibt eine ErsteSeite:
            SetHeaderFlag( nHeadFootFlags, *pPdFirstPgFmt, WW8_HEADER_FIRST );
            SetFooterFlag( nHeadFootFlags, *pPdFirstPgFmt, WW8_FOOTER_FIRST );
        }

        if( nHeadFootFlags && !rWrt.bWrtWW8 )
        {
            BYTE nTmpFlags = nHeadFootFlags;
            if( rWrt.pDop->fFacingPages )
            {
                if( !(nTmpFlags & WW8_FOOTER_EVEN) &&
                    (nTmpFlags & WW8_FOOTER_ODD ) )
                    nTmpFlags |= WW8_FOOTER_EVEN;

                if( !(nTmpFlags & WW8_HEADER_EVEN) &&
                    (nTmpFlags & WW8_HEADER_ODD ) )
                    nTmpFlags |= WW8_HEADER_EVEN;
            }

            // sprmSGprfIhdt, wird nur noch im WW95 benoetigt
            pO->Insert( 153, pO->Count() );
            pO->Insert( nTmpFlags, pO->Count() );
        }

        if( pO->Count() )
        {                   // waren Attrs vorhanden ?
            pA->nLen = pO->Count();
            pA->pData = new BYTE [pO->Count()];
            memcpy( pA->pData, pO->GetData(), pO->Count() );    // -> merken
            pO->Remove( 0, pO->Count() );       // leeren fuer HdFt-Text
        }
        else
        {                               // keine Attrs da
            pA->pData = 0;
            pA->nLen = 0;
        }

/*
!!!!!!!!!!!
            // Umrandungen an Kopf- und Fusstexten muessten etwa so gehen:
            // Dabei muss etwas wie pOut eingebaut werden,
            // das bei jeder Spezialtext-Zeile wiederholt wird.
            const SwFrmFmt* pFFmt = rFt.GetFooterFmt();
            const SvxBoxItem& rBox = pFFmt->GetBox(FALSE);
            OutWW8_SwFmtBox1( rWrt.pOut, rBox, FALSE );
!!!!!!!!!!!
        Man koennt daraus Absatzattribute machen, die dann bei jedem Absatz
        beachtet werden. Gilt fuer Hintergrund/Umrandung
!!!!!!!!!!!
*/

        ULONG nCpPos = rWrt.Fc2Cp( rWrt.Strm().Tell() );
        if( !(nHeadFootFlags & WW8_HEADER_EVEN) && rWrt.pDop->fFacingPages )
            OutHeader( rWrt, *pPdFmt, nCpPos, nHeadFootFlags, WW8_HEADER_ODD );
        else
            OutHeader( rWrt, *pPdLeftFmt, nCpPos, nHeadFootFlags, WW8_HEADER_EVEN );
        OutHeader( rWrt, *pPdFmt, nCpPos, nHeadFootFlags, WW8_HEADER_ODD );

        if( !(nHeadFootFlags & WW8_FOOTER_EVEN) && rWrt.pDop->fFacingPages )
            OutFooter( rWrt, *pPdFmt, nCpPos, nHeadFootFlags, WW8_FOOTER_ODD );
        else
            OutFooter( rWrt, *pPdLeftFmt, nCpPos, nHeadFootFlags, WW8_FOOTER_EVEN );
        OutFooter( rWrt, *pPdFmt, nCpPos, nHeadFootFlags, WW8_FOOTER_ODD );

        OutHeader( rWrt, *pPdFirstPgFmt, nCpPos, nHeadFootFlags, WW8_HEADER_FIRST );
        OutFooter( rWrt, *pPdFirstPgFmt, nCpPos, nHeadFootFlags, WW8_FOOTER_FIRST );
    }

    if( pTxtPos->Count() )
    {
        // HdFt vorhanden ?
        ULONG nCpEnd = rWrt.Fc2Cp( rWrt.Strm().Tell() );
        pTxtPos->Append( nCpEnd );  // Ende letzter Hd/Ft fuer PlcfHdd

        if( nCpEnd > nCpStart )
        {
            ++nCpEnd;
            pTxtPos->Append( nCpEnd + 1 );  // Ende letzter Hd/Ft fuer PlcfHdd

            rWrt.WriteStringAsPara( aEmptyStr ); // CR ans Ende ( sonst mault WW )
        }
        rWrt.pFldHdFt->Finish( nCpEnd, rWrt.pFib->ccpText + rWrt.pFib->ccpFtn );
        rWrt.pFib->ccpHdr = nCpEnd - nCpStart;
    }
    else
        delete pTxtPos, pTxtPos = 0;

    rWrt.bOutPageDescs = bOldPg;
}


void WW8_WrPlcSepx::WriteSepx( SvStream& rStrm ) const
{
    USHORT i;
    for( i = 0; i < aSects.Count(); i++ ) // ueber alle Sections
    {
        WW8_PdAttrDesc* pA = pAttrs + i;
        if( pA->nLen )
        {
            SVBT16 nL;
            pA->nSepxFcPos = rStrm.Tell();
            ShortToSVBT16( pA->nLen, nL );
            rStrm.Write( nL, 2 );
            rStrm.Write( pA->pData, pA->nLen );
        }
    }
}


void WW8_WrPlcSepx::WritePlcSed( SwWW8Writer& rWrt ) const
{
    ASSERT( aCps.Count() == aSects.Count() + 1, "WrPlcSepx: DeSync" );
    ULONG nFcStart = rWrt.pTableStrm->Tell();

    USHORT i;
    // ( ueber alle Sections )
    for( i = 0; i <= aSects.Count(); i++ )
    {
        UINT32 nP = aCps[i];
        SVBT32 nPos;
        LongToSVBT32( nP, nPos );
        rWrt.pTableStrm->Write( nPos, 4 );
    }

    static WW8_SED aSed = { 4, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0xff, 0xff };

    // ( ueber alle Sections )
    for( i = 0; i < aSects.Count(); i++ )
    {
        WW8_PdAttrDesc* pA = pAttrs + i;
        LongToSVBT32( pA->nSepxFcPos, aSed.fcSepx );    // Sepx-Pos
        rWrt.pTableStrm->Write( &aSed, sizeof( aSed ) );
    }
    rWrt.pFib->fcPlcfsed = nFcStart;
    rWrt.pFib->lcbPlcfsed = rWrt.pTableStrm->Tell() - nFcStart;
}


void WW8_WrPlcSepx::WritePlcHdd( SwWW8Writer& rWrt ) const
{
    if( pTxtPos && pTxtPos->Count() )
    {
        rWrt.pFib->fcPlcfhdd = rWrt.pTableStrm->Tell();
        pTxtPos->Write( *rWrt.pTableStrm );             // Plc0
        rWrt.pFib->lcbPlcfhdd = rWrt.pTableStrm->Tell() -
                                rWrt.pFib->fcPlcfhdd;
    }
}

void SwWW8Writer::WriteKFTxt1( const SwFmtCntnt& rCntnt )
{
    SwNodePtr pNode;

    if( rCntnt.GetCntntIdx() )
        pNode = pDoc->GetNodes()[ rCntnt.GetCntntIdx()->GetIndex()+1 ];
    else
        pNode = 0;

    if( pNode )
    {
        // Hole vom Node und vom letzten Node die Position in der Section
        ULONG nStart = pNode->StartOfSectionIndex()+1;
        ULONG nEnd = pNode->EndOfSectionIndex();

        // Bereich also gueltiger Node
        if( nStart < nEnd )
        {
            BOOL bOldKF = bOutKF;
            bOutKF = TRUE;
            WriteSpecialText( nStart, nEnd, TXT_HDFT );
            bOutKF = bOldKF;
        }
        else
            pNode = 0;
    }

    if( !pNode )
    {
        // es gibt keine Kopf-/Fusszeile, aber ein CR ist immer noch noetig
        ASSERT( pNode, "K/F-Text nicht richtig da" );
        WriteStringAsPara( aEmptyStr ); // CR ans Ende ( sonst mault WW )
    }
}

/*  */
//------------------------------------------------------------------------------
// class WW8_WrPlcFtnEdn : Sammeln der Fuss/Endnoten und Ausgeben der Texte
// und Plcs am Ende des Docs.
// WW8_WrPlcFtnEdn ist die Klasse fuer Fuss- und Endnoten
//------------------------------------------------------------------------------
WW8_WrPlcSubDoc::WW8_WrPlcSubDoc()
    : aCps( 0, 16 ), aCntnt( 0, 16 ), pTxtPos( 0 )
{
}

WW8_WrPlcSubDoc::~WW8_WrPlcSubDoc()
{
    delete pTxtPos;
}

void WW8_WrPlcFtnEdn::Append( WW8_CP nCp, const SwFmtFtn& rFtn )
{
    aCps.Insert( nCp, aCps.Count() );
    void* p = (void*)&rFtn;
    aCntnt.Insert( p, aCntnt.Count() );
}

void WW8_WrPlcPostIt::Append( WW8_CP nCp, const SwPostItField& rPostIt )
{
    aCps.Insert( nCp, aCps.Count() );
    void* p = (void*)&rPostIt;
    aCntnt.Insert( p, aCntnt.Count() );
}

void WW8_WrPlcSubDoc::WriteTxt( SwWW8Writer& rWrt, BYTE nTTyp,
                                long& rCount )
{
    USHORT nLen = aCntnt.Count();
    if( nLen )
    {
        ULONG nCpStart = rWrt.Fc2Cp( rWrt.Strm().Tell() );
        pTxtPos = new WW8_WrPlc0( nCpStart );
        USHORT i;

        switch( nTTyp )
        {
        case TXT_ATN:
            for( i = 0; i < nLen; i++ )
            {
                // Anfaenge fuer PlcfAtnTxt
                pTxtPos->Append( rWrt.Fc2Cp( rWrt.Strm().Tell() ));

                const SwPostItField& rPFld = *(SwPostItField*)aCntnt[ i ];
                rWrt.WritePostItBegin( rPFld );
                rWrt.WriteStringAsPara( rPFld.GetTxt() );
            }
            break;

        case TXT_TXTBOX:
        case TXT_HFTXTBOX:
            for( i = 0; i < nLen; i++ )
            {
                // textbox - content
                WW8_CP nCP = rWrt.Fc2Cp( rWrt.Strm().Tell() );
                aCps.Insert( nCP, i );
                pTxtPos->Append( nCP );

                // is it an writer or sdr - textbox?
                const SdrObject& rObj = *(SdrObject*)aCntnt[ i ];
                if (rObj.GetObjInventor() == FmFormInventor)
                {
                    BYTE nOldTyp = rWrt.nTxtTyp;
                    rWrt.nTxtTyp = nTTyp;
                    rWrt.GetOCXExp().ExportControl(rWrt,&rObj);
                    rWrt.nTxtTyp = nOldTyp;
                }
                else if( rObj.ISA( SdrTextObj ) )
                    rWrt.WriteSdrTextObj( rObj );
                else
                {
                    const SwFrmFmt* pFmt = ::FindFrmFmt( &rObj );
                    ASSERT( pFmt, "wo ist das Format geblieben?" );

                    const SwNodeIndex* pNdIdx = pFmt->GetCntnt().GetCntntIdx();
                    ASSERT( pNdIdx, "wo ist der StartNode der Textbox?" );
                    rWrt.WriteSpecialText( pNdIdx->GetIndex() + 1,
                                       pNdIdx->GetNode().EndOfSectionIndex(),
                                       nTTyp );
                }

                // CR at end of one textbox text ( otherwise WW gpft :-( )
                rWrt.WriteStringAsPara( aEmptyStr );
            }
            break;

        case TXT_EDN:
        case TXT_FTN:
            for( i = 0; i < nLen; i++ )
            {
                // Anfaenge fuer PlcfFtnTxt/PlcfEdnTxt
                pTxtPos->Append( rWrt.Fc2Cp( rWrt.Strm().Tell() ));

                // Noten-Inhalt
                const SwFmtFtn* pFtn = (SwFmtFtn*)aCntnt[ i ];
                rWrt.WriteFtnBegin( *pFtn );
                const SwNodeIndex* pIdx = pFtn->GetTxtFtn()->GetStartNode();
                ASSERT( pIdx, "wo ist der StartNode der Fuss-/EndNote?" );
                rWrt.WriteSpecialText( pIdx->GetIndex() + 1,
                                       pIdx->GetNode().EndOfSectionIndex(),
                                       nTTyp );
            }
            break;

        default:
            ASSERT( !this, "was ist das fuer ein SubDocType?" );
        }

        pTxtPos->Append( rWrt.Fc2Cp( rWrt.Strm().Tell() ));
        // CR ans Ende ( sonst mault WW )
        rWrt.WriteStringAsPara( aEmptyStr );

        ULONG nCpEnd = rWrt.Fc2Cp( rWrt.Strm().Tell() );
        pTxtPos->Append( nCpEnd );
        rCount = nCpEnd - nCpStart;
    }
}

void WW8_WrPlcSubDoc::WritePlc( SwWW8Writer& rWrt, BYTE nTTyp,
                                long& rTxtStart, long& rTxtCount,
                                long& rRefStart, long& rRefCount ) const
{
    ULONG nFcStart = rWrt.pTableStrm->Tell();
    USHORT nLen = aCps.Count();
    if( nLen )
    {
        ASSERT( aCps.Count() + 2 == pTxtPos->Count(), "WritePlc: DeSync" );

        SvStringsSortDtor aStrArr( 0, 4 );
        WW8Fib& rFib = *rWrt.pFib;              // n+1-te CP-Pos nach Handbuch
        USHORT i;
        BOOL bWriteCP = TRUE;

        switch( nTTyp )
        {
        case TXT_ATN:
            // then write first the GrpXstAtnOwners
            for( i = 0; i < nLen; ++i )
            {
                const SwPostItField& rPFld = *(SwPostItField*)aCntnt[ i ];
                String* p = new String( rPFld.GetPar1() );
                if( !aStrArr.Insert( p ))
                    delete p;
            }


            if( rWrt.bWrtWW8 )
                for( i = 0; i < aStrArr.Count(); ++i )
                {
                    const String& rStr = *aStrArr[ i ];
                    SwWW8Writer::WriteShort( *rWrt.pTableStrm, rStr.Len() );
                    SwWW8Writer::WriteString16( *rWrt.pTableStrm, rStr, FALSE );
                }
            else
                for( i = 0; i < aStrArr.Count(); ++i )
                {
                    const String& rStr = *aStrArr[ i ];
                    *rWrt.pTableStrm << (BYTE)rStr.Len();
                    SwWW8Writer::WriteString8( *rWrt.pTableStrm, rStr, FALSE,
                                                RTL_TEXTENCODING_MS_1252 );
                }

            rFib.fcGrpStAtnOwners = nFcStart;
            nFcStart = rWrt.pTableStrm->Tell();
            rFib.lcbGrpStAtnOwners = nFcStart - rFib.fcGrpStAtnOwners;
            break;

        case TXT_TXTBOX:
        case TXT_HFTXTBOX:
            {
                pTxtPos->Write( *rWrt.pTableStrm );
                const SvULongs* pShapeIds = GetShapeIdArr();
                ASSERT( pShapeIds, "wo sind die ShapeIds?" );

                // nLen = pTxtPos->Count();
                for( i = 0; i < nLen; ++i )
                {
                    // write textbox story - FTXBXS
                    // is it an writer or sdr - textbox?
                    const SdrObject* pObj = (SdrObject*)aCntnt[ i ];
                    INT32 nCnt = 1;
                    if( !pObj->ISA( SdrTextObj ) )
                    {
                        // find the "highest" SdrObject of this
                        const SwFrmFmt& rFmt = *::FindFrmFmt( pObj );

                        const SwFmtChain* pChn = &rFmt.GetChain();
                        while( pChn->GetNext() )
                        {
                            // has a chain?
                            // then calc the cur pos in the chain
                            ++nCnt;
                            pChn = &pChn->GetNext()->GetChain();
                        }
                    }
                    // long cTxbx / iNextReuse
                    SwWW8Writer::WriteLong( *rWrt.pTableStrm, nCnt );
                    // long cReusable
                    SwWW8Writer::WriteLong( *rWrt.pTableStrm, 0 );
                    // short fReusable
                    SwWW8Writer::WriteShort( *rWrt.pTableStrm, 0 );
                    // long reserved
                    SwWW8Writer::WriteLong( *rWrt.pTableStrm, -1 );
                    // long lid
                    SwWW8Writer::WriteLong( *rWrt.pTableStrm,
                                            (*pShapeIds)[i]);
                    // long txidUndo
                    SwWW8Writer::WriteLong( *rWrt.pTableStrm, 0 );
                }
                SwWW8Writer::FillCount( *rWrt.pTableStrm, 22 );
                bWriteCP = FALSE;
            }
            break;
        }

        if( bWriteCP )
        {
            // Schreibe CP-Positionen
            for( i = 0; i < nLen; i++ )
                SwWW8Writer::WriteLong( *rWrt.pTableStrm, aCps[ i ] );

            // n+1-te CP-Pos nach Handbuch
            SwWW8Writer::WriteLong( *rWrt.pTableStrm,
                                            rFib.ccpText + rFib.ccpFtn +
                                            rFib.ccpHdr + rFib.ccpEdn +
                                            rFib.ccpTxbx + rFib.ccpHdrTxbx + 1 );

            if( TXT_ATN == nTTyp )
            {
                USHORT nFndPos;
                for( i = 0; i < nLen; ++i )
                {
                    const SwPostItField& rPFld = *(SwPostItField*)aCntnt[ i ];
                    String sAuthor( rPFld.GetPar1() );
                    aStrArr.Seek_Entry( &sAuthor, &nFndPos );
                    BYTE nLen = (BYTE)sAuthor.Len();
                    if( nLen > 9 )
                    {
                        sAuthor.Erase( 9 );
                        nLen = 9;
                    }

                    // xstUsrInitl[ 10 ] pascal-style String holding initials
                    // of annotation author
                    if( rWrt.bWrtWW8 )
                    {
                        SwWW8Writer::WriteShort( *rWrt.pTableStrm, nLen );
                        SwWW8Writer::WriteString16( *rWrt.pTableStrm, sAuthor,
                                                    FALSE );
                        SwWW8Writer::FillCount( *rWrt.pTableStrm, (9 - nLen) * 2 );

                    }
                    else
                    {
                        *rWrt.pTableStrm << nLen;
                        SwWW8Writer::WriteString8( *rWrt.pTableStrm, sAuthor,
                                        FALSE, RTL_TEXTENCODING_MS_1252 );
                        SwWW8Writer::FillCount( *rWrt.pTableStrm, 9 - nLen );
                    }

                    //SVBT16 ibst;      // index into GrpXstAtnOwners
                    //SVBT16 ak;        // not used
                    //SVBT16 grfbmc;    // not used
                    //SVBT32 ITagBkmk;  // when not -1, this tag identifies the

                    SwWW8Writer::WriteShort( *rWrt.pTableStrm, nFndPos );
                    SwWW8Writer::WriteShort( *rWrt.pTableStrm, 0 );
                    SwWW8Writer::WriteShort( *rWrt.pTableStrm, 0 );
                    SwWW8Writer::WriteLong( *rWrt.pTableStrm, -1 );
                }
            }
            else
            {
                USHORT nNo = 0;
                for( i = 0; i < nLen; ++i )             // Schreibe Flags
                {
                    const SwFmtFtn* pFtn = (SwFmtFtn*)aCntnt[ i ];
                    SwWW8Writer::WriteShort( *rWrt.pTableStrm,
                                pFtn->GetNumStr().Len() ? 0 : ++nNo );
                }
            }
        }
        rRefStart = nFcStart;
        nFcStart = rWrt.pTableStrm->Tell();
        rRefCount = nFcStart - rRefStart;

        pTxtPos->Write( *rWrt.pTableStrm );

        switch( nTTyp )
        {
        case TXT_TXTBOX:
        case TXT_HFTXTBOX:
            for( i = 0; i < nLen; ++i )
            {
                // write break descriptor (BKD)
                // short itxbxs
                SwWW8Writer::WriteShort( *rWrt.pTableStrm, i );
                // short dcpDepend
                SwWW8Writer::WriteShort( *rWrt.pTableStrm, 0 );
                // short flags : icol/fTableBreak/fColumnBreak/fMarked/
                //               fUnk/fTextOverflow
                SwWW8Writer::WriteShort( *rWrt.pTableStrm, 0x800 );
            }
            SwWW8Writer::FillCount( *rWrt.pTableStrm, 6 );
            break;
        }

        rTxtStart = nFcStart;
        rTxtCount = rWrt.pTableStrm->Tell() - nFcStart;
    }
}

const SvULongs* WW8_WrPlcSubDoc::GetShapeIdArr() const
{
    return 0;
}

/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/ww8/wrtw8sty.cxx,v 1.3 2000-11-21 12:55:21 jp Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.2  2000/10/10 16:54:06  cmc
      MSOffice 97/2000 Controls {Im|Ex}port

      Revision 1.1.1.1  2000/09/18 17:14:58  hr
      initial import

      Revision 1.28  2000/09/18 16:04:58  willem.vandorp
      OpenOffice header added.

      Revision 1.27  2000/08/21 10:14:03  khz
      Export Line Numbering (restart on new section)

      Revision 1.26  2000/07/04 08:58:09  jp
      _OutFont: write zero terminated FontNames

      Revision 1.25  2000/05/12 16:14:02  jp
      Changes for Unicode

      Revision 1.24  2000/05/04 11:41:34  jp
      Bug #75550#: standard para.style must have a language sprm

      Revision 1.23  2000/01/25 20:09:30  jp
      Bug #72146#: convert stylenames only for W95 to ANSI

      Revision 1.22  1999/12/15 17:06:14  jp
      Bug #70913#: write Ftn-/EndNoteInfo - at end of section or page/doc

      Revision 1.21  1999/11/25 21:19:37  jp
      Bug #70134#: dont set def values by charstyles

      Revision 1.20  1999/11/22 18:26:11  jp
      export of TOX and TOX entries

      Revision 1.19  1999/11/12 16:07:16  jp
      write foot-/endnote attributes with correct charfmts

      Revision 1.18  1999/11/10 16:09:50  jp
      Bug #67922#: write header/footer for w95

      Revision 1.17  1999/10/22 14:11:01  jp
      Bug #69408#: if write SfxItemSet set the pointer at writerclass

      Revision 1.16  1999/10/14 21:06:05  jp
      Bug #68617#,#68705# and other little things

      Revision 1.15  1999/10/13 09:08:46  jp
      write character attributs of SdrObjects

      Revision 1.14  1999/10/11 10:58:59  jp
      Bug #69083#: set mirrormargin flag in dop

      Revision 1.13  1999/09/09 17:40:49  jp
      Bug #68628#: write startvalue of SwFtnInfo and SwEndNoteInfo

      Revision 1.12  1999/09/01 17:32:38  JP
      new: write the text of SdrTextObjects


      Rev 1.11   01 Sep 1999 19:32:38   JP
   new: write the text of SdrTextObjects

      Rev 1.10   30 Aug 1999 18:55:02   JP
   write cp's of foot-/endnotes

      Rev 1.9   24 Aug 1999 20:15:48   JP
   Escher Export

      Rev 1.8   16 Aug 1999 17:16:30   JP
   W97 Export: experimental escher export (1)

      Rev 1.7   21 Jul 1999 18:28:22   JP
   W97 Export: experimental escher export

      Rev 1.6   06 Jul 1999 14:54:04   JP
   W97-Export corrections

      Rev 1.5   30 Jun 1999 19:56:42   JP
   W97-Export: Fields/PostIts/References

      Rev 1.4   23 Jun 1999 12:46:16   JP
   Export of numbers und outlinenumbers

      Rev 1.3   16 Jun 1999 19:53:58   JP
   Change interface of base class Writer

      Rev 1.2   04 Jun 1999 17:47:56   JP
   WW97-Export: Code aufgeteilt

      Rev 1.1   21 May 1999 01:59:10   JP
   Export von WW97 Format: StyleSheets

      Rev 1.0   19 May 1999 19:35:16   JP
   WinWord97-ExportFilter

*************************************************************************/

