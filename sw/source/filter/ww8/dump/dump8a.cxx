/*************************************************************************
 *
 *  $RCSfile: dump8a.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:59 $
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


//#include <stdio.h>        // SEEK_SET
#include <string.h>     // memset(), ...
#include <io.h>         // access()


//#include "defs.hxx"
#include <tools/solar.h>
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#include "ww8struc.hxx"
#include "ww8scan.hxx"
#include "ww8darr.hxx"
#include "ww8dout.hxx"

#if 0
#define DELETEZ( p ) ( delete( p ), p = 0 )
//#define LONG_MAX ( (long)0x7fffffff )
#endif
#define ASSERT( a, b ) ( (a)?(void)0:(void)(*pOut<<endl1<<"ASSERTION failed "<< __FILE__<<__LINE__<< b <<endl1) )


#define nWW8MaxListLevel 9

WW8ScannerBase* pSBase = 0;
//WW8PLCFMan* pPLCFMan = 0;

WW8Fib*            pWwFib = 0;
SvStorageRef*      pxStor;
SvStorageStreamRef xStrm;
SvStorageStreamRef xTableStream;    // ist bei Ver6-7 mit xStrm identisch,
                                    // ansonsten entweder 0TABLE oder 1TABLE
SvStorageStreamRef xDataStream;     // ist bei Ver6-7 mit xStrm identisch,
                                    // ansonsten DATA

typedef void (*FNDumpData)( void* );

BOOL DumpChar( BYTE c );
void DumpShortPlainText( WW8_CP nStartCp, long nTextLen, char* pName );
void DumpPlainText( WW8_CP nStartCp, long nTextLen, char* pName );
void DumpSprms( BYTE nVersion, SvStream& rSt, short nLen );


BOOL WW8ReadINT32( SvStream& rStrm, INT32& rTarget )
{
      rStrm >> rTarget;
      return TRUE;


      SVBT32 nData;
      BOOL bOk = TRUE;
      if( 4 == rStrm.Read( &nData, 4 ) )
              rTarget = SVBT32ToLong( nData );
      else
              bOk = FALSE;
      return bOk;
}

BOOL WW8ReadINT16( SvStream& rStrm, INT16& rTarget )
{
      rStrm >> rTarget;
      return TRUE;


      SVBT16 nData;
      BOOL bOk = TRUE;
      if( 2 == rStrm.Read( &nData, 2 ) )
      {
              rTarget = SVBT16ToShort( nData );
      }
      else
              bOk = FALSE;
      return bOk;
}

BOOL WW8ReadBYTE( SvStream& rStrm, BYTE& rTarget )
{
      rStrm >> rTarget;
      return TRUE;


      SVBT8 nData;
      BOOL bOk = TRUE;
      if( 1 == rStrm.Read( &nData, 1 ) )
              rTarget = SVBT8ToByte( nData );
      else
              bOk = FALSE;
      return bOk;
}

BOOL WW8ReadUINT32( SvStream& rStrm, UINT32& rTarget )
     { return WW8ReadINT32( rStrm, (INT32&) rTarget ); }

BOOL WW8ReadUINT16( SvStream& rStrm, UINT16& rTarget )
     { return WW8ReadINT16( rStrm, (INT16&) rTarget ); }


static void Dump_LVL( UINT16 nLevel )
{
    indent( *pOut, *xTableStream );

    long nStart, dxaSpace, dxaIndent;
    BYTE nfc, nFlags, ixchFollow, cbChpx, cbPapx;
    BYTE aOfsNumsXCH[nWW8MaxListLevel];
    *xTableStream >> nStart >> nfc >> nFlags;
    xTableStream->Read( aOfsNumsXCH, 9 );
    *xTableStream >> ixchFollow >> dxaSpace >> dxaIndent
        >> cbChpx >> cbPapx;
    xTableStream->SeekRel( 2 );

    *pOut << " Level: " << nLevel << endl1;
    *pOut << indent2 << "  Start: " << nStart
                    << " Fmt: " << (USHORT)nfc
                    << " Follow: ";
    if( 0 == ixchFollow ) *pOut << "tab";
    else if( 1 == ixchFollow ) *pOut << "blank";
    else if( 2 == ixchFollow ) *pOut << "nothing";
    *pOut << endl1  << indent2<< "  dxSpace: " << dxaSpace
        << " dxaIndent: " << dxaIndent << " LevelCharPos: ";

    for( BYTE x = 0; x < nWW8MaxListLevel; ++x )
        *pOut << (USHORT)aOfsNumsXCH[ x ] << ", ";
    *pOut << endl1;

    if( cbPapx )
    {
        ULONG nXPos = xTableStream->Tell();
        DumpSprms( pWwFib->nVersion, *xTableStream, cbPapx );
        xTableStream->Seek( nXPos + cbPapx );
    }
    if( cbChpx )
    {
        ULONG nXPos = xTableStream->Tell();
        DumpSprms( pWwFib->nVersion, *xTableStream, cbChpx );
        xTableStream->Seek( nXPos + cbChpx );
    }

    USHORT nStrLen, nC;
    *xTableStream >> nStrLen;
    char* pStr = new char[ nStrLen+1 ], *p = pStr;
    while( nStrLen-- )
    {
        *xTableStream >> nC;
        if( 10 > nC ) nC += 0x30;
        *pStr++ = (char)nC;
    }
    *pStr = 0;
    *pOut << indent2 << "Str: \"" << p << '\"' << endl1 << endl1;
}

static void DumpNumList()
{
    if( pWwFib->lcbSttbListNames )
    {
        ULONG nOldPos = xTableStream->Tell();
        xTableStream->Seek( pWwFib->fcSttbListNames );

        *pOut << endl1;
        begin( *pOut, *xTableStream ) << "ListNames, Size ";

        UINT16 nDummy;
        UINT32 nCount;
        *xTableStream >> nDummy >> nCount;

        *pOut << nCount << ", Dummy: " << nDummy << endl1;

        *pOut << indent2;
        for( ; nCount; --nCount )
        {
            UINT16 nLen, nC;
            *xTableStream >> nLen;
            char * pChar = new char[ nLen + 1 ], *p = pChar;
            while( nLen-- )
            {
                *xTableStream >> nC;
                *pChar++ = (char)nC;
            }
            *pChar = 0;
            *pOut << '\"' << p << '\"';
            if( 1 < nCount ) *pOut << ", ";
            delete p;
        }
        *pOut << endl1;

        end( *pOut, *xTableStream ) << "ListNames" << endl1;
        xTableStream->Seek( nOldPos );
    }

    if( pWwFib->lcbPlcfLst )
    {
        ULONG nOldPos = xTableStream->Tell();
        xTableStream->Seek( pWwFib->fcPlcfLst );

        *pOut << endl1;
        begin( *pOut, *xTableStream ) << "LiST Data on File, Size ";

        UINT16 nCount;
        *xTableStream >> nCount;
        *pOut << nCount << endl1;

        ULONG nLVLPos = pWwFib->fcPlcfLst + ( nCount * 0x1c ) + 2;

        // 1.1 alle LST einlesen
        for( UINT16 nList = 0; nList < nCount; nList++ )
        {
            indent( *pOut, *xTableStream );

            BYTE nByte;
            UINT32 nLstId, nTplId;
            *xTableStream >> nLstId >> nTplId;

            USHORT aStyleIdArr[ nWW8MaxListLevel ];
            for( USHORT nLevel = 0; nLevel < nWW8MaxListLevel; nLevel++ )
                *xTableStream >> aStyleIdArr[ nLevel ];
            *xTableStream >> nByte;
            xTableStream->SeekRel( 1 );     // Dummy ueberlesen

            *pOut << "ListId: " << nLstId
                << " TemplateId " << nTplId << endl1
                << indent2 << " StyleIds: ";
            for( nLevel = 0; nLevel < nWW8MaxListLevel; nLevel++ )
                *pOut << aStyleIdArr[ nLevel ] << ", ";

            *pOut << endl1;
            if( 1 & nByte ) *pOut << " <simple List>";
            if( 2 & nByte ) *pOut << " <restart at new section>";
            if( 3 & nByte ) *pOut << endl1;

            ULONG nTmpPos = xTableStream->Tell();
            xTableStream->Seek( nLVLPos );

            // 1.2 alle LVL aller aLST einlesen
            for( nLevel = 0; nLevel < ((1 & nByte) ? 1 : nWW8MaxListLevel); nLevel++ )
                Dump_LVL( nLevel );

            nLVLPos = xTableStream->Tell();
            xTableStream->Seek( nTmpPos );
        }

        end( *pOut, *xTableStream ) << "LiST Data on File" << endl1;
        xTableStream->Seek( nOldPos );
    }

    if( pWwFib->lcbPlfLfo )
    {
        ULONG nOldPos = xTableStream->Tell();
        xTableStream->Seek( pWwFib->fcPlfLfo );

        *pOut << endl1;
        begin( *pOut, *xTableStream ) << "List Format Override, Size ";

        UINT32 nCount, nLstId;
        *xTableStream >> nCount;
        *pOut << nCount << endl1;

        ULONG nLVLPos = pWwFib->fcPlfLfo + ( nCount * 0x10 ) + 4;

        for( ; nCount; --nCount )
        {
            indent( *pOut, *xTableStream );

            BYTE nLevels;
            *xTableStream >> nLstId;
            xTableStream->SeekRel( 8 );
            *xTableStream >> nLevels;
            xTableStream->SeekRel( 3 );

            *pOut << "ListId: " << nLstId
                << " Override Levels: " << (USHORT)nLevels << endl1;

            ULONG nTmpPos = xTableStream->Tell();
            xTableStream->Seek( nLVLPos );

            for( BYTE nLvl = 0; nLvl < nLevels; ++nLvl )
            {
                UINT32 nStartAt;
                BYTE nFlags;
                do {
                    *xTableStream >> nFlags;
                } while( 0xFF == nFlags  );
                xTableStream->SeekRel( -1 );

                indent( *pOut, *xTableStream );
                *xTableStream >> nStartAt >> nFlags;
                xTableStream->SeekRel( 3 );

                if( nFlags & 0x10 ) *pOut << "<Override start>";
                if( nFlags & 0x20 ) *pOut << "<Override formattting>";
                if( nFlags & 0x30 ) *pOut << endl1;

                if( nFlags & 0x20 )
                    Dump_LVL( nFlags & 0x0F );
            }
            nLVLPos = xTableStream->Tell();
            xTableStream->Seek( nTmpPos );
        }

        end( *pOut, *xTableStream ) << "List Format Override" << endl1 << endl1;
        xTableStream->Seek( nOldPos );
    }
}

static void DumpBookLow()
{
    *pOut << begin2 << "Bookmarks Low" << endl1;
    *pOut << indent2 <<"Start:  Adr " << hex6 << pWwFib->fcPlcfbkf
         << ", Len: " << pWwFib->lcbPlcfbkf << endl1;
    *pOut << indent2 << "End:    Adr " << hex6 << pWwFib->fcPlcfbkl
         << ", Len: " << pWwFib->lcbPlcfbkl << endl1;
    *pOut << indent2 << "Strings: Adr " << hex6 << pWwFib->fcSttbfbkmk
         << ", Len: " << pWwFib->lcbSttbfbkmk << endl1;
    *pOut << endl1;

    WW8PLCFspecial aStarts( &xStrm, pWwFib->fcPlcfbkf, pWwFib->lcbPlcfbkf, 4 );
    WW8PLCFspecial aEnds(   &xStrm, pWwFib->fcPlcfbkl, pWwFib->lcbPlcfbkl, 0, -1, TRUE );

    USHORT i = 0;
    while( 1 ){
        long nStart = aStarts.GetPos( i );
        if( nStart >= LONG_MAX )
            break;

        *pOut << indent2 << i << ".StartCp: " << hex6 << nStart;

        const void* p = aStarts.GetData( i );
        if( p ){
            *pOut << ", EndIdx: ";
            USHORT nEndIdx = *((USHORT*)p);
            *pOut << nEndIdx;
            long nEnd = aEnds.GetPos( nEndIdx );
            *pOut << ", End: " << hex6 << nEnd << hex
                  << ", Len: " << nEnd - nStart << dec;
        }else{
            *pOut << " aStarts.GetData() ging schief.";
        }

#if 0       // gibt's im wirklichen Leben nicht
        if( aEnds.Get( n, p ) ){
            *pOut << " aEnds.iBkf: ";
            if( p )
                *pOut << *((USHORT*)p);
            else
                *pOut << "NULL-Ptr";
        }else{
            *pOut << " aEnds.Get() ging schief.";
        }
#endif

        *pOut << endl1;
        i++;
    }
    *pOut << end2 << "Bookmarks Low" << endl1 << endl1;
}

static void DumpBookHigh()
{
    WW8PLCFx_Book aBook( &xStrm, &xTableStream, *pWwFib, 0 );
    if( !aBook.GetIMax() ){
        *pOut << indent1 << "No Bookmarks" << endl1 << endl1;
        return;
    }

    *pOut << indent1 << begin1 << "Bookmarks High" << endl1;

    USHORT i = 0;
    while( 1 ){
        long nPos = aBook.Where();
        if( nPos >= LONG_MAX )
            break;
        *pOut << indent2 << i << ". Cp:" << hex6 << nPos << dec;
        *pOut << ( ( aBook.GetIsEnd() ) ? " Ende  " : " Anfang" );
        *pOut << " Handle: " << aBook.GetHandle();
//      *pOut << " Len: " << hex4 << aBook.GetLen() << dec;
        const char* pN = aBook.GetName();
        if( pN )
            *pOut << " Name: " << pN << endl1;
        else
            *pOut << endl1;
        aBook++;
        i++;
    }
    *pOut << end2 << "Bookmarks High" << endl1 << endl1;
}

static BOOL DumpField3( WW8PLCFspecial& rPlc )
{
    WW8FieldDesc aF;

    BOOL bOk = WW8GetFieldPara( pWwFib->nVersion, rPlc, aF );
    if( !bOk )
    {
        *pOut << "       " << indent1 << "no WW8GetFieldPara()" << endl1;
        return FALSE;
    }
    *pOut << "       " << indent1 << begin1 << "Field Cp: " << hex
          << aF.nSCode << ", Len: " << aF.nLCode << "; Cp: " << aF.nSRes
          << ", Len: "<< aF.nLRes << ", Typ: " << dec << (USHORT)aF.nId
          << ", Options: " << hex << (USHORT)aF.nOpt;
    if( aF.bCodeNest )
        *pOut << " Code Nested";
    if( aF.bResNest )
        *pOut << " Result Nested";
    *pOut << endl1;
    DumpShortPlainText( aF.nSCode, aF.nLCode, "Code" );
    DumpShortPlainText( aF.nSRes, aF.nLRes, "Result" );
    *pOut << "       " << end1 << "Field" << endl1;
    return TRUE;
}

static BOOL DumpField2( WW8PLCFspecial& rPlc )
{
    WW8_CP nSCode, nECode, nSRes, nERes;
    void* pData;
    if( !rPlc.Get( nSCode, pData ) )                // Ende des Plc1 ?
        return FALSE;
    rPlc++;

    if( ((BYTE*)pData)[0] != 19 ){
        *pOut << "Field Error, " << (USHORT)((BYTE*)pData)[0] << endl1;
        return TRUE;    // nicht abbrechen
    }

    *pOut << "       " << indent1 << begin1 << "Field" << " Cp: " << hex
          << nSCode << " Typ: " << dec << (USHORT)((BYTE*)pData)[1] << endl1;

    if( !rPlc.Get( nECode, pData ) )            // Ende des Plc1 ?
        return FALSE;

    DumpShortPlainText( nSCode, nECode - nSCode, "Code" );  // Code, bei nested abgeschnitten
    nSRes = nECode;                             // Default


    while( ((BYTE*)pData)[0] == 19 ){           // immer noch neue (nested) Anfaenge ?
        DumpField2( rPlc );                     // nested Field im Beschreibungsteil
        if( !rPlc.Get( nSRes, pData ) )         // Ende des Plc1 ?
            return FALSE;
    }

    if( ((BYTE*)pData)[0] == 20 ){              // Field Separator ?
        rPlc++;
        *pOut << "       " << indent1 << "Field Seperator" << " Cp: " << hex << nSRes
              << ", Flags = 0x" << hex << (USHORT)((BYTE*)pData)[1] << dec << endl1;
        if( !rPlc.Get( nERes, pData ) )         // Ende des Plc1 ?
            return FALSE;

        while( ((BYTE*)pData)[0] == 19 ){       // immer noch neue (nested) Anfaenge ?
            DumpField2( rPlc );                 // nested Field im Resultatteil
            if( !rPlc.Get( nERes, pData ) )     // Ende des Plc1 ?
                return FALSE;
        }
        DumpShortPlainText( nSRes, nERes - nSRes, "Result" );   // Result, bei nested incl. nested Field

    }else{
        nERes = nSRes;                          // Kein Result vorhanden
    }

    rPlc++;
    if( ((BYTE*)pData)[0] == 21 ){              // Field Ende ?
        *pOut << "       " << end1 << " Field " << " Cp: " << hex << nERes
              << ", Flags = 0x" << hex << (USHORT)((BYTE*)pData)[1] << dec << endl1;
    }else{
        *pOut << "       Unknown Field Type" << endl1;
        *pOut << "       " << end1 << " Field " << endl1;
    }

    return TRUE;
}

static void DumpField1( WW8_FC nPos, long nLen, char* pName )
{
    if( !nLen )
    {
        *pOut << pName << ": No Fields" << endl1 << endl1;
        return;
    }

    WW8PLCFspecial aPlc( &xTableStream, nPos, nLen, 2 );

    *pOut << hex6 << nPos << dec2 <<  ' ' << indent1 << begin1;
    *pOut << "Plc." << pName << ", Len: ";
    *pOut << nLen << ", ca. " << aPlc.GetIMax()/3 << " Elements"

            << " Idx: " << (long)aPlc.GetIdx()
            << " IMax: " << aPlc.GetIMax()

            << endl1;

//  while( DumpField2( aPlc ) ){}
    while( (long)aPlc.GetIdx() < aPlc.GetIMax() )
    {
        DumpField3( aPlc );
        aPlc++;
    }

    end( *pOut, *xStrm ) << "Plcx." << pName << endl1 << endl1;
}


//-----------------------------------------
static void DumpFonts()
{
    WW8Fonts aFonts( *xTableStream, *pWwFib );

    USHORT i;

    *pOut << endl1;
    *pOut << 'T' << hex6 << pWwFib->fcSttbfffn << dec2 << ' ' << indent1 << begin1 << "FFNs" << endl1;

    for( i=0; i<aFonts.GetMax(); i++){
        // const
        WW8_FFN* p = (WW8_FFN*)aFonts.GetFont( i );

        *pOut << "Id:" << i << " Name:\"" << String( p->sFontname ).GetStr() << '"';    // Name
        if( p->ibszAlt )                    // gibt es einen alternativen Font ?

        *pOut << ", Alternativ:" << '"' << String( p->sFontname.Copy( p->ibszAlt ) ).GetStr() << '"';

        *pOut << ", PitchRequest:" << (short)p->prg << ", TrueType:" << (short)p->fTrueType;
        *pOut << ", FontFamily:" << (short)p->ff;
        *pOut << ", BaseWeight:" << p->wWeight;
        *pOut << ", CharacterSet:" << (short)p->chs;
        *pOut << endl1;
    }
    *pOut << "       " << end1 << "FFNs" << endl1 << endl1;
}


//-----------------------------------------
//          class DFib
//-----------------------------------------

class DFib: public WW8Fib
{
public:
    DFib( SvStream& rStrm, BYTE nVersion ) : WW8Fib( rStrm, nVersion ) {}
    void Dump();
};

//-----------------------------------------

void DFib::Dump()
{
    *pOut << "Adressen: FIB " << hex << 0 << ", Text "
         << fcMin << ", TextLen " << ccpText
         << ", Styles " << fcStshf << endl1;

    *pOut << "\twIdent: " << wIdent << endl1;
    *pOut << "\tnFib: " << nFib << endl1;
    *pOut << "\tnProduct: " << nProduct << endl1;
    *pOut << "\tlid: " << lid << endl1;
    *pOut << "\tpnNext: " << pnNext << endl1;

    *pOut << "\tfDot: " << (fDot ? '1' : '0') << endl1;
    *pOut << "\tfGlsy: " << (fGlsy ? '1' : '0') << endl1;
    *pOut << "\tfComplex: " << (fComplex ? '1' : '0') << endl1;
    *pOut << "\tfHasPic: " << (fHasPic ? '1' : '0') << endl1;
    *pOut << "\tcQuickSaves: " << (USHORT)cQuickSaves  << endl1;
    *pOut << "\tfEncrypted: " << (fEncrypted ? '1' : '0') << endl1;
    *pOut << "\tfWhichTblStm: " << (fWhichTblStm ? '1' : '0') << endl1;
    *pOut << "\tfExtChar: " << (fExtChar ? '1' : '0') << endl1;

    *pOut << "\tnFibBack: " << nFibBack << endl1;
    *pOut << "\tlKey1: " << lKey1 << endl1;
    *pOut << "\tlKey2: " << lKey2 << endl1;
    *pOut << "\tenvr: " << envr << endl1;

    *pOut << "\tfMac: " << (fMac ? '1' : '0') << endl1;
    *pOut << "\tfEmptySpecial: " << (fEmptySpecial ? '1' : '0') << endl1;
    *pOut << "\tfLoadOverridePage: " << (fLoadOverridePage ? '1' : '0') << endl1;
    *pOut << "\tfFuturesavedUndo: " << (fFuturesavedUndo ? '1' : '0') << endl1;
    *pOut << "\tfWord97Saved: " << (fWord97Saved ? '1' : '0') << endl1;

    *pOut << "\tchse: " << chse << endl1;
    *pOut << "\tchseTables: " << chseTables << endl1;
    *pOut << "\tfcMin: " << fcMin << endl1;
    *pOut << "\tfcMac: " << fcMac << endl1;
    *pOut << "\tcsw: " << csw << endl1;
    *pOut << "\twMagicCreated: " << wMagicCreated << endl1;
    *pOut << "\twMagicRevised: " << wMagicRevised << endl1;
    *pOut << "\twMagicCreatedPrivate: " << wMagicCreatedPrivate << endl1;
    *pOut << "\twMagicRevisedPrivate: " << wMagicRevisedPrivate << endl1;
    *pOut << "\tlidFE: " << lidFE << endl1;
    *pOut << "\tclw: " << clw << endl1;
    *pOut << "\tcbMac: " << cbMac << endl1;
    *pOut << "\tccpText: " << ccpText << endl1;
    *pOut << "\tccpFtn: " << ccpFtn << endl1;
    *pOut << "\tccpHdr: " << ccpHdr << endl1;
    *pOut << "\tccpMcr: " << ccpMcr << endl1;
    *pOut << "\tccpAtn: " << ccpAtn << endl1;
    *pOut << "\tccpEdn: " << ccpEdn << endl1;
    *pOut << "\tccpTxbx: " << ccpTxbx << endl1;
    *pOut << "\tccpHdrTxbx: " << ccpHdrTxbx << endl1;
    *pOut << "\tpnFbpChpFirst: " << pnFbpChpFirst << endl1;
    *pOut << "\tpnFbpPapFirst: " << pnFbpPapFirst << endl1;
    *pOut << "\tpnFbpLvcFirst: " << pnFbpLvcFirst << endl1;
    *pOut << "\tpnLvcFirst: " << pnLvcFirst << endl1;
    *pOut << "\tcpnBteLvc: " << cpnBteLvc << endl1;
    *pOut << "\tfcIslandFirst: " << fcIslandFirst << endl1;
    *pOut << "\tfcIslandLim: " << fcIslandLim << endl1;
    *pOut << "\tcfclcb: " << cfclcb << endl1;
    *pOut << "\tfcStshfOrig: " << fcStshfOrig << endl1;
    *pOut << "\tlcbStshfOrig: " << lcbStshfOrig << endl1;
    *pOut << "\tfcStshf: " << fcStshf << endl1;
    *pOut << "\tlcbStshf: " << lcbStshf << endl1;
    *pOut << "\tfcPlcffndRef: " << fcPlcffndRef << endl1;
    *pOut << "\tlcbPlcffndRef: " << lcbPlcffndRef << endl1;
    *pOut << "\tfcPlcffndTxt: " << fcPlcffndTxt << endl1;
    *pOut << "\tlcbPlcffndTxt: " << lcbPlcffndTxt << endl1;
    *pOut << "\tfcPlcfandRef: " << fcPlcfandRef << endl1;
    *pOut << "\tlcbPlcfandRef: " << lcbPlcfandRef << endl1;
    *pOut << "\tfcPlcfandTxt: " << fcPlcfandTxt << endl1;
    *pOut << "\tlcbPlcfandTxt: " << lcbPlcfandTxt << endl1;
    *pOut << "\tfcPlcfsed: " << fcPlcfsed << endl1;
    *pOut << "\tlcbPlcfsed: " << lcbPlcfsed << endl1;
    *pOut << "\tfcPlcfpad: " << fcPlcfpad << endl1;
    *pOut << "\tlcbPlcfpad: " << lcbPlcfpad << endl1;
    *pOut << "\tfcPlcfphe: " << fcPlcfphe << endl1;
    *pOut << "\tlcbPlcfphe: " << lcbPlcfphe << endl1;
    *pOut << "\tfcSttbfglsy: " << fcSttbfglsy << endl1;
    *pOut << "\tlcbSttbfglsy: " << lcbSttbfglsy << endl1;
    *pOut << "\tfcPlcfglsy: " << fcPlcfglsy << endl1;
    *pOut << "\tlcbPlcfglsy: " << lcbPlcfglsy << endl1;
    *pOut << "\tfcPlcfhdd: " << fcPlcfhdd << endl1;
    *pOut << "\tlcbPlcfhdd: " << lcbPlcfhdd << endl1;
    *pOut << "\tfcPlcfbteChpx: " << fcPlcfbteChpx << endl1;
    *pOut << "\tlcbPlcfbteChpx: " << lcbPlcfbteChpx << endl1;
    *pOut << "\tfcPlcfbtePapx: " << fcPlcfbtePapx << endl1;
    *pOut << "\tlcbPlcfbtePapx: " << lcbPlcfbtePapx << endl1;
    *pOut << "\tfcPlcfsea: " << fcPlcfsea << endl1;
    *pOut << "\tlcbPlcfsea: " << lcbPlcfsea << endl1;
    *pOut << "\tfcSttbfffn: " << fcSttbfffn << endl1;
    *pOut << "\tlcbSttbfffn: " << lcbSttbfffn << endl1;
    *pOut << "\tfcPlcffldMom: " << fcPlcffldMom << endl1;
    *pOut << "\tlcbPlcffldMom: " << lcbPlcffldMom << endl1;
    *pOut << "\tfcPlcffldHdr: " << fcPlcffldHdr << endl1;
    *pOut << "\tlcbPlcffldHdr: " << lcbPlcffldHdr << endl1;
    *pOut << "\tfcPlcffldFtn: " << fcPlcffldFtn << endl1;
    *pOut << "\tlcbPlcffldFtn: " << lcbPlcffldFtn << endl1;
    *pOut << "\tfcPlcffldAtn: " << fcPlcffldAtn << endl1;
    *pOut << "\tlcbPlcffldAtn: " << lcbPlcffldAtn << endl1;
    *pOut << "\tfcPlcffldMcr: " << fcPlcffldMcr << endl1;
    *pOut << "\tlcbPlcffldMcr: " << lcbPlcffldMcr << endl1;
    *pOut << "\tfcSttbfbkmk: " << fcSttbfbkmk << endl1;
    *pOut << "\tlcbSttbfbkmk: " << lcbSttbfbkmk << endl1;
    *pOut << "\tfcPlcfbkf: " << fcPlcfbkf << endl1;
    *pOut << "\tlcbPlcfbkf: " << lcbPlcfbkf << endl1;
    *pOut << "\tfcPlcfbkl: " << fcPlcfbkl << endl1;
    *pOut << "\tlcbPlcfbkl: " << lcbPlcfbkl << endl1;
    *pOut << "\tfcCmds: " << fcCmds << endl1;
    *pOut << "\tlcbCmds: " << lcbCmds << endl1;
    *pOut << "\tfcPlcfmcr: " << fcPlcfmcr << endl1;
    *pOut << "\tlcbPlcfmcr: " << lcbPlcfmcr << endl1;
    *pOut << "\tfcSttbfmcr: " << fcSttbfmcr << endl1;
    *pOut << "\tlcbSttbfmcr: " << lcbSttbfmcr << endl1;
    *pOut << "\tfcPrDrvr: " << fcPrDrvr << endl1;
    *pOut << "\tlcbPrDrvr: " << lcbPrDrvr << endl1;
    *pOut << "\tfcPrEnvPort: " << fcPrEnvPort << endl1;
    *pOut << "\tlcbPrEnvPort: " << lcbPrEnvPort << endl1;
    *pOut << "\tfcPrEnvLand: " << fcPrEnvLand << endl1;
    *pOut << "\tlcbPrEnvLand: " << lcbPrEnvLand << endl1;
    *pOut << "\tfcWss: " << fcWss << endl1;
    *pOut << "\tlcbWss: " << lcbWss << endl1;
    *pOut << "\tfcDop: " << fcDop << endl1;
    *pOut << "\tlcbDop: " << lcbDop << endl1;
    *pOut << "\tfcSttbfAssoc: " << fcSttbfAssoc << endl1;
    *pOut << "\tcbSttbfAssoc: " << cbSttbfAssoc << endl1;
    *pOut << "\tfcClx: " << fcClx << endl1;
    *pOut << "\tlcbClx: " << lcbClx << endl1;
    *pOut << "\tfcPlcfpgdFtn: " << fcPlcfpgdFtn << endl1;
    *pOut << "\tlcbPlcfpgdFtn: " << lcbPlcfpgdFtn << endl1;
    *pOut << "\tfcAutosaveSource: " << fcAutosaveSource << endl1;
    *pOut << "\tlcbAutosaveSource: " << lcbAutosaveSource << endl1;
    *pOut << "\tfcGrpStAtnOwners: " << fcGrpStAtnOwners << endl1;
    *pOut << "\tlcbGrpStAtnOwners: " << lcbGrpStAtnOwners << endl1;
    *pOut << "\tfcSttbfAtnbkmk: " << fcSttbfAtnbkmk << endl1;
    *pOut << "\tlcbSttbfAtnbkmk: " << lcbSttbfAtnbkmk << endl1;
    *pOut << "\tfcPlcfdoaMom: " << fcPlcfdoaMom << endl1;
    *pOut << "\tlcbPlcfdoaMom: " << lcbPlcfdoaMom << endl1;
    *pOut << "\tfcPlcfdoaHdr: " << fcPlcfdoaHdr << endl1;
    *pOut << "\tlcbPlcfdoaHdr: " << lcbPlcfdoaHdr << endl1;
    *pOut << "\tfcPlcfspaMom: " << fcPlcfspaMom << endl1;
    *pOut << "\tlcbPlcfspaMom: " << lcbPlcfspaMom << endl1;
    *pOut << "\tfcPlcfspaHdr: " << fcPlcfspaHdr << endl1;
    *pOut << "\tlcbPlcfspaHdr: " << lcbPlcfspaHdr << endl1;
    *pOut << "\tfcPlcfAtnbkf: " << fcPlcfAtnbkf << endl1;
    *pOut << "\tlcbPlcfAtnbkf: " << lcbPlcfAtnbkf << endl1;
    *pOut << "\tfcPlcfAtnbkl: " << fcPlcfAtnbkl << endl1;
    *pOut << "\tlcbPlcfAtnbkl: " << lcbPlcfAtnbkl << endl1;
    *pOut << "\tfcPms: " << fcPms << endl1;
    *pOut << "\tlcbPMS: " << lcbPMS << endl1;
    *pOut << "\tfcFormFldSttbf: " << fcFormFldSttbf << endl1;
    *pOut << "\tlcbFormFldSttbf: " << lcbFormFldSttbf << endl1;
    *pOut << "\tfcPlcfendRef: " << fcPlcfendRef << endl1;
    *pOut << "\tlcbPlcfendRef: " << lcbPlcfendRef << endl1;
    *pOut << "\tfcPlcfendTxt: " << fcPlcfendTxt << endl1;
    *pOut << "\tlcbPlcfendTxt: " << lcbPlcfendTxt << endl1;
    *pOut << "\tfcPlcffldEdn: " << fcPlcffldEdn << endl1;
    *pOut << "\tlcbPlcffldEdn: " << lcbPlcffldEdn << endl1;
    *pOut << "\tfcPlcfpgdEdn: " << fcPlcfpgdEdn << endl1;
    *pOut << "\tlcbPlcfpgdEdn: " << lcbPlcfpgdEdn << endl1;
    *pOut << "\tfcDggInfo: " << fcDggInfo << endl1;
    *pOut << "\tlcbDggInfo: " << lcbDggInfo << endl1;
    *pOut << "\tfcSttbfRMark: " << fcSttbfRMark << endl1;
    *pOut << "\tlcbSttbfRMark: " << lcbSttbfRMark << endl1;
    *pOut << "\tfcSttbfCaption: " << fcSttbfCaption << endl1;
    *pOut << "\tlcbSttbfCaption: " << lcbSttbfCaption << endl1;
    *pOut << "\tfcSttbAutoCaption: " << fcSttbAutoCaption << endl1;
    *pOut << "\tlcbSttbAutoCaption: " << lcbSttbAutoCaption << endl1;
    *pOut << "\tfcPlcfwkb: " << fcPlcfwkb << endl1;
    *pOut << "\tlcbPlcfwkb: " << lcbPlcfwkb << endl1;
    *pOut << "\tfcPlcfspl: " << fcPlcfspl << endl1;
    *pOut << "\tlcbPlcfspl: " << lcbPlcfspl << endl1;
    *pOut << "\tfcPlcftxbxTxt: " << fcPlcftxbxTxt << endl1;
    *pOut << "\tlcbPlcftxbxTxt: " << lcbPlcftxbxTxt << endl1;
    *pOut << "\tfcPlcffldTxbx: " << fcPlcffldTxbx << endl1;
    *pOut << "\tlcbPlcffldTxbx: " << lcbPlcffldTxbx << endl1;
    *pOut << "\tfcPlcfHdrtxbxTxt: " << fcPlcfHdrtxbxTxt << endl1;
    *pOut << "\tlcbPlcfHdrtxbxTxt: " << lcbPlcfHdrtxbxTxt << endl1;
    *pOut << "\tfcPlcffldHdrTxbx: " << fcPlcffldHdrTxbx << endl1;
    *pOut << "\tlcbPlcffldHdrTxbx: " << lcbPlcffldHdrTxbx << endl1;
    *pOut << "\tfcPlcfLst: " << fcPlcfLst << endl1;
    *pOut << "\tlcbPlcfLst: " << lcbPlcfLst << endl1;
    *pOut << "\tfcPlfLfo: " << fcPlfLfo << endl1;
    *pOut << "\tlcbPlfLfo: " << lcbPlfLfo << endl1;
    *pOut << "\tfcPlcftxbxBkd: " << fcPlcftxbxBkd << endl1;
    *pOut << "\tlcbPlcftxbxBkd: " << lcbPlcftxbxBkd << endl1;
    *pOut << "\tfcPlcfHdrtxbxBkd: " << fcPlcfHdrtxbxBkd << endl1;
    *pOut << "\tlcbPlcfHdrtxbxBkd: " << lcbPlcfHdrtxbxBkd << endl1;
    *pOut << "\tfcSttbListNames: " << fcSttbListNames << endl1;
    *pOut << "\tlcbSttbListNames: " << lcbSttbListNames << endl1;

    *pOut << "\tpnChpFirst: " << pnChpFirst << endl1;
    *pOut << "\tpnPapFirst: " << pnPapFirst << endl1;
    *pOut << "\tcpnBteChp: " << cpnBteChp << endl1;
    *pOut << "\tcpnBtePap: " << cpnBtePap << endl1;


    *pOut << dec << "END FIB" << endl1 << endl1;
}

//-----------------------------------------
//          class DStyle
//-----------------------------------------

class DStyle: public WW8Style
{
    BYTE nVersion;
public:
    DStyle( SvStream& rStream, WW8Fib& rFib )
            : WW8Style( rStream, rFib ){ nVersion = rFib.nVersion; }
    void Dump1Style( USHORT nNr );
    void Dump();
};

//-----------------------------------------
//      universelle Hilfsroutinen
//-----------------------------------------

static void DumpIt( SvStream& rSt, short nLen )
{
    indent( *pOut, rSt );
    if( nLen <= 0 ){
        *pOut << endl1;
        return;
    }
    while ( nLen  ){
        BYTE c;
        xStrm->Read( &c, sizeof(c) );
        *pOut << "<0x" << hex2 << (USHORT)c << dec2 << "> ";
        nLen--;
    }
    *pOut << endl1;
}

static void DumpItSmall( SvStream& rStrm, short nLen )
{
    if( nLen <= 0 )
        return;

    while ( nLen  )
    {
        BYTE c;

        rStrm.Read( &c, sizeof(c) );

        if( c <= 9 )
            *pOut << (USHORT)c;
        else
            *pOut << "0x" << hex2 << (USHORT)c << dec2;

        nLen--;

        if( nLen )
            *pOut << ',';
    }
}

//-----------------------------------------
//      Hilfsroutinen : Foot-, Endnotes
//-----------------------------------------


#if 0
void DumpText( WW8_CP nStartCp, long nTextLen, WW8ScannerBase* pBase, short nType, char* pName = "" );
static void DumpFtnShort( short nId, long nPos, long nFieldLen )
{
    ASSERT( nId < 261 && nId > 255, "Falsche Id" );

    *pOut << '<' << hex << nPos << dec << ' ';

    *pOut << 'F' << nId << ' ' << WW8GetSprmDumpInfo( nId ).pName << ' ';
/*
    if ( WW8GetSprmDumpInfo( nId ).pOutFnc ){
        WW8GetSprmDumpInfo( nId ).pOutFnc( nId, nFieldLen );    // Rufe AusgabeFunktion
    }
*/
    *pOut << '>' << endl1;

    pPLCFMan->SavePLCF();
    WW8PLCFMan* pOldPLCFMan = pPLCFMan;

    pSBase->pChpPLCF->SeekPos( nPos );
    pSBase->pPapPLCF->SeekPos( nPos );

    DumpText( nPos, nFieldLen, pSBase, MAN_FTN, WW8GetSprmDumpInfo( nId ).pName );

    indent( *pOut, *xStrm );

    pPLCFMan = pOldPLCFMan;             // Attributverwaltung restoren
    pPLCFMan->RestorePLCF();
}
#endif

//-----------------------------------------
//      Hilfsroutinen : SPRMS
//-----------------------------------------


/*
#if 0
static short DumpSprmShort( short nSprmsLen )
{
    long nSprmPos = xStrm->Tell();

    *pOut << '<' << hex << nSprmPos << dec << ' ';

    BYTE x[512];
    xStrm->Read( x, 512 );                  // Token und folgende lesen
    BYTE i = x[0];

    nSprmsLen -= WW8GetSprmSizeBrutto( nVersion, x ); // so viele Sprm-Bytes folgen noch nach diesem Sprm

    if( nSprmsLen < 0 )
        *pOut << "!UEberhang um " << -nSprmsLen << " Bytes!" << endl1;

    *pOut << 'A' << (USHORT)i  << ' ';
    *pOut << aSprmTab[i].pName << ' ';

    xStrm->Seek( nSprmPos + 1 + WW8SprmDataOfs( i ) );// gehe zum eigentlichen
                                                     // Inhalt
    if ( aSprmTab[i].pOutFnc ){
        WW8GetSprmDumpInfo( nId ).pOutFnc( i, WW8GetSprmSizeNetto( x ) );// Rufe AusgabeFunktion
    }else{
        DumpItSmall( WW8GetSprmSizeNetto( x ) );        // oder Dumper
    }
    *pOut << '>';

    return nSprmsLen;
}

static void DumpSprmsShort( short nLen )
{
    if( nLen <= 1 || xStrm->IsEof() ){
        return;
    }
    while ( nLen > 1 ){
        nLen = DumpSprmShort( nLen );
    }
}
#endif
*/




static short DumpSprm( BYTE nVersion, SvStream& rSt, short nSprmsLen )
{
    long nSprmPos = rSt.Tell();
    BYTE nDelta;

    indent( *pOut, rSt );

    BYTE x[512];
    rSt.Read( x, 512 );                 // Token und folgende lesen

    USHORT nId = WW8GetSprmId( nVersion, x, &nDelta );

    short nSprmL = WW8GetSprmSizeBrutto( nVersion, x, &nId );
    short nSprmNL = WW8GetSprmSizeNetto( nVersion, x, &nId );

    nSprmsLen -= nSprmL;
    if( nSprmsLen < 0 )
        *pOut << "!UEberhang um " << -nSprmsLen << " Bytes!" << endl1;

    // Ausgabe: Token in Dez
    if( 8 > nVersion )
        *pOut << (USHORT)x[0];
    else
        *pOut << hex << nId << dec;
    *pOut << '/' << nSprmL;  // Laenge incl. alles in Dez
    *pOut << '/' << nSprmNL;    // Laenge excl Token in Dez


    const SprmDumpInfo& rSprm = WW8GetSprmDumpInfo( nId );

    *pOut << " = " << rSprm.pName << ' ';

    rSt.Seek( nSprmPos + 1 + nDelta + WW8SprmDataOfs( nId ) );// gehe zum eigentlichen
                                                     // Inhalt
    if( rSprm.pOutFnc )
        rSprm.pOutFnc( rSt, nSprmNL );      // Rufe Ausgabefunktion
    else
        DumpItSmall( rSt, nSprmNL );        // oder Dumper
    *pOut << endl1;

    return nSprmsLen;
}

void DumpSprms( BYTE nVersion, SvStream& rSt, short nLen )
{
    if( nLen <= 1 || rSt.IsEof() ){
        return;
    }
    begin( *pOut, rSt ) << "Sprms" << endl1;
    while ( nLen > 1 )
    {
        nLen = DumpSprm( nVersion, rSt, nLen );
    }
    end( *pOut, rSt ) << "Sprms" << endl1;
}

// DumpMemSprm() dumpt ein 2-Byte-Sprm, der im WW8_PCD eingebaut ist
static void DumpMemSprm( BYTE nVersion, INT16* pSprm )
{
    BYTE* p = (BYTE*)pSprm;
    USHORT nId = WW8GetSprmId( nVersion, p, 0 );

    *pOut << (USHORT)p[0];                      // Ausgabe: Token in Dez
    *pOut << '/' << WW8GetSprmSizeBrutto( nVersion, p, &nId );   // Laenge incl. alles in Dez
    *pOut << '/' << WW8GetSprmSizeNetto(  nVersion, p, &nId );    // Laenge excl Token in Dez

    *pOut << " = " << WW8GetSprmDumpInfo( nId ).pName;

    *pOut << " 0x" << hex2 << *(p + 1);
}


//-----------------------------------------
//          Hilfsroutinen: SEPX
//-----------------------------------------
void DumpSepx( BYTE nVersion, long nPos )
{
    USHORT nLen;
    xStrm->Seek( nPos );
    begin( *pOut, *xStrm ) << "Sepx, Len: ";
    xStrm->Read( &nLen, 2 );
    *pOut << nLen << endl1;

    DumpSprms( nVersion, *xStrm, nLen );

    end( *pOut, *xStrm ) << "Sepx" << endl1;
}


//-----------------------------------------
//  Hilfsroutinen: FKP.CHPX, FKP.PAPX
//-----------------------------------------
static void DumpPhe( WW8_PHE_Base& rPhe )
{
    if( rPhe.aBits1 == 0 && rPhe.nlMac == 0
        && SVBT16ToShort( rPhe.dxaCol ) == 0
        && SVBT16ToShort( rPhe.dyl ) == 0 )
    {
        *pOut << indent2 << "  empty PHE" << endl1;
    }
    else
    {
        *pOut << begin2 << "PHE" << endl1;
        *pOut << indent2 << "fSpare: " << ( rPhe.aBits1 & 0x1 );
        *pOut << ", fUnk: " << (( rPhe.aBits1 >> 1 ) & 1 );
        *pOut << ", fDiffLines: " << (( rPhe.aBits1 >> 2 ) & 1 );
        *pOut << ", unused: " << (( rPhe.aBits1 >> 3 ) & 0x1F )
              << " (" << (short)(( rPhe.aBits1 >> 2 ) & 0x3F ) +
                                 (((short)rPhe.nlMac) << 8)
              << ")";
        *pOut << ", nLines: " << (short)rPhe.nlMac << ',' << endl1;
        *pOut << indent2 << "dxaCol: " << (short)SVBT16ToShort( rPhe.dxaCol );
        if( rPhe.aBits1 & 0x4 )
        {
            *pOut << ", total height: " << (USHORT)SVBT16ToShort( rPhe.dyl );
        }
        else
        {
            *pOut << ", height per line: " << (short)SVBT16ToShort( rPhe.dyl );
            *pOut << ", total height: " << rPhe.nlMac * (short)SVBT16ToShort( rPhe.dyl );
        }
        *pOut << endl1;
        *pOut << end2 << "PHE" << endl1;
    }
}

//typedef enum{ CHP=0, PAP, SEP, PLC_END }ePlcT;
static char* NameTab[PLCF_END+4]={ "chpx", "papx", "sepx", "head",
                                    "FootnoteRef", "EndnoteRef",
                                    "AnnotationRef" };

void DumpFkp( BYTE nVersion, long nPos, short nItemSize, ePLCFT ePlc )
{
    char nElem;

    xStrm->Seek( nPos+511 );
    xStrm->Read( &nElem, 1 );


    *pOut << 'D' << hex6 << nPos << dec2 << ' ' << indent1 << begin1 << "Fkp.";
    *pOut << NameTab[ePlc] << ", ";
    *pOut << (USHORT)nElem << " Elements" << endl1;

    int i;
    WW8_FC aF[2];
    BYTE c;
    long nStartOfs = nPos + ( nElem + 1 ) * 4;  // bei dieser Pos faengt Offset-Array an
    short nOfs;
    WW8_PHE_Base aPhe;

    for( i=0; i<nElem; i++ )
    {
        xStrm->Seek( nPos + i * 4 );
        indent( *pOut, *xStrm );
        xStrm->Read( aF, sizeof( aF ) );            // lese 2 FCs

        xStrm->Seek( nStartOfs + i * nItemSize );
        xStrm->Read( &c, 1 );       // lese Word Offset ( evtl. Teil von BX )
        if( ePlc == PAP )
            xStrm->Read( &aPhe, sizeof( aPhe ) );

        nOfs= c * 2;                            // -> Byte Offset
        xStrm->Seek( nPos + nOfs );
        xStrm->Read( &c, 1 );

        if( !c )
            xStrm->Read( &c, 1 );

        *pOut << "Fc: " << hex << aF[0] << ".." << aF[1];
        *pOut << " Cp: " << pSBase->WW8Fc2Cp( aF[0] )
              << ".."    << pSBase->WW8Fc2Cp( aF[1] );
        *pOut << " Offset: " << nOfs << dec2;
        if ( nOfs )
        {
            switch ( ePlc )
            {
            case CHP:
                *pOut << ", Len: " << (short)c << endl1;
                DumpSprms( nVersion, *xStrm, (short)c );
                break;

            case PAP:
                short nLen = c * 2;     // So weit bis zum naechsten !

                short nIStd;

                WW8ReadINT16( *xStrm, nIStd );

                *pOut << ", Len max: " << nLen << ", ID:" << nIStd << endl1;
                DumpSprms( nVersion, *xStrm, nLen - 2 );
                DumpPhe( aPhe );
                break;
            }
        }
        else
        {
            *pOut << dec2 << " No attribute" << endl1;
        }

    }
    end( *pOut, *xStrm ) << "Fkp." << NameTab[ePlc] << endl1;
}

//-----------------------------------------
//      WW8_DOP ( Document Properties )
//-----------------------------------------

//!!! WW8LoadDop gibt's jetzt auch in WW8scan.cxx
void DumpDop( WW8Fib& rFib )
{
    // nicht loeschen!
    xTableStream->Seek( rFib.fcDop );   // diese Aktion dient lediglich dazu, die
                        // Positions-Ausgabe  der folgenden Anweisung
                        // mit dem alten Dumper uebereinstimmen zu lassen.

//  *pOut << begin( *pOut, *xTableStream ) << "Dop, Size " << rFib.lcbDop << ", DefaultSize 84" << endl1;
    begin( *pOut, *xTableStream ) << "Dop, Size " << rFib.lcbDop << ", DefaultSize 84" << endl1;

    WW8Dop* pD = new WW8Dop( *xTableStream, rFib.nFib, rFib.fcDop, rFib.lcbDop );

    *pOut << indent2 << "fFacingPages : " << pD->fFacingPages << endl1;
    *pOut << indent2 << "fWidowControl : " << pD->fWidowControl << endl1;
    *pOut << indent2 << "fPMHMainDoc : " << pD->fPMHMainDoc << endl1;
    *pOut << indent2 << "grfSuppression : " << pD->grfSuppression << endl1;
    *pOut << indent2 << "fpc : " << pD->fpc << endl1;
    *pOut << indent2 << "grpfIhdt : " << pD->grpfIhdt << endl1;
    *pOut << indent2 << "rncFtn : " << pD->rncFtn << endl1;
    *pOut << indent2 << "nFtn : " << pD->nFtn << endl1;
    *pOut << indent2 << "fOutlineDirtySave : " << pD->fOutlineDirtySave << endl1;
    *pOut << indent2 << "fOnlyMacPics : " << pD->fOnlyMacPics << endl1;
    *pOut << indent2 << "fOnlyWinPics : " << pD->fOnlyWinPics << endl1;
    *pOut << indent2 << "fLabelDoc : " << pD->fLabelDoc << endl1;
    *pOut << indent2 << "fHyphCapitals : " << pD->fHyphCapitals << endl1;
    *pOut << indent2 << "fAutoHyphen : " << pD->fAutoHyphen << endl1;
    *pOut << indent2 << "fFormNoFields : " << pD->fFormNoFields << endl1;
    *pOut << indent2 << "fLinkStyles : " << pD->fLinkStyles << endl1;
    *pOut << indent2 << "fRevMarking : " << pD->fRevMarking << endl1;
    *pOut << indent2 << "fBackup : " << pD->fBackup << endl1;
    *pOut << indent2 << "fExactCWords : " << pD->fExactCWords << endl1;
    *pOut << indent2 << "fPagHidden : " << pD->fPagHidden << endl1;
    *pOut << indent2 << "fPagResults : " << pD->fPagResults << endl1;
    *pOut << indent2 << "fLockAtn : " << pD->fLockAtn << endl1;
    *pOut << indent2 << "fMirrorMargins : " << pD->fMirrorMargins << endl1;
    *pOut << indent2 << "fReadOnlyRecommended : " << pD->fReadOnlyRecommended << endl1;
    *pOut << indent2 << "fDfltTrueType : " << pD->fDfltTrueType << endl1;
    *pOut << indent2 << "fPagSuppressTopSpacing : " << pD->fPagSuppressTopSpacing << endl1;
    *pOut << indent2 << "fProtEnabled : " << pD->fProtEnabled << endl1;
    *pOut << indent2 << "fDispFormFldSel : " << pD->fDispFormFldSel << endl1;
    *pOut << indent2 << "fRMView : " << pD->fRMView << endl1;
    *pOut << indent2 << "fRMPrint : " << pD->fRMPrint << endl1;
    *pOut << indent2 << "fWriteReservation : " << pD->fWriteReservation << endl1;
    *pOut << indent2 << "fLockRev : " << pD->fLockRev << endl1;
    *pOut << indent2 << "fEmbedFonts : " << pD->fEmbedFonts << endl1;
    *pOut << indent2 << "copts_fNoTabForInd : " << pD->copts_fNoTabForInd << endl1;
    *pOut << indent2 << "copts_fNoSpaceRaiseLower : " << pD->copts_fNoSpaceRaiseLower << endl1;
    *pOut << indent2 << "copts_fSupressSpbfAfterPgBrk : " << pD->copts_fSupressSpbfAfterPgBrk << endl1;
    *pOut << indent2 << "copts_fWrapTrailSpaces : " << pD->copts_fWrapTrailSpaces << endl1;
    *pOut << indent2 << "copts_fMapPrintTextColor : " << pD->copts_fMapPrintTextColor << endl1;
    *pOut << indent2 << "copts_fNoColumnBalance : " << pD->copts_fNoColumnBalance << endl1;
    *pOut << indent2 << "copts_fConvMailMergeEsc : " << pD->copts_fConvMailMergeEsc << endl1;
    *pOut << indent2 << "copts_fSupressTopSpacing : " << pD->copts_fSupressTopSpacing << endl1;
    *pOut << indent2 << "copts_fOrigWordTableRules : " << pD->copts_fOrigWordTableRules << endl1;
    *pOut << indent2 << "copts_fTransparentMetafiles : " << pD->copts_fTransparentMetafiles << endl1;
    *pOut << indent2 << "copts_fShowBreaksInFrames : " << pD->copts_fShowBreaksInFrames << endl1;
    *pOut << indent2 << "copts_fSwapBordersFacingPgs : " << pD->copts_fSwapBordersFacingPgs << endl1;
    *pOut << indent2 << "dxaTab : " << pD->dxaTab << endl1;
    *pOut << indent2 << "wSpare : " << pD->wSpare << endl1;
    *pOut << indent2 << "dxaHotZ : " << pD->dxaHotZ << endl1;
    *pOut << indent2 << "cConsecHypLim : " << pD->cConsecHypLim << endl1;
    *pOut << indent2 << "wSpare2 : " << pD->wSpare2 << endl1;
    *pOut << indent2 << "dttmCreated : " << pD->dttmCreated << endl1;
    *pOut << indent2 << "dttmRevised : " << pD->dttmRevised << endl1;
    *pOut << indent2 << "dttmLastPrint : " << pD->dttmLastPrint << endl1;
    *pOut << indent2 << "nRevision : " << pD->nRevision << endl1;
    *pOut << indent2 << "tmEdited : " << pD->tmEdited << endl1;
    *pOut << indent2 << "cWords : " << pD->cWords << endl1;
    *pOut << indent2 << "cCh : " << pD->cCh << endl1;
    *pOut << indent2 << "cPg : " << pD->cPg << endl1;
    *pOut << indent2 << "cParas : " << pD->cParas << endl1;
    *pOut << indent2 << "rncEdn : " << pD->rncEdn << endl1;
    *pOut << indent2 << "nEdn : " << pD->nEdn << endl1;
    *pOut << indent2 << "epc : " << pD->epc << endl1;
    *pOut << indent2 << "fPrintFormData : " << pD->fPrintFormData << endl1;
    *pOut << indent2 << "fSaveFormData : " << pD->fSaveFormData << endl1;
    *pOut << indent2 << "fShadeFormData : " << pD->fShadeFormData << endl1;
    *pOut << indent2 << "fWCFtnEdn : " << pD->fWCFtnEdn << endl1;
    *pOut << indent2 << "cLines : " << pD->cLines << endl1;
    *pOut << indent2 << "cWordsFtnEnd : " << pD->cWordsFtnEnd << endl1;
    *pOut << indent2 << "cChFtnEdn : " << pD->cChFtnEdn << endl1;
    *pOut << indent2 << "cPgFtnEdn : " << pD->cPgFtnEdn << endl1;
    *pOut << indent2 << "cParasFtnEdn : " << pD->cParasFtnEdn << endl1;
    *pOut << indent2 << "cLinesFtnEdn : " << pD->cLinesFtnEdn << endl1;
    *pOut << indent2 << "lKeyProtDoc : " << pD->lKeyProtDoc << endl1;
    *pOut << indent2 << "wvkSaved : " << pD->wvkSaved << endl1;
    *pOut << indent2 << "wScaleSaved : " << pD->wScaleSaved << endl1;
    *pOut << indent2 << "zkSaved : " << pD->zkSaved << endl1;
    *pOut << indent2 << "fNoTabForInd : " << pD->fNoTabForInd << endl1;
    *pOut << indent2 << "fNoSpaceRaiseLower : " << pD->fNoSpaceRaiseLower << endl1;
    *pOut << indent2 << "fSupressSpbfAfterPageBreak : " << pD->fSupressSpbfAfterPageBreak << endl1;
    *pOut << indent2 << "fWrapTrailSpaces : " << pD->fWrapTrailSpaces << endl1;
    *pOut << indent2 << "fMapPrintTextColor : " << pD->fMapPrintTextColor << endl1;
    *pOut << indent2 << "fNoColumnBalance : " << pD->fNoColumnBalance << endl1;
    *pOut << indent2 << "fConvMailMergeEsc : " << pD->fConvMailMergeEsc << endl1;
    *pOut << indent2 << "fSupressTopSpacing : " << pD->fSupressTopSpacing << endl1;
    *pOut << indent2 << "fOrigWordTableRules : " << pD->fOrigWordTableRules << endl1;
    *pOut << indent2 << "fTransparentMetafiles : " << pD->fTransparentMetafiles << endl1;
    *pOut << indent2 << "fShowBreaksInFrames : " << pD->fShowBreaksInFrames << endl1;
    *pOut << indent2 << "fSwapBordersFacingPgs : " << pD->fSwapBordersFacingPgs << endl1;
    *pOut << indent2 << "fSuppressTopSpacingMac5 : " << pD->fSuppressTopSpacingMac5 << endl1;
    *pOut << indent2 << "fTruncDxaExpand : " << pD->fTruncDxaExpand << endl1;
    *pOut << indent2 << "fPrintBodyBeforeHdr : " << pD->fPrintBodyBeforeHdr << endl1;
    *pOut << indent2 << "fNoLeading : " << pD->fNoLeading << endl1;
    *pOut << indent2 << "fMWSmallCaps : " << pD->fMWSmallCaps << endl1;

    *pOut << indent2 << "adt : " << pD->adt << endl1;

    *pOut << indent2 << "lvl : " << pD->lvl << endl1;
    *pOut << indent2 << "fHtmlDoc : " << pD->fHtmlDoc << endl1;
    *pOut << indent2 << "fSnapBorder : " << pD->fSnapBorder << endl1;
    *pOut << indent2 << "fIncludeHeader : " << pD->fIncludeHeader << endl1;
    *pOut << indent2 << "fIncludeFooter : " << pD->fIncludeFooter << endl1;
    *pOut << indent2 << "fForcePageSizePag : " << pD->fForcePageSizePag << endl1;
    *pOut << indent2 << "fMinFontSizePag : " << pD->fMinFontSizePag << endl1;

    *pOut << indent2 << "fHaveVersions : " << pD->fHaveVersions << endl1;
    *pOut << indent2 << "fAutoVersion : " << pD->fAutoVersion << endl1;
    *pOut << indent2 << "cChWS : " << pD->cChWS << endl1;
    *pOut << indent2 << "cChWSFtnEdn : " << pD->cChWSFtnEdn << endl1;
    *pOut << indent2 << "grfDocEvents : " << pD->grfDocEvents << endl1;
    *pOut << indent2 << "cDBC : " << pD->cDBC << endl1;
    *pOut << indent2 << "cDBCFtnEdn : " << pD->cDBCFtnEdn << endl1;
    *pOut << indent2 << "nfcFtnRef : " << pD->nfcFtnRef << endl1;
    *pOut << indent2 << "nfcEdnRef : " << pD->nfcEdnRef << endl1;
    *pOut << indent2 << "hpsZoonFontPag : " << pD->hpsZoonFontPag << endl1;
    *pOut << indent2 << "dywDispPag : " << pD->dywDispPag << endl1;

    end( *pOut, *xStrm ) << "Dop" << endl1 << endl1;

    DELETEZ( pD );
}

//-----------------------------------------
//    PLCF.PCD ( Piece Table )
//-----------------------------------------

#ifdef __WW8_NEEDS_PACK
#  pragma pack(2)
#endif

struct WW8_PCD1
{
    BYTE aBits1;
    BYTE aBits2;
//  INT16 fNoParaLast : 1;  // when 1, means that piece contains no end of parag
//  BYTE fPaphNil : 1;      // used internally by Word
//  BYTE fCopied : 1;       // used internally by Word
    //          *   int :5
//  BYTE aBits2;            // fn int:8, used internally by Word
    INT32 fc;               // file offset of beginning of piece. The size of th
    INT16 prm;              // PRM contains either a single sprm or else an inde
};

#ifdef __WW8_NEEDS_PACK
#  pragma pack()
#endif

static void DumpPLCFPcd( BYTE nVersion, long nPos, long nLen )
{
    WW8PLCF aPlc( &xTableStream, nPos , nLen, 8 );

    *pOut << 'D' << hex6 << nPos << dec2 <<  ' ' << indent1 << begin1;
    *pOut << "Plcx.Pcd, Len: ";
    *pOut << nLen << ", " << aPlc.GetIMax() << " Elements" << endl1;

    int i;
    for( i=0; i<aPlc.GetIMax(); i++ )
    {
        long start, ende;
        void* pData;
        aPlc.Get( start, ende, pData );
        *pOut << indent2 << " Cp: " << hex6 << start << ".." << hex6 << ende;

        WW8_PCD1* p = (WW8_PCD1*) pData;
        *pOut << ", Bits: " << hex2  <<  (USHORT)p->aBits1 << ' '
              << hex2 << (USHORT)p->aBits2;
        *pOut << ", FcStart: ";
        if( 8 <= nVersion )
        {
            BOOL bUniCode;
            *pOut << hex6
                  << WW8PLCFx_PCD::TransformPieceAddress( p->fc, bUniCode );
            if( bUniCode )
                *pOut << " (UniCode)";
        }
        else
            *pOut << hex6 << p->fc;

        *pOut << dec << ", prm ";
        if( p->prm & 0x1 ){
            *pOut << "No: " << ( p->prm >> 1 );
        }else if ( p-> prm == 0 ){
            *pOut << "-";
        }else{
            *pOut << "(Sprm): ";
            DumpMemSprm( nVersion, &(p->prm) );
        }
        *pOut << endl1;

        aPlc++;
    }
    end( *pOut, *xTableStream ) << "Plcx.Pcd" << endl1;
}

static void DumpPcd( BYTE nVersion, long nPos, long nLen )
{
    long nLen1 = nLen;
    xTableStream->Seek( nPos );

    *pOut << 'D' << hex6 << nPos << dec2 <<  ' ' << indent1 << begin1;
    *pOut << "PieceTable, Len: " << nLen << endl1;
    if( sizeof( WW8_PCD1 ) != 8 )
        *pOut << "!!! Alignment-Problem !!! sizeof( WW8_PCD1 ) != 8 !!! " << endl1;

    long i = 0;
    while (1){
        BYTE c;
        INT16 cb;

        xTableStream->Read( &c, 1 );
        nLen1 --;
        if( c == 2 )
            break;
        xTableStream->Read( &cb, 2 );
        nLen1 -= 2;
        indent( *pOut, *xTableStream ) << "grpprl No. " << i << ", Len: " << cb << endl1;

        long nPos = xTableStream->Tell();
        DumpSprms( nVersion, *xTableStream, cb );                   // Dumpe Sprms
        xTableStream->Seek( nPos + cb );            // gehe hinter grpprl
        nLen1 -= cb;
        i++;
//      *pOut << "       " << indent1 << "grpprl: " << cb << "Bytes ueberlesen";
//      *pOut << endl1;
    }
    INT32 nLen2;
    xTableStream->Read( &nLen2, 4 );
    nLen1 -= 4;
    if( nLen1 != nLen2 )
        *pOut << "!!! nLen1 ( " << nLen1 << " ) != nLen2 ( " << nLen2 << " ) !"
              << endl1;

    DumpPLCFPcd( nVersion, xTableStream->Tell(), nLen2 );

    end( *pOut, *xTableStream ) << "PieceTable" << endl1 << endl1;
}

//-----------------------------------------
//    PLCF.CHPX, PLCF.PAPX, PLCF.SEPX
//-----------------------------------------

static void DumpPLCF( long nPos, long nLen, ePLCFT ePlc )
{
    static int __READONLY_DATA WW8FkpSizeTabVer6[ PLCF_END ] = {
                                                    1,  7, 0 /*, 0, 0, 0*/ };
    static int __READONLY_DATA PlcSizeTabVer6[ PLCF_END+4 ] = {
                                                    2, 2, 12, 0, 2, 2, 20 };

    static int __READONLY_DATA WW8FkpSizeTabVer8[ PLCF_END ] = {
                                                    1, 13, 0 /*, 0, 0, 0*/ };
    static int __READONLY_DATA PlcSizeTabVer8[ PLCF_END+4 ] = {
                                                    4, 4, 12, 0, 2, 2, 30 };

    const int* pFkpSizeTab;
    const int* pPlcSizeTab;

    switch( pWwFib->nVersion )
    {
    case 6:
    case 7: pFkpSizeTab = WW8FkpSizeTabVer6;
            pPlcSizeTab = PlcSizeTabVer6;
            break;
    case 8: pFkpSizeTab = WW8FkpSizeTabVer8;
            pPlcSizeTab = PlcSizeTabVer8;
            break;
    default:// Programm-Fehler!
                    /*
                        ACHTUNG: im FILTER nicht "FALSE" sondern "!this()" schreiben,
                                            da sonst Warning unter OS/2
                    */
                    ASSERT( FALSE, "Es wurde vergessen, nVersion zu kodieren!" );
                    return;
    }

//  SvStream* pSt = (SEP == ePlc) ? &xTableStream : &xStrm;
    SvStream* pSt = &xTableStream;

    WW8PLCF aPlc( pSt, nPos, nLen, pPlcSizeTab[ ePlc ] );

    *pOut << 'T' << hex6 << nPos << dec2 <<  ' ' << indent1 << begin1;
    *pOut << "Plcx." << NameTab[ePlc] << ", Len: ";
    *pOut << nLen << ", " << aPlc.GetIMax() << " Elements" << endl1;

    int i;
    for( i=0; i<aPlc.GetIMax(); i++ )
    {
        long start, ende;
        void* pData;
        aPlc.Get( start, ende, pData );
        *pOut << indent2 << "Plcx." << NameTab[ePlc];
        switch( ePlc ){
        case SEP: {
                        *pOut << " Cp: " << hex << start << ".." << ende ;
                        long nPo = *((long*)((char*)pData+2));
                    if( nPo == 0xffffffffL )
                        {
                          *pOut << " Empty" << endl1;
                    }
                        else
                        {
                            *pOut << ", Sepx-Position: " << nPo << dec << endl1;
                        DumpSepx( pWwFib->nVersion, nPo );
                    }
                  }
                  break;
        case CHP:
        case PAP: {
                        *pOut << " Fc: " << hex << start << ".." << ende ;
                        long nPo;
                        switch( pWwFib->nVersion )
                        {
                        case 6:
                        case 7: nPo = *((USHORT*)pData);
                                        break;
                        case 8: nPo = *((UINT32*)pData);
                                        break;
                        default:// Programm-Fehler!
                                        /*
                                            ACHTUNG: im FILTER nicht "FALSE" sondern "!this()" schreiben,
                                                                da sonst Warning unter OS/2
                                        */
                                        ASSERT( FALSE, "Es wurde vergessen, nVersion zu kodieren!" );
                                        return;
                        }

                    nPo <<= 9;
                    *pOut << ", Offset: " << nPo << dec << endl1;
                    DumpFkp( pWwFib->nVersion, nPo, pFkpSizeTab[ ePlc ], ePlc );
                  }
                  break;

        case PLCF_END+0:
                  *pOut << " Cp: " << hex << start << ".." << ende << dec << endl1;
                  break;

        case PLCF_END+1:
        case PLCF_END+2:
            {
                *pOut << " Cp: " << hex << start;
                USHORT nFlags= *((USHORT*)pData);
                *pOut << ", Flags: 0x" << nFlags << dec << endl1;
            }
            break;

        case PLCF_END+3:
            {
                *pOut << " Cp: " << hex << start << ", Initial: \"";
                const char* p = (char*)pData;
                BYTE n = *p++;
                while( n-- )
                {
                    if( 8 == pWwFib->nVersion )
                        ++p;
                    *pOut << *p++;
                }

                long nBkmkId;
                USHORT nId;
                if( 8 == pWwFib->nVersion )
                {
                    nId = SVBT16ToShort( ((WW8_ATRD*)pData)->ibst );
                    nBkmkId = SVBT32ToLong( ((WW8_ATRD*)pData)->ITagBkmk );
                }
                else
                {
                    nId = SVBT16ToShort( ((WW67_ATRD*)pData )->ibst );
                    nBkmkId = SVBT32ToLong( ((WW67_ATRD*)pData )->ITagBkmk );
                }

                *pOut << "\", AutorId: " << hex << nId
                      << " BkmkId: " << nBkmkId << dec <<endl1;
            }
            break;

        }
        aPlc++;
    }
    end( *pOut, *xTableStream ) << "Plcx." << NameTab[ePlc] << endl1 << endl1;
}

//-----------------------------------------------------
//  PLCF.CHPX, PLCF.PAPX unter  Benutzung von PLCF_Fkp
//-----------------------------------------------------
#if 0
static void DumpPLCF2( long nPos, long nLen, ePlcT ePlc )
{
    *pOut << 'T' << hex6 << nPos << dec2 <<  ' ' << indent1 << begin1;
    *pOut << "Plcx." << NameTab[ePlc] << endl1;

    PLCF* pPLCF;
    switch( ePlc ){
    case SEP: pPLCF = new PLCF_Sepx( nPos, nLen, ePlc );
              break;
    default:  pPLCF = new PLCF_Fkp( nPos, nLen, ePlc );
              break;
    }

    while(1){
        long nSt, nE;
        short nL;

        WW8_CP nP = pPLCF->WhereNext();
        pPLCF->SeekNextSprms( nSt, nE, nL );            // wo sind naechste Sprms

        if ( nSt == LONG_MAX )
            break;                              // fertig

        *pOut << "       " << indent1 << "Cp: " << hex << nP <<" Cp: " << nSt << ".." << nE << dec;

        if( nL ){
            *pOut << ", Len: " << nL << endl1;
            DumpSprms( *xStrm, nL );
        }else{
            *pOut << ", No Attribute" << endl1;
        }
    }
    end( *pOut, *xStrm ) << "Plcx." << NameTab[ePlc] << endl1 << endl1;
    DELETEZ( pPLCF );
}
#endif
//-----------------------------------------
//              Text ohne Attribute
//-----------------------------------------

static void DumpPlainText1( WW8_CP nStartCp, long nTextLen )
{
    long l;
    BYTE c;


    xStrm->Seek( pSBase->WW8Cp2Fc( nStartCp ) );

    for (l=0; l<nTextLen; l++)
    {
        xStrm->Read( &c, sizeof(c) );
        if ( DumpChar( c ) )
        {
            *pOut << endl1;
            indent( *pOut, *xStrm );
        }
//      DumpChar( c );
    }
}

void DumpShortPlainText( WW8_CP nStartCp, long nTextLen, char* pName )
{
    indent( *pOut, *xStrm ) << pName << " PlainText: Cp: "
                                    << hex << nStartCp
                                    << ".." << nStartCp + nTextLen << dec
                                    << " :\"";
    DumpPlainText1( nStartCp, nTextLen );
    *pOut << "\"" << endl1;
}

void DumpPlainText( WW8_CP nStartCp, long nTextLen, char* pName )
{
    begin( *pOut, *xStrm ) << pName << " Text: Cp: " << hex << nStartCp
                                    << ".." << nStartCp + nTextLen << dec
                                    << endl1;
    indent( *pOut, *xStrm );
    DumpPlainText1( nStartCp, nTextLen );
    *pOut << endl1;
    end( *pOut, *xStrm ) << pName << " Text" << endl1;
}

//-----------------------------------------
//              Text mit Attributen
//-----------------------------------------

BOOL DumpChar( BYTE c )
{
    if ( ( c >= 32 ) && ( c <= 127 ) ){
        *pOut << c;
        return FALSE;
    }else{
        switch (c){
        case 0xe4:                          // dt. Umlaute
        case 0xf6:
        case 0xfc:
        case 0xdf:
        case 0xc4:
        case 0xd6:
        case 0xdc: *pOut << c; return FALSE;

        case 0xd: *pOut << "<CR>";
                  return TRUE;
        case 0x7:
        case 0xc:  *pOut << "<0x" << hex2 << (USHORT)c << dec2 << '>';
                  return TRUE;
        default:  *pOut << "<0x" << hex2 << (USHORT)c << dec2 << '>';
                  return FALSE;
        }
    }
}

#if 0
void DumpText2( long nTextStart, long nTextLen, long nPlcPos, long nPlcLen, ePlcT ePlc )
{
    long l;
    BYTE c;

    PLCF aPLCF( nPlcPos, nPlcLen, ePlc );
    long nNxStart = aPLCF.WhereNext();
    long nNxEnd = LONG_MAX;

    xStrm->Seek( nTextStart );

    begin( *pOut, *xStrm ) << "Text" << endl1;
    indent( *pOut, xStrm );
    for (l=0; l<nTextLen; l++){

        if( nTextStart + l == nNxStart ){
            long nOld = xStrm->Tell();
            short nL;

            aPLCF.SeekNext( nNxStart, nNxEnd, nL );         // wo ist naechstes Attribut
            if( nL ){
                *pOut << '<';
                DumpSprmsShort( nL );
                *pOut << '>';
            }else{
                *pOut << "<No Attribute>";
            }

            nNxStart = aPLCF.WhereNext();
            xStrm->Seek( nOld );
        }
        if( nTextStart + l == nNxEnd )
            *pOut << "<End Attribute>";

        xStrm->Read( &c, sizeof(c) );
        if ( DumpChar( c ) )
        {
            *pOut << endl1;
            indent( *pOut, xStrm );
        }
    }
    *pOut << endl1;
    end( *pOut, *xStrm ) << "Text" << endl1 << endl1;
}
#endif
#if 0
long DumpTxtAttr( WW8PLCFMan& rMan, BOOL& bNl )
{
    long nPos, nNext, nOld = xStrm->Tell();
    short nId;
    long nL;
    BYTE nFlags;

    BOOL b = rMan.SeekNext( nPos, nL, nId, nFlags );    // gehe zu Attribut

    if( nFlags & MAN_MASK_NEW_SEP )             // neue Section
        *pOut << "<Section Break>";

/*  if( nFlags & MAN_MASK_NEW_PAP )             // neuer Absatz
        *pOut << "<ParaStyle:" << pPLCFMan->GetColl( MAN_NUM_PARACOLL ) << '>' << endl1;
*/
    if( nId != -1 && nId != -11 ){                      // leere Attrs ignorieren

        if ( bNl )
            *pOut << hex6 << nOld << ' ' << indent1;
        bNl = FALSE;

        if( nId < 256 ){
            if( b ){                                    // WW-Attribute
                if( nL >= 0 ){                          // Attr-Anfang
                    xStrm->Seek( nPos );
                    DumpSprmShort( (short)nL );
                }else {
                    *pOut << "<A" << nId << " No Attr>";
                }
            }else{                                      // Attr-Ende
                *pOut << "<End A" << nId << '>';
            }
        }else if( nId < 260 ){
            DumpFtnShort( nId, nPos, nL );            // Footnotes u.ae.
        }else{
            *pOut << "<Field>";
        }
    }

    nNext = rMan.WhereNext();
    xStrm->Seek( nOld );
    return nNext;
}

void DumpText( WW8_CP nStartCp, long nTextLen, WW8ScannerBase* pBase, short nType, char* pName )
{
    pPLCFMan = new WW8PLCFMan( pBase, nType );

    long l;
    BYTE c;

    long nNext = pPLCFMan->WhereNext();

    xStrm->Seek( WW8Cp2Fc( nStartCpp, pWwFib->fcMin, 8 != pWwFib->nVersion ) );

    begin( *pOut, *xStrm ) << pName << "Text" << endl1;
    BOOL bNl = TRUE;
    for (l=nStartCp; l<nStartCp+nTextLen; l++){

        while( l >= nNext )
            nNext = DumpTxtAttr( *pPLCFMan, bNl );

        if ( bNl ) indent( *pOut, xStrm );

        xStrm->Read( &c, sizeof(c) );
        bNl = DumpChar( c );
        if ( bNl ){
            *pOut << "<ParaStyle:" << pPLCFMan->GetColl( MAN_NUM_PARACOLL ) << '>' << endl1;
        }
    }

    while( l >= nNext )
        nNext = DumpTxtAttr( *pPLCFMan, bNl );

    if( !bNl )
        *pOut << endl1;

    end( *pOut, *xStrm ) << pName << "Text" << endl1;

    DELETEZ( pPLCFMan );
}
#endif

//-----------------------------------------
//      Header / Footer
//-----------------------------------------

static void DumpPlcText( WW8_FC nPos, long nLen, long nOfs,
                        char* pName, WW8ScannerBase* pBase,
                        long nStruct = 0, FNDumpData pOutFnc = 0 )
{
    if( !nLen ) return;

    WW8PLCF aPlc( &xTableStream, nPos , nLen, nStruct );

    *pOut << 'T' << hex6 << nPos << dec2 <<  ' ' << indent1 << begin1;
    *pOut << "Plc." << pName << " Text, Len: ";
    *pOut << nLen << ", " << aPlc.GetIMax() << " Elements" << endl1;

    int i;
    for( i=0; i < aPlc.GetIMax(); ++i )
    {
        WW8_CP start, ende;
        WW8_FC start2, ende2;
        void* pData;
        aPlc.Get( start, ende, pData );
        start2 = pSBase->WW8Cp2Fc(start + nOfs );
        ende2 =  pSBase->WW8Cp2Fc(ende + nOfs );

        *pOut << "       " << indent1 << "Plcx." << pName;
        *pOut << " Cp: " << hex << start << ".." << ende;
        *pOut << " entspricht Fc: " << start2 << ".." << ende2 << dec << endl1;

        DumpPlainText( nOfs + start, ende - start - 1, pName );

        if( pOutFnc )
            (*pOutFnc)( pData );
        aPlc++;
    }
    end( *pOut, *xTableStream ) << "Plcx." << pName << endl1 << endl1;
}

void DumpHeader( WW8ScannerBase* pBase )
{
    DumpPlcText( pWwFib->fcPlcfhdd,
                 pWwFib->lcbPlcfhdd,
                 pWwFib->ccpText + pWwFib->ccpFtn,
                 "Header/Footer", pBase );
}

static void DumpFootnotes( WW8ScannerBase* pBase )
{
    if( !pWwFib->lcbPlcffndRef ){
        *pOut << "No Footnotes" << endl1 << endl1;
        return;
    }

    DumpPLCF( pWwFib->fcPlcffndRef,
              pWwFib->lcbPlcffndRef,
              ePLCFT(/*FNR*/PLCF_END + 1) );
    DumpPlcText( pWwFib->fcPlcffndTxt,
                 pWwFib->lcbPlcffndTxt,
                 pWwFib->ccpText,
                 "FootNote" , pBase );
}

static void DumpEndnotes( WW8ScannerBase* pBase )
{
    if( !pWwFib->lcbPlcfendRef ){
        *pOut << "No Endnotes" << endl1 << endl1;
        return;
    }

    DumpPLCF( pWwFib->fcPlcfendRef,
              pWwFib->lcbPlcfendRef,
              ePLCFT(/*ENR*/PLCF_END + 2) );
    DumpPlcText( pWwFib->fcPlcfendTxt,
                 pWwFib->lcbPlcfendTxt,
                 pWwFib->ccpText + pWwFib->ccpFtn
                    + pWwFib->ccpHdr + pWwFib->ccpAtn,
                 "EndNote", pBase );
}

static void DumpAnnotations( WW8ScannerBase* pBase )
{
    if( !pWwFib->lcbPlcfandRef ){
        *pOut << "No Annotations" << endl1 << endl1;
        return;
    }

    DumpPLCF( pWwFib->fcPlcfandRef,
              pWwFib->lcbPlcfandRef,
              ePLCFT(/*ENR*/PLCF_END + 3) );
    DumpPlcText( pWwFib->fcPlcfandTxt,
                 pWwFib->lcbPlcfandTxt,
                 pWwFib->ccpText + pWwFib->ccpFtn
                    + pWwFib->ccpHdr,
                 "Annotation", pBase );
}

void DumpTxtStoryEntry( void* pData )
{
    if( 8 == pWwFib->nVersion )
    {
        long* p = (long*)pData;
        begin( *pOut, *xStrm )
            << "TextboxStory" << hex << endl1;

        *pOut << "       " << indent1 << "cTxbx/iNextReuse: 0x" << *p++;
        *pOut << " cReusable: 0x" << *p++;
        short* ps = (short*)p;
        *pOut << " fReusable: 0x" << *ps++ << endl1;
        p = (long*)ps;
        ++p;        // reserved
        *pOut << "       " << indent1 << "lid: 0x" << *p++;
        *pOut << " txidUndo: 0x" << *p++ << dec << endl1;

        end( *pOut, *xStrm ) << "TextboxStory" << endl1 << endl1;
    }
}

static void DumpTextBoxs( WW8ScannerBase* pBase )
{
    if( pWwFib->lcbPlcftxbxTxt )
    {
        DumpPlcText( pWwFib->fcPlcftxbxTxt,
                     pWwFib->lcbPlcftxbxTxt,
                     pWwFib->ccpText + pWwFib->ccpFtn
                        + pWwFib->ccpHdr + pWwFib->ccpAtn + pWwFib->ccpEdn,
                     "TextBoxes", pBase,
                     8 == pWwFib->nVersion ? 22 : 0, &DumpTxtStoryEntry );
    }
    else
        *pOut << "No Textboxes" << endl1 << endl1;

    if( pWwFib->lcbPlcfHdrtxbxTxt )
        DumpPlcText( pWwFib->fcPlcfHdrtxbxTxt,
                     pWwFib->lcbPlcfHdrtxbxTxt,
                     pWwFib->ccpText + pWwFib->ccpFtn
                        + pWwFib->ccpHdr + pWwFib->ccpAtn + pWwFib->ccpEdn
                        + pWwFib->ccpTxbx,
                     "HeaderTextBoxes", pBase,
                     8 == pWwFib->nVersion ? 22 : 0, &DumpTxtStoryEntry );
    else
        *pOut << "No HeaderTextboxes" << endl1 << endl1;

}


static void DumpDrawObjects( const char* pNm, long nStart, long nLen,
                            long nOffset )
{
    if( nStart && nLen )
    {
        WW8PLCFspecial aPLCF( &xTableStream, nStart, nLen,
                                8 == pWwFib->nVersion ? 26 : 6 );
        *pOut << 'T' << hex6 << nStart << dec2 << ' ' << indent1 << begin1
              << pNm << ", Len: " << nLen
              << ", " << aPLCF.GetIMax() << " Elements" << endl1;

        for( USHORT i = 0; i < aPLCF.GetIMax(); ++i )
        {
            long nCp = aPLCF.GetPos( i );
            if( nCp >= LONG_MAX )
                break;

            *pOut << indent2 << i << ".Cp: 0x" << hex << nCp + nOffset;
            long* pFSPA = (long*)aPLCF.GetData( i );
            if( 8 == pWwFib->nVersion )
            {
                *pOut << " ShapeId: 0x" << *pFSPA++;
                *pOut << " left: " << dec << *pFSPA++;
                *pOut << " top: " << *pFSPA++;
                *pOut << " right: " << *pFSPA++;
                *pOut << " bottom: " << *pFSPA++;
                USHORT* pU = (USHORT*)pFSPA;
                *pOut << " flags: 0x" << hex << *pU++;
                pFSPA = (long*)pU;
                *pOut << " xTxbx: " << dec << *pFSPA;
            }
            else
            {
                *pOut << " FC of DO: 0x" << *pFSPA++;
                *pOut << " ctcbx: " << dec << *(USHORT*)pFSPA;
            }

            *pOut << endl1;
        }
        *pOut << end1;
    }
    else
        *pOut << "No ";
    *pOut << pNm << endl1 << endl1;
}

static void DumpTxtboxBrks( const char* pNm, long nStart, long nLen,
                            long nOffset )
{
    if( nStart && nLen )
    {
        WW8PLCFspecial aPLCF( &xTableStream, nStart, nLen, 6 );
        *pOut << 'T' << hex6 << nStart << dec2 << ' ' << indent1 << begin1
              << pNm << ", Len: " << nLen
              << ", " << aPLCF.GetIMax() << " Elements" << endl1;

        for( USHORT i = 0; i < aPLCF.GetIMax(); ++i )
        {
            long nCp = aPLCF.GetPos( i );
            if( nCp >= LONG_MAX )
                break;

            USHORT* pBKD = (USHORT*)aPLCF.GetData( i );
            *pOut << indent2 << i << ".Cp: 0x" << hex << nCp + nOffset
                  << " itxbxs: 0x" << *pBKD++;
            *pOut << " dcpDepend: 0x" << *pBKD++;
            *pOut << " flags: 0x" << hex << *pBKD << dec << endl1;
        }
        *pOut << end2;
    }
    else
        *pOut << "No ";
    *pOut << pNm << endl1 << endl1;
}

static void DumpFdoa( WW8ScannerBase* pBase )
{
    long nOffset = pWwFib->ccpText + pWwFib->ccpFtn
                        + pWwFib->ccpHdr + pWwFib->ccpAtn + pWwFib->ccpEdn;

    if( 8 == pWwFib->nVersion )
    {
        DumpDrawObjects( "DrawObjects in Maintext",
                            pWwFib->fcPlcfspaMom, pWwFib->lcbPlcfspaMom,
                            /*nOffset*/0 );
        // PLCF fuer TextBox-Break-Deskriptoren im Maintext
        DumpTxtboxBrks( "TextBox-Break-Desk. im Maintext",
                        pWwFib->fcPlcftxbxBkd, pWwFib->lcbPlcftxbxBkd,
                        nOffset );
    }
    else
        DumpDrawObjects( "DrawObjects in Maintext",
                            pWwFib->fcPlcfdoaMom, pWwFib->lcbPlcfdoaMom,
                            /*nOffset*/0 );

    nOffset += pWwFib->ccpHdrTxbx;

    if( 8 == pWwFib->nVersion )
    {
        DumpDrawObjects( "DrawObjects in Head/Foot",
                    pWwFib->fcPlcfspaHdr, pWwFib->lcbPlcfspaHdr,
                    /*nOffset*/0 );
        // PLCF fuer TextBox-Break-Deskriptoren im Header-/Footer-Bereich
        DumpTxtboxBrks( "TextBox-Break-Desk. im Head/Foot",
                        pWwFib->fcPlcfHdrtxbxBkd, pWwFib->lcbPlcfHdrtxbxBkd,
                        nOffset );
    }
    else
        DumpDrawObjects( "DrawObjects in Head/Foot",
                    pWwFib->fcPlcfdoaHdr, pWwFib->lcbPlcfdoaHdr,
                    /*nOffset*/0 );
}

BOOL ReadEsherRec( SvStream& rStrm, UINT8& rVer, UINT16& rInst,
                    UINT16& rFbt, UINT32& rLength )
{
    UINT16 aBits;
    if( !WW8ReadUINT16( rStrm, aBits ) ) return FALSE;
    rVer  =  aBits & 0x000F;
    rInst = (aBits & 0xFFF0) >> 4;
    //----------------------------------------------
    if( !WW8ReadUINT16( rStrm, rFbt ) ) return FALSE;
    //----------------------------------------------
    return WW8ReadUINT32( rStrm, rLength );
}

const char* _GetShapeTypeNm( UINT16 nId )
{
    const char* aNmArr[ 202 + 2 + 1 ] = {
/*   0*/    "NotPrimitive","Rectangle","RoundRectangle","Ellipse","Diamond","IsocelesTriangle","RightTriangle","Parallelogram","Trapezoid","Hexagon",
/*  10*/    "Octagon","Plus","Star","Arrow","ThickArrow","HomePlate","Cube","Balloon","Seal","Arc",
/*  20*/    "Line","Plaque","Can","Donut","TextSimple","TextOctagon","TextHexagon","TextCurve","TextWave","TextRing",
/*  30*/    "TextOnCurve","TextOnRing","StraightConnector1","BentConnector2","BentConnector3","BentConnector4","BentConnector5","CurvedConnector2","CurvedConnector3","CurvedConnector4",
/*  40*/    "CurvedConnector5","Callout1","Callout2","Callout3","AccentCallout1","AccentCallout2","AccentCallout3","BorderCallout1","BorderCallout2","BorderCallout3",
/*  50*/    "AccentBorderCallout1","AccentBorderCallout2","AccentBorderCallout3","Ribbon","Ribbon2","Chevron","Pentagon","NoSmoking","Seal8","Seal16",
/*  60*/    "Seal32","WedgeRectCallout","WedgeRRectCallout","WedgeEllipseCallout","Wave","FoldedCorner","LeftArrow","DownArrow","UpArrow","LeftRightArrow",
/*  70*/    "UpDownArrow","IrregularSeal1","IrregularSeal2","LightningBolt","Heart","PictureFrame","QuadArrow","LeftArrowCallout","RightArrowCallout","UpArrowCallout",
/*  80*/    "DownArrowCallout","LeftRightArrowCallout","UpDownArrowCallout","QuadArrowCallout","Bevel","LeftBracket","RightBracket","LeftBrace","RightBrace","LeftUpArrow",
/*  90*/    "BentUpArrow","BentArrow","Seal24","StripedRightArrow","NotchedRightArrow","BlockArc","SmileyFace","VerticalScroll","HorizontalScroll","CircularArrow",
/* 100*/    "NotchedCircularArrow","UturnArrow","CurvedRightArrow","CurvedLeftArrow","CurvedUpArrow","CurvedDownArrow","CloudCallout","EllipseRibbon","EllipseRibbon2","FlowChartProcess",
/* 110*/    "FlowChartDecision","FlowChartInputOutput","FlowChartPredefinedProcess","FlowChartInternalStorage","FlowChartDocument","FlowChartMultidocument","FlowChartTerminator","FlowChartPreparation","FlowChartManualInput","FlowChartManualOperation",
/* 120*/    "FlowChartConnector","FlowChartPunchedCard","FlowChartPunchedTape","FlowChartSummingJunction","FlowChartOr","FlowChartCollate","FlowChartSort","FlowChartExtract","FlowChartMerge","FlowChartOfflineStorage",
/* 130*/    "FlowChartOnlineStorage","FlowChartMagneticTape","FlowChartMagneticDisk","FlowChartMagneticDrum","FlowChartDisplay","FlowChartDelay","TextPlainText","TextStop","TextTriangle","TextTriangleInverted",
/* 140*/    "TextChevron","TextChevronInverted","TextRingInside","TextRingOutside","TextArchUpCurve","TextArchDownCurve","TextCircleCurve","TextButtonCurve","TextArchUpPour","TextArchDownPour",
/* 150*/    "TextCirclePour","TextButtonPour","TextCurveUp","TextCurveDown","TextCascadeUp","TextCascadeDown","TextWave1","TextWave2","TextWave3","TextWave4",
/* 160*/    "TextInflate","TextDeflate","TextInflateBottom","TextDeflateBottom","TextInflateTop","TextDeflateTop","TextDeflateInflate","TextDeflateInflateDeflate","TextFadeRight","TextFadeLeft",
/* 170*/    "TextFadeUp","TextFadeDown","TextSlantUp","TextSlantDown","TextCanUp","TextCanDown","FlowChartAlternateProcess","FlowChartOffpageConnector","Callout90","AccentCallout90",
/* 180*/    "BorderCallout90","AccentBorderCallout90","LeftRightUpArrow","Sun","Moon","BracketPair","BracePair","Seal4","DoubleWave","ActionButtonBlank",
/* 190*/    "ActionButtonHome","ActionButtonHelp","ActionButtonInformation","ActionButtonForwardNext","ActionButtonBackPrevious","ActionButtonEnd","ActionButtonBeginning","ActionButtonReturn","ActionButtonDocument","ActionButtonSound",
/* 200*/    "ActionButtonMovie","HostControl","TextBox","Nil", "???"
    };
    if( 203 < nId )
        nId = 204;
    return aNmArr[ nId ];
}

void DumpEscherProp( UINT16 nId, BOOL bBid, BOOL bComplex, UINT32 nOp,
                        UINT32& rStreamOffset )
{
    const char* pRecNm = 0;
    switch( nId )
    {
    case 4:     pRecNm = "DFF_Prop_Rotation"; break;
// Protection
    case 119:   pRecNm = "DFF_Prop_LockRotation"; break;
    case 120:   pRecNm = "DFF_Prop_LockAspectRatio"; break;
    case 121:   pRecNm = "DFF_Prop_LockPosition"; break;
    case 122:   pRecNm = "DFF_Prop_LockAgainstSelect"; break;
    case 123:   pRecNm = "DFF_Prop_LockCropping"; break;
    case 124:   pRecNm = "DFF_Prop_LockVertices"; break;
    case 125:   pRecNm = "DFF_Prop_LockText"; break;
    case 126:   pRecNm = "DFF_Prop_LockAdjustHandles"; break;
    case 127:   pRecNm = "DFF_Prop_LockAgainstGrouping"; break;
// Text
    case 128:   pRecNm = "DFF_Prop_lTxid"; break;
    case 129:   pRecNm = "DFF_Prop_dxTextLeft"; break;
    case 130:   pRecNm = "DFF_Prop_dyTextTop"; break;
    case 131:   pRecNm = "DFF_Prop_dxTextRight"; break;
    case 132:   pRecNm = "DFF_Prop_dyTextBottom"; break;
    case 133:   pRecNm = "DFF_Prop_WrapText"; break;
    case 134:   pRecNm = "DFF_Prop_scaleText"; break;
    case 135:   pRecNm = "DFF_Prop_anchorText"; break;
    case 136:   pRecNm = "DFF_Prop_txflTextFlow"; break;
    case 137:   pRecNm = "DFF_Prop_cdirFont"; break;
    case 138:   pRecNm = "DFF_Prop_hspNext"; break;
    case 139:   pRecNm = "DFF_Prop_txdir"; break;
    case 187:   pRecNm = "DFF_Prop_SelectText"; break;
    case 188:   pRecNm = "DFF_Prop_AutoTextMargin"; break;
    case 189:   pRecNm = "DFF_Prop_RotateText"; break;
    case 190:   pRecNm = "DFF_Prop_FitShapeToText"; break;
    case 191:   pRecNm = "DFF_Prop_FitTextToShape"; break;
// GeoText
    case 192:   pRecNm = "DFF_Prop_gtextUNICODE"; break;
    case 193:   pRecNm = "DFF_Prop_gtextRTF"; break;
    case 194:   pRecNm = "DFF_Prop_gtextAlign"; break;
    case 195:   pRecNm = "DFF_Prop_gtextSize"; break;
    case 196:   pRecNm = "DFF_Prop_gtextSpacing"; break;
    case 197:   pRecNm = "DFF_Prop_gtextFont"; break;
    case 240:   pRecNm = "DFF_Prop_gtextFReverseRows"; break;
    case 241:   pRecNm = "DFF_Prop_fGtext"; break;
    case 242:   pRecNm = "DFF_Prop_gtextFVertical"; break;
    case 243:   pRecNm = "DFF_Prop_gtextFKern"; break;
    case 244:   pRecNm = "DFF_Prop_gtextFTight"; break;
    case 245:   pRecNm = "DFF_Prop_gtextFStretch"; break;
    case 246:   pRecNm = "DFF_Prop_gtextFShrinkFit"; break;
    case 247:   pRecNm = "DFF_Prop_gtextFBestFit"; break;
    case 248:   pRecNm = "DFF_Prop_gtextFNormalize"; break;
    case 249:   pRecNm = "DFF_Prop_gtextFDxMeasure"; break;
    case 250:   pRecNm = "DFF_Prop_gtextFBold"; break;
    case 251:   pRecNm = "DFF_Prop_gtextFItalic"; break;
    case 252:   pRecNm = "DFF_Prop_gtextFUnderline"; break;
    case 253:   pRecNm = "DFF_Prop_gtextFShadow"; break;
    case 254:   pRecNm = "DFF_Prop_gtextFSmallcaps"; break;
    case 255:   pRecNm = "DFF_Prop_gtextFStrikethrough"; break;
// Blip
    case 256:   pRecNm = "DFF_Prop_cropFromTop"; break;
    case 257:   pRecNm = "DFF_Prop_cropFromBottom"; break;
    case 258:   pRecNm = "DFF_Prop_cropFromLeft"; break;
    case 259:   pRecNm = "DFF_Prop_cropFromRight"; break;
    case 260:   pRecNm = "DFF_Prop_pib"; break;
    case 261:   pRecNm = "DFF_Prop_pibName"; break;
    case 262:   pRecNm = "DFF_Prop_pibFlags"; break;
    case 263:   pRecNm = "DFF_Prop_pictureTransparent"; break;
    case 264:   pRecNm = "DFF_Prop_pictureContrast"; break;
    case 265:   pRecNm = "DFF_Prop_pictureBrightness"; break;
    case 266:   pRecNm = "DFF_Prop_pictureGamma"; break;
    case 267:   pRecNm = "DFF_Prop_pictureId"; break;
    case 268:   pRecNm = "DFF_Prop_pictureDblCrMod"; break;
    case 269:   pRecNm = "DFF_Prop_pictureFillCrMod"; break;
    case 270:   pRecNm = "DFF_Prop_pictureLineCrMod"; break;
    case 271:   pRecNm = "DFF_Prop_pibPrint"; break;
    case 272:   pRecNm = "DFF_Prop_pibPrintName"; break;
    case 273:   pRecNm = "DFF_Prop_pibPrintFlags"; break;
    case 316:   pRecNm = "DFF_Prop_fNoHitTestPicture"; break;
    case 317:   pRecNm = "DFF_Prop_pictureGray"; break;
    case 318:   pRecNm = "DFF_Prop_pictureBiLevel"; break;
    case 319:   pRecNm = "DFF_Prop_pictureActive"; break;
// Geometry
    case 320:   pRecNm = "DFF_Prop_geoLeft"; break;
    case 321:   pRecNm = "DFF_Prop_geoTop"; break;
    case 322:   pRecNm = "DFF_Prop_geoRight"; break;
    case 323:   pRecNm = "DFF_Prop_geoBottom"; break;
    case 324:   pRecNm = "DFF_Prop_shapePath"; break;
    case 325:   pRecNm = "DFF_Prop_pVertices"; break;
    case 326:   pRecNm = "DFF_Prop_pSegmentInfo"; break;
    case 327:   pRecNm = "DFF_Prop_adjustValue"; break;
    case 328:   pRecNm = "DFF_Prop_adjust2Value"; break;
    case 329:   pRecNm = "DFF_Prop_adjust3Value"; break;
    case 330:   pRecNm = "DFF_Prop_adjust4Value"; break;
    case 331:   pRecNm = "DFF_Prop_adjust5Value"; break;
    case 332:   pRecNm = "DFF_Prop_adjust6Value"; break;
    case 333:   pRecNm = "DFF_Prop_adjust7Value"; break;
    case 334:   pRecNm = "DFF_Prop_adjust8Value"; break;
    case 335:   pRecNm = "DFF_Prop_adjust9Value"; break;
    case 336:   pRecNm = "DFF_Prop_adjust10Value"; break;
    case 378:   pRecNm = "DFF_Prop_fShadowOK"; break;
    case 379:   pRecNm = "DFF_Prop_f3DOK"; break;
    case 380:   pRecNm = "DFF_Prop_fLineOK"; break;
    case 381:   pRecNm = "DFF_Prop_fGtextOK"; break;
    case 382:   pRecNm = "DFF_Prop_fFillShadeShapeOK"; break;
    case 383:   pRecNm = "DFF_Prop_fFillOK"; break;
// FillStyle
    case 384:   pRecNm = "DFF_Prop_fillType"; break;
    case 385:   pRecNm = "DFF_Prop_fillColor"; break;
    case 386:   pRecNm = "DFF_Prop_fillOpacity"; break;
    case 387:   pRecNm = "DFF_Prop_fillBackColor"; break;
    case 388:   pRecNm = "DFF_Prop_fillBackOpacity"; break;
    case 389:   pRecNm = "DFF_Prop_fillCrMod"; break;
    case 390:   pRecNm = "DFF_Prop_fillBlip"; break;
    case 391:   pRecNm = "DFF_Prop_fillBlipName"; break;
    case 392:   pRecNm = "DFF_Prop_fillBlipFlags"; break;
    case 393:   pRecNm = "DFF_Prop_fillWidth"; break;
    case 394:   pRecNm = "DFF_Prop_fillHeight"; break;
    case 395:   pRecNm = "DFF_Prop_fillAngle"; break;
    case 396:   pRecNm = "DFF_Prop_fillFocus"; break;
    case 397:   pRecNm = "DFF_Prop_fillToLeft"; break;
    case 398:   pRecNm = "DFF_Prop_fillToTop"; break;
    case 399:   pRecNm = "DFF_Prop_fillToRight"; break;
    case 400:   pRecNm = "DFF_Prop_fillToBottom"; break;
    case 401:   pRecNm = "DFF_Prop_fillRectLeft"; break;
    case 402:   pRecNm = "DFF_Prop_fillRectTop"; break;
    case 403:   pRecNm = "DFF_Prop_fillRectRight"; break;
    case 404:   pRecNm = "DFF_Prop_fillRectBottom"; break;
    case 405:   pRecNm = "DFF_Prop_fillDztype"; break;
    case 406:   pRecNm = "DFF_Prop_fillShadePreset"; break;
    case 407:   pRecNm = "DFF_Prop_fillShadeColors"; break;
    case 408:   pRecNm = "DFF_Prop_fillOriginX"; break;
    case 409:   pRecNm = "DFF_Prop_fillOriginY"; break;
    case 410:   pRecNm = "DFF_Prop_fillShapeOriginX"; break;
    case 411:   pRecNm = "DFF_Prop_fillShapeOriginY"; break;
    case 412:   pRecNm = "DFF_Prop_fillShadeType"; break;
    case 443:   pRecNm = "DFF_Prop_fFilled"; break;
    case 444:   pRecNm = "DFF_Prop_fHitTestFill"; break;
    case 445:   pRecNm = "DFF_Prop_fillShape"; break;
    case 446:   pRecNm = "DFF_Prop_fillUseRect"; break;
    case 447:   pRecNm = "DFF_Prop_fNoFillHitTest"; break;
// LineStyle
    case 448:   pRecNm = "DFF_Prop_lineColor"; break;
    case 449:   pRecNm = "DFF_Prop_lineOpacity"; break;
    case 450:   pRecNm = "DFF_Prop_lineBackColor"; break;
    case 451:   pRecNm = "DFF_Prop_lineCrMod"; break;
    case 452:   pRecNm = "DFF_Prop_lineType"; break;
    case 453:   pRecNm = "DFF_Prop_lineFillBlip"; break;
    case 454:   pRecNm = "DFF_Prop_lineFillBlipName"; break;
    case 455:   pRecNm = "DFF_Prop_lineFillBlipFlags"; break;
    case 456:   pRecNm = "DFF_Prop_lineFillWidth"; break;
    case 457:   pRecNm = "DFF_Prop_lineFillHeight"; break;
    case 458:   pRecNm = "DFF_Prop_lineFillDztype"; break;
    case 459:   pRecNm = "DFF_Prop_lineWidth"; break;
    case 460:   pRecNm = "DFF_Prop_lineMiterLimit"; break;
    case 461:   pRecNm = "DFF_Prop_lineStyle"; break;
    case 462:   pRecNm = "DFF_Prop_lineDashing"; break;
    case 463:   pRecNm = "DFF_Prop_lineDashStyle"; break;
    case 464:   pRecNm = "DFF_Prop_lineStartArrowhead"; break;
    case 465:   pRecNm = "DFF_Prop_lineEndArrowhead"; break;
    case 466:   pRecNm = "DFF_Prop_lineStartArrowWidth"; break;
    case 467:   pRecNm = "DFF_Prop_lineStartArrowLength"; break;
    case 468:   pRecNm = "DFF_Prop_lineEndArrowWidth"; break;
    case 469:   pRecNm = "DFF_Prop_lineEndArrowLength"; break;
    case 470:   pRecNm = "DFF_Prop_lineJoinStyle"; break;
    case 471:   pRecNm = "DFF_Prop_lineEndCapStyle"; break;
    case 507:   pRecNm = "DFF_Prop_fArrowheadsOK"; break;
    case 508:   pRecNm = "DFF_Prop_fLine"; break;
    case 509:   pRecNm = "DFF_Prop_fHitTestLine"; break;
    case 510:   pRecNm = "DFF_Prop_lineFillShape"; break;
    case 511:   pRecNm = "DFF_Prop_fNoLineDrawDash"; break;
// ShadowStyle
    case 512:   pRecNm = "DFF_Prop_shadowType"; break;
    case 513:   pRecNm = "DFF_Prop_shadowColor"; break;
    case 514:   pRecNm = "DFF_Prop_shadowHighlight"; break;
    case 515:   pRecNm = "DFF_Prop_shadowCrMod"; break;
    case 516:   pRecNm = "DFF_Prop_shadowOpacity"; break;
    case 517:   pRecNm = "DFF_Prop_shadowOffsetX"; break;
    case 518:   pRecNm = "DFF_Prop_shadowOffsetY"; break;
    case 519:   pRecNm = "DFF_Prop_shadowSecondOffsetX"; break;
    case 520:   pRecNm = "DFF_Prop_shadowSecondOffsetY"; break;
    case 521:   pRecNm = "DFF_Prop_shadowScaleXToX"; break;
    case 522:   pRecNm = "DFF_Prop_shadowScaleYToX"; break;
    case 523:   pRecNm = "DFF_Prop_shadowScaleXToY"; break;
    case 524:   pRecNm = "DFF_Prop_shadowScaleYToY"; break;
    case 525:   pRecNm = "DFF_Prop_shadowPerspectiveX"; break;
    case 526:   pRecNm = "DFF_Prop_shadowPerspectiveY"; break;
    case 527:   pRecNm = "DFF_Prop_shadowWeight"; break;
    case 528:   pRecNm = "DFF_Prop_shadowOriginX"; break;
    case 529:   pRecNm = "DFF_Prop_shadowOriginY"; break;
    case 574:   pRecNm = "DFF_Prop_fShadow"; break;
    case 575:   pRecNm = "DFF_Prop_fshadowObscured"; break;
// PerspectiveStyle
    case 576:   pRecNm = "DFF_Prop_perspectiveType"; break;
    case 577:   pRecNm = "DFF_Prop_perspectiveOffsetX"; break;
    case 578:   pRecNm = "DFF_Prop_perspectiveOffsetY"; break;
    case 579:   pRecNm = "DFF_Prop_perspectiveScaleXToX"; break;
    case 580:   pRecNm = "DFF_Prop_perspectiveScaleYToX"; break;
    case 581:   pRecNm = "DFF_Prop_perspectiveScaleXToY"; break;
    case 582:   pRecNm = "DFF_Prop_perspectiveScaleYToY"; break;
    case 583:   pRecNm = "DFF_Prop_perspectivePerspectiveX"; break;
    case 584:   pRecNm = "DFF_Prop_perspectivePerspectiveY"; break;
    case 585:   pRecNm = "DFF_Prop_perspectiveWeight"; break;
    case 586:   pRecNm = "DFF_Prop_perspectiveOriginX"; break;
    case 587:   pRecNm = "DFF_Prop_perspectiveOriginY"; break;
    case 639:   pRecNm = "DFF_Prop_fPerspective"; break;
// 3D Object
    case 640:   pRecNm = "DFF_Prop_c3DSpecularAmt"; break;
    case 641:   pRecNm = "DFF_Prop_c3DDiffuseAmt"; break;
    case 642:   pRecNm = "DFF_Prop_c3DShininess"; break;
    case 643:   pRecNm = "DFF_Prop_c3DEdgeThickness"; break;
    case 644:   pRecNm = "DFF_Prop_c3DExtrudeForward"; break;
    case 645:   pRecNm = "DFF_Prop_c3DExtrudeBackward"; break;
    case 646:   pRecNm = "DFF_Prop_c3DExtrudePlane"; break;
    case 647:   pRecNm = "DFF_Prop_c3DExtrusionColor"; break;
    case 648:   pRecNm = "DFF_Prop_c3DCrMod"; break;
    case 700:   pRecNm = "DFF_Prop_f3D"; break;
    case 701:   pRecNm = "DFF_Prop_fc3DMetallic"; break;
    case 702:   pRecNm = "DFF_Prop_fc3DUseExtrusionColor"; break;
    case 703:   pRecNm = "DFF_Prop_fc3DLightFace"; break;
// 3D Style
    case 704:   pRecNm = "DFF_Prop_c3DYRotationAngle"; break;
    case 705:   pRecNm = "DFF_Prop_c3DXRotationAngle"; break;
    case 706:   pRecNm = "DFF_Prop_c3DRotationAxisX"; break;
    case 707:   pRecNm = "DFF_Prop_c3DRotationAxisY"; break;
    case 708:   pRecNm = "DFF_Prop_c3DRotationAxisZ"; break;
    case 709:   pRecNm = "DFF_Prop_c3DRotationAngle"; break;
    case 710:   pRecNm = "DFF_Prop_c3DRotationCenterX"; break;
    case 711:   pRecNm = "DFF_Prop_c3DRotationCenterY"; break;
    case 712:   pRecNm = "DFF_Prop_c3DRotationCenterZ"; break;
    case 713:   pRecNm = "DFF_Prop_c3DRenderMode"; break;
    case 714:   pRecNm = "DFF_Prop_c3DTolerance"; break;
    case 715:   pRecNm = "DFF_Prop_c3DXViewpoint"; break;
    case 716:   pRecNm = "DFF_Prop_c3DYViewpoint"; break;
    case 717:   pRecNm = "DFF_Prop_c3DZViewpoint"; break;
    case 718:   pRecNm = "DFF_Prop_c3DOriginX"; break;
    case 719:   pRecNm = "DFF_Prop_c3DOriginY"; break;
    case 720:   pRecNm = "DFF_Prop_c3DSkewAngle"; break;
    case 721:   pRecNm = "DFF_Prop_c3DSkewAmount"; break;
    case 722:   pRecNm = "DFF_Prop_c3DAmbientIntensity"; break;
    case 723:   pRecNm = "DFF_Prop_c3DKeyX"; break;
    case 724:   pRecNm = "DFF_Prop_c3DKeyY"; break;
    case 725:   pRecNm = "DFF_Prop_c3DKeyZ"; break;
    case 726:   pRecNm = "DFF_Prop_c3DKeyIntensity"; break;
    case 727:   pRecNm = "DFF_Prop_c3DFillX"; break;
    case 728:   pRecNm = "DFF_Prop_c3DFillY"; break;
    case 729:   pRecNm = "DFF_Prop_c3DFillZ"; break;
    case 730:   pRecNm = "DFF_Prop_c3DFillIntensity"; break;
    case 763:   pRecNm = "DFF_Prop_fc3DConstrainRotation"; break;
    case 764:   pRecNm = "DFF_Prop_fc3DRotationCenterAuto"; break;
    case 765:   pRecNm = "DFF_Prop_fc3DParallel"; break;
    case 766:   pRecNm = "DFF_Prop_fc3DKeyHarsh"; break;
    case 767:   pRecNm = "DFF_Prop_fc3DFillHarsh"; break;
// Shape
    case 769:   pRecNm = "DFF_Prop_hspMaster"; break;
    case 771:   pRecNm = "DFF_Prop_cxstyle"; break;
    case 772:   pRecNm = "DFF_Prop_bWMode"; break;
    case 773:   pRecNm = "DFF_Prop_bWModePureBW"; break;
    case 774:   pRecNm = "DFF_Prop_bWModeBW"; break;
    case 826:   pRecNm = "DFF_Prop_fOleIcon"; break;
    case 827:   pRecNm = "DFF_Prop_fPreferRelativeResize"; break;
    case 828:   pRecNm = "DFF_Prop_fLockShapeType"; break;
    case 830:   pRecNm = "DFF_Prop_fDeleteAttachedObject"; break;
    case 831:   pRecNm = "DFF_Prop_fBackground"; break;

// Callout
    case 832:   pRecNm = "DFF_Prop_spcot"; break;
    case 833:   pRecNm = "DFF_Prop_dxyCalloutGap"; break;
    case 834:   pRecNm = "DFF_Prop_spcoa"; break;
    case 835:   pRecNm = "DFF_Prop_spcod"; break;
    case 836:   pRecNm = "DFF_Prop_dxyCalloutDropSpecified"; break;
    case 837:   pRecNm = "DFF_Prop_dxyCalloutLengthSpecified"; break;
    case 889:   pRecNm = "DFF_Prop_fCallout"; break;
    case 890:   pRecNm = "DFF_Prop_fCalloutAccentBar"; break;
    case 891:   pRecNm = "DFF_Prop_fCalloutTextBorder"; break;
    case 892:   pRecNm = "DFF_Prop_fCalloutMinusX"; break;
    case 893:   pRecNm = "DFF_Prop_fCalloutMinusY"; break;
    case 894:   pRecNm = "DFF_Prop_fCalloutDropAuto"; break;
    case 895:   pRecNm = "DFF_Prop_fCalloutLengthSpecified"; break;

// GroupShape
    case 896:   pRecNm = "DFF_Prop_wzName"; break;
    case 897:   pRecNm = "DFF_Prop_wzDescription"; break;
    case 898:   pRecNm = "DFF_Prop_pihlShape"; break;
    case 899:   pRecNm = "DFF_Prop_pWrapPolygonVertices"; break;
    case 900:   pRecNm = "DFF_Prop_dxWrapDistLeft"; break;
    case 901:   pRecNm = "DFF_Prop_dyWrapDistTop"; break;
    case 902:   pRecNm = "DFF_Prop_dxWrapDistRight"; break;
    case 903:   pRecNm = "DFF_Prop_dyWrapDistBottom"; break;
    case 904:   pRecNm = "DFF_Prop_lidRegroup"; break;
    case 953:   pRecNm = "DFF_Prop_fEditedWrap"; break;
    case 954:   pRecNm = "DFF_Prop_fBehindDocument"; break;
    case 955:   pRecNm = "DFF_Prop_fOnDblClickNotify"; break;
    case 956:   pRecNm = "DFF_Prop_fIsButton"; break;
    case 957:   pRecNm = "DFF_Prop_fOneD"; break;
    case 958:   pRecNm = "DFF_Prop_fHidden"; break;
    case 959:   pRecNm = "DFF_Prop_fPrint"; break;
    }

    *pOut << "      " << indent1 << ' ';
    if( pRecNm )
        *pOut << pRecNm;
    else
        *pOut << "Prop" ;

    *pOut   << " Id: " << dec << nId << " (=0x" << hex << nId << ')';
    if( bBid )
        *pOut << " Bid: 0x" << (UINT16)bBid;

    if( bComplex )
    {
        *pOut << " Cmpl: 0x" << (UINT16)bComplex;
        // ....
        rStreamOffset += nOp;
    }
//  else
        *pOut << " op: 0x" << nOp;

    *pOut << dec << endl1;
}

void DumpEscherRec( ULONG nPos, UINT8 nVer, UINT16 nInst,
                    UINT16 nFbt, UINT32 nLength )
{
    const char* pRecNm = 0;
    switch( nFbt )
    {
    case 0xF000:    pRecNm = "DFF_msofbtDggContainer"; break;
    case 0xF006:    pRecNm = "DFF_msofbtDgg"; break;
    case 0xF016:    pRecNm = "DFF_msofbtCLSID"; break;
    case 0xF00B:    pRecNm = "DFF_msofbtOPT"; break;
    case 0xF11A:    pRecNm = "DFF_msofbtColorMRU"; break;
    case 0xF11E:    pRecNm = "DFF_msofbtSplitMenuColors"; break;
    case 0xF001:    pRecNm = "DFF_msofbtBstoreContainer"; break;
    case 0xF007:    pRecNm = "DFF_msofbtBSE"; break;
    case 0xF018:    pRecNm = "DFF_msofbtBlipFirst"; break;
    case 0xF117:    pRecNm = "DFF_msofbtBlipLast"; break;
    case 0xF002:    pRecNm = "DFF_msofbtDgContainer"; break;
    case 0xF008:    pRecNm = "DFF_msofbtDg"; break;
    case 0xF118:    pRecNm = "DFF_msofbtRegroupItems"; break;
    case 0xF120:    pRecNm = "DFF_msofbtColorScheme"; break;
    case 0xF003:    pRecNm = "DFF_msofbtSpgrContainer"; break;
    case 0xF004:    pRecNm = "DFF_msofbtSpContainer"; break;
    case 0xF009:    pRecNm = "DFF_msofbtSpgr"; break;
    case 0xF00A:    pRecNm = "DFF_msofbtSp"; break;
    case 0xF00C:    pRecNm = "DFF_msofbtTextbox"; break;
    case 0xF00D:    pRecNm = "DFF_msofbtClientTextbox"; break;
    case 0xF00E:    pRecNm = "DFF_msofbtAnchor"; break;
    case 0xF00F:    pRecNm = "DFF_msofbtChildAnchor"; break;
    case 0xF010:    pRecNm = "DFF_msofbtClientAnchor"; break;
    case 0xF011:    pRecNm = "DFF_msofbtClientData"; break;
    case 0xF11F:    pRecNm = "DFF_msofbtOleObject"; break;
    case 0xF11D:    pRecNm = "DFF_msofbtDeletedPspl"; break;
    case 0xF005:    pRecNm = "DFF_msofbtSolverContainer"; break;
    case 0xF012:    pRecNm = "DFF_msofbtConnectorRule"; break;
    case 0xF013:    pRecNm = "DFF_msofbtAlignRule"; break;
    case 0xF014:    pRecNm = "DFF_msofbtArcRule"; break;
    case 0xF015:    pRecNm = "DFF_msofbtClientRule"; break;
    case 0xF017:    pRecNm = "DFF_msofbtCalloutRule"; break;
    }

    *pOut << hex6 << nPos << indent1;
    if( pRecNm )
        *pOut << pRecNm;
    else
        *pOut << "Record:";
    *pOut << " Id: 0x" << hex << nFbt << " Instance: 0x" << nInst
          << " Version: 0x" << (UINT16)nVer << " Laenge: 0x" << nLength
          << dec << endl1;

    switch( nFbt )
    {
    case 0xf00b:        // DFF_msofbtOPT
        {
            UINT16 nId; UINT32 nOp, nStreamOffset = nInst * 6;
            BOOL bBid, bComplex;
            for( UINT16 n = 0; n < nInst; ++n )
            {
                if( !WW8ReadUINT16( *xTableStream, nId ) ||
                    !WW8ReadUINT32( *xTableStream, nOp ))
                    break;
                bBid = ( nId >> 14 ) & 1;
                bComplex = ( nId >> 15 ) & 1;
                nId &= 0x3fff;

                ::DumpEscherProp( nId, bBid, bComplex, nOp, nStreamOffset );
            }
        }
        break;

    case 0xF00d:        // DFF_msofbtClientTextbox
    case 0xF010:        // DFF_msofbtClientAnchor
    case 0xF011:        // DFF_msofbtClientData
        {
            UINT32 nData;
            if( 4 == nLength && WW8ReadUINT32( *xTableStream, nData ))
                *pOut << "      " << indent1 << " Data: "
                      << hex << nData << dec << endl1;
        }
        break;

    case 0xf00a:        // DFF_msofbtSp
        {
            UINT32 nId, nData;
            if( WW8ReadUINT32( *xTableStream, nId ) &&
                WW8ReadUINT32( *xTableStream, nData ))
            {
                *pOut << "      " << indent1 << " \""
                      << _GetShapeTypeNm( nInst )
                      << "\" Id: 0x" << hex << nId
                      << " Flags: 0x" << nData << dec << endl1;
            }
        }
        break;

    case 0xf009:        // DFF_msofbtSpgr
    case 0xf00f:        // DFF_msofbtChildAnchor
        {
            UINT32 nL, nT, nR, nB;
            if( WW8ReadUINT32( *xTableStream, nL ) &&
                WW8ReadUINT32( *xTableStream, nT ) &&
                WW8ReadUINT32( *xTableStream, nR ) &&
                WW8ReadUINT32( *xTableStream, nB ) )
            {
                *pOut << "      " << indent1 << " Rect: (L/T/R/B): " << dec
                      << nL << '/' << nT << '/' << nR << '/' << nB << endl;
            }
        }
        break;

    case 0xf006:    //ESCHER_Dgg
        {
            UINT32 spidMax,     // The current maximum shape ID
                   cidcl,       // The number of ID clusters (FIDCLs)
                   cspSaved,    // The total number of shapes saved
                                 // (including deleted shapes, if undo
                                 // information was saved)
                   cdgSaved;    // The total number of drawings saved

            if( WW8ReadUINT32( *xTableStream, spidMax ) &&
                WW8ReadUINT32( *xTableStream, cidcl ) &&
                WW8ReadUINT32( *xTableStream, cspSaved ) &&
                WW8ReadUINT32( *xTableStream, cdgSaved ))
            {
                *pOut << "      " << indent1 << " " << hex
                      << " spidMax: 0x" << spidMax
                      << " cidcl: 0x" << cidcl
                      << " cspSaved: 0x" << cspSaved
                      << " cdgSaved: 0x" << cdgSaved
                      << dec << endl1;


                UINT32 dgid,    // DG owning the SPIDs in this cluster
                          cspidCur;  // number of SPIDs used so far

                for( UINT32 n = 1; n < cidcl; ++n )
                {
                    if( !WW8ReadUINT32( *xTableStream, dgid ) ||
                        !WW8ReadUINT32( *xTableStream, cspidCur ))
                        break;

                    *pOut << "      " << indent1 << "  " << hex
                          << " dgid: 0x" << dgid
                          << " cspidCur: 0x" << cspidCur
                          << dec << endl1;
                }
            }
        }
        break;

    case 0xF016:    //ESCHER_CLSID
    case 0xF11A:    //ESCHER_ColorMRU
    case 0xF11E:    //ESCHER_SplitMenuColors
//  case 0xF001:    //ESCHER_BstoreContainer
    case 0xF007:    //ESCHER_BSE
    case 0xF018:    //ESCHER_BlipFirst
    case 0xF117:    //ESCHER_BlipLast
    case 0xF118:    //ESCHER_RegroupItems
    case 0xF120:    //ESCHER_ColorScheme
    case 0xF00C:    //ESCHER_Textbox
    case 0xF00E:    //ESCHER_Anchor
    case 0xF11F:    //ESCHER_OleObject
    case 0xF11D:    //ESCHER_DeletedPspl
    case 0xF005:    //ESCHER_SolverContainer
    case 0xF012:    //ESCHER_ConnectorRule
    case 0xF013:    //ESCHER_AlignRule
    case 0xF014:    //ESCHER_ArcRule
    case 0xF015:    //ESCHER_ClientRule
    case 0xF017:    //ESCHER_CalloutRule
    case 0xF119:    //ESCHER_Selection
    case 0xf008:    //ESCHER_Dg
        {
            int nCnt = 128;
            while( nLength )
            {
                if( 128 == nCnt || 16 == ++nCnt )
                {
                    if( 128 != nCnt )
                        *pOut << endl1;
                    *pOut << "      " << indent1 << " Data: ";
                    nCnt = 0;
                }

                static char __READONLY_DATA sHex[17] = { "0123456789abcdef" };
                BYTE c;
                *xTableStream >> c;
                *pOut << sHex[ ( c & 0xf0 ) >> 4 ] << sHex[ c & 0x0f ] << ' ';
                --nLength;
            }
            *pOut << dec << endl1;
        }
        break;
    }


}


void DumpEscherRecs( ULONG nPos, UINT32 nLength )
{
    begin( *pOut, *xTableStream ) << endl1;

    UINT16 nOldFbt = 0;
    ULONG nReadLen = 0;
    while( nReadLen < nLength )
    {
        UINT8 nVer;
        UINT16 nInst, nFbt;
        UINT32 nRecLen;

        if( !::ReadEsherRec( *xTableStream, nVer, nInst, nFbt, nRecLen ))
            break;

        if( !( 0xf000 <= nFbt && nFbt <= 0xf120 ))
        {
            xTableStream->Seek( nPos + nReadLen );
            unsigned char c;
            *xTableStream >> c;

            ++nReadLen;
            if( !::ReadEsherRec( *xTableStream, nVer, nInst, nFbt, nRecLen )
                || !( 0xf000 <= nFbt && nFbt <= 0xf120 ) )
                break;

            *pOut << hex6 << nPos + nReadLen - 1 << indent1
                    << "DummyChar: 0x" << hex << (int)c << dec << endl1;
        }

        ::DumpEscherRec( nPos + nReadLen, nVer, nInst,
                        nFbt, nRecLen );

        nReadLen += 2 * sizeof( UINT32 );
        switch( nFbt )
        {
        case 0xF000:
        case 0xF001:    //ESCHER_BstoreContainer
        case 0xF002:
        case 0xF003:
        case 0xF004:
            DumpEscherRecs( nPos + nReadLen, nRecLen );
            break;
        }

        nReadLen += nRecLen;
        xTableStream->Seek( nPos + nReadLen );
        nOldFbt = nFbt;
    }
    end( *pOut, *xTableStream ) << endl1;
}


void DumpDrawing()
{
    if( pWwFib->lcbDggInfo )
    {
        ULONG nOldPos = xTableStream->Tell(), nReadLen = 0;
        xTableStream->Seek( pWwFib->fcDggInfo );

        *pOut << endl << hex6 << pWwFib->fcDggInfo << dec2 <<  ' ' << indent1
              << begin1 << "Escher (DggInfo): Len: " << pWwFib->lcbDggInfo
              << endl1;

        ::DumpEscherRecs( pWwFib->fcDggInfo, (ULONG)pWwFib->lcbDggInfo );

        end( *pOut, *xTableStream ) << endl1 << endl1;
        xTableStream->Seek( nOldPos );
    }
}


//-----------------------------------------
//      Hilfroutinen fuer Styles
//-----------------------------------------

static short DumpStyleUPX( BYTE nVersion, short nLen, BOOL bPAP )
{
    short cbUPX;
    BOOL bEmpty;


    if( nLen <= 0 ){
        indent( *pOut, *xTableStream );
        *pOut << "very empty UPX." << ((bPAP) ? "papx " : "chpx ");
        *pOut << "Len:" << nLen << endl1;
        return nLen;
    }

    nLen -= WW8SkipOdd( &xTableStream );
    indent( *pOut, *xTableStream );

    xTableStream->Read( &cbUPX, 2 );
    nLen-=  2;

    if ( cbUPX > nLen )
    {
        *pOut << "!cbUPX auf nLen verkleinert! ";
        cbUPX = nLen;
    }

    bEmpty = ( cbUPX <= 0 ) || ( bPAP && ( cbUPX <= 2 ) );

    if ( bEmpty )
        *pOut << "empty ";
    else
        *pOut << begin1;

    *pOut << "UPX." << ((bPAP) ? "papx " : "chpx ");
    *pOut << "Len:" << nLen << " cbUPX:" << cbUPX << ' ';

    if( bPAP )
    {
        USHORT id;

        xTableStream->Read( &id, 2 );
        cbUPX-=  2;
        nLen-=  2;
        *pOut << "ID:" << id;
    }

    *pOut << endl1;

    ULONG nPos = xTableStream->Tell();              // falls etwas falsch interpretiert
                                        // wird, gehts danach wieder richtig
    DumpSprms( nVersion, *xTableStream, cbUPX );

    if ( xTableStream->Tell() != nPos + cbUPX ){
        *pOut << "!Um " << xTableStream->Tell() - nPos + cbUPX
             << " Bytes falsch positioniert!" << endl1;
        xTableStream->Seek( nPos+cbUPX );
    }

    nLen -= cbUPX;

    if ( !bEmpty )
        end( *pOut, *xTableStream ) << "UPX." << ((bPAP) ? "papx " : "chpx ") << endl1;

    return nLen;
}

static void DumpStyleGrupx( BYTE nVersion, short nLen, BOOL bPara )
{
    if( nLen <= 0 )
        return;
    nLen -= WW8SkipOdd( &xTableStream );

    begin( *pOut, *xTableStream ) << "Grupx, Len:" << nLen << endl1;

    if( bPara ) nLen = DumpStyleUPX( nVersion, nLen, TRUE );    // Grupx.Papx
    DumpStyleUPX( nVersion, nLen, FALSE );                                      // Grupx.Chpx

    end( *pOut, *xTableStream ) << "Grupx" << endl1;
}

static void PrintStyleId( USHORT nId )
{
    switch ( nId ){
    case 0xffe: *pOut << "User "; break;
    case 0xfff: *pOut << "Nil "; break;
    default:    *pOut <<  nId << ' '; break;
    }
}

//-----------------------------------------
//              Styles
//-----------------------------------------

void DStyle::Dump1Style( USHORT nNr )
{
    short nSkip, cbStd;
    BYTE* pStr;
    char c;
    indent( *pOut, *xTableStream );

    WW8_STD* pStd = Read1Style( nSkip, &pStr, &cbStd ); // lese Style

    if ( pStr ){                                    // echter Style
        *pOut << begin1;
        switch ( pStd->sgc ){
        case 1:  c = 'P'; break;
        case 2:  c = 'C'; break;
        default: c = '?'; break;
        }
        *pOut << c << "-Style Nr:" << nNr << ' ';
        *pOut << "ID:"; PrintStyleId( pStd->sti );
        *pOut << "BasedOn:"; PrintStyleId( pStd->istdBase );
        *pOut << "Next:" << pStd->istdNext << " Name:\"" << &pStr[1] << "\"";
        *pOut << endl1 << "                       ";
        *pOut << "cbStd:" << cbStd << ' ';
        *pOut << "No of Upx & Upe:" << pStd->cupx << ' ';
        *pOut << "bchUpe:" << pStd->bchUpe << ' ';
        *pOut << "nSkip:" << nSkip << endl1;
    }else{                                  // leerer Slot
        *pOut << "empty Slot Nr:" << nNr << endl1;
    }

    long nPos = xTableStream->Tell();               // falls etwas falsch interpretiert
                                        // wird, gehts danach wieder richtig

    if( pStd && ( pStd->sgc == 1 || pStd->sgc == 2 ) )
        DumpStyleGrupx( nVersion, nSkip, pStd->sgc == 1 );

    if ( pStr )                                 // echter Style
        end( *pOut, *xTableStream ) << c << "-Style" << endl1;

    xTableStream->Seek( nPos+nSkip );

    DELETEZ( pStr );
    DELETEZ( pStd );
}

void DStyle::Dump()
{
    *pOut << hex6 << nStyleStart << ' ' << dec2 << indent1;

    *pOut << begin1       << cstd << " Styles, ";
    *pOut << "Base:"      << cbSTDBaseInFile;
    *pOut << ", Written:" << (fStdStylenamesWritten) ? 'T' : 'F';
    *pOut << ", MaxSti:"  << stiMaxWhenSaved;
    *pOut << ", MaxFix:"  << istdMaxFixedWhenSaved;
    *pOut << ", BuildIn:" << nVerBuiltInNamesWhenSaved;
    *pOut << ", StdFnt:"  << ftcStandardChpStsh << endl1;

    USHORT i;
    for( i=0; i<cstd; i++ )
        Dump1Style( i );

    end( *pOut, *xTableStream ) << "Styles" << endl1 << endl1;
}

//-----------------------------------------
//              Main
//-----------------------------------------

//char cName [266];
//char cOutName [266];

int PrepareConvert( String& rName, String& rOutName, String& rMess )
{
#if 0
    if( argc < 2 ) {
        rMess += "Aufruf: Dump8 InFile [OutFile [StorFile] ] ";
        return 1;
    }else{
        strncpy( cName, *++argv, sizeof( cName )-5 );
    }
    if( !strchr( cName, '.' ) )
        strcat( cName, ".DOC" );

    if( argc >= 3 ){
        strncpy( cOutName, *++argv, sizeof( cOutName ) );
    }else{
        strncpy( cOutName, cName, sizeof( cOutName ) );
        char* p = strchr( cOutName, '.' );
        *p = 0;
        strcat( cOutName, ".DMP" );
    }
#endif
    if( access( rName.GetStr(), 0 ) ){
        rMess += "Kann ";
        rMess += rName;
        rMess += " nicht oeffnen";
        return 1;
    }

    pxStor = new SvStorageRef( new SvStorage( rName, STREAM_STD_READ ) );
    xStrm = (*pxStor)->OpenStream( String( "WordDocument" ), STREAM_STD_READ );

    if ( !xStrm.Is() /* || xStrm->GetError() */ ){
        rMess += "Kann StorageStream \"WordDocument\" in ";
        rMess += rName;
        rMess += " nicht zum Lesen oeffnen";
        DELETEZ( pxStor );
        return 1;
    }

    ULONG nL;
    if ( xStrm->Read( &nL, sizeof( nL ) ) == 0 ){
        rMess += "Kann aus StorageStream \"WordDocument\" in ";
        rMess += rName;
        rMess += " nicht Lesen";
        return 1;
    }
    xStrm->Seek( 0 );

    pOut = new fstream( rOutName, ios::out );
    if ( !pOut ){
        rMess += "Kann Ausgabedatei ";
        rMess += rOutName;
        rMess += " nicht zum Schreiben oeffnen";
        return 1;
    }

#if 0       // erstmal raus wg. Umstellung
    if( argc > 3 ) {
        char StorName[266];
        strncpy( StorName, *++argv, sizeof( StorName )-5 );

        if( !strchr( StorName, '.' ) )
            strcat( StorName, ".STO" );

        SvFileStream aStrm( StorName, STREAM_WRITE|STREAM_TRUNC );
        aStrm << *xStrm;

//      fstream aOut2( StorName, ios::out );
//      aOut2 << *xStrm;

        xStrm->Seek( 0 );
    }
#endif
    rMess += "Ausgabe von ";
    rMess += rName;
    rMess += " in Datei ";
    rMess += rOutName;
    rMess += "......";

    if ( xStrm->Read( &nL, sizeof( nL ) ) == 0 ){
        return 1;
    }
    xStrm->Seek( 0 );

    return 0;
}

int DoConvert( const String& rName, BYTE nVersion )
{
    ULONG nL;
    if ( xStrm->Read( &nL, sizeof( nL ) ) == 0 ){
        return 1;
    }
    xStrm->Seek( 0 );

    pWwFib = new DFib( *xStrm, nVersion );

    if ( pWwFib->nFibError )
        return 1;

#if 0
    *pOut << "Eingabe-Dateiname : \"" << cName << '"' << endl1;
    if ( cOutName[0] == 0 )
        *pOut << "Ausgabe auf \"cout\"" << endl1;
    else
        *pOut << "Ausgabe-Dateiname : \"" << cOutName << '"' << endl1;
    *pOut << endl1;
#endif


    // Nachdem wir nun den FIB eingelesen haben, wissen wir ja,
    // welcher Table-Stream gueltig ist.
    // Diesen oeffnen wir nun.

    switch( pWwFib->nVersion )  // 6 steht fuer "6 ODER 7",  7 steht fuer "NUR 7"
    {
    case 6:
    case 7:
            xTableStream = &xStrm;
            xDataStream = &xStrm;
                    break;
    case 8:
            xTableStream = (*pxStor)->OpenStream(
                        ( 1 == pWwFib->fWhichTblStm ) ? "1Table" : "0Table" ,
                        STREAM_STD_READ );
            xDataStream = (*pxStor)->OpenStream( "Data", STREAM_STD_READ | STREAM_NOCREATE );
            if( !xDataStream.Is() || SVSTREAM_OK != xDataStream->GetError() )
                xDataStream = &xStrm;
                    break;
    default:// Programm-Fehler!
                    /*
                        ACHTUNG: im FILTER nicht "FALSE" sondern "!this()" schreiben,
                                            da sonst Warning unter OS/2
                    */
                    ASSERT( FALSE, "Es wurde vergessen, nVersion zu kodieren!" );
                    return 1;
    }


    // dann erstmal den Dateinamen schreiben:
    *pOut << "Datei: " << rName.GetStr() << endl;

    pSBase = new WW8ScannerBase( &xStrm, &xTableStream, &xDataStream, pWwFib );

    // erstmal die Lowlevel-Funktionen

    ((DFib*)pWwFib)->Dump();                                                        // FIB
    DumpDop(  *pWwFib );                                                                                    // WW8_DOP

    if( ( 8 > pWwFib->nVersion && pWwFib->fComplex ) ||
        pWwFib->lcbClx )
        DumpPcd( pWwFib->nVersion, pWwFib->fcClx, pWwFib->lcbClx );

    DumpBookLow();

    DumpBookHigh();


    DumpPLCF( pWwFib->fcPlcfsed, pWwFib->lcbPlcfsed, SEP );             // SEPX

    DumpPLCF( pWwFib->fcPlcfbteChpx, pWwFib->lcbPlcfbteChpx, CHP ); // CHPX

    DumpPLCF( pWwFib->fcPlcfbtePapx, pWwFib->lcbPlcfbtePapx, PAP ); // PAPX

    {
        DStyle aStyle( *xTableStream, *pWwFib );                                        // Styles
        aStyle.Dump();
    }

    DumpFonts();                                                        // WW8_FFN

  // ... und jetzt die High-Level-Funktionen

    WW8_CP nStartCp = 0;

    DumpPlainText( nStartCp, pWwFib->ccpText, "Main" );
    *pOut << endl1;
    DumpField1( pWwFib->fcPlcffldMom, pWwFib->lcbPlcffldMom,
                "MainText Fields" );

    DumpHeader( pSBase );                                           // Header / Footer
    DumpField1( pWwFib->fcPlcffldHdr, pWwFib->lcbPlcffldHdr,
                "Header/Footer Fields" );

    DumpFootnotes( pSBase );
    DumpField1( pWwFib->fcPlcffldFtn, pWwFib->lcbPlcffldFtn,
                "Footnote Fields" );

    DumpEndnotes( pSBase );
    DumpField1( pWwFib->fcPlcffldEdn, pWwFib->lcbPlcffldEdn,
                "Endnote Fields" );

    DumpAnnotations( pSBase );

    DumpTextBoxs( pSBase );

    DumpField1( pWwFib->fcPlcffldTxbx, pWwFib->lcbPlcffldTxbx,
                "Textbox Fields" );
    DumpField1( pWwFib->fcPlcffldHdrTxbx, pWwFib->lcbPlcffldHdrTxbx,
                "Header/Footer Textbox Fields" );

    if( 8 == pWwFib->nVersion )
        DumpNumList();

    DumpFdoa( pSBase );

    if( pWwFib->lcbPlcfspaHdr || pWwFib->lcbPlcfspaMom )
        DumpDrawing();

    DELETEZ( pSBase );
    DELETEZ( pWwFib );
    return 0;
}

void DeInit()
{
//  DELETEZ( aWwStor );
    xStrm.Clear();
    DELETEZ( pxStor );
    DELETEZ( pOut );
}

/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/ww8/dump/dump8a.cxx,v 1.1.1.1 2000-09-18 17:14:59 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.15  2000/09/18 16:05:02  willem.vandorp
      OpenOffice header added.

      Revision 1.14  2000/02/14 14:39:35  jp
      #70473# changes for unicode

      Revision 1.13  2000/02/14 14:12:01  jp
      piece table is in the table stream

      Revision 1.12  2000/01/25 10:44:58  khz
      change: update the dumper according to import improvements

      Revision 1.11  1999/11/18 22:15:11  jp
      dump fields

      Revision 1.10  1999/11/18 22:09:53  jp
      dump fields

      Revision 1.9  1999/10/27 17:57:35  jp
      changes for tables

      Revision 1.8  1999/09/08 11:58:46  jp
      den neuen WW8Scanner benutzen

      Revision 1.7  1999/09/08 11:14:15  jp
      Escher dump verbessert


      Rev 1.5   16 Aug 1999 12:22:38   JP
   neu: Dump EscherObjects

      Rev 1.4   15 Jun 1999 14:19:18   JP
   new: ListTable and other Contents

      Rev 1.3   25 Jun 1998 16:29:52   KHZ
   doppelte Deklaration von FkpSizeTab in Define umgewandelt

      Rev 1.2   25 Jun 1998 15:43:46   KHZ
   Strukturaenderung fuer PAPX FKPs

      Rev 1.1   10 Jun 1998 17:22:38   KHZ
   Zwischenstand-Sicherung Dumper

      Rev 1.0   27 May 1998 15:29:26   KHZ
   Initial revision.

*************************************************************************/

