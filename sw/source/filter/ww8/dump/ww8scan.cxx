/*************************************************************************
 *
 *  $RCSfile: ww8scan.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2000-10-24 14:01:34 $
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




//#define KHZ_TEST0






#include <stdlib.h>

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#include <string.h>         // memset()

#define _SVSTDARR_SHORTS
#define _SVSTDARR_USHORTS
#define _SVSTDARR_USHORTSSORT
#define _SVSTDARR_SVSTRINGS
#include <svtools/svstdarr.hxx>

#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif

#ifdef DUMP

#define ERR_SWG_READ_ERROR 1234
#include <assert.h>
#define ASSERT( a, b )

#else                                   // dump

#ifndef _SWSWERROR_H
#include <swerror.h>        // ERR_WW6_...
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>           // ASSERT()
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>      // DELETEZ
#endif

#endif                                  // dump

#ifndef _DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif

#ifndef _WW8SCAN_HXX
#include <ww8scan.hxx>
#endif




#ifdef __WW8_NEEDS_PACK
#    pragma pack(2)
#endif
struct WW8_PCD1
{
    INT16 fNoParaLast;  // when 1, means that piece contains no end of paragraph marks.
    INT32 fc;           // file offset of beginning of piece. The size of the
    INT16 prm;          // PRM contains either a single sprm or else an index number
};
#ifdef __WW8_NEEDS_PACK
#    pragma pack()
#  endif




USHORT WW8GetSprmId( BYTE nVersion, BYTE* pSp, BYTE* pDelta );



WW8SprmIter::WW8SprmIter(BYTE* pSprms_,     short nLen_,       BYTE nVersion_ )
                    :  pSprms( pSprms_), nRemLen( nLen_), nVersion( nVersion_),
                       nDelta( ( 8 > nVersion ) ? 0 : 1 )
{
    UpdateMyMembers();
}

void WW8SprmIter::SetSprms(BYTE* pSprms_, short nLen_)
{
    pSprms  = pSprms_;
    nRemLen = nLen_;
    UpdateMyMembers();
}

BYTE* WW8SprmIter::operator ++( int )
{
    if( 0 < nRemLen )
    {
        pSprms  += nAktSizeBrutto;
        nRemLen -= nAktSizeBrutto;
        UpdateMyMembers();
    }
    return pSprms;
}

void WW8SprmIter::UpdateMyMembers()
{
    if( pSprms && (0 < nRemLen) )
    {
        nAktId         = WW8GetSprmId( nVersion, pSprms );
        pAktParams     = pSprms + 1 + nDelta + WW8SprmDataOfs( nAktId );
        nAktSizeBrutto = WW8GetSprmSizeBrutto( nVersion, pSprms, &nAktId );
    }
    else
    {
        nAktId         = 0;
        pAktParams     = 0;
        nAktSizeBrutto = 0;
        nRemLen        = 0;
    }
}



//-----------------------------------------
//      temporaerer Test
//-----------------------------------------
// WW8PLCFx_PCDAttrs halten sich an WW8PLCF_Pcd fest und besitzen deshalb keine
// eigenen Iteratoren. Alle sich auf Iteratoren beziehenden Methoden
// sind deshalb Dummies.

WW8PLCFx_PCDAttrs::WW8PLCFx_PCDAttrs( BYTE nVersion, WW8PLCFx_PCD* pPLCFx_PCD, WW8ScannerBase* pBase )
: WW8PLCFx( nVersion, TRUE ),
  pPcd( pPLCFx_PCD ),
  pGrpprls( pBase->pPieceGrpprls ),
  nGrpprls( pBase->nPieceGrpprls ),
  pPcdI( pPLCFx_PCD->GetPLCFIter() )
{
}

WW8PLCFx_PCDAttrs::~WW8PLCFx_PCDAttrs()
{
}

ULONG WW8PLCFx_PCDAttrs::GetIdx() const
{
    return 0;
}

void WW8PLCFx_PCDAttrs::SetIdx( ULONG )
{
}

BOOL WW8PLCFx_PCDAttrs::SeekPos( WW8_CP n )
{
    return TRUE;
}

WW8PLCFx& WW8PLCFx_PCDAttrs::operator ++( int )
{
    return *this;
}

WW8_CP WW8PLCFx_PCDAttrs::Where()
{
    return ( pPcd ) ? pPcd->Where() : LONG_MAX;
}

void WW8PLCFx_PCDAttrs::GetSprms( WW8PLCFxDesc* p )
{
    void* pData;

    p->bRealLineEnd = FALSE;
    if(     !pPcdI
         || !pPcdI->Get( p->nStartPos, p->nEndPos, pData ))
    {
        p->nStartPos = p->nEndPos = LONG_MAX;       // PLCF fertig abgearbeitet
        p->pMemPos = 0;
        return;
    }
    UINT16 nPrm = SVBT16ToShort( ( (WW8_PCD*)pData )->prm );

    if ( nPrm & 1 )
    {   // PRM Variante 2
        UINT16 nSprmIdx = nPrm >> 1;

        if( nSprmIdx >= nGrpprls ){
            p->nStartPos = p->nEndPos = LONG_MAX;       // Bloedsinniger Index
            p->pMemPos = 0;
            return;
        }
        BYTE* pSprms = pGrpprls[ nSprmIdx ];

        p->nSprmsLen = SVBT16ToShort( pSprms ); // Laenge
        pSprms += 2;
        p->pMemPos = pSprms;                    // Position
    }
    else
    {   // PRM Variante 1:  Sprm wird direkt in Member-Var abgelegt
        /*
            Dies sind die Attr, die in der Piece-Table stehen, statt im Text !
        */

        if(8 > GetVersion())
        {
            aShortSprm[0] = (BYTE)( ( nPrm & 0xfe) >> 1 );
            aShortSprm[1] = (BYTE)(   nPrm         >> 8 );
            p->nSprmsLen = ( nPrm ) ? 2 : 0;        // Laenge

            // store Postion of internal mini storage in Data Pointer
            p->pMemPos = aShortSprm;
        }
        else
        {
            p->pMemPos = 0;
            BYTE nSprmListIdx = (BYTE)((nPrm & 0xfe) >> 1);
            if( nSprmListIdx )
            {
                // process Sprm Id Matching as explained in MS Doku
                //
                // ''Property Modifier(variant 1) (PRM)''
                // see file: s62f39.htm
                //
                // Since isprm is 7 bits, rgsprmPrm can hold 0x80 entries.
                static USHORT __READONLY_DATA aSprmId[0x80] = {
                         0,     0,     0,     0,// sprmNoop, sprmNoop, sprmNoop, sprmNoop
                    0x2402,0x2403,0x2404,0x2405,// sprmPIncLvl, sprmPJc, sprmPFSideBySide, sprmPFKeep
                    0x2406,0x2407,0x2408,0x2409,// sprmPFKeepFollow, sprmPFPageBreakBefore, sprmPBrcl, sprmPBrcp
                    0x260A,     0,0x240C,     0,// sprmPIlvl, sprmNoop, sprmPFNoLineNumb, sprmNoop
                         0,     0,     0,     0,// sprmNoop, sprmNoop, sprmNoop, sprmNoop
                         0,     0,     0,     0,// sprmNoop, sprmNoop, sprmNoop, sprmNoop
                    0x2416,0x2417,     0,     0,// sprmPFInTable, sprmPFTtp, sprmNoop, sprmNoop
                         0,0x261B,     0,     0,// sprmNoop, sprmPPc,  sprmNoop, sprmNoop
                         0,     0,     0,     0,// sprmNoop, sprmNoop, sprmNoop, sprmNoop
                         0,0x2423,     0,     0,// sprmNoop, sprmPWr,  sprmNoop, sprmNoop
                         0,     0,     0,     0,// sprmNoop, sprmNoop, sprmNoop, sprmNoop
                    0x242A,     0,     0,     0,// sprmPFNoAutoHyph, sprmNoop, sprmNoop, sprmNoop
                         0,     0,0x2430,0x2431,// sprmNoop, sprmNoop, sprmPFLocked, sprmPFWidowControl
                         0,0x2433,0x2434,0x2435,// sprmNoop, sprmPFKinsoku, sprmPFWordWrap, sprmPFOverflowPunct
                    0x2436,0x2437,0x2438,     0,// sprmPFTopLinePunct, sprmPFAutoSpaceDE, sprmPFAutoSpaceDN, sprmNoop
                         0,0x243B,     0,     0,// sprmNoop, sprmPISnapBaseLine, sprmNoop, sprmNoop
                         0,0x0800,0x0801,0x0802,// sprmNoop, sprmCFStrikeRM, sprmCFRMark, sprmCFFldVanish
                         0,     0,     0,0x0806,// sprmNoop, sprmNoop, sprmNoop, sprmCFData
                         0,     0,     0,0x080A,// sprmNoop, sprmNoop, sprmNoop, sprmCFOle2
                         0,0x2A0C,0x0858,0x2859,// sprmNoop, sprmCHighlight, sprmCFEmboss, sprmCSfxText
                         0,     0,     0,0x2A33,// sprmNoop, sprmNoop, sprmNoop, sprmCPlain
                         0,0x0835,0x0836,0x0837,// sprmNoop, sprmCFBold, sprmCFItalic, sprmCFStrike
                    0x0838,0x0839,0x083a,0x083b,// sprmCFOutline, sprmCFShadow, sprmCFSmallCaps, sprmCFCaps,
                    0x083C,     0,0x2A3E,     0,// sprmCFVanish, sprmNoop, sprmCKul, sprmNoop,
                         0,     0,0x2A42,     0,// sprmNoop, sprmNoop, sprmCIco, sprmNoop,
                    0x2A44,     0,0x2A46,     0,// sprmCHpsInc, sprmNoop, sprmCHpsPosAdj, sprmNoop,
                    0x2A48,     0,     0,     0,// sprmCIss, sprmNoop, sprmNoop, sprmNoop,
                         0,     0,     0,     0,// sprmNoop, sprmNoop, sprmNoop, sprmNoop,
                         0,     0,     0,0x2A53,// sprmNoop, sprmNoop, sprmNoop, sprmCFDStrike,
                    0x0854,0x0855,0x0856,0x2E00,// sprmCFImprint, sprmCFSpec, sprmCFObj, sprmPicBrcl,
                    0x2640,     0,     0,     0,// sprmPOutLvl, sprmNoop, sprmNoop, sprmNoop,
                         0,     0,     0,     0 // sprmNoop, sprmNoop, sprmPPnbrRMarkNot
                };                              //                     ^^^^^^^^^^^^^^^^^ unknown name!

                // find real Sprm Id:
                USHORT nSprmId = aSprmId[ nSprmListIdx ];

                if( nSprmId )
                {
                    // move Sprm Id and Sprm Param to internal mini storage:
                    aShortSprm[0] = (BYTE)( ( nSprmId & 0x00ff)      );
                    aShortSprm[1] = (BYTE)( ( nSprmId & 0xff00) >> 8 );
                    aShortSprm[2] = (BYTE)( nPrm >> 8 );

                    // store Sprm Length in member:
                    p->nSprmsLen = ( nPrm ) ? 3 : 0;

                    // store Postion of internal mini storage in Data Pointer
                    p->pMemPos = aShortSprm;
                }
            }
        }
    }
}

//------------------------------------------------------------------------

WW8PLCFx_PCD::WW8PLCFx_PCD( BYTE nVersion, WW8PLCFpcd* pPLCFpcd,
                            WW8_CP nStartCp, BOOL bVer67P )
    : WW8PLCFx( nVersion, FALSE )
{
    pPcdI = new WW8PLCFpcd_Iter( *pPLCFpcd, nStartCp ); // eigenen Iterator konstruieren
    bVer67= bVer67P;
}

WW8PLCFx_PCD::~WW8PLCFx_PCD()
{                                   // pPcd-Dtor wird in WW8ScannerBase gerufen
    delete( pPcdI );
}

ULONG WW8PLCFx_PCD::GetIMax() const
{
    return ( pPcdI ) ? pPcdI->GetIMax() : 0;
}

ULONG WW8PLCFx_PCD::GetIdx() const
{
    return ( pPcdI ) ? pPcdI->GetIdx() : 0;
}

void WW8PLCFx_PCD::SetIdx( ULONG nIdx )
{
    if( pPcdI )
        pPcdI->SetIdx( nIdx );
}

BOOL WW8PLCFx_PCD::SeekPos( WW8_CP nCpPos )
{
    return ( pPcdI ) ? pPcdI->SeekPos( nCpPos ) : FALSE;
}

WW8_CP WW8PLCFx_PCD::Where()
{
    return ( pPcdI ) ? pPcdI->Where() : LONG_MAX;
}

long WW8PLCFx_PCD::GetNoSprms( long& rStart, long& rEnd, long& rLen )
{
    void* pData;
    rLen = 0;

    if(     !pPcdI
         || !pPcdI->Get( rStart, rEnd, pData ) )
    {
        rStart = rEnd = LONG_MAX;
        return -1;
    }
    return pPcdI->GetIdx();
}

WW8PLCFx& WW8PLCFx_PCD::operator ++( int )
{
    if( !pPcdI ){
        ASSERT( !this, "pPcdI fehlt");
    }else{
        (*pPcdI)++;
    }
    return *this;
}

WW8_FC WW8PLCFx_PCD::AktPieceStartCp2Fc( WW8_CP nCp )
{
    WW8_CP nCpStart, nCpEnd;
    void* pData;

    if ( !pPcdI->Get( nCpStart, nCpEnd, pData ) ){
        ASSERT( !this, "AktPieceStartCp2Fc() mit falschem Cp gerufen (1)" );
        return LONG_MAX;
    }

    ASSERT( nCp >= nCpStart && nCp < nCpEnd,
            "AktPieceCp2Fc() mit falschem Cp gerufen (2)" );

    if( nCp < nCpStart )
        nCp = nCpStart;
    if( nCp >= nCpEnd )
        nCp = nCpEnd - 1;

    BOOL bIsUnicode = FALSE;
    WW8_FC nFC = SVBT32ToLong( ((WW8_PCD*)pData)->fc );
    if( !bVer67 )
        nFC = WW8PLCFx_PCD::TransformPieceAddress( nFC, bIsUnicode );

    return nFC + (nCp - nCpStart) * (bIsUnicode ? 2 : 1);
}


eCutT WW8PLCFx_PCD::AktPieceFc2Cp( long& rStartPos, long& rEndPos )
{
    WW8_CP nCpStart, nCpEnd;
    void* pData;
    eCutT eRet = CUT_NONE;

    if ( !pPcdI->Get( nCpStart, nCpEnd, pData ) ){
        ASSERT( !this, "AktPieceFc2Cp() - Fehler" );
        rStartPos = rEndPos = LONG_MAX;
        return CUT_BOTH;
    }

    BOOL bIsUnicode = FALSE;
    INT32 nFcStart  = SVBT32ToLong( ((WW8_PCD*)pData)->fc );
    if( !bVer67 )
        nFcStart = WW8PLCFx_PCD::TransformPieceAddress( nFcStart, bIsUnicode );

    INT32 nUnicodeFactor = bIsUnicode ? 2 : 1;

    if( rStartPos < nFcStart )
    {
        rStartPos = nFcStart;
        eRet = (eCutT)( eRet | CUT_START );
    }
    if( rStartPos >= nFcStart + (nCpEnd - nCpStart)  * nUnicodeFactor )
    {
        rStartPos = nFcStart + (nCpEnd - nCpStart - 1) * nUnicodeFactor;
        eRet = (eCutT)( eRet | CUT_START );
    }
    // jetzt CP berechnen
    rStartPos = nCpStart + (rStartPos - nFcStart) / nUnicodeFactor;


    if( rEndPos < nFcStart )
    {
        rEndPos = nFcStart;
        eRet = (eCutT)( eRet | CUT_END );
    }
    if( rEndPos > nFcStart + (nCpEnd - nCpStart) * nUnicodeFactor )
    {
        rEndPos = nFcStart + (nCpEnd - nCpStart) * nUnicodeFactor;
        eRet = (eCutT)( eRet | CUT_END );
    }
    // jetzt CP berechnen
    rEndPos = nCpStart + (rEndPos - nFcStart) / nUnicodeFactor;
#ifdef DEBUG
    if (0 > rStartPos)
    {
        BOOL i = 5;
    }
    ASSERT( (0 <= rStartPos), "AktPieceFc2Cp() - rStartPos kleiner Null!" );
#endif
    return eRet;
}

WW8_CP WW8PLCFx_PCD::AktPieceStartFc2Cp( WW8_FC nStartPos )
{
    WW8_CP nCpStart, nCpEnd;
    void* pData;
/*
    if( nStartPos == LONG_MAX )
        return LONG_MAX;
*/
    if ( !pPcdI->Get( nCpStart, nCpEnd, pData ) )
    {
         ASSERT( !this, "AktPieceStartFc2Cp() - Fehler" );
        return LONG_MAX;
    }
    BOOL bIsUnicode = FALSE;
    INT32 nFcStart  = SVBT32ToLong( ((WW8_PCD*)pData)->fc );
    if( !bVer67 )
        nFcStart = WW8PLCFx_PCD::TransformPieceAddress( nFcStart, bIsUnicode );


    INT32 nUnicodeFactor = bIsUnicode ? 2 : 1;

    if( nStartPos < nFcStart )
        nStartPos = nFcStart;

    if( nStartPos >= nFcStart + (nCpEnd - nCpStart)     * nUnicodeFactor )
        nStartPos  = nFcStart + (nCpEnd - nCpStart - 1) * nUnicodeFactor;

    return nCpStart + (nStartPos - nFcStart) / nUnicodeFactor;
}



//-----------------------------------------
//      Hilfsroutinen fuer alle
//-----------------------------------------

DateTime WW8ScannerBase::WW8DTTM2DateTime(long lDTTM)
{
/*
mint    short   :6  0000003F    minutes (0-59)
hr      short   :5  000007C0    hours (0-23)
dom     short   :5  0000F800    days of month (1-31)
mon     short   :4  000F0000    months (1-12)
yr      short   :9  1FF00000    years (1900-2411)-1900
wdy     short   :3  E0000000    weekday(Sunday=0
                                        Monday=1
( wdy can be ignored )                  Tuesday=2
                                        Wednesday=3
                                        Thursday=4
                                        Friday=5
                                        Saturday=6)
*/
    DateTime aDateTime(Date( 0 ), Time( 0 ));
    if( lDTTM )
    {
        USHORT lMin =(USHORT)(lDTTM & 0x0000003F);
        lDTTM >>= 6;
        USHORT lHour=(USHORT)(lDTTM & 0x0000001F);
        lDTTM >>= 5;
        USHORT lDay =(USHORT)(lDTTM & 0x0000001F);
        lDTTM >>= 5;
        USHORT lMon =(USHORT)(lDTTM & 0x0000000F);
        lDTTM >>= 4;
        USHORT lYear=(USHORT)(lDTTM & 0x000001FF) + 1900;
        aDateTime = DateTime(Date(lDay, lMon, lYear), Time(lHour, lMin));
    }
    return aDateTime;
}

WW8_CP WW8ScannerBase::WW8Fc2Cp( WW8_FC nFcPos ) const
{
    if( nFcPos == LONG_MAX )
        return LONG_MAX;

    if( pPieceIter )
    {                               // Complex File ?
        ULONG nOldPos = pPieceIter->GetIdx();
        pPieceIter->SetIdx( 0 );
        while( 1 )
        {
            long nCpStart, nCpEnd;
            void* pData;
            if( !pPieceIter->Get( nCpStart, nCpEnd, pData ) )
            {   // ausserhalb PLCFfpcd ?
                ASSERT( !this, "PLCFpcd-WW8Fc2Cp() ging schief" );
                break;
//              pPieceIter->SetIdx( nOldPos );
//              return 0;
            }
            BOOL bIsUnicode = FALSE;
            INT32 nFcStart  = SVBT32ToLong( ((WW8_PCD*)pData)->fc );
            if( 8 <= pWw8Fib->nVersion )
                nFcStart = WW8PLCFx_PCD::TransformPieceAddress( nFcStart,
                                                                bIsUnicode );

            INT32 nLen = (nCpEnd - nCpStart) * (bIsUnicode ? 2 : 1);

            if( nFcPos >= nFcStart && nFcPos < nFcStart + nLen )
            {   // gefunden
                pPieceIter->SetIdx( nOldPos );

                return nCpStart + ((nFcPos - nFcStart) / (bIsUnicode ? 2 : 1));
            }
            (*pPieceIter)++;
        }
        pPieceIter->SetIdx( nOldPos );      // nicht gefunden
        return LONG_MAX;
    }
    // No complex file
    return nFcPos - pWw8Fib->fcMin;
}

WW8_FC WW8ScannerBase::WW8Cp2Fc( WW8_CP nCpPos, BOOL* pIsUnicode,
                                WW8_CP* pNextPieceCp, BOOL* pTestFlag ) const
{
    if( pTestFlag )
        *pTestFlag = TRUE;
    if( LONG_MAX == nCpPos )
        return LONG_MAX;

    if( pPieceIter )
    {   // Complex File
        if( pNextPieceCp )
            *pNextPieceCp = LONG_MAX;

        if( !pPieceIter->SeekPos( nCpPos ) )
        {
            if( pTestFlag )
                *pTestFlag = FALSE;
            else
                ASSERT( !this, "Falscher CP an WW8Cp2Fc() uebergeben" );
            return LONG_MAX;
        }
        long nCpStart, nCpEnd;
        void* pData;
        if( !pPieceIter->Get( nCpStart, nCpEnd, pData ) )
        {
            if( pTestFlag )
                *pTestFlag = FALSE;
            else
                ASSERT( !this, "PLCFfpcd-Get ging schief" );
            return LONG_MAX;
        }
        if( pNextPieceCp )
            *pNextPieceCp = nCpEnd;
        BOOL bIsUnicode;
        if( !pIsUnicode )
            pIsUnicode = &bIsUnicode;

        WW8_FC nRet = SVBT32ToLong( ((WW8_PCD*)pData)->fc );
        if( 8 > pWw8Fib->nVersion )
            *pIsUnicode = FALSE;
        else
            nRet = WW8PLCFx_PCD::TransformPieceAddress( nRet, *pIsUnicode );


        nRet += (nCpPos - nCpStart) * (*pIsUnicode ? 2 : 1);

        return nRet;
    }

    // No complex file
    if( pIsUnicode )
        *pIsUnicode = FALSE;
    return nCpPos + pWw8Fib->fcMin;
}

//-----------------------------------------
//      class WW8ScannerBase
//-----------------------------------------

WW8PLCFpcd* WW8ScannerBase::OpenPieceTable( SvStream* pStr, WW8Fib* pWwF )
{
    if(    (    (8 > pWw8Fib->nVersion)
             && !pWwF->fComplex
           )
        || !pWwF->lcbClx ) return 0;

    WW8_FC nClxPos = pWwF->fcClx;
    INT32 nClxLen = pWwF->lcbClx;
    register INT32 nLeft = nClxLen;
    INT16 nGrpprl = 0;
    BYTE clxt;

    pStr->Seek( nClxPos );
    while( 1 ){                                 // Zaehle Zahl der Grpprls
        *pStr >> clxt;
        nLeft--;
        if( 2 == clxt )                         // PLCFfpcd ?
            break;                              // PLCFfpcd gefunden
        if( 1 == clxt )                         // clxtGrpprl ?
            nGrpprl++;
        UINT16 nLen;
        *pStr >> nLen;
        nLeft -= 2 + nLen;
        if( nLeft < 0 )
            return 0;                           // schiefgegangen
        pStr->SeekRel( nLen );                  // ueberlies grpprl
    }
    pStr->Seek( nClxPos );
    nLeft = nClxLen;
    pPieceGrpprls = new BYTE*[nGrpprl + 1];
    memset( pPieceGrpprls, 0, ( nGrpprl + 1 ) * 4 );
    nPieceGrpprls = nGrpprl;
    INT16 nAktGrpprl = 0;                       // lies Grpprls ein
    while( 1 )
    {
        *pStr >> clxt;
        nLeft--;
        if( 2 == clxt)                          // PLCFfpcd ?
            break;                              // PLCFfpcd gefunden
        UINT16 nLen;
        *pStr >> nLen;
        nLeft -= 2 + nLen;
        if( nLeft < 0 )
            return 0;                           // schiefgegangen
        if( 1 == clxt )                         // clxtGrpprl ?
        {
            BYTE* p = new BYTE[nLen+2];         // alloziere
            memcpy( p, &nLen, 2 );              // trage Laenge ein
            pStr->Read( p+2, nLen );            // lies grpprl
            pPieceGrpprls[nAktGrpprl++] = p;    // trage in Array ein
        }
        else
            pStr->SeekRel( nLen );              // ueberlies nicht-Grpprl
    }
    // lies Piece Table PLCF ein
    INT32 nPLCFfLen;
    *pStr >> nPLCFfLen;
    ASSERT( 65536 > nPLCFfLen, "PLCFfpcd ueber 64 k" );
    return new WW8PLCFpcd( pStr, pStr->Tell(), nPLCFfLen, 8 );
}

void WW8ScannerBase::DeletePieceTable()
{
    if( pPieceGrpprls )
    {
        for( BYTE** p = pPieceGrpprls; *p; p++ )
            delete[]( *p );
        delete[]( pPieceGrpprls );
        pPieceGrpprls = 0;
    }
}

WW8ScannerBase::WW8ScannerBase( SvStream* pSt, SvStream* pTblSt, SvStream* pDataSt,
                                WW8Fib* pWwFib )
    : pWw8Fib( pWwFib ), pPieceGrpprls( 0 ),
    pMainFdoa( 0 ), pHdFtFdoa( 0 ),
    pMainTxbx( 0 ), pMainTxbxBkd( 0 ), pHdFtTxbx( 0 ), pHdFtTxbxBkd( 0 ),
    nNoAttrScan( 0 )
{
    pPiecePLCF = OpenPieceTable( pTblSt, pWw8Fib );             // Complex
    if( pPiecePLCF )
    {
        pPieceIter      = new WW8PLCFpcd_Iter( *pPiecePLCF );
        pPLCFx_PCD      = new WW8PLCFx_PCD( pWwFib->nVersion, pPiecePLCF, 0,
                                            8 > pWw8Fib->nVersion );
        pPLCFx_PCDAttrs = new WW8PLCFx_PCDAttrs( pWwFib->nVersion, pPLCFx_PCD, this);
    }
    else
    {
        pPieceIter      = 0;
        pPLCFx_PCD      = 0;
        pPLCFx_PCDAttrs = 0;
    }

    // pChpPLCF and pPapPLCF may NOT be created before pPLCFx_PCD !!
    pChpPLCF = new WW8PLCFx_Cp_FKP( pSt, pTblSt, pDataSt, *this, CHP ); // CHPX
    pPapPLCF = new WW8PLCFx_Cp_FKP( pSt, pTblSt, pDataSt, *this, PAP ); // PAPX

    pSepPLCF = new WW8PLCFx_SEPX(   pSt, pTblSt, *pWwFib, 0 );          // SEPX

    // Footnotes
    pFtnPLCF = new WW8PLCFx_SubDoc( pTblSt, pWwFib->nVersion, 0,
                                pWwFib->fcPlcffndRef, pWwFib->lcbPlcffndRef,
                                pWwFib->fcPlcffndTxt, pWwFib->lcbPlcffndTxt,
                                2 );
    // Endnotes
    pEdnPLCF = new WW8PLCFx_SubDoc( pTblSt, pWwFib->nVersion, 0,
                                pWwFib->fcPlcfendRef, pWwFib->lcbPlcfendRef,
                                pWwFib->fcPlcfendTxt, pWwFib->lcbPlcfendTxt,
                                2 );
    // Anmerkungen
    pAndPLCF = new WW8PLCFx_SubDoc( pTblSt, pWwFib->nVersion, 0,
                                pWwFib->fcPlcfandRef, pWwFib->lcbPlcfandRef,
                                pWwFib->fcPlcfandTxt, pWwFib->lcbPlcfandTxt,
                                (8 > pWwFib->nVersion) ? 20 : 30 );

    // Fields Main Text
    pFldPLCF    = new WW8PLCFx_FLD( pTblSt, *pWwFib, MAN_MAINTEXT, 0 );
    // Fields Header / Footer
    pFldHdFtPLCF= new WW8PLCFx_FLD( pTblSt, *pWwFib, MAN_HDFT,     0 );
    // Fields Footnote
    pFldFtnPLCF = new WW8PLCFx_FLD( pTblSt, *pWwFib, MAN_FTN,      0 );
    // Fields Endnote
    pFldEdnPLCF = new WW8PLCFx_FLD( pTblSt, *pWwFib, MAN_EDN,      0 );
    // Fields Anmerkungen
    pFldAndPLCF = new WW8PLCFx_FLD( pTblSt, *pWwFib, MAN_AND,      0 );
    // Fields in Textboxes in Main Text
    pFldTxbxPLCF= new WW8PLCFx_FLD( pTblSt, *pWwFib, MAN_TXBX,     0 );
    // Fields in Textboxes in Header / Footer
    pFldTxbxHdFtPLCF=new WW8PLCFx_FLD(pTblSt,*pWwFib,MAN_TXBX_HDFT,0 );

    switch( pWw8Fib->nVersion ) // beachte: 6 steht fuer "6 ODER 7",  7 steht fuer "NUR 7"
    {
    case 6:
    case 7: if( pWwFib->fcPlcfdoaMom && pWwFib->lcbPlcfdoaMom )
                        pMainFdoa = new WW8PLCFspecial( pTblSt, pWwFib->fcPlcfdoaMom,
                                                pWwFib->lcbPlcfdoaMom, 6 );
                    if( pWwFib->fcPlcfdoaHdr && pWwFib->lcbPlcfdoaHdr )
                        pHdFtFdoa = new WW8PLCFspecial( pTblSt, pWwFib->fcPlcfdoaHdr,
                                                pWwFib->lcbPlcfdoaHdr, 6 );
                    break;
    case 8:
        if( pWwFib->fcPlcfspaMom && pWwFib->lcbPlcfspaMom )
            pMainFdoa = new WW8PLCFspecial( pTblSt, pWwFib->fcPlcfspaMom,
                                                    pWwFib->lcbPlcfspaMom, 26 );
        if( pWwFib->fcPlcfspaHdr && pWwFib->lcbPlcfspaHdr )
            pHdFtFdoa = new WW8PLCFspecial( pTblSt, pWwFib->fcPlcfspaHdr,
                                                    pWwFib->lcbPlcfspaHdr, 26 );
        // PLCF fuer TextBox-Break-Deskriptoren im Maintext
        if( pWwFib->fcPlcftxbxBkd && pWwFib->lcbPlcftxbxBkd )
            pMainTxbxBkd = new WW8PLCFspecial( pTblSt,
                                                pWwFib->fcPlcftxbxBkd,
                                                pWwFib->lcbPlcftxbxBkd, 0);
        // PLCF fuer TextBox-Break-Deskriptoren im Header-/Footer-Bereich
        if( pWwFib->fcPlcfHdrtxbxBkd && pWwFib->lcbPlcfHdrtxbxBkd )
            pHdFtTxbxBkd = new WW8PLCFspecial( pTblSt,
                                                pWwFib->fcPlcfHdrtxbxBkd,
                                                pWwFib->lcbPlcfHdrtxbxBkd, 0);
        break;


    default:ASSERT( !this, "Es wurde vergessen, nVersion zu kodieren!" );
    }

    // PLCF fuer TextBox-Stories im Maintext
    long nLenTxBxS = (8 > pWw8Fib->nVersion) ? 0 : 22;
    if( pWwFib->fcPlcftxbxTxt && pWwFib->lcbPlcftxbxTxt )
        pMainTxbx = new WW8PLCFspecial( pTblSt, pWwFib->fcPlcftxbxTxt,
                              pWwFib->lcbPlcftxbxTxt, nLenTxBxS );

    // PLCF fuer TextBox-Stories im Header-/Footer-Bereich
    if( pWwFib->fcPlcfHdrtxbxTxt && pWwFib->lcbPlcfHdrtxbxTxt )
        pHdFtTxbx = new WW8PLCFspecial( pTblSt, pWwFib->fcPlcfHdrtxbxTxt,
                              pWwFib->lcbPlcfHdrtxbxTxt, nLenTxBxS );

    pBook = new WW8PLCFx_Book( pSt, pTblSt, *pWwFib, 0 );
}

WW8ScannerBase::~WW8ScannerBase()
{
    DeletePieceTable();
    delete pPLCFx_PCDAttrs;
    delete pPLCFx_PCD;
    delete pPieceIter;
    delete pPiecePLCF;
    delete pBook;
    delete pFldEdnPLCF;
    delete pFldFtnPLCF;
    delete pFldAndPLCF;
    delete pFldHdFtPLCF;
    delete pFldPLCF;
    delete pFldTxbxPLCF;
    delete pFldTxbxHdFtPLCF;
    delete pEdnPLCF;
    delete pFtnPLCF;
    delete pAndPLCF;
    delete pSepPLCF;
    delete pPapPLCF;
    delete pChpPLCF;
    // vergessene Schaeflein
    delete pMainFdoa;
    delete pHdFtFdoa;
    delete pMainTxbx;
    delete pMainTxbxBkd;
    delete pHdFtTxbx;
    delete pHdFtTxbxBkd;
}

//-----------------------------------------
//      Stack fuer shorts
//-----------------------------------------

class UShortStk: private SvShorts
{
public:
    UShortStk():SvShorts( 10, 10 ) {}
    ~UShortStk() {}
    void Push( USHORT s ) { Insert( (USHORT)s, SvShorts::Count() ); }
    inline USHORT Top();
    inline USHORT Pop();
    USHORT Count() { return SvShorts::Count(); }
};

inline USHORT UShortStk::Top()
{
    USHORT nPos = SvShorts::Count() - 1;
    USHORT s = (*this)[ nPos ];
    return s;
}

inline USHORT UShortStk::Pop()
{
    USHORT nPos = SvShorts::Count() - 1;
    USHORT s = (*this)[ nPos ];
    Remove( nPos );
    return s;
}


//-----------------------------------------
//          Fields
//-----------------------------------------

static BOOL WW8SkipField( WW8PLCFspecial& rPLCF )
{
    void* pData;
    long nP;

    if( !rPLCF.Get( nP, pData ) )                   // Ende des PLCFspecial ?
        return FALSE;

    rPLCF++;

    if((((BYTE*)pData)[0] & 0x1f ) != 0x13 )        // Kein Anfang ?
        return TRUE;                                // Bei Fehler nicht abbrechen

    if( !rPLCF.Get( nP, pData ) )
        return FALSE;


    while((((BYTE*)pData)[0] & 0x1f ) == 0x13 ){    // immer noch neue (nested) Anfaenge ?
        WW8SkipField( rPLCF );                      // nested Field im Beschreibungsteil
        if( !rPLCF.Get( nP, pData ) )
            return FALSE;
    }

    if((((BYTE*)pData)[0] & 0x1f ) == 0x14 ){       // Field Separator ?
        rPLCF++;

        if( !rPLCF.Get( nP, pData ) )
            return FALSE;

        while((((BYTE*)pData)[0] & 0x1f ) == 0x13 ){// immer noch neue (nested) Anfaenge ?
            WW8SkipField( rPLCF );                  // nested Field im Resultatteil
            if( !rPLCF.Get( nP, pData ) )
                return FALSE;
        }
    }
    rPLCF++;

    return TRUE;
}

BOOL WW8GetFieldPara( BYTE nVersion, WW8PLCFspecial& rPLCF, WW8FieldDesc& rF )
{
    void* pData;
    ULONG nOldIdx = rPLCF.GetIdx();

    rF.nLen = rF.nId = rF.nOpt = rF.bCodeNest = rF.bResNest = 0;

    if( !rPLCF.Get( rF.nSCode, pData ) )             // Ende des PLCFspecial ?
        goto Err;

    rPLCF++;

    if((((BYTE*)pData)[0] & 0x1f ) != 0x13 )        // Kein Anfang ?
        goto Err;

    rF.nId = ((BYTE*)pData)[1];

    if( !rPLCF.Get( rF.nLCode, pData ) )
        goto Err;

    rF.nSRes = rF.nLCode;                           // Default
    rF.nSCode++;                                    // ohne Marken
    rF.nLCode -= rF.nSCode;                         // Pos zu Laenge

    while((((BYTE*)pData)[0] & 0x1f ) == 0x13 ){    // immer noch neue (nested) Anfaenge ?
        WW8SkipField( rPLCF );                      // nested Field im Beschreibungsteil
        rF.bCodeNest = TRUE;
        if( !rPLCF.Get( rF.nSRes, pData ) )
            goto Err;
    }

    if((((BYTE*)pData)[0] & 0x1f ) == 0x14 ){       // Field Separator ?
        rPLCF++;

        if( !rPLCF.Get( rF.nLRes, pData ) )
            goto Err;

        while((((BYTE*)pData)[0] & 0x1f ) == 0x13 ){// immer noch neue (nested) Anfaenge ?
            WW8SkipField( rPLCF );                  // nested Field im Resultatteil
            rF.bResNest = TRUE;
            if( !rPLCF.Get( rF.nLRes, pData ) )
                goto Err;
        }
        rF.nLen = rF.nLRes - rF.nSCode + 2;         // nLRes ist noch die Endposition
        rF.nLRes -= rF.nSRes;                       // nun: nLRes = Laenge
        rF.nSRes++;                                 // Endpos encl. Marken
        rF.nLRes--;

    }else{
        rF.nLRes = 0;                               // Kein Result vorhanden
        rF.nLen = rF.nSRes - rF.nSCode + 2;         // Gesamtlaenge
    }

    rPLCF++;
    if((((BYTE*)pData)[0] & 0x1f ) == 0x15 ){       // Field Ende ?
                                                    // INDEX-Fld hat Bit7 gesetzt!?!
        rF.nOpt = ((BYTE*)pData)[1];                // Ja -> Flags uebernehmen
    }else{
        rF.nId = 0;                                 // Nein -> Feld ungueltig
    }

    rPLCF.SetIdx( nOldIdx );
    return TRUE;
Err:
    rPLCF.SetIdx( nOldIdx );
    return FALSE;
}


//-----------------------------------------


// WW8ReadPString liest einen Pascal-String ein und gibt ihn zurueck. Der Pascal-
// String hat am Ende ein \0, der aber im Laengenbyte nicht mitgezaehlt wird.
// Der Speicher fuer den Pascalstring wird alloziert.
String WW8ReadPString( SvStream& rStrm, rtl_TextEncoding eEnc, BOOL bAtEndSeekRel1 )
{
    UINT8 b;
    rStrm >> b;

    ByteString aByteStr;    // Alloc methode automatically sets Zero at the end
    sal_Char*  pByteData = aByteStr.AllocBuffer( b );

    sal_Size nWasRead = rStrm.Read( pByteData, b );
    if( bAtEndSeekRel1 )
        rStrm.SeekRel( 1 ); // ueberspringe das Null-Byte am Ende.

    if( nWasRead != b )
        aByteStr.ReleaseBufferAccess( nWasRead );

    return String( aByteStr, eEnc );
}


String WW8Read_xstz( SvStream& rStrm, USHORT nChars, BOOL bAtEndSeekRel1 )
{
    UINT16 b;

    if( nChars )
        b = nChars;
    else
        rStrm >> b;

    String aStr;           // Alloc methode automatically sets Zero at the end
    sal_Unicode* pData = aStr.AllocBuffer( b );

    sal_Size nWasRead = rStrm.Read( (sal_Char*)pData, b * 2 );
    if( nWasRead != b*2 )
        aStr.ReleaseBufferAccess( b = (nWasRead / 2) );

#ifdef __BIGENDIAN
    ULONG n;
    sal_Unicode *pWork;
    for( n = 0, pWork = pData; n < b; ++n, ++pWork )
        *pWork = SWAPSHORT( *pWork );
#endif // ifdef __BIGENDIAN

// 2000/03/27 KHZ UNICODE
// still missing: unicode-back conversion AND 0xF0.. specials

    if( bAtEndSeekRel1 )
        rStrm.SeekRel( 2 ); // ueberspringe das Null-Character am Ende.

    return aStr;
}


USHORT WW8ScannerBase::WW8ReadString( SvStream& rStrm, String& rStr,
                                      WW8_CP nAktStartCp, long nTotalLen,
                                      rtl_TextEncoding eEnc ) const
                                      // , unsigned -c-h-a-r-** ppStr ) const
{
    // Klartext einlesen, der sich ueber mehrere Pieces erstrecken kann
    rStr.Erase();

    long   nTotalRead = 0;
    WW8_CP nBehindTextCp = nAktStartCp + nTotalLen;
    WW8_CP nNextPieceCp  = nBehindTextCp; // Initialisierung wichtig fuer Ver6
    do {
        BOOL bIsUnicode, bPosOk;
        WW8_FC fcAct = WW8Cp2Fc( nAktStartCp, &bIsUnicode,
                                 &nNextPieceCp, &bPosOk);

        if( !bPosOk ) break;  // vermutlich uebers Dateiende hinaus gezielt, macht nix!

        rStrm.Seek( fcAct );

        long nLen = (    (nNextPieceCp < nBehindTextCp)
                        ? nNextPieceCp
                        : nBehindTextCp
                    )
                    - nAktStartCp;

        if( 0 >= nLen ) break;

        if( nLen > USHRT_MAX - 1 )
            nLen = USHRT_MAX - 1;

        if( bIsUnicode )
            rStr.Append( WW8Read_xstz( rStrm, (USHORT)nLen, FALSE ) );
        else
        {
            ByteString aByteStr;    // Alloc methode automatically sets Zero at the end
            sal_Char*  pByteData = aByteStr.AllocBuffer( nLen );

            sal_Size nWasRead = rStrm.Read( pByteData, nLen );
            if( nWasRead != nLen )
                aByteStr.ReleaseBufferAccess( nWasRead );

            rStr += String( aByteStr, eEnc );
        }
        nTotalRead  += nLen;
        nAktStartCp += nLen;
        if( nTotalRead != rStr.Len() ) break;
    }
    while( nTotalRead < nTotalLen );

    return rStr.Len();
}


//-----------------------------------------
//              WW8PLCFspecial
//-----------------------------------------

// Bei nStartPos < 0 wird das erste Element des PLCFs genommen
WW8PLCFspecial::WW8PLCFspecial( SvStream* pSt, long nFilePos, long nPLCF,
            long nStruct, long nStartPos, BOOL bNoEnd )
:nIdx( 0 ), nStru( nStruct )
{
    nIMax = ( nPLCF - 4 ) / ( 4 + nStruct );
    pPLCF_PosArray = new INT32[ ( nPLCF + 3 ) / 4 ];    // Pointer auf Pos- u. Struct-Array

    long nOldPos = pSt->Tell();

    pSt->Seek( nFilePos );
    pSt->Read( pPLCF_PosArray, nPLCF );
#ifdef __BIGENDIAN
    for( nIdx = 0; nIdx <= nIMax; nIdx++ ){
        pPLCF_PosArray[nIdx] = SWAPLONG( pPLCF_PosArray[nIdx] );
    }
    nIdx = 0;
#endif // __BIGENDIAN
    if( bNoEnd ) nIMax++;
    if( nStruct )
        pPLCF_Contents = (BYTE*)&pPLCF_PosArray[nIMax + 1]; // Pointer auf Inhalts-Array
    else
        pPLCF_Contents = 0;                         // kein Inhalt
    if( nStartPos >= 0 )
        SeekPos( nStartPos );

    pSt->Seek( nOldPos );
}

// WW8PLCFspecial::SeekPos() stellt den WW8PLCFspecial auf die Stelle nPos, wobei auch noch der
// Eintrag benutzt wird, der vor nPos beginnt und bis hinter nPos reicht.
// geeignet fuer normale Attribute. Allerdings wird der Attributanfang nicht
// auf die Position nPos korrigiert.
BOOL WW8PLCFspecial::SeekPos( long nPos )
{
    long nP = nPos;

    if( nP < pPLCF_PosArray[0] )
    {
        nIdx = 0;
        return FALSE;                   // Nicht gefunden: nPos unterhalb kleinstem Eintrag
    }

    // Search from beginning?
    if( (1 > nIdx) || (nP < pPLCF_PosArray[ nIdx-1 ]) )
        nIdx = 1;

    long nI   = nIdx ? nIdx : 1;
    long nEnd = nIMax;
#ifndef KHZ_TEST0
    for(int n = (1==nIdx ? 1 : 2); n; --n )
    {
        for( ; nI <=nEnd; ++nI)
        {                                   // Suchen mit um 1 erhoehtem Index
            if( nP < pPLCF_PosArray[nI] )
            {                               // Position gefunden
                nIdx = nI - 1;              // nI - 1 ist der richtige Index
                return TRUE;                // ... und fertig
            }
        }
        nI   = 1;
        nEnd = nIdx-1;
    }
#else
    for( nI=1; nI<=nIMax; nI++){        // Suchen mit um 1 erhoehtem Index
        if( nP < pPLCF_PosArray[nI] ){          // Position gefunden
            nIdx = nI - 1;              // nI - 1 ist der richtige Index
            return TRUE;                // ... und fertig
        }
    }
#endif
    nIdx = nIMax;                       // Nicht gefunden, groesser als alle Eintraege
    return FALSE;
}

// WW8PLCFspecial::SeekPosExact() wie SeekPos(), aber es wird sichergestellt, dass kein
// Attribut angeschnitten wird, d.h. das naechste gelieferte Attribut beginnt
// auf oder hinter nPos. Wird benutzt fuer Felder + Bookmarks.
BOOL WW8PLCFspecial::SeekPosExact( long nPos )
{
    long nP = nPos;

    if( nP < pPLCF_PosArray[0] )
    {
        nIdx = 0;
        return FALSE;                   // Nicht gefunden: nPos unterhalb kleinstem Eintrag
    }
    // Search from beginning?
    if( nP <=pPLCF_PosArray[nIdx] )
        nIdx = 0;

    long nI   = nIdx ? nIdx-1 : 0;
    long nEnd = nIMax;
#ifndef KHZ_TEST0
    for(int n = (0==nIdx ? 1 : 2); n; --n )
    {
        for( ; nI < nEnd; ++nI)
        {
            if( nP <=pPLCF_PosArray[nI] )
            {                           // Position gefunden
                nIdx = nI;              // nI     ist der richtige Index
                return TRUE;            // ... und fertig
            }
        }
        nI   = 0;
        nEnd = nIdx;
    }
#else
    for( nI=0; nI<nIMax; nI++){         // Suchen
        if( nP <= pPLCF_PosArray[nI] ){         // Position gefunden
            nIdx = nI;                  // nI ist der richtige Index
            return TRUE;                // ... und fertig
        }
    }
#endif
    nIdx = nIMax;                       // Nicht gefunden, groesser als alle Eintraege
    return FALSE;
}

BOOL WW8PLCFspecial::Get( long& rPos, void*& rpValue )
{
    if ( nIdx >= nIMax ){
        rPos = LONG_MAX;
        return FALSE;
    }
    rPos = pPLCF_PosArray[nIdx];
    rpValue = pPLCF_Contents ? (void*)&pPLCF_Contents[nIdx * nStru] : 0;
    return TRUE;
}

BOOL WW8PLCFspecial::GetData( long nIdx, long& rPos, void*& rpValue )
{
    if ( nIdx >= nIMax ){
        rPos = LONG_MAX;
        return FALSE;
    }
    rPos = pPLCF_PosArray[nIdx];
    rpValue = pPLCF_Contents ? (void*)&pPLCF_Contents[nIdx * nStru] : 0;
    return TRUE;
}

//-----------------------------------------
//              WW8PLCF z.B. fuer SEPX
//-----------------------------------------

// Ctor fuer *andere* als Fkps
// Bei nStartPos < 0 wird das erste Element des PLCFs genommen
WW8PLCF::WW8PLCF( SvStream* pSt, long nFilePos, long nPLCF,
          long nStruct, long nStartPos )
:nIdx( 0 ), nStru( nStruct )
{
    ASSERT( nPLCF, "WW8PLCF: nPLCF ist Null!" );

    nIMax = ( nPLCF - 4 ) / ( 4 + nStruct );

    ReadPLCF( pSt, nFilePos, nPLCF );

    if( nStartPos >= 0 )
        SeekPos( nStartPos );
}

// Ctor *nur* fuer Fkps
// Die letzten 2 Parameter sind fuer PLCF.Chpx und PLCF.Papx noetig.
// ist ncpN != 0, dann wird ein unvollstaendiger PLCF vervollstaendigt.
// Das ist bei WW6 bei Resourcenmangel und bei WordPad (W95) immer noetig.
// Bei nStartPos < 0 wird das erste Element des PLCFs genommen
WW8PLCF::WW8PLCF( SvStream* pSt, long nFilePos, long nPLCF,
          long nStruct, long nStartPos, long nPN, long ncpN )
:nIdx( 0 ), nStru( nStruct )
{
    nIMax = ( nPLCF - 4 ) / ( 4 + nStruct );

    if( nIMax >= (long) ncpN )
        ReadPLCF( pSt, nFilePos, nPLCF );
    else
        GeneratePLCF( pSt, nPN, ncpN );

    if( nStartPos >= 0 )
        SeekPos( nStartPos );

// Damit man sich den Inhalt im Debugger ansehen kann
#ifdef DEBUG
    INT32  (*p1)[200] = (INT32  (*)[200])pPLCF_PosArray;
    USHORT (*p2)[200] = (USHORT (*)[200])pPLCF_Contents;
    p2 = p2;
#endif
}

void WW8PLCF::ReadPLCF( SvStream* pSt, long nFilePos, long nPLCF )
{
    pPLCF_PosArray = new INT32[ ( nPLCF + 3 ) / 4 ];    // Pointer auf Pos-Array

    long nOldPos = pSt->Tell();

    pSt->Seek( nFilePos );
    pSt->Read( pPLCF_PosArray, nPLCF );
#ifdef __BIGENDIAN
    for( nIdx = 0; nIdx <= nIMax; nIdx++ ){
      pPLCF_PosArray[nIdx] = SWAPLONG( pPLCF_PosArray[nIdx] );
    }
    nIdx = 0;
#endif // __BIGENDIAN
    pPLCF_Contents = (BYTE*)&pPLCF_PosArray[nIMax + 1];     // Pointer auf Inhalts-Array

    pSt->Seek( nOldPos );
}

void WW8PLCF::GeneratePLCF( SvStream* pSt, long nPN,
                       long ncpN )
{
    ASSERT( nIMax < (long)ncpN, "Pcl.Fkp: Warum ist PLCF zu gross ?" );
    nIMax = ncpN;
    long nSiz = 6 * nIMax + 4;
    pPLCF_PosArray = new INT32[ ( nSiz + 3 ) / 4 ]; // Pointer auf Pos-Array
    memset( pPLCF_PosArray, 0, (size_t)nSiz );

    INT32 nFc;
    USHORT i;

    for( i = 0; i < ncpN; i++ ){        // Baue FC-Eintraege
        pSt->Seek( ( nPN + i ) << 9 );  // erster FC-Eintrag jedes Fkp
        *pSt >> nFc;
        pPLCF_PosArray[i] = nFc;
    }
    ULONG nLastFkpPos = ( ( nPN + nIMax - 1 ) << 9 );
    pSt->Seek( nLastFkpPos + 511 );     // Anz. Fkp-Eintraege des letzten Fkp
    BYTE nb;
    *pSt >> nb;
    pSt->Seek( nLastFkpPos + nb * 4 );  // letzer FC-Eintrag des letzten Fkp
    *pSt >> nFc;
    pPLCF_PosArray[nIMax] = nFc;        // Ende des letzten Fkp

    pPLCF_Contents = (BYTE*)&pPLCF_PosArray[nIMax + 1]; // Pointer auf Inhalts-Array
    USHORT* p = (USHORT*)pPLCF_Contents;

    for( i = 0; i < ncpN; i++ )         // Baue PNs
        p[i] = nPN + i;

#ifdef DEBUG    // Damit man sich den Inhalt im Debugger ansehen kann
    INT32  (*p1)[200] = (INT32  (*)[200])pPLCF_PosArray;
    USHORT (*p2)[200] = (USHORT (*)[200])pPLCF_Contents;
    p2 = p2;
#endif
}

BOOL WW8PLCF::SeekPos( long nPos )
{
    long nP = nPos;

    if( nP < pPLCF_PosArray[0] )
    {
        nIdx = 0;
        return FALSE;                   // Nicht gefunden: nPos unterhalb kleinstem Eintrag
    }
    // Search from beginning?
    if( (1 > nIdx) || (nP < pPLCF_PosArray[ nIdx-1 ]) )
        nIdx = 1;

    long nI   = nIdx ? nIdx : 1;
    long nEnd = nIMax;
#ifndef KHZ_TEST0
    for(int n = (1==nIdx ? 1 : 2); n; --n )
    {
        for( ; nI <=nEnd; ++nI)             // Suchen mit um 1 erhoehtem Index
        {
            if( nP < pPLCF_PosArray[nI] )   // Position gefunden
            {
                nIdx = nI - 1;              // nI - 1 ist der richtige Index
                return TRUE;                // ... und fertig
            }
        }
        nI   = 1;
        nEnd = nIdx-1;
    }
#else
    for( nI=1; nI<=nIMax; nI++){
        if( nP < pPLCF_PosArray[nI] ){
            nIdx = nI - 1;              // nI - 1 ist der richtige Index
            return TRUE;                // ... und fertig
        }
    }
#endif
    nIdx = nIMax;                       // Nicht gefunden, groesser als alle Eintraege
    return FALSE;
}//4,11,0,11,4,0,0,0,0,0,11,11,11,11,

BOOL WW8PLCF::Get( long& rStart, long& rEnd, void*& rpValue )
{
    if ( nIdx >= nIMax )
    {
        rStart = rEnd = LONG_MAX;
        return FALSE;
    }
    rStart = pPLCF_PosArray[ nIdx     ];
    rEnd   = pPLCF_PosArray[ nIdx + 1 ];
    rpValue = (void*)&pPLCF_Contents[nIdx * nStru];
    return TRUE;
}

long WW8PLCF::Where()
{
    if ( nIdx >= nIMax )
        return LONG_MAX;

    return pPLCF_PosArray[nIdx];
}

//-----------------------------------------
//              WW8PLCFpcd
//-----------------------------------------

WW8PLCFpcd::WW8PLCFpcd( SvStream* pSt, long nFilePos, long nPLCF, long nStruct )
:nStru( nStruct )
{
    nIMax = ( nPLCF - 4 ) / ( 4 + nStruct );
    pPLCF_PosArray = new INT32[ ( nPLCF + 3 ) / 4 ];    // Pointer auf Pos-Array

    long nOldPos = pSt->Tell();

    pSt->Seek( nFilePos );
    pSt->Read( pPLCF_PosArray, nPLCF );
#ifdef __BIGENDIAN
    {
        for( long nI = 0; nI <= nIMax; nI++ )
          pPLCF_PosArray[nI] = SWAPLONG( pPLCF_PosArray[nI] );
    }
#endif // __BIGENDIAN

    pPLCF_Contents = (BYTE*)&pPLCF_PosArray[nIMax + 1]; // Pointer auf Inhalts-Array

    for( INT32 nI = 0; nI < nIMax; nI++ )
    {
        ULONG nKey = (ULONG)(0x8FFFFFFF & ((WW8_PCD1&)pPLCF_Contents[nI * nStru]).fc);
        aFC_sort.Insert( nKey, nI );
    }

    pSt->Seek( nOldPos );
}


ULONG WW8PLCFpcd::FindIdx( WW8_FC nFC ) const
{
    ULONG nFound;
    if( !aFC_sort.SearchKey( nFC, &nFound ) )
    {
        if( !nFound )
            nFound = ULONG_MAX;
        else
            --nFound;  // we were behind the list because nFc is greater max. list entry
    }
    return ULONG_MAX == nFound ? ULONG_MAX : aFC_sort.GetObject( nFound );
}


// Bei nStartPos < 0 wird das erste Element des PLCFs genommen
WW8PLCFpcd_Iter::WW8PLCFpcd_Iter( WW8PLCFpcd& rPLCFpcd, long nStartPos /* = -1 */ )
:rPLCF( rPLCFpcd ), nIdx( 0 )
{
    if( nStartPos >= 0 )
        SeekPos( nStartPos );
}

BOOL WW8PLCFpcd_Iter::SeekPos( long nPos )
{
    long nP = nPos;

    if( nP < rPLCF.pPLCF_PosArray[0] )
    {
        nIdx = 0;
        return FALSE;                   // Nicht gefunden: nPos unterhalb kleinstem Eintrag
    }
    // Search from beginning?
    if( (1 > nIdx) || (nP < rPLCF.pPLCF_PosArray[ nIdx-1 ]) )
        nIdx = 1;

    long nI   = nIdx ? nIdx : 1;
    long nEnd = rPLCF.nIMax;
#ifndef KHZ_TEST0
    for(int n = (1==nIdx ? 1 : 2); n; --n )
    {
        for( ; nI <=nEnd; ++nI)
        {                               // Suchen mit um 1 erhoehtem Index
            if( nP < rPLCF.pPLCF_PosArray[nI] )
            {                           // Position gefunden
                nIdx = nI - 1;          // nI - 1 ist der richtige Index
                return TRUE;            // ... und fertig
            }
        }
        nI   = 1;
        nEnd = nIdx-1;
    }
#else
    for( nI=1; nI<=rPLCF.nIMax; nI++){  // Suchen mit um 1 erhoehtem Index
        if( nP < rPLCF.pPLCF_PosArray[nI] ){// Position gefunden
            nIdx = nI - 1;              // nI - 1 ist der richtige Index
            return TRUE;                // ... und fertig
        }
    }
#endif
    nIdx = rPLCF.nIMax;                 // Nicht gefunden, groesser als alle Eintraege
    return FALSE;
}//0,0,...0,1,0,0,1,0,0,1,1,...1,2,1,1,2,1,1,2,2,...2,4,2,2,3,2,2,3,3,...3,5,7,8,d,10,11,

/*
BOOL WW8PLCFpcd_Iter::SeekMaxMainFC( WW8Fib& rWwF, long& rMaxPosData )
{
    long nCpStart, nCpEnd;
    void* pData;
    nIdx = rPLCF.nIMax-1;
    if( !Get( nCpStart, nCpEnd, pData ) )
    {
        ASSERT( !this, "SeekMaxMainFC findet Eintrag zu nIdx nicht" );
        return FALSE;
    }
    BOOL bIsUnicode;
    WW8_FC nActPosData = WW8PLCFx_PCD::TransformPieceAddress(
                                        SVBT32ToLong( ( (WW8_PCD*)pData )->fc ),
                                        &bIsUnicode );

//  rMaxPosData = nActPosData + ((rWwF.ccpText - nCpStart) * (bIsUnicode ? 2 : 1));

    rMaxPosData = nActPosData + rWwF.ccpText - nCpStart;

    return (0 < rMaxPosData);
}
*/

BOOL WW8PLCFpcd_Iter::Get( long& rStart, long& rEnd, void*& rpValue )
{
    if( nIdx >= rPLCF.nIMax )
    {
        rStart = rEnd = LONG_MAX;
        return FALSE;
    }
    rStart = rPLCF.pPLCF_PosArray[nIdx];
    rEnd = rPLCF.pPLCF_PosArray[nIdx + 1];
    rpValue = (void*)&rPLCF.pPLCF_Contents[nIdx * rPLCF.nStru];
#ifdef DEBUG
    WW8_PCD1* p = (WW8_PCD1*)rpValue;
    p = p;
#endif
    return TRUE;
}

long WW8PLCFpcd_Iter::Where()
{
    if ( nIdx >= rPLCF.nIMax )
        return LONG_MAX;

    return rPLCF.pPLCF_PosArray[nIdx];
}

//-----------------------------------------

WW8PLCFx_Fc_FKP::WW8Fkp::WW8Fkp( BYTE nFibVer, SvStream* pSt, SvStream* pDataSt,
                                 long _nFilePos, long nItemSiz,
                                 ePLCFT ePl, WW8_FC nStartFc )
:nItemSize( nItemSiz ), nFilePos( _nFilePos ), ePLCF( ePl ), nIdx( 0 )
{
    nVersion = nFibVer;

    pFkp = (BYTE*)new INT32[128]; // 512 Byte

    long nOldPos = pSt->Tell();

    pSt->Seek( nFilePos );
    pSt->Read( pFkp, 512 );
    nIMax = pFkp[511];
#ifdef __BIGENDIAN
    register UINT32* p;
    for( nIdx = 0, p = (UINT32*)pFkp; nIdx <= nIMax; nIdx++, p++ )
        *p = SWAPLONG( *p );
#endif // __BIGENDIAN


    // Pointer auf Offset-Bereich in *pFkp
    BYTE* pOfs = pFkp + (nIMax + 1) * 4;

    pGrpprl = new WW8Grpprl[nIMax];
    memset( pGrpprl, 0, sizeof( WW8Grpprl )*nIMax );

    WW8Grpprl* pTmpGrpprl = pGrpprl;
    for( nIdx = 0; nIdx < nIMax; nIdx++, pTmpGrpprl++ )
    {
        USHORT nOfs = (*(pOfs + nIdx * nItemSize)) * 2;

        if ( nOfs )
        {
            switch ( ePLCF )
            {
            case CHP:
                    pTmpGrpprl->nLen  = (short)pFkp[ nOfs ];
                    pTmpGrpprl->pData = pFkp + nOfs + 1;
                    break;
            case PAP:
                    BYTE nDelta = 0;//(8 > nVersion) ? 0 : 1;
                    pTmpGrpprl->nLen = pFkp[ nOfs ];
                    if( 8 <= nVersion && !pTmpGrpprl->nLen )
                    {
                        pTmpGrpprl->nLen = pFkp[ nOfs+1 ];
                        nDelta++;
                    }
                    pTmpGrpprl->nIStd = SVBT16ToShort( (BYTE*) pFkp+nOfs+1+nDelta );
                    pTmpGrpprl->pData = pFkp + nOfs + 3+nDelta ;
                    USHORT nSpId = WW8GetSprmId( nVersion, pTmpGrpprl->pData );
                    if( 0x6645 == nSpId || 0x6646 == nSpId )
                    {
                        UINT32 nPos  = *(UINT32*)(pTmpGrpprl->pData + 2);
                        UINT32 nCurr = pDataSt->Tell();
                        pDataSt->Seek( nPos );
                        *pDataSt >> pTmpGrpprl->nLen;
                        pTmpGrpprl->pData = new BYTE[pTmpGrpprl->nLen];
                        pTmpGrpprl->bMustDelete = TRUE;
                        pDataSt->Read( pTmpGrpprl->pData, pTmpGrpprl->nLen );

                        pDataSt->Seek( nCurr );
                    }
                    else
                    {
                        pTmpGrpprl->nLen *= 2;
                        pTmpGrpprl->nLen -= 2;
                    }
                    break;
            }
        }
    }

    nIdx = 0;

    if( nStartFc >= 0 )
        SeekPos( nStartFc );

    pSt->Seek( nOldPos );
}

WW8PLCFx_Fc_FKP::WW8Fkp::~WW8Fkp()
{
    WW8Grpprl* pTmpGrpprl = pGrpprl;
    for( nIdx = 0; nIdx < nIMax; nIdx++, pTmpGrpprl++ )
    {
        if( pTmpGrpprl->bMustDelete )
            delete pTmpGrpprl->pData;
    }
    delete pGrpprl;
    delete pFkp;
}

BOOL WW8PLCFx_Fc_FKP::WW8Fkp::SeekPos( WW8_FC nFc )
{
    if( nFc < ((WW8_FC*)pFkp)[0] )
    {
        nIdx = 0;
        return FALSE;                   // Nicht gefunden: nPos unterhalb kleinstem Eintrag
    }
    // Search from beginning?
    if( (1 > nIdx) || (nFc < ((WW8_FC*)pFkp)[ nIdx-1 ]) )
        nIdx = 1;

    long nI   = nIdx ? nIdx : 1;
    long nEnd = nIMax;
#ifndef KHZ_TEST0
    for(int n = (1==nIdx ? 1 : 2); n; --n )
    {
        for( ; nI <=nEnd; ++nI)
        {                               // Suchen mit um 1 erhoehtem Index
            if( nFc < ((WW8_FC*)pFkp)[nI] )
            {                           // Position gefunden
                nIdx = nI - 1;          // nI - 1 ist der richtige Index
                return TRUE;            // ... und fertig
            }
        }
        nI   = 1;
        nEnd = nIdx-1;
    }
#else
    for( nI=1; nI<=nIMax; nI++){        // Suchen mit um 1 erhoehtem Index
        if( nFc < ((WW8_FC*)pFkp)[nI] ){    // Position gefunden
            nIdx = nI - 1;              // nI - 1 ist der richtige Index
            return TRUE;                // ... und fertig
        }
    }
#endif
    nIdx = nIMax;                       // Nicht gefunden, groesser als alle Eintraege
    return FALSE;
}//32,0,1,32,0,0,1,1,2,2,3,4,5,

BYTE* WW8PLCFx_Fc_FKP::WW8Fkp::Get( WW8_FC& rStart, WW8_FC& rEnd, short& rLen )
{
    rLen = 0;

    if( nIdx >= nIMax ){
        rStart = LONG_MAX;
        return 0;
    }

    rStart = ((WW8_FC*)pFkp)[nIdx  ];
    rEnd   = ((WW8_FC*)pFkp)[nIdx+1];

    BYTE* pSprms = GetLenAndIStdAndSprms( rLen );
    return pSprms;
}

void WW8PLCFx_Fc_FKP::WW8Fkp::SetIdx( ULONG nI )
{
    if( nI < nIMax) nIdx = (short)nI;
}

BYTE* WW8PLCFx_Fc_FKP::WW8Fkp::GetLenAndIStdAndSprms(short& rLen)
{
    WW8Grpprl* pTmpGrpprl = pGrpprl + nIdx;

    rLen  = pTmpGrpprl->nLen;

    return pTmpGrpprl->pData;
}


BYTE* WW8FindSprm( USHORT nId, long nLen, BYTE* pSprms, BYTE nVersion )
{
    WW8SprmIter aSprmIter( pSprms, nLen, nVersion );

    while( aSprmIter.GetSprms() )
    {
        if( aSprmIter.GetAktId() == nId )
            return aSprmIter.GetAktParams();    // SPRM found!
        aSprmIter++;
    }

    return 0;                                   // SPRM _not_ found
}


BYTE* WW8PLCFx_Fc_FKP::WW8Fkp::HasSprm( USHORT nId )
{
    if( nIdx >= nIMax )
        return 0;

    short nLen;
    BYTE* pSprms = GetLenAndIStdAndSprms( nLen );

    return WW8FindSprm( nId, nLen, pSprms, nVersion );
}

ULONG WW8PLCFx_Fc_FKP::WW8Fkp::GetParaHeight() const
{
    if( ePLCF != PAP ){
        ASSERT( !this, "Fkp::GetParaHeight: Falscher Fkp" );
        return 0;
    }
    if( nIdx >= nIMax )
        return 0;

    WW8_PHE_Base* pPhe = (WW8_PHE_Base*)( (pFkp + (nIMax + 1) * 4)
                                        + ( nIdx * nItemSize ) + 1 );

    if( pPhe->aBits1 & 0x2 )                    // fUnk
        return 0;                               // invalid

    if( pPhe->aBits1 & 0x4 ){                   // fDiffLines
        return SVBT16ToShort( pPhe->dyl );      // Gesamthoehe
    }else{                                      // nlMac gleich hohe Zeilen
        INT32 nH = (INT16)SVBT16ToShort( pPhe->dyl );   // Zeilenhoehe
        if( nH < 0 )                            // negative Werte wollen wir
            nH = 0;                             // nicht
        nH *= (INT32)pPhe->nlMac;               // Hoehe einer Zeile * Zeilen
        return (ULONG)nH;
    }
}

//-----------------------------------------

void WW8PLCFx::GetSprms( WW8PLCFxDesc* p )
{
    ASSERT( !this, "Falsches GetSprms gerufen" );
    p->nStartPos = p->nEndPos = LONG_MAX;
    p->nSprmsLen = 0;
    p->pMemPos = 0;
    p->bRealLineEnd = FALSE;
    return;
}

long WW8PLCFx::GetNoSprms( long& rStart, long& rEnd, long& rLen )
{
    ASSERT( !this, "Falsches GetNoSprms gerufen" );
    rStart = rEnd = LONG_MAX;
    rLen = 0;
    return 0;
}

// ...Idx2: Default: ignorieren
ULONG WW8PLCFx::GetIdx2() const
{
    return 0;
}

void WW8PLCFx::SetIdx2( ULONG nIdx )
{
}

//-----------------------------------------
BOOL WW8PLCFx_Fc_FKP::NewFkp()
{
    long nPLCFStart, nPLCFEnd;
    void* pPage;

    static int __READONLY_DATA WW8FkpSizeTabVer6[ PLCF_END ] = {
                                                    1,  7, 0 /*, 0, 0, 0*/ };
    static int __READONLY_DATA WW8FkpSizeTabVer8[ PLCF_END ] = {
                                                    1, 13, 0 /*, 0, 0, 0*/ };
    const int* pFkpSizeTab;
    switch( GetVersion() )
    {
    case 6:
    case 7: pFkpSizeTab = WW8FkpSizeTabVer6;
            break;
    case 8: pFkpSizeTab = WW8FkpSizeTabVer8;
            break;
    default:// Programm-Fehler!
            ASSERT( !this, "Es wurde vergessen, nVersion zu kodieren!" );
            return FALSE;
    }

    if (!pPLCF->Get( nPLCFStart, nPLCFEnd, pPage ))
    {
        DELETEZ( pFkp );
        return FALSE;                                   // PLCF fertig abgearbeitet
    }
    (*pPLCF)++;
    long nPo = SVBT16ToShort( (unsigned char *)pPage );
    nPo <<= 9;                                          // shift als LONG

    long nAktFkpFilePos = pFkp ? pFkp->GetFilePos() : -1;
    if( nAktFkpFilePos != nPo )
    {
        DELETEZ( pFkp );
        pFkp = new WW8Fkp( GetVersion(), pFKPStrm, pDataStrm, nPo,
                            pFkpSizeTab[ ePLCF ],
                            ePLCF, nStartFc );
    }
    else // khz test1 //
    {
        pFkp->SetIdx( 0 );
        if( nStartFc >= 0 )
            pFkp->SeekPos( nStartFc );
    }

    nStartFc = -1;                                  // Nur das erste Mal
    return TRUE;
}

WW8PLCFx_Fc_FKP::WW8PLCFx_Fc_FKP(SvStream* pSt, SvStream* pTblSt,
                                 SvStream* pDataSt, WW8Fib& rFib, ePLCFT ePl,
                                 WW8_FC nStartFcL,
                                 WW8PLCFx_PCDAttrs* pPLCFx_PCDAttrs_)
    : WW8PLCFx( rFib.nVersion, TRUE ),
    pFKPStrm( pSt ),
    pDataStrm( pDataSt ),
    ePLCF( ePl ),
    pFkp( 0 ),
    nStartFc( nStartFcL ),
    pPCDAttrs( pPLCFx_PCDAttrs_)
{
    long nLenStruct = (8 > rFib.nVersion) ? 2 : 4;
    if( ePl == CHP )
        pPLCF = new WW8PLCF( pTblSt, rFib.fcPlcfbteChpx,
                        rFib.lcbPlcfbteChpx,
                        nLenStruct,
                        nStartFc,
                        rFib.pnChpFirst,
                        rFib.cpnBteChp );
    else
        pPLCF = new WW8PLCF( pTblSt, rFib.fcPlcfbtePapx,
                        rFib.lcbPlcfbtePapx,
                        nLenStruct,
                        nStartFc,
                        rFib.pnPapFirst,
                        rFib.cpnBtePap );
}

WW8PLCFx_Fc_FKP::~WW8PLCFx_Fc_FKP()
{
    delete pFkp;
    delete pPLCF;
}


ULONG WW8PLCFx_Fc_FKP::GetIdx() const
{
    ULONG u = pPLCF->GetIdx() << 8;
    if( pFkp )
        u |= pFkp->GetIdx();
    return u;
}

void WW8PLCFx_Fc_FKP::SetIdx( ULONG nIdx )
{
    if( !( nIdx & 0xffffff00L ) )
    {
        pPLCF->SetIdx( nIdx >> 8 );
        DELETEZ( pFkp );

    }
    else
    {                                   // Es gab einen Fkp
                                        // Lese PLCF um 1 Pos zurueck, um
        pPLCF->SetIdx( ( nIdx >> 8 ) - 1 );  // die Adresse des Fkp wiederzubekommen
        if ( NewFkp() )                     // und lese Fkp wieder ein
            pFkp->SetIdx( nIdx & 0xff );    // Dann stelle Fkp-Pos wieder ein
    }
}


BOOL WW8PLCFx_Fc_FKP::SeekPos( WW8_FC nFcPos )
{
    // StartPos for next Where()
    nStartFc = nFcPos;

    // find StartPos for next pPLCF->Get()
    BOOL bRet = pPLCF->SeekPos( nFcPos );

    // make FKP invalid?
    long nPLCFStart, nPLCFEnd;
    void* pPage;
    if( pFkp && pPLCF->Get( nPLCFStart, nPLCFEnd, pPage ) )
    {
        long nPo = SVBT16ToShort( (unsigned char *)pPage );
        nPo <<= 9;                                          // shift als LONG
        if( nPo != pFkp->GetFilePos() )
            DELETEZ( pFkp );
        else
            pFkp->SeekPos( nFcPos );
    }
    return bRet;
}

WW8_FC WW8PLCFx_Fc_FKP::Where()
{
//  if( bDontModify )
//      return pFkp ? pFkp->Where() : 0;

    if( !pFkp )
    {
        if( !NewFkp() )
            return LONG_MAX;
    }
    WW8_FC nP = pFkp->Where();
    if( nP != LONG_MAX )
        return nP;

    DELETEZ( pFkp );                    // FKP beendet -> hole neuen
    return Where();                     // am einfachsten rekursiv
}



BYTE* WW8PLCFx_Fc_FKP::GetSprms( WW8_FC& rStart, WW8_FC& rEnd, long& rLen )
{
    rLen = 0;                               // Default
    rStart = rEnd = LONG_MAX;

    if( !pFkp ){                            // Fkp nicht da ?
        if( !NewFkp() )
            return 0;
    }

    short nLen;
    BYTE* pPos = pFkp->Get( rStart, rEnd, nLen );
    if( rStart == LONG_MAX )
    {               // nicht gefunden
        return 0;
    }
    rLen = nLen;
    return pPos;
}



WW8PLCFx& WW8PLCFx_Fc_FKP::operator ++( int )
{
    if( !pFkp ){
//      ASSERT( FALSE, "pFkp fehlt");
//      return *this;
        if( !NewFkp() )
            return *this;
    }

    (*pFkp)++;
    if( pFkp->Where() == LONG_MAX ){
        NewFkp();
    }
    return *this;
}

USHORT WW8PLCFx_Fc_FKP::GetIstd() const
{
    return (pFkp) ? pFkp->GetIstd() : 0xffff;
}


void WW8PLCFx_Fc_FKP::GetPCDSprms( WW8PLCFxDesc& rDesc )
{
    rDesc.pMemPos   = 0;
    rDesc.nSprmsLen = 0;
    if( pPCDAttrs )
    {
        if( !pFkp )
        {
            DBG_WARNING( "+Problem: GetPCDSprms: NewFkp noetig ( kein const moeglich )" );
            if( !NewFkp() )
                return;
        }

        long nFcPos, nEnd;              // FCs
        short nLen;
        pFkp->Get( nFcPos, nEnd, nLen );

        WW8PLCFpcd_Iter* pIter = pPCDAttrs->GetIter();

    /**************************************/
        ULONG nSaveIdx = pIter->GetIdx();
        for( int nLoop = 0; 2 > nLoop; ++nLoop )
        {
            long nCpStart, nCpEnd;
            void* pData;

            if( pIter->Get( nCpStart, nCpEnd, pData ) )
            {
                BOOL bIsUnicode = FALSE;
                INT32 nFcStart  = SVBT32ToLong( ((WW8_PCD*)pData)->fc );
                if( 7 < GetVersion() )
                    nFcStart = WW8PLCFx_PCD::TransformPieceAddress( nFcStart, bIsUnicode );

                INT32 nLen = (nCpEnd - nCpStart) * (bIsUnicode ? 2 : 1);

                if( nFcPos >= nFcStart && nFcPos < nFcStart + nLen )
                {
                    // gefunden
                    pPCDAttrs->GetSprms( &rDesc );
                    break;
                }
            }
            if( !nLoop )
            {
                ULONG nFoundIdx = pIter->FindIdx( nFcPos );
                if( nFoundIdx > pIter->GetIMax())
                {
                    break;
                }
                pIter->SetIdx( nFoundIdx );
            }
        }
        pIter->SetIdx( nSaveIdx );
    /**************************************/
    }
}


BYTE* WW8PLCFx_Fc_FKP::HasSprm( USHORT nId )
{                                       // const waere schoener, aber dafuer
                                        // muesste NewFkp() ersetzt werden
                                        // oder wegfallen
    if( !pFkp )
    {
        DBG_WARNING( "+Motz: HasSprm: NewFkp noetig ( kein const moeglich )" );
                // Passiert bei BugDoc 31722
        if( !NewFkp() )
            return 0;
    }

    BYTE* pRes = pFkp->HasSprm( nId );

    if( !pRes )
    {
        WW8PLCFxDesc aDesc;
        GetPCDSprms( aDesc );

        if( aDesc.pMemPos )
        {
            pRes = WW8FindSprm( nId, aDesc.nSprmsLen,
                                aDesc.pMemPos, pFkp->GetVersion() );
        }
    }
    return pRes;
}


ULONG WW8PLCFx_Fc_FKP::GetParaHeight() const
{
    if( !pFkp ){
        ASSERT( !this, "GetParaHeight: pFkp nicht da" );
        return 0;
    }
    return pFkp->GetParaHeight();
}

//-----------------------------------------

WW8PLCFx_Cp_FKP::WW8PLCFx_Cp_FKP( SvStream* pSt, SvStream* pTblSt, SvStream* pDataSt,
                                    const WW8ScannerBase& rBase,
                                    ePLCFT ePl )
    : WW8PLCFx_Fc_FKP( pSt, pTblSt, pDataSt, *rBase.pWw8Fib, ePl,
                        rBase.WW8Cp2Fc( 0 ),
                        rBase.pPLCFx_PCDAttrs ),
    rSBase( rBase ),
//  nFcMin( rBase.pWrFib.fcMin ),
    nAttrStart( -1 ), nAttrEnd( -1 ),
    bLineEnd( FALSE ),
    bComplex( (7 < rBase.pWw8Fib->nVersion) || (0 != rBase.pWw8Fib->fComplex) )
{
    ResetAttrStartEnd();
    pPcd = rSBase.pPiecePLCF ? new WW8PLCFx_PCD(
                                rBase.pWw8Fib->nVersion, rBase.pPiecePLCF, 0,
                                ((6 == GetVersion()) || (7 == GetVersion())) )
                           : 0;
}

WW8PLCFx_Cp_FKP::~WW8PLCFx_Cp_FKP()
{
    delete pPcd;
}

void WW8PLCFx_Cp_FKP::ResetAttrStartEnd()
{
  nAttrStart = -1;
  nAttrEnd   = -1;
  bLineEnd   = FALSE;
}

ULONG WW8PLCFx_Cp_FKP::GetPCDIMax() const
{
    return pPcd ? pPcd->GetIMax() : 0;
}

ULONG WW8PLCFx_Cp_FKP::GetPCDIdx() const
{
    return pPcd ? pPcd->GetIdx() : 0;
}

void WW8PLCFx_Cp_FKP::SetPCDIdx( ULONG nIdx )
{
    if( pPcd )
        pPcd->SetIdx( nIdx );
}

BOOL WW8PLCFx_Cp_FKP::SeekPos( WW8_CP nCpPos )
{
/*
    WW8_FC nTargetFc = WW8Cp2Fc( nCpPos, nFcMin );
    return (LONG_MAX != nTargetFc)
        && WW8PLCFx_Fc_FKP::SeekPos( nTargetFc );
*/
    if( pPcd )  // Complex
    {
        if( !pPcd->SeekPos( nCpPos ) )  // Piece setzen
            return FALSE;
        return WW8PLCFx_Fc_FKP::SeekPos( pPcd->AktPieceStartCp2Fc( nCpPos ) );
    }
                                    // KEINE Piece-Table !!!
    return WW8PLCFx_Fc_FKP::SeekPos( rSBase.WW8Cp2Fc( nCpPos /*,TRUE*/ ) );
}

WW8_CP WW8PLCFx_Cp_FKP::Where()
{
    WW8_FC nFc = WW8PLCFx_Fc_FKP::Where();
/*  if( LONG_MAX == nFc )
        return LONG_MAX;*/
    if( pPcd )
        return pPcd->AktPieceStartFc2Cp( nFc ); // Piece ermitteln
    return rSBase.WW8Fc2Cp( nFc/*,TRUE*/ );     // KEINE Piece-Table !!!
}

void WW8PLCFx_Cp_FKP::GetSprms( WW8PLCFxDesc* p )
{
    p->pMemPos = WW8PLCFx_Fc_FKP::GetSprms( p->nStartPos, p->nEndPos, p->nSprmsLen );

#ifdef DEBUG
    if( ePLCF == PAP ) // um hier einen Brechpunkt zu setzen
        ePLCF = PAP;
#endif

    if( pPcd )  // Piece-Table vorhanden !!!
    {
        if(    (nAttrStart >  nAttrEnd)
            || (nAttrStart ==       -1) )  // Init ( noch kein ++ gerufen )
        {
            eCutT eC = pPcd->AktPieceFc2Cp( p->nStartPos, p->nEndPos );
            p->bRealLineEnd = !( eC & CUT_END ) && ePLCF == PAP;
        }
        else
        {
            p->nStartPos = nAttrStart;
            p->nEndPos = nAttrEnd;
            p->bRealLineEnd = bLineEnd;
        }
    }
    else        // KEINE Piece-Table !!!
    {
        p->nStartPos = rSBase.WW8Fc2Cp( p->nStartPos/*,TRUE*/ );
        p->nEndPos   = rSBase.WW8Fc2Cp( p->nEndPos/*,TRUE*/ );
        p->bRealLineEnd = ePLCF == PAP;
    }
}


// WW8PLCF_Cp_Fkp::SearchParaEnd kann einfacher durchgefuehrt werden, wenn das
// Flag "Zeilenende im Piece" in der Piecetable ausgewertet wird.
void WW8PLCFx_Cp_FKP::SearchParaEnd( long nOldEndCp )
{
    if( !bComplex )
    {
        ASSERT( !this, "SearchParaEnd fuer Non-Complex File gerufen" );
        return;
    }
    if( ePLCF != PAP )
    {
        ASSERT( !this, "SearchParaEnd fuer Non-PAP gerufen" );
        return;
    }
    long nFkpLen;                           // Fkp-Eintrag
    long nPcdStart, nPcdEnd;                // Piece-Grenzen

    WW8PLCFx_Fc_FKP::GetSprms( nAttrStart, nAttrEnd, nFkpLen ); // Fkp-Eintrag holen
/*  if( (LONG_MAX == nAttrStart) || (LONG_MAX == nAttrEnd) )
    {
        return;
    }*/
    eCutT eC = pPcd->AktPieceFc2Cp( nAttrStart, nAttrEnd );
    if( eC == CUT_NONE )            // neuer Eintrag ganz im akt. Piece
    {
        ASSERT( !this, "Nanu?" );
        return;                             // und fertig
    }
    WW8PLCFpcd_Iter* pIter = pPcd->GetPLCFIter();
    void* pData;
    do{
        (*pPcd)++;                                      //  naechstes Piece


        if( GetPCDIdx() >= GetPCDIMax() )
        {
            nAttrStart = nAttrEnd = LONG_MAX;   // kein Piece mehr vorhanden
            return;
        }


        if( !pIter->Get( nPcdStart, nPcdEnd, pData ) ){ // Piece-Grenzen holen
#ifdef DEBUG
            WW8_PCD1* p = (WW8_PCD1*)pData;
            p = p;
#endif
            nAttrStart = nAttrEnd = LONG_MAX;   // kein Piece mehr vorhanden
            return;
        }
#ifdef DEBUG
        WW8_PCD1* p = (WW8_PCD1*)pData;
        p = p;
#endif

    }while( SVBT8ToByte( ( (WW8_PCD*)pData )->aBits1 ) & 0x1 );
                                                // bis NL in Piece

    long nPos = rSBase.WW8Cp2Fc( nPcdStart);
    if( !WW8PLCFx_Fc_FKP::SeekPos( nPos ) )
    {
//      ASSERT( !this, " Can't seek to Piece Start" );
//      nAttrStart = nAttrEnd = LONG_MAX;   // kein Piece mehr vorhanden
        return;
    }
    WW8PLCFx_Fc_FKP::GetSprms( nAttrStart, nAttrEnd, nFkpLen ); // Fkp-Eintrag holen
    eC = pPcd->AktPieceFc2Cp(  nAttrStart, nAttrEnd );  // wird in CPs gebraucht

    nAttrStart = nOldEndCp;     // Aufziehen ueber ganzen Absatz, unabhaengig
                                // davon, wieviel Pieces das sind
}

WW8PLCFx& WW8PLCFx_Cp_FKP::operator ++( int )
{
    WW8PLCFx_Fc_FKP::operator ++( 0 );
    if(    !bComplex
        || !pPcd )                              // !pPcd: Notbremse
        return *this;

    if( GetPCDIdx() >= GetPCDIMax() )           // End of PLCF
    {
        nAttrStart = nAttrEnd = LONG_MAX;
        return *this;
    }

// ohoho
    long nFkpLen;                               // Fkp-Eintrag
    long nPcdStart, nPcdEnd, nPcdLen;           // Piece-Grenzen
    long nOldEndCp = nAttrEnd;

    WW8PLCFx_Fc_FKP::GetSprms( nAttrStart, nAttrEnd, nFkpLen ); // Fkp-Eintrag holen
    eCutT eC = pPcd->AktPieceFc2Cp( nAttrStart, nAttrEnd );
    bLineEnd = !( eC & CUT_END ) && ePLCF == PAP;
    if( !( eC & CUT_START ) ){                  // neuer Eintrag faengt im
                                                // akt. Piece an
#ifdef DEBUG
        if( ePLCF == PAP )
            ePLCF = PAP;                    // um hier einen Brechpunkt zu setzen
        if( ePLCF == CHP )
            ePLCF = CHP;                    // um hier einen Brechpunkt zu setzen
#endif
        return *this;               // und fertig
    }
                                    // naechster Eintrag im naechsten Piece

    if( ePLCF == PAP )
    {
        SearchParaEnd( nOldEndCp );
        bLineEnd = TRUE;
        return *this;
    }

    //  naechstes Piece
    bLineEnd = FALSE;
    (*pPcd)++;
    pPcd->GetNoSprms( nPcdStart, nPcdEnd, nPcdLen );
    // Piece-Grenzen holen
    if( !WW8PLCFx_Fc_FKP::SeekPos( rSBase.WW8Cp2Fc( nPcdStart ) ) )
    {
        ASSERT( nPcdStart == LONG_MAX, "Chp-SeekPos ging schief" );
        nAttrStart = nAttrEnd = LONG_MAX;
        goto Ret;
    }
    // Fkp-Eintrag holen
    WW8PLCFx_Fc_FKP::GetSprms( nAttrStart, nAttrEnd, nFkpLen );
    eC  = pPcd->AktPieceFc2Cp( nAttrStart, nAttrEnd );
Ret:
    return *this;
}

//-----------------------------------------
//-----------------------------------------

WW8PLCFx_SEPX::WW8PLCFx_SEPX( SvStream* pSt, SvStream* pTblSt, WW8Fib& rFib, WW8_CP nStartCp )
: pStrm( pSt ), WW8PLCFx( rFib.nVersion, TRUE ), nArrMax( 256 ), nSprmSiz( 0 )
{
    pPLCF =   rFib.lcbPlcfsed
            ? new WW8PLCF( pTblSt, rFib.fcPlcfsed,
                            rFib.lcbPlcfsed,
                            12, nStartCp )
            : 0;

    pSprms = new BYTE[nArrMax];     // maximale Laenge
}

WW8PLCFx_SEPX::~WW8PLCFx_SEPX()
{
    delete pPLCF;
    delete pSprms;
}

ULONG WW8PLCFx_SEPX::GetIdx() const
{
    return pPLCF ? pPLCF->GetIdx() : 0;
}

void WW8PLCFx_SEPX::SetIdx( ULONG nIdx )
{
    if( pPLCF ) pPLCF->SetIdx( nIdx );
}

BOOL WW8PLCFx_SEPX::SeekPos( WW8_CP nCpPos )
{
    return pPLCF ? pPLCF->SeekPos( nCpPos ) : 0;
}

WW8_CP WW8PLCFx_SEPX::Where()
{
    return pPLCF ? pPLCF->Where() : 0;
}

void WW8PLCFx_SEPX::GetSprms( WW8PLCFxDesc* p )
{
    if( !pPLCF ) return;

    void* pData;

    p->bRealLineEnd = FALSE;
    if (!pPLCF->Get( p->nStartPos, p->nEndPos, pData ))
    {
        p->nStartPos = p->nEndPos = LONG_MAX;       // PLCF fertig abgearbeitet
        p->pMemPos = 0;
    }
    else
    {
        long nPo =  SVBT32ToLong( (BYTE*)pData+2 );
        if ( nPo == 0xffffffffL )
        {
            p->nStartPos = p->nEndPos = LONG_MAX;       // Sepx empty
            p->pMemPos = 0;
        }
        else
        {
            pStrm->Seek( nPo );
            *pStrm >> nSprmSiz; // read len

            if( nSprmSiz > nArrMax )
            {               // passt nicht
                delete( pSprms );
                nArrMax = nSprmSiz;                 // Hole mehr Speicher
                pSprms = new BYTE[nArrMax];
            }
            pStrm->Read( pSprms, nSprmSiz );        // read Sprms

            p->nSprmsLen = nSprmSiz;
            p->pMemPos = pSprms;                    // return Position
        }
    }
}


WW8PLCFx& WW8PLCFx_SEPX::operator ++( int )
{
    if( pPLCF )
        (*pPLCF)++;
    return *this;
}


BYTE* WW8PLCFx_SEPX::HasSprm( USHORT nId ) const
{
    return pPLCF ? WW8FindSprm( nId, nSprmSiz, pSprms, GetVersion() )
                 : 0;
}


BYTE* WW8PLCFx_SEPX::HasSprm( USHORT nId, BYTE*  pOtherSprms,
                                          long   nOtherSprmSiz ) const
{
    return pPLCF ? WW8FindSprm( nId, nOtherSprmSiz, pOtherSprms, GetVersion() )
                 : 0;
}



BOOL WW8PLCFx_SEPX::Find4Sprms(USHORT nId1, USHORT nId2, USHORT nId3, USHORT nId4,
                               BYTE*& p1,   BYTE*& p2,   BYTE*& p3,   BYTE*& p4 ) const
{
    if( !pPLCF ) return 0;

    BOOL bFound = FALSE;
    p1 = 0;
    p2 = 0;
    p3 = 0;
    p4 = 0;

    BYTE* pSp = pSprms;
    short i;
    BYTE nDelta = ( 8 > GetVersion()) ? 0 : 1;
    for( i=0; i+1+nDelta < nSprmSiz;    )
    {
        // Sprm gefunden?
        USHORT nAktId = WW8GetSprmId( GetVersion(), pSp );
        BOOL bOk = TRUE;
        if( nAktId  == nId1 )
            p1 = pSp
                + 1 + nDelta
                + WW8SprmDataOfs( nId1 );
        else
        if( nAktId  == nId2 )
            p2 = pSp
                + 1 + nDelta
                + WW8SprmDataOfs( nId2 );
        else
        if( nAktId  == nId3 )
            p3 = pSp
                + 1 + nDelta
                + WW8SprmDataOfs( nId3 );
        else
        if( nAktId  == nId4 )
            p4 = pSp
                + 1 + nDelta
                + WW8SprmDataOfs( nId4 );
        else
            bOk = FALSE;
        bFound |= bOk;
        // erhoehe Zeiger, so dass er auf naechsten Sprm zeigt
        short x = WW8GetSprmSizeBrutto( GetVersion(), pSp, &nAktId  );
        i += x;
        pSp += x;
    }
    return bFound;
}

BYTE* WW8PLCFx_SEPX::HasSprm( USHORT nId, BYTE n2nd ) const
{
    if( !pPLCF ) return 0;

    BYTE* pSp = pSprms;
    short i;
    BYTE nDelta = ( 8 > GetVersion()) ? 0 : 1;

    for( i=0; i+1+nDelta < nSprmSiz;    )
    {
        // Sprm gefunden?
        USHORT nAktId = WW8GetSprmId( GetVersion(), pSp );
        if(    ( nAktId            == nId  )
                && ( pSp[ 1 + nDelta ] == n2nd ) )
            return pSp
                    + 1 + nDelta
                    + WW8SprmDataOfs( nId );
        // erhoehe Zeiger, so dass er auf naechsten Sprm zeigt
        short x = WW8GetSprmSizeBrutto( GetVersion(), pSp, &nAktId );
        i += x;
        pSp += x;
    }
    return 0;   // Sprm nicht gefunden
}

//-----------------------------------------
//-----------------------------------------

WW8PLCFx_SubDoc::WW8PLCFx_SubDoc( SvStream* pSt, BYTE nVersion,
                                    WW8_CP nStartCp,
                                    long nFcRef, long nLenRef,
                                    long nFcTxt, long nLenTxt,
                                    long nStruct )
    : WW8PLCFx( nVersion, FALSE ), pRef( 0 ), pTxt( 0 )
{
    if( nLenRef && nLenTxt )
    {
        pRef = new WW8PLCF( pSt, nFcRef, nLenRef, nStruct, nStartCp );
        pTxt = new WW8PLCF( pSt, nFcTxt, nLenTxt, 0, nStartCp );
    }
}

WW8PLCFx_SubDoc::~WW8PLCFx_SubDoc()
{
    delete pRef;
    delete pTxt;
}

ULONG WW8PLCFx_SubDoc::GetIdx() const
{
    if( pRef )
        return ( pRef->GetIdx() << 16
                 | pTxt->GetIdx() );        // Wahrscheinlich pTxt... nicht noetig
    return 0;
}

void WW8PLCFx_SubDoc::SetIdx( ULONG nIdx )
{
    if( pRef )
    {
        pRef->SetIdx( nIdx >> 16 );
        pTxt->SetIdx( nIdx & 0xffff );      // Wahrscheinlich pTxt... nicht noetig
    }
}

BOOL WW8PLCFx_SubDoc::SeekPos( WW8_CP nCpPos )
{
    return ( pRef ) ? pRef->SeekPos( nCpPos ) : FALSE;
}

WW8_CP WW8PLCFx_SubDoc::Where()
{
    return ( pRef ) ? pRef->Where() : LONG_MAX;
}

long WW8PLCFx_SubDoc::GetNoSprms( WW8_CP& rStart, long& rEnd, long& rLen )
{
    void* pData;
    long nSt, nE;
    rEnd = LONG_MAX;

    if ( !pRef )
    {
        rStart  = LONG_MAX;             // Es gibt keine Noten
        rLen = 0;
        return -1;
    }

    ULONG nNr = pRef->GetIdx();

    if (!pRef->Get( rStart, nE, pData ))
    {
        rStart = LONG_MAX;              // PLCF fertig abgearbeitet
        rLen = 0;
        return -1;
    }
    pTxt->SetIdx( nNr );

    if(!pTxt->Get( nSt, rLen, pData ))
    {
        rStart = LONG_MAX;              // PLCF fertig abgearbeitet
        rLen = 0;
        return -1;
    }

    rLen -= nSt;
    return nSt;
}

WW8PLCFx& WW8PLCFx_SubDoc::operator ++( int )
{
    if( pRef && pTxt )
    {
        (*pRef)++;
        (*pTxt)++;
    }
    return *this;
}


//-----------------------------------------
//          Felder
//-----------------------------------------

WW8PLCFx_FLD::WW8PLCFx_FLD( SvStream* pSt, WW8Fib& rMyFib,
                            short nType, WW8_CP nStartCp )
    : WW8PLCFx( rMyFib.nVersion, FALSE ), rFib( rMyFib ), pPLCF( 0 )
{

    long nFc, nLen;

    switch( nType )
    {
    case MAN_HDFT: nFc = rFib.fcPlcffldHdr;
                   nLen = rFib.lcbPlcffldHdr;
                   break;
    case MAN_FTN:  nFc = rFib.fcPlcffldFtn;
                   nLen = rFib.lcbPlcffldFtn;
                   break;
    case MAN_EDN:  nFc = rFib.fcPlcffldEdn;
                   nLen = rFib.lcbPlcffldEdn;
                   break;
    case MAN_AND:  nFc = rFib.fcPlcffldAtn;
                   nLen = rFib.lcbPlcffldAtn;
                   break;
    case MAN_TXBX: nFc = rFib.fcPlcffldTxbx;
                   nLen = rFib.lcbPlcffldTxbx;
                   break;
    case MAN_TXBX_HDFT:
                   nFc = rFib.fcPlcffldHdrTxbx;
                   nLen = rFib.lcbPlcffldHdrTxbx;
                   break;
    default:       nFc = rFib.fcPlcffldMom;
                   nLen = rFib.lcbPlcffldMom;
                   break;
    }

    if( nLen )
        pPLCF = new WW8PLCFspecial( pSt, nFc, nLen, 2 );
}

WW8PLCFx_FLD::~WW8PLCFx_FLD()
{
    delete pPLCF;
}

ULONG WW8PLCFx_FLD::GetIdx() const
{
    return ( pPLCF ) ? pPLCF->GetIdx() : 0;
}

void WW8PLCFx_FLD::SetIdx( ULONG nIdx )
{
    if( pPLCF )
        pPLCF->SetIdx( nIdx );
}

BOOL WW8PLCFx_FLD::SeekPos( WW8_CP nCpPos )
{
    return ( pPLCF ) ? pPLCF->SeekPosExact( nCpPos ) : FALSE;
}

WW8_CP WW8PLCFx_FLD::Where()
{
    return ( pPLCF ) ? pPLCF->Where() : LONG_MAX;
}

long WW8PLCFx_FLD::GetNoSprms( WW8_CP& rStart, long& rEnd, long& rLen )
{
    void* pData;
    ULONG nIdx;

    rLen = 0;
    rEnd = LONG_MAX;                            // Es gibt keine Ende

    if ( !pPLCF ){
        rStart = LONG_MAX;                      // Es gibt keine Felder
        return -1;
    }

    if (!pPLCF->Get( rStart, pData )){
        rStart = LONG_MAX;                      // PLCF fertig abgearbeitet
        return -1;
    }

    nIdx = pPLCF->GetIdx();

    return (long)nIdx;
}

WW8PLCFx& WW8PLCFx_FLD::operator ++( int )
{
    WW8SkipField( *pPLCF );                     // gehe zum naechsten Feld
    return *this;
}

BOOL WW8PLCFx_FLD::GetPara( long nIdx, WW8FieldDesc& rF )
{
    ASSERT( pPLCF, "Aufruf ohne Feld PLCFspecial" );
    if( !pPLCF )
        return FALSE;

    ULONG n = pPLCF->GetIdx();
    pPLCF->SetIdx( (ULONG)nIdx );

    BOOL bOk = WW8GetFieldPara( rFib.nVersion, *pPLCF, rF );
    pPLCF->SetIdx( n );
    return bOk;
}

//-----------------------------------------
//      class WW8PLCF_Book
//-----------------------------------------

/*  to be optimized like this:    */
void WW8ReadSTTBF(  BOOL bVer8, SvStream& rStrm,
                    UINT32 nStart, INT32 nLen, USHORT nSkip,
                    rtl_TextEncoding eCS,
                    SvStrings &rArray, SvStrings* pExtraArray  )
{
    ULONG nOldPos = rStrm.Tell();
    rStrm.Seek( nStart );

    String* pWork;

    UINT16 nLen2;
    rStrm >> nLen2; // bVer67: total length of structure
                    // bVer8 : count of strings

    if( bVer8 )
    {
        UINT16 nStrings;
        BOOL bUnicode = (0xFFFF == nLen2);
        if( bUnicode )
            rStrm >> nStrings;
        else
            nStrings = nLen2;

        UINT16 nExtraLen;
        rStrm >> nExtraLen;

        for( USHORT i=0; i < nStrings; i++ )
        {
            if( bUnicode )
                pWork = new String( WW8Read_xstz( rStrm, 0, FALSE ));
            else
            {
                BYTE nBChar;
                rStrm >> nBChar;
                ByteString aTmp;
                BYTE nWasRead = rStrm.Read( aTmp.AllocBuffer( nBChar ), nBChar );
                if( nWasRead != nBChar )
                    aTmp.ReleaseBufferAccess( nWasRead );
                pWork = new String( aTmp, eCS );
            }

            rArray.Insert( pWork, rArray.Count() );

            // Skip the extra data
            if( nExtraLen )
            {
                if( pExtraArray )
                {
                    ByteString aTmp;
                    BYTE nWasRead = rStrm.Read( aTmp.AllocBuffer( nExtraLen ), nExtraLen );
                    if( nWasRead != nExtraLen )
                        aTmp.ReleaseBufferAccess( nWasRead );
                    pWork = new String( aTmp, eCS );
                    pExtraArray->Insert( pWork, pExtraArray->Count() );
                }
                else
                    rStrm.SeekRel( nExtraLen );
            }
        }
    }
    else
    {
        BYTE nBChar;
        if( nLen2 != nLen )
        {
            ASSERT( nLen2 == nLen, "Fib lenght and read length are different" );
            nLen2 = nLen;
        }
        UINT16 nRead = 0;
        for( nLen2 -= 2; nRead < nLen2;  )
        {
            rStrm >> nBChar; ++nRead;
            if( nBChar )
            {
                ByteString aTmp;
                BYTE nWasRead = rStrm.Read( aTmp.AllocBuffer( nBChar ), nBChar );
                if( nWasRead != nBChar )
                    aTmp.ReleaseBufferAccess( nWasRead );
                pWork = new String( aTmp, eCS );
                nRead += nWasRead;
            }
            else
                pWork = new String;
            rArray.Insert( pWork, rArray.Count() );
        }
    }
    rStrm.Seek( nOldPos );
}


WW8PLCFx_Book::WW8PLCFx_Book( SvStream* pSt, SvStream* pTblSt, WW8Fib& rFib, WW8_CP nStartCp )
: WW8PLCFx( rFib.nVersion, FALSE ), nIsEnd( 0 ),  pStatus( 0 )
{
    if( !rFib.fcPlcfbkf || !rFib.lcbPlcfbkf
        || !rFib.fcPlcfbkl || !rFib.lcbPlcfbkl
        || !rFib.fcSttbfbkmk || !rFib.lcbSttbfbkmk )
    {
        pBook[0] = pBook[1] = 0;
        nIMax = 0;
    }
    else
    {
        pBook[0] = new WW8PLCFspecial( pTblSt, rFib.fcPlcfbkf,
                               rFib.lcbPlcfbkf, 4 );

        pBook[1] = new WW8PLCFspecial( pTblSt, rFib.fcPlcfbkl,
                             rFib.lcbPlcfbkl, 0, -1, TRUE );

        rtl_TextEncoding eStructCharSet = 0x0100 == rFib.chseTables
                                            ? RTL_TEXTENCODING_APPLE_ROMAN
                                            : /*rtl_getTextEncodingFromWindowsCharset*/(
                                                rFib.chseTables );

        WW8ReadSTTBF( (7 < rFib.nVersion), *pTblSt,
                        rFib.fcSttbfbkmk, rFib.lcbSttbfbkmk, 0,
                        eStructCharSet, aBookNames );
        nIMax = aBookNames.Count();

        if( pBook[0]->GetIMax() < nIMax )   // Anzahl der Bookmarks
            nIMax = pBook[0]->GetIMax();
        if( pBook[1]->GetIMax() < nIMax )
            nIMax = pBook[1]->GetIMax();
        pStatus = new eBookStatus[ nIMax ];
        memset( pStatus, 0, nIMax * sizeof( eBookStatus ) );
    }
}

WW8PLCFx_Book::~WW8PLCFx_Book()
{
    delete[] ( pStatus );
    aBookNames.DeleteAndDestroy( 0, aBookNames.Count());
    delete( pBook[1] );
    delete( pBook[0] );
}

ULONG WW8PLCFx_Book::GetIdx() const
{
    return nIMax ? pBook[0]->GetIdx() : 0;
}

void WW8PLCFx_Book::SetIdx( ULONG nI )
{
    if( nIMax )
        pBook[0]->SetIdx( nI );
}

ULONG WW8PLCFx_Book::GetIdx2() const
{
    return ( nIMax )
            ? ( pBook[1]->GetIdx() | ( ( nIsEnd ) ? 0x80000000 : 0 ) )
            : 0;
}

void WW8PLCFx_Book::SetIdx2( ULONG nI )
{
    if( nIMax )
    {
        pBook[1]->SetIdx( nI & 0x7fffffff );
        nIsEnd = (USHORT)( ( nI >> 31 ) & 1 );  // 0 oder 1
    }
}

BOOL WW8PLCFx_Book::SeekPos( WW8_CP nCpPos )
{
    if( !pBook[0] )
        return FALSE;
    BOOL bOk = pBook[0]->SeekPosExact( nCpPos );
    bOk &= pBook[1]->SeekPosExact( nCpPos );
    nIsEnd = 0;

    return bOk;
}

WW8_CP WW8PLCFx_Book::Where()
{
    return pBook[nIsEnd]->Where();
}

long WW8PLCFx_Book::GetNoSprms( long& rStart, long& rEnd, long& rLen )
{
    void* pData;
    rEnd = LONG_MAX;
    rLen = 0;

    if ( !pBook[0] || !pBook[1] || !nIMax
         || (long)(pBook[nIsEnd]->GetIdx()) >= nIMax ){
        rStart = rEnd = LONG_MAX;
        return -1;
    }

    pBook[nIsEnd]->Get( rStart, pData );    // Pos. abfragen

    return pBook[nIsEnd]->GetIdx();
}

// Der Operator ++ hat eine Tuecke: Wenn 2 Bookmarks aneinandergrenzen, dann
// sollte erst das Ende des ersten und dann der Anfang des 2. erreicht werden.
// Liegen jedoch 2 Bookmarks der Laenge 0 aufeinander, *muss* von jedem Bookmark
// erst der Anfang und dann das Ende gefunden werden.
// Der Fall: ][
//            [...]
//           ][
// ist noch nicht geloest, dabei muesste ich in den Anfangs- und Endindices
// vor- und zurueckspringen, wobei ein weiterer Index oder ein Bitfeld
// oder etwas aehnliches zum Merken der bereits abgearbeiteten Bookmarks
// noetig wird.
WW8PLCFx& WW8PLCFx_Book::operator ++( int )
{
    if( pBook[0] && pBook[1] && nIMax )
    {
        (*pBook[nIsEnd])++;

        register ULONG l0 = pBook[0]->Where();
        register ULONG l1 = pBook[1]->Where();
        if( l0 < l1 )
            nIsEnd = 0;
        else if( l1 < l0 )
            nIsEnd = 1;
        else
            nIsEnd = ( nIsEnd ) ? 0 : 1;

//      nIsEnd = ( pBook[1]->Where() <= pBook[0]->Where() ) ? 1 : 0;
    }
    return *this;
}

long WW8PLCFx_Book::GetLen() const
{
    if( nIsEnd ){
        ASSERT( !this, "Falscher Aufruf (1) von PLCF_Book::GetLen()" );
        return 0;
    }
    void * p;
    WW8_CP nStartPos;
    if( !pBook[0]->Get( nStartPos, p ) ){
        ASSERT( !this, "Falscher Aufruf (2) von PLCF_Book::GetLen()" );
        return 0;
    }
    USHORT nEndIdx = SVBT16ToShort( *((SVBT16*)p) );
//  USHORT nEndIdx = *(USHORT*)p;
    return pBook[1]->GetPos( nEndIdx ) - nStartPos;
}

// IgnoreBook ist dafuer da, bei Feldern mit implizitem WW-Bookmark
// die Bookmarks zwischen Anfang und Ende des Feldes zu ignorieren,
// die den angegebenen Namen tragen.
BOOL WW8PLCFx_Book::SetStatus( WW8_CP nStartRegion, WW8_CP nEndRegion,
                               const String& rName,
                               eBookStatus eStat )
{
    ASSERT( nStartRegion < LONG_MAX && nEndRegion < LONG_MAX,
            "IgnoreBook mit falschen Parametern" );

    if( !pBook[0] || !pBook[1] )
        return FALSE;

    BOOL bFound = FALSE;
    long i = 0;
    WW8_CP nStartAkt, nEndAkt;
    do{
        void* p;
        USHORT nEndIdx;

        if( pBook[0]->GetData( i, nStartAkt, p ) && p ){
            nEndIdx = SVBT16ToShort( *((SVBT16*)p) );
        }
        else
        {
            ASSERT( !this, "Bookmark-EndIdx nicht lesbar" );
            nEndIdx = (USHORT)i;
        }
        nEndAkt = pBook[1]->GetPos( nEndIdx );
        if(    (nStartAkt >= nStartRegion)
            && (nEndAkt   <= nEndRegion)
            && (rName.Equals( *aBookNames[ i ] )) )
        {
            pStatus[nEndIdx] = (eBookStatus)( pStatus[nEndIdx] | eStat );
            bFound = TRUE;
        }
        i++;
    }
    while( nStartAkt <= nEndRegion && i < pBook[0]->GetIMax() );
    return bFound;
}

eBookStatus WW8PLCFx_Book::GetStatus() const
{
    if( !pStatus )
        return BOOK_NORMAL;
    long nEndIdx = GetHandle();
    return ( nEndIdx < nIMax ) ? pStatus[nEndIdx] : BOOK_NORMAL;
}

long WW8PLCFx_Book::GetHandle() const
{
    if( !pBook[0] || !pBook[1] )
        return LONG_MAX;

    if( nIsEnd ){
        return pBook[1]->GetIdx();
    }else{
//  BOOL     Get( long& rStart, void*& rpValue );
        const void* p = pBook[0]->GetData( pBook[0]->GetIdx() );
        if( p )
        {
            return SVBT16ToShort( *((SVBT16*)p) );
//          return *((USHORT*)p);
        }
        else
            return LONG_MAX;
    }
}

//-----------------------------------------
//          WW8PLCFMan
//-----------------------------------------

#ifndef DUMP

// Am Ende eines Absatzes reichen bei WW6 die Attribute bis hinter das <CR>.
// Das wird fuer die Verwendung mit dem SW um 1 Zeichen zurueckgesetzt, wenn
// dadurch kein AErger zu erwarten ist.
void WW8PLCFMan::AdjustEnds( WW8PLCFxDesc& rDesc )
{
    if ( (&rDesc == pPap) && rDesc.bRealLineEnd )
    {
        if ( pPap->nEndPos != LONG_MAX )    // Para adjust
        {
            nLineEnd = pPap->nEndPos;       // nLineEnd zeigt *hinter* das <CR>
            pPap->nEndPos--;                // Absatzende um 1 Zeichen verkuerzen

            if( pChp->nEndPos == nLineEnd ) // gibt es bereits ein CharAttr-Ende,
                                            // das auf das jetzige Absatzende zeigt ?
                pChp->nEndPos--;            // ... dann auch um 1 Zeichen verkuerzen

            if( pSep->nEndPos == nLineEnd ) // gibt es bereits ein Sep-Ende,
                                            // das auf das jetzige Absatzende zeigt ?
                pSep->nEndPos--;            // ... dann auch um 1 Zeichen verkuerzen
        }
    }
    else
    if(    (&rDesc == pChp)
        || (&rDesc == pSep) )
    {                                       // Char Adjust oder Sep Adjust
        if(     (rDesc.nEndPos == nLineEnd) // Wenn Ende Char-Attr == Absatzende ...
             && (rDesc.nEndPos > rDesc.nStartPos) )
            rDesc.nEndPos--;                    // ... dann um 1 Zeichen verkuerzen
    }

    if( rDesc.nStartPos > rDesc.nEndPos ){  // allgemeiner Plausibilitaetstest

        //if( gfhdf

        ASSERT( !this, "+Anfang und Ende des WW86-Attributes stehen ueber Kreuz" );
        rDesc.nEndPos = rDesc.nStartPos;
    }
}

void WW8PLCFMan::GetNewSprms( WW8PLCFxDesc& rDesc )
{
    rDesc.pPLCFx->GetSprms( &rDesc );

    //if( rDesc.nStartPos <= rDesc.nEndPos

    ASSERT((LONG_MAX == rDesc.nStartPos) || (rDesc.nStartPos <= rDesc.nEndPos),
            "Attr-Anfang und -Ende ueber Kreuz" );

    if( rDesc.nStartPos != LONG_MAX ) rDesc.nStartPos -= rDesc.nCpOfs;
    if( rDesc.nEndPos   != LONG_MAX ) rDesc.nEndPos   -= rDesc.nCpOfs;

    rDesc.bFirstSprm = TRUE;

    AdjustEnds( rDesc );
}

void WW8PLCFMan::GetNewNoSprms( WW8PLCFxDesc& rDesc )
{
    rDesc.nCp2OrIdx =
        rDesc.pPLCFx->GetNoSprms(rDesc.nStartPos, rDesc.nEndPos,
                                                rDesc.nSprmsLen);

    ASSERT((LONG_MAX == rDesc.nStartPos) || (rDesc.nStartPos <= rDesc.nEndPos),
            "Attr-Anfang und -Ende ueber Kreuz" );

    if( rDesc.nStartPos != LONG_MAX ) rDesc.nStartPos -= rDesc.nCpOfs;
    if( rDesc.nEndPos   != LONG_MAX ) rDesc.nEndPos   -= rDesc.nCpOfs;
    rDesc.bFirstSprm = TRUE;
}

static USHORT GetId( BYTE nVersion, WW8PLCFxDesc* p )
{
    USHORT nId;

    if( p->nSprmsLen > 0 )
        nId = WW8GetSprmId( nVersion, p->pMemPos );
    else
        nId = 0;        // Id = 0 fuer leeres Attribut
    return nId;
}

WW8PLCFMan::WW8PLCFMan( WW8ScannerBase* pBase, short nType, long nStartCp )
{
    pWwFib      = pBase->pWw8Fib;
    pNoAttrScan = &pBase->nNoAttrScan;

    nLastWhereIdxCp = 0;
    memset( aD, 0, sizeof( aD ) );
    nLineEnd = LONG_MAX;
    nManType = nType;
    USHORT i;

    if( MAN_MAINTEXT == nType )
    {
        // Suchreihenfolge der Attribute
        nPLCF = MAN_ANZ_PLCF;
        pFld = &aD[0];
        pBkm = &aD[1];
        pEdn = &aD[2];
        pFtn = &aD[3];
        pAnd = &aD[4];
        pChp = &aD[5];
        pPap = &aD[6];
        pSep = &aD[7];
        pPcd = ( pBase->pPLCFx_PCD ) ? &aD[8] : 0;
        pPcdA = ( pBase->pPLCFx_PCDAttrs ) ? &aD[9] : 0;
        pSep->pPLCFx = pBase->pSepPLCF;
        pFtn->pPLCFx = pBase->pFtnPLCF;
        pEdn->pPLCFx = pBase->pEdnPLCF;
        pBkm->pPLCFx = pBase->pBook;
        pAnd->pPLCFx = pBase->pAndPLCF;
    }
    else
    {
        // Suchreihenfolge der Attribute
        nPLCF = 7;
        pFld = &aD[0];
        pBkm = ( pBase->pBook ) ? &aD[1] : 0;
        pChp = &aD[2];
        pPap = &aD[3];
        pSep = &aD[4];          // Dummy
        pPcd = ( pBase->pPLCFx_PCD ) ? &aD[5] : 0;
        pPcdA= ( pBase->pPLCFx_PCDAttrs ) ? &aD[6] : 0;
        pAnd = pFtn = pEdn = 0;     // unbenutzt bei SpezText
    }

    pChp->pPLCFx = pBase->pChpPLCF;
    pPap->pPLCFx = pBase->pPapPLCF;
    if( pPcd )
        pPcd->pPLCFx = pBase->pPLCFx_PCD;
    if( pPcdA )
        pPcdA->pPLCFx= pBase->pPLCFx_PCDAttrs;
    if( pBkm )
        pBkm->pPLCFx = pBase->pBook;

    switch( nType )                 // Feld-Initialisierung
    {
    case MAN_HDFT: pFld->pPLCFx = pBase->pFldHdFtPLCF;
                   nCpO     = pWwFib->ccpText + pWwFib->ccpFtn;
                   pFdoa    = pBase->pHdFtFdoa;
                   pTxbx    = pBase->pHdFtTxbx;
                   pTxbxBkd = pBase->pHdFtTxbxBkd;
                   break;
    case MAN_FTN:  pFld->pPLCFx = pBase->pFldFtnPLCF;
                   nCpO     = pWwFib->ccpText;
                   pFdoa    = pTxbx = pTxbxBkd = 0;
                   break;
    case MAN_EDN:  pFld->pPLCFx = pBase->pFldEdnPLCF;
                   nCpO     = pWwFib->ccpText + pWwFib->ccpFtn
                            + pWwFib->ccpHdr + pWwFib->ccpAtn;
                   pFdoa = pTxbx = pTxbxBkd = 0;
                   break;
    case MAN_AND:  pFld->pPLCFx = pBase->pFldAndPLCF;
                   nCpO = pWwFib->ccpText + pWwFib->ccpFtn
                            + pWwFib->ccpHdr;
                   pFdoa = pTxbx = pTxbxBkd = 0;
                   break;


    case MAN_TXBX: pFld->pPLCFx = pBase->pFldTxbxPLCF;
                   nCpO     = pWwFib->ccpText + pWwFib->ccpFtn
                            + pWwFib->ccpHdr + pWwFib->ccpMcr
                            + pWwFib->ccpAtn + pWwFib->ccpEdn;
                   pTxbx    = pBase->pMainTxbx;
                   pTxbxBkd = pBase->pMainTxbxBkd;
                   pFdoa = 0;
                   break;
    case MAN_TXBX_HDFT:
                   pFld->pPLCFx = pBase->pFldTxbxHdFtPLCF;
                   nCpO     = pWwFib->ccpText + pWwFib->ccpFtn
                            + pWwFib->ccpHdr + pWwFib->ccpMcr
                            + pWwFib->ccpAtn + pWwFib->ccpEdn
                            + pWwFib->ccpTxbx;
                   pTxbx    = pBase->pHdFtTxbx;
                   pTxbxBkd = pBase->pHdFtTxbxBkd;
                   pFdoa = 0;
                   break;


    default: pFld->pPLCFx = pBase->pFldPLCF;
                   nCpO = 0;
                   pFdoa    = pBase->pMainFdoa;
                   pTxbx    = pBase->pMainTxbx;
                   pTxbxBkd = pBase->pMainTxbxBkd;
                   break;
    }

    if( nStartCp || nCpO )
        SeekPos( nStartCp );    // PLCFe auf Text-StartPos einstellen

    // initialisieren der Member-Vars Low-Level
    GetChpPLCF()->ResetAttrStartEnd();
    GetPapPLCF()->ResetAttrStartEnd();
    for( i=0; i<nPLCF; i++)
    {
        register WW8PLCFxDesc* p = &aD[i];

        p->nCpOfs = ( p == pChp || p == pPap || p == pBkm ) ? nCpO : 0;
        p->nCp2OrIdx = 0;
        p->bFirstSprm = FALSE;
        p->pIdStk = 0;
        p->nStartPos = p->nEndPos = LONG_MAX;
    }

    // initialisieren der Member-Vars High-Level
    for( i=0; i<nPLCF; i++){
        register WW8PLCFxDesc* p = &aD[i];

        if( !p->pPLCFx )
            continue;

        if( p->pPLCFx->IsSprm() )
        {
            p->pIdStk = new UShortStk;  // Vorsicht: nEndPos muss bereits
            GetNewSprms( *p );          // bei allen PLCFen initialisiert sein
        }
        else
        {
            if( p->pPLCFx )
                GetNewNoSprms( *p );
        }
    }
}

WW8PLCFMan::~WW8PLCFMan()
{
    for( USHORT i=0; i<nPLCF; i++)
        delete aD[i].pIdStk;
}

// 0. welche Attr.-Klasse,
// 1. ob ein Attr.-Start ist,
// 2. CP, wo ist naechste Attr.-Aenderung
short WW8PLCFMan::WhereIdx( BOOL* pbStart, long* pPos )
{
    BOOL bIgnore;
    long next = LONG_MAX;   // SuchReihenfolge:
    short nextIdx = -1;     // erst Enden finden ( CHP, PAP, ( SEP ) ),
    BOOL bStart = TRUE;     // dann Anfaenge finden ( ( SEP ), PAP, CHP )
    short i;
    register WW8PLCFxDesc* pD;
    for( i=0; i<(short)nPLCF; i++)
    {
        pD = &aD[i];
        if(     ( pD->nEndPos    < next     )
            &&  ( pD->nStartPos == LONG_MAX ) )
        {   // sonst ist Anfang = Ende
            next = pD->nEndPos;
            nextIdx = i;
            bStart = FALSE;
        }
    }
    for( i=(short)nPLCF-1; i>=0; i--)
    {
        bIgnore = FALSE;
        pD = &aD[i];
        if( pD->nStartPos < next )
        {
            // StartPos of Attribute is inside a range of CPs to be ignored?
            if(    pD->pMemPos
                && pNoAttrScan
                && *pNoAttrScan
                && pD->nStartPos == nLastWhereIdxCp )
            {
                // Check if the EndPos of this Attribute is BEHIND the scope
                // of CPs to be ignored: if so  shift the StartPos accordingly!
                ULONG nOldIdx = pD->pPLCFx->GetIdx();
                pD->pPLCFx->SetIdx( nOldIdx+1 );

                const long nNASEnd = pD->nStartPos + *pNoAttrScan;

                if( nNASEnd < pD->pPLCFx->Where() )
                {
                    pD->nStartPos = nNASEnd;
                    bIgnore = TRUE;
                }

                pD->pPLCFx->SetIdx( nOldIdx );
            }

            if( !bIgnore )
            {
                next = pD->nStartPos;
                nextIdx = i;
                bStart = TRUE;
            }
        }
    }
    if( pPos )
        *pPos = next;
    nLastWhereIdxCp = next;
    if( pbStart )
        *pbStart = bStart;
    return nextIdx;
}

WW8_CP WW8PLCFMan::Where()      // gibt die CP-Pos der naechsten Attribut-Aenderung zurueck
{
    long l;
    WhereIdx( 0, &l );
    return l;
}


void WW8PLCFMan::SeekPos( long nNewCp )
{
    pChp->pPLCFx->SeekPos( nNewCp + nCpO ); // Attribute neu
    pPap->pPLCFx->SeekPos( nNewCp + nCpO ); // aufsetzen
    pFld->pPLCFx->SeekPos( nNewCp );
    if( pPcd )
        pPcd->pPLCFx->SeekPos( nNewCp + nCpO );
    if( pBkm )
        pBkm->pPLCFx->SeekPos( nNewCp + nCpO );
}


/*
void WW8PLCFMan::Save1PLCFx( WW8PLCFxDesc* p, WW8PLCFxSave1* pSave ) const
{
    if( !p->pPLCFx )
        return;
    pSave->nPLCFxPos = p->pPLCFx->GetIdx();
    pSave->nPLCFxPos2 = p->pPLCFx->GetIdx2();
    if( p->pPLCFx->IsSprm() )
    {
        WW8PLCFxDesc aD;
        p->pPLCFx->GetSprms( &aD );
        pSave->nPLCFxMemOfs = p->pMemPos - aD.pMemPos;
    }
}

void WW8PLCFMan::Restore1PLCFx( WW8PLCFxDesc* p, WW8PLCFxSave1* pSave )
{
    if( !p->pPLCFx )
        return;
    p->pPLCFx->SetIdx( pSave->nPLCFxPos );          // restore PLCF-Pos
    p->pPLCFx->SetIdx2( pSave->nPLCFxPos2 );
    if( p->pPLCFx->IsSprm() )
    {
        WW8PLCFxDesc aD;
        p->pPLCFx->GetSprms( &aD );
        p->pMemPos = aD.pMemPos + pSave->nPLCFxMemOfs;
    }
}
*/
void WW8PLCFMan::SaveAllPLCFx( WW8PLCFxSaveAll& rSave ) const
{
    USHORT i, n=0;
    if( pPcd )
        pPcd->Save(  rSave.aS[n++] );
    if( pPcdA )
        pPcdA->Save( rSave.aS[n++] );

    for(i=0; i<nPLCF; ++i)
        if( pPcd != &aD[i] && pPcdA != &aD[i] )
            aD[i].Save( rSave.aS[n++] );
}

void WW8PLCFMan::RestoreAllPLCFx( const WW8PLCFxSaveAll& rSave )
{
    USHORT i, n=0;
    if( pPcd )
        pPcd->Restore(  rSave.aS[n++] );
    if( pPcdA )
        pPcdA->Restore( rSave.aS[n++] );

    for(i=0; i<nPLCF; ++i)
        if( pPcd != &aD[i] && pPcdA != &aD[i] )
            aD[i].Restore( rSave.aS[n++] );
}

void WW8PLCFMan::GetSprmStart( short nIdx, WW8PLCFManResult* pRes )
{
    memset( pRes, 0, sizeof( WW8PLCFManResult ) );

    // Pruefen !!!

    pRes->nMemLen = 0;

    register WW8PLCFxDesc* p = &aD[nIdx];

    if(    !*pNoAttrScan
        || (*pNoAttrScan < (p->nEndPos - p->nStartPos + 1)) )
    {
        if( p->bFirstSprm )                         // erster Sprm einer Gruppe
        {
            if( p == pPap )                         // Anfang neuer Pap ?
                pRes->nFlags |= MAN_MASK_NEW_PAP;   // dann muss Aufrufer auf neuen
                                                    // Style testen
            else if( p == pSep )                    // Anfang neuer Sep ?
                pRes->nFlags |= MAN_MASK_NEW_SEP;   // dann muss Aufrufer eine neue
                                                    // Section / Pagedesc erzeugen
        }
        pRes->pMemPos = p->pMemPos;
        pRes->nSprmId = GetId( pWwFib->nVersion, p );
        if( p->nSprmsLen )
            pRes->nMemLen = WW8GetSprmSizeBrutto( pWwFib->nVersion,
                                                pRes->pMemPos,
                                                &pRes->nSprmId ); // Laenge des akt. Sprm
    }
}

void WW8PLCFMan::GetSprmEnd( short nIdx, WW8PLCFManResult* pRes )
{
    memset( pRes, 0, sizeof( WW8PLCFManResult ) );

    register WW8PLCFxDesc* p = &aD[nIdx];

    if(    !*pNoAttrScan
        || (*pNoAttrScan < (p->nEndPos - p->nStartPos + 1)) )
    {
        if( p->pIdStk->Count() )
            pRes->nSprmId = p->pIdStk->Top();       // hole Ende-Position
        else
        {
            ASSERT( !this, "Keine Id auf dem Stack" );
            pRes->nSprmId = 0;
        }
    }
}

void WW8PLCFMan::GetNoSprmStart( short nIdx, WW8PLCFManResult* pRes )
{

    // Pruefen !!!

    register WW8PLCFxDesc* p = &aD[nIdx];
//  long nEnd;

//  p->nCp2OrIdx = p->pPLCFx->GetNoSprms( pRes->nCpPos, nEnd, pRes->nMemLen );
//  p->nCp2OrIdx = p->pPLCFx->GetNoSprms( p->nStartPos, p->nEndPos,
//                                              p->nSprmsLen );
//  GetNoSprms( nIdx );
    pRes->nCpPos = p->nStartPos;
    pRes->nMemLen = p->nSprmsLen;
    pRes->nCp2OrIdx = p->nCp2OrIdx;

    if( p == pFld )
        pRes->nSprmId = 258;
    else if( p == pFtn )
        pRes->nSprmId = 256;
    else if( p == pEdn )
        pRes->nSprmId = 257;
    else if( p == pBkm )
        pRes->nSprmId = 259;
    else if( p == pPcd )
        pRes->nSprmId = 260;
    else if( p == pAnd )
        pRes->nSprmId = 261;
    else
        pRes->nSprmId = 0;          // default: nicht gefunden
}

void WW8PLCFMan::GetNoSprmEnd( short nIdx, WW8PLCFManResult* pRes )
{

    // Pruefen !!!

    if( &aD[nIdx] == pBkm )
        pRes->nSprmId = 259;
    else if( &aD[nIdx] == pPcd )
        pRes->nSprmId = 260;
    else
        pRes->nSprmId = 0;

    pRes->nMemLen = -1;     // Ende-Kennzeichen
}


//
void WW8PLCFMan::AdvSprm( short nIdx, BOOL bStart )
{
    register WW8PLCFxDesc* p = &aD[nIdx];   // Sprm-Klasse(!) ermitteln

    p->bFirstSprm = FALSE;
    if( bStart )
    {
        USHORT nLastId = GetId( pWwFib->nVersion, p );
        p->pIdStk->Push( nLastId ); // merke Id fuer Attribut-Ende

        if( p->nSprmsLen )
        {   /*
                Pruefe, ob noch Sprm(s) abzuarbeiten sind
            */
            if( p->pMemPos )
            {
                // Laenge des letzten Sprm
                short nSprmL = WW8GetSprmSizeBrutto( pWwFib->nVersion, p->pMemPos, &nLastId );

                // Gesamtlaenge Sprms um SprmLaenge verringern
                p->nSprmsLen -= nSprmL;

                // Pos des evtl. naechsten Sprm
                if( p->nSprmsLen <= 0 )
                    p->pMemPos = 0;     // sicherheitshalber auf Null setzen, da Enden folgen!
                else
                    p->pMemPos += nSprmL;
            }
            else
                p->nSprmsLen = 0;
        }
        if( p->nSprmsLen <= 0 )
            p->nStartPos = LONG_MAX;    // es folgen Enden
    }
    else
    {
        if( p->pIdStk->Count() )
            p->pIdStk->Pop();
        if( !p->pIdStk->Count() )
        {                       // Kein weiterer Sprm auf dieser Pos ?
            (*p->pPLCFx)++;     // naechste Gruppe von Sprms
            p->pMemPos = 0;     // !!!
            GetNewSprms( *p );
            ASSERT( p->nStartPos <= p->nEndPos, "Attribut ueber Kreuz" );
        }
    }
}

void WW8PLCFMan::AdvNoSprm( short nIdx, BOOL bStart )
{

    // Pruefen !!!

    register WW8PLCFxDesc* p = &aD[nIdx];

    if( p == pPcd ){                        // NoSprm mit Ende
        if( bStart ){
            p->nStartPos = LONG_MAX;
        }else{                              // naechster NoSprm mit Ende
            (*p->pPLCFx)++;
            p->pMemPos = 0;                 // MemPos ungueltig
            GetNewNoSprms( *p );
//          p->nCp2OrIdx = p->pPLCFx->GetNoSprms( p->nStartPos, p->nEndPos,
//                                              p->nSprmsLen );
        }
    }else{                                  // NoSprm ohne Ende
        (*p->pPLCFx)++;
        p->pMemPos = 0;                     // MemPos ungueltig
        GetNewNoSprms( *p );
//      p->nCp2OrIdx = p->pPLCFx->GetNoSprms( p->nStartPos, p->nEndPos,
//                                          p->nSprmsLen );
    }
}

WW8PLCFMan& WW8PLCFMan::operator ++(int)
{
    BOOL bStart;
    short nIdx = WhereIdx( &bStart, 0 );
    if( nPLCF > nIdx )
    {
        register WW8PLCFxDesc* p = &aD[nIdx];

        p->bFirstSprm = TRUE;                       // Default

        if( p->pPLCFx->IsSprm() )
            AdvSprm( nIdx, bStart );
        else                                        // NoSprm
            AdvNoSprm( nIdx, bStart );
    }
    return *this;
}

// Rueckgabe TRUE fuer Anfang eines Attributes oder Fehler,
//           FALSE fuer Ende d. Attr
// Restliche Rueckgabewerte werden in der vom Aufrufer zu stellenden Struktur
// WW8PclxManResults geliefert.
BOOL WW8PLCFMan::Get( WW8PLCFManResult* pRes )
{
    memset( pRes, 0, sizeof( WW8PLCFManResult ) );
    BOOL bStart;
    short nIdx = WhereIdx( &bStart, 0 );

    if( nIdx < 0 ){
        ASSERT( !this, "Position nicht gefunden" );
        return TRUE;
    }

    if( aD[nIdx].pPLCFx->IsSprm() )
    {
        if( bStart )
        {
            GetSprmStart( nIdx, pRes );
            return TRUE;
        }
        else
        {
            GetSprmEnd( nIdx, pRes );
            return FALSE;
        }
    }else{
/*
        if( &aD[nIdx] == pPcd ){            // Piece-Ende
            pRes->nId = -4;
            pRes->nMemLen = ( bStart ) ? 0 : -1;
            return bStart;
        }else
*/
        if( bStart ){
            GetNoSprmStart( nIdx, pRes );
            return TRUE;
        }else{
            GetNoSprmEnd( nIdx, pRes );
            return FALSE;
        }
    }
}

USHORT WW8PLCFMan::GetColl() const
{
    if( pPap->pPLCFx ){
        return  pPap->pPLCFx->GetIstd();
    }else{
        ASSERT( !this, "GetColl ohne PLCF_Pap" );
        return 0;
    }
}

WW8PLCFx_FLD* WW8PLCFMan::GetFld() const
{
    return (WW8PLCFx_FLD*)pFld->pPLCFx;
}

BYTE* WW8PLCFMan::HasParaSprm( USHORT nId ) const
{
    return ((WW8PLCFx_Cp_FKP*)pPap->pPLCFx)->HasSprm( nId );
}

BYTE* WW8PLCFMan::HasCharSprm( USHORT nId ) const
{
    return ((WW8PLCFx_Cp_FKP*)pChp->pPLCFx)->HasSprm( nId );
}

#endif // !DUMP


void WW8PLCFx::Save( WW8PLCFxSave1& rSave ) const
{
     rSave.nPLCFxPos    = GetIdx();
    rSave.nPLCFxPos2   = GetIdx2();
    rSave.nPLCFxMemOfs = 0;
}

void WW8PLCFx::Restore( const WW8PLCFxSave1& rSave )
{
    SetIdx(  rSave.nPLCFxPos  );
    SetIdx2( rSave.nPLCFxPos2 );
}



ULONG WW8PLCFx_Cp_FKP::GetIdx2() const
{
    return GetPCDIdx();
}

void WW8PLCFx_Cp_FKP::SetIdx2( ULONG nIdx )
{
    SetPCDIdx( nIdx );
}


void WW8PLCFx_Cp_FKP::Save( WW8PLCFxSave1& rSave ) const
{
    WW8PLCFx::Save( rSave );

    rSave.nAttrStart = nAttrStart;
    rSave.nAttrEnd   = nAttrEnd;
    rSave.bLineEnd   = bLineEnd;
}

void WW8PLCFx_Cp_FKP::Restore( const WW8PLCFxSave1& rSave )
{
    WW8PLCFx::Restore( rSave );

    nAttrStart = rSave.nAttrStart;
    nAttrEnd   = rSave.nAttrEnd;
    bLineEnd   = rSave.bLineEnd;
}


void WW8PLCFxDesc::Save( WW8PLCFxSave1& rSave ) const
{
    if( pPLCFx )
    {
        pPLCFx->Save( rSave );
        if( pPLCFx->IsSprm() )
        {
            WW8PLCFxDesc aD;
            pPLCFx->GetSprms( &aD );
            rSave.nPLCFxMemOfs = pMemPos - aD.pMemPos;
        }
    }
}

void WW8PLCFxDesc::Restore( const WW8PLCFxSave1& rSave )
{
    if( pPLCFx )
    {
        pPLCFx->Restore( rSave );
        if( pPLCFx->IsSprm() )
        {
            WW8PLCFxDesc aD;
            pPLCFx->GetSprms( &aD );
            pMemPos = aD.pMemPos + rSave.nPLCFxMemOfs;
        }
    }
}

//-----------------------------------------


WW8Fib::WW8Fib( SvStream& rSt, BYTE nWantedVersion,UINT32 nOffset ): nFibError( 0 )
{
    BYTE aBits1;
    BYTE aBits2;
    BYTE aVer8Bits1;    // nur ab WinWord 8 benutzt
    rSt.Seek( nOffset );
    /*
        Wunsch-Nr vermerken, File-Versionsnummer ermitteln
        und gegen Wunsch-Nr. checken !
    */
    nVersion = nWantedVersion;
    rSt >> wIdent;
    rSt >> nFib;
    rSt >> nProduct;
    if( 0 != rSt.GetError() )
    {
        INT16 nFibMin;
        INT16 nFibMax;
        switch( nVersion )  // beachte: 6 steht fuer "6 ODER 7",  7 steht fuer "NUR 7"
            {
        case 6: nFibMin = 0x0065;   // von 101 WinWord 6.0
                                                            //     102    "
                                                            // und 103 WinWord 6.0 fuer Macintosh
                                                            //     104    "
                    nFibMax = 0x0069;   // bis 105 WinWord 95
                        break;
        case 7: nFibMin = 0x0069;   // von 105 WinWord 95
                        nFibMax = 0x0069;   // bis 105 WinWord 95
                        break;
        case 8: nFibMin = 0x006A;   // von 106 WinWord 97
                        nFibMax = 0x00c1;   // bis 193 WinWord 97 (?)
                        break;
        default:nFibMin = 0;            // Programm-Fehler!
                        nFibMax = 0;
                        nFib    = 1;
                        ASSERT( !this, "Es wurde vergessen, nVersion zu kodieren!" );
        }
        if(    ( nFib < nFibMin )
            || ( nFib > nFibMax ) )
        {
            nFibError = ERR_SWG_READ_ERROR; // Error melden
            return;                         // und hopp raus!
        }
    }

    // praktische Hilfsvariablen:
    BOOL bVer67 = ((6 == nVersion) || (7 == nVersion));
    BOOL bVer8  =  (8 == nVersion);

    // Hilfs-Varis fuer Ver67:
    INT16 pnChpFirst_Ver67;
    INT16 pnPapFirst_Ver67;
    INT16 cpnBteChp_Ver67;
    INT16 cpnBtePap_Ver67;

    // und auf gehts: FIB einlesen
    rSt >> lid;
    rSt >> pnNext;
    rSt >> aBits1;
    rSt >> aBits2;
    rSt >> nFibBack;
    rSt >> lKey1;
    rSt >> lKey2;
    rSt >> envr;
    rSt >> aVer8Bits1;      // unter Ver67  nur leeres Reservefeld
                // Inhalt von aVer8Bits1
                //
                // BYTE fMac              :1;
                // BYTE fEmptySpecial     :1;
                // BYTE fLoadOverridePage :1;
                // BYTE fFuturesavedUndo  :1;
                // BYTE fWord97Saved      :1;
                // BYTE :3;
    rSt >> chse;
    rSt >> chseTables;
    rSt >> fcMin;
    rSt >> fcMac;

// Einschub fuer WW8 *****************************************************
    if( !bVer67 )
    {
        rSt >> csw;

        // Marke: "rgsw"  Beginning of the array of shorts
        rSt >> wMagicCreated;
        rSt >> wMagicRevised;
        rSt >> wMagicCreatedPrivate;
        rSt >> wMagicRevisedPrivate;
        rSt.SeekRel( 9 * sizeof( INT16 ) );

        /*
        // dies sind die 9 unused Felder:
        && (bVer67 || WW8ReadINT16(  rSt, pnFbpChpFirst_W6          ))  // 1
        && (bVer67 || WW8ReadINT16(  rSt, pnChpFirst_W6                 ))  // 2
        && (bVer67 || WW8ReadINT16(  rSt, cpnBteChp_W6                  ))  // 3
        && (bVer67 || WW8ReadINT16(  rSt, pnFbpPapFirst_W6          ))  // 4
        && (bVer67 || WW8ReadINT16(  rSt, pnPapFirst_W6                 ))  // 5
        && (bVer67 || WW8ReadINT16(  rSt, cpnBtePap_W6                  ))  // 6
        && (bVer67 || WW8ReadINT16(  rSt, pnFbpLvcFirst_W6          ))  // 7
        && (bVer67 || WW8ReadINT16(  rSt, pnLvcFirst_W6                 ))  // 8
        && (bVer67 || WW8ReadINT16(  rSt, cpnBteLvc_W6                  ))  // 9
        */
        rSt >> lidFE;
        rSt >> clw;
    }

// Ende des Einschubs fuer WW8 *******************************************

        // Marke: "rglw"  Beginning of the array of longs
    rSt >> cbMac;

        // 2 Longs uebergehen, da unwichtiger Quatsch
    rSt.SeekRel( 2 * sizeof( INT32) );

        // weitere 2 Longs nur bei Ver67 ueberspringen
    if( bVer67 )
        rSt.SeekRel( 2 * sizeof( INT32) );

    rSt >> ccpText;
    rSt >> ccpFtn;
    rSt >> ccpHdr;
    rSt >> ccpMcr;
    rSt >> ccpAtn;
    rSt >> ccpEdn;
    rSt >> ccpTxbx;
    rSt >> ccpHdrTxbx;

        // weiteres Long nur bei Ver67 ueberspringen
    if( bVer67 )
        rSt.SeekRel( 1 * sizeof( INT32) );
    else
    {
// Einschub fuer WW8 *****************************************************
        rSt >> pnFbpChpFirst;
        rSt >> pnChpFirst;
        rSt >> cpnBteChp;
        rSt >> pnFbpPapFirst;
        rSt >> pnPapFirst;
        rSt >> cpnBtePap;
        rSt >> pnFbpLvcFirst;
        rSt >> pnLvcFirst;
        rSt >> cpnBteLvc;
        rSt >> fcIslandFirst;
        rSt >> fcIslandLim;
        rSt >> cfclcb;
    }

// Ende des Einschubs fuer WW8 *******************************************

        // Marke: "rgfclcb" Beginning of array of FC/LCB pairs.
    rSt >> fcStshfOrig;
    rSt >> lcbStshfOrig;
    rSt >> fcStshf;
    rSt >> lcbStshf;
    rSt >> fcPlcffndRef;
    rSt >> lcbPlcffndRef;
    rSt >> fcPlcffndTxt;
    rSt >> lcbPlcffndTxt;
    rSt >> fcPlcfandRef;
    rSt >> lcbPlcfandRef;
    rSt >> fcPlcfandTxt;
    rSt >> lcbPlcfandTxt;
    rSt >> fcPlcfsed;
    rSt >> lcbPlcfsed;
    rSt >> fcPlcfpad;
    rSt >> lcbPlcfpad;
    rSt >> fcPlcfphe;
    rSt >> lcbPlcfphe;
    rSt >> fcSttbfglsy;
    rSt >> lcbSttbfglsy;
    rSt >> fcPlcfglsy;
    rSt >> lcbPlcfglsy;
    rSt >> fcPlcfhdd;
    rSt >> lcbPlcfhdd;
    rSt >> fcPlcfbteChpx;
    rSt >> lcbPlcfbteChpx;
    rSt >> fcPlcfbtePapx;
    rSt >> lcbPlcfbtePapx;
    rSt >> fcPlcfsea;
    rSt >> lcbPlcfsea;
    rSt >> fcSttbfffn;
    rSt >> lcbSttbfffn;
    rSt >> fcPlcffldMom;
    rSt >> lcbPlcffldMom;
    rSt >> fcPlcffldHdr;
    rSt >> lcbPlcffldHdr;
    rSt >> fcPlcffldFtn;
    rSt >> lcbPlcffldFtn;
    rSt >> fcPlcffldAtn;
    rSt >> lcbPlcffldAtn;
    rSt >> fcPlcffldMcr;
    rSt >> lcbPlcffldMcr;
    rSt >> fcSttbfbkmk;
    rSt >> lcbSttbfbkmk;
    rSt >> fcPlcfbkf;
    rSt >> lcbPlcfbkf;
    rSt >> fcPlcfbkl;
    rSt >> lcbPlcfbkl;
    rSt >> fcCmds;
    rSt >> lcbCmds;
    rSt >> fcPlcfmcr;
    rSt >> lcbPlcfmcr;
    rSt >> fcSttbfmcr;
    rSt >> lcbSttbfmcr;
    rSt >> fcPrDrvr;
    rSt >> lcbPrDrvr;
    rSt >> fcPrEnvPort;
    rSt >> lcbPrEnvPort;
    rSt >> fcPrEnvLand;
    rSt >> lcbPrEnvLand;
    rSt >> fcWss;
    rSt >> lcbWss;
    rSt >> fcDop;
    rSt >> lcbDop;
    rSt >> fcSttbfAssoc;
    rSt >> cbSttbfAssoc;
    rSt >> fcClx;
    rSt >> lcbClx;
    rSt >> fcPlcfpgdFtn;
    rSt >> lcbPlcfpgdFtn;
    rSt >> fcAutosaveSource;
    rSt >> lcbAutosaveSource;
    rSt >> fcGrpStAtnOwners;
    rSt >> lcbGrpStAtnOwners;
    rSt >> fcSttbfAtnbkmk;
    rSt >> lcbSttbfAtnbkmk;

        // weiteres short nur bei Ver67 ueberspringen
    if( bVer67 )
    {
        rSt.SeekRel( 1*sizeof( INT16) );

        // folgende 4 Shorts existieren nur bei Ver67;
        rSt >> pnChpFirst_Ver67;
        rSt >> pnPapFirst_Ver67;
        rSt >> cpnBteChp_Ver67;
        rSt >> cpnBtePap_Ver67;
    }

    rSt >> fcPlcfdoaMom;
    rSt >> lcbPlcfdoaMom;
    rSt >> fcPlcfdoaHdr;
    rSt >> lcbPlcfdoaHdr;
    rSt >> fcPlcfspaMom;
    rSt >> lcbPlcfspaMom;
    rSt >> fcPlcfspaHdr;
    rSt >> lcbPlcfspaHdr;

    rSt >> fcPlcfAtnbkf;
    rSt >> lcbPlcfAtnbkf;
    rSt >> fcPlcfAtnbkl;
    rSt >> lcbPlcfAtnbkl;
    rSt >> fcPms;
    rSt >> lcbPMS;
    rSt >> fcFormFldSttbf;
    rSt >> lcbFormFldSttbf;
    rSt >> fcPlcfendRef;
    rSt >> lcbPlcfendRef;
    rSt >> fcPlcfendTxt;
    rSt >> lcbPlcfendTxt;
    rSt >> fcPlcffldEdn;
    rSt >> lcbPlcffldEdn;
    rSt >> fcPlcfpgdEdn;
    rSt >> lcbPlcfpgdEdn;
    rSt >> fcDggInfo;
    rSt >> lcbDggInfo;
    rSt >> fcSttbfRMark;
    rSt >> lcbSttbfRMark;
    rSt >> fcSttbfCaption;
    rSt >> lcbSttbfCaption;
    rSt >> fcSttbAutoCaption;
    rSt >> lcbSttbAutoCaption;
    rSt >> fcPlcfwkb;
    rSt >> lcbPlcfwkb;
    rSt >> fcPlcfspl;
    rSt >> lcbPlcfspl;
    rSt >> fcPlcftxbxTxt;
    rSt >> lcbPlcftxbxTxt;
    rSt >> fcPlcffldTxbx;
    rSt >> lcbPlcffldTxbx;
    rSt >> fcPlcfHdrtxbxTxt;
    rSt >> lcbPlcfHdrtxbxTxt;
    rSt >> fcPlcffldHdrTxbx;
    rSt >> lcbPlcffldHdrTxbx;

    if( 0 == rSt.GetError() )
    {
        // Bit-Flags setzen
        fDot        =   aBits1 & 0x01       ;
        fGlsy       = ( aBits1 & 0x02 ) >> 1;
        fComplex    = ( aBits1 & 0x04 ) >> 2;
        fHasPic     = ( aBits1 & 0x08 ) >> 3;
        cQuickSaves = ( aBits1 & 0xf0 ) >> 4;
        fEncrypted  =   aBits2 & 0x01       ;
        fWhichTblStm= ( aBits2 & 0x02 ) >> 1;
        // dummy    = ( aBits2 & 0x0e ) >> 1;
        fExtChar    = ( aBits2 & 0x10 ) >> 4;
        // dummy    = ( aBits2 & 0xe0 ) >> 5;

        /*
            ggfs. Ziel-Varaiblen, aus xxx_Ver67 fuellen
            oder Flags setzen
        */
        if( bVer67 )
        {
            pnChpFirst = pnChpFirst_Ver67;
            pnPapFirst = pnPapFirst_Ver67;
            cpnBteChp  = cpnBteChp_Ver67;
            cpnBtePap  = cpnBtePap_Ver67;
        }
        else if( bVer8 )
        {
          fMac              =   aVer8Bits1  & 0x01           ;
          fEmptySpecial     = ( aVer8Bits1  & 0x02 ) >> 1;
          fLoadOverridePage = ( aVer8Bits1  & 0x04 ) >> 2;
          fFuturesavedUndo  = ( aVer8Bits1  & 0x08 ) >> 3;
          fWord97Saved      = ( aVer8Bits1  & 0x10 ) >> 4;

            /*
                speziell fuer WW8:
                ermittle die Werte fuer PLCF LST und PLF LFO
                und PLCF fuer TextBox-Break-Deskriptoren
            */
            long nOldPos = rSt.Tell();
            rSt.Seek( 0x02e2 );
            rSt >> fcPlcfLst;
            rSt >> lcbPlcfLst;
            rSt >> fcPlfLfo;
            rSt >> lcbPlfLfo;
            rSt >> fcPlcftxbxBkd;
            rSt >> lcbPlcftxbxBkd;
            rSt >> fcPlcfHdrtxbxBkd;
            rSt >> lcbPlcfHdrtxbxBkd;
            if( 0 != rSt.GetError() )
            {
                nFibError = ERR_SWG_READ_ERROR;
            }

            rSt.Seek( 0x372 );          // fcSttbListNames
            rSt >>  fcSttbListNames;
            rSt >> lcbSttbListNames;
            if( 0 != rSt.GetError() )
                nFibError = ERR_SWG_READ_ERROR;

            rSt.Seek( nOldPos );
        }
    }
    else
    {
        nFibError = ERR_SWG_READ_ERROR;     // Error melden
    }
}


WW8Fib::WW8Fib( BYTE nVer )
{
    memset( this, 0, sizeof( *this ));
    nVersion = nVer;
    if( 8 == nVer )
    {
        fcMin = 0x400;
        wIdent = 0xa5ec;
        nFib = nFibBack = 0xc1;
        nProduct = 0x49;

        csw = 0x0e;     // muss das sein ???
        cfclcb = 0x5d;  //      -""-
        clw = 0x16;     //      -""-
        pnFbpChpFirst = pnFbpPapFirst = pnFbpLvcFirst = 0x000fffff;
        fExtChar = TRUE;

// diese Flags muessen nicht gesetzt werden; koennen aber.
//      wMagicCreated = wMagicRevised = 0x6a62;
//      wMagicCreatedPrivate = wMagicRevisedPrivate = 0xb3b2;
//      fWord97Saved = TRUE;
    }
    else
    {
        fcMin = 0x300;
        wIdent = 0xa5dc;
        nFib = nFibBack = 0x65;
        nProduct = 0xc02d;
    }

    lid = 0x407;
}


BOOL WW8Fib::Write( SvStream& rStrm )
{
    BYTE* pData = new BYTE[ fcMin ];
    BYTE* pDataPtr = pData;
    memset( pData, 0, fcMin );

    BOOL bVer8 = 8 == nVersion;

    ULONG nPos = rStrm.Tell();
    cbMac = rStrm.Seek( STREAM_SEEK_TO_END );
    rStrm.Seek( nPos );

    Set_UInt16( pData, wIdent );
    Set_UInt16( pData, nFib );
    Set_UInt16( pData, nProduct );
    Set_UInt16( pData, lid );
    Set_UInt16( pData, pnNext );

    UINT16 nBits16 = 0;
    if( fDot )          nBits16 |= 0x0001;
    if( fGlsy)          nBits16 |= 0x0002;
    if( fComplex )      nBits16 |= 0x0004;
    if( fHasPic )       nBits16 |= 0x0008;
    nBits16 |= (0xf0 & ( cQuickSaves << 4 ));
    if( fEncrypted )    nBits16 |= 0x0100;
    if( fWhichTblStm )  nBits16 |= 0x0200;
    if( fExtChar )      nBits16 |= 0x1000;
    Set_UInt16( pData, nBits16 );

    Set_UInt16( pData, nFibBack );
    Set_UInt16( pData, lKey1 );
    Set_UInt16( pData, lKey2 );
    Set_UInt8( pData, envr );

    BYTE nBits8 = 0;
    if( bVer8 )
    {
        if( fMac )                  nBits8 |= 0x0001;
        if( fEmptySpecial )         nBits8 |= 0x0002;
        if( fLoadOverridePage )     nBits8 |= 0x0004;
        if( fFuturesavedUndo )      nBits8 |= 0x0008;
        if( fWord97Saved )          nBits8 |= 0x0010;
    }
    Set_UInt8( pData, nBits8  );                // unter Ver67  nur leeres Reservefeld

    Set_UInt16( pData, chse );
    Set_UInt16( pData, chseTables );
    Set_UInt32( pData, fcMin );
    Set_UInt32( pData, fcMac );

// Einschub fuer WW8 *****************************************************

        // Marke: "rgsw"  Beginning of the array of shorts
    if( bVer8 )
    {
        Set_UInt16( pData, csw );
        Set_UInt16( pData, wMagicCreated );
        Set_UInt16( pData, wMagicRevised );
        Set_UInt16( pData, wMagicCreatedPrivate );
        Set_UInt16( pData, wMagicRevisedPrivate );
        pData += 9 * sizeof( INT16 );
        Set_UInt16( pData, lidFE );
        Set_UInt16( pData, clw );
    }

// Ende des Einschubs fuer WW8 *******************************************

        // Marke: "rglw"  Beginning of the array of longs
    Set_UInt32( pData, cbMac );

    // 2 Longs uebergehen, da unwichtiger Quatsch
    pData += 2 * sizeof( INT32);

        // weitere 2 Longs nur bei Ver67 ueberspringen
    if( !bVer8 ) pData += 2 * sizeof( INT32);

    Set_UInt32( pData, ccpText );
    Set_UInt32( pData, ccpFtn );
    Set_UInt32( pData, ccpHdr );
    Set_UInt32( pData, ccpMcr );
    Set_UInt32( pData, ccpAtn );
    Set_UInt32( pData, ccpEdn );
    Set_UInt32( pData, ccpTxbx );
    Set_UInt32( pData, ccpHdrTxbx );

        // weiteres Long nur bei Ver67 ueberspringen
    if( !bVer8 ) pData += 1 * sizeof( INT32);

// Einschub fuer WW8 *****************************************************
    if( bVer8 )
    {
        Set_UInt32( pData, pnFbpChpFirst );
        Set_UInt32( pData, pnChpFirst );
        Set_UInt32( pData, cpnBteChp );
        Set_UInt32( pData, pnFbpPapFirst );
        Set_UInt32( pData, pnPapFirst );
        Set_UInt32( pData, cpnBtePap );
        Set_UInt32( pData, pnFbpLvcFirst );
        Set_UInt32( pData, pnLvcFirst );
        Set_UInt32( pData, cpnBteLvc );
        Set_UInt32( pData, fcIslandFirst );
        Set_UInt32( pData, fcIslandLim );
        Set_UInt16( pData, cfclcb );
    }
// Ende des Einschubs fuer WW8 *******************************************

        // Marke: "rgfclcb" Beginning of array of FC/LCB pairs.
    Set_UInt32( pData, fcStshfOrig );
    Set_UInt32( pData, lcbStshfOrig );
    Set_UInt32( pData, fcStshf );
    Set_UInt32( pData, lcbStshf );
    Set_UInt32( pData, fcPlcffndRef );
    Set_UInt32( pData, lcbPlcffndRef );
    Set_UInt32( pData, fcPlcffndTxt );
    Set_UInt32( pData, lcbPlcffndTxt );
    Set_UInt32( pData, fcPlcfandRef );
    Set_UInt32( pData, lcbPlcfandRef );
    Set_UInt32( pData, fcPlcfandTxt );
    Set_UInt32( pData, lcbPlcfandTxt );
    Set_UInt32( pData, fcPlcfsed );
    Set_UInt32( pData, lcbPlcfsed );
    Set_UInt32( pData, fcPlcfpad );
    Set_UInt32( pData, lcbPlcfpad );
    Set_UInt32( pData, fcPlcfphe );
    Set_UInt32( pData, lcbPlcfphe );
    Set_UInt32( pData, fcSttbfglsy );
    Set_UInt32( pData, lcbSttbfglsy );
    Set_UInt32( pData, fcPlcfglsy );
    Set_UInt32( pData, lcbPlcfglsy );
    Set_UInt32( pData, fcPlcfhdd );
    Set_UInt32( pData, lcbPlcfhdd );
    Set_UInt32( pData, fcPlcfbteChpx );
    Set_UInt32( pData, lcbPlcfbteChpx );
    Set_UInt32( pData, fcPlcfbtePapx );
    Set_UInt32( pData, lcbPlcfbtePapx );
    Set_UInt32( pData, fcPlcfsea );
    Set_UInt32( pData, lcbPlcfsea );
    Set_UInt32( pData, fcSttbfffn );
    Set_UInt32( pData, lcbSttbfffn );
    Set_UInt32( pData, fcPlcffldMom );
    Set_UInt32( pData, lcbPlcffldMom );
    Set_UInt32( pData, fcPlcffldHdr );
    Set_UInt32( pData, lcbPlcffldHdr );
    Set_UInt32( pData, fcPlcffldFtn );
    Set_UInt32( pData, lcbPlcffldFtn );
    Set_UInt32( pData, fcPlcffldAtn );
    Set_UInt32( pData, lcbPlcffldAtn );
    Set_UInt32( pData, fcPlcffldMcr );
    Set_UInt32( pData, lcbPlcffldMcr );
    Set_UInt32( pData, fcSttbfbkmk );
    Set_UInt32( pData, lcbSttbfbkmk );
    Set_UInt32( pData, fcPlcfbkf );
    Set_UInt32( pData, lcbPlcfbkf );
    Set_UInt32( pData, fcPlcfbkl );
    Set_UInt32( pData, lcbPlcfbkl );
    Set_UInt32( pData, fcCmds );
    Set_UInt32( pData, lcbCmds );
    Set_UInt32( pData, fcPlcfmcr );
    Set_UInt32( pData, lcbPlcfmcr );
    Set_UInt32( pData, fcSttbfmcr );
    Set_UInt32( pData, lcbSttbfmcr );
    Set_UInt32( pData, fcPrDrvr );
    Set_UInt32( pData, lcbPrDrvr );
    Set_UInt32( pData, fcPrEnvPort );
    Set_UInt32( pData, lcbPrEnvPort );
    Set_UInt32( pData, fcPrEnvLand );
    Set_UInt32( pData, lcbPrEnvLand );
    Set_UInt32( pData, fcWss );
    Set_UInt32( pData, lcbWss );
    Set_UInt32( pData, fcDop );
    Set_UInt32( pData, lcbDop );
    Set_UInt32( pData, fcSttbfAssoc );
    Set_UInt32( pData, cbSttbfAssoc );
    Set_UInt32( pData, fcClx );
    Set_UInt32( pData, lcbClx );
    Set_UInt32( pData, fcPlcfpgdFtn );
    Set_UInt32( pData, lcbPlcfpgdFtn );
    Set_UInt32( pData, fcAutosaveSource );
    Set_UInt32( pData, lcbAutosaveSource );
    Set_UInt32( pData, fcGrpStAtnOwners );
    Set_UInt32( pData, lcbGrpStAtnOwners );
    Set_UInt32( pData, fcSttbfAtnbkmk );
    Set_UInt32( pData, lcbSttbfAtnbkmk );

        // weiteres short nur bei Ver67 ueberspringen
    if( !bVer8 )
    {
        pData += 1*sizeof( INT16);
        Set_UInt16( pData, (UINT16)pnChpFirst );
        Set_UInt16( pData, (UINT16)pnPapFirst );
        Set_UInt16( pData, (UINT16)cpnBteChp );
        Set_UInt16( pData, (UINT16)cpnBtePap );
    }

    Set_UInt32( pData, fcPlcfdoaMom ); // nur bei Ver67, in Ver8 unused
    Set_UInt32( pData, lcbPlcfdoaMom ); // nur bei Ver67, in Ver8 unused
    Set_UInt32( pData, fcPlcfdoaHdr ); // nur bei Ver67, in Ver8 unused
    Set_UInt32( pData, lcbPlcfdoaHdr ); // nur bei Ver67, in Ver8 unused

    Set_UInt32( pData, fcPlcfspaMom ); // in Ver67 leere Reserve
    Set_UInt32( pData, lcbPlcfspaMom ); // in Ver67 leere Reserve
    Set_UInt32( pData, fcPlcfspaHdr ); // in Ver67 leere Reserve
    Set_UInt32( pData, lcbPlcfspaHdr ); // in Ver67 leere Reserve

    Set_UInt32( pData, fcPlcfAtnbkf );
    Set_UInt32( pData, lcbPlcfAtnbkf );
    Set_UInt32( pData, fcPlcfAtnbkl );
    Set_UInt32( pData, lcbPlcfAtnbkl );
    Set_UInt32( pData, fcPms );
    Set_UInt32( pData, lcbPMS );
    Set_UInt32( pData, fcFormFldSttbf );
    Set_UInt32( pData, lcbFormFldSttbf );
    Set_UInt32( pData, fcPlcfendRef );
    Set_UInt32( pData, lcbPlcfendRef );
    Set_UInt32( pData, fcPlcfendTxt );
    Set_UInt32( pData, lcbPlcfendTxt );
    Set_UInt32( pData, fcPlcffldEdn );
    Set_UInt32( pData, lcbPlcffldEdn );
    Set_UInt32( pData, fcPlcfpgdEdn );
    Set_UInt32( pData, lcbPlcfpgdEdn );
    Set_UInt32( pData, fcDggInfo ); // in Ver67 leere Reserve
    Set_UInt32( pData, lcbDggInfo ); // in Ver67 leere Reserve
    Set_UInt32( pData, fcSttbfRMark );
    Set_UInt32( pData, lcbSttbfRMark );
    Set_UInt32( pData, fcSttbfCaption );
    Set_UInt32( pData, lcbSttbfCaption );
    Set_UInt32( pData, fcSttbAutoCaption );
    Set_UInt32( pData, lcbSttbAutoCaption );
    Set_UInt32( pData, fcPlcfwkb );
    Set_UInt32( pData, lcbPlcfwkb );
    Set_UInt32( pData, fcPlcfspl ); // in Ver67 leere Reserve
    Set_UInt32( pData, lcbPlcfspl ); // in Ver67 leere Reserve
    Set_UInt32( pData, fcPlcftxbxTxt );
    Set_UInt32( pData, lcbPlcftxbxTxt );
    Set_UInt32( pData, fcPlcffldTxbx );
    Set_UInt32( pData, lcbPlcffldTxbx );
    Set_UInt32( pData, fcPlcfHdrtxbxTxt );
    Set_UInt32( pData, lcbPlcfHdrtxbxTxt );
    Set_UInt32( pData, fcPlcffldHdrTxbx );
    Set_UInt32( pData, lcbPlcffldHdrTxbx );

    if( bVer8 )
    {
        pData += 0x2e2 - 0x27a;             // Pos + Offset (fcPlcfLst - fcStwUser)
        Set_UInt32( pData, fcPlcfLst );
        Set_UInt32( pData, lcbPlcfLst );
        Set_UInt32( pData, fcPlfLfo );
        Set_UInt32( pData, lcbPlfLfo );
        Set_UInt32( pData, fcPlcftxbxBkd );
        Set_UInt32( pData, lcbPlcftxbxBkd );
        Set_UInt32( pData, fcPlcfHdrtxbxBkd );
        Set_UInt32( pData, lcbPlcfHdrtxbxBkd );

        pData += 0x372 - 0x302;             // Pos + Offset (fcSttbListNames - fcDocUndo)
        Set_UInt32( pData, fcSttbListNames );
        Set_UInt32( pData, lcbSttbListNames );
    }

    rStrm.Write( pDataPtr, fcMin );
    delete pDataPtr;
    return 0 == rStrm.GetError();
}

WW8Style::WW8Style( SvStream& rStream, WW8Fib& rFibPara ): rSt( rStream ), rFib( rFibPara )
{
    nStyleStart = rFib.fcStshf;
    nStyleLen   = rFib.lcbStshf;

    rSt.Seek( nStyleStart );

    USHORT cbStshi = 0; //  2 bytes size of the following STSHI structure

    // alte Version ?
    if ( rFib.nFib < 67 )
    {
        cbStshi = 4;    // -> Laengenfeld fehlt
    }
    else    // neue Version:
        // lies die Laenge der in der Datei gespeicherten Struktur
        rSt >> cbStshi;

    UINT16 nRead = cbStshi;
    do
    {
        UINT16 a16Bit;

        if(  2 > nRead ) break;
        rSt >> cstd;

        if(  4 > nRead ) break;
        rSt >> cbSTDBaseInFile;

        if(  6 > nRead ) break;
        rSt >> a16Bit;
        fStdStylenamesWritten = a16Bit & 0x0001;

        if(  8 > nRead ) break;
        rSt >> stiMaxWhenSaved;

        if( 10 > nRead ) break;
        rSt >> istdMaxFixedWhenSaved;

        if( 12 > nRead ) break;
        rSt >> nVerBuiltInNamesWhenSaved;

        if( 14 > nRead ) break;
        rSt >> ftcStandardChpStsh;

        // ggfs. den Rest ueberlesen
        if( 14 < nRead )
            rSt.SeekRel( nRead-14 );
    }
    while( !this ); // Trick: obiger Block wird genau einmal durchlaufen
                                    //   und kann vorzeitig per "break" verlassen werden.

    if( 0 != rSt.GetError() )
    {
        // wie denn nun den Error melden?
    }
}


// Read1STDFixed() liest ein Style ein. Wenn der Style vollstaendig vorhanden
// ist, d.h. kein leerer Slot, dann wird Speicher alloziert und ein Pointer auf
// die ( evtl. mit Nullen aufgefuellten ) STD geliefert. Ist es ein leerer
// Slot, dann wird ein Nullpointer zurueckgeliefert.
WW8_STD* WW8Style::Read1STDFixed( short& rSkip, short* pcbStd )
{
    WW8_STD* pStd = (WW8_STD*)0;

    UINT16 cbStd;
    rSt >> cbStd;   // lies Laenge

    UINT16 nRead = cbSTDBaseInFile;
    if( cbStd >= cbSTDBaseInFile ){ // Fixed part vollst. vorhanden

        // read fixed part of STD
        pStd = new WW8_STD;
        memset( pStd, 0, sizeof( *pStd ) );

        do
        {
            UINT16 a16Bit;

            if( 2 > nRead ) break;
            rSt >> a16Bit;
            pStd->sti          =        a16Bit & 0x0fff  ;
            pStd->fScratch     = 0 != ( a16Bit & 0x1000 );
            pStd->fInvalHeight = 0 != ( a16Bit & 0x2000 );
            pStd->fHasUpe      = 0 != ( a16Bit & 0x4000 );
            pStd->fMassCopy    = 0 != ( a16Bit & 0x8000 );

            if( 4 > nRead ) break;
            rSt >> a16Bit;
            pStd->sgc      =   a16Bit & 0x000f       ;
            pStd->istdBase = ( a16Bit & 0xfff0 ) >> 4;

            if( 6 > nRead ) break;
            rSt >> a16Bit;
            pStd->cupx     =   a16Bit & 0x000f       ;
            pStd->istdNext = ( a16Bit & 0xfff0 ) >> 4;

            if( 8 > nRead ) break;
            rSt >> pStd->bchUpe;

            // ab Ver8 sollten diese beiden Felder dazukommen:
            if(10 > nRead ) break;
            rSt >> a16Bit;
            pStd->fAutoRedef =   a16Bit & 0x0001       ;
            pStd->fHidden    = ( a16Bit & 0x0002 ) >> 2;

            // man kann nie wissen: vorsichtshalber ueberlesen
            // wir eventuelle Fuellsel, die noch zum BASE-Part gehoeren...
            if( 10 < nRead )
                rSt.SeekRel( nRead-10 );
        }
        while( !this ); // Trick: obiger Block wird genau einmal durchlaufen
                                        //   und kann vorzeitig per "break" verlassen werden.

        if(    (0 != rSt.GetError())
            || !nRead )
            DELETEZ( pStd );        // per NULL den Error melden

      rSkip = cbStd - cbSTDBaseInFile;
    }
    else
    {           // Fixed part zu kurz
        if( cbStd )
            rSt.SeekRel( cbStd );           // ueberlies Reste
        rSkip = 0;
    }
    if( pcbStd )
        *pcbStd = cbStd;
    return pStd;
}

WW8_STD* WW8Style::Read1Style( short& rSkip, String* pString, short* pcbStd )
{
    // OS2, or WIN with Mac-Doc,...
    // Attention: MacWord-Documents have their Stylenames
    // always in ANSI, even if eStructCharSet == CHARSET_MAC !!

    WW8_STD* pStd;

    pStd = Read1STDFixed( rSkip, pcbStd );          // lese STD

    // String gewuenscht ?
    if( pString )
    {   // echter Style ?
        if ( pStd )
        {

            switch( rFib.nVersion ) // beachte: 6 steht fuer "6 ODER 7",  7 steht fuer "NUR 7"
            {
            case 6:
            case 7: // lies Pascal-String
                            *pString = WW8ReadPString( rSt,
                                                    RTL_TEXTENCODING_MS_1252 );
                                        // leading len and trailing zero --> 2
                            rSkip   -= 2+ pString->Len();
                            break;
            case 8: // lies Unicode-String mit fuehrendem Laengenbyte and trailing zero
                            *pString = WW8Read_xstz( rSt, 0, TRUE );
                            rSkip   -= (pString->Len() + 2) * 2;
                            break;
            default:ASSERT( !this, "Es wurde vergessen, nVersion zu kodieren!" );
            }

        }
        else
            *pString = String();    // Kann keinen Namen liefern
    }
    return pStd;
}


//-----------------------------------------


struct WW8_FFN_Ver6 : public WW8_FFN_BASE
{
    // ab Ver6
    sal_Char szFfn[65]; // 0x6 bzw. 0x40 ab Ver8 zero terminated string that
                        // records name of font.
                        // Maximal size of szFfn is 65 characters.
                        // Vorsicht: Dieses Array kann auch kleiner sein!!!
                        // Possibly followed by a second sz which records the
                        // name of an alternate font to use if the first named
                        // font does not exist on this system.
};
struct WW8_FFN_Ver8 : public WW8_FFN_BASE
{
    // ab Ver8 sind folgende beiden Felder eingeschoben,
    // werden von uns ignoriert.
    sal_Char panose[ 10 ];  //  0x6   PANOSE
    sal_Char fs[ 24     ];  //  0x10  FONTSIGNATURE

    // ab Ver8 als Unicode
    UINT16 szFfn[65];// 0x6 bzw. 0x40 ab Ver8 zero terminated string that
                                        // records name of font.
                                        // Maximal size of szFfn is 65 characters.
                                        // Vorsicht: Dieses Array kann auch kleiner sein!!!
                                        // Possibly followed by a second sz which records the
                                        // name of an alternate font to use if the first named
                                        // font does not exist on this system.
};


WW8Fonts::WW8Fonts( SvStream& rSt, WW8Fib& rFib )
{
    // OS2, or WIN with Mac-Doc,...
    // Attention: MacWord-Documents have their Fontnames
    // always in ANSI, even if eStructCharSet == CHARSET_MAC !!
    if( rFib.lcbSttbfffn <= 2 )
    {
        ASSERT( !this, "Fonttabelle kaputt! (rFib.lcbSttbfffn < 2)" );
        pFontA = 0;
        nMax = 0;
        return;
    }

    BOOL bVer67 = (8 > rFib.nVersion);

    rSt.Seek( rFib.fcSttbfffn );

    // allocate Font Array
    BYTE* pA   = new BYTE[ rFib.lcbSttbfffn - 2 ];
    WW8_FFN* p = (WW8_FFN*)pA;

    if( !bVer67 )
    {
        // bVer8: read the count of strings in nMax
        rSt >> nMax;
    }

    // Ver8:  skip undefined uint16
    // Ver67: skip the herein stored total byte of structure
    //        - we already got that information in rFib.lcbSttbfffn
    rSt.SeekRel( 2 );

    // read all font information
    rSt.Read( pA, rFib.lcbSttbfffn - 2 );

    if( bVer67 )
    {
        // try to figure out how many fonts are defined here
        nMax = 0;
        long nLeft = rFib.lcbSttbfffn - 2;
        for(;;)
        {
            short nNextSiz;

            nNextSiz = p->cbFfnM1 + 1;
            if( nNextSiz > nLeft )
                break;
            nMax++;
            nLeft -= nNextSiz;
            if( nLeft < 1 )     // can we read the given ammount of bytes ?
                break;
            // increase p by nNextSiz Bytes
            p = (WW8_FFN *)( ( (BYTE*)p ) + nNextSiz );
        }
    }

    if( nMax )
    {
        // allocate Index Array
        pFontA = new WW8_FFN[ nMax ];
        p = pFontA;

        if( bVer67 )
        {
            WW8_FFN_Ver6* pVer6 = (WW8_FFN_Ver6*)pA;
            for(USHORT i=0; i<nMax; ++i, ++p)
            {
                p->cbFfnM1   = pVer6->cbFfnM1;
                p->prg       = pVer6->prg;
                p->fTrueType = pVer6->fTrueType;
                p->ff        = pVer6->ff;
                p->wWeight   = SVBT16ToShort( *(SVBT16*)&pVer6->wWeight );
                p->chs       = pVer6->chs;
                p->ibszAlt   = pVer6->ibszAlt;
                p->sFontname = String( pVer6->szFfn, RTL_TEXTENCODING_MS_1252 );
                pVer6 = (WW8_FFN_Ver6*)(  ((BYTE*)pVer6)
                                        + pVer6->cbFfnM1 + 1 );
            }
        }
        else
        {
            WW8_FFN_Ver8* pVer8 = (WW8_FFN_Ver8*)pA;
            BYTE c2;
            for(USHORT i=0; i<nMax; ++i, ++p)
            {
                p->cbFfnM1   = pVer8->cbFfnM1;
                c2           = *(((BYTE*)pVer8) + 1);

                p->prg       =  c2 & 0x02;
                p->fTrueType = (c2 & 0x04) >> 2;
                // ein Reserve-Bit ueberspringen
                p->ff        = (c2 & 0x70) >> 4;

                p->wWeight   = SVBT16ToShort( *(SVBT16*)&pVer8->wWeight );
                p->chs       = pVer8->chs;
                p->ibszAlt   = pVer8->ibszAlt;

#ifdef __WW8_NEEDS_COPY
                {
                    for(UINT16* pTmp = pVer8->szFfn; *pTmp; ++pTmp )
                        *pTmp = SVBT16ToShort( *(SVBT16*)pTmp );
                }
#endif // defined __WW8_NEEDS_COPY

                p->sFontname = pVer8->szFfn;

                // Zeiger auf Ursprungsarray einen Font nach hinten setzen
                pVer8 = (WW8_FFN_Ver8*)( ( (BYTE*)pVer8 ) +  pVer8->cbFfnM1 + 1 );
            }
        }
    }
    delete pA;
}

const WW8_FFN* WW8Fonts::GetFont( USHORT nNum ) const
{
    if( !pFontA || nNum > nMax )
        return 0;

    return &pFontA[ nNum ];
}



//-----------------------------------------


// Suche zu einem Header / Footer den Index in der WW-Liste von Headern / Footern
//
// Pferdefuesse bei WinWord6 und -7:
// 1) Am Anfang des Einlesens muss WWPLCF_HdFt mit Fib und Dop konstruiert werden
// 2) Der Haupttext muss sequentiell ueber alle Sections gelesen werden
// 3) Fuer jedes vorkommende Header / Footer - Attribut des Haupttextes
//  ( Darf pro Section maximal eins sein ) muss UpdateIndex() genau einmal
//  mit dem Parameter des Attributes gerufen werden. Dieser Aufruf muss *nach*
//  dem letzten Aufruf von GetTextPos() passieren.
// 4) GetTextPos() darf mit genau einem der obenstehen WW_... aufgerufen werden
//   ( nicht verodern ! )
// -> dann liefert GetTextPos() vielleicht auch ein richtiges Ergebnis

WW8PLCF_HdFt::WW8PLCF_HdFt( SvStream* pSt, WW8Fib& rFib, WW8Dop& rDop )
: aPLCF( pSt, rFib.fcPlcfhdd , rFib.lcbPlcfhdd , 0 )
{
    nIdxOffset = 0;

//  for( BYTE nI = 0x80; nI >= 0x20; nI >>= 1 ) // Beschreibung stimmt nicht

//  for( BYTE nI = 0x1; nI <= 0x4; nI <<= 1 )   // Vorsicht: Beschreibung stimmt nicht


     /*
      cmc 23/02/2000: This dop.grpfIhdt has a bit set for each special
      footnote *and endnote!!* seperator,continuation seperator, and
      continuation notice entry, the documentation does not mention the
      endnote seperators, the documentation also gets the index numbers
      backwards when specifiying which bits to test. The bottom six bits
      of this value must be tested and skipped over. Each section's
      grpfIhdt is then tested for the existence of the appropiate headers
      and footers, at the end of each section the nIdxOffset must be updated
      to point to the beginning of the next section's group of headers and
      footers in this PLCF, UpdateIndex does that task.
      */
    for( BYTE nI = 0x1; nI <= 0x20; nI <<= 1 )
        if( nI & rDop.grpfIhdt )                // Bit gesetzt ?
            nIdxOffset++;

    nTextOfs = rFib.ccpText + rFib.ccpFtn; // Groesse des Haupttextes
                                                        // und der Fussnoten
}

BOOL WW8PLCF_HdFt::GetTextPos( BYTE grpfIhdt, BYTE nWhich, WW8_CP& rStart, long& rLen )
{
    BYTE nI = 0x1;
    short nIdx = nIdxOffset;
    while( TRUE ){
        if( nI & nWhich )
            break;                  // gefunden
        if( grpfIhdt & nI )
            nIdx++;                 // uninteressanter Header / Footer
        nI <<= 1;                   // naechstes Bit testen
        if( nI > 0x20 )
            return FALSE;       // nicht gefunden
    }
                                            // nIdx ist HdFt-Index
    WW8_CP ende;
    void* pData;

    aPLCF.SetIdx( nIdx );               // dann suche passenden CP
    aPLCF.Get( rStart, ende, pData );
    rLen = ende - rStart;
// Der letzte Header/Footer hat eine Laenge, die um 1 Zeichen zu gross ist
// ( folgendes CR ). Um diesen passend zu kuerzen muesste man den Index
// dieses K/F-Textes finden ( es ist nicht der letzte im PLCF ! ).
//  if( (long)aPLCF.GetIdx() == aPLCF.GetIMax() )   // Falsch!
//      rLen--;                         // Letztes Element ist 1 zu gross
    aPLCF++;
    return TRUE;
}

BOOL WW8PLCF_HdFt::GetTextPosExact( short nIdx, WW8_CP& rStart, long& rLen )
{
    WW8_CP ende;
    void* pData;

    aPLCF.SetIdx( nIdx );               // dann suche passenden CP
    aPLCF.Get( rStart, ende, pData );
    rLen = ende - rStart;
// Der letzte Header/Footer hat eine Laenge, die um 1 Zeichen zu gross ist
// ( folgendes CR ). Um diesen passend zu kuerzen muesste man den Index
// dieses K/F-Textes finden ( es ist nicht der letzte im PLCF ! ).
//  if( (long)aPLCF.GetIdx() == aPLCF.GetIMax() )   // Falsch!
//      rLen--;                         // Letztes Element ist 1 zu gross
    return TRUE;
}

void WW8PLCF_HdFt::UpdateIndex( BYTE grpfIhdt )
{
    for( BYTE nI = 0x1; nI <= 0x20; nI <<= 1 )  // Vorsicht: Beschreibung stimmt nicht
        if( nI & grpfIhdt )
            nIdxOffset++;
}

//-----------------------------------------
//          WW8Dop
//-----------------------------------------



WW8Dop::WW8Dop( SvStream& rSt, INT16 nFib, INT32 nPos, INT32 nSize )
{
    BYTE* pData = new BYTE[ 500 ];
    BYTE* pDataPtr = pData;
    UINT32 nRead = 500 < nSize ? 500 : nSize;
    rSt.Seek( nPos );
    if( 2 > nSize || nRead != rSt.Read( pData, nRead ))
    {
        memset( &nDataStart, 0, (&nDataEnd - &nDataStart) );
        nDopError = ERR_SWG_READ_ERROR;     // Error melden
    }
    else
    {
        if( 500 > nRead )
            memset( pData + nRead, 0, 500 - nRead );

        // dann mal die Daten auswerten
        UINT32 a32Bit;
        UINT16 a16Bit;
        BYTE   a8Bit;

        a16Bit = Get_UShort( pData );
        fFacingPages        = 0 != ( a16Bit  &  0x0001 )     ;
        fWidowControl       = 0 != ( a16Bit  &  0x0002 )     ;
        fPMHMainDoc         = 0 != ( a16Bit  &  0x0004 )     ;
        grfSuppression      =      ( a16Bit  &  0x0018 ) >> 3;
        fpc                 =      ( a16Bit  &  0x0060 ) >> 5;
        grpfIhdt            =      ( a16Bit  &  0xff00 ) >> 8;

        a16Bit = Get_UShort( pData );
        rncFtn              =   a16Bit  &  0x0003        ;
        nFtn                = ( a16Bit  & ~0x0003 ) >> 2 ;

        a8Bit = Get_Byte( pData );
        fOutlineDirtySave      = 0 != ( a8Bit  &  0x01   );

        a8Bit = Get_Byte( pData );
        fOnlyMacPics           = 0 != ( a8Bit  &  0x01   );
        fOnlyWinPics           = 0 != ( a8Bit  &  0x02   );
        fLabelDoc              = 0 != ( a8Bit  &  0x04   );
        fHyphCapitals          = 0 != ( a8Bit  &  0x08   );
        fAutoHyphen            = 0 != ( a8Bit  &  0x10   );
        fFormNoFields          = 0 != ( a8Bit  &  0x20   );
        fLinkStyles            = 0 != ( a8Bit  &  0x40   );
        fRevMarking            = 0 != ( a8Bit  &  0x80   );

        a8Bit = Get_Byte( pData );
        fBackup                = 0 != ( a8Bit  &  0x01   );
        fExactCWords           = 0 != ( a8Bit  &  0x02   );
        fPagHidden             = 0 != ( a8Bit  &  0x04   );
        fPagResults            = 0 != ( a8Bit  &  0x08   );
        fLockAtn               = 0 != ( a8Bit  &  0x10   );
        fMirrorMargins         = 0 != ( a8Bit  &  0x20   );
        fReadOnlyRecommended   = 0 != ( a8Bit  &  0x40   );
        fDfltTrueType          = 0 != ( a8Bit  &  0x80   );

        a8Bit = Get_Byte( pData );
        fPagSuppressTopSpacing = 0 != ( a8Bit  &  0x01   );
        fProtEnabled           = 0 != ( a8Bit  &  0x02   );
        fDispFormFldSel        = 0 != ( a8Bit  &  0x04   );
        fRMView                = 0 != ( a8Bit  &  0x08   );
        fRMPrint               = 0 != ( a8Bit  &  0x10   );
        fWriteReservation      = 0 != ( a8Bit  &  0x20   );
        fLockRev               = 0 != ( a8Bit  &  0x40   );
        fEmbedFonts            = 0 != ( a8Bit  &  0x80   );


        a8Bit = Get_Byte( pData );
        copts_fNoTabForInd           = 0 != ( a8Bit  &  0x01   );
        copts_fNoSpaceRaiseLower     = 0 != ( a8Bit  &  0x02   );
        copts_fSupressSpbfAfterPgBrk = 0 != ( a8Bit  &  0x04   );
        copts_fWrapTrailSpaces       = 0 != ( a8Bit  &  0x08   );
        copts_fMapPrintTextColor     = 0 != ( a8Bit  &  0x10   );
        copts_fNoColumnBalance       = 0 != ( a8Bit  &  0x20   );
        copts_fConvMailMergeEsc      = 0 != ( a8Bit  &  0x40   );
        copts_fSupressTopSpacing     = 0 != ( a8Bit  &  0x80   );

        a8Bit = Get_Byte( pData );
        copts_fOrigWordTableRules    = 0 != ( a8Bit  &  0x01   );
        copts_fTransparentMetafiles  = 0 != ( a8Bit  &  0x02   );
        copts_fShowBreaksInFrames    = 0 != ( a8Bit  &  0x04   );
        copts_fSwapBordersFacingPgs  = 0 != ( a8Bit  &  0x08   );

        dxaTab = Get_Short( pData );
        wSpare = Get_UShort( pData );
        dxaHotZ = Get_UShort( pData );
        cConsecHypLim = Get_UShort( pData );
        wSpare2 = Get_UShort( pData );
        dttmCreated = Get_Long( pData );
        dttmRevised = Get_Long( pData );
        dttmLastPrint = Get_Long( pData );
        nRevision = Get_Short( pData );
        tmEdited = Get_Long( pData );
        cWords = Get_Long( pData );
        cCh = Get_Long( pData );
        cPg = Get_Short( pData );
        cParas = Get_Long( pData );

        a16Bit = Get_UShort( pData );
        rncEdn =   a16Bit &  0x0003       ;
        nEdn   = ( a16Bit & ~0x0003 ) >> 2;

        a16Bit = Get_UShort( pData );
        epc            =   a16Bit &  0x0003       ;
        nfcFtnRef      = ( a16Bit &  0x003c ) >> 2;
        nfcEdnRef      = ( a16Bit &  0x03c0 ) >> 6;
        fPrintFormData = 0 != ( a16Bit &  0x0400 );
        fSaveFormData  = 0 != ( a16Bit &  0x0800 );
        fShadeFormData = 0 != ( a16Bit &  0x1000 );
        fWCFtnEdn      = 0 != ( a16Bit &  0x8000 );

        cLines = Get_Long( pData );
        cWordsFtnEnd = Get_Long( pData );
        cChFtnEdn = Get_Long( pData );
        cPgFtnEdn = Get_Short( pData );
        cParasFtnEdn = Get_Long( pData );
        cLinesFtnEdn = Get_Long( pData );
        lKeyProtDoc = Get_Long( pData );

        a16Bit = Get_UShort( pData );
        wvkSaved    =   a16Bit &  0x0007        ;
        wScaleSaved = ( a16Bit &  0x0ff8 ) >> 3 ;
        zkSaved     = ( a16Bit &  0x3000 ) >> 12;

        /*
            bei nFib >= 103 gehts weiter:
        */
        if( nFib >= 103 )
        {
            a32Bit = Get_ULong( pData );
            fNoTabForInd                = ( a32Bit &  0x00000001 )       ;
            fNoSpaceRaiseLower          = ( a32Bit &  0x00000002 ) >>  1 ;
            fSupressSpbfAfterPageBreak  = ( a32Bit &  0x00000004 ) >>  2 ;
            fWrapTrailSpaces            = ( a32Bit &  0x00000008 ) >>  3 ;
            fMapPrintTextColor          = ( a32Bit &  0x00000010 ) >>  4 ;
            fNoColumnBalance            = ( a32Bit &  0x00000020 ) >>  5 ;
            fConvMailMergeEsc           = ( a32Bit &  0x00000040 ) >>  6 ;
            fSupressTopSpacing          = ( a32Bit &  0x00000080 ) >>  7 ;
            fOrigWordTableRules         = ( a32Bit &  0x00000100 ) >>  8 ;
            fTransparentMetafiles       = ( a32Bit &  0x00000200 ) >>  9 ;
            fShowBreaksInFrames         = ( a32Bit &  0x00000400 ) >> 10 ;
            fSwapBordersFacingPgs       = ( a32Bit &  0x00000800 ) >> 11 ;
            fSuppressTopSpacingMac5     = ( a32Bit &  0x00010000 ) >> 16 ;
            fTruncDxaExpand             = ( a32Bit &  0x00020000 ) >> 17 ;
            fPrintBodyBeforeHdr         = ( a32Bit &  0x00040000 ) >> 18 ;
            fNoLeading                  = ( a32Bit &  0x00080000 ) >> 19 ;
            fMWSmallCaps                = ( a32Bit &  0x00200000 ) >> 21 ;
        }

        /*
            bei nFib > 105 gehts weiter:
        */
        if( nFib > 105 )
        {
            adt = Get_Short( pData );

            memcpy( &doptypography, pData, sizeof( WW8_DOPTYPOGRAPHY ));
            pData += sizeof( WW8_DOPTYPOGRAPHY );

            memcpy( &dogrid, pData, sizeof( WW8_DOGRID ));
            pData += sizeof( WW8_DOGRID );

            a16Bit = Get_UShort( pData );
            // die untersten 9 Bit sind uninteressant
            fHtmlDoc                = ( a16Bit &  0x0200 ) >>  9 ;
            fSnapBorder             = ( a16Bit &  0x0800 ) >> 11 ;
            fIncludeHeader          = ( a16Bit &  0x1000 ) >> 12 ;
            fIncludeFooter          = ( a16Bit &  0x2000 ) >> 13 ;
            fForcePageSizePag       = ( a16Bit &  0x4000 ) >> 14 ;
            fMinFontSizePag         = ( a16Bit &  0x8000 ) >> 15 ;

            a16Bit = Get_UShort( pData );
            fHaveVersions   = 0 != ( a16Bit  &  0x0001 );
            fAutoVersion    = 0 != ( a16Bit  &  0x0002 );

            pData += 12;

            cChWS = Get_Long( pData );
            cChWSFtnEdn = Get_Long( pData );
            grfDocEvents = Get_Long( pData );

            pData += 4+30+8;

            cDBC = Get_Long( pData );
            cDBCFtnEdn = Get_Long( pData );

            pData += 1 * sizeof( long );

            nfcFtnRef = Get_Short( pData );
            nfcEdnRef = Get_Short( pData );
            hpsZoonFontPag = Get_Short( pData );
            dywDispPag = Get_Short( pData );
        }
    }
    delete pDataPtr;
}

WW8Dop::WW8Dop()
{
    // erstmal alles auf 0 defaulten
    memset( &nDataStart, 0, (&nDataEnd - &nDataStart) );

    fWidowControl = 1;
    fpc = 1;
    nFtn = 1;
    fOutlineDirtySave = 1;
    fHyphCapitals = 1;
    fBackup = 1;
    fPagHidden = 1;
    fPagResults = 1;
    fDfltTrueType = 1;
    fRMView = 1;
    fRMPrint = 1;
    dxaTab = 0x2d0;
    dxaHotZ = 0x168;
    dttmCreated = 0x45FBAC69;
    dttmRevised = 0x45FBAC69;
    nRevision = 1;
    nEdn = 1;

    epc = 3;
    nfcEdnRef = 2;
    fShadeFormData = 1;

    wvkSaved = 2;
    wScaleSaved = 100;
    zkSaved = 0;

    lvl = 9;
    fIncludeHeader = 1;
    fIncludeFooter = 1;

    cChWS = /**!!**/ 0;
    cChWSFtnEdn = /**!!**/ 0;

    cDBC = /**!!**/ 0;
    cDBCFtnEdn = /**!!**/ 0;
}

BOOL WW8Dop::Write( SvStream& rStrm, WW8Fib& rFib )
{
    INT32 nLen = 8 == rFib.nVersion ? 500 : 84;
    rFib.fcDop =  rStrm.Tell();
    rFib.lcbDop = nLen;

    BYTE aData[ 500 ];
    memset( aData, 0, 500 );
    BYTE* pData = aData;

    // dann mal die Daten auswerten
    UINT32 a32Bit;
    UINT16 a16Bit;
    BYTE   a8Bit;

    a16Bit = 0;
    if( fFacingPages )      a16Bit |= 0x0001;
    if( fWidowControl )     a16Bit |= 0x0002;
    if( fPMHMainDoc )       a16Bit |= 0x0004;
    a16Bit |= ( 0x0018 & (grfSuppression << 3));
    a16Bit |= ( 0x0060 & (fpc << 5));
    a16Bit |= ( 0xff00 & (grpfIhdt << 8));
    Set_UInt16( pData, a16Bit );

    a16Bit = 0;
    a16Bit |= ( 0x0003 & rncFtn );
    a16Bit |= ( ~0x0003 & (nFtn << 2));
    Set_UInt16( pData, a16Bit );

    a8Bit = 0;
    if( fOutlineDirtySave ) a8Bit |= 0x01;
    Set_UInt8( pData, a8Bit );

    a8Bit = 0;
    if( fOnlyMacPics )  a8Bit |= 0x01;
    if( fOnlyWinPics )  a8Bit |= 0x02;
    if( fLabelDoc )     a8Bit |= 0x04;
    if( fHyphCapitals ) a8Bit |= 0x08;
    if( fAutoHyphen )   a8Bit |= 0x10;
    if( fFormNoFields ) a8Bit |= 0x20;
    if( fLinkStyles )   a8Bit |= 0x40;
    if( fRevMarking )   a8Bit |= 0x80;
    Set_UInt8( pData, a8Bit );

    a8Bit = 0;
    if( fBackup )               a8Bit |= 0x01;
    if( fExactCWords )          a8Bit |= 0x02;
    if( fPagHidden )            a8Bit |= 0x04;
    if( fPagResults )           a8Bit |= 0x08;
    if( fLockAtn )              a8Bit |= 0x10;
    if( fMirrorMargins )        a8Bit |= 0x20;
    if( fReadOnlyRecommended )  a8Bit |= 0x40;
    if( fDfltTrueType )         a8Bit |= 0x80;
    Set_UInt8( pData, a8Bit );

    a8Bit = 0;
    if( fPagSuppressTopSpacing )    a8Bit |= 0x01;
    if( fProtEnabled )              a8Bit |= 0x02;
    if( fDispFormFldSel )           a8Bit |= 0x04;
    if( fRMView )                   a8Bit |= 0x08;
    if( fRMPrint )                  a8Bit |= 0x10;
    if( fWriteReservation )         a8Bit |= 0x20;
    if( fLockRev )                  a8Bit |= 0x40;
    if( fEmbedFonts )               a8Bit |= 0x80;
    Set_UInt8( pData, a8Bit );


    a8Bit = 0;
    if( copts_fNoTabForInd )            a8Bit |= 0x01;
    if( copts_fNoSpaceRaiseLower )      a8Bit |= 0x02;
    if( copts_fSupressSpbfAfterPgBrk )  a8Bit |= 0x04;
    if( copts_fWrapTrailSpaces )        a8Bit |= 0x08;
    if( copts_fMapPrintTextColor )      a8Bit |= 0x10;
    if( copts_fNoColumnBalance )        a8Bit |= 0x20;
    if( copts_fConvMailMergeEsc )       a8Bit |= 0x40;
    if( copts_fSupressTopSpacing )      a8Bit |= 0x80;
    Set_UInt8( pData, a8Bit );

    a8Bit = 0;
    if( copts_fOrigWordTableRules )     a8Bit |= 0x01;
    if( copts_fTransparentMetafiles )   a8Bit |= 0x02;
    if( copts_fShowBreaksInFrames )     a8Bit |= 0x04;
    if( copts_fSwapBordersFacingPgs )   a8Bit |= 0x08;
    Set_UInt8( pData, a8Bit );

    Set_UInt16( pData, dxaTab );
    Set_UInt16( pData, wSpare );
    Set_UInt16( pData, dxaHotZ );
    Set_UInt16( pData, cConsecHypLim );
    Set_UInt16( pData, wSpare2 );
    Set_UInt32( pData, dttmCreated );
    Set_UInt32( pData, dttmRevised );
    Set_UInt32( pData, dttmLastPrint );
    Set_UInt16( pData, nRevision );
    Set_UInt32( pData, tmEdited );
    Set_UInt32( pData, cWords );
    Set_UInt32( pData, cCh );
    Set_UInt16( pData, cPg );
    Set_UInt32( pData, cParas );

    a16Bit = 0;
    a16Bit |= ( 0x0003 & rncEdn );
    a16Bit |= (~0x0003 & ( nEdn << 2));
    Set_UInt16( pData, a16Bit );

    a16Bit = 0;
    a16Bit |= (0x0003 & epc );
    a16Bit |= (0x003c & (nfcFtnRef << 2));
    a16Bit |= (0x03c0 & (nfcEdnRef << 6));
    if( fPrintFormData )    a16Bit |= 0x0400;
    if( fSaveFormData )     a16Bit |= 0x0800;
    if( fShadeFormData )    a16Bit |= 0x1000;
    if( fWCFtnEdn )         a16Bit |= 0x8000;
    Set_UInt16( pData, a16Bit );

    Set_UInt32( pData, cLines );
    Set_UInt32( pData, cWordsFtnEnd );
    Set_UInt32( pData, cChFtnEdn );
    Set_UInt16( pData, cPgFtnEdn );
    Set_UInt32( pData, cParasFtnEdn );
    Set_UInt32( pData, cLinesFtnEdn );
    Set_UInt32( pData, lKeyProtDoc );

    a16Bit = 0;
    if( wvkSaved )  a16Bit |= 0x0007;
    a16Bit |= (0x0ff8 & (wScaleSaved << 3));
    a16Bit |= (0x3000 & (zkSaved << 12));
    Set_UInt16( pData, a16Bit );

    if( 8 == rFib.nVersion )
    {
        a32Bit = 0;
        if( fNoTabForInd )                  a32Bit |= 0x00000001;
        if( fNoSpaceRaiseLower )            a32Bit |= 0x00000002;
        if( fSupressSpbfAfterPageBreak )    a32Bit |= 0x00000004;
        if( fWrapTrailSpaces )              a32Bit |= 0x00000008;
        if( fMapPrintTextColor )            a32Bit |= 0x00000010;
        if( fNoColumnBalance )              a32Bit |= 0x00000020;
        if( fConvMailMergeEsc )             a32Bit |= 0x00000040;
        if( fSupressTopSpacing )            a32Bit |= 0x00000080;
        if( fOrigWordTableRules )           a32Bit |= 0x00000100;
        if( fTransparentMetafiles )         a32Bit |= 0x00000200;
        if( fShowBreaksInFrames )           a32Bit |= 0x00000400;
        if( fSwapBordersFacingPgs )         a32Bit |= 0x00000800;
        if( fSuppressTopSpacingMac5 )       a32Bit |= 0x00010000;
        if( fTruncDxaExpand )               a32Bit |= 0x00020000;
        if( fPrintBodyBeforeHdr )           a32Bit |= 0x00040000;
        if( fNoLeading )                    a32Bit |= 0x00080000;
        if( fMWSmallCaps )                  a32Bit |= 0x00200000;
        Set_UInt32( pData, a32Bit );

        Set_UInt16( pData, adt );

        memcpy( pData, &doptypography, sizeof( WW8_DOPTYPOGRAPHY ));
        pData += sizeof( WW8_DOPTYPOGRAPHY );

        memcpy( pData, &dogrid, sizeof( WW8_DOGRID ));
        pData += sizeof( WW8_DOGRID );

        a16Bit = 0x12;      // lvl auf 9 setzen
        if( fHtmlDoc )          a16Bit |= 0x0200;
        if( fSnapBorder )       a16Bit |= 0x0800;
        if( fIncludeHeader )    a16Bit |= 0x1000;
        if( fIncludeFooter )    a16Bit |= 0x2000;
        if( fForcePageSizePag ) a16Bit |= 0x4000;
        if( fMinFontSizePag )   a16Bit |= 0x8000;
        Set_UInt16( pData, a16Bit );

        a16Bit = 0;
        if( fHaveVersions ) a16Bit |= 0x0001;
        if( fAutoVersion )  a16Bit |= 0x0002;
        Set_UInt16( pData, a16Bit );

        pData += 12;

        Set_UInt32( pData, cChWS );
        Set_UInt32( pData, cChWSFtnEdn );
        Set_UInt32( pData, grfDocEvents );

        pData += 4+30+8;

        Set_UInt32( pData, cDBC );
        Set_UInt32( pData, cDBCFtnEdn );

        pData += 1 * sizeof( long );

        Set_UInt16( pData, nfcFtnRef );
        Set_UInt16( pData, nfcEdnRef );
        Set_UInt16( pData, hpsZoonFontPag );
        Set_UInt16( pData, dywDispPag );
    }
    rStrm.Write( aData, nLen );
    return 0 == rStrm.GetError();
}

/***************************************************************************
#       Array zum Bestimmen der Laenge der SPRMs
#**************************************************************************/


/*
    Werte fuer SprmInfo.nVari
*/
#define L_FIX  0
#define L_VAR  1
#define L_VAR2 2

struct SprmInfo
{
    USHORT nId;
    BYTE   nLen : 6;
    BYTE   nVari: 2;
};


/*
    Zweck: Laenge und Parameter-Offset eines Sprm erfahren
*/
static SprmInfo aWwSprmTab[] = {
       0, 0, L_FIX, // "Default-sprm",  // wird uebersprungen
       2, 2, L_FIX, // "sprmPIstd",  // pap.istd (style code)
       3, 3, L_VAR, // "sprmPIstdPermute", // pap.istd  permutation
       4, 1, L_FIX, //  "sprmPIncLv1", // pap.istddifference
       5, 1, L_FIX, //  "sprmPJc", // pap.jc (justification)
       6, 1, L_FIX, //  "sprmPFSideBySide", // pap.fSideBySide
       7, 1, L_FIX, //  "sprmPFKeep", // pap.fKeep
       8, 1, L_FIX, //  "sprmPFKeepFollow ", // pap.fKeepFollow
       9, 1, L_FIX, //  "sprmPPageBreakBefore", // pap.fPageBreakBefore

      10, 1, L_FIX, // "sprmPBrcl", //  pap.brcl
      11, 1, L_FIX, // "sprmPBrcp ", // pap.brcp
      12, 0, L_VAR, // "sprmPAnld", //  pap.anld (ANLD structure)
      13, 1, L_FIX, //  "sprmPNLvlAnm", // pap.nLvlAnm nn
      14, 1, L_FIX, //  "sprmPFNoLineNumb", //ap.fNoLnn


      // ???
      15, 0, L_VAR, //   "?sprmPChgTabsPapx", // pap.itbdMac, ...


      16, 2, L_FIX, //  "sprmPDxaRight", // pap.dxaRight
      17, 2, L_FIX, //  "sprmPDxaLeft", // pap.dxaLeft
      18, 2, L_FIX, //  "sprmPNest", // pap.dxaLeft
      19, 2, L_FIX, //  "sprmPDxaLeft1", // pap.dxaLeft1

      20, 4, L_FIX, //  "sprmPDyaLine", // pap.lspd an LSPD
      21, 2, L_FIX, //  "sprmPDyaBefore", // pap.dyaBefore
      22, 2, L_FIX, //  "sprmPDyaAfter", // pap.dyaAfter


      // ???
      23, 0, L_VAR, // "?sprmPChgTabs", // pap.itbdMac, pap.rgdxaTab, ...


      24, 1, L_FIX, //  "sprmPFInTable", // pap.fInTable
      25, 1, L_FIX, //  "sprmPTtp", // pap.fTtp
      26, 2, L_FIX, //  "sprmPDxaAbs", // pap.dxaAbs
      27, 2, L_FIX, //  "sprmPDyaAbs", // pap.dyaAbs
      28, 2, L_FIX, // "sprmPDxaWidth", // pap.dxaWidth
      29, 1, L_FIX, // "sprmPPc", // pap.pcHorz, pap.pcVert

      30, 2, L_FIX, // "sprmPBrcTop10", // pap.brcTop BRC10
      31, 2, L_FIX, // "sprmPBrcLeft10", // pap.brcLeft BRC10
      32, 2, L_FIX, // "sprmPBrcBottom10", // pap.brcBottom BRC10
      33, 2, L_FIX, // "sprmPBrcRight10", // pap.brcRight BRC10
      34, 2, L_FIX, // "sprmPBrcBetween10", // pap.brcBetween BRC10
      35, 2, L_FIX, // "sprmPBrcBar10", // pap.brcBar BRC10
      36, 2, L_FIX, // "sprmPFromText10", // pap.dxaFromText dxa
      37, 1, L_FIX, // "sprmPWr", // pap.wr wr
      38, 2, L_FIX, // "sprmPBrcTop", // pap.brcTop BRC
      39, 2, L_FIX, // "sprmPBrcLeft", // pap.brcLeft BRC

      40, 2, L_FIX, // "sprmPBrcBottom", // pap.brcBottom BRC
      41, 2, L_FIX, // "sprmPBrcRight", // pap.brcRight BRC
      42, 2, L_FIX, // "sprmPBrcBetween", // pap.brcBetween BRC
      43, 2, L_FIX, // "sprmPBrcBar",//pap.brcBar BRC word
      44, 1, L_FIX, //  "sprmPFNoAutoHyph",//pap.fNoAutoHyph
      45, 2, L_FIX, //  "sprmPWHeightAbs",//pap.wHeightAbs w
      46, 2, L_FIX, // "sprmPDcs",//pap.dcs DCS
      47, 2, L_FIX, // "sprmPShd",//pap.shd SHD
      48, 2, L_FIX, //  "sprmPDyaFromText",//pap.dyaFromText dya
      49, 2, L_FIX, //  "sprmPDxaFromText",//pap.dxaFromText dxa

      50, 1, L_FIX, //  "sprmPFLocked", // pap.fLocked 0 or 1 byte

      51, 1, L_FIX, //  "sprmPFWidowControl", // pap.fWidowControl 0 or 1 byte


      // ???
    52, 0, L_FIX, // "?sprmPRuler 52", //???


      65, 1, L_FIX, //  "sprmCFStrikeRM", // chp.fRMarkDel 1 or 0 bit
      66, 1, L_FIX, //  "sprmCFRMark", // chp.fRMark 1 or 0 bit
      67, 1, L_FIX, //  "sprmCFFldVanish", // chp.fFldVanish 1 or 0 bit
      68, 0, L_VAR, // "sprmCPicLocation", // chp.fcPic and chp.fSpec
      69, 2, L_FIX, // "sprmCIbstRMark", // chp.ibstRMark index into sttbRMark

      70, 4, L_FIX, // "sprmCDttmRMark", // chp.dttm DTTM long
      71, 1, L_FIX, //  "sprmCFData", // chp.fData 1 or 0 bit
      72, 2, L_FIX, // "sprmCRMReason", // chp.idslRMReason an index to a table
      73, 3, L_FIX, // "sprmCChse", // chp.fChsDiff and chp.chse see below 3 bytes
      74, 0, L_VAR, // "sprmCSymbol", // chp.fSpec, chp.chSym and chp.ftcSym
      75, 1, L_FIX, //  "sprmCFOle2", // chp.fOle2 1 or 0   bit

      80, 2, L_FIX, // "sprmCIstd", // chp.istd istd, see stylesheet definition short
      81, 0, L_VAR, // "sprmCIstdPermute", // chp.istd permutation vector (see below)
      82, 0, L_VAR, // "sprmCDefault", // whole CHP (see below) none variable length
      83, 0, L_FIX, // "sprmCPlain", // whole CHP (see below) none 0
      85, 1, L_FIX, // "sprmCFBold", // chp.fBold 0,1, 128, or 129 (see below) byte
      86, 1, L_FIX, //"sprmCFItalic", // chp.fItalic 0,1, 128, or 129 (see below) byte
      87, 1, L_FIX, // "sprmCFStrike", // chp.fStrike 0,1, 128, or 129 (see below) byte
      88, 1, L_FIX, // "sprmCFOutline", // chp.fOutline 0,1, 128, or 129 (see below) byte
      89, 1, L_FIX, // "sprmCFShadow", // chp.fShadow 0,1, 128, or 129 (see below) byte

      90, 1, L_FIX, // "sprmCFSmallCaps", // chp.fSmallCaps 0,1, 128, or 129 (see below) byte
      91, 1, L_FIX, // "sprmCFCaps", // chp.fCaps 0,1, 128, or 129 (see below) byte
      92, 1, L_FIX, // "sprmCFVanish", // chp.fVanish 0,1, 128, or 129 (see below) byte
      93, 2, L_FIX, // "sprmCFtc", // chp.ftc ftc word
      94, 1, L_FIX, // "sprmCKul", // chp.kul kul byte
      95, 3, L_FIX, // "sprmCSizePos", // chp.hps, chp.hpsPos (see below) 3 bytes
      96, 2, L_FIX, //  "sprmCDxaSpace", // chp.dxaSpace dxa word
      97, 2, L_FIX, //  "sprmCLid", // chp.lid LID word
      98, 1, L_FIX, //  "sprmCIco", // chp.ico ico byte
      99, 2, L_FIX, // "sprmCHps", // chp.hps hps !word!


     100, 1, L_FIX, //  "sprmCHpsInc", // chp.hps (see below) byte
     101, 2, L_FIX, //  "sprmCHpsPos", // chp.hpsPos hps !word!
     102, 1, L_FIX, //  "sprmCHpsPosAdj", // chp.hpsPos hps (see below) byte


     // ???
     103, 0, L_VAR, // "?sprmCMajority", // chp.fBold, chp.fItalic, chp.fSmallCaps, ...


     104, 1, L_FIX, //  "sprmCIss", // chp.iss iss byte
     105, 0, L_VAR, // "sprmCHpsNew50", // chp.hps hps variable width, length always recorded as 2
     106, 0, L_VAR, // "sprmCHpsInc1", // chp.hps complex (see below) variable width, length always recorded as 2
     107, 2, L_FIX, // "sprmCHpsKern", // chp.hpsKern hps short
     108, 0, L_VAR, // "sprmCMajority50", // chp.fBold, chp.fItalic, chp.fSmallCaps, chp.fVanish, ...
     109, 2, L_FIX, // "sprmCHpsMul", // chp.hps percentage to grow hps short

     110, 2, L_FIX, // "sprmCCondHyhen", // chp.ysri ysri short
     117, 1, L_FIX, //  "sprmCFSpec", // chp.fSpec  1 or 0 bit
     118, 1, L_FIX, //  "sprmCFObj", // chp.fObj 1 or 0 bit
     119, 1, L_FIX, // "sprmPicBrcl", // pic.brcl brcl (see PIC structure definition) byte

     120,12, L_VAR, // "sprmPicScale", // pic.mx, pic.my, pic.dxaCropleft,
     121, 2, L_FIX, // "sprmPicBrcTop", // pic.brcTop BRC word
     122, 2, L_FIX, // "sprmPicBrcLeft", // pic.brcLeft BRC word
     123, 2, L_FIX, // "sprmPicBrcBottom", // pic.brcBottom BRC word
     124, 2, L_FIX, // "sprmPicBrcRight", // pic.brcRight BRC word

     131, 1, L_FIX, //  "sprmSScnsPgn", // sep.cnsPgn cns byte
     132, 1, L_FIX, // "sprmSiHeadingPgn", // sep.iHeadingPgn heading number level byte
     133, 0, L_VAR, // "sprmSOlstAnm", // sep.olstAnm OLST variable length
     136, 3, L_FIX, // "sprmSDxaColWidth", // sep.rgdxaColWidthSpacing complex (see below) 3 bytes
     137, 3, L_FIX, // "sprmSDxaColSpacing", // sep.rgdxaColWidthSpacing complex (see below) 3 bytes
     138, 1, L_FIX, //  "sprmSFEvenlySpaced", // sep.fEvenlySpaced 1 or 0 byte
     139, 1, L_FIX, //  "sprmSFProtected", // sep.fUnlocked 1 or 0 byte

     140, 2, L_FIX, //  "sprmSDmBinFirst", // sep.dmBinFirst  word
     141, 2, L_FIX, //  "sprmSDmBinOther", // sep.dmBinOther  word
     142, 1, L_FIX, //  "sprmSBkc", // sep.bkc bkc byte
     143, 1, L_FIX, //  "sprmSFTitlePage", // sep.fTitlePage 0 or 1 byte
     144, 2, L_FIX, //  "sprmSCcolumns", // sep.ccolM1 # of cols - 1 word
     145, 2, L_FIX, //  "sprmSDxaColumns", // sep.dxaColumns dxa word
     146, 1, L_FIX, // "sprmSFAutoPgn", // sep.fAutoPgn obsolete byte
     147, 1, L_FIX, //  "sprmSNfcPgn", // sep.nfcPgn nfc byte
     148, 2, L_FIX, // "sprmSDyaPgn", // sep.dyaPgn dya short
     149, 2, L_FIX, // "sprmSDxaPgn", // sep.dxaPgn dya short


     150, 1, L_FIX, //  "sprmSFPgnRestart", // sep.fPgnRestart 0 or 1 byte
     151, 1, L_FIX, //  "sprmSFEndnote", // sep.fEndnote 0 or 1 byte
     152, 1, L_FIX, //  "sprmSLnc", // sep.lnc lnc byte
     153, 1, L_FIX, // "sprmSGprfIhdt", // sep.grpfIhdt grpfihdt (see Headers and Footers topic) byte
     154, 2, L_FIX, //  "sprmSNLnnMod", // sep.nLnnMod non-neg int. word
     155, 2, L_FIX, // "sprmSDxaLnn", // sep.dxaLnn dxa word
     156, 2, L_FIX, //  "sprmSDyaHdrTop", // sep.dyaHdrTop dya word
     157, 2, L_FIX, //  "sprmSDyaHdrBottom", // sep.dyaHdrBottom dya word
     158, 1, L_FIX, //  "sprmSLBetween", // sep.fLBetween 0 or 1 byte
     159, 1, L_FIX, //  "sprmSVjc", // sep.vjc vjc byte

     160, 2, L_FIX, //  "sprmSLnnMin", // sep.lnnMin lnn word
     161, 2, L_FIX, //  "sprmSPgnStart", // sep.pgnStart pgn word
     162, 1, L_FIX, //  "sprmSBOrientation", // sep.dmOrientPage dm byte
     163, 0, L_FIX, //  "?SprmSBCustomize 163", //???
     164, 2, L_FIX, //  "sprmSXaPage", // sep.xaPage xa word
     165, 2, L_FIX, //  "sprmSYaPage", // sep.yaPage ya word
     166, 2, L_FIX, //  "sprmSDxaLeft", // sep.dxaLeft dxa word
     167, 2, L_FIX, //  "sprmSDxaRight", // sep.dxaRight dxa word
     168, 2, L_FIX, //  "sprmSDyaTop", // sep.dyaTop dya word
     169, 2, L_FIX, //  "sprmSDyaBottom", // sep.dyaBottom dya word

     170, 2, L_FIX, //  "sprmSDzaGutter", // sep.dzaGutter dza word
     171, 2, L_FIX, //  "sprmSDMPaperReq", // sep.dmPaperReq dm word

     182, 2, L_FIX, //  "sprmTJc", // tap.jc jc word (low order byte is significant)
     183, 2, L_FIX, //  "sprmTDxaLeft", // tap.rgdxaCenter (see below) dxa word
     184, 2, L_FIX, //  "sprmTDxaGapHalf", // tap.dxaGapHalf, tap.rgdxaCenter (see below) dxa word
     185, 1, L_FIX, //  "sprmTFCantSplit", // tap.fCantSplit 1 or 0 byte
     186, 1, L_FIX, //  "sprmTTableHeader", // tap.fTableHeader 1 or 0 byte
     187,12, L_FIX, // "sprmTTableBorders", // tap.rgbrcTable complex(see below) 12 bytes
     188, 0, L_VAR, // "sprmTDefTable10", // tap.rgdxaCenter, tap.rgtc complex (see below) variable length
     189, 2, L_FIX, //  "sprmTDyaRowHeight", // tap.dyaRowHeight dya word

     190, 0, L_VAR2, // "sprmTDefTable", // tap.rgtc complex (see below)
     191, 1, L_VAR, // "sprmTDefTableShd", // tap.rgshd complex (see below)
     192, 4, L_FIX, // "sprmTTlp", // tap.tlp TLP 4 bytes
     193, 5, L_FIX, // "sprmTSetBrc", // tap.rgtc[].rgbrc complex (see below) 5 bytes
     194, 4, L_FIX, // "sprmTInsert", // tap.rgdxaCenter,tap.rgtc complex (see below) 4 bytes
     195, 2, L_FIX, // "sprmTDelete", // tap.rgdxaCenter, tap.rgtc complex (see below) word
     196, 4, L_FIX, // "sprmTDxaCol", // tap.rgdxaCenter complex (see below) 4 bytes
     197, 2, L_FIX, // "sprmTMerge", // tap.fFirstMerged, tap.fMerged complex (see below) word
     198, 2, L_FIX, // "sprmTSplit", // tap.fFirstMerged, tap.fMerged complex (see below) word
     199, 5, L_FIX, // "sprmTSetBrc10", // tap.rgtc[].rgbrc complex (see below) 5 bytes

     200, 4, L_FIX, // "sprmTSetShd", // tap.rgshd complex (see below) 4 bytes

//- neue ab Ver8 ------------------------------------------------------------
    /*
        Die neuen Kodes werden einfach hier an die Liste angehaengt.
        So kann ein und dieselbe Liste fuer Ver6 und Ver8 gemeinsam verwendet werden.
        MS hat gluecklicherweise darauf geachtet, fuer Ver8 einen anderen Nummernkreis
        zu nehmen als fuer Ver6-7.
    */
    0x4600, 2, L_FIX, // "sprmPIstd" pap.istd;istd (style code);short;
    0xC601, 0, L_VAR, // "sprmPIstdPermute" pap.istd;permutation vector (see below);variable length;
    0x2602, 1, L_FIX, // "sprmPIncLvl" pap.istd, pap.lvl;difference between istd of base PAP and istd of PAP to be produced (see below);byte;
    0x2403, 1, L_FIX, // "sprmPJc" pap.jc;jc (justification);byte;
    0x2404, 1, L_FIX, // "sprmPFSideBySide" pap.fSideBySide;0 or 1;byte;
    0x2405, 1, L_FIX, // "sprmPFKeep" pap.fKeep;0 or 1;byte;
    0x2406, 1, L_FIX, // "sprmPFKeepFollow" pap.fKeepFollow;0 or 1;byte;
    0x2407, 1, L_FIX, // "sprmPFPageBreakBefore" pap.fPageBreakBefore;0 or 1;byte;
    0x2408, 1, L_FIX, // "sprmPBrcl" pap.brcl;brcl;byte;
    0x2409, 1, L_FIX, // "sprmPBrcp" pap.brcp;brcp;byte;
    0x260A, 1, L_FIX, // "sprmPIlvl" pap.ilvl;ilvl;byte;
    0x460B, 2, L_FIX, // "sprmPIlfo" pap.ilfo;ilfo (list index) ;short;
    0x240C, 1, L_FIX, // "sprmPFNoLineNumb" pap.fNoLnn;0 or 1;byte;
    0xC60D, 0, L_VAR, // "sprmPChgTabsPapx" pap.itbdMac, pap.rgdxaTab, pap.rgtbd;complex - see below;variable length
    0x840E, 2, L_FIX, // "sprmPDxaRight" pap.dxaRight;dxa;word;
    0x840F, 2, L_FIX, // "sprmPDxaLeft" pap.dxaLeft;dxa;word;
    0x4610, 2, L_FIX, // "sprmPNest" pap.dxaLeft;dxa-see below;word;
    0x8411, 2, L_FIX, // "sprmPDxaLeft1" pap.dxaLeft1;dxa;word;
    0x6412, 4, L_FIX, // "sprmPDyaLine" pap.lspd;an LSPD, a long word structure consisting of a short of dyaLine followed by a short of fMultLinespace - see below;long;
    0xA413, 2, L_FIX, // "sprmPDyaBefore" pap.dyaBefore;dya;word;
    0xA414, 2, L_FIX, // "sprmPDyaAfter" pap.dyaAfter;dya;word;
    0xC615, 0, L_VAR, // "sprmPChgTabs" pap.itbdMac, pap.rgdxaTab, pap.rgtbd;complex - see below;variable length;
    0x2416, 1, L_FIX, // "sprmPFInTable" pap.fInTable;0 or 1;byte;
    0x2417, 1, L_FIX, // "sprmPFTtp" pap.fTtp;0 or 1;byte;
    0x8418, 2, L_FIX, // "sprmPDxaAbs" pap.dxaAbs;dxa;word;
    0x8419, 2, L_FIX, // "sprmPDyaAbs" pap.dyaAbs;dya;word;
    0x841A, 2, L_FIX, // "sprmPDxaWidth" pap.dxaWidth;dxa;word;
    0x261B, 1, L_FIX, // "sprmPPc" pap.pcHorz, pap.pcVert;complex - see below;byte;
    0x461C, 2, L_FIX, // "sprmPBrcTop10" pap.brcTop;BRC10;word;
    0x461D, 2, L_FIX, // "sprmPBrcLeft10" pap.brcLeft;BRC10;word;
    0x461E, 2, L_FIX, // "sprmPBrcBottom10" pap.brcBottom;BRC10;word;
    0x461F, 2, L_FIX, // "sprmPBrcRight10" pap.brcRight;BRC10;word;
    0x4620, 2, L_FIX, // "sprmPBrcBetween10" pap.brcBetween;BRC10;word;
    0x4621, 2, L_FIX, // "sprmPBrcBar10" pap.brcBar;BRC10;word;
    0x4622, 2, L_FIX, // "sprmPDxaFromText10" pap.dxaFromText;dxa;word;
    0x2423, 1, L_FIX, // "sprmPWr" pap.wr;wr (see description of PAP for definition;byte;
    0x6424, 4, L_FIX, // "sprmPBrcTop" pap.brcTop;BRC;long;
    0x6425, 4, L_FIX, // "sprmPBrcLeft" pap.brcLeft;BRC;long;
    0x6426, 4, L_FIX, // "sprmPBrcBottom" pap.brcBottom;BRC;long;
    0x6427, 4, L_FIX, // "sprmPBrcRight" pap.brcRight;BRC;long;
    0x6428, 4, L_FIX, // "sprmPBrcBetween" pap.brcBetween;BRC;long;
    0x6629, 4, L_FIX, // "sprmPBrcBar" pap.brcBar;BRC;long;
    0x242A, 1, L_FIX, // "sprmPFNoAutoHyph" pap.fNoAutoHyph;0 or 1;byte;
    0x442B, 2, L_FIX, // "sprmPWHeightAbs" pap.wHeightAbs;w;word;
    0x442C, 2, L_FIX, // "sprmPDcs" pap.dcs;DCS;short;
    0x442D, 2, L_FIX, // "sprmPShd" pap.shd;SHD;word;
    0x842E, 2, L_FIX, // "sprmPDyaFromText" pap.dyaFromText;dya;word;
    0x842F, 2, L_FIX, // "sprmPDxaFromText" pap.dxaFromText;dxa;word;
    0x2430, 1, L_FIX, // "sprmPFLocked" pap.fLocked;0 or 1;byte;
    0x2431, 1, L_FIX, // "sprmPFWidowControl" pap.fWidowControl;0 or 1;byte;
    0xC632, 0, L_VAR, // "sprmPRuler" ;;variable length;
    0x2433, 1, L_FIX, // "sprmPFKinsoku" pap.fKinsoku;0 or 1;byte;
    0x2434, 1, L_FIX, // "sprmPFWordWrap" pap.fWordWrap;0 or 1;byte;
    0x2435, 1, L_FIX, // "sprmPFOverflowPunct" pap.fOverflowPunct;0 or 1;byte;
    0x2436, 1, L_FIX, // "sprmPFTopLinePunct" pap.fTopLinePunct;0 or 1;byte;
    0x2437, 1, L_FIX, // "sprmPFAutoSpaceDE" pap.fAutoSpaceDE;0 or 1;byte;
    0x2438, 1, L_FIX, // "sprmPFAutoSpaceDN" pap.fAutoSpaceDN;0 or 1;byte;
    0x4439, 2, L_FIX, // "sprmPWAlignFont" pap.wAlignFont;iFa (see description of PAP for definition);word;
    0x443A, 2, L_FIX, // "sprmPFrameTextFlow" pap.fVertical pap.fBackward pap.fRotateFont;complex (see description of PAP for definition);word;
    0x243B, 1, L_FIX, // "sprmPISnapBaseLine" obsolete: not applicable in Word97 and later versions;;byte;
    0xC63E, 0, L_VAR, // "sprmPAnld" pap.anld;;variable length;
    0xC63F, 0, L_VAR, // "sprmPPropRMark" pap.fPropRMark;complex (see below);variable length;
    0x2640, 1, L_FIX, // "sprmPOutLvl" pap.lvl;has no effect if pap.istd is < 1 or is > 9;byte;
    0x2441, 1, L_FIX, // "sprmPFBiDi" ;;byte;
    0x2443, 1, L_FIX, // "sprmPFNumRMIns" pap.fNumRMIns;1 or 0;bit;
    0x2444, 1, L_FIX, // "sprmPCrLf" ;;byte;
    0xC645, 0, L_VAR, // "sprmPNumRM" pap.numrm;;variable length;
    0x6645, 4, L_FIX, // "sprmPHugePapx" see below;fc in the data stream to locate the huge grpprl (see below);long;
    0x6646, 4, L_FIX, // "sprmPHugePapx" see below;fc in the data stream to locate the huge grpprl (see below);long;
    0x2447, 1, L_FIX, // "sprmPFUsePgsuSettings" pap.fUsePgsuSettings;1 or 0;byte;
    0x2448, 1, L_FIX, // "sprmPFAdjustRight" pap.fAdjustRight;1 or 0;byte;
    0x0800, 1, L_FIX, // "sprmCFRMarkDel" chp.fRMarkDel;1 or 0;bit;
    0x0801, 1, L_FIX, // "sprmCFRMark" chp.fRMark;1 or 0;bit;
    0x0802, 1, L_FIX, // "sprmCFFldVanish" chp.fFldVanish;1 or 0;bit;

// wahrscheinlich falsch:
//0x6A03, 0, L_VAR, // "sprmCPicLocation" chp.fcPic and chp.fSpec;see below;variable length, length recorded is always 4;

// eher so:
    0x6A03, 4, L_FIX, // "sprmCPicLocation" chp.fcPic and chp.fSpec;see below;
                                        // variable length,
                                        // ????????
                                        // length recorded is always 4;
                                        // ======             =========

    0x4804, 2, L_FIX, // "sprmCIbstRMark" chp.ibstRMark;index into sttbRMark;short;
    0x6805, 4, L_FIX, // "sprmCDttmRMark" chp.dttmRMark;DTTM;long;
    0x0806, 1, L_FIX, // "sprmCFData" chp.fData;1 or 0;bit;
    0x4807, 2, L_FIX, // "sprmCIdslRMark" chp.idslRMReason;an index to a table of strings defined in Word 6.0 executables;short;
    0xEA08, 1, L_FIX, // "sprmCChs" chp.fChsDiff and chp.chse;see below;3 bytes;
    0x6A09, 4, L_FIX, // "sprmCSymbol" chp.fSpec, chp.xchSym and chp.ftcSym;see below;variable length, length recorded is always 4;
    0x080A, 1, L_FIX, // "sprmCFOle2" chp.fOle2;1 or 0;bit;
//0x480B, 0, L_FIX, // "sprmCIdCharType" obsolete: not applicable in Word97 and later versions;;;
    0x2A0C, 1, L_FIX, // "sprmCHighlight" chp.fHighlight, chp.icoHighlight;ico (fHighlight is set to 1 iff ico is not 0);byte;
    0x680E, 4, L_FIX, // "sprmCObjLocation" chp.fcObj;FC;long;
//0x2A10, 0, L_FIX, // "sprmCFFtcAsciSymb" ;;;
    0x4A30, 2, L_FIX, // "sprmCIstd" chp.istd;istd, see stylesheet definition;short;
    0xCA31, 0, L_VAR, // "sprmCIstdPermute" chp.istd;permutation vector (see below);variable length;
    0x2A32, 0, L_VAR, // "sprmCDefault" whole CHP (see below);none;variable length;
    0x2A33, 0, L_FIX, // "sprmCPlain" whole CHP (see below);none;0;
//0x2A34, 0, L_FIX, // "sprmCKcd" ;;;
    0x0835, 1, L_FIX, // "sprmCFBold" chp.fBold;0,1, 128, or 129 (see below);byte;
    0x0836, 1, L_FIX, // "sprmCFItalic" chp.fItalic;0,1, 128, or 129 (see below);byte;
    0x0837, 1, L_FIX, // "sprmCFStrike" chp.fStrike;0,1, 128, or 129 (see below);byte;
    0x0838, 1, L_FIX, // "sprmCFOutline" chp.fOutline;0,1, 128, or 129 (see below);byte;
    0x0839, 1, L_FIX, // "sprmCFShadow" chp.fShadow;0,1, 128, or 129 (see below);byte;
    0x083A, 1, L_FIX, // "sprmCFSmallCaps" chp.fSmallCaps;0,1, 128, or 129 (see below);byte;
    0x083B, 1, L_FIX, // "sprmCFCaps" chp.fCaps;0,1, 128, or 129 (see below);byte;
    0x083C, 1, L_FIX, // "sprmCFVanish" chp.fVanish;0,1, 128, or 129 (see below);byte;
    0x4A3D, 2, L_FIX, // "sprmCFtcDefault" ;ftc, only used internally, never stored in file;word;
    0x2A3E, 1, L_FIX, // "sprmCKul" chp.kul;kul;byte;
    0xEA3F, 3, L_FIX, // "sprmCSizePos" chp.hps, chp.hpsPos;(see below);3 bytes;
    0x8840, 2, L_FIX, // "sprmCDxaSpace" chp.dxaSpace;dxa;word;
    0x4A41, 2, L_FIX, // "sprmCLid" ;only used internally never stored;word;
    0x2A42, 1, L_FIX, // "sprmCIco" chp.ico;ico;byte;
    0x4A43, 2, L_FIX, // "sprmCHps" chp.hps;hps;byte; ACHTUNG: ich nehme mal lieber ein: word;
    0x2A44, 1, L_FIX, // "sprmCHpsInc" chp.hps;(see below);byte;
    0x4845, 1, L_FIX, // "sprmCHpsPos" chp.hpsPos;hps;byte;
    0x2A46, 1, L_FIX, // "sprmCHpsPosAdj" chp.hpsPos;hps (see below);byte;
    0xCA47, 0, L_VAR, // "sprmCMajority" chp.fBold, chp.fItalic, chp.fSmallCaps, chp.fVanish, chp.fStrike, chp.fCaps, chp.rgftc, chp.hps, chp.hpsPos, chp.kul, chp.dxaSpace, chp.ico, chp.rglid;complex (see below);variable length, length byte plus size of following grpprl;
    0x2A48, 1, L_FIX, // "sprmCIss" chp.iss;iss;byte;
    0xCA49, 0, L_VAR, // "sprmCHpsNew50" chp.hps;hps;variable width, length always recorded as 2;
    0xCA4A, 0, L_VAR, // "sprmCHpsInc1" chp.hps;complex (see below);variable width, length always recorded as 2;
    0x484B, 2, L_FIX, // "sprmCHpsKern" chp.hpsKern;hps;short;
    0xCA4C, 2, L_FIX, // "sprmCMajority50" chp.fBold, chp.fItalic, chp.fSmallCaps, chp.fVanish, chp.fStrike, chp.fCaps, chp.ftc, chp.hps, chp.hpsPos, chp.kul, chp.dxaSpace, chp.ico,;complex (see below);variable length;
    0x4A4D, 2, L_FIX, // "sprmCHpsMul" chp.hps;percentage to grow hps;short;
    0x484E, 2, L_FIX, // "sprmCYsri" chp.ysri;ysri;short;
    0x4A4F, 2, L_FIX, // "sprmCRgFtc0" chp.rgftc[0];ftc for ASCII text (see below);short;
    0x4A50, 2, L_FIX, // "sprmCRgFtc1" chp.rgftc[1];ftc for Far East text (see below);short;
    0x4A51, 2, L_FIX, // "sprmCRgFtc2" chp.rgftc[2];ftc for non-Far East text (see below);short;
//0x4852, 0, L_FIX, // "sprmCCharScale" ;;;
    0x2A53, 1, L_FIX, // "sprmCFDStrike" chp.fDStrike;;byte;
    0x0854, 1, L_FIX, // "sprmCFImprint" chp.fImprint;1 or 0;bit;
    0x0855, 1, L_FIX, // "sprmCFSpec" chp.fSpec ;1 or 0;bit;
    0x0856, 1, L_FIX, // "sprmCFObj" chp.fObj;1 or 0;bit;
    0xCA57, 0, L_VAR, // "sprmCPropRMark" chp.fPropRMark, chp.ibstPropRMark, chp.dttmPropRMark;Complex (see below);variable length always recorded as 7 bytes;
    0x0858, 1, L_FIX, // "sprmCFEmboss" chp.fEmboss;1 or 0;bit;
    0x2859, 1, L_FIX, // "sprmCSfxText" chp.sfxtText;text animation;byte;
//0x085A, 0, L_FIX, // "sprmCFBiDi" ;;;
//0x085B, 0, L_FIX, // "sprmCFDiacColor" ;;;
//0x085C, 0, L_FIX, // "sprmCFBoldBi" ;;;
//0x085D, 0, L_FIX, // "sprmCFItalicBi" ;;;
//0x4A5E, 0, L_FIX, // "sprmCFtcBi" ;;;
//0x485F, 0, L_FIX, // "sprmCLidBi" ;;;
//0x4A60, 0, L_FIX, // "sprmCIcoBi" ;;;
//0x4A61, 0, L_FIX, // "sprmCHpsBi" ;;;
    0xCA62, 0, L_VAR, // "sprmCDispFldRMark" chp.fDispFldRMark, chp.ibstDispFldRMark, chp.dttmDispFldRMark ;Complex (see below);variable length always recorded as 39 bytes;
    0x4863, 2, L_FIX, // "sprmCIbstRMarkDel" chp.ibstRMarkDel;index into sttbRMark;short;
    0x6864, 4, L_FIX, // "sprmCDttmRMarkDel" chp.dttmRMarkDel;DTTM;long;
    0x6865, 4, L_FIX, // "sprmCBrc" chp.brc;BRC;long;
    0x4866, 2, L_FIX, // "sprmCShd" chp.shd;SHD;short;
    0x4867, 2, L_FIX, // "sprmCIdslRMarkDel" chp.idslRMReasonDel;an index to a table of strings defined in Word 6.0 executables;short;
    0x0868, 1, L_FIX, // "sprmCFUsePgsuSettings" chp.fUsePgsuSettings;1 or 0;bit;
    0x486B, 2, L_FIX, // "sprmCCpg" ;;word;
    0x486D, 2, L_FIX, // "sprmCRgLid0" chp.rglid[0];LID: for non-Far East text;word;
    0x486E, 2, L_FIX, // "sprmCRgLid1" chp.rglid[1];LID: for Far East text;word;
    0x286F, 1, L_FIX, // "sprmCIdctHint" chp.idctHint;IDCT: (see below);byte;
    0x2E00, 1, L_FIX, // "sprmPicBrcl" pic.brcl;brcl (see PIC structure definition);byte;
    0xCE01, 0, L_VAR, // "sprmPicScale" pic.mx, pic.my, pic.dxaCropleft, pic.dyaCropTop pic.dxaCropRight, pic.dyaCropBottom;Complex (see below);length byte plus 12 bytes;
    0x6C02, 4, L_FIX, // "sprmPicBrcTop" pic.brcTop;BRC;long;
    0x6C03, 4, L_FIX, // "sprmPicBrcLeft" pic.brcLeft;BRC;long;
    0x6C04, 4, L_FIX, // "sprmPicBrcBottom" pic.brcBottom;BRC;long;
    0x6C05, 4, L_FIX, // "sprmPicBrcRight" pic.brcRight;BRC;long;
    0x3000, 1, L_FIX, // "sprmScnsPgn" sep.cnsPgn;cns;byte;
    0x3001, 1, L_FIX, // "sprmSiHeadingPgn" sep.iHeadingPgn;heading number level;byte;
    0xD202, 0, L_VAR, // "sprmSOlstAnm" sep.olstAnm;OLST;variable length;
    0xF203, 3, L_FIX, // "sprmSDxaColWidth" sep.rgdxaColWidthSpacing;complex (see below);3 bytes;
    0xF204, 3, L_FIX, // "sprmSDxaColSpacing" sep.rgdxaColWidthSpacing;complex (see below);3 bytes;
    0x3005, 1, L_FIX, // "sprmSFEvenlySpaced" sep.fEvenlySpaced;1 or 0;byte;
    0x3006, 1, L_FIX, // "sprmSFProtected" sep.fUnlocked;1 or 0;byte;
    0x5007, 2, L_FIX, // "sprmSDmBinFirst" sep.dmBinFirst;;word;
    0x5008, 2, L_FIX, // "sprmSDmBinOther" sep.dmBinOther;;word;
    0x3009, 1, L_FIX, // "sprmSBkc" sep.bkc;bkc;byte;
    0x300A, 1, L_FIX, // "sprmSFTitlePage" sep.fTitlePage;0 or 1;byte;
    0x500B, 2, L_FIX, // "sprmSCcolumns" sep.ccolM1;# of cols - 1;word;
    0x900C, 2, L_FIX, // "sprmSDxaColumns" sep.dxaColumns;dxa;word;
    0x300D, 1, L_FIX, // "sprmSFAutoPgn" sep.fAutoPgn;obsolete;byte;
    0x300E, 1, L_FIX, // "sprmSNfcPgn" sep.nfcPgn;nfc;byte;
    0xB00F, 2, L_FIX, // "sprmSDyaPgn" sep.dyaPgn;dya;short;
    0xB010, 2, L_FIX, // "sprmSDxaPgn" sep.dxaPgn;dya;short;
    0x3011, 1, L_FIX, // "sprmSFPgnRestart" sep.fPgnRestart;0 or 1;byte;
    0x3012, 1, L_FIX, // "sprmSFEndnote" sep.fEndnote;0 or 1;byte;
    0x3013, 1, L_FIX, // "sprmSLnc" sep.lnc;lnc;byte;
    0x3014, 1, L_FIX, // "sprmSGprfIhdt" sep.grpfIhdt;grpfihdt (see Headers and Footers topic);byte;
    0x5015, 2, L_FIX, // "sprmSNLnnMod" sep.nLnnMod;non-neg int.;word;
    0x9016, 2, L_FIX, // "sprmSDxaLnn" sep.dxaLnn;dxa;word;
    0xB017, 2, L_FIX, // "sprmSDyaHdrTop" sep.dyaHdrTop;dya;word;
    0xB018, 2, L_FIX, // "sprmSDyaHdrBottom" sep.dyaHdrBottom;dya;word;
    0x3019, 1, L_FIX, // "sprmSLBetween" sep.fLBetween;0 or 1;byte;
    0x301A, 1, L_FIX, // "sprmSVjc" sep.vjc;vjc;byte;
    0x501B, 2, L_FIX, // "sprmSLnnMin" sep.lnnMin;lnn;word;
    0x501C, 2, L_FIX, // "sprmSPgnStart" sep.pgnStart;pgn;word;
    0x301D, 1, L_FIX, // "sprmSBOrientation" sep.dmOrientPage;dm;byte;
//0x301E, 0, L_FIX, // "sprmSBCustomize" ;;;

    0xB01F, 2, L_FIX, // "sprmSXaPage" sep.xaPage;xa;word;
    0xB020, 2, L_FIX, // "sprmSYaPage" sep.yaPage;ya;word;
    0xB021, 2, L_FIX, // "sprmSDxaLeft" sep.dxaLeft;dxa;word;
    0xB022, 2, L_FIX, // "sprmSDxaRight" sep.dxaRight;dxa;word;
    0x9023, 2, L_FIX, // "sprmSDyaTop" sep.dyaTop;dya;word;
    0x9024, 2, L_FIX, // "sprmSDyaBottom" sep.dyaBottom;dya;word;

    0xB025, 2, L_FIX, // "sprmSDzaGutter" sep.dzaGutter;dza;word;
    0x5026, 2, L_FIX, // "sprmSDmPaperReq" sep.dmPaperReq;dm;word;
    0xD227, 0, L_VAR, // "sprmSPropRMark" sep.fPropRMark, sep.ibstPropRMark, sep.dttmPropRMark ;complex (see below);variable length always recorded as 7 bytes;
//0x3228, 0, L_FIX, // "sprmSFBiDi" ;;;
//0x3229, 0, L_FIX, // "sprmSFFacingCol" ;;;
//0x322A, 0, L_FIX, // "sprmSFRTLGutter" ;;;
    0x702B, 4, L_FIX, // "sprmSBrcTop" sep.brcTop;BRC;long;
    0x702C, 4, L_FIX, // "sprmSBrcLeft" sep.brcLeft;BRC;long;
    0x702D, 4, L_FIX, // "sprmSBrcBottom" sep.brcBottom;BRC;long;
    0x702E, 4, L_FIX, // "sprmSBrcRight" sep.brcRight;BRC;long;
    0x522F, 2, L_FIX, // "sprmSPgbProp" sep.pgbProp;;word;
    0x7030, 4, L_FIX, // "sprmSDxtCharSpace" sep.dxtCharSpace;dxt;long;
    0x9031, 2, L_FIX, // "sprmSDyaLinePitch" sep.dyaLinePitch;dya;  WRONG:long;  RIGHT:short;  !!!
//0x5032, 0, L_FIX, // "sprmSClm" ;;;
    0x5033, 2, L_FIX, // "sprmSTextFlow" sep.wTextFlow;complex (see below);short;
    0x5400, 2, L_FIX, // "sprmTJc" tap.jc;jc;word (low order byte is significant);
    0x9601, 2, L_FIX, // "sprmTDxaLeft" tap.rgdxaCenter (see below);dxa;word;
    0x9602, 2, L_FIX, // "sprmTDxaGapHalf" tap.dxaGapHalf, tap.rgdxaCenter (see below);dxa;word;
    0x3403, 1, L_FIX, // "sprmTFCantSplit" tap.fCantSplit;1 or 0;byte;
    0x3404, 1, L_FIX, // "sprmTTableHeader" tap.fTableHeader;1 or 0;byte;
    0xD605, 0, L_VAR, // "sprmTTableBorders" tap.rgbrcTable;complex(see below);24 bytes;
    0xD606, 0, L_VAR, // "sprmTDefTable10" tap.rgdxaCenter, tap.rgtc;complex (see below);variable length;
    0x9407, 2, L_FIX, // "sprmTDyaRowHeight" tap.dyaRowHeight;dya;word;
    0xD608, 0, L_VAR, // "sprmTDefTable" tap.rgtc;complex (see below);;
    0xD609, 0, L_VAR, // "sprmTDefTableShd" tap.rgshd;complex (see below);;
    0x740A, 2, L_FIX, // "sprmTTlp" tap.tlp;TLP;4 bytes;
//0x560B, 0, L_FIX, // "sprmTFBiDi" ;;;
//0x740C, 0, L_FIX, // "sprmTHTMLProps" ;;;
    0xD620, 0, L_VAR, // "sprmTSetBrc" tap.rgtc[].rgbrc;complex (see below);5 bytes;
    0x7621, 4, L_FIX, // "sprmTInsert" tap.rgdxaCenter, tap.rgtc;complex (see below);4 bytes;
    0x5622, 2, L_FIX, // "sprmTDelete" tap.rgdxaCenter, tap.rgtc;complex (see below);word;
    0x7623, 2, L_FIX, // "sprmTDxaCol" tap.rgdxaCenter;complex (see below);4 bytes;
    0x5624, 0, L_VAR, // "sprmTMerge" tap.fFirstMerged, tap.fMerged;complex (see below);word;
    0x5625, 0, L_VAR, // "sprmTSplit" tap.fFirstMerged, tap.fMerged;complex (see below);word;
    0xD626, 0, L_VAR, // "sprmTSetBrc10" tap.rgtc[].rgbrc;complex (see below);5 bytes;
    0x7627, 0, L_VAR, // "sprmTSetShd" tap.rgshd;complex (see below);4 bytes;
    0x7628, 0, L_VAR, // "sprmTSetShdOdd" tap.rgshd;complex (see below);4 bytes;
    0x7629, 0, L_VAR, // "sprmTTextFlow" tap.rgtc[].fVerticaltap.rgtc[].fBackwardtap.rgtc[].fRotateFont;0 or 10 or 10 or 1;word;
//0xD62A, 0, L_FIX, // "sprmTDiagLine" ;;;
    0xD62B, 0, L_VAR, // "sprmTVertMerge" tap.rgtc[].vertMerge;complex (see below);variable length always recorded as 2 bytes;
        0xD62C, 0, L_VAR, // "sprmTVertAlign" tap.rgtc[].vertAlign;complex (see below);variable length always recorded as 3 byte;

};



static int
#if defined( WNT )
 __cdecl
#endif
#if defined( ICC )
 _Optlink
#endif
    CompSprmId( const void *pFirst, const void *pSecond)
{
    return( ((SprmInfo*)pFirst )->nId - ((SprmInfo*)pSecond)->nId );
}


SprmInfo& WW8GetSprmInfo( USHORT nId )
{
    // ggfs. Tab sortieren
    static BOOL bInit = FALSE;
    if( !bInit )
    {
        qsort( (void*)aWwSprmTab,
            sizeof( aWwSprmTab      ) / sizeof (aWwSprmTab[ 0 ]),
            sizeof( aWwSprmTab[ 0 ] ),
            CompSprmId );
        bInit = TRUE;
    }
    // Sprm heraussuchen
    void* pFound;
    SprmInfo aSrch;
    aSrch.nId = nId;
    if( 0 == ( pFound = bsearch( (char *) &aSrch,
                        (void*) aWwSprmTab,
                        sizeof( aWwSprmTab      ) / sizeof (aWwSprmTab[ 0 ]),
                        sizeof( aWwSprmTab[ 0 ] ),
                        CompSprmId )))
    {
        // im Fehlerfall auf Nulltes Element verweisen
        pFound = (void*)aWwSprmTab;
    }
    return *(SprmInfo*) pFound;
}



//-----------------------------------------
//              Sprms
//-----------------------------------------

static short WW8GetSprmSize0( USHORT nId, BYTE* pSprm, BYTE nDelta)
{
    SprmInfo& rSprm = WW8GetSprmInfo( nId );
    short nL = 0;                       // soviel Bytes sind zu lesen

    //sprmPChgTabs
    switch( nId )
    {
    case 23:
    case 0xC615:
        if( SVBT16ToShort( &pSprm[1 + nDelta] ) != 255 )
            nL = SVBT16ToShort( &pSprm[1 + nDelta] ) + rSprm.nLen - 1;
        else
        {
            BYTE nDel = pSprm[2 + nDelta];
            BYTE nIns = pSprm[3 + nDelta + 4 * nDel];

            nL = 2 + 4 * nDel + 3 * nIns;
        }
        break;

    case 0xd608:
        nL = SVBT16ToShort( &pSprm[1 + nDelta] );
        break;

    default:
        switch ( rSprm.nVari )
        {
        case L_FIX: nL = rSprm.nLen;        // Excl. Token
                    break;
        case L_VAR: nL = (short)pSprm[1 + nDelta]// Variable 1-Byte Laenge?
                     + rSprm.nLen;      // Excl. Token + Var-Laengenbyte
                    break;
    //  case L_VAR2: nL = ( *(short*)&pSprm[1 + nDelta] )   // Variable 2-Byte Laenge?
        case L_VAR2: nL = SVBT16ToShort( &pSprm[1 + nDelta] )   // Variable 2-Byte Laenge?
                     + rSprm.nLen - 1;  // Excl. Token + Var-Laengenbyte
                    break;
        }
        break;
    }
    return nL;
}


USHORT WW8CountSprms( BYTE   nVersion,
                      BYTE*  pSp,
                      long   nSprmSiz,
                      const  SvUShortsSort* pIgnoreSprms )
{
    BYTE nDelta = ( 8 > nVersion ) ? 0 : 1;
    USHORT nMySprms = 0;
    for(short i=0; i+1+nDelta < nSprmSiz;  )
    {
        USHORT nSpId = WW8GetSprmId( nVersion, pSp );

        if( !nSpId )
            break;

        short nSpLen = WW8GetSprmSizeBrutto( nVersion, pSp, &nSpId  );
        // erhoehe Zeiger, so dass er auf naechsten Sprm zeigt
        i += nSpLen;
        pSp += nSpLen;

        if( !pIgnoreSprms || USHRT_MAX == pIgnoreSprms->Seek_Entry( nSpId ) )
        {
            ++nMySprms;
        }
    }
    return nMySprms;
}

BOOL WW8PLCFx_SEPX::CompareSprms( BYTE*  pOtherSprms,
                                  long   nOtherSprmSiz,
                                  const  SvUShortsSort* pIgnoreSprms ) const
{
    BOOL bRes = FALSE;
    BYTE* pSp   = pSprms;
    short i;
    BYTE nDelta = ( 8 > GetVersion()) ? 0 : 1;

    if(    WW8CountSprms(GetVersion(), pSp, nSprmSiz, pIgnoreSprms)
        == WW8CountSprms(GetVersion(), pOtherSprms, nOtherSprmSiz, pIgnoreSprms) )
    {
        bRes = TRUE;
        for( i=0; i+1+nDelta < nSprmSiz;    )
        {
            USHORT nSpId = WW8GetSprmId( GetVersion(), pSp );

            if( !nSpId )
                break;

            short nSpLen = WW8GetSprmSizeBrutto( GetVersion(), pSp, &nSpId  );

            if( !pIgnoreSprms || !pIgnoreSprms->Seek_Entry( nSpId ) )
            {
                BYTE* pOtherSp = HasSprm( nSpId, pOtherSprms, nOtherSprmSiz );

                if(    ( !pOtherSp )
                    || (0 != memcmp( pSp + 1 + nDelta + WW8SprmDataOfs( nSpId ),
                                     pOtherSp,
                                     WW8GetSprmSize0( nSpId, pSp, nDelta ) ) ) )
                {
                    bRes = FALSE;
                    break;
                }
            }
            // erhoehe Zeiger, so dass er auf naechsten Sprm zeigt
            i += nSpLen;
            pSp += nSpLen;
        }
    }
    return bRes;    // unterschiedliche Sprms ?
}


// ein bzw. zwei Byte am Anfang des Sprm sind die Id
USHORT WW8GetSprmId( BYTE nVersion, BYTE* pSp, BYTE* pDelta )
{
    USHORT nId = 0;
    if( pSp )
    {
        switch( nVersion )  // 6 steht fuer "6 ODER 7",  7 steht fuer "NUR 7"
        {
            case 6:
            case 7: nId = *pSp;
                    if( 0x0100 < nId ) nId = 0;
                    if( pDelta ) *pDelta = 0;
                    break;
            case 8: nId = SVBT16ToShort( &pSp[ 0 ] );
                    if( 0x0800 > nId )
                        nId = 0;
                    if( pDelta )
                        *pDelta = 1;
                    break;
            default:ASSERT( nId, "Es wurde vergessen, nVersion zu kodieren!" );
        }
    }
    return nId;
}


// ohne Token und LaengenByte
short WW8GetSprmSizeNetto( BYTE nVersion, BYTE* pSprm, USHORT* pId )
{
    BYTE   nDelta = ( 8 > nVersion ) ? 0 : 1;
    USHORT nId = pId ? *pId : WW8GetSprmId( nVersion, pSprm );
    return WW8GetSprmSize0( nId, pSprm, nDelta );
}


// mit Token und LaengenByte
short WW8GetSprmSizeBrutto( BYTE nVersion, BYTE* pSprm, USHORT* pId )
{
    BYTE   nDelta = ( 8 > nVersion ) ? 0 : 1;
    USHORT nId = pId ? *pId : WW8GetSprmId( nVersion, pSprm );
    return WW8GetSprmSize0( nId, pSprm, nDelta )
            + 1 + nDelta
            + WW8GetSprmInfo( nId ).nVari;
}

BYTE WW8SprmDataOfs( USHORT nId )
{
    return WW8GetSprmInfo( nId ).nVari;
}


/*************************************************************************
      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/ww8/dump/ww8scan.cxx,v 1.2 2000-10-24 14:01:34 jp Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.1.1.1  2000/09/18 17:14:59  hr
      initial import

      Revision 1.106  2000/09/18 16:05:02  willem.vandorp
      OpenOffice header added.

      Revision 1.105  2000/08/24 06:42:57  khz
      #75708# advance StartPos of attributes starting on a Table-Row-End by 1

      Revision 1.104  2000/08/22 12:25:11  khz
      #77550# prevent invalid SPRMs from setting pMemPos to 0x00000001

      Revision 1.103  2000/08/21 15:03:01  khz
      #77692# import page orientation

      Revision 1.102  2000/08/18 09:49:57  khz
      Skip trailing WORD (instead of BYTE) in WW8Read_xstz

      Revision 1.101  2000/07/28 08:01:46  khz
      #77183# avoid accessing pPcd when there is NO Piece Table

      Revision 1.100  2000/07/19 10:36:50  khz
      ##76458# wrong parameter calling WW8Read_xstz in WW8ReadSTTBF() ctor fixed

      Revision 1.99  2000/07/12 12:24:27  khz
      #76503# use WW8PLCFx_Cp_FKP::?etIdx2() to save/restore nIdx of pPcd

      Revision 1.98  2000/06/07 12:43:18  khz
      Changes for Unicode

      Revision 1.97  2000/05/31 12:23:01  khz
      Changes for Unicode

      Revision 1.96  2000/05/25 09:55:51  hr
      workaound for Solaris Workshop compiler, SWAPLONG -> SWAPSHORT

      Revision 1.95  2000/05/25 08:06:31  khz
      Piece Table optimization, Unicode changes, Bugfixes

      Revision 1.94  2000/05/18 10:58:46  jp
      Changes for Unicode

      Revision 1.93  2000/05/16 12:13:09  jp
      ASS_FALSE define removed

      Revision 1.92  2000/05/16 11:07:52  khz
      Unicode code-conversion

      Revision 1.91  2000/05/04 07:39:27  khz
      Task #75379# added Save() and Restore() to WW8PLCFx, WW8PLCFx_Cp_FKP, WW8PLCFxDesc (2)

      Revision 1.90  2000/05/04 07:32:46  khz
      Task #75379# added Save() and Restore() to WW8PLCFx, WW8PLCFx_Cp_FKP, WW8PLCFxDesc

      Revision 1.89  2000/03/06 16:57:48  khz
      Task #73790# Convert Filenames by methode regarding byte order (2)

      Revision 1.88  2000/03/03 15:54:08  khz
      Task #73790# Convert Filenames by methode regarding byte order

      Revision 1.87  2000/02/29 13:33:19  cmc
      #73671# Fix: Error in endnote seperator skip code

      Revision 1.86  2000/02/23 17:42:59  cmc
      #68832# Consider endnotes for header footer code

      Revision 1.85  2000/02/23 13:33:48  khz
      Task #70473# changes for unicode

      Revision 1.83  2000/02/22 16:23:19  khz
      Task #72987# Ignore Sprms covering ONLY a Tab-Row-End Char #7

      Revision 1.82  2000/02/22 10:39:56  cmc
      #73289# RowEnd Search failing due to lack of atributes of piece

      Revision 1.81  2000/02/18 15:10:27  khz
      #70473# changes for unicode

      Revision 1.80  2000/02/15 09:12:54  khz
      Task #71565# workaround because String::ConvertFromWChar cannot convert 0xF0..

      Revision 1.79  2000/02/14 09:11:34  cmc
      #72579# removed old glossary test code

      Revision 1.77  2000/02/09 11:31:50  khz
      Task #70473# char_t --> sal_Unicode

      Revision 1.76  2000/02/09 08:57:28  khz
      Task #72647# Read SPRMs that are stored in piece table grpprls (2)

      Revision 1.75  2000/02/03 10:08:30  cmc
      #72268# Allow create fib from offset, added data to STTBF

      Revision 1.74  2000/02/02 18:07:47  khz
      Task #69885# Read SPRMs that are stored in piece table grpprls

      Revision 1.73  1999/12/21 08:02:45  khz
      Task #68143# nIdx set to Zero caused Hd/Ft to get wrong sprms

      Revision 1.72  1999/12/07 14:29:00  khz
      Task #69508# import sprmPHugePapx by reading DATA stream

      Revision 1.71  1999/12/07 09:53:04  khz
      Task #67979# typo-error corrected (was '==' instead of '=')

      Revision 1.70  1999/12/06 19:34:27  khz
      Task #67979# Absolute Positioned Object must have SPRM 5 or SPRM 29

      Revision 1.69  1999/11/19 15:09:24  khz
      Task #69910# Use extra PLCF structure for field!

      Revision 1.68  1999/11/03 17:20:04  khz
      Task #68676# GPF when loading file under Solaris (reading FFN structures)

      Revision 1.67  1999/10/21 15:36:25  khz
      Import Redlining (4)

      Revision 1.66  1999/10/13 21:06:47  khz
      Import Redlining (3)

      Revision 1.65  1999/10/08 09:25:29  khz
      Import Redlining

      Revision 1.64  1999/09/10 15:36:43  khz
      CharSet matching made by TENCINFO.H::rtl_getTextEncodingFromWindowsCharset()

      Revision 1.63  1999/09/09 18:16:17  khz
      CharSet matching now done in central methode WW8SCAN.HXX::WW8GetCharSet()

      Revision 1.62  1999/09/08 13:26:13  khz
      Better performance by reducing use of SWAP..() and SVBT..To..() methods

      Revision 1.61  1999/08/30 19:53:02  JP
      Bug #68219#: no static members - be reentrant


      Rev 1.60   30 Aug 1999 21:53:02   JP
   Bug #68219#: no static members - be reentrant

      Rev 1.59   09 Aug 1999 18:34:38   KHZ
   Task #67543# Import of Property Modifier(variant 1) (PRM) in WW8 docs

      Rev 1.58   18 Jun 1999 15:53:56   KHZ
   Reference field (page, bookmark, footnote) part#1

      Rev 1.57   15 Jun 1999 14:34:56   JP
   for Export: FIB - read/write SttbListNames

      Rev 1.56   14 Jun 1999 16:00:18   KHZ
   Task #66411# allow import of document even when SEPX is missing completely

      Rev 1.55   03 Jun 1999 16:58:56   KHZ
   Task #66418# Nach C'tor WW8Fkp ein ''pSt->Seek( nOldPos );'' ausfuehren!

      Rev 1.54   03 Jun 1999 11:42:46   KHZ
   Task #66398# Variante 1 fuer PRM ist bei Ver67 Ok.

      Rev 1.53   02 Jun 1999 17:07:46   KHZ
   Task #66393# in WW8Cp2Fc ist *pIsUnicode bei bVer67 zu initialisieren

      Rev 1.52   02 Jun 1999 09:32:44   KHZ
   Task #66227# a) kein Unicode bei Ver67 ;-)  b) Grafik in grupp. Textbox

      Rev 1.51   28 May 1999 09:59:12   JP
   Fib-CTOR: wichtige Flags fuer den W97-Export setzen

      Rev 1.50   28 May 1999 09:19:22   KHZ
   Task #66395# Return-Adresse kein komplexer Ausdruck mehr

      Rev 1.49   26 May 1999 10:58:02   JP
   Fib: einige Konstanten nachgetragen

      Rev 1.48   21 May 1999 01:58:08   JP
   Export von WW97 Format: DOP in TableStream

      Rev 1.47   19 May 1999 11:12:54   JP
   WinWord97-ExportFilter

      Rev 1.46   11 May 1999 17:15:32   KHZ
   Task #66019# FontFamily: andere Bitreihenfolge auf Solaris beachten

      Rev 1.45   14 Apr 1999 14:20:12   KHZ
   Task #61268# Solaris-Problem wegen drei vergessenen SVBT16-Umwandlungen

      Rev 1.44   13 Apr 1999 16:02:28   KHZ
   Task #61268# SVBT16 statt USHORT bei Bookmarks eingesetzt

      Rev 1.43   10 Mar 1999 15:11:12   KHZ
   Task #62521# Einlesen von Sonderzeichen

      Rev 1.42   08 Mar 1999 11:48:48   KHZ
   Task #57749# Writer kann jetzt spaltige Bereiche

      Rev 1.41   26 Feb 1999 14:43:38   KHZ
   Task #59715# Behandlung von Section breaks

      Rev 1.40   25 Jan 1999 10:22:38   KHZ
   Task #60715# in Textobjekt verankerte Grafik als Grafik importieren

      Rev 1.39   19 Jan 1999 10:47:42   KHZ
   Task #60878# WW8Cp2Fc setzt *pIsUniCode bei Non-Complex auf FALSE

      Rev 1.38   18 Jan 1999 08:57:08   KHZ
   Task #60878# WW8Dp2Fc nur dann Assertion, wenn kein pTestFlag

      Rev 1.37   11 Dec 1998 11:12:38   JP
   Task #59580# Unicode (3) - Bug behoben beim Read_Xstz

      Rev 1.36   10 Dec 1998 21:29:00   JP
   Bug #59643#: benutzerdefinierte Zeichen von Fuss-/EndNoten verarbeiten

      Rev 1.35   07 Dec 1998 18:17:06   KHZ
   Task #59580# falsches Feldende: WW8SkipField verglich mit 0x7f statt 0x1f

      Rev 1.34   05 Dec 1998 17:10:26   KHZ
   Task #59580# Unicode (3)

      Rev 1.33   04 Dec 1998 20:19:50   KHZ
   Task #58766# Textboxen mit Unicode-Inhalt

      Rev 1.32   03 Dec 1998 19:10:02   KHZ
   Task #58766# Unicode-Import (2)

      Rev 1.31   03 Dec 1998 10:39:50   JP
   Task #60063#: Kommentare als PostIts einlesen

      Rev 1.30   02 Dec 1998 16:57:02   JP
   Task #60063#: Kommentare als PostIts einlesen

      Rev 1.29   30 Nov 1998 17:46:50   JP
   Task #59822#: OLE-Objecte importieren

      Rev 1.28   30 Nov 1998 17:30:36   KHZ
   Task #54828# Unicode-Import

      Rev 1.27   03 Nov 1998 18:29:54   KHZ
   Task #57243# Performance-Gewinn durch Vermeiden ueberfluessiger WW8GetSprmId()

      Rev 1.26   02 Nov 1998 17:58:32   KHZ
   Task #57017# Textmarken-Namen als UNICODE-Strings

      Rev 1.25   16 Oct 1998 16:12:34   KHZ
   Task #53520# ueberpruefe Feld DRUCKDATUM in DocInfo anhand von WW-internem Feld

      Rev 1.24   22 Sep 1998 19:37:42   KHZ
   Bug #57018# Fontnamen wurden unrichtig importiert

      Rev 1.23   22 Sep 1998 17:40:24   KHZ
   Bug #56310# Kopf-/Fusszeilen in Unicode-Dokumenten jetzt korrekt

      Rev 1.22   03 Sep 1998 22:15:40   KHZ
   Task #55189# Textboxen

      Rev 1.21   27 Aug 1998 10:12:28   KHZ
   Task #55189# Escher-Import fuer Draw, Calc und Writer

      Rev 1.20   11 Aug 1998 12:26:16   KHZ
   Task #52607# Optimierung in AdjustEnds(), GetNewSprms(), GetNoNewSprms()

      Rev 1.19   30 Jul 1998 14:47:24   JP
   Bug #54385#: wchar_t ist unter UNX ein long und kein UINT16

      Rev 1.18   30 Jul 1998 00:02:54   KHZ
   Task #53614# Grafiken, die NICHT ueber dem Text liegen (MSDFF-Quick-Hack)

      Rev 1.17   29 Jul 1998 16:53:18   KHZ
   Task #52607# Fehler in FKP (Teil 2)

      Rev 1.16   29 Jul 1998 13:56:10   KHZ
   Task #52607# Fehler in FKP

      Rev 1.15   28 Jul 1998 22:27:28   KHZ
   Task #52607# nummerierte Listen (Teil 3)

      Rev 1.14   28 Jul 1998 11:05:46   KHZ
   Task #52607# nummerierte Listen (Teil 1)

      Rev 1.13   22 Jul 1998 15:32:56   KHZ
   Task #52607#

      Rev 1.12   21 Jul 1998 14:52:28   KHZ
   Task #52607# (WW 97 Import)

      Rev 1.11   21 Jul 1998 12:32:52   KHZ
   als MSDrawingObject eingebettete Grafik (Teil 1)

      Rev 1.10   16 Jul 1998 21:50:14   KHZ
   Task #52607# Compilererror wg. (rLen *= 2) -= 2

      Rev 1.9   15 Jul 1998 15:47:18   KHZ
   Task #52607# Felder

      Rev 1.8   14 Jul 1998 16:34:18   KHZ
   Task #52607# Absatz- und Zeichen-Styles

      Rev 1.7   13 Jul 1998 15:29:46   KHZ
   Task #52607# Seitenvorlage fuer 1st Page, embed. Grafik (NICHT UEBER Text)

      Rev 1.6   30 Jun 1998 21:33:20   KHZ
   Header/Footer/Footnotes weitgehend ok

      Rev 1.5   26 Jun 1998 20:50:22   KHZ
   Absatz-Attribute jetzt weitestgehend ok

      Rev 1.4   23 Jun 1998 20:49:26   KHZ
   verarbeitet jetzt auch mehrere FKPs

      Rev 1.3   23 Jun 1998 11:24:28   KHZ
   Zwischenstand: die meisten Zeichenattribute Ok!

      Rev 1.2   18 Jun 1998 09:47:12   KHZ
   Zwischenstand fur 396c

      Rev 1.1   16 Jun 1998 18:35:30   KHZ
   DaSi-Stand

      Rev 1.0   16 Jun 1998 10:57:24   KHZ
   Initial revision.

      Rev 1.1   10 Jun 1998 17:22:32   KHZ
   Zwischenstand-Sicherung Dumper

      Rev 1.0   27 May 1998 15:29:02   KHZ
   Initial revision.


*************************************************************************/
